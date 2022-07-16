/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/i258/space.c	1.3.3.3"

/**************************************************************************
 *
 *	iSBC 258 Specific Configuration file.
 *
 *      THIS FILE ASSUMES PROPER VALUES ARE OBTAINED FROM VTOC INFO!
 *
 **************************************************************************/

#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/elog.h"
#include "sys/fdisk.h"
#include "sys/ivlab.h"
#include "sys/ics.h"
#include "sys/mps.h"
#include "sys/vtoc.h"
#include "sys/alttbl.h"
#include "sys/bbh.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/i258.h"
#include "sys/conf.h"

int	i258devflag = D_NEW | D_DMA;			/* SVR4 Device Flags */
int	i25tdevflag = D_NEW | D_DMA | D_TAPE;	/* SVR4 Device Flags */

#define i258TYPE DEVWINIG
#define FLPYTYPE DEV5FLPY
#define TAPETYPE STREAMER

/*
 * ------------------------------------------------------
 * NOTE: Changing default SCSI Target ID and unit numbers.
 * ------------------------------------------------------
 * This file is configured for following TIDs for each instance of
 * PCI server:
 * 		SCSI Target ID 	Device 		struct i258cdrt
 *		--------------	------		---------------
 *			0			Floppy 0	i258f00
 *			1			Floppy 1	i258f00
 * 			2			Wini 0		i258d00
 * 			3 			Wini 1		i258d00
 * 			4			Wini 2		i258d00
 *			5 			Wini 3		i258d00
 * 			6 			Tape 0		i258t00
 *
 * This is reflected in the structure i258cfg. 
 * So if you have a different Target ID assignment, you MUST modify the
 * structure i258cfg.
 * Also, the minor table i258minor MUST be edited for the fields 'unit', 
 * 'drtab' and 'partition'.
 */

/*
 * STREAMER Tape Partition Table (dummy).
 */
struct	i258part i258Mmt0[] = {
	0,V_VOMASK,0,              0                       /* Archive streamer */
};

/*
 * 258 Board 0 unit 8 (Tape) Device-Table Definitions (drtabs)
 *
 * Note: Tape is treated differently from disk.
 *	Formatting a tape unit will cause the Tape to be erased.
 *	Sector Size refers to the block size of the tape device.
 *	#Sec (Number of sectors per Track) is the Number of Blocks
 *	per Tape.  Streamer Tapes (Archive) Must be blocked in a
 *	multiple of the on board buffer size (512 bytes).
 * NOTE: The size field for the tape is total number of 512 byte blocks
 *
 *	/dev/rmt/c0s0n	- no rewind on close, no retension on open
 *	/dev/rmt/c0s0	-    rewind on close, no retension on open
 *	/dev/rmt/c0s0nr	- no rewind on close,    retension on open
 *	/dev/rmt/c0s0r	-    rewind on close,    retension on open
 */
struct	i258cdrt i258t00[] = {
/* present, no-op, no-op, no-op,!rew?,       Part        */
/*Cyls,Hds,#Sec,SecSiz, flags,       Part, Size,  Unit: Drive-Type */
/* 150 MB streamer tape */
  01, 0, 0, 512, 0,          				0, 307200,  i258Mmt0,/*[0] c0s0 */
  01, 0, 0, 512, DR_NO_REWIND, 				0, 307200,  i258Mmt0,/*[1] c0s0n */
  01, 0, 0, 512, DR_RETENSION|DR_NO_REWIND,	0, 307200,  i258Mmt0,/*[2] c0s0nr */
  01, 0, 0, 512, DR_RETENSION, 				0, 307200,  i258Mmt0,/*[3] c0s0r */
/* 2.3 GB Exabyte 8mm tape */
  01, 0, 0, 1024, 0,          				0, 4710400, i258Mmt0,/*[4] c0s0 */
  01, 0, 0, 1024, DR_NO_REWIND, 			0, 4710400, i258Mmt0,/*[5] c0s0n */
  01, 0, 0, 1024, DR_RETENSION|DR_NO_REWIND,0, 4710400, i258Mmt0,/*[6] c0s0nr */
  01, 0, 0, 1024, DR_RETENSION,				0, 4710400, i258Mmt0,/*[7] c0s0r */
};

/*
 * Floppy Partitions.
 *	This table lists the possible partitions for each diskette format.
 *	There are 2 or 3 standard partitions for each format and room for
 *	a few additional definitions.  Each disk format can have up to 5 
 *	partition definitions.  WARNING: It is possible to use i258minor
 *	to specify combinations of i258cdrt and i258part entries that do
 *	not make sense so use caution when filling out the minor table.
 */
