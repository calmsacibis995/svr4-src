/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1985, 1986, 1987, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/enetdrv/enetl.c	1.3.1.1"

 /*
 *   enetl.c: low level routines for handling the state of the endpoint
 *           in relation to the board.  All access to the board is
 *           through this file.
 *
 *	I000	7/14/1987	DF 	Intel
 *		Added support for dynamic iNA961 selection (between R2.0 and
 *		R1.3).
 *
 *	I001	7/24/87		AP	Intel
 *		Posted multiple buffers on VC to compensate for iNA message 
 *		handling (iNA throws away messages if there is no host buffer 
 *		posted).
 *
 *	I002	8/20/87		DF	Intel
 *		Modified enet_data_ind() and enet_data_ind_complete() for
 *		performance improvement.
 *
 *	I003	8/25/1987	DF	Intel
 *		Moved DATA_BUF_LEN definition from enet.h to space.c and 
 *		changed DATA_BUF_LEN to data_buf_len.
 *		Moved NUM_ACT_BUFS definition from enet.h to space.c and 
 *		changed NUM_ACT_BUFS to n_act_bufs.
 *		Moved NUM_PAS_BUFS definition from enet.h to space.c and 
 *		changed NUM_PAS_BUFS to n_pas_bufs.
 *
 *	I004	9/21/87		DF	Intel
 *		Moved the enetintr routine out to file "enetintr.c". This
 *		is done so that enetl.c is common to MBI enet driver
 *		and MBII i530 driver.
 *		Added compile switch for the same purpose.
 *
 *	I005	9/24/87		DF	Intel
 *		Added code to handle "low on rb" situation.
 *
 *	I006	10/20/87	AP	Intel
 *		Rewrote most of data_ind handling, added a rsrv procedure
 *		(in enetrdwri.c), added flow control facilities to
 *		wsrv.  This fixed a number of problems including matching
 *		speeds with iNA (to prevent it discarding messages) and
 *		preventing a single TLI application comsuming too many
 *		buffers.
 *
 *	I007	04/13/88	rjs		Intel
 *		Made DATAGRAM_SIZE configurable by user.  Bug fixes for
 *		datagram support.
 *	I008	07/05/88	rjs		Intel
 *		Added code to queue disconnect indications by renaming
 *		old enet_discon_ind() to enet_discon_ind_continue() and
 *		adding new enet_discon_ind() which queues requests.
 *		enet_data_wait() was modified in order to pass message
 *		type to enetrsrv().
 *	I009	07/06/88	rjs		Intel
 *		Wakeup process in enetclose() when close pending and normal
 *		or expedited send has completed.
 *	I010	09/08/88	rjs		Intel
 *		Add support for ina_ver type 3.  (Similar to type 2).
 *	I011	09/14/88	rjs		Intel
 *		Implemented delay between data sends via configurable parameter
 *		SEND_DELAY in space.c.  The global variable enet_sdelay is
 *		initialized to SEND_DELAY in space.c.  If non-zero, the output
 *		stream is disabled for enet_sdelay/(HZ=100) seconds.  This
 *		fix is needed on connections to slow machines (e.g. PC's and
 *		XENIX 286/310) that are unable to post receive buffers fast
 *		enough for iNA961 R2.0 or R3.0 running on the local comm board.
 *		The back-off and retry timeout in these iNA versions is
 *		painfully slow.
 *	I012	09/16/88	rjs		Intel
 *		Fixed bug in enet_data_ind_complete() when zero-length message
 *		is received from XENIX session application; formerly caused
 *		MBII systems to panic and stream corruption on MBI systems.
 *	I013	01/18/89	nl		Intel
 *		Fixes incorrect handling of disconnect request.
 *	I014	07/11/89	rjf		Intel
 *		Made lint fixes.
 *	I015	08/09/89	rjf		Intel
 *		Fixes incorrect handling of disconnect request on listening ep.
 *	I016	09/12/89	rjf		Intel
 *		Fixed zero transaction problem.
 *	JB	30AUG89
 *		fixed McDD unexpected disconnect problem; endpoint was
 *		freed up too early
 *	I017	09/12/89	rjf		Intel
 *		Ignore c_resp.
 *	I018	09/27/89	rjf		Intel
 *		Set strstate to C_IDLE when no ?_reference.
*/

#define DEBUG 1
#include "sys/enet.h"
#include <sys/immu.h>


/*
 * The following are major variables defined in the /etc/master.d
 * file for enet:
 *
 * enet statistics structure defined in enet.c
 */
extern ulong		enet_stat[enet_SCNT];
extern char		ti_statetbl[TE_NOEVENTS][TS_NOSTATES];
extern endpoint		enet_endpoints[];
extern int		enet_n_endpoints;
extern int		enet_data_buf_len;	/* I003 */
extern int		enet_datagram_size;	/* I007 */
extern int		enet_max_bufs_posted; /* I005 */
extern int		enet_max_data_rq;
extern int		enet_rb_hi;		/* I005 */
extern int		enet_rb_lowat;		/* I005 */
extern int		enet_rbs_used;		/* I005 */
extern int		enet_z;			/* I005 */
extern int		enet_sdelay;		/* I011 */

extern int		enet_debug;

void enet_bind_req_complete();
void enet_do_bind_req_complete();
void enet_discon_ind_complete();
void enet_discon_req_complete();
void enet_conn_ind_complete();
void enet_conn_req_continue();
void enet_data_ok();
void enet_hdr_ok();
void enet_post_buffers();
void enet_expedited_data_ok();
void enet_data_ind_post();
void enet_data_ind_complete();
void enet_expedited_data_ind_complete();

/*
 * enet_restting  defined in enetutil.c
 */
extern int		enet_resetting;

/*
 * iNA961 verson number defined in iNA961.c      I000
 */
extern int		enet_ina_ver;

/*  Max size of message to copy to new buffer */

extern int		enet_maxcopysiz;


int enet_seqno;



/*
 * enet_conn_req
 *
 * Just pass the request to the board
 */
int
enet_conn_req(ep, mptr, conn)
register endpoint *ep;
mblk_t *mptr;
struct T_conn_req *conn;
{

	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_req()\n"));
	DEBUGC('?');
	/*
	 * If this endpoint can listen, then it is listening on l_reference,
	 * so we can't use that reference and must open another one.  If it
	 * can't listen, we can just use l_reference.
	 */
	if(ep->max_pend == 0) {
		ep->c_reference = ep->l_reference;
		ep->l_reference = 0;
		return(enet_do_send_conn_req(ep, mptr, conn));
	} else {
		struct orb *open_rb;

		ep->open_cont = enet_conn_req_continue;
		open_rb = (struct orb *)getrb(ep);
		if(open_rb == NULL)
			return(TSYSERR);
		open_rb->or_ep = ep;
		open_rb->or_message = mptr;
		if(iNA961_open(ep, open_rb)) {
			relrb((struct req_blk *)open_rb);
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_req: open failed\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_req => MIPERR\n"));
			return(MIPERR);
		}
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_req => 0\n"));
	return(0);
}

void
enet_conn_req_continue(open_rb)
struct orb *open_rb;
{
	endpoint *ep;
	mblk_t *mptr;
	int error;

	ep = open_rb->or_ep;
	mptr = open_rb->or_message;
	ep->c_reference = open_rb->or_reference;
	relrb((struct req_blk *)open_rb);
	if(error = enet_do_send_conn_req(ep, mptr,
			(struct T_conn_req *)mptr->b_rptr)) {
		ep->tli_state = ti_statetbl[TE_ERROR_ACK][ep->tli_state];
		switch(error) {
		case TSYSERR:
			enet_pterr(ep, mptr, error|HIGH_PRI, ENOSR);
			break;
		case MIPERR:
			enet_pterr(ep, mptr, error|HIGH_PRI, EIO);
			break;
		default:
			enet_pterr(ep, mptr, error|HIGH_PRI, 0);
			break;
		}
	}
}

int
enet_do_send_conn_req(ep, mptr, conn)
endpoint *ep;
mblk_t *mptr;
struct T_conn_req *conn;
{
	struct crrb *scr_rb;
	mblk_t *data;
	int	x;

	scr_rb = (struct crrb *)getrb(ep);
	if(scr_rb == NULL) {
		return(TSYSERR);
	}
	scr_rb->cr_message = getmptr(sizeof(struct T_conn_con) + MAX_NAME_LEN,
				     BPRI_MED, ep);
	if(scr_rb->cr_message == NULL) {
		relrb((struct req_blk *)scr_rb);
		return(TSYSERR);
	}
	/*
	 * Allocate a buffer to receive (and unfortunately also to send)
	 * any data (there may be up to max(CLOSE_BUF_LEN, CONN_BUF_LEN))
	 */
	scr_rb->cr_message->b_cont = getmptr(CONN_BUF_LEN, BPRI_MED, ep);
	if(scr_rb->cr_message->b_cont == NULL) {
		freemsg(scr_rb->cr_message);
		relrb((struct req_blk *)scr_rb);
		return(TSYSERR);
	}
	data = scr_rb->cr_message->b_cont;
	if(mptr->b_cont) {
		memcpy((char *)data->b_rptr, (char *)mptr->b_cont->b_rptr,
			mptr->b_cont->b_wptr - mptr->b_cont->b_rptr);
		data->b_wptr = data->b_rptr +
			(mptr->b_cont->b_wptr - mptr->b_cont->b_rptr);
	}
	/*
	 *
	 * Don't let the connection confirm come through until we've updated
	 * the tli_state.  Otherwise, conn_con gets invalid state.
	 */
	x = SPL();
	if(iNA961_send_connect_req(ep,
			(char *)(mptr->b_datap->db_base + conn->DEST_offset),
			(int)conn->DEST_length,
			scr_rb,
			data->b_rptr,
			(data->b_wptr - data->b_rptr))) {
		splx(x);
		freemsg(scr_rb->cr_message);
		relrb((struct req_blk *)scr_rb);
		return(MIPERR);
	}
	ep->tli_state = ti_statetbl[TE_OK_ACK1][ep->tli_state];
	splx(x);
	/* Now mark the service as connection oriented */
	/* ep->options |= OPT_COTS; */ /* I007 */
	enet_ok_ack(ep, mptr, 1);
	return(0);
}

/*
 * enet_conn_con
 *
 * Generate a T_CONN_CON message to the local user.
 *
 * Post await close and data receive requests to detect action on the
 * connection.
 *
 * This function automatically checks and updates the state of the
 * virtual endpoint.
 *
 * The reply to the user must be at high priority
 */
void
enet_conn_con(cr_rb)
struct crrb *cr_rb;
{
	int nextstate;
	register mblk_t *mptr;
	register endpoint *ep;
	mblk_t *databuf;
	struct vcrb *rd_rb, *c_rb;
	int	i;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_con()\n"));
	DEBUGC('&');
	mptr = cr_rb->cr_message;
	ep = cr_rb->cr_ep;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(mptr);
		relrb((struct req_blk *)cr_rb);
		return;
	}
	if(cr_rb->cr_crbh.c_resp == OK_CLOSED_RESP) {
		freemsg(mptr);
		relrb((struct req_blk *)cr_rb);
		return;
	}
	if(cr_rb->cr_crbh.c_resp != OK_RESP) {
		enet_discon_ind((struct vcrb *)cr_rb);
		return;
	}
	DEBUGC('~');
	if((nextstate = ti_statetbl[TE_CONN_CON][ep->tli_state]) == TS_INVALID){
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_con: nextstate invalid\n"));
		relrb((struct req_blk *)cr_rb);
	        enet_pterr(ep, mptr, TOUTSTATE, 0);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_con => NULL\n"));
		return;
	}
	ep->tli_state = nextstate;
	for (i=0; i < enet_max_bufs_posted; i++)		/* I001 I003 */
	{							/* I001 */
		databuf = getmptr(enet_data_buf_len, BPRI_MED, ep); /* I003 */
		if(databuf == NULL) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_con: alloc1 failed\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_con => NULL\n"));
			freemsg(mptr);
			relrb((struct req_blk *)cr_rb);
			return;
		}
							/* I005 */ 
		if (enet_rb_hi && (i == 0)) {		
			freemsg(databuf);
			freemsg(mptr);
			relrb((struct req_blk *)cr_rb);
			return;
		}
		rd_rb = (struct vcrb *)getrb(ep);
		if (rd_rb == NULL) {		
			freemsg(databuf);
			freemsg(mptr);
			relrb((struct req_blk *)cr_rb);
			return;
		}
		rd_rb->vc_databuf = databuf;
		((struct req_blk *)rd_rb)->databuf = databuf;	/* I004 */
		if(iNA961_receive_data(ep, ep, rd_rb,
				(unchar *)databuf->b_datap->db_base)) {
			freemsg(databuf);
			relrb((struct req_blk *)rd_rb);
			freemsg(mptr);
			relrb((struct req_blk *)cr_rb);
			return;
		}
	}							/* I001 */
	if (ep->options & OPT_EXPD)
	{
		databuf = getmptr(ETSDU_SIZE, BPRI_MED, ep);
		if(databuf == NULL) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_con: alloc2 failed\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_con => NULL\n"));
			freemsg(mptr);
			relrb((struct req_blk *)cr_rb);
			return;
		}
		rd_rb = (struct vcrb *)getrb(ep);
		if(rd_rb == NULL) {
			freemsg(databuf);
			freemsg(mptr);
			relrb((struct req_blk *)cr_rb);
			return;
		}
		rd_rb->vc_databuf = databuf;
		((struct req_blk *)rd_rb)->databuf = databuf; /* I004 */
		if (iNA961_receive_expedited_data(ep, ep, rd_rb,
						databuf->b_datap->db_base))
		{
			freemsg(databuf);
			relrb((struct req_blk *)rd_rb);
			freemsg(mptr);
			relrb((struct req_blk *)cr_rb);
			return;
		}
	}
	databuf = getmptr(CLOSE_BUF_LEN, BPRI_MED, ep);
	if(databuf == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_con: alloc failed\n"));
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_con => NULL\n"));
		freemsg(mptr);
		relrb((struct req_blk *)cr_rb);
		return;
	}
	c_rb = (struct vcrb *)getrb(ep);
	if(c_rb == NULL) {
		freemsg(databuf);
		freemsg(mptr);
		relrb((struct req_blk *)cr_rb);
		return;
	}
	((struct req_blk *)c_rb)->databuf = databuf; /* I004 */
	c_rb->vc_databuf = databuf;
	c_rb->vc_ep = ep;
	if(iNA961_await_close(ep, ep->c_reference,
				c_rb, databuf->b_datap->db_base)) {
		freemsg(databuf);
		relrb((struct req_blk *)c_rb);
		freemsg(mptr);
		relrb((struct req_blk *)cr_rb);
		return;
	}
	enet_stat[ST_TOTC]++;
	enet_stat[ST_CURC]++;
	mptr->b_cont->b_wptr = mptr->b_cont->b_rptr + cr_rb->cr_u_len;
	enet_conn_con_ack(ep,mptr,(char *)phystokv(*(char **)cr_rb->cr_tabufp));
	relrb((struct req_blk *)cr_rb);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_con => NULL\n"));
}

