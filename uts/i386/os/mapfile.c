/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:mapfile.c	1.3.1.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/systm.h"
#include "sys/sysi86.h"
#include "sys/vnode.h"
#include "sys/pathname.h"
#include "sys/fs/s5inode.h"
#include "sys/fs/xnamnode.h"
#include "sys/sd.h"
#include "sys/file.h"
#include "sys/uio.h"
#include "sys/cmn_err.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_vn.h"
#include "sys/mman.h"


/*
 *	 SI86PCHRGN:
 *	 Change memory mapped file region.
 *	 The term "region" is from SVR3.2; in SVR4, "region"
 *	 generally means "segment".
 */
chmfile(ap, rvp)
struct cmf *ap;
rval_t *rvp;
{
	struct cmf cmf;
	struct segvn_data *svd;
	struct seg *seg, *nseg;
	caddr_t naddr, saddr, eaddr;
	u_int prot;

	if ((copyin((caddr_t)ap, (caddr_t)&cmf, sizeof(cmf))) < 0)
		return EFAULT;
	if (( rvp->r_val1 = cmf.cf_count) == 0)
		return(0);

	if (((seg=as_segat(u.u_procp->p_as, cmf.cf_dstva)) == NULL) ||
		((nseg = as_segat(u.u_procp->p_as, cmf.cf_dstva + cmf.cf_count))
			== NULL) || (seg != nseg))
		return EFAULT;


	svd = (struct segvn_data *) seg->s_data;
	if (svd->type == MAP_SHARED )
		return EFAULT;

	saddr = cmf.cf_dstva;
	eaddr = cmf.cf_dstva + cmf.cf_count;
	do{
		prot = as_getprot(seg->s_as, saddr, &naddr);
		if (prot&PROT_WRITE || !(prot&PROT_EXEC))
			return EFAULT;
		saddr=naddr;
	} while (naddr < eaddr);
	if (as_setprot(u.u_procp->p_as, cmf.cf_dstva, cmf.cf_count, PROT_ALL) != 0)
		return EFAULT;
	
	if (as_fault(u.u_procp->p_as, cmf.cf_dstva, cmf.cf_count, F_PROT, S_WRITE) != 0)
		return EFAULT;
	/*
	 * fault the memory-mapped file in, and lock the pages in-core.
	 */
	if (as_fault(u.u_procp->p_as, cmf.cf_dstva, cmf.cf_count, F_SOFTLOCK, S_WRITE) != 0)
		return EFAULT;

	if ((copyin(cmf.cf_srcva, cmf.cf_dstva, cmf.cf_count)) < 0)
		return EFAULT;

	if (as_fault(u.u_procp->p_as, cmf.cf_dstva, cmf.cf_count, F_SOFTUNLOCK, S_WRITE) != 0)
		return EFAULT;

	if (as_setprot(u.u_procp->p_as, cmf.cf_dstva, cmf.cf_count, PROT_ALL & ~PROT_WRITE) != 0)
		return EFAULT;

	return(0);
}

/*
 *	findhole()  --  find a hole of npgs in process p's user
 *		address space
 */
caddr_t
findhole(p, npgs)
register proc_t *p;
long	npgs;
{
	register int i;
	addr_t base;
	u_int len = ctob(npgs);

	if (len <= 0)
		return(NULL);

	for (i = ptnum(UVSHM) - 1; i >= 0; i--) {
		base = (addr_t)(i << PTNUMSHFT);
		len = ctob(npgs);
		if (as_gap(p->p_as, len, &base, &len, AH_CONTAIN, base) == 0)
			if (npgs == btoc (len))
				return(base);
	}
	return(NULL);
}

/*
 *	SI86SHFIL:
 *	Map a file into user address space.  If mmf.mf_flags & MAP_PRIVATE
 *	is set, the file is mapped into a copy-on-write segment.  Otherwise,
 *	it is mapped into a read-only shareable segment.
 */
