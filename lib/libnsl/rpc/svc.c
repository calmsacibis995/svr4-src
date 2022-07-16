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


#ident	"@(#)librpc:svc.c	1.4.2.1"

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
static char sccsid[] = "@(#)svc.c 1.58 89/03/16 Copyr 1988 Sun Micro";
#endif

/*
 * svc.c, Server-side remote procedure call interface.
 *
 * There are two sets of procedures here.  The xprt routines are
 * for handling transport handles.  The svc routines handle the
 * list of service routines.
 *
 */

#include <sys/errno.h>
#include <rpc/rpc.h>
#ifdef PORTMAP
#include <rpc/pmap_clnt.h>
#endif
#include <netconfig.h>

extern int errno;
static SVCXPRT **xports;

#define NULL_SVC ((struct svc_callout *)0)
#define	RQCRED_SIZE	400		/* this size is excessive */

#define SVC_VERSQUIET 0x0001		/* keep quiet about vers mismatch */
#define version_keepquiet(xp)  ((u_long)(xp)->xp_p3 & SVC_VERSQUIET)

/*
 * The services list
 * Each entry represents a set of procedures (an rpc program).
 * The dispatch routine takes request structs and runs the
 * appropriate procedure.
 */
static struct svc_callout {
	struct svc_callout *sc_next;
	u_long		    sc_prog;
	u_long		    sc_vers;
	void		    (*sc_dispatch)();
} *svc_head;

static struct svc_callout *svc_find();

/* ***************  SVCXPRT related stuff **************** */

/*
 * Activate a transport handle.
 */
void
xprt_register(xprt)
	SVCXPRT *xprt;
{
	register int fd = xprt->xp_fd;
	void svc_getreqset();
	extern void (*_svc_getreqset_proc)();

	if (xports == NULL) {
		xports = (SVCXPRT **)
			mem_alloc(FD_SETSIZE * sizeof(SVCXPRT *));
#ifdef RENTER
		_svc_getreqset_proc = svc_getreqset;
#endif
	}
	if (fd < _rpc_dtbsize()) {
		xports[fd] = xprt;
		FD_SET(fd, &svc_fdset);
	}
}

/*
 * De-activate a transport handle. 
 */
void
xprt_unregister(xprt) 
	SVCXPRT *xprt;
{ 
	register int fd = xprt->xp_fd;

	if ((fd < _rpc_dtbsize()) && (xports[fd] == xprt)) {
		xports[fd] = (SVCXPRT *)NULL;
		FD_CLR(fd, &svc_fdset);
	}
} 


/* ********************** CALLOUT list related stuff ************* */

/*
 * Add a service program to the callout list.
 * The dispatch routine will be called when a rpc request for this
 * program number comes in.
 */
bool_t
svc_reg(xprt, prog, vers, dispatch, nconf)
	SVCXPRT *xprt;
	u_long prog;
	u_long vers;
	void (*dispatch)();
	struct netconfig *nconf;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

	if ((s = svc_find(prog, vers, &prev)) != NULL_SVC) {
		if (s->sc_dispatch == dispatch)
			goto rpcb_it;  /* he is registering another xptr */
		return (FALSE);
	}
	s = (struct svc_callout *)mem_alloc(sizeof(struct svc_callout));
	if (s == (struct svc_callout *)NULL) {
		return (FALSE);
	}
	s->sc_prog = prog;
	s->sc_vers = vers;
	s->sc_dispatch = dispatch;
	s->sc_next = svc_head;
	svc_head = s;
rpcb_it:
	/* now register the information with the local binder service */
	if (nconf) {
		return (rpcb_set(prog, vers, nconf, &xprt->xp_ltaddr));
	}
	return (TRUE);
}

/*
 * Remove a service program from the callout list.
 */
void
svc_unreg(prog, vers)
	u_long prog;
	u_long vers;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

	/* unregister the information anyway */
	(void) rpcb_unset(prog, vers, NULL);
	if ((s = svc_find(prog, vers, &prev)) == NULL_SVC) {
		return;
	}
	if (prev == NULL_SVC) {
		svc_head = s->sc_next;
	} else {
		prev->sc_next = s->sc_next;
	}
	s->sc_next = NULL_SVC;
	(void) mem_free((char *) s, (u_int) sizeof(struct svc_callout));
}

#ifdef PORTMAP
/*
 * Add a service program to the callout list.
 * The dispatch routine will be called when a rpc request for this
 * program number comes in.
 * For version 2 portmappers.
 */
