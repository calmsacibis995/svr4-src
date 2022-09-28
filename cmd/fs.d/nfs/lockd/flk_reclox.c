/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/flk_reclox.c	1.2.3.1"
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
struct reclock *sleeplcks_reclox;

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

extern	char *xmalloc();

/*
 * Insert lock (lckdat) after given lock (fl).  If fl is NULL place the
 * new lock at the beginning of the list and update the head ptr to
 * list which is stored at the address given by lck_list. 
 */
struct reclock *
insflck_reclox(lck_list, lckdat, fl)
	struct	reclock	**lck_list;
	struct	reclock	*fl;
	struct	reclock	*lckdat;
{
	register struct reclock *new;

	if (debug)
		printf("enter insflck_reclox() : start %d len %d type %d\n",
			lckdat->lck.lox.lld.l_start, 
			lckdat->lck.lox.lld.l_len, lckdat->lck.lox.lld.l_type);

	if((new = (reclock *)xmalloc(sizeof(reclock))) == (reclock *)NULL)
                return (reclock *)NULL;
	new->block = lckdat->block;
	new->exclusive = lckdat->exclusive;
	new->reclaim = lckdat->reclaim;
	new->state = lckdat->state;
	new->rel = lckdat->rel;
	if (debug)
		printf("Before copying lckdat->lck.fh...\n");
        obj_copy(&new->lck.fh, &lckdat->lck.fh);
	obj_copy(&new->cookie, &lckdat->cookie);
        new->lck.op = lckdat->lck.op;
        new->lck.svid = lckdat->lck.svid;
	if (debug) 
                printf("Before copying new->lck.oh...\n");
	obj_copy(&new->lck.oh, &lckdat->lck.oh);
	new->lck.clnt = lckdat->lck.clnt;
        new->lck.caller_name = lckdat->lck.caller_name;
	new->lck.lox.granted = lckdat->lck.lox.granted;
        new->lck.lox.color = lckdat->lck.lox.color;
        new->lck.lox.LockID = lckdat->lck.lox.LockID;
        new->lck.lox.class = lckdat->lck.lox.class;
	if (debug) 
                printf("Before copying lckdat->lck.lox.lld...\n");
        new->lck.lox.lld.l_sysid = lckdat->lck.lox.lld.l_sysid;
	new->lck.lox.lld.l_start = lckdat->lck.lox.lld.l_start;
	new->lck.lox.lld.l_len = lckdat->lck.lox.lld.l_len;
	new->lck.lox.lld.l_whence = lckdat->lck.lox.lld.l_whence;
	new->lck.lox.lld.l_pid = lckdat->lck.lox.lld.l_pid;
	new->lck.lox.lld.l_type = lckdat->lck.lox.lld.l_type;
	new->lck.lox.lld.pad[0] = lckdat->lck.lox.lld.pad[0];
	new->w_flag = 0;
	if (debug)  
                printf("Before nserting into list....\n");
	if (fl == NULL) {
		if (lck_list != NULL)
			new->next = *lck_list;
		else
			new->next = NULL;
		if (new->next != NULL)
			new->next->prev = new;
		if (debug)
                	printf("going to nsert into list....\n");
		*lck_list = new;
	} else {
		new->next = fl->next;
		if (fl->next != NULL)
			fl->next->prev = new;
		fl->next = new;
	}
	new->prev = fl;

	if (debug)
		printf("done inserting..\n");
	return new;
}

/*
 * Delete lock (fl) from the record lock list. If fl is the first lock
 * in the list, remove it and update the head ptr to the list which is
 * stored at the address given by lck_list.
 */
delflck_reclox(lck_list, fl)
	struct reclock  **lck_list;
	struct reclock  *fl;
{
	if (debug)
		printf("enter delflck_reclox(). start %d end %d..\n",
			fl->lck.lox.lld.l_start, fl->lck.lox.lld.l_end);

	if (fl->prev != NULL)
		fl->prev->next = fl->next;
	else
		*lck_list = fl->next;
	if (fl->next != NULL)
		fl->next->prev = fl->prev;