mapfile(ap, rvp)
struct mmf *ap;
rval_t *rvp;
{
	struct mmf mmf;
	vnode_t	*vp;
	struct vattr	vattr;
	struct pathname	pn;
	caddr_t vaddr;
	int	save_syscall;
	int	error = 0;
	int rtn;

	if ((copyin((caddr_t)ap, (caddr_t)&mmf, sizeof(mmf))) < 0)
		return EFAULT;
	pn.pn_path = mmf.mf_filename;

	/*
	 * In case the system call goes remote set u_syscall to something
	 * the remote system recognizes.  We restore u_syscall before
	 * leaving mapfile().
	 */
	save_syscall = u.u_syscall;	

	/*
	 * Lookup path name and remember last component for later.
	 */
	if (error = pn_get(mmf.mf_filename, UIO_USERSPACE, &pn)){
		u.u_syscall = save_syscall;	/* restore u_syscall */
		return error;
	}

	if (error = lookuppn(&pn, FOLLOW, NULLVPP, &vp)) {
		pn_free(&pn);
		u.u_syscall = save_syscall;	/* restore u_syscall */
		return error;
	}
	pn_free(&pn);

	if ((vp == NULL) && !is286EMUL ){
		VN_RELE(vp);
		u.u_syscall = save_syscall;	/* restore u_syscall */
		return (0);
	}

	if (error = VOP_ACCESS(vp, VEXEC, 0, u.u_cred)){
		VN_RELE(vp);
		u.u_syscall = save_syscall;	/* restore u_syscall */
		return error;
	}
	vattr.va_mask = AT_SIZE;
	if (error = VOP_GETATTR(vp, &vattr, 0, u.u_cred)){
		VN_RELE(vp);
		u.u_syscall = save_syscall;	/* restore u_syscall */
		return error;
	}

	/* RFS needs it! */
	if ((error = VOP_OPEN(&vp, FREAD, u.u_cred)) != 0) {
#ifdef DEBUG
		cmn_err(CE_CONT,"VOP open failed on vp %x\n", vp);
		call_demon();
#endif
		VN_RELE(vp);
		u.u_syscall = save_syscall;	/* restore u_syscall */
		return error;
	}

	switch(mmf.mf_flags) {
	case 0:						/* shareable segment */
		mmf.mf_filesz = mmf.mf_regsz = 0;

		if (vattr.va_size == 0) 
			goto done;
		if ((rtn = maptfile(vp, &mmf, vattr.va_size)) == -1){
			error = ENOMEM;
			goto done;
		}
		rvp->r_val1 = rtn;
		break;

	case MAP_PRIVATE:				/* copy-on-write */
		if (mmf.mf_filesz > mmf.mf_regsz) {
			error = EFAULT;
			goto done;
		}
		if (mmf.mf_regsz == 0)
			goto done;
		if ((rtn = mapdfile(vp, &mmf)) == -1){
			error = ENOMEM;
			goto done;
		}
		rvp->r_val1 = rtn;
		break;

	default:
		error = EINVAL;
	}
done:
	u.u_syscall = save_syscall;	/* restore u_syscall */
	/*  RFS needs it! */
	VOP_CLOSE(vp, FREAD, 1, 0, u.u_cred);
	VN_RELE(vp);
	if ((copyout((caddr_t)&mmf, (caddr_t)ap, sizeof(mmf))) < 0)
		error =  EFAULT;
	return error;
}


/*
 *	mapdfile()  --  map the vnode into a copy-on-write as.
 *		If successful, return the address.
 */
mapdfile(vp, mp)
	struct vnode	*vp;
	struct mmf	*mp;
{
	caddr_t	vaddr;
	int	npgs;

	/*
	 * Find a virtual address to which to attach the segment.
	 */
	if (((vaddr = findhole(u.u_procp,btoc(mp->mf_regsz))) == NULL)) {
#ifdef DEBUG
		cmn_err(CE_CONT,"mapdfile: findhole failed\n");
#endif
		u.u_error = ENOMEM;
		goto err;
	}

	if (execmap(vp, vaddr, mp->mf_filesz, (mp->mf_regsz - mp->mf_filesz),
			(size_t)0, PROT_ALL)) {
#ifdef DEBUG
		cmn_err(CE_CONT,"mapdfile: execmap failed: vaddr = %x\n",vaddr);
#endif
		goto err;
	}

	return((int)vaddr);
err:
	return(-1);
}

/*
 *	maptfile()  --  map the vnode into a shareable as. If
 *		successful, return the address. 
 */
