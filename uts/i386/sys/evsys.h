/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_EVSYS_H
#define _SYS_EVSYS_H

#ident	"@(#)head.sys:sys/evsys.h	1.24.6.1"
/*			File Contents
**			=============
**
**	This file contains information used in the implementation
**	of the events VFS.  It is intended for inclusion only in
**	the kernel.  User programs and library routines should not
**	need this file.
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
**		REQUIRES	sys/vnode.h
**		REQUIRES	sys.cred.h
*/

/*			Miscellaneous Defines
**			=====================
**
**	The following defines are just convenient to have around.
*/

#ifdef _KERNEL

#define	TRUE	1
#define	FALSE	0
#define	NULL	0
#define	ERROR	(-1)
#define	ZERO	0

#endif	/* _KERNEL	*/

/*	The following define is used to represent an unused structure.
**	The value must not be a legal file descriptor and must also
**	not be equal to EQ_NOQUEUE (defined in events.h).  For an
**	example of where this is used, see ev_main.c/ev_sig_initinfo.
*/

#define	EQ_UNUSED	(-1)

/*	The following are used as arguments to functions ev_mem_alloc,
**	ev_kev_post, etc. to indicate whether to wait or not.  Do not
**	change the values of these symbols.
*/

typedef enum evwait {
	EV_NOWAIT,	/* Do not wait.				*/
	EV_WAIT		/* Wait if necessary.			*/
} evwait_t;

/*			Events VFS Parameter Defines
**			============================
**
**	The following defines set parameters of the events file
**	system type which are not intended to be changable by an
**	administrator.
*/

#define	EV_NMSZ		18		/* Maximum length of an	*/
					/* events queue name.	*/
#define	EV_SLPPRI_INT	(PZERO + 1)	/* Our sleep priority	*/
					/* when we want to be	*/
					/* interruptible.	*/
#define	EV_SLPPRI_NINT	PZERO		/* Our sleep priority	*/
					/* when we want to be	*/
					/* non-interruptible.	*/
#define	EV_ROOTINBR	2		/* I-number of our root	*/
					/* inode.		*/

#define	EV_BASEINBR	(EV_ROOTINBR + 1)
					/* Inode number of our	*/
					/* first queue entry.	*/
#define	EV_NSIGS	(NSIG - 1)	/* Actual nbr of signals*/
					/* which are defined.	*/
					/* NSIG is defined in	*/
					/* sys/signal.h.	*/
#define	EV_BSIZE	1024		/* The block size for	*/
					/* our file system.	*/
					/* Must be a power of 2.*/
#define	EV_L2BSIZE	10		/* The base 2 log of	*/
					/* EV_BSIZE.		*/
#define	EV_FSNAME	"events"	/* The name of our VFS.	*/
#define	EV_PKNAME	"memory"	/* Our "pack name" for	*/
					/* statvfs to return.	*/

/*			General Typedefs
**			================
**
**	The following are some generally useful typedefs.
*/

typedef int		err_t;	/* Type of an error code.	*/

/*	With ansi C, the best way to represent a general pointer is as
**	a (void *).  However, (void *) will not be available in all
**	old (pre-ansi) compilers.  Therefore, we define a general
**	pointer type as follows.
*/

#if defined(__STDC__)
	typedef void *	gptr_t;
#else
	typedef char *	gptr_t;
#endif	/* __STDC__	*/

/*			Miscellaneous Macros
**			====================
**
**	The following macro does an spl to the proper level for the
**	events VFS.
*/

#define	spl_ev()	splhi()


/*	The following macros translate between inode numbers and
**	the corresponding queue entry.  Both macros assume that the
**	queue referred to is an actual event queue and not the queue
**	which represents our root directory.
*/

#define	ev_inbrtoqp(INBR)  (ev_actqp[(INBR) - EV_BASEINBR].eaq_qp)
#define	ev_qptoinbr(QP)	   (((QP)->evq_aqp - ev_actqp) + EV_BASEINBR)


/*	The following macro converts a directory offset to an inode
**	number.  It assumes that the directory entry is not for "." or
**	".." which are the first two (pseudo) entries.
*/

#define	ev_dotoinbr(DO)	(((DO) / EV_DIRSZ) - 2 + EV_BASEINBR)


/*	The following macro defines the size of a message queue.
**	It counts each event as using sizeof(event_t) bytes plus
**	whatever space is used by the type dependent data.  Data in
**	the ev_data field (EF_QUICKD set) or in shared memory (EF_SHM
**	set) is not counted.
*/

#define	ev_qsize(QP)	((QP)->evq_nevents * sizeof(event_t) + \
			 (QP)->evq_memsize)

/*	The following macro converts a size in bytes to a size in
**	blocks where the block size is that of the events VFS.  The
**	conversion is done by rounding up.
*/

#define	ev_bytestoblks(BY)  (((BY) + EV_BSIZE - 1) >> EV_L2BSIZE)

/*			Miscellaneous Macros (Continued)
**			================================
**
**	The following macro rounds the value "VAL" up to the next "BND"
**	boundary where "BND" must be a power of 2.
*/

#define	round(VAL, BND)	(((VAL) + (BND) - 1) & ~((BND) - 1))

/*	Get the absolute value of a number.
*/

#define	abs(VAL)	((VAL) < 0  ?  -(VAL)  :  (VAL))

/*	Get the minimum of two values.
*/

#define	min(A, B)	((A) <= (B) ? (A) : (B))

/*
**	The following macro is used to reduce the number of
**	unnecessary calls to ev_istrap().
*/
#define	EV_ISTRAP(PRP)	((PRP)->p_evpdp != NULL && ev_istrap(PRP))


/*			Events Directory Structure (evdir_t)
**			====================================
**
**	The following is the structure of a directory in the events
**	file system.  This directory is only conceptual.  It does
**	not actually exist anywhere but we manufacture directory
**	entries as needed to make "ls" and stuff like that work.
**
**	The closest thing we have to a directory is the "active queue
**	list" which is an array of pointers to active queues.  The
**	array itself is pointed to by ev_actqp.  The index into the
**	array is used to construct an inumber for the queue.  The list
**	is traversed sequentially to read the directory for readdir.
*/

typedef struct evdir {
	ino_t	ev_d_ino;		/* The inode number.	*/
	char	ev_d_nm[EV_NMSZ + 1];	/* The name.		*/
} evdir_t;

#define	EV_DIRSZ	sizeof(evdir_t)

