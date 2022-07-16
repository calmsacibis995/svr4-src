/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988, 1989, 1990 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/cci.c	1.3.2.1"

#ifndef lint
static char cci_copyright[] = "Copyright 1986, 1987, 1988, 1989, 1990 Intel Corp. 460948";
#endif /* lint */

/************************************************************************
 *
 *	Name:
 *		Driver for CCI protocol control points
 *
 *	Purpose:
 *
 *		This is the set of routines that constitute the device
 *		for the iSBC 410 Intelligent Serial Interface Controller.
 *
 *	Comments:
 *		This cci driver is just a small part of what it will someday
 *		become.  Eventually an application should be able to open
 *		a cci line and request status as well as do downloads (an
 *		as yet to be designed combination of reads, writes, and ioctls)
 *		and assignments of lines to protocol routines.
 *
 * 		Debug messages are controlled by DEBUGCCI for CCI support.
 *
 *************************************************************************/
#include "sys/param.h"
#include "sys/types.h"
#include "sys/cmn_err.h"
#include "sys/termio.h"
#include "sys/stream.h"
#include "sys/strtty.h"
#include "sys/mps.h"
#include "sys/cci.h"
#include "sys/ccimp.h"
#include "sys/ddi.h"

/* #define DEBUGCCI */
#ifdef DEBUGCCI
#define DDEBUG(x,y) if(ccidebug&(x)) cmn_err y
#define DINIT 1		/* places during initialization */
#define DCALL 2		/* when called -- open, close, ioctl, ... */
#define DINTR 4		/* when interrupted */
#define DIOCTL 8	/* ioctl operations */
#define DMSG 16		/* display transport messages going in and out */
int	ccidebug = DINIT; /* debug output control */
#define DDEBUGMSG(x,y) if(ccidebug&(x))cciMsgPrint y
#else
#define DDEBUG(x,y)
#define DDEBUGMSG(x,y)
#endif

extern void wakeup();

/* one structure for each cci server */
struct cci_server cci_server[MAXCCISERVERS];

ulong cci_bitmap = 0;
int cciintr();

int ccidevflag = 0;			/* V4.0 style driver */

/*
 *  cciinit() -- initialize the lines before interrupts go on
 */
cciinit()
{	static int done = 0;
	int i;

	if (done)
		return;
	DDEBUG(DINIT, (CE_CONT, "cciinit: MAXCCISERVERS = %d\n", MAXCCISERVERS));
	/* not much to do with the interrupts off -- just initialize protocols */
	for (i=0; i<MAXCCISERVERS; ++i)
		cci_server[i].c_chan = (uint)-1;
	done++;
#ifdef lint
	cciinit();	/* Keep lint happy */
	ccistart();
#endif
}

/*
 *	This is new code to implement the broadcast locate protocol.
 *	This should ideally be modified to use the location service when
 *	it gets implemented.
*/
long cciBroadcastChannel = 0;

void
ccibroadcast(delay)
int delay;
{	register struct msgbuf *mbp;
	static unchar mssg[20] = { 0x90 };

	if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)0))
		cmn_err (CE_PANIC, "ccibroadcast: Cannot get message buffers");
	mps_mk_brdcst(mbp, CCI_PORT_ID, mssg, sizeof(mssg));
	DDEBUGMSG(DMSG, ("ccibroadcast:", mbp));
	if (mps_AMPsend(cciBroadcastChannel, mbp) == -1) {
		/* this is a panic situation -- error occurs when params are wrong */
		cmn_err(CE_PANIC, "cci: Broadcast failure. mbp=%x\n", mbp);
	}
	if (delay > 10*HZ)
		(void) timeout(ccibroadcast, (caddr_t)delay, delay);
	else
		(void) timeout(ccibroadcast, (caddr_t)(delay*2), delay);
}

int
cciServerMsg(tmsg)
struct msgbuf *tmsg;
{	unsigned char *rp;

