/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mb1:uts/i386/master.d/i214/space.c	1.3"

/**************************************************************************
 *
 *	iSBC 214 Specific Configuration file.
 *      THIS FILE ASSUMES PROPER VALUES ARE OBTAINED FROM VTOC INFO!
 *
 *	REVISION HISTORY
 *
 *	I001	JGR	1/12/88
 *		Port to v.3.1.  Changed alttbl to alt_info.
 *
 ***************************************************************************/

#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/elog.h"
#include "sys/fdisk.h"
#include "sys/ivlab.h"
#include "sys/vtoc.h"
#include "sys/alttbl.h"
#include "sys/bbh.h"
#include "sys/i214.h"

/*
 * Note that FLPYTYPE gets overridden during each floppy-device open.
 */

#define i214TYPE DEVWINIG
#define FLPYTYPE DEV5FLPY
#define TAPETYPE STREAMER


/*
 * STREAMER Tape Partition Table (dummy).
*/
static struct	i214part Mmt0[] = {
	0,V_VOMASK,0,              0                       /* Archive streamer */
};

/*
 * 214 Board 0 unit 8 (Tape) Device-Table Definitions (drtabs)
 *
 * Note: Tape is treated differently from disk.
 *	Formatting a tape unit will cause the Tape to be erased.
 *	Sector Size refers to the block size of the tape device.
 *	#Sec (Number of sectors per Track) is the Number of Blocks
 *	per Tape.  Streamer Tapes (Archive) Must be blocked in a
 *	multiple of the on board buffer size (512 bytes).
 *
 */
static struct	i214cdrt i214t00[] = {
/* pesent, no-op, no-op, no-op, no-op,!rew?, Part, NoPrt.        */
  01,      0,     0,     0,     0,    0, Mmt0, 0, /*[0] Streamer  */
  01,      0,     0,     0,     0,    DR_NO_REWIND, Mmt0, 0 /*[1] No rewind */
};

/*
 * Floppy Partitions.
 *	This table lists the possible partitions for each diskette format.
 *	There are 2 or 3 standard partitions for each format and room for
 *	a few additional definitions.  Each disk format can have up to 5 
 *	partition definitions.  WARNING: It is possible to use i214minor
 *	to specify combinations of i214cdrt and i214part entries that do
 *	not make sense so use caution when filling out the minor table.
 */
static struct	i214part Pf0[] = {
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
};

/*
 * Floppy Device-Table Definitions (drtabs)
 *	This table defines the possible diskette formats that are suppored
 *	by this board.  Each diskette format has a corresponding set of
 *	partitions definitions as specified in i214part.  WARNING: It is 
 *	possible to use i214minor to specify combinations of i214cdrt 
 *	and i214part entries that do not make sense.
 */
static struct	i214cdrt i214f00[] = {
/*Cyls, Fixed, Remov,  #Sec,  SecSiz,  Nalt, Part, NoPrt  Unit: Drive-Type */

/* 5.25 inch - 80 Kbyte capacity:  ISO Standard for Track 0 */
  40,   0,      1,     16,      128,  FLPY_FM,  Pf0, 0, /* [0] */

/* 5.25 inch - 160 Kbyte capacity */
  40,   0,      1,      8,      512,  FLPY_MFM, Pf0, 0, /* [1] */

/* 5.25 inch - 320 Kbyte capacity */
  40,   0,      2,     16,      256,  FLPY_MFM, Pf0, 0, /* [2] */
  40,   0,      2,      8,      512,  FLPY_MFM, Pf0, 0, /* [3] */
  40,   0,      2,      4,     1024,  FLPY_MFM, Pf0, 0, /* [4] */

/* 5.25 inch - 360 Kbyte capacity */
  40,   0,      2,      9,      512,  FLPY_MFM, Pf0, 0, /* [5] */

/* 5.25 inch - 720 Kbyte capacity */
  80,   0,      2,      9,      512,  FLPY_MFM, Pf0, 0, /* [6] */

/* 5.25 inch - 1.2 Mbyte capacity */
  80,   0,      2,     15,      512,  FLPY_MFM, Pf0, 0  /* [7] */
};

/* Wini partition table space.  Space reserved for V_NUMPAR partitions
   on each of 4 drives.  This is filled in at initialization.  */
static struct  i214part Piw00 [V_NUMPAR*8];

