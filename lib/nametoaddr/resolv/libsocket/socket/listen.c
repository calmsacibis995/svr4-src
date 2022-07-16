/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:resolv/libsocket/socket/listen.c	1.1.1.1"

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
#include <syslog.h>
#include "../../resolvabi.h"

extern int	errno;

/* We make the socket module do the unbind,
 * if necessary, to make the timing window
 * of error as small as possible.
 */
int
_rs_listen(s, qlen)
	register int			s;
	register int			qlen;
{
	register char			*buf;
	register struct T_bind_req	*bind_req;
	register void			(*sigsave)();
	register struct _si_user	*siptr;

	if ((siptr = _rs__s_checkfd(s)) == NULL)
		return (-1);

	if (siptr->udata.servtype == T_CLTS) {
		errno = EOPNOTSUPP;
		return (-1);
	}

	buf = siptr->ctlbuf;
	bind_req = (struct T_bind_req *)buf;

	bind_req->PRIM_type = T_BIND_REQ;
	bind_req->ADDR_offset = sizeof (*bind_req);
	bind_req->CONIND_number = qlen;

	if ((siptr->udata.so_state & SS_ISBOUND) == 0) {
		int	family;

		/*
		 * Must have been explicitly bound in the UNIX domain.
		 */
		if ((family = _rs__s_getfamily(siptr)) == AF_UNIX) {
			errno = EINVAL;
			return (-1);
		}

		(void)memcpy(buf + bind_req->ADDR_offset, (caddr_t)&family,
				sizeof (short));
		bind_req->ADDR_length = sizeof (short);
	} else	bind_req->ADDR_length = siptr->udata.addrsize;

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (!_rs__s_do_ioctl(s, siptr->ctlbuf, sizeof (*bind_req) +
				bind_req->ADDR_length, SI_LISTEN, NULL)) {
		(void)sigset(SIGPOLL, sigsave);
		return (-1);
	}
	(void)sigset(SIGPOLL, sigsave);

	siptr->udata.so_options |= SO_ACCEPTCONN;

	return (0);
}
