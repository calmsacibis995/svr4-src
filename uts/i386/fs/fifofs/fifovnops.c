/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:fifofs/fifovnops.c	1.3"
/*
 * FIFOFS file system vnode operations.  This file system
 * type supports STREAMS-based pipes and FIFOs.
 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/cred.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/file.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/kmem.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/immu.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/fs/fifonode.h"
#include "sys/conf.h"
#include "sys/stream.h"
#include "sys/strsubr.h"
#include "sys/stropts.h"
#include "sys/proc.h"
#include "fs/fs_subr.h"

/*
 * Define the routines/data structures used in this file.
 */
int	fifo_open(),	fifo_close(),	fifo_read(),	fifo_write();
int	fifo_getattr(),	fifo_setattr(),	fifo_realvp(),	fifo_access();
int	fifo_link(),	fifo_fid(),	fifo_fsync(),	fifo_seek();
int	fifo_ioctl(),	fifo_poll();
void	fifo_inactive(),		fifo_rwlock(),	fifo_rwunlock();

/*
 * Define the routines/data structures external to this file.
 */
extern	int	strwrite(),	strread(),	strioctl(),	strpoll();
extern	int	strclose(),	strclean(),	putctl();
extern	int	fifo_stropen(),	nm_unmountall();
extern	dev_t	fifodev;
extern	void	fifo_flush(),	fifoclearid(),	fiforemove();
extern	void	fifo_setjmp(),	runqueues();
extern  int	cleanlocks();

extern struct qinit stwdata;
extern struct qinit strdata;
extern char qrunflag;

struct  streamtab fifoinfo = { &strdata, &stwdata, NULL, NULL };

struct vnodeops fifo_vnodeops = {
	fifo_open,
	fifo_close,
	fifo_read,
	fifo_write,
	fifo_ioctl,
	fs_setfl,
	fifo_getattr,
	fifo_setattr,
	fifo_access,
	fs_nosys,	/* lookup */
	fs_nosys,	/* create */
	fs_nosys,	/* remove */
	fifo_link,
	fs_nosys,	/* rename */
	fs_nosys,	/* mkdir */
	fs_nosys,	/* rmdir */
	fs_nosys,	/* readdir */
	fs_nosys,	/* symlink */
	fs_nosys,	/* readlink */
	fifo_fsync,
	fifo_inactive,
	fifo_fid,
	fifo_rwlock,
	fifo_rwunlock,
	fifo_seek,
	fs_cmp,
	fs_frlock,
	fs_nosys,	/* space */
	fifo_realvp,
	fs_nosys,	/* getpage */
	fs_nosys,	/* putpage */
	fs_nosys,	/* mmap */
	fs_nosys,	/* addmap */
	fs_nosys,	/* delmap */
	fifo_poll,
	fs_nosys,	/* dump */
	fs_pathconf,
	fs_nosys,	/* filler */
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
};

/*
 * Open and stream a FIFO.
 * If this is the first open of the file (FIFO is not streaming),
 * initialize the fifonode and attach a stream to the vnode.
 */
int
fifo_open(vpp, flag, crp)
	struct vnode **vpp;
	int flag;
	struct cred *crp;
{
	struct vnode *vp = *vpp;
	register struct fifonode *fnp = VTOF(vp);
	register int error = 0;
	int firstopen = 0;
	label_t	saveq;

	flag &= ~FCREAT;		/* paranoia */
	saveq = u.u_qsav;
	/*
	 * Setjmp in case open is interrupted.
	 * If it is, close and return error.
	 */
	if (setjmp(&u.u_qsav)) {
		(void) fifo_setjmp(*vpp, flag & FMASK);
		u.u_qsav = saveq;
		return (EINTR);
	}

	if ((flag & FREAD) && (fnp->fn_rcnt++ == 0))
		wakeprocs((caddr_t) &fnp->fn_rcnt, PRMPT);

	if (flag & FWRITE) {
		if ((flag & (FNDELAY|FNONBLOCK)) && fnp->fn_rcnt == 0) {
			u.u_qsav = saveq;
			return (ENXIO);
		}
		if (fnp->fn_wcnt++ == 0)
			wakeprocs((caddr_t) &fnp->fn_wcnt, PRMPT);
	}
	if (flag & FREAD) {
		while (fnp->fn_wcnt == 0) {
			if (flag & (FNDELAY|FNONBLOCK))
				goto str;
			(void) sleep((caddr_t) &fnp->fn_wcnt, PPIPE);
		}
	}
	if (flag & FWRITE) {
		while (fnp->fn_rcnt == 0)
			(void) sleep((caddr_t) &fnp->fn_rcnt, PPIPE);
	}
str:
	/*
	 * If successful stream and first open, twist the queues.
	 */
	if (vp->v_stream == NULL)
		firstopen++;
	error = fifo_stropen(vpp, flag, crp);
	if (firstopen && error == 0)
		vp->v_stream->sd_wrq->q_next = RD(vp->v_stream->sd_wrq);
	u.u_qsav = saveq;
	return (error);
}

