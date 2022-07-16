/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_STREAM_H
#define _SYS_STREAM_H

#ident	"@(#)head.sys:sys/stream.h	11.44.3.2"

/*
 * For source compatibility
 */
#include <sys/vnode.h>
#include <sys/poll.h>
#include <sys/strmdep.h>
#include <sys/cred.h>

/*
 * Data queue
 */
#ifdef _STYPES

struct	queue {
	struct	qinit	*q_qinfo;	/* procs and limits for queue */
	struct	msgb	*q_first;	/* first data block */
	struct	msgb	*q_last;	/* last data block */
	struct	queue	*q_next;	/* Q of next stream */
	struct	equeue	*q_eq;		/* extended queue info */
	_VOID		*q_ptr;		/* to private data structure */
	ushort		q_count;	/* number of bytes on Q */
	ushort		q_flag;		/* queue state */
	short		q_minpsz;	/* min packet size accepted by */
					/* this module */
	short		q_maxpsz;	/* max packet size accepted by */
					/* this module */
	ushort		q_hiwat;	/* queue high water mark */
	ushort		q_lowat;	/* queue low water mark */
};

/*
 * Extended queue structure containing information that belongs
 * in the queue, but can't be added because of binary compatibility
 * of STREAMS modules and drivers.
 */
struct equeue {
	struct queue	*eq_link;	/* to next Q for scheduling */
	struct qband	*eq_bandp;	/* separate flow information */
	unsigned char	eq_nband;	/* number of priority bands > 0 */
};

#define q_link	q_eq->eq_link
#define q_bandp	q_eq->eq_bandp
#define q_nband	q_eq->eq_nband

#else /* large definition */

struct	queue {
	struct	qinit	*q_qinfo;	/* procs and limits for queue */
	struct	msgb	*q_first;	/* first data block */
	struct	msgb	*q_last;	/* last data block */
	struct	queue	*q_next;	/* Q of next stream */
	struct	queue	*q_link;	/* to next Q for scheduling */
	_VOID		*q_ptr;		/* to private data structure */
	ulong		q_count;	/* number of bytes on Q */
	ulong		q_flag;		/* queue state */
	long		q_minpsz;	/* min packet size accepted by */
					/* this module */
	long		q_maxpsz;	/* max packet size accepted by */
					/* this module */
	ulong		q_hiwat;	/* queue high water mark */
	ulong		q_lowat;	/* queue low water mark */
	struct qband	*q_bandp;	/* separate flow information */
	unsigned char	q_nband;	/* number of priority bands > 0 */
	unsigned char	q_pad1[3];	/* reserved for future use */
	long		q_pad2[2];	/* reserved for future use */
};

#endif /* _STYPES */

typedef struct queue queue_t;

/*
 * Queue flags
 */
#define	QENAB	0x001			/* Queue is already enabled to run */
#define	QWANTR	0x002			/* Someone wants to read Q */
#define	QWANTW	0x004			/* Someone wants to write Q */
#define	QFULL	0x008			/* Q is considered full */
#define	QREADR	0x010			/* This is the reader (first) Q */
#define	QUSE	0x020			/* This queue in use (allocation) */
#define	QNOENB	0x040			/* Don't enable Q via putq */
#define	QOLD	0x080			/* Pre-SVR4 open/close interface */
#define QBACK	0x100			/* queue has been back-enabled */
#define QHLIST	0x200			/* strhead write queue is on "scanqhead" */

/*
 * Structure that describes the separate information
 * for each priority band in the queue.
 */
struct qband {
	struct qband	*qb_next;	/* next band's info */
	ulong		qb_count;	/* number of bytes in band */
	struct msgb	*qb_first;	/* beginning of band's data */
	struct msgb	*qb_last;	/* end of band's data */
	ulong		qb_hiwat;	/* high water mark for band */
	ulong		qb_lowat;	/* low water mark for band */
	ulong		qb_flag;	/* see below */
	long		qb_pad1;	/* reserved for future use */
};

typedef struct qband qband_t;

/*
 * qband flags
 */
