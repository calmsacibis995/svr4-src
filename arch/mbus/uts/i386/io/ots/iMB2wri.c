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

#ident	"@(#)mbus:uts/i386/io/ots/iMB2wri.c	1.3"

/*
** ABSTRACT:	SV-ots driver mid-level downstream routines
**
**	These routines buffer TLI-specific downstream routines from the
**	those which generate and transport OTS messages using the
**	Transport Kernel Interface (TKI).  They allocate, free and otherwise
**	maintain control and data connection entries.  In addition, short-
**	circuiting is performed by these routines: if a request targets
**	another local endpoint, the call is routed to the appropriate
**	indication routine in iMB2rd.c or iTLIrd.c.
**
** MODIFICATIONS:
*/

#include "sys/ots.h"
#include "sys/otserror.h"
#include "sys/otsprot.h"
#include <sys/immu.h>

extern int ots_debug;			/* globals define in ots.c */
extern ulong ots_stat[];

extern connect *M_c_alloc;		/* globals defined in otsutils.c */
extern connect *M_l_alloc;

extern struct otscfg otscfg;		/* globals defined in Space.c */

extern unchar ics_myslotid();			/* kernel global */


/* FUNCTION:			iMB2_abort()
 *
 * ABSTRACT:	Free all connection resources associated with endpoint
 *
 *	Normally, this routine is called multiple times to free all the
 *	resources associated with a connected endpoint.  The first time
 *	through, an OTS disconnect request is sent to the remote correspondent
 *	and the data connection entry is freed.  When the disconnect response
 *	is received, iMB2_abort is called again to free the connection entry.
 *
 * ALGORITHM:
 *
 *	IF (connected endpoint)
 *		send Disconnect Request [frees data entry]
 *	ELSE IF (bound endpoint)
 *		IF (listening)
 *			FOR (each pending connection)
 *				send negative Connect Response
 *			ENDFOR
 *		ENDIF
 *		unbind control entry from endpoint
 *		IF (endpoint being closed)
 *			mark endpoint IDLE [this makes ep available for reuse]
 *		ENDIF
 *	ELSE IF (endpoint being closed)
 *		mark endpoint IDLE
 *	ENDIF
 *
 * RETURN CODE:	none
 *
 * CALLED FROM:	iTLI_abort() - user closed endpoint
 *		iTLI_pferr() - fatal error on endpoint
 *		iMB2_discon_req() - unbind endpoint from connection entry
 *			after data connection resources freed.
 *		iMB2_ignore_discon_rsp() - unbind endpoint from connection entry
 *			after timing out waiting for OTS disconnect response
 *		ots_control_reply() - unbind endpoint from connection entry
 *			after disconnect response received
 */
void
iMB2_abort(ep)

endpoint *ep;
{
	connect *p;

	DEBUGC('z');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_abort: ep=%x, ce=%x, de=%x\n",
			ep, ep->control, ep->data));

	if (ep->data)
	{
		ep->data->state |= S_ABORT;
		iMB2_discon_req(ep, 0, (mblk_t *)NULL);
	}
	else if (ep->control)
	{
		p = ep->control->pending;
		while (p)
		{
			p->state |= S_ABORT;
			iMB2_discon_req(ep, (int)p->port, (mblk_t *)NULL);
			p = p->pending;
		}
		iMB2_unbind_req(ep, (mblk_t *)NULL);
		if (ep->str_state & C_CLOSE)
			ep->str_state = C_IDLE;
	}
	else if (ep->str_state & C_CLOSE)
		ep->str_state = C_IDLE;
}


/* FUNCTION:			iMB2_bind_req()
 *
 * ABSTRACT:	Perform TLI bind request by enabling control socket
 *
 *	If no address is specified, this routine allocates a port from
 *	the port array reserved for data sockets.
 *
 * ALGORITHM:
 *
 *	IF (  (no bind address specified [RFS does this a lot])
 *	    OR(endpoint would be second listener on socket)
 *	   )
 *		allocate port id and use as address
 *	ENDIF
 *	IF (no connection allocated for this address)
 *		allocate and initialize connection entry
 *		open channel [mps_open_chan()]
 *	ELSE
 *		increment reference count to connection entry
 *	ENDIF
 *	store address in endpoint structure
 *	place endpoint on control entry's endpoint list
 *	call iTLI_bind_req_complete() to ack the request
 *	IF (max_pending, i.e. this is a passive open)
 *		IF (address references network controller)
 *			send OTS ACCEPT protocol to controller
 *		ELSE IF (any local connection requests waiting for endpoint)
 *			send connection indication up endpoint's stream
 *		ENDIF
 *	ELSE IF (endpoint supports datagrams)
 *		IF (address references network controller)
 *			send OTS DATAGRAM_OPEN protocol to controller
 *		ENDIF
 *	ENDIF
 *
 * RETURNS:	TBADADDR - invalid bind address
 *		TSYSERR - couldn't allocate bind resources
 *		0 - successful bind
 *
 * CALLED FROM:	Tp_bind_req()
 */
int
iMB2_bind_req(ep, mptr, bind)

endpoint *ep;			/* local endpoint */
mblk_t *mptr;			/* message containing bind request */
struct T_bind_req *bind;	/* actual bind request */
{
	connect *ce;		/* connection entry being bound */
	connect *tmp;		/* used to search connection table */
	mb2socid_t soc;		/* local socket in bind address */
	ushort alloc_port = 0;	/* allocated port if no address given */
	char *opt;		/* protocol options on local connect */
	ushort opt_length;	/* length of protocol options */

