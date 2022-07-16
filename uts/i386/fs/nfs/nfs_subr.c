/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:nfs/nfs_subr.c	1.3.1.1"

/*	  @(#)nfs_subr.c 2.91 88/08/05 SMI	*/

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/cred.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/buf.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/socket.h>
/* #include <sys/socketvar.h> */
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/tiuser.h>
#include <sys/swap.h>
#include <sys/errno.h>
#include <sys/debug.h>
#include <sys/kmem.h>
#include <sys/cmn_err.h>

#include <netinet/in.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>

#include <nfs/nfs.h>
#include <nfs/nfs_clnt.h>
#include <nfs/rnode.h>
#include <vm/pvn.h>

#ifdef NFSDEBUG
extern int nfsdebug;
#endif
int nfsdprintf = 0;

extern struct vnodeops nfs_vnodeops;
STATIC struct rnode *rfind();
STATIC void	authfree();
STATIC void	clfree();
STATIC void	rp_addhash();
STATIC void	add_free();
STATIC void	rm_free();

extern int nrnode;		/* max rnodes to alloc, set in machdep.c */

/*
 * Client side utilities
 */

/*
 * client side statistics
 */
struct {
	uint	nclsleeps;		/* client handle waits */
	uint	nclgets;		/* client handle gets */
	uint	ncalls;			/* client requests */
	uint	nbadcalls;		/* rpc failures */
	uint	reqs[32];		/* count of each request */
} clstat;

int cltoomany = 0;

#define MAXCLIENTS	6
struct chtab {
	uint	ch_timesused;
	bool_t	ch_inuse;
	CLIENT	*ch_client;
} chtable[MAXCLIENTS];

u_int authdes_win = (60*60);	/* one hour -- should be mount option */

struct desauthent {
	struct mntinfo *da_mi;
	uid_t da_uid;
	short da_inuse;
	AUTH *da_auth;
} desauthtab[MAXCLIENTS];
int nextdesvictim;

struct unixauthent {
	short ua_inuse;
	AUTH *ua_auth;
} unixauthtab[MAXCLIENTS];
int nextunixvictim;