/*
 * enet_conn_res
 *
 * At this point we have up to three CDB's:
 *  1. The CDB which did the await_conn_req (now half connected)
 *  2. The CDB which is now acting as the listener (created by enet_conn_ind)
 *  3. The CDB for the stream which is accepting the connection (may be
 *     equal to 2)
 * At the end of this routine, the accepting stream should be tied to
 * the CDB which did the await, and the listener CDB should be unaffected.
 */
int
enet_conn_res(pending, listen_ep, accept_ep, mptr)
pend_list *pending;
endpoint *listen_ep;
endpoint *accept_ep;
mblk_t *mptr;
{
	int error;
	struct crrb *acr_rb;
	struct vcrb *c_rb;
	mblk_t *data;
	ushort	reference;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_res()\n"));
	DEBUGC(')');
	/*
	 * The accept_ep->l_reference is either a new one created due to
	 * the open of the accepting stream, in which case it is useless,
	 * or, the accept is being done on the listening stream, in
	 * which case, we have to close the new listening CDB
	 *
	 * Either way, the accept_ep->l_reference should be closed.
	 */
	c_rb = (struct vcrb *)getrb(accept_ep);
	if(c_rb == NULL) {
		return(TSYSERR);
	}
	c_rb->vc_databuf = NULL;
	DEBUGP(DEB_FULL,(CE_CONT, "enet_conn_res: closing ep = %x, ref = %x\n", 
			accept_ep, accept_ep->l_reference));
	iNA961_close(accept_ep, accept_ep->l_reference, c_rb,
		RSN_NORMAL, (unchar *)NULL, 0);
	accept_ep->l_reference = 0;
	/*
	 * Now tie the accepting stream to the connected CDB
	 */
	reference = pending->reference;
	/*
	 * Leave a trace to the listening stream for the interrupt handler
	 */
	accept_ep->listen_ep = listen_ep;
	/*
	 * Now accept the connection, sending any user data
	 */
	acr_rb = (struct crrb *)getrb(accept_ep);
	if(acr_rb == NULL)
		return(TSYSERR);
	acr_rb->cr_message = mptr;
	acr_rb->cr_pending = pending;
	data = mptr->b_cont;
	if(data != NULL)
		error = iNA961_accept_conn_req(accept_ep, acr_rb,
					      reference,
					      data->b_rptr,
					      data->b_wptr - data->b_rptr);
	else
		error = iNA961_accept_conn_req(accept_ep, acr_rb,
						reference,
						(unchar *)NULL, 0);
	if(error) {
		freemsg(data);
		relrb((struct req_blk *)acr_rb);
		error = MIPERR;
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_res => %x\n", error));
	return(error);
}

/*
 * enet_accept_conn_ack
 *
 * Interrupt driven response to accept_conn_req.
 * Must reply to user with a PCPROTO message (high priority)
 */
void
enet_accept_conn_ack(acr_rb)
struct crrb *acr_rb;
{
	mblk_t *mptr;
	endpoint *listen_ep, *accept_ep;
	struct vcrb *rd_rb;
	mblk_t *databuf;
	pend_list *prev, *pending;
	int	i;					/* I001 */

	DEBUGP(DEB_CALL,(CE_CONT, "enet_accept_conn_ack()\n"));
	mptr = acr_rb->cr_message;
	accept_ep = acr_rb->cr_ep;
	listen_ep = accept_ep->listen_ep;
	pending = acr_rb->cr_pending;
	DEBUGP(DEB_FULL,(CE_CONT, "enet_accept_conn_ack: accept_ep = %x\n", accept_ep));
	DEBUGP(DEB_FULL,(CE_CONT, "enet_accept_conn_ack: listen_ep = %x\n", listen_ep));
	DEBUGP(DEB_FULL,(CE_CONT, "enet_accept_conn_ack: accept_ep->c_reference = %x\n", 
			accept_ep->c_reference));
	DEBUGP(DEB_FULL,(CE_CONT, "enet_accept_conn_ack: accept_ep->str_state = %x\n", 
			accept_ep->str_state));
	if(accept_ep->str_state & (C_ERROR|C_IDLE)) {
		if(listen_ep->str_state & (C_ERROR|C_IDLE)) {
			freemsg(mptr);
			relrb((struct req_blk *)acr_rb);
		}
		else {
			relrb((struct req_blk *)acr_rb);
			listen_ep->tli_state =
				ti_statetbl[TE_ERROR_ACK][listen_ep->tli_state];
			enet_pterr(listen_ep, mptr, TOUTSTATE|HIGH_PRI, 0);
			if (accept_ep != listen_ep)		/* I016 */
				enet_pferr(accept_ep);		/* I016 */
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_accept_conn_ack => NULL #1\n"));
		return;
	}
	DEBUGP(DEB_FULL,(CE_CONT, "enet_accept_conn_ack: acr_rb->cr_crbh.c_resp = %x\n", 
		acr_rb->cr_crbh.c_resp));
	if((acr_rb->cr_crbh.c_resp == OK_CLOSED_RESP) ||
	   (acr_rb->cr_crbh.c_resp == REM_ABORT)) {
		if(listen_ep->str_state & (C_ERROR|C_IDLE)) {
			freemsg(mptr);
			relrb((struct req_blk *)acr_rb);
		}
		else {
			relrb((struct req_blk *)acr_rb);
			listen_ep->tli_state =
				ti_statetbl[TE_ERROR_ACK][listen_ep->tli_state];
			enet_pterr(listen_ep, mptr, TOUTSTATE|HIGH_PRI, 0);
			if (accept_ep != listen_ep)		/* I016 */
				enet_pferr(accept_ep);		/* I016 */
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_accept_conn_ack => NULL #2\n"));
		return;
	}
	if(acr_rb->cr_crbh.c_resp != OK_RESP) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_accept_conn_ack: bad resp %x\n",
				acr_rb->cr_crbh.c_resp));
		if(listen_ep->str_state & (C_ERROR|C_IDLE)) {
			freemsg(mptr);
			relrb((struct req_blk *)acr_rb);
		}
		else {
			relrb((struct req_blk *)acr_rb);
			listen_ep->tli_state =
				ti_statetbl[TE_ERROR_ACK][listen_ep->tli_state];
			enet_pterr(listen_ep, mptr, TSYSERR|HIGH_PRI, 0);
			if (accept_ep != listen_ep)		/* I016 */
				enet_pferr(accept_ep);		/* I016 */
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_accept_conn_ack => NULL #3\n"));
		return;
	}
	/*
	 * Remove the pending connect from the list of pending connections
	 * associated with the listening endpoint.
	 */
	if(listen_ep->pend_connects == pending)
		listen_ep->pend_connects = (pend_list *)pending->next;
	else {
		prev = NULL;
		for(pending = listen_ep->pend_connects; 
		    pending != NULL;
		    pending = (pend_list *)pending->next) {
			if(acr_rb->cr_pending == pending)
				break;
			prev = pending;
		}
		if(pending == NULL) {
			/*
			 * Can only get here if discon_ind has removed the
			 * pending connect from our list while the board
			 * had the accept rb.  In that case, the user
			 * already knows the connection died, just forget it.
			 */
			relpend(acr_rb->cr_pending);
			freemsg(mptr);
			relrb((struct req_blk *)acr_rb);
			return;
		}
		prev->next = pending->next;
	}
	listen_ep->nbr_pend--;
	relpend(pending);
	accept_ep->c_reference = *(ushort *)acr_rb->cr_reference;
	/*
	 * Post receive buffers for incoming data interrupts
	 */
	for (i=0; i < enet_max_bufs_posted; i++)		/* I001 I003 */
	{							/* I001 */
		databuf = getmptr(enet_data_buf_len, BPRI_MED, listen_ep); /* I003 */
		if(databuf == NULL) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_accept_conn_ack: alloc failed\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "enet_accept_conn_ack => NULL #4\n"));
			freemsg(mptr);
			return;
		}
							/* I005 */
		if (enet_rb_hi && (i == 0)) {		
			freemsg(databuf);
			freemsg(mptr);
			return;
		}
		rd_rb = (struct vcrb *)getrb(listen_ep);
		if (rd_rb == NULL) {
			freemsg(databuf);
			freemsg(mptr);
			return;
		}
		rd_rb->vc_databuf = databuf;
		((struct req_blk *)rd_rb)->databuf = databuf; /* I004 */
		if(iNA961_receive_data(accept_ep, listen_ep, rd_rb,
					 (unchar *)databuf->b_datap->db_base)) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_accept_conn_ack: RD failed\n"));
			freemsg(databuf);
			freemsg(mptr);
			relrb((struct req_blk *)rd_rb);
			DEBUGP(DEB_CALL,(CE_CONT, "enet_accept_conn_ack => NULL #5\n"));
			return;
		}
	}							/* I001 */
	if (accept_ep->options & OPT_EXPD)
	{
		databuf = getmptr(ETSDU_SIZE, BPRI_MED, listen_ep);
		if(databuf == NULL) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_accept_conn_ack: alloc2 failed\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "enet_accept_conn_ack => NULL #6\n"));
			freemsg(mptr);
			return;
		}
		rd_rb = (struct vcrb *)getrb(listen_ep);
		if(rd_rb == NULL) {
			freemsg(databuf);
			freemsg(mptr);
			return;
		}
		rd_rb->vc_databuf = databuf;
		((struct req_blk *)rd_rb)->databuf = databuf; /* I004 */
		if (iNA961_receive_expedited_data(accept_ep, listen_ep,
					rd_rb, databuf->b_datap->db_base))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_accept_conn_ack: ERD failed\n"));
			freemsg(databuf);
			freemsg(mptr);
			relrb((struct req_blk *)rd_rb);
			DEBUGP(DEB_CALL,(CE_CONT, "enet_accept_conn_ack => NULL #7\n"));
			return;
		}
	}
	/*
	 * Transition to the TS_DATA_XFER state.
	 */
	if(listen_ep == accept_ep) {
		listen_ep->tli_state = 
		    ti_statetbl[TE_OK_ACK2][listen_ep->tli_state];
	}
	else {
		register int event;
		event = (listen_ep->nbr_pend == 0) ? TE_OK_ACK3 : TE_OK_ACK4;
		listen_ep->tli_state = ti_statetbl[event][listen_ep->tli_state];
		DEBUGP(DEB_FULL,(CE_CONT, "enet_accept_conn_ack: listen_ep=%x listen_ep->tli_state=%x\n", listen_ep, listen_ep->tli_state));
		accept_ep->tli_state = 
			ti_statetbl[TE_PASS_CONN][accept_ep->tli_state];
		DEBUGP(DEB_FULL,(CE_CONT, "enet_accept_conn_ack: accept_ep=%x accept_ep->tli_state=%x\n", accept_ep, accept_ep->tli_state));
	}
	enet_stat[ST_TOTC]++;
	enet_stat[ST_CURC]++;
	enet_ok_ack(listen_ep, mptr, 1);	/* HIGH PRIORITY */
	relrb((struct req_blk *)acr_rb);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_accept_conn_ack => NULL.\n"));
}

