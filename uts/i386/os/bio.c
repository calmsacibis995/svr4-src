/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:bio.c	1.3.2.8"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/conf.h"
#include "sys/tss.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/disp.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/debug.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/open.h"
#include "sys/iobuf.h"
#include "sys/conf.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/cmn_err.h"
#include "sys/inline.h"
#include "sys/kmem.h"
#include "vm/page.h"

/*
 * Convert logical block number to a physical number
 * given block number and block size of the file system.
 * Assumes 512 byte blocks (see param.h).
 */
#define LTOPBLK(blkno, bsize)	(blkno * ((bsize>>SCTRSHFT)))

/* count and flag for outstanding async writes */
int basyncnt, basynwait;

struct buf bhdrlist;	/* free buf header list */
int nbuf;		/* number of buffer headers allocated */
struct buf pgouthdrlist;		/* pageout buffer header list */
struct buf notpgouthdrlist;		/* sched buffer header list */
int notpgoutwanted;
extern struct buf pgoutbuf[];		/* global storage for buffer headers */
extern struct buf notpgoutbuf[];
struct buf *pgoutblast;
struct buf *notpgoutblast;
extern int npgoutbuf;			/* number of buffers configured */
extern int nnotpgoutbuf;
int pgoutboutcnt;			/* buffer out counts */
int notpgoutboutcnt;
int pgoutbufused;			/* buffer used counts */
int notpgoutbufused;

void	printbuf();

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no one else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, the process waits until it becomes free.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 *	breada
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	bawrite
 *	brelse
 */

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
struct buf *
bread(dev, blkno, bsize)
	register dev_t dev;
	daddr_t blkno;
	long bsize;
{
	register struct buf *bp;

	sysinfo.lread++;
	bp = getblk(dev, blkno, bsize);
	if (bp->b_flags & B_DONE)
		return bp;
	bp->b_flags |= B_READ;
	bp->b_bcount = bsize;
	(*bdevsw[getmajor(dev)].d_strategy)(bp);
	u.u_ior++;
	sysinfo.bread++;
	(void) biowait(bp);
	return bp;
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller).
 */
