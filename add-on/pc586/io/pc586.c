#ident	"@(#)pc586.c	1.2	92/03/01	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pc586:io/pc586.c	1.3.2.1"
/*
**	vi:set ts=4 sw=4:
*/

/*	Copyright (c) 1987, 1988, 1989 Intel Corp.		*/
/*	  All Rights Reserved  	*/
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license
 *	agreement or nondisclosure agreement with Intel Corpo-
 *	ration and may not be copied or disclosed except in
 *	accordance with the terms of that agreement.
 */

/*
 *  9/10/90 Version 2.9 RKL
 *
 *  Made changes to support faster CPU's.
 *
 *	12/7/89 Version 2.7 RKL
 *
 *	Changed splpc586() back to splstr() to fix ARP table entry timeout panic.
 *
 *	11/6/89 Version 2.6	HC
 *	
 *	Removed all references to PG except PG_V.
 *	Added pc586inetstats support for netstat(1) command. 
 *	Added printf() in watchdog() routine to warn possible board or
 *	 ethernet cable failure.
 *	Changed splstr() to splpc586() since splstr() is now defined as spltty()
 *	 in SVR4. splpc586() is defined in sys/pc586.h.
 *
 *	8/27/89	Version 2.5	HC
 *
 *	Fixed bug in receive buffer overrun.
 *
 *	5/12/89	Version 2.4	HC
 *
 *	Merged bug fixes from TWG and Lachman. Changed PG_P to PG_V for 
 *	R4.0.
 *
 *	8/30/88 Version 2.3   WJW
 *	
 *	Only one change: at label d1: moved splx(oldpri) after putq(q,mp);
 *	in response to Michael Garwood's (Lachman Associates) letter of 
 *	Aug 26, 88. This is a fix to rcp hanging and never finishing a file.
 * 
 *	5/27/88 Version 2.1   WJW
 *
 *	changes made to Version 1.0 
 *
 *	1)
 *	Removed all 5 putbq(mp) / buffcall() because a buffcall() must
 *	be called with a ...wsrv() routine as a parameter NOT a ...wput()
 *	It was easier to remove the putbq()s instead of adding a ...wsrv()
 *	which would have been the correct way to fix this.
 *
 *	2)
 *	Fixed the default: part of dl_cmds586() 
 * 	was:		p_error->PRIM_type = p_dl->prim_type;
 *			p_error->ERROR_prim= DL_BIND_REQ;
 *
 *	now:		p_error->PRIM_TYPE = DL_ERROR_ACK;
 *			p_error->ERROR_prim= p_dl->prim_type;
 *
 *	3)
 *	dl_unbind_req586()
 *	response->b_wptr+=DL_OK_ACK_SIZE instead of DL_UNBIND_REQ_SIZE
 *
 *	4)
 *	removed board_number = ... from pc586close(), it did nothing
 *
 *	5) 
 *	added support for type field being in the message sent from
 *	upper layer software instead of the assumption that the type
 *	field intended is in the q's data structure. Use -DTYPEISINTHEMSG
 *	to get this option. See pc586xmt_packet()
 *
 *	6)
 *	added support for loopback enet packets. This required mods to
 *	dl_data_req586() only	
 *	
 *	7)
 *	added missed_intr_count to watchdog to reduce the number of
 *	"PC586 board xx -- missed ..." messages to system console
 *
 *	8)
 *	fixed the handling of "incomplete long" in pc586xmt_packet()
 *	-- this is where streams msg blocks are copied into 586 buffers
 *	 
 */

#include "sys/pc586.h"

struct sockaddr {
	u_short	sa_family;
	char	sa_data[14];
};

#include "sys/kmem.h"
#include "net/if.h"
#include <sys/cmn_err.h>

extern	int			pc586inetstats;		/* inet statistics on/off flag       */
extern	struct		ifstats	*ifstats;	/* per-interface statistics for inet */	
extern	char		*pc586_ifname;		/* name of interface				 */
struct	ifstats		*pc586stat;			/* statistics for pc586 interface    */

char pc586copyright[] = "Copyright 1987, 1988, 1989 Intel Corporation";

extern	unsigned char	pc586default_add[];
extern	struct586_t		pc586struct[];
extern	int				pc586_boards;
extern	int				pc586_0_cmd_prom,			pc586_0_static_ram,
						pc586_0_interrupt_level,	pc586_0_major_dev,
						pc586_1_cmd_prom,			pc586_1_static_ram,
						pc586_1_interrupt_level,	pc586_1_major_dev,
						pc586_2_cmd_prom,			pc586_2_static_ram,
						pc586_2_interrupt_level,	pc586_2_major_dev,
						pc586_3_cmd_prom,			pc586_3_static_ram,
						pc586_3_interrupt_level,	pc586_3_major_dev;

int		pc586open(), pc586close(), pc586rput(), pc586wput();
int		pc586_reset(), pc586rcv_packet(), pc586put_packet(), pc586ru_start();
void 	pc586init();
void	pc586dl_cmds(), pc586dl_info_req(), pc586dl_bind_req(),
		pc586dl_unbind_req(), pc586dl_data_req();
void	pc586_watchdog(), pc586intr(), pc586re_q_fd(), pc586xmt_packet(),
		pc586build_cu(), pc586build_ru();

static void		shuv_word(), shuv_byte(), pc586_print_addr(), chan_attn();
static ushort	pull_word();
static int		prom_address(), wait_scb();

ushort	virt_to_pc586();
int		diagnose_586(), config_586();
void	int586_on(), int586_off();
char *	pc586_to_virt();
int		pc586bcopy();

struct module_stat pc586stats;

struct module_info pc586rminfo = { 586, "pc586", 0, MAX_PACK_SIZE, 4096, 256 };
struct module_info pc586wminfo = { 586, "pc586", 0, MAX_PACK_SIZE, 4096, 256 };

struct qinit pc586rinit = { pc586rput, NULL, pc586open, pc586close, NULL,
							&pc586rminfo, &pc586stats };
struct qinit pc586winit = { pc586wput, NULL, NULL, NULL, NULL, &pc586wminfo,
							&pc586stats };

struct streamtab pc586info ={ &pc586rinit, &pc586winit, NULL, NULL};

/*
 * tuning and debugging variables
 */
struct debug
{
	int	xmt_count;				/*  0 */
	int	rcv_count;				/*  1 */
	int collisions;				/*  2 */
	int	reset_count;			/*  3 */
	int	bad_status_in_intr;		/*  4 */
	int	rec_q_empty;			/*  5 */
	int	rcv_dropped;			/*  6 */
	int	scb_timeouts;			/*  7 */
	int	bad_rcv_status_count;	/*  8 */
	int	spurious_int_count;		/*  9 */
	int	xmt_busy_count;			/* 10 */
	int	rcv_restart_count;		/* 11 */
	int	last_recv_len;			/* 12 */
	int	benchmark;				/* 13 */
	int	watchdog_id;			/* 14 */
}  pc586debug = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

