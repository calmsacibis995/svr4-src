/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mb1:uts/i386/io/dma.c	1.3"

/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
 *	Copyright 1989  Intel Corporation
*/
#ifndef lint
static char dma_copyright[] = "Copyright 1989 Intel Corp. 464459";
#endif

/*
 *	These are stubs since there is no DMA on our Multibus I CPU cards.
 *
 *	The following routines in the interface are implemented:
 *		dmainit()
 *		dma_intr()
 *		_dma_alloc()
 *		_dma_relse()
 *		dma_prog()
 *		dma_swsetup()
 *		dma_swstart()
 *		dma_stop()
 *		dma_enable()
 *		dma_disable()
 *		dma_get_best_mode()
 *		dma_get_chan_stat()
 *	
 *	And these support routines are included for managing the DMA structures
 *		dma_init_cbs()
 *		dma_get_cb()
 *		dma_free_cb()
 *		dma_init_bufs()
 *		dma_get_buf()
 *		dma_free_buf()
*/

#include "sys/types.h"
#include "sys/kmem.h"
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
}


/*
 * dma_alloc() - Request the semaphore for the indicated channel. If the
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
 * dma_relse() - Release the channel semaphore on chan. Assumes caller actually
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
 * dma_get_best_mode() - confirm that data is aligned for efficient flyby mode
 */

int
dma_get_best_mode(dmacbptr)
struct dma_cb *dmacbptr;
{
	int rcycles, tcycles;

	tcycles = 0; /*XXX*/
	rcycles = 0; /*XXX*/
	switch (rcycles) {
	case DMA_BLAST_MODE:
		if ((tcycles == DMA_2CYCLE_MODE) || (tcycles == DMA_FLYBY_MODE))
			return (tcycles);
		break;
	case DMA_FLYBY_MODE:
		if (tcycles == DMA_2CYCLE_MODE)
			return (tcycles);
		break;
	default:
		break;
	}
	return (rcycles);
}

/*
 * dma_prog() - Program chan for the to-be-initiated-by-harware operation
 *           	given in dmacbptr. dma_alloc is called to request the channel
 *          	semaphore and mode is passed as the sleep parameter.
 */

int
dma_prog(dmacbptr, chan, mode)
struct dma_cb *dmacbptr;
int chan;
unsigned char mode;
{
		return(FALSE);
}


/*
 * dma_swsetup() - Setup chan for the operation given in dmacbptr.
 *			dma_alloc is first called
 *			to request the channel semaphore for chan; mode is
 *			passed to dma_alloc().
 */

int
dma_swsetup(dmacbptr, chan, mode)
struct dma_cb *dmacbptr;
int chan;
unsigned char mode;
{
	return(FALSE);
}

/*
 * Routine: dma_swstart() - Start the operation setup by dma_swsetup().
 *			    Go to sleep if mode is DMA_SLEEP after starting
 *			    operation.
 */

void
dma_swstart(dmacbptr, chan, mode)
struct dma_cb *dmacbptr;
int chan;
unsigned char mode;
{
}

/*
 * dma_stop() - stop DMA activity on chan and release the channel semaphore
 */

void
dma_stop(chan)
int chan;
{
	/* stop activity on DMA channel and release semaphore */

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
}

/*
 * dma_get_cb() - Get a DMA Command Block for a future DMA operation.
 *          	Zero it out as well. Return a pointer to the block.
 */

struct dma_cb *
dma_get_cb(mode)
unsigned char mode;
{
	/* get a new dma_cb structure and zero it out */

	struct dma_cb *dmacbptr;
	int i;

	/* anything available? */
	if ((dmacbptr = (struct dma_cb *) kmem_alloc(sizeof(struct dma_cb),
			(int)mode == DMA_NOSLEEP ? KM_NOSLEEP : KM_SLEEP))
			!= (struct dma_cb *) NULL) {
		/* zero out the structure */
		for (i=0; i<sizeof(struct dma_cb); i++)
			((char *)dmacbptr)[i] = 0;
	}

	return(dmacbptr);
}

/*
 * dma_free_cb() - Return a DMA Command Block to the free list
 */

void
dma_free_cb(dmacbptr)
struct dma_cb *dmacbptr;
{
	/* return the dma_cb to the kernel pool */
	kmem_free((caddr_t) dmacbptr, sizeof(struct dma_cb));
}

/*
 * dma_free_buf() - Return a DMA Buffer Descriptor to the free list
 */

