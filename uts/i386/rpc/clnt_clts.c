/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:clnt_clts.c	1.3.1.1"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)clnt_clts.c 1.3 89/01/11 SMI"
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

#ifdef	_KERNEL
/*
 * Implements a kernel based, client side RPC.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/systm.h>
#include <sys/sysmacros.h>
#include <sys/proc.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <sys/file.h>
#include <rpc/rpc_msg.h>
#include <sys/stream.h>
#include <sys/strsubr.h>
#include <sys/tiuser.h>
#include <sys/tihdr.h>
#include <sys/t_kuser.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/kmem.h>
#include <sys/debug.h>
#include <sys/systm.h>

#define	NC_INET	"inet"		/* XXX */

int		ckuwakeup();
void		clnt_clts_init();
void		clnt_clts_reopen();

enum clnt_stat	clnt_clts_kcallit();
void		clnt_clts_kabort();
void		clnt_clts_kerror();
STATIC bool_t	clnt_clts_kfreeres();
bool_t		clnt_clts_kcontrol();
void		clnt_clts_kdestroy();

/*
 * Operations vector for UDP/IP based RPC
 */
static struct clnt_ops udp_ops = {
	clnt_clts_kcallit,	/* do rpc call */
	clnt_clts_kabort,	/* abort call */
	clnt_clts_kerror,	/* return error status */
	clnt_clts_kfreeres,	/* free results */
	clnt_clts_kdestroy,	/* destroy rpc handle */
	clnt_clts_kcontrol	/* the ioctl() of rpc */
};

/*
 * Private data per rpc handle.  This structure is allocated by
 * clnt_clts_kcreate, and freed by clnt_clts_kdestroy.
 */
struct cku_private {
	u_int			 cku_flags;	/* see below */
	CLIENT			 cku_client;	/* client handle */
	int			 cku_retrys;	/* request retrys */
	TIUSER 			*cku_tiptr;	/* open tli file pointer */
	dev_t			 cku_device;	/* device cku_tiptr has open */
	struct netbuf		 cku_addr;	/* remote address */
	struct rpc_err		 cku_err;	/* error status */
	XDR			 cku_outxdr;	/* xdr routine for output */
	XDR			 cku_inxdr;	/* xdr routine for input */
	u_int			 cku_outpos;	/* position of in output mbuf */
	char			*cku_outbuf;	/* output buffer */
	u_int			 cku_outbuflen;	/* size of output buffer */
	char			*cku_inbuf;	/* input buffer */
	struct t_kunitdata	*cku_inudata;	/* input tli buf */
	struct cred		*cku_cred;	/* credentials */
	struct rpc_timers	*cku_timers;	/* for estimating RTT */
	struct rpc_timers	*cku_timeall;	/* for estimating RTT */
	void			 (*cku_feedback)();
	caddr_t			 cku_feedarg;	/* argument for feedback func */
	u_long			 cku_xid;	/* current XID */
	frtn_t			 cku_frtn;	/* message free routine */
};

struct {
	int	rccalls;
	int	rcbadcalls;
	int	rcretrans;
	int	rcbadxids;
	int	rctimeouts;
	int	rcwaits;
	int	rcnewcreds;
	int	rcbadverfs;
	int	rctimers;
	int	rctoobig;
	int	rcnomem;
	int	rccantsend;
	int	rcbufulocks;
} rcstat;


#define	ptoh(p)		(&((p)->cku_client))
#define	htop(h)		((struct cku_private *)((h)->cl_private))

/* cku_flags */
#define	CKU_TIMEDOUT	0x001
#define	CKU_BUSY	0x002
#define	CKU_WANTED	0x004
#define	CKU_BUFBUSY	0x008
#define	CKU_BUFWANTED	0x010
#define CKU_LOANEDBUF	0x020

/* Times to retry 
*/
#define	RECVTRIES	2
#define	SNDTRIES	4

#define	CKU_MAXSIZE	8800

int	clnt_clts_xid = 0;		/* transaction id used by all clients */