#define QB_FULL		0x01		/* band is considered full */
#define QB_WANTW	0x02		/* Someone wants to write to band */
#define QB_BACK		0x04		/* queue has been back-enabled */

/*
 * Maximum number of bands.
 */
#define NBAND	256

/*
 * Fields that can be manipulated through strqset() and strqget().
 */
typedef enum qfields {
	QHIWAT	= 0,		/* q_hiwat or qb_hiwat */
	QLOWAT	= 1,		/* q_lowat or qb_lowat */
	QMAXPSZ	= 2,		/* q_maxpsz */
	QMINPSZ	= 3,		/* q_minpsz */
	QCOUNT	= 4,		/* q_count or qb_count */
	QFIRST	= 5,		/* q_first or qb_first */
	QLAST	= 6,		/* q_last or qb_last */
	QFLAG	= 7,		/* q_flag or qb_flag */
	QBAD	= 8
} qfields_t;

/*
 * Module information structure
 */

#ifdef _STYPES

struct module_info {
	ushort	mi_idnum;		/* module id number */
	char 	*mi_idname;		/* module name */
	short   mi_minpsz;		/* min packet size accepted */
	short   mi_maxpsz;		/* max packet size accepted */
	ushort	mi_hiwat;		/* hi-water mark */
	ushort 	mi_lowat;		/* lo-water mark */
};

#else /* large definition */

struct module_info {
	ushort	mi_idnum;		/* module id number */
	char 	*mi_idname;		/* module name */
	long	mi_minpsz;		/* min packet size accepted */
	long	mi_maxpsz;		/* max packet size accepted */
	ulong	mi_hiwat;		/* hi-water mark */
	ulong 	mi_lowat;		/* lo-water mark */
};

#endif /* _STYPES */

/*
 * queue information structure
 */
struct	qinit {
	int	(*qi_putp)();		/* put procedure */
	int	(*qi_srvp)();		/* service procedure */
	int	(*qi_qopen)();		/* called on startup */
	int	(*qi_qclose)();		/* called on finish */
	int	(*qi_qadmin)();		/* for future use */
	struct module_info *qi_minfo;	/* module information structure */
	struct module_stat *qi_mstat;	/* module statistics structure */
};

/*
 * Streamtab (used in cdevsw and fmodsw to point to module or driver)
 */

struct streamtab {
	struct qinit *st_rdinit;
	struct qinit *st_wrinit;
	struct qinit *st_muxrinit;
	struct qinit *st_muxwinit;
};

/*
 * Structure sent to mux drivers to indicate a link.
 */
#ifdef _STYPES

struct linkblk {
	queue_t *l_qtop;	/* lowest level write queue of upper stream */
				/* (set to NULL for persistent links) */
	queue_t *l_qbot;	/* highest level write queue of lower stream */
	int      l_index;	/* index for lower stream. */
};

#else /* large definition */

struct linkblk {
	queue_t *l_qtop;	/* lowest level write queue of upper stream */
				/* (set to NULL for persistent links) */
	queue_t *l_qbot;	/* highest level write queue of lower stream */
	int      l_index;	/* index for lower stream. */
	long	 l_pad[5];	/* reserved for future use */
};

#endif /* _STYPES */

/*
 * Class 0 data buffer freeing routine
 */
struct free_rtn {
	void (*free_func)();
	char *free_arg;
};

/*
 *  Data block descriptor
 */

#ifdef _STYPES

struct datab {
	union {
		struct datab	*freep;
		struct free_rtn *frtnp;
	} db_f;
	unsigned char	*db_base;
	unsigned char	*db_lim;
	unsigned char	db_ref;
	unsigned char	db_type;
	unsigned char	db_band;
	unsigned char	db_iswhat;	/* status of the mesg/data/buffer triplet */
	unsigned int	db_size;
	unsigned short	db_flag;
	unsigned short	db_pad;	
	caddr_t		db_msgaddr; 	/* triplet mesg header that points to datab */
};

#else /* large definition */

