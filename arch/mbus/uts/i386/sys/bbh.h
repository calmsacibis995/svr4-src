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

#ident	"@(#)mbus:uts/i386/sys/bbh.h	1.3"

#ifndef _SYS_BBH_H
#define _SYS_BBH_H

/* TEMPORARY HACK */
#define BSIZE 1024

/*
 * Some general purpose macros
 */

/*
 * Returns the location (absolute sector #) of the disk resident MDL.
 */
#define MDLLOC(heads,cyls,sectors) 	((((cyls-2)*(heads))+(heads-4))*sectors)

/*
 * Returns the default # of cylinders to reserve for
 * the alternate track pool in System V.3 and V.4.
 */
#define V4_NALTS(heads,cyls)	((((heads)*(cyls))/50)/heads)
#define V3_NALTS(heads,cyls)	(12)

#define NALTS(heads,cyls)	(V4_NALTS(heads,cyls))

/*
 * Returns the absolute sector # (0-based) of the hardware
 * alternate pool for System V.3 and V.4.  'START_ALT' 
 * This implies the # of tracks in the user portion of the disk
 * is: (ALTLOC/sectors)-1
 */
#define V4_ALTLOC(heads,cyls,sectors)					\
			sectors*(((cyls-3)*heads)-(heads*NALTS(heads,cyls)))

#define V3_ALTLOC(heads,cyls,sectors)	((cyls-15)*heads*sectors)

#define START_ALT(heads,cyls,sectors)	V4_ALTLOC(heads,cyls,sectors)/sectors

/*
 * General BBH constants.
 */
#define LOCKED		0x6969		/* Indicates a lock is set. */
#define UNLOCKED	~LOCKED

#define	STRUCTIO	0x9494		/* Flag to read/write wini data. */

#define	STATOK		0x0		/* Indicates no error in subroutine.*/
#define	STATBAD		~STATOK

/*
 * Additional IOCTL commands.  See 'vtoc.h'
 */
#define IIOC            ('I'<<8)
#define V_FMTPART	(IIOC|10)	/* Format a partition */

#define V_L_VLAB	(IIOC|11)	/* Load IVLAB from user */
#define V_U_VLAB	(IIOC|12)	/* Upload IVLAB to user */
#define V_R_VLAB	(IIOC|13)	/* Read IVLAB from disk */
#define V_W_VLAB	(IIOC|14)	/* Write IVLAB from disk */

#define V_L_PDIN	(IIOC|15)	/* Load PDINFO from user */
#define V_U_PDIN	(IIOC|16)	/* Upload PDINFO to user */
#define V_R_PDIN	(IIOC|17)	/* Read PDINFO from disk */
#define V_W_PDIN	(IIOC|18)	/* Write PDINFO from disk */

#define V_L_VTOC	(IIOC|19)	/* Load VTOC from user */
#define V_U_VTOC	(IIOC|20)	/* Upload VTOC to user */
#define V_R_VTOC	(IIOC|21)	/* Read VTOC from disk */
#define V_W_VTOC	(IIOC|22)	/* Write VTOC from disk */

#define V_L_SWALT	(IIOC|23)	/* Load SW ALT TAB from user */
#define V_U_SWALT	(IIOC|24)	/* Upload SW ALT TAB to user */
#define V_R_SWALT	(IIOC|25)	/* Read SW ALT TAB from disk */
#define V_W_SWALT	(IIOC|26)	/* Write SW ALT TAB from disk */

#define V_L_MDL		(IIOC|27)	/* Load MDL from user */
#define V_U_MDL		(IIOC|28)	/* Upload MDL to user */
#define V_R_MDL		(IIOC|29)	/* Read MDL from disk */
#define V_W_MDL		(IIOC|30)	/* Write MDL from disk */

#define V_FMTLOCK	(IIOC|31)	/* Lock device's wini data strucuture */
#define V_FMTUNLOCK	(IIOC|32)	/* Unlock device's wini data struct */

/*
 * Data structure used with V_FMTPART ioctl command.
 */
struct fmtpart {
	ushort  partnum;        /* Partition # */
	ushort  method;         /* Desire formatting method */
	ushort  intlv;          /* interleave factor */
	};		        /* used for Format Partition cmd */

