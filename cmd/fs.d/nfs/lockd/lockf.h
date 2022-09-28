/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/lockf.h	1.6.3.1"
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
/* constants and structures for the locking code ... */


struct data_lock {
        struct data_lock *Next,		/* Next lock in the list           */
                        *Prev,		/* Previous Lock in the list       */
			*NextProc;	/* Link list of process' locks	   */
        struct process_locks *MyProc;	/* Points to process lock list     */
	struct filock	filocks;
	int		granted,        /* The granted flag                */
			color,          /* Used during deadlock search     */
			LockID,         /* ID of this lock                 */
			class;          /* Class of lock (FILE,IO,NET)     */
        };

/* process lock structure holds locks owned by a given process */
struct process_locks {
	long		pid;
	struct process_locks *next;
	struct data_lock *lox;
	};

#define END(l)          ((l)->l_start + (l)->l_len - 1)
 
/* Is TRUE if a is completely contained within b */
#define WITHIN(a,b) (((a)->l_start >= (b)->l_start) && (END(a) <= END(b)))

int local_state;
