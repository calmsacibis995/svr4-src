/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lprof:libprof/symintErr.c	1.4"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include "symint.h"

#include <stdio.h>
#include <varargs.h>

/* * * * * *
 * symintFcns.c -- symbol information interface routines.
 * 
 * these routines form a symbol information access
 * interface, for the profilers to get at object file
 * information.  this interface was designed to aid
 * in the COFF to ELF conversion of prof, lprof and friends.
 * 
 */


/* * * * * *
 * _err_exit(format_s, va_alist)
 * format_s	- printf(3S) arg string.
 * va_alist	- var_args(3?) printf() arguments.
 * 
 * does not return - prints message and calls exit(3).
 * 
 * 
 * this routine spits out a message (passed as above)
 * and exits.
 */

_err_exit(format_s, va_alist)
char *format_s;
va_dcl
{
	va_list ap;

	fprintf(stderr, "fatal error: ");
	va_start(ap);
	vfprintf(stderr, format_s, ap);
	va_end(ap);
	fprintf(stderr, "\n");

debugp1("--- this is where we exit ---\n")

	exit(1);
}


/* * * * * *
 * _err_warn(format_s, va_alist)
 * format_s	- printf(3S) arg string.
 * va_alist	- var_args(3?) printf() arguments.
 * 
 * 
 * This routine prints a warning (passed as above)
 */

_err_warn(format_s, va_alist)
char *format_s;
va_dcl
{
	va_list ap;

	fprintf(stderr, "Warning: ");
	va_start(ap);
	vfprintf(stderr, format_s, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}
