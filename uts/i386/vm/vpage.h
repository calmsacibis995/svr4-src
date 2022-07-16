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

#ifndef _VM_VPAGE_H
#define _VM_VPAGE_H

#ident	"@(#)kern-vm:vpage.h	1.3"

/*
 * VM - Information per virtual page.
 */
struct vpage {
	u_int	vp_prot: 4;		/* see <sys/mman.h> prot flags */
	u_int	vp_advise: 3;		/* see <sys/mman.h> madvise flags */
	u_int	vp_lock: 1;		/* someone has this vpage locked  */
	u_int	vp_want: 1;		/* someone wants this vpage */
	u_int	vp_ref: 1;		/* reference bit */
	u_int	vp_mod: 1;		/* (maybe) modify bit, from hat */
	u_int	vp_pplock: 1;		/* physical page locked by me */
	u_int	: 4;
};

int	vpage_lock(/* l, vp */);
void	vpage_unlock(/* l, vp */);

#endif	/* _VM_VPAGE_H */
