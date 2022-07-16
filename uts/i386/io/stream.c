/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:stream.c	1.3.2.4"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/strsubr.h"
#include "sys/strstat.h"
#include "sys/conf.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/immu.h"
#include "sys/tuneable.h"
#include "sys/map.h"
#include "sys/cmn_err.h"
#include "sys/kmem.h"
#include "sys/errno.h"

/*
 * This file contains all the STREAMS utility routines that may
 * be used by modules and drivers.
 */

extern struct mdbblock 	*mdbfreelist;
extern struct msgb	*msgfreelist;
extern struct strinfo	Strinfo[];	/* dynamic resource info */
extern long Strcount;


extern queue_t *qhead;
extern queue_t *qtail;
extern char qrunflag;
extern char queueflag;
extern char strbcwait;
extern char strbcflag;
extern struct bclist strbcalls;

struct mdbblock *xmdballoc();
struct msgb	*xmsgalloc();


#ifdef	DEBUG
int	_streams_crash = 1; /* to signal the crash routines */   
#else
int	_streams_crash = 0;
#endif


/*
 *	Pointer types:
 *			(struct mdbblock *)	mdbfreelist
 *			(struct msgb *)		msgfreelist
 *			(struct mbinfo *)	m_prev, m_next
 */


#define	FREEMDBBLOCK( mdbp )	{ \
	register int s = splstr(); \
 \
	((struct msgb *)(mdbp))->b_next = (struct msgb *) mdbfreelist; \
	mdbfreelist = (mdbp); \
	strst.mdbblock.use--; \
	_DELETE_MDB_INUSE(mdbp); \
	splx(s); \
}

#define	FREEMSGBLOCK( mesgp )	{ \
	register int s = splstr(); \
 \
	(mesgp)->b_next = (struct msgb *) msgfreelist; \
	msgfreelist = mesgp; \
	strst.msgblock.use--; \
	_DELETE_MSG_INUSE((struct mbinfo *)mesgp); \
	splx(s); \
}


#define ALLOCMDBBLOCK(mdbp)	{ \
	mdbp = mdbfreelist; \
	mdbfreelist = (struct mdbblock *)(((struct msgb *)mdbp)->b_next );\
	BUMPUP(strst.mdbblock);	\
	_INSERT_MDB_INUSE(mdbp); \
}


#define ALLOCMSGBLOCK(mesgp)		{ \
	mesgp = msgfreelist; \
	msgfreelist = mesgp->b_next; \
	BUMPUP(strst.msgblock);	\
	_INSERT_MSG_INUSE((struct mbinfo *)mesgp); \
}

/* Status flags for triplets */
/* _MDBBLK:	Both message and data headers are valid.*/
/* _NOT_M :	Invalid message header.			*/
/* _NOT_D :	Invalid data header.			*/
/* _XCONN :	Valid headers, but the message header points to a different data header */ 

#define		_MDBBLK		0
#define		_NOT_M		1
#define		_NOT_D		2
#define		_XCONN		3


/* Convenient macros for accessing fields */

#define	m_bdatap	m_mblock.b_datap
#define m_bnext		m_mblock.b_next
#define m_bprev		m_mblock.b_prev
#define	m_bcont		m_mblock.b_cont
#define m_brptr		m_mblock.b_rptr
#define	m_bwptr		m_mblock.b_wptr
#define	m_bband		m_mblock.b_band
#define	m_bflag		m_mblock.b_flag

#define	d_dbfreep	d_dblock.db_freep
#define d_dbfrtnp	d_dblock.db_frtnp
#define	d_dbbase	d_dblock.db_base
#define	d_dblim		d_dblock.db_lim
#define	d_dbref		d_dblock.db_ref
#define	d_dbtype	d_dblock.db_type
#define d_dbiswhat	d_dblock.db_iswhat
#define	d_dbsize	d_dblock.db_size
#define	d_dbmsgaddr	d_dblock.db_msgaddr

#define	 _DUPB( newbp, oldbp) { \
	ASSERT((oldbp)); \
	if (!msgfreelist){ \
		newbp	= dupb(oldbp); \
	} \
	else { \
		ALLOCMSGBLOCK( newbp ); \
		saveaddr(&(((struct mbinfo *)newbp)->m_func)); \
		((newbp)->b_datap = (oldbp)->b_datap)->db_ref++; \
		(newbp)->b_next = NULL; \
		(newbp)->b_prev = NULL; \
		(newbp)->b_cont = NULL; \
		(newbp)->b_rptr = (oldbp)->b_rptr; \
		(newbp)->b_wptr = (oldbp)->b_wptr; \
		(newbp)->b_flag = (oldbp)->b_flag; \
		(newbp)->b_band = (oldbp)->b_band; \
	} \
}


#define	 _FREEB(msgp) { \
	register struct datab *dbp; \
	dbp  = msgp->b_datap; \
	ASSERT( dbp->db_ref > 0 ); \
	if (dbp->db_msgaddr != (caddr_t)msgp) { \
		FREEMSGBLOCK(msgp); \
	} \
	else { \
		if (((struct mdbblock *)msgp)->datblk.d_dbiswhat == _NOT_D) { \
			FREEMDBBLOCK((struct mdbblock *)msgp); \
		} \
		else ((struct mdbblock *)msgp)->datblk.d_dbiswhat = _NOT_M; \
	} \
	if ( --(dbp->db_ref) == 0 ) { \
		if ((char *)dbp + sizeof(struct dbinfo) != (char *)(dbp->db_base)) { \
			if (dbp->db_frtnp == NULL) { \
				kmem_free(dbp->db_base, \
				   ((char *)(dbp->db_lim) - (char *)(dbp->db_base))); \
				Strcount -= (dbp->db_lim - dbp->db_base); \
			} \
			else if (dbp->db_frtnp->free_func) { \
				register int s = splstr(); \
\
				 (*dbp->db_frtnp->free_func)(dbp->db_frtnp->free_arg); \
				splx(s); \
			} \
		} \
		if (dbp->db_iswhat == _NOT_M) { \
			msgp = (struct msgb *)((char *)dbp - sizeof(struct mbinfo)); \
			FREEMDBBLOCK((struct mdbblock *)msgp); \
		} \
		else \
			dbp->db_iswhat = _NOT_D; \
	} \
}

# ifdef DEBUG

/* routines for managing in-use lists : only in DEBUG mode */

delete_msg_inuse(msgptr)
register struct	mbinfo 	*msgptr;
{

	if (msgptr->m_prev) { 
		msgptr->m_prev->m_next = msgptr->m_next; 
		if (msgptr->m_next) 
			msgptr->m_next->m_prev = msgptr->m_prev; 
	} 
	else { 
		Strinfo[DYN_MSGBLOCK].sd_head = (void *)(msgptr->m_next); 
		if (msgptr->m_next) 
			msgptr->m_next->m_prev = NULL;
	}
}

delete_mdb_inuse(mdbptr)
struct mdbblock *mdbptr;
{
	register struct mbinfo *msgptr = &(mdbptr->msgblk);
	register struct dbinfo *datptr = &(mdbptr->datblk);


	if (msgptr->m_prev) { 
		msgptr->m_prev->m_next = msgptr->m_next; 
		if (msgptr->m_next) 
			msgptr->m_next->m_prev = msgptr->m_prev; 
	} 
	else { 
		Strinfo[DYN_MSGBLOCK].sd_head = (void *)(msgptr->m_next); 
		if (msgptr->m_next) 
			msgptr->m_next->m_prev = NULL;
	} 
	if (datptr->d_prev) { 
		datptr->d_prev->d_next = datptr->d_next; 
		if (datptr->d_next) 
			datptr->d_next->d_prev = datptr->d_prev; 
	} 
	else { 
		Strinfo[DYN_MDBBLOCK].sd_head = (void *)(datptr->d_next); 
		if (datptr->d_next) 
			datptr->d_next->d_prev = NULL;
	} 
}

insert_msg_inuse(msgptr)
register struct mbinfo *msgptr;
{
	register int s = splstr();

	msgptr->m_next = (struct mbinfo *)(Strinfo[DYN_MSGBLOCK].sd_head);
	Strinfo[DYN_MSGBLOCK].sd_head = (void *)(msgptr);
	if (msgptr->m_next)
		msgptr->m_next->m_prev = (struct mbinfo *)msgptr;
	msgptr->m_prev = NULL;
	splx(s);
}

insert_mdb_inuse(mdbptr)
struct mdbblock *mdbptr;
{
	register struct mbinfo *msgptr = &(mdbptr->msgblk);
	register struct dbinfo *datptr = &(mdbptr->datblk);
	register int s = splstr();

