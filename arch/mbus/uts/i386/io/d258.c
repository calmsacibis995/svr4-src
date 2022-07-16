/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

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

#ident	"@(#)mbus:uts/i386/io/d258.c	1.3.2.2"

#ifndef lint
static char i82258_copyright[] = "Copyright 1989 Intel Corp. 464460";
#endif

#include "sys/types.h"
#include "sys/kmem.h"
#include "sys/dma.h"
#include "sys/i82258.h"
#include "sys/cmn_err.h"
#include "sys/xdebug.h"

#ifndef PRIVATE
#define PRIVATE static
#endif

extern unsigned short d258_base;
extern unsigned long d258_gmr;
extern unsigned char d258_gbr, d258_gdr;
extern unsigned long d258_chan0_base;
extern unsigned long d258_chan1_base;
extern unsigned long d258_chan2_base;
extern unsigned long d258_chan3_base;

struct d258_ccb d258_ccb[MAX_CHAN]; 	/* list of DMAC control blocks */
struct dma_cb *d258_queue[MAX_CHAN];	/* list of DMAC queued waiting for */
					/* complettion			*/


#define LOW(x)	((ushort)(x))
#define HIW(x)	((ushort)((x)>>16))
#define GET_DMA_REQRBUF(dmacb)	dmacb->reqrbufs
#define GET_DMA_TARGBUF(dmacb)	dmacb->targbufs

/*
 * d258_init() - initializes the 82258 dma chip 
 */
void
d258_init()
{

	outw(d258_base+D258_GMR, d258_gmr);
	outb(d258_base+D258_GBR, d258_gbr);
	outb(d258_base+D258_GDR, d258_gdr);

}

/*
 * d258_get_cycle_type () - Verify that the data is 4 byte aligned for 
 *			    flyby mode. Currently does not include support
 *			    for blast mode
 */
unsigned char
d258_get_best_mode(dmabufptr)
struct dma_buf *dmabufptr;
{
	struct dma_buf *tptr;
	int isburst = 1;
	int isflyby = 1;

	tptr = dmabufptr;
	while (tptr) {
		if ((tptr->address & (D258_BURST_ALGN-1))|
			(tptr->count & (D258_BURST_ALGN-1))) {
			isburst = 0;
			break;
		}
		tptr = tptr->next_buf;
	}
	if (isburst)
		return (D258_BURST_MODE);

	tptr = dmabufptr;
	/*
	 * Yeah sure, traversing the list twice seems like a waste especialy
	 * since it can be done in one traversal. However there are 2 reasons
	 * simplicity, and lists generally comprise of 1 element.
	 * For 1 traversal just remove the above stmt.
	 */
	while (tptr) {
		if ((tptr->address & (D258_FLYBY_ALGN-1))|
			(tptr->count & (D258_FLYBY_ALGN-1))) {
				isflyby = 0;
				break;
		}
		tptr = tptr->next_buf;
	}
	if (isflyby)
		return (D258_FLYBY_MODE);
	return (D258_2CYCLE_MODE); 
}



