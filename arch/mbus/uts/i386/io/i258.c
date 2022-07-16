/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/i258.c	1.4.3.3"

#ifndef lint
static char i258copyright[] = "Copyright 1987, 1988, 1989, 1990 Intel Corporation 462847";
#endif

/*
#undef TRACE
#undef RUNINDBG
#undef MAGICTRACE
*/

#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/iobuf.h"
#include "sys/cmn_err.h"
#include "sys/ics.h"
#include "sys/mps.h"
#include "sys/alttbl.h"
#include "sys/fdisk.h"
#include "sys/ivlab.h"
#include "sys/vtoc.h"
#include "sys/bbh.h"
#include "sys/bps.h"

#include "sys/kmem.h"
#include "sys/cred.h"
#include "sys/uio.h"
#define b_dev b_edev

#include "sys/i258.h"
#include "sys/ddi.h"
#include "sys/file.h"
#include "sys/elog.h"
#ifdef DEBUG
#include "sys/xdebug.h"
#endif

#ifdef wakeup
#undef wakeup
#endif

#define EOK 0x0L

extern unsigned char    ics_myslotid();    /* From ics.c */

/* Values configured in i258/space.c */

extern int    N_i258;              /* Number of boards */
extern int    i258retry;             /* Number of retries on soft error */
extern int    i258print_warnings;	/* whether or not to print warnings */
extern int    i258_max_req;          /* Max outstanding requests per brd */
extern struct iobuf		i258tab[];    /* Queue headers, one per board */
#ifndef NOSAR
extern struct iotime	i258stat[][NUMUNITS];	/* I/O statistics */
#endif
extern 	minor_t i258maxmin;			/* maximum minor device number */
extern 	int 	i258pci_bin[];		/* list of PCI controller instances */
extern 	int 	i258pci_sin[];		/* list of PCI server instances */

extern struct i258cfg   i258cfg[];      /* 258 "configuration" */
extern struct i258dev   i258dev[];      /* per-board device data */
extern struct i258minor i258minor[];    /* minor number bit map */
extern struct i258wini  i258winidata[][I258_NUMWINI]; /* Misc data per wini */
extern struct i258rdvfy  i258rvdata[][I258_NUMWINI]; /* ioctl data per wini */

/*
 * bad block handling parameters
 */
extern int i258no_trk_zone;			/* number of tracks per zone */
extern int i258no_alt_sec_zone;		/* the number of slip sectors/zone */
extern int i258no_alt_trk_zone;		/* the number of alt tracks/zone */
extern int i258no_alt_cyl_vol;		/* the number of alternate cylinders */

/*
 *  Configurable Unit options
 */

extern unchar	i258sio_mode;
extern unchar	i258sio_cmd_ordering;
extern unchar	i258sio_cache_size;
extern unchar	i258sio_nretries;
extern unchar	i258sio_read_ahead;


#define			MAX_SERVERS		2

unsigned int 	i258ST506loc();

unsigned short	i258max;         	/* Number of boards found in scan */
long			i258brd_chan = -1;	/* Channel to talk to boards */
long			i258drv_chan = -1;	/* Channel to talk to other drivers */
struct i258resp	i258pci_resp [ICS_MAX_SLOT] = {0};	/* max. locate responses */
struct i258resp	i258pci_servers [MAX_SERVERS*ICS_MAX_SLOT] = {0};
int				i258pciresp = 0;
int				i258pcitimeout = 0;
int				i258pci_bcount = 0;
int				i258pci_scount = 0;
int				i258querydevtype = 0; /* The device type for query Controller */

#ifndef NOSAR
/* I/O statistics for SAR */
int				i258sar_outstanding = 0;/* no. jobs sent, but not done */
time_t			i258sar_base_time = 0;	/* time last non-overlapped io sent */
time_t			i258sar_finish_time = 0;/* time last job completed */
#endif

#ifdef DEBUG
#define DEB_INIT     0x00000001    /* i258init */
#define DEB_START    0x00000002    /* i258istart */
#define DEB_IO       0x00000004    /* i258io */
#define DEB_IOM      0x00000008    /* i258io: Display Message */
#define DEB_IOB      0x00000010    /* i258io: Break before send_rsvp */
#define DEB_BMSG     0x00000020    /* i258bmsg */
#define DEB_OPEN     0x00000040    /* i258open */
#define DEB_STRAT    0x00000080    /* i258strategy */
#define DEB_CHECK    0x00000100    /* i258checkerr */
#define DEB_SETUP    0x00000200    /* i258setup */
#define DEB_BMSGM    0x00000400    /* i258bmsg: showmsg */
#define DEB_BMSGB    0x00000800    /* i258bmsg: break at start*/
#define DEB_BMSGMU   0x00001000    /* i258bmsg: show unsol msg */
#define DEB_BMSGBU   0x00002000    /* i258bmsg: break at unsol msg */
#define DEB_BMSGPR   0x00004000    /* i258checkerr: force message */
#define DEB_BMSGT    0x00008000    /* i258bmsg: tape only */
#define DEB_FLMK     0x00010000    /* i258flmk */
#define DEB_IOCTL    0x00020000    /* i258ioctl */
#define DEB_SIZE     0x00040000    /* i258size */
#define DEB_CHECKM   0x00080000    /* i258checkerr */
#define DEB_CLOSE    0x00100000    /* i258close */
#define DEB_BBH      0x00200000    /* i258 Bad Block Handler routines */
#define DEB_FMT	     0x00400000    /* i258 Format routines */
#define DEB_WILF     0x00800000    /* What I am looking for */
#define DEB_SAR      0x01000000    /* SAR statistics gathering */

unsigned long i258_debug = 0;

#define DEBPR(x,y)    if(i258_debug & (x)) cmn_err y

#else

#define DEBPR(x,y)

#endif /* DEBUG */

#ifdef  TRACE
#define TRSIZE   128
unsigned long    i258log;
unsigned long    i258tridx;
unsigned long    i258trace[TRSIZE];
#define DEBTR(x,y)	i258trace[(TRSIZE-1)-i258tridx] = \
								(x&0xff)|(((ulong)y&0xffffff)<<8); \
					i258tridx = (i258tridx+1)%TRSIZE;
#else
#define DEBTR(x,y)
#endif /* TRACE */

#define	SETERR(x)			error_code = x
#define	ERRRET(x)			return(x)
#define USER_WINI_LOCK		(error_code = i258swl(NULL, dev, 1, wd->lock)) != 0
#define SUSER_WINI_LOCK		(error_code = i258swl(cred_p, dev,1,wd->lock)) != 0
#define SUSER_WINI_NOLOCK	(error_code = i258swl(cred_p, dev,0,0)) != 0

/*
* On AMPsend_rsvp(), with solicited reply,  Completion message returned (when 
* service is complete) is last buffer request (eot set) received by server.
*/
/* UPDATE: this define belongs in transport */
#define msg_isrsvp_solreperr(mbp) (((mbp)->mb_data[MPS_MG_MT]==MPS_MG_BREQ)?(((mbp)->mb_data[MPS_MG_BRTC]&(MPS_MG_RRMSK|MPS_MG_RRTMSK))!=MPS_MG_RSET):1)

unchar	i258checkerr();
int 	i258resrel();
void 	i258add_resp();

#ifndef __STDC__
int		i258bmsg();    /* Needed for the mps_open_chan call */
int		i258read();
int		i258write();
int 	i258open();
int 	i258close();
void 	i258strategy();
void 	i258halt();
#endif

#ifdef MULTICPU
i258dmsg();    /* Needed for the mps_open_chan call */
#endif

/*
 * Assign unit numbers to the i258winidata, i258rdvfy and i258drtab structures.
 * There are only NUMWINI or NUMSPINDLE instances of these structures allocated 
 * to avoid wasting memory. At the same time, the possible unit numbers can
 * go upto NUMUNITS. So for these structures, we assign unit number to these
 * instances.
 */
i258assignstructs (brd, dd)
int	   brd;
struct i258dev *dd	;
{
	int	i	;
	int	j	;
	
	i = 0	;
	for (j=0; j<I258_QUERY_CONTROLLER_SIZE; j++) {
		if ( (dd->w_units[j]) & 0x1) { /* There is a wini here. */
			i258winidata[brd][i].unitnum = j	;
			i258rvdata[brd][i].unitnum   = j	;
			++i;	
			if (i == I258_NUMWINI) 
				break	;
		}
	}	

	i = 0;
	for (j=0; j<I258_QUERY_CONTROLLER_SIZE; j++) {
		if (((dd->w_units[j]) & 0x1) || ((dd->f_units[j]) & 0x1) ||
		    ((dd->t_units[j]) & 0x1)) {
			dd->d_drtab[i++].unitnum = j;
			if (i == NUMSPINDLE) {
				cmn_err (CE_WARN, 
					"i258assignstructs: More than NUMSPINDLE devices connected\n");
				cmn_err (CE_WARN, 
					"                   Ignoring them\n");
				break;
			}
		
		}
	}
}

/*
 * Given unit number and board number, this function returns a pointer to the 
 * appropriate i258winidata structure.
 */
struct	i258wini *
i258winidata_struct(brd, unit)
int	brd, unit;
{
	int		i;

	for (i=0; i<I258_NUMWINI; i++) 
		if (i258winidata[brd][i].unitnum == unit)
			return (&i258winidata[brd][i]);
	
	cmn_err (CE_PANIC, 
		"i258: Cannot find winidata structure for board %d unit %d\n", brd, unit);
}

/*
 * Given unit number and board number, this function returns a pointer to the 
 * appropriate i258rdvfy structure.
 */
struct	i258rdvfy *
i258rdvfy_struct(brd, unit)
int	brd, unit;
{
	int		i;

	for (i=0; i<I258_NUMWINI; i++) 
		if (i258rvdata[brd][i].unitnum == unit)
			return (&i258rvdata[brd][i]);

	cmn_err (CE_PANIC, 
		"i258: Cannot find rdvfy structure for board %d unit %d\n", brd, unit);
}

/*
 * Given board number and unit number, this function returns a pointer to the 
 * appropriate i258drtab structure.
 */
struct	i258drtab *
i258drtab_struct(brd, unit)
int		brd, unit;
{
	int		i;
	struct 	i258dev	*dd;

	dd = &i258dev[brd];
	for (i=0; i<NUMSPINDLE; i++)  {
		if (dd->d_drtab[i].unitnum == unit)
			return (&(dd->d_drtab[i]))	;
	}
	return (NULL);	
}

/*
 * A simplified timeout for locate PCI.  Sets the global flag and will
 * wakeup the proc sleeping on i258pciresp
 */

void
i258loctimeout(var1)
int		var1;
{
	i258pcitimeout++;
	(void) wakeup((caddr_t) &i258pciresp);
}

/*
 *	This code implements a broadcast locate protocol.
 *	This should ideally be modified to use the location service when
 *	that gets implemented.
 *
 *	The algorithm used here are:
 *	Send a broadcast message (locate PCI)
 *	Recieve 0 or more responses
 *	For each response
 *		send a get_server_info with command = 0
 *		receive 0 or 1 response for number of servers
 *		for each of the number of servers present
 *			send a get_server_info with command = 1
 *
 *	Select the server(s) based on configuration info and what's available.	
 */

void
i258locatepci()
{	
	struct i258PCM		pcm;
	unsigned char		our_tid, stoff, st;
	int					bin, sin, index, s_cnt, i, j, k, temp;
	unsigned short		l_cnt, hid;
	mps_msgbuf_t 		*mbp;
	mb2socid_t			socid;
	struct	i258dev		*dd;

	DEBPR(DEB_START,(CE_CONT, "i258locatepci: \n"));


	if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == (mps_msgbuf_t *)0)
		cmn_err (CE_PANIC, "i258locatepci: cannot get message buffers\n");

	pcm.P_cmd_type = LOCATE_PCI;
	pcm.P_resv = 0;
	i258pciresp = 0;
	i258pcitimeout = 0;
	i258pci_scount = 0;				/* total # of servers */

	/* find any server which responds */

	mps_mk_brdcst(mbp,PCI_PORT_ID,(unsigned char *)&pcm,PCMCLEN);
	if (mps_AMPsend(i258brd_chan, mbp) == -1)
		cmn_err(CE_PANIC, "i258locatepci: Broadcast failure. mbp=%x\n", mbp);
	(void) timeout(i258loctimeout, (caddr_t) &i258pcitimeout, (WATCH_TIME*HZ));
	(void) sleep( (caddr_t) &i258pciresp, PZERO);
	if (i258pciresp == 0) {
		/* return for now, so that we can retry again on open */
		cmn_err(CE_CONT, "i258locatepci: No response from PCI server(s)\n");
		return;
	}
	/*
	 * for each server that responded, do a get_server_info to figure out
	 * the total number of servers in the system, arrange the responses in 
	 * sorted order and select the servers the user or configuration 
	 * specified
	 */
	for (l_cnt = 0; l_cnt < ICS_MAX_SLOT; l_cnt++) {
		if (i258pci_resp[l_cnt].portid == 0)
			continue;

		if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == (mps_msgbuf_t *)0)
			cmn_err (CE_PANIC, "i258locatepci: cannot get message buffers\n");
		hid = i258pci_resp[l_cnt].hostid; 
		socid = mps_mk_mb2socid(hid, i258pci_resp[l_cnt].portid);
		pcm.P_cmd_type = GET_SERVER_INFO;
		pcm.P_union1.P_server.cmd = 0;		/* return # of servers */
		i258pci_resp[l_cnt].stoff = hid * MAX_SERVERS; 
		if((our_tid = mps_get_tid(i258brd_chan)) == (unchar)0) {
			cmn_err(CE_PANIC,"i258locatepci: cannot get transaction id\n");
		}
		mps_mk_unsol(mbp, socid, our_tid,
				   	 (unsigned char *)&pcm, PCMCLEN);
		i258pcitimeout = 0;
		i258pciresp = 0;
		if (mps_AMPsend_rsvp(i258brd_chan, mbp, NULL, NULL) == -1)
			cmn_err(CE_PANIC, "i258locatepci: sendrsvp failure. mbp=%x\n", 
					mbp);
		(void) timeout( i258loctimeout, (caddr_t) &i258pcitimeout, 
					(WATCH_TIME*HZ) );
		sleep( (caddr_t) &i258pciresp, PZERO);
		if (i258pcitimeout) {
			cmn_err(CE_CONT, 
			"i258: No response to GET_SERVER_INFO from PCI server host %d\n", 
				i258pci_resp[l_cnt].hostid);
			continue;
		}
		s_cnt = i258pci_resp[l_cnt].nservers;

		/*
		 * For each of the server(s) present on this host, get the port id
		 * information via the get_server_info
		 */

		for (i = 0; i < s_cnt; i++) {
			if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)0))
				cmn_err (CE_PANIC, 
					"i258locatepci: cannot get message buffers\n");
			pcm.P_cmd_type = GET_SERVER_INFO;
			pcm.P_union1.P_server.cmd = 1;		/* return info of server */
			pcm.P_union1.P_server.arg1 = i;		/* server we're looking for */
			if((our_tid = mps_get_tid(i258brd_chan)) == (unchar)0) {
				cmn_err(CE_PANIC,"i258locatepci: cannot get transaction id\n");
			}
			mps_mk_unsol(mbp, socid, our_tid, (unsigned char *)&pcm, PCMCLEN);
			i258pcitimeout = 0;
			if (mps_AMPsend_rsvp(i258brd_chan,mbp,NULL,NULL) == -1)
				cmn_err(CE_PANIC, 
					"i258locatepci: sendrsvp failure. mbp=%x\n", mbp);
			(void) timeout( i258loctimeout, (caddr_t) &i258pcitimeout, 
						(WATCH_TIME*HZ) );
			sleep( (caddr_t) &i258pciresp, PZERO);
			if (i258pcitimeout)
				cmn_err(CE_CONT, 
			"i258: No response to GET_SERVER_INFO from PCI server host %d\n", 
					i258pci_resp[l_cnt].hostid);
		}
		i258pci_scount += i258pci_resp[l_cnt].nservers; 
	}
	/* 
	 * Now that we have all the information, setup values based on 
	 * configuration and actual hardware present
	 */

	dd = &i258dev[0];
	for (i = 0; i < N_i258 ;i++) {
		bin = i258pci_bin[i];
		sin = i258pci_sin[i];
		if ((i258pci_scount != 0) && (sin > i258pci_scount) ) {
			cmn_err(CE_NOTE, 
				"i258: PCI server instance %d configuration mismatch\n", sin);
			continue;
		}

		DEBPR(DEB_START,(CE_CONT, "i258locatepci: bin %d, sin %d, i %d\n",
				bin, sin, i));
		if (bin == 0) {
			if (sin == 0) 
				break;
			else {
				for (l_cnt=0,index=0; (l_cnt<ICS_MAX_SLOT && index < sin);
						l_cnt++) {
					if (i258pci_servers[l_cnt].portid != 0) 
						index++;
				}
				dd->slot = i258pci_servers[l_cnt - 1].hostid;
				dd->host_id = i258pci_servers[l_cnt - 1].hostid;
				dd->port_id = i258pci_servers[l_cnt - 1].portid;
				dd->d_flags |= I258_ALIVE;
				dd++;
			}
		}
		else {
			/* note that instances are counted from 1 */
			if (bin > (int) i258max) {
				cmn_err(CE_NOTE, 
					"i258: Board instance %d not found\n", bin);
				continue;
			}
			else {
				for (l_cnt=0,index=0; (l_cnt<ICS_MAX_SLOT && index < bin);
							l_cnt++) {
					if (i258pci_resp[l_cnt].portid != 0) 
						index++;
				}
				if (sin == 0) {
					dd->slot = i258pci_resp[l_cnt - 1].hostid;
					dd->host_id = i258pci_resp[l_cnt - 1].hostid;
					dd->port_id = i258pci_resp[l_cnt - 1].portid;
					dd->d_flags |= I258_ALIVE;
					dd++;
				}
				else {
					stoff = i258pci_resp[l_cnt - 1].stoff;
					if (i258pci_servers[stoff + sin - 1].portid != 0) {
						dd->slot = i258pci_servers[stoff + sin - 1].hostid;
						dd->host_id = i258pci_servers[stoff + sin - 1].hostid;
						dd->port_id = i258pci_servers[stoff + sin - 1].portid;
						dd->d_flags |= I258_ALIVE;
						dd++;
					}
				}
			}
		}
	}
	if (i == 0) { 		/* default case - nothing specified */
		for (i = 0, j = 0; (j < ICS_MAX_SLOT && i < N_i258); j++) {
			if (i258pci_servers[j].portid != 0) {
				dd->slot = i258pci_servers[j].hostid;
				dd->host_id = i258pci_servers[j].hostid;
				dd->port_id = i258pci_servers[j].portid;
				dd->d_flags |= I258_ALIVE;
				dd++;
				i++;
			}
		}
	}
	/* take care of partial specification XXX  i != 0 and i < N_i258 */
	for (i = 0; i < N_i258; i++) {
		dd = &i258dev[i];
		DEBPR(DEB_START,(CE_CONT, "i258locatepci: done, flags=0x%x\n",
				dd->d_flags));
		if (dd->port_id != 0) {
			j = (i258pci_sin[i] == 0) ? i + 1 : i258pci_sin[i];
			cmn_err(CE_CONT, 
				"i258: Using PCI server instance %d in slot %d, at port 0x%x\n",
				j, dd->slot, dd->port_id);
		}
	}
	return;
}

/*****************************************************************************
 *
 * TITLE:    i258init
 *
 * ABSTRACT:    Called at boot time to "probe" the boards.
 *
 ****************************************************************************/
