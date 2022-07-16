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

#ident	"@(#)scsi.in:io/st01/st01.c	1.3.4.1"

/*
**	SCSI Tape Target Driver.
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
#include "sys/file.h"
#include "sys/open.h"
#include "sys/ioctl.h"
#include "sys/debug.h"
#include "sys/conf.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/kmem.h"
#include "sys/ddi.h"
#include "sys/scsi.h"
#include "sys/sdi.h"
#include "sys/sdi_edt.h"
#include "sys/tape.h"
#include "st01.h"

extern	clock_t	lbolt;

/* Allocated in space.c */
extern 	struct tc_data	St01_data[];	/* Array of TC dev info	    */
extern	unsigned 	St01_datasz;	/* Number of supported TCs  */
extern	long	 	St01_cmajor;	/* Character major number   */
extern	int	 	St01_jobs;	/* Allocation per LU        */

static	int 		st01_tapecnt;	/* Num of tapes configured  */
static	int 		st01_jobcnt;	/* Total allocation of jobs */
static	struct tc_edt  *st01_edt;	/* Array of EDT structures  */
static	struct tape    *st01_tape;	/* Array of Tape structures */
static	struct job     *st01_job;	/* Array of Job structures  */
static	int		st01_errmsgflg;	/* Debug on/off flag	    */

static	struct job     *jbfreelist;	/* List of unallocated jobs */
static	int		jbwant;		/* Process sleep counter */

/* Aliases - see scsi.h */
#define ss_code		ss_addr1
#define ss_mode		ss_addr1
#define ss_parm		ss_len
#define ss_len1		ss_addr
#define ss_len2		ss_len
#define ss_cnt1		ss_addr
#define ss_cnt2		ss_len

#define	GROUP0		0
#define	GROUP1		1
#define	GROUP6		6
#define	GROUP7		7

#define NAMESZ		49

#define	msbyte(x)	(((x) >> 16) & 0xff)
#define	mdbyte(x)	(((x) >>  8) & 0xff)
#define	lsbyte(x)	((x) & 0xff)

void	st01intn();
void	st01intf();
void	st01intrq();
void	st01strategy();




/*
** Function name: st01init()
** Description:
**	Called by kernel to perform driver initialization.
**	This function does not access any devices.
*/

st01init()
{
	register struct	tc_edt *edt_p;	/* pointer to TC EDT	 */
	register struct tape   *tp;	/* Tape pointer		 */
	register struct job    *jp;	/* Job struct pointer	 */

	caddr_t	 base;			/* Base memory pointer	 */

	int  tc_edtsz,			/* EDT size (in bytes)	 */
	     tc_cnt,			/* number of controllers */
	     tapesz,			/* Tape size (in bytes)  */
	     jobsz,			/* Job size (in bytes)	 */
	     i,				/* loop index		 */
	     lu,			/* LU number		 */
	     tc;			/* TC number		 */


	/* Check if SDI has been started */
	if (!sdi_started)
		sdi_init();

	st01_tapecnt = 0;

	/*
	 * Allocate the TC EDT structures
	 */
	tc_edtsz = sdi_hacnt * MAX_TCS * sizeof(struct tc_edt);
	if ((base = kmem_alloc(tc_edtsz, KM_NOSLEEP)) == NULL)
	{
                cmn_err(CE_WARN,
			"ST01: Insufficient memory to configure driver\n");
		return;
	}
	st01_edt = (struct tc_edt *) base;

	/* Get TC EDT data from SDI */
	tc_cnt = sdi_config("ST01", St01_cmajor, 0, St01_data,
				St01_datasz, st01_edt);

	for (tc = 0, edt_p = st01_edt; tc < tc_cnt; tc++, edt_p++) 
		st01_tapecnt += edt_p->n_lus;

	/* Check if there are devices configured */
	if (st01_tapecnt == 0) {
		kmem_free(st01_edt, tc_edtsz);
		return;
	}

	/*
	 * Allocate the tape and job structures
	 */
	tapesz = st01_tapecnt * sizeof(struct tape);
	jobsz  = st01_tapecnt * St01_jobs * sizeof(struct job);
	if ((base = kmem_alloc((tapesz + jobsz), KM_NOSLEEP)) == NULL)
	{
                cmn_err(CE_WARN,
			"ST01: Insufficient memory to configure driver\n");
		kmem_free(st01_edt, tc_edtsz);
		st01_tapecnt = 0;
		return;
	}

	st01_tape = (struct tape *) base;
	st01_job  = (struct job *) (base + tapesz);

	/*
	 * Initialize the tape structures
	 */
	tp = st01_tape;
	for (tc = 0, edt_p = st01_edt; tc < tc_cnt; tc++, edt_p++) 
	{
	    for (i = 0, lu = 0; i < (int)edt_p->n_lus; i++, lu++, tp++)
	    {
	    	for (; lu < MAX_LUS; lu++)	/* Find next equipped LU */
	    		if(edt_p->lu_id[lu])
				break;

		/* Initialize state & counters */
		tp->t_state  = 0;
		tp->t_fltcnt = 0;

		/* Initialize the SCSI address */
		tp->t_addr.sa_exlun = 0;
		tp->t_addr.sa_lun   = lu;
		tp->t_addr.sa_fill  = SDI_DEV(edt_p);

		/* Allocate the fault SBs */
		tp->t_fltreq = sdi_getblk();  /* Request sense */
		tp->t_fltres = sdi_getblk();  /* Resume */
	    }
	}

	/*  Deallocate the TC EDT structure  */
	kmem_free(st01_edt, tc_edtsz);

	st01_jobcnt  = st01_tapecnt * St01_jobs;

	/*  Build list of free job blocks */
	jbfreelist = NULL;
	for (jp = st01_job; jp < st01_job + (st01_jobcnt - 1); jp++) {
		jp->j_next = jbfreelist;
		jbfreelist = jp;
	}
}


