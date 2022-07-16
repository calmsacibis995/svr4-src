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


#ident	"@(#)libyp:yp_all.c	1.2.1.1"

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
static  char sccsid[] = "@(#)yp_all.c 1.8 88/06/11 Copyr 1985 Sun Micro";
#endif

#define NULL 0
#include <rpc/rpc.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_ERR 3
#endif /* SYSLOG */
#include "yp_b.h"
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>


static struct timeval tp_timout = {
	120,				/* 120 seconds */
	0
	};
extern int _yp_dobind();
extern char *malloc();
extern size_t strlen();
extern int syslog();

/*
 * This does the "glommed enumeration" stuff.  callback->foreach is the name
 * of a function which gets called per decoded key-value pair:
 * 
 * (*callback->foreach)(status, key, keylen, val, vallen, callback->data);
 *
 * If the server we get back from _yp_dobind speaks the old protocol, this
 * returns YPERR_VERS, and does not attempt to emulate the new functionality
 * by using the old protocol.
 */
int
yp_all (domain, map, callback)
	char *domain;
	char *map;
	struct ypall_callback *callback;
{
	int domlen;
	int maplen;
	struct ypreq_nokey req;
	int reason;
	struct dom_binding *pdomb;
	enum clnt_stat s;
	CLIENT *allc;

	if ( (map == NULL) || (domain == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = (int) strlen(domain);
	maplen = (int) strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ||
	    (callback == (struct ypall_callback *) NULL) ) {
		return(YPERR_BADARGS);
	}

	if (reason = _yp_dobind(domain, &pdomb) ) {
		return(reason);
	}

	if (pdomb->dom_binding->ypbind_hi_vers < YPVERS) {
		return (YPERR_VERS);
	}
		

	if ((allc = clnt_create(pdomb->dom_binding->ypbind_servername, YPPROG, 
	    YPVERS, "circuit_n")) == (CLIENT *) NULL) {
		    (void) syslog(LOG_ERR,
		    clnt_spcreateerror("yp_all - transport level create failure"));
		    return(YPERR_RPC);
	}

	req.domain = domain;
	req.map = map;
	
	s = clnt_call(allc, YPPROC_ALL, xdr_ypreq_nokey, &req,
	    xdr_ypall, callback, tp_timout);

	if (s != RPC_SUCCESS) {
		(void) syslog(LOG_ERR, clnt_sperror(pdomb->dom_client,
		    "yp_all - RPC clnt_call (transport level) failure"));
	}

	clnt_destroy(allc);	
	if (s == RPC_SUCCESS) {
		return(0);
	} else {
		return(YPERR_RPC);
	}
}

