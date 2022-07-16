/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:clnt.h	1.3"

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
 * clnt.h - Client side remote procedure call interface.
 *
 */

#ifndef _RPC_CLNT_H
#define _RPC_CLNT_H

#include <rpc/rpc_com.h>
/*
 * rpc calls return an enum clnt_stat.  This should be looked at more,
 * since each implementation is required to live with this (implementation
 * independent) list of errors.
 */
enum clnt_stat {
	RPC_SUCCESS=0,			/* call succeeded */
	/*
	 * local errors
	 */
	RPC_CANTENCODEARGS=1,		/* can't encode arguments */
	RPC_CANTDECODERES=2,		/* can't decode results */
	RPC_CANTSEND=3,			/* failure in sending call */
	RPC_CANTRECV=4,			/* failure in receiving result */
	RPC_TIMEDOUT=5,			/* call timed out */
	RPC_INTR=18,			/* call interrupted */
	RPC_UDERROR=23,			/* recv got uderr indication */
	/*
	 * remote errors
	 */
	RPC_VERSMISMATCH=6,		/* rpc versions not compatible */
	RPC_AUTHERROR=7,		/* authentication error */
	RPC_PROGUNAVAIL=8,		/* program not available */
	RPC_PROGVERSMISMATCH=9,		/* program version mismatched */
	RPC_PROCUNAVAIL=10,		/* procedure unavailable */
	RPC_CANTDECODEARGS=11,		/* decode arguments error */
	RPC_SYSTEMERROR=12,		/* generic "other problem" */

	/*
	 * rpc_call & clnt_create errors
	 */
	RPC_UNKNOWNHOST=13,		/* unknown host name */
	RPC_UNKNOWNPROTO=17,		/* unknown protocol */
	RPC_UNKNOWNADDR=19,		/* Remote address unknown */
	RPC_NOBROADCAST=21,		/* Broadcasting not supported */

	/*
	 * rpcbind errors
	 */
	RPC_RPCBFAILURE=14,		/* the pmapper failed in its call */
#define RPC_PMAPFAILURE RPC_RPCBFAILURE
	RPC_PROGNOTREGISTERED=15,	/* remote program is not registered */
	RPC_N2AXLATEFAILURE=22,		/* Name to address translation failed */
	/*
	 * Misc error in the TLI library
	 */
	RPC_TLIERROR=20,
	/*
	 * unspecified error
	 */
	RPC_FAILED=16
};


/*
 * Error info.
 */
struct rpc_err {
	enum clnt_stat re_status;
	union {
		struct {
			int errno;	/* related system error */
			int t_errno;	/* related tli error number */
		} RE_err;
		enum auth_stat RE_why;	/* why the auth error occurred */
		struct {
			u_long low;	/* lowest verion supported */
			u_long high;	/* highest verion supported */
		} RE_vers;
		struct {		/* maybe meaningful if RPC_FAILED */
			long s1;
			long s2;
		} RE_lb;		/* life boot & debugging only */
	} ru;
#define	re_errno	ru.RE_err.errno
#define	re_terrno	ru.RE_err.t_errno
#define	re_why		ru.RE_why
#define	re_vers		ru.RE_vers
#define	re_lb		ru.RE_lb
};


/*
 * Client rpc handle.
 * Created by individual implementations
 * Client is responsible for initializing auth, see e.g. auth_none.c.
 */
typedef struct {
	AUTH	*cl_auth;			/* authenticator */
	struct clnt_ops {
		enum clnt_stat	(*cl_call)();	/* call remote procedure */
		void		(*cl_abort)();	/* abort a call */
		void		(*cl_geterr)();	/* get specific error code */
		bool_t		(*cl_freeres)();/* frees results */
		void		(*cl_destroy)();/* destroy this structure */
		bool_t		(*cl_control)();/* the ioctl() of rpc */
	} *cl_ops;
	caddr_t			cl_private;	/* private stuff */
#ifndef _KERNEL
	char			*cl_netid;	/* network token */
	char			*cl_tp;		/* device name */
#endif
} CLIENT;


/*
 * Timers used for the pseudo-transport protocol when using datagrams
 */
struct rpc_timers {
	u_short		rt_srtt;	/* smoothed round-trip time */
	u_short		rt_deviate;	/* estimated deviation */
	u_long		rt_rtxcur;	/* current (backed-off) rto */
};

/*
 * Feedback values used for possible congestion and rate control
 */
#define	FEEDBACK_REXMIT1	1	/* first retransmit */
#define	FEEDBACK_OK		2	/* no retransmits */

#define	RPCSMALLMSGSIZE	400	/* a more reasonable packet size */

