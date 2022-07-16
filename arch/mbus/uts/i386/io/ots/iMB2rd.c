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

#ident	"@(#)mbus:uts/i386/io/ots/iMB2rd.c	1.3"

/*
** ABSTRACT:	SV-ots driver mid-level upstream routines
**
**	Routines in this module are normally called from routines in
**	otsintr.c when the processing of incoming requests or responses
**	involves manipulation of connection structures before control
**	passes to routines in iTLIrd.c.  Short-circuited calls may be
**	routed through this module as well.
**
** MODIFICATIONS:
*/

#include "sys/ots.h"
#include "sys/otserror.h"
#include "sys/otsprot.h"
#include <sys/immu.h>

extern int ots_debug;			/* globals defined in ots.c */
extern ulong ots_stat[];
extern struct otscfg otscfg;
extern endpoint ots_endpoints[];

extern ushort ots_version_mask;		/* defined in otsutils.c */

extern unchar ics_myslotid();			/* kernel global */


/* FUNCTION:			iMB2_conn_con()
 *
 * ABSTRACT:	Process incoming OTS Connection Response messages
 *
 * NOTE:
 *
 *	Full support of OTS protocol version negotiation
 *	has not been implemented yet.  If connect request fails due to
 *	an E_PROTOCOL_VERSION error, we should send a O_NEGOTIATE_VER
 *	command to the listener socket.  The difficult part is doing this
 *	within the context of the connect request.
 *		
 * CALLED BY:	O_control_reply()
 */
iMB2_conn_con(ce, csoc, ots_m, data)

connect *ce;			/* local control connection entry */
mb2socid_t csoc;		/* remote (server's) control socket */
struct o_connect_rsp *ots_m;	/* ots connect response message */
mblk_t *data;			/* associated data */
{
	mblk_t *mptr;		/* streams buffer for T_conn_con */
	connect *de;		/* waiting data connection entry */
	int header_size;	/* size of mptr */
	char *opt;		/* connect options, if any */
	int opt_length;		/* length of connect options */

	DEBUGC('v');
	header_size = sizeof(struct T_conn_con) + ots_m->options + ots_m->raddr;
	if ((mptr = allocb(header_size, BPRI_HI)) == NULL)
	{
		cmn_err(CE_WARN, "SV-ots: iMB2_conn_con allocb failed\n");
		ots_stat[ST_ALFA]++;
	}
	else if (de = M_find_d(ots_m->rport))
	{
		de->ep->xctid = 0;

		de->skt.data.rem_cntrl = csoc;
		de->skt.data.rem_data = mps_mk_mb2socid(mps_mk_mb2soctohid(csoc),
						    ots_m->lport);
		de->skt.data.bufsize = OTS_BUFSIZE(ots_m->buffer);
		de->skt.data.version = OTS_VERSION;	/* see NOTE above */
		de->skt.data.xcnt = ots_m->qlen;
		/*
		 * update endpoint options
		 */
		if (opt_length = ots_m->options)
			opt = (char *)data->b_rptr;
		else if (de->ep->options != otscfg.vc_defaults)
		{
			opt = (char *)&otscfg.vc_defaults;
			opt_length = sizeof(opts);
		}
		else
			opt = NULL;

		if (opt_length)		/* validate incoming options */
		{
			if (  (*((opts *)opt) & ~de->ep->options)
			    ||((*((opts *)opt) & OPT_COTS) == FALSE)
			   )
			{
				freemsg(mptr);
				if (data)
					freemsg(data);
				iTLI_pferr(de->ep, EPROTO);
				return;
			}
			else
				de->ep->options = *(opts *)opt;
		}

		/* ignoring raddr */

		if (ots_m->userdata)
		{
			data->b_rptr += ots_m->options + ots_m->raddr;
			data->b_wptr = data->b_rptr + ots_m->userdata;
			linkb(mptr, data);
		}
		else if (data)
			freemsg(data);
		iTLI_conn_con(de->ep, mptr, (int)ots_m->errcode,
			    (char *)&csoc, sizeof(mb2socid_t), opt, opt_length);
		if (ots_m->errcode != E_OK)
			M_free_connect(de);
	}
	else
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_conn_con: no one here for %x\n",csoc));
		freemsg(mptr);
		if (data)
			freemsg(data);
	}
}


