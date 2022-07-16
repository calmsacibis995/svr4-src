/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988, 1989, 1990 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/mps.c	1.3.3.1"

#ifndef lint
static char mps_copyright[] = "Copyright 1986, 1987, 1988, 1989, 1990 Intel Corporation 460950";
#endif /* lint */

#include "sys/types.h"
#include "sys/mps.h"
#include "sys/mpc.h"
#include "sys/dma.h"
#include "sys/ics.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include <sys/systm.h>
#include "sys/ddi.h"

/*
 *	If you invoke this define (SNFCHECK) , you will get some code that
 *	breaks to the monitor if an unexpected EOT is received.
 */
/* #define SNFCHECK */
static void mps_handle_err(mps_msgbuf_t *, unsigned char);
static int mps_msg_que(mps_msgbuf_t *, unsigned short);
static int mps_find_port(unsigned short);

extern port_t mps_port_defs[];
extern tinfo_t mps_tinfo[];
extern unsigned char mps_t_ids[];
extern int mps_max_port;
extern unsigned short mps_port_ids[];
extern msgque_t mps_prioq[];
extern long mps_buf_count();
extern int mps_ckprio;
extern int mps_max_tran;

static unsigned char requestid = 0;
unsigned char
mps_requestid()
{
	return(requestid++);
}

/*
 * Used to send an unsolicited message. (ie: buffer reject, non-rsvp message..)
 * Operates synchronously so no completion message returned to user.
 */
long
mps_AMPsend(chan,mbp)
long chan;
register mps_msgbuf_t *mbp;  /* built using mps_mk_unsol, mps_mk_breject, or mps_mk_brdcst */
{
	if(chan<0 || chan>mps_max_port || mps_port_ids[chan] == 0)
		return(-1);
	mps_msg_setsrcpid(mbp, mps_port_ids[chan]);
#ifdef DEBUG
	if(mps_msg_getprotid(mbp) != MG_MTDT ||
	  (mps_msg_getmsgtyp(mbp) != MPS_MG_UNSOL && mps_msg_getmsgtyp(mbp)!= MPS_MG_BRDCST)){
	  	cmn_err(CE_WARN,"bad message in mps_AMPsend()\n");
	  	monitor();
	}
#endif
	mps_msg_setrid(mbp);
	if( impc_send(mbp) == -1)
		return(-1);
	mps_free_msgbuf(mbp);
	return(0);
}


/*
 * Used to send an rsvp message. User has allocated the transaction entry.
 * Completion message returned (when service is complete) is:
 * last buffer request (eot set) received by server if data response;
 * unsol msg (eot set) received by server if control message response.
 *
 *      find transaction entry
 *      fill in remaining fields in entry
 *      if data message
 *          queue up transaction entry for solicited output
 *      else (control message)
 *          send message synchronously
 *          enter request complete state
 *      endif
 */

long
mps_AMPsend_rsvp(chan, omsg, obuf, ibuf)
long chan;
register mps_msgbuf_t *omsg;  /* built using mps_mk_unsol (no data) or mps_mk_sol (data) */
struct dma_buf *obuf;           /* outgoing data buffer; NULL if no data */
struct dma_buf *ibuf;           /* incoming data buffer; NULL if no data */
{
	register tinfo_t *tp;
	int i;
	int s;
	long ret = 0;

	if(chan<0 || chan>mps_max_port || mps_port_ids[chan] == 0)
		return(-1);
	mps_msg_setsrcpid(omsg,mps_port_ids[chan]);
#ifdef DEBUG
	if(mps_msg_gettrnsid(omsg) == 0 || mps_msg_getprotid(omsg) != MG_MTDT ||
	  (mps_msg_getmsgtyp(omsg) != MPS_MG_UNSOL && mps_msg_getmsgtyp(omsg)!= MPS_MG_BREQ)){
	  	cmn_err(CE_WARN,"bad message in mps_AMPsend_rsvp()\n");
	  	monitor();
	}
#endif
	s = splhi();
	i = mps_find_transaction(mps_mk_mb2socid(mps_lhid(),mps_port_ids[chan]),mps_msg_gettrnsid(omsg), MG_LOCTRN);
	if(i == -1) {
		splx(s);
		return(-1);
	}
	tp = &mps_tinfo[i];
	/* The entry must be in init state and t_omsg must be
	 * NULL. t_omsg != NULL implies a previously started 
	 * rsvp request for the same transaction has not
	 * completed yet
	 */
	if(tp->t_state != MG_INITST || tp->t_omsg != NULL) {
		splx (s);
		return(-1);
	}
	tp->t_rsocid = mps_mk_mb2socid(mps_msg_getdstmid(omsg),mps_msg_getdstpid(omsg));
	tp->t_flags = MG_RSVP;
	splx (s);
	tp->t_ocnt = tp->t_orem = mps_buf_count(obuf);
	tp->t_obuf = obuf;
	tp->t_icnt = tp->t_irem = mps_buf_count(ibuf);
	tp->t_ibuf = ibuf;
	tp->t_omsg = omsg;
	tp->t_imsg = NULL;
	

	if(tp->t_ocnt) {        /* sending a data msg; remain in MG_INITST */
#ifdef DEBUG
		if(mps_msg_getmsgtyp(omsg) != MPS_MG_BREQ) {
			cmn_err(CE_WARN,"bad message type in mps_AMPsend_rsvp\n");
			monitor();
		}
#endif
		mps_msg_setbrlen(omsg,tp->t_ocnt);
		impc_sol_que(tp,SOC);
	} else {                      /* send control message synchronously */
		s = splhi();		
		if(impc_send(omsg) == -1) {
			ret = -1;
			/* Since we are returning -1, all resources are still
			 * controlled by the requestor. So do not free
			 * anything, just reset the fields in table
			 */
			tp->t_rsocid = 0;
			tp->t_ocnt = tp->t_orem = 0;
			tp->t_obuf = NULL;
			tp->t_icnt = tp->t_irem = 0;
			tp->t_ibuf = NULL;
			tp->t_omsg = NULL;
		} else
			tp->t_state = MG_RS_RC; /* request sent; await resp */
		splx(s);		
	}
	return(ret);
}

/* 
 * Used to receive data corresponding to buffer request previously received.
 * The buffer request corresponds to an rsvp or non-rsvp request.
 * omsg contains a buffer grant. This buffer grant corresponds to
 * the buffer request received. ibuf is where data is stored.
 * Completion msg returned (when the service is complete) is the buffer grant.
 *
 * *** Currently we store a tid of 0 in the transaction entry. The tid is not
 *     0 if an rsvp request was previously issued. For an rsvp request,
 *     the user will not be able to locally cancel this transaction as the
 *     entry can not be found without its proper tid.
 *
 * 	allocate an entry in the "remote" transaction table
 *	fill in the fields in entry
 *      fix up length field in message (omsg)
 *	queue the transaction entry for solicited input
 */

long
mps_AMPreceive(chan, dsocid, omsg, ibuf)
long chan;
mb2socid_t dsocid;
mps_msgbuf_t *omsg;           /* build using mps_mk_bgrant */
struct dma_buf *ibuf;           /* buffer to receive data */
{
	int i, s;
	register tinfo_t *tp;

	if(chan<0 || chan>mps_max_port || mps_port_ids[chan] == 0 )
		return(-1);
#ifdef DEBUG
	if(mps_msg_getmsgtyp(omsg) != MPS_MG_BGRANT || ibuf == NULL) {
		cmn_err(CE_PANIC, "bad message type in mps_AMPreceive\n");
	}
#endif
	s = splhi();
	i = mps_find_transaction(0,0, MG_REMTRN);
	if(i == -1) {
		cmn_err(CE_WARN, "out of transaction entries\n");
		splx(s);	
		return(-1);
	}
	tp = &mps_tinfo[i];
	tp->t_icnt = mps_buf_count(ibuf);
	tp->t_ibuf = ibuf;
	tp->t_state = MG_INITST;
	tp->t_flags = MG_RCV;
	tp->t_lcid = chan;
	tp->t_lsocid = mps_mk_mb2socid(mps_lhid(),mps_port_ids[chan]);
	tp->t_rsocid = dsocid;
	splx(s);	
	tp->t_omsg = omsg;
	mps_msg_setbglen(omsg, MPS_LOB(LOW(tp->t_icnt)));
	mps_t_ids[i] = 0;     /* Note- should be remote tid for rsvp request */
	tp->t_ocnt = 0;
	tp->t_imsg = NULL;
	tp->t_obuf = NULL;
	tp->t_irem = 0;
	tp->t_orem = 0;
	impc_sol_que(tp,SIC);
	return(0);
}


