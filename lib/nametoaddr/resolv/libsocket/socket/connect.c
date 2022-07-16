/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:resolv/libsocket/socket/connect.c	1.1"

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
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <errno.h>
#include <sys/tiuser.h>
#include <sys/socketvar.h>
#include <sys/socket.h>
#include <sys/stream.h>
#include <sys/sockmod.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <stdio.h>

extern int	errno;
static int	_rs__connect();

int
_rs_connect(s, name, namelen)
	register int			s;
	register struct sockaddr	*name;
	register int			namelen;
{
	struct _si_user			*siptr;

	if ((siptr = _rs__s_checkfd(s)) == NULL)
		return (-1);

	return (_rs__connect(siptr, name, namelen, 1));
}

static int
_rs__connect(siptr, name, namelen, nameflag)
	struct _si_user			*siptr;
	register struct sockaddr	*name;
	register int			namelen;
	register int			nameflag;
{
	struct t_call			sndcall;
	struct bind_ux			bind_ux;

	memset((caddr_t)&sndcall, 0, sizeof (sndcall));

	if (name && name->sa_family == AF_INET) {
		struct sockaddr_in 	*saddr_in;

		if (namelen < sizeof (*saddr_in)) {
			errno = EINVAL;
			return (-1);
		}
		saddr_in = (struct sockaddr_in *)name;
		memset(&saddr_in->sin_zero, 0, 8);
	}

	if (name && name->sa_family == AF_UNIX) {
		struct stat		rstat;
		struct sockaddr_un	*un;
		int			len;

		if (_rs__s_getfamily(siptr) != AF_UNIX) {
			errno = EINVAL;
			return (-1);
		}
		if (namelen > sizeof (name->sa_family)) {
			un = (struct sockaddr_un *)name;

			if (namelen > sizeof (*un) ||
					(len = _rs__s_uxpathlen(un)) ==
						sizeof (un->sun_path)) {
				errno = EMSGSIZE;
				return (-1);
			}

			un->sun_path[len] = 0;	/* Null terminate */

			/*
			 * Stat the file.
			 */
			if (stat((caddr_t)un->sun_path, &rstat) < 0)
				return (-1);

			if (rstat.st_mode != S_IFIFO) {
				errno = ENOTSOCK;
				return (-1);
			}

			(void)memset((caddr_t)&bind_ux, 0, sizeof (bind_ux));
			bind_ux.extdev = rstat.st_dev;
			bind_ux.extino = rstat.st_ino;
			bind_ux.extsize = sizeof (struct ux_dev);
			if (nameflag == 0)
				namelen = sizeof (name->sa_family);
			(void)memcpy((caddr_t)&bind_ux.name,
				(caddr_t)name, namelen);

			sndcall.addr.buf = (caddr_t)&bind_ux;
			sndcall.addr.len = sizeof (bind_ux);
		} else	{
			sndcall.addr.buf = NULL;
			sndcall.addr.len = 0;
		}
	} else	{
		sndcall.addr.buf = (caddr_t)name;
		sndcall.addr.len = _rs__s_min(namelen, siptr->udata.addrsize);
	}

	return (_rs__connect2(siptr, &sndcall));
}

int
_rs__connect2(siptr, sndcall)
	register struct _si_user	*siptr;
	register struct t_call		*sndcall;
{
	register int			fctlflg;
	register void			(*sigsave)();

	if ((fctlflg = _fcntl(siptr->fd, F_GETFL, 0)) < 0)
		return (-1);

	if (fctlflg & O_NDELAY && siptr->udata.servtype != T_CLTS) {
		/*
		 * Secretly tell sockmod not to pass
		 * up the T_CONN_CON, because we
		 * are not going to wait for it.
		 * (But dont tell anyone - especially
		 * the transport provider).
		 */
		sndcall->opt.len = (ulong)-1;	/* secret sign */
	}

	/*
	 * Must be bound for TPI.
	 */
	if ((siptr->udata.so_state & SS_ISBOUND) == 0) {
		if (_rs__bind(siptr, NULL, 0, NULL, NULL) < 0)
			return (-1);
	}

	/*
	 * Save SIGPOLL until the transaction is complete.
	 */
	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (_rs__s_snd_conn_req(siptr, sndcall) < 0) {
		(void)sigset(SIGPOLL, sigsave);
		return (-1);
	}

	/*
	 * If no delay, return with error if not CLTS.
	 */
	if (fctlflg & O_NDELAY && siptr->udata.servtype != T_CLTS) {
		(void)sigset(SIGPOLL, sigsave);
		errno = EINPROGRESS;
		siptr->udata.so_state |= SS_ISCONNECTING;
		return (-1);
	}

	/*
	 * If CLTS, don't get the connection confirm.
	 */
	if (siptr->udata.servtype == T_CLTS) {
		(void)sigset(SIGPOLL, sigsave);
		if (sndcall->addr.len == 0)
			/*
			 * Connect to Null address, breaks
			 * the connection.
			 */
			siptr->udata.so_state &= ~SS_ISCONNECTED;
		else	siptr->udata.so_state |= SS_ISCONNECTED;
		return (0);
	}

	if (_rs__s_rcv_conn_con(siptr) < 0) {
		(void)sigset(SIGPOLL, sigsave);
		return (-1);
	}
	(void)sigset(SIGPOLL, sigsave);

	siptr->udata.so_state |= SS_ISCONNECTED;

	return (0);
}