/* Wini Alternate Info space.  space reserved for 8 alt info structures */
struct  alt_info i214_alts[8];

/*
 * Winchester Device-Table Definitions (drtab's)
 * Set up for 4 winchester drives.
 * Initial assumptions: 2 fixed heads, 1 cylinder, 9 sectors/cylinder,
 * 1024-byte sectors, NO alternates tracks (not letting the controller
 * do it), one partition (which is filled in at initialization to be
 * the entire 1-cylinder, 2-head 'disk').
*/
static struct	i214cdrt i214d00[] = {
	1, 2, 0, 9, 1024, 0, &Piw00[0],          1,
	1, 2, 0, 9, 1024, 0, &Piw00[V_NUMPAR],   1,
	1, 2, 0, 9, 1024, 0, &Piw00[2*V_NUMPAR], 1,
	1, 2, 0, 9, 1024, 0, &Piw00[3*V_NUMPAR], 1,
	1, 2, 0, 9, 1024, 0, &Piw00[4*V_NUMPAR], 1,
	1, 2, 0, 9, 1024, 0, &Piw00[5*V_NUMPAR], 1,
	1, 2, 0, 9, 1024, 0, &Piw00[6*V_NUMPAR], 1,
	1, 2, 0, 9, 1024, 0, &Piw00[7*V_NUMPAR], 1,
};

/*
 * 214 Board configuration.
 *
 * Each structure in this array configures one controller board.  This
 * table specifies the wake-up blocks, and per-controller data-structures
 * for each 218 or 220 controller.
 *
 * Note that FLPYTYPE gets overridden during each floppy-device open.
*/
struct	i214cfg	 i214cfg[] = {
/* WUA, DevCode[0], [1],       [2],   Int, Device Table[unit] */
/* controller 0: */
0x01000L, i214TYPE, FLPYTYPE, TAPETYPE, 5, &i214d00[0], /* Wini 0 */
					   &i214d00[1], /* Wini 1 */
					   &i214d00[2], /* Wini 2 */
					   &i214d00[3], /* Wini 3 */
					   i214f00,     /* Floppy 0 */
					   i214f00,     /* Floppy 1 */
					   i214f00,     /* Floppy 2 */
					   i214f00,     /* Floppy 3 */
					   i214t00,     /* Tape 0 */
					   i214t00,     /* Tape 1 */
					   0,           /* Tape 2 */
					   0,           /* Tape 3 */
};


/* Base-device table -- used only for wini's.  Gives appropriate minor number
 * to use to access partition 0 for a given partition.  BASEDEV in i214.h
 * is used for access.  WARNING: BASEDEV and this table must be changed if
 * going to support more than 4 wini drives per controller or more than
 * 2 controllers.
 */
int	i214bases[][FIRSTFLOPPY] = {
		/* Wini Unit#    0    1    2    3  */
		/* ================================*/
		/* Board 0 */	  0,  16,  32,  48, 
		/* Board 1 */	128, 144, 160, 176
		};

/*
 * i214minor
 *
 * This structure is used to widen the minor number information.
 *
 * This table configures the board number, partition number, drtab number
 * and unit number.  Since these are implemented with 4 bit fields, beware
 * of the C restrictions described in The C Programming Language book by
 * Kernighan/Ritchie (6.7 Fields).
 *
 * I214MINOR is a macro which encodes the board, unit, drtab, and partition
 * table indices into the internal representation of the bitfield.  It is
 * defined in ../h/i214.h, and is not portable.
 *
 * Usage:
 *	I214MINOR(board, unit, drtab, partition),
 */
