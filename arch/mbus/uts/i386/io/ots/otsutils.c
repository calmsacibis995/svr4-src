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

#ident	"@(#)mbus:uts/i386/io/ots/otsutils.c	1.3.1.1"

/*
** ABSTRACT:	SV-ots driver low-level assist routines
**
** MODIFICATIONS:
*/

#include "sys/ots.h"
#include "sys/otsprot.h"
#include "sys/otserror.h"
#include <sys/conf.h>
#include <sys/var.h>
#include <sys/immu.h>

extern int ots_debug;			/* Globals defined in ots.c */
extern ulong ots_stat[];
extern struct module_info ots_winfo;
extern struct module_info ots_rinfo;

extern connect ots_connects[];		/* Globals defined in space.c */
extern endpoint ots_endpoints[];
extern ushort ots_p_index[];
extern struct otscfg otscfg;
#ifdef V_3
extern ushort otsdg_major;
#else
extern major_t otsdg_major;
#endif

/* Globals that head connection table and data port free lists */
connect *M_c_free;
connect *M_c_alloc;
connect *M_l_alloc;
ushort M_p_free;

ushort ots_queue_size;			/* other globals defined here */
ushort ots_version_mask = OTS_VERSION;

static char *ots_rcsid = "@(#)ots  $SV_ots R4.0 - 10/25/89$";


/* FUNCTION:			O_alloc_csolbuf()
 *
 * ABSTRACT:	Allocate streams buffers and data descriptors for breq and snr's
 *					on control sockets
 *
 *	This function provides flow control on control sockets for incoming
 *	datagrams.  STREAMS buffers are allocated and linked together to
 *	receive the entire message.  The length of data received cannot exceed
 *	the sum	of the maximum sizes for datagram plus the options, laddr and
 *	raddr field lengths.  Separate STREAMS buffers are allocated for the
 *	options	and address information.  Later, the information from the main
 *	message buffers at the head of the list will be copied into these
 *	buffers at the rear of the list for passing the info upstream to the
 *	user.
 *
 *	Unlike O_alloc_dsolbuf() below, this routine allocates all or nothing;
 *	O_alloc_dsolbuf() may allocate only fragments of the entire buffer
 *	request.
 *
 * ALGORIGTHM:
 *
 *	allocate TLI header
 *	IF (request larger than 4K STREAMS buffer)
 *		allocate and link 4K STREAM buffers together
 *	ENDIF
 *	allocate final (or only) data buffer
 *	allocate buffers for options and address info
 *	allocate data descriptors per # stream buffer segments
 *	initialize data descriptors
 *	return mblock list
 *
 * RETURNS:	ptr to allocated mblock list
 *		ptr to data descriptor which reference allocated buffers
 *		size of streams buffer which couldn't be allocated
 *
 * CALLED FROM:	O_control_snf()
 *		O_control_breq()
 */
mblk_t *
O_alloc_csolbuf(ep, mbuf_p, dbpp, asize)

