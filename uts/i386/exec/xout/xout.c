/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-exe:xout/xout.c	1.3"

/* XENIX Support */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/mman.h"
#include "sys/kmem.h"
#include "sys/fstyp.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/reg.h"
#include "sys/var.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/fp.h"
#include "sys/seg.h"
#include "sys/tty.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/rf_messg.h"
#include "sys/conf.h"
#include "sys/uio.h"
#include "sys/pathname.h"
#include "sys/x.out.h"
#include "sys/exec.h"

#include "vm/seg.h"
#include "vm/seg_vn.h"
#include "vm/as.h"

short	xoutmagic = X_MAGIC;

#define	 X286EMUL	"/usr/bin/x286emul"

extern	 vnodeops_t	*rf_vnopsp;		/* for RFS Kludge */
/* XXX re work to remove RFS dependency
extern	 int		rf_vp_preSVR4();
 */

extern int userstack[];

int
xoutexec(vp,args, level, execsz, ehdp, setid)
struct vnode	*vp;
struct uarg	*args;
long		*execsz;
exhda_t		*ehdp;
int		setid;
int level;
{
	struct execenv		exenv;
	struct exdata		edp;
	long			vbsize = VBSIZE(vp);
	int			error=0;
	register int		i;
	u_short			rfsflag = 0;
	struct dscr		*dscrp;
	register u_long		base;
	register int		size;
	register caddr_t	offset;
	register u_long		adjust = 0;
	u_long			regva;
	int			adjoff = 0;
	struct proc		*p = u.u_procp;
 	extern short		dufstyp;

	if ((error = getxouthead(vp, &edp, execsz, ehdp, args)) != 0)
		return error;
	
	if (p->p_sdp)
		xsdexit();	/* XENIX shared data exit */

	u.u_userstack = (u_long) userstack;

	remove_proc(args);

	/* allow for text origins other than 0 */
	size = (int) (edp.ux_tsize + (u_int) edp.ux_txtorg);
	offset = (caddr_t) edp.ux_toffset;
	base = (u_long) edp.ux_txtorg;

	/*
	 * First make sure that text & data don't overlap for
	 * separate I & D space processes. (note that 413 x.out binaries
	 * are normally split also.)
	 *
	 * NOTE: for now we just force text to top of address space,
	 * we may be able to fix this up for more efficient use of page
	 * tables later.  With the current x.out layout, we could
	 * usually fit it below the data, but that would required sharing
	 * regions, and re-mapping the start of data.
	 * 
	 */
	regva = (0xbffff000 - (size + (int)offset)) & ~SOFFMASK;

	/*
	 * set up segment descriptor.  The 'adjust' is so we
	 * can handle the non-aligned part in the segment table,
	 * to allow for non-filesystem-block aligned binaries,
	 * without a lot of extra overhead (i.e., no extra read
	 * in S5READMAP()).  Olson, 3/87
	 */
/*  XXX re work this to remove RFS dependency 
	if (vp->v_op == rf_vnopsp && rf_vp_preSVR4(vp)) {
		rfsflag = 1;
	}
	else 
*/
		if (base <= (u_long)offset) {
		if (PAGOFF(base) != PAGOFF(offset)) {
			adjust = PAGOFF((u_long)offset - base);
		}
	}

	dscrp = (struct dscr *)(p->p_ldt)+seltoi(USER_CS);
	setdscrbase(dscrp, regva + adjust);
	setdscrlim(dscrp, btoct(size - 1));
	setdscracc1(dscrp, UTEXT_ACC1);
	setdscracc2(dscrp, TEXT_ACC2);	/* page gran., 32 bit */

	/*
	 * Load the a.out's text and data.
	 */
	if (error = execmap(edp.vp, (caddr_t) (regva+adjust), (size_t) size,
			(size_t) 0, (off_t) offset,
					(PROT_ALL & ~PROT_WRITE))){
		psignal(p, SIGKILL);
		goto done;
	}

	/* handle data section */
	offset = (caddr_t) edp.ux_doffset;
	base = (u_long) edp.ux_datorg;

	/*
	if (rfsflag) {
		adjust = 0;
	}
	else if (adjust = PAGOFF(base)) {
		if (adjust <= (u_long)offset) {
			adjoff = (((u_long)offset - adjust)/vbsize 
				    - (u_long)offset/vbsize)*vbsize;
			adjust = ((u_long)offset - adjust) % vbsize;
		}
		else {
			adjust = (u_long) offset % vbsize;
		}
	}
	else {
		adjust = (u_long) offset % vbsize;
	}
	*/

	dscrp = (struct dscr *)(p->p_ldt)+seltoi(USER_DS);
	setdscrbase(dscrp, UVBASE);
	setdscrlim(dscrp, btoct(MAXUVADR - 1));
	setdscracc1(dscrp, UDATA_ACC1);
	setdscracc2(dscrp, DATA_ACC2);	/* page gran., 32 bit */

	if (error=execmap(edp.vp, (caddr_t) base, (size_t) edp.ux_dsize,
			(size_t) edp.ux_bsize, (off_t) offset,PROT_ALL)) {
		psignal(p, SIGKILL);
		goto done;
	}

	/*
	 * For XENIX binarires the data page at virtual address 0
	 * must be accessible to allow NULL pointer dereferences.
	 * If not already attached to the process address space,
	 * map in the first page.  
	 */
	if (as_segat(p->p_as, 0) == NULL) {
		if (error = as_map(p->p_as, 0, NBPP, 
			           segvn_create, zfod_argsp))
			goto done;
	}

	exenv.ex_brkbase =
		(caddr_t)(base + edp.ux_dsize + edp.ux_bsize);
	exenv.ex_magic = X_MAGIC;
	exenv.ex_vp = vp;
	setexecenv(&exenv);

done:
	if (error == 0) {
		u.u_exdata = edp;	/* XXXX dependency on core file */
		u.u_renv = u.u_exdata.ux_renv;
	}

	return error;
}


