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

#ifndef _VM_MP_H
#define _VM_MP_H

#ident	"@(#)kern-vm:mp.h	1.3"

/*
 * VM - multiprocessor/ing support.
 *
 * Currently the mon_enter() / mon_exit() pair implements a
 * simple monitor for objects protected by the appropriate lock.
 * The cv_wait() / cv_broadcast pait implements a simple
 * condition variable which can be used for `sleeping'
 * and `waking' inside a monitor if some resource
 * is needed which is not available.
 */

typedef struct mon_t {
	unchar	dummy;
} mon_t;

#if defined(DEBUG) || defined(lint)
void	mon_enter(/* lk */);
void	mon_exit(/* lk */);
void	cv_wait(/* lk, cond */);
void	cv_broadcast(/* lk, cond */);

#else

/*
 * mon_enter is used as a type of multiprocess semaphore
 * used to implement a monitor where the lock represents
 * the ability to operate on the associated object.
 * For now, the lock/object association is done
 * by convention only.
 * For single processor systems that are debugged, no lock is needed.
 * For multiprocessor systems that are debugged, a simple lock suffices.
 * Only the single processor macros are included.
 */

#define mon_enter(lk)
#define mon_exit(lk)
#define cv_wait(lk, cond) ((void) sleep(cond, PSWP+1))
#define cv_broadcast(lk, cond) (wakeprocs(cond, PRMPT))

#endif	/* DEBUG */

#define	lock_init(lk)	(lk)->dummy = 0

#endif	/* _VM_MP_H */