int	pc586interrupt[16];

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*              ROUTINES THAT INTERFACE TO THE STREAM HEAD                    */
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586init()
{
	int x, z, zero_address, ff_ff_ff_address;
	register int board;
	char *p_board;
	ushort y;
	board_stuff_t *p_stuff;

	for (x = 0; x < 16; x++)
		pc586interrupt[x] = 0xffffffff;

	if (pc586_boards > 0)
	{
		p_stuff					 = &pc586struct[0].board_stuff;
		p_stuff->board_interrupt = pc586_0_interrupt_level;
		p_stuff->p_cmd_prom      = (char *)pc586_0_cmd_prom;
		p_stuff->p_static_ram    = (char *)pc586_0_static_ram;
		p_stuff->maj_dev         = pc586_0_major_dev;

		if (pc586_0_interrupt_level < 16 && pc586_0_interrupt_level > 0)
			pc586interrupt[pc586_0_interrupt_level] = 0;
	}

	if (pc586_boards > 1)
	{
		p_stuff					 = &pc586struct[1].board_stuff;
		p_stuff->board_interrupt = pc586_1_interrupt_level;
		p_stuff->p_cmd_prom      = (char *)pc586_1_cmd_prom;
		p_stuff->p_static_ram    = (char *)pc586_1_static_ram;
		p_stuff->maj_dev         = pc586_1_major_dev;

		if (pc586_1_interrupt_level < 16 && pc586_1_interrupt_level > 0)
			pc586interrupt[pc586_1_interrupt_level] = 1;
	}

	if (pc586_boards > 2)
	{
		p_stuff					 = &pc586struct[2].board_stuff;
		p_stuff->board_interrupt = pc586_2_interrupt_level;
		p_stuff->p_cmd_prom      = (char *)pc586_2_cmd_prom;
		p_stuff->p_static_ram    = (char *)pc586_2_static_ram;
		p_stuff->maj_dev         = pc586_2_major_dev;

		if (pc586_2_interrupt_level < 16 && pc586_2_interrupt_level > 0)
			pc586interrupt[pc586_2_interrupt_level] = 2;
	}

	if (pc586_boards > 3)
	{
		p_stuff					 = &pc586struct[3].board_stuff;
		p_stuff->board_interrupt = pc586_3_interrupt_level;
		p_stuff->p_cmd_prom      = (char *)pc586_3_cmd_prom;
		p_stuff->p_static_ram    = (char *)pc586_3_static_ram;
		p_stuff->maj_dev         = pc586_3_major_dev;

		if (pc586_3_interrupt_level < 16 && pc586_3_interrupt_level > 0)
			pc586interrupt[pc586_3_interrupt_level] = 3;
	}

	/* 
	 * first write then read RAM to see if
	 * half-card is really in AT backplane
	 */
#ifdef	TYPEISINTHEMSG
	cmn_err(CE_CONT,
"PC586 v2.9x Copyright (c) 1987, 1988, 1989, 1990 Intel Corp., All Rights Reserved\n");

#else
	cmn_err(CE_CONT,
"PC586 v2.9 Copyright (c) 1987, 1988, 1989, 1990 Intel Corp., All Rights Reserved\n");

#endif
	for (x = 0; x < pc586_boards; x++)
	{
		p_stuff = &pc586struct[x].board_stuff;
		p_stuff->seated    = FALSE;
		p_stuff->cmd_timer = -1;

		/* 
		 * see if board is configed in per /etc/conf/pack.d/pc586/space.c
		 */
		if ((int)p_stuff->p_cmd_prom == 0xffffffff)
			continue;

		/*
		 * setup virtual pointers for command and static RAM areas.
		 */
		p_board = p_stuff->p_cmd_prom;
		p_stuff->p_virt_cmd_prom =
				(char *)physmap(p_board, 0x8000, KM_NOSLEEP);

		p_board = p_stuff->p_static_ram;
		p_stuff->p_virt_static_ram =
				(char *)physmap(p_board, 0x8000, KM_NOSLEEP);

		/*
		 * reset to insure board is in 8 bit mode (for reading PROM)
		 */
		p_board  = p_stuff->p_virt_cmd_prom;
		p_board += OFFSET_RESET;

		shuv_byte(p_board, 1);
		drv_usecwait(10);	/* 10 usec delay (must be > 4 clock cycles of 586)*/
		shuv_byte(p_board, 0);

		p_board  = p_stuff->p_virt_static_ram;
		p_board += OFFSET_SCB;
		shuv_word((ushort*)p_board, 0x5a5a);
		y = pull_word((ushort*)p_board);

		if ( y != 0x5a5a )
			cmn_err(CE_CONT, "PC586 board %d is missing\n", x);
		else
		{
			p_stuff->board_currently_open = 0; /* not used */
			p_stuff->seated               = TRUE;

			/*
			 * prom_address index should increment by one, however
			 * the pc586 board is stuck in word mode, thus ++ by 2
			 */
			p_stuff->mac_add[0] = prom_address(x,0);
			p_stuff->mac_add[1] = prom_address(x,2);
			p_stuff->mac_add[2] = prom_address(x,4);
			p_stuff->mac_add[3] = prom_address(x,6);
			p_stuff->mac_add[4] = prom_address(x,8);
			p_stuff->mac_add[5] = prom_address(x,10);

			zero_address     = 0;
			ff_ff_ff_address = 0;

			for (z = 0; z < MAC_ADD_SIZE; z++)
			{
				if (p_stuff->mac_add[z] == 0)
					zero_address++;
				if (p_stuff->mac_add[z] == 0xff)
					ff_ff_ff_address++;
			}

			if ((zero_address     == MAC_ADD_SIZE) ||
				(ff_ff_ff_address == MAC_ADD_SIZE))
			{
				cmn_err(CE_WARN,
		"invalid enet id, using address in /etc/conf/pack.d/pc586/space.c");

				/*
				 * temporary patch for pre-release pc586 boards!!!!!
				 * define default_add[] in space.c
				 */
				p_stuff->mac_add[0] = pc586default_add[0];
				p_stuff->mac_add[1] = pc586default_add[1];
				p_stuff->mac_add[2] = pc586default_add[2];
				p_stuff->mac_add[3] = pc586default_add[3];
				p_stuff->mac_add[4] = pc586default_add[4];
				p_stuff->mac_add[5] = pc586default_add[5];
			}

			cmn_err(CE_CONT, "PC586 board %d was found ", x);
			pc586_print_addr(p_stuff->mac_add);

			/*
			 * setup the ptype_t structures
			 */
			for (z = 0; z < N_LSAP; z++)
			{
				pc586struct[x].ptype[z].ptype_state   = DL_DEAD;
				pc586struct[x].ptype[z].p_board_stuff = p_stuff;
			}
		}
		(void)pc586_reset(x);
	}
	pc586debug.watchdog_id = timeout(pc586_watchdog, 0, HZ);

	if (pc586inetstats)
	{
		pc586stat = (struct ifstats*)kmem_zalloc((sizeof(struct ifstats) *
													pc586_boards), KM_NOSLEEP);
		if (pc586stat)
		{
			for (board = 0; board < pc586_boards; board++)
			{
				pc586stat[board].ifs_name = pc586_ifname;
				pc586stat[board].ifs_unit = (short)board;
				pc586stat[board].ifs_mtu  = MAX_PACK_SIZE;
				pc586stat[board].ifs_next = ifstats;
				ifstats = &pc586stat[board];
			}
		}
	}

	return;
} /* end of pc586init() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/* ARGSUSED */
pc586open(q, dev, flag, sflag)
queue_t *q; /* read q */
dev_t dev;
int flag;
int sflag;
{
	int ptype_free, board_number;
	int oldpri, x, found_board;

	found_board = FALSE;

	for (board_number = 0; board_number < pc586_boards; board_number++)
	{
		if (pc586struct[board_number].board_stuff.maj_dev == major(dev))
		{
			found_board = TRUE;
			break;
		}
	}
	if (found_board == FALSE)
		return(OPENFAIL);

	if (pc586struct[board_number].board_stuff.seated == FALSE)
		return(OPENFAIL);

	oldpri=splstr();

	if (sflag == CLONEOPEN)
	{
		ptype_free = FALSE;
		for (x = 0; x < N_LSAP; x++)
			if (pc586struct[board_number].ptype[x].ptype_state == DL_DEAD)
			{
				ptype_free=TRUE;
				break;
			}

		if (ptype_free)
			dev = ((dev & 0xff00) | (minor(x)));
		else
		{
			splx(oldpri);
			return(OPENFAIL);
		}
	}

	if (sflag != CLONEOPEN)
	{
		if (minor(dev)<0 || minor(dev)>(N_LSAP-1))
		{
			splx(oldpri);
			return(OPENFAIL);
		}

		if(pc586struct[board_number].ptype[(minor(dev))].ptype_state != DL_DEAD)
		{
			splx(oldpri);
			return(dev);
		}
	}

	/*
	 * allow the network board to be opened multiple times
	 */
	pc586struct[board_number].ptype[(minor(dev))].read_q  =    q;
	pc586struct[board_number].ptype[(minor(dev))].write_q = WR(q);

	/*
	 * p_ptr points to an ptype_t -- this is the
	 * "endpoint" for this instance of open
	 */
	q->q_ptr     = (caddr_t)&pc586struct[board_number].ptype[(minor(dev))];
	WR(q)->q_ptr = (caddr_t)&pc586struct[board_number].ptype[(minor(dev))];
	pc586struct[board_number].ptype[minor(dev)].ptype_state = DL_UNBND;

	splx(oldpri);

	return(dev);
} /* end of pc586open() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/* ARGSUSED */
int
pc586close(q, flag)
queue_t *q;                   /* point to read q */
int flag;                     /* file open flags */
{
	ptype_t *p_ptype;
	int oldpri, board_number;

	oldpri=splstr();

	p_ptype              = (ptype_t *)q->q_ptr;
	p_ptype->ptype       = 0;
	p_ptype->ptype_state = DL_DEAD;
	p_ptype->read_q      = NULL;
	p_ptype->write_q     = NULL;

	splx(oldpri);

	if (pc586inetstats && pc586stat)
	{
		board_number = pc586interrupt[p_ptype->p_board_stuff->board_interrupt];
		pc586stat[board_number].ifs_active      = 0;
		pc586stat[board_number].ifs_ipackets    = 0;
		pc586stat[board_number].ifs_ierrors     = 0;
		pc586stat[board_number].ifs_opackets    = 0;
		pc586stat[board_number].ifs_oerrors     = 0;
		pc586stat[board_number].ifs_collisions  = 0;
	}
	return;
} /* end of pc586close() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
int
pc586rput(q,mp)
queue_t *q;
mblk_t *mp;
{

	/*
	 * canput() was already called by pc586intr() ->
	 * pc586rcv_packet() -> pc586put_packet()
	 * canput() is not called by pc586dl_...() so as to make
	 * the design simpler
	 */
	putnext(q, mp);
	return;
} /* end of pc586rput() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
int
pc586wput(q,mp)
queue_t *q;       /* write q */
mblk_t *mp;
{

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
			qreply(q,mp);
		}
		else
			freemsg(mp);
		break;

	case M_IOCTL:	/* There are no supported M_IOCTL's */
		mp->b_datap->db_type = M_IOCNAK;
		qreply(q,mp);
		break;

	case M_PROTO:
	case M_PCPROTO:
		pc586dl_cmds(q, mp);
		break;

	} /* end of switch */

	return;
} /* end of pc586put */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*        ROUTINES THAT IMPLEMENT THE LOGICAL LINK INTERFACE SPEC             */
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586dl_cmds(q, mp)
queue_t *q;       /* write q */
mblk_t *mp;
{
	union DL_primitives *p_dl;
	mblk_t *response;
	DL_error_ack_t *p_error;

	p_dl = (union DL_primitives *)mp->b_datap->db_base;

	switch( (int)p_dl->prim_type )
	{
	case DL_INFO_REQ:
		pc586dl_info_req(q, mp);
		break;

	case DL_BIND_REQ:
		pc586dl_bind_req(q, mp);
		break;

	case DL_UNBIND_REQ:
		pc586dl_unbind_req(q, mp);
		break;

	case DL_UNITDATA_REQ:
		pc586dl_data_req(q, mp);
		break;

	default:
		if ((response = allocb(DL_PRIMITIVES_SIZE,BPRI_MED)) == NULL)
		{
			freemsg(mp);
			return;
		}
		p_error             = (DL_error_ack_t *)response->b_wptr;
		p_error->PRIM_type  = DL_ERROR_ACK;
		p_error->ERROR_prim = p_dl->prim_type;
		p_error->LLC_error  = DLBADSAP;
		p_error->UNIX_error = 0;
		response->b_wptr   += DL_ERROR_ACK_SIZE;

		pc586rput(RD(q),response);

		freemsg(mp);
		break;

	} /* end of switch */

	return;

} /* end of pc586dl_cmd */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586dl_info_req(q, mp)
queue_t *q;       /* write q */
mblk_t *mp;
{
	mblk_t *response;
	DL_info_ack_t *p_info_ack;
	ptype_t *p_ptype;
	int oldpri;

	if ((response = allocb(DL_PRIMITIVES_SIZE, BPRI_MED)) == NULL)
	{
		freemsg(mp);
		return;
	}
	freemsg(mp);

	oldpri = splstr();
	p_ptype                     = (ptype_t *)q->q_ptr;
	p_info_ack                  = (DL_info_ack_t *)response->b_wptr;
	p_info_ack->PRIM_type       = DL_INFO_ACK;
	p_info_ack->SDU_max         = MAX_PACK_SIZE;
	p_info_ack->SDU_min         = 46;
	p_info_ack->ADDR_length     = MAC_ADD_SIZE;
	p_info_ack->SUBNET_type     = DL_ETHER;
	p_info_ack->SERV_class      = DL_NOSERV;
	p_info_ack->CURRENT_state   = (long)(p_ptype->ptype_state);

	response->b_datap->db_type  = M_PCPROTO;
	response->b_wptr           += DL_INFO_ACK_SIZE;

	splx(oldpri);
	pc586rput(RD(q), response);

	return;
} /* end of pc586dl_info_req */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586dl_bind_req(q,mp)
queue_t *q;       /* write q */
mblk_t *mp;
{
	DL_bind_req_t *p_dl;
	board_stuff_t *p_stuff;
	ptype_t *p_ptype;
	mblk_t *response;
	DL_error_ack_t *p_error;
	DL_bind_ack_t *p_bind;
	ushort *p_mac_addr, *p_mac_addr2;
	int board_number, x, oldpri;
	long requested_ptype;

	if ((response = allocb(DL_PRIMITIVES_SIZE, BPRI_MED)) == NULL)
	{
		freemsg(mp);
		return;
	}


	p_dl = (DL_bind_req_t *)mp->b_datap->db_base;
	requested_ptype = p_dl->LLC_sap;

	freemsg(mp);

	oldpri = splstr();

	/*
	 * q->q_ptr set by pc586open, points to the ptype_t
	 * that is "owned" by this queue
	 */
	p_ptype= (ptype_t *)(q->q_ptr);

	switch ( p_ptype->ptype_state )
	{
	case DL_UNBND:
		p_stuff = p_ptype->p_board_stuff;
		board_number = pc586interrupt[p_stuff->board_interrupt];
		for (x = 0; x < N_LSAP; x++)
		{
			/*
			 * if the ptype was already taken, return an error
			 */
			if ((pc586struct[board_number].ptype[x].ptype == requested_ptype) &&
				(pc586struct[board_number].ptype[x].ptype_state == DL_IDLE))
				{
					splx(oldpri);
					p_error             = (DL_error_ack_t *)response->b_wptr;
					p_error->PRIM_type  = DL_ERROR_ACK;
					p_error->ERROR_prim = DL_BIND_REQ;
					p_error->LLC_error  = DLBADSAP;
					p_error->UNIX_error = 0;

					response->b_wptr          += DL_ERROR_ACK_SIZE;
					response->b_datap->db_type = M_PCPROTO;

					pc586rput(RD(q),response);

					return;
				}
			}

		p_ptype->ptype       = (int)requested_ptype;
		p_ptype->ptype_state = DL_IDLE;

		splx(oldpri);

		p_bind              = (DL_bind_ack_t *)response->b_wptr;
		p_bind->PRIM_type   = DL_BIND_ACK;
		p_bind->LLC_sap     = requested_ptype;
		p_bind->ADDR_length = MAC_ADD_SIZE;
		p_bind->ADDR_offset = DL_BIND_ACK_SIZE;

		p_mac_addr  = (ushort *)(response->b_wptr + DL_BIND_ACK_SIZE);
		p_mac_addr2 = (ushort *)(p_stuff->mac_add);

		for (x = 0; x < (MAC_ADD_SIZE / 2); x++)
			*p_mac_addr++ = *p_mac_addr2++;

		response->b_wptr = response->b_wptr + DL_BIND_ACK_SIZE + MAC_ADD_SIZE;
		response->b_datap->db_type = M_PCPROTO;

		pc586rput(RD(q),response);

		if (pc586inetstats && pc586stat) 
			pc586stat[board_number].ifs_active = 1;
		break;

	case DL_IDLE:
	case DL_DEAD:
		p_error             =(DL_error_ack_t *)response->b_wptr;
		p_error->PRIM_type  = DL_ERROR_ACK;
		p_error->ERROR_prim = DL_BIND_REQ;
		p_error->LLC_error  = DLOUTSTATE;
		p_error->UNIX_error = 0;

		response->b_wptr += DL_ERROR_ACK_SIZE;
		response->b_datap->db_type = M_PCPROTO;

		pc586rput(RD(q),response);
		splx(oldpri);
		break;

	} /* end of switch */

	return;
} /* end of pc586dl_bind_req */


