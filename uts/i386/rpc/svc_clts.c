/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:svc_clts.c	1.3.2.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)svc_clts.c 1.2 89/01/11 SMI"
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
 *  		  All rights reserved.
 */

#ifdef _KERNEL
/*
 * svc_clts.c 
 * Server side for RPC in the kernel.
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <rpc/types.h>
#include <netinet/in.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc_msg.h>
#include <sys/tiuser.h>
#include <sys/t_kuser.h>
#include <rpc/svc.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/stream.h>
#include <sys/tihdr.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/kmem.h>
#include <sys/systm.h>
#include <sys/debug.h>

#define rpc_buffer(xprt) ((xprt)->xp_p1)
#define rpc_bufferlen(xprt) ((xprt)->xp_p1len)

static void unhash();

/*
 * Routines exported through ops vector.
 */
bool_t		svc_clts_krecv();
bool_t		svc_clts_ksend();
enum xprt_stat	svc_clts_kstat();
bool_t		svc_clts_kgetargs();
bool_t		svc_clts_kfreeargs();
void		svc_clts_kdestroy();


/*
 * Server transport operations vector.
 */
struct xp_ops svc_clts_op = {
	svc_clts_krecv,		/* Get requests */
	svc_clts_kstat,		/* Return status */
	svc_clts_kgetargs,	/* Deserialize arguments */
	svc_clts_ksend,		/* Send reply */
	svc_clts_kfreeargs,	/* Free argument data space */
	svc_clts_kdestroy	/* Destroy transport handle */
};

/*
 * Transport private data.
 * Kept in xprt->xp_p2.
 */
struct udp_data {
	int	ud_flags;			/* flag bits, see below */
	u_long 	ud_xid;				/* id */
	struct  t_kunitdata *ud_inudata;
	XDR	ud_xdrin;			/* input xdr stream */
	XDR	ud_xdrout;			/* output xdr stream */
	char	ud_verfbody[MAX_AUTH_BYTES];	/* verifier */
	frtn_t	ud_frtn;			/* message free routine */
};

#define	UD_MAXSIZE	8800

/*
 * Flags
 */
#define	UD_BUSY		0x001		/* buffer is busy */
#define	UD_WANTED	0x002		/* buffer wanted */

/*
 * Server statistics
 */
struct {
	int	rscalls;
	int	rsbadcalls;
	int	rsnullrecv;
	int	rsbadlen;
	int	rsxdrcall;
} rsstat;

/*
 * Create a transport record.
 * The transport record, output buffer, and private data structure
 * are allocated.  The output buffer is serialized into using xdrmem.
 * There is one transport record per user process which implements a
 * set of services.
 */
/* ARGSUSED */
int
svc_clts_kcreate(tiptr, sendsz, nxprt)
	register TIUSER			*tiptr;
	register u_int			sendsz;
	register SVCXPRT		**nxprt;
{
	register struct udp_data	*ud;
	int				error;
	SVCXPRT				*xprt;

	RPCLOG(4, "svc_clts_kcreate: Entered tiptr %x\n", tiptr);

	if (nxprt == NULL)
		return EINVAL;

	if (tiptr->tp_info.tsdu > 0)
		sendsz = MIN(tiptr->tp_info.tsdu, UD_MAXSIZE);
	else
		sendsz = UD_MAXSIZE;
	RPCLOG(4, "svc_clts_kcreate: sendsz %d\n", sendsz);

	xprt = (SVCXPRT *)kmem_alloc((u_int)sizeof(SVCXPRT), KM_SLEEP);


	rpc_buffer(xprt) = (caddr_t)kmem_alloc(sendsz, KM_SLEEP);
	rpc_bufferlen(xprt) = sendsz;

	ud = (struct udp_data *)kmem_alloc((u_int)sizeof(struct udp_data), KM_SLEEP);
	bzero((caddr_t)ud, sizeof(*ud));
	xprt->xp_p2 = (caddr_t)ud;
	xprt->xp_p3 = NULL;
	xprt->xp_verf.oa_base = ud->ud_verfbody;
	xprt->xp_ops = &svc_clts_op;
	xprt->xp_tiptr = tiptr;

	xprt->xp_ltaddr.buf = NULL;
	xprt->xp_ltaddr.maxlen = 0;
	xprt->xp_ltaddr.len = 0;

	/* Allocate receive address buffer.
	 */
	xprt->xp_rtaddr.buf = kmem_alloc(tiptr->tp_info.addr, KM_SLEEP);
	xprt->xp_rtaddr.maxlen = tiptr->tp_info.addr;
	xprt->xp_rtaddr.len = 0;
	RPCLOG(4, "svc_clts_kcreate: receive address size %d\n", 
						tiptr->tp_info.addr);
	
	*nxprt = xprt;

	return (0);
}
 
