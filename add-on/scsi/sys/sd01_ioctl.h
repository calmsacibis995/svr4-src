/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:sys/sd01_ioctl.h	1.3"

#define SD_CHAR		('D' << 8)
#define	SD_ELEV		(SD_CHAR | 0x1)		/* Elevator Algorithm */
#define	SD_PDLOC	(SD_CHAR | 0x2)		/* Absolute PD sector */
