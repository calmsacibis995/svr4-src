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

#ident	"@(#)kern-fs:ufs/ufs_dsort.c	1.3"

/*
 * Seek sort for disks.  We depend on the driver
 * which calls us using b_resid as the current cylinder number.
 *
 * The argument dp structure holds a b_actf activity chain pointer
 * on which we keep two queues, sorted in ascending cylinder order.
 * The first queue holds those requests which are positioned after
 * the current cylinder (in the first request); the second holds
 * requests which came in after their cylinder number was passed.
 * Thus we implement a one way scan, retracting after reaching the
 * end of the drive to the first request on the second queue,
 * at which time it becomes the first queue.
 *
 * A one-way scan is natural because of the way UNIX read-ahead
 * blocks are allocated.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>

#define	b_cylin	b_resid

#ifndef i386	/* 386 disksort routine is in os/bio.c */
void
disksort(dp, bp)
	register struct diskhd *dp;
	register struct buf *bp;
{
	register struct buf *ap;

	/*
	 * If nothing on the activity queue, then
	 * we become the only thing.
	 */
	ap = dp->b_actf;
	if(ap == NULL) {
		dp->b_actf = bp;
		dp->b_actl = bp;
		bp->av_forw = NULL;
		return;
	}
	/*
	 * If we lie after the first (currently active)
	 * request, then we must locate the second request list
	 * and add ourselves to it.
	 */
	if (bp->b_cylin < ap->b_cylin) {
		while (ap->av_forw) {
			/*
			 * Check for an ``inversion'' in the
			 * normally ascending cylinder numbers,
			 * indicating the start of the second request list.
			 */
			if (ap->av_forw->b_cylin < ap->b_cylin) {
				/*
				 * Search the second request list
				 * for the first request at a larger
				 * cylinder number.  We go before that;
				 * if there is no such request, we go at end.
				 */
				do {
					if (bp->b_cylin < ap->av_forw->b_cylin)
						goto insert;
					ap = ap->av_forw;
				} while (ap->av_forw);
				goto insert;		/* after last */
			}
			ap = ap->av_forw;
		}
		/*
		 * No inversions... we will go after the last, and
		 * be the first request in the second request list.
		 */
		goto insert;
	}
	/*
	 * Request is at/after the current request...
	 * sort in the first request list.
	 */
	while (ap->av_forw) {
		/*
		 * We want to go after the current request
		 * if there is an inversion after it (i.e. it is
		 * the end of the first request list), or if
		 * the next request is a larger cylinder than our request.
		 */
		if (ap->av_forw->b_cylin < ap->b_cylin ||
		    bp->b_cylin < ap->av_forw->b_cylin)
			goto insert;
		ap = ap->av_forw;
	}
	/*
	 * Neither a second list nor a larger
	 * request... we go at the end of the first list,
	 * which is the same as the end of the whole schebang.
	 */
insert:
	bp->av_forw = ap->av_forw;
	ap->av_forw = bp;
	if (ap == dp->b_actl)
		dp->b_actl = bp;
	return;
}
#endif	/* i386 */
