/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*      INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied under the terms of a license agreement   */
/*	or nondisclosure agreement with Intel Corporation and may not be   */
/*	copied or disclosed except in accordance with the terms of that    */
/*	agreement.							   */

/*	Copyright (c) 1989 TOSHIBA CORPORATION		*/
/*	All Rights Reserved	*/

/*	Copyright (c) 1989 SORD COMPUTER CORPORATION	*/
/*	All Rights Reserved	*/

#ident	"@(#)scsi.in:io/sc01/sc01.c	1.3.2.2"


/*
**	SCSI CD-ROM Target Driver.
*/

#include "sys/errno.h"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/cmn_err.h"
#include "sys/buf.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/open.h"
#include "sys/ioctl.h"
#include "sys/debug.h"
#if	(_SYSTEMENV == 4)
#include "sys/conf.h"
#include "sys/uio.h"
#endif
#include "sys/ddi.h"
#include "sys/scsi.h"
#include "sys/sdi.h"
#include "sys/sdi_edt.h"
#include "sys/cdrom.h"
#include "sc01.h"
#if	(_SYSTEMENV == 4)
extern clock_t lbolt;			/* time in HZ since last boot */
#endif


/* Allocated in space.c */
extern 	struct tc_data	Sc01_data[];	/* Array of TC dev info	    */
extern	unsigned 	Sc01_datasz;	/* Number of supported TCs  */
extern	long	 	Sc01_bmajor;	/* Block major number	    */
extern	long	 	Sc01_cmajor;	/* Character major number   */
extern	int	 	Sc01_jobs;	/* Allocation per LU        */

static	int 		sc01_cdromcnt;	/* Num of cdroms configured  */
static	int 		sc01_jobcnt;	/* Total allocation of jobs */
static	int		sc01_errmsgflg;	/* System error message flag */
static	struct tc_edt  *sc01_edt;	/* Array of EDT structures  */
static	struct cdrom   *sc01_cdrom;	/* Array of cdrom structures */
static	struct job     *sc01_job;	/* Array of Job structures  */
static	struct buf	sc01_buf;	/* Buffer header for cmds   */
static	unsigned char	sc01_tmp[1024];	/* Temporary buffer 	    */
static	int		sc01_tmp_flag;	/* sc01_tmp control flag    */

static	struct job     *jbfreelist;	/* List of unallocated jobs */
static	int		jbwant;		/* Process sleep counter */

#define NAMESZ	49
#define	ARG	a0,a1,a2,a3,a4,a5

int	sc01sendt();
void	sc01intn();
void	sc01intf();
void	sc01intrq();
void	sc01cmn_err();
void	sc01strategy();


#ifdef DEBUG
#define SIZE    10
daddr_t	sc01_Offset = 0;
static char     sc01_Debug[SIZE] = {1,1,1,1,0,0,0,0,0,0};
#define DPR(l)          if (sc01_Debug[l]) sc01cmn_err
#endif


/*
** Function name: sc01init()
** Description:
**	Called by kernel to perform driver initialization.
**	This function does not access any devices.
*/