struct datab {
	union {
		struct datab	*freep;
		struct free_rtn *frtnp;
	} db_f;
	unsigned char	*db_base;
	unsigned char	*db_lim;
	unsigned char	db_ref;
	unsigned char	db_type;
	unsigned char	db_iswhat;	/* status of the mesg/data/buffer triplet */
	unsigned int	db_size;
	long		db_filler;	/* reserved for future use */
	caddr_t		db_msgaddr;	/* triplet mesg header that points to datab */
};

#endif /* _STYPES */

#define db_freep db_f.freep
#define db_frtnp db_f.frtnp

/*
 * Message block descriptor
 */

#ifdef _STYPES

struct	msgb {
	struct	msgb	*b_next;
	struct  msgb	*b_prev;
	struct	msgb	*b_cont;
	unsigned char	*b_rptr;
	unsigned char	*b_wptr;
	struct datab 	*b_datap;
};

#define b_band	b_datap->db_band
#define b_flag	b_datap->db_flag

#else /* large definition */

struct	msgb {
	struct	msgb	*b_next;
	struct  msgb	*b_prev;
	struct	msgb	*b_cont;
	unsigned char	*b_rptr;
	unsigned char	*b_wptr;
	struct datab 	*b_datap;
	unsigned char	b_band;
	unsigned char	b_pad1;
	unsigned short	b_flag;
	long		b_pad2;
};

#endif /* _STYPES */

typedef struct msgb mblk_t;
typedef struct datab dblk_t;
typedef struct free_rtn frtn_t;




/*
 * Message flags.  These are interpreted by the stream head.
 */
#define MSGMARK		0x01		/* last byte of message is "marked" */
#define MSGNOLOOP	0x02		/* don't loop message around to */
					/* write side of stream */
#define MSGDELIM	0x04		/* message is delimited */
#define MSGNOGET	0x08		/* getq does not return message */

/*
 * Streams message types.
 */

/*
 * Data and protocol messages (regular and priority)
 */
#define	M_DATA		0x00		/* regular data */
#define M_PROTO		0x01		/* protocol control */

/*
 * Control messages (regular and priority)
 */
#define	M_BREAK		0x08		/* line break */
#define M_PASSFP	0x09		/* pass file pointer */
#define M_EVENT		0x0a		/* post an event to an event queue */
#define	M_SIG		0x0b		/* generate process signal */
#define	M_DELAY		0x0c		/* real-time xmit delay (1 param) */
#define M_CTL		0x0d		/* device-specific control message */
#define	M_IOCTL		0x0e		/* ioctl; set/get params */
#define M_SETOPTS	0x10		/* set various stream head options */
#define M_RSE		0x11		/* reserved for RSE use only */

/*
 * Control messages (high priority; go to head of queue)
 */
#define	M_IOCACK	0x81		/* acknowledge ioctl */
#define	M_IOCNAK	0x82		/* negative ioctl acknowledge */
#define M_PCPROTO	0x83		/* priority proto message */
#define	M_PCSIG		0x84		/* generate process signal */
#define	M_READ		0x85		/* generate read notification */
#define	M_FLUSH		0x86		/* flush your queues */
#define	M_STOP		0x87		/* stop transmission immediately */
#define	M_START		0x88		/* restart transmission after stop */
#define	M_HANGUP	0x89		/* line disconnect */
#define M_ERROR		0x8a		/* fatal error used to set u.u_error */
#define M_COPYIN	0x8b		/* request to copyin data */
#define M_COPYOUT	0x8c		/* request to copyout data */
#define M_IOCDATA	0x8d		/* response to M_COPYIN and M_COPYOUT */
#define M_PCRSE		0x8e		/* reserved for RSE use only */
#define	M_STOPI		0x8f		/* stop reception immediately */
#define	M_STARTI	0x90		/* restart reception after stop */
#define M_PCEVENT	0x91		/* post an event to an event queue */

/*
 * Queue message class definitions.  
 */
#define QNORM		0x00		/* normal priority messages */
#define QPCTL		0x80		/* high priority cntrl messages */