/*
 * Close down a stream.
 * Call cleanlocks() and strclean() on every close.
 * If last close (count is <= 1 and vnode reference 
 * count is 1), call strclose() to close down the 
 * stream.
 * If closing a pipe, send hangup message and force 
 * the other end to be unmounted.
 */
/*ARGSUSED*/
int
fifo_close(vp, flag, count, offset, crp)
	struct vnode *vp;
	int flag;
	int count;
	off_t offset;
	struct cred *crp;
{
	register struct fifonode *fnp = VTOF(vp);
	register struct fifonode *fnp2;
	register int error = 0;

	/*
	 * if pipe/FIFO is not streaming, a previous open
	 * may have been interrupted.
	 */
	if (!vp->v_stream)
		return (0);
	/*
	 * clean locks and clear events.
	 */
	cleanlocks(vp, u.u_procp->p_epid, u.u_procp->p_sysid);
	strclean(vp);
	/*
	 * If a file still has the pipe/FIFO open, return.
	 */
	if ((unsigned) count > 1)
		return (0);
	/*
	 * Wake up any sleeping readers/writers.
	 */
	if (flag & FREAD) {
		fnp->fn_rcnt--;
		wakeprocs((caddr_t)vp->v_stream->sd_wrq, PRMPT);
		wakeprocs((caddr_t) &fnp->fn_wcnt, PRMPT);
	}
	if (flag & FWRITE) {
		fnp->fn_wcnt--;
		wakeprocs((caddr_t)RD(vp->v_stream->sd_wrq), PRMPT);
		wakeprocs((caddr_t) &fnp->fn_rcnt, PRMPT);
	}
	fnp->fn_open--;
	if (fnp->fn_open > 0)
		return (0);
	/*
	 * If no more readers and writers, tear down the stream, send
	 * hangup message to other side and force an unmount of
	 * other end.
	 */
	if ((fnp->fn_flag & ISPIPE) && fnp->fn_mate) {
		putctl(vp->v_stream->sd_wrq->q_next, M_HANGUP);
		qenable(vp->v_stream->sd_wrq->q_next);
		fnp2 = VTOF(fnp->fn_mate);
		fnp2->fn_mate = NULL;
		if ((fnp->fn_mate)->v_stream->sd_flag & STRMOUNT) {
			(void) nm_unmountall(FTOV(fnp2), crp);
		}
	}
	error = strclose(vp, flag, crp);
	wakeprocs((caddr_t) &fnp->fn_unique, PRMPT);
	fnp->fn_mate = NULL;
	return (error);
}

/*
 * Read from a pipe or FIFO.
 * return 0 if....
 *    (1) user read request is 0 or no stream
 *    (2) broken pipe with no data
 *    (3) write-only FIFO with no data
 *    (4) no data and delay flags set.
 * While there is no data to read.... 
 *   -  if the NDELAY/NONBLOCK flag is set, return 0/EAGAIN.
 *   -  unlock the fifonode and sleep waiting for a writer.
 *   -  if a pipe and it has a mate, sleep waiting for its mate
 *      to write.
 */
/*ARGSUSED*/
int
fifo_read(vp, uiop, ioflag, crp)
	struct vnode *vp;
	struct uio *uiop;
	int ioflag;
	struct cred *crp;
{
	register struct fifonode *fnp = VTOF(vp);
	register int error = 0;

	if (uiop->uio_resid == 0 || !vp->v_stream)
		return (0);
	do {
		if (RD(vp->v_stream->sd_wrq)->q_first == NULL) {
			if ((fnp->fn_flag & ISPIPE) && 
				(!fnp->fn_mate))
					return (0);
			if (!(fnp->fn_flag & ISPIPE) && 
				(fnp->fn_wcnt == 0))
					return (0);
			if (uiop->uio_fmode & FNDELAY)
				return (0);
			if (uiop->uio_fmode & FNONBLOCK)
				return (EAGAIN);
			fnp->fn_flag |= FIFOREAD;
			vp->v_stream->sd_flag |= RSLEEP;
			fifo_rwunlock(FTOV(fnp));
			(void) sleep((caddr_t)RD(vp->v_stream->sd_wrq), PPIPE);
			fifo_rwlock(FTOV(fnp));
			fnp->fn_flag &= ~FIFOREAD;
		}
	} while ((error = strread(vp, uiop, crp)) == ESTRPIPE);
	if (error == 0) {
		if (fnp->fn_realvp) {
			struct vattr va;

			va.va_mask = AT_ATIME;
			va.va_atime = hrestime;
			VOP_SETATTR(fnp->fn_realvp, &va, 0, crp);
		} else {
			fnp->fn_atime = hrestime.tv_sec;
			if (fnp->fn_mate)
				VTOF(fnp->fn_mate)->fn_atime = hrestime.tv_sec;
		}
	}
	wakeprocs((caddr_t)vp->v_stream->sd_wrq, PRMPT);
	return (error);
}

