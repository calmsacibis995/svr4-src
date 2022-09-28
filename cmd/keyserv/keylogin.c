/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)keyserv:keylogin.c	1.1.2.1"

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
static char sccsid[] = "@(#)keylogin.c 1.2 89/03/20 Copyr 1986 Sun Micro";
#endif

/*
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

/*
 * Set secret key on local machine
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>

main()
{
	char fullname[MAXNETNAMELEN + 1];
	char secret[HEXKEYBYTES + 1];
	char *getpass();

	getnetname(fullname);
	if (! getsecretkey(fullname, secret, getpass("Password:"))) {
		fprintf(stderr, "Can't find %s's secret key\n", fullname);
		exit(1);
	}
	if (secret[0] == 0) {
		fprintf(stderr, "Password incorrect for %s\n", fullname);
		exit(1);
	}
	if (key_setsecret(secret) < 0) {
		fprintf(stderr, "Could not set %s's secret key\n", fullname);
		fprintf(stderr, "Maybe the keyserver is down?\n");
		exit(1);
	}
	exit(0);
	/* NOTREACHED */
}
