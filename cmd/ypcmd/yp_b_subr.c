/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:yp_b_subr.c	1.5.2.1"

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

#include <signal.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <netdir.h>
#include <sys/wait.h>
#include "yp_b.h"
#define PINGTOTTIM 20	/*Total seconds for ping timeout*/

static void broadcast_setup();
static struct ypbind_binding *dup_ypbind_binding();
static void enable_exit();
static int ypbind_find();
static bool ypbind_ping();
static struct domain *ypbind_point_to_domain();

extern struct netconfig *getnetconfigent();
extern int setok;

extern void *calloc();
extern int close();
extern void exit();
extern long fork();
extern void free();
extern int gethostname();
extern void *malloc();
extern int pipe();
extern int pong_servers();
extern int strcmp();
extern char *strcpy();

/*ARGSUSED*/
void	*
ypbindproc_null_3(argp, clnt)
void	*argp;
CLIENT *clnt;
{
	static char	res;

	memset((char *) & res, 0, sizeof(res));
	return ((void *) & res);
}

/*ARGSUSED*/
ypbind_resp *
ypbindproc_domain_3(argp, clnt)
ypbind_domain *argp;
CLIENT *clnt;
{
	static ypbind_resp res;
	struct domain *current_domain;

	memset((char *) & res, 0, sizeof(res));

#ifdef DEBUG
	printf("domain: %s\n", *argp);
#endif

	if ( (current_domain = ypbind_point_to_domain(argp->ypbind_domainname) ) != 
	    (struct domain *) NULL) {

		/*
		 * Ping the server to make sure it is up.
		 */


		if (current_domain->dom_boundp) {
#ifdef DEBUG
			printf("domain is bound pinging: %s\n", *argp);
#endif
			(void) ypbind_ping(current_domain);
		}

		/*
		 * Bound or not, return the current state of the binding.
		 */

		if (current_domain->dom_boundp) {
#ifdef DEBUG
			printf("domain is bound returning: %s\n", *argp);
#endif
			res.ypbind_status = YPBIND_SUCC_VAL;
			res.ypbind_resp_u.ypbind_bindinfo = 
			    current_domain->dom_binding;
		} else {
#ifdef DEBUG
			printf("domain is NOT bound returning: %s %d\n", *argp, current_domain->dom_error);
#endif
			res.ypbind_status = YPBIND_FAIL_VAL;
			res.ypbind_resp_u.ypbind_error = 
			    current_domain->dom_error;
		}

	} else {
		res.ypbind_status = YPBIND_FAIL_VAL;
		res.ypbind_resp_u.ypbind_error = YPBIND_ERR_RESC;
	}
	/*should look for answer if not found*/
	/*Also should check version level and look if version is
			too low*/
	if (res.ypbind_status == YPBIND_FAIL_VAL) {
		ypbind_find(current_domain);
	}
	return (&res);
}