int
d258_dma_prog (dp, chan)
struct dma_cb *dp;
int chan;
{
	struct d258_ccb *ccb = &d258_ccb[chan];
	unsigned long p_addr;
	unsigned short rcmd, tcmd;
	unsigned int shift;
	unsigned long base;
	unsigned long local_gmr_value = 0;
#ifdef DEBUG	
	(*cdebugger)(DR_OTHER,NO_FRAME);	
#endif
	shift = 0;
	switch(chan) {
	default:
		return (FALSE);

	case D258_CHAN0:  
		base = d258_chan0_base;
		break;
	case D258_CHAN1:
		base = d258_chan1_base;
		break;
	case D258_CHAN2:	/* mpc solicited input channel */
		/* force read here */
		if (dp->cycles == D258_BURST_MODE)
			shift = D258_BURST_SHIFT; 
		else if (dp->cycles ==  D258_FLYBY_SHIFT)
			shift = D258_FLYBY_SHIFT;
		dp->command = DMA_CMD_READ;

		base = d258_chan2_base;
		break;
	case D258_CHAN3:  	
		/* solicited output chan for 386/1xx boards */
		if (dp->cycles == D258_BURST_MODE)
			shift = D258_BURST_SHIFT; 
		else if (dp->cycles ==  D258_FLYBY_SHIFT)
			shift = D258_FLYBY_SHIFT;
		/* these boards do not support mulitipexing */
		/* force write here */
		dp->command = DMA_CMD_WRITE;
		base = d258_chan3_base;
		break;
	}

	switch (dp->cycles) {
	default:
		return (FALSE);
	case D258_BURST_MODE:
	case D258_FLYBY_MODE:
		d258_fixup_dmabuf (dp, shift);
		local_gmr_value = D258_GMR_1CYCLE << chan; 
		if (dp->command == DMA_CMD_READ) {
			rcmd = d258_mk_cmd(dp->targ_type, dp->targ_step, dp->targ_path);
		 	ccb->command = D258_IN1C_CMD | rcmd; 
			ccb->dst_ptr = (unsigned long)base;
			ccb->src_ptr = (unsigned long)kvtophys((caddr_t)
				&dp->targbufs->count);
		} else {
			rcmd = d258_mk_cmd(dp->reqr_type, dp->reqr_step, dp->reqr_path);
			ccb->command = D258_OUT1C_CMD | rcmd; 
			ccb->dst_ptr = (unsigned long)base;
			ccb->src_ptr = (unsigned long)kvtophys((caddr_t)
				&dp->reqrbufs->count);
		}
		break;
	case D258_2CYCLE_MODE:
		d258_fixup_dmabuf (dp, shift);
		rcmd = d258_mk_cmd(dp->reqr_type, dp->reqr_step, dp->reqr_path);
		tcmd = d258_mk_cmd(dp->targ_type, dp->targ_step, dp->targ_path);
		local_gmr_value = D258_GMR_2CYCLE << chan; 
		if (dp->command == DMA_CMD_READ) {
	  		ccb->command = D258_IN2C_CMD | 
				(tcmd << DST_SHIFT) | rcmd;
			ccb->dst_ptr = (unsigned long)kvtophys((caddr_t)
				&dp->targbufs->count);
			ccb->src_ptr = (long)base;
		} else {
			ccb->command = D258_OUT2C_CMD | 
				(tcmd << DST_SHIFT) | rcmd;
			ccb->dst_ptr = (long)base;
			ccb->src_ptr = (unsigned long)kvtophys((caddr_t)
				&dp->reqrbufs->count);
		}
		break;
	}
	/* save the dma control block for restoration */
	
	d258_queue[chan] = dp;

	/* load 82258 DMAC GMR and CPR regs with required values */
	p_addr = (unsigned long)(kvtophys((caddr_t)&ccb->command));
	
	d258_gmr = inw(d258_base+D258_GMR) & ~(D258_GMR_1CYCLE << chan);
	outw(d258_base+D258_GMR,d258_gmr|local_gmr_value);

	outw(d258_base+D258_CPRL(chan), LOW(p_addr));
	outw(d258_base+D258_CPRH(chan), HIW(p_addr));
	return (TRUE);
}

/*
 * d258_dma_swsetup () - Setup Software initiated a DMA request.
 * Currently Unsupported.
 * This is example code If SBX's are introduced or if CHAN0 is an SBX hook
 * as in 386/1xx boards (not hiint) then this code should be modified (and/or
 * tested.
 */
int
d258_dma_swsetup (dp, chan)
struct dma_cb *dp;
int chan;
{
	struct d258_ccb *ccb = &d258_ccb[chan];
	unsigned long p_addr;
	unsigned short rcmd, tcmd;
	unsigned long local_gmr_value = 0;

	switch(chan) {
	default:
		return (FALSE);

	case D258_CHAN0: 	
	case D258_CHAN2: 	
	case D258_CHAN3:
		return (FALSE);

	case D258_CHAN1:  	
		/* verify if sbx channel's are software configured */
		if (dp->cycles != D258_2CYCLE_MODE) 
			return (FALSE);
		rcmd = d258_mk_cmd(dp->reqr_type, dp->reqr_step, dp->reqr_path);
		tcmd = d258_mk_cmd(dp->targ_type, dp->targ_step, dp->targ_path);
		ccb->command = D258_SBX_CMD | (tcmd << DST_SHIFT) | rcmd;
		local_gmr_value = D258_GMR_2CYCLE << chan;
		if (dp->command == DMA_CMD_READ) {
			ccb->dst_ptr = (unsigned long)kvtophys((caddr_t)
					dp->targbufs->count);
			ccb->src_ptr = (long)d258_chan1_base;
			local_gmr_value = D258_GMR_2CYCLE << chan;
		} else {
			ccb->src_ptr = (unsigned long)kvtophys((caddr_t)
					dp->reqrbufs->count);
			ccb->dst_ptr = (long)d258_chan1_base;
		}
		break;

	}
	/* Save the dma control info for use in the interrupt procedure */

	d258_queue[chan] = dp;
 
	/* load 82258 DMAC GMR and CPR regs with required values */
	p_addr = (unsigned long)(kvtophys((caddr_t)&ccb->command));
	inw(d258_base+D258_GMR,d258_gmr);
	outw(d258_base+D258_GMR,d258_gmr|local_gmr_value);

	outw(d258_base+D258_CPRL(chan), LOW(p_addr));
	outw(d258_base+D258_CPRH(chan), HIW(p_addr));
	return (TRUE);

}

/*
 * d258_dma_swstart () - Start Software initiate a DMA request.
 * Currently unsupported.
 */
int
d258_dma_swstart (chan)
int chan;
{
	outb(d258_base+D258_GCR, (unchar)START_CH(chan));
	return (TRUE);
}


/*
 * d258_dma_stop() - This routine stops an 82258 DMA channel
 */

