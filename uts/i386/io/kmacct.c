/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:kmacct.c	1.3"

#ifdef DEBUG

#include "sys/types.h"
#include "sys/param.h"
#include "sys/vnode.h"
#include "sys/signal.h"
#include "sys/dir.h"
#include "sys/file.h"
#include "sys/user.h"
#include "sys/cmn_err.h"
#include "sys/errno.h"
#include "sys/debug.h"
#include "sys/immu.h"
#include "sys/kmacct.h"

#define	FALSE	0
#define	TRUE	1

/*
 * Variables supplied by master.d/kmacct
 */

extern	int		nkmasym;	/* Number of entries in symbol table */
extern	int		kmadepth;	/* Depth of stack to trace */
extern	int		nkmabuf;	/* Number of buffer headers */
extern	kmasym_t	kmasymtab[];	/* KMA accounting symbol table */
extern	caddr_t		kmastack[];	/* Calling sequences */
extern	kmabuf_t	kmabuf[];	/* Buffer headers */

/*
 * Variables used by KMACCT but defined elsewhere.
 */

extern	int		kmacctflag;
extern	int		Km_pools[];

/*
 * Private variables for KMACCT.
 */

STATIC	int	kmacct_free	= 0;	/* First free entry in symbol table */
STATIC	int	kmasize;		/* Total size of accounting data */

/*
 * Calling sequence symbol table hashing.
 */

#define	KMACHASH	64	/* Number of hash buckets (must be 2^n) */
#define	KMACMASK	0x3F	/* Mask (KMACHASH - 1) */

STATIC	kmasym_t	*hashlist[KMACHASH];

/*
 * Buffer header hashing.
 */

#define	KMBFNHASH	256	/* Number of hash buckets (must be 2^n) */
#define	KMBFMASK	0xFF	/* Mask (KMBFNHASH - 1) */

STATIC	kmabuf_t	bufhash[KMBFNHASH];
STATIC	kmabuf_t	*freebuflist = (kmabuf_t *) NULL;

/*
 * Buffers from 4.0 KMEM are aligned on at least 16 byte boundaries,
 * with addresses of the form 0xAAABBCC0 (the leading AAA is usually the same
 * for all  buffers).  We hash on the low order eight bits of the sum
 * of BB and CC; for most groups of addresses, this ought to give a
 * reasonable spread among the hash buckets.
 */

#define	KMBFHASH(addr)	(int) ((((uint) (addr) >> 4) + \
				((uint) (addr) >> 12)) & KMBFMASK)

/*
 * Return a buffer header to the freelist.
 */

#define	KMBFFREE(bp)	(bp)->next = freebuflist; \
			freebuflist = (bp);

/*
 * KMACCT is a pseudo-device driver (flags "sc" in master.d file).
 * Only init(), read() and ioctl() currently do anything useful.
 */

kmacinit()
{
	register int		i, j;
	register kmabuf_t	*bp;
	register kmasym_t	*kp, **hp;
	register caddr_t	*ksp;

	/*
	 * Put all the buffer headers onto the freelist.
	 */

	for (i = 0, bp = kmabuf; i < nkmabuf; i++, bp++) {
		KMBFFREE(bp);
	}

	/*
	 * Initialize the buffer header hash list.
	 */

	for (i = 0, bp = bufhash; i < KMBFNHASH; i++, bp++) {
		bp->next = bp;
		bp->prev = bp;
	}

	/*
	 * Check for invalid settings of SDEPTH tunable parameter.
	 */

	if (kmadepth > MAXDEPTH) {
		cmn_err(CE_WARN,
			"KMACCT: SDEPTH of %d too large; reset to %d\n",
			kmadepth, MAXDEPTH);
		kmadepth = MAXDEPTH;
	}

	/*
	 * Reset the symbol table hash buckets.
	 */

	for (i = 0, hp = hashlist; i < KMACHASH; i++, hp++) {
		*hp = (kmasym_t *) NULL;
	}

	/*
	 * Zero all symbol table entries.
	 */

	for (i = 0, kp = kmasymtab; i < nkmasym; i++, kp++) {
		kp->next = (kmasym_t *) NULL;
		kp->pc = (caddr_t *) NULL;
		for (j = 0; j < KMSIZES; j++) {
			kp->reqa[j] = 0;
			kp->reqf[j] = 0;
		}
	}

	/*
	 * Reset calling sequence lists.
	 */

	for (i = 0, j = nkmasym * kmadepth, ksp = kmastack; i < j; i++)
		*ksp++ = 0;

	/*
	 * Calculate amount of accounting data that will be read by
	 * user level process.
	 */

	kmasize = sizeof(kmasym_t) * nkmasym +
		sizeof(caddr_t) * kmadepth * nkmasym;

	return 0;
}

