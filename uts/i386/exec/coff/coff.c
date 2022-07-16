/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-exe:coff/coff.c	1.3.1.1"

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
#include "sys/tty.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/rf_messg.h"
#include "sys/conf.h"
#include "sys/uio.h"
#include "sys/pathname.h"
#include "sys/exec.h"
#include "sys/seg.h"
#include "sys/x.out.h"
#include "sys/inline.h"

short	coffmagic = 0514;	/* magic number for COFF executables */

#ifndef i386
extern int mau_present;
#endif

void coffexec_err();
STATIC int getcoffhead();
int getcoffshlibs();

#ifdef i386	/* We highlighted this - helps during porting phase - */
		/* i.e., if we ever become the porting base */
extern int minhidustk, minustkgap;
extern int userstack[];
#endif

/* ARGSUSED */
int
coffexec(vp, args, level, execsz, ehdp, setid)
struct vnode *vp;
struct uarg *args;
int level;
long	 *execsz;
exhda_t *ehdp;
int setid;
{
	struct execenv exenv;
	struct exdata edp;
	int error=0;
#ifdef notnow
	int teseg;
#endif
	register int i;
	struct exdata *shlb_dat,*datp;
	u_int shlb_datsz; 
	char ldatcache[2 * sizeof(struct exdata)];
	int dataprot = PROT_ALL;
	int textprot = PROT_ALL & ~PROT_WRITE;
	struct proc *pp = u.u_procp;

	if ((error = getcoffhead(vp, &edp, execsz, ehdp)) != 0)
		return error;
	
        /*
         * Look at what we got, edp.ux_mag = 410/411/413.
	 *
         * 410 is RO text.
	 * 411 is separated ID (treated like a 0410).
	 * 413 is RO text in an "aligned" a.out file.
	 */
	switch (edp.ux_mag) {
	case 0410:
	case 0411:
	case 0413:
                break;

        case 0443:
                error = ELIBEXEC;
		break;
	default:
		error = ENOEXEC;
		break;
	}

	if (error)
		return error;


	if (edp.ux_nshlibs) {
		shlb_datsz = edp.ux_nshlibs * sizeof(struct exdata);

		if (shlb_datsz > sizeof(ldatcache))
			shlb_dat = (struct exdata *)kmem_alloc(shlb_datsz,KM_SLEEP);
		else
			shlb_dat = (struct exdata *)ldatcache;

		datp = shlb_dat;

		if ((error = getcoffshlibs(vp, &edp, datp, execsz, ehdp)) != 0)
			goto done;
	}

#ifdef i386
	u.u_userstack = (u_long) userstack;	/* always for 386 COFF */
#endif

	if ((error = remove_proc(args)) != 0)
		goto done;

#ifdef i386
	edp.ux_renv = XE_V5|XE_EXEC|U_IS386|U_ISCOFF|U_ISWSWAP;
#endif

	/*
	 * Load any shared libraries that are needed.
	 */

	if (edp.ux_nshlibs) {
		for (i = 0; i < edp.ux_nshlibs; i++, datp++) {
			if (error = execmap(datp->vp,datp->ux_txtorg,
			    datp->ux_tsize, (off_t) 0, datp->ux_toffset,
						textprot)) {
				coffexec_err(++datp, edp.ux_nshlibs -i -1);
				psignal(pp, SIGKILL);
				goto done;
			}

			if (error = execmap(datp->vp,datp->ux_datorg,
			    datp->ux_dsize, (off_t)datp->ux_bsize,
			    datp->ux_doffset, dataprot)) {
				coffexec_err(++datp, edp.ux_nshlibs -i -1);
				psignal(pp, SIGKILL);
				goto done;
			}
			VN_RELE(datp->vp);	/* done with this reference */
		}
	}

	/*
	 * Load the a.out's text and data.
	 */
	if (error = execmap(edp.vp, edp.ux_txtorg,
	    edp.ux_tsize, (off_t)0, edp.ux_toffset,textprot)){
		psignal(pp, SIGKILL);
		goto done;
	}

	if (error = execmap(edp.vp, edp.ux_datorg,
	    edp.ux_dsize, edp.ux_bsize, edp.ux_doffset,dataprot)){
		psignal(pp, SIGKILL);
		goto done;
	}

	exenv.ex_brkbase =
		(caddr_t)(edp.ux_datorg + edp.ux_dsize + edp.ux_bsize);
	exenv.ex_magic = coffmagic;
	exenv.ex_vp = vp;
	setexecenv(&exenv);

#ifdef notnow
	/*
	 *	XXX - Need to re-think this out for COFF.
	 */
	if (u.u_userstack == (unsigned long) userstack) {
 		if (((long)edp.ux_datorg & SOFFMASK) < minhidustk)
 			goto stackdone;
 		teseg = ctos(btoc(edp.ux_tsize + edp.ux_toffset));
 		if ((long)edp.ux_datorg - ctob(stoc(teseg)) < minustkgap)
 			goto stackdone;
 		u.u_userstack = ctob(btoct(edp.ux_datorg)) - sizeof(int);
 	}

stackdone:
#endif


done:
	if (error == 0) {
		u.u_exdata = edp;	/* XXXX dependency on core file */
#ifdef i386
		u.u_renv = u.u_exdata.ux_renv;
#endif
	}

	if (edp.ux_nshlibs){
		if (shlb_dat != (struct exdata *)ldatcache)
			kmem_free(shlb_dat,shlb_datsz);
	}
	return error;
}


