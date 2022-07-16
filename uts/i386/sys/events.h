/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_EVENTS_H
#define _SYS_EVENTS_H

#ident	"@(#)head.sys:sys/events.h	1.6.4.1"
/*			File Contents
**			=============
**
**	This file contains data which must be included in an 
**	application program using the events facility.
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
*/

/*			The Events VFS Version Number
**			=============================
**
**	In order to be able to make changes to the events VFS in the
**	future, we define a version number which is automatically passed
**	through from the application program to the libraries and then
**	to the kernel.  If we want to change the events interface, we
**	just increase the version number so the events library routines
**	and the kernel can tell the difference.  This works with either
**	private or shared libraries.
**	The current version number is as follows.
*/

typedef ulong	evver_t;	/* Type for the version nbr.	*/

#define	EV_VERSION	2	/* The current version number.	*/

/*	The following are previously used version numbers which are
**	still supported.
*/

/*	None yet.
*/

/*	The following are previously used version numbers which are no
**	longer supported.
*/

#define	EV_VER_PROTO1	1	/* Version number used in the	*/
				/* first prototype of events.	*/

/*			The Events Function Interfaces
**			==============================
**
**	When a user program calls one of the functions described on one
**	of the events manual pages, it is actually invoking one of the
**	following macros.  The purpose of the macros is to pass the
**	version number to the library routines.  The library routines
**	will, in turn, pass the version number down to the kernel.  This
**	will allow us to make changes to the events VFS in the future 
**	and still support old *.o and a.out files.
**
**	Thanks to Jeff Smits for the idea of using macros here.
*/

#define	evpost(elp, els, flags)	\
	__evpost(EV_VERSION, elp, els, flags)

#define	evpoll(cmd, elp, els, top) \
	__evpoll(EV_VERSION, cmd, elp, els, top)

#define	evpollmore(elp, els) \
	__evpollmore(EV_VERSION, elp, els)

#define	evtrap(cmd, elp, els, tid, handlerp, tap) \
	__evtrap(EV_VERSION, cmd, elp, els, tid, handlerp, tap)

#define	evtrapcancel(tidp, tids) \
	__evtrapcancel(EV_VERSION, tidp, tids)
	
#define	evcntl(cmd, arg1, arg2) \
	__evcntl(EV_VERSION, cmd, arg1, arg2)

#define	evqcntl(eqd, cmd, arg) \
	__evqcntl(EV_VERSION, eqd, cmd, arg)

#define	evexit(idtype, id, ecbp) \
	__evexit(EV_VERSION, idtype, id, ecbp)

#define	evexitset(psp, hostid, ecbp) \
	__evexitset(EV_VERSION, psp, hostid, ecbp)

#define	evsig(sigsetp, ecbp, silp, sils) \
	__evsig(EV_VERSION, sigsetp, ecbp, silp, sils)

/*			Miscellaneous Definitions
**			=========================
**
**	In our implementation, an event queue identifier is just
**	a file descriptor since we implement events as a VFS.
**	The following value is intended to represent the absence
**	of an event queue.  Therefore, it must have a value that
**	could not be an event queue identifier; i.e., for us, a
**	file descriptor.  We don't want to use a negative value
**	since a negative return value usually indicates an error.
**	Only -1 should really be used to indicate an error but,
**	by convention, many programmers test for "rtrn < 0".
**	Therefore, the following is the largest positive value
**	which we can fit into an short.  Change it if necessary
**	when porting to another system.
*/

#define	EQ_NOQUEUE	0x7fff	/* No event queue.	*/


/*	The following symbol is used to represent the set of all trap
**	identifiers.  It is used in place of a pointer to a list of
**	trap identifiers so its value must be distinguishable from a
**	pointer.  It cannot be zero either since zero is used to
**	indicate that no trap identifiers are being specified.  Change
**	it if necessary when porting.
*/

#define	TR_ALL	((long *)-1)		/* All trap identifiers.*/

/*			The evtypes_t Enumeration
**			=========================
**
**	The following are the event types.  These are the values
**	for the ev_type field of the event_t structure.
*/

