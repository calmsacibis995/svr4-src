/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:lib/pnpsplit.c	1.7.2.2"
/*
 * pnpsplit splits interval into prime & nonprime portions
 * ONLY ROUTINE THAT KNOWS ABOUT HOLIDAYS AND DEFN OF PRIME/NONPRIME
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include "acctdef.h"
#include <time.h>

/* validate that hours and minutes of prime/non-prime read in
 * from holidays file fall within proper boundaries.
 * Time is expected in the form and range of 0000-2359.
 */

static int	thisyear = 1970;	/* this is changed by holidays file */
static int	holidays[NHOLIDAYS];	/* holidays file day-of-year table */

static int day_tab[2][13] = {
	{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

/*
 *	prime(0) and nonprime(1) times during a day
 *	for BTL, prime time is 9AM to 5PM
 */
static struct hours {
	int	h_sec;		/* normally always zero */
	int	h_min;		/* initialized from holidays file (time%100) */
	int	h_hour;		/* initialized from holidays file (time/100) */
	long	h_type;		/* prime/nonprime of previous period */
} h[4];
int	daysend[] = {60, 59, 23};	/* the sec, min, hr of the day's end */

struct tm *localtime();
long	tmsecs();

/*
 * split interval of length etime, starting at start into prime/nonprime
 * values, return as result
 * input values in seconds
 */
pnpsplit(start, etime, result)
long start, etime, result[2];
{
	struct tm cur, end;
	long tcur, tend;
	long tmp;
	register sameday;
	register struct hours *hp;
	char *memcpy();

	if (thisyear)	/* once holidays file is read, this is zero */
		checkhol();

	tcur = start;
	tend = start+etime;
	memcpy(&end, localtime(&tend), sizeof(end));
	result[PRIME] = 0;
	result[NONPRIME] = 0;

	while (tcur < tend) {	/* one iteration per day or part thereof */
		memcpy(&cur, localtime(&tcur), sizeof(cur));
		sameday = cur.tm_yday == end.tm_yday;
		if (ssh(&cur)) {	/* ssh:only NONPRIME */
			if (sameday) {
				result[NONPRIME] += tend-tcur;

				break;
			} else {
				tmp = tmsecs(&cur, daysend);
				result[NONPRIME] += tmp;
				tcur += tmp;
			}
		} else {	/* working day, PRIME or NONPRIME */
			for (hp = h; tmless(hp, &cur); hp++);
			for (; hp->h_sec >= 0; hp++) {
				if (sameday && tmless(&end, hp)) {
			/* WHCC mod, change from = to +=   3/6/86   Paul */
					result[hp->h_type] += tend-tcur;
					tcur = tend;
					break;	/* all done */
				} else {	/* time to next PRIME /NONPRIME change */
					tmp = tmsecs(&cur, hp);
					result[hp->h_type] += tmp;
					tcur += tmp;
					cur.tm_sec = hp->h_sec;
					cur.tm_min = hp->h_min;
					cur.tm_hour = hp->h_hour;
				}
			}
		}
	}
}

/*
 *	Starting day after Christmas, complain if holidays not yet updated.
 *	This code is only executed once per program invocation.
 */
checkhol()
{
	register struct tm *tp;
	long t;

	if(inithol() == 0) {
		fprintf(stderr, "pnpsplit: holidays table setup failed\n");
		thisyear = 0;
		holidays[0] = -1;
		return;
	}
	time(&t);
	tp = localtime(&t);
	tp->tm_year += 1900;
	if ((tp->tm_year == thisyear && tp->tm_yday > 359)
		|| tp->tm_year > thisyear)
		fprintf(stderr,
			"***UPDATE %s WITH NEW HOLIDAYS***\n", HOLFILE);
	thisyear = 0;	/* checkhol() will not be called again */
}

/*
 * ssh returns 1 if Sat, Sun, or Holiday
 */
ssh(ltp)
register struct tm *ltp;
{
	register i;

	if (ltp->tm_wday == 0 || ltp->tm_wday == 6)
		return(1);
	for (i = 0; holidays[i] >= 0; i++) 
		if (ltp->tm_yday == holidays[i])
			return(1);
	return(0);
}

/*
 * inithol - read from an ascii file and initialize the "thisyear"
 * variable, the times that prime and non-prime start, and the
 * holidays array.
 */
inithol()
{
	FILE		*fopen(), *holptr;
	char		*fgets(), holbuf[128];
	register int	line = 0,
			holindx = 0,
			errflag = 0;
	int		pstart, npstart;
	int		doy;	/* day of the year */
	int 		month, day;

	if((holptr=fopen(HOLFILE, "r")) == NULL) {
		perror(HOLFILE);
		fclose(holptr);
		return(0);
	}
	while(fgets(holbuf, sizeof(holbuf), holptr) != NULL) {
		if(holbuf[0] == '*')	/* Skip over comments */
			continue;
		else if(++line == 1) {	/* format: year p-start np-start */
			if(sscanf(holbuf, "%4d %4d %4d",
				&thisyear, &pstart, &npstart) != 3) {
				fprintf(stderr,
					"%s: bad {yr ptime nptime} conversion\n",
					HOLFILE);
				errflag++;
				break;
			}

			/* validate year */
			if(thisyear < 1970 || thisyear > 2000) {
				fprintf(stderr, "pnpsplit: invalid year: %d\n",
					thisyear);
				errflag++;
				break;
			}

			/* validate prime/nonprime hours */
			if((! okay(pstart)) || (! okay(npstart))) {
				fprintf(stderr,
					"pnpsplit: invalid p/np hours\n");
				errflag++;
				break;
			}

			/* Set up start of prime time; 2400 == 0000 */
			h[0].h_sec = 0;
			h[0].h_min = pstart%100;
			h[0].h_hour = (pstart/100==24) ? 0 : pstart/100;
			h[0].h_type = NONPRIME;

			/* Set up start of non-prime time; 2400 == 0000 */
			h[1].h_sec = 0;
			h[1].h_min = npstart%100;
			h[1].h_hour = (npstart/100==24) ? 0 : npstart/100;
			h[1].h_type = PRIME ;

			/* This is the end of the day */
			h[2].h_sec = 60;
			h[2].h_min = 59;
			h[2].h_hour = 23;
			h[2].h_type = NONPRIME;

			/* The end of the array */
			h[3].h_sec = -1;

			continue;
		}
		else if(holindx >= NHOLIDAYS) {
			fprintf(stderr, "pnpsplit: too many holidays, ");
			fprintf(stderr, "recompile with larger NHOLIDAYS\n");
			errflag++;
			break;
		}

		/* Fill up holidays array from holidays file */
		sscanf(holbuf, "%d/%d	%*s %*s	%*[^\n]\n", &month, &day);
		if (month < 0 || month > 12) {
			fprintf(stderr, "pnpsplit: invalid month %d\n", month);
			errflag++;
			break;
		}
		if (day < 0 || day > 31) {
			fprintf(stderr, "pnpsplit: invalid day %d\n", day);
			errflag++;
			break;
		}
		doy = day_of_year(thisyear, month, day); 
		holidays[holindx++] = (doy - 1);
	}
	fclose(holptr);
	if(!errflag && holindx < NHOLIDAYS) {
		holidays[holindx] = -1;
		return(1);
	}
	else
		return(0);
}

/*
 *	tmsecs returns number of seconds from t1 to t2,
 *	times expressed in localtime format.
 *	assumed that t1 <= t2, and are in same day.
 */

long
tmsecs(t1, t2)
register struct tm *t1, *t2;
{
	return((t2->tm_sec - t1->tm_sec) +
		60*(t2->tm_min - t1->tm_min) +
		3600L*(t2->tm_hour - t1->tm_hour));
}

/*
 *	return 1 if t1 earlier than t2 (times in localtime format)
 *	assumed that t1 and t2 are in same day
 */

tmless(t1, t2)
register struct tm *t1, *t2;
{
	if (t1->tm_hour != t2->tm_hour)
		return(t1->tm_hour < t2->tm_hour);
	if (t1->tm_min != t2->tm_min)
		return(t1->tm_min < t2->tm_min);
	return(t1->tm_sec < t2->tm_sec);
}

/* set day of year from month and day */

day_of_year(year, month, day)
{
	int i, leap;

	leap = year%4 == 0 && year%100 || year%400 == 0;
	for (i = 1; i < month; i++)
		day += day_tab[leap][i];
	return(day);
}

