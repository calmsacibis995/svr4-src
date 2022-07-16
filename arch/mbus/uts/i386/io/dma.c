/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mbus:uts/i386/io/dma.c	1.3.2.1"

/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
 *	Copyright 1989, 1990  Intel Corporation
*/
#ifndef lint
static char dma_copyright[] = "Copyright 1989, 1990 Intel Corp. 464461";
#endif

/*
This is the implementation of the kernel DMA interface for the 82258 DMAC.

The following routines in the interface are implemented:
	dmainit()
	dma_intr()
	_dma_alloc()
	_dma_relse()
	dma_prog()
	dma_swsetup()
	dma_swstart()
	dma_stop()
	dma_enable()
	dma_disable()
	dma_get_best_mode ()
	dma_get_chan_stat()

And these support routines are included for managing the DMA structures
	dma_init_cbs()
	dma_get_cb()
	dma_free_cb()
	dma_init_bufs()
	dma_get_buf()
	dma_free_buf()
*/

#include "sys/types.h"
#include "sys/kmem.h"
#include "sys/cmn_err.h"
#include "sys/dma.h"
#include "sys/i82258.h"

#ifndef PRIVATE
#define PRIVATE static
#endif

/* the initialized flag */
PRIVATE unsigned char dma_initialized = 0;

/* the dma channel semaphore structure */
PRIVATE unsigned char dma_alloc_map[MAX_CHAN];

/*
 * dma_init_cbs() - Initialize the free list of DMA Command Blocks
 */

PRIVATE void
dma_init_cbs()
{
	/* nothing to do as we're using kmem_alloc */
}

/*
 * dma_init_bufs() - Initialize the free list of DMA Buffer Descriptors
 */

PRIVATE void
dma_init_bufs()
{
	/* nothing to do, we're using kmem_alloc */
}

/*
 * dmainit() - called to initialize the dma interface, the DMAC, and any
 *              dma data structures. Called during system initialization.
 */

void
dmainit()
{
	int i;

	if (!dma_initialized) {
		++dma_initialized;

		/* initialize semaphore map */
		for (i=0; i< MAX_CHAN; i++)
			dma_alloc_map[i] = 0;

		/* initialize the dma_cb pool */
		dma_init_cbs();

		/* initialize the dma_buf pool */
		dma_init_bufs();

		/* initialize the DMAC */

		d258_init();
	}
}


/*
 * dma_intr() -  service the interrupt from the DMAC.
 * 		 Is a noop for the 82258 DMAC right now.
 */

void
dma_intr(lev)
int lev;
{
	/* call the dmac interrupt handler */
	d258_intr(lev);
}


/*
 * _dma_alloc() - Request the semaphore for the indicated channel. If the
 *          	 channel is busy and mode is DMA_SLEEP then put process
 *          	 to sleep waiting for the semaphore. A mode of DMA_NOSLEEP
 *          	 indicates to simply return on a busy channel.
 */

PRIVATE int
_dma_alloc(chan, mode)
int chan;
unsigned char mode;
{
	int old_spl;

	/* test (and possibly set) semaphore */
	old_spl = splhi();

	while (dma_alloc_map[chan])
		if (mode == DMA_SLEEP)
			sleep((caddr_t)&dma_alloc_map[chan], PDMA);
		else {
			(void) splx(old_spl);
			return(FALSE);
		}

	/* got the semaphore, now set it */
	++dma_alloc_map[chan];
	/*
	ASSERT(dma_alloc_map[chan] == 1);
	*/

	splx(old_spl);

	return(TRUE);
}

/*
 * _dma_relse() - Release the channel semaphore on chan. Assumes caller actually
 *               owns the semaphore (no check made for this). Wakeup is
 *               called to awake anyone sleeping on the semaphore.
 */

PRIVATE int
_dma_relse(chan)
int chan;
{
	/* release the channel semaphore for chan */

	/* is channel even allocated? */
	if (!dma_alloc_map[chan])
		return(FALSE);

	--dma_alloc_map[chan];

	/* wake up any sleepers */
	(void)wakeup((caddr_t)&dma_alloc_map[chan]);

	return(TRUE);
}