#ifdef KERNEL 
/*ARGSUSED*/ 
#endif
bool_t
svc_register(xprt, prog, vers, dispatch, protocol)
	SVCXPRT *xprt;
	u_long prog;
	u_long vers;
	void (*dispatch)();
	int protocol;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

	if ((s = svc_find(prog, vers, &prev)) != NULL_SVC) {
		if (s->sc_dispatch == dispatch)
			goto pmap_it;  /* he is registering another xptr */
		return (FALSE);
	}
	s = (struct svc_callout *)mem_alloc(sizeof(struct svc_callout));
#ifndef KERNEL
	if (s == (struct svc_callout *)0) {
		return (FALSE);
	}
#endif
	s->sc_prog = prog;
	s->sc_vers = vers;
	s->sc_dispatch = dispatch;
	s->sc_next = svc_head;
	svc_head = s;
pmap_it:
#ifndef KERNEL
	/* now register the information with the local binder service */
	if (protocol) {
		return (pmap_set(prog, vers, protocol, xprt->xp_port));
	}
#endif
	return (TRUE);
}

/*
 * Remove a service program from the callout list.
 * For version 2 portmappers.
 */
void
svc_unregister(prog, vers)
	u_long prog;
	u_long vers;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

	if ((s = svc_find(prog, vers, &prev)) == NULL_SVC)
		return;
	if (prev == NULL_SVC) {
		svc_head = s->sc_next;
	} else {
		prev->sc_next = s->sc_next;
	}
	s->sc_next = NULL_SVC;
	mem_free((char *) s, (u_int) sizeof(struct svc_callout));
#ifndef KERNEL
	/* now unregister the information with the local binder service */
	(void)pmap_unset(prog, vers);
#endif
}

#endif /* PORTMAP */
/*
 * Search the callout list for a program number, return the callout
 * struct.
 */
static struct svc_callout *
svc_find(prog, vers, prev)
	u_long prog;
	u_long vers;
	struct svc_callout **prev;
{
	register struct svc_callout *s, *p;

	p = NULL_SVC;
	for (s = svc_head; s != NULL_SVC; s = s->sc_next) {
		if ((s->sc_prog == prog) && (s->sc_vers == vers))
			goto done;
		p = s;
	}
done:
	*prev = p;
	return (s);
}

/* ******************* REPLY GENERATION ROUTINES  ************ */

/*
 * Send a reply to an rpc request
 */
bool_t
svc_sendreply(xprt, xdr_results, xdr_location)
	register SVCXPRT *xprt;
	xdrproc_t xdr_results;
	caddr_t xdr_location;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY;  
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf; 
	rply.acpted_rply.ar_stat = SUCCESS;
	rply.acpted_rply.ar_results.where = xdr_location;
	rply.acpted_rply.ar_results.proc = xdr_results;
	return (SVC_REPLY(xprt, &rply)); 
}

/*
 * No procedure error reply
 */
void
svcerr_noproc(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_ACCEPTED;
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = PROC_UNAVAIL;
	SVC_REPLY(xprt, &rply);
}

/*
 * Can't decode args error reply
 */
void
svcerr_decode(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY; 
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = GARBAGE_ARGS;
	SVC_REPLY(xprt, &rply); 
}

/*
 * Some system error
 */
void
svcerr_systemerr(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY; 
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = SYSTEM_ERR;
	SVC_REPLY(xprt, &rply); 
}

/*
 * Tell RPC package to not complain about version errors to the client.  This
 * is useful when revving broadcast protocols that sit on a fixed address.
 * There is really one (or should be only one) example of this kind of
 * protocol: the portmapper (or rpc binder).
 */
void
svc_versquiet(xprt)
	register SVCXPRT *xprt;
{
	u_long tmp;

	tmp = ((u_long)xprt->xp_p3) | SVC_VERSQUIET;
	xprt->xp_p3 = (caddr_t)tmp;
}

/*
 * Authentication error reply
 */
void
svcerr_auth(xprt, why)
	SVCXPRT *xprt;
	enum auth_stat why;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_DENIED;
	rply.rjcted_rply.rj_stat = AUTH_ERROR;
	rply.rjcted_rply.rj_why = why;
	SVC_REPLY(xprt, &rply);
}

/*
 * Auth too weak error reply
 */
void
svcerr_weakauth(xprt)
	SVCXPRT *xprt;
{
	svcerr_auth(xprt, AUTH_TOOWEAK);
}

/*
 * Program unavailable error reply
 */
