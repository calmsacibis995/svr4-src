/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/mb2tsm.c	1.3.1.1"

#ifndef lint
static char mb2scopyright[] = "Copyright 1988, 1989 Intel Corporation 462653";
#endif

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/strstat.h"
#include "sys/mps.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/mb2taiusr.h"
#include "sys/mb2stai.h"
#include "sys/ddi.h"



static int mb2sopen(), mb2srput(), mb2srsrv(), mb2swput(), mb2sclose();

static struct module_info 
	mb2srwinfo = {0, "mb2s", 0, INFPSZ, 0, 0};

static struct qinit
	mb2swinit = {mb2swput, NULL,  NULL,  NULL, NULL,
			&mb2srwinfo, NULL};
static struct qinit
	mb2srinit = {mb2srput, mb2srsrv,  mb2sopen,  mb2sclose, NULL,
			&mb2srwinfo, NULL};

struct streamtab mb2sinfo = {&mb2srinit, &mb2swinit, NULL, NULL};

/*
 *  the following is a SVR4.0 requirement
 */
int mb2sdevflag = 0;

static void
mb2s_init_modep (mep)
mb2_modep *mep;
{
	mep->m_msgq.mq_head = mep->m_msgq.mq_tail =  NULL;
	mep->m_curref = 0;
	mep->m_ref_counter = 1;	/* 0 is an illegal value for this counter */
	mep->m_iocmptr = NULL;
	mep->m_type = 0;
	mep->m_flag = 0;
}


static int
mb2sopen(rdq, dev, flag, sflag, credp)
queue_t *rdq;
dev_t	 *dev;	
int flag, sflag;
struct cred *credp;
{
	mb2_modep *mep;

	DEBUGS(DBG_CALL,("mb2sopen(dev = %d)\n", dev));
	if (flag || (sflag != MODOPEN)) {
		DEBUGS(DBG_ERR, ("mb2sopen: flag (%d) sflag (%d)\n", flag, sflag));
		return (EINVAL);
	}
	/*
	 * initialize the message queue for the device
	 */
	mep = &mb2s_modep[getminor(*dev)];
	mb2s_init_modep (mep);	
	mep->m_rdq = rdq;
	rdq->q_ptr = (WR(rdq))->q_ptr = (char *)mep;
	rdq->q_hiwat = mb2t_defrcv_hiwat;
	rdq->q_lowat = mb2t_defrcv_lowat;
	DEBUGS(DBG_CALL,("mb2sopen => 0\n" ));
	return(0);
}

/* ARGSUSED */
static int
mb2sclose(rd_q, flag)
queue_t *rd_q;
int flag;
{
	mb2_modep *mep;

	DEBUGS(DBG_CALL,("mb2sclose()\n"));
	mep = (struct mb2_modep *)rd_q->q_ptr; 
	if (mep->m_iocmptr != NULL) 
		freemsg (mep->m_iocmptr);
	mb2s_init_modep (mep);
	rd_q->q_ptr = (WR(rd_q))->q_ptr = (caddr_t) NULL;
	DEBUGS(DBG_CALL,("mb2sclose => NULL\n"));
	return (0);
}