/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586dl_unbind_req(q, mp)
queue_t *q;       /* write q */
mblk_t *mp;
{
	mblk_t *response, *flush;
	ptype_t *p_ptype;
	DL_ok_ack_t *p_ok_ack;
	DL_error_ack_t *p_error;
	int oldpri, board_number;

	p_ptype = (ptype_t *)(q->q_ptr);

	if (((response = allocb(DL_PRIMITIVES_SIZE,BPRI_MED)) == NULL) ||
	    ((flush    = allocb(DL_PRIMITIVES_SIZE,BPRI_MED)) == NULL))
	{
		freemsg(response);  /*one of these might be allocated*/
		freemsg(flush);     /*one of these might be allocated*/
		freemsg(mp);
		return;
	}
	/*
	 * fill in response->... and send to read q
	 */
	else
	{
		oldpri = splstr();

		if (p_ptype->ptype_state == DL_IDLE)
		{
			p_ptype->ptype_state = DL_UNBND;
			p_ptype->ptype       = 0xffff;
			splx(oldpri);

			/*
			 * flush both q's -- the ptype is now out of service
			 */
			flushq(q, FLUSHDATA);
			flushq(RD(q), FLUSHDATA);
			flush->b_datap->db_type = M_FLUSH;
			*(flush->b_wptr) = FLUSHRW;

			pc586rput(RD(q), flush);

			/*
			 * LLI spec says flush before ack msg
			 */
			p_ok_ack               = (DL_ok_ack_t *)(response->b_wptr);
			p_ok_ack->PRIM_type    = DL_OK_ACK;
			p_ok_ack->CORRECT_prim = DL_UNBIND_REQ;

			response->b_wptr +=DL_OK_ACK_SIZE;
			response->b_datap->db_type = M_PCPROTO;

			pc586rput(RD(q),response);

			if (pc586inetstats && pc586stat)
			{
				board_number =
						pc586interrupt[p_ptype->p_board_stuff->board_interrupt];
				pc586stat[board_number].ifs_active = 0;
			}
		}
		else
		{
			splx(oldpri);

			p_error             = (DL_error_ack_t *)response->b_wptr;
			p_error->PRIM_type  = DL_ERROR_ACK;
			p_error->ERROR_prim = DL_UNBIND_REQ;
			p_error->LLC_error  = DLOUTSTATE;
			p_error->UNIX_error = 0;

			response->b_wptr += DL_ERROR_ACK_SIZE;
			response->b_datap->db_type = M_PCPROTO;

			pc586rput(RD(q), response);
			freemsg(flush); /* would have been used if the unbind worked */
		}
	}

	freemsg(mp);
	return;
} /* end of pc586dl_unbind_req */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586dl_data_req(q, mp)
queue_t *q;       /* write q */
mblk_t *mp;
{
	int board_number, oldpri;
	ptype_t *p_ptype;
	mblk_t *response;
	DL_error_ack_t *p_error;
	char broadcast, loopback;
	long off;
	int z;
	mblk_t *mp_broad, *mp_broad2;
	DL_unitdata_req_t *p_data_req;
	ushort *p_addr, *p_host_id;
	DL_unitdata_ind_t *p_dl;
	board_stuff_t *p_stuff;

	p_ptype = (ptype_t *)(q->q_ptr);
	p_stuff = p_ptype->p_board_stuff;
	board_number = pc586interrupt[p_ptype->p_board_stuff->board_interrupt];

	/*
	 * check the length of data in mp, if more than 1500 throw away !!!!!!
	 */
	if (msgdsize(mp->b_cont) > MAX_PACK_SIZE)
	{
		freemsg(mp); /* remember - first mblk_t holds DL_primitives */
		goto oerror1;
	}
	oldpri = splstr();

	/*
	 * if the user has not preceded unitdata req with a bind req, it's an error
	 */
	if (p_ptype->ptype_state == DL_DEAD)
	{
		splx(oldpri);

		if ((response = allocb(DL_PRIMITIVES_SIZE, BPRI_MED)) == NULL)
		{
			freemsg(mp); 
			goto oerror1;
		}

		p_error             =(DL_error_ack_t *)response->b_wptr;
		p_error->PRIM_type  = DL_ERROR_ACK;
		p_error->ERROR_prim = DL_UNBIND_REQ;
		p_error->LLC_error  = DLOUTSTATE;
		p_error->UNIX_error = 0;

		response->b_wptr += DL_ERROR_ACK_SIZE;
		response->b_datap->db_type = M_PCPROTO;

		pc586rput(RD(q), response);
		freemsg(mp);
		goto oerror1;
	}

	p_data_req = (DL_unitdata_req_t *)(mp->b_rptr);
	off = p_data_req->RA_offset;

	p_addr = (ushort *)(mp->b_rptr + off);
	broadcast = FALSE;
	if ((*(p_addr + 0) == 0xffff) &&
		(*(p_addr + 1) == 0xffff) &&
		(*(p_addr + 2) == 0xffff))
		broadcast = TRUE;
 	
	/*
	 * if addressed to us
	 */
 	loopback = FALSE;
 	p_host_id = (ushort *)(p_stuff->mac_add);
 	if ((p_host_id[0] == p_addr[0]) &&
 	    (p_host_id[1] == p_addr[1]) &&
 	    (p_host_id[2] == p_addr[2]))
		loopback = TRUE; 

	if ((broadcast || loopback) && (canput(OTHERQ(q)) != FALSE))
	{
		mp_broad = allocb( (DL_UNITDATA_IND_SIZE+(2*MAC_ADD_SIZE)), BPRI_MED );
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
		p_dl             = (DL_unitdata_ind_t *)mp_broad->b_wptr;
		p_dl->PRIM_type  = DL_UNITDATA_IND;
		p_dl->RA_length  = MAC_ADD_SIZE;
		p_dl->RA_offset  = DL_UNITDATA_IND_SIZE;
		p_dl->LA_length  = MAC_ADD_SIZE;
		p_dl->LA_offset  = DL_UNITDATA_IND_SIZE + MAC_ADD_SIZE;
		p_dl->SERV_class = DL_NOSERV;

		/*
		 * six bytes of remote (source) host id
		 */
		p_addr    = (ushort *)((char *)(p_dl) + p_dl->RA_offset);
		p_host_id = (ushort *)(p_stuff->mac_add);

		/*
		 * get address of src array
		 */
		for (z = 0; z < (MAC_ADD_SIZE / 2); z++)
			*p_addr++ = *p_host_id++;

		/*
		 * six bytes of local (destination) host id
		 */
		p_addr = (ushort *)( (char *)(p_dl) + p_dl->LA_offset );

		for (z = 0; z < (MAC_ADD_SIZE / 2); z++)
			*p_addr++ = 0xffff;

		mp_broad->b_wptr += DL_UNITDATA_IND_SIZE + (2 * MAC_ADD_SIZE);
		mp_broad->b_datap->db_type = M_PROTO;

		linkb(mp_broad, mp_broad2);
		pc586rput(OTHERQ(q), mp_broad);

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
	 *  order of evaluation is important.
	 */
	if ((p_stuff->cmd_timer != (-1)) ||
		(wait_scb(board_number, 10000) == TRUE))	/* wait up to 100 msec */
	{
		if (p_stuff->cmd_timer != -1)
			pc586debug.xmt_busy_count++;
		/*
		 *  cause a reset through watchdog since scb won't clear
		 */
		else
			p_stuff->cmd_timer = 0;
			
		/*
		 * will be retrieved by interrupt() & pc586xmt_packet()
		 */
		putq(q, mp);
		splx(oldpri);

		return;
	}
	/*
	 * translate the packet into 586 data structs & send it. Since both the
	 * stream and isr can call pc586xmt_packet, mutex the two with splstr()
	 */
	pc586xmt_packet(p_ptype->ptype, board_number, mp);

	splx(oldpri);

	return;

oerror1:
	if (pc586inetstats && pc586stat)
		pc586stat[board_number].ifs_oerrors++;
	return;

} /* end of pc586dl_data_req */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
pc586put_packet(board_number, p_fd)
int board_number;
fd_t *p_fd;
{
	int x, y, z;
	mblk_t *mp, *mp2; /* mp points to DL_unitdata_ind & mp2 points to data */
	rbd_t *p_rbd;
	ushort *p_buffer, *p_host_id, *p_addr;
	pack_ushort_t before_swap, after_swap;
	DL_unitdata_ind_t *p_dl;
	ushort bytes_in_msg;

	p_rbd = (rbd_t *)pc586_to_virt(board_number, p_fd->fd_rbd_ofst);

	if (p_rbd == NULL)
		return(FALSE);			/* if so, frame is too short */

	p_buffer = (ushort *)pc586_to_virt(board_number,p_rbd->rbd_buff);
	if (p_buffer == NULL)	/* DIS */
	{
		int586_off(board_number);
		pc586struct[board_number].board_stuff.cmd_timer = 0;
		return(FALSE);
	}

	before_swap.c.b   = p_fd->fd_length;
	after_swap.c.a[0] = before_swap.c.a[1];
	after_swap.c.a[1] = before_swap.c.a[0];

	for (y = x = 0; x < N_LSAP; x++)
		if((pc586struct[board_number].ptype[x].ptype       == after_swap.c.b) &&
		   (pc586struct[board_number].ptype[x].ptype_state == DL_IDLE) )
		{
			y = TRUE;
			break;
		}

	if (y == FALSE )
		return(TRUE);	/* don't count ptype mismatch an as error (rkl) */

	if (canput(pc586struct[board_number].ptype[x].read_q) == FALSE)
		return(FALSE);

	/*
	 * there is someone with a not full q listening at this address
	 */
	mp = allocb( (DL_UNITDATA_IND_SIZE+(2 * MAC_ADD_SIZE)), BPRI_MED);

	for (bytes_in_msg = 0; p_rbd;
			p_rbd = (rbd_t *)pc586_to_virt(board_number,p_rbd->rbd_nxt_ofst))
	{
		bytes_in_msg += (p_rbd->rbd_status & CS_RBD_CNT_MSK);
	}

	p_rbd = (rbd_t *)pc586_to_virt(board_number, p_fd->fd_rbd_ofst);

	/*
	 * pad for 4 byte boundry moves
	 */
	mp2 = allocb( (bytes_in_msg + 4), BPRI_MED);
	pc586debug.last_recv_len = bytes_in_msg + 4;

	/*
	 * if null throw the packet on floor - its a connectionless service
	 */
	if ((mp == NULL) || (mp2 == NULL))
	{
		freemsg(mp);
		freemsg(mp2);
		return(FALSE);
	}

	/*
	 * FIRST>>>FIRST>>>FIRST>>> fill in the dl_unitdata_ind header
	 */
#define	SAP_SIZE	2	/* should be in pc586.h */
	p_dl             = (DL_unitdata_ind_t *)mp->b_wptr;
	p_dl->PRIM_type  = DL_UNITDATA_IND;
	p_dl->RA_length  = MAC_ADD_SIZE + SAP_SIZE;
	p_dl->RA_offset  = DL_UNITDATA_IND_SIZE;
	p_dl->LA_length  = MAC_ADD_SIZE + SAP_SIZE;
	p_dl->LA_offset  = DL_UNITDATA_IND_SIZE + MAC_ADD_SIZE + SAP_SIZE;
	p_dl->SERV_class = DL_NOSERV;

	/*
	 * six bytes of destination host and sap id
	 */
	p_addr    = (ushort *)((char *)(p_dl) + p_dl->RA_offset);
	p_host_id = (ushort *)p_fd->fd_dest;	/* get address of dest array */

	for (z = 0; z < (MAC_ADD_SIZE / 2); z++)
		*p_addr++ = *p_host_id++;
	*p_addr = after_swap.c.b;

	/*
	 * six bytes of source host and sap id
	 */
	p_addr    = (ushort *)((char *)(p_dl) + p_dl->LA_offset );
	p_host_id = (ushort *)p_fd->fd_src;		/* get address of src array */

	for (z = 0; z < (MAC_ADD_SIZE / 2); z++)
		*p_addr++ = *p_host_id++;
	*p_addr = after_swap.c.b;

	mp->b_wptr += DL_UNITDATA_IND_SIZE + ((2 * MAC_ADD_SIZE) + (2 * SAP_SIZE));
	mp->b_datap->db_type = M_PROTO;

	/*
	 * SECOND>>>SECOND>>>SECOND>>> copy rcv data to msg block
	 */
	bytes_in_msg = p_rbd->rbd_status & CS_RBD_CNT_MSK;

	/* all rcv buffers except the last one will have RCVBUFSIZE
	 * bytes of user data. RCVBUFSIZE has been set to a multiple of[
	 * 4, thus adding 3 will NOT cause pc586bcopy to copy any extra
	 * bytes. If the very last rcv buffer holds an uneven multiple
	 * of four bytes, then extra bytes will be copied (but ignored)
	 * Note that code above pads 4 extra bytes on the end of mp2
	 */
	do
	{
		pc586bcopy(p_buffer, mp2->b_wptr, (bytes_in_msg + 3));
		mp2->b_wptr += bytes_in_msg;

		if (((p_rbd->rbd_status & CS_EOF) == CS_EOF) || 
		    ((p_rbd->rbd_size   & CS_EL)  == CS_EL)) 
			break;

		p_rbd = (rbd_t *)pc586_to_virt(board_number, p_rbd->rbd_nxt_ofst);

		if (p_rbd)
		{
			p_buffer = (ushort *)pc586_to_virt(board_number, p_rbd->rbd_buff);
			if (p_buffer == NULL)
			{
			    int586_off(board_number);
			    pc586struct[board_number].board_stuff.cmd_timer = 0;
			    return(FALSE);
			}
			bytes_in_msg = p_rbd->rbd_status & CS_RBD_CNT_MSK;
		}
	} while (p_rbd);

	if (mp2->b_wptr == mp2->b_rptr)
		cmn_err(CE_NOTE, "PC586 rcv'ed 0 length msg");

	/*
	 * THIRD>>>THIRD>>>THIRD>>> send dl_unitdata_ind to user
	 */
	linkb(mp,mp2);
	pc586rput(pc586struct[board_number].ptype[x].read_q, mp);

	return(TRUE);
} /* end of pc586put_packet */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*                   ROUTINES THAT MANAGE THE 82586                          */
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
pc586_reset(board_number)
int board_number;
{
	char *p_hwcomm, *p_cmd_prom, *p_static_ram;
	scb_t *p_scb;
	int x, oldpri;

	/*
	 * pc586_reset() is called during the very first open() and from
	 * pc586_watchdog() and pc586dl_data_req.  It is NEVER called directly
	 * from the isr the isr aka pc586intr() will trip the watchdog() to do
	 * the reset). pc586_reset() does not use the interrupt to reset the 586.
	 * This * means the isr is simpler because it does not handle reset 
	 * interrupts -- only xmt and rcv interrupts. pc586_reset()
	 * does need splstr() splx() protection because it must execute 
	 * without interruption.
	 */
	pc586debug.reset_count++;

	p_cmd_prom   = pc586struct[board_number].board_stuff.p_virt_cmd_prom;
	p_static_ram = pc586struct[board_number].board_stuff.p_virt_static_ram;

	/*
	 * first shut off interrupts from board,
	 * drop chan att - shouldn't be raised
	 */
	int586_off(board_number);
	p_hwcomm = (p_cmd_prom + OFFSET_CHAN_ATT);
	shuv_word((ushort*)p_hwcomm, CMD_0);

	oldpri = splhi();

	/*
	 *  Some 82C501 parts that don't like comming up with loop back enabled.  
	 *  The fix is to toggle the esi loop back mode.  We will force it out
	 *  of loopback here and then go into loop back after reseting the board.
	 */
	p_hwcomm = (p_cmd_prom + OFFSET_NORMMODE);
	shuv_word((ushort*)p_hwcomm, CMD_1);

	/*
	 * hardware reset the 586
	 */
	p_hwcomm = (p_cmd_prom + OFFSET_RESET);
	shuv_word((ushort*)p_hwcomm, CMD_1);
	drv_usecwait(100);	/* 100 usec delay */
	shuv_word((ushort*)p_hwcomm,CMD_0);
	drv_usecwait(100);	/* 100 usec delay */

	pc586struct[board_number].board_stuff.cmd_timer   = -1;
	pc586struct[board_number].board_stuff.round_robin = 0;

	/*
	 * esi loopback - until diagnostics are run
	 */
	p_hwcomm= (p_cmd_prom + OFFSET_NORMMODE);
	shuv_word((ushort*)p_hwcomm, CMD_0);

	/*
	 * 16 bit - for at bus
	 */
	p_hwcomm = (p_cmd_prom + OFFSET_16B_XFER);
	shuv_word((ushort*)p_hwcomm, CMD_1);

	/*
	 * initialize all 586 data structs
	 */
	pc586build_cu(board_number);	/* inits scp, iscp, scb, cb, tbd and tbuf */
	pc586build_ru(board_number);	/* inits scb, fd's, rbd's and rbufs */

	/*
	 * chan attention to feed 586 its data structs
	 */
	chan_attn(board_number);

	/*
	 *  wait up to 10 msec for response.
	 */
	p_scb = (scb_t *)(p_static_ram + OFFSET_SCB);
	for (x = 1000; x; x--)
	{
		if (p_scb->scb_status == (SCB_INT_CX | SCB_INT_CNA))
			break;
		drv_usecwait(10);	/* wait 10 usec before next try */
	}

	/*
	 * see if board failed
	 */
	if (x == 0)
	{
		splx(oldpri);
		return(FALSE);
	}

	p_scb->scb_cmd = (SCB_ACK_CX | SCB_ACK_CNA);
	chan_attn(board_number);

	/*
	 * 586 cmd number 7, busy waits for execution
	 */
	if (diagnose_586(board_number) == FALSE)
	{
		splx(oldpri);
		return(FALSE);
	}

	/*
	 * cmd # 2, load default config and host id
	 */
	if (config_586(board_number) == FALSE)
	{
		splx(oldpri);
		return(FALSE);
	}

	/*
	 * Insert code for loopback test here
	 */

	/*
	 * 586 is ready, turn on interrupt, turn loopback off and RU on
	 */
	p_hwcomm = (p_cmd_prom + OFFSET_NORMMODE);
	shuv_word((ushort*)p_hwcomm, CMD_1);
	int586_on(board_number);

	/*
	 * start receive unit on the 586
	 * NOTE: order of evaluation is important.
	 */
	if ((wait_scb(board_number, 100) == TRUE) ||	/* wait up to 1 msec */
		(pc586ru_start(board_number) == FALSE))
	{
		splx(oldpri);
		return(FALSE);
	}

	/*
	 * since pc586_reset can be called from pc586_watchdog(), flush the
	 * write queues.
	 */
	for (x = 0; x < N_LSAP; x++)
		if ( pc586struct[board_number].ptype[x].write_q != NULL )
			flushq(pc586struct[board_number].ptype[x].write_q, FLUSHALL);

	splx(oldpri);

	/*
	 * board was successfully reset and initialized,
	 * also CU is idle and RU has been started
	 */
	return(TRUE);

} /* end of pc586_reset() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586_watchdog()
{
	int x, y, oldpri;

	/*
	 * perpetuate the callout
	 */
	pc586debug.watchdog_id = timeout(pc586_watchdog, 0, HZ);

	for (x = 0; x < pc586_boards; x++)
	{
		oldpri = splstr();

		if (pc586struct[x].board_stuff.cmd_timer > 0 )
			pc586struct[x].board_stuff.cmd_timer--;

		if (pc586struct[x].board_stuff.cmd_timer == 0 )
		{
			pc586struct[x].board_stuff.cmd_timer = - 1;
			splx(oldpri);
			cmn_err(CE_NOTE,
	"PC586: board %d timed out. Board or ethernet cable may be off line.", x);

			/*
			 * board is not responding with interrupts so rest it.
			 */
			for (y = 0; y < 10; y++)
				if (pc586_reset(x) == TRUE)
					return;

			cmn_err(CE_WARN, "PC586 board %d will not reset", x);
			return;
		}

	splx(oldpri);
	}
	return;
} /* end of pc586_watchdog() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586intr(unix_level)
int unix_level;
{
	int board_number, next, x;
	ushort scb_status;
	char *p_static_ram;
	scb_t *p_scb;
	cmd_t *p_cb;
	mblk_t *p_msg;
	board_stuff_t *p_stuff;


	board_number = pc586interrupt[unix_level];
	if (board_number > pc586_boards - 1)
	{
		cmn_err(CE_NOTE, "Interrupt from PC586 board %d not configured in.",
																board_number);
		return; /* intr not configed in */
	}

	p_stuff = &pc586struct[board_number].board_stuff;
	if (p_stuff->seated == FALSE ) 
	{ 
		cmn_err(CE_WARN, "PC586 board %d not seated.", board_number);
		return;
	}

	p_static_ram = p_stuff->p_virt_static_ram;
	p_scb = (scb_t *)(p_static_ram + OFFSET_SCB);
	p_cb  = (cmd_t *)(p_static_ram + OFFSET_CU);

	/*
	 * If at any point we don't get the PC586 to respond, casuse a reset.
	 */
	if (wait_scb(board_number, 100))	/* wait up to 1 msec */
		goto f1;

	scb_status = p_scb->scb_status;
	if ((scb_status & SCB_INT_MSK) == 0)
	{
		pc586debug.spurious_int_count++;
		return;			/* spurious interrupt */
	}

	p_scb->scb_cmd = (scb_status & SCB_INT_MSK); /* ack the status bits */
	chan_attn(board_number);

	if (scb_status & (SCB_INT_FR | SCB_INT_RNR))
	{
		if (wait_scb(board_number, 100))	/* wait up to 1 msec */
			goto f1;
		/*
		 * rcv /\ /\ /\ /\ /\ /\ /\ /\
		 * reset card if receiver in bad state
		 */
		if (pc586rcv_packet(board_number) == FALSE)
			goto f1;
	}

 	/*
	 * xmt \/ \/ \/ \/ \/ \/ \/ \/
	 */
	if (scb_status & SCB_INT_CNA)
	{
		if (wait_scb(board_number, 100))	/* wait up to 1 msec */
			goto f1;

		/*
		 * see if CB is still busy
		 */
		if ((p_cb->cmd_status & CS_CMPLT) == 0)
			return;

		/*
		 * check for OK status and carrier sense
		 */
		if (((p_cb->cmd_status & CS_OK) == 0) &&
		     (p_cb->cmd_status & 0x0400))	/* no carrier sense */
			cmn_err(CE_WARN, "PC586 transmission link error - check cable.");

		/*
		 *  Keep collision data
		 */
#define	CS_COLLISIONS	0x000f	/* should be in 82586.h */
		pc586debug.collisions += (p_cb->cmd_status & CS_COLLISIONS);
 		if (pc586inetstats && pc586stat)
			pc586stat[board_number].ifs_collisions +=
											(p_cb->cmd_status & CS_COLLISIONS);

		p_stuff->cmd_timer = (-1);
		next = p_stuff->round_robin;

		/* Insert code for 802.3 station component response here */ 

		p_msg = NULL;
		for (x = 0; x < N_LSAP; x++)
		{
			++next;
			if ( next > (N_LSAP -1) )
				next = 0;

			if (pc586struct[board_number].ptype[next].ptype_state == DL_IDLE)
			{
				p_msg = getq(pc586struct[board_number].ptype[next].write_q );
				if (p_msg != NULL)
					break;
			}
		}

		/*
		 * if getq() on all possible ptypes fail, then no
		 * packets are waiting to be xmt. Thus just return.
		 * The next pc586wput() will see .cmd_timer = -1 and
		 * cause pc586xmt_packet() to be called thus restarting the CU
		 */
		if (p_msg != NULL)
			pc586xmt_packet(pc586struct[board_number].ptype[next].ptype,
														board_number, p_msg);

		/*
		 * round_robin gives round robin service total N_LSAPs, no favoritism
		 */
		p_stuff->round_robin = next;

	} /* end of SCB_INT_CNA */

	return;

