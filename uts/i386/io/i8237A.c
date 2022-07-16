/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:i8237A.c	1.3.2.1"

/*      Copyright (c) 1988, 1989 Intel Corp.            */
/*        All Rights Reserved   */
/*
 *      INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *      This software is supplied under the terms of a license
 *      agreement or nondisclosure agreement with Intel Corpo-
 *      ration and may not be copied or disclosed except in
 *      accordance with the terms of that agreement.
*/

#include <sys/types.h>
#include "sys/dma.h"
#include "sys/bootinfo.h"

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

/*
macro to convert one-bit-high value to log2 of that value
*/
#define d37A_log2(b) \
(((b)==0x00)?-1:(((b)&0x0F)?(((b)&0x03)?(((b)&0x01)?0:1):(((b)&0x04)?2:3)):\
(((b)&0x30)?(((b)&0x10)?4:5):(((b)&0x40)?6:7))))

/*
macros to get Bytes from longs
*/
#define BYTE0(x) ((x) & 0x0FF)
#define BYTE1(x) (((x)>>8) & 0x0FF)
#define BYTE2(x) (((x)>>16) & 0x0FF)
#define BYTE3(x) (((x)>>24) & 0x0FF)

#define TOLONG(x, y) ((x)|((y) <<16))

/*
 * data structures for programming the DMAC
 */
struct d37A_chan_reg_addr chan_addr[] = { D37A_BASE_REGS_VALUES };

int	Eisa_dma = FALSE;
int	Dma_addr_28 = FALSE;

extern int	eisa_bus;

struct dma_buf *eisa_curbuf[NCHANS];

/****
*** Routine: d37A_dma_disable()
*** Purpose: Prevent the DMAC from responding to external hardware
***          requests for DMA service on the given channel
*** Caller:  dma_disable()
*** Calls:   d37A macros
****/
void
d37A_dma_disable(chan)
register int	chan;
{
	/* mask off subsequent HW requests */

	dprintf(("d37A_dma_disable:chan = %x, mask_reg = %x, val = %x\n", chan, chan_addr[chan].mask_reg, DMA_SETMSK | chan &
	    0x03));
	outb(chan_addr[chan].mask_reg, DMA_SETMSK | chan & 0x03);
}


/****
*** Routine: d37A_get_best_mode()
*** Purpose: stub routine - determine optimum transfer method
*** Caller:  dma_get_best_mode().
*** Calls:   
****/
unsigned char	d37A_get_best_mode(dmacbptr)
struct dma_cb *dmacbptr;
{
	return(DMA_CYCLES_2);
}


/****
*** Routine: d37A_intr()
*** Purpose: stub routine 
*** Caller:  dma_intr().
*** Calls:   
****/
void
d37A_intr(lev)
int	lev;
{
	void dEISA_setchain();
	register int	i, st;

	if (Eisa_dma == TRUE) {
		st = inb(EISA_DMAIS) & 0xef;    /* channel 4 can't interrupt */
		i = 0;
		while (st) {
			if (st & 1)
				dEISA_setchain(i);
			st >>= 1;
			i++;
		}
	}

	return;
}


/****
*** Routine: dEISA_setchain()
*** Purpose: Set next buffer address/count from chain
*** Caller:  d37A_intr()
*** Calls:   d37A macros
****/
void
dEISA_setchain(chan)
int	chan;
{
	void d37A_write_target();
	void d37A_write_count();
	register struct dma_buf *dp;

	dprintf(("dEISA_setchain(%d)\n", chan));
	if ((dp = eisa_curbuf[chan]) == (struct dma_buf *)0) {
		/*
                 *  clear chain enable bit
                 */
		outb(chan_addr[chan].scm_reg, chan);
		return;
	}
	dprintf(("next buffer:%xbytes @%x\n", dp->count, dp->address));
	outb(chan_addr[chan].scm_reg, chan | EISA_ENCM);
	d37A_write_target(dp->address, chan);
	outb(chan_addr[chan].hpage_reg, BYTE3((long)dp->address));
	if( Eisa_dma == TRUE ) {
		d37A_write_count(((dp->count_hi << 16) | dp->count), chan);
	}
	else {
		d37A_write_count(dp->count, chan);
	}
	outb(chan_addr[chan].scm_reg, chan | EISA_ENCM | EISA_CMOK);
	eisa_curbuf[chan] = dp->next_buf;
	return;
}


