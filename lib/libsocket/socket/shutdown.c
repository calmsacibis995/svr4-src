/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsocket:shutdown.c	1.7.5.1"

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
#include <sys/socketvar.h>
#include <sys/socket.h>
#include <sys/tiuser.h>
#include <sys/sockmod.h>
#include <sys/signal.h>

extern int	errno;

int
shutdown(s, how)
	register int		s;
	int			how;
{
	register void		(*sigsave)();
	struct   _si_user	*siptr;

	if ((siptr = _s_checkfd(s)) == (struct _si_user *)NULL)
		return (-1);

	if (how < 0 || how > 2) {
		errno = EINVAL;
		return (-1);
	}

	if ((siptr->udata.so_state & SS_ISCONNECTED) == 0) {
		if (_s_getudata(s, &siptr) < 0)
			return (-1);
		if ((siptr->udata.so_state & SS_ISCONNECTED) == 0) {
			errno = ENOTCONN;
			return (-1);
		}
	}

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (!_s_do_ioctl(s, &how, sizeof (how), SI_SHUTDOWN, 0)) {
		if (errno != EPIPE) {
			(void)sigset(SIGPOLL, sigsave);
			return (-1);
		} else	errno = 0;
	}
	(void)sigset(SIGPOLL, sigsave);

	/*
	 * If we got EPIPE back from the ioctl, then we can
	 * no longer talk to sockmod. The best we can do now
	 * is set our local state and hope the user doesn't
	 * use read/write.
	 */
	if (how == 0 || how == 2)
		siptr->udata.so_state |= SS_CANTRCVMORE;
	if (how == 1 || how == 2)
		siptr->udata.so_state |= SS_CANTSENDMORE;

	return (0);
}