f1:
	/*
	 *  If we get here, cause the PC586 to reset.
	 */
	p_stuff->cmd_timer = 0;

	/*
	 * it will be turned on during reset
	 */
	int586_off(board_number);
	pc586debug.bad_status_in_intr++;

	return;
} /* end of pc586intr() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
pc586rcv_packet(board_number)
	int board_number;
{
	fd_t *p_fd;
	board_stuff_t *p_stuff = &pc586struct[board_number].board_stuff;

	/* first... & last... are same as begin_rbd... & end... for one fd */
	rbd_t *p_first_rbd, *p_last_rbd;

	for (p_fd = p_stuff->begin_fd; p_fd != NULL; p_fd = p_stuff->begin_fd)
	{
		if (p_fd->fd_status & CS_CMPLT)
		{
			p_stuff->begin_fd =
				    (fd_t *)pc586_to_virt( board_number,p_fd->fd_nxt_ofst);

			p_first_rbd =(rbd_t *)pc586_to_virt(board_number,p_fd->fd_rbd_ofst);

			/* 
			 * see p2-46 of 586 manual
			 */
			if (p_fd->fd_rbd_ofst != 0xffff)
			{
				p_last_rbd = p_first_rbd;
				while ((p_last_rbd->rbd_status & CS_EOF) != CS_EOF &&
					   (p_last_rbd->rbd_size   & CS_EL)  != CS_EL)
					p_last_rbd = (rbd_t *)pc586_to_virt(board_number,
						    						p_last_rbd->rbd_nxt_ofst);

				p_stuff->begin_rbd =
				  (rbd_t *)pc586_to_virt(board_number,p_last_rbd->rbd_nxt_ofst);

				p_last_rbd->rbd_nxt_ofst = 0xffff;
				if (p_fd->fd_status & CS_OK)
				{
					pc586debug.rcv_count++;
				 	if (pc586put_packet(board_number,p_fd)) 
					{
				 		if (pc586inetstats && pc586stat)
							pc586stat[board_number].ifs_ipackets++;
					}
					else 
					{
						pc586debug.rcv_dropped++;
				 		if (pc586inetstats && pc586stat)
							pc586stat[board_number].ifs_ierrors++;
					}
				}
				else	
				{ 
					pc586debug.bad_rcv_status_count++;
					if (pc586inetstats && pc586stat)
						pc586stat[board_number].ifs_ierrors++;
				}
			}
			pc586re_q_fd(board_number,p_fd);
		}
		else
			break;
	}

	return (pc586ru_start(board_number));
} /* end of pc586rcv_packet */