/*
** Function name: st01getjob()
** Description:
**	This function will allocate a tape job structure from the free
**	list.  The function will sleep if there are no jobs available.
**	It will then get a SCSI block from SDI.
*/

struct job *
st01getjob()
{
	register struct job *jp;
	register int oip;

	oip = spl5();
	while ( !(jp = jbfreelist)) {
		jbwant++; 
		sleep((caddr_t)&jbfreelist, PRIBIO);
	}
	jbfreelist = jp->j_next;
	splx(oip);

	/* Get an SB for this job */
	jp->j_sb = sdi_getblk();
	return(jp);
}


/*
** Function name: st01freejob()
** Description:
**	This function returns a job structure to the free list. The
**	SCSI block associated with the job is returned to SDI.
*/

st01freejob(jp)
register struct job *jp;
{
	register int  oip;

	sdi_freeblk(jp->j_sb);

	oip = spl5();
	jp->j_next = jbfreelist;
	jbfreelist = jp;
	if (jbwant) {
		jbwant = 0;
		wakeup((caddr_t)&jbfreelist);
	}
	splx(oip);
}

#define ST01IOCTL(cmd, arg) st01ioctl(dev, cmd, arg, oflag, cred_p, (int*)0)

/*
** Function name: st01open()
** Description:
** 	Driver open() entry point.  Opens the device and reserves
**	it for use by that process only.
*/

st01open(devp, oflag, otyp, cred_p)
dev_t	*devp;
cred_t	*cred_p;
int oflag, otyp;
{
	dev_t	dev = *devp;
	register struct tape *tp;
	unsigned unit;

	if (oflag & FAPPEND)
		return(ENXIO);

	unit = UNIT(dev);
	tp = &st01_tape[unit];

	/* Check for non-existent device */
	if (unit >= st01_tapecnt)
		return(ENXIO);

	/* Only one open allowed at a time */
	if (tp->t_state & T_OPENED)
		return(EBUSY);

	tp->t_state &= T_PARMS;		/* clear all bits except T_PARMS */

	tp->t_addr.sa_major = getmajor(dev);
	tp->t_addr.sa_minor = getminor(dev);
	tp->t_buf.b_flags = 0;

	/* Initialize the fault SBs */
	tp->t_fltcnt = 0;
	tp->t_fltreq->sb_type = ISCB_TYPE;
	tp->t_fltreq->SCB.sc_datapt = SENSE_AD(&tp->t_sense);
	tp->t_fltreq->SCB.sc_datasz = SENSE_SZ;
	tp->t_fltreq->SCB.sc_mode   = SCB_READ;
	tp->t_fltreq->SCB.sc_cmdpt  = SCS_AD(&tp->t_fltcmd);
	sdi_translate(tp->t_fltreq, B_READ, 0);

	if (((tp->t_state & T_PARMS) == 0) && st01config(tp))
		return(ENXIO);

	tp->t_state |= T_OPENED;
	if (RETENSION_ON_OPEN(dev))
		ST01IOCTL(T_RETENSION, 0);
	return(0);
}

/*
** Function name: st01close()
** Description:
** 	Driver close() entry point.  If the device has been opened
**	for writing, a file mark will be written.  If the device
**	has been opened to rewind on close, a rewind will be
**	performed; otherwise the tape head will be positioned after
**	the first filemark.
*/

st01close(dev, oflag, otyp, cred_p)
cred_t	*cred_p;
register dev_t dev;
int oflag, otyp;
{
	register struct tape *tp;
	unsigned unit;

	unit = UNIT(dev);
	tp = &st01_tape[unit];

	if (NOREWIND(dev)) {
		/* move past next file mark */
		if ((tp->t_state & T_READ)
		&& !(tp->t_state & (T_FILEMARK | T_TAPEEND)))
			ST01IOCTL(T_SFF, 1);
		/* write filemark */
		if (tp->t_state & T_WRITTEN)
			ST01IOCTL(T_WRFILEM, 1);
	} else {
		/* rewind the tape */
		ST01IOCTL(T_RWD, 0);
		tp->t_state &= ~T_PARMS; 
	}

	tp->t_state &= ~T_OPENED;
	return(0);
}


/*
** Function name: st01strategy()
** Description:
** 	Driver strategy() entry point.  Initiate I/O to the device.
**	This function only checks the validity of the request.  Most
**	of the work is done by st01io().
*/
void
st01strategy(bp)
register struct buf *bp;
{
	register struct tape *tp;
	unsigned unit;

	unit = UNIT(bp->b_dev);
	tp = &st01_tape[unit];

	/*
	 * If the tape drive is configured for fixed size blocks,
	 * check that request is for an integral number of blocks
	 */
	if (tp->t_bsize && (bp->b_bcount % tp->t_bsize > 0)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EINVAL;
		biodone(bp);
		return;
	}

	if (tp->t_state & (T_FILEMARK | T_TAPEEND)) {
		if (tp->t_state & T_TAPEEND) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENOSPC;
		}
		bp->b_resid = bp->b_bcount;
		biodone(bp);
		return;
	}

	if (bp->b_bcount > ST01_MAXSIZE) { 	/* The job is too big to*/
						/* handle all at once	*/
		st01szsplit(bp, st01strategy);
		return;
	}

	bp->b_resid = bp->b_bcount;
	st01io(tp, bp);
}