unsigned i214minor[] = {                /* [minor] device */
/* Assignments for board 0 */
/* Board 0 - Wini 0 */
	i214MINOR(0,0,0,0),             /* [  0] Wini 0 partition 0  */
	i214MINOR(0,0,0,1),             /* [  1] Wini 0 partition 1  */
	i214MINOR(0,0,0,2),             /* [  2] Wini 0 partition 2  */
	i214MINOR(0,0,0,3),             /* [  3] Wini 0 partition 3  */
	i214MINOR(0,0,0,4),             /* [  4] Wini 0 partition 4  */
	i214MINOR(0,0,0,5),             /* [  5] Wini 0 partition 5  */
	i214MINOR(0,0,0,6),             /* [  6] Wini 0 partition 6  */
	i214MINOR(0,0,0,7),             /* [  7] Wini 0 partition 7  */
	i214MINOR(0,0,0,8),             /* [  8] Wini 0 partition 8  */
	i214MINOR(0,0,0,9),             /* [  9] Wini 0 partition 9  */
	i214MINOR(0,0,0,10),            /* [ 10] Wini 0 partition 10 */
	i214MINOR(0,0,0,11),            /* [ 11] Wini 0 partition 11 */
	i214MINOR(0,0,0,12),            /* [ 12] Wini 0 partition 12 */
	i214MINOR(0,0,0,13),            /* [ 13] Wini 0 partition 13 */
	i214MINOR(0,0,0,14),            /* [ 14] Wini 0 partition 14 */
	i214MINOR(0,0,0,15),            /* [ 15] Wini 0 partition 15 */
/* Board 0 - Wini 1 */
	i214MINOR(0,1,0,0),             /* [ 16] Wini 1 partition 0  */
	i214MINOR(0,1,0,1),             /* [ 17] Wini 1 partition 1  */
	i214MINOR(0,1,0,2),             /* [ 18] Wini 1 partition 2  */
	i214MINOR(0,1,0,3),             /* [ 19] Wini 1 partition 3  */
	i214MINOR(0,1,0,4),             /* [ 20] Wini 1 partition 4  */
	i214MINOR(0,1,0,5),             /* [ 21] Wini 1 partition 5  */
	i214MINOR(0,1,0,6),             /* [ 22] Wini 1 partition 6  */
	i214MINOR(0,1,0,7),             /* [ 23] Wini 1 partition 7  */
	i214MINOR(0,1,0,8),             /* [ 24] Wini 1 partition 8  */
	i214MINOR(0,1,0,9),             /* [ 25] Wini 1 partition 9  */
	i214MINOR(0,1,0,10),            /* [ 26] Wini 1 partition 10 */
	i214MINOR(0,1,0,11),            /* [ 27] Wini 1 partition 11 */
	i214MINOR(0,1,0,12),            /* [ 28] Wini 1 partition 12 */
	i214MINOR(0,1,0,13),            /* [ 29] Wini 1 partition 13 */
	i214MINOR(0,1,0,14),            /* [ 30] Wini 1 partition 14 */
	i214MINOR(0,1,0,15),            /* [ 31] Wini 1 partition 15 */
/* Board 0 - Wini 2 */
	i214MINOR(0,2,0,0),             /* [ 32] Wini 2 partition 0  */
	i214MINOR(0,2,0,1),             /* [ 33] Wini 2 partition 1  */
	i214MINOR(0,2,0,2),             /* [ 34] Wini 2 partition 2  */
	i214MINOR(0,2,0,3),             /* [ 35] Wini 2 partition 3  */
	i214MINOR(0,2,0,4),             /* [ 36] Wini 2 partition 4  */
	i214MINOR(0,2,0,5),             /* [ 37] Wini 2 partition 5  */
	i214MINOR(0,2,0,6),             /* [ 38] Wini 2 partition 6  */
	i214MINOR(0,2,0,7),             /* [ 39] Wini 2 partition 7  */
	i214MINOR(0,2,0,8),             /* [ 40] Wini 2 partition 8  */
	i214MINOR(0,2,0,9),             /* [ 41] Wini 2 partition 9  */
	i214MINOR(0,2,0,10),            /* [ 42] Wini 2 partition 10 */
	i214MINOR(0,2,0,11),            /* [ 43] Wini 2 partition 11 */
	i214MINOR(0,2,0,12),            /* [ 44] Wini 2 partition 12 */
	i214MINOR(0,2,0,13),            /* [ 45] Wini 2 partition 13 */
	i214MINOR(0,2,0,14),            /* [ 46] Wini 2 partition 14 */
	i214MINOR(0,2,0,15),            /* [ 47] Wini 2 partition 15 */
/* Board 0 - Wini 3 */
	i214MINOR(0,3,0,0),             /* [ 48] Wini 3 partition 0  */
	i214MINOR(0,3,0,1),             /* [ 49] Wini 3 partition 1  */
	i214MINOR(0,3,0,2),             /* [ 50] Wini 3 partition 2 */
	i214MINOR(0,3,0,3),             /* [ 51] Wini 3 partition 3 */
	i214MINOR(0,3,0,4),             /* [ 52] Wini 3 partition 4 */
	i214MINOR(0,3,0,5),             /* [ 53] Wini 3 partition 5 */
	i214MINOR(0,3,0,6),             /* [ 54] Wini 3 partition 6 */
	i214MINOR(0,3,0,7),             /* [ 55] Wini 3 partition 7 */
	i214MINOR(0,3,0,8),             /* [ 56] Wini 3 partition 8 */
	i214MINOR(0,3,0,9),             /* [ 57] Wini 3 partition 9 */
	i214MINOR(0,3,0,10),            /* [ 58] Wini 3 partition 10 */
	i214MINOR(0,3,0,11),            /* [ 59] Wini 3 partition 11 */
	i214MINOR(0,3,0,12),            /* [ 60] Wini 3 partition 12 */
	i214MINOR(0,3,0,13),            /* [ 61] Wini 3 partition 13 */
	i214MINOR(0,3,0,14),            /* [ 63] Wini 3 partition 14 */
	i214MINOR(0,3,0,15),            /* [ 63] Wini 3 partition 15 */
/* Board 0 - Floppy 0 */
	i214MINOR(0,4,0,0),             /* [ 64]  80 Kbytes - f05s16t  */
	i214MINOR(0,4,2,10),            /* [ 65] 320 Kbytes - f05d16t  */
	i214MINOR(0,4,2,12),            /* [ 66] 320 Kbytes - f05d16   */
	i214MINOR(0,4,3,15),            /* [ 67] 320 Kbytes - f05d8t   */
	i214MINOR(0,4,3,16),            /* [ 68] 320 Kbytes - f05d8u   */
	i214MINOR(0,4,3,17),            /* [ 69] 320 Kbytes - f05d8    */
	i214MINOR(0,4,4,20),            /* [ 70] 320 Kbytes - f05d4t   */
	i214MINOR(0,4,4,22),            /* [ 71] 320 Kbytes - f05d4    */
	i214MINOR(0,4,5,25),            /* [ 72] 360 Kbytes - f05d9t   */
	i214MINOR(0,4,5,27),            /* [ 73] 360 Kbytes - f05d9    */
	i214MINOR(0,4,6,30),            /* [ 74] 720 Kbytes - f05qt    */
	i214MINOR(0,4,6,32),            /* [ 75] 720 Kbytes - f05q     */
	i214MINOR(0,4,7,35),            /* [ 76] 1.2 Mbytes - f05ht    */
	i214MINOR(0,4,7,37),            /* [ 77] 1.2 Mbytes - f05h     */
	i214MINOR(0,4,7,38),            /* [ 78] 1.2 Mbytes - f05hb    */
/* Board 0 - Floppy 1 */
	i214MINOR(0,5,0,0),             /* [ 79]  80 Kbytes - f05s16t  */
	i214MINOR(0,5,2,10),            /* [ 80] 320 Kbytes - f05d16t  */
	i214MINOR(0,5,2,12),            /* [ 81] 320 Kbytes - f05d16   */
	i214MINOR(0,5,3,15),            /* [ 82] 320 Kbytes - f05d8t   */
	i214MINOR(0,5,3,16),            /* [ 83] 320 Kbytes - f05d8u   */
	i214MINOR(0,5,3,17),            /* [ 84] 320 Kbytes - f05d8    */
	i214MINOR(0,5,4,20),            /* [ 85] 320 Kbytes - f05d4t   */
	i214MINOR(0,5,4,22),            /* [ 86] 320 Kbytes - f05d4    */
	i214MINOR(0,5,5,25),            /* [ 87] 360 Kbytes - f05d9t   */
	i214MINOR(0,5,5,27),            /* [ 88] 360 Kbytes - f05d9    */
	i214MINOR(0,5,6,30),            /* [ 89] 720 Kbytes - f05qt    */
	i214MINOR(0,5,6,32),            /* [ 90] 720 Kbytes - f05q     */
	i214MINOR(0,5,7,35),            /* [ 91] 1.2 Mbytes - f05ht    */
	i214MINOR(0,5,7,37),            /* [ 92] 1.2 Mbytes - f05h     */
	i214MINOR(0,5,7,38),            /* [ 93] 1.2 Mbytes - f05hb    */
/* Board 0 - Floppy 2 */
	i214MINOR(0,6,0,0),             /* [ 94]  80 Kbytes - f05s16t  */
	i214MINOR(0,6,2,10),            /* [ 95] 320 Kbytes - f05d16t  */
	i214MINOR(0,6,2,12),            /* [ 96] 320 Kbytes - f05d16   */
	i214MINOR(0,6,3,15),            /* [ 97] 320 Kbytes - f05d8t   */
	i214MINOR(0,6,3,16),            /* [ 98] 320 Kbytes - f05d8u   */
	i214MINOR(0,6,3,17),            /* [ 99] 320 Kbytes - f05d8    */
	i214MINOR(0,6,4,20),            /* [100] 320 Kbytes - f05d4t   */
	i214MINOR(0,6,4,22),            /* [101] 320 Kbytes - f05d4    */
	i214MINOR(0,6,5,25),            /* [102] 360 Kbytes - f05d9t   */
	i214MINOR(0,6,5,27),            /* [103] 360 Kbytes - f05d9    */
	i214MINOR(0,6,6,30),            /* [104] 720 Kbytes - f05qt    */
	i214MINOR(0,6,6,32),            /* [105] 720 Kbytes - f05q     */
	i214MINOR(0,6,7,35),            /* [106] 1.2 Mbytes - f05ht    */
	i214MINOR(0,6,7,37),            /* [107] 1.2 Mbytes - f05h     */
	i214MINOR(0,6,7,38),            /* [108] 1.2 Mbytes - f05hb    */
/* Board 0 - Floppy 3 */
	i214MINOR(0,7,0,0),             /* [109]  80 Kbytes - f05s16t  */
	i214MINOR(0,7,2,10),            /* [110] 320 Kbytes - f05d16t  */
	i214MINOR(0,7,2,12),            /* [111] 320 Kbytes - f05d16   */
	i214MINOR(0,7,3,15),            /* [112] 320 Kbytes - f05d8t   */
	i214MINOR(0,7,3,16),            /* [113] 320 Kbytes - f05d8u   */
	i214MINOR(0,7,3,17),            /* [114] 320 Kbytes - f05d8    */
	i214MINOR(0,7,4,20),            /* [115] 320 Kbytes - f05d4t   */
	i214MINOR(0,7,4,22),            /* [116] 320 Kbytes - f05d4    */
	i214MINOR(0,7,5,25),            /* [117] 360 Kbytes - f05d9t   */
	i214MINOR(0,7,5,27),            /* [118] 360 Kbytes - f05d9    */
	i214MINOR(0,7,6,30),            /* [119] 720 Kbytes - f05qt    */
	i214MINOR(0,7,6,32),            /* [120] 720 Kbytes - f05q     */
	i214MINOR(0,7,7,35),            /* [121] 1.2 Mbytes - f05ht    */
	i214MINOR(0,7,7,37),            /* [122] 1.2 Mbytes - f05h     */
	i214MINOR(0,7,7,38),            /* [123] 1.2 Mbytes - f05hb    */
/* Board 0 - Tape 0 */
	i214MINOR(0,8,0,0),             /* [124]  smt0 st rewind   */
	i214MINOR(0,8,1,0),             /* [125]  smnt0 st no-rew  */
/* Board 0 - Tape 1 */
	i214MINOR(0,9,0,0),             /* [126]  smt1 st rewind   */
	i214MINOR(0,9,1,0),             /* [127]  smnt1 st no-rew  */

/* Assignments for board 1 */
/* Board 1 - Wini 0 */
	i214MINOR(1,0,0,0),             /* [128] Wini 0 partition 0  */
	i214MINOR(1,0,0,1),             /* [129] Wini 0 partition 1  */
	i214MINOR(1,0,0,2),             /* [130] Wini 0 partition 2  */
	i214MINOR(1,0,0,3),             /* [131] Wini 0 partition 3  */
	i214MINOR(1,0,0,4),             /* [132] Wini 0 partition 4  */
	i214MINOR(1,0,0,5),             /* [133] Wini 0 partition 5  */
	i214MINOR(1,0,0,6),             /* [134] Wini 0 partition 6  */
	i214MINOR(1,0,0,7),             /* [135] Wini 0 partition 7  */
	i214MINOR(1,0,0,8),             /* [136] Wini 0 partition 8  */
	i214MINOR(1,0,0,9),             /* [137] Wini 0 partition 9  */
	i214MINOR(1,0,0,10),            /* [138] Wini 0 partition 10 */
	i214MINOR(1,0,0,11),            /* [139] Wini 0 partition 11 */
	i214MINOR(1,0,0,12),            /* [140] Wini 0 partition 12 */
	i214MINOR(1,0,0,13),            /* [141] Wini 0 partition 13 */
	i214MINOR(1,0,0,14),            /* [142] Wini 0 partition 14 */
	i214MINOR(1,0,0,15),            /* [143] Wini 0 partition 15 */
/* Board 1 - Wini 1 */
	i214MINOR(1,1,0,0),             /* [144] Wini 1 partition 0  */
	i214MINOR(1,1,0,1),             /* [145] Wini 1 partition 1  */
	i214MINOR(1,1,0,2),             /* [146] Wini 1 partition 2  */
	i214MINOR(1,1,0,3),             /* [147] Wini 1 partition 3  */
	i214MINOR(1,1,0,4),             /* [148] Wini 1 partition 4  */
	i214MINOR(1,1,0,5),             /* [149] Wini 1 partition 5  */
	i214MINOR(1,1,0,6),             /* [150] Wini 1 partition 6  */
	i214MINOR(1,1,0,7),             /* [151] Wini 1 partition 7  */
	i214MINOR(1,1,0,8),             /* [152] Wini 1 partition 8  */
	i214MINOR(1,1,0,9),             /* [153] Wini 1 partition 9  */
	i214MINOR(1,1,0,10),            /* [154] Wini 1 partition 10 */
	i214MINOR(1,1,0,11),            /* [155] Wini 1 partition 11 */
	i214MINOR(1,1,0,12),            /* [156] Wini 1 partition 12 */
	i214MINOR(1,1,0,13),            /* [157] Wini 1 partition 13 */
	i214MINOR(1,1,0,14),            /* [158] Wini 1 partition 14 */
	i214MINOR(1,1,0,15),            /* [159] Wini 1 partition 15 */
/* Board 1 - Wini 2 */
	i214MINOR(1,2,0,0),             /* [160] Wini 2 partition 0  */
	i214MINOR(1,2,0,1),             /* [161] Wini 2 partition 1  */
	i214MINOR(1,2,0,2),             /* [162] Wini 2 partition 2  */
	i214MINOR(1,2,0,3),             /* [163] Wini 2 partition 3  */
	i214MINOR(1,2,0,4),             /* [164] Wini 2 partition 4  */
	i214MINOR(1,2,0,5),             /* [165] Wini 2 partition 5  */
	i214MINOR(1,2,0,6),             /* [166] Wini 2 partition 6  */
	i214MINOR(1,2,0,7),             /* [167] Wini 2 partition 7  */
	i214MINOR(1,2,0,8),             /* [168] Wini 2 partition 8  */
	i214MINOR(1,2,0,9),             /* [169] Wini 2 partition 9  */
	i214MINOR(1,2,0,10),            /* [170] Wini 2 partition 10 */
	i214MINOR(1,2,0,11),            /* [171] Wini 2 partition 11 */
	i214MINOR(1,2,0,12),            /* [172] Wini 2 partition 12 */
	i214MINOR(1,2,0,13),            /* [173] Wini 2 partition 13 */
	i214MINOR(1,2,0,14),            /* [174] Wini 2 partition 14 */
	i214MINOR(1,2,0,15),            /* [175] Wini 2 partition 15 */
/* Board 1 - Wini 3 */
	i214MINOR(1,3,0,0),             /* [176] Wini 3 partition 0  */
	i214MINOR(1,3,0,1),             /* [177] Wini 3 partition 1  */
	i214MINOR(1,3,0,2),             /* [178] Wini 3 partition 2  */
	i214MINOR(1,3,0,3),             /* [179] Wini 3 partition 3  */
	i214MINOR(1,3,0,4),             /* [180] Wini 3 partition 4  */
	i214MINOR(1,3,0,5),             /* [181] Wini 3 partition 5  */
	i214MINOR(1,3,0,6),             /* [182] Wini 3 partition 6  */
	i214MINOR(1,3,0,7),             /* [183] Wini 3 partition 7  */
	i214MINOR(1,3,0,8),             /* [184] Wini 3 partition 8  */
	i214MINOR(1,3,0,9),             /* [185] Wini 3 partition 9  */
	i214MINOR(1,3,0,10),            /* [186] Wini 3 partition 10 */
	i214MINOR(1,3,0,11),            /* [187] Wini 3 partition 11 */
	i214MINOR(1,3,0,12),            /* [188] Wini 3 partition 12 */
	i214MINOR(1,3,0,13),            /* [189] Wini 3 partition 13 */
	i214MINOR(1,3,0,14),            /* [190] Wini 3 partition 14 */
	i214MINOR(1,3,0,15),            /* [191] Wini 3 partition 15 */
/* Board 1 - Floppy 0 */
	i214MINOR(1,4,0,0),             /* [192]  80 Kbytes - f05s16t  */
	i214MINOR(1,4,2,10),            /* [193] 320 Kbytes - f05d16t  */
	i214MINOR(1,4,2,12),            /* [194] 320 Kbytes - f05d16   */
	i214MINOR(1,4,3,15),            /* [195] 320 Kbytes - f05d8t   */
	i214MINOR(1,4,3,16),            /* [196] 320 Kbytes - f05d8u   */
	i214MINOR(1,4,3,17),            /* [197] 320 Kbytes - f05d8    */
	i214MINOR(1,4,4,20),            /* [198] 320 Kbytes - f05d4t   */
	i214MINOR(1,4,4,22),            /* [199] 320 Kbytes - f05d4    */
	i214MINOR(1,4,5,25),            /* [200] 360 Kbytes - f05d9t   */
	i214MINOR(1,4,5,27),            /* [201] 360 Kbytes - f05d9    */
	i214MINOR(1,4,6,30),            /* [202] 720 Kbytes - f05qt    */
	i214MINOR(1,4,6,32),            /* [203] 720 Kbytes - f05q     */
	i214MINOR(1,4,7,35),            /* [204] 1.2 Mbytes - f05ht    */
	i214MINOR(1,4,7,37),            /* [205] 1.2 Mbytes - f05h     */
	i214MINOR(1,4,7,38),            /* [206] 1.2 Mbytes - f05hb    */
/* Board 1 - Floppy 1 */
	i214MINOR(1,5,0,0),             /* [207]  80 Kbytes - f05s16t  */
	i214MINOR(1,5,2,10),            /* [208] 320 Kbytes - f05d16t  */
	i214MINOR(1,5,2,12),            /* [209] 320 Kbytes - f05d16   */
	i214MINOR(1,5,3,15),            /* [210] 320 Kbytes - f05d8t   */
	i214MINOR(1,5,3,16),            /* [211] 320 Kbytes - f05d8u   */
	i214MINOR(1,5,3,17),            /* [212] 320 Kbytes - f05d8    */
	i214MINOR(1,5,4,20),            /* [213] 320 Kbytes - f05d4t   */
	i214MINOR(1,5,4,22),            /* [214] 320 Kbytes - f05d4    */
	i214MINOR(1,5,5,25),            /* [215] 360 Kbytes - f05d9t   */
	i214MINOR(1,5,5,27),            /* [216] 360 Kbytes - f05d9    */
	i214MINOR(1,5,6,30),            /* [217] 720 Kbytes - f05qt    */
	i214MINOR(1,5,6,32),            /* [218] 720 Kbytes - f05q     */
	i214MINOR(1,5,7,35),            /* [219] 1.2 Mbytes - f05ht    */
	i214MINOR(1,5,7,37),            /* [220] 1.2 Mbytes - f05h     */
	i214MINOR(1,5,7,38),            /* [221] 1.2 Mbytes - f05hb    */
/* Board 1 - Floppy 2 */
	i214MINOR(1,6,0,0),             /* [222]  80 Kbytes - f05s16t  */
	i214MINOR(1,6,2,10),            /* [223] 320 Kbytes - f05d16t  */
	i214MINOR(1,6,2,12),            /* [224] 320 Kbytes - f05d16   */
	i214MINOR(1,6,3,15),            /* [225] 320 Kbytes - f05d8t   */
	i214MINOR(1,6,3,16),            /* [226] 320 Kbytes - f05d8u   */
	i214MINOR(1,6,3,17),            /* [227] 320 Kbytes - f05d8    */
	i214MINOR(1,6,4,20),            /* [228] 320 Kbytes - f05d4t   */
	i214MINOR(1,6,4,22),            /* [229] 320 Kbytes - f05d4    */
	i214MINOR(1,6,5,25),            /* [230] 360 Kbytes - f05d9t   */
	i214MINOR(1,6,5,27),            /* [231] 360 Kbytes - f05d9    */
	i214MINOR(1,6,6,30),            /* [232] 720 Kbytes - f05qt    */
	i214MINOR(1,6,6,32),            /* [233] 720 Kbytes - f05q     */
	i214MINOR(1,6,7,35),            /* [234] 1.2 Mbytes - f05ht    */
	i214MINOR(1,6,7,37),            /* [235] 1.2 Mbytes - f05h     */
	i214MINOR(1,6,7,38),            /* [236] 1.2 Mbytes - f05hb    */
/* Board 1 - Floppy 3 */
	i214MINOR(1,7,0,0),             /* [237]  80 Kbytes - f05s16t  */
	i214MINOR(1,7,2,10),            /* [238] 320 Kbytes - f05d16t  */
	i214MINOR(1,7,2,12),            /* [239] 320 Kbytes - f05d16   */
	i214MINOR(1,7,3,15),            /* [240] 320 Kbytes - f05d8t   */
	i214MINOR(1,7,3,16),            /* [241] 320 Kbytes - f05d8u   */
	i214MINOR(1,7,3,17),            /* [242] 320 Kbytes - f05d8    */
	i214MINOR(1,7,4,20),            /* [243] 320 Kbytes - f05d4t   */
	i214MINOR(1,7,4,22),            /* [244] 320 Kbytes - f05d4    */
	i214MINOR(1,7,5,25),            /* [245] 360 Kbytes - f05d9t   */
	i214MINOR(1,7,5,27),            /* [246] 360 Kbytes - f05d9    */
	i214MINOR(1,7,6,30),            /* [247] 720 Kbytes - f05qt    */
	i214MINOR(1,7,6,32),            /* [248] 720 Kbytes - f05q     */
	i214MINOR(1,7,7,35),            /* [249] 1.2 Mbytes - f05ht    */
	i214MINOR(1,7,7,37),            /* [250] 1.2 Mbytes - f05h     */
	i214MINOR(1,7,7,38),            /* [251] 1.2 Mbytes - f05hb    */
/* Board 1 - Tape 0 */
	i214MINOR(1,8,0,0),             /* [252]  smt0 st rewind   */
	i214MINOR(1,8,1,0),             /* [253]  smnt0 st no-rew  */
/* Board 2 - Tape 1 */
	i214MINOR(1,9,0,0),             /* [254]  smt1 st rewind   */
	i214MINOR(1,9,1,0),             /* [255]  smnt1 st no-rew  */
};


