/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:rpc.h	1.3"

/*	@(#)rpc.h 1.13 88/12/17 SMI	*/

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
 * rpc.h, Just includes the billions of rpc header files necessary to 
 * do remote procedure calling.
 *
 */
#ifndef _RPC_RPC_H
#define _RPC_RPC_H

#include <rpc/types.h>		/* some typedefs */

#ifndef _KERNEL
#include <tiuser.h>
#include <fcntl.h>
#include <memory.h>
#else
#include <sys/tiuser.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <sys/t_kuser.h>
#endif

#include <rpc/xdr.h>		/* generic (de)serializer */
#include <rpc/auth.h>		/* generic authenticator (client side) */
#include <rpc/clnt.h>		/* generic client side rpc */

#include <rpc/rpc_msg.h>	/* protocol for rpc messages */
#include <rpc/auth_sys.h>	/* protocol for unix style cred */
#include <rpc/auth_des.h>	/* protocol for des style cred */

#include <rpc/svc.h>		/* service manager and multiplexer */
#include <rpc/svc_auth.h>	/* service side authenticator */

#ifndef _KERNEL
#include <rpc/rpcb_clnt.h>	/* rpcbind interface functions */
#endif
#endif /* ! _RPC_RPC_H */
