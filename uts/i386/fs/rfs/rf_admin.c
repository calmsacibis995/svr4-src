/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_admin.c	1.3.1.1"

/*
 * RFS administrative daemon and associated functions.
 */
#include "sys/list.h"
#include "sys/types.h"
#include "sys/sysinfo.h"
#include "sys/time.h"
#include "sys/fs/rf_acct.h"
#include "sys/systm.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/stream.h"
#include "sys/vnode.h"
#include "sys/cred.h"
#include "vm/seg.h"
#include "rf_admin.h"
#include "sys/rf_messg.h"
#include "sys/rf_comm.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/wait.h"
#include "sys/nserve.h"
#include "sys/rf_cirmgr.h"
#include "sys/rf_debug.h"
#include "sys/rf_sys.h"
#include "sys/inline.h"
#include "sys/rf_adv.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/vfs.h"
#include "sys/fs/rf_vfs.h"
#include "rf_serve.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "rfcl_subr.h"
#include "sys/uio.h"
#include "sys/buf.h"
#include "vm/page.h"
#include "rf_cache.h"
#include "vm/seg_kmem.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/file.h"
#include "sys/session.h"
#include "sys/kmem.h"

/* imports */
extern int	cleanlocks();
extern void	dst_clean();
extern int	nsrmount;
extern int	rf_state;
extern int	waitid();

int		rf_daemon_flag;		/* reason for wakeup */
int		rf_daemon_lock;
rcvd_t		*rf_daemon_rd;
proc_t		*rf_recovery_procp;	/* sleep address for rf_recovery */
int		rf_recovery_flag;	/* set to KILL when it's time to exit */

/* queue for user-level daemon */
ls_elt_t	rf_umsgq = { &rf_umsgq, &rf_umsgq };
int		rf_umsgcnt;

/* rf_daemon stray message queue */
STATIC ls_elt_t	rfd_strayq = { &rfd_strayq, &rfd_strayq };

/* queue of asynchronous work to do */
STATIC ls_elt_t	rfa_workq = { &rfa_workq, &rfa_workq };

STATIC proc_t	*rf_daemon_procp;

STATIC void	rf_recovery();
STATIC void	rf_tmo();
STATIC void	rfadmin_reply();
STATIC void	rf_que_umsg();
STATIC void	rf_disable_cache();
STATIC void	rf_check_mount();
STATIC void	clean_SRD();
STATIC void	rf_srmntck();
STATIC void	clean_sndd();
STATIC void	dec_srmcnt();
STATIC void	rf_rec_cleanup();
STATIC void	rf_user_msg();
STATIC void	rf_cl_fumount();
STATIC void	clean_GRD();

/*
 * RFS administrative daemon, started when file sharing starts.
 * Forks new servers, calls rf_rec_cleanup() for rf_recovery.
 * To reduce stack size, rf_daemon returns a pointer to a function
 * to execute (e.g., rf_serve).  Otherwise it exits rather than returns.
 */
