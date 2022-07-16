/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/mon.c	2.24"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 *	Environment variable PROFDIR added such that:
 *		If PROFDIR doesn't exist, "mon.out" is produced as before.
 *		If PROFDIR = NULL, no profiling output is produced.
 *		If PROFDIR = string, "string/pid.progname" is produced,
 *		  where name consists of argv[0] suitably massaged.
 *
 *
 *	Routines:
 *		(global) _monitor	init, cleanup for prof(1)iling
 *		(global) _mcount	function call counter
 *		(global) _mcount_newent	call count entry manager
 *		(static) _mnewblock	call count block allocator
 *
 *
 *	Monitor(), coordinating with mcount(), mcount_newent() and mnewblock(), 
 *	maintains a series of one or more blocks of prof-profiling 
 *	information.  These blocks are added in response to calls to
 *	monitor() (explicitly or via mcrt[01]'s _start) and, via mcount()'s
 *	calls to mcount_newent() thence to mnewblock().
 *	The blocks are tracked via a linked list of block anchors,
 *	which each point to a block.
 *
 *
 *	An anchor points forward, backward and 'down' (to a block).
 *	A block has the profiling information, and consists of
 *	three regions: a header, a function call count array region,
 *	and an optional execution histogram region, as illustrated below.
 *
 *
 *		 "anchor"
 *		+========+
 *	prior<--|        |-->next anchor
 *	anchor	|        |
 *		+========+
 *		 |
 *		 |
 *		 V "block"
 *		+-----------+
 *		+  header   +
 *		+-----------+
 *		+           +
 *		+ fcn call  +	// data collected by mcount
 *		+  counts   +
 *		+  array    +
 *		+           +
 *		+-----------+
 *		+           +
 *		+ execution +	// data collected by system call,
 *		+ profile   +	// profil(2) (assumed ALWAYS specified
 *		+ histogram +	// by monitor()-caller, even if small;
 *		+           +	// never specified by mnewblock() ).
 *		+-----------+
 *
 *	The first time monitor() is called, it sets up the chain
 *	by allocating an anchor and initializing countbase and countlimit
 *	to zero.  Everyone assumes that they start out zeroed.
 *
 *	When a user (or _start from mcrt[01]) calls monitor(), they
 *	register a buffer which contains the third region (either with
 *	a meaningful size, or so short that profil-ing is being shut off).
 *
 *	For each fcn, the first time it calls mcount(), mcount calls
 *	mcount_newent(), which parcels out the fcn call count entries
 *	from the current block, until they are exausted; then it calls
 *	mnewblock().
 *
 *	Mnewbloc() allocates a block Without a third region, and
 *	links in a new associated anchor, adding a new anchor&block pair
 *	to the linked list.  Each new mnewblock() block or user block,
 *	is added to the list as it comes in, FIFO.
 *
 *	When monitor() is called to close up shop, it writes out
 *	a summarizing header, ALL the fcn call counts from ALL
 *	the blocks, and the Last specified execution histogram
 *	(currently there is no neat way to accumulate that info).
 *	This preserves all call count information, even when
 *	new blocks are specified.
 *
 *	NOTE - no block passed to monitor() may be freed, until
 *	it is called to clean up!!!!
 *
 */
#ifdef __STDC__
	#pragma weak monitor = _monitor
#endif
#include "synonyms.h"
#include "shlib.h"
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mon.h>
#include <unistd.h>
#define PROFDIR	"PROFDIR"




char **___Argv = NULL; /* initialized to argv array by mcrt0 (if loaded) */

	/* countbase and countlimit are used to parcel out
	 * the pc,count cells from the current block one at 
	 * a time to each profiled function, the first time 
	 * that function is called.
	 * When countbase reaches countlimit, mcount() calls
	 * mnewblock() to link in a new block.
	 *
	 * Only monitor/mcount/mcount_newent/mnewblock() should change these!!
	 * Correct that: only these routines are ABLE to change these;
	 * countbase/countlimit are now STATIC!
	 */
static
char *countbase;	/* address of next pc,count cell to use in this block */
static
char *_countlimit;	/* address limit for cells (addr after last cell)  */


typedef struct anchor	ANCHOR;

struct anchor
{
	ANCHOR  *next, *prior;	/* forward, backward ptrs for list */
	struct hdr  *monBuffer;	/* 'down' ptr, to block */
	short  flags;		/* indicators - has histogram designation */

	int  histSize;		/* if has region3, this is size. */
};

#define HAS_HISTOGRAM	0x0001		/* this buffer has a histogram */


static ANCHOR 	*curAnchor = NULL;	/* addr of anchor for current block */
static ANCHOR    firstAnchor;		/* the first anchor to use - hopefully */
					/* the Only one needed */
					/* a speedup for most cases. */


static char *mon_out;



void
monitor(alowpc, ahighpc, buffer, bufsize, nfunc)
int	(*alowpc)(), (*ahighpc)(); /* boundaries of text to be monitored */
WORD	*buffer;	/* ptr to space for monitor data (WORDs) */
int	bufsize;	/* size of above space (in WORDs) */
int	nfunc;		/* max no. of functions whose calls are counted
			(default nfunc is 300 on PDP11, 600 on others) */
{
	int scale;
	long text;
	register char *s;
	struct hdr *hdrp;
	ANCHOR  *newanchp;
	int ssiz;
	char	*lowpc = (char *)alowpc;
	char	*highpc = (char *)ahighpc;



	if (lowpc == NULL) {		/* true only at the end */
		if (curAnchor != NULL) { /* if anything was collected!.. */


			profil(NULL, 0, 0, 0);
			if ( !writeBlocks() )
				perror(mon_out);
		}
		return;
	}
	/*
	 * Ok - they want to submit a block for immediate use, for
	 *	function call count consumption, and execution profile
	 *	histogram computation.
	 * If the block fails sanity tests, just bag it.
	 * Next thing - get name to use. If PROFDIR is NULL, let's
	 *	get out now - they want No Profiling done.
	 *
	 * Otherwise:
	 * Set the block hdr cells.
	 * Get an anchor for the block, and link the anchor+block onto 
	 *	the end of the chain.
	 * Init the grabba-cell externs (countbase/limit) for this block.
	 * Finally, call profil and return.
	 */

	ssiz = (sizeof(struct hdr) + nfunc * sizeof(struct cnt))/sizeof(WORD);
	if (ssiz >= bufsize || lowpc >= highpc)
		return;

	if ((s = getenv(PROFDIR)) == NULL) /* PROFDIR not in environment */
		mon_out = MON_OUT; /* use default "mon.out" */
	else if (*s == '\0') /* value of PROFDIR is NULL */
		return; /* no profiling on this run */
	else { /* construct "PROFDIR/pid.progname" */
		register int n;
		register pid_t pid;
		register char *name;
		size_t len;

		len = strlen(s);
		/* 15 is space for /pid.mon.out\0, if necessary */
		if ((mon_out = malloc(len + strlen(___Argv[0]) + 15)) == NULL) {
			perror("");
			return;
		}
		strcpy(mon_out, s);
		name = mon_out + len;
		*name++ = '/'; /* two slashes won't hurt */

		if ((pid = getpid()) <= 0) /* extra test just in case */
			pid = 1; /* getpid returns something inappropriate */
		for (n = 10000; n > pid; n /= 10)
			; /* suppress leading zeros */
		for ( ; ; n /= 10) {
			*name++ = pid/n + '0';
			if (n == 1)
			    break;
			pid %= n;
		}
		*name++ = '.';

		if (___Argv != NULL) /* mcrt0.s executed */
			if ((s = strrchr(___Argv[0], '/')) != NULL)
				strcpy(name, s + 1);
			else
				strcpy(name, ___Argv[0]);
		else
			strcpy(name, MON_OUT);
	}


	hdrp = (struct hdr *)buffer;	/* initialize 1st region */
	hdrp->lpc = lowpc;
	hdrp->hpc = highpc;
	hdrp->nfns = nfunc;

					/* get an anchor for the block */
	newanchp =  (curAnchor==NULL) ?
		&firstAnchor  :
		(ANCHOR *) malloc( sizeof(ANCHOR) );

	if (newanchp == NULL)
	{
		perror("monitor");
		return;
	}

					/* link anchor+block into chain */
	newanchp->monBuffer = hdrp;		/* new, down. */
	newanchp->next  = NULL;			/* new, forward to NULL. */
	newanchp->prior = curAnchor;		/* new, backward. */
	if (curAnchor != NULL)
		curAnchor->next = newanchp;	/* old, forward to new. */
	newanchp->flags = HAS_HISTOGRAM;	/* note that it has a histgm area */

					/* got it - enable use by mcount() */
	countbase  = (char *)buffer + sizeof(struct hdr);
	_countlimit = countbase + (nfunc * sizeof(struct cnt));

						/* (set size of region 3) */
	newanchp->histSize = bufsize*sizeof(WORD) - (_countlimit-(char*)buffer);


					/* done w/regions 1 + 2: setup 3 */
					/* to activate profil processing. */

	buffer += ssiz;			/* move ptr past 2'nd region */
	bufsize -= ssiz;		/* no. WORDs in third region */
					/* no. WORDs of text */
	text = (highpc - lowpc + sizeof(WORD) - 1)/
			sizeof(WORD);
	/* scale is a 16 bit fixed point fraction with the decimal
	   point at the left */
	if (bufsize < text)  {
		/* make sure cast is done first! */
		double temp = (double)bufsize;
		scale = (temp * (long)0200000L) / text;
	} else  {
		/* scale must be less than 1 */
		scale = 0xffff;
	}
	bufsize *= sizeof(WORD);	/* bufsize into # bytes */
	ssiz = ssiz * sizeof(WORD) + bufsize;	/* size into # bytes */
	profil(buffer, bufsize, (int)lowpc, scale);


	curAnchor = newanchp;		/* make latest addition, the cur anchor */

}


	/* writeBlocks() - write accumulated profiling info, std fmt.
	 *
	 * This routine collects the function call counts, and the
	 * last specified profil buffer, and writes out one combined
	 * 'pseudo-block', as expected by current and former versions
	 * of prof.
	 */
static
int
writeBlocks() {
	int fd;
	int ok;

	ANCHOR *ap;		/* temp anchor ptr */
	struct hdr *bp;		/* temp block ptr */
	struct hdr sum;		/* summary header (for 'pseudo' block) */

	ANCHOR *histp;		/* anchor with histogram to use */



	if ( (fd = creat(mon_out, 0666)) < 0 )
		return 0;

				/* this loop (1) computes # funct cts total */
				/*  (2) finds anchor of last block w/hist (histp) */
	histp=NULL;
	for(sum.nfns=0, ap=&firstAnchor; ap!=NULL; ap=ap->next)
	{
		sum.nfns += ap->monBuffer->nfns; /* accum num of cells */
		if( ap->flags & HAS_HISTOGRAM )
			histp=ap;		 /* remember lastone witha histgm */
	}


				/* copy pc range from effective histgm */
	sum.lpc = histp->monBuffer->lpc;
	sum.hpc = histp->monBuffer->hpc;

	ok = (write(fd, (char *)&sum, sizeof(sum)) == sizeof(sum)) ;



	if (ok)			/* if the hdr went out ok.. */
	{
		unsigned int amt;
		char *p;

				/* write out the count arrays (region 2's) */
		for(ap=&firstAnchor; ok && ap!=NULL; ap=ap->next)
		{
			amt	= ap->monBuffer->nfns * sizeof(struct cnt);
			p	= (char *)ap->monBuffer + sizeof(struct hdr);

			ok = (write(fd, p, amt) == amt);
		}

				/* count arrays out; write out histgm area */
		if (ok)
		{
			p	= (char *)histp->monBuffer + sizeof(struct hdr) +
				  (histp->monBuffer->nfns * sizeof(struct cnt));
			amt	= histp->histSize;

			ok = (write(fd, p, amt) == amt);

		}
	}
	
	(void) close(fd);

	return( ok );	/* indicate success */
}




	/* mnewblock() - allocate and link in a new region1&2 block.
	 *
	 * This routine, called by mcount_newent(), allocates a new block
	 * containing only regions 1 & 2 (hdr and fcn call count array),
	 * and an associated anchor (see header comments), inits the
	 * header (region 1) of the block, links the anchor into the
	 * list, and resets the countbase/limit pointers.
	 *
	 * This routine cannot be called recursively, since (each) mcount
	 * has a local lock which prevents recursive calls to mcount_newent.
	 * See mcount_newent for more details.
	 * 
	 */

#define THISMANYFCNS	(MPROGS0*2)

		/* call malloc() to get an anchor & a regn1&2 block, together */
#define GETTHISMUCH	(sizeof(ANCHOR) + 	/* get an ANCHOR */  \
			 (sizeof(struct hdr) +	/* get Region 1 */   \
			  THISMANYFCNS*sizeof(struct cnt) /* Region 2 */  \
			 )			/* but No region 3 */\
			)