/*			Data Structure Overview
**			=======================
**
**	We use a number of different data structures to maintain
**	information about events and their connection to queues and
**	processes.  The following pictures show the relationships
**	among the various structures.  An arrow pointing in one
**	direction indicates a singly linked list or a pointer.  An
**	arrow pointing in both directions indicates a doubly-linked
**	list or a ring.  Following the pictures are the declarations
**	for all of these structures.
**
**	An evqueue_t structure represents an event queue.  The evkev_t
**	structures represent events on this queue.  They are in fifo
**	order.  Each queue has a ring of evexref_t structures attached
**	to it.  This gives the set of expressions which have one or
**	more terms referencing the queue.  When an event is posted to
**	a queue, following the evexref_t ring shows what poll and trap
**	expressions must be checked to see if the newly posted event
**	caused them to be satisfied.
**
**	Each evexpr_t structure represents an evpoll or evtrap
**	expression.  By an expression, we mean a list of terms (events)
**	connected by the disjunction (EC_ONE or EC_ANY) or conjunction
**	(EC_ALL) operator.  All of the outstanding trap expressions for
**	a process are on a ring headed by the epd_exprs field of the
**	evpd_t table.  The poll expressions are not on this list as
**	their existence is more transitory and they are never held.
**	They are found via the evexref_t list.  The following picture
**	shows the relation of the evqueue_t, evkev_t, and evexref_t
**	structures.
**
**	The events on a queue can be requested in either fifo or 
**	priority order.  Currently, we must search the list of events
**	on the queue linearly for the highest priority event when
**	requested.  This can be very inefficient if there are many
**	events on the queue.  It would be possible to improve this by
**	keeping the events in priority order, perhaps by using a
**	priority queue (heap) and then running a fifo list through them
**	to satisfy fifo requests.  This is only important if queues
**	accumulate long lists of events and then are polled or trapped
**	in priority order.  Implementing such a priority queue is a
**	possible future enhancement.

**			Data Structure Overview (Continued)
**			===================================
**
**
**      _____________	      ____________	      ____________
**      |	    |         |	         |	      |		 |
**      | evqueue_t |/_______\|  evkev_t |/__________\|	 evkev_t |
**      |___________|\       /|__________|\          /|__________|
**           /|\
**            |
**            |
**            |
**            |     ________________	      ________________
**	      |     |              |  	      |              |
**	      |____\|  evexref_t   |/________\|  evexref_t   |
**		   /|______________|\        /|______________|
**			   |			      |
**			   |			      |
**			   |    		      |
**   __________     ______\|/_______	      _______\|/______
**   |        |     |              |  	      |              |
**   | proc_t |/___\|   evexpr_t   |/________\|    evexpr_t  |
**   |________|\   /|______________|\        /|______________|
**			  /|\   		     /|\
**			   |			      |
**			   |			      |
**			   |			      |
**                  ______\|/_______          _______\|/______
**		    |	           |	      |              |
**		    |   evterm_t   |          |    evterm_t  |
**		    |______________|          |______________|
**			  /|\   		     /|\
**			   |			      |
**			   |			      |
**			   |			      |
**                  ______\|/_______          _______\|/______
**		    |	           |	      |              |
**		    |   evterm_t   |          |    evterm_t  |
**		    |______________|          |______________|

**			Data Structure Overview (Continued)
**			===================================
**
**
**	A satisfied poll or trap expression is represented by an 
**	evsexpr_t structure.  These structures are stacked for nested 
**	trap handler calls.  The top of the stack is pointed to by the
**	epd_asexp member of the evpd_t structure.  Also on this stack 
**	are satisfied poll expressions which have been retained after 
**	the evpoll returned because there is data remaining and the 
**	EF_DISCARD flag was not set or a file descriptor was passed with
**	the event and not given to the caller yet.  Satisfied
**	expressions of either type which have not yet been handled are
**	linked together on a doubly linked ring headed by the
**	epd_ppsexprs or epd_ptsexprs member of the evpd_t structure.
**	The first list is for pending poll satisfied expressions and
**	the second is for pending trap satisfied expressions.  The
**	evsexpr_t structure points to the evexpr_t structure for the
**	expression which is satisfied.  The evsexpr_t structure also
**	contains a ring of evsterm_t structures, one for each term of
**	the satisfied expression.
**
**	Each evsterm_t structure, in turn, points to the corresponding
**	evterm_t structure.  Note that the number of satisfied terms may
**	be less than the total number of terms so the two lists are not
**	necessarily of the same size.  Only satisfied terms are on the
**	satisfied expression's satisfied term ring while all terms of
**	the user's expression are on the term ring.
**	The following picture shows the use of the evsexpr_t and
**	evsterm_t structures.

**			Data Structure Overview (Continued)
**			===================================
**
**  _________________     _______________                       _______________
**  |               |     |             |                       |             |
**  |    proc_t     |  __\|  evsexpr_t  |  ____________________\|  evsexpr_t  |
**  |               |  | /|             |  |     _____________ /|             |
**  |   epd_asexp   |__|  |  evse_next  |__|  __\|           |  |  evse_next  |
**  |               |     |             |     | /|  evexpr_t |  |             |
**  |   epd_ptsexpr |     |   evse_ep   |_____|  |           |  |   evse_ep   |
**  |_______________|     |             |        |           |  |             |
**          /|\           |   evse_stp  |        |   eve_tp  |  |   evse_stp  |
**           |            |_____________|        |___________|  |_____________|
**    ______\|/______           /|\                   /|\     
**    |             |            |                     |      
**    |  evsexpr_t  |     ______\|/______        _____\|/_____
**    |             |     |             |  _____\|           |
**    |             |     |   evsterm_t |  |    /|  evterm_t |
**    |  evse_next  |     |             |  |     |           |
**    |_____________|     |   evst_tp   |__|     |  evt_next |
**          /|\           |             |        |___________|
**           |            |  evst_next  |             /|\     
**    ______\|/______     |_____________|              |      
**    |             |           /|\              _____\|/_____
**    |  evsexpr_t  |            |               |           |
**    |             |     ______\|/______        |  evterm_t |
**    |             |     |             |        |           |
**    |  evse_next  |     |   evsterm_t |        |  evt_next |
**    |_____________|     |             |        |___________|
**                        |   evst_tp   |___          /|\
**                        |             |  |           |
**                        |  evst_next  |  |     _____\|/_____
**                        |_____________|  |     |           |
**                                         |____\|  evterm_t |
**                                              /|           |
**                                               |  evt_next |
**                                               |___________|

**			Data Structure Overview (Continued)
**			===================================
**
**                                                    
**
**	The evtid_t structures are used to keep track of trap
**	identifiers supplied as the "tid" argument of the evtrap
**	function call.  There is a hash table used for looking up trap
**	identifiers.  It is pointed to by ev_tidhtp and has evci_tidhts
**	entries where evci_tidhts is in the evcinfo_t structure 
**	described in events.h.  The hash table enables us to find the
**	evtid_t structure for a particular tid quickly.  We must do
**	this every time we process an evtrap system call.  In
**	addition, every trap expression, represented by an evexpr_t
**	structure, contains a pointer, eve_tidlp, to a list of pointers
**	to evtid_t structures.  These are the trap ids which should be
**	held when this trap is taken.  The evtid_t structure contains
**	the level (etid_lvl) at which it is held or 0 if it is not
**	currently held.  This can be compared with the trap level of
**	the top entry on the epd_asexp active expression stack
**	(evse_lvl).  The following picture shows the relation between
**	the evtid_t structures, the hash table, and the evexpr_t
**	structure and its reference list.
**
**
**  
** 
**    ________________
**    |              |
**    |   ev_tidhtp  |__
**    |______________|  |
**                      |
**                      |
**     _________________|
**     |  __________     ___________     ___________
**     |_\|        |     |         |     |         |
**       /| tid    |/___\| evtid_t |/___\| evtid_t |
**        | hash   |\   /|_________|\   /|_________|
**        | table  |  .
**        |        |  .  ___________     ___________     ___________
**        |        |/___\|         |     |         |     |         |
**        |________|\   /| evtid_t |/___\| evtid_t |/___\| evtid_t |
**                       |_________|\   /|_________|\   /|_________|
**                                           /|\            /|\
**                                            |              | 
**    _______________     ___________         |              |
**    |             |  __\|         |         |              |
**    |   evexpr_t  |  | /|  expr   |_________|              |
**    |             |  |  |  tid    |                        |
**    |  eve_tidlp  |__|  |  hold   |                        |
**    |_____________|     |  list   |________________________|
**                        |_________| 
*/

/*			The evlisthd_t Structure
**			========================
**
**	We use many doubly linked lists of structures in the events 
**	VFS.  Most of these are maintained as a ring instead of a list
**	with head and tail pointers.  The reason is that enqueueing and
**	dequeueing are faster since no special test for an empty list
**	is required.  The following structure is used to represent the
**	head of a general doubly linked list or ring.  When used as a
**	ring, it is empty when:
**
**		lh_first == lh_last == &lh_first
**
**	In order for this trick to work, the link fields for the ring 
**	structure must be the first members of the structure.  We do
**	this for various structures below.  In each case, it is
**	indicated which members must not be moved.
*/

typedef struct evlisthd {
	struct evlisthd	*lh_first;	/* First structure on	*/
					/* list.		*/
	struct evlisthd	*lh_last;	/* Last structure on	*/
					/* list.		*/
} evlisthd_t;

/*			The evterm_t Structure
**			======================
**
**	The following structure represents a term of a trap or poll
**	expression.  All of the terms of an expression are linked
**	together in order through the evt_next field.  The list of all
**	the terms of an expression, in order, form the eve_terms ring in
**	the evexpr_t structure.  The first two members of this structure
**	must not be moved.
*/

