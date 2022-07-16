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

#ident	"@(#)mbus:uts/i386/io/enetdrv/i530hwdep.c	1.3.2.1"

#ident "@(#)i530hwdep.c  $SV_enet SV-Eval01 - 06/25/90$"

/*
 *
 *   i530hwdep.c: main routines for the stream driver for the Intel 186/530
 *           transport board with iNA960/961 firmware.
 *
 */
/*
 * MODIFICATIONS:
 *
 *	I000	June 22, 1988	rjs
 *		Verify 186/530 board presence via interconnect space driver.
 *		Remove ina download logic -- not used in MBII.
 *		Don't reset board -- will be done by /usr/lib/cci/download
 *		invocation.
 *	I001	Sept 18, 88	rjs
 *		Formerly wakeup at endpoint's str_state field; nobody sleeps
 *		there.  However, closing process may sleep in enetclose()
 *		waiting for output to complete.  Wake those proc's up if
 *		reseting.
 *	I002	9/28/88		rjs
 *		Default ina on 186/530 is R3.0.
 *	I003	11/23/88	rjs
 *		Defined base_tsap variable.  Initialized to word having
 *		slot id in high byte and bit 14 and 15 set.
 *	I004	01/29/89	rjs
 *		Added slot independence logic to enetinit().
 *	I005	01/29/89	rjs
 *		fixed iNA961_reset bug
 *	I006	07/11/89	rjf
 *		Made lint fixes.
 *	I007	10/16/89	rjf
 *		Added support for MIX board.
 */

#define DEBUG 1

#include "sys/enet.h"
#include "sys/lcidef.h"
#include "sys/ics.h"
#include <sys/file.h>
#include <sys/debug.h>

static char i530_rcsid[] = "@(#)Driver.o  $SV_enet SV-Eval01 - 06/25/90$";
static char i530_copyright[] = "Copyright 1988, 1989 Intel Corp. 464228";

#define ICS_INIT_WANTED	0x80		/* I005 */
#define ICSREG		0x44		/* I005 */

extern unsigned char ics_myslotid();

ushort enet_base_tsap;		/* I003 */

extern int cdevcnt;
extern int enet_debug;

extern int enetwput(), enetwsrv();
extern void enet_init_buffers();
extern void enet_closed_open_complete();

/*
 * The following are major variables defined in the /etc/master.d
 * file for 186/530:
 *
 *	enet_endpoints: One for each STREAM (aka virtual circuit)
 *	enet_n_endpoints: Max number of possible endpoints
 */

extern endpoint		enet_endpoints[];
extern int		enet_n_endpoints;
extern int		enet_rbtabsize;
extern int		enet_nvc;
extern struct enetboard	enet_boards[];
extern struct enetcfg	enet_cfg[];
extern int		enet_majtobnum[];
extern int		enet_resetting;
extern ulong		enet_stat[];


/*
 * enetinit
 *
 * Called during system initialization to sense the device and report if its
 * there.  It also initializes the lci communications areas and some internal
 * state.
 */
void
enetinit()
{
	struct enetboard *board_p;
    	ushort i530slot;
	int bnum;
	endpoint *ep;
	int i, j;					/* I007 */
	char board_name[10];				/* I007 */
	int *ip;		/* for walking through enet_majtobnum	*/
	extern int enet_n_boards;	/* previously N552	*/
	extern int enet_ina_ver;
	extern char *enet_ics_name_ptr[];

	ASSERT(cdevcnt<=256);	/* can't cope with this one		*/

	for(ip= &enet_majtobnum[255]; *ip != -1; ip--)
		*ip = -1;

	enet_ina_ver = 31;
	enet_base_tsap = 0xF000 | ((ushort)ics_myslotid() << 8); /* I003 */

    	DEBUGP(DEB_CALL,(CE_CONT, "186/530 Starting board scan\n"));

    	for (i530slot = 0, bnum = 0;			/* I004 */
	     (bnum < enet_n_boards) && (i530slot < ICS_MAX_SLOT);
	     i530slot++){

    		if (ics_agent_cmp(enet_ics_name_ptr, i530slot) != 0) 
            		continue;
		/* I007 Start */
		for (j=0x2; j <= 0xb; j++)
			board_name[j-2] = (unchar)ics_read(i530slot, j);
		cmn_err(CE_CONT, "iSBC %s board in slot %x found.\n",
							board_name, i530slot);
		/* I007 End */
		board_p = &enet_boards[bnum];
		board_p->state |= PRESENT;
		board_p->slot_id = i530slot;
		enet_cfg[bnum].slot_id = i530slot;

		bnum++;
	}
	DEBUGP(DEB_CALL,(CE_CONT, "enetinit 3; About to call lciinit()\n"));
	lciinit(bnum); 
	for (ep=&enet_endpoints[0]; ep<&enet_endpoints[enet_n_endpoints]; ep++) {
		/*
		 * Make sure everything starts out clean.
		 */
		ep->tli_state = 0;
		ep->str_state = C_IDLE;
	}
	/*
	 * Make sure there are no entries in the major device id map to
	 * non-existant boards
	 */
	for(i=0;i<cdevcnt;i++) {
		if(enet_majtobnum[i] == -1)
			continue;
		if(enet_majtobnum[i] >= enet_n_boards) {
			cmn_err(CE_WARN, "Ethernet Driver: Major number %d mapped to board %d (not configured) - disabling\n",
				i, enet_majtobnum[i]);
			enet_majtobnum[i] = -1;
			continue;
		}
		if(!(enet_boards[enet_majtobnum[i]].state & PRESENT)) {
			/* cmn_err(CE_WARN, "Ethernet Driver: Major number %d mapped to board %d (not present) - disabling\n",
				i, enet_majtobnum[i]); */
			enet_majtobnum[i] = -1;
		}
	}
	/*
	 * Initialize the request block and pending connection buffer pools
	 */
	DEBUGP(DEB_CALL,(CE_CONT, "enetinit 4; About to call enet_init_buffers()\n"));
	enet_init_buffers();
	DEBUGP(DEB_CALL,(CE_CONT, "enetinit 5; End of enetinit()\n"));
}


