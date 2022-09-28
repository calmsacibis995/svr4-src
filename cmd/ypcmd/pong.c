/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libyp:pong.c	1.5.4.1"

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
#include <string.h>
#include <rpc/rpc.h>
#include <dirent.h>
#include <limits.h>
#include "yp_b.h"
#include "ypsym.h"
#ifndef NULL
#define NULL  0
#endif

#define BINDING "/var/yp/binding"
#define YPSERVERS "ypservers"

listofnames *names();
static int set_binding();
static bool firsttime = TRUE;

extern void free_listofnames();
extern int pipe_setdom();
extern void sysvconfig();
extern void writeit();
extern int yp_getalias();

#define PINGTIME	10
pong_servers(domain,opaque_domain)
char *domain;
struct domain *opaque_domain; /*to pass back*/
{
	CLIENT *clnt2;
	char *servername;
	char domain_alias[MAXNAMLEN+1];
	char outstring[YPMAXDOMAIN + 256];
	listofnames *list,*lin;
	char serverfile[MAXNAMLEN];
	struct timeval timeout;
	char *pname;
	int count=0;
	int isok, res, tried;

	/*
	 * get list of possible servers for this domain
	 */

	/* get alias for domain */
	sysvconfig();
	(void) yp_getalias(domain, domain_alias, MAXALIASLEN);
	sprintf(serverfile,"%s/%s/%s",BINDING,domain_alias,YPSERVERS);
	list=names(serverfile, count);
	lin=list;
	if (list==NULL){
		fprintf(stderr,"cant read %s\n",serverfile);
			 return(-1);
			}
	for (tried = count;list;list = list->nextname, tried--){
		servername=strtok(list->name," \t\n");
		if (servername == NULL) continue;
		if (tried == 0) {
			/*
			 * After ypbind is started up it will not be bound
			 * immediately.  This is normal, no error message 
			 * is needed
			 */
			if (firsttime == TRUE) {
				firsttime = FALSE;
			} else {
				sprintf(outstring,
				    "yp: server not responding for domain %s; still trying.\n", domain);
			}
			tried = count;
			writeit(outstring);
		}
		
		clnt2 = clnt_create(servername, YPPROG, YPVERS, "netpath");
		if (clnt2 == NULL) {
			perror(servername);
			clnt_pcreateerror("ypbind:");
			continue;
		}

		pname=domain;
		timeout.tv_sec=PINGTIME;
		timeout.tv_usec=0;
		if  ((enum clnt_stat ) clnt_call(clnt2,
		    YPPROC_DOMAIN, xdr_ypdomain_wrap_string, &pname, xdr_int, &isok, timeout) == RPC_SUCCESS) {
			if (isok) {

				res=set_binding(clnt2, domain, servername, 
				    opaque_domain);
				clnt_destroy(clnt2);
				free_listofnames(lin);
				return(res);

			} else { 
				fprintf(stderr, 
				    "server %s doesn't serve domain %s\n",
				    servername, domain);
			}
		} else {
			clnt_perror(clnt2,servername);
		}
		clnt_destroy(clnt2);
	}
	free_listofnames(lin);
	return(-2);
}

/*if it pongs ok*/
static int
set_binding(client,domain,servername,opaque_domain)
CLIENT *client;
char *servername;
char *domain;
struct domain *opaque_domain;
{
	struct netconfig *setnc;
	struct netbuf setua;
	ypbind_binding setb;
	ypbind_setdom setd;

	/* get nconf, netbuf structures */
	setnc = getnetconfigent(client->cl_netid);
	clnt_control(client, CLGET_SVC_ADDR, &setua);
	setb.ypbind_nconf= setnc;
	setb.ypbind_svcaddr= (struct netbuf *)&setua;
	setb.ypbind_servername=servername;
	setb.ypbind_hi_vers=0;
	setb.ypbind_lo_vers=0; /*system will figure this out*/
	setd.ypsetdom_bindinfo= & setb;
	setd.ypsetdom_domain=domain;
#ifdef DEBUG
	printf("pong: saving server settings , supports versions %d thru %d\n",
	    setb.ypbind_lo_vers, 
	    setb.ypbind_hi_vers);
	printf("\t\t nc_lookups %s proto %s protofmly %s\n",
	    &(setb.ypbind_nconf->nc_lookups), 
	    setb.ypbind_nconf->nc_proto, 
	    setb.ypbind_nconf->nc_protofmly);
#endif

	if (pipe_setdom(&setd,opaque_domain )  < 0 ) {
#ifdef DEBUG
	printf("YPBIND pipe_setdom failed to server %s\n", servername);
#endif
                return(-1);
        }
#ifdef DEBUG
	else printf("YPBIND OK-set to server %s\n", servername);
#endif
	return(0);
}

static void
writeit(s)
char *s;
{
	FILE *f;

	if ((f = fopen("/dev/console", "w")) != NULL) {
		(void) fprintf(f, "%s.\n", s);
		(void) fclose(f);
	}
}
