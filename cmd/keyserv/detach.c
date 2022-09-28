/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)keyserv:detach.c	1.1.2.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)detach.c 1.2 89/03/19 Copyr 1986 Sun Micro";
#endif
/*
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

#include <sys/termios.h>
#include <fcntl.h>

/*
 * detach from tty
 */
detachfromtty()
{
	int tt;

	close(0);
	close(1);
	close(2);
	switch (fork()) {
	case -1:
		perror("fork");
		break;
	case 0:
		break;
	default:
		exit(0);
	}
#ifdef TIOCNOTTY
	tt = open("/dev/tty", O_RDWR);
	if (tt > 0) {
		ioctl(tt, TIOCNOTTY, 0);
		close(tt);
	}
#else
	setsid(); /*sys5 stuff*/
#endif
	(void)open("/dev/null", O_RDWR, 0);
	dup(0);
	dup(0);
}
 

