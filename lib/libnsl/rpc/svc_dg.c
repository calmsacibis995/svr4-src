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


#ident	"@(#)librpc:svc_dg.c	1.4.2.1"

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
 * svc_dg.c, Server side for connectionless RPC.
 *
 * Does some caching in the hopes of achieving execute-at-most-once semantics.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <errno.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define	LOG_ERR 3
#endif /* SYSLOG */

#ifndef MAX
#define	MAX(a, b)	(((a) > (b)) ? (a) : (b))
#endif

static struct xp_ops *svc_dg_ops();

extern char *malloc();
extern int errno, t_errno;

#define	MAX_OPT_WORDS	32

/*
 * kept in xprt->xp_p2
 */
struct svc_dg_data {
	struct	netbuf optbuf;
	long	opts[MAX_OPT_WORDS];		/* options */
	u_int   su_iosz;			/* size of send.recv buffer */
	u_long	su_xid;				/* transaction id */
	XDR	su_xdrs;			/* XDR handle */
	char	su_verfbody[MAX_AUTH_BYTES];	/* verifier body */
	char 	*su_cache;			/* cached data, NULL if none */
};
#define	su_data(xprt)	((struct svc_dg_data *)(xprt->xp_p2))
#define	rpc_buffer(xprt) ((xprt)->xp_p1)

/*
 * Usage:
 *	xprt = svc_dg_create(sock, sendsize, recvsize);
 * Does other connectionless specific initializations.
 * Once *xprt is initialized, it is registered.
 * see (svc.h, xprt_register). If recvsize or sendsize are 0 suitable
 * system defaults are chosen.
 * The routines returns NULL if a problem occurred.
 */
SVCXPRT *
svc_dg_create(fd, sendsize, recvsize)
	register int fd;
	u_int sendsize;
	u_int recvsize;
{
	register SVCXPRT *xprt;
	register struct svc_dg_data *su = NULL;
	struct t_info tinfo;

	if (t_getinfo(fd, &tinfo) == -1) {
		if ((sendsize == 0) || (recvsize == 0)) {
			syslog(LOG_ERR,
		"svc_dg_create: could not get transport information");
			return ((SVCXPRT *)NULL);
		}
	} else {
		/*
		 * Find the receive and the send size
		 */
		sendsize = _rpc_get_t_size((int)sendsize, tinfo.tsdu);
		recvsize = _rpc_get_t_size((int)recvsize, tinfo.tsdu);
	}

	xprt = (SVCXPRT *)mem_alloc(sizeof (SVCXPRT));
	if (xprt == NULL)
		goto freedata;
	memset((char *)xprt, 0, sizeof (SVCXPRT));

	su = (struct svc_dg_data *)mem_alloc(sizeof (*su));
	if (su == NULL)
		goto freedata;
	su->su_iosz = ((MAX(sendsize, recvsize) + 3) / 4) * 4;
	if ((rpc_buffer(xprt) = mem_alloc(su->su_iosz)) == NULL)
		goto freedata;
	xdrmem_create(&(su->su_xdrs), rpc_buffer(xprt), su->su_iosz,
		XDR_DECODE);
	su->su_cache = NULL;
	xprt->xp_fd = fd;
	xprt->xp_p2 = (caddr_t)su;
	xprt->xp_p3 = NULL;
	xprt->xp_verf.oa_base = su->su_verfbody;
	xprt->xp_ops = svc_dg_ops();
	xprt_register(xprt);
	return (xprt);
freedata:
	(void) syslog(LOG_ERR, "svc_dg_create: out of memory");
	if (xprt) {
		if (su)
			(void) mem_free((char *) su, sizeof (*su));
		(void) mem_free((char *)xprt, sizeof (SVCXPRT));
	}
	return ((SVCXPRT *)NULL);
}

static enum xprt_stat
svc_dg_stat(xprt)
	SVCXPRT *xprt;
{
	return (XPRT_IDLE);
}

static bool_t
svc_dg_recv(xprt, msg)
	register SVCXPRT *xprt;
	struct rpc_msg *msg;
{
	register struct svc_dg_data *su = su_data(xprt);
	register XDR *xdrs = &(su->su_xdrs);
	char *reply;
	u_long replylen;
	struct t_unitdata tu_data;
	int moreflag;		/* Flag indicating more data */
	int lookres;

