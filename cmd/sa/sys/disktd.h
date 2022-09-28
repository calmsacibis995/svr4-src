/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sa:sys/disktd.h	1.2.3.1"

/* EMACS_MODES: c, width=80, fillcol=80 */

/* Mode sense data defines */
#define MODESZ 4			/* Length of Mode Sense data area */
#define MS_WP 0X80			/* Write protect bit in byte 2 */

/* Job data structure for each active job */
struct job{
	struct job *j_forw;		/* Next job in the work queue */
	struct job *j_back;		/* Pervious job in the work queue */
	struct sb *j_cont;		/* SCB for this job */
	struct job *j_mate;		/* Other job when duplexed write */
	void (*j_done)();		/* Function to call when done */
	pdi_buf_t *j_bp;		/* Pointer to buffer header */
	struct disk *j_dk;		/* Physical device to be accessed */
	union sc{			/* SCSI command block */
		struct scs cs;
		struct scm cm;
	} j_cmd;		
};

/* The disk structure hold the job queue for each disk, the VTOC, the
 * pdsector and the Request Sense data for the last erorr. */
struct disk{
	struct job *dk_forw;		/* 1st entry on work queue */
	struct job *dk_back;		/* Last entry on work queue */
	struct job *dk_next;		/* Next entry for HAD */
	struct job *dk_batch;		/* Elevator batch pointer */
	long dk_state;			/* State of this disk */
	long dk_count;			/* Number of jobs on work que */
	long dk_outcnt;			/* Jobs in HAD for this disk */
	long dk_error;			/* Number of errors detected */
	long dk_jberr;			/* Errors for this job */
	long dk_sendid;			/* Timeout id for sd01send */
	long dk_part_flag[V_NUMPAR];	/* Status of the parititions */
	time_t dk_start;		/* When the disk became active */
	struct scsi_ad dk_addr;		/* Major/Minor number of device */
	struct vtoc dk_vtoc;		/* VTOC for this disk */
	struct pdinfo dk_pdsec;		/* PD sector for this disk */
	struct sb *dk_sbrs;		/* SCSI block for request sense */
	struct sb *dk_sbr;		/* SCSI block for resume */
	long dk_spcount;		/* Retry count for special jobs */
	struct scs dk_rscmd;		/* Request sense command */
	struct sense dk_sense;		/* Reqest sense data */
	char dk_ms_data[MODESZ];	/* Mode sense/select data */
	long fills[5];			/* NOTE: THIS IS ADDED HERE IN R2  */
					/* so that 3B2 SAR doesn't have to */
					/* be re-released in scsi R3.	   */
					/* when the disk structure grows   */
					/*  FOR R3 THE FILLS MUST be       */
					/* adjusted so that the offset to  */
					/* dk_stat remains the same	   */
	pdi_iotime_t dk_stat;		/* Performance data */
};

/* State flags for the disk */
#define DKSUSP 0X0001			/* The HAD as susupended the que */
#define DKDRAIN 0X0002			/* The work que has filled up */
#define DKSEND 0X004			/* sd01send has requested timeout */
#define DKVTOC 0X0008			/* The VTOC has been read in. */
#define DKINIT 0X0010			/* The disk has been initialized. */
#define DKDIR 0X0020			/* Direction flag for elevator */
#define DKEL_OFF 0X0040			/* Elevator off flag */
#define DKUP_VTOC 0X0080		/* VTOC needs to be updated */

/* State values for each partition  */
#define DKFREE 0			/* The partition is not in use */
#define DKONLY 1			/* Part is open for exclusive */
#define DKGEN 2				/* Part is open for general use */


struct free_jobs{			/* List of free job structures  */
	int fj_state;			/* -1 if waiting for jobs */
	int fj_count;			/* Number free jobs */
	struct job *fj_ptr;		/* Pointer to free list */
};
/* fj_state values */
#define FJEMPTY -1			/* The free list was empty */
#define FJOK 0			/* The free list is ok */
#define FJHIGH 8			/* The high water mark */

/* Error codes for disk log records */
#define SFB_ERR 1			/* Function request error */
#define DR_ERR 2			/* Internal driver error */
#define HA_ERR 3			/* Host Adapter error */
#define IO_ERR 4			/* Read/Write error */

/* DR_ERR extended error codes */
#define TYPE_ERR 1			/* Bad type field was detected */
#define JOB_ERR 2			/* Bad job pointer was detected */

extern struct job Sd01jobarray[];	/* Array of job structures */
extern struct disk Sd01_d[];		/* Array of disk structures */
extern int Sd01_diskcnt;		/* Number of disks defined */
extern int Sd01jcnt;			/* Number of allocated jobs */
extern int Sd01imajor;			/* Internal major number */
extern  int Sd01log_marg;		/* Flag to log marginal bad blocks */

/* minor macro for this disk driver */
#define sd01minor(x) ((pdi_iminor(pdi_emajor(x))<<2)+(0X3&(pdi_eminor(x)>>4)))
#define sd01intmin(x,y) ((pdi_iminor(x)<<2) + (0X03&(y>>4)))
#define LUN(x) ((x>>4)&0X03) /* Get logical unit number */
#define LUCNT 4				/* Max number of lu's per tc */

#define PARTMASK 0X0F		       /* Mask for partition number */
#define BLKSZ 512		       /* Physical block size */
#define BLKSHF 9		       /* Shifts for a block size */
#define PDBLKNO 0		       /* Pd sector address */
#define JTIME 2000		       /* Two seconds for a job */
#define LATER 20		       /* How much later when retring */
#define SD01_RETRYCNT 2		       /* Retry count */
#define SD01_RST_ERR (SD01_RETRYCNT-1) /* Reset error retry count */

#define SD01_MAXSIZE 0x20000		/* Maximum job size */
#define SD01_QUEHIGH 50			/* Work que high water mark */
#define SD01_QUELOW 16			/* Work que low water mark */
#define SD01_OUTSZ 2			/* Min number of job maintained in
					 * the HAD for this device */

#define SD01_PBLK 16			/* Disk address is physical */

#define SD01_KERNEL 0			/* The buffer is in kerenl space */
#define SD01_USER 1			/* The buffer is in user space */
#define V_ELEV (VIOC | 6)
