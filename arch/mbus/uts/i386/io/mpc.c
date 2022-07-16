/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/mpc.c	1.3.4.2"

#ifndef lint
static char impc_copyright[] = "Copyright 1986, 1987, 1989, 1990 Intel Corp. 460944";
#endif /* lint */

/*
 *
 *  Multibus II Message Device Driver
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/dma.h"
#include "sys/i82258.h"
#include "sys/mpc.h"
#include "sys/ics.h"
#include "sys/mps.h"
#include "sys/cmn_err.h"
#include "sys/inline.h"
#include "sys/cred.h"
#include "sys/ddi.h"

/* external variables which are configured in */
extern int fp_intr;
extern unsigned long impc_base;
extern int freemsg;
extern int impc_fs_enabled;
extern int impc_retries;


extern int dbgintr();

extern	unsigned char	pc16;		/* declared in ics.c */
extern	unsigned char	pc16_status;
extern	unsigned char	pc16_config;
extern unsigned char ics_myslotid(); /* PSB slot id set by icsinit */

/*
 * globals 
 */
tinfo_t *impc_si_head, *impc_si_tail, *impc_so_head, *impc_so_tail;
struct dma_cb *impc_sic_cb, *impc_soc_cb;

#define GET_DMA_REQRBUF(dmacb)	dmacb->reqrbufs
#define GET_DMA_TARGBUF(dmacb)	dmacb->targbufs

/* define DEBUG switch */
#ifdef DEBUG
#define		MPC_BREAK		2
#define		MPC_TRACE		4
#define		MPC_SHOW_MSG	8
int impcdb= 0x0000;
#endif


/*
 * setup the dma control block for dma transfer
 */
static void
impc_init_dmacb (dmacb)
struct dma_cb *dmacb;
{
	dmacb->targbufs = NULL;
	dmacb->reqrbufs = NULL;
	dmacb->command = DMA_CMD_READ;  		  /* not used */   
	dmacb->cycles = 0;
	dmacb->targ_type = DMA_TYPE_IO;    /* Memory/IO */
	dmacb->reqr_type = DMA_TYPE_MEM;   /* Memory/IO */
	dmacb->targ_step = DMA_STEP_INC;   /* Inc/Dec/Hold */
	dmacb->reqr_step = DMA_STEP_INC;   /* Inc/Dec/Hold */
	dmacb->trans_type = 0;  	   /* not used */
	dmacb->targ_path = DMA_PATH_16;   /* 8/16/32 */
	dmacb->reqr_path = DMA_PATH_16;   /* 8/16/32 */
	dmacb->bufprocess = 0;  	  /* not used */
	dmacb->proc = NULL;
	dmacb->procparms = NULL;

}


/*
 *
 *	impcinit initializes a message device and its corresponding
 *	data structures.
 *
 */
impcinit()
{
	int i;
	unsigned char tcfg;
	struct ics_struct ics;
	unsigned short reg;

	icsinit();	/* initialize any ICS registers */
	dmainit();	/* initialize the DMAC */
	/*
	 * Static allocation of control blocks 1 per DMA channel.
	 */
	if ((impc_soc_cb = dma_get_cb (DMA_NOSLEEP)) != NULL) {
		impc_init_dmacb (impc_soc_cb);
	} else
		cmn_err (CE_PANIC, "Cannot get dma control blocks");
	if ((impc_sic_cb = dma_get_cb (DMA_NOSLEEP)) != NULL) {
		impc_init_dmacb (impc_sic_cb);
	} else
		cmn_err (CE_PANIC, "Cannot get dma control blocks");
	impc_si_head = NULL;
	impc_si_tail = NULL;
	impc_so_head = NULL;
	impc_so_tail = NULL;
	/* reset message device */
	outb(impc_base + MRST, MD_CLEAR);

	/* wait for reset complete */
	for ( i=0; i < MD_DELAY; i++ ) {
		if ( (inb(impc_base + MSTAT) & MD_INIT) == MD_RESET )
			break;
		(void)drv_usecwait(10);
	}
	if ( i == MD_DELAY )
		cmn_err(CE_PANIC,"Message device Not found - failed reset.\n");

	/* program device configuration */
	outb(impc_base + MCON, MSG_CONF);

	/* read back configuration to see if chip accepted it  */
	tcfg= inb(impc_base + MCON);
	if(tcfg != MSG_CONF)
		cmn_err(CE_PANIC,"Message Device Type not supported\n");

	/*
	 * use the cardslot id for message id
	 */
	outb(impc_base + MCTL, SET_INTR); /* MD interrupt mask */
	outb(impc_base + MID, ics_myslotid()); /* system wide id */

	/* wait for MD to finish initiation */
	for ( i=0; i < MD_DELAY; i++ ) {
		if ( (inb(impc_base + MSTAT) & MD_INIT) == MD_INIT)
			break;
		(void)drv_usecwait(10);
	}
	if ( i == MD_DELAY ) 
		cmn_err(CE_PANIC, "Message device Not found - failed initialization.\n");
	
	/* initialize the message id in the interconnect space */
	reg = ics_find_rec(ICS_MY_SLOT_ID, ICS_HOST_ID_TYPE);
	ics.slot_id = ICS_MY_SLOT_ID;
	ics.count = 1;
	if(reg != 0xffff) {
		ics.reg_id = reg+4; /* 4 == offset of MID in HIDrec */
		ics.buffer[0] = ics_myslotid();
		ics_rw(ICS_WRITE_ICS, &ics);
	}

}