/*ARGSUSED*/
void	*
ypbindproc_setdom_3(argp, clnt, transp)
ypbind_setdom *argp;
CLIENT *clnt;
SVCXPRT *transp;
{
	static char	res;
	struct domain *a_domain;
	struct netbuf *who;

	if (transp != NULL) {
		/* find out who originated the request */
		who = svc_getrpccaller(transp);

		if (setok == FALSE || setok == YPSETLOCAL) {
			struct nd_hostservlist *hostservs = NULL;
			struct netbuf addr;
			struct netconfig *nconf;
			char localhost[64+1];

			if ((nconf = getnetconfigent(transp->xp_netid)) 
			    == (struct netconfig *)NULL) {
				svcerr_systemerr(transp);
				return(0);
			}
			addr.maxlen = who->maxlen;
			addr.len = who->len;
			addr.buf = malloc(who->maxlen);
			memcpy(addr.buf, who->buf, who->len);
			netdir_getbyaddr(nconf, &hostservs, &addr);
			if (hostservs == NULL) {
				fprintf(stderr, 
				    "ypbind: netdir_getbyaddr failed\n\t\tcannot set ypbind to %s", 
				    argp->ypsetdom_bindinfo->ypbind_servername);
				return(0);
			}
			switch (setok) { 
			case FALSE:
				fprintf(stderr,"ypbind: Set domain request to host %s failed (ypset not allowed)",
				    argp->ypsetdom_bindinfo->ypbind_servername);
				fprintf(stderr,"from host %s, failed (ypset not allowed)!\n",
				    hostservs->h_hostservs->h_host);
				svcerr_systemerr(transp);
				return(0);
			case YPSETLOCAL:
				gethostname(localhost, 64);
fprintf(stderr, "ypbind: localhost %s host %s\n",localhost, hostservs->h_hostservs->h_host);
				if (strcmp(hostservs->h_hostservs->h_host, localhost) 
				    != 0) {
					fprintf(stderr,
					    "ypbind: Set domain request to host %s, ",
					    argp->ypsetdom_bindinfo->ypbind_servername);
					fprintf(stderr,
					    "from host %s, failed (not local).\n",
					    hostservs->h_hostservs->h_host);
					svcerr_systemerr(transp);
					return(0);
				}
			}
			netdir_free((char *)hostservs, ND_HOSTSERVLIST);
		}
	}

	memset((char *) & res, 0, sizeof(res));

	if ( (a_domain = ypbind_point_to_domain(argp->ypsetdom_domain) )
	    != (struct domain *) NULL) {
		a_domain->dom_boundp = TRUE;
		a_domain->dom_yps_complete = FALSE;
#ifdef DEBUG
		printf("setting domain %s\n", argp->ypsetdom_domain);
#endif
		/*this does the set --should copy the structure*/
		a_domain->dom_binding = dup_ypbind_binding(argp->ypsetdom_bindinfo);

		/* get rid of old pinging client if one exists */
		if (a_domain->ping_clnt != (CLIENT * )NULL) {

			clnt_destroy(a_domain->ping_clnt);
			a_domain->ping_clnt = (CLIENT * )NULL;
		}
	}

	return ((void *) & res);
}


/*
 * This returns a pointer to a domain entry.  If no such domain existed on
 * the list previously, an entry will be allocated, initialized, and linked
 * to the list.  Note:  If no memory can be malloc-ed for the domain structure,
 * the functional value will be (struct domain *) NULL.
 */
static struct domain *known_domains;
static struct domain *
ypbind_point_to_domain(pname )
register char	*pname;
{
	register struct domain *pdom;
	char *strdup();

	for (pdom = known_domains; pdom != (struct domain *)NULL; 
	    pdom = pdom->dom_pnext) {
		if (!strcmp(pname, pdom->dom_name) )
			return (pdom);
	}

	/* Not found.  Add it to the list */

	if (pdom = (struct domain *)calloc(1, sizeof (struct domain ))) {
		pdom->dom_name=strdup(pname);
		if (pdom->dom_name==NULL){
			free((char *)pdom);
			printf("ypbind_point_to_domain: strdup failed\n" );
			return(NULL);
		}
		pdom->dom_pnext = known_domains;
		known_domains = pdom;
		pdom->dom_boundp = FALSE;
		pdom->dom_yps_complete = FALSE;
		pdom->dom_binding = NULL;
		pdom->dom_error = YPBIND_ERR_NOSERV;
		pdom->ping_clnt = (CLIENT * )NULL;
		pdom->dom_report_success = -1;
		pdom->dom_broadcaster_pid = 0;
		pdom->bindfile = -1;
	} 
	else 
		printf("ypbind_point_to_domain: malloc failed\n");

	return (pdom);
}


static bool 
ypbind_ping(pdom)
struct domain *pdom;
{
	enum clnt_stat clnt_stat;
	struct timeval timeout;
	int	vers;
	struct rpc_err rpcerr;
	int	isok;
	char	*pname;
	bool new_binding = FALSE;
	CLIENT	*hold;

	if (pdom->dom_boundp == FALSE)
		return (FALSE);
	if (pdom->dom_yps_complete)
		vers = pdom->dom_binding->ypbind_hi_vers;
	else 
		vers = 0;

