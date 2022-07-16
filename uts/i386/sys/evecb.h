/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_EVECB_H
#define _SYS_EVECB_H

#ident	"@(#)head.sys:sys/evecb.h	1.4.8.1"

/*			File Contents
**			=============
**
**	This file contains the definition of an event control block,
**	or ecb.  This was originally in the events.h header file.
**	It was broken out separately because other header files need
**	the definition of an ecb because they include an ecb as one
**	member of a larger structure that they define.  Note that they
**	include an ecb and not a pointer to an ecb.  One such file is
**	hrtcntl.h.  On the other hand, events.h requires hrtcntl.h to
**	be included before itself because an event_t contains a tofd_t
**	structure (again, not a pointer).
*/


/*			Required Header Files
**			=====================
**
**	The following header files must be includes before including
**	this file.
**
**		REQUIRES	sys/types.h
*/

/*			The ecb_t Structure
**			===================
**
**	The following structure describes an event control block.
**	It is used by many of system calls to specify the parameters of
**	an event that the system is to post for some situation.  See
**	events.h for more details.
*/

typedef struct ecb {
	short	ecb_eqd;	/* The event queue descriptor	*/
				/* to which the event should be	*/
				/* posted.			*/
	ushort	ecb_flags;	/* Various flags defined below.	*/
	long	ecb_eid;	/* The value for the ev_eid	*/
				/* field of the event to be	*/
				/* posted.			*/
	long	ecb_evpri;	/* The value for the ev_pri	*/
				/* field of the event to be	*/
				/* posted.			*/
} ecb_t;


/*	The following are the flags for the ecb_flags field of the
**	ecb_t structure.
*/

#define	ECBF_POSTCAN	0x0001	/* Post an event if the		*/
				/* operation is cancelled.  The	*/
				/* EF_CANCEL flag will be set	*/
				/* in the posted event.  By	*/
				/* default, events are not 	*/
				/* posted when an operation is	*/
				/* cancelled.  This flag 	*/
				/* applies only to the asyncio	*/
				/* and hrtalarm functions.	*/
#define	ECBF_LATEERR	0x0002	/* This flag applies only to	*/
				/* the hrtalarm function.  If a	*/
				/* T_TODALARM specifies a time	*/
				/* which has passed, give an 	*/
				/* error return.  The default	*/
				/* is to post the normal	*/
				/* ET_TIMER event immediately.	*/
#define	ECBF_NOSIGQ	0x0004	/* This flag applies only to	*/
				/* the evsig function.  If set,	*/
				/* then don't post a separate	*/
				/* event for each signal of the	*/
				/* same type which occurs. 	*/
				/* Post only if no ET_SIG event	*/
				/* for the same signal is 	*/
				/* already posted but not yet	*/
				/* received.			*/

#endif	/* _SYS_EVECB_H */
