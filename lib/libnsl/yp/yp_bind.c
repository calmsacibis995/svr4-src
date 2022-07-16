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


#ident	"@(#)libyp:yp_bind.c	1.4.2.1"

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
static  char sccsid[] = "@(#)yp_bind.c 1.36 88/07/16 Copyr 1985 Sun Micro";
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <rpc/rpc.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_ERR 3
#endif /* SYSLOG */
#include "yp_b.h"
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

#define BFSIZE (YPMAXDOMAIN + 32) /* size of binding file */
#define YPBINDPROTO "netpath"     /* use anything*/

/* This should match the one in ypbind.c */

#define CACHE_DIR "/var/yp/binding"	
extern int errno;
extern void free();
extern pid_t getpid();
extern char *malloc();
extern unsigned sleep();
extern char *strcpy();
extern int strcmp();
extern size_t strlen();
extern int syslog();
extern int getdomainname();
extern char * _rpc_gethostname();


static bool check_binding();
static void newborn();
static struct dom_binding *load_dom_binding();

/*
 * Time parameters when talking to the ypbind and pmap processes
 */

#define YPSLEEPTIME 5			/* Time to sleep between tries */
unsigned int _ypsleeptime = YPSLEEPTIME;




/*
 * Time parameters when talking to the ypserv process
 */

#ifdef  DEBUG
#define YPTIMEOUT 120			/* Total seconds for timeout */
#define YPINTER_TRY 60			/* Seconds between tries */
#else
#define YPTIMEOUT 20			/* Total seconds for timeout */
#define YPINTER_TRY 5			/* Seconds between tries */
#endif

#define MAX_TRIES_FOR_NEW_YP 1		/* Number of times we'll try to get
					 *   a new YP server before we'll
					 *   settle for an old one. */
struct timeval _ypserv_timeout = {
	YPTIMEOUT,			/* Seconds */
	0				/* Microseconds */
	};


static struct dom_binding *bound_domains; /* List of bound domains */
static char *default_domain;

/*
 * Attempts to locate a YP server that serves a passed domain.  If one 
 * is found, an entry is created on the static list of domain-server pairs
 * pointed to by cell bound_domains, a udp path to the server is created and
 * the function returns 0.  Otherwise, the function returns a defined errorcode
 * YPERR_xxxx.
 */
int
_yp_dobind(domain, binding)
char *domain;
struct dom_binding **binding;	/* if result == 0, ptr to dom_binding */
{
	struct dom_binding *pdomb;	/* Ptr to new domain binding */
	struct ypbind_resp *ypbind_resp; /* Response from local ypbinder */
	struct ypbind_domain ypbd; 
	int status;
	int tries;
	CLIENT *tb;

	if ( (domain == NULL) ||((int) strlen(domain) == 0) ) {
		return (YPERR_BADARGS);
	}

	newborn();


	if (check_binding(domain, binding) )
		return (0);		/* We are bound */

	while(1){
		tb=clnt_create(_rpc_gethostname(),
		    (u_long)YPBINDPROG,YPBINDVERS,YPBINDPROTO);
		if (tb==NULL) return(YPERR_YPBIND);

		for(tries=0;tries<5;tries++){
			ypbd.ypbind_domainname=domain;
			ypbd.ypbind_vers=YPVERS;
			ypbind_resp=ypbindproc_domain_3(&ypbd,tb);
			if (ypbind_resp==NULL){
				/*lost ypbind?*/
				clnt_perror(tb,"ypbindproc_domain_3");
				break;
			}
			if (ypbind_resp->ypbind_status==YPBIND_SUCC_VAL){
				/*gotcha*/

				if ( (pdomb = load_dom_binding(ypbind_resp,  domain, &status) ) ==
				    (struct dom_binding *) NULL) {
					clnt_destroy(tb);
					return (status);
				}
				clnt_destroy(tb);
				/* Return ptr to the binding entry */
				*binding = pdomb;
				/* This is the go path */
				return (0);
			}
			if (ypbind_resp->ypbind_resp_u.ypbind_error!=YPBIND_ERR_NOSERV)
			{
				clnt_destroy(tb);
				return(YPERR_YPBIND);
			}
			(void) sleep(_ypsleeptime*tries);

		}
		clnt_destroy(tb);
	}
}

/*
 * This is a "wrapper" function for _yp_dobind for vanilla user-level
 * functions which neither know nor care about struct dom_bindings.
 */
int
yp_bind(domain)
char *domain;
{

	struct dom_binding *binding;

	return (_yp_dobind(domain, &binding) );
}

/*
 * Attempts to find a dom_binding in the list at bound_domains having the
 * domain name field equal to the passed domain name, and removes it if found.
 * The domain-server binding will not exist after the call to this function.
 * All resources associated with the binding will be freed.
 */
void
yp_unbind (domain)
char *domain;
{
	struct dom_binding *pdomb;
	struct dom_binding *ptrail = 0;


	if ( (domain == NULL) ||((int) strlen(domain) == 0) ) {
		return;
	}


	for (pdomb = bound_domains; pdomb != NULL;
	    ptrail = pdomb, pdomb = pdomb->dom_pnext) {

		if (strcmp(domain, pdomb->dom_domain) == 0) {

			clnt_destroy(pdomb->dom_client);

			if (pdomb == bound_domains) {
				bound_domains = pdomb->dom_pnext;
			} else {
				ptrail->dom_pnext = pdomb->dom_pnext;
			}

			free((char *) pdomb);
			break;
		}

	}


}

