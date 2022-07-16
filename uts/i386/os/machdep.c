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

#ident	"@(#)kern-os:machdep.c	1.3.2.1"

#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/proc.h"
#include "sys/signal.h"
#include "sys/siginfo.h"
#include "sys/map.h"
#include "sys/reg.h"
#include "sys/seg.h"
#include "sys/utsname.h"
#include "sys/acct.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/fstyp.h"
#include "sys/tss.h"
#include "sys/buf.h"
#ifdef VPIX
#include "sys/v86.h"
#endif
#include "sys/user.h"
#include "sys/fp.h"
#ifdef WEITEK
#include "sys/weitek.h"
#endif
#include "sys/debug.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/ucontext.h"
#include "sys/prsystm.h"
#include "sys/exec.h"

#include "vm/hat.h"
#include "vm/page.h"
#include "vm/seg.h"
#include "vm/seg_vn.h"

#ifdef VPIX
/*      The following flag is used to indicate whether the process
**      is a dual mode process (user mode 386 task OR virtual 8086
**      mode task).
*/

extern char	v86procflag;
#endif

extern int exec_initialstk;

int
extractarg(args)
	struct uarg	*args;
{
	/*
	 * Machine dependent: Depends on the arrangement of the user stack.
	 *			On 80x86 the userstack grows downwards toward
	 *			low addresses; the arrangement is:
	 *
	 * (low address).
	 * argc
	 * prefix ptrs (no NULL ptr)
	 * argv ptrs (with NULL ptr)
	 *	(old argv0 points to fname copy if prefix exits)
	 *	(last argv ptr points to fname copy if EMULATOR present)
	 * env ptrs (with NULL ptr)
	 * postfix values (put here later if they exist)
	 * prefix strings
	 * (fname if a prefix exists)
	 * argv strings
	 * (fname if EMULATOR exists)
	 * env strings
	 * (high address)
	 *
	 */

	int		error;
	u_int		fnsize;
	caddr_t		nsp;
	u_int		xargc;
	u_int		stgsize;
	u_int		actualstksz;
	u_int		bsize;
	u_int		psize;
	caddr_t		*ptrstart;
	caddr_t		cp;
	u_int		ptrdelta;
	caddr_t		argv0;
	extern int	exec_initialstk;
	u_int		vectorsz;

	stgsize = (u_int)args->argsize + (u_int)args->envsize +
			(u_int)args->prefixsize + (u_int)args->auxsize;

	if ((xargc = args->prefixc)) {
		fnsize = strlen(args->fname) + 1;
		stgsize += fnsize;
	}
	else if (args->flags & EMULA) {
		fnsize = strlen(args->fname) + 1;
		stgsize += fnsize;
		xargc = 1;
	}

	stgsize = (stgsize + NBPW-1) & ~(NBPW-1);
	args->stringsize = stgsize;

	bsize = (actualstksz =
		(((vectorsz = (3 + args->argc + args->envc + xargc) * NBPW)
		+ stgsize + ((NBPW*2) - 1)) & ~((NBPW*2) - 1)))
		+ exec_initialstk;
	psize = btoc(bsize);
	bsize = ctob(psize);

	if ((nsp = execstk_addr(bsize, &args->estkhflag)) == NULL) {
#ifdef DEBUG
		cmn_err(CE_CONT,"extractarg: execstk_addr\n");
#endif
		return(ENOMEM);
	}
	args->estkstart = nsp;
	args->estksize = bsize;
	args->stacklow = (addr_t) u.u_userstack + NBPW;

	if (error = as_map(u.u_procp->p_as, nsp, bsize, segvn_create, zfod_argsp)) {
#ifdef DEBUG
		cmn_err(CE_CONT,"extractarg: as_map\n");
#endif
		return error;
	}

	ptrdelta = (u_int)args->stacklow - (u_int)(nsp += bsize);
	ptrstart = (caddr_t *)(nsp -= actualstksz);
	cp = (caddr_t) (nsp + vectorsz);
	args->stackend = (caddr_t)(args->stacklow - actualstksz);

	if (suword((int *)ptrstart, args->argc + xargc)) {
err:
#ifdef DEBUG
		cmn_err(CE_CONT,"extractarg: arguments\n");
#endif
		as_unmap(u.u_procp->p_as, args->estkstart, bsize);
		return EFAULT;
	}
	ptrstart++;

	args->auxaddr = cp + ptrdelta;
	cp += args->auxsize;