	DEBUGC('1');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_bind_req: ep=%x, mptr=%x, bind=%x\n",
				ep,mptr,bind));
	if (bind->ADDR_length == 0)
	{
		if ((alloc_port = M_alloc_port(0)) == 0)
		{
			ots_stat[ST_BADCON3]++;
			DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_bind_req: port alloc failed\n"));
			return(TSYSERR);
		}
		else
			soc = mps_mk_mb2socid(ics_myslotid(), alloc_port);
	}
	else if (  (bind->ADDR_length < sizeof(mb2socid_t))
		 ||(bind->ADDR_length > (long)otscfg.addr_size)
		)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, " addr_len=%x, addr=%x\n",bind->ADDR_length,
	 		*(mb2socid_t *)((char *)bind + bind->ADDR_offset)));
		return(TBADADDR);
	}
	else
		soc = *(mb2socid_t *)((char *)bind + bind->ADDR_offset);


	if (  (bind->CONIND_number)
	    &&(ce = M_find_c(mps_mk_mb2soctopid(soc), M_CT_FIND))
	    &&(M_find_listener(ce))
	   )
	{	/*
		 * This is a second listener on the same socket --
		 *	allocate new socket for listener per "t_bind (3N)"
		 */
		if ((alloc_port = M_alloc_port(0)) == 0)
		{
			ots_stat[ST_BADCON3]++;
			DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_bind_req: port alloc failed\n"));
			return(TSYSERR);
		}
		else
			soc = mps_mk_mb2socid(ics_myslotid(), alloc_port);
	}

	if ((ce = M_find_c(mps_mk_mb2soctopid(soc), M_CT_ALLOC)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_bind_req: connect alloc failed\n"));
		if (alloc_port)
			M_free_port(alloc_port);
		ots_stat[ST_BADCON2]++;
		return(TSYSERR);
	}
	else if (  (++ce->skt.cntrl.ref_cnt == 1)
		 &&((ce->channel = mps_open_chan(mps_mk_mb2soctopid(soc),
				     (int)ots_control_intr, MPS_SRLPRIO)) < 0)
		)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_bind_req: mps_open_chan failed\n"));
		if (alloc_port)
			M_free_port(alloc_port);
		M_free_connect(ce);
		ots_stat[ST_TKIFAILS]++;
		return(TSYSERR);
	}
	else
	{
		if (alloc_port)
		{
			ce->state |= S_ALLOCPORT;
			ep->addr.length = sizeof(mb2socid_t);
			bcopy((char *)&soc, ep->addr.data, sizeof(mb2socid_t));
		}
		else
		{
			ep->addr.length = bind->ADDR_length;
			bcopy((char *)bind + bind->ADDR_offset,
				ep->addr.data,
				(int)bind->ADDR_length);
		}
		M_add_endpoint(ce, ep);
		iTLI_bind_req_complete(ep, mptr, 0);
		if (bind->CONIND_number)
		{
			if (bind->ADDR_length > sizeof(mb2socid_t))
				O_accept();	/* params TBD */
			/* send up indications for
			 *  any waiting local connects
			 */
			tmp = M_l_alloc;
			while (tmp != NULL)
			{
				if (  (tmp->skt.local.rem_cntrl == soc)
				    &&(tmp->ep->tli_state & TS_WCON_CREQ)
				   )
				{
					if (tmp->skt.local.odata)
					{
						opt = (char *)tmp->skt.local.odata->b_rptr;
						opt_length = msgdsize(tmp->skt.local.odata);
					}
					else
					{
						opt = NULL;
						opt_length = 0;
					}

					iMB2_conn_ind(ce,
						mps_mk_mb2socid(ics_myslotid(),tmp->skt.local.cntrl->port),
						tmp->port,
						(struct o_connect *)NULL, 0,
						opt, (int)opt_length,
						(mblk_t *)NULL);
				}
				tmp = tmp->next;
			}
		}
		else if (  (bind->ADDR_length > sizeof(mb2socid_t))
			 &&(ep->options & OPT_CLTS)
			)
			O_datagram_open();	/* params TBD */

		return(0);
	}
}


/* FUNCTION:			iMB2_conn_req()
 *
 * ABSTRACT:	Send connect request to listener
 *
 * ALGORITHM:
 *
 *	validate T_conn_req parameters
 *	allocate connection entry for data socket and update index
 *	allocate data port from free pool
 *	IF (connection to remote host)
 *		open channel [mps_open_chan()]
 *		store channel number in allocated entry
 *		set endpoint structure to reference connection data entry
 *		build and send OTS Connect Request
 *		call iTLI_conn_req_complete to ack request
 *	ELSE
 *		call iTLI_conn_req_complete to ack request
 *		store connection info in allocated entry
 *		set endpoint structure to reference connection data entry
 *		IF (listener endpoint exists)
 *			send connect indication up it's stream
 *		ENDIF
 *	ENDIF
 *
 * RETURNS:	TBADADDR - invalid bind address
 *		TBADOPT - invalid options
 *		TBADDATA - user data too long
 *		TSYSERR - no resources or couldn't send connect request
 *		0 - successful connection request
 *
 * CALLED FROM:	Tp_conn_req()
 */
int
iMB2_conn_req(ep, mptr)

