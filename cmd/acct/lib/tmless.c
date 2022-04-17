/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:lib/tmless.c	1.5.2.2"
/*
 *	return 1 if t1 earlier than t2 (times in localtime format)
 *	assumed that t1 and t2 are in same day
 */
#include <time.h>

tmless(t1, t2)
register struct tm *t1, *t2;
{
	if (t1->tm_hour != t2->tm_hour)
		return(t1->tm_hour < t2->tm_hour);
	if (t1->tm_min != t2->tm_min)
		return(t1->tm_min < t2->tm_min);
	return(t1->tm_sec < t2->tm_sec);
}