static char *
_default_domain()
{
	char temp[256];

	if (default_domain)
		return (default_domain);
	if (getdomainname(temp, sizeof(temp)) < 0)
		return (0);
	if ((int) strlen(temp) > 0) {
		default_domain = (char *)malloc((strlen(temp)+(unsigned)1));
		if (default_domain == 0)
			return (0);
		(void) strcpy(default_domain, temp);
		return (default_domain);
	}
	return (0);
}

/*
 * This is a wrapper for the system call getdomainname which returns a
 * ypclnt.h error code in the failure case.  It also checks to see that
 * the domain name is non-null, knowing that the null string is going to
 * get rejected elsewhere in the yp client package.
 */
int
yp_get_default_domain(domain)
char **domain;
{

	if ((*domain = _default_domain()) != 0)
		return (0);
	return (YPERR_YPERR);
}

/*
 * This checks to see if this is a new process incarnation which has
 * inherited bindings from a parent, and unbinds the world if so.
 */
static void
newborn()
{
	static pid_t mypid;	/* Cached to detect forks */
	pid_t testpid;

	if ((testpid = getpid() ) != mypid) {
		mypid = testpid;

		while (bound_domains) {
			yp_unbind(bound_domains->dom_domain);
		}
	}
}

/*
 * This checks that the socket for a domain which has already been bound
 * hasn't been closed or changed under us.  If it has, unbind the domain
 * without closing the socket, which may be in use by some higher level
 * code.  This returns TRUE and points the binding parameter at the found
 * dom_binding if the binding is found and the socket looks OK, and FALSE
 * otherwise.  
 */
static bool
check_binding(domain, binding)
char *domain;
struct dom_binding **binding;
{
	struct dom_binding *pdomb;

	for (pdomb = bound_domains; pdomb != NULL; pdomb = pdomb->dom_pnext) {

		if (strcmp(domain, pdomb->dom_domain) == 0) {


			*binding = pdomb;
			return (TRUE);
		}
	}

	return (FALSE);
}

/*
 * This allocates some memory for a domain binding, initialize it, and
 * returns a pointer to it.  Based on the program version we ended up
 * talking to ypbind with, fill out an opvector of appropriate protocol
 * modules.  
 */
static struct dom_binding *
load_dom_binding(ypbind_res, domain, err)
struct ypbind_resp *ypbind_res;
char *domain;
int *err;
{
	struct dom_binding *pdomb;
	struct netconfig *nconf;

	pdomb = (struct dom_binding *) NULL;

	if ((pdomb = (struct dom_binding *) malloc(sizeof(struct dom_binding)))
	    == NULL) {
		(void) syslog(LOG_ERR, "load_dom_binding:  malloc failure.");
		*err = YPERR_RESRC;
		return (struct dom_binding *) (NULL);
	}

	pdomb->dom_binding = ypbind_res->ypbind_resp_u.ypbind_bindinfo;
	nconf = pdomb->dom_binding->ypbind_nconf;

	/*
	 * Open up a path to the server, which will remain active globally.
	 *	If using CLTS or "tcp" use netbuf passed by ypbind
	 */
	if (nconf->nc_semantics == (unsigned long) NC_TPI_CLTS || 
	    (strcmp(nconf->nc_netid,"tcp") == 0))
		pdomb->dom_client = clnt_tli_create(RPC_ANYFD, nconf,
		    pdomb->dom_binding->ypbind_svcaddr, YPPROG, YPVERS, 0, 0);
	else 
		pdomb->dom_client = clnt_tp_create(
		    pdomb->dom_binding->ypbind_servername,
		    (u_long) YPPROG, YPVERS, nconf);

	if (pdomb->dom_client == (CLIENT * ) NULL) {
		clnt_pcreateerror("yp_bind: clnt_tp_create");
		free((char *) pdomb);
		*err = YPERR_RPC;
		return (struct dom_binding *) (NULL);
	}

	pdomb->dom_pnext = bound_domains;	/* Link this to the list as */
	pdomb->dom_domain=malloc(strlen(domain)+(unsigned)1);
	if (pdomb->dom_domain == NULL){
		free((char *) pdomb);
		*err = YPERR_RESRC;
		return (struct dom_binding *) (NULL);
	}

	(void) strcpy(pdomb->dom_domain, domain);/* Remember the domain name */
	bound_domains = pdomb;			/* ... the head entry */
	return (pdomb);
}

int
usingypmap(ddn, map)
char **ddn;  /* the default domainname set by this routine */
char *map;  /* the map we are interested in. */
{
	char in, *outval = NULL;
	int outvallen, stat;

	if (_default_domain() == 0) return (FALSE);
	/* does the map exist ? */
	in = 0xFF;
	stat = yp_match(*ddn=default_domain, map, &in, 1, &outval, &outvallen);
	if (outval != NULL)
		free(outval);
	switch (stat) {

	case 0:  /* it actually succeeded! */
	case YPERR_KEY:  /* no such key in map */
	case YPERR_NOMORE:
	case YPERR_BUSY:
		return (TRUE);
	}
	return (FALSE);
}
