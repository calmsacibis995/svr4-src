/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)boot:boot/sys/dib.h	1.1.2.1"

/* Definitions for DIB types */

#define DISK_DIB		0x5961L		/* SCSI disk dib */
#define TAPE_DIB		0x5962L		/* SCSI tape dib */
#define BS_DIB			0x5963L		/* MBII Bootserver DIB */
#define PCI_DISK_DIB	0x5964L		/* PCI  disk DIB */
#define PCI_TAPE_DIB	0x5965L		/* PCI  tape DIB */

#ifndef lint
#pragma pack(01)
#endif
/*
 *	Generic stuff for all DIBs
*/
struct dib_hdr {
	ulong	size;			/* Length of DIB in bytes */
	ulong	device_type;	/* DIB device type as above */
};

/*
 *	Disk specific DIB fields
*/
struct dib_disk {		
	void (*init)();			/* Initialize */
	ushort init_sel;		/* Selector for far call */
	void (*read)();			/* Read a block */
	ushort read_sel;		/* Selector for far call */
	ushort dev_gran;		/* Size of a block */
};

/*
 *	Tape specific DIB fields
*/
struct dib_tape {		
	void (*init)();			/* Initialize */
	ushort init_sel;		/* Selector for far call */
	void (*read)();			/* Read a block */
	ushort read_sel;		/* Selector for far call */
	void (*rewind)();		/* Rewind the tape */
	ushort rewind_sel;		/* Selector for far call */
	void (*read_fm)();		/* Read to filemark */
	ushort read_fm_sel;		/* Selector for far call */
	ushort dev_gran;		/* Size of a block */
};

struct	dib	{
	struct dib_hdr hdr;
	union {
		struct dib_disk disk;
		struct dib_tape tape;
	} un;
};
#ifndef lint
#pragma pack()
#endif

/*
 *	Here's yet another version of the POINTER type.
*/
typedef struct {
	caddr_t	offset;
	ushort sel;
} POINTER;
