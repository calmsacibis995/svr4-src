/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989, 1990 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/mb2stai.c	1.3.2.1"

#ifndef lint
static char mb2tcopyright[] = "Copyright 1988, 1989, 1990 Intel Corporation 462652";
#endif

#include "sys/types.h"
#include "sys/param.h"
#include "sys/file.h"
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


static struct module_info 
	mb2twinfo = {mb2t_NUMBER, mb2t_NAME,
		mb2t_STRMIN, INFPSZ, mb2t_HIWAT, mb2t_LOWAT},
	mb2trinfo = {mb2t_NUMBER, mb2t_NAME,
		mb2t_STRMIN, INFPSZ, mb2t_HIWAT, mb2t_LOWAT};


static struct qinit
	mb2twinit = {mb2twput, mb2twsrv,  NULL,  NULL, NULL,
			&mb2twinfo, NULL},
	mb2trinit = {NULL, mb2trsrv,  mb2topen,  mb2tclose, NULL,
			&mb2trinfo, NULL};

struct streamtab mb2tinfo = {&mb2trinit, &mb2twinit, NULL, NULL};

static struct mb2_prim_bind *mb2t_pb_freelist = 0;

/*
 * the following keeps statistics of the no of sends, receives etc..
 * that takes place in this driver
 */
static ulong mb2t_stat [MAX_STAT] = {0};

/*
 * the following is a SVR4.0 requirement
 */
int mb2tdevflag = 0;

/*
 * the following global variable is used if all the mb2t_pbind structures have
 * been filled and there are streams which require a mb2t_pbind structure.
 */

static ulong mb2t_need_pb = 0;


void
mb2tinit()
{
	end_point *endp;
	int i;

	DEBUGS (DBG_CALL,("mb2tinit  started\n"));
	for(endp = &mb2t_endps[0]; endp < &mb2t_endps[mb2t_no_endps]; endp++) {
		/*
		 * Make sure everything starts out clean.
		 */
		mb2t_init_endp (endp);
	}
	mb2t_init_pb ();
	/*
	 * initialize the port table
	 */
	for (i=0; i<mb2t_no_endps; i++)
		mb2t_ports[i] = 0;
	DEBUGS(DBG_CALL,("mb2tinit  ended \n"));
}


static int
mb2topen(rd_q, devp, oflag, sflag, credp)
queue_t *rd_q;
dev_t *devp;
int oflag, sflag;
struct cred *credp;
{
	register end_point *endp;
	register struct stroptions *soptr;
	minor_t min_dev;
	mblk_t *mptr;

	DEBUGS(DBG_CALL,("mb2topen(dev = %d)\n", *devp));
	if(oflag & ~(FREAD|FWRITE|FNDELAY|FEXCL)) {
		DEBUGS(DBG_ERR,("mb2topen EINVAL: oflag = %x\n", oflag));
		return (EINVAL);
	}
	min_dev = getminor(*devp);
	DEBUGS (DBG_FULL, ("mb2topen: min_dev = %d sflag = %d\n", min_dev, 
				sflag));
	if((sflag == CLONEOPEN) && (min_dev == 0)) {
		/*
		 * Find a free endp (i.e., select an available minor
		 * device number).
		 */
		DEBUGS(DBG_FULL,("mb2topen: finding free endp\n"));
		for(min_dev=1; min_dev < mb2t_no_endps; min_dev++)
			if(mb2t_endps[min_dev].e_strstate == S_IDLE)
				break;
		if(min_dev == mb2t_no_endps) 
			return(EAGAIN);
		/*
	 	 *  make up a new device in devp
         	 */
		*devp = makedevice (getmajor(*devp), min_dev);
	}
	else if((sflag != 0) || (min_dev >= mb2t_no_endps)) {
		/*
		 * Some illegal minor min_device number:
		 */
		DEBUGS(DBG_ERR,("mb2topen ECHRNG: sflag = %x, min_dev = %x\n",
				 sflag, min_dev));
		return(ECHRNG);
	}
	endp = &mb2t_endps[min_dev];
	DEBUGS(DBG_FULL,("mb2topen: endp = %x, min_dev = %x\n", endp, min_dev));
	/*
	 * Detect multiple opens on the same STREAM.
	 */
	if(endp->e_strstate != S_IDLE) {
		/*
		 * The STREAM is already completely opened.
		 */
		DEBUGS(DBG_FULL,("mb2topen: stream already opened\n"));
		DEBUGS(DBG_CALL,("mb2topen => %d\n", min_dev));
		return(EBUSY);
	}
	/*
	 * Set the streamhead's configuration options for the
	 * READ queue to be consistent with those used by mb2t.
	 */
	if((mptr = allocb(sizeof(struct stroptions), BPRI_MED)) == NULL) {
		return(ENOSR);
	}
	/*
	 * set up the stream head options
	 */
	mptr->b_datap->db_type = M_SETOPTS;
	soptr = (struct stroptions *) mptr->b_rptr;
	soptr->so_flags   = SO_MINPSZ | SO_MAXPSZ | SO_HIWAT | SO_LOWAT;
	soptr->so_readopt = 0;
	soptr->so_wroff   = 0;
	soptr->so_minpsz  = mb2t_STRMIN;
	soptr->so_maxpsz  = mb2t_STRMAX;
	soptr->so_hiwat   = mb2t_defrcv_hiwat;
	soptr->so_lowat   = mb2t_defrcv_lowat;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct stroptions);
	putnext(rd_q, mptr);
	mb2t_init_endp (endp);
	endp->e_strstate = (unsigned)~S_IDLE;
	endp->e_rdq = rd_q;
	rd_q->q_ptr = (WR(rd_q))->q_ptr = (caddr_t)endp;
	/*
	 * adjust the flow control values for the read and write service queues
	 */
	endp->e_rsvptout = mb2t_def_rsvptout;
	endp->e_fragtout = mb2t_def_fragtout;
	rd_q->q_hiwat = mb2t_defrcv_hiwat;
	rd_q->q_lowat = mb2t_defrcv_lowat;
	(WR(rd_q))->q_hiwat = mb2t_defsnd_hiwat;
	(WR(rd_q))->q_lowat = mb2t_defsnd_lowat;

	DEBUGS(DBG_FULL,("mb2t: Opened minor device #%x\n", min_dev));
	DEBUGS(DBG_CALL,("mb2topen => %d\n", min_dev));
	return (0);
}


static int
mb2tclose(rd_q)
queue_t *rd_q;
{
	register end_point *endp;
	int ret;

	DEBUGS(DBG_CALL,("mb2tclose()\n"));
	endp = (end_point *)rd_q->q_ptr;
	if (endp->e_chan != 0) {
		endp->e_taistate |= T_CLOSING;
		while (qsize(WR(rd_q)) != 0) {
			if (sleep ((caddr_t) &endp->e_taistate, SWRPRI|PCATCH) == 1) {
				break;
			}
		}
		/*
		 * cleanup
		 */
		mb2t_cancel_outtrans (endp);
		/*
		 * note that we need to sleep if some MB II transaction
		 * has not yet been completed by the transport
		 */
		if (mps_close_chan (endp->e_chan) == -1) {
			DEBUGS(DBG_ERR,("mb2tclose => ERROR close channel\n"));
			while (mps_close_chan (endp->e_chan) == -1) {
				ret = sleep ((caddr_t) &endp->e_taistate, SWRPRI|PCATCH);
				if (ret == 1) 
					break;
			}
		}
	}
	mb2t_free_port (mps_mk_mb2soctopid (endp->e_socket));
	mb2t_send_flush (endp);
	mb2t_init_endp (endp);
	rd_q->q_ptr = (WR(rd_q))->q_ptr = (caddr_t)NULL;
	DEBUGS(DBG_CALL,("mb2tclose => NULL\n"));
	return (0);
}


static int
mb2twput(wr_q, mptr)
queue_t *wr_q;
register mblk_t *mptr;
{
	register end_point *endp;
	ulong type;

	DEBUGS(DBG_CALL,("mb2twput()\n"));
	endp = (end_point *)wr_q->q_ptr;
	switch(mptr->b_datap->db_type) {
	case M_FLUSH:
		if(*mptr->b_rptr & FLUSHW) {
			flushq(wr_q, FLUSHDATA);
		}
		if(*mptr->b_rptr & FLUSHR) {
			flushq(RD(wr_q), FLUSHDATA);
			*mptr->b_rptr &= ~FLUSHW;
			qreply(wr_q, mptr);
		}
		else
			freemsg(mptr);
		break;
	case M_PROTO:
	case M_PCPROTO:
		type = (ulong)((union primitives *)mptr->b_rptr)->type;
		if (mptr->b_datap->db_type == M_PROTO) 
			mb2t_send_queue_msg(endp, mptr, type);
		else
			mb2t_do_cntrl_msg (endp, mptr, type);
		break;
	case M_IOCTL:
		mb2t_do_ioctl_msg (wr_q, mptr, endp);
		break;
	default:
		/*
		 * This is an unexpected STREAMS such as M_DATA message:
		 * 	Throw it away.
		 */
		cmn_err (CE_WARN, "mb2twput: Illegal streams message discarding");
		freemsg(mptr);
		break;
	}
	return (0);
}


static void
mb2t_send_flush(endp)
register end_point *endp;
{
	register mblk_t *mptr;

	DEBUGS(DBG_CALL,("mb2t_send_flush()\n"));
	if((mptr = allocb(1, BPRI_HI)) == NULL) {
		return;
	}
	mptr->b_datap->db_type = M_FLUSH;
	*mptr->b_rptr = FLUSHRW;
	mptr->b_wptr++;
	putnext(endp->e_rdq, mptr);
	DEBUGS(DBG_CALL,("mb2t_send_flush => NULL\n"));
}


/*
 * mb2t_send_queue_msg
 *
 * This function is called from the wput function, to filter out
 * bad messages, and dispatch to the appropriate put routine.
 */

static void
mb2t_send_queue_msg(endp, mptr, type)
end_point *endp;		/* pointer to the endpoint structure */
mblk_t *mptr;			/* pointer to the message block */
ulong type;			/* type of message */
{	int mb2t_val_brdcst_req();

	DEBUGS(DBG_CALL,("mb2t_send_queue_msg() type = %d\n", type));

	switch(type) {
	case MB2_SEND_REQ:
		if (!(mb2t_val_send_req (endp, mptr))) {
			cmn_err (CE_WARN, "mb2t_send_queue_msg: illegal send req received discarding");
			freemsg (mptr);
			return;
		}
		if (!mb2t_send_send_req(endp, mptr)) {
			mb2t_stat[NO_SEND_QUED]++;
			putq(WR(endp->e_rdq), mptr);
		}
		break;
	case MB2_RSVP_REQ:
		if (!(mb2t_val_rsvp_req (endp, mptr))) {
			/*
			 * send a status message back
			 */
			mb2t_send_status_msg (endp, endp->e_socket, 0,
				((struct mb2_rsvp_req *)mptr->b_rptr)->REF_value);
						
			freemsg (mptr);
			return;
		}
		if (!(mb2t_send_rsvp_req(endp, mptr))) {
			mb2t_stat[NO_RSVP_QUED]++;
			putq(WR(endp->e_rdq), mptr);
		}
		break;
	case MB2_REPLY_REQ:
		if (!(mb2t_val_reply_req (endp, mptr))) {
			cmn_err (CE_WARN, "mb2t_send_queue_msg: illegal reply req received discarding");
			freemsg (mptr);
			return;
		}
		if (!(mb2t_send_reply_req(endp, mptr))) {
			mb2t_stat[NO_REPLY_QUED]++;
			putq(WR(endp->e_rdq), mptr);
		}
		break;
	case MB2_FRAG_REQ:
		if (!(mb2t_val_frag_req (endp, mptr))) {
			/*
			 * send a status message back
			 */
			mb2t_send_status_msg (endp, endp->e_socket, 0,
				((struct mb2_frag_req *)mptr->b_rptr)->REF_value);
			freemsg (mptr);
			return;
		}
		if (!(mb2t_send_frag_req(endp, mptr))) {
			mb2t_stat[NO_FRAG_QUED]++;
			putq(WR(endp->e_rdq), mptr);
		}
		break;
	case MB2_CANCEL_REQ:
		if (!(mb2t_send_cancel_req (endp, mptr))) {
			mb2t_stat[NO_CANCEL_QUED]++;
			putq (WR(endp->e_rdq), mptr);
		}
		break;
	case MB2_BRDCST_REQ:
		if (!(mb2t_val_brdcst_req (endp, mptr))) {
			cmn_err (CE_WARN, "mb2t_send_queue_msg: illegal brdcst req received discarding");
			freemsg (mptr);
			return;
		}
		if (!(mb2t_send_brdcst_req(endp, mptr))) {
			mb2t_stat[NO_BRDCST_QUED]++;
			putq(WR(endp->e_rdq), mptr);
		}
		break;
	default:
		/*
		 * A message of unknown type just discard it with a warning.
		 */
		cmn_err (CE_WARN, "mb2t_send_queue_msg: Invalid message discarding");
		freemsg (mptr);
		return;
	}
	DEBUGS(DBG_CALL,("mb2t_send_queue_msg() == 0 \n"));
	return;
}