/*
** Function name: st01read()
** Description:
** 	Driver read() entry point.  Performs a raw read from the 
**	device.  The function calls physio() which locks the user
**	buffer into core.
*/

st01read(dev, uio_p, cred_p)
uio_t	*uio_p;
cred_t	*cred_p;
register dev_t dev;
{
	struct tape *tp;
	unsigned unit;

	unit = UNIT(dev);
	tp = &st01_tape[unit];

	/* Check for non-existent device */
	if (unit >= st01_tapecnt)
		return(ENXIO);

	return(uiophysio(st01strategy, NULL, dev, B_READ, uio_p));
}


/*
** Function name: st01write()
** Description:
** 	Driver write() entry point.  Performs a raw write to the 
**	device.  The function calls physio() which locks the user
**	buffer into core.
*/

st01write(dev, uio_p, cred_p)
uio_t	*uio_p;
cred_t	*cred_p;
register dev_t dev;
{
	struct tape *tp;
	unsigned unit;

	unit = UNIT(dev);
	tp = &st01_tape[unit];

	/* Check for non-existent device */
	if (unit >= st01_tapecnt)
		return(ENXIO);

	return(uiophysio(st01strategy, NULL, dev, B_WRITE, uio_p));
}



/*
** Function name: st01ioctl()
** Description:
**	Driver ioctl() entry point.  Used to implement the following 
**	special functions:
**
**    B_GETTYPE		-  Get bus type and driver name
**    B_GETTYPE		-  Get bus type and driver name
**    C_ERRMSGON	-  System error message ON
**    C_ERRMSGOFF	-  System error message OFF
**
**    T_RWD		-  Rewind tape to BOT
**    T_WRFILEM		-  Write file mark
**    T_SFF/SFB		-  Space filemarks forward/backwards
**    T_SBF/SBB		-  Space blocks forward/backwards
**    T_LOAD/UNLOAD	-  Medium load/unload
**    T_ERASE		-  Erase tape
**    T_RETENSION	-  Retension tape
**    
**    T_PREVMV		-  Prevent medium removal
**    T_ALLOMV		-  Allow medium removal
**    T_RDBLKLEN	-  Read block length limit
**    T_WRBLKLEN	-  Write block length limit
**
*/

