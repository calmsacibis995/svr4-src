/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/enetdrv/edlina.c	1.3.3.1"

#ident "@(#)edlina.c  $SV_enet SV-Eval01 - 06/25/90$"

#include "sys/enet.h"
#include "sys/edlina.h"
#include <sys/socket.h>
#include <net/if.h>
#include <sys/kmem.h>
#include <sys/lihdr.h>
#include <sys/immu.h>
#ifdef V32
#include <sys/cmn_err.h>
#endif

static char edlina_rcsid[] = "@(#)Driver.o  $SV_enet R4.0 - 06/28/90$";
static char edlina_copyright[] = "Copyright 1989 Intel Corp. 464795";

/*
 *  The following are defined in Space.c
 */
extern int		edl_boards;		/* number of installed boards */
extern int 		edl_major_to_board[];	/* major dev to board mapping */
extern int 		edl_max_bufs_posted;	/* recive RB pool size */
extern struct_edlina_t	edl_board_struct[];	/* internal board/sap struct */

/*
 *  The following are defined in the associated enet driver.
 */
#ifdef V32
extern int		i552_debug;
extern int		n_endpoints;	/* number of endpoint structures */
extern endpoint		endpoints[];	/* endpoint structures */
extern struct i552inf	i552inform[];	/* board information structures */
#else
extern int		enet_debug;
extern int		enet_n_endpoints; /* number of endpoint structures */
extern endpoint		enet_endpoints[]; /* endpoint structures */
extern struct enetinf	enet_inform[];	/* board information structures */
#endif
extern int		mipsend();	/* sends RB returning TRUE/FALSE */

#ifndef V32
int edldevflag = 0;		/* V4.0 style driver */
#endif

static int	edlina_open(),
		edlina_close(),
		edlina_rput(),
		edlina_wput();

static int	dl_cmds(),
		dl_info_req(),
		dl_bind_req(),
		dl_unbind_req(),
		dl_data_req();

struct module_stat edlina_stats;
static struct module_info edlina_rminfo =
				{ 0, "edlina", 0, MAX_PACK_SIZE, 4096, 256 };
static struct module_info edlina_wminfo =
				{ 0, "edlina", 0, MAX_PACK_SIZE, 4096, 256 };

static struct qinit edlina_rinit = 
{edlina_rput, NULL, edlina_open, edlina_close, NULL, &edlina_rminfo, &edlina_stats};

static struct qinit edlina_winit =
{edlina_wput, NULL, NULL, NULL, NULL, &edlina_wminfo, &edlina_stats };

struct streamtab edlinfo = {&edlina_rinit, &edlina_winit, NULL, NULL};

/*
 * edlina debug level global variable:
 *	0 = DEB_NONE - show nothing
 *	1 = DEB_ERROR - show most error conditions
 *	2 = DEB_CALL - show only call entries and returns
 *	3 = DEB_FULL - show everything
 */
int edlina_debug = 1;


/*
 * Debug statistics:
 */
struct edlstats {
	ulong bufs_posted;
	ulong overrun_cnt;
	ulong xmt_lost;
	ulong rcv_lost;
	ulong rcv_errors; /* Never greater than edl_max_bufs_posted */
} edl_stats;

/*
 * inet interface statistics
 */
extern	char	*edl_ifname;
extern	int	edl_inetstats;
extern	struct	ifstats	*ifstats;
	struct	ifstats	*edlifstats;

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*              ROUTINES THAT INTERFACE TO THE STREAM HEAD                  */
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

/*
 *  edlinit():
 *
 *  Put all data structures in a known state.
 */
edlinit()
{
	int		x, y;
	struct_edlina_t	*p;

	/*
	 *  If inetstats are wanted, get an ifstats structure.
	 */
	if (edl_inetstats)
		edlifstats = (struct ifstats*)kmem_zalloc(
			(sizeof(struct ifstats) * edl_boards), KM_NOSLEEP);
	else
		edlifstats = 0;

	/*
	 *  Initialize internal data structures.
	 */
	for (x = 0, p = edl_board_struct; x < edl_boards; x++, p++)
	{
		bzero((char *)p, sizeof(struct_edlina_t));
		p->info.maj_dev = edl_major_to_board[ x ];

		for (y = 0; y < N_LSAP; y++)
		{
			p->sap[ y ].sap_state = DL_DEAD;
			p->sap[ y ].p_info    = &p->info;
		}

		/*
		 *  Initialize inet stats if we're keeping them.
		 */
		if (edlifstats)
		{
			edlifstats[ x ].ifs_name   = edl_ifname;
			edlifstats[ x ].ifs_unit   = (short)x;
			edlifstats[ x ].ifs_active = 0;
			edlifstats[ x ].ifs_next   = ifstats;
			edlifstats[ x ].ifs_mtu    = MAX_PACK_SIZE;
			ifstats = &edlifstats[ x ];
		}
	}
	cmn_err(CE_CONT, "EDLINA Driver Initialized: %s\b.\n",
						(char *)(edlina_rcsid+15));

	return;
}

/*
 *  edlina_open():
 */