/* ARGSUSED */
static int
mb2t_val_send_req(endp, mptr) 
end_point *endp;
register mblk_t *mptr;
{
	struct mb2_send_req *send_req;

	DEBUGS(DBG_CALL,("mb2t_val_send_req()\n"));
	send_req = (struct mb2_send_req *) mptr->b_rptr;
	if ((send_req->DATA_length == 0) && (msgdsize (mptr) != 0)) 
		return (0);
	
	if ((send_req->DATA_length != 0) && (msgdsize (mptr) == 0)) 
		return (0); 
	
	DEBUGS(DBG_CALL,("mb2t_val_send_req() ==> 0\n"));
	return (1);
}


/* ARGSUSED */
static int
mb2t_val_rsvp_req(endp, mptr) 
end_point *endp;
register mblk_t *mptr;
{
	struct mb2_rsvp_req *rsvp_req;

	DEBUGS(DBG_CALL,("mb2t_val_rsvp_req()\n"));
	rsvp_req = (struct mb2_rsvp_req *) mptr->b_rptr;

	if ((rsvp_req->DATA_length == 0) && (msgdsize (mptr) != 0)) 
		return (0);
	
	if ((rsvp_req->DATA_length != 0) && (msgdsize (mptr) == 0)) 
		return (0); 
	
	DEBUGS(DBG_FULL,("mb2t_val_rsvp_req => 0\n"));
	return (1);
}

/* ARGSUSED */
static int
mb2t_val_frag_req(endp, mptr) 
end_point *endp;
register mblk_t *mptr;
{
	struct mb2_frag_req *frag_req;

	DEBUGS(DBG_CALL,("mb2t_val_frag_req()\n"));
	frag_req = (struct mb2_frag_req *) mptr->b_rptr;

	if (frag_req->TRAN_id == 0) 
		return (0);
	
	DEBUGS(DBG_CALL,("mb2t_val_frag_req => 0\n"));
	return (1);
}


/* ARGSUSED */
static int
mb2t_val_reply_req(endp, mptr) 
end_point *endp;
register mblk_t *mptr;
{
	struct mb2_reply_req *reply_req;

	DEBUGS(DBG_CALL,("mb2t_val_reply_req()\n"));
	reply_req = (struct mb2_reply_req *) mptr->b_rptr;
	/*
	 *	check for error conditions  Note we dont have to ack errors.
	 */
	
	if ((reply_req->DATA_length == 0) && (msgdsize (mptr) != 0)) 
		return (0);

	if ((reply_req->DATA_length != 0) && (msgdsize (mptr) == 0)) 
		return (0); 
	
	if ((reply_req->DATA_length == 0) && (reply_req->EOT_flag != MB2_EOT))
		return (0);

	if (reply_req->TRAN_id == 0)
		return (0);

	DEBUGS(DBG_CALL,("mb2t_val_reply_req() ==> 0\n"));
	return (1);
}


/* ARGSUSED */
static int
mb2t_val_brdcst_req(endp, mptr) 
end_point *endp;
register mblk_t *mptr;
{

	DEBUGS(DBG_CALL,("mb2t_val_brdcst_req()\n"));
	if (msgdsize (mptr) != 0) 
		return (0);
	
	DEBUGS(DBG_CALL,("mb2t_val_brdcst_req() ==> 0\n"));
	return (1);
}


/*
 * routine to check a control message
 */

static void
mb2t_do_cntrl_msg (endp, mptr, type)
end_point *endp;
register mblk_t *mptr;
ulong type;
{
	switch (type) {
	case MB2_HOSTID_REQ:
		mb2t_hostid_req (endp, mptr);
		break;
	case MB2_BIND_REQ:
		mb2t_bind_req (endp, mptr);
		break;
	case MB2_UNBIND_REQ:
		mb2t_unbind_req (endp, mptr);
		break;
	case MB2_OPTMGMT_REQ:
		mb2t_optmgmt_req (endp, mptr);
		break;
	case MB2_INFO_REQ:
		mb2t_info_req (endp, mptr);
		break;
	default:
		freemsg (mptr);
		break;
	}
	return;
}

/*
 *	routine to process a hostid request. 
 */

static void
mb2t_hostid_req(endp, mptr)
end_point *endp;		/* pointer to an endpoint structure */
register mblk_t *mptr;		/* pointer to a message block */
{

	DEBUGS(DBG_CALL,("mb2t_hostid_req()\n"));
	mb2t_hostid_ack (endp, mptr);
	DEBUGS(DBG_CALL,("mb2t_hostid_req => NULL\n"));
	return;
}

/*
 *	routine to ack a hostid request. 
 */

static void
mb2t_hostid_ack(endp, mptr)
end_point *endp;		/* pointer to an endpoint structure */
register mblk_t *mptr;		/* pointer to a message block */
{
	struct mb2_hostid_ack *hostid_ack;
	ulong size;
	DEBUGS(DBG_CALL,("mb2t_hostid_ack()\n"));

	size = mptr->b_datap->db_lim - mptr->b_datap->db_base;
	if (size < sizeof (struct mb2_hostid_ack)) {
		freemsg (mptr);
		if (!(mptr = mb2t_getmptr(sizeof (struct mb2_hostid_ack), BPRI_MED, endp))) {
			cmn_err (CE_WARN, "hostid_ack: out of message blocks");
			return;
		}
	}
	mptr->b_datap->db_type = M_PCPROTO;
	mptr->b_rptr = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof (struct mb2_hostid_ack);
	hostid_ack = (struct mb2_hostid_ack *) mptr->b_rptr;
	hostid_ack->PRIM_type = MB2_HOSTID_ACK;
	hostid_ack->HOST_id = (ulong) ics_myslotid();
	putnext (endp->e_rdq, mptr);
	DEBUGS(DBG_CALL,("mb2t_hostid_ack => NULL\n"));
	return;
}


/*
 *	routine to process a bind request. If necessary allocates a port,
 *	and calls mps_open_chan to open a channel. Updates the endpoint taistate
 *	to reflect that the endpoint is bound to a port.
 */

static void
mb2t_bind_req(endp, mptr)
end_point *endp;
register mblk_t *mptr;
{
	ushort port;
	long chan;
	struct mb2_bind_req *bind_req;

	DEBUGS(DBG_CALL,("mb2t_bind_req()\n"));
	if (endp->e_taistate & T_BOUND) {
		mb2t_bind_ack (endp, mptr, EBUSY);
		return;
	}
	bind_req = (struct mb2_bind_req *)mptr->b_rptr;
	port = bind_req->SRC_port;
	if (port == 0) {
		if ((port = mb2t_alloc_port ()) == 0) {
			DEBUGS (DBG_FULL, ("mb2t_bind_req: cannot allocate port\n"));
			mb2t_bind_ack (endp, mptr, EINVAL);
			return;
		}
	}
	endp->e_socket = mps_mk_mb2socid (ics_myslotid(), port);
	if ((chan = mps_open_chan (mps_mk_mb2soctopid(endp->e_socket), mb2t_intr, MPS_NRMPRIO)) == -1) {
		DEBUGS(DBG_ERR,("mb2t_bind_req: mps_open_chan failed\n"));
		mb2t_bind_ack (endp, mptr, EBUSY);
		return;
	}
	endp->e_chan = chan;
	endp->e_taistate |= T_BOUND;
	mb2t_bind_ack (endp, mptr, 0);
	DEBUGS(DBG_CALL,("mb2t_bind_req => NULL\n"));
	return;
}

/*
 *  routine to return an ack to a bind request.
 */

static void
mb2t_bind_ack (endp, mptr, flag)
end_point *endp;		/* pointer to the endpoint structure */
register mblk_t *mptr;		/* pointer to the message block */
ulong flag;			/* flag */
{
	struct mb2_bind_ack *bind_ack;
	ulong size;
	
	DEBUGS(DBG_CALL,("mb2t_bind_ack()\n"));
	/*
	 * check if the message has enough buffer space, otherwise allocate
	 * new buffer
	 */
	size = mptr->b_datap->db_lim - mptr->b_datap->db_base;
	if (size < sizeof (struct mb2_bind_ack)) {
		freemsg (mptr);
		if (!(mptr = mb2t_getmptr(sizeof (struct mb2_bind_ack), BPRI_MED, endp))) {
			/*
			 * its okay, to not reply to the control message
			 */
			cmn_err (CE_WARN, "bind_ack: out of message blocks");
			return;
		}
	}
	mptr->b_datap->db_type = M_PCPROTO;
	mptr->b_rptr = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof (struct mb2_bind_ack);
	bind_ack = (struct mb2_bind_ack *) mptr->b_rptr;
	bind_ack->PRIM_type = MB2_BIND_ACK;
	bind_ack->RET_code = flag;
	putnext (endp->e_rdq, mptr);
	DEBUGS(DBG_FULL,("mb2t_bind_ack() ==> NULL\n"));
	return;
}

/*
 * this routine does essentialy the same stuff as the close routine
 * does except that in the case of the close the driver's endpoint
 * is freed
 */

static void
mb2t_unbind_req(endp, mptr)
end_point *endp;		/* pointer to an endpoint structure */
mblk_t *mptr;			/* pointer to a message block */
{

	DEBUGS(DBG_CALL,("mb2t_unbind_req()\n"));
	if (!(endp->e_taistate & T_BOUND)) {
		mb2t_unbind_ack (endp, mptr, EINVAL);
		return;
	}
	if (qsize(WR(endp->e_rdq)) != 0) {
		mb2t_unbind_ack (endp, mptr, EBUSY);
	   return;
	}
	if(mps_close_chan (endp->e_chan) == -1) {
		mb2t_unbind_ack (endp, mptr, EBUSY);
		return;
	}
	mb2t_free_port (mps_mk_mb2soctopid (endp->e_socket));
	endp->e_socket = 0;
	endp->e_taistate &= ~T_BOUND;
	endp->e_chan = 0;
	mb2t_unbind_ack(endp, mptr, 0);
	DEBUGS(DBG_CALL,("mb2t_unbind_req => 0\n"));
	return;
}

/*
 * this routine acks an unbind req
 */

static void
mb2t_unbind_ack (endp, mptr, flag)
end_point *endp;		/* pointer to an endpoint structure */
mblk_t *mptr;			/* pointer to a message block */
ulong flag;
{
	struct mb2_unbind_ack *unbind_ack;
	ulong size;

	DEBUGS(DBG_CALL,("mb2t_unbind_ack()\n"));
	size = mptr->b_datap->db_lim - mptr->b_datap->db_base;
	if (size < sizeof (struct mb2_unbind_ack)) {
		freemsg (mptr);
		if (!(mptr = mb2t_getmptr(sizeof (struct mb2_unbind_ack), BPRI_MED, endp))) {
			cmn_err (CE_WARN, "unbind_ack: out of message blocks");
			return;
		}
	}
	mptr->b_datap->db_type = M_PCPROTO;
	mptr->b_rptr = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof (struct mb2_unbind_ack);
	unbind_ack = (struct mb2_unbind_ack *) mptr->b_rptr;
	unbind_ack->PRIM_type = MB2_UNBIND_ACK;
	unbind_ack->RET_code = flag;
	putnext (endp->e_rdq, mptr);
	DEBUGS(DBG_FULL,("mb2t_unbind_ack() ==> NULL\n"));
	return;
}

/*
 *	routine to process an optmgmt request. 
 */

static void
mb2t_optmgmt_req(endp, mptr)
end_point *endp;		/* pointer to an endpoint structure */
register mblk_t *mptr;		/* pointer to a message block */
{
	struct mb2_optmgmt_req *optmgmt_req;
	struct stroptions *soptr;
	mblk_t *tmptr;

	DEBUGS(DBG_CALL,("mb2t_optmgmt_req()\n"));

	optmgmt_req = (struct mb2_optmgmt_req *) mptr->b_rptr;

	if (optmgmt_req->RSVP_tout != MB2_OPTDEFAULT) {
		if (optmgmt_req->RSVP_tout > mb2t_max_rsvp_tout) {
			/*
			 * the value is > than the configured value
			 */
			mb2t_optmgmt_ack (endp, mptr, EINVAL);
			return;
		}
	}

	if (optmgmt_req->FRAG_tout != MB2_OPTDEFAULT) {
		if (optmgmt_req->FRAG_tout > mb2t_max_frag_tout) {
			/*
			 * the value is > than the configured value
			 */
			mb2t_optmgmt_ack (endp, mptr, EINVAL);
			return;
		}
	}

	if (optmgmt_req->SEND_flow != MB2_OPTDEFAULT) {
		if ((optmgmt_req->SEND_flow > mb2t_max_send_hiwat) || 
		    (optmgmt_req->SEND_flow <= (WR(endp->e_rdq))->q_lowat)) {
			mb2t_optmgmt_ack (endp, mptr, EINVAL);
			return;
		}
	}

	if (optmgmt_req->RECV_flow != MB2_OPTDEFAULT) {
	       DEBUGS(DBG_FULL,("mb2t_ptmgmt:recv_flow != opt_default\n"));
		if ((optmgmt_req->RECV_flow > mb2t_max_recv_hiwat) || 
		    (optmgmt_req->RECV_flow <= ((endp->e_rdq))->q_lowat)) {
			mb2t_optmgmt_ack (endp, mptr, EINVAL);
			return;
		}
	}

	/*
	 * we are neglecting the send flow control value for the streamhead. 
	 */

	if (optmgmt_req->RECV_flow != MB2_OPTDEFAULT) {
		if((tmptr = allocb(sizeof(struct stroptions), BPRI_MED)) == NULL) {
			mb2t_optmgmt_ack (endp, mptr, EAGAIN);
			return;
		}
		tmptr->b_datap->db_type = M_SETOPTS;
		soptr = (struct stroptions *) tmptr->b_rptr;
		soptr->so_flags  =  SO_HIWAT;
		soptr->so_hiwat  = optmgmt_req->RECV_flow;
		tmptr->b_wptr = tmptr->b_rptr + sizeof(struct stroptions);
		DEBUGS(DBG_FULL,("mb2t_optmgmt_req : setting streamhead opts\n"));
		putnext(endp->e_rdq, tmptr);
		endp->e_rdq->q_hiwat = optmgmt_req->RECV_flow;
	}
	if (optmgmt_req->SEND_flow != MB2_OPTDEFAULT) 
		(WR (endp->e_rdq))->q_hiwat = optmgmt_req->SEND_flow;
	endp->e_fragtout = mb2t_def_fragtout;
	if (optmgmt_req->FRAG_tout != MB2_OPTDEFAULT) 
		endp->e_fragtout = optmgmt_req->FRAG_tout;
	endp->e_rsvptout = mb2t_def_rsvptout;
	if (optmgmt_req->RSVP_tout != MB2_OPTDEFAULT) 
		endp->e_rsvptout = optmgmt_req->RSVP_tout;
	mb2t_optmgmt_ack (endp, mptr, 0);
	DEBUGS(DBG_CALL,("mb2t_optmgmt_req => NULL\n"));
	return;
}