i258init()
{
	register unsigned i258slot;
	register struct i258cfg *cf;
	register struct i258dev *dd;
	int      reg;
	ushort   i, tmp;
	char	valbuf [180];
	char	temp [16];
	int		state1, state2, c_code, ret_val = 0;

#ifdef TRACE
	i258tridx = 0;
	i258log = 0;
#endif

	i258max = 0;
	DEBPR(DEB_INIT, (CE_CONT, "i258: Starting board scan\n"));
	for (i258slot = 0;
		 (i258slot < (unsigned short) ICS_MAX_SLOT); i258slot++) {

		if (ics_agent_cmp(i258cfg->name_ptr, i258slot) != 0)
			continue;

		cmn_err(CE_CONT, "iSBC 386/258 board %d in slot %d\n",i258max,i258slot);

		tmp = ics_read(i258slot,ICS_BistSupportLevel); /* BIST Support level */
		if (tmp != 0) {
			tmp = ics_read(i258slot,ICS_BistSlaveStatus);
			if (tmp  != BIST_SLAVE_STAT) {
				cmn_err(CE_NOTE, "BIST Error: iSBC 386/258 slot %d, status %x",
					i258slot, tmp);
				continue;
			}

			tmp = ics_read(i258slot, ICS_BistMasterStatus);
			if ((tmp & BIST_MSTR_STAT) != BIST_MSTR_STAT) {
				cmn_err(CE_NOTE, "BIST Error: iSBC 386/258 slot %d, status %x",
					i258slot, tmp);
				continue;
			}
		}

		if((reg = ics_find_rec(i258slot,FW_COMM_TYPE)) == 0xffff) {
			cmn_err(CE_NOTE, "i258: Firmware Communication Record not found");
			continue;
		}
		i258max++;
	}

	/* fix it for improper config */
	i258_max_req = (i258_max_req > MAXREQ) ? MAXREQ : i258_max_req;

	/* initialize the i258dev structure for all boards configured in */

	for (tmp = 0; tmp < (unsigned short) N_i258; tmp++) {
		dd = &i258dev[tmp];
		dd->d_bufh = &i258tab[tmp];
		(dd->d_bufh)->b_actf = (struct buf *)NULL;
		(dd->d_bufh)->b_actl = (struct buf *)NULL;
		for(i = 0;i<(unsigned short)(NUMUNITS);i++) {
			dd->d_pdev[i] = i; /* since pdev is used only for close */
#ifndef NOSAR
			i258stat[tmp][i].io_cnt = 0;
			i258stat[tmp][i].io_bcnt = 0;
			i258stat[tmp][i].io_resp = 0;
			i258stat[tmp][i].io_act = 0;
#endif
		}
		for(i = 0;i< (unsigned short) i258_max_req;i++)
			dd->reqinfo[i].r_flags = 0;
		for (i = 0; i < I258_NUMTAPE; i++) {
			dd->tbuf[i].unit = 0xff;			/* invalidate */
			dd->tbuf[i].bufpages = 0;
		}
		for (i=0; i<I258_QUERY_CONTROLLER_SIZE; i++) {
			dd->w_units[i] = 0;
			dd->f_units[i] = 0;
			dd->t_units[i] = 0;
		}
		dd->req_use = 0;
		dd->av_max = i258_max_req; 
		dd->d_flags = 0;
		dd->host_id = 0;
		dd->port_id = 0;
	}

	i258pci_bcount = 0;
	i258pci_scount = 0;
	if (!bps_open()) {
		state1 = 0;
		while ( (!bps_get_wcval("i258*", &state1, sizeof(valbuf), valbuf)) &&
				(i258pci_bcount < N_i258) ) {
			state2 = 0;	
			c_code = 0;
			while (!bps_get_opt(valbuf, &state2, "i258pci_bin:i258pci_sin", 
					&c_code, sizeof(temp), temp)) {
				switch (c_code) {
					case  1: 
						if (!bps_get_integer(temp, &ret_val)) { 
							/* 
							 * over-write the default in space.c 
							 */
							i258pci_bin[i258pci_bcount++] = ret_val;

							DEBPR(DEB_INIT,(CE_CONT, 
								"i258init: Found i258pci_bin, value %d\n", 
								ret_val));
							break;
						}
					case  2: 
						if (!bps_get_integer(temp, &ret_val)) { 
							/* 
							 * over-write the default in space.c 
							 */
							i258pci_sin[i258pci_scount++] = ret_val;

							DEBPR(DEB_INIT,(CE_CONT, 
								"i258init: Found i258pci_sin, value %d\n", 
								ret_val));
							break;
						}
					default:
							break;
				}
			}
		}
	}
	DEBPR(DEB_INIT,(CE_CONT, "i258 Board scan complete\n"));
	/*
	 * Open a channel to talk to the boards on and open
	 * a channel to talk to any other incarnations of this driver.
	 * If either of these fail, shut down ALL boards.
	 */
	if(i258brd_chan == -1) {
		i258brd_chan = mps_open_chan(I258_BOARD_PORT, i258bmsg, MPS_BLKPRIO);
		if (i258brd_chan == -1) {
			cmn_err(CE_NOTE, "i258 driver: Cannot open MBII transport channel");
			for(tmp = 0;tmp< i258max;tmp++)
				i258dev[tmp].d_flags = 0;
		}
	}
	DEBPR(DEB_INIT,(CE_CONT, "Board Channel = %x\n",i258brd_chan));

#ifdef MULTICPU
	if(i258drv_chan == -1) {
		i258drv_chan = mps_open_chan(I258_DRIVER_PORT, i258dmsg, MPS_BLKPRIO);
		if (i258drv_chan == -1) {
			cmn_err(CE_NOTE, "i258 driver: Cannot open MBII transport channel");
			for(tmp = 0;tmp< i258max;tmp++)
				i258dev[tmp].d_flags = 0;
		}
	}
	DEBPR(DEB_INIT,(CE_CONT, "Driver Channel = %x\n",i258drv_chan));
#endif
	return(EOK);
}
/*****************************************************************************
 *
 * TITLE:    i258pause
 *
 * ABSTRACT:    Allows i258istart to sleep for 1 second so that it can
 *                poll interconnect space when a board is being initialized.
 *
 ****************************************************************************/

i258pause(dd)
register struct i258dev *dd;
{
	dd->d_flags &= ~I258_PAUSE;
	(void) wakeup((caddr_t)&dd->slot);
}

/*****************************************************************************
 *
 * TITLE:    i258timeout
 *
 * ABSTRACT:    Wakes up at intervals to make sure that no request
 *                has timed out.  One copy is run for each board in operation.
 *
 ****************************************************************************/
void
i258timeout(dd)
struct i258dev *dd;
{
	register struct i258req *rq;
	int      rn;

	for(rn = 0;rn< i258_max_req;rn++) {
		rq = &dd->reqinfo[rn];
		if ((rq->r_flags & i258RQ_WATCH_MASK) 
			== (i258RQ_WATCH_ME | i258RQ_BUSY)) {

			if(rq->r_timeout == 0 && i258print_warnings) {
				cmn_err(CE_WARN,
					"i258: slot %d, dev %x, no response on tid %x command %d\n",
					dd->slot,rq->r_dev,rq->r_tid,rq->r_op);
				rq->r_flags |= i258RQ_TIMED_OUT;
			}
			if(rq->r_timeout > WATCH_TIME)
				rq->r_timeout -= WATCH_TIME;
			else
				rq->r_timeout = 0;
		}
	}

	if(dd->d_flags & I258_TIMEOUT)
		(void) timeout(i258timeout, (caddr_t) dd,(WATCH_TIME*HZ));
}

/*****************************************************************************
 *
 * TITLE:    i258istart
 *
 ****************************************************************************/
i258istart()
{
	(void) i258locatepci();
}

/******************************************************************************
 *
 * TITLE:    i258getreq
 *
 * ABSTRACT:    Get a request slot if possible.
 *
 ****************************************************************************/
int
i258getreq(dd)
register struct i258dev *dd;
{
	register struct i258req *rq = (struct i258req *) 0;
	register i;

	if (dd->req_use < dd->av_max) {
		for(i = 0;i<i258_max_req;i++) {
			if(! (dd->reqinfo[i].r_flags & i258RQ_BUSY)){
				rq = &dd->reqinfo[i];
				rq->r_flags |= i258RQ_BUSY;
				rq->r_mbp = (mps_msgbuf_t *)0;
				rq->r_dbp = (struct dma_buf *)0;
				dd->req_use++;
				return(i);
			}
		}
	}
	return(-1);
}

/******************************************************************************
 *
 * TITLE:    i258freereq
 *
 * ABSTRACT:    Free a request and associated resources.
 *
 ****************************************************************************/
i258freereq(dd,rn)
register struct i258dev *dd;
int                      rn;
{
	register struct i258req *rq = &dd->reqinfo[rn];
	int unit;

	if(rq->r_mbp != (mps_msgbuf_t *)0)
		mps_free_msgbuf(rq->r_mbp);
	rq->r_mbp = (mps_msgbuf_t *)0;
	rq->r_dbp = (struct dma_buf *)0;
	rq->r_flags &= ~i258RQ_BUSY;
	dd->req_use--;
	if(dd->d_flags & I258_NEED_REQ) {
		dd->d_flags &= ~I258_NEED_REQ;
		(void) wakeup((caddr_t)&dd->req_use);
	}
	/*
	 * Is somebody trying to determine when to close the device??
	 */
	unit = UNIT(rq->r_dev);
	if (dd->d_sflags[unit] & SF_CLOSEWAIT) {
		if (!i258checkreq(dd, unit)) {
			DEBPR(DEB_CLOSE,(CE_CONT, "i258freereq: unit 0x%x is idle\n",unit));
			dd->d_sflags[unit] &= ~SF_CLOSEWAIT;
			(void) wakeup((caddr_t)&dd->d_pdev[unit]);
		} else {
			DEBPR(DEB_CLOSE,(CE_CONT, "i258freereq: unit 0x%x still busy\n",
								unit));
		}
	}
}

/******************************************************************************
 *
 * TITLE:    i258checkreq
 *
 * ABSTRACT:    See if there are any requests or buffers outstanding
 *                for a given unit on "dd"
 *
 ****************************************************************************/
int
i258checkreq(dd,unit)
register struct i258dev *dd;
register unit;
{
	struct buf     *bp;
	int            i;

	/*
	 * First, check the outstanding requests.
	 */
	for(i = 0;i<i258_max_req;i++) {
		if (!(dd->reqinfo[i].r_flags & i258RQ_BUSY))
			continue;
		if (!(UNIT(dd->reqinfo[i].r_dev) == unit))
			continue;
		return(1);
	}
	/*
	 * Now, check the buffer queue.
	 */
	bp = (dd->d_bufh)->b_actf;
	while (bp != (struct buf *)0) {
		if(UNIT(bp->b_dev) == unit) {
			return(1);
		}
		bp = bp->av_forw;
	}
	return(0);
}

/******************************************************************************
 *
 * TITLE:    i258io
 *
 * ABSTRACT:    Fire up a request.
 *
 ****************************************************************************/
i258io(brd,rn,op)
int      brd, rn;
unsigned short op;
{
	register    struct   i258dev	*dd;
	register    struct   i258req	*rq;
	struct      i258PCM				pcm;
	mps_msgbuf_t					*mbp;
	struct dma_buf					*obuf = (struct dma_buf *)0;
	struct dma_buf					*ibuf = (struct dma_buf *)0;
	int								i, m_type;    /* Index, Message type */
	unsigned						unit;
	daddr_t							endsec, sector;
	struct 		i258rdvfy			*vd;

	dd = &i258dev[brd];
	rq = &dd->reqinfo[rn];
	unit = UNIT(rq->r_dev);

	DEBPR(DEB_IO,(CE_CONT, "i258io: op=%x\n",op));

	pcm.P_cmd_type = 0;
	pcm.P_resv = 0;
	pcm.P_device_type = 0;
	pcm.P_unit_number = 0;
	pcm.P_union1.P_device_addr = 0;
	pcm.P_union2.P_byte_count = 0;
	for (i=0; i<8; i++)
		pcm.P_reserved[i] = 0;

#ifdef DEBUG
	if(rq->r_mbp == (mps_msgbuf_t *)0) {
		cmn_err(CE_CONT, "i258io: message buffer at 0, brd=%d, rn=%x\n",brd,rn);
		monitor();
	}
#endif
	mbp = rq->r_mbp;
	mbp->mb_bind = (brd<<8) | rn;
	if((rq->r_tid = mps_get_tid(i258brd_chan)) == (unchar)0) {
		cmn_err(CE_PANIC,"i258: cannot get transaction id\n");
	}

	if (ISWINI(rq->r_dev))
		pcm.P_device_type = WINI_TYPE; /* Unit is a wini */
	else if (ISFLOP(rq->r_dev))
		pcm.P_device_type = FLOPPY_TYPE; /* Unit is a floppy */
	else
		pcm.P_device_type = TAPE_TYPE; /* Unit is a tape */
	pcm.P_unit_number = unit;

	switch (op) {

	case QUERY_CONTLR_CMD:
		pcm.P_cmd_type = QUERY_CONTROLLER;
		pcm.P_device_type = i258querydevtype;
		pcm.P_unit_number = 0;
		ibuf = rq->r_dbp;
		m_type = 0;    /* Unsolicited */
		break;
	case RW_CMD:
		if((rq->r_bp)->b_flags & B_READ) {
			pcm.P_cmd_type = READ_DATA;
			ibuf = rq->r_dbp;
			m_type = 0;    /* Unsolicited */
		} else {
			pcm.P_cmd_type = WRITE_DATA;
			obuf = rq->r_dbp;
			m_type = 1;    /* Solicited */
		}
		rq->r_timeout = WAIT_1MINUTE;    /* Wait for at least 60 seconds */
		sector = (rq->r_bp)->b_sector;
		pcm.P_union2.P_byte_count = 0;
		if ((dd->d_sflags[unit] & SF_VTOC_OK) && (PARTITION(rq->r_dev) != 0)) {
			struct alt_info *altptr;
			ulong req_cnt = rq->r_xcount;
			ushort secsiz = i258drtab_struct(brd, unit)->suc_buf.w_f.dr_secsiz;
			/* According to the PCI spec, all requests are specified
			 * as the number of sectors to xfer.  Since i258strategy
			 * has already made sure that the request is in sectorsize
			 * chunks, we don't have to worry about it.
			 */
			if (req_cnt <= secsiz)
				endsec = sector;
			else
				endsec = sector + req_cnt / secsiz - 1;
			altptr = i258drtab_struct(brd, unit)->dr_altptr;
			for (i = 0; i < (int) altptr->alt_sec.alt_used; i++) {
				if (sector == altptr->alt_sec.alt_bad[i]) {
					sector = altptr->alt_sec.alt_base + i;
					pcm.P_union2.P_byte_count = 
						min((int)req_cnt, (int)secsiz); /* XX ulong to int */
					break;
				}
				if ((altptr->alt_sec.alt_bad[i] > sector) &&
				    (altptr->alt_sec.alt_bad[i] <= endsec)) {
					endsec = altptr->alt_sec.alt_bad[i] - 1;
					pcm.P_union2.P_byte_count = 
						(altptr->alt_sec.alt_bad[i] - sector) *secsiz;
				}
			}
		}

		if (pcm.P_union2.P_byte_count == 0)
			pcm.P_union2.P_byte_count = rq->r_xcount;
		pcm.P_union1.P_device_addr = sector;
		break;
	case ERASE_CMD:
		pcm.P_cmd_type = ERASE_UNIT;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_10MINUTES;
		break;
	case FORMAT_CMD:
		pcm.P_cmd_type = FORMAT_UNIT;
		if (ISWINI(rq->r_dev)) {
			/* map out the primary and grown defect list */
			pcm.P_union1.P_format.P_flags = CERTIFY_DISABLE|PRIM_AND_GROWN;
		} else {
			/* No deflist mapped out */
			pcm.P_union1.P_format.P_flags = CERTIFY_DISABLE|NO_DEFECT;
		}
		rq->r_timeout = WAIT_FOREVER;       /* Wait forever.... */
		m_type = 0;    /* Unsolicited */
		break;
	case LOAD_CMD:
		pcm.P_cmd_type = LOAD;
		m_type = 0;    		   /* Unsolicited */
		rq->r_timeout = WAIT_5MINUTES;
		break;
	case RD_VERIFY_CMD:
		vd = i258rdvfy_struct(brd,unit);
		pcm.P_cmd_type = READ_DATA_VERIFY;
		pcm.P_union1.P_device_addr = vd->vfy_io.vfy_in.abs_sec;
		pcm.P_union2.P_byte_count = 
			vd->vfy_io.vfy_in.num_sec * 
					i258drtab_struct(brd, unit)->suc_buf.w_f.dr_secsiz;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = vd->vfy_io.vfy_in.num_sec + WAIT_1MINUTE; 
								/* at least 1 minute plus num_sec seconds */
		break;
	case RECAL_CMD:
		pcm.P_cmd_type = RECALIBERATE;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_5MINUTES;
		break;
	case RELEASE_CMD:
		pcm.P_cmd_type = RELEASE;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_1MINUTE;
		break;
	case RESERVE_CMD:
		pcm.P_cmd_type = RESERVE;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_1MINUTE;
		break;
	case RESET_CMD:
		pcm.P_cmd_type = RESET_DEVICE;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_1MINUTE;
		break;
	case RETEN_TAPE_CMD:
		pcm.P_cmd_type = RETENSION_TAPE;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_10MINUTES;
		break;
	case SEEK_BOT_CMD:
		pcm.P_cmd_type = SEEK_BEG_OF_TAPE;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_5MINUTES;
		break;
	case SFFM_CMD:
		pcm.P_cmd_type = SEEK_FILEMARK;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_10MINUTES;
		pcm.P_union2.P_byte_count = 1; 	/* Seek 1 file mark */
		break;
	case SUC_CMD:
		pcm.P_cmd_type = SET_UNIT_CHAR;
		pcm.P_union1.P_unit_char.P_flags = 1; /* FORCE MODE */
		obuf = rq->r_dbp;
		m_type = 1;    /* Solicited */
		rq->r_timeout = WAIT_1MINUTE;
		break;
	case SIO_CMD:
		pcm.P_cmd_type = SET_UNIT_OPTIONS;
		pcm.P_union1.P_unit_char.P_flags = 1;        /* Force Mode */
		pcm.P_union1.P_unit_char.P_mode = i258sio_mode; /* Logical Addressing */
		pcm.P_union1.P_unit_char.P_retries = i258sio_nretries; /* # of retries */
		pcm.P_union2.P_unit_opt.P_cmd_ord = i258sio_cmd_ordering; /* Cmd Ordering */
		pcm.P_union2.P_unit_opt.P_cache_size = i258sio_cache_size; /* Cache Size */
		pcm.P_union2.P_unit_opt.P_read_ahead = i258sio_read_ahead ; /* Read Ahead */
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_1MINUTE;
		break;
	case UNLOAD_CMD:
		pcm.P_cmd_type = UNLOAD;
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_1MINUTE;
		break;
	case WRFM_CMD:
		pcm.P_cmd_type = WRITE_FILEMARK;
		pcm.P_union2.P_byte_count = 1; /* Write 1 file mark */
		m_type = 0;    /* Unsolicited */
		rq->r_timeout = WAIT_5MINUTES;
		break;
	default:
		cmn_err(CE_NOTE, "i258io: op(%x) not recognized\n",op);
		return;
	}

	switch (m_type) {
	case 0:    /* Unsolicited */
		mps_mk_unsol(mbp,mps_mk_mb2socid(dd->host_id,dd->port_id),rq->r_tid,
		    (unsigned char *)&pcm,PCMCLEN);
		break;
	case 1:    /* Solicited */
		mps_mk_sol(mbp,mps_mk_mb2socid(dd->host_id,dd->port_id),rq->r_tid,
		    (unsigned char *)&pcm,PCMCDLEN);
		break;
	default:
		DEBPR(DEB_IO,(CE_CONT, "i258io: m_type(%x) not recognized\n",m_type));
	}

	rq->r_flags |= i258RQ_WATCH_ME;
	rq->r_op = pcm.P_cmd_type;	/* READ_DATA or WRITE_DATA */
#ifndef NOSAR
	if (op == RW_CMD) {
		drv_getparm (LBOLT, (time_t) &rq->r_tstart);
		/* if no outstanding requests, set base time */
		if (i258sar_outstanding == 0)
			i258sar_base_time = rq->r_tstart;
#ifdef DEBUG
		if ((i258_debug & DEB_SAR) && i258sar_outstanding < 0) 
			(*cdebugger)(DR_OTHER,NO_FRAME);
#endif
		/* Sending off a job, so increment the outstanding job count */
		i258sar_outstanding++;
	}
#endif


#ifdef DEBUG
	if (i258_debug & DEB_IOM) 
		mps_msg_showmsg(mbp);
	if (obuf != ((struct dma_buf *)0))
		DEBPR(DEB_IOM,(CE_CONT, "obuf count = %x\n", obuf->count));
	else
		DEBPR(DEB_IOM,(CE_CONT, "obuf = %x", obuf));
	if (ibuf != ((struct dma_buf *)0))
		DEBPR(DEB_IOM,(CE_CONT, "ibuf count = %x\n", ibuf->count));
	else
		DEBPR(DEB_IOM,(CE_CONT, "ibuf = %x", ibuf));
	DEBPR(DEB_IOM,(CE_CONT, "\n"));
#endif

	if(mps_AMPsend_rsvp(i258brd_chan,mbp,obuf,ibuf)) {
		cmn_err(CE_CONT,"i258: send_rsvp???????\n");
		rq->r_flags &= ~i258RQ_IM_WAITING;
	} else {
		rq->r_mbp = (mps_msgbuf_t *)0;
	}
}

/******************************************************************************
 *
 * TITLE:    i258bmsg
 *
 * ABSTRACT:    Receive a message (presumably) from the 258 board.
 *
 ****************************************************************************/
