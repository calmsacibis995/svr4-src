/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/old_toupper.c	1.4"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * If arg is lower-case, return upper-case, otherwise return arg.
 * If arg is upper-case, return the lower-case, else return the arg.
 * The purpose of this file is to prevent a.outs compiled with the
 * shared C library from failing with the new toupper function
 * The new toupper function introduces an unresolved reference to _libc__ctype
 * that will make those a.outs not work right.
 */
#include "synonyms.h"

int
old_toupper(c)
register int c;
{
	if(c >= 'a' && c <= 'z')
		c += 'A' - 'a';
	return(c);
}
