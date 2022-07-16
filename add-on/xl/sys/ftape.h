/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xl:sys/ftape.h	1.3"

/*	Copyright (c) 1989 Intel Corp.		*/
/*	  All Rights Reserved  	*/
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license
 *	agreement or nondisclosure agreement with Intel Corpo-
 *	ration and may not be copied or disclosed except in
 *	accordance with the terms of that agreement.
 */

/************************************************************************/
/*	Copyright (c) 1988, 1989 ARCHIVE Corporation			*/
/*	This program is an unpublished work fully protected by the	*/
/*	United States Copyright laws and is considered a trade secret	*/
/*	belonging to Archive Corporation.				*/
/************************************************************************/
/*	file: ftape.h							*/
/************************************************************************/
/*
 * the floppy tape device naming convension
 *
 *	f 1 q80 n r 
 *	| |  |  | |_______	retension on open
 *	| |  |  |_________	no rewind on open/close
 *	| |  |____________	recording format  (q80 = QIC-80 80 MB)
 *	| |				 	  (q40 = QIC-40 40 MB)
 *	| |_______________	unit no. (0 or 1)
 *	|_________________	device type (floppy tape)
 *
 *
 * examples: 			floppy tape unit #1 QIC-80 format, 80 MB,
 *
 *		         minor#  control ret_on_open rew_on_close rew_on_open
 *	/dev/rmt/f1q80c    149	   y         -           y            y
 *	/dev/rmt/f1q80      21	   -         -           y            y
 *	/dev/rmt/f1q80n	    17	   -         -           -            y*
 *	/dev/rmt/f1q80nr    25	   -         y           -            y*
 *	/dev/rmt/f1q80r	    29	   -         y           y            y 
 *
 *	* Multi-volume write is not implemented in this version. Always 
 *	  rew_on_open. 
 */

/*
 * the floppy tape minor device number is interpreted as follows:
 *     bits:
 *	 7 6 5 4 3 2 1 0
 * 	+-+-----+-+-+-+-+
 * 	|c| fmt |r|w| |u|
 * 	+-+-----+-+-+-+-+
 *     codes:
 *	c     - control   (1 = yes, 0 = no)
 *	fmt   - format    (0 = QIC-40, 1 = QIC-80)
 *	r     - retension (1 = yes, 0 = no)
 *	w     - rewind    (1 = yes, 0 = no)
 *	u     - unit no.  (0 or 1)
 */

#define	M_CTL		0x80
#define	M_FMT		0x70
#define	M_RET		0x08
#define	M_REW		0x04
#define	M_UNT		0x01
#define	FT_UNIT(x)	(minor(x) & M_UNT)
#define	FT_FORMAT(x)	((minor(x) & M_FMT) >> 4)

/************************************************************************/
/*	QIC-40/80  defines and structs					*/
/************************************************************************/

union xl_status {
	struct {
		unsigned short ready   : 1;	/* drive ready		*/
		unsigned short error   : 1;	/* error detected	*/
		unsigned short cpres   : 1;	/* cart. present	*/
		unsigned short wprot   : 1;	/* write protected	*/
		unsigned short newcart : 1;	/* new cart. inserted	*/
		unsigned short cref    : 1;	/* cart. referenced	*/
		unsigned short bot     : 1;	/* phys beg. of tape	*/
		unsigned short eot     : 1;	/* phys end of tape	*/

		unsigned short errnum : 8;	/* error number		*/
		unsigned short errcmd : 8;	/* error assoc with cmd	*/

		unsigned short sfterr;		/* soft errors		*/
		unsigned short hrderr;		/* hard errors		*/
		unsigned short undrun;		/* underruns		*/
	} status;
	unsigned char stat[9];
};

/* ioctl's for QIC-40/QIC-80 floppy tapes */

#define XLIOC		('x'<<8)        /* ioctl for QIC-40/80 cmds	*/
#define XL_STATUS	(XLIOC | 0)	/* read tape status		*/
#define XL_RESET	(XLIOC | 1)	/* reset tape drive		*/
#define XL_RETEN	(XLIOC | 2)	/* retension tape		*/
#define XL_REWIND	(XLIOC | 3)	/* rewind tape			*/
#define XL_ERASE	(XLIOC | 4)	/* erase tape			*/
#define XL_AMOUNT	(XLIOC | 5)	/* report amount of data xfered	*/
#define XL_RFM		(XLIOC | 6)	/* read 'file mark'		*/
#define XL_FORMAT	(XLIOC | 7)	/* format a pair of tracks	*/
#define XL_DEBUG	(XLIOC | 11)	/* set debug variable on/off	*/

