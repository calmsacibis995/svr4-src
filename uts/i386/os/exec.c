/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:exec.c	1.3.3.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/tss.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/signal.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/fstyp.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/reg.h"
#include "sys/fp.h"
#include "sys/var.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/prsystm.h"
#include "sys/tuneable.h"
#include "sys/tty.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/rf_messg.h"
#include "sys/conf.h"
#include "sys/uio.h"
#include "sys/pathname.h"
#include "sys/disp.h"
#include "sys/fbuf.h"
#include "sys/exec.h"
#include "sys/vm.h"
#include "sys/mman.h"
#include "sys/kmem.h"
#include "sys/seg.h"
#include "sys/x.out.h"
#include "sys/mount.h"

#include "vm/hat.h"
#include "vm/anon.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/page.h"
#include "vm/seg_vn.h"
#include "vm/seg_kmem.h"
#include "vm/seg_map.h"
#include "vm/seg_dummy.h"
#include "sys/inline.h"

#ifdef i386			/* to highlight portability of exec code */
extern struct gdscr def_intf0;
extern struct gate_desc idt[];
extern int	do386b1_x87;	/* 80386 B1 stepping bug workaround enable */
#endif

extern int exec_ncargs;
int exec_initialstk = (ctob(SSIZE));

#ifndef i386
/* this reference should be closer to md use */
extern int	mau_present;
#endif

/*
 * If the PREREAD(size) macro evaluates true, then we will read in
 * the given text or data a.out segment even though the file can be paged.
 */
#define	PREREAD(size) \
	((int)btopr(size) < (int)(freemem - minfree) && size < pgthresh)
STATIC int pgthresh = 0;

int nullmagic = 0;		/* null magic number */

struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

/* ARGSUSED */
int
exece(uap, rvp)
	struct execa *uap;
	rval_t *rvp;
{
	long execsz;		/* temporary count of exec size */
	int error = 0;
	vnode_t *vp;
	char exec_file[PSCOMSIZ];
	struct pathname pn;
	struct uarg args;

	sysinfo.sysexec++;

	/*
	 * Can't do exec if there are any outstanding async I/O operations.
	 */
	if (u.u_procp->p_aiocount)
		return EINVAL;

#ifdef ASYNCIO
	/*
	 * Can't do exec if there is any outstanding raw disk async I/O.
	 */
	if (u.u_procp->p_raiocnt)
		return EINVAL;
#endif /* ASYNC IO */

#ifdef i386
	/*
	 * On the 386 we start off with MINUSIZE (2) pages for the ublock and
	 * expand it on the fly if necessary.
	 */
	execsz = MINUSIZE + SINCR + SSIZE + btoc(exec_ncargs-1);
#else
	execsz = USIZE + SINCR + SSIZE + btoc(exec_ncargs-1);
#endif

	/*
	 * Lookup path name and remember last component for later.
	 */
	if (error = pn_get(uap->fname, UIO_USERSPACE, &pn))
		return error;
	if (error = lookuppn(&pn, FOLLOW, NULLVPP, &vp)) {
		pn_free(&pn);
		return error;
	}
	strncpy(exec_file, pn.pn_path, PSCOMSIZ);
	struct_zero(&args, sizeof(args));

	if (uap->argp) {
		switch (arglistsz(uap->argp, &args.argc, &args.argsize,
		  exec_ncargs)) {
		case -2:
			error = E2BIG;
			goto done;
		case -1:
			error = EFAULT;
			goto done;
		default:
			args.argp = uap->argp;
			break;
		}
	}

	if (uap->envp) {
		switch (arglistsz(uap->envp, &args.envc, &args.envsize,
		    exec_ncargs - args.argsize)) {
		case -2:
			error = E2BIG;
			goto done;
		case -1:
			error = EFAULT;
			goto done;
		default:
			args.envp = uap->envp;
			break;
		}
	}

	args.fname = pn.pn_buf;


	PREEMPT();
	if (error = gexec(&vp, &args, 0, &execsz))
		goto done;
	PREEMPT();

	u.u_execsz = execsz;	/* dependent portion should have checked */

	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag &= ~AFORK;
	bcopy((caddr_t)exec_file, (caddr_t)u.u_comm, PSCOMSIZ);

	if ((error = setregs(&args)) != 0)
		psignal(u.u_procp, SIGKILL);

	/* XENIX Support */
 	/*
	 * return the original break address.  Used by XENIX
  	 * in start0 to set __nd_ for beginning break value.
 	 */
 	if (isXOUT)
 	{
 		rvp->r_val1 = (long) u.u_exdata.ux_datorg + u.u_exdata.ux_dsize +
 				u.u_exdata.ux_bsize;
 		rvp->r_val2 = USER_DS;
 	}
	/* End XENIX Support */

#ifdef i386
	/* 
	 * Let's not inherit exec'er's fp context
	 */
	if (fp_proc == u.u_procp) {
		fp_proc = NULL;
		setts();
	}
	u.u_fpvalid = 0;
	u.u_fps.u_fpstate.status = 0;

	/* if call gate was set, clear it */
	if (u.u_callgatep) {
		u.u_callgatep = 0;
		u.u_callgate[0]= 0;
		u.u_callgate[1]= 0;
	}

	/* XENIX Support */
	/* keep ublock at a modest size */
	if (u.u_procp->p_usize > MINUSIZE) {
		segu_shrink(MINLDTSZ);
	}
	u.u_ldtlimit = MINLDTSZ;
	setdscrlim(&u.u_ldt_desc, (MINLDTSZ + 1) * sizeof(struct dscr) - 1);
	/* End XENIX Support */

	/* if LDT was modified, clear it above USER_FP */
	if (u.u_ldtmodified) {
		u.u_ldtmodified = 0;
		bzero((caddr_t)((struct dscr *)(u.u_procp->p_ldt) + (seltoi(USER_FP)+1)),
		      (MINLDTSZ - seltoi(USER_FP)-1) * sizeof(struct dscr));
	}
	/* XENIX Support */
	/* if IDT was modified, clear it above 0xf0 */
	if (((struct gdscr *) &idt[0xf0])->gd_acc0007!=(GATE_KACC|GATE_386TRP)) {

		register int i;

		*(struct gdscr *) u.u_fpintgate = def_intf0;
		for (i = 0xf0; i <= 0xff; i++) {
			*(struct gdscr *) &idt[i] = def_intf0;
		}
	}
	ASSERT(u.u_ldtlimit == MINLDTSZ);
	ASSERT(u.u_procp->p_usize == MINUSIZE);
	/* End XENIX Support */

#endif	/* i386 */

done:
	PREEMPT();
	pn_free(&pn);
	VN_RELE(vp);
	return error;
}

