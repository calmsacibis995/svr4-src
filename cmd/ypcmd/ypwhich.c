/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:ypwhich.c	1.4.2.1"

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
static  char sccsid[] = "@(#)ypwhich.c 1.12 87/06/23 Copyr 1985 Sun Micro";
#endif

/*
 * This is a user command which tells which yp server is being used by a
 * given machine, or which yp server is the master for a named map.
 * 
 * Usage is:
 * ypwhich [-d domain] [-m [mname] [-t] | [-V1 | -V2] host]
 * 
 * where:  the -d switch can be used to specify a domain other than the
 * default domain.  -m tells the master of that map.  mname is a mapname
 * If the -m option is used, ypwhich will act like a vanilla yp client,
 * and will not attempt to choose a particular yp server.  On the
 * other hand, if no -m switch is used, ypwhich will talk directly to the yp
 * bind process on the named host, or to the local ypbind process if no host
 * name is specified.
 *  
 */

#include <stdio.h>
#include <ctype.h>
#include <rpc/rpc.h>
#include "yp_b.h"
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
#include "ypv2_bind.h"

#define TIMEOUT 30			/* Total seconds for timeout */
#define INTER_TRY 10			/* Seconds between tries */

static bool newvers = FALSE;
static bool oldvers = FALSE;
static bool ask_specific = FALSE;
static char *domain = NULL;
static char default_domain_name[YPMAXDOMAIN];
static char *host = NULL;
static char default_host_name[256];

static bool get_master = FALSE;
static bool get_server = FALSE;
static char *map = NULL;
static struct timeval timeout = {
		TIMEOUT,			/* Seconds */
		0				/* Microseconds */
};
static char err_usage[] =
"Usage:\n\
	ypwhich [-d domain] [[-t] -m [mname] | [-V1 | -V2] host]\n";
static char err_bad_args[] =
"ypwhich:  %s argument is bad.\n";
static char err_cant_get_kname[] =
"ypwhich:  can't get %s back from system call.\n";
static char err_null_kname[] =
"ypwhich:  the %s hasn't been set on this machine.\n";
static char err_bad_mapname[] = "mapname";
static char err_bad_domainname[] = "domainname";
static char err_bad_hostname[] = "hostname";

static void get_command_line_args();
static void getdomain();
static void getlochost();
static void get_server_name();
static bool call_binder();
static void get_map_master();
#ifdef DEBUG
static void dump_response();
#endif
static void dump_ypmaps();
static void dumpmaps();

extern size_t strlen();
extern int strcmp();
extern int strncmp();
extern int getdomainname();
extern int gethostname();
extern void exit();
extern void free();

extern CLIENT *clnt_create();
extern void clnt_pcreateerror();
extern void clnt_perror();

/*
 * This is the main line for the ypwhich process.
 */
