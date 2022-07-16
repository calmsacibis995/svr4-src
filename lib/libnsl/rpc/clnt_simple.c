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


#ident	"@(#)librpc:clnt_simple.c	1.2.2.1"

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
static char sccsid[] = "@(#)clnt_simple.c 1.49 89/01/31 Copyr 1984 Sun Micro";
#endif

/* 
 * clnt_simple.c
 * Simplified front end to client rpc.
 *
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <string.h>
#include <sys/param.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

extern char *calloc();
extern char *malloc();
extern int t_errno, errno;

static struct rpc_call_private {
	int	valid;			/* Is this entry valid ? */
	CLIENT	*client;		/* Client handle */
	u_long	prognum, versnum;	/* Program, version */
	char	host[MAXHOSTNAMELEN];	/* Servers host */
	char	nettype[32];		/* Network type */
} *rpc_call_private;

/*
 * This is the simplified interface to the client rpc layer.
 * The client handle is not destroyed here and is reused for
 * the future calls to same prog, vers, host and nettype combination.
 *
 * The total time available is 25 seconds.
 */
enum clnt_stat
rpc_call(host, prognum, versnum, procnum, inproc, in, outproc, out, nettype)
	char *host;			/* host name */
	u_long prognum;			/* program number */
	u_long versnum;			/* version number */
	u_long procnum;			/* procedure number */
	xdrproc_t inproc, outproc;	/* in/out XDR procedures */
	char *in, *out;			/* recv/send data */
	char *nettype;			/* nettype */
{
	register struct rpc_call_private *rcp = rpc_call_private;
	enum clnt_stat clnt_stat;
	struct timeval tottimeout;

	if (rcp == (struct rpc_call_private *)NULL) {
		rcp = (struct rpc_call_private *)calloc(1, sizeof (*rcp));
		if (rcp == (struct rpc_call_private *)NULL) {
			rpc_createerr.cf_stat = RPC_SYSTEMERROR;
			rpc_createerr.cf_error.re_errno = errno;
			return (rpc_createerr.cf_stat);
		}
		rpc_call_private = rcp;
	}
	if ((nettype == NULL) || (nettype[0] == NULL))
		nettype = "netpath";
	if (!(rcp->valid && rcp->prognum == prognum
		&& rcp->versnum == versnum
		&& (!strcmp(rcp->host, host))
		&& (!strcmp(rcp->nettype, nettype)))) {
		int fd;
		struct t_info tinfo;

		rcp->valid = 0;
		if (rcp->client)
			CLNT_DESTROY(rcp->client);
		/*
		 * Using the first successful transport for that type
		 */
		rcp->client = clnt_create(host, prognum, versnum, nettype);
		if (rcp->client == (CLIENT *)NULL)
			return (rpc_createerr.cf_stat);
		(void) CLNT_CONTROL(rcp->client, CLGET_FD, &fd);
		if (t_getinfo(fd, &tinfo) != -1) {
			if (tinfo.servtype == T_CLTS) {
				struct timeval timeout;

				/*
				 * Set time outs for connectionless case
				 */
				timeout.tv_usec = 0;
				timeout.tv_sec = 5;
				(void) CLNT_CONTROL(rcp->client,
					CLSET_RETRY_TIMEOUT, &timeout);
			}
		} else {
			rpc_createerr.cf_stat = RPC_TLIERROR;
			rpc_createerr.cf_error.re_terrno = t_errno;
			return (rpc_createerr.cf_stat);
		}
		rcp->prognum = prognum;
		rcp->versnum = versnum;
		(void) strcpy(rcp->host, host);
		(void) strcpy(rcp->nettype, nettype);
		rcp->valid = 1;
	} /* else reuse old client */		
	tottimeout.tv_sec = 25;
	tottimeout.tv_usec = 0;
	clnt_stat = CLNT_CALL(rcp->client, procnum, inproc, in, outproc,
				out, tottimeout);
	/* 
	 * if call failed, empty cache
	 */
	if (clnt_stat != RPC_SUCCESS)
		rcp->valid = 0;
	return (clnt_stat);
}