int
d258_dma_stop (chan)
int chan;
{
	uint shift = 0;
	struct dma_cb *dp;

	outb(d258_base+D258_GCR, (unchar)STOP_CH(chan));
	dp = d258_queue[chan];
	switch(chan) {
	default:
		return (FALSE);
	case D258_CHAN0:  
	case D258_CHAN1:
		break;
	case D258_CHAN2:	/* mpc solicited input channel */
		if (dp->cycles == D258_BURST_MODE)
			shift = D258_BURST_SHIFT; 
		else if (dp->cycles ==  D258_FLYBY_SHIFT)
			shift = D258_FLYBY_SHIFT;
		break;
	case D258_CHAN3:  	
		/* solicited output chan for 386/1xx boards */
		if (dp->cycles == D258_BURST_MODE)
			shift = D258_BURST_SHIFT; 
		else if (dp->cycles ==  D258_FLYBY_SHIFT)
			shift = D258_FLYBY_SHIFT;
		break;
	}

	(void) d258_restore_dmabuf(dp,shift);
	return (TRUE);
}

/*
 * d258_intr () - Process an interrupt if the 82258 is configured so that it
 *	        - it interrupts the CPU i.e if the EOD line of the 82258 is
 *		- jumpered appropriately. 
 *                Currently unsupported.
 */
int
d258_intr (level)
int level;
{
	struct dma_cb *dp;
	int chan;

	/* determin from the General status register which channel caused 
	 * the interrupt */
	chan = d258_find_chan ();
	if ((chan == D258_CHAN2) || (chan == D258_CHAN3) || (chan == D258_CHAN0)
		|| (d258_queue[chan] == NULL)) {
		cmn_err (CE_NOTE, "Spurious DMA interrupt");
		return (FALSE);
	}
	dp = d258_queue[chan];
	if (dp->proc) 
		(dp->proc)(dp->procparms);
	return (TRUE);

}

/*
 * d258_dma_enable() - Primes the General command register to begin transfers.
 */
int
d258_dma_enable (chan)
int chan;
{
	outb(d258_base+D258_GCR, (unchar)START_CH(chan));
	return (TRUE);
}

/*
 * d258_dma_disable() - Programs the DMAC to stop activity.
 */
int
d258_dma_disable (chan)
int chan;
{
	outb(d258_base+D258_GCR, (unchar)STOP_CH(chan));
	return (TRUE);
}

/* ARGSUSED */
int
d258_get_chan_stat(dmastat, chan)
struct dma_stat *dmastat;
int chan;
{
	return (FALSE);
}


unsigned short
d258_mk_cmd (type, step, path)
unsigned char type, step, path;
{
	unsigned char stype, sstep, srwb;

	stype = (type == DMA_TYPE_MEM) ? D258_TYPE_MEM : D258_TYPE_IO;
	switch (step) {
	case DMA_STEP_DEC:
		sstep = D258_STEP_DEC;
		break;
	case DMA_STEP_HOLD:
		sstep = D258_STEP_HOLD;
		break;
	default:
		sstep = D258_STEP_INC;
	}
	srwb = (path == DMA_PATH_8) ? D258_WB_8 : D258_WB_16;
	return (srwb|sstep|stype);
}

/*
 * This routine sets up the physical addresses of the data chain. Also
 * For 386/1xx, addr and count are divided by 2 if data is aligned.  
 */
void
d258_fixup_dmabuf(dmacbptr, shift)
struct dma_cb *dmacbptr;
unsigned int shift;
{

	struct dma_buf *dbpt;

	dbpt = (dmacbptr->command == DMA_CMD_READ) ? GET_DMA_TARGBUF(dmacbptr) :
										GET_DMA_REQRBUF(dmacbptr);
	while (dbpt->count != 0) {
		dbpt->physical = kvtophys(dbpt->next_buf) + sizeof(dbpt->reserved);
		dbpt->address >>= shift;
		dbpt->count >>= shift;
		dbpt = dbpt->next_buf;
	}
}

/*
 * For 386/1xx, addr and count are divided by 2 if data is aligned.  
 * We undo this here.
 */
void
d258_restore_dmabuf (dmacbptr,shift)
struct dma_cb *dmacbptr;
uint shift;
{
	struct dma_buf *dbpt;

	dbpt = (dmacbptr->command == DMA_CMD_READ) ? GET_DMA_TARGBUF(dmacbptr) :
										GET_DMA_REQRBUF(dmacbptr);

	/* For 386/1xx, addr and count are divided by shift 
	 * if data is aligned.  We undo this here.
	 */
	while (dbpt->count != 0) {
		dbpt->address <<= shift;
		dbpt->count <<= shift;
		dbpt = dbpt->next_buf;
	}	
	return;

}

/*
 * d258_find_chan - routine to determine which channel caused an interrupt.
 */
int
d258_find_chan ()
{
	unsigned short d258_gsr;
	int chan;

	d258_gsr = inb(d258_base + D258_GSR);
	chan = (d258_gsr & D258_CHAN0_INT_MASK) ? D258_CHAN0 : D258_CHAN1;
	return (chan);  
}
