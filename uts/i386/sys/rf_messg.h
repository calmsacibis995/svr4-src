/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_RF_MESSG_H
#define _SYS_RF_MESSG_H

#ident	"@(#)head.sys:sys/rf_messg.h	1.25.3.1"

/*
 * RFS network message definitions
 *
 * TO DO:  make message formats regular!
 */

#define RF_MAXSNAME	20	/* machine name size in mnt_data */

/*
 * maximum # of group id's per user, can only be increased with a new
 * version of rf_request_t.
 */
#define RF_MAXGROUPS	32

/*
 * A gift denotes a reference to a remote receive descriptor.  If the
 * gift denotes a GENERAL receive descriptor, the receive descriptor
 * is on a server, is persistent, and in turn denotes a file
 * on the server.  Otherwise the receive descriptor is SPECIFIC,
 * transient, and is merely one end of a communications channel.
 * gift_gen tags instances of the denoted receive descriptor, is
 * undefined for old clients/servers.
 */
typedef struct rf_gift {
	long	gift_id;
	long	gift_gen;
} rf_gift_t;

#ifdef _KERNEL

#define DU_DATASIZE	1024	/* max data bytes in old protocol message */

/*
 * RFS file attributes.
 */
typedef struct rf_attr {
	long		rfa_mask;	/* bit-mask of attributes */
	long		rfa_type;	/* vnode type (for create) */
	long		rfa_mode;	/* file access mode */
	long		rfa_uid;	/* owner user id */
	long		rfa_gid;	/* owner group id */
	long		rfa_fsid;	/* file system id (dev for now) */
	long		rfa_nodeid;	/* node id */
	long		rfa_nlink;	/* number of references to file */
	long		rfa_size;	/* file size in bytes */
	timestruc_t	rfa_atime;	/* time of last access */
	timestruc_t	rfa_mtime;	/* time of last modification */
	timestruc_t	rfa_ctime;	/* time file ``created'' */
	long		rfa_rdev;	/* device the file represents */
	long		rfa_blksize;	/* fundamental block size */
	long		rfa_nblocks;	/* # of blocks allocated */
	long		rfa_filler[8];
} rf_attr_t;

/*
 * RFS signal set.  Two longs are PLENTY until the next major protocol rev.
 */
typedef struct rf_sigset {
	long	word[2];
} rf_sigset_t;

/*
 * The message structure is the header to every message.
 */
typedef struct rf_message {
	long		filler0;	/* used as sequence w/ DEBUG */
	long	 	m_stat;		/* see stat values below */
	rf_gift_t	m_dest;		/* rcvd to which this goes */
	rf_gift_t	m_gift;		/* rcvd of reference we are giving */
	long 		m_size;		/* size of this message	*/
	long		m_queue;	/* streams queue message came in on */
} rf_message_t;

/*
 * Point to rf_message_t in streams data block;
 * argument is pointer to streams message block.
 */
#define RF_MSG(p)	((rf_message_t *)(p)->b_rptr)

/* m_stat values */
#define RF_GIFT		0x8		/* are address & index real? */
#define RF_SIGNAL	0x10		/* set for signal messages */
#define RF_VER1		0x20		/* set for NACKABLE messages */


/*
 * Well-known receive queue indices.
 * SIGRDX used to be 1, but that symbol was unused.  DAEMON_RD
 * remains 2 for compatibility.
 */
#define MOUNT_RD	0L	/* Mount request receive descriptor */
#define DAEMON_RD	2L	/* Recovery receive descriptor */

/*
 * In the original remote system call protocol, opcodes for remote service
 * frequently correspond to system call numbers.
 * The "DU" prefix is retained for operations that are always based on
 * system calls, or are specific to the system call protocol.
 *
 * Operations shared between the system call and operation protocol, or
 * specific to the latter, use the "RF" prefix, even though they may
 * still use system call number for compatability.
 *
 * For ops protocol that does not look at u.u_syscall (except for wretched
 * hook in rf_lookup preventing mounts on remote names), the sysent table
 * is of no concern, and opcodes can be chosen arbitrarily.
 */
