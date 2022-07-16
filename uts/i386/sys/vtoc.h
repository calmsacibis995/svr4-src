/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_VTOC_H
#define _SYS_VTOC_H

#ident	"@(#)head.sys:sys/vtoc.h	11.4.7.1"
/*
 * VTOC.H
 */

#define V_NUMPAR 		16		/* The number of partitions */
#define VTOC_SEC       	29      /* VTOC sector number on disk */

#define VTOC_SANE		0x600DDEEE	/* Indicates a sane VTOC */
#define V_VERSION		0x01		/* layout version number */

/* Partition identification tags */
#define V_BOOT		0x01		/* Boot slice */
#define V_ROOT		0x02		/* Root filesystem */
#define V_SWAP		0x03		/* Swap filesystem */
#define V_USR		0x04		/* Usr filesystem */
#define V_BACKUP	0x05		/* full disk */
#define V_ALTS          0x06            /* alternate sector space */
#define V_OTHER         0x07            /* non-unix space */
#define V_ALTTRK	0x08		/* alternate track space */
#define V_STAND		0x09		/* Stand slice */
#define V_VAR		0x0a		/* Var slice */
#define V_HOME		0x0b		/* Home slice */
#define V_DUMP		0x0c		/* dump slice */

/* Partition permission flags */
#define V_UNMNT		0x01		/* Unmountable partition */
#define V_RONLY		0x10		/* Read only */
#define V_OPEN          0x100           /* Partition open (for driver use) */
#define V_VALID         0x200           /* Partition is valid to use */
#define V_VOMASK        0x300           /* mask for open and valid */

/* driver ioctl() commands */
#define VIOC		('V'<<8)
#define V_CONFIG        (VIOC|1)        /* Configure Drive */
#define V_REMOUNT       (VIOC|2)        /* Remount Drive */
#define V_ADDBAD        (VIOC|3)        /* Add Bad Sector */
#define V_GETPARMS      (VIOC|4)        /* Get drive/partition parameters */
#define V_FORMAT        (VIOC|5)        /* Format track(s) */
#define	V_PDLOC		(VIOC|6)	/* Ask driver where pdinfo is on disk */
#define	V_GETERR	(VIOC|7)	/* Get last error */
#define V_EXERR		(VIOC|8)	/* Save extended errors */
#define V_NOEXERR	(VIOC|9)	/* Don't save extended errors (def) */
#define V_RDABS		(VIOC|10)	/* Read a sector at an absolute addr */
#define V_WRABS		(VIOC|11)	/* Write a sector to absolute addr */
#define V_VERIFY	(VIOC|12)	/* Read verify sector(s)           */
#define V_XFORMAT	(VIOC|13)	/* Selectively mark sectors as bad */

/* SCSI driver ioctl() commands */
#define V_PREAD		(VIOC|14)	/* Physical Read */
#define V_PWRITE	(VIOC|15)	/* Physical Write */
#define V_PDREAD	(VIOC|16)	/* Read of Physical Description Area */
#define V_PDWRITE	(VIOC|17)	/* Write of Physical Description Area */

/* SCSI ioctl() error return codes */
#define V_BADREAD		0x01
#define V_BADWRITE		0x02

/* Sanity word for the physical description area */
#define VALID_PD		0xCA5E600D

struct partition	{
	ushort p_tag;			/*ID tag of partition*/
	ushort p_flag;			/*permision flags*/
	daddr_t p_start;		/*start sector no of partition*/
	long p_size;			/*# of blocks in partition*/
};

struct vtoc {
	unsigned long v_sanity;			/*to verify vtoc sanity*/
	unsigned long v_version;		/*layout version*/
	char v_volume[8];			/*volume name*/
	ushort v_nparts;			/*number of partitions*/
	ushort  v_pad;                          /*pad for 286 compiler*/
	unsigned long v_reserved[10];		/*free space*/
	struct partition v_part[V_NUMPAR];	/*partition headers*/
	time_t timestamp[V_NUMPAR];		/* SCSI time stamp */
};

struct pdinfo	{
	unsigned long driveid;		/*identifies the device type*/
	unsigned long sanity;		/*verifies device sanity*/
	unsigned long version;		/*version number*/
	char serial[12];		/*serial number of the device*/
	unsigned long cyls;		/*number of cylinders per drive*/
	unsigned long tracks;		/*number tracks per cylinder*/
	unsigned long sectors;		/*number sectors per track*/
	unsigned long bytes;		/*number of bytes per sector*/
	unsigned long logicalst;	/*sector address of logical sector 0*/
	unsigned long errlogst;		/*sector address of error log area*/
	unsigned long errlogsz;		/*size in bytes of error log area*/
	unsigned long mfgst;		/*sector address of mfg. defect info*/
	unsigned long mfgsz;		/*size in bytes of mfg. defect info*/
	unsigned long defectst;		/*sector address of the defect map*/
	unsigned long defectsz;		/*size in bytes of defect map*/
	unsigned long relno;		/*number of relocation areas*/
	unsigned long relst;		/*sector address of relocation area*/
	unsigned long relsz;		/*size in sectors of relocation area*/
	unsigned long relnext;		/*address of next avail reloc sector*/
/* the previous items are left intact from AT&T's 3b2 pdinfo.  Following
   are added for the 80386 port */
	unsigned long vtoc_ptr;         /*byte offset of vtoc block*/
	unsigned short vtoc_len;        /*byte length of vtoc block*/
	unsigned short vtoc_pad;        /* pad for 16-bit machine alignment */
	unsigned long alt_ptr;          /*byte offset of alternates table*/
	unsigned short alt_len;         /*byte length of alternates table*/
};