STATIC long
authget(mi, cr, ap)
	struct mntinfo *mi;
	struct cred *cr;
	AUTH **ap;
{
	int i;
	AUTH *auth;
	register struct unixauthent *ua;
	register struct desauthent *da;
	int authflavor;
	struct cred *savecred;
	long stat;

	RPCLOG(2, "authget(%x, ", mi);
	RPCLOG(2, "%x, ", cr);
	RPCLOG(2, "%x) ", ap);
	RPCLOG(2, "(pid %d) entered\n", u.u_procp->p_pid);

	if (ap == NULL)
		return (EINVAL);
	*ap = (AUTH *)NULL;

	authflavor = mi->mi_authflavor;
	for (;;) switch (authflavor) {
	case AUTH_NONE:
		/*
		 * XXX: should do real AUTH_NONE, instead of AUTH_UNIX
		 */
	case AUTH_UNIX:
		RPCLOG(2, "authget: AUTH_UNIX authentication\n", 0);
		i = MAXCLIENTS;
		do {
			ua = &unixauthtab[nextunixvictim++];
			nextunixvictim %= MAXCLIENTS;
		} while (ua->ua_inuse && --i > 0);

		if (ua->ua_inuse) {
			/* overflow of unix auths */
			*ap = authkern_create();
			return ((*ap != NULL) ? 0 : EINTR);	/* XXX */
		}

		if (ua->ua_auth == NULL) {
			ua->ua_auth = authkern_create();
		}
		ua->ua_inuse = 1;
		*ap = ua->ua_auth;
		return ((*ap != NULL) ? 0 : EINTR);	/* XXX */

	case AUTH_DES:
		RPCLOG(2, "authget: AUTH_DES authentication\n", 0);
		for (da = desauthtab; da < &desauthtab[MAXCLIENTS]; da++) {
			if (da->da_mi == mi && da->da_uid == cr->cr_uid &&
			    !da->da_inuse && da->da_auth != NULL) {
				RPCLOG(2, "authget: found in desauthtab\n", 0);
				da->da_inuse = 1;
				*ap = da->da_auth;
				return (0);
			} else if (da->da_mi != mi) {
				RPCLOG(2, "\tda->da_mi (%x)", da->da_mi);
				RPCLOG(2, " != mi (%x)\n", mi);
			} else if (da->da_uid != cr->cr_uid) {
				RPCLOG(2, "\tda->da_uid (%d)", da->da_uid);
				RPCLOG(2, " != cr->cr_uid (%d)\n", cr->cr_uid);
			} else if (da->da_inuse)
				RPCLOG(2, "\tda->da_inuse\n", 0);
			else if (da->da_auth == NULL)
				RPCLOG(2, "\tda->da_auth == NULL\n", 0);
		}
		RPCLOG (2, "authget: not in desauthtab, creating new auth\n", 0);
		savecred = u.u_cred;
		u.u_cred = cr;
		stat = authdes_create(mi->mi_netname, authdes_win,
					&mi->mi_syncaddr,
					mi->mi_knetconfig->knc_rdev,
					(des_block *)NULL,
					mi->mi_rpctimesync,
					&auth);
		u.u_cred = savecred;
		*ap = auth;

		if (stat != 0) {
			cmn_err(CE_WARN,
				"authget: authdes_create failure, stat %d\n",
				stat);
			return (stat);
			/* authflavor = AUTH_UNIX; */
			/* continue; */
		}

		i = MAXCLIENTS;
		do {
			da = &desauthtab[nextdesvictim++];
			nextdesvictim %= MAXCLIENTS;
		} while (da->da_inuse && --i > 0);

		if (da->da_inuse) {
			/* overflow of des auths */
			return (stat);
		}

		if (da->da_auth != NULL) {
			auth_destroy(da->da_auth);	/* should reuse!!! */
		}

		da->da_auth = auth;
		da->da_inuse = 1;
		da->da_uid = cr->cr_uid;
		da->da_mi = mi;
		return (stat);

	default:
		/*
		 * auth create must have failed, try AUTH_NONE
		 * (this relies on AUTH_NONE never failing)
		 */
		printf("authget: unknown authflavor %d\n", authflavor);
		authflavor = AUTH_NONE;
	}
}

STATIC void
authfree(auth)
	AUTH *auth;
{
	register struct unixauthent *ua;
	register struct desauthent *da;

	switch (auth->ah_cred.oa_flavor) {
	case AUTH_NONE: /* XXX: do real AUTH_NONE */
	case AUTH_UNIX:
		for (ua = unixauthtab; ua < &unixauthtab[MAXCLIENTS]; ua++) {
			if (ua->ua_auth == auth) {
				ua->ua_inuse = 0;
				return;
			}
		}
		auth_destroy(auth);	/* was overflow */
		break;
	case AUTH_DES:
		for (da = desauthtab; da < &desauthtab[MAXCLIENTS]; da++) {
			if (da->da_auth == auth) {
				da->da_inuse = 0;
				return;
			}
		}
		auth_destroy(auth);	/* was overflow */
		break;
	default:
		printf("authfree: unknown authflavor %d\n", auth->ah_cred.oa_flavor);
		break;
	}
}


