/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/filename.c	1.1"

#include <stdio.h>

char *
filename(pt)
register char *pt;
{
	register char *name;
	char *strrchr();

	if (pt == NULL)
		return "(null)";
	if ((name = strrchr(pt, '/')) == NULL)
		return pt;

	return name + 1;
}
