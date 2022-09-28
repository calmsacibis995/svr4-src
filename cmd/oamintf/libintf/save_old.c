/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/save_old.c	1.2.2.2"

/*LINTLIBRARY*/
#include <stdio.h>
#include <string.h>
#include "intf.h"

extern char *getenv();

void
save_old(string, savfile)
char *string;		/* original string to save */
FILE *savfile;		/* file ptr that says where to save */
{
	(void) fputs(SAVHDR, savfile);
	(void) fputs(string, savfile);
}