/*
 *
 *	impcintr is the interrupt handler routine for a message device.
 *
 */

/* ARGSUSED */
impcintr(level)
int level;
{
	int i, s;
	unsigned int temp;
	unsigned mpc_mask;
	unsigned mpc_val;
	tinfo_t *mp; 
	mps_msgbuf_t *mbp;
	unsigned long *cp;
	struct dma_buf *dbp;


	/* service all pending interrupts */
	s = splhi();
	mpc_mask= inb(impc_base + MCTL) & MD_INTMASK; /* save interrupt enable mask */
	outb(impc_base + MCTL, MDISABLE); /* disable all MD interrupts */
	splx(s);
	
	while ( (mpc_val=(inb(impc_base + MSTAT) & mpc_mask)) != 0) {

		/*
		 * is this a solicited input complete interrupt
		 */
	 	if ( (mpc_val & SICMP ) != 0) {
			/*
			 * read port to clear SICMP interrupt bit
			 * in status register
			 */
			temp = (unsigned int)(inb(impc_base + MSICMP) & 0xff);
			temp &= ~RID_MASK;
			do {
				/* get first device message in SIC queue and return to caller */
				mp = impc_si_head;
				if ( mp == NULL ) {
					cmn_err(CE_WARN, "Message Device: spurious SIC interrupt\n");
					break;
				}
				impc_si_head = mp->t_next;
				/* 
				 * stop dma channel in case it is 
				 * still going 
				 */
				dma_stop (MPC_SICHAN);
				dbp = GET_DMA_TARGBUF(impc_sic_cb);
				if ((temp>>MRID_SIZE) != 0) {
					/*
					 * put back the data buffer back
					 * in the chain
					 */
					mp->t_ibuf = mps_dmabuf_join(dbp,mp->t_ibuf);
				} else {
					/* free the data buffer headers */
					mps_free_dmabuf(dbp);
				}
				/* do completion processing */
				mps_msg_comp(mp,(int)temp);
				/*
				 * start the next request now, so
				 * we can overlap data transfer
				 * and interrupt processing
				 */
				if(impc_si_head)
					temp= impc_start(SIC);
				else
					temp=0;

				/* now examine the status from the
				 * data transfer started above
				 * if error occurred, start the next
				 */
			} while (temp);
			continue;
		}
 
		/*
		 * is this a solicited output complete interrupt
		 */
		if ( (mpc_val & SOCMP ) != 0) {
			temp = (unsigned int)inb(impc_base + MSOCMP) & 0xff;
			temp &= ~RID_MASK;
			do {
				/* get first device message in SOC queue
				 * and return to caller
				 */
				mp = impc_so_head;
				if ( mp == NULL ) {
					cmn_err(CE_WARN, "MPC: spurious SOC interrupt\n");
					break;
				}
				impc_so_head = mp->t_next;
				/* 
				 * stop dma channel in case it is 
				 * still going 
				 */
				dma_stop(MPC_SOCHAN);
				dbp = GET_DMA_REQRBUF(impc_soc_cb);
				if ((temp>>MRID_SIZE) != 0) {
					/* put back the data buffer back
					 * in the chain
					 */
					mp->t_obuf = mps_dmabuf_join(dbp,mp->t_obuf);
				} else {
					/* free the data buffer headers */
					mps_free_dmabuf(dbp);
				}
				/* do completion processing, if any */
				mps_msg_comp(mp,(int)temp);
				/*
				 * start the next request now,
				 * so we can overlap data transfer
				 * and interrupt processing
				 */
				if(impc_so_head)
					temp= impc_start(SOC);
				else
					temp=0;

				/* now examine the status from the
				 * data transfer started above
				 * if error occurred, start the next
				 */
			} while (temp);
			continue;
		}

		/* empty the receive FIFO if no completion interrupts
		 * are pending else handle those first
		 */
		if ( (mpc_val & mpc_mask) == RCVNE) {
			if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == (mps_msgbuf_t *)NULL) {
				mpc_mask &= ~RCVNE;
				continue;
			}
			mbp->mb_count = inb(impc_base + MDATA);

			cp = ( unsigned long *)&mbp->mb_data[0];
			for(i=0; i<(MPS_MAXMSGSZ/sizeof(long)); i++ ) {
				*cp++ = inl(impc_base + MDATA);
			}
			/* terminate FIFO read */
			inb(impc_base + MCMD);

			mbp->mb_flags &= ~(MPS_MG_TERR|MPS_MG_DONE|MPS_MG_ESOL); 

			/*
			* use the source of the message as an index into
			* the message interrupt demultiplexing call vector
			* Queue up the mesage in the priority queues
			* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
			*/
			/* Special Hack for message from CSM */
			if(mbp->mb_count == 4 &&
					mps_msg_getsrcmid(mbp) == 0) {
				mps_mk_unsol(mbp, mps_mk_mb2socid(mps_lhid(),MPS_FP_PORT),0,NULL,0);
				cmn_err (CE_NOTE,"received message from csm");
			}
			if(mps_msg_getprotid(mbp) != MPS_MB2_TPDT) {
				/*
				 * do not print warning if message is a broadcast
				 */
				 if ( mps_msg_getmsgtyp(mbp) != MPS_MG_BRDCST )
					cmn_err(CE_WARN,"Unknown protocol message ignored\n");
				mps_free_msgbuf(mbp);
				continue;
			}
			mps_msg_proc(mbp);
		}


	} /* no interrupts are pending */
	
	s = splhi();
	outb(impc_base + MID, MD_EOI); /* end of interrupt */
	outb(impc_base + MCTL, mpc_mask); /* enable original MD interrupts */
	splx(s);
	/* before you return, dispatch any pending messages */
	mps_msg_dispatch();

}

