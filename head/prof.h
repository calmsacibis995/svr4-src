/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:prof.h	1.10.1.4"

#ifndef MARK
#define MARK(K)	{}
#else
#undef MARK

#if defined(__STDC__)

#if #machine(vax)
#define MARK(K)	{\
		asm("	.data");\
		asm("	.align	4");\
		asm("."#K".:");\
		asm("	.long	0");\
		asm("	.text");\
		asm("M."#K":");\
		asm("	nop;nop");\
		asm("	movab	."#K".,r0");\
		asm("	jsb	mcount");\
		}
#elif #machine(pdp11)
#define MARK(K)	{\
		asm("	.bss");\
		asm("."#K".:");\
		asm("	.=.+2");\
		asm("	.text");\
		asm("M."#K":");\
		asm("	mov	$."#K".,r0");\
		asm("	jsr	pc,mcount");\
		}
#elif #machine(i386)
#define MARK(K)	{\
		asm("	.data");\
		asm("	.align 4");\
		asm("."#K".:");\
		asm("	.long 0");\
		asm("	.text");\
		asm("M."#K":");\
		asm("	movl	$."#K".,%edx");\
		asm("	call _mcount");\
		}
#else
#define MARK(K)	{\
		asm("	.data");\
		asm("	.align	4");\
		asm("."#K".:");\
		asm("	.word	0");\
		asm("	.text");\
		asm("M."#K":");\
		asm("	movw	&."#K".,%r0");\
		asm("	jsb	_mcount");\
		}
#endif

#else
#ifdef vax
#define MARK(K)	{\
		asm("	.data");\
		asm("	.align	4");\
		asm(".K.:");\
		asm("	.long	0");\
		asm("	.text");\
		asm("M.K:");\
		asm("	nop;nop");\
		asm("	movab	.K.,r0");\
		asm("	jsb	mcount");\
		}
#endif
#if u3b || M32 || u3b15 || u3b5 || u3b2
#define MARK(K)	{\
		asm("	.data");\
		asm("	.align	4");\
		asm(".K.:");\
		asm("	.word	0");\
		asm("	.text");\
		asm("M.K:");\
		asm("	movw	&.K.,%r0");\
		asm("	jsb	_mcount");\
		}
#endif
#ifdef i386
#define MARK(K)	{\
		asm("	.data");\
		asm("	.align 4");\
		asm(".K.:");\
		asm("	.long 0");\
		asm("	.text");\
		asm("M.K:");\
		asm("	movl	$.K.,%edx");\
		asm("	call _mcount");\
		}
#endif
#ifdef pdp11
#define MARK(K)	{\
		asm("	.bss");\
		asm(".K.:");\
		asm("	.=.+2");\
		asm("	.text");\
		asm("M.K:");\
		asm("	mov	$.K.,r0");\
		asm("	jsr	pc,mcount");\
		}
#endif

#endif	/* __STDC__ */

#endif  /* MARK */
