/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-klm:klm_lkmgr.c	1.3.1.1"
#ifndef lint
static char sccsid[] = "@(#)klm_lkmgr.c 1.10 89/08/22 SMI";
#endif
/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

        /*
         * Copyright (c) 1989 by Sun Microsystems, Inc.
         */

/*
 * Kernel<->Network Lock-Manager Interface
 *
 * File- and Record-locking requests are forwarded (via RPC) to a
 * Network Lock-Manager running on the local machine.  The protocol
 * for these transactions is defined in /usr/src/protocols/klm_prot.x
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/cred.h>
#include <sys/socket.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/systm.h>
#include <sys/utsname.h>
#include <sys/cmn_err.h>
#include <sys/debug.h>

/* files included by <rpc/rpc.h> */
#include <rpc/types.h>
#include <netinet/in.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc.h>

#include <klm/lockmgr.h>
#include "klm_prot.h"
#include <nfs/nfs.h>
#include <nfs/nfs_clnt.h>
#include <nfs/rnode.h>

#define	NC_LOOPBACK		"loopback"	/* XXX */

static struct knetconfig	config;		/* avoid loopupname next time */

STATIC int			talk_to_lockmgr();

/* Define static parameters for run-time tuning */

static int backoff_timeout = 30;	/* time to wait on klm_denied_nolocks */
static int first_retry = 1;		/* first attempt if klm port# known */
static int first_timeout = 6;
static int normal_retry = 1;		/* attempts after new port# obtained */
static int normal_timeout = 30;
static int working_retry = 3;		/* attempts after klm_working */
static int working_timeout = 10;


/*
 * klm_lockctl - process a lock/unlock/test-lock request
 *
 * Calls (via RPC) the local lock manager to register the request.
 * Lock requests are cancelled if interrupted by signals.
 */
