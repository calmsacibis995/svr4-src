/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsocket:getsocknm.c	1.6.5.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <sys/socketvar.h>
#include <sys/socket.h>
#include <sys/tiuser.h>
#include <sys/sockmod.h>
#include <sys/signal.h>

extern int	errno;

int
getsockname(s, name, namelen)
	register int			s;
	register struct sockaddr	*name;
	register int			*namelen;
{
	register struct _si_user	*siptr;

	if ((siptr = _s_checkfd(s)) == NULL)
		return (-1);

	if (name == NULL || namelen == NULL) {
		errno = EINVAL;
		return (-1);
	}

	return (_getsockname(siptr, name, namelen));
}

int
_getsockname(siptr, name, namelen)
	register struct _si_user	*siptr;
	register struct sockaddr	*name;
	register int			*namelen;
{
	register void			(*sigsave)();
	struct netbuf			netbuf;

	netbuf.len = 0;
	netbuf.maxlen = siptr->ctlsize;
	netbuf.buf = siptr->ctlbuf;

	/*
	 * Get it from sockmod.
	 */
	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (_ioctl(siptr->fd, TI_GETMYNAME, (caddr_t)&netbuf) < 0) {
		switch (errno) {
			case ENXIO:
			case EPIPE:
				errno = 0;
				break;

			case ENOTTY:
			case ENODEV:
			case EINVAL:
				errno = ENOTSOCK;
				break;
		}
		if (errno) {
			(void)sigset(SIGPOLL, sigsave);
			return (-1);
		}
	}
	(void)sigset(SIGPOLL, sigsave);

	errno = 0;
	*namelen = _s_cpaddr(siptr, name, *namelen, netbuf.buf, netbuf.len);
	return (0);
}