	msgptr->m_next = (struct mbinfo *)(Strinfo[DYN_MSGBLOCK].sd_head);
	Strinfo[DYN_MSGBLOCK].sd_head = (void *)(msgptr);
	if (msgptr->m_next)
		msgptr->m_next->m_prev = (struct mbinfo *)msgptr;
	msgptr->m_prev = NULL;

	datptr->d_next = (struct dbinfo *)(Strinfo[DYN_MDBBLOCK].sd_head);
	Strinfo[DYN_MDBBLOCK].sd_head = (void *)(datptr);
	if (datptr->d_next)
		datptr->d_next->d_prev = (struct dbinfo *)datptr;
	datptr->d_prev = NULL;
	splx(s);
}

# endif



/*
 * NOTE: Some code in streamio.c relies on knowledge of the underlying
 * message-data-buffer pool structure.
 *
 * 'pri' is no longer used, but is retained for compatibility.
 */
struct msgb *
allocb(size, pri)
register int size;
uint pri;
{
	register int s;
	register struct mdbblock *mdbptr;
	register unchar *buf = NULL;


	/* allocate the combined message/data block + FASTBUF sized buffer
	 * from the freelist.
	 */

	s = splstr();

	if ( mdbfreelist ) { 
		ALLOCMDBBLOCK(mdbptr);
	} else {
		if (!(mdbptr = xmdballoc())) {
			splx(s);
			return(NULL);
		}
	}
	splx(s);

	/* save return address */
	saveaddr(&(((struct mbinfo *)mdbptr)->m_func));

	((struct msgb *)mdbptr)->b_datap = 
		(struct datab *)(((char *)mdbptr) + sizeof(struct mbinfo)); 
	((struct msgb *)mdbptr)->b_next = NULL;
	((struct msgb *)mdbptr)->b_prev = NULL;
	((struct msgb *)mdbptr)->b_cont = NULL;
	mdbptr->datblk.d_dbfrtnp = NULL;
	((struct msgb *)mdbptr)->b_band = 0;
	((struct msgb *)mdbptr)->b_flag = 0;

	(mdbptr->datblk).d_dbtype = M_DATA;
	(mdbptr->datblk).d_dbref = 1;
	(mdbptr->datblk).d_dbmsgaddr = (caddr_t)mdbptr;
	(mdbptr->datblk).d_dbiswhat = _MDBBLK;

	if (size > (int)FASTBUF) {
		if ((buf = kmem_alloc(size, KM_NOSLEEP)) == NULL ) {
			FREEMDBBLOCK(mdbptr);
			return(NULL);
		}
		else {
			((struct msgb *)mdbptr)->b_rptr = 
				((struct msgb *)mdbptr)->b_wptr = 
					(mdbptr->datblk).d_dbbase = buf;
			(mdbptr->datblk).d_dbsize = size;	
			(mdbptr->datblk).d_dblim = ((unsigned char *)buf) + size;
			Strcount += size;
		}
	}
	else	{

		((struct msgb *)mdbptr)->b_rptr = 
			((struct msgb *)mdbptr)->b_wptr = 
				(mdbptr->datblk).d_dbbase = 
				 	(unsigned char *)(&(mdbptr->databuf));
		if (size == 0 )
			(mdbptr->datblk).d_dbsize = 4;
		else
			(mdbptr->datblk).d_dbsize = size;

		(mdbptr->datblk).d_dblim = 
			(unsigned char *)mdbptr + sizeof(struct mdbblock);
	}

	return((struct msgb *)mdbptr);
			
} 


/* 
 * Allocate a class zero data block and attach an externally-supplied
 * data buffer pointed to by base to it.
 */
/* ARGSUSED */
mblk_t *
esballoc(base, size, pri, fr_rtn)
	unsigned char *base;
	int size;
	int pri;
	frtn_t *fr_rtn;
{
	register struct msgb *mp;

	if (!base || !fr_rtn)
		return(NULL);
	if (!(mp = allocb(-1, pri)))
		return(NULL);
	
	mp->b_datap->db_frtnp = fr_rtn;
	mp->b_datap->db_base = base;
	mp->b_datap->db_lim = base + size;
	mp->b_datap->db_size = size;
	mp->b_rptr = base;
	mp->b_wptr = base;
	return(mp);
}

/*
 * Call function 'func' with 'arg' when a class zero block can 
 * be allocated with priority 'pri'.
 */
/* ARGSUSED */
int
esbbcall(pri, func, arg)
	int pri;
	void (*func)();
	long arg;
{
	register s;
	struct strevent *sep;

	/*
	 * allocate new stream event to add to linked list 
	 */
	if (!(sep = sealloc(SE_NOSLP))) {
		cmn_err(CE_WARN, "esbbcall: could not allocate stream event\n");
		return(0);
	}

	sep->se_next = NULL;
	sep->se_func = func;
	sep->se_arg = arg;
	sep->se_size = -1;

	s = splstr();
	/*
	 * add newly allocated stream event to existing
	 * linked list of events.
	 */
	if ( strbcalls.bc_head == NULL ) {
		strbcalls.bc_head = strbcalls.bc_tail = sep;
	} else {
		strbcalls.bc_tail->se_next = sep;
		strbcalls.bc_tail = sep;
	}
	splx(s);

	strbcwait = 1;	/* so kmem_free() will know to call setqsched() */
	return((int)sep);
}

/*
 * test if block of given size can be allocated with a request of
 * the given priority.
 * 'pri' is no longer used, but is retained for compatibility.
 */
/* ARGSUSED */
int
testb(size, pri)
	register size;
	uint pri;
{
	extern ulong	kmem_avail();
	return(size <= (int)kmem_avail());
}

/*
 * Call function 'func' with argument 'arg' when there is a reasonably
 * good chance that a block of size 'size' can be allocated.
 * 'pri' is no longer used, but is retained for compatibility.
 */
/* ARGSUSED */
int
bufcall(size, pri, func, arg)
	uint size;
	int pri;
	void (*func)();
	long arg;
{
	register s;
	register class;
	struct strevent *sep, **tmp;
	struct dbalcst *dbp;

	ASSERT(size >= 0);

	if (!(sep = sealloc(SE_NOSLP))) {
		cmn_err(CE_WARN, "bufcall: could not allocate stream event\n");
		return(0);
	}

	sep->se_next = NULL;
	sep->se_func = func;
	sep->se_arg = arg;
	sep->se_size = size;

	s = splstr();
	/*
	 * add newly allocated stream event to existing
	 * linked list of events.
	 */
        if ( strbcalls.bc_head == NULL ) {
		strbcalls.bc_head = strbcalls.bc_tail = sep;
	} else {
                strbcalls.bc_tail->se_next = sep;
		strbcalls.bc_tail = sep;
	}
	splx(s);

	strbcwait = 1;	/* so kmem_free() will know to call setqsched() */
	return((int)sep);
}

/*
 * Cancel a bufcall request.
 */

void
unbufcall(id)
	register int id;
{
	register int s;
	register struct strevent *sep;
	register struct strevent *psep;

	s = splstr();
	psep = NULL;
	for (sep = strbcalls.bc_head; sep; sep = sep->se_next) {
		if (id == (int)sep)
			break;
		psep = sep;
	}
	if (sep) {
		if (psep)
			psep->se_next = sep->se_next;
		else
			strbcalls.bc_head = sep->se_next;
		if (sep == strbcalls.bc_tail)
			strbcalls.bc_tail = psep;
		sefree(sep);
	}
	splx(s);
}

/*
 * Free a message block.
 */

void
freeb(bp)
	register struct msgb *bp;
{
	register s;
	register struct datab *dbp;

	ASSERT(bp);
	
	s = splstr();
	dbp = bp->b_datap;

	ASSERT( dbp->db_ref > 0 );

	if (dbp->db_msgaddr != (caddr_t)bp) {
		FREEMSGBLOCK(bp);
	}
	else {
		if (((struct mdbblock *)bp)->datblk.d_dbiswhat == _NOT_D) {
			FREEMDBBLOCK((struct mdbblock *)bp);
		}
		else 
			((struct mdbblock *)bp)->datblk.d_dbiswhat = _NOT_M;
	}

	if ( --(dbp->db_ref) > 0 ) {
		splx(s);
		return; 
	}
	splx(s);

	/* seperate databuffer? */
	if ((char *)dbp + sizeof(struct dbinfo) != (char *)(dbp->db_base)) {
		if (dbp->db_frtnp == NULL) {
			kmem_free(dbp->db_base, 
			   ((char *)(dbp->db_lim) - (char *)(dbp->db_base) ));
			Strcount -= (dbp->db_lim - dbp->db_base);
		}
		else {
			if (dbp->db_frtnp->free_func) {
				s = splstr();
				(*dbp->db_frtnp->free_func)(dbp->db_frtnp->free_arg);
				splx(s);
			}
		}
	}

	/*
	 * and free the triplet so long as the message block part is
	 * not in use
	 */
	if (dbp->db_iswhat == _NOT_M) {
		bp = (struct msgb *)((char *)dbp - sizeof(struct mbinfo));
		FREEMDBBLOCK((struct mdbblock *)bp);
	}
	else 	/*
		 * update the flag to denote that the data block is not
		 * in use
		 */
		dbp->db_iswhat = _NOT_D;

	return;
} 


