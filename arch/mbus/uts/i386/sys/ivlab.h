/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/ivlab.h	1.3"

#ifndef _SYS_IVLAB_H
#define _SYS_IVLAB_H

/*
 * This file defines the Boot Block which consists of the 
 * first 1024 bytes of a hard disk.  The boot block contains
 * data structures that are defined in this file as well as 
 * some data structures that are defined in other include files.
 *
 * The following files must be included prior to this file:
 *
 *	sys/types.h
 *	sys/fdisk.h
 */

/*
 *
 * WARNING: The size of each data structures MUST NOT change.
 * They have been carefully defined so that the entire boot block 
 * is EXACTLY 1024 bytes in length.  NOTE - '#pragma pack(2)' MUST
 * surround EVERY data structure so that automatic padding is not
 * done by the compiler.
 */


/*
 *	Intel's Multibus Boot Block  Definitions for UNIX.
 */
 
/*
 * defines for boot block structure
 */

#define	BTBLK_LOC	0L		/* Sector # of boot block */
#define	BTBLK_SIZE	1024		/* # of bytes in boot block */
#define BTBLK_MAGIC	MBB_MAGIC	/* Magic # for boot block signature */

/*
 * defines for ivlab structure
 */

#define VF_AUTO         0x01    	/* 1 ==> byte is valid */
#define VF_DENSITY      0x02    	/* 0 = FM, 1 = MFM */
#define VF_SIDES        0x04    	/* 1 = double-sided */
#define VF_MINI         0x08    	/* 0 = 8", 1 = 5.25" */
#define VF_NOT_FLOPPY   0x10    	/* 0=flop trk 0 is 128 SD, 1=not flop */
#define UNIX_FD         6       	/* UNIX "file-driver" number;   */
#define UNIX_SID        0x0040  	/* UNIX "system-id" (as above) */
#define VLAB_SECT       BTBLK_LOC	/* Sector # containing IVLAB */
#define VLAB_START      384	 	/* Byte # of IVLAB */
#define VLAB_FLOFF      10  		/* flags field offset (for boot) */
#define VLAB_FSDOFF     56      	/* fsdelta field offset (for boot ) */

/*
 * defines for ipart structure
 */

#define IPART_LOC	446		/* Location of ipart struct */
#define IPART_SIZE 	64		/* Size of ipart area	    */

/*
 * defines for bolt structure
 */

#define	BOLT_LOC	512		/* Location  of bolt struct */
#define	BOLT_SIZE	42		/* Size of bolt struct */
#define	MAGIC_WORD_1	0xb00f10ad	/* bolt sanity word  */
#define	VERSION		2		/* bolt version number */
#define	TYPES		3		/* 32-bit code and data */

/*
 * defines for iso structure
 */

#define	ISO_LOC		768		  /* Size of ISO volume label */
#define	ISO_SIZE	128		  /* Size of ISO volume label */

/*
 * Misc. defines 
 */

#define	REAL_1_SIZE	384		    /* Size of 1st part of boot loader*/
#define	REAL_2_SIZE	5120 		    /* Size of 2nd part of boot loader*/
#define	REAL_BOOT_SIZE	(BTBLK_SIZE + 5120) /* Size of real mode boot area*/
#define	RSRVD_1_SIZE	(256 - BOLT_SIZE)   /* Size of 1st reserved area */
#define	RSRVD_2_SIZE	128		    /* Size of 2nd reserved area */

/*
 * Intel volume label
 */

#ifndef lint
#pragma pack(02)			/* WARNING: All these structs must*/
#endif					/* be PACKED !! 		  */

struct ivlab {
	char    v_name[10];     	/* volume name, blank padded */
	char    v_flags;        	/* flags byte -- see below */
	char    v_fdriver;      	/* file-driver number */
	ushort  v_gran;         	/* volume-gran (bytes) */
	ushort  v_size_l;       	/* size (bytes) of volume (low) */
	ushort  v_size_h;       	/* size (bytes) of volume (low) */
	ushort  v_maxfnode;     	/* max fnode # (0 on UNIX) */
	ushort  v_stfnode_l;    	/* start of fnodes (2 in UNIX)(low)*/
	ushort  v_stfnode_h;    	/* start of fnodes (2 in UNIX)(high)*/
	ushort  v_szfnode;      	/* size of fnode (32 in UNIX) */
	ushort  v_rfnode;       	/* root fnode (2 in UNIX) */
	ushort  v_devgran;      	/* sector size (bytes) */
	ushort  v_intl;         	/* interleave; 0 ==> unknown */
	ushort  v_trskew;       	/* track skew; 0 ==> none */
	ushort  v_sysid;        	/* OS id for OS that formatted vol. */
	char    v_sysname[12];  	/* OS name (as above), blank filled */
	char    v_dspecial[8]; 	 	/* device-special info (drtab entry) */
	ushort	v_fsdelta;		/* Start of ROOT file system */
	char	v_freespace[4];		/* Free space for future use */
};

/*
 * Define the BOLT data structures.
 */

struct	tbl_entry	{
	unsigned long	byte_offset;
	unsigned short	length;
};

struct	bolt_def	{
	unsigned long	reserved[4];
	unsigned long	magic_word_1;
	unsigned long	magic_word_2;
	unsigned short	version;
	unsigned short	types;
	unsigned long	data_size;
	unsigned long	num_entries;
	struct	tbl_entry	tbl_entries;
};

typedef	struct	bolt_def	BOLT;

/*
 * Define the ISO Volume Label
 */

struct isovlab {
	char	vlab[ISO_SIZE];	/* Reserve space for the ISO Volume Label */
};

/*
 * Define the entire Boot Block
 */

struct btblk {
	char	bootcode[ REAL_1_SIZE ];/* 1st part of real-mode boot loader */
	struct ivlab	ivlab;		/* Intel Volume Label */
	struct ipart	ipart[4];       /* [FD_NUMPART] DOS Partition Table */
	ushort 		signature;	/* Sanity work boot block structure */
	struct bolt_def bolt;		/* BOLT data structure */
	char	rsrvd1[ RSRVD_1_SIZE ];	/* ISO Reserved Area #1 */
	struct isovlab	isovlab;	/* ISO Volume Label */
	char	rsrvd2[ RSRVD_2_SIZE ];	/* ISO Reserved Area #2 */
};

#ifndef lint
#pragma pack()
#endif

#endif	/* _SYS_IVLAB_H */