	if (args->prefixc) {
		if (copyarglist(args->prefixc, args->prefixp, ptrdelta,
				ptrstart, cp, 1) != args->prefixsize)  {
			goto err;
		}
		ptrstart += xargc;
		cp += args->prefixsize;

		bcopy(args->fname, cp, fnsize);
		cp += fnsize;
	}

	args->argaddr = cp + ptrdelta;
	if (copyarglist(args->argc, args->argp, ptrdelta, ptrstart, cp, 0)
			!= args->argsize) {
		goto err;
	}
	if (args->prefixc) {
		if (suword((int *)ptrstart, (int)(cp + ptrdelta - fnsize)))
			goto err;
	}
	ptrstart += args->argc + 1;
	cp += args->argsize;

	if (args->flags & EMULA) {
		if (copyarglist(1, (caddr_t *) &args->fname, ptrdelta, ptrstart - 1,
				cp, 1) != fnsize) {
			goto err;
		}
		++ptrstart;
		cp += fnsize;
	}

	if (copyarglist(args->envc, args->envp, ptrdelta, ptrstart, cp, 0)
			!= args->envsize) {
		goto err;
	}
	ASSERT((caddr_t)(ptrstart + args->envc + 1) == (args->auxaddr - ptrdelta));
	ASSERT((addr_t)(((uint)cp + args->envsize + (NBPW*2 - 1)) & ~(NBPW*2 -1)) ==
				(addr_t)args->estkstart + args->estksize);

	return 0;
}

/*
 * Machine-dependent final setup code goes in setregs().
 */
int
setregs(args)
	register struct uarg *args;
{
	register int i;
	register char *sp;
	register struct proc *p = u.u_procp;

        /*
         * Do psargs.
	 */
	sp = u.u_psargs;
	i = min(args->argsize, PSARGSZ -1);
	if (copyin(args->argaddr, sp, i))
		return(EFAULT);
	while (i--) {
		if (*sp == '\0')
			*sp = ' ';
		sp++;
	}
	*sp = '\0';

	p->p_stksize = args->estksize;
	p->p_stkbase = (caddr_t)u.u_userstack;

	u.u_sub = (ulong)(u.u_userstack - p->p_stksize + sizeof(int));

	ASSERT((u_int)args->stackend >= (u_int)u.u_sub);
	u.u_ar0[UESP] = (int)args->stackend;
 	u.u_ar0[EIP] = (int)u.u_exdata.ux_entloc;
 	u.u_ar0[CS] = USER_CS;
 	u.u_ar0[DS] = u.u_ar0[ES] = u.u_ar0[SS] = USER_DS;
 	u.u_ar0[FS] = u.u_ar0[GS] = 0;
 	u.u_ar0[EFL] &= ~PS_D;		/* clear direction flag */

 	/*
 	** If this process is being traced, come up with the single-step
 	** flag set.
 	*/
 	if (u.u_procp->p_flag & STRC)
 		u.u_ar0[EFL] |= PS_T;

 	u.u_debugon = 0;
 
 	/* Clear the user return address for signals */
 	u.u_sigreturn = (void (*)()) NULL;

	return 0;
}

/*
 * Allocate 'size' units from the given map,
 * returning the base of an area which starts on a "mask" boundary.
 * That is, which has all of the bits in "mask" off in the starting
 * address.  Mask is assummed to be (2**N - 1).
 * Algorithm is first-fit.
 */
int
ptmalloc(mp, size, mask)
	struct map *mp;
	int size;
	int mask;
{
	register int a, b;
	register int gap, tail;
	register struct map *bp;

	ASSERT(size >= 0);
	for (bp = mapstart(mp); bp->m_size; bp++) {
		if (bp->m_size >= size) {
			a = bp->m_addr;
			b = (a+mask) & ~mask;
			gap = b - a;
			if (bp->m_size < (gap + size))
				continue;
			if (gap != 0) {
				tail = bp->m_size - size - gap;
				bp->m_size = gap;
				if (tail) 
					mfree(mp, tail, bp->m_addr+gap+size);
			} else {
				bp->m_addr += size;
				if ((bp->m_size -= size) == 0) {
					do {
						bp++;
						(bp-1)->m_addr = bp->m_addr;
					} while ((bp-1)->m_size = bp->m_size);
					mapsize(mp)++;
				}
			}
			ASSERT(bp->m_size < (unsigned) 0x80000000);
			return b;
		}
	}
	return 0;
}