#define RFSETFL		0
#define RFDELMAP	1
#define RFADDMAP	2
#define RFREAD		3
#define RFWRITE		4
#define RFOPEN		5
#define RFCLOSE		6
#define RFLOOKUP	7
#define RFCREATE	8
#define DULINK		9
#define DUUNLINK	10
#define DUEXEC		11
#define DUCHDIR		12
#define RFPUTPAGE	13
#define DUMKNOD		14
#define DUCHMOD		15
#define DUCHOWN		16
#define RFGETPAGE	17
#define DUSTAT		18
#define DUSEEK		19
#define RFGETATTR	20	/* for VOP_GETATTR */
#define DUMOUNT		21	/* retained for compatability;
				 * mount(2), not old rmount(2)
				 * must '..' back to client */
#define DUUMOUNT	22	/* retained for compatability;
				 * umount(2), not old rumount(2)
				 * must '..' back to client */
#define RFSETATTR	23	/* VOP_SETATTR */
#define RFACCESS	24	/* VOP_ACCESS */
#define RFPATHCONF	25	/* VOP_PATHCONF */
#define RFTMO		26	/* virtual circuit timeout */

#define DUFSTAT		28
#define DUUTIME		30
#define DUSACCESS	33	/* access system call */
#define DUSTATFS	35
#define DUFSTATFS	38
#define RFRENAME	40
#define DUSYSACCT	51	/* unused in protocol, but needed to recognize
				 * the sysacct() system call */
#define RFIOCTL		54
#define RFUSTAT  	57
#define RFFSYNC		58
#define DUEXECE		59
#define DUCHROOT	61
#define RFFCNTL		62
#define RFSPACE		63
#define RFFRLOCK	64
#define DURMOUNT	72	/* lookup on remote tree for mount;
				 * retained for compatability */
#define RFRMDIR		79
#define RFMKDIR		80
#define RFREADDIR	81
#define DULSTAT		88	/* unused in protocol, but needed to recognize
				 * the lstat() system call */
#define RFSYMLINK	89
#define RFREADLINK	90
#define RFMOUNT		97	/* differs from sys call for compatability;
				 * 3.X turned this into another op */
#define RFUMOUNT	98	/* differs from sys call for compatability;
				 * 3.X turned this into another op */
#define DUSTATVFS	103	/* name-based */
#define RFSTATVFS	104	/* vnode-based */
#define RFCOPYIN	106
#define RFCOPYOUT	107
#define RFLINK		109
#define DUCOREDUMP	111
#define DUWRITEI	112
#define DUREADI		113
#define RFRSIGNAL	119
#define RFSYNCTIME	122	/* date synchronization */
#define DUXSTAT		123	/* unused in protocol, but needed to recognize
				 * the xstat system call */
#define DULXSTAT	124	/* unused in protocol, but needed to recognize
				 * the lxstat system call */
#define RFDOTDOT	124	/* server sends this back to client when
				 * remote path crosses back to client */
#define RFPATHREVAL	125	/* path to be re-evaluated by client, for
				 * symlinks */
#define DUXMKNOD	126	/* unused in protocol, but needed to recognize
				 * the xmknod system call */
#define RFFUMOUNT	126	/* forced unmount */
#define RFSENDUMSG	127	/* send message to remote user-level */
#define RFGETUMSG	128	/* get message from remote user-level */
#define RFREMOVE	129
#define DULCHOWN	130	/* unused in protocol, but needed to recognize
				 * the lchown() system call */
#define RFINACTIVE	131
#define DUIUPDATE	132
#define DUUPDATE	133
/*
 * NOTE:  This overloading works ONLY because the two messages
 * have disjoint paths through RFS.  It is inevitable that something
 * like this would happen by virtue of the 3.X implementation using
 * existing syscall numbers and preempting future ones.
 */
#define DURENAME	134
#define RFCACHEDIS	134	/* disable cache */

/*
 * Precedes and discriminates response and request structures and
 * their variants.
 */