/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586re_q_fd(board_number, p_fd)
int board_number;
fd_t *p_fd;
{
	rbd_t *p_last_rbd, *p_first_rbd;
	board_stuff_t *p_stuff = &pc586struct[board_number].board_stuff;

	/*
	 * C. Yager's driver example in 586 ref man makes sure
	 * that two fd and two rbd are enqueued before restarting
	 * the RU. The design of this driver ensures that all full
	 * fd/rbds are emptied then re_q ed at interrupt time. Thus
	 * this driver never needs to check for a minimum level
	 * of fd/rbds before calling pc586ru_start(). See chapter 4 of
	 * microcomm handbook.
	 */
	p_first_rbd = (rbd_t *)pc586_to_virt(board_number,p_fd->fd_rbd_ofst);

	p_fd->fd_status   = 0;
	p_fd->fd_cmd      = CS_EL; /* its going to be the last fd on the list */
	p_fd->fd_nxt_ofst = 0xffff;
	p_fd->fd_rbd_ofst = 0xffff;

	/*
	 * this can never happen !!!!!!!!!!!!!!!
	 * if (p_stuff->begin_fd == NULL)
	 *	  p_stuff->begin_fd = p_stuff->end_fd = p_fd;
	 */

	/*
	 * ...end_fd->fd_nxt_ofst MUST be linked
	 * before ...fd_cmd, see p3-8 586 man
	 */
	p_stuff->end_fd->fd_nxt_ofst = virt_to_pc586(board_number, (char*)p_fd);
	p_stuff->end_fd->fd_cmd		 = 0;	/* no last now */
	p_stuff->end_fd				 = p_fd;

	if (p_first_rbd != NULL)
	{
		for (p_last_rbd=p_first_rbd;
				(p_last_rbd->rbd_status & CS_EOF)!= CS_EOF &&
				(p_last_rbd->rbd_size   & CS_EL) != CS_EL;
		    			p_last_rbd= (rbd_t *)pc586_to_virt(board_number,
													p_last_rbd->rbd_nxt_ofst))
		{
			/*
			 * clear eof and act count
			 */
			p_last_rbd->rbd_status = 0;
		}

		p_last_rbd->rbd_status = 0;
		p_last_rbd->rbd_size  |= CS_EL; /* new end of rbd list */

		/*
		 * can this ever happen ??????? !!!!!!!!!!!!!!
		 */
		if (p_stuff->begin_rbd == NULL)
		{
			p_stuff->begin_rbd = p_first_rbd;
			p_stuff->end_rbd   = p_last_rbd;
		}
		else
		{
			/*
			 * end_rbd->rbd_nxt_ofst MUST be linked before ~CS_EL is done
			 */
			p_stuff->end_rbd->rbd_nxt_ofst =
								virt_to_pc586(board_number, (char*)p_first_rbd);
			p_stuff->end_rbd->rbd_size	  &= ~CS_EL;
			p_stuff->end_rbd			   = p_last_rbd;
		}
	}

	return;
} /* end of pc586re_q_fd() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
pc586ru_start(board_number)
	int board_number;
{
	scb_t *p_scb;
	char *p_static_ram;
	fd_t *begin_fd;
	board_stuff_t *p_stuff = &pc586struct[board_number].board_stuff;

	begin_fd     = p_stuff->begin_fd;
	p_static_ram = p_stuff->p_virt_static_ram;
	p_scb        = (scb_t *)( p_static_ram + OFFSET_SCB );

	/*
	 * RU already running -- leave it alone
	 */
	if ((p_scb->scb_status & SCB_RUS_READY) == SCB_RUS_READY)
		return (TRUE);

	/*
	 * Receive queue is exhausted, need to reset the board
	 */
	if (begin_fd == NULL)
	{
		pc586debug.rec_q_empty++;
		return (FALSE);
	}

	/*
	 * if the RU just went not ready and it just completed an fd --
	 * do NOT restart RU -- this will wipe out the just completed fd.
	 * There will be a second interrupt that will remove the fd via
	 * pc586rcv_packet() and thus calls pc586ru_start() which will then
	 * start the RU if necessary.
	 */
	if (begin_fd->fd_status & CS_CMPLT)
		return (TRUE);

	/*
	 * if we get here, then RU is not ready and no completed fd's are avail.
	 * therefore, follow RU start procedures listed under RUC on p2-15
	 */
	pc586debug.rcv_restart_count++;
	begin_fd->fd_rbd_ofst = virt_to_pc586( board_number,
												(char*)(p_stuff->begin_rbd));

	if (wait_scb(board_number, 100))	/* wait up to 1 msec */
		return (FALSE);

	p_scb->scb_rfa_ofst = virt_to_pc586(board_number, (char*)begin_fd);
	p_scb->scb_cmd      = SCB_RUC_STRT;
	chan_attn(board_number);

	return (TRUE);
} /* end of pc586ru_start */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*
 * pc586xmt_packet() is called ONLY by pc586intr() and pc586dl_data_req(). Also,
 * both these routines call wait_scb() just before calling pc586xmt_packet() --
 * therefor, no need to call wait_scb() as the first statement of
 * pc586xmt_packet()
 */
