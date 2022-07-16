/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _RPC_PMAPPROT_H
#define _RPC_PMAPPROT_H

#ident	"@(#)kern-rpc:pmap_prot.h	1.3"

/*	@(#)pmap_prot.h 1.19 88/11/28 SMI	*/

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 * pmap_prot.h
 * Protocol for the local binder service, or pmap.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * The following procedures are supported by the protocol:
 *
 * PMAPPROC_NULL() returns ()
 * 	takes nothing, returns nothing
 *
 * PMAPPROC_SET(struct pmap) returns (bool_t)
 * 	TRUE is success, FALSE is failure.  Registers the tuple
 *	[prog, vers, prot, port].
 *
 * PMAPPROC_UNSET(struct pmap) returns (bool_t)
 *	TRUE is success, FALSE is failure.  Un-registers pair
 *	[prog, vers].  prot and port are ignored.
 *
 * PMAPPROC_GETPORT(struct pmap) returns (long unsigned).
 *	0 is failure.  Otherwise returns the port number where the pair
 *	[prog, vers] is registered.  It may lie!
 *
 * PMAPPROC_DUMP() RETURNS (struct pmaplist *)
 *
 * PMAPPROC_CALLIT(unsigned, unsigned, unsigned, string<>)
 * 	RETURNS (port, string<>);
 * usage: encapsulatedresults = PMAPPROC_CALLIT(prog, vers, proc, encapsulatedargs);
 * 	Calls the procedure on the local machine.  If it is not registered,
 *	this procedure is quite; ie it does not return error information!!!
 *	This procedure only is supported on rpc/udp and calls via
 *	rpc/udp.  This routine only passes null authentication parameters.
 *	This file has no interface to xdr routines for PMAPPROC_CALLIT.
 *
 * The service supports remote procedure calls on udp/ip or tcp/ip socket 111.
 */

#include <rpc/types.h>

#define PMAPPORT		((u_short)111)
#define PMAPPROG		((u_long)100000)
#define PMAPVERS		((u_long)2)
#define PMAPVERS_PROTO		((u_long)2)
#define PMAPVERS_ORIG		((u_long)1)

#define PMAPPROC_NULL		((u_long)0)
#define PMAPPROC_SET		((u_long)1)
#define PMAPPROC_UNSET		((u_long)2)
#define PMAPPROC_GETPORT	((u_long)3)
#define PMAPPROC_DUMP		((u_long)4)
#define PMAPPROC_CALLIT		((u_long)5)

struct pmap {
	long unsigned pm_prog;
	long unsigned pm_vers;
	long unsigned pm_prot;
	long unsigned pm_port;
};
typedef struct pmap PMAP;
extern bool_t xdr_pmap();

struct pmaplist {
	struct pmap pml_map;
	struct pmaplist *pml_next;
};
typedef struct pmaplist PMAPLIST;
#ifndef _KERNEL
extern bool_t xdr_pmaplist();
#endif

#endif	/* _RPC_PMAPPROT_H */