void	(*
rf_daemon())()
{
	int		i;
	sndd_t		reply_port;
	mblk_t		*bp;
	queue_t		*qp;
	rf_request_t	*request;
	rf_response_t	*response;
	rf_common_t	*cop;
	mblk_t		*resp_bp;
	char		usr_msg[ULINESIZ];	/* for user daemon message */
	int		error;
	int		nd;		/* n daemons */
	pid_t		xpid;		/* unwanted out param from newproc */
	int		nret;
	gdp_t		*gp;
	rfa_work_t	*wp;
	file_t		*fp;
	k_siginfo_t	sink;

	/* disassociate this process from terminal */
	sess_create();

	/* ignore all signals */
	for (i = 1; sigismember(&fillset, i); i++) {
		setsigact(i, SIG_IGN, 0, 0);
	}

	u.u_procp->p_flag |= SNOWAIT;
	u.u_procp->p_cstime = u.u_procp->p_stime =
		u.u_procp->p_cutime = u.u_procp->p_utime = 0;
	rf_daemon_procp = u.u_procp;

	bcopy("rf_daemon", u.u_comm, sizeof("rf_daemon"));
	bcopy("rf_daemon", u.u_psargs, sizeof("rf_daemon"));

	relvm(rf_daemon_procp);

	VN_RELE(u.u_cdir);
	u.u_cdir = rootdir;
	VN_HOLD(u.u_cdir);

	if (u.u_rdir) {
		VN_RELE(u.u_rdir);
	}
	u.u_rdir = NULL;

	for (i = 0; i < u.u_nofiles; i++) {
		if (getf(i, &fp) == 0) {
			closef(fp);
			setf(i, NULLFP);
		}
	}

	switch (newproc(NP_FAILOK | NP_NOLAST | NP_SYSPROC, &xpid, &error))  {
	case 0:
		break;
	case 1:
		return rf_recovery;
	default:
		cmn_err(CE_WARN, "rf_recovery newproc failed");
		goto out;
	}

	switch (newproc(NP_FAILOK | NP_NOLAST | NP_SYSPROC, &xpid, &error)) {
	case 0:
		break;
	case 1:
		return rf_tmo;
	default:
		cmn_err(CE_WARN, "rf_tmo newproc failed");
		goto out;
	}

	for (nd = error = 0; nd < minserve && !error; nd++) {
		if ((nret = newproc(NP_FAILOK | NP_NOLAST | NP_SYSPROC, &xpid,
		  &error)) < 0) {
			/* Post signal to the last server so it won't sleep */
			if (rfsr_active_procp != NULL) {
				psignal(rfsr_active_procp, SIGUSR1);
			}
			break;
		} else if (nret == 1) {
			return rf_serve;
		}
	}
	if (nd != minserve) {
		minserve = nd;
		cmn_err(CE_WARN, "rf_daemon:  minserve reset to %d\n", nd);
	}

	/* Discard old user daemon messages. */
	while ((bp = (mblk_t *)LS_REMQUE(&rf_umsgq)) != NULL) {
		rf_freemsg(bp);
	}

	/* Discard old async work items */
	while ((wp = (rfa_work_t *)LS_REMQUE(&rfa_workq)) != NULL) {
		(*wp->rfaw_func)(wp);
	}

	LS_INIT(&rf_umsgq);
	rf_umsgcnt = 0;

	/* TO DO:  allocate a real sndd so recovery can find it. */

	reply_port.sd_stat = SDUSED;
	SDTOV(&reply_port)->v_count = 1;


	/* Discard old stray messages. */
	while ((bp = (mblk_t *)LS_REMQUE(&rfd_strayq)) != NULL) {
		rf_freemsg(bp);
	}

	rf_state = RF_UP;

	/* end of critical section begun in rfsys_start */

	wakeprocs((caddr_t)&rf_state, PRMPT);

	for (;;) {
		register rf_message_t *msg;

		bp = rf_dequeue(rf_daemon_rd);

		if (!bp) {

			/*
			 * Stray message handling schemes that superficially
			 * appear more temperate have subtle drawbacks:
			 * Local processes sleeping on specific RDs are
			 * left hanging or, if awakened, inititate operations
			 * on the remote that collide with ops still in
			 * progress on the remote.  E.g., they close files
			 * that are still being read.
			 */

			bp = (mblk_t *)LS_REMQUE(&rfd_strayq);
			if (bp) {
				gdp_discon("rf_daemon - stray message",
				  QPTOGP(RF_MSG(bp)->m_queue));
				rf_freemsg(bp);
				bp = NULL;
				continue;
			}
		}

		if (!bp) {
			int	s;

			s = splstr();
			if (rf_daemon_flag & RFDDISCON) {
				register struct gdp *tmp;
				register struct gdp *endgdp = gdp + maxgdp;

				rf_daemon_flag &= ~RFDDISCON;
				splx(s);
				for (tmp = gdp; tmp < endgdp; tmp++) {
					if (tmp->constate == GDPDISCONN) {
						rf_rec_cleanup(tmp->queue);
						tmp->constate = GDPRECOVER;
					}
				}
			} else if (rf_daemon_flag & RFDASYNC) {
				splx(s);
				rf_daemon_flag &= ~RFDASYNC;
				while ((wp = (rfa_work_t *)
				  LS_REMQUE(&rfa_workq)) != NULL) {
					(*wp->rfaw_func)(wp);
				}
			} else if (rf_daemon_flag & RFDKILL) {
				splx(s);
				goto out;
			} else if (rf_daemon_flag & RFDSERVE) {
				splx(s);
				rf_daemon_flag &= ~RFDSERVE;
				switch (newproc(NP_FAILOK | NP_NOLAST |
				  NP_SYSPROC, &xpid, &error)) {
				case 0:
					break;
				case 1:
					return rf_serve;
				default:
					/*
					 * Post signal to the last server so
					 * it won't sleep
					 */
					if (rfsr_active_procp != NULL) {
						psignal(rfsr_active_procp, 
						  SIGUSR1);
					}
					if (rfsr_nidle <= 1) {
						cmn_err(CE_NOTE,
						 "rf_daemon: out of servers\n");
					}
					break;
				}

			} else {
				sleep((caddr_t)&rf_daemon_rd->rd_qslp, PRIBIO);
				splx(s);
			}
			continue;
		}

		msg = RF_MSG(bp);
		qp = (queue_t *)msg->m_queue;
		cop = RF_COM(bp);
		gp = QPTOGP(qp);

		if (cop->co_type == RF_REQ_MSG) {

			request = RF_REQ(bp);
			sndd_set(&reply_port, qp, &msg->m_gift);
			reply_port.sd_mntid = cop->co_mntid;

			switch ((int)cop->co_opcode) {

			case REC_FUMOUNT: {
				rf_vfs_t	*rf_vfsp;
				long		req_srmntid ;

				rf_vfsp = findrfvfs(cop->co_mntid);
				req_srmntid = request->rq_rec_fumount.srmntid;
				rf_freemsg(bp);
				bp = NULL;

				/*
				 * We found an rf_vfs with the right mntid;
				 * now see if the resource is from the
				 * originating server.  Note that this is
				 * still ONLY PROBABLISTIC, even though a
				 * little more cautious than SVR3.X.  If the
				 * rf_vfs has no rootvp, it's because the
				 * fumount or recovery kicked off in the
				 * middle of the mount.  It's still okay to go
				 * ahead, though, because we'll mark the rf_vfs
				 * fumounted, and will fail to find sndds or
				 * rcvds linked into the rf_vfs.  Send message
				 * to user level only if mount is fully defined.
				 * Reply to the fumount request in any case.
				 */

				if (rf_vfsp) {
					vnode_t	*rootvp;

					rootvp = rf_vfsp->rfvfs_rootvp;
					if (rootvp &&
					  qp == VTOSD(rootvp)->sd_queue &&
					  req_srmntid ==
					   VTOSD(rootvp)->sd_mntid) {
						size_t	namelen;

						namelen = (size_t)
						  strlen(rf_vfsp->rfvfs_name);
						rf_cl_fumount(rf_vfsp);
						rf_user_msg(RFUD_FUMOUNT,
						  rf_vfsp->rfvfs_name, namelen);
					} else if(!rootvp) {
						rf_cl_fumount(rf_vfsp);
					}
				}
				rfadmin_reply(&reply_port, REC_FUMOUNT);
				break;
			}

			case REC_MSG: {
				size_t	datasz;
				size_t	hdrsz;
				size_t	count;

				/*
				 * Got a message for user-level daemon.
				 * Enque message and wake up daemon.
				 *
				 * ASSERT based on circuit manager guarantees.
				 */

				hdrsz = RF_MIN_REQ(gp->version);

				ASSERT(bp->b_wptr - bp->b_rptr >= hdrsz);

				count = (size_t)request->rq_rec_msg.count;

				if (gp->version < RFS2DOT0) {

					/*
					 * bug-compatible: old RFS sends
					 * DU_DATASIZE bytes, only some
					 * of which are defined.
					 */

					datasz = (size_t)msg->m_size - hdrsz;
				} else {
					datasz = count;
				}

				if (count > ULINESIZ || RF_PULLUP(bp,
				  hdrsz, datasz)) {
					gdp_discon("rf_daemon bad REC_MSG",
					  QPTOGP(qp));
					rf_freemsg(bp);
				} else {

					strncpy(usr_msg, rf_msgdata(bp, hdrsz),
					  count);
					rf_freemsg(bp);
					rfadmin_reply(&reply_port, REC_MSG);
					rf_user_msg(RFUD_GETUMSG, usr_msg,
					  count);
				}
				bp = NULL;
				break;
			}

			case RFSYNCTIME: {

				rfsr_adj_timeskew(gp,
				  request->rq_synctime.time, 0);
				(void)rf_allocmsg(RFV1_MINRESP, (size_t)0,
				  BPRI_MED, FALSE, NULLCADDR, NULLFRP,
				  &resp_bp);
				response = RF_RESP(resp_bp);
				cop = RF_COM(resp_bp);
				response->rp_synctime.time = hrestime.tv_sec;
				response->rp_errno = 0;
				cop->co_type = RF_RESP_MSG;
				cop->co_opcode = RFSYNCTIME;
				rf_clrrpsigs(response, gp->version);
				rf_freemsg(bp);
				bp = NULL;

				/*
				 * Kludge around the fact that recovery can't
				 * find reply_port.
				 */

				if (gp->constate == GDPCONNECT) {
					(void)rf_sndmsg(&reply_port, resp_bp,
					  RFV1_MINRESP, (rcvd_t *)NULL, FALSE);
				} else {
					freemsg(resp_bp);
					resp_bp = NULL;
				}
				break;
			}

			case RFTMO: {
				int	vcver;
				size_t	respsize;

				rf_freemsg(bp);
				bp = NULL;
				vcver = gp->version;
				respsize = RF_MIN_RESP(vcver);

				(void)rf_allocmsg(respsize, (size_t)0,
				  BPRI_MED, FALSE, NULLCADDR, NULLFRP,
				  &resp_bp);
				response = RF_RESP(resp_bp);
				cop = RF_COM(resp_bp);
				response->rp_errno = 0;
				cop->co_type = RF_RESP_MSG;
				cop->co_opcode = RFTMO;
				rf_clrrpsigs(response, vcver);

				/*
				 * Kludge around the fact that recovery can't
				 * find reply_port.
				 */

				if (gp->constate == GDPCONNECT) {
					(void)rf_sndmsg(&reply_port, resp_bp,
					  respsize, (rcvd_t *)NULL, FALSE);
				} else {
					freemsg(resp_bp);
					resp_bp = NULL;
				}
				break;
			}

			case RFCACHEDIS: {
				int	vcver;
				size_t	respsize;
				int	vcode;

				vcver = QPTOGP(qp)->version;
				respsize = RF_MIN_RESP(vcver);

				/* vcode is defined only from newer servers */

				vcode = vcver >= RFS2DOT0 ?
				  request->rq_cachedis.vcode : 0;

				rf_disable_cache(qp,
				  request->rq_cachedis.fhandle, (ulong)vcode);
				rfc_info.rfci_rcv_dis++;
				rf_freemsg(bp);
				bp = NULL;
				(void)rf_allocmsg(respsize, (size_t)0, BPRI_MED,
				  FALSE, NULLCADDR, NULLFRP, &resp_bp);
				ASSERT(resp_bp);
				response = RF_RESP(resp_bp);
				cop = RF_COM(resp_bp);
				response->rp_errno = 0;
				cop->co_type = RF_RESP_MSG;
				cop->co_opcode = RFCACHEDIS;
				rf_clrrpsigs(response, vcver);

				/*
				 * Kludge around the fact that recovery can't
				 * find reply_port.
				 */

				if (gp->constate == GDPCONNECT) {
					(void)rf_sndmsg(&reply_port, resp_bp,
					  respsize, (rcvd_t *)NULL, FALSE);
				} else {
					freemsg(resp_bp);
					resp_bp = NULL;
				}
				break;
			}

			default:
				rf_freemsg(bp);
				bp = NULL;
			}

		} else {

			switch ((int)cop->co_opcode) {

			case RFTMO:
			case RFSYNCTIME:

				/* Request originated by rf_tmo */

			default:
				rf_freemsg(bp);
				bp = NULL;
			}
		}
	}
out:
	
	/*
	 * Wait for children to die.  Set flag bit in case we got
	 * here other than via rfsys_stop, and in case kids are not
	 * yet accepting SIGKILL.
	 */
	
	rf_daemon_flag |= RFDKILL;
	signal(rf_daemon_procp->p_pgrp, SIGKILL);
	(void)waitid(P_ALL, 0, &sink, WEXITED);
	rf_daemon_flag = 0;
	rf_daemon_procp = NULL;
	rf_commdinit();

	/* end of critical section begun in rfsys_start */

	rf_state = RF_DOWN;
	wakeprocs((caddr_t)&rf_state, PRMPT);
	exit(CLD_EXITED, 0);
	/* NOTREACHED */
}

