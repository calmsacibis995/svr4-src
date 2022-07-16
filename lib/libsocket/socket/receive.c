/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsocket:receive.c	1.5.5.1"

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
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/tiuser.h>
#include <sys/sockmod.h>

int
recvmsg(s, msg, flags)
	register int			s;
	register struct msghdr		*msg;
	register int			flags;
{
	register struct _si_user	*siptr;

	if ((siptr = _s_checkfd(s)) == NULL)
		return (-1);

	return (_s_soreceive(siptr, msg, flags));
}

int
recvfrom(s, buf, len, flags, from, fromlen)
	register int			s;
	register char			*buf;
	register u_long			len;
	register int			flags;
	register caddr_t		from;
	register int			*fromlen;
{
	register struct _si_user	*siptr;
	register int			retlen;
	struct iovec			msg_iov[1];
	struct msghdr			msg;

	if ((siptr = _s_checkfd(s)) == NULL)
		return (-1);

	msg.msg_iovlen = 1;
	msg.msg_iov = msg_iov;
	msg.msg_iov[0].iov_base = buf;
	msg.msg_iov[0].iov_len = len;
	msg.msg_namelen = *fromlen;
	msg.msg_name = from;
	msg.msg_accrightslen = 0;
	msg.msg_accrights = NULL;

	if ((retlen = _s_soreceive(siptr, &msg, flags)) < 0)
		return (-1);
	if (fromlen != NULL)
		*fromlen = msg.msg_namelen;

	return (retlen);
}

int
recv(s, buf, len, flags)
	register int			s;
	register char			*buf;
	register u_long			len;
	register int			flags;
{
	register struct _si_user	*siptr;
	struct msghdr			msg;
	struct iovec			msg_iov[1];

	if ((siptr = _s_checkfd(s)) == NULL)
		return (-1);

	msg.msg_iovlen = 1;
	msg.msg_iov = msg_iov;
	msg.msg_iov[0].iov_base = buf;
	msg.msg_iov[0].iov_len = len;
	msg.msg_namelen = 0;
	msg.msg_name = NULL;
	msg.msg_accrightslen = 0;
	msg.msg_accrights = NULL;

	return (_s_soreceive(siptr, &msg, flags));
}