st01ioctl(dev, cmd, arg, oflag, cred_p, rval_p)
cred_t	*cred_p;
int	*rval_p;
dev_t dev;
int cmd, oflag;
caddr_t arg;
{
	register struct job *jp;
	struct tape *tp;
	unsigned unit;
	int ret_code = 0;

	unit = UNIT(dev);
	tp = &st01_tape[unit];

	switch(cmd)
	{
	case T_RST:
		/*
		 * Reset the tape drive -- not implemented
		*/
		break;

	case T_RWD:
		/*
		 * Rewind to beginning of tape
		 */
		if (tp->t_state & T_WRITTEN) {
			/* write double filemark */
			ST01IOCTL(T_WRFILEM, 2);
		}
		ret_code = st01cmd(tp, SS_REWIND, 0, NULL, 0, 0, SCB_READ, 0, 0);
		tp->t_state &= ~(T_FILEMARK | T_TAPEEND | T_READ | T_WRITTEN);
		break;

	case T_WRFILEM:
		/*
		 * Write filemarks
		 */
		{
		register int cnt = (int) arg;

		if (cnt < 0)
			return(EINVAL);
		ret_code = st01cmd(tp, SS_FLMRK, cnt>>8, NULL, 0, cnt, SCB_READ, 0, 0);
		tp->t_state |= T_FILEMARK;
		tp->t_state &= ~T_WRITTEN;
		break;
		}

	case T_SFF:	/* Space filemarks forward   */
	case T_SFB:	/* Space filemarks backwards */
	case T_SBF:	/* Space  blocks  forward    */
	case T_SBB:	/* Space  blocks  backwards  */
		{
		register int cnt = (int) arg;
		register int code;

		if (tp->t_state & T_WRITTEN)
			return(EINVAL);

		if (cnt < 0)
			return(EINVAL);

		if (cmd == T_SFF || cmd == T_SFB)
			code = FILEMARKS;
		else
			code = BLOCKS;

		if (cmd == T_SFB || cmd == T_SBB)
			cnt = (-1) * cnt;

		ret_code = st01cmd(tp, SS_SPACE, cnt>>8, NULL, 0, cnt, SCB_READ, code, 0);
		tp->t_state &= ~(T_FILEMARK | T_TAPEEND | T_READ | T_WRITTEN);
		break;
		}

	case T_LOAD:
		/*
		 * Medium load/unload
		 */
		if (tp->t_state & T_WRITTEN) {
			/* write double filemark */
			ST01IOCTL(T_WRFILEM, 2);
		}
		ret_code = st01cmd(tp, SS_LOAD, 0, NULL, 0, LOAD, SCB_READ, 0, 0);
		tp->t_state &= ~(T_FILEMARK | T_TAPEEND | T_READ | T_WRITTEN);
		break;

	case T_UNLOAD:
		if (tp->t_state & T_WRITTEN) {
			/* write double filemark */
			ST01IOCTL(T_WRFILEM, 2);
		}
		ret_code = st01cmd(tp, SS_LOAD, 0, NULL, 0, UNLOAD, SCB_READ, 0, 0);
		tp->t_state &= ~(T_FILEMARK | T_TAPEEND | T_READ | T_WRITTEN);
		break;

	case T_ERASE:
		/*
		 * Erase tape
		 */
		ST01IOCTL(T_RWD, 1);	/* Must rewind first */
		ret_code = st01cmd(tp, SS_ERASE, 0, NULL, 0, 0, SCB_READ, LONG, 0);
		tp->t_state &= ~(T_FILEMARK | T_TAPEEND | T_READ | T_WRITTEN);
		break;

	case T_RETENSION:
		/*
		 * Retension tape
		 */
		if (tp->t_state & T_WRITTEN) {
			/* write double filemark */
			ST01IOCTL(T_WRFILEM, 2);
		}
		ret_code = st01cmd(tp, SS_LOAD, 0, NULL, 0, RETENSION, SCB_READ, 0, 0);
		tp->t_state &= ~(T_FILEMARK | T_TAPEEND | T_READ | T_WRITTEN);
		break;

	case B_GETTYPE:
		/*
		 * Tell user bus and driver name
		 */
		if (copyout("scsi", ((struct bus_type *)arg)->bus_name, 5))
			return(EFAULT);
		if (copyout("st01", ((struct bus_type *)arg)->drv_name, 5))
			return(EFAULT);
		break;

	case B_GETDEV:
		/*
		 * Return pass-thru device major/minor 
		 * to user in arg.
		 */
		{
		dev_t	pdev;

		sdi_getdev(&tp->t_addr, &pdev);
		if (copyout((caddr_t)&pdev, arg, sizeof(pdev)))
			return(EFAULT);
		break;
		}

	case T_ERRMSGON:
		/*
		 * System Error message ON
		 */
		st01_errmsgflg = 0;
		break;

	case T_ERRMSGOFF:
		/*
		 * System Error message OFF
		 */
		st01_errmsgflg = 1;
		break;

	/*
	 * The following ioctls are group 0 commands
	 */
#ifdef T_TESTUNIT
	case T_TESTUNIT:
		/*
		 * Test Unit Ready
		 */
		ret_code = st01cmd(tp, SS_TEST, 0, NULL, 0, 0, SCB_READ, 0, 0);
		break;
#endif

	case T_PREVMV:
		/*
		 * Prevent media removal
		 */
		ret_code = st01cmd(tp, SS_LOCK, 1, NULL, 0, 0, SCB_READ, 0, 0);
		break;

	case T_ALLOMV:
		/*
		 * Allow media removal
		 */
		ret_code = st01cmd(tp, SS_LOCK, 0, NULL, 0, 0, SCB_READ, 0, 0);
		break;

#ifdef T_INQUIR
	case T_INQUIR:
		/*
		 * Inquiry
		 */
		{
		register struct ident *idp;
		idp = (struct ident *)&tp->t_ident;

		if (copyin(arg, (char *)idp, sizeof(struct ident)) < 0)
			return(EFAULT);

		if (ret_code = st01cmd(tp, SS_INQUIR, 0, idp, sizeof(struct ident), 
				sizeof(struct ident), SCB_READ, 0, 0)) {
			break;
		}
		if (copyout((char *)idp, arg, sizeof(struct ident)))
			return(EFAULT);
		}
		break;
#endif

	case T_RDBLKLEN:
		/*
		 * Read block length limit 
		 */
		{
		register struct blklen *cp;

		cp = &tp->t_blklen;
		if (ret_code = st01cmd(tp, SS_RDBLKLEN, 0, cp, RDBLKLEN_SZ, 0,
			SCB_READ, 0, 0))
			break;
		cp->max_blen = sdi_swap24(cp->max_blen);
		cp->min_blen = sdi_swap16(cp->min_blen);
		if (copyout((char *)cp, arg, sizeof(struct blklen)))
			return(EFAULT);
		}
		break;

	case T_WRBLKLEN:
		/*
		 * Write block length limit 
		 */
		{
		register struct blklen *cp;
		register struct mode *mp = &tp->t_mode;

		cp = &tp->t_blklen;
		if (copyin(arg, (char *)cp, sizeof(struct blklen)) < 0)
			return(EFAULT);

		if (cp->max_blen != cp->min_blen)
			return(EINVAL);

		if (ret_code = st01cmd(tp, SS_MSENSE, 0, mp, sizeof(struct mode),
			sizeof(struct mode), SCB_READ, 0, 0))
			break;

		mp->md_len = 0;			/* Reserved field 	*/
		mp->md_media = 0;		/* Reserved field 	*/
		mp->md_wp = 0;			/* Reserved field 	*/
		mp->md_nblks = 0;		/* Reserved field 	*/
		mp->md_bsize = sdi_swap24(cp->max_blen); /* Fix block size */
		if (ret_code = st01cmd(tp, SS_MSELECT, 0, mp, sizeof(struct mode),
			sizeof(struct mode), SCB_WRITE, 0, 0))
			break;

		tp->t_bsize = mp->md_bsize;	/* Note the new block size */
		}
		break;

#ifdef T_MSENSE
	case T_MSENSE:
		/*
		 * Mode sense
		 */
		{
		register struct mode *cp;
		cp = &tp->t_mode;

		if (copyin(arg, (char *)cp, sizeof(struct mode)) < 0)
			return(EFAULT);

		if (ret_code = st01cmd(tp, SS_MSENSE, 0, cp, sizeof(struct mode),
			sizeof(struct mode), SCB_READ, 0, 0))
			break;

		if (copyout((char *)cp, arg, sizeof(struct mode)))
			return(EFAULT);
		}
		break;
#endif
#ifdef T_MSELECT
	case T_MSELECT:
		/*
		 * Mode select
		 */
		{
		register struct mode *cp;
		cp = &tp->t_mode;

		if (copyin(arg, (char *)cp, sizeof(struct mode)) < 0)
			return(EFAULT);

		ret_code = st01cmd(tp, SS_MSELECT, 0, cp, sizeof(struct mode),
			sizeof(struct mode), SCB_WRITE, 0, 0);
		}
		break;
#endif

	default:
		ret_code = EINVAL;
	}

	return(ret_code);
}

