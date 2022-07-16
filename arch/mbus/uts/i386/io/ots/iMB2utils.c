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

#ident	"@(#)mbus:uts/i386/io/ots/iMB2utils.c	1.3"

/*
** ABSTRACT:	Mid-level assist routines
**
**	These functions are used to maintain the driver's main data structures
**	defined in sys/ots.h, especially the connection table and an
**	index and various lists used to lookup elements in this table.
**
**	The "port index" is an array of shorts.  Each element of the array
**	represents one of the MB-II port ids reserved for use by the driver
**	at configuration time.  The port represented by each element is
**	the sum of the element's array index and the first reserved port id.
**	Stored in the element is the index in the connection table of the
**	associated connection entry.  So finding the endpoint targeted
**	by a message received on a data socket involves a simple look-up
**	operation.  Elements representing unused ports are marked free by
**	setting bit 15; bits 0 thru 14 are used to link the free elements
**	by port index number.  This free list is headed by M_p_free and the
**	value 0x7FFF marks the last free entry.
**
**	Other lists manipulated by routines in this module link
**	the following kinds of elements:
**
**	1) free connection entries
**		This list is headed by M_c_free; elements are linked via the
**		connection entry next field.
**
**	2) control entries
**		This list is headed by M_c_alloc; elements are linked via the
**		connection entry next field.
**
**	3) local data entries
**		This list is headed by M_l_alloc; elements are linked via the
**		connection entry next field.
**
**	4) pending data entries
**		This list is headed by the control entry's pending field.
**		Entries are linked via the pending field.
**
**	5) endpoints associated with control entry
**		This list is headed by the control entry's endpoint field.
**		Endpoint entries are linked via the endpoint next field.
**
** MODIFICATIONS:
*/

#include "sys/ots.h"
#include <sys/cmn_err.h>

extern int ots_debug;			/* global defined in ots.c */

extern connect ots_connects[];		/* globals defined in Space.c */
extern endpoint ots_endpoints[];
extern ushort ots_p_index[];
extern struct otscfg otscfg;
extern ushort ots_resetting;

extern connect *M_c_alloc;		/* globals defined in otsutils.c */
extern connect *M_l_alloc;
extern connect *M_c_free;
extern ushort M_p_free;
extern ushort ots_queue_size;

extern unchar ics_myslotid();			/* kernel global */


/* FUNCTION:			M_alloc_connect()
 *
 * ABSTRACT:	Allocates a connection table entry
 *
 *	If type is data and endpoint supports a queue, this routine allocates
 *	a streams buffer for the queue and initializes the queue pointers.
 *
 * RETURNS:	ptr to initialized table entry or NULL if table full
 */
connect *
M_alloc_connect(port, type)

ushort port;
char type;
{
	connect *new;
	mblk_t *queue = NULL;
	int pri;

	DEBUGC('!');
	DEBUGP(DEB_CALL,(CE_CONT, "M_alloc_connect: port=%x, type=%x\n", port, type));

	if (  (type == CT_DATA)
	    &&(ots_queue_size)
	    &&((queue = allocb((int)ots_queue_size, BPRI_HI)) == NULL)
	   )
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "M_alloc_connect: queue alloc failed\n"));
		return((connect *)NULL);
	}

	pri = SPL();
	if (new = M_c_free)
	{
		M_c_free = new->next;
		if ((new->type = type) == CT_CONTROL) 
		{
			new->next = M_c_alloc;
			M_c_alloc = new;
		}
		else if (new->type == CT_LOCAL)
		{
			new->next = M_l_alloc;
			M_l_alloc = new;
		}
		else
		{
			new->next = NULL;
			if (new->skt.data.queue = queue)
			{
				new->skt.data.qtail = (char *)queue->b_rptr;
				new->skt.data.qhead = (char *)queue->b_rptr;
				queue->b_wptr = queue->b_rptr + ots_queue_size;
				new->skt.data.rcnt = ots_queue_size / OTS_QUEUE_SIZE;
			}
		}
		new->port = port;
		splx(pri);
		return(new);
	}
	else
	{
		splx(pri);
		return((connect *)NULL);
	}
}