#define	KNC_STRSIZE	128	/* maximum length of knetconfig strings */
struct knetconfig {
	unsigned long	knc_semantics;	/* token name */
	char		*knc_protofmly;	/* protocol family */
	char		*knc_proto;	/* protocol */
	dev_t		knc_rdev;	/* device id */
	unsigned long	knc_unused[8];
};

/*
 * client side rpc interface ops
 */

/*
 * enum clnt_stat
 * CLNT_CALL(rh, proc, xargs, argsp, xres, resp, timeout)
 * 	CLIENT *rh;
 *	u_long proc;
 *	xdrproc_t xargs;
 *	caddr_t argsp;
 *	xdrproc_t xres;
 *	caddr_t resp;
 *	struct timeval timeout;
 */
#define	CLNT_CALL(rh, proc, xargs, argsp, xres, resp, secs)	\
	((*(rh)->cl_ops->cl_call)(rh, proc, xargs, argsp, xres, resp, secs))
#define	clnt_call(rh, proc, xargs, argsp, xres, resp, secs)	\
	((*(rh)->cl_ops->cl_call)(rh, proc, xargs, argsp, xres, resp, secs))

/*
 * void
 * CLNT_ABORT(rh);
 * 	CLIENT *rh;
 */
#define	CLNT_ABORT(rh)	((*(rh)->cl_ops->cl_abort)(rh))
#define	clnt_abort(rh)	((*(rh)->cl_ops->cl_abort)(rh))

/*
 * struct rpc_err
 * CLNT_GETERR(rh);
 * 	CLIENT *rh;
 */
#define	CLNT_GETERR(rh, errp)	((*(rh)->cl_ops->cl_geterr)(rh, errp))
#define	clnt_geterr(rh, errp)	((*(rh)->cl_ops->cl_geterr)(rh, errp))

/*
 * bool_t
 * CLNT_FREERES(rh, xres, resp);
 * 	CLIENT *rh;
 *	xdrproc_t xres;
 *	caddr_t resp;
 */
#define	CLNT_FREERES(rh,xres,resp) ((*(rh)->cl_ops->cl_freeres)(rh,xres,resp))
#define	clnt_freeres(rh,xres,resp) ((*(rh)->cl_ops->cl_freeres)(rh,xres,resp))

/*
 * bool_t
 * CLNT_CONTROL(cl, request, info)
 *	CLIENT *cl;
 *	u_int request;
 *	char *info;
 */
#define	CLNT_CONTROL(cl, rq, in) ((*(cl)->cl_ops->cl_control)(cl, rq, in))
#define	clnt_control(cl, rq, in) ((*(cl)->cl_ops->cl_control)(cl, rq, in))


/*
 * control operations that apply to all transports
 */
#define	CLSET_TIMEOUT		1	/* set timeout (timeval) */
#define	CLGET_TIMEOUT		2	/* get timeout (timeval) */
#define	CLGET_SERVER_ADDR	3	/* get server's address (sockaddr) */
#define	CLGET_FD		6	/* get connections file descriptor */
#define	CLGET_SVC_ADDR		7	/* get server's address (netbuf) */
#define	CLSET_FD_CLOSE		8	/* close fd while clnt_destroy */
#define	CLSET_FD_NCLOSE		9	/* Do not close fd while clnt_destroy */
/*
 * Connectionless only control operations
 */
#define	CLSET_RETRY_TIMEOUT 4   /* set retry timeout (timeval) */
#define	CLGET_RETRY_TIMEOUT 5   /* get retry timeout (timeval) */

/*
 * void
 * CLNT_DESTROY(rh);
 * 	CLIENT *rh;
 */
#define	CLNT_DESTROY(rh)	((*(rh)->cl_ops->cl_destroy)(rh))
#define	clnt_destroy(rh)	((*(rh)->cl_ops->cl_destroy)(rh))


/*
 * RPCTEST is a test program which is accessable on every rpc
 * transport/port.  It is used for testing, performance evaluation,
 * and network administration.
 */

#define	RPCTEST_PROGRAM		((u_long)1)
#define	RPCTEST_VERSION		((u_long)1)
#define	RPCTEST_NULL_PROC	((u_long)2)
#define	RPCTEST_NULL_BATCH_PROC	((u_long)3)

/*
 * By convention, procedure 0 takes null arguments and returns them
 */

#define	NULLPROC ((u_long)0)

/*
 * Below are the client handle creation routines for the various
 * implementations of client side rpc.  They can return NULL if a
 * creation failure occurs.
 */