/*
 * Recovery daemon, awakened by rf_rec_cleanup to clean up after
 * fumount or disconnect.  This part of recovery calls routines that
 * can sleep.
 */
STATIC void
rf_recovery()
{
	int	slpret = 0;

	bcopy("rf_recovery", u.u_comm, sizeof("rf_recovery"));
	bcopy("rf_recovery", u.u_psargs, sizeof("rf_recovery"));

	/*
	 * rf_daemon may have issued a SIGKILL, but that would be discarded
	 * if this process was still in newproc() since the parent (rf_daemon)
	 * ignores all signals.	 Exit if RFS is stopping.
	 */

	if (rf_daemon_flag & RFDKILL) {
		rf_recovery_flag = 0;
		exit(CLD_EXITED, 0);
	}

	setsigact(SIGKILL, SIG_DFL, 0, 0);

	u.u_procp->p_cstime = u.u_procp->p_stime =
		u.u_procp->p_cutime = u.u_procp->p_utime = 0;
	rf_recovery_procp = u.u_procp;

	for (;;) {
		while (rf_recovery_flag) {
			if (rf_recovery_flag & RFRECDISCON) {
				register gdp_t *endgdp = gdp + maxgdp;
				register gdp_t *gdpp;

				rf_recovery_flag &= ~RFRECDISCON;
				for (gdpp = gdp; gdpp < endgdp; gdpp++) {
					if (gdpp->constate == GDPRECOVER &&
					  gdpp->queue)  {
						rf_check_mount(gdpp->queue);
						rf_srmntck();
						gdp_put_circuit(&gdpp->queue);
					}
				}
			} else if (rf_recovery_flag & RFRECFUMOUNT) {
				rf_recovery_flag &= ~RFRECFUMOUNT;
				rf_srmntck();
			}
		}
		if (slpret) {
			break;
		}
		slpret =
		  sleep((caddr_t)&rf_recovery_procp,  PZERO + 1 | PCATCH);
	}
	
	if (rf_recovery_procp->p_curinfo) {
		kmem_free((caddr_t)rf_recovery_procp->p_curinfo,
		  sizeof(*rf_recovery_procp->p_curinfo));
		rf_recovery_procp->p_curinfo = NULL;
	}
	rf_recovery_procp->p_cursig = 0;
	rf_recovery_flag = 0;
	rf_recovery_procp = NULL;
	exit(CLD_EXITED, 0);
}