STATIC long
clget(mi, cred, newcl)
	struct mntinfo *mi;
	struct cred *cred;
	CLIENT **newcl;
{
	register struct chtab *ch;
	int retrans;
	CLIENT *client;
	register int	error;

	if (newcl == NULL)
		return (EINVAL);
	*newcl = NULL;

	/*
	 * If soft mount and server is down just try once.
	 * Interruptable hard mounts get counted at this level.
	 */
	if ((!mi->mi_hard && mi->mi_down) || (mi->mi_int && mi->mi_hard)) {
		retrans = 1;
	} else {
		retrans = mi->mi_retrans;
	}

	/*
	 * Find an unused handle or create one
	 */
	clstat.nclgets++;
	for (ch = chtable; ch < &chtable[MAXCLIENTS]; ch++) {
		if (!ch->ch_inuse) {
			ch->ch_inuse = TRUE;
			if (ch->ch_client == NULL) {
				error =
				    clnt_tli_kcreate(mi->mi_knetconfig,
					 &mi->mi_addr, NFS_PROGRAM,
					 NFS_VERSION, 0,
					 retrans, cred, &ch->ch_client);
				if (error != 0) {
					cmn_err(CE_WARN,
			"clget: null client in chtable, ch=%x, error %d\n",
						(u_long)ch, error);
					return (error);
				}
				auth_destroy(ch->ch_client->cl_auth); /* XXX */
			} else {
				clnt_clts_reopen(ch->ch_client, mi->mi_knetconfig);
				clnt_clts_init(ch->ch_client,
				    &mi->mi_addr, retrans, cred);
			}
			error = authget(mi, cred, &ch->ch_client->cl_auth);
			if (error || ch->ch_client->cl_auth == NULL) {
				cmn_err(CE_WARN, "clget: authget failure in chtable, stat %d\n", error);
				(void)clnt_clts_kdestroy(ch->ch_client);
				ch->ch_client = NULL;
				return ((error != 0) ? error : EINTR);
			}
			ch->ch_timesused++;
			*newcl = ch->ch_client;
			return (0);
		}
	}

	/*
	 * If we got here there are no available handles
	 * To avoid deadlock, don't wait, but just grab another
	 */
	cltoomany++;
	error = clnt_tli_kcreate(mi->mi_knetconfig, &mi->mi_addr, NFS_PROGRAM,
			NFS_VERSION, 0, retrans, cred, &client);
	if (error != 0) {
		cmn_err(CE_WARN, "clget: clnt_tli_kcreate returned error %d\n",
					error);
		return (error);
	}
	auth_destroy(client->cl_auth);	 /* XXX */
	error = authget(mi, cred, &client->cl_auth);
	if (error || client->cl_auth == NULL) {
		cmn_err(CE_WARN, "clget: authget failure out of chtable, stat %d\n", error);
		(void) clnt_clts_kdestroy(client);
		return ((error != 0) ? error : EINTR);
	}
	*newcl = client;
	return (0);
}

STATIC void
clfree(cl)
	CLIENT *cl;
{
	register struct chtab *ch;

	authfree(cl->cl_auth);
	cl->cl_auth = NULL;
	for (ch = chtable; ch < &chtable[MAXCLIENTS]; ch++) {
		if (ch->ch_client == cl) {
			ch->ch_inuse = FALSE;
			return;
		}
	}
	/* destroy any extra allocated above MAXCLIENTS */
	CLNT_DESTROY(cl);
}

char *rfsnames[] = {
	"null", "getattr", "setattr", "unused", "lookup", "readlink", "read",
	"unused", "write", "create", "remove", "rename", "link", "symlink",
	"mkdir", "rmdir", "readdir", "fsstat" };

/*
 * This table maps from NFS protocol number into call type.
 * Zero means a "Lookup" type call
 * One  means a "Read" type call
 * Two  means a "Write" type call
 * This is used to select a default time-out as given below.
 */
static char call_type[] = {
	0, 0, 1, 0, 0, 0, 1,
	0, 2, 2, 2, 2, 2, 2,
	2, 2, 1, 0 };

/*
 * Minimum time-out values indexed by call type
 * These units are in "eights" of a second to avoid multiplies
 */
static unsigned int minimum_timeo[] = {
	6, 7, 10 };

/*
 * Similar table, but to determine which timer to use
 * (only real reads and writes!)
 */
static char timer_type[] = {
	0, 0, 0, 0, 0, 0, 1,
	0, 2, 0, 0, 0, 0, 0,
	0, 0, 1, 0 };

/*
 * Back off for retransmission timeout, MAXTIMO is in hz of a sec
 */