kmacread()
/*
 * If the request is for the correct amount of data (see 
 * kmacioctl(KMACCT_SIZE)), return data in the following format:
 * 	kmasymtab entries
 * 	kmastack entries
 */
{
	char	*dest;

	/*
	 * Make sure proper amount of data was requested
	 */

	if (u.u_count != kmasize) {
		u.u_error = EINVAL;
		return -1;
	}

	dest = u.u_base;
	
	if (copyout((char *) kmasymtab, dest,
		(int)(sizeof(kmasym_t) * nkmasym)) == -1) {
		u.u_error = EFAULT;
		return -1;
	}

	dest += sizeof(kmasym_t) * nkmasym;

	if (copyout((char *) kmastack, dest,
		(int)(sizeof(caddr_t) * kmadepth * nkmasym))  == -1) {
		u.u_error = EFAULT;
		return -1;
	}

	u.u_rval1 = kmasize;
	return 0;
}

/* ARGSUSED */
kmacioctl(dev, cmd, arg1, flags)
	unsigned long dev;
	int cmd;
	int arg1;
	int flags;
/*
 * All of the user control of KMACCT is through ioctl calls.
 */
{
	register int		i, j;
	register kmasym_t	*kp, **hp;
	register caddr_t	*ksp;
	register kmabuf_t	*bp;

	switch (cmd) {

	case KMACCT_ON:		/* Enable KMA accounting. */

		kmacctflag = 1;
		break;

	case KMACCT_OFF:	/* Disable KMA accounting. */

		kmacctflag = 0;
		break;

	case KMACCT_ZERO:	/* Reset by zeroing the symbol table. */

		if (kmacctflag) {
			/*
			 * Cannot zero if accounting still active
			 */
			u.u_error = EBUSY;
			break;
		}

		/*
		 * Zero all buffer headers and put them on the freelist.
		 */

		for (i = 0, bp = kmabuf; i < nkmabuf; i++, bp++) {
			bp->kp = (kmasym_t *) NULL;
			bp->addr = (caddr_t *) NULL;
			KMBFFREE(bp);
		}

		/*
		 * Reset the buffer header hash buckets.
		 */

		for (i = 0, bp = bufhash; i < KMBFNHASH; i++, bp++) {
			bp->next = bp;
			bp->prev = bp;
		}

		/*
		 * Reset the symbol table hash buckets.
		 */

		for (i = 0, hp = hashlist; i < KMACHASH; i++, hp++)
			*hp = (kmasym_t *) NULL;

		/*
		 * Zero all symbol table entries.
		 */

		for (i = 0, kp = kmasymtab; i < kmacct_free; i++, kp++) {
			kp->next = (kmasym_t *) NULL;
			kp->pc = (caddr_t *) NULL;
			for (j = 0; j < KMSIZES; j++) {
				kp->reqa[j] = 0;
				kp->reqf[j] = 0;
			}
		}

		kmacct_free = 0;

		/*
		 * Reset calling sequence lists.
		 */

		for (i = 0, j = nkmasym * kmadepth, ksp = kmastack; i < j; i++)
			*ksp++ = 0;

		break;

	case KMACCT_SIZE:	/* Return the size of kmacct data. */

		u.u_rval1 = kmasize;
		break;

	case KMACCT_NDICT:	/* Return the number of symbol table entries. */

		u.u_rval1 = nkmasym;
		break;

	case KMACCT_DEPTH:	/* Return the depth of the trace. */

		u.u_rval1 = kmadepth;
		break;

	case KMACCT_STATE:	/* Return current state (on or off) */

		u.u_rval1 = kmacctflag;
		break;

	default:	/* Any other ioctl() cmd is an error. */

		u.u_error = EINVAL;
		break;
	}

	return 0;
}

 STATIC kmasym_t *
getsymentry(pcstack)
	caddr_t	pcstack[];
{
	/*
	 * Search the symbol table, looking for a structure
	 * with the same calling sequence as pc.  If one is
	 * not found (and the symbol table is not full) create
	 * a new entry with that sequence.
	 */

	register int		match, i, hl = 0;
	register caddr_t	*pc1, *pc2;
	register kmasym_t	*kp, *trail;

	/*
	 * The hashing index is the low order 6 bits (0-63) of
	 * the sum of the pc stack.
	 */

	pc1 = pcstack;

	for (i = 0; i < kmadepth; i++) 
		hl += (int) *pc1++;

	hl &= KMACMASK;
	kp = hashlist[hl];
	trail = (kmasym_t *) &hashlist[hl];

	while (kp != (kmasym_t *) NULL) {
		pc1 = pcstack;
		pc2 = kp->pc;
		match = TRUE;
		for (i = 0; i < kmadepth; i++) {
			if (*pc1++ != *pc2++) {
				match = FALSE;
				break;
			}
		}
		if (match)
			return(kp);

		trail = kp;
		kp = kp->next;
	}

	if (kmacct_free >= nkmasym)
		return ((kmasym_t *) NULL);

	kp = &kmasymtab[kmacct_free];
	kp->pc = pc1 = &kmastack[kmacct_free*kmadepth];
	pc2 = pcstack;
	for (i = 0; i < kmadepth; i++)
		*pc1++ = *pc2++;

	/*
	 * Put this on the end of the list, hoping it is used less
	 * than the items already on the list.
	 */

	trail->next = kp;
	kp->next = (kmasym_t *) NULL;
	++kmacct_free;
	return(kp);
}

