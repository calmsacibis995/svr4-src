/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	This software is supplied under the terms of a license agreement   */
/*	or nondisclosure agreement with Intel Corporation and may not be   */
/*	copied or disclosed except in accordance with the terms of that    */
/*	agreement.							   */

/*	Copyright (C) Ing. C. Olivetti & C. S.p.a., 1989, 1990.*/
/*	All Rights Reserved	*/

#ident	"@(#)eisa:add-on/scsi.in/io/scsi/esc.c	1.3.1.1"

/*
 *	SCSI Host Adapter Driver for Olivetti ESC-1
 *	- combo version for i486/i860
 */

/* TEMPORARY: change the makefile */
#define	_SYSTEMENV	4

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
#include "esc.h"

#if	defined(i860)
#include "sys/io_860.h"
extern	char	*eisa_io_base;	/* memory mapped I/O space for i860 */
#define	EV_1_VECT	11
int	scsi_irq = EV_1_VECT;
#endif	/* i860 */

#if	(_SYSTEMENV == 4)
int scsidevflag = D_NEW;		/* SVR4 style driver	*/
extern	clock_t	lbolt;
#endif

/*@@*/
#ifdef	DEBUG
#undef	DEBUG
#endif

/* Allocated in space.c */
extern long		sc_major;	    /* SCSI driver major #	*/
extern unsigned int	sc_hacnt;	    /* Total # of controllers   */
extern unsigned int	sc_sbcnt;	    /* Total # of SCSI blocks	*/
extern struct scsi_edt	sc_edt[][MAX_TCS];  /* SCSI edt data 		*/
extern struct scsi_cfg	sc_cfg[];	    /* SCSI HA configuration	*/
#if	defined(i860)
extern struct scsi_ha	*sc_ha;	            /* SCSI HA structures	*/
#else
extern struct scsi_ha	sc_ha[];            /* SCSI HA structures	*/
#endif	/* i860 */
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

struct ccb  *scsi_getblk();
void	scsi_freeblk();
void	scsi_init_cbs();
void	scsi_pass_thru();
void	scsi_int();
void	scsi_putq();
void	scsi_next();
void	scsi_cmd();
void	scsi_func();
#ifndef	CLOSER
#define	START	1	/* some inane number for debugging */
#define	ABORT	2
#define	SUCCESS	1
#define	FAILURE	0
#endif

void	scsi_send();
/*void	scsi_done(); */
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
	
	if (!sdi_started) {
#if	defined(i860)
/*##*/		sh_mem->scsi_lock = 0;
		if ((sc_ha = (struct scsi_ha *)
			sptalloc(btoc(sizeof(struct scsi_ha) * sdi_hacnt), 
			      (PG_CD | PG_V), 0, 0)) == (struct scsi_ha *)NULL) 
			cmn_err(CE_PANIC, "scsiinit: can't allocate sc_ha buffer");
#endif	/* i860 */
		sdi_init();
	}
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
/*XX Bus reset not yet implemented */
	cmn_err(CE_WARN, "SCSI: HA %d - Bus idle. reset NOT SUPPORTED\n", c);
		u.u_error = EINVAL;
/*XX when implemented, * force reset on SCSI controller *
			cmn_err(CE_WARN, "SCSI: HA %d - Bus reset\n", c);
		/* physically reset SCSI bus */
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
	register struct scsi_ha  *ha;
	register struct mbo	 *mp;
	register struct ccb	 *cp;
	register struct sb	 *sp;
	register struct scsi_lu  *q;
	int	c,i;

	/*
	 * Determine which host adapter interrupted from
	 * the interrupt vector.
	 */
	for (c = 0; c < sc_hacnt; c++) {
		ha = &sc_ha[c];
		if (ha->ha_vect == vect)
			break;
	}

