#ident	"@(#)ad1542.c	1.3	92/12/21	JPB"

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

#ident	"@(#)scsi.in:io/scsi/ad1542.c	1.4.2.3"
#ident "$Header: ad1542.c 2.2 91/01/09 $"

/*
**	SCSI Host Adapter Driver for Adaptec AHA-1542A
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
#if	(_SYSTEMENV == 4)
#include "sys/mkdev.h"
#include "sys/conf.h"
#include "sys/cred.h"
#include "sys/uio.h"
#include "sys/kmem.h"
#endif
#include "sys/ddi.h"
#include "sys/bootinfo.h" 
#include "sys/debug.h"
#include "sys/scsi.h"
#include "sys/sdi_edt.h"
#include "sys/sdi.h"
#include "ad1542.h"

#if	(_SYSTEMENV == 4)
extern	clock_t	lbolt;
#endif

/* Allocated in space.c */
extern int		scsi_chan;	    /* DMA channnel */
extern char 		*scsi_elist[];	    /* list of embedded ctls */
extern long		sc_major;	    /* SCSI driver major #	*/
extern unsigned int	sc_hacnt;	    /* Total # of controllers   */
extern unsigned int	sc_sbcnt;	    /* Total # of SCSI blocks	*/
extern struct scsi_edt	sc_edt[][MAX_TCS];  /* SCSI edt data 		*/
extern struct scsi_cfg	sc_cfg[];	    /* SCSI HA configuration	*/
extern struct scsi_ha	sc_ha[];	    /* SCSI HA structures	*/
extern sblk_t 		sc_sbtab[];	    /* SCSI block pool		*/
extern int		sc_hiwat;	    /* LU Q high water mark	*/
extern int		sc_lowat;	    /* LU Q low water mark	*/
extern struct ver_no	sdi_ver;	    /* SDI version structure	*/
extern long		sdi_started;	    /* SDI initialization flag	*/

static dma_t		dmalist[NDMA];	    /* DMA list pool		*/
static dma_t		*dfreelist;	    /* List of free DMA lists	*/
static sblk_t		*sfreelist;	    /* List of free SCSI blocks	*/
static struct buf	sc_buf;		    /* Local buffer for comands	*/
static char		sc_cmd[MAX_CMDSZ];  /* SCSI commands		*/
static char		init_time;	    /* Init time (poll) flag	*/
static struct ident	inq_data;	    /* Inquiry data 		*/
char 			scsi_except[MAX_NCTL][MAX_TCS];
					    /* embedde cntls which allow
					       no inquiry on other luns
					       e.g. TANDBERG TDC36XX    */



struct ccb  *scsi_getblk();
void	scsi_freeblk();
void	scsi_init_cbs();
void	scsi_pass_thru();
void	scsi_int();
void	scsi_putq();
void	scsi_next();
void	scsi_cmd();
void	scsi_func();
void	scsi_send();
void	scsi_done();
void	scsi_mkedt();
void	scsi_timer();
void	ha_init();
void	ha_done();



/*
** Function name: scsiinit()
** Description:
**	Called by kernel to perform driver initialization
**	before the kernel data area has been initialized.
*/

void
scsiinit()
{
	extern void  sdi_init();

	if (!sdi_started)
		sdi_init();
}


/*
** Function name: scsistart()
** Description:
**	Called by kernel to perform driver initialization
**	after the kernel data area has been initialized.
*/

void
scsistart()
{
	register int	c, t;

	/*
	 * The following code walks thru the edt table
	 * looking for orphan scsi devices. If found, the
	 * user is warned that they won't be configured.
	 */ 
	for (c = 0; c < sc_hacnt; c++)
	{
	    for (t = 0; t < MAX_TCS; t++)
	    {
		if ((sc_edt[c][t].tc_equip) && 
		    (sc_edt[c][t].b_maj == -1) && 
		    (sc_edt[c][t].c_maj == -1))
		{
			sc_edt[c][t].drv_name[0] = 'V';
			sc_edt[c][t].drv_name[1] = 'O';
			sc_edt[c][t].drv_name[2] = 'I';
			sc_edt[c][t].drv_name[3] = 'D';
			sc_edt[c][t].drv_name[4] = NULL;

			cmn_err(CE_WARN, "SCSI: HA %d TC %d - \"%s\" is",
					c, t, sc_edt[c][t].tc_inquiry); 
			cmn_err(CE_CONT,"\t unsupported and NOT configured.\n");
		}
	    }
	}

	/*
	 * Clear init time flag to stop the HA driver
	 * from polling for interrupts and begin taking
	 * interrupts normally.
	 */
	init_time = FALSE;
}


/*
** Function name: scsiopen()
** Description:
** 	Driver open() entry point. It checks permissions, and in the
**	case of a pass-thru open, suspends the particular LU queue.
*/

