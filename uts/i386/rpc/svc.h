/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:svc.h	1.3"

/*      @(#)svc.h 1.35 88/12/17 SMI      */

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
 * svc.h, Server-side remote procedure call interface.
 *
 */

#ifndef _RPC_SVC_H
#define _RPC_SVC_H

#include <rpc/rpc_com.h>

/*
 * This interface must manage two items concerning remote procedure calling:
 *
 * 1) An arbitrary number of transport connections upon which rpc requests
 * are received. They are created and registered by routines in svc_generic.c,
 * svc_vc.c and svc_dg.c; they in turn call xprt_register and 
 * xprt_unregister.
 *
 * 2) An arbitrary number of locally registered services.  Services are
 * described by the following four data: program number, version number,
 * "service dispatch" function, a transport handle, and a boolean that
 * indicates whether or not the exported program should be registered with a
 * local binder service;  if true the program's number and version and the
 * address from the transport handle are registered with the binder.
 * These data are registered with rpcbind via svc_reg().
 *
 * A service's dispatch function is called whenever an rpc request comes in
 * on a transport.  The request's program and version numbers must match
 * those of the registered service.  The dispatch function is passed two
 * parameters, struct svc_req * and SVCXPRT *, defined below.
 */

enum xprt_stat {
	XPRT_DIED,
	XPRT_MOREREQS,
	XPRT_IDLE
};

/*
 * Server side transport handle
 */
typedef struct {
#ifdef _KERNEL
	TIUSER		*xp_tiptr;
#else
	int		xp_fd;
#define xp_sock		xp_fd
#endif
	u_short		xp_port;	 /* associated port number.
					  * Obsoleted, but still used to
					  * specify whether rendezvouser
					  * or normal connection
					  */
	struct xp_ops {
	    bool_t	(*xp_recv)();	 /* receive incoming requests */
	    enum xprt_stat (*xp_stat)(); /* get transport status */
	    bool_t	(*xp_getargs)(); /* get arguments */
	    bool_t	(*xp_reply)();	 /* send reply */
	    bool_t	(*xp_freeargs)();/* free mem allocated for args */
	    void	(*xp_destroy)(); /* destroy this struct */
	} *xp_ops;
#ifndef _KERNEL
	int		xp_addrlen;	 /* length of remote addr. Obsoleted */
	char		*xp_tp;		 /* transport provider device name */
	char		*xp_netid;	 /* network token */
#endif
	struct netbuf	xp_ltaddr;	 /* local transport address */
	struct netbuf	xp_rtaddr;	 /* remote transport address */
#ifndef _KERNEL
	char		xp_raddr[16];	 /* remote address. Now obsoleted */
#endif
	struct opaque_auth xp_verf;	 /* raw response verifier */
	caddr_t		xp_p1;		 /* private: for use by svc ops */
#ifdef _KERNEL
	u_int		xp_p1len;	 /* size of p1 */
#endif
	caddr_t		xp_p2;		 /* private: for use by svc ops */
	caddr_t		xp_p3;		 /* private: for use by svc lib */
} SVCXPRT;

/*
 *  Approved way of getting address of caller
 */
#define svc_getrpccaller(x) (&(x)->xp_rtaddr)
#ifdef _KERNEL
#define svc_getcaller(x) (&(x)->xp_rtaddr.buf)
#endif

/*
 * Operations defined on an SVCXPRT handle
 *
 * SVCXPRT		*xprt;
 * struct rpc_msg	*msg;
 * xdrproc_t		 xargs;
 * caddr_t		 argsp;
 */
#define SVC_RECV(xprt, msg)				\
	(*(xprt)->xp_ops->xp_recv)((xprt), (msg))
#define svc_recv(xprt, msg)				\
	(*(xprt)->xp_ops->xp_recv)((xprt), (msg))

#define SVC_STAT(xprt)					\
	(*(xprt)->xp_ops->xp_stat)(xprt)
#define svc_stat(xprt)					\
	(*(xprt)->xp_ops->xp_stat)(xprt)

#define SVC_GETARGS(xprt, xargs, argsp)			\
	(*(xprt)->xp_ops->xp_getargs)((xprt), (xargs), (argsp))
#define svc_getargs(xprt, xargs, argsp)			\
	(*(xprt)->xp_ops->xp_getargs)((xprt), (xargs), (argsp))

#define SVC_REPLY(xprt, msg)				\
	(*(xprt)->xp_ops->xp_reply) ((xprt), (msg))
#define svc_reply(xprt, msg)				\
	(*(xprt)->xp_ops->xp_reply) ((xprt), (msg))

#define SVC_FREEARGS(xprt, xargs, argsp)		\
	(*(xprt)->xp_ops->xp_freeargs)((xprt), (xargs), (argsp))
#define svc_freeargs(xprt, xargs, argsp)		\
	(*(xprt)->xp_ops->xp_freeargs)((xprt), (xargs), (argsp))

#define SVC_DESTROY(xprt)				\
	(*(xprt)->xp_ops->xp_destroy)(xprt)
#define svc_destroy(xprt)				\
	(*(xprt)->xp_ops->xp_destroy)(xprt)


/*
 * Service request
 */
struct svc_req {
	u_long		rq_prog;	/* service program number */
	u_long		rq_vers;	/* service protocol version */
	u_long		rq_proc;	/* the desired procedure */
	struct opaque_auth rq_cred;	/* raw creds from the wire */
	caddr_t		rq_clntcred;	/* read only cooked cred */
	SVCXPRT		*rq_xprt;	/* associated transport */
};


/*
 * Service registration
 *
 * svc_reg(xprt, prog, vers, dispatch, nconf)
 *	SVCXPRT *xprt;
 *	u_long prog;
 *	u_long vers;
 *	void (*dispatch)();
 *	struct netconfig *nconf;
 */