/* FUNCTION:			iMB2_conn_ind()
 *
 * ABSTRACT:	Process incoming OTS Connection Request messages
 *
 * ALGORITHM:
 *
 *	find listening endpoint
 *	IF (remote and bad OTS_VERSION)
 *		reply with E_PROTOCOL_VERSION error
 *	ENDIF
 *	allocate streams buffer for T_conn_ind structure
 *	allocate data connection entry
 *	allocate data port id
 *	put data connection entry on control's pending list
 *	IF (remote)
 *		initialize data entry
 *	ELSE
 *		initialize local connection entry
 *		update local entry fields on both ends to seal connection
 *	ENDIF
 *	call iTLI_conn_ind to complete processing
 *
 * CALLED BY:	ots_control_bgrant(), ots_control_unsol() for remote connections
 *		iMB2_bind_req(), iMB2_conn_req() for local connections
 */
iMB2_conn_ind(ce, csoc, dport, ots_m, tid, opt, opt_size, data)

connect *ce;			/* local receiving control socket */
mb2socid_t csoc;		/* remote sending socket */
ushort dport;			/* remote data port */
struct o_connect *ots_m;	/* connect request */
uchar tid;			/* remote host's transaction id */
char *opt;			/* connect address and options */
int opt_size;			/* length of user options */
mblk_t *data;			/* user data accompanying connect request */
{
	char *addr;			/* connect address */
	connect *de;			/* server data entry */
	connect *dest_de = NULL;	/* used if connecting agent local */
	endpoint *ep;			/* listening endpoint */
	ushort entry_type;		/* CT_LOCAL or CT_DATA */
	int addr_size;			/* length of address */
	int mblk_size;			/* size of TLI header */
	mblk_t *mptr;			/* references TLI header */
	mblk_t *udata;			/* user data, if any */

	DEBUGC('w');
	entry_type = (mps_mk_mb2soctohid(csoc) == (ushort)ics_myslotid()) ? CT_LOCAL : CT_DATA;

	if ((ep = M_find_listener(ce)) == NULL)
	{
		iMB2_conn_ind_error(ce, csoc, dport, entry_type, tid);
		return;
	}

	if (entry_type == CT_DATA)
	{
		if (O_version_supported(ots_m->version) == FALSE)
		{
			O_connect_rsp(ce, csoc, (mblk_t *)NULL, (char *)NULL,
				0, dport, 0, E_PROTOCOL_VERSION, tid);
			return;
		}
		if (opt_size = ots_m->options)
			opt = (char *)data->b_rptr;
		else if (ep->options != otscfg.vc_defaults)
		{
			opt_size = sizeof(opts);
			opt = (char *)&otscfg.vc_defaults;
		}
		else
			opt = NULL;

		/* NOTE: we're not passing laddr and raddr up to server */
		/*	later we may want to fix this for router support */

		addr_size = sizeof(mb2socid_t);
		addr = (char *)&csoc;
	}
	else	/* local connection */
	{
		dest_de = M_find_d(dport);

		addr_size = dest_de->ep->addr.length;
		addr = dest_de->ep->addr.data;
	}
	mblk_size = sizeof(struct T_conn_ind) + addr_size + opt_size;

	if ((mptr = allocb(mblk_size, BPRI_HI)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_conn_ind: mptr alloc failed\n"));
		ots_stat[ST_ALFA]++;
		iMB2_conn_ind_error(ce, csoc, dport, entry_type, tid);
	}
	else if ((de = M_alloc_connect(0, (char)entry_type)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_conn_ind: connect alloc failed\n"));
		freemsg(mptr);
		ots_stat[ST_BADCON2]++;
		iMB2_conn_ind_error(ce, csoc, dport, entry_type, tid);
	}
	else if ((de->port = M_alloc_port((ushort)de->index)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_conn_ind: dport alloc failed\n"));
		ots_stat[ST_BADCON3]++;
		freemsg(mptr);
		M_free_connect(de);
		iMB2_conn_ind_error(ce, csoc, dport, entry_type, tid);
	}
	else if (  (entry_type == CT_DATA)
		 &&((de->channel = mps_open_chan(de->port, (int)ots_data_intr,
						MPS_SRLPRIO)) < 0)
		)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_conn_ind: mps_open_chan failed\n"));
		ots_stat[ST_TKIFAILS]++;
		freemsg(mptr);
		M_free_connect(de);
		iMB2_conn_ind_error(ce, csoc, dport, entry_type, tid);
	}
	else
	{
		de->ep = ep;
		M_add_pending(ce, de);

		if (entry_type == CT_DATA)
		{
			de->skt.data.rem_cntrl = csoc;
			de->skt.data.rem_data =
				mps_mk_mb2socid(mps_mk_mb2soctohid(csoc), dport);
			de->skt.data.bufsize = OTS_BUFSIZE(ots_m->buffer);
			de->skt.data.xcnt = ots_m->qlen;
			de->skt.data.rctid = tid;
			de->skt.data.version = ots_m->version;
			if (ots_m->options)
				de->skt.data.options = *(opts *)opt;
			else
				de->skt.data.options = otscfg.vc_defaults;
			if (  (ots_m->userdata)
			    &&(udata = dupmsg(data))
			   )
			{
				udata->b_rptr += ots_m->options +
						ots_m->laddr + ots_m->raddr;
				linkb(mptr, udata);
			}
		}
		else
		{
			de->skt.local.cntrl = ce;
			de->skt.local.rem_cntrl = csoc;
			de->skt.local.dest_cntrl = M_find_c(mps_mk_mb2soctopid(csoc), M_CT_FIND);
			dest_de->skt.local.dest_cntrl = ce;
			de->skt.local.dest_data = dest_de;
			dest_de->skt.local.dest_data = de;

			/* don't forget about user data assoc w/ connect req */
			if (dest_de->skt.local.udata)
			{
				linkb(mptr, dest_de->skt.local.udata);
				dest_de->skt.local.udata = NULL;
			}
		}
		iTLI_conn_ind(de->ep, mptr, opt, opt_size, addr, addr_size,
			(int)de->port);

		if (  (dest_de)
		    &&(dest_de->skt.local.odata)
		   )
		{
			dest_de->skt.local.odata = NULL;
			freemsg(dest_de->skt.local.odata);
		}
	}
}


/* FUNCTION:		iMB2_conn_ind_error()
 *
 * ABSTRACT:	Reject connection request
 *
 *	This routine delivers the rejection upstream if the requester is
 *	a local endpoint, or it calls O_connect_rsp() to deliver the rejection
 *	to the remote agent via OTS.
 *
 * CALLED FROM:	iMB2_conn_ind()
 */
iMB2_conn_ind_error(ce, csoc, dport, entry_type, tid)

connect *ce;		/* receiving control connection entry */
mb2socid_t csoc;	/* requesting socket */
ushort dport;		/* requester's data port */
uchar entry_type;	/* type of port */
uchar tid;		/* transaction id, if requester remote */
{
	connect *de;		/* local requesting data entry */
	mblk_t *mptr;		/* response TLI mblock */

	DEBUGC('x');
	if (entry_type == CT_DATA)
		O_connect_rsp(ce, csoc, (mblk_t *)NULL, (char *)NULL, 0, dport,
			0, E_NO_RESOURCES, tid);

	else if (  (de = M_find_d(dport))
		 &&(mptr = allocb(sizeof(struct T_conn_con), BPRI_HI))
		)
		iTLI_conn_con(de->ep, mptr, E_NO_RESOURCES, NULL, 0, NULL, 0);
}


/* FUNCTION:			iMB2_discon_ind()
 *
 * ABSTRACT:	Process received disconnect request
 *
 * ALGORITHM:
 *
 *	IF (connection remote)
 *		locate associated local data entry (may be pending)
 *		respond to disconnect request
 *		IF (unsol queue NOT empty)
 *			queue disconnect indication behind unsols
 *			RETURN;
 *		ENDIF
 *	ENDIF
 *	send indication upstream [iTLI_discon_ind()]
 *	free data connection entry
 *
 * CALLED FROM:	O_control_unsol() or O_control_bgrant() - remote request
 *		iMB2_discon_req() - request to break local connection
 */
iMB2_discon_ind(ce, csoc, dport, ots_m, tid, ddata)

connect *ce;			/* local control connect entry */
mb2socid_t csoc;		/* remote control socket */
ushort dport;			/* local data port */
struct o_disconnect *ots_m;	/* OTS message block */
uchar tid;			/* request transaction id */
mblk_t *ddata;			/* disconnect data */
{
	connect *de;		/* receiving data entry */
	endpoint *ep;		/* receiving endpoint */
	uchar entry_type;	/* CT_DATA or CT_LOCAL */
	uchar reason;		/* disconnect reason */
	
	DEBUGC('y');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_discon_ind: ce=%x, csoc=%x, dport=%x,",
				ce, csoc, dport));
	DEBUGP(DEB_CALL,(CE_CONT, " ddata=%x, tid=%x\n", ddata, tid));

	entry_type = (mps_mk_mb2soctohid(csoc) == ics_myslotid()) ? CT_LOCAL : CT_DATA;

	if (dport)
		de = M_find_d(dport);
	else
		de = M_find_pending(ce, ots_m->lport);

	if (entry_type == CT_DATA)
	{
		if (  (de)
		    &&(ots_m->lport == mps_mk_mb2soctopid(de->skt.data.rem_data))
		   )
		{
			O_disconnect_rsp(ce, csoc, dport, ots_m->lport, E_OK, tid);
			reason = ots_m->reason;
		}
		else
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_discon_ind: bad lport\n"));
			O_disconnect_rsp(ce, csoc, dport, 0, E_NO_CONNECTION, tid);
			de = NULL;
		}
	}
	else
		reason = M_DISCONNECT;

	if (  (de)
	    &&(ep = de->ep)
	   )
	{
	/* NOTE:
	 *
	 *	In the code commented out below, we were queueing disconnect
	 *	indications behind data indications already received.  This
	 *	causes numerous SVVS NS/LIB tests to fail as they expect all
	 *	disconnects to flush the endpoint's streams.  In the new code,
	 *	we blow away the unsol queue and flush the streams.  OTS users
	 *	who's implementations don't support orderly release may not
	 *	appreciate this change...ah, such is life.
	 *
	 *	if (  (entry_type == CT_DATA)
	 *	    &&(Q_EMPTY(de) == FALSE)
	 *	   )
	 *	{
	 *		if (ep->discon_ind != T_DISCON_IND)
	 *		{
	 *			ep->discon_ind = T_DISCON_IND;
	 *			ep->reason = reason;
	 *			ep->ddata = ddata;
	 *		}
	 *		else if (ddata)
	 *			freemsg(ddata);
	 */
		if (entry_type == CT_DATA)
		{
			if (ep->tli_state >= TS_DATA_XFER)
			{
#ifndef V_3 /* RJF */
				flushq(ep->rd_q, FLUSHALL);
#else
				flushq(ep->rd_q);
#endif
				iTLI_send_flush(ep);
				QHEAD(de) = QTAIL(de);
				de->state &= ~S_QUEUE_FULL;
			}

			/* cancel any locally initiated transaction */

			if (ep->xctid)
			{
				mps_AMPcancel(ce->channel,
				  mps_mk_mb2socid(ics_myslotid(), ce->port), ep->xctid);
				ep->xctid = 0;
			}
 			iTLI_discon_ind(de->ep, (mblk_t *)NULL, reason,
				de->port, ddata);
			M_free_connect(de);
		}
		else
		{
 			iTLI_discon_ind(de->ep, (mblk_t *)NULL, reason,
				de->port, ddata);
			M_free_connect(de);
		}
	}
	else if (ddata)
		freemsg(ddata);
}


