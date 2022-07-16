/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/ots/iTLIrd.c	1.3"

/*
** ABSTRACT:
**
**	Put and Service routines for read queue side of iMB2TLI driver.
**
** MODIFICATIONS:
*/

#include "sys/ots.h"

extern int ots_debug;
extern ulong ots_stat[];
extern struct otscfg otscfg;

/*
 * TPI state transition matrix, defined in the STREAMS system
 * module timod
 */
extern char ti_statetbl[TE_NOEVENTS][TS_NOSTATES];


/* FUNCTION:			iTLIrsrv()
 *
 * ABSTRACT:	Read Service Procedure
 *
 *	Used to throttle and serialize input on streams.  The only thing 
 *	that can be queued here are data indications and disconnects.
 *	Check if its OK to send to stream head, if not, return;
 */
iTLIrsrv(rd_q)
queue_t	*rd_q;
{
	mblk_t *mp;
	union qm *q;

	DEBUGC('A');

	while (  (canput(rd_q->q_next))
	       &&(mp = getq(rd_q))
	      )
	{
		q = (union qm *) mp->b_rptr;

		DEBUGP(DEB_FULL,(CE_CONT, "iTLIrsrv: mp=%x,q=%x,type=%x\n",
			mp,q,q->id.PRIM_type));

		if (q->id.PRIM_type == T_DATA_IND)
			iTLI_data_ind_complete(mp);
		else if (q->id.PRIM_type == T_DISCON_IND)
			iTLI_discon_ind_complete(mp);
		else if (q->id.PRIM_type == T_ORDREL_IND)
			iTLI_ordrel_ind_complete(mp);
		else if (  (q->id.PRIM_type == T_UNITDATA_IND)
			 ||(q->id.PRIM_type == T_EXDATA_IND)
			)
			putnext(rd_q, mp);
		else
			cmn_err(CE_PANIC,"iTLIrsrv: bad message queued\n");
	}
	iMB2_enable_remote((endpoint *)rd_q->q_ptr);
}
 

/* FUNCTION:		iTLI_bind_ack()
 *
 * ABSTRACT:
 *
 *	Issue a T_BIND_ACK message back to the transport user, to 
 *	acknowledge succesfull processing of the T_BIND_REQ TPI primitive.
 *
 * INPUTS:	"mptr" - transport user TPI message that was being processed.
 */
void
iTLI_bind_ack(ep, mptr)

endpoint *ep;
mblk_t *mptr;
{
	struct T_bind_ack *ack;
	ushort size;

	DEBUGC('B');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_bind_ack()\n"));

	size = sizeof(*ack) + ep->addr.length;

	if ((mptr = M_growmptr((int)size, BPRI_HI, ep, mptr)) == NULL)
		cmn_err(CE_WARN,"SV-ots: iTLI_bind_ack can't grow mptr\n");
	else
	{
		/*
		 * Construct the T_BIND_ACK message.
		 */
		mptr->b_datap->db_type = M_PROTO;
		ack = (struct T_bind_ack *)mptr->b_rptr;
		ack->PRIM_type = T_BIND_ACK;
		ack->ADDR_length = ep->addr.length;
		ack->ADDR_offset = sizeof(*ack);
		bcopy(ep->addr.data, (char *)ack + ack->ADDR_offset,
			(int)ack->ADDR_length);
		ack->CONIND_number = ep->max_pend;
		mptr->b_wptr = mptr->b_rptr + sizeof(*ack) + ep->addr.length;
		putnext(ep->rd_q, mptr);
	}
}


/* FUNCTION:			iTLI_bind_req_complete()
 *
 * ABSTRACT:	Change state and send ack after successful bind
 */
void
iTLI_bind_req_complete(ep, mptr, error)

endpoint *ep;
mblk_t *mptr;
int error;
{
	DEBUGC('C');

	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_bind_req_complete()\n"));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
		freemsg(mptr);

	else if (error)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_bind_req_complete: error=%x\n", error));
		DEBUGP(DEB_ERROR,(CE_CONT, " ep = %x\n", ep));
		ep->tli_state = ti_statetbl[TE_ERROR_ACK][ep->tli_state];
		iTLI_pterr(ep, mptr, TSYSERR, 0);
	}
	else	/* successful bind */
	{
		ep->tli_state = ti_statetbl[TE_BIND_ACK][ep->tli_state];
		iTLI_bind_ack(ep, mptr);
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_bind_req_complete => NULL\n"));
	}
}


/* FUNCTION:			iTLI_conn_con()
 *
 * ABSTRACT:
 *
 *	Generate a T_CONN_CON message to the local user.
 *
 *	This function automatically checks and updates the state of the
 *	virtual endpoint.
 *
 *	The reply to the user must be at high priority
 *
 * INPUTS:	streams buffer already large enough for TLI response header
 *		component header fields: address and option info
 *		connect response user data may be linked to streams buffer
 *
 */
void
iTLI_conn_con(ep, mptr, error, addr, addr_length, opt, opt_length)

endpoint *ep;
mblk_t *mptr;
int error;
char *addr;
int addr_length;
char *opt;
int opt_length;
{
	int nextstate;
	struct T_conn_con *conn;

	DEBUGC('D');

	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_conn_con()\n"));
	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
		freemsg(mptr);

	else if (error)		/* if error, send disconnect upstream */
		iTLI_discon_ind(ep, mptr, (ushort)error, 0, unlinkb(mptr));

	else if ((nextstate = ti_statetbl[TE_CONN_CON][ep->tli_state])
					== TS_INVALID)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_conn_con: bad next=%d, cur=%d\n",
					nextstate, ep->tli_state));
	        iTLI_pterr(ep, mptr, TOUTSTATE, 0);
	}
	else	/* OK connect confirmation */
	{
		ep->tli_state = nextstate;
		ots_stat[ST_TOTC]++;
		ots_stat[ST_CURC]++;
		mptr->b_datap->db_type = M_PROTO;
		conn = (struct T_conn_con *)mptr->b_rptr;
		conn->PRIM_type  = T_CONN_CON;
		conn->RES_length = addr_length;
		conn->RES_offset = sizeof(struct T_conn_con);
		conn->OPT_length = opt_length;
		conn->OPT_offset = conn->RES_offset + addr_length;
		bcopy(addr, (char *)conn + conn->RES_offset, addr_length);
		bcopy(opt, (char *)conn + conn->OPT_offset, opt_length);
		mptr->b_wptr = mptr->b_rptr + conn->OPT_offset + opt_length;
		putnext(ep->rd_q, mptr);
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_conn_con => NULL\n"));
	}
}


