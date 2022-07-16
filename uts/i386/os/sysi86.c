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

#ident	"@(#)kern-os:sysi86.c	1.3.2.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/systm.h"
#include "sys/sysi86.h"
#include "sys/utsname.h"
#include "sys/sysmacros.h"
#include "sys/boothdr.h"
#include "sys/uadmin.h"
#include "sys/map.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/swap.h"
#include "sys/var.h"
#include "sys/tuneable.h"
#include "sys/rtc.h"
#include "sys/fp.h"
#include "sys/seg.h"
#include "sys/systm.h"
#include "sys/ioctl.h"  /* for ioctl_arg definition */
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/inline.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/fstyp.h"
#include "sys/uio.h"
#include "sys/mman.h"
#include "sys/bootinfo.h"	/* for ID byte */
#include "vm/seg.h"
#include "vm/vm_hat.h"
#include "vm/as.h"

#include "sys/xdebug.h"
#ifdef WEITEK
#include "sys/weitek.h"
#endif

#ifdef KPERF
#include "sys/file.h"
#include "sys/disp.h"

int kpftraceflg;
int kpchildslp;
#endif /* KPERF */

#if DEBUG
#define DEBUGF(x) printf x
/* usage DEBUGF(("format",arg[,arg ...]))  */
#else
#define DEBUGF(x)
#endif /* DEBUG */

extern int maxmem;                      /* size of memory in clicks     */
int dbug;
time_t localtime_cor = 0;		/* Local time correction in secs */

#ifdef VPIX
int	vpixenable = 1;			/* Flags to enable or disable VP/ix */
#endif /* VPIX */
#ifdef MERGE386
int	merge386enable = 0;		/* and Merge386 for runtime checking */
#endif /* MERGE386 */

/*
 *      sysi86 System Call
 */

struct sysi86 {
	int	cmd;
	int	arg1;
	int	arg2;
	int	arg3;
};
struct sysi86a {
	short    cmd;           /* these can be just about anything */
	union ioctl_arg arg;    /* use this for all new commands!   */
	long     arg2, arg3;    /* backward compatability           */
};
#define arg1 arg.larg

#ifdef KPERF
asm int 
geteip()
{
	leal 0(%esp), %eax
}
#endif /* KPERF */

