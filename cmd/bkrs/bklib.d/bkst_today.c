/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkst_today.c	1.1.2.1"

#include <sys/types.h>
#include <time.h>

/* Compute the date for the history table - midnight today */
bkst_today()
{
	long current = time( 0 );
	struct tm	*current_tm;
	current_tm = localtime( &current );
	current -= current_tm->tm_sec + current_tm->tm_min * 60 
		+ current_tm->tm_hour * 60 * 60;
	return( current );
}
