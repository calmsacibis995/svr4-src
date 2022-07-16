/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:fifofs/fifosubr.c	1.3"
/*
 * The routines defined in this file are supporting routines for FIFOFS 
 * file sytem type.
 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/kmem.h"
#include "sys/immu.h"
#include "sys/inline.h"
#include "sys/file.h"
#include "sys/proc.h"
#include "sys/stat.h"
#include "sys/sysmacros.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/mode.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/uio.h"
#include "sys/fs/fifonode.h"
#include "sys/stream.h"
#include "sys/strsubr.h"
#include "sys/stropts.h"
#include "sys/cmn_err.h"
#include "fs/fs_subr.h"

/*
 * The next set of lines define the bit map for obtaining unique pipe-ino.
 * The number of open pipes is limited to FIFOBYTE. The value of 32K is 
 * chosen so not to exceed the size of a short.
 * fifomap   --> 32K bits, one for each possible value of pipe-ino.
 * testid(x) --> is x already in use?
 * setid(x)  --> mark x as a  used pipe-ino.
 * clearid(x)--> un-mark x as a used pipe-ino.
 */
#define FIFOBYTE	32768
#define FIFOMAP	FIFOBYTE/8
char fifomap 	[FIFOMAP];
#define testid(i)		((fifomap[i/8] & (1 << (i%8))))
#define setid(i)		((fifomap[i/8] |= (1 << (i%8))))
#define clearid(i)		((fifomap[i/8] &= ~(1 << (i%8))))

/*
 * Define routines/data structures within this file.
 */
struct	fifonode	*fifoalloc;
dev_t	fifodev;
struct	vfs	*fifovfsp;
int	fifofstype;
int	fifoinit(),	fs_sync(),	fifo_stropen();
ushort		fifogetid();
void		fifoclearid(),	fifoinsert(),	fiforemove(),	fifo_flush();
struct vnode	*fifovp(),	*makepipe();
struct fifonode	*fifofind();

/*
 * Declare external routines/variables.
 */
extern void	fifo_rwlock(),	fifo_rwunlock(),	freemsg();
extern struct	fifonode 	*fifofind();
extern struct	vnodeops	fifo_vnodeops;
extern int	stropen(),	fifo_close(),	closef();

struct vfsops fifovfsops = {
	fs_nosys,	/* mount */
	fs_nosys,	/* umount */
	fs_nosys,	/* root */
	fs_nosys,	/* statvfs */
	fs_sync,
	fs_nosys,	/* vget */
	fs_nosys,	/* mountroot */
	fs_nosys,	/* swapvp */
	fs_nosys,	/* filler */
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
};

/*
 * Save file system type/index, initialize vfs operations vector, get
 * unique device number for FIFOFS and initialize the FIFOFS hash.
 * Create and initialize a "generic" vfs pointer that will be placed
 * in the v_vfsp field of each pipes vnode.
 */
int
fifoinit(vswp, fstype)
	register struct vfssw *vswp;
	int fstype;
{
	register int dev;

	fifofstype = fstype;
	vswp->vsw_vfsops = &fifovfsops;
	if ((dev = getudev()) == -1) {
		cmn_err(CE_WARN, "fifoinit: can't get unique device number");
		dev = 0;
	}
	fifodev = makedevice(dev, 0);
	fifoalloc = NULL;

	fifovfsp = (struct vfs *)kmem_zalloc(sizeof(struct vfs), KM_SLEEP);
	fifovfsp->vfs_next = NULL;
	fifovfsp->vfs_op = &fifovfsops;
	fifovfsp->vfs_vnodecovered = NULL;
	fifovfsp->vfs_flag = 0;
	fifovfsp->vfs_bsize = 1024;
	fifovfsp->vfs_fstype = fifofstype;
	fifovfsp->vfs_fsid.val[0] = fifodev;
	fifovfsp->vfs_fsid.val[1] = fifofstype;
	fifovfsp->vfs_data = NULL;
	fifovfsp->vfs_dev = fifodev;
	fifovfsp->vfs_bcount = 0;
	return (0);
}