static
void
_mnewblock()
{
	struct hdr  *hdrp;
	ANCHOR	    *newanchp;
	char *p;


					/* get anchor And block, together */
	p = (char *)malloc( GETTHISMUCH );
	if (p == NULL)
	{
		perror("mcount(mnewblock)");
		return;
	}

	newanchp = (ANCHOR *) p;
	hdrp = (struct hdr *)( p + sizeof(ANCHOR) );

					/* initialize 1st region to dflts */
	hdrp->lpc = 0;
	hdrp->hpc = 0;
	hdrp->nfns = THISMANYFCNS;

					/* link anchor+block into chain */
	newanchp->monBuffer = hdrp;		/* new, down. */
	newanchp->next  = NULL;			/* new, forward to NULL. */
	newanchp->prior = curAnchor;		/* new, backward. */
	if (curAnchor != NULL)
		curAnchor->next = newanchp;	/* old, forward to new. */
	newanchp->flags = 0;		/* note that it has NO histgm area */

					/* got it - enable use by mcount() */
	countbase  = (char *)hdrp + sizeof(struct hdr);
	_countlimit = countbase + (THISMANYFCNS * sizeof(struct cnt));

	newanchp->histSize = 0 ;	/* (set size of region 3.. to 0) */


	curAnchor = newanchp;		/* make latest addition, the cur anchor */
}