/*
 * send SIGPIPE and return EPIPE if ...
 *   (1) broken pipe
 *   (2) FIFO is not open for reading
 * return 0 if...
 *   (1) no stream
 *   (2) user request is 0 and STRSNDZERO is not set
 * While the stream is flow controlled.... 
 *   -  if the NDELAY/NONBLOCK flag is set, return 0/EAGAIN.
 *   -  unlock the fifonode and sleep waiting for a reader.
 *   -  if a pipe and it has a mate, sleep waiting for its mate
 *      to read.
 */
/*ARGSUSED*/
int
fifo_write(vp, uiop, ioflag, crp)
	struct vnode *vp;
	struct uio *uiop;
	int ioflag;
	struct cred *crp;
{
	register struct fifonode *fnp = VTOF(vp);
	register int error = 0;
	register int write_size = uiop->uio_resid;

	uiop->uio_offset = 0;
	if (!vp->v_stream)
		return (0);
	if (fnp->fn_rcnt == 0 || (!fnp->fn_mate && fnp->fn_flag & ISPIPE)) {
		psignal(u.u_procp, SIGPIPE);
		return (EPIPE);
	}
	if ((write_size == 0) && !(vp->v_stream->sd_flag & STRSNDZERO))
			return (0);

	while((error = strwrite(vp, uiop, crp)) == ESTRPIPE) {
		if (uiop->uio_fmode & FNDELAY)
			return (0);
		if (uiop->uio_fmode & FNONBLOCK) {
			if (uiop->uio_resid < write_size)
				return (0);
			else
				return (EAGAIN);
		}
		if (fnp->fn_rcnt == 0 || (!fnp->fn_mate && fnp->fn_flag & ISPIPE)) {
			psignal(u.u_procp, SIGPIPE);
			return (EPIPE);
		}
		fnp->fn_flag |= FIFOWRITE;
		vp->v_stream->sd_flag |= WSLEEP;
		fifo_rwunlock(FTOV(fnp));
		(void) sleep((caddr_t)vp->v_stream->sd_wrq, PPIPE);
		fifo_rwlock(FTOV(fnp));
		fnp->fn_flag &= ~FIFOWRITE;
	}
	if (error == 0) {
		if (fnp->fn_realvp) {
			struct vattr va;

			va.va_mask = AT_MTIME;
			va.va_mtime = hrestime;
			VOP_SETATTR(fnp->fn_realvp, &va, 0, crp);
		} else {
			fnp->fn_mtime = fnp->fn_ctime = hrestime.tv_sec;
			if (fnp->fn_mate) {
				VTOF(fnp->fn_mate)->fn_mtime = hrestime.tv_sec;
				VTOF(fnp->fn_mate)->fn_ctime = hrestime.tv_sec;
			}
		}
	}
	wakeprocs((caddr_t)RD(vp->v_stream->sd_wrq), PRMPT);
	return (error);
}

/*
 * Handle I_FLUSH and I_RECVFD request. All ofther request are
 * directly sent to the stream head.
 */