/*
 * Provoke timeouts and recovery over transports that do not themselves
 * provide an adequate facility (e.g., TCP and it's ~10 minute timeout).
 * For now, we use a sixty second timeout, but this should be specifiable
 * for each virtual circuit, e.g., as a mount option.
 */
#define RFTMO_TIME	(60 * HZ)
STATIC int	rftmo_time = RFTMO_TIME;

STATIC void
rf_tmo()
{
	register gdp_t		*endgdp = gdp + maxgdp;
	register gdp_t		*gp;
	register queue_t	*qp;
	mblk_t			*bp;
	int			tid;
	int			opcode;
	size_t			hdrsz;
	union rq_arg		rqarg;
	sndd_t			out_port;

	bcopy("rf_tmo", u.u_comm, sizeof("rf_tmo"));
	bcopy("rf_tmo", u.u_psargs, sizeof("rf_tmo"));

	/*
	 * rf_daemon may have issued a SIGKILL, but that would be discarded
	 * if this process was still in newproc() since the parent (rf_daemon)
	 * ignores all signals.	 Exit if RFS is stopping.
	 */


	if (rf_daemon_flag & RFDKILL) {
		exit(CLD_EXITED, 0);
	}

	setsigact(SIGKILL, SIG_DFL, 0, 0);

	u.u_procp->p_cstime = u.u_procp->p_stime =
		u.u_procp->p_cutime = u.u_procp->p_utime = 0;

	/* TO DO:  allocate a real sndd so recovery can find it. */

	out_port.sd_stat = SDUSED;
	SDTOV(&out_port)->v_count = 1;

	do {
		if (!(dudebug & NO_RECOVER)) {
			for (gp = gdp; gp < endgdp; gp++) {
				if (gp->constate == GDPCONNECT &&
				  (qp = gp->queue) != NULL) {

					/*
					 * timeout is cleared by any arriving
					 * message.
					 */

					if (gp->timeout) {
						gdp_discon(
						  "rfs virtual circuit timeout",
						   gp);
						continue;
					}
					
					/*
					 * We send new peers an RFTMO request,
					 * old ones and RFSYNCTIME.  The former
					 * is not in the old protocol, the
					 * latter is preserved only for
					 * compatability outside this context.
					 *
					 * In either case, we arrange for the
					 * response to go to the rf_daemon_rd,
					 * so we don't have to wait.  The
					 * rf_daemon knows to toss such
					 * responses.
					 */

					rqarg = init_rq_arg;
					sndd_set(&out_port, qp,
					  &rf_daemon_gift);

					if (gp->version < RFS2DOT0) {
						rqarg.rqsynctime.time =
						  hrestime.tv_sec;
						opcode = RFSYNCTIME;
						hdrsz = RFV1_MINREQ;
					} else {
						opcode = RFTMO;
						hdrsz = RF_MIN_REQ(gp->version);
					}

					(void)rf_allocmsg(hdrsz, (size_t)0,
					  BPRI_MED, FALSE, NULLCADDR, NULLFRP,
					  &bp);
					rfcl_reqsetup(bp, &out_port, u.u_cred,
					  opcode, R_ULIMIT);
					RF_REQ(bp)->rq_arg = rqarg;

					/*
					 * Kludge around the fact that recovery can't
					 * find out_port.
					 */

					if (gp->constate == GDPCONNECT) {
						gp->timeout = 1;
						(void)rf_sndmsg(&out_port, bp,
						  hdrsz, rf_daemon_rd, FALSE);
					} else {
						freemsg(bp);
						bp = NULL;
					}
				}
			}
		}

		tid = timeout((void(*)())wakeup, (caddr_t)u.u_procp+1,
		  rftmo_time);

	} while (!sleep((caddr_t)u.u_procp+1, PZERO + 1 | PCATCH));

	untimeout(tid);

	if (u.u_procp->p_curinfo) {
		kmem_free((caddr_t)u.u_procp->p_curinfo,
		  sizeof(*u.u_procp->p_curinfo));
		u.u_procp->p_curinfo = NULL;
	}
	u.u_procp->p_cursig = 0;
	exit(CLD_EXITED, 0);
}

