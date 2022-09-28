/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rpcbind:rpcbind.h	1.2.3.1"

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
/*
 * rpcbind.h
 * The common header declarations
 */

extern int debugging;
extern RPCBLIST *list_rbl;	/* A list of version 3 rpcbind services */
extern char *loopback_dg;	/* CLTS loopback transport, for set/unset */
extern char *loopback_vc;	/* COTS loopback transport, for set/unset */
extern char *loopback_vc_ord;	/* COTS_ORD loopback transport, for set/unset */

#ifdef PORTMAP
extern PMAPLIST *list_pml;	/* A list of version 2 rpcbind services */
extern char *udptrans;		/* Name of UDP transport */
extern char *tcptrans;		/* Name of TCP transport */
extern char *udp_uaddr;		/* Universal UDP address */
extern char *tcp_uaddr;		/* Universal TCP address */
#endif

extern char *mergeaddr();
extern int add_bndlist();
extern bool_t is_bound();
