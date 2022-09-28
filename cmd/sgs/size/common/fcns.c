/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)size:size/common/fcns.c	1.5"	
/* UNIX HEADER */
#include	<stdio.h>

/* SGS SPECIFIC HEADER */
#include	"sgs.h"

    /*  error(file, string)
     *
     *  simply prints the error message string
     *  simply returns
     */

void
error(file, string)

char	*file;
char	*string;

{
    extern	exitcode;
    extern int is_archive;
    extern char *archive;

    (void) fflush(stdout);
    if (is_archive)
    {
    	(void) fprintf(stderr, "%ssize: %s[%s]: %s\n", SGS, archive, file, string);
    }
    else
    {
    	(void) fprintf(stderr, "%ssize: %s: %s\n", SGS, file, string);
    }

    exitcode++;
    return;
}
