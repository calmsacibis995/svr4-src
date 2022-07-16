/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:copylist.c	1.1.3.2"
/*
	copylist copies a file into a block of memory, replacing newlines
	with null characters, and returns a pointer to the copy.
*/

#ifdef __STDC__
	#pragma weak copylist = _copylist
#endif
#include "synonyms.h"

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stdio.h>

static long	linecount;

char *
copylist(filenm, szptr)
const char	*filenm;
off_t	*szptr;
{
	FILE		*strm;
	struct	stat	stbuf;
	register int	c;
	char		*malloc();
	register char	*ptr, *p;
	int		stat();

	/* get size of file */
	if (stat(filenm, &stbuf) == -1) {
		return(NULL);
	}
	*szptr = stbuf.st_size;

	/* get block of memory */
	if((ptr = malloc((unsigned) *szptr)) == NULL) {
		return(NULL);
	}

	/* copy contents of file into memory block, replacing newlines
	with null characters */
	if ((strm = fopen(filenm, "r")) == NULL) {
		return(NULL);
	}
	linecount = 0;
	for (p = ptr; p < ptr + *szptr  &&  (c = getc(strm)) != EOF; p++) {
		if (c == '\n') {
			*p = '\0';
			linecount++;
		}
		else
			*p = c;
	}
	(void)fclose(strm);

	return(ptr);
}
