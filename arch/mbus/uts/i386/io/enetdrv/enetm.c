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

#ident	"@(#)mbus:uts/i386/io/enetdrv/enetm.c	1.3.2.1"

#ident "@(#)enetm.c  $SV_enet SV-Eval01 - 06/25/90$"

 /*
 *   enetm.c: main routines for the stream driver for the Intel enet
 *           transport board with iNA960/961 firmware.
 *
 *	Modification log:
 *	
 *	I000	7/20/1987	DF	Intel
 *		Added support for enetinfo command.
 *	I001	8/14/1987	DF	Intel
 *		Added support for enetfull_reset command.
 *	I002	8/28/87		DF	Intel
 *		Fixed the enetfull_reset not releasing buffer problem.
 *	I003	9/4/87		DF	Intel
 *		Zeroed onboard iNA961 version after enetreset.
 *	I004	9/21/87		DF	Intel
 *		Moved enet_init() and enetfull_reset() to enethwdep.c.
 *		This is done so that enetm.c can be common to the MBI 552
 *		driver and the MBII 530 driver.
 *	I005	9/21/87		DF	Intel
 *		Moved NVC to space.c.
 *	I006	4/13/88		rjs/slh		Intel
 *		Made U_DEFAULTS configurable by user.
 *	I007	4/13/88		rjs		Intel
 *		Made U_DEFAULTS configurable by user.
 *	I008	7/5/88		rjs		Intel
 *		Wait in enetclose() for completion of writes.
 *	I009	1/17/89		NL
 *		Added #ifdef for V32
 *	I010	1/20/89		NL
 *		Made (V31 || V32) the default option.
 *	I011	07/05/89	rjf		Intel
 *		Added support for V4.
 *	I012	07/11/89	rjf		Intel
 *		Made lint fixes.
 *	JB	30AUG89
 *		fixed McDD unexpected disconnect problem; endpoint was
 *		freed up too early
 */

#define DEBUG 1

#include "sys/enet.h"
#include <sys/immu.h>
#include <sys/param.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/debug.h>

extern struct req_blk	enet_rb_list[];	/* I002 */

extern int enet_nvc;				/* I005 */
extern opts enet_u_defaults;			/* I006 */
extern int cdevcnt;
char *enetversion = "4.0";
/* 
 * This must not be commented if any module in enet has DEBUG turned on.
*/
char debugtxt[10001];
char *debugptr = debugtxt;

/*
 * enetmod specific statistics counts are as follows:
 *
 *	enet_stat[ST_ALFA]	Count of STREAMS buffer allocation failures
 *	enet_stat[ST_RMSG]	Count of received messages
 *	enet_stat[ST_SMSG]	Count of sent messages
 *	enet_stat[ST_BRCV]	Count of data bytes passed to users
 *	enet_stat[ST_BSNT]	Count of data bytes written for users
 *	enet_stat[ST_CURO]	Number of currently open endpoints
 *	enet_stat[ST_TOTO]	Total number of opens done
 *	enet_stat[ST_CURC]	Number of currently connected endpoints
 *	enet_stat[ST_TOTC]	Total number of connections made
 *	enet_stat[ST_NORES]	Count of send failures due to full iNA961 queue
 *	enet_stat[ST_UNKCLO]	Count of bad close operations due to bad CDB
 *	enet_stat[ST_RBFA]	Number of failures due to lack of req_blks
 *	enet_stat[ST_PDFA]	Number of failures due to lack of pend_lists
 */

ulong enet_stat[enet_SCNT] = {0};

int enetdevflag = 0;		/* V4.0 style driver */

/*
 * The following are standard STREAMS data structures.
 */
int enetopen(), enetclose();
extern int enetrsrv(), enetwput(), enetwsrv();
/* extern void enet_closed_open_complete(); */	/* JB */