endpoint *ep;		/* receiving endpoint */
mps_msgbuf_t *mbuf_p;	/* buffer request message */
struct dma_buf **dbpp;	/* data buffer descriptor ptr returned to caller */
ulong *asize;		/* buffer size causing allocb() failure */
{
	mblk_t *header;		/* header structure */
	mblk_t *mptr;		/* allocated message ptr */
	mblk_t *info = NULL;	/* ptr to options and addr buffers */
	struct dma_buf *dbp;		/* used to parse data descriptor list */
	struct o_datagram *odg;	/* datagram request message */
	ulong dg_size;		/* size of datagram */
	ulong msg_size;		/* size of message being received */
	ushort options_size;	/* size of options */
	ushort addr_size;	/* size of address plus room for socket */
	ushort max_size;	/* maximum size of streams buffer allocated */
	ushort seg_cnt;		/* data segments in solicited send */

	DEBUGP(DEB_CALL,(CE_CONT, "O_alloc_csolbuf: ep=%x,mbuf_p=%x,dbpp=%x\n",
			ep, mbuf_p, dbpp));

	odg = (struct o_datagram *) &mbuf_p->mb_data[MPS_MG_BRUD];

	*dbpp = (struct dma_buf *)NULL;
	*asize = 0;

	dg_size = min(odg->userdata, otscfg.datagram_size);

	options_size = min(odg->options, otscfg.opts_size);

	addr_size = min((unsigned int)(odg->laddr + odg->raddr),
			  (unsigned int)(otscfg.opts_size + (2 * otscfg.addr_size)));

	msg_size = dg_size + options_size + addr_size;

	addr_size += sizeof(mb2socid_t);

	max_size = (otscfg.buffer_size) ? otscfg.buffer_size : SBLK4096;

	if ((header = allocb(sizeof(union control_header), BPRI_LO)) == NULL)
	{
		*asize = sizeof(union control_header);
		return(NULL);
	}

	while (msg_size > max_size)
	{
		if ((mptr = allocb((int)max_size, BPRI_LO)) == NULL)
		{
			*asize = max_size;
			freemsg(header);
			return(NULL);
		}
		else
		{
			linkb(header, mptr);
			mptr->b_rptr = mptr->b_datap->db_base;
			mptr->b_wptr = mptr->b_rptr + max_size;
			msg_size -= max_size;
		}
	}
	if (msg_size)
	{
		if ((mptr = allocb((int)msg_size, BPRI_LO)) == NULL)
		{
			freemsg(header);
			*asize = msg_size;
			return(NULL);
		}
		else
		{
			linkb(header, mptr);
			mptr->b_rptr = mptr->b_datap->db_base;
			mptr->b_wptr = mptr->b_rptr + msg_size;
		}
	}

	/*
	 * allocate buffer for options field, if necessary
 	 */
	if (options_size)
	{
		if ((info = allocb((int)options_size, BPRI_LO)) == NULL)
		{
			freemsg(header);
			*asize = options_size;
			return(NULL);
		}
		else
			info->b_rptr = info->b_wptr = info->b_datap->db_base;
	}
	/*
	 * allocate buffer for fields
 	 */
	if ((mptr = allocb((int)addr_size, BPRI_LO)) == NULL)
	{
		freemsg(header);
		if (info)
			freemsg(info);
		*asize = addr_size;
		return(NULL);
	}
	else
	{
		mptr->b_rptr = mptr->b_wptr = mptr->b_datap->db_base;
		if (info)
			linkb(info, mptr);
		else
			info = mptr;
	}

	/*
	 * count segments
	 */
	mptr = header->b_cont;
	seg_cnt = 0;
	do
	{
		seg_cnt++;
	}
	while (mptr = mptr->b_cont);

	if ((*dbpp = mps_get_dmabuf(((int)seg_cnt + 1),DMA_NOSLEEP)) == NULL)
	{
		freemsg(header);
		if (info)
			freemsg(info);
		ots_stat[ST_TKIDBLK]++;
		return(NULL);
	}

	DEBUGP(DEB_FULL,(CE_CONT, "  header=%x,seg_cnt=%x,size=%x,dbp=%x\n",
			header,seg_cnt,msgdsize(header->b_cont),*dbpp));

	/*
	 * initialize data descriptors
	 */
	for (mptr = header->b_cont, dbp = *dbpp;
	     seg_cnt != 0;
	     mptr = mptr->b_cont
	    )
	{
		dbp->count = mptr->b_wptr - mptr->b_rptr;
		dbp->address = (ulong)kvtophys((caddr_t)mptr->b_rptr);
		dbp = dbp->next_buf;
		seg_cnt--;
	}
	dbp->count = 0;
	dbp->next_buf = NULL;

	if (info)
		linkb(header, info);

	return(header);
}
	
 
/* FUNCTION:			O_alloc_dsolbuf()
 *
 * ABSTRACT:	Allocate streams buffers and data descriptors for breq and snr's
 *					on data sockets
 *
 * ALGORITHM:
 *
 *	allocate TLI header
 *	IF (request larger than 4K STREAMS buffer)
 *		allocate and link 4K STREAM buffers, as needed and available
 *	ENDIF
 *	allocate final (or only) data buffer
 *	allocate data descriptors per # stream buffer segments
 *	initialize data descriptors
 *	return mblock list
 *
 * RETURNS:	ptr to allocated mblock list
 *		ptr to data descriptor which reference allocated buffers
 *		size of streams buffer which couldn't be allocated
 *
 * CALLED FROM:	O_data_snf()
 *		O_data_breq()
 */