/*
 * Provide a shadow for a vnode. If vp already has a shadow in the hash list,
 * return its shadow. Otherwise, create a vnode to shadow vp, hash the 
 * new vnode and return its pointer to the caller.
 */
struct vnode *
fifovp(vp, crp)
	struct vnode *vp;
	struct cred *crp;
{
	register struct fifonode *fnp;
	register struct vnode *newvp;
	struct vattr va;

	if ((fnp = fifofind(vp)) == NULL) {
		fnp = (struct fifonode *)kmem_zalloc(sizeof(struct fifonode),
			KM_SLEEP);
		       
		/*
		 * initialize the times from vp.
		 */
		va.va_mask = AT_TIMES;
		if (vp && VOP_GETATTR(vp, &va, 0, crp) == 0) {
			fnp->fn_atime = va.va_atime.tv_sec;
			fnp->fn_mtime = va.va_mtime.tv_sec;
			fnp->fn_ctime = va.va_ctime.tv_sec;
		}
		fnp->fn_realvp = vp;
		newvp = FTOV(fnp);
		newvp->v_op = &fifo_vnodeops;
		newvp->v_count = 1;
		newvp->v_data = (caddr_t)fnp;
		if (vp != NULL) {        /* hold as long as shadow is active */
			VN_HOLD(vp);
			newvp->v_type = VFIFO;
			newvp->v_vfsp = vp->v_vfsp;
			newvp->v_rdev = vp->v_rdev;
		} 
		fifoinsert(fnp);
	}
	return FTOV(fnp);
}

/*
 * Create a pipe end by...
 * allocating a vnode-fifonode pair, intializing the fifonode,
 * setting the ISPIPE flag in the fifonode and assigning a unique
 * ino to the fifonode.
 */
struct vnode *
makepipe()
{
	register struct fifonode *fnp;
	register struct vnode *newvp;

	fnp = (struct fifonode *)kmem_zalloc(sizeof(struct fifonode), KM_SLEEP);

	fnp->fn_rcnt = fnp->fn_wcnt = 1;
	fnp->fn_atime = fnp->fn_mtime = fnp->fn_ctime = hrestime.tv_sec;
	fnp->fn_flag |= ISPIPE;

	newvp = FTOV(fnp);
	newvp->v_count = 1;
	newvp->v_op = &fifo_vnodeops;
	newvp->v_vfsp = fifovfsp;
	newvp->v_type = VFIFO;
	newvp->v_rdev = fifodev;
	newvp->v_data = (caddr_t) fnp;
	return (newvp);
}

/*
 * Release a pipe-ino.
 */
void
fifoclearid(ino)
	ushort ino;
{
	clearid(ino);
}

/*
 * Attempt to establish a unique pipe id. Start searching the bit map where 
 * the previous search stopped. If a free bit is located, set the bit and 
 * return the new position in the bit map.
 */
ushort
fifogetid(ino)
	ushort ino;
{
	register ushort i = ino;
	register ushort j;

	for (j = FIFOBYTE; j ; j--) {
		i = (i >= (ushort)(FIFOBYTE -1)) ? 1 : i + 1;
		if (!testid(i))
			break;
	}

	if (j == 0) {
		i = 0;
		cmn_err(CE_WARN, 
			"fifogetid(): could not establish a unique node id\n");
	}
	setid(i);
	return (i);
}

/*
 * Stream a pipe/FIFO.
 * The FIFOPASS flag is used when CONNLD is pushed on the stream.
 * If the flag is set, a new vnode is being passed to the upper
 * layer file system as the vnode representing an open request.
 * In that case, this process will sleep until the FIFOPASS flag
 * has been turned off.
 *
 * After returning from stropen, if the FIFOPASS flag has been set,
 * CONNLD is on the pipe and has placed a new vnode in the
 * fn_unique field of the fifonode. In that case, return the new
 * vnode to the upper layer and release the current vnode.
 */
