/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:strupr.c	1.3"

/* #define	DEBUG		1	/* */

#include	<stdio.h>

/*
			strupr(string)

	Converts passed string to upper case.
*/
strupr(target_file)
char	*target_file;
{
	int	i;

#ifdef DEBUG
	(void) fprintf(stderr, "strupr(): DEBUG - Original value \"%s\"\n", target_file);
#endif

	/*
		Convert target_file to all uppercase
	*/
	for (i = 0; *(target_file + i) != '\0'; i++)
		if (*(target_file + i) > '\140' && *(target_file + i) < '\173')
			*(target_file + i) &= '\337';

#ifdef DEBUG
	(void) fprintf(stderr, "strupr(): DEBUG - Final value \"%s\"\n", target_file);
#endif
}