mblk_t *
O_alloc_dsolbuf(de, mbuf_p, dbpp, asize, frag_ok)

connect *de;		/* data connection entry */
mps_msgbuf_t *mbuf_p;	/* buffer request message */
struct dma_buf **dbpp;	/* data buffer descriptor ptr returned to caller */
ulong *asize;		/* buffer size causing allocb() failure */
bool frag_ok;		/* TRUE = partial messages receives are OK */
{
	mblk_t *header;		/* header structure */
	mblk_t *mptr;		/* allocated message ptr */
	struct dma_buf *dbp;		/* used to parse data descriptor list */
	ulong rem_size;		/* remaining length of message to be received */
	ulong frag_size = 0;	/* total size buffers allocated so far */
	ushort max_size;	/* maximum size of streams buffer allocated */
	ushort seg_cnt;		/* data segments in solicited send */

	*dbpp = (struct dma_buf *)NULL;
	rem_size = mps_msg_getbrlen(mbuf_p); 
	max_size = (otscfg.buffer_size) ? otscfg.buffer_size : SBLK4096;
	*asize = 0;

	DEBUGP(DEB_CALL,(CE_CONT, "O_alloc_dsolbuf: de=%x,mbuf_p=%x,dbpp=%x,frag_ok=%x\n",
			de, mbuf_p, dbpp, frag_ok));

	if ((header = allocb(sizeof(union data_header), BPRI_LO)) == NULL)
	{
		*asize = sizeof(union data_header);
		return(NULL);
	}

	while (  (rem_size > max_size)
	       &&(frag_size <= (otscfg.max_fragment - max_size))
	      )
	{
		if ((mptr = allocb((int)max_size, BPRI_LO)) == NULL)
		{
			*asize = max_size;
			if (  (frag_ok == FALSE)
			    ||(frag_size == 0)
			   )
			{
				freemsg(header);
				return(NULL);
			}
			else
				break;
		}
		else
		{
			linkb(header, mptr);
			mptr->b_rptr = mptr->b_datap->db_base;
			mptr->b_wptr = mptr->b_rptr + max_size;
			frag_size += max_size;
			rem_size -= max_size;
		}
	}
	if (  (rem_size)
	    &&(rem_size <= max_size)
	   )
	{
		if ((mptr = allocb((int)rem_size, BPRI_LO)) == NULL)
		{
			if (  (frag_ok == FALSE)
			    ||(frag_size == 0)
			   )
			{
				freemsg(header);
				*asize = rem_size;
				return(NULL);
			}
		}
		else
		{
			linkb(header, mptr);
			mptr->b_rptr = mptr->b_datap->db_base;
			mptr->b_wptr = mptr->b_rptr + rem_size;
			frag_size += rem_size;
			rem_size = 0;
		}
	}
	/*
	 * count segments
	 */
	mptr = header->b_cont;
	seg_cnt = 0;
	do
		seg_cnt++;
	while (mptr = mptr->b_cont);

	if ((*dbpp = mps_get_dmabuf(((int)seg_cnt + 1), DMA_NOSLEEP)) == NULL)
	{
		freemsg(header);
		ots_stat[ST_TKIDBLK]++;
		return(NULL);
	}

	DEBUGP(DEB_FULL,(CE_CONT, "  header=%x,seg_cnt=%x,size=%x,dbp=%x\n",
			header,seg_cnt,frag_size,*dbpp));

	/*
	 * initialize data descriptors
	 */
	for (mptr = header->b_cont, dbp = *dbpp;
	     seg_cnt != 0;
	     mptr = mptr->b_cont, seg_cnt--, dbp = dbp->next_buf
	    )
	{
		dbp->count = mptr->b_wptr - mptr->b_rptr;
		dbp->address = (ulong)kvtophys((caddr_t)mptr->b_rptr);
	}
	dbp->count = 0;
	dbp->next_buf = NULL;

	/*
	 * store away remaining bytes to be received in message buffer
	 */
	mps_msg_setbrlen(mbuf_p, rem_size);
	
	return(header);
}