maptfile(vp, mp, filesz)
	struct vnode	*vp;
	struct mmf	*mp;
	long filesz;
{
	caddr_t	vaddr;
	struct seg		*seg, *endseg;
	struct segvn_data	*svd;

	if (u.u_procp->p_as && u.u_procp->p_as->a_segs) {
		seg = endseg = u.u_procp->p_as->a_segs;
		do {
			if (seg->s_ops != &segvn_ops)
				continue;
			svd = (struct segvn_data *) seg->s_data;

			if (svd->vp == vp && svd->offset == (u_int)0 &&
				svd->pageprot == 0  &&
				((svd->prot & S_WRITE) == 0) &&
				(filesz <= seg->s_size)) {
#ifdef DEBUG
	cmn_err(CE_CONT,"maptfile: segment exits: vaddr = %x\n",seg->s_base);
#endif
				mp->mf_filesz = filesz;
				mp->mf_regsz = seg->s_size;
				return((int)seg->s_base);
			}

		} while ((seg = seg->s_next) != endseg);
	}
	else {
#ifdef DEBUG
		cmn_err(CE_CONT,"maptfile: process is not yet mapped\n");
#endif
		goto err;
	}

	if ((vaddr = findhole(u.u_procp, btoc(filesz))) == NULL) {
#ifdef DEBUG
		cmn_err(CE_CONT,"maptfile: findhole failed\n");
#endif
		u.u_error = ENOMEM;
		goto err;
	}
	if (execmap(vp, vaddr, filesz, (size_t)0, (off_t)0,
			PROT_ALL & ~PROT_WRITE)) {
#ifdef DEBUG
		cmn_err(CE_CONT,"maptfile: execmap failed\n");
#endif
		goto err;
	}

	mp->mf_filesz = filesz;
	if ((seg = as_segat(u.u_procp->p_as, vaddr)) == (struct seg *) NULL)
		cmn_err(CE_PANIC,"maptfile: no segment for vaddr = %x\n",vaddr);

	mp->mf_regsz = seg->s_size;
	return((int)vaddr);

err:
	return(-1);
}


/*
 *	Xsdswtch - called during process switch if special XENIX shared
 *			data context switching is to be performed. 
 *
 *		dir == 0 when switching from this proc.
 *		dir == 1 when switching to this proc.
 *	
 *	This routine is only called at context switch time, and only if 
 *      BADVISE_XSDSWTCH is true.  The BADVISE_XSDSWTCH can only be affected
 *	by the SI86BADVISE subcommand to the sysi86() system call.
 *
 *	When context switching TO the proc,
 *	the shared data will be copied from the "real" shared data segment,
 *	which starts at sd_addr, to the 286 small data executable's private
 *	data (at sd_cpaddr).  When context switching FROM the proc, the
 *	shared data is copied from the 286 private copy (starting at sd_cpaddr)
 *	to the "real" shared data segment (at sd_addr).  Note that we will
 *	not block during the copy because noswapcnt is greater than zero for
 *	both the region where sd_cpaddr lives and the shared memory
 *	region where sd_addr lives.  The regions' noswapcnt field is
 *	affected as follows:
 *
 *		1.  The SI86SHRGN subcommand to the sysi86() system call 
 *		    will increment the 286 private data and shmem regions' 
 *		    noswapcnt if the shared data segment's sd_cpaddr is 
 *		    changed from NULL to something other than NULL.
 *
 *		2.  The SI86SHRGN subcommand to the sysi86() system call 
 *		    will decrement the 286 private data and shmem regions' 
 *		    noswapcnt if the shared data segment's sd_cpaddr is 
 *	  	    changed from non-NULL to NULL.
 *
 *		3.  At fork time, xsdfork() will increment the 
 *		    regions' noswapcnt for each shared data segment whose
 *		    sd_cpaddr is non-NULL.
 *
 *		4.  When a shared data segment whose sd_cpaddr is non-NULL is
 *		    freed, the regions' noswapcnt is decremented.
 *
 * N.B.  We'd really like to skip the copyin() when dir==0 if the sd
 *       seg is attached read-only.  However, this is not the way XENIX
 *       worked.  Instead, we allow changes made by the 286 small data
 *	 model proc to read-only shared data to be reflected in the
 *       "real" shared data segment.  Sigh.
 */