i258bmsg(mbp)
register mps_msgbuf_t    *mbp;
{
	register struct i258dev   *dd = (struct i258dev *) 0;
	register struct i258req   *rq = (struct i258req *) 0;
	register struct buf       *bp;
	unsigned short            brd;
	unsigned short            rn;
	register struct i258PSM   *psm;
	register struct i258PCM   *pcm;
	unsigned long             xfrcnt;
	unsigned                  index, unit;
	unsigned char			  cmd = 0, loc = 0, tid = 0;
	unsigned short			  hid = 0, pid = 0;
	time_t					  curtime;
	time_t					  new_actual_time;
	register struct iotime	  *i258it;

#ifdef DEBUG
	int                       i;
#endif

#ifdef TRACE
	i258log++;
#endif

	DEBPR(DEB_BMSG,(CE_CONT, "i258bmsg entered\n"));
#ifdef DEBUG
	if(i258_debug & DEB_BMSG)
		mps_msg_showmsg (mbp);

#endif

	psm = (struct i258PSM *)mps_msg_getudp(mbp);
	pcm = (struct i258PCM *)mps_msg_getudp(mbp);
	cmd = psm->P_cmd_type; 
	if ((cmd == LOCATE_PCI) || (cmd == GET_SERVER_INFO)) {
		i258pciresp++;		
		if (cmd == LOCATE_PCI) 
			loc = 1;
		if (psm->P_comp_stat == 0) {
			hid = (unsigned short) mps_msg_getsrcmid(mbp);
			if (loc) {
				pid = mps_msg_getsrcpid(mbp);
				i258add_resp(&i258pci_resp[hid], hid, pid, 1);
				i258add_resp(&i258pci_servers[hid*MAX_SERVERS], hid, pid, 0);
				cmn_err(CE_CONT, 
				"i258: Response to locate PCI server from slot %d, port 0x%x\n",
					hid, pid);
			}
			else {
				if (pcm->P_union1.P_server.cmd == 0x1)
					i258add_resp(&i258pci_servers[hid*MAX_SERVERS], hid,
						pcm->P_union1.P_server.arg1, 0);
				if (pcm->P_union1.P_server.cmd == 0) {
					i258add_resp(&i258pci_resp[hid], hid,
						mps_msg_getsrcpid(mbp), pcm->P_union1.P_server.arg1);
				}
			}
		}
		else {
			if (loc) 
				cmn_err(CE_CONT, 
					"i258: status = 0x%x for Locate PCI server from host %d\n",
					psm->P_comp_stat, mps_msg_getsrcmid(mbp));
			else
				cmn_err(CE_CONT, 
				"i258: status = 0x%x for PCI GET_SERVER_INFO from host %d\n",
					psm->P_comp_stat, mps_msg_getsrcmid(mbp));
		}
		tid = mps_msg_gettrnsid(mbp); 
		if (tid)
			mps_free_tid(i258brd_chan,tid);
		(void) wakeup((caddr_t) &i258pciresp);
		mps_free_msgbuf(mbp);
		return;
	}

	/*
	 * If this message is a response to a send_rsvp, we can use
	 * the mb_bind field to find who it belongs to.
	 */
	if (mbp->mb_bind != MPS_MG_DFBIND) {
		brd = (mbp->mb_bind>>8) & 0xff;
		rn = mbp->mb_bind & 0xff;
		dd = &i258dev[brd];
		rq = &dd->reqinfo[rn];
		unit = UNIT(rq->r_dev);
		DEBPR(DEB_BMSG,(CE_CONT, "Got reply, brd=%d, rn=%x r_flags=%x\n",
					brd, rn, rq->r_flags));
#ifdef RUNINDBG
		if(rq->r_flags & i258RQ_TIMED_OUT) {
			cmn_err(CE_WARN,
				"i258: slot %d, dev %x, timed out tid (%x) completed\n",
				dd->slot,rq->r_dev, rq->r_tid);
		}
		if(rq->r_tid != mps_msg_gettrnsid(mbp)) {
			cmn_err(CE_WARN,
				"i258 expected tid: %x, received tid: %x, bind: %x\n",
				rq->r_tid,mps_msg_gettrnsid(mbp), mbp->mb_bind);
			monitor();
		}
#endif
#ifndef NOSAR
		/* Update the I/O statistics (SAR) */
		if (rq->r_op == READ_DATA || rq->r_op == WRITE_DATA) {
			drv_getparm (LBOLT, (time_t) &curtime);
			i258it = &i258stat[brd][unit];
			/* time to add is either the time since the start of the
			 * 1st non-overlapping job, or the time since the 
			 * completion of the last overlapped job
			 */
			new_actual_time = curtime -
				max (i258sar_base_time, i258sar_finish_time);
			i258it->io_act += new_actual_time;
			i258sar_finish_time = curtime;
			/* Add time job waited on the driver's queue.  Any waiting that
			 * happens at the 258 is counted as actual time.
			 * tqueued == 0 means tqueued is not valid for this transfer,
			 * i.e., we have already counted this transfer's queue time.
			 * Happens when bresid was not 0, and i258io is called below.
			 */
			if (rq->r_tqueued > 0) {
				i258it->io_resp += rq->r_tstart - rq->r_tqueued;
				rq->r_tqueued = 0;
			}
			i258it->io_resp += new_actual_time;
		}
#endif
		rq->r_flags &= ~(i258RQ_WATCH_ME|i258RQ_TIMED_OUT);
		rq->r_mbp = mbp;
		mps_free_tid(i258brd_chan,rq->r_tid);
		rq->r_tid = 0;
		bp = rq->r_bp;
		if(rq->r_flags & i258RQ_IM_WAITING) {
			DEBPR(DEB_BMSG,(CE_CONT, "i258bmsg: i258RQ_IM_WAITING\n"));
			rq->r_flags &= ~i258RQ_IM_WAITING;
			(void) wakeup((caddr_t)rq);
			return;
		}
		if(mps_msg_iserror(mbp) || mps_msg_isreq(mbp)) {
			bp->b_error = EIO;
			bp->b_flags |= B_ERROR;
		} else if ((mps_msg_getmsgtyp(mbp) == MPS_MG_UNSOL) && mps_msg_iscancel(mbp)){
			DEBPR(DEB_BMSG,(CE_CONT, "i258bmsg: iscancel\n"));
			bp->b_error = EIO;
			bp->b_flags |= B_ERROR;
		} else if (bp->b_error = i258checkerr(brd,psm)) {
			bp->b_flags |= B_ERROR;
		} else {
			if (!ISTAPE(rq->r_dev)) {
				xfrcnt = (ulong)psm->P_byte_count;
				bp->b_resid -= xfrcnt;
				if ((rq->r_xcount -= xfrcnt) > 0) {
					bp->b_sector += xfrcnt;
					/* On next interrupt do NOT add any time to queued time */
					rq->r_tqueued = 0;
					i258io(brd,rn,RW_CMD);
					return;
				}
			}
		}
		if(ISTAPE(rq->r_dev)) {
			xfrcnt = (ulong)psm->P_byte_count;
			bp->b_resid -= xfrcnt;
			if (rq->r_flags & i258RQ_BUF_TAPE) {
				/* Changed loc of B_RAMxx flgs */
				if (bp->b_flags & B_READ)
					dd->d_tflags[unit] |= B_RAMRD;
				else
					dd->d_tflags[unit] &= ~B_RAMWT;
				rq->r_flags &= ~i258RQ_BUF_TAPE;
			} else {
				if((bp->b_resid != 0) && (dd->d_sflags[unit] & SF_EOF_MARK)) {
					dd->d_sflags[unit] &= ~SF_EOF_MARK;	/* resid tells user */
				}
			}
		}
		rq->r_bp = (struct buf *)0;
		rq->r_dbp = (struct dma_buf *)0;
#ifndef NOSAR
		/* Job is done, so decrement the outstanding job count */
		if (rq->r_op == READ_DATA || rq->r_op == WRITE_DATA)
			i258sar_outstanding --;
#endif
		biodone(bp);
		i258setup(brd,rn);
		return;
	}
#ifdef DEBUG
	if(i258_debug & DEB_BMSGMU)
		mps_msg_showmsg(mbp);
#endif
	/*
	 *	Unsol received.
	 */
	for(brd = 0;(brd < (unsigned short)N_i258);brd++) {
		if(((unsigned short)mps_msg_getsrcmid(mbp) == i258dev[brd].host_id) &&
			(mps_msg_getsrcpid(mbp) == i258dev[brd].port_id)) {
			DEBPR(DEB_BMSG,(CE_CONT, "i258bmsg: command %d, status %x\n",
				psm->P_cmd_type, psm->P_comp_stat));
			break;
		}
	}
	if(brd >= (unsigned short)N_i258)        /* Unknown board */
		cmn_err(CE_NOTE, 
		"i258: Unknown unsolicited message from board with message id %d\n", 
			mps_msg_getsrcmid(mbp));
	tid = mps_msg_gettrnsid(mbp); 
	if (tid)
		mps_free_tid(i258brd_chan,tid);
	mps_free_msgbuf(mbp);
}

/******************************************************************************
 *
 * TITLE:    i258print
 *
 * ABSTRACT:    Print an error message when called from the kernel via bdevsw.
 *
 ****************************************************************************/
int
i258print (dev,str)
dev_t    dev;
char    *str;
{
	if (getminor (dev) > i258maxmin)
		return (ENXIO);

	/*
	 * If this unit is a tape, give up.
	 */
	if (ISTAPE(dev))
		return (ENXIO);

	cmn_err(CE_NOTE,
	    "%s on disk unit %d, partition %d (iSBC 386/258)\n",
	    str, UNIT(dev), PARTITION(dev));
	return(EOK);
}

int
i258dinit();

/*****************************************************************************
 ****************************************************************************/
int
i258print_unitmap(unit) 
unsigned char unit[]	;
{
	int	i	;

	for (i=0; i<I258_QUERY_CONTROLLER_SIZE; i++) {
		if (unit[i] & 0x1)
			DEBPR(DEB_INIT,(CE_CONT, "unit %d present\t", i));
	}
	DEBPR(DEB_INIT,(CE_CONT, "\n"))	;
}
/*****************************************************************************
 * TITLE:	i258unitmap
 *
 * ABSTRACT:	Issues 'Query controller' command for each od device type
 *				and initializes corresponding data structures for the
 *				board in question.
 ****************************************************************************/
