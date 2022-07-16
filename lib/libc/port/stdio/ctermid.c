/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/ctermid.c	1.12"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#ifdef __STDC__
	#pragma weak ctermid = _ctermid
#endif
#include "synonyms.h"
#include <stdio.h>
#include <string.h>

static char res[L_ctermid];

char *
ctermid(s)
	register char *s;
{
	return strcpy(s != 0 ? s : res, "/dev/tty");
}
