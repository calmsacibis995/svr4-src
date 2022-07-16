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

#ident	"@(#)mbus:uts/i386/io/ots/otsintr.c	1.3"

/*
** ABSTRACT:	This module processes all incoming messages.
**
**	The two main entry points from TKI are:
**
**		ots_control_intr() - handles control socket messages
**		ots_data_intr() - handles data socket messages
**
** MODIFICATIONS:
*/

#include "sys/ots.h"
#include "sys/otserror.h"
#include "sys/otsprot.h"
#include <sys/immu.h>

extern int ots_debug;

extern connect ots_connects[];
extern ulong ots_stat[];
extern struct otscfg otscfg;

extern unchar ics_myslotid();

/* FUNCTION:		ots_control_intr()
 *
 * ABSTRACT:	Processes messages received on control sockets
 *
 * ALGORITHM:
 *
 *	IF (error encountered)
 *		process error
 *	ELSE IF (end of incoming buffer request)
 *		process completed solicited receive
 *	ELSE IF (source socket is local)
 *		free resources after completed solicited send reply
 *	ELSE IF (source socket is remote)
 *		IF (end of RSVP transaction)
 *			process input reply
 *		ELSE IF (unsolicited message received)
 *			process input unsolicited message
 *		ELSE IF (buffer request received)
 *			grant buffer input unsolicited message
 *		ENDIF
 *	ENDIF
 *
 * NOTE: 1) data saved in mb_bind are always streams messages
 *	 2) we don't support fragmentation on control sockets
 *
 * INPUTS:	Received message buffer
 *
 * OUTPUTS:	processed message
 *
 * RETURN CODE:	none
 *
 * CALLED FROM:	TKI's mps_msg_dispatch()
 */
void
ots_control_intr(mbuf_p)

mps_msgbuf_t *mbuf_p;		/* incoming message */
{
	mb2socid_t dest;	/* destination socket */
	mb2socid_t src;		/* source socket */
	connect *ce;		/* local control connection entry */

	DEBUGC('r');
	DEBUGP(DEB_CALL,(CE_CONT, "ots_control_intr called: mbuf_p=%x\n",mbuf_p));
	DEBUGP(DEB_FULL,(CE_CONT, "  mb_flags=%x,mb_data[MPS_MG_MT]=%x,",
			(char)mbuf_p->mb_flags, mbuf_p->mb_data[MPS_MG_MT]));
	DEBUGP(DEB_FULL,(CE_CONT, "mb_bind=%x\n", mbuf_p->mb_bind));

	dest = ((mb2socid_t)mps_msg_getdstmid(mbuf_p)<<16) | mps_msg_getdstpid(mbuf_p);
	src = ((mb2socid_t)mps_msg_getsrcmid(mbuf_p)<<16) | mps_msg_getsrcpid(mbuf_p);

	DEBUGP(DEB_FULL,(CE_CONT, "  dest=%x,src=%x\n",dest,src));

	if (mbuf_p->mb_flags & (MPS_MG_TERR | MPS_MG_ESOL))
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "ots_control_intr: MPS_MG_TERR\n"));
		O_control_error(mbuf_p, src, dest);
		mps_free_msgbuf(mbuf_p);
	}
	else if (mbuf_p->mb_data[MPS_MG_MT] == MPS_MG_BGRANT)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "  bgrant complete\n"));
		O_control_bgrant(mbuf_p);
		mps_free_msgbuf(mbuf_p);
	}
	else if (  (mps_mk_mb2soctohid(src) == ics_myslotid())
		 &&(ce = M_find_c(mps_mk_mb2soctopid(src), M_CT_FIND))
		)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "  reply complete\n"));
		O_control_done(ce, mbuf_p, dest);
		if (mbuf_p->mb_bind)
			freemsg((mblk_t *)mbuf_p->mb_bind);
		mps_free_msgbuf(mbuf_p);
	}
	else if (  (mps_mk_mb2soctohid(dest) == ics_myslotid())
		 &&(ce = M_find_c(mps_mk_mb2soctopid(dest), M_CT_FIND))
		)
	{
		if ((mbuf_p->mb_flags & MPS_MG_DONE) == MPS_MG_DONE)
		{
			DEBUGP(DEB_FULL,(CE_CONT, "  transaction %x complete\n",
				mps_msg_gettrnsid(mbuf_p)));
			mps_free_tid((long)ce->channel, mps_msg_gettrnsid(mbuf_p));
			O_control_reply(ce, mbuf_p, src);
			mps_free_msgbuf(mbuf_p);
		}
		else if (mbuf_p->mb_data[MPS_MG_MT] == MPS_MG_UNSOL)
		{
			O_control_unsol(ce, mbuf_p, src);
			mps_free_msgbuf(mbuf_p);
		}
		else if (mbuf_p->mb_data[MPS_MG_MT] == MPS_MG_BREQ)
			O_control_breq(ce, mbuf_p, src);
		else
		{
			cmn_err(CE_WARN,
			  "SV-ots: unknown message src=%x, dest=%x\n",src,dest);
			mps_free_msgbuf(mbuf_p);
		}
	}
	else
	{
		cmn_err(CE_WARN,
			"SV-ots: unknown message src=%x, dest=%x\n",src,dest);
		mps_free_msgbuf(mbuf_p);
	}
}


/* FUNCTION:		ots_data_intr()
 *
 * ABSTRACT:	Processes messages received on data sockets
 *
 * ALGORITHM:
 *
 *	IF (error encountered)
 *		process error message
 *	ELSE IF (end of incoming buffer request)
 *		process completed solicited receive
 *	ELSE IF (source socket is local)
 *		free resources after completed send
 *	ELSE IF (source socket is remote)
 *		IF (end of RSVP transaction)
 *			process input reply
 *		ELSE IF (unsolicited message received)
 *			process input unsolicited message
 *		ELSE IF (buffer request received)
 *			grant buffer input unsolicited message
 *		ENDIF
 *	ENDIF
 *
 * INPUTS:	message buffer
 *
 * OUTPUTS:	processed message
 *
 * RETURN CODE:	none
 *
 * CALLED FROM:	TKI's mps_msg_dispatch()
 */
void
ots_data_intr(mbuf_p)

mps_msgbuf_t *mbuf_p;		/* incoming message buffer */
{
	mb2socid_t dest;	/* destination socket */
	mb2socid_t src;		/* source socket */
	connect *de;		/* local data connection entry */

	DEBUGC('s');
	DEBUGP(DEB_CALL,(CE_CONT, "ots_data_intr called: mbuf_p=%x\n",mbuf_p));
	DEBUGP(DEB_FULL,(CE_CONT, "  mb_flags=%x,mb_data[MPS_MG_MT]=%x\n",
		(char)mbuf_p->mb_flags, mbuf_p->mb_data[MPS_MG_MT]));
	dest = ((mb2socid_t)mps_msg_getdstmid(mbuf_p) << 16) | mps_msg_getdstpid(mbuf_p);
	src = ((mb2socid_t)mps_msg_getsrcmid(mbuf_p) << 16) | mps_msg_getsrcpid(mbuf_p);
	DEBUGP(DEB_FULL,(CE_CONT, "  dest=%x,src=%x\n",dest,src));

	if (mbuf_p->mb_flags & (MPS_MG_TERR | MPS_MG_ESOL))
	{
		DEBUGP(DEB_FULL,(CE_CONT, "ots_data_intr: MPS_MG_TERR mbuf_p=%x,", mbuf_p));
		DEBUGP(DEB_FULL,(CE_CONT, "flags=%x,mb_data=%x,",
			(char)mbuf_p->mb_flags, mbuf_p->mb_data[MPS_MG_MT]));
		DEBUGP(DEB_FULL,(CE_CONT, "src=%x,dst=%x,rid=%x\n",
			src,dest,mps_msg_getrid(mbuf_p)));
		O_data_error(mbuf_p, src, dest);
	}
	else if (mbuf_p->mb_data[MPS_MG_MT] == MPS_MG_BGRANT)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "  bgrant complete\n"));
		O_data_bgrant(mbuf_p);
		mps_free_msgbuf(mbuf_p);
	}
	else if (  (mps_mk_mb2soctohid(src) == ics_myslotid())
		 &&(de = M_find_d(mps_mk_mb2soctopid(src)))
		)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "  transactionless send complete\n"));
		O_data_done(de, mbuf_p);
		mps_free_msgbuf(mbuf_p);
	}
	else if (  (mps_mk_mb2soctohid(dest) == ics_myslotid())
		 &&(de = M_find_d(mps_mk_mb2soctopid(dest)))
		)
	{
		if ((mbuf_p->mb_flags & MPS_MG_DONE) == MPS_MG_DONE)
		{
			DEBUGP(DEB_FULL,(CE_CONT, "  transaction %x complete\n",
				mps_msg_gettrnsid(mbuf_p)));
			mps_free_tid((long)de->channel, mps_msg_gettrnsid(mbuf_p));
			O_data_reply(de, mbuf_p);
			mps_free_msgbuf(mbuf_p);
		}
		else if (mbuf_p->mb_data[MPS_MG_MT] == MPS_MG_UNSOL)
		{
			O_data_unsol(de, mbuf_p, src);
			mps_free_msgbuf(mbuf_p);
		}
		else if (mbuf_p->mb_data[MPS_MG_MT] == MPS_MG_BREQ)
			O_data_breq(de, mbuf_p, src);
		else
		{
			cmn_err(CE_WARN,
			  "SV-ots: unknown message src=%x, dest=%x\n",src,dest);
			mps_free_msgbuf(mbuf_p);
		}
	}
	else
	{
		cmn_err(CE_WARN,
			  "SV-ots: unknown message src=%x, dest=%x\n",src,dest);
		mps_free_msgbuf(mbuf_p);
	}

}