/*
 *	routine to ack an optmgmt request. 
 */

static void
mb2t_optmgmt_ack(endp, mptr, flag)
end_point *endp;		/* pointer to an endpoint structure */
register mblk_t *mptr;		/* pointer to a message block */
ulong flag;
{
	struct mb2_optmgmt_ack *optmgmt_ack;
	ulong size;

	DEBUGS(DBG_CALL,("mb2t_optmgmt_ack()\n"));

	size = mptr->b_datap->db_lim - mptr->b_datap->db_base;
	if (size < sizeof (struct mb2_optmgmt_ack)) {
		freemsg (mptr);
		if (!(mptr = mb2t_getmptr(sizeof (struct mb2_optmgmt_ack), BPRI_MED, endp))) {
			cmn_err (CE_WARN, "optmgmt_ack: out of message blocks");
			return;
		}
	}
	mptr->b_datap->db_type = M_PCPROTO;
	mptr->b_rptr = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof (struct mb2_optmgmt_ack);
	optmgmt_ack = (struct mb2_optmgmt_ack *) mptr->b_rptr;
	optmgmt_ack->PRIM_type = MB2_OPTMGMT_ACK;
	optmgmt_ack->RET_code =  flag;
	putnext (endp->e_rdq, mptr);
	DEBUGS(DBG_CALL,("mb2t_optmgmt_ack => NULL\n"));
	return;
}

/*
 *	routine to process an info request. 
 */

static void
mb2t_info_req(endp, mptr)
end_point *endp;		/* pointer to an endpoint structure */
register mblk_t *mptr;		/* pointer to a message block */
{

	DEBUGS(DBG_CALL,("mb2t_info_req()\n"));
	mb2t_info_ack (endp, mptr);
	DEBUGS(DBG_CALL,("mb2t_info_req => NULL\n"));
	return;
}

/*
 *	routine to ack an info request. 
 */


static void
mb2t_info_ack(endp, mptr)
end_point *endp;		/* pointer to an endpoint structure */
register mblk_t *mptr;		/* pointer to a message block */
{
	struct mb2_info_ack *info_ack;
	ulong size;

	DEBUGS(DBG_CALL,("mb2t_info_ack()\n"));

	size = mptr->b_datap->db_lim - mptr->b_datap->db_base;
	if (size < sizeof (struct mb2_info_ack)) {
		freemsg (mptr);
		if (!(mptr = mb2t_getmptr(sizeof (struct mb2_info_ack), BPRI_MED, endp))) {
			cmn_err (CE_WARN, "info_ack: out of message blocks");
			return;
		}
	}
	mptr->b_datap->db_type = M_PCPROTO;
	mptr->b_rptr = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof (struct mb2_info_ack);
	info_ack = (struct mb2_info_ack *) mptr->b_rptr;
	info_ack->PRIM_type = MB2_INFO_ACK;
	info_ack->SRC_addr =  endp->e_socket;
	info_ack->RSVP_tout = endp->e_rsvptout;
	info_ack->FRAG_tout = endp->e_fragtout;
	info_ack->SEND_flow = (WR(endp->e_rdq))->q_hiwat;
	info_ack->RECV_flow = (endp->e_rdq)->q_hiwat;
	putnext (endp->e_rdq, mptr);
	DEBUGS(DBG_CALL,("mb2t_info_ack => NULL\n"));
	return;
}


static int
mb2twsrv(wr_q)
register queue_t *wr_q;
{
	register mblk_t *mptr;
	end_point *endp;
	register int type;

	DEBUGS(DBG_CALL,("mb2twsrv()\n"));
	endp = (end_point *)wr_q->q_ptr;
	while ((mptr = getq(wr_q)) != NULL)
	{
		/*
		 * we first check if the process is closing the
		 * endpoint and do the appropriate things.
		 */
		if (endp->e_taistate & T_CLOSING) {
			if (!(mb2t_do_closing (endp, mptr))) {
				putbq (wr_q, mptr);
				return(0);
			}
			continue;
		}
		type = ((union primitives *)mptr->b_rptr)->type;
		/*
		 * Process the message.
		 */
		switch (type) {
		case MB2_SEND_REQ:
			 if (!(mb2t_send_send_req (endp, mptr))) {
				putbq (wr_q, mptr);
				return(0);
			 }
			break;
		case MB2_RSVP_REQ:
			if (!(mb2t_send_rsvp_req (endp, mptr))) {
				putbq (wr_q, mptr);
				return(0);
			}
			break;
		case MB2_REPLY_REQ:
			if (!(mb2t_send_reply_req (endp, mptr))) {
				putbq (wr_q, mptr);
				return(0);
			}
			break;
		case MB2_FRAG_REQ:
			if (!(mb2t_send_frag_req (endp, mptr))) {
				putbq (wr_q, mptr);
				return(0);
			}
			break;
		case MB2_CANCEL_REQ:
			if (!(mb2t_send_cancel_req (endp, mptr))) {
				putbq (wr_q, mptr);
				return(0);
			}
			break;
		case MB2_BRDCST_REQ:
			if (!(mb2t_send_brdcst_req (endp, mptr))) {
				putbq (wr_q, mptr);
				return(0);
			}
			break;
		default: 
			cmn_err (CE_PANIC, "mb2twsrv: inval type");
		}
	}
	DEBUGS(DBG_CALL,("mb2twsrv() ==> 0\n"));
	return(0);
}



static int
mb2t_do_closing (endp, mptr)
end_point *endp;
mblk_t *mptr;
{
	ulong type;
	int need_wakeup;
	struct mb2_send_req *send_req;
	struct mb2_reply_req *reply_req;

	DEBUGS(DBG_CALL,("mb2t_do_closing ()\n"));
	type = ((union primitives *)mptr->b_rptr)->type;
	need_wakeup = 0;
	switch (type) {
	case MB2_SEND_REQ:
		send_req = (struct mb2_send_req *) mptr->b_rptr;
		if (!(send_req->DATA_length > 0))
			need_wakeup = 1;
		if (!(mb2t_send_send_req (endp, mptr))) 
			return (0);
		break;
	case MB2_RSVP_REQ:
	case MB2_FRAG_REQ:
		/* 
		 * we dont want to complete these requests because it 
		 * expects a response.
		 */
		freemsg (mptr);
		need_wakeup = 1;
		break;
	case MB2_REPLY_REQ:
		reply_req = (struct mb2_reply_req *) mptr->b_rptr;
		if (!(reply_req->DATA_length > 0))
			need_wakeup = 1;
		if (!(mb2t_send_reply_req (endp, mptr))) 
			return (0);
		break;
	case MB2_CANCEL_REQ:
		if (!(mb2t_send_cancel_req (endp, mptr))) 
			return (0);
		need_wakeup = 1;
		break;
	case MB2_BRDCST_REQ:
		if (!(mb2t_send_brdcst_req (endp, mptr))) 
			return (0);
		need_wakeup = 1;
		break;
	default: 
		cmn_err (CE_PANIC, "mb2t_do_closing: inval type");
	}
	if (need_wakeup)
		wakeup ((caddr_t) &endp->e_taistate);
	DEBUGS(DBG_CALL,("mb2t_do_closing () ==> 1\n"));
	return (1);
}


static int
mb2trsrv(rd_q)
queue_t	*rd_q;
{
	mblk_t	*mp;

	DEBUGS(DBG_CALL,("mb2trsrv ()\n"));
	while  ((mp = getq(rd_q)) != NULL) {
		if (!canput(rd_q->q_next)) {
			DEBUGS(DBG_FULL,("mb2trsrv () !canput\n"));
			putbq (rd_q, mp);
			break;
		} else {
			if (mp->b_cont) {
				DEBUGS(DBG_FULL,("length of data = %d\n", mp->b_cont->b_wptr - mp->b_cont->b_rptr));
			} else
				DEBUGS(DBG_FULL,("NO data\n"));
				
			DEBUGS(DBG_FULL,("mb2trsrv () flow flag == DONE\n"));
			putnext(rd_q, mp);
		}
	}
	DEBUGS(DBG_CALL,("mb2trsrv () ==> 0\n"));
	return(0);
}


static void
mb2t_putq (mptr, endp)
mblk_t *mptr;
end_point *endp;
{
	if (!canput (endp->e_rdq->q_next)) {

		/* also enable the read service procedure */

		putq (endp->e_rdq, mptr);
		qenable (endp->e_rdq);
	}
	else
		putnext(endp->e_rdq, mptr);
}


static int
mb2t_can_send_upstr (mptr, endp) 
mblk_t *mptr;
end_point *endp;
{
	if (!canput(endp->e_rdq->q_next)) {
		mb2t_stat[NO_MSGS_REJ_FLO]++;
		return (0);
	}
	putnext (endp->e_rdq, mptr);
	return (1);
}


static int
mb2t_send_send_req(endp, mptr) 
end_point *endp;
register mblk_t *mptr;
{
	struct mb2_send_req *send_req;
	int ret = 0;

	DEBUGS(DBG_CALL,("mb2t_send_send_req()\n"));
	send_req = (struct mb2_send_req *) mptr->b_rptr;
	if (send_req->DATA_length == 0) 
		ret = mb2t_send_unsol (endp,mptr);
	else
		ret = mb2t_send_sol (endp, mptr);
	DEBUGS(DBG_CALL,("mb2t_send_send_req => 0\n"));
	return (ret);
}