/*
 * kernel RPC transaction ID handling: Clnt_clts_getxid() is the one entry point
 * that all users of KRPC should call to obtain a new transaction id. They
 * should then call clnt_clts_setxid() to save taht transaction ID in the client
 * handle. Note that clnt_clts_kcreate(), clnt_clts_init(), and
 * clnt_clts_reopen() all zero p->cku_xid, so setxid() should be called after
 * any of those routines. Clnt_clts_kcallit_addr() will use the value in
 * p->cku_xid if it is set, or will call getxid() if not.
 */
int
clnt_clts_getxid()
{
	if (clnt_clts_xid == 0)
		clnt_clts_xid = hrestime.tv_sec;
	else
		clnt_clts_xid++;
	return (clnt_clts_xid);
}

void
clnt_clts_setxid(h, xid)
	CLIENT *h;
	int xid;
{
	/* LINTED pointer alignment */
	register struct cku_private *p = htop(h);
	p->cku_xid = xid;
}

STATIC void
buffree(p)
	struct cku_private *p;
{
	RPCLOG(2, "buffree: (client) entered p %x\n", p);
	p->cku_flags &= ~CKU_BUFBUSY;
	if (p->cku_flags & CKU_BUFWANTED) {
		RPCLOG(2, "buffree: (client) waking sleepers p %x\n", p);
		p->cku_flags &= ~CKU_BUFWANTED;
		rcstat.rcbufulocks++;
		wakeprocs((caddr_t)&p->cku_outbuf, PRMPT);
	}
}

/*
 * Create an rpc handle for a clts rpc connection.
 * Allocates space for the handle structure and the private data, and
 * opens a socket.  Note sockets and handles are one to one.
 */
/* ARGSUSED */
int
clnt_clts_kcreate(tiptr, rdev, addr, pgm, vers, sendsz, retrys, cred, cl)
	register TIUSER			*tiptr;
	dev_t				rdev;
	struct netbuf			*addr;
	u_long				pgm;
	u_long				vers;
	int				retrys;
	struct cred			*cred;
	u_int				sendsz;
	CLIENT				**cl;
{
	register CLIENT			*h;
	register struct cku_private	*p;
	struct rpc_msg			call_msg;
	int				error;

	RPCLOG(2, "clnt_clts_kcreate: pgm %d, ", pgm);
	RPCLOG(2, "vers %d, ", vers);
	RPCLOG(2, "retries %d\n", retrys);

	if (cl == NULL)
		return EINVAL;

	*cl = NULL;
	error = 0;

	p = (struct cku_private *)kmem_zalloc(sizeof(*p), KM_SLEEP);
	h = ptoh(p);

	/* handle */
	h->cl_ops = &udp_ops;
	h->cl_private = (caddr_t) p;
	h->cl_auth = authkern_create();

	/* call message, just used to pre-serialize below */
	call_msg.rm_xid = 0;
	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = pgm;
	call_msg.rm_call.cb_vers = vers;

	/* private */
	clnt_clts_init(h, addr, retrys, cred);

	if (tiptr->tp_info.tsdu > 0)
		sendsz = MIN(tiptr->tp_info.tsdu, CKU_MAXSIZE);
	else
		sendsz = CKU_MAXSIZE;
	RPCLOG(2, "clnt_clts_kcreate: sendsz %d\n", sendsz);

	p->cku_outbuflen = sendsz;
	p->cku_outbuf = (char *)kmem_alloc(sendsz, KM_SLEEP);
	xdrmem_create(&p->cku_outxdr, p->cku_outbuf, sendsz, XDR_ENCODE);

	/* pre-serialize call message header */
	if (! xdr_callhdr(&(p->cku_outxdr), &call_msg)) {
		printf("clnt_clts_kcreate - Fatal header serialization error.");
		error = EINVAL;		/* XXX */
		goto bad;
	}
	p->cku_outpos = XDR_GETPOS(&(p->cku_outxdr));
	p->cku_tiptr = tiptr; 
	p->cku_device = rdev;
	*cl = h;
	return (0);

bad:
	if (p->cku_outbuflen)
		kmem_free((caddr_t)p->cku_outbuf, p->cku_outbuflen);
	kmem_free((caddr_t)p, (u_int)sizeof (struct cku_private));

	RPCLOG(1, "clnt_clts_kcreate: create failed error %d\n", error);

	return (error);
}

