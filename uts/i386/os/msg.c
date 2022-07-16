/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:msg.c	1.3"
/*
 * Inter-Process Communication Message Facility.
 */

#include	"sys/types.h"
#include	"sys/debug.h"
#include	"sys/param.h"
#include	"sys/cred.h"
#include	"sys/signal.h"
#include	"sys/immu.h"
#include	"sys/tss.h"
#include	"sys/user.h"
#include	"sys/proc.h"
#include	"sys/buf.h"
#include	"sys/errno.h"
#include	"sys/time.h"
#include	"sys/map.h"
#include	"sys/ipc.h"
#include	"sys/msg.h"
#include	"sys/systm.h"
#include	"sys/sysmacros.h"
#include	"sys/cmn_err.h"
#include	"sys/sysinfo.h"
#include	"sys/disp.h"

extern struct map	msgmap[];	/* msg allocation map */
extern struct msqid_ds	msgque[];	/* msg queue headers */
extern struct msg	msgh[];		/* message headers */
extern struct msginfo	msginfo;	/* message parameters */
extern char		msglock[];

STATIC struct msg	*msgfp;		/* ptr to head of free header list */
STATIC paddr_t		msg;		/* base address of message buffer */

/* Convert bytes to msg segments. */
#define	btoq(X)	(((long)(X) + msginfo.msgssz - 1) / msginfo.msgssz)

#define	INT16_MAX	32767		/* For 286 compatibility, message
					 * identifiers should only use the low
					 * 15 bits.
					 */
/*
 * Argument vectors for the various flavors of msgsys().
 */

#define	MSGGET	0
#define	MSGCTL	1
#define	MSGRCV	2
#define	MSGSND	3

struct msgsysa {
	int		opcode;
};

struct msgctla {
	int		opcode;
	int		msgid;
	int		cmd;
	struct msqid_ds	*buf;
};
	
struct msggeta {
	int		opcode;
	key_t		key;
	int		msgflg;
};
	
struct msgrcva {
	int		opcode;
	int		msqid;
	struct msgbuf	*msgp;
	int		msgsz;
	long		msgtyp;
	int		msgflg;
};

struct msgsnda {
	int		opcode;
	int		msqid;
	struct msgbuf	*msgp;
	int		msgsz;
	int		msgflg;
};

/*
 * msgfree - Free up space and message header, relink pointers on q,
 * and wakeup anyone waiting for resources.
 */
STATIC void
msgfree(qp, pmp, mp, wflag)
	register struct msqid_ds	*qp; /* ptr to q of mesg being freed */
	register struct msg		*mp; /* ptr to msg being freed */
	register struct msg		*pmp; /* ptr to mp's predecessor */
	int wflag;
{
	/* Unlink message from the q. */
	if (pmp == NULL)
		qp->msg_first = mp->msg_next;
	else
		pmp->msg_next = mp->msg_next;
	if (mp->msg_next == NULL)
		qp->msg_last = pmp;
	qp->msg_qnum--;
	if (qp->msg_perm.mode & MSG_WWAIT) {
		qp->msg_perm.mode &= ~MSG_WWAIT;
		wakeprocs((caddr_t)qp, wflag);
	}

	/* Free up message text. */
	if (mp->msg_ts)
		rmfree(msgmap, btoq(mp->msg_ts), (ulong)(mp->msg_spot + 1));

	/* Free up header */
	mp->msg_next = msgfp;
	if (msgfp == NULL)
		wakeprocs((caddr_t)&msgfp, wflag);
	msgfp = mp;
}

/*
 * msgconv - Convert a user supplied message queue id into a ptr to a
 * msqid_ds structure.
 */
STATIC int
msgconv(id, qpp)
	register int	id;
	struct msqid_ds	**qpp;
{
	register struct msqid_ds	*qp;	/* ptr to associated q slot */
	register char			*lockp;	/* ptr to lock.		*/