/*
 * Non-sleeping part of rf_recovery:  mark the resources
 * that need to be cleaned up, and awaken the recover
 * daemon to clean them.
 *
 * This routine is called by the rf_daemon when a circuit
 * gets disconnected and when a resource is fumounted
 * (server side of fumount). THIS ROUTINE MUST NOT SLEEP.
 * It must always be runnable to wake up servers
 * sleeping in resources that have been disconnected.  Otherwise
 * these servers and the recovery daemon can deadlock.
 */
STATIC void
rf_rec_cleanup(bad_q)
	queue_t	*bad_q;		/* stream that has gone away */
{
	register rf_resource_t *rsrcp = rf_resource_head.rh_nextp;
	sr_mount_t *srmp;
	sysid_t bad_sysid;

	clean_sndd(bad_q);
	clean_SRD(bad_q);

	/* Wakeup procs sleeping on stream head */
	wakeprocs((caddr_t)bad_q->q_ptr, PRMPT);

	bad_sysid = QPTOGP(bad_q)->sysid;

	/*
	 * Mark bad sr_mount entries on the
	 * resource that has mount from bad system and
	 * fumount is not in progress on that mount
	 */
	while (rsrcp != (rf_resource_t *)&rf_resource_head) {
		if ((srmp = id_to_srm(rsrcp, bad_sysid)) != 0 &&
		     !(srmp->srm_flags & SRM_FUMOUNT)) {
			rf_signal_serve(rsrcp->r_mntid, srmp);
			srmp->srm_flags |= SRM_LINKDOWN;
		}
		rsrcp = rsrcp->r_nextp;
	}
	rf_recovery_flag |= RFRECDISCON;
	wakeprocs((caddr_t)&rf_recovery_procp, PRMPT);
	return;
}


/*
 * Go through vfs list looking for remote mounts over bad stream.
 * Send message to user-level daemon for every mount with bad link.
 * (Kernel recovery works without this routine.)
 */
STATIC void
rf_check_mount(bad_q)
	queue_t	*bad_q;
{
	register rf_vfs_t *rfvfsp = rf_head.rfh_next;

	gdp_put_discon(WR(bad_q));
	while (rfvfsp != (rf_vfs_t *)&rf_head) {
		/*
		 * rfvfsp->rfvfs_mntproc will be non-NULL iff
		 * a mount is in progress on that rf_vfs.
		 * In this case simply mark the associated rd LINKDOWN
		 * and signal any process sleeping waiting for
		 * the mount to complete.  (TO DO:  this mntproc
		 * business is stupid.  We should probably be
		 * pointing at the virtual circuit instead of a
		 * process.)
		 * Else, the rf_vfs->rfvfs_rootvp will be set.
		 * Then the user must be notified of the dropped
		 * link and extra cleanup must be done.
		 */
		if (rfvfsp->rfvfs_mntproc) {
			clean_SRD(bad_q);
		} else if (VTOSD(rfvfsp->rfvfs_rootvp)->sd_queue == bad_q) {
			rfc_mountinval(rfvfsp);
			rf_user_msg(RFUD_DISCONN, rfvfsp->rfvfs_name,
			  (size_t)RFS_NMSZ);
		}
		rfvfsp = rfvfsp->rfvfs_next;
	}
}


