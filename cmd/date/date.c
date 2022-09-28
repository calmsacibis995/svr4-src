/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)date:date.c	1.29"

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
**	date - with format capabilities and international flair
*/

#include        <stdio.h>
#include	<time.h>
#include	<sys/time.h>
#include	<sys/types.h>
#include	<locale.h>
#include	<fcntl.h>
#include	"utmp.h"

#ifdef i386
#include	<sys/uadmin.h>
#endif

#define	year_size(A)	(((A) % 4) ? 365 : 366)

static 	char	buf[1024];
static	time_t	clock_val;
static  short	month_size[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static  struct  utmp	wtmp[2] = { {"", "", OTIME_MSG, 0, OLD_TIME, 0, 0, 0}, 
			    {"", "", NTIME_MSG, 0, NEW_TIME, 0, 0, 0} };
static char *usage = "usage: date [-u] [+format] [hhmm | mmddhhmm[[cc]yy]] [-a sss.fff]\n";
int uflag;
char *ap;

main(argc, argv)
int	argc;
char	**argv;
{
	register struct tm *tp;
	struct timeval tv;
	char *fmt = NULL;
	char *locp;

	locp = setlocale(LC_ALL, "");
	if (locp == (char *)0) {
		(void)setlocale(LC_TIME, "");
		(void)setlocale(LC_CTYPE, "");
	}

	/*  Initialize variables  */

	(void)time(&clock_val);

	if (argc > 1) {
		if (strcmp(argv[1], "-u") == 0) {
			argc--; argv++;
			uflag++;
		}
		else if (strcmp(argv[1], "-a") == 0) {
			if (argc < 3) {
				fprintf(stderr, usage);
				exit(1);
			}
			get_adj(argv[2], &tv);
			if(adjtime(&tv, 0) < 0) {
				perror("date: Failed to adjust date");
				exit(1);
			}
			exit(0);
		}		
	}
	if (argc > 1) {	
		if (*argv[1] == '+') 
			fmt = &argv[1][1];
		else if (setdate(localtime(&clock_val), argv[1]))
			exit(1);
	}

	if (uflag) {
		tp = gmtime(&clock_val);
		ascftime(buf, "%a %b %e %H:%M:%S GMT %Y", tp);
	}
	else {
		tp = localtime(&clock_val);
		ascftime(buf, fmt, tp);
	}

	puts(buf);

	exit(0);
}

setdate(current_date, date)
struct tm	*current_date;
char		*date;
{
	register int i;
	int mm, dd=0, hh, min, yy, wf=0;
	int minidx = 6;

#ifdef i386
	time_t	correction;
#endif

	/*  Parse date string  */
	switch(strlen(date)) {
	case 12:
		yy = atoi(&date[8]);
		date[8] = '\0';
		break;
	case 10:
		yy = 1900 + atoi(&date[8]);
		date[8] = '\0';
		break;
	case 8:
		yy = 1900 + current_date->tm_year;
		break;
	case 4: 
		yy = 1900 + current_date->tm_year;
		mm = current_date->tm_mon + 1; 	/* tm_mon goes from 1 to 11*/
		dd = current_date->tm_mday;
		minidx = 2;
		break;	
	default:
		(void) fprintf(stderr, "date: bad conversion\n");
		return(1);
	}
	min = atoi(&date[minidx]);
	date[minidx] = '\0';
	hh = atoi(&date[minidx-2]);
	date[minidx-2] = '\0';
	if (!dd) { 
		/* if dd is 0 (not between 1 and 31), then 
		 * read the value supplied by the user.
		 */
		dd = atoi(&date[2]);
		date[2] = '\0';
		mm = atoi(&date[0]);
	}
	if(hh == 24)
		hh = 0, dd++;

	/*  Validate date elements  */
	if(!((mm >= 1 && mm <= 12) && (dd >= 1 && dd <= 31) &&
		(hh >= 0 && hh <= 23) && (min >= 0 && min <= 59))) {
		(void) fprintf(stderr, "date: bad conversion\n");
		return(1);
	}

	/*  Build date and time number  */
	for(clock_val = 0, i = 1970; i < yy; i++)
		clock_val += year_size(i);
	/*  Adjust for leap year  */
	if (year_size(yy) == 366 && mm >= 3)
		clock_val += 1;
	/*  Adjust for different month lengths  */
	while(--mm)
		clock_val += (time_t)month_size[mm - 1];
	/*  Load up the rest  */
	clock_val += (time_t)(dd - 1);
	clock_val *= 24;
	clock_val += (time_t)hh;
	clock_val *= 60;
	clock_val += (time_t)min;
	clock_val *= 60;

	if (!uflag) {
		/* convert to GMT assuming standard time */
		/* correction is made in localtime(3C) */

		clock_val += (time_t)timezone;

#ifdef i386
		correction = timezone;
#endif

		/* correct if daylight savings time in effect */

#ifdef i386
		if (localtime(&clock_val)->tm_isdst) { 
			clock_val = clock_val - (time_t)(timezone - altzone); 
		}
#else
		if (localtime(&clock_val)->tm_isdst)
			clock_val = clock_val - (time_t)(timezone - altzone); 
#endif

	}

	(void) time(&wtmp[0].ut_time);

#ifdef i386
	uadmin(A_CLOCK, correction, 0);
#endif

	if(stime(&clock_val) < 0) {
		(void) fprintf(stderr, "date: no permission\n");
		return(1);
	}
	(void) time(&wtmp[1].ut_time);
	pututline(&wtmp[0]);
	pututline(&wtmp[1]);
	if ((wf = open(WTMP_FILE, O_WRONLY | O_APPEND)) >= 0) {
		(void) write(wf, (char *) wtmp, sizeof(wtmp));
		close(wf);
	}
	return(0);
}

get_adj(cp, tp)
        char *cp;
        struct timeval *tp;
{
        register int mult;
        int sign;

        tp->tv_sec = tp->tv_usec = 0;
        if (*cp == '-') {
                sign = -1;
                cp++;
        } else {
                sign = 1;
        }
        while (*cp >= '0' && *cp <= '9') {
                tp->tv_sec *= 10;
                tp->tv_sec += *cp++ - '0';
        }
        if (*cp == '.') {
                cp++;
                mult = 100000;
                while (*cp >= '0' && *cp <= '9') {
                        tp->tv_usec += (*cp++ - '0') * mult;
                        mult /= 10;
                }
        }
        if (*cp) {
                fprintf(stderr,usage);
                exit(1);
        }
        tp->tv_sec *= sign;
        tp->tv_usec *= sign;
}

