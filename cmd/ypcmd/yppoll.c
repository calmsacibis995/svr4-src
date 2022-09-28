/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:yppoll.c	1.4.2.1"

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
static	char sccsid[] = "@(#)yppoll.c 1.7 88/08/07 Copyr 1985 Sun Micro";
#endif

/*
 * This is a user command which asks a particular ypserv which version of a
 * map it is using.  Usage is:
 * 
 * yppoll [-h <host>] [-d <domainname>] mapname
 * 
 * If the host is ommitted, the local host will be used.  If host is specified
 * as an internet address, no yp services need to be locally available.
 *  
 */
#include <stdio.h>
#include <ctype.h>
#include <rpc/rpc.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>
#include "yp_b.h"

#ifdef NULL
#undef NULL
#endif
#define NULL 0

#define TIMEOUT 30			/* Total seconds for timeout */

static int status = 0;				/* exit status */
static char *domain = NULL;
static char default_domain_name[YPMAXDOMAIN];
static char *map = NULL;
static char *host = NULL;
static char default_host_name[256];

static char err_usage[] =
"Usage:\n\
	yppoll [-h <host>] [-d <domainname>] mapname\n\n";
static char err_bad_args[] =
	"Bad %s argument.\n";
static char err_cant_get_kname[] =
	"Can't get %s back from system call.\n";
static char err_null_kname[] =
	"%s hasn't been set on this machine.\n";
static char err_bad_hostname[] = "hostname";
static char err_bad_mapname[] = "mapname";
static char err_bad_domainname[] = "domainname";
static char err_bad_resp[] =
	"Ill-formed response returned from ypserv on host %s.\n";

static void get_command_line_args();
static void getdomain();
static void getlochost();
static void getmapparms();
static void newresults();
static void getypserv();

extern void exit();
extern int getdomainname();
extern int gethostname();
extern unsigned int strlen();
extern int strcmp();

/*
 * This is the mainline for the yppoll process.
 */

void
main (argc, argv)
	int argc;
	char **argv;
	
{
	get_command_line_args(argc, argv);

	if (!domain) {
		getdomain();
	}
	
	if (!host) {
		getypserv();
	}
	
	getmapparms();
	exit(status);
	/* NOTREACHED */
}

/*
 * This does the command line argument processing.
 */
static void
get_command_line_args(argc, argv)
	int argc;
	char **argv;
	
{
	argv++;
	
	while (--argc) {

		if ( (*argv)[0] == '-') {

			switch ((*argv)[1]) {

			case 'h': 

				if (argc > 1) {
					argv++;
					argc--;
					host = *argv;
					argv++;

					if ((int)strlen(host) > 256) {
						(void) fprintf(stderr,
						    err_bad_args,
						    err_bad_hostname);
						exit(1);
					}
					
				} else {
					(void) fprintf(stderr, err_usage);
					exit(1);
				}
				
				break;
				
			case 'd': 

				if (argc > 1) {
					argv++;
					argc--;
					domain = *argv;
					argv++;

					if ((int)strlen(domain) > YPMAXDOMAIN) {
						(void) fprintf(stderr,
						    err_bad_args,
						    err_bad_domainname);
						exit(1);
					}
					
				} else {
					(void) fprintf(stderr, err_usage);
					exit(1);
				}
				
				break;
				
			default: 
				(void) fprintf(stderr, err_usage);
				exit(1);
			
			}
			
		} else {
			if (!map) {
				map = *argv;

				if ((int)strlen(map) > YPMAXMAP) {
					(void) fprintf(stderr, err_bad_args,
					    err_bad_mapname);
					exit(1);
				}

			} else {
				(void) fprintf(stderr, err_usage);
				exit(1);
			}
		}
	}

	if (!map) {
		(void) fprintf(stderr, err_usage);
		exit(1);
	}
}

/*
 * This gets the local default domainname, and makes sure that it's set
 * to something reasonable.  domain is set here.
 */
static void
getdomain()		
{
	if (!getdomainname(default_domain_name, YPMAXDOMAIN) ) {
		domain = default_domain_name;
	} else {
		(void) fprintf(stderr, err_cant_get_kname, err_bad_domainname);
		exit(1);
	}

	if ((int)strlen(domain) == 0) {
		(void) fprintf(stderr, err_null_kname, err_bad_domainname);
		exit(1);
	}
}

