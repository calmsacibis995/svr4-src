/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_EVSYSCALL_H
#define _SYS_EVSYSCALL_H

#ident	"@(#)head.sys:sys/evsyscall.h	1.6.8.1"
/*			File Contents
**			=============
**
**	This file contains the data concerned with the evsys system
**	call.  This system call is the interface between user mode and
**	the kernel.  The only other events system call is evtrapret for
**	returning from event trap handlers.  Of course, since events
**	are implemented as a VFS, lots of other system calls (open,
**	close, etc.) also work on event queues.  Users are not expected
**	to do invoked the evsys system call directly but rather to
**	invoke the library routines such as evpost, evpoll, etc. which,
**	in turn, do the system call.  For this reason, this header file
**	is not intended to be included in user's programs.
*/


/*			Required Header Files
**			=====================
**
**	The following header files must be includes before including
**	this file.
**
**		REQUIRES	sys/types.h
**		REQUIRES	sys/signal.h
**		REQUIRES	sys/evecb.h
**		REQUIRES	sys/hrtcntl.h
**		REQUIRES	sys/priocntl.h
**		REQUIRES	sys/procset.h
**		REQUIRES	sys/events.h
*/

/*			The evsys System Call
**			=====================
**
**	The evsys system call is used to obtain a direct entry into
**	the events code without going through the VFS interface.  This
**	is necessary in some cases because no file descriptor is
**	required with certain events function calls (e.g., evcntl).
**	Without a file descriptor referring to an event queue (file),
**	it is impossible to go through the VFS mechanism.  We always
**	use evsys rather than an existing system call such as ioctl
**	so that we always get into the events code, even for errors.
**	This enables us to give more specific error messages for some
**	cases.  In addition, extension of events to work over RFS will
**	require that the ioctl interface not be used.
**	The following is the list of functions which are implemented
**	through the evsys interface.  The comment with each command
**	indicates the value of the variable third argument to the
**	function.  This argument appears in the evsa_arg member of the
**	evsys_args_t structure defined at the end of this file.
*/

typedef enum evscmds {

	EVS_EVPOST,	/* An evpost function.			*/
			/* evsa_arg = (evsys_post_t *).		*/
	EVS_EVPOLL,	/* An evpoll function.			*/
			/* evsa_arg = (evsys_poll_t *).		*/
	EVS_EVPOLLMORE,	/* An evpollmore function.		*/
			/* evsa_arg = (evsys_pollmore_t *).	*/
	EVS_EVTRAP,	/* An evtrap function.			*/
			/* evsa_arg = (evsys_trap_t *).		*/
	EVS_EVTRAPCAN,	/* An evtrapcancel function.		*/
			/* evsa_arg = (evsys_trapcancel_t *).	*/
	EVS_EVCNTL,	/* An evcntl function.			*/
			/* evsa_arg = (evsys_cntl_t *).		*/
	EVS_EVQCNTL,	/* An evqcntl function.			*/
			/* evsa_arg = (evsys_qcntl_t *).	*/
	EVS_EVEXIT,	/* An evexit with eqid = EQ_NOQUEUE.	*/
			/* evsa_arg = (evsys_exit_t *).		*/
	EVS_EVSIG	/* An evsig with eqid = EQ_NOQUEUE.	*/
			/* evsa_arg = (evsys_sig_t *).		*/
} evscmds_t;

/*			The evsys System Call (Continued)
**			=================================
**
**	The following structure describes the arguments to the evsys
**	system call.
**
**	The field evsa_ver below should really be type evver_t.
**	However, this type is a short and the following structure
**	describes arguments to a system call which, like all function
**	arguments, are passed as int's.
*/

typedef struct evsys_args {
	evscmds_t	evsa_cmd;	/* One of the EVS_XXXX	*/
					/* commands listed	*/
					/* above.		*/
	int		evsa_ver;	/* A version number	*/
					/* like EV_VERSION.	*/
	_VOID		*evsa_arg;	/* The actual argument.	*/
					/* A pointer to one of	*/
					/* the structures	*/
					/* defined below.	*/
} evsys_args_t;

/*			The evpost Interface
**			====================
**
**	The following structure is pointed to by "arg" on the evsys
**	system call for an evpost function.
*/

typedef struct evsys_post {
	event_t	*evs_post_elp;		/* Pointer to array of	*/
					/* event structures to	*/
					/* post.		*/
	int	evs_post_els;		/* Nbr of structures in	*/
					/* list.		*/
	int	evs_post_flags;		/* Flags passed with	*/
					/* evpost.		*/
} evsys_post_t;

/*			The evpoll Interface
**			====================
**
**	The following structure is pointed to by "arg" on the evsys
**	system call for an evpoll function.
*/

