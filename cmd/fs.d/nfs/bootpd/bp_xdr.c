/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/bootpd/bp_xdr.c	1.2.2.1"

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
/*
 * Copyright (c) 1986 by Sun Microsystems, Inc.
 */

#ifdef KERNEL
#include <rpc/rpc.h>
#include <rpcsvc/bootparam.h>
#else
#include <rpc/rpc.h>
#include "bootparam.h"
#endif


bool_t
xdr_bp_machine_name_t(xdrs,objp)
	XDR *xdrs;
	bp_machine_name_t *objp;
{
	if (! xdr_string(xdrs, objp, MAX_MACHINE_NAME)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_path_t(xdrs,objp)
	XDR *xdrs;
	bp_path_t *objp;
{
	if (! xdr_string(xdrs, objp, MAX_PATH_LEN)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_fileid_t(xdrs,objp)
	XDR *xdrs;
	bp_fileid_t *objp;
{
	if (! xdr_string(xdrs, objp, MAX_FILEID)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_ip_addr_t(xdrs,objp)
	XDR *xdrs;
	ip_addr_t *objp;
{
	if (! xdr_char(xdrs, &objp->net)) {
		return(FALSE);
	}
	if (! xdr_char(xdrs, &objp->host)) {
		return(FALSE);
	}
	if (! xdr_char(xdrs, &objp->lh)) {
		return(FALSE);
	}
	if (! xdr_char(xdrs, &objp->impno)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_address(xdrs,objp)
	XDR *xdrs;
	bp_address *objp;
{
	static struct xdr_discrim choices[] = {
		{ (int) IP_ADDR_TYPE, xdr_ip_addr_t },
		{ __dontcare__, NULL }
	};

	if (! xdr_union(xdrs, (enum_t *) &objp->address_type, (char *) &objp->bp_address, choices, (xdrproc_t) NULL)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_whoami_arg(xdrs,objp)
	XDR *xdrs;
	bp_whoami_arg *objp;
{
	if (! xdr_bp_address(xdrs, &objp->client_address)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_whoami_res(xdrs,objp)
	XDR *xdrs;
	bp_whoami_res *objp;
{
	if (! xdr_bp_machine_name_t(xdrs, &objp->client_name)) {
		return(FALSE);
	}
	if (! xdr_bp_machine_name_t(xdrs, &objp->domain_name)) {
		return(FALSE);
	}
	if (! xdr_bp_address(xdrs, &objp->router_address)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_getfile_arg(xdrs,objp)
	XDR *xdrs;
	bp_getfile_arg *objp;
{
	if (! xdr_bp_machine_name_t(xdrs, &objp->client_name)) {
		return(FALSE);
	}
	if (! xdr_bp_fileid_t(xdrs, &objp->file_id)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_getfile_res(xdrs,objp)
	XDR *xdrs;
	bp_getfile_res *objp;
{
	if (! xdr_bp_machine_name_t(xdrs, &objp->server_name)) {
		return(FALSE);
	}
	if (! xdr_bp_address(xdrs, &objp->server_address)) {
		return(FALSE);
	}
	if (! xdr_bp_path_t(xdrs, &objp->server_path)) {
		return(FALSE);
	}
	return(TRUE);
}