/* FUNCTION:			iTLI_conn_ind()
 *
 */
void
iTLI_conn_ind(ep, mptr, opt, opt_length, addr, addr_length, seqno)

endpoint *ep;
mblk_t *mptr;
char *opt;
int opt_length;
char *addr;
int addr_length;
int seqno;
{
	struct T_conn_ind *conn;
	int nextstate;

	DEBUGC('E');

	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_conn_ind:\n"));
	DEBUGP(DEB_FULL,(CE_CONT, "  ep=%x,mptr=%x,optlen=%x,addrlen=%x,\n",
			ep,mptr,opt_length, addr_length));
	DEBUGP(DEB_FULL,(CE_CONT, "  seqno=%x,addr=%x\n",
			seqno, *(long *)addr));

	if (  (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
	    ||(ep->nbr_pend >= ep->max_pend)
	   )
		freemsg(mptr);
	else if (  (ti_statetbl[TE_CONN_IND][ep->tli_state] == TS_INVALID)
		 &&(ep->tli_state != TS_WACK_CRES)
		)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_conn_ind: bad next=%d,cur=%d\n",
				ti_statetbl[TE_CONN_IND][ep->tli_state],
				ep->tli_state));
		freemsg(mptr);
		iTLI_pferr(ep, M_PROTOCOL_ERROR);
	}
	else
	{
		mptr->b_datap->db_type = M_PROTO;
		mptr->b_rptr = mptr->b_datap->db_base;
		conn = (struct T_conn_ind *)mptr->b_rptr;
		conn->PRIM_type = T_CONN_IND;
		conn->SRC_length = addr_length;
		conn->SRC_offset = sizeof(struct T_conn_ind);
		conn->OPT_length = opt_length;
		conn->OPT_offset = conn->SRC_offset + addr_length;
		conn->SEQ_number = seqno;
		bcopy(addr, (char *)conn + conn->SRC_offset, addr_length);
		bcopy(opt, (char *)conn + conn->OPT_offset, opt_length);
		mptr->b_wptr = mptr->b_rptr + conn->OPT_offset + opt_length;
		ep->nbr_pend++;
		nextstate = ti_statetbl[TE_CONN_IND][ep->tli_state];
		ep->tli_state = (nextstate == TS_INVALID) ?
					     TS_WACK_CRES : nextstate;
		putnext(ep->rd_q, mptr);
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_conn_ind: mptr = %x\n", mptr));
		DEBUGP(DEB_FULL,(CE_CONT, "  rptr=%x,wptr=%x,limit=%x\n",
			mptr->b_rptr,mptr->b_wptr,mptr->b_datap->db_lim));
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_conn_ind => NULL\n"));
	}
}


/* FUNCTION:			iTLI_conn_req_complete()
 *
 * ABSTRACT:	Notify TLI connection request succeeded
 */
void
iTLI_conn_req_complete(ep, mptr)

endpoint *ep;
mblk_t *mptr;
{
	DEBUGC('F');

	ep->tli_state = ti_statetbl[TE_OK_ACK1][ep->tli_state];
	iTLI_ok_ack(ep, mptr, 1);
}


/* FUNCTION:		iTLI_conn_res_complete()
 *
 * ABSTRACT:	Interrupt driven response to iTLI_conn_res().
 *
 *	Must reply to user with a PCPROTO message (high priority)
 */
void
iTLI_conn_res_complete(listen_ep, accept_ep, mptr, error)

endpoint *listen_ep, *accept_ep;
mblk_t *mptr;
int error;
{
	int event;

	DEBUGC('G');

	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_conn_res_complete()\n"));

	if (  (accept_ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
	    ||(error)
	   )
	{
		if (listen_ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
			freemsg(mptr);
		else
		{
			listen_ep->tli_state =
				ti_statetbl[TE_ERROR_ACK][listen_ep->tli_state];
			iTLI_pterr(listen_ep, mptr, TOUTSTATE|HIGH_PRI, 0);
		}
	}

	listen_ep->nbr_pend--;		/* decrement pending connection count */

	/*
	 * Transition to the TS_DATA_XFER state.
	 */
	if (listen_ep == accept_ep)
	{
		listen_ep->tli_state = 
		    ti_statetbl[TE_OK_ACK2][listen_ep->tli_state];
	}
	else
	{
		event = (listen_ep->nbr_pend == 0) ? TE_OK_ACK3 : TE_OK_ACK4;
		listen_ep->tli_state = ti_statetbl[event][listen_ep->tli_state];
		accept_ep->tli_state = 
			ti_statetbl[TE_PASS_CONN][accept_ep->tli_state];
	}
	ots_stat[ST_TOTC]++;
	ots_stat[ST_CURC]++;
	iTLI_ok_ack(listen_ep, mptr, 1);	/* HIGH PRIORITY */
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_conn_res_complete => NULL\n"));
}

/* FUNCTION:		iTLI_data_ind()
 *
 * ABSTRACT:
 *
 *	This function queues data indication.
 */
void
iTLI_data_ind(ep, mptr, more)

endpoint *ep;
mblk_t *mptr;
int more;
{
	mblk_t *data;
	int data_length;

	DEBUGC('H');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_data_ind: ep=%x,hdr=%x,data=%x,more=%x\n",
				ep,mptr,mptr->b_cont,more));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
	{
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_data_ind: ep = %x, ep->tli_state = %x\n",
				ep, ep->tli_state));
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_data_ind: str_state = %x\n",
				ep->str_state));
		freemsg(mptr);
	}
	else if (ti_statetbl[TE_DATA_IND][ep->tli_state] == TS_INVALID)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_data_ind: bad next=%d,cur=%d\n",
				ti_statetbl[TE_CONN_IND][ep->tli_state],
				ep->tli_state));
		freemsg(mptr);
	}
	else if (  ((data = mptr->b_cont) == NULL)
		 ||((data_length = msgdsize(data)) == 0)
		)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_data_ind: no data\n"));
		freemsg(mptr);
	}
	else if (  (otscfg.tsdu_size)
		 &&(  ((ep->tsdu_rbytes += data_length) > otscfg.tsdu_size)
		    ||(  (more)
		       &&(ep->tsdu_rbytes == otscfg.tsdu_size)
		      )
		   )
		)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_data_ind: TSDU exceeded\n"));
		freemsg(mptr);
		iTLI_pferr(ep, EPROTO);
	}
	else	/* queue message for service */
	{
		if (more == FALSE)
		{
			ots_stat[ST_RMSG]++;
			ep->tsdu_rbytes = 0;
		}
		ots_stat[ST_BRCV] += data_length;
		((union qm *)(mptr->b_rptr))->id.PRIM_type = T_DATA_IND;
		((union qm *)(mptr->b_rptr))->id.ep = ep;
		((union qm *)(mptr->b_rptr))->id.more =
				((otscfg.tsdu_size) ? more : FALSE);
		mptr->b_wptr = mptr->b_rptr + sizeof(union qm);
		putq(ep->rd_q, mptr);
	}
}


