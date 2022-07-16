#ident	"@(#)wdhdw.h	1.2	92/02/17	JPB"

/*
 * Module: WD8003
 * Project: System V ViaNet
 *
 *		Copyright (c) 1987 by Western Digital Corporation.
 *		All rights reserved.  Contains confidential information and
 *		trade secrets proprietary to
 *			Western Digital Corporation
 *			2445 McCabe Way
 *			Irvine, California 92714
 */

#ident "@(#)wdhdw.h	1.2 - 92/02/17"
#ident "$Header: wdhdw.h 2.2 90/06/08 $"

/* Hardware definitions for the WD8003 and 8390 LAN controller		*/

/* WD8003 Definitions and Statistics					*/

#define TX_BUF_LEN	(6*256)
#define	SEG_LEN		(1024 * 8)

/* WD8003 Commands							*/

#define SFTRST	0x80			/* software reset command	*/
#define MEMENA	0x40			/* memory enable switch		*/
 
/* WD8003 register locations						*/

/* WD8003 register locations						*/

#define	IRR		4	/* Interrupt Request Register (on	*/
				/*	boards with WD83C583 chip)	*/
#define	CCR		5	/* Configuration Control Register	*/
				/*	(Micro Channel boards with 	*/
				/*	WD83C593 chip)			*/
#define LAAR		5	/* LA Address Register (16 bit AT bus	*/
				/*	boards)				*/
#define	UBA_STA		8
#define	WD_BYTE		0xE

/* IRR register definitions */
#define IRR_IEN		0x80	/* Interrupt Enable */
#define IRR_IRQMASK	0x60	/* mask for IRQ bits */
#define IRR_IRQ2	0x00	/* IRQ2 selected */
#define IRR_IRQ3	0x20	/* IRQ3 selected */
#define IRR_IRQ4	0x40	/* IRQ4 selected */
#define IRR_IRQ7	0x60	/* IRQ7 selected */

/* CCR register definitions */
#define EIL		0x4		/* enable interrupts		*/

/* LAAR register definitions */
#define MEM16ENB	0x80
#define LAN16ENB	0x40

/* 8390 Registers: Page 1						*/
/* NOTE: All addresses are offsets from the command register (cmd_reg)	*/

#define	PSTART	0x1
#define	PSTOP	0x2
#define	BNRY	0x3
#define	TPSR	0x4
#define	TBCR0	0x5
#define	TBCR1	0x6
#define	ISR	0x7
#define RBCR0	0xA
#define RBCR1	0xB
#define	RCR	0xC
#define	TCR	0xD
#define	DCR	0xE
#define	IMR	0xF
#define	RSR	0xC
#define	CNTR0	0xD
#define CNTR1	0xE
#define	CNTR2	0xF

/* 8390 Registers: Page 2						*/
/* NOTE: All addresses are offsets from the command register (cmd_reg)	*/

#define	PAR0	0x1
#define	CURR	0x7
#define MAR0	0x8

/* 8390 Commands							*/

#define	PAGE_0	0x00
#define	PAGE_1	0x40
#define	PAGE_2	0x80
#define	PAGE_3	0xC0

#define	PG_MSK	0x3F			/* Used to zero the page select
					   bits in the command register */

#define	STA	0x2			/* Start 8390			*/
#define STP	0x1			/* Stop 8390			*/
#define	TXP	0x4			/* Transmit Packet		*/
#define	ABR	0x20			/* Value for Remote DMA CMD	*/

/* 8390 ISR conditions							*/

#define	PRX	0x1
#define	PTX	0x2
#define	RXE	0x4
#define	TXE	0x8
#define	OVW	0x10
#define	CNT	0x20

/* 8390 IMR bit definitions						*/

#define	PRXE	0x1
#define	PTXE	0x2
#define	RXEE	0x4
#define	TXEE	0x8
#define	OVWE	0x10
#define	CNTE	0x20
#define	RDCE	0x40