#ifdef V32
static int
edlina_open(q, dev, flag, sflag)
queue_t	*q;		/* read q */
dev_t	dev;
int	flag;
int	sflag;
#else
edlina_open(q, dev, flag, sflag, credp)
queue_t	*q;		/* read q */
dev_t	*dev;
int	flag, sflag;
struct cred *credp;
#endif
{
#ifdef V32
	register int minor_dev;
#else
	minor_t minor_dev;
#endif
	int		oldpri, x, board;
	endpoint	*ep;
	struct_edlina_t	*p;
	struct connrb	*cr_rb;

	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_open()\n"));

	/*
	 *  Find the structure for the board they are using.
	 * for (p = edl_board_struct, board = 0; board < edl_boards; p++, board++)
	 *	if (p->info.maj_dev == major(dev))
	 *	{
	 *		found_board = TRUE;
	 *		break;
	 *	}
	 * 
	 * if (found_board == FALSE)
	 * 	return (OPENFAIL);
	 */
	/*
	 * For now, support only one board.
	 */
	p = edl_board_struct;
	board = 0;

#ifdef V32
	minor_dev = minor(dev);
#else
	minor_dev = getminor(*dev);
#endif
	/*
	 *  If this is a clone open, then select an available minor dev number.
	 */
	oldpri = splstr();

	if (sflag == CLONEOPEN)
	{
		for (minor_dev = 0; minor_dev < N_LSAP; minor_dev++)
			if (p->sap[ minor_dev ].sap_state == DL_DEAD)
				break;
		if (minor_dev == N_LSAP)
		{
			splx(oldpri);
			EDEBUGP(DEB_ERROR,(CE_CONT, "edlina_open: no free saps\n"));
#ifdef V32
			return (OPENFAIL);
#else
			return (EAGAIN);
#endif
		}
#ifdef V32
		dev = ((dev & 0xff00) | minor_dev);
#else
		*dev = makedevice(getmajor(*dev), minor_dev);
#endif
	}
	/*
	 *  Not a clone open so use the minor device number that was passed.
	 */
	else
	{
		/*
		 *  Validate the minor device number.
		 */
		if ((minor_dev < 0) || (minor_dev > (N_LSAP - 1)))
		{
			splx(oldpri);
			EDEBUGP(DEB_ERROR,(CE_CONT, "edlina_open: invalid minor device number\n"));
#ifdef V32
			return (OPENFAIL);
#else
			return (ECHRNG);
#endif
		}

		/*
		 *  See if the device was already opened.  If so just return.
		 */
		if (p->sap[ minor_dev ].sap_state != DL_DEAD)
		{
			splx(oldpri);
#ifdef V32
			return (minor_dev);
#else
			return (0);
#endif
		}
	}

	/*
	 *  If this is the first open, get an iNA endpoint structure and
	 *  initialize it.
	 */
	if (p->info.opens == 0)
	{
#ifdef V32
		for (x = 1, ep = &endpoints[x]; x < n_endpoints; x++, ep++)
			if (ep->str_state == C_IDLE)
				break;
		if (x == n_endpoints)
#else
		for (x=1, ep=&enet_endpoints[x]; x<enet_n_endpoints; x++, ep++)
			if (ep->str_state == C_IDLE)
				break;
		if (x == enet_n_endpoints)
#endif
		{

			cmn_err(CE_WARN, "EDLINA: No endpoint for open request\n");
			splx(oldpri);
#ifdef V32
			return (OPENFAIL);
#else
			return (ENOSPC);
#endif
		}

		EDEBUGP(DEB_FULL,(CE_CONT, "edlina_open: allocated ep = %x\n", ep));
		bzero((char *)ep, sizeof(endpoint));

		ep->tli_state		= TS_UNBND;
		ep->rd_q		= q;
		ep->str_state		= C_OPEN;
		ep->open_cont		= NULL;
		ep->bnum		= board;
		ep->req_seqno		= 0;
		ep->complete_seqno	= 0;
		ep->options		= OPT_EXCL;

		p->info.board_num	= board;
		p->info.p_ep		= ep;
#ifdef V32
		p->info.mac_addr	= i552inform[ board ].eaddr;
#else
		p->info.mac_addr	= enet_inform[ board ].eaddr;
#endif

		/*
		 *  Send a connect to iNA to let them know we want to use the
		 *  service.
		 */
		if ((cr_rb = (struct connrb *)getrb(ep)) == NULL)
		{
			cmn_err(CE_WARN, "EDLINA: No rb for connect request\n");
			ep->str_state = C_IDLE;
			splx(oldpri);
#ifdef V32
			return (OPENFAIL);
#else
			return (ENOSPC);
#endif
		}

		if (edlina_connect(ep, cr_rb) == FALSE)
		{
			cmn_err(CE_WARN, "EDLINA: Connect request failed\n");
			relrb(cr_rb);
			ep->str_state = C_IDLE;
			splx(oldpri);
#ifdef V32
			return (OPENFAIL);
#else
			return (ENOSPC);
#endif
		}
		/*
		 *  Re-initialize statistics.
		 */
		edl_stats.bufs_posted = 0;
		edl_stats.overrun_cnt = 0;
		edl_stats.xmt_lost    = 0;
		edl_stats.rcv_lost    = 0;
		edl_stats.rcv_errors  = 0;
	}

	/*
	 * Set up the stream head options.
	 */
	/* TBD */

	/*
	 *  Link the read and write queues.
	 */
	p->sap[ minor_dev ].read_q  = q;
	p->sap[ minor_dev ].write_q = WR(q);

	/*
	 *  p_ptr points to a sap_t -- this is our "endpoint" for this
	 *  instance of open.
	 */
	q->q_ptr     = (caddr_t)&p->sap[ minor_dev ];
	WR(q)->q_ptr = (caddr_t)&p->sap[ minor_dev ];

	p->sap[ minor_dev ].sap_state	= DL_UNBND;
	p->info.opens++;

	splx(oldpri);
	EDEBUGP(DEB_FULL,(CE_CONT, "edlina_open: Opened minor dev %x\n", minor_dev));
#ifdef V32
	return (minor_dev);
#else
	return (0);
#endif
}

/*
 *  edlina_close():
 */
static
edlina_close(q, flag)
queue_t	*q;		/* point to read q */
int	flag;		/* file open flags */
{
	sap_t		*p_sap;
	int		oldpri, board;
	struct discrb	*dr_rb;

	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_close()\n"));
	oldpri = splstr();

	/*
	 *  Clean up the minor device pointers.
	 */
	p_sap		 = (sap_t *)q->q_ptr;
	p_sap->sap	 = 0;
	p_sap->sap_state = DL_DEAD;
	p_sap->read_q	 = NULL;
	p_sap->write_q	 = NULL;

	/*
	 *  If this is the last close on the device, disconnect the
	 *  iNA endpoint.
	 */
	p_sap->p_info->opens--;
	if (!p_sap->p_info->opens)
	{
		/*
		 * The endpoint is returned on confirm
		 */
		if((dr_rb = (struct discrb*)getrb(p_sap->p_info->p_ep)) == NULL)
		{
			cmn_err(CE_WARN, "EDLINA: Could not allocate rb for disconnect.\n");
		}
		else if (edlina_disconnect(p_sap->p_info->p_ep, dr_rb) == FALSE)
		{
			cmn_err(CE_WARN, "EDLINA: Disconnect request failed.\n");
			relrb(dr_rb);
		}

		if (edlifstats)
		{
			board = p_sap->p_info->board_num;
			edlifstats[ board ].ifs_active     = 0;
			edlifstats[ board ].ifs_ipackets   = 0;
			edlifstats[ board ].ifs_ierrors    = 0;
			edlifstats[ board ].ifs_opackets   = 0;
			edlifstats[ board ].ifs_oerrors    = 0;
			edlifstats[ board ].ifs_collisions = 0;
		}
	}

	splx(oldpri);
	return;
} 

