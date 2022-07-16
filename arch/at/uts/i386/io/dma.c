/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/dma.c	1.3.2.2"

/*
This is the implementation of the kernel DMA interface for the
AT Class machine's using an Intel 8237A DMAC.

The following routines in the interface are implemented:
        dma_init()
        dma_intr()
        _dma_alloc()
        _dma_relse()
        dma_prog()
        dma_swsetup()
        dma_swstart()
        dma_stop()
        dma_enable()
        dma_disable()
        dma_get_best_mode()
        dma_get_chan_stat()

And these support routines are included for managing the DMA structures
        dma_init_cbs()
        dma_get_cb()
        dma_free_cb()
        dma_init_bufs()
        dma_get_buf()
        dma_free_buf()
*/

#include "sys/param.h"
#include "sys/types.h"
#include "sys/buf.h"
#include "sys/kmem.h"

/***
** dma.h Includes i8237A.h so don't include i8237A.h in your source.
***/
#include "sys/dma.h"

#include "sys/cmn_err.h"

#ifdef DEBUG
#ifdef INKERNEL
#define dprintf(x)      cmn_err(CE_WARN, x)
#else
#define dprintf(x)      printf x
#endif
#else
#define dprintf(x)
#endif

/* the initialized flag */
unsigned char	dma_initialized = 0;

/* the dma channel semaphore structure */
unsigned char	dma_alloc_map[D37A_MAX_CHAN];

/****
*** Routine: dma_init_cbs()
*** Purpose: Initialize the free list of DMA Command Blocks
*** Caller:  dma_init()
*** Calls:   none.
****/

void dma_init_cbs()
{
	/* nothing to do as we're using kmem_zalloc */
	dprintf(("dma_init_cbs: do nothing.\n"));
}


/****
*** Routine: dma_init_bufs()
*** Purpose: Initialize the free list of DMA Buffer Descriptors
*** Caller:  dma_init()
*** Calls:   none.
****/

void dma_init_bufs()
{
	/* nothing to do, we're using kmem_zalloc */
	dprintf(("dma_init_bufs: do nothing.\n"));
}


/****
*** Routine: dma_init()
*** Purpose: called to initialize the dma interface, the DMAC, and any
***          dma data structures. Called during system initialization.
*** Caller:  main()
*** Calls:   d37A_init()
****/

void
dma_init()
{
	int	i;

	if (!dma_initialized) {
		dprintf(("dma_init: initializing dma.\n"));

		/* initialize semaphore map */
		for (i = 0; i < D37A_MAX_CHAN; i++)
			dma_alloc_map[i] = 0;

		/* initialize the dma_cb pool */
		dma_init_cbs();

		/* initialize the dma_buf pool */
		dma_init_bufs();

		/* initialize the 8237A DMAC */
		d37A_init();

		++dma_initialized;
	}
}


/****
*** Routine: dma_get_best_mode()
*** Purpose: confirm that data is aligned for efficient flyby mode
*** Caller:  driver routines.
*** Calls:   d37A_get_best_mode.
****/

unsigned char	dma_get_best_mode(dmacbptr)
struct dma_cb *dmacbptr;
{
	return(d37A_get_best_mode(dmacbptr));
}


/****
*** Routine: dma_intr()
*** Purpose: service the interrupt from the DMAC.
*** Caller:  k_trap(), u_trap() through (*ivect[lev])()
*** Calls:   d37A_intr()
****/

void
dma_intr(lev)
int	lev;
{
	/* call the d37A interrupt handler */
	dprintf(("dma_intr: calling d37A_intr(%d).\n", lev));
	d37A_intr(lev);
}


/****
*** Routine: _dma_alloc()
*** Purpose: Request the semaphore for the indicated channel. If the
***          channel is busy and mode is DMA_SLEEP then put process
***          to sleep waiting for the semaphore. A mode of DMA_NOSLEEP
***          indicates to simply return on a busy channel.
*** Caller:  dma_prog(), dma_swsetup(), dma_stop()
*** Calls:   splhi(), sleep(), splx(), ASSERT()
****/