struct module_info 
	enet_winfo = {enet_NUMBER, enet_NAME,
		enet_STRMIN, enet_STRMAX, enet_HIWAT, enet_LOWAT},
	enet_rinfo = {enet_NUMBER, enet_NAME,
		enet_STRMIN, enet_STRMAX, enet_HIWAT, enet_LOWAT};

struct module_stat enetmod_stat = {0, 0, 0, 0, 0,
					(char *)enet_stat, enet_SCNT};

struct qinit
	enetwinit = {enetwput, enetwsrv,  NULL,  NULL, NULL,
			&enet_winfo, &enetmod_stat},
	enetrinit = {NULL, enetrsrv,  enetopen,  enetclose, NULL,
			&enet_rinfo, &enetmod_stat};

struct streamtab enetinfo = {&enetrinit, &enetwinit, NULL, NULL};

/*
 * enet debug level global variable:
 *	0 = DEB_NONE - show nothing
 *	1 = DEB_ERROR - show most error conditions
 *	2 = DEB_CALL - show only call entries and returns
 *	3 = DEB_FULL - show everything
 */
int enet_debug = 0;


/*
 * The following are major variables defined in the /etc/master.d
 * file for the ethernet driver:
 *
 *	enet_endpoints: One for each STREAM (aka virtual circuit)
 *	enet_n_endpoints: Max number of possible endpoints
 */

extern endpoint		enet_endpoints[];
extern int		enet_n_endpoints;
extern struct enetboard	enet_boards[];
extern int		enet_sh_hiwat;
extern int		enet_sh_lowat;
extern int		enet_majtobnum[];


/*
 * enetopen
 *
 * Called when the driver is opened.
 *
 * The user is trying to establish a virtual circuit endpoint.
 * There are two ways the driver can be opened:
 *
 *     minor(rdev) = n, sflag = 0
 *			Indicates that a user is trying to open endpoint 'n'.
 *     minor(rdev) = 0, slfag = CLONEOPEN  
 *			Indicates that a user is trying to allocate a new
 *                      endpoint.
 *
 * Note the use of the flag FNDELAY.  Since the only place we can get hung up
 * is waiting for a free buffer to set the streamhead configurations, that is
 * the only place it is used.  All other opens happen immediately anyway
 * (assuming the up-stream modules don't delay processing the SETOPTS message).
 */
int
enetopen(rd_q, rdev, oflag, sflag, credp)
queue_t *rd_q;
dev_t *rdev;		/* Pointer to device name */
int oflag, sflag;
struct cred *credp;
{
	minor_t minor_dev;
	register endpoint *ep;
	mblk_t *mptr;
	register struct stroptions *soptr;
	int i;
	static minor_t last;

	DEBUGP(DEB_CALL,(CE_CONT, "enetopen(dev = %d)\n", *rdev));
	DEBUGP(DEB_CALL,(CE_CONT, "enetopen: oflag = %x sflag = %x\n", oflag, sflag));
	if(oflag & ~(FREAD|FWRITE|FNDELAY|FEXCL)) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enetopen EINVAL: oflag = %x\n", oflag));
		return(EINVAL);
	}
	/*
	 * Check that we are configured to support this major device
	 */
	if(enet_majtobnum[getmajor(*rdev)] == -1) {
		/*cmn_err(CE_WARN, "Ethernet Driver: Open on invalid major number - %d\n",
			getmajor(*rdev));*/
		return(EACCES);
	}
	/*
	 * Make sure the board is booted
	 */
	minor_dev = getminor(*rdev);