void
savecontext(ucp, mask)
register ucontext_t *ucp;
k_sigset_t mask;
{
	register proc_t	*pp = u.u_procp;

	ucp->uc_flags = UC_ALL;
	ucp->uc_link = u.u_oldcontext;

	/*
	 * Save current stack state 
	 */
	ucp->uc_stack.ss_sp = (char *)
		(pp->p_stkbase + sizeof(int) - pp->p_stksize);
	ucp->uc_stack.ss_size = pp->p_stksize;
	ucp->uc_stack.ss_flags = 0;
	if (ucp->uc_stack.ss_sp == u.u_sigaltstack.ss_sp)
		ucp->uc_stack.ss_flags |= SS_ONSTACK;

 	/* If this process owns the floating point unit,                */
 	/* save its state in the u block,                               */
 	/* and set the TS bit so that the unit will be re-initialized   */
 	/* if it used in the signal handler.                             */
 	if (fp_proc == u.u_procp) {
 		fpsave();
 		setts();
 	}
#ifdef WEITEK
 	if (u.u_procp == weitek_proc)
 		weitek_save();
#endif

	/* 
	 * Save machine context 
	 */
	prgetregs(PTOU(pp), ucp->uc_mcontext.gregs);
	prgetfpregs(pp, &ucp->uc_mcontext.fpregs);

	/* At this point, u.u_fpvalid indicates whether the process
	 * is using floating point, since we are after the call to
	 * fpsave.
	 */
	if (u.u_fpvalid)
		u.u_fpvalid = 0;
	else
		ucp->uc_flags &= ~UC_FP;

#ifdef WEITEK
 	if (u.u_procp != weitek_proc)
		ucp->uc_flags &= ~UC_WEITEK;
#endif

	/* save signal mask */
	sigktou(&mask,&ucp->uc_sigmask);

 	((flags_t *)&u.u_ar0[EFL])->fl_tf = 0;    /* disable single step */
}


/* These structures define what is pushd on the stack */

struct argpframe {
	void		(*retadr)();
	u_int		signo;
	siginfo_t	*sip;
	ucontext_t	*ucp;
};

struct compat_frame {
	void		(*retadr)();
	u_int		signo;
	gregset_t	gregs;
	char		*fpsp;
	char		*wsp;
};

/*
 * dispatch signal handler
 */