void
dma_free_buf(dmabufptr)
struct dma_buf *dmabufptr;
{
	struct dma_buf *dptr, *sdptr;

	if(dmabufptr == NULL)
		return;
	dptr = dmabufptr;
	while(dptr) {
		sdptr = dptr;
		dptr = dptr->next_buf;
		kmem_free((caddr_t) sdptr, sizeof(struct dma_buf));
	}
}


/* this routine splits a a data buffer into two portions.
 * count number of bytes are left in the first buffer
 * rest of the byte are in the second buffer.
 * pointer to the incoming buffer points to the first buffer after split
 * pointer to the second buffer is returned
 * both buffers after the breakup are terminated by a descriptor
 * with 0 count and NULL next pointer
 */

struct dma_buf *
dma_buf_breakup(dmabufptr,count)
struct dma_buf *dmabufptr;
register long count;
{
	register struct dma_buf *dp1, *dp2;

	if(count == 0 || dmabufptr == NULL || dmabufptr->count == 0)
		return (NULL);
	dp1=dmabufptr;
	while((dp1 != NULL) && ((unsigned)count >= dp1->count)) {
		count -= dp1->count;
		dp1 = dp1->next_buf;
	}
	if(dp1 == NULL && count>0)
		return (NULL);
	if(count == 0) {
		if(dp1 == NULL || dp1->count == 0)
			return(NULL);
		else {
			dp2 = dmabufptr;
			while(dp2->next_buf != dp1)
				dp2 = dp2->next_buf;
			dp2->next_buf = dma_get_buf(DMA_NOSLEEP);
			dp2->next_buf->next_buf = NULL;
			dp2->next_buf->count = 0;
			return(dp1);
		}
	}else {
		if(dp1 == dmabufptr) {
			dp1 = dma_get_buf(DMA_NOSLEEP);
			dp2 = dma_get_buf(DMA_NOSLEEP);
			dp1->next_buf = dp2;
			dp1->count = dmabufptr->count - count;
			dp1->address = dmabufptr->address + count;
			dp1->next_buf = dmabufptr->next_buf;
			dmabufptr->count = count;
			dmabufptr->next_buf = dp2;
		} else {
			dp2 = dmabufptr;
			while(dp2->next_buf != dp1)
				dp2 = dp2->next_buf;
			dp2->next_buf = dma_get_buf(DMA_NOSLEEP);
			dp2 = dp2->next_buf;
			dp2->next_buf = dma_get_buf(DMA_NOSLEEP);
			dp2->count = count;
			dp2->address = dp1->address;
			dp1->count -= count;
			dp1->address += count;
		}
	}
	return(dp1);
}


/* join two linked lists of data buffers
 * first buffer is put at the head
 * ptr to head of the resultant buffer is returned
 *
 */
struct dma_buf *
dma_buf_join(db1,db2)
register struct dma_buf *db1;
register struct dma_buf *db2;
{
	struct dma_buf *ret = db1;
	register struct dma_buf *tail;
	if(db1 == NULL)
		return(db2);
	if(db2 == NULL)
		return(db1);
	if(db1->count == NULL) {
		dma_free_buf(db1);
		return(db2);
	}
	if(db2->count == 0) {
		dma_free_buf(db2);
		return(db1);
	}
	tail = db1;
	while( db1->count != 0) {
		tail = db1;
		db1 = db1->next_buf;
	}
	dma_free_buf(db1);
	tail->next_buf = db2;
	return(ret);
}

/*
 * dma_get_buf() -  Get a list of DMA Buffer Descriptors for a future DMA
 *		    operation. Return a pointer to the head of the list.
 */

struct dma_buf *
dma_get_buf(mode)
unsigned char mode;
{
	struct dma_buf *dbhead, *dbtail;

	if ((dbhead = (struct dma_buf *) kmem_zalloc(sizeof(struct dma_buf),
			mode == DMA_NOSLEEP ? KM_NOSLEEP : KM_SLEEP))
			== (struct dma_buf *) NULL) {
		return (NULL);
	}
	dbtail = dbhead;
	if ((dbtail->next_buf = (struct dma_buf *) kmem_zalloc(sizeof(struct dma_buf),
			mode == DMA_NOSLEEP ? KM_NOSLEEP : KM_SLEEP))
			== (struct dma_buf *) NULL) {
		(void) dma_free_buf (dbhead);
		return (NULL);
	}
	dbtail = dbtail->next_buf;
	dbtail->next_buf = NULL;
	return(dbhead);
}