klm_lockctl(lh, bfp, cmd, cred, clid)
	lockhandle_t *lh;
	struct flock *bfp;
	int cmd;
	struct cred *cred;
	pid_t clid;
{
	register int	error;
	char 		*args;
	klm_lockargs	klm_lockargs_args;
	klm_unlockargs  klm_unlockargs_args;
	klm_testargs    klm_testargs_args;
	klm_testrply	reply;
	u_long		xdrproc;
	xdrproc_t	xdrargs;
	xdrproc_t	xdrreply;
	int		timeid;

#ifdef LOCKDEBUG
	cmn_err(CE_CONT, "entering klm_lockctl() : cmd %d clid %d\n", cmd, clid);
#endif

	if (!bfp->l_pid) bfp->l_pid = clid; /* FIXME */

	switch (cmd) {
	case F_SETLK:
	case F_SETLKW:
		if (bfp->l_type != F_UNLCK) {
			if (cmd == F_SETLKW)
				klm_lockargs_args.block = TRUE;
			else
				klm_lockargs_args.block = FALSE;
			if (bfp->l_type == F_WRLCK) {
				klm_lockargs_args.exclusive = TRUE;
			} else {
				klm_lockargs_args.exclusive = FALSE;
			}
			klm_lockargs_args.alock.fh.n_bytes = (char *)&lh->lh_id;
			klm_lockargs_args.alock.fh.n_len = sizeof (lh->lh_id);
			klm_lockargs_args.alock.server_name = lh->lh_servername;
			klm_lockargs_args.alock.pid = clid;
			klm_lockargs_args.alock.base = bfp->l_start;
			klm_lockargs_args.alock.length = bfp->l_len;
			klm_lockargs_args.alock.rsys = bfp->l_sysid;
			args = (char *) &klm_lockargs_args;
			xdrproc = KLM_LOCK;
			xdrargs = (xdrproc_t)xdr_klm_lockargs;
			xdrreply = (xdrproc_t)xdr_klm_stat;
		} else {
			klm_unlockargs_args.alock.fh.n_bytes = (char *)&lh->lh_id;
			klm_unlockargs_args.alock.fh.n_len = sizeof (lh->lh_id);
			klm_unlockargs_args.alock.server_name = lh->lh_servername;
			klm_unlockargs_args.alock.pid = clid;
			klm_unlockargs_args.alock.base = bfp->l_start;
			klm_unlockargs_args.alock.length = bfp->l_len;
			klm_unlockargs_args.alock.rsys = bfp->l_sysid;
			args = (char *) &klm_unlockargs_args;
			xdrreply = (xdrproc_t)xdr_klm_stat;
			xdrproc = KLM_UNLOCK;
			xdrargs = (xdrproc_t)xdr_klm_unlockargs;
		}
		break;

	case F_GETLK:
		if (bfp->l_type == F_WRLCK) {
			klm_testargs_args.exclusive = TRUE;
		} else {
			klm_testargs_args.exclusive = FALSE;
		}
		klm_testargs_args.alock.fh.n_bytes = (char *)&lh->lh_id;
		klm_testargs_args.alock.fh.n_len = sizeof (lh->lh_id);
		klm_testargs_args.alock.server_name = lh->lh_servername;
		klm_testargs_args.alock.pid = clid;
		klm_testargs_args.alock.base = bfp->l_start;
		klm_testargs_args.alock.length = bfp->l_len;
		klm_testargs_args.alock.rsys = bfp->l_sysid;
		args = (char *) &klm_testargs_args;
		xdrproc = KLM_TEST;
		xdrargs = (xdrproc_t)xdr_klm_testargs;
		xdrreply = (xdrproc_t)xdr_klm_testrply;
		break;
	}

requestloop:
	/* send the request out to the local lock-manager and wait for reply */
	error = talk_to_lockmgr(xdrproc, xdrargs, args, xdrreply, &reply, cred);
	if (error == ENOLCK || error == RPC_UDERROR) {
		error = ENOLCK;
		goto ereturn;	/* no way the request could have gotten out */
	}

	/*
	 * The only other possible return values are:
	 *   klm_granted  |  klm_denied  | klm_denied_nolocks |  EINTR
	 */
	switch (xdrproc) {
	case KLM_LOCK:
		switch (error) {
		case klm_granted:
			error = 0;		/* got the requested lock */
			goto ereturn;
		case klm_denied:
			if (klm_lockargs_args.block) {
				cmn_err(CE_CONT,
					"klm_lockmgr: blocking lock denied?!\n");
				goto requestloop;	/* loop forever */
			}
			error = EACCES;		/* EAGAIN?? */
			goto ereturn;
		case klm_denied_nolocks:
			error = ENOLCK;		/* no resources available?! */
			goto ereturn;
		case klm_deadlck:
			error = EDEADLK;	/* deadlock condition */
			goto ereturn;
		case EINTR:
			goto cancel;
		}

	case KLM_UNLOCK:
		switch (error) {
		case klm_granted:
			error = 0;
			goto ereturn;
		case klm_denied:
		case EINTR:
#ifdef LOCKDEBUG
			cmn_err(CE_CONT, "klm_lockmgr: unlock denied?!\n");
#endif
			error = EINVAL;
			goto ereturn;
		case klm_denied_nolocks:
			goto nolocks_wait;	/* back off; loop forever */
#ifdef NOTUSE
		case EINTR:
			return(EINTR);
#endif
		}

	case KLM_TEST:
		switch (error) {
		case klm_granted:
			bfp->l_type = F_UNLCK;	/* mark lock available */
			error = 0;
#ifdef LOCKDEBUG
			cmn_err(CE_CONT,
				"KLM_GRANTED : pid=%d start=%d len=%d\n",
				bfp->l_pid,bfp->l_start,bfp->l_len);
#endif
			goto ereturn;
		case klm_denied:
			bfp->l_type = (reply.klm_testrply_u.holder.exclusive) ?
			    F_WRLCK : F_RDLCK;
			bfp->l_start = reply.klm_testrply_u.holder.base;
			bfp->l_len = reply.klm_testrply_u.holder.length;
			bfp->l_pid = reply.klm_testrply_u.holder.pid;
			bfp->l_sysid = reply.klm_testrply_u.holder.rsys;
#ifdef LOCKDEBUG
			cmn_err(CE_CONT,
				"KLM_DENIED : pid=%d start=%d len=%d\n", 
                                bfp->l_pid,bfp->l_start,bfp->l_len);
#endif
			error = 0;
			goto ereturn;
		case klm_denied_nolocks:
			goto nolocks_wait;	/* back off; loop forever */
		case EINTR:
			/* may want to take a longjmp here */
			goto ereturn;	/* quit */
		}
	}

/* NOTREACHED */
nolocks_wait:
	timeid = timeout(wakeup, (caddr_t)&config, (backoff_timeout * HZ));
	(void) sleep((caddr_t)&config, PZERO|PCATCH);
	untimeout(timeid);
	goto requestloop;	/* now try again */

cancel:
	/*
	 * If we get here, a signal interrupted a rqst that must be cancelled.
	 * Change the procedure number to KLM_CANCEL and reissue the exact same
	 * request.  Use the results to decide what return value to give.
	 */
	xdrproc = KLM_CANCEL;
	error = talk_to_lockmgr(xdrproc, xdrargs, args, xdrreply, &reply, cred);
	switch (error) {
	case klm_granted:
		error = 0;		/* lock granted */
		goto ereturn;
	case klm_denied:
		/* may want to take a longjmp here */
		error = EINTR;
		goto ereturn;
	case klm_deadlck:
		error = EDEADLK;
		goto ereturn;
	case EINTR:
		goto cancel;		/* ignore signals til cancel succeeds */

	case klm_denied_nolocks:
		error = ENOLCK;		/* no resources available?! */
		goto ereturn;
	case ENOLCK:
		cmn_err(CE_CONT, "klm_lockctl: ENOLCK on KLM_CANCEL request\n");
		goto ereturn;
	}
/* NOTREACHED */
ereturn:
	return (error);
}


