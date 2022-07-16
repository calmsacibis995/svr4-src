/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/timezone.c	1.1.3.1"

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

/*
 * The arguments are the number of minutes of time
 * you are westward from Greenwich and whether DST is in effect.
 * It returns a string
 * giving the name of the local timezone.
 *
 * Sorry, I don't know all the names.
 */

static struct zone {
	int	offset;
	char	*stdzone;
	char	*dlzone;
} zonetab[] = {
	-12*60,    "NZST", "NZDT",	/* New Zealand */
	-10*60,    "EST",  "EST",	/* Aust: Eastern */
	-10*60+30, "CST",  "CST",	/* Aust: Central */
	 -8*60,    "WST",  0,		/* Aust: Western */
	 -9*60,    "JST",  0,		/* Japanese */
	  0*60,    "GMT",  "BST",	/* Great Britain and Eire */
	 -1*60,    "MET",  "MET DST",	/* Middle European */
	 -2*60,    "EET",  "EET DST",	/* Eastern European */
	  3*60+30, "NST",  "NDT",	/* Newfoundland */
	  4*60,    "AST",  "ADT",	/* Atlantic */
	  5*60,    "EST",  "EDT",	/* Eastern */
	  6*60,    "CST",  "CDT",	/* Central */
	  7*60,    "MST",  "MDT",	/* Mountain */
	  8*60,    "PST",  "PDT",	/* Pacific */
	  9*60,    "YST",  "YDT",	/* Yukon */
	 10*60,    "HST",  "HDT",	/* Hawaiian */
	-1
};

char *timezone(zone, dst)
{
	register struct zone *zp;
	static char czone[10];
	char *sign;
	register char *p, *q;
	char *getenv(), *index();

	if (p = getenv("TZNAME")) {
		if (q = index(p, ',')) {
			if (dst)
				return(++q);
			else {
				*q = '\0';
				strncpy(czone, p, sizeof(czone)-1);
				czone[sizeof(czone)-1] = '\0';
				*q = ',';
				return (czone);
			}
		}
		return(p);
	}
	for (zp=zonetab; zp->offset!=-1; zp++)
		if (zp->offset==zone) {
			if (dst && zp->dlzone)
				return(zp->dlzone);
			if (!dst && zp->stdzone)
				return(zp->stdzone);
		}
	if (zone<0) {
		zone = -zone;
		sign = "+";
	} else
		sign = "-";
	sprintf(czone, "GMT%s%d:%02d", sign, zone/60, zone%60);
	return(czone);
}