/* FUNCTION:			iTLI_data_ind_complete()
 *
 * ABSTRACT:
 *
 *	Send incoming message up to user with a T_data_ind header. 
 *	This procedure is invoked from the read service routine.
 */
void
iTLI_data_ind_complete(mp)

mblk_t *mp;
{
	union qm *q;
	endpoint *ep;
	int more;
	mblk_t *mptr;
	mblk_t *tmp;
	int data_length;
	struct T_data_ind *ind;

	DEBUGC('I');
	q = (union qm *) mp->b_rptr;
	ep = q->id.ep;
	more = q->id.more;
	ind = (struct T_data_ind *) mp->b_rptr;
	mptr = mp->b_cont;

	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_data_ind_complete:ep=%x,mp=%x,mptr=%x,data=%x\n",
			ep,mp,mptr,mptr->b_cont));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE)) 
		freemsg(mp);
	else
	{
		tmp = mptr;
		data_length = 0;
		do			/* compute message size */
		{
			data_length += tmp->b_wptr - tmp->b_rptr;
			tmp->b_datap->db_type = M_DATA;
		}
		while ((tmp = tmp->b_cont) != NULL);

		if (data_length > 0)
		{
			mp->b_datap->db_type = M_PROTO;
			ind->PRIM_type = T_DATA_IND;
	
			DEBUGP(DEB_FULL,(CE_CONT, "  len=%d, data=%x\n",data_length,
				*(long *)mptr->b_rptr));
	
			ind->MORE_flag = more;
			mp->b_wptr = mp->b_rptr + sizeof(struct T_data_ind);
			putnext(ep->rd_q, mp);
		}
		else
			freemsg(mp);
	
		ep->tli_state = ti_statetbl[TE_DATA_IND][ep->tli_state];
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_data_ind_complete => NULL\n"));
	}
}


/* FUNCTION:		iTLI_data_req_complete()
 *
 * ABSTRACT:	TLI processing after data send
 */
void
iTLI_data_req_complete(ep, mptr, error)

endpoint *ep;
mblk_t *mptr;
int error;
{
	DEBUGC('J');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_data_req_complete(): ep=%x, mptr=%x, err=%x\n",
			ep, mptr, error));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
		freemsg(mptr);

	else if (error == M_FLOW_CONTROL)
	{
		/* The queue will be re-enabled when the blocking condition
		 * goes away
		 */
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_data_req_complete: M_FLOW_CONTROL\n"));
		putbq(WR(ep->rd_q), mptr);
	}
	else if (error)
	{
		/* Don't print a message if we've been here before */
		if (ep->tli_state != TS_IDLE)
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_data_req_complete: bad resp %x, state %x\n",
				error, ep->tli_state));
		}
		iTLI_pterr(ep, mptr, TSYSERR|HIGH_PRI, error);
	}
	else	/* successful send completed */
	{
		freemsg(mptr);
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_data_req_complete => NULL\n"));
	}

	ep->nbr_datarq--;

	/* wakeup close proc if last send completed */

	if (  (ep->str_state & C_DRAIN)
	    &&(ep->nbr_datarq == 0)
	   )
		wakeup((caddr_t) ep);
}


/* FUNCTION:			iTLI_discon_ind()
 *
 * ABSTRACT:	Queue disconnect indication on read service queue
 */
void
iTLI_discon_ind(ep, mptr, error, seqno, ddata)

endpoint *ep;
mblk_t *mptr;
ushort error;
ushort seqno;
mblk_t *ddata;
{
	DEBUGC('K');
	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
	{
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_discon_ind: ep=%x, ep->tli_state = %x\n",
				ep, ep->tli_state));
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_discon_ind: str_state=%x, c_resp = %x\n",
				ep->str_state, error));
		if (mptr)
			freemsg(mptr);
	}
	else if (  (mptr == NULL)
		 &&((mptr = allocb(sizeof(union qm), BPRI_HI)) == NULL)
		)
	{
		ots_stat[ST_ALFA]++;
		cmn_err(CE_WARN, "SV-ots: iTLI_discon_ind allocb failed\n");
	}
	else if ((mptr = M_growmptr(sizeof(union qm),BPRI_HI,ep,mptr)) == NULL)
		cmn_err(CE_WARN, "SV-ots: iTLI_discon_ind grow_mptr failed\n");

	else	/* queue message for service */
	{
		((union qm *)(mptr->b_rptr))->id.PRIM_type = T_DISCON_IND;
		((union qm *)(mptr->b_rptr))->id.ep = ep;
		((union qm *)(mptr->b_rptr))->id.error = (ushort)error;
		((union qm *)(mptr->b_rptr))->id.seqno = (ushort)seqno;
		mptr->b_wptr = mptr->b_rptr + sizeof(union qm);
		if (ddata)
		{
			DEBUGP(DEB_FULL,(CE_CONT, "iTLI_discon_ind: ddata=%x\n",ddata));
			linkb(mptr, ddata);
			ddata = (mblk_t *)NULL;
		}
		putq(ep->rd_q, mptr);
	}

	if (ddata)
		freemsg(ddata);
}


