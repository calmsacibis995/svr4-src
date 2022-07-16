/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* 	Portions Copyright(c) 1988, Sun Microsystems Inc.	*/
/*	All Rights Reserved					*/

#ident	"@(#)ucblibc:port/gen/gettimeofday.c	1.1.3.1"

#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/evecb.h>
#include <sys/hrtcntl.h>
#include <errno.h>

extern int stime();

/*
 * Get the time of day information.
 * BSD compatibility on top of SVr4 facilities:
 * u_sec always zero, and don't do anything with timezone pointer.
 */
int
gettimeofday(tp, tzp)
	struct timeval *tp;
	struct timezone *tzp;
{
	hrtime_t	tod;
	long		rval;
	
#ifdef lint
        tzp = tzp;
#endif
        
        if (tp == NULL)
                return (0);
	
	tod.hrt_res = MICROSEC;
	rval = hrtcntl(HRT_TOFD, CLK_STD, NULL, &tod);
	if (rval != 0)
		return(-1);
        
        tp->tv_sec = (long)tod.hrt_secs;
        tp->tv_usec = tod.hrt_rem;
	
        return (0);
}

/*
 * Set the time.
 * Don't do anything with the timezone information.
 */
int
settimeofday(tp, tzp)
	struct timeval *tp;
	struct timezone *tzp;
{
        time_t t;                       /* time in seconds */
	
#ifdef lint
        tzp = tzp;
#endif
        if (tp == NULL)
                return (0);
	
        t = (time_t) tp->tv_sec;
        if (tp->tv_usec >= 500000)
                /* round up */
                t++;
	
        return(stime(&t));
}
