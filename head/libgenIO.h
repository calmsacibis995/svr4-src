/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.usr:libgenIO.h	1.2.3.1"

/*
 * Code review updated version.
 */

/* device types */

#define G_NO_DEV	0	/* device does not require special treatment */
#define	G_FILE		1	/* file, not a device */
#define G_3B2_HD	2	/* 3B2 hard disk */
#define G_3B2_FD	3	/* 3B2 floppy diskette */
#define G_3B2_CTC	4	/* 3B2 cartridge tape */
#define G_SCSI_HD	5	/* scsi hard disk */
#define G_SCSI_FD	6	/* scsi floppy diskette */
#define G_SCSI_9T	7	/* scsi 9-track tape */
#define G_SCSI_Q24	8	/* scsi QIC-24 tape */
#define G_SCSI_Q120	9	/* scsi QIC-120 tape */
#define G_386_HD	10	/* 386 hard disk */
#define G_386_FD	11	/* 386 floppy disk */
#define G_386_Q24	12	/* 386 QIC-24 tape */
#define	G_TAPE		13	/* 9 Track tape */
#define	G_DEV_MAX	14	/* last valid device type */

/* special defines for the 3B2 cartridge tape */

#define O_CTSPECIAL	0200
#define STREAMON	's'
