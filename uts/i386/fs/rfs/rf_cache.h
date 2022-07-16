/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_cache.h	1.3"
/*
 * Interfaces supporting RFS reads and writes through the VM page cache
 */

#include "sys/vnode.h"

/*
 * To maintain control flow information for cache reads.
 */
typedef enum rfc_ctl {
	RFC_RETRY,		/* restart read */
	RFC_INCACHE		/* read is going through cache */
} rfc_ctl_t;

/*
 * Arg structure for reads and writes.
 *
 *	*uiop - read/write addresses, etc.
 *	replysdp - for copysync responses to old servers for reads.  For
 *		writes, channel to send copyin responses.
 *	cached - set for cached reads and writes.
 *
 *	Set only for cached reads -
 *	  *ctlp - set to control restarting of read (e.g. if we lose the
 *		cache.
 *	  sdsize - the local notion of file size when we started the read, to
 *		close race with cache disable messages.
 *	  pfx - points to resident prefix, or NULL.
 *	  infix - intitially NULL, points to pages being filled.
 *	  sfx - points to resident suffix, or NULL.
 *	  startoff - the file offset of the first page in the local read
 *		request
 *	  firstnrb - the file offset of the first nonresident byte requested
 *	  nextnrb - where the next byte of data goes in the cache
 *	  endnrb - the file offset of the first byte past the last nonresident
 *		byte requested
 *
 *	For writes -
 *	  ioflags - IO_APPEND (others?) or nothing.
 *	  kern - true iff io data is likely to be in kernel when
 *		 RFS message is assembled and vnode can be paged.
 *		 Set by rf_putpage and for cached writes.  Last
 *		 condition is necessary because rfcl_esbwrmsg
 *		 ultimately gets to rf_getpage to map the pages
 *		 in.
 *	  bufp - defined iff kern.  non-NULL if I/O pages are covered
 *	       by a buffer header.  Guaranteed that b_bcount <= MAXBSIZE
 *	       and that pages are virtually addressable.
 *	       Assumes that cleanup at end of IO will be done at lower level,
 *	       e.g., by using an esballoc-ed steams message.
 *	  uio - defined only if cached, used for I/O into page cache.
 *	  iovec - defined only if cached, used for I/O into page cache.
 */
typedef struct rf_rwa {
	uio_t		*uiop;
	sndd_t		*replysdp;
	u_char		cached;
	union {
		struct {
			rfc_ctl_t	*ctlp;
			size_t		sdsize;
			struct page	*pfx;
			struct page	*infix;
			struct page	*sfx;
			off_t		startoff;
			off_t		firstnrb;
			off_t		nextnrb;
			off_t		endnrb;
		} rd;
		struct {
			int		ioflag;
			int		kern;
			buf_t		*bufp;
			uio_t		uio;
			iovec_t		iovec;
		} wr;
	} rf_rwa_un;
} rf_rwa_t;

#define rd_ctlp		rf_rwa_un.rd.ctlp
#define rd_sdsize	rf_rwa_un.rd.sdsize
#define rd_pfx		rf_rwa_un.rd.pfx
#define rd_infix	rf_rwa_un.rd.infix
#define rd_sfx		rf_rwa_un.rd.sfx
#define rd_startoff	rf_rwa_un.rd.startoff
#define rd_firstnrb	rf_rwa_un.rd.firstnrb
#define rd_nextnrb	rf_rwa_un.rd.nextnrb
#define rd_endnrb	rf_rwa_un.rd.endnrb
#define wr_ioflag	rf_rwa_un.wr.ioflag
#define wr_kern		rf_rwa_un.wr.kern
#define wr_bufp		rf_rwa_un.wr.bufp
#define cwruio		rf_rwa_un.wr.uio
#define wr_iovec	rf_rwa_un.wr.iovec

#define rfc_aborted(pp)	((pp)->p_vnode == NULL || (pp)->p_gone)
#define rfc_pptokv(pp)	((caddr_t)pfntokv(page_pptonum(pp)))

/*
 * vcode must be (ulong)0 if unknown
 */
#define	rfc_disable(sdp, vcode)			\
	((void)((sdp)->sd_stat &= ~SDCACHE,	\
	  (sdp)->sd_vcode = (vcode) ? (vcode) : (sdp)->sd_vcode))

/*
 * Enable caching on the denoted send descriptor, and install the
 * attribute params.
 */
#define rfc_enable(sdp, vcode, fhandle)		\
	((void)((sdp)->sd_vcode = (vcode),	\
	(sdp)->sd_fhandle = (fhandle), (sdp)->sd_stat |= SDCACHE))

extern void	rfc_pageunlock();
extern int	rfc_pagelist();
extern page_t	*rfc_page_lookup();
extern void	rfc_plrele();
extern int	rfc_readmove();
extern int	rfc_plmove();
extern int	rfc_writefill();
extern void	rfc_disable_msg();
extern void	rfc_pageabort();
extern void	rfc_mountinval();
extern int	rfc_v2vcodeck();
extern void	rfc_sdabort();
extern sndd_t	*rfc_sdsearch();

extern long	rfc_time;	/* number of ticks to elapse after an update
				 * before re-enabling cache;  -1 implies
				 * no caching */

/*
 * Whenever a new vnode reference is obtained via an RFLOOKUP request,
 * the server is expected to evaluate 3 VOP_ACCESS()'s, with modes of
 * VREAD, VWRITE, and VEXEC, and to evaluate a VOP_GETATTR() for the
 * result vnode.  The client caches the results of these operations
 * in rfcl_lookup_cache.  Subsequent rf_getattr() and rf_access() ops
 * may find their results here and thus avoid network messages.
 *
 * lkc_info contains 3 errnos and an rf_attr structure.  These are loaded
 * directly from the response message.
 *
 * lkc_crp identifies the credentials associated with the cached
 * permissions.  A NULL value indicates an empty cache.
 *
 * lkc_vp identifies the vnode whose info is cached.
 *
 * Instead of va_uid and va_gid, id-mapped xuid and xgid are reported
 * to VOP_GETATTRs setting the VATTR_EXEC flag.  They are after the
 * rest of the cached data to allow it to be assigned conventienl.
 * (It travels together in the message, but the xids are from the
 * message header.)
 */
typedef struct rfcl_lookup_cache {
	cred_t		*lkc_crp;
	rflkc_info_t 	lkc_info;
	uid_t		lkc_xuid;
	gid_t		lkc_xgid;
	vnode_t		*lkc_vp;
} rfcl_lookup_cache_t;

extern rfcl_lookup_cache_t	rfcl_lookup_cache;

#define lkc_read_err		lkc_info.rflkc_read_err
#define lkc_write_err		lkc_info.rflkc_write_err
#define lkc_exec_err		lkc_info.rflkc_exec_err
#define lkc_attr		lkc_info.rflkc_attr