/*
 * The following routines and macros are machine dependent.  They 
 * backtrack through the stack, collecting return PC's.
 */

 asm int
getfp()
{
	/* MOVW	%fp, %r0 */
	movl %ebp,%eax
}

 asm int 
getsp()
{
	/* MOVW	%sp, %r0 */
	movl %esp,%eax
}

/*
 * Getcaller(num, pcstack) backtracks through the stack to retrieve
 * the previous NUM calling routines; PCSTACK is a pointer to an
 * array of at least NUM entries where the addresses are to be
 * placed.  The first address is always the routine that called
 * getcaller().  This code is taken from debug/trace.c.
 * There is no guarantee that all NUM entries belong in the same
 * trace, and there is no protection against stack underflow.
 */

 STATIC void
getcaller(num, pcstack)
	int	num;
	caddr_t	pcstack[];
{
	register unsigned int *fp = (unsigned int *) getfp();
	register int	sp	= getsp();
	register caddr_t *pc	= pcstack;	
	register int	i;
	register unsigned int	*oldpcptr; 

	if ((fp < (unsigned int *)u.u_stack) && (fp > (unsigned int *)&u.u_stack[KSTKSZ]))
	{
		cmn_err(CE_WARN, "Initial Frame pointer out of range\n");
		return ;
	}


	if ((*fp < (unsigned int)&u.u_stack) && (*fp > (unsigned int)&u.u_stack[KSTKSZ]))
	{
		cmn_err(CE_WARN, "Initial Frame pointer value out of range\n");
		return ;
	}

	for (i = 0; i < num; i++)
	{
		oldpcptr = (unsigned int *)(*fp); 
		*pc = (caddr_t) *(++fp); 
	
		if ((*pc < (caddr_t) KVSBASE) || (*pc > (caddr_t) (KVSBASE + KTXTLEN)))
		{
			*pc++ = (caddr_t) NULL;
			break;
		}
		else
			pc++;

		if ((oldpcptr > (unsigned int *)0) && (*oldpcptr > (unsigned int)&u.u_stack) && (*oldpcptr < (unsigned int)&u.u_stack[KSTKSZ]))
			fp = oldpcptr; 
		else
			break;
	} 

	while (i < num) {
		*pc++ = (caddr_t) NULL;
		++i;
	}
}

STATIC	int	kmacctsizes[] =
	{16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 0x7FFFFFFF};

 int
kmaccount(type, size, addr)
	int	type;
	size_t	size;
	caddr_t	*addr;
{
	register kmasym_t	*kp;
	register kmabuf_t	*bp, *hl;
	register int		class = 0;
	register int		oldpri;
	caddr_t			pc[MAXDEPTH+2];

	/*
	 * Find the class of buffer, which is used as an index into
	 * the accounting arrays.
	 */

	while (size > kmacctsizes[class])
		++class;

	switch (type) {

	case KMACCT_ALLOC:

		/*
		 * The first two entries in the pc stack will be 
		 * kmaccount() and kmem_alloc() or kmem_free();
		 * ignore those.
		 */

		getcaller(kmadepth+2, pc);

		/*
		 * Get the symbol table entry (possible new) that corresponds
		 * to this calling sequence.
		 */

		if ((kp = getsymentry(&pc[2])) == (kmasym_t *) NULL) {
			cmn_err(CE_WARN, "Not enough KMARRAY entries\n");
			return -1;
		}

		/*
		 * Get a buffer header from the freelist, initialize it,
		 * and add it to the proper hash chain.
		 */
		oldpri = splhi();
		if ((bp = freebuflist) == (kmabuf_t *) NULL) {
			cmn_err(CE_WARN, "Not enough KMBUFFER entries\n");
			return -1;
		}
		freebuflist = bp->next;
		splx(oldpri);

		bp->addr = addr;
		bp->kp = kp;

		/*
		 * Add buffer header to hash chain.
		 */
		oldpri = splhi();
		hl = &bufhash[KMBFHASH(addr)];
		bp->next = hl->next;
		if (hl->next->prev)
			hl->next->prev = bp;
		bp->prev = hl;
		hl->next = bp;
		splx(oldpri);

		/*
		 * Increment the counter for allocations of this class.
		 */

		++kp->reqa[class];
		break;

	case KMACCT_FREE:

		/*
		 * Look in the hash chain for the buffer header for this
		 * buffer.  It is not necessarily an error if a header 
		 * cannnot be found.
		 */
		oldpri = splhi();
		hl = &bufhash[KMBFHASH(addr)];
		bp = hl->next;
		while (bp != hl) {
			if (addr == (bp)->addr)
				break;
			bp = bp->next;
		}
		splx(oldpri);

		if (bp == hl)
			return 0;

		/*
		 * Increment the counter for frees of this class.
		 */

		++bp->kp->reqf[class];

		/*
		 * Remove buffer header bp from the hash chain.
		 */

		bp->prev->next = bp->next;
		bp->next->prev = bp->prev;

		KMBFFREE(bp);

		break;

	default:

		break;

	}

	return 0;
}