void
pc586xmt_packet(p_type, board_number, mp)
int p_type, board_number;
mblk_t *mp;		/* assuming mp is only DL_unitdata_req type send error msg!! */
{
	int y, jj, bytes_left_over;
	unsigned long *p_userdata, *p_xmtdata;
	pack_ulong_t partial_long;
	pack_ushort_t before_swap, after_swap;
	char *p_static_ram;
	ushort *p_addr;
	ushort *p_destaddr;
	long off;
	int bytes_in_msg;
	scb_t *p_scb;
	cmd_t *p_cb;
	tbd_t *p_tbd;
	DL_unitdata_req_t *p_data_req;
	mblk_t *mp2;

	pc586debug.benchmark = 0; /* used for benchmarking */

x1:
	p_static_ram = pc586struct[board_number].board_stuff.p_virt_static_ram;
	p_scb = (scb_t *)(p_static_ram + OFFSET_SCB);
	p_cb  = (cmd_t *)(p_static_ram + OFFSET_CU);
	p_tbd = (tbd_t *)(p_static_ram + OFFSET_TBD);
	p_data_req = (DL_unitdata_req_t *)(mp->b_rptr);
	off = (p_data_req->RA_offset);
	mp2 = mp->b_cont;

	/*
	 * 1st msg block is dl command header, 2nd is data (if any)
	 */
	pc586struct[board_number].board_stuff.cmd_timer = 3;
	/* .cmd_timer * timeout() value in pc586_watchdog sec max for 586 */

	/*
	 * FIRST>>>FIRST>>>FIRST>>> fill in the 586 command block
	 */
	p_cb->cmd_status   = 0;
	p_cb->cmd_cmd      = CS_EL | CS_CMD_XMIT | CS_INT;
	p_cb->cmd_nxt_ofst = OFFSET_CU;
	/*
	 * only one cb and it points to itself
	 */
	p_cb->prmtr.prm_xmit.xmt_tbd_ofst = OFFSET_TBD;

	p_addr     = (ushort *)(mp->b_rptr + off);
	p_destaddr = (ushort *)(p_cb->prmtr.prm_xmit.xmt_dest);

	/*
	 * bcopy( p_addr, p_destaddr, MAC_ADD_SIZE ); wont work on pc586 !!!!
	 */
	for (y = 0; y < (MAC_ADD_SIZE / 2); y++)
		*p_destaddr++ = *p_addr++;

	before_swap.c.b   = (ushort)p_type;
	after_swap.c.a[0] = before_swap.c.a[1];
	after_swap.c.a[1] = before_swap.c.a[0];

	p_cb->prmtr.prm_xmit.xmt_length=after_swap.c.b;

#ifdef TYPEISINTHEMSG 
	/*
	 * Support for type field intended by upper modules being in the msg
	 */ 
	p_type = *((unsigned short *)(mp->b_rptr + off + MAC_ADD_SIZE ));
	p_cb->prmtr.prm_xmit.xmt_length=p_type;
#endif

	/*
	 * SECOND>>>SECOND>>>SECOND>>> fill in transmit buffer descriptor
	 */
	p_tbd->tbd_count     = ( msgdsize(mp->b_cont) ) | CS_EOF;
	p_tbd->tbd_nxt_ofst  = 0xffff;
	p_tbd->tbd_buff      = OFFSET_TBUF;
	p_tbd->tbd_buff_base = 0;

	/*
	 * THIRD>>>THIRD>>>THIRD>>> put user data in transmit buffer
	 */
	p_xmtdata = (unsigned long *)(p_static_ram + OFFSET_TBUF);

	/*
	 * data in 386 ram does not have to be word
	 * aligned but data on pc586 does
	 *
	 * if msg ends with a partial long, this is written to pc586 board
	 * the <4 garbage bytes will be ignored by 586
	 */
	while (mp2)
	{
		bytes_in_msg = mp2->b_wptr - mp2->b_rptr;
		p_userdata = (unsigned long *)(mp2->b_rptr);
		bytes_left_over = bytes_in_msg & 3;

		pc586bcopy(p_userdata, p_xmtdata, (bytes_in_msg - bytes_left_over));

		p_xmtdata   += (bytes_in_msg - bytes_left_over) / 4;
		mp2->b_rptr += (bytes_in_msg - bytes_left_over);

		if (bytes_left_over)
		{
			for (jj = 0; jj < 4; jj++)
			{
				partial_long.c.a[jj] = *(mp2->b_rptr);
				(mp2->b_rptr)++;

				if (mp2->b_rptr >= mp2->b_wptr)	
					mp2 = mp2->b_cont;

				if (mp2 == NULL)
					break;
			}
			*p_xmtdata++ = partial_long.c.b;
		}
		else
			mp2 = mp2->b_cont;
	}

	/*
	 * FOURTH>>>FOURTH>>>FOURTH>>> make 586 do a transmit
	 */
	p_scb->scb_cmd = SCB_CUC_STRT;
	chan_attn(board_number);

	pc586debug.xmt_count++;
	if (pc586inetstats && pc586stat)
		pc586stat[board_number].ifs_opackets++;

	if (pc586debug.benchmark != 0) /* set to 1 with kernel debug  */
	{
		(void)wait_scb(board_number, 10000);	/* wait up to 100 msec */
		goto x1;
	}

	freemsg(mp);
	return;
} /* end of pc586xmt_packet */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*                      UTILITY ROUTINES FOR THE 82586                        */
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
static void
shuv_word(virtual_addr, value)
ushort *virtual_addr;
ushort value;
{
	*virtual_addr = value;
	return;
} /* end of shuv_word() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
static void
shuv_byte(virtual_addr, value)
char *virtual_addr;
char value;
{
	*virtual_addr = value;
	return;
} /* end of shuv_byte() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
static ushort
pull_word(virtual_addr)
ushort *virtual_addr;
{
	ushort value;
	value = *virtual_addr;
	return(value);
} /* end of pull_word() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586build_cu(board_number) /* inits scp, iscp, scb, cb, tbd and tbuf */
int board_number;
{
	char *p_ram, *p_static_ram;
	cmd_t *p_cb;
	tbd_t *p_tbd;

	p_static_ram = pc586struct[board_number].board_stuff.p_virt_static_ram;

	/*
	 * set up data structs as listed above
	 */
	p_ram = (p_static_ram + OFFSET_SCP);
	((scp_t *)p_ram)->scp_sysbus = 0;

	/*
	 * 16 bit bus see page 2-12 of 586 ref
	 */
	((scp_t *)p_ram)->scp_iscp      = (OFFSET_ISCP);
	((scp_t *)p_ram)->scp_iscp_base = 0;

	p_ram= (p_static_ram + OFFSET_ISCP);
	((iscp_t *)p_ram)->iscp_busy     = 1;
	((iscp_t *)p_ram)->iscp_scb_ofst = OFFSET_SCB;
	((iscp_t *)p_ram)->iscp_scb_base = 0;

	p_ram= (p_static_ram + OFFSET_SCB);
	((scb_t *)p_ram)->scb_status   = 0;
	((scb_t *)p_ram)->scb_cmd      = 0;
	((scb_t *)p_ram)->scb_cbl_ofst = OFFSET_CU;
	((scb_t *)p_ram)->scb_rfa_ofst = OFFSET_RU;
	((scb_t *)p_ram)->scb_crc_err  = 0;
	((scb_t *)p_ram)->scb_aln_err  = 0;
	((scb_t *)p_ram)->scb_rsc_err  = 0;
	((scb_t *)p_ram)->scb_ovrn_err = 0;

	p_cb               = (cmd_t *)(p_static_ram + OFFSET_CU);
	p_cb->cmd_status   = 0;
	p_cb->cmd_cmd      = CS_EL;
	p_cb->cmd_nxt_ofst = OFFSET_CU;
	/* just to be safe - its not needed for "simple" command processing */

	p_tbd=(tbd_t *)(p_static_ram + OFFSET_TBD);
	p_tbd->tbd_count     = 0;
	p_tbd->tbd_nxt_ofst  = 0xffff;  /* "simple" cmnd processing ref page 3-6 */
	p_tbd->tbd_buff      = 0;		/* gets proper value in pc586xmt_packet() */
	p_tbd->tbd_buff_base = 0;

	return;
} /* end of pc586build_cu() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/* builds linear linked lists of fd's and
 * rbd's see page 4-32 of 1986 intel microcomm handbook
 */