/*
 * dma_get_cycle_type() - confirm that data is aligned for efficient flyby mode
 */

unsigned char
dma_get_best_mode(dmacbptr)
struct dma_cb *dmacbptr;
{
	unsigned char rcycles, tcycles;

	tcycles = d258_get_best_mode (dmacbptr->targbufs);
	rcycles = d258_get_best_mode (dmacbptr->reqrbufs);
	return (max(rcycles, tcycles));
}

/*
 * dma_prog() - Program chan for the to-be-initiated-by-harware operation
 *           	given in dmacbptr. _dma_alloc is called to request the channel
 *          	semaphore and mode is passed as the sleep parameter.
 */

int
dma_prog(struct dma_cb *dmacbptr, int chan, unsigned char mode)
{
	if (_dma_alloc(chan, mode) != TRUE)
		return(FALSE);

	if (!d258_dma_prog (dmacbptr, chan)) {
		/* problems, release semaphore so we can try again later */
		_dma_relse(chan);
		return(FALSE);
	}
	return(TRUE);
}


/*
 * dma_swsetup() - Setup chan for the operation given in dmacbptr.
 *			_dma_alloc is first called
 *			to request the channel semaphore for chan; mode is
 *			passed to _dma_alloc().
 */

int
dma_swsetup(struct dma_cb *dmacbptr, int chan, unsigned char mode)
{
	if (_dma_alloc(chan, mode) != TRUE)
		return(FALSE);

	/* got the semaphore, let d258 deal with it */
	if (!d258_dma_swsetup(dmacbptr, chan)) {
		/* oops, release semaphore for later retry */
		_dma_relse(chan);
		return(FALSE);
	}
	return(TRUE);
}

/*
 * Routine: dma_swstart() - Start the operation setup by dma_swsetup().
 *			    Go to sleep if mode is DMA_SLEEP after starting
 *			    operation.
 */

void
dma_swstart(struct dma_cb *dmacbptr, int chan, unsigned char mode)
{

	d258_dma_swstart(chan);

	/* go to sleep waiting for transfer to complete */
	if (mode == DMA_SLEEP)
		sleep(dmacbptr, PDMA);

}

/*
 * dma_stop() - stop DMA activity on chan and release the channel semaphore
 */

void
dma_stop(chan)
int chan;
{
	/* stop activity on DMA channel and release semaphore */

	/* call d258 the stop the channel */
	d258_dma_stop(chan);

	/* release the semaphore */
	_dma_relse(chan);
}

/*
 * dma_enable() - Allow the hardware tied to channel chan to request service
 *          	from the DMAC. dma_prog() should have been called prior
 *          	to this.
 */

void
dma_enable(chan)
int chan;
{
	/* allow preprogrammed hardware transfer to occur */

	d258_dma_enable(chan);
}

/*
 * dma_disable() - Called to mask off hardware requests on channel chan.
 *		Assumes the caller owns the channel.
 */

void
dma_disable(chan)
int chan;
{
	/* disallow subsequent hardware requests on channel chan */

	d258_dma_disable(chan);

	/* free the semaphore */
	_dma_relse(chan);
}

/*
 * dma_get_chan_stat() - Obtain the current channel status from the DMAC
 */

void
dma_get_chan_stat(dmastat, chan)
struct dma_stat *dmastat;
int chan;
{
	/* obtain current channel status from the DMAC */
	d258_get_chan_stat(dmastat, chan);
}


/*
 *	DMA control block management.  The policy is to try to
 *	maintain dma_cb_lowat buffers against the possibility that
 *	kma will not have memory for us at interrupt time.
*/
struct dma_cb *dma_cb_freelist = 0;
int dma_cb_free = 0;
extern int dma_cb_lowat;	/* from space.c */