	if (id < 0)
		return EINVAL;
	qp = &msgque[id % msginfo.msgmni];
	lockp = MSGLOCK(qp);
	while (*lockp)
		(void) sleep(lockp, PMSG);
	*lockp = 1;
	if ((qp->msg_perm.mode & IPC_ALLOC) == 0
	  || id / msginfo.msgmni != qp->msg_perm.seq) {
		*lockp = 0;
		wakeprocs(lockp, PRMPT);
		return EINVAL;
	}
	*qpp = qp;
	return 0;
}

/*
 * msgctl - Msgctl system call.
 */
STATIC int
msgctl(uap, rvp)
	register struct msgctla *uap;
	rval_t *rvp;
{
	struct o_msqid_ds		ods;	/* SVR3 queue work area -
						** to support non-eft applications
						*/

	struct msqid_ds			ds;	/* SVR4 queue work area */
	struct msqid_ds			*qp;	/* ptr to associated q */
	register char			*lockp;
	register int			error;

	if (error = msgconv(uap->msgid, &qp)) /* get msqid_ds for this msgid */
		return error;
	lockp = MSGLOCK(qp);
	rvp->r_val1 = 0;
	switch (uap->cmd) {
	case IPC_O_RMID:
	case IPC_RMID:	/* for use with expanded msqid_ds struct - 
				** msqid_ds not currently used with 
				** this command.
				*/
		if (u.u_cred->cr_uid != qp->msg_perm.uid
		  && u.u_cred->cr_uid != qp->msg_perm.cuid
		  && !suser(u.u_cred)) {
			error = EPERM;
			break;
		}
		while (qp->msg_first)
			msgfree(qp, (struct msg *)NULL, qp->msg_first, PRMPT);
		qp->msg_cbytes = 0;
		if ((unsigned)(uap->msgid + msginfo.msgmni) > INT16_MAX)
			qp->msg_perm.seq = 0;
		else
			qp->msg_perm.seq++;
		if (qp->msg_perm.mode & MSG_RWAIT)
			wakeprocs((caddr_t)&qp->msg_qnum, PRMPT);
		if (qp->msg_perm.mode & MSG_WWAIT)
			wakeprocs((caddr_t)qp, PRMPT);
		qp->msg_perm.mode = 0;
		break;

	case IPC_O_SET:
		if (u.u_cred->cr_uid != qp->msg_perm.uid
		  && u.u_cred->cr_uid != qp->msg_perm.cuid
		  && !suser(u.u_cred)) {
			error = EPERM;
			break;
		}
		if (copyin((caddr_t)uap->buf, (caddr_t)&ods, sizeof(ods))) {
			error = EFAULT;
			break;
		}
		if (ods.msg_qbytes > qp->msg_qbytes && !suser(u.u_cred)) {
			error = EPERM;
			break;
		}
		if (ods.msg_perm.uid > MAXUID || ods.msg_perm.gid > MAXUID){
			error = EINVAL;
			break;
		}
		qp->msg_perm.uid = ods.msg_perm.uid;
		qp->msg_perm.gid = ods.msg_perm.gid;
		qp->msg_perm.mode =
		  (qp->msg_perm.mode & ~0777) | (ods.msg_perm.mode & 0777);
		qp->msg_qbytes = ods.msg_qbytes;
		qp->msg_ctime = hrestime.tv_sec;
		break;

	case IPC_SET:
		if (u.u_cred->cr_uid != qp->msg_perm.uid
		  && u.u_cred->cr_uid != qp->msg_perm.cuid
		  && !suser(u.u_cred)) {
			error = EPERM;
			break;
		}
		if (copyin((caddr_t)uap->buf, (caddr_t)&ds, sizeof(ds))) {
			error = EFAULT;
			break;
		}
		if (ds.msg_qbytes > qp->msg_qbytes && !suser(u.u_cred)) {
			error = EPERM;
			break;
		}
		if (ds.msg_perm.uid < (uid_t)0 || ds.msg_perm.uid > MAXUID ||
			ds.msg_perm.gid < (gid_t)0 || ds.msg_perm.gid > MAXUID){
			error = EINVAL;
			break;
		}
		qp->msg_perm.uid = ds.msg_perm.uid;
		qp->msg_perm.gid = ds.msg_perm.gid;
		qp->msg_perm.mode =
		  (qp->msg_perm.mode & ~0777) | (ds.msg_perm.mode & 0777);
		qp->msg_qbytes = ds.msg_qbytes;
		qp->msg_ctime = hrestime.tv_sec;
		break;

	case IPC_O_STAT:
		if (error = ipcaccess(&qp->msg_perm, MSG_R, u.u_cred))
			break;

		/* copy expanded msqid_ds struct to SVR3 msqid_ds structure -
		** support for non-eft applications.
		** Check whether SVR4 values are too large to store into an SVR3
		** msqid_ds structure.
		*/
		if (qp->msg_perm.uid > USHRT_MAX || qp->msg_perm.gid > USHRT_MAX ||
			qp->msg_perm.cuid > USHRT_MAX || qp->msg_perm.cgid > USHRT_MAX ||
			qp->msg_perm.seq >USHRT_MAX || qp->msg_cbytes > USHRT_MAX ||
			qp->msg_qnum > USHRT_MAX || qp->msg_qbytes > USHRT_MAX || 
			qp->msg_lspid > SHRT_MAX || qp->msg_lrpid > SHRT_MAX){

				error = EOVERFLOW;
				break;
		}
		ods.msg_perm.uid = (o_uid_t) qp->msg_perm.uid;
		ods.msg_perm.gid = (o_gid_t) qp->msg_perm.gid;
		ods.msg_perm.cuid = (o_uid_t) qp->msg_perm.cuid;
		ods.msg_perm.cgid = (o_gid_t) qp->msg_perm.cgid;
		ods.msg_perm.mode = (o_mode_t) qp->msg_perm.mode;
		ods.msg_perm.seq = (ushort) qp->msg_perm.seq;
		ods.msg_perm.key = qp->msg_perm.key;
		ods.msg_first = NULL; 	/* kernel addr */
		ods.msg_last = NULL;
		ods.msg_cbytes = (ushort) qp->msg_cbytes;
		ods.msg_qnum = (ushort) qp->msg_qnum;
		ods.msg_qbytes = (ushort) qp->msg_qbytes;
		ods.msg_lspid = (o_pid_t) qp->msg_lspid;
		ods.msg_lrpid = (o_pid_t) qp->msg_lrpid;
		ods.msg_stime = qp->msg_stime;
		ods.msg_rtime = qp->msg_rtime;
		ods.msg_ctime = qp->msg_ctime;

		if (copyout((caddr_t)&ods, (caddr_t)uap->buf, sizeof(ods))) {
			error = EFAULT;
			break;
		}
		break;

	case IPC_STAT:
		if (error = ipcaccess(&qp->msg_perm, MSG_R, u.u_cred))
			break;

		if (copyout((caddr_t)qp, (caddr_t)uap->buf, sizeof(*qp))) {
			error = EFAULT;
			break;
		}
		break;

	default:
		error = EINVAL;
		break;
	}

	*lockp = 0;
	wakeprocs(lockp, PRMPT);
	return error;
}

