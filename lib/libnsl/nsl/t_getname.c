/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libnsl:nsl/t_getname.c	1.1.3.1"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/tihdr.h"
#include "sys/tiuser.h"
#include "sys/timod.h"
#include "sys/signal.h"
#include "_import.h"


extern int t_errno;
extern int errno;
extern void (*sigset())();
extern int ioctl();

t_getname(fd, name, type)
int fd;
struct netbuf *name;
register int type;
{
	void (*sigsave)();

	if (!name || ((type != LOCALNAME) && (type != REMOTENAME))) {
		errno = EINVAL;
		return(-1);
	}

	if (_t_checkfd(fd) == 0)
		return(-1);

	sigsave = sigset(SIGPOLL, SIG_HOLD);

	if (type == LOCALNAME) {
		if (ioctl(fd, TI_GETMYNAME, name) < 0) {
			sigset(SIGPOLL, sigsave);
			t_errno = TSYSERR;
			return(-1);
		}
	} else {	/* REMOTENAME */
		if (ioctl(fd, TI_GETPEERNAME, name) < 0) {
			sigset(SIGPOLL, sigsave);
			t_errno = TSYSERR;
			return(-1);
		}
	}

	sigset(SIGPOLL, sigsave);

	return(0);
}