void
pc586build_ru(board_number)
int board_number; 
{
	fd_t *p_fd;
	int x;
	typedef struct {
		rbd_t r;
		char rbd_pad[2];		/* puts rbuffer[] on 4 byte boundry */
		char rbuffer[RCVBUFSIZE];
	} ru_t;
	ru_t *p_rbd;
	board_stuff_t *p_stuff = &pc586struct[board_number].board_stuff;

	p_fd = (fd_t *)(p_stuff->p_virt_static_ram + OFFSET_RU);
	p_stuff->begin_fd = p_fd;
	for (x = 0; x < N_FD; x++)
	{
		p_fd->fd_status   = 0;
		p_fd->fd_cmd      = 0;
		p_fd->fd_nxt_ofst = virt_to_pc586(board_number, (char *)(p_fd + 1));
		p_fd->fd_rbd_ofst = 0xffff;		/* must be 0xffff see page 2-46 */
		p_fd++;
	}

	/*
	 * point to &fd[N_FD-1]
	 */
	p_stuff->end_fd   = --p_fd;
	p_fd->fd_nxt_ofst = 0xffff;			/* nothing to point to */
	p_fd->fd_cmd      = CS_EL;			/* end of list         */

	/*
	 * p_stuff->begin_fd, like p_stuff->end_fd is used to manage buffer pools.
	 * It is used to allocate and deallocate frame descriptors by the driver
	 */
	p_rbd = (ru_t *)(p_stuff->p_virt_static_ram + OFFSET_RBD);
	p_stuff->begin_rbd = (rbd_t *)p_rbd;

	/*
	 * the first fd will point to the linked list of rbd's
	 */
	p_fd->fd_rbd_ofst = virt_to_pc586(board_number, (char *)p_rbd);
	for (x = 0; x < N_RBD; x++)
	{
		p_rbd->r.rbd_status    = 0;
		p_rbd->r.rbd_nxt_ofst  = virt_to_pc586(board_number,(char *)(p_rbd+1));
		p_rbd->r.rbd_buff      = virt_to_pc586(board_number, p_rbd->rbuffer);
		p_rbd->r.rbd_buff_base = 0;
		p_rbd->r.rbd_size      = RCVBUFSIZE;
		p_rbd++;
	}

	p_stuff->end_rbd       = (rbd_t *)(--p_rbd);
	p_rbd->r.rbd_nxt_ofst  = 0xffff;     /* last rbd points to ground */
	p_rbd->r.rbd_size     |= CS_EL;      /* eof on the last rbd */

	return;
} /* end of pc586build_ru() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * does 586 op-code number 7
 */