/*
 * Free all message blocks in a message using freeb().  
 * The message may be NULL.
 */
void
freemsg(bp)
	register mblk_t *bp;
{
	register mblk_t *tp;
	register int s;

	s = splstr();
	while (bp) {
		tp = bp->b_cont;
		_FREEB(bp);
		bp = tp;
	}
	splx(s);
}

/*
 * Duplicate a message block
 *
 * Allocate a message block and assign proper
 * values to it (read and write pointers)
 * and link it to existing data block.
 * Increment reference count of data block.
 */
mblk_t *
dupb(bp)
	register mblk_t *bp;
{
	register mblk_t *nbp;
	register int	s;

	ASSERT(bp);

	s = splstr();
	if (msgfreelist) {
		ALLOCMSGBLOCK(nbp);
	} else {
		if ((nbp = xmsgalloc()) == NULL) {
			splx(s);
			return(NULL);
		}
	}
	(nbp->b_datap = bp->b_datap)->db_ref++;
	splx(s);

	/* save return address */
	saveaddr(&(((struct mbinfo *)nbp)->m_func));

	nbp->b_next = NULL;
	nbp->b_prev = NULL;
	nbp->b_cont = NULL;
	nbp->b_rptr = bp->b_rptr;
	nbp->b_wptr = bp->b_wptr;
	nbp->b_flag = bp->b_flag;
	nbp->b_band = bp->b_band;
	return(nbp);
} 

/*
 * Duplicate a message block by block (uses dupb), returning
 * a pointer to the duplicate message.
 * Returns a non-NULL value only if the entire message
 * was dup'd.
 */

mblk_t *
dupmsg(bp)
	register mblk_t *bp;
{
	register mblk_t *head, *nbp;
	register int s;

	s = splstr();
	if (!bp) {
		splx(s);
		return NULL;
	}
	else	{
		_DUPB(nbp, bp);
		if ( !nbp ) {
			splx(s);
			return NULL;
		}
		else	
			head = nbp;
	}

	while (bp->b_cont) {
		_DUPB( nbp->b_cont, bp->b_cont);
		if (!(nbp->b_cont)) {
			freemsg(head);
			splx(s);
			return(NULL);
		}
		nbp = nbp->b_cont;
		bp = bp->b_cont;
	}
	splx(s);
	return(head);
}

/*
 * Copy data from message block to newly allocated message block and
 * data block.  The copy is rounded out to full word boundaries so that
 * the (usually) more efficient word copy can be done.
 * Returns new message block pointer, or NULL if error.
 */

mblk_t *
copyb(bp)
	register mblk_t *bp;
{
	register mblk_t *nbp;
	register dblk_t *dp, *ndp;
	caddr_t base;

	ASSERT(bp);
	ASSERT(bp->b_wptr >= bp->b_rptr);

	dp = bp->b_datap;
	if (!(nbp = allocb(dp->db_lim - dp->db_base, BPRI_MED)))
		return(NULL);
	nbp->b_flag = bp->b_flag;
	nbp->b_band = bp->b_band;
	ndp = nbp->b_datap;	
	ndp->db_type = dp->db_type;
	nbp->b_rptr = ndp->db_base + (bp->b_rptr - dp->db_base);
	nbp->b_wptr = ndp->db_base + (bp->b_wptr - dp->db_base);
	base = straln(nbp->b_rptr);
	strbcpy(straln(bp->b_rptr), base, straln(nbp->b_wptr + (sizeof(int)-1)) - base);
	return(nbp);
}


/*
 * Copy data from message to newly allocated message using new
 * data blocks.  Returns a pointer to the new message, or NULL if error.
 */
mblk_t *
copymsg(bp)
	register mblk_t *bp;
{
	register mblk_t *head, *nbp;
	register int s;

	if (!bp || !(nbp = head = copyb(bp)))
		return(NULL);

	s = splstr();
	while (bp->b_cont) {
		if (!(nbp->b_cont = copyb(bp->b_cont))) {
			freemsg(head);
			splx(s);
			return(NULL);
		}
		nbp = nbp->b_cont;
		bp = bp->b_cont;
	}
	splx(s);
	return(head);
}

/* 
 * link a message block to tail of message
 */
void
linkb(mp, bp)
	register mblk_t *mp;
	register mblk_t *bp;
{
	register int s;

	ASSERT(mp && bp);

	s = splstr();
	for (; mp->b_cont; mp = mp->b_cont)
		;
	mp->b_cont = bp;
	splx(s);
}

/*
 * unlink a message block from head of message
 * return pointer to new message.
 * NULL if message becomes empty.
 */
mblk_t *
unlinkb(bp)
	register mblk_t *bp;
{
	register mblk_t *bp1;
	register int s;

	ASSERT(bp);

	s = splstr();
	bp1 = bp->b_cont;
	bp->b_cont = NULL;
	splx(s);
	return(bp1);
}

/* 
 * remove a message block "bp" from message "mp"
 *
 * Return pointer to new message or NULL if no message remains.
 * Return -1 if bp is not found in message.
 */
mblk_t *
rmvb(mp, bp)
	register mblk_t *mp;
	register mblk_t *bp;
{
	register mblk_t *tmp;
	register mblk_t *lastp = NULL;
	register int s;

	ASSERT(mp && bp);
	s = splstr();
	for (tmp = mp; tmp; tmp = tmp->b_cont) {
		if (tmp == bp) {
			if (lastp)
				lastp->b_cont = tmp->b_cont;
			else
				mp = tmp->b_cont;
			tmp->b_cont = NULL;
			splx(s);
			return(mp);
		}
		lastp = tmp;
	}
	splx(s);
	return((mblk_t *)-1);
}



/*
 * Concatenate and align first len bytes of common message type.
 *
 * Len == -1  means concatenate everything.
 *
 * Returns 1 on success, 0 on failure.
 *
 * After pullup, mp points to the pulled up data.
 */

int
pullupmsg(mp, len)
register struct msgb *mp;
register int len;