	if (pdom->ping_clnt == (CLIENT * ) NULL) {
#ifdef DEBUG
	int	i;

		printf("device %s netid %s\n",
		    pdom->dom_binding->ypbind_nconf->nc_device,
		    pdom->dom_binding->ypbind_nconf->nc_netid);
		printf("addr max %d len %d\n",
		    pdom->dom_binding->ypbind_svcaddr->maxlen,
		    pdom->dom_binding->ypbind_svcaddr->len);
		for (i = 0; i < pdom->dom_binding->ypbind_svcaddr->len; i++) {
			printf("%d.", pdom->dom_binding->ypbind_svcaddr->buf[i]);
		}
		printf("\n");
	printf("ypbind_ping: supports versions %d thru %d\n",
	    pdom->dom_binding->ypbind_lo_vers, 
	    pdom->dom_binding->ypbind_hi_vers);
	printf("\t\t nc_lookups %s proto %s protofmly %s\n",
	    *(pdom->dom_binding->ypbind_nconf->nc_lookups), 
	    pdom->dom_binding->ypbind_nconf->nc_proto, 
	    pdom->dom_binding->ypbind_nconf->nc_protofmly);
#endif
		if (pdom->dom_binding->ypbind_nconf->nc_semantics == 
		    (unsigned long) NC_TPI_CLTS || 
		    (strcmp(pdom->dom_binding->ypbind_nconf->nc_netid,"tcp") 
		    == 0))
			pdom->ping_clnt = clnt_tli_create(RPC_ANYFD,
			    pdom->dom_binding->ypbind_nconf,
			    pdom->dom_binding->ypbind_svcaddr,
			    YPPROG, vers, 0, 0);
		else
			pdom->ping_clnt = clnt_tp_create(
			    pdom->dom_binding->ypbind_servername,
			    YPPROG, vers, pdom->dom_binding->ypbind_nconf);
	}

	if (pdom->ping_clnt == (CLIENT * ) NULL) {
		perror("clnt_create");
		clnt_pcreateerror("ypbind_ping()");
		pdom->dom_boundp = FALSE;
		pdom->dom_yps_complete = FALSE;
		pdom->dom_error = YPBIND_ERR_NOSERV;
		return(FALSE);
	}


#ifdef DEBUG
	printf("ypbind: ypbind_ping()\n");
#endif
	timeout.tv_sec = PINGTOTTIM;
	timeout.tv_usec =  0;
	if (!pdom->dom_yps_complete) {
		/*Find out versions*/
		if ((clnt_stat = (enum clnt_stat ) clnt_call(pdom->ping_clnt,
		    YPPROC_NULL, xdr_void, 0, xdr_void, 0, timeout)) == RPC_SUCCESS) {
#ifdef DEBUG
			printf("Error couldn't find versions!!!\n");
#endif
			pdom->dom_boundp = FALSE;
			pdom->dom_yps_complete = FALSE;
			pdom->dom_error = YPBIND_ERR_NOSERV;
			return(new_binding);
			/*Didnt find versions*/
		} else if (clnt_stat == RPC_PROGVERSMISMATCH) {
			clnt_geterr(pdom->ping_clnt, &rpcerr);
			pdom->dom_binding->ypbind_lo_vers = rpcerr.re_vers.low;
			pdom->dom_binding->ypbind_hi_vers = rpcerr.re_vers.high;
			pdom->dom_boundp = TRUE;
			pdom->dom_yps_complete = TRUE;
#ifdef DEBUG
			printf("Server pinged successfully, supports versions %d thru %d\n",
			    pdom->dom_binding->ypbind_lo_vers, 
			    pdom->dom_binding->ypbind_hi_vers);
#endif
			hold=pdom->ping_clnt;
			pdom->ping_clnt = (CLIENT * ) NULL;
			new_binding=ypbind_ping(pdom);
			if (hold) clnt_destroy(hold);
			return(new_binding);
		} else
		{
			clnt_perror(pdom->ping_clnt, "ping for versions");
			pdom->dom_boundp = FALSE;
			if (pdom->ping_clnt)
				clnt_destroy(pdom->ping_clnt);
			pdom->ping_clnt = (CLIENT * )NULL;
		}
	}
	if (pdom->dom_boundp) {
		pname = pdom->dom_name;
		if ((clnt_stat = (enum clnt_stat ) clnt_call(pdom->ping_clnt,
		    YPPROC_DOMAIN, xdr_ypdomain_wrap_string, &pname, xdr_int, &isok, timeout)) == RPC_SUCCESS) {
			pdom->dom_boundp = isok;
#ifdef DEBUG
			printf("server status for domain %s is %d\n", pname, isok);
#endif
			return(new_binding);

		} else	{
			clnt_perror(pdom->ping_clnt, "ping");
			pdom->dom_boundp = FALSE;
			pdom->dom_yps_complete = FALSE;
			pdom->dom_error = YPBIND_ERR_NOSERV;
		}
	}
#ifdef DEBUG
	printf("ypbind: ypbind_ping() rpc call returned %d\n", clnt_stat);
#endif
	/*Could leave this open*/
	if (pdom->ping_clnt)
		clnt_destroy(pdom->ping_clnt);
	pdom->ping_clnt = (CLIENT * )NULL;
	return(new_binding);

}