/*
 *	Number of retries in case of soft error (configurable).
 */
int i214retry = 3;

/*
 * The following are static initialization variables
 * which are based on the configuration. These variables
 * MUST NOT CHANGE because the i214 device driver makes
 * most of the calculations based on these variables.
 */
#define	NUM214	((sizeof i214cfg) / (sizeof (struct i214cfg)))
int    i214_cnt =   NUM214;			/* number of configured boards   */
struct iobuf    i214tab[NUM214];	/* buffer headers per board      */
struct iobuf    i214tbuf[NUM214];       /* I015 tape buffer hdrs per board */
struct i214dev  *i214bdd[NUM214];	/* board-idx -> "dev" map        */
struct i214winidata i214winidata[NUM214][FIRSTFLOPPY]; /* per-wini misc data */
short  i214maxmin = ((sizeof i214minor)/(sizeof (struct i214minor)));
					/* maximum minor number posible. */

#ifndef TAPEBOOT
/*
 * READ-AHEAD AND WRITE-BEHIND TAPE BUFFERING:
 *
 * The default (for an installed system) is no tape buffering by the driver.
 * The tape read/write requests are handled synchronously, and errors on i/o
 * operations (like end-of-tape) are reported back to the requesting process.
 *
 * For better performance, you may want to use read-ahead and write-behind
 * tape buffering/caching by the driver; For tape buffering set i214TMEM to
 * the number of 32K buffers you would like to have (typically 8).
 */
#define	i214TMEM	0		/* Default: No tape buffering */
#else
/*
 * For the initial installation, we use tape buffering.
 */
#define	i214TMEM	8		/* The number of external buffers */
#endif

int i214tbuf_max = i214TMEM;		/* The maximum number to allocate */
struct buf i214tmem[i214TMEM+1];	/* external buffer structures */
ulong i214dma_limit = (16<<20)-(1<<19);	/* Use direct DMA if end < limit */
