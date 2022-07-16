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


#ident	"@(#)libns:ns_info.c	1.9.6.1"

#include  <stdio.h>
#include  <nserve.h>
#include  <tiuser.h>
#include  <nsaddr.h>
#include  "stdns.h"
#include  "nsdb.h"

extern int ns_errno;

ns_info(name)
char	*name;
{

	static	char	*flg[] = {
		"rw",
		"ro"
	};
	char	dname[SZ_DELEMENT];
	struct	nssend 	send, *rtn;


	if (name[strlen(name)-1] == SEPARATOR) {
		sprintf(dname,"%s%c",name,WILDCARD);
		name = dname;
		send.ns_code = NS_QUERY;
	}
	else if (*name == WILDCARD)
		send.ns_code = NS_QUERY;
	else
		send.ns_code = NS_BYMACHINE;

	send.ns_type = 0;
	send.ns_flag = 0;
	send.ns_name = name;
	send.ns_path = NULL;
	send.ns_desc = NULL;
	send.ns_mach = NULL;

	/*
	 *	Setup communication path to the name server.
	 */
	
	if (ns_setup() == FAILURE)
		return(FAILURE);
	
	if (ns_send(&send) == FAILURE) {
		ns_close();
		return(FAILURE);
	}

	if ((rtn = ns_rcv()) == NULL) {
		ns_close();
		return(FAILURE);
	}
	
	do {
		if (rtn->ns_code == FAILURE) {
			if (ns_errno == R_NONAME)
				break;
			ns_close();
			return(FAILURE);
		}

		fprintf(stdout,"%-14.14s  %-6.2s  %-8.8s  %-9.9s  %s\n",
				rtn->ns_name,flg[rtn->ns_flag],
				namepart(*rtn->ns_mach), rtn->ns_addr->protocol,
				(rtn->ns_desc) ? rtn->ns_desc : " "); 

		if (!(rtn->ns_code & MORE_DATA))
			break;

	} while ((rtn = ns_rcv()) != NULL);

	ns_close();
	return(SUCCESS);
}