/*
 * Paranoia code
 *
 * if Km_Paranoid is set, kmem_worry() will  be called on entering and leaving 
 * kmem_alloc(), kmem_free(), and kmem_coalesce().
 * kmem_worry() calls kmem_check() to check freelist headers.
 * if the freelist is trashed, will call demon.
 * if Km_CheckAll is also set, kmem_check() will check the entire
 * freelist, not just the headers.
 *
 * if the memory being allocated or freed is from the kma pools (i.e., it's
 * not an outsize request), kmem_worry() updates the callrec.
 *
 * if kmacctflag is set, kmem_worry() calls kmacct to trace memory leaks.
 *
 * if Km_TraceAddr is set, kmem_worry() will call demon when allocating
 * or freeing that addr.
 *
 * Km_Paranoid, Km_CheckAll, etc may be set from demon proms ("sw Km_Paranoid 1").
 *
 */

#define	MINASMALL	16	/* smallest allocable in small pool	*/
#define	MAXASMALL	256	/* largest allocable in small pool	*/
#define	MAXABIG		4096	/* largest allocable in big pool	*/
#define	MINSIDX		3	/* exp of smallest class in small pool	*/
#define	MAXBIDX		14	/* exp of largest class in big pool	*/
#define	MINASIDX	4	/* exp of smallest allocable in small*/
#define	MAXABIDX	12	/* exp of largest allocable in big	*/
#define	SMINOFFSET	(MINASIDX - MINSIDX)
#define	BMAXOFFSET	(MAXABIDX - MINSIDX)
#define	DELAYED	1


typedef struct fbuf {
	struct fbuf	*fb_nextp;	  /* next free buffer		*/
	struct fbuf	*fb_prevp;	  /* previous free buffer	*/
	ulong		fb_mask;	  /* mask to apply to bitmap	*/
	union {
		struct bpool	*fb_poolp; /* corresp. pool		*/
		ulong		fb_state;  /* DELAYED or not		*/
		ulong		*fb_mapp;  /* corresp. word of bitmap	*/
	} fb_union;
} freebuf;

typedef struct flist {
	freebuf		*fl_nextp;	/* first buffer in free list	*/
	freebuf		*fl_prevp;	/* last buffer in free list	*/
	ulong		fl_slack;	/* state var. for memory mgmt	*/
	ulong		fl_mask;	/* mask to apply to bitmap	*/
	ulong		fl_nbits;	/* # of bits to mask on/off	*/
} freelist;

typedef struct bpool {
	unchar		*bp_startp;	/* unaligned start address	*/
	unchar		*bp_alignp;	/* aligned start address	*/
	ulong		bp_inuse;	/* # of MAXA* buffers in use	*/
	ulong		bp_status;	/* current status of pool	*/
	ulong		*bp_bitmapp;	/* pointer to bitmap		*/
	ulong		bp_expmin;	/* exponent of smallest class	*/
	ulong		bp_expmax;	/* exponent of largest class	*/
} bufpool; 


/*
 * use Km_Alloc and Km_Free to keep track of who alloc'd & freed addrs.
 * have NREC of each -- will wrap when full.
 */

#define NUMCALL 6
#define NREC    2000

struct callrec {
	caddr_t	caller[NUMCALL];
	unchar	*addr;
	ulong	size;
} Km_Alloc[NREC+1], Km_Free[NREC+1];

STATIC int      Km_i, Km_j;	/* indices for Km_Alloc & Km_Free resp. */
STATIC int      Km_iwrap, Km_jwrap;	/* wrap flags	*/
STATIC int	Km_Max = 5; 	/* max number of alloc/frees to print */
STATIC int      Km_Low = MINASMALL;
STATIC int      Km_High = MAXABIG;

