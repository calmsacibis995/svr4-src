/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)rpcsvc:rwall_subr.c	1.3.1.1"

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
#ifndef lint
static	char sccsid[] = "@(#)rwall_subr.c 1.1 89/03/09 Copyr 1984 Sun Micro";
#endif

/*
 * rwall_subr.c
 *	The server procedure for rwalld
 * 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <stdio.h>
#include <rpcsvc/rwall.h>

static char *oldmsg;
extern char *strdup();

void *
wallproc_wall_1(argp, clnt)
	wrapstring *argp;
	CLIENT *clnt;
{
	static char res;
	char *msg;
	FILE *fp;

	msg = *argp;
	if ((oldmsg == (char *) 0) || (strcmp (msg, oldmsg))) {
		fp = popen("/etc/wall", "w");
		fprintf(fp, "%s", msg);
		(void) pclose(fp);
		if (oldmsg)
			free(oldmsg);
		oldmsg = strdup(msg);
	}
	return ((void *)&res);
}