/*
 *  IOCTL structure - this structure is the format of the M_IOCTL message type.
 */

#ifdef _STYPES

struct iocblk {
	int 	ioc_cmd;		/* ioctl command type */
	o_uid_t	ioc_uid;		/* effective uid of user */
	o_gid_t	ioc_gid;		/* effective gid of user */
	uint	ioc_id;			/* ioctl id */
	uint	ioc_count;		/* count of bytes in data field */
	int	ioc_error;		/* error code */
	int	ioc_rval;		/* return value  */
};

#else /* large definition */

struct iocblk {
	int 	ioc_cmd;		/* ioctl command type */
	cred_t	*ioc_cr;		/* full credentials */
	uint	ioc_id;			/* ioctl id */
	uint	ioc_count;		/* count of bytes in data field */
	int	ioc_error;		/* error code */
	int	ioc_rval;		/* return value  */
	long	ioc_filler[4];		/* reserved for future use */
};

#define ioc_uid ioc_cr->cr_uid
#define ioc_gid ioc_cr->cr_gid

#endif /* _STYPES */

/*
 * structure for the M_COPYIN and M_COPYOUT message types.
 */

#ifdef _STYPES

struct copyreq {
	int	cq_cmd;			/* ioctl command (from ioc_cmd) */
	o_uid_t	cq_uid;			/* effective uid of user */
	o_gid_t	cq_gid;			/* effective gid of user */
	uint	cq_id;			/* ioctl id (from ioc_id) */
	caddr_t	cq_addr;		/* address to copy data to/from */
	uint	cq_size;		/* number of bytes to copy */
	int	cq_flag;		/* see below */
	mblk_t *cq_private;		/* privtate state information */
};

#else /* large defintion */

struct copyreq {
	int	cq_cmd;			/* ioctl command (from ioc_cmd) */
	cred_t	*cq_cr;			/* full credentials */
	uint	cq_id;			/* ioctl id (from ioc_id) */
	caddr_t	cq_addr;		/* address to copy data to/from */
	uint	cq_size;		/* number of bytes to copy */
	int	cq_flag;		/* see below */
	mblk_t *cq_private;		/* privtate state information */
	long	cq_filler[4];		/* reserved for future use */
};

#define cq_uid cq_cr->cr_uid
#define cq_gid cq_cr->cr_gid

#endif /* _STYPES */

/* cq_flag values */

#define STRCANON	0x01		/* b_cont data block contains */
					/* canonical format specifier */
#define RECOPY		0x02		/* perform I_STR copyin again, */
					/* this time using canonical */
					/* format specifier */

/*
 * structure for the M_IOCDATA message type.
 */

#ifdef _STYPES

struct copyresp {
	int	cp_cmd;			/* ioctl command (from ioc_cmd) */
	o_uid_t	cp_uid;			/* effective uid of user */
	o_gid_t	cp_gid;			/* effective gid of user */
	uint	cp_id;			/* ioctl id (from ioc_id) */
	caddr_t	cp_rval;		/* status of request: 0 -> success */
					/*             non-zero -> failure */
	uint	cp_pad1;		/* reserved */
	int	cp_pad2;		/* reserved */
	mblk_t *cp_private;		/* private state information */
};

#else /* large definition */

struct copyresp {
	int	cp_cmd;			/* ioctl command (from ioc_cmd) */
	cred_t	*cp_cr;			/* full credentials */
	uint	cp_id;			/* ioctl id (from ioc_id) */
	caddr_t	cp_rval;		/* status of request: 0 -> success */
					/*             non-zero -> failure */
	uint	cp_pad1;		/* reserved */
	int	cp_pad2;		/* reserved */
	mblk_t *cp_private;		/* private state information */
	long	cp_filler[4];		/* reserved for future use */
};

#define cp_uid cp_cr->cr_uid
#define cp_gid cp_cr->cr_gid

#endif /* _STYPES */

/*
 * Options structure for M_SETOPTS message.  This is sent upstream
 * by a module or driver to set stream head options.
 */

#ifdef _STYPES