/* FUNCTION:			iTLI_discon_ind_complete()
 *
 * ABSTRACT:	Build and send disconnect indication message upstream
 *
 *	iTLIwsrv() is called to prime the write service procedure.
 */
void
iTLI_discon_ind_complete(mp)

mblk_t *mp;
{
	union qm *q;
	endpoint *ep;
	struct T_discon_ind *ind;
	int nextstate;
	ushort seqno;
	ushort error;

	q = (union qm *) mp->b_rptr;
	ind = (struct T_discon_ind *) mp->b_rptr;
	ep = q->id.ep;
	seqno = q->id.seqno;
	error = q->id.error;

	DEBUGC('L');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_discon_ind_complete(): "));
	DEBUGP(DEB_CALL,(CE_CONT, "ep=%x, reason=%x, seqno=%x\n", ep,error,seqno));

	if (ep->nbr_pend > 1)
 		nextstate = ti_statetbl[TE_DISCON_IND3][ep->tli_state];
	else if (ep->nbr_pend == 1)
 		nextstate = ti_statetbl[TE_DISCON_IND2][ep->tli_state];
	else
		nextstate = ti_statetbl[TE_DISCON_IND1][ep->tli_state];

	DEBUGP(DEB_FULL,(CE_CONT, " state=%x, nextstate=%x\n", ep->tli_state,nextstate));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
		freemsg(mp);

	else if (nextstate == TS_INVALID)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_discon_ind_complete: bad next=%d,cur=%d\n",
				nextstate, ep->tli_state));
		DEBUGP(DEB_ERROR,(CE_CONT, "  seqno = %x nbr_pend = %x\n",
					seqno, ep->nbr_pend));
		freemsg(mp);
	}
	else
	{
		mp->b_datap->db_type  = M_PROTO;
		mp->b_rptr = mp->b_datap->db_base;
		mp->b_wptr = mp->b_rptr + sizeof(struct T_discon_ind);
		ind->DISCON_reason = error;
		ind->PRIM_type = T_DISCON_IND;
		if (  (ind->SEQ_number = seqno)
		    &&(ep->tli_state == TS_WRES_CIND)
		   )
			ep->nbr_pend--;
		ots_stat[ST_CURC]--;
		ep->tli_state = nextstate;
		putnext(ep->rd_q, mp);
		/*
		 * drain the send queue
		 */
		 ep->nbr_datarq = 0;
		 enableok(WR(ep->rd_q));
		 iTLIwsrv(WR(ep->rd_q));
	}
}


/* FUNCTION:		iTLI_discon_req_complete()
 *
 * ABSTRACT:	Send disconnect upstream to application after updating state
 *
 *	We enable the write queue in case there is an unbind or another
 *	connect request	waiting there.
 * 
 * CALLED FROM:	iTLIrsrv()
 */
void
iTLI_discon_req_complete(ep, mptr, error)

endpoint *ep;
mblk_t *mptr;
int error;
{
	DEBUGC('M');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_discon_req_complete()\n"));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
		freemsg(mptr);

	else if (error)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_discon_req_complete: bad resp %x\n",
				error));
		ep->tli_state = ti_statetbl[TE_ERROR_ACK][ep->tli_state];
		iTLI_pterr(ep, mptr, TSYSERR|HIGH_PRI, 0);
	}
	else
	{
		switch(ep->tli_state)
		{
		case TS_WACK_DREQ7:
			if (--ep->nbr_pend != 0)
				ep->tli_state = ti_statetbl[TE_OK_ACK4][ep->tli_state];
			else
				ep->tli_state = ti_statetbl[TE_OK_ACK2][ep->tli_state];
			break;
		case TS_WACK_DREQ10:
		case TS_WACK_DREQ11:
		case TS_WACK_DREQ9:
		case TS_WACK_DREQ6:
		default:
			ep->tli_state = ti_statetbl[TE_OK_ACK1][ep->tli_state];
			break;
		}
		ots_stat[ST_CURC]--;
		iTLI_ok_ack(ep, mptr, 1);	/* HIGH PRIORITY */
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_discon_req_complete => NULL\n"));
	}
}


/* FUNCTION:		iTLI_exdata_ind()
 *
 * ABSTRACT:
 *	Generate a T_EXDATA_IND message to the local user.
 *
 * NOTES:
 *  1)	These messages are queued ahead of normal data messages.
 *  2)	assumes mptr is message already containing header linked to data
 */
void
iTLI_exdata_ind(ep, mptr, more)