/*  I001
	if((!(enet_boards[enet_majtobnum[major(rdev)]].state & BOOTED)) && 
	((sflag == CLONEOPEN) || (minor_dev != 0))) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enetopen: board state = %x, sflag = %x, minor_dev = %x\n",
				enet_boards[enet_majtobnum[major(rdev)]].state,
				sflag, minor_dev));
		u.u_error = EINVAL;
		return(OPENFAIL);
	}
*/
	/*
	 * Make sure we don't exceed the configured max number of VC's 
	 */
	if(enet_stat[ST_CURO] >= enet_nvc) {
		DEBUGP(DEB_ERROR,(CE_CONT, "enetopen ENOSPC: exceeded max VC's\n"));
		return(ENOSPC);
	}
	if((sflag == CLONEOPEN) && (minor_dev == 0)) {
		/*
		 * Find a free endpoint (i.e., select an available minor
		 * device number).
		 */
		/*
                 * The problem is that ep's are released too soon. When an ep
                 * is closed, close RB's are sent to all dependent CDB's
                 * and the ep is marked as available. The problem happens
                 * when another open allocates the same ep again before
                 * the close RB's have returned from iNA. When the close
                 * RB's then come in, the new ep doesn't expect them and
                 * things go wrong.
                 *
                 * This change here is an attempt to reduce the probability
                 * of this happening by allocating the ep's on a circular
                 * basis, i.e. starting the search for an available ep where
                 * we left off last time. This makes it unlikely that an ep
                 * will be allocated again soon after it is closed.
                 *
                 * The old code was:
                 *
		 * for(minor_dev=1; minor_dev < enet_n_endpoints; minor_dev++)
		 *	if(enet_endpoints[minor_dev].str_state == C_IDLE)
		 *		break;
		 * if(minor_dev == enet_n_endpoints) {
		 *	return(ENOSPC);
		 */
		DEBUGP(DEB_FULL,(CE_CONT, "enetopen: finding free endpoint\n"));
		for(i=1,minor_dev=(last+1); i<enet_n_endpoints; i++, minor_dev++) {
			if ((minor_dev < 1) || (minor_dev >= enet_n_endpoints))
				minor_dev = 1;
			if(enet_endpoints[minor_dev].str_state == C_IDLE) {
				last = minor_dev;
				break;
				}
			}
		/*printf("enetopen: i=%x minor_dev = %x\n", i, last);*/
		if(i == enet_n_endpoints) {
		 	return(ENOSPC);
		}
		/*
		 *	Make new device in *rdev.
		 */
		*rdev = makedevice(getmajor(*rdev), minor_dev);
	}
	else if((sflag != 0) || (minor_dev < 0) || (minor_dev >= enet_n_endpoints)) {
		/*
		 * Some illegal minor minor_device number:
		 * Perhaps an out-of-range minor device was opened.
		 */
		DEBUGP(DEB_ERROR,(CE_CONT, "enetopen ECHRNG: sflag = %x, minor_dev = %x\n",
				 sflag, minor_dev));
		return(ECHRNG);
	}
	ep = &enet_endpoints[minor_dev];
	DEBUGP(DEB_FULL,(CE_CONT, "enetopen: ep = %x, minor_dev = %x\n", ep, minor_dev));
	/*
	 * Detect multiple opens on the same STREAM.
	 */
	if(ep->str_state & C_OPEN) {
		/*
		 * The STREAM is already completely opened.
		 */
		DEBUGP(DEB_FULL,(CE_CONT, "enetopen: stream already opened\n"));
		DEBUGP(DEB_CALL,(CE_CONT, "enetopen => %d\n", minor_dev));
		if(ep->options & OPT_EXCL) {
			DEBUGP(DEB_ERROR,(CE_CONT, "enetopen: Re_open of exclusive dev\n"));
			DEBUGP(DEB_CALL,(CE_CONT, "enetopen => EACCES\n"));
			return(EACCES);
		}
		return(0);
	}
	/*
	 * User wants a specific endpoint (or the one just found for him)
	 * which is currently idle.
	 * Clear the entire endpoint data structure.
	 */
	bzero((char *)ep, sizeof(endpoint));
	/*
	 * Set the streamhead's configuration options for the
	 * READ queue to be consistent with those used by enet.
	 */
	if((mptr = allocb(sizeof(struct stroptions), BPRI_MED)) == NULL) {
		enet_stat[ST_ALFA]++;
		return(ENOSR);
/*		if(flag & FNDELAY)				*/
/*			 don't wait for the buffer to free up	*/
	}
	mptr->b_datap->db_type = M_SETOPTS;
	soptr = (struct stroptions *) mptr->b_rptr;
	soptr->so_flags   = SO_MINPSZ | SO_MAXPSZ | SO_HIWAT | SO_LOWAT;
	soptr->so_readopt = 0;
	soptr->so_wroff   = 0;
	soptr->so_minpsz  = enet_STRMIN;
	soptr->so_maxpsz  = enet_STRMAX;
	soptr->so_hiwat   = enet_sh_hiwat;
	soptr->so_lowat   = enet_sh_lowat;
	mptr->b_wptr = mptr->b_rptr + sizeof(struct stroptions);
	putnext(rd_q, mptr);
	ep->tli_state = TS_UNBND;
	ep->rd_q = rd_q;
	DEBUGP(DEB_FULL,(CE_CONT, "enetopen: ep = %x ep->rd_q = %x\n", ep, ep->rd_q));
	rd_q->q_ptr = (WR(rd_q))->q_ptr = (caddr_t)ep;
	ep->str_state = C_OPEN;
	ep->open_cont = NULL;
	ep->bnum = enet_majtobnum[getmajor(*rdev)];
	ep->req_seqno = 0;
	ep->complete_seqno = 0;
	ep->options = enet_u_defaults;
	if(oflag & FEXCL)
		ep->options |= OPT_EXCL;
	enet_stat[ST_CURO]++;
	enet_stat[ST_TOTO]++;
	DEBUGP(DEB_FULL,(CE_CONT, "Ethernet Driver: Opened minor device #%x\n", minor_dev));
	DEBUGP(DEB_CALL,(CE_CONT, "enetopen => %d\n", minor_dev));
	return(0);
}

