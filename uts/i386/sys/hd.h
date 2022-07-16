/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_HD_H
#define _SYS_HD_H

#ident	"@(#)head.sys:sys/hd.h	1.1.2.1"

/*
 * PC AT Hard disk controller definitions.
 */


#define	HD0	0x1f0	/* base for hard controller I/O port addresses */
#define	FDR	0x3f6	/* I/O port address for fixed disk register */

/*
 * Bit 3 of the fixed disk register must be set to 1 to access heads
 * 8 - 15 of a hard disk.
 */
#define	HD_EXTRAHDS	0x08	/* set into FDR to access high disk heads */
#define	HD_NOEXTRAHDS	0x00	/* set into FDR if no high disk heads */

/*
 * port offsets from base above.
 */
#define	HD_DATA		0x00	/* data register */
#define	HD_ERROR	0x01	/* error register/write precomp */
#define	HD_PRECOMP	0x01	/* error register/write precomp */
#define	HD_NSECT	0x02	/* sector count */
#define	HD_SECT		0x03	/* sector number */
#define	HD_LCYL		0x04	/* cylinder low byte */
#define	HD_HCYL		0x05	/* cylinder high byte */
#define	HD_DRV		0x06	/* drive/head register */
#define	HD_STATUS	0x07	/* status/command register */
#define	HD_CMD		0x07	/* status/command register */

/*
 * Status bits
 */
#define	BUSY		0x80	/* controller busy */
#define	READY		0x40	/* drive ready */
#define	WRFAULT		0x20	/* write fault */
#define	SEEKDONE	0x10	/* seek operation complete */
#define	DATARQ		0x08	/* data request */
#define	ECC		0x04	/* ECC correction applied */
#define	INDEX		0x02	/* disk revolution index */
#define ERROR		0x01	/* error flag */

/*
 * Drive selectors
 */
#define	HD_DHFIXED	0xa0	/* bits always set in drive/head reg. */
#define	HD_DRIVE0	0x00	/* or into HD_DHFIXED to select drive 0 */
#define	HD_DRIVE1	0x10	/* or into HD_DHFIXED to select drive 1 */

/*
 * Hard disk commands. 
 */
#define	HD_RESTORE	0x10	/* restore cmd, bottom 4 bits set step rate */
#define	HD_SEEK		0x70	/* seek cmd, bottom 4 bits set step rate */
#define	HD_RDSEC	0x20	/* read sector cmd, bottom 2 bits set ECC and
					retry modes */
#define	HD_WRSEC	0x30	/* write sector cmd, bottom 2 bits set ECC and
					retry modes */
#define	HD_FORMAT	0x50	/* format track command */
#define	HD_RDVER	0x40	/* read verify cmd, bot. bit sets retry mode */
#define	HD_DIAG		0x90	/* diagnose command */
#define	HD_SETPARAM	0x91	/* set parameters command */

#define	HDTIMOUT	25000	/* how many 10usecs in a 1/4 sec.*/

#define NUMDRV  2	/* maximum number of drives */
#define SECSIZE 512	/* default sector size */
#define SECSHFT 9
#define SECMASK (SECSIZE-1)
#define cylin   av_back

/* Values of hd_state */
#define HD_OPEN		0x01	/* drive is open */
#define HD_OPENING	0x02	/* drive is being opened */
#define HD_DO_RST	0x04	/* hardware restore command should be issued */
#define HD_DO_FMT	0x08	/* track is being formatted */
#define HD_VTOC_OK	0x10	/* VTOC (pdinfo, vtoc, alts table) OK */
#define HD_FMT_RST	0x20	/* restore needs to happen before format */
#define HD_BADBLK	0x40	/* bad block is being remapped */
#define HD_BBH_VFY	0x0080	/* potential bad block is being verified */
#define HD_BBH_MAP	0x0100	/* bad block is being assigned an alt.  */
#define HD_DO_VFY	0x0200	/* Sector(s) being verified.            */
#define HD_BADTRK	0x0400	/* bad block is in alt trk area being remapped */

/*
 * the hard disk minor device number is interpreted as follows:
 *     bits:
 *	 7 5 4 3  0
 * 	+---+-+----+
 * 	|   |u|part|
 * 	+---+-+--+-+
 *     codes:
 *	u     - unit no. (0 or 1)
 *	part  - partition no. (0 - 15)
 */
#define PARTITION(x)	(getminor(x) & 0x0F)
#define UNIT(x)		((getminor(x) >> 4) & 0x01)
#define BASEDEV(x)	(dev_t)((x) & ~0x0F)

/*
 * Logical blocks to physical blocks
 */
#define lbtopb(lb) (((lb) << BSHIFT) >> SECSHFT)
#define pbtolb(pb) (((pb) << SECSHFT) >> BSHIFT)

/*
 * controller interface templates
 */
struct AT_cmd {
	unsigned char nhd_precomp;	/* write precomp */
        unsigned char nhd_nsect; /* decremented during operation - 0 == 256 */
	unsigned char nhd_sect;                   /* starting sector number */
	unsigned int  nhd_cyl;                      /* up to 1024 cylinders */
	/*
	 * must have
	 *	bit	7	1
	 *	bit	6	0
	 *	bit	5	1
	 *	bit	4	drive number
	 *	bits  3-0	head number
	 */
	unsigned char nhd_drv;
	unsigned char nhd_cmd;
};

/*
 * r3, r2, r1, r0 is stepping rate:
 *	0		 .35 micro-seconds
 *	1		 .5	milli-seconds
 *	2		1.0	milli-seconds
 *	2		1.0	milli-seconds
 *	.
 *	.
 *	.
 *	15		7.5	milli-seconds
 */

/*
 * bit	definition		value
 *			0			1
 * L	data mode	data only		data plus 4 byte ECC
 * T	retry mode	retries enabled		retries disabled
 */

/*
 * operational mode
 */
#define	DAM_NOT_FOUND	0x01	/* Data Address Mark not found   */
#define	TR000_ERR	0x02	/* Track 0 not found             */
#define	ABORTED		0x04	/* Command Aborted               */
#define	ID_NOT_FOUND	0x10	/* Sector ID not found           */
#define	ECC_ERR		0x40	/* Uncorrectable data read error */
#define	BAD_BLK		0x80	/* Bad block flag detected       */

#define HDPDLOC		29	/* Sector number on disk where pdinfo is */

/* New ioctls to be used for the purpose of testing BBH */
#define HIOC	('H'<<8)
#define GETALTTBL	(HIOC | 1)	/* get alt_table from kernel memory */
#define FMTBAD		(HIOC | 2)	/* format tracks as bad             */

#endif	/* _SYS_HD_H */
