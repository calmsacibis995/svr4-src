/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)csh:signal.h	1.2.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#ifndef CSH_SIGNAL_H
#define CSH_SIGNAL_H
/*
 * 4.3BSD signal compatibility header
 *
 */
#define sigmask(m)	(m > 32 ? 0 : (1 << ((m)-1)))

/*
 * 4.3BSD structure used in sigstack call.
 */

struct sigstack {
        char    *ss_sp;                 /* signal stack pointer */
        int     ss_onstack;             /* current status */
};

/*
 * 4.3BSD signal vector structure used in sigvec call.
 */
struct  sigvec {
        void    (*sv_handler)();        /* signal handler */
        int     sv_mask;                /* signal mask to apply */
        int     sv_flags;               /* see signal options below */
};

#define SV_ONSTACK      0x0001  /* take signal on signal stack */
#define SV_INTERRUPT    0x0002  /* do not restart system on signal return */
#define SV_RESETHAND    0x0004  /* reset handler to SIG_DFL when signal taken */

#define sv_onstack sv_flags

struct  sigcontext {
        int     sc_onstack;             /* sigstack state to restore */
        int     sc_mask;                /* signal mask to restore */
#ifdef i386
        int     sc_esp;                 /* stack to restore */
        int     sc_eip;                 /* instruction to restore */
        int     sc_ebp;                 /* frame to restore */
        int     sc_eax;                 /* eax to restore */
        int   	sc_edx;                 /* edx to restore */
#endif /* i386 */
#ifdef u3b2
        int     sc_sp;                  /* sp to restore */
        int     sc_fp;                  /* fp to restore */
        int     sc_ap;                  /* ap to restore */
        int     sc_pc;                  /* pc to restore */
        int   	sc_ps;                  /* psw to restore */
#endif
#ifdef vax
        int     sc_sp;                  /* sp to restore */
        int     sc_fp;                  /* fp to restore */
        int     sc_ap;                  /* ap to restore */
        int     sc_pc;                  /* pc to restore */
        int     sc_ps;                  /* psl to restore */
#endif vax
#ifdef mc68000
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to retore */
        int     sc_ps;                  /* psl to restore */
#endif mc68000
#ifdef sparc
#define MAXWINDOW       31              /* max usable windows in sparc */
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to retore */
        int     sc_npc;                 /* next pc to restore */
        int     sc_psr;                 /* psr to restore */
        int     sc_g1;                  /* register that must be restored */
        int     sc_o0;
        int     sc_wbcnt;               /* number of outstanding windows */
        char    *sc_spbuf[MAXWINDOW];   /* sp's for each wbuf */
        int     sc_wbuf[MAXWINDOW][16]; /* outstanding window save buffer */
#endif sparc
#ifdef sun386
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to retore */
        int     sc_ps;                  /* psl to restore */
        int     sc_eax;                 /* eax to restore */
        int     sc_edx;                 /* edx to restore */
#endif
};

#define SI_DFLCODE	1

#ifdef vax
#define ILL_RESAD_FAULT	ILL_ILLADR	/* reserved addressing fault */
#define ILL_PRIVIN_FAULT ILL_PRVOPC	/* privileged instruction fault */
#define ILL_RESOP_FAULT	ILL_ILLOPC	/* reserved operand fault */

#define FPE_INTOVF_TRAP	FPE_INTOVF	/* integer overflow */
#define FPE_INTDIV_TRAP	FPE_INTDIV	/* integer divide by zero */
#define FPE_FLTOVF_TRAP	FPE_FLTOVF	/* floating overflow */
#define FPE_FLTDIV_TRAP	FPE_FLTDIV	/* floating/decimal divide by zero */
#define FPE_FLTUND_TRAP	FPE_FLTUND	/* floating underflow */
#define FPE_DECOVF_TRAP	FPE_INTOVF	/* decimal overflow */
#define FPE_SUBRNG_TRAP	FPE_FLTSUB	/* subscript out of range */
#define FPE_FLTOVF_FAULT FPR_FLTOVF	/* floating overflow fault */
#define FPE_FLTDIV_FAULT FPE_FLTDIV	/* divide by zero floating fault */
#define FPE_FLTUND_FAULT FPE_FLTUND	/* floating underflow fault */

#endif vax

#ifdef mc68000

