/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/getkey.c	1.1.4.1"

#include "sys/types.h"
#include "time.h"
#include "stdlib.h"

#include "lpsched.h"

long
#if	defined(__STDC__)
getkey (
	void
)
#else
getkey ()
#endif
{
	ENTRY ("getkey")

	static int		started = 0;

	if (!started) {
		srand48 (time((time_t *)0));
		started = 1;
	}
	return (lrand48());
}