static struct ypbind_binding *
dup_ypbind_binding(a)
struct ypbind_binding *a;
{
	char *strdup();
	struct ypbind_binding *b;
	struct netconfig *nca, *ncb;
	struct netbuf *nxa, *nxb;
	int i;

	b=(struct ypbind_binding *)calloc(1,sizeof(*b));
	if (b==NULL) return(b);
	b->ypbind_hi_vers=a->ypbind_hi_vers;
	b->ypbind_lo_vers=a->ypbind_lo_vers;
	b->ypbind_servername=strdup(a->ypbind_servername);
	ncb=(b->ypbind_nconf=(struct netconfig *) calloc(1,sizeof(struct netconfig)));
	nxb=(b->ypbind_svcaddr= (struct netbuf *) calloc(1,sizeof(struct netbuf)));
	nca=a->ypbind_nconf;
	nxa=a->ypbind_svcaddr;
	ncb->nc_flag=nca->nc_flag;
	ncb->nc_protofmly=nca->nc_protofmly;
	ncb->nc_proto=nca->nc_proto;
	ncb->nc_semantics=nca->nc_semantics;
	ncb->nc_netid=strdup(nca->nc_netid);
	ncb->nc_device=strdup(nca->nc_device);
	ncb->nc_nlookups=nca->nc_nlookups;
	ncb->nc_lookups=nca->nc_lookups;
	for (i=0;i < 8;i++)
		ncb->nc_unused[i]=nca->nc_unused[i];
	nxb->maxlen=nxa->maxlen;
	nxb->len=nxa->len;
	nxb->buf=malloc(nxa->maxlen);
	memcpy(nxb->buf,nxa->buf,nxb->len);
	return(b);
}

void
broadcast_proc_exit()
{
	int pid;
	int wait_status;
	register struct domain *pdom;
	pid = 0;

	pid = wait(&wait_status);
#ifdef DEBUG
	fprintf(stderr,"got wait from %d status=%d\n",pid,wait_status);
#endif
	if (pid == 0) {
		enable_exit();
		return;
	} else if (pid == -1) {
		enable_exit();
		return;
	}

	for (pdom = known_domains; pdom != (struct domain *)NULL;
	    pdom = pdom->dom_pnext) {
#ifdef DEBUG
		fprintf(stderr,"domain %s pid=%d\n",pdom->dom_name,
		    pdom->dom_broadcaster_pid);
#endif

		if (pdom->dom_broadcaster_pid == pid) {
			pdom->dom_broadcaster_pid = 0;
#ifdef DEBUG
			fprintf(stderr,"got match %s\n",pdom->dom_name);
#endif
                         if ((WTERMSIG(wait_status) == 0) &&
                             (WEXITSTATUS(wait_status) == 0))
			{
				broadcast_setup(pdom);
			}

		}
	}
	enable_exit();
	return;
}

