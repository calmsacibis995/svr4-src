/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/fpos.c	1.2"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <stdio.h>

int
fgetpos(stream, pos)
FILE *stream;
fpos_t *pos;
{
	if ((*pos = ftell(stream)) == -1L)
		return(-1);
	return(0);
}

int
fsetpos(stream, pos)
FILE *stream;
const fpos_t *pos;
{
	if (fseek(stream, *pos, SEEK_SET) != 0)
		return(-1);
	return(0);
}
