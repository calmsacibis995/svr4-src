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

#ident	"@(#)mbus:uts/i386/io/ots/otswri.c	1.3"

/*
** ABSTRACT:	SV-ots driver routines that transmit OTS msgs via TKI
**
**	One routine exists for each OTS request and response message supported
**	with the following exceptions:
**
**		O_accept_rsp() - accept connection response
**		O_accept_cancel_rsp - cancel accept response
**		O_datagram_open_rsp - accept datagrams response
**		O_datagram_cancel_rsp - cancel accept datagram response
**
**	These functions, normally implemented on network controllers, would
**	only be required if router support were added to this driver.
**
** MODIFICATIONS:
*/

#include "sys/ots.h"
#include "sys/otsprot.h"
#include "sys/otserror.h"
#include <sys/immu.h>

extern int ots_debug;			/* defined in ots.c */
extern ulong ots_stat[];

extern struct otscfg otscfg;		/* defined in Space.c */

extern ushort ots_version_mask;		/* defined in otsutils.c */

extern unchar ics_myslotid();			/* kernel Global variable */


/* FUNCTION:		O_accept()
 *
 * ABSTRACT:	Send OTS ACCEPT request
 *
 *	This function, required for network controller support, awaits
 *	implementation.
 *
 * RETURN CODE:	TSYSERR until implemented
 */
O_accept()
{
	cmn_err(CE_WARN, "SV-ots: O_accept called\n");
	return(TSYSERR);
}


/* FUNCTION:		O_accept_cancel()
 *
 * ABSTRACT:	Send OTS ACCEPT_CANCEL request
 *
 *	This function, required for network controller support, awaits
 *	implementation.
 *
 * RETURN CODE:	TSYSERR until implemented
 */
O_accept_cancel()
{
	cmn_err(CE_WARN, "SV-ots: O_accept_cancel called\n");
	return(TSYSERR);
}


/* FUNCTION:			O_breq_reject()
 *
 * ABSTRACT:	Reject buffer request
 *
 * CALLED FROM:	O_control_breq() and O_data_breq() (otsintr.c)
 */
O_breq_reject(channel, dsoc, mbuf_p)

ushort channel;
mb2socid_t dsoc;
mps_msgbuf_t *mbuf_p;
{
	mps_mk_breject(mbuf_p, dsoc, (uchar) mbuf_p->mb_data[MPS_MG_BRLI]);
	if (O_tki_send(A_UNSOL, (int)channel, mbuf_p, (struct dma_buf *)NULL,
			(struct dma_buf *)NULL))
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_breq_reject: failed for port %x\n",dsoc));
		mps_free_msgbuf(mbuf_p);
	}
}


/* FUNCTION:			O_connect()
 *
 * ABSTRACT:	Send OTS CONNECT request
 *
 *	compute RSVP response buffer size and allocate from STREAMS pool
 *	compute send buffer size and allocate from STREAMS pool
 *	allocate message buffer
 *	allocate transaction id
 *	allocate data pointer for RSVP response
 *	build O_CONNECT message on stack
 *	IF (any optional data associated with request)
 *		allocate STREAMS buffer and copy data there
 *		allocate and set data pointers for solicited send
 *		build a solicited RSVP out of the message buffer
 *	ELSE
 *		build an unsolicited RSVP out of the message buffer
 *	ENDIF
 *	send RSVP message
 *
 * NOTE:
 *
 *	Because we don't fragment on control sockets (except on datagrams)
 *	and we only post one STREAMS buffer for replies on control sockets,
 *	the variable length data associated with the connect response must fit
 *	in the largest STREAMS buffer configured (hopefully 4K).  This is why
 *	we limit the size on certain configuration parameters such as
 *	otscfg.cdata_size.
 *
 *	Unlike O_datagram(), little effort was expended in this routine
 *	and in O_connect_rsp() and O_disconnect() to ensure that component
 *	segments terminate on long-word boundaries (necessary because of
 *	problems with ADMA data chaining).  Currently, user data may be
 *	chained behind options.  This is ok because sizeof(opts) = 4.
 *	We will have to get more elaborate when support for network
 *	controllers is completed (i.e. variable length options, raddr and
 *	laddr fields are used).
 *
 * INPUTS:	options and length (via TLI message or endpoint defaults)
 *		destination address and length (via TLI message)
 *		local data port (indirect through endpoint (ep->de))
 *		local address and length (via endpoint)
 *		connect request user data (input parameter)
 *		queue length (driver default)
 *		solicited buffer size (driver default)
 *
 * OUTPUTS:	sent connect request and/or return code
 *
 * RETURN CODE:	zero	- Successful send
 *		non-zero- Error encountered
 *
 * CALLED FROM:	iMB2_conn_req()
 */
int
O_connect(ce, ep, dest_soc, conn, cdata)