/****
*** Routine: d37A_write_target()
*** Purpose: write the 24-bit target address into the Base Target
***          Register for the indicated channel.
*** Caller:  d37A_prog_chan(), d37A_dma_swsetup().
*** Calls:   d37A macros
****/

void
d37A_write_target(targ_addr, chan)
register unsigned long	targ_addr;
register int	chan;
{
	/* write the target device address, one byte at a time */
	dprintf(("d37A_write_target: writing Target Address %x to channel %d.\n",
	    targ_addr, chan));

	outb(chan_addr[chan].ff_reg, 0);        /* set flipflop */
	tenmicrosec();
	/* write the base address */
	outb(chan_addr[chan].addr_reg, BYTE0(targ_addr));
	tenmicrosec();
	outb(chan_addr[chan].addr_reg, BYTE1(targ_addr));
	tenmicrosec();
	/* set page register */
	outb(chan_addr[chan].page_reg, BYTE2(targ_addr));

	/* Set High Page Register */
	if (Eisa_dma == TRUE) {
		outb(chan_addr[chan].hpage_reg, BYTE3(targ_addr));
	} else if (Dma_addr_28) {
		outb(chan_addr[chan].hpage_reg, (0x0F & BYTE3(targ_addr)));
	}

	tenmicrosec();
}


/****
*** Routine: d37A_read_target()
*** Purpose: read the 24-bit target address from the Current Target
***          Register for the indicated channel.
*** Caller:  d37A_get_chan_stat().
*** Calls:   d37A macros
****/

unsigned long	d37A_read_target(chan)
register int	chan;
{
	/* read the 24 bit target address a byte at a time */
	unsigned long	targ_addr;

	dprintf(("d37A_read_target: reading channel %d's Target Address.\n",
	    chan));

	outb(chan_addr[chan].ff_reg, 0);        /* set flipflop */
	/* read the address */
	targ_addr = inb(chan_addr[chan].addr_reg);
	targ_addr |= inb(chan_addr[chan].addr_reg) << 8;
	targ_addr |= inb(chan_addr[chan].page_reg) << 16;

	/***
        ** Always read the high byte, for compatible accesses
        ** it will be 0 anyway
        ***/
	if (Eisa_dma == TRUE) {
		targ_addr |= inb(chan_addr[chan].hpage_reg) << 24;
	} else if (Dma_addr_28) {
		targ_addr |= (0x0F & inb(chan_addr[chan].hpage_reg)) << 24;
	}

	dprintf(("d37A_read_target: channel %d's Target Address= %x.\n",
	    chan, (unsigned long) targ_addr));
	return(targ_addr);
}


/****
*** Routine: d37A_write_count()
*** Purpose: write the 16-bit count into the Base Count Register for
***          the indicated channel.
*** Caller:  d37A_prog_chan(), d37A_dma_swsetup()
*** Calls:   d37A macros
****/

void
d37A_write_count(count, chan)
register unsigned long	count;
register int	chan;
{
	/* write the transfer byte count, one byte at a time */
	dprintf(("d37A_write_count: writing Count %x to channel %d.\n",
	    count, chan));

	/* check validity of channel number */

	outb(chan_addr[chan].ff_reg, 0);        /* set flipflop */
	tenmicrosec();
	/* write the word count */
	outb(chan_addr[chan].cnt_reg, BYTE0(count));
	tenmicrosec();
	outb(chan_addr[chan].cnt_reg, BYTE1(count));
	tenmicrosec();

	if (Eisa_dma == TRUE)
		outb(chan_addr[chan].hcnt_reg, BYTE2(count));

}


/****
*** Routine: d37A_read_count()
*** Purpose: read the 16-bit count from the Current Count Register for
***          the indicated channel
*** Caller:  d37A_get_chan_stat()
*** Calls:   d37A macros
****/