/* FUNCTION:			iMB2_ordrel_ind()
 *
 * ABSTRACT:	Process orderly release indication
 *
 * 	Similar to the former handling of disconnect indications, orderly
 *	release	indications must be queued behind any unsol data indications
 *	queued in the data socket.
 *
 *	The tid must be preserved in the endpoint structure until
 *	the O_ORD_RELEASE_RSP is sent.  Subsequent data sends must NOT
 *	destroy the tid.
 *
 * CALLED FROM:	O_control_unsol()
 */
iMB2_ordrel_ind(ce, csoc, ots_m, tid)

connect *ce;			/* local control connect entry */
mb2socid_t csoc;		/* remote control socket */
struct o_release *ots_m;	/* OTS message block */
uchar tid;			/* request transaction id */
{
	connect *de;		/* local data connect entry */
	endpoint *ep;		/* local endpoint */
	mblk_t *mptr;		/* message block for TLI indication */

	DEBUGC('z');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_ordrel_ind: ce=%x, csoc=%x,", ce, csoc));
	DEBUGP(DEB_CALL,(CE_CONT, " ots_m=%x, tid=%x\n", ots_m, tid));

	if (  ((de = M_find_d(ots_m->rport)) == NULL)
	    ||(csoc != de->skt.data.rem_cntrl)
	   )
		O_ord_release_rsp(ce, ots_m->rport,
				ots_m->lport, csoc, E_NO_CONNECTION, tid);
	else if ((mptr = allocb(sizeof(union qm), BPRI_HI)) == NULL)
		O_ord_release_rsp(ce, ots_m->rport,
				ots_m->lport, csoc, E_NO_RESOURCES, tid);
	else if (ep = de->ep)
	{
		ep->rctid = tid;
		if (Q_EMPTY(de) == FALSE)
		{
			if (ep->discon_ind == 0)
			{
				ep->ddata = mptr;
				ep->discon_ind = T_ORDREL_IND;
			}
		}
		else
			iTLI_ordrel_ind(ep, mptr, 0);
	}
}


