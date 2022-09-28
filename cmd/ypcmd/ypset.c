/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:ypset.c	1.3.2.1"

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
static	char sccsid[] = "@(#)ypset.c 1.6 88/02/08 Copyr 1985 Sun Micro";
#endif

/*
 * This is a user command which issues a "Set domain binding" command to a 
 * YP binder (ypbind) process
 *
 *	ypset [-h <host>] [-d <domainname>] server_to_use
 *
 * where host and server_to_use may be either names or internet addresses.
 */
#include <stdio.h>
#include <ctype.h>
#include <rpc/rpc.h>
#include <rpcsvc/ypclnt.h>
#include "yp_b.h"
#include <rpcsvc/yp_prot.h>

#ifdef NULL
#undef NULL
#endif
#define NULL 0

#define TIMEOUT 30			/* Total seconds for timeout */

static char *pusage;
static char *domain = NULL;
static char default_domain_name[YPMAXDOMAIN];
static char default_host_name[256];
static char *host = NULL;
static char *server_to_use;
static struct timeval timeout = {
	TIMEOUT,			/* Seconds */
	0				/* Microseconds */
	};

static char err_usage_set[] =
"Usage:\n\
	ypset [-h <host>] [-d <domainname>] server_to_use\n\n";
static char err_bad_args[] =
	"Sorry, the %s argument is bad.\n";
static char err_cant_get_kname[] =
	"Sorry, can't get %s back from system call.\n";
static char err_null_kname[] =
	"Sorry, the %s hasn't been set on this machine.\n";
static char err_bad_hostname[] = "hostname";
static char err_bad_domainname[] = "domainname";
static char err_bad_server[] = "server_to_use";
static char err_tp_failure[] =
	"Sorry, I can't set up a connection to host %s.\n";
static char err_rpc_failure[] =
	"Sorry, I couldn't send my rpc message to ypbind on host %s.\n";

static void get_command_line_args();
static void send_message();

extern void exit();
extern int getdomainname();
extern int gethostname();
extern struct netconfig *getnetconfigent();
extern unsigned int strlen();

/*
 * This is the mainline for the ypset process.  It pulls whatever arguments
 * have been passed from the command line, and uses defaults for the rest.
 */

void
main (argc, argv)
	int argc;
	char **argv;
	
{
	get_command_line_args(argc, argv);

	if (!domain) {
		
		if (!getdomainname(default_domain_name, YPMAXDOMAIN) ) {
			domain = default_domain_name;
		} else {
			(void) fprintf(stderr, err_cant_get_kname, err_bad_domainname);
			exit(1);
		}

		if ((int) strlen(domain) == 0) {
			(void) fprintf(stderr, err_null_kname, err_bad_domainname);
			exit(1);
		}
	}

	if (!host) {
		
		if (! gethostname(default_host_name, 256)) {
			host = default_host_name;
		} else {
			(void) fprintf(stderr, err_cant_get_kname, err_bad_hostname);
			exit(1);
		}
	}

	send_message();
	exit(0);
}

/*
 * This does the command line argument processing.
 */
static void
get_command_line_args(argc, argv)
	int argc;
	char **argv;
	
{
	pusage = err_usage_set;
	argv++;
	
	while (--argc > 1) {

		if ( (*argv)[0] == '-') {

			switch ((*argv)[1]) {

			case 'h': {

				if (argc > 1) {
					argv++;
					argc--;
					host = *argv;
					argv++;

					if ((int) strlen(host) > 256) {
						(void) fprintf(stderr, err_bad_args,
						    err_bad_hostname);
						exit(1);
					}
					
				} else {
					(void) fprintf(stderr, pusage);
					exit(1);
				}
				
				break;
			}
				
			case 'd': {

				if (argc > 1) {
					argv++;
					argc--;
					domain = *argv;
					argv++;

					if ((int) strlen(domain) > YPMAXDOMAIN) {
						(void) fprintf(stderr, err_bad_args,
						    err_bad_domainname);
						exit(1);
					}
					
				} else {
					(void) fprintf(stderr, pusage);
					exit(1);
				}
				
				break;
			}
				
			default: {
				(void) fprintf(stderr, pusage);
				exit(1);
			}
			
			}
			
		} else {
			(void) fprintf(stderr, pusage);
			exit(1);
		}
	}

	if (argc == 1) {
		
		if ( (*argv)[0] == '-') {
			(void) fprintf(stderr, pusage);
			exit(1);
		}
		
		server_to_use = *argv;

		if ((int) strlen(server_to_use) > 256) {
			(void) fprintf(stderr, err_bad_args,
			    err_bad_server);
			exit(1);
		}

	} else {
		(void) fprintf(stderr, pusage);
		exit(1);
	}
}

/*
 * This takes the name of the YP host of interest, and fires off 
 * the "set domain binding" message to the ypbind process.
 */
 
static void
send_message()
{
	CLIENT *server, *client;
	struct ypbind_setdom req;
	struct ypbind_binding ypbind_info;
	enum clnt_stat clnt_stat;
	struct netconfig *nconf;
	struct netbuf nbuf;

	/*
	 * Open up a path to the server
	 */

	if ((server = clnt_create(server_to_use, YPPROG, YPVERS, 
	    "datagram_n")) == NULL) {
		(void) fprintf(stderr, err_tp_failure, server_to_use);
		exit(1);
	}

	/* get nconf, netbuf structures */
	nconf = getnetconfigent(server->cl_netid);
	clnt_control(server, CLGET_SVC_ADDR, &nbuf);

	/*
	 * Open a path to host
	 */
        if ((client = clnt_create(host, YPBINDPROG, YPBINDVERS, 
	   "netpath")) == NULL) {
                clnt_pcreateerror("ypset: clnt_create");
                exit(1);
        }

	client->cl_auth = authunix_create_default();

	/*
	 * Load up the message structure and fire it off.
	 */
	ypbind_info.ypbind_nconf = nconf;
	ypbind_info.ypbind_svcaddr = (struct netbuf *)(&nbuf);
	ypbind_info.ypbind_servername = server_to_use;
	ypbind_info.ypbind_hi_vers = 0;
	ypbind_info.ypbind_lo_vers = 0;
	req.ypsetdom_bindinfo = &ypbind_info;
	req.ypsetdom_domain =  domain;

	clnt_stat = (enum clnt_stat) clnt_call(client,
	    YPBINDPROC_SETDOM, xdr_ypbind_setdom, &req, xdr_void, 0,
	    timeout);
	auth_destroy((client)->cl_auth);
	if( clnt_stat != RPC_SUCCESS) {
		(void) fprintf(stderr, err_rpc_failure, host);
		exit(1);
	}
}
