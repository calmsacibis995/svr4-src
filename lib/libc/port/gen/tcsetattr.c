/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/tcsetattr.c	1.2"
#ifdef __STDC__
	#pragma weak tcsetattr = _tcsetattr
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/termios.h>
#include <errno.h>

/* 
 * set parameters associated with termios
 */

int tcsetattr (fildes, optional_actions, termios_p)
int fildes;
int optional_actions;
struct termios *termios_p;
{

	int rval;
	
	switch (optional_actions) {
	
		case TCSANOW:

			rval = ioctl(fildes,TCSETS,termios_p);
			break;

		case TCSADRAIN:

			rval = ioctl(fildes,TCSETSW,termios_p);
			break;

		case TCSAFLUSH:

			rval = ioctl(fildes,TCSETSF,termios_p);
			break;

		default:

			rval = -1;
			errno = EINVAL;
	}
	return(rval);
}
