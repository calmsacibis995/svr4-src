/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_RF_COMM_H
#define _SYS_RF_COMM_H

#ident	"@(#)head.sys:sys/rf_comm.h	1.22.3.1"

/* parameters of this implementation */

#define NADDRLEN	6	/* length of network address */

#define RF_UP		1	/* on network */
#define RF_DOWN 	0	/* not on network */
#define RF_INTER 	2	/* RFS in an intermediate state */

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#ifndef NULL
#define NULL	0
#endif

/* is a receive queue empty? */
#define RCVDEMP(RD)	LS_ISEMPTY(&(RD)->rd_rcvdq)

typedef struct naddr	{
	ushort	length;
	char	address[NADDRLEN];
} naddr_t;

/*
 * receive descriptor structure
 */

typedef struct rcvd	{
	union {
		/* vp for RDGENERAL, sdp for RDSPECIFIC */
		struct vnode	*rdun_vp;
		struct sndd	*rdun_sdp;
	} rd_un;
	ls_elt_t 	rd_rcvdq;	/* receive descriptor queue */
	time_t		rd_mtime;	/* last write time - for cache */
	struct rd_user	*rd_user_list;	/* one for each time RD is a gift
					 * - meaningful only for general RD */
	struct rcvd	*rd_next;	/* pointer to next rcvd */
	long		rd_gen;		/* generation for gifts */
	int		rd_qslp;	/* recv desc queue sleep lock */
	char		rd_stat;	/* flags */
	char		rd_qtype;	/* RDGENERAL or RDSPECIFIC */
	ushort		rd_qcnt;	/* how many msgs queued */
	int		rd_refcnt;	/* how many remote clients */
} rcvd_t;

#define rd_vp	rd_un.rdun_vp
#define rd_sdp	rd_un.rdun_sdp

/* rd_stat values */
#define RDUSED		0x1
#define RDUNUSED	0x2
#define RDLINKDOWN	0x4
#define RDWANT		0x10
#define RDLOCKED	0x20

/* rd_qtype */
#define RDGENERAL	0x1
#define RDSPECIFIC	0x2

/* active general and specific RDs */
#define ACTIVE_GRD(rdp) \
 ((rdp)->rd_stat & RDUSED && (rdp)->rd_qtype & RDGENERAL && (rdp)->rd_user_list)
#define ACTIVE_SRD(rdp) \
 ((rdp)->rd_stat & RDUSED && (rdp)->rd_qtype & RDSPECIFIC)

/*
 * given a ptr to an rd, yield ptr to denoted sd
 */
#define RDTOSD(rdp)	((rdp) ? (rdp)->rd_sdp : NULL)
/*
 * given a ptr to an rd, yield ptr to denoted vnode
 */
#define RDTOV(rdp)	((rdp) ? (rdp)->rd_vp : NULLVP)
/*
 * given a ptr to a vnode, yield ptr to denoted rd
 */
#define VTORD(vp)	(vtord(vp))

extern rcvd_t	*rcvd;

/*
 * A send descriptor is the send end of a channel to a
 * remote system and contains a pointer to a virtual circuit that
 * identifies the remote machine.
 *
 * On the client side the send descriptor is kept as the file system
 * specific part (lower level) of the corresponding vnode for a persistent
 * reference, and as a temporary channel for a transient reference.
 *
 * Each server has its own private send descriptor that it uses to
 * send replies.  This send descriptor on the server does not point
 * to any particular resource.
 *
 * For the client side send descriptor, the sd_mntid is a cookie
 * used to access the remote side's resource list so that counts can
 * be kept properly for umount.	In the server side send descriptor
 * this same field is used as a cookie into the local resource list.
 */

typedef struct sndd {
	ls_elt_t 	sd_hash;	/* for hash list links */
	ls_elt_t	sd_free;	/* for free list links */
	long		sd_size;
	char		sd_stat;
	rf_gift_t	sd_gift;	/* for the remote rcvd */
	long 		sd_mntid;	/* denotes remote srmount entry */
	struct proc	*sd_srvproc;	/* points to proc of dedicated server */
	struct queue	*sd_queue;	/* points to stream head queue */
	long		sd_fhandle;	/* file handle for client caching */
	ulong		sd_vcode;	/* file vcode for client caching */
	daddr_t		sd_nextr;	/* read-ahead offset */
	/*
	 * Clients talking to 3.x servers sometimes hide stuff in sd_stashp.
	 */
	struct dustash	*sd_stashp;
	/*
	 * Clients talking to 4.0++ servers remember their contribution to
	 * the server's v_count in sd_remcnt.
	 */
	long		sd_remcnt;
	struct {
		int	want : 1;
		int	writer : 1;
		uint	nreaders;
	} sd_crwlock;			/* n readers, 1 writer in cache */
	struct vnode	sd_vn;		/* upper level node denoting sd */
} sndd_t;

/*
 * Values for sd_stat.  These must NOT be overloaded.
 */
