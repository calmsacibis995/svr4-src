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
#ident	"@(#)fmli:oh/externoot.c	1.2"

#include "sizes.h"

char *
externoot(obj)
char *obj;
{
	char *extdir = "/info/OH/externals/";
	extern char *Oasys;
	static char fname[PATHSIZ];

	strcpy(fname, Oasys);
	strcat(fname, extdir);
	strcat(fname, obj);
	return(fname);
}

