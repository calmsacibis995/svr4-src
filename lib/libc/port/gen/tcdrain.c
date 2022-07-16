/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/tcdrain.c	1.1"

#ifdef __STDC__
	#pragma weak tcdrain = _tcdrain
#endif
#include "synonyms.h"
#include <sys/termios.h>

/*
 * wait until all output on the filedes is drained
 */

int tcdrain(fildes)
int fildes;
{
	return(ioctl(fildes,TCSBRK,1));
}