/*
 * Used to received a fragment of an rsvp request.
 * The message being sent out here is going to be a send next fragment
 * message. The only reason we ask the user to pass us a message
 * buffer is so that he can tell us the value of mb_bind that he
 * wants returned in the completion message. Note, if count in send next
 * fragment message is 0, the service is completed after sending the message.
 * Completion message returned is: snf msg if count is 0; else buffer grant 
 * (sent when we receive buffer request next frag msg).
 *
 *    allocate an entry in "remote" transaction table
 *    fill in entry
 *    fix up send next fragment message (omsg)
 *    send message synchronously
 *    if fragment count is 0
 *        call mps_msg_comp() to complete the service
 *    endif
 */

long
mps_AMPreceive_frag(chan, omsg, dsocid, tid, ibuf)
long chan;
mps_msgbuf_t *omsg;   /* msg buffer used to make snf msg */
mb2socid_t dsocid;
unsigned char tid;
struct dma_buf *ibuf;            /* buffer to receive data (fragment) */
{
	int i, s;
	register tinfo_t *tp;
	unsigned long fragcount;

	if(chan<0 || chan>mps_max_port || mps_port_ids[chan] == 0 || tid == 0) {
#ifdef XDEBUG
		cmn_err(CE_CONT,"mps_AMPreceive_frag: bad parms; chan: %d mps_port_ids[chan]: %d tid: %u",chan,mps_port_ids[chan],tid);
#endif
		return(-1);
	}
	if( ibuf == NULL)
		fragcount = 0;
	else
		fragcount = mps_buf_count(ibuf);

	s = splhi();	
	i = mps_find_transaction(0,0, MG_REMTRN);
	if(i == -1) {
		cmn_err(CE_WARN, "out of transaction entries\n");
		splx(s);
		return(-1);
	}
	tp = &mps_tinfo[i];
	mps_t_ids[i] = tid;
	tp->t_lcid = chan;
	tp->t_lsocid = mps_mk_mb2socid(mps_lhid(), mps_port_ids[chan]);
	tp->t_rsocid = dsocid;
	splx(s);	
	tp->t_flags = MG_FRAG;
	tp->t_omsg = omsg;
	tp->t_imsg = NULL;
	tp->t_obuf = NULL;
	tp->t_ocnt = 0;
	tp->t_orem = 0;
	tp->t_ibuf = ibuf;
	tp->t_icnt = tp->t_irem = fragcount;
	tp->t_state = MG_INITST;
	mps_mk_snf(omsg, dsocid, tid, fragcount);
	mps_msg_setsrcpid(omsg, mps_port_ids[chan]);
	mps_msg_setrid(omsg);

#ifdef XDEBUG
	cmn_err(CE_CONT,"mps_AMPreceive_frag: sending snf\n");
	cmn_err(CE_CONT,"msg:\n");
	mb2_showmsg(omsg);
#endif
	if(impc_send(omsg) == -1) {
		/* error */
		s = splhi();	
		tp->t_flags = 0;
		tp->t_lsocid = 0;
		tp->t_rsocid = 0;
		mps_t_ids[i] = 0;
		tp->t_lcid = 0;
		tp->t_omsg = NULL;
		tp->t_ibuf = NULL;
		tp->t_icnt = tp->t_irem = 0;
		splx(s);	
		return(-1);
	}
	/*  if zero count receive frag, signal completion */
	if(fragcount == 0) {
		s = splhi();	
		mps_msg_comp(tp,0);
		splx(s);	
	}
	return(0);
}

/*
 * Used to send a control or data message response corresponding to a
 * previously received rsvp request. If a data response, obuf may contain
 * a response fragment, or the entire response. Completion message returned
 * (if a data response) is outgoing buffer request (omsg).
 *
 *    if response is a control msg
 *        call mps_AMPsend to send response
 *    endif
 *    allocate an entry in "remote" transaction table
 *    fill in fields in entry
 *    fix up buffer request length
 *    queue up transaction entry for solicited output
 */

long
mps_AMPsend_reply(chan, omsg, obuf)
long chan;
register mps_msgbuf_t *omsg;     /* built using mps_mk_solrply or mps_mk_unsolrply */
struct dma_buf *obuf;              /* buffer of outgoing data; NULL if no data */
{
	register tinfo_t *tp;
	int i, s;

	if(chan<0 || chan>mps_max_port || mps_port_ids[chan] == 0 )
		return(-1);
	if((mps_msg_gettransctl(omsg)&MPS_MG_RRMSK) != MPS_MG_RES)
		return(-1);
#ifdef DEBUG
	if(obuf == NULL && mps_msg_getmsgtyp(omsg) != MPS_MG_UNSOL)
		cmn_err(CE_PANIC, "Parameter mismatch in send_reply\n");
#endif
	if(obuf == NULL)
		return(mps_AMPsend(chan,omsg));
	s = splhi();
	i = mps_find_transaction(0,0,MG_REMTRN);
	if(i == -1) {
		cmn_err(CE_WARN, "out of transaction entries\n");
		splx(s);
		return(-1);
	}
	mps_msg_setsrcpid(omsg,mps_port_ids[chan]);
	tp = &mps_tinfo[i];
	mps_t_ids[i] = mps_msg_gettrnsid(omsg); 
	tp->t_flags = MG_SRPLY;
	tp->t_state = MG_INITST;
	tp->t_lcid = chan;
	tp->t_lsocid = mps_mk_mb2socid(mps_lhid(),mps_port_ids[chan]);
	tp->t_rsocid = mps_mk_mb2socid(mps_msg_getdstmid(omsg), mps_msg_getdstpid(omsg));
	splx(s);	
	tp->t_omsg = omsg;
	tp->t_obuf = obuf;
	tp->t_ocnt = mps_buf_count(obuf);
	tp->t_orem = tp->t_ocnt;
	mps_msg_setbrlen(omsg,tp->t_ocnt);
	tp->t_icnt = tp->t_irem = 0;
	tp->t_imsg = NULL;
	tp->t_ibuf = NULL;
	impc_sol_que(tp,SOC);
	return(0);
}

/*
 * Used to send non-rsvp solicited message (tid = 0).
 *
 *    allocate an entry in "local" transaction table
 *    fill in fields in entry
 *    fix up buffer request length
 *    queue up transaction entry for solicited output
 */
long
mps_AMPsend_data(chan, omsg, obuf)
long chan;
register mps_msgbuf_t *omsg;   /* built using mps_mk_sol or mps_mk_unsol */
struct dma_buf *obuf;            /* buffer for outgoing data; NULL if no data */
{
	register tinfo_t *tp;
	int i, s;

	if(chan<0 || chan>mps_max_port || mps_port_ids[chan] == 0 )
		return(-1);
	if(mps_msg_gettrnsid(omsg) != 0)
		return(-1);
#ifdef DEBUG
	if(obuf == NULL || mps_msg_getmsgtyp(omsg) != MPS_MG_BREQ)
		cmn_err(CE_PANIC, "bad msg in send_data\n");
#endif
	s = splhi();	
	i = mps_find_transaction(0,0,MG_LOCTRN);
	if(i == -1) {
		cmn_err(CE_WARN, "out of transaction entries\n");
		splx(s);	
		return(-1);
	}
	mps_msg_setsrcpid(omsg,mps_port_ids[chan]);
	tp = &mps_tinfo[i];
	mps_t_ids[i] = mps_msg_gettrnsid(omsg); 
	tp->t_flags = MG_SDATA;
	tp->t_state = MG_INITST;
	tp->t_lcid = chan;
	tp->t_lsocid = mps_mk_mb2socid(mps_lhid(),mps_port_ids[chan]);
	tp->t_rsocid = mps_mk_mb2socid(mps_msg_getdstmid(omsg),mps_msg_getdstpid(omsg));
	splx(s);	
	tp->t_omsg = omsg;
	tp->t_obuf = obuf;
	tp->t_ocnt = mps_buf_count(obuf);
	tp->t_orem = tp->t_ocnt;
	mps_msg_setbrlen(omsg,tp->t_ocnt);
	tp->t_icnt = tp->t_irem = 0;
	tp->t_imsg = NULL;
	tp->t_ibuf = NULL;
	impc_sol_que(tp,SOC);
	return(0);
}

