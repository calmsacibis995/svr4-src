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


#ident	"@(#)librpc:rpcb_prot.c	1.4.1.1"

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
static char sccsid[] = "@(#)rpcb_prot.c 1.9 89/04/21 Copyr 1984 Sun Micro";
#endif

/*
 * rpcb_prot.c
 * XDR routines for the rpcbinder version 3.
 *
 * Copyright (C) 1984, 1988, Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/rpcb_prot.h>


bool_t
xdr_rpcb(xdrs, objp)
	XDR *xdrs;
	RPCB *objp;
{
	if (!xdr_u_long(xdrs, &objp->r_prog)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->r_vers)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->r_netid, ~0)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->r_addr, ~0)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->r_owner, ~0)) {
		return (FALSE);
	}
	return (TRUE);
}

/*
 * What is going on with linked lists? (!)
 * First recall the link list declaration from rpc_prot.h:
 *
 * struct rpcblist {
 *	RPCB rpcb_map;
 *	struct rpcblist *rpcb_next
 * };
 *
 * Compare that declaration with a corresponding xdr declaration that
 * is (a) pointer-less, and (b) recursive:
 *
 * typedef union switch (bool_t) {
 *
 *	case TRUE: struct {
 *		struct rpcb;
 * 		rpcblist_t foo;
 *	};
 *
 *	case FALSE: struct {};
 * } rpcblist_t;
 *
 * Notice that the xdr declaration has no nxt pointer while
 * the C declaration has no bool_t variable.  The bool_t can be
 * interpreted as ``more data follows me''; if FALSE then nothing
 * follows this bool_t; if TRUE then the bool_t is followed by
 * an actual struct rpcb, and then (recursively) by the
 * xdr union, rpcblist_t.
 *
 * This could be implemented via the xdr_union primitive, though this
 * would cause a one recursive call per element in the list.  Rather than do
 * that we can ``unwind'' the recursion
 * into a while loop and do the union arms in-place.
 *
 * The head of the list is what the C programmer wishes to past around
 * the net, yet is the data that the pointer points to which is interesting;
 * this sounds like a job for xdr_reference!
 */
/*
 * And this one encodes and decode the tli version of the mappings.
 */
bool_t
xdr_rpcblist(xdrs, rp)
	register XDR *xdrs;
	register RPCBLIST **rp;
{
	/*
	 * more_elements is pre-computed in case the direction is
	 * XDR_ENCODE or XDR_FREE.  more_elements is overwritten by
	 * xdr_bool when the direction is XDR_DECODE.
	 */
	bool_t more_elements;
	register int freeing = (xdrs->x_op == XDR_FREE);
	register RPCBLIST **next;

	while (TRUE) {
		more_elements = (bool_t)(*rp != NULL);
		if (! xdr_bool(xdrs, &more_elements))
			return (FALSE);
		if (! more_elements)
			return (TRUE);  /* we are done */
		/*
		 * the unfortunate side effect of non-recursion is that in
		 * the case of freeing we must remember the next object
		 * before we free the current object ...
		 */
		if (freeing)
			next = &((*rp)->rpcb_next);
		if (! xdr_reference(xdrs, (caddr_t *)rp,
		    (u_int)sizeof (RPCBLIST), xdr_rpcb))
			return (FALSE);
		rp = (freeing) ? next : &((*rp)->rpcb_next);
	}
}

/*
 * XDR remote call arguments
 * written for XDR_ENCODE direction only
 */
bool_t
xdr_rpcb_rmtcallargs(xdrs, objp)
	XDR *xdrs;
	struct rpcb_rmtcallargs *objp;
{
	u_int lenposition, argposition, position;

	if (!xdr_u_long(xdrs, &objp->prog)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->vers)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->proc)) {
		return (FALSE);
	}
	/*
	 * All the jugglery for just getting the size of the arguments
	 */
	lenposition = XDR_GETPOS(xdrs);
	if (! xdr_u_long(xdrs, &(objp->arglen)))
	    return (FALSE);
	argposition = XDR_GETPOS(xdrs);
	if (! (*(objp->xdr_args))(xdrs, objp->args_ptr))
	    return (FALSE);
	position = XDR_GETPOS(xdrs);
	objp->arglen = (u_long)position - (u_long)argposition;
	XDR_SETPOS(xdrs, lenposition);
	if (! xdr_u_long(xdrs, &(objp->arglen)))
	    return (FALSE);
	XDR_SETPOS(xdrs, position);
	return (TRUE);
}

/*
 * XDR remote call results
 * written for XDR_DECODE direction only
 */
bool_t
xdr_rpcb_rmtcallres(xdrs, objp)
	XDR *xdrs;
	struct rpcb_rmtcallres *objp;
{
	if (!xdr_string(xdrs, &objp->addr_ptr, ~0)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->resultslen)) {
		return (FALSE);
	}
	return ((*(objp->xdr_results))(xdrs, objp->results_ptr));
}

bool_t
xdr_netbuf(xdrs, objp)
	XDR *xdrs;
	struct netbuf *objp;
{
	if (!xdr_u_long(xdrs, &objp->maxlen)) {
		return (FALSE);
	}
	return (xdr_bytes(xdrs, (char **)&(objp->buf),
			(u_int *)&(objp->len), objp->maxlen));
}