/* FUNCTION:		O_control_bgrant()
 *
 * ABSTRACT:	Process received solicited RSVP request
 *
 *	The input message is interpreted and the appropriate upstream
 *	handler is called.
 *
 * CALLED FROM:	ots_control_intr()
 */
O_control_bgrant(mbuf_p)

mps_msgbuf_t *mbuf_p;	/* current message pointer */
{
	struct cbreq_info *req;	/* saved buffer request information */
	connect *ce;		/* targeted control entry */
	mb2socid_t rsoc;	/* requesting socket */
	mblk_t *mptr;		/* references granted data buffer */
	otsstr *ots_m;		/* OTS message structure */
	int tid;		/* transaction id */

	mptr = (mblk_t *)mbuf_p->mb_bind;
	req = (struct cbreq_info *)mptr->b_datap->db_base;
	ots_m = &req->control;
	ce = req->ce;
	rsoc = req->rsoc;
	tid = req->tid;

	/*
	 * Don't process the message if the socket's associated endpoint
	 *	went away while buffer request completed.  Instead, free
 	 *	the resources and cancel the transaction.
	 */
	if (ce->ep)
	{
		switch (ots_m->command)
		{
		case O_CONNECT:
			iMB2_conn_ind(ce, rsoc, ots_m->co.lport,
				(struct o_connect *)ots_m, (uchar)tid, NULL,
				0, mptr);
			freemsg(mptr);
			return;
		case O_DISCONNECT:
			mptr->b_wptr = mptr->b_rptr + ots_m->di.userdata;
			iMB2_discon_ind(ce, rsoc, ots_m->di.rport,
				(struct o_disconnect *)ots_m,
				(uchar)tid, mptr);
			return;
		case O_DATAGRAM:
			iMB2_unitdata_ind(ce, rsoc, (struct o_datagram *)ots_m,
				tid, mptr);
			return;
		case O_ACCEPT:		/* for routing implementation */
		case O_ACCEPT_CANCEL:
		case O_DATAGRAM_OPEN:
		case O_DATAGRAM_CANCEL:
		default:
			DEBUGP(DEB_ERROR,(CE_CONT, "O_control_bgrant: bad command %x\n",
					ots_m->command));
			freemsg(mptr);
			break;
			/* falls through to cancel */
		}
	}
	freemsg(mptr);
	mps_AMPcancel(ce->channel, rsoc, (unchar)tid);
}


/* FUNCTION:			O_control_breq()
 *
 * ABSTRACT:	Process buffer request messages received at control ports
 *
 *	One streams buffer is used to hold control and data portions of message.
 *	The client is only granted space from the data portion.
 *
 *	Because we don't fragment on control sockets (except on incoming
 *	datagrams), the allocb() better succeed because we won't try again.
 *	That's why BPRI_HI is specified and we cancel the transaction if
 *	we can't get a buffer.
 *
 * ALGORITHM:
 *
 *	IF (request not a transaction)
 *		reject the buffer request
 *		RETURN
 *	ELSE IF (no associated endpoint)
 *		cancel transaction
 *		RETURN
 *	ENDIF
 *	IF (datagram request)
 *		IF (queue blocked or can't allocate resources for request)
 *			issue a buffer reject (will initiate fragmentation)
 *			save copy of buffer request in endpoint
 *			schedule fragmentation via timeout() or bufcall()
 *			RETURN
 *		ENDIF
 *		initialize resources to satisfy request [O_alloc_csolbuf()]
 *	ELSE
 *		allocate and initialize resources to satisfy request
 *		IF (can't allocate resources)
 *			cancel request
 *			RETURN
 *		ENDIF
 *	ENDIF
 *	receive data [mps_mk_bgrant() and mps_AMPreceive()]
 *	
 *
 * NOTE: this routine consumes mbuf_p
 *
 * CALLED FROM:	ots_control_intr()
 */
O_control_breq(ce, mbuf_p, rsoc)