static int
mb2t_send_unsol (endp, mptr)
end_point *endp;		/* pointer to the endpoint structure */
mblk_t *mptr;			/* pointer to a message block */
{
	struct mb2_send_req *send_req;
	struct mb2_prim_bind *pb;
	mb2socid_t dsocid;
	mps_msgbuf_t *mbp;

	DEBUGS(DBG_CALL,("mb2t_send_unsol()\n"));
	if ((pb = mb2t_alloc_pb (endp)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_unsol: alloc_pb failed\n"));
		mb2t_await_resource (endp, T_WAITPBIND);
		return (0);
	}
	if ((mbp = mps_get_msgbuf (KM_NOSLEEP)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_unsol: msgbuf = NULL\n"));
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, 0, pb);
		return (0);
	}
	send_req = (struct mb2_send_req *) mptr->b_rptr;
	dsocid = send_req->DEST_addr;
	mps_mk_unsol (mbp, dsocid, 0, 
			((unsigned char *)send_req + send_req->CTRL_offset), 
			send_req->CTRL_length);
#ifdef DEBUG
	mps_msg_showmsg (mbp);
#endif

	if (mps_AMPsend (endp->e_chan, mbp) == -1) {
		/*
		 * we cant do anything here just drop the message
		 */
		DEBUGS(DBG_ERR, ("mb2t_send_unsol: send failed\n"));
		mb2t_free_resource (endp, mbp, mptr, (struct dma_buf *)NULL, 
				(mblk_t *)NULL, (struct dma_buf *)NULL, 0, pb);
		DEBUGS(DBG_CALL,("mb2t_send_unsol() ==> 0\n"));
		return (1);
	}
	mb2t_stat[NO_SEND_SENT]++;
	mb2t_stat[NO_SEND_COMP]++;
	/* 
	 * we need to free the resources here because send is synchronous
	 */
	mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, mptr, 
					(struct dma_buf *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, 0, pb);
	DEBUGS(DBG_CALL,("mb2t_send_unsol() ==> 0\n"));
	return (1);
}

 
static int
mb2t_send_sol (endp, mptr)
end_point *endp;		/* pointer to the endpoint structure */
mblk_t *mptr;			/* pointer to a message block */
{
	struct mb2_send_req *send_req;
	struct mb2_prim_bind *pb;
	mb2socid_t dsocid;
	mps_msgbuf_t *mbp;
	struct dma_buf *odbuf;

	DEBUGS(DBG_CALL,("mb2t_send_sol()\n"));
	if ((pb = mb2t_alloc_pb (endp)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_sol: alloc_pb failed\n"));
		mb2t_await_resource (endp, T_WAITPBIND);
		return (0);
	}
	if ((mbp = mps_get_msgbuf (KM_NOSLEEP)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_sol: msgbuf = NULL\n"));
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, (mblk_t *)NULL, 					(struct dma_buf *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, 0, pb);
		return (0);
	}
	if ((odbuf = mb2t_mk_dbuf (mptr)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_sol: databuf = NULL\n"));
		mb2t_free_resource (endp, mbp, (mblk_t *)NULL, (struct dma_buf *)NULL, 
					(mblk_t *)NULL, (struct dma_buf *)NULL, 0, pb);
		return (0);
	}
	/*
	 * all resources have been successfully allocated can send the message
	 */
	send_req = (struct mb2_send_req *) mptr->b_rptr;
	dsocid = send_req->DEST_addr;
	mps_mk_sol (mbp, dsocid, 0, ((unsigned char *)send_req + send_req->CTRL_offset), 
			send_req->CTRL_length);
	mb2t_fill_pb (pb, MB2_OUTGOING|MB2_SEND_REQ, 0, 0, 
			 (ulong)msgdsize(mptr), endp, mptr, odbuf, (mblk_t *)NULL, (struct dma_buf *)NULL);
	mbp->mb_bind = (long) pb;

#ifdef DEBUG
	mps_msg_showmsg (mbp);
#endif

	if (mps_AMPsend_data (endp->e_chan, mbp, odbuf) == -1) {
		DEBUGS (DBG_ERR, ("mb2t_send_sol: mps_AMPsend_data failed\n"));
		mb2t_free_resource (endp, mbp, mptr, odbuf, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, 0, pb);
	}
	mb2t_stat[NO_SEND_SENT]++;
	/* 
	 * the interrupt routine will free the resources when the send has
	 * been done
	 */
	DEBUGS(DBG_CALL,("mb2t_send_sol => 0\n"));
	return (1);
}




static int
mb2t_send_rsvp_req (endp, mptr)
end_point *endp;
mblk_t *mptr;
{
	struct mb2_rsvp_req *rsvp_req;
	mblk_t *imptr, *idptr;
	struct dma_buf *odbuf, *idbuf;
	mb2socid_t dsocid;
	mps_msgbuf_t *mbp;
	unchar tid;
	struct mb2_prim_bind *pb;
	ulong msize;

	DEBUGS(DBG_CALL,("mb2t_send_rsvp()\n"));
	if ((pb = mb2t_alloc_pb (endp)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_rsvp_req: alloc_pb failed\n"));
		mb2t_await_resource (endp, T_WAITPBIND);
		return (0);
	}

	if ((mbp = mps_get_msgbuf (KM_NOSLEEP)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_rsvp_req: msgbuf = NULL\n"));
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, 0, pb);
		return (0);
	}
	rsvp_req = (struct mb2_rsvp_req *) mptr->b_rptr;
	odbuf = NULL;
	if (rsvp_req->DATA_length != 0) {
		if ((odbuf = mb2t_mk_dbuf (mptr)) == NULL) {
			DEBUGS(DBG_ERR, ("mb2t_send_rsvp_req: databuf = NULL\n"));
			mb2t_free_resource (endp, mbp, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, 0, pb);
			return (0);
		}
	}
	msize = sizeof (struct mb2_msg_ind) + UNSOL_LENGTH;
	if ((imptr = mb2t_getmptr(msize, BPRI_LO, endp)) == NULL) {
		DEBUGS(DBG_ERR,("mb2t_send_rsvp_req: imptr == NULL\n"));
		mb2t_free_resource (endp, mbp, (mblk_t *)NULL, odbuf, 
				(mblk_t *)NULL, (struct dma_buf *)NULL, 0, pb);
		return (mb2t_do_bufcall (endp, msize, 0, rsvp_req->REF_value));
	}

	idbuf = NULL;
	if (rsvp_req->RESP_length != 0) {
		if ((idptr = mb2t_get_strbuf (rsvp_req->RESP_length, BPRI_LO)) == NULL) {
			DEBUGS(DBG_ERR,("mb2t_send_rsvp_req: idptr == 0\n"));
			mb2t_free_resource (endp, mbp, (mblk_t *)NULL, 
					odbuf, imptr, (struct dma_buf *)NULL, 0, pb); 
			return (mb2t_do_bufcall (endp, rsvp_req->RESP_length, 0, 
						rsvp_req->REF_value));
		}
		linkb (imptr, idptr);
		if ((idbuf = mb2t_mk_dbuf (imptr)) == NULL) {
			DEBUGS(DBG_ERR,("mb2t_send_rsvp_req() idbuf == NULL\n"));
			mb2t_free_resource (endp, mbp, (mblk_t *)NULL, 
					odbuf, imptr, (struct dma_buf *)NULL, 0, pb);
			return (0);
		}
	}

	if ((tid = mps_get_tid (endp->e_chan)) == 0) {
		DEBUGS(DBG_ERR,("mb2t_send_rsvp_req: tid == 0\n"));
		mb2t_free_resource (endp, mbp, (mblk_t *)NULL, 
						odbuf, imptr, idbuf, 0, pb); 
		mb2t_await_resource (endp, T_WAITTID);	
		return (0);
	}
	/*
	 * have allocated all resources can't turn back now
	 */
	dsocid = rsvp_req->DEST_addr;
	if (rsvp_req->DATA_length > 0) {
		mps_mk_sol (mbp, dsocid, tid, 
			((unsigned char *)rsvp_req + rsvp_req->CTRL_offset), 
			rsvp_req->CTRL_length);
	} else {
		mps_mk_unsol (mbp, dsocid, tid, 
			((unsigned char *)rsvp_req + rsvp_req->CTRL_offset), 
			rsvp_req->CTRL_length);
	}
	mb2t_fill_pb (pb, MB2_OUTGOING|MB2_RSVP_REQ, rsvp_req->REF_value,
				tid, rsvp_req->RESP_length, endp, mptr, odbuf, 
				imptr, idbuf);
	mbp->mb_bind = (long) pb;

#ifdef DEBUG 
	mps_msg_showmsg (mbp);
#endif
	mb2t_stat[NO_RSVP_SENT]++;
	if (endp->e_rsvptout != 0)
		pb->pb_tval = timeout (mb2t_rsvp_tout, (caddr_t) pb, 
					endp->e_rsvptout); 
	if (mps_AMPsend_rsvp (endp->e_chan, mbp, odbuf, idbuf) == -1) {
	       DEBUGS (DBG_ERR, ("mb2t_send_rsvp_req: mps_AMPsend_rsvp failed\n"));
		mb2t_stat[NO_RSVP_SENT]--;
		if (endp->e_rsvptout != 0)
			untimeout (pb->pb_tval);
		/*
		 *  send a status message up
		 */
		mb2t_send_status_msg (endp, endp->e_socket, tid, 
							rsvp_req->REF_value);
		mb2t_free_resource (endp, mbp, mptr, odbuf, imptr, idbuf, tid, pb);
	} 
	DEBUGS(DBG_CALL,("mb2t_send_rsvp_req => 0\n"));
	return (1);
}


static int
mb2t_send_frag_req (endp, mptr)
end_point *endp;		/* pointer to the endpoint structure */
mblk_t *mptr;			/* pointer to a message block */
{
	struct mb2_frag_req *frag_req;
	mblk_t *imptr, *idptr;
	mps_msgbuf_t *mbp;
	struct dma_buf *idbuf;
	struct mb2_prim_bind *pb;
	ulong msize;

	DEBUGS(DBG_CALL,("mb2t_send_frag_req()\n"));
	if ((pb = mb2t_alloc_pb (endp)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_frag_req: alloc_pb failed\n"));
		mb2t_await_resource (endp, T_WAITPBIND);
		return (0);
	}

	if ((mbp = mps_get_msgbuf (KM_NOSLEEP)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_frag_req: msgbuf = NULL\n"));
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, 0, pb);
		return (0);
	}
	frag_req = (struct mb2_frag_req *) mptr->b_rptr;
	msize = sizeof (struct mb2_msg_ind) + UNSOL_LENGTH;
	if ((imptr = mb2t_getmptr(msize, BPRI_LO, endp)) == NULL) {
		DEBUGS(DBG_ERR,("mb2t_send_frag_req: imptr == NULL\n"));
		mb2t_free_resource (endp, mbp, (mblk_t *)NULL, (struct dma_buf *)NULL, 
					(mblk_t *)NULL, (struct dma_buf *)NULL, 0, pb);
		return (mb2t_do_bufcall (endp, msize, (unchar)frag_req->TRAN_id, 
						frag_req->REF_value));
	}

	idbuf = NULL;
	if (frag_req->DATA_length != 0) {
		if ((idptr = mb2t_get_strbuf (frag_req->DATA_length, BPRI_LO)) == NULL) {
			DEBUGS(DBG_ERR,("mb2t_send_frag_req: idptr == NULL\n"));
			mb2t_free_resource (endp, mbp, 
					(mblk_t *) NULL, (struct dma_buf *)NULL, imptr,
					(struct dma_buf *)NULL, 0, pb);
			return (mb2t_do_bufcall (endp, frag_req->DATA_length, 
						(unchar)frag_req->TRAN_id, 
						frag_req->REF_value));
		}
		linkb (imptr, idptr);
		if ((idbuf = mb2t_mk_dbuf (imptr)) == NULL) {
			DEBUGS(DBG_ERR,("mb2t_send_frag_req() idbuf == NULL\n"));
			mb2t_free_resource (endp, mbp, 
						(mblk_t *)NULL, 
				 		(struct dma_buf *)NULL, imptr, 
						(struct dma_buf *)NULL, 0, pb);
			return (0);
		}
	}

	mb2t_fill_pb (pb, MB2_OUTGOING|MB2_FRAG_REQ, frag_req->REF_value,  
				(unchar)frag_req->TRAN_id, frag_req->DATA_length,
				endp, mptr, (struct dma_buf *)NULL, imptr, idbuf);
	mbp->mb_bind = (long) pb;

#ifdef DEBUG 
	mps_msg_showmsg (mbp);
#endif  
	mb2t_stat[NO_FRAG_SENT]++;
	if (endp->e_fragtout != 0)
		pb->pb_tval = timeout (mb2t_frag_tout, (caddr_t) pb, 
				endp->e_fragtout);
	if (mps_AMPreceive_frag (endp->e_chan, mbp, frag_req->DEST_addr,
					frag_req->TRAN_id,idbuf) == -1) {
		DEBUGS(DBG_ERR,("mb2t_send_frag_req mps_AMPreceive_frag failed\n"));
		mb2t_stat[NO_FRAG_SENT]--;
		if (endp->e_fragtout != 0)
			untimeout (pb->pb_tval); 
		/*
		 * send a status message 
		*/
		mb2t_send_status_msg (endp, endp->e_socket, 
				(unchar)frag_req->TRAN_id, frag_req->REF_value);
		mb2t_free_resource (endp, mbp, mptr, (struct dma_buf *)NULL, 
						imptr, idbuf, 0, pb);
	} 
	DEBUGS(DBG_CALL,("mb2t_send_frag_req => 0\n"));
	return (1);
}



static int
mb2t_send_reply_req (endp, mptr)
end_point *endp;
mblk_t *mptr;
{
	struct mb2_reply_req *reply_req;
	mb2socid_t dsocid;
	mps_msgbuf_t *mbp;
	struct dma_buf *odbuf;
	struct mb2_prim_bind *pb; 
	unsigned char lastflag;
	ulong datalen;

	DEBUGS(DBG_CALL,("mb2t_send_reply_req()\n"));
	if ((pb = mb2t_alloc_pb (endp)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_reply_req: alloc_pb failed\n"));
		mb2t_await_resource (endp, T_WAITPBIND);
		return (0);
	}
	if ((mbp = mps_get_msgbuf (KM_NOSLEEP)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_reply_req: msgbuf = NULL\n"));
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, 0, pb);
		return (0);
	}
	reply_req = (struct mb2_reply_req *) mptr->b_rptr;
	odbuf = NULL;
	datalen = reply_req->DATA_length;
	if (datalen != 0) {
		if ((odbuf = mb2t_mk_dbuf (mptr)) == NULL) {
			DEBUGS(DBG_ERR, ("mb2t_send_reply: databuf = NULL\n"));
			mb2t_free_resource (endp, mbp, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, 
					(mblk_t *)NULL, (struct dma_buf *)NULL, 0, pb);
			return (0);
		}
	}

	dsocid = reply_req->DEST_addr;
	if (datalen != 0) {
		lastflag = 0;
		if (reply_req->EOT_flag == MB2_EOT)
			lastflag = 1;
		mps_mk_solrply (mbp, dsocid, reply_req->TRAN_id, 
			((unsigned char *)reply_req + reply_req->CTRL_offset), 
			reply_req->CTRL_length, lastflag);
	} else {
		mps_mk_unsolrply (mbp, dsocid, reply_req->TRAN_id, 
			((unsigned char *)reply_req + reply_req->CTRL_offset), 
			reply_req->CTRL_length);
	}

	mb2t_fill_pb (pb, MB2_OUTGOING|MB2_REPLY_REQ, 
						0, 
						(unchar)reply_req->TRAN_id, 
						(ulong)msgdsize (mptr), endp, mptr, odbuf, 
						(mblk_t *)NULL, (struct dma_buf *)NULL);
	mbp->mb_bind = (long) pb;

#ifdef DEBUG 
	mps_msg_showmsg (mbp);
#endif 

	if (mps_AMPsend_reply (endp->e_chan, mbp, odbuf) == -1) {
		DEBUGS (DBG_ERR,("mb2t_send_reply_req: mps_AMPsend_reply failed\n"));
		mb2t_free_resource (endp, mbp, mptr, odbuf, 
					(mblk_t *)NULL, (struct dma_buf *)NULL, 0, pb);
		return (1);
	}
	mb2t_stat[NO_REPLY_SENT]++;
	/*
	 * we have to free resources for an unsol sendreply because it is
	 * synchronous
	 */
	if (datalen == 0) {
		mb2t_stat[NO_REPLY_COMP]++;
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, mptr, 
				(struct dma_buf *)NULL, 
				(mblk_t *)NULL, (struct dma_buf *)NULL, 0, pb);
	}
	DEBUGS(DBG_CALL,("mb2t_send_reply_req => 0\n"));
	return (1);
}



static int
mb2t_send_cancel_req (endp, mptr)
end_point *endp;		/* pointer to the endpoint structure */
mblk_t *mptr;			/* pointer to a message block */
{
	struct mb2_cancel_req *cancel_req;
	mb2socid_t dsocid;
	unchar tid;

	cancel_req = (struct mb2_cancel_req *) mptr->b_rptr;
	dsocid =  (mb2socid_t) cancel_req->DEST_addr;
	tid = cancel_req->TRAN_id;
	if (mps_AMPcancel (endp->e_chan, dsocid, tid) == -1) {
		/*
		 * Wrong tid provided so nothing can be done
		 */
		DEBUGS (DBG_ERR, ("mb2t_send_cancel_req: mps_AMPcancel failed\n"));
	} else {
		mb2t_stat[NO_CANCEL_SENT]++;
		mb2t_stat[NO_CANCEL_COMP]++;
	}
	mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, mptr, 
				(struct dma_buf *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, 0, (struct mb2_prim_bind *)NULL);
	return (1);	 
}


static int
mb2t_send_brdcst_req (endp, mptr)
end_point *endp;		/* pointer to the endpoint structure */
mblk_t *mptr;			/* pointer to a message block */
{
	struct mb2_brdcst_req *brdcst_req;
	struct mb2_prim_bind *pb;
	mb2socid_t dportid;
	mps_msgbuf_t *mbp;

	DEBUGS(DBG_CALL,("mb2t_send_brdcst_req ()\n"));
	if ((pb = mb2t_alloc_pb (endp)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_brdcst_req: alloc_pb failed\n"));
		mb2t_await_resource (endp, T_WAITPBIND);
		return (0);
	}
	if ((mbp = mps_get_msgbuf (KM_NOSLEEP)) == NULL) {
		DEBUGS(DBG_ERR, ("mb2t_send_brdcst_req: msgbuf = NULL\n"));
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, 0, pb);
		return (0);
	}
	brdcst_req = (struct mb2_brdcst_req *) mptr->b_rptr;
	dportid = brdcst_req->DEST_addr;
	mps_mk_brdcst (mbp, dportid,((unsigned char *)brdcst_req + brdcst_req->CTRL_offset), 
				brdcst_req->CTRL_length);
#ifdef DEBUG
	mps_msg_showmsg (mbp);
#endif

	if (mps_AMPsend (endp->e_chan, mbp) == -1) {
		/*
		 * we cant do anything here just drop the message
		 */
		DEBUGS(DBG_ERR, ("mb2t_send_brdcst_req: send failed\n"));
		mb2t_free_resource (endp, mbp, mptr, (struct dma_buf *)NULL, 
				(mblk_t *)NULL, (struct dma_buf *)NULL, 0, pb);
		DEBUGS(DBG_CALL,("mb2t_send_brdcst_req() ==> 0\n"));
		return (1);
	}
	mb2t_stat[NO_BRDCST_SENT]++;
	mb2t_stat[NO_BRDCST_COMP]++;
	/* 
	 * we need to free the resources here because send is synchronous
	 */
	mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, mptr, 
					(struct dma_buf *)NULL, (mblk_t *)NULL, 
					(struct dma_buf *)NULL, 0, pb);
	DEBUGS(DBG_CALL,("mb2t_send_brdcst_req() ==> 1\n"));
	return (1);
}
/*
 * cancel all outstanding transactions. Called from close 
 */

static void
mb2t_cancel_outtrans (endp) 
end_point *endp;
{
	struct mb2_prim_bind *pb;
	struct mb2_frag_req *frag_req;
	mb2socid_t dsocid;
	int ret, s;

	DEBUGS (DBG_CALL, ("mb2t_cancel_outtrans ()  \n"));
	s = SPL ();
	pb = endp->e_pb.pb_next;
	while (pb != (&(endp->e_pb))) {
		if (pb->pb_tid != 0) {
			if (pb->pb_flags == (MB2_OUTGOING|MB2_FRAG_REQ)) {
				frag_req = (struct mb2_frag_req *)
						pb->pb_mptr->b_rptr;
				dsocid = frag_req->DEST_addr;
			} else {
				if (pb->pb_flags == (MB2_OUTGOING|MB2_RSVP_REQ)) 
					dsocid = endp->e_socket;
				
			}
			ret = mps_AMPcancel (endp->e_chan, dsocid, 
					(unchar)pb->pb_tid);
			if (ret == -1) {
				DEBUGS (DBG_ERR, ("mb2t_cancel_outtrans: mps_AMPcancel failed\n"));
			}
		}
		pb = pb->pb_next;
	}
	splx (s);
	DEBUGS (DBG_CALL, ("mb2t_cancel_outtrans ()\n"));
	return;
}

/*
 *	interrupt handler
 */

static int
mb2t_intr(mbp)
mps_msgbuf_t *mbp;
{
	ulong bindid;

	DEBUGS(DBG_CALL, ("mb2t_intr\n"));
	bindid = mbp->mb_bind;
	if(bindid == (unsigned)MPS_MG_DFBIND) {
		/* Handle all incoming request messages */
		mb2t_handle_inreq(mbp);
	} else {
		mb2t_handle_comp (mbp);
	}
	return(0);
}



static void
mb2t_handle_comp (mbp)
mps_msgbuf_t *mbp;
{
	unchar msgtype;
	mb2socid_t src_socid;
	mb2socid_t dst_socid;
	struct mb2_prim_bind *pb;
	unchar tid;
	ulong msg_length;
	ulong datalen;
	end_point *endp;

	DEBUGS (DBG_CALL, ("completion message recieved\n")); 
	mb2t_stat[NO_COMP_MSGS]++;

#ifdef DEBUG
	mps_msg_showmsg (mbp);
#endif 

	msgtype = mps_msg_getmsgtyp (mbp);
	tid = mps_msg_gettrnsid(mbp);
	src_socid = mps_mk_mb2socid (mps_msg_getsrcmid(mbp), mps_msg_getsrcpid(mbp));
	dst_socid = mps_mk_mb2socid (mps_msg_getdstmid(mbp), mps_msg_getdstpid(mbp));

	pb = (struct mb2_prim_bind *)mbp->mb_bind;
	endp = (end_point *) pb->pb_endp;
	/*
	 * if we are in the process of closing all we need to do is
	 * handle the completion message appropriatly and free resources
	 */
	if (endp->e_taistate & T_CLOSING) {
		mb2t_do_comp_close (endp, pb);
		mps_free_msgbuf (mbp);
		return;
	}
	switch (pb->pb_flags) {
	case MB2_INCOMING|MB2_NTRAN_MSG:
	case MB2_INCOMING|MB2_REQ_MSG:
		/*
		 * this is a completion of the receive of a message
		 */
		if(MSG_ISERR(mbp)) {
			cmn_err (CE_WARN, "mb2t_handle_comp: receive error");
			mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, 
						(mblk_t *)NULL, 
						(struct dma_buf *)NULL, pb->pb_imptr, 
						(struct dma_buf *)NULL, 0, pb);
		} else {
			DEBUGS(DBG_FULL, ("mb2t_handle_comp:  receive completed ok"));
			mb2t_msg_ind_comp(endp, pb);
		}
		break;
	case MB2_OUTGOING|MB2_SEND_REQ:
		if(MSG_ISERR(mbp)) {
			DEBUGS(DBG_ERR, ("mb2t_handle_comp:  sol_req completed error"));
		} else {
			DEBUGS(DBG_FULL, ("mb2t_handle_comp:  receive completed ok"));
		}
		mb2t_send_req_comp (endp, pb);
		break;
	case MB2_OUTGOING|MB2_FRAG_REQ:
		/*
		 * this is a completion of the receive of a  frag request
		 */
		untimeout (pb->pb_tval); 
		datalen = (msgtype == MPS_MG_UNSOL) ? 0: mps_msg_getbrlen (mbp);
		if(MSG_ISERR(mbp)) {
			DEBUGS(DBG_ERR, ("mb2t_handle_comp:  receive completed error"));
			mb2t_send_status_msg (endp, endp->e_socket, 
						(unchar)pb->pb_tid, pb->pb_uref); 
			mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, 
					pb->pb_mptr, 
					(struct dma_buf *) NULL, pb->pb_imptr, 
					(struct dma_buf *) NULL, 0, pb);
		} else {
			DEBUGS(DBG_FULL, ("mb2t_handle_comp:  receive completed ok"));
			mb2t_frag_req_comp (endp, pb, src_socid, tid, datalen);
		}
		break;
	case MB2_OUTGOING|MB2_RSVP_REQ:
		/*
		 * Completion of an rsvp request. We could have recieved a 
		 * cancel too in this case.
		 */
		untimeout (pb->pb_tval);
		if(MSG_ISERR(mbp)) {
			DEBUGS(DBG_ERR, ("mb2t_handle_comp:  rsvp completed error"));
			mb2t_send_status_msg (endp, endp->e_socket, 
						(unchar)pb->pb_tid, pb->pb_uref); 
			mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, 
					pb->pb_mptr, 
					(struct dma_buf *) NULL, pb->pb_imptr, 
					(struct dma_buf *) NULL, (unchar)pb->pb_tid, pb);
			break; 
		} 
		msg_length = (msgtype == MPS_MG_UNSOL) ? mbp->mb_count - MPS_MG_UMOVHD: 
						mbp->mb_count - MPS_MG_BROVHD;
		datalen = (msgtype == MPS_MG_UNSOL) ? 
					mps_get_reply_len(dst_socid, tid): mps_msg_getbrlen (mbp);
		if (mps_msg_iscancel(mbp)) {
			DEBUGS(DBG_ERR, ("mb2t_handle_comp:  rsvp completed cancel"));
			mb2t_rsvp_req_comp (endp, pb, src_socid, tid, msg_length, 
				(char *)mps_msg_getudp(mbp), datalen, MB2_CANCEL_MSG);
		} else {
		
			DEBUGS(DBG_FULL,("mb2t_handle_comp:rsvp completed ok"));
			mb2t_rsvp_req_comp (endp, pb, src_socid, tid, msg_length, 
				(char *)mps_msg_getudp(mbp), datalen, MB2_RESP_MSG);
		}
		break;
	case MB2_OUTGOING|MB2_REPLY_REQ:
		if(MSG_ISERR(mbp)) {
			DEBUGS(DBG_ERR, ("mb2t_handle_comp:  send_reply completed error"));
		} else {
			DEBUGS(DBG_FULL, ("mb2t_handle_comp:  send_reply completed ok"));
		}
		mb2t_reply_req_comp (endp, pb, mbp);
		break;
	default:	
		cmn_err (CE_WARN, "mb2t_handle_comp: Illegal completion message");
		break;
	}
	mps_free_msgbuf (mbp);
	return;
}


static void
mb2t_do_comp_close (endp, pb)
end_point *endp;
struct mb2_prim_bind *pb;
{
	if (pb->pb_flags == (MB2_OUTGOING|MB2_RSVP_REQ)) {
		/*
		 * free the tid only in the case when we have allocated it
		 * and not if the tid is a completion of a receive
		 */
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL,
					(mblk_t *)NULL, (struct dma_buf *)NULL,
					(mblk_t *)NULL, (struct dma_buf *)NULL,
					(unchar)pb->pb_tid, 
					(struct mb2_prim_bind *)NULL);
	}
	if (pb->pb_tval != 0)
		untimeout (pb->pb_tval);
	mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, 
				pb->pb_mptr, 
				(struct dma_buf *)NULL, pb->pb_imptr, 
				(struct dma_buf *)NULL, 0, pb);
	wakeup ((caddr_t) &endp->e_taistate);
	return;
}


