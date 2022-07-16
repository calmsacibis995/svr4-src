/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsocket:setsockopt.c	1.6.5.1"

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
#include <sys/socket.h>
#include <sys/tiuser.h>
#include <sys/sockmod.h>
#include <sys/signal.h>

extern int	errno;

int
setsockopt(s, level, optname, optval, optlen)
	register int			s;
	register int			level;
	register int			optname;
	register char			*optval;
	register int			optlen;
{
	register struct _si_user	*siptr;

	if ((siptr = _s_checkfd(s)) == NULL)
		return (-1);

	return (_setsockopt(siptr, level, optname, optval, optlen));
}

int
_setsockopt(siptr, level, optname, optval, optlen)
	struct _si_user			*siptr;
	register int			level;
	register int			optname;
	register char			*optval;
	register int			optlen;
{
	register char			*buf;
	register struct T_optmgmt_req	*opt_req;
	register int			s;
	register void			(*sigsave)();
	register int			size;
	register struct opthdr		*opt;

	s = siptr->fd;

	buf = siptr->ctlbuf;
	opt_req = (struct T_optmgmt_req *)buf;
	opt_req->PRIM_type = T_OPTMGMT_REQ;
	opt_req->OPT_length = sizeof (*opt) + optlen;
	opt_req->OPT_offset = sizeof (*opt_req);
	opt_req->MGMT_flags = T_NEGOTIATE;

	opt = (struct opthdr *)(buf + sizeof (*opt_req));
	opt->level = level;
	opt->name = optname;
	opt->len = optlen;
	(void)memcpy((caddr_t)opt + sizeof (*opt), optval, optlen);

	size = opt_req->OPT_offset + opt_req->OPT_length;

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (!_s_do_ioctl(s, buf, size, TI_OPTMGMT, 0)) {
		(void)sigset(SIGPOLL, sigsave);
		return (-1);
	}
	(void)sigset(SIGPOLL, sigsave);

	return (0);
}