/*
 * exec system calls, without and with environments.
 */
int
exec(uap, rvp)
	struct execa *uap;
	rval_t *rvp;
{
	uap->envp = NULL;
	return exece(uap, rvp);
}

exhdmap_t *exhd_freelist;
int exhd_freeincr = 8;

STATIC int
exhd_getfbuf(ehdap, off, size, keep, mappp)
	exhda_t *ehdap;
	off_t off;
	int size;
	int keep;
	exhdmap_t **mappp;
{
	register exhdmap_t *mapp;
	extern struct as kas;
	off_t boff, eoff, bpoff, epoff;
	size_t len;
	register struct fbuf *fbp;
	faultcode_t err;
	int error;

	boff = off & MAXBMASK;
	bpoff = off & PAGEMASK;
	eoff = off + size;
	epoff = boff + MAXBSIZE;
	if (epoff > eoff)
		epoff = (eoff + PAGESIZE-1) & PAGEMASK;
	for (mapp = ehdap->maplist; mapp != NULL; mapp = mapp->nextmap) {
		fbp = mapp->fbufp;
		if (fbp == NULL)
			continue;
		if (mapp->curbase == boff) {
			if (bpoff < mapp->curoff) {
				len = (size_t)(mapp->curoff - bpoff);
				err = as_fault(&kas,
						fbp->fb_addr - len,
						len,
						F_SOFTLOCK,
						S_READ);
				if (err) {
					if (FC_CODE(err) == FC_OBJERR)
						return FC_ERRNO(err);
					else
						return EIO;
				}
				fbp->fb_addr -= len;
				fbp->fb_count += len;
				mapp->curoff -= len;
			}
			if (epoff > mapp->cureoff) {
				len = epoff - mapp->cureoff;
				err = as_fault(&kas,
						fbp->fb_addr + fbp->fb_count,
						len,
						F_SOFTLOCK,
						S_READ);
				if (err) {
					if (FC_CODE(err) == FC_OBJERR)
						return FC_ERRNO(err);
					else
						return EIO;
				}
				fbp->fb_count += len;
				mapp->cureoff = epoff;
			}
			*mappp = mapp;
			return 0;
		}
	}

	/* need a new fbuf */
	mapp = (exhdmap_t *) kmem_fast_alloc(
		(caddr_t *) &exhd_freelist,
		sizeof(*exhd_freelist),
		exhd_freeincr,
		KM_SLEEP);
	struct_zero(mapp, sizeof(*mapp));
	error = fbread(ehdap->vp, bpoff, epoff - bpoff, S_READ, &mapp->fbufp);
	if (error) {
		kmem_fast_free((caddr_t *) &exhd_freelist, (caddr_t)mapp);
		return error;
	}
	mapp->nextmap = ehdap->maplist;
	ehdap->maplist = mapp;
	mapp->curbase = boff;
	mapp->curoff = bpoff;
	mapp->cureoff = epoff;
	*mappp = mapp;
	return 0;
}