/*
 * Destroy a transport record.
 * Frees the space allocated for a transport record.
 */
void
svc_clts_kdestroy(xprt)
	register SVCXPRT		*xprt;
{
	/* LINTED pointer alignment */
	register struct udp_data	*ud = (struct udp_data *)xprt->xp_p2;
	int				error;

	RPCLOG(4, "svc_clts_kdestroy %x\n", xprt);

	if (ud->ud_inudata)
		(void)t_kfree(xprt->xp_tiptr, (char *)ud->ud_inudata,
						 T_UNITDATA);
	if (xprt->xp_ltaddr.buf)
		kmem_free(xprt->xp_ltaddr.buf, xprt->xp_ltaddr.maxlen);

	if (xprt->xp_rtaddr.buf)
		kmem_free(xprt->xp_rtaddr.buf, xprt->xp_rtaddr.maxlen);

	t_kclose(xprt->xp_tiptr, 0);
	kmem_free((caddr_t)ud, (u_int)sizeof(struct udp_data));
	kmem_free((caddr_t)rpc_buffer(xprt), rpc_bufferlen(xprt));
	kmem_free((caddr_t)xprt, (u_int)sizeof(SVCXPRT));
}

/*
 * Receive rpc requests.
 * Pulls a request in off the socket, checks if the packet is intact,
 * and deserializes the call packet.
 */
bool_t
svc_clts_krecv(xprt, msg)
	register SVCXPRT	 	*xprt;
	struct rpc_msg		 	*msg;
{
	/* LINTED pointer alignment */
	register struct udp_data	*ud = (struct udp_data *)xprt->xp_p2;
	register XDR			*xdrs = &(ud->ud_xdrin);
	struct t_kunitdata		*inudata;
	int				type;
	int				uderr;
	int				error;

	RPCLOG(4, "svc_clts_krecv %x\n", xprt);

	/* get a receive buffer
	 */
	if ((error = t_kalloc(xprt->xp_tiptr, T_UNITDATA, T_ADDR|T_UDATA,
					 (char **)&inudata)) != 0) {
		RPCLOG(1, "svc_clts_krecv: t_kalloc: %d\n", error);
		goto bad;
	}

	rsstat.rscalls++;
	if ((error = t_krcvudata(xprt->xp_tiptr, inudata, &type, &uderr))
								 != 0) {
		RPCLOG(1, "svc_clts_krecv: t_krcvudata: %d\n", error);
		if (error == EAGAIN) {
			rsstat.rsnullrecv++;
			return FALSE;
		}
		else    goto bad;
	}
	if (type != T_DATA) {
		RPCLOG(1, "svc_clts_krecv: t_krcvudata: bad type %d\n", type);
		/* Got T_UDERROR_IND
		 */
		goto bad;
	}

	RPCLOG(4, "svc_clts_krecv: t_krcvudata returned %d bytes\n", inudata->udata.len);

	if (inudata->addr.len > xprt->xp_rtaddr.maxlen) {
		RPCLOG(4, "svc_clts_krecv: Bad address len %d\n",
						inudata->addr.len);
		goto bad;
	}
	bcopy(inudata->addr.buf, xprt->xp_rtaddr.buf, inudata->addr.len);
	xprt->xp_rtaddr.len = inudata->addr.len;
 
