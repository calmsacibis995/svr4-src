/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)zdump:time.h	1.1.1.1"

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

#ifndef _TIME_H				
#define _TIME_H

#ifndef NULL
#define NULL	0
#endif

#ifndef _SIZE_T				
#define _SIZE_T
typedef unsigned	size_t;
#endif 
#ifndef _CLOCK_T
#define _CLOCK_T
typedef long 	clock_t;
#endif 
#ifndef _TIME_T
#define _TIME_T
typedef long 	time_t;
#endif 

#define CLK_TCK		1000000

struct	tm {	/* see ctime(3) */
	int	tm_sec;
	int	tm_min;
	int	tm_hour;
	int	tm_mday;
	int	tm_mon;
	int	tm_year;
	int	tm_wday;
	int	tm_yday;
	int	tm_isdst;
	char	*tm_zone;
	long	tm_gmtoff;
};

#if defined(__STDC__) 	

extern clock_t clock(void);			
extern double difftime(time_t, time_t);		
extern time_t mktime(struct tm *);		
extern time_t time(time_t *);			
extern char *asctime(const struct tm *);		
extern char *ctime (const time_t *);		
extern struct tm *gmtime(const time_t *);		
extern struct tm *localtime(const time_t *);	
extern size_t strftime(char *, size_t, const char *, const struct tm *);

#if __STDC__ == 0	
extern int cftime(char *, char *, const time_t *);
extern int ascftime(char *, const char *, const struct tm *);
extern void tzset(void);

extern long timezone, altzone;
extern int daylight;
extern char *tzname[];
#endif

#else			

extern struct tm *gmtime(), *localtime();
extern char *ctime(), *asctime();
extern int cftime(), ascftime();

extern long timezone, altzone;
extern int daylight;
extern char *tzname[];

#endif	/* __STDC__ */

#endif /* _TIME_H */