void		kmem_check();	/* check freelist 			*/
void     	kmem_trace();	/* trace usage of an address		*/
void		kmem_find();	/* look for an addr in freelist		*/
int		cntausers();	/* count allocations of an addr		*/
int		cntfusers();	/* count free's of an addr		*/
void		cntusers();	/* count alloc's and free's of an addr	*/
void		prausers();	/* print alloc's of an addr		*/
void		prfusers();	/* print free's of an addr		*/
void		prusers();	/* print alloc's and free's of an addr	*/
STATIC void	prarange();	/* internal routine for print trace	*/
STATIC void	prfrange();	/* internal routine for print trace	*/

extern int	Km_Paranoid;
extern int	Km_CheckAll;
extern unchar	*Km_TraceAddr;
extern freelist	Km_FreeLists[];


void
kmem_worry(s, type, addr, size)
char	*s;
int	type;
_VOID	*addr;
size_t	size;
{

	int	i;

	if ( Km_Paranoid )
		kmem_check(s, Km_CheckAll);

	if ( addr == (_VOID *)NULL || size == 0 )
		return;

	if ( Km_Paranoid ) {
		if ( size <= MAXABIG ) {
			/*
		 	* only keep Km_Alloc[] and Km_Free[] recs
		 	* for bufs from our own pools
		 	*/
			if ( type == KMACCT_ALLOC ) {
				if ( (i = Km_i++) >= NREC ) {
					Km_iwrap++;
					i = Km_i = 0;
				}
				getcaller(NUMCALL,
					Km_Alloc[i].caller);
				Km_Alloc[i].size = size;
				Km_Alloc[i].addr = (unchar *)addr;
			} else if ( type == KMACCT_FREE ) {
				if ( (i = Km_j++) >= NREC ) {
					Km_jwrap++;
					i = Km_j = 0;
				}
				getcaller(NUMCALL,
					Km_Free[i].caller);
				Km_Free[i].size = size;
				Km_Free[i].addr = (unchar *)addr;
			}
		}
		if ( addr == (freebuf *)Km_TraceAddr ) {
			cmn_err(CE_NOTE,
		    	"%s: %s traced address %x\n", s,
			(type == KMACCT_ALLOC ? "allocating"
			                      : "freeing"), addr);
			call_demon();
		}
	}
	if ( kmacctflag )
		kmaccount(type, size, (caddr_t *)addr);

	return;
}

/*
 * kmem_check(s, all)		freelist check
 * char	s;		text to print on error
 * int	all;		whether to walk entire freelist or just headers
 *
 * for debugging, to be called from demon debugger ("c kmem_check u.u_comm 0"
 * or "c kmem_check u.u_comm 1")
 *
 * also called on entry to or exit from kmem_alloc, kmem_free, kmem_coealesce
 * if Km_Paranoid is set.
 *
 * walks freelist, looking for trashed pointers.
 * if all is set, walks entire freelist.  if not, just looks at head & tail
 * pointers (fl_nextp and fl_prevp) off Km_FreeLists.
 *
 * if any pointer is trashed, prints warning and calls demon.
 * only check pointers that are not one of the elements of the Km_FreeLists
 * array.  "trashed" if it's not properly aligned or if it's < 0x40000000
 * or > 0x40500000
 *
 */

#define	ADDR1	(freebuf *)0x40000000
#define	ADDR2	(freebuf *)0x40500000
/* BADADDR1 -- address must be 'size'-aligned */
#define BADADDR1(x)  ( ( (x) < (freebuf *)&Km_FreeLists[SMINOFFSET] \
		        || (x) > (freebuf *)&Km_FreeLists[BMAXOFFSET] ) \
		      && ( ((ulong)(x) & (ulong)(size-1)) != 0 \
		           || ((x) < ADDR1) || ((x) > ADDR2) ) )
/* BADADDR2 -- only word alignment matters */
#define BADADDR2(x)  ( ((ulong)(x) & (ulong)3) != 0 \
		           || ((x) < ADDR1) || ((x) > ADDR2) )

/* is this the size request we are interested in */
#define SIZECHECK(s,b) ((s==0) || ((b <= s) && (b > (s/2))))

