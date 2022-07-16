/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/ctime.c	1.15"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * This routine converts time as follows.
 * The epoch is 0000 Jan 1 1970 GMT.
 * The argument time is in seconds since then.
 * The localtime(t) entry returns a pointer to an array
 * containing
 *  seconds (0-59)
 *  minutes (0-59)
 *  hours (0-23)
 *  day of month (1-31)
 *  month (0-11)
 *  year-1970
 *  weekday (0-6, Sun is 0)
 *  day of the year
 *  daylight savings flag
 *
 * The routine corrects for daylight saving
 * time and will work in any time zone provided
 * "timezone" is adjusted to the difference between
 * Greenwich and local standard time (measured in seconds).
 * In places like Michigan "daylight" must
 * be initialized to 0 to prevent the conversion
 * to daylight time.
 * There is a table which accounts for the peculiarities
 * undergone by daylight time in 1974-1975.
 *
 * The routine does not work
 * in Saudi Arabia which runs on Solar time.
 *
 * asctime(tvec)
 * where tvec is produced by localtime
 * returns a ptr to a character string
 * that has the ascii time in the form
 *	Thu Jan 01 00:00:00 1970\n\0
 *	01234567890123456789012345
 *	0	  1	    2
 *
 * ctime(t) just calls localtime, then asctime.
 *
 * tzset() looks for an environment variable named
 * TZ. 
 * If the variable is present, it will set the external
 * variables "timezone", "altzone", "daylight", and "tzname"
 * appropriately. It is called by localtime, and
 * may also be called explicitly by the user.
 */

#define	dysize(A) (((A)%4)? 365: 366)
#include "synonyms.h"
#include <time.h>
#include <sys/types.h>


static char cbuf[26];



char *
ctime(t)
const time_t	*t;
{
	return(asctime(localtime(t)));
}


char *
asctime(t)
const struct tm *t;
{
	register char *cp;
	register const char *ncp;
	register const int *tp;
	static char	*ct_numb();
	const char *Date = "Day Mon 00 00:00:00 0000\n";
	const char *Day  = "SunMonTueWedThuFriSat";
	const char *Month = "JanFebMarAprMayJunJulAugSepOctNovDec";

	cp = cbuf;
	for ( ncp = Date; *cp++ = *ncp++; );
	ncp = Day + (3*t->tm_wday);
	cp = cbuf;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp++;
	tp = &t->tm_mon;
	ncp = Month + ( (*tp) * 3 );
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp = ct_numb(cp, *--tp);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, (t->tm_year+1900)/100);
	cp--;
	cp = ct_numb(cp, t->tm_year+100);
	return(cbuf);
}

static char *
ct_numb(cp, n)
register char *cp;
int	n;
{
	cp++;
	if(n >= 10)
		*cp++ = (n/10)%10 + '0';
	else
		*cp++ = ' ';		/* Pad with blanks */
	*cp++ = n%10 + '0';
	return(cp);
}