/* FUNCTION:			M_alloc_port()
 *
 * ABSTRACT:	Fetches a port id from the free pool
 *
 * RETURNS:	non-zero: ID of allocated port
 *		0:	  No ports left in free pool
 */
ushort
M_alloc_port(index)

ushort index;	/* index to connection table of referenced entry */
{
	ushort new;
	int pri;

	DEBUGC('@');
	DEBUGP(DEB_CALL,(CE_CONT, "M_alloc_port: index=%x\n", index));

	pri = SPL();
	if ((new = M_p_free) != 0x7FFF)
	{
		ots_p_index[new] &= ~D_FREE;
		M_p_free = ots_p_index[new];
		ots_p_index[new] = index;
		splx(pri);
		return(new + otscfg.first_port);
	}
	else
	{
		splx(pri);
		return(0);		/* no free ports left */
	}
}

/* FUNCTION:		M_add_endpoint()
 *
 * ABSTRACT:	Add endpoint to tail end of control's endpoint list
 */
void
M_add_endpoint(ce, ep)

connect *ce;
endpoint *ep;
{
	int pri;
	endpoint *current;

	DEBUGC(',');
	DEBUGP(DEB_CALL,(CE_CONT, "M_add_endpoint: ce=%x, ep=%x\n", ce, ep));

	pri = SPL();
	if (current = ce->ep)
	{
		while (current->next)
			current = current->next;
		current->next = ep;
	}
	else
		ce->ep = ep;

	ep->control = ce;
	ep->next = NULL;
	splx(pri);
}

/* FUNCTION:		M_add_pending()
 *
 * ABSTRACT:	Add data entry to control's pending list
 */
void
M_add_pending(ce, de)

connect *ce;
connect *de;
{
	int pri;

	DEBUGC('#');
	DEBUGP(DEB_CALL,(CE_CONT, "M_add_pending: ce=%x, de=%x\n", ce, de));

	pri = SPL();
	de->pending = ce->pending;
	ce->pending = de;
	de->state |= S_PENDING;
	splx(pri);
}


/* FUNCTION:			M_free_connect()
 *
 * ABSTRACT:	Returns connection table entry to free list
 *
 *	This routine may be called multiple times before an entry is
 *	completely freed.  Such a situation arises when outstanding
 *	transactions exist on the socket referenced by the connection
 *	entry.  These transactions are canceled the first time through
 *	this routine but the mps_close_chan() will fail if these transactions
 *	were initiated by us because the transaction id's were never freed.
 *	TKI will eventually return to the driver the message block that
 *	initiated the transaction and the STREAMs resources associated with
 *	the message.  The tid and STREAMs resources will be freed and this
 *	routine will be called again to complete the freeing of the connection
 *	entry.
 *
 * NOTE:
 *
 *   1)	nbr_datarq is reset here so we won't block on the downstream
 *	queue in iTLIwsrv.  The pending send buffer will be returned
 *	by TKI with an error.
 */
void
M_free_connect(entry)