STATIC int
exhd_nomap(ehdap, off, size, flags, cpp)
	exhda_t *ehdap;
	off_t off;
	int size;
	int flags;
	caddr_t cpp;
{
	register exhdmap_t *mapp;
	register exhdmap_t **mpp;
	int error;
	int resid;
	off_t poff, eoff, epoff;
	register long cnt;
	exhdmap_t *copymapp = NULL;
	int icnt = 0;
	caddr_t tcp;
	vnode_t *vp = ehdap->vp;

	eoff = off + size;
	poff = off & PAGEMASK;
	epoff = (eoff + (PAGESIZE-1)) & PAGEMASK;

	/*
	 * The code rejects doing the autofree for VNOMAP files
	 * to avoid losing this hidden cache.
	 * So, only non-vnode pages are autofreed.
	 */
	for (mpp = &ehdap->maplist; (mapp = *mpp) != NULL; ) {
		if (mapp->keepcnt || mapp->cureoff) {
			mpp = &mapp->nextmap;
			continue;
		}
		if (mapp->bndrycasep)
			kmem_free(mapp->bndrycasep, mapp->bndrycasesz);
		*mpp = mapp->nextmap;
		kmem_fast_free((caddr_t *) &exhd_freelist, (caddr_t)mapp);
	}
	if (!(flags & EXHD_COPY) && (flags & EXHD_4BALIGN) && (off & 3)) {
		mapp = (exhdmap_t *) kmem_fast_alloc(
			(caddr_t *) &exhd_freelist,
			sizeof(*exhd_freelist),
			exhd_freeincr,
			KM_SLEEP);
		struct_zero(mapp, sizeof(*mapp));
		mapp->bndrycasep = kmem_alloc(size, KM_SLEEP);
		mapp->bndrycasesz = size;
		if (flags & EXHD_KEEPMAP)
			mapp->keepcnt++;
		mapp->nextmap = ehdap->maplist;
		ehdap->maplist = mapp;
		copymapp = mapp;
	}

	/*
	 * Keep the maplist sorted by offset and allow no overlaps.
	 */
	for (mpp = &ehdap->maplist; (mapp = *mpp) != NULL; ) {
		if (mapp->cureoff == 0) {
			mpp = &mapp->nextmap;
			continue;
		}
		if (mapp->curoff >= epoff)
			break;
		if (mapp->cureoff > poff && mapp->curoff < epoff) {
			if (mapp->curoff <= poff && mapp->cureoff >= epoff) {
				/* the simple case: it has it all */
				if (!(flags & EXHD_COPY)) {
					if (copymapp != NULL) {
						*((caddr_t *)cpp) =
							copymapp->bndrycasep;
						bcopy(mapp->bndrycasep
							  + off - mapp->curoff,
							copymapp->bndrycasep,
							size);
						return 0;
					}
					*((caddr_t *)cpp) = mapp->bndrycasep
						+ (off - mapp->curoff);
					if (flags & EXHD_KEEPMAP)
						mapp->keepcnt++;
					return 0;
				}
				else {
					bcopy(mapp->bndrycasep
						+ (off-mapp->curoff),
						cpp, size);
					return 0;
				}
			}
			icnt++;
			break;
		}
		mpp = &mapp->nextmap;
	}
	if (icnt == 0) {
		mapp = (exhdmap_t *) kmem_fast_alloc(
			(caddr_t *) &exhd_freelist,
			sizeof(*exhd_freelist),
			exhd_freeincr,
			KM_SLEEP);
		struct_zero(mapp, sizeof(*mapp));
		cnt = epoff - poff;
		mapp->bndrycasep = kmem_alloc(cnt, KM_SLEEP);
		mapp->bndrycasesz = cnt;
		mapp->curoff = poff;
		mapp->cureoff = epoff;
		error = vn_rdwr(UIO_READ, vp, mapp->bndrycasep, cnt, poff,
			UIO_SYSSPACE, 0, (long) 0, u.u_cred, &resid);
		if (error || (resid && resid + ehdap->vnsize != epoff)) {
			kmem_fast_free((caddr_t *) &exhd_freelist,
			  (caddr_t)mapp);
			ehdap->state = EXHDA_HADERROR;
			if (error)
				return error;
			return ENOEXEC;
		}
		mapp->nextmap = *mpp;
		*mpp = mapp;
		if (!(flags & EXHD_COPY)) {
			if (copymapp != NULL) {
				*((caddr_t *)cpp) =
					copymapp->bndrycasep;
				bcopy(mapp->bndrycasep + off - mapp->curoff,
					copymapp->bndrycasep, size);
				return 0;
			}
			*((caddr_t *)cpp) = mapp->bndrycasep
				+ (off - mapp->curoff);
			if (flags & EXHD_KEEPMAP)
				mapp->keepcnt++;
			return 0;
		}
		else {
			bcopy(mapp->bndrycasep + (off-mapp->curoff),
				cpp, size);
			return 0;
		}
	}
	/*
	 * A partial overlap:
	 * copy to a separate buffer
	 * rather than fiddling with merging vnode pages.
	 */
	if (!(flags & EXHD_COPY)) {
		if (copymapp)
			tcp = copymapp->bndrycasep;
		else {
			mapp = (exhdmap_t *) kmem_fast_alloc(
				(caddr_t *) &exhd_freelist,
				sizeof(*exhd_freelist),
				exhd_freeincr,
				KM_SLEEP);
			struct_zero(mapp, sizeof(*mapp));
			mapp->bndrycasep = kmem_alloc(size, KM_SLEEP);
			mapp->bndrycasesz = size;
			if (flags & EXHD_KEEPMAP)
				mapp->keepcnt++;
			mapp->nextmap = *mpp;
			*mpp = mapp;
			tcp = mapp->bndrycasep;
			mpp = &mapp->nextmap;
			mapp = mapp->nextmap;
		}
		*((caddr_t *)cpp) = tcp;
	} else
		tcp = cpp;
	ASSERT(mapp != NULL);
	while ((mapp = *mpp) != NULL) {
		if (mapp->cureoff == 0) {
			mpp = &mapp->nextmap;
			continue;
		}
		if (mapp->curoff > off) {
			poff = off & PAGEMASK;
			cnt = mapp->curoff - poff;
			if (cnt > size)
				cnt = size;
			cnt = (cnt + (PAGESIZE-1)) & PAGEMASK;
			mapp = (exhdmap_t *) kmem_fast_alloc(
				(caddr_t *) &exhd_freelist,
				sizeof(*exhd_freelist),
				exhd_freeincr,
				KM_SLEEP);
			struct_zero(mapp, sizeof(*mapp));
			mapp->bndrycasep = kmem_alloc(cnt, KM_SLEEP);
			mapp->bndrycasesz = cnt;
			mapp->nextmap = *mpp;
			*mpp = mapp;
			error = vn_rdwr(UIO_READ, vp, mapp->bndrycasep,
				cnt, poff,
				UIO_SYSSPACE, 0, (long) 0, u.u_cred, &resid);
			if (error || resid) {
				ehdap->state = EXHDA_HADERROR;
				if (error)
					return error;
				return ENOEXEC;
			}
			mapp->curoff = poff;
			mapp->cureoff = poff + cnt;
		}
		ASSERT(off >= mapp->curoff && off < mapp->cureoff);
		cnt = mapp->cureoff - off;
		if (cnt > size)
			cnt = size;
		bcopy(mapp->bndrycasep + off - mapp->curoff, tcp, cnt);
		if ((size -= cnt) <= 0)
			return 0;
		off += cnt;
		tcp += cnt;
		mpp = &mapp->nextmap;
	}
	cnt = size;
	cnt = (cnt + (PAGESIZE-1)) & PAGEMASK;
	mapp = (exhdmap_t *) kmem_fast_alloc(
		(caddr_t *) &exhd_freelist,
		sizeof(*exhd_freelist),
		exhd_freeincr,
		KM_SLEEP);
	struct_zero(mapp, sizeof(*mapp));
	mapp->bndrycasep = kmem_alloc(cnt, KM_SLEEP);
	mapp->bndrycasesz = cnt;
	mapp->nextmap = *mpp;
	*mpp = mapp;
	error = vn_rdwr(UIO_READ, vp, mapp->bndrycasep, cnt, off,
		UIO_SYSSPACE, 0, (long) 0, u.u_cred, &resid);
	if (error || resid) {
		ehdap->state = EXHDA_HADERROR;
		if (error)
			return error;
		return ENOEXEC;
	}
	mapp->curoff = off;
	mapp->cureoff = off + cnt;
	bcopy(mapp->bndrycasep, tcp, size);
	return 0;
}

