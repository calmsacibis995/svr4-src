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


#ident	"@(#)librpc:rtime_tli.c	1.2.2.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)rtime_tli.c 1.7 89/04/18 Copyr 1989 Sun Micro";
#endif 

/*
 * rtime_tli.c - get time from remote machine
 *
 * gets time, obtaining value from host
 * on the (udp, tcp)/time tli connection. Since timeserver returns
 * with time of day in seconds since Jan 1, 1900, must
 * subtract seconds before Jan 1, 1970 to get
 * what unix uses.
 */
#include <rpc/rpc.h>
#include <sys/errno.h>
#include <rpc/nettype.h>
#include <netdir.h>
#include <stdio.h>
#include <sys/byteorder.h>

#ifdef DEBUG
#define debug(msg)	t_error(msg)
#else
#define debug(msg)
#endif

#define NYEARS	(1970 - 1900)
#define TOFFSET ((u_long)60*60*24*(365*NYEARS + (NYEARS/4)))

extern int errno;

/*
 * This is based upon the internet time server, but it contacts it by
 * using TLI instead of socket.
 */
rtime_tli(host, timep, timeout)
	char *host;
	struct timeval *timep;
	struct timeval *timeout;
{
	unsigned long thetime;
	int flag;
	struct nd_addrlist *nlist = NULL;
	struct nd_hostserv rpcbind_hs;
	struct netconfig *nconf = NULL;
	int foundit = 0;
	int fd = -1;
	
	nconf = _rpc_getconfip(timeout == NULL ? "tcp" : "udp");
	if (nconf == (struct netconfig *)NULL)
		goto error;

	if ((fd = t_open(nconf->nc_device, O_RDWR, NULL)) == -1) {
		debug("open");
		goto error;
	}
	if (t_bind(fd, (struct t_info *)NULL, (struct t_info *)NULL) < 0) {
		debug("bind");
		goto error;
	}

	/* Get the address of the rpcbind */
	rpcbind_hs.h_host = host;
	rpcbind_hs.h_serv = "time";
	/* Basically get the address of the remote machine on IP */
	if (netdir_getbyname(nconf, &rpcbind_hs, &nlist))
		goto error;

	if (nconf->nc_semantics == NC_TPI_CLTS) {
		struct t_unitdata tu_data;
		fd_set readfds;
		int res;

		tu_data.addr = *nlist->n_addrs;
		tu_data.udata.buf = (char *)&thetime;
		tu_data.udata.len = sizeof(thetime);
		tu_data.udata.maxlen = tu_data.udata.len;
		tu_data.opt.len = 0;
		tu_data.opt.maxlen = 0;
		if (t_sndudata(fd, &tu_data) == -1) {
			debug("udp");
			goto error;
		}
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		do {
			res = select(_rpc_dtbsize(), &readfds, (fd_set *)NULL, 
					(fd_set *)NULL, timeout);
		} while (res < 0 && errno == EINTR);
		if (res <= 0)
			goto error;
		if (t_rcvudata(fd, &tu_data, &flag) < 0) {
			debug("udp");
			goto error;
		}
		foundit = 1;
	} else {
		struct t_call sndcall;

		sndcall.addr = *nlist->n_addrs;
		sndcall.opt.len = sndcall.opt.maxlen = 0;
		sndcall.udata.len = sndcall.udata.maxlen = 0;

		if (t_connect(fd, &sndcall, NULL) == -1) {
			debug("tcp");
			goto error;
		}
		if (t_rcv(fd, (char *)&thetime, sizeof(thetime), &flag)
				!= sizeof(thetime)) {
			debug("tcp");
			goto error;
		}
		foundit = 1;
	}

	thetime = ntohl(thetime);
	timep->tv_sec = thetime - TOFFSET;
	timep->tv_usec = 0;

error:
	if (nconf) {
		(void) freenetconfigent(nconf);
		if (fd != -1) {
			(void) t_close(fd);
			if (nlist)
				netdir_free((char *)nlist, ND_ADDRLIST);
		}
	}
	return (foundit);
}
