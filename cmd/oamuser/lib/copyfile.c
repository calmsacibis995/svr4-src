/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/copyfile.c	1.1"

#include	<stdio.h>

copyfile(src, dest)
char	*src;
char	*dest;
{
	char	buf[BUFSIZ];
	FILE	*fpsrc, *fpdest;

	if ((fpsrc = fopen(src, "r")) == NULL) {
		fprintf(stderr, "cannot open %s for reading\n", src);
		return(-1);
	}

	if ((fpdest = fopen(dest, "w")) == NULL) {
		fprintf(stderr, "cannot open %s for writing\n", src);
		return(-1);
	}

	while(fgets(buf, BUFSIZ, fpsrc) != NULL)
		fputs(buf, fpdest);

	fclose(fpsrc);
	fclose(fpdest);

	return(0);
}