typedef enum evtypes {

		ET_POST,	/* A user event created with	*/
				/* the evpost function call.	*/
		ET_ASYNC,	/* Return from an asynchronous	*/
				/* system call such as asyncio.	*/
		ET_TIMER,	/* Expiration of a timer set by */
				/* the hrtalarm function call.	*/
		ET_SIG,		/* An event indicating the fact */
				/* that a signal was sent to a	*/
				/* process.  Posted due to an	*/
				/* evsig function call.		*/
		ET_STREAM,	/* A change of state of a	*/
				/* stream head.  Posted due to	*/
				/* an I_SETEV type streams	*/
				/* ioctl.			*/
		ET_DRIVER,	/* The event was posted by a	*/
				/* device driver or streams	*/
				/* module.			*/
		ET_EXIT		/* A message created on process */
				/* exit due to a previous call	*/
				/* of evexit.			*/
} evtypes_t;

/*			The event_t Structure
**			=====================
**
**	The following structure describes an event at user
**	level.  It is the parameter passed on many of the
**	events function calls.
*/


typedef struct event {
	int		ev_eqd;		/* The event queue	*/
					/* descriptor.  For us,	*/
					/* a file descriptor.	*/
	long		ev_flags;	/* Flags defined below.	*/
	long		ev_eid;		/* The event identifier.*/
	long		ev_eidlim;	/* High limit of event	*/
					/* identifiers for	*/
					/* range type match	*/
					/* (EF_EIDRNG set).	*/
	long		ev_pri;		/* The event's priority.*/
	hostid_t	ev_hostid;	/* The hostid of the	*/
					/* system on which the	*/
					/* process ev_pid was	*/
					/* running when it 	*/
					/* requested that the	*/
					/* event be posted.	*/
	int		ev_pfd;		/* A file descriptor	*/
					/* being passed with	*/
					/* the event (only if	*/
					/* EF_PFD is set).	*/
	evtypes_t	ev_type;	/* The event type as	*/
					/* defined above.	*/
	pid_t		ev_pid;		/* The process id of	*/
					/* the poster of the	*/
					/* event.		*/
	uid_t		ev_uid;		/* The effective user	*/
					/* id of the poster of	*/
					/* the event.		*/
	int		ev_error;	/* An error code.	*/
					/* Values are defined	*/
					/* in sys/errno.h.	*/
					/* This field has a	*/
					/* value only if 	*/
					/* EF_ERROR is set in	*/
					/* the ev_flags field.	*/
	hrtime_t	ev_time;	/* The time that the	*/
					/* event was posted.	*/
	size_t		ev_datamax;	/* Size of buffer for	*/
					/* event type dependent	*/
					/* data (in bytes).	*/
	size_t		ev_datasize;	/* Actual nbr of bytes	*/
					/* of event type	*/
					/* dependent data.	*/
	char		*ev_data;	/* Ptr to the dependent	*/
					/* data.		*/
} event_t;

/*			The event_t Structure (Continued)
**			=================================
**
**	The following is a list of flags that can be set in the
**	ev_flags field of the event_t structure.  They are divided
**	into various categories which are described following the
**	list.
*/

#define	EF_PRI		0x0001	/* Return the highest priority	*/
				/* event on the queue.		*/
				/* Otherwise, return the first	*/
				/* (FIFO order) event.		*/
#define	EF_TYPE		0x0002	/* The ev_type field must match	*/
				/* on an evpoll or evtrap.	*/
#define	EF_EID		0x0004	/* The ev_eid field must match	*/
				/* on an evpoll or evtrap.	*/
#define	EF_EIDRNG	0x0008	/* The eid must be greater than	*/
				/* or equal to ev_eid and less	*/
				/* than or equal to ev_eidlim 	*/
				/* on an ev_poll or ev_trap.	*/
#define	EF_SHM		0x0010	/* Event dependent data is to	*/
				/* be transferred in shared	*/
				/* memory.  Otherwise, copy	*/
				/* into and out of the kernel.	*/
#define	EF_DISCARD	0x0020	/* Receive event in discard	*/
				/* mode.  Throw away data which	*/
				/* won't fit in the buffer.	*/
				/* Otherwise, save it for a	*/
				/* possible evpollmore.		*/