static void
mb2t_send_req_comp (endp, pb)
end_point *endp;
struct mb2_prim_bind *pb;
{

	DEBUGS (DBG_CALL, ("mb2t_send_req_comp: pb = %x \n", pb));
	mb2t_stat[NO_SEND_COMP]++;
	mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, pb->pb_mptr, 
				(struct dma_buf *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, 0, pb);
	DEBUGS (DBG_CALL, ("mb2t_send_req_comp: ==> 0\n"));
}


static void
mb2t_reply_req_comp (endp, pb, mbp)
end_point *endp;
struct mb2_prim_bind *pb;
mps_msgbuf_t *mbp;
{
	DEBUGS (DBG_CALL, ("mb2t_reply_req_comp: pb = %x \n", pb));
	mb2t_stat[NO_REPLY_COMP]++;
	mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, pb->pb_mptr, 
				(struct dma_buf *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, 0, pb);
	DEBUGS (DBG_CALL, ("mb2t_reply_req_comp: ==> 0\n"));
}

 
static void
mb2t_frag_req_comp (endp, pb, socid, tid, datalen)
end_point *endp;
struct mb2_prim_bind *pb;
mb2socid_t socid;
unchar tid;
ulong datalen;
{ 
	int len;

	DEBUGS (DBG_CALL, ("mb2t_frag_req_comp: pb = %x \n", pb));
	if (tid != pb->pb_tid) {
		cmn_err (CE_WARN, "Transport error mismatching tid's");
		mb2t_send_status_msg (endp, endp->e_socket, 
						(unchar)pb->pb_tid, pb->pb_uref); 
	} else {
		mb2t_stat[NO_FRAG_COMP]++;
		len = datalen - pb->pb_datalen;
		if (len < 0)
			adjmsg (pb->pb_imptr->b_cont, len);
		mb2t_fill_msg (pb->pb_imptr, MB2_FRAGRES_MSG, 
					socid, tid, pb->pb_uref, datalen, 0, (char *)NULL);
		mb2t_putq (pb->pb_imptr, endp);
	}
	mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, pb->pb_mptr, 
				(struct dma_buf *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, 0, pb);
	DEBUGS (DBG_CALL, ("mb2t_frag_req_comp: ==> 0\n"));
	return;
}