connect *ce;			/* control entry */
endpoint *ep;			/* connecting endpoint */
mb2socid_t dest_soc;		/* destination socket */
struct T_conn_req *conn;	/* TLI connect request */
mblk_t *cdata;			/* connect request user data */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct dma_buf *dbp, *obp, *ibp;	/* data buffer ptrs */
	mblk_t *reply;			/* buffer for connect response */
	mblk_t *send = NULL;		/* send options and addrs */
	char *opt;			/* ptr to protocol options */
	struct o_connect req;		/* OTS connect request */
	ushort seg_count;		/* data segments in solicited send */
	ushort reply_length;		/* size of reply buffer */
	ushort send_length;		/* size of options and addrs */
	uchar tid;			/* MB-II transport transaction id */

	DEBUGP(DEB_CALL,(CE_CONT, "O_connect():\n"));
	DEBUGP(DEB_FULL,(CE_CONT, "  ce=%x,ep=%x,dest_soc=%x,conn=%x,cdata=%x\n",
				ce,ep,dest_soc,conn,cdata));

	reply_length = otscfg.opts_size + otscfg.cdata_size + (2 * otscfg.addr_size);
	if ((reply = allocb((int)reply_length, BPRI_HI)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_connect: reply allocb failed\n"));
		ots_stat[ST_ALFA]++;
		return(TSYSERR);
	}
	else if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_connect: msgbuf alloc failed\n"));
		freemsg(reply);
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else if ((tid = mps_get_tid((long)ce->channel)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_connect: tid alloc failed\n"));
		freemsg(reply);
		mps_free_msgbuf(mbuf_p);
		ots_stat[ST_TKITID]++;
		return(TSYSERR);
	}
	else if ((ibp = mps_get_dmabuf(2, DMA_NOSLEEP)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_connect: first dbuf alloc failed\n"));
		freemsg(reply);
		mps_free_msgbuf(mbuf_p);
		mps_free_tid((long)ce->channel, tid);
		ots_stat[ST_TKIDBLK]++;
		return(TSYSERR);
	}
	else
	{
		req.command = O_CONNECT;
		req.version = OTS_VERSION;
		req.qlen = otscfg.queue_len;
		req.buffer = otscfg.buffer_size;
		req.lport = ep->data->port;

		ibp->count = reply_length;
		ibp->address = kvtophys((caddr_t)reply->b_rptr);
		(ibp->next_buf)->count = 0;
		(ibp->next_buf)->next_buf = NULL;

		seg_count = 0;
		obp = (struct dma_buf *)NULL;
		if (req.options = conn->OPT_length)
			opt = (char *)conn + conn->OPT_offset;
		else if (ep->options != otscfg.vc_defaults)
		{
			req.options = sizeof(opts);
			opt = (char *)&ep->options;
		}
		req.laddr = ep->addr.length - sizeof(mb2socid_t);
		req.raddr = conn->DEST_length - sizeof(mb2socid_t);
		if (send_length = req.options + req.laddr + req.raddr)
		{
			if ((send = allocb((int)send_length, BPRI_HI)) == NULL)
			{
				DEBUGP(DEB_ERROR,(CE_CONT, "O_connect: send allocb failed\n"));
				freemsg(reply);
				mps_free_msgbuf(mbuf_p);
				mps_free_tid((long)ce->channel, tid);
				ots_stat[ST_ALFA]++;
				return(TSYSERR);
			}
			if (req.options)
			{
				bcopy(opt, (char *)send->b_rptr,
					(int)req.options);
				send->b_wptr = send->b_rptr + req.options;
			}
			if (req.laddr)
			{
				bcopy(ep->addr.data, (char *)send->b_wptr,
					(int)req.laddr);
				send->b_wptr += req.laddr;
			}
			if (req.raddr)
			{
				bcopy((char *)conn + conn->DEST_offset,
					(char *)send->b_wptr, (int)req.raddr);
				send->b_wptr += req.raddr;
			}
			seg_count++;
		}
		if (req.userdata = msgdsize(cdata))
			seg_count++;
		if (seg_count)	/* request must be sent solicited */
		{
			if ((obp = mps_get_dmabuf(((int)seg_count + 1), DMA_NOSLEEP)) == NULL)
			{
				DEBUGP(DEB_ERROR,(CE_CONT, "O_connect: datbuf alloc failed\n"));
				ots_stat[ST_TKIDBLK]++;
				freemsg(reply);
				if (send_length)
					freemsg(send);
				mps_free_msgbuf(mbuf_p);
				mps_free_tid((long)ce->channel, tid);
				mps_free_dmabuf(ibp);
				return(TSYSERR);
			}

			dbp = obp;
			if (send_length)
			{
				dbp->count = send_length;
				dbp->address = kvtophys((caddr_t)send->b_rptr);
				dbp = dbp->next_buf;
			}
			if (req.userdata)
			{
				dbp->count = req.userdata;
				dbp->address = kvtophys((caddr_t)cdata->b_rptr);
				dbp = dbp->next_buf;
			}
			dbp->count = 0;
			dbp->next_buf = NULL;
			mps_mk_sol(mbuf_p, dest_soc, tid, 
					(unsigned char *)&req, sizeof(req));
		}
		else		/* send request unsolicited */ 
			mps_mk_unsol(mbuf_p, dest_soc, tid,
					(unsigned char *)&req, sizeof(req));

		if (send)
			linkb(reply, send);
		if (cdata)
			linkb(reply, cdata);

		mbuf_p->mb_bind = (ulong)reply;
		ep->xctid = tid;

		DEBUGP(DEB_FULL,(CE_CONT, "  seg_cnt=%x, reply=%x,",seg_count,reply));
		DEBUGP(DEB_FULL,(CE_CONT, " transaction=%x\n",tid));

		if (O_tki_send(A_RSVP, ce->channel, mbuf_p, obp, ibp))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_connect: A_RSVP failed\n"));
			freemsg(reply);
			mps_free_msgbuf(mbuf_p);
			mps_free_tid((long)ce->channel, tid);
			ep->xctid = 0;
			mps_free_dmabuf(ibp);
			if (obp)
				mps_free_dmabuf(obp);
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:		O_connect_rsp()
 *
 * ABSTRACT:	Send OTS CONNECT_RSP response
 *
 *	allocate message buffer
 *	build O_CONNECT_RSP message on stack from input parameters
 *	IF (any optional data associated with response)
 *		allocate and set data pointers for solicited send
 *		build a solicited RSVP REPLY out of the message buffer
 *	ELSE
 *		build an unsolicited RSVP REPLY out of the message buffer
 *	ENDIF
 *	send RSVP REPLY message
 *
 *	Logic for keeping track of the remote transaction id and option
 *	data is done by the calling routine.  Because option data may
 *	exist in non-perminent STREAMS buffers, we allocate our own
 *	buffer to hold the data until the send completes.
 *
 * RETURN CODE:	TSYSERR if error in allocating resources or sending message
 *
 * CALLED FROM:	iMB2_conn_res() - to accept connection request
 *		iMB2_discon_req() - reject if endpoint being been deleted
 *		iMB2_conn_ind() - reject if bad OTS protocol negotiated
 *		iMB2_conn_ind_error() - reject if no resources
 */
int
O_connect_rsp(ce, dest_soc, udata, opt, opt_length, rport, lport, errcode, tid)

connect *ce;		/* local control entry */
mb2socid_t dest_soc;	/* destination control socket */
mblk_t *udata;		/* connect response (or disconnect request) user data */
char *opt;		/* connection options */
ushort opt_length;	/* connection options length */
ushort rport;		/* remote data port */
ushort lport;		/* local data port */
uchar errcode;		/* connect error code, if any */
uchar tid;		/* MB-II transport transaction id */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct dma_buf *dbp, *obp;		/* data buffer ptrs */
	mblk_t *odata = NULL;		/* mblock containing options */
	struct o_connect_rsp res;	/* OTS connect response */
	int seg_count;			/* data segments in solicited send */

	DEBUGP(DEB_CALL,(CE_CONT, "O_connect_rsp():\n"));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_connect_rsp: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else
	{
		res.command = O_CONNECT_RSP;
		res.errcode = errcode;
		res.qlen = otscfg.queue_len;
		res.buffer = otscfg.buffer_size;
		res.lport = lport;
		res.rport = rport;
		res.raddr = 0;	/* Router note: no way to get this from user */
		seg_count = 0;
		obp = (struct dma_buf *)NULL;
		if (res.options = opt_length)
		{
			if ((odata = allocb((int)opt_length, BPRI_HI)) == NULL)
			{
				DEBUGP(DEB_ERROR,(CE_CONT, "O_connect_rsp: allocb failed\n"));
				ots_stat[ST_ALFA]++;
				mps_free_msgbuf(mbuf_p);
				return(TSYSERR);
			}
			bcopy(opt, (char *)odata->b_rptr, (int)opt_length);
			seg_count++;
		}
		if (res.userdata = msgdsize(udata))
			seg_count++;
		if (seg_count)	/* response must be sent solicited */
		{
			if ((obp = mps_get_dmabuf((seg_count + 1),DMA_NOSLEEP)) == NULL)
			{
				DEBUGP(DEB_ERROR,(CE_CONT, "O_connect_rsp: datbuf alloc failed\n"));
				ots_stat[ST_TKIDBLK]++;
				mps_free_msgbuf(mbuf_p);
				return(TSYSERR);
			}
			dbp = obp;
			if (res.options)
			{
				dbp->count = res.options;
				dbp->address = kvtophys((caddr_t)odata->b_rptr);
				dbp = dbp->next_buf;
			}
			if (res.userdata)
			{
				dbp->count = msgdsize(udata);
				dbp->address = kvtophys((caddr_t)udata->b_rptr);
				dbp = dbp->next_buf;
			}
			dbp->count = 0;
			dbp->next_buf = NULL;
			mps_mk_solrply(mbuf_p, dest_soc, tid,
					(unsigned char *)&res, sizeof(res), 1);
		}
		else		/* send response unsolicited */ 
			mps_mk_unsolrply(mbuf_p, dest_soc, tid,
					(unsigned char *)&res, sizeof(res));

		if (odata)
		{
			mbuf_p->mb_bind = (ulong)odata;
			if (udata)
				linkb(odata, udata);
		}
		else
			mbuf_p->mb_bind = (ulong)udata;

		if (O_tki_send(A_REPLY, ce->channel, mbuf_p, obp,
				(struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_connect_rsp: A_REPLY failed\n"));
			mps_free_msgbuf(mbuf_p);
			if (obp)
				mps_free_dmabuf(obp);
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:			O_data()
 *
 * ABSTRACT:	Send OTS DATA, EOM_DATA, EXPEDITED_DATA request
 *
 *	IF (connection terminated)
 *		drain message
 *	ELSE IF (message send already in progress)
 *		put message back on queue and disable queue
 *	ELSE
 *		allocate message buffer
 *		IF (  (remote unsol queue almost full)
 *		    OR(send is expedited)
 *		    OR(message is larger than remote's bufsize)
 *		   )
 *			allocate transaction id
 *		ENDIF
 *		decrement remote queue length count
 *		IF (message size OK for unsolicited send)
 *			IF (sending transaction)
 *				build and send unsol RSVP, save tid in endpoint
 *			ELSE
 *				build and send unsolicited message
 *				free STREAMS buffer and service next send
 *			ENDIF
 *		ELSE [send solicited]
 *			allocate and set data pointers for solicited send
 *			IF (sending transaction)
 *				save tid in endpoint
 *				build and send solicited RSVP
 *			ELSE
 *				mark data connection for "sol-in-progress"
 *				build and send solicited
 *			ENDIF
 *		ENDIF
 *	ENDIF
 *	enable interrupts
 *
 *
 * RETURN CODE:	TSYSERR - unable to send message
 *
 * NOTE:
 *   1)	Client message fragmentation is NOT yet handled.  Driver-implemented
 *	fragmentation would be necessary if the remote correspondent has
 *	negotiated a buffer size less than 4096 bytes and does not support
 *	server message fragmentation.  One approach to fragmentation support
 *	would involve transferring only a portion of the STREAMS buffer.
 *	Duplicate the message block, update the duplicate's b_rptr field
 *	and put it back on the service queue.
 *
 *   2)	We store the send type in the mblock b_next pointer so when the
 *	transaction completes, we can call the appropriate completion handler.
 *
 *   3) Interrupts are disabled around updates to endpoint and connect fields
 *	which are also updated at interrupt time.
 */
O_data(ep, mptr, more, cmd)

endpoint *ep;			/* endpoint structure */
mblk_t *mptr;			/* message block referencing data to be sent */
int more;			/* MORE flag */
int cmd;			/* EXPEDITED or normal DATA send */
{
	connect *de;			/* data connection entry */
	struct dma_buf *dbp;			/* solicited data buffer descriptor */
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	uchar tid = 0;			/* MB-II transport transaction id */
	int size;			/* size of data message */
	int pri;			/* saved interrupt priority */
	union				/* send structures */
	{
		struct o_solicited_data sd;
		struct o_unsolicited_data ud;
	} buf;

	DEBUGP(DEB_CALL,(CE_CONT, "O_data():\n"));
	DEBUGP(DEB_FULL,(CE_CONT, "  ep=%x,mptr=%x,more=%x,cmd=%x\n",ep,mptr,more,cmd));

	if ((de = ep->data) == NULL)
	{
		/*
		 * We must have received a disconnect; ignore the message
		 *  by treating it as if the send completed
		 */
		freemsg(mptr);
		pri = SPL();
		ep->nbr_datarq--;
		splx(pri);
		return(0);
	}
	else if (  (de->skt.data.xdtid)
		 ||(de->state & S_SOLICITED_SEND)
		)
		return(M_FLOW_CONTROL);

	else if (cmd == T_DATA_REQ)
		cmd = (more) ? O_DATA : O_EOM_DATA;
	else
		cmd = O_EXPEDITED_DATA;

	size = msgdsize(mptr);
	
	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		ots_stat[ST_TKIMBLK]++;
		DEBUGP(DEB_ERROR,(CE_CONT, "O_data: msgbuf alloc (de=%x) failed\n", de));
		return(TSYSERR);
	}
	else if (  (de->skt.data.xcnt <= 1)
		 ||(cmd == O_EXPEDITED_DATA)
		 ||(size > de->skt.data.bufsize)
		)
	{
		if ((tid = mps_get_tid((long)de->channel)) == 0)
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_data: tid alloc failed\n"));
			mps_free_msgbuf(mbuf_p);
			ots_stat[ST_TKITID]++;
			return(TSYSERR);
		}
		de->skt.data.xdtid = tid;
		mptr->b_next = (mblk_t *)cmd;		/* Note 2 */
	}

	DEBUGP(DEB_FULL,(CE_CONT, "  sending %x bytes",size));
	DEBUGP(DEB_FULL,(CE_CONT, " to socket %x out port %x, tid=%x\n",
			de->skt.data.rem_data,de->port,tid));

	if (size <= USEND_SIZE)
	{
		bcopy((char *)mptr->b_rptr, buf.ud.data, size);
		buf.ud.dlen = size;
		buf.ud.type = cmd;
		mps_mk_unsol(mbuf_p, de->skt.data.rem_data, tid,
		     (unsigned char *)&buf, sizeof(struct o_unsolicited_data));
		if (tid)
		{
			mbuf_p->mb_bind = (ulong)mptr;
			if (O_tki_send(A_RSVP, de->channel, mbuf_p,
					(struct dma_buf *)NULL, (struct dma_buf *)NULL))
			{
				cmn_err(CE_WARN, "SV-ots: O_data A_RSVP failure\n");
				mps_free_tid((long)de->channel, tid);
				de->skt.data.xdtid = 0;
				mps_free_msgbuf(mbuf_p);
				return(TSYSERR);
			}
			pri = SPL();
			de->skt.data.xcnt--;
			splx(pri);
		}
		else
		{
			mbuf_p->mb_bind = NULL;
			if (O_tki_send(A_UNSOL, de->channel, mbuf_p,
					(struct dma_buf *)NULL, (struct dma_buf *)NULL))
			{
				cmn_err(CE_WARN, "SV-ots: O_data A_UNSOL failure\n");
				mps_free_msgbuf(mbuf_p);
				return(TSYSERR);
			}
			pri = SPL();
			de->skt.data.xcnt--;
			ep->nbr_datarq--;
			splx(pri);
			freemsg(mptr);
		}
		return(0);
	}
	else if (dbp = mps_get_dmabuf(2, DMA_NOSLEEP))
	{
		buf.sd.dlen = 0;
		buf.sd.type = cmd;
		mps_mk_sol(mbuf_p, de->skt.data.rem_data, tid,
			(unsigned char *)&buf, sizeof(struct o_solicited_data));
		mbuf_p->mb_bind = (ulong)mptr;
		dbp->address = (ulong)kvtophys((caddr_t)mptr->b_rptr);
		dbp->count = size;
		(dbp->next_buf)->count = 0;
		(dbp->next_buf)->next_buf = NULL;
		if (tid)
		{
			if (O_tki_send(A_RSVP, de->channel, mbuf_p, dbp,
					(struct dma_buf *)NULL))
			{
				cmn_err(CE_WARN, "SV-ots: O_data A_RSVP failure\n");
				mps_free_msgbuf(mbuf_p);
				mps_free_tid((long)de->channel, tid);
				de->skt.data.xdtid = 0;
				mps_free_dmabuf(dbp);
				return(TSYSERR);
			}
			pri = SPL();
			de->skt.data.xcnt--;
			splx(pri);
		}
		else
		{
			pri = SPL();
			if (O_tki_send(A_SOL, de->channel, mbuf_p, dbp,
					(struct dma_buf *)NULL))
			{
				splx(pri);
				cmn_err(CE_WARN, "SV-ots: O_data A_SOL failure\n");
				mps_free_msgbuf(mbuf_p);
				mps_free_dmabuf(dbp);
				return(TSYSERR);
			}
			de->state |= S_SOLICITED_SEND;
			de->skt.data.xcnt--;
			splx(pri);
		}
		return(0);
	}
	else
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_data: datbuf alloc (de=%x) failed\n", de));
		ots_stat[ST_TKIDBLK]++;
		return(TSYSERR);
	}
}


/* FUNCTION:		O_datagram()
 *
 * ABSTRACT:	Send OTS DATAGRAM request
 *
 *	This routine assumes data references the a STREAMS buffer with
 *	non-zero length	and contains the datagram.  We need to allocate an
 *	addtional STREAMS buffer to hold the laddr, raddr and options fields.
 *	The code gets real messy below because we need to ensure that component
 *	segments in a data chain do not end on an odd-word boundary.  This is
 *	necessary because of a bug in ADMA.  We collapse the STREAMS buffers
 *	via pullupmsg() so that all but the last buffer in the chain are full
 *	(and thus are word-aligned at their rears).
 *
 * RETURN CODE: 0 - datagram sent
 *		TSYSERR - error encountered
 *
 * CALLED FROM:	iMB2_unitdata_req()
 */
O_datagram(ce, ep, dest_soc, udhdr, data)

connect *ce;			/* control entry */
endpoint *ep;			/* connecting endpoint */
mb2socid_t dest_soc;		/* destination socket */
struct T_unitdata_req *udhdr;	/* TLI datagram request */
mblk_t *data;			/* datagram */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct dma_buf *dbp, *obp;		/* data buffer ptrs */
	mblk_t *info = NULL;		/* buffer for options & addrs */
	mblk_t *mptr;			/* used to parse mblock list */
	struct o_datagram req;		/* OTS datagram request */
	uchar tid;			/* MB-II transport transaction id */
	int seg_count;			/* data segments in solicited send */
	int info_length;		/* size of info STREAMS buffer */
	int msg_length;			/* total size of O_DATAGRAM message */

	DEBUGP(DEB_CALL,(CE_CONT, "O_datagram():\n"));
	DEBUGP(DEB_FULL,(CE_CONT, "  ce=%x,ep=%x,dest_soc=%x,data=%x\n",
				ce,ep,dest_soc,udhdr,data));

	req.options = udhdr->OPT_length;
	req.laddr = ep->addr.length - sizeof(mb2socid_t);
	req.raddr = udhdr->DEST_length - sizeof(mb2socid_t);
	info_length = req.options + req.laddr + req.raddr;

	if (ep->xctid)
		return(M_FLOW_CONTROL);
	else if (  (info_length)
		 &&((info = allocb(info_length, BPRI_HI)) == NULL)
		)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_datagram: allocb failed\n"));
		return(TSYSERR);
	}
	else if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_datagram: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else if ((tid = mps_get_tid((long)ce->channel)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_datagram: tid alloc failed\n"));
		mps_free_msgbuf(mbuf_p);
		ots_stat[ST_TKITID]++;
		return(TSYSERR);
	}
	else
	{
		req.command = O_DATAGRAM;
		req.version = OTS_VERSION;
		req.userdata = msgdsize(data);

		if (info_length)
		{
			linkb(data, info);
			info->b_wptr = info->b_rptr;
			if (req.options)
			{
				bcopy(((char *)udhdr + udhdr->OPT_offset),
				      (char *)info->b_wptr, (int)req.options);
				info->b_wptr += req.options;
			}
			if (req.laddr)
			{
				bcopy(((char *)ep->addr.data +
					sizeof(mb2socid_t)),
				      (char *)info->b_wptr, (int)req.laddr);
				info->b_wptr += req.laddr;
			}
			if (req.raddr)
			{
				bcopy(((char *)udhdr + udhdr->DEST_offset +
					sizeof(mb2socid_t)),
					(char *)info->b_wptr, (int)req.raddr);
				info->b_wptr += req.raddr;
			}
		}
		
		if (  (info_length)	/* KLUDGE to get around ADMA bug */
		    &&(req.userdata & 0x3)
		   )
		{
			if ((msg_length = info_length+req.userdata) > SBLK4096)
			{
				if ((mptr = dupb(info)) == NULL)
				{
					DEBUGP(DEB_ERROR,(CE_CONT, "O_datagram: dubp failed\n"));
					mps_free_msgbuf(mbuf_p);
					mps_free_tid((long)ce->channel, tid);
					return(TSYSERR);
				}
				info->b_wptr -= (msg_length - SBLK4096);
				mptr->b_rptr = info->b_wptr;
				linkb(data, mptr);
				
			}
			if (pullupmsg(data, msg_length) == 0)
			{
				DEBUGP(DEB_ERROR,(CE_CONT, "O_datagram: pullupmsg failed\n"));
				mps_free_msgbuf(mbuf_p);
				mps_free_tid((long)ce->channel, tid);
				return(TSYSERR);
			}
		}
		/*
		 * count segments
		 */
		mptr = data;
		seg_count = 0;
		do
		{
			seg_count++;
		}
		while (mptr = mptr->b_cont) ;
		/*
		 * allocate and initialize data descriptors
		 */
		if ((obp = mps_get_dmabuf((seg_count + 1),DMA_NOSLEEP)) == NULL)
		{
			ots_stat[ST_TKIDBLK]++;
			DEBUGP(DEB_ERROR,(CE_CONT, "O_datagram: datbuf alloc failed\n"));
			mps_free_msgbuf(mbuf_p);
			mps_free_tid((long)ce->channel, tid);
			return(TSYSERR);
		}
		dbp = obp;
		mptr = data;
		do
		{
			dbp->count = mptr->b_wptr - mptr->b_rptr;
			dbp->address = kvtophys((caddr_t) mptr->b_rptr);
			dbp = dbp->next_buf;
		}
		while (mptr = mptr->b_cont) ;
		dbp->count = 0;
		dbp->next_buf = NULL;

		mps_mk_sol(mbuf_p, dest_soc, tid,
				(unsigned char *)&req, sizeof(req));

		mbuf_p->mb_bind = (ulong)data;
		ep->xctid = tid;

		DEBUGP(DEB_FULL,(CE_CONT, " transaction=%x\n",tid));
		DEBUGP(DEB_FULL,(CE_CONT, "  seg_cnt=%x,",seg_count));

		if (O_tki_send(A_RSVP, ce->channel, mbuf_p, obp,
				(struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_datagram: A_RSVP failed\n"));
			mps_free_msgbuf(mbuf_p);
			mps_free_tid((long)ce->channel, tid);
			ep->xctid = 0;
			mps_free_dmabuf(obp);
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:		O_datagram_open()
 *
 * ABSTRACT:	Send OTS DATAGRAM_OPEN request
 *
 *	This function, required for network controller support, awaits
 *	implementation.
 *
 * RETURN CODE:	TSYSERR until implemented
 */
O_datagram_open()
{
	cmn_err(CE_WARN, "SV-ots: O_datagram_open() called\n");
	return(TSYSERR);
}


/* FUNCTION:		O_datagram_cancel()
 *
 * ABSTRACT:	Send OTS DATAGRAM_CANCEL request
 *
 *	This function, required for network controller support, awaits
 *	implementation.
 *
 * RETURN CODE:	TSYSERR until implemented
 */
O_datagram_cancel()
{
	cmn_err(CE_WARN, "SV-ots: O_datagram_cancel() called\n");
	return(TSYSERR);
}


/* FUNCTION:		O_datagram_rsp()
 *
 * ABSTRACT:	Send OTS DATAGRAM response
 *
 * RETURN CODE: 0 - message sent
 *		TSYSERR - error encountered
 *
 * CALLED FROM:	iMB2_unitdata_ind() (iMB2rd.c)
 */
O_datagram_rsp(ce, dest_soc, errcode, tid)

connect *ce;			/* control entry */
mb2socid_t dest_soc;		/* destination socket */
uchar errcode;			/* disconnect error code, if any */
uchar tid;			/* MB-II transport transaction id */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct o_datagram_rsp res;	/* OTS datagram response */

	DEBUGP(DEB_CALL,(CE_CONT, "O_datagram_rsp():\n"));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_datagram_rsp: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else
	{
		res.command = O_DATAGRAM_RSP;
		res.errcode = errcode;
		mps_mk_unsolrply(mbuf_p, dest_soc, tid,
					(unsigned char *)&res, sizeof(res));
		if (O_tki_send(A_REPLY, ce->channel, mbuf_p, (struct dma_buf *)NULL,
				(struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_datagram_rsp: A_REPLY failed\n"));
			mps_free_msgbuf(mbuf_p);
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:		O_disconnect()
 *
 * ABSTRACT:	Send OTS DISCONNECT request
 *
 *	allocate message buffer
 *	allocate transaction id
 *	build O_DISCONNECT message on stack
 *	IF (any optional data associated with request)
 *		allocate and set data pointers for solicited send
 *		build a solicited RSVP out of the message buffer
 *	ELSE
 *		build an unsolicited RSVP out of the message buffer
 *	ENDIF
 *	IF (previous transaction outstanding, see NOTE)
 *		cancel previous transaction
 *	ENDIF
 *	send RSVP message
 *
 * NOTE:    1)	The endpoint may already have an outstanding transaction, (e.g.
 *		a connect or orderly release request) which this request
 *		overrides.  The old transaction must be canceled.
 *
 * RETURN CODE: 0 - message sent
 *		TSYSERR - error encountered
 *
 * CALLED FROM:	iMB2_discon_req()
 */
O_disconnect(ce, de, reason, mptr, ddata)

connect *ce;		/* control entry */
connect *de;		/* data entry */
int reason;		/* disconnect reason */
mblk_t *mptr;		/* TLI message header - used for TPI ack */
mblk_t *ddata;		/* disconnect user data, if any */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct dma_buf *dbp, *obp;		/* data buffer ptrs */
	struct o_disconnect req;	/* OTS disconnect request */
	mb2socid_t dest_soc;		/* destination socket */
	uchar tid;			/* MB-II transport transaction id */
	int seg_count;			/* data segments in solicited send */

	DEBUGP(DEB_CALL,(CE_CONT, "O_disconnect():\n"));
	DEBUGP(DEB_FULL,(CE_CONT, "  ce=%x,de=%x,reason=%x,ddata=%x\n",
				ce,de,reason,ddata));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_disconnect: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else if ((tid = mps_get_tid((long)ce->channel)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_disconnect: tid alloc failed\n"));
		mps_free_msgbuf(mbuf_p);
		ots_stat[ST_TKITID]++;
		return(TSYSERR);
	}
	else
	{
		req.command = O_DISCONNECT;
		req.reason = reason;
		req.lport = de->port;
		req.rport = mps_mk_mb2soctopid(de->skt.data.rem_data);
		dest_soc = de->skt.data.rem_cntrl;

		seg_count = 0;
		obp = (struct dma_buf *)NULL;
		if (req.userdata = msgdsize(ddata))
		{
			linkb(mptr, ddata);
			seg_count++;
		}
		if (seg_count)	/* request must be sent solicited */
		{
			if ((obp = mps_get_dmabuf((seg_count + 1), DMA_NOSLEEP)) == NULL)
			{
				DEBUGP(DEB_ERROR,(CE_CONT, "O_disconnect: second dbuf alloc failed\n"));
				ots_stat[ST_TKIDBLK]++;
				mps_free_msgbuf(mbuf_p);
				mps_free_tid((long)ce->channel, tid);
				return(TSYSERR);
			}

			dbp = obp;
			if (req.userdata)
			{
				dbp->count = msgdsize(ddata);
				dbp->address = kvtophys((caddr_t)ddata->b_rptr);
				dbp = dbp->next_buf;
			}
			dbp->count = 0;
			dbp->next_buf = NULL;
			mps_mk_sol(mbuf_p, dest_soc, tid,
					(unsigned char *)&req, sizeof(req));
		}
		else		/* send request unsolicited */ 
			mps_mk_unsol(mbuf_p, dest_soc, tid,
					(unsigned char *)&req, sizeof(req));

		mbuf_p->mb_bind = (ulong)mptr;

		if (  (de->ep->xctid)			/* NOTE 1 */
		    &&(mps_AMPcancel(ce->channel,
			mps_mk_mb2socid(ics_myslotid(), de->ep->control->port),
			de->ep->xctid))
		   )
			cmn_err(CE_WARN,"SV-ots: can't cancel xctid");
		de->ep->xctid = tid;

		DEBUGP(DEB_FULL,(CE_CONT, "  seg_cnt=%x, tid = %x\n",seg_count,tid));

		if (O_tki_send(A_RSVP, ce->channel, mbuf_p, obp,
				(struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_disconnect: A_RSVP failed\n"));
			mps_free_msgbuf(mbuf_p);
			mps_free_tid((long)ce->channel, tid);
			de->ep->xctid = 0;
			if (obp)
				mps_free_dmabuf(obp);
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:			O_disconnect_rsp()
 *
 * ABSTRACT:	Send OTS O_DISCONNECT_RSP message
 *
 * RETURN CODE: 0 - message sent
 *		TSYSERR - error encountered
 *
 * CALLED FROM:	iMB2_discon_ind()
 */
O_disconnect_rsp(ce, dest_soc, lport, rport, errcode, tid)

connect *ce;		/* control entry */
mb2socid_t dest_soc;	/* destination socket */
ushort rport;		/* remote data port */
ushort lport;		/* local data port */
uchar errcode;		/* disconnect error code, if any */
uchar tid;		/* MB-II transport transaction id */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct o_disconnect_rsp res;	/* OTS disconnect response */

	DEBUGP(DEB_CALL,(CE_CONT, "O_disconnect_rsp():\n"));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_disconnect_rsp: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else
	{
		res.command = O_DISCONNECT_RSP;
		res.errcode = errcode;
		res.lport = lport;
		res.rport = rport;
		mps_mk_unsolrply(mbuf_p, dest_soc, tid,
					(unsigned char *)&res, sizeof(res));
		if (O_tki_send(A_REPLY, ce->channel, mbuf_p,
				(struct dma_buf *)NULL, (struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_disconnect_rsp: A_REPLY failed\n"));
			mps_free_msgbuf(mbuf_p);
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:			O_get_info()
 *
 * ABSTRACT:	Send OTS O_GET_INFO request
 *
 * RETURN CODE: 0 - message sent
 *		TSYSERR - error encountered
 *
 * CALLED BY:	TBD
 */
O_get_info(ce, ep, dest_soc)

connect *ce;			/* local control connect entry */
endpoint *ep;			/* local endpoint */
mb2socid_t dest_soc;		/* destination socket */
{
	mps_msgbuf_t *mbuf_p;	/* MB-II transport message */
	mblk_t *reply;		/* buffer for get_info response */
	struct dma_buf *ibp;		/* data buffer ptr */
	struct o_get_info req;	/* OTS get_info request */
	ushort reply_length;	/* size of reply buffer */
	uchar tid;		/* MB-II transport transaction id */

	DEBUGP(DEB_CALL,(CE_CONT, "O_get_info(): ce=%x,ep=%x,soc=%x\n",ce,ep,dest_soc));

	reply_length = otscfg.opts_size + sizeof(struct get_info_data);

	if ((reply = allocb((int)reply_length, BPRI_HI)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_get_info: reply allocb failed\n"));
		ots_stat[ST_ALFA]++;
		return(TSYSERR);
	}
	else if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_get_info: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else if ((tid = mps_get_tid((long)ce->channel)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_get_info: tid alloc failed\n"));
		mps_free_msgbuf(mbuf_p);
		ots_stat[ST_TKITID]++;
		return(TSYSERR);
	}
	else if ((ibp = mps_get_dmabuf(2,DMA_NOSLEEP)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_get_info: ibp alloc failed\n"));
		freemsg(reply);
		mps_free_msgbuf(mbuf_p);
		mps_free_tid((long)ce->channel, tid);
		ots_stat[ST_TKIDBLK]++;
		return(TSYSERR);
	}
	else
	{
		req.command = O_GET_INFO;
		req.version = OTS_VERSION;

		ibp->count = reply_length;
		ibp->address = kvtophys((caddr_t)reply->b_rptr);
		(ibp->next_buf)->count = 0;
		(ibp->next_buf)->next_buf = NULL;

		mbuf_p->mb_bind = (ulong)reply;
		ep->xctid = tid;
		DEBUGP(DEB_FULL,(CE_CONT, " tid = %x\n",tid));

		/* send request unsolicited */ 
		mps_mk_unsol(mbuf_p, dest_soc, tid,
					(unsigned char *)&req, sizeof(req));

		if (O_tki_send(A_RSVP, ce->channel, mbuf_p, (struct dma_buf *)NULL,
				ibp))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_get_info: A_RSVP failed\n"));
			freemsg(reply);
			mps_free_msgbuf(mbuf_p);
			mps_free_tid((long)ce->channel, tid);
			mps_free_dmabuf(ibp);
			ep->xctid = 0;
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:			O_get_info_rsp()
 *
 * ABSTRACT:	Send OTS O_GET_INFO_RSP reply message
 *
 * NOTE: We don't send the options field (this info is in services).
 *
 * RETURN CODE: 0 - message sent
 *		TSYSERR - error encountered
 *
 * CALLED BY:	O_control_unsol()
 */
O_get_info_rsp(ce, dest_soc, errcode, tid)

connect *ce;		/* local control connect entry */
mb2socid_t dest_soc;	/* destination socket */
uchar errcode;		/* negotiate error code, if any */
uchar tid;		/* MB-II transport transaction id */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct dma_buf *obp = NULL;		/* data buffer ptrs */
	mblk_t *reply = NULL;		/* buffer for get_info response */
	struct get_info_data *data;	/* ptr to info data */
	struct o_get_info_rsp res;	/* OTS negotiate version response */

	DEBUGP(DEB_CALL,(CE_CONT, "O_get_info_rsp():\n"));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_get_info_rsp: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else
	{
		res.command = O_GET_INFO_RSP;
		if (  ((res.errcode = errcode) == E_OK)
		    &&(reply = allocb(sizeof(*data), BPRI_HI))
		    &&(obp = mps_get_dmabuf(2, DMA_NOSLEEP))
		   )
		{
			res.data = sizeof(*data);
			res.options = 0;
			data = (struct get_info_data *)reply->b_rptr;
			data->netaddr = otscfg.addr_size;
			data->options = otscfg.opts_size;
			data->tsdu_size = otscfg.tsdu_size;
			data->etsdu_size = otscfg.etsdu_size;
			data->connect = otscfg.cdata_size;
			data->discon = otscfg.ddata_size;
			data->datagram = otscfg.datagram_size;
			data->service = otscfg.addr_size;
			bcopy(TP_TYPE, data->tp_type, TP_TYPE_SIZE);

			obp->count = res.data;
			obp->address = kvtophys((caddr_t)reply->b_rptr);
			(obp->next_buf)->count = 0;
			(obp->next_buf)->next_buf = NULL;

			mbuf_p->mb_bind = (ulong)reply;

			mps_mk_solrply(mbuf_p, dest_soc, tid,
					(unsigned char *)&res, sizeof(res), 1);
		}
		else		/* send response unsolicited */ 
		{
			if (res.command == E_OK)
			{
				res.command = E_NO_RESOURCES;
				if (reply)
				{
					freemsg(reply);
					reply = (mblk_t *)NULL;
					ots_stat[ST_TKIDBLK]++;
				}
				else
					ots_stat[ST_ALFA]++;
			}
			res.data = 0;
			res.options = 0;
			mps_mk_unsolrply(mbuf_p, dest_soc, tid,
					(unsigned char *)&res, sizeof(res));
		}

		if (O_tki_send(A_REPLY, ce->channel, mbuf_p, obp,
				(struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_get_info_rsp: A_REPLY failed\n"));
			mps_free_msgbuf(mbuf_p);
			if (obp)
				mps_free_dmabuf(obp);
			if (reply)
				freemsg(reply);
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:			O_negotiate_ver()
 *
 * ABSTRACT:	Send OTS O_NEGOTIATE_VER request
 *
 * RETURN CODE: 0 - message sent
 *		TSYSERR - error encountered
 *
 * CALLED BY:	TBD
 */
O_negotiate_ver(ce, ep, dest_soc)

connect *ce;			/* local control connect entry */
endpoint *ep;			/* local endpoint */
mb2socid_t dest_soc;		/* destination socket */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	uchar tid;			/* MB-II transport transaction id */
	struct o_negotiate_ver req;	/* OTS negotiate version request */

	DEBUGP(DEB_CALL,(CE_CONT, "O_negotiate_ver(): ce=%x,dest_soc=%x\n",ce,dest_soc));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_negotiate_ver: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else if ((tid = mps_get_tid((long)ce->channel)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_negotiate_ver: tid alloc failed\n"));
		mps_free_msgbuf(mbuf_p);
		ots_stat[ST_TKITID]++;
		return(TSYSERR);
	}
	else
	{
		req.command = O_NEGOTIATE_VER;
		req.list = ots_version_mask;

		mbuf_p->mb_bind = 0;
		ep->xctid = tid;

		/* send request unsolicited */ 
		mps_mk_unsol(mbuf_p, dest_soc, tid,
					(unsigned char *)&req, sizeof(req));

		DEBUGP(DEB_FULL,(CE_CONT, " tid = %x\n",tid));

		if (O_tki_send(A_RSVP, ce->channel, mbuf_p,
				(struct dma_buf *)NULL, (struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_negotiate_ver: A_RSVP failed\n"));
			mps_free_msgbuf(mbuf_p);
			mps_free_tid((long)ce->channel, tid);
			ep->xctid = 0;
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:			O_negotiate_ver_rsp()
 *
 * ABSTRACT:	Send OTS O_NEGOTIATE_VER_RSP reply message
 *
 * RETURN CODE: 0 - message sent
 *		TSYSERR - error encountered
 *
 * CALLED BY:	iMB2_negotiate_version() (iMB2rd.c)
 */
O_negotiate_ver_rsp(ce, dest_soc, errcode, version, tid)

connect *ce;		/* local control connect entry */
mb2socid_t dest_soc;	/* destination socket */
uchar errcode;		/* negotiate error code, if any */
uchar version;		/* negotiated version if errcode = 0 */
uchar tid;		/* MB-II transport transaction id */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct o_negotiate_ver_rsp res;	/* OTS negotiate version response */

	DEBUGP(DEB_CALL,(CE_CONT, "O_negotiate_ver_rsp():\n"));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_negotiate_ver_rsp: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else
	{
		res.command = O_NEGOTIATE_VER_RSP;
		res.errcode = errcode;
		res.version = version;

		/* send reply unsolicited */ 
		mps_mk_unsolrply(mbuf_p, dest_soc, tid,
					(unsigned char *)&res, sizeof(res));
		if (O_tki_send(A_REPLY, ce->channel, mbuf_p,
				(struct dma_buf *)NULL, (struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_negotiate_ver_rsp: A_REPLY failed\n"));
			mps_free_msgbuf(mbuf_p);
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:			O_ord_release()
 * 
 * ABSTRACT:	Send OTS O_ORD_RELEASE request
 *
 * RETURN CODE: 0 - message sent
 *		TSYSERR - error encountered
 *
 * CALLED FROM:	iMB2_ordrel_req() - to initiate orderly release
 */
O_ord_release(ce, de, mptr)

connect *ce;			/* local control connect entry */
connect *de;			/* local data connect entry */
mblk_t *mptr;			/* save this for TLI response */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct o_release req;		/* OTS disconnect request */
	mb2socid_t dest_soc;		/* destination socket */
	uchar tid;			/* MB-II transport transaction id */

	DEBUGP(DEB_CALL,(CE_CONT, "O_ord_release():\n"));
	DEBUGP(DEB_FULL,(CE_CONT, "ce=%x,de=%x", ce, de));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_ord_release: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else if ((tid = mps_get_tid((long)ce->channel)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_ord_release: tid alloc failed\n"));
		mps_free_msgbuf(mbuf_p);
		ots_stat[ST_TKITID]++;
		return(TSYSERR);
	}
	else
	{
		req.command = O_ORD_RELEASE;
		req.reason = M_NO_REASON;
		req.lport = de->port;
		req.rport = mps_mk_mb2soctopid(de->skt.data.rem_data);
		dest_soc = de->skt.data.rem_cntrl;

		mbuf_p->mb_bind = (ulong)mptr;
		de->ep->xctid = tid;

		/* send request unsolicited */ 
		mps_mk_unsol(mbuf_p, dest_soc, tid,
					(unsigned char *)&req, sizeof(req));

		DEBUGP(DEB_FULL,(CE_CONT, " tid = %x\n",tid));

		if (O_tki_send(A_RSVP, ce->channel, mbuf_p,
				(struct dma_buf *)NULL, (struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_ord_release: A_RSVP failed\n"));
			mps_free_msgbuf(mbuf_p);
			mps_free_tid((long)ce->channel, tid);
			de->ep->xctid = 0;
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:			O_ord_release_rsp()
 * 
 * ABSTRACT:	Send OTS O_ORD_RELEASE_RSP response message
 *
 * RETURN CODE: 0 - message sent
 *		TSYSERR - error encountered
 *
 * CALLED FROM:	iMB2_ordrel_req() - normal orderly release response
 *		iMB2_discon_req() - premature abort during orderly release
 */
O_ord_release_rsp(ce, lport, rport, dest_soc, errcode, tid)

connect *ce;		/* local control connect entry */
ushort lport;		/* local port */
ushort rport;		/* remote port */
mb2socid_t dest_soc;	/* destination socket */
uchar errcode;		/* connect error code, if any */
uchar tid;		/* MB-II transport transaction id */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct o_release_rsp res;	/* OTS orderly release response */

	DEBUGP(DEB_CALL,(CE_CONT, "O_ord_release_rsp():\n"));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_ord_release_rsp: msgbuf alloc failed\n"));
		ots_stat[ST_TKIMBLK]++;
		return(TSYSERR);
	}
	else
	{
		res.command = O_ORD_RELEASE_RSP;
		res.errcode = errcode;
		res.lport = lport;
		res.rport = rport;

		/* send reply unsolicited */ 
		mps_mk_unsolrply(mbuf_p, dest_soc, tid,
					(unsigned char *)&res, sizeof(res));
		if (O_tki_send(A_REPLY, ce->channel, mbuf_p,
				(struct dma_buf *)NULL, (struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_ord_release_rsp: A_REPLY failed\n"));
			mps_free_msgbuf(mbuf_p);
			return(TSYSERR);
		}
		return(0);
	}
}


/* FUNCTION:		O_transaction_rsp()
 *
 * ABSTRACT:	Send OTS O_TRANSACTION_RSP on data socket
 *
 * RETURN CODE:	none
 *
 * CALLED FROM:	O_data_unsol() and O_data_bgrant() (otsintr.c)
 *		O_take_unsol_queue() (otsutils.c)
 */
O_transaction_rsp(de, errcode)

connect *de;			/* data connection entry */
uchar errcode;			/* error code */
{
	mps_msgbuf_t *mbuf_p;		/* MB-II transport message */
	struct o_transaction_rsp res;	/* response being sent */
	uchar tid;			/* requester transaction id */

	tid = de->skt.data.rdtid;

	DEBUGP(DEB_CALL,(CE_CONT, "O_transaction_rsp():\n"));
	DEBUGP(DEB_FULL,(CE_CONT, "  de=%x,tid=%x\n",de,tid));

	if ((mbuf_p = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "O_transaction_rsp: msgbuf alloc (de=%x) failed\n", de));
		ots_stat[ST_TKIMBLK]++;
	}
	else
	{
		res.type = O_TRANSACTION_RSP;
		res.qlen = de->skt.data.rcnt;
		res.errcode = errcode;
		de->skt.data.rdtid = 0;

		/* send reply unsolicited */ 
		mps_mk_unsolrply(mbuf_p, de->skt.data.rem_data, tid,
					(unsigned char *)&res, sizeof(res));
		if (O_tki_send(A_REPLY, de->channel, mbuf_p, 
				(struct dma_buf *)NULL, (struct dma_buf *)NULL))
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "O_transaction_rsp: A_RSVP failed\n"));
			mps_free_msgbuf(mbuf_p);
		}
	}
}

/* FUNCTION:			O_tki_send()
 *
 * ABSTRACT:	Calls TKI send routines with retries
 *
 * CALLED FROM:	All other routines in this module
 */
O_tki_send(type, chan, mbuf_p, obuf, ibuf)

int type;		/* message type to send */
int chan;		/* port's channel */
mps_msgbuf_t *mbuf_p;	/* MB-II transport message */
struct dma_buf *obuf;		/* output solicited data buffer descriptor */
struct dma_buf *ibuf;		/* input solicited data buffer descriptor */
{
	unsigned int retries;	/* retry count */
	int pri;	/* saved interrupt priority */
	long ret;	/* saved AMP return code */

	for (retries = 0; retries < otscfg.xmt_retries; retries++)
	{
		pri = SPL();
		switch (type)
		{
		case A_SOL:
			ret = mps_AMPsend_data((long)chan, mbuf_p, obuf);
			break;
		case A_UNSOL:
			ret = mps_AMPsend((long)chan, mbuf_p);
			break;
		case A_RSVP:
			ret = mps_AMPsend_rsvp((long)chan, mbuf_p, obuf, ibuf);
			break;
		case A_REPLY:
			ret = mps_AMPsend_reply((long)chan, mbuf_p, obuf);
			break;
		}
		splx(pri);
		if (ret == 0)
			return(0);
		else
			ots_stat[ST_TKIRETRY]++;
	}
	ots_stat[ST_TKIFAILS]++;
	return(TSYSERR);
}