#define	EF_QUICKD	0x0040	/* Data may be or has been sent	*/
				/* or returned in the ev_data	*/
				/* field of the event rather	*/
				/* than in a buffer.		*/
#define	EF_PFD		0x0080	/* Flag set if a file 		*/
				/* descriptor is being passed	*/
				/* in the ev_pfd field.		*/
#define	EF_TIME		0x0100	/* The ev_time field should be	*/
				/* supplied in the returned	*/
				/* event.  The ev_time.hrt_res	*/
				/* field has been set by the	*/
				/* caller.			*/
#define	EF_DONE		0x0200	/* The event has occurred.  	*/
				/* Otherwise, it is being	*/
				/* waited for.			*/
#define	EF_MORE		0x0400	/* There is more dependent data	*/
				/* with this event than buffer	*/
				/* space allocated.		*/
#define	EF_CANCEL	0x0800	/* Indicates that the event was */
				/* posted as a result of	*/
				/* something being cancelled	*/
				/* rather than completing.	*/
#define	EF_ERROR	0x1000	/* Indicates that an error has	*/
				/* occurred.			*/

/*			The event_t Structure (Continued)
**			=================================
**
**	The above flags are divided into the following categories.
**
**		EV_FLAGS_USER	Set by the user and interrogated by
**				the system.
**		EV_FLAGS_SYS	Set by the system and interrogated by
**				the user.
**		EV_FLAGS_POST	Subset of EV_FLAGS_USR that are valid
**				on an evpost function.  The other flags
**				(in EV_FLAGS_USR but not in
**				EV_FLAGS_POST) are valid only with
**				evpoll and evtrap.
**		EV_FLAGS_MORE	Subset of EV_FLAGS_USR.  The setting of
**				these flags on an evpollmore must
**				match their setting on the original
**				evpoll or evtrap.
*/

#define	EV_FLAGS_USER	(EF_PRI | EF_TYPE | EF_EID | EF_EIDRNG | \
			 EF_SHM | EF_DISCARD | EF_QUICKD | \
			 EF_PFD | EF_TIME)
#define	EV_FLAGS_SYS	(EF_DONE | EF_MORE  | EF_CANCEL | EF_ERROR)
#define	EV_FLAGS_POST	(EF_SHM | EF_QUICKD | EF_PFD)
#define	EV_FLAGS_MORE	(EF_PRI | EF_TYPE | EF_EID | EF_EIDRNG)


/*	The following symbol defines the maximum number of bytes of data
**	which can be passed in EF_QUICKD mode.  It must be equal to the
**	size of the ev_data field in the event_t structure defined
**	above.
*/

#define	EV_MAXQD	sizeof(char *)

/*			Data for the evpost Function
**			============================
**
**	The following are the possible flags which can be passed as
**	an argument to evpost.
*/

#define	EPF_NONBLOCK	0x0001	/* Don't wait if events can't	*/
				/* be posted immediately.	*/

/*			Data for the evpoll and evtrap Functions
**			========================================
**
**	The following are the possible commands for the evpoll and 
**	evtrap functions.
*/

typedef enum evpollcmds {

	EC_ONE,		/* Return one event.			*/
	EC_ANY,		/* Return any which are done, but at	*/
			/* least one.				*/
	EC_ALL		/* Return only if all are done.		*/
} evpollcmds_t;

/*	The following is the structure of the optional "tap" (trap
**	argument pointer) argument to evtrap.
*/

typedef struct evta {
	pcparms_t	*eta_prip;	/* Ptr to the scheduler	*/
					/* priority at which 	*/
					/* this trap handler is	*/
					/* to run or NULL if no	*/
					/* change in priority 	*/
					/* is requested.	*/
	long		*eta_hlp;	/* Ptr to a list of 	*/
					/* tid's to hold when	*/
					/* this handler is 	*/
					/* called or TR_ALL or	*/
					/* NULL to hold only	*/
					/* the tid being 	*/
					/* handled.		*/
	size_t		eta_hls;	/* Nbr of elements in	*/
					/* the hold array	*/
					/* eta_hlp.		*/
	uint		eta_flags;	/* Various flags as	*/
					/* defined below.	*/
} evta_t;

/*	The flags in eta_flags are as follows.
*/

