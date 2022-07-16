/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:flock.c	1.3.2.2"

/*
 * This file contains all of the file/record locking specific routines.
 * 
 * All record lock lists (referenced by a pointer in the vnode) are
 * ordered by starting position relative to the beginning of the file.
 * 
 * In this file the name "l_end" is a macro and is used in place of
 * "l_len" because the end, not the length, of the record lock is
 * stored internally.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/fstyp.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/fcntl.h"
#include "sys/kmem.h"
#include "sys/flock.h"
#include "sys/conf.h"
#include "sys/systm.h"
#include "sys/tuneable.h"

#define	SLEEP(ptr, pri)	sleep((caddr_t)(ptr), pri)

#define	WAKEUP(ptr)	if (ptr->stat.wakeflg) { \
				wakeprocs((caddr_t)(ptr), PRMPT); \
				ptr->stat.wakeflg = 0 ; \
			}
#define SAMEOWNER(a, b)	(((a)->l_pid == (b)->l_pid) && \
				((a)->l_sysid == (b)->l_sysid))

/* region types */
#define	S_BEFORE	010
#define	S_START		020
#define	S_MIDDLE	030
#define	S_END		040
#define	S_AFTER		050
#define	E_BEFORE	001
#define	E_START		002
#define	E_MIDDLE	003
#define	E_END		004
#define	E_AFTER		005

extern	struct	flckinfo flckinfo;	/* configuration and acct info */
struct	filock	*sleeplcks;		/* head of chain of sleeping locks */

/*
 * Insert lock (lckdat) after given lock (fl).  If fl is NULL place the
 * new lock at the beginning of the list and update the head ptr to
 * list which is stored at the address given by lck_list. 
 */
struct filock *
insflck(lck_list, lckdat, fl)
	struct	filock	**lck_list;
	struct	filock	*fl;
	struct	flock	*lckdat;
{
	register struct filock *new;

	if ((flckinfo.reccnt >= tune.t_flckrec) ||
	  ((new = (filock_t *)kmem_zalloc(sizeof(filock_t),KM_NOSLEEP))==NULL))
		return NULL;
	++flckinfo.reccnt;
	++flckinfo.rectot;
	new->set = *lckdat;
	new->set.l_pid = lckdat->l_pid;
	new->set.l_sysid = lckdat->l_sysid;
	new->stat.wakeflg = 0;
	if (fl == NULL) {
		new->next = *lck_list;
		if (new->next != NULL)
			new->next->prev = new;
		*lck_list = new;
	} else {
		new->next = fl->next;
		if (fl->next != NULL)
			fl->next->prev = new;
		fl->next = new;
	}
	new->prev = fl;

	return new;
}

/*
 * Delete lock (fl) from the record lock list. If fl is the first lock
 * in the list, remove it and update the head ptr to the list which is
 * stored at the address given by lck_list.
 */
delflck(lck_list, fl)
	struct filock  **lck_list;
	struct filock  *fl;
{
	if (fl->prev != NULL)
		fl->prev->next = fl->next;
	else
		*lck_list = fl->next;
	if (fl->next != NULL)
		fl->next->prev = fl->prev;
	WAKEUP(fl);

	--flckinfo.reccnt;
	kmem_free((caddr_t)fl, sizeof(struct filock));
	return 0;
}

/*
 * regflck sets the type of span of this (un)lock relative to the specified
 * already existing locked section.
 * There are five regions:
 *
 *  S_BEFORE        S_START         S_MIDDLE         S_END          S_AFTER
 *     010            020             030             040             050
 *  E_BEFORE        E_START         E_MIDDLE         E_END          E_AFTER
 *      01             02              03              04              05
 * 			|-------------------------------|
 *
 * relative to the already locked section.  The type is two octal digits,
 * the 8's digit is the start type and the 1's digit is the end type.
 */
int
regflck(ld, flp)
	struct flock *ld;
	struct filock *flp;
{
	register int regntype;

	if (ld->l_start > flp->set.l_start) {
		if (ld->l_start-1 == flp->set.l_end)
			return S_END|E_AFTER;
		if (ld->l_start > flp->set.l_end)
			return S_AFTER|E_AFTER;
		regntype = S_MIDDLE;
	} else if (ld->l_start == flp->set.l_start)
		regntype = S_START;
	else
		regntype = S_BEFORE;

