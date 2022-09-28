/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/hdrs/bkdaemon.h	1.9.2.1"

/* A parsed entry from the backup schedule table is put here */
typedef	struct	brentry_s {
	unsigned char *tag;
	unsigned char *dependencies;
	unsigned char *oname;
	unsigned char *odevice;
	unsigned char *olabel;
	bkrotate_t	date;
	unsigned char *options;
	int priority;
	unsigned char *method;
	unsigned char *dgroup;
	unsigned char *ddevice;
	unsigned char *dchar;
	unsigned char *dlabel;
} brentry_t;

/*
	Process information structure - this information is kept in a separate
	place from owner_t and method_t to facilitate pid -> type lookup.
*/
typedef	struct	proc_s	{
	pid_t pid;
	int type;
	int	slot;	/* slot number in table for this process's type */
	uid_t uid;	/* user id of this process */
	gid_t	gid;	/* group id of this process */
} proc_t;

/* Values for process type */
#define	UNUSED_P	0
#define	OWNER_P	1	/* a backup command that owns methods */
#define	CONTROL_P	2	/* a backup command doing restart, suspend, or cancel */
#define	METHOD_P	3
#define	OPERATOR_P	4	/* a bkoper command */

/* Data Structure to describe Methods */
/* Used label list */
typedef struct lbl_s {
	struct lbl_s *next;
	char *label;
} lbl_t;

/* Dependency list */
typedef struct dep_s {
	struct dep_s *next;	/* next dependency in the chain */
	int m_slot;	/* ptr. to method structure of dependency */
} dep_t;

#define	MD_MAXARGS	60 /* maximum number of arguments for a method */
typedef struct	method_s	{
	int	o_slot;	/* slot number in process table of owning backup command */
	int	p_slot;	/* process slot number of this process */
	int c_slot;	/* which CONTROL this method is responding to */
	int	use_count;	/* number of cqueues that this method is on */
	int	next_l;	/* slot number of next method on local schedule */
	int	next_g;	/* slot number of next method on global schedule */
	int state;	/* what state the method is in */
	int	status;	/* what the current status (e.g. PENDING, etc.) is */
	int	ostatus; /* If status is "halted", what the status was. */
	brentry_t	entry;	/* entry from the table */
	unsigned char *dchar; /* Final dchar given to method */
	time_t starttime;	/* when this backup became active */
	time_t toctime;	/* save starttime of original method for TOC processing */
	int e_volumes;	/* # volumes from estimate msg. */
	int e_blocks;	/* # blocks from estimate msg. */
	int nblocks;	/* No. blocks from DONE message */
	lbl_t *used_labels;	/* List of 'used labels' */
	dep_t *deps;	/* dependency list */
	dep_t *m_waiting;	/* methods to wakeup when we're done */
	char *argv[ MD_MAXARGS ];	/* argv structure for spawning */
	void	(*ev_fcn)();	/* Function that this method is sleeping on */
	long	ev_arg;	/* Argument that this method is sleeping on */
} method_t;

/* defines for method_t state */
#define	MD_ALLOCATED	0x1
#define	MD_NEED_OPER	0x2
#define	MD_NOT_SPAWNED	0x4		/* Unable to spawn the method */
#define	MD_ARGVFILLED	0x8
#define	MD_DEVALLOCED	0x10	/* Devices have been allocated */
#define	MD_DONE			0x20	/* Method is finished */
#define	MD_SPAWNED		0x40	/* Method is currently running */
#define	MD_CONTROLLING	0x80	/* Method hasn't responded to controling msg */
#define	MD_IS_TOC		0x100	/* Is a Table of contents method */
#define	MD_NEEDTOC		0x200	/* Need to spawn a TOC method */
#define	MD_MSG_SENT		0x400	/* Success/Fail msg. sent to owner */
#define	MD_FAIL			0x800	/* Method terminated abnormally */
#define	MD_SLEEPING		0x1000	/* Waiting for an event to occur */

/* why md_terminate() was called */
#define	MDT_EXIT	1	/* method called exit(2) */
#define	MDT_CANCEL	2	/* method was canceled */
#define	MDT_FAIL	3	/* method sent FAILED msg */
#define	MDT_DONE	4	/* method sent DONE msg */