struct	i258part i258Pf0[] = {
/* 5.25 inch - 80 Kbytes:  Hds-1  Cyls-40  Secs-16  Bytes-128 */
/* ISO Standard for Track 0 */
0,V_VOMASK,    0,      640,    /* [ 0]  Whole disk */
0,V_VOMASK,   16,      628,    /* [ 1]  Skip track/cylinder 0 */
0,       0,    0,        0,    /* [ 2]  Future use */
0,       0,    0,        0,    /* [ 3]  Future use */
0,       0,    0,        0,    /* [ 4]  Future use */

/* 5.25 inch - 160 Kbytes:  Hds-1  Cyls-40  Secs-8  Bytes-512 */
0,V_VOMASK,    0,      320,    /* [ 5]  Whole disk */
0,V_VOMASK,    8,      312,    /* [ 6]  Skip track/cylinder 0 */
0,       0,    0,        0,    /* [ 7]  Future use */
0,       0,    0,        0,    /* [ 8]  Future use */
0,       0,    0,        0,    /* [ 9]  Future use */

/* 5.25 inch - 320 Kbytes:  Hds-2  Cyls-40  Secs-16  Bytes-256 */
0,V_VOMASK,    0,     1280,    /* [10]  Whole disk */
0,V_VOMASK,   16,     1264,    /* [11]  Skip track 0 */
0,V_VOMASK,   32,     1248,    /* [12]  Skip cylinder 0 */
0,       0,    0,        0,    /* [13]  Future use */
0,       0,    0,        0,    /* [14]  Future use */

/* 5.25 inch - 320 Kbytes:  Hds-2  Cyls-40  Secs-8  Bytes-512 */
0,V_VOMASK,    0,      640,    /* [15]  Whole disk */
0,V_VOMASK,    8,      632,    /* [16]  Skip track 0 */
0,V_VOMASK,   16,      624,    /* [17]  Skip cylinder 0 */
0,       0,    0,        0,    /* [18]  Future use */
0,       0,    0,        0,    /* [19]  Future use */

/* 5.25 inch - 320 Kbytes:  Hds-2  Cyls-40  Secs-4  Bytes-1024 */
0,V_VOMASK,    0,      320,    /* [20]  Whole disk */
0,V_VOMASK,    4,      316,    /* [21]  Skip track 0 */
0,V_VOMASK,    8,      312,    /* [22]  Skip cylinder 0 */
0,       0,    0,        0,    /* [23]  Future use */
0,       0,    0,        0,    /* [24]  Future use */

/* 5.25 inch - 360 Kbytes:  Hds-2  Cyls-40  Secs-9  Bytes-512 */
0,V_VOMASK,    0,      720,    /* [25]  Whole disk */
0,V_VOMASK,    9,      711,    /* [26]  Skip track 0 */
0,V_VOMASK,   18,      702,    /* [27]  Skip cylinder 0 */
0,       0,    0,        0,    /* [28]  Future use */
0,       0,    0,        0,    /* [29]  Future use */

/* 5.25 inch - 720 Kbytes:  Hds-2  Cyls-80  Secs-9  Bytes-512 */
0,V_VOMASK,    0,     1440,    /* [30]  Whole disk */
0,V_VOMASK,    9,     1431,    /* [31]  Skip track 0 */
0,V_VOMASK,   18,     1422,    /* [32]  Skip cylinder 0 */
0,       0,    0,        0,    /* [33]  Future use */
0,       0,    0,        0,    /* [34]  Future use */

/* 5.25 inch - 1.2 Mbytes:  Hds-2  Cyls-80  Secs-15  Bytes-512 */
0,V_VOMASK,    0,     2400,    /* [35]  Whole disk */
0,V_VOMASK,   15,     2385,    /* [36]  Skip track 0 */
0,V_VOMASK,   30,     2370,    /* [37]  Skip cylinder 0 */
0,V_VOMASK,   60,     2340,    /* [38]  Skip cyls 0 & 1 - For PCI boot floppy */
0,       0,    0,        0,    /* [39]  Future use */

/* 3.5 inch - 720 Kbytes:  Hds-2  Cyls-80  Secs-9  Bytes-512 */
0,V_VOMASK,    0,     1440,    /* [40]  Whole disk */
0,V_VOMASK,   18,     1422,    /* [41]  Skip track/cylinder 0 */
0,       0,    0,        0,    /* [42]  Future use */
0,       0,    0,        0,    /* [43]  Future use */
0,       0,    0,        0,    /* [44]  Future use */

/* 3.5 inch - 1.44 Kbytes:  Hds-2  Cyls-80  Secs-18  Bytes-512 */
0,V_VOMASK,    0,     2880,    /* [45]  Whole disk */
0,V_VOMASK,   36,     2844,    /* [46]  Skip track/cylinder 0 */
0,       0,    0,        0,    /* [47]  Future use */
0,       0,    0,        0,    /* [48]  Future use */
0,       0,    0,        0,    /* [49]  Future use */
};

/*
 * Floppy Device-Table Definitions (drtabs)
 *	This table defines the possible diskette formats that are suppored
 *	by this board.  Each diskette format has a corresponding set of
 *	partitions definitions as specified in i258part.  WARNING: It is 
 *	possible to use i258minor to specify combinations of i258cdrt 
 *	and i258part entries that do not make sense.
 */
struct	i258cdrt i258f00[] = {

/* 5.25 inch - 80 Kbyte capacity:  FS5_3979BR_48TPI  */
  40,   1, 16,  128, I258FLP(2, 1, 0, 5), 0, 0, i258Pf0, /* [0] */

/* 5.25 inch - 160 Kbyte capacity  FS5_3979BR_48TPI */
  40,   1,  8,  512, I258FLP(2, 1, 0, 5), 0, 0, i258Pf0, /* [1] */

/* 5.25 inch - 320 Kbyte capacity  FD5_7958BR_48TPI */
  40,   2, 16,  256, I258FLP(2, 1, 0, 6), 0, 0, i258Pf0, /* [2] */
  40,   2,  8,  512, I258FLP(2, 1, 0, 6), 0, 0, i258Pf0, /* [3] */
  40,   2,  4, 1024, I258FLP(2, 1, 0, 6), 0, 0, i258Pf0, /* [4] */

/* 5.25 inch - 360 Kbyte capacity  FD5_7958BR_48TPI */
  40,   2,  9,  512, I258FLP(2, 1, 0, 6), 0, 0, i258Pf0, /* [5] */

/* 5.25 inch - 720 Kbyte capacity  FD5_7958BR_96TPI */
  80,   2,  9,  512, I258FLP(2, 0, 0, 7), 0, 0, i258Pf0, /* [6] */

/* 5.25 inch - 1.2 Mbyte capacity  FD5_13262BR_96TPI */
  80,   2, 15,  512, I258FLP(3, 0, 0, 8), 0, 0, i258Pf0,  /* [7] */

/* 3.5 inch - 720 Kbyte capacity:  FD3_7958BR_135TPI */
  80,   2, 9,  512, I258FLP(2, 0, 0, 9),  0, 0, i258Pf0, /* [8] */

/* 3.5 inch - 1.44 Mbyte capacity  FD3_7958BR_135TPI */
  80,   2, 18,  512, I258FLP(3, 0, 0, 9), 0, 0, i258Pf0  /* [9] */

};