	if (ld->l_end < flp->set.l_end) {
		if (ld->l_end == flp->set.l_start-1)
			regntype |= E_START;
		else if (ld->l_end < flp->set.l_start)
			regntype |= E_BEFORE;
		else
			regntype |= E_MIDDLE;
	} else if (ld->l_end == flp->set.l_end)
		regntype |= E_END;
	else
		regntype |= E_AFTER;

	return  regntype;
}

/*
 * Adjust file lock from region specified by 'ld', in the record
 * lock list indicated by the head ptr stored at the address given
 * by lck_list. Start updates at the lock given by 'insrtp'.  It is 
 * assumed the list is ordered on starting position, relative to 
 * the beginning of the file, and no updating is required on any
 * locks in the list previous to the one pointed to by insrtp.
 * Insrtp is a result from the routine blocked().  Flckadj() scans
 * the list looking for locks owned by the process requesting the
 * new (un)lock :
 *
 * 	- If the new record (un)lock overlays an existing lock of
 * 	  a different type, the region overlaid is released.
 *
 * 	- If the new record (un)lock overlays or adjoins an exist-
 * 	  ing lock of the same type, the existing lock is deleted
 * 	  and its region is coalesced into the new (un)lock.
 *
 * When the list is sufficiently scanned and the new lock is not 
 * an unlock, the new lock is inserted into the appropriate
 * position in the list.
 */
int
flckadj(lck_list, insrtp, ld)
	struct filock	**lck_list;
	register struct filock *insrtp;
	struct flock	*ld;
{
	register struct	filock	*flp, *nflp;
	int regtyp;

	nflp = (insrtp == NULL) ? *lck_list : insrtp;