typedef struct rf_common {
	long	co_opcode;	/* what to do */
	long	co_sysid;	/* where we came from */
	long	co_type;	/* message type - request/response */
	long	co_pid;		/* client pid */
	long	co_uid;		/* client uid */
	long	co_gid;		/* client gid */
	long	co_ftype;
	union {
		long	nlink;	/* RFS1DOT0 only */
		long	svr4pad;
	} co_un;
	long	co_size;
	long	co_mntid;
} rf_common_t;

#define co_nlink co_un.nlink

/*
 * co_type values are tags for variant message types
 */
#define RF_REQ_MSG	1	/* request message */
#define RF_RESP_MSG	2	/* response message */
#define RF_NACK_MSG	3	/* RFS flow control nack message */

#define RF_COM(bp)	((rf_common_t *)(RF_MSG(bp) + 1))

#define RF_MCSZ		(sizeof(rf_message_t) + sizeof(rf_common_t))

/* Arg structures for RFS request opcodes.
 *
 * The svr3pad elements are to preserve alignment assumptions in
 * original protocol.  Do not depend on their continued existence.
 */
/* RFACCESS, DUCHMOD */
struct rqmode_op {
	long	svr3pad0;
	long	svr3pad1;
	long	svr3pad2;
	long	fmode;
};

struct rqchown {
	long	uid;
	long	gid;
};

/*
 * rqrele is used for RFS2DOT0 RFINACTIVE and RFUMOUNT requests
 */
struct rqrele {
	long	vcount;		/* unused in SVR3 */
};

/* unused in SVR3 */
struct rqpathconf {
	long	cmd;
};

#define RFPROT_TO_PROT(x) ((((x) & 0x1) ? PROT_READ : 0) |\
			   (((x) & 0x2) ? PROT_WRITE : 0) |\
			   (((x) & 0x4) ? PROT_EXEC : 0))

#define PROT_TO_RFPROT(x) ((((x) & PROT_READ) ? 0x1 : 0) |\
			   (((x) & PROT_WRITE) ? 0x2 : 0) |\
			   (((x) & PROT_EXEC) ? 0x4 : 0))
/*
 * Used by RFADDMAP, and RFDELMAP.  The server is able to record
 * current mapping because addmaps and delmaps also go remote.  Only maxprot
 * is sent in all cases because prot may be manipulated above the file
 * system interface.
 */
struct rqmap {
	long	offset;
	ulong	len;
	ulong	maxprot;
};

struct rqclose {
	long	vcount;		/* used only when lastclose */
	long	count;
	long	foffset;
	long	fmode;
	long	lastclose;	/* unused in SVR3 */
};

struct rqcreate {
	long	svr3pad0;
	long	ex;	/* unused in SVR3 */
	long	cmask;	/* unused in SVR4 */
	long	fmode;
};

struct rqfcntl {
	long	cmd;
	long	fcntl;
	long	offset;
	long	fflag;
	long	prewrite;
};
#define DUFRPREWRITE  0x20     /* Compatability:  redundant prewrite indicator
				* for file locks */

struct rqioctl {
	long	cmd;
	long	arg;
	long	svr3pad2;
	long	fflag;
};

struct rqmknod {
	long	svr3pad0;
	long	dev;
	long	cmask;
	long	fmode;
};

struct rqopen {
	long	svr3pad0;
	long	crtmode;
	long	cmask;
	long	fmode;
};

struct rqseek {
	long	svr3pad0;
	long	whence;
};

/* DUSTAT, DUFSTAT */
struct rqstat_op {
	long	buf;
};

/* DUSTATFS, DUFSTATFS */
struct rqstatfs_op {
	long	buf;
	long	len;
	long	fstyp;
};

struct rqutime {
	long	buf;
};

struct rqustat {
	long	buf;
	long	dev;
};

/* RFREAD(I), RFREADDIR, RFWRITE(I), RFGETPAGE
 */
struct rqxfer {
	long	offset;
	long	count;
	long	base;
	long	fmode;
	long	prewrite;		/* only used for RFWRITE */
};

struct rqmkdir {
	long	svr3pad0;
	long	svr3pad1;
	long	cmask;
	long	fmode;
};