endpoint *ep;		/* local endpoint */
mblk_t *mptr;		/* message containing connection request */
{
	struct T_conn_req *conn;	/* TLI connect request */
	connect *ce;			/* control connect entry */
	connect *de;			/* data connect entry */
	connect *tmp;			/* used to search pending list */
	mblk_t *cdata;			/* connect request user data */
	mblk_t *optptr = NULL;		/* mblock referencing options */
	int entry_type;			/* CT_LOCAL or CT_DATA */
	mb2socid_t dest_soc;		/* destination socket */
	char *opt;			/* protocol options */
	ushort opt_length;		/* length of protocol options */
	
	DEBUGC('2');
	conn = (struct T_conn_req *)mptr->b_rptr;
	opt = (char *)conn + conn->OPT_offset;

	if (  (conn->DEST_length < sizeof(mb2socid_t))
	    ||(conn->DEST_length > (long)otscfg.addr_size)
	   )
		return(TBADADDR);
	else if (  ((opt_length = conn->OPT_length) > otscfg.opts_size)
		 ||(  (opt_length)
		    &&(M_valid_options(ep, (opts *)opt) == FALSE)
		   )
		)
		return(TBADOPT);
	else if (  (cdata = unlinkb(mptr))
		 &&(msgdsize(cdata) > (int)otscfg.cdata_size)
		)
	{
		freemsg(cdata);
		return(TBADDATA);
	}
	ce = ep->control;
	dest_soc = *(mb2socid_t *)((char *)conn + conn->DEST_offset);
	entry_type = (mps_mk_mb2soctohid(dest_soc) == (ushort)ics_myslotid()) ? CT_LOCAL:CT_DATA;

	if ((de = M_alloc_connect(0, entry_type)) == NULL)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_conn_req: connect alloc failed\n"));
		ots_stat[ST_BADCON2]++;
		if (cdata)
			freemsg(cdata);
		return(TSYSERR);
	}
	else if ((de->port = M_alloc_port((ushort)de->index)) == 0)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_conn_req: port alloc failed\n"));
		ots_stat[ST_BADCON3]++;
		M_free_connect(de);
		if (cdata)
			freemsg(cdata);
		return(TSYSERR);
	}
	else if (entry_type == CT_DATA)
	{
		if ((de->channel = mps_open_chan(de->port, (int)ots_data_intr,
						MPS_SRLPRIO)) < 0)
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_conn_req: mps_open_chan failed\n"));
			ots_stat[ST_TKIFAILS]++;
			M_free_connect(de);
			if (cdata)
				freemsg(cdata);
			return(TSYSERR);
		}

		de->ep = ep;			/* tie data socket to ep */
		ep->data = de;
		de->skt.data.rem_cntrl = dest_soc;
		ep->rsoc = dest_soc;

		if (O_connect(ce, ep, dest_soc, conn, cdata))
		{
			M_free_connect(de);
			if (cdata)
				freemsg(cdata);
			ots_stat[ST_BADCON1]++;
			return(TSYSERR);
		}
		else
			iTLI_conn_req_complete(de->ep, mptr);
	}
	else	/* CT_LOCAL => connection to local endpoint */
	{
		/*
		 * Save protocol options
		 */
		if (opt_length == 0)
		{
			if (  (ep->options != otscfg.vc_defaults)
			    &&(optptr = allocb(sizeof(opts), BPRI_HI))
			   )
			{
				opt_length = sizeof(opts);
				opt = (char *)optptr->b_rptr;
				optptr->b_wptr = optptr->b_rptr + opt_length;
				*(opts *)opt = ep->options;
			}
			else
				opt = (char *) NULL;
		}
		else if (optptr = allocb((int)opt_length, BPRI_HI))
		{
			opt = (char *)optptr->b_rptr;
			optptr->b_wptr = optptr->b_rptr + opt_length;
			bcopy(opt, (char *)optptr->b_rptr, (int)opt_length);
		}

		iTLI_conn_req_complete(ep, mptr);

		de->ep = ep;
		ep->data = de;
		de->skt.local.cntrl = ce;
		de->skt.local.rem_cntrl = dest_soc;
		de->skt.local.udata = cdata;		/* save conn msg */
		de->skt.local.odata = optptr;		/* save options */
		/*
		 * search through local bound endpoints
		 * for destination
		 */
		tmp = M_c_alloc;
		while (tmp != NULL)
		{
			if (  (tmp->port == mps_mk_mb2soctopid(dest_soc))
			    &&(M_find_listener(tmp))
			   )
			{
				iMB2_conn_ind(tmp,
						mps_mk_mb2socid(ics_myslotid(), ce->port),
						de->port,
						(struct o_connect *)NULL, 0,
						opt, (int)opt_length,
						(mblk_t *)NULL);
				break;
			}
			tmp = tmp->next;
		}
	}
	return(0);
}


/* FUNCTION:		iMB2_conn_res()
 *
 * ABSTRACT:	Accept connection by sending positive connection response
 *
 * ALGORITHM:
 *
 *	validate parameters
 *	find pending data entry corresponding to seqno
 *	remove from control entry's pending list
 *	IF (consumer remote)
 *		disable interrupts [NOTE]
 *		build and send positive OTS Connect Response
 *		call iTLI_conn_res_complete() to ack request
 *		enable interrupts
 *	ELSE 
 *		allocate streams buffer for T_conn_con header
 *		compute protocol options
 *		call iTLI_conn_con() to inform consumer connection acked
 *		call iTLI_conn_res_complete() to ack request
 *	ENDIF
 *
 * NOTE:
 *
 *  1)	Interrupts are disabled around call to O_connect_rsp and subsequent
 *	acknowledgement to ensure that the endpoint state will be set
 *	correctly before the first message arrives at the endpoint's
 *	data socket.
 *
 *  2)	We respond on the control socket associated with the accepting
 *	endpoint.  Its valid for this socket to be different from the socket
 *	associated with the listening endpoint.
 *
 * RETURNS:	TBADSEQ - couldn't find specified pending connection entry
 *		TBADOPT - invalid options
 *		TBADDATA - user data too long
 *		TSYSERR - no resources or couldn't send connect response
 *		0 - successful connection response
 *
 * CALLED FROM:	Tp_conn_res()
 */
int
iMB2_conn_res(seqno, listen_ep, accept_ep, mptr)

