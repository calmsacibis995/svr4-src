/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:predki.c	1.3.1.2"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/conf.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/debug.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/var.h"
#include "sys/vnode.h"
#include "sys/cmn_err.h"
#include "vm/page.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/ddi.h"
#include "sys/immu.h"

extern int basyncnt;

extern void gen_strategy();
extern int gen_read();
extern int gen_write();
extern int gen_ioctl();
extern int gen_bopen();
extern int gen_bclose();
extern int gen_copen();
extern int gen_cclose();

void
fix_swtbls()
{
	register int i;

	for (i = 0; i < bdevcnt; i++) {
		if (*bdevsw[i].d_flag & D_OLD) {
			shadowbsw[i].d_open = bdevsw[i].d_open;
			bdevsw[i].d_open = gen_bopen;
			shadowbsw[i].d_close = bdevsw[i].d_close;
			bdevsw[i].d_close = gen_bclose;
		}
		if (!(*bdevsw[i].d_flag & D_NOBRKUP)) {
			shadowbsw[i].d_strategy = bdevsw[i].d_strategy;
			bdevsw[i].d_strategy = (int(*)())gen_strategy;
		}
	}
	for (i = 0; i < cdevcnt; i++) {
		if (*cdevsw[i].d_flag & D_OLD) {
			shadowcsw[i].d_open = cdevsw[i].d_open;
			cdevsw[i].d_open = gen_copen;
			shadowcsw[i].d_close = cdevsw[i].d_close;
			cdevsw[i].d_close = gen_cclose;
			shadowcsw[i].d_read = cdevsw[i].d_read;
			cdevsw[i].d_read = gen_read;
			shadowcsw[i].d_write = cdevsw[i].d_write;
			cdevsw[i].d_write = gen_write;
			shadowcsw[i].d_ioctl = cdevsw[i].d_ioctl;
			cdevsw[i].d_ioctl = gen_ioctl;
		}
	}
}

void
gen_setup_idinfo(cr)
register struct cred *cr;
{
	u.u_uid = cr->cr_uid;
	u.u_ruid = cr->cr_ruid;
	u.u_gid = cr->cr_gid;
	u.u_rgid = cr->cr_rgid;
}

/* ARGSUSED */
int 
gen_copen(devp, flag, type, cr)
	dev_t *devp;
	int flag;
	int type;
	struct cred *cr;
{
	gen_setup_idinfo(cr);
	(void)(*shadowcsw[getmajor(*devp)].d_open)(cmpdev(*devp), flag, type);
	return u.u_error;
}

/* ARGSUSED */
int 
gen_bopen(devp, flag, type, cr)
	dev_t *devp;
	int flag;
	int type;
	struct cred *cr;
{
	gen_setup_idinfo(cr);
	(void)(*shadowbsw[getmajor(*devp)].d_open)(cmpdev(*devp), flag, type);
	return u.u_error;
}
/* ARGSUSED */
int 
gen_cclose(dev, flag, type, cr)
	dev_t dev;
	int flag;
	int type;
	struct cred *cr;
{
	gen_setup_idinfo(cr);
	(void)(*shadowcsw[getmajor(dev)].d_close)(cmpdev(dev), flag, type);
	return u.u_error;
}

/* ARGSUSED */
int 
gen_bclose(dev, flag, type, cr)
	dev_t dev;
	int flag;
	int type;
	struct cred *cr;
{
	gen_setup_idinfo(cr);
	(void)(*shadowbsw[getmajor(dev)].d_close)(cmpdev(dev), flag, type);
	return u.u_error;
}

/* ARGSUSED */
int
gen_read(dev, uiop, cr)
	register dev_t dev;
	register struct uio *uiop;
	struct cred *cr;
{
	gen_setup_idinfo(cr);
	u.u_offset = uiop->uio_offset;
	u.u_base = uiop->uio_iov->iov_base;
	u.u_count = uiop->uio_resid;
	u.u_segflg = uiop->uio_segflg;
	u.u_fmode = uiop->uio_fmode;

	(void)(*shadowcsw[getmajor(dev)].d_read)(cmpdev(dev));

	uiop->uio_resid = u.u_count;
	uiop->uio_offset = u.u_offset;
	return u.u_error;
}

/* ARGSUSED */
int
gen_write(dev, uiop, cr)
	register dev_t dev;
	register struct uio *uiop;
	struct cred *cr;
{
	gen_setup_idinfo(cr);
	u.u_offset = uiop->uio_offset;
	u.u_base = uiop->uio_iov->iov_base;
	u.u_count = uiop->uio_resid;
	u.u_segflg = uiop->uio_segflg;
	u.u_fmode = uiop->uio_fmode;