int
sysi86(uap, rvp)
	register struct sysi86a *uap;
	rval_t *rvp;
{
	int	error = 0;

	register int	idx;
	register int	c;

	struct rtc_t	clkx;
	char    sysnamex[sizeof(utsname.sysname)];

	extern time_t	time;
	extern timestruc_t hrestime;
struct bcd_tm {
	unsigned char unit_sec;
	unsigned char ten_sec;
	unsigned char unit_min;
	unsigned char ten_min;
	unsigned char unit_hr;
	unsigned char ten_hr;
	unsigned char unit_day;
	unsigned char ten_day;
	unsigned char unit_mon;
	unsigned char ten_mon;
	unsigned char unit_yr;
	unsigned char ten_yr;
	unsigned char llyr;
};

/*	DEBUGF(("Sysi86 entered! cmd = %d\n",uap->cmd));*/

	switch ((short)uap->cmd) {
	case 23:
		switch ((short)uap->arg1) {
			case 0:
				break;
			case 1:
				dbug = uap->arg2;
				break;
			default:
				error = EINVAL;
		}
		rvp->r_val1 = dbug;
		break;

	/*
 	 * Copy a string from a given kernel address to a user buffer. Uses
	 * copyout() for each byte to trap bad kernel addresses while watching
	 * for the null terminator.
	 */
	case SI86KSTR :
	{
		register char	*src;
		register char	*dst;
		register char	*dstlim;

		if (!suser(u.u_cred))
			return(EPERM);
		src = uap->arg.cparg;                   /*????*/
		dstlim = (dst = (char *) uap->arg2) + (unsigned int) uap->arg3;
		do {
			if (dst == dstlim || copyout(src, dst++, 1) < 0) {
				return(EINVAL);
			}
		} while (*src++);
		return (0) ;
	}

	case RTODC:	/* read TODC */
		if (!suser(u.u_cred))
			return(EPERM);
		if (rtodc(&clkx))
			error = ENXIO;
		else
		if (copyout((caddr_t) &clkx, (caddr_t) uap->arg1, sizeof(clkx)))
			error = EFAULT;
		break;

	case STIME:	/* set internal time, not hardware clock */
		if (!suser(u.u_cred))
			return(EPERM);
		time = hrestime.tv_sec = (time_t) uap->arg.larg;
		hrestime.tv_nsec = 0;
		break;
 
	case SETNAME:	/* rename the system */
		if (!suser(u.u_cred))
			return(EPERM);

		for (idx = 0;
		  (c = fubyte((caddr_t) uap->arg.cparg + idx)) > 0
		    && idx < sizeof(sysnamex) - 1;
		  ++idx)
			sysnamex[idx] = c;
		if (c) {
			error = c < 0 ? EFAULT : EINVAL;
			break;
		}
		sysnamex[idx] = '\0';
		for ( idx = 0; idx < sizeof(sysnamex); idx++) 
		      utsname.sysname[idx] = utsname.nodename[idx] = sysnamex[idx];
		/* XENIX Support */
		for ( idx = 0; idx < sizeof(xutsname.sysname); idx++) 
		      xutsname.sysname[idx] = xutsname.nodename[idx] = sysnamex[idx];
		/* End XENIX Support */
		break;

	/*  return the size of memory */
	case SI86MEM:
		rvp->r_val1 = ctob( physmem );
		break;


	/*	This function is just here for compatibility.
	**      It is really just a part of SI86SWPI.  This
	**	new function should be used for all new
	**	applications.
	*/

	case SI86SWAP:
	{
		swpi_t	swpbuf;

		swpbuf.si_cmd   = SI_ADD;
		swpbuf.si_buf   = uap->arg.cparg;
		swpbuf.si_swplo = uap->arg2;
		swpbuf.si_nblks = uap->arg3;
		swapfunc(&swpbuf);
		break;
	}

	/*	General interface for adding, deleting, or
	**	finding out about swap files.  See swap.h
	**	for a description of the argument.
	**              sys3b(SI86SWPI, arg_ptr);
	**/

	case SI86SWPI:
	{
		swpi_t	swpbuf;

		if ((copyin(uap->arg.cparg, (caddr_t)&swpbuf, sizeof(swpi_t)) ) < 0 )
			error = EFAULT;
		else
			swapfunc(&swpbuf);
		break;
	}

	/*      Transfer to software  debugger.  Debug levels
	**	may be set there and then we can return to run a
	**	test.
	*/

	case SI86TODEMON : {
		if (!suser(u.u_cred))
			return(EPERM);
		if (!si86_call_demon())
			error = EINVAL;
		break;
	}

	/*
	**      Tell a user what kind of Floating Point support we have.
	**      fp_kind (defined in fp.h) is returned in the low-order byte.
	**      If Weitek support is included, weitek_type (defined in
	**      weitek.h) is returned in the second byte.
	*/

	case SI86FPHW:
		c = fp_kind & 0xFF;
#ifdef WEITEK
		c |= ((weitek_kind & 0xFF) << 8);
#endif
		if ( suword( uap->arg.iparg, c )  == -1)
			error = EFAULT;
		break;

	/*
	**  Set a segment descriptor
	*/

	case SI86DSCR:
		error = setdscr(uap->arg.iparg);
		break;

	/*
	 * Read a user process u-block.
	 * XXX this interface should be moved someplace else.
	 */
	case RDUBLK:
	{
		register struct proc *p;
		caddr_t addr;
		int ocount;
		struct uio uio;
		struct iovec iov;

		if (uap->arg3 < 0) {
			error = EINVAL;
			break;
		}		
		if ((p = prfind(uap->arg1)) == NULL) {
			error = ESRCH;
			break;
		}
		if (p->p_stat == 0 || p->p_stat == SIDL
		  || p->p_stat == SZOMB) {
			error = EINVAL;
			break;
		}
		ocount = min(uap->arg3, ctob(USIZE));
		iov.iov_base = (caddr_t) uap->arg2;
		iov.iov_len = uio.uio_resid = ocount;
		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = 0;
		uio.uio_fmode = 0;
		uio.uio_segflg = UIO_USERSPACE;

		addr = (caddr_t)KUSER(p->p_segu);
		error = uiomove(addr, uio.uio_resid, UIO_READ, &uio);
		rvp->r_val1 = ocount - uio.uio_resid;
		break;
	}

#if     defined(VPIX)
	/* The SI86V86 subsystem call of the SYSI86 system call has     */
	/* sub system calls defined for it v86.h. The SI86V86 call      */
	/* is processed in the module v86.c.                            */

	case SI86V86:
		return v86syscall();		/* Process V86 system call */
#endif /* VPIX */

	/*
	**  Set the local time correction in secs (includes Daylight savings)
	*/

	case SI86SLTIME:
		if (suser(u.u_cred))
			localtime_cor = (time_t)(uap->arg1);
		else
			error = EACCES;	/* Should be EPERM, but leave it
					   as EACCES for backward
					   compatibility */
		break;

	/* NFA  system entry hook  */
	case SI86NFA:
		nfa_sys(uap,rvp);	/* nfa_sys finds its own way
					   around the u struct. added
					   for opennet nfa */
		break;
	/* End NFA */

	/* XENIX Support */
	case SI86BADVISE:

		/*
	 	 * Specify XENIX variant behavior.
	 	 */
		switch(uap->arg.iarg & 0xFF00) {
			case SI86B_GET:
				/* Return badvise bits */
				if (BADVISE_PRE_SV)
					rvp->r_val1 |= SI86B_PRE_SV;
				if (BADVISE_XOUT)
					rvp->r_val1 |= SI86B_XOUT;
				if (BADVISE_XSDSWTCH)
					rvp->r_val1 |= SI86B_XSDSWTCH;
				break;
			case SI86B_SET:
				/* Set badvise bits.
				 * We assume that if pre-System V behavior
				 * is specified, then the x.out behavior is
				 * implied (i.e., the caller gets the same
				 * behavior by specifying SI86B_PRE_SV alone
				 * as they get by specifying both SI86B_PRE_SV
				 * and SI86B_XOUT).
				 */ 
				if (uap->arg.iarg & SI86B_PRE_SV) 
					u.u_renv |= (UB_PRE_SV | UB_XOUT);
				else
				if (uap->arg.iarg & SI86B_XOUT) {
					u.u_renv |= UB_XOUT;
					u.u_renv &= ~UB_PRE_SV;
				}
				else
					u.u_renv &= ~(UB_PRE_SV | UB_XOUT);	
				if (uap->arg.iarg & SI86B_XSDSWTCH) {
					/* copy the "real" sd to the 286 copy */
					xsdswtch(1);
					u.u_renv |= UB_XSDSWTCH;
				}
				else {
					/* copy the 286 sd to the "real" copy */
					xsdswtch(0);
					u.u_renv &= ~UB_XSDSWTCH;
				}
				break;
			default:
				error = EINVAL;
				break;
		}
		break;
	case SI86SHRGN:
	{

		extern	xsd86shrgn();
		/*
		 * Enable/disable XENIX small model shared data context    
		 * switching support.  The 'cparg' argument is a pointer   
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


		xsd86shrgn(uap->arg.cparg);
		break;
	}
	case SI86SHFIL:
		error = mapfile(uap->arg.sparg, rvp);
		break;

	case SI86PCHRGN:
		error = chmfile(uap->arg.sparg, rvp);
		break;

	case SI86CHIDT:
		error = error = chidt(uap->arg.cparg);
		break;

	/* End XENIX Support */

	/* remove 286 emulator special read access */
	case SI86EMULRDA:
		u.u_renv &= ~UE_EMUL;
		break;

#ifdef MERGE386
	/*
	**  VM86 command for Merge 386 functions 
	*/
	case SI86VM86:
		if (merge386enable)
			error = vm86(uap->arg.iparg, u.u_cred, rvp);
		else
			error = EINVAL;
		break;
#endif /* MERGE386 */
#if MERGE386 || VPIX
	case SI86VMENABLE:
		if (!suser(u.u_cred)) 
			return(EPERM);
		switch (uap->arg1) {
		case 0:
#ifdef VPIX
			vpixenable = 1;
#endif /* VPIX */
#ifdef MERGE386
			merge386enable = 0;
#endif /* MERGE386 */
			break;
		case 1:
#ifdef VPIX
			vpixenable = 0;
#endif /* VPIX */
#ifdef MERGE386
			merge386enable = 1;
#endif /* MERGE386 */
			break;
		default:
			error = EINVAL;
			break;
		}
		break;
#endif	/* MERGE386 || VPIX */

#ifdef	KPERF
	/* synchronization between parent and child  
	*/
	case KPFCHILDSLP:
		if (!suser(u.u_cred))
			return(EPERM);
		if (kpchildslp == 0)
			sleep((caddr_t) &kpchildslp, PPIPE);
		break;

	/*	Turn the kernel performance measurement code on 
	*/

	case KPFTRON:
		if (!suser(u.u_cred))
			return(EPERM);

		takephase = 0;
		putphase = 0;
		numrccount = 0;
		outbuf = 0;
		pre_trace = 1;
		kpftraceflg = 0;
		kpchildslp = 0;
		/* DEBUG */
		break;

	/*	Wait for a buffer of kernel perf statistics to fill, and return
	**	the data to the user.  First record is number of records
	**	to be returned (maximum is NUMRC).
	**	Usage: sys3b( KPFTRON2, &buffer, sizeof(buffer))
	**	the following logic is used:
	**	1. kpftraceflg on  A. takephase = putphase, sleep waiting for a 
	**			   buffer to fill
	**			B. takephase !=putphase, go copy records
	**			   and takephase = takephase +1 %NUMPHASE
	** 	always check for abnormal conditions
	**      2. kpftraceflg off A. takephase = putphase, copy numrccount
	**			   of records, ( buffer may not be full)
	**			B. takephase != putphase , go copy records
	**			   and takephase = takephase +1 % NUMPHASE
	*/

	case KPFTRON2: 
		{
                register int *buffer = (int*)(uap->arg1);
		int *dataddr;
                register int size = uap->arg2;


		if (!suser(u.u_cred))
			return(EPERM);


		if ( size != (1+NUMRC)*sizeof(kernperf_t) ) {
			/* buffer too small even for size */
			cmn_err(CE_CONT, "sys3b, kpftron2, Buffer too small\n");
			u.u_error = EINVAL;
			break;
		}

		numrc = NUMRC;
		dataddr = (int*)&kpft[takephase*NUMRC];
		if (kpftraceflg == 1)  
			if (takephase == putphase )  {
				kpchildslp = 1;
				wakeup((caddr_t) &kpchildslp);
				sleep((caddr_t) &kpft[takephase*NUMRC],PPIPE);
			}
		 /* full buffer i.e. abnormal termination */
		if (outbuf == 1) {
			u.u_error = EINVAL;
			break;
		}
	
		/* tracing is off, here under normal conditions */
		if ((kpftraceflg  == 0 ) && (takephase == putphase))
			numrc = numrccount;
		copyrecords(dataddr,buffer,numrc);
		takephase = (takephase + 1) % NUMPHASE;
		break;
		}

	/*	Turn the kernel performance measurement off.
	*/

	case KPFTROFF: 
	{
		if (!suser(u.u_cred))
			return(EPERM);

		/* asm(" MOVAW 0(%pc),Kpc "); */
		Kpc = geteip();
		kperf_write(KPT_END,Kpc,curproc);
		pre_trace = 0;
		kpftraceflg = 0;
		wakeup((caddr_t) &kpft[takephase*NUMRC]);
		break;
	}
#endif	/* KPERF */

	case SI86LIMUSER:
		switch (uap->arg1) {
			case EUA_GET_LIM:
			case EUA_GET_CUR:
				break;
			case EUA_ADD_USER:
			case EUA_UUCP:
				if (!suser(u.u_cred))
					return(EPERM);
				break;
			default:
				return EINVAL;
		}
		rvp->r_val1 = enable_user_alloc(uap->arg1);
		break;

	case SI86RDID:	/* Read the ROM BIOS ID bit */
		if ((bootinfo.id[0] == 'I') && (bootinfo.id[1] == 'D') &&
		    (bootinfo.id[2] == 'N') && (bootinfo.id[3] == 'O'))
			rvp->r_val1 = bootinfo.id[4];
		else
			rvp->r_val1 = 0;
		break;

	case SI86RDBOOT: /* Bootable Non-SCSI Hard Disk? */
		if (bootinfo.hdparams[0].hdp_ncyl == 0)
			rvp->r_val1 = 0;
		else
			rvp->r_val1 = 1;
		break;

	default:
		error = EINVAL;
	}
	return(error);
}

si86_call_demon()
{
#ifndef	NODEBUGGER
	if (cdebugger != nullsys) {
		DEBUGF(("Calling from sysi86... "));
		(*cdebugger)(DR_SECURE_USER, NO_FRAME);
		return(1);
	}
#endif
	DEBUGF(("Debugger not installed\n"));
	return(0);
}

void
call_demon()
{
	calldebug();
}


/*
 *  SI86DSCR:
 *  Set a segment or gate descriptor.
 *  The following are accepted:
 *      executable and data segments in the LDT at DPL 3
 *      a call gate in the GDT at DPL 3 that points to a segment in the LDT
 *  The request structure is declared in sysi86.h.
*/

/* call gate structure */
struct cg {
	unsigned short off1; /* low order word of offset */
	unsigned short sel;  /* descriptor selector */
	unsigned char  cnt;  /* word count */
	unsigned char  acc1; /* access byte */
	unsigned short off2; /* high order word of offset */
};

extern struct seg_desc gdt[];

int
setdscr(ap)
int *ap;
{
	struct ssd ssd;         /* request structure buffer */
	u_short seli;  		/* selector index */
	struct dscr *dscrp;     /* descriptor pointer */
	struct cg *cgp;         /* call gate pointer */
	unsigned int	newsz;

	if ((copyin((caddr_t)ap, (caddr_t)&ssd, sizeof(struct ssd)) ) < 0 ) {
		return(EFAULT);
	}

	/* LDT segments: executable and data at DPL 3 only */

	if (ssd.sel & 4) {              /* test TI bit */
		/* check the selector index */
		seli = seltoi(ssd.sel);
		if ((seli <= (u_short)seltoi(USER_DS)) || (seli >= MAXLDTSZ))
			goto bad;
		if ((u_int)seli > u.u_ldtlimit) {
			newsz = btoc(u.u_procp->p_ldt - (addr_t)PTOU(u.u_procp) + (seli + 1) * sizeof(struct dscr));
		     	if (!segu_expand(newsz)) {
				return(ENOMEM);
			}
			u.u_ldtlimit = min(MAXLDTSZ,
				(ctob(u.u_procp->p_usize) -
				 (u.u_procp->p_ldt - (addr_t)PTOU(u.u_procp))) /
						sizeof(struct seg_desc)) - 1;

			setdscrlim(&u.u_ldt_desc,
				(u.u_ldtlimit+1)*sizeof(struct seg_desc) - 1);
			gdt[seltoi(LDTSEL)] = u.u_ldt_desc;

			loadldt(LDTSEL);
		}
		ASSERT(seli <= u.u_ldtlimit);
		dscrp = (struct dscr *)(u.u_procp->p_ldt) + seli;
		/* if acc1 is zero, clear the descriptor */
		if (! ssd.acc1) {
			((unsigned int *)dscrp)[0] = 0;
			((unsigned int *)dscrp)[1] = 0;
			return(0);
		}
		/* check segment type */
		if ((ssd.acc1 & 0xF0) != 0xF0)
			goto bad;
		/* set up the descriptor */
		setdscrbase(dscrp, ssd.bo);
		setdscrlim(dscrp, ssd.ls);
		setdscracc1(dscrp, ssd.acc1);
		setdscracc2(dscrp, ssd.acc2);
		/* flag the process as having a modified LDT */
		u.u_ldtmodified = 1;
	}

	/* GDT segment: call gate into LDT at DPL 3 only */

	else {
		seli = seltoi(ssd.sel);
		if (seli <= 25 || seli >= GDTSZ)
			goto bad;
		switch(seli) {
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 50:
			goto bad;
		default:
			break;
		}

		/* if acc1 is zero, clear the descriptor and U-struct */
		cgp = (struct cg *)gdt + seli;
		if (! ssd.acc1) {
			((unsigned int *)cgp)[0] = 0;
			((unsigned int *)cgp)[1] = 0;
			u.u_callgatep = 0;
			u.u_callgate[0] = 0;
			u.u_callgate[1] = 0;
			return(0);
		}

		/* check that a call gate does not already exist */
		if (u.u_callgatep != 0)
			goto bad;

		/* check that call gate points to an LDT descriptor */
		if (((ssd.acc1 & 0xF7) != 0xE4) || (! (ssd.ls & 4)))           /* LDT */
			goto bad;

		cgp->off1 = ssd.bo;
		cgp->sel  = ssd.ls;
		cgp->cnt  = ssd.acc2;
		cgp->acc1 = ssd.acc1;
		cgp->off2 = ((unsigned short *)&ssd.bo)[1];

		/* copy call gate and its pointer into the user structure */
		u.u_callgatep = (int *)cgp;
		u.u_callgate[0] = ((int *)cgp)[0];
		u.u_callgate[1] = ((int *)cgp)[1];
	}
	return(0);

bad:
	return(EINVAL);
}

/*
 *  SI86CHIDT:
 *  Revector int 0xf0 to int 0xff to a user routine.  In xenix 286
 *  binaries, these precede each floating point instruction.
 */
int
chidt(fun)
char *fun;
{
	register struct gdscr * idte;
	extern struct gdscr def_intf0;
	extern struct gate_desc idt[];

	idte = (struct gdscr *) u.u_fpintgate;

	if (fun == (char *) 0) {
		*idte = def_intf0;
	}
	/*
	 * verify the validity of the address in user space.  For now,
	 * we ignore the type of segment: text, data, ....
	 */
	else if ((as_segat(u.u_procp->p_as, fun)) != NULL) {
		idte->gd_off0015 = (ushort) fun;
		idte->gd_selector = (short) USER_CS;
		idte->gd_unused = 0;
		idte->gd_acc0007 = GATE_UACC|GATE_386TRP;
		idte->gd_off1631 = ((int) fun >> 16) & 0xFFFF;
	}
	else {
		return(EFAULT);
	}

	/*
	 * since int 0xf0, ... are embedded in user text,
	 * and occur synchronously, this is ok.
	 */
	idte = (struct gdscr *) idt + 0xf0;
	for (; idte <= (struct gdscr *) idt + 0xff; idte++) {
		*idte = * (struct gdscr *) u.u_fpintgate;
	}
	return(0);
}
/* #endif */


#ifdef	KPERF
/*	Copy kernel perf statistics from kernel buffer to
**	user buffer.  First record is number of records
**	copied.
*/

copyrecords(dataddr,buffer,numrcx)
int *dataddr, *buffer, numrcx;
{
	register int datalen;
	kernperf_t *bufx;

	datalen = numrcx * sizeof( kernperf_t);
	if (suword(buffer,numrcx) == -1) {
		u.u_error = EFAULT;
		return;
	}

	bufx = ( kernperf_t *) buffer;
	if ((copyout(dataddr,bufx+1,datalen)) == -1)
		u.u_error = EFAULT;
}
#endif	/* KPERF */