void
kmem_check(s, all)
char	*s;
int	all;
{
	register freebuf	*head, *tail, *p;
	register int		i, size;
	int			bad, stop = 0, oldpri;
	void			why1(), why2();

	for ( size = MINASMALL, i = SMINOFFSET;
	      size <= MAXABIG && i <= BMAXOFFSET; ++i, size *= 2 ) {


		/*
		 * have to spl this or interrupts cause bogus failures
		 */
		oldpri = splhi();

		if ( (head = Km_FreeLists[i].fl_nextp) == (freebuf *)&Km_FreeLists[i] ) {
			splx(oldpri);
			continue;	/* empty list -- continue in 'for' */
		}
		tail = Km_FreeLists[i].fl_prevp;
		bad = 0; 	/* innocent until proven guilty */

		if ( BADADDR1(head) ) {
			cmn_err(CE_WARN,
			"^%s: %u-byte freelist header trashed:\n\t\tfl_nextp = 0x%x\n\t\tfl_prevp = 0x%x\n",
			s, size, head, tail);
			why1(head, size);
			bad++;
		} else if ( BADADDR1(tail) ) {
			cmn_err(CE_WARN,
			"^%s: %u-byte freelist header trashed:\n\t\tfl_nextp = 0x%x\n\t\tfl_prevp = 0x%x\n",
			s, size, head, tail);
			why1(tail, size);
			bad++;
		} else {
			if ( BADADDR1(head->fb_nextp) ) {
				cmn_err(CE_WARN,
				"^%s: %u-byte freelist head pointer (fl_nextp) trashed:\n\t\tfl_nextp = 0x%x\n\t\tfl_nextp->fb_nextp = 0x%x\n\t\tfl_nextp->fb_prevp = 0x%x\n",
				s, size, head, head->fb_nextp, head->fb_prevp);
				why1(head->fb_nextp, size);
				bad++;
			}
			if ( BADADDR1(head->fb_prevp) ) {
				cmn_err(CE_WARN,
				"^%s: %u-byte freelist head pointer (fl_nextp) trashed:\n\t\tfl_nextp = 0x%x\n\t\tfl_nextp->fb_nextp = 0x%x\n\t\tfl_nextp->fb_prevp = 0x%x\n",
				s, size, head, head->fb_nextp, head->fb_prevp);
				why1(head->fb_prevp, size);
				bad++;
			}
			if ( BADADDR1(tail->fb_nextp) ) {
				cmn_err(CE_WARN,
				"^%s: %u-byte freelist tail pointer (fl_prevp) trashed:\n\t\tfl_prevp = 0x%x\n\t\tfl_prevp->fb_nextp = 0x%x\n\t\tfl_prevp->fb_prevp = 0x%x\n",
				s, size, tail, tail->fb_nextp, tail->fb_prevp);
				why1(tail->fb_nextp, size);
				bad++;
			}
			if ( BADADDR1(tail->fb_prevp) ) {
				cmn_err(CE_WARN,
				"^%s: %u-byte freelist tail pointer (fl_prevp) trashed:\n\t\tfl_prevp = 0x%x\n\t\tfl_prevp->fb_nextp = 0x%x\n\t\tfl_prevp->fb_prevp = 0x%x\n",
				s, size, tail, tail->fb_nextp, tail->fb_prevp);
				why1(tail->fb_prevp, size);
				bad++;
			}


			/* if union holds a pool or bitmap pointer, check it */
			if ( head->fb_union.fb_state != DELAYED
			&& BADADDR2((freebuf *)head->fb_union.fb_mapp) ) {
				cmn_err(CE_WARN,
				"^%s: %u-byte freelist head pointer (fl_nextp) trashed:\n\t\tfl_nextp = 0x%x\n\t\tfl_nextp->fb_fb_union.fb_%s = 0x%x\n",
				s, size, head,
				( ( size == MAXASMALL || size == MAXABIG) ?
					"poolp" : "mapp" ),
				head->fb_union.fb_mapp);
				why2(head->fb_union.fb_mapp);
				bad++;
			}
			if ( tail->fb_union.fb_state != DELAYED
			&& BADADDR2((freebuf *)tail->fb_union.fb_mapp) ) {
				cmn_err(CE_WARN,
				"^%s: %u-byte freelist tail pointer (fl_nextp) trashed:\n\t\tfl_nextp = 0x%x\n\t\tfl_nextp->fb_fb_union.fb_%s = 0x%x\n",
				s, size, tail,
				( ( size == MAXASMALL || size == MAXABIG) ?
					"poolp" : "mapp" ),
				tail->fb_union.fb_mapp);
				why2(tail->fb_union.fb_mapp);
				bad++;
			}

		}
		stop += bad;
		if ( bad || !all ) {	/* don't go further with bad pointers */
			splx(oldpri);
			continue;	/* for */
		}

		/* walk entire freelist */
		for ( p = head->fb_nextp; p != (freebuf *)&Km_FreeLists[i];
		      p = p->fb_nextp ) {
			if ( BADADDR1(p->fb_nextp) ) {
				cmn_err(CE_WARN,
				"^%s: %u-byte freelist trashed:\n\t\tbufp = 0x%x\n\t\tbufp->fb_nextp = 0x%x\n\t\tbufp->fb_prevp = 0x%x\n",
				s, size, p, p->fb_nextp, p->fb_prevp);
				why1(p->fb_nextp, size);
				bad++;
				break;
			}
			if ( BADADDR1(p->fb_prevp) ) {
				cmn_err(CE_WARN,
				"^%s: %u-byte freelist trashed:\n\t\tbufp = 0x%x\n\t\tbufp->fb_nextp = 0x%x\n\t\tbufp->fb_prevp = 0x%x\n",
				s, size, p, p->fb_nextp, p->fb_prevp);
				why1(p->fb_prevp, size);
				bad++;
				break;
			}
			if ( p->fb_union.fb_state != DELAYED
			&& BADADDR2((freebuf *)p->fb_union.fb_mapp) ) {
				cmn_err(CE_WARN,
				"^%s: %u-byte freelist trashed:\n\t\tfl_nextp = 0x%x\n\t\tfl_nextp->fb_fb_union.fb_%s = 0x%x\n",
				s, size, p,
				( ( size == MAXASMALL || size == MAXABIG) ?
					"poolp" : "mapp" ),
				p->fb_union.fb_mapp);
				why2(p->fb_union.fb_mapp);
				bad++;
			}
		}
		splx(oldpri);
		stop += bad;
	}
	if (stop)
		call_demon();
	return;
}