#ifdef DEBUG

/* FUNCTION:		O_display_message(mbuf_p)
 *
 * ABSTRACT:	Displays MB-II message, used for debugging
 */
O_display_message(mbuf_p)

mps_msgbuf_t *mbuf_p;
{
	int i;

	for (i=0; i<mbuf_p->mb_count; i++)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, " %x", (uchar)mbuf_p->mb_data[i]));
	}
	DEBUGP(DEB_ERROR,(CE_CONT, "\n"));
}
#endif


/* FUNCTION:			O_put_unsol_queue()
 *
 * ABSTRACT:	Add unsolicited message to data entry's unsol queue
 *
 * ALGORITHM:
 *
 *	IF (queue full)
 *		output error message and dump message
 *	ELSE
 *		increment head pointer
 *		copy message into queue
 *		IF (queue went non-empty)
 *			IF (upstream blocked)
 *				schedule take routine via timeout
 *			ELSE	
 *				schedule take routine via bufcall()
 *			ENDIF
 *		ELSE IF (queue went full)
 *			mark queue full
 *		ENDIF
 *	ENDIF
 *
 * CALLED FROM:	O_data_unsol() - runs at interrupt time
 */
void
O_put_unsol_queue(de, mbuf_p, dsoc)

connect *de;		/* receiving data connection entry */
mps_msgbuf_t *mbuf_p;	/* message to be queued */
mb2socid_t dsoc;	/* requesting socket */
{
	int pri;
	void O_take_unsol_queue();

	pri = SPL();

	if (Q_FULL(de))
	{
		cmn_err(CE_WARN,
			"SV-ots: queue full; can't save unsol from socket %x\n",
			dsoc);
	}
	else
	{
		if (  (Q_EMPTY(de))
		    &&(  ((canput(de->ep->rd_q)) == FALSE)
		       ||(bufcall(sizeof(union qm), BPRI_LO,
				O_take_unsol_queue, de) == 0)
		      )
		   )
			timeout(O_take_unsol_queue, de, (int)DUBR_INIT(de));
			
		bcopy((char *)&mbuf_p->mb_data[MPS_MG_UMUD], QHEAD(de),
			OTS_QUEUE_SIZE);
		de->skt.data.rcnt--;

		QHEAD_INC(de);

		if (QTAIL(de) == QHEAD(de))
			de->state |= S_QUEUE_FULL;
	}
	splx(pri);
}


/* FUNCTION:			O_take_unsol_queue()
 *
 * ABSTRACT:	Empty data entry's unsol queue, if possible
 *
 *	Once queue is emptied, the routine processes any additional messages
 *	which may have arrived such as a solicited buffer request or
 *	disconnect.
 *
 * ALGORITHM:
 *
 *	ensure endpoint still around
 *	WHILE (unsol queue NOT empty)
 *		IF (upstream still blocked)
 *			schedule routine to be recalled via timeout()
 *			RETURN;
 *		ENDIF
 *		IF (can't allocate buffers to send message upstream)
 *			schedule routine to be recalled [via bufcall()]
 *			break
 *		ELSE
 *			build and send message upstream
 *			increment endpoint's data socket qlen count
 *			increment queue tail
 *				reset FULL flag
 *			ENDIF
 *		ENDIF
 *	ENDWHILE
 *	IF (unsol queue now empty)
 *		IF (disconnect indication queued)
 *			send indication upstream
 *			RETURN;
 *		ELSE IF (buffer request queued up behind unsols)
 *			process buffer request [O_data_snf()]
 *	ENDIF
 *	IF (local qlen > 1 AND transaction pending)
 *		send transaction response
 *	ENDIF
 *
 * INPUTS:	data connection entry
 *
 * CALLED FROM: STREAMS scheduler via bufcall()
 *		timeout()
 */
