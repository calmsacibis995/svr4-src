/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:sys/iohw.h	1.1"
/*	Convergent Technologies - System V - Jun 1983	*/

#ifndef iohw_h
#define iohw_h

#include <sys/types.h>
#include <sys/target.h>

/*	Video Bitmap Definitions		*/

#define VIDMEM		((ushort *)0x420000)	/* mem addr	*/
#define VIDWIDTH	720				/* pixels	*/
#define VIDHEIGHT	348
#define VIDBYTES	(VIDWIDTH/8)			/* bytes	*/

/*  value to use in touching processor registers for some control functions */
#define	ACTIVATE	(ushort)0
/*
---------------------------------------------------------
|	RS-232 Ports A + B Registers  - 8274		|
|	Port A is used for RS-232, port B is used for	|
|	the SCM modem.					|
|	Uses the least significant byte of 16 bit word	|
---------------------------------------------------------
*/
#define A_DATA_ADDR		((ushort *)0xE50000) /* R/W */
#define B_DATA_ADDR		((ushort *)0xE50002) /* R/W */
#define A_CMND_ADDR		((ushort *)0xE50004) /* R/W */
#define B_CMND_ADDR		((ushort *)0xE50006) /* R/W */

/*
---------------------------------------------------------
|	Keyboard serial driver, using the Motorola 6850	|
|	Uses the least significant byte of 16 bit word	|
---------------------------------------------------------
*/
#define C_DATA_ADDR		((ushort *)0xE70002) /* R/W */
#define C_CMND_ADDR		((ushort *)0xE70000) /* R/W */

/*
---------------------------------------------------------
|	Baud generator for channel A, the lower 3	|
|	nibble of the address is the counter value.	|
|	No baud generator for modem channel, constant	|
|	19.2k at clock input, use divide by 16 and 	|
|	divide by 64 logic of 8274 to obtain 1200 and	|
|	300 baud.					|
---------------------------------------------------------
*/
#define A_BAUD_ADDR		((ushort *)0x4B0000) /* WO */

/*
---------------------------------------------------------
|	Phone Status Register 				|
|	Handset offhook (b3 = 0)			|
|	Line 1 ringing  (b2 = 0)			|
|	Line 2 ringing  (b1 = 0)			|
|	Message waiting (b0 is complemented every pulse	|
---------------------------------------------------------
*/
#define PHONE_STATUS		((ushort *) 0x450000)	/* RO */

/*
---------------------------------------------------------
|	Parallel Line Printer Registers 		|
|	Uses the least significant byte of 16 bit word  |
---------------------------------------------------------
*/
#define LP_STATUS_ADDR		((ushort *)0x470000) /* R */
#define LP_DATA_ADDR		((ushort *)0x4F0000) /* WO */
/*	bits in the status register on reading */
#define LP_BUSY			0x80	/* LPB+     1 = lp busy */
#define LP_SELECTED		0x40	/* LPS+     1 = lp selected */
#define LP_OUT_PAPER		0x20	/* NP+      1 = lp out of paper */
#define LP_ERR			0x10	/* LPERR-   0 = lp error */
#define FDINTRQ			0x08	/* FDINTRQ+ 1 = floppy interrupt */
#define HDINTRQ			0x04	/* HDINTRQ+ 1 = hard disk interrupt */
#define PARITY_ERR		0x02	/* PERR-    0 = parity error */
#define DTDET			0x01	/* DTDET-   0 = dial tone detected */

/*
---------------------------------------------------------
|	DISK Bus interface registers			|
|	Uses the least significant byte of 16 bit word  |
---------------------------------------------------------
*/
#define HD_BASE			((ushort *)0xE00000)
#define FD_BASE			((ushort *)0xE10000)

#define DMA_CNT		 	((ushort *)0x460000) /* RW */
#define DMA_LOADDR	 	((unsigned char *)0x4D0000) /* W  */
#define DMA_HIADDR		((unsigned char *)0x4D4000) /* W  */

#define DISK_CNTRL	 	((ushort *)0x4E0000) /* RW */

/* Bits in disk control register */

#define	NOT_FDRST		0x80	/* 0 = reset, 1 = not reset */
#define	FDR0			0x40	/* 1 = floppy selected */
#define	FDMTR			0x20	/* 1 = floppy motor on */
#define	NOT_HDRST		0x10	/* 0 = hdc reset, 1 = hdc not reset */
#define	HDR0			0x08	/* 1 = hard disk 0 selected */
#define	HDSEL			0x07	/* Head select mask */

/* Note: Bit 7 of the general control register is used as hard disk 1
 select in machines with this hardware modification. This is an otherwise
 unused, bit-addressable bit at address 4c7000 */

/* Also note: All bits of the disk control register will be 0 after reset */

/* Bits in dma count register */

#define	DMA_ENABLE		0x8000	/* 0 = dma disable, 1 = dma enabled */
#define	DMA_CNT_MASK		0x3fff	/* Bits 13...0 holds dma count */
#define	DMA_ERROR		0x8000	/* dma error bit mask, 0 = error */

/* Masks for dma addresses */

#define DMA_LO_MASK		0x01fe
#define	DMA_HI_MASK		0x3ffe

#endif iohw.h
