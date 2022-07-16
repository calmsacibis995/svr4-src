/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_STRSUBR_H
#define _SYS_STRSUBR_H

#ident	"@(#)head.sys:sys/strsubr.h	1.17.3.1"

/*
 * WARNING:
 * Everything in this file is private, belonging to the
 * STREAMS subsystem.  The only guarantee made about the
 * contents of this file is that if you include it, your
 * code will not port to the next release.
 */
#include <sys/vnode.h>
#include <sys/evecb.h>

/*
 * Header for a stream: interface to rest of system.
 */
typedef struct stdata {
	struct queue *sd_wrq;		/* write queue */
	struct msgb *sd_iocblk;		/* return block for ioctl */
	struct vnode *sd_vnode;		/* pointer to associated vnode */
	struct streamtab *sd_strtab;	/* pointer to streamtab for stream */
	long sd_flag;			/* state/flags */
	long sd_iocid;			/* ioctl id */
	ushort sd_iocwait;		/* count of procs waiting to do ioctl */
	struct pid *sd_sidp;		/* controlling session info */
	struct pid *sd_pgidp;		/* controlling process group info */
	ushort sd_wroff;		/* write offset */
	int sd_rerror;			/* read error to set u.u_error */
	int sd_werror;			/* write error to set u.u_error */
	int sd_pushcnt;			/* number of pushes done on stream */
	int sd_sigflags;		/* logical OR of all siglist events */
	struct strevent *sd_siglist;	/* pid linked list to rcv SIGPOLL sig */
	int sd_eventflags;		/* logical OR of all eventlist events */
	struct strevent *sd_eventlist;	/* linked list to rcv general events */
	struct pollhead sd_pollist;	/* list of all pollers to wake up */
	struct msgb *sd_mark;		/* "marked" message on read queue */
	int sd_closetime;		/* time to wait to drain q in close */
	clock_t sd_rtime;		/* time to release held message */
} stdata_t;

/*
 * stdata flag field defines
 */
#define	IOCWAIT		0x00000001	/* Someone wants to do ioctl */
#define RSLEEP		0x00000002	/* Someone wants to read/recv msg */
#define	WSLEEP		0x00000004	/* Someone wants to write */
#define STRPRI		0x00000008	/* An M_PCPROTO is at stream head */
#define	STRHUP		0x00000010	/* Device has vanished */
#define	STWOPEN		0x00000020	/* waiting for 1st open */
#define STPLEX		0x00000040	/* stream is being multiplexed */
#define STRISTTY	0x00000080	/* stream is a terminal */
#define RMSGDIS		0x00000100	/* read msg discard */
#define RMSGNODIS	0x00000200	/* read msg no discard */
#define STRDERR		0x00000400	/* fatal read error from M_ERROR */
#define STRTIME		0x00000800	/* used with timeout strtime */
#define STR2TIME	0x00001000	/* used with timeout str2time */
#define STR3TIME	0x00002000	/* used with timeout str3time */
#define STRCLOSE	0x00004000	/* wait for a close to complete */
#define SNDMREAD	0x00008000	/* used for read notification */
#define OLDNDELAY	0x00010000	/* use old TTY semantics for NDELAY
					/* reads and writes */
#define RDBUFWAIT	0x00020000	/* used with bufcall in strqbuf() */
#define STRSNDZERO	0x00040000	/* send 0-length msg. down pipe/FIFO */
#define STRTOSTOP	0x00080000	/* block background writes */
#define	RDPROTDAT	0x00100000	/* read M_[PC]PROTO contents as data */
#define RDPROTDIS	0x00200000	/* discard M_[PC]PROTO blocks and */
					/* retain data blocks */
#define STRMOUNT	0x00400000	/* stream is mounted */
#define STRSIGPIPE		0x00800000	/* controlling process not group */
#define STRDELIM	0x01000000	/* generate delimited messages */
#define STWRERR		0x02000000	/* fatal write error from M_ERROR */
#define STRHOLD		0x04000000	/* enable strwrite message coalescing */


/*
 * Structure of list of processes to be sent SIGSEL signal
 * on request, or for processes sleeping on select().  The valid 
 * SIGSEL events are defined in stropts.h, and the valid select()
 * events are defined in select.h.
 */
struct strevent {
	union {
		struct {
			struct proc	*procp;
			long		events;
		} e;	/* stream event */
		struct {
			void (*func)();
			long arg;
			int size;
		} b;	/* bufcall event */
		struct {
			vnode_t		*vp;
			ecb_t		 ecb;
			pid_t		 pid;
			hostid_t	 hostid;
			uid_t		 uid;
			long		 mask;
			unchar		 band;
		} g;	/* general events mechanism */
	} x;
	struct strevent *se_next;
};

#define se_procp	x.e.procp
#define se_events	x.e.events
#define se_func		x.b.func
#define se_arg		x.b.arg
#define se_size		x.b.size
#define se_vp		x.g.vp
#define se_kecb		x.g.ecb		/* name collision with struct str_sev */
#define se_pid		x.g.pid
#define se_hostid	x.g.hostid
#define se_uid		x.g.uid
#define se_kmask	x.g.mask	/* name collision with struct str_sev */
#define se_kband	x.g.band	/* name collision with struct str_sev */