/*
 * msgget - Msgget system call.
 */
STATIC int
msgget(uap, rvp)
	register struct msggeta *uap;
	rval_t *rvp;
{
	struct msqid_ds		*qp;	/* ptr to associated q */
	int			s;	/* ipcget status return */
	register int		error;

	if (error = ipcget(uap->key, uap->msgflg, (struct ipc_perm *)msgque,
	  msginfo.msgmni, sizeof(*qp), &s, (struct ipc_perm **)&qp))
		return error;

	if (s) {
		/* This is a new queue.  Finish initialization. */
		qp->msg_first = qp->msg_last = NULL;
		qp->msg_qnum = 0;
		qp->msg_qbytes = (ushort)msginfo.msgmnb;
		qp->msg_lspid = qp->msg_lrpid = 0;
		qp->msg_stime = qp->msg_rtime = 0;
		qp->msg_ctime = hrestime.tv_sec;

		/* initialize reserve area */
		{
		int i;
			for(i=0;i<4;i++)
				qp->msg_perm.pad[i] = 0;
			qp->msg_pad1 = 0;
			qp->msg_pad2 = 0;
			qp->msg_pad3 = 0;

			for(i=0;i<4;i++)
				qp->msg_pad4[i] = 0;
		}
	}
	rvp->r_val1 = qp->msg_perm.seq * msginfo.msgmni + (qp - msgque);
	return 0;
}