{
	register struct datab	*dbp;
	register struct msgb	*bp;
	register int n;
	register int s;
	register struct msgb	*bcont;	
	ASSERT(mp != NULL);

	/*
	 * special cases
	 */

	if (len == -1)	{
		if (mp->b_cont == NULL) {
			if (str_aligned(mp->b_rptr)) 
				return(1);
			/* else */
			len = (int) (mp->b_wptr - mp->b_rptr);
		}
		else {
			len = xmsgsize(mp, 1);
		}
	}
	else {
		ASSERT( mp->b_wptr >= mp->b_rptr );
		if (mp->b_wptr - mp->b_rptr >= len) {
			if (str_aligned(mp->b_rptr))
				return (1);
			/* Stretch len to include what's in the first
			 * mblock, so we don't worry about residue in
			 * the first block after copying. The idea is
			 * to avoid having to set aside a new mblock for
			 * that residue. Also, this means, if db_ref==1, we
			 * can free the data buffer.
			 */
			len = mp->b_wptr - mp->b_rptr;
		}
		else  {
			if (xmsgsize(mp, 1) < len)
				return (0);
		}
	}

	/* Get a new "data block" if the current one is shared. 
	 * Otherwise, get only a new databuffer of size len.
	 */


	ASSERT( mp->b_datap->db_ref > 0 );
	if ((dbp = mp->b_datap)->db_ref > 1) {
		register struct msgb *newmp;
					
		if ((newmp = allocb(len, BPRI_MED)) == NULL )
			return(0);
		/* save return address */
		saveaddr(&(((struct mbinfo *)newmp)->m_func));

		((struct mdbblock *)newmp)->datblk.d_dbtype = dbp->db_type;
		--( dbp->db_ref );	
		
		/*--- copy the data into the new data buffer ---*/
		bp = mp;	
		while (len && bp) {
			ASSERT( bp->b_wptr >= bp->b_rptr );
			n = min((int)(bp->b_wptr - bp->b_rptr), len);
			bcopy(bp->b_rptr, newmp->b_wptr, n);
			newmp->b_wptr += n;
			len -= n;
			bp->b_rptr += n;
			if (bp->b_wptr != bp->b_rptr) 
				break;
			bcont = bp->b_cont;
			if (bp != mp)
				freeb(bp); /* free if not the head block */
			bp = bcont;
		} /* while */

		/* and update the message block and the (new) data
		 * block headers.
		 */
		newmp->b_band = mp->b_band; /* we don't care about message */
					    /* header *newmp; but the band */
					    /* and flag fields are in the  */
		newmp->b_flag = mp->b_flag; /* data block in some cases,   */
					    /* so we copy them             */

		s = splstr();

		if (dbp->db_msgaddr == (caddr_t)mp) {
			((struct mdbblock *)newmp)->datblk.d_dbmsgaddr = (caddr_t)mp;
			if (((struct mdbblock *)mp)->datblk.d_dbiswhat != _NOT_D)
			   ((struct mdbblock *)mp)->datblk.d_dbiswhat = _XCONN;
		}
		((struct mdbblock *)newmp)->datblk.d_dbiswhat = _NOT_M;

		mp->b_datap = newmp->b_datap;
		mp->b_rptr = newmp->b_rptr;
		mp->b_wptr = newmp->b_wptr;
		mp->b_cont = bp;

		splx(s);

		return(1);
	}
	else {
		register caddr_t newbuf_wp;
		caddr_t	newbuf;

		if ((newbuf = kmem_alloc(len, KM_NOSLEEP)) == NULL)
			return(0);
		/*
		 * we'll use the db_size field to remember the new
		 * data buffer size. Later, this will be used to
		 * set the db_lim.
		 */
		dbp->db_size = len;
		Strcount += len;

		/*
		 * Copy data into the new data buffer.
		 */
		newbuf_wp = newbuf;	
		bp = mp;
		while (len && bp ) {
			ASSERT(bp->b_wptr >= bp->b_rptr );
			n = min((int)(bp->b_wptr - bp->b_rptr), len);
			bcopy( bp->b_rptr, newbuf_wp, n );
			newbuf_wp += n;
			len -= n;
			bp->b_rptr += n;
			if (bp->b_wptr != bp->b_rptr)
				break;
			bcont = bp->b_cont;
			if (bp != mp)
				freeb(bp); /* free if not the head block */
			bp = bcont;
		} /* end while */
		mp->b_cont = bp;

		/* Release previous data buffer is it was seperate
		 * from the data header.
		 */
		if ((unsigned char *)dbp + sizeof(struct dbinfo)
				!= dbp->db_base) {
			if (dbp->db_frtnp == NULL) {
				kmem_free(dbp->db_base, ((char *)(dbp->db_lim) 
					- (char *)(dbp->db_base)));
				Strcount -= (dbp->db_lim - dbp->db_base);
			}
			else {
				if (dbp->db_frtnp->free_func)
					(*dbp->db_frtnp->free_func)(dbp->db_frtnp->free_arg);
				dbp->db_frtnp = NULL;
			}
		}
		
		/*
		 * Update header information.
		 */
		s = splstr();
		mp->b_rptr = (unsigned char *)newbuf;
		mp->b_wptr = (unsigned char *)newbuf_wp;
		dbp->db_base = (unsigned char *)newbuf;
		dbp->db_lim = (unsigned char *)newbuf + dbp->db_size;
		splx(s);
		return(1);
	}

} 





/*
 * Trim bytes from message
 *  len > 0, trim from head
 *  len < 0, trim from tail
 * Returns 1 on success, 0 on failure.
 */
int
adjmsg(mp, len)
	mblk_t *mp;
	register int len;
{
	register mblk_t *bp;
	register n;
	int fromhead;

	ASSERT(mp != NULL);

	fromhead = 1;
	if (len < 0) {
		fromhead = 0;
		len = -len;
	}
	if (xmsgsize(mp, fromhead) < len)
		return(0);

	if (fromhead) {
		bp = mp;
		while (len) {
			n = min(bp->b_wptr - bp->b_rptr, len);
			bp->b_rptr += n;
			len -= n;
			bp = bp->b_cont;
		}
	} else {
		register mblk_t *endbp = NULL;

		while (len) {
			for (bp = mp; bp->b_cont != endbp; bp = bp->b_cont)
				;
			n = min(bp->b_wptr - bp->b_rptr, len);
			bp->b_wptr -= n;
			len -= n;
			endbp = bp;
		}
	}
	return(1);
}

/*
 * get number of data bytes in message
 */
int
msgdsize(bp)
	register mblk_t *bp;
{
	register int count = 0;
	register int s;

	s = splstr();
	for (; bp; bp = bp->b_cont)
		if (bp->b_datap->db_type == M_DATA) {
			ASSERT(bp->b_wptr >= bp->b_rptr);
			count += bp->b_wptr - bp->b_rptr;
		}
	splx(s);
	return(count);
}

/*
 * Get a message off head of queue
 *
 * If queue has no buffers then mark queue
 * with QWANTR. (queue wants to be read by
 * someone when data becomes available)
 *
 * If there is something to take off then do so.
 * If queue falls below hi water mark turn off QFULL
 * flag.  Decrement weighted count of queue.
 * Also turn off QWANTR because queue is being read.
 *
 * The queue count is maintained on a per-band basis.
 * Priority band 0 (normal messages) uses q_count,
 * q_lowat, etc.  Non-zero priority bands use the
 * fields in their respective qband structures
 * (qb_count, qb_lowat, etc.)  All messages appear
 * on the same list, linked via their b_next pointers.
 * q_first is the head of the list.  q_count does
 * not reflect the size of all the messages on the
 * queue.  It only reflects those messages in the
 * normal band of flow.  The one exception to this
 * deals with high priority messages.  They are in
 * their own conceptual "band", but are accounted
 * against q_count.
 *
 * If queue count is below the lo water mark and QWANTW
 * is set, enable the closest backq which has a service 
 * procedure and turn off the QWANTW flag.
 *
 * getq could be built on top of rmvq, but isn't because
 * of performance considerations.
 */
mblk_t *
getq(q)
	register queue_t *q;
{
	register mblk_t *bp;
	register mblk_t *tmp;
	register int s;
	register qband_t *qbp = (qband_t *) NULL;
	int backenable = 0;
	int wantw = 0;

	ASSERT(q);
	s = splstr();
	if (!(bp = q->q_first)) {
		q->q_flag |= QWANTR;
		backenable = 1;		/* we might backenable */
		wantw = q->q_flag & QWANTW;
	} else {
		if (bp->b_flag & MSGNOGET) {	/* hack hack hack */
			while (bp && (bp->b_flag & MSGNOGET))
				bp = bp->b_next;
			if (bp)
				rmvq(q, bp);
			splx(s);
			return (bp);
		}
		if (!(q->q_first = bp->b_next))
			q->q_last = NULL;
		else
			q->q_first->b_prev = NULL;

		if (bp->b_band == 0) {
			wantw = q->q_flag & QWANTW;
			for (tmp = bp; tmp; tmp = tmp->b_cont)
				q->q_count -= (tmp->b_wptr - tmp->b_rptr);
			if (q->q_count < q->q_hiwat)
				q->q_flag &= ~QFULL;
			if (q->q_count <= q->q_lowat)
				backenable = 1;
		} else {
			register int i;

			ASSERT(bp->b_band <= q->q_nband);
			ASSERT(q->q_bandp != NULL);
			qbp = q->q_bandp;
			i = bp->b_band;
			while (--i > 0)
				qbp = qbp->qb_next;
			if (qbp->qb_first == qbp->qb_last) {
				qbp->qb_first = NULL;
				qbp->qb_last = NULL;
			} else {
				qbp->qb_first = bp->b_next;
			}
			wantw = qbp->qb_flag & QB_WANTW;
			for (tmp = bp; tmp; tmp = tmp->b_cont)
				qbp->qb_count -= (tmp->b_wptr - tmp->b_rptr);
			if (qbp->qb_count < qbp->qb_hiwat)
				qbp->qb_flag &= ~QB_FULL;
			if (qbp->qb_count <= qbp->qb_lowat)
				backenable = 1;
		}
		q->q_flag &= ~QWANTR;
		bp->b_next = NULL;
		bp->b_prev = NULL;
	}
	if (backenable && wantw) {
		if (bp && bp->b_band != 0)
			qbp->qb_flag &= ~QB_WANTW;
		else
			q->q_flag &= ~QWANTW;
		/* find nearest back queue with service proc */
		for (q = backq(q); q && !q->q_qinfo->qi_srvp; q = backq(q))
			;
		if (q) {
			qenable(q);
			setqback(q, bp ? bp->b_band : 0);
		}
	}
	splx(s);
	return(bp);
}