/*
 * enet_discon_req
 *
 * We need to return to the IDLE state, which means we need to keep a
 * CDB.  If we are a listening endpoint, the CDB must be able to
 * receive connect requests.  There is no way to "unbind" a CDB on the
 * board, so just close the existing CDB, and start a new one.
 *
 * However, if the ep state is C_DISCON, we are here due to a close or an
 * abort, so just quit everything.
 *
 * Any replies to the user must be high priority
 */
int
enet_discon_req(ep, reference, pending, rsn, mptr)
register endpoint *ep;
ushort reference;
pend_list *pending;
int rsn;
register mblk_t *mptr;
{
	struct orb *open_rb;
	struct vcrb *c_rb;
	mblk_t *data;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_req()\n"));
	DEBUGC('H');
	data = unlinkb(mptr);
	c_rb = (struct vcrb *)getrb(ep);
	if(c_rb == NULL)
		return(TSYSERR);
	c_rb->vc_databuf = data;
	((struct req_blk *)c_rb)->databuf = data; /* I004 */
	iNA961_close(ep,
		     reference,
		     c_rb,
		     rsn,
		     (data==NULL)?(unchar *)NULL:data->b_rptr,
		     (data==NULL)?0:(data->b_wptr - data->b_rptr));
	if(pending) {
		relpend(pending);
		enet_ok_ack(ep, mptr, 1);	/* HIGH PRIORITY */
		return(0);
	}
	if(ep->c_reference != reference) {	/* I013 */
		enet_ok_ack(ep, mptr, 1);	/* HIGH PRIORITY */
		return(0);
	}
	if(ep->str_state & C_DISCON) {
		enet_ok_ack(ep, mptr, 1);	/* HIGH PRIORITY */
		return(0);
	}
	ep->open_cont = enet_discon_req_complete;
	open_rb = (struct orb *)getrb(ep);
	if(open_rb == NULL)
		return(TSYSERR);
	open_rb->or_ep = ep;
	open_rb->or_message = mptr;
	if(iNA961_open(ep, open_rb)) {
		relrb((struct req_blk *)open_rb);
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_req: open failed\n"));
		DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_req => MIPERR\n"));
		return(MIPERR);
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_req => 0\n"));
	return(0);
}

void
enet_discon_req_complete(o_rb)
struct orb *o_rb;
{
	mblk_t *databuf, *mptr;
	endpoint *ep;
	struct crrb *acr_rb;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_req_complete()\n"));
	mptr = o_rb->or_message;
	ep = o_rb->or_ep;
	enableok(WR(ep->rd_q));
	qenable(WR(ep->rd_q));
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(mptr);
		relrb((struct req_blk *)o_rb);
		return;
	}
	if(o_rb->or_crbh.c_resp != OK_RESP) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_req_complete: bad resp %x\n",
				o_rb->or_crbh.c_resp));
		relrb((struct req_blk *)o_rb);
		ep->tli_state = ti_statetbl[TE_ERROR_ACK][ep->tli_state];
		enet_pterr(ep, mptr, TSYSERR|HIGH_PRI, 0);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_req_complete => NULL\n"));
		return;
	}
	ep->l_reference = o_rb->or_reference;
	relrb((struct req_blk *)o_rb);
	/*
	 * If this is a listening endpoint, post a buffer for incoming
	 * connect requests
	 */
	if(ep->max_pend != 0) {
		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_req_complete:listening endpoint\n"));
		databuf = getmptr(CONN_BUF_LEN, BPRI_MED, ep);
		if(databuf == NULL) {
			freemsg(mptr);
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_req_complete: alloc failed\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_req_complete => NULL\n"));
			return;
		}
		acr_rb = (struct crrb *)getrb(ep);
		if(acr_rb == NULL) {
			freemsg(mptr);
			freemsg(databuf);
			return;
		}
		acr_rb->cr_databuf = databuf;
		if(iNA961_await_conn_req(ep, acr_rb, 
					 databuf->b_datap->db_base)) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_req_complete: ACR failed\n"));
			freemsg(mptr);
			freemsg(databuf);
			relrb((struct req_blk *)acr_rb);
			DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_req_complete => NULL\n"));
			return;
		}
	}
	switch(ep->tli_state) {
	case TS_WACK_DREQ7:
		if(ep->nbr_pend > 0)
			ep->tli_state = ti_statetbl[TE_OK_ACK4][ep->tli_state];
		else
			ep->tli_state = ti_statetbl[TE_OK_ACK2][ep->tli_state];
		break;
	case TS_WACK_DREQ9:
	case TS_WACK_DREQ10:
	case TS_WACK_DREQ11:
	case TS_WACK_DREQ6:
	default:
		ep->tli_state = ti_statetbl[TE_OK_ACK1][ep->tli_state];
		break;
	}
	enet_ok_ack(ep, mptr, 1);	/* HIGH PRIORITY */
	DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_req_complete => NULL\n"));
}

/*
 * enet_bind_req
 *
 * Call await connect req if this is to be a server endpoint (listening).
 * Due to the way the board handles connect requests, this is the only way we
 * can start queueing incoming connect requests.
 */
/*ARGSUSED*/
int
enet_bind_req(ep, mptr, bind)
endpoint *ep;
mblk_t *mptr;
struct T_bind_req *bind;
{
	struct orb *open_rb;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_req()\n"));
	ep->open_cont = enet_bind_req_complete;
	open_rb = (struct orb *)getrb(ep);
	if(open_rb == NULL) {
		return(TSYSERR);
	}
	open_rb->or_ep = ep;
	open_rb->or_message = mptr;
	if(iNA961_open(ep, open_rb)) {
		DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_req => open failed\n"));
		relrb((struct req_blk *)open_rb);
		return(MIPERR);
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_req => 0\n"));
	return(0);
}

void
enet_bind_req_complete(o_rb)
struct orb *o_rb;
{
	endpoint *ep;
	mblk_t *databuf, *mptr;
	struct crrb *acr_rb;
	struct drb *d_rb;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_req_complete()\n"));
	mptr = o_rb->or_message;
	ep = o_rb->or_ep;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(mptr);
		relrb((struct req_blk *)o_rb);
		return;
	}
	if(o_rb->or_crbh.c_resp != OK_RESP) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_bind_req_complete: bad resp (= %x)\n",
				o_rb->or_crbh.c_resp));
		DEBUGP(DEB_ERROR,(CE_CONT, " o_rb = %x, ep = %x\n", o_rb, ep));
		relrb((struct req_blk *)o_rb);
		ep->tli_state=ti_statetbl[TE_ERROR_ACK][ep->tli_state];
		enet_pterr(ep, mptr, TSYSERR, 0);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_req_complete => NULL\n"));
		return;
	}
	ep->l_reference = o_rb->or_reference;
	relrb((struct req_blk *)o_rb);

    /* I007 - don't need to post datagram buffer on VC-only endpoints */

    if ((enet_ina_ver != 1) && (ep->options & OPT_CLTS))	/* I000, I010 */
    {
	databuf = getmptr(enet_datagram_size, BPRI_MED, ep);	/* I007 */
	if(databuf == NULL) {
		freemsg(mptr);
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_bind_req_complete: alloc1 failed\n"));
		DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_req_complete => NULL\n"));
		return;
	}
	d_rb = (struct drb *)getrb(ep);
	if(d_rb == NULL) {
		freemsg(databuf);
		freemsg(mptr);
		return;
	}
	d_rb->dr_databuf = databuf;
	if(iNA961_receive_datagram(ep, d_rb,
			(unchar *)databuf->b_datap->db_base)) {
		freemsg(databuf);
		relrb((struct req_blk *)d_rb);
		freemsg(mptr);
		return;
	}
     }
	/*
	 * If this is a listening endpoint, post a buffer for incoming
	 * connect requests
	 */
	if(ep->max_pend != 0) {
		DEBUGP(DEB_FULL,(CE_CONT, "enet_bind_req_complete: listening endpoint\n"));
		databuf = getmptr(CONN_BUF_LEN, BPRI_MED, ep);
		if(databuf == NULL) {
			freemsg(mptr);
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_bind_req_complete: alloc fail\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_req_complete => NULL\n"));
			return;
		}
		acr_rb = (struct crrb *)getrb(ep);
		if(acr_rb == NULL) {
			freemsg(mptr);
			freemsg(databuf);
			return;
		}
		acr_rb->cr_databuf = databuf;
		if(iNA961_await_conn_req(ep, acr_rb,
					 databuf->b_datap->db_base)) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_bind_req_complete: ACR failed\n"));
			freemsg(mptr);
			freemsg(databuf);
			relrb((struct req_blk *)acr_rb);
			DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_req_complete => NULL\n"));
			return;
		}
	}
	ep->tli_state = ti_statetbl[TE_BIND_ACK][ep->tli_state];
	DEBUGP(DEB_FULL,(CE_CONT, "enet_bind_req_complete: ep=%x ep->tli_state=%x\n", ep, ep->tli_state));
	enet_bind_ack(ep, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_bind_req_complete => NULL\n"));
}

/*
 * enet_do_bind_req
 *
 * Same as enet_bind_req but without the state change or ack message
 */
void
enet_do_bind_req(ep)
register endpoint *ep;
{
	struct orb *open_rb;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_do_bind_req()\n"));
	ep->open_cont = enet_do_bind_req_complete;
	open_rb = (struct orb *)getrb(ep);
	if(open_rb == NULL)
		return;
	open_rb->or_ep = ep;
	if(iNA961_open(ep, open_rb)) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_do_bind_req: open failed\n"));
		relrb((struct req_blk *)open_rb);
		enet_pferr(ep);
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_do_bind_req => NULL\n"));
}

/*
 * enet_do_bind_req_complete
 *
 * Same as enet_bind_req_complete but without the state change or ack message
 */
void
enet_do_bind_req_complete(o_rb)
struct orb *o_rb;
{
	endpoint *ep;
	mblk_t *databuf;
	struct crrb *acr_rb;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_do_bind_req_complete()\n"));
	DEBUGC('p');
	ep = o_rb->or_ep;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		relrb((struct req_blk *)o_rb);
		return;
	}
	if(o_rb->or_crbh.c_resp != OK_RESP) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_do_bind_req_complete: bad resp %x\n",
				o_rb->or_crbh.c_resp));
		relrb((struct req_blk *)o_rb);
		enet_pferr(ep);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_do_bind_req_complete => NULL"));
		return;
	}
	ep->l_reference = o_rb->or_reference;
	relrb((struct req_blk *)o_rb);
	/*
	 * If this is a listening endpoint, post a buffer for incoming
	 * connect requests
	 */
	if(ep->max_pend == 0) {
		DEBUGP(DEB_CALL,(CE_CONT, "enet_do_bind_req_complete => NULL"));
		return;
	}
	DEBUGP(DEB_FULL,(CE_CONT, "enet_do_bind_req_complete: listening endpoint\n"));
	databuf = getmptr(CONN_BUF_LEN, BPRI_MED, ep);
	if(databuf == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_do_bind_req_complete: alloc failed\n"));
		DEBUGP(DEB_CALL,(CE_CONT, "enet_do_bind_req_complete => NULL"));
		return;
	}
	acr_rb = (struct crrb *)getrb(ep);
	if(acr_rb == NULL) {
		freemsg(databuf);
		return;
	}
	acr_rb->cr_databuf = databuf;
	if(iNA961_await_conn_req(ep, acr_rb, databuf->b_datap->db_base)) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_do_bind_req_complete: ACR failed\n"));
		freemsg(databuf);
		relrb((struct req_blk *)acr_rb);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_do_bind_req_complete => NULL"));
		return;
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_do_bind_req_complete => NULL\n"));
}

/*
 * enet_unbind_req
 *
 * Since there is really no "bind" at the board level, we just drop the
 * current CDB.
 */
/*ARGSUSED*/
int
enet_unbind_req(ep, mptr, unbind)
register endpoint *ep;
mblk_t *mptr;
struct T_unbind_req *unbind;
{
	struct vcrb *c_rb;
	struct drb *d_rb;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_unbind_req()\n"));
	c_rb = (struct vcrb *)getrb(ep);
	if(c_rb == NULL)
		return(TSYSERR);
	c_rb->vc_databuf = NULL;
	iNA961_close(ep, ep->l_reference, c_rb, RSN_NORMAL, (unchar *)NULL, 0);

