/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/flk_filock.c	1.2.4.1"
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
 * This file contains all of the file/record locking specific routines
 * for the lock manager.
 * 
 * All record lock lists (referenced by a pointer in the vnode) are
 * ordered by starting position relative to the beginning of the file.
 * 
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/vfs.h"
#include "sys/fstyp.h"
#include "sys/proc.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/fcntl.h"
#include "sys/kmem.h"
#include "sys/conf.h"
#include "sys/systm.h"
#include "prot_lock.h"

#undef wakeup

extern void wakeup();
extern int debug;

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

struct	flckinfo flckinfo;	/* configuration and acct info */
struct	filock	*sleeplcks;		/* head of chain of sleeping locks */
extern	char *xmalloc();

/*
 * Insert lock (lckdat) after given lock (fl).  If fl is NULL place the
 * new lock at the beginning of the list and update the head ptr to
 * list which is stored at the address given by lck_list. 
 */
struct filock *
insflck(lck_list, lckdat, fl)
	struct	filock	**lck_list;
	struct  flock   *lckdat;
	struct	filock	*fl;
{
	register struct filock *new;

	if (debug)
		printf("enter insflck() : start %d len %d type %d\n",
			lckdat->l_start, lckdat->l_len, lckdat->l_type);

	if((new = (filock_t *)xmalloc(sizeof(filock_t))) == NULL)
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
	if (debug)
		printf("enter delflck(). start %d end %d..\n",fl->set.l_start,
			fl->set.l_end);

	if (fl->prev != NULL)
		fl->prev->next = fl->next;
	else
		*lck_list = fl->next;
	if (fl->next != NULL)
		fl->next->prev = fl->prev;

	--flckinfo.reccnt;
	release_filock(fl);
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

	if (debug)
		printf("enter regflck(): ld->l_start %d flp->set.l_start %d ld->l_len %d flp->set.l_len %d\n",
			ld->l_start, flp->set.l_start, ld->l_len, flp->set.l_len); 
	if (flp->set.l_len && flp->set.l_len != MAXEND) {
		if (ld->l_start > flp->set.l_start) {
			if (ld->l_start-1 == END(&flp->set))
				return S_END|E_AFTER;
			if (ld->l_start > END(&flp->set)) 
				return S_AFTER|E_AFTER;
			regntype = S_MIDDLE;
		} else if (ld->l_start == flp->set.l_start)
			regntype = S_START;
		else
			regntype = S_BEFORE;
	
		if (END(ld) < END(&flp->set)) {
			if (END(ld) == flp->set.l_start-1)
				regntype |= E_START;
			else if (END(ld) < flp->set.l_start)
				regntype |= E_BEFORE;
			else
				regntype |= E_MIDDLE;
		} else if (END(ld) == END(&flp->set))
			regntype |= E_END;
		else
			regntype |= E_AFTER;
	} else {
		if (ld->l_start > flp->set.l_start) {
                        if (ld->l_start-1 == flp->set.l_len)
                                return S_END|E_AFTER;       
                        if (ld->l_start > flp->set.l_len)
                                return S_AFTER|E_AFTER;   
                        regntype = S_MIDDLE;           
                } else if (ld->l_start == flp->set.l_start)
                        regntype = S_START;                
                else                       
                        regntype = S_BEFORE;
                                            
                if (ld->l_len < flp->set.l_len) {
                        if (ld->l_len == flp->set.l_start-1)
                                regntype |= E_START;      
                        else if (ld->l_len < flp->set.l_start)
                                regntype |= E_BEFORE;       
                        else                         
                                regntype |= E_MIDDLE;
                } else if (ld->l_len == flp->set.l_len)
                        regntype |= E_END;
                else            
                        regntype |= E_AFTER;
	}

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
	struct filock 	**lck_list;
	struct filock 	*insrtp;
	struct flock	*ld;
{
	struct filock	*flp, *nflp;
	int regtyp;

	if (debug)
		printf("enter flckadj(): start %d len %d type %d\n",
			ld->l_start, ld->l_len, ld->l_type);

	nflp = (insrtp == (struct filock *)NULL) ? *lck_list : insrtp;

	if (debug) {
		if (nflp) {
                	printf("nflp : start %d len %d type %d\n",
                		nflp->set.l_start, nflp->set.l_len, 
				nflp->set.l_type);
		}
        }