/*
** Function name: st01print()
** Description:
**	This function prints the error message provided by the kernel.
*/

int
st01print(dev, str)
dev_t dev;
register char *str;
{
	char name[NAMESZ];
	register struct tape *tp;

	tp = &st01_tape[UNIT(dev)];
	sdi_name(&tp->t_addr, name);
	cmn_err(CE_WARN, "ST01: %s, %s\n", name, str);
	return(0);
}

/*
** Function name: st01io()
** Description:
**	This function creates and sends a SCSI I/O request.
*/

st01io(tp, bp)
register struct tape *tp;
register buf_t *bp;
{
	register struct job *jp;
	register struct scb *scb;
	register struct scs *cmd;
	register struct mode *mp;
	register int nblk;
	int s;

	jp = st01getjob();
	jp->j_tp = tp;
	jp->j_bp = bp;
	mp = &tp->t_mode;

	jp->j_sb->sb_type = SCB_TYPE;
	jp->j_time = JTIME;

	/*
	 * Fill in the scb for this job.
	 */

	scb = &jp->j_sb->SCB;
	scb->sc_cmdpt = SCS_AD(&jp->j_cmd);
	scb->sc_cmdsz = SCS_SZ;
	scb->sc_datapt = bp->b_un.b_addr;
	scb->sc_datasz = bp->b_bcount;
	scb->sc_link = NULL;
	scb->sc_mode = (bp->b_flags & B_READ) ?
			 SCB_READ : SCB_WRITE;

	sdi_translate(jp->j_sb, bp->b_flags, bp->b_proc);

	scb->sc_int = st01intn;
	scb->sc_dev = tp->t_addr;
	scb->sc_time = JTIME;
	scb->sc_wd = (long) jp;

	/*
	 * Fill in the command for this job.
	 */

	cmd = (struct scs *)&jp->j_cmd;
	cmd->ss_op = (bp->b_flags & B_READ) ?
			SS_READ : SS_WRITE;
	cmd->ss_lun  = tp->t_addr.sa_lun;

	if (tp->t_bsize) {	 /* Fixed block transfer mode	*/
		cmd->ss_mode = FIXED;
		nblk = bp->b_bcount / tp->t_bsize;
		cmd->ss_len1 = mdbyte(nblk) << 8 | msbyte(nblk);
		cmd->ss_len2 = lsbyte(nblk);
	} else {		/* Variable block transfer mode */
		cmd->ss_mode = VARIABLE;
		cmd->ss_len1 = mdbyte(bp->b_bcount) << 8 | msbyte(bp->b_bcount);
		cmd->ss_len2 = lsbyte(bp->b_bcount);
	}
	cmd->ss_cont = 0;

	sdi_send(jp->j_sb);	/* send it */
}


/*
** Function name: st01comp()
** Description:
**	Called on completion of a job, both successful and failed.
**	The function de-allocates the job structure used and calls
**	biodone().  Restarts the logical unit queue if necessary.
*/

st01comp(jp)
register struct job *jp;
{
        register struct tape *tp;
	register struct buf *bp;
        
        tp = jp->j_tp;
	bp = jp->j_bp;

	/* Check if job completed successfully */
	if (jp->j_sb->SCB.sc_comp_code == SDI_ASW) {
		bp->b_resid  = 0;
		tp->t_lastop = jp->j_cmd.ss.ss_op;
		if (tp->t_lastop == SS_READ)
			tp->t_state |= T_READ;
		if (tp->t_lastop == SS_WRITE)
			tp->t_state |= T_WRITTEN;
	} else {
		bp->b_flags |= B_ERROR;
		if (jp->j_sb->SCB.sc_comp_code == SDI_NOSELE)
			bp->b_error = ENODEV;
		else
			bp->b_error = EIO;

		if (tp->t_state & (T_FILEMARK | T_TAPEEND)) 
			bp->b_flags &= ~B_ERROR;
	}

	biodone(bp);
	st01freejob(jp);

	/* Resume queue if suspended */
	if (tp->t_state & T_SUSPEND)
	{
		tp->t_fltres->sb_type = SFB_TYPE;
		tp->t_fltres->SFB.sf_int  = st01intf;
		tp->t_fltres->SFB.sf_dev  = tp->t_addr;
		tp->t_fltres->SFB.sf_wd = (long) tp;
		tp->t_fltres->SFB.sf_func = SFB_RESUME;
		sdi_icmd(tp->t_fltres);

		tp->t_state &= ~T_SUSPEND;
		tp->t_fltcnt = 0;
	}

	return;
}


/*
** Function name: st01intn()
** Description:
**	This function is called by the host adapter driver when an 
**	SCB job completes.  If the job completed with an error, the 
**	appropriate error handling is performed.
*/