int	
_dma_alloc(chan, mode)
register int	chan;
unsigned char	mode;
{
	/* request the semaphore for the indicated channel */
	int	old_spl;

	dprintf(("_dma_alloc: attempting to allocate channel %d, mode is %s\n",
	    chan, (mode == DMA_SLEEP ? "DMA_SLEEP" : "DMA_NOSLEEP")));

	/* test (and possibly set) semaphore */
	old_spl = splhi();

	while (dma_alloc_map[chan]) {
		dprintf(("_dma_alloc: channel %d is busy.\n", chan));

		if (mode == DMA_SLEEP) {
			dprintf(("_dma_alloc: sleeping on address %l\n",
			    &dma_alloc_map[chan]));

			sleep(&dma_alloc_map[chan], PDMA);
		} else {
			splx(old_spl);
			return(FALSE);
		}
	}

	/* got the semaphore, now set it */
	dma_alloc_map[chan] = 1;
	dprintf(("_dma_alloc: channel %d now allocated.\n", chan));

	splx(old_spl);

	return(TRUE);
}


/****
*** Routine: _dma_relse()
*** Purpose: Release the channel semaphore on chan. Assumes caller actually
***          owns the semaphore (no check made for this). Wakeup is
***          called to awake anyone sleeping on the semaphore.
*** Caller:  dma_prog(), dma_swsetup(), dma_stop()
*** Calls:   none
****/

int	
_dma_relse(chan)
register int	chan;
{
	/* release the channel semaphore for chan */
	/*dprintf(("_dma_relse: attempting to release channel %d.\n", chan)); */

	/* is channel even allocated? */
	if (!dma_alloc_map[chan]) {
		/* dprintf(("_dma_relse: channel %d not allocated.\n", chan)); */

		return(FALSE);
	}

	dma_alloc_map[chan] = 0;
	/* dprintf(("_dma_relse: channel %d released.\n", chan)); */

	/* wake up any sleepers */
	/* dprintf(("_dma_relse: waking up sleepers on address %x.\n",
                                (unsigned long) &dma_alloc_map[chan])); */
	wakeup(&dma_alloc_map[chan]);

	return(TRUE);
}


/****
*** Routine: dma_prog()
*** Purpose: Program chan for the to-be-initiated-by-harware operation
***          given in dmacbptr. _dma_alloc is called to request the channel
***          semaphore and mode is passed as the sleep parameter.
*** Caller:  driver routines
*** Calls:   _dma_alloc(), d37A_prog_chan(), _dma_relse()
****/

int
dma_prog(dmacbptr, chan, mode)
register struct dma_cb *dmacbptr;
register int	chan;
unsigned char	mode;
{
	/* attempt to program channel chan */
	dprintf(("dma_prog: attempting to program channel %d: mode is %s",
	    chan, (mode == DMA_SLEEP ? "DMA_SLEEP" : "DMA_NOSLEEP")));
	dprintf((" dmacbptr= %x.\n", (unsigned long) dmacbptr));

	/* first step, try to get the semaphore */
	if (_dma_alloc(chan, mode) != TRUE) {
		dprintf(("dma_prog: channel %d not programmed, channel busy.\n", 
		    chan));
		return(FALSE);
	}

	/* ok, now let d37A deal with it */
	if (!d37A_prog_chan(dmacbptr, chan)) {
		/* problems, release semaphore so we can try again later */
		dprintf(("dma_prog: channel %d not programmed.\n", chan));

		_dma_relse(chan);
		return(FALSE);
	}

	dprintf(("dma_prog: channel %d programmed.\n", chan));
	return(TRUE);
}


/****
*** Routine: dma_swsetup()
*** Purpose: Setup chan for the operation given in dmacbptr.
***          _dma_alloc is first called
***          to request the channel semaphore for chan; mode is
***          passed to _dma_alloc().
*** Caller:  driver routines
*** Calls:   _dma_alloc(), d37A_dma_swsetup(), _dma_relse(), proc().
****/

