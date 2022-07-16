/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbtermcap:tputs.c	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#ifndef lint
static	char sccsid[] = "@(#)tputs.c 1.5 88/02/08 SMI"; /* from UCB 4.1 6/27/83 */
#endif

/* Copyright (c) 1979 Regents of the University of California */
#include <sgtty.h>
#include <ctype.h>

/*
 * The following array gives the number of tens of milliseconds per
 * character for each speed as returned by gtty.  Thus since 300
 * baud returns a 7, there are 33.3 milliseconds per char at 300 baud.
 */
static
short	tmspc10[] = {
	0, 2000, 1333, 909, 743, 666, 500, 333, 166, 83, 55, 41, 20, 10, 5
};

short	ospeed;
char	PC;

/*
 * Put the character string cp out, with padding.
 * The number of affected lines is affcnt, and the routine
 * used to output one character is outc.
 */
tputs(cp, affcnt, outc)
	register char *cp;
	int affcnt;
	int (*outc)();
{
	register int i = 0;
	register int mspc10;

	if (cp == 0)
		return;

	/*
	 * Convert the number representing the delay.
	 */
	if (isdigit(*cp)) {
		do
			i = i * 10 + *cp++ - '0';
		while (isdigit(*cp));
	}
	i *= 10;
	if (*cp == '.') {
		cp++;
		if (isdigit(*cp))
			i += *cp - '0';
		/*
		 * Only one digit to the right of the decimal point.
		 */
		while (isdigit(*cp))
			cp++;
	}

	/*
	 * If the delay is followed by a `*', then
	 * multiply by the affected lines count.
	 */
	if (*cp == '*')
		cp++, i *= affcnt;

	/*
	 * The guts of the string.
	 */
	while (*cp)
		(*outc)(*cp++);

	/*
	 * If no delay needed, or output speed is
	 * not comprehensible, then don't try to delay.
	 */
	if (i == 0)
		return;
	if (ospeed <= 0 || ospeed >= (sizeof tmspc10 / sizeof tmspc10[0]))
		return;

	/*
	 * Round up by a half a character frame,
	 * and then do the delay.
	 * Too bad there are no user program accessible programmed delays.
	 * Transmitting pad characters slows many
	 * terminals down and also loads the system.
	 */
	mspc10 = tmspc10[ospeed];
	i += mspc10 / 2;
	for (i /= mspc10; i > 0; i--)
		(*outc)(PC);
}