/*
 *  edlina_rput():
 */
static
edlina_rput(q, mp)
queue_t	*q;
mblk_t	*mp;
{
	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_rput()\n"));

	/*
	 *  canput() was already called by edlina_rawrecv_complete().
	 *  canput() is not called by dl_...() so as to make the design simpler.
	 */
	putnext(q, mp);
	return;
}

/*
 *  edlina_wput():
 */
static
edlina_wput(q, mp)
queue_t	*q;       /* write q */
mblk_t	*mp;
{
	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_wput()\n"));

	/*
	 *  Switch on the data block type.
	 */
	switch (mp->b_datap->db_type)
	{
	default:
		freemsg(mp);
		break;

	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			flushq(q, FLUSHDATA);

		if (*mp->b_rptr & FLUSHR)
		{
			flushq(RD(q), FLUSHDATA);
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
		}
		else
			freemsg(mp);
		break;

	case M_IOCTL:
		mp->b_datap->db_type = M_IOCNAK;
		qreply(q, mp);
		break;

	case M_PROTO:
	case M_PCPROTO:
		dl_cmds(q, mp);
		break;
	} 

	return;
}

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*        ROUTINES THAT IMPLEMENT THE LOGICAL LINK INTERFACE SPEC            */
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

/*
 *  dl_cmds():
 */
static
dl_cmds(q, mp)
queue_t *q;       /* write q */
mblk_t	*mp;
{
	union DL_primitives	*p_dl;
	mblk_t			*response;
	DL_error_ack_t		*p_error;

	EDEBUGP(DEB_CALL,(CE_CONT, "dl_cmds()\n"));
	p_dl = (union DL_primitives *)mp->b_datap->db_base;

	/*
	 *  Switch on the data link primative type.
	 */
	switch((int)p_dl->prim_type)
	{
	case DL_INFO_REQ:
		dl_info_req(q, mp);
		break;

	case DL_BIND_REQ:
		dl_bind_req(q, mp);
		break;

	case DL_UNBIND_REQ:
		dl_unbind_req(q, mp);
		break;

	case DL_UNITDATA_REQ:
		dl_data_req(q, mp);
		break;

	default:
		if ((response = allocb(DL_PRIMITIVES_SIZE, BPRI_MED)) == NULL)
		{
			freemsg(mp);
			return;
		}

		p_error			=(DL_error_ack_t *)response->b_wptr;
		p_error->PRIM_type	= DL_ERROR_ACK;
		p_error->ERROR_prim	= p_dl->prim_type;
		p_error->LLC_error	= DLBADSAP;
		p_error->UNIX_error	= 0;

		response->b_wptr += DL_ERROR_ACK_SIZE;

		edlina_rput(RD(q), response);
		freemsg(mp);
		break;
	}

	return;
}

/*
 *  dl_info_req():
 */
static
dl_info_req(q, mp)
queue_t *q;       /* write q */
mblk_t	*mp;
{
	mblk_t		*response;
	DL_info_ack_t	*p_info_ack;
	sap_t		*p_sap;
	int		oldpri;

	EDEBUGP(DEB_CALL,(CE_CONT, "dl_info_req()\n"));

	/*
	 *  Free message blocks since we don't need them.
	 */
	freemsg(mp);

	/*
	 *  Allocate memory for the response.
	 */
	if ((response = allocb(DL_PRIMITIVES_SIZE, BPRI_MED)) == NULL)
		return;

	oldpri = splstr();

	/*
	 *  Give them the info they requested.
	 */
	p_sap = (sap_t *)q->q_ptr;

	p_info_ack		  = (DL_info_ack_t *)response->b_wptr;
	p_info_ack->PRIM_type     = DL_INFO_ACK;
	p_info_ack->SDU_max       = MAX_PACK_SIZE;
	p_info_ack->SDU_min       = 46;
	p_info_ack->ADDR_length   = MAC_ADD_SIZE;
	p_info_ack->SUBNET_type   = DL_ETHER;
	p_info_ack->SERV_class    = DL_NOSERV;
	p_info_ack->CURRENT_state = (long)(p_sap->sap_state);

	response->b_datap->db_type  = M_PCPROTO;
	response->b_wptr	   += DL_INFO_ACK_SIZE;

	splx(oldpri);
	edlina_rput(RD(q), response);
	return;
}

/*
 *  dl_bind_req():
 */
