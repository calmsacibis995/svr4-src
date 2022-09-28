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
#ident	"@(#)fmli:sys/chgepenv.c	1.2"

#include	<stdio.h>
#include	"sizes.h"


char *
chgepenv(name, value)
char	*name, *value;
{
	char dirpath[PATHSIZ];
	extern char	*Home;
	char	*strcat();
	char	*strcpy();
	char	*chgenv();

	return chgenv(strcat(strcpy(dirpath, Home), "/pref/.environ"), name, value);
}