typedef struct evterm {
	struct evterm	*evt_next;	/* Next term in the	*/
					/* expression.		*/
	struct evterm	*evt_prev;	/* Previous term in the	*/
					/* expression.		*/
	event_t		evt_ev;		/* The event which the	*/
					/* user specified.	*/
	vnode_t		*evt_vp;	/* Ptr to the vnode for */
					/* the event queue	*/
					/* evt_ev.ev_eqd.	*/
	ushort		evt_seq;	/* Sequence nbr of this */
					/* term in the list of	*/
					/* terms for the	*/
					/* expression - zero	*/
					/* origin.		*/
} evterm_t;

/*			The evexpr_t Structure
**			======================
**
**	The following structure represents a trap or poll expression.
**	A doubly linked ring of these structures which represent all
**	outstanding trap or poll expressions for a process is pointed
**	to by the epd_exprs field of a evpd_t structure entry.  Note
**	that the first two members of this structure must not be moved.
*/

typedef struct evexpr {
	struct evexpr	*eve_next;	/* Next expression for	*/
					/* this process.	*/
	struct evexpr	*eve_prev;	/* Previous  expression */
					/* for this process.	*/
	evlisthd_t	eve_terms;	/* The ring of terms in */
					/* the expression.	*/
	event_t		*eve_uevp;	/* Ptr to user's events */
					/* array.  This is 	*/
					/* where evpoll and	*/
					/* evtrap return 	*/
					/* results.		*/
	struct proc	*eve_procp;	/* Ptr to process this	*/
					/* expression is for.	*/
	void		(*eve_lfunc)();	/* Ptr to our library	*/
					/* routine used to 	*/
					/* transfer to a user's	*/
					/* trap handler.	*/
	void		(*eve_ufunc)();	/* Ptr to the user's	*/
					/* trap handler 	*/
					/* function.		*/
	struct evtid	*eve_tidp;	/* Ptr to trap id 	*/
					/* struct for this trap	*/
					/* expression.		*/
	pcparms_t	eve_schedpri;	/* Scheduling priority	*/
					/* to be set when	*/
					/* invoking a trap	*/
					/* handler if the 	*/
					/* eve_trappri flag 	*/
					/* is set.		*/
	struct evtid	**eve_tidlp;	/* Ptr to an array of	*/
					/* pointers to evtid_t	*/
					/* structures.  The 	*/
					/* list of tids to be	*/
					/* held when trapping	*/
					/* on this expression.	*/
	ushort		eve_tidls;	/* Nbr of elements in	*/
					/* the eve_tidlp array.	*/
	evpollcmds_t	eve_cmd;	/* Expression command.  */
					/* One of EC_ONE, 	*/
					/* EC_ANY, or EC_ALL.	*/

/*			The evexpr_t Structure (Continued)
**			==================================
*/

	ushort		eve_poll    :1;	/* Set for an evpoll	*/
					/* expression.  If not	*/
					/* set, then an evtrap	*/
					/* expression.		*/
	ushort		eve_trappri :1;	/* Set if a special	*/
					/* priority was given	*/
					/* with an evtrap.  	*/
					/* Never set when 	*/
					/* eve_poll is set.	*/
	ushort		eve_taken   :1;	/* Set if this 		*/
					/* expression has been	*/
					/* satisfied but user's	*/
					/* handler has not been	*/
					/* called yet.  Don't	*/
					/* try to satisfy it	*/
					/* again.  Never set if	*/
					/* eve_poll is.		*/
	ushort		eve_holdall :1;	/* Set if all future	*/
					/* traps should be held	*/
					/* when calling the	*/
					/* handler for this 	*/
					/* expression.		*/
	ushort		eve_canned  :1;	/* Set if this trap	*/
					/* expression has been	*/
					/* cancelled.		*/
	ushort		eve_restart :1;	/* Set if restartable	*/
					/* system calls should	*/
					/* be restarted after	*/
					/* trapping on this	*/
					/* expression.  Never	*/
					/* set if eve_poll is.	*/
	ushort		eve_astk : 1;	/* Set if we should	*/
					/* switch to alternate	*/
					/* stack before calling	*/
					/* handler for this	*/
					/* expression.		*/
	ushort		eve_nterms;	/* Number of terms in	*/
					/* the event expression.*/
	ushort		eve_refcnt;	/* Count of the number	*/
					/* of evsexpr_t structs	*/
					/* which are pointing 	*/
					/* to this expression.	*/
} evexpr_t;

/*			The evexref_t Structure
**			=======================
**
**	The following structure is used to connect an event queue
**	with the outstanding trap and poll expressions which reference
**	the queue.  Each event queue points to a ring of evexref_t
**	structures.  Each evexref_t structure points to one trap or
**	poll expression as represented by an evexpr_t structure.  Each
**	expression pointed to references the indicated queue in at
**	least one of its terms.  The first two members of this
**	structure must not be moved.
*/

typedef struct evexref {
	struct evexref	*exr_next;	/* Ptr to next reference*/
					/* on the ring for this	*/
					/* queue.		*/
	struct evexref	*exr_prev;	/* Ptr to previous	*/
					/* reference on the 	*/
					/* ring for this queue.	*/
	evexpr_t	*exr_ep;	/* Ptr to the		*/
					/* expression.		*/
} evexref_t;

/*			The evsterm_t Structure
**			=======================
**
**	The following structure is used to represent a satisfied term
**	of a satisfied expression.  All the satisfied terms of the
**	satisfied expression are linked together through the evst_next
**	field.  The start of the ring is the evse_sterms field of the
**	evsexpr_t structure.  Note that this list will contain
**	evsterm_t structures only for satisfied terms of the 
**	expression.  This means that for EC_ONE and
**	EC_ANY, there may be fewer terms in the evsterm_t set than in
**	the evterm_t set for the expression.  The evst_tp field of the
**	evsterm_t structure points to the evterm_t structure which is
**	satisfied.  The evt_seq field of the evterm_t structure
**	indicates which event in the user's event list the term
**	corresponds to.  It is used to find the address of the user's
**	event_t structure to which this satisfied term should be copied.
**	The first two members of this structure must not be moved.
*/

typedef struct evsterm {
	struct evsterm	*evst_next;	/* Next satisfied term.	*/
	struct evsterm	*evst_prev;	/* Previous satisfied	*/
					/* term.		*/
	evterm_t	*evst_tp;	/* Ptr to the term	*/
					/* which has been	*/
					/* satisfied.		*/
	struct evkev	*evst_kevp;	/* Ptr to the event	*/
					/* which satisfies the	*/
					/* term.		*/
} evsterm_t;

/*			The evsexpr_t Structure
**			=======================
**
**	This structure represents a satisfied poll or trap expression.
**	It will be saved for poll only if the satisfied expression must
**	be retained after the evpoll returns.  This occurs if not all
**	of the data was returned and EF_DISCARD was not set for at
**	least one of the terms in the expression or if a file descriptor
**	was passed with the event and has not yet been given to the
**	receiver of the event.  For trap, there will be one evsexpr_t
**	structure for each active trap handler call.  Note that these
**	can be nested and even recursive since the handler can unblock
**	the automatic block which we impose when we call it.  The
**	evsexpr_t structures are stacked for nested trap handler calls.
**	The top (current) entry is pointed to by the epd_asexpr field of
**	the evpd_t structure.  Satisfied *xpression structures for trap
**	handlers which have not yet been called are doubly linked on a
**	ring whose header is epd_ptsexprs.  Satisfied expressions for
**	polls which have not yet been received by the process doing the
**	poll are on a similar ring whose head is epd_ppsexprs.  The
**	first two members of this structure must not be moved.
*/

/*			The evsexpr_t Structure (Continued)
**			===================================
*/