#define MAXTIMO	(20*HZ)
#define backoff(tim)	((((tim) << 1) > MAXTIMO) ? MAXTIMO : ((tim) << 1))

#define MIN_NFS_TSIZE 512	/* minimum "chunk" of NFS IO */
#define REDUCE_NFS_TIME (HZ/2)	/* rtxcur we try to keep under */
#define INCREASE_NFS_TIME (HZ/3) /* srtt we try to keep under (scaled*8) */

/*
 * Function called when the RPC package notices that we are
 * re-transmitting, or when we get a response without retransmissions.
 */
void
nfs_feedback(flag, which, mi)
	int flag;
	int which;
	register struct mntinfo *mi;
{
	int kind;
	if (flag == FEEDBACK_REXMIT1) {
		if (mi->mi_timers[NFS_CALLTYPES].rt_rtxcur != 0 &&
		    mi->mi_timers[NFS_CALLTYPES].rt_rtxcur < REDUCE_NFS_TIME)
		    	return;
		if (mi->mi_curread > MIN_NFS_TSIZE) {
			mi->mi_curread /= 2;
			if (mi->mi_curread < MIN_NFS_TSIZE)
				mi->mi_curread = MIN_NFS_TSIZE;
		}
		if (mi->mi_curwrite > MIN_NFS_TSIZE) {
			mi->mi_curwrite /= 2;
			if (mi->mi_curwrite < MIN_NFS_TSIZE)
				mi->mi_curwrite = MIN_NFS_TSIZE;
		}
	} else if (flag == FEEDBACK_OK) {
		kind = timer_type[which];
		if (kind == 0) return;
		if (mi->mi_timers[kind].rt_srtt >= (u_short) INCREASE_NFS_TIME)
			return;
		if (kind==1) {
			if (mi->mi_curread >= mi->mi_tsize)
				return;
			mi->mi_curread +=  MIN_NFS_TSIZE;
			if (mi->mi_curread > mi->mi_tsize/2)
				mi->mi_curread = mi->mi_tsize;
		}
		if (kind==2) {
			if (mi->mi_curwrite >= mi->mi_stsize)
				return;
			mi->mi_curwrite += MIN_NFS_TSIZE;	
			if (mi->mi_curwrite > mi->mi_tsize/2)
				mi->mi_curwrite = mi->mi_tsize;
		}
	}
}

int
rfscall(mi, which, xid, xdrargs, argsp, xdrres, resp, cred)
	register struct mntinfo *mi;
	int	 which;
	int	 xid;
	xdrproc_t xdrargs;
	caddr_t	argsp;
	xdrproc_t xdrres;
	caddr_t	resp;
	struct cred *cred;
{
	CLIENT *client;
	register enum clnt_stat status;
	struct rpc_err rpcerr;
	struct timeval wait;
	struct cred *newcred;
	int timeo;		/* in units of HZ, 20ms. */
	int count;
	bool_t tryagain;

#ifdef NFSDEBUG
	printf("rfscall: %x, %d, %x, %x, %x, %x\n",
	    mi, which, xdrargs, argsp, xdrres, resp);
#endif
	clstat.ncalls++;
	clstat.reqs[which]++;
	if (nfsdprintf) printf("rfscall: Entered op = %d\n", which);

	rpcerr.re_errno = 0;
	rpcerr.re_status = RPC_SUCCESS;
	newcred = NULL;
retry:
	rpcerr.re_errno = clget(mi, cred, &client);
	if (rpcerr.re_errno != 0) {
		RPCLOG(1, "rfscall: clget failure: error %d\n",rpcerr.re_errno);
		rpcerr.re_status = RPC_FAILED;
		return (rpcerr.re_errno);
	}
	timeo = clnt_clts_settimers(client, 
	    &(mi->mi_timers[timer_type[which]]),
	    &(mi->mi_timers[NFS_CALLTYPES]),
	    (minimum_timeo[call_type[which]]*HZ)>>3,
	    mi->mi_dynamic ? nfs_feedback : (void (*)()) 0, (caddr_t)mi );