static int
mb2swput(wr_q, mptr)
queue_t *wr_q;
register mblk_t *mptr;
{
	register mb2_modep *mep;
	register ulong type;
	struct mb2_recv_req *recv_req;
 	struct iocblk *iocbp;

	DEBUGS(DBG_CALL,("mb2swput()\n"));
	mep = (mb2_modep *)wr_q->q_ptr;
	switch(mptr->b_datap->db_type) {
	case M_FLUSH:
		if(*mptr->b_rptr & FLUSHR) {
			flushq(RD(wr_q), FLUSHDATA);
			*mptr->b_rptr &= ~FLUSHW;
			qreply(wr_q, mptr);
		} else
			freemsg(mptr);
		break;
	case M_PROTO:
		DEBUGS(DBG_FULL,("mb2swput: message is an M_PROTO\n"));
		type = (ulong)(((union primitives *)mptr->b_rptr)->type);
		switch (type) {
		default:
			/* just pass it down stream */
			break;
		case MB2_RSVP_REQ:
		case MB2_FRAG_REQ:
			mb2s_do_dnstr_msg (mep, mptr, type);
			break;
		}
		putnext (wr_q, mptr);
		break;
	case M_PCPROTO:
		DEBUGS(DBG_FULL,("mb2swput: message is an M_PCPROTO\n"));
		type = (ulong)(((union primitives *)mptr->b_rptr)->type);
		if (type == MB2_OPTMGMT_REQ)
			mb2s_do_optmgmt (mep, mptr);
		putnext (wr_q, mptr);
		break;
	case M_IOCTL:
		DEBUGS(DBG_FULL,("mb2swput: message is an M_IOCTL \n"));
 		iocbp = (struct iocblk *)mptr->b_rptr;

		switch (iocbp->ioc_cmd) {
		default:
			putnext(wr_q, mptr);
			break;
		case MB2_SYNC_CMD:
			/*
			 * this is introduced to clear the current state.
			 * and to flush the stream head read queue
			 */
			mptr->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			/* reply to the ioctl first */
			qreply(wr_q, mptr);
			if (mep->m_iocmptr != NULL)
				freemsg (mep->m_iocmptr);
			mep->m_iocmptr = NULL;
			mep->m_flag &= ~MB2_WAIT_MSG;
			mep->m_curref = 0;
			mb2s_send_flush (RD(wr_q));
			return(0);
			 
		case MB2_RECV_CMD:
			if (mptr->b_cont == NULL) {
				DEBUGS(DBG_FULL,("mb2swput: b_cont == NULL\n"));
				mptr->b_datap->db_type = M_IOCNAK;
				iocbp->ioc_error = EINVAL;
				qreply (wr_q, mptr);
				return (0);
			}
			recv_req = (struct mb2_recv_req *)mptr->b_cont->b_rptr;
			if (recv_req->PRIM_type != MB2_RECV_REQ) {
				DEBUGS(DBG_FULL,("mb2swput: PRIM_type != RECV_REQ\n"));
				mptr->b_datap->db_type = M_IOCNAK;
				iocbp->ioc_error = EPROTO;
				qreply (wr_q, mptr);
				return (0);
			}
			/*
			 * save the ioc block  and enable the read service que
			 */
			if (mep->m_flag & MB2_WAIT_MSG) {
				DEBUGS(DBG_FULL,("mb2swput: Wait msg set\n"));
				if (mep->m_iocmptr == NULL) 
					cmn_err (CE_PANIC, "mb2swput: invalid state");
				freemsg (mep->m_iocmptr);
				mep->m_iocmptr = NULL;
				mep->m_flag &= ~MB2_WAIT_MSG;
			}
			mep->m_flag |= MB2_WAIT_MSG;
			mep->m_iocmptr = mptr;
			qenable (RD(wr_q));
		}
		break;
	default:
		DEBUGS(DBG_FULL,("mb2swput: message is unknow type (%d)\n", mptr->b_datap->db_type));
		putnext (wr_q, mptr);
		break;
	}
	DEBUGS(DBG_CALL,("mb2swput() => 0\n"));
	return (0);
}

static void
mb2s_send_flush (rd_q)
queue_t *rd_q;
{
	mblk_t *mptr;
	DEBUGS(DBG_CALL,("mb2s_send_flush() \n"));
	/*
	 * if there were any messages sent upstream
	 * flush them.
	 */
	if ((mptr = allocb (1, BPRI_HI)) == NULL) {
		cmn_err (CE_WARN, "mb2s_send_flush: cant alloc mptr");
		/*
		 * note that we cant send a flush. Theres nothing we can
		 * do but just return.
		 */
		return;
	}
	mptr->b_datap->db_type = M_FLUSH;
	*mptr->b_rptr = FLUSHR;
	mptr->b_wptr++;
	putnext (rd_q, mptr);
	DEBUGS(DBG_CALL,("mb2s_send_flush() ==> 0\n"));
	return;
}
 