/* Wini partition table space.  Space reserved for V_NUMPAR partitions
   on each of 4 drives.  This is filled in at initialization.  */
struct  i258part i258Piw00 [V_NUMPAR*8];

/*
 * Winchester Device-Table Definitions (drtab's)
 * Set up for 4 winchester drives.
 * Initial assumptions: 2 fixed heads, 1 cylinder, 9 sectors/cylinder,
 * 1024-byte sectors, NO alternates tracks (not letting the controller
 * do it), one partition (which is filled in at initialization to be
 * the entire 1-cylinder, 2-head 'disk').
*/
struct	i258cdrt i258d00[] = {
	1, 2, 9, 1024, 0, 1, 0, &i258Piw00[0],
	1, 2, 9, 1024, 0, 1, 0, &i258Piw00[V_NUMPAR],
	1, 2, 9, 1024, 0, 1, 0, &i258Piw00[2*V_NUMPAR],
	1, 2, 9, 1024, 0, 1, 0, &i258Piw00[3*V_NUMPAR],
	1, 2, 9, 1024, 0, 1, 0, &i258Piw00[4*V_NUMPAR],
	1, 2, 9, 1024, 0, 1, 0, &i258Piw00[5*V_NUMPAR],
	1, 2, 9, 1024, 0, 1, 0, &i258Piw00[6*V_NUMPAR],
	1, 2, 9, 1024, 0, 1, 0, &i258Piw00[7*V_NUMPAR],
};

/*
 * 258 Board configuration.
 *
 * Each structure in this array configures one controller board.  This
 * table specifies the wake-up blocks, and per-controller data-structures
 * for each 258 controller.
 *
*/
/* Array of strings to match against board ID in interconnect space */
char *i258names[] = {"386/258", 0};

struct	i258cfg	 i258cfg[] = {
/* controller 0: */
			i258names,	        
			i258f00,     	/* Floppy 0 */
			i258f00,     	/* Floppy 1 */
			i258d00, 		/* Wini 0 */
			i258d00, 		/* Wini 1 */
			i258d00, 		/* Wini 2 */
			i258d00, 		/* Wini 3 */
			i258t00,     	/* Tape 0 */
			0,     			/* Not Used */
			0,            	/* Not Used */
			0,     			/* Not Used */
			0,            	/* Not Used */
			0,            	/* Not Used */
#ifndef TAPEBOOT
/* controller 1: */
			i258names,		
			i258f00,     	/* Floppy 0 */	
			i258f00,     	/* Floppy 1 */
			i258d00, 		/* Wini 0 */
			i258d00, 		/* Wini 1 */
			i258d00, 		/* Wini 2 */
			i258d00, 		/* Wini 3 */
			i258t00,     	/* Tape 0 */
			0,     			/* Not Used */
			0,            	/* Not Used */
			0,     			/* Not Used */
			0,            	/* Not Used */
			0,            	/* Not Used */
#endif
};

/*
 * i258minor
 *
 * This structure is used to widen the minor number information.
 *
 * This table configures the board number, partition number, drtab number
 * and unit number.  Since these are implemented with 4 bit fields, beware
 * of the C restrictions described in The C Programming Language book by
 * Kernighan/Ritchie (6.7 Fields).
 *
 * i258MINOR is a macro which encodes the board, unit, drtab, and partition
 * table indices into the internal representation of the bitfield.  It is
 * defined in ../sys/i258.h, and is not portable.
 *
 * Usage:
 *	i258MINOR(board, unit, drtab, partition),
 */