/*
 * msginit - Called by main(main.c) to initialize message queues.
 */
void
msginit()
{
	register int		i;	/* loop control */
	register struct msg	*mp;	/* ptr to msg begin linked */
	extern char *kseg();

	/* Allocate physical memory for message buffer. */
	if ((msg = (paddr_t)kseg((int)btoc(msginfo.msgseg * msginfo.msgssz))) ==
	  NULL) {
		cmn_err(CE_NOTE,"Can't allocate message buffer.\n");
		msginfo.msgseg = 0;
	}
	mapinit(msgmap, msginfo.msgmap);
	rmfree(msgmap, (long)msginfo.msgseg, 1L);
	for (i = 0, mp = msgfp = msgh; ++i < msginfo.msgtql; mp++)
		mp->msg_next = mp + 1;
}

/*
 * msgrcv - Msgrcv system call.
 */
STATIC int
msgrcv(uap, rvp)
	register struct msgrcva *uap;
	rval_t *rvp;
{
	register struct msg		*mp;	/* ptr to msg on q */
	register struct msg		*pmp;	/* ptr to mp's predecessor */
	register struct msg		*smp;	/* ptr to best msg on q */
	register struct msg		*spmp;	/* ptr to smp's predecessor */
	struct msqid_ds			*qp;	/* ptr to associated q */
	register char			*lockp;
	struct msqid_ds			*qp1;
	int				sz;	/* transfer byte count */
	int				error;
	int wflag = PRMPT;

	sysinfo.msg++;			/* bump message send/rcv count */
	if (error = msgconv(uap->msqid, &qp))
		return error;
	lockp = MSGLOCK(qp);
	if (error = ipcaccess(&qp->msg_perm, MSG_R, u.u_cred))
		goto msgrcv_out;
	if (uap->msgsz < 0) {
		error = EINVAL;
		goto msgrcv_out;
	}
	smp = spmp = NULL;
	*lockp = 0;
	wakeprocs(lockp, PRMPT);
findmsg:
	if (msgconv(uap->msqid, &qp1) != 0)
		return EIDRM;
	if (qp1 != qp) {
		lockp = MSGLOCK(qp1);
		*lockp = 0;
		wakeprocs(lockp, PRMPT);
		return EIDRM;
	}
	pmp = NULL;
	mp = qp->msg_first;
	if (uap->msgtyp == 0)
		smp = mp;
	else
		for (; mp; pmp = mp, mp = mp->msg_next) {
			if (uap->msgtyp > 0) {
				if (uap->msgtyp != mp->msg_type)
					continue;
				smp = mp;
				spmp = pmp;
				break;
			}
			if (mp->msg_type <= -uap->msgtyp) {
				if (smp && smp->msg_type <= mp->msg_type)
					continue;
				smp = mp;
				spmp = pmp;
			}
		}
	if (smp) {
		if ((unsigned)uap->msgsz < smp->msg_ts)
			if (!(uap->msgflg & MSG_NOERROR)) {
				error = E2BIG;
				goto msgrcv_out;
			} else
				sz = uap->msgsz;
		else
			sz = smp->msg_ts;
		if (copyout((caddr_t)&smp->msg_type, (caddr_t)uap->msgp,
		  sizeof(smp->msg_type))) {
			error = EFAULT;
			goto msgrcv_out;
		}
		if (sz
		  && copyout((caddr_t)(msg + msginfo.msgssz * smp->msg_spot),
		    (caddr_t)uap->msgp + sizeof(smp->msg_type), sz)) {
			error = EFAULT;
			goto msgrcv_out;
		}
		rvp->r_val1 = sz;
		qp->msg_cbytes -= smp->msg_ts;
		qp->msg_lrpid = u.u_procp->p_pid;
		qp->msg_rtime = hrestime.tv_sec;
		wflag = NOPRMPT;
		msgfree(qp, spmp, smp, NOPRMPT);
		goto msgrcv_out;
	}
	if (uap->msgflg & IPC_NOWAIT) {
		error = ENOMSG;
		goto msgrcv_out;
	}
	qp->msg_perm.mode |= MSG_RWAIT;
	*lockp = 0;
	wakeprocs(lockp, wflag);
	if (sleep((caddr_t)&qp->msg_qnum, PMSG|PCATCH))
		return EINTR;
	goto findmsg;

msgrcv_out:

	*lockp = 0;
	wakeprocs(lockp, PRMPT);
	return error;
}

