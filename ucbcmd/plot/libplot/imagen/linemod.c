/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ucbplot:libplot/imagen/linemod.c	1.1.4.1"

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */


#include "imp.h"

/*
 * Hack to set font.
 */
linemod(s)
char *s;
{
	register char *tit;
	register char *nam;
	int siz = 0;
	nam = s;
	for(tit = "charset="; *tit; )
		if (*tit++ != *nam++)
			return;
	s = nam;
	while(*nam) 
		switch(*nam++) {
		case ',':
		case '\n':
			*--nam = 0;
		}
	siz = atoi(++nam);
	if (siz == 0) {
		while (*--nam >= '0' && *nam <= '9')
			;
		siz = (atoi(++nam)*4)/3;
	}
	if (siz == 0)
		siz = imPcsize;
	setfont(s, siz);
}