/*
 * Get the file header.
 */
int
getxouthead(vp, edp, execsz, ehdp, args)
struct vnode *vp;
register struct exdata *edp;
long *execsz;
exhda_t *ehdp;
struct uarg *args;
{
	struct xexec *filhdrp;
	off_t	offset;
	int error, indir = 0;

	if ((error = exhd_getmap(ehdp, (off_t) 0, sizeof(*filhdrp),
			EXHD_4BALIGN | EXHD_KEEPMAP, (caddr_t) &filhdrp)) != 0)
		goto bad;

	if (!(filhdrp->x_renv & XE_EXEC)) {
		error = ENOEXEC;
		goto bad;
	}

	switch(filhdrp->x_magic) {
		case X_MAGIC:
			if ((filhdrp->x_cpu & XC_386) != XC_386) {

				/*
				 * the emulator must determine if this is
			 	 * a valid 8086 or 80286 binary.
			 	 */

				if (error = setemulate(X286EMUL, vp, args, execsz)) 
					goto bad;
				else {
					/* support for execute only binaries */
					u.u_renv |= UE_EMUL;	
					return -1;	/* let gexec know - don't
							   want to go any further */
				}
			}
			break;
		default:
			error = ENOEXEC;
			goto bad;
	}

	if (!readxouthdr(vp, edp, filhdrp, ehdp, execsz)) {
		error = ENOEXEC;
		goto bad;
	}

	/*
 	 * Check total memory requirements (in clicks) for a new process
	 * against the available memory or upper limit of memory allowed.
	 */

	if (*execsz > btoc(u.u_rlimit[RLIMIT_VMEM].rlim_cur)) {
		error = ENOMEM;
		goto bad;
	}

	edp->vp = vp;
	return 0;
bad:
	return error;
}


/*
 *	XENIX 386 x.out binary COMPATIBILITY
 */