static void
mb2t_rsvp_req_comp (endp, pb, socid, tid, ctllen, ctlbuf, datalen, msgtype)
end_point *endp;
struct mb2_prim_bind *pb;
mb2socid_t socid;
unchar tid;
ulong ctllen;
char *ctlbuf;
ulong datalen;
ulong msgtype;
{ 
	int len;

	DEBUGS (DBG_CALL, ("mb2t_rsvp_req_comp: pb = %x \n", pb));
	if (tid != pb->pb_tid) {
		cmn_err (CE_WARN, "rsvp_req: Transport error in tid");
		mb2t_send_status_msg (endp, endp->e_socket, 
						(unchar)pb->pb_tid, pb->pb_uref); 
	} else {
		/*
		 * we need to trim the length of the message according
		 * to the response received.
		 */
		mb2t_stat[NO_RSVP_COMP]++;
		len = datalen - pb->pb_datalen;
		if (len < 0)
			adjmsg (pb->pb_imptr->b_cont, len);
		mb2t_fill_msg (pb->pb_imptr, msgtype, socid, 
					tid, pb->pb_uref, datalen, ctllen, ctlbuf);
		mb2t_putq (pb->pb_imptr, endp);
	}
	mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, pb->pb_mptr, 
				(struct dma_buf *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, (unchar)pb->pb_tid, pb);
	DEBUGS (DBG_CALL, ("mb2t_rsvp_req_comp: ==> 0\n"));
	return;
}



static void
mb2t_handle_inreq (mbp)
mps_msgbuf_t *mbp;
{
	unchar tid;
	mb2socid_t  src_socid;
	unchar lsnid;
	unchar msgtyp;
	ushort bind_flag;
	ulong  msg_len;
	end_point *endp;
	struct dma_buf *ibuf;
	mps_msgbuf_t *omsg;
	struct mb2_prim_bind *pb;

	DEBUGS(DBG_CALL, ("mb2t_handle_inreq\n"));
	mb2t_stat[NO_INC_MSGS]++;

	/*
	 * the freeing of the Transport message buffer is important in this
	 * routine. Be Careful when freeing it.
	 */

#ifdef DEBUG 
	mps_msg_showmsg (mbp);
#endif

	msgtyp = mps_msg_getmsgtyp (mbp);
	tid = mps_msg_gettrnsid(mbp);
	lsnid = mps_msg_getlsnid(mbp);
	src_socid = mps_mk_mb2socid (mps_msg_getsrcmid(mbp), mps_msg_getsrcpid(mbp));

	endp = mb2t_getendp (mps_msg_getdstmid(mbp), mps_msg_getdstpid(mbp), msgtyp);

	if (endp == NULL) {
		cmn_err (CE_PANIC, "handle_inreq: Illegal message recieved discarding");
		return;
	}
	/*
	 * we dont accept any fresh new messages when we are
	 * in the process of closing.
	 */
	if (endp->e_taistate & T_CLOSING) {
		mb2t_handle_undelmsg (endp, mbp);
		wakeup ((caddr_t) &endp->e_taistate);
		return;
	}

	if (MSG_ISERR(mbp)) {
		cmn_err (CE_WARN, "handle_inreq: Illegal message recieved discarding");
		mb2t_handle_undelmsg (endp, mbp);
		return;
	}

	if (msgtyp == MPS_MG_BRDCST) {
		bind_flag = MB2_INCOMING|MB2_BRDCST_MSG;
	} else {
		if(tid == 0) {
			bind_flag = MB2_INCOMING|MB2_NTRAN_MSG;
		} else {
			bind_flag = MB2_INCOMING|MB2_REQ_MSG;
		}
	}

	if(msgtyp == MPS_MG_BREQ) {
		msg_len = mps_msg_getbrlen (mbp);
		DEBUGS (DBG_FULL, ("mb2t_handle_inreq: message len = %d\n", msg_len));
		if ((pb = mb2t_get_sol_buf(src_socid, 
				     (ulong)bind_flag, tid, (ulong) mbp->mb_bind,
				     endp, (ulong)(mbp->mb_count - MPS_MG_BROVHD), 
				     (char *)mps_msg_getudp(mbp), (ulong)msg_len, &ibuf)) == NULL) {
			if (tid != 0) {
				if (mb2t_send_reqfrag_msg (mbp, endp) == 1) {
						mps_mk_breject (mbp, src_socid, lsnid);
						mps_AMPsend (endp->e_chan, mbp);
				} else {
					/*
				 	 * if the send_req_frag routine failed then a cancel would
				 	 * have been sent by the routine. However we need to free the
					 * the message buffer.
				 	 */
					mps_free_msgbuf (mbp);
				}
			} else {
				mps_mk_breject (mbp, src_socid, lsnid);
				mps_AMPsend (endp->e_chan, mbp);
			}
			return;
		}

		if ((omsg = mps_get_msgbuf(KM_NOSLEEP)) == NULL) {
			DEBUGS(DBG_ERR, ("mb2t_handle_inreq: can not get msgbuf"));
			if (tid != 0) {
				if (mb2t_send_reqfrag_msg (mbp, endp) == 1) {
						mps_mk_breject (mbp, src_socid, lsnid);
						mps_AMPsend (endp->e_chan, mbp);
				} else {
					/*
				 	 * if the send_req_frag routine failed then a cancel would
				 	 * have been sent by the routine. However we need to free
					 * the message buffer.
				 	 */
					mps_free_msgbuf (mbp);
				}
			} else {
				mps_mk_breject (mbp, src_socid, lsnid);
				mps_AMPsend (endp->e_chan, mbp);
			}
			/*
			 * Note that send_reqfrag_msg only frees the resources that it
			 * allocates. So we have to free resources allocated by other
			 * routines here.
			 */
			mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, 
							(mblk_t *)NULL, 
							(struct dma_buf *)NULL, 
						   pb->pb_imptr, ibuf, 0, pb);
			return;
		}

		mps_mk_bgrant(omsg, src_socid, mps_msg_getlsnid(mbp), msg_len);
		omsg->mb_bind = (long) pb;
		DEBUGS(DBG_FULL, ("mb2t_handle_inreq: doing recieve\n"));

		if(mps_AMPreceive(endp->e_chan, src_socid, omsg, ibuf) == -1) {
			DEBUGS(DBG_ERR, ("mb2t_handle_inreq: sol receive failed"));
			if (tid != 0) {
				if (mps_AMPcancel (endp->e_chan, src_socid, tid) == -1)
						cmn_err (CE_WARN, "mb2t_handle_inreq: Cancel failed\n");
				mps_free_msgbuf (mbp);
			} else {
				mps_mk_breject (mbp, src_socid, lsnid);
				mps_AMPsend (endp->e_chan, mbp);
			}
			mb2t_free_resource (endp, omsg, (mblk_t *)NULL, 
								(struct dma_buf *)NULL, 
						     	pb->pb_imptr, ibuf, 0, pb);
			return;
		}
		mps_free_msgbuf (mbp);
		DEBUGS(DBG_FULL, ("mb2t_handle_inreq: done recieve\n"));
		return;
	}
 
	/* got an unsolicited message  or a broadcast message */

	if ((pb = mb2t_get_unsol_buf (src_socid, (ulong)bind_flag, tid,
			    	mbp->mb_bind, endp, 
				(ulong) (mbp->mb_count - MPS_MG_UMOVHD), 
				(char *)mps_msg_getudp(mbp))) == NULL) {
			if (tid != 0) {
				if (mps_AMPcancel (endp->e_chan, src_socid, tid) == -1) {
						cmn_err (CE_WARN, "mb2t_handle_inreq: Cancel failed\n");
				}
			}
	} else {
		mb2t_msg_ind_comp (endp, pb); 
	}
	mps_free_msgbuf (mbp);
	DEBUGS(DBG_CALL, ("mb2t_handle_inreq: ==> NULL\n"));
	return;

}



static void
mb2t_msg_ind_comp (endp, pb)
end_point *endp;
struct mb2_prim_bind *pb;
{ 
	struct mb2_msg_ind *msg_ind;
	mb2socid_t src_socid;
	unchar tid;

	DEBUGS (DBG_CALL, ("mb2t_msg_ind_comp: pb = %x \n", pb));
	/*
	 * check if we can send it up stream
	 */
	msg_ind = (struct mb2_msg_ind *) pb->pb_imptr->b_rptr;
	/*
	 * we need to get the dest socid because receive does not return the
	 * the dest socid.
	 */
	src_socid = (mb2socid_t)(msg_ind->SRC_addr);
	tid = (unchar)msg_ind->TRAN_id;
	if (mb2t_can_send_upstr (pb->pb_imptr, endp) == 0) {
		if ((pb->pb_flags & ~MB2_INCOMING) == MB2_REQ_MSG) {
			if (mps_AMPcancel (endp->e_chan, src_socid, tid) == -1) {
				DEBUGS (DBG_ERR, ("mb2t_msg_ind_comp: Cancel failed\n"));
			}
		}
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, 
						(mblk_t *)NULL, 
						(struct dma_buf *)NULL, pb->pb_imptr, 
						(struct dma_buf *)NULL, 0, pb);
		return;
	}
	mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, (mblk_t *)NULL, 
				(struct dma_buf *)NULL, 0, pb);
	mb2t_stat[NO_MSGS_RECV]++;
	DEBUGS (DBG_CALL, ("mb2t_msg_ind_comp: ==> 0\n"));
	return;
}


static int
mb2t_send_reqfrag_msg (mbp, endp)
mps_msgbuf_t *mbp;
end_point *endp;
{
	mblk_t *mptr;
	mb2socid_t socid;
	unchar tid;

	DEBUGS (DBG_CALL, ("mb2t_send_reqfrag_msg () \n"));
	/*
	 * this routine can only be called for a fresh incoming request
	 * and the assumption is that buffers are unavailable.
	 * and the message type is a buffer request.
	 */
	socid = mps_mk_mb2socid (mps_msg_getsrcmid (mbp), mps_msg_getsrcpid (mbp));
	tid = mps_msg_gettrnsid(mbp);

	mptr = mb2t_getmptr((ulong)(sizeof(struct mb2_msg_ind)+(mbp->mb_count- MPS_MG_BROVHD)),
						BPRI_HI,endp);
	if(mptr == NULL) {
		DEBUGS(DBG_ERR,("mb2t_send_reqfrag_msg: alloc failed\n"));
		if (mps_AMPcancel (endp->e_chan, socid, tid) == -1) {
			DEBUGS(DBG_ERR,("mb2t_send_reqfrag_msg: Cancel failed\n"));
		}
		return(0);
	}

	mb2t_fill_msg (mptr, MB2_REQFRAG_MSG, socid, tid, 
				mbp->mb_bind, mps_msg_getbrlen(mbp), (ulong)(mbp->mb_count - MPS_MG_BROVHD), 
				(char *) mps_msg_getudp (mbp));
	if (mb2t_can_send_upstr (mptr, endp) == 0) {
		if (mps_AMPcancel (endp->e_chan, socid, tid) == -1) {
			DEBUGS(DBG_ERR,("mb2t_send_reqfrag_msg: Cancel failed\n"));
		}
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, 
				(mblk_t *)NULL, 
				(struct dma_buf *)NULL, mptr, 
				(struct dma_buf *)NULL, 0, (struct mb2_prim_bind *)NULL);
		return(0);
	}
	mb2t_stat[NO_FRAGMENTED]++;
	DEBUGS (DBG_CALL, ("mb2t_send_reqfrag_msg () \n"));
	return(1);
}