	while (flp = nflp) {
		nflp = flp->next;
		if( SAMEOWNER(&(flp->set), ld) ) {

			/* Release already locked region if necessary */

			switch (regtyp = regflck(ld, flp)) {
			case S_BEFORE|E_BEFORE:
				nflp = NULL;
				break;
			case S_BEFORE|E_START:
				if (ld->l_type == flp->set.l_type) {
					ld->l_end = flp->set.l_end;
					delflck(lck_list, flp);
				}
				nflp = NULL;
				break;
			case S_START|E_END:
				/*
				 * Don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return 0;
			case S_START|E_AFTER:
				insrtp = flp->prev;
				delflck(lck_list, flp);
				break;
			case S_BEFORE|E_END:
				if (ld->l_type == flp->set.l_type)
					nflp = NULL;
			case S_BEFORE|E_AFTER:
				delflck(lck_list, flp);
				break;
			case S_START|E_MIDDLE:
				insrtp = flp->prev;
			case S_MIDDLE|E_MIDDLE:
				/*
				 * Don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return 0;
			case S_BEFORE|E_MIDDLE:
				if (ld->l_type == flp->set.l_type)
					ld->l_end = flp->set.l_end;
				else {
					/* setup piece after end of (un)lock */
					register struct	filock *tdi, *tdp;
					struct flock td;

					td = flp->set;
					td.l_start = ld->l_end + 1;
					tdp = tdi = flp;
					do {
						if (tdp->set.l_start < td.l_start)
							tdi = tdp;
						else
							break;
					} while (tdp = tdp->next);
					if (insflck(lck_list, &td, tdi) == NULL)
						return ENOLCK;
				}
				if (regtyp == (S_MIDDLE|E_MIDDLE)) {
					/* setup piece before (un)lock */
					flp->set.l_end = ld->l_start - 1;
					WAKEUP(flp);
					insrtp = flp;
				} else
					delflck(lck_list, flp);
				nflp = NULL;
				break;
			case S_MIDDLE|E_END:
				/*
				 * Don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return 0;
				flp->set.l_end = ld->l_start - 1;
				WAKEUP(flp);
				insrtp = flp;
				break;
			case S_MIDDLE|E_AFTER:
				if (ld->l_type == flp->set.l_type) {
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					delflck(lck_list, flp);
				} else {
					flp->set.l_end = ld->l_start - 1;
					WAKEUP(flp);
					insrtp = flp;
				}
				break;
			case S_END|E_AFTER:
				if (ld->l_type == flp->set.l_type) {
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					delflck(lck_list, flp);
				}
				break;
			case S_AFTER|E_AFTER:
				insrtp = flp;
				break;
			}
		}
	}

	if (ld->l_type != F_UNLCK) {
		if (flp = insrtp) {
			do {
				if (flp->set.l_start < ld->l_start)
					insrtp = flp;
				else
					break;
			} while (flp = flp->next);
		}
		if (insflck(lck_list, ld, insrtp) == NULL)
			return ENOLCK;
	}

	return 0;
}

/*
 * blocked() checks whether a new lock (lckdat) would be
 * blocked by a previously set lock owned by another process.
 * Insrt is set to point to the lock where lock list updating
 * should begin to place the new lock.
 */ 
struct filock *
blocked(flp, lckdat, insrt)
	struct filock *flp;
	struct flock  *lckdat;
	struct filock **insrt;
{
	register struct filock *f;
	/* XENIX Support */
	register unsigned xenix_compat;

			/* XENIX binaries doing an fcntl() with
			 * GETLK expect to receive blocking info
			 * for F_UNLCK.  This is bogus, but that's
			 * the way XENIX behaved...
			 * Pre-System V XENIX binaries cannot
			 * have overlapping read locks.  The flag
			 * xenix_compat specifies whether either of
			 * these conditions is true.
			 */
	xenix_compat = ((VIRTUAL_XOUT && ISFCNTL) && lckdat->l_type == F_UNLCK)
				|| BADVISE_PRE_SV;
	/* End XENIX Support */

	*insrt = NULL;
	for (f = flp; f != NULL; f = f->next) {
		if (f->set.l_start < lckdat->l_start)
			*insrt = f;
		else
			break;
		if( SAMEOWNER(&(f->set), lckdat) ) {
			if ((lckdat->l_start-1) <= f->set.l_end)
				break;
		} else if (lckdat->l_start <= f->set.l_end
		  && (f->set.l_type == F_WRLCK
	/* XENIX Support */
				|| (xenix_compat && ISLOCKING)
	/* End XENIX Support */
		    || (f->set.l_type == F_RDLCK && lckdat->l_type == F_WRLCK)))
			return f;
	}

	for (; f != NULL; f = f->next) {
		if (lckdat->l_end < f->set.l_start)
			break;
		if (lckdat->l_start <= f->set.l_end
		  && ( !SAMEOWNER(&(f->set), lckdat) )
		  && (f->set.l_type == F_WRLCK
	/* XENIX Support */
				|| (xenix_compat && ISLOCKING)
	/* End XENIX Support */
		    || (f->set.l_type == F_RDLCK
		            && lckdat->l_type == F_WRLCK)))
			return f;
	}

	return NULL;
}

/*
 * get and set file/record locks
 *
 * cmd & SETFLCK indicates setting a lock.
 * cmd & SLPFLCK indicates waiting if there is a blocking lock.
 */
int
reclock(vp, lckdat, cmd, flag, offset)
	struct	vnode  *vp;
	struct	flock *lckdat;
	int	cmd;
	int	flag;
	off_t	offset;
{
	register struct filock  **lock_list, *sf;
	struct	filock *found, *insrt = NULL;
	int retval;
	int contflg;
	short whence;

	/* check access permissions */
	/* XENIX Support */
	/*
	 * If this is not a pre-System V XENIX binary, then
	 * check access permissions if trying to set a lock, or if this
	 * is a XENIX binary doing an fcntl() system call or a binary
	 * doing a XENIX locking() system call.  
	 */ 
	if (!BADVISE_PRE_SV) 
	if (((cmd & SETFLCK) || (((VIRTUAL_XOUT)&&ISFCNTL)||ISLOCKING))
	/* End XENIX Support - UNIX SVR3 Code was deleted */
	  && ((lckdat->l_type == F_RDLCK && (flag & FREAD) == 0)
	    || (lckdat->l_type == F_WRLCK && (flag & FWRITE) == 0)))
	/* XENIX Support */
		if (VIRTUAL_XOUT || ISLOCKING)
			/* return EINVAL vs. EBADF for XENIX compatibility */
			return(EINVAL);	
		else
	/* End XENIX Support */
			return (EBADF);
	
	/* Convert start to be relative to beginning of file */
	whence = lckdat->l_whence;
	if (retval = convoff(vp, lckdat, 0, offset))
		return retval;

	/* Convert l_len to be the end of the rec lock l_end */
	if (lckdat->l_len < 0)
		return EINVAL;
	if (lckdat->l_len == 0)
		lckdat->l_end = MAXEND;
	else
		lckdat->l_end += (lckdat->l_start - 1);

	/* check for arithmetic overflow */
	if (lckdat->l_start > lckdat->l_end)
		return EINVAL;

	lock_list = &vp->v_filocks;

	do {
		contflg = 0;
		switch (lckdat->l_type) {
		case F_RDLCK:
		case F_WRLCK:
			if ((found = blocked(*lock_list, lckdat, &insrt))
			  == NULL) {
				if (cmd & SETFLCK)
					retval =
					  flckadj(lock_list, insrt, lckdat);
				else
					lckdat->l_type = F_UNLCK;
				if ((cmd & (RCMDLCK|SLPFLCK)) == (RCMDLCK|SLPFLCK)) {
					/*
					 * Remove this lock from sleeplcks if
					 * it's there to prevent sleeplcks from
					 * filling up with old NFS lock requests
					 */
cleanslp:
					for (sf = sleeplcks; sf != NULL; sf = sf->next) {
						if (sf->set.l_whence == lckdat->l_whence &&
						    sf->set.l_start >= lckdat->l_start &&
						    sf->set.l_len <= lckdat->l_len &&
						    sf->set.l_sysid == lckdat->l_sysid &&
						    sf->set.l_pid == lckdat->l_pid) {
							delflck(&sleeplcks, sf);
							goto cleanslp;
						}
					}
				}
			} else if (cmd & SLPFLCK) {
				/* do deadlock detection here */
				if (deadflck(found, lckdat))
					retval = EDEADLK;
				else if (cmd & RCMDLCK) {
				   retval = EINTR;

				   /* If request not already on sleeplcks,
				    * put it there. (for deadlock detection)
				    */

				   for (sf = sleeplcks; sf != NULL; sf = sf->next) {
				      if (sf->set.l_type == lckdat->l_type &&
				      sf->set.l_whence == lckdat->l_whence &&
				      sf->set.l_start == lckdat->l_start &&
				      sf->set.l_len == lckdat->l_len &&
				      sf->set.l_sysid == lckdat->l_sysid &&
				      sf->set.l_pid == lckdat->l_pid && 
				      sf->stat.blk.sysid == found->set.l_sysid &&
				      sf->stat.blk.pid == found->set.l_pid) {
				         break;
				      }
				   }
				   if (sf == NULL) {
				      sf = insflck(&sleeplcks, lckdat, 
					   (struct filock *)NULL);
				      if (sf == NULL)
				         retval = ENOLCK;
				      else {
					 sf->stat.blk.pid = found->set.l_pid;
					 sf->stat.blk.sysid = found->set.l_sysid;
				      }
				   }
				}
				else if ((sf = insflck(&sleeplcks, lckdat,
				  (struct filock *)NULL)) == NULL)
					retval = ENOLCK;
				else {
					found->stat.wakeflg++;
					sf->stat.blk.pid = found->set.l_pid;
					sf->stat.blk.sysid = found->set.l_sysid;
					if (cmd & INOFLCK)
						VOP_RWUNLOCK(vp);
					if (SLEEP(found, PCATCH|(PZERO+1)))
						retval = EINTR;
					else
						contflg = 1;

					if (cmd & INOFLCK)
						VOP_RWLOCK(vp);
					sf->stat.blk.pid = 0;
					sf->stat.blk.sysid = 0;
					delflck(&sleeplcks, sf);
				}
			} else if (cmd & SETFLCK)
				retval = EAGAIN;
			else
				*lckdat = found->set;
			break;
		case F_UNLCK:
			/* removing a file record lock */
			if (cmd & SETFLCK)
				retval = flckadj(lock_list, *lock_list, lckdat);
			/* XENIX Support */
			/* XENIX binaries doing an fcntl() with
			 * GETLK expect to receive blocking info
			 * for F_UNLCK.  This is bogus, but that's
			 * the way XENIX behaved...
			 */
			else if (((VIRTUAL_XOUT) && ISFCNTL) &&
				((found=blocked(*lock_list, lckdat, &insrt)) 
								!= NULL))   {
				*lckdat = found->set;
			}
			/* End XENIX Support */
			if (cmd & RCMDLCK) {
				/*
				 * Remove this lock from sleeplcks if
				 * it's there to prevent sleeplcks from
				 * filling up with old NFS lock requests
				 */
cleanslp2:
				for (sf = sleeplcks; sf != NULL; sf = sf->next) {
					if (sf->set.l_whence == lckdat->l_whence &&
					    sf->set.l_start >= lckdat->l_start &&
					    sf->set.l_len <= lckdat->l_len &&
					    sf->set.l_sysid == lckdat->l_sysid &&
					    sf->set.l_pid == lckdat->l_pid) {
						delflck(&sleeplcks, sf);
						goto cleanslp2;
					}
				}
			}
			break;
		default:
			/* invalid lock type */
			retval = EINVAL;
			break;
		}
	} while (contflg);

	/* Restore l_len */
	if (lckdat->l_end == MAXEND)
		lckdat->l_len = 0;
	else
		lckdat->l_len -= (lckdat->l_start-1);
	(void) convoff(vp, lckdat, whence, offset);

	return retval;
}

/*
 * Enforce record locking protocol on regular file vp.
 */
int
chklock(vp, iomode, offset, len, fmode)
	register struct vnode *vp;
	int iomode;
	off_t offset;
	int len;
	int fmode;
{
	register int i;
	struct flock bf;
	int error = 0;

	bf.l_type = (iomode & FWRITE) ? F_WRLCK : F_RDLCK;
	bf.l_whence = 0;
	bf.l_start = offset;
	bf.l_len = len;
	/* XENIX Support */
	if (bf.l_len == 0)	/* Don't check whole file in reclock() */
	   if (VIRTUAL_XOUT || ISLOCKING)
		bf.l_len = 1;
	/* End XENIX Support */
	bf.l_pid = u.u_procp->p_epid;
	bf.l_sysid = u.u_procp->p_sysid;
	i = (fmode & (FNDELAY|FNONBLOCK)) ? INOFLCK : INOFLCK|SLPFLCK;
	if ((i = reclock(vp, &bf, i, 0, offset)) || bf.l_type != F_UNLCK) {
		error = i ? i : EAGAIN;
		/* XENIX Support */
		if (BADVISE_PRE_SV && (error == EAGAIN))
			error = EACCES;
		/* End XENIX Support */
	}
	return error;
}

/*
 * convoff - converts the given data (start, whence) to the
 * given whence.
 */
int
convoff(vp, lckdat, whence, offset)
	struct vnode *vp;
	struct flock *lckdat;
	int whence;
	off_t offset;
{
	int error;
	struct vattr vattr;

	vattr.va_mask = AT_SIZE;
	if (error = VOP_GETATTR(vp, &vattr, 0, u.u_cred))
		return error;
	if (lckdat->l_whence == 1)
		lckdat->l_start += offset;
	else if (lckdat->l_whence == 2)
		lckdat->l_start += vattr.va_size;
	else if (lckdat->l_whence != 0)
		return EINVAL;
	if (lckdat->l_start < 0)
		return EINVAL;
	if (whence == 1)
		lckdat->l_start -= offset;
	else if (whence == 2)
		lckdat->l_start -= vattr.va_size;
	else if (whence != 0)
		return EINVAL;
	lckdat->l_whence = (short)whence;
	return 0;
}

/*
 * deadflck does deadlock detection for a given record.
 */
int
deadflck(flp, lckdat)
	struct filock *flp;
	struct flock *lckdat;
{
	register struct filock *blck, *sf;
	pid_t blckpid;
	long blcksysid;

	blck = flp;	/* current blocking lock pointer */
	blckpid = blck->set.l_pid;
	blcksysid = blck->set.l_sysid;
	do {
		if (blckpid == lckdat->l_pid
		  && blcksysid == lckdat->l_sysid)
			return 1;
		/*
		 * If the blocking process is sleeping on a locked region,
		 * change the blocked lock to this one.
		 */
		for (sf = sleeplcks; sf != NULL; sf = sf->next) {
			if (blckpid == sf->set.l_pid
			  && blcksysid == sf->set.l_sysid) {
				blckpid = sf->stat.blk.pid;
				blcksysid = sf->stat.blk.sysid;
				break;
			}
		}
		blck = sf;
	} while (blck != NULL);
	return 0;
}

/*
 * Clean up record locks left around by process.
 */
cleanlocks(vp, pid, sysid)
	struct vnode *vp;
	pid_t pid;
	sysid_t sysid;
{
	register struct filock *flp, *nflp, **lock_list;

	lock_list = &vp->v_filocks;
	nflp = (struct filock *)0;
	for (flp = *lock_list; flp != NULL; flp = nflp) {
		nflp = flp->next;
		if (((flp->set.l_pid == pid) || (pid == IGN_PID))
		  && flp->set.l_sysid == sysid)
			delflck(lock_list, flp);
	};
	return 0;
}