	if (inudata->udata.len < 4*sizeof(u_long)) {
		RPCLOG(1, "svc_clts_krecv: bad length %d\n", inudata->udata.len);  

		rsstat.rsbadlen++;
		goto bad;
	}
	xdrmblk_init(xdrs, inudata->udata.udata_mp, XDR_DECODE);
	if (! xdr_callmsg(xdrs, msg)) {
		RPCLOG(1, "svc_clts_krecv: bad xdr_callmsg\n", 0);
		rsstat.rsxdrcall++;
		goto bad;
	}
	ud->ud_xid = msg->rm_xid;
	ud->ud_inudata = inudata;

	RPCLOG(4, "svc_clts_krecv done\n", 0);

	return (TRUE);
bad:
	(void)t_kfree(xprt->xp_tiptr, (char *)inudata, T_UNITDATA);
	ud->ud_inudata = NULL;

	rsstat.rsbadcalls++;
	return (FALSE);
}


static void
buffree(ud)
	register struct udp_data *ud;
{
	RPCLOG(4, "buffree: (svc) entered ud %x\n", ud);
	ud->ud_flags &= ~UD_BUSY;
	if (ud->ud_flags & UD_WANTED) {
		ud->ud_flags &= ~UD_WANTED;
		RPCLOG(4, "buffree: (svc) waking sleeper\n", 0);
		wakeprocs((caddr_t)ud, PRMPT);
	}
}

/*
 * Send rpc reply.
 * Serialize the reply packet into the output buffer then
 * call t_ksndudata to send it.
 */
bool_t
/* ARGSUSED */
svc_clts_ksend(xprt, msg)
	register SVCXPRT		*xprt; 
	struct rpc_msg			*msg; 
{
	/* LINTED pointer alignment */
	register struct udp_data	*ud = (struct udp_data *)xprt->xp_p2;
	register XDR			*xdrs = &(ud->ud_xdrout);
	register int			slen;
	register int			stat = FALSE;
	int				s;
	struct t_kunitdata		*unitdata;
	int				error;

	RPCLOG(4, "svc_clts_ksend %x\n", xprt);

	s = splstr();
	while (ud->ud_flags & UD_BUSY) {
		RPCLOG(4, "svc_clts_ksend: pid %d UD_BUSY set - sleeping\n", u.u_procp->p_pid);
		ud->ud_flags |= UD_WANTED;
		(void) sleep((caddr_t)ud, PZERO-2);
	}
	ud->ud_flags |= UD_BUSY;
	(void) splx(s);

	RPCLOG(4, "svc_clts_ksend: pid %d UD_BUSY notset\n", u.u_procp->p_pid);
	xdrmem_create(xdrs, rpc_buffer(xprt), rpc_bufferlen(xprt), XDR_ENCODE);
	msg->rm_xid = ud->ud_xid;
	if (xdr_replymsg(xdrs, msg)) {
		slen = (int)XDR_GETPOS(xdrs);
		if ((error = t_kalloc(xprt->xp_tiptr, T_UNITDATA,
				 T_ADDR|T_UDATA, (char **)&unitdata)) != 0) {
			RPCLOG(1, "svc_clts_ksend: t_kalloc: %d\n", error);    
		}
		else    {
			unitdata->addr.len = xprt->xp_rtaddr.len;
			bcopy(xprt->xp_rtaddr.buf, unitdata->addr.buf,
						unitdata->addr.len);
 
			unitdata->udata.buf = rpc_buffer(xprt);
			unitdata->udata.len = slen;
			ud->ud_frtn.free_func = buffree;
			ud->ud_frtn.free_arg  = (char *)ud;
 
RPCLOG(4, "svc_clts_ksend: calling t_ksndudata fd = %x\n", xprt->xp_tiptr);
RPCLOG(4, "svc_clts_ksend: calling t_ksndudata bytes = %d\n", unitdata->udata.len);
			if ((error = t_ksndudata(xprt->xp_tiptr, unitdata,
					 &ud->ud_frtn)) != 0) {
				RPCLOG(1,
				 "svc_clts_ksend: t_ksndudata: %d\n", error);
			}
			else	{
				stat = TRUE;
			}
			/* now we have to free up the unitdata
			 */
			(void)t_kfree(xprt->xp_tiptr, (char *)unitdata, 
					T_UNITDATA);
		}
	} else	{
		RPCLOG(4, "svc_clts_ksend: xdr_replymsg failed\n", 0);
		buffree (ud);
	}
	/*
	 * This is completely disgusting.  If public is set it is
	 * a pointer to a structure whose first field is the address
	 * of the function to free that structure and any related
	 * stuff.  (see rrokfree in nfs_xdr.c).
	 */

