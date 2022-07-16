#ident	"@(#)libyp:yp_b.x	1.1"

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

struct netconfigx {
	string nc_netid<>;			/* token name */
	unsigned long nc_semantics;	/* type of transport */
	unsigned long nc_flag;		/* some flags */
	unsigned long nc_protofmly;	/* Protocol family */
	unsigned long nc_proto;		/* protocol */
	string nc_device<>;	/* device entry */
	unsigned long nc_nlookups;	/* Number of lookup routines */
	unsigned long nc_lookups;	/* look up routines */
	unsigned long nc_unused[8];
};

struct netbufx {
        unsigned int maxlen;
	opaque	buf<>;
/*        unsigned int len;
        char *buf;*/
};


/*
 * Domain binding data structure, used by ypclnt package and ypserv modules.
 * Users of the ypclnt package (or of this protocol) don't HAVE to know about
 * it, but it must be available to users because _yp_dobind is a public
 * interface.
 */

#ifdef RPC_HDR
%
%#include <stdio.h>
%
%extern bool_t xdr_netconfig();
%
%#define YPSETLOCAL 3
%
%struct dom_binding {
%	struct dom_binding *dom_pnext;
%	char *dom_domain;
%	struct ypbind_binding *dom_binding;
%	CLIENT *dom_client;
%};
%
%struct domain {
%	struct domain *dom_pnext;
%	char	*dom_name;
%	bool_t dom_boundp;
%	bool_t dom_yps_complete;
%	unsigned long	dom_error;
%	CLIENT * ping_clnt;
%	struct ypbind_binding *dom_binding;
%	int	dom_report_success;	/* Controls msg to /dev/console*/
%	int	dom_broadcaster_pid;
%	int	bindfile;		/* File with binding info in it */
%	int 	broadcaster_fd;
%	FILE    *broadcaster_pipe;	/*to get answer from locater*/
%	XDR	broadcaster_xdr;	/*xdr for pipe*/
%};
#endif
#ifdef RPC_CLNT
%#define bzero(a,b) memset(a,0,b)
#endif
/*
 *		Protocol between clients and yp binder servers
 */

/*
 * The following procedures are supported by the protocol:
 *
 * YPBINDPROC_NULL() returns ()
 * 	takes nothing, returns nothing
 *
 * YPBINDPROC_DOMAIN takes (char *) returns (struct ypbind_resp)
 *
 * YPBINDPROC_SETDOM takes (struct ypbind_setdom) returns nothing
 */
 
/* Program and version symbols, magic numbers */
/*
 * Response structure and overall result status codes.  Success and failure
 * represent two separate response message types.
 */

enum ypbind_resptype {YPBIND_SUCC_VAL = 1, YPBIND_FAIL_VAL = 2};

%#define YPBIND_ERR_ERR 1		/* Internal error */
%#define YPBIND_ERR_NOSERV 2		/* No bound server for passed domain */
%#define YPBIND_ERR_RESC 3		/* System resource allocation failure */
%#define YPBIND_ERR_NODOMAIN 4		/* Domain doesn't exist */

/*typedef string ypbind_domain<>;*/
struct ypbind_domain {
	string ypbind_domainname<>;
	long int ypbind_vers; /*demanded version == 2*/
	             };

struct ypbind_binding {
	struct netconfig *ypbind_nconf;
	struct netbufx *ypbind_svcaddr;
	string ypbind_servername<>;
	long int ypbind_hi_vers;
	long int ypbind_lo_vers;
};

union ypbind_resp switch (ypbind_resptype ypbind_status){
case YPBIND_FAIL_VAL:
	unsigned long ypbind_error;
case YPBIND_SUCC_VAL:
	struct ypbind_binding *ypbind_bindinfo;
};


/* Detailed failure reason codes for response field ypbind_error*/

#define YPBIND_ERR_ERR 1		/* Internal error */
#define YPBIND_ERR_NOSERV 2		/* No bound server for passed domain */
#define YPBIND_ERR_RESC 3		/* System resource allocation failure */
#define YPBIND_ERR_NODOMAIN 4		/* Domain doesn't exist */

/*
 * Request data structure for ypbind "Set domain" procedure.
 */
struct ypbind_setdom {
	string ypsetdom_domain<>;
	struct ypbind_binding *ypsetdom_bindinfo;
};
#define ypsetdom_netconf ypsetdom_binding.ypbind_binding_netconf
#define ypsetdom_netbuf ypsetdom_binding.ypbind_binding_netbuf

program YPBINDPROG {
	version YPBINDVERS {
	void YPBINDPROC_NULL(void)=0;
	ypbind_resp YPBINDPROC_DOMAIN(ypbind_domain)=1;
	void	YPBINDPROC_SETDOM (ypbind_setdom)=2;
	}=	3;
}= 100007;
