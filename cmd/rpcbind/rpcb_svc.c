/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rpcbind:rpcb_svc.c	1.13.3.1"

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

/*
 * rpcb_svc.c
 * The server procedure for the version 3 rpcbind (TLI).
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*
 * It maintains a separate list of all the registered services with the
 * version 3 of rpcbind.
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/rpcb_prot.h>
#include <netconfig.h>
#include <sys/param.h>
#ifdef PORTMAP
#include <netinet/in.h>
#include <rpc/pmap_prot.h>
#endif /* PORTMAP */
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define	LOG_ERR 3
#endif /* SYSLOG */
#include <netdir.h>
#include "rpcbind.h"

extern void *malloc();
extern char *strdup();
static bool_t xdr_opaque_parms();
static bool_t xdr_len_opaque_parms();
static RPCBLIST *find_service();
static bool_t *rpcbproc_set_3();
static bool_t *rpcbproc_unset_3();
static char **rpcbproc_getaddr_3();
static RPCBLIST **rpcbproc_dump_3();
static u_long *rpcbproc_gettime_3();
static struct netbuf *rpcbproc_uaddr2taddr_3();
static char **rpcbproc_taddr2uaddr_3();
static char *getowner();

static char *nullstring = "";

/*
 * Called by svc_getreqset. There is a separate server handle for
 * every transport that it waits on.
 * 1 OK, 0 not
 */