/* 
 * cancel an outstanding request
 * if dsocid indicates a local transaction
 * 	search for it in local transaction table
 *	if not found 
 *		return failure
 *	check to see if it is on head of a solicited queue
 *	if yes
 *		write to the appropriate cancel port
 *		return success( indication of cancel will be
 *				sent when solicited transfer 
 *				completion happens)
 *	search for it in the solicited queues
 *	if found
 *		remove from queues
 *		call mps_msg_comp as if an error occured in transfer
 *		return success
 *	return failure
 * else(it is a tran started by a remote host)
 *	send him a cancel
 *	if you are head of a solicited transfer
 *		write cancel to the appropriate port
 *		return success
 *	search for it in the solicited queues
 *	if found
 *		remove from queues
 *		call mps_msg_comp as if an error occured in transfer
 *		return success
 *	return success
 */

long
mps_AMPcancel(chan, dsocid, tid)
long chan;
mb2socid_t dsocid;
unsigned char tid;
{
	int i, s;
	register mps_msgbuf_t *mbp;
	register tinfo_t *tp;
	int status;

	s = splhi();		
	if( mps_mk_mb2soctohid(dsocid) == mps_lhid()) {
		/* local cancel */
		i = mps_find_transaction(dsocid,tid, MG_LOCTRN);
		if(i == -1) {
			splx (s);
			return (-1);
		}
		tp = &mps_tinfo[i];
		/* Check if the transaction has already completed */
		if(tp->t_omsg == NULL && tp->t_state == MG_INITST) {
			splx(s);
			return (0);
		}
	} else {
		/* remote cancel */
		/* send a cancel */
		mbp = mps_get_msgbuf(KM_NOSLEEP);
		if (mbp == ((mps_msgbuf_t *)NULL)) 
			cmn_err (CE_PANIC, "mps_AMPcancel: Cannot get message buffer");
		mps_mk_unsolrply(mbp, dsocid, tid, NULL, 0);
		mps_msg_setsrcpid(mbp,mps_port_ids[chan]);
		mps_msg_setcancel(mbp);
		mps_msg_setrid(mbp);
		status = impc_send(mbp);
		mps_free_msgbuf(mbp);
		if( status == -1) {
			splx(s);
			return (-1);
		}
		i = mps_find_transaction(dsocid,tid,MG_REMTRN);
		if(i == -1) {
			splx(s);
			return (0);
		}
		tp = &mps_tinfo[i];
	}
	/* at this point we have pointer to a transaction entry
	 * corresponding to this transaction
	 */
	/* if transaction is doing solicited transfer
	 * remove it from that queue
	 */
	/* The entry does not have to be on any queue
	 * So even if impc_sol_deque indicates that it did not
	 * remove it from a queue, just indicate completion
	 * If impc_sol_deque indicate success, the completion has
	 * already be done
	 */
	if( impc_sol_deque(tp) == -1)
		mps_msg_comp(tp,MD_SCANCL);
	splx(s);		
	return(0);
}

/*
 * This routine handles all messages incoming from the MPC.
 * Things you look at are:
 *	Broadcasts
 *		Simply queue up at the priority queues.
 *	Buffer Requests
 *		Check if response/request phase
 *		if response phase
 *			locate appropriate rsvp entry
 *			if EOT
 *				save message
 *			construct buffer grant
 *			put buffer grant in appropriate place
 *			queue up in sic_head & sic_tail
 *		if request phase
 *			queue up the buffer request
 *	unsolicited
 *		if tid is zero => non transaction message
 *			queue it up
 *		if cancel bit is set
 *			cancel
 *			save message to be passed up
 *		if send next fragment
 *			find appropriate entry
 *			construct buffer request
 *			queue up in soc_head & soc_tail
 *		otherwise
 *			queue up in priority queue
 *
 * When you return from this routine the message has been fully
 * taken care of. Either it has been queued up in a queue or it
 * has been processed and message buffer freed.
 */
void
mps_msg_proc(mbp)
register mps_msgbuf_t *mbp;        /* incoming message */
{
	int i;
	register tinfo_t *tp;
	mb2socid_t dsocid, ssocid;
	unsigned liaisonid;
	unsigned char tid;
	unsigned long len;
	tid = mps_msg_gettrnsid(mbp);
	dsocid = mps_mk_mb2socid(mps_msg_getdstmid(mbp),mps_msg_getdstpid(mbp));
	ssocid = mps_mk_mb2socid(mps_msg_getsrcmid(mbp),mps_msg_getsrcpid(mbp));
	switch(mps_msg_getmsgtyp(mbp)) {
	    case MPS_MG_BRDCST:
		mbp->mb_bind = (unsigned)MPS_MG_DFBIND;
		if(mps_msg_que(mbp,mps_msg_getdstpid(mbp)) == -1)
			mps_free_msgbuf(mbp);   /* ignore broadcast if no port */
		break;
	    case MPS_MG_BREQ:
		liaisonid = mps_msg_getlsnid(mbp);
		if(tid == 0 || mps_msg_isreq(mbp)) {
			
 			/* if next frament msg */
			if((tid != 0) && ((mps_msg_gettransctl(mbp)&(MPS_MG_RRMSK|MPS_MG_RRTMSK)) == (MPS_MG_REQ|MPS_MG_RQXF))) {
				i = mps_find_transaction(ssocid,tid,MG_REMTRN);
				if(i != -1) {
					tp = &mps_tinfo[i];
					if(((tp->t_flags&MG_ENTRY) == MG_FRAG) && (tp->t_imsg == NULL)){
						len = mps_msg_getbrlen(mbp);
						if(len > mps_msg_getfraglen(tp->t_omsg)){
							cmn_err(CE_WARN,"len in next frag msg > len in snf\n");
							len = mps_msg_getfraglen(tp->t_omsg); /* use len in snf */
						}
						if(len < mps_msg_getfraglen(tp->t_omsg))
							cmn_err(CE_WARN,"len in next frag msg < len in snf\n");
	
						mps_mk_bgrant(tp->t_omsg,ssocid,liaisonid,len);
						tp->t_imsg = mbp;
						impc_sol_que(tp,SIC);
					} else {
						mps_handle_err(mbp,ESTATE);
					}
					break;
				}
				mps_handle_err(mbp,ENOENTRY);
				break;
			}
			/* else it's rsvp or non-rsvp buffer request */
			mbp->mb_bind = (unsigned)MPS_MG_DFBIND;
			if(mps_msg_que(mbp,mps_msg_getdstpid(mbp)) == -1) 
				mps_handle_err(mbp,EDELIV);
			break;
		} else {
                        /* it's a response phase buffer request */
			i = mps_find_transaction(dsocid,tid, MG_LOCTRN);
			if( i == -1) {
				mps_handle_err(mbp,ENOENTRY);
				break;
			}
			tp = &mps_tinfo[i];
			len = mps_msg_getbrlen(mbp);
			if(tp->t_state == MG_RS_RC || tp->t_state == MG_RS_PRR) {
				tp->t_imsg = mbp;
				if(len > tp->t_irem) {
					cmn_err(CE_WARN,"can not satisfy buffer request\n");
					if(tp->t_irem == 0) {
						/* we've received all of resp */
						mps_mk_breject(tp->t_omsg,ssocid,liaisonid);
						mps_msg_setrid(tp->t_omsg);
						if(impc_send(tp->t_omsg) == -1)
							cmn_err(CE_PANIC, "sending buffer reject failed\n");
						mps_msg_comp(tp,0); /* check if eot */
						break;
					}
					len = tp->t_irem;
					mps_msg_setbrlen(tp->t_imsg,len)
				}
				mps_mk_bgrant(tp->t_omsg,ssocid,liaisonid,len);
				impc_sol_que(tp,SIC);
			} else {
				mps_handle_err(mbp,ESTATE);
			}
		}
		break;
	    case MPS_MG_UNSOL:
		if(tid == 0) {
			/* non-rsvp request */
			mbp->mb_bind = (unsigned)MPS_MG_DFBIND;
			if(mps_msg_que(mbp,mps_msg_getdstpid(mbp)) == -1)
				mps_handle_err(mbp,EDELIV);
			break;
		}
		if(mps_msg_isreq(mbp)) {
			/* if send next fragment msg */
			if((mps_msg_gettransctl(mbp)&(MPS_MG_RRMSK|MPS_MG_RRTMSK)) == (MPS_MG_REQ|MPS_MG_RQSF)) {
				i = mps_find_transaction(dsocid, tid, MG_LOCTRN);
				if(i == -1) {
					mps_handle_err(mbp,ENOENTRY);
					break;
				}
				tp = &mps_tinfo[i];
				len = mps_msg_getfraglen(mbp);
				if(tp->t_state != MG_RS_FR) {
					mps_handle_err(mbp,ESTATE);
					break;
				} else if(len == 0) {
					tp->t_state = MG_RS_RC;
					if(tp->t_obuf != NULL) {
						mps_free_dmabuf(tp->t_obuf);
						tp->t_obuf = NULL;
					}
					mps_free_msgbuf(mbp);
					break;
				}
				if(len > tp->t_orem) {
					cmn_err(CE_NOTE,"can not satisfy fragment request\n");
					len = tp->t_orem;
				}
#ifdef SNFCHECK
				if((tp->t_state==MG_RS_FR) &&
					((mps_msg_gettransctl(mbp)&(MPS_MG_RRMSK|MPS_MG_RRTMSK))!=(MPS_MG_REQ|MPS_MG_RQSF))) {
					cmn_err(CE_CONT,"(debug) Protocol error: SNF expected\n");
					mps_msg_showmsg(tp->t_imsg);
					mps_msg_showmsg(mbp);
					monitor();
				}
				if(tp->t_imsg)
					mps_free_msgbuf(tp->t_imsg);
#endif	/* SNFCHECK */ 
				tp->t_imsg = mbp;
				tp->t_omsg->mb_data[MPS_MG_BRTC] |= MPS_MG_RQXF;
				mps_msg_setbrlen(tp->t_omsg,len);
				impc_sol_que(tp,SOC);
			} else {
				/* it's an unsolicited rsvp request msg */
				mbp->mb_bind = (unsigned)MPS_MG_DFBIND;
				if(mps_msg_que(mbp,mps_msg_getdstpid(mbp)) == -1)
					mps_handle_err(mbp,EDELIV);
			}
			break;
		}
		/* else msg is response phase */
		if(mps_msg_iscancel(mbp) || mps_msg_iseot(mbp)) {
			/*
		 	 * pass up the message as the completion message
		 	 * of the transaction
			 */
			i = mps_find_transaction(dsocid, tid, MG_LOCTRN);
			if(i == -1) {
				mps_handle_err(mbp,ENOENTRY);
				break;
			}
			tp = &mps_tinfo[i];
#ifdef SNFCHECK
			if((tp->t_state==MG_RS_FR) &&
				((mps_msg_gettransctl(mbp)&(MPS_MG_RRMSK|MPS_MG_RRTMSK))!=(MPS_MG_REQ|MPS_MG_RQSF))) {
				cmn_err(CE_CONT,"(debug) Protocol error: SNF expected\n");
				mps_msg_showmsg(tp->t_imsg);
				mps_msg_showmsg(mbp);
				monitor();
			}
			if(tp->t_imsg)
				mps_free_msgbuf(tp->t_imsg);
#endif	/* SNFCHECK */
			tp->t_imsg = mbp;
			if(mps_msg_iscancel(mbp)) {
				/* If omsg is NULL, a completion for this
				 * transaction has already been sent
				 */
				if(tp->t_omsg == NULL)
					tp->t_imsg = NULL;
				else
					if(impc_sol_deque(tp) == -1)
						mps_msg_comp(tp,MD_SCANCL);
			}
			else { /* eot */
				if(tp->t_state == MG_RS_RC || tp->t_state == MG_RS_PRR)
					mps_msg_comp(tp,0);
				else {
					mps_handle_err(mbp,ESTATE);
				}

  			}
			break;
		}
		/* discard any unsolicited messages that are response 
	         * but not EOT
		 */
		mps_free_msgbuf(mbp);
		break;
	    default :
		cmn_err(CE_WARN,"unknown incoming message type\n");
		mps_free_msgbuf(mbp);
		break;
	}
}