	tu_data.addr = xprt->xp_rtaddr;
	tu_data.udata.buf = (char *)rpc_buffer(xprt);
	tu_data.opt.buf= (char *) su->opts;
	/*
	 * If moreflag is set, drop that data packet. Something wrong
	 */
again:
	tu_data.addr.len = 0;
	tu_data.opt.len  = 0;
	tu_data.udata.len  = 0;

	tu_data.udata.maxlen = su->su_iosz;
	tu_data.opt.maxlen = MAX_OPT_WORDS <<2;

	moreflag = 0;

	if (t_rcvudata(xprt->xp_fd, &tu_data, &moreflag) == -1) {
		syslog(LOG_ERR, "rcvudata t_errno=%d errno=%d\n",
				t_errno, errno);
		if (t_errno == TLOOK) {
			lookres = t_look(xprt->xp_fd);
			if ((lookres & T_UDERR) &&
				(t_rcvuderr(xprt->xp_fd,
				  (struct t_uderr *) 0) < 0)) {
				syslog(LOG_ERR,
				"t_rcvuderr terrno=%d\n", t_errno);
			}
			if (lookres & T_DATA)
				goto again;
		} else if ((errno == EINTR) && (t_errno == TSYSERR))
			goto again;
		else
			return (FALSE);
	}

	if ((moreflag) || (tu_data.udata.len < 4 * sizeof (u_long)))
		return (FALSE);
	su->optbuf= tu_data.opt;
	xprt->xp_rtaddr.len = tu_data.addr.len;
	xdrs->x_op = XDR_DECODE;
	XDR_SETPOS(xdrs, 0);
	if (! xdr_callmsg(xdrs, msg))
		return (FALSE);
	su->su_xid = msg->rm_xid;
	if (su->su_cache != NULL) {
		if (cache_get(xprt, msg, &reply, &replylen)) {
			tu_data.addr = xprt->xp_rtaddr;
			tu_data.udata.buf = reply;
			tu_data.udata.len = (u_int)replylen;
			tu_data.opt.len = 0;
			(void) t_sndudata(xprt->xp_fd, &tu_data);
			return (TRUE);
		}
	}
	return (TRUE);
}

static bool_t
svc_dg_reply(xprt, msg)
	register SVCXPRT *xprt;
	struct rpc_msg *msg;
{
	register struct svc_dg_data *su = su_data(xprt);
	register XDR *xdrs = &(su->su_xdrs);
	register int slen;
	register bool_t stat = FALSE;
	struct t_unitdata tu_data;

	xdrs->x_op = XDR_ENCODE;
	XDR_SETPOS(xdrs, 0);
	msg->rm_xid = su->su_xid;
	if (xdr_replymsg(xdrs, msg)) {
		slen = (int)XDR_GETPOS(xdrs);
		tu_data.addr = xprt->xp_rtaddr;
		tu_data.udata.buf = rpc_buffer(xprt);
		tu_data.udata.len = slen;
		tu_data.opt.len = 0;
		if (t_sndudata(xprt->xp_fd, &tu_data) == 0) {
			stat = TRUE;
			if (su->su_cache && slen >= 0) {
				cache_set(xprt, (u_long) slen);
			}
		} else {
			syslog(LOG_ERR,
			"t_sndudata error t_errno=%d errno=%d\n",
				t_errno, errno);
		}
	}
	return (stat);
}

static bool_t
svc_dg_getargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{
	return ((*xdr_args)(&(su_data(xprt)->su_xdrs), args_ptr));
}

static bool_t
svc_dg_freeargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{
	register XDR *xdrs = &(su_data(xprt)->su_xdrs);

	xdrs->x_op = XDR_FREE;
	return ((*xdr_args)(xdrs, args_ptr));
}

static void
svc_dg_destroy(xprt)
	register SVCXPRT *xprt;
{
	register struct svc_dg_data *su = su_data(xprt);

	xprt_unregister(xprt);
	(void) t_close(xprt->xp_fd);
	XDR_DESTROY(&(su->su_xdrs));
	(void) mem_free(rpc_buffer(xprt), su->su_iosz);
	(void) mem_free((caddr_t)su, sizeof (*su));
	if (xprt->xp_rtaddr.buf)
		(void) mem_free(xprt->xp_rtaddr.buf, xprt->xp_rtaddr.maxlen);
	if (xprt->xp_ltaddr.buf)
		(void) mem_free(xprt->xp_ltaddr.buf, xprt->xp_ltaddr.maxlen);
	if (xprt->xp_tp)
		(void) free(xprt->xp_tp);
	(void) mem_free((caddr_t)xprt, sizeof (SVCXPRT));
}