/*
 * msgsnd - Msgsnd system call.
 */
STATIC int
msgsnd(uap, rvp)
	register struct msgsnda *uap;
	rval_t *rvp;
{
	struct msqid_ds			*qp;	/* ptr to associated q */
	register struct msg		*mp;	/* ptr to allocated msg hdr */
	register int			cnt;	/* byte count */
	register ulong			spot;	/* msg pool allocation spot */
	register char			*lockp;
	struct msqid_ds			*qp1;
	long				type;	/* msg type */
	int				error;
	int wflag = PRMPT;

	sysinfo.msg++;			/* bump message send/rcv count */
	if (error = msgconv(uap->msqid, &qp))
		return error;
	lockp = MSGLOCK(qp);
	if (error = ipcaccess(&qp->msg_perm, MSG_W, u.u_cred))
		goto msgsnd_out;
	if ((cnt = uap->msgsz) < 0 || cnt > msginfo.msgmax) {
		error = EINVAL;
		goto msgsnd_out;
	}
	if (copyin((caddr_t)uap->msgp, (caddr_t)&type, sizeof(type))) {
		error = EFAULT;
		goto msgsnd_out;
	}
	if (type < 1) {
		error = EINVAL;
		goto msgsnd_out;
	}
	*lockp = 0;
	wakeprocs(lockp, PRMPT);
getres:
	/* Be sure that q has not been removed. */

	if (msgconv(uap->msqid, &qp1) != 0)
		return EIDRM;
	if (qp1 != qp) {
		lockp = MSGLOCK(qp1);
		*lockp = 0;
		wakeprocs(lockp, PRMPT);
		return EIDRM;
	}

	/* Allocate space on q, message header, & buffer space. */
	if (cnt + qp->msg_cbytes > (uint)qp->msg_qbytes) {
		if (uap->msgflg & IPC_NOWAIT) {
			error = EAGAIN;
			goto msgsnd_out;
		}
		qp->msg_perm.mode |= MSG_WWAIT;
		*lockp = 0;
		wakeprocs(lockp, PRMPT);
		if (sleep((caddr_t)qp, PMSG|PCATCH)) {
			if (error = msgconv(uap->msqid, &qp1))
				return error;
			error = EINTR;
			if (qp1 != qp) {
				lockp = MSGLOCK(qp1);
				*lockp = 0;
				wakeprocs(lockp, PRMPT);
				return error;
			}
			qp->msg_perm.mode &= ~MSG_WWAIT;
			wakeprocs((caddr_t)qp, PRMPT);
			goto msgsnd_out;
		}
		goto getres;
	}
	if (msgfp == NULL) {
		if (uap->msgflg & IPC_NOWAIT) {
			error = EAGAIN;
			goto msgsnd_out;
		}
		*lockp = 0;
		wakeprocs(lockp, PRMPT);
		if (sleep((caddr_t)&msgfp, PMSG|PCATCH)) {
			if (error = msgconv(uap->msqid, &qp1))
				return error;
			error = EINTR;
			if (qp1 != qp) {
				lockp = MSGLOCK(qp1);
				*lockp = 0;
				wakeprocs(lockp, PRMPT);
				return error;
			}
			goto msgsnd_out;
		}
		goto getres;
	}
	mp = msgfp;
	msgfp = mp->msg_next;
	if (cnt && (spot = rmalloc(msgmap, btoq(cnt))) == NULL) {
		if (uap->msgflg & IPC_NOWAIT) {
			error = EAGAIN;
			goto msgsnd_out1;
		}
		mapwant(msgmap)++;
		mp->msg_next = msgfp;
		if (msgfp == NULL)
			wakeprocs((caddr_t)&msgfp, PRMPT);
		msgfp = mp;
		*lockp = 0;
		wakeprocs(lockp, PRMPT);
		if (sleep((caddr_t)msgmap, PMSG|PCATCH)) {
			if (error = msgconv(uap->msqid, &qp1))
				return error;
			error = EINTR;
			if (qp1 != qp) {
				lockp = MSGLOCK(qp1);
				*lockp = 0;
				wakeprocs(lockp, PRMPT);
				return error;
			}
			goto msgsnd_out;
		}
		goto getres;
	}

	/* Everything is available, copy in text and put msg on q. */
	if (cnt
	  && copyin((caddr_t)uap->msgp + sizeof(type),
	    (caddr_t)(msg + msginfo.msgssz * --spot), cnt)) {
		error = EFAULT;
		rmfree(msgmap, btoq(cnt), spot + 1);
		goto msgsnd_out1;
	}
	qp->msg_qnum++;
	qp->msg_cbytes += cnt;
	qp->msg_lspid = u.u_procp->p_pid;
	qp->msg_stime = hrestime.tv_sec;
	mp->msg_next = NULL;
	mp->msg_type = type;
	mp->msg_ts = (ushort)cnt;
	mp->msg_spot = (short)(cnt ? spot : -1);
	if (qp->msg_last == NULL)
		qp->msg_first = qp->msg_last = mp;
	else {
		qp->msg_last->msg_next = mp;
		qp->msg_last = mp;
	}
	if (qp->msg_perm.mode & MSG_RWAIT) {
		qp->msg_perm.mode &= ~MSG_RWAIT;
		wflag = NOPRMPT;
		wakeprocs((caddr_t)&qp->msg_qnum, NOPRMPT);

	}
	rvp->r_val1 = 0;
	goto msgsnd_out;

msgsnd_out1:

	mp->msg_next = msgfp;
	if (msgfp == NULL)
		wakeprocs((caddr_t)&msgfp, PRMPT);
	msgfp = mp;

msgsnd_out:

	*lockp = 0;
	wakeprocs(lockp, wflag);
	return error;
}

/*
 * msgsys - System entry point for msgctl, msgget, msgrcv, and msgsnd
 * system calls.
 */
int
msgsys(uap, rvp)
	register struct msgsysa *uap;
	rval_t *rvp;
{
	register int error;

	switch (uap->opcode) {
	case MSGGET:
		error = msgget((struct msggeta *)uap, rvp);
		break;
	case MSGCTL:
		error = msgctl((struct msgctla *)uap, rvp);
		break;
	case MSGRCV:
		error = msgrcv((struct msgrcva *)uap, rvp);
		break;
	case MSGSND:
		error = msgsnd((struct msgsnda *)uap, rvp);
		break;
	default:
		error = EINVAL;
		break;
	}

	return error;
}