unsigned long	
d37A_read_count(chan)
register int	chan;
{
	/* read the 3 byte count address a byte at a time */
	unsigned long	count;

	dprintf(("d37A_read_count: reading channel %d's Count.\n", chan));

	/* read the count */

	outb(chan_addr[chan].ff_reg, 0);        /* set flipflop */
	count = inb(chan_addr[chan].cnt_reg);
	count |= inb(chan_addr[chan].cnt_reg) << 8;

	if (Eisa_dma == TRUE) {
		count |= inb(chan_addr[chan].hcnt_reg) << 16;
	}

	dprintf(("d37A_read_count: channel %d's Count= %x.\n",
	    chan, count));
	return(count);
}


/****
*** Routine: d37A_init()
*** Purpose: initializes the 8237A.
*** Caller:  dma_init()
*** Calls:   d37A macros, p37A_init()
****/

void
d37A_init()
{

	char i_val;

	/***
        ** Determine machine type. If the ROM Machine ID
        ** indicates we are running on a machine that
        ** support 28 Bit DMA, set the flag accordingly.
        **
        ****
        **
        ** NOTE: As of 03/08/90, 28 Bit DMA support is not
        **       being allowed. This is due to a hardware
        **       bug in the INTEL System Boards that support
        **       this feature. If/When INTEL provides a hardware
        **       solution (including a way for this software to
        **       know this fix is in place), the Dma_addr_28 flag
        **       can be turned on for the appropriate machines.
	**
	**	 05/12/90: Added logic to detect the "enhanced"
	**	 Cascade 4 motherboards (working 28 bit DMA).
        ***/
        if ((bootinfo.id[0] == 'I') && (bootinfo.id[1] == 'D') &&
            (bootinfo.id[2] == 'N') && (bootinfo.id[3] == 'O')) {
                switch ( bootinfo.id[4] ) {
		case 'F':
			Dma_addr_28 = FALSE;
			break;
		case 'G':
			i_val = inb( 0x79 );
			outb( 0x79, ( i_val & 0xfe ) );
			if( inb( 0x79 ) & 0x01 ) {
				Dma_addr_28 = TRUE;
			}
			else {
				Dma_addr_28 = FALSE;
			}
			break;
                default:
                        Dma_addr_28 = FALSE;
                        break;
                }

        }

        if( eisa_bus == 1 ) {
                Eisa_dma = TRUE;
        }

        return;
}


/****
*** Routine: d37A_mode_set()
*** Purpose: program the Mode registers of the
***          DMAC for a subsequent hardware-initiated transfer. 
*** Caller:  dma_prog_chan()
*** Calls:  
****/
void
d37A_mode_set(dmacbptr, chan)
register struct dma_cb *dmacbptr;
register int	chan;
{
	unsigned char	mode;

	mode = chan & 0x03;

	if (dmacbptr->command == DMA_CMD_VRFY)
		mode |= DMA_VR << 2;
	else if (dmacbptr->command == DMA_CMD_READ)
		mode |= DMA_RD << 2;
	else
		mode |= DMA_WR << 2;

	if (dmacbptr->bufprocess == DMA_BUF_AUTO)
		mode |= 1 << 4;

	if (dmacbptr->targ_step == DMA_STEP_DEC)
		mode |= 1 << 5;

	if (dmacbptr->trans_type == DMA_TRANS_SNGL)
		mode |= 1 << 6;
	else if (dmacbptr->trans_type == DMA_TRANS_BLCK)
		mode |= 2 << 6;
	else if (dmacbptr->trans_type == DMA_TRANS_CSCD)
		mode |= 3 << 6;

	dprintf(("set mode: chan= %d mode = %x, mode_reg = %x \n", chan, mode, chan_addr[chan].mode_reg));
	outb(chan_addr[chan].mode_reg, mode);

	if (Eisa_dma == TRUE) {
		mode &= 3;
		switch (dmacbptr->targ_path) {

		case DMA_PATH_16:
			mode |= EISA_DMA_16 << 2;
		case DMA_PATH_8:
			/* mode |= EISA_DMA_8 << 2; */
			break;
		case DMA_PATH_32:
			mode |= EISA_DMA_32 << 2;
			break;
		case DMA_PATH_16B:
			mode |= EISA_DMA_16B << 2;
			break;

		}
		mode |= dmacbptr->cycles << 4;
		outb(chan_addr[chan].xmode_reg, mode);
		dprintf(("ext mode: chan = %d, mode = %x, reg = %x\n", chan, mode, chan_addr[chan].xmode_reg));
	}
}