int
#if	(_SYSTEMENV == 4)
scsiopen(devp, flags, otype, cred_p)
dev_t	*devp;
cred_t	*cred_p;
#else
scsiopen(dev, flags, otype)
#endif
int	flags;
int	otype;
{
#if	(_SYSTEMENV == 4)
	dev_t	dev = *devp;
#endif
	register int	c = SC_HAN(dev);
	register int	t = SC_TCN(dev);
	register int	l = SC_LUN(dev);
	register struct scsi_lu *q;
	int  oip;

	/*
	 * The following code is used to allow mkdev
	 * to do a Read EDT and a HA Count before any
	 * nodes have been built.
	 */
	if (c == 7)
		return(0);
	else {
		if (SC_ILLEGAL(c, t)) {
			u.u_error = ENXIO;
			return(ENXIO);
		}
	}
#if	(_SYSTEMENV == 4)
	if (cred_p->cr_uid != 0) {
#else
	if (u.u_uid != 0) {
#endif
		u.u_error = EPERM;
		return(EPERM);
	}
	if (t == sc_ha[c].ha_id)
		return(0);

	/* This is the pass-thru section */

	q = &LU_Q(c, t, l);

	oip = spl5();
 	if ((q->q_count > 0)  || 
	    (q->q_flag & (QBUSY | QSUSP | QPTHRU)))
	{
		splx(oip);
		u.u_error = EBUSY;
		return(EBUSY);
	}

	q->q_flag |= QPTHRU;
	splx(oip);
	return(0);
}


/*
** Function name: scsiclose()
** Description:
** 	Driver close() entry point.  In the case of a pass-thru close
**	it resumes the queue and calls the target driver event handler
**	if one is present.
*/

int
#if	(_SYSTEMENV == 4)
scsiclose(dev, flags, otype, cred_p)
cred_t	*cred_p;
#else
scsiclose(dev, flags, otype)
#endif
dev_t	dev;
int	flags;
int	otype;
{
	register int	c = SC_HAN(dev);
	register int	t = SC_TCN(dev);
	register int	l = SC_LUN(dev);
	register struct scsi_lu *q;
	int  oip;

	/*
	 * The following code is used to allow mkdev
	 * to do a Read EDT and a HA Count before any
	 * nodes have been built.
	 */
	if (c == 7)
		return(0);
	else {
		if (SC_ILLEGAL(c, t)) {
			u.u_error = ENXIO;
			return(ENXIO);
		}
	}

	if (t == sc_ha[c].ha_id)
		return(0);

	q = &LU_Q(c, t, l);

	oip = spl5();
	q->q_flag &= ~QPTHRU;

	if (q->q_func != NULL)
		(*q->q_func) (q->q_param, SDI_FLT_PTHRU);

	scsi_next(q);
	splx(oip);
	return(0);
}



/*
** Function name: scsiioctl()
** Description:
**	Driver ioctl() entry point.  Used to implement the following 
**	special functions:
**
**	SDI_SEND     -	Send a user supplied SCSI Control Block to
**			the specified device.
**	B_GETTYPE    -  Get bus type and driver name
**	B_HA_CNT     -	Get number of HA boards configured
**	REDT	     -	Read the extended EDT from RAM memory
**	SDI_BRESET   -	Reset the specified SCSI bus 
*/

int
#if	(_SYSTEMENV == 4)
scsiioctl(dev, cmd, arg, mode, cred_p, rval_p)
cred_t	*cred_p;
int	*rval_p;
#else
scsiioctl(dev, cmd, arg, mode, cred_p, rval_p)
#endif
dev_t	dev;
int	cmd;
caddr_t	arg;
int	mode;
{
	register int	c = SC_HAN(dev);
	register int	t = SC_TCN(dev);
	register int	l = SC_LUN(dev);
	register struct scsi_lu *q;
	register struct sb *sp;
	int  oip;

	/*
	 * The following code is used to allow mkdev
	 * to do a Read EDT and a HA Count before any
	 * nodes have been built.
	 */
	switch(cmd) {
	case B_HA_CNT:
		if (copyout((caddr_t)&sdi_hacnt, arg, sizeof(long)))
			u.u_error = EFAULT;
		return(u.u_error);

	case B_REDT:
		{
		unsigned edtsz;

		edtsz = sdi_hacnt * MAX_TCS * sizeof(struct scsi_edt);
		if (copyout((caddr_t)sc_edt, arg, edtsz))
			u.u_error = EFAULT;
		return(u.u_error);
		}
	default:
		/* Other ioctls require validation of c and t */
		if (SC_ILLEGAL(c, t)) {
			u.u_error = ENXIO;
			return(ENXIO);
		}
	}

	/*
	 *	Now we know there is a host adaptor, do other ioctls
	*/
	switch(cmd)
	{
	case SDI_SEND: {
		register buf_t *bp = &sc_buf;
		struct sb  karg;
		int  rw;

		if (t == sc_ha[c].ha_id) { 	/* illegal ID */
			u.u_error = ENXIO;
			break;
		}
		if (copyin(arg, (caddr_t)&karg, sizeof(struct sb))) {
			u.u_error = EFAULT;
			break;
		}
		if ((karg.sb_type != ISCB_TYPE) ||
		    (karg.SCB.sc_cmdsz <= 0 )   ||
		    (karg.SCB.sc_cmdsz > MAX_CMDSZ ))
		{ 
			u.u_error = EINVAL;
			break;
		}

		sp = sdi_getblk();
		bcopy((caddr_t)&karg, (caddr_t)sp, sizeof(struct sb));

		oip = spl5();
		while (bp->b_flags & B_BUSY)
			sleep((caddr_t)bp, PRIBIO);

		sp->SCB.sc_wd = (long)bp;
		sp->SCB.sc_cmdpt = sc_cmd;

		if (copyin(karg.SCB.sc_cmdpt, sp->SCB.sc_cmdpt,
				sp->SCB.sc_cmdsz))
		{
			u.u_error = EFAULT;
			goto done;
		}

		rw = (sp->SCB.sc_mode & SCB_READ) ? B_READ : B_WRITE;
		bp->b_resid = (long)sp;

		/*
		 * If the job involves a data transfer then the
		 * request is done thru physio() so that the user
		 * data area is locked in memory. If the job doesn't
		 * involve any data transfer then scsi_pass_thru()
		 * is called directly.
		 */
		if (sp->SCB.sc_datasz > 0)
		{ 
#if	(_SYSTEMENV == 4)
			struct iovec  ha_iov;
			struct uio    ha_uio;

			ha_iov.iov_base = sp->SCB.sc_datapt;	
			ha_iov.iov_len = sp->SCB.sc_datasz;	
			ha_uio.uio_iov = &ha_iov;
			ha_uio.uio_iovcnt = 1;
			ha_uio.uio_offset = 0;
			ha_uio.uio_segflg = UIO_USERSPACE;
			ha_uio.uio_fmode = 0;
			ha_uio.uio_resid = sp->SCB.sc_datasz;

			u.u_error = physiock(scsi_pass_thru, bp, dev, rw,
						256, &ha_uio);
#else
			u.u_count = sp->SCB.sc_datasz;
			u.u_base = sp->SCB.sc_datapt;
			u.u_offset = NULL;
			physio(scsi_pass_thru, bp, dev, rw);
#endif
		}
		else
		{
			bp->b_un.b_addr = sp->SCB.sc_datapt;
			bp->b_bcount = sp->SCB.sc_datasz;
			bp->b_blkno = NULL;
			bp->b_dev = dev;
			bp->b_flags = B_BUSY | B_PHYS | rw;

			scsi_pass_thru(bp);  /* fake physio call */
			iowait(bp);
		}

		/* update user SCB fields */

		karg.SCB.sc_comp_code = sp->SCB.sc_comp_code;
		karg.SCB.sc_status = sp->SCB.sc_status;
		karg.SCB.sc_time = sp->SCB.sc_time;

		if (copyout((caddr_t)&karg, arg, sizeof(struct sb)))
			u.u_error = EFAULT;

	   done:
		sdi_freeblk(sp);
		splx(oip);
		break;
		}

	case B_GETTYPE:
		if (copyout("scsi",
		    ((struct bus_type *)arg)->bus_name, 5))
		{
			u.u_error = EFAULT;
			break;
		}
		if (copyout("scsi",
		    ((struct bus_type *)arg)->drv_name, 5))
		{
			u.u_error = EFAULT;
		}
		break;

	case SDI_BRESET: {
		register struct scsi_ha *ha = &sc_ha[c];

		oip = spl5();
		if (ha->ha_npend > 0)     /* jobs are outstanding */
			u.u_error = EBUSY;
		else
		{
			cmn_err(CE_WARN, "SCSI: HA %d - Bus reset\n", c);
			outb(ha->ha_base + HA_CNTL, HA_SBR);
		}
		splx(oip);
		break;
		}

	default:
		u.u_error = EINVAL;
	}

	return(u.u_error);
}


/*
** Function name: scsiintr()
** Description:
**	Driver interrupt handler entry point.  Called by kernel when
**	a host adapter interrupt occurs.
*/

void
scsiintr(vect)
unsigned int  vect;	/* HA interrupt vector	*/
{
	register struct scsi_ha	 *ha;
	register struct ccb 	 *cp;
	register struct mbx	 *mp;
	register int		 c,n;
	unsigned long		 addr;

	/*
	 * Determine which host adapter interrupted from
	 * the interrupt vector.
	 */
	for (c = 0; c < sc_hacnt; c++) {
		ha = &sc_ha[c];
		if (ha->ha_vect == vect)
			break;
	}

	if (c == sc_hacnt   ||
	   (inb(ha->ha_base + HA_ISR) & HA_INTR) == 0)
	{
		cmn_err(CE_WARN, "SCSI: HA spurious interrupt\n");
		return;
	}

	outb((ha->ha_base + HA_CNTL), HA_IACK); /* acknowledge */

	/*
	 * Search for an incoming mailbox that is not empty.
	 * A round-robin search algorithm is used.
	 */
	n  = NMBX;
	mp = ha->ha_take;
	do {
		if (mp->m_stat != EMPTY)
			break;
		if (++mp == &ha->ha_mbi[NMBX])
			mp = ha->ha_mbi;
	} while (--n);

	if (n == 0)
		cmn_err(CE_WARN, "SCSI: HA %d - Incoming message not found\n", c);
	/*
	 * Repeat until no outstanding incoming messages
	 * are left before exiting the interrupt routine.
	 */
	while (mp->m_stat != EMPTY)
	{
		switch (mp->m_stat)
		{
		case SUCCESS:
		case FAILURE:
		case ABORTED:
			addr = sdi_swap24((int)mp->m_addr);
			cp = (struct ccb *) xphystokv(addr);

			cp->c_active = FALSE;
			ha->ha_npend--;

			if ((cp->c_opcode == SCSI_CMD) || 
			    (cp->c_opcode == SCSI_DMA_CMD))
				scsi_done(c, cp, mp->m_stat);
			else 
				ha_done(c, cp, mp->m_stat);

			break;

		case NOT_FND:	/* CB presented before timeout abort */
			break;

		default:
			cmn_err (CE_WARN,
				"SCSI: HA %d - Unexpected MBI status 0x%x\n",
				 c, mp->m_stat);
		}

		/*
		 * Mark mail box as empty and advance 
		 * to the next incoming mail box.
		 */
		mp->m_stat = EMPTY;
		if (++mp == &ha->ha_mbi[NMBX])
			mp = ha->ha_mbi;
	}

	ha->ha_take = mp;   /* save pointer value */
}


/*
** Function name: scsi_done()
** Description:
**	This is the interrupt handler routine for SCSI jobs which have
**	a controller CB and a SCB structure defining the job.
*/

void
scsi_done(c, cp, status)
int		c;	  /* HA controller number */
int		status;	  /* HA mbx status */
struct ccb      *cp;	  /* command block */
{
	register struct scsi_lu  *q;
	register struct sb *sp;
	char	t,l;

	t = (cp->c_dev >> 5);
	l = (cp->c_dev & 0x07);
	q = &LU_Q(c, t, l);

	sp = cp->c_bind;

	ASSERT(sp);

	q->q_flag &= ~QSENSE;	/* Old sense data now invalid */

	/* Determine completion status of the job */
	switch (status)
	{
	case SUCCESS:
		sp->SCB.sc_comp_code = SDI_ASW;
		break;

	case FAILURE:
		if (cp->c_hstat == NO_ERROR)	/* good HA status */
		{
			if (cp->c_tstat == S_GOOD) {
				sp->SCB.sc_comp_code = SDI_ASW;
			} else {
				sp->SCB.sc_comp_code = SDI_CKSTAT;
				sp->SCB.sc_status = cp->c_tstat;
				/* Cache request sense info (note padding) */
				bcopy((caddr_t)(cp->c_cdb+cp->c_cmdsz),
					(caddr_t)(&q->q_sense)+1,
					sizeof(q->q_sense)-1);
				q->q_flag |= QSENSE;
			}
		}
		else if (cp->c_hstat == NO_SELECT)
			sp->SCB.sc_comp_code = SDI_NOSELE;
		else if (cp->c_hstat == TC_PROTO)
			sp->SCB.sc_comp_code = SDI_TCERR; 
		else {
			cmn_err(CE_WARN, "HA error code 0x%x on command 0x%x\n", cp->c_hstat, cp->c_opcode);
			sp->SCB.sc_comp_code = SDI_HAERR;
		}
		break;

	case ABORTED:
		if (cp->c_time > 0)
			sp->SCB.sc_comp_code = SDI_ABORT;
		else 
			sp->SCB.sc_comp_code = SDI_TIME;
		/*
		 * We have aborted a command, most probably because
		 * either a target device or the SCSI bus had hung;
		 * Send a Bus Device Reset to the target controller
		 */
		{
			register struct sb *sp;

			sp = sdi_getblk();
			sp->sb_type = SFB_TYPE;
			sp->SFB.sf_func = SFB_RESETM;
			sp->SFB.sf_int = NULL;
			sp->SFB.sf_dev.sa_lun  = l;
			sp->SFB.sf_dev.sa_fill = ((c << 3) | t);
			scsi_putq(q, (sblk_t *)sp);
		}
		break;

	default:
		ASSERT(0);
		return;
	}

	sp->SCB.sc_time = ((lbolt-cp->c_start)+HZ-1) / HZ;

	if ((sp->SCB.sc_comp_code & SDI_SUSPEND) && 
	    (sp->SCB.sc_int != scsi_int))
	{
		q->q_flag |= QSUSP;
	}

	/* call target driver interrupt handler */
	if (sp->SCB.sc_int)
		(*sp->SCB.sc_int) (sp);

	scsi_freeblk(c, cp);

	q->q_flag &= ~QBUSY;
	scsi_next(q);
} 


/*
** Function name: ha_done()
** Description:
**	This is the interrupt handler routine for SCSI jobs which have
**	a controller CB and a SFB structure defining the job.
*/

void
ha_done(c, cp, status)
int		c;	  /* HA controller number */
int		status;	  /* HA mbx status */
struct ccb      *cp;	  /* command block */
{
	register struct scsi_lu  *q;
	register struct sb *sp;
	char	t,l;

	t = (cp->c_dev >> 5);
	l = (cp->c_dev & 0x07);
	q = &LU_Q(c, t, l);

	sp = cp->c_bind;

	ASSERT(sp);

	/* Determine completion status of the job */

	switch (status)
	{
	case SUCCESS:
		sp->SFB.sf_comp_code = SDI_ASW;
		scsi_flushq(q, SDI_CRESET, 0);
		break;
	case FAILURE:
		sp->SFB.sf_comp_code = SDI_HAERR;
		q->q_flag |= QSUSP;
		break;
	default:
		ASSERT(0);
		return;
	}

	/* call target driver interrupt handler */
	if (sp->SFB.sf_int)
		(*sp->SFB.sf_int) (sp);

	scsi_freeblk(c, cp);

	q->q_flag &= ~QBUSY;
	scsi_next(q);
} 



/*
** Function name: scsi_timer()
** Description:
**	Scheduled at minute intervals to perform timing.  Callout
**	routines (like this one) run at hi priority.  No spl necessary.
*/

void
scsi_timer(c)
{
	register struct ccb *cp;
	register int n;

	cp = &sc_ha[c].ha_ccb[0];
	for (n = 0; n < NMBX; n++, cp++)
	{
		if (!cp->c_active)	/*  not pending	 */
			continue;	/*      or       */
		if (cp->c_time == 0) 	/*  not timed    */
			continue;
		
		if ((cp->c_time + cp->c_start) > lbolt)
			continue;

		/* Job timed out - terminate the command */

		cp->c_time = 0;
		scsi_send(c, ABORT, cp);
	}

	(void) timeout(scsi_timer, (caddr_t)c, 60*HZ);
}


/*===========================================================================*/
/* SCSI Driver Interface (SDI-386) Functions
/*===========================================================================*/


/*
** Function name: sdi_init()
** Description:
**	This is the initialization routine for the SCSI HA driver.
**	All data structures are initialized, the boards are initialized
**	and the EDT is built for every HA configured in the system.
*/

void
sdi_init()
{
	register struct scsi_ha *ha;
	register struct scsi_lu *q;
	register sblk_t *sp;
	register dma_t  *dp;
	int  c, i;

	if (sdi_started)
		return;

	sdi_started = TRUE;
	init_time   = TRUE;

	sdi_ver.sv_release = 1;
	sdi_ver.sv_machine = SDI_386_AT;
	sdi_ver.sv_modes   = SDI_BASIC1;

	sc_buf.b_flags = 0;

	for (c = 0; c < sc_hacnt; c++)
	{
		ha = &sc_ha[c];
		ha->ha_state  = 0;
		ha->ha_id     = sc_cfg[c].ha_id;
		ha->ha_base   = sc_cfg[c].io_base;
		ha->ha_vect   = sc_cfg[c].ivect;
		ha->ha_npend  = 0;
		ha->ha_give   = &ha->ha_mbo[0];
		ha->ha_take   = &ha->ha_mbi[0];
		ha->ha_cblist = NULL;

		for (i = 0; i < MAX_EQ; i++)
		{
			q = &ha->ha_dev[i];
			q->q_first = NULL;
			q->q_last  = NULL;
			q->q_count = 0;
			q->q_flag  = 0;
			q->q_func  = NULL;
		}

		scsi_init_cbs(c);
	}

	/* Build list of free SCSI blocks */
	sfreelist = NULL;
	for (i = 0; i < sc_sbcnt; i++)
	{
		sp = &sc_sbtab[i];
		sp->s_next = sfreelist;
		sfreelist = sp;
	}

	/* Build list of free DMA lists */
	dfreelist = NULL;
	for (i = 0; i < NDMA; i++)
	{
		dp = &dmalist[i];
		dp->d_next = dfreelist;
		dfreelist = dp;
	}

	for (c = 0; c < sc_hacnt; c++)
	{
		ha_init(c);	 /* initialize HA communication */
		scsi_mkedt(c);	 /* build edt */
		scsi_timer(c);	 /* start timer */
	}
}


/*
** Function name: sdi_config()
** Description:
**	The target drivers pass to this function a pointer to their
**	tc_data structure which contains the inquiry strings of the
**	devices they support. This routine walks through the EDT data
**	searching for the inquiry strings that match. It returns the
**	number of TCs found, and for each TC a tc_edt structure is
**	populated.
**
**	One special case is the configuration for the primary disk driver,
**	SD01, where all RANDOM type targets with logical units are assumed
**	to be SCSI disks that need to be configured even if not supported.
*/

sdi_config (drv_name, c_maj, b_maj, tc_data, tc_size, tc_edtp)
char		*drv_name;	/* driver ASCII name	*/
int		 c_maj;		/* character major num 	*/
int		 b_maj;		/* block major num   	*/
struct tc_data	*tc_data;	/* TC inquiry data	*/
int		 tc_size;	/* TC data size 	*/
struct tc_edt	*tc_edtp;	/* pointer to TC edt	*/
{
	register int 	 c, t, l;
	register int	 i, j;
	struct tc_data	*tc_p;
	int		 tc_count = 0;
	char		 found, *np;

	for (c = 0; c < sc_hacnt; c++)
	{
	    for (t = 0; t < MAX_TCS; t++)
	    {
		if (t == sc_ha[c].ha_id || !(sc_edt[c][t].tc_equip))
			continue;

		found = FALSE;
		for (i = 0, tc_p = tc_data; i < tc_size; i++, tc_p++)
		{
			if (scsi_comp(sc_edt[c][t].tc_inquiry, 
					tc_p->tc_inquiry, INQ_LEN) == 0)
			{
				found = TRUE;
				break;
			}
		}
		if (!found &&
			sc_edt[c][t].pdtype == RANDOM &&
			sc_edt[c][t].n_lus &&
			scsi_comp(drv_name, "SD01", 4) == 0)
		{
			found = TRUE;
			cmn_err(CE_WARN, "SCSI: HA %d TC %d - \"%s\" is",
					c, t, sc_edt[c][t].tc_inquiry); 
			cmn_err(CE_CONT,"\t not in the disk table (tc_data), but\n");
			cmn_err(CE_CONT,"\t it will be configured as a disk.\n");
		}
		if (found)
		{
			np = drv_name;
			for (j = 0; j < NAME_LEN-1; j++)
				sc_edt[c][t].drv_name[j] = *np++;
			sc_edt[c][t].drv_name[j] = NULL;

			sc_edt[c][t].c_maj = c_maj;
			sc_edt[c][t].b_maj = b_maj;

			tc_edtp->ha_slot = sc_edt[c][t].ha_slot;
			tc_edtp->tc_id = t;
			tc_edtp->n_lus = sc_edt[c][t].n_lus;
			for (l = 0; l < MAX_LUS; l++)
			      tc_edtp->lu_id[l] = sc_edt[c][t].lu_id[l];

			tc_count++;
			tc_edtp++;
		}
	    }
	}
	return (tc_count);
}


/*
** Function name: sdi_send()
** Description:
** 	Send a SCSI command to a controller.  Commands sent via this
**	function are executed in the order they are received.
*/

sdi_send(sp)
register sblk_t *sp;
{
	register struct scsi_ad *sa;
	register struct scsi_lu *q;
	register int	c, t;
	int  i, oip;

	sa = &sp->sb.SCB.sc_dev;
	c = SDI_HAN(sa);
	t = SDI_TCN(sa);

	i = sp - sc_sbtab;
	if (i < 0 || i >= sc_sbcnt)
	{
		cmn_err(CE_WARN, "SCSI: SB not allocated by SDI\n");
		return (SDI_RET_ERR);
	}

	if (sp->sb.sb_type != SCB_TYPE)
		return (SDI_RET_ERR);

	if (SDI_ILLEGAL(c, t, sa->sa_major))
	{
		sp->sb.SCB.sc_comp_code = SDI_SCBERR;
		if (sp->sb.SCB.sc_int)
			(*sp->sb.SCB.sc_int)((caddr_t)sp);
		return (SDI_RET_OK);
	}

	q = &LU_Q(c, t, sa->sa_lun);

	oip = spl5();
	if (q->q_flag & QPTHRU) {
		splx(oip);
		return (SDI_RET_RETRY);
	}

	sp->sb.SCB.sc_comp_code = SDI_PROGRES;
	sp->sb.SCB.sc_status = 0;

	scsi_putq(q, sp);
	if ( !(q->q_flag & (QBUSY | QSUSP)))
		scsi_next(q);

	splx(oip);
	return (SDI_RET_OK);
}


/*
** Function name: sdi_icmd()
** Description:
**	Send an immediate command.  If the logical unit is busy, the job
**	will be queued until the unit is free.  SFB operations will take
**	priority over SCB operations.
*/

sdi_icmd(sp)
register sblk_t *sp;
{
	register struct scsi_ad *sa;
	register struct scsi_lu *q;
	register int	c, t;
	int  i, oip;

	i = sp - sc_sbtab;
	if (i < 0 || i >= sc_sbcnt)
	{
		cmn_err(CE_WARN, "SCSI: SB not allocated by SDI\n");
		return (SDI_RET_ERR);
	}

	oip = spl5();

	switch (sp->sb.sb_type)
	{
	case SFB_TYPE:
		sa = &sp->sb.SFB.sf_dev;
		c = SDI_HAN(sa);
		t = SDI_TCN(sa);
		q = &LU_Q(c, t, sa->sa_lun);
#ifdef DEBUG
		printf("sdi_icmd: SFB c=%d t=%d l=%d \n", c, t, sa->sa_lun);
#endif

		if (SDI_ILLEGAL(c, t, sa->sa_major))
		{
			sp->sb.SFB.sf_comp_code = SDI_SFBERR;
			if (sp->sb.SFB.sf_int)
				(*sp->sb.SFB.sf_int)((caddr_t)sp);
			splx(oip);
			return (SDI_RET_OK);
		}

		sp->sb.SFB.sf_comp_code = SDI_ASW;

		switch (sp->sb.SFB.sf_func) 
		{
		case SFB_RESUME:
			q->q_flag &= ~QSUSP;
			scsi_next(q);
			break;
		case SFB_SUSPEND:
			q->q_flag |= QSUSP;
			break;
		case SFB_ABORTM:
		case SFB_RESETM:
			sp->sb.SFB.sf_comp_code = SDI_PROGRES;
			scsi_putq(q, sp);
			scsi_next(q);
			splx(oip);
			return (SDI_RET_OK);
		case SFB_FLUSHR:
			scsi_flushq(q, SDI_QFLUSH, 0);
			break;
		case SFB_NOPF:
			break;
		default:
			sp->sb.SFB.sf_comp_code = SDI_SFBERR;
		}

		if (sp->sb.SFB.sf_int)
			timeout(sp->sb.SFB.sf_int, (caddr_t)sp, 2);
		splx(oip);
		return (SDI_RET_OK);

	case ISCB_TYPE:
		sa = &sp->sb.SCB.sc_dev;
		c = SDI_HAN(sa);
		t = SDI_TCN(sa);
		q = &LU_Q(c, t, sa->sa_lun);
#ifdef DEBUG
		printf("sdi_icmd: SCB c=%d t=%d l=%d \n", c, t, sa->sa_lun);
#endif

		if (SDI_ILLEGAL(c, t, sa->sa_major))
		{
			sp->sb.SCB.sc_comp_code = SDI_SCBERR;
			if (sp->sb.SCB.sc_int)
				(*sp->sb.SCB.sc_int)((caddr_t)sp);
			splx(oip);
			return (SDI_RET_OK);
		}

		sp->sb.SCB.sc_comp_code = SDI_PROGRES;
		sp->sb.SCB.sc_status = 0;

		scsi_putq(q, sp);
		scsi_next(q);
		splx(oip);
		return (SDI_RET_OK);

	default:
		splx(oip);
		return (SDI_RET_ERR);
	}
}



/*
** Function name: sdi_translate()
** Description:
**	Perform the virtual to physical translation on the SCB
**	data pointer. 
*/

void
sdi_translate(sp, flag, procp)
register sblk_t *sp;
int flag;
struct proc *procp;
{
	extern dma_t   *dma_makelist();
	extern void	dma_freelist();

	if (sp->s_dmap)
	{
		dma_freelist(sp->s_dmap);
		sp->s_dmap = NULL;
	}

	if (sp->sb.SCB.sc_link)
	{
	 	cmn_err(CE_WARN, "SCSI: Linked commands NOT available\n");
		sp->sb.SCB.sc_link = NULL;
	}

	if (sp->sb.SCB.sc_datapt)
	{
		if ( !(flag & B_PHYS))
		{
			sp->s_addr = vtop(sp->sb.SCB.sc_datapt, procp);
			if (sp->s_addr == NULL) {
				sp->sb.SCB.sc_comp_code = SDI_V2PERR;
				cmn_err (CE_PANIC,
				    "SCSI: Bad address returned by VTOP\n");
			}
		}
		else {
			sp->s_dmap = dma_makelist(sp->sb.SCB.sc_datapt,
				 	  	sp->sb.SCB.sc_datasz, procp);
			sp->s_addr = vtop((caddr_t)sp->s_dmap->d_list, NULL);
		}
	}
	else {
		sp->s_addr = 0;
		sp->sb.SCB.sc_datasz = 0;
	}
}



/*
** Function name: sdi_getblk()
** Description:
**	Allocate a SB structure for the caller.  The function will
**	sleep if there are no SCSI blocks available.
*/

struct sb *
sdi_getblk()
{
	register int	oip;
	register sblk_t *sp;

	oip = spl6();
	while ( !(sp = sfreelist))
		sleep((caddr_t)&sfreelist, PRIBIO);
	sfreelist = sp->s_next;
	splx(oip);

	sp->sb.SCB.sc_comp_code = SDI_UNUSED;
	sp->s_dmap = NULL;
	sp->s_addr = 0;

	return ((struct sb *)sp);
}


/*
** Function name: sdi_freeblk()
** Description:
**	Release previously allocated SB structure. If a scatter/gather
**	list is associated with the SB, it is freed via dma_freelist().
**	A nonzero return indicates an error in pointer or type field.
*/

sdi_freeblk(sp)
register sblk_t *sp;
{
	register int	i, oip;
	extern void	dma_freelist();

	i = sp - sc_sbtab;
	if (i < 0 || i >= sc_sbcnt)
	{
		cmn_err(CE_WARN, "SCSI: SB not allocated by SDI\n");
		return (SDI_RET_ERR);
	}

	if (sp->s_dmap)
		dma_freelist(sp->s_dmap);

	oip = spl6();
	sp->s_next = sfreelist;
	if (sfreelist == NULL) 
		wakeup((caddr_t)&sfreelist);
	sfreelist = sp;

	splx(oip);
	return (SDI_RET_OK);
}


/*
** Function name: sdi_name()
** Description:
**	Return the name of the given device.  The name is copied into
**	a string pointed to by the second argument.
*/

void
sdi_name(sa, name)
struct scsi_ad *sa;
char *name;
{
	register char  *s1, *s2;
	static char temp[] = "HA X TC X";

	s1 = temp;
	s2 = name;
	temp[3] = SDI_HAN(sa) + '0';
	temp[8] = SDI_TCN(sa) + '0';

	while ((*s2++ = *s1++) != '\0')
		;
}


/*
** Function name: sdi_getdev()
** Description:
**	Convert SCSI device address to pass-through device number.
*/

void
sdi_getdev(sa, pdev)
struct scsi_ad *sa;
dev_t *pdev;
{
	register int	c = SDI_HAN(sa);
	register int	t = SDI_TCN(sa);

	if (SDI_ILLEGAL(c, t, sa->sa_major))
	{
		cmn_err(CE_WARN, "SCSI: Unequipped or illegal TC");
		return;
	}
	*pdev = makedevice(sc_major, HAMINOR(c, t, sa->sa_lun));
}



/*
** Function name: sdi_fltinit()
** Description:
**	Initialize target driver event handler.  The event handler is
**	called when a SCSI bus reset occurs or when the LU is closed 
**	after a pass-thru operation.
*/

void
sdi_fltinit(sa, func, param)
struct scsi_ad *sa;
void (*func)();
long param;
{
	register int	c = SDI_HAN(sa);
	register int	t = SDI_TCN(sa);
	register struct scsi_lu	*q;

	q = &LU_Q(c, t, sa->sa_lun);
        q->q_func  = func;
        q->q_param = param;
}


/*
** Function name: sdi_swap16()
** Description:
**	This function swaps bytes in a 16 bit data type.
*/

short
sdi_swap16(x)
unsigned int x;
{
	unsigned short rval;

	rval =  (x & 0x00ff) << 8;
	rval |= (x & 0xff00) >> 8;
	return (rval);
}


/*
** Function name: sdi_swap24()
** Description:
**	This function swaps bytes in a 24 bit data type.
*/

sdi_swap24(x)
unsigned int x;
{
	unsigned int rval;

	rval =  (x & 0x0000ff) << 16;
	rval |= (x & 0x00ff00);
	rval |= (x & 0xff0000) >> 16;
	return (rval);
}


/*
** Function name: sdi_swap32()
** Description:
**	This function swaps bytes in a 32 bit data type.
*/

long
sdi_swap32(x)
unsigned long x;
{
	unsigned long rval;

	rval =  (x & 0x000000ff) << 24;
	rval |= (x & 0x0000ff00) << 8;
	rval |= (x & 0x00ff0000) >> 8;
	rval |= (x & 0xff000000) >> 24;
	return (rval);
}


/*===========================================================================*/
/* SCSI Host Adapter Driver Utilities
/*===========================================================================*/


/*
** Function name: scsi_pass_thru()
** Description:
**	Send a pass-thru job to the HA board.
*/

void
scsi_pass_thru(bp)
struct buf  *bp;
{
	int	c = SC_HAN(bp->b_dev);
	int	t = SC_TCN(bp->b_dev);
	int	l = SC_LUN(bp->b_dev);
	register struct scsi_lu	*q;
	register sblk_t	*sp;
	struct proc *procp;
	int  oip;

	sp = (sblk_t *) bp->b_resid;
	if (sp->sb.SCB.sc_wd != (long) bp)
	     cmn_err(CE_PANIC, "SCSI: Corrupted address from physio\n");

	sp->sb.SCB.sc_dev.sa_lun = l;
	sp->sb.SCB.sc_dev.sa_fill = (c << 3) | t;
	sp->sb.SCB.sc_datapt = (caddr_t) paddr(bp);
	sp->sb.SCB.sc_int = scsi_int;

	drv_getparm(UPROCP, (ulong) &procp);
	sdi_translate(sp, bp->b_flags, procp);

	q = &LU_Q(c, t, l);

	oip = spl5();
	scsi_putq(q, sp);
	if ( !(q->q_flag & QBUSY))
		scsi_next(q);
	splx(oip);
}


/*
** Function name: scsi_int()
** Description:
**	This is the interrupt handler for pass-thru jobs.  It just
**	wakes up the sleeping process.
*/

void
scsi_int(sp)
struct sb *sp;
{
	struct buf  *bp;

#ifdef DEBUG
	printf("scsi_int: sp=%x \n",sp);
#endif
	bp = (struct buf *) sp->SCB.sc_wd;
	bp->b_flags &= ~B_BUSY;
	iodone(bp);
}



/*
** Function name: scsi_flushq()
** Description:
**	Empty a logical unit queue.  If flag is set, remove all jobs.
**	Otherwise, remove only non-control jobs.
*/

scsi_flushq(q, cc, flag)
register struct scsi_lu *q;
int  cc, flag;
{
	register sblk_t  *sp, *nsp;

	ASSERT(q);

	sp = q->q_first;
	q->q_first = q->q_last = NULL;
	q->q_count = 0;
	q->q_flag &= ~QFULL;

	while (sp) {
		nsp = sp->s_next;
		if (!flag && (queclass(sp) > QNORM))
			scsi_putq(q, sp);
		else
		{
			sp->sb.SCB.sc_comp_code = (ulong)cc;
			(*sp->sb.SCB.sc_int)(sp);
		}
		sp = nsp;
	}
}


/*
** Function name: scsi_putq()
** Description:
**	Put a job on a logical unit queue.  Jobs are enqueued
**	on a priority basis.
*/

void
scsi_putq(q, sp)
register struct scsi_lu	*q;
register sblk_t *sp;
{
	register cls = queclass(sp);

	ASSERT(q);

	/* 
	 * If queue is empty or queue class of job is less than
	 * that of the last one on the queue, tack on to the end.
	 */
	if ( !q->q_first || (cls <= queclass(q->q_last)) ){
		if (q->q_first) {
			q->q_last->s_next = sp;
			sp->s_prev = q->q_last;
		} else {
			q->q_first = sp;
			sp->s_prev = NULL;
		}
		sp->s_next = NULL;
		q->q_last = sp;

	} else {
		register sblk_t *nsp = q->q_first;

		while (queclass(nsp) >= cls)
			nsp = nsp->s_next;
		sp->s_next = nsp;
		sp->s_prev = nsp->s_prev;
		if (nsp->s_prev)
			nsp->s_prev->s_next = sp;
		else
			q->q_first = sp;
		nsp->s_prev = sp;
	}

	if (q->q_count++ >= sc_hiwat)
		q->q_flag |= QFULL;
}


/*
** Function name: scsi_next()
** Description:
**	Attempt to send the next job on the logical unit queue.
**	All jobs are not sent if the Q is busy.
*/

void
scsi_next(q)
register struct scsi_lu *q;
{
	register sblk_t  *sp;

	ASSERT(q);

	if ((sp = q->q_first) == NULL)	 /*  queue empty  */
		return;			 /*       or	  */
	if (q->q_flag & QBUSY)		 /*  device busy  */
		return;			 /*       or	  */
	if (q->q_flag & QSUSP  &&	 /*  Q suspended  */
	    sp->sb.sb_type == SCB_TYPE)
		return;

	q->q_flag |= QBUSY;

	if ( !(q->q_first = sp->s_next))
		q->q_last = NULL;

	q->q_count--;
	if ((q->q_flag & QFULL) && (q->q_count <= sc_lowat))
		q->q_flag &= ~QFULL;
	
	if (sp->sb.sb_type == SFB_TYPE)
		scsi_func(sp);
	else
		scsi_cmd(sp);
}



/*
** Function name: scsi_cmd()
** Description:
**	Create and send an SCB associated command. 
*/

void
scsi_cmd(sp)
register sblk_t *sp;
{
	register struct scsi_ad *sa;
	register struct ccb *cp;
	register struct scsi_lu	*q;
	register int  i;
	register char *p;
	unsigned long cnt;
	int  c, t;

	sa = &sp->sb.SCB.sc_dev;
	c = SDI_HAN(sa);
	t = SDI_TCN(sa);

	cp = scsi_getblk(c);
	cp->c_bind = (struct sb *)sp;
	cp->c_time = (sp->sb.SCB.sc_time*HZ) / 1000;

	/* fill in the controller command block */

	cp->c_dev = (char)((t << 5) | sa->sa_lun);
	cp->c_hstat = 0;
	cp->c_tstat = 0;
	cp->c_cmdsz = sp->sb.SCB.sc_cmdsz;
	cp->c_sensz = sizeof(cp->c_sense);

	if (sp->s_dmap)  /* raw mode, i.e, scatter/gather */
	{
		cp->c_opcode = SCSI_DMA_CMD;
		cnt = sp->s_dmap->d_size;
	}
	else	    /* block mode, i.e, NO scatter/gather */
	{
		cp->c_opcode = SCSI_CMD;
		cnt = sp->sb.SCB.sc_datasz;
	}
	cp->c_datasz[0] = msbyte(cnt);
	cp->c_datasz[1] = mdbyte(cnt);
	cp->c_datasz[2] = lsbyte(cnt);
	cp->c_datapt[0] = msbyte(sp->s_addr);
	cp->c_datapt[1] = mdbyte(sp->s_addr);
	cp->c_datapt[2] = lsbyte(sp->s_addr);
	cp->c_linkpt[0] = NULL;
	cp->c_linkpt[1] = NULL;
	cp->c_linkpt[2] = NULL;

	/* copy SCB cdb to controller CB cdb */

	p = sp->sb.SCB.sc_cmdpt;
	for (i = 0; i < sp->sb.SCB.sc_cmdsz; i++)
		cp->c_cdb[i] = *p++;

	q = &LU_Q(c, t, sa->sa_lun);
	if ((q->q_flag & QSENSE) && (cp->c_cdb[0] == SS_REQSEN)) {
		q->q_flag &= ~QSENSE;
		bcopy((caddr_t)(&q->q_sense)+1,
			(caddr_t)xphystokv(sp->s_addr),
			cnt);
		scsi_done(c, cp, SUCCESS);
		return;
	}

	scsi_send(c, START, cp);   /* send cmd */
}


/*
** Function name: scsi_func()
** Description:
**	Create and send an SFB associated command. 
*/

void
scsi_func(sp)
register sblk_t *sp;
{
	register struct scsi_ad *sa;
	register struct ccb *cp;
	int  c, t;

	sa = &sp->sb.SFB.sf_dev;
	c = SDI_HAN(sa);
	t = SDI_TCN(sa);

	cp = scsi_getblk(c);
	cp->c_bind = (struct sb *)sp;
	cp->c_time = 0;

	/* fill in the controller command block */

	cp->c_opcode = SCSI_TRESET;
	cp->c_dev = (char)((t << 5) | sa->sa_lun);
	cp->c_hstat = 0;
	cp->c_tstat = 0;

	cmn_err(CE_WARN, "SCSI: HA %d TC %d is being reset\n", c, t);
	scsi_send(c, START, cp);   /* send cmd */
}



/*
** Function name: scsi_send()
** Description:
**	Send a command to the host adapter board.
*/

void
scsi_send(c, cmd, cp)
int		c;	/* HA controller number */
int		cmd;	/* HA mbx command */
struct ccb	*cp;	/* command block */
{
	register struct scsi_ha *ha = &sc_ha[c];
	register struct mbx	*mp;
	register int 	n;

	if (cmd == START) {
		cp->c_active = TRUE;
		cp->c_start = lbolt;
		ha->ha_npend++;
	}

	/*
	 * Search for an empty outgoing mail box.
	 * A round-robin search algorithm is used.
	 */
	n = NMBX;
	mp = ha->ha_give;
	do {
		if (mp->m_stat == EMPTY)
			break;
		if (++mp == &ha->ha_mbo[NMBX])
			mp = ha->ha_mbo;
	} while (--n);

	if (mp->m_stat != EMPTY)
		cmn_err(CE_PANIC, "SCSI: MBO overflow");

	/*
	 * Fill in the mailbox and inform host adapter
	 * of the new request.
         */
	mp->m_addr = sdi_swap24((int)cp->c_addr);
	mp->m_stat = cmd;
	outb((ha->ha_base + HA_CMD), HA_CKMAIL);	

	/*
	 * Advance to the next outgoing mail box and
	 * save pointer value.
	 */
	if (++mp == &ha->ha_mbo[NMBX])
		mp = ha->ha_mbo;
	ha->ha_give = mp;
}


/*
** Function name: scsi_getblk()
** Description:
**	Allocate a controller command block structure.
*/

struct ccb *
scsi_getblk(c)
int	c;
{
	register struct scsi_ha	*ha = &sc_ha[c];
	register struct ccb	*cp;

	if (ha->ha_cblist) {
		cp = ha->ha_cblist;
		ha->ha_cblist = cp->c_next;
		return (cp);
	}
	cmn_err(CE_PANIC, "SCSI: Out of CBs");
}


/*
** Function name: scsi_freeblk()
** Description:
**	Release a previously allocated command block.
*/

void
scsi_freeblk(c, cp)
int  c;
struct ccb  *cp;
{
	register struct scsi_ha	*ha = &sc_ha[c];

	cp = &ha->ha_ccb[cp->c_index];
	cp->c_bind = NULL;
	cp->c_next = ha->ha_cblist;
	ha->ha_cblist = cp;
}


/*
** Function name: dma_makelist()
** Description:
**	Build a scatter/gather DMA list.
*/

static dma_t *
dma_makelist(vaddr, count, procp)
caddr_t	vaddr;
long	count;
struct proc *procp;
{
	register dma_t  *dmap;
	register int    i;
	register struct dma_vect  *pp;
	register long   fraglen, thispage;
	paddr_t addr, base;
	int  oip;

	oip = spl6();
	while ( !(dmap = dfreelist))
		sleep((caddr_t)&dfreelist, PRIBIO);
	dfreelist = dmap->d_next;
	splx(oip);

	pp = &dmap->d_list[0];
	for (i = 0; (i < MAX_DMASZ) && count; ++i, ++pp) {
		base = vtop(vaddr, procp);	/* Compute physical address of segment */
		fraglen = 0;					/* Zero bytes so far */
		do {
			thispage = min(count, pgbnd(vaddr));
			fraglen += thispage;			/* This many more are contiguous */
			vaddr += thispage;				/* Bump virtual address */
			count -= thispage;				/* Recompute amount left */
			if (!count)
				break;						/* End of request */
			addr = vtop(vaddr, procp);		/* Get next page's address */
		} while (base + fraglen == addr);

		/* Now set up dma list element */
		pp->d_addr[0] = msbyte(base);
		pp->d_addr[1] = mdbyte(base);
		pp->d_addr[2] = lsbyte(base);
		pp->d_cnt[0] = msbyte(fraglen);
		pp->d_cnt[1] = mdbyte(fraglen);
		pp->d_cnt[2] = lsbyte(fraglen);
	}
	if (count != 0)
		cmn_err(CE_PANIC, "SCSI: Job too big for DMA list.\n");

	dmap->d_size = i * sizeof(struct dma_vect);
	return (dmap);
}


/*
** Function name: dma_freelist()
** Description:
**	Release a previously allocated scatter/gather DMA list.
*/

static void
dma_freelist(dmap)
dma_t *dmap;
{
	register int  oip;
	
	ASSERT(dmap);

	oip = spl6();
	dmap->d_next = dfreelist;
	if (dfreelist == NULL)
		wakeup((caddr_t)&dfreelist);
	dfreelist = dmap;
	splx(oip);
}


/*
** Function name: scsi_comp()
** Description:
**	This function compares two strings for size of len.
*/

scsi_comp(s1, s2, len)
register char *s1, *s2;
int len;
{
	register int 	i;

	for (i = 0; i < len; i++, s1++, s2++)
		if (*s1 != *s2)
			return (-1);

	return (0);
}



/*
** Function name: scsi_mkedt()
** Description:
**	Build the equipped device table for the given SCSI Bus. This
**	function sends inquiries to every TC, then for all TCs which
**	answered the inquiry.  The number of LUs is obtained by sending
**	inquiries to every LU.  For disks test unit ready is also used.
*/

void
scsi_mkedt (c)
register int	c;	/* HA controller number */
{
	register int	t, l;
	int		i, comp_code;
	int		tc_count = 0;
	char		*p;
	struct scs	inq_cdb;


	for (t = 0; t < MAX_TCS; t++)	/* clear previous edt data */
	{
		sc_edt[c][t].c_maj = -1;
		sc_edt[c][t].b_maj = -1;
		sc_edt[c][t].pdtype = ID_NODEV;
		sc_edt[c][t].tc_equip = 0;
		sc_edt[c][t].ha_slot = 0;
		sc_edt[c][t].n_lus = 0;
		sc_edt[c][t].drv_name[0] = NULL;
		sc_edt[c][t].tc_inquiry[0] = NULL;
		for (l = 0; l < MAX_LUS; l++)
			sc_edt[c][t].lu_id[l] = 0;
	}

	if ( !(sc_ha[c].ha_state & C_SANITY))
		return;


	inq_cdb.ss_op = SS_INQUIR;	/* inquiry cdb		*/
	inq_cdb.ss_lun = 0;		/* first try lu 0	*/
	inq_cdb.ss_addr1 = 0;
	inq_cdb.ss_addr = 0;
	inq_cdb.ss_len = IDENT_SZ;
	inq_cdb.ss_cont = 0;


	for (t = 0; t < MAX_TCS; t++)	/* determine equipped TCs   */
	{
		if (t != sc_ha[c].ha_id)
		{
			comp_code = scsi_docmd(c, t, 0, &inq_cdb, SCS_SZ, 
					&inq_data, IDENT_SZ, B_READ);

			if (comp_code == SDI_ASW)
			{
				int 	j;

				tc_count++;
				sc_edt[c][t].tc_equip = 1;
				sc_edt[c][t].ha_slot = c;

				p = &inq_data.id_vendor[0];
				for (i = 0; i < (INQ_LEN -1); i++, p++)
					sc_edt[c][t].tc_inquiry[i] = *p;
				sc_edt[c][t].tc_inquiry[INQ_LEN-1] = NULL;
				for(j = 0; scsi_elist[j]; j++)
					if (scsi_comp(sc_edt[c][t].tc_inquiry, 
					    scsi_elist[j], INQ_LEN - 1) == 0)
						scsi_except[c][t] = 1;
			}
			else if (comp_code == SDI_TIME)
			{
				if (bootinfo.bootflags != BF_FLOPPY)
					cmn_err (CE_WARN,
					    "SCSI: Bus %d NOT functional\n", c);
				return;
			}	
		}
		else
		{
			sc_edt[c][t].tc_equip = 1;
			sc_edt[c][t].c_maj = sc_major;
			sc_edt[c][t].b_maj = -1;
			sc_edt[c][t].ha_slot = c;
			sc_edt[c][t].drv_name[0] = 'S';
			sc_edt[c][t].drv_name[1] = 'C';
			sc_edt[c][t].drv_name[2] = 'S';
			sc_edt[c][t].drv_name[3] = 'I';
			sc_edt[c][t].drv_name[4] = NULL;
		}
	}
	if (tc_count == 0)
	{
		cmn_err(CE_WARN, "SCSI: HA %d has no equipped TCs\n", c);
		return;
	}

	for (t = 0; t < MAX_TCS; t++)	/* determine LU equippage */
	{
		if (!(sc_edt[c][t].tc_equip) || (t == sc_ha[c].ha_id))
			continue;

		for (l = 0; l < MAX_LUS; l++)
		{
			if(scsi_except[c][t])
			{
	/*			printf("exception for id %d\n", t); */
				scsi_cklu(c, t, 0);
				break;
			}
			else
				scsi_cklu(c, t, l);
		}

		/* make sure at least one LU is configured */
		if (sc_edt[c][t].n_lus == 0)
		{
		     cmn_err(CE_WARN, "SCSI: HA %d TC %d has no LUs\n", c, t);
		     sc_edt[c][t].tc_equip = 0;
		}
	}

	/*
	 * This next loop is one last chance for a target to 
	 * get in the EDT. If a previously unequipped target 
	 * shows any signs of life, it will be equipped.
	 */
	for (t = 0; t < MAX_TCS; t++)	
	{
		if (!(sc_edt[c][t].tc_equip))
		{
			comp_code = scsi_docmd(c, t, 0, &inq_cdb, SCS_SZ, 
					&inq_data, IDENT_SZ, B_READ);

			if ((comp_code == SDI_ASW) || (comp_code == SDI_CKSTAT))
			{
				sc_edt[c][t].tc_equip = 1;
				sc_edt[c][t].ha_slot = c;

				p = &inq_data.id_vendor[0];
				for (i = 0; i < (INQ_LEN -1); i++, p++)
					sc_edt[c][t].tc_inquiry[i] = *p;
				sc_edt[c][t].tc_inquiry[INQ_LEN-1] = NULL;

				sc_edt[c][t].n_lus = 1 ;
				sc_edt[c][t].lu_id[0] = 1;
				sc_edt[c][t].drv_name[0] = 'V';
				sc_edt[c][t].drv_name[1] = 'O';
				sc_edt[c][t].drv_name[2] = 'I';
				sc_edt[c][t].drv_name[3] = 'D';
				sc_edt[c][t].drv_name[4] = NULL;
			}
		}
	}
}



/*
** Function name: scsi_cklu()
** Description:
**	Determines if the given LU is equipped.  First sends an inquiry
**	command.  If it passes, the LU is marked equipped.  A test unit
**	ready is also sent for disks, since some vendors don't return
**	the correct inquiry data to indicate that the addressed LU is
**	not present.
*/

scsi_cklu (c, t, l)
int	c;		/* HA Controller 	*/
int	t;		/* target controller	*/
int	l;		/* logical unit		*/
{
	struct scs	inq_cdb;
	struct scs	tur_cdb;
	int		comp_code;

	inq_cdb.ss_op = SS_INQUIR;	/* inquiry cdb	*/
	inq_cdb.ss_lun = l;	
	inq_cdb.ss_addr1 = 0;
	inq_cdb.ss_addr = 0;
	inq_cdb.ss_len = IDENT_SZ;
	inq_cdb.ss_cont = 0;

	comp_code = scsi_docmd(c, t, l, &inq_cdb, SCS_SZ, 
				&inq_data, IDENT_SZ, B_READ);

	if (comp_code == SDI_ASW)
	{
		if ((inq_data.id_type == RANDOM) && !(inq_data.id_rmb))
		{
			tur_cdb.ss_op = SS_TEST; /* test unit ready cdb */
			tur_cdb.ss_lun = l;
			tur_cdb.ss_addr1 = 0;
			tur_cdb.ss_addr = 0;
			tur_cdb.ss_len  = 0;
			tur_cdb.ss_cont = 0;

			comp_code = scsi_docmd(c, t, l, &tur_cdb, SCS_SZ, 
						NULL, NULL, B_READ);

			/* send it again first one clears unit attention */

			comp_code = scsi_docmd(c, t, l, &tur_cdb, SCS_SZ, 
						NULL, NULL, B_READ);
						
			if (comp_code == SDI_ASW) 
			{
				sc_edt[c][t].n_lus++ ;
				sc_edt[c][t].lu_id[l] = 1;
					sc_edt[c][t].pdtype = inq_data.id_type;
			}
		}
		else if (inq_data.id_type != NO_DEV)
		{
			sc_edt[c][t].n_lus++ ;
			sc_edt[c][t].lu_id[l] = 1;
			sc_edt[c][t].pdtype = inq_data.id_type;
		}
	}

	return (comp_code);
}



/*
** Function name: scsi_docmd()
** Description:
**	Create and send an SCB associated SCSI command. 
*/

scsi_docmd (c, t, l, cdb_p, cdbsz, data_p, datasz, rw_flag)
int	c;		/* HA Controller 	*/
int	t;		/* target controller	*/
int	l;		/* logical unit		*/
caddr_t	cdb_p;		/* pointer to cdb 	*/
long	cdbsz;		/* size of cdb		*/
caddr_t	data_p;		/* command data area 	*/
long	datasz;		/* size of buffer	*/
int	rw_flag;	/* read/write flag	*/
{
	register struct sb   *sp;
	register struct buf  *bp;
	struct scsi_lu	     *q;
	struct proc	     *procp;
	int		     retcode;


	bp = &sc_buf;
	while (bp->b_flags & B_BUSY)
		sleep((caddr_t)bp, PZERO);

	bp->b_flags = B_BUSY;

	sp = sdi_getblk();
	sp->sb_type       = SCB_TYPE;
	sp->SCB.sc_int    = scsi_int;
	sp->SCB.sc_cmdpt  = cdb_p;
	sp->SCB.sc_cmdsz  = cdbsz;
	sp->SCB.sc_datapt = data_p;
	sp->SCB.sc_datasz = datasz;
	sp->SCB.sc_wd     = (long) bp;
	sp->SCB.sc_time   = (30 * ONE_SEC);
	sp->SCB.sc_dev.sa_lun  = l;
	sp->SCB.sc_dev.sa_fill = ((c << 3) | t);

	drv_getparm (UPROCP, (ulong)&procp);
	sdi_translate (sp, rw_flag, procp);

	q = &LU_Q(c, t, l);

	sp->SCB.sc_comp_code = SDI_PROGRES;
	sp->SCB.sc_status = 0;

	scsi_putq(q, (sblk_t *)sp);
	scsi_next(q);

	if (init_time)
	{
		if (scsi_wait(c, 15000) == FAILURE)
			sp->SCB.sc_comp_code = SDI_TIME;
	}
	else
	{
		while (bp->b_flags & B_BUSY)
			sleep((caddr_t)bp, PZERO);
	}
	retcode = sp->SCB.sc_comp_code;
	sdi_freeblk(sp);

	return (retcode);
}


/*
** Function Name: scsi_wait()
** Description:
**	Poll for a completion from the host adapter.  If an interrupt
**	is seen, the HA's interrupt service routine is manually called. 
**  NOTE:	
**	This routine allows for no concurrency and as such, should 
**	be used selectively.
*/

scsi_wait(c, time)
int c, time;
{
	register struct scsi_ha  *ha = &sc_ha[c];
	unsigned char	status;

	while (time > 0)
	{
		status = inb(ha->ha_base + HA_ISR);
		if (status & HA_INTR )
		{
			scsiintr(ha->ha_vect);
			return (SUCCESS);
		}
		drv_usecwait(1000);  /* wait 1 msec */
		time--;
	}
	return (FAILURE);	
}


/*
** Function name: scsi_init_cbs()
** Description:
**	Initialize the controller CB free list.
*/

void
scsi_init_cbs(c)
int	c;
{
	register struct scsi_ha  *ha = &sc_ha[c];
	register struct ccb  *cp;
	register int	i;

	ha->ha_cblist = NULL;

	for (i = 0; i < NMBX ; i++)
	{
		cp = &ha->ha_ccb[i];
		cp->c_active = 0;
		cp->c_index  = i;
		cp->c_bind = NULL;
		cp->c_addr = kvtophys((caddr_t)cp);
		cp->c_next = ha->ha_cblist;
		ha->ha_cblist = cp;
	}
}



/*
** Function name: ha_init()
** Description:
**	Reset the HA board and initialize the communication area.
*/

void
ha_init(c)
int	c;	/* HA controller number */
{
	register struct scsi_ha  *ha = &sc_ha[c];
	register int	i;
	register char 	*ch;
	unsigned char	status;
	unsigned long	addr;
	int		retry;

	/* attempt to reset host adapter */
	outb(ha->ha_base, HA_HRST);
	cmn_err(CE_CONT,"AHA1542 board %d IO port: %x DMA %d\n", c,
								ha->ha_base,
								scsi_chan);
#if 0
	cmn_cfg("scsi",					/* DRIVER NAME */
		ha->ha_base,				/* PORT LO */
		HA_ISR,					/* PORT HI */
		ha->ha_vect,				/* IRQ */
		scsi_chan,				/* DMA */
		0L,					/* MEM BASE */
		0L,					/* MEM END */
		"AHA1540/2"); 				/* COMMENT */
#endif
	switch (scsi_chan)
	{
		case 0:
			outb(0xb, 0xc);
			outb(0xa, 0);
			break;
		case 5:
			outb(0xd6, 0xc1);
			outb(0xd4, 1);
			break;
		case 6:
			outb(0xd6, 0xc2);
			outb(0xd4, 2);
			break;
		case 7:
			outb(0xd6, 0xc3);
			outb(0xd4, 3);
			break;
		default:
			printf("unknown dma channel %d\n", scsi_chan);
	}

	status = inb(ha->ha_base + HA_STAT) & 0xff;
	retry = 1000000;
	while((status & HA_STIP) && --retry)
		status = inb(ha->ha_base + HA_STAT) & 0xff;
	if (!(status & HA_DIAGF) && (status & HA_IREQD))
	{
		/* initialize the communication area */

		for (i = 0; i < NMBX; i++) {
			ha->ha_mbo[i].m_stat = EMPTY;
			ha->ha_mbi[i].m_stat = EMPTY;
		}

		addr = kvtophys((caddr_t)ha->ha_mbo);
		ch = (char *)&addr;

mbinit:		ha_sndcmd(ha->ha_base, HA_INIT);
		ha_out(ha->ha_base, NMBX);
		ha_out(ha->ha_base, ch[2]);
		ha_out(ha->ha_base, ch[1]);
		ha_out(ha->ha_base, ch[0]);
		if(ha_wait(ha->ha_base) & HA_IREQD)
		{
			outb(ha->ha_base, HA_RST);
			while(!(inb(ha->ha_base + HA_STAT) & HA_READY))
				;
			goto mbinit;
		}
	}
	ha_sndcmd(ha->ha_base, HA_BONT);
	ha_out(ha->ha_base, 8);
	ha_wait(ha->ha_base);
	ha_sndcmd(ha->ha_base, HA_BOFFT);
	ha_out(ha->ha_base, 5);
	ha_wait(ha->ha_base);
	ha_sndcmd(ha->ha_base, HA_XFERS);
	ha_out(ha->ha_base, XFER_5_0);
	ha_wait(ha->ha_base);

	status = inb(ha->ha_base + HA_STAT);
	if (!(status & HA_IREQD) && (status & HA_READY))
	{
		/* mark this HA operational */
		ha->ha_state |= C_SANITY;
	}
}

ha_sndcmd(port, cmd)
int port;
char cmd;
{
	char status;
	long cntr = R_LIMIT;

	while(--cntr)
	{
		status = inb(port + HA_STAT) & 0xff;
		if(status & HA_READY)
		{
			ha_out(port, cmd);
			return;
		}
	}
	cmn_err(CE_WARN, "ha_sndcmd: unexpected status %b", status);
}

ha_out(port, data)
long port;
char data;
{
	while(inb(port) & HA_DOFULL)
		;			/* 20 - 30 us max */
	outb(port + HA_CMD, data);
}

ha_wait(port)
long port;
{
	char status;
	char int_reg = 0;

	while((int_reg |= inb(port + HA_ISR) & HA_CMDDONE) == 0)
		;
	status = (inb(port + HA_STAT) & 0xff);
	outb(port + HA_CNTL, HA_IACK);
	return(status);
}

ha_in(port)
int port;
{
	char d;
	while(!(inb(port) & HA_DIFULL))
		;
	d = inb(port + HA_DATA);
	return(d);
}
