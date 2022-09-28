/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/CC/stdarg.h	1.1"
/*ident	"@(#)cfront:incl/stdarg.h	1.5"*/
/* stdarg.h */
/* ADAPTED FROM: */
/*	@(#)varargs.h	1.2	*/

#ifndef STDARGH
#define STDARGH

/*
	USAGE:
		f( arg-declarations ... ) {
			va_list ap;
			va_start(ap, parmN);	// parmN == last named arg
			// ...
			type arg = va_arg(ap, type);
			// ...
			va_end(ap);
		}
*/

#ifndef va_start

typedef char *va_list;
#define va_end(ap)
#ifdef u370
#define va_start(ap, parmN) ap =\
	(char *) ((int)&parmN + 2*sizeof(parmN) - 1 & -sizeof(parmN))
#define va_arg(ap, mode) ((mode *)(ap = \
	(char *) ((int)ap + 2*sizeof(mode) - 1 & -sizeof(mode))))[-1]
#else
#define va_start(ap, parmN) ap = (char *)( &parmN+1 )
#define va_arg(ap, mode) ((mode *)(ap += sizeof(mode)))[-1]
#endif

extern int vprintf(char*, va_list),
	vfprintf(FILE*, char*, va_list),
	vsprintf(char*, char*, va_list),
	setvbuf(FILE*, char*, int, int);
#endif

#endif