#if	defined(i860)
	if (c == sc_hacnt) {
#else
	if ((c == sc_hacnt) ||
	   (inb(ha->ha_base + BELLOUT) & 0x80) == 0) {
#endif	/* i860 */
		cmn_err(CE_WARN, "SCSI: *HA* spurious interrupt\n");
		return;
	}

#if !defined(i860)
	outb(ha->ha_base + BELLOUT, 0x80);  /* intr acknowledge */
#endif	/* i860 */

	/* read the outgoing mailbox */
	for (i = 0; i < sizeof(struct mbo); i++)
#if	defined(i860)
		((caddr_t)&ha->ha_mbo)[i] = eisa_io_base[ha->ha_base + MBO_ADDR + i];
#else
		((caddr_t)&ha->ha_mbo)[i] = inb(ha->ha_base + MBO_ADDR + i);
#endif	/*i860 */
	/*	
	{
	register unsigned long * lp = (unsigned long *)&ha->ha_mbo;
			i = ha->ha_base + MBO_ADDR;
			*lp++ = inl(i); *lp++ = inl(i+4);
	} 
	*/

	/* release outgoing semaphore  */
#if	defined(i860)
	eisa_io_base[ha->ha_base + SEMOUT] = 0x00;
#else
	outb(ha->ha_base + SEMOUT, 0x00);
#endif	/* i860 */

#define	haphystokv(x)	((uint)(x) - kvtophys(sc_ha) + (uint)sc_ha)

	/* get the related ccb */
	mp = &ha->ha_mbo;
#if	defined(i860)
	cp = (struct ccb *)(haphystokv(mp->m_addr) - CCB_OFF); 
	if (cp->c_xfer == 1) 	/* cmd in phase (read) */
		flush();
#else
	cp = (struct ccb *)xphystokv(mp->m_addr);
#endif	/* i860 */
	cp->c_active = FALSE;
	ha->ha_npend--;

	q = &LU_Q(c, cp->c_tcn, cp->c_lun);
	sp = cp->c_bind;

	ASSERT(sp);

	/*
	 * Determine the job completion status.
	 */
	if (mp->m_hstat == NOERROR) {
		if (mp->m_tstat == S_GOOD)
			sp->SCB.sc_comp_code = SDI_ASW;
		else {
			sp->SCB.sc_comp_code = SDI_CKSTAT;
			sp->SCB.sc_status = mp->m_tstat;
		}
	}
	else if (mp->m_hstat == NOSELECT)
		sp->SCB.sc_comp_code = SDI_NOSELE;
	else if (mp->m_hstat == TCPROTO)
		sp->SCB.sc_comp_code = SDI_TCERR; 
	else
		sp->SCB.sc_comp_code = SDI_HAERR;

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

#if	defined(i860)
	sh_mem->scsi_lock = 0;
#endif	/* i860 */
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

	q = &LU_Q(c, cp->c_tcn, cp->c_lun);

	sp = cp->c_bind;

	ASSERT(sp);

	/* Determine completion status of the job */

	switch (status)
	{
	case SUCCESS:
		sp->SFB.sf_comp_code = SDI_ASW;
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
	for (n = 0; n < NCCB; n++, cp++)
	{
		if (!cp->c_active)	/*  not pending	 */
			continue;	/*      or       */
		if (cp->c_time == 0) 	/*  not timed    */
			continue;
		
		if ((cp->c_time + cp->c_start) > lbolt)
			continue;

		/* Job timed out - terminate the command */

		cp->c_time = 0;
		cmn_err(CE_WARN, "SCSI: HA %d TC %d LU %d - Job timed out\n",
					c, cp->c_tcn, cp->c_lun);
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
/*
			timeout(sp->sb.SCB.sc_int, (caddr_t)sp, HZ);
*/
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
#ifdef DEBUG
		printf("sdi_icmd: SFB_TYPE illegal !! \n");
#endif
			sp->sb.SFB.sf_comp_code = SDI_SFBERR;
			if (sp->sb.SFB.sf_int)
/*
				timeout(sp->sb.SFB.sf_int, (caddr_t)sp, HZ);
 */
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
/*XX NOT SUPPORTED *
		case SFB_ABORTM:
		case SFB_RESETM:
			sp->sb.SFB.sf_comp_code = SDI_PROGRES;
			scsi_putq(q, sp);
			scsi_next(q);
			splx(oip);
			return (SDI_RET_OK);
/*XX*/
		case SFB_FLUSHR:
			scsi_flushq(q, SDI_QFLUSH, 0);
			break;
		case SFB_NOPF:
			break;
		default:
			cmn_err (CE_WARN, "SCSI: sdi_icmd SFB type %x - not implemented\n",
				sp->sb.SFB.sf_func);
			sp->sb.SFB.sf_comp_code = SDI_SFBERR;
		}

		if (sp->sb.SFB.sf_int)
/*
			timeout(sp->sb.SFB.sf_int, (caddr_t)sp, HZ);
 */
			(*sp->sb.SFB.sf_int)((caddr_t)sp);
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
#ifdef DEBUG
		printf("sdi_icmd: ISCB_TYPE illegal !! \n");
#endif
			sp->sb.SCB.sc_comp_code = SDI_SCBERR;
			if (sp->sb.SCB.sc_int)
/*
				timeout(sp->sb.SCB.sc_int, (caddr_t)sp, HZ);
 */
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

/*#ifdef DEBUG2*/
	if (sp->sb.SCB.sc_datasz < 512) {
		cmn_err(CE_WARN, "sdi_translate: count = %d", sp->sb.SCB.sc_datasz);
	}
/*#endif*/
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
#if	(_SYSTEMENV == 4)
	*pdev = makedevice(sc_major, HAMINOR(c, t, sa->sa_lun));
#else
	*pdev = makedev(sc_major, HAMINOR(c, t, sa->sa_lun));
#endif
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
#ifdef	DEBUG2
	if (sp->sb.SCB.sc_datasz < 512)
		cmn_err(CE_WARN, "scsi_pass_thru: count = %d", sp->sb.SCB.sc_datasz );
#endif

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
	
#ifdef	CLOSER
	if (sp->sb.sb_type == SFB_TYPE)
		scsi_func(sp);
	else
#endif
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
	register int  i;
	register char *p;
	int  c;

	sa = &sp->sb.SCB.sc_dev;
	c = SDI_HAN(sa);

	cp = scsi_getblk(c);
	cp->c_bind = (struct sb *)sp;
	cp->c_time = (sp->sb.SCB.sc_time*HZ) / 1000;

	/* fill in the controller command block */

	cp->c_tcn = SDI_TCN(sa);
	cp->c_lun = sa->sa_lun;
	cp->c_cmdsz = sp->sb.SCB.sc_cmdsz;

	if (sp->s_dmap)  /* scatter/gather */
		cp->c_addlen = sp->s_dmap->d_size;
	else
		cp->c_addlen = 0;

	cp->c_datasz = sp->sb.SCB.sc_datasz;
	cp->c_datapt = sp->s_addr;
	cp->c_linkpt = NULL;
	switch(*(caddr_t)(sp->sb.SCB.sc_cmdpt)) {
		case SM_WRDB:
		case SM_WRITE:
		case SS_WRITE:	
			cp->c_xfer = 2;
			break;
		default:
			cp->c_xfer = 1;
			break;
	}

#ifdef	DEBUG
	printf("scsi_cmd: sc_mode = 0x%b - op = 0x%b\n", sp->sb.SCB.sc_mode, *(unsigned char *)(sp->sb.SCB.sc_cmdpt));
#endif

	/* copy SCB cdb to controller CB cdb */
	p = sp->sb.SCB.sc_cmdpt;
	for (i = 0; i < sp->sb.SCB.sc_cmdsz; i++)
		cp->c_cdb[i] = *p++;

#ifdef	SENSE_SPECIAL
	q = &LU_Q(c, t, sa->sa_lun);
	if ((q->q_flag & QSENSE) && (cp->c_cdb[0] == SS_REQSEN)) {
		q->q_flag &= ~QSENSE;
		bcopy((caddr_t)(&q->q_sense)+1,
			(caddr_t)phystokv(sp->s_addr),
			cnt);
		scsi_done(c, cp, SUCCESS);
		return;
	}
#endif	/* SENSE_SPECIAL */

	scsi_send(c, START, cp);   /* send cmd */
}


#ifdef	CLOSER
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

#ifdef	RESET_OK
	cp->c_opcode = SCSI_TRESET;
#endif	/* RESET_OK */
	cp->c_tcn = SDI_TCN(sa);
	cp->c_lun = sa->sa_lun;

#ifdef	DEBUG
	printf("scsi_func: sf_func = 0x%b - sf_wd = 0x%b\n", sp->sb.SFB.sf_func, sp->sb.SFB.sf_wd);
#endif
/*XX this command must be sent immediately to the target controller.
 *   what means will the ESC-1 provide for this ? */

	scsi_send(c, START, cp);   /* send cmd */
}
#endif	/* CLOSER to adaptec, that is */



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
	register struct mbi  *mp;
	register int  i;

