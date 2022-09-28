/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tsort:errusage.c	1.1"

#include  "errmsg.h"
#include  <stdio.h>
#include  <varargs.h>

#define   USAGENO  255	/* exit value for usage messages */

/* This routine prints the standard command usage message.
*/

void
errusage( va_alist )
va_dcl
{
	va_list	ap;
	char	*format;

	va_start( ap );
	format = va_arg( ap, char * );
	va_end( ap );

	fputs( "Usage:  ", stderr );
	if( Err.vsource  &&  Err.source ) {
		fputs( Err.source, stderr );
		fputc( ' ', stderr );
	}
	vfprintf( stderr, format, ap );
	fputc( '\n', stderr );

	errexit( USAGENO );
	erraction( EEXIT );
}