void
clnt_clts_init(h, addr, retrys, cred)
	CLIENT			*h;
	struct netbuf		*addr;
	register int		retrys;
	struct cred		*cred;
{
	/* LINTED pointer alignment */
	struct cku_private	*p = htop(h);

	p->cku_retrys = retrys;

	if (p->cku_addr.maxlen < addr->len) {
		if (p->cku_addr.maxlen != 0 && p->cku_addr.buf != NULL)
			(void) kmem_free(p->cku_addr.buf, p->cku_addr.maxlen);

		p->cku_addr.buf = (char *)kmem_zalloc(addr->maxlen, KM_SLEEP);
		p->cku_addr.maxlen = addr->maxlen;
	}

	p->cku_addr.len = addr->len;
	RPCLOG(2, "clnt_clts_init: addr.len %d, ", addr->len);
	RPCLOG(2, "addr.maxlen %d\n", addr->maxlen);
	bcopy(addr->buf, p->cku_addr.buf, addr->len);

	p->cku_cred = cred;
	p->cku_xid = 0;
	p->cku_flags &= (CKU_BUFBUSY | CKU_BUFWANTED);
	p->cku_flags |= CKU_LOANEDBUF;
}

/*
 * set the timers.  Return current retransmission timeout.
 */
clnt_clts_settimers(h, t, all, minimum, feedback, arg)
	CLIENT			*h;
	struct rpc_timers	*t;
	struct rpc_timers	*all;
	unsigned int		minimum;
	void			(*feedback)();
	caddr_t			arg;
{
	/* LINTED pointer alignment */
	struct cku_private	*p = htop(h);
	int value;

	p->cku_feedback = feedback;
	p->cku_feedarg = arg;
	p->cku_timers = t;
	p->cku_timeall = all;
	value = all->rt_rtxcur;
	value += t->rt_rtxcur;
	if (value < minimum)
		return(minimum);
	rcstat.rctimers++;
	return(value);
}

/*
 * Time out back off function. tim is in HZ
 */
#define MAXTIMO	(20 * HZ)
#define backoff(tim)	((((tim) << 1) > MAXTIMO) ? MAXTIMO : ((tim) << 1))

STATIC int retry_poll_timo = 30*HZ;

/*
 * Call remote procedure.
 * Most of the work of rpc is done here.  We serialize what is left
 * of the header (some was pre-serialized in the handle), serialize
 * the arguments, and send it off.  We wait for a reply or a time out.
 * Timeout causes an immediate return, other packet problems may cause
 * a retry on the receive.  When a good packet is received we deserialize
 * it, and check verification.  A bad reply code will cause one retry
 * with full (longhand) credentials.
 */

enum clnt_stat
clnt_clts_kcallit_addr(h, procnum, xdr_args, argsp, xdr_results, resultsp, wait,
		sin)
	register CLIENT			*h;
	u_long				procnum;
	xdrproc_t			xdr_args;
	caddr_t				argsp;
	xdrproc_t			xdr_results;
	caddr_t				resultsp;
	struct timeval			wait;
	struct netbuf			*sin;
{
	/* LINTED pointer alignment */
	register struct cku_private	*p = htop(h);
	register XDR			*xdrs;
	register TIUSER			*tiptr = p->cku_tiptr;
	int				rtries;
	int				stries = p->cku_retrys;
	int				s;
	struct cred			*tmpcred;
	int				timohz;
	int				ret;
	u_long				xid;
	u_int				rempos = 0;
	int				refreshes = 2;	/* number of times
						 to refresh credential */
	int				round_trip;	/* time the RPC */
	struct t_kunitdata		*unitdata;
	int				type;
	int				uderr;
	frtn_t				*cku_frtn;
	int				error;

/*
# define time_in_hz (hrestime.tv_sec*hz + hrestime.tv_usec/(1000000/hz))
*/
#define time_in_hz lbolt