/****
*** Routine: d37A_prog_chan()
*** Purpose: program the Mode registers and the Base registers of the
***          DMAC for a subsequent hardware-initiated transfer. 
*** Caller:  dma_prog_chan()
*** Calls:   d37A_write_target(), d37A_write_count(), d37A macros.
****/

int
d37A_prog_chan(dmacbptr, chan)
register struct dma_cb *dmacbptr;
register int	chan;
{
	void dEISA_setchain();
	register unsigned long	addr;
	register int	oldpri;
	long tcount;

	/* prepare for a hardware operation on the specified channel */
	dprintf(("d37A_prog_chan: programming channel %d, dmacbptr= %x.\n",
	    chan, (unsigned long) dmacbptr));

	if (chan == DMA_CH4 || chan == DMA_CH0 ) {
		dprintf(("d37A_prog_chan: channel %d not supported.\n", chan));
		return(FALSE);
	}

	if (!Eisa_dma == TRUE && dmacbptr->targ_type == DMA_TYPE_MEM ) {
		dprintf(("d37A_prog_chan: memory to memory mode not supported.\n"));
		return(FALSE);
	}

	/* keep the channel quiet while programming it */
	d37A_dma_disable(chan);

	switch (chan) {
	case DMA_CH1:
	case DMA_CH2:
	case DMA_CH3:
		if (!Eisa_dma == TRUE && (dmacbptr->trans_type == DMA_TRANS_CSCD || dmacbptr->targ_path != DMA_PATH_8)) {
			dprintf(("d37A_prog_chan: channel %d not programmed.\n", chan));
			return(FALSE);
		}

		if( dmacbptr->targbufs->count_hi == 0 ) {
			dmacbptr->targbufs->count -= 1;
		}
		else {
			tcount = (dmacbptr->targbufs->count_hi << 16) | dmacbptr->targbufs->count;
			--tcount;
			dmacbptr->targbufs->count = tcount & 0xFFFF;
			dmacbptr->targbufs->count_hi = (tcount >> 16) & 0xFFFF;
		}
		
		break;

	case DMA_CH5:
	case DMA_CH6:
	case DMA_CH7:
		if (dmacbptr->trans_type == DMA_TRANS_CSCD || dmacbptr->targ_path != DMA_PATH_16) {
			dprintf(("d37A_prog_chan: channel %d not programmed.\n", chan));
			return(FALSE);
		}

		if ( (addr = dmacbptr->targbufs->address) & 0x01) {
			dprintf(("d37A_prog_chan: channel %d not programmed.\n", chan));
			return(FALSE);
		}

		/***
		** Channels 5, 6 and 7 require the count to
		** be the number of 16 bit words to be transferred
		** rather than 1 less than the number needed as is
		** the case for channels 1, 2 and 3.
		***/
		if( dmacbptr->targbufs->count_hi == 0 ) {
			dmacbptr->targbufs->count >>= 1;
		}
		else {
			tcount = (dmacbptr->targbufs->count_hi << 16) | dmacbptr->targbufs->count;
			tcount >>= 1;
			dmacbptr->targbufs->count = tcount & 0xFFFF;
			dmacbptr->targbufs->count_hi = (tcount >> 16) & 0xFFFF;
		}

		addr = (addr & ~0x1ffff) | ((addr & 0x1ffff) >> 1);
		dmacbptr->targbufs->address = (paddr_t)addr;

		if (dmacbptr->trans_type != DMA_TRANS_BLCK)
			dmacbptr->trans_type = DMA_TRANS_BLCK;

		break;

	default:
		dprintf(("d37A_prog_chan: channel %d not programmed.\n", chan));
		return(FALSE);
	}

	oldpri = splhi();
	d37A_mode_set(dmacbptr, chan);
	d37A_write_target(dmacbptr->targbufs->address, chan);

	if( dmacbptr->targbufs->count_hi == 0 ) {
		d37A_write_count(dmacbptr->targbufs->count, chan);
	}
	else {
		d37A_write_count(((dmacbptr->targbufs->count_hi << 16) | dmacbptr->targbufs->count), chan);
	}

	if (Eisa_dma == TRUE && dmacbptr->bufprocess == DMA_BUF_CHAIN) {
		eisa_curbuf[chan] = dmacbptr->targbufs;
		dEISA_setchain(chan);
	}

	splx(oldpri);
	return(TRUE);
}