int seqno;		/* identifies pending data connection entry */
endpoint *listen_ep;	/* endpoint which will continue listening */
endpoint *accept_ep;	/* endpoint accepting the connection */
mblk_t *mptr;		/* mblock referencing the TLI connect response */
{
	struct T_conn_res *conn;	/* actual connect response message */
	connect *de;			/* pending data connection entry */
	connect *ce;			/* control connection entry */
	connect *rem_de;		/* other side of local connection */
	int pri;			/* saved interrupt priority */
	mblk_t *cdata;			/* user data accompanying response */
	mblk_t *mptr1;			/* TLI header for connection confirm */
	ushort hdr_length;		/* length of connection confirm mblock*/
	ushort opt_length;		/* length of *opt */
	char *opt;			/* protocol options */

	DEBUGC('3');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_conn_res: seqno=%x,listen_ep=%x,accept_ep=%x,mptr=%x\n",
				seqno,listen_ep,accept_ep,mptr));
	conn = (struct T_conn_res *)mptr->b_rptr;
	opt = (char *)conn + conn->OPT_offset;

	if (  ((opt_length = conn->OPT_length) > otscfg.opts_size)
	    ||(  (opt_length)
	       &&(M_valid_options(accept_ep, (opts *)opt) == FALSE)
	      )
	   )
		return(TBADOPT);

	else if (  (cdata = unlinkb(mptr))
		 &&(msgdsize(cdata) > (int)otscfg.cdata_size)
		)
	{
		freemsg(cdata);
		return(TBADDATA);
	}

	ce = listen_ep->control;

	if ((de = M_remove_pending(ce, (ushort)seqno)) == NULL)
	{
		if (cdata)
			freemsg(cdata);
		return(TBADSEQ);
	}
	else if (de->type == CT_DATA)
	{
		de->ep = accept_ep;		/* tie ep to data socket */
		accept_ep->data = de;
		accept_ep->rsoc = de->skt.data.rem_cntrl;
		accept_ep->rctid = de->skt.data.rctid;
		de->skt.data.rctid = 0;

		if (accept_ep->options & ~de->skt.data.options)
		{
			M_add_pending(ce, de);	/* can't add options */
			return(TBADOPT);
		}
		else if (  (opt_length == 0)
			 &&(accept_ep->options != otscfg.vc_defaults)
			)
		{
			opt_length = sizeof(opts);
			opt = (char *)&accept_ep->options;
		}

		pri = SPL();			/* Note 1 */

		/* if (O_connect_rsp(ce, accept_ep->rsoc... Note 2 */

		if (O_connect_rsp(accept_ep->control, accept_ep->rsoc,
				  cdata, opt, opt_length,
				  mps_mk_mb2soctopid(de->skt.data.rem_data),
				  de->port, E_OK, accept_ep->rctid))
		{
			splx(pri);
			if (cdata)
				freemsg(cdata);
			ots_stat[ST_BADCON1]++;
			return(TSYSERR);
		}
		else
		{
			iTLI_conn_res_complete(listen_ep, accept_ep, mptr, 0);
			if (  (cdata == NULL)
			    &&(opt_length == 0)
			   )
				accept_ep->rctid = 0;
			splx(pri);
			return(0);
		}
	}
	else			/* responding to local endpoint */
	{
		hdr_length = sizeof(struct T_conn_con) +
				accept_ep->addr.length + conn->OPT_length;

		if ((mptr1 = allocb((int)hdr_length, BPRI_HI)) == NULL)
		{
			if (cdata)
				freemsg(cdata);
			return(TSYSERR);
		}

		de->ep = accept_ep;
		accept_ep->data = de;

		rem_de = de->skt.local.dest_data;
		/*
		 * IF (no user supplied options)
		 *	 options = intersection of both endpoint options
		 */
		if (  (opt_length == 0)
		    &&(accept_ep->options != rem_de->ep->options)
		   )
		{
			accept_ep->options &= rem_de->ep->options;
			rem_de->ep->options = accept_ep->options;
			opt_length = sizeof(opts);
			opt = (char *)&accept_ep->options;
		}

		if (cdata)
			linkb(mptr1, cdata);

		iTLI_conn_con(rem_de->ep, mptr1, 0,
				accept_ep->addr.data, accept_ep->addr.length,
				opt, (int)opt_length);
		iTLI_conn_res_complete(listen_ep, accept_ep, mptr, 0);
		return(0);
	}
}

/* FUNCTION:			iMB2_data_req()
 *
 * ABSTRACT:	Send data message to remote correspondent
 *
 * ALGORITHM:
 *
 *	IF (endpoint connected)
 *		IF (connection remote)
 *			send OTS DATA or EOM_DATA message via O_data()
 *		ELSE
 *			IF (destination STREAM blocked)
 *				RETURN M_FLOW_CONTROL
 *			ENDIF
 *			duplicate message
 *			allocate TLI indication header, link duplicate
 *			send message via iTLI_data_ind() call
 *			complete sender processing via iTLI_data_req_complete()
 *		ENDIF
 *		RETURN success
 *	ELSE IF (Virtual Circuit has been aborted)
 *		free message
 *		RETURN success
 *	ELSE
 *		RETURN error
 *	ENDIF
 *
 * RETURNS:	TSYSERR - send failed or connection went away
 *		M_FLOW_CONTROL - receiver blocked
 *		0 - data is on its way
 *
 * CALLED FROM:	Ts_data_req()
 */
int
iMB2_data_req(ep, mptr, more)