int
dma_swsetup(dmacbptr, chan, mode)
register struct dma_cb *dmacbptr;
register int	chan;
unsigned char	mode;
{
	/* program and software initiate a DMA transfer */
	dprintf(("dma_swsetup: attempting to setup channel %d: mode is %s",
	    chan, (mode == DMA_SLEEP ? "DMA_SLEEP" : "DMA_NOSLEEP")));
	dprintf((" dmacbptr= %x.\n", (unsigned long) dmacbptr));

	/* first step, try and get the semaphore */
	if (_dma_alloc(chan, mode) != TRUE) {
		dprintf(("dma_swsetup: channel %d not set up, channel busy\n", 
		    chan));

		return(FALSE);
	}

	/* got the semaphore, let d37A deal with it */
	if (!d37A_dma_swsetup(dmacbptr, chan)) {
		/* oops, release semaphore for later retry */
		dprintf(("dma_swsetup: channel %d not set up.\n", chan));

		_dma_relse(chan);
		return(FALSE);
	}

	dprintf(("dma_swsetup: channel %d set up.\n", chan));

	/* call the caller's routine if set */
	if (dmacbptr->proc) {
		dprintf(("dma_swsetup: calling (dmacbptr->proc)(%x).\n",
		    (unsigned long) dmacbptr->procparms));

		(dmacbptr->proc)(dmacbptr->procparms);
	}

	return(TRUE);
}


/****
*** Routine: dma_swstart()
*** Purpose: Start the operation setup by dma_swsetup(). Go to sleep
***          if mode is DMA_SLEEP after starting operation.
*** Caller:  driver routines
*** Calls:   d37A_dma_swstart(), sleep().
****/

void
dma_swstart(dmacbptr, chan, mode)
register struct dma_cb *dmacbptr;
register int	chan;
unsigned char	mode;
{
	/* start operation previously set up on chan */
	dprintf(("dma_swstart: starting channel %d.\n", chan));

	d37A_dma_swstart(chan);

	/* go to sleep waiting for transfer to complete */
	if (mode == DMA_SLEEP) {
		dprintf(("dma_swstart: sleeping on address %x.\n",
		    (unsigned long) dmacbptr));

		sleep(dmacbptr, PDMA);
	}
}


/****
*** Routine: dma_stop()
*** Purpose: stop DMA activity on chan and release the channel semaphore
*** Caller:  driver routines
*** Calls:   splhi(), _dma_alloc(), _dma_relse(), splx(), 
***          d37A_dma_stop().
****/

void
dma_stop(chan)
register int	chan;
{
	/* stop activity on DMA channel and release semaphore */
	dprintf(("dma_stop: stopping channel %d.\n", chan));

	/* call d37A the stop the channel */
	d37A_dma_stop(chan);

	/* release the semaphore */
	_dma_relse(chan);
}


/****
*** Routine: dma_enable()
*** Purpose: Allow the hardware tied to channel chan to request service
***          from the DMAC. dma_prog() should have been called prior
***          to this.
*** Caller:  driver routines.
*** Calls:   d37A_dma_enable()
****/

void
dma_enable(chan)
register int	chan;
{
	/* allow preprogrammed hardware transfer to occur */
	dprintf(("dma_enable: enable channel %d.\n", chan));

	d37A_dma_enable(chan);
}


/****
*** Routine: dma_disable()
*** Purpose: Called to mask off hardware requests on channel chan. Assumes
***          the caller owns the channel.
*** Caller:  driver routines.
*** Calls:   d37A_dma_disable()
****/

void
dma_disable(chan)
register int	chan;
{
	/* disallow subsequent hardware requests on channel chan */
	/* dprintf(("dma_disable: disable channel %d.\n", chan)); */

	d37A_dma_disable(chan);

	/* free the semaphore */
	_dma_relse(chan);
}


/****
*** Routine: dma_get_chan_stat()
*** Purpose: Obtain the current channel status from the DMAC
*** Caller:  driver routines.
*** Calls:   d37A_get_chan_stat()
****/