/****
*** Routine: d37A_dma_swsetup()
*** Purpose: program the Mode registers and the Base register for the
***          specified channel.
*** Caller:  dma_swsetup()
*** Calls:   d37A_write_target(), d37A_write_count(), d37A macros.
****/

int	d37A_dma_swsetup(dmacbptr, chan)
register struct dma_cb *dmacbptr;
register int	chan;
{
	void dEISA_setchain();
	register unsigned long	addr;
	register int	oldpri;
	long tcount;

	/* prepare for a software operation on the specified channel */
	dprintf(("d37A_dma_swsetup: set up channel %d, dmacbptr= %x.\n",
	    chan, (unsigned long) dmacbptr));

	if (chan == 4 || chan == 0) {
		dprintf(("d37A_dma_swsetup: channel %d not supported.\n", chan));
		return(FALSE);
	}

	/* MUST BE IN BLOCK MODE FOR SOFTWARE INITIATED REQUESTS */
	if (dmacbptr->trans_type != DMA_TRANS_BLCK)
		dmacbptr->trans_type = DMA_TRANS_BLCK;

	/* keep channel quiet while programming */
	d37A_dma_disable(chan);

	switch (chan) {
	case DMA_CH1:
	case DMA_CH2:
	case DMA_CH3:
		if (!Eisa_dma == TRUE && (dmacbptr->trans_type == DMA_TRANS_CSCD || dmacbptr->targ_path != DMA_PATH_8)) {
			dprintf(("d37A_prog_chan: channel %d not programmed.\n", chan));
			return(FALSE);
		}

		if( dmacbptr->targbufs->count_hi == 0 ) {
			dmacbptr->targbufs->count -= 1;
		}
		else {
			tcount = ((dmacbptr->targbufs->count_hi << 16) | dmacbptr->targbufs->count);
			tcount -= 1;
			dmacbptr->targbufs->count = tcount & 0xFFFF;
			dmacbptr->targbufs->count_hi = (tcount >> 16) & 0xFFFF;
		}
		break;

	case DMA_CH5:
	case DMA_CH6:
	case DMA_CH7:
		if (dmacbptr->trans_type == DMA_TRANS_CSCD || dmacbptr->targ_path != DMA_PATH_16) {
			dprintf(("d37A_prog_chan: channel %d not programmed.\n", chan));
			return(FALSE);
		}

		if ( (addr = dmacbptr->targbufs->address) & 0x01) {
			dprintf(("d37A_prog_chan: channel %d not programmed.\n", chan));
			return(FALSE);
		}

		/***
		** Channels 5, 6 and 7 require the count to
		** be the number of 16 bit words to be transferred
		** rather than 1 less than the number needed as is
		** the case for channels 1, 2 and 3.
		***/
		if( dmacbptr->targbufs->count_hi == 0 ) {
			dmacbptr->targbufs->count >>= 1;
		}
		else {
			tcount = (dmacbptr->targbufs->count_hi << 16) | dmacbptr->targbufs->count;
			tcount >>= 1;
			dmacbptr->targbufs->count = tcount & 0xFFFF;
			dmacbptr->targbufs->count_hi = (tcount >> 16) & 0xFFFF;
		}

		addr = (addr & ~0x1ffff) | ((addr & 0x1ffff) >> 1);
		dmacbptr->targbufs->address = (paddr_t)addr;

		if (dmacbptr->trans_type != DMA_TRANS_BLCK)
			dmacbptr->trans_type = DMA_TRANS_BLCK;

		break;
	default:
		dprintf(("d37A_dma_swsetup: channel %d not set up.\n", chan));
		return(FALSE);
	};

	oldpri = splhi();
	d37A_mode_set(dmacbptr, chan);
	d37A_write_target(dmacbptr->targbufs->address, chan);

	if( Eisa_dma == TRUE ) {
		d37A_write_count(((dmacbptr->targbufs->count_hi << 16) | dmacbptr->targbufs->count), chan);
	}
	else {
		d37A_write_count(dmacbptr->targbufs->count, chan);
	}

	if (Eisa_dma == TRUE && dmacbptr->bufprocess == DMA_BUF_CHAIN) {
		eisa_curbuf[chan] = dmacbptr->targbufs;
		dEISA_setchain(chan);
	}

	splx(oldpri);

	return(TRUE);
}