typedef struct evsexpr {
	struct evsexpr	*evse_next;	/* Ptr to next		*/
					/* satisfied expression	*/
					/* on the stack or on a	*/
					/* list.		*/
	struct evsexpr	*evse_prev;	/* Ptr to the previous	*/
					/* satisfied expression	*/
					/* on a list.		*/
	evexpr_t	*evse_ep;	/* Ptr to the		*/
					/* expression which has	*/
					/* been satisfied.	*/
	evlisthd_t	evse_sterms;	/* The ring of		*/
					/* satisfied terms for	*/
					/* this expression.	*/
	short		evse_lvl;	/* The nesting level at	*/
					/* which this trap	*/
					/* handler is running.	*/
	ushort		evse_priset :1;	/* Set if the process'	*/
					/* priority has been 	*/
					/* set from this  expr.	*/
					/* The priority to 	*/
					/* restore is in the	*/
					/* evse_oldpri member.	*/
	ushort		evse_astk : 1;	/* Set if we switched	*/
					/* to the alternate	*/
					/* stack when calling 	*/
					/* the handler with	*/
					/* this expression.	*/
					/* Not set if already	*/
					/* running on alternate	*/
					/* stack before calling	*/
					/* handler with this	*/
					/* satisfied expression.*/
	pcparms_t	evse_oldpri;	/* The scheduling	*/
					/* priority which was 	*/
					/* in effect before	*/
					/* calling the trap	*/
					/* handler.  It must be	*/
					/* restored when the	*/
					/* handler returns.	*/
} evsexpr_t;

/*			The evtid_t Structure
**			=====================
**
**	The following structure is used to maintain information about
**	trap identifiers.  Trap holds exist at a particular level.
**	Level zero is the main program.  Level one is a trap handler
**	called while running in the main program.  Level 2 is a trap
**	handler called while running in a level 1 trap handler.  In
**	general, level N (for N > 1) is a trap handler called while
**	running in a level N-1 handler.
**
**	Every evtid_t structure is on two rings.  The first is headed
**	by the evpd_t structure field epd_tids.  This list contains
**	every evtid_t structure for the process.  The link fields for
**	this ring are etid_pnextp and etid_pprevp.  The evtid_t
**	structure is also on a hash ring.  The link fields for this
**	ring are etid_hnextp and etid_hprevp.
**
**	The global variable ev_tidhtp points to the hash table for these
**	trap identifier structures hashed on the trap id and process
**	identifier.  This table contains evci_tidhts entries.  Each
**	entry in the hash table is an evlisthd_t structure heading the
**	ring of evtid_t structures.  There is one such structure for
**	each trap identifier being used by a process.  If two processes
**	are using the same tid, there will be two separate evtid_t
**	entries.  Each evtid_t entry indicates the level at which it is
**	being held or TR_NOTHELD if it is not currently held.  In
**	addition to traps being held automatically when a trap handler
**	is called, the user can explicitly hold traps using the evcntl
**	system call.  A trap held in this way will have the etid_holdlvl
**	field set in the same way as for an automatic hold.
**
**	This structure also contains a count of the number of pointers
**	which exist to the structure.  When the last use is released
**	for that tid and process, the evtid_t structure is deleted.
**	The hash is performed on the tid and proc id values.
**
**	The first 4 fields of this structure must not be moved.  Note
**	the special code in ev_subrs.c/ev_tid_hash and
**	ev_subrs.c/ev_tid_init which make assumptions about the 
**	locations of these fields.
*/

/*			The evtid_t Structure (Continued)
**			=================================
*/

typedef struct evtid {
	struct evtid	*etid_pnextp;	/* Ptr to the next	*/
					/* evtid_t structure on */
					/* the proc ring.	*/
	struct evtid	*etid_pprevp;	/* Ptr to the previous	*/
					/* evtid_t structure on */
					/* the proc ring.	*/
	struct evtid	*etid_hnextp;	/* Ptr to the next	*/
					/* evtid_t structure on	*/
					/* the hash ring.	*/
	struct evtid	*etid_hprevp;	/* Ptr to the previous	*/
					/* evtid_t structure on	*/
					/* the hash ring.	*/
	long		etid_tid;	/* The trap identifier	*/
					/* being held.		*/
	pid_t		etid_pid;	/* Process id that this	*/
					/* entry belongs to.	*/
	short		etid_holdlvl;	/* The level at which	*/
					/* this tid is being 	*/
					/* held or TR_NOTHELD	*/
					/* if not currently	*/
					/* held.		*/
	ushort		etid_use;	/* Nbr of pointers to	*/
					/* this structure.	*/
} evtid_t;

#define	TR_NOTHELD	-1		/* Value for 		*/
					/* etid_holdlvl when	*/
					/* the trap is not held.*/

/*			The evkev_t Structure
**			=====================
**
**	The following structure is used to represent an event in the
**	kernel.  A ring of these events starts from an event queue.
**	The first two members of this structure must not be moved.
*/

typedef struct evkev {
	struct evkev	*kev_next;	/* Next event on the	*/
					/* same queue as this	*/
					/* one.			*/
	struct evkev	*kev_prev;	/* Previous event on 	*/
					/* the queue.		*/
	event_t		kev_ev;		/* The event info.	*/
	struct vnode	*kev_vp;	/* Ptr to the vnode for */
					/* the queue this event	*/
					/* is to be posted to 	*/
					/* or taken from.	*/
	struct vnode	*kev_pfdvp;	/* Ptr to the vnode for	*/
					/* the file descriptor	*/
					/* being passed in	*/
					/* ev_pfd if EF_PFD is	*/
					/* set in ev_flags.	*/
	union {
		struct anon_map	*kev_amp;	/* Ptr to anon map	*/
						/* structure. For	*/
						/* EF_SHM only.		*/
		struct evsigr	*kev_srp;	/* Ptr to evsigr_t	*/
						/* structure or NULL.	*/
						/* If not null, clear	*/
						/* the EVS_F_BLOCK flag	*/
						/* in the evsigr_t 	*/
						/* structure when this	*/
						/* event is dequeued.	*/
						/* For ET_SIG events	*/
						/* only.		*/
	}		kev_un;		/* An ET_SIG event will	*/
					/* never be in shm.	*/
	size_t		kev_datasent;	/* When event sent in  	*/
					/* parts, this is 	*/
					/* amount already sent.	*/
					/* The value of		*/
					/* kev_ev.ev_datasize	*/
					/* can be used to	*/
					/* determine the amount	*/
					/* of data remaining.	*/

/*			The evkev_t Structure (Continued)
**			=================================
*/

	ushort		kev_onqueue :1;	/* Set if this event is	*/
					/* on a queue.		*/
	ushort		kev_taken   :1;	/* Set if this event 	*/
					/* has been taken (at	*/
					/* least temporarily) 	*/
					/* to satisfy a poll or	*/
					/* trap request.	*/
	char		kev_pfdflags;	/* The f_flag field of	*/
					/* the file table entry	*/
					/* for the passed file	*/
					/* descriptor ev_pfd if	*/
					/* EF_PFD is set in	*/
					/* ev_flags.		*/
} evkev_t;

/*			The evqueue_t Structure
**			=======================
**
**	The following structure is used within the kernel to represent
**	an event queue.  In VFS terms, it is the inode which is pointed
**	to by the vnode.  The following are some notes on the usage of
**	certain fields of this structure.
**
**		evq_memsize	This is the total number of bytes of
**				data in private memory for all events on
**				the queue.  It is the sum of the 
**				ev_datasize fields of all events on the
**				queue for which the EF_SHM and
**				EF_QUICKD flags are off and the
**				ev_datasize is greater than zero.  The 
**				total file size will be the number of 
**				events on the queue (evq_nevents) times
**				the size of an event (sizeof(event_t)) 
**				plus the value of the evq_memsize field.
**				This approximates the space taken up by
**				the event.
**
**		evq_atime	The access time is updated when an
**				event is removed from the queue via 
**				either evpoll or evtrap.
**
**		evq_mtime	The modify time is updated when an
**				event is placed on the queue via 
**				evpost, evsig, or any other mechanism
**				such as hrtcntl, asynchronous system
**				call, etc.
**
**		evq_ctime	The change time is updated when the
**				status of an event queue is changed via
**				one of the evqcntl commands.
**
**	The event queues are kept on a hash ring based on the file name
**	of the queue (evq_name).  The hash table is pointed to by the
**	global variable ev_fnhtp.  The links for the ring are evq_next
**	and evq_prev.  The first two members of this structure must not
**	be moved.
*/

/*			The evqueue_t Structure (Continued)
**			===================================
*/