connect *ce;		/* targeted control entry */
mps_msgbuf_t *mbuf_p;	/* current message pointer */
mb2socid_t rsoc;	/* requesting socket */
{
	ulong dsize;		/* size of data portion of message */
	ulong asize = 0;	/* needed buffer size (passed to bufcall) */
	struct dma_buf *dbp;		/* references data portion of message */
	mblk_t *mptr = NULL;	/* control and data message block */
	otsstr *ots_m;		/* control message in buffer request */
	struct cbreq_info *req;	/* saved buffer request information */
	endpoint *ep;		/* component endpoint on ce's list */
	mps_msgbuf_t *mbuf_psav;	/* pointer to copy of current message */
	int tid;		/* transaction id */
	bool unblocked;		/* TRUE: upstream is NOT blocked */
	bool supported;		/* TRUE: OTS Datagram protocol supported */
	void O_control_snf();

	ots_m = (otsstr *)&mbuf_p->mb_data[MPS_MG_BRUD];
	dsize = mps_msg_getbrlen(mbuf_p); 

	if ((tid = mps_msg_gettrnsid(mbuf_p)) == 0)	/* only allow RSVP's */
		O_breq_reject((ushort)ce->channel, rsoc, mbuf_p);

	else if ((ep = ce->ep) == NULL)		/* cancel if no endpoint */
	{
		mps_AMPcancel(ce->channel, rsoc, (unchar)tid);
		mps_free_msgbuf(mbuf_p);
	}

	else if (  (ots_m->command == O_DATAGRAM)
		 &&(  ((ep = M_find_dgendpoint(ce)) == NULL)
		    ||((supported = O_version_supported(ots_m->da.version)) == FALSE)
		    ||((unblocked = canput(ep->rd_q)) == FALSE)
		    ||((mptr = O_alloc_csolbuf(ep, mbuf_p, &dbp, &asize)) == NULL)
		   )
		)
	{
		if (supported == FALSE)
		{
			O_datagram_rsp(ce, rsoc, E_PROTOCOL_VERSION,
				(uchar)tid);
			mps_free_msgbuf(mbuf_p);
			return;
		}
		else if (  (ep == NULL)
			 ||((mps_msg_gettransctl(mbuf_p) & MPS_MG_RQMF) == FALSE)
			 ||((mbuf_psav = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
			)
		{
			mps_AMPcancel(ce->channel, rsoc, (unchar)tid);
			mps_free_msgbuf(mbuf_p);
			return;
		}

		/* preserve message on endpoint */

		bcopy((char *)mbuf_p->mb_data, (char *)mbuf_psav->mb_data,
			MPS_MAXMSGSZ);
		ep->breq = mbuf_psav;

		/* reject to initiate fragmentation */

		O_breq_reject((ushort)ce->channel, rsoc, mbuf_p);

		if (unblocked == FALSE)
			timeout(O_control_snf, ep, (int)CSBR_INIT(ep));
		else if (asize)
		{
			if (bufcall((uint)asize, BPRI_LO, O_control_snf, ep)
									== 0)
			{
				cmn_err(CE_WARN,
				"SV-ots: O_control_snf bufcall failed\n");
				timeout(O_control_snf, ep, (int)CSBR_INIT(ep));
			}
		}
		else
			timeout(O_control_snf, ep, (int)CSBR_INIT(ep));
	}

	else if (  (ots_m->command != O_DATAGRAM)
		 &&(  ((mptr = allocb((int)(sizeof(*req)+dsize), BPRI_HI))
									== NULL)
		    ||((dbp = mps_get_dmabuf(2,DMA_NOSLEEP)) == NULL)
		   )
		)
	{
		if (mptr)
		{
			freemsg(mptr);
			ots_stat[ST_TKIDBLK]++;
		}
		mps_free_msgbuf(mbuf_p);
		mps_AMPcancel(ce->channel, rsoc, (unchar)tid);
	}
	else
	{
		mptr->b_rptr = mptr->b_datap->db_base;
		req = (struct cbreq_info *)mptr->b_rptr;
		bcopy(ots_m, &req->control, sizeof(otsstr));
		req->rsoc = rsoc;
		req->ce = ce;
		req->tid = tid;
		if (ots_m->command != O_DATAGRAM)
		{
			mptr->b_rptr += sizeof(*req);
			mptr->b_wptr = mptr->b_rptr + dsize;
			dbp->count = dsize;
			dbp->address = (ulong)kvtophys((caddr_t)mptr->b_rptr);
			(dbp->next_buf)->count = 0;
			(dbp->next_buf)->next_buf = NULL;
		}
		mps_mk_bgrant(mbuf_p, rsoc, mbuf_p->mb_data[MPS_MG_RI], (int)dsize);
		mbuf_p->mb_bind = (ulong) mptr;
		if (mps_AMPreceive((long)ce->channel, rsoc, mbuf_p, dbp))
		{
			cmn_err(CE_WARN,"SV-ots: O_control_breq mps_AMPreceive error on port %x\n",ce->port);
			ots_stat[ST_TKIFAILS]++;
			freemsg(mptr);
			mps_free_msgbuf(mbuf_p);
			mps_free_dmabuf(dbp);
			do
				iTLI_pferr(ep, M_NO_RESOURCES);
			while (ep = ep->next);
		}
	}
}


/* FUNCTION:			O_control_cancel()
 *
 * ABSTRACT:	Notify appropriate endpoint of cancelled transaction
 *
 *	This routine searches through the control entry's endpoint list
 *	looking for the endpoint which sent the cancelled transaction.  If
 *	found, a protocol error is sent upstream and any resource ptrs are
 *	cleared.  If the endpoint list is gone, the control entry is in the
 *	process of being freed, so free it.
 *
 * CALLED FROM:	O_control_reply()
 */
O_control_cancel(ce, tid, mbuf_p)

connect *ce;		/* local control connection entry */
uchar tid;		/* message MB-II transaction id */
mps_msgbuf_t *mbuf_p;	/* received message */
{
	endpoint *ep;		/* associated endpoint(s) */
	mblk_t *mptr = NULL;	/* used to send T_error_ack upstream */
	int count = 0;		/* number of associated endpoint */

	DEBUGP(DEB_CALL,(CE_CONT, "O_control_cancel: ce=%x, tid=%x\n", ce, tid));

	if (  (ep = ce->ep)
	    &&(  (mptr = (mblk_t *)mbuf_p->mb_bind)
	       ||(mptr = allocb(sizeof(struct T_error_ack), BPRI_HI))
	      )
	   )	/* notify endpoint of error, if possible */
	{
		if (ep = M_find_xctid_ep(ce, tid, &count))
		{
			ep->xctid = 0;
			DEBUGP(DEB_FULL,(CE_CONT, "  cancel on ep=%x\n",ep));
			iTLI_pterr(ep, mptr, TSYSERR, ECOMM);
			return;
		}
		else if (count == 1)
			iTLI_pterr(ce->ep, mptr, TSYSERR, ECOMM);
		else
			/*
			 *  don't know who sent the transaction,
			 *   just free the mblock
			 */
			freemsg(mptr);
	}
	else	/* free control entry */
	{
		if (mptr)
			freemsg(mptr);
		M_free_connect(ce);
	}
}


/* FUNCTION:		 O_control_done()
 *
 * ABSTRACT:	Clear receive transaction variable in endpoint structure
 *
 * CALLED FROM:	ots_control_intr()
 */
O_control_done(ce, mbuf_p, rsoc)

connect *ce;		/* receiving connection entry */
mps_msgbuf_t *mbuf_p;	/* message buffer */
mb2socid_t rsoc;	/* remote socket */
{
	uchar tid;
	endpoint *ep;

	if (  (tid = mps_msg_gettrnsid(mbuf_p))
	    &&(ep = M_find_rctid_ep(ce, tid, rsoc, NULL))
	   )
		ep->rctid = 0;
	else
	{
		DEBUGP(DEB_ERROR,
			(CE_CONT, "O_control_done: unknown MPS_MG_DONE ce=%x,mbuf_p=%x\n",
			ce, mbuf_p));
	}
}


/* FUNCTION:		 O_control_error()
 *
 * ABSTRACT:
 *
 *	This routine is normally called when:
 *
 *	1. We couldn't reply to an RSVP.  We need to free any message
 *	   resources and notify the endpoint.
 *	  
 *	2. TKI is returning us the message block and STREAMS resources
 *	   associated with a cancelled transaction.  If no endpoint
 *	   is associated with the control entry, an earlier call to
 *	   M_free_connect() cancelled the transaction and we need to free
 *	   the tid, STREAMS buffers and call M_free_connect() again to
 *	   close the port.  If the control entry references an endpoint,
 *	   the socket is still active and we shouldn't call M_free_connect().
 *
 * CALLED FROM:	ots_control_intr()
 */
O_control_error(mbuf_p, src, dest)

mps_msgbuf_t *mbuf_p;	/* received message */
mb2socid_t src;		/* source socket */
mb2socid_t dest;	/* destination socket */
{
	otsstr *ots_m;		/* OTS response message */
	connect *ce;		/* local control connection entry */
	endpoint *ep;		/* endpoint(s) associated with ce */
	mblk_t *mptr;		/* used to send T_error_ack upstream */
	ushort tid;		/* message MB-II transaction id */
	ushort count;		/* number of endpoints tied to ce */

	if (  (mps_mk_mb2soctohid(src) == ics_myslotid())
	    &&(tid = mps_msg_gettrnsid(mbuf_p))
	    &&(ce = M_find_c(mps_mk_mb2soctopid(src), M_CT_FIND))
	   )
	{
		DEBUGP(DEB_FULL,(CE_CONT, "O_control_error:ce=%x,mptr=%x,tid=%x\n",
				ce,mbuf_p->mb_bind,tid));
		if (mbuf_p->mb_data[MPS_MG_MT] == MPS_MG_UNSOL)
			ots_m = (otsstr *)&mbuf_p->mb_data[MPS_MG_UMUD];
		else
			ots_m = (otsstr *)&mbuf_p->mb_data[MPS_MG_BRUD];

		if (ots_m->command & O_CONTROL_RESPONSE)
		{
			if (  (mptr = (mblk_t *)mbuf_p->mb_bind)
			    ||(mptr = allocb(sizeof(struct T_error_ack), BPRI_HI))
			   )
			{
				if (ep = M_find_rctid_ep(ce, tid, dest,
								(int *)&count))
				{
					ep->rctid = 0;
					iTLI_pterr(ep, mptr, TSYSERR, ECOMM);
					return;
				}
				if (count == 1)
					iTLI_pterr(ce->ep, mptr, TSYSERR,ECOMM);
				else
					freemsg(mptr);
			}
		}
		else
		{
			mps_free_tid((long)ce->channel, tid);
			if (mbuf_p->mb_bind)
				freemsg((mblk_t *)mbuf_p->mb_bind);
			if (ce->ep == NULL)
				M_free_connect(ce);
		}
	}
	else if (mbuf_p->mb_bind)
		freemsg((mblk_t *)mbuf_p->mb_bind);
}


/* FUNCTION:			O_control_snf()
 *
 * ABSTRACT:	Process fragmented buffer request message on control socket
 *
 * ALGORITHM:
 *
 *	ensure endpoint hasn't been deleted since routine scheduled
 *	IF (unblocked upstream AND got buffers to grant)
 *		save request info in reserved TPI header
 *		grant data buffer [mps_mk_bgrant, mps_AMPreceive]
 *	ELSE
 *		IF (buffers NOT available)
 *			schedule fragmentation via bufcall()
 *		ELSE
 *			schedule fragmentation via timeout()
 *		ENDIF
 *	ENDIF
 *
 * CALLED FROM:	timeout() - upstream stream is block
 */
void
O_control_snf(ep)

endpoint *ep;		/* receiving endpoint */
{
	mps_msgbuf_t *mbuf_p;	/* current message pointer */
	mb2socid_t rsoc;	/* requesting socket */
	struct dma_buf *dbp;		/* references data portion of message */
	mblk_t *mptr = NULL;	/* data message block */
	otsstr *ots_m;		/* control message in buffer request */
	struct cbreq_info *req;	/* saved buffer request information */
	ulong dsize;		/* size of requested data buffer */
	ulong asize = 0;	/* needed buffer size (passed to bufcall) */
	uchar tid;		/* transaction id */
	int pri;		/* saved interrupt priority */

	pri = SPL();

	if ((mbuf_p = ep->breq) == NULL)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "O_control_snf: ep=%x gone\n",ep));
		splx(pri);
		return;
	}

	dsize = mps_msg_getbrlen(mbuf_p); 
	tid = mps_msg_gettrnsid(mbuf_p);
	rsoc = ((mb2socid_t)mps_msg_getsrcmid(mbuf_p)<<16) | mps_msg_getsrcpid(mbuf_p);
	ots_m = (otsstr *)&mbuf_p->mb_data[MPS_MG_BRUD];

	DEBUGP(DEB_FULL,(CE_CONT, "O_control_snf: ep=%x,mbuf=%x,",ep,mbuf_p));
	DEBUGP(DEB_FULL,(CE_CONT, "rsoc=%x,dsize=%x,tid=%x\n",rsoc,dsize,tid));

	if (  (canput(ep->rd_q))
	    &&(mptr = O_alloc_csolbuf(ep, mbuf_p, &dbp, &asize))
	   )
	{
		ep->breq = NULL;
		mptr->b_rptr = mptr->b_datap->db_base;
		req = (struct cbreq_info *)mptr->b_rptr;
		bcopy(ots_m, &req->control, sizeof(otsstr));
		req->rsoc = rsoc;
		req->ce = ep->control;
		req->tid = tid;
		mps_mk_bgrant(mbuf_p, rsoc, mbuf_p->mb_data[MPS_MG_RI], (int)dsize);
		mbuf_p->mb_bind = (ulong) mptr;
		if (mps_AMPreceive_frag((long)req->ce->channel, mbuf_p, rsoc, tid,
					dbp))
		{
			cmn_err(CE_WARN,"SV-ots: O_control_snf mps_AMPreceive_frag error on port %x\n",req->ce->port);
			ots_stat[ST_TKIFAILS]++;
			freemsg(mptr);
			mps_free_msgbuf(mbuf_p);
			mps_free_dmabuf(dbp);
			iTLI_pferr(ep, M_NO_RESOURCES);
		}
	}
	else		/* no receive fragment this time */
	{
		if (asize)
		{
			if (bufcall((uint)asize, BPRI_LO, O_control_snf, ep)
									== 0)
			{
				cmn_err(CE_WARN,
				  "SV-ots: O_control_snf bufcall failed\n");
				timeout(O_control_snf, ep, (int)CSBR_RETRY(ep));
			}
		}
		else
			timeout(O_control_snf, ep, (int)CSBR_RETRY(ep));
	}
	splx(pri);
}


