/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sa:sys/sd01.h	1.2"

/*
 * New sd01.h header file for SVR4.0 version i386
 */

/* 
 * Job data structure for each active job.
 */

struct job{
	struct	job	*j_forw;	/* Next job in the work queue 	*/
	struct	job	*j_back;	/* Pervious job in the work queue*/
	struct	sb	*j_cont;	/* SCB for this job 		*/
	struct	job	*j_mate;	/* Other job when duplexed write*/
	void		(*j_done)();	/* Function to call when done 	*/
	buf_t	*j_bp;			/* Pointer to buffer header	*/
	struct	disk	*j_dk;		/* Physical device to be accessed*/
	union	sc{			/* SCSI command block 		*/
		struct scs cs;
		struct scm cm;
	} j_cmd;		
};

/* 
 * Ported from pdi.h for SAR.
 */

#define	NTRACK		10	/* # of entries in drive's perf. queue 	*/

/* 
 * Performance data gathering queue.
 */

typedef struct	scsidf_ptrack
{
	long		b_blkno;	/* start disk block address 	*/
	buf_t		*bp;		/* pointer to user buffer header*/
} scsiptrk_t;

/*
 * Structures for system accounting.
 */

typedef struct scsi_iotime {
	struct iostat ios;	/* iostat sub structure 		*/
	long	io_bcnt;	/* total blocks transferred 		*/
	time_t	io_resp;	/* total block response time in ms 	*/
	time_t	io_act;		/* cumulative use in ms 		*/
	major_t	maj;		/* major number of device 		*/
	minor_t	min;		/* minor number of device 		*/
	long	tnrreq;		/* total number of read requests 	*/
	long	tnwreq;		/* total number of write requests 	*/
	long	cumqlen;	/* cumulative queue length 		*/
	long	maxqlen;	/* max queue length 			*/
	long	minqlen;	/* minimum queue length 		*/
	long	cumseekd;	/* cumulative seek distance 		*/
	time_t	io_liact;	/* time drive active for last interval in ms*/
	time_t	io_intv;	/* drive perf reporting interval in ms 	*/
	scsiptrk_t	*pttrack;	/* start of queue 		*/
	scsiptrk_t	*endptrack;	/* last entry in queue 		*/
	scsiptrk_t	ptrackq[NTRACK];	/* queue 		*/
} scsi_iotime_t;

/*
 * Define for Reassign Blocks defect list size.
 */

#define RABLKSSZ	8	/* Defect list in bytes		*/

/*
 * Define for Read Capacity data size.
 */

#define RDCAPSZ 	8	/* Length of data area		*/

/*
 * Defines for Mode sense data command.
 */

#define FPGSZ 		0x1C	/* Length of page 3 data area	*/
#define RPGSZ 		0x18	/* Length of page 4 data area	*/
#define	SENSE_PLH_SZ	4	/* Length of page header	*/

/*  
 * Define the Read Capacity Data Header format.
 */

typedef struct capacity {
	int cd_addr;		/* Logical Block Address	*/
	int cd_len;		/* Block Length			*/
} CAPACITY_T;

/*
 *  Define the Mode Sense Parameter List Header format.
 */

typedef struct sense_plh {
	unchar 	plh_len;	/* Data Length			*/
	unchar 	plh_type;	/* Medium Type			*/
	uint  	plh_res : 7;	/* Reserved			*/
	uint  	plh_wp : 1;	/* Write Protect		*/
	unchar 	plh_bdl;	/* Block Descriptor Length	*/
} SENSE_PLH_T;

/*  
 * Define the Direct Access Device Format Parameter Page format.
 */