static void
mb2s_do_dnstr_msg (mep, mptr, type)
mb2_modep *mep;		/* pointer to the modules private structure */
mblk_t *mptr;		/* pointer to the message block	*/
ulong type;		/* type of message	*/
{
	struct mb2_rsvp_req *rsvp_req;
	struct mb2_frag_req *frag_req;

	DEBUGS(DBG_CALL,("mb2s_do_dnstr_msg: type (%d)\n", type));
	mep->m_type = type;
	/*
	 * we assign our own reference value here.
	 */
	if (type == MB2_RSVP_REQ) {
		rsvp_req = (struct mb2_rsvp_req *) mptr->b_rptr;
		mep->m_curref = mb2s_allocref (mep);
		rsvp_req->REF_value = mep->m_curref;
	} else {
		frag_req = (struct mb2_frag_req *) mptr->b_rptr;
		mep->m_curref = mb2s_allocref (mep);
		frag_req->REF_value = mep->m_curref;
	}
	DEBUGS(DBG_CALL,("mb2s_do_dnstr_msg ==> NULL\n"));
	return;
}

static void
mb2s_do_optmgmt(mep, mptr)
mb2_modep *mep;			/* pointer to an endpoint structure */
register mblk_t *mptr;		/* pointer to a message block */
{
	struct mb2_optmgmt_req *optmgmt_req;

	DEBUGS(DBG_CALL,("mb2s_do_optmgmt()\n"));
	/*
	 * we need to set only the read side flow control values.
	 */

	optmgmt_req = (struct mb2_optmgmt_req *) mptr->b_rptr;
	if (optmgmt_req->RECV_flow != MB2_OPTDEFAULT) {
	       DEBUGS(DBG_FULL,("mb2s_do_optmgmt:recv_flow != opt_default\n"));
		if ((optmgmt_req->RECV_flow > mb2t_max_recv_hiwat) || 
		    (optmgmt_req->RECV_flow <= ((mep->m_rdq))->q_lowat)) {
			return;
		}
		mep->m_rdq->q_hiwat = optmgmt_req->RECV_flow;
	}
	DEBUGS(DBG_CALL,("mb2s_do_optmgmt() ==> 0\n"));
	return;
}

static int
mb2srput(rd_q, mptr)
queue_t *rd_q;		/* pointer to the read queue structure */
register mblk_t *mptr;	/* pointer to the message block */
{
	mb2_modep *mep;
	ulong type;

	DEBUGS(DBG_CALL,("mb2srput()\n"));
	mep = (mb2_modep *)rd_q->q_ptr;
	switch(mptr->b_datap->db_type) {
	case M_PROTO:
		DEBUGS(DBG_FULL,("mb2srput: message is M_PROTO \n"));
		type = (ulong)(((union primitives *)mptr->b_rptr)->type);
		mb2s_do_upstr_msg (rd_q, mep, mptr, type); 
		break;
	case M_PCPROTO:
		DEBUGS(DBG_FULL,("mb2srput: message is M_PCPROTO\n"));
		type = (ulong)(((union primitives *)mptr->b_rptr)->type);
		DEBUGS(DBG_FULL,("mb2srput: type = %d size = %d\n", type,
					mptr->b_wptr - mptr->b_rptr));
		putnext (rd_q, mptr);
		break;
	default:
		DEBUGS(DBG_FULL,("mb2srput: default type (%d)\n", mptr->b_datap->db_type));
		/*
	 	 * just pass the message through
	 	 */
		putnext (rd_q, mptr);
		break;
	}
	DEBUGS(DBG_CALL,("mb2srput() => 0\n"));
	return (0);
}