	while (flp = nflp) {
		if (debug) {
			if (!flp->next) 
				printf("flp->next == NULL\n");
			if (flp->next)
                                printf("flp->next != NULL\n");
			printf("flp : start %d len %d type %d\n",
                        	flp->set.l_start, flp->set.l_len, 
				flp->set.l_type);
		}
		nflp = flp->next;
		if (debug) 
			printf("result from SAMEOWNER(&(flp->set), ld) %d\n",
				SAMEOWNER(&(flp->set), ld));
		if( SAMEOWNER(&(flp->set), ld) ) {

			/* Release already locked region if necessary */
			if (debug)
				printf("SAMEOWNER calling regflck()...\n");

			switch (regtyp = regflck(ld, flp)) {
			case S_BEFORE|E_BEFORE:
				if (debug)
					printf("S_BEFORE|E_BEFORE\n");
				nflp = (struct filock *)NULL;
				break;
			case S_BEFORE|E_START:
				if (debug)
					printf("S_BEFORE|E_START\n");
				if (ld->l_type == flp->set.l_type) {
					if (flp->set.l_len && flp->set.l_len != MAXEND)
						ld->l_end = END(&flp->set) + 
							1 - ld->l_start;
					else
						ld->l_end = flp->set.l_len +
							1 - ld->l_start;
					delflck(lck_list, flp);
				}
				nflp = (struct filock *)NULL;
				break;
			case S_START|E_END:
				if (debug)
					printf("S_START|E_END\n");
				/*
				 * Don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return 0;
			case S_START|E_AFTER:
				if (debug)
					printf("S_START|E_AFTER\n");
				insrtp = flp->prev;
				delflck(lck_list, flp);
				break;
			case S_BEFORE|E_END:
				if (debug)
					printf("S_BEFORE|E_END\n");
				if (ld->l_type == flp->set.l_type)
					nflp = (struct filock *)NULL;
			case S_BEFORE|E_AFTER:
				if (debug)
					printf("S_BEFORE|E_AFTER\n");
				delflck(lck_list, flp);
				break;
			case S_START|E_MIDDLE:
				if (debug)
					printf("S_START|E_MIDDLE\n");
				insrtp = flp->prev;
			case S_MIDDLE|E_MIDDLE:
				if (debug)
					printf("S_MIDDLE|E_MIDDLE\n");
				/*
				 * Don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return 0;
			case S_BEFORE|E_MIDDLE:
				if (debug)
					printf("S_BEFORE|E_MIDDLE\n");
				if (ld->l_type == flp->set.l_type)
					if (flp->set.l_len && flp->set.l_len != MAXEND)
						ld->l_end = END(&flp->set) - 
							ld->l_start + 1;
					else
						ld->l_end = flp->set.l_len -
							ld->l_start + 1;
				else {
					/* setup piece after end of (un)lock */
					register struct	filock *tdi, *tdp;
					struct flock td;

					td = flp->set;
					if (ld->l_len && ld->l_len != MAXEND)
						td.l_start = END(ld) + 1;
					else
						td.l_start = ld->l_len + 1;
					tdp = tdi = flp;
					do {
						if (debug)
							printf("tdp->set.l_start %d td.l_start %d\n",tdp->set.l_start, td.l_start);
						if (tdp->set.l_start < td.l_start)
							tdi = tdp;
						else
							break;
					} while (tdp = tdp->next);
					if (flp->set.l_len && flp->set.l_len != MAXEND)
                                        	td.l_len = END(&flp->set) - 
							td.l_start + 1;
					else
						td.l_len = flp->set.l_len -
							td.l_start + 1;
					if (debug)
						printf("td.l_start %d td.l_end %d tdi->set.l_start %d tdi->set.l_len %d\n",
							td.l_start, td.l_end,
							tdi->set.l_start, tdi->set.l_len);
					if (insflck(lck_list, &td, tdi) == (struct filock *)NULL)
						return ENOLCK;
				}
				if (regtyp == (S_MIDDLE|E_MIDDLE)) {
					if (debug)
						printf("S_MIDDLE|E_MIDDLE\n");	
					/* setup piece before (un)lock */
					flp->set.l_end = ld->l_start - 
						flp->set.l_start;
					if (debug)
						printf("flp->set.l_end=%d\n",
							flp->set.l_end);
					wakeup(flp);
					insrtp = flp;
				} else
					delflck(lck_list, flp);
				nflp = (struct filock *)NULL;
				break;
			case S_MIDDLE|E_END:
				if (debug)
					printf("S_MIDDLE|E_END\n");
				/*
				 * Don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return 0;
				flp->set.l_end = ld->l_start - 
					flp->set.l_start;
				wakeup(flp);
				insrtp = flp;
				break;
			case S_MIDDLE|E_AFTER:
				if (debug)
					printf("S_MIDDLE|E_AFTER: ld->l_type %d flp->set.l_type %d\n", ld->l_type, flp->set.l_type);
				if (ld->l_type == flp->set.l_type) {
					if (debug) 
                                                printf("ld->l_type == flp->set.l_type\n");
                                        ld->l_len += ld->l_start - flp->set.l_start;
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					delflck(lck_list, flp);
				} else {
					if (debug)
						printf("ld->l_type != flp->set.l_type\n");
					flp->set.l_end = ld->l_start - flp->set.l_start;
					wakeup(flp);
					insrtp = flp;
				}
				break;
			case S_END|E_AFTER:
				if (debug)
					printf("S_END|E_AFTER\n");
				if (ld->l_type == flp->set.l_type) {
                                        ld->l_len += ld->l_start - flp->set.l_start;
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					delflck(lck_list, flp);
				}
				break;
			case S_AFTER|E_AFTER:
				if (debug)
					printf("S_AFTER|E_AFTER\n");
				insrtp = flp;
				break;
			}
		}
	}

	if (debug)
		printf("ld->l_type=%d ld->l_start %d ld->l_end %d\n",
			ld->l_type,ld->l_start,ld->l_end);
	if (ld->l_type && ld->l_type != F_UNLCK) {
		if (flp = insrtp) {
			do {
				if (flp->set.l_start < ld->l_start)
					insrtp = flp;
				else
					break;
			} while (flp = flp->next);
		}
		if (insflck(lck_list, ld, insrtp) == (struct filock *)NULL)
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

	if (debug)
		printf("enter blocked() : start %d len %d type %d\n",
			lckdat->l_start, lckdat->l_len, lckdat->l_type);

	*insrt = NULL;
	for (f = flp; f != NULL; f = f->next) {
		if (f->set.l_start < lckdat->l_start)
			*insrt = f;
		else
			break;
		if (f->set.l_len && f->set.l_len != MAXEND) {
			if( SAMEOWNER(&(f->set), lckdat) ) {
				if ((lckdat->l_start-1) <= END(&f->set))
					break;
			} else if (lckdat->l_start <= END(&f->set) &&
		  		(f->set.l_type == F_WRLCK
		    		|| (f->set.l_type == F_RDLCK && lckdat->l_type == F_WRLCK)))
					return f;
		} else {
			if( SAMEOWNER(&(f->set), lckdat) ) {
                                if ((lckdat->l_start-1) <= f->set.l_len)
                                        break;
                        } else if (lckdat->l_start <= f->set.l_len &&
                                (f->set.l_type == F_WRLCK
                                || (f->set.l_type == F_RDLCK && lckdat->l_type == F_WRLCK))) 
                                        return f;
		}
	}

	for (; f != NULL; f = f->next) {
		if (lckdat->l_len && lckdat->l_len != MAXEND) {
			if (END(lckdat) < f->set.l_start)
				break;
			if (lckdat->l_start <= END(&f->set) 
		  		&& ( !SAMEOWNER(&(f->set), lckdat) )
		  		&& (f->set.l_type == F_WRLCK
		    		|| (f->set.l_type == F_RDLCK
		            	&& lckdat->l_type == F_WRLCK)))
				return f;
		} else {
			if (lckdat->l_len < f->set.l_start)
                                break;                   
                        if (lckdat->l_start <= f->set.l_len
                        && ( !SAMEOWNER(&(f->set), lckdat) )
                        && (f->set.l_type == F_WRLCK    
                        || (f->set.l_type == F_RDLCK
                                && lckdat->l_type == F_WRLCK)))
                                return f;
		}
	}

	return NULL;
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
	short blckpid, blcksysid;

	if (debug) {
		printf("enter deadflck(): start %d end %d type %d\n",
			lckdat->l_start, END(lckdat), lckdat->l_type);
	}

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

