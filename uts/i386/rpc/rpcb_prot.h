/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:rpcb_prot.h	1.3"

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
*	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
/*
 * rpcb_prot.h
 * Protocol for the local rpcbinder service
 */

/*
 * The following procedures are supported by the protocol:
 *
 * RPCBPROC_NULL() returns ()
 * 	takes nothing, returns nothing
 *
 * RPCBPROC_SET(RPCB) returns (bool_t)
 * 	TRUE is success, FALSE is failure.  Registers the tuple
 *	[prog, vers, netid] with address
 *
 * RPCBPROC_UNSET(RPCB) returns (bool_t)
 *	TRUE is success, FALSE is failure.  Un-registers tuple
 *	[prog, vers, netid].  address is ignored.
 *
 * RPCBPROC_GETADDR(RPCB) returns (Universal address).
 *	0 is failure.  Otherwise returns the universal address where the pair
 *	[prog, vers, netid] is registered.
 *
 * RPCBPROC_DUMP() RETURNS (RPCBLIST *)
 *	used for dumping the entire rpcbind maps
 *
 * RPCBPROC_CALLIT(unsigned, unsigned, unsigned, string<>)
 * 	RETURNS (address, string<>);
 * usage: encapsulatedresults = RPCBPROC_CALLIT(prog, vers, proc, encapsulatedargs);
 * 	Calls the procedure on the local machine.  If it is not registered,
 *	this procedure is quiet; i.e. it does not return error information!!!
 *	This routine only passes null authentication parameters.
 *	This file has no interface to xdr routines for RPCBPROC_CALLIT.
 *
 * RPCBPROC_GETTIME() returns (bool_t).
 *	TRUE is success, FALSE is failure.  Gets the remote machines time
 *
 */

#ifndef _RPC_RPCB_PROT_H
#define _RPC_RPCB_PROT_H

#include <rpc/types.h>

#define RPCBPROG		((u_long)100000)
#define RPCBVERS		((u_long)3)

/*
 * All the defined procedures on it
 */
#define RPCBPROC_NULL		((u_long)0)
#define RPCBPROC_SET		((u_long)1)
#define RPCBPROC_UNSET		((u_long)2)
#define RPCBPROC_GETADDR	((u_long)3)
#define RPCBPROC_DUMP		((u_long)4)
#define RPCBPROC_CALLIT		((u_long)5)
#define RPCBPROC_GETTIME	((u_long)6)
#define RPCBPROC_UADDR2TADDR	((u_long)7)
#define RPCBPROC_TADDR2UADDR	((u_long)8)

/*
 * All rpcbind stuff (vers 3)
 */

/*
 * A mapping of (program, version, network ID) to address
 */
struct rpcb {
	u_long r_prog;			/* program number */
	u_long r_vers;			/* version number */
	char *r_netid;			/* network id */
	char *r_addr;			/* universal address */
	char *r_owner;			/* owner of the mapping */
};
typedef struct rpcb RPCB;
extern bool_t xdr_rpcb();

extern bool_t xdr_netbuf();

/*
 * A list of mappings
 */
struct rpcblist {
	RPCB rpcb_map;
	struct rpcblist *rpcb_next;
};
typedef struct rpcblist RPCBLIST;
extern bool_t xdr_rpcblist();

/*
 * Remote calls arguments
 */
struct rpcb_rmtcallargs {
	u_long prog;			/* program number */
	u_long vers;			/* version number */
	u_long proc;			/* procedure number */
	u_long arglen;			/* arg len */
	caddr_t args_ptr;		/* argument */
	xdrproc_t xdr_args;		/* XDR routine for argument */
};
extern bool_t xdr_rpcb_rmtcallargs();

/*
 * Remote calls results
 */
struct rpcb_rmtcallres {
	char *addr_ptr;			/* remote universal address */
	u_long resultslen;		/* results length */
	caddr_t results_ptr;		/* results */
	xdrproc_t xdr_results;		/* XDR routine for result */
};
extern bool_t xdr_rpcb_rmtcallres();

#endif /*!_RPC_RPCB_PROT_H*/