endpoint *ep;
mblk_t *mptr;
int more;
{
	mblk_t *tmp;			/* used to parse queue message list */
	struct T_exdata_ind *ind;
	int data_length;
	int nextstate;

	DEBUGC('N');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_exdata_ind: ep=%x,more=%x\n",ep,more));
	DEBUGP(DEB_FULL,(CE_CONT, "  header=%x,data=%x\n",mptr,mptr->b_cont));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
		freemsg(mptr);
	else if ((ep->options & OPT_EXP) == FALSE)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_exdata_ind: NOT supported on ep.\n"));
		freemsg(mptr);
		iTLI_pferr(ep, M_PROTOCOL_ERROR);
	}
	else if ((nextstate = ti_statetbl[TE_EXDATA_IND][ep->tli_state])
					 == TS_INVALID)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_exdata_ind: bad next=%d,cur=%d\n",
				nextstate, ep->tli_state));
		freemsg(mptr);
	}
	else
	{
		data_length = msgdsize(mptr->b_cont);	/* Note 2 */

		if (  (otscfg.etsdu_size)
		    &&(  ((ep->etsdu_rbytes += data_length) > (int)otscfg.etsdu_size)
		       	||(  (more)
			   &&(ep->etsdu_rbytes == otscfg.etsdu_size)
			  )
		      )
		   )
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_exdata_ind: ETSDU exceeded\n"));
			freemsg(mptr);
			iTLI_pferr(ep, EPROTO);
		}
		else
		{
			mptr->b_datap->db_type = M_PROTO;
			ind = (struct T_exdata_ind *)mptr->b_rptr;
			ind->PRIM_type = T_EXDATA_IND;
			ind->MORE_flag = (otscfg.etsdu_size) ? more : FALSE;
			if (more == FALSE)
				ep->etsdu_rbytes = 0;
			mptr->b_wptr = mptr->b_rptr + sizeof(*ind);
			ep->tli_state = nextstate;
			ots_stat[ST_REXP]++;
			ots_stat[ST_EPCK]++;
			ots_stat[ST_ERCV] += data_length;

			/* queue message ahead of any normal data messages */

			tmp = ep->rd_q->q_first;
			while (tmp != NULL)
			{
				if (  (tmp->b_datap->db_type == M_DATA)
				    ||(((union T_primitives *)tmp->b_rptr)->type == T_DATA_REQ)
				   )
					break;
				tmp = tmp->b_next;
			}
			insq(ep->rd_q, tmp, mptr);
		}
	}
}


/* FUNCTION		iTLI_exdata_req_complete()
 *
 * ABSTRACT:	Complete processing for expedited send
 *
 *	Any error except for that associated with flow control is fatal.
 */
void
iTLI_exdata_req_complete(ep, mptr, error)

endpoint *ep;
mblk_t *mptr;
int error;
{
	DEBUGC('O');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_exdata_req_complete: ep=%x,mptr=%x,error=%x\n",
			ep, mptr,error));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
		freemsg(mptr);

	else if (error == M_FLOW_CONTROL)
	{
		/* The queue will be re-enabled when the blocking condition
		 * goes away
		 */
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_exdata_req_complete: M_FLOW_CONTROL\n"));
		putbq(WR(ep->rd_q), mptr);
	}
	else if (error)
	{
		DEBUGP(DEB_ERROR,
		  (CE_CONT, "iTLI_expedited_data_req_complete: bad resp %x, state %x\n",
				error, ep->tli_state));
		freemsg(mptr);
		iTLI_pferr(ep, error);
	}
	else
		freemsg(mptr);

	ep->nbr_datarq--;

	/* wakeup close proc if last send completed */

	if (  (ep->str_state & C_DRAIN)
	    &&(ep->nbr_datarq == 0)
	   )
		wakeup((caddr_t) ep);
}


/* FUNCTION:		iTLI_info_ack()
 *
 * ABSTRACT:	Respond to T_INFO_REQ
 *
 */
void
iTLI_info_ack(ep, mptr, tsdu_size, etsdu_size, cdata_size,
	      ddata_size, addr_size, opt_size, tidu_size,
	      tli_state, serv_type)

endpoint *ep;
mblk_t *mptr;
long tsdu_size, etsdu_size, cdata_size, ddata_size, addr_size;
long opt_size, tidu_size, tli_state, serv_type;
{
	struct T_info_ack *ack;

	DEBUGC('?');

	if ((mptr = M_growmptr(sizeof(struct T_info_ack),
				BPRI_MED, ep, mptr)) != NULL)
	{
		/*
		 * Build a T_INFO_ACK message.
		 */
		mptr->b_datap->db_type  = M_PROTO;
		mptr->b_rptr  = mptr->b_datap->db_base;
		mptr->b_wptr = mptr->b_rptr + sizeof(struct T_info_ack);
		ack = (struct T_info_ack *)mptr->b_rptr;
		ack->PRIM_type     = T_INFO_ACK;
		ack->TSDU_size     = tsdu_size;
		ack->ETSDU_size    = etsdu_size;
		ack->CDATA_size    = cdata_size;
		ack->DDATA_size    = ddata_size;
		ack->ADDR_size     = addr_size;
		ack->OPT_size      = opt_size;
		ack->TIDU_size     = tidu_size;
		ack->CURRENT_state = tli_state;
		ack->SERV_type     = serv_type;
		putnext(ep->rd_q, mptr);
	}
}


/* FUNCTION:		iTLI_ok_ack()
 *
 * ABSTRACT:
 *
 *	Issue a T_OK_ACK message back to the transport user, to 
 *	acknowledge succesfull processing of a TPI primitive.
 *
 * INPUTS:
 *	"mptr" is the transport user TPI message that was being processed.
 *	"flag" is 1 if this is to be a PCPROTO message
 */
void
iTLI_ok_ack(ep, mptr, flag)

register endpoint *ep;
register mblk_t *mptr;
int flag;
{
	long tli_type;
	struct T_ok_ack *ack;

	DEBUGC('Q');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_ok_ack: ep = %x, mptr = %x\n", ep, mptr));
	tli_type = ((union T_primitives *)mptr->b_rptr)->type;
	/*
	 * See if the original message is large enough to be used to construct
	 * the T_OK_ACK message.
	 */
	if ((mptr->b_datap->db_lim - mptr->b_datap->db_base) < 
	      sizeof(struct T_ok_ack))
	{
		/*
		 * The original message is too small: Get a new message
		 * and free up the original.
		 */
		freemsg(mptr);
		mptr = M_getmptr(sizeof(struct T_ok_ack), BPRI_HI, ep);
		if (mptr == NULL)
			return;
	}
	else
		/*
		 * Only the first block of the original message is
		 * needed.
		 */
		freemsg(unlinkb(mptr));
	/*
	 * Construct the T_OK_ACK message.
	 */
	mptr->b_datap->db_type  = flag?M_PCPROTO:M_PROTO;
	mptr->b_rptr  = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_ok_ack);
	ack = (struct T_ok_ack *)mptr->b_rptr;
	ack->PRIM_type    = T_OK_ACK;
	ack->CORRECT_prim = tli_type;
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_ok_ack => NULL\n"));
}