	if (xid == 0)
		clnt_clts_setxid(client, clnt_clts_getxid());
	else
		clnt_clts_setxid(client, xid);

	/*
	 * If hard mounted fs, retry call forever unless hard error occurs.
	 * If interruptable, need to count retransmissions at this level.
	 */
	if (mi->mi_int && mi->mi_hard)
		count = mi->mi_retrans;
	else
		count = 1;
	do {
		tryagain = FALSE;

		wait.tv_sec = timeo / HZ;
		wait.tv_usec = 1000000/HZ * (timeo % HZ);
		status = CLNT_CALL(client, which, xdrargs, argsp,
		    xdrres, resp, wait);
		switch (status) {
		case RPC_SUCCESS:
			break;

		/*
		 * Unrecoverable errors: give up immediately
		 */
		case RPC_AUTHERROR:
		case RPC_CANTENCODEARGS:
		case RPC_CANTDECODERES:
		case RPC_VERSMISMATCH:
		case RPC_PROCUNAVAIL:
		case RPC_PROGUNAVAIL:
		case RPC_PROGVERSMISMATCH:
		case RPC_CANTDECODEARGS:
			break;

		default:
			if (status == RPC_INTR) {
				tryagain = (bool_t)(mi->mi_hard && !mi->mi_int);
				if (tryagain)
					continue;
				rpcerr.re_status = RPC_INTR;
				rpcerr.re_errno = EINTR;
			} else
				tryagain = (bool_t)mi->mi_hard;

			if (tryagain) {
				timeo = backoff(timeo);
				if (--count > 0 && timeo < HZ*15)
					continue;
				if (!mi->mi_printed) {
					mi->mi_printed = 1;
	printf("NFS server %s not responding still trying\n", mi->mi_hostname);
				}
				if (timer_type[which] != 0) {
					/*
					 * On read or write calls, return
					 * back to the vnode ops level
					 */
					clfree(client);
					if (newcred)
						crfree(newcred);
					return (ENFS_TRYAGAIN);
				}
			}
		}
	} while (tryagain);

	if (status != RPC_SUCCESS) {
		clstat.nbadcalls++;
		mi->mi_down = 1;
		if (status != RPC_INTR) {
			CLNT_GETERR(client, &rpcerr);
			printf("NFS %s failed for server %s: %s\n",
				rfsnames[which], mi->mi_hostname,
				clnt_sperrno(status));
		}
	/* LINTED pointer alignment */
	} else if (resp && *(int *)resp == EACCES &&
	    newcred == NULL && cred->cr_uid == 0 && cred->cr_ruid != 0) {
		/*
		 * Boy is this a kludge!  If the reply status is EACCES
		 * it may be because we are root (no root net access).
		 * Check the real uid, if it isn't root make that
		 * the uid instead and retry the call.
		 */
		newcred = crdup(cred);
		cred = newcred;
		cred->cr_uid = cred->cr_ruid;
		clfree(client);
		goto retry;
	} else if (mi->mi_hard) {
		if (mi->mi_printed) {
			printf("NFS server %s ok\n", mi->mi_hostname);
			mi->mi_printed = 0;
		}
	} else {
		mi->mi_down = 0;
	}

	clfree(client);
	if (newcred) {
		crfree(newcred);
	}
	/*
	 *	This ``should never happen'', but if it ever does it's
	 *	a disaster, since callers of rfscall rely only on re_errno
	 *	to indicate failures.
	 */
	if (rpcerr.re_status != RPC_SUCCESS && rpcerr.re_errno == 0) {
		cmn_err(CE_PANIC, "rfscall: re_status %d, re_errno 0\n",
			rpcerr.re_status);
	}

#ifdef NFSDEBUG
	printf("rfscall: returning %d\n", rpcerr.re_errno);
#endif
	return (rpcerr.re_errno);
}