struct rqsrmount {
	long	mntflag;
	long	svr3pad1;
	long	synctime;
};

struct rqslink {
	long 	tflag;
	long	targetln;
};

struct rqlink {
	rf_gift_t	from;	/* gen meaningful only SVR4 and later */
};

/*
 * Client pathname should be in "normal" form before it is sent with
 * a RFLOOKUP request.  Otherwise, path displacement with ".." or
 * ENOENT will be unexpected.
 */
struct rqlookup {
	long	flags;
};

/*
 * RFRENAME request.  We assume two canononized component names will
 * be able to fit in the data portion of the request.  If not the
 * call will fail gracefully.
 */
struct rqrename {
	rf_gift_t	from;
	rf_gift_t	to;
};

struct rqcoredump {
	long	svr3pad0;
	long	svr3pad1;
	long	cmask;
};

struct rqsynctime {
	long	svr3pad0;
	long	svr3pad1;
	long	time;
};

struct rqcachedis {
	long	fhandle;
	long	vcode;		/* SVR4 and later */
};

struct rqgetattr {
	long	mask;
	long	flags;
};

struct rqsetattr {
	long	flags;
};

/* SVR4 and later */
struct rqrmdir {
	rf_gift_t	dir;
};

/* RECOVERY OPCODES
 *
 * REC_FUMOUNT
 */
struct rqrec_fumount {
	long	srmntid;
};

/* REC_MSG */
struct rqrec_msg {
	long	svr3pad0;
	long	count;
};

union rq_arg {
	struct rqmode_op	rqmode_op;
	struct rqchown		rqchown;
	struct rqrele		rqrele;
	struct rqpathconf	rqpathconf;
	struct rqmap		rqmap;
	struct rqclose		rqclose;
	struct rqcreate		rqcreate;
	struct rqopen		rqopen;
	struct rqmknod		rqmknod;
	struct rqcoredump	rqcoredump;
	struct rqmkdir		rqmkdir;
	struct rqfcntl		rqfcntl;
	struct rqioctl		rqioctl;
	struct rqseek		rqseek;
	struct rqslink		rqslink;
	struct rqstat_op	rqstat_op;
	struct rqstatfs_op	rqstatfs_op;
	struct rqutime		rqutime;
	struct rqustat		rqustat;
	struct rqxfer		rqxfer;
	struct rqsrmount	rqsrmount;
	struct rqlink		rqlink;
	struct rqlookup		rqlookup;
	struct rqrename		rqrename;
	struct rqsynctime	rqsynctime;
	struct rqcachedis	rqcachedis;
	struct rqrec_fumount	rqrec_fumount;
	struct rqrec_msg	rqrec_msg;
	struct rqgetattr	rqgetattr;
	struct rqsetattr	rqsetattr;
	struct rqrmdir		rqrmdir;
};

/*
 * Header extenstion present only in messages later than version 1.
 * v2_ngroups and v2_groups extend the request message header and
 * the version 2 data portion is at a different offset from the old.
 */
struct rqv2 {
 	long	rqv2_ngroups;
 	long	rqv2_groups[RF_MAXGROUPS];
	long	rqv2_rrdir_gen;
	long	pad[2];		/* with RF_MAXGROUPS at 32, takes the
				 * message + request less data to 256 bytes */
};

/*
 * Op-specific structures that don't fit in the header go in
 * the data portion of the request.
 */

/*
 * VOP_SYMLINK, VOP_CREATE, and VOP_MKDIR create a new directory entry
 */
struct rqmkdent {
	rf_attr_t	attr;
	char		nm[MAXNAMELEN];
};

/*
 * for RFSYMLINK op; target is present if it can fit into the request.
 * Otherwise rqsymlink.target is undefined and the server will copy it in.
 */
struct rqsymlink {
	struct rqmkdent	rqmkdent;	/* link name and attributes */
	char		target[1];	/* begins target name string */
};

/*
 * SVR3 keeps write limit in blocks, SVR4 in bytes.  We continue
 * the message protocol convention of communicating blocks.
 */