#define SDUNUSED	0
#define SDUSED		0x1
#define SDLOCKED	0x2
#define SDLINKDOWN	0x4
#define SDSERVE		0x8
#define SDWANT		0x10
#define SDCACHE		0x20	/* remote file is cacheable */
#define SDMNDLCK	0x40	/* remote file mandatory lock set
				 * Old protocol only.
				 * (not updated if someone turns off mandatory
				 * lock with chmod on remote file before
				 * last file close)
				 */
#define SDINTER		0x80

/*
 * given a ptr to an sd, yield ptr to denoted vnode
 */
#define SDTOV(sdp)	(&(sdp)->sd_vn)
/*
 * given a ptr to a vnode, yield ptr to denoted sd
 */
#define VTOSD(vp)	((struct sndd *)(vp)->v_data)
/*
 * yield the sd containing the referenced sd_hash or sd_free ls_elt_t
 */
#define HASHTOSD(hp)	\
	((sndd_t *)(((char *)(hp)) - (char *)&((sndd_t *)0)->sd_hash))
#define FREETOSD(fp)	\
	((sndd_t *)(((char *)(fp)) - (char *)&((sndd_t *)0)->sd_free))
/*
 * Lock and unlock send descriptors.
 */
#define SDLOCK(sdp) {				\
	while ((sdp)->sd_stat & SDLOCKED) {	\
	    (sdp)->sd_stat |= SDWANT;		\
	    (void) sleep((caddr_t)(sdp), PSNDD);\
	}					\
	(sdp)->sd_stat |= SDLOCKED;		\
}

#define SDUNLOCK(sdp) {				\
	ASSERT((sdp)->sd_stat & SDLOCKED);	\
	(sdp)->sd_stat &= ~SDLOCKED;		\
	if ((sdp)->sd_stat & SDWANT) {		\
	    (sdp)->sd_stat &= ~SDWANT;		\
	    wakeprocs((caddr_t)(sdp), PRMPT);	\
	}					\
}

extern sndd_t	*sndd;
extern void	sndd_hash();
extern void	sndd_unhash();

/*
 * Every time a client creates a new reference to a file the
 * server creates a reference to an rduser structure.	This
 * server action can take two forms :
 *	1) create a new rduser structure;
 *	2) increment the ru_vcount.
 * At the same time, the rd_refcnt is incremented.
 * Each counted reference to an rduser structure contributes
 * 1 to the v_count on that file on the server.
 */

typedef struct rd_user {
	long		ru_srmntid;	/* which srmount entry */
	struct rd_user	*ru_next;	/* next user */
	struct queue	*ru_queue;	/* which stream queue */
	ushort		ru_vcount;	/* references to denoted vnode */
	ushort		ru_gen;		/* generation number */
	ushort		ru_frwcnt;	/* n r/w opens */
	ushort		ru_frcnt;	/* n read opens */
	ushort		ru_fwcnt;	/* n write opens */
	ushort		ru_flag;	/* flag (see below for values) */
	ls_elt_t	ru_mapdlist;	/* heads list of mapping descriptors */
} rd_user_t;

/* ru_flag values */
#define RU_CACHE_ON		0x1	/* remote cacheing is enabled */
#define RU_CACHE_DISABLE	0x2	/* remote cacheing is disabled
					 * as opposed to being off entirely
					 * which is assumed when ru_flag
					 * is 0.  This is only necessary
					 * for old clients who only expect
					 * vcode and fhandle with gifts.
					 */
#define RU_MAPDLOCK		0x4	/* ru_mapdlist is locked */
#define RU_MAPDWANT		0x8	/* ru_mapdlist lock is wanted */

#define RU_R_CREAT		0x10	/* This client successfully created
					 * this file for reading.
					 */
#define RU_W_CREAT		0x20	/* This client successfully created
					 * this file for writing.
					 */
#define RU_DONE			0x40	/* When walking rd_user_list,
					 * already processed
					 */

/*
 * Each rd_user_t has a list of rf_mapd_t's describing a client machine's
 * mappings to a file.  rfm_prots counts the number of times that each
 * combination of the PROT_READ, PROT_WRITE, and PROT_EXEC bits
 * are used in the maxprot of mappings to an offset/length pair.
 *
 * NOTE: rfm_prots[0] is present as padding for coding convenience. 
 * maxprot == 0 is disallowed by the vm/vfs interface.
 */
typedef struct rf_mapd {
	ls_elt_t	rfm_list;	/* heads list of mapping descriptors */
	off_t		rfm_off;	/* file offset of mapping */
	size_t		rfm_len;	/* length of mapping */
	uint		rfm_prots[8];	/* counts of R, W, RW, X, RX, WX, RWX */
} rf_mapd_t;

extern rd_user_t	*rd_user;

/* tunables */
extern int	nrcvd;		/* number of receive descriptors */
extern int	nsndd;		/* number of send descriptors */
extern int	nrduser;	/* number of rd_user entries */
extern int	maxserve;
extern int	minserve;