static void
impc_setup_dmabufs(dmacbptr, dbp, que)
struct dma_cb *dmacbptr;
struct dma_buf *dbp;
int que;
{

	int flag;

	flag = dmacbptr->command = (que == SOC) ? DMA_CMD_WRITE : DMA_CMD_READ;
	if (flag == DMA_CMD_READ) {
		dmacbptr->targbufs = dbp;
		dmacbptr->targ_type = DMA_TYPE_MEM;
		dmacbptr->targ_path = DMA_PATH_16;
		dmacbptr->targ_step = DMA_STEP_INC;
		dmacbptr->reqrbufs = NULL;
		dmacbptr->reqr_type = DMA_TYPE_IO;
		dmacbptr->reqr_path = DMA_PATH_16;
		dmacbptr->reqr_step = DMA_STEP_INC;
	} else {
		dmacbptr->targbufs = NULL;
		dmacbptr->targ_type = DMA_TYPE_IO;
		dmacbptr->targ_step = DMA_STEP_INC;
		dmacbptr->targ_path = DMA_PATH_16;
		dmacbptr->reqrbufs = dbp;
		dmacbptr->reqr_type = DMA_TYPE_MEM;
		dmacbptr->reqr_step = DMA_STEP_INC;
		dmacbptr->reqr_path = DMA_PATH_16;
	}
	dmacbptr->cycles = dma_get_best_mode(dmacbptr);
	if (dmacbptr->cycles == D258_BURST_MODE) /* BURST not supported for mpc */
		dmacbptr->cycles = D258_FLYBY_MODE;
}


/*
 * An error has occurred starting or during a solicited transfer. This routine
 * restores the buffer back to its origional state before the transfer occurred.
 * dbp references the buffer descriptor chain involved in the transfer 
 * tp->t_obuf (or tp->t_ibuf) references the remaining buffer descriptor chain 
 * to be sent (or received). (Must be NULL if no remaining buffer portion.)
 *
 */