void
st01intn(sp)
register struct sb *sp;
{
	register struct tape *tp;
	register struct job *jp;

	jp = (struct job *)sp->SCB.sc_wd;
	tp = jp->j_tp;

	if (sp->SCB.sc_comp_code == SDI_ASW) {
		st01comp(jp);
		return;
	}

	if (sp->SCB.sc_comp_code & SDI_SUSPEND)
		tp->t_state |= T_SUSPEND;

	if (sp->SCB.sc_comp_code == SDI_CKSTAT &&
	    sp->SCB.sc_status == S_CKCON)
	{
		tp->t_fltjob = jp;

		tp->t_fltreq->sb_type = ISCB_TYPE;
		tp->t_fltreq->SCB.sc_int = st01intrq;
		tp->t_fltreq->SCB.sc_cmdsz = SCS_SZ;
		tp->t_fltreq->SCB.sc_time = JTIME;
		tp->t_fltreq->SCB.sc_mode = SCB_READ;
		tp->t_fltreq->SCB.sc_dev = sp->SCB.sc_dev;
		tp->t_fltreq->SCB.sc_wd = (long) tp;
		tp->t_fltcmd.ss_op = SS_REQSEN;
		tp->t_fltcmd.ss_lun = sp->SCB.sc_dev.sa_lun;
		tp->t_fltcmd.ss_addr1 = 0;
		tp->t_fltcmd.ss_addr = 0;
		tp->t_fltcmd.ss_len = SENSE_SZ;
		tp->t_fltcmd.ss_cont = 0;

		/* clear old sense key, set EOF */
		tp->t_sense.sd_key = SD_NOSENSE;

		sdi_icmd(tp->t_fltreq);
	}
	else
	{
		/* Fail the job */
		st01logerr(tp, sp);
		st01comp(jp);
	}

	return;
}


/*
** Function name: st01intrq()
** Description:
**	This function is called by the host adapter driver when a
**	request sense job completes.  The job will be retried if it
**	failed.  Calls st01sense() on successful completions to
**	examine the request sense data.
*/

void
st01intrq(sp)
register struct sb *sp;
{
	register struct tape *tp;

	tp = (struct tape *)sp->SCB.sc_wd;

	if (sp->SCB.sc_comp_code != SDI_CKSTAT  &&
	    sp->SCB.sc_comp_code &  SDI_RETRY   &&
	    ++tp->t_fltcnt <= MAX_RETRY)
	{
		sp->SCB.sc_time = JTIME;
		sdi_icmd(sp);
		return;
	}

	if (sp->SCB.sc_comp_code != SDI_ASW) {
		st01logerr(tp, sp);
		st01comp(tp->t_fltjob);
		return;
	}

	st01sense(tp);
}


/*
** Function name: st01intf()
** Description:
**	This function is called by the host adapter driver when a host
**	adapter function request has completed.  If there was an error
**	the request is retried.  Used for resume function completions.
*/

void
st01intf(sp)
register struct sb *sp;
{
	register struct tape *tp;

	tp = (struct tape *)sp->SFB.sf_wd;

	if (sp->SFB.sf_comp_code & SDI_RETRY  &&
	    ++tp->t_fltcnt <= MAX_RETRY)
	{
		sdi_icmd(sp);
		return;
	}

	if (sp->SFB.sf_comp_code != SDI_ASW) 
		st01logerr(tp, sp);
}


/*
** Function name: st01sense()
** Description:
**	Performs error handling based on SCSI sense key values.
*/

st01sense(tp)
register struct tape *tp;
{
	register struct job *jp;
	register struct sb *sp;
	register struct mode *mp;

	jp = tp->t_fltjob;
	sp = jp->j_sb;
	mp = &tp->t_mode;

        switch(tp->t_sense.sd_key) {
	case SD_NOSENSE:
		if (jp->j_cmd.ss.ss_op == SS_READ  ||
		    jp->j_cmd.ss.ss_op == SS_WRITE)
		{
			register struct buf *bp = jp->j_bp;
			register nblks;

			if (tp->t_sense.sd_valid) {
				if (tp->t_bsize) {	/* Fixed Block Len */
					nblks = sdi_swap32(tp->t_sense.sd_ba);
					bp->b_resid = tp->t_bsize * nblks;
				} else			/* Variable	   */
					bp->b_resid = sdi_swap32(tp->t_sense.sd_ba);
			}
			if (tp->t_sense.sd_ili) {
				st01logerr(tp, sp);
				cmn_err(CE_WARN, 
				"ST01: Incorrect Length. Requested = %d, Actual block length = %d\n",
				bp->b_bcount, bp->b_bcount - (int) bp->b_resid);
			/*
			 * The t_sense.sd_ba information field now contains
			 *  (signed) ((bp->b_bcount) - (actual recordsize))
			 *
			 * If the actual record size was larger than the
			 * requested amount, requested data has been read,
			 * but we have skipped over the rest of the record.
			 */
				if ((int) bp->b_resid < 0)
					bp->b_resid = 0;
				bp->b_flags |= B_ERROR;
			}
		}

		if (tp->t_sense.sd_fm)
			tp->t_state |= T_FILEMARK;
		if (tp->t_sense.sd_eom)
			tp->t_state |= T_TAPEEND;

		st01comp(jp);
		break;

	case SD_UNATTEN:
		if (++tp->t_fltcnt > MAX_RETRY) {
			st01logerr(tp, sp);
			st01comp(jp);
		} else {
			sp->sb_type = ISCB_TYPE;
			sp->SCB.sc_time = jp->j_time;
			sdi_icmd(sp);
		}
		break;

	case SD_RECOVER:
		st01logerr(tp, sp);
		sp->SCB.sc_comp_code = SDI_ASW;
		st01comp(jp);
		break;

	/* Some drives give a blank check during positioning */
	case SD_BLANKCK:
		if ((jp->j_cmd.ss.ss_op == SS_READ)
		||  (jp->j_cmd.ss.ss_op == SS_WRITE))
			st01logerr(tp, sp);
		else
			sp->SCB.sc_comp_code = SDI_ASW;
		st01comp(jp);
		break;

	default:
		st01logerr(tp, sp);
		st01comp(jp);
	}
}