#define ULIMSHIFT 9
#define R_ULIMIT		(u.u_rlimit[RLIMIT_FSIZE].rlim_cur)

typedef struct rf_request {
	long			rq_rrdir_id;
	daddr_t			rq_ulimit;
	union rq_arg		rq_arg;
	long			rq_flags;
	long			rq_sec;
	long			rq_nsec;
 	struct rqv2		rqv2;
} rf_request_t;

#define rq_mode_op	rq_arg.rqmode_op
#define rq_chown	rq_arg.rqchown
#define rq_rele		rq_arg.rqrele
#define rq_pathconf	rq_arg.rqpathconf
#define rq_map		rq_arg.rqmap
#define rq_close	rq_arg.rqclose
#define rq_create	rq_arg.rqcreate
#define rq_open		rq_arg.rqopen
#define rq_mknod	rq_arg.rqmknod
#define rq_coredump	rq_arg.rqcoredump
#define rq_mkdir	rq_arg.rqmkdir
#define rq_fcntl	rq_arg.rqfcntl
#define rq_ioctl	rq_arg.rqioctl
#define rq_seek		rq_arg.rqseek
#define rq_slink	rq_arg.rqslink
#define rq_stat_op	rq_arg.rqstat_op
#define rq_statfs_op	rq_arg.rqstatfs_op
#define rq_utime	rq_arg.rqutime
#define rq_ustat	rq_arg.rqustat
#define rq_xfer		rq_arg.rqxfer
#define rq_srmount	rq_arg.rqsrmount
#define rq_link		rq_arg.rqlink
#define rq_lookup	rq_arg.rqlookup
#define rq_rename	rq_arg.rqrename
#define rq_synctime	rq_arg.rqsynctime
#define rq_cachedis	rq_arg.rqcachedis
#define rq_rec_fumount	rq_arg.rqrec_fumount
#define rq_rec_msg	rq_arg.rqrec_msg
#define rq_getattr	rq_arg.rqgetattr
#define rq_setattr	rq_arg.rqsetattr
#define rq_rmdir	rq_arg.rqrmdir
#define rq_ngroups	rqv2.rqv2_ngroups
#define rq_groups	rqv2.rqv2_groups
#define rq_rrdir_gen	rqv2.rqv2_rrdir_gen

#define RF_REQ(bp)	((rf_request_t *)(RF_COM(bp) + 1))

/*
 * The size of a request message depends on the version of the circuit
 * over which it is sent.
 */
#define RFV1_MINREQ	(sizeof(rf_request_t) + RF_MCSZ - sizeof(struct rqv2))
#define RFV2_MINREQ	(sizeof(rf_request_t) + RF_MCSZ)
#define RF_MIN_REQ(version) (((version) == 1) ? RFV1_MINREQ : RFV2_MINREQ)

/* request flags (rq_flags) */
#define RQ_MNDLCK	0x1	/* used only by old protocol, suppresses
				 * inode locking on remote op and caching
				 * of mand lock-enabled files */

/* Arg structures for RFS response opcodes.
 *
 * The svr3pad elements are to preserve alignment assumptions in
 * original protocol.  Do not depend on their continued existence.
 */

/* These members are widely used, so we assume for now that all responses
 * have them.
 */
struct rp_all {
	union {
		long	v1sig;
		long	v2pad0;
	} all_u0;
	long 	nodata;	/* was subyte */
	union {
		/*
		 * This union is used only be RFWRITE(I) and RFFCNTL.
		 * The version code ('vcode') is used for client caching.
 		 * The 'offset' and 'cache' are overloaded by the server
		 * in case of opcode RFREADDIR.
		 */
		long	vcode;
		long	offset;
		long	cache;
	} all_u1;
	long	fhandle;	/* file handle for client caching */
};

/*
 * For old protocol ops that return new file references.
 */
struct rpv1giftinfo {
	long	svr3pad0;
	long	mode;
};

/* pathlen set by lookup, not by flakey /proc ioctl that opens a file */
struct rpv2giftinfo {
	long	pathlen;
	long	flags;
};
#define RPG_NOMAP	0x01
#define RPG_NOSWAP	0x02