/* FUNCTION:			O_control_unsol()
 *
 * ABSTRACT:	Process unsolicited message received at control socket
 *
 * CALLED FROM:	ots_intr_control()
 */
O_control_unsol(ce, mbuf_p, rsoc)

connect *ce;		/* targeted control entry */
mps_msgbuf_t *mbuf_p;	/* current message pointer */
mb2socid_t rsoc;	/* requesting socket */
{
	otsstr *ots_m;		/* OTS message structure */
	int tid;		/* transaction id */

	ots_m = (otsstr *)&mbuf_p->mb_data[MPS_MG_UMUD];

	if ((tid = mps_msg_gettrnsid(mbuf_p)) == 0)
	{
		/* just ignore message if not RSVP */
		DEBUGP(DEB_ERROR,
		  (CE_CONT, "O_control_unsol: no transaction from socket %x\n",rsoc));
		return;
	}
	else if (ce->ep)
	{
		switch (ots_m->command)
		{
			case O_CONNECT:
				iMB2_conn_ind(ce, rsoc, ots_m->co.lport,
					(struct o_connect *)ots_m, (uchar)tid,
					NULL, 0, (mblk_t *)NULL);
				return;
			case O_DISCONNECT:
				iMB2_discon_ind(ce, rsoc, ots_m->di.rport,
					(struct o_disconnect *)ots_m,
					(uchar)tid, (mblk_t *)NULL);
				return;
			case O_ORD_RELEASE:
				iMB2_ordrel_ind(ce, rsoc,
					(struct o_release *)ots_m, (uchar)tid);
				return;
			case O_NEGOTIATE_VER:
				iMB2_negotiate_version(ce, rsoc,
					(struct o_negotiate_ver *)ots_m, tid);
				return;
			case O_GET_INFO:
				O_get_info_rsp(ce, rsoc, E_OK, (uchar)tid);
				return;
			case O_ACCEPT:
			case O_ACCEPT_CANCEL:
			case O_DATAGRAM_OPEN:
			case O_DATAGRAM_CANCEL:
			default:
				DEBUGP(DEB_ERROR,
					(CE_CONT, "O_control_unsol: bad command %x\n",
						ots_m->command));
				/* fall through to cancel */
				break;
		}
	}
	if (mps_AMPcancel(ce->channel, rsoc, (unchar)tid))
	{
		DEBUGP(DEB_ERROR,
		(CE_CONT, "O_control_unsol: can't cancel tid %x from socket %x\n",
			tid,rsoc));
	}
}


/* FUNCTION:			O_control_reply()
 *
 * ABSTRACT:	Process completed solicited RSVP at control socket
 *
 *	The transaction may have been cancelled either by the remote MPC
 *	or OTS correspondent.  O_control_cancel() is called to send an error
 *	up the appropriate stream which initiated the transaction.
 *	
 *	If the transaction completed successfully, the control portion of
 *	message is examined for the OTS response and the appropriate upstream
 *	handler is called.
 *
 * CALLED FROM:	ots_control_intr()
 */
O_control_reply(ce, mbuf_p, rsoc)