/*
 * rf_maxkmem limits persistently allocated kernel memory used by rfs, including
 * idmapping info, authorization lists and mapping descriptors.  A zero value
 * means no limit.  rf_availkmem is the amount of the maximum remaining.
 */
extern int	rf_maxkmem;
extern int	rf_availkmem;

#ifdef _KERNEL

extern void	rf_freeb();
extern void	rf_freemsg();

extern mblk_t	*rf_dequeue();

extern int	sndd_create();
extern void	sndd_free();

#if defined(DEBUG)
#define sndd_set(sd, queue, giftp)				\
	((void)(ASSERT((sd)->sd_stat & SDUSED),			\
	  (sd)->sd_queue = (queue_t *)(queue), (sd)->sd_gift = (giftp)[0]))
#else
#define sndd_set(sd, queue, giftp)			\
	((void)((sd)->sd_queue = (queue_t *)(queue),	\
		(sd)->sd_gift = (giftp)[0]))
#endif /* DEBUG */

extern int	rf_sndmsg();
extern		rf_rcvmsg();

extern int	rf_allocmsg();
extern int	rf_allocb();

extern int	rcvd_create();
extern void	rcvd_delete();
extern void	rcvd_free();
extern rcvd_t	*vtord();

extern mblk_t	*rf_dequeue();
extern int	rf_comminit();
extern void	rf_commdinit();
extern void	rf_deliver();

#ifdef DEBUG
extern int	rdu_match();
#else
#define rdu_match(rdup, sid, mntid) \
  (QPTOGP((rdup)->ru_queue)->sysid == (sid) && (rdup)->ru_srmntid == (mntid))
#endif

extern int	rfesb_fbread();
extern int	rfesb_pageio_setup();

#define NULLFRP		((frtn_t *)NULL)
#define NULLCADDR	((caddr_t)NULL)
#define NULLUIO		((uio_t *)NULL)
#define NULLCRED	((cred_t *)NULL)

/*
 * Signal conversion ops.
 *	rf_response_t	*rp;
 *	int		vcver;	-- virtual circuit version
 *	k_sigset_t	*kp;
 */
#define rf_sigisempty(rp, vcver)	\
	((vcver) == RFS1DOT0 ?	\
	  (rp)->rp_v1sig == 0 : (rp)->rp_v2sigset.word[0] == 0)

#define rf_setrpsigs(rp, vcver, kp)	\
	((void)((vcver) == RFS1DOT0 ?	\
	  ((rp)->rp_v1sig = (kp)[0]) : ((rp)->rp_v2sigset.word[0] = (kp)[0])))

#define rf_getrpsigs(rp, vcver, kp)	\
	((void)((vcver) == RFS1DOT0 ?	\
	  ((kp)[0] = (rp)->rp_v1sig) : ((kp)[0] = (rp)->rp_v2sigset.word[0])))

#define rf_clrrpsigs(rp, vcver)	\
	((void)((vcver) == RFS1DOT0 ?	\
	  ((rp)->rp_v1sig = 0) : ((rp)->rp_v2sigset.word[0] = 0)))

extern void	rf_postrpsigs();
extern void	rf_delsig();

/*
 * Operations on rd_user structures.
 */
extern rd_user_t	*rdu_get();
extern rd_user_t	*rdu_find();
extern void		rdu_del();
extern void		rdu_open();
extern int		rdu_modecheck();

extern void		rftov_attr();
extern void		vtorf_attr();
extern int		rf_pullupmsg();
extern void		rf_iov_alloc();
extern mblk_t		*rf_dropbytes();
extern caddr_t		rf_msgdata();

extern rcvd_t	*rf_gifttord();
extern vnode_t	*rf_gifttovp();

/* Gift templates. */
extern rf_gift_t	rf_daemon_gift;
extern rf_gift_t	rf_null_gift;

#define RF_IOV_FREE(iovp, niov)	\
	kmem_free((caddr_t)(iovp), (size_t)((niov) * sizeof(iovec_t)))

/* fast for simple cases */
#ifdef DEBUG
#define RF_PULLUP(bp, hdrsz, datasz) \
	(RF_MSG(bp)->m_size == (hdrsz) + (datasz) && !(bp)->b_cont ? \
	  (ASSERT((bp)->b_wptr - (bp)->b_rptr == RF_MSG(bp)->m_size), 0) : \
	  rf_pullupmsg((bp), (hdrsz), (datasz)))
#else
#define RF_PULLUP(bp, hdrsz, datasz) \
	(RF_MSG(bp)->m_size == (hdrsz) + (datasz) && !(bp)->b_cont ? 0 : \
	  rf_pullupmsg((bp), (hdrsz), (datasz)))
#endif

#if defined(u3b2) && !defined(lint) && !defined(CXREF)
asm	int
ipl()
{
	ANDW3 &0x1e000,%psw,%r0
}
#endif

#endif /* _KERNEL */
#endif /* _SYS_RF_COMM_H */
