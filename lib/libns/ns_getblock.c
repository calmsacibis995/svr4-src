/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libns:ns_getblock.c	1.3.5.1"

#include <stdio.h>
#include <tiuser.h>
#include <nsaddr.h>
#include "nserve.h"

struct nssend *
ns_getblock(send)
struct nssend *send;
{
	struct nssend *rcv;

	/*
	 *	Setup the communication path to the name server.
	 */
	
	if (ns_setup() == FAILURE)
		return((struct nssend *)NULL);
	
	if (ns_send(send) == FAILURE) {
		ns_close();
		return((struct nssend *)NULL);
	}

	/*
	 *	Get a return structure and check the return code
	 *	from the name server.
	 */
	
	if ((rcv = ns_rcv()) == (struct nssend *)NULL) {
		ns_close();
		return((struct nssend *)NULL);
	}

	if ((rcv->ns_code&(~MORE_DATA)) != SUCCESS) {
		ns_close();
		return((struct nssend *)NULL);
	}

	ns_close();

	return(rcv);
}

struct nssend *
ns_getblocks(send)
struct nssend *send;
{
	struct nssend *rcv;

	/*
	 *	Setup the communication path to the name server.
	 */
	
	if (send)
	{
		if (ns_setup() == FAILURE)
			return((struct nssend *)NULL);
	
		if (ns_send(send) == FAILURE) {
			ns_close();
			return((struct nssend *)NULL);
		}
	}

	/*
	 *	Get a return structure and check the return code
	 *	from the name server.
	 */
	
	if ((rcv = ns_rcv()) == (struct nssend *)NULL) {
		ns_close();
		return((struct nssend *)NULL);
	}

	if ((rcv->ns_code&(~MORE_DATA)) != SUCCESS) {
		ns_close();
		return((struct nssend *)NULL);
	}

	if (!(rcv->ns_code & MORE_DATA))
		ns_close();

	return(rcv);
}