struct {
	int printit;
	char *keymsg;
} st01keymsg[] = {
	1,	"SUCCESS",
	1,	"RECOVERED",
	1,	"NOT READY",
	1,	"MEDIUM ERROR",
	1,	"HARDWARE ERROR",
	1,	"ILLEGAL REQUEST",
	1,	"UNIT ATTENTION",
	1,	"DATA PROTECTED",
	1,	"BLANK CHECK",
	1,	"VENDOR",
	1,	"COPY ABORTED",
	1,	"COMMAND ABORTED",
	1,	"EQUAL",
	1,	"VOLUME OVERFLOW",
	1,	"MISCOMPARE",
	1,	"RESERVED",
};


/*
** Function name: st01logerr()
** Description:
**	This function will print the error messages for errors detected
**	by the host adapter driver.  No message will be printed for
**	thoses errors that the host adapter driver has already reported.
*/

st01logerr(tp, sp)
register struct tape *tp;
register struct sb *sp;
{
	char name[NAMESZ];

	sdi_name(&tp->t_addr, name);

	if (sp->sb_type == SFB_TYPE)
	{
		cmn_err(CE_WARN, "ST01: %s LU %d - I/O Error",
			name, sp->SFB.sf_dev.sa_lun);
		cmn_err(CE_CONT, "SDI comp code = 0x%x\n", 
			sp->SFB.sf_comp_code);
		return;
	}

	if (sp->SCB.sc_comp_code == SDI_CKSTAT  &&
	    sp->SCB.sc_status == S_CKCON)
	{
		if (!st01keymsg[tp->t_sense.sd_key].printit)
			return;
		cmn_err(CE_WARN, "ST01: %s LU %d",
			name, sp->SCB.sc_dev.sa_lun);
		cmn_err(CE_CONT, "Sense Key = %s, Ext Sense = 0x%x\n",
			st01keymsg[tp->t_sense.sd_key].keymsg,
			(tp->t_sense.sd_sencode<<8)+tp->t_sense.sd_res5);
		return;
	}

	if (sp->SCB.sc_comp_code == SDI_CKSTAT)
	{
		cmn_err(CE_WARN, "ST01: %s LU %d - I/O Error",
			name, sp->SCB.sc_dev.sa_lun);
		cmn_err(CE_CONT, "TC status = 0x%x\n", 
			sp->SCB.sc_status);
	}
}

/*	 st01szsplit - Splits large transfers.	*/

st01szsplit(obp, strategy)
register buf_t *obp;
void (*strategy)();
{
	register buf_t *bp;
	register int count;
	register int actual;

	bp = (buf_t *) getrbuf(0);
	*bp = *obp;
	count = obp->b_bcount;
	while(count > 0)
	{
		bp->b_bcount = count > ST01_MAXSIZE ? ST01_MAXSIZE : count;
		bp->b_flags &= ~B_DONE;
		strategy(bp);
		biowait(bp);
		actual = bp->b_bcount - bp->b_resid;
		if (bp->b_flags & B_ERROR)
		{
			obp->b_flags |= B_ERROR;
			obp->b_error = bp->b_error;
			break;
		}
		if (actual == 0)
			break;
		bp->b_un.b_addr += actual;
		count -= actual;
	}
	obp->b_resid = count;
	freerbuf(bp);
	biodone(obp);
}

/*
** Function name: st01config()
** Description:
**	Initializes the tape driver's tape parameter structure. To
**	support Autoconfig, the command sequence should be:
**
**		INQUIRY*
**		LOAD
**		TEST UNIT READY
**		MODE SENSE 
**		MODE SELECT*
**
**	Autoconfig is not implemented at this point. In this case, both 
**	INQUIRY and MODE SELECT are not called.	Instead, the T_MSELECT ioctl
**	should be used to set drive parameters. The driver will use whatever
**	settings return by MODE SENSE as default.
*/