int
rpcb_service(rqstp, transp)
	register struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	union {
		RPCB rpcbproc_set_3_arg;
		RPCB rpcbproc_unset_3_arg;
		RPCB rpcbproc_getaddr_3_arg;
		struct rpcb_rmtcallargs rpcbproc_callit_3_arg;
		char *rpcbproc_uaddr2taddr_3_arg;
		struct netbuf rpcbproc_taddr2uaddr_3_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	if (debugging)
		printf("rpcbind: request for proc %d\n", rqstp->rq_proc);
	switch (rqstp->rq_proc) {
	case RPCBPROC_NULL:
		/*
		 * Null proc call
		 */
#ifdef DEBUG
		printf("RPCBPROC_NULL\n");
#endif
		(void)svc_sendreply(transp, xdr_void, (char *)NULL);
		return;

	case RPCBPROC_SET:
#ifdef DEBUG
		printf("RPCBPROC_SET\n");
#endif
		/*
		 * Check to see whether the message came from
		 * loopback transports (for security reasons)
		 */
		if (strcasecmp(transp->xp_netid, loopback_dg) &&
			strcasecmp(transp->xp_netid, loopback_vc) &&
			strcasecmp(transp->xp_netid, loopback_vc_ord)) {
			syslog(LOG_ERR, "non-local attempt to set");
			svcerr_weakauth(transp);
			return;
		}
		xdr_argument = xdr_rpcb;
		xdr_result = xdr_bool;
		local = (char *(*)()) rpcbproc_set_3;
		break;

	case RPCBPROC_UNSET:
#ifdef DEBUG
		printf("RPCBPROC_UNSET\n");
#endif
		/*
		 * Check to see whether the message came from
		 * loopback transports (for security reasons)
		 */
		if (strcasecmp(transp->xp_netid, loopback_dg) &&
			strcasecmp(transp->xp_netid, loopback_vc) &&
			strcasecmp(transp->xp_netid, loopback_vc_ord)) {
			syslog(LOG_ERR, "non-local attempt to unset");
			svcerr_weakauth(transp);
			return;
		}
		xdr_argument = xdr_rpcb;
		xdr_result = xdr_bool;
		local = (char *(*)()) rpcbproc_unset_3;
		break;

	case RPCBPROC_GETADDR:
#ifdef DEBUG
		printf("RPCBPROC_GETADDR\n");
#endif
		xdr_argument = xdr_rpcb;
		xdr_result = xdr_wrapstring;
		local = (char *(*)()) rpcbproc_getaddr_3;
		break;

	case RPCBPROC_DUMP:
#ifdef DEBUG
		printf("RPCBPROC_DUMP\n");
#endif
		xdr_argument = xdr_void;
		xdr_result = xdr_rpcblist;
		local = (char *(*)()) rpcbproc_dump_3;
		break;

	case RPCBPROC_CALLIT:
#ifdef DEBUG
		printf("RPCBPROC_CALLIT\n");
#endif
		callit(rqstp, transp);
		return;

	case RPCBPROC_GETTIME:
#ifdef DEBUG
		printf("RPCBPROC_GETTIME\n");
#endif
		xdr_argument = xdr_void;
		xdr_result = xdr_u_long;
		local = (char *(*)()) rpcbproc_gettime_3;
		break;

	case RPCBPROC_UADDR2TADDR:
#ifdef DEBUG
		printf("RPCBPROC_UADDR2TADDR\n");
#endif
		xdr_argument = xdr_wrapstring;
		xdr_result = xdr_netbuf;
		local = (char *(*)()) rpcbproc_uaddr2taddr_3;
		break;

	case RPCBPROC_TADDR2UADDR:
#ifdef DEBUG
		printf("RPCBPROC_TADDR2UADDR\n");
#endif
		xdr_argument = xdr_netbuf;
		xdr_result = xdr_wrapstring;
		local = (char *(*)()) rpcbproc_taddr2uaddr_3;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	memset((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		if (debugging)
			(void) fprintf(stderr, "rpcbind: could not decode\n");
		return;
	}
	result = (*local)(&argument, rqstp, transp);
	if (result != NULL && !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
		if (debugging) {
			(void) fprintf(stderr, "rpcbind: svc_sendreply\n");
			abort();
		}
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		if (debugging) {
			(void) fprintf(stderr, "unable to free arguments\n");
			abort();
		}
	}
	return;
}

/*
 * Set a mapping of program, version, netid
 */
static bool_t *
rpcbproc_set_3(regp, rqstp, transp)
	RPCB *regp;
	struct svc_req *rqstp;	/* Not used here */
	SVCXPRT *transp;
{
	RPCB reg;
	static int ans;
	RPCBLIST *rbl, *fnd;

	reg = *regp;
	/*
	 * check to see if already used
	 * find_service returns a hit even if
	 * the versions don't match, so check for it
	 */
	fnd = find_service(reg.r_prog, reg.r_vers, reg.r_netid);
	if (fnd && (fnd->rpcb_map.r_vers == reg.r_vers)) {
		if (!strcmp(fnd->rpcb_map.r_addr, reg.r_addr))
			/*
			 * if these match then it is already
			 * registered so just say "OK".
			 */
			ans = 1;
		else
			ans = 0;
		return (&ans);
	} else {
		/*
		 * add to END of list
		 */
		RPCB *a;

		rbl = (RPCBLIST *) malloc((u_int)sizeof (RPCBLIST));
		if (rbl == (RPCBLIST *)NULL) {
			ans = 0;
			return (&ans);
		}
		a = &(rbl->rpcb_map);
		a->r_prog = reg.r_prog;
		a->r_vers = reg.r_vers;
		a->r_netid = strdup(reg.r_netid);
		a->r_addr = strdup(reg.r_addr);
		a->r_owner = getowner(transp);
		if (!a->r_addr || !a->r_netid || !a->r_owner) {
			free((char *)rbl);
			ans = 0;
			return (&ans);
		}
		rbl->rpcb_next = (RPCBLIST *)NULL;
		if (list_rbl == NULL) {
			list_rbl = rbl;
		} else {
			for (fnd= list_rbl; fnd->rpcb_next;
				fnd = fnd->rpcb_next);
			fnd->rpcb_next = rbl;
		}
#ifdef PORTMAP
		(void) add_pmaplist(regp);
#endif
		ans = 1;
		return (&ans);
	}
}

/*
 * Unset a mapping of program, version, netid
 */
static bool_t *
rpcbproc_unset_3(regp, rqstp, transp)
	RPCB *regp;
	struct svc_req *rqstp;	/* Not used here */
	SVCXPRT *transp;
{
	static int ans;
	char *owner;

	ans = 0;
	owner = getowner(transp);
	if (owner == NULL)
		return (&ans);
	ans = unset(regp, owner);
	free (owner);
	return (&ans);
}

static bool_t
unset(regp, owner)
	RPCB *regp;
	char *owner;
{
	int ans = 0;
	RPCBLIST *rbl, *prev, *tmp;

	if (owner == NULL)
		return (0);

	for (prev = NULL, rbl = list_rbl; rbl; ) {
		if ((rbl->rpcb_map.r_prog != regp->r_prog) ||
			(rbl->rpcb_map.r_vers != regp->r_vers) ||
			(regp->r_netid[0] && strcasecmp(regp->r_netid,
				rbl->rpcb_map.r_netid))) {
			/* both rbl & prev move forwards */
			prev = rbl;
			rbl = rbl->rpcb_next;
			continue;
		}
		/*
		 * Check whether appropriate uid. Unset only
		 * if superuser or the owner itself.
		 */
		if (strcmp(owner, "superuser") &&
			strcmp(rbl->rpcb_map.r_owner, owner))
			return (0);
		/* found it; rbl moves forward, prev stays */
		ans = 1;
		tmp = rbl;
		rbl = rbl->rpcb_next;
		if (prev == NULL)
			list_rbl = rbl;
		else
			prev->rpcb_next = rbl;
		free(tmp->rpcb_map.r_addr);
		free(tmp->rpcb_map.r_netid);
		free(tmp->rpcb_map.r_owner);
		free((char *)tmp);
	}
#ifdef PORTMAP
	if (ans)
		(void) del_pmaplist(regp);
#endif
	return (ans);
}

/*
 * Lookup the mapping for a program, version and return its
 * address. Assuming that the caller wants the address of the
 * server running on the transport on which the request came.
 *
 * We also try to resolve the universal address in terms of
 * address of the caller.
 */
static char **
rpcbproc_getaddr_3(regp, rqstp, transp)
	RPCB *regp;
	struct svc_req *rqstp;	/* Not used here */
	SVCXPRT *transp;
{
	static char *uaddr;
	RPCBLIST *fnd;

	if (uaddr && uaddr[0])
		(void) free(uaddr);
	fnd = find_service(regp->r_prog, regp->r_vers, transp->xp_netid);
	if (fnd) {
		if (!(uaddr = mergeaddr(transp, fnd->rpcb_map.r_addr))) {
			/* Try whatever we have */
			uaddr = fnd->rpcb_map.r_addr;
		} else if (!uaddr[0]) {
			/* the server died. Unset this combination */
			uaddr = nullstring;
			(void) unset(regp, "superuser");
		}
	} else {
		uaddr = nullstring;
	}
#ifdef DEBUG
	printf("getaddr: %s\n", uaddr);
#endif
	return (&uaddr);
}

/* VARARGS */
static RPCBLIST **
rpcbproc_dump_3()
{
	return (&list_rbl);
}

/* VARARGS */
static u_long *
rpcbproc_gettime_3()
{
	static time_t curtime;

	time(&curtime);
	return ((u_long *)&curtime);
}

/*
 * Convert uaddr to taddr. Should be used only by
 * local servers/clients. (kernel level stuff only)
 */
static struct netbuf *
rpcbproc_uaddr2taddr_3(uaddrp, rqstp, transp)
	char **uaddrp;
	struct svc_req *rqstp;	/* Not used here */
	SVCXPRT *transp;
{
	struct netconfig *nconf;
	static struct netbuf nbuf;
	static struct netbuf *taddr;

	if (!(nconf = getnetconfigent(transp->xp_netid))) {
		memset((char *)&nbuf, 0, sizeof (struct netbuf));
		return (&nbuf);
	}
	if (taddr) {
		free(taddr->buf);
		free((char *)taddr);
	}
	taddr = uaddr2taddr(nconf, *uaddrp);
	freenetconfigent(nconf);
	if (taddr == NULL) {
		memset((char *)&nbuf, 0, sizeof (struct netbuf));
		return (&nbuf);
	}
	return (taddr);
}

/*
 * Convert taddr to uaddr. Should be used only by
 * local servers/clients. (kernel level stuff only)
 */
static char **
rpcbproc_taddr2uaddr_3(taddr, rqstp, transp)
	struct netbuf *taddr;
	struct svc_req *rqstp;	/* Not used here */
	SVCXPRT *transp;
{
	static char *uaddr;
	struct netconfig *nconf;

	if (uaddr && !uaddr[0])
		(void) free(uaddr);
	if (!(nconf = getnetconfigent(transp->xp_netid))) {
		uaddr = nullstring;
		return (&uaddr);
	}
	if (!(uaddr = taddr2uaddr(nconf, taddr)))
		uaddr = nullstring;
	freenetconfigent(nconf);
	return (&uaddr);
}

/*
 * Stuff for the rmtcall service
 */
/*
 * XXX: This size stuff has to be fixed, because it pertains only to udp
 * We want it transport independent.
 */
#define	ARGSIZE 9000

struct encap_parms {
	u_long arglen;
	char *args;
};

static bool_t
xdr_encap_parms(xdrs, epp)
	XDR *xdrs;
	struct encap_parms *epp;
{
	return (xdr_bytes(xdrs, &(epp->args), &(epp->arglen), ARGSIZE));
}

struct rmtcall_args {
	u_long 	rmt_prog;
	u_long 	rmt_vers;
	u_long 	rmt_proc;
	char 	*rmt_uaddr;
	struct encap_parms rmt_args;
};

/*
 * XDR remote call arguments
 * written for XDR_DECODE direction only
 */
static bool_t
xdr_rmtcall_args(xdrs, cap)
	register XDR *xdrs;
	register struct rmtcall_args *cap;
{
	/* does not get the address */
	if (xdr_u_long(xdrs, &(cap->rmt_prog)) &&
	    xdr_u_long(xdrs, &(cap->rmt_vers)) &&
	    xdr_u_long(xdrs, &(cap->rmt_proc))) {
		return (xdr_encap_parms(xdrs, &(cap->rmt_args)));
	}
	return (FALSE);
}

static bool_t
xdr_rmtcall_result(xdrs, cap)
	register XDR *xdrs;
	register struct rmtcall_args *cap;
{
	if (xdr_wrapstring(xdrs, &(cap->rmt_uaddr)))
		return (xdr_encap_parms(xdrs, &(cap->rmt_args)));
	return (FALSE);
}

/*
 * only worries about the struct encap_parms part of struct rmtcall_args.
 * The arglen must already be set!!
 */
static bool_t
xdr_opaque_parms(xdrs, cap)
	XDR *xdrs;
	struct rmtcall_args *cap;
{
	return (xdr_opaque(xdrs, cap->rmt_args.args, cap->rmt_args.arglen));
}

/*
 * This routine finds and sets the length of incoming opaque paraters
 * and then calls xdr_opaque_parms.
 */
static bool_t
xdr_len_opaque_parms(xdrs, cap)
	register XDR *xdrs;
	struct rmtcall_args *cap;
{
	register u_int beginpos, lowpos, highpos, currpos, pos;

	beginpos = lowpos = pos = xdr_getpos(xdrs);
	highpos = lowpos + ARGSIZE;
	while ((int)(highpos - lowpos) >= 0) {
		currpos = (lowpos + highpos) / 2;
		if (xdr_setpos(xdrs, currpos)) {
			/* Within the range */
			pos = currpos;
			lowpos = currpos + 1;
		} else {
			/* Out of the range */
			highpos = currpos - 1;
		}
	}
	xdr_setpos(xdrs, beginpos);
	cap->rmt_args.arglen = pos - beginpos;
	return (xdr_opaque_parms(xdrs, cap));
}

/*
 * Call a remote procedure service
 * This procedure is very quiet when things go wrong.
 * The proc is written to support broadcast rpc. In the broadcast case,
 * a machine should shut-up instead of complain, less the requestor be
 * overrun with complaints at the expense of not hearing a valid reply ...
 */
static int
callit(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	register RPCBLIST *rbl;
	CLIENT *client;
	struct netconfig *nconf;
	struct rmtcall_args a;
	pid_t pid;
	int fd = RPC_ANYFD;
	struct authsys_parms *au = (struct authsys_parms *)rqstp->rq_clntcred;
	struct timeval timeout;
	char buf[ARGSIZE];	/* XXX: A self imposed upper limit. */
	struct netbuf *na;


	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	a.rmt_args.args = buf;

	if (!svc_getargs(transp, xdr_rmtcall_args, &a))
		return;

	/* Disallow calling remote rpcbind so user may not unset an rpc services */
	if (a.rmt_prog == RPCBPROG)
		return;
#ifdef DEBUG
	printf("rmtcall for %d %d %d on %s\n", a.rmt_prog, a.rmt_vers,
				a.rmt_proc, transp->xp_netid);
#endif
	rbl = find_service(a.rmt_prog, a.rmt_vers, transp->xp_netid);
	if (rbl == (RPCBLIST *)NULL)
		return;
	/*
	 * fork a child to do the work. Parent immediately returns.
	 * Child exits upon completion.
	 */
	if ((pid = fork()) != 0) {
		if (debugging && (pid < (pid_t)0)) {
			fprintf(stderr, "rpcbind CALLIT: cannot fork.\n");
		}
		return;
	}

	nconf = getnetconfigent(transp->xp_netid);
	if (nconf == (struct netconfig *)NULL)
		exit(1);
	na = uaddr2taddr(nconf, rbl->rpcb_map.r_addr);
#ifdef ND_DEBUG
	fprintf(stderr, "\tRemote address is [%s].\n", rbl->rpcb_map.r_addr);
	if (!na)
		fprintf(stderr, "\tCouldn't resolve remote address!\n");
#endif
	client = clnt_tli_create(fd, nconf, na, a.rmt_prog,
				a.rmt_vers, 0, 0);
	if (client != (CLIENT *)NULL) {
		if (rqstp->rq_cred.oa_flavor == AUTH_UNIX) {
			client->cl_auth = authsys_create(au->aup_machname,
						au->aup_uid, au->aup_gid,
						au->aup_len, au->aup_gids);
		}
		if (clnt_call(client, a.rmt_proc, xdr_opaque_parms,
				&a, xdr_len_opaque_parms,
				&a, timeout) == RPC_SUCCESS) {
			svc_sendreply(transp, xdr_rmtcall_result, &a);
		} /* Shut up if it fails */
		CLNT_DESTROY(client);
	}
	exit(0);
}

/*
 * returns the item with the given program, version number and netid.
 * If that version number is not found, it returns the item with that
 * program number, so that address is now returned to the caller. The
 * caller when makes a call to this program, version number, the call
 * will fail and it will return with PROGVERS_MISMATCH. The user can
 * then determine the highest and the lowest version number for this
 * program using clnt_geterr() and use those program version numbers.
 *
 * Returns the rpcblist for the given prog, vers and netid
 */
static RPCBLIST *
find_service(prog, vers, netid)
	u_long prog;	/* Program Number */
	u_long vers;	/* Version Number */
	char *netid;	/* Transport Provider token */
{
	register RPCBLIST *hit = NULL;
	register RPCBLIST *rbl;

	for (rbl = list_rbl; rbl != NULL; rbl = rbl->rpcb_next) {
		if ((rbl->rpcb_map.r_prog != prog) ||
		    ((rbl->rpcb_map.r_netid != NULL) &&
			(strcasecmp(rbl->rpcb_map.r_netid, netid) != 0)))
			continue;
		hit = rbl;
		if (rbl->rpcb_map.r_vers == vers)
			break;
	}
	return (hit);
}

/*
 * Returns allocated string names for the uid associated
 * with the process.
 */
static char *
getowner(transp)
	SVCXPRT *transp;
{
	uid_t uid;
	char uidname[128];

	if (_rpc_get_local_uid(transp, &uid) < 0)
		return (strdup("unknown"));
	if (uid == 0)
		return (strdup("superuser"));
	(void) sprintf(uidname, "%d", uid);
	return (strdup(uidname));
}


#ifdef PORTMAP
/*
 * Add this to the pmap list only if it is UDP or TCP.
 */
static int
add_pmaplist(arg)
	RPCB *arg;
{
	PMAP pmap;
	PMAPLIST *pml;
	int h1, h2, h3, h4, p1, p2;

	if (strcmp(arg->r_netid, udptrans) == 0) {
		/* It is UDP! */
		pmap.pm_prot = IPPROTO_UDP;
	} else if (strcmp(arg->r_netid, tcptrans) == 0) {
		/* It is TCP */
		pmap.pm_prot = IPPROTO_TCP;
	} else
		/* Not a IP protocol */
		return (0);

	/* interpret the universal address for TCP/IP */
	sscanf(arg->r_addr, "%d.%d.%d.%d.%d.%d", &h1, &h2, &h3, &h4, &p1, &p2);
	pmap.pm_port = ((p1 & 0xff) << 8) + (p2 & 0xff);
	pmap.pm_prog = arg->r_prog;
	pmap.pm_vers = arg->r_vers;
	/*
	 * add to END of list
	 */
	pml = (PMAPLIST *) malloc((u_int)sizeof (PMAPLIST));
	if (pml == NULL) {
		(void) syslog(LOG_ERR, "rpcbind: no memory!\n");
		return (1);
	}
	pml->pml_map = pmap;
	pml->pml_next = NULL;
	if (list_pml == NULL) {
		list_pml = pml;
	} else {
		PMAPLIST *fnd;

		/* Attach to the end of the list */
		for (fnd = list_pml; fnd->pml_next; fnd = fnd->pml_next);
		fnd->pml_next = pml;
	}
	return (0);
}

/*
 * Delete this from the pmap list only if it is UDP or TCP.
 */
static int
del_pmaplist(arg)
	RPCB *arg;
{
	register PMAPLIST *pml;
	PMAPLIST *prevpml, *fnd;
	long prot;

	if (strcmp(arg->r_netid, udptrans) == 0) {
		/* It is UDP! */
		prot = IPPROTO_UDP;
	} else if (strcmp(arg->r_netid, tcptrans) == 0) {
		/* It is TCP */
		prot = IPPROTO_TCP;
	} else if (arg->r_netid[0] == NULL) {
		prot = 0;	/* Remove all occurrences */
	} else
		/* Not a IP protocol */
		return (0);
	for (prevpml = NULL, pml = list_pml; pml; ) {
		if ((pml->pml_map.pm_prog != arg->r_prog) ||
			(pml->pml_map.pm_vers != arg->r_vers) ||
			(prot && (pml->pml_map.pm_prot != prot))) {
			/* both pml & prevpml move forwards */
			prevpml = pml;
			pml = pml->pml_next;
			continue;
		}
		/* found it; pml moves forward, prevpml stays */
		fnd = pml;
		pml = pml->pml_next;
		if (prevpml == NULL)
			list_pml = pml;
		else
			prevpml->pml_next = pml;
		free((char *)fnd);
	}
	return (0);
}
#endif /* PORTMAP */