	RPCLOG(2, "clnt_clts_kcallit_addr entered\n", 0);

	rcstat.rccalls++;

	s = splstr();
	while (p->cku_flags & CKU_BUSY) {
		RPCLOG(2, "clnt_clts_kcallit_addr: pid %d cku busy - sleeping\n", u.u_procp->p_pid);
		rcstat.rcwaits++;
		p->cku_flags |= CKU_WANTED;
		(void) sleep((caddr_t)h, PZERO-2);
	}
	p->cku_flags |= CKU_BUSY;
	(void)splx(s);
	RPCLOG(2, "clnt_clts_kcallit_addr: pid %d cku not busy\n", u.u_procp->p_pid);

	/*
	 * Set credentials into the u structure
	 */
	tmpcred = u.u_procp->p_cred;
	u.u_procp->p_cred = p->cku_cred;

	if (p->cku_xid == 0)
		xid = p->cku_xid = clnt_clts_getxid();
	else
		xid = p->cku_xid;

	/*
	 * This is dumb but easy: keep the time out in units of hz
	 * so it is easy to call timeout and modify the value.
	 */
	timohz = wait.tv_sec * HZ + (wait.tv_usec * HZ) / 1000000;

call_again:

	/*
	 * Wait til buffer gets freed then make a type 2 mbuf point at it
	 * The buffree routine clears CKU_BUFBUSY and does a wakeup when
	 * the mbuf gets freed.
	 */
	s = splstr();
	while (p->cku_flags & CKU_BUFBUSY) {
		RPCLOG(2, "clnt_clts_kcallit_addr: pid %d loaned buffer busy\n", u.u_procp->p_pid);
		p->cku_flags |= CKU_BUFWANTED;
		(void)sleep((caddr_t)&p->cku_outbuf, PZERO-3);
	 }
	 p->cku_flags |= CKU_BUFBUSY;
	 (void) splx(s);
	RPCLOG(2, "clnt_clts_kcallit_addr: pid %d loaned buffer not busy\n", u.u_procp->p_pid);

	xdrs = &p->cku_outxdr;
	/*
	 * The transaction id is the first thing in the
	 * preserialized output buffer.
	 */
	/* LINTED pointer alignment */
	(*(u_long *)(p->cku_outbuf)) = xid;

	xdrmem_create(xdrs, p->cku_outbuf, p->cku_outbuflen, XDR_ENCODE);

	if (rempos != 0) {
		XDR_SETPOS(xdrs, rempos);
	} else {
		/*
		 * Serialize dynamic stuff into the output buffer.
		 */
		XDR_SETPOS(xdrs, p->cku_outpos);
		if ((! XDR_PUTLONG(xdrs, (long *)&procnum)) ||
		    (! AUTH_MARSHALL(h->cl_auth, xdrs)) ||
		    (! (*xdr_args)(xdrs, argsp))) {
			p->cku_err.re_status = RPC_CANTENCODEARGS;
			p->cku_err.re_errno = EIO;
			goto done;
		}
		rempos = XDR_GETPOS(xdrs);
	}

	round_trip = time_in_hz;
	if ((error = t_kalloc(tiptr, T_UNITDATA, T_UDATA|T_ADDR,
						(char **)&unitdata)) != 0) {
		rcstat.rcnomem++;
		buffree(p);
		goto done;
	}
	
	RPCLOG(2, "clnt_clts_kcallit_addr: addr.maxlen %d\n", unitdata->addr.maxlen);
	RPCLOG(2, "clnt_clts_kcallit_addr: cku_addr.len %d\n", p->cku_addr.len);
	bcopy(p->cku_addr.buf, unitdata->addr.buf, p->cku_addr.len);
	unitdata->addr.len = p->cku_addr.len;
 
	unitdata->udata.buf = p->cku_outbuf;
	unitdata->udata.maxlen = p->cku_outbuflen;
	unitdata->udata.len = rempos;