endpoint *ep;		/* local endpoint */
mblk_t *mptr;		/* mblock referencing data to be sent */
int more;		/* more flag (if FALSE => send EOM) */
{
	mblk_t *header = NULL;	/* TLI header local data indication */
	mblk_t *mptr1;		/* duplicate reference to data */
	connect *de;		/* local data connection entry */

	DEBUGC('4');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_data_req: ep=%x, mptr=%x, more=%x\n",
				ep, mptr, more));

	if (de = ep->data)
	{
		if  (de->type == CT_DATA)
		{
			return (O_data(ep, mptr, more, T_DATA_REQ));
		}
				/* transmit to local endpoint */
		else if (  (canput(de->skt.local.dest_data->ep->rd_q))
			 &&(header = allocb(sizeof(union qm), BPRI_MED))
	    		 &&(mptr1 = dupmsg(mptr))
			)
		{
			linkb(header, mptr1);
			iTLI_data_ind(de->skt.local.dest_data->ep, header,more);
			iTLI_data_req_complete(ep, mptr, 0);
			return(0);
		}
		else		/* receiving local queue is blocked */
		{
			if (header)
				freemsg(header);
			return(M_FLOW_CONTROL);
		}
	}
	else if (ep->tli_state == TS_IDLE)	/* ignore if VC aborted */
	{
		ep->nbr_datarq--;
		freemsg(mptr);
		return(0);
	}
	else
		return(TSYSERR);
}


/* FUNCTION:		iMB2_discon_req()
 *
 * ABSTRACT:	Break connection, if any, with remote endpoint
 *
 * ALGORITHM:
 *
 *	validate disconnect parameters
 *	identify data entry to be disconnected
 *	IF (remote connection)
 *		delete data connection entry
 * 		IF (data entry was a pending connect)
 * 			build and send negative OTS connect response
 *		ELSE IF (endpoint waiting for listener to accept connection)
 *			cancel connect request transaction
 *			build and send OTS Disconnect Request
 *			timeout the OTS Disconnect Response
 *		ELSE IF (remote agent waiting for OTS orderly release response)
 * 			build and send OTS orderly release response
 *			IF (closing endpoint)
 *				remove endpoint reference to control entry
 *			ENDIF
 *		ELSE
 *			build and send OTS Disconnect Request
 *			timeout the OTS Disconnect Response
 *		ENDIF
 *		ack disconnect request [if mptr provided]
 *	ELSE
 *		ack disconnect request [if mptr provided]
 *		flush connected endpoint stream
 *		send disconnect indication for connected endpoint
 *		delete data connection entry
 *		IF (closing endpoint)
 *			remove endpoint reference to control entry
 *		ENDIF
 *	ENDIF
 *	
 * NOTE:
 *   1)	A disconnect request on a pending endpoint translates into an
 *	OTS connection response error.  Disconnect user data, if any
 *	should be sent with the response.
 *
 *   2)	If endpoint was waiting for connection response from listener,
 *	cancel the connect request transaction and issue a disconnect
 *	request with rport = 0.  If the endpoint got tired of waiting for
 *	a response to an orderly release, cancel the transaction and
 *	issue a disconnect request.  Interrupts are disabled here so we can
 *	free the data entry before TKI returns the resources associated 
 *	with the cancelled connection request transaction.
 *
 *   3)	If the user is prematurely aborting the response side of an orderly
 *	release, respond to the current transaction with an OTS orderly release
 *	response.  Disconnect user data is ignored.
 *
 *   4)	timeout() is called to ensure that resources aren't tied up
 *	forever if remote correspondent doesn't respond to the disconnect
 *	request.
 *
 *   5) SVVS NS/LIB loopback tests expect streams to be flushed when a
 *	disconnect indications arrives.  Any data queued ahead of the disconnect
 *	will be blown away.  This is OK in loop-back mode because the driver
 *	supports orderly release.  However, this stream flush is NOT done
 *	if the disconnect indication arrives from a remote endpoint.  The
 *	remote agent or the connection itself may not support the orderly
 *	release operation so we don't want to lose data that arrived before
 *	the disconnect.  *** UPDATE *** We now blow away queued data when
 *	disconnect indications are received from remote endpoints.
 *
 * RETURNS:	TBADSEQ - couldn't find specified pending connection entry
 *		TBADF - no connection resources associated with endpoint
 *		TBADDATA - user data too long
 *		TSYSERR - no resources or couldn't send disconnect response
 *		0 - successful disconnect response
 *
 * CALLED FROM:	Ts_discon_req() - disconnect request from user (mptr supplied)
 *		iMB2_abort() - user closing endpoint (no mptr)
 */
int
iMB2_discon_req(ep, seqno, mptr)