/*
* Formating methods supported by the V_FMTPART IOCTL command.
*/
#define FMTMETHODS	5	/* Total number methods defined */
#define FMTANY		0x50	/* Use any method available */
#define FMTNORM		0x51	/* Fmt trk using normal method */
#define FMTALTTRK	0x52	/* Fmt defective trk using Alternate Tracking */
#define FMTSWALTS	0x53	/* Fmt defective trk using SW Alternates */
#define FMTFAIL		0x54	/* Last fmt method to try, all others failed */

/*
 * Define ST506 layout of the Manufacturer's Defect List (MDL).
 *
 * The ST506 MDL is written in next to last cylinder on 4 different
 * tracks using a bytes/sector value of: 128, 256, 512, and 1024.
 *
 * The track assignments (within the cylinder) are:
 *	 128 bytes/sec		Last track in cylinder.
 *	 256 bytes/sec		Last track - 1.
 *	 512 bytes/sec		Last track - 2.
 *	1024 bytes/sec		Last track - 3.
 *
 * Each track contains 4 copies the MDL starting at the beginning
 * of the track and spaced every 2K-bytes i.e the byte offset of 
 * each copy is: 0k-bytes, 2k-bytes, 4k-bytes and 8k-bytes.
 *	
 */
#define BBH506MDLVALID		0xABCD	/* Magic # to indicate a valid MDL. */
#define BBH506MAXDFCTS		255	/* Max # of ST506 defects. */
#define BBH506COPYCNT		4	/* # of MDL copies per track. */
#define BBH506COPYSZ		2*1024	/* # of bytes between MDL copies. */

struct st506hdr {			/* ST506 header information. */
	unsigned short	bb_valid;
	unsigned short	bb_num;
	};

struct st506defect {			/* ST506 individual defect info. */
	unsigned short	be_cyl;
	unsigned char	be_surface;
	unsigned char	be_reserved;
	};

struct st506mdl {			/* ST506 MDL */
	struct st506hdr		header;
	struct st506defect	defects[BBH506MAXDFCTS];
	};



/*
 * Define the ESDI layout of the Manufacturer's Defect List (MDL).
 *
 * The ESDI MDL is written in the last cylinder - 2 of the drive.
 * Each track of that cylinder contains the defect information
 * of the corresponding head/surface.  Each track contains 4 copies
 * the head's defect information.  The entire ESDI MDL consists of 
 * several head-defect data structures, each corresponding to a 
 * particular head of the drive.
 */
#define BBHESDIMDLVALID		0xC5DF	/* Magic # to indicate a valid MDL. */
#define BBHESDIMAXDFCTS		203	/* Max # of ESDI defects per head. */
#define	BBHESDIMAXHEADS		18	/* Max # of ESDI heads. */
#define BBHESDICOPYCNT		4	/* # of MDL copies per head. */ 
#define BBHESDICOPYSZ		2*1024	/* # of bytes between MDL copies. */

struct esdihdr {			/* ESDI header information. */
	unsigned short	magic;
	unsigned short	version;
	unsigned char	head;
	unsigned char	reserved;
	};

struct esdidefect {			/* ESDI individual defect infor. */
	unsigned char	hi_cyl;		/* Hi-order byte of cylinder #.	*/
	unsigned char	lo_cyl;		/* Lo-order byte of cylinder #.	*/
	unsigned char	hi_offset;	/* Hi-order byte of byte offset #. */
	unsigned char	lo_offset;	/* Lo-order byte of byte offset #. */
	unsigned char	length;		/* Error length in # of bits.	*/
	};

struct esdiheadmdl {			/* ESDI per head MDL. */
	struct esdihdr		header;
	struct esdidefect 	defects[BBHESDIMAXDFCTS];
	};


/*
 * Old style defines need to remain until all programs are
 * changed to use the new ones,  e.k. 'mkpart.c'.
 */
#define MDL_VALID	BBH506MDLVALID

#define	BBHVENDDFCTSZ	2*1024

/*
 * Union to hold both ST506 amd ESDI mdl.
 */
union esdi506mdl {
	struct st506mdl	st506mdl	;
	struct esdiheadmdl esdimdl[BBHESDIMAXHEADS]	;
};

/*
 * Ioctl flags for V_R_MDL.
 */
#define I_ESDI	0x1 /* Read ESDI  mdl. */
#define	I_ST506 0x2 /* Read ST506 mdl. */

#endif	/* _SYS_BBH_H */