/*
 * tp points to a transaction table entry whose current
 * solicited transaction just completed.
 * status contains the completion status.
 * status == -1 => send of unsolicited message to start the
 *		   solicited transfer failed
 *			ie BREQ did not go for solicited output
 *			   BGRANT did not go for solicited input
 *		   if this happened, the actual error status is
 *		   already in the MPS_MG_RI byte in the message
 *		   being sent
 * status == 0     solicited transfer completed without error
 * status == anything else	this is the error status
 */
void
mps_msg_comp(tp,status)
register tinfo_t *tp;
int status;
{
	register mps_msgbuf_t *mbp;
	long len;

	/* everything went hunky-dori */
	/* figure out what to do next */
	switch(tp->t_flags & MG_ENTRY) {
	    case MG_RSVP:
		if(tp->t_state == MG_INITST && (status&MD_SCANCL)&&
				(tp->t_imsg ==NULL)) {
			/* we recieved a reject from server */
			/* go into fragmentation state */
			tp->t_state = MG_RS_FR;
			break;
		}
		if(status != 0 ) {
			if(tp->t_imsg !=NULL && mps_msg_iscancel(tp->t_imsg)) {
				mbp = tp->t_imsg;
				mbp->mb_bind = tp->t_omsg->mb_bind;
				mps_free_msgbuf(tp->t_omsg);
				status=0;
			} else {
				mbp = tp->t_omsg;
				if(tp->t_imsg)
					mps_free_msgbuf(tp->t_imsg);
			}
			tp->t_omsg = NULL;
			tp->t_imsg = NULL;
			tp->t_state = MG_INITST;
			if(tp->t_ibuf) {
				mps_free_dmabuf(tp->t_ibuf);
				tp->t_ibuf = NULL;
			}
			if(tp->t_obuf) {
				mps_free_dmabuf(tp->t_obuf);
				tp->t_obuf = NULL;
			}
			if(status != -1 && !mps_msg_iscancel(mbp)) {
				/* error in solicited tranmission */
				mbp->mb_data[MPS_MG_RI] = status;
				mbp->mb_flags |= MPS_MG_ESOL;
			}
			/* else error in unsol transmission. do nothing */
			/* queue up error message as completion */
			mbp->mb_flags |= MPS_MG_DONE;
			if(mps_msg_que(mbp, mps_mk_mb2soctopid(tp->t_lsocid)) == -1)
				mps_handle_err(mbp,EDELIV);
			break;
		}
		/* no error */
		switch(tp->t_state) {
		    case MG_INITST:
			tp->t_state = MG_RS_RC;
			if (tp->t_omsg)
				tp->t_orem -= mps_msg_getbrlen(tp->t_omsg);
			if(tp->t_obuf) {
				mps_free_dmabuf(tp->t_obuf);
				tp->t_obuf = NULL;
			}
			break;
		    case MG_RS_FR:
			if(tp->t_imsg) {
				mbp = tp->t_imsg;
				if((mps_msg_gettransctl(mbp)&(MPS_MG_RRMSK|MPS_MG_RRTMSK)) == (MPS_MG_REQ|MPS_MG_RQSF)) {
					len = mps_msg_getbrlen(tp->t_omsg);
					tp->t_orem -= len;
				} else {
					cmn_err(CE_NOTE,"Protocol error: SNF expected\n");
#ifdef SNFCHECK
					mps_msg_showmsg(mbp);
					monitor();
#endif	/* SNFCHECK */
				}
#ifndef SNFCHECK
				tp->t_imsg = NULL;
				mps_free_msgbuf(mbp);
#endif	/* SNFCHECK */
			}
			if(tp->t_orem <= 0 )
				tp->t_state = MG_RS_RC;
			break;
		    case MG_RS_RC:
			tp->t_state = MG_RS_PRR;
			/* FALLTHRU */
		    case MG_RS_PRR:
			mbp = tp->t_imsg;
			tp->t_imsg = NULL;
			if(mbp) {
				if(mps_msg_getmsgtyp(mbp) == MPS_MG_BREQ) {
					if(tp->t_irem != 0)
						tp->t_irem -= mps_msg_getbrlen(mbp);
					if(mps_msg_iseot(mbp))
						mps_msg_setbrlen(mbp, tp->t_icnt-tp->t_irem);
				}
				if(mps_msg_iseot(mbp)) {
					tp->t_state = MG_INITST;
					mbp->mb_bind = tp->t_omsg->mb_bind;
					mps_free_msgbuf(tp->t_omsg);
					mbp->mb_flags |= MPS_MG_DONE;
					if(mps_msg_que(mbp,mps_port_ids[tp->t_lcid]) == -1)
						mps_handle_err(mbp,EDELIV);
					if(tp->t_ibuf)
						mps_free_dmabuf(tp->t_ibuf);
					tp->t_ibuf = NULL;
					tp->t_omsg = NULL;
				} else
					/* not EOT. free it */
					mps_free_msgbuf(mbp);
			}
			break;
		    default: /* This should never happen! */
			cmn_err(CE_PANIC,
			"unknown transaction state in mps_msg_comp\n");
			break;
		}
		break;
	    case MG_RCV:
		/* fall thru */
	    case MG_FRAG:
		/* as above no local transactionID. Also
		 * table entry is in remote transaction
		 * table.
		 */
		mps_free_dmabuf(tp->t_ibuf);
		mbp = tp->t_omsg;
		tp->t_omsg = NULL;
		if(tp->t_imsg) {
			mps_free_msgbuf(tp->t_imsg);
			tp->t_imsg = NULL;
		}
		mbp->mb_flags |= MPS_MG_DONE;
		if(status && status != -1) {
			mbp->mb_flags |= MPS_MG_ESOL;
			mbp->mb_data[MPS_MG_RI] = status;
		}
		if(mps_msg_que(mbp,mps_port_ids[tp->t_lcid]) == -1)
			mps_handle_err(mbp,EDELIV);
		tp->t_rsocid = 0;
		tp->t_lsocid = 0;
		tp->t_lcid = 0;
		tp->t_flags = 0;
		mps_t_ids[tp-mps_tinfo] = 0;
		break;
	    case MG_SDATA:
		/* fall thru */
	    case MG_SRPLY:
		mbp = tp->t_omsg;
		mbp->mb_flags |= MPS_MG_DONE;
		if(status && status != -1) {
			mbp->mb_flags |= MPS_MG_ESOL;
			mbp->mb_data[MPS_MG_RI] = status;
		}
		if(mps_msg_que(mbp,mps_port_ids[tp->t_lcid]) == -1)
			mps_handle_err(mbp,EDELIV);
		mps_free_dmabuf(tp->t_obuf);
		tp->t_obuf = NULL;
		tp->t_omsg = NULL;
		tp->t_state = MG_INITST;
		mps_t_ids[tp-mps_tinfo]=0;
		tp->t_lcid = 0;
		tp->t_flags = 0;
		tp->t_lsocid = 0;
		tp->t_rsocid = 0;
		break;
	    default: /* This should never happen! */
		cmn_err(CE_PANIC,"bad transaction type in mps_msg_comp\n");
		break;
	}
}