diagnose_586(board_number)
int board_number;
{
	char  *p_static_ram;
	scb_t *p_scb;
	cmd_t *p_cb;
	int x;

	p_static_ram = pc586struct[board_number].board_stuff.p_virt_static_ram;
	p_scb = (scb_t *)(p_static_ram + OFFSET_SCB);
	p_cb  = (cmd_t *)(p_static_ram + OFFSET_CU);

	/*
	 *  wait up to 10 msec for scb to be ready
	 */
	if (wait_scb(board_number, 1000))
		return (FALSE);

	p_scb->scb_cmd = ((p_scb->scb_status) & SCB_INT_MSK);
	if (p_scb->scb_cmd)
		chan_attn(board_number);	/* ack the status bits */

	/*
	 *  wait up to 10 msec for scb to be ready
	 */
	if (wait_scb(board_number, 1000))
		return (FALSE);

	p_cb->cmd_status = 0; /* 586 will write this */
	p_cb->cmd_cmd    = CS_CMD_DGNS | CS_EL;
	p_scb->scb_cmd   = SCB_CUC_STRT;
	chan_attn(board_number);

	/*
	 *  wait up to 10 msec for response.
	 */
	for (x = 100; x; x--)
	{
		if (p_cb->cmd_status & CS_OK)
			break;
		drv_usecwait(10);	/* wait 10 usec before next try */
	}
	if (x == 0)
		return(FALSE);

	p_scb->scb_cmd = (p_scb->scb_status) & SCB_INT_MSK;
	if (p_scb->scb_cmd)
		chan_attn(board_number);

	return(TRUE);
} /* end of diagnose_586() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*
 * std config then an ia-setup (read prom)
 */
config_586(board_number)
int board_number;
{
	ushort *p_addr, *p_addr2;
	char *p_static_ram;
	scb_t *p_scb;
	cmd_t *p_cb;
	int x;

	p_static_ram = pc586struct[board_number].board_stuff.p_virt_static_ram;
	p_scb = (scb_t *)(p_static_ram + OFFSET_SCB);
	p_cb  = (cmd_t *)(p_static_ram + OFFSET_CU);

	/*
	 *  wait up to 10 msec for scb to be ready
	 */
	if (wait_scb(board_number, 1000))
		return (FALSE);

	p_scb->scb_cmd = (p_scb->scb_status) & SCB_INT_MSK;
	if (p_scb->scb_cmd)
		chan_attn(board_number); /* ack the stat bits */

	/*
	 *  wait up to 10 msec for scb to be ready
	 */
	if (wait_scb(board_number, 1000))
		return (FALSE);

	p_cb->cmd_status = 0; /* 586 will write this */
	p_cb->cmd_cmd    = CS_CMD_CONF | CS_EL;

	/*
	 * default config p2-28 586 book
	 */
	p_cb->prmtr.prm_conf.cnf_fifo_byte = 0x080c;
	p_cb->prmtr.prm_conf.cnf_add_mode  = 0x2600;
	p_cb->prmtr.prm_conf.cnf_pri_data  = 0x6000;
	p_cb->prmtr.prm_conf.cnf_slot      = 0xf200;
	p_cb->prmtr.prm_conf.cnf_hrdwr     = 0x0000;
	p_cb->prmtr.prm_conf.cnf_min_len   = 0x0040;

	p_scb->scb_cmd = SCB_CUC_STRT;
	chan_attn(board_number);

	/*
	 *  wait up to 10 msec for response.
	 */
	for (x = 100; x; x--)
	{
		if (p_cb->cmd_status & CS_OK)
			goto c1;
		drv_usecwait(10);	/* wait 10 usec before next try */
	}

	return(FALSE);
c1:
	p_scb->scb_cmd = (p_scb->scb_status) & SCB_INT_MSK;
	if (p_scb->scb_cmd)
		chan_attn(board_number);

	/*
	 * now do ia set-up, first read 586 half-card prom
	 */
	if (wait_scb(board_number, 1000))	/* wait up to 10 msec */
		return (FALSE);

	p_scb->scb_cmd = (p_scb->scb_status) & SCB_INT_MSK;
	if (p_scb->scb_cmd)
		chan_attn(board_number); /* ack the stat bits */

	/*
	 *  wait up to 10 msec for scb to be ready
	 */
	if (wait_scb(board_number, 1000))
		return (FALSE);

	p_cb->cmd_status = 0; /* 586 will write this */
	p_cb->cmd_cmd    = CS_CMD_IASET | CS_EL;

	p_addr  = (ushort *)&( pc586struct[board_number].board_stuff.mac_add[0] );
	p_addr2 = (ushort *)&(p_cb->prmtr.prm_ia_set[0]);

	for (x = 0; x < (MAC_ADD_SIZE / 2); x++)
		*p_addr2++ = *p_addr++;

	p_scb->scb_cmd = SCB_CUC_STRT;
	chan_attn(board_number);
	/*
	 *  wait up to 10 msec for response.
	 */
	for (x = 100; x; x--)
	{
		if (p_cb->cmd_status & CS_OK)
			break;
		drv_usecwait(10);	/* wait 10 usec before next try */
	}

	if (x == 0)
		return(FALSE);

	p_scb->scb_cmd = (p_scb->scb_status) & SCB_INT_MSK;
	chan_attn(board_number);

	return(TRUE);
} /* end of config_586() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
static
prom_address(board_number, index)
int board_number, index;
{
char *p_cmd_prom;

	p_cmd_prom  = pc586struct[board_number].board_stuff.p_virt_cmd_prom;
	p_cmd_prom += OFFSET_ADDR_PROM;
	p_cmd_prom += index;

	return(*p_cmd_prom);
} /* end of prom_address() */
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
static void
chan_attn(board_number)
int board_number;
{
	char *p_hwcomm, *p_cmd_prom;

	p_cmd_prom = pc586struct[board_number].board_stuff.p_virt_cmd_prom;
	p_hwcomm   = (p_cmd_prom + OFFSET_CHAN_ATT);

	/*
	 * first byte of word is 1 - this sets the CA
	 * second byte of word is zero - this clears CA
	 */
	shuv_word((ushort*)p_hwcomm, 0x01);

	return;
} /* end of chan_attn() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*
 * Acceptance of a Control Command is indicated
 * by the 82586 clearing the SCB command field
 * page 2-16 of the intel microcom handbook
 */
static
wait_scb(board_number, how_long)
int board_number;
int	how_long;
{
	scb_t *p_scb;
	char *p_static_ram;

	p_static_ram = pc586struct[board_number].board_stuff.p_virt_static_ram;
	p_scb        = (scb_t *)(p_static_ram + OFFSET_SCB);

	/*
	 *  Wait as long as the caller wants.
	 */
	while (how_long--)
	{
		if (p_scb->scb_cmd == 0)
			return(FALSE);
		drv_usecwait(10);	/* wait 10 usec before next try */
	}

	pc586debug.scb_timeouts++;

	return(TRUE);
} /* end of wait_scb() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*
 * interrupt on out at 586 card
 */
void
int586_on(board_number)
int board_number;
{
	char *p_hwcomm, *p_cmd_prom;

	p_cmd_prom = pc586struct[board_number].board_stuff.p_virt_cmd_prom;
	p_hwcomm   = (p_cmd_prom + OFFSET_INT_ENAB);
	shuv_word((ushort*)p_hwcomm, CMD_1);

	return;
} /* end of int586_on() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *
 * interrupt off out at 586 card
 */
void
int586_off(board_number)
int board_number;
{
	char *p_hwcomm, *p_cmd_prom;

	p_cmd_prom = pc586struct[board_number].board_stuff.p_virt_cmd_prom;
	p_hwcomm   = (p_cmd_prom + OFFSET_INT_ENAB);
	shuv_word((ushort*)p_hwcomm, CMD_0);

	return;
} /* end of int586_off() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
ushort
virt_to_pc586(board_number, kernel_virt_addr)
int		board_number;
char	*kernel_virt_addr;
{
	char *p_static_ram;
	ushort pc586;

	/*
	 * 586 uses 0xffff for null as "c" uses 0 for null
	 */
	if (kernel_virt_addr == NULL)
		return(0xffff);

	if ((board_number > pc586_boards-1) || (board_number < 0))
		return(0xffff); /* error */

	p_static_ram = pc586struct[board_number].board_stuff.p_virt_static_ram;
	if (p_static_ram > kernel_virt_addr)
		return(0xffff);	/* error */

	pc586 = (ushort)(kernel_virt_addr - p_static_ram);
	if (pc586 >0x7fff)
		return(0xffff); /* error */

	return(pc586);
} /* end of virt_to_pc586() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
char *
pc586_to_virt(board_number, pc586_addr)
int board_number;
ushort pc586_addr;
{
	/*
	 * 586 uses 0xffff for null as "c" uses 0 for null
	 */
	if (pc586_addr == 0xffff)
		return (NULL);

	if (board_number > pc586_boards - 1 || board_number < 0)
		return(NULL); /* error */

	if (pc586_addr > 0x7fff)
		return(NULL);

	return(pc586struct[board_number].board_stuff.p_virt_static_ram+pc586_addr);
} /* end of pc586_to_virt() */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
char
pc586_ntoa(i)
unsigned char i;
{
	if (i <= 9)
		return(i+0x30);
	else
		return(i+0x57);
}

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
void
pc586_print_addr(eaddr)
unsigned char eaddr[];
{
	int i;
	char a_eaddr[18];

	for (i = 0; i < 6; i++)
	{
		a_eaddr[i*3]   = pc586_ntoa(eaddr[i]>>4&0xf);
		a_eaddr[i*3+1] = pc586_ntoa(eaddr[i]&0xf);
		a_eaddr[i*3+2] = ':';
	}
	a_eaddr[17] = '\0';
	cmn_err(CE_CONT, "--- Ethernet Address: %s\n", a_eaddr);
}