/*
 * enetclose
 *
 * Called whenever a user closes the device.
 *
 * Should close the virtual circuit (if any) and unbind the endpoint
 * 
 */
int
enetclose(rd_q)
queue_t *rd_q;
{
	register endpoint *ep;
	int	x;

	DEBUGP(DEB_CALL,(CE_CONT, "enetclose()\n"));
	DEBUGC('U');
	ep = (endpoint *)rd_q->q_ptr;

	while (  (ep->req_seqno > ep->complete_seqno)	/* begin I008 */
	       ||(ep->ex_req_seqno > ep->ex_complete_seqno)
	      )
	{
		ep->str_state |= C_DISCON;
		sleep((caddr_t)ep, STOPRI|PCATCH);

/* I009 I0010 */
#if defined(V30)
		if (issig())
#else
		if (issig(JUSTLOOKING))
#endif
			break;
	}						/* end I008 */

	/*
	 * Take down any connections, using abort
	 */
	x = SPL();
	enet_abort(ep);
	/* ep->open_cont = enet_closed_open_complete; */	/* JB */
	splx(x);
	rd_q->q_ptr = (WR(rd_q))->q_ptr = (caddr_t)NULL;
#ifdef JB
	/* ep->tli_state = 0; */	/* JB */
	/* ep->str_state = C_IDLE; */	/* JB */
#else
	ep->tli_state = 0;
	ep->str_state = C_IDLE;
#endif
	enet_stat[ST_CURO]--;
	DEBUGP(DEB_CALL,(CE_CONT, "enetclose => NULL\n"));
	return(0);
}

/*
 * enetreset
 *
 * May be called at any time after initialization.  Will reset the specified
 * board.  Usually followed by writes to the administrator device (minor 0)
 * with object code to be loaded onto the board.
 */
void
enetreset(bnum)
int	bnum;
{
	struct enetboard *board_p;

	DEBUGP(3,(CE_CONT, "enetreset: bnum %d\n", bnum));
	board_p = &enet_boards[bnum];
	iNA961_reset(bnum);
	board_p->state |= RESET;
	board_p->state &= ~BOOTED;
}
