/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

#ident	"@(#)ucbrefer:refer8.c	1.1.3.1"

#include "refer..c"

static char ahead[1024];
static int peeked = 0;
static char *noteof = (char *) 1;

char *
input(s)
char *s;
{
	if (peeked) {
		peeked = 0;
		if (noteof == 0)
			return(0);
		strcpy(s, ahead);
		return(s);
	}
	return(fgets(s, 1000, in));
}

char *
lookat()
{
	if (peeked)
		return(ahead);
	noteof = input(ahead);
	peeked = 1;
	return(noteof);
}

addch(s, c)
char *s;
{
	while (*s)
		s++;
	*s++ = c;
	*s = 0;
}