    if ((enet_ina_ver != 1) && (ep->options & OPT_CLTS))	/* I000, I010 */
    {		
	d_rb = (struct drb *)getrb(ep);
	if(d_rb == NULL)
		return(TSYSERR);
	iNA961_withdraw_datagram(ep, d_rb);
    }
	ep->l_reference = 0;
	DEBUGP(DEB_CALL,(CE_CONT, "enet_unbind_req => NULL\n"));
	return(0);
}

/*
 * enet_data_req
 *
 * Pass the send data request to the board
 */
int
enet_data_req(ep, mptr, more)
endpoint *ep;
mblk_t *mptr;
int more;
{
	mblk_t	*nextp;
	unchar	*datap;
	int	len, error, x;
	struct	vcrb *dr_rb;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_data_req()\n"));
	if(more)
		DEBUGC('^');
	else
		DEBUGC('+');
	enet_stat[ST_SMSG]++;
	while(mptr != NULL) {
		nextp = unlinkb(mptr);
		datap = mptr->b_rptr;
		/* Is there anything in this block? */
		len = mptr->b_wptr - mptr->b_rptr;
		if(len != 0) {
			dr_rb = (struct vcrb *)getrb(ep);
			if(dr_rb == NULL) {
				freemsg(mptr);
				if(nextp)
					freemsg(nextp);
				return(TSYSERR);
			}
			dr_rb->vc_databuf = mptr;
			((struct req_blk *)dr_rb)->databuf = mptr; /* I004 */
			x = SPL();
			dr_rb->seqno = ++ep->req_seqno;
			if(error = iNA961_send_data(ep, datap, len, 
						   nextp?1:more, dr_rb)){
				freemsg(mptr);
				if(nextp)
					freemsg(nextp);
				ep->req_seqno--;
				splx(x);
				relrb((struct req_blk *)dr_rb);
				DEBUGP(DEB_CALL,(CE_CONT, "enet_data_req => %x\n", error));
				return(MIPERR);
			}
			ep->nbr_datarq++;
			splx(x);
			enet_stat[ST_BSNT] += len;
			enet_stat[ST_SPCK]++;
		}
		else
		{
			freeb(mptr);
		}
		mptr = nextp;
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_data_req => 0\n"));
	return(0);
}

void
enet_data_req_complete(dr_rb)
struct vcrb *dr_rb;
{
	endpoint *ep;
	mblk_t *mptr;

	void enet_restart_output();

	DEBUGP(DEB_CALL,(CE_CONT, "enet_data_req_complete()\n"));
	mptr = dr_rb->vc_databuf;
	ep = dr_rb->vc_ep;
	DEBUGC('R');
	ep->complete_seqno = dr_rb->seqno;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(mptr);
		relrb((struct req_blk *)dr_rb);
		ep->nbr_datarq--;
		return;
	}
	else if (ep->str_state & C_DISCON)		/* I009 */
		wakeup((caddr_t) ep);
	/*
	 * The user has already returned, all we can do is
	 * check the return value and send discon_ind for
	 * broken connections.
	 */
	if(dr_rb->vc_crbh.c_resp == NO_RESOURCES) {
		/*
		 * Note: if we got a no resources, we assume there are
		 * other requests outstanding which will eventually
		 * finish, and re-enable the queue.
		 */
		DEBUGP(DEB_FULL,(CE_CONT, "enet_data_req_complete: NO_RESOURCES\n"));
		relrb((struct req_blk *)dr_rb);
		enet_stat[ST_NORES]++;
		noenable(WR(ep->rd_q));
		if(ep->flow) {
			DEBUGC('M');
			linkb(ep->flow, mptr);
		}
		else {
			DEBUGC('N');
			ep->flow = mptr;
		}
		ep->nbr_datarq--;
		return;
	}
	if(dr_rb->vc_crbh.c_resp == OK_CLOSED_RESP) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_data_req_complete: OK_CLOSED_RESP\n"));
		if(ep->flow) {
			freemsg(ep->flow);
			ep->flow = NULL;
			enableok(WR(ep->rd_q));
			qenable(WR(ep->rd_q));
		}
		freemsg(mptr);
		relrb((struct req_blk *)dr_rb);
		ep->nbr_datarq--;
		return;
	}
	if(dr_rb->vc_crbh.c_resp != OK_RESP) {
		/* Don't print a message if we've been here before */
		if(ep->tli_state != TS_IDLE)
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_data_req_complete: bad resp %x, state %x\n",
				dr_rb->vc_crbh.c_resp,
				ep->tli_state));
		if(ep->flow) {
			freemsg(ep->flow);
			ep->flow = NULL;
			enableok(WR(ep->rd_q));
			qenable(WR(ep->rd_q));
		}
		enet_discon_ind((struct vcrb *)dr_rb);
		ep->nbr_datarq--;
		return;
	}
	freemsg(mptr);
	relrb((struct req_blk *)dr_rb);
	if(ep->flow) {
		putbq(WR(ep->rd_q),ep->flow);
		enableok(WR(ep->rd_q));
		qenable(WR(ep->rd_q));
		ep->flow = NULL;
	}
	ep->nbr_datarq--;

	/* enetwsrv(WR(ep->rd_q)); */		/* Begin I011 */
	if (enet_sdelay)
	{
		noenable(WR(ep->rd_q));
		timeout(enet_restart_output, (caddr_t)WR(ep->rd_q), enet_sdelay);
	}
	else
		qenable(WR(ep->rd_q));		/* End I011 */

	DEBUGP(DEB_CALL,(CE_CONT, "enet_data_req_complete => NULL\n"));
}

/*
 * enet_expedited_data_req
 *
 * Pass the send expedited data request to the board
 */
int
enet_expedited_data_req(ep, mptr)
endpoint *ep;
mblk_t *mptr;
{
	mblk_t	*nextp;
	unchar	*datap;
	int	len, error, x;
	struct	vcrb *dr_rb;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_req()\n"));
	DEBUGC('e');
	enet_stat[ST_SEXP]++;
	while(mptr != NULL) {
		nextp = unlinkb(mptr);
		datap = mptr->b_datap->db_base;
		/* Is there anything in this block? */
		len = mptr->b_wptr - mptr->b_rptr;
		if(len != 0) {
			dr_rb = (struct vcrb *)getrb(ep);
			if(dr_rb == NULL) {
				freemsg(mptr);
				if(nextp)
					freemsg(nextp);
				return(TSYSERR);
			}
			dr_rb->vc_databuf = mptr;
			((struct req_blk *)dr_rb)->databuf = mptr; /* I004 */
			x = SPL();
			dr_rb->seqno = ++ep->ex_req_seqno;
			if(error = iNA961_send_expedited_data(ep, datap,
								len, dr_rb)){
				freemsg(mptr);
				if(nextp)
					freemsg(nextp);
				ep->ex_req_seqno--;
				splx(x);
				relrb((struct req_blk *)dr_rb);
				DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_req => %x\n", error));
				return(MIPERR);
			}
			splx(x);
			enet_stat[ST_ESNT] += len;
			enet_stat[ST_EPCK]++;
		}
		mptr = nextp;
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_req => NULL\n"));
	return(0);
}

void
enet_expedited_data_req_complete(dr_rb)
struct vcrb *dr_rb;
{
	endpoint *ep;
	mblk_t *mptr;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_req_complete()\n"));
	mptr = dr_rb->vc_databuf;
	ep = dr_rb->vc_ep;
	DEBUGC('r');
	ep->ex_complete_seqno = dr_rb->seqno;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(mptr);
		relrb((struct req_blk *)dr_rb);
		return;
	}
	else if (ep->str_state & C_DISCON)		/* I009 */
		wakeup((caddr_t) ep);
	/*
	 * The user has already returned, all we can do is
	 * check the return value and send discon_ind for
	 * broken connections.
	 */
	if(dr_rb->vc_crbh.c_resp == NO_RESOURCES) {
		/*
		 * Note: if we got a no resources, we assume there are
		 * other requests outstanding which will eventually
		 * finish.  However, we can't stop the flow of expedited
		 * messages, so we just keep queuing them internally.
		 */
		DEBUGP(DEB_FULL,(CE_CONT, "enet_expedited_data_req_complete: NO_RESOURCES\n"));
		relrb((struct req_blk *)dr_rb);
		enet_stat[ST_NOERES]++;
		if(ep->ex_flow) {
			DEBUGC('m');
			linkb(ep->ex_flow, mptr);
		}
		else {
			DEBUGC('n');
			ep->ex_flow = mptr;
		}
		return;
	}
	if(dr_rb->vc_crbh.c_resp == OK_CLOSED_RESP) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_req_complete: OK_CLOSED_RESP\n"));
		if(ep->ex_flow) {
			freemsg(ep->ex_flow);
			ep->ex_flow = NULL;
		}
		freemsg(mptr);
		relrb((struct req_blk *)dr_rb);
		return;
	}
	if(dr_rb->vc_crbh.c_resp != OK_RESP) {
		/* Don't print a message if we've been here before */
		if(ep->tli_state != TS_IDLE)
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_exedited_data_req_complete: bad resp %x, state %x\n",
				dr_rb->vc_crbh.c_resp,
				ep->tli_state));
		if(ep->ex_flow) {
			freemsg(ep->ex_flow);
			ep->ex_flow = NULL;
		}
		enet_discon_ind((struct vcrb *)dr_rb);
		return;
	}
	freemsg(mptr);
	relrb((struct req_blk *)dr_rb);
	if(ep->ex_flow) {
		mblk_t *tmp;

		tmp = ep->ex_flow;
		ep->ex_flow = NULL;
		if(enet_expedited_data_req(ep, tmp)) {
			freemsg(ep->ex_flow);
			ep->ex_flow = 0;
			return;
		}
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_exedited_data_req_complete => NULL\n"));
}

/*
 * enet_expedited_data_ind
 *
 * Generate a T_EXDATA_IND message to the local user.
 *
 * This function automatically checks and updates the state of the
 * virtual endpoint.
 */
void
enet_expedited_data_ind(vc_rb)
struct vcrb *vc_rb;
{
	register endpoint *ep;
	register mblk_t *mptr;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ind()\n"));
	DEBUGC('\'');
	ep = vc_rb->vc_ep;
	if((ep->str_state & (C_ERROR|C_IDLE)) ||
	   (vc_rb->vc_crbh.c_resp == UNKNOWN_REFERENCE)) {
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		return;
	}
	/* If the receive failed, just ignore it.  If the failure is connection
	 * loss, await_close will catch it */
	if(vc_rb->vc_crbh.c_resp != OK_EOM_RESP) {
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		if(vc_rb->vc_crbh.c_resp == REM_ABORT) {
			DEBUGP(DEB_FULL,(CE_CONT, "enet_expedited_data_ind: bad resp\n"));
			DEBUGP(DEB_FULL,(CE_CONT, "enet_expedited_data_ind: ep = %x, ep->tli_state = %x\n",
				ep, ep->tli_state));
			DEBUGP(DEB_FULL,(CE_CONT, "enet_expedited_data_ind: str_state = %x, c_resp = %x\n",
				ep->str_state, vc_rb->vc_crbh.c_resp));
		}
		else {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ind: bad resp\n"));
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ind: ep = %x, ep->tli_state = %x\n",
				ep, ep->tli_state));
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ind: str_state = %x, c_resp = %x\n",
				ep->str_state, vc_rb->vc_crbh.c_resp));
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ind => NULL\n"));
		return;
	}
	if(ti_statetbl[TE_EXDATA_IND][ep->tli_state] == TS_INVALID){
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ind: nextstate invalid\n"));
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ind => NULL\n"));
		return;
	}
	mptr = getmptr(sizeof(struct T_exdata_ind),BPRI_HI,ep);
	if(mptr == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ind: alloc1 failed\n"));
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ind => NULL\n"));
		return;
	}
	/*
	 * Make sure we can get a buffer to post back to the board for the
	 * next data ind.
	 */
	mptr->b_cont = vc_rb->vc_databuf;
	vc_rb->vc_databuf = allocb(ETSDU_SIZE, BPRI_HI);
	((struct req_blk *)vc_rb)->databuf = vc_rb->vc_databuf; /* I004 */
	if(vc_rb->vc_databuf == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ind: alloc3 failed\n"));
		DEBUGC(',');
		enet_stat[ST_ALFA]++;
		vc_rb->vc_message = mptr;
		if(!bufcall(ETSDU_SIZE,BPRI_HI,enet_expedited_data_ok,vc_rb)) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ind: buffcall failed\n"));
			freemsg(vc_rb->vc_databuf);
			relrb((struct req_blk *)vc_rb);
			freemsg(mptr);
			return;
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ind => NULL\n"));
		return;
	}
	enet_expedited_data_ind_complete(ep, mptr, vc_rb);
}