struct i258minor i258minor[] = {                /* [minor] device */
/* Assignments for board 0 */
/* Board 0 - Wini 0 */
	i258MINOR(0,2,0,0),             /* [  0] Wini 0 partition 0  */
	i258MINOR(0,2,0,1),             /* [  1] Wini 0 partition 1  */
	i258MINOR(0,2,0,2),             /* [  2] Wini 0 partition 2  */
	i258MINOR(0,2,0,3),             /* [  3] Wini 0 partition 3  */
	i258MINOR(0,2,0,4),             /* [  4] Wini 0 partition 4  */
	i258MINOR(0,2,0,5),             /* [  5] Wini 0 partition 5  */
	i258MINOR(0,2,0,6),             /* [  6] Wini 0 partition 6  */
	i258MINOR(0,2,0,7),             /* [  7] Wini 0 partition 7  */
	i258MINOR(0,2,0,8),             /* [  8] Wini 0 partition 8  */
	i258MINOR(0,2,0,9),             /* [  9] Wini 0 partition 9  */
	i258MINOR(0,2,0,10),            /* [ 10] Wini 0 partition 10 */
	i258MINOR(0,2,0,11),            /* [ 11] Wini 0 partition 11 */
	i258MINOR(0,2,0,12),            /* [ 12] Wini 0 partition 12 */
	i258MINOR(0,2,0,13),            /* [ 13] Wini 0 partition 13 */
	i258MINOR(0,2,0,14),            /* [ 14] Wini 0 partition 14 */
	i258MINOR(0,2,0,15),            /* [ 15] Wini 0 partition 15 */
/* Board 0 - Wini 1 */
	i258MINOR(0,3,1,0),             /* [ 16] Wini 1 partition 0  */
	i258MINOR(0,3,1,1),             /* [ 17] Wini 1 partition 1  */
	i258MINOR(0,3,1,2),             /* [ 18] Wini 1 partition 2  */
	i258MINOR(0,3,1,3),             /* [ 19] Wini 1 partition 3  */
	i258MINOR(0,3,1,4),             /* [ 20] Wini 1 partition 4  */
	i258MINOR(0,3,1,5),             /* [ 21] Wini 1 partition 5  */
	i258MINOR(0,3,1,6),             /* [ 22] Wini 1 partition 6  */
	i258MINOR(0,3,1,7),             /* [ 23] Wini 1 partition 7  */
	i258MINOR(0,3,1,8),             /* [ 24] Wini 1 partition 8  */
	i258MINOR(0,3,1,9),             /* [ 25] Wini 1 partition 9  */
	i258MINOR(0,3,1,10),            /* [ 26] Wini 1 partition 10 */
	i258MINOR(0,3,1,11),            /* [ 27] Wini 1 partition 11 */
	i258MINOR(0,3,1,12),            /* [ 28] Wini 1 partition 12 */
	i258MINOR(0,3,1,13),            /* [ 29] Wini 1 partition 13 */
	i258MINOR(0,3,1,14),            /* [ 30] Wini 1 partition 14 */
	i258MINOR(0,3,1,15),            /* [ 31] Wini 1 partition 15 */
/* Board 0 - Wini 2 */
	i258MINOR(0,4,2,0),             /* [ 32] Wini 2 partition 0  */
	i258MINOR(0,4,2,1),             /* [ 33] Wini 2 partition 1  */
	i258MINOR(0,4,2,2),             /* [ 34] Wini 2 partition 2  */
	i258MINOR(0,4,2,3),             /* [ 35] Wini 2 partition 3  */
	i258MINOR(0,4,2,4),             /* [ 36] Wini 2 partition 4  */
	i258MINOR(0,4,2,5),             /* [ 37] Wini 2 partition 5  */
	i258MINOR(0,4,2,6),             /* [ 38] Wini 2 partition 6  */
	i258MINOR(0,4,2,7),             /* [ 39] Wini 2 partition 7  */
	i258MINOR(0,4,2,8),             /* [ 40] Wini 2 partition 8  */
	i258MINOR(0,4,2,9),             /* [ 41] Wini 2 partition 9  */
	i258MINOR(0,4,2,10),            /* [ 42] Wini 2 partition 10 */
	i258MINOR(0,4,2,11),            /* [ 43] Wini 2 partition 11 */
	i258MINOR(0,4,2,12),            /* [ 44] Wini 2 partition 12 */
	i258MINOR(0,4,2,13),            /* [ 45] Wini 2 partition 13 */
	i258MINOR(0,4,2,14),            /* [ 46] Wini 2 partition 14 */
	i258MINOR(0,4,2,15),            /* [ 47] Wini 2 partition 15 */
/* Board 0 - Wini 3 */
	i258MINOR(0,5,3,0),             /* [ 48] Wini 3 partition 0  */
	i258MINOR(0,5,3,1),             /* [ 49] Wini 3 partition 1  */
	i258MINOR(0,5,3,2),             /* [ 50] Wini 3 partition 2 */
	i258MINOR(0,5,3,3),             /* [ 51] Wini 3 partition 3 */
	i258MINOR(0,5,3,4),             /* [ 52] Wini 3 partition 4 */
	i258MINOR(0,5,3,5),             /* [ 53] Wini 3 partition 5 */
	i258MINOR(0,5,3,6),             /* [ 54] Wini 3 partition 6 */
	i258MINOR(0,5,3,7),             /* [ 55] Wini 3 partition 7 */
	i258MINOR(0,5,3,8),             /* [ 56] Wini 3 partition 8 */
	i258MINOR(0,5,3,9),             /* [ 57] Wini 3 partition 9 */
	i258MINOR(0,5,3,10),            /* [ 58] Wini 3 partition 10 */
	i258MINOR(0,5,3,11),            /* [ 59] Wini 3 partition 11 */
	i258MINOR(0,5,3,12),            /* [ 60] Wini 3 partition 12 */
	i258MINOR(0,5,3,13),            /* [ 61] Wini 3 partition 13 */
	i258MINOR(0,5,3,14),            /* [ 63] Wini 3 partition 14 */
	i258MINOR(0,5,3,15),            /* [ 63] Wini 3 partition 15 */
/* Board 0 - Floppy 0 */
	i258MINOR(0,0,0,0),             /* [ 64]  80 Kbytes - f05s16t  */
	i258MINOR(0,0,2,10),            /* [ 65] 320 Kbytes - f05d16t  */
	i258MINOR(0,0,2,12),            /* [ 66] 320 Kbytes - f05d16   */
	i258MINOR(0,0,3,15),            /* [ 67] 320 Kbytes - f05d8t   */
	i258MINOR(0,0,3,16),            /* [ 68] 320 Kbytes - f05d8u   */
	i258MINOR(0,0,3,17),            /* [ 69] 320 Kbytes - f05d8    */
	i258MINOR(0,0,4,20),            /* [ 70] 320 Kbytes - f05d4t   */
	i258MINOR(0,0,4,22),            /* [ 71] 320 Kbytes - f05d4    */
	i258MINOR(0,0,5,25),            /* [ 72] 360 Kbytes - f05d9t   */
	i258MINOR(0,0,5,27),            /* [ 73] 360 Kbytes - f05d9    */
	i258MINOR(0,0,6,30),            /* [ 74] 720 Kbytes - f05qt    */
	i258MINOR(0,0,6,32),            /* [ 75] 720 Kbytes - f05q     */
	i258MINOR(0,0,7,35),            /* [ 76] 1.2 Mbytes - f05ht    */
	i258MINOR(0,0,7,37),            /* [ 77] 1.2 Mbytes - f05h     */
	i258MINOR(0,0,7,38),            /* [ 78] 1.2 Mbytes - f05hb    */
/* Board 0 - Floppy 1 */
	i258MINOR(0,1,0,0),             /* [ 79] 720 Kbytes - f03d     */
	i258MINOR(0,1,2,10),            /* [ 80] 1.4 Mbytes - f03h     */
	i258MINOR(0,1,2,12),            /* [ 81] 320 Kbytes - f05d16   */
	i258MINOR(0,1,3,15),            /* [ 82] 320 Kbytes - f05d8t   */
	i258MINOR(0,1,3,16),            /* [ 83] 320 Kbytes - f05d8u   */
	i258MINOR(0,1,3,17),            /* [ 84] 320 Kbytes - f05d8    */
	i258MINOR(0,1,4,20),            /* [ 85] 320 Kbytes - f05d4t   */
	i258MINOR(0,1,4,22),            /* [ 86] 320 Kbytes - f05d4    */
	i258MINOR(0,1,5,25),            /* [ 87] 360 Kbytes - f05d9t   */
	i258MINOR(0,1,5,27),            /* [ 88] 360 Kbytes - f05d9    */
	i258MINOR(0,1,6,30),            /* [ 89] 720 Kbytes - f05qt    */
	i258MINOR(0,1,6,32),            /* [ 90] 720 Kbytes - f05q     */
	i258MINOR(0,1,7,35),            /* [ 91] 1.2 Mbytes - f05ht    */
	i258MINOR(0,1,7,37),            /* [ 92] 1.2 Mbytes - f05h     */
	i258MINOR(0,1,7,38),            /* [ 93] 1.2 Mbytes - f05hb    */
/*
 * Floppy 2 and 3 are unsupported since the 258 supports only two floppy
 * drives. The following entries are mapped to floppy 0 and 1 respectively.
 */
/* Board 0 - Floppy 2 */
	i258MINOR(0,0,0,0),             /* [ 94] 720 Kbytes - f03d     */
	i258MINOR(0,0,2,10),            /* [ 95] 1.4 Mbytes - f03h     */
	i258MINOR(0,0,2,12),            /* [ 96] 320 Kbytes - f05d16   */
	i258MINOR(0,0,3,15),            /* [ 97] 320 Kbytes - f05d8t   */
	i258MINOR(0,0,3,16),            /* [ 98] 320 Kbytes - f05d8u   */
	i258MINOR(0,0,3,17),            /* [ 99] 320 Kbytes - f05d8    */
	i258MINOR(0,0,4,20),            /* [100] 320 Kbytes - f05d4t   */
	i258MINOR(0,0,4,22),            /* [101] 320 Kbytes - f05d4    */
	i258MINOR(0,0,5,25),            /* [102] 360 Kbytes - f05d9t   */
	i258MINOR(0,0,5,27),            /* [103] 360 Kbytes - f05d9    */
	i258MINOR(0,0,6,30),            /* [104] 720 Kbytes - f05qt    */
	i258MINOR(0,0,6,32),            /* [105] 720 Kbytes - f05q     */
	i258MINOR(0,0,7,35),            /* [106] 1.2 Mbytes - f05ht    */
	i258MINOR(0,0,7,37),            /* [107] 1.2 Mbytes - f05h     */
	i258MINOR(0,0,7,38),            /* [108] 1.2 Mbytes - f05hb    */
/* Board 0 - Floppy 3 */
	i258MINOR(0,1,0,0),             /* [109] 720 Kbytes - f03h     */
	i258MINOR(0,1,2,10),            /* [110] 1.4 Mbytes - f03d     */
	i258MINOR(0,1,2,12),            /* [111] 320 Kbytes - f05d16   */
	i258MINOR(0,1,3,15),            /* [112] 320 Kbytes - f05d8t   */
	i258MINOR(0,1,3,16),            /* [113] 320 Kbytes - f05d8u   */
	i258MINOR(0,1,3,17),            /* [114] 320 Kbytes - f05d8    */
	i258MINOR(0,1,4,20),            /* [115] 320 Kbytes - f05d4t   */
	i258MINOR(0,1,4,22),            /* [116] 320 Kbytes - f05d4    */
	i258MINOR(0,1,5,25),            /* [117] 360 Kbytes - f05d9t   */
	i258MINOR(0,1,5,27),            /* [118] 360 Kbytes - f05d9    */
	i258MINOR(0,1,6,30),            /* [119] 720 Kbytes - f05qt    */
/*  Tape  1 - optional */
/*	NOTE: the unit number for tape 1 and wini 3 are the same.  Select
 *		  appropriate for your hardware configuration and check the
 *		  i258cfg structure for consistency 					*/
	i258MINOR(0,5,4,0),            	/* [120] Exabyte - c0s1		*/
	i258MINOR(0,5,5,0),            	/* [121] Exabyte - c0s1n	*/
	i258MINOR(0,5,6,0),            	/* [122] Exabyte - c0s1nr	*/
	i258MINOR(0,5,7,0),            	/* [123] Exabyte - c0s1r	*/
/*  Tape  0 - default */
	i258MINOR(0,6,0,0),             /* [124] Archive - c0s0		*/
	i258MINOR(0,6,1,0),             /* [125] Archive - c0s0n	*/
	i258MINOR(0,6,2,0),             /* [126] Archive - c0s0nr	*/
	i258MINOR(0,6,3,0),             /* [127] Archive - c0s0r	*/

/* Assignments for board 1 */
/* Board 1 - Wini 0 */
	i258MINOR(1,2,4,0),             /* [128] Wini 0 partition 0  */
	i258MINOR(1,2,4,1),             /* [129] Wini 0 partition 1  */
	i258MINOR(1,2,4,2),             /* [130] Wini 0 partition 2  */
	i258MINOR(1,2,4,3),             /* [131] Wini 0 partition 3  */
	i258MINOR(1,2,4,4),             /* [132] Wini 0 partition 4  */
	i258MINOR(1,2,4,5),             /* [133] Wini 0 partition 5  */
	i258MINOR(1,2,4,6),             /* [134] Wini 0 partition 6  */
	i258MINOR(1,2,4,7),             /* [135] Wini 0 partition 7  */
	i258MINOR(1,2,4,8),             /* [136] Wini 0 partition 8  */
	i258MINOR(1,2,4,9),             /* [137] Wini 0 partition 9  */
	i258MINOR(1,2,4,10),            /* [138] Wini 0 partition 10 */
	i258MINOR(1,2,4,11),            /* [139] Wini 0 partition 11 */
	i258MINOR(1,2,4,12),            /* [140] Wini 0 partition 12 */
	i258MINOR(1,2,4,13),            /* [141] Wini 0 partition 13 */
	i258MINOR(1,2,4,14),            /* [142] Wini 0 partition 14 */
	i258MINOR(1,2,4,15),            /* [143] Wini 0 partition 15 */
/* Board 1 - Wini 1 */
	i258MINOR(1,3,5,0),             /* [144] Wini 1 partition 0  */
	i258MINOR(1,3,5,1),             /* [145] Wini 1 partition 1  */
	i258MINOR(1,3,5,2),             /* [146] Wini 1 partition 2  */
	i258MINOR(1,3,5,3),             /* [147] Wini 1 partition 3  */
	i258MINOR(1,3,5,4),             /* [148] Wini 1 partition 4  */
	i258MINOR(1,3,5,5),             /* [149] Wini 1 partition 5  */
	i258MINOR(1,3,5,6),             /* [150] Wini 1 partition 6  */
	i258MINOR(1,3,5,7),             /* [151] Wini 1 partition 7  */
	i258MINOR(1,3,5,8),             /* [152] Wini 1 partition 8  */
	i258MINOR(1,3,5,9),             /* [153] Wini 1 partition 9  */
	i258MINOR(1,3,5,10),            /* [154] Wini 1 partition 10 */
	i258MINOR(1,3,5,11),            /* [155] Wini 1 partition 11 */
	i258MINOR(1,3,5,12),            /* [156] Wini 1 partition 12 */
	i258MINOR(1,3,5,13),            /* [157] Wini 1 partition 13 */
	i258MINOR(1,3,5,14),            /* [158] Wini 1 partition 14 */
	i258MINOR(1,3,5,15),            /* [159] Wini 1 partition 15 */
/* Board 1 - Wini 2 */
	i258MINOR(1,4,6,0),             /* [160] Wini 2 partition 0  */
	i258MINOR(1,4,6,1),             /* [161] Wini 2 partition 1  */
	i258MINOR(1,4,6,2),             /* [162] Wini 2 partition 2  */
	i258MINOR(1,4,6,3),             /* [163] Wini 2 partition 3  */
	i258MINOR(1,4,6,4),             /* [164] Wini 2 partition 4  */
	i258MINOR(1,4,6,5),             /* [165] Wini 2 partition 5  */
	i258MINOR(1,4,6,6),             /* [166] Wini 2 partition 6  */
	i258MINOR(1,4,6,7),             /* [167] Wini 2 partition 7  */
	i258MINOR(1,4,6,8),             /* [168] Wini 2 partition 8  */
	i258MINOR(1,4,6,9),             /* [169] Wini 2 partition 9  */
	i258MINOR(1,4,6,10),            /* [170] Wini 2 partition 10 */
	i258MINOR(1,4,6,11),            /* [171] Wini 2 partition 11 */
	i258MINOR(1,4,6,12),            /* [172] Wini 2 partition 12 */
	i258MINOR(1,4,6,13),            /* [173] Wini 2 partition 13 */
	i258MINOR(1,4,6,14),            /* [174] Wini 2 partition 14 */
	i258MINOR(1,4,6,15),            /* [175] Wini 2 partition 15 */
/* Board 1 - Wini 3 */
	i258MINOR(1,5,7,0),             /* [176] Wini 3 partition 0  */
	i258MINOR(1,5,7,1),             /* [177] Wini 3 partition 1  */
	i258MINOR(1,5,7,2),             /* [178] Wini 3 partition 2  */
	i258MINOR(1,5,7,3),             /* [179] Wini 3 partition 3  */
	i258MINOR(1,5,7,4),             /* [180] Wini 3 partition 4  */
	i258MINOR(1,5,7,5),             /* [181] Wini 3 partition 5  */
	i258MINOR(1,5,7,6),             /* [182] Wini 3 partition 6  */
	i258MINOR(1,5,7,7),             /* [183] Wini 3 partition 7  */
	i258MINOR(1,5,7,8),             /* [184] Wini 3 partition 8  */
	i258MINOR(1,5,7,9),             /* [185] Wini 3 partition 9  */
	i258MINOR(1,5,7,10),            /* [186] Wini 3 partition 10 */
	i258MINOR(1,5,7,11),            /* [187] Wini 3 partition 11 */
	i258MINOR(1,5,7,12),            /* [188] Wini 3 partition 12 */
	i258MINOR(1,5,7,13),            /* [189] Wini 3 partition 13 */
	i258MINOR(1,5,7,14),            /* [190] Wini 3 partition 14 */
	i258MINOR(1,5,7,15),            /* [191] Wini 3 partition 15 */
/* Board 1 - Floppy 0 */
	i258MINOR(1,0,0,0),             /* [192]  80 Kbytes - f05s16t  */
	i258MINOR(1,0,2,10),            /* [193] 320 Kbytes - f05d16t  */
	i258MINOR(1,0,2,12),            /* [194] 320 Kbytes - f05d16   */
	i258MINOR(1,0,3,15),            /* [195] 320 Kbytes - f05d8t   */
	i258MINOR(1,0,3,16),            /* [196] 320 Kbytes - f05d8u   */
	i258MINOR(1,0,3,17),            /* [197] 320 Kbytes - f05d8    */
	i258MINOR(1,0,4,20),            /* [198] 320 Kbytes - f05d4t   */
	i258MINOR(1,0,4,22),            /* [199] 320 Kbytes - f05d4    */
	i258MINOR(1,0,5,25),            /* [200] 360 Kbytes - f05d9t   */
	i258MINOR(1,0,5,27),            /* [201] 360 Kbytes - f05d9    */
	i258MINOR(1,0,6,30),            /* [202] 720 Kbytes - f05qt    */
	i258MINOR(1,0,6,32),            /* [203] 720 Kbytes - f05q     */
	i258MINOR(1,0,7,35),            /* [204] 1.2 Mbytes - f05ht    */
	i258MINOR(1,0,7,37),            /* [205] 1.2 Mbytes - f05h     */
	i258MINOR(1,0,7,38),            /* [206] 1.2 Mbytes - f05hb    */
/* Board 1 - Floppy 1 */
	i258MINOR(1,1,0,0),             /* [207]  80 Kbytes - f05s16t  */
	i258MINOR(1,1,2,10),            /* [208] 320 Kbytes - f05d16t  */
	i258MINOR(1,1,2,12),            /* [209] 320 Kbytes - f05d16   */
	i258MINOR(1,1,3,15),            /* [210] 320 Kbytes - f05d8t   */
	i258MINOR(1,1,3,16),            /* [211] 320 Kbytes - f05d8u   */
	i258MINOR(1,1,3,17),            /* [212] 320 Kbytes - f05d8    */
	i258MINOR(1,1,4,20),            /* [213] 320 Kbytes - f05d4t   */
	i258MINOR(1,1,4,22),            /* [214] 320 Kbytes - f05d4    */
	i258MINOR(1,1,5,25),            /* [215] 360 Kbytes - f05d9t   */
	i258MINOR(1,1,5,27),            /* [216] 360 Kbytes - f05d9    */
	i258MINOR(1,1,6,30),            /* [217] 720 Kbytes - f05qt    */
	i258MINOR(1,1,6,32),            /* [218] 720 Kbytes - f05q     */
	i258MINOR(1,1,7,35),            /* [219] 1.2 Mbytes - f05ht    */
	i258MINOR(1,1,7,37),            /* [220] 1.2 Mbytes - f05h     */
	i258MINOR(1,1,7,38),            /* [221] 1.2 Mbytes - f05hb    */
/*
 * Floppy 2 and 3 are unsupported since the 258 supports only two floppy
 * drives. The following entries are mapped to floppy 0 and 1 respectively.
 */
/* Board 1 - Floppy 2 */
	i258MINOR(1,0,0,0),             /* [222]  80 Kbytes - f05s16t  */
	i258MINOR(1,0,2,10),            /* [223] 320 Kbytes - f05d16t  */
	i258MINOR(1,0,2,12),            /* [224] 320 Kbytes - f05d16   */
	i258MINOR(1,0,3,15),            /* [225] 320 Kbytes - f05d8t   */
	i258MINOR(1,0,3,16),            /* [226] 320 Kbytes - f05d8u   */
	i258MINOR(1,0,3,17),            /* [227] 320 Kbytes - f05d8    */
	i258MINOR(1,0,4,20),            /* [228] 320 Kbytes - f05d4t   */
	i258MINOR(1,0,4,22),            /* [229] 320 Kbytes - f05d4    */
	i258MINOR(1,0,5,25),            /* [230] 360 Kbytes - f05d9t   */
	i258MINOR(1,0,5,27),            /* [231] 360 Kbytes - f05d9    */
	i258MINOR(1,0,6,30),            /* [232] 720 Kbytes - f05qt    */
	i258MINOR(1,0,6,32),            /* [233] 720 Kbytes - f05q     */
	i258MINOR(1,0,7,35),            /* [234] 1.2 Mbytes - f05ht    */
	i258MINOR(1,0,7,37),            /* [235] 1.2 Mbytes - f05h     */
	i258MINOR(1,0,7,38),            /* [236] 1.2 Mbytes - f05hb    */
/* Board 1 - Floppy 3 */
	i258MINOR(1,1,0,0),             /* [237]  80 Kbytes - f05s16t  */
	i258MINOR(1,1,2,10),            /* [238] 320 Kbytes - f05d16t  */
	i258MINOR(1,1,2,12),            /* [239] 320 Kbytes - f05d16   */
	i258MINOR(1,1,3,15),            /* [240] 320 Kbytes - f05d8t   */
	i258MINOR(1,1,3,16),            /* [241] 320 Kbytes - f05d8u   */
	i258MINOR(1,1,3,17),            /* [242] 320 Kbytes - f05d8    */
	i258MINOR(1,1,4,20),            /* [243] 320 Kbytes - f05d4t   */
	i258MINOR(1,1,4,22),            /* [244] 320 Kbytes - f05d4    */
	i258MINOR(1,1,5,25),            /* [245] 360 Kbytes - f05d9t   */
	i258MINOR(1,1,5,27),            /* [246] 360 Kbytes - f05d9    */
	i258MINOR(1,1,6,30),            /* [247] 720 Kbytes - f05qt    */
/* Board 1 - Tape  1 - optional */
/*	NOTE: the unit number for tape 1 and wini 3 are the same.  Select 
 *		  appropriate for your hardware configuration and check the
 *		  i258cfg structure for consistency 					*/
	i258MINOR(1,5,4,0),            	/* [248] Exabyte - c1s1		*/
	i258MINOR(1,5,5,0),            	/* [249] Exabyte - c1s1n	*/
	i258MINOR(1,5,6,0),            	/* [250] Exabyte - c1s1nr	*/
	i258MINOR(1,5,7,0),            	/* [251] Exabyte - c1s1r	*/
/* Board 1 - Tape  0 - default */
	i258MINOR(1,6,0,0),             /* [252] Archive - c1s0		*/
	i258MINOR(1,6,1,0),             /* [253] Archive - c1s0n	*/
	i258MINOR(1,6,2,0),             /* [254] Archive - c1s0nr	*/
	i258MINOR(1,6,3,0),             /* [255] Archive - c1s0r	*/
};