connect *ce;		/* local control entry */
mps_msgbuf_t *mbuf_p;	/* current message pointer */
mb2socid_t rsoc;	/* remote socket */
{
	connect *de;		/* local data entry */
	endpoint *ep;		/* endpoint */
	mblk_t *data;		/* data message block */
	mblk_t *edata;		/* extended data message block */
	otsstr *ots_m;		/* OTS response message */
	uchar tid;		/* transaction */
	
	if (mbuf_p->mb_data[MPS_MG_MT] == MPS_MG_UNSOL)
		ots_m = (otsstr *)&mbuf_p->mb_data[MPS_MG_UMUD];
	else
		ots_m = (otsstr *)&mbuf_p->mb_data[MPS_MG_BRUD];

	if (  (ce->ep == NULL)
	    ||(mps_msg_gettransctl(mbuf_p) & MPS_MG_RSCN) /* transaction cancelled */
	   )
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_control_reply: transaction cancelled\n"));
		O_control_cancel(ce, mps_msg_gettrnsid(mbuf_p), mbuf_p);
		return;
	}

	DEBUGP(DEB_FULL,(CE_CONT, "O_control_reply: ce=%x, cmd=%x\n",ce,ots_m->command));
	switch (ots_m->command)
	{
		case O_CONNECT_RSP:
			data = (mblk_t *)mbuf_p->mb_bind;
			/* free initial rsvp resources */
			if (edata = unlinkb(data))
				freemsg(edata);
			iMB2_conn_con(ce, rsoc, (struct o_connect_rsp *)ots_m,
				data);
			break;
		case O_DISCONNECT_RSP:
			tid = mps_msg_gettrnsid(mbuf_p);
			if (data = (mblk_t *)mbuf_p->mb_bind)
			{
				if (edata = (mblk_t *)unlinkb(data))
					freemsg(edata);
			}
			if (ep = M_find_xctid_ep(ce, tid, NULL))
			{
				ep->xctid = 0;
				if (ep->timeout)
				{
					untimeout(ep->timeout);
					ep->timeout = 0;
				}
				if (data)
					iTLI_discon_req_complete(ep, data,
						(int)ots_m->dir.errcode);
				/*
				 * if endpoint closing down, call iMB2_abort()
				 *  (again) to remove reference to control
				 *  socket (and possibly free ce).
				 */
				if (ep->str_state & C_CLOSE)
					iMB2_abort(ep);
			}
			break;
		case O_DATAGRAM_RSP:
			tid = mps_msg_gettrnsid(mbuf_p);
			if (ep = M_find_xctid_ep(ce, tid, NULL))
			{
				ep->xctid = 0;
				iTLI_unitdata_req_complete(ep,
					(mblk_t *)mbuf_p->mb_bind,
					(int)ots_m->dar.errcode);
				if (  (qsize(WR(ep->rd_q)))
				    ||(ep->nbr_datarq)
				   )
					timeout(qenable, WR(ep->rd_q), 1);	
			}
			break;
		case O_ORD_RELEASE_RSP:
			if (  (ots_m->rer.errcode == E_OK)
			    &&(de = M_find_d(ots_m->rer.rport))
			   )
			{
				de->ep->xctid = 0;
				iTLI_ordrel_ind(de->ep,
					(mblk_t *)mbuf_p->mb_bind, 0);
				M_free_connect(de);
			}
			else
			{
				DEBUGP(DEB_ERROR,(CE_CONT, " bad ord release rsp\n"));
			}
			break;
		/*
		 * O_NEGOTIATE_VER and O_GET_INFO support is only partially
		 * implemented.  Currently, no upper-level SV-ots routines
		 * send these requests, so there's no routine to call if
		 * we get a response.
		 */
		case O_NEGOTIATE_VER_RSP:
			DEBUGP(DEB_FULL,(CE_CONT, "O_control_reply: O_NEGO_VER_RSP:"));
			DEBUGP(DEB_FULL,(CE_CONT, "errcode=%x,version=%x\n",
				ots_m->nvr.errcode, ots_m->nvr.version));
			break;
		case O_GET_INFO_RSP:
			DEBUGP(DEB_FULL,(CE_CONT, "O_control_reply: O_GET_INFO_RSP:"));
			DEBUGP(DEB_FULL,(CE_CONT, "errcode=%x,data=%x,options=%x\n",
				ots_m->gir.errcode, ots_m->gir.data,
				ots_m->gir.options));
			if (data = (mblk_t *)mbuf_p->mb_bind)
			{
				struct get_info_data *gidata;
				{
				gidata = (struct get_info_data *)data->b_rptr;
				DEBUGP(DEB_FULL,
					(CE_CONT, " netaddr=%x\n",gidata->netaddr));
				DEBUGP(DEB_FULL,
					(CE_CONT, " options=%x\n",gidata->options));
				DEBUGP(DEB_FULL,
					(CE_CONT, " tsdu_size=%x\n",gidata->tsdu_size));
				DEBUGP(DEB_FULL,
				       (CE_CONT, " etsdu_dize=%x\n",gidata->etsdu_size));
				DEBUGP(DEB_FULL,
					(CE_CONT, " connect=%x\n",gidata->connect));
				DEBUGP(DEB_FULL,
					(CE_CONT, " discon=%x\n",gidata->discon));
				DEBUGP(DEB_FULL,
					(CE_CONT, " service=%x\n",gidata->service));
				DEBUGP(DEB_FULL,
					(CE_CONT, " tp_type=%s\n",gidata->tp_type));
				freemsg(data);
				}
			}
			break;
		/*
		 * implementation of the following responses
		 *  awaits the addition of network controller support
		 */
		case O_ACCEPT_RSP:
		case O_ACCEPT_CANCEL_RSP:
		case O_DATAGRAM_OPEN_RSP:
		case O_DATAGRAM_CANCEL_RSP:

		default:
			DEBUGP(DEB_ERROR,(CE_CONT, "O_control_reply: bad command %x\n",
					ots_m->command));
			if (mbuf_p->mb_bind)
				freemsg((mblk_t *)mbuf_p->mb_bind);
			break;
	}
}


/* FUNCTION:			O_data_bgrant()
 *
 * ABSTRACT:	Process received solicited message
 *
 *	After queueing the message on the driver's local upstream queue
 *	(iTLI_data_ind()), the routine may either (1) call O_data_snf() if
 *	additional fragments are yet to be received or (2) send a
 *	transaction response if the saved received tid is non-zero.
 *
 * NOTE:
 *
 *	The check on connect entry type is necessary because during the
 *	period between breq and bgrant, the data connection entry may have
 *	been deleted.
 *
 * CALLED FROM:	ots_data_intr()
 */
O_data_bgrant(mbuf_p)

mps_msgbuf_t *mbuf_p;	/* current message pointer */
{
	connect *de;		/* receiving data entry */
	struct dbreq_info *db;	/* saved breq info (also used as TLI hdr) */
	mblk_t *header;		/* header */
	uchar errcode = 0;	/* error code for O_transaction_rsp() */
	void O_data_snf();

	header = (mblk_t *)mbuf_p->mb_bind;
	db = (struct dbreq_info *)header->b_datap->db_base;
	de = db->de;

	if (  (de->ep)			/* NOTE */
	    &&(de->type == CT_DATA)
	   )
	{
		DEBUGP(DEB_FULL,(CE_CONT, "O_data_bgrant: de=%x,tid=%x\n",
				de,de->skt.data.rdtid));

		switch (db->type)
		{
			case O_DATA:
				iTLI_data_ind(de->ep, header, TRUE);
				break;
			case O_EOM_DATA:
				iTLI_data_ind(de->ep, header, FALSE);
				break;
			case O_EXPEDITED_DATA:
				iTLI_exdata_ind(de->ep, header, FALSE);
				break;
			default:
				DEBUGP(DEB_ERROR,(CE_CONT, "O_data_bgrant: bad command %x\n",
						db->type));
				freemsg(header);
				errcode = E_PROTOCOL_VERSION;  	/* what code ?? */
				break;
		}
		if (de->skt.data.breq)
		{
			DSBR_INIT(de);
			O_data_snf(de);
		}
		else if (de->skt.data.rdtid)
			O_transaction_rsp(de, errcode);
	}
	else
	{
		/*
		 * Any old transactions should have been cancelled by
		 * M_free_connect(de), so just free the STREAMS resources
		 */
		freemsg(header);
	}
}