void
enet_expedited_data_ok(vc_rb)
struct vcrb *vc_rb;
{
	register endpoint *ep;
	register mblk_t *mptr;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ok()\n"));
	DEBUGC('\'');
	ep = vc_rb->vc_ep;
	mptr = vc_rb->vc_message;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		freemsg(mptr);
		return;
	}
	/*
	 * Is there really a buffer big enough?
	 */
	vc_rb->vc_databuf = allocb(ETSDU_SIZE, BPRI_HI);
	((struct req_blk *)vc_rb)->databuf = vc_rb->vc_databuf; /* I004 */
	if(vc_rb->vc_databuf == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ok: alloc3 failed\n"));
		DEBUGC(',');
		enet_stat[ST_ALFA]++;
		if(!bufcall(ETSDU_SIZE,BPRI_HI,enet_expedited_data_ok,vc_rb)) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ok: buffcall failed\n"));
			freemsg(vc_rb->vc_databuf);
			relrb((struct req_blk *)vc_rb);
			freemsg(mptr);
			return;
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ok => NULL\n"));
		return;
	}
	enet_expedited_data_ind_complete(ep, mptr, vc_rb);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ok => NULL\n"));
}

void
enet_expedited_data_ind_complete(ep, mptr, vc_rb)
endpoint *ep;
register mblk_t *mptr;
register struct vcrb *vc_rb;
{
	int size;
	int nextstate;
	struct T_exdata_ind *ind;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ind_complete()\n"));
	DEBUGC('.');
	size = *(ushort *)vc_rb->vc_count;
	mptr->b_cont->b_wptr = mptr->b_cont->b_rptr + size;
	mptr->b_datap->db_type = M_PCPROTO;
	ind = (struct T_exdata_ind *)mptr->b_rptr;
	ind->PRIM_type = T_EXDATA_IND;
	ind->MORE_flag = 0;
	mptr->b_wptr += sizeof(struct T_exdata_ind);
	/*
	 * Post the next receive buffer
	 */
	if(iNA961_receive_expedited_data(ep, ep, vc_rb,
					 vc_rb->vc_databuf->b_rptr)) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_expedited_data_ind: EX_RD failed\n"));
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		freemsg(mptr);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ind => NULL\n"));
		return;
	}
	nextstate = ti_statetbl[TE_EXDATA_IND][ep->tli_state];
	ep->tli_state = nextstate;
	enet_stat[ST_REXP]++;
	enet_stat[ST_ERCV] += size;
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_expedited_data_ind => NULL\n"));
}

int
enet_unitdata_req(ep, mptr, udata)
endpoint *ep;
mblk_t *mptr;
struct T_unitdata_req *udata;
{
	struct drb *d_rb;
	mblk_t *data;
	mblk_t	*nextp;
	unchar	*datap;
	int	len, error;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_unitdata_req()\n"));
	enet_stat[ST_SUNI]++;
	data = mptr;
	while(data != NULL) {			/* I007 - rewrite */
		nextp = unlinkb(data);
		datap = data->b_datap->db_base;
		/* Is there anything in this block? */
		len = data->b_wptr - data->b_rptr;
		DEBUGP(DEB_FULL,(CE_CONT, "  nextp=%x,datap=%x,len=%x,address=%x\n",
			nextp, datap, len, ((char *)udata+udata->DEST_offset)));
		if(len != 0) {
			d_rb = (struct drb *)getrb(ep);
			if(d_rb == NULL) {
				freemsg(data);
				if(nextp)
					freemsg(nextp);
				return(TSYSERR);
			}
			d_rb->dr_databuf = data;
			if(error = iNA961_send_datagram(ep,
				  ((char *)udata + udata->DEST_offset),
				  d_rb, datap, len)) {
				freemsg(data);
				if(nextp)
					freemsg(nextp);
				relrb((struct req_blk *)d_rb);
				DEBUGP(DEB_CALL,(CE_CONT, "enet_unitdata_req => %x\n", error));
				return(MIPERR);
			}
			enet_stat[ST_USNT] += len;
			enet_stat[ST_UPCK]++;
		}
		data = nextp;
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_unitdata_req => 0\n"));
	return(0);
}

void
enet_send_datagram_complete(d_rb)
struct drb *d_rb;
{
	mblk_t *mptr;
#if DEBUG
	int i;
#endif

	DEBUGP(DEB_CALL,(CE_CONT, "enet_send_datagram_complete()\n"));
#if DEBUG
	DEBUGP(DEB_FULL,(CE_CONT, " INCOMING REQUEST BLOCK:\n"));
	for (i=0;i<80;i++)
	{
		if (i%16 == 0)
			DEBUGP(DEB_FULL,(CE_CONT, "\n"));
		DEBUGP(DEB_FULL,(CE_CONT, " %x", (unsigned char) *((char*)d_rb+i)));
	}
	DEBUGP(DEB_FULL,(CE_CONT, "\n"));
#endif
	mptr = d_rb->dr_databuf;
	DEBUGC('u');
	/*
	 * Since datagrams are unguaranteed, ignore any failures
	 */
	freemsg(mptr);
	relrb((struct req_blk *)d_rb);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_data_req_complete => NULL\n"));
}

/*
 * enet_datagram_ind
 *
 * Generate a T_UNITDATA_IND message to the local user.
 *
 * This function automatically checks and updates the state of the
 * virtual endpoint.
 */
void
enet_datagram_ind(d_rb)
struct drb *d_rb;
{
	register endpoint *ep;
	int nextstate;
	register mblk_t *mptr;
	int	size;
	struct T_unitdata_ind *ind;
	int	addr_length;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_datagram_ind()\n"));
	DEBUGC('_');
	ep = d_rb->dr_ep;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(d_rb->dr_databuf);
		relrb((struct req_blk *)d_rb);
		return;
	}
	/* If the receive failed, just ignore it, and post another one */
	if((d_rb->dr_crbh.c_resp != OK_RESP) &&
	   (d_rb->dr_crbh.c_resp != OK_EOM_RESP)) {
		freemsg(d_rb->dr_databuf);
		relrb((struct req_blk *)d_rb);
		if(d_rb->dr_crbh.c_resp == OK_WITHDRAW_RESP) {
			DEBUGP(DEB_FULL,(CE_CONT, "enet_datagram_ind: bad resp\n"));
			DEBUGP(DEB_FULL,(CE_CONT, "enet_datagram_ind: ep = %x, ep->tli_state = %x\n",
				ep, ep->tli_state));
			DEBUGP(DEB_FULL,(CE_CONT, "enet_datagram_ind: str_state = %x, c_resp = %x\n",
				ep->str_state, d_rb->dr_crbh.c_resp));
		}
		else {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_datagram_ind: bad resp\n"));
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_datagram_ind: ep = %x, ep->tli_state = %x\n",
				ep, ep->tli_state));
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_datagram_ind: str_state = %x, c_resp = %x\n",
				ep->str_state, d_rb->dr_crbh.c_resp));
		}
		return;
	}
	if((nextstate = ti_statetbl[TE_UNITDATA_IND][ep->tli_state]) ==
			TS_INVALID){
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_datagram_ind: nextstate invalid\n"));
		freemsg(d_rb->dr_databuf);
		relrb((struct req_blk *)d_rb);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_datagram_ind => NULL\n"));
		return;
	}
	addr_length=ADDR_LENGTH(REM_ADDR(phystokv(*(char **)d_rb->dr_tabufp)));
	mptr = getmptr((int)(sizeof(struct T_unitdata_ind)+addr_length),
			BPRI_MED,ep);
	if(mptr == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_datagram_ind: alloc1 failed\n"));
		freemsg(d_rb->dr_databuf);
		relrb((struct req_blk *)d_rb);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_datagram_ind => NULL\n"));
		return;
	}
	/*
	 * Make sure we can get a buffer to pass the data to the user in
	 * (Don't use the one we posted, it may be way too big!)
	 * If we can't get the buffer, just forget it.  We never promised
	 * delivery.
	 */
	size = *(ushort *)d_rb->dr_count;
	if(size > 0) {
		mptr->b_cont = allocb(size,BPRI_MED);
		if(mptr->b_cont == NULL) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_datagram_ind: alloc2 failed\n"));
			DEBUGC('@');
			enet_stat[ST_ALFA]++;
			freemsg(d_rb->dr_databuf);
			relrb((struct req_blk *)d_rb);
			freemsg(mptr);
			return;
		}
		mptr->b_cont->b_datap->db_type = M_DATA;
		memcpy((char *)mptr->b_cont->b_wptr,
			(char *)phystokv(*(char **)(d_rb->dr_bufp)),
			size);
		mptr->b_cont->b_wptr = mptr->b_cont->b_rptr + size;
	}
	mptr->b_datap->db_type = M_PROTO;
	ind = (struct T_unitdata_ind *)mptr->b_rptr;
	ind->PRIM_type = T_UNITDATA_IND;
	ind->SRC_length = addr_length;
	ind->SRC_offset = sizeof(struct T_unitdata_ind);
	ind->OPT_length = 0;
	ind->OPT_offset = 0;
	memcpy((char *)mptr->b_rptr+sizeof(struct T_unitdata_ind),
	       REM_ADDR((char *)phystokv(*(char **)d_rb->dr_tabufp)),
	       addr_length);
	mptr->b_wptr+=sizeof(struct T_unitdata_ind) + addr_length;
	/*
	 * All done with the request block and the receive buffer, repost 
	 */
	if(iNA961_receive_datagram(ep, d_rb,
			(unchar *)phystokv(*(char **)(d_rb->dr_bufp)))) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_datagram_ind: RD failed\n"));
		freemsg(d_rb->dr_databuf);
		relrb((struct req_blk *)d_rb);
		freemsg(mptr);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_datagram_ind => NULL\n"));
		return;
	}
	nextstate = ti_statetbl[TE_UNITDATA_IND][ep->tli_state];
	ep->tli_state = nextstate;
	enet_stat[ST_RUNI]++;
	enet_stat[ST_URCV] += size;
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_datagram_ind => NULL\n"));
	return;
}

void
enet_close_req_complete(cl_rb)
struct vcrb *cl_rb;
{
	mblk_t *mptr;
#ifdef JB
	endpoint *ep;	/* JB */
#endif

	DEBUGP(DEB_CALL,(CE_CONT, "enet_close_req_complete()\n"));
	mptr = cl_rb->vc_databuf;
	/*
	 * The user has already returned, and won't do another
	 * request.  The best we can do is indicate that 
	 * problems are occuring if there is an error
	 */
	DEBUGP(DEB_FULL,(CE_CONT, "enet_close_req_complete: c_resp=%x\n", cl_rb->vc_crbh.c_resp));
	if(cl_rb->vc_crbh.c_resp == UNKNOWN_REFERENCE) {
		DEBUGP(DEB_FULL,(CE_CONT, "enet_close_req_complete: UNK_REF\n"));
		enet_stat[ST_UNKCLO]++;
	}
	if(mptr)
		freemsg(mptr);

#ifdef JB
	/* Inserted by JB. Free up the endpoint when we have seen a
	 * CLOSE RB returning for both c_reference and l_reference
	 */

	ep = cl_rb->vc_ep;
	DEBUGP(DEB_FULL,(CE_CONT, "enet_close_req_complete: ep=%x, ep->tli_state=%x, ep->str_state=%x\n",
				ep, ep->tli_state, ep->str_state));
	DEBUGP(DEB_FULL,(CE_CONT, "enet_close_req_complete: cl_rb->vc_reference=%x\n",
				cl_rb->vc_reference));
	DEBUGP(DEB_FULL,(CE_CONT, "enet_close_req_complete: ep->l_reference=%x, ep->c_reference=%x\n",
				ep->l_reference, ep->c_reference));
/*	if((cl_rb->vc_crbh.c_resp == OK_RESP) && (ep->str_state &C_DISCON)) {*/
	if (ep->str_state & C_DISCON) {				/* I017 */
		if (cl_rb->vc_reference == ep->l_reference)
			ep->l_reference = 0;
		if (cl_rb->vc_reference == ep->c_reference)
			ep->c_reference = 0;
		if ((ep->c_reference == 0) && (ep->l_reference == 0)) {
			DEBUGP(DEB_FULL,(CE_CONT, "enet_close_req_complete: setting ep->str_state to C_IDLE\n"));
			ep->tli_state = 0;
			ep->str_state = C_IDLE;
		}
	  }

	/* End of JB insertion.
	 */
#endif

	relrb((struct req_blk *)cl_rb);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_close_req_complete => NULL\n"));
}

void
enet_withdraw_datagram_complete(d_rb)
struct drb *d_rb;
{
	DEBUGP(DEB_CALL,(CE_CONT, "enet_withdraw_datagram_complete()\n"));
	/*
	 * The user has already returned, and won't do another
	 * request.  There's not much we can do.
	 */
	relrb((struct req_blk *)d_rb);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_withdraw_datagram_complete => NULL\n"));
}

/*
 * enet_abort
 *
 * Abort all connections associated with the endpoint.
 * Do a flushing disconnect whether the user asked for it or not.
 */
