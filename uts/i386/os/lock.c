/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:lock.c	1.3"
#include "sys/types.h"
#include "sys/bitmap.h"
#include "sys/sysmacros.h"
#include "sys/kmem.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/lock.h"
#include "sys/proc.h"
#include "sys/mman.h"
#include "sys/debug.h"
#include "sys/tuneable.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/vmsystm.h"

#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/page.h"

struct locka {
	int op;
};

#ifdef __STDC__
STATIC void mem_unlock(struct as *, addr_t, int, caddr_t, u_long *,
	size_t, size_t);
STATIC int textlock(void);
STATIC int datalock(void);
STATIC int ublock(void);
STATIC int proclock(void);
STATIC void tunlock(void);
STATIC void dunlock(void);
STATIC void ubunlock(void);
int punlock(void);
#else
STATIC void mem_unlock();
STATIC int textlock();
STATIC int datalock();
STATIC int ublock();
STATIC int proclock();
STATIC void tunlock();
STATIC void dunlock();
STATIC void ubunlock();
int punlock();
#endif

/* ARGSUSED */
int
lock_mem(uap, rvp)
	struct locka *uap;
	rval_t *rvp;
{
	register int error;

	if (!suser(u.u_cred))
		return EPERM;

	switch (uap->op) {
	case TXTLOCK:
		if (u.u_lock & (PROCLOCK|TXTLOCK))
			error = EINVAL;
		else 
			error = textlock();
		break;

	case PROCLOCK:
		if (u.u_lock & (PROCLOCK|TXTLOCK|DATLOCK))
			error = EINVAL;
		else
			error = proclock();
		break;

	case DATLOCK:
		if (u.u_lock & (PROCLOCK|DATLOCK))
			error = EINVAL;
		else
			error = datalock();
		break;

	case UNLOCK:
		error = punlock();
		break;

	default:
		error = EINVAL;
	}

	return error;
}


/* lock text segment */

STATIC int
textlock()
{

	register struct proc *p = u.u_procp;
	register int error;

	error = as_ctl(p->p_as, 0, 0, MC_LOCKAS, PROC_TEXT|PRIVATE|PROT_USER,
			MCL_CURRENT, (ulong *)NULL, (size_t)NULL);
	if (error) {
		tunlock();
		/* for compatibility we have to return EAGAIN */
		error = EAGAIN;
	} else
		u.u_lock |= TXTLOCK;
	return error;

}
		
/* unlock text segment */

STATIC void
tunlock()
{
	register struct proc *p = u.u_procp;

	(void) as_ctl(p->p_as, 0, 0, MC_UNLOCKAS, PROC_TEXT|PRIVATE|PROT_USER,
			0, (ulong *)NULL, (size_t)NULL);
	u.u_lock &= ~TXTLOCK;
}

/* lock data segment */

STATIC int
datalock()
{

	register struct proc *p = u.u_procp;
	register int error;

	error = as_ctl(p->p_as, 0, 0, MC_LOCKAS, PROC_DATA|PRIVATE|PROT_USER,
			MCL_CURRENT, (ulong *)NULL, (size_t)NULL);
	if (error) {
		dunlock();
		/* for compatibility we have to return EAGAIN */
		error = EAGAIN;
	} else
		u.u_lock |= DATLOCK;
	return error;

}
		
/* unlock data segment */

STATIC void
dunlock()
{
	register struct proc *p = u.u_procp;

	(void) as_ctl(p->p_as, 0, 0, MC_UNLOCKAS, PROC_DATA|PRIVATE|PROT_USER,
			0, (ulong *)NULL, (size_t)NULL);
	u.u_lock &= ~DATLOCK;
}
		
/*
 * Do accounting when locking process and ublock in core.
 * Ublocks for system processes are counted in segu_alloc and segu_free.
 * SSYS processes are also SLOCK'ed. We will only do the accounting for
 * non SSYS processes.
 * Allow user to mix plock and memlockall.
 */

STATIC int
ublock()
{

	/* return if already locked */
	if (u.u_lock & (PROCLOCK|MEMLOCK))
		return 0;
	if ((u.u_procp->p_flag & (SSYS|SLOCK)) == 0) {
		if ((availrmem - USIZE) < tune.t_minarmem) {
			nomemmsg("proclock", USIZE, 0, 1);
			return EAGAIN;
		}
		availrmem -= USIZE;
		pages_pp_locked += USIZE;
		u.u_procp->p_flag |= SLOCK;
	}
	return 0;
}

STATIC void
ubunlock()
{

	/* return if still locked */
	if (u.u_lock & (PROCLOCK|MEMLOCK))
		return;
	if ((u.u_procp->p_flag & (SSYS|SLOCK)) == SLOCK) {
		u.u_procp->p_flag &= ~SLOCK;
		availrmem += USIZE;
		pages_pp_locked -= USIZE;
	}
}

STATIC int
proclock()
{
	register int error;

	if ((error = ublock()) != 0)
		return error;

	if ((error = textlock()) != 0) {
		ubunlock();
		return error;
	}

	if ((error = datalock()) != 0) {
		tunlock();
		ubunlock();
		return error;
	}
	u.u_lock |= PROCLOCK;
	return error;
}

int
punlock()
{
	if ((u.u_lock & (PROCLOCK|TXTLOCK|DATLOCK)) == 0)
		return EINVAL;
	if (u.u_lock & PROCLOCK) {
		u.u_lock &= ~PROCLOCK;
		ubunlock();
	}
	if (u.u_lock & TXTLOCK)
		tunlock();
	if (u.u_lock & DATLOCK)
		dunlock();
	return 0;
}