static void
mb2t_send_status_msg (endp, socket, tid, ref)
end_point *endp;
mb2socid_t socket;
unchar tid;
ulong ref;
{
	mblk_t *mptr;

	DEBUGS (DBG_CALL, ("mb2t_send_status_msg () \n"));
	mptr = mb2t_getmptr(sizeof(struct mb2_msg_ind),BPRI_HI,endp);
	if(mptr == NULL) {
		DEBUGS(DBG_ERR,("mb2t_send_status_msg: alloc failed\n"));
		return;
	}
	mb2t_fill_msg (mptr, MB2_STATUS_MSG, socket, 
				tid, ref, 0, 0, NULL); 
	mb2t_putq (mptr, endp);
	DEBUGS (DBG_CALL, ("mb2t_send_status_msg () \n"));
	return;
}




static struct mb2_prim_bind *
mb2t_get_unsol_buf (src_socid, type, tid, ref_value, endp, ctllen, ctlbuf)
mb2socid_t src_socid;
ulong type;
unchar tid;
ulong ref_value;
end_point *endp;
ulong ctllen;
char *ctlbuf;
{
	register mblk_t *mptr;
	struct mb2_prim_bind *pb;

	DEBUGS(DBG_CALL,("mb2t_get_unsol_buf()\n"));
	mptr = mb2t_getmptr((ulong)(sizeof(struct mb2_msg_ind)+ctllen),
					BPRI_MED,endp);
	if (mptr == NULL) {
		DEBUGS(DBG_ERR,("mb2t_get_unsol_buf: alloc1 failed\n"));
		return (NULL);
	}
	if ((pb = mb2t_alloc_pb (endp)) == NULL) {
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, (mblk_t *)NULL,
 					(struct dma_buf *)NULL, mptr, 
					(struct dma_buf *)NULL, 0, (struct mb2_prim_bind *)NULL);
		return (NULL);
	}
	mb2t_fill_msg (mptr, type&~MB2_INCOMING, src_socid, tid, 
				ref_value, 0, ctllen, ctlbuf);
	mb2t_fill_pb (pb, type, 0, tid, 0,
			 endp, (mblk_t *)NULL, (struct dma_buf *)NULL, mptr, (struct dma_buf *)NULL);
	DEBUGS (DBG_CALL, ("mb2t_get_unsol_buf: ==> pb = %x\n", pb));
	return(pb);
}



static struct mb2_prim_bind *
mb2t_get_sol_buf(src_socid, type, tid, ref_value, endp, ctllen, ctlbuf, datalen, databuf)
mb2socid_t src_socid;
ulong type;
unchar tid;
ulong ref_value;
end_point *endp;
ulong ctllen;
char *ctlbuf;
ulong datalen;
struct dma_buf **databuf;
{
	register mblk_t *mptr, *dmptr;
	struct mb2_prim_bind *pb;

	DEBUGS(DBG_CALL,("mb2t_get_sol_buf()\n"));
#ifdef OLDWAY	/* Workaround for a kernel bug which has been fixed? */
	/*
	 * we first check if the datalen > strmaxmsgsz. If so we need to
 	 * return NULL. because the max msgsz is defined to be the limit
	 * Note this is a kludge to get around the fact that the stream
	 * head can read messages > strmsgsz but cannot write messages >
	 * strmsgsz.
	 */
	if (datalen >= strmsgsz) {
		cmn_err (CE_NOTE,"mb2t_get_sol_buf: datalen > strmsgsz");
		return (NULL);
	}
#endif

	mptr = mb2t_getmptr((ulong)(sizeof(struct mb2_msg_ind)+ctllen),
				BPRI_MED,endp);
	if(mptr == NULL) {
		DEBUGS(DBG_ERR,("mb2t_get_sol_buf: alloc1 failed\n"));
		return (NULL);
	}
	if ((pb = mb2t_alloc_pb (endp)) == NULL) {
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, (mblk_t *)NULL, 					(struct dma_buf *)NULL, mptr, 
					(struct dma_buf *)NULL, 0, (struct mb2_prim_bind *)NULL);
		return (NULL);
	}
	*databuf = NULL;

	if ((dmptr = mb2t_get_strbuf (datalen, BPRI_MED)) == NULL) {
		DEBUGS (DBG_ERR, ("mb2t_get_sol_buf: alloc2 failed\n"));
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, 
				(mblk_t *) NULL, (struct dma_buf *)NULL, mptr,
				(struct dma_buf *)NULL, 0, pb);
		return (NULL);
	}
	linkb (mptr, dmptr);
	if ((*databuf = mb2t_mk_dbuf (mptr)) == NULL) {
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, 
					(mblk_t *)NULL, 
				 	(struct dma_buf *)NULL, mptr, 
					(struct dma_buf *)NULL, 0, pb);
		return (NULL);
	}
	mb2t_fill_msg (mptr, type&~MB2_INCOMING, src_socid, tid, 
				ref_value, datalen, ctllen, ctlbuf);
	mb2t_fill_pb (pb, type, 0, tid, datalen,
			 endp, (mblk_t *)NULL, (struct dma_buf *)NULL, mptr, (struct dma_buf *)NULL);
	DEBUGS (DBG_CALL, ("mb2t_get_sol_buf: ==> pb = %x\n", pb));
	return (pb);
}


static void
mb2t_handle_undelmsg(endp, mbp)
end_point *endp;
mps_msgbuf_t *mbp;
{
	unchar msgtyp;
	unchar srcmid;
	unchar tid;
	ushort srcpid;
	unchar lsnid;


	DEBUGS (DBG_CALL, ("mb2t_handle_undelmsg ()\n"));
	mb2t_stat[NO_MSGS_UNDEL]++;
	
	msgtyp = mps_msg_getmsgtyp(mbp);
	srcmid = mps_msg_getsrcmid(mbp);
	tid = mps_msg_gettrnsid(mbp);
	srcpid = mps_msg_getsrcpid(mbp);
	lsnid = mps_msg_getlsnid(mbp);


	if(tid == 0) {
		if(msgtyp == MPS_MG_BREQ) {
			mps_mk_breject(mbp,mps_mk_mb2socid(srcmid,srcpid),lsnid);
			if (mps_AMPsend(endp->e_chan, mbp) == -1) {
				cmn_err (CE_WARN, "mb2t_handle_undelmsg: Send buffer reject failed"); 
			}
		} else {
			/*
		 	 * dont do anything in the case of unsol or brdcst msgs
		 	 */
			mps_free_msgbuf (mbp);
		}
	} else {
		if(mps_msg_isreq(mbp)) {
			if (mps_AMPcancel(endp->e_chan, mps_mk_mb2socid(srcmid, srcpid), tid) == -1) {
				cmn_err (CE_WARN, "mb2t_handle_undelmsg: Send buffer reject failed"); 
			}
			mps_free_msgbuf (mbp);
		} else {
			if(msgtyp == MPS_MG_BREQ) {
				mps_mk_breject(mbp,mps_mk_mb2socid(srcmid,srcpid),lsnid);
				if (mps_AMPsend(endp->e_chan, mbp) == -1) {
					cmn_err (CE_WARN, "mb2t_handle_undelmsg: Send buffer reject failed"); 
				}
			} else {
				cmn_err (CE_WARN, "mb2t_handle_undelmsg: Unknown message");
				mps_free_msgbuf (mbp);
			}
			
		}
	}
	DEBUGS (DBG_CALL, ("mb2t_handle_undelmsg () ==> NULL\n"));
	return;
}



/* ARGSUSED */
static mblk_t *
mb2t_getmptr(size, pri, endp)
ulong	size;
uint	pri;
end_point *endp;
{
	mblk_t	*mptr;

	DEBUGS(DBG_CALL,("mb2t_getmptr ()\n"));
	if((mptr = allocb((int)size, pri)) == NULL) {
		DEBUGS(DBG_ERR,("mb2t_getmptr: alloc failed\n"));
		return (mptr);
	}
	DEBUGS(DBG_CALL,("mb2t_getmptr => %x\n", mptr));
	*(unsigned short *)mptr->b_wptr = 0xffff;
	*(unsigned short *)(mptr->b_wptr+2) = size;
	if((mptr->b_datap->db_lim - mptr->b_datap->db_base) >= 28) {
		mblk_t **wptr = (mblk_t **)mptr->b_wptr;
		unsigned long *sp = (unsigned long *)&mptr;
		int i;

		wptr++;
		*wptr++ = mptr;
		sp++;
		for(i=0;i<5;i++) {
			sp++;
			*wptr++ = *(mblk_t **)sp;
			sp--;
			if(sp < *(unsigned long **)sp)
				break;
			sp = *(unsigned long **)sp;
		}
	}
	DEBUGS(DBG_CALL,("mb2t_getmptr => %x\n", mptr));
	return(mptr);
}


static end_point *
mb2t_getendp (hostid, portid, msgtype)
ushort hostid;
ushort portid;
unchar msgtype;
{
	end_point *endp;
	mb2socid_t socket;
	if (msgtype == MPS_MG_BRDCST)
		/*
	 	 * this is a special kludge for broadcast messages
		 */
		hostid = ics_myslotid();
	socket = mps_mk_mb2socid (hostid, portid);

	for  (endp = &mb2t_endps[0]; endp < &mb2t_endps[mb2t_no_endps]; endp++) {
		if (endp->e_strstate == S_IDLE)
			continue;
		if (endp->e_socket != 0)
			DEBUGS (DBG_FULL, ("endp->socket = %x socid = %x\n", endp->e_socket, socket));
		if (endp->e_socket == socket) 
			return (endp);
	}
	return (NULL);
}



/* ARGSUSED */
static void 
mb2t_do_ioctl_msg (wr_q, mptr, endp)
queue_t *wr_q;
mblk_t *mptr;
end_point *endp;
{
 	struct iocblk *iocbp;

	DEBUGS (DBG_CALL, ("mb2t_do_ioctl_msg ()\n"));
	/* 
	 * right now no ioctls are supported by the driver
	 */
 	iocbp = (struct iocblk *)mptr->b_rptr;
	mptr->b_datap->db_type = M_IOCNAK;
	iocbp->ioc_error = EPROTO;
	qreply(wr_q, mptr);
	DEBUGS (DBG_CALL, ("mb2t_do_ioctl_msg ==> 0\n"));
	return;
	
}



static void 
mb2t_fill_pb (pb, flag, ref_value, tid, datalen, endp, mptr, odbuf,imptr, idbuf)
struct mb2_prim_bind *pb;
ulong flag;
ulong ref_value;
unchar tid;
ulong datalen;
end_point *endp;
mblk_t *mptr;
struct dma_buf *odbuf;
mblk_t *imptr;
struct dma_buf *idbuf;
{
	pb->pb_flags = flag;
	pb->pb_uref = ref_value;
	pb->pb_tid = tid;
	pb->pb_datalen = datalen;
	pb->pb_endp = endp;
	pb->pb_mptr = mptr;
	pb->pb_odbuf = odbuf;
	pb->pb_imptr = imptr;
	pb->pb_idbuf = idbuf;
}

static mblk_t *
mb2t_get_strbuf (datalen, pri)
ulong datalen;
uint pri;
{
	mblk_t *dmptr, *tdmptr;
	int class_size;
	ulong count;

	DEBUGS (DBG_CALL, ("mb2t_get_strbuf () datalen = %d\n", datalen));
	count = datalen;
	dmptr = tdmptr = NULL;
	do {
		class_size = mb2t_getsclass (count);
		DEBUGS (DBG_FULL, ("mb2t_get_strbuf: classsize= %d\n", class_size));
		if ((tdmptr = allocb (class_size, pri)) == NULL) {
			DEBUGS (DBG_FULL, ("mb2t_get_strbuf: allocb failed\n"));
			if (dmptr != NULL)
				freemsg (dmptr);
			return (NULL);
		}
		tdmptr->b_datap->db_type = M_DATA;
		tdmptr->b_wptr = tdmptr->b_rptr + class_size;
		if (dmptr != NULL)
			linkb (dmptr, tdmptr);
		else
			dmptr = tdmptr;
		/*
		 * the following is a slight kludge because count is ulong
		 */
		if ((long)(count-class_size) < 0) {
			count = 0;
		} else
			count -= class_size;
	} while (count != 0);
	return (dmptr);
}

	
static int
mb2t_getsclass (len)
ulong len;
{
#define K 1024
	return ((len > (4*K) ? (4*K) : len));
}


