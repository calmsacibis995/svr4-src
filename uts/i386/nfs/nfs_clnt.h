/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NFS_NFS_CLNT_H
#define _NFS_NFS_CLNT_H

#ident	"@(#)kern-nfs:nfs_clnt.h	1.1"

/*	  @(#)nfs_clnt.h 2.28 88/08/19 SMI	*/

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

/*
 * vfs pointer to mount info
 */
#define	vftomi(vfsp)	((struct mntinfo *)((vfsp)->vfs_data))

/*
 * vnode pointer to mount info
 */
#define	vtomi(vp)	((struct mntinfo *)(((vp)->v_vfsp)->vfs_data))

/*
 * NFS vnode to server's block size
 */
#define	vtoblksz(vp)	(vtomi(vp)->mi_bsize)


#define	HOSTNAMESZ	32
#define	ACREGMIN	3	/* min secs to hold cached file attr */
#define	ACREGMAX	60	/* max secs to hold cached file attr */
#define	ACDIRMIN	30	/* min secs to hold cached dir attr */
#define	ACDIRMAX	60	/* max secs to hold cached dir attr */
#define	ACMINMAX	3600	/* 1 hr is longest min timeout */
#define	ACMAXMAX	36000	/* 10 hr is longest max timeout */

#define	NFS_CALLTYPES	3	/* Lookups, Reads, Writes */

/*
 * Fake errno passed back from rfscall to indicate transfer size adjustment
 */
#define	ENFS_TRYAGAIN	999

/*
 * NFS private data per mounted file system
 */
struct mntinfo {
	struct knetconfig	*mi_knetconfig;		/* bound TLI fd */
	struct netbuf	 mi_addr;	/* server's address */
	struct netbuf	 mi_syncaddr;	/* AUTH_DES time sync addr */
	struct vnode	*mi_rootvp;	/* root vnode */
	u_int		 mi_hard:1;	/* hard or soft mount */
	u_int		 mi_printed:1;	/* not responding message printed */
	u_int		 mi_int:1;	/* interrupts allowed on hard mount */
	u_int		 mi_down:1;	/* server is down */
	u_int		 mi_noac:1;	/* don't cache attributes */
	u_int		 mi_nocto:1;	/* no close-to-open consistency */
	u_int		 mi_dynamic:1;	/* dynamic transfer size adjustment */
	u_int		 mi_grpid:1;	/* System V group id inheritance */
	u_int		 mi_rpctimesync:1;	/* RPC time sync */
	int		 mi_refct;	/* active vnodes for this vfs */
	long		 mi_tsize;	/* transfer size (bytes) */
					/* really read size */
	long		 mi_stsize;	/* server's max transfer size (bytes) */
					/* really write size */
	long		 mi_bsize;	/* server's disk block size */
	int		 mi_mntno;	/* kludge to set client rdev for stat*/
	int		 mi_timeo;	/* inital timeout in 10th sec */
	int		 mi_retrans;	/* times to retry request */
	char		 mi_hostname[HOSTNAMESZ];	/* server's hostname */
	char		*mi_netname;	/* server's netname */
	int		 mi_netnamelen;	/* length of netname */
	int		 mi_authflavor;	/* authentication type */
	u_int		 mi_acregmin;	/* min secs to hold cached file attr */
	u_int		 mi_acregmax;	/* max secs to hold cached file attr */
	u_int		 mi_acdirmin;	/* min secs to hold cached dir attr */
	u_int		 mi_acdirmax;	/* max secs to hold cached dir attr */
	/*
	 * Extra fields for congestion control, one per NFS call type,
	 * plus one global one.
	 */
	struct rpc_timers mi_timers[NFS_CALLTYPES+1];
	long		mi_curread;	/* current read size */
	long		mi_curwrite;	/* current write size */
};

/*
 * Mark cached attributes as timed out
 */
#define	PURGE_ATTRCACHE(vp)	{vtor(vp)->r_attrtime.tv_sec = hrestime.tv_sec; \
				 vtor(vp)->r_attrtime.tv_usec = hrestime.tv_nsec / 1000;}

/*
 * Mark cached attributes as uninitialized (must purge all caches first)
 */
#define	INVAL_ATTRCACHE(vp)	(vtor(vp)->r_attrtime.tv_sec = 0)

/*
 * If returned error is ESTALE flush all caches.
 */
#define	PURGE_STALE_FH(errno, vp) \
	if ((errno) == ESTALE) { pvn_vptrunc(vp, 0, 0); nfs_purge_caches(vp); }

/*
 * Is cache valid?
 * Swap is always valid, if no attributes (attrtime == 0) or
 * if mtime matches cached mtime it is valid
 * NOTE: mtime is now a timestruc_t.
 */
#define	CACHE_VALID(rp, mtime) \
	((rtov(rp)->v_flag & VISSWAP) == VISSWAP || \
	 (rp)->r_attrtime.tv_sec == 0 || \
	 ((mtime).tv_sec  == (rp)->r_attr.va_mtime.tv_sec && \
	  (mtime).tv_nsec == (rp)->r_attr.va_mtime.tv_nsec))

#endif	/* _NFS_NFS_CLNT_H */