void
dma_get_chan_stat(dmastat, chan)
struct dma_stat *dmastat;
int	chan;
{
	/* obtain current channel status from the DMAC */
	dprintf(("dma_get_chan_stat: obtaining channel %d's status\n",
	    chan));
	dprintf((" dmastat= %x.\n", dmastat));

	d37A_get_chan_stat(dmastat, chan);
}


/****
*** Routine: dma_get_cb()
*** Purpose: Get a DMA Command Block for a future DMA operation.
***          Zero it out as well. Return a pointer to the block.
*** Caller:  driver routines.
*** Calls:   kmem_zalloc().
****/

struct dma_cb *dma_get_cb(mode)
unsigned char	mode;
{
	struct dma_cb *dmacbptr;
	int	i;

	/* get a new dma_cb structure and zero it out */
	dprintf(("dma_get_cb: get new dma_cb, mode is %s.\n",
	    (mode == DMA_SLEEP ? "DMA_SLEEP" : "DMA_NOSLEEP")));

	/* anything available? */
	dmacbptr = (struct dma_cb *) kmem_zalloc(sizeof(struct dma_cb),
			mode == DMA_NOSLEEP ? KM_NOSLEEP : KM_SLEEP);
	if ( dmacbptr == NULL ) {
		cmn_err( CE_WARN, "Unable to allocate memory for a DMA Control Block" );
		return(dmacbptr);
	}

	dprintf(("dma_get_cb: new dmacbptr= %x.\n", (unsigned long) dmacbptr));
	return(dmacbptr);
}


/****
*** Routine: dma_free_cb()
*** Purpose: Return a DMA Command Block to the free list
*** Caller:  driver routines.
*** Calls:   kmem_free().
****/

void
dma_free_cb(dmacbptr)
struct dma_cb *dmacbptr;
{
	/* return the dma_cb to the kernel pool */
	dprintf(("dma_free_cb: freeing dmacbptr= %x.\n",
	    (unsigned long) dmacbptr));

	kmem_free((void *) dmacbptr, sizeof(struct dma_cb));
}


/****
*** Routine: dma_get_buf()
*** Purpose: Get a DMA Buffer Descriptor for a future DMA operation.
***          Return a pointer to the block.
*** Caller:  driver routines.
*** Calls:   kmem_zalloc().
****/

struct dma_buf *dma_get_buf(mode)
unsigned char	mode;
{
	struct dma_buf *dmabufptr;
	int	i;

	/* get a new dma_buf structure */
	dprintf(("dma_get_buf: get new dma_buf, mode is %s.\n",
	    (mode == DMA_SLEEP ? "DMA_SLEEP" : "DMA_NOSLEEP")));

	/* anything available? */
	dmabufptr = (struct dma_buf *) kmem_zalloc(sizeof(struct dma_buf),
			mode == DMA_NOSLEEP ? KM_NOSLEEP : KM_SLEEP);
	if ( dmabufptr == NULL ) {
		cmn_err( CE_WARN, "Unable to allocate memory for a DMA Buffer" );
		return(dmabufptr);
	}

	dprintf(("dma_get_buf: new dmabufptr= %x.\n", (unsigned long) dmabufptr));
	return(dmabufptr);
}


/****
*** Routine: dma_free_buf()
*** Purpose: Return a DMA Buffer Descriptor to the free list
*** Caller:  driver routines.
*** Calls:   kmem_free()
****/

void
dma_free_buf(dmabufptr)
struct dma_buf *dmabufptr;
{
	dprintf(("dma_free_buf: freeing dmabufptr= %x.\n",
	    (unsigned long) dmabufptr));

	kmem_free((void *) dmabufptr, sizeof(struct dma_buf));
}


/* FOLLOWING ROUTINES EXIST for BACKWARDS COMPATABILITY (XENIX ?) */

/*      chtorq - convert channel number to request queue index.
 *
 *      Normally all channels are used simultaneously and the
 *      channel number is used as the index into the request
 *      queues.  Some DMA chips, however, have bugs that only
 *      allow one transfer to take place at a time.  In this
 *      case, dma_single should be set non-zero so that the
 *      channel number is converted into a chip number (0=8 bit,
 *      1=16 bit) forcing drivers to allocate the chip instead
 *      of the channel.
 */