static struct xp_ops *
svc_dg_ops()
{
	static struct xp_ops ops;

	if (ops.xp_recv == NULL) {
		ops.xp_recv = svc_dg_recv;
		ops.xp_stat = svc_dg_stat;
		ops.xp_getargs = svc_dg_getargs;
		ops.xp_reply = svc_dg_reply;
		ops.xp_freeargs = svc_dg_freeargs;
		ops.xp_destroy = svc_dg_destroy;
	}
	return (&ops);
}

/*********************************************************************/

/*
 * Could have been a separate file, but some part of it depends upon the
 * private structure of the client handle.
 *
 * Fifo cache for cl server
 * Copies pointers to reply buffers into fifo cache
 * Buffers are sent again if retransmissions are detected.
 */

#define	SPARSENESS 4	/* 75% sparse */

#define	ALLOC(type, size)	\
	(type *) mem_alloc((unsigned) (sizeof (type) * (size)))

#define	MEMZERO(addr, type, size)	 \
	(void) memset((char *) (addr), 0, sizeof (type) * (int) (size))

#define	FREE(addr, type, size)	\
	(type *) mem_free((char *) (addr), (sizeof (type) * (size)))

/*
 * An entry in the cache
 */
typedef struct cache_node *cache_ptr;
struct cache_node {
	/*
	 * Index into cache is xid, proc, vers, prog and address
	 */
	u_long cache_xid;
	u_long cache_proc;
	u_long cache_vers;
	u_long cache_prog;
	struct netbuf cache_addr;
	/*
	 * The cached reply and length
	 */
	char *cache_reply;
	u_long cache_replylen;
	/*
	 * Next node on the list, if there is a collision
	 */
	cache_ptr cache_next;
};

/*
 * The entire cache
 */
struct cl_cache {
	u_long uc_size;		/* size of cache */
	cache_ptr *uc_entries;	/* hash table of entries in cache */
	cache_ptr *uc_fifo;	/* fifo list of entries in cache */
	u_long uc_nextvictim;	/* points to next victim in fifo list */
	u_long uc_prog;		/* saved program number */
	u_long uc_vers;		/* saved version number */
	u_long uc_proc;		/* saved procedure number */
	struct netbuf uc_addr;	/* saved caller's address */
};


/*
 * the hashing function
 */
#define	CACHE_LOC(transp, xid)	\
 (xid % (SPARSENESS*((struct cl_cache *) su_data(transp)->su_cache)->uc_size))
/*
 * Enable use of the cache. Returns 1 on success, 0 on failure.
 * Note: there is no disable.
 */
int
svc_dg_enablecache(transp, size)
	SVCXPRT *transp;
	u_long size;
{
	struct svc_dg_data *su = su_data(transp);
	struct cl_cache *uc;

	if (su->su_cache != NULL) {
		(void) syslog(LOG_ERR, "enablecache: cache already enabled");
		return (0);
	}
	uc = ALLOC(struct cl_cache, 1);
	if (uc == NULL) {
		(void) syslog(LOG_ERR, "enablecache: could not allocate cache");
		return (0);
	}
	uc->uc_size = size;
	uc->uc_nextvictim = 0;
	uc->uc_entries = ALLOC(cache_ptr, size * SPARSENESS);
	if (uc->uc_entries == NULL) {
		(void) syslog(LOG_ERR,
			"enablecache: could not allocate cache data");
		FREE(uc, struct cl_cache, 1);
		return (0);
	}
	MEMZERO(uc->uc_entries, cache_ptr, size * SPARSENESS);
	uc->uc_fifo = ALLOC(cache_ptr, size);
	if (uc->uc_fifo == NULL) {
		(void) syslog(LOG_ERR,
			"enablecache: could not allocate cache fifo");
		FREE(uc->uc_entries, cache_ptr, size * SPARSENESS);
		FREE(uc, struct cl_cache, 1);
		return (0);
	}
	MEMZERO(uc->uc_fifo, cache_ptr, size);
	su->su_cache = (char *) uc;
	return (1);
}