readxouthdr(vp, up, xouthdr, ehdp, execsz)
register struct vnode *vp;
register struct exdata *up;
register struct xexec  *xouthdr;
exhda_t *ehdp;
long *execsz;
{
 	unsigned segsize, i;
 	off_t	 segoffset;
 	struct xext  *exext;	/* extension to header */
 	struct xseg  *xseg;
	int    resid;
	int    error;
 
 	/* Fill in what we can now */
 
	up->ux_tsize = (u_int) xouthdr->x_text;
 	up->ux_dsize = (u_int) xouthdr->x_data;
 	up->ux_bsize = (u_int) xouthdr->x_bss;
 
 	*execsz += btoc(up->ux_tsize + up->ux_dsize + up->ux_bsize);
 
 	/* No shared library support yet */
 	up->ux_nshlibs = 0;
 	up->ux_lsize = 0;
 	up->ux_loffset = 0;
 
 	/*
 	 * don't allow 386 impure or multiple segment binaries.
 	 */
 	if (((xouthdr->x_cpu & XC_386) == XC_386) &&
 	    (((xouthdr->x_renv & XE_SEP) == 0) ||
   	    (xouthdr->x_renv & (XE_LTEXT|XE_LDATA)))) {
 		return 0;
 	}
 
 	up->ux_entloc = (caddr_t)xouthdr->x_entry;

	/*
	 * this may be the wrong magic number to use
	 * since we do paging only if the file alignment
	 * meets paging requirment
	 */
 	up->ux_mag = ZMAGIC;
 	up->ux_renv = xouthdr->x_renv | (xouthdr->x_cpu << 16);
 
 	/*
 	 * get header extension 
 	 * xe_eseg is the last structure element in xext
 	 */
 	if (xouthdr->x_ext < STRUCTOFF(xext,xe_eseg) + sizeof(exext->xe_eseg))
 		return 0;

	if ((error = exhd_getmap(ehdp, (off_t) sizeof(struct xexec),
			min(sizeof(exext), xouthdr->x_ext),
			EXHD_4BALIGN | EXHD_KEEPMAP, (caddr_t) &exext)) != 0)
		return 0;

 	/* Get the segment table */
 	for(segsize=0, segoffset=exext->xe_segpos;
	    segsize < exext->xe_segsize;
	    segsize += sizeof(xseg), segoffset += sizeof(xseg)) {

		if ((error = exhd_getmap(ehdp, (off_t) segoffset, sizeof(*xseg),
			EXHD_4BALIGN | EXHD_KEEPMAP, (caddr_t) &xseg)) != 0)
			return 0;
 
 		if(xseg->xs_type == XS_TTEXT) {
 			if((xseg->xs_seg&(SEL_LDT|SEL_RPL)) != (SEL_LDT|SEL_RPL))
 				return 0;	/* must be in LDT at ring 3 */
 			up->ux_toffset = (long) xseg->xs_filpos;
 			up->ux_txtorg  = (caddr_t) xseg->xs_rbase;
 		}
 		else if (xseg->xs_type == XS_TDATA) {
 			if((xseg->xs_seg&(SEL_LDT|SEL_RPL)) != (SEL_LDT|SEL_RPL))
 				return 0;	/* must be in LDT at ring 3 */
 			up->ux_doffset = (long) xseg->xs_filpos;
 			up->ux_datorg  = (caddr_t) xseg->xs_rbase;

 			/*
 			   some xenix binaries (mainly 8086/80286) treat data
 			   that is initialized to 0's as though it was BSS, so
 			   we have to recalculate the dsize and bsize, so we
 			   won't get errors in mapping segment, since the 'non-existent'
 			   initialized 0 data may extend past the actual end of
 			   the file.  Olson, 5/87
 			*/
 			if(!(xouthdr->x_renv & (XE_LTEXT|XE_LDATA)) &&
 				(u_int) xseg->xs_psize < up->ux_dsize) {
 				up->ux_dsize = (u_int) xseg->xs_psize;
 				up->ux_bsize = (u_int)(xseg->xs_vsize - xseg->xs_psize);
 			}
 		}
 	}
 	return 1;	/* OK */
}

xoutcore(vp, pp, credp, rlimit, sig)
	vnode_t		*vp;
	proc_t		*pp;
	struct cred	*credp;
	rlim_t		rlimit;
	int		sig;
{
	return(coffcore(vp, pp, credp, rlimit, sig));

}
/* End XENIX Support */