main(argc, argv)
char **argv;
{
	get_command_line_args(argc, argv);

	if (!domain) {
		getdomain();
	}

	if (get_server) {

		if (!host) {
			getlochost();
		}

		get_server_name();
	} else {

		if (map) {
			get_map_master();
		} else {
			dump_ypmaps();
		}
	}

	return(0);
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

	if (argc == 1) {
		get_server = TRUE;
		return;
	}

	while (--argc) {

		if ( (*argv)[0] == '-') {

			switch ((*argv)[1]) {

			case 'V':

				if ((*argv)[2] == '1') {
					oldvers = TRUE;
					argv++;
				} else if ((*argv)[2] == '2') {
					newvers = TRUE;
					argv++;
				} else {
					(void) fprintf(stderr, err_usage);
					exit(1);
				}
				break;

			case 'm':
				get_master = TRUE;
				argv++;

				if (argc > 1) {

					if ( (*(argv))[0] == '-') {
						break;
					}

					argc--;
					map = *argv;
					argv++;

					if ((int)strlen(map) > YPMAXMAP) {
						(void) fprintf(stderr, err_bad_args,
						    err_bad_mapname);
						exit(1);
					}

				}

				break;

			case 'd':

				if (argc > 1) {
					argv++;
					argc--;
					domain = *argv;
					argv++;

					if ((int)strlen(domain) > YPMAXDOMAIN) {
						(void) fprintf(stderr, err_bad_args,
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

			if (get_server) {
				(void) fprintf(stderr, err_usage);
				exit(1);
			}

			get_server = TRUE;
			host = *argv;
			argv++;

			if ((int)strlen(host) > 256) {
				(void) fprintf(stderr, err_bad_args, err_bad_hostname);
				exit(1);
			}
		}
	}

	if (newvers && oldvers) {
		(void) fprintf(stderr, err_usage);
		exit(1);
	}

	if (newvers || oldvers) {
		ask_specific = TRUE;
	}

	if (get_master && get_server) {
		(void) fprintf(stderr, err_usage);
		exit(1);
	}

	if (!get_master && !get_server) {
		get_server = TRUE;
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

/*
 * This tries to find the name of the server to which the binder in question
 * is bound.  If one of the -Vx flags was specified, it will try only for
 * that protocol version, otherwise, it will start with the current version,
 * then drop back to the previous version.
 */
static void
get_server_name()
{
	int vers;
	char *notbound = "Domain %s not bound.\n";

	if (ask_specific) {
		vers = oldvers ? YPBINDOLDVERS : YPBINDVERS;

		if (!call_binder(vers )) {
			(void) fprintf(stderr, notbound, domain);
		}

	} else {
		vers = YPBINDVERS;

		if (!call_binder(vers )) {

			vers = YPBINDOLDVERS;

			if (!call_binder(vers)) {
				(void) fprintf(stderr, notbound, domain);
			}
		}
	}
}

/*
 * This sends a message to the ypbind process on the node with 
 * the host name
 */
static bool
call_binder(vers )
int vers;
{
	CLIENT *client;
	struct ypbind_resp *response;
	struct ypbind_domain ypbd;
	char errstring[256];
	extern struct rpc_createerr rpc_createerr;


	if ((client = clnt_create(host, YPBINDPROG, YPBINDVERS, "netpath"
	    )) == NULL) {
		if (rpc_createerr.cf_stat == RPC_PROGNOTREGISTERED) {
			(void) fprintf(stderr, "ypwhich: %s is not running ypbind\n", 
			    host);
			exit(1);
		}
		if (rpc_createerr.cf_stat == RPC_PMAPFAILURE) {
			(void) fprintf(stderr, "ypwhich: %s is not running rpcbind\n", 
			    host);
			exit(1);
		}
		(void) clnt_pcreateerror("ypwhich:  clnt_create error");
		exit(1);
	}
	ypbd.ypbind_domainname=domain;
	ypbd.ypbind_vers=vers;
	response= ypbindproc_domain_3(&ypbd,client);

	if (response==NULL){
		(void) sprintf(errstring,
		    "ypwhich: can't call ypbind on %s", host);
		(void) clnt_perror(client, errstring);
		exit(1);
	}

	clnt_destroy(client);

	if(response->ypbind_status != YPBIND_SUCC_VAL)  {
		return (FALSE);
	}
	if (response->ypbind_resp_u.ypbind_bindinfo)
		(void) fprintf(stdout, "%s\n",response->ypbind_resp_u.ypbind_bindinfo->ypbind_servername);
#ifdef DEBUG
	dump_response(response);
#endif
	return (TRUE);
}

#ifdef DEBUG
static void
dump_response(which)
ypbind_resp * which;
{
	struct netconfig *nc;
	struct netbuf *ua;
	ypbind_binding * b;

	int i;

	{
		b = which->ypbind_resp_u.ypbind_bindinfo;
		if (b == NULL)
			(void) fprintf(stderr, "???NO Binding information\n");
		else {
			(void) fprintf(stderr, "server=%s lovers=%ld hivers=%ld\n", 
			    b->ypbind_servername, b->ypbind_lo_vers, b->ypbind_hi_vers);
			nc = b->ypbind_nconf;
			ua = b->ypbind_svcaddr;
			if (nc == NULL)
				(void) fprintf(stderr, "ypwhich: NO netconfig information\n");
			else {
				(void) fprintf(stderr, "ypwhich: id %s device %s flag %x protofmly %s proto %s\n",
				    nc->nc_netid, nc->nc_device, 
				    (int) nc->nc_flag, nc->nc_protofmly,
				    nc->nc_proto);
			}
			if (ua == NULL)
				(void) fprintf(stderr,"ypwhich: NO netbuf information available from binder\n");
			else {
				(void) fprintf(stderr, "maxlen=%d len=%d\naddr=", ua->maxlen, ua->len);
				for (i = 0; i < ua->len; i++) {
					if (i != (ua->len - 1))
						(void) fprintf(stderr, "%d.", ua->buf[i]);
					else 
						(void) fprintf(stderr, "%d\n", ua->buf[i]);
				}
			}
		}
	}

}
#endif

/*
 * This translates a server address to a name and prints it.  If the address
 * is the same as the local address as returned by get_myaddress, the name
 * is that retrieved from the kernel.  If it's any other address (including
 * another ip address for the local machine), we'll get a name by using the
 * standard library routine (which calls the yp).  
 */

/*
 * This asks any yp server for the map's master.  
 */
static void
get_map_master()
{
	int err;
	char *master;

	err = yp_master(domain, map, &master);

	if (err) {
		(void) fprintf(stderr,
		    "ypwhich:  Can't find the master of %s.  Reason: %s.\n",
		    map, yperr_string(err) );
	} else {
		(void) printf("%s\n", master);
	}
}

/*
 * This enumerates the entries within map "ypmaps" in the domain at global 
 * "domain", and prints them out key and value per single line.  dump_ypmaps
 * just decides whether we are (probably) able to speak the new YP protocol,
 * and dispatches to the appropriate function.
 */
static void
dump_ypmaps()
{
	int err;
	struct dom_binding *binding;

	if (err = _yp_dobind(domain, &binding)) {
		(void) fprintf(stderr,
		    "dump_ypmaps: Can't bind for domain %s.  Reason: %s\n",
		    domain, yperr_string(ypprot_err(err)));
		return;
	}

	if (binding->dom_binding->ypbind_hi_vers  >= YPVERS) {
		dumpmaps(binding);
	} 
}

static void
dumpmaps(binding)
struct dom_binding *binding;
{
	enum clnt_stat rpc_stat;
	int err;
	char *master;
	struct ypmaplist *pmpl;
	struct ypresp_maplist maplist;

	maplist.list = (struct ypmaplist *) NULL;

	rpc_stat = clnt_call(binding->dom_client, YPPROC_MAPLIST,
	    xdr_ypdomain_wrap_string, &domain, xdr_ypresp_maplist, &maplist,
	    timeout);

	if (rpc_stat != RPC_SUCCESS) {
		(void) clnt_perror(binding->dom_client,
		    "ypwhich(dumpmaps): can't get maplist");
		exit(1);
	}

	if (maplist.status != YP_TRUE) {
		(void) fprintf(stderr,
		    "ypwhich:  Can't get maplist.  Reason:  %s.\n",
		    yperr_string(ypprot_err(maplist.status)) );
		exit(1);
	}

	for (pmpl = maplist.list; pmpl; pmpl = pmpl->ypml_next) {
		(void) printf("%s ", pmpl->ypml_name);

		err = yp_master(domain, pmpl->ypml_name, &master);

		if (err) {
			(void) printf("????????\n");
			(void) fprintf(stderr,
			    "ypwhich:  Can't find the master of %s.  Reason: %s.\n",
			    pmpl->ypml_name, yperr_string(err) );
		} else {
			(void) printf("%s\n", master);
		}
	}
}