#define	ETAF_RESTART	0x0001	/* Set if restartable system	*/
				/* calls are to be restarted	*/
				/* after calling the trap	*/
				/* handler for this trap id.	*/
#define	ETAF_ALTSTACK	0x0002	/* Set if this trap handler is	*/
				/* to be called on an alternate	*/
				/* stack.			*/

/*			Data for the evtrap Handler Function
**			====================================
**
**	The following is the type of the context argument which is
**	passed to a user trap handler.  Since a user program is not
**	suppossed to reference the contents of this struture, it is
**	defined in terms of another structure which is defined in
**	evsys.h
*/

typedef struct evcntxt	evcontext_t;

/*			Data for the evcntl Commands
**			============================
**
**	The following are the commands for the evcntl function.
*/

typedef enum evcntlcmds {
	EC_TRAPHOLD,	/* Add to the current set of held trap	*/
			/* identifiers.				*/
	EC_TRAPSET,	/* Redefine the contents of the current	*/
			/* set of held trap identifiers.	*/
	EC_TRAPRELSE,	/* Delete from the current set of held	*/
			/* trap identifiers.			*/
	EC_TRAPPAUSE,	/* An EC_TRAPRELSE followed by a 	*/
			/* pause() but done as one atomic op.	*/
	EC_TRAPRET,	/* Return from one or more trap 	*/
			/* handlers to a user specfied context.	*/
	EC_TRAPEND,	/* Process has exited all trap handlers.*/
	EC_ALTSTACK,	/* Specify use of an alternate stack 	*/
			/* when calling a trap handler.		*/
	EC_GETPRINFO,	/* Get the per-process event		*/
			/* information.				*/
	EC_SETPRINFO,	/* Set the per-process event		*/
			/* information.				*/
	EC_GETCFGINFO,	/* Get the events VFS configuration	*/
			/* information.				*/
	EC_GETMEMINFO	/* Get the events VFS memory usage	*/
			/* information.				*/
} evcntlcmds_t;

/*		Data for the evqcntl Commands
**		=============================
**
**	The following are the commands for the evqcntl
**	function.
*/

typedef enum evqcntlcmds {
	EC_GETCM,	/* Get the current close mode of the	*/
			/* queue.				*/
	EC_SETCM,	/* Set close mode for the queue.	*/
	EC_GETQINFO,	/* Get the current values of the queue	*/
			/* parameters.				*/
	EC_SETQINFO	/* Set the settable queue parameters.	*/
} evqcntlcmds_t;

/*
**	The following are the different close modes which can be
**	set for a queue using the EC_SETCM command of the evqcntl
**	function.
*/

typedef enum evcm {
	ECM_DELALL,	/* Delete all events on the queue when	*/
			/* the last close of the queue occurs.	*/
	ECM_DELNONE,	/* Leave all events on the queue when	*/
			/* the last close of the queue occurs.	*/
	ECM_DELSYS,	/* Delete all events posted by the 	*/
			/* system (ev_type != ET_POST) when the	*/
			/* last close of the queue occurs.	*/
	ECM_DELUSER	/* Delete only those events with type	*/
			/* equal to ET_POST when the last close	*/
			/* of the queue occurs.			*/
} evcm_t;

/*			The evcntl evaltstk_t Structure
**			===============================
**
**	The following is the evaltstk_t structure.  It is used with
**	the evcntl command EC_ALTSTACK.
*/

typedef struct evaltstk {
	caddr_t		etas_stkp;	/* Ptr to the start of	*/
					/* the stack.		*/
	ulong		etas_stks;	/* Size of the stack in	*/
					/* bytes.		*/
	uint		etas_flags;	/* The flags defined	*/
					/* below.		*/
} evaltstk_t;


/*	The flags for the etas_flags field are as follows.
*/

#define	EASF_DISABLE	0x0001	/* The use of an alternate	*/
				/* stack is or should be	*/
				/* disabled.  Can be set by the	*/
				/* user or the kernel.		*/
#define	EASF_ONSTACK	0x0002	/* The process is currently	*/
				/* running on the alternate	*/
				/* stack.  Set only by the 	*/
				/* kernel to return to the user	*/

