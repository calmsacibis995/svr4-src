/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/time_gdata.c	1.4"

#ifdef __STDC__
	#pragma weak altzone = _altzone
	#pragma weak daylight = _daylight
	#pragma weak timezone = _timezone
	#pragma weak tzname = _tzname
#endif
#include	"synonyms.h"
#include	<sys/types.h>
#include 	<time.h>

time_t	timezone = 0;
time_t	altzone = 0;
int 	daylight = 0;
char 	*tzname[] = {"GMT","   "};