void
O_take_unsol_queue(de)

connect *de;
{
	endpoint *ep;			/* local endpoint */
	mblk_t *header;			/* TLI header */
	mblk_t *data;			/* data message block */
	struct o_unsolicited_data *ud;	/* data transfer unsol message */
	int pri;			/* saved interrupt priority */
	ushort size;			/* bufsize allocb() failed on */

	pri = SPL();

	DEBUGP(DEB_CALL,(CE_CONT, "O_take_unsol_queue: de=%x\n",de));

	if ((ep = de->ep) == NULL)	/* if endpoint is gone, forget it */
		return;

	while (Q_EMPTY(de) == FALSE)
	{
		if ((canput(ep->rd_q)) == FALSE)
		{
			timeout(O_take_unsol_queue, de, (int)DUBR_RETRY(de));
			return;
		}
		ud = (struct o_unsolicited_data *)QTAIL(de);
		header = (mblk_t *)NULL;
		if (  ((header = allocb(sizeof(union qm), BPRI_LO)) == NULL)
		    ||((data = allocb((int)ud->dlen, BPRI_LO)) == NULL)
		   )
		{
			if (header)
			{
				freemsg(header);
				size = sizeof(union qm);
			}
			else
				size = ud->dlen;

		    	if (bufcall(size, BPRI_LO, O_take_unsol_queue, de) == 0)
				cmn_err(CE_WARN,"SV-ots: O_take_unsol_queue bufcall failed\n");
			break;
		}
		else	/* buffers found for unsol, pass to TLI */
		{
			data->b_rptr = data->b_datap->db_base;
			data->b_wptr = data->b_rptr + ud->dlen;
			bcopy(QTAIL(de), (char *)data->b_rptr, (int)ud->dlen);
			linkb(header, data);
			switch (ud->type)
			{
			case O_DATA:
				iTLI_data_ind(ep, header, TRUE);
				break;
			case O_EOM_DATA:
				iTLI_data_ind(ep, header, FALSE);
				break;
			case O_EXPEDITED_DATA:
				iTLI_exdata_ind(ep, header, FALSE);
				break;
			default:
				DEBUGP(DEB_ERROR,(CE_CONT, "O_put_unsol_queue: bad command %x\n",
						ud->type));
				freemsg(header);
				break;
			}
			de->skt.data.rcnt++;
			de->state &= ~S_QUEUE_FULL;
			QTAIL_INC(de);
		}
	}
	if (Q_EMPTY(de))
	{
		if (ep->discon_ind)
		{
			if (ep->discon_ind == T_ORDREL_IND)
				iTLI_ordrel_ind(ep, ep->ddata, 0);
			else
			{
				iTLI_discon_ind(ep, (mblk_t *)NULL,
					ep->reason, de->port, ep->ddata);
				M_free_connect(de);
			}
			ep->ddata = 0;
			ep->discon_ind = 0;
			return;
		}
		else if (de->skt.data.breq)
		{
			DSBR_INIT(de);
			O_data_snf(de);
			return;
		}
	}
	if (  (de->skt.data.rdtid)
	    &&(de->skt.data.rcnt > 1)
	   )
		O_transaction_rsp(de, E_OK);

	splx(pri);
}


/* FUNCTION:			otsinit()
 *
 * ABSTRACT:	Initialize driver structures
 *
 * CALLED BY:	kernel during system initialization
 */