sc01init()
{
	register struct	tc_edt *edt_p;	/* pointer to TC EDT	 */
	register struct cdrom  *cdp;	/* cdrom pointer	 */
	register struct job    *jp;	/* Job struct pointer	 */

	caddr_t	 base;			/* Base memory pointer	 */

	int  tc_edtsz,			/* EDT size (in bytes)	 */
	     tc_cnt,			/* number of controllers */
	     cdromsz,			/* cdrom size (in bytes) */
	     jobsz,			/* Job size (in bytes)	 */
	     i,				/* loop index - level 1	 */
	     j,				/* loop index - level 2	 */
	     lu,			/* LU number		 */
	     tc;			/* TC number		 */


#ifdef	DEBUG
	DPR (1)(CE_CONT, "SC01 DEBUG DRIVER INSTALLED\n");
#endif
	/* Check if SDI has been started */
	if (!sdi_started)
	{
		/*
		 * Call HAD to build SCSI edt
		 */
		sdi_init();
	}

	sc01_cdromcnt = 0;
	sc01_errmsgflg = 0;		/* System error message ON */

	/*
	 * Allocate the TC EDT structures
	 */
	tc_edtsz = sdi_hacnt * MAX_TCS * sizeof(struct tc_edt);
#if	(_SYSTEMENV == 4)
	if ((base = (caddr_t)sptalloc(btoc(tc_edtsz), PG_V,
#else
	if ((base = (caddr_t)sptalloc(btoc(tc_edtsz), (PG_P|PG_LOCK),
#endif
			 NULL, NOSLEEP)) == 0)
	{
                sc01cmn_err(CE_WARN,
			"SC01: Insufficient memory to configure driver\n");
		sc01_cdromcnt = CDNOTCS;
		return;
	}
	sc01_edt = (struct tc_edt *) base;

	/* Get TC EDT data from SDI */
	tc_cnt = sdi_config("SC01", Sc01_cmajor, Sc01_bmajor, Sc01_data,
				Sc01_datasz, sc01_edt);

	for (tc = 0, edt_p = sc01_edt; tc < tc_cnt; tc++, edt_p++) 
		sc01_cdromcnt += edt_p->n_lus;

	/* Check if there are devices configured */
	if (sc01_cdromcnt == 0) {
		sptfree(sc01_edt, btoc(tc_edtsz), 1);
		sc01_cdromcnt = CDNOTCS;
		return;
	}

	/*
	 * Allocate the cdrom and job structures
	 */
	cdromsz = sc01_cdromcnt * sizeof(struct cdrom);
	jobsz  = sc01_cdromcnt * Sc01_jobs * sizeof(struct job);
#if	(_SYSTEMENV == 4)
        if ((base = (caddr_t)sptalloc(btoc(cdromsz + jobsz), PG_V,
#else
        if ((base = (caddr_t)sptalloc(btoc(cdromsz + jobsz), (PG_P|PG_LOCK),
#endif
			 NULL, NOSLEEP)) == 0)
	{
                sc01cmn_err(CE_WARN,
			"SC01: Insufficient memory to configure driver\n");
		sptfree(sc01_edt, btoc(tc_edtsz), 1);
		sc01_cdromcnt = CDNOTCS;
		return;
	}

	sc01_cdrom = (struct cdrom *) base;
	sc01_job  = (struct job *) (base + cdromsz);

	/*
	 * Initialize the cdrom structures
	 */
	cdp = sc01_cdrom;
	for (tc = 0, edt_p = sc01_edt; tc < tc_cnt; tc++, edt_p++) 
	{
	    for (i = 0, lu = 0; i < (int)edt_p->n_lus; i++, lu++, cdp++)
	    {
	    	for (; lu < MAX_LUS; lu++)	/* Find next equipped LU */
	    		if(edt_p->lu_id[lu])
				break;

		/* Initialize the queue ptrs */
		cdp->cd_first = (struct job *) cdp;
		cdp->cd_last  = (struct job *) cdp;
		cdp->cd_next  = (struct job *) cdp;
		cdp->cd_batch = (struct job *) cdp;

		/* Initialize state & counters */
		cdp->cd_state  = 0;
		cdp->cd_count  = 0;
		cdp->cd_npend  = 0;
		cdp->cd_fltcnt = 0;

		/* Initialize the SCSI address */
		cdp->cd_addr.sa_exlun = 0;
		cdp->cd_addr.sa_lun   = lu;
	    	cdp->cd_addr.sa_fill  = SDI_DEV(edt_p);
	    }
	}

	/*  Deallocate the TC EDT structure  */
	sptfree((caddr_t)sc01_edt, btoc(tc_edtsz), 1);

	sc01_jobcnt  = sc01_cdromcnt * Sc01_jobs;

	/*  Build list of free job blocks */
	jbfreelist = NULL;
	for (jp = sc01_job; jp < sc01_job + (sc01_jobcnt - 1); jp++) {
		jp->j_next = jbfreelist;
		jbfreelist = jp;
	}
}


/*
** Function name: sc01getjob()
** Description:
**	This function will allocate a cdrom job structure from the free
**	list.  The function will sleep if there are no jobs available.
**	It will then get a SCSI block from SDI.
*/

struct job *
sc01getjob()
{
	register struct job *jp;
	register int s;

#ifdef	DEBUG
	DPR (4)(CE_CONT, "sc01: getjob\n");
#endif
	s = spl5();
	while ( !(jp = jbfreelist)) {
		jbwant++; 
		sleep((caddr_t)&jbfreelist, PRIBIO+1);
	}
	jbfreelist = jp->j_next;
	splx(s);

	/* Get an SB for this job */
	jp->j_sb = sdi_getblk();
	return(jp);
}
	

/*
** Function name: sc01freejob()
** Description:
**	This function returns a job structure to the free list. The
**	SCSI block associated with the job is returned to SDI.
*/

sc01freejob(jp)
register struct job *jp;
{
	register int s;

#ifdef	DEBUG
	DPR (4)(CE_CONT, "sc01: freejob\n");
#endif
	sdi_freeblk(jp->j_sb);

	s = spl5();
	jp->j_next = jbfreelist;
	jbfreelist = jp;
	if (jbwant) {
		jbwant = 0;
		wakeup((caddr_t)&jbfreelist);
	}
	splx(s);
}


/*
** Function name: sc01send()
** Description:
**	This function sends jobs from the work queue to the host adapter.
**	It will send as many jobs as available or the number required to
**	keep the logical unit busy.  If the job cannot be accepted by SDI 
**	the function will reschedule itself via the timeout mechanizism.
*/

sc01send(cdp)
register struct cdrom *cdp;
{
	register struct job *jp;
	register int rval;

#ifdef	DEBUG
	DPR (1)(CE_CONT, "sc01: sc01send\n");
#endif
	while (cdp->cd_npend < MAXPEND &&  cdp->cd_next != (struct job *)cdp)
	{
		jp = cdp->cd_next;
		cdp->cd_next = jp->j_next;

		if (jp == cdp->cd_batch) {
			/* Start new batch */
			cdp->cd_batch = (struct job *)cdp;
			cdp->cd_state ^= CD_DIR;
		}

		if ((rval = sdi_send(jp->j_sb)) != SDI_RET_OK)
		{
			if (rval == SDI_RET_RETRY) {
#ifdef DEBUG
				sc01cmn_err(CE_NOTE, "SC01: SDI rejected job");
#endif
				/* Reset next position */
				cdp->cd_next = jp;

				if (cdp->cd_state & CD_SEND)
					return;

				/* Call back later */
				cdp->cd_state |= CD_SEND;
				cdp->cd_sendid =
					 timeout((void(*)())sc01sendt, (caddr_t)cdp, LATER);
				return;
			} else {
				sc01cmn_err(CE_WARN, "SC01: Bad SB type to SDI\n");
				cdp->cd_npend++;
				sc01comp(jp);
				continue;
			}
		}

		cdp->cd_npend++;
	}

	if (cdp->cd_state & CD_SEND) {
		cdp->cd_state &= ~CD_SEND;
		untimeout(cdp->cd_sendid);
	}
}


/*
** Function name: sc01sendt()
** Description:
**	This function calls sc01send() after the record of the pending
**	timeout is erased.
*/

sc01sendt(cdp)
register struct cdrom *cdp;
{
	cdp->cd_state &= ~CD_SEND;
	sc01send(cdp);
}


/*
** Function name: sc01open()
** Description:
** 	Driver open() entry point.  Determines the type of open being
**	being requested.  On the first open to a device, the PD and
**	VTOC information is read. 
*/

#if	(_SYSTEMENV == 4)
sc01open(devp, flag, otyp, cred_p)
dev_t	*devp;
#else
sc01open(dev, flag, otyp)
dev_t	dev;
#endif
int	flag;
int	otyp;
#if	(_SYSTEMENV == 4)
struct cred	*cred_p;
#endif
{
	register struct cdrom	*cdp;
	unsigned		unit;
#if	(_SYSTEMENV == 4)
	dev_t	dev = *devp;
#endif

#ifdef	DEBUG
	if (getminor(dev) & 0x08) {		/* Debug off */
		int	i;

		for (i = 0; i < SIZE; i++)
			sc01_Debug[i] = 0;
	} else if (getminor(dev) & 0x10) {		/* Debug on */
		int	i;

		for (i = 0; i < SIZE; i++)
			sc01_Debug[i] = 1;
	}

	DPR (1)(CE_CONT, "sc01open: (dev=0x%x flag=0x%x otype=0x%x)\n",
			dev, flag, otyp);
#endif

	unit = UNIT(dev);
	cdp = &sc01_cdrom[unit];

	/* Check for non-existent device */
	if ((int)unit >= sc01_cdromcnt) {
		u.u_error = ENXIO;
#ifdef	DEBUG
		DPR (3)(CE_CONT, "sc01open: open error\n");
#endif
		return(ENXIO);
	}

	/* Sleep if someone else already opening */
	while (cdp->cd_state & CD_WOPEN)
		sleep((caddr_t)&cdp->cd_state, PRIBIO);

	/* Lock out other attempts */
	cdp->cd_state |= CD_WOPEN;

	if (!(cdp->cd_state & CD_INIT))
	{
		cdp->cd_fltreq = sdi_getblk();  /* Request sense */
		cdp->cd_fltres = sdi_getblk();  /* Resume */

		cdp->cd_fltreq->sb_type = ISCB_TYPE;
		cdp->cd_fltreq->SCB.sc_datapt = SENSE_AD(&cdp->cd_sense);
		cdp->cd_fltreq->SCB.sc_datasz = SENSE_SZ;
		cdp->cd_fltreq->SCB.sc_mode   = SCB_READ;
		cdp->cd_fltreq->SCB.sc_cmdpt  = SCS_AD(&cdp->cd_fltcmd);
		sdi_translate(cdp->cd_fltreq, B_READ, 0);

#if	(_SYSTEMENV == 4)
		cdp->cd_addr.sa_major = getmajor(dev);
		cdp->cd_addr.sa_minor = getminor(dev);
#else
		cdp->cd_addr.sa_major = major(dev);
		cdp->cd_addr.sa_minor = minor(dev);
#endif

		cdp->cd_state |= CD_INIT;
	}

	cdp->cd_state &= ~CD_WOPEN;
	wakeup((caddr_t)&cdp->cd_state);

#ifdef	DEBUG
	DPR (1)(CE_CONT, "sc01open: end\n");
#endif
	return(0);
}


/*
** Function name: sc01close()
** Description:
** 	Driver close() entry point.  Determine the type of close
**	being requested.
*/

#if	(_SYSTEMENV == 4)
sc01close(dev, flag, otyp, cred_p)
#else
sc01close(dev, flag, otyp)
#endif
register dev_t	dev;
int		flag;
int		otyp;
#if	(_SYSTEMENV == 4)
struct cred	*cred_p;
#endif
{
	register struct cdrom	*cdp;
#ifndef	BUG
	/*
	 * Normally, if prefixopen() is failed, prefixclose() isn't called.
	 * But prefixclose() is called from inner kernel in UNIX R4.0 B9.
	 */
	unsigned		unit;

	unit = UNIT(dev);
	/* Check for non-existent device */
	if ((int)unit >= sc01_cdromcnt) {
		u.u_error = ENXIO;
#ifdef	DEBUG
		DPR (3)(CE_CONT, "sc01close: close error\n");
#endif
		return(ENXIO);
	}
	cdp = &sc01_cdrom[unit];
#else
	cdp = &sc01_cdrom[UNIT(dev)];
#endif
	cdp->cd_state &= ~CD_PARMS;

	return(0);
}


/*
** Function name: sc01strategy()
** Description:
** 	Driver strategy() entry point.  Initiate I/O to the device.
**	The buffer pointer passed from the kernel contains all the
**	necessary information to perform the job.  This function only
**	checks the validity of the request.  Most of the work is done
**	by sc01io().
*/

void
sc01strategy(bp)
register struct buf	*bp;
{
	register struct cdrom	*cdp;
	register int		lastsect;	/* Last sector of device */
	register int		sectlen;	/* Sector length */
	unsigned		sectcnt;	/* Sector count */

	cdp = &sc01_cdrom[UNIT(bp->b_dev)];

#ifdef	DEBUG
	DPR (1)(CE_CONT, "sc01: sc01strategy\n");
#endif
	/*
	 * CD-ROM is a read-only device.
	 */
	if ((bp->b_flags & B_READ) == 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EACCES;
		iodone(bp);
		return;
	}

	if (((cdp->cd_state & CD_PARMS) == 0) && sc01config(cdp)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}

	lastsect = cdp->cd_capacity.cd_addr;
	sectlen = cdp->cd_capacity.cd_len;
	sectcnt = (bp->b_bcount + sectlen - 1) / sectlen;

	/*
	 * Reject a partial sector request.
	 */
	if ((sectlen & BLKMASK) || (bp->b_blkno & ((sectlen / BLKSIZE) - 1)))
	{
		bp->b_flags |= B_ERROR;
		bp->b_error = EACCES;
		iodone(bp);
		return;
		
	}
	bp->b_sector = bp->b_blkno / (sectlen / BLKSIZE);

#ifdef	DEBUG
	DPR (3)(CE_CONT, "Block 0x%x -> Sector 0x%x\n",
		bp->b_blkno, bp->b_sector);

	bp->b_sector += sc01_Offset;
	DPR (3)(CE_CONT, "Offset 0x%x -> Sector 0x%x\n",
		sc01_Offset, bp->b_sector);
#endif

	/*
	 * Check if request fits in CD-ROM device.
	 */
	if (bp->b_sector + sectcnt > lastsect) {
		if (bp->b_sector > lastsect) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
		}
		bp->b_resid = bp->b_bcount -
			((lastsect - bp->b_sector) * sectlen);
		if (bp->b_bcount == bp->b_resid) {
			iodone(bp);
			return;
		}
		bp->b_bcount -= bp->b_resid;
	}

	sc01io(cdp, bp);
}


/*
** Function name: sc01io()
** Description:
**	This function creates a SCSI I/O request from the information in
**	the cdrom structure and the buffer header.  The request is queued
**	according to an elevator algorithm.
*/

sc01io(cdp, bp)
register struct cdrom	*cdp;
register buf_t		*bp;
{
	register struct job	*jp;
	register struct scb	*scb;
	register struct scm	*cmd;
	register int		sectlen;
	unsigned		sectcnt;
	int			s;

#ifdef	DEBUG
	DPR (1)(CE_CONT, "sc01: sc01io\n");
#endif
	jp = sc01getjob();
	jp->j_bp = bp;
	jp->j_cdp = cdp;
	jp->j_errcnt = 0;

	jp->j_sb->sb_type = SCB_TYPE;

	/*
	 * Fill in the scb for this job.
	 */
	scb = &jp->j_sb->SCB;
	scb->sc_cmdpt = SCM_AD(&jp->j_cmd);
	scb->sc_cmdsz = SCM_SZ;
	scb->sc_datapt = bp->b_un.b_addr;
	scb->sc_datasz = bp->b_bcount;
	scb->sc_link = NULL;
	scb->sc_mode = SCB_READ;

	sdi_translate(jp->j_sb, bp->b_flags, bp->b_proc);

	scb->sc_int = sc01intn;
	scb->sc_dev = cdp->cd_addr;
	scb->sc_time = JTIME;
	scb->sc_wd = (long) jp;

	/*
	 * Fill in the command for this job.
	 */
	cmd = &jp->j_cmd.sm;
	cmd->sm_op = SM_READ;
	cmd->sm_lun = cdp->cd_addr.sa_lun;
	cmd->sm_res1 = 0;
	cmd->sm_res2 = 0;
	cmd->sm_cont = 0;

	jp->j_addr = bp->b_sector;

	sectlen = cdp->cd_capacity.cd_len;
	sectcnt = (bp->b_bcount + sectlen - 1) / sectlen;

	cmd->sm_len  = sdi_swap16(sectcnt);
	cmd->sm_addr = sdi_swap32(jp->j_addr);

#ifdef	DEBUG
	DPR (3)(CE_CONT, "Sector = 0x%x,   Count = 0x%x\n",
		bp->b_sector, sectcnt);
#endif
	/* Is this a partial block? */
	if ((scb->sc_resid = (cmd->sm_len * sectlen) - bp->b_bcount) != 0)
		scb->sc_mode |= SCB_PARTBLK;

	bp->b_start = lbolt;

	/*
 	 * Put the job onto the drive worklist using
 	 * an elevator algorithm.
 	 */
	s = spl5();
	cdp->cd_count++;
        if (cdp->cd_next == (struct job *) cdp) {
                jp->j_next = (struct job *) cdp;
                jp->j_prev = cdp->cd_last;
                cdp->cd_last->j_next = jp;
		cdp->cd_last = jp;
		cdp->cd_next = jp;
	} else {
		register struct job *jp1 = cdp->cd_batch;

                if (cdp->cd_state & CD_DIR) { 
                        while (jp1 != (struct job *) cdp) {
				if (jp1->j_addr < jp->j_addr)
					break;
                                jp1 = jp1->j_next;
			}
		} else {
                        while (jp1 != (struct job *) cdp) {
				if (jp1->j_addr > jp->j_addr)
					break;
                                jp1 = jp1->j_next;
			}
		}

                jp->j_next = jp1;
                jp->j_prev = jp1->j_prev;
                jp1->j_prev->j_next = jp;
                jp1->j_prev = jp;

                if (jp1 == cdp->cd_batch)
                        cdp->cd_batch = jp;
                if (jp1 == cdp->cd_next)
                        cdp->cd_next = jp;
	}

	sc01send(cdp);
	splx(s);
}


/*
** Function name: sc01breakup()
** Description:
** 	Driver raw I/O entry point.  Performs a raw transfer using the 
**	user's locked down buffer.  The function calls sc01strategy()
**	with contiguous DMA-able pieces of the request.
*/
sc01breakup(bp)
struct buf *bp;
{
dma_pageio(sc01strategy, bp);
}


/*
** Function name: sc01read()
** Description:
** 	Driver read() entry point.  Performs a "raw" read.
*/

#if	(_SYSTEMENV == 4)
sc01read(dev, uio_p, cred_p)
#else
sc01read(dev)
#endif
dev_t dev;
#if	(_SYSTEMENV == 4)
struct uio	*uio_p;
struct cred	*cred_p;
#endif
{
	register struct cdrom	*cdp;

	cdp = &sc01_cdrom[UNIT(dev)];

	if (((cdp->cd_state & CD_PARMS) == 0) && sc01config(cdp)) {
		u.u_error = ENXIO;
		return(ENXIO);
	}

#if	(_SYSTEMENV == 4)
	return(uiophysio(sc01breakup, NULL, dev, B_READ, uio_p));
#else
	physio(sc01strategy, NULL, dev, B_READ);
	return(u.u_error);
#endif
}


/*
** Function name: sc01print()
** Description:
** 	Driver print() entry point.  Prints an error message on
**	the system console.
*/

sc01print(dev, str)
dev_t	dev;
char	*str;
{
	register struct cdrom	*cdp;
	char	name[NAMESZ];

	cdp = &sc01_cdrom[UNIT(dev)];

	sdi_name(&cdp->cd_addr, name);
	sc01cmn_err(CE_WARN, "SC01 %s LU %d - %s\n",
		name, cdp->cd_addr.sa_lun, str);
}

#if	(_SYSTEMENV == 4)
/*
** Function name: sc01size()
** Description:
**	Driver size() entry point.  Return the device size.
*/
sc01size(dev)
dev_t	dev;
{
	register struct cdrom	*cdp;
	
	cdp = &sc01_cdrom[UNIT(dev)];
	if ((cdp->cd_state & CD_PARMS) == 0) {
		int	old_errmsgflg;

		old_errmsgflg = sc01_errmsgflg;
		sc01_errmsgflg = 1;
#if	(_SYSTEMENV == 4)
		if (sc01open(&dev, 0, OTYP_LYR, (struct cred *)0) ||
			sc01config(cdp))
		{
#else
		if (sc01open(dev, 0, OTYP_LYR) || sc01config(cdp))
		{
#endif
			sc01_errmsgflg = old_errmsgflg;
			return(-1);
		}
		sc01_errmsgflg = old_errmsgflg;
	}
	return(cdp->cd_capacity.cd_addr * cdp->cd_capacity.cd_len / BLKSIZE);
}
#endif

/*
** Function name: sc01ioctl()
** Description:
**	Driver ioctl() entry point.  Used to implement the following 
**	special functions:
**
**    B_GETTYPE		-  Get bus type and driver name
**    B_GETDEV		-  Get pass-thru major/minor numbers 
**    C_ERRMSGON	-  System error message ON	
**    C_ERRMSGOFF	-  System error message OFF
**    
**  Group 0 commands
**    C_TESTUNIT	-  Test unit ready
**    C_REZERO		-  Rezero unit
**    C_SEEK		-  Seek
**    C_INQUIR		-  Inquiry
**    C_STARTUNIT	-  Start unit
**    C_STOPUNIT	-  Stop unit
**    C_PREVMV		-  Prevent medium removal
**    C_ALLOMV		-  Allow medium removal
**
**  Group 1 commands
**    C_READCAPA	-  Read capacity
**
**  Group 6 commands
**    C_AUDIOSEARCH	-  Audio track search
**    C_PLAYAUDIO	-  Play audio
**    C_STILL		-  Still
**    C_TRAYOPEN	-  tray open
**    C_TRAYCLOSE	-  tray close
*/

#if	(_SYSTEMENV == 4)
sc01ioctl(dev, cmd, arg, mode, cred_p, rval_p)
#else
sc01ioctl(dev, cmd, arg, mode)
#endif
dev_t	dev;
int	cmd;
caddr_t	arg;
int	mode;
#if	(_SYSTEMENV == 4)
struct cred	*cred_p;
int		*rval_p;
#endif
{
	register struct cdrom	*cdp;

	cdp = &sc01_cdrom[UNIT(dev)];
	u.u_error = 0;

	switch(cmd) {
	case	B_GETTYPE:
		/*
		 * Tell user bus and driver name
		 */
		if (copyout("scsi", ((struct bus_type *)arg)->bus_name, 5))
		{
			u.u_error = EFAULT;
			break;
		}
		if (copyout("sc01", ((struct bus_type *)arg)->drv_name, 5))
		{
			u.u_error = EFAULT;
			break;
		}
		break;

	case	B_GETDEV:
		/*
		 * Return pass-thru device major/minor 
		 * to user in arg.
		 */
		{
			dev_t	pdev;

			sdi_getdev(&cdp->cd_addr, &pdev);
			if (copyout((caddr_t)&pdev, arg, sizeof(pdev))) {
				u.u_error = EFAULT;
			}
			break;
		}

	case	SDI_RELEASE:	/* XXX */
		/*
		 * allow another processor on the same SCSI bus to accsess a 
		 * reserved drive.
		 */
		if (sc01cmd(cdp, SS_RELES, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;

	case	SDI_RESERVE:	/* XXX */
		/*
		 * reserve a drive to a processor.
		 */
		if (sc01cmd(cdp, SS_RESERV, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;

	case	C_ERRMSGON:
		/*
		 * System Error message ON
		 */
		sc01_errmsgflg = 0;
		break;

	case	C_ERRMSGOFF:
		/*
		 * System Error message OFF
		 */
		sc01_errmsgflg = 1;
		break;
		
	/*
	 * The following ioctls are group 0 commamds
	 */
	case	C_TESTUNIT:	/* XXX */
		/*
		 * Test Unit Ready
		 */
		if (sc01cmd(cdp, SS_TEST, 0, NULL, 0, 0, SCB_READ, 0, 0))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;

	case	C_REZERO:	/* XXX */
		/*
		 * Rezero Unit 
		 */
		if (sc01cmd(cdp, SS_REZERO, 0, NULL, 0, 0, SCB_READ, 0, 0))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;

	case	C_SEEK:	/* XXX */
		/*
		 * Seek 
		 */
		if (sc01cmd(cdp, SS_SEEK, 0, NULL, 0, (int)arg, SCB_READ, 0, 0))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;

	case	C_INQUIR:
		/*
		 * Inquire
		 */
	{
		struct cdrom_inq	*inqp;
		int			inqlen;

		while (sc01_tmp_flag & B_BUSY) {
			sc01_tmp_flag |= B_WANTED;
			sleep((caddr_t)sc01_tmp, PRIBIO+1);
		}

		inqp = (struct cdrom_inq *)&sc01_tmp[200];
		if (copyin((caddr_t)arg, (caddr_t)inqp,
			sizeof(struct cdrom_inq)) < 0)
		{
			u.u_error = EFAULT;
			goto INQUIR_EXIT;
		}
		if ((inqp->length > INQ_SZ) || (inqp->length == 0))
			inqlen = INQ_SZ;
		else
			inqlen = inqp->length;
#ifdef DEBUG
		DPR (3)(CE_CONT,
			"sc01:SC_INQUIR length=%x addr=%x inqlen %x\n",
				inqp->length, inqp->addr, inqlen);
#endif
		if (sc01cmd(cdp, SS_INQUIR, 0, (char *)sc01_tmp, inqlen,
			inqlen, SCB_READ, 0, 0))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
			goto INQUIR_EXIT;
		}
		if (copyout((caddr_t)sc01_tmp, inqp->addr, inqlen))
			u.u_error = EFAULT;
INQUIR_EXIT:
		sc01_tmp_flag &= ~B_BUSY;
		if (sc01_tmp_flag & B_WANTED)
			wakeup((caddr_t)sc01_tmp);
		break;
	}

	case	C_STARTUNIT:	/* XXX */
	case	C_STOPUNIT:	/* XXX */
		/*
		 * Start/Stop unit
		 */
		if (sc01cmd(cdp, SS_ST_SP, 0, NULL, 0, 
			(cmd == C_STARTUNIT) ? 1 : 0, SCB_READ, 0, 0))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;

	case	C_PREVMV:	/* XXX */
	case	C_ALLOMV:	/* XXX */
		/*
		 * Prevent/Allow media removal
		 */
		if (sc01cmd(cdp, SS_LOCK, 0, NULL, 0, 
			(cmd == C_PREVMV) ? 1 : 0, SCB_READ, 0, 0))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;

	/*
	 * The following ioctls are group 1 commamds
	 */
	case	C_READCAPA:
		/*
		 * Read capacity
		 */
	{
		register struct capacity	*cp;

		while (sc01_tmp_flag & B_BUSY) {
			sc01_tmp_flag |= B_WANTED;
			sleep((caddr_t)sc01_tmp, PRIBIO+1);
		}
		cp = (struct capacity *)sc01_tmp;
		if (sc01cmd(cdp, SM_RDCAP, 0, (char *)cp, RDCAP_SZ, 0,
			SCB_READ, 0, 0))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
			goto READCAPA_EXIT;
		}
		cp->cd_addr = sdi_swap32(cp->cd_addr);
		cp->cd_len  = sdi_swap32(cp->cd_len);
		if (copyout((caddr_t)cp, arg, sizeof(struct capacity)))
			u.u_error = EFAULT;

READCAPA_EXIT:
		sc01_tmp_flag &= ~B_BUSY;
		if (sc01_tmp_flag & B_WANTED)
			wakeup((caddr_t)sc01_tmp);
		break;
	}

	/*
	 * The following ioctls are group 6 commamds
	 */

	case	C_AUDIOSEARCH:
	case	C_PLAYAUDIO:
	{
		/*
		 * Audio track search &
		 * Play audio
		 */
		struct cdrom_audio	audio;

		if (copyin((caddr_t)arg, (caddr_t)&audio,
			sizeof(struct cdrom_audio)))
		{
			u.u_error = EFAULT;
			break;
		}
		if (sc01cmd(cdp, 
			(cmd == C_AUDIOSEARCH) ? SV_AUDIOSEARCH : SV_PLAYAUDIO,
			audio.addr_logical, NULL, 0, 0, SCB_READ, audio.play,
			(audio.type << 6)))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;
	}
	case	C_STILL:
		/*
		 * Still
		 */
		if (sc01cmd(cdp, SV_STILL, 0, NULL, 0, 0, SCB_READ, 0, 0))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;

	case	C_TRAYOPEN:
	case	C_TRAYCLOSE:
	{
		/*
		 * Tray open/close
		 */
		if (sc01cmd(cdp,
			(cmd == C_TRAYOPEN) ? SV_TRAYOPEN : SV_TRAYCLOSE,
			0, NULL, 0, 0, SCB_READ, (int)arg & 0x01, 0))
		{
			if (u.u_error == 0) {
				u.u_error = ENXIO;
			}
		}
		break;
	}

	default:
		u.u_error = EINVAL;
	}
	return(u.u_error);
}


/*
** Function name: sc01cmd()
** Description:
**	This function performs a SCSI command such as Mode Sense on
**	the addressed cdrom.  The op code indicates the type of job
**	but is not decoded by this function.  The data area is
**	supplied by the caller and assumed to be in kernel space. 
**	This function will sleep.
*/

sc01cmd(cdp, op_code, addr, buffer, size, length, mode, param, control)
register struct cdrom	*cdp;
unsigned char	op_code;		/* Command opcode		*/
unsigned int	addr;			/* Address field of command 	*/
char		*buffer;		/* Buffer for command data 	*/
unsigned int	size;			/* Size of the data buffer 	*/
unsigned int	length;			/* Block length in the CDB	*/
unsigned short	mode;			/* Direction of the transfer 	*/
unsigned int	param;			/* Parameter bit		*/
unsigned int	control;		/* Control byte			*/
{
	register struct job *jp;
	register struct scb *scb;
	register buf_t *bp;
	int s;

	bp = &sc01_buf;
	while (bp->b_flags & B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PRIBIO+1);
	}
	bp->b_flags = B_BUSY;
	
	jp = sc01getjob();
	scb = &jp->j_sb->SCB;
	
	bp->b_sector = addr;
	bp->b_blkno = addr * (cdp->cd_capacity.cd_len / BLKSIZE);
	bp->b_flags |= mode & SCB_READ ? B_READ : B_WRITE;
	bp->b_error = 0;
	
	jp->j_bp = bp;
	jp->j_cdp = cdp;
	jp->j_errcnt = 0;
	jp->j_sb->sb_type = SCB_TYPE;

	if (op_code & 0x60) {		/* Group 6 commands */
		register struct scv	*cmd;

		cmd = (struct scv *)&jp->j_cmd.sv;
		cmd->sv_op   = op_code;
		cmd->sv_lun  = cdp->cd_addr.sa_lun;
		cmd->sv_res1 = 0;
		cmd->sv_param = param;
		cmd->sv_addr = sdi_swap32(addr);
		cmd->sv_res2 = 0;
		cmd->sv_cont = control & 0xc0;

		scb->sc_cmdpt = SCV_AD(cmd);
		scb->sc_cmdsz = SCV_SZ;
	} else if (op_code & 0x20) {	/* Group 1 commands */
		register struct scm	*cmd;

		cmd = (struct scm *)&jp->j_cmd.sm;
		cmd->sm_op   = op_code;
		cmd->sm_lun  = cdp->cd_addr.sa_lun;
		cmd->sm_res1 = 0;
		cmd->sm_addr = sdi_swap32(addr);
		cmd->sm_res2 = 0;
		cmd->sm_len  = sdi_swap16(length);
		cmd->sm_cont = 0;

		scb->sc_cmdpt = SCM_AD(cmd);
		scb->sc_cmdsz = SCM_SZ;
	} else {		  /* Group 0 commands */
		register struct scs	*cmd;

		cmd = (struct scs *)&jp->j_cmd.ss;
		cmd->ss_op    = op_code;
		cmd->ss_lun   = cdp->cd_addr.sa_lun;
		cmd->ss_addr1 = (addr & 0x1F0000);
		cmd->ss_addr = sdi_swap16(addr & 0xFFFF);
		cmd->ss_len   = length;
		cmd->ss_cont  = 0;

		scb->sc_cmdpt = SCS_AD(cmd);
		scb->sc_cmdsz = SCS_SZ;
	}
	
	/* Fill in the SCB */
	scb->sc_int = sc01intn;
	scb->sc_dev = cdp->cd_addr;
	scb->sc_datapt = buffer;
	scb->sc_datasz = size;
	scb->sc_mode = mode;
	scb->sc_resid = 0;
	scb->sc_time = JTIME;
	scb->sc_wd = (long) jp;
	sdi_translate(jp->j_sb, bp->b_flags, (caddr_t)0);

	/* Add job to the queue at the end and batch the queue */
	s = spl5();
	cdp->cd_count++;
	jp->j_next = (struct job *) cdp;
	jp->j_prev = cdp->cd_last;
	cdp->cd_last->j_next = jp;
	cdp->cd_last = jp;
	if (cdp->cd_next == (struct job *) cdp)
		cdp->cd_next = jp;
	cdp->cd_batch = (struct job *) cdp;
	
	sc01send(cdp);
	splx(s);

	iowait(bp);
	bp->b_flags &= ~B_BUSY;
	if (bp->b_flags & B_WANTED)
		wakeup((caddr_t)bp);
		
	return(bp->b_flags & B_ERROR);
}


/*
** Function name: sc01config()
** Description:
**	Initializes the cdrom driver's cdrom paramerter structure.
*/

sc01config(cdp)
register struct cdrom *cdp;
{
	register struct capacity *cp;

	cp = &cdp->cd_capacity;

	/* Send TEST UNIT READY */
	if (sc01cmd(cdp, SS_TEST, 0, NULL, 0, 0, SCB_READ, 0, 0))
	{
       		sc01cmn_err(CE_WARN, "SC01: LU %d - Not Ready",
			cdp->cd_addr.sa_lun);
		u.u_error = 0;
		return(-1);
	}
	/* Send READ CAPACITY to obtain last sector address */
	if (sc01cmd(cdp, SM_RDCAP, 0, (char *)cp, RDCAP_SZ, 0, SCB_READ, 0, 0))
	{
       		sc01cmn_err(CE_WARN, "SC01: LU %d - Read capacity error",
			cdp->cd_addr.sa_lun);
		u.u_error = 0;
		return(-1);
	}

	cp->cd_addr = sdi_swap32(cp->cd_addr);
	cp->cd_len  = sdi_swap32(cp->cd_len);

	/*
	 * Check if the parameters are vaild
	 */
	if (cp->cd_addr <= 0)
	{
       		sc01cmn_err(CE_WARN,
			"SC01: LU %d - Last Logic Block Address error",
			cdp->cd_addr.sa_lun);
		return(-1);
	}
	if (cp->cd_len <= 0)
	{
       		sc01cmn_err(CE_WARN, "SC01: LU %d - Block Length error",
			cdp->cd_addr.sa_lun);
		return(-1);
	}

#ifdef	DEBUG
	DPR (3)(CE_CONT, "addr = 0x%x, len = 0x%x\n", cp->cd_addr, cp->cd_len);
#endif
	/*
	 * Indicate parameters are set and valid
	 */
	cdp->cd_state |= CD_PARMS; 
	return(0);
}
	

/*
** Function name: sc01comp()
** Description:
**	On completion of a job, both successful and failed, this function
**	removes the job from the work queue, de-allocates the job structure
**	used, and calls iodone().  The function will restart the logical
**	unit queue if necessary.
*/

sc01comp(jp)
register struct job *jp;
{
        register struct cdrom *cdp;
	register struct buf *bp;
        
        cdp = jp->j_cdp;
        bp = jp->j_bp;

	/* Remove job from the queue */
	jp->j_next->j_prev = jp->j_prev;
	jp->j_prev->j_next = jp->j_next;

	cdp->cd_count--;
	cdp->cd_npend--;

	/* Check if job completed successfully */
	if (jp->j_sb->SCB.sc_comp_code != SDI_ASW)
	{
		bp->b_flags |= B_ERROR;
		if (jp->j_sb->SCB.sc_comp_code == SDI_NOSELE)
			bp->b_error = ENODEV;
		else
			bp->b_error = EIO;
	}

	iodone(bp);
	sc01freejob(jp);

	/* Resume queue if suspended */
	if (cdp->cd_state & CD_SUSP)
	{
		cdp->cd_fltres->sb_type = SFB_TYPE;
		cdp->cd_fltres->SFB.sf_int  = sc01intf;
		cdp->cd_fltres->SFB.sf_dev  = cdp->cd_addr;
		cdp->cd_fltres->SFB.sf_wd = (long) cdp;
		cdp->cd_fltres->SFB.sf_func = SFB_RESUME;
		sdi_icmd(cdp->cd_fltres);

		cdp->cd_state &= ~CD_SUSP;
		cdp->cd_fltcnt = 0;
	}

        sc01send(cdp); 
}


/*
** Function name: sc01intn()
** Description:
**	This function is called by the host adapter driver when a SCB
**	job completes.  If the job completed with an error the job will
**	be retried when appropriate.  Requests which still fail or are
**	not retried are failed.
*/

void
sc01intn(sp)
register struct sb	*sp;
{
	register struct cdrom	*cdp;
	register struct job	*jp;

	jp = (struct job *)sp->SCB.sc_wd;
	cdp = jp->j_cdp;

	if (sp->SCB.sc_comp_code == SDI_ASW) {
		sc01comp(jp);
		return;
	}

	if (sp->SCB.sc_comp_code & SDI_SUSPEND)
		cdp->cd_state |= CD_SUSP;

	if (sp->SCB.sc_comp_code == SDI_CKSTAT && sp->SCB.sc_status == S_CKCON)
	{
		cdp->cd_fltjob = jp;

		cdp->cd_fltreq->sb_type = ISCB_TYPE;
		cdp->cd_fltreq->SCB.sc_int = sc01intrq;
		cdp->cd_fltreq->SCB.sc_cmdsz = SCS_SZ;
		cdp->cd_fltreq->SCB.sc_time = JTIME;
		cdp->cd_fltreq->SCB.sc_mode = SCB_READ;
		cdp->cd_fltreq->SCB.sc_dev = sp->SCB.sc_dev;
		cdp->cd_fltreq->SCB.sc_wd = (long) cdp;
		cdp->cd_fltcmd.ss_op = SS_REQSEN;
		cdp->cd_fltcmd.ss_lun = sp->SCB.sc_dev.sa_lun;
		cdp->cd_fltcmd.ss_addr1 = 0;
		cdp->cd_fltcmd.ss_addr = 0;
		cdp->cd_fltcmd.ss_len = SENSE_SZ;
		cdp->cd_fltcmd.ss_cont = 0;

		/* Clear old sense key */
		cdp->cd_sense.sd_key = SD_NOSENSE;

		sdi_icmd(cdp->cd_fltreq);
		return;
	}

	if (sp->SCB.sc_comp_code & SDI_RETRY && ++jp->j_errcnt <= MAXRETRY)
	{
		sp->sb_type = ISCB_TYPE;
		sp->SCB.sc_time = JTIME;
		sdi_icmd(sp);
		return;
	}

	sc01logerr(cdp, sp);
	sc01comp(jp);
}


/*
** Function name: sc01intrq()
** Description:
**	This function is called by the host adapter driver when a
**	request sense job completes.  The job will be retied if it
**	failed.  Calls sc01sense() on sucessful completions to
**	examine the request sense data.
*/

void
sc01intrq(sp)
register struct sb *sp;
{
	register struct cdrom *cdp;

	cdp = (struct cdrom *)sp->SCB.sc_wd;

	if (sp->SCB.sc_comp_code != SDI_CKSTAT  &&
	    sp->SCB.sc_comp_code &  SDI_RETRY   &&
	    ++cdp->cd_fltcnt <= MAXRETRY)
	{
		sp->SCB.sc_time = JTIME;
		sdi_icmd(sp);
		return;
	}

	if (sp->SCB.sc_comp_code != SDI_ASW) {
		sc01logerr(cdp, sp);
		sc01comp(cdp->cd_fltjob);
		return;
	}

	sc01sense(cdp);
}


/*
** Function name: sc01intf()
** Description:
**	This function is called by the host adapter driver when a host
**	adapter function request has completed.  If there was an error
**	the request is retried.  Used for resume function completions.
*/

void
sc01intf(sp)
register struct sb *sp;
{
	register struct cdrom *cdp;

	cdp = (struct cdrom *)sp->SFB.sf_wd;

	if (sp->SFB.sf_comp_code & SDI_RETRY && ++cdp->cd_fltcnt <= MAXRETRY)
	{
		sdi_icmd(sp);
		return;
	}

	if (sp->SFB.sf_comp_code != SDI_ASW) 
		sc01logerr(cdp, sp);
}


/*
** Function name: sc01sense()
** Description:
**	This function uses the Request Sense information to determine
**	what to do with the original job.
*/

sc01sense(cdp)
register struct cdrom *cdp;
{
	register struct job *jp;
	register struct sb *sp;

	jp = cdp->cd_fltjob;
	sp = jp->j_sb;

        switch(cdp->cd_sense.sd_key)
	{
	case SD_NOSENSE:
	case SD_ABORT:
	case SD_VENUNI:
		sc01logerr(cdp, sp);

	case SD_UNATTEN:
		cdp->cd_state &= ~CD_PARMS;	/* CD-ROM exchanged ? */
		if (++jp->j_errcnt > MAXRETRY)
			sc01comp(jp);
		else {
			sp->sb_type = ISCB_TYPE;
			sp->SCB.sc_time = JTIME;
			sdi_icmd(sp);
		}
		break;

	case SD_RECOVER:
		sc01logerr(cdp, sp);
		sp->SCB.sc_comp_code = SDI_ASW;
		sc01comp(jp);
		break;

	default:
		sc01logerr(cdp, sp);
		sc01comp(jp);
        }
}


/*
** Function name: sc01logerr()
** Description:
**	This function will print the error messages for errors detected
**	by the host adapter driver.  No message will be printed for
**	thoses errors that the host adapter driver has already reported.
*/

sc01logerr(cdp, sp)
register struct cdrom *cdp;
register struct sb *sp;
{
	daddr_t blkno;
	char name[NAMESZ];

	sdi_name(&cdp->cd_addr, name);

	if (sp->sb_type == SFB_TYPE)
	{
		sc01cmn_err(CE_WARN, "SC01: %s LU %d - I/O Error",
			name, sp->SFB.sf_dev.sa_lun);
		sc01cmn_err(CE_CONT, "SDI comp code = 0x%x\n", 
			sp->SFB.sf_comp_code);
		return;
	}

	if (sp->SCB.sc_comp_code == SDI_CKSTAT && sp->SCB.sc_status == S_CKCON)
	{
		sc01cmn_err(CE_WARN, "SC01: %s LU %d - I/O Error",
			name, sp->SCB.sc_dev.sa_lun);

		if (cdp->cd_sense.sd_valid)
		{
			blkno = sdi_swap32(cdp->cd_sense.sd_ba);
			sc01cmn_err(CE_CONT, "Block Address = 0x%x\n", blkno);
		}

		sc01cmn_err(CE_CONT, "Sense Key = 0x%x, Ext Sense = 0x%x\n",
			cdp->cd_sense.sd_key, cdp->cd_sense.sd_sencode);

		return;
	}

	if (sp->SCB.sc_comp_code == SDI_CKSTAT)
	{
		sc01cmn_err(CE_WARN, "SC01: %s LU %d - I/O Error",
			name, sp->SCB.sc_dev.sa_lun);
		sc01cmn_err(CE_CONT, "TC status = 0x%x\n", sp->SCB.sc_status);
	}
}

void
sc01cmn_err(level, fmt, ARG)
int	level;
char	*fmt;
int	ARG;
{

	if (sc01_errmsgflg == 0) {
		cmn_err(level, fmt, ARG);
	} else {
		char		nfmt[256];
		register char	*nfmtp = nfmt;

		*nfmtp++ = '!';
		while(*fmt && (nfmtp < &nfmt[256]))
			*nfmtp++ = *fmt++;
		*nfmtp = NULL;
		cmn_err(level, nfmt, ARG);
	}
}