	if (xdrs->x_public) {
		/* LINTED pointer alignment */
		(**((int (**)())xdrs->x_public))(xdrs->x_public);
	}
	RPCLOG(4, "svc_clts_ksend done\n", 0);

	return (stat);
}

/*
 * Return transport status.
 */
/*ARGSUSED*/
enum xprt_stat
svc_clts_kstat(xprt)
	SVCXPRT *xprt;
{

	return (XPRT_IDLE); 
}

/*
 * Deserialize arguments.
 */
bool_t
svc_clts_kgetargs(xprt, xdr_args, args_ptr)
	SVCXPRT		*xprt;
	xdrproc_t	xdr_args;
	caddr_t		args_ptr;
{

	/* LINTED pointer alignment */
	return ((*xdr_args)(&(((struct udp_data *)(xprt->xp_p2))->ud_xdrin), args_ptr));
}

bool_t
svc_clts_kfreeargs(xprt, xdr_args, args_ptr)
	SVCXPRT				*xprt;
	xdrproc_t			xdr_args;
	caddr_t				args_ptr;
{
	/* LINTED pointer alignment */
	register XDR			*xdrs;
	/* LINTED pointer alignment */
	register struct	udp_data	*ud;
	int				error;

	xdrs = &(((struct udp_data *)(xprt->xp_p2))->ud_xdrin);
	ud = (struct udp_data *)xprt->xp_p2;

	if (ud->ud_inudata)
		(void)t_kfree(xprt->xp_tiptr, (char *)ud->ud_inudata,
					 T_UNITDATA);

	ud->ud_inudata = (struct t_kunitdata *)NULL;
	if (args_ptr) {
		xdrs->x_op = XDR_FREE;
		return ((*xdr_args)(xdrs, args_ptr));
	} else {
		return (TRUE);
	}
}

/*
 * the dup cacheing routines below provide a cache of non-failure
 * transaction id's.  rpc service routines can use this to detect
 * retransmissions and re-send a non-failure response.
 */

struct dupreq {
	u_long		dr_xid;
	struct netbuf	dr_addr;
	u_long		dr_proc;
	u_long		dr_vers;
	u_long		dr_prog;
	u_long		dr_flags;	/* status and in-process flags */
	long		dr_time;	/* completion timestamp */
	struct dupreq	*dr_next;
	struct dupreq	*dr_chain;
};

#define SVC_KDUP_INPROCESS	0x1
#define SVC_KDUP_SUCCESS	0x2
#define SVC_KDUP_FAILURE	0x4

/*
 * MAXDUPREQS is the number of cached items.  It should be adjusted
 * to the service load so that there is likely to be a response entry
 * when the first retransmission comes in.
 */
#define	MAXDUPREQS	400

/*
 * KDUP_THROWAWAY is the throwaway window size. Duplicate requests received
 * within this number of seconds after the original has successfully completed
 * will be thrown away without a response. The number 4 is based on the range
 * of 3-6 seconds suggested in the USENIX paper on which this scheme is based.
 */
#define KDUP_THROWAWAY	4

#define	DRHASHSZ	32
#define	XIDHASH(xid)	((xid) & (DRHASHSZ-1))
#define	DRHASH(dr)	XIDHASH((dr)->dr_xid)
#define	REQTOXID(req)	((struct udp_data *)((req)->rq_xprt->xp_p2))->ud_xid

