/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/clock.c	1.6.2.3"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>	/* for HZ (clock frequency in Hz) */
#define TIMES(B)	(B.tms_utime+B.tms_stime+B.tms_cutime+B.tms_cstime)

extern int gethz();
static long first = 0L;

long
clock()
{
	struct tms buffer;
	static int Hz = 0;

	if (!Hz && (Hz = gethz()) == 0)
		Hz = HZ;

	if (times(&buffer) != -1L && first == 0L)
		first = TIMES(buffer);
	return ((TIMES(buffer) - first) * (1000000L/Hz));
}