static struct dma_buf *
impc_fetch_db(tp)
tinfo_t *tp;
{
	struct dma_buf *dbp;
	long len;
	extern struct dma_buf *mps_dmabuf_breakup();
	switch(tp->t_flags&MG_ENTRY) {
	    case MG_FRAG:
	    case MG_RCV:
		dbp = tp->t_ibuf;
		tp->t_ibuf = NULL;
		break;
	    case MG_SDATA:
	    case MG_SRPLY:
		dbp = tp->t_obuf;
		tp->t_obuf = NULL;
		break;
	    case MG_RSVP:
		switch(tp->t_state) {
		    case MG_INITST:
			dbp = tp->t_obuf;
			tp->t_obuf = NULL;
			break;
		    case MG_RS_FR:
			len = mps_msg_getbrlen(tp->t_omsg);
			dbp = tp->t_obuf;
			tp->t_obuf = mps_dmabuf_breakup(tp->t_obuf,len);
			break;
		    case MG_RS_RC:
		    case MG_RS_PRR:
			len = mps_msg_getbrlen(tp->t_imsg);
			dbp = tp->t_ibuf;
			tp->t_ibuf = mps_dmabuf_breakup(tp->t_ibuf,len);
			break;
		}
		break;
	}
	return(dbp);
}

static mps_msgbuf_t *
impc_fetch_mb(tp)
tinfo_t *tp;
{
	return(tp->t_omsg);
}

/*
 *
 *	impc_start start a solicited data transfer
 *
 */

impc_start(que)
int que;
{
	struct dma_cb *dmacbptr;
	struct dma_buf *dbp;
	mps_msgbuf_t *mbp;
	int dma_chan;
	long len;

	if (que == SIC) {
		dbp = impc_fetch_db (impc_si_head);
		mbp = impc_fetch_mb (impc_si_head);
		len = mps_buf_count (dbp);
		dmacbptr = impc_sic_cb;
		impc_setup_dmabufs(dmacbptr, dbp, SIC);
		mps_msg_setbglen (mbp,len); 
		if (dmacbptr->cycles == D258_FLYBY_MODE) 
			mps_msg_setduty(mbp,MPS_MG_DUTY1C);
		else 
			mps_msg_setduty(mbp,MPS_MG_DUTY2C);
		dma_chan = MPC_SICHAN;
	} else {
		dbp = impc_fetch_db (impc_so_head);
		mbp = impc_fetch_mb (impc_so_head);
		len = mps_buf_count (dbp);
		dmacbptr = impc_soc_cb;
		impc_setup_dmabufs(dmacbptr, dbp, SOC);
		mps_msg_setbrlen(mbp,len);
		dma_chan = MPC_SOCHAN;
	}
	if (dma_prog (dmacbptr, dma_chan, DMA_NOSLEEP) == FALSE)
	      cmn_err (CE_PANIC, "Cannot program DMA sol input chan");
	dma_enable (dma_chan);
	mps_msg_setrid(mbp);

	/* set fail-safe counter according configurable var. FS_ENABLED
	 */
	if (impc_fs_enabled)
		mbp->mb_data[MPS_MG_RI] &= ~0x80;
	else
		mbp->mb_data[MPS_MG_RI] |= 0x80;


	return (impc_send(mbp));
	
}

/*
 *
 *	impc_send sends an unsolicited message.
 *
 */


impc_send(mbp)
mps_msgbuf_t *mbp;
{
	int status;
	register int i, j, s;
	register long *cp;
	register char *dp;
	unsigned char emb_data[MPS_MAXMSGSZ];
	long len;

	len=mbp->mb_count;
	len = (len+sizeof(long)-1) / sizeof(long);		/* bytes to dwords */
	s = splhi();
	for (j=0; j<=impc_retries; j++) {
		cp = (long *)mbp->mb_data;
		/* write control message to MD output FIFO */    
		for (i=0; i < len; i++) {
			outl(impc_base + MDATA, *cp++);
		}
	
		/* write to Command Port (MCMD) */
		outb(impc_base + MCMD, 0);
	
		while (((status= inb(impc_base + MSTAT)) & (XMTERR|XMTNF)) == 0)
			;
	
		if (status & XMTERR) {
			/*
		 	 * got an error
		 	 */
#ifdef XDEBUG
				cmn_err(CE_CONT,"impc_send: XMTERR\n");
				mps_msg_showmsg(mbp);
				cmn_err(CE_CONT,"mbp->mb_count: %d\n",mbp->mb_count);
#endif
			/*
		 	 * read out message from error FIFO
		 	 */
			mbp->mb_count = (unsigned short)inb(impc_base + MERR);
			cp = (long *)emb_data;
			for(i=0;i<(mbp->mb_count/sizeof(long));i++)
				*cp++ = inl(impc_base + MERR);
			outl(impc_base + MERR,0); /* indicate that error msg is read */
			mbp->mb_flags |= MPS_MG_TERR;
			/*
			 * we want to retry only if retry expired bit (5th bit)
			 * error message request id (4th byte). If retry expired
			 * bit is not set, then copy the error information from
			 * the message buffer to the temp buffer
			 */
			if (emb_data[3] & 0x10) {
				for (i=0; i<5;i++); /* just a short delay before retry */
			}
			else {
				j = impc_retries;
				dp = (char *)mbp->mb_data;
				for (i=0; i<mbp->mb_count; i++)
					*dp++ = emb_data[i];
			}
			status = -1;
		} else {
			status= 0;
			mbp->mb_flags &= ~MPS_MG_TERR;
			break;
		}
	}
	splx(s);
	return (status);
}