int
exhd_getmap(ehdap, off, size, flags, cpp)
	exhda_t *ehdap;
	off_t off;
	int size;
	int flags;
	caddr_t cpp;
{
	register exhdmap_t *mapp;
	register exhdmap_t **mpp;
	int error;
	off_t boff, eoff, eboff;
	register long cnt;
	char *fcp, *tcp;
	exhdmap_t *curmapp;

	ASSERT(size > 0);
	if (ehdap->state == EXHDA_HADERROR)
		return ENOEXEC;	/* we failed previously */
	eoff = off + size;
	if (eoff < off || eoff > ehdap->vnsize) {
		ehdap->state = EXHDA_HADERROR;
		return ENOEXEC;
	}

	/*
	 * Assumption: the mappability of a vnode is constant during exec.
	 */
/*
	if (ehdap->vp->v_flag & VNOMAP)
*/
	if (ehdap->nomap)
		return exhd_nomap(ehdap, off, size, flags, cpp);
	boff = off & MAXBMASK;
	eboff = (eoff-1) & MAXBMASK;
	for (mpp = &ehdap->maplist; (mapp = *mpp) != NULL; ) {
		if (mapp->keepcnt
		  || (mapp->cureoff && mapp->curbase >= boff
		      && mapp->curbase <= eboff)) {
			mpp = &mapp->nextmap;
			continue;
		}
		if (mapp->bndrycasep)
			kmem_free(mapp->bndrycasep, mapp->bndrycasesz);
		if (mapp->fbufp)
			fbrelse(mapp->fbufp, S_READ);
		*mpp = mapp->nextmap;
		kmem_fast_free((caddr_t *) &exhd_freelist, (caddr_t)mapp);
	}
	if (!(flags & EXHD_COPY || boff != eboff
	    || !((flags & EXHD_4BALIGN) == 0 || (off & 3) == 0))) {
		/*
		 * The simple case of returning a pointer to seg_map space
		 */
		error = exhd_getfbuf(ehdap, off, size, flags & EXHD_KEEPMAP,
			&curmapp);
		if (error) {
			ehdap->state = EXHDA_HADERROR;
			return error;
		}
		mapp = curmapp;
		*((caddr_t *)cpp) = mapp->fbufp->fb_addr + (off - mapp->curoff);
		return 0;
	}

	if (flags & EXHD_COPY)
		tcp = (caddr_t) cpp;
	else {
		tcp = kmem_alloc(size, KM_SLEEP);
		mapp = (exhdmap_t *) kmem_fast_alloc(
			(caddr_t *) &exhd_freelist,
			sizeof(*exhd_freelist),
			exhd_freeincr,
			KM_SLEEP);
		struct_zero(mapp, sizeof(*mapp));
		mapp->nextmap = ehdap->maplist;
		ehdap->maplist = mapp;
		mapp->bndrycasep = tcp;
		mapp->bndrycasesz = size;
		if (flags & EXHD_KEEPMAP)
			mapp->keepcnt = 1;
		*((caddr_t *) cpp) = tcp;
	}
	error = exhd_getfbuf(ehdap, off, size, flags & EXHD_KEEPMAP,
		&curmapp);
	if (error) {
		ehdap->state = EXHDA_HADERROR;
		return error;
	}
	mapp = curmapp;
	fcp = mapp->fbufp->fb_addr + (off - mapp->curoff);
	eoff = mapp->cureoff;
	cnt = eoff - off;
	if (cnt > size)
		cnt = size;
	for (;;) {
		bcopy(fcp, tcp, cnt);
		if ((size -= cnt) <= 0)
			return 0;
		tcp += cnt;
		off += cnt;
		error = exhd_getfbuf(ehdap, off, size, flags & EXHD_KEEPMAP,
			&curmapp);
		if (error) {
			ehdap->state = EXHDA_HADERROR;
			return error;
		}
		mapp = curmapp;
		fcp = mapp->fbufp->fb_addr + (off - mapp->curoff);
		eoff = mapp->cureoff;
		cnt = eoff - off;
		if (cnt > size)
			cnt = size;
	}
}

