/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/dates.c	1.3.7.1"



#include	<time.h>

extern int putenv();

static int mask_defined = 0;

static char *dmaskpath = "DATEMSK=/etc/datemsk";

extern int _getdate_err;

/* Parse a date string and return time_t value */
time_t
p_getdate( string )
char *string;
{
	struct tm *tmptr, *getdate();
	time_t rtime;

	if ( !mask_defined ) {
		if ( putenv( dmaskpath ) != 0 )
			return( (time_t) 0 );
		mask_defined = 1;
	}
	if( !(tmptr = getdate( string )) )
		return( (time_t) 0 );
	if ( (rtime = mktime( tmptr )) < 0)
		return( (time_t) 0);
	return( rtime );
}

