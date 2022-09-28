/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:oops.c	1.1"

/*
 *	@(#) oops.c 1.1 86/12/12 
 *
 *	Copyright (C) The Santa Cruz Operation, 1984, 1985, 1986, 1987.
 *	Copyright (C) Microsoft Corporation, 1984, 1985, 1986, 1987.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 */

#include <stdio.h>

/*
 * print an error message on stderr.
 * this is only for syntactic brevity.
 */
error(fmt, args)
char *fmt;
int args;
{
	fprintf(stderr, fmt, &args);
}

/*
 * print an error message and quit. 
 * again, only for syntactic brevity.
 */
oops(fmt, args)
char *fmt;
int args;
{
	fprintf(stderr, fmt, &args);
	exit(1);
}