/*
 *	Cleanup RDSPECIFIC RDs:
 *	Wakeup procs waiting for reply over stream that went bad.
 */

STATIC void
clean_SRD(bad_q)
	queue_t	*bad_q;		/* stream that has gone away */
{
	register rcvd_t		*rd;
	register rcvd_t		*endrcvd = rcvd + nrcvd;
	sndd_t			*sd;
	long			srm_mntid;

	for (rd = rcvd; rd < endrcvd; rd++)  {
		if (ACTIVE_SRD(rd) &&
		   (sd = rd->rd_sdp) != NULL &&
	    	   sd->sd_queue == bad_q) {
			srm_mntid = rd->rd_sdp->sd_mntid;
			rf_checkq(rd, srm_mntid);
			rd->rd_stat |= RDLINKDOWN;
			wakeprocs((caddr_t)&rd->rd_qslp, PRMPT);
		}
	}
}


/*
 * Check server mounts.  Wake server procs sleeping
 * in resource that went bad.  Pretend client gave up
 * references to rf_rsrc.
 */

STATIC void
rf_srmntck()
{
	rf_resource_t	*rsrcp = rf_resource_head.rh_nextp;
	register rcvd_t	*rd;
	queue_t		*bad_q;
	queue_t		*gdp_sysidtoq();

	while (rsrcp != (rf_resource_t *)&rf_resource_head) {
		sr_mount_t		*srmp = rsrcp->r_mountp;
		register rf_resource_t	*rsrcnextp = rsrcp->r_nextp;
		register long		mntid = rsrcp->r_mntid;

		while (srmp) {
			/* srmp becomes dangling reference in clean_GRD
			 */
			register sr_mount_t *srmnextp = srmp->srm_nextp;

			if (srmp->srm_flags & (SRM_LINKDOWN | SRM_FUMOUNT)) {
				register rcvd_t *endrcvd = rcvd + nrcvd;

				/* Wait for servers to wake, leave resource. */
				while (srmp->srm_slpcnt) {
					sleep((caddr_t)srmp, PZERO);
				}
				bad_q = gdp_sysidtoq(srmp->srm_sysid);
				ASSERT(bad_q);
				/*
				 * Now clean up RDGENERAL RDs
				 */
				for (rd = rcvd; rd < endrcvd; rd++) {
					if (ACTIVE_GRD(rd)) {
						clean_GRD(&rsrcp, rd,
						  bad_q, &srmp, mntid);
					}
				}
			}
			srmp = srmnextp;
		}
		/* free this resource if unadvertised and not mounted */
		if (rsrcp && !rsrcp->r_mountp && (rsrcp->r_flags & R_UNADV)) {
			vnode_t	*rvp = rsrcp->r_rootvp;

			freersc(&rsrcp);
			VN_RELE(rvp);
		}
		rsrcp = rsrcnextp;
	}
}

/*
 * On the server side, signal any server process sleeping
 * in this sr_mount. Count the servers we signal - we must
 * wait for them to finish.
 */
void
rf_signal_serve(srmntid, srmp)
	register long srmntid;
	register sr_mount_t *srmp;
{
	register sndd_t *sd;
	register sndd_t *endsndd = sndd + nsndd;
	register sysid_t sysid = srmp->srm_sysid;

	for (sd = sndd; sd < endsndd; sd++)
		/* TO DO:  make sure there is no race with srmount
		 */
		if (sd->sd_stat & SDSERVE
		    && sd->sd_stat & SDUSED
		    && sd->sd_srvproc
		    && sd->sd_mntid == srmntid
		    && QPTOGP(sd->sd_queue)->sysid == sysid) {
			sd->sd_stat |= SDLINKDOWN;
			psignal(sd->sd_srvproc, SIGTERM);
			srmp->srm_slpcnt++;
		}
}

/*
 * Link is down.  Trash send-descriptors that use it.
 */
STATIC void
clean_sndd(bad_q)
	queue_t	*bad_q;
{
	register sndd_t *sd;
	register sndd_t *endsndd = sndd + nsndd;

	for (sd = sndd; sd < endsndd; sd++) {
		if (sd->sd_stat & SDUSED && !(sd->sd_stat & SDSERVE) &&
		  sd->sd_queue == bad_q) {
			vnode_t	*vp = SDTOV(sd);

			/* CONSTCOND */
			rfc_disable(sd, (ulong)0);
			sd->sd_stat |= SDLINKDOWN;
			if (vp->v_flag & VROOT) {
				rfc_sdabort(VFTORF(vp->v_vfsp));
			}
		}
	}
}

/*
 * Clean RDGENERAL RDs.
 *
 * Traverse rd_user list of this RD.  For each rd_user from
 * this sr_mount index, pretend that client gave up all refs
 * to this RD.
 *
 * (Need bad_q to get sysid for cleanlocks.)
 */