void
enet_abort(ep)
register endpoint *ep;
{
	struct vcrb *c_rb;
	struct drb *d_rb;
	register pend_list *pending, *next;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_abort()\n"));
	DEBUGC('A');
	/*
	 *	1. Flush any output not yet processed.
	 *	2. Throw away any output being sent.
	 *	3. Make sure the write queue is available for
	 *		scheduling by STREAMS.
	 */
	/* ### TEMP - Just in case */
	DEBUGP(DEB_FULL,(CE_CONT, "enet_abort: ep = %x\n", ep))
	if (ep == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_abort: abort of NULL ep\n"));
		return;
	}
	/* ### TEMP */
	if(ep->flow)
		freemsg(ep->flow);
	ep->flow = NULL;
	flushq(WR(ep->rd_q), FLUSHALL);
	enableok(WR(ep->rd_q));
	qenable(WR(ep->rd_q));
	if (enet_resetting)
	{
		enet_resetting = 0;
		return;
	}
	/*
	 * Indicate that the stream is closing down
	 */
	ep->str_state |= C_DISCON;
	/*
	 * If we were listening, take down any pending connections
	 */
	pending = ep->pend_connects; 
	while(pending != NULL) {
		next = pending->next;
		c_rb = (struct vcrb *)getrb(ep);
		if(c_rb == NULL)
			break;
		c_rb->vc_databuf = NULL;
		iNA961_close(ep, pending->reference, c_rb,
				RSN_NORMAL, (unchar *)NULL, 0);
		relpend(pending);
		pending = next;
	}
	ep->pend_connects = NULL;
#ifdef JB
	if ((ep->c_reference == 0) && (ep->l_reference == 0)) {	/* I018 */
		ep->tli_state = 0;				/* I018 */
		ep->str_state = C_IDLE;				/* I018 */
	}							/* I018 */
	else {							/* I018 */
		/*
	 	* And take down the main CDB(s)
	 	*/
		if(ep->l_reference) {
			c_rb = (struct vcrb *)getrb(ep);
			if(c_rb != NULL) {
				c_rb->vc_databuf = NULL;
				iNA961_close(ep, ep->l_reference, c_rb,
						RSN_NORMAL, (unchar *)NULL, 0);
			}
		}
		/* ep->l_reference = 0; */	/* JB */
		if(ep->c_reference) {
			c_rb = (struct vcrb *)getrb(ep);
			if(c_rb != NULL) {
				c_rb->vc_databuf = NULL;
				iNA961_close(ep, ep->c_reference, c_rb,
						RSN_NORMAL, (unchar *)NULL, 0);
			}
		}
		/* ep->c_reference = 0; */	/* JB */
	}							/* I018 */
#else
	/*
	 * And take down the main CDB(s)
	 */
	if(ep->l_reference) {
		c_rb = (struct vcrb *)getrb(ep);
		if(c_rb != NULL) {
			c_rb->vc_databuf = NULL;
			iNA961_close(ep, ep->l_reference, c_rb,
					RSN_NORMAL, (unchar *)NULL, 0);
		}
	}
	ep->l_reference = 0;
	if(ep->c_reference) {
		c_rb = (struct vcrb *)getrb(ep);
		if(c_rb != NULL) {
			c_rb->vc_databuf = NULL;
			iNA961_close(ep, ep->c_reference, c_rb,
					RSN_NORMAL, (unchar *)NULL, 0);
		}
	}
	ep->c_reference = 0;
#endif
    if ((enet_ina_ver != 1) && (ep->options & OPT_CLTS))
    {
	/*
	 * Stop listening for datagrams
	 */
	d_rb = (struct drb *)getrb(ep);
	if(d_rb != NULL)
		iNA961_withdraw_datagram(ep, d_rb);
    }
	DEBUGP(DEB_CALL,(CE_CONT, "enet_abort => NULL\n"));
}

void					/* begin I008 */
enet_discon_ind(vc_rb)
struct	vcrb	*vc_rb;
{
	endpoint *ep;
	mblk_t	*mp;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind()\n"));
	DEBUGC('.');
	ep = vc_rb->vc_ep;

	if((ep->str_state & (C_ERROR|C_IDLE|C_DISCON)) ||
	   (vc_rb->vc_crbh.c_resp == UNKNOWN_REFERENCE))
	{
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		if (ep->str_state & C_DISCON)		/* I009 */
			wakeup((caddr_t) ep);
	}
	else if (!(mp = allocb(sizeof(struct vcp), BPRI_HI)))
	{
		if(!bufcall(sizeof(struct vcp), BPRI_HI, enet_discon_ind, vc_rb))
		{
			cmn_err(CE_WARN, "Ethernet Driver: discon_ind - buffcall failed\n");
			freemsg(vc_rb->vc_databuf);
			relrb((struct req_blk *)vc_rb);
		}
		else
			DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind => NULL\n"));
	}
	else
	{
		((struct vcp *)(mp->b_rptr))->type = T_DISCON_IND;
		((struct vcp *)(mp->b_rptr))->vp = vc_rb;
		((struct vcp *)(mp->b_rptr))->post = NULL;
		mp->b_wptr += sizeof(struct vcp);
		putq(ep->rd_q, mp);
	}
}

void
enet_discon_ind_continue(rb_p)		/* end I008 */
struct req_blk *rb_p;
{
	register endpoint *ep = NULL;
	mblk_t *mptr, *databuf;
	struct T_discon_ind *ind;
	ushort reference;
	int nextstate;
	ushort size;
	struct vcrb *c_rb;
	pend_list *pending;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_continue()\n"));
	DEBUGC('Z');
	switch(((crbh *)rb_p)->c_opcode) {
	case OP_ACLOSE:
		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: close\n"));
		reference = rb_p->u.vc_rb.vc_reference;
		databuf = rb_p->u.vc_rb.vc_databuf;
		/*
		 * Is this a simple close on the endpoint?
		 */
		if(rb_p->u.vc_rb.vc_ep->c_reference == reference) {
			ep = rb_p->u.vc_rb.vc_ep;
			break;
		}
		/*
		 * Is it a close on a pending connection?
		 */
		for(pending = rb_p->u.vc_rb.vc_ep->pend_connects; 
		    pending != NULL;
		    pending = (pend_list *)pending->next) {
			if(reference == pending->reference) {
				ep = rb_p->u.vc_rb.vc_ep;
				break;
			}
		}
		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: pending ep = %x\n",
									ep))
		/*
		 * No, it must be a close on an accepted connection, but
		 * then the endpoint in the request is the listener, not
		 * the accepter.  Find the accepter.
		 */
		if(ep == NULL) {
			int i;

			for (i=0;i < enet_n_endpoints;i++){
				if((enet_endpoints[i].c_reference ==
								reference) &&
				   (enet_endpoints[i].listen_ep ==
							 rb_p->u.vc_rb.vc_ep)) {
						ep = &enet_endpoints[i];
						break;
				}
			}
			if (i == enet_n_endpoints) {
				/*
				 * Can only get here if the discon_ind is due
				 * to our own abort (which cleared out all
				 * the references).  So, this must be the
				 * right endpoint.
				 */
				ep = rb_p->u.vc_rb.vc_ep;
			}
	DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: accepted conn ep = %x\n",
									ep))
		}
		break;
	case OP_SD:
	case OP_EOM_SD:
		reference = rb_p->u.vc_rb.vc_reference;
		databuf = rb_p->u.vc_rb.vc_databuf;
		ep = rb_p->u.vc_rb.vc_ep;
		break;
	case OP_SCR:
		reference = *(ushort *)rb_p->u.cr_rb.cr_reference;
		databuf = rb_p->u.cr_rb.cr_message;
		ep = rb_p->u.cr_rb.cr_ep;
		break;
	case OP_ACRU:
		reference = *(ushort *)rb_p->u.cr_rb.cr_reference;
		databuf = rb_p->u.cr_rb.cr_databuf;
		ep = rb_p->u.cr_rb.cr_ep;
		break;
	default:
		cmn_err(CE_WARN, "Ethernet Driver: unexpected op_type to discon_ind - %d\n",
			((crbh *)rb_p)->c_opcode);
		relrb((struct req_blk *)rb_p);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_continue => NULL\n"));
		return;
	}
	if(ep->str_state & C_IDLE) {
		if(databuf)
			freemsg(databuf);
		relrb((struct req_blk *)rb_p);
		return;
	}
	if(ep->c_reference != reference) {
		if(ep->nbr_pend <= 1)
			nextstate = ti_statetbl[TE_DISCON_IND2][ep->tli_state];
		else
			nextstate = ti_statetbl[TE_DISCON_IND3][ep->tli_state];
	}
	else
		nextstate = ti_statetbl[TE_DISCON_IND1][ep->tli_state];
	DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: nextstate = %x\n",
								nextstate))
	if(nextstate == TS_INVALID){
		/* Can only get here if there was never a connection,
		 * (We posted an await_close, then the send_conn_req failed
		 *  or the listening stream was used to accept, and we closed
		 *  the outstanding (2nd) awaiting CDB),
		 * or, if this not the first error on this connection (we
		 *  have closed the connection by now).
		 * So, just release the rb and ignore it.
		 */
		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: nextstate invalid (ignored)\n"));
		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: current state = %x\n",
				ep->tli_state));
		DEBUGP(DEB_FULL,(CE_CONT, "c_reference = %x l_reference = %x\n",
				ep->c_reference, ep->l_reference));
		DEBUGP(DEB_FULL,(CE_CONT, "reference = %x nbr_pend = %x\n",
				reference, ep->nbr_pend));
		if(databuf)
			freemsg(databuf);
		relrb((struct req_blk *)rb_p);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_continue => NULL\n"));
		return;
	}
	/*
	 * Really close the connection to be sure it is closed
	 * (This indication may be from a dup. request or timeout)
	 * Note: this causes UNKNOWN REFERENCE failures on closes sometimes,
	 * but close_req_complete just ignores them.
	 */
	c_rb = (struct vcrb *)getrb(ep);
	if(c_rb == NULL) {
		if(databuf)
			freemsg(databuf);
		relrb((struct req_blk *)rb_p);
		return;
	}
	c_rb->vc_databuf = NULL;
	iNA961_close(ep, reference, c_rb,
			RSN_NORMAL, (unchar *)NULL, 0);
	mptr = getmptr(sizeof(struct T_discon_ind), BPRI_HI, ep);
	if(mptr == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_ind_continue: alloc failed\n"));
		if(databuf)
			freemsg(databuf);
		relrb((struct req_blk *)rb_p);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_continue => NULL\n"));
		return;
	}
	switch(((crbh *)rb_p)->c_opcode) {
	case OP_ACLOSE:
		size = *(ushort *)((struct vcrb *)rb_p)->vc_count;
		break;
	case OP_SD:
	case OP_EOM_SD:
	case OP_ACRU:
	case OP_SCR:
		size = 0;
		break;
	default: /* Can't get here */
		break;
	}
	if(size != 0) {
		char	*temp;

		mptr->b_cont = getmptr((int)size, BPRI_MED, ep);
		if(mptr->b_cont == NULL) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_ind_continue: alloc2 failed\n"));
			freemsg(mptr);
			relrb((struct req_blk *)rb_p);
			if(databuf)
				freemsg(databuf);
			DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_continue => NULL\n"));
			return;
		}
		temp = *(char **)((struct vcrb *)rb_p)->vc_bufp;
		memcpy((char *)mptr->b_cont->b_wptr,
		       (char *)phystokv(temp),
		       (int)size);
		mptr->b_cont->b_wptr += size;
	}
	mptr->b_datap->db_type  = M_PROTO;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_discon_ind);
	ind = (struct T_discon_ind *)mptr->b_rptr;
	ind->DISCON_reason = rb_p->u.vc_rb.vc_iso_rc;
	ind->PRIM_type = T_DISCON_IND;
	DEBUGP(DEB_FULL,(CE_CONT, "reference = %x c_reference = %x l_reference = %x\n",
				reference, ep->c_reference, ep->l_reference));
	if((ep->c_reference != reference) &&
	   (ep->l_reference != reference)) {
		/*
		 * This is a listening endpoint and a pending connection,
		 * return the sequence number to the user
		 */
		register pend_list *prev;

		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: listening endpoint\n"));
		for(pending = ep->pend_connects; 
		    pending != NULL;
		    pending = (pend_list *)pending->next) {
			if(reference == pending->reference)
				break;
			prev = pending;
		}
		if(pending == NULL) {
			cmn_err(CE_WARN, "Ethernet Driver: discon_ind of unexpected reference\n");
			cmn_err(CE_CONT, "c_ref = %x, l_ref = %x, ref = %x, databuf = %x\n",
					ep->c_reference, ep->l_reference,
					reference, databuf);
			cmn_err(CE_CONT, "nbr_pend = %x\n", ep->nbr_pend);
			for(pending = ep->pend_connects; 
			    pending != NULL;
			    pending = (pend_list *)pending->next)
				cmn_err(CE_CONT, "pend_ref = %x\n",
					pending->reference);
			if(databuf)
				freemsg(databuf);
			relrb((struct req_blk *)rb_p);
			ind->SEQ_number = 0;
			putnext(ep->rd_q, mptr);
			return;
		}
		/*
		 * Clear out the indication of the connection.
		 */
		ep->nbr_pend--;
		if(ep->pend_connects == pending)
			ep->pend_connects = (pend_list *)pending->next;
		else
			prev->next = pending->next;
		ind->SEQ_number = pending->seqno;
		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: seqno = %x\n",
							pending->seqno));
		relpend(pending);
		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: ep = %x ep->rd_q = %x\n",
							ep, ep->rd_q));
		putnext(ep->rd_q, mptr);
	}
	/*
	 * If this is from a pferr, or we have pferr'ed before, just tell the
	 * user he's broken (perhaps again)
	 */
	else if(ep->str_state & C_ERROR) {
		ind->SEQ_number = 0;
		putnext(ep->rd_q, mptr);
	}
	/*
	 * If this is the user's connected reference, and he is already
	 * listening on another reference, then just tell him he is
	 * disconnected.
	 */
	else if((reference == ep->c_reference) && (ep->l_reference != 0)) {
		ind->SEQ_number = 0;
		/* Mark the service as unknown type again */
		/* ep->options &= ~(OPT_COTS|OPT_CLTS); */ /* I007 */
		if(ep->pend_connects)
			ep->tli_state = TS_WRES_CIND;
		else
			ep->tli_state = TS_IDLE;
		ep->c_reference = 0;
		putnext(ep->rd_q, mptr);
	}
	/*
	 * Otherwise, we are closing (for the first time) the user's only
	 * connection.
	 * Make a new CDB, send him notification, and set him to TS_IDLE
	 */
	else {
		register struct orb *open_rb;

		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_continue: only connection\n"));
		ind->SEQ_number = 0;
		/* Mark the service as unknown type again */
		/* ep->options &= ~(OPT_COTS|OPT_CLTS); */ /* I007 */
		ep->tli_state = TS_IDLE;
		open_rb = (struct orb *)getrb(ep);
		if(open_rb == NULL) {
			freemsg(mptr);
			relrb((struct req_blk *)rb_p);
			if(databuf)
				freemsg(databuf);
			return;
		}
		ep->open_cont = enet_discon_ind_complete;
		open_rb->or_message = mptr;
		open_rb->or_ep = ep;
		if(iNA961_open(ep, open_rb)) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_ind_continue: open failed\n"));
			freemsg(mptr);
			relrb((struct req_blk *)rb_p);
			if(databuf)
				freemsg(databuf);
			enet_pferr(ep);
		}
	}
	if(databuf)
		freemsg(databuf);
	relrb((struct req_blk *)rb_p);
	DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_continue => NULL\n"));
}