static
dl_bind_req(q, mp)
queue_t *q;       /* write q */
mblk_t	*mp;
{
	DL_bind_req_t	*p_dl;
	sap_t		*p_sap;
	mblk_t		*response;
	DL_error_ack_t	*p_error;
	DL_bind_ack_t	*p_bind;
	ushort		*p_mac_addr, *p_mac_addr2;
	int		board, x, oldpri;
	long		requested_sap;

	EDEBUGP(DEB_CALL,(CE_CONT, "dl_bind_req()\n"));

	/*
	 *  Allocate memory for the response.
	 */
	if ((response = allocb(DL_PRIMITIVES_SIZE, BPRI_MED)) == NULL)
	{
		freemsg(mp);
		return;
	}

	p_dl = (DL_bind_req_t *)mp->b_datap->db_base;
	requested_sap = p_dl->LLC_sap;
	EDEBUGP(DEB_FULL,(CE_CONT, "dl_bind_req: requested_sap = %x\n", requested_sap));

	freemsg(mp);
	oldpri = splstr();

	/*
	 *  q->q_ptr set by edlina_open, points to the sap_t that is "owned" by
	 *  this queue
	 */
	p_sap = (sap_t *)(q->q_ptr);

	/*
	 *  Switch on the current SAP state.
	 */
	switch (p_sap->sap_state)
	{
	case DL_UNBND:
		board = p_sap->p_info->board_num;
		for (x = 0; x < N_LSAP; x++)
		{
			/*
			 *  See if this SAP was already - would be an error.
			 */
			if ((edl_board_struct[board].sap[x].sap == requested_sap)&&
			    (edl_board_struct[board].sap[x].sap_state == DL_IDLE))
			{
				splx(oldpri);

				p_error	 = (DL_error_ack_t *)response->b_wptr;
				p_error->PRIM_type	= DL_ERROR_ACK;
				p_error->ERROR_prim	= DL_BIND_REQ;
				p_error->LLC_error	= DLBADSAP;
				p_error->UNIX_error	= 0;

				response->b_wptr	  += DL_ERROR_ACK_SIZE;
				response->b_datap->db_type = M_PCPROTO;

				edlina_rput(RD(q), response);
				return;
			}
		}

		/*
		 *  This device has not been bound, so bind it.
		 */
		p_sap->sap	 = (int)requested_sap;
		p_sap->sap_state = DL_IDLE;

		splx(oldpri);

		p_bind			= (DL_bind_ack_t *)response->b_wptr;
		p_bind->PRIM_type	= DL_BIND_ACK;
		p_bind->LLC_sap		= requested_sap;
		p_bind->ADDR_length	= MAC_ADD_SIZE;
		p_bind->ADDR_offset	= DL_BIND_ACK_SIZE;

		p_mac_addr  = (ushort*)(response->b_wptr + DL_BIND_ACK_SIZE);
		p_mac_addr2 = (ushort*)(p_sap->p_info->mac_addr);

		for (x = 0; x < (MAC_ADD_SIZE / 2); x++)
			*p_mac_addr++ = *p_mac_addr2++;

		response->b_wptr	  += DL_BIND_ACK_SIZE + MAC_ADD_SIZE;
		response->b_datap->db_type = M_PCPROTO;

		edlina_rput(RD(q), response);

		if (edlifstats)
			edlifstats[ board ].ifs_active = 1;
		break;

	case DL_IDLE:
	case DL_DEAD:
		EDEBUGP(DEB_ERROR,(CE_CONT, "dl_bind_req: invalid bind state\n"));

		p_error			=(DL_error_ack_t *)response->b_wptr;
		p_error->PRIM_type	= DL_ERROR_ACK;
		p_error->ERROR_prim	= DL_BIND_REQ;
		p_error->LLC_error	= DLOUTSTATE;
		p_error->UNIX_error	= 0;

		response->b_wptr	  += DL_ERROR_ACK_SIZE;
		response->b_datap->db_type = M_PCPROTO;

		edlina_rput(RD(q), response);
		splx(oldpri);
		break;
	}
	return;
}

/*
 *  dl_unbind_req():
 */
static
dl_unbind_req(q, mp)
queue_t *q;       /* write q */
mblk_t	*mp;
{
	mblk_t		*response, *flush;
	sap_t		*p_sap;
	DL_ok_ack_t	*p_ok_ack;
	DL_error_ack_t	*p_error;
	int		oldpri;

	EDEBUGP(DEB_CALL,(CE_CONT, "dl_unbind_req()\n"));

	/*
	 *  Allocate resorces for response and flush operations.
	 */
	p_sap = (sap_t *)(q->q_ptr);
	if (((response = allocb(DL_PRIMITIVES_SIZE, BPRI_MED)) == NULL) ||
	    ((flush    = allocb(DL_PRIMITIVES_SIZE, BPRI_MED)) == NULL))
		{
			freemsg(response);  /*one of these might be allocated*/
			freemsg(flush);     /*one of these might be allocated*/
			freemsg(mp);
			return;
		}
	/*
	 *  Fill in response and send to read q
	 */
	else
	{
		oldpri = splstr();

		/*
		 *  Process request when device is in idle state.
		 */
		if (p_sap->sap_state == DL_IDLE)
		{
			p_sap->sap_state = DL_UNBND;
			p_sap->sap	 = 0xffff;
			splx(oldpri);

			/*
			 *  Flush both q's -- the SAP is now out of service
			 */
			flushq(q, FLUSHDATA);
			flushq(RD(q), FLUSHDATA);

			flush->b_datap->db_type = M_FLUSH;
			*(flush->b_wptr)	= FLUSHRW;

			/*
			 *  LLI spec says flush before ack msg
			 */
			edlina_rput(RD(q), flush);

			/*
			 * Do ACK
			 */
			p_ok_ack = (DL_ok_ack_t *)(response->b_wptr);
			p_ok_ack->PRIM_type	= DL_OK_ACK;
			p_ok_ack->CORRECT_prim	= DL_UNBIND_REQ;

			response->b_wptr	  += DL_OK_ACK_SIZE;
			response->b_datap->db_type = M_PCPROTO;

			edlina_rput(RD(q),response);

			if (edlifstats)
			{
				int	board;
				board = p_sap->p_info->board_num;
				edlifstats[ board ].ifs_active = 0;
			}

		}

		/*
		 *  The device is not idle so generate an error.
		 */
		else
		{
			EDEBUGP(DEB_ERROR,(CE_CONT, "dl_unbind_req: invalid state\n"));
			splx(oldpri);

			p_error	= (DL_error_ack_t *)response->b_wptr;
			p_error->PRIM_type	= DL_ERROR_ACK;
			p_error->ERROR_prim	= DL_UNBIND_REQ;
			p_error->LLC_error	= DLOUTSTATE;
			p_error->UNIX_error	= 0;

			response->b_wptr	  += DL_ERROR_ACK_SIZE;
			response->b_datap->db_type = M_PCPROTO;

			edlina_rput(RD(q), response);

			/*
			 *  Would have been used if the unbind worked
			 */
			freemsg(flush);
		}
	}

	freemsg(mp);
	return;
}

/*
 *  dl_data_req():
 */