/****
*** Routine: d37A_dma_enable()
*** Purpose: Enable to DMAC to respond to hardware requests for DMA
***          service on the specified channel.
*** Caller:  dma_enable()
*** Calls:   d37A macros
****/

void
d37A_dma_enable(chan)
register int	chan;
{
	/* enable a HW request to be seen */
	dprintf(("d37A_dma_enable:chan = %x ,mask_reg = %x, val = %x\n", chan, chan_addr[chan].mask_reg, chan & 0x03));

	outb(chan_addr[chan].mask_reg, chan & 0x03);
}


/****
*** Routine: d37A_dma_swstart()
*** Purpose: SW start transfer setup on the indicated channel.
*** Caller:  dma_swstart()
*** Calls:   d37A_dma_enable(), d37A macros
****/

void
d37A_dma_swstart(chan)
register int	chan;
{
	/* enable the channel and kick it into gear */
	dprintf(("d37A_dma_swstart: software start channel %d.\n", chan));

	d37A_dma_enable(chan);
	outb(chan_addr[chan].reqt_reg, DMA_SETMSK | chan);        /* set request bit */
}


/****
*** Routine: d37A_dma_stop()
*** Purpose: Stop any activity on the indicated channel.
*** Caller:  dma_stop()
*** Calls:   d37A macros
****/

int	d37A_dma_stop(chan)
register int	chan;
{
	/* stop whatever is going on channel chan */
	dprintf(("d37A_dma_stop: stop channel %d.\n", chan));

	d37A_dma_disable(chan);
	outb(chan_addr[chan].reqt_reg, chan & 0x03);    /* reset request bit */
	return(TRUE);
}


/****
*** Routine: d37A_get_chan_stat()
*** Purpose: retrieve the Current Address and Count registers for the
***          specified channel.
*** Caller:  dma_get_chan_stat()
*** Calls:   d37A_read_target(), d37A_read_count().
****/
void
d37A_get_chan_stat(dmastat, chan)
register struct dma_stat *dmastat;
register int	chan;
{
	long tcount;

	/* read the Current Registers for channel chan */
	dprintf(("d37A_get_chan_stat: retrieve channel %d's status.\n", chan));

	dmastat->targaddr = d37A_read_target(chan);

	dmastat->count = d37A_read_count(chan) & 0xFFFF;
	dmastat->count_hi = (d37A_read_count(chan) >> 16) & 0xFFFF;

	switch (chan) {
	case DMA_CH1:                                                                   
	case DMA_CH2:
	case DMA_CH3:
		/***
		** Remember, we decremented the user supplied count
		** for these channels, so we have to add one back
		** to provide a consistent view to the user.
		***/
		tcount = (dmastat->count_hi << 16) | dmastat->count;
		++tcount;
		dmastat->count = tcount & 0xFFFF;
		dmastat->count_hi = (tcount << 16) & 0xFFFF;
		break;
	}

	dprintf(("d37A_get_chan_stat: channel %d's status:\n", chan));
	dprintf(("d37A_get_chan_stat:\ttarget=    %x\n", dmastat->targaddr));
	dprintf(("d37A_get_chan_stat:\tcount=     %x\n", TOLONG(dmastat->count, dmastat->count_hi)));
}