void
exhd_release(hdp)
	register exhda_t *hdp;
{
	register exhdmap_t *mapp, *nmapp;

	if (hdp == NULL)
		return;
	for (mapp = hdp->maplist; mapp != NULL; mapp = nmapp) {
		if (mapp->bndrycasep)
			kmem_free(mapp->bndrycasep, mapp->bndrycasesz);
		if (mapp->fbufp)
			fbrelse(mapp->fbufp, S_READ);
		nmapp = mapp->nextmap;
		kmem_fast_free((caddr_t *) &exhd_freelist, (caddr_t)mapp);
	}
}

int
execpermissions(vp, vattrp, ehdp, args)
	struct vnode *vp;
	struct vattr *vattrp;
	exhda_t *ehdp;
	struct uarg *args;
{
	int error;
	register proc_t *p = u.u_procp;

	struct_zero(ehdp, sizeof(*ehdp));
	vattrp->va_mask = AT_MODE|AT_UID|AT_GID|AT_SIZE;
	if (error = VOP_GETATTR(vp, vattrp, ATTR_EXEC, p->p_cred))
		return error;
	/*
	 * Check the access mode.
	 */
	if ((error = VOP_ACCESS(vp, VEXEC, 0, u.u_cred)) != 0
	  || vp->v_type != VREG
	  || (vattrp->va_mode & (VEXEC|(VEXEC>>3)|(VEXEC>>6))) == 0) {
		if (error == 0)
			error = EACCES;
		return error;
	}

       if ((p->p_trace || (p->p_flag & (STRC|SPROCTR)))
          && (error = VOP_ACCESS(vp, VREAD, 0, u.u_cred))) {
		/*
                 * If process is traced via ptrace(2), fail the exec(2).
                 */
                if (p->p_flag & STRC)
                        goto bad;
		/*
                 * Process is traced via /proc.
                 * Arrange to invalidate the /proc vnode.
                 */
                args->traceinval = 1;
	}

	ehdp->vp = vp;
	ehdp->vnsize = vattrp->va_size;
	ehdp->nomap = vp->v_flag & VNOMAP;
	return 0;
bad:
	if (error == 0)
		error = ENOEXEC;
	return error;
}

STATIC int
execsetid(vp, vattrp, uidp, gidp)
	struct vnode *vp;
	struct vattr *vattrp;
	uid_t *uidp;
	uid_t *gidp;
{
	proc_t *pp = u.u_procp;
	uid_t uid, gid;

	/*
	 * Remember credentials.
	 */
	uid = pp->p_cred->cr_uid;
	gid = pp->p_cred->cr_gid;

	if ((vp->v_vfsp->vfs_flag & VFS_NOSUID) == 0) {
		if (vattrp->va_mode & VSUID)
			uid = vattrp->va_uid;
		if (vattrp->va_mode & VSGID)
			gid = vattrp->va_gid;
	}

	/*
 	 * Set setuid/setgid protections, if no tracing.  
	 * For the super-user, honor setuid/setgid even in 
	 * the presence of tracing.
 	 */
	if (((pp->p_flag & STRC) == 0 || pp->p_cred->cr_uid == 0)
  	  && (pp->p_cred->cr_uid != uid
	    || pp->p_cred->cr_gid != gid
	    || pp->p_cred->cr_suid != uid
	    || pp->p_cred->cr_sgid != gid)) {
		*uidp = uid;
		*gidp = gid;
		return 1;
	} 
	return 0;
}