/*
 * Remove a message from a queue.  The queue count and other
 * flow control parameters are adjusted and the back queue
 * enabled if necessary.
 */
void
rmvq(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register int s;
	register mblk_t *tmp;
	register int i;
	register qband_t *qbp = NULL;
	int backenable = 0;
	int wantw = 0;

	ASSERT(q);
	ASSERT(mp);
	s = splstr();
	ASSERT(mp->b_band <= q->q_nband);
	if (mp->b_band != 0) {		/* Adjust band pointers */
		ASSERT(q->q_bandp != NULL);
		qbp = q->q_bandp;
		i = mp->b_band;
		while (--i > 0)
			qbp = qbp->qb_next;
		if (mp == qbp->qb_first) {
			if (mp->b_next && mp->b_band == mp->b_next->b_band)
				qbp->qb_first = mp->b_next;
			else
				qbp->qb_first = NULL;
		}
		if (mp == qbp->qb_last) {
			if (mp->b_prev && mp->b_band == mp->b_prev->b_band)
				qbp->qb_last = mp->b_prev;
			else
				qbp->qb_last = NULL;
		}
	}

	/*
	 * Remove the message from the list.
	 */
	if (mp->b_prev)
		mp->b_prev->b_next = mp->b_next;
	else
		q->q_first = mp->b_next;
	if (mp->b_next)
		mp->b_next->b_prev = mp->b_prev;
	else
		q->q_last = mp->b_prev;
	mp->b_next = NULL;
	mp->b_prev = NULL;

	if (mp->b_band == 0) {		/* Perform q_count accounting */
		wantw = q->q_flag & QWANTW;
		for (tmp = mp; tmp; tmp = tmp->b_cont)
			q->q_count -= (tmp->b_wptr - tmp->b_rptr);
		if (q->q_count < q->q_hiwat)
			q->q_flag &= ~QFULL;
		if (q->q_count <= q->q_lowat)
			backenable = 1;
	} else {			/* Perform qb_count accounting */
		wantw = qbp->qb_flag & QB_WANTW;
		for (tmp = mp; tmp; tmp = tmp->b_cont)
			qbp->qb_count -= (tmp->b_wptr - tmp->b_rptr);
		if (qbp->qb_count < qbp->qb_hiwat)
			qbp->qb_flag &= ~QB_FULL;
		if (qbp->qb_count <= qbp->qb_lowat)
			backenable = 1;
	}

	if (backenable && wantw) {
		if (mp->b_band != 0)
			qbp->qb_flag &= ~QB_WANTW;
		else
			q->q_flag &= ~QWANTW;
		/* find nearest back queue with service proc */
		for (q = backq(q); q && !q->q_qinfo->qi_srvp; q = backq(q))
			;
		if (q) {
			qenable(q);
			setqback(q, mp->b_band);
		}
	}
	splx(s);
}

/*
 * Empty a queue.  
 * If flag is set, remove all messages.  Otherwise, remove
 * only non-control messages.  If queue falls below its low
 * water mark, and QWANTW is set , enable the nearest upstream
 * service procedure.
 */
void
flushq(q, flag)
	register queue_t *q;
	int flag;
{
	register mblk_t *mp, *nmp;
	register int s;
	register qband_t *qbp;
	int backenable = 0;
	unsigned char bpri;
	extern unsigned char qbf[];

	ASSERT(q);
	s = splstr();
	mp = q->q_first;
	q->q_first = NULL;
	q->q_last = NULL;
	q->q_count = 0;
	for (qbp = q->q_bandp; qbp; qbp = qbp->qb_next) {
		qbp->qb_first = NULL;
		qbp->qb_last = NULL;
		qbp->qb_count = 0;
		qbp->qb_flag &= ~QB_FULL;
	}
	q->q_flag &= ~QFULL;
	while (mp) {
		nmp = mp->b_next;
		if (flag || datamsg(mp->b_datap->db_type))
			freemsg(mp);
		else
			putq(q, mp);
		mp = nmp;
	}
	bzero(qbf, NBAND);
	bpri = 1;
	for (qbp = q->q_bandp; qbp; qbp = qbp->qb_next) {
		if ((qbp->qb_flag & QB_WANTW) && (qbp->qb_count <= qbp->qb_lowat)) {
			qbp->qb_flag &= ~QB_WANTW;
			backenable = 1;
			qbf[bpri] = 1;
		}
		bpri++;
	}
	if ((q->q_flag & QWANTW) && (q->q_count <= q->q_lowat)) {
		q->q_flag &= ~QWANTW;
		backenable = 1;
		qbf[0] = 1;
	}

	/*
	 * If any band can now be written to, and there is a writer
	 * for that band, then backenable the closest service procedure.
	 */
	if (backenable) {
		/* find nearest back queue with service proc */
		for (q = backq(q); q && !q->q_qinfo->qi_srvp; q = backq(q))
			;
		if (q) {
			qenable(q);
			for (bpri = q->q_nband; bpri; bpri--)
				if (qbf[bpri])
					setqback(q, bpri);
			if (qbf[0])
				setqback(q, 0);
		}
	}
	splx(s);
}

/*
 * Flush the queue of messages of the given priority band.
 * There is some duplication of code between flushq and flushband.
 * This is because we want to optimize the code as much as possible.
 * The assumption is that there will be more messages in the normal
 * (priority 0) band than in any other.
 */
void
flushband(q, pri, flag)
	register queue_t *q;
	unsigned char pri;
	int flag;
{
	register int s;
	register mblk_t *mp;
	register mblk_t *nmp;
	register mblk_t *last;
	register qband_t *qbp;
	int band;

	ASSERT(q);
	s = splstr();
	if (pri > q->q_nband) {
		splx(s);
		return;
	}
	if (pri == 0) {
		mp = q->q_first;
		q->q_first = NULL;
		q->q_last = NULL;
		q->q_count = 0;
		for (qbp = q->q_bandp; qbp; qbp = qbp->qb_next) {
			qbp->qb_first = NULL;
			qbp->qb_last = NULL;
			qbp->qb_count = 0;
			qbp->qb_flag &= ~QB_FULL;
		}
		q->q_flag &= ~QFULL;
		while (mp) {
			nmp = mp->b_next;
			if ((mp->b_band == 0) &&
			    (mp->b_datap->db_type < QPCTL) &&
			    (flag || datamsg(mp->b_datap->db_type)))
				freemsg(mp);
			else
				putq(q, mp);
			mp = nmp;
		}
		if ((q->q_flag & QWANTW) && (q->q_count <= q->q_lowat)) {
			/* find nearest back queue with service proc */
			q->q_flag &= ~QWANTW;
			for (q = backq(q); q && !q->q_qinfo->qi_srvp; q = backq(q))
				;
			if (q) {
				qenable(q);
				setqback(q, pri);
			}
		}
	} else {	/* pri != 0 */
		ASSERT(q->q_bandp != NULL);
		band = pri;
		qbp = q->q_bandp;
		while (--band > 0)
			qbp = qbp->qb_next;
		mp = qbp->qb_first;
		if (mp == NULL) {
			splx(s);
			return;
		}
		last = qbp->qb_last;
		if (mp == last)		/* only message in band */
			last = mp->b_next;
		while (mp != last) {
			nmp = mp->b_next;
			if (mp->b_band == pri) {
				if (flag || datamsg(mp->b_datap->db_type)) {
					rmvq(q, mp);
					freemsg(mp);
				}
			}
			mp = nmp;
		}
		if (mp && mp->b_band == pri) {
			if (flag || datamsg(mp->b_datap->db_type)) {
				rmvq(q, mp);
				freemsg(mp);
			}
		}
	}
	splx(s);
}

/*
 * Return 1 if the queue is not full.  If the queue is full, return
 * 0 (may not put message) and set QWANTW flag (caller wants to write
 * to the queue).
 */
int
canput(q)
	register queue_t *q;
{
	register int s;

	if (!q)
		return(0);
	s = splstr();
	while (q->q_next && !q->q_qinfo->qi_srvp)
		q = q->q_next;
	if (q->q_flag & QFULL) {
		q->q_flag |= QWANTW;
		splx(s);
		return(0);
	}
	splx(s);
	return(1);
}