typedef struct evqueue {
	struct evqueue	*evq_next;	/* Ptr to the next	*/
					/* active event queue	*/
					/* on the ev_fnhtp hash	*/
					/* ring.		*/
	struct evqueue	*evq_prev;	/* Ptr to previous	*/
					/* event queue on the	*/
					/* hash ring.		*/
	evlisthd_t	evq_events;	/* The ring of events	*/
					/* on the queue.	*/
	evlisthd_t	evq_exrefs;	/* The ring of		*/
					/* references to exprs	*/
					/* which reference this	*/
					/* queue.		*/
	ulong		evq_memsize;	/* Total bytes of	*/
					/* private data on the	*/
					/* queue.  Events with	*/
					/* data in shared 	*/
					/* memory or in the	*/
					/* ev_data member of an	*/
					/* event will not be	*/
					/* counted in this	*/
					/* member.		*/
	ulong		evq_shmsize;	/* Total bytes of	*/
					/* shared memory data 	*/
					/* used by all events	*/
					/* on this queue.	*/
	ushort		evq_nevents;	/* Total number of	*/
					/* events on this queue.*/
	evcm_t		evq_closemd :3;	/* The close mode of	*/
					/* the queue.  One of	*/
					/* the ECM_XXXX values	*/
					/* defined in events.h.	*/
					/* We have left an 	*/
					/* extra bit in case 	*/
					/* any new values have	*/
					/* to be added to the	*/
					/* enumeration.		*/
	ushort		evq_wspace  :1;	/* Set if a process is	*/
					/* waiting for space to	*/
					/* post an event to	*/
					/* this queue.		*/
	ushort		evq_wevent  :1;	/* Set if a process is	*/
					/* waiting for an	*/
					/* event to be posted	*/
					/* to this queue.	*/
	ushort		evq_locked  :1;	/* Set if this queue is	*/
					/* locked.		*/
	ushort		evq_wanted  :1;	/* Set if someone is	*/
					/* waiting for the lock	*/
					/* evq_locked to clear.	*/

/*			The evqueue_t Structure (Continued)
**			===================================
*/

	uid_t		evq_uid;	/* User id of owner of	*/
					/* queue.		*/
	gid_t		evq_gid;	/* Group id of owner of	*/
					/* queue.		*/
	mode_t		evq_mode;	/* File mode of queue.	*/
	ushort		evq_opencnt;	/* Count of number of	*/
					/* open's on this queue.*/
	struct evactq	*evq_aqp;	/* Ptr to the entry in	*/
					/* the ev_actqp array	*/
					/* for this queue.	*/
	time_t		evq_atime;	/* Access time of queue.*/
	time_t		evq_mtime;	/* Modify time of queue.*/
	time_t		evq_ctime;	/* Change time of queue.*/
	ulong		evq_maxev;	/* Max nbr of events	*/
					/* allowed on the queue.*/
	ulong		evq_maxdpe;	/* Max nbr of bytes of	*/
					/* data per event.	*/
	ulong		evq_maxmem;	/* Max value for	*/
					/* evq_memsize.		*/
	struct vnode	evq_vnode;	/* The vnode for this	*/
					/* queue.		*/

	char		evq_name[EV_NMSZ + 1];
					/* The name of the	*/
					/* queue.		*/
} evqueue_t;

/*	The following is the default close mode for a newly created
**	event queue.
*/

#define	ECM_DEFAULT	ECM_DELALL


/*	The locking of an event queue is based on the fact that
**	interrupt level routines are allowed to post events to a
**	queue but not to remove them from the queue.  For this reason,
**	any process manipulating the evq_events ring must do an spl_ev()
**	to block out interrupts.
**
**	In addition, there are some routines which sleep in a loop
**	which is traversing the events ring.  These routines must lock
**	the queue using ev_rwlock to prevent interactions.
*/

/*			The evretry_t Structure
**			=======================
**
**	The following structure is used only by the ev_evpost function
**	in ev_main.c.  It is used to remember events which couldn't be
**	posted immediately but which should be retried.
*/

typedef struct evretry {
	struct evretry	*ert_next;	/* Next one on		*/
					/* double-linked ring.	*/
	struct evretry	*ert_prev;	/* Previous one on ring.*/
	event_t		*ert_uevp;	/* Ptr to the user's	*/
					/* event structure in	*/
					/* user address space.	*/
	evkev_t		*ert_kevp;	/* Pointer to the	*/
					/* kernel event to be	*/
					/* posted.		*/
	vnode_t		*ert_vp;	/* Ptr to the vnode for	*/
					/* the queue to which	*/
					/* the event should be	*/
					/* posted.		*/
} evretry_t;

/*			The evexitr_t Structure
**			=======================
**
**	The following structure is used to implement the evexit
**	function.  A ring of these structures exists for each process.
**	The head of this list is the epd_exits member of the evpd_t
**	structure.  The first two members of this structure must not
**	be moved.
*/

typedef struct evexitr {
	struct evexitr	*evx_next;	/* Next entry on the	*/
					/* ring.		*/
	struct evexitr	*evx_prev;	/* Previous entry on 	*/
					/* the ring.		*/
	evkev_t		*evx_kevp;	/* Ptr to preallocated	*/
					/* event structure.	*/
	vnode_t		*evx_vp;	/* Ptr to the vnode for	*/
					/* the queue to which	*/
					/* the event should be	*/
					/* posted.		*/
} evexitr_t;

/*	The following structure is used to pass parameters between
**	ev_evexit and the functions ev_exit_add and ev_exit_cancel which
**	ev_evexit calls indirectly via os/subr.c/dotoprocs.
*/

typedef struct evexitprms {
	cred_t	*exp_crdp;	/* Ptr to the credentials for	*/
				/* the process doing the evexit.*/
	vnode_t	*exp_vp;	/* Ptr to the vnode for the	*/
				/* queue to which the exit	*/
				/* event is to be posted.	*/
	ecb_t	*exp_ecbp;	/* Ptr to the event control	*/
				/* block describing the event	*/
				/* to post.			*/
	cnt_t	*exp_rvp;	/* Pointer to where the return	*/
				/* value should be stored.	*/
	err_t	exp_error;	/* An error returned by the 	*/
				/* function we call.		*/
} evexitprms_t;

/*			The evsigr_t Structure
**			======================
**
**	The following structure is used to implement the evsig
**	function.  An array of these structures is pointed to by the
**	epd_sigrp member of the evpd_t structure.  The size of this
**	array is EV_NSIGS.  The index into the array is the signal
**	number minus one since signal numbers start at one.  The only
**	valid elements of this array are those for which the signal
**	number is in the set epd_sigset in the proc table.
*/

typedef struct evsigr {
	long		evs_pri;	/* Priority of event 	*/
					/* to post.		*/
	int		evs_eqd;	/* Event queue		*/
					/* descriptor from 	*/
					/* which we got evs_vp.	*/
	vnode_t		*evs_vp;	/* Queue to which event */
					/* should be posted.	*/
	ushort		evs_noqueue :1;	/* Set if signal	*/
					/* queueing is not to	*/
					/* be done.  Don't post	*/
					/* an event for the	*/
					/* signal if one has	*/
					/* already been posted	*/
					/* and not received by	*/
					/* a process.		*/
	ushort		evs_blocked :1;	/* Set if an event is	*/
					/* not to be posted for	*/
					/* this signal.  Used 	*/
					/* to implement the	*/
					/* evs_noqueue function.*/
} evsigr_t;

/*			The evpd_t Structure
**			====================
**
**	The following structure contains data concerning a process
**	using the events facility.  One of these structures is pointed
**	to by the p_evpdp field of the proc table entry.  The structure
**	is allocated the first time a process uses any of the events
**	system calls.  At each point where a process can enter the 
**	events VFS for the first time, we must check that an evpd_t
**	structure is allocated and, if not, allocate one.  There are
**	not very many such places since most events operations occur on
**	an open file descriptor and will only get to the events VFS if
**	an event queue has been opened.  The places where we must check
**	are currently:
**
**		ev_vnodeops.c/ev_open	Opening an event queue.
**		ev_vnodeops.c/ev_create	Creating an event queue.
**		ev_sysint.c/ev_evsys	The evsys system call which is
**					used for events stuff which does
**					not apply to a particular event
**					queue.
**		ev_sysint.c/ev_fork	Must set up the child based on
**					the state of the parent.
**
**	The evpd_t structure is freed in ev_exit when the process exits.
**
**	If new entry points are added to events, they may also need
**	checks to insure that an evpd_t structure is always allocated.
**	In particular, be carful when extending events to work in a
**	distributed environment.
*/