/* * * * * *
 * mcount_newent() -- call to get a new mcount call count entry.
 * 
 * this function is called by _mcount to get a new call count entry
 * (struct cnt, in the region allocated by _monitor()), or to return
 * zero if profiling is off.
 *
 * This function acts as a funnel, an access function to make sure
 * that all instances of mcount (the one in the a.out, and any in
 * any shared objects) all get entries from the same array, and
 * all know when profiling is off.
 * 
 * NOTE: when mcount calls this function, it sets a private flag
 * so that it does not call again until this function returns,
 * thus preventing recursion.
 * 
 * At Worst, the mcount in either a shared object or the a.out
 * could call once, and then the mcount living in the shared object
 * with monitor could call a second time (i.e. libc.so.1, although
 * presently it does not have mcount in it).  This worst case
 * would involve Two active calls to mcount_newent, which it can
 * handle, since the second one would find a already-set value
 * in countbase.
 * 
 * The only unfortunate result is that No new call counts
 * will be handed out until this function returns.
 * Thus if malloc or other routines called inductively by
 * this routine have not yet been provided with a call count entry,
 * they will not get one until this function call is completed.
 * Thus a few calls to library routines during the course of
 * profiling setup, may not be counted.
 *
 * NOTE: countbase points at the next available entry, and
 * countlimit points past the last valid entry, in the current
 * function call counts array.
 * 
 * 
 * if profiling is off		// countbase==0
 *   just return 0
 *
 * else
 *   if need more entries	// because countbase points last valid entry
 *     link in a new block, resetting countbase and countlimit
 *   endif
 *   if Got more entries
 *     return pointer to the next available entry, and
 *     update pointer-to-next-slot before you return.
 *
 *   else			// failed to get more entries
 *     just return 0
 *
 *   endif
 * endif
 */

struct cnt *
_mcount_newent()
{
	if ( countbase == 0 )
		return ((struct cnt *) 0 );

	if ( countbase  >= _countlimit )
		_mnewblock();		/* get a new block; set countbase */

	if ( countbase != 0 )
	{
		struct cnt *cur_countbase = (struct cnt *) countbase;

		countbase += sizeof(struct cnt);
		return ( cur_countbase );
	} else
		return ((struct cnt *) 0 );
}