otsinit()
{
	connect *ce;
	unsigned int i;

	/*
	 * Initialize endpoint table
	 */
	for (i = 0; i < otscfg.n_endpoints; i++)
	{
		ots_endpoints[i].str_state = C_IDLE;
		ots_endpoints[i].tli_state = 0;
	}

	/*
	 * Initialize connection table and associated ptrs
	 */
	for (i = 0, ce = ots_connects ; i < otscfg.n_connects; i++, ce++)
	{
		ce->type = CT_FREE;
		ce->next = (connect *) (ce + 1);
		ce->index = i;
	}
	ots_connects[otscfg.n_connects].next = (connect *) NULL;
	M_c_free = ots_connects;
	M_c_alloc = (connect *) NULL;
	M_l_alloc = (connect *) NULL;

	/*
	 * Initialize data port free pool and associated ptrs
	 */
	if (otscfg.first_port == 0)			/* port 0 reserved */
	{
		otscfg.first_port = 1;
		otscfg.n_ports--;
	}
	for (i = 0; i < otscfg.n_ports; i++)
		ots_p_index[i] = (D_FREE | (i+1));
	ots_p_index[otscfg.n_ports] = 0x7FFF;		/* mark end of pool */
	M_p_free = 0;

	/*
	 * ensure configuration parameters don't exceed driver maximums
	 */
	otscfg.max_pend = min(otscfg.max_pend, AM_MAX_PEND);
	otscfg.addr_size = min(otscfg.addr_size, AM_ADDR_SIZE);
	otscfg.opts_size = min(otscfg.opts_size, AM_OPTS_SIZE);
	otscfg.tsdu_size = min(otscfg.tsdu_size, AM_TSDU_SIZE);
	otscfg.etsdu_size = min(otscfg.etsdu_size, AM_ETSDU_SIZE);
	otscfg.cdata_size = min(otscfg.cdata_size, AM_CDATA_SIZE);
	otscfg.ddata_size = min(otscfg.ddata_size, AM_DDATA_SIZE);
	otscfg.datagram_size = min(otscfg.datagram_size, AM_DATAGRAM_SIZE);
	otscfg.buffer_size = min(otscfg.buffer_size, AM_BUFFER_SIZE);
	otscfg.queue_len = min(otscfg.queue_len, AM_QUEUE_LEN);

	/*
	 * ensure configured default endpoint options are reasonable
	 */
	otscfg.vc_defaults &= ~OPT_CLTS;
	otscfg.vc_defaults |= OPT_COTS;
	otscfg.dg_defaults &= ~(OPT_COTS | OPT_EXP | OPT_ORD);
	otscfg.dg_defaults |= OPT_CLTS;

	/*
	 * compute size of STREAMS buffer needed for unsol queue
	 */
	if (otscfg.queue_len > 1)
		ots_queue_size = otscfg.queue_len * OTS_QUEUE_SIZE;
	else
		ots_queue_size = 0;

	/*
	 * ensure bufsize not larger than largest stream buffer
	 */
	if (otscfg.buffer_size > SBLK4096)
		otscfg.buffer_size = SBLK4096;

	ots_rinfo.mi_hiwat = otscfg.rd_hiwat;
	ots_rinfo.mi_lowat = otscfg.rd_lowat;
	ots_winfo.mi_hiwat = otscfg.wr_hiwat;
	ots_winfo.mi_lowat = otscfg.wr_lowat;

#ifdef V_3 /* ### RJF */
	/*
	 * compute maximum message fragment size if configured as zero
	 *
	 *	size = (# 4K buffers) / (# endpoints)
	 */
	if (otscfg.max_fragment == 0)
		otscfg.max_fragment = (v.v_nblk4096 / otscfg.n_endpoints) * SBLK4096;
#endif

	if (otscfg.max_fragment < SBLK4096)
		otscfg.max_fragment = SBLK4096;

	/*
	 * find otsdg driver's minor number so we can recognize during
	 * otsopen() when users want datagram services.  If no datagram
	 * driver is found, use all endpoints for VC's.
	 */
	otsdg_major = cdevcnt;
	for (i = 0; i < cdevcnt; i++)
	{
		if (strcmp(OTSDG_NAME, cdevsw[i].d_name) == 0)
		{
			otsdg_major = i;
			break;
		}
	}
	if (otsdg_major == cdevcnt)
	{
		otscfg.n_vcs = otscfg.n_endpoints - 1;
		otscfg.n_dgs = 0;
	}

	cmn_err(CE_CONT, "OTS Driver Initialized: %s\b.\n", (char *)(ots_rcsid+10));
}
