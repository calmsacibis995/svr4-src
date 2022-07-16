/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)scsi:io/scsi/scsi.c	1.4.2.1"

/*
 *	The following is a SVR4 386 SCSI Driver. This driver was written 
 *	to talk to an AT&T version of the Western Digital 7000-ASC Host 
 *	adapter(E037) and the WD7000-FASST2 and has been proven to do so.
 *	If this source code is modified in an attempt to work with an older
 *	WD Host Adapter, minimally, the following changes should be made.
 *	
 *		- Disable synchronous negotiations. This can be disabled 
 *		  in this driver by removing the call to scsi_sync_init() in 
 *		  the sdi_init() routine. This will insure all SCSI transfers 
 * 		  are done in SCSI asynchronous mode.
 *		
 *		- do not use the scatter/gather functionality. This function 
 *		  does not exist in the older WD firmware. Each scatter/gather 
 *		  job must be broken down into separate SCSI requests. This 
 *		  will significantly reduce the performance of large raw jobs.
 */		  
#include	"sys/errno.h"
#include	"sys/types.h"
#include	"sys/param.h"
#include	"sys/mkdev.h"
#include	"sys/dir.h"
#include	"sys/signal.h"
#include	"sys/user.h"
#include	"sys/cmn_err.h"
#include	"sys/buf.h"
#include	"sys/systm.h"
#include	"sys/cred.h"
#include	"sys/i8237A.h"
#include 	"sys/uio.h"
#include	"sys/kmem.h"
#include	"sys/bootinfo.h" 
#include	"sys/sdi_edt.h"
#include	"sys/ddi.h"
#include	"sys/scsi.h"
#include	"sys/sdi.h"
#include	"had.h"
#include	"wd7000.h"

void		scsi_dmainit ();
void		scsi_dmamakelist ();
void		scsi_dmafreelist ();
void		scsi_sync_init ();
void		read_ha_version();

DMA_LIST	dma_pool[NUM_DMA_LISTS];
DMA_LIST	*dma_free_list = dma_pool;

/* The following are allocated in the space file */

extern struct scsi_edt	sc_edt[][MAX_TCS];/* SCSI edt data structure	*/
extern struct scsi_addr	scsi_ad[MAX_TCS];/* SCSI I/O addresses 		*/
extern struct scsi_ctrl	scsi_ctrl[];	/* one per HA card = #C 	*/
extern union cdb_item	scsi_cdb_pool[];/* HA driver CDB pool.		*/
extern char dmac[3];			/* DMA channels			*/

extern int	scsi_major;	/* Major number from master.d file	*/
extern int	scsi_cnt;	/* Total # of controllers    = #C	*/
long		sdi_hacnt;	/* Total # of controllers    = #C	*/
extern int	scsi_cdbsz;	/* HA driver CDB pool size		*/

/* The following are driver allocated variables */

char		scsi_sync_buf[25];
union sb_item	scsi_scb;	/* SCSI SCB used at init time		*/
union sb_item	*scsi_sblist;	/* SCSI SCB free list pointer		*/
long		scsi_psz;	/* SCSI SCB data size in bytes		*/
char 		*scsi_sbstart;	/* SCSI SCB data area start address	*/
char 		*scsi_sbend;	/* SCSI SCB data area end address	*/
char 		*scsi_tmp_buff;
long		scsi_tot_mem;	/* total size of system memory.		*/

union cdb_item	*scsi_cdb_list;	/* HA driver CDB free list pointer	*/
struct ver_no	sdi_ver;	/* HA drv. SDI version structure	*/
long		sdi_started;	/* HA drv. SDI initialization flag	*/
buf_t 		sc_bp;		/* Local buffer for sleep at init time	*/
static	long	scsi_timer_flag;/* SCSI delay variable			*/
struct ident	inq_data;	/* inquiry data structure		*/
HA_RB		rd_ha_vers;	/* read host adapter version structure */
HA_RB		rexp;		/* read parameters data structure */


static int	send_job();
static int	scsi_done();
static int	scsi_cmd();
static int	scsi_cklu();
static int	scsi_getq();
static int	ha_edt();
static int	ha_done();
static void	scsi_cint();
static void	scsi_default();
static void	scsi_reset();
static void	scsi_restime();
static void	scsi_resetck();
static void	free_cdb();
static void	enqueue();
static void	add_job();
static void	add_ijob();
static void	sub_job();
static void	next_job();
static void	start_jobs();
int		scsi_pass_thru();
static struct	scsi_cdb_buf	*get_cdb();
void		scsi_int();
void		scsi_timer();
void		scsi_sanity();
void		scsi_init_mailboxes();
void		scsi_init_srbs();


/*===========================================================================*/
/* Debugging mechanism that is to be compiled only if -DDEBUG is included
/* on the compile line for the driver				       
/* DPR provides levels 0 - 9 of debug information.                  
/*	0: No debugging
/*	1: entry and exit points of major routines
/*	2: entry and exit of major loops within major routines
/*	3: Variable values within major routines
/*	4 - 6: Same as above for ll functions
/*	7 - 9: Same as above for all driver functions
/*============================================================================*/



extern char 	scsi_Debug[];
extern char 	scsi_Board[];
extern int 	scsi_dbgsize;
int		scsi_tid;
int		scsi_tflag;

#ifdef DEBUG

#define DPR(b,l)	if (((b == 8) || (scsi_Board[b])) && scsi_Debug[l]) printf

#endif


#define	SCSI_DELAY(x)	scsi_timer_flag = 1; \
			timeout (tdelay, NULL, x); \
			while (scsi_timer_flag)



/*============================================================================
** Function name: scsiinit()
** Description:
**	Driver Initialization Entry Point. Called by kernel to perform 
**	driver initialization. Before the kernel data area has been 
**	initialized.
*/

void
scsiinit ()
{
	extern void	sdi_init();


	if (!sdi_started)
		(void) sdi_init ();
}

/*============================================================================
** Function name: scsistart()
** Description:
**	Called by kernel to perform driver initialization
**	After the kernel data area has been initialized.
*/

void
scsistart ()
{

	int		c, t;
	/*
	 * The following code walks thru the edt table looking for 
	 * orphan scsi devices. If found, the user is warned that
	 * they won't be configured.
	 */ 
	for (c = 0; c < scsi_cnt; c++)
	{
		for (t = 0; t < MAX_TCS; t++)
		{
			if ((sc_edt[c][t].tc_equip) && 
			   (sc_edt[c][t].drv_name[0] == NULL))

			{
				sc_edt[c][t].drv_name[0] = 'V';
				sc_edt[c][t].drv_name[1] = 'O';
				sc_edt[c][t].drv_name[2] = 'I';
				sc_edt[c][t].drv_name[3] = 'D';
				sc_edt[c][t].drv_name[4] = NULL;
				cmn_err (CE_WARN,"SCSI device \"%s\" at \n\tHA %d TC %u is unsupported and will not be configured in the system",&sc_edt[c][t].tc_inquiry[0], c,t);

			}
		}
	}

	/*
	 * this loop turns off init time flag which
	 * stops the HA driver from polling for interrupts 
	 * and begins taking interrupts normally.
	 */
	for (c = 0; c < scsi_cnt; c++)
		SC(c).state &= ~C_INIT_TIME; 


	return;
}


/*============================================================================
** Function name: scsiopen()
** Description:
** 	Driver Open Entry Point. It checks permissions. And in the case of
**	a pass-thru open it suspends the particular TC LU queue.
*/

int
scsiopen (devp, flags, otype, cred_p)
dev_t	*devp;	/* External Device # */
int	flags;	/* read only, write only, read/write, etc. */
int	otype;
cred_t	*cred_p;
{
	int		oip;
	register int	c = SC_CONTROL(getminor(*devp));
	register int	t = SC_TARGET(getminor (*devp));
	register int	l = SC_LUN(getminor (*devp));

#ifdef DEBUG
	DPR(8,1)(" open dev=%x c=%d t=%d l=%d ",*devp,c,t,l);
#endif

	/* This following code is used to allow mkdev to do a
	 * read_edt and an Host Adapter Count before 
	 * any nodes have been built. It is safe to use 
	 * 7 because only 3 HA's are supported.
	 */
	if (c == 7)
		return(0);
	else
	{
		if (SC_ILLEGAL(c, t))
		{
			return(ENXIO);
		}
	}

	if (cred_p->cr_uid != 0)
	{
		return(EPERM);
	}
	
	if (t == SC(c).ha_id)
		return(0);

	/* This is the pass-thru section */

	oip = spl5 ();

	if (LU(c, t, l).jobs.all)
	{
		splx(oip);
		return(EBUSY);
	}
	else if (t != SC(c).ha_id)
		LU(c, t, l).jobs.t.suspended |= PT_SUSPEND;

	splx (oip);
	return(0);
}



/*============================================================================
** Function name: scsiclose()
** Description:
** 	Driver Close Entry Point. In the case of a pass-thru close it 
**	resumes the queue and calls the target driver event handler if 
**	one is present.
*/

int
scsiclose (dev, flags, otype, cred_p)
dev_t	dev;	/* External Device # */
int	flags;
int	otype;
cred_t	*cred_p;
{
	int		oip;
	register int	c = SC_CONTROL(getminor (dev));
	register int	t = SC_TARGET(getminor (dev));
	register int	l = SC_LUN(getminor (dev));
	register struct lu_ctrl	*lp = &LU(c, t, l);

#ifdef DEBUG
	DPR(8,1)(" close dev=%x c=%d t=%d l=%d ",dev,c,t,l);
#endif /* DEBUG */

	
	/* This following code is used to allow mkdev to do a
	 * read_edt and an Host Adapter Count before 
	 * any nodes have been built. It is safe to use 
	 * 7 because only 3 HA's are supported.
	 */
	if (c == 7)
		return(0);
	else
	if (SC_ILLEGAL(c, t))
	{
		return(ENXIO);
	}
	lp = &LU(c, t, l);
	oip = spl5 ();
	if (lp->jobs.t.function) 
	{
		cmn_err (CE_WARN,"SCSI: HA %u tc %u lu %u, was busy during the close.\n", c, t, l);

	}
	else if (t != SC(c).ha_id) 	/* Resume it */
	{	
		lp->jobs.t.suspended &= ~PT_SUSPEND;
		if (lp->dv_func != NULL)
			lp->dv_func (lp->dv_param,SDI_FLT_PTHRU);

		send_job (c, t, l);
	}
	splx (oip);
	return(0);
}


/*============================================================================
** Function name: scsiioctl()
** Description:
**	Driver ioctl () Entry Point. Used to implement the following 
**	special functions:
**
**
**    SDI_SEND		- Send a user supplied SCSI Control Block to the
**			  specified scsi function.
**    GETVER		- Get the host adapter HW version
**    HA_VER		- Get sdi version structure 
**    REDT		- Read the extended EDT from RAM memory
**    SDI_BRESET	- Reset the specified bus 
**    RD_EXP		- Read the parameters setting the HA is using.
**    WR_EXP		- Write parameters for the HA to use.
**
*/

int
scsiioctl (dev, cmd, argp, mode, cred_p, rval_p)
dev_t	dev;
int	cmd;
char	*argp;
int	mode;
cred_t	*cred_p;
int	rval_p;
{
	struct bus_type	b;
	int		oip;
	int		c = SC_CONTROL(getminor (dev));
	int		t = SC_TARGET(getminor (dev));
	int		uerror = 0;
	
#ifdef DEBUG
	paddr_t		p;
	register short	dbg;
	DPR (c,1)("\nscsiioctl: dev=%x c=%d t=%d func=%x ", dev,c,t,cmd);
#endif /* DEBUG */

	/* This following code is used to allow mkdev to do a
	 * read_edt and an Host Adapter Count before 
	 * any nodes have been built. It is safe to use 
	 * 7 because only 3 HA's are supported.
	 */


	if (c == 7)
	{
		switch (cmd)
		{
		case	B_HA_CNT:
			if (copyout((caddr_t)&sdi_hacnt,argp, sizeof(sdi_hacnt)))
			uerror = EFAULT;
			break;
		case	B_REDT:
			if (copyout((caddr_t)sc_edt,argp ,(sizeof(struct scsi_edt) * MAX_TCS * scsi_cnt)))
			uerror = EFAULT;
			break;
		default:		/* Invalid Request */
			uerror = EINVAL;
			break;

		}
		return(uerror);
	}

	if (SC_ILLEGAL(c, t))
	{
		return(ENXIO);
	}
	/* If reset in progress wait for it to complete */
	while (SC(c).state & C_RS_BLK)
	{
		sleep ((caddr_t)&SC(c).state, PZERO);
	}
	switch (cmd)
	{
	case	SDI_SEND:	
		uerror = ll_send_cmd (dev, argp);
		break;
	case	SDI_BRESET:
		oip = spl5 ();
		ha_init(c);
		scsi_init_srbs(c);
		scsi_reset(c);
		scsi_resetck(c);
		splx (oip);
		break;
	case	B_GETTYPE:
		b.bus_name[0] = 's';
		b.bus_name[1] = 'c';
		b.bus_name[2] = 's';
		b.bus_name[3] = 'i';
		b.bus_name[4] = NULL;
		b.drv_name[0] = NULL;
		if (copyout ((caddr_t)&b, argp, sizeof (struct bus_type)) != SUCCESS)
			uerror = EFAULT;
		break;
	case	HA_VER:
		if (copyout((caddr_t)&sdi_ver,argp, sizeof(struct ver_no)))
			uerror = EFAULT;
		break;
	case	B_HA_CNT:
		if (copyout((caddr_t)&sdi_hacnt,argp, sizeof(sdi_hacnt)))
			uerror = EFAULT;
		break;
	case	B_REDT:
		if (copyout((caddr_t)sc_edt, argp, 
				(sizeof(struct scsi_edt) * MAX_TCS * scsi_cnt)))
			uerror = EFAULT;
		break;

	case	RD_HA_VER:
		oip = spl5 ();
		read_ha_version (c);
		cmn_err(CE_WARN,"Primary Revision Level = %d\n",rd_ha_vers.bytes[0]);
		cmn_err(CE_WARN,"Secondary Revision Level = %d\n",rd_ha_vers.bytes[1]);
		cmn_err(CE_WARN,"HA ID String = %s\n",&rd_ha_vers.bytes[4]);

		splx (oip);
		break;
#ifdef DEBUG
	case	RD_EXP:
		oip = spl5 ();
		rw_exp (c, RD_EXP, argp);
		splx (oip);
		break;
	case	WR_EXP:
		oip = spl5 ();
		rw_exp (c, WR_EXP, argp);
		splx (oip);
		break;
	case	TIMEOUTFLG:
		if (scsi_tflag = ~scsi_tflag)
		{
			scsi_tid = timeout (scsi_timer, NULL, ONE_MINUTE);
			printf ("\nScsi timeouts are turned on.\n");
		}
		else 
		{
			untimeout (scsi_tid);
			printf ("\nScsi timeouts are turned off.\n");
		}
		break;
	case	DEBUGFLG:
		if (copyin (argp, scsi_Debug, scsi_dbgsize) != SUCCESS)
		{
			return(EFAULT);
		}
		scsi_Board[c] = 1;
		printf ("\nNew debug values :");
		{
		int dbg;
		for (dbg = 0; dbg < scsi_dbgsize; dbg++)
			printf (" %d", scsi_Debug[dbg]);
		}
		if (scsi_Debug[0] == 0)
			scsi_Board[c] = 0;
		printf ("\n");
		break;
#endif /* DEBUG */
	default:		/* Invalid Request */
		uerror = EINVAL;
		break;
	} 
	return(uerror);
}


