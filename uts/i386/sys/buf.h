/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_BUF_H
#define _SYS_BUF_H

#ident	"@(#)head.sys:sys/buf.h	11.21.4.1"

/*
 *	Each buffer in the pool is usually doubly linked into 2 lists:
 *	the device with which it is currently associated (always)
 *	and also on a list of blocks available for allocation
 *	for other use (usually).
 *	The latter list is kept in last-used order, and the two
 *	lists are doubly linked to make it easy to remove
 *	a buffer from one list when it was found by
 *	looking through the other.
 *	A buffer is on the available list, and is liable
 *	to be reassigned to another disk block, if and only
 *	if it is not marked BUSY.  When a buffer is busy, the
 *	available-list pointers can be used for other purposes.
 *	Most drivers use the forward ptr as a link in their I/O active queue.
 *	A buffer header contains all the information required to perform I/O.
 *	Most of the routines which manipulate these things are in bio.c.
 */
typedef struct	buf {
	uint	b_flags;		/* see defines below */
	struct	buf *b_forw;		/* headed by d_tab of conf.c */
	struct	buf *b_back;		/*  "  */
	struct	buf *av_forw;		/* position on free list, */
	struct	buf *av_back;		/*     if not BUSY*/
	o_dev_t	b_dev;			/* major+minor device name */
	unsigned b_bcount;		/* transfer count */
	union {
	    caddr_t b_addr;		/* low order core address */
	    int	*b_words;		/* words for clearing */
	    daddr_t *b_daddr;		/* disk blocks */
	} b_un;

#define paddr(X)	(paddr_t)(X->b_un.b_addr)

	daddr_t	b_blkno;		/* block # on device */
	char	b_oerror;		/* OLD error field returned after I/O */
	char	b_res;			/* XENIX Compatibility	*/
	ushort	b_cylin;		/* XENIX Compatibility	*/
	unsigned int b_resid;		/* words not transferred after error */
	daddr_t	b_sector;		/* physical sector of disk request */
	clock_t	b_start;		/* request start time */
	struct  proc  *b_proc;		/* process doing physical or swap I/O */
	struct	page  *b_pages;		/* page list for PAGEIO */
	clock_t	b_reltime;      /* previous release time */
	/* Begin new stuff */
#define b_actf	av_forw
#define	b_actl	av_back
#define	b_active b_bcount
#define	b_errcnt b_resid
	long	b_bufsize;		/* size of allocated buffer */
	int	(*b_iodone)();		/* function called by iodone */
	struct	vnode *b_vp;		/* vnode associated with block */
	struct 	buf *b_chain;		/* chain together all buffers here */
	int	b_reqcnt;		/* number of I/O request generated */
	int	b_error;		/* expanded error field */
	dev_t	b_edev;			/* expanded dev field */
	char *b_private;		/* private data structure */
} buf_t;

/*
 * Bufhd structures used at the head of the hashed buffer queues.
 * We only need three words for these, so this abbreviated
 * definition saves some space.
 */
struct bufhd {
	long	b_flags;		/* see defines below */
	struct	buf *b_forw, *b_back;	/* fwd/bkwd pointer in chain */
};
struct diskhd {
	long b_flags;                   /* not used, needed for consistency */
 	struct  buf *b_forw, *b_back;   /* queue of unit queues */
 	struct  buf *av_forw, *av_back; /* queue of bufs for this unit */
 	long    b_bcount;               /* active flag */
};

extern	struct	buf	bfreelist;	/* head of available list */
struct	pfree	{
	int	b_flags;
	struct	buf	*av_forw;
};
extern	struct	pfree	pfreelist;
extern	int		pfreecnt;
extern	struct	buf	pbuf[];
#ifndef BUFDEFINE
extern	char		*buffers[];
#endif

/*
 *	These flags are kept in b_flags.
 */
#define B_WRITE   0x0000	/* non-read pseudo-flag */
#define B_READ    0x0001	/* read when I/O occurs */
#define B_DONE    0x0002	/* transaction finished */
#define B_ERROR   0x0004	/* transaction aborted */
#define B_BUSY    0x0008	/* not on av_forw/back list */
#define B_PHYS    0x0010	/* Physical IO potentially using UNIBUS map */
#define B_MAP     0x0020	/* This block has the UNIBUS map allocated */
#define B_WANTED  0x0040	/* issue wakeup when BUSY goes off */
#define B_AGE     0x0080	/* delayed write for correct aging */
#define B_ASYNC   0x0100	/* don't wait for I/O completion */
#define B_DELWRI  0x0200	/* delayed write - wait until buffer needed */
#define B_OPEN    0x0400	/* open routine called */
#define B_STALE   0x0800
#define B_VERIFY  0x1000
#define B_FORMAT  0x2000
#define B_S52K	  0x8000	/* 2k buffer flag */
#define B_PRIVLG  0xf000	/* privileged operation (internal driver use) */