int
gexec(vpp, args, level, execsz)
	struct vnode **vpp;
	struct uarg *args;
	int level;
	long *execsz;
{
	register proc_t *pp = u.u_procp;
	register int i;
	register vnode_t *vp;
	int error, closerr = 0;
	int resid;
	uid_t uid, gid;
	struct vattr vattr;
	short magic;
	char *mcp;
	exhda_t ehda;
	int setid;
	struct execsw *save_execsw = u.u_execsw;

	vp = *vpp;
	if ((error = execpermissions(vp, &vattr, &ehda, args)) != 0)
		goto out;

	if ((error = VOP_OPEN(vpp, FREAD, u.u_cred) != 0))
		goto out;

	vp = *vpp;
	if ((error = exhd_getmap(&ehda, 0, 2, EXHD_NOALIGN, (caddr_t)&mcp))
	  != 0) {
		exhd_release(&ehda);
		goto closevp;
	}
	magic = getexmag(mcp);

	setid = execsetid(vp, &vattr, &uid, &gid);

	error = ENOEXEC;
	for (i = 0; i < nexectype; i++) {
		if (execsw[i].exec_magic && magic != *execsw[i].exec_magic)
			continue;
		u.u_execsw = &execsw[i];
		error = (*execsw[i].exec_func)
				  (vp, args, level, execsz, &ehda, setid);
		if (error != ENOEXEC)
			break;
	}

	exhd_release(&ehda);

#ifdef i386
	if (error == -1) {	/* special case for the emulators */
		error = 0;
		goto closevp;
	}
#endif

	if (error) {
		u.u_execsw = save_execsw;
		goto closevp;
	}

	if (level == 0) {
		if (setid) {
			/*
			 * Prevent unprivileged processes from enforcing
			 * resource limitations on setuid/setgid processes
			 * by reinitializing them to system defaults.
			 */
			for (i = 0; i < RLIM_NLIMITS; i++) {
				if (u.u_rlimit[i].rlim_cur < rlimits[i].rlim_cur)
					u.u_rlimit[i].rlim_cur = rlimits[i].rlim_cur;
				if (u.u_rlimit[i].rlim_max < rlimits[i].rlim_max)
					u.u_rlimit[i].rlim_max = rlimits[i].rlim_max;
			}

			pp->p_cred = crcopy(pp->p_cred);
			pp->p_cred->cr_uid = uid;
			pp->p_cred->cr_gid = gid;
			pp->p_cred->cr_suid = uid;
			pp->p_cred->cr_sgid = gid;
			if (uid < USHRT_MAX)
				u.u_uid = (o_uid_t) uid;
			else
				u.u_uid = (o_uid_t) UID_NOBODY;

			if (gid < USHRT_MAX)
				u.u_gid = (o_gid_t) gid;
			else
				u.u_gid = (o_gid_t) UID_NOBODY;

			/*
			 * If process is traced via /proc, arrange to
			 * invalidate the associated /proc vnode.
			 */
			if (pp->p_trace || (pp->p_flag & SPROCTR))
				args->traceinval = 1;
		}

		if (pp->p_flag & STRC)
			psignal(pp, SIGTRAP);

		if (args->traceinval)
			prinvalidate(&u);
	}

closevp:
	closerr = VOP_CLOSE(vp, FREAD, 1, 0, u.u_cred);
out:
	return error ? error : closerr;
}

int
execmap(vp, addr, len, zfodlen, offset, prot)
	struct vnode *vp;
	caddr_t addr; 
	size_t len, zfodlen;
	off_t  offset;
	int prot;
{
	int error = 0;
	int page = 0;
	caddr_t zfodbase, oldaddr;
	size_t zfoddiff, end, oldlen;
	proc_t *p = u.u_procp;
	off_t oldoffset;

	if (((long)offset & PAGEOFFSET) == ((long)addr & PAGEOFFSET)
	  && (!(vp->v_flag & VNOMAP)))
			page = 1;
		
#ifdef i386
	/* On the 386, write privileges imply read privileges */
	if (prot & PROT_WRITE)
		prot |= PROT_READ;
#endif /* i386 */

	oldaddr = addr;
	addr = (caddr_t)((long)addr & PAGEMASK);
	if (len) {
		oldlen = len;
		len += ((size_t)oldaddr - (size_t)addr);
		oldoffset = offset;
		offset = (off_t)((long)offset & PAGEMASK);
		if (page) {
			if (error = VOP_MAP(vp, offset, p->p_as, &addr,
				len, prot, PROT_ALL,
				 MAP_PRIVATE | MAP_FIXED, p->p_cred))
					goto bad;
			/*
			 * If the segment can fit, then we prefault
			 * the entire segment in.  This is based on the
			 * model that says the best working set of a
			 * small program is all of its pages.
			 */
			if (PREREAD(len)) {
				(void) as_fault(p->p_as, (caddr_t)addr,
				  	   	 len, F_INVAL, S_READ);
			}
		} else {	
			if (error = as_map(p->p_as, addr, len,
			  segvn_create, zfod_argsp))
				goto bad;
			/*
			 * Read in the segment in one big chunk.
			 */
			if (error = vn_rdwr(UIO_READ, vp, (caddr_t)oldaddr,
			  oldlen, oldoffset, UIO_USERSPACE, 0,
			  (u_long) 0, p->p_cred, (int *)0))
				goto bad;
			/*
			 * Now set protections.
			 */
			(void)as_setprot(p->p_as, (caddr_t)addr, len, prot);
		}
	}

	if (zfodlen) {
		end = (size_t)addr + len;
		zfodbase = (caddr_t)roundup(end, PAGESIZE);
		zfoddiff = (size_t)zfodbase - end;
		if (zfoddiff != 0) {
#ifdef i386
			/*
			 *	386 kernel mode cannot handle protection
			 *	fault(s).
			 */
			(void) as_fault(p->p_as,
				(caddr_t) addr + len - PAGOFF(addr+len),
				NBPP, F_PROT, S_WRITE);
#endif
			if ((error = uzero((caddr_t)end, zfoddiff)) != 0)
				goto bad;
		}
		if (zfodlen > zfoddiff) {
			zfodlen -= zfoddiff;
			if (error = as_map(p->p_as, (caddr_t)zfodbase, zfodlen,
			  segvn_create, zfod_argsp))
				goto bad;

			(void)as_setprot(p->p_as, (caddr_t)zfodbase, 
			  zfodlen, prot);
		}
	}

	return 0;
bad:
	return error;
}