static int
mb2srsrv(rd_q)
queue_t	*rd_q;
{
	mblk_t	*mptr;
	mb2_modep *mep;
	struct mb2_recv_req *recv_req;
	struct mb2_msg_ind *msg_ind;
	struct iocblk *iocbp;

	DEBUGS(DBG_CALL,("mb2srsrv ()\n"));
	mep = (mb2_modep *)rd_q->q_ptr;
	if (mep->m_flag & MB2_WAIT_MSG) {
		if (mep->m_iocmptr == NULL)
			cmn_err (CE_PANIC, "mb2srsrv: Invalid state");
	}
	while  ((mptr = getq(rd_q)) != NULL) {
		if (!canput(rd_q->q_next))  {
			DEBUGS(DBG_FULL,("mb2srsrv () !canput\n"));
			putbq (rd_q, mptr);
			break;
		}
		if (!(mep->m_flag & MB2_WAIT_MSG)) {
			DEBUGS(DBG_FULL,("mb2srsrv () !MB2_WAIT_MSG\n"));
			putbq (rd_q, mptr);
			break;
		}
 		iocbp = (struct iocblk *)mep->m_iocmptr->b_rptr;
		recv_req = (struct mb2_recv_req *)mep->m_iocmptr->b_cont->b_rptr;
		msg_ind = (struct mb2_msg_ind *) mptr->b_rptr;
		mep->m_flag &= ~MB2_WAIT_MSG;
		if (recv_req->CTRL_length < msg_ind->CTRL_length) {
			putbq (rd_q, mptr);
			mep->m_iocmptr->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_error = EINVAL;
			qreply (WR(rd_q), mep->m_iocmptr);
			mep->m_iocmptr = NULL;
			break;
		}
		mep->m_iocmptr->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(WR(rd_q), mep->m_iocmptr);
		mep->m_iocmptr = NULL;
		putnext (rd_q, mptr);
	}
	DEBUGS(DBG_CALL,("mb2trsrv () ==> 0\n"));
	return(0);
}

static void
mb2s_do_upstr_msg (rd_q, mep, mptr, type)
queue_t *rd_q;		/* pointer to the read queue structure */
mb2_modep *mep;		/* pointer to the private data structure */
mblk_t *mptr;		/* pointer to the message block */
ulong type;		/* type of message */
{
	struct mb2_msg_ind *msg_ind;
	
	DEBUGS(DBG_CALL,("mb2s_do_upstr_msg ()\n"));
	msg_ind = (struct mb2_msg_ind *) mptr->b_rptr;
	switch (type) {
	default:
		if (mb2s_match_type (mep->m_type, type) != 0) {
			cmn_err (CE_WARN, "error in mb2s: illegal message received");
			freemsg (mptr);
			return;
		}
		if (mep->m_curref != msg_ind->REF_value) {
			/* just discard the message */
			freemsg (mptr);
		} else {
			mep->m_curref = 0;
			putnext (rd_q, mptr);
		}
		break;

	case MB2_NTRAN_MSG:
	case MB2_REQ_MSG:
	case MB2_REQFRAG_MSG:
	case MB2_BRDCST_MSG:
		putq (rd_q, mptr);
		qenable (rd_q);
		break;
	}
	DEBUGS(DBG_CALL,("mb2s_do_upstr_msg () ==> 0\n"));
	return;
}

static int 
mb2s_match_type (curtype, intype)
ulong curtype;
ulong intype;
{
	if (curtype == MB2_RSVP_REQ) {
		if ((intype == MB2_RESP_MSG) || (intype == MB2_CANCEL_MSG) ||
			(intype == MB2_STATUS_MSG))
			return (0);
	}
	if (curtype == MB2_FRAG_REQ) {
		if ((intype == MB2_FRAGRES_MSG) || (intype == MB2_STATUS_MSG))
			return (0);
	}
	return (-1);
}



static ulong
mb2s_allocref (mep)
mb2_modep *mep;		/* pointer to the private data structure */
{
	ulong ret;
	int s;
	/*
	 * get a reference also note that the reference will wrap around
	 * eventually we dont free references.
	 */
	s = SPL ();
	ret = mep->m_ref_counter;
	if ((mep->m_ref_counter++) == 0)
		/*
		 * we dont want to use zero 
		 */
		mep->m_ref_counter++;
	splx (s);
	return (ret);
}