/* FUNCTION:		iTLI_optmgmt_ack()
 *
 * ABSTRACT:	Respond to T_optmgmt request
 */
void
iTLI_optmgmt_ack(ep, mptr, flags, length, options)

endpoint *ep;
mblk_t *mptr;
int flags;
int length;
opts options;
{
	struct T_optmgmt_req *opt_req;

	DEBUGC('R');

	/*
	 * Make sure there is enough space to return options to
	 * the user.
	 */
	if (mptr = M_growmptr((int)(sizeof(struct T_optmgmt_ack) + length),
				BPRI_MED, ep, mptr))
	{
		opt_req = (struct T_optmgmt_req *)mptr->b_rptr;
		mptr->b_datap->db_type = M_PROTO;
		opt_req->PRIM_type  = T_OPTMGMT_ACK;
		opt_req->OPT_length = length;
		opt_req->OPT_offset = (length == 0) ? 
					0 : sizeof(struct T_optmgmt_ack);
		opt_req->MGMT_flags = flags;
		if (length)
			*(opts *)(mptr->b_rptr + sizeof(struct T_optmgmt_ack))
					= options;
		mptr->b_wptr = mptr->b_rptr + sizeof(struct T_optmgmt_ack)
					+ length;
		if (ep->tli_state == TS_WRES_CIND)
			ep->tli_state = TS_WRES_CIND;
		else
			ep->tli_state = ti_statetbl[TE_OPTMGMT_ACK][ep->tli_state];
		putnext(ep->rd_q, mptr);
	}
}


/* FUNCTION:			iTLI_ordrel_ind()
 *
 * ABSTRACT:
 *
 *	Queue orderly disconnect operation for service
 *	NOTE: reason is ignored; TLI doesn't recognize a reason code.
 */
void
iTLI_ordrel_ind(ep, mptr, reason)

endpoint *ep;
mblk_t *mptr;
int reason;
{
	DEBUGC('q');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_ordrel_ind: ep=%x, mptr=%x, reason=%x\n",
			ep, mptr, reason));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
	{
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_ordrel_ind: ep=%x, ep->tli_state = %x\n",
				ep, ep->tli_state));
		if (mptr)
			freemsg(mptr);
	}
	else if ((ep->options & OPT_ORD) == FALSE)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_ordrel_ind: NOT supported on ep.\n"));
		if (mptr)
			freemsg(mptr);
		iTLI_pferr(ep, M_PROTOCOL_ERROR);
	}
	else if (  (mptr == NULL)
		 &&((mptr = allocb(sizeof(union qm), BPRI_HI)) == NULL)
		)
	{
		ots_stat[ST_ALFA]++;
		cmn_err(CE_WARN, "SV-ots: iTLI_ordrel_ind allocb failed\n");
	}
	else if ((mptr = M_growmptr(sizeof(union qm),BPRI_HI,ep,mptr)) == NULL)
		cmn_err(CE_WARN, "SV-ots: iTLI_ordrel_ind grow_mptr failed\n");

	else	/* queue message for service */
	{
		((union qm *)(mptr->b_rptr))->id.PRIM_type = T_ORDREL_IND;
		((union qm *)(mptr->b_rptr))->id.ep = ep;
		mptr->b_wptr = mptr->b_rptr + sizeof(union qm);
		putq(ep->rd_q, mptr);
	}
}


/* FUNCTION:		iTLI_ordrel_ind_complete()
 *
 * ABSTRACT:
 *
 *	build message and send T_ORDREL_IND message upstream
 *	update tli stream state
 *
 *	NOTE: need to update ots_stats.
 */
void
iTLI_ordrel_ind_complete(mp)

mblk_t *mp;
{
	union qm *q;
	endpoint *ep;
	struct T_ordrel_ind *ind;

	DEBUGC('q');

	q = (union qm *) mp->b_rptr;
	ep = q->id.ep;

	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_ordrel_ind_complete: ep=%x, mp=%x\n", ep, mp));

	mp->b_datap->db_type = M_PROTO;
	ind = (struct T_ordrel_ind *) mp->b_rptr;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_ordrel_ind);
	ind->PRIM_type = T_ORDREL_IND;
	putnext(ep->rd_q, mp);
	ep->tli_state = ti_statetbl[TE_ORDREL_IND][ep->tli_state];
}


/* FUNCTION:		iTLI_pferr()
 *
 * ABSTRACT:	A fatal error has occurred on the endpoint
 */
void
iTLI_pferr(ep, error)

endpoint *ep;
int error;
{
	mblk_t *mptr;

	DEBUGC('S');

	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_pferr()\n"));
	DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_pferr: STOP\n"));
	DEBUGP(DEB_ERROR,(CE_CONT, "ep = %x\n", ep));

	if (ep == NULL)
	{
		cmn_err(CE_WARN, "iTLI_pferr: error on unknown endpoint\n");
		return;
	}
	/*
	 * Watch out for "rolling" and "recursive" errors: Only do this once.
	 */
	if ((ep->str_state & (C_ERROR | C_CLOSE | C_IDLE)) == FALSE)
	{
		ep->str_state |= C_ERROR;

		/*
		 * Take down a connection:
		 *
		 *	1. Notify all remote endpoints this endpoint is down.
		 *		Note that a "flushing disconnect" is used.
		 *	2. Notify local user this endpoint is down. Either:
		 *		a. Generate a T_DISCON_IND,
		 *		   if such a message would be legal and
		 *		   fatal error is NOT a TLI EPROTO.
		 *		b. Otherwise generate a M_ERROR of type EPROTO.
		 */
		iMB2_abort(ep);
		if (  (error == EPROTO)
		    ||(Tdiscon_ind(ep, error, 0) == FALSE)
		   )
		{
			/*
			 * Couldn't send user T_discon_ind; can only report
			 *  the error through an M_ERROR message.
			 */
			if ((mptr = allocb(1, BPRI_HI)) != NULL)
			{
				mptr->b_datap->db_type  = M_ERROR;
				mptr->b_wptr = mptr->b_rptr + sizeof(char);
				/*
				 * There is only one fatal TPI message.
				 */
				*mptr->b_rptr = EPROTO;
				putnext(ep->rd_q, mptr);
				DEBUGP(DEB_CALL,(CE_CONT, "iTLI_pferr => NULL\n"));
			}
			else
				ots_stat[ST_ALFA]++;
		}
	}
}


