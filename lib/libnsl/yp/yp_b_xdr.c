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


#ident	"@(#)libyp:yp_b_xdr.c	1.2.1.1"


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
#include <rpc/rpc.h>
#include <netconfig.h>
#include "yp_b.h"

bool_t
xdr_ypbind_resptype(xdrs, objp)
	XDR *xdrs;
	ypbind_resptype *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}


#define YPBIND_ERR_ERR 1		/* Internal error */
#define YPBIND_ERR_NOSERV 2		/* No bound server for passed domain */
#define YPBIND_ERR_RESC 3		/* System resource allocation failure */
#define YPBIND_ERR_NODOMAIN 4		/* Domain doesn't exist */


bool_t
xdr_ypbind_domain(xdrs, objp)
	XDR *xdrs;
	ypbind_domain *objp;
{
	if (!xdr_string(xdrs, &objp->ypbind_domainname, ~0)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->ypbind_vers)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_ypbind_binding(xdrs, objp)
	XDR *xdrs;
	ypbind_binding *objp;
{
	if (!xdr_pointer(xdrs, (char **)&objp->ypbind_nconf, sizeof(struct netconfig), xdr_netconfig)) {
		return (FALSE);
	}
	if (!xdr_pointer(xdrs, (char **)&objp->ypbind_svcaddr, sizeof(struct netbuf), xdr_netbuf)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->ypbind_servername, ~0)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->ypbind_hi_vers)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->ypbind_lo_vers)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_ypbind_resp(xdrs, objp)
	XDR *xdrs;
	ypbind_resp *objp;
{
	if (!xdr_ypbind_resptype(xdrs, &objp->ypbind_status)) {
		return (FALSE);
	}
	switch (objp->ypbind_status) {
	case YPBIND_FAIL_VAL:
		if (!xdr_u_long(xdrs, &objp->ypbind_resp_u.ypbind_error)) {
			return (FALSE);
		}
		break;
	case YPBIND_SUCC_VAL:
		if (!xdr_pointer(xdrs, (char **)&objp->ypbind_resp_u.ypbind_bindinfo, sizeof(ypbind_binding), xdr_ypbind_binding)) {
			return (FALSE);
		}
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_ypbind_setdom(xdrs, objp)
	XDR *xdrs;
	ypbind_setdom *objp;
{
	if (!xdr_string(xdrs, &objp->ypsetdom_domain, ~0)) {
		return (FALSE);
	}
	if (!xdr_pointer(xdrs, (char **)&objp->ypsetdom_bindinfo, sizeof(ypbind_binding), xdr_ypbind_binding)) {
		return (FALSE);
	}
	return (TRUE);
}