impc_sol_que(tp,que)
tinfo_t *tp;
int que;
{
	struct dma_buf *dbp;
	int status, s;

	tp->t_next = NULL;
	s = splhi();	
	switch(que) {
	    case SOC:
		if(impc_so_head == NULL) {
			impc_so_head = impc_so_tail = tp;
			if ((status=impc_start(SOC)) == -1) {
				impc_so_head = impc_so_tail = NULL;
				dma_stop(MPC_SOCHAN);
				dbp = GET_DMA_REQRBUF(impc_soc_cb);
				tp->t_obuf = mps_dmabuf_join(dbp,tp->t_obuf);
				mps_msg_comp(tp,status);
			}
		} else {
			impc_so_tail->t_next = tp;
			impc_so_tail = tp;
		}
		break;
	    case SIC:
		if(impc_si_head == NULL) {
			impc_si_head = impc_si_tail = tp;
			if ((status=impc_start(SIC)) == -1) {
				impc_si_head = impc_si_tail = NULL;
				dma_stop(MPC_SICHAN);
				dbp = GET_DMA_TARGBUF(impc_sic_cb);
				tp->t_ibuf = mps_dmabuf_join(dbp,tp->t_ibuf);
				mps_msg_comp(tp,status);
			}
		} else {
			impc_si_tail->t_next = tp;
			impc_si_tail = tp;
		}
		break;
	    default:
		cmn_err(CE_PANIC,"bad solicited type in impc_sol_que\n");
		break;
	}
	splx(s);	
}

/*
 * remove a transaction tabel entry from a solicited queue.
 * This is required for handling local/remote cancels.
 * If it succeeds in removing the given tran_tab_entry
 * return 0. -1 otherwise
 * 
 * If at head issue a cancel, otherwise simply remove from queue
 * and pretend as if a cancel was actually done
 */
impc_sol_deque(tp)
tinfo_t *tp;
{
	unsigned char rid;
	register tinfo_t *onep, *twop;
	int s;

	s = splhi();	
	/* first try the sol output queue */
	if( impc_so_head != NULL ) {
		if(impc_so_head == tp) {
			/* request ID is valid only if
			 * a transfer is in progress
			 */
			rid = mps_msg_getrid(tp->t_omsg);
			outb(MSOCAN,rid&RID_MASK);
			while((inb(MSTAT)&SOCMP) != SOCMP)
				;
			splx(s);
			return(0);
		}
		onep = impc_so_head->t_next;
		twop = impc_so_head;
		while(onep != NULL && onep != tp) {
			twop = onep;
			onep = onep->t_next;
		}
		if(onep != NULL) {
			twop->t_next = onep->t_next;
			mps_msg_comp(tp,MD_SCANCL);
			splx(s);	
			return(0);
		}
	}
	if( impc_si_head != NULL ) {
		if(impc_si_head == tp) {
			/* request ID is valid only if
			 * a transfer is in progress
			 */
			rid = mps_msg_getrid(tp->t_omsg);
			outb(MSICAN,rid&RID_MASK);
			while((inb(MSTAT)&SICMP) != SICMP)
				;
			splx(s);
			return(0);
		}
		onep = impc_si_head->t_next;
		twop = impc_si_head;
		while(onep != NULL && onep != tp) {
			twop = onep;
			onep = onep->t_next;
		}
		if(onep != NULL) {
			twop->t_next = onep->t_next;
			mps_msg_comp(tp,MD_SCANCL);
			splx(s);
			return(0);
		}
	}
	splx(s);	
	return(-1);
}