struct dma_cb *
dma_get_cb(unsigned char mode)
{
	struct dma_cb *cbp;
	int old_spl;

	mode = (mode == DMA_NOSLEEP)? KM_NOSLEEP : KM_SLEEP;
	/*
	 *	Get above the low water mark if possible
	*/
	while (dma_cb_free < dma_cb_lowat) {
		cbp = (struct dma_cb *)kmem_zalloc(sizeof(struct dma_cb),mode);
		if (cbp == (struct dma_cb *) NULL) {
			break;
		}
		old_spl = splhi();
		cbp->next = dma_cb_freelist;
		dma_cb_freelist = cbp;
		dma_cb_free++;
		splx(old_spl);
	}
	/*
	 *	Do dynamic allocation if possible
	*/
	cbp = (struct dma_cb *) kmem_zalloc(sizeof(struct dma_cb), mode);
	/*
	 *	Take from emergency pool if necessary
	*/
	if (cbp == (struct dma_cb *) NULL) {
		old_spl = splhi();
		cbp = dma_cb_freelist;
		if (cbp) {
			dma_cb_freelist = cbp->next;
			dma_cb_free--;
		}
		splx(old_spl);
	}
	/*
	 *	Warn the user if no message buffer available
	 *	This means deep trouble for the system.
	*/
	if (cbp == (struct dma_cb *) NULL)
		cmn_err(CE_WARN,"dma_get_cb - out of control blocks");
	return(cbp);
}


void
dma_free_cb(cbp)
struct dma_cb *cbp;
{
	int old_spl;

	/*
	 *	If we are low on emergency buffers, restock
	 *	Enforce the invariant that everything on the
	 *	free list has been zeroed.
	*/
	if (dma_cb_free < dma_cb_lowat) {
		bzero((caddr_t)cbp, sizeof(*cbp));
		old_spl = splhi();
		cbp->next = dma_cb_freelist;
		dma_cb_freelist = cbp;
		dma_cb_free++;
		splx(old_spl);
		return;
	}
	/*
	 *	If we are well stocked, return the memory to the kernel
	*/
	kmem_free((caddr_t) cbp, sizeof(struct dma_cb));
}


/*
 *	DMA control block management.  The policy is to try to
 *	maintain dma_buf_lowat buffers against the possibility that
 *	kma will not have memory for us at interrupt time.
*/
struct dma_buf *dma_buf_freelist = 0;
int dma_buf_free = 0;
extern int dma_buf_lowat;	/* from space.c */

struct dma_buf *
dma_get_buf(unsigned char mode)
{
	struct dma_buf *dbp;
	int old_spl;

	mode = (mode == DMA_NOSLEEP)? KM_NOSLEEP : KM_SLEEP;
	/*
	 *	Get above the low water mark if possible
	*/
	while (dma_buf_free < dma_buf_lowat) {
		dbp = (struct dma_buf *)kmem_zalloc(sizeof(struct dma_buf),mode);
		if (dbp == (struct dma_buf *) NULL) {
			break;
		}
		old_spl = splhi();
		dbp->next_buf = dma_buf_freelist;
		dma_buf_freelist = dbp;
		dma_buf_free++;
		splx(old_spl);
	}
	/*
	 *	Do dynamic allocation if possible
	*/
	dbp = (struct dma_buf *) kmem_zalloc(sizeof(struct dma_buf), mode);
	/*
	 *	Take from emergency pool if necessary
	*/
	if (dbp == (struct dma_buf *) NULL) {
		old_spl = splhi();
		dbp = dma_buf_freelist;
		if (dbp) {
			dma_buf_freelist = dbp->next_buf;
			dma_buf_free--;
		}
		splx(old_spl);
	}
	/*
	 *	Warn the user if no message buffer available
	 *	This means deep trouble for the system.
	*/
	if (dbp == (struct dma_buf *) NULL)
		cmn_err(CE_WARN,"dma_get_buf - out of control blocks");
	return(dbp);
}


void
dma_free_buf(dbp)
struct dma_buf *dbp;
{
	int old_spl;

	/*
	 *	If we are low on emergency buffers, restock
	 *	Enforce the invariant that everything on the
	 *	free list has been zeroed.
	*/
	if (dma_buf_free < dma_buf_lowat) {
		bzero((caddr_t)dbp, sizeof(*dbp));
		old_spl = splhi();
		dbp->next_buf = dma_buf_freelist;
		dma_buf_freelist = dbp;
		dma_buf_free++;
		splx(old_spl);
		return;
	}
	/*
	 *	If we are well stocked, return the memory to the kernel
	*/
	kmem_free((caddr_t) dbp, sizeof(struct dma_buf));
}
