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
#ident	"@(#)fmli:sys/readfile.c	1.2"

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"wish.h"
#include	"terror.h"

#define PADNUM	20

/* 
 * read in the file
 */
char *
readfile(file, trans_nl)
char *file;
int trans_nl;
{
	FILE *fp;
	register int retval, padding, ch;
	register char *tptr;
	char *text;
	unsigned int bufsize;
	struct stat statbuf;

	if (access(file, 0) < 0)
		return(NULL);
	if ((fp = fopen(file, "r")) == NULL || fstat(fileno(fp), &statbuf) < 0) {
		error(NOPEN, file);
		return(NULL);
	}

	if ((text = malloc(bufsize = statbuf.st_size + PADNUM + 1)) == NULL)
		fatal(NOMEM, NULL);

	padding = PADNUM;
	for (tptr = text; (ch = getc(fp)) != EOF; tptr++) {
		if ((*tptr = ch) == '\n' && trans_nl == TRUE) { 
			*tptr = ' ';
			if (tptr == text)
				continue;
			switch(*(tptr - 1)) {	/* check char before newline */
			case '.':
			case '?':
			case ':':
			case '!':
				/* add an extra blank */
				if (padding-- <= 0) {
					/* just in case */
					unsigned offset;

					offset = tptr - text;
					if ((text = realloc(text, bufsize += PADNUM)) == NULL)
						fatal(NOMEM, NULL);
					padding = PADNUM;
					tptr = text + offset;
				}
				*(++tptr) = ' ';
				break;
			default:
				;
			}
		}
	}
	*tptr = '\0';
	fclose(fp);
	return(text);
}