connect *entry;		/* connection entry being freed */
{
	connect *prev;
	connect **head;
	int pri;

	DEBUGC('$');
	DEBUGP(DEB_CALL,(CE_CONT, "M_free_connect: entry=%x\n", entry));

	pri = SPL();

	if (entry->type == CT_FREE)
	{
		DEBUGP(DEB_ERROR,(CE_CONT, "M_free_connect: entry already freed\n"));
		splx(pri);
		return;
	}
	else if (entry->type == CT_DATA)	/* clear de resources */
	{
		if (entry->skt.data.queue)
		{
			freemsg(entry->skt.data.queue);
			entry->skt.data.queue = NULL;
		}
		if (entry->skt.data.rdtid)
		{
			DEBUGP(DEB_FULL,(CE_CONT, "M_free_connect: canceling rdtid=%x\n",
				entry->skt.data.rdtid));
			if (mps_AMPcancel(entry->channel, entry->skt.data.rem_data,
				entry->skt.data.rdtid))
				cmn_err(CE_WARN,"SV-ots: can't cancel rdtid");
			else
				entry->skt.data.rdtid = 0;
		}

		if (entry->skt.data.xdtid)
		{
			DEBUGP(DEB_FULL,(CE_CONT, "M_free_connect: canceling xdtid=%x\n",
				entry->skt.data.xdtid));
			if (mps_AMPcancel(entry->channel,
				mps_mk_mb2socid(ics_myslotid(), entry->port),
				entry->skt.data.xdtid))
				cmn_err(CE_WARN,"SV-ots: can't cancel xdtid");
			else
				entry->skt.data.xdtid = 0;
		}
		if (entry->skt.data.breq);
		{
			mps_free_msgbuf(entry->skt.data.breq);
			entry->skt.data.breq = NULL;
		}
	}
	else if (entry->type == CT_LOCAL)
	{
		if (entry->skt.local.udata)
		{
			freemsg(entry->skt.local.udata);
			entry->skt.local.udata = NULL;
		}
		if (entry->skt.local.odata)
		{
			freemsg(entry->skt.local.odata);
			entry->skt.local.odata = NULL;
		}
	}

	if (entry->ep)		/* disassociate from endpoint */
	{
		if (entry->state & S_PENDING)
			M_remove_pending(entry->ep->control, entry->port);

		if (entry->type == CT_CONTROL)
			entry->ep->control = NULL;
		else if (entry->type == CT_DATA)
		{
			entry->ep->nbr_datarq = 0;	/* NOTE 1 */
			entry->ep->data = NULL;
		}
		else
			entry->ep->data = NULL;
		entry->ep = NULL;
	}

	if (  (entry->type != CT_LOCAL)			/* close channel */
	    &&(mps_close_chan((long)entry->channel) < 0)
	   )
	{
		DEBUGP(DEB_FULL,(CE_CONT, "M_free_connect: mps_close_chan %d failed\n",
				entry->channel));
		splx(pri);
		return;
	}

	/* remove from control/local allocated list */
	if (entry->type != CT_DATA)
	{
		head = (entry->type == CT_CONTROL) ? &M_c_alloc : &M_l_alloc;
		if ((prev = *head) == entry)
			*head = entry->next;
		else
		{
			while (prev->next != entry)
				if ((prev = prev->next) == NULL)
					cmn_err(CE_PANIC,"M_free_connect: can't find connection entry\n");
			prev->next = entry->next;
		}
	}

	if (  (entry->type != CT_CONTROL)	/* free port entry */
	    ||(entry->state & S_ALLOCPORT)
	   )
	{
		M_free_port(entry->port);
		entry->port = 0;
	}

	entry->next = M_c_free;		/* add to free list */
	M_c_free = entry;
	entry->type = CT_FREE;

	entry->state = 0;
	entry->port = 0;
	entry->pending = NULL;
	bzero(&entry->skt, sizeof(entry->skt));
	splx(pri);
}


/* FUNCTION:			M_free_port()
 *
 * ABSTRACT:	Returns port id "port" to free pool
 */
void
M_free_port(port)

ushort port;
{
	ushort index;
	ushort save;
	int pri;

	DEBUGC('%');
	DEBUGP(DEB_CALL,(CE_CONT, "M_free_port: port=%x\n", port));

	pri = SPL();
	save = M_p_free;
	index = port - otscfg.first_port;
	M_p_free = index;
	if ((ots_p_index[index] = save) != 0x7FFF)
		ots_p_index[index] |= D_FREE;
	splx(pri);
}


/* FUNCTION:			M_find_c()
 *
 * ABSTRACT:	Returns ptr to new or existing control connection entry
 *
 * INPUTS:	control port id
 *		action flag:	M_CT_ALLOC - allocate new entry if NOT found
 *				M_CT_FIND  - return error if entry NOT found
 *
 * RETURNS:	ptr to table entry or NULL if table full or entry NOT found
 */
connect *
M_find_c(cport, flag)
ushort cport;
ushort flag;
{
	connect *tmp;
	int pri;

	DEBUGC('^');
	DEBUGP(DEB_CALL,(CE_CONT, "M_find_c: cport=%x, flag=%x\n", cport, flag));

	pri = SPL();
	tmp = M_c_alloc;
	while (  (tmp != NULL)
	       &&(tmp->port != cport)
	      )
		tmp = tmp->next;
	splx(pri);
	if (  (tmp != NULL)
	    ||(flag == M_CT_FIND)
	   )
		return(tmp);
	else
		return(M_alloc_connect(cport,CT_CONTROL));
}