/*			The evpd_t Structure (Continued)
**			================================
*/

typedef struct evpd {
	evlisthd_t	epd_exprs;	/* Head of a ring of	*/
					/* all of the event	*/
					/* expressions for	*/
					/* which this process	*/
					/* has an outstanding	*/
					/* evtrap.		*/
	evlisthd_t	epd_ppsexprs;	/* Head of a ring of	*/
					/* pending poll		*/
					/* satisfied		*/
					/* expressions for the	*/
					/* process.		*/
	evlisthd_t	epd_ptsexprs;	/* Head of a ring of	*/
					/* pending trap		*/
					/* satisfied		*/
					/* expressions for the	*/
					/* process.		*/
	evlisthd_t	epd_tids;	/* Head of a ring of	*/
					/* evtid_t structures	*/
					/* for this process.	*/
	k_sigset_t	epd_sigset;	/* Set of signals for	*/
					/* which an event is to	*/
					/* be generated.	*/
	k_sigset_t	epd_sigignset;	/* Set of signals to	*/
					/* set back to SIG_DFL	*/
					/* after the evsig is	*/
					/* cancelled.		*/
	evsexpr_t	*epd_asexp;	/* Ptr to the top of	*/
					/* the stack of active	*/
					/* satisfied poll and	*/
					/* trap expressions.	*/
					/* Trap handlers for	*/
					/* all of the trap 	*/
					/* exprs are currently	*/
					/* active and calls to	*/
					/* them are nested.	*/
					/* Recursive calls of 	*/
					/* the same handler are	*/
					/* possible.		*/
	evsigr_t	*epd_sigrp;	/* Ptr to the start of	*/
					/* an array of		*/
					/* descriptions of	*/
					/* events to be 	*/
					/* generated for	*/
					/* signals.		*/
	evlisthd_t	epd_exits;	/* Head of a ring of	*/
					/* descriptions of	*/
					/* events to be		*/
					/* generated when this	*/
					/* process exits.	*/

/*			The evpd_t Structure (Continued)
**			================================
*/

	ushort		epd_ntraps;	/* Nbr of trap		*/
					/* expressions for this	*/
					/* process.  Ring of	*/
					/* these expressions is	*/
					/* headed by epd_exprs.	*/
	ushort		epd_maxtraps;	/* Max nbr of trap	*/
					/* expressions allowed	*/
					/* for this process.	*/
	ushort		epd_maxeterms;	/* Max nbr of terms	*/
					/* allowed per 		*/
					/* expression for this	*/
					/* process.		*/
	ushort		epd_holdall :1;	/* Set if a TR_ALL hold	*/
					/* has been imposed on	*/
					/* the process by the	*/
					/* call of a trap	*/
					/* handler or by an	*/
					/* evcntl EC_TRAPHOLD	*/
					/* or EC_TRAPSET type	*/
					/* function.		*/
	ushort		epd_restart :1;	/* Flag set if we	*/
					/* interrupted process	*/
					/* to call trap handler	*/
					/* which specified	*/
					/* restart.		*/
	ushort		epd_astk : 1;	/* The process has	*/
					/* specified an		*/
					/* alternate stack.	*/
	ushort		epd_onastk : 1;	/* The process is 	*/
					/* currently running on	*/
					/* the alternate stack.	*/
	ushort		epd_lvl;	/* Trap expression	*/
					/* nesting level on	*/
					/* epd_asexp stack.	*/
					/*			*/
					/*  0 = No stacked 	*/
					/*      expressions.	*/
					/*  1 = Stacked trap	*/
					/*      expression from */
					/*      main program	*/
					/*      level.		*/
					/*  2 = Trap handler	*/
					/*      called while in	*/
					/*      level 1 trap	*/
					/*      handler.	*/
	caddr_t		epd_astkp;	/* Ptr to the user's	*/
					/* alternate stack.	*/
	ulong		epd_astks;	/* Size of the user's	*/
					/* alternate stack in	*/
					/* bytes.		*/
} evpd_t;

/*			The evactq_t Structure
**			======================
**
**	We keep an array of structures to refer to the active event
**	queues.  The global variable ev_actqp is a pointer to the start
**	of this array.  The array is allocated in ev_init at system 
**	startup time.  It has as many entries as the maximum number of
**	event queues we will ever create (evcinfo.evci_mevqueues).  Each
**	entry in the array has the structure defined below.  The 
**	routines ev_aq_* manipulate this array.
**
**	This array is used for several purposed.  One is to provide a
**	"directory" to read.  Since user's must be able to seek to a
**	particular directory entry and read from there, we must have
**	some way of consistently defining where we are in the "/events"
**	directory.  We can't run down a linked list of queues both for
**	performance reasons and because this could give inconsistent
**	results (like returning the same entry twice of an earlier
**	entry was deleted between successive reads).
**
**	Another use for this array is to maintain something like inode
**	numbers which we can use to return for the va_nodeid (inode
**	number) member of the vattr_t structure.  The macros
**	ev_inbrtoqp and ev_qptoinbr are used to translate back and
**	forth between a pointer to a queue and an inumber.  These
**	macros use the index into the ev_actqp array.
**
**	Yet another use of the ev_actqp array is to keep track of
**	generation numbers.  This is needed for NFS.  Because it doesn't
**	keep any state, an inode number can be reused behind its back.
**	The generation number is used to detect that this has happened.
*/

typedef struct evactq {
	evqueue_t	*eaq_qp;	/* Ptr to the queue or 	*/
					/* NULL if not queue	*/
					/* is allocated for 	*/
					/* this entry.		*/
	long		eaq_gen;	/* The generation 	*/
					/* number for this slot.*/
} evactq_t;

/*			The evfid_t Structure
**			=====================
**
**	This structure describes or "file identifier".  It is used to
**	keep track of generation numbers.  This is needed for dumb old
**	stateless NFS which never knows what is going on.
**
**	This structure must map on top of the fid_t structure defined
**	in sys/vfs.h.  A limit on the size of this structure is also
**	defined there.
*/

typedef struct evfid {
	ushort		efid_len;	/* Length of the rest	*/
					/* of this structure	*/
					/* not including this	*/
					/* member.		*/
	evactq_t	*efid_aqp;	/* Ptr to the entry in	*/
					/* the ev_actqp array	*/
					/* for the file (queue).*/
	long		efid_gen;	/* The saved generation	*/
					/* number of a file.	*/
} evfid_t;

/*			Memory Manager Interface
**			========================
**
**	The following definitions are for the different memory types
**	which the memory manager in evmmgt.c must handle.  These
**	symbols are used as the first argument to ev_mem_alloc and
**	ev_mem_free.
**
**	Note that enumeration members are guaranteed to be allocated
**	consecutive values starting at zero unless specific values are
**	assigned.  The code relies on this fact.
*/

typedef enum evmt {

	EV_MT_NONE,	/* No data required indicator.		*/
	EV_MT_EVQ,	/* An evqueue_t structure.		*/
	EV_MT_KEV,	/* A evkev_t structure.			*/
	EV_MT_EXREF,	/* An evexref_t structure.		*/
	EV_MT_EXPR,	/* An evexpr_t structure.		*/
	EV_MT_TERM,	/* An evterm_t structure.		*/
	EV_MT_SEXPR,	/* An evsexpr_t structure.		*/
	EV_MT_STERM,	/* An evsterm_t structure.		*/
	EV_MT_TID,	/* An evtid_t structure.		*/
	EV_MT_RETRY,	/* An evretry_t structure.		*/
	EV_MT_EXITR,	/* An evexitr_t structure.		*/
	EV_MT_SIGR,	/* An array of evsigr_t structure.	*/
	EV_MT_PD,	/* An evpd_t structure.			*/
	EV_MT_FID,	/* An evfid_t structure.		*/
	EV_MT_EXITD,	/* An evd_exit_t structure.		*/
	EV_MT_STREAMD,	/* An evd_stream_t structure.		*/
	EV_MT_DIRENT,	/* An events dirent_t structure.	*/
	EV_MT_DATA,	/* Space for event type dependent data	*/
			/* and other miscellaneous uses.	*/

	EV_MT_NBR	/* The number of different memory types	*/
			/* we handle.  This member must be the	*/
			/* last one in the enumeration.  Add	*/
			/* new types before this one.		*/
} evmt_t;