/* FUNCTION:		iTLI_pterr()
 *
 * ABSTRACT:
 *
 *	Issue an error message for a non-fatal TLI error condition.
 *
 *	"mptr" is the transport user TLI message that was being processed.
 */
void
iTLI_pterr(ep, mptr, tli_err, unx_err)

endpoint *ep;
register mblk_t *mptr;
int tli_err, unx_err;
{
	long tli_type;
	register struct T_error_ack *ack;

	DEBUGC('T');

	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_pterr()\n"));
	DEBUGP(DEB_CALL,(CE_CONT, "  ep = %x, tli_err = %x, unx_err = %d\n", 
			ep, tli_err, unx_err));

	if (  (ep == NULL)
	    ||(ep->str_state & (C_CLOSE | C_ERROR | C_IDLE))
	   )
	{
		freemsg(mptr);
		return;
	}

	tli_type = ((union T_primitives *)mptr->b_rptr)->type;
	/*
	 * See if the original message is large enough to be used to construct a
	 * T_ERROR_ACK message.
	 */
	if ((mptr->b_datap->db_lim - mptr->b_datap->db_base) < 
	     sizeof(struct T_error_ack))
	{	 /*
		 * The original message is too small: Get a new message
		 * and free up the original.
		 */
		freemsg(mptr);
		mptr = M_getmptr(sizeof(struct T_error_ack), BPRI_HI, ep);
		if (mptr == NULL)
			return;
	}
	/*
	 * Construct the T_ERROR_ACK message.
	 */
	mptr->b_datap->db_type  = (tli_err&HIGH_PRI)?M_PCPROTO:M_PROTO;
	DEBUGP(DEB_FULL,(CE_CONT, "iTLI_pterr: error message of type %x\n",
			mptr->b_datap->db_type));
	mptr->b_rptr  = mptr->b_datap->db_base;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_error_ack);
	ack = (struct T_error_ack *)mptr->b_rptr;
	ack->PRIM_type  = T_ERROR_ACK;
	ack->ERROR_prim = tli_type;
	ack->TLI_error  = tli_err&~HIGH_PRI;
	ack->UNIX_error = unx_err;
	if (mptr->b_cont)
	{
		freemsg(mptr->b_cont);
		mptr->b_cont = 0;
	}
	putnext(ep->rd_q, mptr);
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_pterr => NULL\n"));
}


/* FUNCTION:		iTLI_send_flush()
 *
 * Issue a M_FLUSH, to flush both the read and writer queues.
 */
void
iTLI_send_flush(ep)
register endpoint *ep;
{
	register mblk_t *mptr;

	DEBUGC('U');

	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_send_flush()\n"));
	if ((mptr = allocb(1, BPRI_HI)) == NULL)
		ots_stat[ST_ALFA]++;
	else
	{
		mptr->b_datap->db_type = M_FLUSH;
		*mptr->b_rptr = FLUSHRW;
		mptr->b_wptr++;
		putnext(ep->rd_q, mptr);
		enableok(WR(ep->rd_q));
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_send_flush => NULL\n"));
	}
}


/* FUNCTION:			iTLI_unbind_req_complete()
 *
 * ABSTRACT:	Notify TLI connection request succeeded
 */
void
iTLI_unbind_req_complete(ep, mptr)

endpoint *ep;
mblk_t *mptr;
{
	DEBUGC('V');
	/*
	 * Update the state of the virtual endpoint.
	 */
	ep->tli_state = ti_statetbl[TE_OK_ACK1][ep->tli_state];
	iTLI_ok_ack(ep, mptr, 0);
}


/* FUNCTION:			iTLI_unitdata_ind()
 *
 * ABSTRACT:
 *
 *	Generate and queue T_UNITDATA_IND message.  The read service procedure
 *	will send the message upstream to the user.  This will allow
 *	iTLIrsrv() to enable any blocked write queues if this datagram came
 *	from a local endpoint.
 *
 * INPUTS:
 *	receiving endpoint
 *	TLI header and data mblocks linked together
 *	datagram source address info
 *	datagram options info
 */
void
iTLI_unitdata_ind(ep, header, addr_length, addr, opt_length, opt)

endpoint *ep;
mblk_t *header;
int addr_length;
char *addr;
int opt_length;
char *opt;
{
	int nextstate;
	int data_length;
	struct T_unitdata_ind *ind;

	DEBUGC('W');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_unitdata_ind:ep=%x,header=%x,",ep,header));
	DEBUGP(DEB_CALL,(CE_CONT, "alen=%x,addr=%x,optl=%x,size=%x\n", addr_length,
			*(long*)addr,opt_length,msgdsize(header->b_cont)));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_unitdata_ind: bad stream state\n"));
		freemsg(header);
	}
	else if ((nextstate = ti_statetbl[TE_UNITDATA_IND][ep->tli_state]) ==
			TS_INVALID)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_unitdata_ind: bad next=%d,cur=%d\n",
				nextstate,ep->tli_state));
		freemsg(header);
	}

	else if ((data_length = msgdsize(header->b_cont)) > (int)otscfg.datagram_size)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_unitdata_ind: datagram too large\n"));
		freemsg(header);

	}
	else
	{
		header->b_datap->db_type = M_PROTO;
		ind = (struct T_unitdata_ind *)header->b_rptr;
		ind->PRIM_type = T_UNITDATA_IND;
		ind->SRC_length = addr_length;
		ind->SRC_offset = sizeof(struct T_unitdata_ind);
		bcopy(addr, (char *)ind + ind->SRC_offset, addr_length);
		ind->OPT_offset = ind->SRC_offset + ind->SRC_length;
		if (ind->OPT_length = opt_length)
			bcopy(opt, (char *)ind + ind->OPT_offset, opt_length);
		header->b_wptr = header->b_rptr + ind->OPT_offset + opt_length;
		ep->tli_state = nextstate;
		ots_stat[ST_RUNI]++;
		ots_stat[ST_URCV] += data_length;
		putq(ep->rd_q, header);
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_unitdata_ind => NULL\n"));
	}
}