/*
 * Read the a.out headers.  There must be at least three sections,
 * and they must be .text, .data and .bss (although not necessarily
 * in that order).
 *
 * Possible magic numbers are 0410, 0411 (treated as 0410), and
 * 0413.  If there is no optional UNIX header then magic number
 * 0410 is assumed.
 */

/*
 *   Common object file header.
 */

/* f_magic (magic number)
 *
 *   NOTE:   For 3b-5, the old values of magic numbers
 *           will be in the optional header in the
 *           structure "aouthdr" (identical to old
 *           unix aouthdr).
 */

#define  IAPX386MAGIC	0514

/* f_flags
 *
 *	F_EXEC  	file is executable (i.e. no unresolved 
 *	        	  externel references).
 *	F_AR16WR	this file created on AR16WR machine
 *	        	  (e.g. 11/70).
 *	F_AR32WR	this file created on AR32WR machine
 *	        	  (e.g. vax, 386).
 *	F_AR32W		this file created on AR32W machine
 *	        	  (e.g. 3B, maxi).
 */
#define  F_EXEC		0000002
#define  F_AR16WR	0000200
#define  F_AR32WR	0000400
#define  F_AR32W	0001000

struct filehdr {
	u_short	f_magic;	
	u_short	f_nscns;	/* number of sections */
	long	f_timdat;	/* time & date stamp */
	long	f_symptr;	/* file pointer to symtab */
	long	f_nsyms;	/* number of symtab entries */
	u_short	f_opthdr;	/* sizeof(optional hdr) */
	u_short	f_flags;
};

/*
 *  Common object file section header.
 */

/*
 *  s_name
 */
#define _TEXT ".text"
#define _DATA ".data"
#define _BSS  ".bss"
#define _LIB  ".lib"

/*
 * s_flags
 */
#define	STYP_TEXT	0x0020	/* section contains text only */
#define STYP_DATA	0x0040	/* section contains data only */
#define STYP_BSS	0x0080	/* section contains bss only  */
#define STYP_LIB	0x0800	/* section contains lib only  */

struct scnhdr {
	char	s_name[8];	/* section name */
	long	s_paddr;	/* physical address */
	long	s_vaddr;	/* virtual address */
	long	s_size;		/* section size */
	long	s_scnptr;	/* file ptr to raw	*/
				/* data for section	*/
	long	s_relptr;	/* file ptr to relocation */
	long	s_lnnoptr;	/* file ptr to line numbers */
	u_short	s_nreloc;	/* number of relocation	*/
				/* entries		*/
	u_short	s_nlnno;	/* number of line	*/
				/* number entries	*/
	long	s_flags;	/* flags */
};

/*
 * Common object file optional unix header.
 */

struct aouthdr {
	short	o_magic;	/* magic number */
	short	o_stamp;	/* stamp */
	long	o_tsize;	/* text size */
	long	o_dsize;	/* data size */
	long	o_bsize;	/* bss size */
	long	o_entloc;	/* entry location */
	long	o_tstart;
	long	o_dstart;
};

/*
 * Get the file header, handling '#!' indirection if required.
 */