static struct dma_buf *
mb2t_mk_dbuf (mptr)
mblk_t *mptr;
{
	mblk_t *nextp, *dptr;
	unchar	*datap;
	struct dma_buf *mb2t_datbufp;
	ulong len;

	dptr = mptr->b_cont;
	mb2t_datbufp = NULL;
	while(dptr != NULL) {
		nextp = dptr->b_cont;
		datap = dptr->b_datap->db_base;
		len = dptr->b_wptr - dptr->b_rptr;
		if(len != 0) {
			/* build a databuffer chain */

#ifdef OLDWAY
			if ((mb2t_datbufp = mb2t_mk_datbuf (mb2t_datbufp, 
						(paddr_t) kvtophys ((caddr_t)datap), 
						len, 0)) == NULL) {
#endif
			if ((mb2t_datbufp = mb2t_mk_datbuf (mb2t_datbufp, 
						(caddr_t)datap,
						len, 0)) == NULL) {
				/* free datbuf if not null */
				if (mb2t_datbufp != NULL)
					mps_free_dmabuf (mb2t_datbufp);
					
				return (NULL);
			} 
				
		}
		dptr = nextp;
	}
	return (mb2t_datbufp);
}



/* ARGSUSED */
static struct dma_buf *
mb2t_mk_datbuf(head, addr, len, slpflag)
struct dma_buf *head;
caddr_t addr;
ulong len;
ulong slpflag;
{
/* #define MAX_ONEDATBUF 0xFFFF */
#define MAX_ONEDATBUF 0x1000
	struct dma_buf *start, *newtail, *tail;

	if(len == 0)
		return(head);
	/* create the head node */
	start = mps_get_dmabuf(1,DMA_NOSLEEP);
	if( start == NULL)
		return(NULL);
	newtail = start;
	/* if data len > MAX_ONEDATBUF, buffer has to be broken up */
	while( len > MAX_ONEDATBUF ) {
		newtail->address = (paddr_t)kvtophys (addr);
		newtail->count = MAX_ONEDATBUF;
		addr += MAX_ONEDATBUF;
		len -= MAX_ONEDATBUF;
		newtail->next_buf = mps_get_dmabuf(1,DMA_NOSLEEP);
		newtail = newtail->next_buf;
		if(newtail == NULL) {
			mps_free_dmabuf(start);
			return(NULL);
		}
	}
	/* fix the last node in the new list */
	newtail->address = (paddr_t) kvtophys(addr);
	newtail->count = len;

	/* attach to the old list if there is one */
	if( head == NULL ) {
		/* no old list */
		head = start;
		/* create the tail null node */
		newtail->next_buf = mps_get_dmabuf(1,DMA_NOSLEEP);
		newtail = newtail->next_buf;
		if(newtail == NULL) {
			mps_free_dmabuf(start);
			return(NULL);
		}
		newtail->address = NULL;
		newtail->count = 0;
		newtail->next_buf = NULL;
	} else {
		/* scan to the last but one node in old list */
		for(tail=head;tail->next_buf->count!=0;tail=tail->next_buf);
		/* insert new list at last but one node */
		newtail->next_buf = tail->next_buf;
		tail->next_buf = start;
	}
	return(head);
}

/*
 * free the respective resources and if stream message buffers are
 * freed and the stream is blocked by flow control manualy enable
 * the write service procedure
 */

static void
mb2t_free_resource (endp, mbp, mptr, odbuf, imptr, idbuf, tid, pb)
end_point *endp;
mps_msgbuf_t *mbp;
mblk_t *mptr;
struct dma_buf *odbuf;
mblk_t *imptr;
struct dma_buf *idbuf;
unchar tid;
struct mb2_prim_bind *pb;
{
	int i;
	end_point *tep;

	if (mbp)
		mps_free_msgbuf (mbp);
	if (mptr) {
		freemsg (mptr);
	}
	if (odbuf)
		mps_free_dmabuf (odbuf);
	if (imptr) {
		freemsg (imptr);
	}
	if (idbuf)
		mps_free_dmabuf (idbuf);
	if (tid) {
		mps_free_tid (endp->e_chan, tid);
		/*
		 * We enable this stream. Since tid's are allocated and
		 * freed per stream basis.
		 */
		if (endp->e_taistate & T_WAITTID) {
			endp->e_taistate &= ~T_WAITTID;
			qenable (WR(endp->e_rdq));
		}
	}
	if (pb) {
		mb2t_free_pb (endp, pb);
		/* 
		 * we also need to check if there are any streams that
		 * are waiting for this resource. If so we enable the 
		 * service procedure for that stream.
		 */
		if (mb2t_need_pb != 0) {
			for (i=0; i<mb2t_no_endps; i++) {
				tep = &mb2t_endps[i];
				if (tep->e_taistate & T_WAITPBIND) {
					if (tep->e_taistate & S_IDLE)
				    	     cmn_err (CE_PANIC, "Invalid endpoint state");
					tep->e_taistate &= ~T_WAITPBIND;
					mb2t_need_pb--;
					qenable (WR (tep->e_rdq));
					break;
				}
			}
		}
	}
}


static void
mb2t_fill_msg (mptr, type, socket, tid, ref_value, datalen, ctllen, ctlbuf)
mblk_t *mptr;
ulong type;
mb2socid_t socket;
unchar tid;
ulong ref_value;
ulong datalen;
ulong ctllen;
char *ctlbuf;
{
	struct mb2_msg_ind *msg_ind;

	mptr->b_datap->db_type = M_PROTO;
	msg_ind = (struct mb2_msg_ind *)mptr->b_rptr;
	msg_ind->PRIM_type = type; 
	msg_ind->TRAN_id = tid;
	msg_ind->REF_value = ref_value;
	msg_ind->SRC_addr = (long) socket;
	msg_ind->CTRL_length = ctllen;
	msg_ind->CTRL_offset = sizeof (struct mb2_msg_ind);
	msg_ind->DATA_length = datalen;
	if (msg_ind->CTRL_length)
		memcpy(((char *)mptr->b_rptr+msg_ind->CTRL_offset), ctlbuf,
	        	(int)msg_ind->CTRL_length);
	mptr->b_wptr= mptr->b_rptr + sizeof(struct mb2_msg_ind)  + msg_ind->CTRL_length;

}

static void
mb2t_rsvp_tout (pb)
struct mb2_prim_bind *pb;
{
	end_point *endp;

	DEBUGS (DBG_CALL, ("mb2t_rsvp_tout ()\n"));
	mb2t_stat[NO_RSVP_TOUT]++;
	endp = pb->pb_endp;
	if (mps_AMPcancel (endp->e_chan, endp->e_socket, (unchar)pb->pb_tid) == -1) {
		cmn_err (CE_NOTE, "mb2t_rsvp_tout: mps_AMPcancel failed tid = %x socket = %x", pb->pb_tid, endp->e_socket);
		DEBUGS (DBG_ERR, ("mb2t_rsvp_tout: mps_AMPcancel failed\n"));
		mb2t_send_status_msg (endp, endp->e_socket, 
					(unchar)pb->pb_tid, pb->pb_uref); 
		mb2t_free_resource (endp, (mps_msgbuf_t *) NULL, 
						pb->pb_mptr, 
						(struct dma_buf *)NULL, pb->pb_imptr, 
						(struct dma_buf *)NULL, 
						(unchar)pb->pb_tid, pb);
	}
	/*
	 * the interrupt routine will free resources when the request
	 * cancelled completes.
	 */
	DEBUGS (DBG_CALL, ("mb2t_rsvp_tout () ==> NULL\n"));
	return;
}


static void
mb2t_frag_tout (pb)
struct mb2_prim_bind *pb;
{
	end_point *endp;
	struct mb2_frag_req *frag_req;
	mb2socid_t dsocid;

	DEBUGS (DBG_CALL, ("mb2t_frag_tout ()\n"));
	mb2t_stat[NO_FRAG_TOUT]++;
	endp = pb->pb_endp;
	/*
	 * we need to look at the message because in this case
	 * cancel requires a destination socket
	 */
	frag_req = (struct mb2_frag_req *) pb->pb_mptr->b_rptr;
	dsocid = (mb2socid_t)frag_req->DEST_addr;
	if (mps_AMPcancel (endp->e_chan, dsocid, (unchar)pb->pb_tid) == -1) {
		cmn_err (CE_NOTE, "!mb2t_frag_tout: mps_AMPcancel failed");
		DEBUGS (DBG_ERR, ("mb2t_frag_tout: mps_AMPcancel failed\n"));
		mb2t_send_status_msg (endp, endp->e_socket, 
					(unchar)pb->pb_tid, pb->pb_uref); 
		mb2t_free_resource (endp, (mps_msgbuf_t *)NULL, pb->pb_mptr, 
						(struct dma_buf *)NULL, pb->pb_imptr,
						(struct dma_buf *)NULL, 0, pb);
	}
	/*
	 * the interrrupt routine will free the resources when the request
	 * for which the cancel is issued completes
	 */
	DEBUGS (DBG_CALL, ("mb2t_frag_tout () ==> NULL\n"));
	return;
}

static struct mb2_prim_bind *
mb2t_alloc_pb (endp)
end_point *endp;
{
	struct mb2_prim_bind *pb;
	int s;

	DEBUGS (DBG_CALL, ("mb2t_alloc_pb () ==> NULL\n"));
	s = SPL ();
	if (mb2t_pb_freelist == NULL) {
		splx (s);
		return (NULL);
	}
	pb = mb2t_pb_freelist;
	mb2t_pb_freelist = mb2t_pb_freelist->pb_free;

	pb->pb_next = endp->e_pb.pb_prev->pb_next;
	endp->e_pb.pb_prev->pb_next = pb;
	pb->pb_prev = endp->e_pb.pb_prev;
	endp->e_pb.pb_prev = pb;

	pb->pb_flags = 0; pb->pb_uref = 0;
	pb->pb_tid = 0; pb->pb_datalen = 0; pb->pb_tval = 0;
	pb->pb_endp =  NULL;
	pb->pb_mptr = NULL; 
	pb->pb_odbuf = NULL;
	pb->pb_mptr = NULL;
	pb->pb_idbuf = NULL;
	splx (s);
	DEBUGS (DBG_CALL, ("mb2t_alloc_pb ()  pb = %x\n", pb));
	return (pb);
}

/* ARGSUSED */
static void
mb2t_free_pb (endp, pb)
end_point *endp;
struct mb2_prim_bind *pb;
{
	int s;

	DEBUGS (DBG_CALL, ("mb2t_free_pb ()  pb = %x\n", pb));
	s = SPL ();
	/* take it off the endpoint's busy list */

	pb->pb_prev->pb_next = pb->pb_next;
	pb->pb_next->pb_prev = pb->pb_prev;

	/* add it to the free pool */

	if (mb2t_pb_freelist == NULL) {
		pb->pb_free = NULL;
		mb2t_pb_freelist = pb;
	} else {
		pb->pb_free = mb2t_pb_freelist;
		mb2t_pb_freelist = pb;
	}
	DEBUGS (DBG_CALL, ("mb2t_free_pb ()  pb = %x\n", pb));
	splx (s);
	return;
}

static ushort
mb2t_alloc_port ()
{
	int i;
	for (i=0; i< mb2t_no_endps; i++) {
		if (mb2t_ports[i] == 0) {
			mb2t_ports[i] = mb2t_first_port + i;
			DEBUGS (DBG_CALL, ("allocated port %d\n", mb2t_ports[i])); 
			return (mb2t_ports[i]);
		}
	}
	return (0);
}

static void
mb2t_free_port (port)
ushort port;
{
	ushort intport;
	
	intport = port - mb2t_first_port;
	if (intport >= (unsigned int)mb2t_no_endps)
		return;
	mb2t_ports[intport] = 0;
 	DEBUGS (DBG_FULL, ("freed port %d\n", port));
	return;	
	
}


static void
mb2t_init_endp (endp)
end_point *endp;
{

	endp->e_taistate = 0;
	endp->e_strstate = S_IDLE;
	endp->e_socket = 0;
	endp->e_chan = 0;			
	endp->e_rsvptout = MB2_DEF_RSVPTOUT;
	endp->e_fragtout = MB2_DEF_RSVPTOUT;
	endp->e_pb.pb_next = &(endp->e_pb);
	endp->e_pb.pb_prev = &(endp->e_pb);
}

static void 
mb2t_init_pb ()
{
	int i;
	struct mb2_prim_bind *pb;

	for (i=0; i< mb2t_maxmsgs; i++) {
		pb = &mb2t_pbind[i];
		pb->pb_flags = 0; pb->pb_uref = 0;
		pb->pb_tid = 0; pb->pb_datalen = 0; pb->pb_tval = 0;
		pb->pb_endp =  NULL;
		pb->pb_mptr =  NULL;
		pb->pb_odbuf = NULL;
		pb->pb_mptr = NULL;
		pb->pb_idbuf = NULL;
		if (i < (mb2t_maxmsgs - 1))
			pb->pb_free = &mb2t_pbind [i+1];
	}
	mb2t_pbind[mb2t_maxmsgs-1].pb_free = NULL;
	mb2t_pb_freelist = &mb2t_pbind[0];

}

static void
mb2t_await_resource (endp, flag)
end_point *endp;
int flag;
{
	if (flag & T_WAITPBIND)
		mb2t_need_pb++;
	endp->e_taistate |= flag;
}


/*
 * this routine is called when sending rsvps or frag_reqs only. hence the
 * the status message.
 */

static int
mb2t_do_bufcall (endp, msize, tid, ref)
end_point *endp;
ulong msize;
unchar tid;
ulong ref;
{
	if (bufcall ((uint)msize, BPRI_LO, qenable, WR(endp->e_rdq)) == 0) {
		cmn_err (CE_WARN, "!mb2t_do_bufcall: bufcall failed");
		mb2t_send_status_msg (endp, endp->e_socket, tid, ref);
		return (1);
	}
	return (0);
}