int
sendsig(sig, sip, hdlr)
	int sig;
	k_siginfo_t *sip;
	register void (*hdlr)();
{
	ucontext_t uc;
	siginfo_t si;
	int newstack;		/* if true, switching to alternate stack */
	int minstacksz;		/* size of stack required to catch signal */
 	register uint sp, ap;
	proc_t	*p = u.u_procp;
	int	setsegregs = 0;		/* set segment register flag */
	char 	*dp;			/* pointer a stack descriptor in LDT */
	struct argpframe argpframe;
	int	old_style;

#ifdef VPIX
	flags_t	*flags = (flags_t *)&(u.u_ar0[EFL]);

	/* This routine is called only when we are going to user
	 * mode. If it is a dual mode process and we are going
	 * back to 386 user mode process normally. If we are going
	 * back to v86 mode, then force a virtual interrupt. This
	 * force a switch to 386 user mode.
	 */

	if (flags->fl_vm) {                     /* If going to V86 mode */
 		v86sighdlint(hdlr, sig);	/* Signal handler routine */
		return 1;
	}
#endif

	old_style = sigismember(&u.u_oldsig, sig);
	if (old_style) {
		minstacksz =
			sizeof(ucontext_t) + 	/* user context structure */
			sizeof(struct compat_frame); 	/* current signal */
		sip = NULL;
	} else {
		minstacksz = 
			sizeof(ucontext_t) + 	/* user context structure */
			sizeof(struct argpframe); 	/* current signal */
		if (sip != NULL) {
			bzero((caddr_t)&si, sizeof(si));
			bcopy((caddr_t)sip, (caddr_t)&si, sizeof(k_siginfo_t));
			minstacksz += sizeof(siginfo_t);
		}
	}
 
 	newstack = (sigismember(&u.u_sigonstack, sig)
	  && !(u.u_sigaltstack.ss_flags & (SS_ONSTACK|SS_DISABLE))); 

	if (newstack != 0) {
 		if (minstacksz >= u.u_sigaltstack.ss_size) {
			return 0;
		}
 		sp = (uint)u.u_sigaltstack.ss_sp + u.u_sigaltstack.ss_size;
 	} else {
 		register uint sub;

		sub = u.u_ar0[UESP];

		/* if the stack segment selector is not the
		 * 386 user data selector, convert the SS:SP
	  	 * to the equivalent 386 virtual address;
		 * and set the flag to load SS and DS with
		 * the correct values after they have been saved.
		 */

		if (u.u_ar0[SS] != USER_DS) {
			dp = (char *)(((struct dscr *)(u.u_procp->p_ldt))
				+ seltoi(u.u_ar0[SS]));
			sub &= 0xFFFF;
			sub += (dp[7] << 24) | (*(int *)&dp[2] & 0x00FFFFFF);
			setsegregs++;
		}
 		
		sp = sub;
		sub -= minstacksz;
 		if (sub < (uint)u.u_sub && !grow((int *)sub)) {
			return 0;
		}
	}

	if (sip != NULL) {
		sp -= sizeof(siginfo_t);
		if (copyout((caddr_t)&si, (caddr_t)sp, sizeof(siginfo_t)) < 0) 
			return 0;
		argpframe.sip = (siginfo_t *)sp;
	} else
		argpframe.sip = NULL;

	savecontext(&uc, u.u_sigoldmask);

#ifdef WEITEK
	if (sig == SIGFPE)
		weitek_reset_intr();
#endif /* WEITEK */

	sp -= sizeof(ucontext_t);
	if (copyout((caddr_t)&uc, (caddr_t)sp, sizeof(ucontext_t)) < 0) 
		return 0;
	argpframe.ucp = (ucontext_t *)sp;

	if (old_style) {
		struct compat_frame	cframe;

		cframe.retadr = u.u_sigreturn;
		cframe.signo = sig;
		bcopy((caddr_t)uc.uc_mcontext.gregs, (caddr_t)cframe.gregs,
				sizeof(gregset_t));
		cframe.fpsp = (char *)&argpframe.ucp->uc_mcontext.fpregs.fp_reg_set;
		cframe.wsp = (char *)&argpframe.ucp->uc_mcontext.fpregs.f_wregs[0];

		sp -= sizeof(struct compat_frame);
		if (copyout((caddr_t)&cframe, (caddr_t)sp,
					sizeof(struct compat_frame)) < 0) 
			return 0;
	} else {
		argpframe.retadr = (void (*)())0xFFFFFFFF;
					/* Shouldn't return via this;
					   if they do, fault. */
		argpframe.signo = sig;

		sp -= sizeof(struct argpframe);
		if (copyout((caddr_t)&argpframe, (caddr_t)sp,
					sizeof(struct argpframe)) < 0) 
			return 0;
	}

	/* now that we can no longer fault, update the u-block */

	/* push context */
	u.u_oldcontext = argpframe.ucp;

	u.u_ar0[EIP] = (unsigned int) hdlr;
	u.u_ar0[UESP] = (unsigned int) sp;
	
	if (setsegregs) {
		u.u_ar0[DS] = u.u_ar0[ES] = u.u_ar0[SS] = USER_DS;
		u.u_ar0[CS] = USER_CS;
	}

	((flags_t *)&u.u_ar0[EFL])->fl_tf = 0;	/* disable single step */

	if (newstack) {
		u.u_sigaltstack.ss_flags |= SS_ONSTACK;
 		u.u_sub = (ulong)u.u_sigaltstack.ss_sp; 
		u.u_userstack = (ulong)(u.u_sub + u.u_sigaltstack.ss_size -
					sizeof(int));
		u.u_procp->p_stkbase = (caddr_t)u.u_userstack;
		u.u_procp->p_stksize = (u_int)u.u_sigaltstack.ss_size;
	}
	return 1; 
}

/*
 * Zero out a block of memory
 */

wzero(ptr, count)
register int *ptr;
register int count;
{
	count >>= 2;
	while (count--)
		*ptr++ = 0;
}

/*
 * 1. Map tss of the new process by JTSS
 * 2. Mark JTSS an available TSS
 * 3. Map ldt of the new process by LDTSEL descriptor
 */

mapnewtss(p)
struct proc *p;
{
 	user_t	*nu;
 	extern struct seg_desc gdt[];

#ifdef VPIX
 	register v86_t	*v86p;
 	char utssflag;
#endif

 	nu = PTOU(p);

