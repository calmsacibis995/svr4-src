/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nfs.cmds:nfs/automount/bindresvport.c	1.2.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 *     Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 *
 *  (c) 1986,1987,1988.1989  Sun Microsystems, Inc
 *  (c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *            All rights reserved.
 *
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <rpc/nettype.h>

#define STARTPORT 600
#define ENDPORT (IPPORT_RESERVED - 1)
#define NPORTS	(ENDPORT - STARTPORT + 1)

/*
 * The argument is a client handle for a UDP connection.
 * Unbind its transport endpoint from the existing port
 * and rebind it to a reserved port.
 */
bindresvport(cl)
	CLIENT *cl;
{
	int fd;
	int res;
	static short port;
	struct sockaddr_in *sin;
	extern int errno;
	extern int t_errno;
	struct t_bind *tbind, *tres;
	int i;
	struct netconfig *nconf;
	struct netconfig *getnetconfigent();

	/* make sure it's a UDP connection */
	nconf = getnetconfigent(cl->cl_netid);
	if (nconf == NULL)
		return (-1);
        if ((nconf->nc_semantics != NC_TPI_CLTS) ||
            strcmp(nconf->nc_protofmly, NC_INET) ||
            strcmp(nconf->nc_proto, NC_UDP)) {
		freenetconfigent(nconf);
		return (0);	/* not udp - don't need resv port */
	}
	freenetconfigent(nconf);

	/* reserved ports are for superusers only */
	if (geteuid()) {
		errno = EACCES;
		return (-1);
	}

	if (!clnt_control(cl, CLGET_FD, &fd)) {
		return (-1);
	}

	/* If fd is already bound - unbind it */
	if (t_getstate(fd) != T_UNBND) {
		if (t_unbind(fd) < 0) {
			return (-1);
		}
	}

	tbind = (struct t_bind *) t_alloc(fd, T_BIND, T_ADDR);
	if (tbind == NULL) {
		if (t_errno == TBADF)
			errno = EBADF;
		return (-1);
	}
	tres = (struct t_bind *) t_alloc(fd, T_BIND, T_ADDR);
	if (tres == NULL) {
		(void) t_free((char *) tbind, T_BIND);
		return (-1);
	}

	(void) memset((char *) tbind->addr.buf, 0, tbind->addr.len);
	/* warning: this sockaddr_in is truncated to 8 bytes */
	sin = (struct sockaddr_in *) tbind->addr.buf;
	sin->sin_family = AF_INET;

	tbind->qlen = 0;
	tbind->addr.len = tbind->addr.maxlen;

	/* Need to find a reserved port in the interval
	 * STARTPORT - ENDPORT.  Choose a random starting
	 * place in the interval based on the process pid
	 * and sequentially search the ports for one
	 * that is available.
	 */
	port = (getpid() % NPORTS) + STARTPORT;
	res = -1;
	errno = EADDRINUSE;

	for (i = 0; i < NPORTS && errno == EADDRINUSE; i++) {
		sin->sin_port = htons(port++);
		if (port > ENDPORT)
			port = STARTPORT;
		res = t_bind(fd, tbind, tres);
		if ((res == 0) && (memcmp(tbind->addr.buf, tres->addr.buf,
					(int) tres->addr.len) == 0))
			break;
	}

	(void) t_free((char *) tbind, T_BIND);
	(void) t_free((char *) tres,  T_BIND);
	return (res);
}