#define	EV_MT_NMSZ	16	/* Size for the emmi_name field	*/
				/* below.  Should be large 	*/
				/* for the largest name plus a	*/
				/* terminating null byte.	*/

/*			The evmminfo_t Structure
**			========================
**
**	This structure is used by the memory management routines in the
**	file ev_mmgt.c.  No other part of the events VFS should
**	reference this structure.  The structure contains data about
**	one of the data items which the memory manager allocates.  An
**	array of these items, evmminfo, is indexed by the item codes
**	EV_MT_XXX defined above.
*/

typedef struct evmminfo {
	ushort		emmi_size;	/* Size of the item.	*/
	ushort		emmi_wanted :1;	/* Set if one or more	*/
					/* processes are 	*/
					/* waiting to allocate	*/
					/* one of these items	*/
					/* because the number 	*/
					/* allocated reached 	*/
					/* the limit.		*/
	int		emmi_nalloced;	/* The nbr of items 	*/
					/* allocated.		*/
	int		emmi_maxalloc;	/* The maximum nbr of	*/
					/* items to allocate.	*/
					/* This member is	*/
					/* initialized from the	*/
					/* evcinfo table 	*/
					/* defined in our 	*/
					/* master file.		*/
	char		*emmi_firstp;	/* Ptr to start of the	*/
					/* allocated area.  	*/
					/* Used only for	*/
					/* debugging.		*/
	char		*emmi_lastp;	/* Ptr to next byte	*/
					/* beyond end of the	*/
					/* allocated area.	*/
					/* Used only for 	*/
					/* debugging.		*/
	char		*emmi_freep;	/* Ptr to head of free	*/
					/* list of structures.	*/

	char		emmi_name[EV_MT_NMSZ];
					/* Name to be used in	*/
					/* messages about this	*/
					/* structure.		*/
} evmminfo_t;

/*			The evdr_t Structure
**			====================
**
**	This structure is used by drivers and streams modules which
**	wish to post events.  A pointer to one of these structures is
**	an argument to the ev_dr_post function in ev_sysint.c.  The
**	driver interface is described on the evdri(7) manual page.
*/

typedef struct evdr {
	long		evdr_flags;	/* Same as ev_flags in	*/
					/* the event_t 		*/
					/* structure.		*/
	long		evdr_eid;	/* Same as ev_eid in	*/
					/* the event_t 		*/
					/* structure.		*/
	long		evdr_pri;	/* Same as ev_pri in	*/
					/* the event_t 		*/
					/* structure.		*/
	hostid_t	evdr_hostid;	/* Same as ev_hostid in	*/
					/* the event_t 		*/
					/* structure.		*/
	pid_t		evdr_pid;	/* Same as ev_pid in	*/
					/* the event_t 		*/
					/* structure.		*/
	uid_t		evdr_uid;	/* Same as ev_uid in	*/
					/* the event_t 		*/
					/* structure.		*/
	size_t		evdr_datasize;	/* Same as ev_datasize	*/
					/* in the event_t	*/
					/* structure.		*/
	char		*evdr_data;	/* Same as ev_data in	*/
					/* the event_t 		*/
					/* structure.		*/
} evdr_t;

/*			The evcntxt_t Structure
**			=======================
**
**	The following structure is an event trap handler context.
**	It is the structure which is built on the stack when calling
**	a trap handler.  A pointer to this structure is the last
**	argument to the user's handler and is the argument a user must
**	supply to the EC_TRAPRET evcntl command to return to the 
**	interrupted context.
*/

typedef struct evcntxt {
	event_t		*ectxt_elp;	/* Ptr to list of 	*/
					/* events from trap.	*/
	int		ectxt_els;	/* Nbr of events in the	*/
					/* list extxt_elp.	*/
	long		ectxt_tid;	/* The trap identifier	*/
					/* for the expression	*/
					/* which was satisfied.	*/
	struct evcntxt	*ectxt_cntxtp;	/* Pointer to this 	*/
					/* context.		*/
	uint		ectxt_lvl;	/* The nesting level of	*/
					/* this handler.  This	*/
					/* is the level of the	*/
					/* handler called with	*/
					/* a ptr to this 	*/
					/* structure as its 	*/
					/* argument.		*/

	void		(*ectxt_ufunc)();
					/* Ptr to the user's	*/
					/* trap handler 	*/
					/* function.		*/
} evcntxt_t;

/*			Global Data
**			===========
**
**	The following data is defined in the events master file.
*/

#ifdef	_KERNEL

extern evcinfo_t	evcinfo;	/* Event configuration 	*/
					/* information.		*/


/*	The following data is in evfilenames.c
*/

extern evlisthd_t	*ev_fnhtp;	/* Ptr to the hash 	*/
					/* table for file names.*/

/*	The following data is defined in evqueues.c.
*/

extern evactq_t		*ev_actqp;	/* Ptr to an array of	*/
					/* ptrs to active	*/
					/* queues.		*/

/*	The following data is defined in evtids.c.
*/

extern evlisthd_t	*ev_tidhtp;	/* Ptr to the hash 	*/
					/* table for the trap	*/
					/* identifiers.		*/

/*	The following data is defined in evvfsops.c.
*/

extern evdir_t		ev_dotdirs[];	/* The directory 	*/
					/* entries for "." and	*/
					/* "..".		*/
extern vnode_t		*ev_rootvp;	/* Ptr to the vnode 	*/
					/* for the root of the	*/
					/* events VFS.		*/
extern evqueue_t	*ev_rootqp;	/* Ptr to the queue for	*/
					/* the root of the	*/
					/* events VFS.		*/
extern dev_t		ev_dev;		/* Our device code.	*/
extern short		ev_fstype;	/* Our file system type.*/
extern off_t		ev_dirsize;	/* Total size of our 	*/
					/* directory.		*/
extern unchar		ev_mounted;	/* Flag set if our VFS	*/
					/* is mounted.		*/
extern unchar		ev_init_ok;	/* Flag set if events	*/
					/* vfs initialization 	*/
					/* is completed		*/
					/* successfully.	*/

/*			Global Data (Continued)
**			=======================
**
**	The following is kernel (non-events) data which we reference.
*/

extern int		mau_present;	/* Set if a mau is	*/
					/* present on the 	*/
					/* system.		*/

/*			Function Declarations
**			=====================
**
**	The following are all the functions in the events files.
**
**	The following functions are in evcntl.c.
*/

extern err_t		ev_cntl_evsys();
extern err_t		ev_evcntl();
extern err_t		ev_cntl_traphold();
extern err_t		ev_cntl_trapset();
extern err_t		ev_cntl_traprelse();
extern err_t		ev_cntl_trappause();
extern err_t		ev_cntl_trapret();
extern err_t		ev_cntl_trapend();
extern err_t		ev_cntl_altstack();
extern err_t		ev_cntl_getprinfo();
extern err_t		ev_cntl_setprinfo();
extern err_t		ev_cntl_getcfginfo();
extern err_t		ev_cntl_getmeminfo();

/*	The following functions are in evexit.c.
*/

extern err_t		ev_exit_evsys();
extern err_t		ev_evexit();
extern err_t		ev_exit_add();
extern err_t		ev_exit_cancel();
extern void		ev_exit_post();
extern void		ev_exit_free();

/*	These functions are in evexprs.c.
*/

extern err_t		ev_expr_bld();
extern err_t		ev_expr_bldholdlist();
extern err_t		ev_expr_satisfy();
extern err_t		ev_expr_satisfymore();
extern err_t		ev_term_check();
extern err_t		ev_term_satisfy();
extern err_t		ev_expr_dup();
extern err_t		ev_term_dup();
extern void		ev_expr_free();
extern void		ev_term_free();
extern err_t		ev_expr_list();
extern void		ev_expr_unlist();
extern err_t		ev_expr_tolist();
extern void		ev_expr_fromlist();

