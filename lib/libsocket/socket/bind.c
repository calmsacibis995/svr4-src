/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsocket:bind.c	1.9.6.3"

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
#include <netinet/in.h>
#include <sys/stat.h>
#include <syslog.h>

extern int	errno;
static int	_unbind();

int
bind(s, name, namelen)
	register int			s;
	register struct sockaddr	*name;
	register int			namelen;
{
	register struct _si_user	*siptr;

	if ((siptr = _s_checkfd(s)) == NULL)
		return (-1);

	if (name == NULL || namelen == 0) {
		errno = EINVAL;
		return (-1);
	}

	if (siptr->udata.so_state & SS_ISBOUND) {
		errno = EINVAL;
		return (-1);
	}

	/*
	 * In UNIX domain a bind address must be given.
	 */
	if (name->sa_family == AF_UNIX) {
		if (namelen <= sizeof (name->sa_family)) {
			errno = EISDIR;
			return (-1);
		}
		if (namelen > sizeof (struct sockaddr_un)) {
			errno = EINVAL;
			return (-1);
		}
		if (name->sa_data[0] == 0) {
			errno = EINVAL;
			return (-1);
		}
	}
	return (_bind(siptr, name, namelen, NULL, NULL));
}

_bind(siptr, name, namelen, raddr, raddrlen)
	register struct _si_user	*siptr;
	register struct sockaddr	*name;
	register int			namelen;
	register char			*raddr;
	register int			*raddrlen;
{
	register char			*buf;
	register struct T_bind_req	*bind_req;
	register struct T_bind_ack	*bind_ack;
	register int			size;
	register void			(*sigsave)();
	struct stat			rstat;
	struct bind_ux			bind_ux;
	int				fflag;

	if (_s_getfamily(siptr) == AF_UNIX) {
		(void)memset((caddr_t)&bind_ux, 0, sizeof (bind_ux));

		if (name == NULL) {
			bind_ux.name.sun_family = AF_UNIX;
			fflag = 0;
		} else	{
			struct sockaddr_un	*un;
			int			len;

			un = (struct sockaddr_un *)name;

			if (un->sun_family != AF_UNIX) {
				errno = EINVAL;
				return (-1);
			}
			if (namelen > sizeof (*un) ||
					(len = _s_uxpathlen(un)) ==
						sizeof (un->sun_path)) {
				errno = EMSGSIZE;
				return (-1);
			}
			un->sun_path[len] = 0;	/* Null terminate */

			/*
			 * We really have to bind to the actual vnode, so
			 * that renames will not cause a problem. First
			 * create the file and then get the dev/ino to
			 * bind to.
			 */
			if (mknod(un->sun_path, S_IFIFO, 0) < 0 ||
				stat((caddr_t)un->sun_path, &rstat) < 0) {
				if (errno == EEXIST)
					errno = EADDRINUSE;
				return (-1);
			}

			bind_ux.extdev = rstat.st_dev;
			bind_ux.extino = rstat.st_ino;
			bind_ux.extsize= sizeof (struct ux_dev);

			(void)memcpy((caddr_t)&bind_ux.name, (caddr_t)name,
								namelen);
			fflag = 1;
		}
		name = (struct sockaddr *)&bind_ux;
		namelen = sizeof (bind_ux);
	} else	namelen = _s_min(namelen, siptr->udata.addrsize);

	buf = siptr->ctlbuf;
	bind_req = (struct T_bind_req *)buf;
	size = sizeof (*bind_req);

	bind_req->PRIM_type = T_BIND_REQ;
	bind_req->ADDR_length = name == NULL ? 0 : namelen;
	bind_req->ADDR_offset = 0;
	bind_req->CONIND_number = 0;

	if ((int)bind_req->ADDR_length > 0 && buf != (char *)NULL) {
		_s_aligned_copy(buf, bind_req->ADDR_length, size,
				(caddr_t)name,
				&bind_req->ADDR_offset);
		size = bind_req->ADDR_offset + bind_req->ADDR_length;
	}

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (!_s_do_ioctl(siptr->fd, buf, size, TI_BIND, NULL)) {
		(void)sigset(SIGPOLL, sigsave);
		return (-1);
	}
	(void)sigset(SIGPOLL, sigsave);

	bind_ack = (struct T_bind_ack *)buf;
	buf += bind_ack->ADDR_offset;

	/*
	 * Check that the address returned by the
	 * transport provider meets the criteria.
	 */
	errno = 0;
	if (name != (struct sockaddr *)NULL) {
		/*
		 * Some programs like inetd(8) don't set the
		 * family field.
		 */
		switch (_s_getfamily(siptr)) {
		case AF_INET: {
			struct sockaddr_in	*rname;
			struct sockaddr_in	*aname;

			rname = (struct sockaddr_in *)buf;
			aname = (struct sockaddr_in *)name;

			if (aname->sin_port != 0 &&
				aname->sin_port != rname->sin_port)
				errno = EADDRINUSE;

			if (aname->sin_addr.s_addr != INADDR_ANY &&
			    aname->sin_addr.s_addr != rname->sin_addr.s_addr)
				errno = EADDRNOTAVAIL;
			break;
		}

		case AF_UNIX:
			if (fflag) {

				register struct bind_ux	*rbind_ux;
				rbind_ux = (struct bind_ux *)buf;
				if (rbind_ux->extdev != bind_ux.extdev ||
					rbind_ux->extino != bind_ux.extino)
					errno = EADDRINUSE;
			}
			break;

		default:
			if (bind_ack->ADDR_length != namelen)
				errno = EADDRNOTAVAIL;
			else	if (memcmp((caddr_t)name, buf, namelen) != 0)
					errno = EADDRNOTAVAIL;
		}
	}

	if (errno) {
		register int error;

		error = errno;			/* Save it */
		(void)_unbind(siptr);
		if (name && name->sa_family == AF_UNIX && fflag)
			(void)unlink(name->sa_data);
		errno = error;
		return (-1);
	}

	/*
	 * Copy back the bound address if requested.
	 */
	if (raddr != NULL)
		*raddrlen = _s_cpaddr(siptr, raddr, *raddrlen,
				buf, bind_ack->ADDR_length);

	siptr->udata.so_state |= SS_ISBOUND;

	return (0);
}

static int
_unbind(siptr)
	register struct _si_user	*siptr;
{
	register void			(*sigsave)();

	((struct T_unbind_req *)siptr->ctlbuf)->PRIM_type = T_UNBIND_REQ;

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (!_s_do_ioctl(siptr->fd, siptr->ctlbuf,
				sizeof (struct T_unbind_req),
					TI_UNBIND, NULL)) {
		(void)sigset(SIGPOLL, sigsave);
		return (-1);
	}
	(void)sigset(SIGPOLL, sigsave);

	siptr->udata.so_state &= ~SS_ISBOUND;
	return (0);
}