#define chtorq(ch)      ((dma_single) ? (ch)>>2 : (ch))

extern int	dma_single;

unsigned int	dmamap = 0; /* task time channel allocation bits */

unsigned short	dmapage[] = {
	DMA_0XADR,
	DMA_1XADR,
	DMA_2XADR,
	DMA_3XADR,
	0,
	DMA_5XADR,
	DMA_6XADR,
	DMA_7XADR,
};


unsigned short	dmaaddr[] = {
	DMA_0ADR,
	DMA_1ADR,
	DMA_2ADR,
	DMA_3ADR,
	0,
	DMA_5ADR,
	DMA_6ADR,
	DMA_7ADR,
};


unsigned short	dmawcnt[] = {
	DMA_0WCNT,
	DMA_1WCNT,
	DMA_2WCNT,
	DMA_3WCNT,
	0,
	DMA_5WCNT,
	DMA_6WCNT,
	DMA_7WCNT,
};



/* interrupt time request queue */
struct dmareq *dmahead[NCHANS] = {
	(struct dmareq *)0};


struct dmareq *dmatail[NCHANS] = {
	(struct dmareq *)0};


dma_param( ch, mode, addr, cnt )
unsigned	ch, mode;
paddr_t addr;
long	cnt;
{
	int	s;
	short	msk, clff, modp;
	short	chan;
	register unsigned char	*p;

	p = (unsigned char *) & addr;

	if ( ch & 4 ) {
		msk  = CTL2_MASK;
		clff = CTL2_CLFF;
		modp  = CTL2_MODE;
		cnt >>= 1;          /* convert byte count to a word count */

		/* Corrected next line, M000 */
		/* one more time, M001 */
		*((ushort * ) p) = (*(ushort * )p >> 1) | ((*(p + 2) & 1) << 15);
	} else
	 {
		msk  = CTL1_MASK;
		clff = CTL1_CLFF;
		modp  = CTL1_MODE;
	}

	chan = ch & 3;
	s = spl6();
	outb( msk, chan | DMA_SETMSK);            /* mask channel */
	outb( clff, 0);                         /* clear flip-flop */
	if ( mode != DMA_Nomode )
		outb( modp, mode | chan);
	/* else will be set by DMAstart */
	outb( dmaaddr[ch], p[0]);
	outb( dmaaddr[ch], p[1]);
	outb( dmapage[ch], p[2]);

	p = (unsigned char *) & cnt;
	outb( dmawcnt[ch], p[0]);
	outb( dmawcnt[ch], p[1]);
	splx(s);
}


/*      dma_resid - return residual count of a dma channel.
 *
 */

long
dma_resid(ch)
unsigned	ch;
{

	int	s;
	short	msk, clff;
	int	chan;
	long	count;

	if ( ch & 4 ) {
		msk  = CTL2_MASK;
		clff = CTL2_CLFF;
	} else
	 {
		msk  = CTL1_MASK;
		clff = CTL1_CLFF;
	}

	chan = ch & 3;
	s = spl6();
	outb( msk, chan | DMA_SETMSK);            /* mask channel */
	outb( clff, 0);                         /* clear flip-flop */
	/*
         * successful transfers are indicated by 0xffff in count register.
         * must be sure to sign extend the value read from the register
         * since we are constructing a short and return a long.  also, add
         * one before the possible left shift for 16 bit channels.
         */
	count = inb( dmawcnt[ch] );
	count = ((long)(short)(count | inb( dmawcnt[ch] ) << 8)) + 1;
	splx(s);
	if (ch & 4)
		count <<= 1;
	return( count );
}



/*      dma_relse - task time release of dma_channels
 *
 */

dma_relse( chan )
unsigned	chan;
{
	unsigned	s;
	register struct dmareq *dmap;
	unsigned	indx;

	indx = chtorq(chan);
	s = spl6();
	if ( dmap = dmahead[indx] ) {
		if ( (dmahead[indx] = dmap->d_nxt) == (struct dmareq *)0 )
			dmatail[indx] = 0;
		(*dmap->d_proc)(dmap);
		splx(s);
		return;
	}
	splx(s);
	_dma_relse(chan);
	return;
}