/*
 * Machine-independent final setup code goes in setexecenv().
 */
void
setexecenv(ep)
	struct execenv *ep;
{
	register int	i;
	register struct proc *p = u.u_procp;
	file_t *fp;

	u.u_execid = (int)ep->ex_magic;
	p->p_brkbase = ep->ex_brkbase;
	p->p_brksize = 0;
	if (p->p_exec)
		VN_RELE(p->p_exec);	/* out with the old */
	p->p_exec = ep->ex_vp;
	if (p->p_exec)
		VN_HOLD(p->p_exec);	/* in with the new */

	u.u_oldcontext = 0;

	u.u_sigresethand = 0;
	u.u_signodefer = 0;
	u.u_sigonstack = 0;

	u.u_sigaltstack.ss_sp = 0;
	u.u_sigaltstack.ss_size = 0;
	u.u_sigaltstack.ss_flags = SS_DISABLE;

	/*
	 * Any pending signals remain held, so don't clear p_hold and
	 * p_sig.
	 */	

	/*
	 * If the action was to catch the signal, then the action
	 * must be reset to SIG_DFL.
	 */
	for (i = 1; i < NSIG; i++) {
		if (u.u_signal[i-1] != SIG_DFL && u.u_signal[i-1] != SIG_IGN) {
			ev_signal(p, i);
			u.u_signal[i - 1] = SIG_DFL;
			sigemptyset(&u.u_sigmask[i - 1]);
			if (sigismember(&ignoredefault, i))
				sigdelq(p, i);
		}
	}

	sigorset(&p->p_ignore, &ignoredefault);
	sigdiffset(&p->p_siginfo, &ignoredefault);
	sigdiffset(&p->p_sig, &ignoredefault);

	p->p_flag &= ~(SNOWAIT|SJCTL);
	p->p_flag |= SEXECED;

#ifndef i386
	/*
	 * Clear illegal opcode handler.
	 */
	u.u_iop = NULL;
#endif

	for (i = 0; i < u.u_nofiles; i++) {
		if (getf(i, &fp) == 0 && (getpof(i) & FCLOSEXEC)) {
			closef(fp);
			setf(i, NULLFP);
		}
	}
}

int
remove_proc(args)
	struct uarg	*args;
{
	extern void shmexec();
	extern void punlock();
	register struct proc *p;	/* process exiting or exec'ing */
	struct as *nas;
	int error;
#ifdef i386
	struct dscr	*dscrp;
#endif
#ifdef VPIX
	extern char	v86procflag;
#endif

	p = u.u_procp;
#ifdef	ASYNCIO
	if (u.u_raioaddr) {
		(void) as_fault(p->p_as, u.u_raioaddr, u.u_raiosize, F_SOFTUNLOCK, S_WRITE);
		u.u_raioaddr = 0;
		u.u_raiosize = 0;
	}
#endif /* ASYNC IO */
	punlock();
	
	u.u_prof.pr_scale = 0;

#ifdef VPIX
	if (v86procflag)
		v86exit(p);
#endif

	if (error = extractarg(args))
		return error;
	ev_exec(p);

	if (u.u_nshmseg)
		shmexec(p);
	/* XENIX Support */
	if (p->p_sdp)
		xsdexit();
	/* End XENIX Support */

#ifdef i386
	u.u_nshmseg = 0;
#else
	u.u_nshmseg = u.u_dmm = 0;
#endif

	nas = as_alloc();
	as_exec(p->p_as, args->estkstart, args->estksize,
		nas, args->stacklow - args->estksize, args->estkhflag);
	relvm(p);

	p->p_as = nas;
	hat_asload();

#ifdef i386
	/* 80386 B1 stepping Errata #10 */
	if (do386b1_x87)
		(void)as_map(nas, 0x80000000, ctob(1), segdummy_create, NULL);

