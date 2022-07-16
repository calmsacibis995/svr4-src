/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_FDISK_H
#define _SYS_FDISK_H

#ident	"@(#)head.sys:sys/fdisk.h	1.1.2.1"

#define BOOTSZ		446	/* size of boot code in master boot block */
#define FD_NUMPART	4	/* number of 'partitions' in fdisk table */
#define MBB_MAGIC	0xAA55	/* magic number for mboot.signature */
#define DEFAULT_INTLV	4	/* default interleave for testing tracks */
#define MINPSIZE	4	/* minimum number of cylinders in a partition */
#define TSTPAT		0xE5	/* test pattern for verifying disk */

/*
 * structure to hold the fdisk partition table
 */
struct ipart {
	unsigned char bootid;	/* bootable or not */
	unsigned char beghead;	/* beginning head, sector, cylinder */
	unsigned char begsect;	/* begcyl is a 10-bit number. High 2 bits */
	unsigned char begcyl;	/*     are in begsect. */
	unsigned char systid;	/* OS type */
	unsigned char endhead;	/* ending head, sector, cylinder */
	unsigned char endsect;	/* endcyl is a 10-bit number.  High 2 bits */
	unsigned char endcyl;	/*     are in endsect. */
	long    relsect;	/* first sector relative to start of disk */
	long    numsect;	/* number of sectors in partition */
};
/*
 * Values for bootid.
 */
#define NOTACTIVE	0
#define ACTIVE		128
/*
 * Values for systid.
 */
#define DOSOS12		1	/* DOS partition, 12-bit FAT */
#define PCIXOS		2	/* PC/IX partition */
#define DOSDATA		86	/* DOS data partition */
#define DOSOS16		4	/* DOS partition, 16-bit FAT */
#define EXTDOS		5	/* EXT-DOS partition */
#define OTHEROS		98	/* part. type for appl. (DB?) needs raw partition */
				/* ID was 0 but conflicted with DOS 3.3 fdisk    */
#define UNIXOS		99	/* UNIX V.x partition */
#define UNUSED		100	/* unassigned partition */
#define MAXDOS		65535L	/* max size (sectors) for DOS partition */
/*
 * structure to hold master boot block in physical sector 0 of the disk.
 * Note that partitions stuff can't be directly included in the structure
 * because of lameo '386 compiler alignment design.
 */

struct  mboot {     /* master boot block */
	char    bootinst[BOOTSZ];
	char    parts[FD_NUMPART * sizeof(struct ipart)];
	ushort   signature;
};

#endif	/* _SYS_FDISK_H */