/*
 * This is the new canput for use with priority bands.  Return 1 if the
 * band is not full.  If the band is full, return 0 (may not put message)
 * and set QWANTW(QB_WANTW) flag for zero(non-zero) band (caller wants to
 * write to the queue).
 */
int
bcanput(q, pri)
	register queue_t *q;
	unsigned char pri;
{
	register int s;
	register qband_t *qbp;

	if (!q)
		return(0);
	s = splstr();
	while (q->q_next && !q->q_qinfo->qi_srvp)
		q = q->q_next;
	if (pri == 0) {
		if (q->q_flag & QFULL) {
			q->q_flag |= QWANTW;
			splx(s);
			return(0);
		}
	} else {	/* pri != 0 */
		if (pri > q->q_nband) {
			/*
			 * No band exists yet, so return success.
			 */
			splx(s);
			return(1);
		}
		qbp = q->q_bandp;
		while (--pri)
			qbp = qbp->qb_next;
		if (qbp->qb_flag & QB_FULL) {
			qbp->qb_flag |= QB_WANTW;
			splx(s);
			return(0);
		}
	}
	splx(s);
	return(1);
}

/*
 * Put a message on a queue.  
 *
 * Messages are enqueued on a priority basis.  The priority classes
 * are HIGH PRIORITY (type >= QPCTL), PRIORITY (type < QPCTL && band > 0),
 * and B_NORMAL (type < QPCTL && band == 0). 
 *
 * Add appropriate weighted data block sizes to queue count.
 * If queue hits high water mark then set QFULL flag.
 *
 * If QNOENAB is not set (putq is allowed to enable the queue),
 * enable the queue only if the message is PRIORITY,
 * or the QWANTR flag is set (indicating that the service procedure 
 * is ready to read the queue.  This implies that a service 
 * procedure must NEVER put a priority message back on its own
 * queue, as this would result in an infinite loop (!).
 */
int
putq(q, bp)
	register queue_t *q;
	register mblk_t *bp;
{
	register int s;
	register mblk_t *tmp;
	register qband_t *qbp = NULL;
	int mcls = (int)queclass(bp);

	ASSERT(q && bp);
	s = splstr();

	/*
	 * Make sanity checks and if qband structure is not yet
	 * allocated, do so.
	 */
	if (mcls == QPCTL) {
		if (bp->b_band != 0)
			bp->b_band = 0;		/* force to be correct */
	} else if (bp->b_band != 0) {
		register int i;
		qband_t **qbpp;

		if (bp->b_band > q->q_nband) {

			/*
			 * The qband structure for this priority band is
			 * not on the queue yet, so we have to allocate
			 * one on the fly.  It would be wasteful to
			 * associate the qband structures with every
			 * queue when the queues are allocated.  This is
			 * because most queues will only need the normal
			 * band of flow which can be described entirely
			 * by the queue itself.
			 */
			qbpp = &q->q_bandp;
			while (*qbpp)
				qbpp = &(*qbpp)->qb_next;
			while (bp->b_band > q->q_nband) {
				if ((*qbpp = allocband()) == NULL) {
					splx(s);
					return(0);
				}
				(*qbpp)->qb_hiwat = q->q_hiwat;
				(*qbpp)->qb_lowat = q->q_lowat;
				q->q_nband++;
				qbpp = &(*qbpp)->qb_next;
			}
		}
		qbp = q->q_bandp;
		i = bp->b_band;
		while (--i)
			qbp = qbp->qb_next;
	}

	/*
	 * If queue is empty, add the message and initialize the pointers.
	 * Otherwise, adjust message pointers and queue pointers based on
	 * the type of the message and where it belongs on the queue.  Some
	 * code is duplicated to minimize the number of conditionals and
	 * hopefully minimize the amount of time this routine takes.
	 */
	if (!q->q_first) {
		bp->b_next = NULL;
		bp->b_prev = NULL;
		q->q_first = bp;
		q->q_last = bp;
		if (qbp) {
			qbp->qb_first = bp;
			qbp->qb_last = bp;
		}
	} else if (!qbp) {	/* bp->b_band == 0 */

		/* 
		 * If queue class of message is less than or equal to
		 * that of the last one on the queue, tack on to the end.
		 */
		tmp = q->q_last;
		if (mcls <= (int)queclass(tmp)) {
			bp->b_next = NULL;
			bp->b_prev = tmp;
			tmp->b_next = bp;
			q->q_last = bp;
		} else {
			tmp = q->q_first;
			while ((int)queclass(tmp) >= mcls)
				tmp = tmp->b_next;
			ASSERT(tmp != NULL);

			/*
			 * Insert bp before tmp.
			 */
			bp->b_next = tmp;
			bp->b_prev = tmp->b_prev;
			if (tmp->b_prev)
				tmp->b_prev->b_next = bp;
			else
				q->q_first = bp;
			tmp->b_prev = bp;
		}
	} else {		/* bp->b_band != 0 */
		if (qbp->qb_first) {
			ASSERT(qbp->qb_last != NULL);
			tmp = qbp->qb_last;

			/*
			 * Insert bp after the last message in this band.
			 */
			bp->b_next = tmp->b_next;
			if (tmp->b_next)
				tmp->b_next->b_prev = bp;
			else
				q->q_last = bp;
			bp->b_prev = tmp;
			tmp->b_next = bp;
		} else {
			tmp = q->q_last;
			if ((mcls < (int)queclass(tmp)) ||
			    (bp->b_band <= tmp->b_band)) {

				/*
				 * Tack bp on end of queue.
				 */
				bp->b_next = NULL;
				bp->b_prev = tmp;
				tmp->b_next = bp;
				q->q_last = bp;
			} else {
				tmp = q->q_first;
				while (tmp->b_datap->db_type >= QPCTL)
					tmp = tmp->b_next;
				while (tmp->b_band >= bp->b_band)
					tmp = tmp->b_next;
				ASSERT(tmp != NULL);

				/*
				 * Insert bp before tmp.
				 */
				bp->b_next = tmp;
				bp->b_prev = tmp->b_prev;
				if (tmp->b_prev)
					tmp->b_prev->b_next = bp;
				else
					q->q_first = bp;
				tmp->b_prev = bp;
			}
			qbp->qb_first = bp;
		}
		qbp->qb_last = bp;
	}

	if (qbp) {
		for (tmp = bp; tmp; tmp = tmp->b_cont)
			qbp->qb_count += (tmp->b_wptr - tmp->b_rptr);
		if (qbp->qb_count >= qbp->qb_hiwat)
			qbp->qb_flag |= QB_FULL;
	} else {
		for (tmp = bp; tmp; tmp = tmp->b_cont)
			q->q_count += (tmp->b_wptr - tmp->b_rptr);
		if (q->q_count >= q->q_hiwat)
			q->q_flag |= QFULL;
	}
	if ((mcls > QNORM) || (canenable(q) && (q->q_flag & QWANTR)))
		qenable(q);
	splx(s);
	return(1);
}

/*
 * Put stuff back at beginning of Q according to priority order.
 * See comment on putq above for details.
 */