/* RFREAD(I), RFWRITE(I) - used only by SVR4 server talking to old client */
struct rprdwr {
	long	isize;
	long	svr3pad1;
};

/* RFCOPYIN, RFREADDIR, DUFSTAT, DUFSTATFS, DUSTAT, DUSTATFS, RFUSTAT
 */
struct rpxfer {
	long	eof;
	long	buf;
};

struct rpcopyout {
	long	copysync;	/* vestige of static buffer allocation */
	long	buf;
};

struct rpfcntl {
	long	isize;	/* used only by SVR4 server talking to old client */
	long	buf;
};

struct rpsynctime {
	long	svr3pad0;
	long	time;
};

/*
 * Header extension present only in post-version 1 messages.
 */
struct rpv2 {
	rf_sigset_t	rpv2_sigset;
	long		pad[2];	/* takes the messsage + response less data
				 * to 128 bytes */
};

union rp_arg {
	struct rpv1giftinfo	rpv1giftinfo;
	struct rpv2giftinfo	rpv2giftinfo;
	struct rprdwr		rprdwr;
	struct rpxfer		rpxfer;
	struct rpcopyout	rpcopyout;
	struct rpfcntl		rpfcntl;
	struct rpsynctime	rpsynctime;
};

typedef struct rf_response {
	long			rp_errno;
	long			rp_v2vcode;
	long			rp_rval;	/* return value	 */
	long			rp_count;	/* not reliably set in 3.x */
	union rp_arg		rp_arg;
	struct rp_all		rp_all;
	struct rpv2		rpv2;
} rf_response_t;

#define RP_MNDLCK	0x8000	/* used only by old protocol, suppresses
				 * inode locking on remote op and caching
				 * of mand lock-enabled files */
#define RP_CACHE_ON	0x1	/* This file's data may be cached.  When
				 * this is set, rpv2_vcode is also set.
				 */
#define DU_CACHE_ENABLE 0x2     /* remote cacheing is enabled by 3.X server */


#define rp_v1giftinfo	rp_arg.rpv1giftinfo
#define rp_v2giftinfo	rp_arg.rpv2giftinfo
#define rp_rdwr		rp_arg.rprdwr
#define rp_xfer		rp_arg.rpxfer
#define rp_copyout	rp_arg.rpcopyout
#define rp_fcntl	rp_arg.rpfcntl
#define rp_synctime	rp_arg.rpsynctime
#define rp_v1sig	rp_all.all_u0.v1sig
#define rp_v2sigset	rpv2.rpv2_sigset
#define rp_nodata	rp_all.nodata
#define rp_vcode	rp_all.all_u1.vcode
#define rp_offset	rp_all.all_u1.offset
#define rp_cache	rp_all.all_u1.cache
#define rp_fhandle	rp_all.fhandle

#define RF_RESP(bp)	((rf_response_t *)(RF_COM(bp) + 1))

/*
 * RFLOOKUP responses ordinarily contain results of 3 VOP_ACCESS()'s and
 * a VOP_GETATTR().  These are briefly cached by the client in order to avoid
 * sending some messages.
 */
typedef struct rflkc_info {
	int		rflkc_read_err;
	int		rflkc_write_err;
	int		rflkc_exec_err;
	rf_attr_t	rflkc_attr;
} rflkc_info_t;

/*
 * The size of a request message depends on the version of the circuit
 * over which it is sent.
 */
#define RFV1_MINRESP	(sizeof(rf_response_t) + RF_MCSZ - sizeof(struct rpv2))
#define RFV2_MINRESP	(sizeof(rf_response_t) + RF_MCSZ)
#define RF_MIN_RESP(version) (((version) == 1) ? RFV1_MINRESP : RFV2_MINRESP)

#define RF_MAXHEAD(version) \
  (RF_MIN_RESP(version) >= RF_MIN_REQ(version) ? \
    RF_MIN_RESP(version) : RF_MIN_REQ(version))


#endif /* _KERNEL */
#endif /* _SYS_RF_MESSG_H */