/*============================================================================
** Function name: scsiintr()
** Description:
**	Driver Interrupt Handler Entry Point. This code is entered upon 
**	the occurrence of an ha controller interrupt.
*/

void
scsiintr (vect)
register int  vect;	/* HA interrupt vector	*/
{
	SCSI_RB				*srb=NULL;
	unsigned char			status;
	unsigned char			cq_status;
	int				c, cqn;
	int				padr=NULL;

	/* calculate which HA board interrupted */

	for (c = 0; c < scsi_cnt; c++)
		if (scsi_ad[c].ha_vect == vect)
			break;

	if (c >= scsi_cnt)
	{
		cmn_err (CE_WARN,"SCSI: Received an unknown interrupt.\n");
		return;
	}
	status = inb(INT_STATUS(c));	/* read interrupt status REG */

#ifdef DEBUG
	DPR(8,1)("\nINT_S=0x%x ", status);
#endif
	if (status & CMD_COMPLETE)
	{
		cqn = (status & 0x3F);			/* get completion Q */
		cq_status = SC(c).cq[cqn].status;	/* number and status*/
		
		switch (cq_status)
		{
		case TC_RESET:
		case AB_RESET:
		case SPC_ERROR:
			cmn_err (CE_WARN,"SCSI: HA %d Unsolicited interrupt.\n",c);
			cmn_err (CE_WARN,"SCSI: Resetting SCSI Bus %d. \n",c);
			ha_init(c);
			scsi_init_srbs(c);
			scsi_reset(c);
			scsi_resetck(c);
			next_job(c);
			return;
			break;
		case NO_ERROR:		/* these completion cases have	*/
		case CK_SSB:		/* a pointer in the completion  */
		case HA_ERROR:		/* Q address field. 		*/

			/* get SRB address and process the completion	*/

			padr = sdi_swap24(SC(c).cq[cqn].pdp);
			srb = (SCSI_RB *) xphystokv(padr);
#ifdef DEBUG
			DPR(8,9)("padr=0x%x srb=0x%x\n",padr,srb);
#endif
			outb(INT_ACK(c), 0);	/* acknowledge the interrupt */

			if ((srb->opcode == SCSI_CMD) || 
			   (srb->opcode == SCSI_DMA_CMD))
				scsi_done (c, srb, cq_status);
			else 
				ha_done(c, srb,cq_status);
			break;
		default:
			cmn_err (CE_WARN,"SCSI: HA %u Illegal completion status=%x\n",c, cq_status);
			break;
		}
	}
	else
	{
		cmn_err (CE_WARN,"SCSI: Unexpected interrupt received form HA %d \n",c);
		cmn_err (CE_CONT,"Interrupt Status = 0x%x \n",status);
	}
	if (srb == NULL)		/* Int. not yet acknowledged */
		outb(INT_ACK(c), 0);

	next_job (c);
	return;
} 

/*============================================================================
** Function name: scsi_done()
** Description:
**	This is the interrupt handler routine for SCSI jobs which have
**	a SCSI request block and SCB structure defining the job.
*/

static int
scsi_done (c, srb, cq_status)
register int  	c;		/* HA Controller number	    */
SCSI_RB		*srb;		/* SRB pointer for this job */
int		cq_status;	/* completion Q status byte */
{
	register struct scsi_ctrl	*scp = &SC(c);
	register struct lu_ctrl		*lp;
	register struct sb		*psb;
	char				t,l;
	unsigned char			ssb,hasb;


	/* get job TC, LU and status bytes from the srb */

	t = (srb->dev >> 5);
	l = (srb->dev & 0x07);
	ssb = srb->ssb;
	hasb= srb->hasb;

	/* release the srb to the free list	*/

	((union srb_buf *)srb)->next = NULL;
	((union srb_buf *)srb)->next = scp->srb_list;
	scp->srb_list = (union srb_buf *) srb;

#ifdef DEBUG
	DPR(8,1)("c=%d t=%d l=%d cqs=0x%x ssb=0x%x hasb=0x%x\n", 
			c, t, l, cq_status, ssb, hasb);
#endif
	lp = &LU(c, t, l);
	psb = (struct sb *) lp->jp;

	/* check if there are no active jobs or the completion */
	/* is for a TC LU combination which has no active jobs */

	if ((!scp->active_jobs) || ((psb == NULL) || (!lp->jp->sp_sent)))
	{
#ifdef DEBUG
		if (!scp->active_jobs)
			DPR (8,0)("SCSI: active_jobs not set\n");
#endif
		cmn_err (CE_WARN,"SCSI: Unexpected completion reported");
		cmn_err (CE_CONT,"for HA %u tc %u lu %u.\n", c, t, l);
		return;
	}
	/* At this point we received a valid 	*/
	/* completion update job control flags	*/

	scp->active_jobs--;
	scp->jobs--;

	if (psb->sb_type == SCB_TYPE)
		lp->jobs.t.normal--;
	else
	{
		lp->jobs.t.immediate--;
		if (psb->SCB.sc_int == scsi_int)
			lp->jobs.t.suspended &= ~PT_ACTIVE;
	}
	/* determine completion status of the job */

	switch (cq_status)
	{
	case NO_ERROR:
		psb->SCB.sc_comp_code = SDI_ASW;
		break;
	case CK_SSB:
		if (ssb == S_GOOD)	/* good SCSI status */
			psb->SCB.sc_comp_code = SDI_ASW;
		else
		{
			psb->SCB.sc_comp_code = SDI_CKSTAT;
			psb->SCB.sc_status = ssb;
		}
		break;
	case HA_ERROR:
		if (hasb == NO_SELECT)
			psb->SCB.sc_comp_code = SDI_NOSELE;

		/* HA was required to do padding(0x41). This flag may not
		 * manifest itself in this manner with other versions of
		 * the firmware.
		 */ 
		else if (hasb == 0x41) 
			psb->SCB.sc_comp_code = SDI_ASW;
		else
			psb->SCB.sc_comp_code = (SDI_HAERR | SDI_SUSPEND);
		break;
	case B_RESET:
		psb->SCB.sc_comp_code = SDI_RESET;
		break;
	default:	/* other cases already handled in scsiint() */
		break;
	}
	if ((psb->SCB.sc_comp_code & SDI_SUSPEND) && 
		(psb->SCB.sc_int != scsi_int) && (psb->SCB.sc_int != scsi_cint))
	{
		lp->jobs.t.suspended |= SUSPEND;
	}
	sub_job (psb, c, t, l);
	send_job (c, t, l);

	if (psb->SCB.sc_int)		/* call target drv int. handler */
		psb->SCB.sc_int (psb);

	return;
} 

/*============================================================================
** Function name: ha_done()
** Description:
**	This is the interrupt handler routine for HA jobs which have
**	a request block structure defining the job.
*/

static int
ha_done (c, ha_rb, cq_status)
register int  	c;		/* HA Controller number	    */
HA_RB		*ha_rb;		/* HA request pointer for this job */
int		cq_status;	/* completion Q status byte */
{

#ifdef DEBUG
	DPR(8,1)("ha_done: NON SCSI COMPLETION opcode=%x\n",ha_rb->opcode);
#endif

	SC(c).state &= ~C_REDT_REQ;	
	wakeup ((caddr_t)&SC(c).state);

}

/*============================================================================
** Function name: scsi_reset()
** Description:
**	This is called upon the occurence of a SCSI bus reset. It starts
**	the 5 second delay during which no jobs are sent to the HA board.
**	It notifies the target drivers which have an event handler
**	assigned and also schedules scsi_restime() to be called after 5 
**	seconds.
*/

static void
scsi_reset (c)
register int	c;	/* Controller number */
{
	register struct scsi_ctrl	*scp = &SC(c);
	register struct lu_ctrl		*lp;
	int				t, l;

	if (scp->state & C_RS_BLK)
	{
		scp->state &= ~C_RS_BLK;
		scp->state |= C_RS_START;
		untimeout(scp->resid);
		scp->resid = timeout(scsi_restime,(caddr_t) c, 5*ONE_SECOND);
		return;
	}
	scp->state |= C_RS_START;
	scp->resid = timeout (scsi_restime,(caddr_t) c, 5*ONE_SECOND);

	for (t = 0; t < MAX_TCS; t++)	/* Notify target drivers */
	{
		for (l = 0; l < MAX_LUS; l++)
		{
			lp = &LU(c, t, l);
			if (lp->dv_func != NULL)
				lp->dv_func (lp->dv_param,SDI_FLT_RESET);
		}
	}
	return;
}

/*============================================================================
** Function name: scsi_restime()
** Description:
**	Called after 5 seconds to start the 1 second dalay timing during
**	SCSI bus resets during which only immediate jobs can be sent.
**	This function schedules itself to run in 1 second to stop the
**	timing sequence.
*/

static void
scsi_restime (c)
register int	c;	/* Controller number */
{
	register struct scsi_ctrl	*scp = &SC(c);

	if (scp->state & C_RS_START)
	{
		scp->state &= ~C_RS_START;
		scp->state |= C_RS_IMM;
		scp->resid = timeout (scsi_restime, (caddr_t)c, 1*ONE_SECOND);
#ifdef DEBUG
		DPR (c,10)("[");
#endif
	}
	else
	{
		scp->state &= ~C_RS_IMM;
		wakeup ((caddr_t)&scp->state); 
#ifdef DEBUG
		DPR (c,10)("]\n");
#endif
	}
	start_jobs(c);
}

/*============================================================================
** Function name: scsi_resetck()
** Description:
**	This routine checks for any jobs
**	not returned and deletes them from the LU Qs and returns them 
**	to the target drivers.
*/

static void
scsi_resetck (c)
register int	c;	/* Controller number */
{
	register struct scsi_ctrl	*scp = &SC(c);
	register struct lu_ctrl		*lp;
	register struct sb2		*sp, *next, *prev;
	int				t, l;

	if (!(scp->state & C_RS_START))
		return;
	for (t = 0; t < MAX_TCS; t++)
	{
	    for (l = 0; l < MAX_LUS; l++)
	    {
		lp = &LU(c, t, l);
		prev = NULL;
		next = lp->jp;
		while (prev != next)
		{
			sp = next;	/* sp is current job to check */
			prev = next;
			next = next->sp_forw;
			if (next == lp->jp)	/* first job stop */
				next = prev;
			if (sp->sp_sent)
			{

#ifdef DEBUG
  DPR (c,10)("sp=0x%x sp_sent=%d type=%d forw=0x%x back=0x%x\n",sp,sp->sp_sent, sp->sb.sb_type,sp->sp_forw,sp->sp_back);
#endif
				switch (sp->sb.sb_type) 
				{
				case SCB_TYPE:
				case ISCB_TYPE:
					if (scp->active_jobs > 0) 
					{
						scp->active_jobs--;
						scp->jobs--;
					}
					if (sp->sb.sb_type == SCB_TYPE) 
					{
						if (lp->jobs.t.normal > 0 )
							lp->jobs.t.normal-- ;
					}
					else
					{
						if (lp->jobs.t.immediate > 0 )
							lp->jobs.t.immediate-- ;
					}
					sub_job (sp, c, t, l);
					if (sp->sb.SCB.sc_comp_code != SDI_TIME)
						sp->sb.SCB.sc_comp_code = SDI_RESET;
					if (sp->sb.SCB.sc_int)
						sp->sb.SCB.sc_int (sp);
					break;
				default:
					cmn_err (CE_WARN,"SCSI: Illegal type on SCB job list for lu %u, tc %u, HA %u.\n",l,t,scp->maj);
					break;
				}
			}
		}
		if ((lp->fp != NULL) && (lp->fp->sp_sent))
		{
#ifdef DEBUG
  DPR (c,10)("Recovering from outstanding SFB job oustanding\n");
#endif
				     
			cmn_err (CE_WARN,
				"SCSI: Recovering from SFB job mismatch on lu %u, tc %u, HA %u.\n",l,t,scp->maj);
			if (lp->jp->sb.sb_type == SFB_TYPE) 
			{
				if (scp->active_jobs > 0) 
				{
					scp->active_jobs--;
					scp->jobs--;
				}
				if (lp->jobs.t.function > 0 )
					lp->jobs.t.function-- ;
				sp = lp->fp;
				lp->fp = NULL;
				sp->sp_sent = FALSE;
				sp->sb.SFB.sf_comp_code = SDI_RESET;

				if (sp->sb.SFB.sf_int)
					sp->sb.SFB.sf_int (sp);
			}
			else
				cmn_err (CE_WARN,"SCSI: Illegal type on SFB job list for lu %u, tc %u, HA %u.\n",l,t,scp->maj);
		}
	    }
	}
#ifdef DEBUG
  	DPR (c,10)(" out st =0x%x \n",scp->state);
#endif
}

/*============================================================================
** Function name: scsi_timer()
** Description:
**	This function is scheduled every minute to perform timing on SCB
**	associated jobs. Timing is not performed if the LU is open for
**	pass-tru although the pass-tru job itself is timed. If a job times
**	out a SCSI Bus reset is sent to the HA card and the SANITY flag is
**	set. The falg is cleared as soon as a completion comes back from
**	the HA card. If no completions return from the HA board within
**	20 seconds the HA is marked out-of-service. If a reset can not be
**	sent because the Q is busy jobs are stopped and 10 seconds later
**	the reset is sent by scsi_timout() regardless.
*/