union   io_arg {
	struct  {
		ushort  ncyl;           /* number of cylinders on drive */
		unsigned char nhead;    /* number of heads/cyl */
		unsigned char nsec;     /* number of sectors/track */
		ushort  secsiz;         /* number of bytes/sector */
		} ia_cd;                /* used for Configure Drive cmd */
	struct  {
		ushort  flags;          /* flags (see below) */
		daddr_t bad_sector;     /* absolute sector number */
		daddr_t new_sector;     /* RETURNED alternate sect assigned */
		} ia_abs;               /* used for Add Bad Sector cmd */
	struct  {
		ushort  start_trk;      /* first track # */
		ushort  num_trks;       /* number of tracks to format */
		ushort  intlv;          /* interleave factor */
		} ia_fmt;               /* used for Format Tracks cmd */
	struct	{
		ushort	start_trk;	/* first track	*/
		char    *intlv_tbl;	/* interleave table */
		} ia_xfmt;		/* used for the V_XFORMAT ioctl */

};

/*
 * Data structure for the V_VERIFY ioctl
 */
union	vfy_io	{
	struct	{
		daddr_t abs_sec;	/* absolute sector number        */
		ushort  num_sec;	/* number of sectors to verify   */
		ushort  time_flg;	/* flag to indicate time the operation */
		}vfy_in;
	struct	{
		time_t  deltatime;	/* duration of operation */
		ushort  err_code;	/* reason for failure    */
		}vfy_out;
};

/* Flags for Add Bad Sector command */
#define V_ABS_NEAR      1       /* Assign closest alternate available */


/* data structure returned by the Get Parameters ioctl: */

struct  disk_parms {
	char    dp_type;                /* Disk type (see below) */
	unchar  dp_heads;               /* Number of heads */
	ushort  dp_cyls;                /* Number of cylinders */
	unchar  dp_sectors;             /* Number of sectors/track */
	ushort  dp_secsiz;              /* Number of bytes/sector */
					/* for this partition: */
	ushort  dp_ptag;                /* Partition tag */
	ushort  dp_pflag;               /* Partition flag */
	daddr_t dp_pstartsec;           /* Starting absolute sector number */
	daddr_t dp_pnumsec;             /* Number of sectors */
	};

/* Disk types for disk_parms.dp_type: */
#define DPT_WINI        1               /* Winchester disk */
#define DPT_FLOPPY      2               /* Floppy */
#define DPT_OTHER       3               /* Other type of disk */
#define DPT_NOTDISK     0               /* Not a disk device */
#define DPT_SCSI_HD	4               /* SCSI hard disk device */
#define DPT_SCSI_OD	5               /* SCSI optical disk device */
#define DPT_SCSI_FD	6               /* SCSI floppy drive device */
#define DPT_ESDI_HD	0x11            /* ESDI hard disk device */

/* Data structure for V_RDABS/V_WRABS ioctl's */
struct absio {
	daddr_t	abs_sec;		/* Absolute sector number (from 0) */
	char	*abs_buf;		/* Sector buffer */
};

/* Data structure for SCSI physical read/write ioctl's */
struct phyio {
	int retval;			/* Return value			*/
	unsigned long sectst;		/* Sector address		*/
	unsigned long memaddr;		/* Buffer address		*/
	unsigned long datasz;		/* Transfer size in bytes	*/
};

#ifdef	IOCTL_ERROR
/* Errors which may be retrieved with an ioctl when IOCTL_ERROR is used.  */

/* Error message types */

#define	FD_NOARGS	0		/* No arguments are applicable */
#define	FD_TRKERR	1		/* Track number is applicable */
#define	FD_BLKERR	2		/* Block number is applicable */

#define	FD_ENOERROR	0		/* No error */
#define	FD_ECMDTIMEOUT	1		/* command timeout */
#define	FD_ESTATIMEOUT	2		/* status timeout */
#define	FD_EBUSY	3		/* busy */
#define	FD_EMISSDADDR	4		/* Missing data address mark */
#define	FD_EBADCYL	5		/* Cylinder marked bad */
#define	FD_EWRONGCYL	6		/* Seek error (wrong cylinder) */
#define	FD_ECANTREAD	7		/* Uncorrectable data read */
#define	FD_EBADSECTOR	8		/* Sector marked bad */
#define	FD_EMISSHADDR	9		/* Missing header address mark */
#define	FD_EWRITEPROT	10		/* Write protected */
#define	FD_ESECNOTFND	11		/* Sector not found */
#define	FD_EDATAOVRUN	12		/* Data overrun */
#define	FD_EHCANTREAD	13		/* Header read error */
#define	FD_ILLSECT	14		/* Illegal sector */
#define	FD_EDOOROPEN	15		/* Door open */

typedef	struct lasterr_t {
	char	number;
	char	type;
	union	{
		int	trk;
		int	blk;
	} arg1;
} lasterr_t;
#endif	/* IOCTL_ERROR */

#endif	/* _SYS_VTOC_H */