struct memcntla {
	caddr_t addr;
	size_t	len;
	int	cmd;
	caddr_t	arg;
	int	attr;
	int	mask;
};

/*
 * Memory control operations
 */

/* ARGSUSED */
int
memcntl(uap, rvp)
	register struct memcntla *uap;
	rval_t *rvp;
{
	register struct seg *seg;		/* working segment */
	register struct seg *sseg;		/* starting segment */
	register u_int	rlen = 0;		/* rounded as length */
	caddr_t addr = uap->addr;
	uint    len = uap->len;
	int	 attr = uap->attr;
	caddr_t  arg = uap->arg;
	struct	 as  *as_pp = u.u_procp->p_as;  /* address space pointer */
	register ulong	 *mlock_map; 		/* pointer to bitmap used
						 * to represent the locked
						 * pages. */
	addr_t	raddr;				/* rounded address counter */
	size_t  mlock_size;			/* size of bitmap */
	size_t inx;
	size_t npages;
	int error = 0;

	if (uap->mask)
		return(EINVAL);
	if ((uap->cmd == MC_LOCKAS) || (uap->cmd == MC_UNLOCKAS)) {
		if ((addr != 0) || (len != 0)) {
			return EINVAL;
		}
	} else {
		if (((int)addr & PAGEOFFSET) != 0)
			return EINVAL;
		if (valid_usr_range(addr, len) == 0)
			return ENOMEM;
	}

	if ((VALID_ATTR & attr) != attr)
		return EINVAL;

	if ( (attr & SHARED) && (attr & PRIVATE) )
		return EINVAL;

	if ( ((uap->cmd == MC_LOCKAS) || (uap->cmd == MC_LOCK) ||
	     (uap->cmd == MC_UNLOCKAS) || (uap->cmd == MC_UNLOCK))
		    && (!suser(u.u_cred)) )
		return(EPERM);

	if (attr)
		attr |= PROT_USER;

	switch (uap->cmd) {
	case MC_SYNC:
		if ((int)arg & ~(MS_ASYNC|MS_INVALIDATE))
			return EINVAL;
		else {
			error = as_ctl(as_pp, addr, len, 
			    uap->cmd, attr, arg, (ulong *)NULL, (size_t)NULL);
		}
		return error;
	case MC_LOCKAS:
		if ((int)arg & ~(MCL_FUTURE|MCL_CURRENT) || (int)arg == 0)
			return EINVAL;

		/* do ublock availrmem accounting */
		if ((error = ublock()) != 0)
			return error;
		u.u_lock |= MEMLOCK;
		sseg = seg = as_pp->a_segs;
		if (seg == NULL)
			return 0;
		do {
			raddr = (addr_t)((u_int)seg->s_base & PAGEMASK);
			rlen += (((u_int)(seg->s_base + seg->s_size) +
				PAGEOFFSET) & PAGEMASK) - (u_int)raddr;
		} while ((seg = seg->s_next) != sseg);
			
		break;
	case MC_LOCK:
		/*
		 * Normalize addresses and lengths
		 */
		raddr = (addr_t)((u_int)addr & PAGEMASK);
		rlen  = (((u_int)(addr + len) + PAGEOFFSET) & PAGEMASK) -
				(u_int)raddr;
		break;
	case MC_UNLOCKAS:
		/* remove claim for ublock */
		u.u_lock &= ~MEMLOCK;
		ubunlock();
	case MC_UNLOCK:
		mlock_map = NULL;
		mlock_size = NULL;
		break;
	default:
		error = EINVAL;
	}

	if ( (uap->cmd == MC_LOCK) || (uap->cmd == MC_LOCKAS) ) {
		mlock_size = BT_BITOUL(btoc(rlen));
		mlock_map = (ulong *)kmem_zalloc((u_int)
			mlock_size * sizeof(ulong), KM_SLEEP);
	}
	error = as_ctl(as_pp, addr, len,
		uap->cmd, attr, arg, mlock_map, 0);
	if ( uap->cmd == MC_LOCK || uap->cmd == MC_LOCKAS ) { 
		if (error) {
			if (uap->cmd == MC_LOCKAS) {
				/* remove claim for ublock */
				u.u_lock &= ~MEMLOCK;
				ubunlock();

				inx = 0;
				npages = 0;
				sseg = seg = as_pp->a_segs;
				do {
					raddr = (addr_t)((u_int)seg->s_base
							& PAGEMASK);
					npages += seg_pages(seg);
					mem_unlock(as_pp, raddr, attr, arg, 
					     mlock_map, inx, npages);
					inx += seg_pages(seg);
				} while ((seg = seg->s_next) != sseg);
			}
			else  /* MC_LOCK */
				mem_unlock(as_pp, raddr, attr, arg, 
						mlock_map, 0, btoc(rlen));
		}
		kmem_free((caddr_t)mlock_map, mlock_size * sizeof(ulong));
	}
	return(error);
}

STATIC void
mem_unlock(as, addr, attr, arg, bitmap, position, nbits)
struct	as  *as;
addr_t	addr;
int	attr;
caddr_t arg;
ulong	*bitmap;
size_t position;
size_t	nbits;
{
	addr_t	range_start;
	size_t	pos1, pos2;   	
	u_int	size;

	pos1 = position;

	while(bt_range(bitmap, &pos1, &pos2, nbits)) {
		size = ctob((pos2 - pos1) + 1);
		range_start = addr + ctob(pos1);
		(void) as_ctl(as, range_start, size, MC_UNLOCK,
			attr, arg, (ulong *)NULL, (size_t)NULL);
		pos1 = pos2 + 1;
	}
}
