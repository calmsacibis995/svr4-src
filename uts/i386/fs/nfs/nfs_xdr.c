/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:nfs/nfs_xdr.c	1.3.1.1"

/*	@(#)nfs_xdr.c 2.38 88/02/08 SMI 	*/

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

#define NFSSERVER

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/dirent.h>
#include <sys/vfs.h>
#include <sys/stream.h>
#include <sys/debug.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#undef NFSSERVER
#include <nfs/nfs.h>
#include <netinet/in.h>
#include <vm/hat.h>
#include <vm/as.h>
#include <vm/seg.h>
#include <vm/seg_map.h>
#include <vm/seg_kmem.h>

#ifdef NFSDEBUG
extern int nfsdebug;

char *xdropnames[] = { "encode", "decode", "free" };
#endif
extern int nfsdprintf;
/*XXX*/	char *xdropnames[] = { "encode", "decode", "free" };

STATIC	void	rrokfree();

/*
 * These are the XDR routines used to serialize and deserialize
 * the various structures passed as parameters accross the network
 * between NFS clients and servers.
 */

/*
 * File access handle
 * The fhandle struct is treated a opaque data on the wire
 */
bool_t
xdr_fhandle(xdrs, fh)
	XDR *xdrs;
	fhandle_t *fh;
{

	if (xdr_opaque(xdrs, (caddr_t)fh, NFS_FHSIZE)) {
#ifdef NFSDEBUG
		printf("xdr_fhandle: %s %x\n", xdropnames[(int)xdrs->x_op], fh);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_fhandle %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}


/*
 * Arguments to remote write and writecache
 */
bool_t
xdr_writeargs(xdrs, wa)
	XDR *xdrs;
	struct nfswriteargs *wa;
{
	if (xdr_fhandle(xdrs, &wa->wa_fhandle) &&
	    xdr_long(xdrs, (long *)&wa->wa_begoff) &&
	    xdr_long(xdrs, (long *)&wa->wa_offset) &&
	    xdr_long(xdrs, (long *)&wa->wa_totcount) &&
	    xdr_bytes(xdrs, &wa->wa_data, (u_int *)&wa->wa_count,
		NFS_MAXDATA)) {
#ifdef NFSDEBUG
		printf("xdr_writeargs: %s off %d ct %d\n",
		    xdropnames[(int)xdrs->x_op],
		    wa->wa_offset, wa->wa_totcount);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_writeargs: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}


/*
 * File attributes
 */
bool_t
xdr_fattr(xdrs, na)
	XDR *xdrs;
	register struct nfsfattr *na;
{
	register long *ptr;

#ifdef NFSDEBUG
	printf("xdr_fattr: %s\n", xdropnames[(int)xdrs->x_op]);
#endif
	if (xdrs->x_op == XDR_ENCODE) {
		ptr = XDR_INLINE(xdrs, 17 * BYTES_PER_XDR_UNIT);
		if (ptr != NULL) {
			IXDR_PUT_ENUM(ptr, na->na_type);
			IXDR_PUT_LONG(ptr, na->na_mode);
			IXDR_PUT_LONG(ptr, na->na_nlink);
			IXDR_PUT_LONG(ptr, na->na_uid);
			IXDR_PUT_LONG(ptr, na->na_gid);
			IXDR_PUT_LONG(ptr, na->na_size);
			IXDR_PUT_LONG(ptr, na->na_blocksize);
			IXDR_PUT_LONG(ptr, na->na_rdev);
			IXDR_PUT_LONG(ptr, na->na_blocks);
			IXDR_PUT_LONG(ptr, na->na_fsid);
			IXDR_PUT_LONG(ptr, na->na_nodeid);
			IXDR_PUT_LONG(ptr, na->na_atime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_atime.tv_usec);
			IXDR_PUT_LONG(ptr, na->na_mtime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_mtime.tv_usec);
			IXDR_PUT_LONG(ptr, na->na_ctime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_ctime.tv_usec);
			return (TRUE);
		}
	} else {
		ptr = XDR_INLINE(xdrs, 17 * BYTES_PER_XDR_UNIT);
		if (ptr != NULL) {
			na->na_type = IXDR_GET_ENUM(ptr, enum nfsftype);
			na->na_mode = IXDR_GET_LONG(ptr);
			na->na_nlink = IXDR_GET_LONG(ptr);
			na->na_uid = IXDR_GET_LONG(ptr);
			na->na_gid = IXDR_GET_LONG(ptr);
			na->na_size = IXDR_GET_LONG(ptr);
			na->na_blocksize = IXDR_GET_LONG(ptr);
			na->na_rdev = IXDR_GET_LONG(ptr);
			na->na_blocks = IXDR_GET_LONG(ptr);
			na->na_fsid = IXDR_GET_LONG(ptr);
			na->na_nodeid = IXDR_GET_LONG(ptr);
			na->na_atime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_atime.tv_usec = IXDR_GET_LONG(ptr);
			na->na_mtime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_mtime.tv_usec = IXDR_GET_LONG(ptr);
			na->na_ctime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_ctime.tv_usec = IXDR_GET_LONG(ptr);
			return (TRUE);
		}
	}
	if (xdr_enum(xdrs, (enum_t *)&na->na_type) &&
	    xdr_u_long(xdrs, &na->na_mode) &&
	    xdr_u_long(xdrs, &na->na_nlink) &&
	    xdr_u_long(xdrs, &na->na_uid) &&
	    xdr_u_long(xdrs, &na->na_gid) &&
	    xdr_u_long(xdrs, &na->na_size) &&
	    xdr_u_long(xdrs, &na->na_blocksize) &&
	    xdr_u_long(xdrs, &na->na_rdev) &&
	    xdr_u_long(xdrs, &na->na_blocks) &&
	    xdr_u_long(xdrs, &na->na_fsid) &&
	    xdr_u_long(xdrs, &na->na_nodeid) &&
	    xdr_timeval(xdrs, &na->na_atime) &&
	    xdr_timeval(xdrs, &na->na_mtime) &&
	    xdr_timeval(xdrs, &na->na_ctime) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_fattr: %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * Arguments to remote read
 */
bool_t
xdr_readargs(xdrs, ra)
	XDR *xdrs;
	struct nfsreadargs *ra;
{

	if (xdr_fhandle(xdrs, &ra->ra_fhandle) &&
	    xdr_long(xdrs, (long *)&ra->ra_offset) &&
	    xdr_long(xdrs, (long *)&ra->ra_count) &&
	    xdr_long(xdrs, (long *)&ra->ra_totcount) ) {
#ifdef NFSDEBUG
		printf("xdr_readargs: %s off %d ct %d\n",
		    xdropnames[(int)xdrs->x_op],
		    ra->ra_offset, ra->ra_totcount);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_readargs: %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 *	Info necessary to free the mapping which is also dynamically allocated.
 */
struct rrokinfo {
	void	(*func)();
	mblk_t	*arg;
	int	done;
	struct vnode *vp;
	char	*map;
	char	*data;
	u_int	count;
};

STATIC void
rrokfree(rip)
	struct rrokinfo *rip;
{
	int s;
	if (nfsdprintf) printf("rrokfree: Entered, rip %x\n", rip);
	s = splimp();
	while (rip->done == 0) {
		(void) sleep((caddr_t)rip, PZERO-1);
	}
	(void) splx(s);
	/*
	 * Unlock and release the mapping.
	 */
	(void) as_fault(&kas, rip->data, rip->count, F_SOFTUNLOCK, S_READ);
	(void) segmap_release(segkmap, rip->map, 0);
	if (nfsdprintf) printf("rrokfree: release vnode %x\n", rip->vp);
	VN_RELE(rip->vp);
	if (nfsdprintf) printf("rrokfree: free block %x\n", rip->arg);
	(void) freeb(rip->arg);
	if (nfsdprintf) printf("rrokfree: Done\n");
}

/*
 * Wake up user process to free mapping and vp (rrokfree)
 */
/*
STATIC void
rrokwakeup(rip)
	struct rrokinfo *rip;
{

	rip->done = 1;
	wakeprocs((caddr_t)rip, PRMPT);
}
*/

/*
 * Status OK portion of remote read reply
 */
bool_t
xdr_rrok(xdrs, rrok)
	XDR *xdrs;
	struct nfsrrok *rrok;
{

	if (xdr_fattr(xdrs, &rrok->rrok_attr)) {
		if (xdrs->x_op == XDR_ENCODE && rrok->rrok_map) {
			/* server side */
			struct rrokinfo *rip;
			mblk_t *mp;

			while (!(mp = allocb(sizeof(*rip), BPRI_LO)))
				if (strwaitbuf(sizeof(*rip), BPRI_LO, 1)) {
					if (nfsdprintf) printf("xdr_rrok: allocb failed\n");
					return (FALSE);
				}
			/* LINTED pointer alignment */
			rip = (struct rrokinfo *) mp->b_rptr;
			rip->func = rrokfree;
			rip->arg = mp;
			rip->done = 0;
			rip->vp = rrok->rrok_vp;
			rip->map = rrok->rrok_map;
			rip->data = rrok->rrok_data;
			rip->count = rrok->rrok_count;
			xdrs->x_public = (caddr_t)rip;
			rip->done = 1;
			/*
			 * Try it the old, slow way.
			 */
			if (xdr_bytes(xdrs, &rrok->rrok_data,
			    (u_int *)&rrok->rrok_count, NFS_MAXDATA) ) {
#ifdef NFSDEBUG
				printf("xdr_rrok: %s %d addr %x\n",
				    xdropnames[(int)xdrs->x_op],
				    rrok->rrok_count, rrok->rrok_data);
#endif
				return (TRUE);
			}
		} else {			/* client side */
			if (xdr_bytes(xdrs, &rrok->rrok_data,
			    (u_int *)&rrok->rrok_count, NFS_MAXDATA) ) {
#ifdef NFSDEBUG
				printf("xdr_rrok: %s %d addr %x\n",
				    xdropnames[(int)xdrs->x_op],
				    rrok->rrok_count, rrok->rrok_data);
#endif
				return (TRUE);
			}
		}
	}
#ifdef NFSDEBUG
	printf("xdr_rrok: %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

struct xdr_discrim rdres_discrim[2] = {
	{ (int)NFS_OK, xdr_rrok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Reply from remote read
 */
bool_t
xdr_rdresult(xdrs, rr)
	XDR *xdrs;
	struct nfsrdresult *rr;
{

#ifdef NFSDEBUG
	printf("xdr_rdresult: %s\n", xdropnames[(int)xdrs->x_op]);
#endif
	if (xdr_union(xdrs, (enum_t *)&(rr->rr_status),
	      (caddr_t)&(rr->rr_ok), rdres_discrim, xdr_void) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_rdresult: %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * File attributes which can be set
 */
bool_t
xdr_sattr(xdrs, sa)
	XDR *xdrs;
	struct nfssattr *sa;
{

	if (xdr_u_long(xdrs, &sa->sa_mode) &&
	    xdr_u_long(xdrs, &sa->sa_uid) &&
	    xdr_u_long(xdrs, &sa->sa_gid) &&
	    xdr_u_long(xdrs, &sa->sa_size) &&
	    xdr_timeval(xdrs, &sa->sa_atime) &&
	    xdr_timeval(xdrs, &sa->sa_mtime) ) {
#ifdef NFSDEBUG
		printf("xdr_sattr: %s mode %o uid %d gid %d size %d\n",
		    xdropnames[(int)xdrs->x_op], sa->sa_mode, sa->sa_uid,
		    sa->sa_gid, sa->sa_size);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_sattr: %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

struct xdr_discrim attrstat_discrim[2] = {
	{ (int)NFS_OK, xdr_fattr },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Reply status with file attributes
 */
bool_t
xdr_attrstat(xdrs, ns)
	XDR *xdrs;
	struct nfsattrstat *ns;
{

	if (xdr_union(xdrs, (enum_t *)&(ns->ns_status),
	      (caddr_t)&(ns->ns_attr), attrstat_discrim, xdr_void) ) {
#ifdef NFSDEBUG
		printf("xdr_attrstat: %s stat %d\n",
		    xdropnames[(int)xdrs->x_op], ns->ns_status);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_attrstat: %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * NFS_OK part of read sym link reply union
 */
bool_t
xdr_srok(xdrs, srok)
	XDR *xdrs;
	struct nfssrok *srok;
{

	if (xdr_bytes(xdrs, &srok->srok_data, (u_int *)&srok->srok_count,
	    NFS_MAXPATHLEN) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_srok: %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

struct xdr_discrim rdlnres_discrim[2] = {
	{ (int)NFS_OK, xdr_srok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Result of reading symbolic link
 */
bool_t
xdr_rdlnres(xdrs, rl)
	XDR *xdrs;
	struct nfsrdlnres *rl;
{

#ifdef NFSDEBUG
	printf("xdr_rdlnres: %s\n", xdropnames[(int)xdrs->x_op]);
#endif
	if (xdr_union(xdrs, (enum_t *)&(rl->rl_status),
	      (caddr_t)&(rl->rl_srok), rdlnres_discrim, xdr_void) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_rdlnres: %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * Arguments to readdir
 */
bool_t
xdr_rddirargs(xdrs, rda)
	XDR *xdrs;
	struct nfsrddirargs *rda;
{

	if (xdr_fhandle(xdrs, &rda->rda_fh) &&
	    xdr_u_long(xdrs, &rda->rda_offset) &&
	    xdr_u_long(xdrs, &rda->rda_count) ) {
#ifdef NFSDEBUG
		printf("xdr_rddirargs: %s off %d, cnt %d\n",
		    xdropnames[(int)xdrs->x_op], rda->rda_offset,
		    rda->rda_count);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_rddirargs: %s FAILED\n", xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * Directory read reply:
 * union (enum status) {
 *	NFS_OK: entlist;
 *		boolean eof;
 *	default:
 * }
 *
 * Directory entries
 *	struct  direct {
 *		off_t   d_off;	       	       * offset of next entry *
 *		u_long  d_fileno;	       * inode number of entry *
 *		u_short d_reclen;	       * length of this record *
 *		u_short d_namlen;	       * length of string in d_name *
 *		char    d_name[MAXNAMLEN + 1];  * name no longer than this *
 *	};
 * are on the wire as:
 * union entlist (boolean valid) {
 * 	TRUE: struct otw_dirent;
 *	      u_long nxtoffset;
 * 	      union entlist;
 *	FALSE:
 * }
 * where otw_dirent is:
 * 	struct dirent {
 *		u_long	de_fid;
 *		string	de_name<NFS_MAXNAMELEN>;
 *	}
 */

#define	nextdp(dp)	((struct dirent *)((int)(dp) + (dp)->d_reclen))
#ifdef	SYSV
/* #define MINDIRSIZ(dp)	(sizeof(struct dirent) + strlen((dp)->d_name)) */
/* sizeof(struct dirent) is rounded up, so subtract 1 */
#define MINDIRSIZ(dp)	(sizeof(struct dirent) - 1 + strlen((dp)->d_name))
#else
#define	MINDIRSIZ(dp)	(sizeof(struct dirent) - MAXNAMLEN + (dp)->d_namlen)
#endif

#ifdef	SYSV
#define	d_fileno	d_ino
#endif

/*
 * ENCODE ONLY
 */
bool_t
xdr_putrddirres(xdrs, rd)
	XDR *xdrs;
	struct nfsrddirres *rd;
{
	struct dirent *dp;
	char *name;
	int size;
	int xdrpos;
	int lastxdrpos;
	u_int namlen;
	bool_t true = TRUE;
	bool_t false = FALSE;

#ifdef NFSDEBUG
	printf("xdr_putrddirres: %s size %d offset %d\n",
	    xdropnames[(int)xdrs->x_op], rd->rd_size, rd->rd_offset);
#endif
	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}
	if (!xdr_enum(xdrs, (enum_t *)&rd->rd_status)) {
		return (FALSE);
	}
	if (rd->rd_status != NFS_OK) {
		return (TRUE);
	}

	xdrpos = XDR_GETPOS(xdrs);
	lastxdrpos = xdrpos;
	for (size = rd->rd_size, dp = rd->rd_entries;
	     size > 0;
	     size -= dp->d_reclen, dp = nextdp(dp) ) {
		if (dp->d_reclen == 0 || MINDIRSIZ(dp) > dp->d_reclen) {
#ifdef NFSDEBUG
			printf("xdr_putrddirres: bad directory\n");
#endif
			return (FALSE);
		}
#ifdef NFSDEBUG
#ifdef	SYSV
		printf("xdr_putrddirres: entry %d %s(%d) %d %d %d %d\n",
		    dp->d_fileno, dp->d_name, strlen(dp->d_name), dp->d_off,
		    dp->d_reclen, XDR_GETPOS(xdrs), size);
#else
		printf("xdr_putrddirres: entry %d %s(%d) %d %d %d %d\n",
		    dp->d_fileno, dp->d_name, dp->d_namlen, dp->d_off,
		    dp->d_reclen, XDR_GETPOS(xdrs), size);
#endif	/* SYSV */
#endif	/* NFSDEBUG */
		if (dp->d_fileno == 0) {
			continue;
		}
		name = dp->d_name;
#ifdef	SYSV
		namlen = strlen(name);
#else
		namlen = dp->d_namlen;
#endif
		if (!xdr_bool(xdrs, &true) ||
		    !xdr_u_long(xdrs, &dp->d_fileno) ||
		    !xdr_bytes(xdrs, &name, &namlen, NFS_MAXNAMLEN) ||
		    !xdr_u_long(xdrs, (u_long *) &dp->d_off) ) {
			return (FALSE);
		}
		if (XDR_GETPOS(xdrs) - xdrpos >= rd->rd_origreqsize -
		    2 * RNDUP(sizeof (bool_t))) {
			XDR_SETPOS(xdrs,lastxdrpos);
			rd->rd_eof = FALSE;
			break;
		} else
			lastxdrpos = XDR_GETPOS(xdrs);
	}
	if (!xdr_bool(xdrs, &false)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &rd->rd_eof)) {
		return (FALSE);
	}
	if (XDR_GETPOS(xdrs) - xdrpos >= rd->rd_origreqsize) {
		printf("xdr_putrddirres: encoding overrun\n");
		return (FALSE);
	}
	return (TRUE);
}

#define roundtoint(x)	(((x) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#ifdef	SYSV

#define		reclen(dp)	roundtoint((strlen((dp)->d_name) + 1 + sizeof(long) +\
				sizeof(unsigned short)))
#undef	DIRSIZ
#define	DIRSIZ(dp,namlen)	\
	(((sizeof (struct dirent) - 1 + (namlen + 1)) + 3) & ~3)

#else

#define reclen(dp)	roundtoint(((dp)->d_namlen + 1 + sizeof(u_long) +\
				2 * sizeof(u_short)))
#undef	DIRSIZ
#define	DIRSIZ(dp)	\
	(((sizeof (struct dirent) - (MAXNAMLEN + 1) + ((dp)->d_namlen+1)) + 3) & ~3)

#endif	/* SYSV */

/*
 * DECODE ONLY
 */
bool_t
xdr_getrddirres(xdrs, rd)
	XDR *xdrs;
	struct nfsrddirres *rd;
{
	struct dirent *dp;
	int size;
	bool_t valid;
	bool_t first = TRUE;
        off_t offset = (off_t)-1;

	if (!xdr_enum(xdrs, (enum_t *)&rd->rd_status)) {
		return (FALSE);
	}
	if (rd->rd_status != NFS_OK) {
		return (TRUE);
	}

#ifdef NFSDEBUG
	printf("xdr_getrddirres: %s size %d\n",
	    xdropnames[(int)xdrs->x_op], rd->rd_size);
#endif
	size = rd->rd_size;
	dp = rd->rd_entries;
	for (;;) {
		if (!xdr_bool(xdrs, &valid)) {
			return (FALSE);
		}
		if (valid) {
#ifdef	SYSV
			u_int namlen;
			u_long tmp_fileno;

			if (!xdr_u_long(xdrs, &tmp_fileno) ||
			    !xdr_u_int(xdrs, &namlen)) {
				return (FALSE);
			} else {
				dp->d_fileno = tmp_fileno;
			}

			if (DIRSIZ(dp, namlen) > size) {
				/*
				 *	Entry won't fit.  If this isn't the
				 *	first one, just quit and return what
				 *	we've already XDRed.  Else, it's an
				 *	error
				 */
				if (first == FALSE) {
					rd->rd_eof = FALSE;
					rd->rd_size = (int)dp -
							(int)(rd->rd_entries);
					rd->rd_offset = offset;
					return (TRUE);
				} else {
					return (FALSE);
				}
			}
			if (!xdr_opaque(xdrs, dp->d_name, namlen)||
			    !xdr_u_long(xdrs, (u_long *) &dp->d_off) ) {
#ifdef NFSDEBUG
				printf("xdr_getrddirres: entry error\n");
#endif
				return (FALSE);
			}
			dp->d_reclen = DIRSIZ(dp, namlen);
			dp->d_name[namlen] = '\0';
			offset = dp->d_off;
			first = FALSE;
#ifdef NFSDEBUG
			printf("xdr_getrddirres: entry %d %s(%d) %d %d\n",
			    dp->d_fileno, dp->d_name, namlen,
			    dp->d_reclen, dp->d_off);
#endif

#else	/* SYSV */
			if (!xdr_u_long(xdrs, &dp->d_fileno) ||
			    !xdr_u_short(xdrs, &dp->d_namlen) ||
			    (DIRSIZ(dp) > size) ||
			    !xdr_opaque(xdrs, dp->d_name, (u_int)dp->d_namlen)||
			    !xdr_u_long(xdrs, (u_long *) &dp->d_off) ) {
#ifdef NFSDEBUG
				printf("xdr_getrddirres: entry error\n");
#endif
				return (FALSE);
			}
			dp->d_reclen = DIRSIZ(dp);
			dp->d_name[dp->d_namlen] = '\0';
			offset = dp->d_off;
#ifdef NFSDEBUG
			printf("xdr_getrddirres: entry %d %s(%d) %d %d\n",
			    dp->d_fileno, dp->d_name, dp->d_namlen,
			    dp->d_reclen, dp->d_off);
#endif
#endif	/* SYSV */
		} else {
			break;
		}
		size -= reclen(dp);
		if (size <= 0) {
			return (FALSE);
		}
		dp = nextdp(dp);
	}
	if (!xdr_bool(xdrs, &rd->rd_eof)) {
		return (FALSE);
	}
	rd->rd_size = (int)dp - (int)(rd->rd_entries);
	rd->rd_offset = offset;
#ifdef NFSDEBUG
	printf("xdr_getrddirres: returning size %d offset %d eof %d\n",
	    rd->rd_size, rd->rd_offset, rd->rd_eof);
#endif
	return (TRUE);
}

/*
 * Arguments for directory operations
 */
bool_t
xdr_diropargs(xdrs, da)
	XDR *xdrs;
	struct nfsdiropargs *da;
{

	if (xdr_fhandle(xdrs, &da->da_fhandle) &&
	    xdr_string(xdrs, &da->da_name, NFS_MAXNAMLEN) ) {
#ifdef NFSDEBUG
		printf("xdr_diropargs: %s '%s'\n",
		    xdropnames[(int)xdrs->x_op], da->da_name);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_diropargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * NFS_OK part of directory operation result
 */
bool_t
xdr_drok(xdrs, drok)
	XDR *xdrs;
	struct nfsdrok *drok;
{

	if (xdr_fhandle(xdrs, &drok->drok_fhandle) &&
	    xdr_fattr(xdrs, &drok->drok_attr) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_drok: FAILED\n");
#endif
	return (FALSE);
}

struct xdr_discrim diropres_discrim[2] = {
	{ (int)NFS_OK, xdr_drok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Results from directory operation 
 */
bool_t
xdr_diropres(xdrs, dr)
	XDR *xdrs;
	struct nfsdiropres *dr;
{

	if (xdr_union(xdrs, (enum_t *)&(dr->dr_status),
	      (caddr_t)&(dr->dr_drok), diropres_discrim, xdr_void) ) {
#ifdef NFSDEBUG
		printf("xdr_diropres: %s stat %d\n",
		    xdropnames[(int)xdrs->x_op], dr->dr_status);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_diropres: FAILED\n");
#endif
	return (FALSE);
}

/*
 * Time structure
 */
bool_t
xdr_timeval(xdrs, tv)
	XDR *xdrs;
	struct timeval *tv;
{

	if (xdr_long(xdrs, &tv->tv_sec) &&
	    xdr_long(xdrs, &tv->tv_usec) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_timeval: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to setattr
 */
bool_t
xdr_saargs(xdrs, argp)
	XDR *xdrs;
	struct nfssaargs *argp;
{

	if (xdr_fhandle(xdrs, &argp->saa_fh) &&
	    xdr_sattr(xdrs, &argp->saa_sa) ) {
#ifdef NFSDEBUG
		printf("xdr_saargs: %s\n", xdropnames[(int)xdrs->x_op]);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_saargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to create and mkdir
 */
bool_t
xdr_creatargs(xdrs, argp)
	XDR *xdrs;
	struct nfscreatargs *argp;
{

	if (xdr_diropargs(xdrs, &argp->ca_da) &&
	    xdr_sattr(xdrs, &argp->ca_sa) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_creatargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to link
 */
bool_t
xdr_linkargs(xdrs, argp)
	XDR *xdrs;
	struct nfslinkargs *argp;
{

	if (xdr_fhandle(xdrs, &argp->la_from) &&
	    xdr_diropargs(xdrs, &argp->la_to) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_linkargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to rename
 */
bool_t
xdr_rnmargs(xdrs, argp)
	XDR *xdrs;
	struct nfsrnmargs *argp;
{

	if (xdr_diropargs(xdrs, &argp->rna_from) &&
	    xdr_diropargs(xdrs, &argp->rna_to) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_rnmargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to symlink
 */
bool_t
xdr_slargs(xdrs, argp)
	XDR *xdrs;
	struct nfsslargs *argp;
{

	if (xdr_diropargs(xdrs, &argp->sla_from) &&
	    xdr_string(xdrs, &argp->sla_tnm, (u_int)MAXPATHLEN) &&
	    xdr_sattr(xdrs, &argp->sla_sa) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_slargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * NFS_OK part of statfs operation
 */
xdr_fsok(xdrs, fsok)
	XDR *xdrs;
	struct nfsstatfsok *fsok;
{

	if (xdr_long(xdrs, (long *)&fsok->fsok_tsize) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_bsize) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_blocks) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_bfree) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_bavail) ) {
#ifdef NFSDEBUG
		printf("xdr_fsok: %s tsz %d bsz %d blks %d bfree %d bavail %d\n",
		    xdropnames[(int)xdrs->x_op], fsok->fsok_tsize,
		    fsok->fsok_bsize, fsok->fsok_blocks, fsok->fsok_bfree,
		    fsok->fsok_bavail);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_fsok: FAILED\n");
#endif
	return (FALSE);
}

struct xdr_discrim statfs_discrim[2] = {
	{ (int)NFS_OK, xdr_fsok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Results of statfs operation
 */
xdr_statfs(xdrs, fs)
	XDR *xdrs;
	struct nfsstatfs *fs;
{

	if (xdr_union(xdrs, (enum_t *)&(fs->fs_status),
	      (caddr_t)&(fs->fs_fsok), statfs_discrim, xdr_void) ) {
#ifdef NFSDEBUG
		printf("xdr_statfs: %s stat %d\n",
		    xdropnames[(int)xdrs->x_op], fs->fs_status);
#endif
		return (TRUE);
	}
#ifdef NFSDEBUG
	printf("xdr_statfs: FAILED\n");
#endif
	return (FALSE);
}