#define SE_SLEEP	0	/* ok to sleep in allocation */
#define SE_NOSLP	1	/* don't sleep in allocation */

/*
 * bufcall list
 */
struct bclist {
	struct strevent	*bc_head;
	struct strevent	*bc_tail;
};

/*
 * Structure used to track mux links and unlinks.
 */
struct mux_node {
	long		 mn_imaj;	/* internal major device number */
	ushort		 mn_indegree;	/* number of incoming edges */
	struct mux_node *mn_originp;	/* where we came from during search */
	struct mux_edge *mn_startp;	/* where search left off in mn_outp */
	struct mux_edge *mn_outp;	/* list of outgoing edges */
	uint		 mn_flags;	/* see below */
};

/*
 * Flags for mux_nodes.
 */
#define VISITED	1

/*
 * Edge structure - a list of these is hung off the
 * mux_node to represent the outgoing edges.
 */
struct mux_edge {
	struct mux_node	*me_nodep;	/* edge leads to this node */
	struct mux_edge	*me_nextp;	/* next edge */
	int		 me_muxid;	/* id of link */
};

/*
 * Structure to keep track of resources that have been allocated
 * for streams - an array of these are kept, one entry per
 * resource.  This is used by crash to dump the data structures.
 */

struct strinfo {
	void	*sd_head;	/* head of in-use list */
	int	sd_cnt;		/* total # allocated */
};

#define DYN_STREAM	0	/* for stream heads */
#define DYN_QUEUE	1	/* for queues */
#define DYN_MSGBLOCK	2	/* for message blocks */
#define DYN_MDBBLOCK	3	/* for mesg/data/buffer triplets */
#define DYN_LINKBLK	4	/* for mux links */
#define DYN_STREVENT	5	/* for stream event cells */
#define DYN_QBAND	6	/* for qband structures */

#define NDYNAMIC	7	/* number of different data types that are */
				/* dynamically allocated */

/*
 * The following structures are mainly used to keep track of
 * the resources that have been allocated so crash can find
 * them (they are stored in a doubly-linked list with the head
 * of it stored in the Strinfo array.  Other data may be stored
 * away here too since this is private to streams.  Pointers
 * to these objects are returned by the allocating procedures,
 * which are later passed to the freeing routine.  The data
 * structure itself must appear first because the pointer is
 * overloaded to refer to both the structure itself or its
 * envelope, depending on context.
 */

/*
 * Stream head info
 */
struct shinfo {
	stdata_t	sh_stdata;	/* must be first */
	struct shinfo	*sh_next;	/* next in list */
	struct shinfo	*sh_prev;	/* previous in list */
};

/*
 * data block info
 */
#ifdef DEBUG
struct dbinfo {
	dblk_t	d_dblock;	/* the data block itself */
	struct dbinfo *d_next;	/* next data block */
	struct dbinfo *d_prev;	/* previous */
};

/*
 * message block info
 */
struct mbinfo {
	mblk_t	m_mblock;	/* the message block itself */
	struct mbinfo *m_next;	/* next message block */
	struct mbinfo *m_prev;	/* previous message block */
	void	(*m_func)();	/* address of allocation function */	
};
#else
/*
 * data block info
 */
struct dbinfo {
	dblk_t	d_dblock;
};
/*
 * message block info
 */
struct mbinfo {
	mblk_t	m_mblock;
	void	(*m_func)();
};
#endif



/* convenient power of 2 */
#define	FASTBUF	(128 - sizeof(struct mbinfo) - sizeof(struct dbinfo))

/*
 * triplet
 */
struct	mdbblock {
	struct	mbinfo	msgblk;
	struct	dbinfo	datblk;
	char	databuf[FASTBUF];
};


#ifdef DEBUG
#define _INSERT_MSG_INUSE(x)	insert_msg_inuse(x)
#define _INSERT_MDB_INUSE(y)	insert_mdb_inuse(y)
#define _DELETE_MSG_INUSE(x)	delete_msg_inuse(x)
#define _DELETE_MDB_INUSE(y)	delete_mdb_inuse(y)
#else
#define _INSERT_MSG_INUSE(x)
#define _INSERT_MDB_INUSE(y)
#define _DELETE_MSG_INUSE(x)
#define _DELETE_MDB_INUSE(y)
#endif


/*
 * Stream event info
 */
struct seinfo {
	struct strevent	s_strevent;	/* must be first */
	struct seinfo	*s_next;	/* next in list */
	struct seinfo	*s_prev;	/* previous in list */
};

/*
 * Queue info
 */
#ifdef _STYPES

struct queinfo {
	struct queue	qu_rqueue;	/* read queue - must be first */
	struct queue	qu_wqueue;	/* write queue - must be second */
	struct equeue	qu_requeue;	/* extended queue info (read) */
	struct equeue	qu_wequeue;	/* extended queue info (write) */
	struct queinfo	*qu_next;	/* next in list */
	struct queinfo	*qu_prev;	/* previous in list */
};

