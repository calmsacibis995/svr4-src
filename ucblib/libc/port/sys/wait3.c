/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/wait3.c	1.6.3.1"

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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/siginfo.h>
#include <sys/procset.h>
#include <sys/param.h>

/* get the local version of sys/resource.h */

#include <sys/resource.h>

#define clk2tv(clk, tv) \
{ \
	tv.tv_sec = clk / HZ; \
	tv.tv_usec = (clk % HZ) / HZ * 1000000; \
}

wait3(status, options, rusage)
        int	*status;
        int     options;
        struct  rusage  *rusage;
 
{
        siginfo_t info;

        if (waitid(P_ALL, 0, &info, 
	  (options & (WNOHANG|WUNTRACED)) | WEXITED | WTRAPPED))
		return -1;

	if (info.si_pid != 0) {

		if (rusage) {
			memset((void *)rusage, 0, sizeof(struct rusage));
			clk2tv(info.si_utime, rusage->ru_utime);
			clk2tv(info.si_stime, rusage->ru_stime);
		}

		if (status) {	

			*status = (info.si_status & 0377);

			switch (info.si_code) {
				case CLD_EXITED:
					(*status) <<= 8;
					break;
				case CLD_DUMPED:
					(*status) |= WCOREFLG;
					break;
				case CLD_KILLED:
					break;
				case CLD_TRAPPED:
				case CLD_STOPPED:
					(*status) <<= 8;
					(*status) |= WSTOPFLG;
					break;
			}
		}
        } 

	return info.si_pid;
}