/*			Functions Declarations (Continued)
**			==================================
**
**	The following functions are in evfilenames.c.
*/

extern err_t		ev_fn_lookup();
extern void		ev_fn_insert();
extern void		ev_fn_delete();
extern evqueue_t	*ev_fn_hash();

/*	The following functions are in evkevs.c
*/

extern err_t		ev_kev_post();
extern void		ev_kev_free();
extern void		ev_kev_enq();
extern void		ev_kev_deq();

/*	These functions are in evmmgt.c.
*/

extern err_t		ev_mem_init();
extern err_t		ev_mem_alloc();
extern void		ev_mem_free();
extern void		ev_mem_rtrninfo();
extern void		ev_mem_statvfs();

/*	The following functions are in evpoll.c.
*/

extern err_t		ev_poll_evsys();
extern err_t		ev_evpoll();
extern void		ev_poll_timeout();
extern err_t		ev_poll_quick();
extern err_t		ev_poll_dupexpr();
extern err_t		ev_poll_bldsexpr();
extern err_t		ev_pollmore_evsys();
extern err_t		ev_evpollmore();

/*	These functions are in evpost.c.
*/

extern err_t		ev_post_evsys();
extern err_t		ev_evpost();
extern err_t		ev_post_event();
extern err_t		ev_post_retry();
extern err_t		ev_post_getpfd();
extern err_t		ev_post_getmem();
extern err_t		ev_post_getshm();
extern err_t		ev_post_err();

/*			Functions Declarations (Continued)
**			==================================
**
**	The following functions are in evqcntl.c.
*/

extern err_t		ev_qcntl_evsys();
extern err_t		ev_evqcntl();
extern err_t		ev_qcntl_getcm();
extern err_t		ev_qcntl_setcm();
extern err_t		ev_qcntl_getqinfo();
extern err_t		ev_qcntl_setqinfo();
extern err_t		ev_qcntl_getevinfo();

/*	The following functions are in evqueues.c.
*/

extern void		ev_aq_insert();
extern void		ev_aq_delete();
extern err_t		ev_qaccess();
extern void		ev_qtrunc();

/*	The following functions are in evsexprs.c.
*/

extern void		ev_sexpr_toproc();
extern err_t		ev_sexpr_dupstk();
extern evsexpr_t	*ev_sexpr_dup();
extern err_t		ev_sexpr_tousr();
extern err_t		ev_sterm_tousr();
extern err_t		ev_sterm_memtousr();
extern err_t		ev_sterm_shmtousr();
extern void		ev_sterm_quickdtousr();
extern err_t		ev_sterm_pfdtousr();
extern void		ev_sexpr_hipri();
extern evsexpr_t	*ev_sexpr_findpending();
extern evsexpr_t	*ev_sexpr_getpending();
extern void		ev_sexpr_push();
extern evsexpr_t	*ev_sexpr_pop();
extern void		ev_sexpr_unsatisfy();
extern void		ev_sexpr_free();
extern void		ev_sterm_free();

/*			Functions Declarations (Continued)
**			==================================
**
**	The following functions are in evsig.c.
*/

extern err_t		ev_sig_evsys();
extern err_t		ev_evsig();
extern void		ev_sig_initinfo();
extern void		ev_sig_saveinfo();
extern err_t		ev_sig_rtrninfo();
extern err_t		ev_sig_dup();
extern void		ev_sig_del();

/*	The following functions are in evsubrs.c.
*/

extern void		ev_proc_check();
extern struct proc	*ev_checkq();
extern void		ev_proc_clean();
extern err_t		ev_trapret();
extern void		ev_untrap();
extern caddr_t		ev_trap_getstk();
extern err_t		ev_eqdtovp();
extern err_t		ev_read_dir();
extern err_t		ev_write_getdata();
extern void		ev_nonfatalerr();

/*	These functions are in evsysint.c.
*/

extern err_t		ev_evsys();
extern int		ev_istrap();
extern int		ev_intr_restart();
extern void		ev_traptousr();
extern err_t		ev_evtrapret();
extern void		ev_exec();
extern void		ev_exit();
extern err_t		ev_fork();
extern void		ev_gotsig();
extern void		ev_signal();
extern void		ev_newpri();
extern err_t		ev_stream_post();
extern err_t		ev_dr_post();

/*			Functions Declarations (Continued)
**			==================================
**
**	The following functions are in evtids.c.
*/

extern err_t		ev_tid_init();
extern err_t		ev_tid_dupproclist();
extern err_t		ev_tid_dupexprlist();
extern void		ev_tid_clean();
extern evtid_t		*ev_tid_add();
extern void		ev_tid_rem();
extern evtid_t		*ev_tid_find();
extern void		ev_tid_hold();
extern void		ev_tid_relse();
extern evtid_t		*ev_tid_hash();

/*	The following functions are in evtrap.c.
*/

extern err_t		ev_trap_evsys();
extern err_t		ev_evtrap();
extern err_t		ev_trapcan_evsys();
extern err_t		ev_evtrapcancel();
extern void		ev_trapcancel();

/*	The following are all of the functions in evvfsops.c.  They
**	must be here because they are forward referenced to define
**	the ev_vfsops array in that file.
*/

extern void		ev_init();
extern err_t		ev_mount();
extern err_t		ev_unmount();
extern err_t		ev_root();
extern err_t		ev_statvfs();
extern err_t		ev_sync();
extern err_t		ev_vget();
extern err_t		ev_mountroot();
extern err_t		ev_swapvp();

/*			Functions Declarations (Continued)
**			==================================
**
**	The following are all of the functions in evvnodeops.c.  They
**	must be here because they are forward referenced to define the
**	ev_vnodeops array in that file.
*/

extern err_t		ev_open();
extern err_t		ev_close();
extern err_t		ev_read();
extern err_t		ev_write();
extern err_t		ev_ioctl();
extern err_t		ev_setfl();
extern err_t		ev_getattr();
extern err_t		ev_setattr();
extern err_t		ev_access();
extern err_t		ev_lookup();
extern err_t		ev_create();
extern err_t		ev_remove();
extern err_t		ev_link();
extern err_t		ev_rename();
extern err_t		ev_mkdir();
extern err_t		ev_rmdir();
extern err_t		ev_readdir();
extern err_t		ev_symlink();
extern err_t		ev_readlink();
extern err_t		ev_fsync();
extern void		ev_inactive();
extern err_t		ev_fid();
extern void		ev_rwlock();
extern void		ev_rwunlock();
extern err_t		ev_seek();
extern int		ev_cmp();
extern err_t		ev_frlock();
extern err_t		ev_space();
extern err_t		ev_realvp();

/*			Function Declarations (Continued)
**			=================================
**
**	These are all of the kernel (non-events) functions which we
**	use.
*/

extern struct seg	*as_segat();	/* Find the segment	*/
					/* containing a 	*/
					/* particular virtual	*/
					/* address for a	*/
					/* particular process.	*/
extern struct seg	*amtoseg();	/* Find the segment 	*/
					/* which refers to a	*/
					/* particular anon_map 	*/
					/* for a process.	*/
extern struct anon_map	*as_shmlookup();/* Find the anon_map 	*/
					/* for the shared memory*/
					/* segment containing	*/
					/* a particular virtual	*/
					/* address in this 	*/
					/* address space.	*/
extern int		ttimeout();	/* Call a kernel	*/
					/* function after a	*/
					/* specified time	*/
					/* interval in ticks.	*/
extern int		untimeout();	/* Cancel a timer set	*/
					/* with the timeout	*/
					/* function.		*/
extern int		reglock();	/* Lock a region.	*/
extern int		regrele();	/* Unlock a region.	*/
extern void		freereg();	/* Free a region.	*/
extern int		getudev();	/* Get an unused major	*/
					/* device code.		*/
extern int		fixuserpsw();	/* Fix a psw to be	*/
					/* valid for running in	*/
					/* user mode.		*/
#endif	/* _KERNEL	*/

#endif	/* _SYS_EVSYS_H */