/*	The following values are just ballpark figures.  The first one
**	should really involve sizeof(pcb_t) and sizeof(evcntxt_t) but
**	we don't want these definitions to have to be known to all
**	programs which include this header file.
*/

#define	ETAS_MINSIZE	 256	/* Minimum size for alternate	*/
				/* stack.			*/
#define	ETAS_SIZE	2048	/* Recommended size for 	*/
				/* alternate stack.		*/

/*			The evcntl evprinfo_t Structure
**			===============================
**
**	The following is the evprinfo_t structure.  It is used with the
**	evcntl commands EC_SETPRINFO and EC_SETPRINFO.
*/

typedef struct evprinfo {
	ushort	evpi_maxtraps;	/* Maximum number of trap	*/
				/* expressions allowed for this	*/
				/* process.  The default is 	*/
				/* evci_maxtraps in the		*/
				/* evcinfo_t structure.		*/
	ushort	evpi_maxeterms;	/* Maximum number of terms	*/
				/* allowed in an expression.	*/
				/* The default value is		*/
				/* evci_maxeterms in the	*/
				/* evcinfo_t structure.		*/
} evprinfo_t;

/*			The evcntl evcinfo_t Structure
**			==============================
**
**	The following is the evcinfo_t structure.  It is used with the
**	evcntl command EC_GETCFGINFO.
*/

typedef struct evcinfo {

	/*	The following are global limits on data allocation for
	**	the entire events VFS.
	*/

	ushort	evci_mevqueues;		/* Max nbr of event	*/
					/* queue structures	*/
					/* (evqueue_t) to 	*/
					/* allocate.		*/
	ushort	evci_mevkevs;		/* Max nbr of kernel	*/
					/* event structures	*/
					/* (evkev_t) to 	*/
					/* allocate.		*/
	ushort	evci_mevexrefs;		/* Max nbr of event	*/
					/* expression reference	*/
					/* structures		*/
					/* (evexref_t) to 	*/
					/* allocate.		*/
	ushort	evci_mevexprs;		/* Max nbr of event	*/
					/* expression 		*/
					/* structures (evexpr_t)*/
					/* to allocate.		*/
	ushort	evci_mevterms;		/* Max nbr of event	*/
					/* term structures	*/
					/* (evterm_t) to	*/
					/* allocate.		*/
	ushort	evci_mevsexprs;		/* Max nbr of event	*/
					/* satisfied expression	*/
					/* structures		*/
					/* (evsexpr_t) to	*/
					/* allocate.		*/
	ushort	evci_mevsterms;		/* Max nbr of event	*/
					/* satisfied term	*/
					/* structures		*/
					/* (evsterm_t) to	*/
					/* allocate.		*/
	ushort	evci_mevtids;		/* Max nbr of trap	*/
					/* identifier structures*/
					/* (evtid_t) to 	*/
					/* allocate.		*/
	ushort	evci_mevretrys;		/* Max nbr of retry	*/
					/* structures		*/
					/* (evretry_t) to	*/
					/* allocate.		*/
	ushort	evci_mevexits;		/* Max nbr of event	*/
					/* exit structures	*/
					/* (evexit_t) to	*/
					/* allocate.		*/

/*			The evcntl evcinfo_t Structure (Continued)
**			==========================================
*/

	ushort	evci_mevsigs;		/* Max nbr of event	*/
					/* signal structures	*/
					/* (evsig_t) to		*/
					/* allocate.		*/
	ushort	evci_mevstrds;		/* Max nbr of stream	*/
					/* data structures	*/
					/* (evd_stream_t) to	*/
					/* allocate.		*/
	ushort	evci_mevdirents;	/* Max nbr of directory	*/
					/* entries (dirent_t)	*/
					/* to allocate.		*/
	ulong	evci_mevdata;		/* Max nbr of bytes to	*/
					/* allocate for holding	*/
					/* events type dependent*/
					/* data and other	*/
					/* miscellaneous uses.	*/
	ushort	evci_tidhts;		/* Number of entries in	*/
					/* the trap id hash	*/
					/* table.  Must be a	*/
					/* power of 2.		*/
	ushort	evci_fnhts;		/* Number of entries in	*/
					/* the file name hash	*/
					/* table.  Must be a 	*/
					/* power of 2.		*/

/*			The evcntl evcinfo_t Structure (Continued)
**			==========================================
*/

	/*	The following are default per-queue limits which can
	**	be modified with evqcntl.
	*/

	ushort	evci_maxev;		/* Default max nbr of	*/
					/* events allowed on a	*/
					/* queue.		*/
	ulong	evci_maxdpe;		/* Default max nbr of	*/
					/* bytes of data allowed*/
					/* per event.		*/
	ulong	evci_maxmem;		/* Default max total	*/
					/* nbr of bytes of data	*/
					/* allowed in memory	*/
					/* for all events on an	*/
					/* event queue.		*/

	/*	The following are default per-process limits which can
	**	be modified with evcntl.
	*/

	ushort	evci_maxtraps;		/* Default max nbr of	*/
					/* trap expressions	*/
					/* allowed for a	*/
					/* process.		*/
	ushort	evci_maxeterms;		/* Default max nbr of	*/
					/* terms allowed in an	*/
					/* expression.		*/
} evcinfo_t;