/* Routine to handle the following error conditions (errtype):
 *       EDELIV : a msg could not be delivered to a port
 *       ENOENTRY : transaction (to receive msg) doesn't exist
 *       ESTATE : msg arrived for a transaction out of state
 * In all cases an error is printed and the message (mbp) is freed.
 * A cancel is sent for undeliverable rsvp msgs as defined by the 
 * specification. Any other arriving buffer req's are rejected.
 * NOTE: completion msgs may be a variety of types; if buf req we
 * musn't send a reject! (ie. it didn't recently arrive remotely).
 *
 * Called By: mps_msg_comp - for undeliverable completion msgs
 *            mps_msg_proc - (msg has just arrived) for all other cases
 */

static void
mps_handle_err(mps_msgbuf_t *mbp, unsigned char errtype)
{
	                         /* remote socket id which sent msg */
	mb2socid_t ssocid = mps_mk_mb2socid(mps_msg_getsrcmid(mbp),mps_msg_getsrcpid(mbp));
	unsigned char tid = mps_msg_gettrnsid(mbp);
	char *s;
	int sendcan=0;
	int sendrej=0;
	unsigned char rrtype;    /* req/resp type of rsvp related msg */

	if(errtype&EDELIV) {     /* rsvp/non-rsvp request or completion msg */
		if(mps_msg_iscompletion(mbp))
			cmn_err(CE_NOTE,"!could not deliver completion msg\n");
		else if(tid != 0) {
			cmn_err(CE_NOTE,"!could not deliver rsvp request msg\n");
			mps_mk_unsolrply(mbp,ssocid,tid,NULL,0);
			mps_msg_setcancel(mbp);
			sendcan++;
		} else
			cmn_err(CE_NOTE,"!could not deliver non-rsvp request msg\n");
	} else {                 /* response, cancel, snf, or nxt frag msg */
		rrtype = mps_msg_gettransctl(mbp)&MPS_MG_RRTMSK;
		if(mps_msg_isreq(mbp)) {
			if (rrtype == MPS_MG_RQXF)
				s = "next fragment";
			else
				s = "SNF";
		} else {
			if(mps_msg_iscancel(mbp))
				s = "cancel";
			else
				s = "response";
		}
		if(errtype&ESTATE)
			cmn_err(CE_WARN,"Out of state %s msg.\n",s);
		else /* ENOENTRY */
			cmn_err(CE_NOTE,"!Unknown %s msg.\n",s);
	}
	if(mps_msg_getmsgtyp(mbp) == MPS_MG_BREQ && !sendcan && !mps_msg_iscompletion(mbp)){
		mps_mk_breject(mbp,ssocid,mps_msg_getlsnid(mbp));
		sendrej++;
	}
	if(sendcan | sendrej) {
		mps_msg_setrid(mbp);
		if (impc_send(mbp) == -1)
			cmn_err(CE_PANIC,"sending reject failed\n");
#ifdef XDEBUG
		if(sendcan)
			cmn_err(CE_CONT,"sent cancel\n");
		else
			cmn_err(CE_CONT,"sent reject\n");
#endif
	}
	mps_free_msgbuf(mbp);
}
 
/*
 * This routine will look up the priority of a mesage and queue
 * up the message in the appropriate queue.
 * If the port is not open, message is not queued and
 * -1 is returned. Otherwise 0 is returned
 */
static int
mps_msg_que(mps_msgbuf_t *mbp, unsigned short portid)
{
	int chan;
	int prio;
	int s;

	if((chan=mps_find_port(portid)) == -1)
		return(-1);
	s = splhi();
	mbp->mb_next = (mps_msgbuf_t *)NULL;
	prio = mps_port_defs[chan].pr_level;
	mbp->mb_intr = mps_port_defs[chan].pr_intr;
	mps_prioq[prio].m_count++;
	if(mps_prioq[prio].m_qhead == (mps_msgbuf_t *)NULL)
		mps_prioq[prio].m_qhead = mps_prioq[prio].m_qtail = mbp;
	else {
		mps_prioq[prio].m_qtail->mb_next = mbp;
		mps_prioq[prio].m_qtail = mbp;
	}
	splx(s);		
	return(0);
}

/* 
 * this routine runs with all interrupts turned off except when
 * actually calling an interrupt handler
 */
void
mps_msg_dispatch()
{
	int i,s;
	int savprio;
	register mps_msgbuf_t *mbp;

	s = splhi();	
	if( mps_ckprio >= MPS_CLKPRIO) {
		splx(s);	
		return;
	}
	for(i=MPS_CLKPRIO;i>mps_ckprio;i--) {
		savprio = mps_ckprio;
		mps_ckprio = i;
		while((mbp = mps_prioq[i].m_qhead) != NULL) {
			mps_prioq[i].m_qhead = mbp->mb_next;
			mps_prioq[i].m_count--;
			splx(s);	
			if( mbp->mb_intr != NULL)
				(*mbp->mb_intr)(mbp);
			else
				mps_free_msgbuf(mbp);
			s = splhi();	
		}
		mps_ckprio = savprio; 
	}
	splx(s);	
}


/*
 * This routine returns open port table index given a port number
 */

static int
mps_find_port(unsigned short portid)
{
	register int chan;
	for(chan=0;chan<mps_max_port;chan++)
		if(mps_port_ids[chan] == portid)
			return(chan);
	return(-1);
}
/*
 * set up a port number to send and receive messages
 * portid 0 is ivvalid by convention
 */