void 
svcerr_noprog(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply;  

	rply.rm_direction = REPLY;   
	rply.rm_reply.rp_stat = MSG_ACCEPTED;  
	rply.acpted_rply.ar_verf = xprt->xp_verf;  
	rply.acpted_rply.ar_stat = PROG_UNAVAIL;
	SVC_REPLY(xprt, &rply);
}

/*
 * Program version mismatch error reply
 */
void  
svcerr_progvers(xprt, low_vers, high_vers)
	register SVCXPRT *xprt; 
	u_long low_vers;
	u_long high_vers;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_ACCEPTED;
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = PROG_MISMATCH;
	rply.acpted_rply.ar_vers.low = low_vers;
	rply.acpted_rply.ar_vers.high = high_vers;
	SVC_REPLY(xprt, &rply);
}

/* ******************* SERVER INPUT STUFF ******************* */

/*
 * Get server side input from some transport.
 *
 * Statement of authentication parameters management:
 * This function owns and manages all authentication parameters, specifically
 * the "raw" parameters (msg.rm_call.cb_cred and msg.rm_call.cb_verf) and
 * the "cooked" credentials (rqst->rq_clntcred).
 * However, this function does not know the structure of the cooked 
 * credentials, so it make the following assumptions: 
 *   a) the structure is contiguous (no pointers), and
 *   b) the cred structure size does not exceed RQCRED_SIZE bytes. 
 * In all events, all three parameters are freed upon exit from this routine.
 * The storage is trivially management on the call stack in user land, but
 * is mallocated in kernel land.
 */

void
svc_getreq(rdfds)
	int rdfds;
{
	fd_set readfds;

	FD_ZERO(&readfds);
	readfds.fds_bits[0] = rdfds;
	svc_getreqset(&readfds);
}

void
svc_getreqset(readfds)
	fd_set *readfds;
{
	enum xprt_stat stat;
	struct rpc_msg msg;
	int prog_found;
	u_long low_vers;
	u_long high_vers;
	struct svc_req r;
	register SVCXPRT *xprt;
	register u_long mask;
	register int bit;
	register u_long *maskp;
	register int setsize;
	register int fd;
	char cred_area[2 * MAX_AUTH_BYTES + RQCRED_SIZE];

	msg.rm_call.cb_cred.oa_base = cred_area;
	msg.rm_call.cb_verf.oa_base = &(cred_area[MAX_AUTH_BYTES]);
	r.rq_clntcred = &(cred_area[2 * MAX_AUTH_BYTES]);

	setsize = _rpc_dtbsize();	
	maskp = (u_long *)readfds->fds_bits;
	for (fd = 0; fd < setsize; fd += NFDBITS) {
	    for (mask = *maskp++; bit = ffs(mask); mask ^= (1 << (bit - 1))) {
		/* fd has input waiting */
		xprt = xports[fd + bit - 1];
		/* now receive msgs from xprtprt (support batch calls) */
		do {
			if (SVC_RECV(xprt, &msg)) {

				/* now find the exported program and call it */
				register struct svc_callout *s;
				enum auth_stat why;

				r.rq_xprt = xprt;
				r.rq_prog = msg.rm_call.cb_prog;
				r.rq_vers = msg.rm_call.cb_vers;
				r.rq_proc = msg.rm_call.cb_proc;
				r.rq_cred = msg.rm_call.cb_cred;
				/* first authenticate the message */
				if ((why= _authenticate(&r, &msg)) != AUTH_OK) {
					svcerr_auth(xprt, why);
					goto call_done;
				}
				/* now match message with a registered service*/
				prog_found = FALSE;
				low_vers = 0 - 1;
				high_vers = 0;
				for (s = svc_head; s != NULL_SVC; s = s->sc_next) {
					if (s->sc_prog == r.rq_prog) {
						if (s->sc_vers == r.rq_vers) {
							(*s->sc_dispatch)(&r, xprt);
							goto call_done;
						}  /* found correct version */
						prog_found = TRUE;
						if (s->sc_vers < low_vers)
							low_vers = s->sc_vers;
						if (s->sc_vers > high_vers)
							high_vers = s->sc_vers;
					}   /* found correct program */
				}
				/*
				 * if we got here, the program or version
				 * is not served ...
				 */
				if (prog_found && !version_keepquiet(xprt))
					svcerr_progvers(xprt,
					low_vers, high_vers);
				else
					 svcerr_noprog(xprt);
				/* Fall through to ... */
			}
		call_done:
			if ((stat = SVC_STAT(xprt)) == XPRT_DIED){
				SVC_DESTROY(xprt);
				break;
			}
		} while (stat == XPRT_MOREREQS);
	    }
	}
}