typedef struct evsys_poll {
	evpollcmds_t	evs_poll_cmd;	/* The poll cmd.  One	*/
					/* of the EC_XXXX 	*/
					/* commands defined in	*/
					/* events.h.		*/
	event_t		*evs_poll_elp;	/* Ptr to array of	*/
					/* events to poll for.	*/
	int		evs_poll_els;	/* Nbr of events in the	*/
					/* array.		*/

	hrtime_t	*evs_poll_top;	/* Ptr to the hrtime_t	*/
					/* structure giving the	*/
					/* timeout for the poll	*/
					/* or NULL (immediate	*/
					/* return) or -1 (wait	*/
					/* forever).		*/
} evsys_poll_t;

/*			The evpollmore Interface
**			========================
**
**	The following structure is pointed to by "arg" on the evsys
**	system call for the evpollmore function.
*/

typedef struct evsys_pollmore {
	event_t		*evs_pollmore_elp;
					/* Ptr to array of	*/
					/* events to poll for.	*/

	int		evs_pollmore_els;
					/* Nbr of events in the	*/
					/* array.		*/
} evsys_pollmore_t;

/*			The evtrap Interface
**			====================
**
**	The following structure is pointed to by "arg" on the evsys
**	system call for the evtrap function.
*/

typedef struct evsys_trap {
	evpollcmds_t	evs_trap_cmd;	/* The trap cmd.  One	*/
					/* of the EC_XXX 	*/
					/* commands defined in	*/
					/* events.h		*/
	event_t		*evs_trap_elp;	/* Ptr to array of	*/
					/* events to trap for.	*/
	int		evs_trap_els;	/* Nbr of events in the	*/
					/* array.		*/
	long		evs_trap_tid;	/* The trap identifier.	*/

	void		(*evs_trap_lfunc)();
					/* The library routine	*/
					/* to call a user's 	*/
					/* trap handler.	*/

	void		(*evs_trap_ufunc)();
					/* The user's trap	*/
					/* handler to call.	*/

	evta_t	*evs_trap_tap;	/* The optional trap	*/
					/* argument.		*/
} evsys_trap_t;

/*			The evtrapcancel Interface
**			==========================
**
**	The following structure is pointed to by "arg" on the evsys
**	system call for the evtrapcancel function.
*/

typedef struct evsys_trapcan {
	long		*evs_trapcan_tidp;
					/* Ptr to the array of	*/
					/* trap identifiers	*/
					/* which are to be	*/
					/* cancelled.		*/
	int		evs_trapcan_tids;
					/* Size of the array	*/
					/* evs_trapcan_tidp.	*/
} evsys_trapcan_t;

/*			The evcntl Interface
**			====================
**
**	The following structure is pointed to by "arg" on the evsys
**	system call for the evcntl function.
*/

typedef struct evsys_cntl {
	evcntlcmds_t	evs_cntl_cmd;	/* The command argument.*/
	long		evs_cntl_arg1;	/* First argument.	*/
	long		evs_cntl_arg2;	/* Second argument.	*/
} evsys_cntl_t;





/*			The evqcntl Interface
**			=====================
**
**	The following structure is pointed to by "arg" on the evsys
**	system call for the evqcntl function.
*/

typedef struct evsys_qcntl {
	int		evs_qcntl_eqd;	/* The event queue	*/
					/* descriptor (file	*/
					/* descriptor) for the	*/
					/* queue to access.	*/
	evqcntlcmds_t	evs_qcntl_cmd;	/* The value of "cmd"	*/
					/* from the evqcntl	*/
					/* call.  Will be one	*/
					/* of EC_XXXX from	*/
					/* events.h.		*/
	long		evs_qcntl_arg;	/* The value of "arg"	*/
					/* from the evqcntl	*/
					/* call.		*/
} evsys_qcntl_t;

/*			The evexit Interface
**			====================
**
**	The following structure is pointed to by "arg" on the evsys
**	system call for the evexit function.
*/

typedef struct evsys_exit {
	procset_t	*evs_exit_psp;	/* Ptr to the process	*/
					/* set structure which	*/
					/* defines the set of	*/
					/* processes which are	*/
					/* to be operated on.	*/
	hostid_t	evs_exit_hostid;/* The id of the host	*/
					/* system to which the	*/
					/* process set applies.	*/

	ecb_t	*evs_exit_ecbp;	/* Ptr to the ecb for	*/
					/* the event to be	*/
					/* posted.		*/
} evsys_exit_t;





/*			The evsig Interface
**			===================
**
**	The following structure is pointed to by "arg" on the evsys
**	system call for the evsig function.
*/

typedef struct evsys_sig {
	sigset_t	*evs_sig_setp;	/* Ptr to the set of	*/
					/* signals to which 	*/
					/* this call applies.	*/

	ecb_t	*evs_sig_ecbp;		/* Ptr to the ecb for	*/
					/* the event to be 	*/
					/* posted.		*/
	evsiginfo_t	*evs_sig_silp;	/* Ptr to list of	*/
					/* structures to use 	*/
					/* for returning the	*/
					/* old status of the	*/
					/* signals.		*/
	int		evs_sig_sils;	/* Nbr of elements in	*/
					/* the evs_sig_silp	*/
					/* list.		*/
} evsys_sig_t;

#endif	/* _SYS_EVSYSCALL_H */