#ifndef	ABORT_OK
	if (cmd == ABORT) return;
#endif	/*ABORT_OK*/

	if (cmd == START) {
		cp->c_active = TRUE;
		cp->c_start = lbolt;
		ha->ha_npend++;
	}

	/* wait for previous command to be accepted */
	for (i = 100; i >= 0; i--) {
#if	defined(i860)
		eisa_io_base[ha->ha_base + SEMINC] = 0x01;
		if ((eisa_io_base[ha->ha_base + SEMINC] & 0x03) == 0x01)
			break;
#else
		outb(ha->ha_base + SEMINC, 0x01);
		if ((inb(ha->ha_base + SEMINC) & 0x03) == 0x01)
			break;
#endif	/* i860 */
		if (i == 0)
			cmn_err(CE_WARN, "SCSI: HA %c - Semaphore is stuck\n");
		drv_usecwait(10);  /* wait 10 usec */
	}

	mp = &ha->ha_mbi;
	mp->m_taskid = cp->c_tcn << 3;
	mp->m_cmd = GETCCB;
	mp->m_cmdlen = FIXED_LEN + cp->c_cmdsz;
#if	defined(i860)
	mp->m_addr = cp->c_addr + CCB_OFF;
#else
	mp->m_addr = cp->c_addr;
