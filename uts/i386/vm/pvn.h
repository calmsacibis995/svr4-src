/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ifndef _VM_PVN_H
#define _VM_PVN_H

#ident	"@(#)kern-vm:pvn.h	1.3.1.2"

/*
 * VM - paged vnode.
 *
 * The VM system manages memory as a cache of paged vnodes.
 * This file desribes the interfaces to common subroutines
 * used to help implement the VM/file system routines.
 */

struct	page *pvn_kluster(/* vp, off, seg, addr, offp, lenp, vp_off,
	    vp_len, isra */);
void	pvn_fail(/* plist, flags */);
void	pvn_done(/* bp */);
struct	page *pvn_vplist_dirty(/* vp, off, flags */);
struct	page *pvn_range_dirty(/* vp, off, eoff, offlo, offhi, flags */);
void	pvn_vptrunc(/* vp, vplen, zbytes */);
void	pvn_unloadmap(/* vp, offset, ref, mod */);
int	pvn_getpages(/* getapage, vp, off, len, protp, pl, plsz, seg, addr,
	    rw, cred */);

/*
 * When requesting pages from the getpage routines, pvn_getpages will
 * allocate space to return PVN_GETPAGE_NUM pages which map PVN_GETPAGE_SZ
 * worth of bytes.  These numbers are chosen to be the minimum of the max's
 * given in terms of bytes and pages.
 */
#define	PVN_MAX_GETPAGE_SZ	0x10000		/* getpage size limit */
#define	PVN_MAX_GETPAGE_NUM	0x8		/* getpage page limit */

#if PVN_MAX_GETPAGE_SZ > PVN_MAX_GETPAGE_NUM * PAGESIZE

#define	PVN_GETPAGE_SZ	ptob(PVN_MAX_GETPAGE_NUM)
#define	PVN_GETPAGE_NUM	PVN_MAX_GETPAGE_NUM

#else

#define	PVN_GETPAGE_SZ	PVN_MAX_GETPAGE_SZ
#define	PVN_GETPAGE_NUM	btop(PVN_MAX_GETPAGE_SZ)

#endif

#endif	/* _VM_PVN_H */