	dscrp = (struct dscr *)(u.u_procp->p_ldt)+seltoi(USER_CS);
	setdscrbase(dscrp, 0);
	setdscrlim(dscrp, btoct(MAXUVADR-1));
	setdscracc1(dscrp, UTEXT_ACC1);
	setdscracc2(dscrp, TEXT_ACC2);		/* page gran., 32 bit */
#endif
	return 0;
}

int
execopen(vpp, fdp)
	struct vnode **vpp;
	int *fdp;
{
	struct vnode *vp = *vpp;
	struct cred *credp;
	file_t *fp;
	int error = 0;
	int filemode = FREAD;

	VN_HOLD(vp);		/* open reference */
	if (error = falloc((struct vnode *)NULL, filemode, &fp, fdp)) {
		VN_RELE(vp);
		*fdp = -1;	/* just in case falloc changed value */
		return error;
	}
	credp = crdup(u.u_cred);
	credp->cr_uid = 0; 	/* make sure we can open file */
	credp->cr_gid = 0;
	if (error = VOP_OPEN(&vp, filemode, credp)){
		VN_RELE(vp);
		setf(*fdp, NULLFP);
		unfalloc(fp);
		*fdp = -1;		
		return error;
	}
	(void) crfree(credp);
	*vpp = vp;		/* vnode should not have changed */
	fp->f_vnode = vp;
	return 0;
}

int
execclose(fd)
	int fd;
{
	int error;
	file_t *fp;

	if (error = getf(fd, &fp))
		return error;
	setf(fd, NULLFP);
	return closef(fp);
}


/* ARGSUSED */
int
noexec(vp, args, level, ehdp, setid)
	struct vnode *vp;
	struct uarg *args;
	int level;
	exhda_t *ehdp;
	int setid;
{
	cmn_err(CE_WARN, "missing exec capability for %s\n", args->fname);
	return ENOEXEC;
}


#ifdef i386
int
setemulate(emul, vp, args, execsz)
	char		*emul;
	struct vnode	*vp;
	struct uarg	*args;
	long		*execsz;
{
	int		error;
	register ushort	uid;
	register ushort	gid;
	struct vattr	vattr;
	register int	i;

	vattr.va_mask = AT_UID | AT_GID | AT_SIZE;
	if ((error = VOP_GETATTR(vp, &vattr, ATTR_EXEC, u.u_cred)) != 0)
		return error;

	uid = u.u_cred->cr_uid;
	gid = u.u_cred->cr_gid;

	if ((vp->v_vfsp->vfs_flag & VFS_NOSUID) == 0) {
		if (vattr.va_mode & VSUID)
			uid = vattr.va_uid;
		if (vattr.va_mode & VSGID)
			gid = vattr.va_gid;
	}

	if (((u.u_procp->p_flag & STRC) == 0 || u.u_cred->cr_uid == 0)
		&& (u.u_cred->cr_uid != uid || u.u_cred->cr_gid != gid
			|| u.u_cred->cr_suid != uid || u.u_cred->cr_sgid != gid)) {

			for (i = 0; i < RLIM_NLIMITS; i++) {
				u.u_rlimit[i].rlim_cur = rlimits[i].rlim_cur;
				u.u_rlimit[i].rlim_max = rlimits[i].rlim_max;
			}
			u.u_cred = crcopy(u.u_cred);
			u.u_cred->cr_uid = uid;
			u.u_cred->cr_gid = gid;
			u.u_cred->cr_suid = uid;
			u.u_cred->cr_sgid = gid;

			if (uid < USHRT_MAX)
				u.u_uid = (o_uid_t) uid;
			else
				u.u_uid = (o_uid_t) UID_NOBODY;

			if (gid < USHRT_MAX)
				u.u_gid = (o_gid_t) gid;
			else
				u.u_gid = (o_gid_t) UID_NOBODY;

			if (u.u_procp->p_trace ||
				(u.u_procp->p_flag & SPROCTR))
				args->traceinval = 1;
	}

	setxemulate(emul, args, execsz);
}

int
setxemulate(emul, args, execsz)
	char		*emul;
	struct uarg	*args;
	long		*execsz;
{
	int		error;
	struct vnode	*nvp;

	if (args->traceinval) {
		prinvalidate(&u);
		args->traceinval = 0;
	}

	args->flags |= EMULA;

	/* open emulator */
	if ((error = lookupname(emul, UIO_SYSSPACE, FOLLOW, NULLVP, &nvp)) != 0)
		return error;

	return(gexec(&nvp, args, 1, execsz));
}
/* CHANGE FROM LOCUS FOR MERGE */
/*
** set0emulate - This routine is identical to "setxemulate", except
** that the call to "gexec" has a zero for the "level" parameter.
** The reason for using level 0 is to have the setuid permissions
** of the emulator used.
*/
int
set0emulate(emul, args, execsz)
	char		*emul;
	struct uarg	*args;
	long		*execsz;
{
	int		error;
	struct vnode	*nvp;

	if (args->traceinval) {
		prinvalidate(&u);
		args->traceinval = 0;
	}

	args->flags |= EMULA;

	/* open emulator */
	if ((error = lookupname(emul, UIO_SYSSPACE, FOLLOW, NULLVP, &nvp)) != 0)
		return error;

	return(gexec(&nvp, args, 0, execsz));
}
#endif