	DDEBUGMSG(DMSG, ("cciServerMsg:", tmsg));
	rp = (unsigned char *)mps_msg_getudp(tmsg);
	if (rp[0] != 0x90) {
		cmn_err(CE_WARN,
				"cciServerMsg: Bogus broadcast response. Smid=%x,Spid=%x\n",
				mps_msg_getsrcmid(tmsg),
				mps_msg_getsrcpid(tmsg));
		mps_msg_showmsg(tmsg);
		mps_free_msgbuf(tmsg);
		return;
	}
	if (rp[1] != 0x00) {
		cmn_err(CE_WARN,
				"cciServerMsg: Dead cci server.  Smid=%x, Spid=%x\n",
				mps_msg_getsrcmid(tmsg),
				mps_msg_getsrcpid(tmsg));
		mps_msg_showmsg(tmsg);
		mps_free_msgbuf(tmsg);
		return;
	}
	if (!(cci_bitmap & (1 << mps_msg_getsrcmid(tmsg))))
		cci_bitmap |= 1 << mps_msg_getsrcmid(tmsg);
	mps_free_msgbuf(tmsg);
}

/*
 *	ccistart() - Broadcast locate requests for a while
 *	Servers will respond and get registered by cciintr().
 *	This is set up for broadcasts at intervals of 0.8, 1.6, 3.2, 6.4 seconds.
 *	(That is, at 0.0, 0.8, 2.4, 5.6, and 12.0 seconds.)
 *	After that, broadcasts are done every 12.8 seconds forever.
 */
ccistart()
{
	if (!cciBroadcastChannel)
		cciBroadcastChannel=mps_open_chan(CCI_BRDCST_PORT, cciServerMsg, MPS_SRLPRIO);
	if (cciBroadcastChannel == -1) {
		cmn_err(CE_PANIC, "ccistart: cannot open broadcast channel\n");
	}
	ccibroadcast(8*HZ/10);
}
/* End of broadcast locate code */

/*
 *	Close_chan sometimes fails because the channel is busy.  Let's wait for
 *	it here.
*/
void
cci_close_chan(chan)
long chan;
{	int i;
	for (i = 0; i < 10; i++) {
		if (mps_close_chan(chan) != -1)
			return;
	}
	cmn_err(CE_PANIC, "cci: mps_close_chan failed\n");
}

/*
 *  ccistartsock() -- initialize the line now that the interrupts are on
 *	ccistartsock will return 0 if no error occurs.
 *	ccistartsock will return -1 if the send_rsvp fails.
 */
int
ccistartsock(ccisocket, disc)
mb2socid_t ccisocket;	/* the connection to my cci */
int disc;		/* The line discipline needed */
{
	int i; 
	minor_t minor;
	struct cci_server *cp;
	struct CCIGetServerInfoReq gmsg;
	struct CCIGetServerInfoResp *gmsgr;
	struct CCICreateReq cmsg;
	struct CCICreateResp *cmsgr;

	DDEBUG(DINIT, (CE_CONT, "ccistartsock: socket=%x\n", ccisocket));

	cp = &cci_server[mps_mk_mb2soctohid(ccisocket)%MAXCCISERVERS];
	if (cp->c_chan != (uint)-1)
		cmn_err(CE_PANIC, "ccistartsock: cci channel reallocated\n");
	cp->c_index = cp - &cci_server[0];
	cp->c_MySocket = mps_mk_mb2socid(ics_myslotid(), 0x2260-MAXCCISERVERS+cp->c_index);
	cp->c_HisSocket = ccisocket;
	cp->c_chan = mps_open_chan(mps_mk_mb2soctopid(cp->c_MySocket), cciintr, MPS_SRLPRIO);
	if (cp->c_chan == (uint)-1)
		cmn_err(CE_PANIC, "ccistartsock: cannot open message channel\n");
	DDEBUG(DINIT, (CE_CONT, "ccistartsock: i=%d, chan=%xH, Mysoc=%xH, Hissoc=%xH\n",
		cp->c_index, cp->c_chan, cp->c_MySocket, cp->c_HisSocket));

	/* get the number of lines this CCI control point has */
	gmsg.Type = CCIGetServerInfoC;
	if ( cciSendCommand(cp, A_SYNC, &gmsg, sizeof(gmsg)) == -1) {
		cci_close_chan(cp->c_chan);
		cp->c_chan = -1;
		return(-1);
	} 
	gmsgr = (struct CCIGetServerInfoResp *)&cp->c_lmsg[0];
	if (gmsgr->Status == CCIFailure) {
		cci_close_chan(cp->c_chan);
		cp->c_chan = -1;
		cmn_err(CE_WARN,
			"ccistartsock: GetInfo failure. CCI not initialized. Soc=%x\n",
			cp->c_HisSocket);
		return(-1);
	}
	cp->c_lines = gmsgr->NumLines;

	/* "create" script 1 */
	cmsg.Type = CCICreateC;
	cmsg.LineDiscID = disc;
	if ( cciSendCommand(cp, A_SYNC, &cmsg, sizeof(cmsg)) == -1) {
		cci_close_chan(cp->c_chan);
		cp->c_chan = -1;
		return(-1);
	}
	cmsgr = (struct CCICreateResp *)&cp->c_lmsg[0];
	if (cmsgr->Status == CCIFailure) {
		if (cciCleanup(cp, disc) == -1) {
			cci_close_chan(cp->c_chan);
			cp->c_chan = -1;
			cmn_err(CE_WARN, "ccistartsock: Unable to create discipline %d on socket 0x%x\n", disc, ccisocket);
			return(-1);
		}
	}
	return(0);
}