/*			The evqcntl evinfo_t Structure
**			==============================
**
**	The following is the events information structure.  It is used
**	in conjunction with the evqinfo_t structure below to get
**	information about individual events on the queue being checked
**	with the evqcntl command EC_GETQINFO.
*/

typedef struct evinfo {
	long		evi_flags;	/* Same as ev_flags.	*/
	long		evi_eid;	/* Same as ev_eid.	*/
	long		evi_pri;	/* Same as ev_pri.	*/
	hostid_t	evi_hostid;	/* Same as ev_hostid.	*/
	int		evi_pfd;	/* Same as ev_pfd.	*/
	evtypes_t	evi_type;	/* Same as ev_type.	*/
	pid_t		evi_pid;	/* Same as ev_pid.	*/
	uid_t		evi_uid;	/* Same as ev_uid.	*/
	hrtime_t	evi_time;	/* Same as ev_time.	*/
	size_t		evi_datasize;	/* Same as ev_datasize.	*/
} evinfo_t;

/*			The evqcntl evqinfo_t Structure
**			===============================
**
**	The following is the evqinfo_t structure.  It is used with the
**	evqcntl commands EC_SETQINFO and EC_GETQINFO.  Only the
**	following fields are set from the user's structure for
**	EC_SETQINFO.
**
**		evqi_closemd
**		evqi_maxev	evqi_maxdpe	evqi_maxmem
**
**	The fields on the last line cannot be increased above their
**	system default values (in evinfo) except by the super-user.
*/

/*			The evqcntl evqinfo_t Structure (Continued)
**			===========================================
*/

typedef struct evqinfo {
	ulong		evqi_memsize;	/* Total nbr of bytes of*/
					/* data in private	*/
					/* memory used by all	*/
					/* the events on the	*/
					/* queue.  Data in	*/
					/* shared memory or the	*/
					/* ev_data member of an	*/
					/* event will not be	*/
					/* counted in this	*/
					/* value.		*/
	ulong		evqi_shmsize;	/* Total nbr of bytes	*/
					/* of data in shared	*/
					/* memory for all	*/
					/* events on the queue.	*/
	evinfo_t	*evqi_evinfop;	/* Ptr to an array of	*/
					/* evevinfo_t		*/
					/* structures to return	*/
					/* data about the	*/
					/* events on the queue.	*/
	ushort		evqi_evinfos;	/* Nbr of elements in	*/
					/* the evqi_evinfop	*/
					/* array.		*/
	ushort		evqi_nevents;	/* Nbr of events on the */
					/* queue.		*/
	evcm_t		evqi_closemd;	/* The close mode.  The	*/
					/* values are ECM_XXXX	*/
					/* as defined above.	*/
	ushort		evqi_opencnt;	/* Number of opens (not */
					/* dup's) for the queue.*/
	ulong		evqi_maxev;	/* Max nbr of events	*/
					/* which can be on the	*/
					/* queue.  Initialized	*/
					/* from evci_maxev.	*/
	ulong		evqi_maxdpe;	/* Max data per event	*/
					/* for this queue.	*/
					/* Initialized from	*/
					/* evci_maxdpe.		*/
	ulong		evqi_maxmem;	/* Max total memory for	*/
					/* all events on the	*/
					/* queue.  Initialized	*/
					/* from evci_maxmem.	*/
} evqinfo_t;