/* FUNCTION:			M_find_d()
 *
 * ABSTRACT:	Returns ptr to existing data connection entry
 *
 * RETURNS:	ptr to table entry or NULL if entry NOT found
 */
connect *
M_find_d(port)

ushort port;
{
	ushort index;
	int pri;

	DEBUGC('&');
	DEBUGP(DEB_CALL,(CE_CONT, "M_find_d: port=%x\n", port));

	pri = SPL();
	if (  ((index = port - otscfg.first_port) > otscfg.n_ports)
	    ||((index = ots_p_index[index]) & D_FREE)
	    ||(index == 0x7FFF)
	   )
	{
		splx(pri);
		return((connect *)NULL);
	}
	else
	{
		splx(pri);
		return((connect *)&ots_connects[index]);
	}
}

/* FUNCTION:			M_find_listener()
 *
 * ABSTRACT:	Return ptr to control entry's first listening endpoint
 */
endpoint *
M_find_listener(ce)

connect *ce;
{
	endpoint *ep;

	DEBUGC('&');
	DEBUGP(DEB_CALL,(CE_CONT, "M_find_listener: ce=%x\n", ce));

	ep = ce->ep;
	while (ep != NULL)
	{
		if (  ((ep->max_pend - ep->nbr_pend) != 0)
		    &&(  (ep->tli_state == TS_IDLE)
		       ||(ep->tli_state == TS_WRES_CIND)
		      )
		   )
			return(ep);
		else
			ep = ep->next;
	}
	return(ep);
}


/* FUNCTION:			M_find_dgendpoint()
 *
 * ABSTRACT:	Returns ptr to first endpoint able to receive datagrams
 */
endpoint *
M_find_dgendpoint(ce)

connect *ce;
{
	endpoint *ep;

	DEBUGC('&');
	DEBUGP(DEB_CALL,(CE_CONT, "M_find_dgendpoint: ce=%x\n", ce));

	ep = ce->ep;
	while (ep != NULL)
	{
		if (  (ep->options & OPT_CLTS)
		    &&(ep->tli_state == TS_IDLE)
		   )
			return(ep);
		else
			ep = ep->next;
	}
	return(ep);
}


/* FUNCTION:		M_find_pending()
 *
 * ABSTRACT:	return pending data entry associated with remote port id
 */
connect *
M_find_pending(ce, rport)

connect *ce;
ushort rport;
{
	connect *de;
	int pri;

	DEBUGP(DEB_CALL,(CE_CONT, "M_find_pending: ce=%x, rport=%x\n", ce, rport));

	pri = SPL();
	if (de = ce->pending)
	{
		do
		{
			if (  (de->type == CT_DATA)
			    &&(mps_mk_mb2soctopid(de->skt.data.rem_data) == rport)
			   )
			{
				splx(pri);
				return(de);
			}
			else if (  (de->type == CT_LOCAL)
				 &&(de->skt.local.dest_data->port == rport)
				)
			{
				splx(pri);
				return(de);
			}
		}
		while (de = de->pending);
	}
	splx(pri);
	return((connect *) NULL);
}


/* FUNCTION:		M_find_rctid_ep()
 *
 * ABSTRACT:	return endpoint associated with remote tid
 */
endpoint *
M_find_rctid_ep(ce, tid, rsoc, count)

connect *ce;
uchar tid;
mb2socid_t rsoc;
int *count;
{
	endpoint *ep;

	if (count) *count = 0;
	ep = ce->ep;

	while (ep)
	{
		if (count) *count++;

		if (  (ep->rctid == tid)
		    &&(ep->rsoc == rsoc)
		   )
			return(ep);
		ep = ep->next;
	}
	return((endpoint *)NULL);
}


/* FUNCTION:		M_find_xctid_ep()
 *
 * ABSTRACT:	return endpoint associated with local tid
 */
endpoint *
M_find_xctid_ep(ce, tid, count)

connect *ce;
uchar tid;
int *count;
{
	endpoint *ep;

	if (count) *count = 0;
	ep = ce->ep;

	while (ep)
	{
		if (count) *count++;

		if (ep->xctid == tid)
			return(ep);
		ep = ep->next;
	}
	return((endpoint *)NULL);
}