/* FUNCTION:			O_data_breq()
 *
 * ABSTRACT:	Process buffer request messages received at data ports
 *
 *	The goal of this routine is to grant a data buffer only if
 *	STREAMS buffers are available to receive the data AND the upstream
 *	queues are NOT blocked due to flow control.  In the former situation,
 *	bufcall() is used to retry the grant when buffers become available.
 *	In the latter situation, timeout is used to reschedule grant retry
 *	after an arbitrary wait period.  In both situations, fragmentation is
 *	used to receive the buffer and O_data_snf() is called to perform the
 *	fragmentation.  Thus, the buffer request must be sent as a transaction.
 *	In addition, only one buffer may be fragmented at a time and this
 *	fragmentation does NOT begin until the unsol queue is drained.
 *
 * ALGORITHM:
 *
 *	IF (endpoint went [or going] away)
 *		cancel or reject the buffer request
 *	ELSE IF (valid message)
 *		IF (need to fragment)
 *			IF (ok to fragment this request)
 *				save copy of buffer request in connection entry
 *				IF (upstream blocked)
 *					schedule fragmentation via timeout()
 *				ELSE IF (buffers NOT available)
 *					schedule fragmentation via bufcall()
 *				ELSE
 *					schedule fragmentation via timeout()
 *			ENDIF
 *			issue a buffer reject (will initiate fragmentation)
 *		ELSE
 *			save request info in reserved TPI header
 *			grant data buffer [mps_mk_bgrant, mps_AMPreceive]
 *		ENDIF
 *	ELSE
 *		send fatal error upstream
 *	ENDIF
 *
 * NOTE: this routine consumes mbuf_p
 *
 * CALLED FROM:	ots_data_intr()
 */
O_data_breq(de, mbuf_p, dsoc)

connect *de;		/* targeted data entry */
mps_msgbuf_t *mbuf_p;	/* current message pointer */
mb2socid_t dsoc;	/* requesting socket */
{
	struct dma_buf *dbp;		/* references data portion of message */
	mblk_t *mptr;		/* data message block */
	mps_msgbuf_t *mbuf_psav;	/* pointer to copy of current message */
	struct o_solicited_data
		*sd;		/* OTS data transfer buffer request message */
	struct dbreq_info *db;	/* saved breq info (used later as TLI hdr) */
	ulong dsize;		/* size of requested data buffer */
	ulong asize;		/* needed buffer size (passed to bufcall) */
	bool unblocked;		/* TRUE: upstream is NOT blocked */
	bool empty;		/* unsol queue is empty */
	uchar tid;		/* transaction id */
	int pri;		/* saved interrupt priority */
	void O_data_snf();

	pri = SPL();

	sd = (struct o_solicited_data *)&mbuf_p->mb_data[MPS_MG_BRUD];
	dsize = mps_msg_getbrlen(mbuf_p); 
	asize = 0;
	tid = mps_msg_gettrnsid(mbuf_p);

	DEBUGP(DEB_FULL,(CE_CONT, "O_data_breq: de=%x,mbuf=%x,dsoc=%x,",de,mbuf_p,dsoc));
	DEBUGP(DEB_FULL,(CE_CONT, "dsize=%x,type=%x,tid=%x\n",dsize,sd->type,tid));

	if (  (de->ep == NULL)		/* ensure ep still around */
	    ||(de->ep->discon_ind)	/* and no discon ind pending */
	   )
	{
		if (  (tid)
		    &&(mps_AMPcancel(de->channel, dsoc, tid))
		   )
		{
			mps_free_msgbuf(mbuf_p);
			DEBUGP(DEB_ERROR,
			(CE_CONT, "O_data_bgrant: can't cancel tid %x on chan %x\n",
			de,tid));
		}
		else
			O_breq_reject((ushort)de->channel, dsoc, mbuf_p);
		splx(pri);
		return;
	}

	switch (sd->type)
	{
	 case O_DATA:
	 case O_EOM_DATA:
	 case O_EXPEDITED_DATA:
		break;
	 default:
		DEBUGP(DEB_ERROR,(CE_CONT, "O_data_breq: bad command %x\n", sd->type));
		mps_free_msgbuf(mbuf_p);
		iTLI_pferr(de->ep, M_PROTOCOL_ERROR);
		splx(pri);
		return;
	}

	if (  ((empty = Q_EMPTY(de)) == FALSE)
	    ||((unblocked = canput(de->ep->rd_q)) == FALSE)
	    ||(dsize > otscfg.buffer_size)
	    ||((mptr = O_alloc_dsolbuf(de, mbuf_p, &dbp, &asize, FALSE)) == NULL)
	   )
	{
		if (  (tid != 0)
		    &&(de->skt.data.rdtid == 0)
		    &&(mps_msg_gettransctl(mbuf_p) & MPS_MG_RQMF)
		    &&(mbuf_psav = mps_get_msgbuf(KM_NOSLEEP))
		   )
		{	/* setup for fragmentation */
			bcopy((char *)mbuf_p->mb_data,
				(char *)mbuf_psav->mb_data, MPS_MAXMSGSZ);
			de->skt.data.breq = mbuf_psav;

			if (empty)
			{
				if (unblocked == FALSE)
					timeout(O_data_snf, de,
						(int)DSBR_INIT(de));
				else if (dsize > otscfg.buffer_size)
				{
					O_breq_reject((ushort)de->channel,
						dsoc, mbuf_p);
					DSBR_INIT(de);
					O_data_snf(de);
					splx(pri);
					return;
				}
				else if (asize)
				{
					if (bufcall((uint)asize, BPRI_LO,
							O_data_snf,de) == 0)
					{
						cmn_err(CE_WARN,
					"SV-ots: O_data_breq bufcall failed\n");
						timeout(O_data_snf, de,
							(int)DSBR_INIT(de));
					}
				}
				else
					timeout(O_data_snf, de,
						(int)DSBR_INIT(de));
			}
		}
		O_breq_reject((ushort)de->channel, dsoc, mbuf_p);
	}
	else
	{
		DEBUGP(DEB_FULL,(CE_CONT, "  header=%x,data=%x\n",mptr,mptr->b_cont));
		db = (struct dbreq_info *)mptr->b_rptr;
		db->type = sd->type;
		db->de = de;
		de->skt.data.rdtid = tid;
		mps_mk_bgrant(mbuf_p, dsoc, mbuf_p->mb_data[MPS_MG_RI], (int)dsize);
		mbuf_p->mb_bind = (ulong) mptr;
		if (mps_AMPreceive((long)de->channel, dsoc, mbuf_p, dbp))
		{
			cmn_err(CE_WARN, "SV-ots: O_data_breq mps_AMPreceive error on port %x\n",de->port);
			ots_stat[ST_TKIFAILS]++;
			freemsg(mptr);
			mps_free_msgbuf(mbuf_p);
			mps_free_dmabuf(dbp);
			iTLI_pferr(de->ep, M_NO_RESOURCES);
		}
	}
	splx(pri);
}


/* FUNCTION:			O_data_snf()
 *
 * ABSTRACT:	Process fragmented buffer request message on data socket
 *
 * ALGORITHM:
 *
 *	ensure data connection entry hasn't been deleted since routine scheduled
 *	IF (unblocked upstream AND got buffers to grant)
 *		IF (still more data to receive)
 *			modify message type in OTS header to "NOT EOM"
 *			copy message and store in connection entry
 *		ENDIF
 *		save request info in reserved TPI header
 *		grant data buffer [mps_mk_bgrant, mps_AMPreceive]
 *	ELSE
 *		IF (buffers NOT available)
 *			schedule fragmentation via bufcall()
 *		ELSE
 *			schedule fragmentation via timeout()
 *		ENDIF
 *	ENDIF
 *
 * CALLED FROM:	timeout() - upstream stream is block
 *		O_take_unsol_queue() - unsol were queued when breq came in
 *		O_data_bgrant() - done with last fragment
 *		O_data_breq() - breq is for buffer larger than our bufsize
 */