static void
enable_exit()
{
	sigset(SIGCHLD,(void (*)())broadcast_proc_exit);
}

static void
broadcast_setup(pdom)
struct domain *pdom;
{
	ypbind_setdom req;
	memset(&req,0,sizeof(req));
	if (pdom->broadcaster_pipe){
		if(xdr_ypbind_setdom( &(pdom->broadcaster_xdr), &req)){
			pdom->dom_report_success = -1;
#ifdef DEBUG
			fprintf(stderr,"setup: got xdr ok \n");
#endif
			ypbindproc_setdom_3(&req, (CLIENT *)NULL, 
			    (SVCXPRT *)NULL);
#ifdef DEBUG
		} else {
			fprintf(stderr,"setup: xdr failed\n");
#endif
		}
		xdr_destroy( &(pdom->broadcaster_xdr));
		fclose(pdom->broadcaster_pipe);
#ifdef DEBUG
	} else {
		fprintf(stderr,"setup: no broadcaster pipe\n");
#endif
	}

	close(pdom->broadcaster_fd);
	pdom->broadcaster_pipe=0;
	pdom->broadcaster_fd= -1;
}

static int
ypbind_find(domain)
struct domain *domain;
{
	int bpid;
	int fildes[2];
	if ((domain) && (!domain->dom_boundp) &&
	    (!domain->dom_broadcaster_pid)) {
		/*
		 * The current domain is unbound, and there is no broadcaster 
		 * process active now.  Fork off a child who will yell out on 
		 * the net.  Because of the flavor of request we're making of 
		 * the server, we only expect positive ("I do serve this
		 * domain") responses.
		 */
		if (pipe(fildes)<0) return(-1);

		enable_exit();
		sighold(SIGCLD);
		bpid=fork();
		if (bpid!=0) { /*parent*/
			if (bpid>0){ /*parent started*/
				close (fildes[1]);
				domain->dom_broadcaster_pid=bpid;
				domain->dom_report_success++;
				domain->broadcaster_fd=fildes[0];
				domain->broadcaster_pipe=fdopen(fildes[0],"r");
				if (domain->broadcaster_pipe)
					xdrstdio_create(&(domain->broadcaster_xdr), (domain->broadcaster_pipe),XDR_DECODE);

#ifdef DEBUG
				printf("ypbind_find: %s starting pid=%d try=%d\n",
				    domain->dom_name,bpid,
				    domain->dom_report_success);
#endif
				sigrelse(SIGCLD);
				return(0);
			}
			else { /*fork failed*/
				perror("fork");
				close(fildes[0]);
				close(fildes[1]);
				sigrelse(SIGCLD);
				return(-1);
			}
		} /*end parent*/
		/*child only code*/
		sigrelse(SIGCLD);
		close(fildes[0]);
		domain->broadcaster_fd=fildes[1];
		domain->broadcaster_pipe=fdopen(fildes[1],"w");
		if (domain->broadcaster_pipe)
			xdrstdio_create(&(domain->broadcaster_xdr), (domain->broadcaster_pipe),XDR_ENCODE);
		exit(pong_servers(domain->dom_name,domain));
	}
	return(0);
}

int
pipe_setdom(res,pdom)
ypbind_setdom *res;
struct domain *pdom;
{
	int retval;

	if(xdr_ypbind_setdom( &(pdom->broadcaster_xdr), res)) {
#ifdef DEBUG
		fprintf(stderr,"setdom:sent xdr ok \n");
#endif
		retval = 0;
	} else {
#ifdef DEBUG
		fprintf(stderr,"setdom:xdr failed\n") ;
#endif
		retval = -1;
	}
	xdr_destroy( &(pdom->broadcaster_xdr));
	fclose(pdom->broadcaster_pipe);
	close(pdom->broadcaster_fd);
	pdom->broadcaster_pipe=0;
	pdom->broadcaster_fd= -1;
	return(retval);
}