#endif	/* i860 */

#ifdef	DEBUG
    {
	struct	ccb	*vcp;
	printf("scsi_send: c=%d cp=0x%x \n", c, cp);

	printf("scsi_send: mp = 0x%x\n", mp);
	printf("\tCMD = ");
#if	defined(i860)
	for (i = 0; i < (int)cp->c_cmdsz + FIXED_LEN + CCB_OFF; i++)
#else
	for (i = 0; i < (int)cp->c_cmdsz + FIXED_LEN; i++)
#endif	/* i860 */
		printf("%b  ", ((unsigned char *)cp)[i]);
	printf("\n");
	if (cp->c_addlen) {
/* dump scatter/gather list HERE*/
	printf("scsi_send: scatter/gather list\n");
	}
	printf("\tMBOX = ");
	for (i = 0; i < sizeof(struct mbi); i++)
		printf("%b  ", ((unsigned char *)mp)[i]);
	printf("\n");

    }
#endif	/* DEBUG */
	
	/* write incoming mailbox */
	for (i = 0; i < sizeof(struct mbi); i++) {
#if	defined(i860)
		eisa_io_base[ha->ha_base + MBI_ADDR + i] = ((caddr_t)mp)[i];
#else
		outb((ha->ha_base + MBI_ADDR + i), ((caddr_t)mp)[i]);
#endif	/* i860 */
	}