/* 
 * This routine is called when we think that we may have been rebooted.
 * We do detach, unbind for each lines and then free the script
 * This should make the cci line free of previous commitments for our
 * CPU.  Then we create the script again. If it fails, we know something
 * is wrong.
 */
cciCleanup(cp, disc)
struct cci_server *cp;	/* Server that seems to have old state */
int disc;		/* Line discipline desired by client */
{
	int i;
	struct CCICreateReq cmsg;
	struct CCICreateResp *cmsgr;
	struct CCIFreeReq fmsg;
	struct CCIUnbindReq umsg;
	struct CCIDetachReq dmsg;

	DDEBUG(DINIT, (CE_CONT, "cciCleanup: cci_server ptr =%x\n", cp));
	dmsg.Type = CCIDetachC;
	dmsg.Subchannel = 0;
	dmsg.SessionStatus[0] = 0;
	dmsg.SessionStatus[1] = 0;
	dmsg.SessionStatus[2] = 0;
	dmsg.SessionStatus[3] = 0;
	umsg.Type = CCIUnbindC;
	/* OK to undo all lines -- NO-OP if we never init'ed them. */
	for (i=0; i < cp->c_lines; i++) {
		DDEBUG(DINIT, (CE_CONT, "detach, unbind line # %d\n", i));
		dmsg.LineNumber = i;
		umsg.LineNumber = i;
		if (cciSendCommand(cp, A_SYNC, &dmsg, sizeof(dmsg)) == -1) {
			return(-1); 
		} 
		if (cciSendCommand(cp, A_SYNC, &umsg, sizeof(umsg)) == -1) {
			return(-1); 
		} 
	}
	fmsg.Type = CCIFreeC;
	fmsg.LineDiscID = disc;
	DDEBUG(DINIT, (CE_CONT, "Free the script of cci_server ptr =%x\n", cp));
	if ( cciSendCommand(cp, A_SYNC, &fmsg, sizeof(fmsg)) == -1) {
		return(-1);
	} 
	cmsg.Type = CCICreateC;
	cmsg.LineDiscID = disc;
	DDEBUG(DINIT, (CE_CONT, "Create (2) the script of cci_server ptr =%x\n", cp));
	if ( cciSendCommand(cp, A_SYNC, &cmsg, sizeof(cmsg)) == -1) {
		return(-1);
	} 
	cmsgr = (struct CCICreateResp *)&cp->c_lmsg[0];
	if (cmsgr->Status == CCIFailure) {	/* now something is wrong */
		return(-1);
	}
	return(0);
}


/*
 *	Routine to bind a particular line discipline to a hardware unit.
*/
int
cciBindLine(slot, line, disc)
int	slot;		/* Slot where CCI server resides */
int	line;		/* Line where disc should be bound */
int disc;		/* Line discipline to use on (slot, line) */
{
	struct cci_server *cp;
	struct CCIBindReq bmsg;
	struct CCIBindResp *bmsgr;

	bmsg.Type = CCIBindC;
	bmsg.LineDiscID = disc;
	bmsg.LineNumber = line;
	cp = &cci_server[slot];
	if (cciSendCommand(cp, A_SYNC, &bmsg, sizeof(bmsg)) == -1) {
		cmn_err(CE_WARN,
			"ccistartsock: Bind failure. Line not initialized. Soc=%x, ln=%d\n",
			cp->c_HisSocket, line);
		return(-1);
	}
	bmsgr = (struct CCIBindResp *)&cp->c_lmsg[0];
	if (bmsgr->Status == CCIFailure) {
		cmn_err(CE_WARN,
			"ccistartsock: Bind failure. Line not initialized. Soc=%x, ln=%d\n",
			cp->c_HisSocket, line);
		return(-1);
	}
	return(0);
}


