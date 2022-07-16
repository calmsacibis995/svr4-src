/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:resolv/libsocket/socket/socket.c	1.1"

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
#include <sys/socket.h>
#include <sys/tiuser.h>
#include <sys/sockmod.h>
#include <sys/signal.h>
#include <fcntl.h>

extern int	errno;

int
_rs_socket(family, type, protocol)
	register int			family;
	register int			type;
	register int			protocol;
{
	register struct _si_user	*siptr;
	register struct netconfig	*net;
	void				*nethandle;

	if ((net = _rs__s_match(family, type, protocol, &nethandle)) == NULL)
		return (-1);

	if (strcmp(net->nc_proto, NC_NOPROTO) != 0)
		protocol = 0;

	/*
	 * Do common open.
	 */
	siptr = _rs__s_open(net->nc_device, protocol);
	endnetconfig(nethandle); /* finished with netconfig struct */

	if (siptr == NULL)
		return (-1);

	siptr->family  = family;

	return (siptr->fd);
}
