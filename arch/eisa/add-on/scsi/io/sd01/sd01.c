/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)eisa:add-on/scsi/io/sd01/sd01.c	1.1"

#include "sys/types.h"
#include "sys/vtoc.h"
#include "sys/fdisk.h"		/* Included for 386 disk layout		*/

#include "sys/param.h"
#include "sys/systm.h"  	/* Included for lbolt time      	*/

#include "sys/errno.h"
#include "sys/buf.h"		/* Included for dma routines		*/
#include "sys/bootinfo.h"	/* Included to check boot device	*/
#include "sys/elog.h"
#include "sys/open.h"
#include "sys/kmem.h"		/* Included for DDI kmem_alloc routines	*/

#include "sys/cred.h"		/* Included for	cred structure argument */
#include "sys/uio.h"		/* Included for	uio structure argument  */
#include "sys/ddi.h"	
#include "sys/cmn_err.h"

#include "sys/sdi.h"
#include "sys/scsi.h"
#include "sys/sdi_edt.h"
#include "sd01.h"
#include "sys/sd01_ioctl.h"

buf_t 	sd01_io_buf;			/* Buffer header for kernel r/w */
buf_t 	sd01_hde_buf;			/* Buffer header for BBH routine*/
struct 	job sd01_bbh_job;		/* Job struct for BBH routine	*/
struct 	free_jobs sd01freej;		/* Free job list header 	*/
struct 	sb *sd01_fltsbp;		/* SB exclusively for RESUMEs 	*/
struct 	resume sd01_resume;		/* Linked list of disks waiting */

char   	sd01name[49];			/* sd01flterr & sd01logerr dev info */
char 	sd01instbl[65];			/* Major number instance table	*/
dev_t 	sd01pt_dev;			/* and p_dev number. Declared	*/
					/* here to reduce int stack	*/
int 	sd01_diskcnt;			/* Number of disks configured	*/
int 	sd01_tccnt;			/* Number of controllers        */
int 	sd01_jobcnt;			/* Number of bytes in job struct*/

/* Function table of contents	*/
void		sd01init();		/* Init entry point  - 0dd01000	*/
struct	job	*sd01getjob();		/* Get job structure - 0dd02000	*/
void		sd01freejob();		/* Free job structure- 0dd03000	*/
void  		sd01send(); 		/* Sends jobs from Q - 0dd04000	*/
void  		sd01sendt(); 		/* Send via timeout  - 0dd05000	*/
void		sd01strat1();		/* Core strategy     - 0dd06000	*/
void		sd01strategy(); 	/* Strategy entry    - 0dd07000	*/
int		sd01close();		/* Close entry point - 0dd08000	*/
int		sd01read();		/* Read entry point  - 0dd09000	*/
int		sd01write();		/* write entry point - 0dd0a000	*/
int		sd01open1();		/* Core open function- 0dd0b000	*/
int		sd01open(); 		/* Open entry point  - 0dd0c000	*/
void  		sd01done(); 		/* Completion routine- 0dd0d000	*/
void		sd01comp1();		/* Complete disk job - 0dd0e000	*/
void		sd01intf();		/* HA SFB int routine- 0dd0f000	*/
void		sd01logerr();		/* Prints error msg. - 0dd10000	*/
void  		sd01retry(); 		/* Retry failed job  - 0dd11000	*/
void		sd01intn();		/* Normal int routine- 0dd13000	*/
int		sd01phyrw();		/* Do physical I/O   - 0dd14000	*/
int    		sd01cmd();		/* Send normal cmd   - 0dd15000	*/
int		sd01wrtimestamp(); 	/* Write time stamp  - 0dd18000	*/
int		sd01ioctl();		/* Ioctl entry point - 0dd1a000	*/
int		sd01print();		/* Print entry point - 0dd1b000	*/
void    	sd01batch(); 		/* Start new batch   - 0dd1c000	*/
void		sd01insane_dsk();	/* Display warning   - 0dd1f000	*/
int		sd01vtoc_ck();		/* Check VTOC        - 0dd20000	*/
void		sd01szsplit();	 	/* Splits large jobs - 0dd22000	*/
void		sd01flt();		/* HA flt routine    - 0dd23000	*/
void		sd01flts();		/* Start flt recovery- 0dd24000	*/
void		sd01ints();		/* SFB int handler   - 0dd25000	*/
void  		sd01intrq(); 		/* Sense int handler - 0dd26000	*/
void  		sd01intres();		/* Res. int handler  - 0dd27000	*/
void  		sd01resume();		/* Resume disk Q     - 0dd28000	*/
void  		sd01flterr(); 		/* Clean up errors   - 0dd29000	*/
void  		sd01qresume(); 		/* Checks SB is busy - 0dd2a000	*/
void  		sd01fltjob(); 		/* Eval. sense data  - 0dd2b000	*/
int		sd01part_ck();		/* Check partitions  - 0dd2c000	*/
void  		sd01hdelog();		/* Reassign blocks   - 0dd2d000	*/
void		sd01start();		/* Not used          - 0dd2f000	*/
void		sd01lognberr();		/* Not used 	     - 0dd30000	*/
int		sd01config();		/* Get configuration - 0dd31000 */
void		sd01icmd();		/* Send immed. cmd   - 0dd32000	*/
void  		sd01intb(); 		/* HDE Int handler   - 0dd33000	*/
int 		sd01addr();		/* Get device address- 0dd34000	*/
int		sd01slice();		/* Get slice number  - 0dd35000	*/


int sd01devflag = 0;			/* Indicate new style driver	*/

/*
 * Messages used by bad block handling.
 */

static char *hde_mesg[] = {
        "SD01: Soft read error corrected by ECC algorithm: block %d",

	"SD01: A potential bad block has been detected (block 0x%x)\n\
on a sacred area of the hard disk. If this block goes bad, the UNIX System\n\
will be lost.  Please backup your system.",

	"SD01: A potential bad block has been detected (block 0x%x).\n\
The system is out of spare blocks for surrogates. If the block goes bad,\n\
it can not be mapped.",

	"SD01: A potential bad block has been detected (block 0x%x).\n\
The system can not reassign a surrogate block. Try remapping this\n\
block manually.",

	"SD01: An alternate block has been assigned to block 0x%x.",

	"SD01: Block 0x%x is not writable.\nData of this block has been lost.",

	"SD01: A bad block (block 0x%x) has been detected on a sacred\n\
area of the hard disk.  The system can not recover from this failure.\n\
Must reinstall the UNIX System and restore from previous backups.",

	"SD01: Data from block 0x%x is not readable.\nData of this block \
has been lost.  An alternate block has\nbeen assigned.",

	"SD01: Data from block 0x%x is not readable.\nData of this block \
has been lost. Trying to reassign an alternate block.",

	"SD01: The system is out of spare blocks for surrogates.\n\
Block 0x%x can not be mapped.",

	"SD01: The system can not reassign a surrogate block.\n\
Try remapping block 0x%x manually.",

	"SD01: Block 0x%x is not writable.\nFailed to initialize block.",
};

/*
 * Mirror driver information structure.
 */

int	sd01_mdinfo[] = {
	(int) sd01wrtimestamp,		/* Addr of time stamp function	*/
	(int) sd01freejob,		/* Addr of free job function	*/
	(int) sd01strat1,		/* Addr of core strat routine	*/
	(int) sd01szsplit,		/* Addr of size split function	*/
	(int) sd01getjob,		/* Addr of get job function	*/
	(int) sd01ioctl,		/* Addr of ioctl routine	*/
	(int) sd01batch,		/* Addr of batch job function	*/
	(int) sd01strategy,		/* Addr of strategy routine	*/
	(int) sd01close,		/* Addr of close routine	*/
	(int) sd01open1,		/* Addr of core open function	*/
	(int) sd01addr,			/* Addr of device addr function	*/
	(int) sd01slice,		/* Addr of device slice function*/
	(int) &Sd01_cmajors,		/* Addr of char majors variable	*/
	(int) Sd01_majors		/* Addr of multiple major table */
};

/*===========================================================================*/
/* Debugging mechanism that is to be compiled only if -DDEBUG is included
/* on the compile line for the driver                                  
/* DPR provides levels 0 - 9 of debug information.                  
/*      0: No debugging.
/*      1: Entry points of major routines.
/*      2: Exit points of some major routines. 
/*      3: Variable values within major routines.
/*      4: Variable values within interrupt routine (inte, intn).
/*      5: Variable values within sd01logerr routine.
/*      6: Bad Block Handling
/*      7: Multiple Major numbers
/*      8: - 9: Not used
/*============================================================================*/

#define DEBUGFLG        73
#define O_DEBUG         0100

#ifdef DEBUG
#define DPR(l)          if (Sd01_debug[l]) printf
#endif


/* %BEGIN HEADER
 * %NAME: sd01init - Initialize the SCSI disk driver. - 0dd01000
 * %DESCRIPTION:
	This function allocates and initializes the disk driver's data 
	structures.  This function also initializes the disk drivers device 
	instance array. An instance number (starting with zero) will be assigned
	for each set of block and character major numbers. Note: the
	system must allocate the same major number for both the block
	and character devices or disk corruption will occur.
	This function does not access any devices.
 * %CALLED BY: Kernel
 * %SIDE EFFECTS: 
	The disk queues are set empty and all partitions are marked as
	closed. 
 * %RETURN VALUES: None
 * %ERROR 8dd01001
	The disk driver can not determine equipage.  This is caused by
	insufficient virtual or physical memory to allocate the driver
	Equipped Device Table (EDT) data structure.
 * %END ERROR
 * %ERROR 8dd01002
	The disk driver is not fully configured.  This is caused by insufficient
	virtual or physical memory to allocate internal driver structures.
 * %END ERROR
 * %ERROR 8dd01003
	The disk driver space.c file is not valid.  This is caused by specifying
	an invalid number of subdevices in Sd01_nodes.  The driver does not
	support less then 8 or greater then 16 partitions per drive (OBSOLETE
	ERROR).
 * %END ERROR
 * %END HEADER */
void
sd01init()
{

	extern 	struct 	tc_data	Sd01_data[];	/* Contains TC info	*/
	extern	struct 	disk *Sd01_dp;		/* Base Disk pointer	*/
	extern	struct 	job  *Sd01_jp;		/* Job struct pointer	*/

	register caddr_t tcedt_bp;		/* Base EDT buf pointer	*/

	struct 	disk 	*sd01_dp2;		/* Temp. disk pointer 	*/
	struct 	tc_data *tcdata_p;		/* Base TC data pointer	*/
	struct	tc_edt  *tcedt_p;		/* pointer passed to HAD*/

	int tc_edtsz,				/* EDT size (in bytes)	*/
	    disksz,				/* Disk size (in bytes)	*/
	    jobsz,				/* Job size (in bytes)	*/
	    i,					/* Loop index - level 1	*/
	    j,					/* Loop index - level 2	*/
	    lu,					/* LU num. if configured*/
	    tc;					/* TC num. if configured*/

#ifdef DEBUG
	DPR (1)("\n\t\tSD01 DEBUG DRIVER INSTALLED\n");
#endif
	
/* Call HAD to build SCSI edt */
	sdi_init();

/* Allocate the TC edt structure */

	tc_edtsz = sdi_hacnt * MAX_TCS * sizeof(struct tc_edt);
	if((tcedt_bp = kmem_alloc(tc_edtsz, KM_NOSLEEP)) == NULL)
	{
		cmn_err(CE_WARN, "SD01: Insufficient memory to allocate driver EDT. Err: 8dd01001");
		sd01_diskcnt = DKNOTCS;
		return;
	}
	tcedt_p  = (struct tc_edt *) tcedt_bp;

/* Get TC EDT data from HAD */

	tcdata_p = Sd01_data;
	sd01_tccnt = sdi_config("SD01", NULL, NULL, tcdata_p, 
		                Sd01_datasz, tcedt_p);

	tcedt_p = (struct tc_edt *) tcedt_bp;
	for(i=0, sd01_diskcnt=0; i < sd01_tccnt; i++, tcedt_p++) {
		sd01_diskcnt += tcedt_p->n_lus;
	}

/* Check if there are devices configured */

	if(!sd01_diskcnt) {
		kmem_free(tcedt_bp, tc_edtsz);
		sd01_diskcnt = DKNOTCS;
		return;
	}

/* Allocate the disk and job structures */
	
	disksz = sd01_diskcnt * sizeof(struct disk);
	jobsz  = sd01_diskcnt * Sd01_jobs * sizeof(struct job);
	if((Sd01_dp = (struct disk *) kmem_alloc((disksz + jobsz), KM_NOSLEEP)) == NULL)
	{
		kmem_free(tcedt_bp, tc_edtsz);
		cmn_err(CE_WARN, "SD01: Insufficient memory to configure driver. Err: 8dd01002");
		sd01_diskcnt = DKNOTCS;
		return;
	}
	Sd01_jp = (struct job *) (Sd01_dp + sd01_diskcnt);

/* Initialize the disk structures */
	sd01_dp2 = Sd01_dp;
	tcedt_p  = (struct tc_edt *) tcedt_bp;
	for(tc=0; tc < sd01_tccnt; tc++, tcedt_p++) 
	{
	    for(i=0, lu=0; (unsigned) i < tcedt_p->n_lus; i++, lu++, sd01_dp2++)
	    {
	    	for(; lu < MAX_LUS; lu++)	/* Find next equipped lu */
	    		if(tcedt_p->lu_id[lu])
				break;

					/* Initialize the queue ptrs */
		sd01_dp2->dk_forw   = (struct job *) sd01_dp2;
		sd01_dp2->dk_back   = (struct job *) sd01_dp2;
		sd01_dp2->dk_next   = (struct job *) sd01_dp2;
		sd01_dp2->dk_batch  = (struct job *) sd01_dp2;		

					/* Initialize state & error counters */
		sd01_dp2->hde_state = 0;
		sd01_dp2->dk_state  = 0;
		sd01_dp2->dk_count  = 0;
		sd01_dp2->dk_outcnt = 0;
		sd01_dp2->dk_error  = 0;
		sd01_dp2->dk_jberr  = 0;

					/* Clear the partition states 	*/
		for(j = 0; j < V_NUMPAR; j++)
			sd01_dp2->dk_part_flag[j] = 0;

					/* Clear the sanity fields  	*/
		sd01_dp2->dk_vtoc.v_sanity = 0;
		sd01_dp2->dk_pdsec.sanity  = 0;

					/* Initialize the disk params	*/
		sd01_dp2->dk_parms.dp_type      = DPT_SCSI_HD;
		sd01_dp2->dk_parms.dp_heads     = 0;
		sd01_dp2->dk_parms.dp_cyls      = 0;
		sd01_dp2->dk_parms.dp_sectors   = 0;
		sd01_dp2->dk_parms.dp_secsiz    = BLKSZ;
		sd01_dp2->dk_parms.dp_ptag      = 0;
		sd01_dp2->dk_parms.dp_pflag     = 0;
		sd01_dp2->dk_parms.dp_pstartsec = 0;
		sd01_dp2->dk_parms.dp_pnumsec   = 0;

					/* Initialize the perf data 	*/
		sd01_dp2->dk_stat.ios.io_ops   = 0;
		sd01_dp2->dk_stat.ios.io_misc  = 0;
		sd01_dp2->dk_stat.ios.io_qcnt  = 0;
		sd01_dp2->dk_stat.ios.io_unlog = 0;
		sd01_dp2->dk_stat.io_resp      = 0;
		sd01_dp2->dk_stat.io_act       = 0;
		sd01_dp2->dk_stat.tnrreq       = 0;
		sd01_dp2->dk_stat.tnwreq       = 0;
		sd01_dp2->dk_stat.cumqlen      = 0;
		sd01_dp2->dk_stat.maxqlen      = 0;
		sd01_dp2->dk_stat.minqlen      = 0;
		sd01_dp2->dk_stat.cumseekd     = 0;
		sd01_dp2->dk_stat.io_liact     = 0;
		sd01_dp2->dk_stat.io_intv      = 0;
		sd01_dp2->dk_stat.pttrack      = sd01_dp2->dk_stat.ptrackq;
		sd01_dp2->dk_stat.endptrack    = 
			&sd01_dp2->dk_stat.ptrackq[NTRACK];

					/* Initialize the dev addr data	*/
		sd01_dp2->dk_addr.sa_exlun  = 0;
		sd01_dp2->dk_addr.sa_lun    = lu;
	    	sd01_dp2->dk_addr.sa_fill   = SDI_DEV(tcedt_p);
	    }
	}

/* Un-Allocate the TC edt structure */

	kmem_free(tcedt_bp, tc_edtsz);

/* Initialize the job structures  */
	sd01_jobcnt = sd01_diskcnt * Sd01_jobs;
	sd01freej.fj_state = 0;	/* No one waiting for jobs */
	sd01freej.fj_count = sd01_jobcnt;
	for(sd01freej.fj_ptr = Sd01_jp;
		sd01freej.fj_ptr < Sd01_jp + (sd01_jobcnt - 1);
		sd01freej.fj_ptr++)
	{
		sd01freej.fj_ptr->j_forw = sd01freej.fj_ptr + 1;
		sd01freej.fj_ptr->j_mate = NULL;
	}
	sd01freej.fj_ptr = Sd01_jp; /* Set to head of list */

	(Sd01_jp + (sd01_jobcnt - 1))->j_forw = NULL; /* Terminate list */
	(Sd01_jp + (sd01_jobcnt - 1))->j_mate = NULL;

/* Setup the linked list for Resuming LU queues */
 	sd01_resume.res_head = (struct disk *) &sd01_resume;
 	sd01_resume.res_tail = (struct disk *) &sd01_resume;

	sd01_fltsbp = sdi_getblk();

/* Init array to indicate unsupported major numbers   */
        for (i = 0; i <= 64; i++)
                sd01instbl[i] = DKNOMAJ;

/* Assign instance number for supported major numbers */
        for (i = 0; i < Sd01_cmajors; i++)
                sd01instbl[Sd01_majors[i].c_maj] = i;
}

/* %BEGIN HEADER
 * %NAME: sd01getjob - Allocate a job structure. - 0dd02000
 * %DESCRIPTION:
	This function checks whether the disk queue is full.  If it is or
	was it will sleep on the disk structure waiting for the queue to
	drain.  The function will awaken after the queue has drained
	below a low water mark.  Next it will allocate a disk job structure.
	Finally, it will get a \fIscb\fR from the host adapter driver.
 * %CALLED BY: sd01strategy and mdstrategy
 * %SIDE EFFECTS: 
	A job structure and SCSI control block is allocated.
 * %RETURN VALUES: 
	A pointer to the allocated job structure.
 * %ERROR 0dd02001
	The addressed disk queue was filled up. This is caused by
	by overloading of the disk or the disk not executing jobs.
	If the condition continues verify that the disk is executing
	requests.
 * %END ERROR
 * %ERROR 2dd02002
	The SCSI disk controller ran out of job structures for I/O
	requests.  It is caused by a large number of outstanding disk
	I/O requests. If the error occurs often increase the
	sd01_jobcnt parameter in the /etc/master.d/disktd.
 * %END ERROR
 * %END HEADER */
struct job *
sd01getjob(dk)
register struct disk *dk;
{
	register struct job *retval;
	register int sps;

#ifdef DEBUG
        DPR (1)("sd01getjob: (dk=0x%x) ", dk);
#endif
	
/* Is the disk queue full or is someone already waiting on it? */
	sps = spl5();
	while (dk->dk_state & DKDRAIN || dk->dk_count > SD01_QUEHIGH)
	{
#ifdef TUNNING
		dev_t pt_dev;
		char name[48];

		if ((dk->dk_state & DKDRAIN) == 0)
		{			/* If this is the first overflow */
					/* log so we know about it */
			sdi_name(&dk->dk_addr, name);
			sdi_getdev(&dk->dk_addr, &pt_dev);
			cmn_err(CE_NOTE, "SD01: %s, Unit %d, job queue is full. Err: 0dd02001\n",
				name, dk->dk_addr.sa_lun);
			sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, 0, 0,
				&dk->dk_stat.ios, 0x0dd02001, 0, 0);
		}
#endif
		dk->dk_state |= DKDRAIN; /* Indicate we are waiting */
		sleep((caddr_t)dk, PRIBIO);
	}
	
/* Are there jobs in the que */
	while (sd01freej.fj_state  != FJOK || sd01freej.fj_ptr == NULL)
	{				/* No so wait for some */
#ifdef TUNNING
		if (sd01freej.fj_state == FJOK)
		{			/* Log if first time */
			sdi_getdev(&dk->dk_addr, &pt_dev);
			cmn_err(CE_NOTE, "SD01: The driver is out of jobs.  Err: 2dd02002\n");
			sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, 0, 0,
				&dk->dk_stat.ios, 0x2dd02002, 0, 0);
		}
#endif
		sd01freej.fj_state = FJEMPTY;
		sleep((caddr_t)&sd01freej,PRIBIO);
		spl5();
	}
	
	retval = sd01freej.fj_ptr;

	sd01freej.fj_count--;
	sd01freej.fj_ptr = retval->j_forw;
	splx(sps);
	
/* Get an scb for this job */
	retval->j_cont = sdi_getblk();
	retval->j_cont->sb_type = SCB_TYPE;

#ifdef DEBUG
        DPR (2)("sd01getjob: - exit(0x%x) ", retval);
#endif
	return(retval);
}

/* %BEGIN HEADER
 * %NAME: sd01freejob - Free a disk job structure. - 0dd03000
 * %DESCRIPTION: 
	This function returns a job structure to the free list.  The
	\fIscb\fR attached to the job is returned to the host adapter
	driver.
 * %CALLED BY: mddone sd01done
 * %SIDE EFFECTS:
	Allocated job and scb structures are returned.
 * %RETURN VALUE None
 * %END HEADER */
