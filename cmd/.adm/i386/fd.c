/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)adm:i386/fd.c	1.1.2.1"

#include "time.h"
#include "sys/types.h"
#include "sys/uadmin.h"
/*
	correct unix clock by determining the correction factor
	between local time (which is read from the real time
	clock) and gmt.  this correction is passed to the kernel
	via uadmin(A_CLOCK, correction).  the correction is the
	number of seconds that must be added to local time to
	get gmt.  this may not work east of the prime meridian
*/
main()
{

	struct tm *ptr;
	time_t gmticks, time();
	int correct;

	gmticks = time();

	ptr = localtime(&gmticks);

	if(ptr->tm_isdst) correct = timezone - 3600;
		else correct = timezone;

	uadmin(A_CLOCK, correct);
}