struct stroptions {
	short	so_flags;		/* options to set */
	short	so_readopt;		/* read option */
	ushort	so_wroff;		/* write offset */
	short	so_minpsz;		/* minimum read packet size */
	short	so_maxpsz;		/* maximum read packet size */
	ushort	so_hiwat;		/* read queue high water mark */
	ushort	so_lowat;		/* read queue low water mark */
	unsigned char so_band;		/* band for water marks */
};

#else /* large definition */

struct stroptions {
	ulong	so_flags;		/* options to set */
	short	so_readopt;		/* read option */
	ushort	so_wroff;		/* write offset */
	long	so_minpsz;		/* minimum read packet size */
	long	so_maxpsz;		/* maximum read packet size */
	ulong	so_hiwat;		/* read queue high water mark */
	ulong	so_lowat;		/* read queue low water mark */
	unsigned char so_band;		/* band for water marks */
};

#endif /* _STYPES */

/* flags for stream options set message */

#define SO_ALL		0x003f	/* set all old options */
#define SO_READOPT	0x0001	/* set read option */
#define SO_WROFF	0x0002	/* set write offset */
#define SO_MINPSZ	0x0004	/* set min packet size */
#define SO_MAXPSZ	0x0008	/* set max packet size */
#define SO_HIWAT	0x0010	/* set high water mark */
#define SO_LOWAT	0x0020	/* set low water mark */
#define SO_MREADON      0x0040	/* set read notification ON */
#define SO_MREADOFF     0x0080	/* set read notification OFF */
#define SO_NDELON	0x0100	/* old TTY semantics for NDELAY reads/writes */
#define SO_NDELOFF      0x0200	/* STREAMS semantics for NDELAY reads/writes */
#define SO_ISTTY	0x0400	/* the stream is acting as a terminal */
#define SO_ISNTTY	0x0800	/* the stream is not acting as a terminal */
#define SO_TOSTOP	0x1000	/* stop on background writes to this stream */
#define SO_TONSTOP	0x2000	/* do not stop on background writes to stream */
#define SO_BAND		0x4000	/* water marks affect band */
#define SO_DELIM	0x8000	/* messages are delimited */
#ifndef _STYPES
#define SO_NODELIM	0x010000	/* turn off delimiters */
#define SO_STRHOLD    0x020000    /* enable strwrite message coalescing */
#endif /* _STYPES */

/*
 * Structure for M_EVENT and M_PCEVENT messages.  This is sent upstream
 * by a module or driver to have the stream head generate a call to the
 * General Events subsystem.  It is also contained in the first M_DATA
 * block of an M_IOCTL message for the I_STREV and I_UNSTREV ioctls.
 */
struct str_evmsg {
	long		 sv_event;	/* the event (module-specific) */
	vnode_t		*sv_vp;		/* vnode pointer of event queue */
	long		 sv_eid;	/* same as ev_eid */
	long		 sv_evpri;	/* same as ev_pri */
	long		 sv_flags;	/* same as ev_flags */
	uid_t		 sv_uid;	/* user id of posting process */
	pid_t		 sv_pid;	/* process id of posting process */
	hostid_t	 sv_hostid;	/* host id of posting process */
	long		 sv_pad[4];	/* reserved for future use */
};

/*
 * Miscellaneous parameters and flags.
 */

/*
 * New code for two-byte M_ERROR message.
 */
#define NOERROR	((unsigned char)-1)

/*
 * Values for stream flag in open to indicate module open, clone open;
 * return value for failure.
 */
#define MODOPEN 	0x1		/* open as a module */
#define CLONEOPEN	0x2		/* open for clone, pick own minor device */
#define OPENFAIL	-1		/* returned for open failure */

/*
 * Priority definitions for block allocation.
 */
#define BPRI_LO		1
#define BPRI_MED	2
#define BPRI_HI		3

/*
 * Value for packet size that denotes infinity
 */
#define INFPSZ		-1

/*
 * Flags for flushq()
 */