int
i258unitmap(dev, unittype)
dev_t	dev;
int		unittype;
{
	unsigned int brd;
	struct   i258dev      *dd;
	register struct i258req *rq;
	int		 rn;
	unsigned int retval;
	int		 x;

	/*
	 * Get the board specific data structures.
	 */
	brd = BOARD(dev);
	dd = &i258dev[brd];

	/*
	 * Get a request slot for initializing.
	 */
	while((rn=i258getreq(dd)) == -1) {
		dd->d_flags |= I258_NEED_REQ;
		(void) sleep((caddr_t)&dd->req_use,PRIBIO);
	}

	x = SPL()	;
	rq = &dd->reqinfo[rn];
	rq->r_dev = dev;
	rq->r_bp = (struct buf *)0;
	if ((rq->r_mbp = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get a message buffer\n");

	if ((rq->r_dbp = mps_get_dmabuf(2,DMA_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get data buffer\n");
	(rq->r_dbp)->count = I258_QUERY_CONTROLLER_SIZE;
	(rq->r_dbp)->next_buf->count=0;
	switch (unittype) {
		case WINI_TYPE:
			(rq->r_dbp)->address  = kvtophys(dd->w_units);
			i258querydevtype = WINI_TYPE	;
			break	;	
		case FLOPPY_TYPE:
			(rq->r_dbp)->address  = kvtophys(dd->f_units);
			i258querydevtype = FLOPPY_TYPE	;
			break	;	
		case TAPE_TYPE:
			(rq->r_dbp)->address  = kvtophys(dd->t_units);
			i258querydevtype = TAPE_TYPE	;
			break	;	
	}

	rq->r_flags |= i258RQ_IM_WAITING;
	i258io(brd,rn,QUERY_CONTLR_CMD);
	splx(x)	;
	while(rq->r_flags & i258RQ_IM_WAITING)
		(void) sleep((caddr_t)rq,PRIBIO+1);

#ifdef DEBUG
	if (mps_msg_iserror(rq->r_mbp)) {
		DEBPR(DEB_OPEN,(CE_CONT, "i258unitmap --> rmsg_iserror"));
	}
	if (msg_isrsvp_solreperr(rq->r_mbp)) {
		DEBPR(DEB_OPEN,(CE_CONT, "i258unitmap --> msg_isrsvp_solreperr"));
		mps_msg_showmsg (rq->r_mbp)	;
	}
	if (mps_msg_isreq(rq->r_mbp)) {
		DEBPR(DEB_OPEN,(CE_CONT, "i258unitmap --> rmsg_isreq"));
	}
	if ((int) i258checkerr(brd, (struct i258PSM *)mps_msg_getudp(rq->r_mbp))) { 
		DEBPR(DEB_OPEN,(CE_CONT, "i258unitmap --> i258checkerr"));
	}
#endif
	retval = mps_msg_iserror(rq->r_mbp) || msg_isrsvp_solreperr(rq->r_mbp) ||
	    	 mps_msg_isreq(rq->r_mbp) || 
			 (int) i258checkerr(brd, (struct i258PSM *)mps_msg_getudp(rq->r_mbp));
	
	if (retval) {
		i258freereq (dd, rn)	;
		return (retval)	;
	}

#ifdef DEBUG
	switch (unittype) {
		case WINI_TYPE:
			i258print_unitmap(dd->w_units)	;
			break	;	
		case FLOPPY_TYPE:
			i258print_unitmap(dd->f_units)	;
			break	;	
		case TAPE_TYPE:
			i258print_unitmap(dd->t_units)	;
			break	;	
	}
#endif

	i258freereq (dd, rn)	;
	return (0);
}
/*****************************************************************************
 *
 * TITLE:    i258open
 *
 * ABSTRACT:    Open a unit.  Sets a given partition (i.e. special file) open.
 *
 *    If this function is opening the first partition on a physical
 *    device, it calls i258dinit to configure the device.
 *
 ****************************************************************************/
/* ARGSUSED */
i258open(devp, flag, otyp, cred_p)
dev_t		*devp;
int			flag;
int			otyp;            /* not used */
struct cred	*cred_p;
{
	struct   i258dev      *dd;
	struct   i258drtab    *dr;
	struct   ilabdrtab    *ivlab_dr;
	struct   i258cdrt     *cdr;

	int          		  error_code;
	unsigned int   		  brd;
	unsigned int          unit;
	unsigned int          part;
	unsigned int          x;
	struct   buf          *sbp = (struct buf *) NULL;

	struct   i258wini     *wd;
	struct   ivlab        *ivlab;
	struct   pdinfo       *pdinfo;
	struct   vtoc         *vtoc;
	struct   alt_info     *altinfo;
	dev_t				  dev;
	int	excl_open_error = 0;	/* allows us to set EBUSY instead
					 * of the normal ENXIO in badopen:
					 */

	dev = *devp;

	/*
	 * Make sure board, minor number and device exist.
	 */
	if (getminor(dev) > i258maxmin)
		return(ENXIO);

	if ((brd = BOARD(dev)) >= N_i258)
		return(ENXIO);

	if (ISTAPE(dev))
		return(ENXIO);

	/*
	 * Initialization
	 */
	unit = UNIT(dev);
	part = PARTITION(dev);
	dd = &i258dev[brd];
	cdr = &i258cfg[brd].c_drtab[unit&TID_MASK][DRTAB(dev)];
	error_code = 0;

	DEBPR(DEB_OPEN,(CE_CONT, 
		"i258open: board=%d, unit=%d, drtab=%x, part = %d, port = 0x%x\n",
			brd, unit, DRTAB(dev), part, dd->port_id));

	x = SPL();
	/*
	 * See if this is the first time through, if so, call i258istart to
	 * initialize the board if required.  Make sure that there is only
	 * one attempt to initialize the board.
	 */
	while (dd->d_flags & I258_INITTING) {
		(void) sleep( (caddr_t) &dd->d_flags, PZERO);
	}


	if ( !(dd->d_flags & I258_ALIVE) ) {
		dd->d_flags |= I258_INITTING;
		(void) i258istart();
		dd->d_flags &= ~I258_INITTING;
	}

	if ( !(dd->d_flags & I258_ALIVE) ) {
		splx(x);
		ERRRET(ENXIO);
	}

	/*
	 * Make sure no one else is opening or closing this unit.
	 */
	while (dd->d_sflags[unit] & SF_OPENCLOSE) {
		(void) sleep ((caddr_t)&dd->d_sflags[unit],PZERO);
	}
	dd->d_sflags[unit] |= SF_OPENCLOSE;

	if (!(ISWINI(dev)) || !(ISFLOP(dev))) {
		/*
		 * Figure out the actual units connected on each Target Id.
		 */
		if ( i258unitmap (dev,WINI_TYPE) || i258unitmap (dev,FLOPPY_TYPE) || 
				i258unitmap (dev,TAPE_TYPE) ) { 
			cmn_err(CE_PANIC, "i258: Cannot query the controller\n");
		}

		/*
		 * Assign unit numbers to the i258winidata and i258rdvfy structures. 
		 */
		i258assignstructs (brd, dd)	;
	}
	if ( (dr = i258drtab_struct(brd, unit)) == NULL) {
		/*
		 * This can happen if a device is configured in space.c
		 * but is not physically connected or the controller does not
	 	 * see it for some reason.
		 */
		goto badopen;
	}

    /*
     * If never yet opened, initialize device by doing an
     * init-sweep (must wait for idle I/O activity first).
     * If successful, mark it "open" and "ready".
     */
	if (dd->d_sflags[unit] & SF_OPEN) {
		if (!(dr->dr_part[part].p_flag & V_VALID)) {
			goto badopen;
		}
		/* if currently opening exclusive, or was already opened
		 * exclusive, current open fails.
		 */
		if ((flag & FEXCL) || (dd->d_sflags[unit] & SF_OEXCL)) {
			excl_open_error = 1;
			goto badopen;
		}
		dr->dr_part[part].p_flag |= V_OPEN;
		goto unitopen;
	}

	/*
	 * If we aren't watching for timeout already, do so.
	 */
	if(!(dd->d_flags & I258_TIMEOUT)) {
		dd->d_flags |= I258_TIMEOUT;
		(void) timeout( i258timeout, (caddr_t) dd, (WATCH_TIME*HZ) );
	}

	/*
	 * Before we go any farther, reserve the device if needed.
	 */
	if(!(dd->d_sflags[unit] & SF_RESERVED)) {
		if (i258resrel( dev, 1 )) {
			cmn_err( CE_WARN, "Cannot reserve device %x unit %d\n", dev, unit);
			dd->d_sflags[unit] &= ~SF_OPENCLOSE;
			splx(x);
			(void) wakeup((caddr_t)&dd->d_sflags[unit]);
			ERRRET(EBUSY);
		}
		dd->d_sflags[unit] |= SF_RESERVED;
	}

	if (i258dinit( brd, dev, cdr )) {
		cmn_err( CE_WARN, "Error initializing device %x\n", dev);
		goto badopen;
	}
	dd->d_sflags[unit] |= (SF_OPEN | SF_READY);
	if (flag & FEXCL)
		dd->d_sflags[unit] |= SF_OEXCL;

	/*
	 * If this is not a wini device then we're done.
	 */
	if (!ISWINI(dev))
		goto unitopen;

	/*
	 * TEMPORARY CODE: Until 'mkpart' is fixed to use the 
	 * 'FMTLOCK' and 'FMTUNLOCK' ioctl calls.
	 *
	 * Lock the device's wini data structures 
	 * 
	 */
	wd = i258winidata_struct(brd, unit)	;
	if (wd->lock != LOCKED) {
		if (i258lockwini( dev, wd, ~STRUCTIO) != EOK) {
			i258print( dev, "Unable to lock wini structures" );
			goto badopen;
		}
	}
	/*
	 * End temporary code.
	 */

	/*
	 * Get some temporary buffer space to temporarily store 
	 * some data structures (ivlab, vtoc, etc) while the 
	 * device is being opened.
	 */
	sbp = geteblk();
	sbp->b_flags |= (B_STALE | B_AGE);

	/*
	 * Read and validate the Intel Volume Label.
	 */
	wd->ivlabloc = VLAB_SECT * dr->suc_buf.w_f.dr_secsiz + VLAB_START;
	wd->ivlablen = sizeof(struct ivlab);

	ivlab = (struct ivlab *) sbp->b_un.b_addr;
	if (i258rdivlab(dev, ivlab) != EOK) {
		goto badopen;
	}
	/*
	 * Ivlab contains the 1st part of a DRTAB structure which
	 * describes the physical parameters of the device.
	 */
	ivlab_dr = (struct ilabdrtab *) &ivlab->v_dspecial[0];
	cdr->cdr_ncyl = ivlab_dr->dr_ncyl;
	cdr->cdr_nhead = ivlab_dr->dr_nfhead;
	cdr->cdr_nsec = ivlab_dr->dr_nsec;
	if (i258dinit( brd, dev, cdr )) {
		cmn_err( CE_WARN,
		    "ERROR SENDING NEW DISK PARAMETERS FOR DEVICE %x\n",dev);
		goto badopen;
	}

	/* 
	 * Read and validate PDINFO.
	 */

	wd->pdinfoloc = VTOC_SEC * dr->suc_buf.w_f.dr_secsiz;
	wd->pdinfolen = sizeof(struct pdinfo);

	pdinfo = (struct pdinfo *) sbp->b_un.b_addr;
	if (i258rdpdinfo( dev, pdinfo) != EOK) {
		goto badopen;
	}
	if ((pdinfo->cyls != dr->suc_buf.w_f.dr_ncyl) ||
	    (pdinfo->tracks != dr->suc_buf.w_f.dr_nhead) ||
	    (pdinfo->sectors != dr->suc_buf.w_f.dr_nsec)) {
		cmn_err( CE_WARN,
		    "PDINFO DOES NOT MATCH LABEL ON DEVICE %x\n",dev);
		goto badopen;
	}
	wd->vtocloc = pdinfo->vtoc_ptr;
	wd->vtoclen = pdinfo->vtoc_len;
	wd->altinfoloc = pdinfo->alt_ptr;
	wd->altinfolen = pdinfo->alt_len;
	wd->mdlloc = i258ST506loc( dev, pdinfo->mfgst );
	wd->mdllen = sizeof(struct st506mdl);

	/*
 * Read and validate VTOC.  If it's a valid VTOC then
	 * set the driver's internal partition table with the 
	 * new partition info.
	 */
	vtoc = (struct vtoc *) sbp->b_un.b_addr;
	if (i258rdvtoc(dev, vtoc) != EOK) {
		goto badopen;
	}
	if(i258setparts(dev, &vtoc->v_part[0], vtoc->v_nparts) != EOK) {
		goto badopen;
	}

	/*
	 * Read and validate Alternate Info.
	 */
	altinfo = &(wd->altinfo);
	if (i258rdaltinfo(dev, altinfo) != EOK) {
		goto badopen;
	}

	/*
	 * Device successfully opened.
	 */
	dd->d_sflags[unit] |= SF_VTOC_OK;
	if ( !(dr->dr_part[part].p_flag & V_VALID))
		goto badopen;

	/*
	 * If opening the root device, see if swap device is on the same
	 * unit.  If so, get the partition size for swapdev and set
	 * nswap accordingly.
	 */
	if ((dev == rootdev) && (swapdev != NODEV) &&
	    (brd == BOARD(swapdev)) && (unit == UNIT(swapdev)))
		nswap = dr->dr_part[PARTITION(swapdev)].p_nsec * dr->dr_lbps;

	dr->dr_part[part].p_flag |= V_OPEN;

	/*
	 * The following KLUDGE leaves partition 0 permanently open.
	 * This nullifies the erroneous calls to close by preventing
	 * close from shutting down the unit.
	 */
	dr->dr_part[0].p_flag |= V_OPEN;
	goto unitopen;

badopen:

	DEBPR(DEB_OPEN,(CE_NOTE, "at *badopen* for dev %x\n",dev));

	/*
	 * If there is a device out there and this is partition 0, then ignore 
	 * the error so that a totally hosed disk can be formatted.
	 */
	if ( (dr != NULL) && (PARTITION(dev) == 0))
		SETERR(0);
	else if (excl_open_error)
		SETERR(EBUSY);
	else
		SETERR(ENXIO);

	if (dd->d_sflags[unit] & SF_RESERVED) {
		if (i258resrel( dev, 0 ))
			cmn_err( CE_WARN, "Cannot release device %x\n", dev);
	}
	dd->d_sflags[unit] &= ~(SF_RESERVED | SF_HOLD_RSRV);

unitopen:
	/* 
	 * The device is now open.  Wake up any processes waiting to
	 * do an open.  Also, release the temporary buffer.
	 */
	dd->d_sflags[unit] &= ~SF_OPENCLOSE;
	(void) wakeup( (caddr_t)&dd->d_sflags[unit] );
	if (sbp != (struct buf *) NULL)
		brelse(sbp);
	splx(x);

	DEBPR(DEB_OPEN,(CE_CONT, "i258open: open complete\n"));
	return(error_code);
}

/******************************************************************************
 * TITLE:    i258halt
 *
 * ABSTRACT:    final cleanup actions for the driver
 *
 *	Called when the system shutsdown.  Releases the disk that
 *	contains the swap device.  close is never called for the swap
 *	device, so close thinks that the disk containing the swap device
 *	still has open partitions and does not release the disk.
 *
 *****************************************************************************/
i258halt()
{
	i258resrel(swapdev,0);
	return;
}

/******************************************************************************
 * TITLE:    i258close
 *
 * ABSTRACT:    Close a unit.
 *
 *    Called on last close of a partition; thus, "close" the
 *    partition.  If this was last partition, mark the unit
 *    closed and not-ready.  In this case, next open will
 *    re-initialize.
 *
 *****************************************************************************/
/* ARGSUSED */
i258close(dev, flag, otyp, cred_p)
register dev_t	dev;		/* major, minor numbers */
int				flag;		/* not used */
int				otyp;		/* not used */
struct cred		*cred_p;
{
	register struct i258dev    *dd = &i258dev[BOARD(dev)];
	unsigned    	unit = UNIT(dev);
	unsigned    	s;
	int             openflg = 0;
	int             i;
	struct	i258drtab	*dr	;

#ifdef NOCLOSEROOT
	if ((dev == rootdev) || (dev == swapdev) || (dev == pipedev)) {
		return(EOK);        /* never close these */
	}
#endif

	s = SPL();

	/*
	 * Make sure no one else is opening or closing this unit.
	 */
	DEBPR(DEB_CLOSE,(CE_CONT, "i258close: BEFORE SF_OPENCLOSE check\n"));
	while (dd->d_sflags[unit] & SF_OPENCLOSE)
		(void) sleep ((caddr_t)&dd->d_sflags[unit],PRIBIO);
	DEBPR(DEB_CLOSE,(CE_CONT, "i258close: AFTER SF_OPENCLOSE check\n"));

	dd->d_sflags[unit] |= SF_OPENCLOSE;

	/*
	 * Clear the bit that said the partition was open.
	 * If last close of drive insure drtab queue is
	 * empty before returning.
	 */
	if (ISWINI(dev)) { /* only reset partitions on hard disk */
		dr = i258drtab_struct(BOARD(dev), unit);
		dr->dr_part[PARTITION(dev)].p_flag &= ~V_OPEN;
		for (i=0; i<dr->dr_pnum; i++) {
			if (dr->dr_part[i].p_flag & V_OPEN) {
				openflg++;
				break;
			}
		}
	}
	if(!openflg) {
		while (i258checkreq(dd, unit)) {
			DEBPR(DEB_CLOSE,(CE_CONT, "Waiting for flush on unit %x\n",unit));
			dd->d_sflags[unit] |= SF_CLOSEWAIT;
			(void) sleep((caddr_t)&dd->d_pdev[unit],PRIBIO);
			DEBPR(DEB_CLOSE,(CE_CONT, "Woke up flush on unit %x\n",unit));
		}
		DEBPR(DEB_CLOSE,(CE_CONT, "About to release unit %x\n",unit));
		dd->d_sflags[unit] &= ~(SF_READY|SF_OPEN|SF_VTOC_OK);
		if(!(dd->d_sflags[unit] & SF_HOLD_RSRV)) {
			if (i258resrel(dev,0))
				cmn_err(CE_WARN,"Cannot release device %x\n",dev);
			dd->d_sflags[unit] &= ~SF_RESERVED;
		}
	}
	dd->d_sflags[unit] &= ~SF_OPENCLOSE;
	(void) wakeup((caddr_t)&dd->d_sflags[unit]);
	splx(s);
	return(EOK);
}

/******************************************************************************
 *
 * TITLE:    i258dinit
 *
 * ABSTRACT:    Initialize a device.
 *
 ****************************************************************************/
int
i258dinit(brd,dev,cdr)
int             brd;
dev_t           dev;
struct i258cdrt *cdr;
{
	register struct i258dev *dd;
	register struct i258req *rq;
	int                     i, rn, retval;
	struct   i258drtab      *dr;
	unsigned                unit;

	dd = &i258dev[brd];
	/*
	 * Get a request slot for initializing.
	 */
	while((rn=i258getreq(dd)) == -1) {
		dd->d_flags |= I258_NEED_REQ;
		(void) sleep((caddr_t)&dd->req_use,PRIBIO);
	}
	rq = &dd->reqinfo[rn];
	rq->r_dev = dev;
	rq->r_bp = (struct buf *)0;
	if ((rq->r_mbp = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get a message buffer\n");

	/* Figure unit & drtab entry. */
	unit = UNIT(dev);
	DEBPR(DEB_INIT,(CE_CONT, "i258dinit: unit=%x\n",unit));
	dr = i258drtab_struct(brd, unit);

	/*
	 * Set up drtab entry for initialization of the device.  By
	 * building a copy of the drtab in the i258dev, we make all these
	 * values accessible thru a register pointer to the i258dev.
	 */
	DEBPR(DEB_INIT,(CE_CONT, "i258dinit: ncyl(%x) nhead(%x) nsec(%x) \
	secsiz(%x)\n", cdr->cdr_ncyl,cdr->cdr_nhead,cdr->cdr_nsec,cdr->cdr_secsiz));

	if (ISWINI(rq->r_dev)) { /* it is a wini */
		dr->suc_buf.w_f.dr_ncyl = cdr->cdr_ncyl;
		dr->suc_buf.w_f.dr_nhead = cdr->cdr_nhead;
		dr->suc_buf.w_f.dr_nsec = cdr->cdr_nsec;
		dr->suc_buf.w_f.dr_secsiz = cdr->cdr_secsiz;
		dr->suc_buf.w_f.dr_floppy = 0;     /* Take the default */
		dr->suc_buf.w_f.dr_rwcc = cdr->cdr_ncyl + 1; /* Disable rwcc */
		dr->suc_buf.w_f.dr_wpc = cdr->cdr_ncyl + 1;  /* Disable wpc */
		dr->suc_buf.w_f.dr_step_rate = 0;       /* Take the default */
		dr->suc_buf.w_f.dr_alt_sect_z = i258no_alt_sec_zone; /* from Space.c */
		dr->suc_buf.w_f.dr_alt_trk_z = i258no_alt_trk_zone;  /* from Space.c */
		dr->suc_buf.w_f.dr_alt_trk_v = i258no_alt_cyl_vol*cdr->cdr_nhead;
		dr->suc_buf.w_f.dr_trk_z = i258no_trk_zone;  /*  from Space.c */
		dr->suc_buf.w_f.dr_trk_skew = 0;     /* Take the default */
		dr->suc_buf.w_f.dr_cyl_skew = 0;     /* Take the default */
		dr->suc_buf.w_f.dr_interleave = 1;   /* Interleave of 1 */
		for (i = 0; i < 218; i++)
			dr->suc_buf.w_f.dr_resv[i] = 0;

		/*
		 * Compute spc (sectors/cylinder), spb (sectors/block), secsiz.
		 * Also, copy partition-table pointer for i258strategy().
		 */
		dr->dr_spc = dr->suc_buf.w_f.dr_nhead * dr->suc_buf.w_f.dr_nsec;
		dr->dr_lbps = dr->suc_buf.w_f.dr_secsiz / NBPSCTR;
		dr->dr_part = cdr->cdr_part;
		dr->dr_pnum = cdr->cdr_pnum;
	} else if (ISFLOP(rq->r_dev)) { /* it is a floppy */
		dr->suc_buf.w_f.dr_ncyl = cdr->cdr_ncyl;
		dr->suc_buf.w_f.dr_nhead = cdr->cdr_nhead;
		dr->suc_buf.w_f.dr_nsec = cdr->cdr_nsec;
		dr->suc_buf.w_f.dr_secsiz = cdr->cdr_secsiz;
		dr->suc_buf.w_f.dr_floppy = cdr->cdr_flags;
		dr->suc_buf.w_f.dr_rwcc = 0;       /* Disable rwcc */
		dr->suc_buf.w_f.dr_wpc = 0;        /* Disable wpc */
		dr->suc_buf.w_f.dr_step_rate = 0;  /* Take the default */
		dr->suc_buf.w_f.dr_alt_sect_z = 0; /* Take the default */
		dr->suc_buf.w_f.dr_alt_trk_z = 0;  /* Take the default */
		dr->suc_buf.w_f.dr_alt_trk_v = 0;  /* Take the default */
		dr->suc_buf.w_f.dr_trk_z = 0;      /* Take the default */
		dr->suc_buf.w_f.dr_trk_skew = 0;   /* Take the default */
		dr->suc_buf.w_f.dr_cyl_skew = 0;   /* Take the default */
		dr->suc_buf.w_f.dr_interleave = 1; /* Interleave of 1 */
		for (i = 0; i < 218; i++)
			dr->suc_buf.w_f.dr_resv[i] = 0;
		DEBPR(DEB_INIT, (CE_CONT, 
			"i258dinit: heads=%x cyl=%x sec=%x secsize=%x floppy=%x\n",
			 dr->suc_buf.w_f.dr_nhead, dr->suc_buf.w_f.dr_ncyl,
			 dr->suc_buf.w_f.dr_nsec, dr->suc_buf.w_f.dr_secsiz,
			 dr->suc_buf.w_f.dr_floppy));
		/*
		 * Compute spc (sectors/cylinder), spb (sectors/block), secsiz.
		 * Also, copy partition-table pointer for i258strategy().
		 */
		dr->dr_spc = dr->suc_buf.w_f.dr_nhead * dr->suc_buf.w_f.dr_nsec;
		dr->dr_lbps = dr->suc_buf.w_f.dr_secsiz / NBPSCTR;
		dr->dr_part = cdr->cdr_part;
		dr->dr_pnum = cdr->cdr_pnum;
	} else { /* it is a tape */
		dr->suc_buf.tape.dr_bytes_block = cdr->cdr_secsiz;
		for (i = 0; i < 254; i++)
			dr->suc_buf.tape.dr_resv[i] = 0;
	}

	if (ISWINI(rq->r_dev)) {

		/* set up a wini to be first 2-head cylinder.
		   (open will take care of rest if VTOC ok). */
		dr->dr_part->p_tag = V_BACKUP;
		dr->dr_part->p_flag = V_RONLY | V_OPEN | V_VALID;
		dr->dr_part->p_fsec = 0L;
		dr->dr_part->p_nsec = (long)dr->dr_spc * dr->suc_buf.w_f.dr_ncyl;
		dr->dr_altptr = &(i258winidata_struct(brd, unit)->altinfo);
	}

	rq->r_flags |= i258RQ_IM_WAITING;
	if ((rq->r_dbp = mps_get_dmabuf(2,DMA_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get data buffer\n");
	(rq->r_dbp)->count = I258_SET_CHAR_SIZE;
	(rq->r_dbp)->address = kvtophys(dr);
#ifdef DEBUG
	if(i258_debug & DEB_IO){
		i258prtstring ((char *)dr, 256);
	}
#endif
	i258io(brd,rn,SUC_CMD);
	while(dd->reqinfo[rn].r_flags & i258RQ_IM_WAITING)
		(void) sleep((caddr_t)&dd->reqinfo[rn],PRIBIO+1);

	retval = mps_msg_iserror(rq->r_mbp) || mps_msg_iscancel(rq->r_mbp) ||
	    	 mps_msg_isreq(rq->r_mbp) || 
			 i258checkerr(brd, (struct i258PSM *)mps_msg_getudp(rq->r_mbp));

	i258setup(brd,rn);

	if (retval)
		return(retval);

	/*
	 * Get a request slot for initializing.
	 */
	while((rn = i258getreq(dd)) == -1) {
		dd->d_flags |= I258_NEED_REQ;
		(void) sleep((caddr_t)&dd->req_use,PRIBIO);
	}
	rq = &dd->reqinfo[rn];
	rq->r_dev = dev;
	rq->r_bp = (struct buf *)0;
	if ((rq->r_mbp = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get a message buffer\n");

	rq->r_flags |= i258RQ_IM_WAITING;
	i258io(brd,rn,SIO_CMD);
	while(dd->reqinfo[rn].r_flags & i258RQ_IM_WAITING)
		(void) sleep((caddr_t)&dd->reqinfo[rn],PRIBIO+1);

	retval = mps_msg_iserror(rq->r_mbp) || mps_msg_iscancel(rq->r_mbp) ||
	    	 mps_msg_isreq(rq->r_mbp) || 
			 (int) i258checkerr(brd, (struct i258PSM *)mps_msg_getudp(rq->r_mbp));

	i258setup(brd,rn);

	DEBPR(DEB_INIT,(CE_CONT, "i258dinit: end\n"));
	return(retval);
}

/*****************************************************************************
 *
 * TITLE:    i258setup
 *
 * ABSTRACT:    Setup an I/O request to run.
 *
 *    Handles idle-waiting, by allowing these priority over normal
 *    I/O's.  Not called if I/O is busy.
 *
 *    Called from i258strategy to start executing a request.  Called
 *    from i258bmsg to execute the next request on the queue, if any.
 *
 *    Called from i258[open|close|ioctl] in case a request was held up while
 *    waiting to finish one of the above, to restart the delayed request.
 *
 ****************************************************************************/
i258setup(brd,rn)
unsigned    brd,rn;
{
	register struct i258dev *dd;
	register struct i258req *rq;
	register struct iobuf   *bufh;
	struct   buf            *bp;
	caddr_t                 addrv;
	paddr_t                 addrp;
	struct dma_buf                *dbp;
	unsigned long           xfer_size, this_size;

	dd = &i258dev[brd];
	rq = &dd->reqinfo[rn];
	bufh = dd->d_bufh;

	DEBPR(DEB_SETUP,(CE_CONT, "i258setup: dd is: %x (bufh is: %x)\n",dd,bufh));

	/*
	 * Wrap up I/O on the requested slot and free the slot.
	 * If there is anything waiting to be done and no-one
	 * (open, close or ioctl) is waiting, reclaim the request slot
	 * and do the next I/O.
	 */
	i258freereq(dd,rn);
	if ((bp = bufh->b_actf) == NULL)
		return;
	if (dd->d_flags & I258_NEED_REQ)
		return;
	rq->r_flags |= i258RQ_BUSY;	/* Re-use freed slot */
	dd->req_use++;

	rq->r_retries = 0;            /* clear retry count */
	bufh->b_actf = bp->av_forw;
	rq->r_bp = bp;
	rq->r_dev = bp->b_dev;
	/* bp->b_resid is not always 0 */
	rq->r_xcount = xfer_size = bp->b_bcount - bp->b_resid;
	addrv = bp->b_un.b_addr;
	if ((rq->r_dbp = mps_get_dmabuf(1,DMA_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get data buffer\n");
	dbp = rq->r_dbp;
	/*
	 * First chunk is special since it may not be a full page.
	 */
	dbp->address = addrp = vtop(addrv, bp->b_proc);
	if ((addrp % ptob(1)) == 0)
		this_size = ptob(1);
	else
		this_size = ptob(1) - (addrp % ptob(1));
	dbp->count = this_size = min((int) this_size, 
									(int) xfer_size); /* XXX ulong to int */
	xfer_size -= this_size;
	addrv += this_size;
	/*
	 * Now, we are page aligned.  Scan through page by page
	 * until the request is satisfied.  For each page, see if
	 * it is physically next to the previous page.
	 * And... just to make things fun, data blocks are limited
	 * to, you guessed it, 64K.
	 */
	while (xfer_size) {
		addrp = vtop(addrv, bp->b_proc);
		this_size = min(ptob(1), xfer_size);
		if (((dbp->count + this_size) <= 0xffff)
		    && (addrp == (dbp->address+dbp->count))){
			/*
			 *	Physically contiguous and size is less than 64K, so
			 *	extend existing dbp.
			*/
			dbp->count += this_size;
		} else {
			/*
			 *	Start a new dbp.
			*/
			if ((dbp->next_buf = mps_get_dmabuf(1,DMA_NOSLEEP)) == NULL)
				cmn_err(CE_PANIC,"i258: cannot get data buffer\n");
			dbp = dbp->next_buf;
			dbp->address = addrp;
			dbp->count = this_size;
		}
		xfer_size -= this_size;
		addrv += this_size;
	}
	if ((dbp->next_buf = mps_get_dmabuf(1,DMA_NOSLEEP)) == NULL)  /* Null terminate list */
		cmn_err(CE_PANIC,"i258: cannot get data buffer\n");
	/*
	 *	At this point rq->r_dpb points to a properly constructed
	 *	list of data blocks of total length bp->b_bcount-bp->b_resid,
	 *	as per rq->r_xcount above.
	*/
	bp->b_resid = bp->b_bcount;  /* reset for return value */

#ifndef NOSAR
	/* remember the time the request was queued. */
	rq->r_tqueued = bp->b_start;
#endif

	/*
	 *	Get a message buffer.
	 */
	if (rq->r_mbp == (mps_msgbuf_t *)0)
		if ((rq->r_mbp = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
			cmn_err(CE_PANIC,"i258: cannot get a message buffer\n");

	/* fire up the controller */

	i258io(brd,rn,RW_CMD);
}

/*****************************************************************************
 *
 * TITLE:    i258strategy
 *
 * ABSTRACT:    
 * 		Try to start an I/O request.  Queue it if there are no
 *		available request slots.
 *
 *		Check legality, and adjust for partitions.  Reject request if
 *		unit is not-ready.
 *
 *    	NOTE: check for not-ready done here ==> could get requests
 *		queued prior to unit going not-ready.  258 gives
 *		not-ready status to those requests that are attempted
 *		before a new volume is inserted.  Once a new volume is
 *		inserted, would get good I/O's to wrong volume.
 *
 *		NOTE: The partition-check algorithm insists that requests must
 *		not cross a sector boundary.  If partition size is not a
 *		multiple of BSIZE, the last few sectors in the partition
 *		are not accessible.
 *
 ****************************************************************************/
void
i258strategy(bp)
register struct buf *bp;    /* buffer header */
{
	register struct    i258dev  *dd;
	register struct    i258req  *rq;
	struct   i258drtab          *dr;
	struct   i258part           *p;
	daddr_t                     secno;
	int                         rn;
	unsigned                    brd;
	unsigned                    x;
	unsigned                    unit;
	unsigned                    bytes_left;

	/* initializations */

	if (getminor(bp->b_dev) > i258maxmin) {
		bp->b_error = ENXIO;
		return;
	}

	if ((brd = BOARD(bp->b_dev)) >= N_i258) {
		bp->b_error = ENXIO;
		return;
	}

	if (ISTAPE(bp->b_dev)) {
		bp->b_error = ENXIO;
		return;
	}

	unit = UNIT(bp->b_dev);
	dd = &i258dev[brd];
	dr = i258drtab_struct(brd, unit);
	p = &dr->dr_part[PARTITION(bp->b_dev)];

	/* set b_resid to b_bcount because we haven't done bp yet */
	bp->b_resid = bp->b_bcount;
	if ((dd->d_sflags[unit] & SF_READY) == 0) {
		bp->b_flags |= B_ERROR;        /* not ready */
		bp->b_error = ENXIO;
		biodone(bp);
		return;
	}
	/* 
	 * Make sure that the request is of devgran granularity. 
	 * This is needed because the 258 accepts request counts
	 * in number of sectors. 
	 */
	if((bp->b_bcount % dr->suc_buf.w_f.dr_secsiz) != 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		biodone(bp);
		return;
	}
	/* 
     * See if partition is valid.  If not, error.  If so, see if we're
	 * trying to write to a read-only partition and not super-user.  If
	 * so, error. -- check for super user ? XXX
	 */
	if(!(p->p_flag & V_VALID) || (!(bp->b_flags & B_READ) &&
	    (p->p_flag & V_RONLY)))    {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		biodone(bp);
		return;
	}

	/*
	 * Figure "secno" from b_blkno. Adjust sector # for partition.
	 * Check if ready, and see if fits in partition.
	 *
	 * If reading just past the end of the device, it's
	 * End of File.  If not reading, or if read starts further in
	 * than the first sector after the partition, it's an error.
	 * secno is logical blockno / # of logical blocks per sector
	 * UNLESS IT'S A 128-byte sectored FLOPPY!!! 
	 */
	if (dr->dr_lbps) {
		secno = bp->b_blkno / (daddr_t)dr->dr_lbps;
	    if (secno * dr->dr_lbps != bp->b_blkno) {
	        bp->b_flags |= B_ERROR;
	        bp->b_error = ENXIO;
	        biodone(bp);
	        return;
	    }
	}
	else
		secno = bp->b_blkno * (NBPSCTR/dr->suc_buf.w_f.dr_secsiz);

	DEBPR(DEB_STRAT,(CE_CONT, "i258strat: secno is: %d\n",secno));

	if (secno >= p->p_nsec) {
		if (!((bp->b_flags & B_READ) && (secno == p->p_nsec))) {
			/* off the deep end */
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		biodone(bp);                            /* finish this */
		return;
	}

	/*
	 * At this point, it is no longer possible to directly return from .
	 * strategy. We now set b_resid to the number of bytes we cannot 
	 * transfer because they lie beyond the end of the request's 
	 * partition.  This value is 0 if the entire request is within the 
	 * partition.
	 */
	bytes_left = (p->p_nsec - secno) * dr->suc_buf.w_f.dr_secsiz;
	bp->b_resid = ((bp->b_bcount <= bytes_left)
	    ? 0 : (bp->b_bcount - bytes_left));

	secno += p->p_fsec;
	bp->b_sector = secno;

	x = SPL();

	i258disksort(dd->d_bufh, bp);        /* queue the request */

	DEBPR(DEB_STRAT,(CE_CONT, "i258strat: buffer queued secno %d\n",secno));
	if((rn=i258getreq(dd)) != -1) {
		rq = &dd->reqinfo[rn];
		rq->r_flags &= ~i258RQ_IM_WAITING;
		rq->r_mbp = (mps_msgbuf_t *)0;
		i258setup(brd,rn);
	} else {
		DEBPR(DEB_STRAT,(CE_CONT, "i258strat: no request available\n"));
	}

	splx(x);
	return;
}

/*****************************************************************************
 *
 * TITLE:    i258read
 *
 * ABSTRACT:    "Raw" read.  Uses dma_breakup (via i258breakup)
 *                to transform raw requests into requests that contain
 *                only contiguous memory.  Sometime this will be changed
 *                to use the scatter/gather feature of the DMA on the
 *                386 MB2 boards.
 *
 ****************************************************************************/
/*ARGSUSED*/
i258read(dev, uio_p, cred_p)
register	dev_t    dev;
struct		uio		*uio_p;
struct		cred	*cred_p;
{
	int error_code;
	int num_blks;

	num_blks = i258blksize(dev); 
	error_code = physiock(i258strategy, NULL, dev, B_READ,num_blks,uio_p);
	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258write
 *
 * ABSTRACT:    "Raw" write.  Uses dma_breakup (via i258breakup)
 *                to transform raw requests into requests that contain
 *                only contiguous memory.  Sometime this will be changed
 *                to use the scatter/gather feature of the DMA on the
 *                386 MB2 boards.
 *
 *
 ****************************************************************************/
/*ARGSUSED*/
i258write(dev, uio_p, cred_p)
register	dev_t	dev;
struct 		uio 	*uio_p;
struct 		cred	*cred_p;
{
	int error_code;
	int num_blks;

	num_blks = i258blksize(dev); 
	error_code = physiock(i258strategy,NULL,dev,B_WRITE,num_blks,uio_p);
	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258checkerr
 *
 * ABSTRACT:    Return the most appropriate error.
 *
 *    Called from i258bmsg to determine hard error or retry exhaustion.
 *    This is a separate function for debugging and diagnostic purposes.
 *    It is easy to trap to and see what's going on.
 *
 *     Assumes that the controller's transfer status function has been
 *    executed.  It decodes the bits and bytes in the error status
 *    buffer to decide what the most appropriate error to return is.
 *
 ****************************************************************************/
char *i258er_msg[] = {
	"Command Completed",
	"End Of Media",
	"End of Record Media",
	"File Mark Detected",
	"Command Timeout",
	"Device Reserved",
	"Device Not Initialized",
	"Device Already Initialized",
	"Invalid Parameter Command Message",
	"Invalid Parameter Buffer",
	"Device Not Ready",
	"Media Write Protected",
	"Media Error",
};

struct i258er_tbl {
	unsigned char er_type;       /* error status from 386/258 */
	unsigned char er_num;        /* Unix error number */
	unsigned char er_flags;      /* Action flags */
	unsigned char i258er_msg;    /* error message number */
	void           (*er_hand)();  /* Handling routine */
};

#define E_NO    0x0    /* No action required */
#define E_PR    0x1    /* Print the message */
#define E_DHN   0x2    /* Call the error handler for disks */
#define E_THN   0x4    /* Call the error handler for tapes */
#define E_BHN   0x6    /* Call the error handler for disks and tapes */
#define E_NOTS  0x8    /* Don't set tapestate to nothing */


void i258mchng();
void i258flmk();

struct i258er_tbl i258er_tbl[] = {
/*	Error Type,        Error Num,	Flags,                 Msg#, Handler */
	ER258_CMD_COMP,         0,		E_NO,                  0,    0,
	ER258_END_MEDIA,        ENOSPC, E_PR,                  1,    0,
	ER258_EOR_MEDIA,        EIO,    E_PR,                  2,    0,
	ER258_FILE_MARK,          0,    E_THN,                 3,    i258flmk,
	ER258_COMM_TIME,        EIO,    E_PR|E_NOTS,           4,    0,
	ER258_RESERVED,         EBUSY,  E_PR|E_NOTS,           5,    0,
	ER258_NOT_INITIAL,      EIO,    E_PR,                  6,    0,
	ER258_ALREADY_INIT,     EIO,    E_PR,                  7,    0,
	ER258_INVAL_COMM,       EIO,    E_PR|E_NOTS,           8,    0,
	ER258_INVAL_PARB,       EIO,    E_PR|E_NOTS,           9,    0,
	ER258_NOT_READY,        ENXIO,  E_DHN|E_THN|E_PR,     10,    i258mchng,
	ER258_WRITE_PRO,        ENODEV,	E_PR,                 11,    0,
	ER258_MEDIA_ERR,        EIO,    E_PR,                 12,    0
};

#define    	ER_MAX	(sizeof(i258er_tbl)/sizeof(struct i258er_tbl))

char *i258un_msg = "Unknown Status";

char *i258d_types[] = {
	"board",
	"wini",
	"floppy",
	"tape"
};

unchar
i258checkerr(brd,psm)
int                      brd;
register struct i258PSM *psm;
{
	struct	i258dev *dd;
	unchar	err_stat;
	char	*errmsg;
	unchar	err_flags;
	int	cstat, unit;


	unit = psm->P_unit_number;
	DEBPR((DEB_CHECKM),(CE_CONT, "i258checkerr entered, brd=%x, dev=%x\n",
	    brd,psm->P_unit_number));
	/*
	 * we need to check if it is a notify command 
	 * Note that the PCI spec says that a REQ_UNIT_NOTIFY has to be done
	 * to receive this. This is not implemented yet.
	 */
	if (psm->P_cmd_type == UNIT_ATTN_NOTIFY) {
		cmn_err(CE_NOTE, "received UNIT_ATTN_NOTIFY\n");
		psm->P_comp_stat =  ER258_NOT_READY;
	}

	if (psm->P_comp_stat == 0)
		return(0);

	dd = &i258dev[brd];
	cstat = psm->P_comp_stat;

	errmsg = i258un_msg;
	err_flags = E_PR;
	err_stat = EIO;

	if ((cstat > 0) && (cstat <= ER_MAX)) {
		errmsg = i258er_msg[i258er_tbl[cstat].i258er_msg];
		err_flags = i258er_tbl[cstat].er_flags;
		err_stat = i258er_tbl[cstat].er_num;
	}

	if(err_flags & E_PR) {
		cmn_err(CE_CONT, "iSBC258 controller %d %s %d (%s)\n",brd,
		    i258d_types[psm->P_device_type],
		    psm->P_unit_number,errmsg);
		cmn_err(CE_CONT, "%s Error\n",(err_stat ? "Hard":"Soft"));
	}
	/*
	 * call the appropriate error handler
	 */
	if(psm->P_device_type == TAPE_TYPE) {
		if(err_flags & E_THN)
			(*i258er_tbl[cstat].er_hand)(brd,psm,psm->P_unit_number);
		if(!(err_flags & E_NOTS))
			dd->d_tstate[unit] = TS_NOTHING;
	} else {
		if(err_flags & E_DHN)
			(*i258er_tbl[cstat].er_hand)(brd,psm,psm->P_unit_number);
	}
	return(err_stat);
}

/*****************************************************************************
 *
 * TITLE:    i258mchng
 *
 * ABSTRACT:    If called by i258checkerr, clears the SF_READY flag
 *                for the given unit.
 *
 ****************************************************************************/
/*ARGSUSED*/
void
i258mchng(brd,psm,dunit)
int                      brd;
register struct i258PSM *psm;
unsigned char           dunit;
{
	register struct i258dev *dd = &i258dev[brd];

	if(dunit < (unsigned short)(NUMUNITS))
		dd->d_sflags[dunit] &= ~SF_READY;

}

/*****************************************************************************
 *
 * TITLE:    i258flmk
 *
 * ABSTRACT:    If called by i258checkerr, sets the filemark flag
 *                in the tape flags.
 *
 ****************************************************************************/
/*ARGSUSED*/
void
i258flmk(brd,psm,dunit)
int                      brd;
register struct i258PSM *psm;
unsigned char           dunit;
{
	register struct i258dev *dd = &i258dev[brd];

	if(dunit < (unsigned short)(NUMUNITS))
		dd->d_sflags[dunit] |= SF_EOF_MARK;

	DEBPR(DEB_FLMK,(CE_CONT, "i258: file mark detected, brd %d, dunit %d\n",
				brd,dunit));
}


/******************************************************************************
 *
 * TITLE:    i258resrel
 *
 * ABSTRACT:    Reserve or Release a unit on a given device.
 *
 ****************************************************************************/
int
i258resrel(dev,flag)
dev_t dev;
int   flag;
{
	register struct i258dev *dd;
	register struct i258req *rq;
	int                     brd;
	int                     rn, retval;
	unsigned                op;

	brd = BOARD(dev);
	dd = &i258dev[brd];

	/*
	 * Get a request slot for talking to the board.
	 */
	while((rn = i258getreq(dd)) == -1) {
		dd->d_flags |= I258_NEED_REQ;
		(void) sleep((caddr_t)&dd->req_use,PRIBIO);
	}
	rq = &dd->reqinfo[rn];
	rq->r_dev = dev;
	rq->r_bp = (struct buf *)0;
	if ((rq->r_mbp = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get a message buffer\n");

	/* Figure unit & drtab entry. */

	rq->r_flags |= i258RQ_IM_WAITING;
	if(flag)
		op = RESERVE_CMD;
	else
		op = RELEASE_CMD;

	i258io(brd,rn,op);
	while(dd->reqinfo[rn].r_flags & i258RQ_IM_WAITING)
		(void) sleep((caddr_t)&dd->reqinfo[rn],PRIBIO+1);

	retval = mps_msg_iserror(rq->r_mbp) || mps_msg_iscancel(rq->r_mbp) ||
	    	 mps_msg_isreq(rq->r_mbp) || 
			 (int) i258checkerr(brd, (struct i258PSM *)mps_msg_getudp(rq->r_mbp));

	i258setup(brd,rn);
	return(retval);
}

/* 
 * handy routine to check if caller is super-user, device is wini and locked.  
 * The user and lock is checked under control of parameters usr and ckl.
 */

int
i258swl(susr, dev, ckl, lock)
struct cred	*susr;
dev_t	dev;
int		ckl;
int		lock;
{
	int	e_code;

	e_code = 0;
	if (susr != NULL) {				/* need to check the credentials ? */ 
		if ((e_code = drv_priv(susr)) != 0)
			return(e_code);
	}

	if (!ISWINI(dev)) {
		e_code = ENODEV;
		return(e_code);
	}

	if (ckl) {
		if (lock != LOCKED) {
			e_code = ENOLCK;
			return(e_code);
		}
	}
	return(e_code);
}


/*****************************************************************************
 *
 * TITLE:    i258ioctl
 *
 * ABSTRACT:    258 driver special functions.
 *
 *
 ****************************************************************************/
i258ioctl (dev, cmd, cmdarg, flag, cred_p, rval_p)
dev_t	dev;        /* major, minor numbers */
int		cmd;        /* command code */
caddr_t	cmdarg;     /* user structure with parameters */
int		flag;		/* not used */
struct	cred *cred_p;
int		*rval_p;
{
	register struct i258dev    *dd;
	struct i258drtab  *dr;
	struct ilabdrtab  *ivlab_dr;
	struct i258part   *pt;
	struct i258cdrt   *cdr;
	struct i258wini   *wd;
	struct i258rdvfy  *vd;

	int         		 error_code;
	unsigned int         brd;
	unsigned int         unit;
	unsigned int         part;
	unsigned int         x;
	unsigned short       i;

	struct   disk_parms  i258dp;
	union    io_arg      varg;
	struct   absio       absio;
	struct   fmtpart     fmtpart;

	struct   buf         *tmpbuf;
	struct   ivlab       *ivlab;
	struct   pdinfo      *pdinfo;
	struct   vtoc        *vtoc;
	struct   alt_info    *altinfo;
	struct   st506mdl    *mdl;

	union    i258specs   specs;

#ifdef DEBUG
	ushort msgsave;
#endif /* DEBUG */

	if (getminor(dev) > i258maxmin)
		return(ENXIO);

	if ((brd = BOARD(dev)) >= N_i258)
		return(ENXIO);

	if (ISTAPE(dev))
		return(ENXIO);

	unit = UNIT(dev);
	part = PARTITION(dev);
	dd = &i258dev[brd];
	dr = i258drtab_struct(brd, unit);
	pt = &dr->dr_part[part];
	cdr = &i258cfg[brd].c_drtab[unit&TID_MASK][DRTAB(dev)];
	wd = i258winidata_struct(brd, unit)	;
	error_code = 0;

	DEBPR(DEB_IOCTL,(CE_CONT, "i258ioctl: brd=%d, unit=%d, cmd=0x%x, part=%d\n",
	    brd,unit,cmd,PARTITION(dev)));

	if (!(dd->d_sflags[unit] & SF_READY)) {
		ERRRET(ENXIO);
	}

	switch (cmd) {
	case V_CONFIG:
		/*
		 * Reconfigure the board according to the specified parameters.
		 */
		DEBPR(DEB_IOCTL,(CE_CONT,"i258ioctl: V_CONFIG IOCTL on dev %x\n",dev));

		/* 
		 * Make sure the user is the 'super user' and device is a wini. 
		 */
		if (SUSER_WINI_NOLOCK) 
			break;
		/*
		 * Get the new disk parameters and reconfigure the controller.
		 */
		copyin( cmdarg, (caddr_t) &varg, sizeof(union io_arg));

		x = SPL();
		/*
		 * Make sure no partitions are open, except for partition 0.
		 */
		for (i = 1; i < (unsigned short) dr->dr_pnum; i++) {
			if (dr->dr_part[i].p_flag & V_OPEN) {
				splx(x);
				if (part == 0) {
					ERRRET(EBUSY);
				}
				else {
					ERRRET(ENODEV);
				}
			}
		}
		cdr->cdr_ncyl = varg.ia_cd.ncyl;
		cdr->cdr_nhead = varg.ia_cd.nhead;
		cdr->cdr_nsec = varg.ia_cd.nsec;
		if (i258dinit( brd, dev, cdr )) {
			cmn_err( CE_WARN, 
			    "i258ioctl: ERROR SENDING NEW DISK PARAMETERS FOR DEVICE %x\n",
			    dev );
		}
		splx(x);
		break;

	case V_REMOUNT:
		/* 
	 	 * Force the next call to open to re-read all of the
		 * devices data structures.
		 */

		DEBPR(DEB_IOCTL,(CE_CONT,"i258ioctl: V_REMOUNT IOCTL on dev %x\n",dev));

		/* 
		 * Make sure the user is the 'super user' and device is a wini. 
		 */
		if (SUSER_WINI_NOLOCK) {
			break;
		}

		x = SPL();
		/*
		 * Make sure no other partitions are open except for 0.
		 */
		for (i = 1; i < (unsigned short) dr->dr_pnum; i++) {
			if (dr->dr_part[i].p_flag & V_OPEN) {
				DEBPR(DEB_IOCTL,(CE_CONT, "i258ioctl: i=%d\n",i));
				splx(x);
				if (part == 0) {
					ERRRET(EBUSY);
				}
				else {
					ERRRET(ENODEV);
				}
			}
		}

		/*
		 * Wait for the controller to be come idle, set clear
		 * the state flags to force a read during the next open,
		 * then restart normal I/O.
		 */
		while (i258checkreq(dd, unit)) {
			dd->d_sflags[unit] |= SF_CLOSEWAIT;
			(void) sleep( (caddr_t) &dd->d_pdev[unit], PRIBIO );
		}
		splx(x);
		break;

	case V_ADDBAD:
		copyin(cmdarg, (caddr_t) &varg, sizeof(union io_arg));
		ERRRET(ENXIO);    /* Not supported */

	case V_FORMAT:
		/*
		 * Format one track of the device.
		 * If formatting a wini, make sure caller is 'root'.
		 */
		if (ISWINI(dev))
			if (SUSER_WINI_NOLOCK)
				break;

#ifdef O_EXCL_FIXED
		/* device must have been open for exclusive access */
		if (dd->d_sflags[unit] & SF_OEXCL) {
			SETERR(ENXIO);
			break;
		}
#endif

		/* 
	 	 * Verify the format parameters and setup the 
		 * format argument structure.  
		 *
		 * Note - Track # is relative to the start of the partition.
		 */
		copyin(cmdarg, (caddr_t)&varg, (unsigned)sizeof(union io_arg));

		dd->d_ftk.f_track = varg.ia_fmt.start_trk;
		dd->d_ftk.f_intl = varg.ia_fmt.intlv;
		dd->d_ftk.f_type = FORMAT_DATA;
		if (dd->d_ftk.f_track == 0)
			error_code = i258fmtpart( dev, dd->d_ftk.f_intl, 
								(unsigned short) dd->d_ftk.f_type);

		DEBPR(DEB_IOCTL,(CE_CONT, "i258ioctl: V_FORMAT track=%x\n",
				dd->d_ftk.f_track));
		break;

	case V_GETPARMS:
		/* 
		 * Send the device parameters to the user.
		 */

		DEBPR(DEB_IOCTL,(CE_CONT,"i258ioctl:V_GETPARMS IOCTL on dev %x\n",dev));

		if(ISWINI(dev)) {
			i258dp.dp_type = DPT_WINI;
		} else if (ISFLOP(dev)) {
			i258dp.dp_type = DPT_FLOPPY;
		} else {
			i258dp.dp_type = DPT_NOTDISK;
		}
		DEBPR(DEB_IOCTL,(CE_CONT, "i258ioctl: heads=%d, cyl=%d, sec=%d\n",
						 dr->suc_buf.w_f.dr_nhead,dr->suc_buf.w_f.dr_ncyl,
						 dr->suc_buf.w_f.dr_nsec));
		i258dp.dp_heads = dr->suc_buf.w_f.dr_nhead;
		i258dp.dp_cyls = dr->suc_buf.w_f.dr_ncyl;
		i258dp.dp_sectors = dr->suc_buf.w_f.dr_nsec;
		i258dp.dp_secsiz = dr->suc_buf.w_f.dr_secsiz;
		i258dp.dp_ptag = pt->p_tag;
		i258dp.dp_pflag = pt->p_flag;
		i258dp.dp_pstartsec = pt->p_fsec;
		i258dp.dp_pnumsec = pt->p_nsec;
		/*
		 * Copy parameters to user space.
		 */
		copyout( (caddr_t) &i258dp, cmdarg, sizeof(struct disk_parms));
		break;

	case V_XFORMAT:
/*	case FMTBAD:		Format tracks as bad */
		error_code = ENXIO;
		break;

	case V_VERIFY:		/* Verify sectors command */
		{
		register int	i, secno, oldpri;
		unsigned int	totsec;
		union vfy_io	kvfyio;
		
		/* check for - super user, wini, and no lock **/

		if (SUSER_WINI_NOLOCK)
			break;

		vd = i258rdvfy_struct(brd,unit);
		x = SPL();
		if (vd->vfyreq != 0) {  		/* another verify in progress */
			splx(x);
			return(EBUSY);
		}
		splx(x);

		if (copyin(cmdarg, (caddr_t)(&vd->vfy_io), sizeof(kvfyio)) != 0) 
			return(EFAULT);
	
		totsec = dr->suc_buf.w_f.dr_ncyl * dr->dr_spc;
		if ((vd->vfy_io.vfy_in.abs_sec >= totsec) ||
		    (vd->vfy_io.vfy_in.abs_sec + vd->vfy_io.vfy_in.num_sec > totsec) ||
			(vd->vfy_io.vfy_in.num_sec == 0)) 
				return(EINVAL);

		x = SPL();
		drv_getparm (LBOLT, (time_t) &vd->vfytime);
		vd->vfyreq = 1;
		splx(x);
		error_code = i258sendcmd(dev, RD_VERIFY_CMD); 
		x = SPL();
		vd->vfy_io.vfy_out.err_code = error_code;
		vd->vfyreq = 0;
		drv_getparm (LBOLT, (time_t) &vd->vfy_io.vfy_out.deltatime);
		vd->vfy_io.vfy_out.deltatime -= vd->vfytime;
		splx(x);
	
		if ((error_code != 0 ) && 
			(copyout((caddr_t)(&vd->vfy_io), cmdarg, sizeof(kvfyio)) != 0)) 
			return(EFAULT);

		break;
		}

	case V_PDLOC: {	/* Tell user where pdinfo is on disk */
		unsigned long	vtocloc = VTOC_SEC;

		if (copyout((caddr_t)&vtocloc, cmdarg, sizeof(vtocloc)) != 0) 
			return(EFAULT);
		
		break;
		}

	case I258_RESERVE:
		dd->d_sflags[unit] |= SF_HOLD_RSRV;
		break;

	case I258_RELEASE:
		dd->d_sflags[unit] &= ~SF_HOLD_RSRV;
		break;

	case I258_GET_SPECS:
		copyin(cmdarg, (caddr_t)&specs, (unsigned)sizeof(struct sp_get));
		if((specs.sp_get.sp_type != SP_258) || (specs.sp_get.sp_count == 0)) {
			ERRRET(EINVAL);
		}
		else {
			specs.sp_get.sp_rq_slot = i258_max_req;
			if(specs.sp_get.sp_count > 1)
				specs.sp_get.sp_reqs = dd->av_max;
			copyout((caddr_t)&specs, cmdarg, (unsigned)sizeof(struct sp_get));
		}
		break;

	case I258_SET_SPECS:
		copyin(cmdarg, (caddr_t)&specs, (unsigned)sizeof(struct sp_set));
		if((specs.sp_set.sp_type != SP_258) || (specs.sp_set.sp_count == 0)) {
			ERRRET(EINVAL);
		}
		else {
			if(specs.sp_set.sp_req_val) {
				if((specs.sp_set.sp_reqs == 0) ||
					(specs.sp_set.sp_reqs > (unsigned short) i258_max_req)) {
					ERRRET(EINVAL);
				}
				dd->av_max = specs.sp_set.sp_reqs;
			}
		}
		break;

	case V_FMTPART:
		/*
		 * Format the current partition.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;

		copyin(cmdarg, (caddr_t) &fmtpart, sizeof(struct fmtpart) );
		error_code = i258fmtpart( dev, fmtpart.intlv, fmtpart.method );
		break;

	case V_L_VLAB:
		/*
		 * Load IVLAB from the user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		/*
		 * Get and validate the new IVLAB before overlaying the 
		 * existing one.
		 */
		tmpbuf = geteblk();
		tmpbuf->b_flags |= (B_STALE | B_AGE);
		ivlab = (struct ivlab *) tmpbuf->b_un.b_addr;

		copyin( cmdarg, (caddr_t) ivlab, sizeof(struct ivlab) );
		x = SPL();
		/*
		 * Reconfigure the board and update the driver's internal 
		 * variables with the new IVLAB information.  But first,
		 * make sure no partitions are open, except for partition 0.
		 */
		for (i = 1; i < (unsigned short) dr->dr_pnum; i++) {
			if (dr->dr_part[i].p_flag & V_OPEN) {
				splx(x);
				brelse(tmpbuf);
				if (part == 0) {
					ERRRET(EBUSY);
 				}
 				else {
 					ERRRET(ENODEV);
 				}
			}
		}
		ivlab_dr = (struct ilabdrtab *) &ivlab->v_dspecial[0];
		cdr->cdr_ncyl = ivlab_dr->dr_ncyl;
		cdr->cdr_nhead = ivlab_dr->dr_nfhead;
		cdr->cdr_nsec = ivlab_dr->dr_nsec;
		if (i258dinit( brd, dev, cdr )) {
			splx(x);
			brelse(tmpbuf);
			SETERR(EIO);
			cmn_err( CE_WARN, 
			    "i258ioctl: ERROR SENDING NEW DISK PARAMETERS FOR DEVICE %x\n",
			    dev );
			break;
		}

		*(wd->ivlab) = *ivlab;
		wd->ivlabloc = VLAB_SECT * dr->suc_buf.w_f.dr_secsiz + VLAB_START;
		wd->ivlablen = sizeof(struct ivlab);
		splx(x);
		brelse(tmpbuf);
		break;

	case V_U_VLAB:
		/*
		 * Upload IVLAB to the user.
		 *
		 * But first make sure: the device is a wini, the 
		 * device's wini data structures are properly locked
		 * and the current ivlab is valid.
		 */
		if (USER_WINI_LOCK)
			break;

		copyout( (caddr_t) wd->ivlab, cmdarg, sizeof(struct ivlab));
		break;

	case V_R_VLAB:
		/*
		 * Read IVLAB from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258rdivlab( dev, wd->ivlab );
		splx(x);
		break;

	case V_W_VLAB:
		/*
		 * Write IVLAB to disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258wrtivlab( dev, wd->ivlab );
		splx(x);
		break;

	case V_L_PDIN:
		/*
		 * Load PDINFO from the user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		/*
		 * Get and validate the new PDINFO before overlaying
		 * the existing one.
		 */
		tmpbuf = geteblk();
		tmpbuf->b_flags |= (B_STALE | B_AGE);
		pdinfo = (struct pdinfo *) tmpbuf->b_un.b_addr;

		copyin( cmdarg, (caddr_t) pdinfo, sizeof(struct pdinfo));
		if ((pdinfo->sanity != VALID_PD) || 
		    (dr->suc_buf.w_f.dr_ncyl != pdinfo->cyls) ||
		    (dr->suc_buf.w_f.dr_nhead != pdinfo->tracks) ||
		    (dr->suc_buf.w_f.dr_secsiz != pdinfo->bytes) ||
		    (dr->suc_buf.w_f.dr_nsec != pdinfo->sectors))
		{
			(void) i258print( dev, "PDINFO does not match Volume Label" );
			brelse(tmpbuf);
			SETERR(EINVAL);
			break;
		}
		x = SPL();
		*(wd->pdinfo) = *pdinfo;
		wd->pdinfoloc = VTOC_SEC * dr->suc_buf.w_f.dr_secsiz;
		wd->pdinfolen= sizeof(struct pdinfo);
		wd->vtocloc = pdinfo->vtoc_ptr;
		wd->vtoclen = pdinfo->vtoc_len;
		wd->altinfoloc = pdinfo->alt_ptr;
		wd->altinfolen = pdinfo->alt_len;
		wd->mdlloc = i258ST506loc( dev, pdinfo->mfgst );
		wd->mdllen = sizeof(struct st506mdl);
		splx(x);
		brelse(tmpbuf);
		break;

	case V_U_PDIN:
		/*
		 * Upload PDINFO to the user.
		 *
		 * But first make sure: the device is a wini, and the 
		 * wini data structures are properly locked and the
		 * the current PDINFO is valid.
		 */
		if (USER_WINI_LOCK)
			break;
		if ((wd->pdinfo)->sanity != VALID_PD) {
			SETERR(EIO);
			break;
		}
		copyout( (caddr_t) wd->pdinfo, cmdarg, sizeof(struct pdinfo));
		break;

	case V_R_PDIN:
		/*
		 * Read PDINFO from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258rdpdinfo( dev, wd->pdinfo );
		splx(x);
		break;

	case V_W_PDIN:
		/*
		 * Write PDINFO to disk.
	 	*
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258wrtpdinfo( dev, wd->pdinfo);
		splx(x);
		break;

	case V_L_VTOC:
		/*
		 * Load VTOC from the user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		/*
		 * Get and validate new vtoc before overlaying the 
		 * existing one.
		 */
		tmpbuf = geteblk();
		tmpbuf->b_flags |= (B_STALE | B_AGE);
		vtoc = (struct vtoc *) tmpbuf->b_un.b_addr;

		copyin( cmdarg, (caddr_t) vtoc, sizeof(struct vtoc) );
		if (vtoc->v_sanity != VTOC_SANE) {
			SETERR(EINVAL);
			brelse(tmpbuf);
			break;
		}
		x = SPL();
		/*
		 * Update the driver's internal partition table with the
		 * partition information in the new VTOC.  But first, make
		 * sure no other partitions are open, except for partition 0.
		 * Also, after the partition table has been updated, make sure
		 * the current partition is left open. 
		 */
		for (i = 1; i < (unsigned short) dr->dr_pnum; i++) {
			if ( dr->dr_part[i].p_flag & V_OPEN) {
				splx(x);
				brelse(tmpbuf);
				if (part == 0) {
					ERRRET(EBUSY);
				}
				else {
					ERRRET(ENODEV);
				}
			}
		}
		*(wd->vtoc) = *vtoc;
		error_code = i258setparts( dev, &vtoc->v_part[0], vtoc->v_nparts );
		dr->dr_part[part].p_flag |= V_OPEN;
		splx(x);
		brelse(tmpbuf);
		break;

	case V_U_VTOC:
		/*
		 * Upload VTOC to the user.
		 *
		 * But first, make sure the device is a wini, and 
		 * the wini data structures are properly locked.
		 */
		if (USER_WINI_LOCK)
			break;
		if ((wd->vtoc)->v_sanity != VTOC_SANE) {
			SETERR(EIO);
			break;
		}
		copyout( (caddr_t) wd->vtoc, cmdarg, sizeof(struct vtoc));
		break;

	case V_R_VTOC:
		/*
		 * Read VTOC from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258rdvtoc( dev, wd->vtoc );
		splx(x);
		break;

	case V_W_VTOC:
		/*
		 * Format the opened partition.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258wrtvtoc( dev, wd->vtoc);
		splx(x);
		break;

	case V_L_SWALT:
		/*
		 * Load SW ALTINFO from user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		/*
		 * Get and validate the new SW ALTINFO before overlaying
		 * the existing one.
		 */
		altinfo = (struct alt_info *)
		kmem_zalloc(sizeof(struct alt_info), KM_SLEEP);
		copyin( cmdarg, (caddr_t) altinfo, sizeof(struct alt_info));
		if (altinfo->alt_sanity != ALT_SANITY) {
			SETERR(EINVAL);
			kmem_free( (caddr_t)altinfo, sizeof(struct alt_info));
			break;
		}
		x = SPL();
		wd->altinfo = *altinfo;
		splx(x);
		kmem_free( (caddr_t)altinfo, sizeof(struct alt_info));
		break;

	case V_U_SWALT:
		/*
		 * Upload SW ALTINFO to user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (USER_WINI_LOCK)
			break;
		if ((wd->altinfo).alt_sanity != ALT_SANITY) {
			SETERR(EIO);
			break;
		}
		copyout( (caddr_t)&(wd->altinfo), cmdarg, sizeof(struct alt_info));
		break;

	case V_R_SWALT:
		/*
		 * Read SW ALTINFO from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258rdaltinfo( dev, &(wd->altinfo));
		splx(x);
		break;

	case V_W_SWALT:
		/*
		 * Write SW ALTINFO to disk.
		 *
		 * But first, make sure the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258wrtaltinfo( dev, &(wd->altinfo) );
		splx(x);
		break;

	case V_L_MDL:
		/*
		 * Load MDL from user.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		/*
		 * Get and validate the new MDL before overlaying
		 * the existing one.
		 */
		tmpbuf = geteblk();
		tmpbuf->b_flags |= (B_STALE | B_AGE);
		mdl = (struct st506mdl *) tmpbuf->b_un.b_addr;

		copyin( cmdarg, (caddr_t) mdl, sizeof(struct st506mdl));
		if (mdl->header.bb_valid != MDL_VALID) {
			SETERR(EINVAL);
			brelse(tmpbuf);
			break;
		}
		x = SPL();
		*(wd->mdl) = *mdl;
		splx(x);
		brelse(tmpbuf);
		break;

	case V_U_MDL:
		/*
		 * Upload MDL to the user.
		 *
		 * But first make sure: the device is a wini, and
		 * the wini data structures are properly locked.
		 */
		if (USER_WINI_LOCK)
			break;
		if ((wd->mdl)->header.bb_valid != MDL_VALID) {
			SETERR(EIO);
			break;
		}
		copyout( (caddr_t) wd->mdl, cmdarg, sizeof(struct st506mdl));
		break;

	case V_R_MDL:
		/*
		 * Read MDL from disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258rdmdl( dev, wd->mdl );
		splx(x);
		break;

	case V_W_MDL:
		/*
		 * Write MDL to disk.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini, and the wini data structures
		 * are properly locked.
		 */
		if (SUSER_WINI_LOCK)
			break;
		x = SPL();
		error_code = i258wrtmdl( dev, wd->mdl);
		splx(x);
		break;

	case V_FMTLOCK:
		/*
		 * Lock the device's wini data structures.
		 *
		 * Make sure the caller is 'super user' and device is a wini.
		 */
		if (SUSER_WINI_NOLOCK)
			break;
		error_code = i258lockwini( dev, wd, (unsigned int) cmdarg );
		break;

	case V_FMTUNLOCK:
		/*
		 * Unlock the device's wini data structures.
		 *
		 * But first make sure: the caller is the 'super user',
		 * the device is a wini.
		 */
		if (SUSER_WINI_NOLOCK)
			break;
		error_code = i258unlockwini( dev, wd, (unsigned int) cmdarg );
		break;

	case V_RDABS:
		/*
		 * Read a sector from disk at the specified absolute sector #.
		 */
		copyin( cmdarg, (caddr_t) &absio, sizeof(struct absio));
		tmpbuf = geteblk();
		if (i258rawsread( dev, absio.abs_sec, dr->suc_buf.w_f.dr_secsiz,
			    tmpbuf, &error_code) == dr->suc_buf.w_f.dr_secsiz) {

			copyout( (caddr_t) tmpbuf->b_un.b_addr,
			    (caddr_t) absio.abs_buf, dr->suc_buf.w_f.dr_secsiz );
		}
		brelse(tmpbuf);
		break;

	case V_WRABS:
		/*
		 * Write a sector to disk at the specified absolute sector #.
		 */
		copyin( cmdarg, (caddr_t) &absio, sizeof(struct absio));
		tmpbuf = geteblk();
		copyin( absio.abs_buf, (caddr_t) tmpbuf->b_un.b_addr, 
		    dr->suc_buf.w_f.dr_secsiz );
		(void) i258rawswrite( dev, absio.abs_sec, dr->suc_buf.w_f.dr_secsiz, 
				tmpbuf, &error_code);
		brelse(tmpbuf);
		break;

	default:
		SETERR(EINVAL);    /* bad command */
		break;
	}
	return(error_code);
}

/***************************************************************************
 *  TITLE:  i258disksort
 *
 *  ABSTRACT:
 *    Two-way elevator algorithm, based on sectors.
 **************************************************************************/

#define	INWARD	1		/* Arm movement toward higher sectors	*/
#define	OUTWARD	0		/* Arm movement toward smaller sectors	*/
#define ADVANCE	bp1=bp2,bp2=bp2->av_forw

i258disksort(dp, bp)
struct iobuf *dp;		/* Pointer to head of active queue	*/
struct buf   *bp;		/* Pointer to buffer to be inserted	*/
{
	register struct buf *bp2; /* Pointer to next buffer in queue	*/
	register struct buf *bp1; /* Pointer to where to insert buffer  */
	int	 direct, this_d;  /* Direction of arm movement		*/
	daddr_t  sector;	  /* Disk address of target buffer	*/
	register struct iotime	  *i258it;

#ifndef NOSAR
	/* Update I/O statistics (SAR) */
	i258it = &i258stat[BOARD(bp->b_dev)][UNIT(bp->b_dev)];
	i258it->io_cnt++;
	i258it->io_bcnt += (bp->b_bcount + DEV_BSIZE - 1) >> DEV_BSHIFT;
	drv_getparm (LBOLT, (clock_t) &bp->b_start);
#endif
	bp2 = dp->b_actf;
	/*
	 * trivial case: nothing active
	 */
	if ( bp2 == NULL ) {
		dp->b_actf = dp->b_actl = bp;
		bp->av_back = (struct buf *) dp;	/* probably a no-op */
		bp->av_forw = NULL;
		return;
	}
	/*
	 * advance thru all i/o queued
	 * with the same sector number
	 */
	do
		ADVANCE;
	while ( bp2 && bp1->b_sector == bp2->b_sector );
	/*
	 * if all queued i/o has the same sector number,
	 * or if this request has the same sector number,
	 * simply queue at the tail of these i/o's, and exit.
	 */
	if ( bp2 == NULL || ( sector = bp->b_sector ) == bp1->b_sector ) {
		bp1->av_forw = bp;
		bp->av_back = bp1;		/* probably a no-op */
		bp->av_forw = bp2;
		if ( bp2 == NULL )
			dp->b_actl = bp;
		else
			bp2->av_back = bp;	/* probably a no-op */
		return;
	}
	direct = ( bp1->b_sector <= bp2->b_sector ) ? INWARD : OUTWARD;
	this_d = ( bp1->b_sector <= sector ) ? INWARD : OUTWARD;
	/*
	 * find the correct sector position
	 */
	if ( this_d == INWARD ) {
		/*
		 * if direction is inconvenient, skip outward list
		 */
		if ( direct == OUTWARD )
			while ( bp2 &&
				bp1->b_sector >= bp2->b_sector )
				ADVANCE;
		/*
		 * find position in the inward list
		 */
		while ( bp2 &&
			bp1->b_sector <= bp2->b_sector &&
			bp2->b_sector <= sector )
			ADVANCE;
	}
	else /* this_d is outward */ {
		/*
		 * if direction is inconvenient, skip inward list
		 */
		if ( direct == INWARD )
			while ( bp2 &&
				bp1->b_sector <= bp2->b_sector )
				ADVANCE;
		/*
		 * find position in the outward list
		 */
		while ( bp2 &&
			bp1->b_sector >= bp2->b_sector &&
			bp2->b_sector >= sector )
			ADVANCE;
	}
	/*
	 * Queue the request between bp1 and bp2.
	 */
	bp1->av_forw = bp;
	bp->av_back = bp1;		/* probably a no-op */
	bp->av_forw = bp2;
	if ( bp2 == NULL )
		dp->b_actl = bp;
	else
		bp2->av_back = bp;	/* probably a no-op */
}

/*****************************************************************************
 *
 * TITLE:    i258setparts
 *
 * ABSTRACT: Set the driver's internal partition table according to 
 *           the specified partition information.
 *
 ****************************************************************************/
i258setparts( dev, newpart, newcnt )
dev_t                 dev;
struct   partition    newpart[];    /* VTOC-style partition table */
unsigned short        newcnt;
{
	struct i258dev     *dd;
	struct i258drtab   *dr;
	struct i258part    *part;
	unsigned short     i;
	unsigned short     x;

	dd = &i258dev[BOARD(dev)];
	dr = i258drtab_struct(BOARD(dev), UNIT(dev));

	DEBPR(DEB_BBH,(CE_CONT,"i258setparts: Found %d new partitions:\n", newcnt));

	x = SPL();
	/*
	 * Copy partition info to driver's interal partition table.
	 * Also, zero-out any unspecified partitions. 
	 */
	dr->dr_pnum = newcnt;
	part = dr->dr_part;
	for (i = 0; i < newcnt; i++) {
		part->p_tag = newpart[i].p_tag;
		part->p_flag = newpart[i].p_flag;
		part->p_fsec = newpart[i].p_start;
		part->p_nsec = newpart[i].p_size;
		part++;
	}

	for (i = newcnt; i < V_NUMPAR; i++) {
		part->p_tag = 0;
		part->p_flag = 0;
		part->p_fsec = 0;
		part->p_nsec = 0;
		part++;
	}

#ifdef DEBUG
	if (i258_debug & (DEB_OPEN | DEB_BBH) ) {
		part = dr->dr_part;
		for (i = 0; i < newcnt; i++) { 
			cmn_err(CE_CONT, "    Part %d: tag %x flag %x fsec %lx nsec %lx\n",
			    i, part->p_tag, part->p_flag, part->p_fsec, 
			    part->p_nsec);
			part++;
		}
	}
#endif /* DEBUG*/

	splx(x);
	return(EOK);
}

/*****************************************************************************
 *
 * TITLE:    i258rdivlab
 *
 * ABSTRACT: Read Intel Volume Label (IVLAB) from disk and copy it to
 *           to the specified location.
 *
 ****************************************************************************/
i258rdivlab( dev, ivlab )
dev_t        dev;
struct ivlab    *ivlab;
{
	unsigned int    brd;
	unsigned int    unit;
	unsigned int    ivlabloc;
	unsigned int    ivlablen;
	unsigned int    readcnt;
	struct buf		*bp;
	struct btblk	*btblk;
	int				error_code;

	brd = BOARD(dev);
	unit = UNIT(dev);
	ivlabloc = i258winidata_struct(brd,unit)->ivlabloc;
	ivlablen = i258winidata_struct(brd,unit)->ivlablen;
	error_code = 0;

	if (ivlabloc == 0) {
		(void) i258print( dev, "Unable to read IVLAB - Invalid location" );
		ERRRET(ENXIO);
	}
	if (ivlablen != sizeof(*ivlab)) {
		(void) i258print( dev, "Inconsistent IVLAB size");
		ERRRET(ENXIO);
	}
	bp= geteblk();
	if (bp == (struct buf *) 0) {
		(void) i258print( dev, "Read IVLAB: No tmp buffers" );
		ERRRET(ENOMEM);
	}
	bp->b_flags |= (B_STALE | B_AGE);
	readcnt= i258rawsread( dev, BTBLK_LOC, sizeof(*btblk), bp, &error_code);
	if (readcnt != sizeof(*btblk)) {
		(void) i258print( dev, "Unable to read IVLAB sector - Disk I/O Error");
		brelse(bp);
		return(error_code);
	}
	btblk = (struct btblk *) bp->b_un.b_addr;
	if (btblk->signature != BTBLK_MAGIC) {
		i258print( dev, "Invalid VOLUME LABEL sector found" );
		brelse(bp);
		ERRRET(ENXIO);
	}
	*ivlab = btblk->ivlab;

#ifdef DEBUG
	if(i258_debug & (DEB_BBH)) {
		struct ilabdrtab     *ivlab_dr;
		ivlab_dr = (struct ilabdrtab *) &ivlab->v_dspecial[0];
		cmn_err(CE_CONT, "i258open: Volume label says disk has:\n");
		cmn_err(CE_CONT, " %d cylinders, %d heads, and %d sectors/track\n",
		    ivlab_dr->dr_ncyl, ivlab_dr->dr_nfhead, ivlab_dr->dr_nsec);
	}
#endif /* DEBUG */

	brelse(bp);
	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258wrtivlab
 *
 * ABSTRACT: Write the Intel Volume Label (IVLAB) to disk.
 *
 ****************************************************************************/
i258wrtivlab( dev, ivlab )
dev_t        dev;
struct ivlab    *ivlab;
{
	unsigned int    brd;
	unsigned int    unit;
	unsigned int    ivlabloc;
	unsigned int    ivlablen;
	unsigned int    writecnt;
	int				error_code;

	brd = BOARD(dev);
	unit = UNIT(dev);
	ivlabloc = i258winidata_struct(brd,unit)->ivlabloc;
	ivlablen = i258winidata_struct(brd,unit)->ivlablen;
	error_code = EOK;

	if (ivlabloc == 0) {
		(void) i258print( dev, "Unable to write IVLAB - Invalid location");
		ERRRET(ENXIO);
	}
	if (ivlablen != sizeof(*ivlab)) {
		(void) i258print( dev, "INCONSISTENT IVLAB SIZE");
		ERRRET(ENXIO);
	}
	writecnt =  i258rawbwrite(dev, ivlabloc, ivlablen, ivlab, 
					(struct buf *)0, &error_code);
	if (writecnt != ivlablen) {
		(void) i258print( dev, "Unable to write VOLUME LABEL");
		return(error_code);
	}

#ifdef DEBUG
	if(i258_debug & (DEB_BBH)) {
		struct ilabdrtab    *ivlab_dr;
		ivlab_dr = (struct ilabdrtab *) &ivlab->v_dspecial[0];
		cmn_err(CE_CONT, "i258open: Volume label written to disk has:\n      ");
		cmn_err(CE_CONT, " %d cylinders, %d heads, and %d sectors/track\n",
		    ivlab_dr->dr_ncyl, ivlab_dr->dr_nfhead, ivlab_dr->dr_nsec);
	}
#endif /* DEBUG */

	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258rdpdinfo
 *
 * ABSTRACT: Read Physical Device Information (PDINFO) from disk
 *
 ****************************************************************************/
i258rdpdinfo( dev, pdinfo )
dev_t        dev;
struct pdinfo    *pdinfo;
{
	unsigned int    brd;
	unsigned int    unit;
	unsigned int    pdinfoloc;
	unsigned int    pdinfolen;
	unsigned int    readcnt;
	int				error_code;

	brd = BOARD(dev);
	unit = UNIT(dev);
	pdinfoloc = i258winidata_struct(brd,unit)->pdinfoloc;
	pdinfolen = i258winidata_struct(brd,unit)->pdinfolen;
	error_code = EOK;

	if (pdinfoloc == 0) {
		(void) i258print( dev, "Unable to read PDINFO - Invalid location");
		ERRRET(ENXIO);
	}

	DEBPR(DEB_BBH,(CE_CONT, "i258open: Read PDINFO at absolute byte %d.\n", 
			pdinfoloc));

	readcnt = i258rawbread( dev, pdinfoloc, pdinfolen, pdinfo, 
						(struct buf *)0, &error_code);
	if (readcnt != pdinfolen) {
		(void) i258print( dev, "Unable to read PDINFO - Disk I/O error");
		return(error_code);
	}

	DEBPR(DEB_BBH,(CE_CONT,"Got %lx as pdinfo sanity word\n", pdinfo->sanity));

	if (pdinfo->sanity != VALID_PD) {
		(void) i258print( dev, "Invalid PDINFO found" );
		ERRRET(ENXIO);
	}
	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258wrtpdinfo
 *
 * ABSTRACT: Write Physical Device Information (PDINFO) to disk.
 *
 ****************************************************************************/
i258wrtpdinfo( dev, pdinfo )
dev_t        dev;
struct pdinfo    *pdinfo;
{
	unsigned int    brd;
	unsigned int    unit;
	unsigned int    pdinfoloc;
	unsigned int    pdinfolen;
	unsigned int    writecnt;
	int				error_code;

	brd = BOARD(dev);
	unit = UNIT(dev);
	pdinfoloc = i258winidata_struct(brd,unit)->pdinfoloc;
	pdinfolen = i258winidata_struct(brd,unit)->pdinfolen;
	error_code = EOK;

	if (pdinfo->sanity != VALID_PD) {
		(void) i258print( dev, "Unable to write PDINFO - Invalid SANITY.");
		ERRRET(ENXIO);
	}
	if (pdinfoloc == 0) {
		(void) i258print( dev, "Unable to write PDINFO - Invalid location.");
		ERRRET(ENXIO);
	}

	DEBPR(DEB_BBH,(CE_CONT,"PDINFO written to location %x:\n", pdinfoloc));

	writecnt = i258rawbwrite(dev, pdinfoloc, pdinfolen, pdinfo, 
				(struct buf *)0, &error_code);
	if (writecnt != pdinfolen) {
		(void) i258print( dev, "Unable to write PDINFO - Disk I/O error");
		return(error_code);
	}
	return(error_code);
}


/*****************************************************************************
 *
 * TITLE:    i258rdvtoc
 *
 * ABSTRACT: Read Volume Table Of Contents (VTOC) from disk.
 *
 ****************************************************************************/
i258rdvtoc( dev, vtoc )
dev_t        dev;
struct vtoc    *vtoc;
{
	unsigned int    brd;
	unsigned int    unit;
	unsigned int    vtocloc;
	unsigned int    vtoclen;
	unsigned int    readcnt;
	int				error_code;

	brd = BOARD(dev);
	unit = UNIT(dev);
	vtocloc = i258winidata_struct(brd,unit)->vtocloc;
	vtoclen = i258winidata_struct(brd,unit)->vtoclen;
	error_code = EOK;

	if (vtocloc == 0) {
		(void) i258print( dev, "Unable to read VTOC - Invalid location" );
		ERRRET(ENXIO);
	}
	readcnt = i258rawbread( dev, vtocloc, vtoclen, vtoc, 
						(struct buf *)0, &error_code);
	if (readcnt != vtoclen) {
		(void) i258print( dev, "Unable to read VTOC - Disk I/O error" );
		return(error_code);
	}

	DEBPR(DEB_BBH,(CE_CONT, "*** Partitions found: %d\n", vtoc->v_nparts));

	if (vtoc->v_sanity != VTOC_SANE) {
		(void) i258print( dev, "Invalid VTOC found" );
		ERRRET(ENXIO);
	}
	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258wrtvtoc
 *
 * ABSTRACT: Write Volume Table of Contents (VTOC) to disk.
 *
 ****************************************************************************/
i258wrtvtoc( dev, vtoc )
dev_t        dev;
struct vtoc    *vtoc;
{
	unsigned int    brd;
	unsigned int    unit;
	unsigned int    vtocloc;
	unsigned int    vtoclen;
	unsigned int    writecnt;
	int				error_code;

	brd = BOARD(dev);
	unit = UNIT(dev);
	vtocloc = i258winidata_struct(brd,unit)->vtocloc;
	vtoclen = i258winidata_struct(brd,unit)->vtoclen;
	error_code = EOK;

	if (vtoc->v_sanity != VTOC_SANE) {
		(void) i258print( dev, "Unable to write VTOC - Invalid SANITY" );
		ERRRET(ENXIO);
	}
	if (vtocloc == 0) {
		(void) i258print( dev, "Unable to write VTOC - Invalid location" );
		ERRRET(ENXIO);
	}
	writecnt =  i258rawbwrite( dev, vtocloc, vtoclen, vtoc, 
						(struct buf *)0, &error_code);
	if (writecnt != vtoclen) {
		(void) i258print( dev, "Unable to write VTOC - Disk I/O error" );
		return(error_code);
	}

	DEBPR(DEB_BBH,(CE_CONT,  "VTOC written to location %x\n", vtocloc));

	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258rdaltinfo
 *
 * ABSTRACT: Read Software Alternate Information (ALT INFO) from disk.
 *
 ****************************************************************************/
i258rdaltinfo( dev, altinfo )
dev_t        dev;
struct alt_info    *altinfo;                    /* I002 */
{
	unsigned int    brd;
	unsigned int    unit;
	unsigned int    altinfoloc;
	unsigned int    altinfolen;
	unsigned int    readcnt;
	int				error_code;

	brd = BOARD(dev);
	unit = UNIT(dev);
	altinfoloc = i258winidata_struct(brd,unit)->altinfoloc;
	altinfolen = i258winidata_struct(brd,unit)->altinfolen;
	error_code = EOK;

	if (altinfoloc == 0) {
		(void) i258print( dev, "Unable to read ALT INFO - Invalid location");
		ERRRET(ENXIO);
	}
	if (altinfolen != sizeof(*altinfo)) {
		(void) i258print( dev, "Inconsistent ALT INFO size" );
		ERRRET(ENXIO);
	}
	readcnt = i258rawbread( dev, altinfoloc, altinfolen, 
						altinfo, (struct buf *)0, &error_code);
	if (readcnt != altinfolen) {
		(void) i258print( dev, "Unable to read ALT INFO - Disk I/O error" );
		return(error_code);
	}
	if (altinfo->alt_sanity != ALT_SANITY) {
		(void) i258print( dev, "Invalid ALT INFO found" );
		ERRRET(ENXIO);
	}

#ifdef DEBUG
	if (i258_debug & (DEB_BBH)) {
		cmn_err(CE_CONT, "* SW Alt secs used: %d, SW Alt sec present: %d *\n",
		    altinfo->alt_sec.alt_used, altinfo->alt_sec.alt_reserved);
		cmn_err(CE_CONT, "* SW Alt trks used: %d, SW Alt trk present: %d *\n",
		    altinfo->alt_trk.alt_used, altinfo->alt_trk.alt_reserved);
	}
#endif /* DEBUG */

	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258wrtaltinfo
 *
 * ABSTRACT: Write Software Alternate Information (ALT INFO) to disk.
 *
 ****************************************************************************/
i258wrtaltinfo( dev, altinfo )
dev_t        dev;
struct alt_info    *altinfo;                    /* I002 */
{
	unsigned int    brd;
	unsigned int    unit;
	unsigned int    altinfoloc;
	unsigned int    altinfolen;
	unsigned int    writecnt;
	int				error_code;

	brd = BOARD(dev);
	unit = UNIT(dev);
	altinfoloc = i258winidata_struct(brd,unit)->altinfoloc;
	altinfolen = i258winidata_struct(brd,unit)->altinfolen;
	error_code = EOK;

	if (altinfo->alt_sanity != ALT_SANITY) {
		(void) i258print( dev, "Unable to write ALT INFO - Invalid SANITY" );
		ERRRET(ENXIO);
	}
	if (altinfoloc == 0) {
		(void) i258print( dev, "Unable to write ALT INFO - Invalid location" );
		ERRRET(ENXIO);
	}
	if (altinfolen != sizeof(*altinfo)) {
		(void) i258print( dev, "Inconsistent ALT INFO size" );
		ERRRET(ENXIO);
	}
	writecnt =  i258rawbwrite( dev, altinfoloc, altinfolen, 
								altinfo, (struct buf *)0, &error_code);
	if (writecnt != altinfolen) { 
		(void) i258print( dev, "Unable to write ALT INFO - Disk I/O error" );
		return(error_code);
	}

	DEBPR(DEB_BBH,(CE_CONT,"ALT INFO written to location: %x:\n", altinfoloc));

	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258ST506loc
 *
 * ABSTRACT: Calculate the disk location (absolute byte address) of the 
 *           1st copy of the Manufacturer's Defect List corresponding to 
 *           the current sector size.
 *
 *           The ST506 info is stored in the 2nd cyl of the MDL partition.
 *           This info is written on 4 different surfaces (heads) using a
 *           different sector size for each surface as follows:
 *
 *           128 bytes/sec written on last surface
 *           256 bytes/sec written on last surface - 1
 *           512 bytes/sec written on last surface - 2
 *           1024 bytes/sec written on last surface - 3
 *
 *           Each surface contains 4 copies of the defect info written with
 *           the appropriate bytes/sector configuration.  Each copy is written
 *           on every other 1K boundary starting at the beginning of the track. 
 *
 *           For example, the 128 bytes/sec defect info is written using the
 *           last head in sectors 0, 16, 32, and 48.
 *
 ****************************************************************************/
unsigned int
i258ST506loc( dev, mdlstart )
dev_t        dev;
daddr_t      mdlstart;
{
	unsigned int        brd;
	unsigned int        unit;
	struct   i258drtab  *dr;
	struct   i258dev    *dd;

	unsigned int        trkskips;
	unsigned int        mdlloc;

	brd = BOARD(dev);
	unit = UNIT(dev);
	dd = &i258dev[brd];
	dr = i258drtab_struct(brd, unit);

	/*
	 * Calculate the # of tracks to skip from the beginning of
	 * the MDL partitions in order to get proper track.
	 *
	 * Skip the 1st cylinder of the MDL partition since the ST506
	 * defect lists are contained in the 2nd cylinder of the partition.
	 *
	 * Also, skip to first few tracks of the 2nd cylinder according
	 * to the current sector size.
	 */
	trkskips = dr->suc_buf.w_f.dr_nhead;
	switch (dr->suc_buf.w_f.dr_secsiz) {
	case 1024:    
		trkskips += dr->suc_buf.w_f.dr_nhead - 4;
		break;
	case 512:    
		trkskips += dr->suc_buf.w_f.dr_nhead - 3;
		break;
	case 256:    
		trkskips += dr->suc_buf.w_f.dr_nhead - 2;
		break;
	case 128:    
		trkskips += dr->suc_buf.w_f.dr_nhead - 1;
		break;
	default:
		cmn_err(CE_CONT,  "Unable to calculate MDL location - " );
		i258print( dev, "Invalid sector size" );
		return(0);
	}

	/*
	 * Convert location of the 1st copy to an absolute byte address.
	 */
	mdlloc = (mdlstart + (trkskips*dr->suc_buf.w_f.dr_nsec)) * dr->suc_buf.w_f.dr_secsiz;
	return(mdlloc);
}

/*****************************************************************************
 *
 * TITLE:    i258rdmdl
 *
 * ABSTRACT: Read Manufacturer's Defect List (MDL) from disk.
 *
 * SCSI presents a perfect disk to the driver so fake up a
 * perfect mdl.
 *
 * This will all change with the next release of iSBC 386/258 firmware
 *
 ****************************************************************************/
i258rdmdl( dev, i258mdl)
dev_t           dev;
struct st506mdl *i258mdl;
{
	struct   st506mdl       dskmdl;
	int                     inx;

	dskmdl.header.bb_valid = MDL_VALID;
	dskmdl.header.bb_num = 0;

	for (inx=0; inx < BBH506MAXDFCTS; inx++) {
		dskmdl.defects[inx].be_cyl = 0x3030;
		dskmdl.defects[inx].be_surface = 0x30;
		dskmdl.defects[inx].be_reserved = 0x30;
	}

	*i258mdl = dskmdl;
	return(EOK);
}

/*****************************************************************************
 *
 * TITLE:    i258wrtmdl
 *
 * ABSTRACT: Write Manufacturer's Defect List (MDL) to disk.
 *
 * SCSI presents a perfect drive to the driver so this routine
 * should never be call.
 *
 * This will all change with the nex version of iSBC 386/258 firmware.
 ****************************************************************************/
i258wrtmdl( dev, i258mdl )
dev_t           dev;
struct st506mdl *i258mdl;
{
	(void) i258print(dev, 
			"MFG's DEFECT LIST should not be written to SCSI drive.");
	return(ENXIO);
}

/*****************************************************************************
 *
 * TITLE:    i258lockwini
 *
 * ABSTRACT: Locks the device's wini data structures, allocates memory to hold
 *           temporary copies of each wini data structure, and reads
 *           in a current copy of each structure.
 *
 ****************************************************************************/
i258lockwini( dev, wd, flags )
dev_t               dev;
struct   i258wini   *wd;
unsigned int        flags;
{
	unsigned int    bytecnt;
	unsigned int    byteaddr;
	unsigned int    x;
	int				error_code;

	/*
	 * Lock wini data structures so format can update structure
	 * if need without worry of a collision.
	 */
	error_code = EOK;
	x = SPL();
	if (wd->lock == LOCKED) {
		(void) i258print( dev, "Wini data structures already locked" );
		splx(x);
		ERRRET(EBUSY);
	}
	wd->lock = LOCKED;

	/*
	 * Allocate enough memory to hold all the wini data structures
	 * for this device.  Then read in a current copy of each structure.
	 */
	bytecnt = sizeof(struct ivlab) + sizeof(struct pdinfo) + 
	    		sizeof(struct vtoc) + sizeof(struct alt_info) +
			    sizeof(struct st506mdl);

	wd->pgcnt = bytecnt ;
	wd->pgaddr = (unsigned int) kmem_zalloc( wd->pgcnt, KM_SLEEP );

	DEBPR(DEB_OPEN,(CE_CONT,"i258lockwini: allocated %d bytes at %x\n", 
			wd->pgcnt, wd->pgaddr));
	byteaddr = wd->pgaddr;
	wd->ivlab = (struct ivlab *) byteaddr;
	byteaddr += sizeof(struct ivlab);

	wd->pdinfo = (struct pdinfo *) byteaddr;
	byteaddr += sizeof(struct pdinfo);

	wd->vtoc = (struct vtoc *) byteaddr;
	byteaddr += sizeof(struct vtoc);

	wd->mdl = (struct st506mdl *) byteaddr;
	byteaddr += sizeof(struct st506mdl);

	/*
	 * Read in a current copy of each data structure from
	 * the disk ONLY if the caller asked for it.
	 */
	if (flags & STRUCTIO) {
		if ( (i258rdivlab( dev, wd->ivlab) != EOK) ||
		    (i258rdpdinfo( dev, wd->pdinfo) != EOK) ||
		    (i258rdvtoc( dev, wd->vtoc) != EOK) ||
		    (i258rdaltinfo( dev, &(wd->altinfo)) != EOK)    ||
		    (i258rdmdl( dev, wd->mdl) != EOK)) {

			/*
			 * If all structures are not successfully read,  
			 * then unlock everything.
			 */
			(void) i258print( dev,
			    "Unable to read all wini data structures" );
			(void) i258unlockwini( dev, wd, ~STRUCTIO );
			return(EIO);
		}
	}
	splx(x);

	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258unlockwini
 *
 * ABSTRACT: Unlock the device's wini data structures.
 *
 ****************************************************************************/
i258unlockwini( dev, wd, flags )
dev_t               dev;
struct   i258wini   *wd;
unsigned int        flags;
{
	unsigned short  errcnt;
	unsigned int    x;
	int				error_code;

	error_code = EOK;
	if (wd->lock != LOCKED) {
		i258print( dev, "Wini data not locked" );
		ERRRET(ENOLCK);
	}

	/*
	 * Update the disk copies of the each data structure
	 * ONLY if the caller asked for it.
	 */
	errcnt = 0;
	if (flags & STRUCTIO) {

		if (i258wrtivlab( dev, wd->ivlab) != EOK)
			errcnt++;

		if (i258wrtpdinfo( dev, wd->pdinfo) != EOK)
			errcnt++;

		if (i258wrtvtoc( dev, wd->vtoc) != EOK)
			errcnt++;

		if (i258wrtaltinfo( dev, &(wd->altinfo)) != EOK)
			errcnt++;

		if (i258wrtmdl( dev, wd->mdl) != EOK)
			errcnt++;
	}

	x = SPL();
	wd->ivlab = (struct ivlab *) 0;
	wd->pdinfo = (struct pdinfo *) 0;
	wd->vtoc = (struct vtoc *) 0;
	wd->mdl = (struct st506mdl *) 0;

 	kmem_free( (caddr_t)wd->pgaddr, wd->pgcnt);
	wd->pgcnt = 0;
	wd->pgaddr = 0;

	wd->lock = ~LOCKED;
	splx(x);

	if (errcnt != 0) {
		(void) i258print( dev, "Unable to update wini data structures" );
		return(EIO);
	}

	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258fmtpart
 *
 * ABSTRACT: Format the opened partition using the specified method(s).
 *
 ****************************************************************************/
i258fmtpart( dev, intlv, method )
dev_t     dev;
ushort    intlv;
ushort    method;
{
	struct i258dev      *dd;

	dd = &i258dev[BOARD(dev)];
	dd->d_ftk.f_intl = intlv;
	dd->d_ftk.f_type = method;

	return(i258fmtsend(dev));

}

/*****************************************************************************
 *
 * TITLE:    i258fmtsend
 *
 * ABSTRACT: Send a format request to the controller
 *
 ****************************************************************************/
i258fmtsend( dev )
{
	struct i258dev      *dd;
	struct i258req      *rq = (struct i258req *) NULL;

	int    			brd;
	int    			rn;
	unsigned short  error_code;

	brd = BOARD(dev);
	dd = &i258dev[brd];

	DEBPR(DEB_IOCTL,(CE_CONT, "i258ioctl: format (%x) (%x) (%x) (%c)\n", 
			dd->d_ftk.f_track, dd->d_ftk.f_type, dd->d_ftk.f_intl, 
			dd->d_ftk.f_pat[0]));

	/*
	 * Get a request slot for doing the operation.
	 */
	while ((rn = i258getreq(dd)) == -1) {
		dd->d_flags |= I258_NEED_REQ;
		(void) sleep( (caddr_t) &dd->req_use, PRIBIO );
	}

	DEBPR(DEB_IOCTL,(CE_CONT, "i258tpioctl: rn=%x, tstate=%x\n",
					rn,dd->d_tstate[UNIT(dev)]));

	/*
	 * Fill in the request information.
	 */
	rq = &dd->reqinfo[rn];
	rq->r_dev = dev;
	rq->r_bp = (struct buf *)0;
	if ((rq->r_mbp = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get a message buffer\n");
	rq->r_flags |= i258RQ_IM_WAITING;
	if ((rq->r_dbp = mps_get_dmabuf(2,DMA_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get data buffer\n");
	(rq->r_dbp)->count = sizeof(struct i258format);
	(rq->r_dbp)->address = kvtophys(&dd->d_format);

	/*
	 * Send the request and wait for the response. 
	 */
	i258io( brd, rn, FORMAT_CMD );
	while (dd->reqinfo[rn].r_flags & i258RQ_IM_WAITING)
		(void) sleep( (caddr_t) &dd->reqinfo[rn], PRIBIO+1 );

	/*
	 * Check the returned message for an error.
	 */
	if (mps_msg_iserror(rq->r_mbp) ||
	    mps_msg_iscancel(rq->r_mbp) ||
	    mps_msg_isreq(rq->r_mbp) ||
	    i258checkerr( brd, (struct i258PSM *)mps_msg_getudp(rq->r_mbp) )) {
		SETERR(EIO);

		DEBPR(DEB_FMT,(CE_CONT,  "FMT ERROR: trk %d,  type %d\n",
			    dd->d_ftk.f_track, dd->d_ftk.f_type));

	} else {
		error_code = EOK;
	}

	i258setup( brd, rn );
	return(error_code);
}

/*****************************************************************************
 *
 * TITLE:    i258sendcmd
 *
 * ABSTRACT: Format a request and send the command to the controller
 *
 ****************************************************************************/
i258sendcmd(dev, cmd)
dev_t	dev;
ushort	cmd;        /* command code */
{
	struct i258dev      *dd;
	struct i258req      *rq = (struct i258req *) NULL;
	struct i258PSM		*psm;
	struct i258rdvfy  	*vd;

	int    				brd;
	int    				rn;
	int    				x;
	unsigned int        unit;
	unsigned short  	rtncode;

	brd = BOARD(dev);
	unit = UNIT(dev);
	dd = &i258dev[brd];

	DEBPR(DEB_IOCTL,(CE_CONT, "i258sendcmd: command code (%x)",  cmd));

	/*
	 * Get a request slot for doing the operation.
	 */
	while ((rn = i258getreq(dd)) == -1) {
		dd->d_flags |= I258_NEED_REQ;
		(void) sleep( (caddr_t) &dd->req_use, PRIBIO );
	}

	DEBPR(DEB_IOCTL,(CE_CONT, "i258sendcmd: rn=%x, tstate=%x\n",
			rn, dd->d_tstate[unit]));

	/*
	 * Fill in the request information.
	 */
	x = SPL();
	rq = &dd->reqinfo[rn];
	rq->r_dev = dev;
	rq->r_bp = (struct buf *)0;
	if ((rq->r_mbp = mps_get_msgbuf(KM_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,"i258: cannot get a message buffer\n");
	rq->r_flags |= i258RQ_IM_WAITING;
	rq->r_dbp = NULL;

	/*
	 * Send the request and wait for the response. 
	 */
	i258io( brd, rn, cmd );
	splx(x);
	while (dd->reqinfo[rn].r_flags & i258RQ_IM_WAITING)
		(void) sleep( (caddr_t) &dd->reqinfo[rn], PRIBIO+1 );

	x = SPL();
	psm = (struct i258PSM *)mps_msg_getudp(rq->r_mbp);
	if (psm->P_cmd_type == READ_DATA_VERIFY) {
		vd = i258rdvfy_struct(brd,unit);
		drv_getparm (LBOLT, (time_t) &vd->vfytime);
	}
	splx(x);
	/*
	 * Check the returned message for an error.
	 */
	if (mps_msg_iserror(rq->r_mbp) ||
	    mps_msg_iscancel(rq->r_mbp) ||
	    mps_msg_isreq(rq->r_mbp) ||
		i258checkerr(brd, (struct i258PSM *)mps_msg_getudp(rq->r_mbp))) {
		rtncode = EIO;
	}
	else {
		rtncode = EOK;
	}
	i258setup(brd, rn);
	return(rtncode);
}


/*****************************************************************************
 *
 * TITLE:    i258rawbread
 *
 * ABSTRACT: i258 Raw Byte Read
 *
 *           Read data from the specified device beginning with the specified
 *           absolute byte offset (from the beginning of the disk).  Data is
 *           repeatly read from the device until 'count' bytes have been read
 *           or until an error occurs.  The data is placed in the specified
 *           destination address.  Optionally, the callers own buffer 'bp' can
 *           be used for reading.  If 'bp' has a value of '0' then a temporary
 *           buffer will be allocated.  The actual number of bytes read is
 *           returned to the user.
 *
 ****************************************************************************/
i258rawbread( dev, start, count, destaddr, bp, ecode)
dev_t			dev;		/* Device (maj/min) to read from */
unsigned int	start;		/* Absolute byte offset to begin reading */
unsigned int	count;		/* Number of bytes to read */
char			*destaddr;	/* Destination address to place the data */
struct buf		*bp;		/* Disk buffer to use when reading */
int				*ecode;

#define    LOCALBUF    0x0001    /* Indicates a temporary buffer was allocated */

{
	struct i258dev	*dd;		/* Ptr to the device's dev structure */
	char			*dest;		/* Destination addr of next data byte */
	char			*src;		/* Source addr of next data byte */

	daddr_t			sec;		/* Sector # of next byte to be read */
	unsigned short	secsize;	/* # of bytes in a disk sector */

	unsigned int    readlen;    /* # of bytes actually read from disk */
	unsigned int    offset;		/* Offset into sec to begin reading */
	unsigned int    copylen;    /* # of bytes to copy on this loop */
	ushort			flags;		/* Misc status flags */
	ushort			i;			/* Loop counter */

	/*
	 * Setup the pointers and counters.
	 */
	dd = &i258dev[ BOARD(dev) ];
	secsize = i258drtab_struct(BOARD(dev), UNIT(dev))->suc_buf.w_f.dr_secsiz;

	flags = 0;
	if (bp == (struct buf *) 0) {
		bp = geteblk();
		if (bp == (struct buf *) 0) {
			(void) i258print( dev, "Unable to get temporary disk block" );
			*ecode = ENOMEM;
			return(-1);
		}
		flags |= LOCALBUF;
	}

	/*
	 * While there is still more data to be read, keep 
	 * reading sectors from the disk and copy the data
	 * to it final destination. 
	 */
	dest = destaddr;
	while (count > 0) {
		sec = start / secsize;
		offset = start % secsize;
		/*
		readlen = min( offset+count, BSIZE );
		*/
		readlen = i258rawsread( dev, sec, BSIZE, bp, ecode);
		if (readlen != BSIZE) {
			break;
		}
		src = (char *) (bp->b_un.b_addr + offset);
		copylen = min( (int)(readlen - offset), (int)count);
		for (i = 0; i < copylen; i++ )
			*dest++ = *src++;
		start += copylen;
		count -= copylen;
	}

	if (flags & LOCALBUF)
		brelse(bp);

	return( dest-destaddr );
}

/*****************************************************************************
 *
 * TITLE:    i258rawsread
 *
 * ABSTRACT: i258 Raw Sector Read
 *
 *           Reads data from the specified device beginning with the specified
 *           absolute sector #.  One attempt is made to read 'count' bytes into
 *           the specified buffer.  The buffer MUST be large enough to hold all
 *           of the 'count' bytes.  The number of bytes actually read are
 *           returned to the caller.
 *
 ****************************************************************************/
i258rawsread( dev, sec, count, bp, ecode)
dev_t          dev;        /* Disk device to read from */
daddr_t        sec;        /* Absolute sector number to begin reading */
unsigned int   count;      /* # of bytes to read */
struct   buf   *bp;        /* Buffer to hold the data */
int			   *ecode;
{
	struct i258dev      *dd;     /* Pntr to the device's dev structure */
	struct i258drtab    *dr;     /* Pntr to dev's dr_tab data struct */

	struct i258req      *rq;

	int        			rn;
	unsigned int        totsec;  /* Total # of sec of physical device */
	unsigned int        x;       /* Current interrupt level */

	/*
	 * Find the proper dev structure.
	 */
	dd = &i258dev[BOARD(dev)];
	dr = i258drtab_struct(BOARD(dev), UNIT(dev));
	totsec = dr->suc_buf.w_f.dr_ncyl * dr->dr_spc;

	/*
	 * Make sure the device is ready to be used.
	 */
	if ((dd->d_sflags[UNIT(dev)] & SF_READY) == 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EBUSY;
		*ecode = EBUSY;
		biodone(bp);
		return(-1);
	}

	/*
	 * Since the 258 can handle only whole-sector reads,
	 * make sure the byte count is an integral # of sectors.
	 */
	if ((count % dr->suc_buf.w_f.dr_secsiz) != 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		*ecode = ENXIO;
		biodone(bp);
		return(-1);
	}

	/*
	 * Make sure we're not starting beyond or reading past the 
	 * physical end of media.
	 */
	if ((sec >= totsec) ||
	    ((sec + (count/dr->suc_buf.w_f.dr_secsiz) -1) > totsec)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		*ecode = ENXIO;
		biodone(bp);
		return(-1);
	}

	/*
	 * Setup the request buffer.
	 */
	bp->b_flags |= B_READ;
	bp->b_flags &= ~(B_DONE | B_ERROR);
	bp->b_error = 0;
	bp->b_blkno = sec * dr->dr_lbps;
	bp->b_sector = sec;
	bp->b_dev = BASEDEV(dev);
	bp->b_proc = 0x00;
	bp->b_bcount = count;
	bp->b_resid = 0;

	/*
	 * Queue the request and wait for a response.
	 * Note - We don't use the strategy routine
	 * because we may want to read outside of the
	 * partition boundaries.
	 */
	x = SPL();
	i258disksort( dd->d_bufh, bp );

	rn = i258getreq(dd);
	if (rn != -1) {
		rq = &dd->reqinfo[rn];
		rq->r_flags &= ~i258RQ_IM_WAITING;
		rq->r_mbp = (mps_msgbuf_t *) 0;
		i258setup((int)BOARD(dev),rn);
	} else {
#ifdef DEBUG
		if (i258_debug & DEB_BBH)
			(void) i258print( dev, "i258rawsread: No request slots available" );
#endif
	}

	splx(x);
	biowait(bp);

	/*
	 * Check for errors.
	 */
	if (bp->b_flags & B_ERROR) {
		geterror(bp);
		*ecode = EIO;
		return(-1);
	}

	return( count - bp->b_resid );
}

/*****************************************************************************
 *
 * TITLE:    i258rawbwrite
 *
 * ABSTRACT: i258 Raw Byte Write
 *
 *           Write data to the specified device beginning with the specified
 *           absolute byte offset (from the beginning of the disk).  Data is
 *           repeatly written to the device until 'count' bytes have been
 *           written or until an error occurs.  Optionally, the callers own 
 *           buffer 'bp' can be used for writing.  If 'bp' has a value of '0'
 *           then a temporary buffer will be allocated.  The number of bytes
 *           actually written is returned to the user.
 *
 ****************************************************************************/
i258rawbwrite( dev, start, count, srcaddr, bp, ecode )
dev_t			dev;		/* Device (maj/min) to write to */
unsigned int	start;		/* Absolute byte offset to begin writing */
unsigned int	count;		/* Number of bytes to write */
char			*srcaddr;	/* Source address of the data to write */
struct buf		*bp;		/* Disk buffer to use when writing */
int				*ecode;		/* error code */

#define    LOCALBUF    0x0001    /* Indicates a temporary buffer was allocated */

{
	struct i258dev	*dd;		/* Ptr to the device's dev structure */
	char			*src;		/* Source addr of next data byte */
	char			*dest;		/* Destination addr of next data byte */

	daddr_t			sec;		/* Sector # of next byte to write */
	unsigned short	secsize;    /* # of bytes in a disk sector */

	unsigned int    readlen;    /* # of bytes actually read from dsk */
	unsigned int    writelen;	/* # of bytes actually written to dsk */
	unsigned int    offset;		/* Offset into sec to begin writing */
	unsigned int    copylen;    /* # of bytes to copy on this loop */
	ushort			flags;		/* Misc status flags */
	ushort			i;			/* Loop counter */
	/*
	 * Setup the pointers and counters.
	 */
	dd = &i258dev[ BOARD(dev) ];
	secsize = i258drtab_struct(BOARD(dev), UNIT(dev))->suc_buf.w_f.dr_secsiz;

	flags = 0;
	if (bp == (struct buf *) 0) {
		bp = geteblk();
		if (bp == (struct buf *) 0) {
			(void) i258print( dev, "Unable to get temporary disk block" );
			*ecode = ENOMEM;
			return(-1);
		}
		flags |= LOCALBUF;
	}

	/*
	 * While there is still more data to be written, keep 
	 * writing sectors to disk.
	 * 
	 * Note - 'secsize' should probably be used for the read/write
	 *    granularity but we're not sure if non-BSIZE sizes are
	 *    readable and writable.
	 */
	src = srcaddr;
	while (count > 0) {
		sec = start / secsize;
		offset = start % secsize;
		readlen = i258rawsread( dev, sec, BSIZE, bp, ecode);
		if (readlen != BSIZE) {
			break;
		}
		dest = (char *) (bp->b_un.b_addr + offset);
		copylen = min( (int) (readlen - offset), (int)count);
		for (i = 0; i < copylen; i++ )
			*dest++ = *src++;
		writelen =  i258rawswrite( dev, sec, BSIZE, bp, ecode);
		if (writelen != BSIZE) {
			src -= copylen;
			break;
		}
		start += copylen;
		count -= copylen;
	}

	if (flags & LOCALBUF)
		brelse(bp);

	if (*ecode != EOK)
		return(-1);
	else
		return( src-srcaddr );
}

/*****************************************************************************
 *
 * TITLE:    i258rawswrite
 *
 * ABSTRACT: i258 Raw Sector Write
 *
 *           Writes data to the specified device beginning with the specified
 *           absolute sector #.  One attempt is made to write 'count' bytes from
 *           the specified buffer.  The number of bytes actually write are
 *           returned to the caller.
 *
 ****************************************************************************/
i258rawswrite( dev, sec, count, bp, ecode)
dev_t          dev;        /* Disk device to write to */
daddr_t        sec;        /* Absolute sector number to begin writing */
unsigned int   count;      /* # of bytes to write */
struct   buf   *bp;        /* Buffer to hold the data */
int			   *ecode;	   /* error code */
{
	struct i258dev      *dd;    /* Pntr to the device's dev structure */
	struct i258drtab    *dr;    /* Pntr to dev's dr_tab data struct */
	struct i258req      *rq;
	unsigned int        rn;
	unsigned int        x;      /* Current interrupt level */
	unsigned int        totsec; /* Total # of sec of physical device */

	/*
	 * Find the proper dev structure.
	 */
	dd = &i258dev[BOARD(dev)];
	dr = i258drtab_struct(BOARD(dev), UNIT(dev));
	totsec = dr->suc_buf.w_f.dr_ncyl * dr->dr_spc;

	/*
	 * Make sure the device is ready to be used.
	 */
	if ((dd->d_sflags[UNIT(dev)] & SF_READY) == 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EBUSY;
		*ecode = EBUSY;
		biodone(bp);
		return(-1);
	}

	/*
	 * Since the 258 can NOT handle non sector-size byte counts,
	 * make sure the byte count is an integral # of sectors.
	 */
	if ((count % dr->suc_buf.w_f.dr_secsiz) != 0) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		*ecode = ENXIO;
		biodone(bp);
		return(-1);
	}

	/*
	 * Make sure we're not starting beyond or reading past the 
	 * physical end of media.
	 */
	if ((sec >= totsec) ||
	    ((sec + (count/dr->suc_buf.w_f.dr_secsiz) -1) > totsec)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		*ecode = ENXIO;
		biodone(bp);
		return(-1);
	}

	/*
	 * Setup the request buffer.
	 */
	bp->b_flags |= B_WRITE;
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR);
	bp->b_error = 0;
	bp->b_blkno = sec * dr->dr_lbps;
	bp->b_sector = sec;
	bp->b_dev = BASEDEV(dev);
	bp->b_proc = 0x00;
	bp->b_bcount = count;
	bp->b_resid = 0;

	x = SPL();
	/*
	 * Queue the request and wait for a response.
	 * Note - We don't use the strategy routine
	 * because we may want to write outside of the
	 * partition boundaries.
	 */
	i258disksort( dd->d_bufh, bp );

	rn = i258getreq(dd);
	if (rn != -1) {
		rq = &dd->reqinfo[rn];
		rq->r_flags &= ~i258RQ_IM_WAITING;
		rq->r_mbp = (mps_msgbuf_t *) 0;
		i258setup(BOARD(dev),rn);
	} else {
#ifdef DEBUG
		if (i258_debug & DEB_BBH)
			(void) i258print(dev, "i258rawswrite: No request slots available");
#endif
	}

	splx(x);
	biowait(bp);

	/*
	 * Check for errors.
	 */
	if (bp->b_flags & B_ERROR) {
		geterror(bp);
		return(-1);
	}

	return( count - bp->b_resid );
}

/*****************************************************************************
 *
 * TITLE:    i258prtstring
 *
 * ABSTRACT: i258 HEX dump
 *
 *           Given a string and a length put out a HEX dump
 *
 ****************************************************************************/
i258prtstring( string, len )
char	*string;
int	len;
{
	char *ptr;
	int  i;

	ptr=string;
	for (i = 0; i < len; i++ ){
		if ((i % 16) == 0) /* print out 16 bytes and then a return */
			cmn_err(CE_CONT, "\n");
		cmn_err(CE_CONT, "%x%x ", (*ptr>>4)&0xf, *ptr&0xf);
		ptr++;
	}
	cmn_err(CE_CONT, "\n");
}
/*****************************************************************************
 *
 * TITLE:    i258add_resp
 *
 * ABSTRACT: 
 *           Add the response to locate in a sorted order
 *
 ****************************************************************************/
void
i258add_resp(resp, hostid, portid, cnt)
struct		i258resp	*resp;
unsigned 	short		hostid, portid;
unsigned 	char		cnt;
{
	struct	i258resp	xresp;
	struct	i258resp	yresp;
	unsigned short		i, tmp;
	
	/* keep the list sorted by inserting in the right place */

	xresp.hostid = hostid;
	xresp.portid = portid;
	xresp.nservers = cnt;
	tmp = (cnt == 0) ? (sizeof(i258pci_servers)/sizeof(struct i258resp)) :
			(sizeof(i258pci_resp )/sizeof(struct i258resp));
  
	for (i = hostid; i < tmp; i++) {
		if (((xresp.hostid == resp->hostid) && 
			 (xresp.portid == resp->portid )) ||
			(resp->portid == 0)) {
			resp->hostid = xresp.hostid;
			resp->portid = xresp.portid;
			resp->nservers = xresp.nservers;
			break;	
		}
		if (xresp.hostid < resp->hostid) { /* insert it here */
			yresp.hostid = resp->hostid;
			yresp.portid = resp->portid;
			yresp.nservers = resp->nservers;

			resp->hostid = xresp.hostid;
			resp->portid = xresp.portid;
			resp->nservers = xresp.nservers;

			xresp.hostid = yresp.hostid;
			xresp.portid = yresp.portid;
			xresp.nservers = yresp.nservers;
		}
		resp++;
	}
}

int
i258size(dev)
dev_t dev;
{
	register int nblks;
	struct	i258dev		*dd;
	struct	i258drtab	*dr;
	unsigned			unit;
	unsigned char		devopened;

	if (getminor(dev) > i258maxmin)
		return(ENXIO);

	if (BOARD(dev) >= (unsigned char)N_i258)
		return(ENXIO);

	if (ISTAPE(dev)) 
		return(ENXIO);

	unit = UNIT(dev);

	dd = &i258dev[BOARD(dev)];
	dr = i258drtab_struct(BOARD(dev), unit);

	DEBPR(DEB_SIZE,(CE_CONT, 
		"i258size: board=%d, unit=%x, flags=%x, partition=%d\n",
			BOARD(dev), unit, dd->d_flags, PARTITION(dev)));

	devopened = 0;	
	nblks = -1;

	if (!(dd->d_flags & I258_ALIVE) || 
		!(dd->d_sflags[unit] & SF_VTOC_OK))  {

		if (i258open(&dev, 0, 0, (struct cred *)0) != EOK) { /* XXX - flags? */
				return(nblks);  
		}
		devopened++;	
	}

	if (ISWINI(dev) && (PARTITION(dev) != 0)) {
		DEBPR(DEB_SIZE,(CE_CONT,
			"i258size: dr_part[%d].p_flag=%x, d_sflags=%x\n",
			PARTITION(dev), dr->dr_part[PARTITION(dev)].p_flag, 
			dd->d_sflags[unit]));
	}

	/*
	 *  Note: the following three conditions apply:
	 * 1. Device is a floppy ( does not have vtoc)
	 * 2. Device is a formatted wini (has vtoc)
	 * 3. Device is an unformatted wini(does not have vol. label and vtoc)
	 *    and it is for partition 0.
	 */
				
	if ( ISFLOP(dev) || 
	   ( (dr->dr_part[PARTITION(dev)].p_flag & V_VALID) &&
			(dd->d_sflags[unit] & SF_VTOC_OK) ) ||
	   (!(dd->d_sflags[unit] & SF_VTOC_OK) && (PARTITION(dev) == 0)) )  {

		nblks = i258blksize(dev); 
	}
	if (devopened)
		(void) i258close(dev, 0, 0, (struct cred *)0);

	DEBPR(DEB_SIZE,(CE_CONT, "i258size: Size: %d dev: %x unit %x\n",
			   nblks, dev ,unit));

	return (nblks);
}

i258blksize(dev)
register	dev_t	dev;
{
	int 				nblks;
	struct	i258dev		*dd;
	struct	i258drtab	*dr;
	unsigned short 		secsiz; 
	unsigned			unit;
		

	dd = &i258dev[BOARD(dev)];
	unit = UNIT(dev);
	dr = i258drtab_struct(BOARD(dev), unit);
	nblks = dr->dr_part[PARTITION(dev)].p_nsec;
	secsiz = dr->suc_buf.w_f.dr_secsiz;
	/* convert to 512 byte blocks */
	if (secsiz < 512) 
		nblks = (int) (nblks * secsiz)/(int) 512;	
	else
		nblks = nblks * (secsiz/512);	

	return(nblks);
}