void
O_data_snf(de)

connect *de;		/* targeted data entry */
{
	mps_msgbuf_t *mbuf_p;	/* current message pointer */
	mb2socid_t dsoc;	/* requesting socket */
	struct dma_buf *dbp;		/* references data portion of message */
	mblk_t *mptr;		/* data message block */
	mps_msgbuf_t *mbuf_psav;	/* pointer to copy of current message */
	struct o_solicited_data
		*sd;		/* OTS data transfer buffer request message */
	struct dbreq_info *db;	/* saved breq info (used later as TLI hdr) */
	ulong dsize;		/* size of requested data buffer */
	ulong fsize;		/* size of fragment to be sent */
	ulong asize;		/* needed buffer size (passed to bufcall) */
	uchar tid;		/* transaction id */

	if ((mbuf_p = de->skt.data.breq) == NULL)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "O_data_snf: de=%x gone\n",de));
		return;
	}

	dsize = mps_msg_getbrlen(mbuf_p); 
	sd = (struct o_solicited_data *)&mbuf_p->mb_data[MPS_MG_BRUD];
	dsoc = de->skt.data.rem_data;
	tid = mps_msg_gettrnsid(mbuf_p);
	asize = 0;

	DEBUGP(DEB_FULL,(CE_CONT, "O_data_snf: de=%x,mbuf=%x,dsoc=%x,",de,mbuf_p,dsoc));
	DEBUGP(DEB_FULL,(CE_CONT, "dsize=%x,type=%x,tid=%x\n",dsize,sd->type,tid));

	mptr = NULL;
	mbuf_psav = NULL;

	if (  (canput(de->ep->rd_q))
	    &&(mbuf_psav = mps_get_msgbuf(KM_NOSLEEP))
	    &&(mptr = O_alloc_dsolbuf(de, mbuf_p, &dbp, &asize, TRUE))
	   )
	{
		DEBUGP(DEB_FULL,(CE_CONT, "  header=%x,data=%x\n",mptr,mptr->b_cont));
		if (fsize = mps_msg_getbrlen(mbuf_p))
		{	/* setup for fragmentation */
			dsize -= fsize;
			bcopy((char *)mbuf_p->mb_data,
				(char *)mbuf_psav->mb_data, MPS_MAXMSGSZ);
			de->skt.data.breq = mbuf_psav;
			if (sd->type == O_EOM_DATA)
				sd->type = O_DATA;
			else if (sd->type == O_EXPEDITED_DATA)
				sd->type = O_NOEOM_EXPEDITED_DATA;
		}
		else
		{
			mps_free_msgbuf(mbuf_psav);
			de->skt.data.breq = (mps_msgbuf_t *)NULL;
		}
		db = (struct dbreq_info *)mptr->b_rptr;
		db->type = sd->type;
		db->de = de;
		de->skt.data.rdtid = tid;
		mps_mk_bgrant(mbuf_p, dsoc, mbuf_p->mb_data[MPS_MG_RI], (int)dsize);
		mbuf_p->mb_bind = (ulong) mptr;
		if (mps_AMPreceive_frag((long)de->channel, mbuf_p, dsoc, tid, dbp))
		{
			cmn_err(CE_WARN, "SV-ots: O_data_snf mps_AMPreceive_frag error on port %x\n", de->port);
			freemsg(mptr);
			mps_free_msgbuf(mbuf_p);
			mps_free_dmabuf(dbp);
			iTLI_pferr(de->ep, M_NO_RESOURCES);
		}
	}
	else		/* no receive fragment this time */
	{
		if (mbuf_psav)
			mps_free_msgbuf(mbuf_psav);

		if (asize)
		{
			if (bufcall((uint)asize, BPRI_LO, O_data_snf, de) == 0)
			{
				cmn_err(CE_WARN,
					 "SV-ots: O_data_snf bufcall failed\n");
				timeout(O_data_snf, de, (int)DSBR_RETRY(de));
			}
		}
		else
			timeout(O_data_snf, de, (int)DSBR_RETRY(de));
	}
}


/* FUNCTION:			O_data_done()
 *
 * ABSTRACT:	Process transactionless solicited send completion
 *
 *	Call iTLI_data_req_complete to free messages.  If messages are waiting
 *	on the write queue (or about to be put back there), we need to
 *	reenable the queue.  However, this must be done outside the context
 *	of the interrupt handler because we may still be in the write service
 *	procedure (iTLIwsrv()).  So we call timeout().
 * 
 * CALLED FROM:	ots_data_intr()
 */
O_data_done(de, mbuf_p)

connect *de;		/* sending data connection entry */
mps_msgbuf_t *mbuf_p;	/* sent solicited message */
{
	mblk_t *mptr;			/* data message block */
	struct o_solicited_data *sd;	/* data transfer breq message */
	endpoint *ep;			/* sending endpoint */

	DEBUGP(DEB_FULL,(CE_CONT, "O_data_done: de=%x,mbuf_p=%x\n",de,mbuf_p));

	mptr = (mblk_t *)mbuf_p->mb_bind;
	sd = (struct o_solicited_data *)&mbuf_p->mb_data[MPS_MG_BRUD];
	de->state &= ~S_SOLICITED_SEND;
	if (ep = de->ep)
	{
		if (sd->type == O_EXPEDITED_DATA)
			iTLI_exdata_req_complete(ep, mptr, 0);
		else
			iTLI_data_req_complete(ep, mptr, 0);

		if (  (qsize(WR(ep->rd_q)))
		    ||(ep->nbr_datarq)
		   )
			timeout(qenable, WR(ep->rd_q), 1);	
	}
	else
		freemsg(mptr);
}


/* FUNCTION:			O_data_error()
 *
 * ABSTRACT:	Process transport error on data socket; retransmit if possible.
 *
 * This routine is normally called when:
 *
 *	1) A buffer request is rejected.  This is expected if we sent the
 *		original request transactionless.  Here, we retransmit RSVP
 *		to allow the receiver to fragment the request.
 *	2) After M_free_connect(de) cancels a transaction we initiated
 *		because the endpoint is going away.
 *
 * ALGORITHM:
 *
 *	IF (we sent the message)
 *		IF (message is failed buffer request and NOT rsvp)
 *			allocate tid
 *			reconstruct buffer request
 *			retransmit
 *		ELSE
 *			IF (transaction pending)
 *				cancel transaction
 *			ENDIF
 *			IF (endpoint active)
 *				call iTLI_data_req_complete with error
 *			ELSE
 *				free data connection entry
 *			ENDIF
 *			free message buffer
 *		ENDIF
 *	ELSE
 *		output error message
 *		free all resources
 *	ENDIF
 *
 * CALLED FROM:	ots_data_intr()
 */
O_data_error(mbuf_p, src, dest)