/* FUNCTION:			iMB2_unitdata_ind()
 *
 * ABSTRACT:	Process datagram indications
 *
 *	send datagram response
 *	call iTLI_unitdata_ind to build and queue TLI datagram indication
 *
 * NOTES:
 *
 *   1) all buffers required to send the message upstream have been pre-posted
 *	and are already linked together.  The option and address fields
 *	exist in the buffer list right being the datagram.  Buffers trailing
 *	this information can be used to save contiguous copies of the option
 *	and address information.  The logic is incredibly messy because in order
 *	to avoid the ADMA data chaining problem, we have to fill chained STREAMS
 *	buffers on receive; the bounds of the buffers don't necessarily coincide
 *	with O_DATAGRAM fields.
 *   2) In the address STREAMS buffer we have left room for the sender's socket.
 *
 * CALLED FROM:	O_control_bgrant()
 */
iMB2_unitdata_ind(ce, csoc, ots_m, tid, mptr)

connect *ce;			/* local control connect entry */
mb2socid_t csoc;		/* remote control socket */
struct o_datagram *ots_m;	/* OTS message block */
int tid;			/* request transaction id */
mblk_t *mptr;			/* pre-allocated TLI header and data */
{
	endpoint *ep;		/* receiving endpoint */
	int addr_length;	/* length of sender's address */
	int msg_length;		/* # bytes sender thought it delivered */
	mblk_t *info_head;	/* first block after one containing data */
	mblk_t *next;		/* mblock ptr used to parse message list */
	mblk_t *prev;		/* mblock previous to next */
	char *addr;		/* ptr to sender's address */
	char *opt;		/* ptr to datagram options */
	char *src, *dest;	/* ptrs used to copy info fields */
	ushort size;		/* size of fields being examined/copied */

	DEBUGC('z');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_unitdata_ind: ce=%x, csoc=%x,", ce, csoc));
	DEBUGP(DEB_CALL,(CE_CONT, " header= %x, tid=%x\n", mptr, tid));

	addr_length = ots_m->laddr + ots_m->raddr;
	msg_length = ots_m->userdata + ots_m->options + addr_length;

	if (  ((ep = M_find_dgendpoint(ce)) == NULL)
	    ||(msg_length > msgdsize(mptr->b_cont))	/* didn't get it all */
	   )
	{
		O_datagram_rsp(ce, csoc, E_DATAGRAM, (uchar)tid);
		freemsg(mptr);
	}
	else
	{
		O_datagram_rsp(ce, csoc, E_OK, (uchar)tid);
		/*
		 * remove reserved options and address mblocks from list
		 */
		next = prev = mptr->b_cont;
		while (next->b_wptr - next->b_rptr)
		{
			prev = next;
			if ((next = next->b_cont) == NULL)
				break;
		}

		if ((info_head = unlinkb(prev)) == NULL)
		{
			O_datagram_rsp(ce, csoc, E_DATAGRAM, (uchar)tid);
			freemsg(mptr);
			cmn_err(CE_WARN, "SV-ots: iMB2_unitdata_ind bad alloc\n");
		}

		if (ots_m->options)
		{
			opt = (char *)info_head->b_rptr;
			addr = (char *)info_head->b_cont->b_rptr;
		}
		else
		{
			opt = (char *)NULL;
			addr = (char *)info_head->b_rptr;
		}
		/*
		 * find and mark end of datagram
 		 */
		next = mptr->b_cont;
		size = 0;
		while ((size += next->b_wptr - next->b_rptr) < ots_m->userdata)
			next = next->b_cont;
		next->b_wptr -= (size - ots_m->userdata);

		src = (char *)next->b_wptr;

		/*
		 * copy options into waiting buffer
 		 */
		for (size = ots_m->options, dest = opt; size != 0; size--)
		{
			if (src > (char *)next->b_datap->db_lim)
			{
				next = next->b_cont;
				src = (char *)next->b_rptr;
			}
			*dest++ = *src++;
		}
		/*
		 * copy laddr and raddr into waiting buffer
		 */
		for (size = addr_length, dest = addr + sizeof(mb2socid_t); size != 0; size--)
		{
			if (src > (char *)next->b_datap->db_lim)
			{
				next = next->b_cont;
				src = (char *)next->b_rptr;
			}
			*dest++ = *src++;
		}

		addr_length += sizeof(mb2socid_t);
		*(mb2socid_t *)addr = csoc;

		iTLI_unitdata_ind(ep, mptr, addr_length, addr,
				(int)ots_m->options, opt);

		freemsg(info_head);
	}
}