/* FUNCTION:		iTLI_unitdata_req_complete()
 *
 * ABSTRACT:
 *
 *	Free message block. If error was generated, iTLI_uderror_ind() is
 *	called instead.
 */
void
iTLI_unitdata_req_complete(ep, mptr, error)

endpoint *ep;
mblk_t *mptr;
int error;
{
	struct T_unitdata_req *udata;

	DEBUGC('X');
	DEBUGP(DEB_CALL,
		(CE_CONT, "iTLI_unitdata_req_complete(): ep=%x, mptr=%x, err=%x\n",
			ep, mptr, error));

	if (ep->str_state & (C_ERROR|C_IDLE|C_CLOSE))
		freemsg(mptr);

	else if (error == M_FLOW_CONTROL)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "iTLI_unitdata_req_complete: M_FLOW_CONTROL\n"));
		putbq(WR(ep->rd_q), mptr);
	}
	else if (error)
	{
		udata = (struct T_unitdata_req *)mptr->b_rptr;
		iTLI_uderror_ind(ep, TSYSERR,
			(int)udata->DEST_length,
			(char *)udata+udata->DEST_offset,
			(int)udata->OPT_length,
			(char *)udata+udata->OPT_offset);
		freemsg(mptr);
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_unitdata_req_complete => TSYSERR\n"));
	}
	else
	{
		freemsg(mptr);
		DEBUGP(DEB_CALL,(CE_CONT, "iTLI_unitdata_req_complete => NULL\n"));
	}

	ep->nbr_datarq--;

	/* wakeup close proc if last send completed */

	if (  (ep->str_state & C_DRAIN)
	    &&(ep->nbr_datarq == 0)
	   )
		wakeup((caddr_t) ep);
}


/* NAME:		iTLI_uderror_ind()
 *
 * ABSTRACT:	Generate and return T_UDERROR_IND message to user
 */
void
iTLI_uderror_ind(ep, error, addr_length, addr, opt_length, opt)

endpoint *ep;
int error;
int addr_length;
char *addr;
int opt_length;
char *opt;
{
	mblk_t *mptr;
	struct T_uderror_ind *ind;
	int ind_size;

	DEBUGC('W');
	DEBUGP(DEB_CALL,(CE_CONT, "iTLI_uderror_ind()\n"));

	ind_size = sizeof(struct T_uderror_ind) + addr_length + opt_length;

	if ((mptr = allocb(ind_size, BPRI_MED)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iTLI_uderror_ind: allocb failed\n"));
	}
	else
	{
		mptr->b_wptr = mptr->b_rptr + ind_size;
		mptr->b_datap->db_type = M_PROTO;
		ind = (struct T_uderror_ind *)mptr->b_rptr;
		ind->PRIM_type = T_UDERROR_IND;
		ind->ERROR_type = error;
		ind->DEST_length = addr_length;
		ind->OPT_length = opt_length;
		ind->DEST_offset = sizeof(struct T_uderror_ind);
		ind->OPT_offset = ind->DEST_offset + addr_length;
		bcopy(addr, (char *)ind+ind->DEST_offset, addr_length);
		bcopy(opt, (char *)ind+ind->OPT_offset, (int)opt_length);
		ep->tli_state = ti_statetbl[TE_UDERROR_IND][ep->tli_state];
		putnext(ep->rd_q, mptr);
	}
}

/* FUNCTION:		Tdiscon_ind()
 *
 * ABSTRACT:	Generate a T_DISCON_IND message to the local user
 *
 *	This function returns TRUE if the message was written, and FALSE
 *	otherwise.
 *
 *	The message will only be generated if the state of the virtual
 *	curcuit would permit it.
 *
 *	The state of the virtual endpoint is updated to reflect the message
 *	if it is generted.
 */
int
Tdiscon_ind(ep, reason, seqno)

endpoint *ep;
int reason, seqno;
{
	int nextstate, i;
	mblk_t *mptr;
	struct T_discon_ind *dis;

	DEBUGC('Y');
	DEBUGP(DEB_CALL,(CE_CONT, "Tdiscon_ind()\n"));
	i = (ep->nbr_pend == 0) ? TE_DISCON_IND1
			       : ((ep->nbr_pend == 1) ? TE_DISCON_IND2
						     : TE_DISCON_IND3);
	if ((nextstate = ti_statetbl[i][ep->tli_state]) == TS_INVALID)
	{
		DEBUGP(3, (CE_CONT, "Tdiscon_ind(): TS_INVALID\n"));
		return(FALSE);
	}			
	/*
	 * A T_DISCON_IND to the local user is valid.
	 */
	mptr = M_getmptr(sizeof(struct T_discon_ind), BPRI_HI, ep);
	if (mptr == NULL)
	{
		DEBUGP(3, (CE_CONT, "Tdiscon_ind(): getmptr failed\n"));	
		return(FALSE);
	}
	mptr->b_datap->db_type = M_PCPROTO;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct T_discon_ind);
	dis = (struct T_discon_ind *)mptr->b_rptr;
	dis->PRIM_type	   = T_DISCON_IND;
	dis->DISCON_reason = reason;
	dis->SEQ_number	   = seqno;
	/*
	 * If necessary, generate a M_FLUSH.
	 */
	if (ep->tli_state == TS_DATA_XFER)
		iTLI_send_flush(ep);
	putnext(ep->rd_q, mptr);
	ep->tli_state = nextstate;
	DEBUGP(DEB_CALL,(CE_CONT, "discon_ind => TRUE\n"));
	return(TRUE);
}