	if (p->cku_flags & CKU_LOANEDBUF) {
		p->cku_frtn.free_func = buffree;
		p->cku_frtn.free_arg = (char *)p;
		cku_frtn = &p->cku_frtn;
	}
	else	cku_frtn = NULL;
 
	if ((error = t_ksndudata(tiptr, unitdata, cku_frtn)) != 0) {
		p->cku_err.re_status = RPC_CANTSEND;
		p->cku_err.re_errno = error;

		RPCLOG(1, "clnt_clts_kcallit_addr: t_ksndudata: error %d\n",error);

		(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
		rcstat.rccantsend++;
		buffree(p);
		goto done;
	}
	(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);

	/* If the rpc user did not use a loaned buffer then
	 * we can reset the buffer busy flag.
	 */
	if ((p->cku_flags & CKU_LOANEDBUF) == 0)
		buffree(p);

tryread:

	for (rtries = RECVTRIES; rtries; rtries--) {
		if ((error = t_kalloc(tiptr, T_UNITDATA, T_UDATA|T_ADDR,
						 (char **)&unitdata)) != 0)
			goto done;

		RPCLOG(2, "clnt_clts_kcallit_addr (pid %d): ", u.u_procp->p_pid);
		RPCLOG(2, "calling t_kspoll (timeout %x)\n", timohz);
		if ((error = t_kspoll(tiptr, timohz, READWAIT, &ret)) != 0) {
			if (error == EINTR) {
				RPCLOG(1, "clnt_clts_kcallit_addr (pid %d): t_kspoll interrupted\n", u.u_procp->p_pid);
				p->cku_err.re_status = RPC_INTR;
				p->cku_err.re_errno = EINTR;
				(void)t_kfree(tiptr, (char *)unitdata,
								T_UNITDATA);
				goto done;
			}
			RPCLOG(1, "clnt_clts_kcallit_addr (pid %d): ", u.u_procp->p_pid);
			RPCLOG(1, "t_kspoll: error %d\n", error);
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			continue;       /* is this correct? */
		}
		if (ret == 0) {
			RPCLOG(1, "clnt_clts_kcallit_addr (pid %d): t_kspoll timed out\n", u.u_procp->p_pid);
			p->cku_err.re_status = RPC_TIMEDOUT;
			p->cku_err.re_errno = ETIMEDOUT;
			rcstat.rctimeouts++;
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			goto done;
		}

		/* something waiting, so read it in
		 */
		if ((error = t_krcvudata(tiptr, unitdata, &type, &uderr)) != 0) {
			p->cku_err.re_errno = error;
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			goto done;
		}
		if (type != T_DATA) {
			(void)t_kfree(tiptr, (char *)unitdata, T_UNITDATA);
			continue;
		}
		if (sin) {
			bcopy(unitdata->addr.buf, sin->buf, unitdata->addr.len);
			sin->len = unitdata->addr.len;
		}
		p->cku_inudata = unitdata;
 
		p->cku_inbuf = unitdata->udata.buf;

		if (p->cku_inudata->udata.len < sizeof (u_long)) {
			RPCLOG(1, "clnt_clts_kcallit_addr: len too small %d\n", p->cku_inudata->udata.len);
			(void)t_kfree(tiptr, (char *)p->cku_inudata, T_UNITDATA);
			continue;
		}

		/*
		 * If reply transaction id matches id sent
		 * we have a good packet.
		 */
		/* LINTED pointer alignment */
		if (*((u_long *)(p->cku_inbuf)) != *((u_long *)(p->cku_outbuf))) {
			rcstat.rcbadxids++;
			(void)t_kfree(tiptr, (char *)p->cku_inudata,
								 T_UNITDATA);
			continue;
		}
		break;
	}

	if (rtries == 0) {
		p->cku_err.re_status = RPC_CANTRECV;
		p->cku_err.re_errno = EIO;
		goto done;
	}

	round_trip = time_in_hz - round_trip;
	/*
	 * Van Jacobson timer algorithm here, only if NOT a retransmission.
	 */
	if (p->cku_timers != (struct rpc_timers *)0 &&
	    stries == p->cku_retrys) {
		register int rt;

		rt = round_trip;
		rt -= (p->cku_timers->rt_srtt >> 3);
		p->cku_timers->rt_srtt += rt;
		if (rt < 0)
			rt = - rt;
		rt -= (p->cku_timers->rt_deviate >> 2);
		p->cku_timers->rt_deviate += rt;
		p->cku_timers->rt_rtxcur = 
			(u_long)((p->cku_timers->rt_srtt >> 2) +
			  p->cku_timers->rt_deviate) >> 1;

		rt = round_trip;
		rt -= (p->cku_timeall->rt_srtt >> 3);
		p->cku_timeall->rt_srtt += rt;
		if (rt < 0)
			rt = - rt;
		rt -= (p->cku_timeall->rt_deviate >> 2);
		p->cku_timeall->rt_deviate += rt;
		p->cku_timeall->rt_rtxcur = 
			(u_long)((p->cku_timeall->rt_srtt >> 2) + 
			  p->cku_timeall->rt_deviate) >> 1;
		if (p->cku_feedback != (void (*)()) 0)
		    (*p->cku_feedback)(FEEDBACK_OK, procnum, p->cku_feedarg);
	}

	/*
	 * Process reply
	 */

	xdrs = &(p->cku_inxdr);
	xdrmblk_init(xdrs, unitdata->udata.udata_mp, XDR_DECODE);

	{
		/*
		 * Declare this variable here to have smaller
		 * demand for stack space in this procedure.
		 */
		struct rpc_msg		   reply_msg;

		reply_msg.acpted_rply.ar_verf = _null_auth;
		reply_msg.acpted_rply.ar_results.where = resultsp;
		reply_msg.acpted_rply.ar_results.proc = xdr_results;

		/*
		 * Decode and validate the response.
		 */
		if (xdr_replymsg(xdrs, &reply_msg)) {
			_seterr_reply(&reply_msg, &(p->cku_err));

			if (p->cku_err.re_status == RPC_SUCCESS) {
				/*
				 * Reply is good, check auth.
				 */
				if (! AUTH_VALIDATE(h->cl_auth,
				    &reply_msg.acpted_rply.ar_verf)) {
					p->cku_err.re_status = RPC_AUTHERROR;
					p->cku_err.re_why = AUTH_INVALIDRESP;
					rcstat.rcbadverfs++;
					/*
					 * See if another message is here. If so,
					 * maybe it is the right response.
					 */
					(void)t_kspoll(tiptr, retry_poll_timo, READWAIT, &ret);
					if (ret != 0) {
						RPCLOG(1,
		"clnt_clts_kcallit_addr (pid %d): validation failure: found another message\n", u.u_procp->p_pid);
						(void)t_kfree(tiptr,
							(char *)p->cku_inudata,
							T_UNITDATA);   
						p->cku_inudata = NULL;
						goto tryread;
					} else
						RPCLOG(1,
		"clnt_clts_kcallit_addr (pid %d): validation failure: no messages waiting\n", u.u_procp->p_pid);
				}
				if (reply_msg.acpted_rply.ar_verf.oa_base !=
				    NULL) {
					/* free auth handle */
					xdrs->x_op = XDR_FREE;
					(void) xdr_opaque_auth(xdrs,
					    &(reply_msg.acpted_rply.ar_verf));
				}
			} else {
				/*
				 * Maybe our credential needs refreshed
				 */
				if (refreshes > 0 && AUTH_REFRESH(h->cl_auth)) {
					refreshes--;
					rcstat.rcnewcreds++;
					rempos = 0;
				}
			}
		} else {
			/* probably buffree() wasn't called */
			buffree(p);
			p->cku_err.re_status = RPC_CANTDECODERES;
			p->cku_err.re_errno = EIO;
		}
	}

	(void)t_kfree(tiptr, (char *)p->cku_inudata, T_UNITDATA);   
	p->cku_inudata = NULL;

	RPCLOG(2, "clnt_clts_kcallit_addr done\n", 0);

done:
	if ((p->cku_err.re_status != RPC_SUCCESS) &&
	    (p->cku_err.re_status != RPC_INTR) &&
	    (p->cku_err.re_status != RPC_CANTENCODEARGS)) {
		if (p->cku_feedback != (void (*)()) 0 &&
		    stries == p->cku_retrys)
			(*p->cku_feedback)(FEEDBACK_REXMIT1, 
				procnum, p->cku_feedarg);
		timohz = backoff(timohz);
		if (p->cku_timeall != (struct rpc_timers *)0)
			p->cku_timeall->rt_rtxcur = timohz;
		if (p->cku_err.re_status == RPC_SYSTEMERROR ||
		    p->cku_err.re_status == RPC_CANTSEND) {
			/*
			 * Errors due to lack of resources, wait a bit
			 * and try again.
			 */
			(void) delay(10);
			/* (void) sleep((caddr_t)&lbolt, PZERO-4); */
		}
		if (--stries > 0) {
			rcstat.rcretrans++;
			goto call_again;
		}
	}
	u.u_procp->p_cred = tmpcred;
	/*
	 * Insure that buffer is not busy prior to releasing client handle.
	 */
	s = splstr();
	while (p->cku_flags & CKU_BUFBUSY) {
		RPCLOG(2, "clnt_clts_kcallit_addr: pid %d loaned buffer busy - sleeping\n", u.u_procp->p_pid);
		p->cku_flags |= CKU_BUFWANTED;
		(void)sleep((caddr_t)&p->cku_outbuf, PZERO-3);
	}
	(void) splx(s);

	RPCLOG(2, "clnt_clts_kcallit_addr: pid %d loaned buffer not busy\n", u.u_procp->p_pid);
	p->cku_flags &= ~CKU_BUSY;
	if (p->cku_flags & CKU_WANTED) {
		p->cku_flags &= ~CKU_WANTED;
		wakeprocs((caddr_t)h, PRMPT);
	}
	if (p->cku_err.re_status != RPC_SUCCESS) {
		rcstat.rcbadcalls++;
	}
	return (p->cku_err.re_status);
}

enum clnt_stat
clnt_clts_kcallit(h, procnum, xdr_args, argsp, xdr_results, resultsp, wait)
	register CLIENT		*h;
	register u_long		procnum;
	register xdrproc_t	xdr_args;
	register caddr_t	argsp;
	register xdrproc_t	xdr_results;
	register caddr_t	resultsp;
	struct timeval		wait;
{
	return (clnt_clts_kcallit_addr(h, procnum, xdr_args, argsp, xdr_results,
		resultsp, wait, (struct netbuf *)0));
}


/*
 * Return error info on this handle.
 */
void
clnt_clts_kerror(h, err)
	register CLIENT			*h;
	register struct rpc_err		*err;
{
	/* LINTED pointer alignment */
	register struct cku_private	*p = htop(h);

	*err = p->cku_err;
}

STATIC bool_t
clnt_clts_kfreeres(cl, xdr_res, res_ptr)
	register CLIENT			*cl;
	register xdrproc_t		xdr_res;
	register caddr_t		res_ptr;
{
	/* LINTED pointer alignment */
	register struct cku_private	*p = (struct cku_private *)cl->cl_private;
	register XDR			*xdrs = &(p->cku_outxdr);

	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr));
}

