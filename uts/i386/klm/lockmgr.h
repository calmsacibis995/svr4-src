/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-klm:lockmgr.h	1.3"
/*	@(#)lockmgr.h 1.6 88/02/08 SMI	*/
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
 * Header file for Kernel<->Network Lock-Manager implementation
 */

/* NOTE: size of a lockhandle-id should track the size of an fhandle */
#define KLM_LHSIZE	32

/* the lockhandle uniquely describes any file in a domain */
typedef struct {
	struct vnode *lh_vp;			/* vnode of file */
	char *lh_servername;			/* file server machine name */
	struct {				/* fhandle (sort of) */
		struct __lh_ufsid {
			fsid_t		__lh_fsid;
			struct fid	__lh_fid;
		} __lh_ufs;
#define KLM_LHPAD	(KLM_LHSIZE - sizeof (struct __lh_ufsid))
		char	__lh_pad[KLM_LHPAD];
	} lh_id;
} lockhandle_t;
#define lh_fsid	lh_id.__lh_ufs.__lh_fsid
#define lh_fid	lh_id.__lh_ufs.__lh_fid


/* define 'well-known' information */
#define KLM_PROTO	IPPROTO_UDP

/* define public routines */
int  klm_lockctl();