/*
 * enet_discon_ind_complete
 *
 * Handle the results of the iNA961_open call for a new CDB after the board
 * indicates failure on the old one (which we then closed).
 * This is only called for the non-listening case, since if a listening endpoint
 * got a connection closed, it would just drop the pending connect.
 */
void
enet_discon_ind_complete(o_rb)
struct orb *o_rb;
{
	endpoint *ep;
	mblk_t	*mptr;
	mblk_t *databuf;				/* I015 */
	struct crrb *acr_rb;				/* I015 */

	DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_complete()\n"));
	DEBUGC('c');
	mptr = o_rb->or_message;
	ep = o_rb->or_ep;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(mptr);
		relrb((struct req_blk *)o_rb);
		return;
	}
	if(o_rb->or_crbh.c_resp != OK_RESP) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_ind_complete: bad resp %x\n",
				o_rb->or_crbh.c_resp));
		freemsg(mptr);
		relrb((struct req_blk *)o_rb);
		enet_pferr(ep);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_complete => NULL\n"));
		return;
	}
	ep->l_reference = o_rb->or_reference;
	relrb((struct req_blk *)o_rb);
	/* I015 Start */
	/*
	 * If this is a listening endpoint, post a buffer for incoming
	 * connect requests
	 */
	if(ep->max_pend != 0) {
		DEBUGP(DEB_FULL,(CE_CONT, "enet_discon_ind_complete:listening endpoint\n"));
		databuf = getmptr(CONN_BUF_LEN, BPRI_MED, ep);
		if(databuf == NULL) {
			freemsg(mptr);
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_ind_complete: alloc failed\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_complete => NULL\n"));
			return;
		}
		acr_rb = (struct crrb *)getrb(ep);
		if(acr_rb == NULL) {
			freemsg(mptr);
			freemsg(databuf);
			return;
		}
		acr_rb->cr_databuf = databuf;
		if(iNA961_await_conn_req(ep, acr_rb, 
					 databuf->b_datap->db_base)) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_discon_ind_complete: ACR failed\n"));
			freemsg(mptr);
			freemsg(databuf);
			relrb((struct req_blk *)acr_rb);
			DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_complete => NULL\n"));
			return;
		}
	}
	/* I015 End */
	/*putnext(ep->rd_q, o_rb->or_message);*/	/* I015 */
	putnext(ep->rd_q, mptr);			/* I015 */
	DEBUGP(DEB_CALL,(CE_CONT, "enet_discon_ind_complete => NULL\n"));
}

void
enet_conn_ind(cr_rb)
struct crrb *cr_rb;
{
	register endpoint *ep;
	ushort reference;
	mblk_t *mptr;
	struct T_conn_ind *conn;
	register pend_list *pending;
	int size;
	struct vcrb *c_rb;
	int addr_length;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind()\n"));
	DEBUGC('(');
	/*DEBUGC((char)cr_rb);*/
	DEBUGC((char)((ulong)cr_rb>>8));
	ep = cr_rb->cr_ep;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(cr_rb->cr_databuf);
		relrb((struct req_blk *)cr_rb);
		return;
	}
	if(cr_rb->cr_crbh.c_resp != OK_DECIDE_REQ_RESP) {
		if(cr_rb->cr_crbh.c_resp == OK_CLOSED_RESP)
		{
			DEBUGP(DEB_FULL,(CE_CONT, "enet_conn_ind: bad resp = %x\n",
					cr_rb->cr_crbh.c_resp))
		}
		else
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_ind: bad resp = %x\n",
					cr_rb->cr_crbh.c_resp));
		}
		freemsg(cr_rb->cr_databuf);
		relrb((struct req_blk *)cr_rb);
		return;
	}
	DEBUGC('[');
	if((ti_statetbl[TE_CONN_IND][ep->tli_state] == TS_INVALID) &&
	   (ep->tli_state != TS_WACK_CRES)) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_ind: nextstate invalid\n"));
		freemsg(cr_rb->cr_databuf);
		relrb((struct req_blk *)cr_rb);
		enet_pferr(ep);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind => NULL\n"));
		return;
	}
	reference = *(ushort *)cr_rb->cr_reference;
	if(ep->nbr_pend == ep->max_pend) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_ind: pend_list full\n"));
		/* Couldn't handle another await connection right now,
		 * flush it, and go back to listening */
		freemsg(cr_rb->cr_databuf);
		c_rb = (struct vcrb *)cr_rb;
		c_rb->vc_databuf = NULL;
		iNA961_close(ep, reference, c_rb,
				RSN_NORMAL, (unchar *)NULL, 0);
		enet_do_bind_req(ep);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind => NULL\n"));
		return;
	}
	addr_length=ADDR_LENGTH(REM_ADDR(phystokv(*(char **)cr_rb->cr_tabufp)));
	mptr=getmptr((int)(sizeof(struct T_conn_ind) + addr_length),
			BPRI_HI, ep);
	if(mptr == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_ind: alloc failed\n"));
		freemsg(cr_rb->cr_databuf);
		relrb((struct req_blk *)cr_rb);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind => NULL\n"));
		return;
	}
	size = cr_rb->cr_u_len;
	mptr->b_cont = cr_rb->cr_databuf;
	mptr->b_cont->b_wptr = mptr->b_cont->b_rptr + size;
	mptr->b_datap->db_type  = M_PROTO;
	conn = (struct T_conn_ind *)mptr->b_rptr;
	conn->PRIM_type = T_CONN_IND;
	conn->SRC_length = addr_length;
	conn->SRC_offset = sizeof(struct T_conn_ind);
	conn->OPT_length = 0;
	conn->OPT_offset = 0;
	conn->SEQ_number = enet_seqno;
	/*
	 * Forget the local stuff, and copy the rem_addr out to the user
	 */
	memcpy((char *)mptr->b_rptr+sizeof(struct T_conn_ind),
	       REM_ADDR((char *)phystokv(*(char **)cr_rb->cr_tabufp)),
	       addr_length);
	mptr->b_wptr+=sizeof(struct T_conn_ind) + addr_length;
	pending = getpend(ep);
	if(pending == NULL) {
		relrb((struct req_blk *)cr_rb);
		freemsg(mptr);
		c_rb = (struct vcrb *)getrb(ep);
		if(c_rb == NULL)
			return;
		c_rb->vc_databuf = NULL;
		iNA961_close(ep, reference, c_rb,
				RSN_NORMAL, (unchar *)NULL, 0);
		enet_do_bind_req(ep);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind => NULL\n"));
		return;
	}
	pending->seqno = enet_seqno++;
	pending->next = ep->pend_connects;
	pending->reference = reference;
	pending->o_rb = (struct orb *)getrb(ep);
	if(pending->o_rb == NULL) {
		freemsg(mptr);
		relrb((struct req_blk *)cr_rb);
		relpend(pending);
		c_rb = (struct vcrb *)getrb(ep);
		if(c_rb == NULL)
			return;
		c_rb->vc_databuf = NULL;
		iNA961_close(ep, reference, c_rb,
				RSN_NORMAL, (unchar *)NULL, 0);
		enet_do_bind_req(ep);
		return;
	}
	pending->o_rb->or_message = mptr;
	pending->o_rb->or_ep = ep;
	ep->pend_connects = pending;
	ep->nbr_pend++;
	relrb((struct req_blk *)cr_rb);
	/* Try to open a new CDB to continue listening, the interrupt
	 * will ack mptr to the user if successful */
	ep->open_cont = enet_conn_ind_complete;
	if(iNA961_open(ep, pending->o_rb)) {
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind: open failed\n"));
		/* Opening a new listener failed, punt the incoming connection,
		 * and (hopefully) use it's resources to continue listening */
		ep->nbr_pend--;
		ep->pend_connects = pending->next;
		freemsg(mptr);
		relrb((struct req_blk *)pending->o_rb);
		relpend(pending);
		c_rb = (struct vcrb *)getrb(ep);
		if(c_rb == NULL)
			return;
		c_rb->vc_databuf = NULL;
		iNA961_close(ep, reference, c_rb,
			RSN_NORMAL, (unchar *)NULL, 0);
		enet_do_bind_req(ep);
		return;
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind => NULL\n"));
}