int
fifo_stropen(vpp, flag, crp)
	struct vnode **vpp;
	int flag;
	struct cred *crp;
{
	register error = 0;
	register struct vnode *oldvp = *vpp;
	struct fifonode *fnp = VTOF(*vpp);
	struct stdata *stp;
	dev_t pdev = 0;


	if (fnp->fn_flag & FIFOPASS)
		(void) fifo_rwlock(FTOV(fnp));
	
	if ((error = stropen(oldvp, &pdev, flag, crp)) != 0)
		return (error);
	
	fnp->fn_open++;

	/*
	 * If the vnode was switched (connld on the pipe), return the
	 * new vnode (in fn_unique field) to the upper layer and 
	 * release the old/original one.
	 */
	if (fnp->fn_flag & FIFOPASS) {
		*vpp = fnp->fn_unique;
		fnp->fn_unique->v_flag |= VNOMAP;
		fnp->fn_flag &= ~FIFOPASS;
		(void) fifo_rwunlock(FTOV(fnp));
		(void) fifo_close(oldvp, 0, 0, 0, crp);
		VN_RELE(oldvp);
	}
	/*
	 * Set up the stream head in order to maintain compatibility.
	 * Check the hi-water, low-water and packet sizes to ensure 
	 * the user can at least write PIPE_BUF bytes to the stream 
	 * head and that a message at least PIPE_BUF bytes can be 
	 * packaged and placed on the stream head's read queue
	 * (atomic writes).
	 */
	stp = (*vpp)->v_stream;
	stp->sd_flag |= OLDNDELAY;
	if (stp->sd_strtab->st_rdinit->qi_minfo->mi_hiwat < PIPE_BUF) {
		stp->sd_strtab->st_rdinit->qi_minfo->mi_hiwat = PIPE_BUF;
		RD(stp->sd_wrq)->q_hiwat = PIPE_BUF;
	}
	if (stp->sd_strtab->st_rdinit->qi_minfo->mi_maxpsz < PIPE_BUF) {
		stp->sd_strtab->st_rdinit->qi_minfo->mi_maxpsz = PIPE_BUF;
		RD(stp->sd_wrq)->q_maxpsz = PIPE_BUF;
	}
	if (stp->sd_strtab->st_rdinit->qi_minfo->mi_lowat > PIPE_BUF) {
		stp->sd_strtab->st_rdinit->qi_minfo->mi_lowat = PIPE_BUF;
		RD(stp->sd_wrq)->q_lowat = PIPE_BUF;
	}
	if (stp->sd_strtab->st_rdinit->qi_minfo->mi_minpsz > PIPE_BUF) {
		stp->sd_strtab->st_rdinit->qi_minfo->mi_minpsz = PIPE_BUF;
		RD(stp->sd_wrq)->q_minpsz = PIPE_BUF;
	}
	return (0);
}

/*
 * Clean up the state of a FIFO and/or mounted pipe in the
 * event that a fifo_open() was interrupted while the 
 * process was sleeping.
 */
void
fifo_setjmp(vp, flag)
	struct vnode *vp;
	int flag;
{
	register struct fifonode *fnp = VTOF(vp);

	cleanlocks(vp, u.u_procp->p_epid, u.u_procp->p_sysid);

	if (flag & FREAD) {
		fnp->fn_rcnt--;
		wakeprocs((caddr_t) &fnp->fn_wcnt, PRMPT);
	}
	if (flag & FWRITE) {
		fnp->fn_wcnt--;
		wakeprocs((caddr_t) &fnp->fn_rcnt, PRMPT);
	}
}

/*
 * Insert a fifonode-vnode pair onto the fifoalloc hash list.
 */