fifo_ioctl(vp, cmd, arg, mode, cr, rvalp)
	register struct vnode *vp;
	int cmd;
	int arg;
	int mode;
	struct cred *cr;
	int *rvalp;
{
	register struct stdata *stp = vp->v_stream;
	register struct fifonode *fnp = VTOF(vp);
	register int error = 0;
	caddr_t wakeadr;

	switch (cmd) {

	default:
		return (strioctl(vp, cmd, arg, mode, U_TO_K, cr, rvalp));

	case I_FLUSH:
		stp = vp->v_stream;
		if (arg & ~FLUSHRW)
			return (EINVAL);
		/*
		 * If there are modules on the stream, pass
		 * the flush request to the stream head.
		 */
		if (stp->sd_wrq->q_next && 
			stp->sd_wrq->q_next->q_qinfo != &strdata)
				return (strioctl(vp, cmd, arg, mode, U_TO_K,
				    cr, rvalp));
		/*
		 * flush the queues.
		 */
		if (arg & FLUSHR) {
			(void) fifo_flush(RD(stp->sd_wrq));
			wakeprocs((caddr_t)vp->v_stream->sd_wrq, PRMPT);
		}
		if ((arg & FLUSHW) && (stp->sd_wrq->q_next)) {
			(void) fifo_flush(stp->sd_wrq->q_next);
			if (fnp->fn_flag & ISPIPE && fnp->fn_mate)
				wakeadr = (caddr_t)(fnp->fn_mate)->v_stream->sd_wrq;
			else
				wakeadr = (caddr_t)vp->v_stream->sd_wrq;
			wakeprocs(wakeadr, PRMPT);
		}
		/*
		 * run the queues 
		 */
		if (qready())
			runqueues();
		break;

	/*
	 * Set the FIFOSEND flag to inform other processes that a file 
	 * descriptor is pending at the stream head of this pipe.
	 * If the flag was already set, sleep until the other
	 * process has completed processing the file descriptor.
	 *
	 * The FIFOSEND flag is set by CONNLD when it is about to
	 * block waiting for the server to recieve the file
	 * descriptor.
	 */
	case I_E_RECVFD:
	case I_RECVFD: 
		if (fnp->fn_flag & FIFOSEND)
			fifo_rwlock(FTOV(fnp));
		if ((error = strioctl(vp, cmd, arg, mode, U_TO_K, cr,
		    rvalp)) == 0) {
			if (fnp->fn_flag & FIFOSEND) {
				fnp->fn_flag &= ~FIFOSEND;
				fifo_rwunlock(FTOV(fnp));
				wakeprocs((caddr_t) &fnp->fn_unique, PRMPT);
			}
		}
		break;
	}
	return (error);
}

/*
 * If shadowing a vnode (FIFOs), apply the VOP_GETATTR to the shadowed 
 * vnode to Obtain the node information. If not shadowing (pipes), obtain
 * the node information from the credentials structure.
 */
int
fifo_getattr(vp, vap, flags, crp)
	struct vnode *vp;
	struct vattr *vap;
	int flags;
	struct cred *crp;
{
	register int error = 0;
	register struct fifonode *fnp = VTOF(vp);
	struct queue *qp;
	struct qband *bandp;

	if (fnp->fn_realvp) {
		/*
		 * for FIFOs or mounted pipes
		 */
		if (error = VOP_GETATTR(fnp->fn_realvp, vap, flags, crp))
			return (error);
	} else {
		/*
		 * for non-attached/ordinary pipes
		 */
		vap->va_mode = 0;
		vap->va_atime.tv_sec = fnp->fn_atime;
		vap->va_atime.tv_nsec = 0;
		vap->va_mtime.tv_sec = fnp->fn_mtime;
		vap->va_mtime.tv_nsec = 0;
		vap->va_ctime.tv_sec = fnp->fn_ctime;
		vap->va_ctime.tv_nsec = 0;
		vap->va_uid = crp->cr_uid;
		vap->va_gid = crp->cr_gid;
		vap->va_nlink = 0;
		vap->va_fsid = fifodev;
		vap->va_nodeid = fnp->fn_ino;
		vap->va_rdev = 0;
	}
	vap->va_type = VFIFO;
	vap->va_blksize = PIPE_BUF;
	/*
	 * Size is number of un-read bytes at the stream head and
	 * nblocks is the unread bytes expressed in blocks.
	 */
	if (vp->v_stream) {
		qp = RD(vp->v_stream->sd_wrq);
		if (qp->q_nband == 0)
			vap->va_size = qp->q_count;
		else {
			for (vap->va_size = qp->q_count, bandp = qp->q_bandp; 
				bandp; 
					bandp = bandp->qb_next)
						vap->va_size += bandp->qb_count;
		}
		vap->va_nblocks = btod(vap->va_size);
	} else {
		vap->va_size = 0;
		vap->va_nblocks = 0;
	}
	vap->va_vcode = 0;
	return (0);
}

/*
 * If shadowing a vnode, apply the VOP_SETATTR to it.
 * Otherwise, set the time and return 0.
 */
int
fifo_setattr(vp, vap, flags, crp)
	struct vnode *vp;
	register struct vattr *vap;
	int flags;
	struct cred *crp;
{
	register struct fifonode *fnp = VTOF(vp);
	register int error = 0;