/* fix these numbers; remove ones we don't need */
#define	B_PAGEIO	0x10000		/* do I/O to pages on bp->p_pages */
#define	B_DONTNEED	0x20000		/* after write, need not be cached */
#define	B_TAPE		0x40000		/* this is a magtape (no bdwrite) */
#define	B_UAREA		0x80000		/* add u-area to a swap operation */
#define	B_REMAPPED	0x100000	/* buffer is kernel addressable */
#define	B_FREE		0x200000	/* free page when done */
#define	B_PGIN		0x400000	/* pagein op, so swap() can count it */
#define	B_CACHE		0x800000	/* did bread find us in the cache ? */
#define	B_INVAL		0x1000000	/* does not contain valid info  */
#define	B_FORCE		0x2000000	/* semi-permanent removal from cache */
#define	B_HEAD		0x4000000	/* a buffer header, not a buffer */
#define	B_NOCACHE	0x8000000	/* don't cache block when released */
#define	B_BAD		0x10000000	/* bad block revectoring in progress */
#define B_KERNBUF	0x20000000	/* buffer is a kernel buffer */
#define	B_DMA_REMAPPED	0x40000000	/* > 16MB DMA remap */
#define	B_RAIO		0x80000000	/* raw disk async I/O */

/*
 * Insq/Remq for the buffer hash lists (used by pageio).
 */
#define	bremhash(bp) { \
	(bp)->b_back->b_forw = (bp)->b_forw; \
	(bp)->b_forw->b_back = (bp)->b_back; \
}
#define	binshash(bp, dp) { \
	(bp)->b_forw = (dp)->b_forw; \
	(bp)->b_back = (dp); \
	(dp)->b_forw->b_back = (bp); \
	(dp)->b_forw = (bp); \
}

/*
 * Insq/Remq for the buffer free lists.
 */
#define	bremfree(bp) { \
	(bp)->av_back->av_forw = (bp)->av_forw; \
	(bp)->av_forw->av_back = (bp)->av_back; \
}
#define	binsheadfree(bp, dp) { \
	(dp)->av_forw->av_back = (bp); \
	(bp)->av_forw = (dp)->av_forw; \
	(dp)->av_forw = (bp); \
	(bp)->av_back = (dp); \
}
#define	binstailfree(bp, dp) { \
	(dp)->av_back->av_forw = (bp); \
	(bp)->av_back = (dp)->av_back; \
	(dp)->av_back = (bp); \
	(bp)->av_forw = (dp); \
}

/*
 *	Fast access to buffers in cache by hashing.
 */

#define bhash(d, b)	((struct buf *)&hbuf[((int)d+(int)b)&v.v_hmask])

struct	hbuf {
	int	b_flags;
	struct	buf	*b_forw;
	struct	buf	*b_back;
	int	b_pad;			/* round size to 2^n */
};

extern	struct	hbuf	hbuf[];

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized code
 */
#define geterror(bp) \
	(((bp)->b_flags & B_ERROR) == 0 ? 0 : \
	(bp)->b_error ? (bp)->b_error : \
	(bp)->b_oerror ? (bp)->b_oerror : EIO)
/*
 * Unlink a buffer from the available list and mark it busy.
 * (internal interface)
 */
#define notavail(bp) \
{\
	register s;\
\
	s = spl6();\
	(bp)->av_back->av_forw = (bp)->av_forw;\
	(bp)->av_forw->av_back = (bp)->av_back;\
	(bp)->b_flags |= B_BUSY;\
	bfreelist.b_bcount--;\
	splx(s);\
}

struct buf	*bread();
struct buf	*breada();
void		bwrite();
void		bdwrite();
void		bawrite();
void		brelse();
int		incore();
struct buf	*getfreeblk();
struct buf	*getblk();
struct buf	*ngeteblk();
struct buf	*geteblk();
int		iowait();
void		iodone();
void		clrbuf();
void		bflush();
void		blkflush();
void		bdflush();
void		bdwait();
void		binval();
void		binit();
int		biowait();
int		get_error();
struct buf	*pageio_setup();
void		pageio_done();
void		biodone();
void		buf_breakup();

#if defined(__STDC__)
extern void dma_breakup(void (*)(), struct buf *);
extern void dma_pageio(void (*)(), struct buf *);
extern void dma_access(u_char, u_int, u_int, u_char, u_char);
#else
extern void dma_breakup();
extern void dma_pageio();
extern void dma_access();
#endif

#ifdef i386
#define 	bimap(bp)	((caddr_t)(paddr(bp)))
#endif

#define bigetl(bp,cp) (*(long *)((paddr(bp))+cp))
#endif	/* _SYS_BUF_H */