void
fifoinsert(fnp)
	struct fifonode *fnp;
{
	fnp->fn_backp = NULL;
	fnp->fn_nextp = fifoalloc;
	fifoalloc = fnp;
	if (fnp->fn_nextp)
		fnp->fn_nextp->fn_backp = fnp;
}

/*
 * Find a fifonode-vnode pair on the fifoalloc hash list. 
 * vp is a vnode to be shadowed. If it's on the hash list,
 * it already has a shadow, therefore return its corresponding 
 * fifonode.
 * Since this routine is used for FIFOs, a reference needs to be created
 * on the FIFOs vnode.
 */
struct fifonode *
fifofind(vp)
	struct vnode *vp;
{
	register struct fifonode *fnode;

	for (fnode = fifoalloc;  fnode;  fnode = fnode->fn_nextp)
		if (fnode->fn_realvp == vp) {
			VN_HOLD(FTOV(fnode));
			return (fnode);
		}
	return (NULL);
}

/*
 * Remove a fifonode-vnode pair from the fifoalloc hash list.
 * This routine is called from the fifo_inactive() routine when a
 * FIFO is being released.
 * If the link to be removed is the only link, set fifoalloc to NULL.
 */
void
fiforemove(fnp)
	struct fifonode *fnp;
{
	register struct fifonode *fnode;

	if (fifoalloc != NULL && fifoalloc == fnp && 
		!fifoalloc->fn_nextp && !fifoalloc->fn_backp)
			fifoalloc = NULL;

	for (fnode = fifoalloc;  fnode;  fnode = fnode->fn_nextp)
		if (fnode == fnp) {
			if (fnp == fifoalloc)
				fifoalloc = fnp->fn_nextp;
			if (fnode->fn_nextp)
				fnode->fn_nextp->fn_backp = fnode->fn_backp;
			if (fnode->fn_backp)
				fnode->fn_backp->fn_nextp = fnode->fn_nextp;
			break;
		}
				
}

/*
 * Flush "all" messages on qp. 
 * If pending PASSFD messages on the queue, close the file.
 * If flow control has been lifted, enable the queues.
 */
void
fifo_flush(qp)
register queue_t *qp;
{
	mblk_t *mp, *tmp;
	int wantw;
	queue_t *nq;
	register int s;

	s = splstr();
	wantw = qp->q_flag & QWANTW;
	mp = qp->q_first;
	qp->q_first = qp->q_last = NULL;
	qp->q_count = 0;
	qp->q_flag &= ~(QFULL | QWANTW);
	splx(s);
	while (mp) {
		tmp = mp->b_next;
		if (mp->b_datap->db_type == M_PASSFP) 
			closef(((struct strrecvfd *)mp->b_rptr)->f.fp);
		freemsg(mp);
		mp = tmp;
	}

	/*
	 * Only data messages can be queued on the
	 * stream head read queue.  We just flushed
	 * the queue, so there is no need to check
	 * if q_count < q_lowat.
	 */
	if (wantw) {
		/* find nearest back queue with service proc */
		for (nq = backq(qp); nq && !nq->q_qinfo->qi_srvp; nq = backq(nq))
			;
		if (nq)
			qenable(nq);
	}
}

/* XENIX Support */
/*
 * XENIX rdchk support.
 */
int
fifo_rdchk(vp)
struct vnode *vp;
{
	struct fifonode *fnp = VTOF(vp);

	if (vp->v_type != VFIFO || vp->v_op != &fifo_vnodeops)
		return 0;

	if (fnp->fn_flag & ISPIPE)
		/*
		 * If it's a pipe and the other end is still open,
		 * return 1. Otherwise, return 0.
		 */
		if (fnp->fn_mate)
			return 1;
		else
			return 0;
	else
		/*
		 * For non-pipe FIFO, return number of writers.
		 */
		return (fnp->fn_wcnt);
}
/* End XENIX Support */
