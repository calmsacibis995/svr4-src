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

#ident	"@(#)scsi.in:sys/cdrom.h	1.3"

/****************************************************************
 *								*
 *		cdrom.h						*
 *								*
 *	SCSI CD-ROM Include File				*
 *								*
 ****************************************************************/

#define CIOC		('C' << 8)
/* Group 0 commands */
#define	C_TESTUNIT	(CIOC | 0x01)	/* test unit ready	      (0x00) */
#define	C_REZERO	(CIOC | 0x02)	/* rezero unit		      (0x01) */
#define	C_SEEK		(CIOC | 0x03)	/* seek			      (0x0B) */
#define	C_INQUIR	(CIOC | 0x04)	/* inquiry		      (0x12) */
#define	C_STARTUNIT	(CIOC | 0x05)	/* start unit		      (0x1B) */
#define	C_STOPUNIT	(CIOC | 0x06)	/* stop unit		      (0x1B) */
#define	C_PREVMV	(CIOC | 0x07)	/* prevent medium removal     (0x1E) */
#define	C_ALLOMV	(CIOC | 0x08)	/* allow medium removal	      (0x1E) */

/* Group 1 commands */
#define	C_READCAPA	(CIOC | 0x09)	/* read capacity	      (0x25) */

/* Group 6 commands */
#define	C_AUDIOSEARCH	(CIOC | 0x0A)	/* audio track search	      (0xC0) */
#define	C_PLAYAUDIO	(CIOC | 0x0B)	/* play audio		      (0xC1) */
#define	C_STILL		(CIOC | 0x0C)	/* still		      (0xC2) */
#define	C_TRAYOPEN	(CIOC | 0x0D)	/* tray open		      (0xC4) */
#define	C_TRAYCLOSE	(CIOC | 0x0E)	/* tray close		      (0xC5) */

#define	C_ERRMSGON	(CIOC | 0x20)	/* System error message ON	     */
#define	C_ERRMSGOFF	(CIOC | 0x21)	/* System error message OFF	     */

/* arg of the C_INQUIRY ioctl */
struct cdrom_inq {
	unsigned short length;	/* The length of the required sense data*/
	char		*addr;	/* First address of the space where the	*/
				/* inquiry data is stored */
};

/* arg of the C_READCAPA ioctl */
struct cdrom_capacity {
	unsigned long    addr;	/* Logical block address */
	unsigned long    len;	/* Block length */
};

/* arg of the C_AUDIOSEARCH and C_PLAYAUDIO ioctl */
struct cdrom_audio {
	union		_addr {		/* address	*/
		unsigned	type_00;
		struct {
			unsigned	res	: 8;
			unsigned	frame	: 8;
			unsigned	sec	: 8;
			unsigned	min	: 8;
		} type_01;
		struct {
			unsigned	res	: 24;
			unsigned	track	: 8;
		} type_10;
	} _addr;
	char		play;		/* play mode	*/
	char		type;		/* type		*/
};

#define	addr_logical	_addr.type_00
#define	addr_min	_addr.type_01.min
#define	addr_sec	_addr.type_01.sec
#define	addr_frame	_addr.type_01.frame
#define	addr_track	_addr.type_10.track