int
putbq(q, bp)
	register queue_t *q;
	register mblk_t *bp;
{
	register s;
	register mblk_t *tmp;
	register qband_t *qbp = NULL;
	int mcls = (int)queclass(bp);

	ASSERT(q && bp);
	ASSERT(bp->b_next == NULL);
	s = splstr();

	/*
	 * Make sanity checks and if qband structure is not yet
	 * allocated, do so.
	 */
	if (mcls == QPCTL) {
		if (bp->b_band != 0)
			bp->b_band = 0;		/* force to be correct */
	} else if (bp->b_band != 0) {
		register int i;
		qband_t **qbpp;

		if (bp->b_band > q->q_nband) {
			qbpp = &q->q_bandp;
			while (*qbpp)
				qbpp = &(*qbpp)->qb_next;
			while (bp->b_band > q->q_nband) {
				if ((*qbpp = allocband()) == NULL) {
					splx(s);
					return(0);
				}
				(*qbpp)->qb_hiwat = q->q_hiwat;
				(*qbpp)->qb_lowat = q->q_lowat;
				q->q_nband++;
				qbpp = &(*qbpp)->qb_next;
			}
		}
		qbp = q->q_bandp;
		i = bp->b_band;
		while (--i)
			qbp = qbp->qb_next;
	}

	/* 
	 * If queue is empty or if message is high priority,
	 * place on the front of the queue.
	 */
	tmp = q->q_first;
	if ((!tmp) || (mcls == QPCTL)) {
		bp->b_next = tmp;
		if (tmp)
			tmp->b_prev = bp;
		else
			q->q_last = bp;
		q->q_first = bp;
		bp->b_prev = NULL;
		if (qbp) {
			qbp->qb_first = bp;
			qbp->qb_last = bp;
		}
	} else if (qbp) {	/* bp->b_band != 0 */
		tmp = qbp->qb_first;
		if (tmp) {

			/*
			 * Insert bp before the first message in this band.
			 */
			bp->b_next = tmp;
			bp->b_prev = tmp->b_prev;
			if (tmp->b_prev)
				tmp->b_prev->b_next = bp;
			else
				q->q_first = bp;
			tmp->b_prev = bp;
		} else {
			tmp = q->q_last;
			if ((mcls < (int)queclass(tmp)) ||
			    (bp->b_band < tmp->b_band)) {

				/*
				 * Tack bp on end of queue.
				 */
				bp->b_next = NULL;
				bp->b_prev = tmp;
				tmp->b_next = bp;
				q->q_last = bp;
			} else {
				tmp = q->q_first;
				while (tmp->b_datap->db_type >= QPCTL)
					tmp = tmp->b_next;
				while (tmp->b_band > bp->b_band)
					tmp = tmp->b_next;
				ASSERT(tmp != NULL);

				/*
				 * Insert bp before tmp.
				 */
				bp->b_next = tmp;
				bp->b_prev = tmp->b_prev;
				if (tmp->b_prev)
					tmp->b_prev->b_next = bp;
				else
					q->q_first = bp;
				tmp->b_prev = bp;
			}
			qbp->qb_last = bp;
		}
		qbp->qb_first = bp;
	} else {		/* bp->b_band == 0 && !QPCTL */

		/*
		 * If the queue class or band is less than that of the last
		 * message on the queue, tack bp on the end of the queue.
		 */
		tmp = q->q_last;
		if ((mcls < (int)queclass(tmp)) || (bp->b_band < tmp->b_band)) {
			bp->b_next = NULL;
			bp->b_prev = tmp;
			tmp->b_next = bp;
			q->q_last = bp;
		} else {
			tmp = q->q_first;
			while (tmp->b_datap->db_type >= QPCTL)
				tmp = tmp->b_next;
			while (tmp->b_band > bp->b_band)
				tmp = tmp->b_next;
			ASSERT(tmp != NULL);

			/*
			 * Insert bp before tmp.
			 */
			bp->b_next = tmp;
			bp->b_prev = tmp->b_prev;
			if (tmp->b_prev)
				tmp->b_prev->b_next = bp;
			else
				q->q_first = bp;
			tmp->b_prev = bp;
		}
	}

	if (qbp) {
		for (tmp = bp; tmp; tmp = tmp->b_cont)
			qbp->qb_count += (tmp->b_wptr - tmp->b_rptr);
		if (qbp->qb_count >= qbp->qb_hiwat)
			qbp->qb_flag |= QB_FULL;
	} else {
		for (tmp = bp; tmp; tmp = tmp->b_cont)
			q->q_count += (tmp->b_wptr - tmp->b_rptr);
		if (q->q_count >= q->q_hiwat)
			q->q_flag |= QFULL;
	}
	if ((mcls > QNORM) || (canenable(q) && (q->q_flag & QWANTR)))
		qenable(q);
	splx(s);
	return(1);
}

/*
 * Insert a message before an existing message on the queue.  If the
 * existing message is NULL, the new messages is placed on the end of
 * the queue.  The queue class of the new message is ignored.  However,
 * the priority band of the new message must adhere to the following
 * ordering:
 *
 *	emp->b_prev->b_band >= mp->b_band >= emp->b_band.
 *
 * All flow control parameters are updated.
 */
int
insq(q, emp, mp)
	register queue_t *q;
	register mblk_t *emp;
	register mblk_t *mp;
{
	register mblk_t *tmp;
	register int s;
	register qband_t *qbp = NULL;
	int mcls = (int)queclass(mp);

	s = splstr();
	if (mcls == QPCTL) {
		if (mp->b_band != 0)
			mp->b_band = 0;		/* force to be correct */
		if (emp && emp->b_prev &&
		    (emp->b_prev->b_datap->db_type < QPCTL))
			goto badord;
	}
	if (emp) {
		if (((mcls == QNORM) && (mp->b_band < emp->b_band)) ||
		    (emp->b_prev && (emp->b_prev->b_datap->db_type < QPCTL) &&
		    (emp->b_prev->b_band < mp->b_band))) {
			goto badord;
		}
	} else {
		tmp = q->q_last;
		if (tmp && (mcls == QNORM) && (mp->b_band > tmp->b_band)) {
badord:
			splx(s);
			cmn_err(CE_WARN, "insq: attempt to insert message out of order on q %x\n", q);
			return(0);
		}
	}

	if (mp->b_band != 0) {
		register int i;
		qband_t **qbpp;

		if (mp->b_band > q->q_nband) {
			qbpp = &q->q_bandp;
			while (*qbpp)
				qbpp = &(*qbpp)->qb_next;
			while (mp->b_band > q->q_nband) {
				if ((*qbpp = allocband()) == NULL) {
					splx(s);
					return(0);
				}
				(*qbpp)->qb_hiwat = q->q_hiwat;
				(*qbpp)->qb_lowat = q->q_lowat;
				q->q_nband++;
				qbpp = &(*qbpp)->qb_next;
			}
		}
		qbp = q->q_bandp;
		i = mp->b_band;
		while (--i)
			qbp = qbp->qb_next;
	}

	if (mp->b_next = emp) {
		if (mp->b_prev = emp->b_prev) 
			emp->b_prev->b_next = mp;
		else
			q->q_first = mp;
		emp->b_prev = mp;
	} else {
		if (mp->b_prev = q->q_last) 
			q->q_last->b_next = mp;
		else 
			q->q_first = mp;
		q->q_last = mp;
	}

	if (qbp) {	/* adjust qband pointers and count */
		if (!qbp->qb_first) {
			qbp->qb_first = mp;
			qbp->qb_last = mp;
		} else {
			if (qbp->qb_first == emp)
				qbp->qb_first = mp;
			else if (mp->b_next && (mp->b_next->b_band !=
			    mp->b_band))
				qbp->qb_last = mp;
		}
		for (tmp = mp; tmp; tmp = tmp->b_cont)
			qbp->qb_count += (tmp->b_wptr - tmp->b_rptr);
		if (qbp->qb_count >= qbp->qb_hiwat)
			qbp->qb_flag |= QB_FULL;
	} else {
		for (tmp = mp; tmp; tmp = tmp->b_cont)
			q->q_count += (tmp->b_wptr - tmp->b_rptr);
		if (q->q_count >= q->q_hiwat)
			q->q_flag |= QFULL;
	}
	if (canenable(q) && (q->q_flag & QWANTR))
		qenable(q);
	splx(s);
	return(1);
}

/*
 * Create and put a control message on queue.
 */
int
putctl(q, type)
	queue_t *q;
{
	register mblk_t *bp;

	if ((datamsg(type) && (type != M_DELAY)) || !(bp = allocb(0, BPRI_HI)))
		return(0);
	bp->b_datap->db_type = type;
	(*q->q_qinfo->qi_putp)(q, bp);
	return(1);
}

/*
 * Control message with a single-byte parameter
 */
int
putctl1(q, type, param)
	queue_t *q;
{
	register mblk_t *bp;

	if ((datamsg(type) && (type != M_DELAY)) || !(bp = allocb(1, BPRI_HI)))
		return(0);
	bp->b_datap->db_type = type;
	*bp->b_wptr++ = param;
	(*q->q_qinfo->qi_putp)(q, bp);
	return(1);
}

/*
 * Return the queue upstream from this one
 */
queue_t *
backq(q)
	register queue_t *q;
{
	ASSERT(q);

	q = OTHERQ(q);
	if (q->q_next) {
		q = q->q_next;
		return(OTHERQ(q));
	}
	return(NULL);
}



/*
 * Send a block back up the queue in reverse from this
 * one (e.g. to respond to ioctls)
 */
void
qreply(q, bp)
	register queue_t *q;
	mblk_t *bp;
{
	ASSERT(q && bp);

	q = OTHERQ(q);
	ASSERT(q->q_next);
	(*q->q_next->q_qinfo->qi_putp)(q->q_next, bp);
}

/*
 * Streams Queue Scheduling
 * 
 * Queues are enabled through qenable() when they have messages to 
 * process.  They are serviced by queuerun(), which runs each enabled
 * queue's service procedure.  The call to queuerun() is processor
 * dependent - the general principle is that it be run whenever a queue
 * is enabled but before returning to user level.  For system calls,
 * the function runqueues() is called if their action causes a queue
 * to be enabled.  For device interrupts, queuerun() should be
 * called before returning from the last level of interrupt.  Beyond
 * this, no timing assumptions should be made about queue scheduling.
 */