#else /* expanded struct */

struct queinfo {
	struct queue	qu_rqueue;	/* read queue - must be first */
	struct queue	qu_wqueue;	/* write queue - must be second */
	struct queinfo	*qu_next;	/* next in list */
	struct queinfo	*qu_prev;	/* previous in list */
};

#endif /* _STYPES */


/*
 * Multiplexed streams info
 */
struct linkinfo {
	struct linkblk	li_lblk;	/* must be first */
	struct file	*li_fpdown;	/* file pointer for lower stream */
	struct linkinfo	*li_next;	/* next in list */
	struct linkinfo *li_prev;	/* previous in list */
};

/*
 * Qband info
 */
struct qbinfo {
	struct qband	qbi_qband;	/* must be first */
	struct qbinfo	*qbi_next;	/* next in list */
	struct qbinfo	*qbi_prev;	/* previous in list */
};

/*
 * Miscellaneous parameters and flags.
 */

/*
 * Default timeout in seconds for ioctls and close
 */
#define STRTIMOUT 15

/*
 * Flag values for stream io waiting procedure (strwaitq)
 */
#define WRITEWAIT	0x1	/* waiting for write event */
#define READWAIT	0x2	/* waiting for read event */
#define NOINTR		0x4	/* error is not to be set for signal */
#define GETWAIT		0x8	/* waiting for getmsg event */

/*
 * Copy modes for tty and I_STR ioctls
 */
#define	U_TO_K 	01			/* User to Kernel */
#define	K_TO_K  02			/* Kernel to Kernel */

/*
 * canonical structure definitions
 */

#define STRLINK		"lli"
#define STRIOCTL	"iiil"
#define STRPEEK		"iiliill"
#define STRFDINSERT	"iiliillii"
#define O_STRRECVFD	"lssc8"
#define STRRECVFD	"lllc8"
#define STRNAME		"c0"
#define STRINT		"i"
#define STRTERMIO	"ssssc12"
#define STRTERMCB	"c6"
#define STRSGTTYB	"c4i"
#define STRTERMIOS	"llllc20"
#define STRLIST		"il"
#define STRSEV		"issllc1"
#define STRGEV		"ili"
#define STREVENT	"lssllliil"
#define STRLONG		"l"
#define STRBANDINFO	"ci"

#ifndef _STYPES
#define STRPIDT		"l"
#else
#define STRPIDT		"s"
#endif

/*
 * Tables we reference during open(2) processing.
 */
#define CDEVSW	0
#define FMODSW	1

/*
 * Mux defines.
 */
#define LINKNORMAL	0x01		/* normal mux link */
#define LINKPERSIST	0x02		/* persistent mux link */
#define LINKTYPEMASK	0x03		/* bitmask of all link types */
#define LINKCLOSE	0x04		/* unlink from strclose */
#define LINKIOCTL	0x08		/* unlink from strioctl */
#define LINKNOEDGE	0x10		/* no edge to remove from graph */

/*
 * Definitions of Streams macros and function interfaces.
 */

/*
 *  Queue scheduling macros
 */
#define setqsched()     qrunflag = 1	/* set up queue scheduler */
#define qready()	qrunflag	/* test if queues are ready to run */

/*
 * Macros dealing with mux_nodes.
 */
#define MUX_VISIT(X)	((X)->mn_flags |= VISITED)
#define MUX_CLEAR(X)	((X)->mn_flags &= (~VISITED)); \
			((X)->mn_originp = NULL)
#define MUX_DIDVISIT(X)	((X)->mn_flags & VISITED)

/*
 * Declarations of private routines.
 */
extern void strsendsig();
extern void strevpost();
extern void strdrpost();
extern int qattach();
extern void qdetach();
extern void strtime();
extern void str2time();
extern void str3time();
extern int putiocd();
extern int getiocd();
extern struct linkinfo *alloclink();
extern void lbfree();
extern int linkcycle();
extern struct linkinfo *findlinks();
extern queue_t *getendq();
extern int munlink();
extern int munlinkall();
extern int mux_addedge();
extern void mux_rmvedge();
extern void setq();
extern int strmakemsg();
extern int strwaitbuf();
extern void strunbcall();
extern int strwaitq();
extern void strqbuf();
extern int strctty();
extern int straccess();
extern int xmsgsize();
extern struct stdata *shalloc();
extern void shfree();
extern struct mdbblock *xmdballoc();
extern mblk_t *xmsgalloc();
extern queue_t *allocq();
extern void freeq();
extern qband_t *allocband();
extern void freeband();
extern struct strevent *sealloc();
extern void sefree();
extern void queuerun();
extern void runqueues();
extern int findmod();
extern caddr_t allocstrpage();
extern void adjfmtp();
extern int str2num();
extern void strclearpg();
extern void strclearsid();
extern void strclearctty();
extern void setqback();
extern int strcopyin();
extern int strcopyout();
extern void strsignal();
extern void strscan();

#endif	/* _SYS_STRSUBR_H */
