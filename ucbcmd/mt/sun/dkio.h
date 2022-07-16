/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbmt:sun/dkio.h	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#ifndef _DKIO_
#define _DKIO_

#include "dklabel.h"

/*
 * Structures and definitions for disk io control commands
 */

/*
 * Structures used as data by ioctl calls.
 */

/*
 * Used for controller info
 */
struct dk_info {
	int	dki_ctlr;		/* controller address */
	short	dki_unit;		/* unit (slave) address */
	short	dki_ctype;		/* controller type */
	short	dki_flags;		/* flags */
};

#define	DK_DEVLEN	16		/* device name max length, including */
					/* unit # & NULL (ie - "xyc1") */
/*
 * Used for configuration info
 */
struct dk_conf {
	char	dkc_cname[DK_DEVLEN];	/* controller name (no unit #) */
	u_short	dkc_ctype;		/* controller type */
	u_short	dkc_flags;		/* flags */
	short	dkc_cnum;		/* controller number */
	int	dkc_addr;		/* controller address */
	u_int	dkc_space;		/* controller bus type */
	int	dkc_prio;		/* interrupt priority */
	int	dkc_vec;		/* interrupt vector */
	char	dkc_dname[DK_DEVLEN];	/* drive name (no unit #) */
	short	dkc_unit;		/* unit number */
	short	dkc_slave;		/* slave number */
};

/*
 * Controller types
 */
#define	DKC_UNKNOWN	0
/* 1 used to be Interphase 2180 */
#define	DKC_WDC2880	2
/* 3 used to be Interphase 2181 */
/* 4 used to be Xylogics 440 */
#define	DKC_DSD5215	5
#define	DKC_XY450	6
#define	DKC_ACB4000	7
#define DKC_MD21	8
/* 9 used to be Xylogics 751 */
#define	DKC_XB1401	10
#define	DKC_XD7053	11

/*
 * Flags
 */
#define	DKI_BAD144	0x01	/* use DEC std 144 bad sector fwding */
#define	DKI_MAPTRK	0x02	/* controller does track mapping */
#define	DKI_FMTTRK	0x04	/* formats only full track at a time */
#define	DKI_FMTVOL	0x08	/* formats only full volume at a time */

/*
 * Used for drive info
 */
struct dk_type {
	u_short dkt_hsect;		/* hard sector count (read only) */
	u_short dkt_promrev;		/* prom revision (read only) */
	u_char	dkt_drtype;		/* drive type (ctlr specific) */
	u_char	dkt_drstat;		/* drive status (ctlr specific, ro) */
};

/*
 * Used for all partitions
 */
struct dk_allmap {
	struct dk_map	dka_map[NDKMAP];
};

/*
 * Used for bad sector map
 */
struct dk_badmap {
	caddr_t dkb_bufaddr;		/* address of user's map buffer */
};

/*
 * Definition of a disk's geometry
 */
struct dk_geom {
	unsigned short	dkg_ncyl;	/* # of data cylinders */
	unsigned short	dkg_acyl;	/* # of alternate cylinders */
	unsigned short	dkg_bcyl;	/* cyl offset (for fixed head area) */
	unsigned short	dkg_nhead;	/* # of heads */
	unsigned short	dkg_obs1;	/* obsolete */
	unsigned short	dkg_nsect;	/* # of data sectors per track */
	unsigned short	dkg_intrlv;	/* interleave factor */
	unsigned short	dkg_obs2;	/* obsolete */
	unsigned short	dkg_obs3;	/* obsolete */
	unsigned short	dkg_apc;	/* alternates per cyl (SCSI only) */
	unsigned short	dkg_rpm;	/* revolutions per minute */
	unsigned short	dkg_pcyl;	/* # of physical cylinders */
	unsigned short	dkg_extra[7];	/* for compatible expansion */
};
/*
 * These defines are for historic compatibility with old drivers.
 */
#define	dkg_bhead	dkg_obs1	/* used to be head offset */
#define	dkg_gap1	dkg_obs2	/* used to be gap1 */
#define	dkg_gap2	dkg_obs3	/* used to be gap2 */

/*
 * Used for generic commands
 */
struct dk_cmd {
	u_short	dkc_cmd;		/* command to be executed */
	int	dkc_flags;		/* execution flags */
	daddr_t	dkc_blkno;		/* disk address for command */
	int	dkc_secnt;		/* sector count for command */
	caddr_t	dkc_bufaddr;		/* user's buffer address */
	u_int	dkc_buflen;		/* size of user's buffer */
};

/*
 * Execution flags.
 */
#define	DK_SILENT	0x01		/* no error messages */
#define	DK_DIAGNOSE	0x02		/* fail if any error occurs */
#define	DK_ISOLATE	0x04		/* isolate from normal commands */

/*
 * Used for disk diagnostics
 */
struct dk_diag {
	u_short	dkd_errcmd;		/* most recent command in error */
	daddr_t	dkd_errsect;		/* most recent sector in error */
	u_char	dkd_errno;		/* most recent error number */
	u_char	dkd_severe;		/* severity of most recent error */
};

/*
 * Severity values
 */
#define	DK_NOERROR	0
#define	DK_CORRECTED	1
#define	DK_RECOVERED	2
#define	DK_FATAL	3

/*
 * Error types
 */
#define	DK_NONMEDIA	0		/* not caused by a media defect */
#define	DK_ISMEDIA	1		/* caused by a media defect */

/*
 * Disk io control commands
 */
#define DIOC		('d'<<8)
#define	DKIOCGGEOM	(DIOC|2)			/* Get geometry */
#define	DKIOCSGEOM	(DIOC|3)			/* Set geometry */
#define	DKIOCGPART	(DIOC|4)			/* Get partition info */
#define	DKIOCSPART	(DIOC|5)			/* Set partition info */
#define	DKIOCINFO	(DIOC|8)			/* Get info */
#define	DKIOCGCONF	(DIOC|126)			/* Get conf info */
#define DKIOCSTYPE	(DIOC|125)			/* Set drive info */
#define DKIOCGTYPE	(DIOC|124)			/* Get drive info */
#define DKIOCSAPART	(DIOC|123)			/* Set all partitions */
#define DKIOCGAPART	(DIOC|122)			/* Get all partitions */
#define DKIOCSBAD	(DIOC|121)			/* Set bad sector map */
#define DKIOCGBAD	(DIOC|120)			/* Get bad sector map */
#define	DKIOCSCMD	(DIOC|119)			/* Set generic cmd */
#define	DKIOCGDIAG	(DIOC|116)			/* Get diagnostics */

#endif !_DKIO_
