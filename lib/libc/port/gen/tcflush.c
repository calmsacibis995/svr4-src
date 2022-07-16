/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/tcflush.c	1.1"

#ifdef __STDC__
	#pragma weak tcflush = _tcflush
#endif
#include "synonyms.h"
#include <sys/termios.h>

/*
 * flush read, write or both sides
 */

/*
 * TCIFLUSH  (0) -> flush data received but not read
 * TCOFLUSH  (1) -> flush data written but not transmitted
 * TCIOFLUSH (2) -> flush both
 */

int tcflush(fildes,queue_selector)
int fildes;
int queue_selector;
{
	return(ioctl(fildes,TCFLSH,queue_selector));
}