typedef struct dadf {
	int pg_pc	: 6;	/* Page Code			*/
	int pg_res1	: 2;	/* Reserved			*/
	unchar pg_len;		/* Page Length			*/
	int pg_trk_z	: 16;	/* Tracks per Zone		*/
	int pg_asec_z	: 16;	/* Alternate Sectors per Zone	*/
	int pg_atrk_z	: 16;	/* Alternate Tracks per Zone	*/
	int pg_atrk_v	: 16;	/* Alternate Tracks per Volume	*/
	int pg_sec_t	: 16;	/* Sectors per Track		*/
	int pg_bytes_s	: 16;	/* Bytes per Physical Sector	*/
	int pg_intl	: 16;	/* Interleave Field		*/
	int pg_trkskew	: 16;	/* Track Skew Factor		*/
	int pg_cylskew	: 16;	/* Cylinder Skew Factor		*/
	int pg_res2	: 27;	/* Reserved			*/
	int pg_ins	: 1;	/* Inhibit Save			*/
	int pg_surf	: 1;	/* Allocate Surface Sectors	*/
	int pg_rmb	: 1;	/* Removable			*/
	int pg_hsec	: 1;	/* Hard Sector Formatting	*/
	int pg_ssec	: 1;	/* Soft Sector Formatting	*/
} DADF_T;

/*  
 * Define the Rigid Disk Drive Geometry Parameter Page format.
 */

typedef struct rddg {
	int pg_pc	: 6;	/* Page Code			 */
	int pg_res1	: 2;	/* Reserved			 */
	unchar pg_len;		/* Page Length			 */
	int pg_cylu	: 16;	/* Number of Cylinders (Upper)	 */
	unchar pg_cyll;		/* Number of Cylinders (Lower)	 */
	unchar pg_head;		/* Number of Heads		 */
	int pg_wrpcompu	: 16;	/* Write Precompensation (Upper) */
	unchar pg_wrpcompl;	/* Write Precompensation (Lower) */
	int pg_redwrcur	: 24;	/* Reduced Write Current	 */
	int pg_drstep	: 16;	/* Drive Step Rate		 */
	int pg_landu	: 16;	/* Landing Zone Cylinder (Upper) */
	unchar pg_landl;	/* Landing Zone Cylinder (Lower) */
	int pg_res2	: 24;	/* Reserved			 */
} RDDG_T;

/*
 * The disk structure holds the job queue for each disk, the VTOC, the
 * pdsector and the Request Sense data for the last erorr.
 */

struct disk {
	struct job *dk_forw;		/* 1st entry on work queue 	*/
	struct job *dk_back;		/* Last entry on work queue 	*/
	struct job *dk_next;		/* Next entry for HAD 		*/
	struct job *dk_batch;		/* Elevator batch pointer 	*/
	long hde_state;			/* State of hard disk errors	*/
	long dk_state;			/* State of this disk 		*/
	long dk_count;			/* Number of jobs on work que 	*/
	long dk_outcnt;			/* Jobs in HAD for this disk 	*/
	long dk_error;			/* Number of errors detected 	*/
	long dk_jberr;			/* Errors for this job 		*/
	long dk_sendid;			/* Timeout id for sd01send 	*/
	long dk_part_flag[V_NUMPAR];	/* Status of the parititions 	*/
	time_t dk_start;		/* When the disk became active 	*/
	struct scsi_ad dk_addr;		/* Major/Minor number of device */
	struct vtoc dk_vtoc;		/* VTOC for this disk 		*/
	struct pdinfo dk_pdsec;		/* PD sector for this disk 	*/
	struct disk_parms dk_parms;	/* Current drive configuration	*/
	daddr_t unixst;			/* First sector active partition*/
	struct sb *dk_fltreq;		/* SCSI block for request sense */
	struct sb *dk_fltres;		/* SCSI block for reserve job 	*/
	struct sb *dk_fltsus;		/* SCSI block for suspend job 	*/
	struct sb *dk_fltmblk;		/* SCB for reassigning blocks	*/
	struct sb *dk_fltwblk;		/* SCB for writing bad blocks   */
	struct sb *dk_fltrblk;		/* SCB for reading bad blocks   */
	struct disk *dk_fltnext;	/* Next disk in RESUME list 	*/
	long dk_rescnt;			/* Number of RESUME Bus Resets 	*/
	long dk_spcount;		/* Retry count for special jobs */
	struct scs dk_fltcmd;		/* Request Sense/Reserve command*/
	struct scs dk_blkcmd;		/* Reassign/Read/Write command	*/
	struct sense dk_sense;		/* Request Sense data 		*/
	char dk_rc_data[RDCAPSZ];	/* Read Capacity data	 	*/
	char dk_ms_data[FPGSZ];		/* Mode Sense or Select data 	*/
	char dk_dl_data[RABLKSSZ];	/* Defect List data 		*/
	char blkbuf[512];		/* Marginal bad block data	*/
	struct scsi_iotime dk_stat;	/* Performance data.SAR requires*/
					/* dk_stat to be at end of struct*/

};