void
why1(x, size)
freebuf *x;
int	size;
{
	cmn_err(CE_CONT, "^kmem_check failed this test:\n");
	cmn_err(CE_CONT,
"^( ( (x) < (freebuf *)&Km_FreeLists[SMINOFFSET]\\\n");
	cmn_err(CE_CONT,
"^    || (x) > (freebuf *)&Km_FreeLists[BMAXOFFSET] ) \\\n");
	cmn_err(CE_CONT,
"^ && ( ((ulong)(x) & (ulong)(size-1)) != 0 \\\n");
	cmn_err(CE_CONT,
"^      || ((x) < ADDR1) || ((x) > ADDR2) ) )\n");
	cmn_err(CE_CONT,
"^( ( 0x%x < 0x%x\\\n", x, &Km_FreeLists[SMINOFFSET]);
	cmn_err(CE_CONT,
"^    || 0x%x > 0x%x ) \\\n", x, &Km_FreeLists[BMAXOFFSET]);
	cmn_err(CE_CONT,
"^ && ( ((ulong)0x%x & (ulong)(0x%x)) != 0 \\\n", x, (size-1));
	cmn_err(CE_CONT,
"^      || (0x%x < 0x%x) || (0x%x > 0x%x) ) )\n", x, ADDR1, x, ADDR2);

	return;
}


void
why2(x)
ulong *x;
{
	cmn_err(CE_CONT, "^kmem_check failed this test:\n");
	cmn_err(CE_CONT,
"^( ((ulong)(x) & (ulong)3) != 0 \\\n");
	cmn_err(CE_CONT,
"^  || ((x) < ADDR1) || ((x) > ADDR2) )\n");
	cmn_err(CE_CONT,
"^( (0x%x & (ulong)3) != 0 \\\n", x);
	cmn_err(CE_CONT,
"^  || (0x%x < 0x%x) || (0x%x > 0x%x) )\n", x, ADDR1, x, ADDR2);

	return;
}

/*
 * kmem_trace(addr, flag)	trace/count uses of addr
 *
 * if flag is  0 then print out all uses of addr (and before) addr;
 * if flag is !0 then print out number of uses of addr (and before) addr;
 */
void
kmem_trace(addr, flag)
register uint	addr;
int		flag;
{
	register uint	size, mask;
	register uint	baddr;

	/* since we are only saving records for this range
	 * we can only find matches in this range
	 */
	for ( size = Km_Low;  size <= Km_High; size *= 2 ) {

		/*
		 * if addr is on boundary x
		 * 	check(addr, x)
		 *	check(addr-x, x)
		 *	NOTE not checking addr + x
		 * else addr is inside an x-byte buffer
		 *	check(addr&(x-1), x);
		 */

		mask = ~(size - 1);
		baddr = (uint)addr & (uint)mask;
		if ( baddr == addr)  {

			if (flag != 0) {  /* just do count */
				cntusers(addr, size);
				cntusers(addr-size, size);
			} else {
				prausers(addr, size);
				prfusers(addr, size);
				prausers(addr-size, size);
				prfusers(addr-size, size);
			}
		} else {
			if (flag != 0) {  /* just do count */
				cntusers(baddr, size);
			} else {
				prusers(baddr, size);
			}
		}

	}
}

/* cntusers -- count alloc/free's of addr */
void
cntusers(addr, size)
uint addr, size;
{
	register int cnt;
	if (cnt = cntausers(addr, size))
		cmn_err(CE_CONT, "^%d kmem_alloc of size %d at addr 0x%x\n",
			cnt, size, addr);

	if (cnt = cntfusers(addr, size))
		cmn_err(CE_CONT, "^%d kmem_free  of size %d at addr 0x%x\n",
			cnt, size, addr);
}