void
vattr_to_sattr(vap, sa)
	register struct vattr *vap;
	register struct nfssattr *sa;
{

	sa->sa_mode = vap->va_mode;
	sa->sa_uid = vap->va_uid;
	sa->sa_gid = vap->va_gid;
	sa->sa_size = vap->va_size;
	sa->sa_atime.tv_sec  = vap->va_atime.tv_sec;
	sa->sa_atime.tv_usec = vap->va_atime.tv_nsec/1000;
	sa->sa_mtime.tv_sec  = vap->va_mtime.tv_sec;
	sa->sa_mtime.tv_usec = vap->va_mtime.tv_nsec/1000;
}

void
setdiropargs(da, nm, dvp)
	struct nfsdiropargs *da;
	char *nm;
	struct vnode *dvp;
{
	/* LINTED pointer alignment */
	da->da_fhandle = *vtofh(dvp);
	da->da_name = nm;
}

int
setdirgid(dvp)
	struct vnode *dvp;
{
	/*
	 * To determine the expected group-id of the created file:
	 *  1)	If the filesystem was not mounted with the Old-BSD-compatible
	 *	GRPID option, and the directory's set-gid bit is clear,
	 *	then use the process's gid.
	 *  2)	Otherwise, set the group-id to the gid of the parent directory.
	 */
	/* LINTED pointer alignment */
	if (!(vtomi(dvp)->mi_grpid) && !(vtor(dvp)->r_attr.va_mode & VSGID))
		return ((int)u.u_cred->cr_gid);
	else
		/* LINTED pointer alignment */
		return ((int)vtor(dvp)->r_attr.va_gid);
}

u_int
setdirmode(dvp, om)
	struct vnode *dvp;
	u_int om;
{
	/*
	 * Modify the expected mode (om) so that the set-gid bit matches
	 * that of the parent directory (dvp).
	 */
	om &= ~VSGID;
	/* LINTED pointer alignment */
	if (vtor(dvp)->r_attr.va_mode & VSGID)
		om |= VSGID;
	return (om);
}

struct rnode *rpfreelist = NULL;
int rreuse, rnew, ractive, rreactive, rnfree, rnhash, rnpages;

/*
 * Return a vnode for the given fhandle.
 * If no rnode exists for this fhandle create one and put it
 * in a table hashed by fh_fsid and fs_fid.  If the rnode for
 * this fhandle is already in the table return it (ref count is
 * incremented by rfind.  The rnode will be flushed from the
 * table when nfs_inactive calls runsave.
 */
struct vnode *
makenfsnode(fh, attr, vfsp)
	fhandle_t *fh;
	struct nfsfattr *attr;
	struct vfs *vfsp;
{
	register struct rnode *rp;
	char newnode = 0;

	if ((rp = rfind(fh, vfsp)) == NULL) {
		if (rpfreelist &&
		    (rtov(rpfreelist)->v_pages == NULL || rnew >= nrnode)) {
			rp = rpfreelist;
			rpfreelist = rpfreelist->r_freef;
			rm_free(rp);
			rp_rmhash(rp);
			rinactive(rp);
			rreuse++;
		} else {
			rp = (struct rnode *)kmem_alloc(sizeof (*rp), KM_SLEEP);
			rnew++;
		}
		bzero((caddr_t)rp, sizeof (*rp));
		rp->r_fh = *fh;
		rtov(rp)->v_count = 1;
		rtov(rp)->v_op = &nfs_vnodeops;
		if (attr) {
			rtov(rp)->v_type = n2v_type(attr);
			rtov(rp)->v_rdev = n2v_rdev(attr);
		}
		rtov(rp)->v_data = (caddr_t)rp;
		rtov(rp)->v_vfsp = vfsp;
		rp_addhash(rp);
		/* LINTED pointer alignment */
		((struct mntinfo *)(vfsp->vfs_data))->mi_refct++;
		newnode++;
	}
	if (attr) {
		if (!newnode) {
			timestruc_t	mtime;	/* nfs_cache_check expects one */
			mtime.tv_sec = attr->na_mtime.tv_sec;
			mtime.tv_nsec = attr->na_mtime.tv_usec*1000;
			nfs_cache_check(rtov(rp), mtime);
		}
		nfs_attrcache(rtov(rp), attr);
	}
	return (rtov(rp));
}