startDMA(addr, func, cnt, chan)
register unsigned	chan;
paddr_t addr;
{
	dma_param(chan, func & B_READ ? DMA_Rdmode : DMA_Wrmode, addr, (long)cnt );

	if ( chan != DMA_CH3 ) /* This is for backwards compatability */
		outb( (chan & 4) ? CTL2_MASK : CTL1_MASK, (chan & 3) | DMA_CLRMSK);
}



/*      dma_alloc - task time allocation of dma channels
 *
 */

dma_alloc( chan, mod)
unsigned	chan;
unsigned	mod;
{
	if ( mod == DMA_NBLOCK ) {
		return(_dma_alloc(chan, DMA_NOSLEEP));
	} else
		return(_dma_alloc(chan, DMA_SLEEP));
}


#ifndef STANDALONE
DMAalloc(chan, pri)
register unsigned	chan;
unsigned	pri;
{
	/* We sleep here and do non blocking dma_alloc so that
           we can properly emulate the pri argument.  dma_alloc
           always uses DMAPRI.
         */
#if Q_XT || Q_PLUS
	chan = cvtchan(chan);
#endif
	return (_dma_alloc( chan, DMA_SLEEP));
}


/*      dma_relse - task time release of dma_channels
 *
 */

DMArelse(chan)
register unsigned	chan;
{
#if Q_XT || Q_PLUS
	chan = cvtchan(chan);
#endif
	_dma_relse(chan);
}


#endif
/***  END  M003 ***/

/* and XT compatability .  The
 * following assume only channels 0-4 */

DMAsetup( chan, addr, count )
unsigned	chan, count;
paddr_t addr;
{
	dma_param( cvtchan(chan), DMA_Nomode, addr, (long)count );
}


DMAstart(chan, flags)
register unsigned	chan;
register unsigned	flags;
{
	outb(CTL1_MODE, (flags & B_READ ? DMA_Rdmode : DMA_Wrmode ) | cvtchan(chan));
}


DMAclrmsk(chan)
register unsigned	chan;
{
	outb(CTL1_MASK, DMA_CLRMSK | cvtchan(chan));
}


/*
 * Return number of bytes not transferred.
 */
unsigned	DMAresid(chan)
register unsigned	chan;
{
	return( (unsigned)dma_resid( cvtchan(chan) ) );
}


cvtchan( ch )
unsigned	ch;
{
	register unsigned	i, j;
	for (i = 0, j = 1; j != ch; ++i, j <<= 1)
		;
	return(i);
}


#ifdef VERBOSE_DEBUG

dma_dump(len)                   /* display dma registers */
int	len ;
{
	unsigned	i, a, w ;

	for (i = 0; i < len; i++) {
		if ((i % 4) == 0) {
			outb((i & 4) ? CTL2_CLFF : CTL1_CLFF, 0) ;
			printf("dma %d - status: %x\n", i >> 2,
			    inb((i >> 2) ? CTL2_STAT : CTL1_STAT)) ;
			continue;
		}
		a = (inb(dmaaddr[i]) << 8) + inb(dmaaddr[i]) ;
		w = (inb(dmawcnt[i]) << 8) + inb(dmawcnt[i]) ;
		printf("ch(%d) pg=%x, addr=%x, wc=%x\n", i, inb(dmapage[i]), a, w) ;
	}
}


dma_dump1(ch)
int	ch ;
{
	unsigned	a, w ;

	outb((ch & 4) ? CTL2_CLFF : CTL1_CLFF, 0) ;
	a = (inb(dmaaddr[ch]) << 8) + inb(dmaaddr[ch]) ;
	w = (inb(dmawcnt[ch]) << 8) + inb(dmawcnt[ch]) ;
	printf("tc=%d pg=%x addr=%x wc=%x\n",
	    (inb((ch >> 2) ? CTL2_STAT : CTL1_STAT) >> ch) & 1,
	    inb(dmapage[ch]), a, w) ;

}

#endif
