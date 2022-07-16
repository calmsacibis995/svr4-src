/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_FLOCK_H
#define _SYS_FLOCK_H

#ident	"@(#)head.sys:sys/flock.h	11.11.2.1"

#define	INOFLCK		1	/* Vnode is locked when reclock() is called. */
#define	SETFLCK		2	/* Set a file lock. */
#define	SLPFLCK		4	/* Wait if blocked. */
#define	RCMDLCK		8	/* RGETLK/RSETLK/RSETLKW specified */

#define IGN_PID		(-1)	/* ignore epid when cleaning locks */

/* file locking structure (connected to vnode) */

#define l_end 		l_len
#define MAXEND  	017777777777

typedef struct filock {
	struct	flock set;	/* contains type, start, and end */
	union	{
		int wakeflg;	/* for locks sleeping on this one */
		struct {
			long sysid;
			pid_t pid;
		} blk;			/* for sleeping locks only */
	}	stat;
#ifdef	u3b
	int	wakesem;
#endif
	struct	filock *prev;
	struct	filock *next;
} filock_t;

/* file and record locking configuration structure */
/* record use total may overflow */
struct flckinfo {
	long reccnt;	/* number of records currently in use */
	long rectot;	/* number of records used since system boot */
};

extern struct flckinfo	flckinfo;

#if defined(__STDC__)
int	reclock(struct vnode *, struct flock *, int, int, off_t);
#else
int	reclock();
#endif

#endif	/* _SYS_FLOCK_H */