int
cciintr(tmsg)
struct msgbuf *tmsg;
{
	register struct cci_server 	*cp;

	DDEBUGMSG(DMSG, ("cciintr:", tmsg));
	if (/* !(mps_msg_iscompletion(tmsg)) ||*/ (tmsg->mb_bind == -1)) {
		/* a message that isn't for us, wonder where it came from */
		cmn_err(CE_WARN, "cciintr: unknown msg. Smid=%x, Spid=%x\n",
			mps_msg_getsrcmid(tmsg), mps_msg_getsrcpid(tmsg));
		return;
	}

	cp = &cci_server[BIND_CP(tmsg)];

	if (mps_msg_iserror(tmsg)) {
		cmn_err(CE_WARN, "cciintr: msg error.  Smid=%x, Spid=%x\n",
			mps_msg_getsrcmid(tmsg), mps_msg_getsrcpid(tmsg));
	}
	if (mps_msg_iscancel(tmsg)) {
		cmn_err(CE_WARN, "cciintr: msg cancel.  Smid=%x, Spid=%x\n",
			mps_msg_getsrcmid(tmsg), mps_msg_getsrcpid(tmsg));
	}

	switch (BIND_ACTION(tmsg)) {
		case A_SYNC: {
			char *rp, *wp;
			int i;
			rp = (char *)mps_msg_getudp(tmsg);
			wp = &cp->c_lmsg[0];
			for (i=0; i<18; ++i, ++wp, ++rp)
				*wp = *rp;
			wakeup((caddr_t)&cp->c_chan);
			break;
		}
		case A_ASYNC:
			break;
		default:
			cmn_err(CE_PANIC, "cciintr: invalid bind action\n");
	}
	if (mps_msg_gettrnsid(tmsg))
		(void) mps_free_tid(cp->c_chan, mps_msg_gettrnsid(tmsg));
	mps_free_msgbuf(tmsg);
}

/* cciSendCommand -- send an CCIs command
 *   The built message is placed into a message control block and
 *   sent out.  If the action is A_SYNC this driver will sleep until
 *   all of the synchronious requests are done for this line.  Other
 *   actions are legal to be called from interrupt routines.
 */
int
cciSendCommand(cp, action, mssg, mssgl)
	struct cci_server *cp;
	int action;		/* A_xxxx -- action to take when complete */
	char *mssg;		/* message to send */
	int mssgl;		/* number of bytes in mssg */
{
	register struct msgbuf *mbp;
	unsigned char	our_tid; 

	if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)0))
		cmn_err (CE_PANIC, "ccisendcomm: Cannot get message buffers");
	if ((our_tid = mps_get_tid(cp->c_chan)) == (unchar)0)
		cmn_err (CE_PANIC, "cciSendCommand: Cannot get transaction id");
	mps_mk_unsol(mbp, cp->c_HisSocket, our_tid, (unsigned char *)mssg, mssgl);
	BIND_ACTION(mbp) = action;
	BIND_CP(mbp) = cp->c_index;
	DDEBUGMSG(DMSG, ("cci scmd:", mbp));
	if (mps_AMPsend_rsvp(cp->c_chan, mbp, (struct dma_buf *)0, (struct dma_buf *)0) == -1) {
		cmn_err(CE_WARN, "cci: mps_AMPsend_rsvp failure. Dsoc=%x, mbp=%x\n",
						cp->c_HisSocket, mbp);	/* Load ATCS/CCI next time! */
		cmn_err(CE_CONT,"You sent a 0x%x request\n",
						((struct CCICreateReq *)mssg)->Type);
		return(-1);
	} else {
		if (action == A_SYNC)
			(void) sleep((caddr_t)&cp->c_chan, TTIPRI);
	}
	return(0);
}

#ifdef DEBUGCCI
/* print the MB2 transport message */
cciMsgPrint(hdr, mbp)
	char *hdr;			/* text to print as header */
	struct msgbuf *mbp;	/* message to display */
{
	cmn_err (CE_CONT,hdr);
	mps_msg_showmsg(mbp);
}
#endif