/*
 * Send the given request to the local lock-manager.
 * If timeout or error, go back to the portmapper to check the port number.
 * This routine loops forever until one of the following occurs:
 *	1) A legitimate (not 'klm_working') reply is returned (returns 'stat').
 *
 *	2) A signal occurs (returns EINTR).  In this case, at least one try
 *	   has been made to do the RPC; this protects against jamming the
 *	   CPU if a KLM_CANCEL request has yet to go out.
 *
 *	3) A drastic error occurs (e.g., the local lock-manager has never
 *	   been activated OR cannot create a client-handle) (returns ENOLCK).
 */
STATIC int
talk_to_lockmgr(xdrproc, xdrargs, args, xdrreply, reply, cred)
	u_long xdrproc;
	xdrproc_t xdrargs;
	char *args;
	xdrproc_t xdrreply;
	klm_testrply *reply;
	struct cred *cred;
{
	struct timeval			tmo;
        struct netbuf			netaddr;
	CLIENT				*client;
	enum clnt_stat			stat;
	struct vnode			*vp;
	int				error, timeid;
	static char			keyname[SYS_NMLN+16];

#ifdef LOCKDEBUG
	cmn_err(CE_CONT, "entering talk_to_lockmgr()...\n");
#endif

	strcpy(keyname, utsname.nodename);
	netaddr.len = strlen(keyname);
	strcpy(&keyname[netaddr.len], ".lockd");
	netaddr.buf = keyname;
	netaddr.len = netaddr.maxlen = netaddr.len + 6;

        /* 
	 * filch a knetconfig structure.
         */
	if (config.knc_rdev == 0){
		if ((error = lookupname("/dev/ticlts", UIO_SYSSPACE, FOLLOW,
			NULLVPP, &vp)) != 0) {
			cmn_err(CE_CONT, "klm_lkmgr: lookupname: %d\n", error);
			return (error);
		}
		config.knc_rdev = vp->v_rdev;
		config.knc_protofmly = NC_LOOPBACK;
		VN_RELE(vp);
	}

#ifdef LOCKDEBUG
	cmn_err(CE_CONT, "calling clnt_tli_kcreate()\n");
#endif
	/*
	 * now call the proper stuff.
	 */
	if ((error = clnt_tli_kcreate(&config, &netaddr, (u_long)KLM_PROG,
		(u_long)KLM_VERS, 0, first_retry, cred, &client)) != 0) {
		cmn_err(CE_CONT, "klm_lkmgr: clnt_tli_kcreate: %d\n", error);
		return (ENOLCK);
	}
	tmo.tv_sec = first_timeout;
	tmo.tv_usec = 0;

retryloop:
	/* retry the request until completion, timeout, or error */
	for (;;) {
		error = (int) CLNT_CALL(client, xdrproc,
			xdrargs, (caddr_t)args, xdrreply,
			(caddr_t)reply, tmo);
#ifdef LOCKDEBUG
		cmn_err(CE_CONT, "klm_lkmgr: CLNT_CALL: error %d\n", error);
#endif
		switch (error) {
		case RPC_SUCCESS:
		case klm_denied:
			error = (int) reply->stat;
			if (error == (int) klm_working) {
				if (ISSIG(u.u_procp, 0)) {
					error = EINTR;
					goto out;
				}
				/* lock-mgr is up...can wait longer */
				/* addr is already set up */
				clnt_clts_init(client, &netaddr,
					working_retry, cred);
				tmo.tv_sec = working_timeout;
				continue;	/* retry */
			}
			goto out;	/* got a legitimate answer */

		case RPC_UDERROR:
			goto out;

		case RPC_TIMEDOUT:
			goto retryloop;	/* ask for port# again */

		case klm_denied_nolocks:
			goto out;

		default:
			cmn_err(CE_CONT,
				"lock-manager: RPC error: %s\n",
			clnt_sperrno((enum clnt_stat) error));

			/* on RPC error, wait a bit and try again */
			timeid = timeout(wakeup, (caddr_t)&config,
			    (normal_timeout * HZ));
			error = sleep((caddr_t)&config, ((PZERO+1)|PCATCH));
			untimeout(timeid);
			if (error) {
			    error = EINTR;
			    goto out;
			}
			goto retryloop;	/* ask for port# again */

		} /* switch */

	} /* for */	/* loop until timeout, error, or completion */

out:
	AUTH_DESTROY(client->cl_auth);	/* drop the authenticator */
	CLNT_DESTROY(client);		/* drop the client handle */
	return (error);
}
