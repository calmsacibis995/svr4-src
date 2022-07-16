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

#ident	"@(#)kern-vm:vm_vpage.c	1.3"

/*
 * VM - virtual page utilities.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "vm/vpage.h"
#include "vm/mp.h"

/*
 * Lock a virtual page via its prot structure.
 * First acquire a higher-level lock against multiprocessing
 * which protects a set of page-level locks (to save space).
 * If the vpage structure is locked, wait for it to be unlocked,
 * and then indicate failure to the caller.  Otherwise,
 * lock the virtual page, release the higher level lock and return.
 */
int
vpage_lock(l, vp)
	register mon_t *l;
	register struct vpage *vp;
{
	register int v;

	mon_enter(l);

	if (vp->vp_lock) {
		while (vp->vp_lock) {
			vp->vp_want = 1;
			cv_wait(l, (char *)vp);
		}
		v = -1;
	} else {
		vp->vp_lock = 1;
		v = 0;
	}

	mon_exit(l);
	return (v);
}

/*
 * Unlock a vpage.
 * Get a high-level lock to protect the prot structure
 * against multiprocessing.  Clear the lock and clear
 * the want, remembering if anyone wants the page.
 * Release the high-level lock and kick any waiters.
 */
void
vpage_unlock(l, vp)
	register mon_t *l;
	register struct vpage *vp;
{
	register int w;

	mon_enter(l);
	vp->vp_lock = 0;
	w = vp->vp_want;
	vp->vp_want = 0;
	if (w)
		cv_broadcast(l, (char *)vp);
	mon_exit(l);
}