mps_msgbuf_t *mbuf_p;
mb2socid_t src;
mb2socid_t dest;
{
	connect *de = NULL;
	struct dma_buf *dbp;
	mblk_t *mptr = NULL;
	struct o_solicited_data sd;
	uchar tid;
	uchar type;

	tid = mps_msg_gettrnsid(mbuf_p);
	type = mbuf_p->mb_data[MPS_MG_MT];

	if (  (mps_mk_mb2soctohid(src) == ics_myslotid())
	    &&(de = M_find_d(mps_mk_mb2soctopid(src)))
	    &&(type == MPS_MG_BREQ)
	    &&(mptr = (mblk_t *) mbuf_p->mb_bind)
	   )
	{
		if (  (tid == 0)
		    &&(de->ep)
		    &&(tid = mps_get_tid((long)de->channel))
		    &&(dbp = mps_get_dmabuf(2,DMA_NOSLEEP))
		   )
		{		/* retransmit as transaction */
			sd.dlen = 0;
			sd.type = mbuf_p->mb_data[MPS_MG_BRUD + 15];
			mptr->b_next = (mblk_t *)sd.type;
			de->skt.data.xdtid = tid;
			mps_mk_sol(mbuf_p, dest, tid, (unsigned char *)&sd,
					sizeof(struct o_solicited_data));
			dbp->address = (ulong)kvtophys((caddr_t)mptr->b_rptr);
			dbp->count = msgdsize(mptr);
			(dbp->next_buf)->count = 0;
			(dbp->next_buf)->next_buf = NULL;
			if (O_tki_send(A_RSVP, de->channel, mbuf_p, dbp,
				(struct dma_buf *)NULL))
			{
				cmn_err(CE_WARN,"SV-ots: O_data_error A_RSVP failure\n");
				mps_free_msgbuf(mbuf_p);
				mps_free_tid((long)de->channel, tid);
				de->skt.data.xdtid = 0;
				mps_free_dmabuf(dbp);
			}
		}
		else		/* we sent a transaction which failed */
		{
			if (tid)
			 	mps_free_tid((long)de->channel, tid);
			if (tid == de->skt.data.xdtid)
				de->skt.data.xdtid = 0;

			if (de->ep)
				iTLI_data_req_complete(de->ep, mptr, ECOMM);
			else
			{
				freemsg(mptr);
				M_free_connect(de);
			}
			mps_free_msgbuf(mbuf_p);
		}
		return;
	}
	else if (de)
	{
		if (tid)
			mps_free_tid((long)de->channel, tid);
	}
	else if (mps_mk_mb2soctohid(dest) == ics_myslotid())
		 de = M_find_d(mps_mk_mb2soctopid(dest));
	
	cmn_err(CE_WARN,
		"SV-ots: transmission error -- src=%x,dest=%x,type=%x,rid=%x\n",
		src,dest,type,mps_msg_getrid(mbuf_p));

	if (mbuf_p->mb_bind)
		freemsg((mblk_t *)mbuf_p->mb_bind);

	if (de)
	{
		if (de->ep)
			iTLI_pferr(de->ep,  M_NO_RESOURCES);
		else
			M_free_connect(de);
	}
	mps_free_msgbuf(mbuf_p);
}


/* FUNCTION:			O_data_reply()
 *
 * ABSTRACT:	Process completed solicited transaction at data socket
 *
 *	The write queue needs to be enabled if messages to be sent are
 *	backed up behind this one.
 *
 * NOTE:	The initiating command type is stored in the b_next ptr
 *		of the sent message's mblock
 *
 * CALLED FROM:	ots_data_intr()
 */
O_data_reply(de, mbuf_p)

connect *de;		/* local data entry */
mps_msgbuf_t *mbuf_p;	/* current message pointer */
{
	mblk_t *data;			/* data message block */
	uchar type;			/* initial data transfer command */
	struct o_transaction_rsp *tr;	/* received response */
	endpoint *ep;			/* associated endpoint */

	de->skt.data.xdtid = 0;

	tr = (struct o_transaction_rsp *)&mbuf_p->mb_data[MPS_MG_UMUD];

	if ((data = (mblk_t *)mbuf_p->mb_bind) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_data_reply: no saved message block\n"));
		if (de->ep)
			iTLI_pferr(de->ep, M_NO_RESOURCES);
		else
			M_free_connect(de);
	}
	else if (  (mps_msg_gettransctl(mbuf_p) & MPS_MG_RSCN)
		 ||(tr->type != O_TRANSACTION_RSP)
		 ||(tr->errcode != E_OK)
		)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_data_reply: bad transaction\n"));
		if (de->ep)
			iTLI_pterr(de->ep, data, TSYSERR, ECOMM);
		else
			M_free_connect(de);
	}
	else if (ep = de->ep)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "O_data_reply: de=%x,qlen=%x\n",de,tr->qlen));
		de->skt.data.xcnt = tr->qlen;
		de->state &= ~S_SOLICITED_SEND;
		type = (uchar) data->b_next;
		data->b_next = (mblk_t *)NULL;
		switch (type)
		{
		case O_DATA:
		case O_EOM_DATA:
			iTLI_data_req_complete(ep, data, 0);
			break;
		case O_EXPEDITED_DATA:
			iTLI_exdata_req_complete(ep, data, 0);
			break;
		default:
			DEBUGP(DEB_ERROR,(CE_CONT, "O_data_reply: bad command %x\n", type));
			freemsg(data);
			break;
		}
		if (  (qsize(WR(ep->rd_q)))
		    ||(ep->nbr_datarq)
		   )
			timeout(qenable, WR(ep->rd_q), 1);	
	}
}


/* FUNCTION:			O_data_unsol()
 *
 * ABSTRACT:	process incoming unsol message on data socket
 *
 * ALGORITHM:
 *
 *	verify endpoint still exists and message is valid
 *	IF (upstream blocked OR unable to allocate STREAMS buffers)
 *		queue unsol message [O_put_unsol_queue()]
 *	ELSE
 *		copy message into allocated streams buffer
 *		send message to upstream modules
 *	ENDIF
 *
 * CALLED FROM:	ots_data_intr()
 */
O_data_unsol(de, mbuf_p, dsoc)

connect *de;		/* receiving data entry */
mps_msgbuf_t *mbuf_p;	/* current message pointer */
mb2socid_t dsoc;	/* requesting socket */
{
	mblk_t *header = NULL;		/* TLI header */
	mblk_t *data;			/* data message block */
	struct o_unsolicited_data *ud;	/* data transfer unsol message */
	uchar tid;			/* transaction id */

	tid = mps_msg_gettrnsid(mbuf_p);

	if (  (de->ep == NULL)		/* ensure ep still around */
	    ||(de->ep->discon_ind)	/* and no discon ind pending */
	   )
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_data_unsol: de %x ep gone\n", de));
		if (  (tid)
		    &&(mps_AMPcancel(de->channel, dsoc, tid))
		   )
		{
			DEBUGP(DEB_ERROR,
			(CE_CONT, "O_data_unsol: can't cancel tid %x\n",tid));
		}
		return;
	}

	ud = (struct o_unsolicited_data *)&mbuf_p->mb_data[MPS_MG_UMUD];
	de->skt.data.rdtid = tid;

	switch (ud->type)
	{
	 case O_DATA:
	 case O_EOM_DATA:
	 case O_EXPEDITED_DATA:
		break;
	 default:
		DEBUGP(DEB_ERROR,(CE_CONT, "O_data_unsol: bad command %x\n", ud->type));
		iTLI_pferr(de->ep, M_PROTOCOL_ERROR);
		return;
	}
	
	DEBUGP(DEB_FULL,(CE_CONT, "O_data_unsol: de=%x,mbuf_p=%x,dsoc=%x,tid=%x\n",
		de, mbuf_p, dsoc, de->skt.data.rdtid));

	if (  (canput(de->ep->rd_q) == FALSE)
	    ||(Q_EMPTY(de) == FALSE)
	    ||((header = allocb(sizeof(union qm), BPRI_MED)) == NULL)
	    ||((data = allocb((int)ud->dlen, BPRI_MED)) == NULL)
	   )
	{
		if (header)
			freemsg(header);
		if (de->skt.data.queue)
		{
			DEBUGP(DEB_FULL,(CE_CONT, "  queueing message\n"));
			O_put_unsol_queue(de, mbuf_p, dsoc);
		}
		else
			cmn_err(CE_WARN, "SV-ots: can't queue unsol from socket %x\n", dsoc);
	}
	else	/* buffers found for unsol, pass to TLI */
	{
		data->b_rptr = data->b_datap->db_base;
		data->b_wptr = data->b_rptr + ud->dlen;
		bcopy((char *)&mbuf_p->mb_data[MPS_MG_UMUD], (char *) data->b_rptr,
			(int)ud->dlen);
		linkb(header, data);
		switch (ud->type)
		{
			case O_DATA:
				iTLI_data_ind(de->ep, header, TRUE);
				break;
			case O_EOM_DATA:
				iTLI_data_ind(de->ep, header, FALSE);
				break;
			case O_EXPEDITED_DATA:
				iTLI_exdata_ind(de->ep, header, FALSE);
				break;
		}
		if (de->skt.data.rdtid)
			O_transaction_rsp(de, E_OK);
	}
}
