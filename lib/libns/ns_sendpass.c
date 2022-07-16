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


#ident	"@(#)libns:ns_sendpass.c	1.4.6.1"

#include <stdio.h>
#include <tiuser.h>
#include <nsaddr.h>
#include "nserve.h"

ns_sendpass(name, oldpass, newpass)
char	*name, *oldpass, *newpass;
{
	struct nssend send;
	struct nssend *ns_getblock();
	char *malloc();

	/*
	 *	Initialize the information structure to send to the
	 *	name server.
	 *	oldpass is not encrypted, newpass is.
	 */

	send.ns_code = NS_SENDPASS;
	send.ns_name = name;
	send.ns_type = 0;
	send.ns_flag = 0;
	send.ns_path = NULL;
	send.ns_addr = NULL;
	send.ns_mach = NULL;
	send.ns_desc = malloc(strlen(oldpass)+strlen(newpass)+3);
	strcpy(send.ns_desc, oldpass);
	strcat(send.ns_desc, ":");
	strcat(send.ns_desc, newpass);

	if (ns_getblock(&send) == (struct nssend *)NULL)
		return(FAILURE);

	return(SUCCESS);
}