/* State flags for the disk */
#define DKSUSP	    0X0001		/* The HAD susupended the que	*/
#define DKDRAIN	    0X0002		/* The work que has filled up	*/
#define DKSEND	    0X0004		/* sd01send has requested timeout*/
#define DKVTOC	    0X0008		/* The VTOC has been read in.	*/
#define DKINIT	    0X0010		/* The disk has been initialized.*/
#define DKDIR	    0X0020		/* Direction flag for elevator 	*/
#define DKEL_OFF    0X0040		/* Elevator off flag 		*/
#define DKUP_VTOC   0X0080		/* VTOC needs to be updated 	*/
#define DKTSMD      0X0100		/* Updating TS for mirrored part*/
#define	DKFLT       0X0200		/* Disk is recovering from a fault*/
#define	DKRESERVE   0X0400		/* Disk is currently reserved 	*/
#define	DKRESDEVICE 0X0800		/* Disk has been reserved 	*/
#define	DKONRESQ    0X1000		/* Disk on the Resume Queue 	*/
#define	DKPENDRES   0X2000		/* Disk has a pending Resume 	*/
#define DKCONFLICT  0X4000		/* Reservation Confict on Open 	*/
#define DKPARMS     0X10000		/* Drive parameters set & valid	*/
#define DKFDISK     0X20000		/* Fdisk table read and valid	*/

/* State flags for bad block handling */
#define HDERECERR   0X100000		/* Hit marginal bad block	*/
#define HDEECCERR   0X200000		/* Hit actual bad block		*/
#define HDEBADBLK   0X300000		/* Disk has a bad block to fix	*/
#define HDEMASK     0XF00000		/* Bad block type state mask 	*/
#define HDEENOSPR   0X010000		/* Error no spare sectors 	*/
#define HDESMASK    0X00FFFF		/* Bad block command state mask	*/
#define HDESINIT    0X000000		/* Initial state		*/
#define HDESI       0X000001		/* Read command failed		*/
#define HDESII      0X000002		/* Read command passed		*/
#define HDESIII     0X000003		/* Reassign command failed	*/
#define HDESIV      0X000004		/* Reassign command passed	*/
#define HDESV       0X000005		/* Write command failed		*/

/* Define indices for bad block messages */

#define HDEECCMSG  0			/* ECC corection needed		*/
#define HDESACRED  1			/* Marginal in sacred area	*/
#define HDENOSPAR  2			/* No spares for marginal block */
#define HDEBADMAP  3			/* Reassign failed on marginal 	*/
#define HDEMAPBLK  4			/* Alternate for marginal 	*/
#define HDEBADWRT  5			/* Write of data failed		*/
#define HDEBSACRD  6			/* Bad block in sacred area	*/
#define HDEREASGN  7			/* Alternate for bad block 	*/
#define HDEBADRED  8			/* Read of marginal failed	*/
#define HDEBNOSPR  9			/* No spares for bad block	*/
#define HDEBNOMAP  10			/* Reassign failed on bad block	*/
#define HDENOINIT  11			/* Write of zeros failed 	*/

/* State values for each partition  */
#define DKFREE 0			/* The partition is not in use 	*/
#define DKONLY 1			/* Part is open for exclusive 	*/
#define DKGEN  2			/* Part is open for general use */
#define DKMNT  0X100			/* Part opened for Mounted FS 	*/
#define DKSWP  0X200			/* Part opened for Swapping Device*/
#define DKBLK  0X400			/* Part opened Buffered I/O 	*/
#define DKCHR  0X800			/* Part opened for Char I/O 	*/
#define DKLYR  0X10000			/* Inc/dec the Driver open count*/
/* The upper 16 bits are reserved for Driver opens */
/* See matching set of define's in mirror.h        */

