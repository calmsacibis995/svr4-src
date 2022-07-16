/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _TIME_H				
#define _TIME_H

#ident	"@(#)head:time.h	1.18"

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

#define CLOCKS_PER_SEC		1000000



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

#if __STDC__ == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
extern void tzset(void);

extern char *tzname[2];

#ifndef CLK_TCK
#define CLK_TCK	_sysconf(3)	/* 3B2 clock ticks per second */
				/* 3 is _SC_CLK_TCK */
#endif

#if (__STDC__ == 0 && !defined(_POSIX_SOURCE)) || defined(_XOPEN_SOURCE)
extern long timezone;
extern int daylight;
#endif

#endif

#if __STDC__ == 0 && !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
extern int cftime(char *, char *, const time_t *);
extern int ascftime(char *, const char *, const struct tm *);
extern long altzone;
extern struct tm *getdate(const char *);
extern int getdate_err;
#endif

#else			

extern long clock();			
extern double difftime();		
extern time_t mktime();		
extern time_t time();			
extern size_t strftime();
extern struct tm *gmtime(), *localtime();
extern char *ctime(), *asctime();
extern int cftime(), ascftime();
extern void tzset();

extern long timezone, altzone;
extern int daylight;
extern char *tzname[2];

extern struct tm *getdate();
extern int getdate_err;

#endif	/* __STDC__ */

#endif /* _TIME_H */
