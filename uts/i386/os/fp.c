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

#ident	"@(#)kern-os:fp.c	1.3.1.2"

/*
 * Dual-Mode Floating Point support:
 * Copyright (c) 1989 Phoenix Technologies Ltd.
 * All Rights Reserved
*/

/*
** routines that deal with floating point
*/

#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/sysmacros.h"
#include	"sys/systm.h"
#include	"sys/dir.h"
#include	"sys/signal.h"
#include	"sys/cred.h"
#include	"sys/user.h"
#include	"sys/errno.h"
#include	"sys/trap.h"
#include	"sys/seg.h"
#include	"sys/sysinfo.h"
#include        "sys/immu.h"
#include	"sys/proc.h"
#include	"sys/fp.h"
#include        "sys/cmn_err.h"
#include        "sys/reg.h"
#include        "sys/tss.h"
#include	"sys/vnode.h"
#include	"sys/fstyp.h"
#include	"sys/conf.h"
#include	"sys/debug.h"
#include	"../fp/sizes.h"
#include	"sys/pathname.h"
#include	"sys/uio.h"
#include	"sys/exec.h"
#ifdef VPIX
#include	"sys/v86.h"
#endif
#ifdef WEITEK
#include "sys/weitek.h"
#endif
#include "sys/elf.h"
#ifdef AT386	/* 16 MB support */
#include	"sys/kmem.h"
#endif	/* 16 MB support */

#include	"vm/faultcatch.h"

char	fp_vers;	/* which format of emulator (elf, coff or x.out) */
char    fp_kind;        /* kind of floating point hardware or emulation */
struct proc *fp_proc;   /* owner of floating point extension            */
int     finitstate;     /* control word temporary during initialization */
int     fpsw;           /* status word temporary                        */

extern int getcoffhead();
extern int getelfhead();

#ifdef VPIX
extern char v86procflag;
#endif


/*
** This is a workaround.
**	The interim compiler with the register-model floating point compilation
** (which requires that the fp stack be initialized)
** generates an unresolved reference to the following symbol so that user
** programs will grab a floating point initialization routine out of libc.
** The kernel doesn't want such user-level support. This define might
** be removed after the compiler does floating point stack-model computation.
*/
long ___Fp_Used;

/* This is a workaround for the 80386 B1 stepping bug errata #21
 * which needs either a special PAL on the motherboard or
 * 0x80000000 set in cr3 to prevent a hang in the kernel
 * while trying to save the 387 state.
 * It is conditionally initialized during machine initialization.
 */

paddr_t fp387cr3 = 0;