static
dl_data_req(q, mp)
queue_t *q;       /* write q */
mblk_t	*mp;
{
	sap_t		*p_sap;
	mblk_t		*response;
	DL_error_ack_t	*p_error;
	char		broadcast, loopback;
	long		off;
	int		board, oldpri, z, len, tlen;
	mblk_t		*mp_broad, *mp_broad2, *mp2, *new_mp;
	ushort		*p_addr, *p_host_id, *p_dest;
	DL_unitdata_ind_t *p_dl;
	DL_unitdata_req_t *p_data_req;
	struct tranrb	*tr_rb;

	EDEBUGP(DEB_CALL,(CE_CONT, "dl_data_req()\n"));

	/*
	 *  Check the length of data in mp, if more than 1500 throw away !!!!!!
	 */
	if ((len = msgdsize(mp->b_cont)) > MAX_PACK_SIZE )
	{
		EDEBUGP(DEB_ERROR,(CE_CONT, "dl_data_req: bad size %d\n", len));
		freemsg(mp); /* remember - first mblk_t holds DL_primitives */
		goto oerror1;
	}
	oldpri = splstr();

	p_sap = (sap_t *)(q->q_ptr);
	board = p_sap->p_info->board_num;

	/*
	 *  If the user has not preceded unitdata req with a bind req,
	 *  it's an error
	 */
	if ( p_sap->sap_state == DL_DEAD )
	{
		EDEBUGP(DEB_ERROR,(CE_CONT, "dl_data_req: bad state\n"));
		splx(oldpri);

		if ((response = allocb(DL_PRIMITIVES_SIZE, BPRI_MED)) == NULL)
		{
			freemsg(mp); 
			goto oerror1;
		}

		p_error			=(DL_error_ack_t *)response->b_wptr;
		p_error->PRIM_type	= DL_ERROR_ACK;
		p_error->ERROR_prim	= DL_UNBIND_REQ;
		p_error->LLC_error	= DLOUTSTATE;
		p_error->UNIX_error	= 0;

		response->b_wptr	  += DL_ERROR_ACK_SIZE;
		response->b_datap->db_type = M_PCPROTO;

		edlina_rput(RD(q), response);
		freemsg(mp);
		goto oerror1;
	}

	p_data_req	= (DL_unitdata_req_t *)(mp->b_rptr);
	off		= p_data_req->RA_offset;
	p_dest = p_addr = (ushort *)(mp->b_rptr + off);

	/*
	 *  See if this is a broadcast Ethernet address.
	 */
	broadcast = FALSE;
	if ((*(p_addr + 0) == 0xffff) &&
	    (*(p_addr + 1) == 0xffff) &&
	    (*(p_addr + 2) == 0xffff))
		broadcast = TRUE;
 	
	/*
	 *  See if addressed to us.
	 */
 	loopback = FALSE;
 	p_host_id = (ushort *)edl_board_struct[ board ].info.mac_addr;
 	if ((p_host_id[0] == p_addr[0]) &&
 	    (p_host_id[1] == p_addr[1]) &&
 	    (p_host_id[2] == p_addr[2]))
		loopback = TRUE; 

	/*
	 *  If this is a broadcast or loopback, send this frame to ourself.
	 */
	if ((broadcast || loopback)	&& (canput(OTHERQ(q)) != FALSE))
	{
		mp_broad  = allocb(DL_UNITDATA_IND_SIZE + (2 * MAC_ADD_SIZE), BPRI_MED);
 		mp_broad2 = (loopback ? mp->b_cont : copymsg(mp->b_cont));

		if ((mp_broad == NULL) || (mp_broad2 == NULL))
		{
			freemsg(mp_broad);
			if (loopback)
			{
				freemsg(mp);
				splx(oldpri);
				goto oerror1;
			}
			freemsg(mp_broad2);
			goto d1;
		}

		/*
		 * FIRST>>>FIRST>>>FIRST>>> fill in the dl_unitdata_ind header
		 */
		p_dl		 =(DL_unitdata_ind_t *)mp_broad->b_wptr;
		p_dl->PRIM_type  = DL_UNITDATA_IND;
		p_dl->RA_length  = MAC_ADD_SIZE;
		p_dl->RA_offset  = DL_UNITDATA_IND_SIZE;
		p_dl->LA_length  = MAC_ADD_SIZE;
		p_dl->LA_offset  = DL_UNITDATA_IND_SIZE + MAC_ADD_SIZE;
		p_dl->SERV_class = DL_NOSERV;

		/*
		 *  Six bytes of remote (source) host id
		 */
		p_addr	  = (ushort *)((char *)(p_dl) + p_dl->RA_offset);
		p_host_id = (ushort *)edl_board_struct[ board ].info.mac_addr;

		/*
		 *  Get address of src array
		 */
		for (z = 0; z < (MAC_ADD_SIZE / 2); z++)
			*p_addr++ = *p_host_id++;

		/*
		 *  Six bytes of local (destination) host id
		 */
		p_addr = (ushort *)((char *)(p_dl) + p_dl->LA_offset);
		for (z = 0; z < (MAC_ADD_SIZE / 2); z++)
			*p_addr++ = 0xffff;

		mp_broad->b_wptr += DL_UNITDATA_IND_SIZE + (2 * MAC_ADD_SIZE);
		mp_broad->b_datap->db_type = M_PROTO;

		linkb(mp_broad, mp_broad2);
		edlina_rput(OTHERQ(q), mp_broad);

 		if (loopback)
		{
 			unlinkb(mp);
 			freeb(mp);
 			splx(oldpri);
 			return;
 		}
	}

d1:
	/*
	 *  Send the frame to iNA. Since both the stream and dl_send_complete()
	 *  can call edlina_rawtran(), mutex the two with splstr()
	 */
	if ((tr_rb = (struct tranrb *)getrb(p_sap->p_info->p_ep)) == NULL)
	{
		cmn_err(CE_WARN, "EDLINA: Out of rb's - dropping outgoing message.\n");
		edl_stats.xmt_lost++;
		freemsg(mp);
		splx(oldpri);
		return;
	}
	/*
	 * Grab one block for entire message
	 */
	if ((new_mp = allocb(len, BPRI_MED)) == NULL)
	{
		EDEBUGP(DEB_ERROR,(CE_CONT, "dl_data_req: Out of buffers - dropping outgoing message.\n"));
		edl_stats.xmt_lost++;
		relrb(tr_rb);
		freemsg(mp);
		splx(oldpri);
		return;
	}
	mp2 = mp->b_cont;
	while(mp2)
	{
		tlen = mp2->b_wptr - mp2->b_rptr;
		bcopy( (caddr_t) mp2->b_rptr, (caddr_t)new_mp->b_wptr, tlen);
		new_mp->b_wptr += tlen;	
		mp2 = mp2->b_cont;
	}
	if (edlina_rawtran(p_sap, tr_rb, new_mp, p_dest) == FALSE)
	{
		cmn_err(CE_WARN, "EDLINA: Error on send - dropping outgoing message.\n");
		edl_stats.xmt_lost++;
		freemsg(new_mp);
		relrb(tr_rb);
	}
	freemsg(mp);
	splx(oldpri);
	return;
oerror1:
	if (edlifstats)
		edlifstats[ board ].ifs_oerrors++;
	return;
}