/*
 *	Number of retries in case of soft error (configurable).
*/
int i258retry = 10;

int i258print_warnings = 0;		/* do not print warnings */

/*
 * Note: i258 MUST have contiguous major numbers AND have
 *	 same 1st index in bdevsw[] & cdevsw[].
*/
int	i258fmaj = 0;				/* 1st {b,c}devsw index */

/*
 * The following are static initialization variables
 * which are based on the configuration. These variables
 * MUST NOT CHANGE because the i258 device driver makes
 * most of the calculations based on these variables.
*/
#define	NUM258	((sizeof i258cfg) / (sizeof (struct i258cfg)))
int			N_i258 =   NUM258;		/* number of configured boards   */
int			i258_max_req = MAXREQ;		/* Max outstanding requests per board*/
struct 		iobuf    i258tab[NUM258];	/* buffer headers per board      */
#ifndef NOSAR
struct 		iotime	 i258stat[NUM258][NUMUNITS]; /* I/O statistics / device*/
#endif
struct 		i258dev i258dev[NUM258];	/* per-board data-structures     */
struct 		i258wini i258winidata[NUM258][I258_NUMWINI]; /* per-wini data */
struct 		i258rdvfy i258rvdata[NUM258][I258_NUMWINI]; /* per-wini data */
minor_t		i258maxmin = ((sizeof i258minor)/(sizeof (struct i258minor)));
											/* maximum minor number posible. */
