/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:sys/mirror.h	1.3"

#define MDMINOR   0xff
struct normpart{
	short np_state;			/* State of this partition */
	dev_t np_dev;			/* Device number of normal part */
	dev_t np_bdev;			/* Device number of normal part */
        int np_mirminor;
        int np_count;
};
/* Normal partition state values */
#define NP_ACTIVE    0
#define NP_OOD       1			/* Out-of-date */
#define NP_UNDEFINED 2

extern struct md_info    {
        int             (*md_wrtimestamp)(); /* Addr of wrtimestamp function */
        int             (*md_freejob)();     /* Addr of freejob routine */
        int             (*md_strat1)();      /* Addr of strat1 routine */
        int             (*md_szsplit)();     /* Addr of szsplit routine */
        struct job      *(*md_getjob)();     /* Addr of getjob routine */
        int             (*md_ioctl)();       /* Addr of ioctl routine */
        int             (*md_batch)();       /* Addr of batch routine */
        void            (*md_strategy)();    /* Addr of strategy routine */
        int             (*md_close)();       /* Addr of close routine */
        int             (*md_open1)();       /* Addr of open1 routine */
        struct disk     *(*md_addr)();       /* Return address of disk struct */
        int             (*md_slice)();       /* Return slice */
        int             *md_number_of_maj;   /* # of maj numbers supported */
        struct drv_majors *md_maj_ptr;       /* Addr of maj number table */
};

struct pmirror{
	struct normpart pm_n[2];
	struct disk	*pm_dk[2];	/* Pointer to disk structure */
	struct md_info	*pm_info[2];	/* Pointer to md_info structure */
	long		pm_flags;	/* Flages for mirrored state */
	daddr_t		pm_lowwin;	/* Lowest addr in restore window */
	daddr_t		pm_highwin;	/* Highest addr in restore window */
	long		pm_size;	/* Size of partition */
	long		pm_jobcnt;	/* Count of outstanding jobs */
};

/* pm_flag values */
#define PM_BUSY    0X02			/* Busy updating  time stamps */
#define PM_SETWR   0X04			/* Update timestamon on write */
#define PM_RESTORE 0X08			/* Restore is in progress */
#define	PM_ALTRESTORE 0X80		/* Alternate Restore enabled*/
#define PM_FAILRS  0X10			/* Fail the restore */
#define PM_1MRU    0X20			/* Part one was most recently used */
#define PM_SETTS   0X40			/* Update time stamp on active */
#define PM_MNT    0X100			/* Part opened for Mounted FS */
#define PM_SWP    0X200			/* Part opened for Swapping Device */
#define PM_BLK    0X400			/* Part opened Buffered I/O */
#define PM_CHR    0X800			/* Part opened for Char I/O */
#define PM_LYR  0X10000			/* Increment/Decrement layer open */
#define PM_INUSE 0X200000		/* Entries in the mirrortable inuse */ 
/* The upper 16 bits are reserved for Driver opens */
/* See matching set of define's in disktd.h        */

/* Restore data structure */
struct resctl{
	long re_size;			/* Size of buffer in words */
	long re_status;			/* Return status of restore */
};
/* Restore status values */
#define RE_ASW     1			/* Restore succeeded */
#define RE_BADST   2			/* Partition is in wrong state */
#define RE_RDDES   3			/* Read of destination failed */
#define RE_WRDES   4			/* Write of destination failed */
#define RE_RDSRC   5			/* Read of source failed */
#define RE_WRSRC   6 			/* Write of source failed */
#define RE_NOSPACE 7			/* No space for buffer */
#define RE_BREAK   8			/* User requested break */
#define RE_BUSY    9			/* Restore is already being done */

/* Verify data structure */
struct verctl {
	char	*ve_buf1;		/* Partition 1 buffer pointer */
	char	*ve_buf2;		/* Partition 2 buffer pointer */
	long	ve_cnt;			/* Number of blocks to read into buffer */
	daddr_t	ve_daddr;		/* Partition disk address */
};

/* Mirror data structure */
struct mirror{
	long		mi_cmd[2];	/* Command for partitions */
	struct normpart mi_part[2];	/* Device info for partitions */
	long		mi_size;	/* Size of the mirror partition */
};

/* Mirror command codes */
#define MI_NOP   0X00
#define MI_ACT   0X01			/* Make partition active */
#define MI_UNDEF 0X02			/* Make partition undefined */
#define MI_OOD   0X03			/* Make partition Out-of-date */

#define MD_BACT(x) (x->pm_n[0].np_state == 0 && x->pm_n[1].np_state == 0)
#define MD_OTHER(x) ( x ? 0 : 1)

extern struct job *sd01getjob();

/* Ioctl commands */
#define MIRR		('M'<<8)
#define M_RESTORE	(MIRR | 1)		/* Restore command */
#define M_MIRROR	(MIRR | 2)		/* Mirror command */
#define M_VERIFY	(MIRR | 3)		/* Verify command */
#define M_ALTRESTORE	(MIRR | 4)		/* Alternate Restore command */
#define M_MIR_CHECK	(MIRR | 5)		/* Mirror check command */
#define M_GETROOTDEV	(MIRR | 6)		/* ioctl to get mirrordev[] */
#define M_GETSWAPDEV	(MIRR | 7)		/* ioctl to get mdswapdev[] */

/* Arugments to mirror driver ioctl */
union Arg {
	struct resctl *a_rest;  /* calls the restore function for mirroring */
	struct mirror *a_mirr;  /* calls the mirror function in mirror driver */
	struct verctl *a_ver;  /* calls the mirror verify function in driver */
	long	      *a_altr; /* calls the alternate restore in mirror driver */
        struct md_info *a_mircheck; /* calls the mdmircheck function in mirror
                                       driver */
        dev_t          *a_dev; /* ioctl to get mirrordev arguments */
};

struct Mir_min_t {
	struct pmirror  *addr;
} Mir_min_t[256];

extern struct pmirror Md_p[];
extern long Md_partcnt;
extern struct drv_majors *md_maj_ptr, *Sd01_maj_ptr[];
extern dev_t rootdev, mirrordev[], mdswapdev[];
extern struct md_info *MD_DRIVERS[];

/*
*  Mirror Table
*/
#define		MIRROR_KEY	"/etc/scsi/mirror"
#define		MIRROR_TABLE	"/etc/scsi/mirrortab"
#define		MT_OWNER	"root"
#define		FILEMODE	00644		/* rw-r--r-- */

/*
*  Mirror Table Entry
*/
#define		PARTITION_LEN	80
struct	MTABLE	{
	long	mt_size;		/* Partition size        */
	struct	DPART	{
		char	dp_name[PARTITION_LEN];	/* Disk Partition Name   */
		dev_t	dp_dev;			/* Partition dev value */
		dev_t	dp_bdev;		/* Partition Block dev value */
                minor_t mirmin;                 /* Assigned minor nimber */
	} mt_dpart[2];
	time_t	mt_date;		/* Mirror Table entry time stamp */
};