	release_reclock(fl);
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
regflck_reclox(ld, flp)
	struct reclock *ld;
	struct reclock *flp;
{
	register int regntype;

	if (debug)
		printf("enter regflck_reclox(): ld->lck.lox.lld.l_start %d flp->lck.lox.lld.l_start %d ld->lck.lox.lld.l_end %d flp->lck.lox.lld.l_end %d\n",
			ld->lck.lox.lld.l_start, flp->lck.lox.lld.l_start,
			ld->lck.lox.lld.l_len, flp->lck.lox.lld.l_len); 
	if (flp->lck.lox.lld.l_len && flp->lck.lox.lld.l_len != MAXEND) {
		if (ld->lck.lox.lld.l_start > flp->lck.lox.lld.l_start) {
			if (ld->lck.lox.lld.l_start-1 == END(&flp->lck.lox.lld))
				return S_END|E_AFTER;
			if (ld->lck.lox.lld.l_start > END(&flp->lck.lox.lld)) 
				return S_AFTER|E_AFTER;
			regntype = S_MIDDLE;
		} else if (ld->lck.lox.lld.l_start == flp->lck.lox.lld.l_start)
			regntype = S_START;
		else
			regntype = S_BEFORE;
	
		if (END(&ld->lck.lox.lld) < END(&flp->lck.lox.lld)) {
			if (END(&ld->lck.lox.lld) == flp->lck.lox.lld.l_start-1)
				regntype |= E_START;
			else if (END(&ld->lck.lox.lld) < flp->lck.lox.lld.l_start)
				regntype |= E_BEFORE;
			else
				regntype |= E_MIDDLE;
		} else if (END(&ld->lck.lox.lld) == END(&flp->lck.lox.lld))
			regntype |= E_END;
		else
			regntype |= E_AFTER;
	} else {
		if (ld->lck.lox.lld.l_start > flp->lck.lox.lld.l_start) {
                        if (ld->lck.lox.lld.l_start-1 == flp->lck.lox.lld.l_len)
                                return S_END|E_AFTER;     
                        if (ld->lck.lox.lld.l_start > flp->lck.lox.lld.l_len)
                                return S_AFTER|E_AFTER;   
                        regntype = S_MIDDLE;           
                } else if (ld->lck.lox.lld.l_start == flp->lck.lox.lld.l_start)
                        regntype = S_START;
                else
                        regntype = S_BEFORE;
                                            
                if (ld->lck.lox.lld.l_len < flp->lck.lox.lld.l_len) {
                        if (ld->lck.lox.lld.l_len == flp->lck.lox.lld.l_start-1)                                regntype |= E_START;      
                        else if (ld->lck.lox.lld.l_len < flp->lck.lox.lld.l_start) 
                                regntype |= E_BEFORE;     
                        else                         
                                regntype |= E_MIDDLE;
                } else if (ld->lck.lox.lld.l_len == flp->lck.lox.lld.l_len)
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
 * Insrtp is a result from the routine blocked_reclox().  Flckadj() scans
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
flckadj_reclox(lck_list, insrtp, ld)
	struct reclock	**lck_list;
	struct reclock *insrtp;
	struct reclock	*ld;
{
	struct	reclock	*flp, *nflp;
	int regtyp;

	if (debug)
		printf("enter flckadj_reclox(): start %d len %d type %d\n",
			ld->lck.lox.lld.l_start, ld->lck.lox.lld.l_len, 
			ld->lck.lox.lld.l_type);

	nflp = (insrtp == (struct reclock *)NULL) ? *lck_list : insrtp;

	while (flp = nflp) {
		nflp = flp->next;
		if (debug)
			printf("SAMEOWNER(&(flp->lck.lox.lld), &(ld->lck.lox.lld))=%d \n",
				SAMEOWNER(&(flp->lck.lox.lld), &(ld->lck.lox.lld)));
		if( SAMEOWNER(&(flp->lck.lox.lld), &(ld->lck.lox.lld)) ) {

			/* Release already locked region if necessary */
			if (debug) {
				printf("calling regflck_reclox()..\n");
				printf("flp: start %d len %d type %d\n",
                        		flp->lck.lox.lld.l_start, 
					flp->lck.lox.lld.l_len, 
					flp->lck.lox.lld.l_type);
			}

			switch (regtyp = regflck_reclox(ld, flp)) {
			case S_BEFORE|E_BEFORE:
				if (debug)
					printf("S_BEFORE|E_BEFORE\n");
				nflp = NULL;
				break;
			case S_BEFORE|E_START:
				if (debug)
					printf("S_BEFORE|E_START\n");
				if (ld->lck.lox.lld.l_type == flp->lck.lox.lld.l_type) {
					if (flp->lck.lox.lld.l_len && flp->lck.lox.lld.l_len != MAXEND)
						ld->lck.lox.lld.l_end = END(&flp->lck.lox.lld) + 
							1 - ld->lck.lox.lld.l_start;
					else
						ld->lck.lox.lld.l_end = flp->lck.lox.lld.l_len +
                                                        1 - ld->lck.lox.lld.l_start;
					delflck_reclox(lck_list, flp);
				}
				nflp = NULL;
				break;
			case S_START|E_END:
				if (debug)
					printf("S_START|E_END\n");
				/*
				 * Don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->lck.lox.lld.l_type == flp->lck.lox.lld.l_type)
					return 0;
			case S_START|E_AFTER:
				if (debug)
					printf("S_START|E_AFTER\n");
				insrtp = flp->prev;
				delflck_reclox(lck_list, flp);
				break;
			case S_BEFORE|E_END:
				if (debug)
					printf("S_BEFORE|E_END\n");
				if (ld->lck.lox.lld.l_type == flp->lck.lox.lld.l_type)
					nflp = NULL;
			case S_BEFORE|E_AFTER:
				if (debug)
					printf("S_BEFORE|E_AFTER\n");
				delflck_reclox(lck_list, flp);
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
				if (ld->lck.lox.lld.l_type == flp->lck.lox.lld.l_type)
					return 0;
			case S_BEFORE|E_MIDDLE:
				if (debug)
					printf("S_BEFORE|E_MIDDLE\n");
				if (ld->lck.lox.lld.l_type == flp->lck.lox.lld.l_type)
					if (flp->lck.lox.lld.l_len && flp->lck.lox.lld.l_len != MAXEND)
						ld->lck.lox.lld.l_end = END(&flp->lck.lox.lld) - 
							ld->lck.lox.lld.l_start + 1;
					else
						ld->lck.lox.lld.l_end = flp->lck.lox.lld.l_len -                                
                                                        ld->lck.lox.lld.l_start + 1;
				else {
					/* setup piece after end of (un)lock */
					register struct	reclock *tdi, *tdp;
					struct reclock td;

					td.lck.lox.lld.l_type = flp->lck.lox.lld.l_type;
					if (ld->lck.lox.lld.l_len && ld->lck.lox.lld.l_len != MAXEND)
						td.lck.lox.lld.l_start = END(&ld->lck.lox.lld) + 1;
					else 
						td.lck.lox.lld.l_start = ld->lck.lox.lld.l_len + 1;
					td.block = ld->block;
        				td.exclusive = ld->exclusive;
        				td.reclaim = ld->reclaim;
        				td.state = ld->state;
        				td.rel = ld->rel;
        				if (debug)
                				printf("Before copying ld->lck.fh...\n");
        				obj_copy(&td.lck.fh, &ld->lck.fh);
        				obj_copy(&td.cookie, &ld->cookie);
        				td.lck.op = ld->lck.op;
        				td.lck.svid = ld->lck.svid;
        				if (debug)
                				printf("Before copying td.lck.oh...\n");
        				obj_copy(&td.lck.oh, &ld->lck.oh);
        				td.lck.clnt = ld->lck.clnt;
        				td.lck.caller_name = ld->lck.caller_name;
        				td.lck.lox.granted = ld->lck.lox.granted;
        				td.lck.lox.color = ld->lck.lox.color;
        				td.lck.lox.LockID = ld->lck.lox.LockID;
        				td.lck.lox.class = ld->lck.lox.class;
        				if (debug)
                				printf("Before copying ld->lck.lox.lld...\n");
        				td.lck.lox.lld.l_sysid = ld->lck.lox.lld.l_sysid;
        				td.lck.lox.lld.l_start = ld->lck.lox.lld.l_start;
        				td.lck.lox.lld.l_len = ld->lck.lox.lld.l_len;
        				td.lck.lox.lld.l_whence = ld->lck.lox.lld.l_whence;
        				td.lck.lox.lld.l_pid = ld->lck.lox.lld.l_pid;
        				td.lck.lox.lld.l_type = ld->lck.lox.lld.l_type;
        				td.lck.lox.lld.pad[0] = ld->lck.lox.lld.pad[0];
        				td.w_flag = 0;
					tdp = tdi = flp;
					do {
						if (debug)
							printf("tdp->lck.lox.lld.l_start %d td.lck.lox.lld.l_start %d\n",tdp->lck.lox.lld.l_start, td.lck.lox.lld.l_start);
						if (tdp->lck.lox.lld.l_start < td.lck.lox.lld.l_start)
							tdi = tdp;
						else
							break;
					} while (tdp = tdp->next);
					if (flp->lck.lox.lld.l_len && flp->lck.lox.lld.l_len != MAXEND)
                                        	td.lck.lox.lld.l_len = END(&flp->lck.lox.lld) - 
							td.lck.lox.lld.l_start + 1;
					else
						td.lck.lox.lld.l_len = flp->lck.lox.lld.l_len -                                 
                                                        td.lck.lox.lld.l_start + 1;
					if (debug)
						printf("td.lck.lox.lld.l_start %d td.lck.lox.lld.l_end %d tdi->lck.lox.lld.l_start %d tdi->lck.lox.lld.l_len %d\n",
							td.lck.lox.lld.l_start, td.lck.lox.lld.l_end,
							tdi->lck.lox.lld.l_start, tdi->lck.lox.lld.l_len);
					if (insflck_reclox(lck_list, &td, tdi) == NULL)
						return ENOLCK;
				}
				if (regtyp == (S_MIDDLE|E_MIDDLE)) {
					if (debug)
						printf("S_MIDDLE|E_MIDDLE\n");	
					/* setup piece before (un)lock */
					flp->lck.lox.lld.l_end = ld->lck.lox.lld.l_start - 
						flp->lck.lox.lld.l_start;
					if (debug)
						printf("flp->lck.lox.lld.l_end=%d\n",
							flp->lck.lox.lld.l_end);
					insrtp = flp;
				} else
					delflck_reclox(lck_list, flp);
				nflp = NULL;
				break;
			case S_MIDDLE|E_END:
				if (debug)
					printf("S_MIDDLE|E_END\n");
				/*
				 * Don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->lck.lox.lld.l_type == flp->lck.lox.lld.l_type)
					return 0;
				flp->lck.lox.lld.l_end = ld->lck.lox.lld.l_start - 
					flp->lck.lox.lld.l_start;
				insrtp = flp;
				break;
			case S_MIDDLE|E_AFTER:
				if (debug)
					printf("S_MIDDLE|E_AFTER: ld->lck.lox.lld.l_type %d flp->lck.lox.lld.l_type %d\n", ld->lck.lox.lld.l_type, flp->lck.lox.lld.l_type);
				if (ld->lck.lox.lld.l_type == flp->lck.lox.lld.l_type) {
					if (debug) 
                                                printf("ld->lck.lox.lld.l_type == flp->lck.lox.lld.l_type\n");
                                        ld->lck.lox.lld.l_len += ld->lck.lox.lld.l_start - flp->lck.lox.lld.l_start;
					ld->lck.lox.lld.l_start = flp->lck.lox.lld.l_start;
					insrtp = flp->prev;
					delflck_reclox(lck_list, flp);
				} else {
					if (debug)
						printf("ld->lck.lox.lld.l_type != flp->lck.lox.lld.l_type\n");
					flp->lck.lox.lld.l_end = ld->lck.lox.lld.l_start - flp->lck.lox.lld.l_start;
					insrtp = flp;
				}
				break;
			case S_END|E_AFTER:
				if (debug)
					printf("S_END|E_AFTER\n");
				if (ld->lck.lox.lld.l_type == flp->lck.lox.lld.l_type) {
                                        ld->lck.lox.lld.l_len += ld->lck.lox.lld.l_start - flp->lck.lox.lld.l_start;
					ld->lck.lox.lld.l_start = flp->lck.lox.lld.l_start;
					insrtp = flp->prev;
					delflck_reclox(lck_list, flp);
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
		printf("ld->lck.lox.lld.l_type=%d ld->lck.lox.lld.l_start %d ld->lck.lox.lld.l_end %d END(&ld->lck.lox.lld) %d\n",
			ld->lck.lox.lld.l_type,ld->lck.lox.lld.l_start,ld->lck.lox.lld.l_end,END(&ld->lck.lox.lld));
	if (ld->lck.lox.lld.l_type && ld->lck.lox.lld.l_type != F_UNLCK) {
		if (flp = insrtp) {
			do {
				if (flp->lck.lox.lld.l_start < ld->lck.lox.lld.l_start)
					insrtp = flp;
				else
					break;
			} while (flp = flp->next);
		}
		if (insflck_reclox(lck_list, ld, insrtp) == NULL)
			return ENOLCK;
	}

	return 0;
}

/*
 * blocked_reclox() checks whether a new lock (lckdat) would be
 * blocked by a previously set lock owned by another process.
 * Insrt is set to point to the lock where lock list updating
 * should begin to place the new lock.
 */ 
struct reclock *
blocked_reclox(flp, lckdat, insrt)
        struct reclock *flp;
        struct flock  *lckdat;
        struct reclock **insrt;
{
        register struct reclock *f;

        if (debug)
                printf("enter blocked_reclox() : start %d len %d type %d\n",
                        lckdat->l_start, lckdat->l_len, lckdat->l_type);

        *insrt = NULL;
        for (f = flp; f != NULL; f = f->next) {
                if (f->lck.lox.lld.l_start < lckdat->l_start)
                        *insrt = f;
                else
                        break;
		if (f->lck.lox.lld.l_len && f->lck.lox.lld.l_len != MAXEND) {
                	if( SAMEOWNER(&(f->lck.lox.lld), lckdat) ) {
                        	if ((lckdat->l_start-1) <= END(&f->lck.lox.lld))
                                	break;
                	} else if (lckdat->l_start <= END(&f->lck.lox.lld) && 
                  		(f->lck.lox.lld.l_type == F_WRLCK ||
                    		(f->lck.lox.lld.l_type == F_RDLCK && lckdat->l_type == F_WRLCK)))
                        	return f;
		} else {
			if( SAMEOWNER(&(f->lck.lox.lld), lckdat) ) {
                                if ((lckdat->l_start-1) <= f->lck.lox.lld.l_len)
					break;        
                        } else if (lckdat->l_start <= f->lck.lox.lld.l_len && 
                                (f->lck.lox.lld.l_type == F_WRLCK ||
                                (f->lck.lox.lld.l_type == F_RDLCK && lckdat->l_type == F_WRLCK))) 
                                return f;
		}
        }

	for (; f != NULL; f = f->next) {
		if (lckdat->l_len && lckdat->l_len != MAXEND) {
                	if (END(lckdat) < f->lck.lox.lld.l_start)
                        	break;
                	if (lckdat->l_start <= END(&f->lck.lox.lld) 
                  		&& ( !SAMEOWNER(&(f->lck.lox.lld), lckdat) )
                  		&& (f->lck.lox.lld.l_type == F_WRLCK
                    		|| (f->lck.lox.lld.l_type == F_RDLCK
                            	&& lckdat->l_type == F_WRLCK)))
                        	return f;
		} else {
			if (lckdat->l_len < f->lck.lox.lld.l_start)
                                break;
                        if (lckdat->l_start <= f->lck.lox.lld.l_len
                                && ( !SAMEOWNER(&(f->lck.lox.lld), lckdat) )
                                && (f->lck.lox.lld.l_type == F_WRLCK
                                || (f->lck.lox.lld.l_type == F_RDLCK
                                && lckdat->l_type == F_WRLCK)))
                                return f;
		}
        }

        return NULL;
}