	if (fnp->fn_realvp)
		error = VOP_SETATTR(fnp->fn_realvp, vap, flags, crp);
	else {
		if (vap->va_mask & AT_ATIME)
			fnp->fn_atime = vap->va_atime.tv_sec;
		if (vap->va_mask & AT_MTIME) {
			fnp->fn_mtime = vap->va_mtime.tv_sec;
			fnp->fn_ctime = hrestime.tv_sec;
		}
	}
	return (error);
}

/*
 * If shadowing a vnode, apply VOP_ACCESS to it.
 * Otherwise, return 0 (allow all access).
 */
int
fifo_access(vp, mode, flags, crp)
	struct vnode *vp;
	int mode;
	int flags;
	struct cred *crp;
{

	if (VTOF(vp)->fn_realvp)
		return (VOP_ACCESS(VTOF(vp)->fn_realvp, mode, flags, crp));
	else
		return (0);
}

/*
 * If shadowing a vnode, apply the VOP_LINK to it.
 * Otherwise, return ENOENT.
 */
int
fifo_link(tdvp, vp, tnm, crp)
	struct vnode *tdvp;
	struct vnode *vp;
	char *tnm;
	struct cred *crp;
{

	if (VTOF(vp)->fn_realvp)
		return (VOP_LINK(tdvp, VTOF(vp)->fn_realvp, tnm, crp));
	else
		return (ENOENT);
}

/*
 * If shadowing a vnode, apply the VOP_FSYNC to it.
 * Otherwise, return 0.
 */
int
fifo_fsync(vp, crp)
	struct vnode *vp;
	struct cred *crp;
{
	if (VTOF(vp)->fn_realvp)
		return (VOP_FSYNC(VTOF(vp)->fn_realvp, crp));
	else
		return (0);
}

/*
 * Called when the upper level no longer holds references to the
 * vnode. Sync the file system and free the fifonode.
 */
void
fifo_inactive(vp, crp)
	struct vnode *vp;
	struct cred *crp;
{
	register struct fifonode *fnp = VTOF(vp);

	if ((fnp->fn_flag & ISPIPE) && !fnp->fn_mate)
		fifoclearid(fnp->fn_ino);
	if (fnp->fn_realvp) {
		(void) fifo_fsync(vp, crp);
		(void) fiforemove(fnp);
		VN_RELE(fnp->fn_realvp);
	}
	kmem_free((caddr_t) fnp, sizeof(struct fifonode));
}

/*
 * If shadowing a vnode, apply the VOP_FID to it.
 * Otherwise, return EINVAL.
 */
int
fifo_fid(vp, fidfnp)
	struct vnode *vp;
	struct fid **fidfnp;
{
	if (VTOF(vp)->fn_realvp)
		return (VOP_FID(VTOF(vp)->fn_realvp, fidfnp));
	else
		return (EINVAL);
}

/*
 * Lock a fifonode.
 */
void
fifo_rwlock(vp)
	struct vnode *vp;
{
	register struct fifonode *fnp = VTOF(vp);

	while (fnp->fn_flag & FIFOLOCK) {
		fnp->fn_flag |= FIFOWANT;
		(void) sleep((caddr_t) fnp, PINOD);
	}
	fnp->fn_flag |= FIFOLOCK;
}

/*
 * Unlock a fifonode.
 */
void
fifo_rwunlock(vp)
	struct vnode *vp;
{
	register struct fifonode *fnp = VTOF(vp);

	fnp->fn_flag &= ~FIFOLOCK;
	if( fnp->fn_flag & FIFOWANT) {
		fnp->fn_flag &= ~FIFOWANT;
		wakeprocs((caddr_t) fnp, PRMPT);
	}
}

/*
 * Return error since seeks are not allowed on pipes.
 */
/*ARGSUSED*/
int
fifo_seek(vp, ooff, noffp)
	struct vnode *vp;
	off_t ooff;
	off_t *noffp;
{
	return (ESPIPE);
}

/*
 * If there is a realvp associated with vp, return it.
 */
int
fifo_realvp(vp, vpp)
	register struct vnode *vp;
	register struct vnode **vpp;
{
	register struct fifonode *fnp = VTOF(vp);
	struct vnode *rvp;

	if ((vp = fnp->fn_realvp) != NULL)
		if (VOP_REALVP(vp, &rvp) == 0)
			vp = rvp;
	*vpp = vp;
	return (0);
}

/*
 * Poll for interesting events on a stream pipe
 */
int
fifo_poll(vp, events, anyyet, reventsp, phpp)
	vnode_t *vp;
	short events;
	int anyyet;
	short *reventsp;
	struct pollhead **phpp;
{
	if (!vp->v_stream)
		return (EINVAL);
	return (strpoll(vp->v_stream, events, anyyet, reventsp, phpp));
}