void
enet_conn_ind_complete(o_rb)
struct orb *o_rb;
{
	endpoint *ep;
	mblk_t *databuf, *mptr;
	ushort reference;
	pend_list *pending, *prev;
	struct crrb *acr_rb;
	struct vcrb *c_rb;
	int nextstate;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind_complete()\n"));
	mptr = o_rb->or_message;
	ep = o_rb->or_ep;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		freemsg(mptr);
		relrb((struct req_blk *)o_rb);
		return;
	}
	prev = NULL;
	for(pending = ep->pend_connects;
	    pending != NULL;
	    pending = pending->next) {
		if(pending->o_rb == o_rb)
			break;
		prev = pending;
	}
	pending->o_rb = NULL;
	reference = pending->reference;
	if(o_rb->or_crbh.c_resp != OK_RESP) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_ind_complete: bad resp\n",
				o_rb->or_crbh.c_resp));
		/* Opening a new listener failed, punt the incoming connection,
		 * and (hopefully) use it's resources to continue listening */
		relrb((struct req_blk *)o_rb);
		c_rb = (struct vcrb *)getrb(ep);
		if(c_rb == NULL) {
			freemsg(mptr);
			return;
		}
		c_rb->vc_databuf = NULL;
		iNA961_close(ep, reference, c_rb,
				RSN_NORMAL, (unchar *)NULL, 0);
		ep->nbr_pend--;
		if(ep->pend_connects == pending)
			ep->pend_connects = pending->next;
		else
			prev->next = pending->next;
		freemsg(mptr);
		relpend(pending);
	 	enet_do_bind_req(ep);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind_complete => NULL\n"));
		return;
	}
	ep->l_reference = o_rb->or_reference;
	relrb((struct req_blk *)o_rb);
	databuf = getmptr(CONN_BUF_LEN, BPRI_MED, ep);
	if(databuf == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_ind_complete: alloc failed\n"));
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind_complete => NULL\n"));
		freemsg(mptr);
		return;
	}
	acr_rb = (struct crrb *)getrb(ep);
	if(acr_rb == NULL) {
		freemsg(mptr);
		freemsg(databuf);
		return;
	}
	acr_rb->cr_databuf = databuf;
	if(iNA961_await_conn_req(ep, acr_rb, databuf->b_datap->db_base)) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_ind_complete: ACR failed\n"));
		freemsg(mptr);
		freemsg(databuf);
		relrb((struct req_blk *)acr_rb);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind_complete => NULL\n"));
		return;
	}
	/*
	 * TLI expects us to detect connection loss anytime after sending
	 * the conn_ind.  So, post an await_close request.
	 */
	c_rb = (struct vcrb *)getrb(ep);
	if(c_rb == NULL) {
		freemsg(mptr);
		return;
	}
	databuf = getmptr(CLOSE_BUF_LEN, BPRI_MED, ep);
	if(databuf == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_conn_ind_complete: alloc2 failed\n"));
		DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind_complete => NULL\n"));
		freemsg(mptr);
		relrb((struct req_blk *)c_rb);
		return;
	}
	c_rb->vc_ep = ep;
	c_rb->vc_databuf = databuf;
	((struct req_blk *)c_rb)->databuf = databuf; /* I004 */
	if(iNA961_await_close(ep, reference, c_rb, databuf->b_datap->db_base)) {
		freemsg(mptr);
		freemsg(databuf);
		relrb((struct req_blk *)c_rb);
		return;
	}
	nextstate = ti_statetbl[TE_CONN_IND][ep->tli_state];
	ep->tli_state = (nextstate==TS_INVALID) ? TS_WACK_CRES : nextstate;
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_FULL,(CE_CONT, "enet_conn_ind_complete: mptr = %x\n", o_rb->or_message));
	DEBUGP(DEB_CALL,(CE_CONT, "enet_conn_ind_complete => NULL\n"));
}

/*
 * enet_data_ind
 *
 * Generate a T_DATA_IND message to the local user.
 *
 * This function automatically checks and updates the state of the
 * virtual endpoint.
 */
void
enet_data_ind(vc_rb)
struct vcrb *vc_rb;
{
	register endpoint *ep;
	int	post;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_data_ind()\n"));
	DEBUGC('"');
	ep = vc_rb->vc_ep;

	if((ep->str_state & (C_ERROR|C_IDLE)) ||
	   (vc_rb->vc_crbh.c_resp == UNKNOWN_REFERENCE)) {
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		return;
	}
	/* If the receive failed, just ignore it.  If the failure is connection
	 * loss, await_close will catch it */
	if((vc_rb->vc_crbh.c_resp != OK_RESP) &&
	   (vc_rb->vc_crbh.c_resp != OK_EOM_RESP)) {
		if(vc_rb->vc_crbh.c_resp == REM_ABORT) {
			DEBUGP(DEB_FULL,(CE_CONT, "enet_data_ind: bad resp\n"));
			DEBUGP(DEB_FULL,(CE_CONT, "enet_data_ind: ep = %x, ep->tli_state = %x\n",
				ep, ep->tli_state));
			DEBUGP(DEB_FULL,(CE_CONT, "enet_data_ind: str_state = %x, c_resp = %x\n",
				ep->str_state, vc_rb->vc_crbh.c_resp));
		}
		else {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_data_ind: bad resp\n"));
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_data_ind: ep = %x, ep->tli_state = %x\n",
				ep, ep->tli_state));
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_data_ind: str_state = %x, c_resp = %x\n",
				ep->str_state, vc_rb->vc_crbh.c_resp));
		}
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_data_ind => NULL\n"));
		return;
	}
	if(ti_statetbl[TE_DATA_IND][ep->tli_state] == TS_INVALID){
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_data_ind: nextstate invalid\n"));
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		DEBUGP(DEB_CALL,(CE_CONT, "enet_data_ind => NULL\n"));
		return;
	}


	if (post = canput(ep->rd_q->q_next))
	{
		enet_buf_post(vc_rb);
	}
	splx(enet_z);
	if (post)
		enet_data_wait(vc_rb);
	else
		enet_data_wait_post(vc_rb);
}


/* enqueue request for processing by service routine */
void
enet_data_wait(vc_rb)
struct	vcrb	*vc_rb;
{
	endpoint *ep;
	mblk_t	*mp;

	ep = vc_rb->vc_ep;
	if (!(mp = allocb(sizeof(struct vcp), BPRI_HI)))
	{
		if(!bufcall(sizeof(struct vcp), BPRI_HI,
				enet_data_wait, vc_rb))
		{
			cmn_err(CE_WARN, "Ethernet Driver: data_wait - buffcall failed\n"); /* I002 */
			freemsg(vc_rb->vc_databuf);
			relrb((struct req_blk *)vc_rb);
			return;
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_data_wait => NULL\n"));
		return;
	}
	((struct vcp *)(mp->b_rptr))->type = T_DATA_IND;	/* I008 */
	((struct vcp *)(mp->b_rptr))->vp = vc_rb;
	((struct vcp *)(mp->b_rptr))->post = 1;
	mp->b_wptr += sizeof(struct vcp);
	putq(ep->rd_q, mp);
}

/* enqueue request for processing by service routine */
void
enet_data_wait_post(vc_rb)
struct	vcrb	*vc_rb;
{
	endpoint *ep;
	mblk_t	*mp;

	ep = vc_rb->vc_ep;
	if (!(mp = allocb(sizeof(struct vcp), BPRI_HI)))
	{
		if(!bufcall(sizeof(struct vcp), BPRI_HI,
				enet_data_wait_post, vc_rb))
		{
			cmn_err(CE_WARN, "Ethernet Driver: data_wait_post - buffcall failed\n"); /* I002 */
			freemsg(vc_rb->vc_databuf);
			relrb((struct req_blk *)vc_rb);
			return;
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_data_wait_post => NULL\n"));
		return;
	}
	((struct vcp *)(mp->b_rptr))->type = T_DATA_IND;	/* I008 */
	((struct vcp *)(mp->b_rptr))->vp = vc_rb;
	((struct vcp *)(mp->b_rptr))->post = 0;
	mp->b_wptr += sizeof(struct vcp);
	putq(ep->rd_q, mp);
}

void
enet_buf_post(vc_rb)
struct vcrb *vc_rb;
{
	register endpoint *ep;
	register mblk_t *databuf;
	register struct vcrb *rd_rb;		/* I002 */

	DEBUGP(DEB_CALL,(CE_CONT, "enet_buf_post()\n"));
	DEBUGC('"');
	ep = vc_rb->vc_ep;
	if(ep->str_state & (C_ERROR|C_IDLE)) {
		cmn_err(CE_WARN, "Ethernet Driver: stream state error or idle.\n");
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		return;
	}

	/*
	 * Is there a buffer big enough?
	 */
	if (!(databuf = allocb(enet_data_buf_len, BPRI_MED))) {	/* I003 */
		DEBUGP(3,(CE_CONT, "enet_buf_post: alloc3 failed\n"));
		DEBUGC('<');
		enet_stat[ST_ALFA]++;
		if(!bufcall((uint)enet_data_buf_len, BPRI_MED,
				enet_buf_post, vc_rb)) 
		{
			cmn_err(CE_WARN, "Ethernet Driver: buf_post - buffcall failed\n"); /* I002 */
			freemsg(vc_rb->vc_databuf);
			relrb((struct req_blk *)vc_rb);
			return;
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_buf_post => NULL\n"));
		return;
	}

	DEBUGP(DEB_CALL,(CE_CONT, "enet_buf_post()\n"));
	DEBUGC('>');
	DEBUGC(*(phystokv(*(char **)vc_rb->vc_bufp)));
	if ((rd_rb = (struct vcrb *)getrb(ep)) == NULL) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_buf_post: getrb() failed\n"));
		freemsg(databuf);
	}
	else {
		rd_rb->vc_databuf = databuf;
		((struct req_blk *)rd_rb)->databuf = databuf; /* I004 */
		if(iNA961_receive_data(ep, ep, rd_rb,
				(unchar *)databuf->b_datap->db_base)) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enet_buf_post: iNA961 receive failed\n"));
			freemsg(databuf);
			relrb((struct req_blk *)rd_rb);
			freemsg(vc_rb->vc_databuf);
			relrb((struct req_blk *)vc_rb);
		}
	}							/* I002 */
}

/* Send incoming message up to user with a T_data_ind header. 
*  This procedure is invoked from the read service routine.
*/
void
enet_data_ind_complete(vc_rb)
struct	vcrb	*vc_rb;
{
	register endpoint *ep;
	register mblk_t *mptr;
	int size;
	int nextstate;
	struct T_data_ind *ind;

	DEBUGP(DEB_CALL,(CE_CONT, "enet_data_ind_complete()\n"));
	DEBUGC('"');
	ep = vc_rb->vc_ep;
	if(ep->str_state & (C_ERROR|C_IDLE)) 
	{
		freemsg(vc_rb->vc_databuf);
		relrb((struct req_blk *)vc_rb);
		return;
	}

	if (!(mptr = allocb(sizeof(struct T_data_ind), BPRI_MED))) 
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "enet_data_ind_complete: getmptr() failed, buf len %d\n", enet_data_buf_len));
		enet_stat[ST_ALFA]++;
		if(!(bufcall(sizeof(struct T_data_ind), BPRI_MED, 
					enet_data_ind_complete, vc_rb))) 
		{
			cmn_err(CE_WARN, "Ethernet Driver: data_ind_complete - bufcall failed\n");
			freemsg(vc_rb->vc_databuf);
			relrb((struct req_blk *)vc_rb);
			return;
		}
		DEBUGP(DEB_CALL,(CE_CONT, "enet_data_ind_complete => NULL\n"));
		return;
	}

	/*
	 * Set up buffer to pass to user.  If message is small, 
	 * copy to another buffer, else use existing buffer.
	 */
	size = *(ushort *)vc_rb->vc_count;
	if (size > 0) 
	{
	   if (  (size <= enet_maxcopysiz)		/* Begin I012 */
	       &&(mptr->b_cont = allocb(size, BPRI_MED)) 
	      )
	   {
		memcpy((char *)mptr->b_cont->b_rptr,
			(char *)vc_rb->vc_databuf->b_rptr, size);
		freemsg(vc_rb->vc_databuf);
	   }
	   else	
		mptr->b_cont = vc_rb->vc_databuf;
						/* End I012 */
	   mptr->b_cont->b_datap->db_type = M_DATA;
   	   mptr->b_cont->b_wptr = mptr->b_cont->b_rptr + size;
	   mptr->b_datap->db_type = M_PROTO;
	   ind = (struct T_data_ind *)mptr->b_rptr;
	   ind->PRIM_type = T_DATA_IND;
	   ind->MORE_flag = (vc_rb->vc_crbh.c_resp == OK_EOM_RESP)?0:1;
	   DEBUGC((char)((size>>8)&0xff));
	   DEBUGC((char)(size&0xff));
	   mptr->b_wptr += sizeof(struct T_data_ind);
	   putnext(ep->rd_q, mptr);
	}
	else
	{
	   freemsg(mptr);
	   freemsg(vc_rb->vc_databuf);
	}
	relrb((struct req_blk *)vc_rb);
	nextstate = ti_statetbl[TE_DATA_IND][ep->tli_state];
	ep->tli_state = nextstate;
	enet_stat[ST_RMSG]++;
	enet_stat[ST_BRCV] += size;
	DEBUGP(DEB_CALL,(CE_CONT, "enet_data_ind_complete => NULL\n"));
}

void
enet_open_complete(o_rb)
struct orb *o_rb;
{
	if(o_rb->or_ep->open_cont)
		o_rb->or_ep->open_cont(o_rb);
	else {
		cmn_err(CE_WARN, "Ethernet Driver: unexpected open_complete\n");
	}
}

/* JB - this routine is no longer used
 * void
 * enet_closed_open_complete(o_rb)
 * struct orb *o_rb;
 * {
 * 	freemsg(o_rb->or_message);
 * 	o_rb->or_ep->open_cont = NULL;
 * 	relrb((struct req_blk *)o_rb);
 * 	DEBUGP(DEB_FULL,(CE_CONT, "enet_closed_open_complete()\n"));
 * }
 */

void
enet_restart_output(q)		/* I011 */
queue_t *q;
{
	extern struct qinit enetwinit;

	if (  (q->q_flag & QUSE)
	    &&(q->q_qinfo == &enetwinit)
	    &&(q->q_ptr)
	   )
	{
		DEBUGP(DEB_FULL,(CE_CONT, "enet_restart_output on queue %x\n", q));
		enableok(q);
		qenable(q);
	}
}
