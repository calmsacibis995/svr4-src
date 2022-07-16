/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/tcgetpgrp.c	1.1"
#ifdef __STDC__
	#pragma weak tcgetpgrp = _tcgetpgrp
#endif
#include "synonyms.h"
#include <sys/termios.h>
#include <sys/types.h>

pid_t
tcgetpgrp(fd)
int fd;
{
	pid_t pgrp;

	if (tcgetsid(fd) < 0 || ioctl(fd,TIOCGPGRP,&pgrp) < 0)
		return -1;
	return pgrp;
}