/*
 *  edlina_connect():
 */
edlina_connect(ep, cr_rb)
endpoint	*ep;
struct connrb	*cr_rb;
{
	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_connect()\n"));
	init_crbh(&cr_rb->cr_rbh);

	cr_rb->cr_rbh.c_resp	= 0;
	cr_rb->cr_rbh.c_len	= CONNRB_SIZE;
	cr_rb->cr_rbh.c_subsys	= SUB_EDL;
	cr_rb->cr_rbh.c_opcode	= OP_EDL_CONN;
	cr_rb->cr_ep		= ep;
	cr_rb->cr_lsap		= ERAWLSAP;
	cr_rb->cr_reserved	= 4;		/* filter option */
	cr_rb->cr_port		= 0xFF;

	return(mipsend(ep->bnum, PORT_960, cr_rb, cr_rb->cr_rbh.c_len));
}

/*
 *  edlina_conn_complete():
 *
 *  This function is called after iNAina has accepted/rejected a connect
 *  request to the raw EDL.  If the connect was accepted, we will prime
 *  the receive pump with RB's.
 */
edlina_conn_complete(cr_rb)
struct connrb	*cr_rb;
{
	struct postrb	*pr_rb;
	mblk_t		*mp;
	endpoint	*ep;
	int		i;

	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_conn_complete()\n"));
	ep = cr_rb->cr_ep;
	if (cr_rb->cr_rbh.c_resp != OK_RESP)
	{
		cmn_err(CE_WARN, "EDLINA: Connect failed - response = %x\n",
							cr_rb->cr_rbh.c_resp);
		kill_saps(ep->bnum);
		relrb((struct req_blk *)cr_rb);
		return;
	}
	
	relrb((struct req_blk *)cr_rb);

	/*
	 * Allocate and post input message buffers.
	 */
	for (i = 0; i < edl_max_bufs_posted; i++)
	{
		if ((pr_rb = (struct postrb *)getrb(ep)) == NULL)
			break;

		if ((mp = allocb(MAX_PACK_SIZE + POSTFRAMEH_SIZE, BPRI_MED)) == NULL)
		{
			relrb((struct req_blk *)pr_rb);
			break;
		}
		/*
		 * Adjust length of write pointer.
		 */
		mp->b_wptr += MAX_PACK_SIZE + POSTFRAMEH_SIZE;

		if (edlina_rawrecv(ep, mp, pr_rb) == FALSE)
		{
			relrb((struct req_blk *)pr_rb);
			break;
		}
	}

	/*
	 *  See if we have enough to operate.  If we didn't get the full amount,
	 *  print how many we did allocate.
	 */
	if (i == 0)
	{
		cmn_err(CE_WARN, "EDLINA: No buffers - endpoint not established.\n");
		kill_saps(ep->bnum);
	}
	else if (i < edl_max_bufs_posted)
		cmn_err(CE_WARN, "EDLINA: Only %d out of %d buffers posted.\n",
						   i, edl_max_bufs_posted);
	return;
}

/*
 *  edlina_disconnect():
 */
edlina_disconnect(ep, dr_rb)
endpoint	*ep;
struct discrb	*dr_rb;
{
	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_disconnect()\n"));
	init_crbh(&dr_rb->dr_rbh);

	dr_rb->dr_rbh.c_resp	= 0;
	dr_rb->dr_rbh.c_len	= DISCRB_SIZE;
	dr_rb->dr_rbh.c_subsys	= SUB_EDL;
	dr_rb->dr_rbh.c_opcode	= OP_EDL_DISC;
	dr_rb->dr_ep		= ep;
	dr_rb->dr_lsap		= ERAWLSAP;

	return(mipsend(ep->bnum, PORT_960, dr_rb, dr_rb->dr_rbh.c_len));
}

/*
 *  edlina_disconnect_complete():
 */
edlina_disc_complete(dr_rb)
struct discrb	*dr_rb;
{
	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_disc_complete()\n"));
	if (dr_rb->dr_rbh.c_resp != OK_RESP)
		cmn_err(CE_WARN, "EDLINA: Bad response on disconnect - %x\n",
							dr_rb->dr_rbh.c_resp);

	/*
	 * Free the endpoint.
	 */
	EDEBUGP(DEB_FULL,(CE_CONT, "edlina_disc_complete: freed ep = %x\n", dr_rb->dr_ep));
	dr_rb->dr_ep->tli_state = 0;
	dr_rb->dr_ep->str_state = C_IDLE;

	relrb((struct req_blk *)dr_rb);
	return;
}

/*
 *  edlina_rawtran():
 */
#define SBSWAP(x) ((((x) >> 8) & 0x00ff) | (((x) << 8) & 0xff0))
edlina_rawtran(p_sap, tr_rb, mp, p_addr)
sap_t	*p_sap;
struct tranrb	*tr_rb;
mblk_t	*mp;
ushort	*p_addr;
{
	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_rawtran()\n"));
	init_crbh(&tr_rb->tr_rbh);

	tr_rb->tr_rbh.c_resp	= 0;
	tr_rb->tr_rbh.c_len	= TRANRB_SIZE;
	tr_rb->tr_rbh.c_subsys	= SUB_EDL;
	tr_rb->tr_rbh.c_opcode	= OP_EDL_RAWTRAN;
	tr_rb->tr_q		= p_sap->read_q;
	tr_rb->tr_mp		= mp;
	tr_rb->tr_byte_cnt	= mp->b_wptr - mp->b_rptr;
*(int *)tr_rb->tr_buffer_ptr	= (int)kvtophys((caddr_t)tr_rb->tr_mp->b_rptr);
*(int *)tr_rb->tr_dst_addr_ptr	= (int)kvtophys((caddr_t)tr_rb->tr_tabuf);

	bcopy((caddr_t)p_addr, (caddr_t)tr_rb->tr_tabuf, MAC_ADD_SIZE);
	bzero((caddr_t)tr_rb->tr_src_addr, MAC_ADD_SIZE);

#ifdef TYPEISINTHEMSG
	tr_rb->tr_ether_type = *( (ushort *)((uchar *)p_addr + MAC_ADD_SIZE) );
#else
	tr_rb->tr_ether_type = SBSWAP((ushort)p_sap->sap);
#endif

	EDEBUGP(DEB_FULL,(CE_CONT, "edlina_rawtran: sap = %x len = %d\n",
			SBSWAP(tr_rb->tr_ether_type), tr_rb->tr_byte_cnt));
	/*
	 *  Keep statistics if wanted.
	 */
	if (edlifstats)
		edlifstats[ p_sap->p_info->p_ep->bnum].ifs_opackets++;

	return(mipsend(p_sap->p_info->p_ep->bnum, PORT_960, tr_rb,
							tr_rb->tr_rbh.c_len));
}