/*
 * Rnode lookup stuff.
 * These routines maintain a table of rnodes hashed by fhandle so
 * that the rnode for an fhandle can be found if it already exists.
 * NOTE: RTABLESIZE must be a power of 2 for rtablehash to work!
 */

#define	BACK	0
#define	FRONT	1

#define	RTABLESIZE	64
#define	rtablehash(fh) \
    ((fh->fh_data[2] ^ fh->fh_data[5] ^ fh->fh_data[15]) & (RTABLESIZE-1))

struct rnode *rtable[RTABLESIZE];

/*
 * Put a rnode in the hash table
 */
STATIC void
rp_addhash(rp)
	struct rnode *rp;
{

	rp->r_hash = rtable[rtablehash(rtofh(rp))];
	rtable[rtablehash(rtofh(rp))] = rp;
	rnhash++;
}

/*
 * Remove a rnode from the hash table
 */
void
rp_rmhash(rp)
	struct rnode *rp;
{
	register struct rnode *rt;
	register struct rnode *rtprev = NULL;

	rt = rtable[rtablehash(rtofh(rp))];
	while (rt != NULL) {
		if (rt == rp) {
			if (rtprev == NULL) {
				rtable[rtablehash(rtofh(rp))] = rt->r_hash;
			} else {
				rtprev->r_hash = rt->r_hash;
			}
			rnhash--;
			return;
		}
		rtprev = rt;
		rt = rt->r_hash;
	}
}

/*
 * Add an rnode to the front of the free list
 */
STATIC void
add_free(rp, front)
	register struct rnode *rp;
	int front;
{
	if (rp->r_freef != NULL) {
		return;
	}
	if (rpfreelist == NULL) {
		rp->r_freef = rp;
		rp->r_freeb = rp;
		rpfreelist = rp;
	} else {
		rp->r_freef = rpfreelist;
		rp->r_freeb = rpfreelist->r_freeb;
		rpfreelist->r_freeb->r_freef = rp;
		rpfreelist->r_freeb = rp;
		if (front) {
			rpfreelist = rp;
		}
	}
	rnfree++;
}

/*
 * Remove an rnode from the free list
 */
STATIC void
rm_free(rp)
	register struct rnode *rp;
{
	if (rp->r_freef == NULL) {
		return;
	}
	if (rp->r_freef == rp) {
		rpfreelist = NULL;
	} else {
		if (rp == rpfreelist) {
			rpfreelist = rp->r_freef;
		}
		rp->r_freeb->r_freef = rp->r_freef;
		rp->r_freef->r_freeb = rp->r_freeb;
	}
	rp->r_freef = rp->r_freeb = NULL;
	rnfree--;
}

/*
 * free resource for rnode
 */
void
rinactive(rp)
	struct rnode *rp;
{
	if (rtov(rp)->v_pages) {
		(void) VOP_PUTPAGE(rtov(rp), 0, 0, B_INVAL, rp->r_cred);
	}
	if (rp->r_cred) {
		crfree(rp->r_cred);
		rp->r_cred = NULL;
	}
}

/*
 * Put an rnode on the free list.
 * The rnode has already been removed from the hash table.
 * If there are no pages on the vnode remove inactivate it,
 * otherwise put it back in the hash table so it can be reused
 * and the vnode pages don't go away.
 */
void
rfree(rp)
	register struct rnode *rp;
{
	/* LINTED pointer alignment */
	((struct mntinfo *)rtov(rp)->v_vfsp->vfs_data)->mi_refct--;
	if (rtov(rp)->v_pages == NULL) {
		rinactive(rp);
		add_free(rp, FRONT);
	} else {
		rp_addhash(rp);
		add_free(rp, BACK);
	}
}