/*
 * i258pci_bin is a list of instances of board(s) capable of running PCI
 * this driver should be talking to.  Default value of 0 indicates any PCI
 * controller which responds to the locate PCI message. The instance values
 * are counted from 1. Valid values are from 1-21.
 * Note: if you have only one iSBC 386/258 in the system then specify 0 so
 * that the board can be moved around in the backplane without having to
 * regenerate a new kernel.
 *
 * i258pci_sin is the instance of the PCI server on a particular controller.
 * 0 - is default (any server).  Valid values are 1 - n, where,  n is number
 * of PCI servers on a controller.
 */
int 	i258pci_bin[NUM258] = {0};	/* list of PCI board instances */
int 	i258pci_sin[NUM258] = {0};	/* list of PCI server instances */

/*
 * bad block handling parameters
 */
int i258no_trk_zone = 1;			/* number of tracks per zone */
int i258no_alt_sec_zone = 0;		/* the number of slip sectors/zone */
int i258no_alt_trk_zone = 0;		/* the number of alt tracks/zone */
int i258no_alt_cyl_vol = 12;		/* the number of alternate cylinders */

/*
 *  Configurable Unit options
 */

unchar	i258sio_mode = CMD_REORDER_ENABLE|CACHE_MODE_ENABLE	;
										/* logical addressing, command re- */
										/* ordering enable, cache enable */
