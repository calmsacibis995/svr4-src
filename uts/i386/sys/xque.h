/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/xque.h	1.1.2.1"

#ifndef _SYS_XQUE_H
#define _SYS_XQUE_H

/*
 * Keyboard/mouse event queue entries
 */

typedef struct xqEvent {
	unchar	xq_type;	/* event type (see below) */
	unchar	xq_code;	/* when xq_type is XQ_KEY, => scan code;
				   when xq_type is XQ_MOTION or XQ_BUTTON, =>
					bit 0 clear if right button pushed;
					bit 1 clear if middle button pushed;
					bit 2 clear if left button pushed; */
	char	xq_x;		/* delta x movement (mouse motion only) */
	char	xq_y;		/* delta y movement (mouse motion only) */
	time_t	xq_time; 	/* event timestamp in "milliseconds" */
} xqEvent;

/*	xq_type values		*/

#define XQ_BUTTON	0	/* button state change only */
#define XQ_MOTION	1	/* mouse movement (and maybe button change) */
#define XQ_KEY		2	/* key pressed or released */

/*
 * The event queue
 */

typedef struct xqEventQueue {
	char	xq_sigenable;	/* allow signal when queue becomes non-empty
				   0 => don't send signals
				   non-zero => send a signal if queue is empty
				      and a new event is added */
	int	xq_head;	/* index into queue of next event to be dequeued */
	int	xq_tail;	/* index into queue of next event slot to be filled */
	time_t	xq_curtime;	/* time in milliseconds since 1/1/70 GMT */
	int	xq_size;	/* number of elements in xq_events array */
	xqEvent	xq_events[1];	/* configurable-size array of events */
} xqEventQueue;

#ifdef _KERNEL

/*
 * The driver's private data structure to keep track of xqEventQueue
 */

typedef struct xqInfo {
	xqEventQueue	*xq_queue;	/* pointer to the xqEventQueue structure */
	int	xq_ptail;	/* private copy of xq_tail */
	int	xq_psize;	/* private copy of xq_size */
	int	xq_signo;	/* signal number to send for xq_sigenable */
	proc_t	*xq_proc;	/* pointer to x server process (for signalling) */
	int	xq_pid;		/* process id of server process */
	struct xqInfo	*xq_next,	/* next xqInfo structure in list */
			*xq_prev;	/* previous xqInfo structure in list */
	addr_t	xq_uaddr;
	unsigned	xq_npages;
} xqInfo;

#endif	/* _KERNEL */

caddr_t xq_init();

#endif	/* _SYS_XQUE_H */
