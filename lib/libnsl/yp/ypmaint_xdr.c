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

#ident	"@(#)libyp:ypmaint_xdr.c	1.3.1.1"


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
static  char sccsid[] = "@(#)ypmaint_xdr.c 1.4 88/02/08 Copyr 1985 Sun Micro";
#endif

/*
 * This contains xdr routines used by the YP rpc interface
 * for systems and maintenance programs only.  This is a separate module
 * because most yp clients should not need to link to it.
 */

#define NULL 0
#include <rpc/rpc.h>
#include "yp_b.h"
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

static bool xdr_ypmaplist();
static bool xdr_ypmaplist_wrap_string();

extern char *calloc();

/*
 * Serializes/deserializes a ypresp_master structure.
 */
bool
xdr_ypresp_master(xdrs, ps)
	XDR * xdrs;
	struct ypresp_master *ps;
{
	return (xdr_u_long(xdrs, &ps->status) &&
	     xdr_ypowner_wrap_string(xdrs, &ps->master) );
}

/*
 * Serializes/deserializes a ypresp_order structure.
 */
bool
xdr_ypresp_order(xdrs, ps)
	XDR * xdrs;
	struct ypresp_order *ps;
{
	return (xdr_u_long(xdrs, &ps->status) &&
	     xdr_u_long(xdrs, &ps->ordernum) );
}

/*
 * This is like xdr_ypmap_wrap_string except that it serializes/deserializes
 * an array, instead of a pointer, so xdr_reference can work on the structure
 * containing the char array itself.
 */
static bool
xdr_ypmaplist_wrap_string(xdrs, pstring)
	XDR * xdrs;
	char *pstring;
{
	char *s;

	s = pstring;
	return (xdr_string(xdrs, &s, YPMAXMAP) );
}

/*
 * Serializes/deserializes a ypmaplist.
 */
static bool
xdr_ypmaplist(xdrs, lst)
	XDR *xdrs;
	struct ypmaplist **lst;
{
	bool more_elements;
	int freeing = (xdrs->x_op == XDR_FREE);
	struct ypmaplist **next;

	for (;;) {
		more_elements = (*lst != (struct ypmaplist *) NULL);
		
		if (! xdr_bool(xdrs, &more_elements))
			return (FALSE);
			
		if (! more_elements)
			return (TRUE);  /* All done */
			
		if (freeing)
			next = &((*lst)->ypml_next);

		if (! xdr_reference(xdrs, lst, (u_int) sizeof(struct ypmaplist),
		    xdr_ypmaplist_wrap_string))
			return (FALSE);
			
		lst = (freeing) ? next : &((*lst)->ypml_next);
	}
	/*NOTREACHED*/
}

/*
 * Serializes/deserializes a ypresp_maplist.
 */
bool
xdr_ypresp_maplist(xdrs, ps)
	XDR * xdrs;
	struct ypresp_maplist *ps;

{
	return (xdr_u_long(xdrs, &ps->status) &&
	   xdr_ypmaplist(xdrs, &ps->list) );
}

/*
 * Serializes/deserializes a yppushresp_xfr structure.
 */
bool
xdr_yppushresp_xfr(xdrs, ps)
	XDR *xdrs;
	struct yppushresp_xfr *ps;
{
	return (xdr_u_long(xdrs, &ps->transid) &&
	    xdr_u_long(xdrs, &ps->status));
}


/*
 * Serializes/deserializes a ypreq_xfr structure.
 */
bool
xdr_ypreq_newxfr(xdrs, ps)
	XDR * xdrs;
	struct ypreq_newxfr *ps;
{
	return (xdr_ypmap_parms(xdrs, &ps->map_parms) &&
	    xdr_u_long(xdrs, &ps->transid) &&
	    xdr_u_long(xdrs, &ps->proto) &&
	    xdr_string(xdrs, &ps->name, 256) );
}

/*
 * Serializes/deserializes a ypreq_xfr structure.
 */
bool
xdr_ypreq_xfr(xdrs, ps)
	XDR * xdrs;
	struct ypreq_xfr *ps;
{
	return (xdr_ypmap_parms(xdrs, &ps->map_parms) &&
	    xdr_u_long(xdrs, &ps->transid) &&
	    xdr_u_long(xdrs, &ps->proto) &&
	    xdr_u_short(xdrs, &ps->port) );
}


/*
 * Serializes/deserializes a stream of struct ypresp_key_val's.  This is used
 * only by the client side of the batch enumerate operation.
 */
bool
xdr_ypall(xdrs, callback)
	XDR * xdrs;
	struct ypall_callback *callback;
{
	bool more;
	struct ypresp_key_val kv;
	bool s;
	char keybuf[YPMAXRECORD];
	char valbuf[YPMAXRECORD];

	if (xdrs->x_op == XDR_ENCODE)
		return(FALSE);

	if (xdrs->x_op == XDR_FREE)
		return(TRUE);

	kv.keydat.dptr = keybuf;
	kv.valdat.dptr = valbuf;
	kv.keydat.dsize = YPMAXRECORD;
	kv.valdat.dsize = YPMAXRECORD;
	
	for (;;) {
		if (! xdr_bool(xdrs, &more) )
			return (FALSE);
			
		if (! more)
			return (TRUE);

		s = xdr_ypresp_key_val(xdrs, &kv);
		
		if (s) {
			s = (*callback->foreach)(kv.status, kv.keydat.dptr,
			    kv.keydat.dsize, kv.valdat.dptr, kv.valdat.dsize,
			    callback->data);
			
			if (s)
				return (TRUE);
		} else {
			return (FALSE);
		}
	}
}

bool_t
xdr_netconfig(xdrs, objp)
	XDR *xdrs;
	struct netconfig *objp;
{
	char **tmp;
	int i;

	if (!xdr_string(xdrs, &objp->nc_netid, ~0)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->nc_semantics)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->nc_flag)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->nc_protofmly, ~0)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->nc_proto, ~0)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->nc_device, ~0)) {
		return (FALSE);
	}
	if (!xdr_array(xdrs, &objp->nc_lookups, &objp->nc_nlookups, 100, sizeof (char *), xdr_wrapstring)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->nc_unused, 8, sizeof(u_long), xdr_u_long)) {
		return (FALSE);
	}
	return (TRUE);
}