	(void)(*shadowcsw[getmajor(dev)].d_write)(cmpdev(dev));

	uiop->uio_resid = u.u_count;
	uiop->uio_offset = u.u_offset;
	return u.u_error;
}

/* ARGSUSED */
int
gen_ioctl(dev, cmd, arg, mode, cr, rvalp)
	register dev_t dev;
	int cmd;
	int arg;
	int mode;
	struct cred *cr;
	int *rvalp;
{
	gen_setup_idinfo(cr);
	(void)(*shadowcsw[getmajor(dev)].d_ioctl)(cmpdev(dev), cmd, arg, mode);
	*rvalp = u.u_rval1;
	return u.u_error;
}
	
STATIC void
gen_iodone(bp)
	register struct buf *bp;
{
	register struct buf *parentbp;

	if (bp->b_chain == NULL) {
		biodone(bp);
		return;
	}
	bp->b_iodone = NULL;

	parentbp = bp->b_chain;
	if (bp->b_flags & B_ERROR) {
		parentbp->b_flags |= B_ERROR;
		if (bp->b_error)
			parentbp->b_error = bp->b_error;
		else if (bp->b_oerror)
			parentbp->b_error = bp->b_oerror;
	}
	if (bp->b_flags & B_ASYNC) {
		if ((bp->b_flags & B_READ) == 0)
			basyncnt--;
		pageio_done(bp);
		parentbp->b_reqcnt--;
		if (parentbp->b_reqcnt == 0) {
			biodone(parentbp);
			return;
		}
	} else {
		biodone(bp);
		return;
	}
}

#define MAXIOREQ 28  /* this is really arbitrary */

void
gen_strategy(bp) 
	struct buf *bp;
{
	register struct page *pp;
	register int i, flags;
	struct buf *bufp[MAXIOREQ];
	int bytescnt, s, req, err;
	int blkincr;

	gen_setup_idinfo(u.u_cred);
	if (bp->b_bcount <= PAGESIZE) {
		if (bp->b_flags & B_PAGEIO) {
			pp = bp->b_pages;
			/* b_addr is the offset into the 1st page of the list */
			bp->b_un.b_addr += (u_int)pfntokv(page_pptonum(pp));
		}
		(*shadowbsw[getmajor(bp->b_edev)].d_strategy)(bp);
		return;
	}
	blkincr = PAGESIZE / NBPSCTR;
	req = bp->b_bcount / PAGESIZE;
	if (bp->b_bcount % PAGESIZE)
		req++;
	bp->b_reqcnt = req;
	
	pp = bp->b_pages;
	if (pp == NULL) {
		(void)buf_breakup(gen_strategy, bp);
		return;
	}
	i = 0;
	do {
		/*
		 * Assumption: pages in the list were sorted.
		 */
		if (i == (req - 1)) {	/* last I/O */
		    if (bp->b_bcount % PAGESIZE)
		    	bytescnt = bp->b_bcount % PAGESIZE;
		    else 
		    	bytescnt = PAGESIZE;
		} else
		    bytescnt = PAGESIZE;
		flags = (bp->b_flags & B_ASYNC) ? B_ASYNC : 0;
		flags |= (bp->b_flags & B_READ) ? B_READ : B_WRITE;
		bufp[i] = pageio_setup(pp, bytescnt, 0, flags);
		bufp[i]->b_edev = bp->b_vp->v_rdev;
		bufp[i]->b_dev = cmpdev(bufp[i]->b_edev);
		bufp[i]->b_blkno = bp->b_blkno + (blkincr * i);
		bufp[i]->b_chain = bp;
		bufp[i]->b_iodone = (int(*)())gen_iodone;
		bufp[i]->b_flags &= ~B_PAGEIO;
		bufp[i]->b_un.b_addr = (caddr_t)pfntokv(page_pptonum(pp));
		(*shadowbsw[getmajor(bufp[i]->b_edev)].d_strategy)(bufp[i]);
		i++;
		pp = pp->p_next;
	} while (pp != bp->b_pages);

	if (!(bp->b_flags & B_ASYNC)) {
		err = 0;
		for (i = 0; i < req; i++) {
	    		if (err)
				(void) biowait(bufp[i]);
	    		else
				err = biowait(bufp[i]);
	    		pageio_done(bufp[i]);
	    		bp->b_reqcnt--;
		}
		/* arbitrarily picking up one of the errors */
		if (err) {
			bp->b_flags |= B_ERROR;
			bp->b_error = err;
		}
		ASSERT(bp->b_reqcnt == 0);
	    	ASSERT((bp->b_flags & B_DONE) == 0);
	    	s = spl6();
		biodone(bp);
	    	splx(s);
	}	
}
