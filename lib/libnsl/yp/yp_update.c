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


#ident	"@(#)libyp:yp_update.c	1.3.1.1"

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
static char sccsid[] = "@(#)yp_update.c 1.10 88/03/22 Copyr 1986 Sun Micro";
#endif

/*
 * YP updater interface
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include "yp_b.h"
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/ypupd.h>

#define	WINDOW (60*60)
#define	TOTAL_TIMEOUT	300

#ifdef DEBUG
#define	debugging 1
#define	debug(msg)  fprintf(stderr, "%s\n", msg);
#else
#define	debugging 0
#define	debug(msg)
#endif
extern AUTH *authdes_seccreate();

yp_update(domain, map, op, key, keylen, data, datalen)
	char *domain;
	char *map;
	unsigned op;
	char *key;
	int keylen;
	char *data;
	int datalen;
{
	struct ypupdate_args args;
	u_int rslt;
	struct timeval total;
	CLIENT *client;
	char *ypmaster;
	char ypmastername[MAXNETNAMELEN+1];
	enum clnt_stat stat;
	u_int proc;

	switch (op) {
	case YPOP_DELETE:
		proc = YPU_DELETE;
		break;
	case YPOP_INSERT:
		proc = YPU_INSERT;
		break;
	case YPOP_CHANGE:
		proc = YPU_CHANGE;
		break;
	case YPOP_STORE:
		proc = YPU_STORE;
		break;
	default:
		return (YPERR_BADARGS);
	}
	if (yp_master(domain, map, &ypmaster) != 0) {
		debug("no master found");
		return (YPERR_BADDB);
	}

	client = clnt_create(ypmaster, YPU_PROG, YPU_VERS, "circuit_n");
	if (client == NULL) {
		if (debugging) {
			clnt_pcreateerror("client create failed");
		}
		free(ypmaster);
		return (YPERR_RPC);
	}

	if (! host2netname(ypmastername, ypmaster, domain)) {
		clnt_destroy(client);
		free(ypmaster);
		return (YPERR_BADARGS);
	}
	client->cl_auth = authdes_seccreate(ypmastername, WINDOW,
				ypmaster, NULL);
	free(ypmaster);
	if (client->cl_auth == NULL) {
		debug("auth create failed");
		clnt_destroy(client);
		return (YPERR_RPC);
	}

	args.mapname = map;
	args.key.yp_buf_len = keylen;
	args.key.yp_buf_val = key;
	args.datum.yp_buf_len = datalen;
	args.datum.yp_buf_val = data;

	total.tv_sec = TOTAL_TIMEOUT;
	total.tv_usec = 0;
	clnt_control(client, CLSET_TIMEOUT, &total);
	stat = clnt_call(client, proc,
		xdr_ypupdate_args, &args,
		xdr_u_int, &rslt, total);

	if (stat != RPC_SUCCESS) {
		debug("ypupdate RPC call failed");
		if (debugging)
			clnt_perror(client, "ypupdate call failed");
		rslt = YPERR_RPC;
	}
	auth_destroy(client->cl_auth);
	clnt_destroy(client);
	return (rslt);
}