/*
 * This gets the local hostname back from the kernel
 */
static void
getlochost()
{

	if (! gethostname(default_host_name, 256)) {
		host = default_host_name;
	} else {
		(void) fprintf(stderr, err_cant_get_kname, err_bad_hostname);
		exit(1);
	}
}

static void
getmapparms()
{
        CLIENT * map_clnt;
        struct ypresp_order oresp;
        struct ypreq_nokey req;
        struct ypresp_master mresp;
	struct ypresp_master *mresults = (struct ypresp_master *) NULL;
        struct ypresp_order *oresults = (struct ypresp_order *) NULL;

	struct timeval timeout;
	enum clnt_stat s;

	if ((map_clnt = clnt_create(host, YPPROG, YPVERS, 
	    "netpath"))  == NULL) {
		(void) fprintf(stderr,
		    "Can't create connection to %s.\n", host);
		clnt_pcreateerror("Reason");
		exit(1);
	}

	timeout.tv_sec=TIMEOUT;
        timeout.tv_usec=0;
	req.domain = domain;
	req.map = map;
        mresp.master = NULL;

	if (clnt_call(map_clnt, YPPROC_MASTER, xdr_ypreq_nokey, &req, 
	    xdr_ypresp_master, &mresp, timeout) == RPC_SUCCESS) {
		mresults = &mresp;
		s = (enum clnt_stat) clnt_call(map_clnt, YPPROC_ORDER,
	    	    xdr_ypreq_nokey, &req, xdr_ypresp_order, &oresp,
		    timeout);

		if(s == RPC_SUCCESS) {
			oresults = &oresp;
			newresults(mresults, oresults);
		} else {
			(void) fprintf(stderr,
		"Can't make YPPROC_ORDER call to ypserv at %s.\n	",
			        host);
			clnt_perror(map_clnt, "Reason");
			exit(1);
		}
		
	} else {
		clnt_destroy(map_clnt);
	}
}

static void
newresults(m, o)
	struct ypresp_master *m;
	struct ypresp_order *o;
{
	char *s_domok = "Domain %s is supported.\n";
	char *s_ook = "Map %s has order number %d.\n";
	char *s_mok = "The master server is %s.\n";
	char *s_mbad = "Can't get master for map %s.\n	Reason:  %s\n";
	char *s_obad = "Can't get order number for map %s.\n	Reason:  %s\n";

	if (m->status == YP_TRUE && o->status == YP_TRUE) {
		(void) printf(s_domok, domain);
		(void) printf(s_ook, map, o->ordernum);
		(void) printf(s_mok, m->master);
	} else if (o->status == YP_TRUE)  {
		(void) printf(s_domok, domain);
		(void) printf(s_ook, map, o->ordernum);
		(void) fprintf(stderr, s_mbad, map,
		    yperr_string(ypprot_err(m->status)) );
		status = 1;
	} else if (m->status == YP_TRUE)  {
		(void) printf(s_domok, domain);
		(void) fprintf(stderr, s_obad, map,
		    yperr_string(ypprot_err(o->status)) );
		(void) printf(s_mok, m->master);
		status = 1;
	} else {
		(void) fprintf(stderr,"Can't get any map parameter information.\n");
		(void) fprintf(stderr, s_obad, map,
		    yperr_string(ypprot_err(o->status)) );
		(void) fprintf(stderr, s_mbad, map,
		    yperr_string(ypprot_err(m->status)) );
		status = 1;
	}
}

static void
getypserv()
{
	struct ypbind_resp response;

	getlochost();

	(void) memset((char *)&response, 0, sizeof(response));
	(void) rpc_call(host, YPBINDPROG, YPBINDVERS, YPBINDPROC_DOMAIN,
	    xdr_ypdomain_wrap_string, &domain, xdr_ypbind_resp, 
	    &response, "netpath");
	if (response.ypbind_status != YPBIND_SUCC_VAL) {
		(void) fprintf(stderr, "couldn't get yp server - status %u\n",
		    response.ypbind_status);
		exit(1);
	}
}