endpoint *ep;		/* local endpoint */
int seqno;		/* identifies pending connection entry, if any */
mblk_t *mptr;		/* mblock referencing TLI disconnect request */
{
	connect *ce;		/* associated control entry */
	connect *de;		/* associated data entry */
	connect *dest_de;	/* connected data entry */
	mblk_t *ddata = NULL;	/* associated user data with discon request */
	int pri;		/* saved interrupt priority */
	void iMB2_ignore_discon_rsp();

	DEBUGC('5');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_discon_req: ep=%x, seqno=%x, mptr=%x\n",
				ep, seqno, mptr));

	if ((ce = ep->control) == NULL)
		return(TBADF);

	else if (  (mptr)
		 &&(ddata = unlinkb(mptr))
		 &&(msgdsize(ddata) > (int)otscfg.ddata_size)
		)
	{
		freemsg(ddata);
		return(TBADDATA|HIGH_PRI);
	}

	else if (  (seqno)
		 &&((de = M_remove_pending(ep->control, (ushort)seqno)) == NULL)
		)
	{
		if (mptr)
			linkb(mptr, ddata);
		return(TBADSEQ);
	}

	else if (  (seqno == 0)
		 &&((de = ep->data) == NULL)
		)
	{
		if (mptr)
			linkb(mptr, ddata);
		return(TBADF);
	}

	else if (de->type == CT_DATA)		/* remote */
	{
		if (seqno)	/* Note 1 */
		{
			ep->rctid = de->skt.data.rctid;
			ep->rsoc = de->skt.data.rem_cntrl;
			de->skt.data.rctid = 0;

			if (O_connect_rsp(ce, ep->rsoc, ddata, NULL, 0,
				mps_mk_mb2soctopid(de->skt.data.rem_data), de->port,
				E_NO_PASSIVE_OPEN, ep->rctid))
			{
				DEBUGP(DEB_ERROR,(CE_CONT, 
				  "iMB2_discon_req: O_connect_rsp error\n"));
				if (mptr)
					linkb(mptr, ddata);
				M_free_connect(de);
				return(TSYSERR);
			}
			else if (mptr)
				iTLI_discon_req_complete(ep, mptr, 0);
			if (ddata == NULL)
				ep->rctid = 0;
			M_free_connect(de);
			return(0);
		}
		else if (  (ep->tli_state == TS_WCON_CREQ)
			 ||(ep->tli_state == TS_WACK_DREQ6)
			 ||(ep->tli_state == TS_WIND_ORDREL)
			 ||(ep->tli_state == TS_WACK_DREQ10)
			)
		{			/* Note 2 */
			pri = SPL();
			if (  (ep->xctid)
			    &&(mps_AMPcancel(ep->control->channel,
				mps_mk_mb2socid(ics_myslotid(), ce->port), ep->xctid))
			   )
			{
				splx(pri);
				if (mptr)
					linkb(mptr, ddata);
				DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_discon_req: can't cancel transaction %x\n",ep->xctid));
		    		if (de->state & S_ABORT)
				{
					M_free_connect(de);
					iMB2_abort(ep);
				}
				else
					M_free_connect(de);
				return(TSYSERR);
			}

			ep->xctid = 0;

			if (O_disconnect(ce, de, M_DISCONNECT, mptr,ddata))
			{
				splx(pri);
				if (mptr)
					linkb(mptr, ddata);
				DEBUGP(DEB_ERROR,(CE_CONT,
				   "iMB2_discon_req: O_disconnect error\n"));
		    		if (de->state & S_ABORT)
				{
					M_free_connect(de);
					iMB2_abort(ep);
				}
				else
					M_free_connect(de);
				return(TSYSERR);
			}
			else
			{
				M_free_connect(de);
				splx(pri);
				ep->timeout =
				   timeout(iMB2_ignore_discon_rsp, ep, 500);
				return(0);
			}
		}
		else if (ep->tli_state == TS_WREQ_ORDREL)	/* Note 3 */
		{
			if (ddata)
				freemsg(ddata);
			if (O_ord_release_rsp(ce, de->port,
					mps_mk_mb2soctopid(de->skt.data.rem_data),
					de->skt.data.rem_cntrl,
					E_OK, ep->rctid))
			{
		    		if (de->state & S_ABORT)
				{
					M_free_connect(de);
					iMB2_abort(ep);
				}
				else
					M_free_connect(de);
				return(TSYSERR);
			}
			else
			{
				if (mptr)
					iTLI_discon_req_complete(ep, mptr, 0);
		    		if (de->state & S_ABORT)
				{
					M_free_connect(de);
					iMB2_abort(ep);
				}
				else
					M_free_connect(de);
				return(0);
			}
		}
		else if (O_disconnect(ce, de, M_DISCONNECT, mptr, ddata))
		{
			if (mptr)
				linkb(mptr, ddata);
			DEBUGP(DEB_ERROR,
				(CE_CONT, "iMB2_discon_req: O_disconnect error\n"));
	    		if (de->state & S_ABORT)
			{
				M_free_connect(de);
				iMB2_abort(ep);
			}
			else
				M_free_connect(de);
			return(TSYSERR);
		}
		else
		{
			M_free_connect(de);
			/*
			 * Note 4: Wait for disconnect response before deleting
			 *  control entry.  Don't wait longer than 5 seconds.
			 */
			ep->timeout = timeout(iMB2_ignore_discon_rsp, ep, 500);
			return(0);
		}
	}
	else			/* local */
	{
		if (mptr)
			iTLI_discon_req_complete(ep, mptr, 0);

		if (dest_de = de->skt.local.dest_data)
		{
			if (  (dest_de->ep)		/* Note 5 */
			    &&(mptr)
			    &&(ep->tli_state >= TS_WACK_DREQ9)
			   )
			{
				iTLI_send_flush(dest_de->ep);
			}
				
			iMB2_discon_ind(de->skt.local.dest_cntrl,
				mps_mk_mb2socid(ics_myslotid(), ce->port),
				dest_de->port, (struct o_disconnect *)NULL, 0,
				ddata);
		}

		if (  (seqno == 0)
		    &&(de->state & S_ABORT)
		   )
		{
			M_free_connect(de);
			/*
			 * recursive call to get rid of control entry
			 */
			iMB2_abort(ep);
		}
		else
			M_free_connect(de);
		return(0);
	}
}


/* FUNCTION:		iMB2_exdata_req()
 * 
 * ABSTRACT:	Send expedited data message
 *
 *	This routine is called from the driver's put procedure rather than
 *	the service procedure as iMB2_data_req() is called.  Therefore,
 *	expedited messages will be delivered before any locally queued
 *	data messages.
 *
 * ALGORITHM:
 *
 *	IF (endpoint connected)
 *		IF (connection remote)
 *			send OTS DATA or EOM_DATA message via O_data()
 *			RETURN success
 *		ELSE IF (local receiving stream is blocked)
 *			RETURN M_FLOW_CONTROL
 *		ELSE
 *			duplicate message
 *			allocate TLI indication header, link duplicate
 *			send message via iTLI_exdata_ind() call
 *			complete processing via iTLI_exdata_req_complete()
 *			RETURN success
 *		ENDIF
 *	ELSE
 *		RETURN error
 *	ENDIF
 *
 * RETURNS:	TSYSERR - send failed or connection went away
 *		M_FLOW_CONTROL - receiver blocked
 *		0 - data is on its way
 *
 * CALLED FROM:	Tp_exdata_req() - ordinary expedited send
 *		iMB2_exdata_req_complete() - retrying due to flow control
 */