/*
** fpnoextflt
**      handle a processor extension not present fault
**
**      This fault occurs under two conditions:
**
**      1)  There is a floating point processor extension, and a floating
**      point instruction is encountered when the task-switched (TS) bit
**      is set.  We save and restore floating point states.
**
**      2)  We are doing floating point emulation, and a floating point
**      instruction is encountered.  We emulate the instruction.
**
**	When emulating, this fault traps directly to the emulator and
**	never comes in here.  The emulator may, however, realize that fp
**	has not been initialized (u_fpvalid == 0).  In this case, it will
**	software trap into this code to do the fpinit().
**	THIS IS THE ONLY SITUATION IN WHICH fpnoextflt() IS EXECUTED WITH
**	FP_SW SET.
**	
**	We're trying to establish an invariant for emulated processes:
**	u_fpvalid == 0 until the first fp instruction.  Before the first fp
**	instruction, fpinit() is executed.  After fpinit(), u_fpvalid == 1.
**	And fpinit() is executed only once for any process (exclusive of
**	fpinits for fp inside of signal handlers).
*/
fpnoextflt(r0ptr)
	int    *r0ptr;         /* pointer to registers on stack */
{
	unsigned int ts;        /* saved task-switched bit */
	flags_t *flags;         /* pointer to saved flags on stack */
	label_t fpsav;          /* saved state for longjmp */
	long vmflag;			/* returning to v86 mode? */
#ifdef VPIX
	v86_t	*v86p;
#endif

	asm(" clts" );		/* clear TS bit in CR0  */

	vmflag = r0ptr[EFL] & PS_VM;

	/* check for no floating point support */
	if(fp_kind == FP_NO) {
		/*
		 * If we have neither a processor extension nor
		 * an emulator, kill the process OR panic if the kernel.
		 */
#ifdef VPIX
		if (vmflag) {
			v86setint(u.u_procp->p_v86, V86VI_COPROC);
			return;
		} else if (USERMODE(r0ptr[CS])) {
#else
		if (vmflag || USERMODE(r0ptr[CS])) {
#endif
			psignal( u.u_procp, SIGFPE );
			return;
		} else
			cmn_err(CE_PANIC,
				"NOEXTFLT in kernel mode, no FP support");
	}

#ifdef VPIX
	v86p = (v86_t *)u.u_procp->p_v86;
#endif

	/*
	 * If the current process does not own the processor extension,
	 * save the fp state in the owner's user structure,
	 * and restore or establish the current process's fp state.
	*/
	if(fp_proc != u.u_procp) {
		if(fp_proc && (fp_kind != FP_SW))
			fpsave();
		fp_proc = u.u_procp;
		/*
		 * If the current process' state is valid, restore
		 * it. Otherwise, this is the first time this
		 * process has executed a fp instruction,
		 * so initialize the fp unit for it.
		*/
#ifdef VPIX
		if((u.u_fpvalid && !vmflag) || (v86p && v86p->vp_fpvalid && vmflag))
#else
		if(u.u_fpvalid)
#endif
			/* never executed for FP_SW since fpvalid == 0 */
			fprestore(vmflag);
		else
			fpinit();
#ifdef VPIX
	} else
	/*
	 * The current process owns the floating point unit.  If
	 * it is a dual-mode process, determine if we are switching
	 * between components of the process.
	*/
	if(v86procflag) {
		/* v86 -> v86 = no work */
		if((v86p->vp_fpproc == V86FPP_V86LAST) && vmflag)
			return;

		/* ECT -> ECT = no work */
		if((v86p->vp_fpproc == V86FPP_ECTLAST) && !vmflag)
			return;

		/* Switching component use of floating point */
		fpsave();
		if((vmflag && v86p->vp_fpvalid == V86FPV_VALID) ||
			(!vmflag && u.u_fpvalid))
			fprestore(vmflag);

		/* Set vp_fpproc to new process mode */
		v86p->vp_fpproc = vmflag ? V86FPP_V86LAST : V86FPP_ECTLAST;
#endif
	}

/*	if ( fp_kind == FP_SW ) {       * software emulation *
*
*		 *  this has been commented out because
*		 *  the only way we get in here when software emulating
*		 *  is when the emulator decides that it hasn't yet been
*		 *  initialized (u_fpvalid == 0) and traps in here to
*		 *  get things rolling.
*		 *  nothing needs be done except the fpinit() above, so
*		 *  this block is obsolete.
*
*		 * call setjmp to establish a state to longjmp back to
*		 * if there is an uncorrectable fault in the emulator
*		 *
*		if (setjmp(fpsav)) {
*			psignal( fp_proc, SIGSEGV );
*			return;
*		}
*
*		e80387( r0ptr, fpsav );        * call the emulator *
*
*		*
*		** if the user mode trace bit is set, we must notify
*		** the parent ( by signalling the child )
*		*
*		flags = (flags_t *)&r0ptr[EFL];
*		if ( USERMODE(r0ptr[CS]) && flags->fl_tf ) {
*			flags->fl_tf = 0;       * turn off trace bit *
*			psignal( fp_proc, SIGTRAP );
*		}
*
*	}
*/
}

/*
** fpextovrflt
**      handle a processor extension overrun fault
*/
fpextovrflt(r0ptr)
	int    *r0ptr;         /* pointer to registers on stack */
{
	ASSERT(fp_kind != FP_SW);

	printf("\nEXTOVRFLT: eip = 0x%x\n", r0ptr[EIP]);

	asm( "  clts    " );    /* clear TS bit in CR0  */

	if ( fp_kind == FP_NO ) {
		printf("\nEXTOVRFLT WARNING: no FP support\n");
		return;
	}

	/* re-initialize the extension */
	fpinit();

	/* send segmentation violation error signal to the process
	 * that owns the processor extension
	 */
	if (fp_proc)
		psignal( fp_proc, SIGSEGV );
	else {
		printf("\nEXTOVRFLT WARNING: no FP process\n");
		return;
	}

	/* if the current process is not the process that owns the extension,
	 * set the TS bit.
	 */
	if ( fp_proc != u.u_procp )
		setts();
}

/*
** fpexterrflt
**      handle a processor extension error fault
*/
fpexterrflt()
{
	user_t	*nu;

	asm(" clts");		/* clear TS bit in CR0  */

	if(fp_kind == FP_NO) {
		cmn_err(CE_WARN, "EXTERRFLT: no FP support");
		return;
	}

	fpsw = 0;		/* clear temporary for status word */

	asm(" fnstsw  fpsw");	/* store co-processor status word */
	asm(" fnclex");		/* clear processor exceptions */

	/*
	 * if the current process is not the process that owns the extension,
	 * set the TS bit.
	*/
	if(fp_kind == FP_SW)
		fp_proc = u.u_procp;
	if(fp_proc != u.u_procp)
		setts();

	if(fp_proc == (proc_t *)NULL) {
		cmn_err(CE_WARN, "EXTERRFLT: no FP process");
		return;
	}

#ifdef VPIX
	/*
	 * This is not a dual-mode process OR it is and the ECT
	 * is using the floating point.
	*/
	if(!fp_proc->p_v86 || (fp_proc->p_v86->vp_fpproc == V86FPP_ECTLAST)) {
#endif

		/*
		 * Send a floating-point error signal to the process
		 * that owns the processor extension.
		*/
		psignal(fp_proc, SIGFPE);

		/* Save the status word in owner's u block. */
		CATCH_FAULTS(CATCH_SEGU_FAULT)
			PTOU(fp_proc)->u_fps.u_fpstate.status = fpsw;
		END_CATCH();
#ifdef VPIX
	} else
	/*
	 * This is a dual-mode process and the virtual 86 task
	 * is using the floating point.
	*/
	if(fp_proc->p_v86 && (fp_proc->p_v86->vp_fpproc == V86FPP_V86LAST)) {

		/*
		 * Send a psuedorupt to the virtual 86 task,
		 * and store the floating point unit status
		 * in vp_fpu.vp_fpstate.status.
		*/
		v86setint(fp_proc->p_v86, V86VI_COPROC);
		fp_proc->p_v86->vp_fpu.vp_fpstate.status = fpsw;
	}
#endif
}

/* XENIX Support */
/*
 * fpukill - used by x.out floating point emulator to send a FPE signal
 *	to user processes when there is an exception.
 *
 *	When we get here the emulator has cleaned up its stack and is
 *	ready to return to the user program. It has pushed the user EFL, CS, 
 *	EIP on the stack and calls the system, by doing an "int".
 *
 *	We copy the real user values off the stack, set the user stack pointer
 *      and other registers back to the correct value and return the signal
 *	to send to the process.
 */

int
fpukill(regptr)
int *regptr;
{
	struct saveinfo {
		unsigned int savedEIP;
		unsigned int savedCS;
		unsigned int savedEFL;
	} ptr;
	unsigned int off;

	if ( (regptr[CS] & 0xffff) != FPESEL ) {
		cmn_err(CE_WARN,
			"Coprocessor error not from emulator, CS=0x%x\n",
			regptr[CS] & 0xffff);
		return(0);
	}
		
	/* copy registers from user space */
	off = getdscraddr(regptr[SS]) - getdscraddr(USER_DS) + regptr[UESP];

	if (copyin((caddr_t)off,(caddr_t)&ptr, sizeof(struct saveinfo)) == -1) {
		return(SIGSEGV);
	}

	/* stuff back real user registers */
	regptr[EIP] = ptr.savedEIP;
	regptr[CS] = ptr.savedCS;
	regptr[EFL] = ptr.savedEFL;

	/* cleanup user stack */
	regptr[UESP] = regptr[UESP] + (sizeof(struct saveinfo));

	/* send him signal */
	return(SIGFPE);
}
/* End XENIX Support */

#ifdef AT386
/*
** fpintr
**      handle a processor extension error interrupt on the AT386
**
**      this comes in on line 5 of the slave PIC at SPL1
*/
fpintr()
{
	oem_fclex();		/* Clear NDP BUSY latch */

#ifdef WEITEK
	if (weitek_kind != WEITEK_NO) {
		/*
		 * wtl 1167 and 80387 errors are or'd and the result
		 * is sent to the PIC.  therefore, we
		 * need to check whether this interrupt is from
		 * weitek or 387
		 * we'll do this by looking at the 387 status reg.
		 */
		int stat387;

		if (fp_kind == FP_NO) {
			/* with no 387 support, assume weitek */
			weitek_reset_intr();
			weitintr(0);
			return;
		}
		stat387 = get87();
		if ((stat387 & FPS_ES) == 0) {	/* no 387 error */
			weitek_reset_intr();
			weitintr(0);
			return;
		}
	}
#endif
	fpexterrflt();
}
#endif

/*
** fpinit
**	initialize the floating point unit for this user
*/
fpinit()
{
	int i;

	/* to keep the floating point emulator from looping, we
	 * set fpvalid here when emulating.  remember that the emulator
	 * was keying on fpvalid == 0 to trap to us in the first place.
	 */
	if (fp_kind == FP_SW)
		u.u_fpvalid = 1;

	if ( fp_kind == FP_NO )
		cmn_err(CE_PANIC, "fpinit: no FP support");

	asm( "  fninit" );

	/*
	** must allow invalid operation, zero divide, and
	** overflow interrupt conditions and change to use
	** long real precision
	*/
	asm( "  fstcw   finitstate" );

	finitstate &= ~( FPINV | FPZDIV | FPOVR | FPPC );
	finitstate |= ( FPSIG53 | FPIC );

	asm( "  fldcw   finitstate" );

	/* to fill FP stack with zeros as before, un-comment the following: */
	/*
	for( i=0; i<8; i++) {
		asm( "  fldz" );
	}
	*/
}


/*
** fpsave
**      save the floating point state into fp_proc's appropriate
**	storage area.
**
**      fp_proc must be valid!
*/
fpsave( )
{
	struct user	*fp_u;
#ifdef VPIX
	v86_t	*v86p;
#endif

	if(fp_proc == NULL)
		cmn_err(CE_PANIC, "fpsave: no fp_proc");

#ifdef VPIX
	/*
	 * The process that owns the floating point unit
	 * is not a dual-mode process OR it is and the ECT
	 * was the last user of floating point.
	*/
	if(!(v86p = fp_proc->p_v86) || v86p->vp_fpproc == V86FPP_ECTLAST) {
#endif
		/* Get access to the extension owner's u block. */
		fp_u = PTOU(fp_proc);

		CATCH_FAULTS(CATCH_SEGU_FAULT) {
			/* if chip present, save its state */
			if (fp_kind & FP_HW)
				savefp(fp_u->u_fps.u_fpstate.state);

			/* say that the saved state is valid */
			fp_u->u_fpvalid = 1;
		}
		END_CATCH();
#ifdef VPIX
	} else
	/*
	 * The process that owns the floating point unit
	 * is a dual-mode process and the virtual 86 task
	 * was the last user of floating point.
	*/
	if(v86p && v86p->vp_fpproc == V86FPP_V86LAST) {

		/* if chip present, save its state */
		if(fp_kind & FP_HW)
			savefp(v86p->vp_fpu.vp_fpstate.state);

		/* say that the saved state is valid */
		v86p->vp_fpvalid = V86FPV_VALID;
	}
#endif

	/* Now nobody owns the fp unit */
	fp_proc = 0;
}

/*
** fprestore
**	restore the floating point state from the current
**	appropriate storage area.
*/
fprestore(vmflag)
int	vmflag;		/* Are we returning to a virtual 86 task? */
{
#ifdef VPIX
	v86_t	*v86p;
#endif

#ifdef VPIX
	/*
	 * The process that needs the floating point unit
	 * is not a dual-mode process OR it is and the ECT
	 * is going to use the floating point unit.
	*/
	if(!v86procflag || !vmflag) {
#endif
		/* if chip present, restore its state */
		if(fp_kind & FP_HW)
			restorefp(u.u_fps.u_fpstate.state);

		/* say that the saved state is not valid */
		u.u_fpvalid = 0;
#ifdef VPIX
	} else
	/*
	 * The process that needs the floating point unit
	 * is a dual-mode process and the virtual 86 task
	 * was the last user of floating point.
	*/
	if(v86procflag && vmflag) {
		v86p = u.u_procp->p_v86;

		/* if chip present, restore its state */
		if(fp_kind & FP_HW)
			restorefp(v86p->vp_fpu.vp_fpstate.state);

		/* say that the saved state is not valid */
		v86p->vp_fpvalid = V86FPV_NOTVALID;
	}
#endif
}


/*
** fpksave
**      Save the floating point state into fp_proc's user structure,
**      and re-initialize for kernel use.  Process must not sleep
**      before calling fpkreset().  Called by Weitek emulator.
*/
fpksave( )
{
	if ( fp_proc )
		fpsave( );
	asm( "  clts    " );    /* clear TS bit in CR0  */
	fpinit();
}


/*
** fpkreset
**      Reset after a fpksave().
**      Called by Weitek emulator.
*/
fpkreset( )
{
	fp_proc = 0;
	setts();
}

/*
** savefp
**      asm code to actually save the fp state.
**      called from fpsave()
*/
savefp( addr )
	int *addr;
{
	asm( "  clts                    " );    /* clear TS bit in CR0  */
	asm( "  movl    8(%ebp), %eax   " );    /* load save address    */
	asm( "  fnsave  (%eax)          " );    /* save state           */
	asm( "  fwait                   " );    /* wait for completeion */
}

/*
** restorefp
**      asm code to actually save the fp state.
**      called from fprestore()
*/
restorefp( addr )
	int *addr;
{
	asm( "  clts                    " );    /* clear TS bit in CR0  */
	asm( "  movl    8(%ebp), %eax   " );    /* load restore address */
	asm( "  frstor  (%eax)          " );    /* restore state        */
	asm( "  fwait                   " );    /* wait for completeion */
}

/*
** setts
**      asm code to set the ts bit in CR0
*/
setts()
{
	asm( "  movl    %cr0, %eax      " );    /* get CR0              */
	asm( "  orl     $0x08, %eax     " );    /* OR in the TS bit     */
	asm( "  movl    %eax, %cr0      " );    /* load CR0             */
}


#define EMULPATH "/etc/emulator"
static char *filename = EMULPATH;
char *fpeloc;

# define  IAPX386MAGIC	0514

#define   ELFMAGIC	0x457f
unsigned int	EM80387 = 0;

/*
** fpeinit
**	fpeinit is called from main() to read in the floating point
**	emulator and set up the interrupt vectors for emulation, if
**	emulation is necessary
*/
void
fpeinit()
{
	extern struct seg_desc gdt[];
	extern struct gate_desc idt[];

	struct dscr *gdte;
	struct gdscr *idte;
	exhda_t exhda;
	struct exdata em_data;
	int    error;
	int    resid;
	struct vattr	vattr;
	struct pathname pn;
	char	*mcp;

	/*
	 * do nothing if we have already decided that there will be no fp
	 * also do nothing if we have hardware
	 */
	if (fp_kind == FP_NO) {
		cmn_err(CE_WARN, "No floating point is available");
		return;
	}
	if (fp_kind != FP_SW) {
		return;
	}
	struct_zero((caddr_t)&exhda, sizeof(exhda));

	/*
	 * Lookup path name and remember last component 
	 */
	if (pn_get(filename, UIO_SYSSPACE, &pn) != 0)
		return;
	error = lookuppn(&pn, FOLLOW, NULLVPP, &exhda.vp);
	pn_free(&pn);	
	if (error) {
		cmn_err(CE_WARN, "Cannot load floating point emulator");
		fp_kind = FP_NO;
		return;
	}

	if (exhda.vp->v_type != VREG) {
		cmn_err(CE_WARN, "Cannot load floating point emulator");
bad1:
		VN_RELE(exhda.vp);
		fp_kind = FP_NO;
		return;
	}
	
	vattr.va_mask = AT_SIZE;
	if (VOP_GETATTR(exhda.vp, &vattr, 0, u.u_cred) != 0) {
		cmn_err(CE_WARN, "Cannot read FP emulator attributes");
		goto bad1;
	}

	if ((exhda.vnsize = vattr.va_size) == 0L) {
		cmn_err(CE_WARN, "Zero length FP emulator file");
		goto bad1;
	}
	exhda.nomap = exhda.vp->v_flag & VNOMAP;

	/*
	 * check the first two bytes of the file for an a.out magic
	 * number.  if not found we assume this is an emulator with
	 * no header (the XENIX fp emulator is shipped w/o a header).
	 */
	if (exhd_getmap(&exhda, (off_t)0, 2, EXHD_NOALIGN, (caddr_t)&mcp) != 0) {
		cmn_err(CE_WARN, "Can't read magic number in emulator file");
bad2:
		exhd_release(&exhda);
		goto bad1;
	}

	/*
	 * get some memory for the emulator, and set up a gdt entry for it.
	 * the gdt entry is already set, except for base and limit.
	 */
	fpeloc = (char *)sptalloc(btoc(vattr.va_size), PG_V|PG_RW|PG_US, 0,
#ifdef AT386	/* 16 MB support */
		KM_NO_DMA
#else
		0
#endif	/* 16 MB support */
	);
	ASSERT(fpeloc != (char *)NULL);

	setdscrbase(gdt + seltoi(FPESEL), (int)fpeloc);
	setdscrlim(gdt + seltoi(FPESEL), vattr.va_size - 1);
	gdte = (struct dscr *)gdt + seltoi(FPESEL);
	gdte->a_acc0007 |= SEG_CONFORM;		/* make this a conforming
						   segment so it will work
						   for both user and kernel */

	/* Emulator is in ELF, XOUT, or COFF Format? */
	if ( * ((short *) mcp) == ELFMAGIC) {	/* IS in Elf format */
		Elf32_Ehdr	*ehdrp;			/* Elf header */
		Elf32_Phdr	*phdr;			/* Elf program header */
		caddr_t		phdrbase = 0;
		long		phdrsize = 0;
		int i;
		int is_elf_text = 0;			/* Must be only Text */

#ifdef DEBUG
		cmn_err(CE_CONT,"FP Emulator is in ELF format\n");
#endif
		/* Get the Elf header */
		if (getelfhead(&ehdrp, &phdrbase, &phdrsize, &exhda) != 0)
			goto elf_bad;

		/* Scan all valid program header(s) to find file offset and
		 * and size of Elf Format Emulator text.
		 */
		for (i = 0; i < (int) ehdrp->e_phnum; i++) {

			phdr = (Elf32_Phdr *)(phdrbase + (ehdrp->e_phentsize * i));

			switch (phdr->p_type) {	/* Program header type? */

				case PT_LOAD :	/* Should be ELF fp text */

					/*  Text should be executable but NOT
					 *  writeable
					 */
					if ((phdr->p_flags & PF_X) &&
						! (phdr->p_flags & PF_W)) {


						/* Set File offset, File Size
						 * and entry location
						 */
						em_data.ux_toffset = phdr->p_offset;
						em_data.ux_tsize = phdr->p_filesz;
						em_data.ux_entloc =
							(caddr_t)ehdrp->e_entry;

						/*  Did find Emulator Text
						 */
						is_elf_text = 1;
					}
					break;
				default:
					break;
			}
			
		}

		if (!is_elf_text)		/* No Emulator Text? */
			goto elf_bad;

		fp_vers = FP_ELF;		/* Same as FP_COFF */
		goto read_fp_text;		/* Read in the Emulator Text */

elf_bad:
		cmn_err(CE_WARN, "Cannot read emulator ELF file header");
bad3:
		sptfree(fpeloc, btoc(vattr.va_size), 1);
		gdte->a_acc0007 = 0;	/* clear the present bit
					   (among others) */
		goto bad2;
	}
	else if ( *((short *)mcp) != IAPX386MAGIC )
	{
		fp_vers = FP_XOUT;
		em_data.ux_tsize = vattr.va_size;
		em_data.ux_toffset = 0;
		em_data.ux_entloc = (caddr_t)EMUL_START;
	}
	else {
		long	execsz = 0;
		/*
		 * read the emulator a.out header
		 * this code assumes the fp emulator is in coff format.
		 * if this assumption is incorrect, the code must be
		 * changed to handle additional format. - XXX
		 */
		if (getcoffhead(exhda.vp, &em_data, &execsz, &exhda,
						(struct uarg *)NULL) != 0) {
			cmn_err(CE_WARN, "Cannot read emulator COFF file header");
			goto bad3;
		}
		fp_vers = FP_COFF;
	}

read_fp_text:
	exhd_release(&exhda);
	/*
	 * read the emulator
	 * this thing's all text, so all we have to concern ourselves with
	 * is the text section.
	 */
	error = vn_rdwr(UIO_READ, exhda.vp, fpeloc, em_data.ux_tsize, 
		(off_t)em_data.ux_toffset, UIO_SYSSPACE, 
	        0, (ulong) 0, (struct cred *)NULL, &resid);
		    
	if (error != 0 || resid != 0) {
		cmn_err(CE_WARN, "Error reading FP emulator file");
		goto bad3;
	}

	VN_RELE(exhda.vp);

	/*
	 * now, if there's no fp hardware, set up the idt to branch to
	 * the emulator rather than the kernel
	 */
	if (fp_kind == FP_SW) {
		idte = (struct gdscr *)idt + NOEXTFLT;
		idte->gd_off0015 = (ushort)em_data.ux_entloc;
		idte->gd_off1631 = ((u_int)em_data.ux_entloc >> 16) & 0x0000FFFF;
		idte->gd_selector = FPESEL;
		idte->gd_acc0007 = GATE_UACC | GATE_386TRP;
		EM80387 = (unsigned int) em_data.ux_entloc;
	}
}



/*
**  fpeclean: remove the fp emulator from the user stack in preparation
**	for delivery of a fp-related signal.
**	
**	At the point when fpeclean is invoked, the user
**	stack looks like this:
**	
**		old esp ->
**				old eflags
**				old cs
**				old eip
**				0
**				7
**				<8 registers>
**				ds
**				es
**				fs
**				gs
**				___
**				|
**				| global reentrant segment
**				| (220 bytes)
**				|
**		current ebp ->	---
**				.
**				.
**		current esp ->	.
**	
**	"old" means roughly "when the fp instrucion was invoked."
**	"current" means "in u.u_ar0[]"
*/
fpeclean()
{
	struct intframe {
		ulong	ueip;
		ulong	ucs;
		ulong	uefl;
	} userframe;
	struct reglist {
		ulong gs;
		ulong fs;
		ulong es;
		ulong ds;
		ulong di;
		ulong si;
		ulong bp;
		ulong sp;
		ulong bx;
		ulong dx;
		ulong cx;
		ulong ax;
	} savedregs;
	u_int base;

	ASSERT(fp_kind == FP_SW);

	/* copy saved registers from user stack */

	base = u.u_ar0[EBP];
	/* if the stack segment selector is not the 386 user data selector,
	 * convert the SS:BP to the equivalent 386 virtual address.
	 */
	if (u.u_ar0[SS] != USER_DS) {
		char *dp;	/* pointer to stack descriptor in LDT */

		dp = (char *)
		  (((struct dscr *)(u.u_procp->p_ldt)) + seltoi(u.u_ar0[SS]));
		base &= 0xFFFF;
		base += (dp[7] << 24) | (*(int *)&dp[2] & 0x00FFFFFF);
	}
	base += GRSL;		/* base is now addr of gs on user stack */

	if (copyin((caddr_t)base, (caddr_t)&savedregs, sizeof(savedregs)))
		return(SIGSEGV);

	/* copy registers from user stack */

	base += sizeof(savedregs) + 2*4;	/* addr of old eip */
	if (copyin((caddr_t)base, (caddr_t)&userframe, sizeof(userframe)))
		return(SIGSEGV);

	/* and fill this info into the frame for the current trap */

	u.u_ar0[GS] = savedregs.gs;
	u.u_ar0[FS] = savedregs.fs;
	u.u_ar0[ES] = savedregs.es;
	u.u_ar0[DS] = savedregs.ds;
	u.u_ar0[EDI] = savedregs.di;
	u.u_ar0[ESI] = savedregs.si;
	u.u_ar0[EBP] = savedregs.bp;
	u.u_ar0[EBX] = savedregs.bx;
	u.u_ar0[EDX] = savedregs.dx;
	u.u_ar0[ECX] = savedregs.cx;
	u.u_ar0[EAX] = savedregs.ax;

	u.u_ar0[EIP] = userframe.ueip;
	u.u_ar0[CS] = userframe.ucs;
	u.u_ar0[EFL] = userframe.uefl;

	/* remove the emulator from the user stack */

	u.u_ar0[UESP] = (int)(base + 3*4);

	return;
}