STATIC int	ndupreqs;
STATIC int	dupreqthrows;
STATIC int	dupreqreplies;
STATIC int	dupchecks;
STATIC struct dupreq *drhashtbl[DRHASHSZ];

/*
 * drmru points to the head of a circular linked list in lru order.
 * drmru->dr_next == drlru
 */
STATIC struct dupreq *drmru;

void
svc_clts_kdupstart(req)
	register struct svc_req *req;
{

	register struct dupreq *dr;

	if (ndupreqs < MAXDUPREQS) {
		dr = (struct dupreq *)kmem_zalloc(sizeof(*dr), KM_SLEEP);
		if (drmru) {
			dr->dr_next = drmru->dr_next;
			drmru->dr_next = dr;
		} else {
			dr->dr_next = dr;
		}
		ndupreqs++;
	} else {
		dr = drmru->dr_next;
		unhash(dr);
	}
	drmru = dr;

	/* LINTED pointer alignment */
	dr->dr_xid = REQTOXID(req);
	dr->dr_prog = req->rq_prog;
	dr->dr_vers = req->rq_vers;
	dr->dr_proc = req->rq_proc;
	if (dr->dr_addr.maxlen < req->rq_xprt->xp_rtaddr.maxlen) {
		if (dr->dr_addr.maxlen != 0)
			kmem_free(dr->dr_addr.buf, dr->dr_addr.maxlen);
		dr->dr_addr.maxlen = req->rq_xprt->xp_rtaddr.maxlen;
		dr->dr_addr.buf = kmem_alloc(dr->dr_addr.maxlen, KM_SLEEP);
	}
	dr->dr_addr.len = req->rq_xprt->xp_rtaddr.len;
	bcopy(req->rq_xprt->xp_rtaddr.buf, dr->dr_addr.buf, dr->dr_addr.len);
	dr->dr_flags |= SVC_KDUP_INPROCESS;
	dr->dr_chain = drhashtbl[DRHASH(dr)];
	drhashtbl[DRHASH(dr)] = dr;
}

void
svc_clts_kdupend(req, status)
	register struct svc_req *req;
	int status;
{
	register struct dupreq *dr;
	u_long xid;

	dupchecks++;
	/*
	 * First try to find the original request entry.
	 */
	/* LINTED pointer alignment */
	xid = REQTOXID(req);
	dr = drhashtbl[XIDHASH(xid)];
	while (dr != NULL) {
		if (dr->dr_xid != xid ||
		    dr->dr_prog != req->rq_prog ||
		    dr->dr_vers != req->rq_vers ||
		    dr->dr_proc != req->rq_proc ||
		    dr->dr_addr.len != req->rq_xprt->xp_rtaddr.len ||
		    bcmp((caddr_t)dr->dr_addr.buf,
			 (caddr_t)req->rq_xprt->xp_rtaddr.buf,
			 dr->dr_addr.len) != 0) {
			dr = dr->dr_chain;
			continue;
		} else
			break;
	}
	if (dr == NULL) {
		/*
		 * Strange, it wasn't there. Let's create a new one to record
		 * the completion status.
		 */
		RPCLOG(1, "svc_clts_kdupend: original not found for prog %d, ", req->rq_prog);
		RPCLOG(1, "vers %d, ", req->rq_vers);
		RPCLOG(1, "proc %d, ", req->rq_proc);
		RPCLOG(1, "xid %d\n", REQTOXID(req));
		if (ndupreqs < MAXDUPREQS) {
			dr = (struct dupreq *)kmem_zalloc(sizeof(*dr), KM_SLEEP);
			if (drmru) {
				dr->dr_next = drmru->dr_next;
				drmru->dr_next = dr;
			} else {
				dr->dr_next = dr;
			}
			ndupreqs++;
		} else {
			dr = drmru->dr_next;
			unhash(dr);
		}
		drmru = dr;
		dr->dr_chain = drhashtbl[DRHASH(dr)];
		drhashtbl[DRHASH(dr)] = dr;
	}

	/* LINTED pointer alignment */
	dr->dr_xid = REQTOXID(req);
	dr->dr_prog = req->rq_prog;
	dr->dr_vers = req->rq_vers;
	dr->dr_proc = req->rq_proc;
	if (dr->dr_addr.maxlen < req->rq_xprt->xp_rtaddr.maxlen) {
		if (dr->dr_addr.maxlen != 0)
			kmem_free(dr->dr_addr.buf, dr->dr_addr.maxlen);
		dr->dr_addr.maxlen = req->rq_xprt->xp_rtaddr.maxlen;
		dr->dr_addr.buf = kmem_alloc(dr->dr_addr.maxlen, KM_SLEEP);
	}
	dr->dr_addr.len = req->rq_xprt->xp_rtaddr.len;
	bcopy(req->rq_xprt->xp_rtaddr.buf, dr->dr_addr.buf, dr->dr_addr.len);
	dr->dr_flags &= ~SVC_KDUP_INPROCESS;
	dr->dr_flags |= ((status == 0) ? SVC_KDUP_SUCCESS : SVC_KDUP_FAILURE);
	dr->dr_time = hrestime.tv_sec;
}