unchar	i258sio_nretries = 0x8;			/* 8 retries */
unchar	i258sio_cmd_ordering = DEF_SEEK_ORDERING;
										/* default seek ordering and equal */
										/* priority in reads and writes */
unchar	i258sio_cache_size = 0xFF;		/* the default cache size */
unchar	i258sio_read_ahead = 0xFF;		/* default controller selection */

#ifdef TAPEBOOT
/*
 * For the initial installation, we use tape buffering.
 */
struct	i258tp_buf 	i28tbuf[] = {
			MAXTAPEPAGES, 6, 			/* The maximum allowed */
			MAXTAPEPAGES, 5				 /* The maximum allowed */
};
#else
/*
 *	READ-AHEAD AND WRITE-BEHIND TAPE BUFFERING:
 *
 * The default (for an installed system) is no tape buffering by the driver.
 * The tape read/write requests are handled synchronously, and errors on i/o
 * operations (like end-of-tape) are reported back to the requesting process.
 *
 * For better performance, you may want to use read-ahead and write-behind
 * tape buffering/caching by the driver; For tape buffering set i258tbuf_pages
 * to a large enough value (but no greater than MAXTAPEPAGES).
 */
struct	i258tp_buf	i25tbuf[] = {
				0, 6,					/* Default: No tape buffering */
				0, 5					/* Optional: No tape buffering */
};
#endif
int		i25t_elements = ((sizeof i25tbuf)/(sizeof (struct i258tp_buf)));