 	/* Set up the new ldt as it will appear in the new
 	 * process.  Be careful not to use ldt based selectors
 	 * until after the context switch.
 	 */
	gdt[seltoi(LDTSEL)] = nu->u_ldt_desc;

	/*
 	 * set up size of the TEMPORARY new tss and mark it available
 	 */
#ifdef VPIX
 	/*
 	**  If outgoing process is a dual-mode process, remember which
 	**  task he was in when he entered the kernel.
 	*/
 	v86p = (v86_t *)(u.u_procp->p_v86);	/* Get ptr to v86_t struct */
 	if (v86p)                       /* Save TR for v86 process */
 	    v86p->vp_oldtr = (sel_t)get_tr() & (sel_t)0x0FFF8;

 	/*
 	**  Always set EM bit in CR0 if there is emulation, since if the
 	**  previous task was a dual-mode process in V86 mode, we may have
 	**  unset it to fool it into thinking there's no emulation.
 	*/
 	if (fp_kind == FP_SW)
 		setem();

 	/*
 	**  If incoming process is a dual-mode process then:
 	**    a)  If he entered the kernel in Virtual 86 mode, we must
 	**        map his XTSS, not his U-block TSS, into the window.
 	**        The XTSS descriptor must be marked AVAILABLE because
 	**        we will switch into it in misc.s:swtch(). The U-block
 	**        TSS is already marked AVAILABLE.
 	**    b)  If he entered the kernel in protected mode we treat
 	**        him like everyone else, but the XTSS descriptor must
 	**        be marked BUSY, because the ECT will IRET into it.
 	**  We can tell the difference by looking at the TSS selector we
 	**  saved when we switched him out (see immediately above).
	*/

 	utssflag = 1;     /* Assume no XTSS to map in                    */
	v86p = (v86_t *)(p->p_v86);	/* Get ptr to v86_t struct */
 	if (v86p)
 	{   gdt[seltoi(XTSSSEL)] = v86p->vp_xtss_desc;
 	    v86procflag = 1;
 	    if (v86p->vp_oldtr == XTSSSEL) {
		gdt[seltoi(JTSSSEL)] = v86p->vp_xtss_desc;
 		/*
 		 * Make sure the xtss has interrupts disabled, in case
 		 * we were swapped out
 		 */
 		v86p->vp_xtss->xt_tss.t_eflags &= ~PS_IE;
 		utssflag = 0;
 	    }
 	    else
 		setdscracc1(&gdt[seltoi(XTSSSEL)], TSS3_KBACC1);
 	} else
	    v86procflag = 0;	/* No user-mode idt switch for new proc */
 	if (utssflag)
#endif
	{
		gdt[seltoi(JTSSSEL)] = nu->u_tss_desc;

		ASSERT((((struct tss386 *)((char *)nu + (uint)nu->u_tss - UVUBLK))
			->t_eflags & PS_IE) == 0);
	}
}

 
setdscrbase(dscr, base)
struct dscr *dscr;
unsigned int base;
{
 	dscr->a_base0015 = (ushort)base;
 	dscr->a_base1623 = (base>>16)&0x000000FF;
 	dscr->a_base2431 = (base>>24)&0x000000FF;
}

setdscracc1(dscr, acc1)
struct dscr *dscr;
unsigned int	acc1;
{
 	dscr->a_acc0007 = (unsigned char)acc1;
}

setdscracc2(dscr, acc2)
struct dscr *dscr;
unsigned int	acc2;
{
 	dscr->a_acc0811 = acc2;
}
 
unsigned int
getdscrbase(dscr)
struct dscr *dscr;
{
 	register unsigned int base;
 
 	base = (dscr->a_base2431 << 24);
 	base = (base | (dscr->a_base1623 << 16));
 	base = (base | dscr->a_base0015);
 	return(base);
}
 
/*
 * Restore user's context after execution of user signal handler
 * This code restores all registers to what they were at the time
 * signal occured. So any changes made to things like flags will
 * disappear.
 *
 * The saved context is assumed to be at esp+xxx address on the user's
 * stack. If user has mucked with his stack, he will suffer.
 * Called from the sig_clean.
 *
 * On entry, assume all registers are pushed.  r0ptr points to registers
 * on stack.
 * This function returns like other system calls.
 */