/* FUNCTION:			iMB2_negotiate_version()
 *
 * ABSTRACT:	Process O_NEGOTIATE_VER request
 *
 *	Return highest protocol in local mask that matches remote mask
 *
 * CALLED BY:	O_control_unsol()
 */
iMB2_negotiate_version(ce, csoc, ots_m, tid)

connect *ce;			/* local control connect entry */
mb2socid_t csoc;		/* remote control socket */
struct o_negotiate_ver *ots_m;	/* OTS message block */
int tid;			/* request transaction id */
{
	ushort mask;
	uchar version = 1;

	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_negotiate_version: ce=%x,csoc=%x\n",ce,csoc));

	if ((mask = (ots_m->list & ots_version_mask)) == 0)
		O_negotiate_ver_rsp(ce, csoc, E_PROTOCOL_VERSION, 0,
			(uchar)tid);

	while (mask >>= 1)	/* shift right until all versions split off */
		version++;

	O_negotiate_ver_rsp(ce, csoc, E_OK, version, (uchar)tid);
}


/* FUNCTION:		O_version_supported()
 *
 * ABSTRACT:	Verify protocol in OTS message block supported
 */
O_version_supported(version)

uchar version;
{
	ushort mask = 1;

	DEBUGP(DEB_CALL,(CE_CONT, "O_version_supported: version=%x\n",version));

	mask <<= (version - 1);
	if (mask & ots_version_mask)
		return(TRUE);
	else
		return(FALSE);
}

/* FUNCTION:			iMB2_enable_remote()
 *
 * ABSTRACT:	enable local sender's write queue, if blocked
 */
iMB2_enable_remote(ep)

endpoint *ep;
{
	connect *de;
	endpoint *sender_ep;

	if (  (de = ep->data)
	    &&(de->type == CT_LOCAL)
	   )
		qenable(WR(de->skt.local.dest_data->ep->rd_q));
	else if (ep->options & OPT_CLTS)
	{
		/*
		 * enable all local senders blocked on this read queue
		 */
		for (sender_ep = &ots_endpoints[otscfg.n_vcs + 1];
		     sender_ep <= &ots_endpoints[otscfg.n_endpoints];
		     sender_ep++
		    )
		{
			if (sender_ep->dest_ep == ep)
				qenable(WR(sender_ep->rd_q));
		}
	}
}
