/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:nettype.h	1.3"

/*      @(#)nettype.h 1.4 88/12/14 SMI      */

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
 * nettype.h, Nettype definitions.
 * All for the topmost layer of rpc
 *
 */

#ifndef _RPC_NETTYPE_H
#define _RPC_NETTYPE_H

#include <netconfig.h>

#define _RPC_NONE	0
#define _RPC_NETPATH	1
#define _RPC_VISIBLE	2
#define _RPC_CIRCUIT_V	3
#define _RPC_DATAGRAM_V	4
#define _RPC_CIRCUIT_N	5
#define _RPC_DATAGRAM_N	6
#define _RPC_TCP	7
#define _RPC_UDP	8

extern int _rpc_setconf();
extern int _rpc_endconf();
extern struct netconfig *_rpc_getconf();
extern struct netconfig *_rpc_getconfip();
#endif /* !_RPC_NETTYPE_H */