sigclean(r0ptr)
register int	*r0ptr;		/* registers on stack */
{
	register struct compat_frame *cframe;
	ucontext_t	uc, *ucp;

	/* The user's stack pointer currently points into compat_frame
	 * on the user stack.  Adjust it to the base of compat_frame.
	 */
	cframe = (struct compat_frame *)(r0ptr[UESP] - 2 * sizeof(int));
	
	ucp = (ucontext_t *)(cframe + 1);
 
	if (copyin((caddr_t)ucp, (caddr_t)&uc, sizeof(ucontext_t)) == -1) {
		u.u_ar0 = r0ptr;	/* core will use these registers */
		exit( (core("core", u.u_procp, u.u_cred,
			u.u_rlimit[RLIMIT_CORE].rlim_cur, SIGSEGV) ?
				CLD_DUMPED|CLD_KILLED : CLD_KILLED), SIGSEGV);
		return;
	}
 
	/* Old stack frame has gregs in a different place.
	   Copy it into the ucontext structure. */
	if (copyin((caddr_t)&cframe->gregs, (caddr_t)&uc.uc_mcontext.gregs,
						sizeof(gregset_t)) == -1) {
		u.u_ar0 = r0ptr;	/* core will use these registers */
		exit( (core("core", u.u_procp, u.u_cred,
			u.u_rlimit[RLIMIT_CORE].rlim_cur, SIGSEGV) ?
				CLD_DUMPED|CLD_KILLED : CLD_KILLED), SIGSEGV);
		return;
	}

	restorecontext(&uc);
}

/* dfgetuserTSS - Get the user TSS pointer from back link for double fault */
 
struct tss386 *
dfgetuserTSS()
{
 	extern struct seg_desc gdt[];
 	extern struct tss386 dftss;
 	struct seg_desc *tss_dscr;

 	tss_dscr = &gdt[seltoi(dftss.t_link)];  /* Get user TSS descriptor */
	return((struct tss386 *)getdscrbase(tss_dscr));
}

#if	DEBUG
/*
 *	db_resume()  --  debugging routine called at the end of resume()
 */
db_resume()
{
 	unsigned int getdscrlimit();
 
 	ASSERT(getdscrlimit(LDTSEL) ==
 				(u.u_ldtlimit+1)*sizeof(struct seg_desc) - 1);
}
#endif

/* return limit (in bytes) of a descriptor which may be in LDT or GDT */
unsigned int
getdscrlimit(seg)
ushort seg;
{
 	extern struct seg_desc gdt[];
 	register struct dscr *dp;
 	register unsigned int limit;

 	if(!(SEL_LDT & seg))
 		dp = (struct dscr *)&gdt[seltoi(seg)];
 	else {
 		seg = seltoi(seg);
 		if(seg >= u.u_ldtlimit)
 			return 0;
 		dp = (struct dscr *)(u.u_procp->p_ldt) + seg;
	}

 	/*	separated because compiler doesn't seem to handle
 		shifting of char bitfields quite right...
 	*/
 	limit = (unsigned int)dp->a_lim1619;
 	limit <<= 16;
 	limit |= (unsigned int)dp->a_lim0015;
 
 	if(dp->a_acc0811 & GRANBIT)
 		return (limit<<BPCSHIFT) | (NBPC-1);
 	return(limit);
}

unsigned int
getdscraddr(seg)
unsigned short seg;
{
 	extern struct seg_desc gdt[];
 	register struct dscr *dp;
 	register unsigned addr;

 	if(!(SEL_LDT & seg))
 		dp = (struct dscr *)&gdt[seltoi(seg)];
 	else {
 		seg = seltoi(seg);
 		if(seg >= u.u_ldtlimit)
 			return 0;
 		dp = (struct dscr *)(u.u_procp->p_ldt) + seg;
 	}
 
 	addr = dp->a_base0015;
 	addr += dp->a_base1623 << 16;
 	addr += dp->a_base2431 << 24;
 
 	return(addr);
}

/*
 * dhalt
 *	Halt devices.
 *
 * Called late in system shutdown to allow devices to stop cleanly
 * AFTER interrupts are shut off.  Interrupts may be turned on
 * again (e.g., the a.t.&t. console driver needs interrupts on to
 * recognize the ctrl-alt-del sequence) so the drivers should make
 * sure no interrupt is pending from their peripheral.
 */

dhalt()
{
 	extern int (*io_halt[])(); 
 	register int i;
 
 	for(i = 0; io_halt[i]; i++)
 		(*io_halt[i])();
}