/*
 * Lookup a rnode by fhandle.
 */
STATIC struct rnode *
rfind(fh, vfsp)
	fhandle_t *fh;
	struct vfs *vfsp;
{
	register struct rnode *rt;

	rt = rtable[rtablehash(fh)];
	while (rt != NULL) {
		if (bcmp((caddr_t)rtofh(rt), (caddr_t)fh, sizeof (*fh)) == 0 &&
		    vfsp == rtov(rt)->v_vfsp) {
			if (++rtov(rt)->v_count == 1) {
				/*
				 * reactivating a free rnode, up vfs ref count
				 * and remove rnode from free list.
				 */
				rm_free(rt);
				/* LINTED pointer alignment */
				((struct mntinfo *)(rtov(rt)->v_vfsp->vfs_data))->mi_refct++;
				rreactive++;
				if (rtov(rt)->v_pages) {
					rnpages++;
				}
			} else {
				ractive++;
			}
			rm_free(rt);
			return (rt);
		}
		rt = rt->r_hash;
	}
	return (NULL);
}

/*
 * Invalidate all vnodes for this vfs.
 * NOTE:  assumes vnodes have been written already via rflush().
 * Called by nfs_unmount in an attempt to get the "mount info count"
 * back to zero.  This routine will be filled in if we ever have
 * an LRU rnode cache.
 */
/*ARGSUSED*/
void
rinval(vfsp)
	struct vfs *vfsp;
{
	register struct rnode *rp;

	if (rpfreelist == NULL) {
		return;
	}
	rp = rpfreelist;
	do {
		if (rtov(rp)->v_vfsp == vfsp && rtov(rp)->v_count == 0) {
			rp_rmhash(rp);
			rinactive(rp);
		}
		rp = rp->r_freef;
	} while (rp != rpfreelist);
}

/*
 * Flush all vnodes in this (or every) vfs.
 * Used by nfs_sync and by nfs_unmount.
 */
void
rflush(vfsp)
	struct vfs *vfsp;
{
	register struct rnode **rpp, *rp;
	register struct vnode *vp;

	for (rpp = rtable; rpp < &rtable[RTABLESIZE]; rpp++) {
		for (rp = *rpp; rp != (struct rnode *)NULL; rp = rp->r_hash) {
			vp = rtov(rp);
			/*
			 * Don't bother sync'ing a vp if it
			 * is part of virtual swap device or
			 * if VFS is read-only
			 */
			if (IS_SWAPVP(vp) ||
			    (vp->v_vfsp->vfs_flag & VFS_RDONLY) != 0)
				continue;
			if (vfsp == (struct vfs *)NULL || vp->v_vfsp == vfsp) {
				(void) VOP_PUTPAGE(vp, 0, 0, B_ASYNC,
				    rp->r_cred);
			}
		}
	}
}

#define	PREFIXLEN	4
static char prefix[PREFIXLEN + 1] = ".nfs";

char *
newname()
{
	char *news;
	register char *s1, *s2;
	register uint id;
	static uint newnum = 0;

	if (newnum == 0)
		newnum = hrestime.tv_sec & 0xffff;

	news = (char *)kmem_alloc((u_int)NFS_MAXNAMLEN, KM_SLEEP);
	for (s1 = news, s2 = prefix; s2 < &prefix[PREFIXLEN]; )
		*s1++ = *s2++;
	id = newnum++;
	while (id) {
		*s1++ = "0123456789ABCDEF"[id & 0x0f];
		id >>= 4;
	}
	*s1 = '\0';
	return (news);
}

/*ARGSUSED*/
void
rlockk(rp, file, line)
	register struct rnode *rp;
	char *file;
	int line;
{
	RLOCK(rp);
}

/*ARGSUSED*/
void
runlockk(rp, file, line)
	register struct rnode *rp;
	char *file;
	int line;
{
	RUNLOCK(rp);
}