#define ILL_ILLINSTR_FAULT ILL_ILLOPC	/* illegal instruction fault */
#define ILL_PRIVVIO_FAULT ILL_PRVREG	/* privilege violation fault */
#define ILL_COPROCERR_FAULT ILL_COPERR	/* [coprocessor protocol error fault] */
#define ILL_TRAP1_FAULT	ILL_ILLTRP	/* trap #1 fault */
#define ILL_TRAP2_FAULT	ILL_ILLTRP	/* trap #2 fault */
#define ILL_TRAP3_FAULT	ILL_ILLTRP	/* trap #3 fault */
#define ILL_TRAP4_FAULT	ILL_ILLTRP	/* trap #4 fault */
#define ILL_TRAP5_FAULT	ILL_ILLTRP	/* trap #5 fault */
#define ILL_TRAP6_FAULT	ILL_ILLTRP	/* trap #6 fault */
#define ILL_TRAP7_FAULT	ILL_ILLTRP	/* trap #7 fault */
#define ILL_TRAP8_FAULT	ILL_ILLTRP	/* trap #8 fault */
#define ILL_TRAP9_FAULT	ILL_ILLTRP	/* trap #9 fault */
#define ILL_TRAP10_FAULT ILL_ILLTRP	/* trap #10 fault */
#define ILL_TRAP11_FAULT ILL_ILLTRP	/* trap #11 fault */
#define ILL_TRAP12_FAULT ILL_ILLTRP	/* trap #12 fault */
#define ILL_TRAP13_FAULT ILL_ILLTRP	/* trap #13 fault */
#define ILL_TRAP14_FAULT ILL_ILLTRP	/* trap #14 fault */

#define EMT_EMU1010	SI_DFLCODE	/* line 1010 emulator trap */
#define EMT_EMU1111	SI_DFLCODE	/* line 1111 emulator trap */

#define FPE_INTDIV_TRAP	FPE_INTDIV	/* integer divide by zero */
#define FPE_CHKINST_TRAP SI_DFLCODE	/* CHK [CHK2] instruction */
#define FPE_TRAPV_TRAP	SI_DFLCODE	/* TRAPV [cpTRAPcc TRAPcc] instr */
#define FPE_FLTBSUN_TRAPSI_DFLCODE	/* [branch or set on unordered cond] */
#define FPE_FLTINEX_TRAP FPE_FLTRES	/* [floating inexact result] */
#define FPE_FLTDIV_TRAP	FPE_FLTDIV	/* [floating divide by zero] */
#define FPE_FLTUND_TRAP	FPE_FLTUND	/* [floating underflow] */
#define FPE_FLTOPERR_TRAP FPE_FLTINV	/* [floating operand error] */
#define FPE_FLTOVF_TRAP	FPE_FLTOVF	/* [floating overflow] */
#define FPE_FLTNAN_TRAP	FPE_FLTINV	/* [floating Not-A-Number] */

#ifdef sun
#define FPE_FPA_ENABLE	SI_DFLCODE	/* [FPA not enabled] */
#define FPE_FPA_ERROR	SI_DFLCODE	/* [FPA arithmetic exception] */
#endif sun

#endif mc68000

#ifdef sparc

#define ILL_STACK	ILL_STKERR	/* bad stack */
#define ILL_ILLINSTR_FAULT ILL_ILLOPC	/* illegal instruction fault */
#define ILL_PRIVINSTR_FAULT ILL_PRVOPC	/* privileged instruction fault */
#define ILL_TRAP_FAULT(n) ILL_ILLTRP	 /* trap n fault */

#define	EMT_TAG		SI_DFLCODE	/* tag overflow */

#define FPE_INTOVF_TRAP	FPE_INTOVF	/* integer overflow */
#define FPE_INTDIV_TRAP	FPE_INTDIV	/* integer divide by zero */
#define FPE_FLTINEX_TRAP FPE_FLTRES	/* [floating inexact result] */
#define FPE_FLTDIV_TRAP	FPE_FLTDIV	/* [floating divide by zero] */
#define FPE_FLTUND_TRAP	FPE_FLTUND	/* [floating underflow] */
#define FPE_FLTOPERR_TRAP FPE_FLTSUB	/* [floating operand error] */
#define FPE_FLTOVF_TRAP	FPE_FLTOVF	/* [floating overflow] */

#endif sparc

#define BUS_HWERR	BUS_ADRERR	/* misc hardware error (e.g. timeout) */
#define BUS_ALIGN	BUS_ADRALN	/* hardware alignment error */

#define SEGV_NOMAP	SEGV_MAPERR	/* no mapping at the fault address */
#define SEGV_PROT	SEGV_ACCERR	/* access exceeded protections */

/*
 * The SEGV_CODE(code) will be SEGV_NOMAP, SEGV_PROT, or SEGV_OBJERR.
 * In the SEGV_OBJERR case, doing a SEGV_ERRNO(code) gives an errno value
 * reported by the underlying file object mapped at the fault address.
 */

#define SIG_NOADDR	((char *)~0)

#define	SEGV_CODE(fc)	((fc) & 0xff)
#define	SEGV_ERRNO(fc)	((unsigned)(fc) >> 8)
#define	SEGV_MAKE_ERR(e) (((e) << 8) | SEGV_MAPERR)
#endif CSH_SIGNAL_H