long
mps_open_chan(portid, intr, priolev)
unsigned short portid;
int (*intr)();
unsigned short priolev;
{
	register long chan;
	int s;

	if(portid == 0 || ((priolev != MPS_NRMPRIO) && (priolev != MPS_SRLPRIO)
	   && (priolev != MPS_BLKPRIO) && (priolev != MPS_CLKPRIO)) )
		return(-1);
	
	s = splhi();
	/* check if already open */
	if( mps_find_port(portid) != -1) {
		splx(s);
		return(-1);
	}
	/* find free entry */
	chan = mps_find_port(0);
	if(chan != -1) {
		mps_port_defs[chan].pr_flags = 0;
		mps_port_defs[chan].pr_ltid = 0;
		mps_port_defs[chan].pr_intr = intr;
		mps_port_defs[chan].pr_level = priolev;
		mps_port_ids[chan] = portid;
	}
	splx(s);	
	return(chan);
}

long
mps_close_chan(chan)
long chan;
{
	static port_t zport = { 0, 0, 0, 0};
	unsigned short port;
	unsigned char tid;
	int i, s;
	if(chan<0 || chan>= mps_max_port)
		return(-1);
	port = mps_port_ids[chan];
	if(port == 0)
		return(-1);

	/* search for any active transactions */
	s = splhi();
	tid=0;
	do {
		i = mps_find_transaction(mps_mk_mb2socid(mps_lhid(),port),tid, MG_LOCTRN);
		if( i != -1) {
			splx(s);
			return(-1);
		}
		++tid;
	} while(tid);
	mps_port_defs[chan] = zport;
	mps_port_ids[chan] = 0;
	splx (s);
	return(0);
}

static unsigned short lhid;
unsigned short
mps_lhid()
{
	return(lhid);
}

void
mpsinit()
{
	unsigned short reg;
	struct ics_struct ics;


	impcinit();

	/* 
	 * until we have a name server use slot id as host id
	 * Initialize interconnect record accordingly
	 */
	lhid = (unsigned short)ics_myslotid();
	reg = ics_find_rec(ICS_MY_SLOT_ID, ICS_HOST_ID_TYPE);
	ics.slot_id = ICS_MY_SLOT_ID;
	ics.count = 1;
	if(reg != 0xffff) {
		ics.reg_id = reg+2;
		ics.buffer[0] = MPS_LOB(lhid);
		ics_rw(ICS_WRITE_ICS, &ics);
		ics.reg_id++;
		ics.buffer[0] = MPS_HIB(lhid);
		ics_rw(ICS_WRITE_ICS, &ics);
	}
}

/*
 *	Message buffer management.  The policy is to try to maintain
 *	mps_msg_lowat buffers against the possibility that kma
 *	will not have memory for us at interrupt time.
*/
mps_msgbuf_t *mps_msg_freelist = 0;
int mps_msg_free = 0;
extern int mps_msg_lowat;	/* from space.c */

mps_msgbuf_t *
mps_get_msgbuf(mode)
int mode;
{
	mps_msgbuf_t *mbp;
	int s;

	/*
	 *	Get above the low water mark if possible
	*/
	while (mps_msg_free < mps_msg_lowat) {
		mbp = (mps_msgbuf_t *)kmem_zalloc(sizeof(mps_msgbuf_t),mode);
		if (mbp == (mps_msgbuf_t *) NULL) {
			break;
		}
		s = splhi();
		mbp->mb_next = mps_msg_freelist;
		mps_msg_freelist = mbp;
		mps_msg_free++;
		splx(s);
	}
	/*
	 *	Do dynamic allocation if possible
	*/
	mbp = (mps_msgbuf_t *) kmem_zalloc(sizeof(mps_msgbuf_t), mode);
	/*
	 *	Take from emergency pool if necessary
	*/
	if (mbp == (mps_msgbuf_t *) NULL) {
		s = splhi();
		mbp = mps_msg_freelist;
		if (mbp) {
			mps_msg_freelist = mbp->mb_next;
			mps_msg_free--;
		}
		splx(s);
	}
	/*
	 *	Warn the user if no message buffer available
	 *	This means deep trouble for the system.
	*/
	if (mbp == (mps_msgbuf_t *) NULL)
		cmn_err(CE_WARN,"mps_get_msgbuf - out of message buffers");
	return(mbp);
}


void
mps_free_msgbuf(mbp)
mps_msgbuf_t *mbp;
{
	int s;

	/*
	 *	If we are low on emergency buffers, restock
	 *	Enforce the invariant that everything on the
	 *	free list has been zeroed.
	*/
	if (mps_msg_free < mps_msg_lowat) {
		bzero((caddr_t)mbp, sizeof(*mbp));
		s = splhi();
		mbp->mb_next = mps_msg_freelist;
		mps_msg_freelist = mbp;
		mps_msg_free++;
		splx(s);
		return;
	}
	/*
	 *	If we are well stocked, return the memory to the kernel
	*/
	kmem_free((caddr_t) mbp, sizeof(mps_msgbuf_t));
}

/*
 * mps_free_dmabuf() - Return a DMA Buffer Descriptor list to the free list
*/
void
mps_free_dmabuf (dmabufptr)
struct dma_buf *dmabufptr;
{
	struct dma_buf *dptr, *sdptr;

	if(dmabufptr == NULL)
		return;
	dptr = dmabufptr;
	while(dptr) {
		sdptr = dptr;
		dptr = dptr->next_buf;
		dma_free_buf (sdptr);
	}
}


/*
 * mps_get_dmabuf() -  Get a list of DMA Buffer Descriptors for a future DMA
 *		    operation. Return a pointer to the head of the list.
 */

struct dma_buf *
mps_get_dmabuf(cnt,mode)
unsigned long cnt;
unsigned char mode;
{
	struct dma_buf *dbhead, *dbtail;

	if(cnt == 0)
		return(NULL);

	if ((dbhead = dma_get_buf(mode)) == (struct dma_buf *) NULL) {
		return (NULL);
	}
	dbtail = dbhead;
	while (--cnt) {
		if ((dbtail->next_buf = dma_get_buf(mode)) 
				== (struct dma_buf *) NULL) {
			mps_free_dmabuf (dbhead);
			return (NULL);
		}
		dbtail = dbtail->next_buf;
	}
	dbtail->next_buf = NULL;
	return(dbhead);
}

/* this routine splits a a data buffer into two portions.
 * count number of bytes are left in the first buffer
 * rest of the byte are in the second buffer.
 * pointer to the incoming buffer points to the first buffer after split
 * pointer to the second buffer is returned
 * both buffers after the breakup are terminated by a descriptor
 * with 0 count and NULL next pointer
 */

struct dma_buf *
mps_dmabuf_breakup(dmabufptr,count)
struct dma_buf *dmabufptr;
register long count;
{
	register struct dma_buf *dp1, *dp2;

	if(count == 0 || dmabufptr == NULL || dmabufptr->count == 0)
		return (NULL);
	dp1=dmabufptr;
	while((dp1 != NULL) && ((unsigned)count >= dp1->count)) {
		count -= dp1->count;
		dp1 = dp1->next_buf;
	}
	if(dp1 == NULL && count>0)
		return (NULL);
	if(count == 0) {
		if(dp1 == NULL || dp1->count == 0)
			return(NULL);
		else {
			dp2 = dmabufptr;
			while(dp2->next_buf != dp1)
				dp2 = dp2->next_buf;
			if ((dp2->next_buf = mps_get_dmabuf(1, DMA_NOSLEEP)) 
					== (struct dma_buf *)NULL)
				return (NULL);
			dp2->next_buf->next_buf = NULL;
			dp2->next_buf->count = 0;
			return(dp1);
		}
	}else {
		if(dp1 == dmabufptr) {
			if ((dp1 = mps_get_dmabuf(2, DMA_NOSLEEP))
					== (struct dma_buf *)NULL)
				return (NULL);
			dp2 = dp1->next_buf;
			dp1->count = dmabufptr->count - count;
			dp1->address = dmabufptr->address + count;
			dp1->next_buf = dmabufptr->next_buf;
			dmabufptr->count = count;
			dmabufptr->next_buf = dp2;
		} else {
			dp2 = dmabufptr;
			while(dp2->next_buf != dp1)
				dp2 = dp2->next_buf;
			if ((dp2->next_buf = mps_get_dmabuf(2, DMA_NOSLEEP)) 
					== (struct dma_buf *)NULL)
				return (NULL);
			dp2 = dp2->next_buf;
			dp2->count = count;
			dp2->address = dp1->address;
			dp1->count -= count;
			dp1->address += count;
		}
	}
	return(dp1);
}


