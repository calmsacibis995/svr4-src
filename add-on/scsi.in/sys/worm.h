/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989 TOSHIBA CORPORATION		*/
/*		All Rights Reserved			*/

/*	Copyright (c) 1989 SORD COMPUTER CORPORATION	*/
/*		All Rights Reserved			*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF		*/
/*	TOSHIBA CORPORATION and SORD COMPUTER CORPORATION	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi.in:sys/worm.h	1.3"

/****************************************************************
 *								*
 *		worm.h						*
 *								*
 *	SCSI WORM Include File					*
 *								*
 ****************************************************************/

/* driver ioctl commands */

#define	WIOC		('W' << 8)	/* For WORM ioctl() commands */
/* Group 0 commands */
#define	C_TRAYCLOSE	(CIOC | 0x0E)	/* tray close		      (0xC5) */
#define	W_TESTUNIT	(WIOC | 0x01)	/* Test unit ready	      (0x00) */
#define	W_REZERO	(WIOC | 0x02)	/* Rezero unit	 	      (0x01) */
#define	W_SEEK		(WIOC | 0x04)	/* Seek 		      (0x0B) */
#define	W_INQUIR	(WIOC | 0x05)	/* Inquiry 		      (0x12) */
#define	W_STRTUNIT	(WIOC | 0x06)	/* Start unit 		      (0x1B) */
#define	W_STOPUNIT	(WIOC | 0x07)	/* Stop unit 		      (0x1B) */
#define	W_PREVMV	(WIOC | 0x08)	/* Prevent media removal      (0x1E) */
#define	W_ALLOMV	(WIOC | 0x09)	/* Allow media removal 	      (0x1E) */

/* Group 1 commands */
#define	W_READCAPA	(WIOC | 0x0A)	/* Read capacity 	      (0x25) */
#define	W_VERIFY	(WIOC | 0x0B)	/* Verify	 	      (0x2F) */

/* Group 6 commands */
#define	W_STNCHECK	(WIOC | 0x0C)	/* Standby check 	      (0xCE) */
#define	W_LOADCART	(WIOC | 0x0D)	/* Load cartridge 	      (0xCF) */
#define	W_UNLOADCA	(WIOC | 0x0E)	/* Unload cartridge 	      (0xCF) */
#define	W_READCB	(WIOC | 0x0F)	/* Read control block 	      (0xD2) */

/* Group 7 commands */
#define	W_CHECK		(WIOC | 0x11)	/* Check 		      (0xE4) */
#define	W_CCHECK	(WIOC | 0x12)	/* Contrary check 	      (0xE4) */

#define	W_ERRMSGON	(WIOC | 0x20)	/* System error message ON	     */
#define	W_ERRMSGOFF	(WIOC | 0x21)	/* System error message OFF	     */

/* arg of the W_INQUIR ioctl */
struct worm_inq {
	unsigned short length;	/* The length of the required sense data*/
	char		*addr;	/* First address of the space where the */
				/* inquiry data is stored */
};

/* arg of the W_READCAPA ioctl */
struct worm_capacity {
	unsigned long    addr;	/* Logical block address */
	unsigned long    len;	/* Block length */
};

/* arg of the W_VEIRFY ioctl */
struct worm_verify {
	long	start;		/* Block number at which verification starts */
	long	num;		/* Number of blocks on which verification is */
			   	/* to be performed */
};

/* arg of the W_CHECK, and W_CCHECK ioctl */
struct worm_check {
	long	start;		/* Block number where checking starts */
	long	num;		/* Number of blocks to be checked */
	long	block;		/* The number of the block checked lastly */
	long	result;		/* The result of the check */
};