/*
 *  enetfull_reset()
 *
 *	function: reset the specified enetboard, the corresponding mip queues,
 *		  release the rb's queued up in the mip queues.
 *	input: bnum - board number
 */
void
enetfull_reset(bnum)
int	bnum;
{

	extern    unsigned long lci_rb_chan;
	extern    unsigned long lci_xmit_chan;
	extern    unsigned long lci_rcv_chan;
	extern    struct  req_blk   enet_rb_list[];

	endpoint		*ep;
	int			x;
	struct	req_blk		*rb_p;	
	int			i;

	if (enet_stat[ST_CURO] > 1)
	{
		cmn_err(CE_WARN, "Reset failed - close all vc's before reset\n");
		cmn_err(CE_CONT, "  %d currently open endpoints\n", enet_stat[ST_CURO]-1);
		enet_resetting = 0;
		return;
	}
	
	x = SPL();

	for(ep=&enet_endpoints[0]; ep<&enet_endpoints[enet_n_endpoints]; ep++) {
		if ((ep->bnum == bnum) && ((ep->l_reference != 0) ||
			(ep->c_reference != 0)))
		{
			DEBUGP(3,(CE_CONT, "enetfull_reset: sending C_DISCON upstream\n"));
			if (ep->tli_state != 0)
				if((discon_ind(ep, EIO, -1)) == FALSE) 
				{	
					DEBUGP(3,(CE_CONT, "enetfull_reset: discon_ind returns err"));
				}					
			if(ep->flow)
				freemsg(ep->flow);
			ep->flow = NULL;
			flushq(WR(ep->rd_q), FLUSHALL);
			wakeup((caddr_t)ep);			/* I001 */
			enableok(WR(ep->rd_q));
			qenable(WR(ep->rd_q));
			ep->nbr_pend = 0;
			ep->pend_connects = NULL;
			ep->l_reference = 0;
			ep->c_reference = 0;

		}					
	}
	/*
	 * 	Release posted buffers and re-initialize rb table.
	 *
	 */

	for(i=0; i<enet_rbtabsize; i++)
	{
		rb_p = &enet_rb_list[i];
		if ((rb_p->in_use) && (rb_p->databuf != 0))
			freemsg(rb_p->databuf);
	}

	if (mps_close_chan((long)lci_rb_chan) != 0)
	{
		cmn_err(CE_WARN, "Error in closing the lci_rb_chan\n");
		splx(x);	
	}
	else if (mps_close_chan((long)lci_xmit_chan) != 0)
	{
		cmn_err(CE_WARN, "Error in closing the lci_xmit_chan\n");
		splx(x);	
	}
	else if (mps_close_chan((long)lci_rcv_chan) != 0)
	{
		cmn_err(CE_WARN, "Error in closing the lci_rcv_chan\n");
		splx(x);	
	}
	else
	{
		enetreset(bnum);
		lciinit(bnum); 
		enet_init_buffers(); 
		splx(x);	
		cmn_err(CE_NOTE, "Board at slot %d reset\n",
				enet_boards[bnum].slot_id);
	}
}


void
done_or_delay(addr)
unchar	*addr;
{
	register int	i;

	for (i = 0; (*addr == 0) && (i < IDELAY); i++) {
		delay(1);
	}
}

/*
* iNA961_reset
*
* Reset the comm board
*/
void
iNA961_reset(bnum)
int bnum;
{
	ushort slot_id;

	slot_id = enet_boards[bnum].slot_id;

	if (  (ics_write((ushort) slot_id, (ushort)ICS_GCR,	/* I005 */
			(unchar)ICS_LOCAL_RESET) == -1)
	    ||(ics_write(slot_id, ICSREG, ICS_INIT_WANTED) ==  -1)
	   )
		cmn_err(CE_WARN, "Error reseting Ethernet controller in slot %x\n", slot_id);

	delay(100);
	if (ics_write((ushort) slot_id, (ushort)ICS_GCR, 0) == -1)
		cmn_err(CE_WARN, "Error reseting Ethernet controller in slot %x.\n", slot_id);
}

/*
 * codeblock()
 *
 * Downloading of iNA is done independent from the driver via the
 * application /usr/lib/cci/download.  This hook exists to satify references
 * in enetutil.c
 *
 */
int
iNA961_codeblock(bnum, bimg)
int	bnum;
inabimg	bimg;		/* Boot image header */
{
	return(1);	/* return error */
}