int
iMB2_exdata_req(ep, mptr, more)

endpoint *ep;		/* local endpoint */
mblk_t *mptr;		/* mblock referencing data to be sent */
int more;		/* more flag (if FALSE => send EOM) */
{
	mblk_t *header = NULL;	/* TLI header local data indication */
	mblk_t *mptr1;		/* duplicate reference to data */
	connect *de;		/* local data connection entry */

	DEBUGC('6');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_exdata_req: ep=%x, mptr=%x\n",ep, mptr));

	if (de = ep->data)
	{
		if (de->type == CT_DATA)
		{
			return(O_data(ep, mptr, more, T_EXDATA_REQ));
		}
				/* transmit to local endpoint */
		else if (  (canput(de->skt.local.dest_data->ep->rd_q))
			 &&(header = allocb(sizeof(struct T_exdata_ind), BPRI_HI))
			 &&(mptr1 = dupb(mptr))
			)
		{
			linkb(header, mptr1);
			iTLI_exdata_ind(de->skt.local.dest_data->ep, header, more);
			iTLI_exdata_req_complete(ep, mptr, 0);
			return(0);
		}
		else		/* receiving local queue is blocked */
		{
			if (header)
				freemsg(header);
			return(M_FLOW_CONTROL);
		}
	}
	return(TSYSERR);
}


/* FUNCTION:		iMB2_ignore_discon_rsp()
 *
 * ABSTRACT:	Clean up when disconnect response too long in coming.
 *
 *	If endpoint is being closed, call iMB2_abort() to continue the
 *	shutdown of the endpoint.  Otherwise, respond to user's disconnect
 *	request.
 *
 * CALLED FROM:	timeout()
 */
void
iMB2_ignore_discon_rsp(ep)

endpoint *ep;
{
	mblk_t *mptr;

	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_ignore_discon_rsp: ep=%x\n",ep));

	ep->timeout = 0;

	if (  (ep->control)
	    &&(  (ep->str_state & C_CLOSE)
	       ||(  (ep->tli_state >= TS_WACK_DREQ6)
	          &&(ep->tli_state <= TS_WACK_DREQ11)
		 )
	      )
	   )
	{
		if (  (ep->xctid)
		    &&(mps_AMPcancel(ep->control->channel,
			mps_mk_mb2socid(ics_myslotid(), ep->control->port), ep->xctid))
		   )
		{
			DEBUGP(DEB_ERROR,(CE_CONT, "iMB2_ignore_discon_rsp: can't cancel transaction %x\n",ep->xctid));
		}

		if (ep->str_state & C_CLOSE)
			iMB2_abort(ep);		/* get rid of control entry */

		else if (mptr = allocb(sizeof(union qm), BPRI_HI))
			iTLI_discon_req_complete(ep, mptr, 0);
	}
}


/* FUNCTION:		iMB2_ordrel_req()
 *
 * ABSTRACT:	Initiate or complete orderly release operation
 *
 * ALGORITHM:
 *
 *	IF (endpoint is in the DATA TRANSFER state)
 *		IF (connection is remote)
 *			send OTS orderly release request to correspondent
 *		ELSE [local connection]
 *			find attached local data entry
 *			send orderly release indication via iTLI_ordrel_ind()
 *		ENDIF
 *	ELSE  [endpoint is in IDLE or UNBND state]
 *		IF (connection is remote)
 *			send OTS orderly release response to correspondent
 *		ELSE
 *			find attached local data entry
 *			send orderly release indication via iTLI_ordrel_ind()
 *		ENDIF
 *		free data connection entry
 *	ENDIF
 *
 * NOTES:
 *
 *   1)	No need for iTLI_ordrel_req_complete() function
 *	requests/indications with orderly release operations.
 *
 *   2)	An unbind request may have snuck in behind the queued TE_ORDREL_REQ
 *	causing the state to be unbound instead of idle.  Send the responses
 *	anyway.
 *
 * RETURNS:	TSYSERR - couldn't build or send OTS message
 *		0 - successful send
 *
 * CALLED FROM:	Ts_ordrel_req()
 */
int
iMB2_ordrel_req(ep, mptr)

endpoint *ep;
mblk_t *mptr;
{
	connect *ce;
	connect *de;

	DEBUGC('8');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_ordrel_req: ep=%x, mptr=%x\n", ep, mptr));

	ce = ep->control;
	de = ep->data;
	if (ep->tli_state == TS_WIND_ORDREL)
	{
		if (de->type == CT_DATA)
		{
			if (O_ord_release(ce, de, mptr))
				return(TSYSERR);
			else
				return(0);
		}
		else
		{
			iTLI_ordrel_ind(de->skt.local.dest_data->ep, mptr, 0);
			return(0);
		}
	}
	else if (  (ep->tli_state == TS_IDLE)
		 ||(  (ce)
		    &&(ep->tli_state == TS_UNBND)	/* Note 2 */
		   )
		)
	{
		if (de->type == CT_DATA)
		{
			if (O_ord_release_rsp(ce, de->port,
					mps_mk_mb2soctopid(de->skt.data.rem_data),
					de->skt.data.rem_cntrl,
					E_OK, ep->rctid))
				return(TSYSERR);
			else
			{
				ep->rctid = 0;
				freemsg(mptr);
			}
		}
		else
		{
			iTLI_ordrel_ind(de->skt.local.dest_data->ep, mptr, 0);
			M_free_connect(de->skt.local.dest_data);
		}
		M_free_connect(de);
		return(0);
	}
	else
		return(TSYSERR);
}