extern bool_t	svc_reg();

/*
 * Service un-registration
 *
 * svc_unreg(prog, vers)
 *	u_long prog;
 *	u_long vers;
 */
extern void	svc_unreg();

/*
 * Transport registration.
 *
 * xprt_register(xprt)
 *	SVCXPRT *xprt;
 */
extern void	xprt_register();

/*
 * Transport un-register
 *
 * xprt_unregister(xprt)
 *	SVCXPRT *xprt;
 */
extern void	xprt_unregister();


/*
 * When the service routine is called, it must first check to see if it
 * knows about the procedure;  if not, it should call svcerr_noproc
 * and return.  If so, it should deserialize its arguments via 
 * SVC_GETARGS (defined above).  If the deserialization does not work,
 * svcerr_decode should be called followed by a return.  Successful
 * decoding of the arguments should be followed the execution of the
 * procedure's code and a call to svc_sendreply.
 *
 * Also, if the service refuses to execute the procedure due to too-
 * weak authentication parameters, svcerr_weakauth should be called.
 * Note: do not confuse access-control failure with weak authentication!
 *
 * NB: In pure implementations of rpc, the caller always waits for a reply
 * msg.  This message is sent when svc_sendreply is called.  
 * Therefore pure service implementations should always call
 * svc_sendreply even if the function logically returns void;  use
 * xdr.h - xdr_void for the xdr routine.  HOWEVER, connectionful rpc allows
 * for the abuse of pure rpc via batched calling or pipelining.  In the
 * case of a batched call, svc_sendreply should NOT be called since
 * this would send a return message, which is what batching tries to avoid.
 * It is the service/protocol writer's responsibility to know which calls are
 * batched and which are not.  Warning: responding to batch calls may
 * deadlock the caller and server processes!
 */

extern bool_t	svc_sendreply();
extern void	svcerr_decode();
extern void	svcerr_weakauth();
extern void	svcerr_noproc();
extern void	svcerr_progvers();
extern void	svcerr_auth();
extern void	svcerr_noprog();
#ifndef _KERNEL
extern void	svcerr_systemerr();
#endif
    
/*
 * Lowest level dispatching -OR- who owns this process anyway.
 * Somebody has to wait for incoming requests and then call the correct
 * service routine.  The routine svc_run does infinite waiting; i.e.,
 * svc_run never returns.
 * Since another (co-existant) package may wish to selectively wait for
 * incoming calls or other events outside of the rpc architecture, the
 * routine svc_getreq is provided.  It must be passed readfds, the
 * "in-place" results of a select call (see select, section XXX).
 */

#ifndef _KERNEL
/*
 * Global keeper of rpc service descriptors in use
 * dynamic; must be inspected before each call to select 
 */
extern fd_set svc_fdset;
#define svc_fds svc_fdset.fds_bits[0]	/* compatibility */

/*
 * a small program implemented by the svc_rpc implementation itself;
 * also see clnt.h for protocol numbers.
 */
extern void rpctest_service();
#endif /* !_KERNEL */

extern void	svc_getreq();
#ifndef _KERNEL
extern void	svc_getreqset();	/* takes fdset instead of int */
#endif
extern void	svc_run();


#ifndef _KERNEL
/*
 * These are the existing service side transport implementations
 */
/*
 * Transport independent svc_create routine.
 */
extern int
svc_create(/* dispatch, prognum, versnum, nettype*/); /*
	void (*dispatch)();		-- dispatch routine
	u_long prognum;			-- program number
	u_long versnum;			-- version number
	char *nettype;			-- network type
*/

/*
 * Generic server creation routine. It takes a netconfig structure
 * instead of a nettype.
 */
extern SVCXPRT	*
svc_tp_create(/* dispatch, prognum, versnum, nconf*/); /*
	void (*dispatch)();		-- dispatch routine
	u_long prognum;			-- program number
	u_long versnum;			-- version number
	struct netconfig *nconf;	-- netconfig structure
*/

/*
 * Generic TLI create routine
 */
extern SVCXPRT *
svc_tli_create(/* fd, nconf, bindaddr, sendsz, recvsz*/) ; /*
	int fd;			 	-- connection end point
	struct netconfig *nconf;	-- netconfig structure for network
	struct t_bind *bindaddr;	-- local bind address
	u_int sendsz;			-- max sendsize
	u_int recvsz;			-- max recvsize
*/

/*
 * Connectionless and connectionful create routines
 */
extern SVCXPRT	*
svc_vc_create(/* fd, sendsize, recvsize*/); /*
	int fd;				-- open connection end point
	u_int sendsize;			-- max send size
	u_int recvsize;			-- max recv size
*/

extern SVCXPRT	*
svc_dg_create(/* fd, sendsize, recvsize*/); /*
	int fd;				-- open connection end point
	u_int sendsize;			-- max send size
	u_int recvsize;			-- max recv size
*/

/*
 * the routine takes any *open* TLI file
 * descriptor as its first input and is used for open connections.
 */
extern SVCXPRT *
svc_fd_create(/* fd, sendsize, recvsize*/); /*
	int fd;				-- open connection end point
	u_int sendsize;			-- max send size
	u_int recvsize;			-- max recv size
*/

/*
 * Memory based rpc (for speed check and testing)
 */
extern SVCXPRT *
svc_raw_create();

#ifdef PORTMAP
/* For backword compatibility */
#include <rpc/svc_soc.h>
#endif

#else
/* kernel based rpc
 */
extern int svc_tli_kcreate();
#endif /* !_KERNEL */
#endif /* !_RPC_SVC_H */