#ifndef _KERNEL
/*
 * Generic client creation routine. Supported protocols are which belong
 * to the nettype name space
 */
extern CLIENT *
clnt_create(/*hostname, prog, vers, nettype*/); /*
	char *hostname;			-- hostname
	u_long prog;			-- program number
	u_long vers;			-- version number
	char *nettype;			-- network type
*/

/*
 * Generic client creation routine. It takes a netconfig structure
 * instead of nettype
 */
extern CLIENT *
clnt_tp_create(/*hostname, prog, vers, netconf*/); /*
	char *hostname;			-- hostname
	u_long prog;			-- program number
	u_long vers;			-- version number
	struct netconfig *netconf; 	-- network config structure
*/

/*
 * Generic TLI create routine
 */
extern CLIENT *
clnt_tli_create(/*fd, netconf, svcaddr, prog, vers, sendsz, recvsz*/); /*
	register int fd;		-- fd
	struct netconfig *nconf;	-- netconfig structure
	struct netbuf *svcaddr;		-- servers address
	u_long prog;			-- program number
	u_long vers;			-- version number
	u_int sendsz;			-- send size
	u_int recvsz;			-- recv size
*/

/*
 * Low level clnt create routine for connectionful transports, e.g. tcp.
 */
extern CLIENT *
clnt_vc_create(/*fd, svcaddr, prog, vers, sendsz, recvsz*/); /*
	int fd;				-- open file descriptor
	struct netbuf *svcaddr;		-- servers address
	u_long prog;			-- program number
	u_long vers;			-- version number
	u_int sendsz;			-- buffer recv size
	u_int recvsz;			-- buffer send size
*/

/*
 * Low level clnt create routine for connectionless transports, e.g. udp.
 */
extern CLIENT *
clnt_dg_create(/*fd, svcaddr, program, version, sendsz, recvsz*/); /*
	int fd;				-- open file descriptor
	struct netbuf *svcaddr;		-- servers address
	u_long program;			-- program number
	u_long version;			-- version number
	u_int sendsz;			-- buffer recv size
	u_int recvsz;			-- buffer send size
*/

/*
 * Memory based rpc (for speed check and testing)
 * CLIENT *
 * clnt_raw_create(prog, vers)
 *	u_long prog;			-- program number
 *	u_long vers;			-- version number
 */
extern CLIENT *clnt_raw_create();


/*
 * Print why creation failed
 */
void clnt_pcreateerror(/* char *msg */);	/* stderr */
char *clnt_spcreateerror(/* char *msg */);	/* string */

/*
 * Like clnt_perror(), but is more verbose in its output
 */
void clnt_perrno(/* enum clnt_stat num */);	/* stderr */

/*
 * Print an error message, given the client error code
 */
void clnt_perror(/* CLIENT *clnt, char *msg */); 	/* stderr */
char *clnt_sperror(/* CLIENT *clnt, char *msg */);	/* string */

/*
 * If a creation fails, the following allows the user to figure out why.
 */
struct rpc_createerr {
	enum clnt_stat cf_stat;
	struct rpc_err cf_error; /* useful when cf_stat == RPC_PMAPFAILURE */
};

extern struct rpc_createerr rpc_createerr;

/*
 * The simplified interface:
 * enum clnt_stat
 * rpc_call(host, prognum, versnum, procnum, inproc, in, outproc, out, nettype)
 *	char *host;
 *	u_long prognum, versnum, procnum;
 *	xdrproc_t inproc, outproc;
 *	char *in, *out;
 *	char *nettype;
 */
extern enum clnt_stat rpc_call();

/*
 * RPC broadcast interface
 * extern enum clnt_stat
 * rpc_broadcast(prog, vers, proc, xargs, argsp, xresults, resultsp,
 *			eachresult, nettype)
 *	u_long		prog;		-- program number
 *	u_long		vers;		-- version number
 *	u_long		proc;		-- procedure number
 *	xdrproc_t	xargs;		-- xdr routine for args
 *	caddr_t		argsp;		-- pointer to args
 *	xdrproc_t	xresults;	-- xdr routine for results
 *	caddr_t		resultsp;	-- pointer to results
 *	resultproc_t	eachresult;	-- call with each result obtained
 *	char		*nettype;	-- Transport type
 */
extern enum clnt_stat rpc_broadcast();

#endif /* !KERNEL */

/*
 * Copy error message to buffer.
 */
char *clnt_sperrno(/* enum clnt_stat num */);	/* string */

#ifdef PORTMAP
/* For backword compatibility */
#include <rpc/clnt_soc.h>
#endif

#endif /* !_RPC_CLNT_H */