/* ARGSUSED */
STATIC void
clean_GRD(rsrcpp, rd, bad_q, srmpp, srm_mntid)
	rf_resource_t	**rsrcpp;
	rcvd_t		*rd;
	queue_t		*bad_q;
	sr_mount_t	**srmpp;
	long		srm_mntid;
{
	vnode_t		*vp;
	rd_user_t	*rduptr;
	rd_user_t	*rdup_next;
	cred_t		cred;
	sysid_t		badsysid = QPTOGP(bad_q)->sysid;
	int		rdgen;

	bzero((caddr_t)&cred, sizeof(cred_t));
	crhold(&cred);

	rf_checkq(rd, srm_mntid); /* get rid of old messages */
	rdgen = rd->rd_gen;

	/*
	 * rdgen tells us if rd has been deallocated, wouldn't work if
	 * rcvds were kmem_allocked.
	 */
restart:
	if (rd && rd->rd_gen == rdgen && (vp = rd->rd_vp)) {

		/*
		 * We allow the list to change under us.  ru_gen tells
		 * us if rd_user was deallocated, wouldn't work if
		 * they were kmem_allocked.
		 */

		/*
		 * Skip rd_users already processed; assumes that new
		 * rd_users are put on the end of the list.
		 */

		for (rduptr = rd->rd_user_list;
		  rduptr && rduptr->ru_flag & RU_DONE;
		  rduptr = rduptr->ru_next) {
			;
		}

		/* Recover unprocessed rd_users. */

		/* LINTED - stupid lint says rdup_next is uninitialized */
		for ( ; rduptr; rduptr = rdup_next) {
			int	nextgen;
			int	rugen;

		rdup_next = rduptr->ru_next;
			rugen = rduptr->ru_gen;
			if (rdup_next) {
				nextgen = rdup_next->ru_gen;
			}
			rduptr->ru_flag |= RU_DONE;
		if (rduptr->ru_srmntid != srm_mntid
		    || QPTOGP(rduptr->ru_queue)->sysid != badsysid) {
			continue;
		}

		/*
		 * Mimic what a server would do to get rid of references
		 * from client denoted by rduptr.
		 */

		(void)cleanlocks(vp, IGN_PID, badsysid);
			if (rduptr->ru_gen != rugen) {
				/* rduptr was deallocated. */
				goto restart;
			}
		if (*srmpp) {
			ASSERT(rsrcpp && *rsrcpp);
				dec_srmcnt(rsrcpp, srmpp, bad_q,
				  rduptr->ru_vcount);
		}
			if (rduptr->ru_gen != rugen) {
				/* rduptr was deallocated. */
				goto restart;
			}
		rcvd_delete(&rd, QPTOGP(bad_q)->sysid, srm_mntid,
		  rduptr->ru_vcount, u.u_cred);
			if (rdup_next && rdup_next->ru_gen != nextgen) {
				/* rdup_next was deallocated. */
				goto restart;
			}
		}
		
		/* Entire list was processed.  Now clean up. */

		if (rd && rd->rd_gen == rdgen) {
			for (rduptr = rd->rd_user_list;
			  rduptr;
			  rduptr = rduptr->ru_next) {
				rduptr->ru_flag &= ~RU_DONE;
	}
		}
	}
}


/*
 * Decrement the reference count in the sr_mount structure by count.
 * If it goes to zero, do the unmount.
 */
STATIC void
dec_srmcnt(rsrcpp, srmpp, bad_q, count)
	rf_resource_t **rsrcpp;
	sr_mount_t **srmpp;
	queue_t *bad_q;
	unsigned	count;
{
	gdp_t *gdpp;

	if (((*srmpp)->srm_refcnt -= count) > 1) {
		return;
	}

	/*
	 * Giving up last ref for this sr_mount entry - free it.
	 */
	(void)srm_remove(rsrcpp, srmpp);

	/* Client usually cleans up circuit, but client is gone. */
	gdpp = QPTOGP(bad_q);
	--gdpp->mntcnt;
}

/*
 *  An sr_mount entry went bad (disconnect or fumount),
 *  and rf_recovery is cleaning it up.  Throw away any old
 *  messages that are on this rd for the bad entry.
 */
void
rf_checkq(rd, srmid)
	rcvd_t	*rd;
	long srmid;
{
	register int cnt, qcnt;
	mblk_t  * bp;
	long msg_srmid;


	ASSERT(!rd->rd_qslp++);

	qcnt = rd->rd_qcnt;
	for (cnt = 0; cnt < qcnt; cnt++) {
		bp = (mblk_t *)LS_REMQUE(&rd->rd_rcvdq);
		ASSERT(bp);
		msg_srmid = RF_COM(bp)->co_mntid;
		if (msg_srmid == srmid) {
			/* don't service this message - toss it */
			rf_freemsg(bp);
			bp = NULL;
			rd->rd_qcnt--;
			if (RCVDEMP(rd)) {
				rfsr_rmmsg(rd);
				break;
			}
		} else {
			/* message OK - put it back */
			LS_INSQUE(&rd->rd_rcvdq, bp);
		}
	}

	ASSERT(!--rd->rd_qslp);
}

/*
 * Create message for local user-level daemon, and enque it.
 */
