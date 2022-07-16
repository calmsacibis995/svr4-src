/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#ident	"@(#)head.sys:sys/time.h	1.16.3.1"

/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */

#include <sys/types.h>

#if !defined(_POSIX_SOURCE) 
struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};

struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};
#define	DST_NONE	0	/* not on dst */
#define	DST_USA		1	/* USA style dst */
#define	DST_AUST	2	/* Australian style dst */
#define	DST_WET		3	/* Western European dst */
#define	DST_MET		4	/* Middle European dst */
#define	DST_EET		5	/* Eastern European dst */
#define	DST_CAN		6	/* Canada */
#define	DST_GB		7	/* Great Britain and Eire */
#define	DST_RUM		8	/* Rumania */
#define	DST_TUR		9	/* Turkey */
#define	DST_AUSTALT	10	/* Australian style with shift in 1986 */

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 */
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, cmp)	\
	((tvp)->tv_sec cmp (uvp)->tv_sec || \
	 (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define	timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_usec = 0

/*
 * Names of the interval timers, and structure
 * defining a timer setting.
 */
#define	ITIMER_REAL	0
#define	ITIMER_VIRTUAL	1
#define	ITIMER_PROF	2

struct	itimerval {
	struct	timeval it_interval;	/* timer interval */
	struct	timeval it_value;	/* current value */
};

/*
 * Time expressed in seconds and nanoseconds
 */
#endif /* !defined(_POSIX_SOURCE) */ 

typedef struct 	timestruc {
	time_t 		tv_sec;		/* seconds */
	long		tv_nsec;	/* and nanoseconds */
} timestruc_t;

#ifdef _KERNEL
/*
 * Bump a timestruc by a small number of nsec
 */

#define	BUMPTIME(t, nsec, flag) { \
	register timestruc_t	*tp = (t); \
\
	tp->tv_nsec += (nsec); \
	if (tp->tv_nsec >= 1000000000) { \
		tp->tv_nsec -= 1000000000; \
		tp->tv_sec++; \
		flag = 1; \
	} \
}

extern	timestruc_t	hrestime;
#endif

#if !defined(_KERNEL) && !defined(_POSIX_SOURCE)
#if defined(__STDC__)
int adjtime(struct timeval *, struct timeval *);
int getitimer(int, struct itimerval *);
int setitimer(int, struct itimerval *, struct itimerval *);
#endif /* __STDC__ */
#if !defined(_XOPEN_SOURCE)
#include <time.h>
#endif
#endif /* _KERNEL */


#endif	/* _SYS_TIME_H */