/*			Data for the evexit Function
**			============================
**
**	The following structure describes the event dependent data
**	returned for an ET_EXIT event.
*/

typedef struct evd_exit {
	hostid_t	ee_hostid;	/* The host on which	*/
					/* the process exited.	*/
	pid_t		ee_pid;		/* The process id of	*/
					/* the exiting	process.*/
	int		ee_status;	/* The exit status of	*/
					/* the process.		*/
}evd_exit_t;





/*			Data for the evsig Function
**			===========================
**
**	The following are the flags which can be returned in the
**	ev_data field of an ET_SIG event.  This event will always have
**	the EF_QUICKD flag set and the ev_datasize equal to
**	sizeof(char *).
*/

#define	ESF_USER	0x01	/* Signal generated from user	*/
				/* level (system call) such as	*/
				/* kill or sigsend.  Otherwise,	*/
				/* was generated by the kernel.	*/
#define	ESF_INTR	0x02	/* Signal was generated from	*/
				/* interrupt level.		*/


/*	The following structure is used with the evsig function
**	to get back the previous signal event disposition.
*/

typedef struct evsiginfo {
	sigset_t	evsi_sigset;	/* The set of signals	*/
					/* whose disposition is	*/
					/* described in this	*/
					/* structure.		*/
	int		evsi_eqd;	/* The event queue	*/
					/* descriptor to which	*/
					/* a signal event is to	*/
					/* be posted or the	*/
					/* value EQ_NOQUEUE.	*/
	long		evsi_pri;	/* The priority of the	*/
					/* event to be posted.	*/
} evsiginfo_t;


/*	When an ET_STREAM event is posted, the ev_data field will point
**	to a structure of the following format.
*/

typedef struct evd_stream {
	int	es_mask;	/* The condition which has	*/
				/* occurred.  Exactly one of 	*/
				/* the S_XXXX flags will be on	*/
				/* in this word.		*/
	long	es_data;	/* If the bit on in es_mask is	*/
				/* one of those for which a	*/
				/* priority band applies, then	*/
				/* this is the band for which a	*/
				/* message has arrived.		*/
				/* If es_mask is S_ERROR, then	*/
				/* this is the error code.	*/
				/* Otherwise, this field is not	*/
				/* used.			*/
} evd_stream_t;

/*			Function Prototypes
**			===================
**
**	The following are prototypes for the library functions which
**	users call (indirectly using the macros at the start of this
**	file.)
*/

#if !defined(_KERNEL)
#if defined(__STDC__)

int	__evpost(const evver_t, event_t *const, const int, const int);
int	__evpoll(const evver_t, const evpollcmds_t, event_t *const,
		 const int, const hrtime_t *const);
int	__evpollmore(const evver_t, event_t *const, const int);
int	__evtrap(const evver_t, const evpollcmds_t, event_t *const,
		 const int, const long,
		 void (*const)(event_t *, int, long, evcontext_t *),
		 const evta_t *const);
int	__evtrapcancel(const evver_t, long *const, const int);
int	__evcntl(const evver_t, const evcntlcmds_t, const long,
		 const long);
int	__evqcntl(const evver_t, const eqd, const evqcntlcmds_t,
		 const long);
int	__evexit(const evver_t, const idtype_t, const id_t,
		 const ecb_t *const);
int	__evexitset(const evver_t, const procset_t *const,
		    const hostid_t hostid, const ecb_t *const);
int	__evsig(const evver_t, const sigset_t *const,
		const ecb_t *const, evsiginfo_t *const, const int);

#else

int	__evpost();
int	__evpoll();
int	__evpollmore();
int	__evtrap();
int	__evtrapcancel();
int	__evcntl();
int	__evqcntl();
int	__evexit();
int	__evexitset();
int	__evsig();

#endif	/* defined(__STDC__)	*/
#endif	/* !defined(_KERNEL)	*/

#endif	/* _SYS_EVENTS_H */