/* join two linked lists of data buffers
 * first buffer is put at the head
 * ptr to head of the resultant buffer is returned
 *
 */
struct dma_buf *
mps_dmabuf_join(db1,db2)
register struct dma_buf *db1;
register struct dma_buf *db2;
{
	struct dma_buf *ret = db1;
	register struct dma_buf *tail;

	if(db1 == NULL)
		return(db2);
	if(db2 == NULL)
		return(db1);
	if(db1->count == NULL) {
		mps_free_dmabuf(db1);
		return(db2);
	}
	if(db2->count == 0) {
		mps_free_dmabuf(db2);
		return(db1);
	}
	tail = db1;
	while( db1->count != 0) {
		tail = db1;
		db1 = db1->next_buf;
	}
	mps_free_dmabuf(db1);
	tail->next_buf = db2;
	return(ret);
}


/* Get a Transaction ID and a Transaction Table Entry
 * 	if channel is invalid return 0(0 is an invalid Transaction ID)
 *	otherwise start with last transaction allocated+1
 *	while not searched all transactions IDs
 *		find if this transaction ID is in use
 *		If not in use
 *			find a free transaction table entry
 *			set values of port and transaction IDs
 *			return
 *		increment tid and try again
 *		
 */
unsigned char
mps_get_tid(chan)
long chan;
{
	unsigned char tid;
	unsigned char stid;	/* starting tid */
	int i,s;

	/* channel not open */
	if(mps_port_ids[chan] == 0)
		return(0);

	tid = stid = mps_port_defs[chan].pr_ltid+1;
	s = splhi();
	do {
		if(tid == 0)
			tid=1;
		if( mps_find_transaction(mps_mk_mb2socid(lhid,mps_port_ids[chan]),tid, MG_LOCTRN) == -1) {
			/*
			 *	We have found a free tid.  Allocate and return it.
			*/
			i = mps_find_transaction(0, 0, MG_LOCTRN);
			if (i == -1) {
				cmn_err(CE_WARN, "mps_get_tid: can't allocate tid\n");
				splx(s);
				return(0);
			}
			mps_t_ids[i] = tid; /* set tid */
			mps_tinfo[i].t_lcid = chan;
			mps_tinfo[i].t_lsocid
				= mps_mk_mb2socid(lhid,mps_port_ids[chan]); /* set port id */
			mps_tinfo[i].t_rsocid = 0;
			mps_tinfo[i].t_state = MG_INITST;/* initial state */
			mps_tinfo[i].t_flags = MG_RSVP;  /* request type */
			mps_port_defs[chan].pr_ltid = tid;
			splx(s);
			return(tid);
		}
		tid++;
	} while(tid != stid);
	splx(s);
	return(0);
}


/* free a transaction id and its transaction table entry.
 * Note that tid here is not invalid since
 * transaction table entries could belong to a transaction-less
 * data transmission request
 */
long
mps_free_tid(chan,tid)
long chan;
unsigned char tid;
{
	int	tindex;	/* index into transaction table */
	int 	s;
	/* channel not open */
	if(mps_port_ids[chan] == 0)
		return(-1);
	s = splhi();
	tindex = mps_find_transaction(mps_mk_mb2socid(lhid,mps_port_ids[chan]), tid, MG_LOCTRN);
	if(tindex == -1) {
		splx(s);
		return(-1);
	}
	/* we should not allow freeing of a busy transaction */
	if(mps_tinfo[tindex].t_state != MG_INITST) {
		splx(s);
		return(-1);
	}
	mps_tinfo[tindex].t_lsocid = 0; /* null out local socket field */
	mps_tinfo[tindex].t_rsocid = 0; /* null out remote socket field */
	mps_tinfo[tindex].t_lcid = 0; /* null out the chan field */
	mps_tinfo[tindex].t_flags = 0;
	mps_t_ids[tindex] = 0; /* zero out the transaction id */
	splx(s);
	return(0);
}

/* search for a pid:tid pair in the table
 * should be replaced by an assembly routine
 */

int
mps_find_transaction(socid,tid, which)
mb2socid_t socid;
unsigned char tid;
int which;
{
	register int i;

	for(i=0;i<mps_max_tran;i++) {
		if(mps_t_ids[i] != tid)
			continue;
		if(which == MG_LOCTRN) {
			switch(mps_tinfo[i].t_flags&MG_ENTRY){
			case MG_RSVP:
			case MG_SDATA:
				break;
			case 0:
				if(tid != 0)
					continue;
				break;
			default:
				continue;
			}
			if(mps_tinfo[i].t_lsocid == socid)
				return(i);
		} else if(which == MG_REMTRN) {
			switch(mps_tinfo[i].t_flags&MG_ENTRY){
			case MG_FRAG:
			case MG_RCV:
			case MG_SRPLY:
				break;
			case 0:
				if(tid !=0)
					continue;
				break;
			default:
				continue;
			}
			if(mps_tinfo[i].t_rsocid == socid)
				return(i);
		} else
			return(-1);
	}
	return(-1);
}


/* 
 * returns count of bytes in the buffer
 */
long
mps_buf_count(dbp)
struct dma_buf *dbp;
{
	long cnt=0;
	while(dbp != NULL) {
		cnt += dbp->count;
		dbp = dbp->next_buf;
	}
	return(cnt);
}

void
mps_msg_showmsg(mbp)
mps_msgbuf_t *mbp;
{	static char hex[] = "0123456789ABCDEF";
	int i;
	int count;
	switch(mps_msg_getmsgtyp(mbp)) {
	case MPS_MG_UNSOL:
	case MPS_MG_BRDCST:
		count=MPS_MG_UMOVHD;
		break;
	case MPS_MG_BREQ:
		count=MPS_MG_BROVHD;
		break;
	case MPS_MG_BREJ:
		count=MPS_MG_BJOVHD;
		break;
	case MPS_MG_BGRANT:
		count=MPS_MG_BGOVHD;
		break;
	}
	for(i=0;i<count;i++)
		cmn_err(CE_CONT," %c%c",hex[(mbp->mb_data[i]>>4)&0xf],
				hex[mbp->mb_data[i]&0xf]);
	
	cmn_err(CE_CONT," B: %x F:",mbp->mb_bind);
	if(mbp->mb_flags&MPS_MG_TERR)
		cmn_err(CE_CONT," TERR");
	if(mbp->mb_flags&MPS_MG_ESOL)
		cmn_err(CE_CONT," ESOL");
	if(mbp->mb_flags&MPS_MG_DONE)
		cmn_err(CE_CONT," DONE");
	cmn_err(CE_CONT,"\n");
	count = (mbp->mb_count>MPS_MAXMSGSZ-count?MPS_MAXMSGSZ-count
						:mbp->mb_count);
	for(;i<count;i++)
		cmn_err(CE_CONT," %c%c",hex[(mbp->mb_data[i]>>4)&0xf],
				hex[mbp->mb_data[i]&0xf]);
	cmn_err(CE_CONT,"\n");	
}


/* routines to set up MB II Messages
 */

void
mps_mk_unsol(mbp, dsocid, tid, dptr, count)
register mps_msgbuf_t *mbp;
mb2socid_t dsocid;
unsigned char tid;
unsigned char *dptr;
unsigned long count;
{	register caddr_t dst;

	mbp->mb_data[MPS_MG_DA] = MPS_LOB(HIW((unsigned long)dsocid));
	mbp->mb_data[MPS_MG_SA] = MPS_LOB(lhid);
	mbp->mb_data[MPS_MG_MT] = MPS_MG_UNSOL;
	mbp->mb_data[MPS_MG_UMPI] = MPS_MB2_TPDT;
	*(unsigned short *)&mbp->mb_data[MPS_MG_UMDP] = LOW(dsocid);
	mbp->mb_data[MPS_MG_UMTI] = tid;
	mbp->mb_data[MPS_MG_UMTC] = MPS_MG_REQ|MPS_MG_RQNF;
	mbp->mb_count = count + MPS_MG_UMOVHD;
	dst = (caddr_t)&mbp->mb_data[MPS_MG_UMUD];
	bcopy((caddr_t)dptr, dst, count);
	dst += count;
	while(dst < (caddr_t)&mbp->mb_data[MPS_MAXMSGSZ])
		*dst++ = 0;
}