struct buf *
breada(dev, blkno, rablkno, bsize)
	register dev_t dev;
	daddr_t blkno, rablkno;
	long bsize;
{
	register struct buf *bp, *rabp;

	bp = NULL;
	if (!incore(dev, blkno, bsize)) {
		sysinfo.lread++;
		bp = getblk(dev, blkno, bsize);
		if ((bp->b_flags & B_DONE) == 0) {
			bp->b_flags |= B_READ;
			bp->b_bcount = bsize;
			(*bdevsw[getmajor(dev)].d_strategy)(bp);
			u.u_ior++;
			sysinfo.bread++;
		}
	}
	if (rablkno && bfreelist.b_bcount>1 && !incore(dev, rablkno, bsize)) {
		rabp = getblk(dev, rablkno, bsize);
		if (rabp->b_flags & B_DONE)
			brelse(rabp);
		else {
			rabp->b_flags |= B_READ|B_ASYNC;
			rabp->b_bcount = bsize;
			(*bdevsw[getmajor(dev)].d_strategy)(rabp);
			u.u_ior++;
			sysinfo.bread++;
		}
	}
	if (bp == NULL)
		return bread(dev, blkno, bsize);
	(void) biowait(bp);
	return bp;
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
void
bwrite(bp)
	register struct buf *bp;
{
	register flag;

	sysinfo.lwrite++;
	flag = bp->b_flags;
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	u.u_iow++;
	sysinfo.bwrite++;
	(*bdevsw[getmajor(bp->b_edev)].d_strategy)(bp);
	if ((flag & B_ASYNC) == 0) {
		(void) biowait(bp);
		brelse(bp);
	} else
		basyncnt++;
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * Also save the time that the block is first marked as delayed
 * so that it will be written in a reasonable time.
 */
void
bdwrite(bp)
	register struct buf *bp;
{
	sysinfo.lwrite++;
	if ((bp->b_flags & B_DELWRI) == 0)
		bp->b_start = lbolt;
	bp->b_flags |= B_DELWRI | B_DONE;
	bp->b_resid = 0;
	brelse(bp);
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
void
bawrite(bp)
	register struct buf *bp;
{

	if (bfreelist.b_bcount > 4)
		bp->b_flags |= B_ASYNC;
	bwrite(bp);
}

/*
 * Release the buffer, with no I/O implied.
 */
void
brelse(bp)
	register struct buf *bp;
{
	register struct buf **backp;
	register s;

	if (bp->b_flags & B_WANTED)
		wakeprocs((caddr_t)bp, PRMPT);
	if (bfreelist.b_flags & B_WANTED) {
		bfreelist.b_flags &= ~B_WANTED;
		wakeprocs((caddr_t)&bfreelist, PRMPT);
	}
	if (bp->b_flags & B_ERROR) {
		bp->b_flags |= B_STALE|B_AGE;
		bp->b_flags &= ~(B_ERROR|B_DELWRI);
		bp->b_error = 0;
		bp->b_oerror = 0;
	}
	s = spl6();
	if (bp->b_flags & B_AGE) {
		backp = &bfreelist.av_forw;
		(*backp)->av_back = bp;
		bp->av_forw = *backp;
		*backp = bp;
		bp->av_back = &bfreelist;
	} else {
		backp = &bfreelist.av_back;
		(*backp)->av_forw = bp;
		bp->av_back = *backp;
		*backp = bp;
		bp->av_forw = &bfreelist;
	}
	bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC);
	bfreelist.b_bcount++;
	bp->b_reltime = (unsigned long)lbolt;
	splx(s);
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada).
 */
int
incore(dev, blkno, bsize)
	register dev_t dev;
	register daddr_t blkno;
	register long bsize;
{
	register struct buf *bp;
	register struct buf *dp;

	blkno = LTOPBLK(blkno, bsize);
	dp = bhash(dev, blkno);
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
		if (bp->b_blkno == blkno && bp->b_edev == dev
		  && (bp->b_flags & B_STALE) == 0)
			return 1;
	return 0;
}

/* 
 * getfreeblk() is called from getblk() or ngeteblk()
 * It runs down the free buffer list to free up
 * buffers when total number of buffer or total memory used
 * by buffers exceeds thresholds
 * If there is a buffer matches the request size, reuse
 * that buffer.
 * Otherwise, free buffers and re-allocate a new buffer
 */
struct buf *
getfreeblk(bsize)
	long bsize;
{
	register struct buf *bp, *savebp = NULL;
	register int s;
	
	s = spl6();
loop:
	bp = bfreelist.av_forw;
	if (bp != &bfreelist
	  && savebp == NULL 
	  && (bfreelist.b_bufsize < bsize
	    || (bp->b_flags & B_AGE) )) {
		ASSERT(bp != NULL);
		notavail(bp);			
		bp->av_forw = bp->av_back = NULL;

		/*
		 * This buffer hasn't been written to disk yet.
		 * Do it now and free it later.
		 */
		if (bp->b_flags & B_DELWRI) {
			bp->b_flags |= B_ASYNC | B_AGE;
			bwrite(bp);
			(void)spl6();
		}
		else {
			bremhash(bp);
			bp->b_forw = bp->b_back = bp;
			if (savebp == NULL && bp->b_bufsize == bsize)
				savebp = bp;
			else {
				/*
				 * If size doesn't match, free it.
				 */
				kmem_free(bp->b_un.b_addr, bp->b_bufsize);
				bfreelist.b_bufsize += bp->b_bufsize;
				struct_zero(bp, sizeof(struct buf));
				bp->b_flags |= B_KERNBUF;
				bp->b_forw = bp->b_back = bp;
				bp->av_forw = bhdrlist.av_forw;
				bhdrlist.av_forw = bp;
			}
		}
		goto loop;
	}
	if (savebp != NULL) {
		(void)splx(s);
		return savebp;
	}
	/*
	 * If not enough memory for this buffer, sleep.  When we
	 * return from sleep(), we must return to the caller to
	 * check the hash queue again.
	 */	
	if (bfreelist.b_bufsize < bsize) {
		ASSERT(bfreelist.av_forw == &bfreelist);
		bfreelist.b_flags |= B_WANTED;
		sleep((caddr_t)&bfreelist, PRIBIO+1);
		(void)splx(s);
		return NULL;
	}
	/*
	 * Allocate a new buffer.  Get a buffer header first.
	 * If no free buffer header, allocate a chunk of
	 * buffer headers.
	 */	
	bfreelist.b_bufsize -= bsize;
	(void)splx(s);
	if (bhdrlist.av_forw == NULL) {
		struct buf *dp,*tdp;
		int i;

		dp = (struct buf *)kmem_zalloc(sizeof(struct buf) * v.v_buf,
				KM_SLEEP);
		ASSERT(dp != NULL);

		if (bhdrlist.av_forw != NULL) {
			kmem_free(dp, sizeof(struct buf) * v.v_buf);
		} else {
			tdp = dp;
			for (i = 0; i < v.v_buf ; i++,dp++) {
				dp->b_dev = (o_dev_t)NODEV;
				dp->b_edev = (dev_t)NODEV;
				dp->b_un.b_addr = NULL;
				dp->b_flags = B_KERNBUF;
				dp->b_bcount = 0;
				dp->b_forw = dp->b_back = dp;
				dp->av_forw = dp + 1;
			}
			(--dp)->av_forw = bhdrlist.av_forw;
			bhdrlist.av_forw = tdp;
			nbuf += v.v_buf;
		}
	}
	bp = bhdrlist.av_forw;
	bhdrlist.av_forw = bp->av_forw;
	bp->av_forw = bp->av_back = NULL;
	bp->b_un.b_addr = (caddr_t)kmem_zalloc(bsize, KM_SLEEP);
	bp->b_bufsize = bsize;
	return bp;
}		

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 */
struct buf *
getblk(dev, blkno, bsize)
	register dev_t dev;
	register daddr_t blkno;
	long bsize;
{
	register struct buf *bp;
	register struct buf *dp, *nbp = NULL; 
	register int s;

	if (getmajor(dev) >= bdevcnt)
		cmn_err(CE_PANIC,"blkdev");

	blkno = LTOPBLK(blkno, bsize);
	s = spl0();
loop:
	if ((dp = bhash(dev, blkno)) == NULL)
		cmn_err(CE_PANIC,"devtab");
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
		if (bp->b_blkno != blkno || bp->b_edev != dev
		  || bp->b_flags & B_STALE)
			continue;
		spl6();
		if (bp->b_flags & B_BUSY) {
			bp->b_flags |= B_WANTED;
			syswait.iowait++;
			sleep((caddr_t)bp, PRIBIO+1);
			syswait.iowait--;
			spl0();
			goto loop;
		}
		splx(s);
		bp->b_flags &= ~B_AGE;
		notavail(bp);
		if (nbp) {
			nbp->b_forw = nbp->b_back = nbp;
			nbp->b_flags = B_KERNBUF | B_BUSY;
			nbp->b_dev = (o_dev_t)NODEV;
			nbp->b_edev = (dev_t)NODEV;
			nbp->b_bcount = bsize;
			brelse(nbp);
		}
		return bp;
	}

	splx(s);
	if (nbp == NULL) {
		nbp = getfreeblk(bsize);
		spl0();
		goto loop;
	}
     	bp = nbp;
	bp->b_flags = B_KERNBUF | B_BUSY;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_edev = dev;
	bp->b_dev = (o_dev_t)cmpdev(dev);
	bp->b_blkno = blkno;
	bp->b_bcount = bsize;
	bp->b_iodone = NULL;
	return bp;
}

/*
 * get an empty block,
 * not assigned to any particular device.
 */
struct buf *
ngeteblk(bsize)
	long bsize;
{
	register struct buf *bp;
	register struct buf *dp;


	while ((bp = getfreeblk(bsize)) == NULL)
		;

	dp = &bfreelist;
	bp->b_flags = B_KERNBUF | B_BUSY | B_AGE;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = (o_dev_t)NODEV;
	bp->b_edev = (dev_t)NODEV;
	bp->b_bcount = bsize;
	bp->b_iodone = NULL;
	return bp;
}

/* 
 * Interface of geteblk() is kept intact to maintain driver compatibility.
 * Use ngeteblk() to allocate block size other than 1 KB.
 */
struct buf *
geteblk()
{
	return ngeteblk((long)1024);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
int
iowait(bp)
	struct buf *bp;
{
	return biowait(bp);
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
void
iodone(bp)
	struct buf *bp;
{
	biodone(bp);
}

/*
 * Zero the core associated with a buffer.
 */
void
clrbuf(bp)
	struct buf *bp;
{
	bzero((caddr_t)bp->b_un.b_words, bp->b_bcount);
	bp->b_resid = 0;
}

/*
 * Make sure all write-behind blocks on dev (or NODEV for all)
 * are flushed out.
 */
void
bflush(dev)
	register dev_t dev;
{
	register struct buf *bp;
	register int s;

loop:
	s = spl6();
	for (bp = bfreelist.av_forw; bp != &bfreelist; bp = bp->av_forw) {
		if ((bp->b_flags & B_DELWRI)
		  && (dev == NODEV || dev == bp->b_edev)) {
			bp->b_flags |= B_ASYNC;
			notavail(bp);
			bwrite(bp);
			(void) splx(s);
			goto loop;
		}
	}
	(void) splx(s);
}

/*
 * Ensure that a specified block is up-to-date on disk.
 */
void
blkflush(dev, blkno, bsize)
	dev_t dev;
	daddr_t blkno;
	int bsize;
{
	register struct buf *bp, *dp;
	int s;

	blkno = LTOPBLK(blkno, bsize);
	dp = bhash(dev, blkno);
loop:
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
		if (bp->b_blkno != blkno || bp->b_edev != dev
		  || (bp->b_flags & B_STALE))
			continue;
		s = spl6();
		if (bp->b_flags & B_BUSY) {
			bp->b_flags |= B_WANTED;
			syswait.iowait++;
			(void) sleep((caddr_t) bp, PRIBIO+1);
			syswait.iowait--;
			(void) splx(s);
			goto loop;
		}
		if (bp->b_flags & B_DELWRI) {
			(void) splx(s);
			notavail(bp);
			bwrite(bp);
			goto loop;
		}
		(void) splx(s);
	}
}

/*
 * Wait for asynchronous writes to finish.
 */
void
bdwait()
{
	register int s;

	s = spl6();
	if (basyncnt)
		delay(200);
/*
	while (basyncnt) {
		basynwait = 1;
		sleep((caddr_t)&basyncnt, PRIBIO);
	}
*/
	splx(s);
}

/*
 * Invalidate blocks for a dev after last close.
 */
void
binval(dev)
	register dev_t dev;
{
	register struct buf *dp;
	register struct buf *bp;
	register i;

	for (i = 0; i < v.v_hbuf; i++) {
		dp = (struct buf *)&hbuf[i];
		for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
			if (bp->b_edev == dev)
				bp->b_flags |= B_STALE|B_AGE;
	}
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device hash buffer lists to empty.
 */
void
binit()
{
	register struct buf *bp;
	register unsigned i;
		
	/*
	 * Change buffer memory usage high-water-mark from kbytes
	 * to bytes.
	 */
	bfreelist.b_bufsize = v.v_bufhwm * 1024;

	bp = &bfreelist;
	bp->b_forw = bp->b_back = bp->av_forw = bp->av_back = bp;
	bhdrlist.av_forw = NULL;

	pfreecnt = v.v_pbuf;
	pfreelist.av_forw = bp = pbuf;
	for (; bp < &pbuf[v.v_pbuf-1]; bp++)
		bp->av_forw = bp+1;
	bp->av_forw = NULL;
	for (i = 0; i < v.v_hbuf; i++)
		hbuf[i].b_forw = hbuf[i].b_back = (struct buf *)&hbuf[i];
 	pgouthdrlist.av_forw = bp = pgoutbuf;
 	for (; bp < &pgoutbuf[npgoutbuf-1]; bp++)
 		bp->av_forw = bp+1;
 	bp->av_forw = NULL;
 	pgoutblast = bp;
 	notpgouthdrlist.av_forw = bp = notpgoutbuf;
 	for (; bp < &notpgoutbuf[nnotpgoutbuf-1]; bp++)
 		bp->av_forw = bp+1;
 	bp->av_forw = NULL;
 	notpgoutblast = bp;
}

/*
 * Wait for I/O completion on the buffer; return error code.
 * If bp was for synchronous I/O, bp is invalid and associated
 * resources are freed on return.
 */
int
biowait(bp)
	register struct buf *bp;
{
	int error = 0, s;

	syswait.iowait++;
	s = spl6();
	curproc->p_swlocks++;
	curproc->p_flag |= SSWLOCKS;
	while ((bp->b_flags & B_DONE) == 0) {
		bp->b_flags |= B_WANTED;
		(void) sleep((caddr_t)bp, PRIBIO);
	}
	if (--curproc->p_swlocks == 0)
		curproc->p_flag &= ~SSWLOCKS;
	(void) splx(s);
	syswait.iowait--;
	error = geterror(bp);

	if ((bp->b_flags & B_ASYNC) == 0) {
		if (bp->b_flags & B_PAGEIO)
			pvn_done(bp);
		else if (bp->b_flags & B_REMAPPED)
			bp_mapout(bp);
	}
	return error;
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
void
biodone(bp)
	register struct buf *bp;
{
#ifdef ASYNCIO
	extern	void	raiodone();
#endif /* ASYNC IO */

	if (bp->b_iodone && (bp->b_flags & B_KERNBUF)) {
		(*(bp->b_iodone))(bp);
		return;
	}
	ASSERT((bp->b_flags & B_DONE) == 0);
	bp->b_flags |= B_DONE;

#ifdef ASYNCIO
        /*
         * call raiodone routine if this
	 * is a raw disk async I/O
	 */
	if (bp->b_flags & B_RAIO) {
		(void) raiodone(bp);
		return;
	}
#endif /* ASYNC IO */

	if (bp->b_flags & B_ASYNC) {
		if ((bp->b_flags & B_READ) == 0)
			basyncnt--;
		if (basyncnt == 0 && basynwait) {
			basynwait = 0;
			wakeprocs((caddr_t)&basyncnt, PRMPT);
		}
		if (bp->b_flags & (B_PAGEIO|B_REMAPPED))
			swdone(bp);
		else
			brelse(bp);		/* release bp to 1k freelist */
	} else {
		bp->b_flags &= ~B_WANTED;
		wakeprocs((caddr_t)bp, PRMPT);
	}
}

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized code.
 */

#undef geterror

int
geterror(bp)
	register struct buf *bp;
{
	int error = 0;

	if (bp->b_flags & B_ERROR) {
		if (bp->b_flags & B_KERNBUF)
			error = bp->b_error;
		if (!error)
			error = bp->b_oerror;
		if (!error)
			error = EIO;
	}
	return error;
}

/*
 * Support for pageio buffers.
 *
 * This stuff should be generalized to provide a generalized bp
 * header facility that can be used for things other than pageio.
 */

/*
 * Pageio_out is a list of all the buffers currently checked out
 * for pageio use.
 */
STATIC struct bufhd pageio_out = {
	B_HEAD,
	(struct buf *)&pageio_out,
	(struct buf *)&pageio_out,
};

#ifdef AT386	/* 16 MB DMA support */
/*
 * Initialize a buf struct for use with pageio.
 */
void
dma_pageio_setup(bp, pp, len, vp, flags)
	register struct buf *bp;
	struct page *pp;
	u_int len;
	struct vnode *vp;
	int flags;
{
	register int s;

        if (bp == (struct buf *) NULL) {
		cmn_err(CE_PANIC,"dma_pageio_setup: buffer header is NULL\n");
        }         

	s = splhi();
	binshash(bp, (struct buf *)&pageio_out);
	(void)splx(s);
	bp->b_un.b_addr = 0;
	bp->b_error = 0;
	bp->b_oerror = 0;
	bp->b_resid = 0;
	bp->b_bcount = len;
	bp->b_bufsize = len;
	bp->b_pages = pp;
	bp->b_flags = B_KERNBUF | B_PAGEIO | B_NOCACHE | B_BUSY | flags;

	VN_HOLD(vp);
	bp->b_vp = vp;

	/*
	 * This count is bumped for async writes here and in
	 * bwrite().
	 */
	if ((flags & (B_ASYNC|B_READ)) == B_ASYNC)
		basyncnt++;

	/*
	 * Caller sets dev & blkno and can adjust
	 * b_addr for page offset and can use bp_mapin
	 * to make pages kernel addressable.
	 */
}
#endif	/* 16 MB DMA support */

#define	NOMEMWAIT() (u.u_procp == proc_pageout || u.u_procp == proc_sched)
#define	PAGEOUT() (u.u_procp == proc_pageout)

/*
 * Allocate and initialize a buf struct for use with pageio.
 */
struct buf *
pageio_setup(pp, len, vp, flags)
	struct page *pp;
	u_int len;
	struct vnode *vp;
	int flags;
{
	register struct buf *bp;
	register int s;

loop:
        bp = (struct buf *) kmem_zalloc(sizeof (*bp),
          NOMEMWAIT() ? KM_NOSLEEP : KM_SLEEP);

        if (bp == NULL) {
		/* use hidden buffer headers rather than sleep on memory.
		* Even regular I/O can cause deadlock if inodes
		* doing I/O are the ones that must be locked
		* to free up memory.
		*/
		if (!PAGEOUT()) {
			if ((bp = notpgouthdrlist.av_forw) != NULL) {
				s = splhi();
				notpgouthdrlist.av_forw = bp->av_forw;
				bp->av_forw = bp->av_back = NULL;
				splx(s);
				notpgoutbufused++;
				goto gotone;
			}
			notpgoutboutcnt++;
			notpgoutwanted = 1;
			(void) sleep((caddr_t)&notpgoutwanted, PSWP+1);
			goto loop;
		} else {
			if ((bp = pgouthdrlist.av_forw) != NULL) {
				s = splhi();
				pgouthdrlist.av_forw = bp->av_forw;
				bp->av_forw = bp->av_back = NULL;
				splx(s);
				pgoutbufused++;
				goto gotone;
			}
			pgoutboutcnt++;
		}
		/*
		* We are pageout and cannot risk sleeping for more
		* memory so we return an error condition instead.
		*/
		return NULL;
	} 

gotone:
	s = splhi();
	binshash(bp, (struct buf *)&pageio_out);
	splx(s);
	bp->b_un.b_addr = 0;
	bp->b_error = 0;
	bp->b_oerror = 0;
	bp->b_resid = 0;
	bp->b_bcount = len;
	bp->b_bufsize = len;
	bp->b_pages = pp;
	bp->b_flags = B_KERNBUF | B_PAGEIO | B_NOCACHE | B_BUSY | flags;

	/*
	 * If vp is not NULL, this is the parent bp. Increment the
	 * vcount of the vnode.
	 */
	if (vp) {
		VN_HOLD(vp);
		bp->b_vp = vp;
	} else {
		bp->b_vp = NULL;
	}

	/*
	 * This count is bumped for async writes here and in
	 * bwrite().
	 */
	if ((flags & (B_ASYNC|B_READ)) == B_ASYNC)
		basyncnt++;

	/*
	 * Caller sets dev & blkno and can adjust
	 * b_addr for page offset and can use bp_mapin
	 * to make pages kernel addressable.
	 */
	return bp;
}

void
pageio_done(bp)
	register struct buf *bp;
{
	register int s;

	if (bp->b_flags & B_REMAPPED)
		bp_mapout(bp);
	s = splhi();
	bremhash(bp);
	(void)splx(s);
	if (bp->b_vp)
		VN_RELE(bp->b_vp);
	if (bp >= pgoutbuf && bp <= pgoutblast) {
		s = splhi();
		bp->av_forw = pgouthdrlist.av_forw;
		pgouthdrlist.av_forw = bp;
		splx(s);
	} else if (bp >= notpgoutbuf && bp <= notpgoutblast) {
		s = splhi();
		bp->av_forw = notpgouthdrlist.av_forw;
		notpgouthdrlist.av_forw = bp;
		splx(s);
		if (notpgoutwanted) {
			notpgoutwanted = 0;
			wakeprocs((caddr_t)&notpgoutwanted, PRMPT);
		}
	} else
		kmem_free((caddr_t)bp, sizeof (*bp));
}
	
/*
 * Break up the request that came from bread/bwrite into chunks of
 * contiguous memory so we can get around the DMAC limitations
 */

/*
 * Determine number of bytes to page boundary.
 */
#define	pgbnd(a)	(NBPP - ((NBPP - 1) & (int)(a)))

void
buf_breakup(strat, obp)
	int (*strat)();
	register struct buf *obp;
{
	register int cc, iocount, s;
	register struct buf *bp;	
	
/*	ASSERT((obp->b_flags & B_PAGEIO)== NULL); */
	bp = (struct buf *)kmem_zalloc(sizeof (*bp), KM_SLEEP);
	bcopy((caddr_t)obp, (caddr_t)bp, sizeof(*bp));
	iocount = obp->b_bcount;
	bp->b_iodone = NULL;
	bp->b_flags &= ~B_ASYNC;

	/*
	 * The buffer is on a sector boundary but not necessarily
	 * on a page boundary.
	 */
	if ((bp->b_bcount = cc = 
	  min(iocount, pgbnd(bp->b_un.b_addr))) < NBPP) {
		/*
		 * Do the fragment of the buffer that's in the
		 * first page.
		 */
		bp->b_flags &= ~B_DONE;
		(*strat)(bp);
		s = spl6();
		while ((bp->b_flags & B_DONE) == 0) {
			bp->b_flags |= B_WANTED;
			sleep((caddr_t)bp, PRIBIO);
		}
		(void) splx(s);
		if (bp->b_flags & B_ERROR) {
			goto out;
		}
		bp->b_blkno += btod(cc);
		bp->b_un.b_addr += cc;
		iocount -= cc;
	}

	/*
	 * Now do the DMA a page at a time.
	 */
	while (iocount > 0) {
		bp->b_bcount = cc = min(iocount, NBPP);
		bp->b_flags &= ~B_DONE;
		(*strat)(bp);
		s = spl6();
		while ((bp->b_flags & B_DONE) == 0) {
			bp->b_flags |= B_WANTED;
			sleep((caddr_t)bp, PRIBIO);
		}
		(void) splx(s);
		if (bp->b_flags & B_ERROR)
			goto out;
		bp->b_blkno += btod(cc);
		bp->b_un.b_addr += cc;
		iocount -= cc;
	}
	kmem_free((caddr_t)bp, sizeof(*bp));
	s = spl6();
	biodone(obp);
	splx(s);
	return;
out:
	if (bp->b_error)
		obp->b_error = bp->b_error;
	else if (bp->b_oerror)
		obp->b_error = bp->b_oerror;
	obp->b_flags |= B_ERROR;
	kmem_free((caddr_t)bp, sizeof(*bp));
	s = spl6();
	biodone(obp);
	splx(s);
}

/* 
 * Debugging print of buffer headers.  Can be invoked from kernel debugger.
 */
#ifdef DEBUG

STATIC void
printbuf()
{
	struct buf *bp;
	register i;

	printf("bufno		av_f		av_b		forw	back	flag	count\n");
	bp = &bfreelist;
	printf("%x\n", bp);
	printf("freelist	%x	%x	%x	%x	%d\n",
	  bp->av_forw, bp->av_back,bp->b_forw, bp->b_back,
	  bp->b_bcount);
	bp = &bhdrlist;
	printf("%x\n", bp);
	printf("bhdrlist	%x\n", bp->av_forw);

}

#endif

/* XENIX Support: XENIX drivers may call these.
 * get an empty block,
 * not assigned to any particular device
 * flag can be ignored for the 386 - provided for compatibility
 */
struct buf *
getablk(flag)
{
	return(geteblk());
}

/*
 * generalized seek sort for disk.  sorts on the b_sector field of
 * the buf structure.
 */

int Dsort = 0;                  /* == 1 for special read preference */

disksort(dp, bp)
struct iobuf *dp;
register struct buf *bp;
{
	register struct buf *ap;
	struct buf *tp;

	tp = dp->b_actf;
	if(tp == NULL) {
		dp->b_actf = bp;
		dp->b_actl = bp;
		bp->av_forw = NULL;
		return;
	}
	for(ap = tp->av_forw; ap != NULL; tp = ap, ap = ap->av_forw) {
		if (Dsort) {
			if ((bp->b_flags&B_READ) && (ap->b_flags&B_READ) == 0)
			    break; /* incoming reads preceed queued writes */
			if ((bp->b_flags&B_READ) == 0 && (ap->b_flags&B_READ))
			    continue; /* incoming writes follow queued reads */
		}
		/* classic elevator up-down sorting */
		if(ap->b_sector < bp->b_sector  &&  bp->b_sector <= tp->b_sector)
			break;
		if(ap->b_sector > bp->b_sector  &&  bp->b_sector >= tp->b_sector)
			break;
	}
	bp->av_forw = ap;
	tp->av_forw = bp;
	if(tp == dp->b_actl)
		dp->b_actl = bp;
}
/* End XENIX Support */

STATIC char sbuffer[512];
STATIC struct buf sbuf;
STATIC int sbuf_inuse = 0;

int
dksize(dev)
	dev_t dev;
{
	register int maj, error, max, min, middle;
	register char saved_error;
	int (*size)();
#if 0
	int old;
#endif

	if ((maj = getmajor(dev)) >= bdevcnt)
		return(-1);
	size = bdevsw[maj].d_size;
	if (size != nodev)
		return((*size)(dev));

	/* Avoid multiple access of the buffer header */
	while (sbuf_inuse)
		(void) sleep((caddr_t)&sbuf, PSWP+2);
	sbuf_inuse = 1;

	saved_error = u.u_error;
	u.u_error = 0;
	(void)(*bdevsw[maj].d_open)(cmpdev(dev), 0, OTYP_BLK);
	if (u.u_error) {
		(void)(*bdevsw[maj].d_close)(cmpdev(dev), 0, OTYP_BLK);
		u.u_error = saved_error;
		sbuf_inuse = 0;
		return(-1);
	}

#if 0
	if (*bdevsw[maj].d_flag & D_OLD) {
		old = 1;
		(void)(*bdevsw[maj].d_open)(dev, 0, OTYP_BLK);
		error = u.u_error;
	} else {
		old = 0;
		error = (*bdevsw[maj].d_open)(dev, 0, OTYP_BLK, cr);
	}
	if (error) {
		if (old)
			u.u_error = saved_error;
		return ENXIO;
	}
#endif
	bzero((caddr_t)&sbuf, sizeof(struct buf));
	sbuf.b_dev = cmpdev(dev);
	sbuf.b_bcount = NBPSCTR;
	sbuf.b_un.b_addr = &sbuffer[0];

	max = 8 * 1024 * 1024;
	min = 0;
	do {
		middle = (max + min) / 2;
		sbuf.b_blkno = middle;
		sbuf.b_flags = B_KERNBUF | B_BUSY | B_READ;
		sbuf.b_error = 0;
		(*bdevsw[maj].d_strategy)(&sbuf);
		error = biowait(&sbuf);
		if (error) 
			max = middle;
		else
			min = middle;
	} while (max != (min + 1));
	
	(void)(*bdevsw[maj].d_close)(cmpdev(dev), 0, OTYP_BLK);
	u.u_error = saved_error;
	sbuf_inuse = 0;
	wakeprocs((caddr_t)&sbuf, PRMPT);
	return (max - 1);
}