void
clnt_clts_kabort()
{
}

bool_t
clnt_clts_kcontrol(h, cmd, arg)
	register CLIENT			*h;
{
	register struct cku_private	*p = htop(h);

	switch(cmd) {
	case CKU_LOANEDBUF:
		/* Use a loaned buffer or not.
		 */
		if (arg)
			p->cku_flags |= CKU_LOANEDBUF;
		else	p->cku_flags &= ~CKU_LOANEDBUF;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


/*
 * Destroy rpc handle.
 * Frees the space used for output buffer, private data, and handle
 * structure, and the file pointer/TLI data on last reference.
 */
void
clnt_clts_kdestroy(h)
	register CLIENT			*h;
{
	/* LINTED pointer alignment */
	register struct cku_private	*p = htop(h);
	register TIUSER			*tiptr;

	RPCLOG(2, "clnt_clts_kdestroy %x\n", h);

	tiptr = p->cku_tiptr;
	kmem_free((caddr_t)p->cku_outbuf, p->cku_outbuflen);
	kmem_free((caddr_t)p->cku_addr.buf, p->cku_addr.maxlen);
	kmem_free((caddr_t)p, sizeof (*p));

	(void)t_kclose(tiptr, 1);
}

/*
 * Ensure that the client handle's transport is opened over the transport
 * provider we expect.
 */
void
clnt_clts_reopen(h, kncp)
	register CLIENT			*h;
	struct knetconfig		*kncp;
{
	/* LINTED pointer alignment */
	register struct cku_private	*p = htop(h);
	struct cred			*tmpcred;
	struct cred			*savecred;
	struct rpc_msg			call_msg;
	int				error;
	int				sendsz;

	if (p->cku_device != kncp->knc_rdev) {
		/* first close the old transport */
		while ((error = t_kclose(p->cku_tiptr, 1)) != 0) {
			RPCLOG(1, "clnt_clts_reopen: t_kclose: error %d\n", error);
			(void)delay(HZ);
		}

		/* Now open a new one, as root */
		do {
			tmpcred = crdup(u.u_cred);
			savecred = u.u_cred;
			u.u_cred = tmpcred;
			u.u_cred->cr_uid = 0;
			error = t_kopen(NULL, kncp->knc_rdev, FREAD|FWRITE|FNDELAY,
				&p->cku_tiptr);
			u.u_cred = savecred;
			crfree(tmpcred);
			if (error) {
				RPCLOG(1, "clnt_clts_reopen: t_kopen: error %d\n", error);
				(void)delay(HZ);
			}
		} while (error);

		/* Now bind the new transport to an address */
		if (strcmp(kncp->knc_protofmly, NC_INET) == 0) {
			while ((error = bindresvport(p->cku_tiptr)) != 0) {
				RPCLOG(1, "clnt_clts_reopen: bindresvport failed: error %d\n", error);
				(void)delay(HZ);
			}
		}
		else {
			while ((error = t_kbind(p->cku_tiptr, NULL, NULL)) != 0) {
				RPCLOG(1, "clnt_clts_reopen: t_kbind: %d\n", error);
				(void)delay(HZ);
			}
		}

		p->cku_device = kncp->knc_rdev;

		if (p->cku_tiptr->tp_info.tsdu > 0)
			sendsz = MIN(p->cku_tiptr->tp_info.tsdu, CKU_MAXSIZE);
		else
			sendsz = CKU_MAXSIZE;

		/* reallocate the output buffer if necessary */
		if (p->cku_outbuf && p->cku_outbuflen &&
		    (p->cku_outbuflen < sendsz)) {

			/* call message, just used to pre-serialize below */
			call_msg.rm_xid = 0;
			call_msg.rm_direction = CALL;
			call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
			/* LINTED pointer alignment */
			call_msg.rm_call.cb_prog = *((u_long *)p->cku_outbuf+3);
			/* LINTED pointer alignment */
			call_msg.rm_call.cb_vers = *((u_long *)p->cku_outbuf+4);

			(void)kmem_free(p->cku_outbuf, p->cku_outbuflen);
			p->cku_outbuflen = sendsz;
			p->cku_outbuf = kmem_alloc(p->cku_outbuflen, KM_SLEEP);

			xdrmem_create(&p->cku_outxdr, p->cku_outbuf,
					p->cku_outbuflen, XDR_ENCODE);

			/* pre-serialize call message header */
			while (! xdr_callhdr(&(p->cku_outxdr), &call_msg)) {
				RPCLOG(1, "clnt_clts_reopen - header serialization error.", 0);
				(void)delay(HZ);
			}
			p->cku_outpos = XDR_GETPOS(&(p->cku_outxdr));
		}
		return;
	}
}
#endif
