/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:io/scsi/wd7000.h	1.3"

#define HA_ID			0x07
#define	SHA_BASE		0x300

/* bit masks for the asc status byte */

#define	INIT_FLAG		0x10
#define	CMD_REJECT		0x20
#define	CMD_READY		0x40
#define	INT_FLAG		0x80

/* Interrupt Status Reg. codes */

#define	DGN_NTR		0x00
#define	DGN_ATP		0x01
#define	RAM_FAIL	0x02
#define	FIFO_FAIL	0x03
#define	SBIC_FAIL	0x04
#define	DFF_FAIL	0x05
#define	IRQ_FAIL	0x06
#define	CKSUM_FAIL	0x07
#define	CMD_COMPLETE	0xC0

/* bit mask for the host control register */

#define	INT_ENABLE		0x08
#define	DMA_REQ_ENABLE		0x04
#define	SCSI_PORT_RESET		0x02
#define	ASC_RESET		0x01

#define	LOW_NIB_MASK		0xf0
#define	HI_NIB_MASK		0x0f

/* HA board op codes */

#define	NOOP		0x00		/* no operation 		*/
#define	INIT		0x01		/* initialization 		*/
#define	D_UINTS		0x02		/* disable unsolicited ints 	*/
#define	E_UINTS		0x03		/* enable unsolicited ints 	*/
#define	IFREE_RQ	0x04		/* interrupt on free OGMB 	*/
#define	S_RESET		0x05		/* SCSI soft reset 		*/
#define	RST_ACK		0x06		/* SCSI hard reset ack 		*/
#define	RD_MB		0x80		/* Read mail box # command 	*/
#define	RD_ALLMB	0xC0		/* Read all mail boxes command	*/

#define	SET_EXECUT_PARAMS 0x8A		/* Set Execution Parameters	*/
#define	RD_EXECUT_PARAMS  0x8B		/* Read Execution Parameters	*/
#define	READ_HA_VER	  0x8C		/* Read HA version string	*/


/* definitions for synchronous rate setup */
/* These values are only accurate for the SBIC-a running at 16 MHz */

#define FOUR_MEG	0x20		/* negotiate 4 Meg on scsi bus */
#define OFFSET_12	0x0A		/* negotiate offset 12 on scsi bus */

#define ICMD_SIZE	10		/* INIT cmd size */

/* command definitions for the SCSI Request block */

#define		SCSI_CMD	0x00	/* send cmd to target controller */
#define		SCSI_DMA_CMD	0x01	/* send cmd to target controller */
					/* but use scatter/gather 	 */


/* Copletion queue status codes	*/

#define NO_ERROR	0x1	/* command completed without errors	*/
#define CK_SSB		0x2	/* cmd completed check SCSI status byte	*/
#define HA_ERROR	0x4	/* cmd failed due to HA detected error	*/
#define B_RESET		0x5	/* SCSI Bus reset command terminated	*/
#define SPC_ERROR	0x6	/* cmd failed SCSI protocol chip error	*/
#define TC_RESET	0x7	/* SCSI target reset cmd completed	*/
#define AB_RESET	0x84	/* SCSI Bus reset no cmds outstanding	*/


#define NO_ADDR		0xFFF	/* completion Q address is not valid	*/
#define NO_SELECT	0x4D	/* HA vendor error selection time out	*/


#define	DOS 0
#define ABORT 0
#define TCRST 0
#define SCSI_CTL 0


#define RD_EXP		77	/* read execution parameters	*/
#define WR_EXP		78	/* write executions parameters	*/

#define DEBUGFLG	73	/* Turn on or off debug values	*/
#define TIMEOUTFLG	74	
#define HA_RESET	75	
#define SB_RESET	76	


/* dma stuff */

#define	CASCADE			0xc0

#define	RQ_BUSY			1

#define	RD_HA_STATUS(c)	(inb(HA_STATUS(c)) & LOW_NIB_MASK)

#define msbyte(x)	((unsigned char) (x >> 16) & 0x0000FF)
#define mdbyte(x)	((unsigned char) (x >> 8) & 0x0000FF)
#define lsbyte(x)	((unsigned char) (x & 0x0000FF))