void
scsi_timer ()
{
	register int		c;
	register int		t;
	register int		l;
	register struct sb2	*p;
	register struct lu_ctrl	*lp;

	for (c = 0; c < scsi_cnt; c++) 		/* HA Card           */
	{
		if (SC(c).jobs == NULL)
			continue;
	for (t = 0; t < MAX_TCS; t++) 		/* Target Controller */
	{
	   for (l = 0; l < MAX_LUS; l++) 	/* Logical Unit      */
	   {
		lp = &LU(c, t, l);

		/* if queue is "pass thru suspended" and no pass thru 
		   jobs are active or  the queue is pump suspended */
		if (((lp->jobs.t.suspended&PT_SUSPEND) &&
			(!(lp->jobs.t.suspended&PT_ACTIVE))) ||
			(lp->jobs.t.suspended&PUMP_SUSPEND))
		{
			continue;
		}
		
		/* if there are no jobs for that queue */
		if ((p = lp->jp) == NULL) {
			if ((p = lp->fp) == NULL)
				continue;
		}
		if ((lp->jobs.t.suspended&SUSPEND) && (p == lp->jp) &&
			(p->sb.sb_type == SCB_TYPE))
		{
			continue;
		}
		p->sp_time -= ONE_MINUTE;

		if (p->sp_time > 0) {
			continue;
		}
		cmn_err (CE_WARN,"SCSI: Job timeout for HA %u tc %u lu %u\n",
					c,t,l);
		p->sb.SCB.sc_comp_code = SDI_TIME;
		/* has job been sent? */
		if (!p->sp_sent)
		{
#ifdef DEBUG
	DPR(8,1)("SCSI: TIMEOUT JOB NOT SENT \n");
#endif
			SC(c).jobs--;
			switch (p->sb.sb_type)
			{
			case SCB_TYPE:
				if (lp->jobs.t.normal > 0)
					lp->jobs.t.normal--;
				break;
			case ISCB_TYPE:
				if (lp->jobs.t.immediate > 0)
					lp->jobs.t.immediate--;
				break;
			case SFB_TYPE:
				if (lp->jobs.t.function > 0)
					lp->jobs.t.function--;
				break;
			default:
				break;
			}
			if ((p->sb.sb_type != SFB_TYPE) &&
				(TC(c, t).waiting_jobs))
			{
				SC(c).waiting_jobs--;
				TC(c, t).waiting_jobs--;
			}
			p->sp_sent = TRUE;	/* fake out sub_job */
			sub_job(p, c, t, l);
			if (p->sb.sb_type == SFB_TYPE)
			{
				if (p->sb.SFB.sf_int)
					p->sb.SFB.sf_int (p);
			}
			else
			{
				if (p->sb.SCB.sc_int)
					p->sb.SCB.sc_int (p);
			}
			continue;
		}
		else	/* timed out job is on ha */
		{
			/* reset the card and proceed */
			ha_init(c);
			scsi_init_srbs(c);
			scsi_reset(c);
			scsi_resetck(c);
		}
	   } /* for l */
	} /* for t */
	} /* for c */
#ifdef DEBUG
	if (scsi_tflag) {
		scsi_tid = timeout (scsi_timer, NULL, ONE_MINUTE);
	}
#else
	(void) timeout (scsi_timer, NULL, ONE_MINUTE);
#endif
	return;
}

/*============================================================================
** Function name: scsi_sanity()
** Description:
**	This function is called 20 seconds after a job has timed out. It
**	checks the sanity flag. If its not set it means the HA is working.
**	If the flag is still set the HA card is no longer responding so
**	all queues are cleaned up and the jobs returned to the target
**	drivers.
*/

void
scsi_sanity (c)
register int	c;
{
	register int		t;
	register int		l;
	register struct sb2	*p;

	if ((SC(c).state & C_SANITY) == NULL) 
	{
#ifdef DEBUG
		DPR (8,0)("Firmware in HA %d is sane.\n", SC(c).maj);
#endif
		return;
	}
	SC(c).state &= ~C_OPERATIONAL;
	cmn_err (CE_WARN,"SCSI: Firmware on board in slot %u is not responding,\n\t\tplease repump the board.\n", SC(c).maj);

	for (t = 0; t < MAX_TCS; t++) 
	{
		for (l = 0; l < MAX_LUS; l++) 
		{
			while (((p = LU(c, t, l).jp) != NULL) ||
				((p = LU(c, t, l).fp) != NULL)) 
			{
#ifdef DEBUG
				DPR (8,0)("\nKilling job : psb = 0x%x ", p);
				DPR (8,0)("type = %d ", p->sb.sb_type);
				DPR (8,0)("c = %d t = %d l = %d\n", c, t, l);
#endif
				p->sp_sent = TRUE;
				switch (p->sb.sb_type) 
				{
				case SCB_TYPE:
					sub_job (p, c, t, l);
					p->sb.SCB.sc_comp_code = SDI_TIME;

					if (p->sb.SCB.sc_int) {
						p->sb.SCB.sc_int (p);
					}
					break;
				case ISCB_TYPE:
					sub_job (p, c, t, l);
					p->sb.SCB.sc_comp_code = SDI_TIME;

					if (p->sb.SCB.sc_int) {
						p->sb.SCB.sc_int (p);
					}
					break;
				case SFB_TYPE:
					p->sb.SFB.sf_comp_code = SDI_TIME;

					if (p->sb.SFB.sf_int) {
						p->sb.SFB.sf_int (p);
					}
					LU(c, t, l).fp = NULL;
					break;
				default:
					break;
				}
			}
			LU(c, t, l).jobs.all = NULL;
		}
		TC(c, t).waiting_jobs = NULL;
	}
	SC(c).jobs = NULL;
	SC(c).active_jobs = NULL;
	SC(c).waiting_jobs = NULL;
/*
	if (SC(c).free_rq == 0) 	FIX 
	{
		SC(c).rq->fr_busy = FALSE; 
#ifdef DEBUG
   		DPR (8,0)("scsi_sanity: RESET BUSY bit\n");
#endif
	}
*/

}

/*============================================================================
** Function name: sdi_init()
** Description:
**	This is the initialization routine for the HA driver. All data
**	structures are initiliazed. The board is initialized and the EDT
**	is build for every HA configured in the system.
*/

void
sdi_init ()
{
	register int	c, t, l;
	int		scsi_lus;
	union sb_item	*p;

	if (sdi_started) {
		return;
	}
#ifdef DEBUG
	scsi_tflag = TRUE;
#endif
	sdi_started = TRUE;

	sdi_ver.sv_release = 1;
	sdi_ver.sv_machine = SDI_386_AT;
	sdi_ver.sv_modes = SDI_BASIC1;

#ifdef DEBUG
	DPR (8,0)("\tSCSI-386 DEBUG DRIVER INSTALLED\n");
#endif
	sc_bp.b_flags = NULL;

	for (c = 0; c < scsi_cnt; c++)
	{
		SC(c).state    = NULL;
		SC(c).max_jobs = NULL;
		SC(c).jobs     = NULL;
		SC(c).maj      = scsi_major;
		SC(c).tc_cnt   = NULL;
		SC(c).ha_id    = HA_ID;
		SC(c).resid    = NULL;
		SC(c).waiting_jobs = NULL;
		SC(c).active_jobs = NULL;
		SC(c).next_tc = NULL;
		SC(c).next_lu = NULL;
		SC(c).bp      = NULL;
		SC(c).srb_list = NULL;

		for (t = 0; t < MAX_TCS; t++) {
			TC(c, t).lu_cnt = NULL;
			TC(c, t).waiting_jobs = NULL;

			for (l = 0; l < MAX_LUS; l++) {
				LU(c, t, l).jobs.all = NULL;
				LU(c, t, l).dv_func = NULL;
				LU(c, t, l).dv_param = NULL;
				LU(c, t, l).fp       = NULL;
				LU(c, t, l).jp       = NULL;
			}
		}

		
		scsi_init_mailboxes(c);
		scsi_init_srbs(c);
		
	}
	/* initialize init time SCB */
	scsi_scb.sb2.sb.SCB.sc_comp_code = SDI_NOALLOC;

	/* Initialize the pool of cdb's */
	for (l = 0; l < scsi_cdbsz - 1; l++) {
		scsi_cdb_pool[l].next = &scsi_cdb_pool[l+1];
	}
	scsi_cdb_pool[l].next = NULL;
	scsi_cdb_list = &scsi_cdb_pool[0];

	for (c = 0; c < scsi_cnt; c++)
	{
		/* 
		 * Set init_time flag which the driver uses to
		 * decide whether to poll the HA or take interrupts.
		 * (Sleeping is not allowed at init time.)
		 */ 
		SC(c).state |= C_INIT_TIME; 

		if(ha_init(c) == PASS)	/* initialize HA communication */
		{
			ha_edt (c);	/* build edt  */
			scsi_sync_init (c); /* WD7000-FASST2 supports sync mode */
		}

	}

	/* Calculate the total size of memory in the system. This is needed for 
	 * for a WD 7000-ASC HA bug that is further described in dma_makelist()
	 */
	scsi_tot_mem = ptob(maxclick);

	
#ifdef DEBUG
	DPR (8,0)("SCSI: TOTAL MEMORY = %D.\n",scsi_tot_mem);
#endif


	if ((scsi_tmp_buff = (char *)kmem_zalloc(NBPP, KM_NOSLEEP)) == NULL)
	{
		cmn_err (CE_PANIC,"SCSI: could not allocate scsi_tmp_buff.\n");
	}
	
	/* calculate total number of LUs in system. 	*/
	/* minimally allocate SCBs for at least one LU	*/

	scsi_lus = 1;
	for (c = 0; c < scsi_cnt; c++)
	{
		for (t = 0; t < MAX_TCS; t++)
			scsi_lus += sc_edt[c][t].n_lus;
	}
	/* get byte size of SCB data space */

	scsi_psz = scsi_lus * N_SCBS * sizeof(union sb_item);

	
	if ((scsi_sbstart = kmem_zalloc(scsi_psz, KM_NOSLEEP)) == NULL) {
		cmn_err(CE_PANIC,"SCSI: could not allocate SCB space.\n");
	}

#ifdef DEBUG
	DPR(8,4)("scsi_init_scbs: sizeof(sb_item) = %u bytes, total space = %u.\n", sizeof(union sb_item), scsi_psz);
#endif

	
	/* link up all SCBs */
	scsi_sbend = scsi_sbstart + scsi_psz;
	for (p = (union sb_item *) scsi_sbstart; 
			(p < (union sb_item *)scsi_sbend); p++)
	{
		p->sb2.sb.SCB.sc_comp_code = SDI_NOALLOC;
		p->sb2.sp_vip = NULL;
		p->sb2.sp_isz = NULL;
		p->sb2.sp_sent = FALSE;
		p->next = p + 1;
	}
	--p;
	p->next = NULL;

	scsi_sblist = (union sb_item *) scsi_sbstart; /* init head of list */

	scsi_dmainit ();
	
	scsi_timer ();
}



void
scsi_init_mailboxes(c)
int	c;
{
int	i;
	
	for (i = 0; i < NUM_OGMB; i++)
	{
		SC(c).rq[i].status = NULL;
		SC(c).rq[i].pdp = NULL;
	}
	for (i = 0; i < NUM_ICMB; i++)
	{
		SC(c).rq[i].status = NULL;
		SC(c).rq[i].pdp = NULL;
	}
}


void
scsi_init_srbs(c)
int	c;
{
int	i;

	SC(c).srb_list = NULL;

	/* Initialize the SRB linked list */
	for (i = 0; i < (NUM_OGMB - 1) ; i++)
		SC(c).srb[i].next = &SC(c).srb[i+1];

	SC(c).srb[i].next = NULL;
	SC(c).srb_list = &SC(c).srb[0];
}

void
scsi_sync_init(c)
int	c;
{
	int 	rqn,i;
	int 	padr=NULL;
	unsigned char cmd;
	
	for (i=0;i<25;i++)
		scsi_sync_buf[i] = NULL;	/* clear it out */
		
	/* negotiate for 4 Meg with an offset of 12 */
	for (i=0;i<16;i=i+2)
		scsi_sync_buf[i] = FOUR_MEG | OFFSET_12;

	rexp.opcode = SET_EXECUT_PARAMS;	/* write exp */

	rexp.bytes[0] = NULL;	/*reserved byte */
	rexp.bytes[1] = NULL;
	rexp.bytes[2] = NULL;
	rexp.bytes[3] = 16;	/* xfer size */

	if ((padr = vtop(scsi_sync_buf, NULL)) == NULL)
		cmn_err (CE_PANIC,"SCSI: Bad address returned by VTOP.\n");

	rexp.bytes[4] = msbyte(padr);	/* cmd data area read from */
	rexp.bytes[5] = mdbyte(padr);
	rexp.bytes[6] = lsbyte(padr);
	rexp.bytes[7] = NULL;
	rexp.bytes[8] = NULL;
	rexp.bytes[9] = NULL;
	rexp.bytes[10] = NULL;
	rexp.bytes[11] = NULL;
	rexp.bytes[12] = NULL;
	rexp.bytes[13] = NULL;
	rexp.status = NULL;

	/* set up the request Q entry */

	rqn = scsi_getq (c);
	if ((SC(c).rq[rqn].pdp = sdi_swap24(vtop((caddr_t)&rexp, NULL))) == NULL)
		cmn_err (CE_PANIC,"SCSI: Bad address returned by VTOP.\n");

	SC(c).rq[rqn].status = RQ_BUSY;
	cmd = (RD_MB | (rqn & 0x3F));	/* send cmd to HA board */
	ha_cmd(c, cmd);

	if (scsi_wait_for_comp(c,1000000) == FAIL) /* 1 sec */
	{
		cmn_err (CE_WARN,"SCSI: Write Parameters command timed out.\n");
	}

}


/*============================================================================
** Function name: scsi_comp()
** Description:
**	This function compares two strings for size of len.
*/

int
scsi_comp (s1, s2, len)
char	*s1;	/* ptr to string one	*/
char	*s2;	/* ptr to string two	*/
int	len;	/* length of string to compare	*/
{
	int 	i;

	for (i = 0; i < len; i++, s1++, s2++)
		if (*s1 != *s2)
			return (FAIL);

	return (PASS);
}

/*============================================================================
** Function name: sdi_config()
** Description:
**	This function is called by the target drivers to determine how many
**	of the target controllers that they support are configured in the
**	system. The target drivers pass to this function a pointer to their
**	tc_data structure which contains the inquiry strings of the TCs that
**	they support. This routines walks through the EDT data searching for
**	the inquiry strings that match. Then the target drivers are returned
**	the number of TCs found and for each TC a tc_edt structure is also
**	populated. The inquiry matching is only done on the Product ID field
**	only. The reason is that so Vendors can use KS numbers in the Product
**	ID field and keep the Vendor ID field with their own names.
**
**	One special case is the configuration for the primary disk driver,
**	SD01, where all RANDOM type targets with logical units are assumed
**	to be SCSI disks that need to be configured even if not supported.
*/

int
sdi_config (drv_name, c_maj, b_maj, tc_ptr, tc_size, tc_edtp)
char		*drv_name;	/* target driver ASCII name		*/
int		c_maj;		/* target driver character major number	*/
int		b_maj;		/* target driver block major number	*/
struct tc_data	*tc_ptr;	/* pointer to target driver tc data	*/
int		tc_size;	/* number of entries in tc_data		*/
struct tc_edt	*tc_edtp;	/* pointer to tc edt to be filled in	*/
{
	struct tc_data	*tcp;