/* 8390 DCR bit definitions						*/

#define	WTS	0x1
#define	BOS	0x2
#define	LAS	0x4
#define	BMS	0x8
#define	FT0	0x20
#define	FT1	0x40


/* 8390 TCR bit definitions						*/

#define	CRC	0x1
#define	LB0_1	0x2
#define	ATD	0x8
#define	OFST	0x10


/* RCR bit definitions							*/

#define	SEP	0x1
#define	AR	0x2
#define	AB	0x4
#define	AM	0x8
#define	PRO	0x10
#define	MON	0x20

/* TSR bit definitions							*/
#define TSR_COL 0x4
#define TSR_ABT 0x8

/* 8390 Register initialization values					*/

#define	INIT_IMR	PRXE + PTXE + RXEE + TXEE
#define INIT_DCR	BMS + FT1
#define	INIT_TCR	0
#define	INIT_RCR	AB + AM
#define	RCRMON		MON

/* Misc. Commands & Values						*/

#define	CLR_INT		0xFF		/* Used to clear the ISR */
#define	NO_INT		0		/* no interrupts conditions */
#define ADDR_LEN	6
#define NETPRI		PZERO+3

/* PS2 specific defines */
/* Defines for PS/2 Microchannel POS ports */

#define SYS_ENAB	0x94		/* System board enable / setup */
#define ADAP_ENAB	0x96		/* Adaptor board enable / setup */
#define POS_0		0x100		/* POS reg 0 - adaptor ID lsb */
#define POS_1		0x101		/* POS reg 1 - adaptor ID msb */
#define POS_2		0x102		/* Option Select Data byte 1 */
#define POS_3		0x103		/* Option Select Data byte 2 */
#define POS_4		0x104		/* Option Select Data byte 3 */
#define POS_5		0x105		/* Option Select Data byte 4 */
#define POS_6		0x106		/* Subaddress extension lsb */
#define POS_7		0x107		/* Subaddress extension msb */

/* Defines for Adaptor Board ID's for Microchannel */

#define WD_ID	0xABCD			/* generic id for WD test */
#define ETI_ID	0x6FC0			/* 8003et/a ID */
#define STA_ID	0x6FC1			/* 8003st/a ID */
#define WA_ID	0x6FC2			/* 8003w/a ID */

#define NUM_ID	4

#define PS2_RAMSZ	16384

/* Define board id values */

#define	STARLAN_MEDIA		0x00000001
#define	ETHERNET_MEDIA		0x00000002
#define	TWISTED_PAIR_MEDIA	0x00000003
#define	MICROCHANNEL		0x00000008
#define	INTERFACE_CHIP		0x00000010
#define	INTELLIGENT		0x00000020
#define	BOARD_16BIT		0x00000040
#define	RAM_SIZE_UNKNOWN	0x00000000	/* 000 => Unknown RAM Size */
#define	RAM_SIZE_RESERVED_1	0x00010000	/* 001 => Reserved */
#define	RAM_SIZE_8K		0x00020000	/* 010 => 8k RAM */
#define	RAM_SIZE_16K		0x00030000	/* 011 => 16k RAM */
#define	RAM_SIZE_32K		0x00040000	/* 100 => 32k RAM */
#define	RAM_SIZE_64K		0x00050000	/* 101 => 64k RAM */ 
#define	RAM_SIZE_RESERVED_6	0x00060000	/* 110 => Reserved */ 
#define	RAM_SIZE_RESERVED_7	0x00070000	/* 111 => Reserved */ 
#define	SLOT_16BIT		0x00080000
#define	NIC_690_BIT		0x00100000
#define	ALTERNATE_IRQ_BIT	0x00200000

#define	MEDIA_MASK		0x00000007
#define	RAM_SIZE_MASK		0x00070000
#define	STATIC_ID_MASK		0x0000FFFF
