/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acomp:common/cgstuff.h	55.1"
/* cgstuff.h */

/* Declarations for routines that interface to CG. */

extern void cg_defnam();
extern void cg_nameinfo();
extern void cg_incode();
extern void cg_treeok();
extern void cg_ecode();
extern void cg_begf();
extern void cg_endf();
extern void cg_copyprm();
extern void cg_bmove();
extern void cg_deflab(), cg_goto();
extern void cg_swbeg(), cg_swcase(), cg_swend();
extern void cg_filename();
extern void cg_ident();
extern void cg_asmold();
extern void cg_begfile();
extern void cg_eof();
extern void cg_profile();
extern void cg_ldcheck();
extern void cg_zecode();
extern void cg_setlocctr();
extern void cg_instart();
extern void cg_inend();

extern ND1 * cg_defstat();
extern ND1 * cg_strinit();

extern char * cg_extname();

/* cg_tconv() should only be used in cgstuff and p1allo */
extern TWORD cg_tconv();

/* Declarations for functions to convert OFFSET's. */
#define	cg_off_bigger(space, o1, o2)	off_bigger((space),(o1),(o2))
#define cg_off_incr(space, o, n)	off_incr((space),(o),(n))
#define	cg_off_is_err(space, o)		off_is_err((space),(o))
extern OFFSET cg_off_conv();

/* For cg_eof(): */
#define	C_TOKEN		1	/* file had tokens in it */
#define	C_NOTOKEN	0	/* file had no tokens */

/* For cg_indata(), cg_strinit(): */
#define	C_READONLY	0	/* read-only data section */
#define	C_READWRITE	1	/* read/write data section */

/* Support for optimizer interface. */

#ifdef	OPTIM_SUPPORT

extern void os_uand();
extern void os_symbol();
extern void os_loop();

#ifdef	FAT_ACOMP

#define	OS_UAND(sid)	cg_q_sid(os_uand, sid)
#define	OS_SYMBOL(sid)	cg_q_sid(os_symbol, sid)
#define	OS_LOOP(code)	cg_q_int(os_loop, code)

#else	/* ! FAT_ACOMP */

#define OS_UAND(sid)	os_uand(sid)
#define OS_SYMBOL(sid)	os_symbol(sid)
#define OS_LOOP(code)	os_loop(code)

#endif	/* def FAT_ACOMP */

#endif /* def OPTIM_SUPPORT */

/* Support for function-at-a-time compiling */

#ifdef	FAT_ACOMP

#ifdef __STDC__
extern void cg_printf(const char *, ...);
#else
extern void cg_printf();
#endif
extern void cg_q_sid();
extern void cg_q_int();
extern void cg_q_nd1();
extern void cg_q_str();
extern void cg_q_type();
extern void cg_q_call();
#if 0
extern void cg_q_puts();
#endif

#define	CG_PRINTF(args)	cg_printf args
#define	CG_PUTCHAR(x)	cg_printf("%c", x)
#define	CG_COPYPRM(sid) cg_q_sid(cg_copyprm, sid)
#define	CG_ECODE(p)	cg_q_nd1(p)

#else	/* ! FAT_ACOMP */

#define	CG_PRINTF(args)	printf args
#define	CG_PUTCHAR(c)	putchar(c)
#define	CG_COPYPRM(sid)	cg_copyprm(sid)
#define	CG_ECODE(p)	cg_ecode(p)

#endif	/* def FAT_ACOMP */