/* count allocs */
int
cntausers(addr, size)
uint	addr, size;
{
	register int i, cnt = 0;

	for ( i = 0; i < NREC; i++ ) {
		if ((Km_Alloc[i].addr == (unchar *)addr) &&
		    SIZECHECK(size, Km_Alloc[i].size)) {
			cnt++;
		}
	}
	return cnt;
}

/* count frees */
int
cntfusers(addr, size)
uint	addr, size;
{
	register int i, cnt = 0;
	for ( i = 0; i < NREC; i++ ) {
		if ((Km_Free[i].addr == (unchar *)addr) &&
		    SIZECHECK(size, Km_Free[i].size)) {
			cnt++;
		}
	}
	return cnt;
}

/* prusers -- print trace of alloc/free's of addr */
void
prusers(addr, size)
uint addr, size;
{
	prausers(addr, size);
	prfusers(addr, size);
}


/* print allocs */
void
prausers(addr, size)
uint addr, size;
{
	int more = 0;

	/* We do this in reverse order to get last usage,
	 * Km_[ij] always points to next entry - entry is still valid
	 */
	if (Km_i != 0) {
		prarange(Km_i - 1, 0, addr, size, &more);
	}
	prarange(NREC - 1, Km_i, addr, size, &more);

}

/* print frees */
void
prfusers(addr, size)
uint addr, size;
{
	int more = 0;

	/* We do this in reverse order to get last usage,
	 * Km_[ij] always points to next entry - entry is still valid
	 */
	if (Km_j != 0) {
		prfrange(Km_j - 1, 0, addr, size, &more);
	}
	prfrange(NREC - 1, Km_j, addr, size, &more);

}

STATIC
void
prarange(start, end, addr, size, more)
int	start, end;
uint	addr, size;
int	*more;
{
	register int i, j;

	for ( i = start; i >= end; i-- ) {
		if ((Km_Alloc[i].addr == (unchar *)addr) &&
		    SIZECHECK(size, Km_Alloc[i].size)) {
			if ( (*more)++ > Km_Max ) {
				cmn_err(CE_CONT, "^Stopping at Km_Max\n");
				break;
			}
			cmn_err(CE_CONT, "^allocated %u bytes at 0x%x to:\n",
				Km_Alloc[i].size, addr );
			for ( j = 0; j < NUMCALL; ++j )
				if (Km_Alloc[i].caller[j])
					    cmn_err(CE_CONT, "^\t\t0x%x\n",
					    Km_Alloc[i].caller[j]);
		}
	}
}

STATIC
void
prfrange(start, end, addr, size, more)
int	start, end;
uint	addr, size;
int	*more;
{
	register int i, j;

	for ( i = start; i >= end; i-- ) {
		if ((Km_Free[i].addr == (unchar *)addr) &&
		    SIZECHECK(size, Km_Free[i].size)) {
			if ( (*more)++ > Km_Max ) {
				cmn_err(CE_CONT, "^Stopping at Km_Max\n");
				break;
			}
			cmn_err(CE_CONT, "^freed %u bytes at 0x%x from:\n",
				Km_Free[i].size, addr );
			for ( j = 0; j < NUMCALL; ++j )
				if (Km_Free[i].caller[j])
					    cmn_err(CE_CONT, "^\t\t0x%x\n",
					    Km_Free[i].caller[j]);
		}
	}
}

/*
 * The kernel text start address and kernel text length parameters should
 * agree with the kernel text origin and length values given in the
 * kernel ifile.  These are used to detect when we traced back out of
 * the kernel stack.
 */

#define	KTXTSTRT	0x40000000
#define	KTXTLEN		0x00160000


/*
 *	kmem_find:	look for 'addr' in freelists
 */
void
kmem_find(addr)
register void	*addr;
{
	register freebuf	*p;
	register int		i, size;

	for ( size = MINASMALL, i = SMINOFFSET;
	      size <= MAXABIG && i <= BMAXOFFSET;
	      ++i, size *= 2 ) {
		for ( p = Km_FreeLists[i].fl_nextp;
		      p != (freebuf *)&Km_FreeLists[i];
		      p = p->fb_nextp ) {
			if ( p == (freebuf *)addr ) {
				cmn_err(CE_NOTE,
				"^kmem_find: found address 0x%x in %u-byte freelist\nprev ptr = 0x%x, next = 0x%x\n",
				p, size, p->fb_prevp, p->fb_nextp);
				return;
			}
			if ( BADADDR1(p) ) {
				cmn_err(CE_WARN,
				"^kmem_find: %u-byte freelist trashed:\n\t\tbufp = 0x%x\n",
				size, p);
				break;
			}
		}
	}
	return;
}
#else	/* define stubs so that /etc/conf/cf.d/conf.o finds routines */
int	kmacinit() { return(0); }
int	kmacread(){ return(0); }
int	kmacioctl(){ return(0); }
#endif /* DEBUG */