st01config(tp)
register struct tape *tp;
{
	register struct blklen *cp;
	register struct mode *mp;

	cp = &tp->t_blklen;

	if (st01cmd(tp, SS_LOAD, 0, NULL, 0, LOAD, SCB_READ, 0, 0)) {
       		cmn_err(CE_WARN, "ST01: LU %d - Load failed",
			tp->t_addr.sa_lun);
		return(ENXIO);
	}
	if (st01cmd(tp, SS_TEST, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
       		cmn_err(CE_WARN, "ST01: LU %d - Not Ready",
			tp->t_addr.sa_lun);
		return(ENXIO);
	}
	/* Send READ BLOCK LIMIT to obtain max/min block length limit */
	if (st01cmd(tp, SS_RDBLKLEN, 0, (char *)cp, RDBLKLEN_SZ, 0, SCB_READ, 0, 0))
	{
       		cmn_err(CE_WARN, "ST01: LU %d - Read block limit error",
			tp->t_addr.sa_lun);
		return(ENXIO);
	}
	cp->max_blen = sdi_swap24(cp->max_blen);
	cp->min_blen = sdi_swap16(cp->min_blen);

	/*
	 * Check if the parameters are valid
	 */
	if (cp->max_blen < 0)
	{
       		cmn_err(CE_WARN,
			"ST01: LU %d - Max Block Length 0x%x error",
			tp->t_addr.sa_lun, cp->max_blen);
		return(ENXIO);
	}
	if (cp->min_blen < 0)
	{
       		cmn_err(CE_WARN, "ST01: LU %d - Min Block Length 0x%x error",
			tp->t_addr.sa_lun, cp->min_blen);
		return(ENXIO);
	}

	/* Send Mode Sense	*/
	mp = &tp->t_mode;
	if (st01cmd(tp, SS_MSENSE, 0, mp, sizeof(struct mode),
		sizeof(struct mode), SCB_READ, 0, 0)) {
		cmn_err(CE_WARN, "ST01: LU %d - Mode sense error",
		tp->t_addr.sa_lun);
		return(ENXIO);
	}
	mp->md_bsize = sdi_swap24(mp->md_bsize);
	if ((mp->md_bsize < cp->min_blen) && (mp->md_bsize != 0)) {
       		cmn_err(CE_WARN,
	 "ST01: LU %d - block size %d invalid, minimum block length = %d",
 			tp->t_addr.sa_lun, mp->md_bsize, cp->min_blen);
		return(ENXIO);
	}
	if ((mp->md_bsize > cp->max_blen) && (mp->md_bsize != 0)
			&& (cp->max_blen !=0)) {
       		cmn_err(CE_WARN, 
	 "ST01: LU %d - block size %d invalid, maximum block length = %d",
			 tp->t_addr.sa_lun, mp->md_bsize, cp->max_blen);
		return(ENXIO);
	}
	tp->t_bsize = mp->md_bsize;

	/*
	 * Indicate parameters are set and valid
	 */
	tp->t_state |= T_PARMS; 
	return(0);
}

/*
** Function name: st01cmd()
** Description:
**	This function performs a SCSI command such as Mode Sense on
**	the addressed tape.  The op code indicates the type of job
**	but is not decoded by this function.  The data area is
**	supplied by the caller and assumed to be in kernel space. 
**	This function will sleep.
*/

st01cmd(tp, op_code, addr, buffer, size, length, mode, param, control)
register struct tape	*tp;
unsigned char	op_code;		/* Command opcode		*/
unsigned int	addr;			/* Address field of command 	*/
char		*buffer;		/* Buffer for command data 	*/
unsigned int	size;			/* Size of the data buffer 	*/
unsigned int	length;			/* Block length in the CDB	*/
unsigned short	mode;			/* Direction of the transfer 	*/
unsigned int	param;
unsigned int	control;
{
	register struct job *jp;
	register struct scb *scb;
	register buf_t *bp;

	bp = &tp->t_buf;
	while (bp->b_flags & B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PRIBIO);
	}
	bp->b_flags = B_BUSY;

	jp = st01getjob();
	scb = &jp->j_sb->SCB;

	bp->b_blkno = addr;
	bp->b_flags |= mode & SCB_READ ? B_READ : B_WRITE;
	bp->b_error = 0;

	jp->j_bp = bp;
	jp->j_tp = tp;
	jp->j_sb->sb_type = SCB_TYPE;

	switch(op_code >> 5){
	case	GROUP7:
	{
		register struct scm *cmd;

		cmd = (struct scm *)&jp->j_cmd.sm;
		cmd->sm_op   = op_code;
		cmd->sm_lun  = tp->t_addr.sa_lun;
		cmd->sm_res1 = 0;
		cmd->sm_addr = sdi_swap32(addr);
		cmd->sm_res2 = 0;
		cmd->sm_len  = sdi_swap16(length);
		cmd->sm_res1 = param;
		cmd->sm_cont = control;

		scb->sc_cmdpt = SCM_AD(cmd);
		scb->sc_cmdsz = SCM_SZ;
	}
		break;
	case	GROUP6:
	{
		register struct scs *cmd;

		cmd = (struct scs *)&jp->j_cmd.ss;
		cmd->ss_op    = op_code;
		cmd->ss_lun   = tp->t_addr.sa_lun;
		cmd->ss_addr1 = ((addr & 0x1F0000) >> 16);
		cmd->ss_addr  = sdi_swap16(addr & 0xFFFF);
		cmd->ss_len   = length;
		cmd->ss_cont  = control;

		scb->sc_cmdpt = SCS_AD(cmd);
		scb->sc_cmdsz = SCS_SZ;
	}
		break;
	case	GROUP1:
	{
		register struct scm *cmd;

		cmd = (struct scm *)&jp->j_cmd.sm;
		cmd->sm_op   = op_code;
		cmd->sm_lun  = tp->t_addr.sa_lun;
		cmd->sm_res1 = param;
		cmd->sm_addr = sdi_swap32(addr);
		cmd->sm_res2 = 0;
		cmd->sm_len  = sdi_swap16(length);
		cmd->sm_cont = 0;

		scb->sc_cmdpt = SCM_AD(cmd);
		scb->sc_cmdsz = SCM_SZ;
	}
		break;
	case	GROUP0:
	{
		register struct scs *cmd;

		cmd = (struct scs *)&jp->j_cmd.ss;
		cmd->ss_op    = op_code;
		cmd->ss_lun   = tp->t_addr.sa_lun;
		cmd->ss_addr1 = param;
		cmd->ss_addr  = sdi_swap16(addr & 0xFFFF);
		cmd->ss_len   = length;
		cmd->ss_cont  = 0;

		scb->sc_cmdpt = SCS_AD(cmd);
		scb->sc_cmdsz = SCS_SZ;
	}
		break;
	default:
		cmn_err(CE_WARN,"ST01 : Unknown op_code = %x\n",op_code);
	}

	/* Fill in the SCB */
	scb->sc_int = st01intn;
	scb->sc_dev = tp->t_addr;
	scb->sc_datapt = buffer;
	scb->sc_datasz = size;
	scb->sc_mode = mode;
	scb->sc_resid = 0;
	scb->sc_time = 120 * ONE_MIN;
	scb->sc_wd = (long) jp;
	sdi_translate(jp->j_sb, bp->b_flags, (caddr_t)0);

	sdi_send(jp->j_sb);

	biowait(bp);
	bp->b_flags &= ~B_BUSY;
	if (bp->b_flags & B_WANTED)
		wakeup((caddr_t)bp);

	if (bp->b_flags & B_ERROR)
		return(bp->b_error);
	else
		return(0);
}
