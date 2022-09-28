/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libyp:ypv2_bind.h	1.2.3.1"

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
 * This file contains symbols and structures defining diffences
 * between Version 3 and Version 2 of the yp bind protocol.
 */

#include "netinet/in.h"

#define YPBINDOLDVERS	((u_long)2)
#define YPOLDMAXDOMAIN	((u_long)64)

struct domv2_binding {
	struct domv2_binding *dom_pnext;
	char dom_domain[YPMAXDOMAIN + 1];
	struct sockaddr_in dom_server_addr;
	unsigned short int dom_server_port;
	int dom_socket;
	CLIENT *dom_client;
	unsigned short int dom_local_port;
	long int dom_vers;
};


/*
 *		Protocol between clients and yp binder servers
 */

/*
 * Response structure and binding info
 */

struct ypbindv2_binding {
	struct in_addr ypbind_binding_addr;	/* In network order */
	unsigned short int ypbind_binding_port;	/* In network order */
};
struct ypbindv2_resp {
	enum ypbind_resptype ypbind_status;
	union {
		unsigned long ypbind_error;
		struct ypbindv2_binding ypbind_bindinfo;
	} ypbind_respbody;
};

/*
 * Request data structure for ypbind "Set domain" procedure.
 */
struct ypbindv2_setdom {
	char ypsetdom_domain[YPMAXDOMAIN + 1];
	struct ypbindv2_binding ypsetdom_binding;
	unsigned short ypsetdom_vers;
};
#define ypsetdom_addr ypsetdom_binding.ypbind_binding_addr
#define ypsetdom_port ypsetdom_binding.ypbind_binding_port
