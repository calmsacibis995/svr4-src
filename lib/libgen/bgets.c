/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:bgets.c	1.1.3.2"
/*
	read no more than <count> characters into <buf> from stream <fp>,
	stoping at any character slisted in <stopstr>.
	NOTE: This function will not work for multi-byte characters.
*/

#ifdef __STDC__
	#pragma weak bgets = _bgets
#endif
#include "synonyms.h"

#include <sys/types.h>
#include <stdio.h>

#define CHARS	256

static char	stop[CHARS];

char *
bgets( buf, count, fp, stopstr )
char	*buf;
register
size_t	count;
FILE	*fp;
char	*stopstr;
{
	register char	*cp;
	register int	c;
	register size_t i;

	/* clear and set stopstr array */
	for( cp = stop;  cp < &stop[CHARS]; )
		*cp++ = 0;
	for( cp = stopstr;  *cp; )
		stop[(unsigned char)*cp++] = 1;
	i = 0;
	for( cp = buf;  ; ) {
		if(i++ == count) {
			*cp = '\0';
			break;
		}
		if( (c = getc(fp)) == EOF ) {
			*cp = '\0';
			if( cp == buf )
				cp = (char *) 0;
			break;
		}
		*cp++ = c;
		if( stop[ c ] ) {
			*cp = '\0';
			break;
		}
	}
	return  cp;
}
