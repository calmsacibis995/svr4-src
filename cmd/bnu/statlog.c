/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:statlog.c	1.5.3.1"

#include "uucp.h"

/*
	Report and log file transfer rate statistics.
	This is ugly because we are not using floating point.
*/

void
statlog( direction, bytes, millisecs, breakmsg)
char		*direction;
unsigned long	bytes;
time_t		millisecs;
char		*breakmsg; /* "PARTIAL FILE" or "" */
{
	char		text[ 100 ];
	unsigned long	bytes1000;

	/* bytes1000 = bytes * 1000; */
	/* on fast machines, times(2) resolution may not be enough */
	/* so millisecs may be zero.  just use 1 as best guess */
	if ( millisecs == 0 )
		millisecs = 1;
		
		
	if (bytes < 1<<22)
		bytes1000 = (bytes*1000/millisecs);
	else
		bytes1000 = ((bytes/millisecs)*1000);
		
	(void) sprintf(text, "%s %lu / %lu.%.3lu secs, %lu bytes/sec %s",
		direction, bytes, millisecs/1000, millisecs%1000,
		bytes1000, breakmsg );
		/* bytes1000/millisecs, breakmsg ); */
	CDEBUG(4, "%s\n", text);
	syslog(text);
	return;
}

static unsigned long	filesize;	/* size of file been 
					transferred or received */
/*
	return the size of file been transferred or received
*/
unsigned long
getfilesize()
{
	return(filesize);
}

/*
	update the size of file been transferred or received
*/
void
putfilesize(bytes)
unsigned long bytes;
{
	filesize = bytes;
	return;
}