/*
 *  edlina_rawtran_complete():
 *
 *  This function is called from the iNAina driver when a xmit RB has been
 *  processed.
 */
edlina_rawtran_complete(tr_rb)
struct tranrb	*tr_rb;
{
	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_rawtran_complete()\n"));

	if (tr_rb->tr_rbh.c_resp != OK_RESP)
	{
		cmn_err(CE_WARN, "EDLINA: Transmit failure - response = %x.\n",
							tr_rb->tr_rbh.c_resp);
		edl_stats.xmt_lost++;
		if (edlifstats)
		{
			sap_t	*p_sap;
			p_sap = (sap_t *)tr_rb->tr_q->q_ptr;
			edlifstats[p_sap->p_info->board_num ].ifs_oerrors++;
		}
	}
	freemsg(tr_rb->tr_mp);
	relrb((struct req_blk *)tr_rb);
	return;
}

/*
 *  edlina_rawrecv():
 */
edlina_rawrecv(ep, mp, pr_rb)
endpoint	*ep;
mblk_t		*mp;
struct postrb	*pr_rb;
{
	int len;

	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_rawrecv()\n"));
	len = mp->b_wptr - mp->b_rptr;
	init_crbh(&pr_rb->rbh);

	pr_rb->rbh.c_resp	= 0xff;		/* Initialized for disconnect */
	pr_rb->rbh.c_len	= POSTRB_SIZE;
	pr_rb->rbh.c_subsys	= SUB_EDL;
	pr_rb->rbh.c_opcode	= OP_EDL_RAWRECV;
	pr_rb->ep		= ep;
	pr_rb->mp		= mp;
	pr_rb->lsap		= ERAWLSAP;
	pr_rb->num_blks		= 1;
	pr_rb->filled_length	= 0;
	pr_rb->buffer_length	= len;
	pr_rb->max_copy_len	= len;
	pr_rb->max_frames	= 1;
	pr_rb->actual_frames	= 0;
*(int *)pr_rb->buffer_ptr	= (int)kvtophys((caddr_t)pr_rb->mp->b_rptr);

	bzero((caddr_t)pr_rb->mp->b_rptr, POSTFRAMEH_SIZE);
	if (!mipsend(ep->bnum, PORT_960, pr_rb, pr_rb->rbh.c_len))
		return(FALSE);
	edl_stats.bufs_posted++;
	return(TRUE);
}

/*
 *  edlina_rawrecv_complete():
 *
 *  This function is called from the iNAina driver when a posted recive RB
 *  has been filled/returned.
 */
edlina_rawrecv_complete(pr_rb)
struct postrb	*pr_rb;
{
	register sap_t		*p_sap;
	register int		x;

	mblk_t			*mp;		/*  mp  -> DL_unitdata_ind */
	mblk_t			*mp2;		/*  mp2 -> the data	*/
	ushort			*p_host_id, *p_addr;
	DL_unitdata_ind_t	*p_dl;
	struct postframeh	*p_hdr;
	ushort			sap;
	int			framelen;

	EDEBUGP(DEB_CALL,(CE_CONT, "edlina_rawrecv_complete()\n"));

	edl_stats.bufs_posted--;
	if((pr_rb->rbh.c_resp != OK_RESP) && (pr_rb->rbh.c_resp != OK_EOM_RESP))
	{
		/*
		 *  A response of 0xff indicates that the buffer is being
		 *  returned without being filled.  This will happen when
		 *  we disconnect from raw EDL.
		 */
		if (pr_rb->rbh.c_resp != 0xff)
		{
			/*
			 *  If it is not a "bad endpoint" error, let's tell
			 *  them about it.  There are cases were we will get
			 *  a "bad endpoint" error because the disconnect is
			 *  not syncronized with a subsequent open but this
			 *  is usually harmless - but should be fixed!
			 */
			if (pr_rb->rbh.c_resp != 0xa)
				cmn_err(CE_WARN, "EDLINA: Receive error = %x\n",
							pr_rb->rbh.c_resp);
			edl_stats.rcv_errors++;
			if (edlifstats)
				edlifstats[ pr_rb->ep->bnum ].ifs_ierrors++;

			/*
		 	 *  Let the poor SAPs no that we are shutdown.
		 	 */
			if (edl_stats.bufs_posted == 0)
			{
				cmn_err(CE_WARN, "EDLINA: No buffers - receiver shutdown\n");
				kill_saps(pr_rb->ep->bnum);
			}
		}

		freemsg(pr_rb->mp);
		relrb((struct req_blk *)pr_rb);
		return;
	}

	p_hdr = (struct postframeh *)pr_rb->mp->b_rptr;
	framelen = p_hdr->record_length - POSTFRAMEH_SIZE;

	/*
	 *  The lost_count is sometimes reported as very large!
	 */
	edl_stats.overrun_cnt += (p_hdr->lost_count & 0xff);

	EDEBUGP(DEB_FULL,(CE_CONT, "edlina_rawrecv_complete: p_hdr->record_length = %x\n",
							p_hdr->record_length));
	EDEBUGP(DEB_FULL,(CE_CONT, "edlina_rawrecv_complete: p_hdr->lost_count = %x\n",
							p_hdr->lost_count));
	EDEBUGP(DEB_FULL,(CE_CONT, "edlina_rawrecv_complete: p_hdr->len_or_type = %x\n",
							p_hdr->len_or_type));

