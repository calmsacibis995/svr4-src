/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbmt:sundev/screg.h	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#define	HOST_ADDR	0x00	/* 0x80 is right but Sysgen violates spec */

#define SC_RESET_DELAY		4000000	/*  1 us  ( 4 Sec.) */
#define SC_LONG_WAIT		3000000	/* 10 us  (30 Sec.) */
#define SC_WAIT_COUNT		200000	/* 10 us  ( 2 Sec.) */
#define SC_SHORT_WAIT		25000	/* 10 us  (.25 Sec.) */


#if	defined(sun2)  ||  defined(sun3)
/* 
 * SCSI Sun host adapter control registers for Sun-2 and Sun-3.
 */
#define	SET_DMA_ADDR(har,val)	(har)->dma_addr = val;
#define	GET_DMA_ADDR(har)	(har)->dma_addr;

struct	scsi_ha_reg {		/* host adapter (I/O space) registers */
	u_char	data;		/* data register */
	u_char	unused;
	u_char	cmd_stat;	/* command/status register */
	u_char	unused2;
	u_short	icr;		/* interface control register */
	u_short	unused3;
	u_long	dma_addr;	/* dma base address */
	u_short	dma_count;	/* dma count register */
	u_char	unused4;
	u_char	intvec;		/* interrupt vector for VMEbus versions */
};
#endif	defined(sun2)  ||  defined(sun3)


#ifdef	sun4
/* 
 * SCSI Sun host adapter control registers for Sun-4.
 */
#define	SET_DMA_ADDR(har,val)	(har)->dma_addrh = (val >> 16);\
				(har)->dma_addrl = (val & 0xffff);
#define	GET_DMA_ADDR(har)	(((har)->dma_addrh  << 16) | ((har)->dma_addrl))

struct	scsi_ha_reg {		/* host adapter (I/O space) registers */
	u_char	data;		/* data register */
	u_char	unused;
	u_char	cmd_stat;	/* command/status register */
	u_char	unused2;
	u_short	icr;		/* interface control register */
	u_short	unused3;
	u_short	dma_addrh;	/* dma base address */
	u_short	dma_addrl;
	u_short	dma_count;	/* dma count register */
	u_char	unused4;
	u_char	intvec;		/* interrupt vector for VMEbus versions */
};
#endif	sun4


/*
 * bits in the interface control register 
 */
#define	ICR_PARITY_ERROR	0x8000
#define	ICR_BUS_ERROR		0x4000
#define	ICR_ODD_LENGTH		0x2000
#define	ICR_INTERRUPT_REQUEST	0x1000
#define	ICR_REQUEST		0x0800
#define	ICR_MESSAGE		0x0400
#define	ICR_COMMAND_DATA	0x0200	/* command=1, data=0 */
#define	ICR_INPUT_OUTPUT	0x0100	/* input=1, output=0 */
#define	ICR_PARITY		0x0080
#define	ICR_BUSY		0x0040
/* Only the following bits may usefully be set by the CPU */
#define	ICR_SELECT		0x0020
#define	ICR_RESET		0x0010
#define	ICR_PARITY_ENABLE	0x0008
#define	ICR_WORD_MODE		0x0004
#define	ICR_DMA_ENABLE		0x0002
#define	ICR_INTERRUPT_ENABLE	0x0001

/*
 * Compound conditions of icr bits message, command/data and input/output.
 */
#define	ICR_COMMAND	(ICR_COMMAND_DATA)
#define	ICR_STATUS	(ICR_COMMAND_DATA | ICR_INPUT_OUTPUT)
#define	ICR_MESSAGE_IN	(ICR_MESSAGE | ICR_COMMAND_DATA | ICR_INPUT_OUTPUT)
#define	ICR_BITS	(ICR_MESSAGE | ICR_COMMAND_DATA | ICR_INPUT_OUTPUT)
