/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:legal.c	1.3.3.1"
/*
    NAME
	legal - check existence of file

    SYNOPSIS
	int legal(char *file)

    DESCRIPTION
	legal() checks to see if "file" is a writable file name.

	Returns:
		0	-> file or directory exists, but is unwriteable
		1	-> file exists writeable
		2	-> file does not exist, but can be created
*/

#include "mail.h"
int legal(file)
register char *file;
{
	register char *sp;
	char dfile[MAXFILENAME];

	/*
		If file does not exist then try "." if file name has
		no "/". For file names that have a "/", try check
		for existence of previous directory.
	*/
	if (access(file, A_EXIST) == A_OK) {
		if (access(file, A_WRITE) == A_OK) return(1);
		else return(0);
	} else {
		if ((sp=strrchr(file, '/')) == NULL) {
			strcpy(dfile, ".");
		} else if (sp == file) {
			strcpy(dfile, "/");
		} else {
			strncpy(dfile, file, sp - file);
			dfile[sp - file] = '\0';
		}
		if (access(dfile, A_WRITE) == CERROR)
			return(0);
		return(2);
	}
}
