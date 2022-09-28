/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:basename.c	1.3.3.1"
/*
    NAME
	basename - return base from pathname

    SYNOPSIS
	char *basename(char *path)

    DESCRIPTION
	basename() returns a pointer to the base
	component of a pathname.
*/
#include "mail.h"

char *
basename(path)
	char *path;
{
	char *cp;

	cp = strrchr(path, '/');
	return cp==NULL ? path : cp+1;
}
