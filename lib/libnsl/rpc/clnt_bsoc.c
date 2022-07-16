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


#ident	"@(#)librpc:clnt_bsoc.c	1.2.2.1"

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


/* TEMPORARILY STUBBED OUT UNTIL BROADCAST ROUTINES ARRIVE */
/* OLD ROUTINES HAD INTERNET DEPENDANCIES	*/

/*
 * clnt_bsoc.c
 * Help interface to broadcast service.
 *
 * the whole stuff should go away with dynamic transport specific libraries
 */

#include <rpc/rpc.h>
#include <netconfig.h>

/*
 * Finds a list of broadcast addresses for the given transport tokenid
 * returns the number of such addresses found. Uses rpcbind and the
 * netdir daemon
 *
 * Returns the number of broadcast addresses found
 */
int
getbroadcastnets(fd, addrs, nconf)
	int fd;		/* Not being used currently */
	struct netbuf* addrs;
	struct netconfig *nconf;
{
	return (1);
}

/*
 * Do all the options managements stuff here. This is very protocol specific
 * and should be provided by the vendor. The following case is just for
 * socket/IP.
 * Returns 0 if succeeds else returns 1.
 */
int
negotiate_broadcast(fd, nconf)
	int fd;
	struct netconfig *nconf;
{
	return (0);
}