/*
{
	register unsigned long * lp = (unsigned long *)mp;
	i = ha->ha_base + MBI_ADDR;
	outl(i, *lp++); outl(i+4, *lp++);
} 
*/

#if	defined(i860)
	if (cp->c_xfer == 2)  /* phase out cmd (write) */
		flush();
	/* send interrupt request to the adapter */
	eisa_io_base[ha->ha_base + BELLINC] = 0x80;
#else
	/* send interrupt request to the adapter */
	outb(ha->ha_base + BELLINC, 0x80);
#endif	/* i860 */

#ifdef	DEBUG
/*	spinwait(1000); */
#endif
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
	register long   cnt;
	register long   fraglen, thispage;
	paddr_t addr, oaddr;
	int  oip;

	oip = spl6();
	while ( !(dmap = dfreelist))
		sleep((caddr_t)&dfreelist, PRIBIO);
	dfreelist = dmap->d_next;
	splx(oip);

	pp = &dmap->d_list[0];

	for (i = 0; (i < MAX_DMASZ) && count; ++i, ++pp) {
#ifdef	OLD_STYLE
		cnt = min(count, pgbnd(vaddr));
		pp->d_cnt = cnt;
		pp->d_addr = (long) vtop(vaddr, procp);
		count -= cnt;
		vaddr += cnt;
#else
		oaddr = vtop(vaddr, procp);	/* Compute physical address of segment */
		fraglen = 0;					/* Zero bytes so far */
		do {
			thispage = min(count, pgbnd(vaddr));
			fraglen += thispage;			/* This many more are contiguous */
			vaddr += thispage;				/* Bump virtual address */
			count -= thispage;				/* Recompute amount left */
			if (!count)
				break;						/* End of request */
			addr = vtop(vaddr, procp);		/* Get next page's address */
/*@@MEG*/	} while (oaddr+fraglen == addr); 
/* was:		} while (oaddr+thispage == addr);  */

		pp->d_cnt = fraglen;
		pp->d_addr = oaddr;

#endif	/* OLD_STYLE */
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

	if (!sc_ha[c].ha_state)
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
				tc_count++;
				sc_edt[c][t].tc_equip = 1;
				sc_edt[c][t].ha_slot = c;

				p = &inq_data.id_vendor[0];
				for (i = 0; i < (INQ_LEN -1); i++, p++)
					sc_edt[c][t].tc_inquiry[i] = *p;
				sc_edt[c][t].tc_inquiry[INQ_LEN-1] = NULL;
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
	sp->SCB.sc_time   = (10 * ONE_SEC);
	sp->SCB.sc_dev.sa_lun  = l;
	sp->SCB.sc_dev.sa_fill = ((c << 3) | t);
#ifdef	DEBUG2
	if (sp->SCB.sc_datasz < 512)
		cmn_err(CE_WARN, "scsi_docmd: count = %d", sp->SCB.sc_datasz );
#endif

	drv_getparm (UPROCP, (ulong)&procp);
	sdi_translate (sp, rw_flag, procp);

	q = &LU_Q(c, t, l);

	sp->SCB.sc_comp_code = SDI_PROGRES;
	sp->SCB.sc_status = 0;

	scsi_putq(q, (sblk_t *)sp);
	scsi_next(q);

	if (init_time)
	{
		if (scsi_wait(c, 500) == FAILURE)
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
#if	defined(i860)
	int	s;
	unsigned short intrs;

	time <<= 16;
	s = splhi();
	while (time > 0) {
		intrs = *(eisa_io_base+C4_REGOFF);
		if (intrs & C4_I860INTR) {
			short	msk;

			msk = (1 << scsi_irq);
			LOCK_INTMASK(&(sh_mem->lock));
			if (msk & sh_mem->intmask) {
				sh_mem->intmask &= ~msk;
				if (sh_mem->intmask == 0)
					reset_hostintr();
				UNLOCK_INTMASK();
				splx(s);
				scsiintr(ha->ha_vect);
				return (SUCCESS);
			} else
				UNLOCK_INTMASK();
		}
		xdelay(1000);
		time--;
	}
	splx(s);
#else
	while (time > 0)
	{
		if (inb(ha->ha_base + BELLOUT) & 0x80)
		{
			scsiintr(ha->ha_vect);
			return (SUCCESS);
		}
		drv_usecwait(1000);  /* wait 1 msec */
		time--;
	}
#endif	/* i860 */
	
	cmn_err(CE_WARN, "Unexpected timeout on SCSI board #%d\n", c);
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

	for (i = 0; i < NCCB ; i++)
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
	unsigned char	boardid[4];
	unsigned char	icreg;
	unsigned int	tmp;
#if	defined(i860)
	/*$$*/int	onlytest = 0;
#endif	/* i860 */

	/* get base slot for board ID scan. */
	/* start above last successful board ID scan to avoid duplication. */
	ha->ha_base = 0x1000;
	if (c)
		ha->ha_base = sc_ha[c-1].ha_base + 0x1000;

	for (; ha->ha_base <= 0xF000; ha->ha_base += 0x1000)
	{
#if	defined(i860)
		tmp = *(int *)&eisa_io_base[ha->ha_base + SYSID];
#else
	        tmp = inl(ha->ha_base + SYSID); 
#endif	/* i860 */
		*(unsigned int *)boardid = tmp; 

		if (boardid[0] == OLI1 && boardid[1] == OLI2 &&
			boardid[2] == BOARDID && boardid[3] == BOARDID2)
		{
#if	defined(i860)
			if (onlytest++ == 1) {
				ha->ha_state = TRUE;
				break;
			}
#else
			ha->ha_state = TRUE;
			break;
#endif	/* i860 */
		} 
	}

	if (ha->ha_state) {
		/* attempt to reset host adapter */
		printf("Olivetti ESC-1 SCSI host adapter was found at address %xH.\n",
			ha->ha_base);

#if	defined(i860)
		/* disable interrupt from BMIC */
		eisa_io_base[ha->ha_base + INTMASK] = 0x00;

		/* clear any previously latched interrupt */
		eisa_io_base[ha->ha_base + BELLOUT] = 0xFF;

		/* enable doorbell */
		eisa_io_base[ha->ha_base + MASKOUT] = 0x80;
	
		/* set IRQ to level triggered */
		*(short *)&eisa_io_base[ELCREG] |= (1 << ha->ha_vect);

		/* enable interrupts from BMIC */
		eisa_io_base[ha->ha_base + INTMASK] = 0x01;

#else
		/* disable interrupt from BMIC */
		outb(ha->ha_base + INTMASK, 0x00);

		/* clear any previously latched interrupt */
		outb(ha->ha_base + BELLOUT, 0xFF);

		/* enable doorbell */
		outb(ha->ha_base + MASKOUT, 0x80);
	
		/* set IRQ to level triggered */
		icreg = inw(ELCREG); 
		outw(ELCREG, (icreg | (1 << ha->ha_vect)) );

		/* enable interrupts from BMIC */
		outb(ha->ha_base + INTMASK, 0x01);
#endif	/* i860 */

	/* @@ temporary: kludge to make happy the interrupt controller */
#if	defined(i860)
		icreg = eisa_io_base[0x461];
		eisa_io_base[0x461] = (icreg & (~0x8));
#else
		icreg = inb(0x461);
		outb(0x461, icreg & (~0x8));
#endif	/* i860 */
	}
}

#if	defined(i860)
/* @@ temporary */
tenmicrosec()
{
	register i;
	for (i = 0; i < 500; i++);
}
#endif