	int		c, t, l, found;
	int		i, j, tc_count;
	char		*drvp;

#ifdef DEBUG
     DPR(8,5)("sdi_conf: drv=%s c_maj=%d tc_size=%d\n",drv_name, c_maj,tc_size);
#endif
	tc_count = 0;
	for (c = 0; c < scsi_cnt; c++)
	{
	  for (t = 0; t < MAX_TCS; t++)
	  {
	    if ((t != SC(c).ha_id) && (sc_edt[c][t].tc_equip))
	    {
		tcp = tc_ptr;
		found = FALSE;
		for (i = 0; (i < tc_size) && (found == FALSE) ; i++, tcp++)
		{
			if (scsi_comp (&sc_edt[c][t].tc_inquiry[VID_LEN], 
				&tcp->tc_inquiry[VID_LEN], PID_LEN) == PASS)
				found = TRUE;	/* stops for loop */
		}
		if (found == FALSE &&
			sc_edt[c][t].pdtype == RANDOM &&
			sc_edt[c][t].n_lus > 0 &&
			scsi_comp(drv_name, "SD01", 4) == PASS)
		{
			found = TRUE;
			cmn_err(CE_WARN,"SCSI device \"%s\" at \n\tHA %d TC %u is not in the disk table (tc_data), but it will be configured as a disk",&sc_edt[c][t].tc_inquiry[0],c,t);
		}
		if (found == TRUE)
		{
			sc_edt[c][t].c_maj = c_maj;
			sc_edt[c][t].b_maj = b_maj;

			drvp = drv_name;
			for (j=0; j < sizeof(drv_name); j++)
				sc_edt[c][t].drv_name[j] = *drvp++;
			sc_edt[c][t].drv_name[j] = NULL;

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
	}
#ifdef DEBUG
	DPR(8,5)("sdi_conf: matched on %d TCs\n", tc_count);
#endif
	return (tc_count);
}


/*============================================================================
** Function name: sdi_send()
** Description:
**	Send the command descriptor block to the controller.
**	Commands sent via this function are executed in the order they
**	are received.
**
**	Return values:
**		 0   : Request has not completed, so the target driver
**		       interrupt handler will be called.
**		-1   : The type field is invalid.
**		 1   : The job has not completed, and the target driver
**		       should retry the job later.
*/

long
sdi_send (sp)
register struct	sb2	*sp;
{
	register struct lu_ctrl		*lp;
	register struct scsi_ctrl	*cp;
	register int	c = SDI_CONTROL(sp->sb.SCB.sc_dev.sa_fill);
	register int	t = SDI_TARGET(sp->sb.SCB.sc_dev.sa_fill);
	register int	l = sp->sb.SCB.sc_dev.sa_lun;
	int		oip;

#ifdef DEBUG
	DPR (8,2)("sdi_send c%d t%d l%d ",c,t,l);
#endif
	if ((sp == NULL) || (sp->sb.sb_type != SCB_TYPE))
	{
		cmn_err (CE_WARN,
			"SCSI: sdi_send called with an illegal pointer.\n");
		return (SDI_RET_ERR);
	}
	if ((sp < (struct sb2 *) scsi_sbstart) || 
			(sp > (struct sb2 *) scsi_sbend))
	{
		cmn_err (CE_WARN,
			"SCSI: sdi_send SCB not allocated by SCSI driver.\n");
		return (SDI_RET_ERR);
	}
	if (SDI_ILLEGAL(c, t, sp->sb.SCB.sc_dev.sa_major))
	{
		cmn_err (CE_WARN,
		"SCSI: sdi_send called for an unequipped or illegal tc %u.\n",t);

		sp->sb.SCB.sc_comp_code = SDI_SCBERR;

		if (sp->sb.SCB.sc_int)
			timeout (sp->sb.SCB.sc_int, (caddr_t)sp, ONE_MSEC);

		return (SDI_RET_OK);
	}
	if (sp->sb.SCB.sc_datasz > 131072) /* WD HA does not support > 128K jobs */ 
	{
		sp->sb.SCB.sc_comp_code = SDI_SCBERR;
		return (SDI_RET_ERR);
	}
	lp = &LU(c, t, l);
	cp = &SC(c);
	oip = spl5 ();
	if (lp->jobs.t.normal >= cp->max_jobs)
	{
		splx (oip);
		return (SDI_RET_RETRY);
	}
	if (((cp->state & C_OPERATIONAL) == NULL) && !lp->jobs.t.suspended)
	{
		splx (oip);
		sp->sb.SCB.sc_comp_code = SDI_OOS;
		if (sp->sb.SCB.sc_int)
			timeout (sp->sb.SCB.sc_int, (caddr_t)sp, ONE_MSEC);
		return (SDI_RET_OK);
	}
	if (sp->sb.SCB.sc_time < 0) 
	{
		cmn_err (CE_WARN,
		     "SCSI: sdi_send called with a negative timeout value.\n");

		sp->sb.SCB.sc_comp_code = SDI_SCBERR;
		if (sp->sb.SCB.sc_int)
			timeout (sp->sb.SCB.sc_int,(caddr_t) sp, ONE_MSEC);
		splx (oip);
		return (SDI_RET_OK);
	}

	/* convert milliseconds to ticks; but only if > 0 */
	if (sp->sb.SCB.sc_time)
	{
		if (sp->sb.SCB.sc_time < 1000 * 1000)
			sp->sb.SCB.sc_time = drv_usectohz(sp->sb.SCB.sc_time * 1000); 
		else
			sp->sb.SCB.sc_time = drv_usectohz(sp->sb.SCB.sc_time) * 1000; 
	}
		
	sp->sb.SCB.sc_comp_code = SDI_PROGRES;
	lp->jobs.t.normal++;
	cp->jobs++;
	add_job (sp, c, t, l);
	send_job (c, t, l);
	splx (oip);

	return (SDI_RET_OK);
}

/*============================================================================
** Function name: sdi_icmd()
** Description:
**	Send an immediate command. Only one immediate command
**	may be sent per logical unit at a time. If the logical unit
**	is busy, the job will be queued until the unit is free.
**	SFB operations are executed in the order they are submitted,
**	and will take priority over the SCB operations.
**
**	Return values:
**		 0   : Request has not completed, so the target driver
**		       interrupt handler will be called.
**		-1   : The type field is invalid.
**		 1   : The job has not completed, and the target driver
**		       should retry the job later.
*/

int
sdi_icmd (psb)
register struct sb2 *psb;
{
	register struct lu_ctrl	*lp;
	register int	c;
	register int	t;
	register int	l;
	int		oip;
	int		opcode;

	if (psb == NULL)
	{
		cmn_err (CE_WARN,
			"SCSI: sdi_icmd called with an illegal pointer.\n");
		return (SDI_RET_ERR);
	}
	if ((psb < (struct sb2 *) scsi_sbstart) || 
			(psb > (struct sb2 *) scsi_sbend))
	{
		cmn_err (CE_WARN,
			"SCSI: sdi_icmd SCB not allocated by SCSI driver.\n");
		return (SDI_RET_ERR);
	}
	oip = spl5 ();

	switch (psb->sb.sb_type)
	{
	case	SFB_TYPE:
		c = SDI_CONTROL(psb->sb.SFB.sf_dev.sa_fill);
		t = SDI_TARGET(psb->sb.SFB.sf_dev.sa_fill);
		l = psb->sb.SFB.sf_dev.sa_lun;
		lp = &LU(c, t, l);
#ifdef DEBUG
		DPR (8,2)(" sdi_icmd SFB c%d t%d l%d ",c,t,l);
#endif

		if (SDI_ILLEGAL(c, t, psb->sb.SFB.sf_dev.sa_major))
		{
			cmn_err (CE_WARN,
       			"SCSI: sdi_icmd called for an unequipped or illegal tc %u.\n",t);
			psb->sb.SFB.sf_comp_code = SDI_SFBERR;

			if (psb->sb.SFB.sf_int)
				timeout (psb->sb.SFB.sf_int, (caddr_t)psb, ONE_MSEC);

			splx (oip);
			return (SDI_RET_OK);
		}
		psb->sb.SFB.sf_comp_code = SDI_ASW;
		opcode = DOS;
		switch (psb->sb.SFB.sf_func) 
		{
		case	SFB_NOPF:	/* Do nothing */
			break;
		case	SFB_RESETM:	/* Reset message */
			opcode = TCRST;
			break;
		case	SFB_ABORTM:	/* Abort message */
			if (SC(c).state & C_RAM)
				 opcode = ABORT;
			break;
		case	SFB_SUSPEND:
			lp->jobs.t.suspended |= SUSPEND;
			break;
		case	SFB_RESUME:
			lp->jobs.t.suspended &= ~SUSPEND;
			send_job (c, t, l);
			break;
		case	SFB_FLUSHR:	/* Flush LU no longer supported */
			cmn_err (CE_WARN,
			      "SCSI: sdi_icmd queue flush not implemented.\n");
			psb->sb.SFB.sf_comp_code = SDI_SFBERR;
			break;
		default:
			cmn_err (CE_WARN,"SCSI: sdi_icmd called with an illegal opcode of %u.\n", psb->sb.SFB.sf_func);
			psb->sb.SFB.sf_comp_code = SDI_SFBERR;
			break;
		}
		if (((SC(c).state & C_OPERATIONAL) == NULL) &&
				(!lp->jobs.t.suspended))
		{
			psb->sb.SFB.sf_comp_code = SDI_OOS;
			if (psb->sb.SFB.sf_int)
				timeout (psb->sb.SFB.sf_int, (caddr_t)psb, ONE_MSEC);
			splx (oip);
			return (SDI_RET_OK);
		}
		if (lp->jobs.t.function)
		{
			psb->sb.SFB.sf_comp_code = SDI_ONEIC;
			if (psb->sb.SFB.sf_int)
				timeout (psb->sb.SFB.sf_int,(caddr_t) psb, ONE_MSEC);
			splx (oip);
			return (SDI_RET_OK);
		}
		if (opcode == DOS) 
		{
			if (psb->sb.SFB.sf_int)
				timeout (psb->sb.SFB.sf_int, (caddr_t)psb, ONE_MSEC);
			splx (oip);
			return (SDI_RET_OK);
		}
		lp->jobs.t.function++;
		SC(c).jobs++;
		lp->fp = psb;
		psb->sp_time = ONE_MINUTE + 1;
		psb->sp_sent = FALSE;
		send_job (c, t, l);
		splx (oip);
		return (SDI_RET_OK);
		break;

	case	ISCB_TYPE:
		c = SDI_CONTROL(psb->sb.SCB.sc_dev.sa_fill);
		t = SDI_TARGET(psb->sb.SCB.sc_dev.sa_fill);
		l = psb->sb.SCB.sc_dev.sa_lun;
		lp = &LU(c, t, l);
#ifdef DEBUG
		DPR (8,2)(" sdi_icmd ISCB c%d t%d l%d ",c,t,l);
#endif

		if (SDI_ILLEGAL(c, t, psb->sb.SCB.sc_dev.sa_major)) 
		{
	      		cmn_err (CE_WARN,"SCSI: sdi_icmd called for an unequipped or illegal tc %u.\n",t);

			psb->sb.SCB.sc_comp_code = SDI_SCBERR;
			if (psb->sb.SCB.sc_int)
				timeout (psb->sb.SCB.sc_int, (caddr_t)psb, ONE_MSEC);
			splx (oip);
			return (SDI_RET_OK);
		}
		/* WD HA does not support > 128K jobs */ 
		if (psb->sb.SCB.sc_datasz > 131072) 
		{
			psb->sb.SCB.sc_comp_code = SDI_SCBERR;
			return (SDI_RET_ERR);
		}
		if (((SC(c).state & C_OPERATIONAL) == NULL) && 
					(!lp->jobs.t.suspended))
		{
			psb->sb.SCB.sc_comp_code = SDI_OOS;
			if (psb->sb.SCB.sc_int)
				timeout (psb->sb.SCB.sc_int, (caddr_t)psb, ONE_MSEC);
			splx (oip);
			return (SDI_RET_OK);
		}
		if ((lp->jobs.t.immediate) && 
			(!(lp->jobs.t.suspended & PT_ACTIVE))) 
		{
			psb->sb.SCB.sc_comp_code = SDI_ONEIC;
			if (psb->sb.SCB.sc_int)
				timeout (psb->sb.SCB.sc_int, (caddr_t)psb, ONE_MSEC);
			splx (oip);
			return (SDI_RET_OK);
		}
		if (psb->sb.SCB.sc_time < 0)
		{
			cmn_err (CE_WARN,
			  "SCSI: sdi_icmd called with negative timeout value.\n");
			psb->sb.SCB.sc_comp_code = SDI_SCBERR;
			if (psb->sb.SCB.sc_int)
				timeout (psb->sb.SCB.sc_int, (caddr_t)psb, ONE_MSEC);
			splx (oip);
			return (SDI_RET_OK);
		}

		/* convert milliseconds to ticks; but only if > 0 */
		if (psb->sb.SCB.sc_time)
		{
			if (psb->sb.SCB.sc_time < 1000 * 1000)
				psb->sb.SCB.sc_time = drv_usectohz(psb->sb.SCB.sc_time * 1000); 
			else
				psb->sb.SCB.sc_time = drv_usectohz(psb->sb.SCB.sc_time) * 1000; 
		}

		psb->sb.SCB.sc_comp_code = SDI_PROGRES;
		lp->jobs.t.immediate++;
		SC(c).jobs++;
		add_ijob (psb, c, t, l, FALSE);
		send_job (c, t, l);
		splx (oip);
		return (SDI_RET_OK);
		break;
	default:
		cmn_err (CE_WARN,"SCSI: sdi_icmd called with an illegal type of %u.\n", psb->sb.sb_type);

		splx (oip);
		return (SDI_RET_ERR);
		break;
	}
}

/*============================================================================
** Function name: sdi_translate()
** Description:
**	This function performs the virtual to physical translation on the
**	SCB data pointer. 
*/

void
sdi_translate (sp, flag, procp)
register struct sb2	*sp;
register int		flag;
struct   proc		*procp;
{

#ifdef DEBUG
	DPR (8,2)("sdi_translate ");
#endif

	if (sp == NULL) 
	{
		cmn_err (CE_WARN,
			"SCSI: sdi_translate called with an illegal pointer");
		return;
	}
	if (sp->sp_vip) 
	{
		scsi_dmafreelist ((DMA_LIST *)sp->sp_vip);
		sp->sp_vip = NULL;
		sp->sp_isz = NULL;
	}
	if (sp->sb.SCB.sc_link != NULL) 
	{
	     cmn_err (CE_WARN,"SCSI: Linked commands NOT available.\n");
	     sp->sb.SCB.sc_link = NULL;
	}
	if (sp->sb.SCB.sc_datapt != NULL) 
	{
		if (!(flag&B_PHYS))
		{
			if ((sp->sp_pdb = vtop (sp->sb.SCB.sc_datapt, procp)) == NULL)
			{
				cmn_err (CE_PANIC,
				       "SCSI: Bad address returned by VTOP.\n");
			}
		}
		else
		/*
		 * The following path (B_PHYS) should only be run using newer
		 * WD Host Adaptors. The older WD controllers do not support 
		 * scatter/gather. 
		 */ 
		{
			scsi_dmamakelist (sp, sp->sb.SCB.sc_datapt,
						sp->sb.SCB.sc_datasz, procp);
			if ((sp->sp_pdb = vtop ((caddr_t)sp->sp_vip, procp)) == NULL) 
			{
				cmn_err (CE_PANIC,
				       "SCSI: Bad address returned by VTOP.\n");
			}
#ifdef	DEBUG
	DPR (8,5)("sdi_translate: sp->sp_isz=%X, sp->sp_vip=%X, sp->sp_pdb=%X\n",
					sp->sp_isz, sp->sp_vip, sp->sp_pdb);
#endif
		}
	}
	else
	{
		sp->sp_pdb = NULL;
		sp->sb.SCB.sc_datasz = NULL;
	}
	sp->sp_read = (flag & B_READ) ? 1 : 0;	/* set job direction */

	return;
}

/*============================================================================
** Function name: sdi_getblk()
** Description:
**	This function returns a SCB to the caller. The function will 
**	sleep if there are no SCSI blocks available.
**	
*/

struct sb *
sdi_getblk ()
{
	register int		oip;
	register union sb_item	*psb;

	oip = spl5 ();
	while (scsi_sblist == NULL)
		sleep ((caddr_t)&scsi_sblist, PZERO);

	psb = scsi_sblist;
	scsi_sblist = scsi_sblist->next;
	psb->next = NULL;
	splx (oip);

	if (psb->sb2.sb.SCB.sc_comp_code != SDI_NOALLOC)
		cmn_err (CE_WARN,
		       "SCSI: sdi_getblk called with a corrupted free list.\n");

	psb->sb2.sb.SCB.sc_comp_code = SDI_UNUSED;

	return ((struct sb *)psb);
}


/*============================================================================
** Function name: sdi_freeblk()
** Description:
**	This function frees up a previously allocated SCB structure.
**	The SCB is returned to the pool and if there is a scatter/gather
**	list associated with the SCB it is freed via scsi_dmafreelist().
**	A nonzero return indicates an error in the pointer or type field.
*/

long
sdi_freeblk (psb)
register union sb_item	*psb;
{
	int	oip;

	if (psb == NULL)
	{
		cmn_err (CE_WARN,
			"SCSI: sdi_freeblk called with an illegal pointer.\n");
		return (SDI_RET_ERR);
	}
	if ((psb < (union sb_item *) scsi_sbstart) || 
			(psb > (union sb_item *) scsi_sbend))
	{
		cmn_err (CE_WARN,
		       "SCSI: sdi_freeblk SCB not allocated by SCSI driver.\n");
		return (SDI_RET_ERR);
	}
#ifdef never
/* This test was rejecting unused blocks allocated by sdi_getblk().  VRS */
	if ((psb->sb2.sb.sb_type != SCB_TYPE) &&
		(psb->sb2.sb.sb_type != SFB_TYPE) &&
			(psb->sb2.sb.sb_type != ISCB_TYPE)) 
	{
		cmn_err (CE_WARN,
		      "SCSI: sdi_freeblk called with a corrupted free list.\n");
		return (SDI_RET_ERR);
	}
#else
	if (psb->sb2.sb.SCB.sc_comp_code == SDI_NOALLOC)
	{
		cmn_err (CE_WARN,
		      "SCSI: sdi_freeblk called with a free block.\n");
		return (SDI_RET_ERR);
	}
#endif
	psb->sb2.sb.SCB.sc_comp_code = SDI_NOALLOC;

	if (psb->sb2.sp_vip)
	{
		scsi_dmafreelist ((DMA_LIST *)psb->sb2.sp_vip);
		psb->sb2.sp_vip = NULL;
		psb->sb2.sp_isz = NULL;
	}
	oip = spl5 ();
	psb->next = scsi_sblist;
	scsi_sblist = psb;

	if (psb->next == NULL)
		wakeup ((caddr_t)&scsi_sblist);

	splx (oip);
	return (SDI_RET_OK);
}

/*============================================================================
** Function name: sdi_name()
** Description:
**	This function returns the name of the given device. It decodes the 
**	major number and produces a string indicating the name of the 
**	addressed controller. The name is copied into a string pointed to 
**	by the second argument. The returned string may be as long as 48 bytes.
**	This function is only for use by the target driver when it wants 
**	to print a message.
*/

void
sdi_name (addr, name)
struct scsi_ad	*addr;
char		*name;
{
	register int	c = SDI_CONTROL(addr->sa_fill);
	register int	t = SDI_TARGET(addr->sa_fill);
	register int	i = 0;

#ifdef DEBUG
	DPR (8,2)(" sdi_name c%d t%d maj%d ",c,t,addr->sa_major);
#endif

	if (SDI_ILLEGAL(c, t, addr->sa_major))
	{
	      	cmn_err (CE_WARN,"SCSI: sdi_name called for an unequipped or illegal HA %d tc %u.\n",c,t);
		return;
	}
	name[i++] = 'H';
	name[i++] = 'A';
	name[i++] = ' ';

	if (c > 9) {
		name[i++] = '1';
		name[i++] = '0' + (c - 10);
	}
	else {
		name[i++] = '0' + c;
	}
	name[i++] = ' ';
	name[i++] = 'T';
	name[i++] = 'C';
	name[i++] = ' ';
	name[i++] = '0' + t;
	name[i] = NULL;

	return;
}


/*============================================================================
** Function name: sdi_getdev()
** Description:
**	This function given a target driver major number returns a device
**	number major/minor to be used to do pass thru operations.
*/

void
sdi_getdev (sa, pdev)
struct scsi_ad	*sa;
dev_t		*pdev;
{
	int	c = SDI_CONTROL(sa->sa_fill);
	int	t = SDI_TARGET(sa->sa_fill);

	if (SDI_ILLEGAL(c, t, sa->sa_major))
	{
	      	cmn_err (CE_WARN,"SCSI: sdi_getdev called for an unequipped or illegal tc %u.\n",t);
		return;
	}

	*pdev = makedevice(SC(c).maj, ((c << 5) | (t << 2) | sa->sa_lun));

	return;
}

/*============================================================================
** Function name: sdi_fltinit()
** Description:
**	This function is called by the target drivers to enable their 
**	event handler for a given TC LU conbination. The event handler 
**	is called when a SCSI Bus reset occurs or when the LU is closed 
**	after a pass-thru operation.
*/

void
sdi_fltinit(sa, func, param)
struct scsi_ad *sa;
void (*func)();
long param;
{
	int	c = SDI_CONTROL(sa->sa_fill);
	int	t = SDI_TARGET(sa->sa_fill);
	int	l = sa->sa_lun;
	register struct	lu_ctrl	*lup;

	lup = &LU(c, t, l);
        lup->dv_func = func;
        lup->dv_param = param;

	return;
}

/*============================================================================
** Function name: sdi_swap16()
** Description:
**	This function swaps bytes in a 16 bit data type.
*/

short
sdi_swap16(padr)
unsigned int	padr;
{
	unsigned short	i=0;

	i = ((padr & 0x00FF) << 8);
	i |= ((padr & 0xFF00) >> 8);

	return (i);
}

/*============================================================================
** Function name: sdi_swap24()
** Description:
**	This function swaps bytes in a 24 bit data type.
*/

sdi_swap24(padr)
unsigned int	padr;
{
	unsigned int	i=0;

	i = ((padr & 0x0000FF) << 16);
	i |= (padr & 0x00FF00);
	i |= ((padr & 0xFF0000) >> 16);

	return (i);
}

/*============================================================================
** Function name: sdi_swap32()
** Description:
**	This function swaps bytes in a 32 bit data type.
*/

long
sdi_swap32(padr)
unsigned long	padr;
{
	unsigned long	i=0;

	i = ((padr & 0x000000FF) << 24);
	i |= ((padr & 0x0000FF00) << 8);
	i |= ((padr & 0x00FF0000) >> 8);
	i |= ((padr & 0xFF000000) >> 24);

	return (i);
}

/*============================================================================
** Function name: sdi_alloc()
** Description:
**	This function allows the host adapter driver to allocate
**	memory which may be more efficient than main store.
**	This memory may NOT be in the address space of the CPU and
**	should only be accessed by routines provided by the host
**	adapter driver. A NULL return indicates that storage could NOT
**	be allocated.
**	This function is not supported by this implementation of SDI.
*/

char *
sdi_alloc (size, addr)
int		size;
struct scsi_ad	*addr;
{
	return (NULL);
}

/*============================================================================
** Function name: sdi_free()
** Description:
**	Free up the previously allocated "special" memory.
**	This function should free up space allocated by sdi_alloc
**	only, and returns nonzero on error.
*/

int
sdi_free (pt)
char	*pt;
{
	return (TRUE);
}

/*============================================================================
** Function name: sdi_copy()
** Description:
**	Copies data from the CPU's memory to the "special"
**	memory allocated by the sdi_alloc routine.
**	If the IMMED bit is set, the function will not return until
**	the data is copied. If the IMMED bit is not set, the function
**	may return before the copy is completed; however, the host
**	adapter driver must gaurantee that the copy will be complete
**	before the data is used by a SCSI command.
**	If the copy fails a value of -1 is returned.
**	If a delayed copy fails, any SCSI job accessing that area will
**	be returned with the MEMERR completion status.
**	This function is not supported by this implementation of SDI.
*/

long
sdi_copy (cmd, pt1, pt2, size)
long	cmd;
char	*pt1;
char	*pt2;
long	size;
{
	return (-1);
}

/*============================================================================
** Function name: ll_send_cmd()
** Description:
**	This function starts a pass-thru job. The user SCB and CDB is read
**	into a local SCB and CDB. Then ll_send_scb() is called to do the
**	work. Upon return the local SCB is copied back to the user SCB.
*/

int
ll_send_cmd (dev, pa)
dev_t		dev;
struct sb	*pa;
{
	struct sb		*psb;
	struct scsi_cdb_buf	*p;
	caddr_t			user_cdb;
	int			c = SC_CONTROL(getminor (dev));
	int			t = SC_TARGET(getminor (dev));
	int 			uerror = 0;
	if (t == SC(c).ha_id) 
	{
		cmn_err (CE_WARN,"SCSI: Pass-thru was tried with an illegal id %u, HA %u.\n", SC(c).ha_id, c);
		return (ENXIO);
	}
	psb = sdi_getblk ();

	if (copyin ((caddr_t)pa, (caddr_t)psb, sizeof (struct sb)) != SUCCESS)
	{
		sdi_freeblk (psb);
		return(EFAULT);
	}
	if ((psb->sb_type == ISCB_TYPE) && (psb->SCB.sc_cmdpt != NULL) &&
		(psb->SCB.sc_cmdsz > NULL) && 
			(psb->SCB.sc_cmdsz <= MAX_CMDSZ))
	{
		user_cdb = psb->SCB.sc_cmdpt;
		p = get_cdb ();
		if (copyin (user_cdb, p->cdb, 
				psb->SCB.sc_cmdsz) == SUCCESS)
		{
			psb->SCB.sc_cmdpt = (caddr_t) p;
			uerror = ll_send_scb (dev, psb);

			psb->SCB.sc_cmdpt = user_cdb;
			if (copyout ((caddr_t)psb, (caddr_t)pa, sizeof(struct sb)) != SUCCESS)
				uerror = EFAULT;
		}
		else
			uerror = EFAULT;

		free_cdb (p);
	}
	else 
		uerror = EINVAL;

	sdi_freeblk (psb);
	return(uerror);
}

/*============================================================================
** Function name: ll_send_scb()
** Description:
**	This function given the SCB starts the I/O request for the 
**	pass-thru job. If the job involves a data transfer then the 
**	request if done thru physio() so that the user data area is 
**	locked in memory. If the job doesn't involve any data transfer 
**	then scsi_pass_thru() is called directly.
*/

int
ll_send_scb (dev, psb)
dev_t		dev;
struct sb2	*psb;
{
	long			user_wd;
	caddr_t			user_datapt;
	struct scsi_cdb_buf	*ha_cdb;
	buf_t			*bp;
	int			uerror = 0;
	struct uio		hauio;
	struct iovec		havec;
	
	
	ha_cdb = (struct scsi_cdb_buf *) psb->sb.SCB.sc_cmdpt;
	bp = &(ha_cdb->bp);
	psb->sb.SCB.sc_cmdpt = (caddr_t) ha_cdb->cdb;

	if (psb->sb.SCB.sc_mode & SCB_READ)
		bp->b_flags = B_READ;
	else
		bp->b_flags = B_WRITE;

	user_wd = psb->sb.SCB.sc_wd;		/* save user word     */
	user_datapt = psb->sb.SCB.sc_datapt;	/* save user data ptr */
	psb->sb.SCB.sc_wd = (long)bp;
	psb->sb.SCB.sc_int = scsi_int;

	bp->b_resid = (long)psb;

	havec.iov_base = (psb->sb.SCB.sc_datapt);	
	havec.iov_len = (psb->sb.SCB.sc_datasz);	
	hauio.uio_iov = &havec;
	hauio.uio_iovcnt = 1;
	hauio.uio_offset = 0;
	hauio.uio_segflg = UIO_USERSPACE;
	hauio.uio_fmode = 0;
	hauio.uio_resid = (psb->sb.SCB.sc_datasz);

	if (psb->sb.SCB.sc_datasz == NULL)
	{ 
		bp->b_flags |= B_BUSY | B_PHYS;
		bp->b_un.b_addr = psb->sb.SCB.sc_datapt;
		bp->b_bcount = psb->sb.SCB.sc_datasz;
		bp->b_blkno = NULL;
		bp->b_edev = dev;
				
		scsi_pass_thru (bp);	/* Fake physio call */
		iowait (bp);
	}
	else 
	{
		uerror = physiock((void(*)())scsi_pass_thru, bp, dev, bp->b_flags, 
							256, &hauio);
	}
	/* restore user SCB modified fields */

	psb->sb.SCB.sc_wd = user_wd;
	psb->sb.SCB.sc_datapt = user_datapt;
	psb->sb.SCB.sc_int = NULL;

	return(uerror);
}

/*============================================================================
** Function name: scsi_pass_thru()
** Description:
**	This function checks if a pass-tru job can be performed. If so
**	The job is sent to the Ha board.
*/
int
scsi_pass_thru (bp)
buf_t	*bp;
{
	struct sb2	*psb;
	int		oip;
	int		c = SC_CONTROL(getminor (bp->b_edev));
	int		t = SC_TARGET(getminor (bp->b_edev));
	int		l = SC_LUN(getminor (bp->b_edev));
	struct proc	*procp;

	psb = (struct sb2 *)bp->b_resid;
	psb->sb.SCB.sc_dev.sa_fill = ((c << 3) | t);
	if (psb->sb.SCB.sc_wd != (long)bp)
	{
	     cmn_err (CE_WARN,"SCSI: Corrupted address from physio.\n");
	     iodone (bp);
	     return(EIO);
	}
	psb->sb.SCB.sc_datapt = (caddr_t) paddr(bp);
	drv_getparm (UPROCP, (unsigned long) &procp);
	sdi_translate (psb, bp->b_flags, procp);
	oip = spl5 ();
	if ((SC(c).state & C_OPERATIONAL) == NULL) 
	{
		splx (oip);
		psb->sb.SCB.sc_comp_code = SDI_OOS;
		iodone (bp);
		return(EIO);
	}
	/* allow only one pass-tru job at a time */
	if ((LU(c, t, l).jobs.t.immediate) && 
		(LU(c, t, l).jobs.t.suspended & PT_ACTIVE))
	{
		splx (oip);
		psb->sb.SCB.sc_comp_code = SDI_ONEIC;
		iodone (bp);
		return(EBUSY);
	}
	LU(c, t, l).jobs.t.immediate++;
	SC(c).jobs++;
	add_ijob (psb, c, t, l, TRUE);
	send_job (c, t, l);
	splx (oip);

	return(0);
}
/*============================================================================
** Function name: scsi_int()
** Description:
**	This is the interrupt handler for pass-thru jobs. It basically
**	just wakes up the ll_send_scb() function.
*/

void
scsi_int (psb)
struct sb	*psb;
{
	buf_t	*bp = (buf_t *)psb->SCB.sc_wd;

	if (bp->b_resid != (long)psb)
	{
		cmn_err (CE_WARN,"SCSI: Corrupted address returned during pass through operation.\n");
		return;
	}
	if (psb->SCB.sc_comp_code == SDI_PROGRES)
	{
		cmn_err (CE_WARN,"SCSI: Bad completion code returned during pass through operation.\n");
		psb->SCB.sc_comp_code = SDI_ASW;
	}
	bp->b_resid = NULL;
	iodone (bp);
}

/*============================================================================
** Function name: get_cbd()
** Description:
**	This routine returns the next available cdb from the free cdb
**	list. If none are available, the process is put to sleep
**	until one becomes available. The free list is simply a
**	NULL terminated singly linked list of cdb's.
**
**			+----+		+----+		+----+
**  scsi_cdb_list ----->|next|--------->|next|--------->|next|-|-|-
**			+----+		+----+		+----+
**
*/

static struct scsi_cdb_buf *
get_cdb ()
{
	register union cdb_item	*cdb;
	register int	oip;
	
	oip = spl5 ();
	while (scsi_cdb_list == NULL)
		sleep ((caddr_t)&scsi_cdb_list, PZERO);

	cdb = scsi_cdb_list;
	scsi_cdb_list = cdb->next;
	cdb->next = NULL;

	splx (oip);
	return ((struct scsi_cdb_buf *)cdb);
}

/*============================================================================
** Function name: free_cdb()
** Description:
**	This routine frees up the specified cdb and places it back on
**	the free list of cdb's. If the free list was empty, a wakeup
**	call is made to allow any processes that were waiting for a
**	cdb to fight for the one that was just freed.
*/

static void
free_cdb (cdb)
register union cdb_item	*cdb;
{
	register int	oip;
	
	oip = spl5 ();
	cdb->next = scsi_cdb_list;
	scsi_cdb_list = cdb;

	if (cdb->next == NULL)
		wakeup ((caddr_t)&scsi_cdb_list);
	splx (oip);

	return;
}

/*============================================================================
** Function name: send_job()
** Description:
**	This function will attempt to send the first job on the queue
**	for the given TC LU. All jobs are not sent if the Q is busy.
**	Normal jobs are sent if:
**		1. No reset timing is in progress
**		2. The LU is not pump_suspended OR pass-tru suspended
**		   OR suspended because of an error.
**	Immediate jobs are sent if:
**		1. We not doing the 5 second reset delay.
**		2. We are not pump suspended.
**		3. If open for pass-tru then only pass-thru jobs can be 
**		   sent. They are indicated by the PT_ACTIVE flag being set.
*/

static int
send_job (c, t, l)
int	c;	/* HA Controller 	*/
int	t;	/* target controller	*/
int	l;	/* logical unit		*/
{
	register struct lu_ctrl	*lup;	/* LU pointer	*/
	register struct sb2	*sp;	/* sb pointer 	*/
	int			cmd;	/* SFB type cmd */
	int			retcode;/* return code	*/

#ifdef DEBUG
	DPR (8,4)("send_job c%d t%d l%d ",c,t,l);
#endif
	ha_ready(c);
	retcode = FALSE;

	if (scsi_getq (c) >= NUM_OGMB)
	{
#ifdef DEBUG
		DPR (8,0)("send_job: NO RQs ");
#endif
		return (retcode);
	}
	lup = &LU(c, t, l);

	/* check if there are free SRBs and there are jobs to be sent */

	if ((SC(c).srb_list != NULL) && (((TC(c, t).waiting_jobs) && 
			(lup->jp) && (!lup->jp->sp_sent)) || (lup->fp)))
	{
		if (lup->fp)
			sp = lup->fp;
		else
			sp = lup->jp;
#ifdef DEBUG
		if (sp->sp_sent)
		{
  	  		DPR (8,0)("TRIED to send a job twice!!!! sent=0x%x\n",
				sp->sp_sent);
	  		return (retcode);
		}
#endif
		if (sp->sb.sb_type == SCB_TYPE)
		{
			if ((!(SC(c).state & C_RS_BLK)) &&
					!lup->jobs.t.suspended)
			{
				SC(c).waiting_jobs--;
				TC(c, t).waiting_jobs--;
				retcode = TRUE;
				enqueue (c, SCSI_CTL, SUBDEV(t, l), sp->sp_pdb, 
						sp->sb.SCB.sc_datasz, sp);
			}
		}
		else	/* its an immediate job	*/
		{
			if ((!(SC(c).state & C_RS_START)) &&
				(!(lup->jobs.t.suspended & PUMP_SUSPEND)))
			{
				if ((!(lup->jobs.t.suspended & PT_SUSPEND)) ||
				    ((lup->jobs.t.suspended & PT_SUSPEND) &&
				     (lup->jobs.t.suspended & PT_ACTIVE)))
				{
					if (sp->sb.sb_type == ISCB_TYPE)
					{
				  
						SC(c).waiting_jobs--;
						TC(c, t).waiting_jobs--;
					   	enqueue (c, SCSI_CTL, 
							SUBDEV(t, l),
						    	sp->sp_pdb, 
						    	sp->sb.SCB.sc_datasz, 
							sp);
					}
					else
					{
						if (sp->sb.SFB.sf_func == TCRST)
							cmd = TCRST;
						else
							cmd = ABORT;
						enqueue (c, cmd, SUBDEV(t, l),
						    	NULL, NULL, NULL);
					}
					retcode = TRUE;
				}
			}
		}
	}
	return (retcode);
}

/*============================================================================
** Function name: enqueue()
** Description:
**	This function fills in the request queue entry and interrupts the
**	HA board. Non SCB jobs generate express ints. while SCB jobs
**	generate normal interrupts.
*/

static void
enqueue (c, cmd, dev, addr, length, sp)
int			c;	/* Controller */
char			cmd;	/* Command    */
char			dev;	/* Subdevice  */
long			addr;	/* Address    */
int			length;	/* Length     */
register struct sb2	*sp;	/* sb pointer */
{

	SCSI_RB		*srb;	/* SCSI cmd request block ptr 	*/
	int		rqn;	/* index of request Q to use	*/
	caddr_t		p;	/* pointer to CDB in the SCB	*/
	int		i;

#ifdef DEBUG
	DPR (8,4)("enqueue c%d dev=%x addr=%x len=%x sp=%x\n", c, dev,
			addr, length, sp);
#endif

	SC(c).active_jobs++;
	sp->sp_sent = TRUE;

	/* get a free SRB from list */

	srb = (SCSI_RB *) SC(c).srb_list;
	SC(c).srb_list = SC(c).srb_list->next;
	((union srb_buf *)srb)->next = NULL;

	/* fill in the SCSI request block */

	srb->dev = dev;
	srb->ssb = NULL;
	srb->hasb = NULL;
	if (sp->sp_isz)	/* raw mode, i.e, scatter/gather */
	{
		srb->opcode = SCSI_DMA_CMD;
		srb->xfer_size[0] = msbyte(sp->sp_isz);
		srb->xfer_size[1] = mdbyte(sp->sp_isz);
		srb->xfer_size[2] = lsbyte(sp->sp_isz);
	}
	else		/* block mode, i.e, NO scatter/gather */
	{
		srb->opcode = SCSI_CMD;
		srb->xfer_size[0] = msbyte(length);
		srb->xfer_size[1] = mdbyte(length);
		srb->xfer_size[2] = lsbyte(length);
	}
	srb->pdp[0] = msbyte(addr);
	srb->pdp[1] = mdbyte(addr);
	srb->pdp[2] = lsbyte(addr);
	srb->link[0] = NULL;
	srb->link[1] = NULL;
	srb->link[2] = NULL;
	srb->rw = (sp->sp_read) ? 1 : 0;	/* set job direction 	*/

	/* copy SCB cdb to SCSI request block */

	p = sp->sb.SCB.sc_cmdpt;
	for (i = 0; i < sp->sb.SCB.sc_cmdsz; i++)
		srb->cdb[i] = *p++;
	
	/* set up the request Q entry */

	rqn = scsi_getq (c);
	if ((SC(c).rq[rqn].pdp = sdi_swap24(vtop((caddr_t)srb, NULL))) == NULL)
		cmn_err (CE_PANIC,"SCSI: Bad address returned by VTOP.\n");

	SC(c).rq[rqn].status = RQ_BUSY;

	cmd = (RD_MB | (rqn & 0x3F));	/* send cmd to HA board */
	ha_cmd(c, cmd);
	return;
}

/*============================================================================
** Function name: add_job()
** Description:
**	This function adds normal jobs to the LU queue. The job is
**	always added to the end of the list.
**
**  +-------------------+  forw  +------------------+
**  | LU(c, t, l).jp    | -----> | next job to send |
**  | first job to send | <----- |                  |
**  +-------------------+  back  +------------------+
*/

static void
add_job (sp, c, t, l)
register struct sb2	*sp;
int			c;
int			t;
int			l;
{
	register struct lu_ctrl	*lp = &LU(c, t, l);

#ifdef DEBUG
	DPR (8,4)("add_job c%d t%d l%d ",c,t,l);
#endif
	if (lp->jp == NULL)
	{
		lp->jp = sp;
		sp->sp_forw = sp;
		sp->sp_back = sp;
		SC(c).waiting_jobs++;
		TC(c, t).waiting_jobs++;
	}
	else
	{
		sp->sp_forw = lp->jp;
		sp->sp_back = lp->jp->sp_back;
		lp->jp->sp_back->sp_forw = sp;
		lp->jp->sp_back = sp;
	}
	if (sp->sb.SCB.sc_time)
		sp->sp_time = ONE_MINUTE + sp->sb.SCB.sc_time;
	else
		sp->sp_time = ONE_WEEK;
	return;
}


/*============================================================================
** Function name: add_ijob()
** Description:
** 	This function adds an immediate job to the LU queue. Immediate jobs
**	are always added to the head of the list except if a jobs has already
**	been sent or there is a pass-tru job on the Q in which case the job
**	is added behind that first job.
*/

static void
add_ijob (sp, c, t, l, ptru)
register struct sb2	*sp;
int			c;
int			t;
int			l;
int			ptru; 	/* TRUE if pass-thru job */
{
	register struct lu_ctrl	*lp = &LU(c, t, l);

	if (lp->jp == NULL)
	{	
		lp->jp = sp;
		sp->sp_forw = sp;
		sp->sp_back = sp;
		SC(c).waiting_jobs++;
		TC(c, t).waiting_jobs++;
	}
	else if ((lp->jp->sp_sent) || (lp->jobs.t.suspended & PT_ACTIVE))
	{
		sp->sp_forw = lp->jp->sp_forw;
		lp->jp->sp_forw = sp;
		sp->sp_back = lp->jp;
		sp->sp_forw->sp_back = sp;
	}
	else 
	{
		sp->sp_forw = lp->jp;
		sp->sp_back = lp->jp->sp_back;
		lp->jp->sp_back->sp_forw = sp;
		lp->jp->sp_back = sp;
		lp->jp = sp;
	}
	if (sp->sb.SCB.sc_time)
		sp->sp_time = ONE_MINUTE + sp->sb.SCB.sc_time;
	else
		sp->sp_time = ONE_WEEK;
	if (ptru)
		lp->jobs.t.suspended |= PT_ACTIVE;

	return;
}


/*============================================================================
** Function name: sub_job()
** Description:
**	This function removes the first job from the LU Q. The first
**	job on the Q is passed into the function.
*/

static void
sub_job (sp, c, t, l)
register struct sb2	*sp;
int			c;
int			t;
int			l;
{
	register struct lu_ctrl	*lp = &LU(c, t, l);

#ifdef DEBUG
	if (!sp->sp_sent)
  		DPR (8,0)("Subtracting a job that has NOT been sent.\n");
#endif
	if (sp->sp_forw == sp)
		lp->jp = NULL;
	else 
	{
		lp->jp = sp->sp_forw;
		sp->sp_back->sp_forw = sp->sp_forw;
		sp->sp_forw->sp_back = sp->sp_back;
		SC(c).waiting_jobs++;
		TC(c, t).waiting_jobs++;
	}
	sp->sp_forw = NULL;
	sp->sp_back = NULL;
	sp->sp_time = NULL;
	sp->sp_sent = FALSE;

	return;
}

/*============================================================================
** Function name: next_job()
** Description:
**	This function sends the next available job to the ha board.
**	The next available job is the first job found in the Q which
**	can be sent. The routine stops when a job is sent or when
**	all the queues for this board have been searched. The search
**	allways starts it the Q pointed by next_tc next_lu vars.
*/

static void
next_job (c)
int	c;
{
	register struct scsi_ctrl *cp = &SC(c);
	register struct tc_ctrl *tp;
	register int		t;
	register int		l;
	int			last_tc, done, tc_cnt;

	if ((cp->srb_list == NULL) || (cp->state & C_RS_START) || 
					(cp->waiting_jobs == NULL))
	{
		return;
	}
	last_tc = MAX_TCS;
	tc_cnt = 0; 
	done = FALSE;
	while (done == FALSE)
	{
		for (t = cp->next_tc, tp = &cp->tc[t]; 
			((t < last_tc) && (done == FALSE)); t++, tp++, tc_cnt++)
		{
			if (tp->waiting_jobs > 0)
			{
				for (l = cp->next_lu;
					((l < MAX_LUS) && (done == FALSE));l++) 
				{
					if (send_job (c, t, l))
					{
					    done = TRUE;
					    break;
					}
				}
				if (done == TRUE)
				{
					cp->next_lu = (l + 1) % MAX_LUS;
					if (cp->next_lu == 0)
						cp->next_tc = (t + 1) % MAX_TCS;
					else
						cp->next_tc = t;
				}
				else
					cp->next_lu = NULL;
			}
			else
				cp->next_lu = NULL;
		}
		if (done == FALSE)
		{
			if (tc_cnt >= MAX_TCS)
				done = TRUE;
			else
			{
				last_tc = (cp->next_tc + 1) % (unsigned)MAX_TCS;
				cp->next_tc = 0;
			}
		}
	}
	return;
}

/*============================================================================
** Function name: stop_jobs()
** Description:
**	Used to stop all jobs in order to allow the board to be pumped.
**	This function is also used in other cases to stop jobs.
*/

static void
stop_jobs (c)
register int	c;
{
	register int	t, l, oip;

	oip = spl5 ();

	for (t = 0; t < MAX_TCS; t++) 
	{
		for (l = 0; l < MAX_LUS; l++)
			LU(c, t, l).jobs.t.suspended |= PUMP_SUSPEND;
	}
	splx (oip);
	return;
}

/*============================================================================
** Function name: start_jobs()
** Description:
**	Used to restart jobs after they have been stopped.
*/

static void
start_jobs (c)
register int	c;
{
	register int	t;
	register int	l;
	register int	oip;
	register struct lu_ctrl	*lp;

	oip = spl5 ();

	for (t = 0; t < MAX_TCS; t++)
	{
		for (l = 0; l < MAX_LUS; l++)
		{
			lp = &LU(c, t, l);
			lp->jobs.t.suspended &= ~PUMP_SUSPEND;
			send_job (c, t, l);
		}
	}
	splx (oip);
}

/*---------------------------------------------------------------------------*/
/* SCSI Host Adapter Driver Utilities
/*---------------------------------------------------------------------------*/

/*============================================================================
** Function name: ha_dgn()
** Description:
**	Check HA board diagnostic results. The HA is still usable even
**	if the diagnostics show a failure.
*/

ha_dgn(c)
register int	c;	/* ha controller number */
{
	unsigned char status;

	status = inb(INT_STATUS(c));

	switch (status)
	{
	case DGN_ATP:		/* DGN ran ATP */
		break;
	case DGN_NTR:
		cmn_err(CE_NOTE,"SCSI: HA %d DGN NTR.\n",c);
		break;
	case RAM_FAIL:
		cmn_err(CE_WARN,"SCSI: HA %d DGN RAM TESTS FAILED.\n",c);
		break;
	case FIFO_FAIL:
		cmn_err(CE_WARN,"SCSI: HA %d DGN FIFO TESTS FAILED.\n",c);
		break;
	case SBIC_FAIL:
		cmn_err(CE_WARN,"SCSI: HA %d DGN SBIC TESTS FAILED.",c);
		break;
	case DFF_FAIL:
		cmn_err(CE_WARN,"SCSI: HA %d DGN INIT FF FAILED.\n",c);
		break;
	case IRQ_FAIL:
		cmn_err(CE_WARN,"SCSI: HA %d DGN IRQ FF FAILED.\n",c);
		break;
	case CKSUM_FAIL:
		cmn_err(CE_WARN,"SCSI: HA %d DGN ROM CHECK SUM FAILED.\n",c);
		break;
	default:
		cmn_err(CE_WARN,"SCSI: HA %d DGN Illegal status = 0x%x.\n",
					c, status);
		break;
	}
	return ;
}

/*============================================================================
** Function name: ha_init()
** Description:
**	This function resets the HA board sets up the dma channel and
**	initializes the commucation queues.
*/

ha_init(c)
register int	c;	/* HA controller number */
{
	union init_blk	iblk;	/* init cmd block */
	int		i;
	unsigned int	padr;
	unsigned char 	status;
	int		retcode = FAIL;	/* assume fail */
	
	outb(CONTROL_REG(c), SCSI_PORT_RESET ); /* reset scsi bus */
	for (i = 0; i < 50; i++);
	outb(CONTROL_REG(c), 0x00);

	outb(CONTROL_REG(c),  ASC_RESET); /* reset ha board */
	outb(CONTROL_REG(c), 0x00);
	{
		int	time = THREE_MILL_U_SEC; /* 3 second to wait max */
		while (time > 0)
		{
			if (!(RD_HA_STATUS(c) & INT_FLAG))
			{
				drv_usecwait(1000);	/* wait 1 ms */
				time -= 1000;
			}
			else 
			{
			outb(INT_ACK(c),0); /* ack the int from reset */
			break;
			}
		}
	}
	drv_usecwait(THREE_MILL_U_SEC);	/* wait 3 seconds */

	if(inb(HA_STATUS(c)) == 0xFF)
	{
		cmn_err(CE_WARN,"SCSI Host Adapter %d not seen equipped at address 0x %X.\n",c,scsi_ad[c].ha_status);
		return(retcode);
	}

	if (!(RD_HA_STATUS(c) == CMD_READY))
	{
		cmn_err(CE_NOTE,"SCSI Host Adapter %d not ready to accept commands\n",c);
		return(retcode);
	}

#ifdef DEBUG
	DPR(8,0)("SCSI: HA %d RESET OK \n",c);
#endif
	ha_dgn (c);

	/* setup the initialization command block */

	iblk.init.opcode = INIT;
	iblk.init.ha_id = HA_ID;
	iblk.init.on_time = 24;
	iblk.init.off_time = 24;
	iblk.init.resv = 0;
	if ((padr = vtop((caddr_t)SC(c).rq,NULL)) == NULL)
		cmn_err (CE_PANIC,"SCSI: Bad address returned by VTOP.\n");

	iblk.init.mbox_pdp[0] = msbyte(padr);
	iblk.init.mbox_pdp[1] = mdbyte(padr);
	iblk.init.mbox_pdp[2] = lsbyte(padr);
	iblk.init.num_ic_mb = NUM_ICMB;
	iblk.init.num_og_mb = NUM_OGMB;

	/* sent init cmd one byte at a time */

	for (i = 0; i < ICMD_SIZE; i++)
		ha_cmd(c, iblk.data[i]);
	
	ha_ready(c);
	drv_usecwait(ONE_MILL_U_SEC);	/* 1 second */
	status = inb(HA_STATUS(c));

	if (RD_HA_STATUS(c) & INIT_FLAG)	/* init flag must be set */
	{
#ifdef DEBUG
		DPR(8,0)("SCSI: HA %d INIT OK st=0x%x\n",c,status);
#endif
		/* mark this HA operational */

		SC(c).state |= (C_OPERATIONAL | C_RAM);
		SC(c).max_jobs = MAX_RAM_JOBS;

		/* Set enable the ASC DMA Image */
		outb(CONTROL_REG(c), (INT_ENABLE | DMA_REQ_ENABLE)); 

		/* Disable the DMA Channel in the AT DMA chip */
		outb(DMA2WMR, (dmac[c] | CASCADE));	

		/* Enable the 386 DMA by clearing DMA Channel Mask */
		outb(DMA2WSMR, dmac[c]);	

		retcode = PASS;
	}
	else
	{
	     cmn_err(CE_WARN,"SCSI: HA %d Initialization failed status=0x%x.\n"
					,c,status);
	}
	return (retcode);
}

/*============================================================================
** Function name: ha_edt()
** Description:
**	This function builds the equipped device table for the given
**	SCSI Bus. It sends inquiries to every TC. Then, for all TCs
**	which answered the inquiry the number of LUs is calculated by 
**	calling scsi_cklu().
*/

static int
ha_edt (c)
register int	c;	/* HA controller number */
{
	struct scs	inq_cdb;
	int		t, lu, i, comp_code, config;
	char		*p;

#ifdef DEBUG
	DPR(8,3)("ha_edt: c=%d ", c);
#endif
	config = FALSE;		/* set to true if bus has at least 1 tc */
	for (t = 0; t < MAX_TCS; t++)	/* clear previous edt data */
	{
		sc_edt[c][t].c_maj = -1;
		sc_edt[c][t].b_maj = -1;
		sc_edt[c][t].pdtype = 63;
		sc_edt[c][t].tc_equip = NULL;
		sc_edt[c][t].ha_slot = NULL;
		sc_edt[c][t].n_lus = NULL;
		sc_edt[c][t].drv_name[0] = NULL;
		sc_edt[c][t].tc_inquiry[0] = NULL;
			for (lu = 0; lu < MAX_LUS; lu++)
				sc_edt[c][t].lu_id[lu] = NULL;
	}
	inq_cdb.ss_op = SS_INQUIR;	/* inquiry cdb		*/
	inq_cdb.ss_lun = NULL;		/* it first try lu 0	*/
	inq_cdb.ss_addr = NULL;
	inq_cdb.ss_addr1 = NULL;
	inq_cdb.ss_len = IDENT_SZ;
	inq_cdb.ss_cont = NULL;

	comp_code = scsi_cmd (c, SC(c).ha_id, 0, &inq_cdb, SCS_SZ, 
			&inq_data, IDENT_SZ, B_READ);

	if ((comp_code == SDI_ASW) || (comp_code == SDI_CKSTAT))
	{
		cmn_err(CE_WARN,"SCSI: Target set at Host Adapter SCSI ID. ");
		cmn_err(CE_CONT,"HA %d, Target match at ID %d \n",c,SC(c).ha_id);
		cmn_err(CE_CONT,"SCSI subsystem being turned off!!");
			return;
	}
	else if (comp_code == SDI_TIME)
	{
		cmn_err(CE_WARN,"SCSI: SCSI bus %d not functional.",c);
		return;
	}	

	for (t = 0; t < MAX_TCS; t++)	/* determine equipped TCs   */
	{
		if (SC(c).ha_id != t)	/* don't do it for the HA ID */
		{
			comp_code = scsi_cmd (c, t, 0, &inq_cdb, SCS_SZ, 
					&inq_data, IDENT_SZ, B_READ);
			if (comp_code == SDI_ASW)
			{
				config = TRUE;
				sc_edt[c][t].tc_equip = 1;
				sc_edt[c][t].ha_slot = c;
				p = &inq_data.id_vendor[0];
							
				for (i = 0; i < (INQ_LEN -1); i++, p++)
					sc_edt[c][t].tc_inquiry[i] = *p;
				sc_edt[c][t].tc_inquiry[INQ_LEN-1] = NULL;
			}
			else if (comp_code == SDI_TIME)
			{
				cmn_err(CE_WARN,"SCSI: SCSI bus %d not functional.",c);
				return;
			}	
		}
		else
		{
			sc_edt[c][t].tc_equip = 1;
			sc_edt[c][t].lu_id[0] = 1;
			sc_edt[c][t].c_maj = scsi_major;
			sc_edt[c][t].b_maj = -1;
			sc_edt[c][t].ha_slot = c;
			sc_edt[c][t].drv_name[0] = 'S';
			sc_edt[c][t].drv_name[1] = 'C';
			sc_edt[c][t].drv_name[2] = 'S';
			sc_edt[c][t].drv_name[3] = 'I';
			sc_edt[c][t].drv_name[4] = NULL;
			sc_edt[c][t].tc_inquiry[0]= 'R';
			sc_edt[c][t].tc_inquiry[1]= 'E';
			sc_edt[c][t].tc_inquiry[2]= 'V';
			sc_edt[c][t].tc_inquiry[3]= rd_ha_vers.bytes[0] + '0';
			sc_edt[c][t].tc_inquiry[4]= ' ';
			sc_edt[c][t].tc_inquiry[5]= 'S';
			sc_edt[c][t].tc_inquiry[6]= rd_ha_vers.bytes[1] + '0';
			sc_edt[c][t].tc_inquiry[7]= ' ';
			sc_edt[c][t].tc_inquiry[8]= 'W';
			sc_edt[c][t].tc_inquiry[9]= 'D';
			sc_edt[c][t].tc_inquiry[10]= '7';
			sc_edt[c][t].tc_inquiry[11]= '0';
			sc_edt[c][t].tc_inquiry[12]= '0';
			sc_edt[c][t].tc_inquiry[13]= '0';
			sc_edt[c][t].tc_inquiry[14]= '-';
			sc_edt[c][t].tc_inquiry[15]= 'A';
			sc_edt[c][t].tc_inquiry[16]= 'S';
			sc_edt[c][t].tc_inquiry[17]= 'C';
			sc_edt[c][t].tc_inquiry[18]= ' ';
			sc_edt[c][t].tc_inquiry[19]= ' ';
			sc_edt[c][t].tc_inquiry[20]= ' ';
			sc_edt[c][t].tc_inquiry[21]= ' ';
			sc_edt[c][t].tc_inquiry[22]= ' ';
			sc_edt[c][t].tc_inquiry[23]= ' ';
			sc_edt[c][t].tc_inquiry[24]= NULL;
		
			TC(c, t).lu_cnt = -1;	/* for debug purpose */
		}
	}
	if (config == FALSE)
	{
		cmn_err(CE_WARN,"SCSI: Host Adapter %d has no equipped target controllers.\n",c);
		return;
	}
	for (t = 0; t < MAX_TCS; t++)	/* determine LU equippage */
	{
		TC(c, t).lu_cnt = NULL;
		if ((sc_edt[c][t].tc_equip) && (t != SC(c).ha_id))
		{
			SC(c).tc_cnt++;
			for (lu = 0; lu < MAX_LUS; lu++)
				scsi_cklu (c, t, lu);

			/* make sure at least one lu is configured */
			if (sc_edt[c][t].n_lus == NULL)
			{
			     cmn_err(CE_WARN,"SCSI: HA %d TC %d has no LUs.\n", c, t);
			     sc_edt[c][t].tc_equip = NULL;
			     SC(c).tc_cnt--;
			}
			else
				TC(c, t).lu_cnt = sc_edt[c][t].n_lus;
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
			comp_code = scsi_cmd (c, t, 0, &inq_cdb, SCS_SZ, 
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
			else if (comp_code == SDI_TIME)
			{
				cmn_err(CE_WARN,"SCSI: SCSI bus %d not functional.",c);
				return;
			}	
		}
	}


#ifdef DEBUG
	DPR(8,0)("\n\tSCSI EDT FOR HA %d :\n",c);
	for (t = 0; t < MAX_TCS; t++)
	{
		if ((sc_edt[c][t].tc_equip) && (t != SC(c).ha_id))
		{
		      DPR(8,0)("\tTC ID%d INQ=\"%s\"\n",t,
					sc_edt[c][t].tc_inquiry);
		      DPR(8,0)("\t\tLUs ");
		      for (lu = 0; lu < MAX_LUS; lu++)
				if (sc_edt[c][t].lu_id[lu])
					DPR(8,0)("ID%d ",lu);
		      DPR(8,0)("\n");
		}
	}
#endif

	return ;
}

/*============================================================================
** Function name: scsi_cklu()
** Description:
**	This function determines if the given LU is equipped. First An Inquiry
**	cammand is send if it passes the LU is marked equipped. For disks
**	a test unit ready is also send since some vendors don't return the
**	correct inquiry data to indicate that the addressed LU is not
**	present.
*/

static int
scsi_cklu (c, t, l)
int	c;		/* HA Controller 	*/
int	t;		/* target controller	*/
int	l;		/* logical unit		*/
{
	struct scs	inq_cdb;	/* inquiry cdb		*/
	struct scs	tur_cdb;	/* test unit ready cdb	*/
	int		comp_code;	/* SDI completion code 	*/

#ifdef DEBUG
	DPR(8,3)("scsi_cklu: c=%d t=%d l=%d\n", c, t, l);
#endif
	inq_cdb.ss_op = SS_INQUIR;	/* inquiry cdb		*/
	inq_cdb.ss_lun = l;
	inq_cdb.ss_addr = NULL;
	inq_cdb.ss_addr1 = NULL;
	inq_cdb.ss_len = IDENT_SZ;
	inq_cdb.ss_cont = NULL;

	comp_code = scsi_cmd (c, t, l, &inq_cdb, SCS_SZ,
					&inq_data, IDENT_SZ, B_READ);
	if (comp_code == SDI_ASW)
	{
		if ((inq_data.id_type == RANDOM) && (!(inq_data.id_rmb)))
		{
			tur_cdb.ss_op = SS_TEST; /* test unit ready cdb	*/
			tur_cdb.ss_lun = l;
			tur_cdb.ss_addr = NULL;
			tur_cdb.ss_addr1 = NULL;
			tur_cdb.ss_len = NULL;
			tur_cdb.ss_cont = NULL;

			comp_code = scsi_cmd (c, t, l, &tur_cdb, SCS_SZ, 
						NULL, NULL, B_READ);

			/* send it again first one clears unit attention */

			comp_code = scsi_cmd (c, t, l, &tur_cdb, SCS_SZ, 
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
}


/*============================================================================
** Function name: scsi_cmd()
** Description:
**	This function builds and sends an SCB associated scsi command. 
*/

static int
scsi_cmd (c, t, l, cdb_p, cdbsz, data_p, datasz, rw_flag)
int	c;		/* HA Controller 	*/
int	t;		/* target controller	*/
int	l;		/* logical unit		*/
caddr_t	cdb_p;		/* pointer to cdb 	*/
long	cdbsz;		/* size of cdb		*/
caddr_t	data_p;		/* command data area 	*/
long	datasz;		/* size of buffer	*/
int	rw_flag;	/* read write flag	*/
{
	struct sb	*psb;
	buf_t		*bp;
	struct proc	*procp;
	int		retcode;

#ifdef DEBUG
	DPR(8,3)("scsi_cmd: c=%d t=%d l=%d\n", c, t, l);
#endif

	bp = &sc_bp;
	while (bp->b_flags & B_BUSY)
		sleep ((caddr_t)bp, PZERO);

	bp->b_flags = B_BUSY;

	psb = (struct sb *) &scsi_scb;
	psb->sb_type = SCB_TYPE;
	psb->SCB.sc_int = scsi_cint;
	psb->SCB.sc_cmdpt = cdb_p;
	psb->SCB.sc_cmdsz = cdbsz;
	psb->SCB.sc_datapt = data_p;
	psb->SCB.sc_datasz = datasz;
	psb->SCB.sc_wd = (long) bp;
	psb->SCB.sc_time = (30 * ONE_SECOND);
	psb->SCB.sc_dev.sa_lun = l;
	psb->SCB.sc_dev.sa_fill = ((c << 3) | t);

	drv_getparm (UPROCP, (unsigned long) &procp);
	sdi_translate (psb, rw_flag, procp);

	psb->SCB.sc_comp_code = SDI_PROGRES;
	SC(c).jobs++;
	SC(c).tc[t].lu[l].jobs.t.normal++;
	add_job (psb, c, t, l);
	send_job (c, t, l);

	if ( SC(c).state & C_INIT_TIME )
	{
		if ((scsi_wait_for_comp(c,THREE_MILL_U_SEC)) == FAIL)
		{
			cmn_err(CE_WARN,"SCSI: HA %d, TC %d, LU %d job timeout.",c,t,l);
			SC(c).jobs--;
			SC(c).tc[t].lu[l].jobs.t.normal--;
			psb->SCB.sc_comp_code = SDI_TIME;

		}
	}
	else
	{
		while (bp->b_flags & B_BUSY)
			sleep ((caddr_t)bp, PZERO);
	}
	retcode = psb->SCB.sc_comp_code;
	scsi_scb.sb2.sb.SCB.sc_comp_code = SDI_NOALLOC;

	return (retcode);
}

/*============================================================================
** Function Name: scsi_wait_for_comp()
** Description:
**	This routine is used to wait for a completion from the host 
**	adapter. This routine poll the stataus register on the HA every
**	milli-second. After the interrupt is seen, the HA's interrupt 
**	service routine is manually called. This routine is used primarily
**	at init time before the sleeping is legal.
** NOTE:	
	This routine allow for no  concurrency and as such, should 
**	be used selectivly.
*/
scsi_wait_for_comp(c,time)
int	c;		/* HA controller */
int	time;		/* microseconds to wait for completion */
{
	while (time > 0)
	{
		if ((RD_HA_STATUS(c) & INT_FLAG))
			{
				scsiintr(scsi_ad[c].ha_vect);
				return (PASS);
			}
		drv_usecwait(1000);	/* wait 1 milli-seconds */
		time -= 1000;
	}
	return(FAIL);	
}

/*============================================================================
** Function name: scsi_cint()
** Description:
**	This is the interrupt handler for the scsi_cmd() function.
*/

static void
scsi_cint (psb)
struct sb	*psb;
{
	buf_t	*bp;

#ifdef DEBUG
	DPR(8,3)("scsi_cint ");
	DPR(8,3)("scsi_cint psb=%x ",psb);
#endif
	bp =  (buf_t *) psb->SCB.sc_wd;
	bp->b_flags &= ~B_BUSY;
	wakeup ((caddr_t)bp);

}


/*============================================================================
** Function name: ha_ready()
** Description:
**	This function checks to see if the CMD port of the HA board
**	is ready to take in another command.
*/

ha_ready(c)
int	c;	/* HA Controller 	*/
{
	int		i;
	unsigned char ha_status;

	for (i = 0; i < 3000; i++)
	{
		ha_status = inb(HA_STATUS(c));
		if ((ha_status & 0xF0) & CMD_READY)
			return (PASS);
	}
	cmn_err(CE_WARN,"SCSI: HA %d port hung st = 0x%x ",c,ha_status);
	return (FAIL);
}

/*============================================================================
** Function name: ha_cmd()
** Description:
**	This function writes a command to the HA command register.
*/

ha_cmd(c, cmd)
int		c;	/* HA Controller 	*/
unsigned char	cmd;
{
	int	i;

	for (i=0; i<3000;i++)
	{
#ifdef DEBUG
		DPR(8,5)("writing %x to command register for %xth time.\n", cmd, i);
#endif
		outb(CMD_REG(c), cmd);
		if (ha_ready(c) == FAIL)
			return;

		if (!(RD_HA_STATUS(c) & CMD_REJECT))
			return;
	}
#ifdef DEBUG
	DPR(8,0)("SCSI: HA %d CMD REJECTED ",c);
#endif
}

/*============================================================================
** Function name: scsi_getq()
** Description:
**	This function returns the next available request queue entry.
*/

static int
scsi_getq(c)
int	c;
{
	int i;

	for (i=0;i<NUM_OGMB;i++)
		if (SC(c).rq[i].status != RQ_BUSY)
			break;
	return (i);
}


char rd_buf[25];	/* returned data buffer */
int
rw_exp (c, opcode, pa)
int	c;
int	opcode;
char	*pa;	/* user data pointer */
{
	int 	rqn,i;
	int 	padr=NULL;
	unsigned char cmd;
	
	for (i=0;i<25;i++)
		rd_buf[i] = NULL;

	if (opcode == WR_EXP)
	{
		if (copyin (pa, rd_buf, 25) != SUCCESS)
		{
			return(EFAULT);
		}
	}
	rexp.opcode = (opcode == RD_EXP) ? RD_EXECUT_PARAMS : SET_EXECUT_PARAMS;

	rexp.bytes[0] = NULL;	/*reserved byte */
	rexp.bytes[1] = NULL;
	rexp.bytes[2] = NULL;
	rexp.bytes[3] = 25;	/* xfer size */

	if ((padr = vtop(rd_buf, NULL)) == NULL)
		cmn_err (CE_PANIC,"SCSI: Bad address returned by VTOP.\n");

	rexp.bytes[4] = msbyte(padr);	/* cmd data area to write to */
	rexp.bytes[5] = mdbyte(padr);
	rexp.bytes[6] = lsbyte(padr);
	rexp.bytes[7] = NULL;
	rexp.bytes[8] = NULL;
	rexp.bytes[9] = NULL;
	rexp.bytes[10] = NULL;
	rexp.bytes[11] = NULL;
	rexp.bytes[12] = NULL;
	rexp.bytes[13] = NULL;
	rexp.status = NULL;

	/* set up the request Q entry */

	rqn = scsi_getq (c);
	if ((SC(c).rq[rqn].pdp = sdi_swap24(vtop((caddr_t)&rexp, NULL))) == NULL)
		cmn_err (CE_PANIC,"SCSI: Bad address returned by VTOP.\n");

	SC(c).rq[rqn].status = RQ_BUSY;
	cmd = (RD_MB | (rqn & 0x3F));	/* send cmd to HA board */
	ha_cmd(c, cmd);

	if (scsi_wait_for_comp(c,TEN_MILL_U_SEC) == PASS)
	{
		if (copyout(rd_buf, pa, 25) != SUCCESS)
		{
			return(EFAULT);
		}
	}	
	else	/* timeout */
	{
		cmn_err (CE_WARN,"SCSI: Read Parameters command timed out.\n");
		return (EIO);
	}

	return(0) ;
}
void
read_ha_version (c)
int	c;
{
	int 	rqn,i;
	unsigned char cmd;


	rd_ha_vers.opcode = READ_HA_VER; /* read firmware revision opcode */

	for (i=0; i < 14; i++)
		rd_ha_vers.bytes[i] = NULL;

	rd_ha_vers.status = NULL;

	/* set up the request Q entry */

	rqn = scsi_getq (c);
	if ((SC(c).rq[rqn].pdp = sdi_swap24(vtop((caddr_t)&rd_ha_vers, NULL))) == NULL)
		cmn_err (CE_PANIC,"SCSI: Bad address returned by VTOP.\n");

	SC(c).rq[rqn].status = RQ_BUSY;
	cmd = (RD_MB | (rqn & 0x3F));	/* send cmd to HA board */
	ha_cmd(c, cmd);


	if (scsi_wait_for_comp(c,TEN_MILL_U_SEC) == FAIL)
	{
		cmn_err (CE_WARN,"SCSI: Read HA Version command timed out.\n");
	}

}

static void
scsi_dmainit ()
{
	register int i, oip;
	
#ifdef DEBUG
	DPR(8,1)("scsi_dmainit:\n");
#endif
	oip = spl5 ();
	
	dma_free_list = dma_pool;
	for (i = 0; i < (NUM_DMA_LISTS - 1); ++i)
	{
		dma_pool[i].next = &dma_pool[i + 1];
	}
	dma_pool[i].next = NULL;	/* last element */

	splx (oip);
	
} /* scsi_dmainit () */


static void
scsi_dmamakelist (sp, vaddr, count, procp)
struct sb2	*sp;
caddr_t	vaddr;
long	count;
struct proc	*procp;
{
	int oip;
	DMA_LIST *lp;
	register int i,j;
	register DMA_PAIR *pp;
	register long tmpcount, tmpphysaddr;
	char	*kluge_addr;	
#ifdef DEBUG
	DPR(8,5)("scsi_dmamakelist:\n");
#endif
	oip = spl5 ();
	while (dma_free_list == NULL)
	{
#ifdef DEBUG
		DPR(8,5)("scsi_dmamakelist: sleeping on dma free list\n");
#endif
		sleep ((caddr_t) &dma_free_list, PZERO);
	}
	lp = dma_free_list;
	dma_free_list = lp->next;
	lp->next = NULL;

	for (i = 0, pp = &lp->pair[0]; (i < NUM_DMA_PAIR_PER_LIST) && count; ++i)
	{
		tmpcount = min (count, Bytes_Til_Page_Boundary (vaddr));
		pp->count[0] = msbyte (tmpcount);
		pp->count[1] = mdbyte (tmpcount);
		pp->count[2] = lsbyte (tmpcount);
		tmpphysaddr = (long) vtop (vaddr, procp);

		
		if (tmpphysaddr+tmpcount > scsi_tot_mem ) 
			cmn_err (CE_PANIC, "SCSI: VTOP translated out of memory range. 0x%X.\n",tmpphysaddr+tmpcount);

		/* This is a kludge to handle a WD7000-ASC HA bug. They cannot DMA
		 * out of the very last block of system memory without accessing 
		 * past the end of memory(which would cause the system to crash!).
		 * If the kernel ever asks us to tell the HA to read out of the 
		 * last page of memory, we must move the data to a safer spot.
		 */
		
		if ((sp->sp_read == FALSE)
		&& (tmpphysaddr+tmpcount >= (scsi_tot_mem - NBPP))) {
			char *countp;
			countp = scsi_tmp_buff;
			kluge_addr =  vaddr;
			/* copy data to temp buffer */
			j = 0;
			while (j < tmpcount)
			{
				*countp =  *kluge_addr;
				countp++;
				kluge_addr++;
				j++;
			}
			tmpphysaddr = (long) vtop(scsi_tmp_buff, NULL);
		} 

		pp->physaddr[0] = msbyte (tmpphysaddr);
		pp->physaddr[1] = mdbyte (tmpphysaddr);
		pp->physaddr[2] = lsbyte (tmpphysaddr);
		count -= tmpcount;
		vaddr += tmpcount;
		++pp;
#ifdef	DEBUG
		DPR(8,5)("%X %X;", tmpcount, tmpphysaddr);
#endif
	}
	if (count != 0)
	{
		cmn_err (CE_WARN,"SCSI: job too big for dma list.\n");
		return;
	}
	sp->sp_isz = i * sizeof (DMA_PAIR);
	sp->sp_vip = (long) lp;
#ifdef	DEBUG
	DPR (8,5)("\nsp->sp_isz=%X, sp->sp_vip=%X\n", sp->sp_isz, sp->sp_vip);
#endif
	
	splx (oip);
	
} /* scsi_dmamakelist () */


static void
scsi_dmafreelist (lp)
DMA_LIST *lp;
{
	register int oip;
	
#ifdef DEBUG
	DPR(8,5)("scsi_dmafreelist:\n");
#endif
	oip = spl5 ();
	
	lp->next = dma_free_list;
	dma_free_list = lp;
	
	splx (oip);
	
	if (lp->next == NULL)
	{
		wakeup ((caddr_t) &dma_free_list);
	}
} /* scsi_dmafreelist () */