/*
 * Set an entry in the cache
 */
static
cache_set(xprt, replylen)
	SVCXPRT *xprt;
	u_long replylen;
{
	register cache_ptr victim;
	register cache_ptr *vicp;
	register struct svc_dg_data *su = su_data(xprt);
	struct cl_cache *uc = (struct cl_cache *) su->su_cache;
	u_int loc;
	char *newbuf;

	/*
	 * Find space for the new entry, either by
	 * reusing an old entry, or by mallocing a new one
	 */
	victim = uc->uc_fifo[uc->uc_nextvictim];
	if (victim != NULL) {
		loc = CACHE_LOC(xprt, victim->cache_xid);
		for (vicp = &uc->uc_entries[loc];
			*vicp != NULL && *vicp != victim;
			vicp = &(*vicp)->cache_next)
			;
		if (*vicp == NULL) {
			(void) syslog(LOG_ERR, "cache_set: victim not found");
			return;
		}
		*vicp = victim->cache_next;	/* remote from cache */
		newbuf = victim->cache_reply;
	} else {
		victim = ALLOC(struct cache_node, 1);
		if (victim == NULL) {
			(void) syslog(LOG_ERR,
				"cache_set: victim alloc failed");
			return;
		}
		newbuf = mem_alloc(su->su_iosz);
		if (newbuf == NULL) {
			(void) syslog(LOG_ERR,
			"cache_set: could not allocate new rpc_buffer");
			FREE(victim, struct cache_node, 1);
			return;
		}
	}

	/*
	 * Store it away
	 */
	victim->cache_replylen = replylen;
	victim->cache_reply = rpc_buffer(xprt);
	rpc_buffer(xprt) = newbuf;
	xdrmem_create(&(su->su_xdrs), rpc_buffer(xprt),
			su->su_iosz, XDR_ENCODE);
	victim->cache_xid = su->su_xid;
	victim->cache_proc = uc->uc_proc;
	victim->cache_vers = uc->uc_vers;
	victim->cache_prog = uc->uc_prog;
	victim->cache_addr = uc->uc_addr;
	victim->cache_addr.buf = ALLOC(char, uc->uc_addr.len);
	(void) memcpy(victim->cache_addr.buf, uc->uc_addr.buf,
			(int)uc->uc_addr.len);
	loc = CACHE_LOC(xprt, victim->cache_xid);
	victim->cache_next = uc->uc_entries[loc];
	uc->uc_entries[loc] = victim;
	uc->uc_fifo[uc->uc_nextvictim++] = victim;
	uc->uc_nextvictim %= uc->uc_size;
}

/*
 * Try to get an entry from the cache
 * return 1 if found, 0 if not found
 */
static int
cache_get(xprt, msg, replyp, replylenp)
	SVCXPRT *xprt;
	struct rpc_msg *msg;
	char **replyp;
	u_long *replylenp;
{
	u_int loc;
	register cache_ptr ent;
	register struct svc_dg_data *su = su_data(xprt);
	register struct cl_cache *uc = (struct cl_cache *) su->su_cache;

#define	EQADDR(a1, a2)	(memcmp((char*)&a1, (char*)&a2, sizeof (a1)) == 0)

	loc = CACHE_LOC(xprt, su->su_xid);
	for (ent = uc->uc_entries[loc]; ent != NULL; ent = ent->cache_next) {
		if (ent->cache_xid == su->su_xid &&
			ent->cache_proc == uc->uc_proc &&
			ent->cache_vers == uc->uc_vers &&
			ent->cache_prog == uc->uc_prog &&
			EQADDR(ent->cache_addr.buf, uc->uc_addr.buf)) {
			*replyp = ent->cache_reply;
			*replylenp = ent->cache_replylen;
			return (1);
		}
	}
	/*
	 * Failed to find entry
	 * Remember a few things so we can do a set later
	 */
	uc->uc_proc = msg->rm_call.cb_proc;
	uc->uc_vers = msg->rm_call.cb_vers;
	uc->uc_prog = msg->rm_call.cb_prog;
	uc->uc_addr = xprt->xp_rtaddr;
	uc->uc_addr.buf = ALLOC(char, uc->uc_addr.len);
	(void) memcpy(uc->uc_addr.buf, xprt->xp_rtaddr.buf,
			(int)uc->uc_addr.len);
	return (0);
}