struct resume{
	struct disk *res_head;		/* Next disk to use the RESUME SB*/
	struct disk *res_tail;		/* Last disk to use the RESUME SB*/
};

struct free_jobs{			/* List of free job structures	*/
	int fj_state;			/* -1 if waiting for jobs 	*/
	int fj_count;			/* Number free jobs 		*/
	struct job *fj_ptr;		/* Pointer to free list 	*/
};

/* fj_state values. */
#define FJEMPTY -1			/* The free list was empty 	*/
#define FJOK 0				/* The free list is ok 		*/
#define FJHIGH 8			/* The high water mark 		*/

/* Error codes for disk log records. */
#define SFB_ERR 1			/* Function request error 	*/
#define DR_ERR  2			/* Internal driver error 	*/
#define HA_ERR  3			/* Host Adapter error 		*/
#define IO_ERR  4			/* Read/Write error 		*/

/* DR_ERR extended error codes. */
#define TYPE_ERR 1			/* Bad type field was detected 	*/
#define JOB_ERR  2			/* Bad job pointer was detected */

/* Space file declarations. */
extern struct tc_data Sd01_data[];	/* Array of tc device info	*/
extern struct drv_majors Sd01_majors[];	/* Array of major number info	*/
extern int Sd01_datasz;			/* Number of supported TC's    	*/
extern struct disk *Sd01_dp;		/* Array of disk structures 	*/
extern struct job *Sd01_jp;		/* Array of job structures 	*/
extern short  Sd01diskinfo;		/* Flag to set disk parameters	*/
extern int Sd01_jobs;			/* Number of job structures 	*/
extern int Sd01log_marg;		/* Flag to log marginal blocks	*/
extern int Sd01_cmajors;		/* Number of c major numbers	*/
extern char Sd01_debug[];		/* Array of debug levels	*/

extern int sd01_diskcnt;		/* Number of disks defined 	*/
extern int sd01_tccnt;			/* Number of controllers defined*/
extern int sd01_jobcnt;			/* Number of allocated jobs 	*/

/* Number of TC'c supported.	*/
#define	DATASZ	sizeof(Sd01_data)/sizeof(struct tc_data)

/* Minor number macros. */
#define DKSLICE(x)	(getminor(x) % V_NUMPAR)
#define DKINDEX(x)	(DKMJINS(x) + (getminor(x) / V_NUMPAR))
#define DKMJINS(x)	(sd01instbl[getmajor(x)] * 16)

#define DKNOTCS		-1		/* No TC's configured in system	*/
#define DKNOMAJ		-1		/* Unsupported major number	*/

#define BLKSZ		512		/* Physical block size 		*/
#define BLKSHF 		9		/* Shifts for a block size 	*/
#define PDBLKNO		29		/* PD sector address 		*/
#define FDBLKNO		0		/* fdisk table block address 	*/
#define VTBLKNO		PDBLKNO		/* VTOC sector address 		*/
#define JTIME		10000		/* Ten seconds for a job 	*/
#define LATER		20000		/* How much later when retrying */

#define SD01_RETRYCNT	2		/* Retry count 			*/
#define SD01_RST_ERR (SD01_RETRYCNT-1)  /* Reset error retry count 	*/
#define SD01_MAXSIZE	0xF000		/* Maximum job size 		*/
#define SD01_QUEHIGH	50		/* Work que high water mark 	*/
#define SD01_QUELOW	16		/* Work que low water mark 	*/
#define SD01_OUTSZ	2		/* Min number of job maintained in
					 * the HAD for this device 	*/

#define SD01_PBLK	16		/* Disk address is physical 	*/

#define SD01_KERNEL	0		/* The buffer is in kerenl space*/
#define SD01_USER	1		/* The buffer is in user space 	*/

#define SD01_DEBUGFLG	73		 /* Turn debugs on/off 		*/