/* status and ostatus values */
#define	MD_ACTIVE	1
#define	MD_PENDING	2
#define	MD_WAITING	3
#define	MD_HALTED	4
#define	MD_FAILED	5
#define	MD_SUCCESS	6

/* Structure to describe "owning" backup commands */
typedef struct	owner_s {
	int	p_slot;	/* process number slot */
	int methods;	/* linked list of methods that this process owns */
	char table[ BKFNAME_SZ ];	/* table that this method is using */
	char fname[ BKFNAME_SZ ];	/* do only this origination */
	char owner[ BKMAIL_SZ ];	/* who to send mail to */
	int controling;	/* number of methods responding to a control message */
	int state;
	int options;
	int week, day;
	time_t starttime;
	int m_succeeded, m_failed;
} owner_t;

/* defines for owner_t state */
#define	O_ALLOCATED	0x1
#define	O_TALKING	0x2
	
/*
	Structure to describe "controlling" backup commands - those that
	are doing suspends, restarts, or cancels
*/
/* A CONTROL's queue of controled methods */
typedef struct	cqueue_s {
	int m_slot;	/* SLOT of this method */
	struct cqueue_s	*next_c;	/* SLOT of next method on the queue */
} cqueue_t;

typedef	struct	cntrl_s {
	int p_slot;	/* process slot of this process */
	int o_slot;	/* OWNER slot iff OWNER is doing the CONTROL */
	uid_t	uid;	/* UID of who's methods to control */
	pid_t pid;	/* PID of OWNER of methods to control */
	cqueue_t *cqueue;	/* queue of controlled methods */
	int ntodo;	/* Number of methods that haven't yet responded */
	int state;
	int type;	/* Control message type */
} control_t;

#define	C_ALLOCATED	0x1

/* Structure for operators */
typedef struct	op_s {
	int p_slot;	/* process slot of this process */
	int	state;	
} operator_t;

/* Values for state */
#define	NEW_MESSAGES	0x1
#define ACTIVE_METHODS	0x2
#define	SOME_DONE	0x4
#define	TALKING	0x8
#define	NEW_EVENTS	0x10
#define	EVENTS	0x20

/* Various other things */
/* slot to pointer mappings */
#define	C_SLOT(n)	((n > 0 && n < controltabsz)? (controltab + n): 0)
#define	MD_SLOT(n)	((n > 0 && n < methodtabsz)? (methodtab + n): 0 )
#define	O_SLOT(n)	((n > 0 && n < ownertabsz)? (ownertab + n): 0)
#define	P_SLOT(n)	((n > 0 && n < proctabsz)? (proctab + n): 0)

/* ptr to slot mappings */
#define	C_SLOTNO(c)	\
	((c >= controltab && c <= controlab + controltabsz)? c - controltab: 0)
#define	MD_SLOTNO(m) \
	((m >= methodtab && m <= methodtab + methodtabsz)? m - methodtab: 0)
#define	O_SLOTNO(o) \
	((o >= ownertab && o <= ownertab + ownertabsz)? o - ownertab: 0)
#define	P_SLOTNO(p) \
	((p >= proctab && p <= proctab + proctabsz)? p - proctab: 0)

/* Status setting routines */
#define	st_setactive( m_slot )	st_set( m_slot, MD_ACTIVE, NULL )
#define	st_setpending( m_slot )	st_set( m_slot, MD_PENDING, NULL )
#define	st_setsuccess( m_slot )	st_set( m_slot, MD_SUCCESS, NULL )
#define	st_setwaiting( m_slot, label ) st_set( m_slot, MD_WAITING, label )
#define	st_setsuspended( m_slot ) st_set( m_slot, MD_HALTED, NULL )

/* md_flagset() argument - check for ALL flags set, or SOME flags set */
#define MD_ALL	1
#define	MD_SOME	2

#define	NULLSLOT	-1
#define	LOWEST_PRIORITY	-1

	/* Does this process exist */
#define	P_EXISTS( pid )	(p_findproc( pid ) != -1)

#define	md_alldone( oslot )	md_flagset( oslot, (MD_DONE|MD_NOT_SPAWNED), MD_ALL )
#define	md_controlling( oslot ) md_flagset( oslot, MD_CONTROLLING, MD_SOME )
#define	md_anysucceeded( oslot ) (!md_flagset( oslot, MD_FAIL, MD_ALL ))