xsdswtch(dir)
int dir;
{
	register struct sd *sdp;

	for (sdp = u.u_procp->p_sdp; sdp != NULL; sdp = sdp->sd_link) {
		if ((sdp->sd_addr != NULL) && (sdp->sd_cpaddr != NULL)) {
			if (dir) {
				/* switch TO this proc */
				if (copyout(sdp->sd_addr, sdp->sd_cpaddr, 
					sdp->sd_xnamnode->x_sd->x_len + 1)==-1)
					cmn_err(CE_WARN,
					   "xsdswtch - couldn't copy %d bytes of XENIX shared data from 0x%x to 0x%x",
				   		sdp->sd_xnamnode->x_sd->x_len + 1,
						sdp->sd_addr, sdp->sd_cpaddr);
			}
			else {
				/* switch FROM this proc */
				if (copyin(sdp->sd_cpaddr, sdp->sd_addr, 
					sdp->sd_xnamnode->x_sd->x_len + 1)==-1)
					cmn_err(CE_WARN,
					   "xsdswtch - couldn't copy %d bytes of XENIX shared data from 0x%x to 0x%x",
				   		sdp->sd_xnamnode->x_sd->x_len + 1,
				   		sdp->sd_cpaddr, sdp->sd_addr);
			}
		}
	}
}

xsd86shrgn(xsdbp)
struct xsdbuf *xsdbp;
{
	struct sd *sdp;
	struct xsdbuf	xsdb;

	if  ((copyin((caddr_t)xsdbp, (caddr_t)&xsdb, sizeof(xsdb))) == -1) {
		u.u_error = EFAULT;
		return;
	}

	/* find the right sd entry */
	if (sdsrch(xsdb.xsd_386vaddr, &sdp) == NULL)
		return;

	switch(xsdb.xsd_cmd) {
		case SI86SHR_SZ:
		/* 
		 * Return size of shared data segment.
		 */
			xsdb.xsd_un.xsd_size = 
				(unsigned long) sdp->sd_xnamnode->x_sd->x_len + 1;
			if  ((copyout((caddr_t)&xsdb, (caddr_t)xsdbp,
							sizeof(xsdb))) == -1) {
				u.u_error = EFAULT;
				return;
			}
			break;
		case SI86SHR_CP:
		/*
		 * Enable/disable XENIX small model shared data context    
		 * switching support.  The 'xsdbp' argument is a pointer   
		 * to an xsdbuf struct which contains the 386 start addr
		 * for the sd seg and the 286 start addr for the sd seg.
		 * When a proc that has requested shared data copying (via
		 * SI86BADVISE) is switched
		 * to, the kernel copies the sd seg from the 386 addr to 
		 * the 286 addr.  When the proc is switched from, the kernel 
		 * copies the sd seg from the 286 addr to the 386 addr. 
		 * Note that if the 286 addr is NULL, the   
		 * shared data segment's context switching support is 
		 * disabled.
		 */
			if (xsdb.xsd_un.xsd_286vaddr == NULL) {
				/* 
				 * Changing cpaddr to NULL.
				 */
				if (sdp->sd_cpaddr != NULL) {
					char	*cpaddr;	
					
					cpaddr = sdp->sd_cpaddr;
					sdp->sd_cpaddr = NULL;
					/* decrement regions' noswapcnt */
					xsdunlock(u.u_procp, cpaddr); 
					xsdunlock(u.u_procp, sdp->sd_addr); 
				}
			}
			else {
				/* 
				 * Changing cpaddr.  If changing from NULL, lock
				 * down the regions.
				 */
				if (sdp->sd_cpaddr == NULL) { 
					/* inc the regions' noswapcnt */
					if (xsdlock(u.u_procp, xsdb.xsd_un.xsd_286vaddr) == 0)	
						/*  couldn't inc noswapcnt */
						return;	    
					if (xsdlock(u.u_procp, sdp->sd_addr) == 0)	
						/*  couldn't inc noswapcnt */
						return;	    
				}
				/* change to new cpaddr */
				sdp->sd_cpaddr = xsdb.xsd_un.xsd_286vaddr; 
			}

			break;
	}
	
}

/*
 * In 4.0, vm takes care of locking memory on a per-page basis
 */
xsdlock(procp, addr)
struct	proc *procp;
char	*addr;
{
	return(1);
}
		
/*
 * In 4.0, vm takes care of unlocking memory on a per-page basis
 */
xsdunlock(procp, addr)
struct	proc *procp;
char	*addr;
{
	return(1);
}
