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


#ident	"@(#)libyp:yp_order.c	1.2.1.1"

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
static  char sccsid[] = "@(#)yp_order.c 1.7 88/02/08 Copyr 1985 Sun Micro";
#endif

#define NULL 0
#include <rpc/rpc.h>
#include "yp_b.h"
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

static int doorder();

extern struct timeval _ypserv_timeout;
extern int _yp_dobind();
extern unsigned int _ypsleeptime;
extern char *malloc();
extern unsigned sleep();
extern size_t strlen();

/*
 * This checks parameters, and implements the outer "until binding success"
 * loop.
 */
int
yp_order (domain, map, order)
	char *domain;
	char *map;
	unsigned long *order;
{
	int domlen;
	int maplen;
	int reason;
	struct dom_binding *pdomb;

	if ( (map == NULL) || (domain == NULL) ) {
		return (YPERR_BADARGS);
	}
	
	domlen = (int) strlen(domain);
	maplen = (int) strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ||
	    (order == NULL) ) {
		return (YPERR_BADARGS);
	}

	for (;;) {
		
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return (reason);
		}

		if (pdomb->dom_binding->ypbind_hi_vers >= YPVERS) {

			reason = doorder(domain, map, pdomb, _ypserv_timeout,
			    order);

			if (reason == YPERR_RPC) {
				yp_unbind(domain);
				(void) sleep(_ypsleeptime);
			} else {
				break;
			}
		} else {
			return(YPERR_VERS);
		}
	}
	
	return (reason);

}

/*
 * This talks v3 to ypserv
 */
static int
doorder (domain, map, pdomb, timeout, order)
	char *domain;
	char *map;
	struct dom_binding *pdomb;
	struct timeval timeout;
	unsigned long *order;
{
	struct ypreq_nokey req;
	struct ypresp_order resp;
	unsigned int retval = 0;

	req.domain = domain;
	req.map = map;
	memset((char *)&resp, 0, sizeof(struct ypresp_order));

	/*
	 * Do the get_order request.  If the rpc call failed, return with
	 * status from this point.  
	 */
	
	if(clnt_call(pdomb->dom_client, YPPROC_ORDER, xdr_ypreq_nokey, &req,
	    xdr_ypresp_order, &resp, timeout) != RPC_SUCCESS) {
		return (YPERR_RPC);
	}

	/* See if the request succeeded */
	
	if (resp.status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.status);
	}

	*order = resp.ordernum;
	CLNT_FREERES(pdomb->dom_client, xdr_ypresp_order, &resp);
	return (retval);

}