STATIC void
rf_user_msg(opcode, name, size)
	int		opcode;
	char		*name;
	size_t		size;
{
	mblk_t		*bp;
	struct 	u_d_msg	*request;

	if ((bp = allocb(sizeof(struct u_d_msg), BPRI_MED)) == NULL) {
		cmn_err(CE_NOTE, "rf_user_msg allocb fails: ");
		cmn_err(CE_CONT, "resource %s disconnected", name);
		return;
	}
	request = (struct u_d_msg *)bp->b_wptr;
	request->opcode = opcode;
	strcpy(request->res_name, name);
	request->count = size;
	rf_que_umsg(bp);
}

/*
 * Send reply with opcode over destination SD.
 */
STATIC void
rfadmin_reply(dest, opcode)
	sndd_t		*dest;		/* reply path */
	int		opcode;		/* what we did */
{
	mblk_t		*resp_bp;
	rf_response_t	*response;
	rf_common_t	*cop;
	gdp_t		*gp = QPTOGP(dest->sd_queue);
	size_t		respsize = RF_MIN_RESP(gp->version);

	(void)rf_allocmsg(respsize, (size_t)0, BPRI_MED, FALSE,
	  NULLCADDR, NULLFRP, &resp_bp);
	ASSERT(resp_bp);
	response = RF_RESP(resp_bp);
	cop = RF_COM(resp_bp);
	response->rp_errno = 0;
	cop->co_type = RF_RESP_MSG;
	cop->co_opcode = opcode;
	rf_clrrpsigs(response, QPTOGP(dest->sd_queue)->version);

	/* Kludge around the fact that recovery can't find reply_port. */

	if (gp->constate == GDPCONNECT) {
		(void)rf_sndmsg(dest, resp_bp, respsize, (rcvd_t *)NULL, FALSE);
	} else {
		freemsg(resp_bp);
		resp_bp = NULL;
	}
}

/*
 * If there's room on user_daemon queue, enque message and wake daemon.
 */
STATIC void
rf_que_umsg(bp)
	mblk_t	*bp;
{
	++rf_umsgcnt;
	LS_INIT(bp);
	LS_INSQUE(&rf_umsgq, bp);
	wakeprocs((caddr_t)&rf_daemon_lock, PRMPT);
}

/*
 * vcode should be 0 if unknown
 */
STATIC void
rf_disable_cache(qp, fhandle, vcode)
	register queue_t *qp;
	register long	fhandle;
	register ulong	vcode;
{
	register sndd_t *sdp;
	register sndd_t	*endsdp = sndd + nsndd;

	for (sdp = sndd; sdp < endsdp; sdp ++) {
		if ((sdp->sd_stat & SDUSED || !LS_ISEMPTY(&sdp->sd_hash)) &&
		  sdp->sd_queue == qp &&
		  sdp->sd_fhandle == fhandle) {
			rfc_disable(sdp, vcode);
			break;
		}
	}
}

/*
 * Client side of forced unmount.
 * Set MFUMOUNT flag in rf_vfs, in case fumount message preceeded completion
 * of another message defining a gift.  Code setting up gift sndds is
 * responsible for checking this flag.
 *
 * Mark SDs that point into resource being force-unmounted.  Wake up
 * processes waiting for reply over this mount point.
 *
 */
STATIC void
rf_cl_fumount(rf_vfsp)
	register rf_vfs_t *rf_vfsp;
{
	register sndd_t *sd;
	register sndd_t *endsndd = sndd + nsndd;
	register rcvd_t *rd;
	register rcvd_t *endrcvd = rcvd + nrcvd;
	register vfs_t *vfsp = RFTOVF(rf_vfsp);	/* upper level */

	rf_vfsp->rfvfs_flags |= MFUMOUNT;
	DUPRINT2(DB_MNT_ADV, "rf_cl_fumount: rf_vfsp %x\n", rf_vfsp);
	rfc_mountinval(rf_vfsp);
	for (sd = sndd; sd < endsndd; sd++) {
		if (sd->sd_stat & SDUSED && SDTOV(sd)->v_vfsp == vfsp) {
			DUPRINT2(DB_MNT_ADV, "sd %x fumounted\n", sd);
			sd->sd_stat |= SDLINKDOWN;
			dst_clean(sd);
		}
	}
	/*
	 * RD waiting for reply has SD in rd_sdp field
	 */
	for (rd = rcvd; rd < endrcvd; rd++)  {
		sd = rd->rd_sdp;
		if (ACTIVE_SRD(rd) && sd && SDTOV(sd)->v_vfsp == vfsp) {
			DUPRINT2(DB_MNT_ADV,
			    "fumount: waking SRD for rf_vfsp %x\n", rf_vfsp);
			rd->rd_stat |= RDLINKDOWN;
			wakeprocs((caddr_t)&rd->rd_qslp, PRMPT);
		}
	}
}

void
rfd_stray(bp)
	mblk_t	*bp;
{
	if (!(RF_MSG(bp)->m_stat & RF_GIFT)) {
		rf_freemsg(bp);
		return;
	}
	LS_INSQUE(&rfd_strayq, bp);
	wakeprocs((caddr_t)&rf_daemon_rd->rd_qslp, PRMPT);
}

/*
 * Post work for rf_daemon() to do, wake it up.
 */
int
rfa_workenq(wp)
	rfa_work_t	*wp;
{
	LS_INSQUE(&rfa_workq, &wp->rfaw_elt);
	rf_daemon_flag |= RFDASYNC;
	wakeprocs((caddr_t)&rf_daemon_rd->rd_qslp, PRMPT);
	return 0;
}
