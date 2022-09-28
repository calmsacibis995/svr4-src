/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/regsub.c	1.3.3.1"
#include "regprog.h"
#define NULL 0

/* substitute into one string using the matches from the last regexec() */
extern void
regsub (sp, dp, mp, ms)
	char *sp;	/* source string */
	char *dp;	/* destination string */
	regsubexp *mp;	/* subexpression elements */
	int ms;		/* number of elements pointed to by mp */
{
	char *ssp;
	register int i;

	while (*sp != '\0') {
		if (*sp == '\\') {
			switch (*++sp) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				i = *sp-'0';
				if (mp[i].sp != NULL && mp!=NULL && ms>i)
					for (ssp = mp[i].sp;
					     ssp < mp[i].ep;
					     ssp++)
						*dp++ = *ssp;
				break;
			case '\\':
				*dp++ = '\\';
				break;
			case '\0':
				sp--;
				break;
			default:
				*dp++ = *sp;
				break;
			}
		} else if (*sp == '&') {				
			if (mp[0].sp != NULL && mp!=NULL && ms>0)
			if (mp[0].sp != NULL)
				for (ssp = mp[0].sp;
				     ssp < mp[0].ep; ssp++)
					*dp++ = *ssp;
		} else
			*dp++ = *sp;
		sp++;
	}
	*dp = '\0';
}
