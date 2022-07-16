/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:fsflush.c	1.3.2.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/tuneable.h"
#include "sys/inline.h"
#include "sys/systm.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/var.h"
#include "sys/buf.h"
#include "sys/vfs.h"
#include "sys/cred.h"
#include "sys/kmem.h"
#include "sys/vnode.h"
#include "sys/swap.h"
#include "sys/vm.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/sysinfo.h"
#include "sys/disp.h"	/* XXX needed for PREEMPT() */

#include "vm/hat.h"
#include "vm/page.h"
#include "vm/pvn.h"

extern int	Km_OsizeWanted;		/* In os/kma.c: kmem_alloc() is
					   sleeping waiting for memory */

int doiflush = 1;	/* non-zero to turn inode flushing on */
int dopageflush = 1;	/* non-zero to turn page flushing on */

/*
 * As part of file system hardening, this daemon is waken
 * every second to flush cached data which includes the
 * buffer cache, the inode cache and mapped pages.
 */
void
fsflush()
{
	register struct buf *bp;
	register autoup;
	register struct page *pp = pages;
	unsigned int i, s, nscan, pcount, icount, count = 0;


	ASSERT(v.v_autoup > 0);
	ASSERT(tune.t_fsflushr > 0);
	autoup = v.v_autoup * HZ;
	nscan = ((epages-pages) * (tune.t_fsflushr))/v.v_autoup;
	icount = v.v_autoup / tune.t_fsflushr;
loop:
	s = spl6();
	for (bp = bfreelist.av_forw; bp != &bfreelist; bp = bp->av_forw) {
		if ((bp->b_flags & B_DELWRI) && lbolt - bp->b_start >= autoup) {
			bp->b_flags |= B_ASYNC;
			notavail(bp);
			bwrite(bp);
			(void) splx(s);
			PREEMPT();
			goto loop;
		}
	}
	(void) splx(s);

	if (!dopageflush)
		goto iflush_out;

	/*
	 * Flush dirty pages.
	 */
	pcount = 0;
	while (pcount++ <= nscan) {
		/*
		 * Reject pages that don't make sense to free.
		 */
		s = spl6();
		if (!pp->p_vnode || pp->p_free || pp->p_lock || pp->p_intrans
		  || pp->p_lckcnt > 0 || pp->p_cowcnt > 0 || pp->p_keepcnt > 0
		  || IS_SWAPVP(pp->p_vnode) || pp->p_vnode->v_type == VCHR) {
			if (++pp >= epages)
				pp = pages;
			(void) splx(s);
			PREEMPT();
			continue;
		}

		/*
		 * hat_pagesync will turn off ref and mod bits loaded
		 * into the hardware.
		 */
		hat_pagesync(pp);

		if (pp->p_mod) {
			vnode_t	*pvp;
			off_t	poff;

			pvp = pp->p_vnode;
			poff = pp->p_offset;
			splx(s);
			VOP_PUTPAGE(pvp, poff, PAGESIZE, B_ASYNC,
			   (struct cred *)0);
		} else {
			splx(s);
		}

		if (++pp >= epages)
			pp = pages;
		PREEMPT();
	}

iflush_out:
	if (!doiflush)
		goto out;

	/*
	 * Flush cached attribute information (e.g. inodes).
	 */
	if (count++ >= icount) {
		count = 0;

		/*
		 * Sync back cached data.
		 */
		for (i = 1; i < nfstype; i++)
			(void)(*vfssw[i].vsw_vfsops->vfs_sync)(NULL,
				SYNC_ATTR, u.u_cred);
	}

out:
  	/*
  	 * Let's see whether something is waiting for memory.
	 * This is not really a fsflush() type thing to do; but we
	 * don't want to create a separate deamon to check for this.
  	 */
  	if ( Km_OsizeWanted > 0 &&
  	     ( (availrmem - btoc(Km_OsizeWanted)) > tune.t_minarmem ) &&
  	     ( (availsmem - btoc(Km_OsizeWanted)) > tune.t_minasmem) ) {
  		Km_OsizeWanted = 0;
  		wakeprocs((caddr_t)&Km_OsizeWanted, PRMPT);
  	}

	sleep((caddr_t)fsflush, PRIBIO);
	goto loop;
}