void
mps_mk_brdcst(mbp, dpid, dptr, count)
register mps_msgbuf_t *mbp;
unsigned short dpid;
unsigned char *dptr;
unsigned long count;
{	register caddr_t dst;

	mbp->mb_data[MPS_MG_DA] = 0xff ;
	mbp->mb_data[MPS_MG_SA] = MPS_LOB(lhid);
	mbp->mb_data[MPS_MG_MT] = MPS_MG_BRDCST;
	mbp->mb_data[MPS_MG_UMPI] = MPS_MB2_TPDT;
	*(unsigned short *)&mbp->mb_data[MPS_MG_UMDP] = dpid;
	mbp->mb_data[MPS_MG_UMTI] = 0;
	mbp->mb_data[MPS_MG_UMTC] = 0;
	mbp->mb_count = count + MPS_MG_UMOVHD;
	dst = (caddr_t)&mbp->mb_data[MPS_MG_UMUD];
	bcopy((caddr_t)dptr, dst, count);
	dst += count;
	while(dst < (caddr_t)&mbp->mb_data[MPS_MAXMSGSZ])
		*dst++ = 0;
}

void
mps_mk_sol(mbp, dsocid, tid, dptr, count)
register mps_msgbuf_t *mbp;
mb2socid_t dsocid;
unsigned char tid;
unsigned char *dptr;
unsigned long count;
{	register caddr_t dst;

	mbp->mb_data[MPS_MG_DA] = MPS_LOB(HIW(dsocid));
	mbp->mb_data[MPS_MG_SA] = MPS_LOB(lhid);
	mbp->mb_data[MPS_MG_MT] = MPS_MG_BREQ;
	mbp->mb_data[MPS_MG_BRPI] = MPS_MB2_TPDT;
	*(unsigned short *)&mbp->mb_data[MPS_MG_BRDP] = LOW(dsocid);
	mbp->mb_data[MPS_MG_BRTI] = tid;
	mbp->mb_data[MPS_MG_BRTC] = MPS_MG_REQ|MPS_MG_RQMF;
	mbp->mb_count = count + MPS_MG_BROVHD;
	dst = (caddr_t)&mbp->mb_data[MPS_MG_BRUD];
	bcopy((caddr_t)dptr, dst, count);
	dst += count;
	while(dst < (caddr_t)&mbp->mb_data[MPS_MAXMSGSZ])
		*dst++ = 0;
}

void
mps_mk_unsolrply(mbp, dsocid, tid, dptr, count)
register mps_msgbuf_t *mbp;
mb2socid_t dsocid;
unsigned char tid;
unsigned char *dptr;
unsigned long count;
{
	register caddr_t dst;

	mbp->mb_data[MPS_MG_DA] = MPS_LOB(HIW(dsocid));
	mbp->mb_data[MPS_MG_SA] = MPS_LOB(lhid);
	mbp->mb_data[MPS_MG_MT] = MPS_MG_UNSOL;
	mbp->mb_data[MPS_MG_UMPI] = MPS_MB2_TPDT;
	*(unsigned short *)&mbp->mb_data[MPS_MG_UMDP] = LOW(dsocid);
	mbp->mb_data[MPS_MG_UMTI] = tid;
	mbp->mb_data[MPS_MG_UMTC] = MPS_MG_RES|MPS_MG_RSET;
	mbp->mb_count = count + MPS_MG_UMOVHD;
	dst = (caddr_t)&mbp->mb_data[MPS_MG_UMUD];
	bcopy((caddr_t)dptr, dst, count);
	dst += count;
	while(dst < (caddr_t)&mbp->mb_data[MPS_MAXMSGSZ])
		*dst++ = 0;
}

void
mps_mk_solrply(mbp, dsocid, tid, dptr, count, eotflag)
register mps_msgbuf_t *mbp;
mb2socid_t dsocid;
unsigned char tid;
unsigned char *dptr;
unsigned long count;
unsigned char eotflag;
{	register caddr_t dst;

	mbp->mb_data[MPS_MG_DA] = MPS_LOB(HIW(dsocid));
	mbp->mb_data[MPS_MG_SA] = MPS_LOB(lhid);
	mbp->mb_data[MPS_MG_MT] = MPS_MG_BREQ;
	mbp->mb_data[MPS_MG_BRPI] = MPS_MB2_TPDT;
	*(unsigned short *)&mbp->mb_data[MPS_MG_BRDP] = LOW(dsocid);
	mbp->mb_data[MPS_MG_BRTI] = tid;
	mbp->mb_data[MPS_MG_BRTC] = MPS_MG_RES|(eotflag?MPS_MG_RSET:MPS_MG_RSNE);
	mbp->mb_count = count + MPS_MG_BROVHD;
	dst = (caddr_t)&mbp->mb_data[MPS_MG_BRUD];
	bcopy((caddr_t)dptr, dst, count);
	dst += count;
	while(dst < (caddr_t)&mbp->mb_data[MPS_MAXMSGSZ])
		*dst++ = 0;
}

void
mps_mk_bgrant(mbp, dsocid, lid, count)
register mps_msgbuf_t *mbp;
mb2socid_t dsocid;
unsigned char lid;
unsigned long count;
{
	mbp->mb_data[MPS_MG_DA] = MPS_LOB(HIW(dsocid));
	mbp->mb_data[MPS_MG_SA] = LOW(lhid);
	mbp->mb_data[MPS_MG_MT] = MPS_MG_BGRANT;
	mbp->mb_data[MPS_MG_BGLI] = lid;
	mbp->mb_data[MPS_MG_BGLL] = count;
	mbp->mb_count = MPS_MG_BGOVHD;
}

void
mps_mk_breject(mbp,dsocid, lid)
register mps_msgbuf_t *mbp;
mb2socid_t dsocid;
unsigned char lid;
{
	mbp->mb_data[MPS_MG_DA] = MPS_LOB(HIW(dsocid));
	mbp->mb_data[MPS_MG_SA] = LOW(lhid);
	mbp->mb_data[MPS_MG_MT] = MPS_MG_BREJ;
	mbp->mb_data[MPS_MG_BJLI] = lid;
	mbp->mb_count = MPS_MG_BJOVHD;
}

void
mps_mk_snf(mbp, dsocid, tid, len)
register mps_msgbuf_t *mbp;
mb2socid_t dsocid;
unsigned char tid;
unsigned long len;
{
	mbp->mb_data[MPS_MG_DA] = MPS_LOB(HIW(dsocid));
	mbp->mb_data[MPS_MG_SA] = MPS_LOB(lhid);
	mbp->mb_data[MPS_MG_MT] = MPS_MG_UNSOL;
	mbp->mb_data[MPS_MG_NFPI] = MPS_MB2_TPDT;
	mbp->mb_data[MPS_MG_NFTC] = MPS_MG_REQ|MPS_MG_RQSF;
	mbp->mb_data[MPS_MG_NFTI] = tid;
	*(unsigned short *)&mbp->mb_data[MPS_MG_NFDP] = LOW(dsocid);
	*(long *)&mbp->mb_data[MPS_MG_NFFL] = len;
	mbp->mb_count =  MPS_MG_NFOVHD;
}

void
mps_get_unsoldata(mbp, dptr, count)
mps_msgbuf_t *mbp;
register unsigned char *dptr;
unsigned long count;
{
	register unsigned char *src;
	src = &mbp->mb_data[MPS_MG_UMUD];
	while(count--)
		*dptr++ = *src++;
}

void
mps_get_soldata(mbp, dptr, count)
mps_msgbuf_t *mbp;
register unsigned char *dptr;
unsigned long count;
{
	register unsigned char *src;
	src = &mbp->mb_data[MPS_MG_BRUD];
	while(count--)
		*dptr++ = *src++;
}

/*
 * special entry for unsol rply in response
 */

mps_get_reply_len (socid, tid)
mb2socid_t socid;
unsigned char tid;
{
	int i,s;
	tinfo_t *tp;

	s = splhi();

	for(i=0;i<mps_max_tran;i++) {
		if(mps_t_ids[i] != tid)
			continue;
		if(mps_tinfo[i].t_lsocid == socid)
			break;
	}
	
	if (i == mps_max_tran) {
		cmn_err (CE_WARN, "Invalid tid (%x) in msg_get_rply_len", tid);
		splx (s);
		return (0);
	}
	tp = &mps_tinfo[i];
	if (tp->t_icnt != 0) {
		splx(s);	
		return (tp->t_icnt - tp->t_irem);
	}
	splx(s);	
	return (0);
}