	/*
	 *  Check to see if a disconnect has happened which would cause unused
	 *  buffers to be returned.
	 */
	if (edl_board_struct[ pr_rb->ep->bnum ].info.opens == 0)
	{
		EDEBUGP(DEB_FULL,(CE_CONT, "edlina_rawrecv_complete: unused rb returned on disconnect.\n"));
		freemsg(pr_rb->mp);
		relrb((struct req_blk *)pr_rb);
		return;
	}

	/*
	 *  Keep statistics if wanted.
	 */
	if (edlifstats)
		edlifstats[ pr_rb->ep->bnum ].ifs_ipackets++;

	/*
	 *  Find the user listening on this device.
	 */
	sap = p_hdr->len_or_type;
	p_sap = edl_board_struct[ pr_rb->ep->bnum ].sap;
	for (x = 0; x < N_LSAP; x++, p_sap++)
	{
		if ((sap == p_sap->sap) && (p_sap->sap_state == DL_IDLE))
			break;
	}

	if (x == N_LSAP)
	{
		EDEBUGP(DEB_FULL,(CE_CONT, "edlina_rawrecv_complete: unknown sap = %x\n",sap));
		if (edlina_rawrecv(pr_rb->ep, pr_rb->mp, pr_rb) == FALSE)
		{
			freemsg(pr_rb->mp);
			relrb((struct req_blk *)pr_rb);
			EDEBUGP(DEB_ERROR,(CE_CONT, "edlina_rawrecv_complete: couldn't post receive buffer.\n"));
		}
		return;
	}

	/*
	 *  See if there is room in the queue.
	 */
	if (canput(p_sap->read_q) == FALSE)
	{
		EDEBUGP(DEB_ERROR,(CE_CONT, "edlina_rawrecv_complete: Read queue full - dropping incoming message.\n"));
		edl_stats.rcv_lost++;
		if (edlina_rawrecv(pr_rb->ep, pr_rb->mp, pr_rb) == FALSE)
		{
			freemsg(pr_rb->mp);
			relrb((struct req_blk *)pr_rb);
			EDEBUGP(DEB_ERROR,(CE_CONT, "edlina_rawrecv_complete: couldn't re-post receive rb.\n"));
		}
		return;
	}

	/*
	 *  There is someone with a not full queue listening at this address.
	 *  Allocate buffers.
	 */
	mp  = allocb(DL_UNITDATA_IND_SIZE + (2 * MAC_ADD_SIZE), BPRI_MED);
	mp2 = allocb(framelen, BPRI_MED);

	/*
	 *  If null throw the packet on floor - its a connectionless service
	 */
	if ((mp == NULL) || (mp2 == NULL))
	{
		freemsg(mp);
		freemsg(mp2);
		EDEBUGP(DEB_ERROR,(CE_CONT, "edlina_rawrecv_complete: Out of buffers - dropping incoming message.\n"));
		edl_stats.rcv_lost++;
		if (edlina_rawrecv(pr_rb->ep, pr_rb->mp, pr_rb) == FALSE)
		{
			freemsg(pr_rb->mp);
			relrb((struct req_blk *)pr_rb);
			EDEBUGP(DEB_ERROR,(CE_CONT, "edlina_rawrecv_complete: couldn't re-post receive rb.\n"));
		}
		return;
	}

	/*
	 * FIRST>>>FIRST>>>FIRST>>> fill in the dl_unitdata_ind header
	 */
	p_dl		 =(DL_unitdata_ind_t *)mp->b_wptr;
	p_dl->PRIM_type  = DL_UNITDATA_IND;
	p_dl->RA_length  = MAC_ADD_SIZE;
	p_dl->RA_offset  = DL_UNITDATA_IND_SIZE;
	p_dl->LA_length  = MAC_ADD_SIZE;
	p_dl->LA_offset  = DL_UNITDATA_IND_SIZE + MAC_ADD_SIZE;
	p_dl->SERV_class = DL_NOSERV;

	/*
	 *  Six bytes of remote (source) host id
	 */
	p_host_id = (ushort *)p_hdr->src_addr;	/* get address of src array */
	p_addr	  = (ushort *)( (char *)(p_dl) + p_dl->RA_offset );

	for (x = 0; x < (MAC_ADD_SIZE / 2); x++)
		*p_addr++ = *p_host_id++;

	/*
	 *  Six bytes of local (destination) host id
	 */
	p_addr	  = (ushort *)( (char *)(p_dl) + p_dl->LA_offset);
	p_host_id = (ushort *)p_hdr->dest_addr;	 /* get address of dest array */

	for (x = 0; x < (MAC_ADD_SIZE / 2); x++)
		*p_addr++ = *p_host_id++;

	mp->b_wptr	    += DL_UNITDATA_IND_SIZE + (2 * MAC_ADD_SIZE);
	mp->b_datap->db_type = M_PROTO;

	/*
	 *  SECOND>>>SECOND>>>SECOND>>> copy rcv data to msg block
	 */
	bcopy((caddr_t)(pr_rb->mp->b_rptr + POSTFRAMEH_SIZE), (caddr_t)mp2->b_wptr, framelen);
	mp2->b_wptr += framelen;

	/*
	 *  THIRD>>>THIRD>>>THIRD>>> send dl_unitdata_ind to user
	 */
	linkb(mp, mp2);
	edlina_rput(p_sap->read_q, mp);

	/*
	 *  Give this RB back to iNA
	 */
	if (edlina_rawrecv(pr_rb->ep, pr_rb->mp, pr_rb) == FALSE)
	{
		freemsg(pr_rb->mp);
		relrb((struct req_blk *)pr_rb);
		EDEBUGP(DEB_ERROR,(CE_CONT, "edlina_rawrecv_complete: couldn't post receive buffer.\n"));
	}
	return;
} 

/*
 *  kill_saps():
 *
 *  Mark all saps on a board as dead.
 */
static
kill_saps(bnum)
int bnum;
{
	sap_t	*p_sap;
	int	i;

	p_sap = edl_board_struct[ bnum ].sap;
	for (i = 0; i < N_LSAP; i++, p_sap++)
		p_sap->sap_state = DL_DEAD;
	return;
}