int
getcoffhead(vp, edp, execsz, ehdp)
struct vnode *vp;
register struct exdata *edp;
long *execsz;
exhda_t *ehdp;
{
	struct filehdr *filhdrp;
	struct aouthdr *aouthdrp;
	struct scnhdr  *scnhdrp;
	int    opt_hdr = 0;
	int    scns    = 0;
	off_t	offset;
	int error;
	int nscns;
	int ssz;

	if ((error = exhd_getmap(ehdp, (off_t) 0, sizeof(*filhdrp),
		EXHD_4BALIGN, (caddr_t)&filhdrp)) != 0)
		goto bad;

	if (filhdrp->f_magic != IAPX386MAGIC || !(filhdrp->f_flags & F_EXEC)) {
		error = ENOEXEC;
		goto bad;
	}

	/* get what is needed from filhdrp now, since it is
	 * not kosher to modify the file page and little is needed.
	 */
	nscns = filhdrp->f_nscns;
	offset = sizeof(*filhdrp) + filhdrp->f_opthdr;
	
	/*
	 * Next, read the optional unix header if present; if not,
	 * then we will assume the file is a 410.
	 */

	if (filhdrp->f_opthdr >= sizeof(*aouthdrp)) {
		error = exhd_getmap(ehdp, (off_t) sizeof(*filhdrp),
				sizeof(*aouthdrp),
				EXHD_4BALIGN, (caddr_t) &aouthdrp);

			if (error)
				goto bad;

		opt_hdr = 1;
		edp->ux_mag = aouthdrp->o_magic;
		edp->ux_entloc = (caddr_t)aouthdrp->o_entloc;
	}

	/*
	 * Next, read the section headers.  There had better be at
	 * least three: .text, .data and .bss.  The shared library
	 * section is optional; initialize the number needed to 0.
	 */

	edp->ux_nshlibs = 0;

	ssz = nscns * sizeof(*scnhdrp);
	error = exhd_getmap(ehdp, offset, ssz,
			EXHD_4BALIGN|EXHD_KEEPMAP,
			(caddr_t) &scnhdrp);
	if (error) goto bad;

	for ( ; nscns > 0; scnhdrp++, nscns--) {

		switch ((int) scnhdrp->s_flags) {

		case STYP_TEXT:
			scns |= STYP_TEXT;

			if (!opt_hdr) {
				edp->ux_mag = 0410;
				edp->ux_entloc = (caddr_t)scnhdrp->s_vaddr;
			}

			edp->ux_txtorg = (caddr_t)scnhdrp->s_vaddr;
			edp->ux_toffset = scnhdrp->s_scnptr;
			*execsz += btoc(edp->ux_tsize = scnhdrp->s_size);
			break;

		case STYP_DATA:
			scns |= STYP_DATA;
			edp->ux_datorg = (caddr_t)scnhdrp->s_vaddr;
			edp->ux_doffset = scnhdrp->s_scnptr;
			*execsz += btoc(edp->ux_dsize = scnhdrp->s_size);
			break;

		case STYP_BSS:
			scns |= STYP_BSS;
			*execsz += btoc(edp->ux_bsize = scnhdrp->s_size);
			break;

		case STYP_LIB:
			++shlbinfo.shlblnks;

			if ((edp->ux_nshlibs = scnhdrp->s_paddr) >
                          shlbinfo.shlbs) {
                                ++shlbinfo.shlbovf;
                                error = ELIBMAX;
                                goto bad;
                        }

			edp->ux_lsize = scnhdrp->s_size;
			edp->ux_loffset = scnhdrp->s_scnptr;
			break;
		}
	}

	if (scns != (STYP_TEXT|STYP_DATA|STYP_BSS)) {
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

int
getcoffshlibs(vp, edp, dat_start, execsz, ehdp)
struct	vnode	*vp; 
struct exdata *edp;
struct	exdata *dat_start;
long *execsz;
exhda_t	*ehdp;
{
	struct vnode *nvp;
	unsigned *bp;
	struct	exdata *dat = dat_start;
	unsigned n = 0;
	unsigned *libend;
	char *shlibname;
	struct vattr vattr;
	int error;
	exhda_t lhda;

	error = exhd_getmap(ehdp, edp->ux_loffset, edp->ux_lsize,
			EXHD_4BALIGN, (caddr_t) &bp);
	if (error)
		return error;

	edp->ux_nshlibs = 0;	/* Elf may call this code */
	libend = bp + (edp->ux_lsize / sizeof(*bp));

	while (bp < libend) {

		/* Check the validity of the shared lib entry. */

		if (bp[0] * NBPW > edp->ux_lsize
		  || bp[1] > edp->ux_lsize || bp[0] < 3) {
			error = ELIBSCN;
			goto bad;
		}

		/* Locate the shared lib and get its header info.  */

		shlibname = (caddr_t)(bp + bp[1]);
		bp += bp[0];

		if (lookupname(shlibname, UIO_SYSSPACE, FOLLOW, NULLVPP, &nvp)) {
			error = ELIBACC;
			goto bad;
		}
		/*
		 * nvp has a VN_HOLD on it,
		 * so a VN_RELE must happen for all error cases.
		 */

		vattr.va_mask = AT_MODE;
		if (error = VOP_GETATTR(nvp, &vattr, ATTR_EXEC, u.u_cred)) {
			VN_RELE(nvp);
			goto bad;
		}

		if ((error = VOP_ACCESS(nvp, VEXEC, 0, u.u_cred)) != 0
		      || nvp->v_type != VREG
		      || (vattr.va_mode & (VEXEC|(VEXEC>>3)|(VEXEC>>6))) == 0){
			error = ELIBACC;
			VN_RELE(nvp);
			goto bad;
		}

		struct_zero(&lhda, sizeof(lhda));
		lhda.vp = nvp;
		lhda.vnsize = vattr.va_size;
		lhda.nomap = nvp->v_flag & VNOMAP;


		error = getcoffhead(nvp, dat, execsz, &lhda, (struct uarg *) 0);
		exhd_release(&lhda);
		if (error) {
			if (error != ENOMEM)
				error = ELIBBAD;
			VN_RELE(nvp);
			goto bad;
		}

		if (dat->ux_mag != 0443) {
			error = ELIBBAD;
			VN_RELE(nvp);
			goto bad;
		}

		++dat;
		++edp->ux_nshlibs;
		++n;
	}

	return 0;

bad:
	coffexec_err(dat_start, (long) n);
	return error;
}


void
coffexec_err(shlb_data, n)
	register struct exdata *shlb_data;
	register long n;
{
	for (; n > 0; --n, ++shlb_data)
		VN_RELE(shlb_data->vp);
}

coffcore(vp, pp, credp, rlimit, sig)
	vnode_t *vp;
	proc_t *pp;
	struct cred *credp;
	rlim_t rlimit;
	int sig;
{
	struct user *up = PTOU(pp);
	off_t offset;
	caddr_t base;
	int count, error;

	up->u_sysabort = sig;	/* record reason for core */

	/*
	 * Put the text, data and stack sizes (in pages)
	 * into the u-block for the dump.
	 */
	up->u_tsize = btoc(up->u_exdata.ux_tsize);
	up->u_dsize = btoc(up->u_exdata.ux_dsize + up->u_exdata.ux_bsize + pp->p_brksize);
	up->u_ssize = btoc(pp->p_stksize);


	/*
	 * Check the sizes against the current ulimit and
	 * don't write a file bigger than ulimit.  If we
	 * can't write everything, we would prefer to
	 * write the stack and not the data rather than
	 * the other way around.
	*/

	if (ctob(pp->p_usize + up->u_dsize + up->u_ssize) > rlimit) {
		up->u_dsize = 0;
		if (ctob(pp->p_usize + up->u_ssize) > rlimit)
			up->u_ssize = 0;
	}

	error = vn_rdwr(UIO_WRITE, vp, (caddr_t)up, ctob(pp->p_usize),
	  (off_t) 0, UIO_SYSSPACE, 0,
	  rlimit, credp, (int *)NULL);

	offset = ctob(pp->p_usize);

	/* Write the data and stack to the dump file. */
	
	if (error == 0 && up->u_dsize) {
		base = (caddr_t)up->u_exdata.ux_datorg;
		count = ctob(up->u_dsize) - PAGOFF(base);
		error = core_seg(pp, vp, offset, base, count, rlimit, credp);
		offset += ctob(btoc(count));
	}

	/*
	 *	Caution: user stack grows downwards, and the stack base must be
	 *		 page aligned - for old versions of sdb to run.
	 */
	if (error == 0 && up->u_ssize)
		error = core_seg(pp, vp, offset,
#ifdef i386
			/* Stack grows downwards on 386 */
			(caddr_t)(ctob(btoc(pp->p_stkbase)) - ctob(up->u_ssize)),
#else
			p->p_stkbase,
#endif
			ctob(up->u_ssize), rlimit, credp);

	u.u_sysabort = 0;

	return error;
}