#define FLUSHALL	1	/* flush all messages */
#define FLUSHDATA	0	/* don't flush control messages */

/*
 * Flag for transparent ioctls
 */
#define TRANSPARENT	(unsigned int)(-1)

/*
 * Sleep priorities for stream io
 */
#define	STIPRI	PZERO+3
#define	STOPRI	PZERO+3

/*
 * Stream head default high/low water marks 
 */
#define STRHIGH 5120
#define STRLOW	1024

/*
 * Block allocation parameters
 */
#define MAXIOCBSZ	1024		/* max ioctl data block size */

/*
 * amount of time to hold small messages in strwrite hoping to to
 * able to append more data from a subsequent write.  one tick min.
 */
#define STRSCANP	((10*HZ+999)/1000)	/* 10 ms in ticks */

/*
 * Definitions of Streams macros and function interfaces.
 */

/*
 * Definition of spl function needed to provide critical region protection
 * for streams drivers and modules.
 */
#define splstr() spl6()

/*
 * canenable - check if queue can be enabled by putq().
 */
#define canenable(q)	!((q)->q_flag & QNOENB)

/*
 * Finding related queues
 */
#define	OTHERQ(q)	((q)->q_flag&QREADR? (q)+1: (q)-1)
#define	WR(q)		((q)+1)
#define	RD(q)		((q)-1)
#define SAMESTR(q)	(((q)->q_next) && (((q)->q_flag&QREADR) == ((q)->q_next->q_flag&QREADR)))

/*
 * Put a message of the next queue of the given queue.
 */
#define putnext(q, mp)	((*(q)->q_next->q_qinfo->qi_putp)((q)->q_next, (mp)))

/*
 * Test if data block type is one of the data messages (i.e. not a control
 * message).
 */
#define datamsg(type) ((type) == M_DATA || (type) == M_PROTO || (type) == M_PCPROTO || (type) == M_DELAY)

/*
 * Extract queue class of message block.
 */
#define queclass(bp) (((bp)->b_datap->db_type >= QPCTL) ? QPCTL : QNORM)

/*
 * Align address on next lower word boundary.
 */
#define straln(a)	(caddr_t)((long)(a) & ~(sizeof(int)-1))

/*
 * Find the max size of data block.
 */
#define bpsize(bp) ((unsigned int)(bp->b_datap->db_lim - bp->b_datap->db_base))

/*
 * declarations of common routines
 */
extern mblk_t *allocb();
extern mblk_t *esballoc();
extern int esbbcall();
extern int testb();
extern int bufcall();
extern void freeb();
extern void freemsg();
extern mblk_t *dupb();
extern mblk_t *dupmsg();
extern mblk_t *copyb();
extern mblk_t *copymsg();
extern void linkb();
extern mblk_t *unlinkb();
extern mblk_t *rmvb();
extern int pullupmsg();
extern int adjmsg();
extern int msgdsize();
extern mblk_t *getq();
extern void rmvq();
extern void flushq();
extern void flushband();
extern int canput();
extern int bcanput();
extern int putq();
extern int putbq();
extern int insq();
extern int putctl();
extern int putctl1();
extern queue_t *backq();
extern void qreply();
extern void qenable();
extern int qsize();
extern void noenable();
extern void enableok();
extern ushort getmid();
extern int strqset();
extern int strqget();
extern void unbufcall();

/*
 * shared or externally configured data structures
 */
extern int strmsgsz;			/* maximum stream message size */
extern int strctlsz;			/* maximum size of ctl part of message */
extern int nstrpush;			/* maxmimum number of pushes allowed */
extern struct strstat strst;		/* STREAMS statistics structure */
extern char queueflag;          /* set iff inside queuerun() */

/*
 * Structure for 386 ioctls requiring user context
*/
struct  v86blk {
    struct  proc    *v86_u_procp;
    ulong   v86_u_renv ;
    pid_t   v86_p_pid;
    pid_t   v86_p_ppid;
    struct cred *v86_p_cred;
    struct  v86dat  *v86_p_v86;
};

#endif	/* _SYS_STREAM_H */
