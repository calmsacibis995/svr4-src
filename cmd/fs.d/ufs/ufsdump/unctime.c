/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)ufs.cmds:ufs/ufsdump/unctime.c	1.2.3.1"

#include <sys/types.h>
#include <time.h>
#include <stdio.h>
/*
 * Convert a ctime(3) format string into a system format date.
 * Return the date thus calculated.
 *
 * Return -1 if the string is not in ctime format.
 */

/*
 * Offsets into the ctime string to various parts.
 */

#define	E_MONTH		4
#define	E_DAY		8
#define	E_HOUR		11
#define	E_MINUTE	14
#define	E_SECOND	17
#define	E_YEAR		20

time_t unctime(str)
	char *str;
{
	struct tm then;
	char dbuf[30];
	time_t emitl();

	if (strlen(str) != 25)
		str[25] = 0;
	strcpy(dbuf, str);
	dbuf[E_MONTH+3] = 0;
	if ( (then.tm_mon = lookup(&dbuf[E_MONTH])) < 0) {
		return(-1);;
	}
	then.tm_mday = atoi(&dbuf[E_DAY]);
	then.tm_hour = atoi(&dbuf[E_HOUR]);
	then.tm_min = atoi(&dbuf[E_MINUTE]);
	then.tm_sec = atoi(&dbuf[E_SECOND]);
	then.tm_year = atoi(&dbuf[E_YEAR]) - 1900;
	return(emitl(&then));
}

static char months[] =
	"JanFebMarAprMayJunJulAugSepOctNovDec";

static
lookup(str)
	char *str;
{
	register char *cp, *cp2;

	for (cp = months, cp2 = str; *cp != 0; cp += 3)
		if (strncmp(cp, cp2, 3) == 0)
			return((cp-months) / 3);
	return(-1);
}
/*
 * Routine to convert a localtime(3) format date back into
 * a system format date.
 *
 *	Use a binary search.
 */

struct tm *localtime();

time_t emitl(dp)
	struct tm *dp;
{
	time_t conv;
	register int i, bit;
	struct tm dcopy;

	dcopy = *dp;
	dp = &dcopy;
	conv = 0;
	for (i = 30; i >= 0; i--) {
		bit = 1 << i;
		conv |= bit;
		if (dcmp(localtime(&conv), dp) > 0)
			conv &= ~bit;
	}
	return(conv);
}

/*
 * Compare two localtime dates, return result.
 */

#define DECIDE(a) \
	if (dp->a > dp2->a) \
		return(1); \
	if (dp->a < dp2->a) \
		return(-1)

static
dcmp(dp, dp2)
	register struct tm *dp, *dp2;
{

	DECIDE(tm_year);
	DECIDE(tm_mon);
	DECIDE(tm_mday);
	DECIDE(tm_hour);
	DECIDE(tm_min);
	DECIDE(tm_sec);
	return(0);
}