/* FUNCTION:		M_remove_endpoint()
 *
 * ABSTRACT:	Find and remove endpoint from control's endpoint list
 */
void
M_remove_endpoint(ce, ep)

connect *ce;		/* control entry */
endpoint *ep;		/* endpoint to remove from control's endpoint list */
{
	endpoint *current;
	int pri;

	DEBUGC(',');
	DEBUGP(DEB_CALL,(CE_CONT, "M_remove_endpoint: ce=%x, ep=%x\n", ce, ep));

	pri = SPL();

	/*
	 * first cancel any active transactions
	 */
	if (ep->xctid)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "M_remove_endpoint: canceling xctid=%x\n",
				ep->xctid));
		if (mps_AMPcancel(ep->control->channel,
			mps_mk_mb2socid(ics_myslotid(), ep->control->port), ep->xctid))
			cmn_err(CE_WARN,"SV-ots: can't cancel xctid");
		else
			ep->xctid = 0;
	}
	if (ep->rctid)
	{
		DEBUGP(DEB_FULL,(CE_CONT, "M_remove_endpoint: canceling rctid=%x\n",
			ep->rctid));
		if (mps_AMPcancel(ep->control->channel, ep->rsoc, ep->rctid))
			cmn_err(CE_WARN,"SV-ots: can't cancel rctid");
		else
			ep->rctid = 0;
	}
	if (ep->ddata)
	{
		freemsg(ep->ddata);
		ep->ddata = NULL;
	}
	if (ep->breq)
	{
		mps_free_msgbuf(ep->breq);
		ep->breq = NULL;
	}
	/*
	 * Now remove from list
	 */
	if ((current = ce->ep) == ep)
		ce->ep = ep->next;
	else
	{
		while (current->next != ep)
			current = current->next;
		current->next = ep->next;
	}
	ep->control = NULL;
	ep->next = NULL;
	splx(pri);
}


/* FUNCTION:		M_remove_pending()
 *
 * ABSTRACT:	Remove data entry to control's pending list
 *
 * RETURNS:	pointer to removed data entry
 */
connect *
M_remove_pending(ce, port)

connect *ce;
ushort port;
{
	connect *previous;
	connect *current;
	int pri;

	DEBUGC('*');
	DEBUGP(DEB_CALL,(CE_CONT, "M_remove_pending: ce=%x, port=%x\n", ce, port));

	pri = SPL();
	if (  (current = ce->pending)
	    &&(current->port == port)
	   )
	{
		ce->pending = current->pending;
		current->state &= ~S_PENDING;
		splx(pri);
		return(current);
	}
	previous = current;
	while (previous != NULL)
	{
		if (previous->pending->port == port)
		{
			current = previous->pending;
			previous->pending = current->pending;
			current->state &= ~S_PENDING;
			splx(pri);
			return(current);
		}
		previous = previous->pending;
	}
	splx(pri);
	return((connect *) NULL);
}


/* FUNCTION:		iMB2_reset()
 *
 * ABSTRACT:	Recover any resources upon driver reset
 *
 *	Step through connection table cleaning entries, if not free.
 *
 * CALLED BY:	timeout() after delay allowing cleanup
 */
void
iMB2_reset()
{
	connect *cn;
	unsigned int i;

	for (i = 0, cn = ots_connects ; i < otscfg.n_connects; i++, cn++)
	{
		if (cn->type != CT_FREE)
			M_free_connect(cn);
	}
	ots_resetting = FALSE;
}

/* FUNCTION:		M_valid_options()
 *
 * ABSTRACT: Update endpoint options if input connection options are valid
 *
 * RETURNS:	TRUE - options valid
 *		FALSE - options invalid
 */
M_valid_options(ep, options)

endpoint *ep;
opts *options;
{
	DEBUGP(DEB_CALL,(CE_CONT, "M_valid_options: ep=%x,opts=%x,ep->opts=%x\n",
			ep, *options, ep->options));

	if (  (*options & ~otscfg.vc_defaults)
	    ||((*options & OPT_COTS) == FALSE)
	   )
		return(FALSE);
	else
	{
		ep->options = *options;
		return(TRUE);
	}
}