int
svc_clts_kdup_throw(req)
	register struct svc_req *req;
{
	register struct dupreq *dr;
	u_long xid;

	dupchecks++;
	/* LINTED pointer alignment */
	xid = REQTOXID(req);
	dr = drhashtbl[XIDHASH(xid)];
	while (dr != NULL) {
		if (dr->dr_xid != xid ||
		    dr->dr_prog != req->rq_prog ||
		    dr->dr_vers != req->rq_vers ||
		    dr->dr_proc != req->rq_proc ||
		    dr->dr_addr.len != req->rq_xprt->xp_rtaddr.len ||
		    bcmp((caddr_t)dr->dr_addr.buf,
			 (caddr_t)req->rq_xprt->xp_rtaddr.buf,
			 dr->dr_addr.len) != 0) {
			dr = dr->dr_chain;
			continue;
		} else {
			if ((dr->dr_flags & SVC_KDUP_INPROCESS) ||
			    ((dr->dr_flags & SVC_KDUP_SUCCESS) &&
			     (hrestime.tv_sec-KDUP_THROWAWAY <= dr->dr_time))) {
				dupreqthrows++;
				return (1);
			} else
				return (0);
		}
	}
	return (0);
}

int
svc_clts_kdup_reply(req)
	register struct svc_req *req;
{
	register struct dupreq *dr;
	u_long xid;

	dupchecks++;
	/* LINTED pointer alignment */
	xid = REQTOXID(req);
	dr = drhashtbl[XIDHASH(xid)];
	while (dr != NULL) {
		if (dr->dr_xid != xid ||
		    dr->dr_prog != req->rq_prog ||
		    dr->dr_vers != req->rq_vers ||
		    dr->dr_proc != req->rq_proc ||
		    dr->dr_addr.len != req->rq_xprt->xp_rtaddr.len ||
		    bcmp((caddr_t)dr->dr_addr.buf,
			 (caddr_t)req->rq_xprt->xp_rtaddr.buf,
			 dr->dr_addr.len) != 0) {
			dr = dr->dr_chain;
			continue;
		} else {
			if ((dr->dr_flags & SVC_KDUP_SUCCESS) &&
			    (hrestime.tv_sec - KDUP_THROWAWAY > dr->dr_time)) {
				dupreqreplies++;
				return (1);
			} else
				return (0);
		}
	}
	return (0);
}

static void
unhash(dr)
	struct dupreq *dr;
{
	struct dupreq *drt;
	struct dupreq *drtprev = NULL;
	 
	drt = drhashtbl[DRHASH(dr)]; 
	while (drt != NULL) { 
		if (drt == dr) { 
			if (drtprev == NULL) {
				drhashtbl[DRHASH(dr)] = drt->dr_chain;
			} else {
				drtprev->dr_chain = drt->dr_chain;
			}
			return; 
		}	
		drtprev = drt;
		drt = drt->dr_chain;
	}	
}
#endif	/* _KERNEL */