void
sd01freejob(jp)
register struct job *jp;
{
	register int sps;
	
#ifdef DEBUG
        DPR (1)("sd01freejob: (jp=0x%x) ", jp);
#endif
	
	sdi_freeblk(jp->j_cont);
	sps = spl5();
	jp->j_forw = sd01freej.fj_ptr;
	sd01freej.fj_ptr = jp;
	sd01freej.fj_count++;
	
/* If someone is waiting and there are enough jobs wake them up */
	if (sd01freej.fj_state != FJOK && sd01freej.fj_count > FJHIGH)
	{
		sd01freej.fj_state = FJOK;
		wakeup((caddr_t)&sd01freej);
	}
	splx(sps);

#ifdef DEBUG
        DPR (2)("sd01freejob: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01send - Sends jobs from the work queue to the host adapter. - 0dd04000
 * %DESCRIPTION: 
	This function sends jobs to the host adapter driver.  It will send
	as many jobs as available or the maximum number required to keep the
	logical unit busy.  If the job cannot be accepted by the host
	adapter driver, the function will reschedule itself via the timeout
	mechanizism. This routine must be called at slp6.
 * %CALLED BY: sd01cmd sd01strat1 and sd01intn
 * %SIDE EFFECTS: 
	Jobs are sent to the host adapter driver.
 * %RETURN VALUES: None
 * %ERROR 8dd04001
	The host adapter rejected a request from the SCSI disk driver.
	This is caused by a parameter mismatch within the driver. The system
	should be rebooted.
 * %END ERROR
 * %END HEADER */
void
sd01send(dk)
register struct disk *dk;
{
	register int sendret;		/* sd_send return value 	*/ 
	register struct job *jp;	/* Job which caused send error 	*/
	extern void	sd01sendt();
	dev_t pt_dev;			/* Pass thru major/minor number	*/

#ifdef DEBUG
        DPR (1)("sd01send: (dk=0x%x) ", dk);
#endif
	
	while (dk->dk_outcnt < SD01_OUTSZ && dk->dk_next !=(struct job *)dk)
	{
		jp = dk->dk_next;
		if (jp == dk->dk_batch)
		{			/* Start a new batch */
			dk->dk_batch = (struct job *) dk;
			dk->dk_state ^= DKDIR;
		}

		/* Swap bytes in the address field */
		if (jp->j_cont->SCB.sc_cmdsz == SCS_SZ)
			jp->j_cmd.cs.ss_addr = sdi_swap16(jp->j_cmd.cs.ss_addr);
		else {
			jp->j_cmd.cm.sm_addr = sdi_swap32(jp->j_cmd.cm.sm_addr);
			jp->j_cmd.cm.sm_len  = (short) sdi_swap16(jp->j_cmd.cm.sm_len);
		}

		dk->dk_next = jp->j_forw;
		if ((sendret = sdi_send(jp->j_cont)) != SDI_RET_OK)
		{
			if (sendret == SDI_RET_RETRY)
			{
				dk->dk_next = jp; /* Reset next pointer */
				/* Call back later */
				if (dk->dk_state & DKSEND)
					return; /* We have a call waiting */
#ifdef DEBUG
				DPR(3)("SD01: Host adapter rejected job\n");
#endif
				dk->dk_state |= DKSEND;
				dk->dk_sendid = timeout(sd01sendt,
					(caddr_t) dk, drv_usectohz(LATER));
				return;
			}
			else
			{
					/* The driver is messed up */
				dk->dk_outcnt++;
					/* sd01comp1 will undo the outcnt */
				cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd04001");
				sdi_getdev(&dk->dk_addr, &pt_dev);
				sd01lognberr(getmajor(pt_dev), getminor(pt_dev),0,0,0,
					&dk->dk_stat.ios, 0x8dd04001, 0, 0);
				sd01comp1(jp);
				continue;
			}
		}
		dk->dk_outcnt++;
	}
	
	if (dk->dk_state & DKSEND)
	{
		dk->dk_state &= ~DKSEND;
		untimeout(dk->dk_sendid);
	}

#ifdef DEBUG
        DPR (2)("sd01send: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01sendt - Send function timeout. - 0dd05000
 * %DESCRIPTION: 
	This function call \fIsd01send\fR after it turns off the DKSEND
	bit in the disk status work.
 * %CALLED BY: timeout
 * %SIDE EFFECTS: 
	The send function is called and the record of the pending
	timeout is erased.
 * %RETURN VALUES: None
 * %END HEADER */
void
sd01sendt(dk)
register struct disk *dk;
{
#ifdef DEBUG
        DPR (1)("sd01sendt: (dk=0x%x) ", dk);
#endif

	dk->dk_state &= ~DKSEND;
	sd01send(dk);

#ifdef DEBUG
        DPR (2)("sd01sendt: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01strat1 - Core strategy routine. - 0dd06000
 * %DESCRIPTION: 
	This function takes the information included in the job
	structure and the buffer header, and creates a SCSI bus
	request.  The request is queued according to the elevator
	algorithm.  This routine may be called at interrupt level. 
	The buffer and mode fields are filled in by the calling 
	functions.  If the partition argument is equal to 16 or
	greater the block address is assumed to be physical.
 * %CALLED BY: sd01strategy, sd01phyrw, mdstrategy, and mddone
 * %SIDE EFFECTS: 
	A job is queued for the disk.
 * %RETURN VALUES: None
 * %END HEADER */
void
sd01strat1(jp, part)
register struct job *jp;
int part;
{
	register struct scb *scb;
	register struct scm *cmd;
	register struct disk *dk;
	register struct job *jp1;
	register daddr_t jaddr;
	register buf_t *bp;
	register struct scsi_iotime *stat;
	int sps;
	unsigned long	*lbolt_p;
	extern void sd01intn();

#ifdef DEBUG
        DPR (1)("sd01strat1: (jp=0x%x part=%x) ", jp, part);
#endif
	
	dk = jp->j_dk;
	scb = &jp->j_cont->sb_b.b_scb;
	cmd = &jp->j_cmd.cm;
	bp = jp->j_bp;
	stat = &dk->dk_stat;
	
/* Fill in the scb for this job */
	if (bp->b_flags & B_READ)
	{
		cmd->sm_op = SM_READ;
		scb->sc_mode = SCB_READ;
		stat->tnrreq++;
	}
	else
	{
		cmd->sm_op = SM_WRITE;
		scb->sc_mode = SCB_WRITE;
		stat->tnwreq++;
	}
	cmd->sm_lun = dk->dk_addr.sa_lun;
	cmd->sm_res1 = 0;
	if (part < SD01_PBLK)
	{				/* Logical address */

#ifdef DEBUG
        DPR (3)("l_start=0x%x p_start=0x%x b_blkno=0x%x p_sz=0x%x ", dk->dk_pdsec.logicalst, dk->dk_vtoc.v_part[part].p_start, bp->b_blkno, dk->dk_vtoc.v_part[part].p_size);
#endif

		cmd->sm_addr = dk->dk_vtoc.v_part[part].p_start + bp->b_blkno;
	}
	else
	{
		cmd->sm_addr = bp->b_blkno;
	}
	cmd->sm_res2 = 0;
	cmd->sm_len = (bp->b_bcount+BLKSZ-1)>>BLKSHF;
	cmd->sm_cont = 0;
	scb->sc_cmdsz = SCM_SZ;
	/* The data elements are filled in by the calling routine */
	scb->sc_link = 0;
	scb->sc_time = JTIME;
	/* Is this a partial block? */
	if ((scb->sc_resid = (cmd->sm_len<<BLKSHF) - bp->b_bcount) != 0)
		scb->sc_mode |= SCB_PARTBLK;
	scb->sc_int = sd01intn;
	scb->sc_wd = (long) jp;
	scb->sc_dev = dk->dk_addr;	
	/* Inicate the start request */
	/* Get the pointer to number of clock ticks since last boot */
	drv_getparm(LBOLT, &lbolt_p);
	bp->b_start = drv_hztousec(lbolt_p);
	
/* Add the job to the queue according to the elevator algor */
	sps = spl5();
	dk->dk_count++;
	if(dk->dk_next == (struct job *) dk)
	{				/* Que is nearly empty so no elev */
		jp->j_forw = (struct job *) dk;
		jp->j_back = dk->dk_back;
		dk->dk_back->j_forw = jp;
		dk->dk_back = jp;
		dk->dk_next = jp;
	}
	else
	{
		jaddr = cmd->sm_addr;
		if (dk->dk_state & DKEL_OFF)
		{
			jp1 = (struct job *) dk;
		}
		else if (dk->dk_state & DKDIR)
		{			/* Going up */
			for(jp1 = dk->dk_batch; jp1 != (struct job *) dk; 
				jp1 = jp1->j_forw)
			{
				if (jp1->j_cmd.cm.sm_addr > jaddr)
					break; /* This is our floor */
			}
		}
		else
		{			/* Going down */
			for(jp1 = dk->dk_batch; jp1 != (struct job *) dk; 
				jp1 = jp1->j_forw)
			{
				if (jp1->j_cmd.cm.sm_addr < jaddr)
					break; /* This is our floor */
			}
		}
					/* Add the job into the queue */
		jp->j_forw = (struct job *) jp1;
		jp->j_back = jp1->j_back;
		jp1->j_back->j_forw = jp;
		jp1->j_back = jp;
		if (jp1 == dk->dk_batch)
			dk->dk_batch = jp;
		if (jp1 == dk->dk_next)
			dk->dk_next = jp;
	}
	sd01send(dk);
	
/* Update performance data */
	stat->ios.io_ops++;
	stat->ios.io_qcnt++;
	stat->io_bcnt += cmd->sm_len;
	if (dk->dk_count > stat->maxqlen)
		stat->maxqlen = dk->dk_count;
	if (dk->dk_count == 1) {	/* Is the disk becoming active? */
		drv_getparm(LBOLT, &lbolt_p);
		dk->dk_start = drv_hztousec(lbolt_p);
	}
	splx(sps);

#ifdef DEBUG
        DPR (2)("sd01strat1: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01strategy - Strategy routine for SCSI disks. - 0dd07000
 * %DESCRIPTION: 
	\fIsd01strategy\fR  generates the SCSI command needed to fulfill the
	I/O request.  The buffer pointer is passed from the kernel and
	contains the necessary information to perform the job.  Most of the
	work is done by \fIsd01strat1\fR.
 * %CALLED BY: kernel
 * %SIDE EFFECTS: 
	The I/O statistics are updated for the device and an I/O job is
	added to the work queue. 
 * %RETURN VALUES: None 
 * %END HEADER */
void
sd01strategy(bp)
register buf_t *bp;
{
	register struct job *jp;
	register struct disk *dk;
	extern void sd01done();
	register int last;		/* Last block in partition */
	register int numblk;		/* Number of blocks requested */
	register int part;
	int	 ret_val = 0;
	
	dk = Sd01_dp + DKINDEX(bp->b_edev);

#ifdef DEBUG
        DPR (1)("sd01strategy: (bp=0x%x) dk=0x%x ", bp, dk);
#endif

	if (dk->dk_vtoc.v_sanity != VTOC_SANE || dk->dk_vtoc.v_version != V_VERSION)
	{
		if (dk->dk_state & DKUP_VTOC && /* Was it just updated */
		(ret_val = sd01open1(getmajor(bp->b_edev), getminor(bp->b_edev), DKGEN)) == 0)
		{			/* Use open1 to read in the VTOC */
			dk->dk_state &= ~DKUP_VTOC;
		}
		else 			/* Fail the request */
		{
			/* Return EBUSY for reservation conflict */
			if (dk->dk_state & DKCONFLICT)
			{
				bp->b_error = EBUSY;
				dk->dk_state &= ~DKCONFLICT;
			}
			else if (ret_val > 0)
				bp->b_error = ret_val;
			else
			{
				if(bootinfo.bootflags != BF_FLOPPY)
					sd01insane_dsk(dk);
				bp->b_error = EIO;
			}
			dk->dk_state &= ~DKUP_VTOC;
			bp->b_flags |= B_ERROR;
			iodone(bp);
			return;
		}
	}

	/*
	*  The b_resid is initialized before the call to sd01szsplit.
	*  A problem was discovered that if the job exceeds the max size
	*  and must be broken down to smaller jobs by sd01szsplit,
	*  the b_resid field of the original buffer never gets initialized.
	*  Only the temporary buf structures used by sd01szsplit were
	*  initialized properly. It resulted in a successful job appearing
	*  to fail because the b_resid value was non-zero.
	*/
	bp->b_resid = 0;

	if (bp->b_bcount > SD01_MAXSIZE)
	{				/* The job is too big to  */
					/* handle all at once. */
		sd01szsplit(bp, sd01strategy);
		return;
	}

	part = DKSLICE(bp->b_edev);                           
	last = dk->dk_vtoc.v_part[part].p_size;
	numblk = (bp->b_bcount + BLKSZ-1) >> BLKSHF;
	if (bp->b_flags & B_READ)    
	{                          
		if ((unsigned)bp->b_blkno + numblk >  last)
		{
			if ((unsigned) bp->b_blkno > last)
			{
				bp->b_flags |= B_ERROR;
				bp->b_error = ENXIO;
				iodone(bp);
				return;
			}
			
			bp->b_resid=bp->b_bcount-
				((last-bp->b_blkno)<<BLKSHF);
			if (bp->b_bcount == bp->b_resid)
			{		/* The request is done */
				iodone(bp);
				return;
			}
			else
				bp->b_bcount -= bp->b_resid;
		}
	}
	else				/* This is a write request */
	{
		if ((unsigned)bp->b_blkno + numblk >  last)
		{			/* Fail the request */
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
		}

					/* Make sure PD and VTOC are safe */
		if (dk->dk_vtoc.v_part[part].p_start <= dk->unixst + VTBLKNO  &&
			bp->b_blkno <= dk->unixst + VTBLKNO && 
		        (ret_val = sd01vtoc_ck(dk, bp, part)))
			
		{
			bp->b_flags |= B_ERROR;
			bp->b_error = ret_val;
			iodone(bp);
			return;
		}

		if (dk->dk_vtoc.v_part[part].p_flag & V_RONLY)
		{			/* The parition is read only */
			bp->b_flags |= B_ERROR;
			bp->b_error = EACCES;
			iodone(bp);
			return;
		}
			
		if (dk->dk_vtoc.timestamp[part] != (time_t) 0)
		{
			if (sd01wrtimestamp(dk, part, 0) != 0)
			{				
				bp->b_flags |= B_ERROR;
				bp->b_error = ENODEV;
				iodone(bp);
				return;
			}
		}

	}
			
	jp = sd01getjob(dk);
	jp->j_bp = bp;
	jp->j_dk = dk;
	jp->j_done = sd01done;
	jp->j_cont->SCB.sc_datapt = bp->b_un.b_addr;
	jp->j_cont->SCB.sc_datasz =  bp->b_bcount;
	jp->j_cont->SCB.sc_cmdpt = SCM_AD(&jp->j_cmd.cm);
	sdi_translate(jp->j_cont, bp->b_flags, bp->b_proc);
	sd01strat1(jp, part);

#ifdef DEBUG
        DPR (2)("sd01strategy: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01close - close for the SCSI disk driver. - 0dd08000
 * %DESCRIPTION: 
	Clear the open flags.
 * %CALLED BY: Kernel and mdmirror
 * %SIDE EFFECTS: 
	The device is marked as unopened.
 * %RETURN VALUES: Zero
 * %END HEADER */
int
sd01close(dev, flags, otype, cred_p)
dev_t dev;
int   flags;
int otype;		/* Type of open */
struct cred *cred_p;	/* Pointer to user credential structure */
{
 	register struct disk *dk;
	register int	      part;
	register int sps;
	register struct buf *bp;
	register struct job *jp;
 
 	dk = Sd01_dp + DKINDEX(dev);
	part = DKSLICE(dev);

#ifdef DEBUG
        if (flags & O_DEBUG) { /* For DEBUGFLG ioctl */
                return(0);
        }
        DPR (1)("sd01close: (dev=0x%x flags=0x%x otype=0x%x cred_p=0x%x) ", dev, flags, otype, cred_p);
#endif

	/* Drain the i/o queue for the device to be closed */
	sps = spl5();
	for (jp = dk->dk_next; jp != (struct job *) dk; )
	{
		if (jp->j_bp->b_dev == dev)
		{
			iowait(bp);
			jp = dk->dk_next;	/* re-examine job queue */
		}
		else
			jp = jp->j_forw;	/* next job in the queue */
	}
	splx(sps);

	/* Determine the type of close being requested */
	switch(otype)
	{
	  case	OTYP_BLK:			/* Close for Block I/O */
		dk->dk_part_flag[part] &= ~DKBLK;
		break;

	  case	OTYP_MNT:			/* Close for Mounting */
		dk->dk_part_flag[part] &= ~DKMNT;
		break;

	  case	OTYP_CHR:			/* Close for Character I/O */
		dk->dk_part_flag[part] &= ~DKCHR;
		break;

	  case	OTYP_SWP:			/* Close for Swapping Device */
		dk->dk_part_flag[part] &= ~DKSWP;
		break;

	  case	OTYP_LYR:			/* Layered driver close */
		dk->dk_part_flag[part] -= DKLYR;
		break;
	}

	/*
	*  Always clear the DKONLY flag since the partition can no
	*  longer be opened for exclusive open if this function is called.
	*  Check if all flags have been cleared and the layers counter is
	*  zero before clearing the General Open flag.
	*/
 	dk->dk_part_flag[part] &= ~DKONLY;
	if (!(dk->dk_part_flag[part] & ~DKGEN))
 		dk->dk_part_flag[part] &= ~DKGEN;

#ifdef DEBUG
        DPR (2)("sd01close: - exit(0)\n");
#endif

	return(0);
}

/* %BEGIN HEADER
 * %NAME: sd01read - Raw SCSI disk read. - 0dd09000
 * %DESCRIPTION: 
	This is the raw read routine for the SCSI disk driver.  The request
	is validated to see that it is within a legal partition.  A read
	request will start on a disk block boundary reading the requested
	number of bytes.  This routine calls \fIphysiock\fR which locks the
	user buffer into core and checks that the user can access the area.
 * %CALLED BY: Kernel
 * %SIDE EFFECTS: 
	Error ENXIO is returned via physiock if the read is not to a legal
	partition.  Indirectly, the user buffer area is locked into core,
	and a SCSI read job is queued for the device.
 * %RETURN VALUES: Error value
 * %END HEADER */
int
sd01read(dev, uio_p, cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;
{
	register struct  disk *dk;
	register int part;
	int	 ret_val;

	dk   = Sd01_dp + DKINDEX(dev);
	part = DKSLICE(dev);                           

#ifdef DEBUG
        DPR (1)("sd01read: (dev=0x%x, uio_p=0x%x, cred_p=0x%x) dk=0x%x part=0x%x ", dev, uio_p, cred_p, dk, part);
#endif

	ret_val = physiock(sd01strategy, 0, dev, B_READ, dk->dk_vtoc.v_part[part].p_size, uio_p);

#ifdef DEBUG
        DPR (2)("sd01read: - exit(%d) ", ret_val);
#endif
	return(ret_val);
}

/* %BEGIN HEADER
 * %NAME: sd01write - Raw SCSI disk write. - 0dd0a000
 * %DESCRIPTION: 
	This function performs a raw write to a SCSI disk.  The request is
	validated to see that it is within a legal partition. The write will
	always start at the beginning of a disk block boundary.  Partially
	written blocks will be filled with zeros. 
	This function calls \fIphysiock\fR which locks the user buffer into
	core and checks that the user can access the area.
 * %CALLED BY: Kernel
 * %SIDE EFFECTS: 
	Error ENXIO is returned via physiock if the write is not to a legal
	partition.  Indirectly, the user buffer area is locked into core,
	and a SCSI read job is queued for the device.
 * %RETURN: Error value
 * %END HEADER */
int
sd01write(dev, uio_p, cred_p)
dev_t dev;
struct uio *uio_p;
struct cred *cred_p;
{
	register struct  disk *dk;
	register int part;
	int	 ret_val;

	dk   = Sd01_dp + DKINDEX(dev);
	part = DKSLICE(dev);                           

#ifdef DEBUG
        DPR (1)("sd01write: (dev=0x%x, uio_p=0x%x, cred_p=0x%x) dk=0x%x part=0x%x ", dev, uio_p, cred_p, dk, part);
#endif

	ret_val = physiock(sd01strategy, 0, dev, B_WRITE, dk->dk_vtoc.v_part[part].p_size, uio_p);

#ifdef DEBUG
        DPR (2)("sd01write: - exit(%d) ", ret_val);
#endif
	return(ret_val);
}

/* %BEGIN HEADER
 * %NAME: sd01open1 - Core open function. - 0dd0b000
 * %DESCRIPTION: 
	This is the core open routine for the disk and mirror driver.  If
	the VTOC has not been read in yet, \fIsd01open1\fR will attempt to
	read the pdsector and the VTOC.  
	The mode field indicates the type of open exclusive use or
	general use.  Exclusive use is requested by the mirror driver when
	the partition is bound to a mirror partition.  Once the partition is
	opened for exclusive use all other open requests for the partition
	are failed.  The partition must not be open for general use or the
	exclusive use request is failed.  The partition may be opened many
	times for general use. If the open is for exclusive use, the 
	PD sector and VTOC must be valid and the sector must not contain
	the VTOC.
 * %CALLED BY: sd01open, sd01strategy and mdmirror
 * %SIDE EFFECTS: 
	The type of open is noted.
 * %RETURN VALUES: 
	.VL 4 
	.LI "-1. "
	The open succeeded but the VTOC is insane or the partition is
	undefined.
	.LI " 0. "
	The open succeeded.
	.LI ">0. "
	The error value. 
	.LE
 * %ERROR 6dd0b001
	The disk partition table is not sane.  This is caused by a corrupted
	or uninitialized sanity word in the fdisk table (OBSOLETE ERROR).
 * %END ERROR
 * %ERROR 6dd0b002
	The SCSI disk is not configured properly.  This is caused by not
	defining an ACTIVE operating system in the partition table (fdisk).
 * %END ERROR
 * %ERROR 6dd0b003
	The SCSI disk is not configured for the UNIX operating System.  This
	is caused by specifying an ACTIVE partition in the partition table
	that may not be intended to be used with the UNIX Operating System.
 * %END ERROR
 * %ERROR 6dd0b004
	The SCSI disk does not contain a VTOC.  This is caused by specifying a
	different VTOC location in the /etc/partitions file.  The driver expects
	the VTOC to be in the PD sector at location 29.
 * %END ERROR
 * %ERROR 6dd0b005
	The UNIX system partition entry in the VTOC does not match the partition
	defined in the fdisk table.  This is caused by changing the starting
	sector address or size of slice 0 in the VTOC with out performing an
	fdisk(1M).
 * %END ERROR
 * %END HEADER */

int
sd01open1(emaj, emin, mode)
major_t	emaj;
minor_t	emin;
int  mode;
{
	register struct disk *dk;	/* Pointer to disk structure	*/
	register int part;		/* Partition number		*/
	struct   phyio	 phyarg;	/* For reading Pd and VTOC 	*/
	struct	 ipart 	*ipart;		/* The fdisk table entry 	*/
	dev_t 	 dev;			/* External device number	*/

	unsigned long	*proc_p;
	int	 ret_val, i;

	extern void	sd01flt();
	
	phyarg.retval = 0;

#ifdef DEBUG
        DPR (1)("sd01open1: (emaj=0x%x emin=0x%x mode=%d) ", emaj, emin, mode);
#endif

/* Check if the major number is valid */
	if (sd01instbl[emaj] == DKNOMAJ )
		return(ENODEV);

/* Check if the minor number is valid */
	dev = makedevice(emaj, emin);
	if ((DKINDEX(dev) + 1) > sd01_diskcnt)
		return(ENODEV);
	
	dk = Sd01_dp + DKINDEX(dev);
	part = DKSLICE(emin);
	
/* Check if the partition is busy already */
recheck:
	if(dk->dk_part_flag[part] & DKONLY)
	{
		return(EBUSY);
	}

	if(mode & DKONLY && dk->dk_part_flag[part] & DKGEN)
	{
		return(EBUSY);
	}
	
#ifdef DEBUG
        DPR (3)("sa_fill=0x%x ", dk->dk_addr.sa_fill);
#endif

/* Is the VTOC sane? If not try to read it in. */
	if(dk->dk_vtoc.v_sanity != VTOC_SANE)
	{

#ifdef DEBUG
        DPR (3)("VTOC insane ");
#endif
		if ((dk->dk_state & DKINIT) == 0)
		{
			/* Initialize disk structure */
			drv_getparm(UPROCP, &proc_p);
			dk->dk_fltsus  = sdi_getblk();	/* Suspend       */
			dk->dk_fltreq  = sdi_getblk();	/* Request Sense */
			dk->dk_fltres  = sdi_getblk();	/* Reserve       */
			dk->dk_fltrblk = sdi_getblk();	/* Read Block 	 */
			dk->dk_fltwblk = sdi_getblk();	/* Write Block	 */
			dk->dk_fltmblk = sdi_getblk();	/* Reassign Block*/

			dk->dk_fltsus->sb_type  = SFB_TYPE;
			dk->dk_fltreq->sb_type  = ISCB_TYPE;
			dk->dk_fltres->sb_type  = ISCB_TYPE;
			dk->dk_fltrblk->sb_type = ISCB_TYPE;
			dk->dk_fltwblk->sb_type = ISCB_TYPE;
			dk->dk_fltmblk->sb_type = ISCB_TYPE;

			dk->dk_stat.maj = emaj;
			dk->dk_stat.min = emin;
			dk->dk_addr.sa_major = emaj;
			dk->dk_addr.sa_minor = emin;

			dk->dk_fltreq->SCB.sc_datapt = SENSE_AD(&dk->dk_sense);
			dk->dk_fltreq->SCB.sc_datasz = SENSE_SZ;
			dk->dk_fltreq->SCB.sc_mode   = SCB_READ;
			dk->dk_fltreq->SCB.sc_cmdpt  = SCS_AD(&dk->dk_fltcmd);
			sdi_translate(dk->dk_fltreq, B_READ, (caddr_t)proc_p);

			dk->dk_fltres->SCB.sc_datapt = NULL;
			dk->dk_fltres->SCB.sc_datasz = 0;
			dk->dk_fltres->SCB.sc_mode   = SCB_WRITE;
			dk->dk_fltres->SCB.sc_cmdpt  = SCS_AD(&dk->dk_fltcmd);
			sdi_translate(dk->dk_fltres, B_WRITE,(caddr_t)proc_p);

			dk->dk_fltrblk->SCB.sc_datapt = dk->blkbuf;
			dk->dk_fltrblk->SCB.sc_datasz = BLKSZ;
			dk->dk_fltrblk->SCB.sc_mode   = SCB_READ;
			dk->dk_fltrblk->SCB.sc_cmdpt  = SCM_AD(&dk->dk_blkcmd);
			sdi_translate(dk->dk_fltrblk, B_READ, (caddr_t)proc_p);

			dk->dk_fltwblk->SCB.sc_datapt = dk->blkbuf;
			dk->dk_fltwblk->SCB.sc_datasz = BLKSZ;
			dk->dk_fltwblk->SCB.sc_mode   = SCB_WRITE;
			dk->dk_fltwblk->SCB.sc_cmdpt  = SCM_AD(&dk->dk_blkcmd);
			sdi_translate(dk->dk_fltwblk, B_WRITE, (caddr_t)proc_p);

			dk->dk_fltmblk->SCB.sc_datapt = dk->dk_dl_data;
			dk->dk_fltmblk->SCB.sc_datasz = RABLKSSZ;
			dk->dk_fltmblk->SCB.sc_mode   = SCB_WRITE;
			dk->dk_fltmblk->SCB.sc_cmdpt  = SCS_AD(&dk->dk_blkcmd);
			sdi_translate(dk->dk_fltmblk, B_WRITE, (caddr_t)proc_p);

			/* Set by sd01intn, cleared by sd01intres */
			dk->dk_fltres->SCB.sc_wd = NULL;
			dk->dk_state |= DKINIT;
		}
		
		/* Is VTOC being read */
		if ((dk->dk_state & DKVTOC) == 0) 
		{
			struct 	pdinfo 	*pdptr;
			struct 	vtoc 	*vtocptr;
			uint 	voffset;
			daddr_t	pdsect, vtocsect;
			char 	*p;
			caddr_t secbuf;
			int	i, z;

			dk->dk_state |=  DKVTOC;
			dk->dk_state &= ~DKCONFLICT;
			dk->dk_state &= ~DKFDISK;
                	dk->unixst   = 0;

			/* Check if disk parameters need to be set */
			if ((dk->dk_state & DKPARMS) == 0)
				if(ret_val = sd01config(dk)) {
					dk->dk_state &= ~DKVTOC;
					return(ret_val);
				}

			/* Allocate temporary memory for sector 0 */
			if((secbuf = kmem_alloc(BLKSZ, KM_SLEEP)) == NULL)
			{ 
				dk->dk_state &= ~DKVTOC;
				dk->dk_part_flag[part] |= DKGEN & mode;
				wakeup((caddr_t)dk);
				return(-1);
			}

			/* Read in the FDISK sector */
			phyarg.sectst  = FDBLKNO; /* sector zero */
			phyarg.memaddr = (long) secbuf;
  			phyarg.datasz  = BLKSZ;
			sd01phyrw(dk, V_RDABS, &phyarg, SD01_KERNEL);
			if (phyarg.retval != 0 ||
			   ((struct mboot *)secbuf)->signature != MBB_MAGIC)
			{
				dk->dk_state &= ~DKVTOC;
				dk->dk_part_flag[part] |= DKGEN & mode;
				wakeup((caddr_t)dk);
				kmem_free(secbuf, BLKSZ);
				return(-1);
			}

			ipart = (struct ipart *)((struct mboot *)secbuf)->parts;

                	/*
                 	 * Find active Operating System partition.
                 	 */

                	for (z = FD_NUMPART; ipart->bootid != ACTIVE; ipart++) {
                              if (--z == 0) {
				  if(part)
                                  	cmn_err(CE_WARN, "SD01: No ACTIVE partition. Err: 6dd0b002");
			          dk->dk_state &= ~DKVTOC;
			          dk->dk_part_flag[part] |= DKGEN & mode;
			          wakeup((caddr_t)dk);
				  kmem_free(secbuf, BLKSZ);
			          return(-1);
                              }
                	}

			/* Indicate Fdisk is valid */
			dk->dk_state |=  DKFDISK;

			/* Check if UNIX is the active partition */
                	if (ipart->systid != UNIXOS) {
				  if(part)
                        		cmn_err(CE_WARN, "SD01: ACTIVE partition is not a UNIX System partition. Err: 6dd0b003");
			          dk->dk_state &= ~DKVTOC;
			          dk->dk_part_flag[part] |= DKGEN & mode;
			          wakeup((caddr_t)dk);
				  kmem_free(secbuf, BLKSZ);
			          return(-1);
                	}

			/* Save starting sector of UNIX partition */
                	dk->unixst = ipart->relsect;
			
			/* Make partition 0 the whole UNIX section */

			dk->dk_vtoc.v_part[0].p_start = dk->unixst;
			dk->dk_vtoc.v_part[0].p_size  = ipart->numsect;
                	dk->dk_parms.dp_pstartsec     = dk->unixst; 

			/* Read in the PD and VTOC sector */
			phyarg.sectst  = dk->unixst + PDBLKNO; 
			phyarg.memaddr = (long) secbuf;
  			phyarg.datasz  = BLKSZ;
			sd01phyrw(dk, V_RDABS, &phyarg, SD01_KERNEL);

			pdptr = (struct pdinfo *) secbuf;
			if (pdptr->sanity != VALID_PD ||
				phyarg.retval != 0)
			{
				if (phyarg.retval == 0 && DKONLY & mode)
					sd01insane_dsk(dk);
				/* Mark the VTOC as insane */
				dk->dk_vtoc.v_sanity = 0;
				dk->dk_state &= ~DKVTOC;
				dk->dk_part_flag[part] |= DKGEN & mode;
#ifdef DEBUG
        DPR (2)("sd01open1: PD not valid - return(-1) ");
#endif
				wakeup((caddr_t)dk);
				kmem_free(secbuf, BLKSZ);
				return(-1);
			}

			/* Compare pdinfo parameters with drivers */
                	if ((pdptr->cyls != dk->dk_parms.dp_cyls) ||
                    	(pdptr->tracks != dk->dk_parms.dp_heads) ||
                    	(pdptr->sectors != dk->dk_parms.dp_sectors) ||
                    	(pdptr->bytes != BLKSZ)) {

#ifdef DEBUG
        DPR (3)("SD01: Pdinfo doesn't match disk parameters\n");
        DPR (3)("cyls=0x%x / 0x%x tracks=0x%x / 0x%x sec=0x%x / 0x%x bytes=0x%x / 0x%x ", 
	pdptr->cyls, dk->dk_parms.dp_cyls, pdptr->tracks, dk->dk_parms.dp_heads,
	pdptr->sectors, dk->dk_parms.dp_sectors, pdptr->bytes, BLKSZ);
#endif

				dk->dk_parms.dp_heads   = pdptr->tracks;
				dk->dk_parms.dp_cyls    = pdptr->cyls;
				dk->dk_parms.dp_sectors = pdptr->sectors;
				dk->dk_state |= DKPARMS; 
			}

			/* Update the in-core PD info structure */
			p = (caddr_t) &dk->dk_pdsec;
			for (z = 0; z < sizeof(dk->dk_pdsec); z++,p++)
				*p = secbuf[z];

                	/* Determine VTOC location on the disk */
                	vtocsect = dk->unixst + (dk->dk_pdsec.vtoc_ptr >> BLKSHF);

			/* Check if VTOC and PD info are in the same sector */
                	voffset = dk->dk_pdsec.vtoc_ptr & (BLKSZ-1);
                	if (vtocsect != (dk->unixst + PDBLKNO)) {
                        	/* VTOC is in a different sector. */
				if(part)
                        		cmn_err(CE_WARN, "SD01: Can not determine VTOC location. Err: 6dd0b004");
				/* Mark the VTOC as insane */
				dk->dk_vtoc.v_sanity = 0;
				dk->dk_state &= ~DKVTOC;
				dk->dk_part_flag[part] |= DKGEN & mode;
				wakeup((caddr_t)dk);
				kmem_free(secbuf, BLKSZ);
				return(-1);
                	}

                        vtocptr = (struct vtoc *) (secbuf + voffset);
	
                	if (vtocptr->v_sanity != VTOC_SANE) {
#ifdef DEBUG
        DPR (3)("sd01open1: VTOC not valid - return(-1) ");
#endif
				/* Mark the VTOC as insane */
				dk->dk_vtoc.v_sanity = 0;
				dk->dk_state &= ~DKVTOC;
				dk->dk_part_flag[part] |= DKGEN & mode;
				wakeup((caddr_t)dk);
				kmem_free(secbuf, BLKSZ);
				return(-1);
                	}

                	/* Make sure partition 0 is correct.  */
                	if (vtocptr->v_part[0].p_start != dk->dk_vtoc.v_part[0].p_start
                               	|| vtocptr->v_part[0].p_size != dk->dk_vtoc.v_part[0].p_size) {
				  if(part)
                        		cmn_err(CE_WARN, "SD01: Incorrect partition 0. Err: 6dd0b005");
				/* Mark the VTOC as insane */
				dk->dk_vtoc.v_sanity = 0;
				dk->dk_state &= ~DKVTOC;
				dk->dk_part_flag[part] |= DKGEN & mode;
				wakeup((caddr_t)dk);
				kmem_free(secbuf, BLKSZ);
				return(-1);
                	}

			/* Update the in-core VTOC structure */
			p = (caddr_t) &dk->dk_vtoc;
			for (z=voffset, i=0; i < sizeof(dk->dk_vtoc); i++,p++) {
				*p = secbuf[z++];
			}

			dk->dk_state &= ~DKVTOC;
			wakeup((caddr_t)dk);
			kmem_free(secbuf, BLKSZ);
		}
		else 
		{			/* Someone else is reading VTOC */
			while(dk->dk_state & DKVTOC)
				sleep((caddr_t)dk, PRIBIO);
			goto recheck;
		}
	}

/* Check if mirroring an insane VTOC */
	if (dk->dk_vtoc.v_sanity != VTOC_SANE || 
	    dk->dk_vtoc.v_version != V_VERSION) {
		if (phyarg.retval == 0 && DKONLY & mode)
			sd01insane_dsk(dk);
	}

/* Check if VTOC is valid */
	if (dk->dk_vtoc.v_sanity != VTOC_SANE || 
	    dk->dk_vtoc.v_version != V_VERSION ||
	    dk->dk_vtoc.v_part[part].p_size == 0 ||
	    dk->dk_vtoc.v_part[part].p_tag == V_OTHER)
	{

#ifdef DEBUG
        DPR (3)("sanity=0x%x version=0x%x size=0x%x ",dk->dk_vtoc.v_sanity, dk->dk_vtoc.v_version, dk->dk_vtoc.v_part[part].p_size);
        DPR (2)("sd01open1: VTOC still insane - return(-1) ");
#endif
		dk->dk_part_flag[part] |= mode & DKGEN;
		return(-1);
	}


	if (mode & DKONLY && dk->dk_vtoc.v_part[part].p_start <= dk->unixst + VTBLKNO)
	{				/* Can't mirror, it includes VTOC */
		return(-1);
	}
	
	dk->dk_part_flag[part] |= mode;

#ifdef DEBUG
        DPR (2)("sd01open1: - exit(0) ");
#endif
	return(0);
}

/* %BEGIN HEADER
 * %NAME: sd01open - Open routine for SCSI disks. - 0dd0c000
 * %DESCRIPTION: 
	If this is the first open to the device, \fIsd01open\fR
	will read in the VTOC.  If the time stamp is set for this partition,
	\fIsd01strategy\fR will write the time stamp back
	with zero when the first write occurs to this partition.
	\fIsd01open\fR will fail if the partition is part of a mirrored pair
	or if the pdsector and VTOC cannont be read in.
 * %CALLED BY: Kernel
 * %SIDE EFFECTS: 
	The PD and VTOC onformation is read into the disk data structure.
 * %RETURN VALUES: Zero or error value 
 * %END HEADER */
int
sd01open(dev_p, flags, otype, cred_p)
dev_t *dev_p;
int flags;
int otype;		/* Type of open */
struct cred *cred_p;	/* Pointer to user credential structure */
{
	register struct disk *dk;
	register int	      part;
	major_t	 emaj;
	minor_t	 emin;
	long	 save_flags;
	int	 ret_val;

#ifdef DEBUG
        DPR (1)("\nsd01open: (dev=0x%x flags=0x%x otype=0x%x cred_p=0x%x) ", *dev_p, flags, otype, cred_p);

        if (flags & O_DEBUG)	 /* For DEBUGFLG ioctl */
                return(0);
#endif

	emaj = getmajor(*dev_p);
	emin = getminor(*dev_p);
	
	/* Don't set open flag unless open succeeds */
	if ((ret_val = sd01open1(emaj, emin, DKGEN)) > 0)
		return(ret_val);

	/* Assign device pointer after minor number is validated in sd01open1 */
	dk   = Sd01_dp + DKINDEX(*dev_p);
	part = DKSLICE(*dev_p);
	save_flags = dk->dk_part_flag[part];

	/* Set swap device size */
	if ((*dev_p == rootdev) && (swapdev != NODEV) && DKINDEX(rootdev) == DKINDEX(swapdev)) {
		nswap = dk->dk_vtoc.v_part[DKSLICE(swapdev)].p_size;
	}


	/* Determine the type of open being requested */
	switch(otype)
	{
	  case	OTYP_BLK:		/* Open for Block I/O */
		dk->dk_part_flag[part] |= DKBLK;
		break;

	  case	OTYP_MNT:		/* Open for Mounting */
		if(ret_val == -1)
		{ /* bad vtoc or PD sector */
			/* If no flags were set before the open, clear DKGEN */
			if (!(save_flags & ~DKGEN))
				dk->dk_part_flag[part] &= ~DKGEN;
			return(ENXIO);
		}
		if (dk->dk_vtoc.v_sanity == VTOC_SANE &&
		    dk->dk_vtoc.v_part[part].p_flag == V_UNMNT)
		{
			/* If no flags were set before the open, clear DKGEN */
			if (!(save_flags & ~DKGEN))
				dk->dk_part_flag[part] &= ~DKGEN;
			return(EACCES);
		}
		else
			dk->dk_part_flag[part] |= DKMNT;
		break;

	  case	OTYP_CHR:		/* Open for Character I/O */
		dk->dk_part_flag[part] |= DKCHR;
		break;

	  case	OTYP_SWP:		/* Open for Swapping Device */
		dk->dk_part_flag[part] |= DKSWP;
		break;

	  case	OTYP_LYR:		/* Layered driver counter */
		dk->dk_part_flag[part] += DKLYR;
		break;
	}

#ifdef DEBUG
        DPR (2)("\nsd01open: - exit ");
#endif

	return(0);
}

/* %BEGIN HEADER
 * %NAME: sd01done - Disk driver I/O completion routine. - 0dd0d000
 * %DESCRIPTION: 
	This function completes the I/O request after a job is returned by
	the host adapter. It will return the job structure and call
	\fIiodone\fR.
 * %CALLED BY: sd01comp1
 * %SIDE EFFECTS: 
	The kernel is notified that one of its requests completed.
 * %RETURN VALUES: None
 * %END HEADER */
void
sd01done(jp)
register struct job *jp;
{

#ifdef DEBUG
        DPR (1)("sd01done: (jp=0x%x) ", jp);
#endif

	if (jp->j_cont->SCB.sc_comp_code != SDI_ASW)
	{

		/*
		*  Check to see if the failed job was an update of
		*  the time stamps for a mirrored partition.
		*  Don't set the error since the mirrored disk may complete.
		*/
		if (jp->j_dk->dk_state & DKTSMD)
		{
			jp->j_dk->dk_state &= ~DKTSMD;

		}
		else
		{	/* Record the error for a normal job */
			jp->j_bp->b_flags |= B_ERROR;
			if (jp->j_cont->SCB.sc_comp_code == SDI_NOTEQ)
				jp->j_bp->b_error = ENODEV;
			else if (jp->j_cont->SCB.sc_comp_code == SDI_OOS)
				jp->j_bp->b_error = EIO;
			else if (jp->j_cont->SCB.sc_comp_code == SDI_CKSTAT && jp->j_cont->SCB.sc_status == S_RESER)
			{
				jp->j_bp->b_error = EBUSY;	/* Reservation Conflict */
				jp->j_dk->dk_state |= DKCONFLICT;
			}
			else
				jp->j_bp->b_error = EIO;
		}
	}
	
	iodone(jp->j_bp);
	sd01freejob(jp);

#ifdef DEBUG
        DPR (2)("sd01done: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01comp1 - Complete a disk job. - 0dd0e000
 * %DESCRIPTION: 
	This function removes the job from the queue.  Updates the disk
	counts.  Restarts the logical unit queue if necessary, and prints an
	error for failing jobs.
 * %CALLED BY: sd01intn 
 * %SIDE EFFECTS: 
	Removes the job from the disk queue.
 * %RETURN VALUES: None
 * %ERROR 6dd0e001
	An I/O request failed due to an error returned by the host adpater.
	All recovery action failed and the I/O request was returned to the 
	requestor.  The secondary error code is equal to the SDI return
	code.  See the SDI error codes for more information.
 * %END ERROR
 * %ERROR 6dd0e002
	A bad block was detected.  The driver can not read this block. A 
	Error Correction Code (ECC) was unsuccessful.  The data in this block
	has been lost.  The data can only be retrived from a previous backup.
 * %END ERROR
 * %END HEADER */
void
sd01comp1(jp)
register struct job *jp;
{
	register struct disk *dk;
	time_t resp_time, lbolt_time; 
	unsigned long *lbolt_p;
	
	dk = jp->j_dk;

#ifdef DEBUG
        DPR (1)("sd01comp1: (jp=0x%x) dk=0x%x ", jp, dk);
#endif
	
	jp->j_forw->j_back = jp->j_back; /* Remove job from the queue */
	jp->j_back->j_forw = jp->j_forw;
	
	dk->dk_count--;
	dk->dk_outcnt--;

	sd01send(dk);			/* Send out the next job */
	
/* Update performance stat */
	if (dk->dk_stat.minqlen > dk->dk_count)
		dk->dk_stat.minqlen = dk->dk_count;
	
	dk->dk_stat.ios.io_qcnt--;

	drv_getparm(LBOLT, &lbolt_p);
	lbolt_time = (time_t) drv_hztousec(lbolt_p);
	resp_time = lbolt_time - jp->j_bp->b_start;
	if (jp->j_forw != (struct job *) dk)
		dk->dk_stat.io_act -= (lbolt_time - jp->j_forw->j_bp->b_start);
	dk->dk_stat.io_act += resp_time;
		
/* If this is a read or write update the I/O queue */
	if ((char) *jp->j_cont->SCB.sc_cmdpt == SM_READ ||
		(char) *jp->j_cont->SCB.sc_cmdpt == SM_WRITE)
	{
		if(dk->dk_stat.pttrack >= dk->dk_stat.endptrack)
			dk->dk_stat.pttrack = &dk->dk_stat.ptrackq[0];
					/* Save the physical address */
		dk->dk_stat.pttrack->b_blkno = sdi_swap32(jp->j_cmd.cm.sm_addr);
		dk->dk_stat.pttrack->bp = jp->j_bp;
		dk->dk_stat.pttrack++;	/* This is how idfc does it */
		dk->dk_stat.io_resp += resp_time;
	}
/* Log error if necessary */
	if (jp->j_cont->SCB.sc_comp_code != SDI_ASW)
	{
		if(dk->hde_state & HDEECCERR) {
			sd01logerr(jp->j_cont, jp, 0x6dd0e002);
			jp->j_cont->SCB.sc_wd = SC_UNRECOVRRD;
		}
		else
			sd01logerr(jp->j_cont, jp, 0x6dd0e001);
	}

/* Fix bad block if necessary - lu Q is suspended */
	if(dk->hde_state & HDEBADBLK) 
		sd01hdelog(dk);

	else if (dk->dk_state & DKSUSP)	/* Is the lu Q suspended */
		sd01qresume(dk); 	/* Resume Que */

	
	dk->dk_jberr = 0;		/* Reset retry counter */
	jp->j_done(jp);			/* Call the done routine */
	
	if (dk->dk_state & DKDRAIN && dk->dk_count < SD01_QUELOW)
	{
		dk->dk_state &= ~DKDRAIN;
		wakeup((caddr_t)dk);
	}
	
#ifdef DEBUG
        DPR (2)("sd01comp1: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01intf - Function block completion handler. - 0dd0f000
 * %DESCRIPTION: 
	This function is called by the host adapter driver when a host
	adapter function request has completed.  If the request completed
	without error, then the block is marked as free.  If there was error
	the request is retried.  Used for resume function completions.
 * %CALLED BY: Host Adapter driver.
 * %SIDE EFFECTS: None
 * %RETURN VALUES: None
 * %ERROR 4dd0f001
	A SCSI disk driver function request was retried.  The retry 
	performed because the host adapter driver detected an error.  
	The SDI return code is the second error code word.  See
	the SDI return codes for more information.
 * %END ERROR
 * %ERROR 8dd0f002
	The host adapter rejected a request from the SCSI disk driver.
	This is caused by a parameter mismatch within the driver. The system
	should be rebooted.
 * %END ERROR
 * %ERROR 6dd0f003
	A SCSI disk driver function request failed because  the host
	adapter driver detected a fatal error or the retry count was
	exceeded.  This failure will cause the effected unit to
	hang.  The system must be rebooted. The SDI return code is
	the second error code word.  See the SDI return codes for
	more information.
 * %END ERROR
 * %END HEADER */
void
sd01intf(sbp)
register struct sb *sbp;
{
	register struct disk *dk;
	dev_t dev;			/* External device number	*/
	dev_t pt_dev;			/* Pass thru device number 	*/

	extern void  sd01resume();
	
	dev = makedevice(sbp->SFB.sf_dev.sa_major, sbp->SFB.sf_dev.sa_minor);
	dk  = Sd01_dp + DKINDEX(dev);

#ifdef DEBUG
        DPR (1)("sd01intf: (sbp=0x%x) dk=0x%x ", sbp, dk);
#endif

	if (sbp->SFB.sf_comp_code & SDI_RETRY &&
		dk->dk_spcount < SD01_RETRYCNT)
	{				/* Retry the function request */

		dk->dk_spcount++;
		dk->dk_error++;
		sd01logerr(sbp, (struct job *) NULL, 0x4dd0f001);
		if (sdi_icmd(sbp) != SDI_RET_OK) 
		{			/* Resend the function request */
			cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd0f002");
			sdi_getdev(&dk->dk_addr, &pt_dev);
			sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, 0, 0,
				&dk->dk_stat.ios, 0x8dd0f002, 0, 0);
		}
		return;
	}
	
	if (sbp->SFB.sf_comp_code != SDI_ASW)
	{
		sd01logerr(sbp, (struct job *) NULL, 0x6dd0f003);
	}

	dk->dk_spcount = 0;		/* Zero retry count */
 
	/*
	*  Currently, only RESUME SFB uses this interrupt handler
	*  so the following block of code is OK as is.
	*/

 	/* This disk LU has just been resumed */
 	sd01_resume.res_head = dk->dk_fltnext;
 
	/* Are there any more disk queues needing resuming */
 	if (sd01_resume.res_head == (struct disk *) &sd01_resume)
	{	/* Queue is empty */

		/*
		*  There is a pending resume for this device so 
		*  since the Q is empty, just put the device back
		*  at the head of the Q.
		*/
		if (dk->dk_state & DKPENDRES)
		{
			dk->dk_state &= ~DKPENDRES;
			sd01_resume.res_head = dk;
			sd01resume(sd01_resume.res_head);
		}
		/*
		*  The Resume has finished for this device so clear
		*  the bit indicating that this device was on the Q.
		*/
		else
		{
			dk->dk_state &= ~DKONRESQ;
 			sd01_resume.res_tail = (struct disk *) &sd01_resume;
		}
	}
 	else
	{	/* Queue not empty */

		/*  Is there another RESUME pending for this disk */
		if (dk->dk_state & DKPENDRES)
		{
			dk->dk_state &= ~DKPENDRES;

			/* Move Next Resume for this disk to end of Queue */
			sd01_resume.res_tail->dk_fltnext = dk;
			sd01_resume.res_tail = dk;
			dk->dk_fltnext = (struct disk *) &sd01_resume;
		}
		else	/* Resume next disk */
			dk->dk_state &= ~DKONRESQ;

 		sd01resume(sd01_resume.res_head);
	}

#ifdef DEBUG
        DPR (2)("sd01intf: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01logerr - Logs and prints error messages. - 0dd10000
 * %DESCRIPTION: 
	This function will log and print the error messages for errors
	detected by the host adapter driver.  No message will be printed
	for thoses errors that the host adapter driver has already 
	detected.  Other errors such as write protection are
	not reported.  The job argument maybe NULL.
 * %CALLED BY: sd01comp1, sd01intf, sd01intn, sd01ints, sd01intrq, sd01intres
 * %SIDE EFFECTS: An error report is generated.
 * %RETURN VALUES: None
 * %END HEADER */
void
sd01logerr(sbp, jp, err_code)
register struct sb *sbp;
register struct job *jp;
register int err_code;
{
	register struct disk *dk;
	buf_t *bp;
	dev_t dev;			/* External device number 	*/


#ifdef DEBUG
        DPR (1)("sd01logerr: (sbp=0x%x jp=0x%x err_code=0x%x) ", sbp, jp, err_code);
#endif
	
	/* If OOS, then don't complain */
	if (sbp->sb_type == SFB_TYPE && sbp->SFB.sf_comp_code != SDI_OOS)
	{
		dev = makedevice(sbp->SFB.sf_dev.sa_major, sbp->SFB.sf_dev.sa_minor);
		dk  = Sd01_dp + DKINDEX(dev);
#ifdef DEBUG
		DPR (4)("SD01: SFB failed. SB addr = %x\n", sbp);
		DPR (4)("Completion code = %x\n", sbp->SFB.sf_comp_code);
		DPR (4)("Interrupt function address = %x\n", sbp->SFB.sf_int);
		DPR (4)("Major number = %d\n", sbp->SFB.sf_dev.sa_major);
		DPR (4)("Minor number = %d\n", sbp->SFB.sf_dev.sa_minor);
		DPR (4)("Logical unit = %d\n", sbp->SFB.sf_dev.sa_lun);
		DPR (4)("Function code = %x\n", sbp->SFB.sf_func);
		DPR (4)("Word = %x\n", sbp->SFB.sf_wd);
#endif
		sdi_name(&sbp->SFB.sf_dev, sd01name);
		cmn_err(CE_WARN, "SD01: I/O error. %s, Unit = %d, Err: %x",
			sd01name, sbp->SFB.sf_dev.sa_lun, err_code);
		cmn_err(CE_CONT, "SDI return code = %x\n",
			sbp->SFB.sf_comp_code);
		sdi_getdev(&dk->dk_addr, &sd01pt_dev);
		sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), 0 , 0 , 0,
			&dk->dk_stat.ios,err_code,sbp->SFB.sf_comp_code,0);
		return;
	}

	
#ifdef DEBUG
	DPR(5)("SD01: SCB failed. SB addr = %x, Job addr = %x\n", sbp, jp);
	DPR(5)("Completion code = %x\n", sbp->SCB.sc_comp_code);
	DPR(5)("Device address = %d, %d, %d\n", sbp->SCB.sc_dev.sa_major,
		sbp->SCB.sc_dev.sa_minor, sbp->SCB.sc_dev.sa_lun);
	DPR(5)("Command Addr = %x, Size = %x\n",
		sbp->SCB.sc_cmdpt, sbp->SCB.sc_cmdsz);
	DPR(5)("Data = %x, Size = %x\n",
		sbp->SCB.sc_datapt, sbp->SCB.sc_datasz);
	DPR(5)("Word = %x\n", sbp->SCB.sc_wd);

	DPR(5)("\nStatus = %x\n", sbp->SCB.sc_status);
	DPR(5)("Interrupt function address = %x\n", sbp->SCB.sc_int);
	DPR(5)("Residue = %d\n", sbp->SCB.sc_resid);
	DPR(5)("Time = %d\n", sbp->SCB.sc_time);
	DPR(5)("Mode = %x\n", sbp->SCB.sc_mode);
	DPR(5)("Link = %x\n", sbp->SCB.sc_link);
#endif

/* Ignore the error if it is unequipped or out of service. */
	if (sbp->SCB.sc_comp_code == SDI_OOS ||
		sbp->SCB.sc_comp_code == SDI_NOTEQ)
	{
		return;
	}

	/* Ignore the error if we are doing a test unit ready	*/
	/* test unix ready fails if the LU is not equipped	*/
	if ((char) *sbp->SCB.sc_cmdpt == SS_TEST)
	{
		return;
	}

	/* Don't report RESERVATION Conflicts   */
	/* User will know them from errno value */
	if (sbp->SCB.sc_comp_code == SDI_CKSTAT &&
	    sbp->SCB.sc_status == S_RESER)
	{
		return;
	}
	
/* Now check for a Check Status error */

	dev = makedevice(sbp->SCB.sc_dev.sa_major, sbp->SCB.sc_dev.sa_minor);
	dk  = Sd01_dp + DKINDEX(dev);
	sdi_name(&sbp->SCB.sc_dev, sd01name);
	sdi_getdev(&sbp->SCB.sc_dev, &sd01pt_dev);
	

	if (jp != NULL)
		bp = jp->j_bp;
	else
		bp = NULL;
		
	if (sbp->SCB.sc_comp_code == SDI_CKSTAT && 
		sbp->SCB.sc_status == S_CKCON &&
		(char) *sbp->SCB.sc_cmdpt != SS_REQSEN)
	{			/* Determine request sense data */

		cmn_err(CE_WARN, "SD01: I/O error. %s, Unit = %d, Err: %x",
			sd01name, sbp->SCB.sc_dev.sa_lun, err_code);
		if ( bp != NULL && ((char)*sbp->SCB.sc_cmdpt == SM_READ ||
			(char) *sbp->SCB.sc_cmdpt == SM_WRITE))
			cmn_err(CE_CONT, "block= 0x%x, count= %d\n",
				dk->dk_sense.sd_ba,  bp->b_bcount);
			cmn_err(CE_CONT, "Sense key: 0x%x, Extended sense: 0x%x, Op code: 0x%x\n",
				dk->dk_sense.sd_key,dk->dk_sense.sd_sencode,
				(char) *sbp->SCB.sc_cmdpt);
		
		if ( bp != NULL && ((char)*sbp->SCB.sc_cmdpt == SM_READ ||
			(char) *sbp->SCB.sc_cmdpt == SM_WRITE))
			sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), bp->b_flags,
				dk->dk_sense.sd_ba,
				bp->b_bcount, &dk->dk_stat.ios, err_code,
				dk->dk_sense.sd_key,
				dk->dk_sense.sd_sencode);
		else
			sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), 0, 0, 0,
				&dk->dk_stat.ios, err_code,
				dk->dk_sense.sd_key,
				dk->dk_sense.sd_sencode);
				
		
		dk->dk_sense.sd_key = 0;
		dk->dk_sense.sd_sencode = 0;
		return;
	}
	
	if (sbp->SCB.sc_comp_code  == SDI_CKSTAT)
	{
		cmn_err(CE_WARN, "SD01: I/O error. %s, Unit = %d, Err: %x",
			sd01name, sbp->SCB.sc_dev.sa_lun, err_code);
		cmn_err(CE_CONT, "Target controller status: %x\n",
			sbp->SCB.sc_status);
		sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), 0, 0, 0,
			&dk->dk_stat.ios, err_code,
			sbp->SCB.sc_status, 0);
		return;
	}
	
#ifdef DEBUG
	if (jp != NULL)
	{
		DPR(5)("SD01: Failing job. Addr = %x Disk addr = %x\n", 
			jp, dk);
		DPR(5)("Forward pointer = %x\n", jp->j_forw);
		DPR(5)("Backward pointer = %x\n", jp->j_back);
		DPR(5)("SB pointer = %x\n", jp->j_cont);
		DPR(5)("Other job pointer = %x\n", jp->j_mate);
		DPR(5)("Done funciton pointer = %x\n", jp->j_done);
		DPR(5)("Buffer pointer = %x\n", jp->j_bp);
		DPR(5)("Device number = %d, %d\n", sbp->SCB.sc_dev.sa_major,
			 sbp->SCB.sc_dev.sa_minor);
	}
#endif

	cmn_err(CE_WARN, "SD01: I/O error. %s, Unit = %d, Err: %x",
		sd01name, sbp->SCB.sc_dev.sa_lun, err_code);
	if ( bp != NULL && ((char)*sbp->SCB.sc_cmdpt == SM_READ ||
		(char) *sbp->SCB.sc_cmdpt == SM_WRITE))
		cmn_err(CE_CONT, "block= 0X%x, count= %d\n",
			sdi_swap32(jp->j_cmd.cm.sm_addr), bp->b_bcount);
	cmn_err(CE_CONT, "SDI return code: 0x%x\n", 
		sbp->SCB.sc_comp_code);
	
	if ( bp != NULL && ((char)*sbp->SCB.sc_cmdpt == SM_READ ||
		(char) *sbp->SCB.sc_cmdpt == SM_WRITE))
		sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), bp->b_flags,
			sdi_swap32(jp->j_cmd.cm.sm_addr),bp->b_bcount,&dk->dk_stat.ios, 
			err_code, sbp->SCB.sc_comp_code, 0);

	else
		sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), 0, 0, 0,
			&dk->dk_stat.ios, err_code,
			sbp->SCB.sc_comp_code, 0);

#ifdef DEBUG
        DPR (2)("sd01logerr: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01retry - retry a failed job. - 0dd11000
 * %DESCRIPTION: 
	This function retries a failed job. 
 * %CALLED BY: sd01intf, and sd01intn
 * %SIDE EFFECTS: If necessary the que suspended bit is set.
 * %RETURN VALUES: None
 * %ERROR 8dd11001
	The host adapter rejected a request from the SCSI disk driver.
	This is caused by a parameter mismatch within the driver. The system
	should be rebooted.
 * %END ERROR
 * %END HEADER */
void
sd01retry(jp)
register struct job *jp;
{
	register struct sb *sbp;
	struct disk *dk;
	dev_t pt_dev;

	sbp = jp->j_cont;
	dk = jp->j_dk;

#ifdef DEBUG
        DPR (1)("sd01retry: (jp=0x%x) dk=0x%x", jp, dk);
#endif
	
	dk->dk_jberr++;			/* Update the error counts */
	dk->dk_error++;
	
	if (sbp->SCB.sc_comp_code & SDI_SUSPEND)
	{
		dk->dk_state |= DKSUSP;
	}
		
	sbp->SCB.sc_time = JTIME;	/* Reset the job time */
	sbp->sb_type = ISCB_TYPE;
	
	if (sdi_icmd(sbp) != SDI_RET_OK)
	{
		cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd11001");
		sdi_getdev(&dk->dk_addr, &pt_dev);
		sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, 0, 0,
			&dk->dk_stat.ios, 0x8dd11001, 0, 0);
					/* Fail the job */
		sd01comp1((struct job *)sbp->SCB.sc_wd);
	}

#ifdef DEBUG
        DPR (2)("sd01retry: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01inte - Request sense job completion handler. - 0dd12000
 * %DESCRIPTION: 
	This function and its error numbers are obsolete.
 * %END HEADER */

/* %BEGIN HEADER
 * %NAME: sd01intn - Normal interrupt routine for SCSI job completion. - 0dd13000
 * %DESCRIPTION: 
	This function is called by the host adapter driver when a
	SCSI job completes.  If the job completed normally the job
	is removed from the disk job queue, and the requester's
	completion function is called to complete the request.  If
	the job completed with an error the job will be retried when
	appropriate.  Requests which still fail or are not retried
	are failed and the error number is set to EIO.    Device and
	controller errors are logged and printed to the user
	console.
 * %CALLED BY: Host adapter driver
 * %SIDE EFFECTS: 
	Requests are marked as complete.  The residual count and error
	number are set if required.
 * %RETURN VALUES: None 
 * %ERROR 8dd13001
	The host adapter rejected a request sense job from the SCSI 
	disk driver.  The originally failing job will also be failed.
	This is caused by a parameter mismatch within the driver. The system
	should be rebooted.
 * %END ERROR
 * %ERROR 4dd13002
	The SCSI disk driver is retrying an I/O request because of a fault
	which was detected by the host adapter driver.  The second error
	code indicates the SDI return code.  See the SDI return code for 
	more information as to the detected fault.
 * %END ERROR
 * %ERROR 4dd13003
	The addressed SCSI disk returned an unusual status.  The job
	will be  retried later.  The second error code is the status
	which was  returned.  This condition is usually caused by a
	problem in  the target controller.
 * %END ERROR
 * %END HEADER */
void
sd01intn(sbp)
register struct sb *sbp;
{
	register struct disk *dk;
	extern void sd01flts();
	extern sd01sendcmd();
	
	if (sbp->SCB.sc_comp_code == SDI_ASW)
	{
		sd01comp1((struct job *)sbp->SCB.sc_wd);
		return;
	}
	
	dk = ((struct job *) sbp->SCB.sc_wd)->j_dk;

#ifdef DEBUG
        DPR (1)("sd01intn: (sbp=0x%x) dk=0x%x ", sbp, dk);
#endif

	if (sbp->SCB.sc_comp_code & SDI_SUSPEND)
	{
		dk->dk_state |= DKSUSP;	/* Note the queue is suspended */
	}

	if (sbp->SCB.sc_comp_code == SDI_CKSTAT && 
		sbp->SCB.sc_status == S_CKCON &&
		dk->dk_jberr < SD01_RETRYCNT)
	{				/* We need to do a request sense */
 		/*
		*  Enter the gauntlet!
		*  The job pointer is set here and cleared in the
		*  gauntlet when the job is eventually retried
		*  or when the gauntlet exits due to an error.
 		*/
		dk->dk_fltres->SCB.sc_wd = sbp->SCB.sc_wd;
 		sd01flts(dk);
#ifdef DEBUG
        DPR (2)("sd01intn: - return ");
#endif
 		return;
	}
	
 	if (sbp->SCB.sc_comp_code == SDI_CRESET || sbp->SCB.sc_comp_code == SDI_RESET)
 	{
 		/*
		*  Enter the gauntlet!
		*  The job pointer will be cleared when the job
		*  eventually is retried or when the gauntlet
		*  decides to exit due to an error.
		*/
		dk->dk_fltres->SCB.sc_wd = sbp->SCB.sc_wd;
 		sd01flts(dk);
 		return;
 	}
	
	/*
	*  To get here, the failure of the job was not due to a bus reset
	*  nor to a Check Condition state.
	*
	*  Now check for the following conditions:
	*     -  The RETRY bit was not set on SDI completion code.
	*     -  Exceeded the retry count for this job.
	*     -  Job status indicates RESERVATION Conflict.
	*
	*  If one of the conditions is TRUE, then return the failed job
	*  to the user.
	*/
	if ((sbp->SCB.sc_comp_code & SDI_RETRY) == 0 ||
		dk->dk_jberr >= SD01_RETRYCNT ||
		(sbp->SCB.sc_comp_code == SDI_CKSTAT && sbp->SCB.sc_status == S_RESER))
	{
		sd01comp1((struct job *)sbp->SCB.sc_wd);
		return;
	}
	
	if (sbp->SCB.sc_comp_code == SDI_CKSTAT)
	{				/* Retry the job later */
#ifdef DEBUG
		DPR(4)("SD01: Controller %d, %d, returned an abnormal status: %x.\n",
			dk->dk_addr.sa_major, dk->dk_addr.sa_minor, 
			sbp->SCB.sc_status);
#endif
		sd01logerr(sbp, (struct job *) 0, 0x4dd13003);
		timeout(sd01retry, (caddr_t)sbp->SCB.sc_wd, drv_usectohz(LATER));
		return;
	}

/* Retry the job */
	sd01logerr(sbp, (struct job *) sbp->SCB.sc_wd, 0x4dd13002);
	sd01retry((struct job *) sbp->SCB.sc_wd);

#ifdef DEBUG
        DPR (2)("sd01intn: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01phyrw - Perform a physical read or write. - 0dd14000
 * %DESCRIPTION: 
	This function performs a physical read or write without
	regrad to the VTOC.   It is called by the \fIioctl\fR.  The
	argument for the \fIioctl\fR is  a pointer to a structure
	which indicates the physical block address and the address
	of the data buffer in which the data is to be transferred. 
	The mode indicates whether the buffer is located in user or
	kernel space.
 * %CALLED BY: sd01wrtimestamp, sd01ioctl, sd01open1
 * %SIDE EFFECTS: 
	 The requested physical sector is read or written. If the PD sector
	 or VTOC have been updated they will be read in-core on the next access 
	 to the disk (DKUP_VTOC set in sd01vtoc_ck). However, if the FDISK 
	 sector is updated it will be read in-core on the next open.
 * %RETURN VALUES: 
	Status is returned in the \fIretval\fR byte of the structure
	for driver routines only.
 * %END HEADER */
int
sd01phyrw(dk, dir, phyarg, mode)
register struct disk *dk;
long dir;
struct phyio *phyarg;
int mode;
{
	extern   void sd01done();
	register struct job *jp;
	register buf_t *bp, *bp1;
	int 	 size;			/* Size of the I/O request 	*/
	register char *ptr1, *ptr2;	/* Used for data copies 	*/
	int 	 i,			/* Index 			*/
		 ret_val = 0;		/* Return value			*/
	unsigned long	*proc_p;
	
#ifdef DEBUG
        DPR (1)("sd01phyrw: (dk=0x%x dir=0x%x mode=0x%x) ", dk, dir, mode);
        DPR (1)("phyarg: sectst=0x%x memaddr=0x%x datasz=0x%x ", phyarg->sectst, phyarg->memaddr, phyarg->datasz);
#endif

/* If the request is for the PD sector override the supplied sector parameter */
	if (dk->dk_pdsec.sanity == VALID_PD &&
		phyarg->sectst == dk->unixst + PDBLKNO && mode == SD01_USER)
	{
		if (phyarg->datasz > BLKSZ)
			phyarg->datasz = BLKSZ;
	}
	
	bp = &sd01_io_buf;
	while (bp->b_flags & B_BUSY)
	{
		sleep((caddr_t)bp, PRIBIO);
	}
	
	bp->b_flags  = B_BUSY;
	bp->b_bcount = BLKSZ;

	if (mode != SD01_KERNEL)
	{
		bp1 = geteblk();
		bp1->b_flags = B_BUSY;
	}
	
	phyarg->retval = 0;
	while(phyarg->datasz > 0)
	{
		size = phyarg->datasz > BLKSZ ? BLKSZ : phyarg->datasz;
		if ( mode == SD01_KERNEL)
		{
			bp->b_un.b_addr = (char *) phyarg->memaddr;
		}
		else
		{
			bp->b_un.b_addr = bp1->b_un.b_addr;
		}
		
		bp->b_bcount = size;
		jp = sd01getjob(dk); /* Job is returned by sd01done */
		jp->j_dk = dk;
		jp->j_bp = bp;
		jp->j_done = sd01done;
		jp->j_cont->SCB.sc_datapt = bp->b_un.b_addr;
		jp->j_cont->SCB.sc_datasz =  size;
		if (dir == V_WRABS)
		{
			bp->b_flags |= B_WRITE;
			jp->j_cont->SCB.sc_mode = SCB_WRITE;
		}
		else
		{
			bp->b_flags |= B_READ;
			jp->j_cont->SCB.sc_mode = SCB_READ;
		}

		drv_getparm(UPROCP, &proc_p);

		jp->j_cont->SCB.sc_cmdpt = SCM_AD(&jp->j_cmd.cm);
		sdi_translate(jp->j_cont, bp->b_flags, (caddr_t)proc_p);
		bp->b_blkno = phyarg->sectst;
		bp->b_flags &= ~B_DONE;
		if (dir == V_WRABS && mode != SD01_KERNEL)
		{			/* Copy the data from the user */
			if(copyin((caddr_t)phyarg->memaddr,bp->b_un.b_addr,size))
			{
				phyarg->retval = V_BADWRITE;
				ret_val=EFAULT;
				break;
			}
		}

		/*
		 * If over-writing the PD sector and VTOC, make sure the PD 
		 * sector and VTOC data is sane before updating the disk.
		 * Also, make sure the user is not destroying a mounted or 
		 * mirrored partition.
		 */
		if (dir == V_WRABS && phyarg->sectst == dk->unixst + PDBLKNO && 
		    mode == SD01_USER)
		{
			if (ret_val = sd01vtoc_ck(dk, bp, SD01_PBLK))
			{
				phyarg->retval = V_BADWRITE;
				break;
			}
		}
		
		sd01batch(dk);
		sd01strat1(jp, SD01_PBLK);
		sd01batch(dk);
		iowait(bp);
		if(bp->b_flags & B_ERROR)
		{			/* Fail the request */
			phyarg->retval = dir == V_RDABS ? V_BADREAD:V_BADWRITE;
			ret_val = bp->b_error;
			break;
		}
		
		/* Check if we are updating the fdisk table */
		if (dir == V_WRABS && phyarg->sectst == FDBLKNO 
			&& mode == SD01_USER)
		{	
			if (((struct mboot *)bp->b_un.b_addr)->signature 
			   != MBB_MAGIC) {
				dk->dk_state |= DKUP_VTOC;
				dk->dk_vtoc.v_sanity = 0;
			}
		}

		if(dir == V_RDABS && mode != SD01_KERNEL)	
		{			/* Update the requestor's buffer */
			if (copyout((caddr_t)bp->b_un.b_addr, (caddr_t)phyarg->memaddr, size))
			{
				phyarg->retval = V_BADREAD;
				ret_val=EFAULT;
				break;
			}
		}
		
		phyarg->memaddr += size;
		phyarg->datasz -= size;
		phyarg->sectst++;
	}
	
	bp->b_flags &= ~B_BUSY;
	wakeup((caddr_t)bp);

	if (mode != SD01_KERNEL)
		brelse(bp1);

#ifdef DEBUG
        DPR (2)("sd01phyrw: - exit(%d) ", ret_val);
#endif
	return(ret_val);
}

/* %BEGIN HEADER
 * %NAME: sd01cmd - Perform a SCSI command. - 0dd15000
 * %DESCRIPTION: 
	This funtion performs a SCSI command such as Mode Sense on
	the addressed disk. The op code indicates the type of job
	but is not decoded by this function. The data area is
	supplied by the caller and assumed to be in kernel space. 
	This function will sleep.
 * %CALLED BY: sd01wrtimestamp, sd01flt, sd01ioctl
 * %SIDE EFFECTS: 
	A Mode Sense command is added to the job queue and sent to
	the host adapter.
 * %RETURN VALUES:
	Zero is returned if the command succeeds. 
 * %END HEADER */
int
sd01cmd(dk, op_code, addr, buffer, size, length, mode)
register struct disk *dk;
char op_code;
unsigned int addr;			/* Address field of command 	*/
char *buffer;				/* Buffer for Mode Sense data 	*/
unsigned int size;			/* Size of the data buffer 	*/
unsigned int length;			/* Block length specified in CDB*/
unsigned short mode;			/* Direction of the transfer 	*/
{
	extern void sd01done();
	register struct job *jp;
	register struct scb *scb;
	register buf_t *bp;
	register struct scsi_iotime *stat;
	int spls;
	unsigned long *proc_p;

#ifdef DEBUG
        DPR (1)("sd01cmd: (dk=0x%x) ", dk);
#endif
	
	bp = &sd01_io_buf;
	while (bp->b_flags & B_BUSY)
	{
		sleep((caddr_t)bp, PRIBIO);
	}
	bp->b_flags = B_BUSY;
	
	stat = &dk->dk_stat;
	jp   = sd01getjob(dk);
	scb  = &jp->j_cont->SCB;
	
	bp->b_flags |= mode & SCB_READ ? B_READ : B_WRITE;
	bp->b_un.b_addr = (caddr_t) buffer;	/* not used in sd01intn */
	bp->b_bcount = size;
	bp->b_error = 0;
	
	jp->j_dk = dk;
	jp->j_bp = bp;
	jp->j_done = sd01done;
	
	if (op_code & 0x20) { /* Group 1 commands */
		register struct scm *cmd;

		cmd = &jp->j_cmd.cm;
		cmd->sm_op   = op_code;
		cmd->sm_lun  = dk->dk_addr.sa_lun;
		cmd->sm_res1 = 0;
		cmd->sm_addr = addr;
		cmd->sm_res2 = 0;
		cmd->sm_len  = length;
		cmd->sm_cont = 0;

		scb->sc_cmdpt = SCM_AD(cmd);
		scb->sc_cmdsz = SCM_SZ;
	}
	else {	/* Group 0 commands */
		register struct scs *cmd;

		cmd = &jp->j_cmd.cs;
		cmd->ss_op    = op_code;
		cmd->ss_lun   = dk->dk_addr.sa_lun;
		cmd->ss_addr1 = ((addr & 0x1F0000) >> 16);
		cmd->ss_addr  = (addr & 0xFFFF);
		cmd->ss_len   = length;
		cmd->ss_cont  = 0;

		scb->sc_cmdpt = SCS_AD(cmd);
		scb->sc_cmdsz = SCS_SZ;
	}
	
	drv_getparm(UPROCP, &proc_p);

	scb->sc_int = sd01intn;
	scb->sc_dev = dk->dk_addr;
	scb->sc_datapt = buffer;
	scb->sc_datasz = size;
	scb->sc_mode = mode;
	scb->sc_resid = 0;
	scb->sc_time = JTIME;
	scb->sc_wd = (long) jp;
	sdi_translate(jp->j_cont, bp->b_flags,(caddr_t)proc_p);
	dk->dk_count++;
	stat->ios.io_ops++;
	stat->ios.io_misc++;
	stat->ios.io_qcnt++;
	
/* Add job to the queue at the end of the queue and batch the queue */
	spls = spl5();
	jp->j_forw = (struct job *) dk;
	jp->j_back = dk->dk_back;
	dk->dk_back->j_forw = jp;
	dk->dk_back = jp;
	if (dk->dk_next == (struct job *) dk)
		dk->dk_next = jp;
	dk->dk_batch = (struct job *) dk;
	
	sd01send(dk);
	splx(spls);
	iowait(bp);

	bp->b_flags &= ~B_BUSY;
	wakeup((caddr_t)bp);
		
#ifdef DEBUG
	if (bp->b_flags & B_ERROR)
        	DPR (2)("sd01cmd: - exit(%d) ", bp->b_error);
	else
        	DPR (2)("sd01cmd: - exit(0) ");
#endif
	if (bp->b_flags & B_ERROR)
		return (bp->b_error);
	else
		return(0);
}

/* %BEGIN HEADER
 * %NAME: sd01wrtimestamp - Write the disk time stamp. - 0dd18000
 * %DESCRIPTION: 
	This function is used to update the time stamps in the VTOC on disk.
	The time stamp will not be effected if the disk or the partition is
	write protected.  The function will fail if the write fail or the
	VTOC is undefined. 
 * %CALLED BY: sd01strategy, mdstrategy, mdmirror, mdclose
 * %SIDE EFFECTS: 
	The time stamp is set to \fItime\fR and the VTOC and PD info sector 
	is written to disk.
 * %RETURN VALUES: 
	.VL 4
	.LI "-1. "
	The VTOC could not be written.
	.LI " 0. "
	The time stamp was updated normally.
	.LE
 * %END HEADER */
int
sd01wrtimestamp(dk, part, time)
register struct disk *dk;
register int part;
time_t time;
{
	struct 	 phyio arg;
	register char *ptr1, *ptr2;	/* Used for data copies 	*/
	unsigned voffset;
	caddr_t  secbuf;
	int 	 i;

	if (dk->dk_vtoc.v_sanity != VTOC_SANE)
		return(-1);
		
	if (dk->dk_vtoc.v_part[part].p_flag & V_RONLY)
		return(0);

	/*
	*  Disktd has been asked to update the timestamp, make sure that
	*  if the partition is mirrored, set the flag so that a resulting
	*  error does not kill the job!
	*  Once the flag is set, it must be cleared before returning.
	*/
	if (dk->dk_part_flag[part] & DKONLY)
	{
		while(dk->dk_state & DKTSMD)
			sleep((caddr_t)&dk->dk_state, PRIBIO);
		dk->dk_state |= DKTSMD;
	}
	
	/* Allocate temporary memory for the PD/VTOC sector */
	if((secbuf = kmem_alloc(BLKSZ, KM_SLEEP)) == NULL)
		return(-1);

	/* Update the sector buffer PD info */
	ptr1 = (char *) &dk->dk_pdsec;
	ptr2 = secbuf;
	for(i = 0; i < sizeof(dk->dk_pdsec); i++)
		*ptr2++ = *ptr1++;

	/* Update the time stamp */
	dk->dk_vtoc.timestamp[part] = time;
	
	/* Update the sector buffer VTOC */
        voffset = dk->dk_pdsec.vtoc_ptr & (BLKSZ-1);
	ptr1 = (char *) &dk->dk_vtoc;
	ptr2 = secbuf + voffset;
	for(i = 0; i < sizeof(dk->dk_vtoc); i++)
		*ptr2++ = *ptr1++;

	/* Write the time stamp */
	arg.sectst  = dk->unixst + PDBLKNO;
	arg.memaddr = (long) secbuf;
	arg.datasz  = BLKSZ;
	sd01phyrw(dk, V_WRABS, &arg, SD01_KERNEL);

	/* Clear the update timestamp for a mirrored partition flag */
	dk->dk_state &= ~DKTSMD;
	wakeup((caddr_t)&dk->dk_state);
	kmem_free(secbuf, BLKSZ);
	
	if (arg.retval)			/* Check if the write was done */
		return(-1);
	else
		return(0);
}

/* %BEGIN HEADER
 * %NAME: sd01rdtimestamp - Read the partition's time stamp. - 0dd18000
 * %DESCRIPTION: 
	This function and its error numbers are obsolete.
 * %END HEADER */

/* %BEGIN HEADER
 * %NAME: sd01ioctl - ioctl function for SCSI disk driver. - 0dd1a000
 * %DESCRIPTION: 
	This function provides a number of different functions for use by
	utilities. They are: physical read or write, and read or write
	physical descriptor sector.  
	.VL 10
	.LI "READ ABSOLUTE"
	The Absolute Read command is used to read a physical sector on the
	disk regardless of the VTOC.  The data is transferred into buffer
	specified by the argument structure.
	.LI "WRITE ABSOLUTE"
	The Absolute Write command is used to write a physical sector on the
	disk regardless of the VTOC.  The data is transferred from a buffer
	specified by the argument structure.
	.LI "PHYSICAL READ"
	The Physical Read command is used to read any size data block on the
	disk regardless of the VTOC or sector size.  The data is transferred 
	into buffer specified by the argument structure.
	.LI "PHYSICAL WRITE"
	The Physical Write command is used to write any size data block on the
	disk regardless of the VTOC or sector size.  The data is transferred 
	from a buffer specified by the argument structure.
	.LI "READ PD SECTOR"
	This function reads the physical descriptor sector on the disk.
	.LI "WRITE PD SECTOR"
	This function writes the physical descriptor sector on the disk.
	.LI "CONFIGURATION"
	The Configuration command is used by mkpart to reconfigure a drive.
	The driver will update the in-core disk configuration parameters.
	.LI "GET PARAMETERS"
	The Get Parameters command is used by mkpart and fdisk to get 
	information about a drive.  The disk parameters are transferred
	into the disk_parms structure specified by the argument.
	.LI "RE-MOUNT"
	The Remount command is used by mkpart to inform the driver that the 
	contents of the VTOC has been updated.  The driver will update the
	in-core copy of the VTOC on the next open of the device.
	.LI "PD SECTOR NUMBER"
	The PD sector number command is used by 386 utilities that need to
	access the PD and VTOC information. The PD and VTOC information
	will always be located in the 29th block of the UNIX partition.
	.LI "PD SECTOR LOCATION"
	The PD sector location command is used by SCSI utilities that need to
	access the PD and VTOC information. The absolute address of this
	sector is transferred into an integer specified by the argument.
	.LI "ELEVATOR"
	This Elevator command allows the user to enable or disable the
	use of the elevator algorithm.
	.LI "RESERVE"
	This Reserve command will reserve the addressed device so that no other
	initiator can use it.
	.LI "RELEASE"
	This Release command releases a device so that other initiators can
	use the device.
	.LI "RESERVATION STATUS"
	This Reservation Status command informs the host if a device is 
	currently reserved, reserved by another host or not reserved.
	.LE
 * %CALLED BY: Kernel
 * %SIDE EFFECTS: 
	The requested action is performed.
 * %RETURN VALUES: Error value 
 * %END HEADER */
int
sd01ioctl(dev, cmd, arg, mode, cred_p, rval_p)
dev_t dev;
register int cmd;
register int arg;
int mode;
struct cred *cred_p;
int *rval_p;			/* Success return value; not used here */
{
	register struct disk *dk;
	int	state;			/* Device's RESERVE Status 	*/
	struct 	phyio 	  phyarg;	/* Physical block(s) I/O	*/
	union 	io_arg 	  ioarg;
	struct 	absio 	  absarg;	/* Absolute sector I/O only	*/
	struct 	disk_parms  darg;
	dev_t 	pt_dev;			/* Pass through device number 	*/
	int 	ret_val = 0;

	extern 	void	sd01flt();
	
 	dk = Sd01_dp + DKINDEX(dev);

#ifdef DEBUG
        DPR (1)("sd01ioctl: (dev=0x%x cmd=0x%x, arg=0x%x) dk=0x%x ", dev, cmd, arg, dk);
#endif

        if (cmd == DEBUGFLG) {
#ifdef DEBUG
                register short  dbg;

		if (copyin ((char *)arg, Sd01_debug, 10) != 0)
			return(EFAULT);

                printf ("\nNew debug values :");

                for (dbg = 0; dbg < 10; dbg++) {
                        printf (" %d", Sd01_debug[dbg]);
                }
                printf ("\n");
#endif
                return(0);
        }

	switch(cmd)
	{
		case V_WRABS: 	/* Write absolute sector number */
		case V_PWRITE:	/* Write physical data block(s)	*/
		case V_PDWRITE:	/* Write PD sector only		*/

                	/* Make sure user has permission */
			if (ret_val = drv_priv(cred_p))
			{
				return(ret_val);
			}

                	/* Make sure vtoc version is supported */
			if (dk->dk_vtoc.v_sanity == VTOC_SANE && 
				dk->dk_vtoc.v_version != V_VERSION)
			{
				sd01insane_dsk(dk);
				return(EIO);
			}

                	/* Make sure partition is not mirrored */
			if (dk->dk_part_flag[DKSLICE(dev)] & DKONLY)
                        	return(EBUSY);


		case V_RDABS: 	/* Read absolute sector number 	*/
		case V_PREAD:	/* Read physical data block(s)	*/
		case V_PDREAD:	/* Read PD sector only		*/

			if (cmd == V_WRABS || cmd == V_RDABS) {
				if (copyin((caddr_t)arg, (caddr_t)&absarg, sizeof(absarg)))
					return(EFAULT);

				phyarg.sectst  = (unsigned long) absarg.abs_sec;
				phyarg.memaddr = (unsigned long) absarg.abs_buf;
				phyarg.datasz  = BLKSZ;

				ret_val = sd01phyrw(dk, cmd, &phyarg, SD01_USER);
			}
			else {
				if (copyin((caddr_t)arg, (caddr_t)&phyarg, sizeof(phyarg)))
					return(EFAULT);

				/* Assign PD sector address */
				if (cmd == V_PDREAD || cmd == V_PDWRITE)
				{
					phyarg.sectst = dk->unixst + PDBLKNO;
					cmd = cmd == V_PDREAD ? V_RDABS : V_WRABS;
				}
				else
					cmd = cmd == V_PREAD ? V_RDABS : V_WRABS;

				ret_val = sd01phyrw(dk, cmd, &phyarg, SD01_USER);
				/* Copy out return value to user */
                		if (copyout((caddr_t)&phyarg, (caddr_t)arg, sizeof(phyarg)))
                        		return(EFAULT);
			}

			return(ret_val);

		/* Change drive configuration parameters. */
		case V_CONFIG: {
			int part;

			if ((ret_val = drv_priv(cred_p)) != 0)
				return(ret_val);

			/* Make sure no other partitions are open or mirrored */
			/* (This also insures DKSLICE(dev) == 0) */
			for (part = 1; part < V_NUMPAR; part++) {
				if (dk->dk_part_flag[part] != DKFREE)
								return(EBUSY);
			}
            /* Make sure partition 0 is not mirrored */
			if (dk->dk_part_flag[0] & DKONLY)
                        	return(EBUSY);

			if (copyin((caddr_t)arg, (caddr_t)&ioarg, sizeof(ioarg)))
				return(EFAULT);

			/* Don't allow user to change sector size. */
			if (ioarg.ia_cd.secsiz != BLKSZ)
				return(EINVAL);

			/* Don't allow user to set dp_cyls to zero. */
			if (ioarg.ia_cd.ncyl == 0)
				return(EINVAL);

			dk->dk_parms.dp_heads   = ioarg.ia_cd.nhead;
			dk->dk_parms.dp_cyls    = ioarg.ia_cd.ncyl;
			dk->dk_parms.dp_sectors = ioarg.ia_cd.nsec;

			/* Indicate drive parms have been set */
			dk->dk_state |= DKPARMS; 
			break;
		}

		/* Get info about the current drive configuration */
		case V_GETPARMS: {
			struct disk_parms	dk_parms;
			int part = DKSLICE(dev);

			if((dk->dk_state & DKPARMS) == 0)
				return(ENXIO);

			dk_parms.dp_type     = DPT_SCSI_HD;
			dk_parms.dp_heads    = dk->dk_parms.dp_heads;
			dk_parms.dp_cyls     = dk->dk_parms.dp_cyls;
			dk_parms.dp_sectors  = dk->dk_parms.dp_sectors;
			dk_parms.dp_secsiz   = dk->dk_parms.dp_secsiz;
			dk_parms.dp_ptag     = dk->dk_vtoc.v_part[part].p_tag;
			dk_parms.dp_pflag    = dk->dk_vtoc.v_part[part].p_flag;
			dk_parms.dp_pstartsec= dk->dk_vtoc.v_part[part].p_start;
			dk_parms.dp_pnumsec  = dk->dk_vtoc.v_part[part].p_size;

                	if (copyout((caddr_t)&dk_parms, (caddr_t)arg, sizeof(dk_parms))) 
				return(EFAULT);
                	break;
		}

		/* Force read of vtoc on next open.*/
		case V_REMOUNT:	 {
           	register int    part;

           	/* Make sure user is root */
			if ((ret_val = drv_priv(cred_p)) != 0)
				return(ret_val);

           	/* Make sure no partitions other than 0 are open. */
			for (part=0; part < V_NUMPAR; part++)
			{
				if ((dk->dk_part_flag[part] != DKFREE) &&
				     part != 0)
					break;
			}

                	if (part != V_NUMPAR)
                        	return(EBUSY);

			/* We have made no other checks here. */
			dk->dk_state |= DKUP_VTOC;
			dk->dk_vtoc.v_sanity = 0;
                	break;

		}

		/* Tell user the block number of the pdinfo structure */
        	case V_PDLOC: {	
                	unsigned long   pdloc;

			/* Check if fdisk is sane */
			if((dk->dk_state & DKFDISK)) 
                		pdloc = PDBLKNO;
			else 
                        	return(ENXIO);

                	if (copyout((caddr_t)&pdloc, (caddr_t)arg, sizeof(pdloc)))
                        	return(EFAULT);
                	break;
		}

		/* Tell user where pdinfo structure is on the disk */
        	case SD_PDLOC: {	
                	unsigned long   pdloc;

			/* Check if fdisk is sane */
			if((dk->dk_state & DKFDISK))
                		pdloc = dk->unixst + PDBLKNO;
			else 
                        	return(ENXIO);

                	if (copyout((caddr_t)&pdloc, (caddr_t)arg, sizeof(pdloc)))
                        	return(EFAULT);
                	break;
		}

		case SD_ELEV:
			if ((long) arg)
				dk->dk_state |= DKEL_OFF;
			else
				dk->dk_state &= ~DKEL_OFF;
			break;

		case SDI_RESERVE:
			if (ret_val = sd01cmd(dk, SS_RESERV, 0, NULL, 0, 0, SCB_WRITE) == 0)
			{
				dk->dk_state |= DKRESERVE;
				dk->dk_state |= DKRESDEVICE;
				sdi_fltinit(&dk->dk_addr, sd01flt, dk);
			}
			break;

		case SDI_RELEASE:
			if (ret_val = sd01cmd(dk, SS_TEST, 0, (char *) 0, 0, 0, SCB_READ) == 0) {
				if (ret_val = sd01cmd(dk, SS_RELES, 0, NULL, 0, 0, SCB_WRITE) == 0)
				{
					dk->dk_state &= ~DKRESERVE;
					dk->dk_state &= ~DKRESDEVICE;
					dk->dk_state |= DKUP_VTOC;
					dk->dk_vtoc.v_sanity = 0;
					sdi_fltinit(&dk->dk_addr, NULL, 0);
				}
			}
			break;

		case SDI_RESTAT:
			if (dk->dk_state & DKRESERVE)
				state = 1;	/* Currently Reserved */
			else {
				if(sd01cmd(dk, SS_TEST, 0, (char *) 0, 0, 0, SCB_READ) == EBUSY)
					state = 2;	/* Reserved by another host*/
				else
					state = 0;	/* Not Reserved */
			}

			if (copyout((caddr_t)&state, (caddr_t)arg, 4))
				return(EFAULT);
			break;

		case B_GETTYPE:
			if (copyout((caddr_t)"scsi", 
				((struct bus_type *) arg)->bus_name, 5))
			{
				return(EFAULT);
			}
			if (copyout((caddr_t)"sd01", 
				((struct bus_type *) arg)->drv_name, 5))
			{
				return(EFAULT);
			}
			break;

		case B_GETDEV:
			sdi_getdev(&dk->dk_addr, &pt_dev);
			if (copyout((caddr_t)&pt_dev, (caddr_t)arg, sizeof(pt_dev)))
				return(EFAULT);
			break;	
		default:
			return(EINVAL);
	}

#ifdef DEBUG
        DPR (2)("sd01ioctl: - exit(%d) ", ret_val);
#endif
	return(ret_val);
}

/* %BEGIN HEADER
 * %NAME: sd01print - Print routine for the kernel. - 0dd1b000
 * %DESCRIPTION: 
	The function prints the name of the addressed disk unit along 
	with an error message provided by the kernel.
 * %CALLED BY: Kernel
 * %SIDE EFFECTS: None
 * %RETURN VALUES: None
 * %END HEADER */
int
sd01print(dev, str)
dev_t dev;
register char *str;
{
	char name[49];
	struct scsi_ad addr;

#ifdef DEBUG
        DPR (1)("sd01print: (dev=0x%x) ", dev);
#endif
	
	addr.sa_major = getmajor(dev);
	addr.sa_minor = getminor(dev);
	addr.sa_lun   = (Sd01_dp + DKINDEX(dev))->dk_addr.sa_lun;
	addr.sa_exlun = (Sd01_dp + DKINDEX(dev))->dk_addr.sa_exlun;
	addr.sa_fill  = (Sd01_dp + DKINDEX(dev))->dk_addr.sa_fill;
	sdi_name(&addr, name);
	cmn_err(CE_WARN, "%s, Unit %d, Partition %d:  %s", name, 
		addr.sa_lun, DKSLICE(dev), str);

#ifdef DEBUG
        DPR (2)("sd01print: - exit(0) ");
#endif
	return(0);
}

/* %BEGIN HEADER
 * %NAME: sd01batch - Start a new batch in the disk queue. - 0dd1c000
 * %DESCRIPTION: 
	This function set a new batch in the disk queue.  It is to 
	ensure that the following jobs which are sent will not be
	reordered with any currently pending jobs.  
 * %CALLED BY: mdrestore
 * %SIDE EFFECTS: A new batch group is setup.
 * %RETURN VALUES: None
 * %END HEADER */
void
sd01batch(dk)
register struct disk *dk;
{
	dk->dk_batch = (struct job *) dk;
	dk->dk_state ^= DKDIR;
}

/* %BEGIN HEADER
 * %NAME: sd01insane_dsk - Log an error for an insane PD sector or VTOC. - 0dd1f000
 * %DESCRIPTION: 
	This function prints and logs an error when some attempts to 
	access a disk which does not have a valid PD sector or VTOC.
 * %CALLED BY: sd01strategy, sd01open1, sd01ioctl
 * %SIDE EFFECTS: None
 * %RETURN VALUES: None
 * %ERROR 6dd1f001
	The physical descriptor sector is bad on the addressed disk.
	The disk must be formated before it can be accessed for 
	normal use. See format(1M).
 * %END ERROR
 * %ERROR 6dd1f002
	The Volume Table of Contents is bad on the addressed disk. The 
	UNIX system must be partitioned before it can be accessed for normal 
	use. See mkpart(1M) and edvtoc(1M).
 * %END ERROR
 * %ERROR 6dd1f003
	The Volume Table of Contents version on the addressed disk is not 
	supported. The disk must be re-formated and re-partitioned before 
	it can be accessed for normal use. See scsiformat(1M), mkpart(1M)
	and edvtoc(1M).
 * %END ERROR
 * %ERROR 6dd1f004
	The Partition Table is bad on the addressed disk. The disk must 
	be partitioned before it can be accessed for normal use. See fdisk(1M).
 * %END ERROR
 * %END HEADER */
void
sd01insane_dsk(dk)
register struct disk *dk;
{
	char name[46];
	dev_t pt_dev;

#ifdef DEBUG
        DPR (1)("sd01insane_dsk: (dk=0x%x) ",dk);
#endif
	
	sdi_name(&dk->dk_addr, name);
	sdi_getdev(&dk->dk_addr, &pt_dev);
	
	/* Check if fdisk has been initialized */
	if(!(dk->dk_state & DKFDISK)) {
		cmn_err(CE_WARN, "SD01: %s Unit %d, Err: 6dd1f004\n",
			 name, dk->dk_addr.sa_lun);
		cmn_err(CE_CONT, "Disk does not have a sane Partition Table.\n");
		sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, PDBLKNO, BLKSZ,
			&dk->dk_stat.ios, 0x6dd1f004, 0, 0);
	}
	/* Check if PD sector is sane */
	else if (dk->dk_pdsec.sanity != VALID_PD)
	{
		cmn_err(CE_WARN, "SD01: %s Unit %d, Err: 6dd1f001\n",
			 name, dk->dk_addr.sa_lun);
		cmn_err(CE_CONT, "Disk does not have a sane Physical Descriptor sector.\n");
		sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, PDBLKNO, BLKSZ,
			&dk->dk_stat.ios, 0x6dd1f001, 0, 0);
	}
	/* Check if VTOC version is supported */
	else if (dk->dk_vtoc.v_sanity == VTOC_SANE && dk->dk_vtoc.v_version != V_VERSION)
	{
		cmn_err(CE_WARN, "SD01: %s Unit %d, Err: 6dd1f003\n",
			 name, dk->dk_addr.sa_lun);
		cmn_err(CE_CONT, "Disk VTOC version is not supported.\n");
		sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, dk->dk_pdsec.logicalst,
		BLKSZ, &dk->dk_stat.ios, 0x6dd1f003, 0, 0);
	}
	/* VTOC must be insane */
	else {

		cmn_err(CE_WARN, "SD01: %s Unit %d, Err: 6dd1f002\n",
		 	name, dk->dk_addr.sa_lun);
		cmn_err(CE_CONT, "Disk does not have a sane VTOC.\n");
		sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, dk->dk_pdsec.logicalst,
			BLKSZ, &dk->dk_stat.ios, 0x6dd1f002, 0, 0);
	}

#ifdef DEBUG
        DPR (2)("sd01insane_dsk: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01vtoc_ck - Check that the VTOC will not be destroyed. - 0dd20000
 * %DESCRIPTION: 
	This function checks writes which might destroy the VTOC
	sector.  This is to prevent the user from accidently over
	writing the VTOC or PD sector with invalid data.  It will also
	invalidate the current copy of the VTOC so that the next
	access will read in both the VTOC and PD info. The user data must 
	be directly accessable when this function is called.
	If the partition argument is equal to 16 or greater the block address 
	is assumed to be physical (same as sd01strat1).
 * %CALLED BY: sd01strategy, sd01phyrw, mdstrategy, and mddone
 * %SIDE EFFECTS: The VTOC entry is updated.
 * %RETURN VALUES: 0 if the write is ok, else an error number.
 * %END HEADER */
int
sd01vtoc_ck(dk, bp, part)
register struct disk *dk;
register buf_t *bp;
int part;
{
	char  	*pt1, *pt2;
	struct	vtoc  *vtocptr;
	long    i, start, slice, voffset, vtocblk, blksz, offset, retval;

        voffset = dk->dk_pdsec.vtoc_ptr & (BLKSZ-1);
	slice   = DKSLICE(bp->b_edev);
	vtocblk = dk->unixst + VTBLKNO;
	blksz   = sizeof(dk->dk_pdsec) + sizeof(dk->dk_vtoc);
	retval  = 0;

	if (part < SD01_PBLK)			/* Logical address  */
		start = bp->b_blkno + dk->dk_vtoc.v_part[slice].p_start;
	else					/* Physical address */
		start = bp->b_blkno;

#ifdef DEBUG
        DPR (1)("sd01vtoc_ck: (dk=0x%x bp=0x%x) start=0x%x bcount=0x%x blkno=0x%x ", dk, bp, start, bp->b_bcount, bp->b_blkno);
#endif

	if (start > vtocblk || start < vtocblk 
	   && bp->b_bcount <= BLKSZ * (vtocblk - start)) {
		return(0);	/* We are not hitting the PD/VTOC sector */
	}
		
	if ((start < vtocblk && bp->b_bcount > BLKSZ * (vtocblk-start)) &&
		(bp->b_bcount < BLKSZ * (vtocblk-start) + blksz) ||
		(start == vtocblk && bp->b_bcount < blksz)) {
		return(EACCES);	/* Must write the entire PD/VTOC sector */
	}
		
	/* Check in-core VTOC sanity */
	if (dk->dk_vtoc.v_sanity != VTOC_SANE)
	{
		/* Update new VTOC and PD sector on next access. */
		dk->dk_state |= DKUP_VTOC;
		dk->dk_vtoc.v_sanity = 0;
		return(0);	/* Let user overwrite bad in-core copy. */
	}

	/* Check PD info sanity */
	offset = (char *) &dk->dk_pdsec.sanity - (char *) &dk->dk_pdsec;
	pt1 = (char *) &dk->dk_pdsec.sanity;
	pt2 = bp->b_un.b_addr + (BLKSZ*(vtocblk-start)) + offset;

	for (i = 0; i < sizeof(dk->dk_pdsec.sanity); i++)
	{
		if (*pt1++ != *pt2++) {
			return(EACCES);	/* The new one is not sane. */
		}
	}

	/* Check new VTOC sanity */
	offset = (char *) &dk->dk_vtoc.v_sanity - (char *) &dk->dk_vtoc;
	pt1 = (char *) &dk->dk_vtoc.v_sanity;
	pt2 = bp->b_un.b_addr + (BLKSZ*(vtocblk-start)) + voffset + offset;

	for (i = 0; i < sizeof(dk->dk_vtoc.v_sanity); i++)
	{
		if (*pt1++ != *pt2++) {
			return(EACCES);	/* The new one is not sane */
		}
	}

	/* Check if the number of partitions is supported */
	pt2 = bp->b_un.b_addr + (BLKSZ*(vtocblk-start)) + voffset;

	if(((struct vtoc *)pt2)->v_nparts > V_NUMPAR)
		return(EACCES);

	/* Check for mount/mirror violations */
	if (retval = sd01part_ck(dk, bp, start))
	{
		return(retval);
	}

	/* We have passed all of the checks. */
	dk->dk_state |= DKUP_VTOC;
	dk->dk_vtoc.v_sanity = 0;

#ifdef DEBUG
        DPR (2)("sd01vtoc_ck: - exit(0) ");
#endif
	return(0);
}

/* %BEGIN HEADER
 * %NAME: sd01szsplit - Splits large transfers. - 0dd22000
 * %DESCRIPTION: 
	This function splits up large transfers so they do not
	use lots of resources.  Each piece is done separately.
 * %CALLED BY: sd01strategy, mdstrategy
 * %SIDE EFFECTS: None
 * %RETURN VALUES: None
 * %ERROR 0dd22001
	The disk driver can not allocate a raw buffer header.  This is 
	caused by insufficient virtual or physical memory.
 * %END ERROR
 * %END HEADER */
void
sd01szsplit(obp, strategy)
register buf_t *obp;
void (*strategy)();
{
	register buf_t *bp;
	register int count;
	
 	if ((bp = (buf_t *) getrbuf(0)) == 0)	{
 		obp->b_flags |= B_ERROR;
 		obp->b_error = EIO;
 		cmn_err(CE_WARN, "SD01: driver could not allocate buffer header. Err: 0dd22001");
 		iodone(obp);
 		return;
 	}
	*bp = *obp;
	count = obp->b_bcount;
	while(count > 0)
	{
		bp->b_bcount = count > SD01_MAXSIZE ? SD01_MAXSIZE : count;
		bp->b_flags &= ~B_DONE;
		strategy(bp);
		iowait(bp);
		if (bp->b_flags & B_ERROR)
		{
			obp->b_flags |= B_ERROR;
			obp->b_error = bp->b_error;
			break;
		}
		
		bp->b_un.b_addr += bp->b_bcount;
		bp->b_blkno += bp->b_bcount >> BLKSHF;
		count -= bp->b_bcount;
	}
	
	freerbuf(bp);
	iodone(obp);
}

/* %BEGIN HEADER
 * %NAME: sd01flt - Fault Handling routine called by HAD. - 0dd23000
 * %DESCRIPTION:
	This function is called by the host adapter driver when either
	a Bus Reset has occurred or a device has been closed after
	using Pass-Thru. This function begins a series of steps that
	ensure that the device is RESERVED before trying further jobs.
	If this function is called due to Pass-Thru, then the PD Sector
	and VTOC should be updated. This function cannot set the
	job pointer since you may be overwriting a valid job pointer.
 * %CALLED BY:  Host Adapter Driver
 * %SIDE EFFECTS:
	If due to Pass-Thru, PD sector and VTOC will be updated.
 * %RETURN VALUES: None.
 * %ERROR 8dd23001
	The HAD called this function due to a fault condition on the
	SCSI bus but passed an unknown parameter to this function.
 * %END ERROR
 * %END HEADER */
void
sd01flt(dk, flag)
struct	disk	*dk;	/* Points to Disk Structure */
long		flag;	/* Type of fault */
{
	dev_t	pt_dev;		/* Pass-Thru major/minor number */

	extern	void	sd01flts();

#ifdef DEBUG
        DPR (1)("sd01flt: (dk=0x%x flag=%d) ", dk, flag);
#endif

	switch (flag)
	{
	  case	SDI_FLT_RESET:		/* LU was reset */
		break;

	  case	SDI_FLT_PTHRU:		/* Pass-Thru was used */
		dk->dk_state |= DKUP_VTOC;
		dk->dk_vtoc.v_sanity = 0;

		/*
		*  If the device had previously been RESERVED,
		*  then try to re-RESERVE it.
		*/
		if (dk->dk_state & DKRESDEVICE)
		{
			/*
			*  If the RESERVE fails, then the gauntlet will
			*  have been entered and the appropriate message
			*  will have been printed.
			*/
			if (sd01cmd(dk, SS_RESERV, 0, NULL, 0, 0, SCB_WRITE) == 0)
			{
				dk->dk_state |= DKRESERVE;
				sdi_fltinit(&dk->dk_addr, sd01flt, dk);
			}
		}
		return;

	  default:			/* Unknown type */
		cmn_err(CE_WARN, "SD01: Bad type from host adapter! Err: 8dd23001");
		sdi_getdev(&dk->dk_addr, &pt_dev);
		sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, 0, 0, &dk->dk_stat.ios, 0x8dd23001, 0, 0);
		return;
	}

	/* Go on to next step */
	sd01flts(dk);

#ifdef DEBUG
        DPR (2)("sd01flt: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01flts - Common start point for handling faults. - 0dd24000
 * %DESCRIPTION:
	This is the beginning of the 'gauntlet'. All faults must
	come thru this function in case something caused the device
	to become released. Before any corrective action can be taken,
	the device must be reserved again. A suspend SFB is sent to HAD
	to stop any further jobs to the device. The interrupt routine
	will handle the next step of the gauntlet.
 * %CALLED BY: sd01flt, sd01intn
 * %SIDE EFFECTS:
	A suspend SFB is issued for this device.
 * %RETURN VALUES: None.
 * %ERROR 8dd24001
	The HAD rejected a Function request to Suspend the LU queue
	for the current device. The disk driver cannot proceed with the
	handling of the fault so the original I/O request will be failed.
 * %END ERROR
 * %END HEADER */
void
sd01flts(dk)
struct	disk *dk;
{
	dev_t 	pt_dev;			/* Pass-thru device number 	*/

#ifdef DEBUG
        DPR (1)("sd01flts: (dk=0x%x) ", dk);
#endif

	/* Initialize the RESERVE Reset counter for later in the gauntlet */
	dk->dk_rescnt = 0;

	/* If disk is already in the gauntlet, do nothing */
	if (dk->dk_state & DKFLT)
		return;
	
	dk->dk_state |= DKFLT;

	dk->dk_spcount = 0;
	dk->dk_fltsus->sb_type = SFB_TYPE;
	dk->dk_fltsus->SFB.sf_int = sd01ints;
	dk->dk_fltsus->SFB.sf_func = SFB_SUSPEND;
	sdi_getdev(&dk->dk_addr, &pt_dev);
	dk->dk_fltsus->SFB.sf_dev = dk->dk_addr;

	if (sdi_icmd(dk->dk_fltsus) != SDI_RET_OK)
	{
		cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd24001");
		sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, 0, 0, &dk->dk_stat.ios, 0x8dd24001, 0, 0);

		/* Clean up after the job */
		sd01flterr(dk, 0);
	}

#ifdef DEBUG
        DPR (2)("sd01flts: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01ints - Interrupt Handler for SUSPEND SFB jobs. -  0dd25000
 * %DESCRIPTION:
	This function is called by the Host Adapter Driver when the
	SUSPEND function has completed. If it failed, retry the job until
	the retry limit is exceeded. Once the disk queue is suspended,
	send a Request Sense job to find out what happened to the device.
 * %CALLED BY:  Host Adapter Driver
 * %SIDE EFFECTS:  Disk queue SUSPEND flag will be set. A Request Sense
	job will have been started.
 * %RETURN VALUES:  None.
 * %ERROR 4dd25001
	The SCSI disk driver retried a Function request. The retry
	was performed because the HAD detected an error.
 * %END ERROR
 * %ERROR 6dd25002
	The HAD detected an error with the SUSPEND function request
	and the retry count has already been exceeded.
 * %END ERROR
 * %ERROR 8dd25003
	The HAD rejected a Request Sense job from the SCSI disk
	driver. The original job will also be failed.
 * %END ERROR
 * %END HEADER */
void
sd01ints(sbp)
struct	sb *sbp;	/* SFB */
{
	register struct disk *dk;
	dev_t	dev;			/* External device number 	*/
	dev_t 	pt_dev;			/* Pass-thru device number 	*/

	extern void	sd01intrq();
	extern void	sd01flterr();


	dev = makedevice(sbp->SFB.sf_dev.sa_major, sbp->SFB.sf_dev.sa_minor);
	dk  = Sd01_dp + DKINDEX(dev);

#ifdef DEBUG
        DPR (1)("sd01ints: (sbp=0x%x) dk=0x%x ", sbp, dk);
#endif

	if (sbp->SFB.sf_comp_code & SDI_RETRY && dk->dk_spcount < SD01_RETRYCNT)
	{
		/* Retry the Suspend SFB */
		dk->dk_spcount++;
		dk->dk_error++;
		sd01logerr(sbp, (struct job *) NULL, 0x4dd25001);
		if (sdi_icmd(sbp) != SDI_RET_OK)
		{
			cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd25003");
			sdi_getdev(&dk->dk_addr, &pt_dev);
			sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, 0, 0, &dk->dk_stat.ios, 0x8dd25003, 0, 0);
			sd01flterr(dk, 0);
		}
		return;
	}

	if (sbp->SFB.sf_comp_code != SDI_ASW)
	{
		sd01logerr(sbp, (struct job *) NULL, 0x6dd25002);
		sd01flterr(dk, 0);
		return;
	}

	/*
	*  The device is now SUSPENDED. Start the Request Sense Job.
	*/
	dk->dk_state |= DKSUSP;

	dk->dk_spcount = 0;
	dk->dk_fltreq->sb_type = ISCB_TYPE;
	dk->dk_fltreq->SCB.sc_int = sd01intrq;
	dk->dk_fltreq->SCB.sc_cmdsz = SCS_SZ;
	dk->dk_fltreq->SCB.sc_link = 0;
	dk->dk_fltreq->SCB.sc_resid = 0;
	dk->dk_fltreq->SCB.sc_time = JTIME;
	dk->dk_fltreq->SCB.sc_mode = SCB_READ;
	dk->dk_fltreq->SCB.sc_dev = sbp->SFB.sf_dev;
	dk->dk_fltcmd.ss_op = SS_REQSEN;
	dk->dk_fltcmd.ss_lun = sbp->SFB.sf_dev.sa_lun;
	dk->dk_fltcmd.ss_addr1 = 0;
	dk->dk_fltcmd.ss_addr  = 0;
	dk->dk_fltcmd.ss_len = SENSE_SZ;
	dk->dk_fltcmd.ss_cont = 0;
	dk->dk_sense.sd_key = SD_NOSENSE; /* Clear old sense key */

	if (sdi_icmd(dk->dk_fltreq) != SDI_RET_OK)
	{

		cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd25003");
		sdi_getdev(&dk->dk_addr, &pt_dev);
		sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, 0, 0,
			&dk->dk_stat.ios, 0x8dd25003, 0, 0);
		sd01flterr(dk, 0);
		return;
	}

#ifdef DEBUG
        DPR (2)("sd01ints: - exit ");
#endif
}

/* BEGIN HEADER
 * %NAME: sd01intrq - Request Sense Interrupt handler. - 0dd26000
 * %DESCRIPTION:
	This function is called by the host adapter driver when a
	Request Sense job completes. This function will not examine the
	Sense data because there is still one more step before a normal
	I/O job can be restarted. Send the RESERVE command to the device
	to prevent some other host from using the device.
 * %CALLED BY: Host Adapter Driver
 * %SIDE EFFECTS:
	Either the Request Sense will be retried or the RESERVE command
	is sent to the device.
 * %RETURN VALUES: None
 * %ERROR 4dd26001
	The SCSI disk driver retried a Request Sense job that the
	HAD failed.
 * %END ERROR
 * %ERROR 8dd26002
	The HAD rejected a Request Sense job issued by the SCSI disk driver.
	The original job will also be failed.
 * %END ERROR
 * %ERROR 6dd26003
	The HAD detected an error in the last Request Sense job issued by the
	SCSI disk driver. The retry count has been exceeded so the original
	I/O request will also be failed.
 * %END ERROR
 * %ERROR 8dd26004
	The HAD rejected a Reserve job requested by the SCSI disk
	driver. The original job will also be failed.
 * %END ERROR
 * %END HEADER */
void
sd01intrq(sbp)
struct sb *sbp;		/* SCB */
{
	register struct	disk *dk;
	dev_t	 dev;			/* External device number 	*/

	dev = makedevice(sbp->SCB.sc_dev.sa_major, sbp->SCB.sc_dev.sa_minor);
	dk  = Sd01_dp + DKINDEX(dev);

#ifdef DEBUG
        DPR (1)("sd01intrq: (sbp=0x%x) ", sbp);
        DPR (6)("sd01intrq: (sbp=0x%x) ", sbp);
#endif

	if (sbp->SCB.sc_comp_code != SDI_CKSTAT &&
	    sbp->SCB.sc_comp_code & SDI_RETRY &&
	    dk->dk_spcount <= SD01_RETRYCNT)
	{
		dk->dk_spcount++;
		dk->dk_error++;
		sbp->SCB.sc_time = JTIME;
		sd01logerr(sbp, (struct job *) NULL, 0x4dd26001);

		if (sdi_icmd(sbp) != SDI_RET_OK)
		{

			cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd26002");
			sdi_getdev(&dk->dk_addr, &sd01pt_dev);
			sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), 0, 0, 0,
				&dk->dk_stat.ios, 0x8dd26002, 0, 0);
			sd01flterr(dk, 0);
		}
		return;
	}

	if (sbp->SCB.sc_comp_code != SDI_ASW)
	{
		dk->dk_error++;
		sd01logerr(sbp, (struct job *) NULL, 0x6dd26003);
		sd01flterr(dk, 0);
		return;
	}


	/*
	*  The sc_wd field must have been filled in when the
	*  fault was first detected by either sd01flt or sd01intn.
	*  It indicates if there is a real job associated with this fault!
	*/
	dk->dk_spcount = 0;
	dk->dk_fltres->sb_type = ISCB_TYPE;
	dk->dk_fltres->SCB.sc_int = sd01intres;
	dk->dk_fltres->SCB.sc_cmdsz = SCS_SZ;
	dk->dk_fltres->SCB.sc_datapt = NULL;
	dk->dk_fltres->SCB.sc_datasz = 0;
	dk->dk_fltres->SCB.sc_link = 0;
	dk->dk_fltres->SCB.sc_resid = 0;
	dk->dk_fltres->SCB.sc_time = JTIME;
	dk->dk_fltres->SCB.sc_mode = SCB_WRITE;
	dk->dk_fltres->SCB.sc_dev = sbp->SCB.sc_dev;
	dk->dk_fltcmd.ss_op = SS_RESERV;
	dk->dk_fltcmd.ss_lun = sbp->SCB.sc_dev.sa_lun;
	dk->dk_fltcmd.ss_addr1 = 0;
	dk->dk_fltcmd.ss_addr  = 0;
	dk->dk_fltcmd.ss_len = 0;
	dk->dk_fltcmd.ss_cont = 0;

	/*
	*  If the device is not suppose to be reserved,
	*  then go directly to the function to restart the original job.
	*  Put the check here so that the SB is initialized.
	*  Some of its data will be used even if no RESERVE is issued.
	*/
	if ((dk->dk_state & DKRESDEVICE) == 0)
	{
		sd01fltjob(dk);
		return;
	}

	if (sdi_icmd(dk->dk_fltres) != SDI_RET_OK)
	{

		cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd26004");
		sdi_getdev(&dk->dk_addr, &sd01pt_dev);
		sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), 0, 0, 0,
			&dk->dk_stat.ios, 0x8dd26004, 0, 0);
		sd01flterr(dk, 0);
	}

#ifdef DEBUG
        DPR (2)("sd01intrq: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01intres - Interrupt Handler for RESERVE device jobs -  0dd27000
 * %DESCRIPTION:
	This function is called by the host adapter driver when a RESERVE
	job completes. The job will be retried if it failed.
	If a RESET or CRESET prevented the RESERVE from completing, then
	the gauntlet must be started again. The SUSPEND job need not be redone
	since the queue is already suspended so go back to the Request Sense.
	If the SUSPEND succeeded, then if there was a real I/O job in progress
	the Request Sense data read previously will be examined to determine
	what to do.  Clear the disk fault flag that indicates this disk is in
	the gauntlet.
 * %CALLED BY: Host Adapter Driver
 * %SIDE EFFECTS: Either the RESERVE will be retried, the gauntlet will
 	be restarted, or an I/O job will be restarted.
 * %RETURN VALUES: None.
 * %ERROR 8dd27001
	The HAD rejected a Request Sense job issued by the SCSI disk
	driver. The original job will also be failed.
 * %END ERROR
 * %ERROR 8dd27002
	The HAD rejected a Reserve job issued by the SCSI disk driver.
	The original job will also be failed.
 * %END ERROR
 * %ERROR 6dd27003
	The HAD detected a failure in the last Reserve job issued
	by the SCSI disk driver. The retry count has been exceeded
	so the original job has been failed.
 * %END ERROR
 * %ERROR 4dd27004
	The SCSI disk driver is retrying an I/O request because of an error
	detected by the target controller. The cause of the error is
	indicated by the second and third error codes. These error
	codes are the sense key and extended sense code respectively.
	See the disk target controller code for more information.
	(OBSOLETE ERROR)
 * %END ERROR
 * %ERROR 4dd27005
	The disk controller performed retry or ECC which was
	successful. The cause of the error is indicated by the second
	and third error codes. There error codes are the sense key and
	extended sense code respectively. See the disk target controller
	codes for more information.  (OBSOLETE ERROR)
 * %END ERROR
 * %ERROR 6dd27006
	The RESERVE command caused the bus to reset and has exceeded
	its maximum retry count. The original job will be failed and the
	error handling code will be exited.
 * %END ERROR
 * %END HEADER */
void
sd01intres(sbp)
struct	sb *sbp;	/* SCB */
{
	struct	disk *dk;
	dev_t	 dev;			/* External device number 	*/

	dev = makedevice(sbp->SCB.sc_dev.sa_major, sbp->SCB.sc_dev.sa_minor);
	dk  = Sd01_dp + DKINDEX(dev);

#ifdef DEBUG
        DPR (1)("sd01intres: (sbp=0x%x) ", sbp);
        DPR (6)("sd01intres: (sbp=0x%x) ", sbp);
#endif

	if (sbp->SCB.sc_comp_code & SDI_RETRY && dk->dk_spcount <= SD01_RETRYCNT)
	{
		if (sbp->SCB.sc_comp_code == SDI_RESET ||
		    sbp->SCB.sc_comp_code == SDI_CRESET ||
		    (sbp->SCB.sc_comp_code == SDI_CKSTAT &&
		     sbp->SCB.sc_status == S_CKCON))
		{
			/*
			*  Must restart the gauntlet!
			*  The queue has already been SUSPENDED, so go
			*  back and do the Request Sense job.
			*/
			if (sbp->SCB.sc_comp_code == SDI_CRESET && dk->dk_rescnt > SD01_RST_ERR)
			{
				/* This job has caused to many resets */
				sd01logerr(sbp, (struct job *) NULL, 0x6dd27006);
				sd01flterr(dk, 0);
				return;
			}

			dk->dk_rescnt++;
			dk->dk_spcount = 0;
			dk->dk_fltreq->sb_type = ISCB_TYPE;
			dk->dk_fltreq->SCB.sc_int = sd01intrq;
			dk->dk_fltreq->SCB.sc_cmdsz = SCS_SZ;
			dk->dk_fltreq->SCB.sc_link = 0;
			dk->dk_fltreq->SCB.sc_resid = 0;
			dk->dk_fltreq->SCB.sc_time = JTIME;
			dk->dk_fltreq->SCB.sc_mode = SCB_READ;
			dk->dk_fltreq->SCB.sc_dev = sbp->SCB.sc_dev;
			dk->dk_fltcmd.ss_op = SS_REQSEN;
			dk->dk_fltcmd.ss_lun = sbp->SCB.sc_dev.sa_lun;
			dk->dk_fltcmd.ss_addr1 = 0;
			dk->dk_fltcmd.ss_addr  = 0;
			dk->dk_fltcmd.ss_len = SENSE_SZ;
			dk->dk_fltcmd.ss_cont = 0;
			dk->dk_sense.sd_key = SD_NOSENSE; /* Clear old sense key */

			if (sdi_icmd(dk->dk_fltreq) != SDI_RET_OK)
			{

				cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd27001");
				sdi_getdev(&dk->dk_addr, &sd01pt_dev);
				sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), 0, 0, 0,
					&dk->dk_stat.ios, 0x8dd27001, 0, 0);
				sd01flterr(dk, 0);
			}
			return;
		}
		else 	/* Not RESET or CRESET */
		{
			dk->dk_fltres->sb_type = ISCB_TYPE;
			dk->dk_fltres->SCB.sc_int = sd01intres;
			dk->dk_fltres->SCB.sc_cmdsz = SCS_SZ;
			dk->dk_fltres->SCB.sc_link = 0;
			dk->dk_fltres->SCB.sc_resid = 0;
			dk->dk_fltres->SCB.sc_time = JTIME;
			dk->dk_fltres->SCB.sc_mode = SCB_WRITE;
			dk->dk_fltres->SCB.sc_dev = sbp->SCB.sc_dev;
			dk->dk_fltcmd.ss_op = SS_RESERV;
			dk->dk_fltcmd.ss_lun = sbp->SCB.sc_dev.sa_lun;
			dk->dk_fltcmd.ss_addr1 = 0;
			dk->dk_fltcmd.ss_addr  = 0;
			dk->dk_fltcmd.ss_len = 0;
			dk->dk_fltcmd.ss_cont = 0;
			dk->dk_sense.sd_key = 0;

			dk->dk_spcount++;
			dk->dk_error++;
		
			if (sdi_icmd(dk->dk_fltres) != SDI_RET_OK)
			{

				cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd27002");
				sdi_getdev(&dk->dk_addr, &sd01pt_dev);
				sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), 0, 0, 0,
					&dk->dk_stat.ios, 0x8dd27002, 0, 0);
				sd01flterr(dk, 0);
			}
			return;
		}
	}

	if (sbp->SCB.sc_comp_code != SDI_ASW)
	{
		dk->dk_error++;
		sd01logerr(sbp, (struct job *) NULL, 0x6dd27003);
		sd01flterr(dk, 0);
		return;
	}

	/* RESERVE Job completed ASW */
	dk->dk_state |= DKRESERVE;
	sd01fltjob(dk);

#ifdef DEBUG
        DPR (2)("sd01intres: - exit ");
#endif
}

/* BEGIN HEADER
 * %NAME: sd01resume - Resume a suspended disk queue - 0dd28000
 * %DESCRIPTION:
	This function is called only if a queue has been suspended and must
	now be resumed. It is called by sd01comp1 when a job has been
	failed and a disk queue must be resumed or by sdflterr when there
	is no job to fail but the queue needs to be resumed anyway.
 * %CALLED BY: sd01comp1, sd01flterr,
 * %SIDE EFFECTS: THe LU queue will have been resumed.
 * %RETURN VALUES: None.
 * %ERROR 8dd28001
	The HAD rejected a Resume function request by the SCSI disk driver.
	This is caused by a parameter mismatch within the driver.
	The system should be rebooted.
 * %END ERROR
 * %END HEADER */
void
sd01resume(dk)
struct disk *dk;
{
	dev_t	pt_dev;		/* Pass-thru information */
	extern void	sd01intf();

#ifdef DEBUG
        DPR (1)("sd01resume: (dk=0x%x) ", dk);
#endif

	dk->dk_spcount = 0;	/* Reset special count */
	sd01_fltsbp->sb_type = SFB_TYPE;
	sd01_fltsbp->SFB.sf_int = sd01intf;
	sd01_fltsbp->SFB.sf_dev = dk->dk_addr;
	sd01_fltsbp->SFB.sf_func = SFB_RESUME;

	if (sdi_icmd(sd01_fltsbp) != SDI_RET_OK)
	{
		cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd28001");
		sdi_getdev(&dk->dk_addr, &pt_dev);
		sd01lognberr(getmajor(pt_dev), getminor(pt_dev), 0, 0, 0,
			&dk->dk_stat.ios, 0x8dd28001, 0, 0);
	}
	dk->dk_state &= ~DKSUSP;

#ifdef DEBUG
        DPR (2)("sd01resume: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01flterr - Clean up after an unrecoverable error - 0dd29000
 * %DESCRIPTION:
	This function is called in the gauntlet when an error
	occurs. If the Gauntlet was unable to re-RESERVE the device,
	then let the user know with the logged error.
	If there is a job waiting to be retried, call
	sd01comp1 to fail the job and resume the queue.
	Otherwise, just resume the queue. A separate function was
	made since this test will need to be made all thru the gauntlet.
	It is assumed that the error has already been logged before
	this function is called.
 * %CALLED BY: sd01intn, sd01flt, sd01flts, sd01ints, sd01intrq, sd01intres
 * %SIDE EFFECTS: None.
 * %RETURN VALUES: None.
 * %ERROR 6dd29001
	The SCSI Disk Driver was unable to re-RESERVE a device.
	Some hardware problem or the device was reserved by some
	other host is probably causing the driver to fail in its attempt
	to RESERVE the device.
 * %END ERROR
 * %END HEADER */
void
sd01flterr(dk, res_flag)
struct disk *dk;
int	res_flag;	/* Indicates if device is still RESERVED */
{
	register struct job *jp;

#ifdef DEBUG
        DPR (1)("sd01flterr: (dk=0x%x res_flag=%d) ", dk, res_flag);
#endif

	/*
	*  If the device was RESERVED but the gauntlet was unable to
	*  re-RESERVE the device, clear the RESERVE flag, let user
	*  know there was a problem and log the error.
	*  Also set the flag indicating that the device should be RESERVED
	*  the next chance it gets.
	*/
	if (dk->dk_state & DKRESERVE && res_flag == 0)
	{
		dk->dk_state &= ~DKRESERVE;

		sdi_name(&dk->dk_addr, sd01name);
		cmn_err(CE_WARN, "SD01: Device no longer RESERVED! Err: 6dd29001");
		cmn_err(CE_WARN, "%s, Unit = %d, Err: %x", sd01name, dk->dk_addr.sa_lun, 0x6dd29001);
		sdi_getdev(&dk->dk_addr, &sd01pt_dev);
		sd01lognberr(getmajor(sd01pt_dev), getminor(sd01pt_dev), 0, 0, 0, &dk->dk_stat.ios, 0x6dd29001, 0, 0);
	}

	/*
	*  The gauntlet is finished!
	*  Clear the state and reset the job pointer so that
	*  the next time the gauntlet is entered, it has been initialized
	*  to the proper state.
	*/
	jp = (struct job *)dk->dk_fltres->SCB.sc_wd;
#ifdef DEBUG
        DPR (2)("jp in comp 0x%x ",jp);
#endif
	dk->dk_fltres->SCB.sc_wd = NULL;
	dk->dk_state &= ~DKFLT;

	/* Is there a job to restart */
	if (jp == NULL)
		sd01qresume(dk);	/* No job */
	else
		sd01comp1(jp);		/* Return the job */

#ifdef DEBUG
        DPR (2)("sd01flterr: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01qresume - Checks if the SB for Resuming s LU queue is busy - 0dd2a000
 * %DESCRIPTION:
	This function will check if the SB used for resuming a LU queue
	is currently busy. If it is busy, the current disk is added to the
	end of the list of disks waiting for a resume to be issued.
	If the SB is not busy, this disk is put at the front of the
	list and the resume for this disk is started immediately.
 * %CALLED BY: sd01comp1, sd01flterr
 * %SIDE EFFECTS: A disk structure is added to the Resume queue.
 * %RETURN VALUES: None.
 * %END HEADER */
void
sd01qresume(dk)
struct disk *dk;
{

#ifdef DEBUG
        DPR (1)("sd01qresume: (dk=0x%x) ", dk);
#endif
	
	/* Check if the Resume SB is currently being used */
	if (sd01_resume.res_head == (struct disk *) &sd01_resume)
	{	/* Resume Q not busy */

		dk->dk_state |= DKONRESQ;
		sd01_resume.res_head = dk;
		sd01_resume.res_tail = dk;
		dk->dk_fltnext = (struct disk *) &sd01_resume;
		sd01resume(dk);
	}
	else
	{	/* Resume Q is Busy */

		/*
		*  This disk may already be on the Resume Queue.
		*  If it is, then set the flag to indicate that
		*  another Resume is pending for this disk.
		*/
		if (dk->dk_state & DKONRESQ)
		{
			dk->dk_state |= DKPENDRES;
		}
		else
		{	/* Not on Q, put it there */
			dk->dk_state |= DKONRESQ;
			sd01_resume.res_tail->dk_fltnext = dk;
			sd01_resume.res_tail = dk;
			dk->dk_fltnext = (struct disk *) &sd01_resume;
		}
	}
#ifdef DEBUG
        DPR (2)("sd01qresume: - exit ");
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01fltjob - Determine what to do with the original job - 0dd2b000
 * %DESCRIPTION:
	This function uses the Request Sense information to determine
	what to do with the original job. Of course, there may not be an
	original job if the gauntlet had been entered via the HAD bus
	reset entry point.
 * %CALLED BY: sd01intrq, sd01intres
 * %SIDE EFFECTS: May restart the original job.
 * %RETURN VALUES: None.
 * %ERROR 4dd2b001
	The SCSI disk driver is retrying an I/O request because of an error
	detected by the target controller. The cause of the error is
	indicated by the second and third error codes. These error codes
	are the sense key and extended sense code respectively. See the
	disk target controller code for more information.
 * %END ERROR
 * %ERROR 4dd2b002
	The disk controller performed retry or ECC which was successful.
	The cause of the error is indicated by the second and third
	error codes. These error codes are the sense key and extended
	sense codes respectively. See the disk target controller codes
	for more information.
 * %END ERROR
 * %END HEADER */
void
sd01fltjob(dk)
struct disk *dk;
{
	struct job *jp;		/* Job structure to be restarted */
	struct sb  *osbp;	/* Original job SB pointer */

#ifdef DEBUG
        DPR (1)("sd01fltjob: (dk=0x%x) ", dk);
        DPR (6)("sd01fltjob: (dk=0x%x) ", dk);
#endif

	if ((jp = (struct job *) dk->dk_fltres->SCB.sc_wd) != NULL)
		osbp = jp->j_cont;	/* SB of a real job */
	else
		osbp = dk->dk_fltres;	/* No Job but still need an SB */

	dk->dk_sense.sd_ba = sdi_swap32(dk->dk_sense.sd_ba);

	/* Request Sense information */
	switch(dk->dk_sense.sd_key){
		case SD_NOSENSE:
		case SD_ABORT:
		case SD_VENUNI:
			sd01logerr(osbp, jp, 0x4dd2b001);

		case SD_UNATTEN: /* Don't log unit attention */

			/* Is there a real job to retry */
			if (jp !=  (struct job *) NULL)
			{
				
				/*
				*  If the job retry count or the reset count
				*  has exceeded it's limit, then fail the job.
				*  Otherwise try it again.
				*/
				if ((osbp->SCB.sc_comp_code == SDI_CRESET &&
				    dk->dk_jberr >= SD01_RST_ERR) ||
				    dk->dk_jberr >= SD01_RETRYCNT)
					sd01flterr(dk, DKRESERVE);
				else
				{
					/*
					*  Exit the gauntlet before
					*  retrying the original job.
					*/
					dk->dk_fltres->SCB.sc_wd = NULL;
					dk->dk_state &= ~DKFLT;
					sd01retry(jp);
				}
			}
			else
				/* No job to retry! Clean up as usual */
				sd01flterr(dk, DKRESERVE);
#ifdef DEBUG
        DPR (2)("sd01fltjob: - skey(0x%x) ", dk->dk_sense.sd_key);
#endif
			return;

		case SD_RECOVER:
			dk->dk_error++;
			if (Sd01log_marg) 
			{
				switch(dk->dk_sense.sd_sencode)
				{
			  	case SC_DATASYNCMK:
			  	case SC_RECOVRRD:
			  	case SC_RECOVRRDECC:
			  	case SC_RECOVRIDECC:
					/* Indicate marginal bad block found */
					dk->hde_state |= HDERECERR; 
					break;
			  	default:
					sd01logerr(osbp, jp, 0x4dd2b002);
				}
			}
			osbp->SCB.sc_comp_code = SDI_ASW;
			sd01flterr(dk, DKRESERVE);
#ifdef DEBUG
        DPR (2)("sd01fltjob: - skey(0x%x) ", dk->dk_sense.sd_key);
#endif
			return;

		case SD_MEDIUM:
			switch(dk->dk_sense.sd_sencode)
			{
			  case SC_IDERR:
			  case SC_UNRECOVRRD:
			  case SC_NOADDRID:
			  case SC_NOADDRDATA:
			  case SC_NORECORD:
			  case SC_DATASYNCMK:
			  case SC_RECOVRIDECC:
				/* Indicate actual bad block found */
				dk->hde_state |= HDEECCERR; 
			}
		default:
			dk->dk_error++;
			sd01flterr(dk, DKRESERVE);
#ifdef DEBUG
        DPR (2)("sd01fltjob: - skey(0x%x) ", dk->dk_sense.sd_key);
#endif
			return;
	}
}

/* %BEGIN HEADER
 * %NAME: sd01part_ck - Check that mounted/mirrored partitions do not change - 0dd2c000
 * %DESCRIPTION:
	Now that the Disk Driver uses the 'open type' field in sd01open()
	The driver can make further checks on the state of a partition before
	allowing a user to change the VTOC out from under: 1) a mounted
	partition, 2) a mirrored partition, by changing the starting address
	of a partition or by changing the size of a partition, or by changing
	the flag of a mounted partition to unmountable.
	This function should only be called when it is determined
	that a new VTOC is to be written.
 * %CALLED BY: sd01strategy, sd01phyrw, sd01vtoc_ck
 * %SIDE EFFECTS: none
 * %RETURN VALUES:
	Returns 0 if the requested change does not conflict with the
	current partitions and returns EBUSY if something was found.
	The errno will be set to busy for the error case to indicate to the
	user that the partition is already in use.
 * %END HEADER */
int
sd01part_ck(dk, bp, start)
register struct disk *dk;
register buf_t *bp;
register long	start;
{
	register int part;
	int	offset,
		voffset;
	int	i;
	long	vtocblk;
	char	*pt1, *pt2;
	ushort	flag = V_UNMNT;
	union {
		char	cstr[4];	/* Value is built a char at a time */
		long	val;		/* Used to examine the value */
	} psize;			/* Size of a partition */

#ifdef DEBUG
        DPR (1)("sd01part_ck: (dk=0x%x bp=0x%x) ", dk, bp);
#endif

        voffset = dk->dk_pdsec.vtoc_ptr & (BLKSZ-1);
	vtocblk = dk->unixst + VTBLKNO;

	/* Look for any partition that is mounted or mirrored */
	for (part=0; part < V_NUMPAR; part++)
	{
		if (dk->dk_part_flag[part] & DKMNT ||
		    dk->dk_part_flag[part] & DKONLY)
		{

			/* CHECK: p_start must be same! */
			offset = (char *) &dk->dk_vtoc.v_part[part].p_start - (char *) &dk->dk_vtoc;
			pt1 = (char *) &dk->dk_vtoc.v_part[part].p_start;
			pt2 = (char *) bp->b_un.b_addr + (BLKSZ*(vtocblk-start)) + voffset + offset;
			for (i=0; i < sizeof(dk->dk_vtoc.v_part[part].p_start); i++)
			{
				if (*pt1++ != *pt2++)
					return(EBUSY);
			}

			/* CHECK: p_size cannot change (except for root) */
			offset = (char *) &dk->dk_vtoc.v_part[part].p_size - (char *) &dk->dk_vtoc;
			pt2 = (char *) bp->b_un.b_addr + (BLKSZ*(vtocblk-start)) + voffset + offset;

			/* copy the data into a union */
			for (i=0; i < sizeof(dk->dk_vtoc.v_part[part].p_size); i++)
				psize.cstr[i] = *pt2++;

			/* Is the user trying to change the size */
			if (psize.val != dk->dk_vtoc.v_part[part].p_size)
			{	
				/*
				*  I could not determine a 'clean' scheme to prevent
				*  a user from changing the size of mounted partitions
				*  but still allow the Root partition to be changed.
				*  A few ideas were considered:
				*   1) Only let the boot device change a Root partition.
				*      (What does a boot device mean on an Adjunct)
				*   2) Allow a Root partition to be changed if it is
				*      the only mounted partition.
				*      (What about any device other than the boot device)
				*   3) A combination of 1 & 2 still has the same
				*      problem as idea 1.
				*
				*   So my current solution is to keep it simple
				*   and allow any mounted Root partition to be changed.
				*/
				if (!(dk->dk_part_flag[part] & DKMNT && dk->dk_vtoc.v_part[part].p_tag == V_ROOT))
					return(EBUSY);
			}

			/* CHECK: That a Mounted partition is not UNMOUNTABLE */
			offset = (char *) &dk->dk_vtoc.v_part[part].p_flag - (char *) &dk->dk_vtoc;
			pt1 = (char *) &flag;
			pt2 = (char *) bp->b_un.b_addr + (BLKSZ*(vtocblk-start)) + voffset + offset;
			for (i=0; i < sizeof(dk->dk_vtoc.v_part[part].p_flag); i++)
			{
				if (*pt1++ != *pt2++)
					break;	/* Partition is mountable */
			}
			/*
			*  If i == sizeof(), then Partition is not mountable
			*  but if it is also currently mounted, fail!
			*/
			if (i == sizeof(dk->dk_vtoc.v_part[part].p_flag) &&
			    dk->dk_part_flag[part] & DKMNT)
			{
				return(EBUSY);
			}
		}
	}

#ifdef DEBUG
        DPR (2)("sd01part_ck: - exit(0) ");
#endif
	return(0);
}

/* %BEGIN HEADER
 * %NAME: sd01hdelog - Reassign hard disk errors. - 0dd2d000
 * %DESCRIPTION:
	This function is called for mapping bad sector errors on disk drives.
 * %CALLED BY: sd01comp, sd01intb
 * %SIDE EFFECTS: 
	Data in an actual bad block is lost. No information is logged.
 * %RETURN VALUES: None
 * %ERROR 0dd2d001
	A marginal bad block was detected.  The driver is having difficulty
	reading this block. A Error Correction Code (ECC) had to be used.
 * %END ERROR
 * %END HEADER */
void
sd01hdelog (dk)
register struct disk *dk;		/* Pointer to disk structure	*/
{
	daddr_t	blkaddr;		/* Block address of error 	*/
	register int part;		/* Partition number		*/
	major_t	 maj;			/* Device major number		*/
	minor_t	 min;			/* Device minor number		*/
	char 	 name[49];		/* Device information		*/
	int	 sacred,		/* Sacred flag			*/
		 ptag,			/* Partition tag		*/
		 resflag,		/* Reassign bad block flag	*/
		 i,			/* Loop index			*/
		 rc;			/* Return error code		*/

	maj = dk->dk_fltreq->SCB.sc_dev.sa_major;
	min = dk->dk_fltreq->SCB.sc_dev.sa_minor; 
	part    = DKSLICE(min); 
	ptag    = dk->dk_vtoc.v_part[part].p_tag;
	blkaddr = dk->dk_sense.sd_ba;

#ifdef DEBUG
        DPR (1)("sd01hdelog: (blkaddr=0x%x) ", blkaddr);
        DPR (6)("sd01hdelog: (blkaddr=0x%x) state=0x%x ",blkaddr,dk->hde_state);
        DPR (6)("part=0x%x p_tag=0x%x ",part, ptag);
#endif

	sdi_name(&dk->dk_addr, name);

	/* Check if bad block is in non-UNIX area of the disk. */
	if (dk->dk_vtoc.v_sanity == VTOC_SANE && ptag == V_OTHER)
	{
		/* Resume the Q */
		sd01qresume(dk);

		/* Clear bad block state */
		dk->hde_state = HDESINIT;
		return;
	}

	/* Clear reassign and sacred flags */
	resflag = 0;
	sacred  = 0;

	/* Check if bad block is in sacred area of UNIX System partition. */
	if(blkaddr >= dk->unixst && blkaddr <= (dk->unixst + PDBLKNO))
		sacred = 1;

	/* If Sd01 log flag is invalid, set it to default mode. */
	if(Sd01log_marg > 2)
		Sd01log_marg = 0;

	/* Set up defect list header */
	dk->dk_dl_data[0] = 0;
	dk->dk_dl_data[1] = 0;
	dk->dk_dl_data[2] = 0;
	dk->dk_dl_data[3] = 4;

	/* Swap defect address */
	dk->dk_dl_data[4] = ((blkaddr & (unsigned) 0xFF000000) >> 24);
	dk->dk_dl_data[5] = ((blkaddr & 0x00FF0000) >> 16);
	dk->dk_dl_data[6] = ((blkaddr & 0x0000FF00) >> 8);
	dk->dk_dl_data[7] = ( blkaddr & 0x000000FF);

	/* Determine error type */
	switch(dk->hde_state & HDEMASK) {
	case HDERECERR:

		switch(dk->hde_state & HDESMASK) {
		case HDESINIT:	/* Start by sending a Read command */

#ifdef DEBUG
        DPR (6)("marginal bad block 0x%x ", blkaddr);
#endif

			if(Sd01log_marg || sacred) {
				cmn_err(CE_NOTE, "SD01: %s, Unit %d, Slice %d, Bad Block 0x%x, Err: 0dd2d001",
				name, dk->dk_addr.sa_lun, part, blkaddr);
			}

			if(sacred)
				cmn_err(CE_WARN, hde_mesg[HDESACRED], blkaddr);

			else if(Sd01log_marg == 1)
				cmn_err(CE_NOTE, hde_mesg[HDEECCMSG], blkaddr);

			else if(Sd01log_marg == 2) 
			{
				/* Send READ command to obtain data. */
				sd01icmd(dk,SM_READ,blkaddr,dk->blkbuf,BLKSZ,1,SCB_READ);
				return;
			}
			break;

		case HDESI:	 /* Read failed	*/
			cmn_err(CE_WARN, hde_mesg[HDEBADRED], blkaddr);
			/* Pass the job and continue to next case HDESII*/
			dk->hde_state += 1;

		case HDESII:	/* Read passed	*/
			/* Send REASSIGN BLOCKS to map out the bad sector */
			sd01icmd(dk,SS_REASGN,0,dk->dk_dl_data,RABLKSSZ,0,SCB_WRITE);
			return;

		case HDESIII:	/* Reassign failed */
			if(dk->hde_state & HDEENOSPR)
				cmn_err(CE_WARN, hde_mesg[HDENOSPAR], blkaddr);
			else
				cmn_err(CE_WARN, hde_mesg[HDEBADMAP], blkaddr);
			break;

		case HDESIV:	/* Reassign passed */
			resflag = 1;

			cmn_err(CE_NOTE, hde_mesg[HDEMAPBLK], blkaddr);

			/* Send WRITE command to initialize sector. */
			sd01icmd(dk,SM_WRITE,blkaddr,dk->blkbuf,BLKSZ,1,SCB_WRITE);
			return;

		case HDESV:	/* Write failed */
			cmn_err(CE_WARN, hde_mesg[HDEBADWRT], blkaddr);
			break;

		default:
			/* Write passed - fall out of case statements */
			break;

		}
		break;

	case HDEECCERR:

		switch(dk->hde_state & HDESMASK) {
		case HDESINIT:	/* Start by sending a Reassign command */
#ifdef DEBUG
        DPR (6)("actual bad block 0x%x ", blkaddr);
#endif
			if(sacred)
				cmn_err(CE_NOTE, hde_mesg[HDEBSACRD], blkaddr);

			/* Set state to skip read stage */
			dk->hde_state += 2;
			/* Send REASSIGN BLOCKS to map out the bad sector */
			sd01icmd(dk,SS_REASGN,0,dk->dk_dl_data,RABLKSSZ,0,SCB_WRITE);
			return;

		case HDESIII:	/* Reassign failed */
			if(dk->hde_state & HDEENOSPR)
				cmn_err(CE_WARN, hde_mesg[HDEBNOSPR], blkaddr);
			else
				cmn_err(CE_WARN, hde_mesg[HDEBNOMAP], blkaddr);
			break;

		case HDESIV:	/* Reassign passed */
			resflag = 1;
			cmn_err(CE_WARN, hde_mesg[HDEREASGN], blkaddr);

			/* Clear buffer */
			for(i=0; i < BLKSZ; i++)
				dk->blkbuf[i] = 0;

			/* Send WRITE command to initialize sector. */
			sd01icmd(dk,SM_WRITE,blkaddr,dk->blkbuf,BLKSZ,1,SCB_WRITE);
			return;

		case HDESV:	/* Write failed */
			cmn_err(CE_WARN, hde_mesg[HDENOINIT], blkaddr);
			break;

		default:
			/* Write passed - fall out of case statements */
			break;
		}
		break;
	}

	/* Check if a block was reassigned */
	if(resflag && dk->dk_vtoc.v_sanity == VTOC_SANE)
	{
		/* Check if block was in a valid file system. */
		if (dk->dk_vtoc.v_part[part].p_flag & V_VALID && ptag == V_ROOT || ptag == V_USR);
        	{
			/* Mark the file system dirty */
			fshadbad(makedev(maj,min), blkaddr - dk->dk_vtoc.v_part[part].p_start);
		}
	}

	/* Resume the Q */
	sd01qresume(dk);

	/* Clear bad block state */
	dk->hde_state = HDESINIT;

#ifdef DEBUG
        DPR (2)("sd01hdelog: - exit ");
        DPR (6)("sd01hdelog: - exit ");
#endif

}

/* %BEGIN HEADER
 * %NAME: sd01start - Start access to a device. - 0dd2f000
 * %DESCRIPTION:
	This function is not used but is required by UNIX.
 * %CALLED BY: Kernel
 * %SIDE EFFECTS: None
 * %RETURN VALUES: None
 * %END HEADER */
void
sd01start()
{
}

/* %BEGIN HEADER
 * %NAME: sd01lognberr - Log block type error. - 0dd30000
 * %DESCRIPTION:
	This function is used to log block type errors when there is
	no buffer header available.  
 * %CALLED BY: 
	sd01getjob, sd01send, sd01intf, sd01retry, sd01insane_dsk
	sd01flt, sd01flts, sd01ints, sd01intrq, sd01intres, sd01resume
	sd01flterr, sd01start
 * %SIDE EFFECTS: Log entry in the system error log.
 * %RETURN VALUES: None
 * %END HEADER */
void
sd01lognberr (emaj, emin, flags, blkno, blkcnt, iosp, estat1, estat2, estat3)
major_t	 	emaj;		/* external major 	*/
minor_t	 	emin;		/* external minor 	*/
int	 	flags;		/* b-flags 		*/
daddr_t		blkno;		/* block number	 	*/
unsigned	blkcnt;		/* block count 		*/
struct iostat	*iosp; 		/* iostat record 	*/
long		estat1;
long		estat2;  	/* driver defined errors*/
long		estat3;
{
	/*no block error logging on 3B2 or 386 */
}

/* %BEGIN HEADER
 * %NAME: sd01config - Determine drive configuration. - 0dd31000
 * %DESCRIPTION:
	This function initializes the disk driver's disk parameter 
	structure. If either MODE SENSE or READ CAPACITY are not supported,
	u_error is cleared so not to fail the open routine. In this case, 
	the V_CONFIG ioctl can be used to set drive parameters.
 * %CALLED BY: sd01open1
 * %SIDE EFFECTS: The disk state flag will indicate if the drive parameters 
	 	  are valid.
 * %RETURN VALUES: Zero or error value.
 * %ERROR 6dd31001
	The sectors per cylinder parameter calculates to less than or equal
	to zero.  This is caused by incorrect data returned by the MODE
	SENSE command or the drivers master file. This may not be an AT&T 
	supported device.
 * %END ERROR
 * %ERROR 6dd31002
	The number of cylinder calculates to less than or equal to zero.  
	This is caused by incorrect data returned by the READ CAPACITY
	command. This may not be an AT&T supported device.
 * %END ERROR
 * %END HEADER */
int
sd01config(dk)
register struct disk *dk;
{
	DADF_T	   *dadf = (DADF_T *) NULL;
	RDDG_T	   *rddg = (RDDG_T *) NULL;
	CAPACITY_T *cap  = (CAPACITY_T *) NULL;

	uint pg_asec_z;
	long cyls,
	     sec_cyl;
	
#ifdef DEBUG
        DPR (1)("sd01config: (dk=0x%x) ", dk);
#endif

	/* Send READ CAPACITY to obtain last sector address */
	if (sd01cmd(dk,SM_RDCAP,0,dk->dk_rc_data,RDCAPSZ,0,SCB_READ))
		return(EIO);

	cap = (CAPACITY_T *) (dk->dk_rc_data);

	cap->cd_addr = sdi_swap32(cap->cd_addr);
	cap->cd_len  = sdi_swap32(cap->cd_len);

	/* Check if the parameters are specified in space.c */
	if (Sd01diskinfo) {
#ifdef DEBUG
        DPR (3)("hard coded parms set ");
#endif
		/* Assign head and sector parameters for V_GETPARMS */
		dk->dk_parms.dp_sectors  = (Sd01diskinfo >> 8) & 0x00FF;
		dk->dk_parms.dp_heads    = Sd01diskinfo & 0x00FF;
		sec_cyl = dk->dk_parms.dp_sectors * dk->dk_parms.dp_heads;
	}
	else {
#ifdef DEBUG
        DPR (3)("automatic parms set ");
#endif
		/* Send MODE SENSE to obtain page 3 parameters */
		if (sd01cmd(dk,SS_MSENSE,0x0300,dk->dk_ms_data,FPGSZ,FPGSZ,SCB_READ)) {
			return(0);
		}

		dadf = (DADF_T *) (dk->dk_ms_data + SENSE_PLH_SZ + 
	       		((SENSE_PLH_T *) dk->dk_ms_data)->plh_bdl);

		/* Swap page 3 data */
		dadf->pg_sec_t   = sdi_swap16(dadf->pg_sec_t);
		dadf->pg_asec_z  = sdi_swap16(dadf->pg_asec_z);
		dadf->pg_bytes_s = sdi_swap16(dadf->pg_bytes_s);

		/* Assign sector parameter for V_GETPARMS */
		dk->dk_parms.dp_sectors = (dadf->pg_bytes_s * dadf->pg_sec_t) / cap->cd_len;  

		dk->dk_parms.dp_secsiz  = cap->cd_len;
		pg_asec_z = dadf->pg_asec_z;

		/* Send MODE SENSE to obtain page 4 parameters */
		if (sd01cmd(dk,SS_MSENSE,0x0400,dk->dk_ms_data,RPGSZ,RPGSZ,SCB_READ)) {
			return(0);
		}

		rddg = (RDDG_T *) (dk->dk_ms_data + SENSE_PLH_SZ + 
	       		((SENSE_PLH_T *) dk->dk_ms_data)->plh_bdl);

		sec_cyl = rddg->pg_head * (dk->dk_parms.dp_sectors - pg_asec_z);

		/* Assign head parameter for V_GETPARMS */
		dk->dk_parms.dp_heads = rddg->pg_head;

	}

	/* Check sec_cly calculation */
	if (sec_cyl <= 0)
	{
       		cmn_err(CE_WARN, "SD01: Sectors per cylinder error. Err: 6dd31001");
		return(0);
	}

	cyls = (cap->cd_addr + 1) / sec_cyl;

	/* Check cyls calculation */
	if (cyls <= 0)
	{
       		cmn_err(CE_WARN, "SD01: Number of cylinders error. Err: 6dd31002");
		return(0);
	}

	/* Make room for diagnostic scratch area */
	if ((cap->cd_addr + 1) == (cyls * sec_cyl))
		cyls--;

	/* Assign cylinder parameter for V_GETPARMS */
	dk->dk_parms.dp_cyls = cyls;

	/* Indicate parameters are set and valid */
	dk->dk_state |= DKPARMS; 

#ifdef DEBUG
        DPR (3)("sec=0x%x siz=0x%x heads=0x%x cyls=0x%x ", dk->dk_parms.dp_sectors, dk->dk_parms.dp_secsiz, dk->dk_parms.dp_heads, dk->dk_parms.dp_cyls);
#endif
	
#ifdef DEBUG
        DPR (2)("sd01config: parms set - exit(0) ");
#endif
	return(0);
}

/* %BEGIN HEADER
 * %NAME: sd01icmd - Perform an immediate SCSI command. - 0dd32000
 * %DESCRIPTION: 
	This funtion performs a SCSI command such as Reassign Blocks for
	the drivers bad block handling routine. The op code determines 
	the SCB for the job. The data area is supplied by the caller and 
	assumed to be in kernel space. 
 * %CALLED BY: sd01hdelog
 * %SIDE EFFECTS: 
	A immediate command is sent to the host adapter. It is NOT
	sent via the drivers job queue.
 * %RETURN VALUES:
	None;
 * %ERROR 8dd03201
	The host adapter rejected a request from the SCSI disk driver.
	This is caused by a parameter mismatch within the driver. The system
	should be rebooted.
 * %END ERROR
 * %END HEADER */
void
sd01icmd(dk, op_code, addr, buffer, size, length, mode)
register struct disk *dk;
char op_code;				/* Command Opcode		*/
unsigned int addr;			/* Address field of command 	*/
char *buffer;				/* Buffer for CDB data 		*/
unsigned int size;			/* Size of the buffer 		*/
unsigned int length;			/* Block length in the CDB	*/
unsigned short mode;			/* Direction of the transfer 	*/
{
	register struct job *jp;
	register struct scb *scb;
	register buf_t  *bp;

#ifdef DEBUG
        DPR (1)("sd01icmd: (dk=0x%x) ", dk);
        DPR (6)("sd01icmd: (dk=0x%x) ", dk);
#endif
	
	/* Set up buffer and job pointers */
	bp = &sd01_hde_buf;
	bp->b_flags = B_BUSY;
	jp = &sd01_bbh_job;

	/* Set up SB pointer */
	if (op_code == SM_READ)
		jp->j_cont = dk->dk_fltrblk;
	else if (op_code == SM_WRITE)
		jp->j_cont = dk->dk_fltwblk;
	else 
		jp->j_cont = dk->dk_fltmblk;

	jp->j_cont->sb_type = ISCB_TYPE;
	
	/* Set up buffer header */
	bp->b_flags |= mode & SCB_READ ? B_READ : B_WRITE;
	bp->b_un.b_addr = (caddr_t) buffer;	/* not used in sd01intb */
	bp->b_bcount = size;
	bp->b_error  = 0;
	
	/* Set up job structure */
	jp->j_dk = dk;
	jp->j_bp = bp;
	jp->j_done = 0;				/* not used in sd01intb */
	
	/* Set up SCB pointer */
	scb = &jp->j_cont->SCB;

	if (op_code & 0x20) { /* Group 1 commands */
		register struct scm *cmd;

		cmd = &jp->j_cmd.cm;
		cmd->sm_op   = op_code;
		cmd->sm_lun  = dk->dk_addr.sa_lun;
		cmd->sm_res1 = 0;
		cmd->sm_addr = addr;
		cmd->sm_res2 = 0;
		cmd->sm_len  = length;
		cmd->sm_cont = 0;

		scb->sc_cmdpt = SCM_AD(cmd);
		scb->sc_cmdsz = SCM_SZ;
	}
	else { /* Group 0 commands */
		register struct scs *cmd;

		cmd = &jp->j_cmd.cs;
		cmd->ss_op    = op_code;
		cmd->ss_lun   = dk->dk_addr.sa_lun;
		cmd->ss_addr1 = ((addr & 0x1F0000) >> 16);
		cmd->ss_addr  = (addr & 0xFFFF);
		cmd->ss_len   = length;
		cmd->ss_cont  = 0;

		scb->sc_cmdpt = SCS_AD(cmd);
		scb->sc_cmdsz = SCS_SZ;
	}
	
	/* Swap bytes in the address and length fields */
	if (jp->j_cont->SCB.sc_cmdsz == SCS_SZ)
		jp->j_cmd.cs.ss_addr = sdi_swap16(jp->j_cmd.cs.ss_addr);
	else {
		jp->j_cmd.cm.sm_addr = sdi_swap32(jp->j_cmd.cm.sm_addr);
		jp->j_cmd.cm.sm_len  = (short) sdi_swap16(jp->j_cmd.cm.sm_len);
	}
	
	/* Initialize SCB */
	scb->sc_int    = sd01intb;
	scb->sc_dev    = dk->dk_addr;
	scb->sc_datapt = buffer;
	scb->sc_datasz = size;
	scb->sc_mode   = mode;
	scb->sc_resid  = 0;
	scb->sc_time   = JTIME;
	scb->sc_wd     = (long) jp;

	/* Send the job */
	if (sdi_icmd(jp->j_cont) != SDI_RET_OK)
	{
		cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd32001");
		/* Fail the job */
		dk->hde_state += 1;
		sd01hdelog(dk);
	}

#ifdef DEBUG
        DPR (2)("sd01icmd: - exit(%d) ", jp->j_bp->b_error);
        DPR (6)("sd01icmd: - exit(%d) ", jp->j_bp->b_error);
#endif
}

/* %BEGIN HEADER
 * %NAME: sd01intb - Interrupt routine for Bad Block Handling. - 0dd33000
 * %DESCRIPTION: 
	This function is called by the host adapter driver when a
	SCSI Bad Block job or Request Sense job completes.  If the job fails, 
	the job is retried.  If the retry fails, the error is marked in the 
	buffer and the function returns.  However, if the job that failed 
	was a Reassign Blocks command, this function will send a Request 
	Sense to determine if the drive has run out of spare sectors.
 * %CALLED BY: Host adapter driver
 * %SIDE EFFECTS: None
 * %RETURN VALUES: None 
 * %ERROR 8dd03301
	The host adapter rejected a retry job request from the SCSI disk driver.
	This is caused by a parameter mismatch within the driver. The system
	should be rebooted.
 * %END ERROR
 * %ERROR 8dd03302
	The host adapter rejected a Request Sense request from the SCSI disk 
	driver.  This is caused by a parameter mismatch within the driver. The 
	system should be rebooted.
 * %END ERROR
 * %END HEADER */
void
sd01intb(sbp)
register struct sb *sbp;
{
	register struct job *jp;
	register struct disk *dk;
	register struct scb *scb;
	register struct scs *cmd;

	jp  = (struct job *) sbp->SCB.sc_wd;
	scb = &jp->j_cont->SCB;
	cmd = &jp->j_cmd.cs;
	dk  = jp->j_dk;

#ifdef DEBUG
        DPR (1)("sd01intb: (sbp=0x%x) jp=0x%x ", sbp, jp);
        DPR (6)("sd01intb: (sbp=0x%x) jp=0x%x ", sbp, jp);
#endif

	/* Check if interrupt was due to a Request Sense job */
	if (sbp == dk->dk_fltreq) {

		/* Swap new address in sense data */
		dk->dk_sense.sd_ba = sdi_swap32(dk->dk_sense.sd_ba);

		/* Fail the job */
		dk->hde_state += 1;

		/* Check if the Request Sense was ok */
		if (sbp->SCB.sc_comp_code == SDI_ASW) 
		{
			switch(dk->dk_sense.sd_key)
			{
				case SD_RECOVER:
					switch(dk->dk_sense.sd_sencode)
					{
			  		case SC_DATASYNCMK:
			  		case SC_RECOVRRD:
			  		case SC_RECOVRRDECC:
			  		case SC_RECOVRIDECC:
						/* Pass the job */
						dk->hde_state += 1;
					}
					break;
				case SD_MEDIUM:
					/* Check for Reassign command */
					if (cmd->ss_op == SS_REASGN)
						/* Indicate no spare sectors */
						dk->hde_state |= HDEENOSPR;
			}
		}
		/* Resume execution */
		sd01hdelog(dk);
		return;
	}
	
	/* Check if Bad Block job completed successfully. */
	if (jp->j_cont->SCB.sc_comp_code != SDI_ASW)
	{
		/* Retry the job */
		if (jp->j_bp->b_error < SD01_RETRYCNT)
		{
#ifdef DEBUG
        DPR (6)("retry ");
#endif
			jp->j_bp->b_error++;	  	 /* Update error count*/
			jp->j_cont->SCB.sc_time = JTIME; /* Reset the job time*/

			if (sdi_icmd(jp->j_cont) != SDI_RET_OK)
			{
				cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd33001");
				/* Fail the job */
				dk->hde_state += 1;
			}
			else
				return;
		}
		else {
#ifdef DEBUG
        DPR (6)("send REQSEN ");
#endif
			/* Send a Request Sense job */
			dk->dk_fltreq->sb_type 	    = ISCB_TYPE;
			dk->dk_fltreq->SCB.sc_int   = sd01intb;
			dk->dk_fltreq->SCB.sc_cmdsz = SCS_SZ;
			dk->dk_fltreq->SCB.sc_link  = 0;
			dk->dk_fltreq->SCB.sc_resid = 0;
			dk->dk_fltreq->SCB.sc_time  = JTIME;
			dk->dk_fltreq->SCB.sc_mode  = SCB_READ;
			dk->dk_fltreq->SCB.sc_dev   = scb->sc_dev;
			dk->dk_fltreq->SCB.sc_wd    = (long) jp;
			dk->dk_fltcmd.ss_op         = SS_REQSEN;
			dk->dk_fltcmd.ss_lun        = cmd->ss_lun;
			dk->dk_fltcmd.ss_addr1      = 0;
			dk->dk_fltcmd.ss_addr       = 0;
			dk->dk_fltcmd.ss_len        = SENSE_SZ;
			dk->dk_fltcmd.ss_cont       = 0;
			dk->dk_sense.sd_key         = SD_NOSENSE; 

			if (sdi_icmd(dk->dk_fltreq) != SDI_RET_OK)
				cmn_err(CE_WARN, "SD01: Bad type to host adapter. Err: 8dd33002");
			else
				return;

			/* Fail the job */
			dk->hde_state += 1;
		}
	}
	else
		dk->hde_state += 2;

#ifdef DEBUG
        DPR (2)("sd01intb: - exit ");
        DPR (6)("sd01intb: - exit ");
#endif

	/* Resume execution */
	sd01hdelog(dk);
}

/* %BEGIN HEADER
 * %NAME: sd01addr - Get device structure address. - 0dd34000
 * %DESCRIPTION: 
	This function returns the address of a disk structure
	for the specified device.
 * %CALLED BY: Mirror driver
 * %SIDE EFFECTS: None
 * %RETURN VALUES:  Address if device number is valid, NULL if not.
 * %END HEADER */
int
sd01addr(dev)
dev_t dev;
{
	/* Check if the device number is valid */
	if (sd01instbl[getemajor(dev)] == DKNOMAJ ||
	   (DKINDEX(dev) + 1) > sd01_diskcnt)
		return(NULL);
	else
		return((int)(Sd01_dp + DKINDEX(dev)));
}

/* %BEGIN HEADER
 * %NAME: sd01slice - Get device slice number. - 0dd35000
 * %DESCRIPTION: 
	This function returns the slice number for the specified device.
 * %CALLED BY: Mirror driver
 * %SIDE EFFECTS: None
 * %RETURN VALUES: 0 through V_NUMPAR
 * %END HEADER */
int
sd01slice(dev)
dev_t dev;
{
	return(DKSLICE(dev));                           
}
