/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_POLL_H
#define _SYS_POLL_H

#ident	"@(#)head.sys:sys/poll.h	11.9.7.1"

/*
 * Structure of file descriptor/event pairs supplied in
 * the poll arrays.
 */
struct pollfd {
	int fd;				/* file desc to poll */
	short events;			/* events of interest on fd */
	short revents;			/* events that occurred on fd */
};

/*
 * Testable select events 
 */
#define POLLIN		0x0001		/* fd is readable */
#define POLLPRI		0x0002		/* high priority info at fd */
#define	POLLOUT		0x0004		/* fd is writeable (won't block) */
#define POLLRDNORM	0x0040		/* normal data is readable */
#define POLLWRNORM	POLLOUT
#define POLLRDBAND	0x0080		/* out-of-band data is readable */
#define POLLWRBAND	0x0100		/* out-of-band data is writeable */

#define POLLNORM	POLLRDNORM

/*
 * Non-testable poll events (may not be specified in events field,
 * but may be returned in revents field).
 */
#define POLLERR		0x0008		/* fd has error condition */
#define POLLHUP		0x0010		/* fd has been hung up on */
#define POLLNVAL	0x0020		/* invalid pollfd entry */

/*
 * Poll list head structure.  A pointer to this is passed
 * to pollwakeup() from the caller indicating the event has
 * occurred.  NOTE: First two pointers correspond to first
 * two elements in polldat structure.
 */
struct pollhead {
	struct polldat	*ph_list;	/* list of pollers */
	struct polldat	*ph_dummy;	/* dummy pointer */
	short		ph_events;	/* events pending on list */
	long		ph_filler[5];	/* reserved for future use */
};

/*
 * Data necessary to notify process sleeping in poll(2)
 * when an event has occurred.
 */
struct polldat {
	struct polldat *pd_next;	/* next in poll list */
	struct polldat *pd_prev;	/* previous in poll list */
	struct polldat *pd_chain;	/* other fds polled in same call */
	short		pd_events;	/* events being polled */
	struct pollhead *pd_headp;	/* backpointer to head of list */
	void		(*pd_fn)();	/* function to call */
	long		pd_arg;		/* argument to function */
};

/*
 * Routine called to notify a process of the occurrence
 * of an event.
 */
extern void pollwakeup();

/*
 * Internal routines.
 */
extern void polltime(), pollrun(), polladd(), polldel();

#if defined(__STDC__) && !defined(_KERNEL)
int poll(struct pollfd *, unsigned long, int);
#endif

#endif	/* _SYS_POLL_H */
