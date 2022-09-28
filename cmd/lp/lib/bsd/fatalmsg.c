/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/bsd/fatalmsg.c	1.3.2.1"

#include <stdio.h>
#include <string.h>
#if defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#define WHO_AM_I	I_AM_OZ		/* to get oam.h to unfold */
#include "oam.h"
#include "lpd.h"

/*
 * Report fatal error and exit
 */
/*VARARGS1*/
void
#if defined (__STDC__)
fatal(char *fmt, ...)
#else
fatal(fmt, va_alist)
char	*fmt;
va_dcl
#endif
{
	va_list	argp;

	if (Rhost)
		(void)printf("%s: ", Lhost);
	printf("%s: ", Name);
	if (Printer)
		(void)printf("%s: ", Printer);
#if defined (__STDC__)
	va_start(argp, fmt);
#else
	va_start(argp);
#endif
	(void)vprintf(fmt, argp);
	va_end(argp);
	putchar('\n');
	fflush(stdout);
	done(1);		/* defined by invoker */
	/*NOTREACHED*/
}

/*
 * Format lp error message to stderr
 * (this will probably change to remain compatible with LP)
 */
/*VARARGS1*/
void
#if defined (__STDC__)
_lp_msg(long msgid, va_list argp)
#else
_lp_msg(msgid, argp)
long	msgid;
va_list	argp;
#endif
{
	char	 label[20];

	(void)vsprintf(_m_, agettxt(msgid, _a_, MSGSIZ), argp);
	strcpy(label, "UX:");
	strcat(label, basename(Name));
	fmtmsg(SOFT|UTIL|RECOVER, 
	       label, 
	       ERROR, 
	       _m_, 
	       agettxt(msgid+1, _a_, MSGSIZ),
	       tagit(msgid));
}

/*
 * Format lp error message to stderr
 */
/*VARARGS1*/
void
#if defined (__STDC__)
lp_msg(long msgid, ...)
#else
lp_msg(msgid, va_alist)
long	msgid;
va_dcl
#endif
{
	va_list	argp;

#if defined (__STDC__)
	va_start(argp, msgid);
#else
	va_start(argp);
#endif
	_lp_msg(msgid, argp);
	va_end(argp);
}

/*
 * Report lp error message to stderr and exit
 */
/*VARARGS1*/
void
#if defined (__STDC__)
lp_fatal(long msgid, ...)
#else
lp_fatal(msgid, va_alist)
long	msgid;
va_dcl
#endif
{
	va_list	argp;

#if defined (__STDC__)
	va_start(argp, msgid);
#else
	va_start(argp);
#endif
	_lp_msg(msgid, argp);
	va_end(argp);

	done(1);			/* Supplied by caller */
	/*NOTREACHED*/
}