/* FUNCTION:		iMB2_unbind_req()
 *
 * ABSTRACT:	Remove endpoint's reference to control entry
 *
 * ALGORITHM:
 *
 *	remove endpoint from control entry's endpoint list
 *	IF (control entry reference count > 1)
 *		decrement reference count
 *	ELSE
 *		close control port
 *		deallocate control connection entry
 *	ENDIF
 *	clear endpoint address fields
 *	call iTLI_unbind_req_complete() to ack request
 *
 * NOTE:	Network controller support is merely stubbed
 *
 * RETURNS:	0 - successful unbind (always)
 *
 * CALLED FROM:	Ts_unbind_req()
 */
int
iMB2_unbind_req(ep, mptr)

endpoint *ep;		/* local endpoint */
mblk_t *mptr;		/* mblock referencing bind request */
{
	connect *ce;	/* control entry being de-referenced/freed */

	DEBUGC('8');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_unbind_req: ep=%x, mptr=%x\n", ep, mptr));

	if (ce = ep->control)
	{
		if (ep->addr.length > sizeof(mb2socid_t))
		{
			if (ep->max_pend)
				O_accept_cancel();	/* params TBD */
			else if (ep->options & OPT_CLTS)
				O_datagram_cancel();	/* params TBD */
		}
		M_remove_endpoint(ce, ep);
		if (--ce->skt.cntrl.ref_cnt == 0)
			M_free_connect(ce);
	}

	ep->addr.length = 0;

	if (mptr)
		iTLI_unbind_req_complete(ep, mptr);

	return(0);
}


/* FUNCTION:		iMB2_unitdata_req()
 *
 * ABSTRACT:	Send datagram
 *
 * ALGORITHM:
 *
 *	validate parameters
 *	IF (remote send)
 *		IF (datagram send already in progress)
 *			return M_FLOW_CONTROL
 *		ELSE
 *			send OTS datagram request
 *		ENDIF
 *	ELSE	[target is local endpoint]
 *		IF (target endpoint found)
 *			IF (receiving stream NOT blocked)
 *				send datagram via iTLI_unitdata_ind() call
 *			ELSE
 *				return M_FLOW_CONTROL
 *			END
 *		ENDIF
 *	ENDIF
 *	IF (couldn't send datagram AND not retrying later)
 *		return datagram to sender as error [iTLI_uderror_ind()]
 *		free message
 *	ENDIF
 *	RETURN success
 *
 * RETURNS:	0 - successful request, always
 *		M_FLOW_CONTROL - receiver blocked
 *
 *	If a NON flow control errors occurs, the datagram is returned to the
 *	user as a T_UDERROR_IND with the following ERROR_types:
 *
 *		TSYSERR - couldn't send datagram
 *		TBADDATA - datagram too large
 *		TBADADDR - invalid target address specified
 *		TBADOPT - options too large
 *
 * CALLED FROM:	Ts_unitdata_req()
 */
int
iMB2_unitdata_req(ep, data, udata)

endpoint *ep;				/* local endpoint */
mblk_t *data;				/* datagram request mblock */
struct T_unitdata_req *udata;		/* datagram request */
{
	connect *ce;			/* local control entry */
	connect *dest_ce;		/* short circuit target control entry */
	int dest_type;			/* remote or local connection */
	mblk_t *header = NULL;		/* TLI header local indication */
	mblk_t *mptr1;			/* duplicate reference to data */
	int error = 0;			/* datagram send error, assume none */
	int alloc_size;			/* allocated header size */
	mb2socid_t dest_addr;		/* destination socket */

	DEBUGC('9');
	DEBUGP(DEB_CALL,(CE_CONT, "iMB2_unitdata_req: ep=%x, data=%x, udata=%x\n",
				ep, data, udata));

	dest_addr = *(mb2socid_t *)((char *)udata + udata->DEST_offset);
	dest_type = (mps_mk_mb2soctohid(dest_addr) == (ushort)ics_myslotid()) ? CT_LOCAL:CT_DATA;
	alloc_size = sizeof(struct T_unitdata_ind) +
		     udata->DEST_length +
		     udata->OPT_length;

	if ((ce = ep->control) == NULL)
		error = TSYSERR;

	else if (  (udata->DEST_length < sizeof(mb2socid_t))
		 ||(udata->DEST_length > (long)otscfg.addr_size)
		)
		error = TBADADDR;

	else if (udata->OPT_length > (long)otscfg.opts_size)
		error = TBADOPT;

	else if (dest_type == CT_DATA)
	{
		error = O_datagram(ce, ep, dest_addr, udata, data);
		if (error == M_FLOW_CONTROL)
			return(M_FLOW_CONTROL);
	}
	else if (  ((dest_ce = M_find_c(mps_mk_mb2soctopid(dest_addr), M_CT_FIND)) == NULL)
		 ||((ep->dest_ep = M_find_dgendpoint(dest_ce)) == NULL)
		)
		error = TSYSERR;

	else if (  (canput(ep->dest_ep->rd_q))
		 &&(header = allocb(alloc_size, BPRI_LO))
		 &&(mptr1 = dupb(data))
		)
	{
		linkb(header, mptr1);
		iTLI_unitdata_ind(ep->dest_ep,
			header,
			ep->addr.length,
			ep->addr.data,
			(int)udata->OPT_length,
			(char *)udata + udata->OPT_offset);
		ep->dest_ep = (endpoint *)NULL;
		iTLI_unitdata_req_complete(ep, data, 0);
	}
	else
	{
		if (header)
			freemsg(header);
		return(M_FLOW_CONTROL);
	}

	if (error)
	{
		iTLI_uderror_ind(ep, error,
			(int)udata->DEST_length,
			(char *)udata+udata->DEST_offset,
			(int)udata->OPT_length,
			(char *)udata+udata->OPT_offset);
		freemsg(data);
	}
	return(0);
}