/*
 * Enable a queue: put it on list of those whose service procedures are
 * ready to run and set up the scheduling mechanism.
 */
void
qenable(q)
	register queue_t *q;
{
	register s;

	ASSERT(q);

	if (!q->q_qinfo->qi_srvp)
		return;

	s = splstr();
	/*
	 * Do not place on run queue if already enabled.
	 */
	if (q->q_flag & QENAB) {
		splx(s);
		return;
	}

	/*
	 * mark queue enabled and place on run list
	 */
	q->q_flag |= QENAB;

	if (!qhead)
		qhead = q;
	else
		qtail->q_link = q;
	qtail = q;
	q->q_link = NULL;

	/*
	 * set up scheduling mechanism
	 */
	setqsched();
	splx(s);
}

/* 
 * Return number of messages on queue
 */
int
qsize(qp)
	register queue_t *qp;
{
	register count = 0;
	register mblk_t *mp;
	register int s;

	ASSERT(qp);

	s = splstr();
	for (mp = qp->q_first; mp; mp = mp->b_next)
		count++;
	splx(s);
	return(count);
}

/*
 * noenable - set queue so that putq() will not enable it.
 * enableok - set queue so that putq() can enable it.
 */
void
noenable(q)
	queue_t *q;
{ 
	register int s;

	s = splstr();
	q->q_flag |= QNOENB;
	splx(s);
}

void
enableok(q)
	queue_t *q;
{
	register int s;

	s = splstr();
	q->q_flag &= ~QNOENB;
	splx(s);
}

/*
 * Given a name, return the module id.
 * Returns 0 on error.
 */
ushort
getmid(name)
	char *name;
{
	register struct cdevsw *cdp;
	register struct fmodsw *fmp;
	register struct module_info *mip;

	if (!name || *name == (char)0)
		return((ushort)0);
	for (cdp = cdevsw; cdp < &cdevsw[cdevcnt]; cdp++) {
		if (cdp->d_str) {
			mip = cdp->d_str->st_rdinit->qi_minfo;
			if (strcmp(name, mip->mi_idname) == 0)
				return(mip->mi_idnum);
		}
	}
	for (fmp = fmodsw; fmp < &fmodsw[fmodcnt]; fmp++) {
		if (strcmp(name, fmp->f_name) == 0) {
			mip = fmp->f_str->st_rdinit->qi_minfo;
			return(mip->mi_idnum);
		}
	}
	return((ushort)0);
}

/*
 * Given a module id, return the qadmin pointer.
 * Returns NULL on error.
 */
int
(*getadmin(mid))()
	ushort mid;
{
	register struct cdevsw *cdp;
	register struct fmodsw *fmp;
	register struct qinit *qip;

	if (mid == 0)
		return(NULL);
	for (cdp = cdevsw; cdp < &cdevsw[cdevcnt]; cdp++) {
		if (cdp->d_str) {
			qip = cdp->d_str->st_rdinit;
			if (mid == qip->qi_minfo->mi_idnum)
				return(qip->qi_qadmin);
		}
	}
	for (fmp = fmodsw; fmp < &fmodsw[fmodcnt]; fmp++) {
		qip = fmp->f_str->st_rdinit;
		if (mid == qip->qi_minfo->mi_idnum)
			return(qip->qi_qadmin);
	}
	return(NULL);
}

/*
 * Set queue fields.
 */
int
strqset(q, what, pri, val)
	register queue_t *q;
	qfields_t what;
	register unsigned char pri;
	long val;
{
	register int s;
	register qband_t *qbp = NULL;
	int error = 0;

	if (what >= QBAD)
		return (EINVAL);
	if (pri != 0) {
		register int i;
		qband_t **qbpp;

		s = splstr();
		if (pri > q->q_nband) {
			qbpp = &q->q_bandp;
			while (*qbpp)
				qbpp = &(*qbpp)->qb_next;
			while (pri > q->q_nband) {
				if ((*qbpp = allocband()) == NULL) {
					splx(s);
					return (EAGAIN);
				}
				(*qbpp)->qb_hiwat = q->q_hiwat;
				(*qbpp)->qb_lowat = q->q_lowat;
				q->q_nband++;
				qbpp = &(*qbpp)->qb_next;
			}
		}
		qbp = q->q_bandp;
		i = pri;
		while (--i)
			qbp = qbp->qb_next;
		splx(s);
	}
	switch (what) {
	case QHIWAT:
		if (qbp)
			qbp->qb_hiwat = (ulong)val;
		else
#ifdef _STYPES
			q->q_hiwat = (ushort)val;
#else
			q->q_hiwat = (ulong)val;
#endif
		break;

	case QLOWAT:
		if (qbp)
			qbp->qb_lowat = (ulong)val;
		else
#ifdef _STYPES
			q->q_lowat = (ushort)val;
#else
			q->q_lowat = (ulong)val;
#endif
		break;

	case QMAXPSZ:
		if (qbp)
			error = EINVAL;
		else
#ifdef _STYPES
			q->q_maxpsz = (short)val;
#else
			q->q_maxpsz = val;
#endif
		break;

	case QMINPSZ:
		if (qbp)
			error = EINVAL;
		else
#ifdef _STYPES
			q->q_minpsz = (short)val;
#else
			q->q_minpsz = val;
#endif
		break;

	case QCOUNT:
	case QFIRST:
	case QLAST:
	case QFLAG:
		error = EPERM;
		break;

	default:
		error = EINVAL;
		break;
	}
	return (error);
}

/*
 * Get queue fields.
 */
int
strqget(q, what, pri, valp)
	register queue_t *q;
	qfields_t what;
	register unsigned char pri;
	long *valp;
{
	register int s;
	register qband_t *qbp = NULL;
	int error = 0;

	if (what >= QBAD)
		return (EINVAL);
	if (pri != 0) {
		register int i;
		qband_t **qbpp;

		s = splstr();
		if (pri > q->q_nband) {
			qbpp = &q->q_bandp;
			while (*qbpp)
				qbpp = &(*qbpp)->qb_next;
			while (pri > q->q_nband) {
				if ((*qbpp = allocband()) == NULL) {
					splx(s);
					return (EAGAIN);
				}
				(*qbpp)->qb_hiwat = q->q_hiwat;
				(*qbpp)->qb_lowat = q->q_lowat;
				q->q_nband++;
				qbpp = &(*qbpp)->qb_next;
			}
		}
		qbp = q->q_bandp;
		i = pri;
		while (--i)
			qbp = qbp->qb_next;
		splx(s);
	}
	switch (what) {
	case QHIWAT:
		if (qbp)
			*(ulong *)valp = qbp->qb_hiwat;
		else
#ifdef _STYPES
			*(ushort *)valp = q->q_hiwat;
#else
			*(ulong *)valp = q->q_hiwat;
#endif
		break;

	case QLOWAT:
		if (qbp)
			*(ulong *)valp = qbp->qb_lowat;
		else
#ifdef _STYPES
			*(ushort *)valp = q->q_lowat;
#else
			*(ulong *)valp = q->q_lowat;
#endif
		break;

	case QMAXPSZ:
		if (qbp)
			error = EINVAL;
		else
#ifdef _STYPES
			*(short *)valp = q->q_maxpsz;
#else
			*(long *)valp = q->q_maxpsz;
#endif
		break;

	case QMINPSZ:
		if (qbp)
			error = EINVAL;
		else
#ifdef _STYPES
			*(short *)valp = q->q_minpsz;
#else
			*(long *)valp = q->q_minpsz;
#endif
		break;

	case QCOUNT:
		if (qbp)
			*(ulong *)valp = qbp->qb_count;
		else
#ifdef _STYPES
			*(ushort *)valp = q->q_count;
#else
			*(ulong *)valp = q->q_count;
#endif
		break;

	case QFIRST:
		if (qbp)
			*(mblk_t **)valp = qbp->qb_first;
		else
			*(mblk_t **)valp = q->q_first;
		break;

	case QLAST:
		if (qbp)
			*(mblk_t **)valp = qbp->qb_last;
		else
			*(mblk_t **)valp = q->q_last;
		break;

	case QFLAG:
		if (qbp)
			*(ulong *)valp = qbp->qb_flag;
		else
#ifdef _STYPES
			*(ushort *)valp = q->q_flag;
#else
			*(ulong *)valp = q->q_flag;
#endif
		break;

	default:
		error = EINVAL;
		break;
	}
	return (error);
}