/*
 * This function is called to check that a psw is suitable for
 * running in user mode.  If not, it is fixed to make it
 * suitable.  This is necessary when a psw is saved on the user
 * stack where some sneaky devil could set kernel mode or
 * something.
 */

void 
fixuserefl(r0ptr)
register int	*r0ptr;
{
	/* set IOPL in user flags to 0 */
	r0ptr[EFL] &= ~PS_IOPL;
	
	/* set interrupt enable flag */
	r0ptr[EFL] |= PS_IE;
	
	/* set TI bit in user CS for LDT access */
	r0ptr[CS] |= 4;
}

void
restorecontext(ucp)
	register ucontext_t *ucp;
{
	register proc_t	*pp = u.u_procp;

	u.u_oldcontext = ucp->uc_link;

	if (ucp->uc_flags & UC_STACK) {
		if (pp->p_stkbase != (caddr_t)ucp->uc_stack.ss_sp +
					ucp->uc_stack.ss_size - sizeof(int)) {
			pp->p_stksize = ucp->uc_stack.ss_size;
			u.u_sub = (u_long)ucp->uc_stack.ss_sp;
			u.u_userstack = (u_long)(u.u_sub + pp->p_stksize) -
						sizeof(int);
			pp->p_stkbase = (caddr_t)u.u_userstack;
		}
		if (ucp->uc_stack.ss_flags & SS_ONSTACK)
			bcopy((caddr_t)&ucp->uc_stack,
			      (caddr_t)&u.u_sigaltstack,
			      sizeof(struct sigaltstack));
		else
			u.u_sigaltstack.ss_flags &= ~SS_ONSTACK;
	}

	if (ucp->uc_flags & UC_CPU)
		prsetregs(PTOU(pp), ucp->uc_mcontext.gregs);

#ifdef WEITEK
	if (ucp->uc_flags & (UC_FP|UC_WEITEK)) {
#else
	if (ucp->uc_flags & UC_FP) {
#endif
		prsetfpregs(pp, &ucp->uc_mcontext.fpregs);
#ifdef WEITEK
		if (ucp->uc_flags & UC_FP)
#endif
			u.u_fpvalid = 1;
	}
 	else
 		u.u_fpvalid = 0;
 
 	/* If this process owns the floating point unit,                */
 	/* give up ownership,                                           */
 	/* and set the TS bit so that the saved floating point state    */
 	/* will be restored if this process uses it again.              */
 	if (fp_proc == u.u_procp) {
 		fp_proc = (proc_t *)0;
 		setts();
	}

#ifdef WEITEK
	/* Were we using the Weitek before the signal? */
	if (ucp->uc_flags & UC_WEITEK) {
 		init_weitek();
 		/* clear AE byte of context register */
 		clear_weitek_ae ();
 		weitek_restore(u.u_weitek_reg);
 	} 
#endif

	if (ucp->uc_flags & UC_SIGMASK) {
		sigutok(&ucp->uc_sigmask,&u.u_procp->p_hold);
		sigdiffset(&u.u_procp->p_hold,&cantmask);
	}
}

/*
 * Adjust the time to UNIX based time
 * This routine must run at IPL15
 */

void
clkset(oldtime)
register time_t	oldtime;
{
	hrestime.tv_sec = (unsigned)oldtime;
}

/*
 * Stop the clock.
 * This routine must run at IPL15
 */

clkreld()
{
}

/*
** Can user write virtual address va?
** This routine is necessary because user addresses are being accessed
** from the kernel, and therefore the hardware won't enforce user permissions.
** Returns 0 unless an address-fault would occur.
** 386 specific: called from misc.s during copyout()/copyin().
**
** We have to do an explicit write fault here, since, on the 386, kernel
** writes to read-only pages don't generate faults.
*/

int
userwrite(va, nbytes)
register caddr_t va;
int nbytes;
{
	register pte_t		*ppte;
	register caddr_t	addr = va;
	register caddr_t	eaddr = va + nbytes;

	/*
	 *	Optimization: If user pages are valid, user accessible and writeable,
	 *	then no need to break user copy-on-write.
	 */

	if (! PG_ISVALID(ppte = vatopdte(addr)))
		goto resolveflt;
	ppte = vatopte(addr, ppte);

	while (addr < eaddr) {

		if ((ppte->pg_pte & (PG_V | PTE_RW)) != (PG_V | PTE_RW))
			goto resolveflt;

		addr += PAGESIZE;
		nbytes -= PAGESIZE;
		ppte++;

		if (((ulong)ppte & PTMASK) == 0) {
			if (addr >= eaddr)
				break;
			if (! PG_ISVALID(ppte = vatopdte(addr)))
				goto resolveflt;
			ppte = vatopte(addr, ppte);
		}
	}
	return(0);

resolveflt:
	/*
	** Generate a write fault on the remaining pages.
	** This has the side effect of checking protections at the page level.
	*/
	if (as_fault(u.u_procp->p_as, addr, nbytes, F_INVAL, S_WRITE) == 0)
		return 0;		/* success: is indeed writeable */
	else 
		return SIGSEGV;		/* failure: segmentation violation */
}

/* ARGUSED */
int
buscheck(bp)
	struct buf *bp;
{

	return(0);
}


long mapin_count = 0;

/*
 * Map the data referred to by the buffer bp into the kernel
 * at kernel virtual address addr.  
 */

void
bp_map(bp, addr)
	register struct buf *bp;
	caddr_t addr;
{
	register struct page *pp;
	register int npf;
	register pte_t	*ppte;
	register pte_t	*pdte;

	npf = btoc(bp->b_bcount + ((int)bp->b_un.b_addr & PAGEOFFSET));

	if (bp->b_flags & B_PAGEIO) {
		pp = bp->b_pages;
		ASSERT(pp != NULL);
		while (npf--) {
			pdte = vatopdte((caddr_t)addr);
			ppte = vatopte((caddr_t)addr, pdte);
			ppte->pg_pte = (u_int)mkpte(PG_V, page_pptonum(pp));
			pp = pp->p_next;
			addr += PAGESIZE;
		}
		flushtlb();
	} else 
		cmn_err(CE_PANIC, "bp_map - non B_PAGEIO\n");
}

/*
 * Called to convert bp for pageio to a kernel addressable location.
 * We allocate virtual space from the sptmap and then use bp_map to do
 * most of the real work.
 */

void
bp_mapin(bp)
	register struct buf *bp;
{
	int npf, o;
	caddr_t kaddr;

	mapin_count++;

	if ((bp->b_flags & (B_PAGEIO | B_PHYS)) == 0 ||
	    (bp->b_flags & B_REMAPPED) != 0)
		return;		/* no pageio or already mapped in */

	if ((bp->b_flags & (B_PAGEIO | B_PHYS)) == (B_PAGEIO | B_PHYS))
		cmn_err(CE_PANIC, "bp_mapin");

	o = (int)bp->b_un.b_addr & PAGEOFFSET;
	npf = btoc(bp->b_bcount + o);

	/*
	 * Allocate kernel virtual space for remapping.
	 */
	while ((kaddr = (caddr_t)malloc(sptmap, npf)) == 0) {
		mapwant(sptmap)++;
		(void) sleep((caddr_t)sptmap, PSWP);
	}
	kaddr = (caddr_t)((int)kaddr << PAGESHIFT);

	/* map the bp into the virtual space we just allocated */
	bp_map(bp, kaddr);

	bp->b_flags |= B_REMAPPED;
	bp->b_un.b_addr = kaddr + o;
}

/*
 * bp_mapout will release all the resources associated with a bp_mapin call.
 */
void
bp_mapout(bp)
	register struct buf *bp;
{
	register int npf, saved_npf;
	register pte_t *ppte;
	register pte_t *pdte;
	register struct page *pp;
	caddr_t addr, saved_addr;

	mapin_count--;

	if (bp->b_flags & B_REMAPPED) {
		pp = bp->b_pages;
		npf = btoc(bp->b_bcount + ((int)bp->b_un.b_addr & PAGEOFFSET));
		saved_npf = npf;
		saved_addr = addr = (caddr_t)((int)bp->b_un.b_addr & PAGEMASK);
		while (npf--) {
			ASSERT(pp != NULL);
			pdte = vatopdte((caddr_t)addr);
			ppte = vatopte((caddr_t)addr, pdte);
			ppte->pg_pte = 0;
			addr += PAGESIZE;
			pp = pp->p_next;
		}
		flushtlb();
		saved_addr = (caddr_t)((int)saved_addr >> PAGESHIFT);
		rmfree(sptmap, saved_npf, (u_long)saved_addr);
		bp->b_un.b_addr = (caddr_t)((int)bp->b_un.b_addr & PAGEOFFSET);
		bp->b_flags &= ~B_REMAPPED;
	}
}
