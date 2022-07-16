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

#ident	"@(#)mbus:uts/i386/io/atcs.c	1.3.2.2"

#ifndef lint
static char atcs_copyright[] = "Copyright 1986, 1987, 1988, 1989, 1990 Intel Corp. 460952";
#endif /*lint*/

/*
 *	Name:
 *		Async Terminal Controller Script driver for UNIX V.3 on `386
 *
 *	Purpose:
 *		This is the device dependant code for the ATCS driver.
 *
 *	Comments:
 *		Set your tab stops to 4.  (For vi, ":set ts=4 sw=4").
 *		In some places operations are broken out as separate functions
 *		  and the compiler is smart enough to put a function that is
 *		  only called once inline.
 *      The minor number indexes the array atcs_lines to get the controlling
 *		  line structure.  Within the line structure is a pointer to the tty
 *		  structure.
 *
 *	05/01/89	vrs
 *		A T_CONNECT case was added to the proc routine.  This is supposed
 *		  to listen for a connection and arrange that CARR_ON will get set
 *		  when a connection appears.
 *	05/02/89	vrs
 *		The proc routine (and hence the param routine) now returns an error
 *		  code, and the caller is expected to figure out how to notify the
 *		  user.  Previously they assumed they had user context and directly
 *		  set u.u_error.
 *  07/16/89  	am
 *		The range of minor numbers that this driver supports are from 0 - 127.
 *	    If more minor nos are required then the BIND_MINOR and the MINORMSK
 *		macros in atcs.h need to be altered accordingly.
 *
*/
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/signal.h"
#include "sys/param.h"
#include "sys/conf.h"
#include "sys/termio.h"
#include "sys/stream.h"
#include "sys/strtty.h"
#include "sys/ics.h"
#include "sys/mps.h"
#include "sys/ccimp.h"
#include "sys/atcs.h"
#include "sys/atcsmp.h"
#include "sys/ics.h"
#include "sys/iasy.h"
#include "sys/bps.h"
#include "sys/ddi.h"

extern void bcopy();
extern void signal();
extern void splx();
extern void wakeup();

/* the one and only script that's in the early 410 */
#define LINEDISC 1

extern struct strtty *iasy_register();
extern void iasy_hup();
extern void iasy_input();
extern int iasy_output();

extern struct conssw conssw;	/* console switch */
extern ulong cci_bitmap;

#ifdef DEBUGATCS
#define DINIT 1		/* during initialization */
#define DCALL 2		/* when called (startio, open, close, ...) */
#define DINTR 4		/* interrupts */
#define DIOCTL 8	/* ioctl operations */
#define DMSG 16		/* display messages on console */
#define DKEEP 32	/* keep messages in circular queue */
#define DDMON 64	/* trap to debugging monitor */
#define DWILF 128	/* "what I'm looking for" Your own short term debugging */

/* static int	atcsdebug = DINIT+DCALL+DINTR+DIOCTL+DMSG+DKEEP+DWILF; */
static int	atcsdebug = DWILF; 
static int atcsi = 0;
#define ATCSIMAX 64
static struct msgbuf atcskeep[ATCSIMAX];
char *ActionName[] = {
	"A_ODD", "A_ASYNC", "A_SYNC", "A_SPCHAR", "A_CD",
	"A_CLEARBUSY", "A_OUTFLOW", "A_OUTBUFC", "A_INCHARS", "A_CLOSE"};

/* Conditionally print information on the console */
#define DDEBUG(x,y) if(atcsdebug&(x))	cmn_err  y
/* conditionally print message packet on the console */
#define DDEBUGMSG(x,y) if(atcsdebug&(x))atcsMsgPrint y
/* keep message packets in a circular buffer */
#define DMSGKEEP(x) if(atcsdebug&DKEEP) \
		atcskeep[atcsi=(atcsi+1)%ATCSIMAX]=(*(x))
#define DMONITOR() if(atcsdebug&DDMON)monitor()
#else
#define DDEBUG(x,y)
#define DDEBUGMSG(x,y)
#define DMSGKEEP(x)
#define DMONITOR()
#endif

/* Routine for putting one character in the input c-list */
#define PutInputChar(tp, c) if ((tp)->t_in.bu_ptr != NULL) { \
		*(tp)->t_in.bu_ptr = (c); \
		(tp)->t_in.bu_cnt--; \
		iasy_input((tp), L_BUF); \
	}
/* define each bit in atcsHaveSetupLines to represent each board.
 * and redefine atcsSetupDone for each board.
 */
#define MAX_BOARD 	32		/* size of ulong = 32 bits */
#define SETUP_INIT 	0x00
#define SETUP_OK 	0x01
#define SETUP_FAIL 	0x0F
static ulong atcsHaveSetupLines = 0;
static char atcsSetupDone[MAX_BOARD];
extern int atcsMin2CCILine[];
static int atcsAlreadyInit = 0;
extern struct atcs_lines atcs_line[];
struct strtty *atcs_tty;
extern int atcsbaud[];
extern int atcsMaxLines;
extern int atcsMaxIChars;
extern int atcsMaxOChars;
extern ushort mps_lhid();	/* (remove when defn in mb2tran.h) */
extern struct atcs_info atcs_info[];

/*
 *	This is here to resolve an extern in rci.c.  Since it is not
 *	configured, it should never get called.
*/
atcsopen()
{	cmn_err(CE_PANIC, "atcsopen() called\n");
}


/*
 *	Conversion from line (minor number) to [slot,unit] pair and vice-versa.
*/
unsigned short
atcs_line_to_slot(line)
int line;
{
	struct atcs_info *p;

	p = atcs_info;
	while (p->slot != ICS_MAX_SLOT) {
		if ((line >= p->beg_minor_num) && (line <= p->end_minor_num))
			return (p->slot);
		p++;
	}
	return (-1);     /* we get here only if the minor number was not found */
}

int
atcs_line_to_unit(line)
int line;
{
	struct atcs_info *p;

	p = atcs_info;
	while (p->slot != ICS_MAX_SLOT) {
		if ((line >= p->beg_minor_num) && (line <= p->end_minor_num))
			return (p->beg_port_num + (line-p->beg_minor_num));
		p++;
	}
	return (-1);     /* we get here only if the minor number was not found */
}


int
atcs_slot_to_line(slot)
unsigned short slot;
{
	struct atcs_info *p;

	p = atcs_info;
	while (p->slot != ICS_MAX_SLOT) {
		if (p->slot == slot) {
			return (p->beg_minor_num);
		}
		p++;
	}
	return (-1);     /* we get here only if the minor number was not found */
}


int
atcsSetupLine(line)
int line;
{	int brd, unit;
	register struct atcs_lines *ln;

	brd = atcs_line_to_slot(line);
	unit = atcs_line_to_unit(line);
	if (!(cci_bitmap & (1<<brd)))  /* existing device? */
		return(ENXIO);
	if (!(atcsHaveSetupLines & (1<<brd))) {
		/* set up the particular board once when the system comes up */
		/* we just happen to know the cci port */
		if (ccistartsock(mps_mk_mb2socid(brd, CCI_PORT_ID), LINEDISC) == -1) {
			atcsSetupDone[brd] = SETUP_FAIL;
		} else {
			atcsSetupDone[brd] = SETUP_OK;
			atcsHaveSetupLines |= (1<<brd);
		}
		wakeup((caddr_t)&atcsSetupDone[brd]);
	}
	while (atcsSetupDone[brd] != SETUP_OK)  {
		if (atcsSetupDone[brd] == SETUP_FAIL)
			return(ENXIO);
		if (sleep((caddr_t)&atcsSetupDone[brd], TTIPRI|PCATCH))
			return(ENXIO);
	}
	/*
	 *	Now do per-line setup
	*/
	ln = &atcs_line[line];
	if (ln->l_chan == (uint)-1) {
		if (cciBindLine(brd, unit, LINEDISC) != 0)
			return(ENXIO);
		atcsstart1(line, mps_mk_mb2socid(brd, CCI_PORT_ID), unit);
	}
	return(0);
}

void
atcsunbreak(tp)
struct strtty *tp;
{	register struct atcs_lines *ln;
	struct ATCSSetModemSignalReq fmsg;
	int s;

	s = SPL();
	ln = &atcs_line[tp-atcs_tty];
	fmsg.Type = ATCSSetModemSignalC;
	fmsg.RTSCommand = ATCSSMSNoChange;
	fmsg.DTRCommand = ATCSSMSNoChange;
	fmsg.BreakCommand = ATCSSMSClear;
	atcsSendCommand(ln, A_ASYNC, &fmsg, sizeof(fmsg));
	tp->t_state &= ~TTSTOP;
	atcsproc(tp, T_OUTPUT);
	splx(s);
}

/*
 * atcsproc() -- driver dependent protocal operations
 *
 * This function is the common interface for line discipline
 * routines to call the device driver. It performs device
 * specific I/O functions.
 */
int
atcsproc(tp, comd)
register struct strtty *tp;
int comd;
{	int s, unit;
	register struct atcs_lines *ln;
	int ret = 0;

	s = SPL();
	unit = tp - atcs_tty;
	ln = &atcs_line[unit];

	if ((comd != T_CONNECT) && !(ln->l_cci_state & ATTACHED)) {
		splx(s);
		return(0);	/* we don't own the controller */
	}

	DDEBUG(DCALL, (CE_CONT, "atcsproc: tp=%x, ln=%x, c=%x\n",
		tp, ln, comd) );
	switch (comd) {
	/* flush output queue */
	case T_WFLUSH: {
		struct ATCSOutputControlReq fmsg;
		tp->t_out.bu_cnt = 0;	/* Abort this buffer */
		fmsg.Type = ATCSOutputControlC;
		fmsg.Command = ATCSOCFlushOutput;
		atcsSendCommand(ln, A_ASYNC, &fmsg, sizeof(fmsg));
		/*** FALL THROUGH ***/
	}
	/* resume output */
	case T_RESUME: {
		struct ATCSOutputControlReq fmsg;
		fmsg.Type = ATCSOutputControlC;
		fmsg.Command = ATCSOCResumeOutput;
		atcsSendCommand(ln, A_ASYNC, &fmsg, sizeof(fmsg));
		tp->t_state &= ~TTSTOP;	/* Don't clear busy */
		atcsstartio(tp, ln);
		break;
	}
	/* suspend output */
	case T_SUSPEND: {
		struct ATCSOutputControlReq fmsg;
		fmsg.Type = ATCSOutputControlC;
		fmsg.Command = ATCSOCSuspendOutput;
		atcsSendCommand(ln, A_ASYNC, &fmsg, sizeof(fmsg));
		tp->t_state |= TTSTOP;
		break;
	}
	case T_BLOCK: {
		struct ATCSTandemControlReq fmsg;
		fmsg.Type = ATCSTandemControlC;
		fmsg.Command = ATCSTMCSuspendInput;
		atcsSendCommand(ln, A_ASYNC, &fmsg, sizeof(fmsg));
		tp->t_state |= TBLOCK;
		atcsstartio(tp, ln);
		break;
	}
	/* flush input queue */
	case T_RFLUSH: {
		struct ATCSFlushInputReq fmsg;
		fmsg.Type = ATCSFlushInputC;
		atcsSendCommand(ln, A_ASYNC, &fmsg, sizeof(fmsg));
		if (!(tp->t_state & TBLOCK))
			break;
		/*** FALL THROUGH ***/
	}
	/* send start character */
	case T_UNBLOCK: {
		struct ATCSTandemControlReq fmsg;
		if (tp->t_iflag & IXOFF) {
			fmsg.Type = ATCSTandemControlC;
			fmsg.Command = ATCSTMCResumeInput;
			atcsSendCommand(ln, A_ASYNC, &fmsg, sizeof(fmsg));
		}
		tp->t_state &= ~TBLOCK;
		atcsstartio(tp, ln);
		break;
	}
	/* remove timeout condition */
	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		atcsstartio(tp, ln);
		break;
	/* output break condition on line */
	case T_BREAK: {
		struct ATCSSetModemSignalReq fmsg;
		tp->t_state |= TTSTOP;
		fmsg.Type = ATCSSetModemSignalC;
		fmsg.RTSCommand = ATCSSMSNoChange;
		fmsg.DTRCommand = ATCSSMSNoChange;
		fmsg.BreakCommand = ATCSSMSSet;
		atcsSendCommand(ln, A_ASYNC, &fmsg, sizeof(fmsg));
		(void) timeout(atcsunbreak, (caddr_t)tp, HZ/4);
		break;
	}
	/* start output */
	case T_OUTPUT:
		atcsstartio(tp, ln);
		break;
	case T_CONNECT:	/* connect to the server */
#ifdef lint
		atcsinit();	/* Dummy reference to keep lint happy */
		atcsopen();	/* Dummy reference to keep lint happy */
#endif
		if (atcsSetupLine(tp - atcs_tty) != 0) {
			ret = ENXIO;
			break;
		}
		/* If still detaching, wait for that to finish */
		while (ln->l_cci_state & DETACH_SENT) {
			(void) timeout(wakeup, (caddr_t)&ln->l_reserved1, HZ/4);
			if (sleep((caddr_t)&ln->l_reserved1, TTOPRI|PCATCH)) {
				ret = ENXIO;
				break;
			}
		}
		if (ret != 0)
			break;
		ln->l_sess_stat = 0;	/* Haven't lost carrier yet */
		if ((ret = atcsDoAttach(ln)) != 0)	{/* Send CCI attach */
			break;
		}
		atcsSendSetDTR(ln, A_ASYNC);/* Listen for a hardware connection */
		atcsstartio(tp, ln);	/* Post atcsWaitForCarrier request */
		atcsSetCarr(tp, ln);		/* Might already be a hardware connection */
		break;
	case T_DISCONNECT:	/* disconnect from the server */
		atcsSendClearDTR(ln, A_ASYNC);
		if (ln->l_cci_state & ATTACH_SENT)
			atcsDoFlush(ln); /* Send CCI Detach */
		ln->l_state &= ~CARRF;
		if ((tp->t_state&CARR_ON) && !(tp->t_cflag&CLOCAL))
			iasy_hup(tp);
		atcsSetCarr(tp, ln);
		break;
	case T_PARM:	/* force the setting of the terminal parameters */
		ret = atcsparam(tp, ln);
		break;
	case T_SWTCH:
		break;
	default:
		break;
	}
	splx(s);
	return(ret);
}

/*
 * This code is protected by proper spl by the caller.
 *	Attach to a CCI line
*/
int
atcsDoAttach(ln)
register struct atcs_lines *ln;
{	struct CCIAttachReq amsg;
	struct CCIAttachResp *amsgr;

	if (!(ln->l_cci_state & ATTACH_SENT)) {
		ln->l_cci_state |= ATTACH_SENT;
		DDEBUG(DCALL,(CE_CONT, "atcsDoAttach: attach called\n"));
		amsg.Type = CCIAttachC;
		amsg.LineNumber = atcsMin2CCILine[ln->l_minor];
		amsg.Subchannel = 0;
		amsg.DefaultPortID = mps_mk_mb2soctopid(ln->l_MySocket);
		if (atcsSendCCICommand(ln, A_SYNC, &amsg, sizeof(amsg)) != 0) {
			ln->l_cci_state &= ~(ATTACH_SENT|ATTACHED|SWITCHED);
			ln->l_cci_state |= DETACH_SENT;
			ln->l_cci_state |= ATTACH_CANCEL;
			atcsDoDetach(ln);
			return(EINTR);
		}
		amsgr = (struct CCIAttachResp *)&ln->l_lmsg[0];

		if ((amsgr->Status == CCISuccess) || (amsgr->Status == CCISwitched)) {

			ln->l_tbufsiz = amsgr->ScriptInfo[0]*256;
			ln->l_tbufavail = ln->l_tbufsiz;
			ln->l_MQSize = amsgr->ScriptInfo[1];
			ln->l_rbufsiz = amsgr->ScriptInfo[2]*256;
			ln->l_ucodeVer = amsgr->ScriptInfo[3];
			ln->l_HisSocket = mps_mk_mb2socid(mps_mk_mb2soctohid(ln->l_CCISocket), amsgr->PortID);
			DDEBUG(DCALL,(CE_CONT, "atcsDoAttach: attach success, &l=%xH, m=%d, ms=%xH, hs=%xH\n", ln, &atcs_line[0]-ln, ln->l_MySocket, ln->l_HisSocket));
		} else {
			DDEBUG(DCALL,(CE_CONT, "atcsDoAttach: attach failed, &l=%xH, m=%d, ms=%xH, hs=%xH\n", ln, &atcs_line[0]-ln, ln->l_MySocket, ln->l_HisSocket));
			/* report error back to upper level routine */
			if (mps_close_chan(ln->l_chan) != 0)
				cmn_err(CE_PANIC, "atcs: cannot mps_close_chan\n");
			ln->l_chan = -1;
			return(ENXIO);
		}
		ln->l_cci_state |= ATTACHED;
	} 
	return(0);
}

/*
 * This code is protected by proper spl by the caller.
 *	The message queue on the controller needs to be flushed
 *	before detaching the line. The Waitfor Carrier Sense
 *	command is sent to flush the controller message queue
*/
atcsDoFlush(ln)
register struct atcs_lines *ln;
{	struct ATCSWaitForCarrierReq cmsg;

	cmsg.Type = ATCSWaitForCarrierC;
	cmsg.State = 0;
	ln->l_cci_state |= DETACH_SENT;
	ln->l_cci_state &= ~(ATTACH_SENT|ATTACHED|SWITCHED);
	atcsSendCommand(ln, A_CLOSE, &cmsg, sizeof(cmsg));
	DDEBUG(DCALL, (CE_CONT, "atcsclose : CCILine = %d, port queue flushed\n",
				atcsMin2CCILine[ln->l_minor]));
}

/*
 *	This procedure is called from the interrupt handler to do cleanup when
 *	a line is closed.
*/
atcsDoDetach(ln)
register struct atcs_lines *ln;
{	struct CCIDetachReq	dmsg;

	DDEBUG(DCALL, (CE_CONT, "atcsclose : detach called\n"));
	dmsg.Type = CCIDetachC;
	dmsg.LineNumber = atcsMin2CCILine[ln->l_minor];
	dmsg.Subchannel = 0;
	/*
	 * if carrier is lost and we are on a dialup line send 
	 * session stat 1 to the next host 
	 */
	if (ln->l_sess_stat == 1) {
		ln->l_sess_stat = 0;
		dmsg.SessionStatus[0] = 1;
		DDEBUG(DCALL, (CE_CONT, "Detach: CCI Line = %d, Session status 1\n",
					atcsMin2CCILine[ln->l_minor]));
	} else
		dmsg.SessionStatus[0] = 0;
	dmsg.SessionStatus[1] = 0;
	dmsg.SessionStatus[2] = 0;
	dmsg.SessionStatus[3] = 0;
	(void) atcsSendCCICommand(ln, A_ASYNC, &dmsg, sizeof(dmsg));
	ln->l_cci_state &= ~DETACH_SENT;
}

/*
 * atcsDoSwitch() -- make and send the message to switch the line to a new host
 */
atcsDoSwitch(unit, CCILine, new_host)
int	unit;
int	CCILine;
ushort new_host;
{	register struct	strtty *tp;
	register struct	atcs_lines *ln;
	struct CCISwitchReq smsg;
	struct CCISwitchResp *smsgr;
	int s;

	DDEBUG(DIOCTL, (CE_CONT, "atcsDoSwitch: In Switch\n"));
	if (new_host >= ICS_MAX_SLOT) {
		cmn_err(CE_WARN, "Switch failed at the ioctl\n");
		return(EINVAL);
	}

	ln = &atcs_line[unit];
	s = SPL();
	ln->l_cci_state |= SWITCHED;
	splx(s);

	smsg.Type = CCISwitchC;
	smsg.LineNumber = CCILine;
	smsg.Subchannel = 0;
	smsg.NewHost = new_host;
	smsg.SessionStatus[0] = 0;
	smsg.SessionStatus[1] = 0;
	smsg.SessionStatus[2] = 0;
	smsg.SessionStatus[3] = 0;
	/*
	**	The only time atcsSendCCICommand returns a non-zero value is when
	**	a signal comes in while it is sleeping waiting for switch response.
	**	The response will still come in & the interrupt routine deals with
	**	this situation.  We need only return the errno value to the ioctl.
	*/
	if (atcsSendCCICommand(ln, A_SYNC, &smsg, sizeof(smsg)))
		return(EINTR);

	smsgr = (struct CCISwitchResp *)&ln->l_lmsg[0];

	if ((smsgr->Status == CCISuccess) || (smsgr->Status == CCISwitched)) {
		DDEBUG(DWILF, (CE_CONT, "atcsDoSwitch: Switch success\n"));

		s = SPL();
		ln->l_cci_state &= ~SWITCHED; 

		splx(s);

		/* Previous host detected carrier loss: 
		   If not local line send signal if open */

		if ((smsgr->Status== CCISwitched) && (smsgr->SessionStatus[0] == 1)) {

			DDEBUG(DWILF, (CE_CONT, "Switch Resp:CCI Line = %d, Carrier loss detected\n",
					CCILine));
			ln->l_state &= ~CARRF;
			tp = ln->l_tty;
			if ((tp->t_state&CARR_ON) && !(tp->t_cflag&CLOCAL))
				iasy_hup(tp);
			atcsSetCarr(tp, ln);
		}
	} else {
		DDEBUG(DWILF, (CE_CONT, "atcsDoSwitch: Switch failure\n"));
		s = SPL();
		ln->l_cci_state &= ~SWITCHED;
		splx(s);
		return(ENXIO);
	}
	return(0);
}
 
/*
 * atcsintr(msg) -- receive a message packet from the device
 *
 * The received message is not closely examined but the action code
 * that is returned with the response is what keys the activity.
 *
 * Notice that atcsstartio() is called directly or indirectly via the
 * proc routines.  The atcsWaitForCarrier() like routines are not called
 * directly here because if two people have the device open at the
 * same time, the control of only one request outstanding at a time
 * is handled by atcsstartio().
 */
int
atcsintr(tmsg)
struct msgbuf *tmsg;
{	register struct strtty 		*tp;
	register struct atcs_lines 	*ln;
	int	unit;
	int rtnstat;		/* status returned by CCI request */
	unsigned	char	c;

	DDEBUGMSG(DMSG, ("atcs intr:", tmsg));
	DMSGKEEP(tmsg);
	if (!(mps_msg_iscompletion(tmsg)) || (tmsg->mb_bind == -1)) {
		/* a message that isn't for us, wonder where it came from */
		cmn_err(CE_WARN, "atcs: unknown mb2 transport message\n");
		DMONITOR();
		goto free_msg;
	}
	unit = BIND_MINOR(tmsg);
	ln = &atcs_line[unit];
	tp = ln->l_tty;

	if (mps_msg_iserror(tmsg)) {
		cmn_err(CE_WARN, "atcs: mb2 msg err: Smid=%xh, Spid=%xH, bind=%x\n",
			mps_msg_getsrcmid(tmsg), mps_msg_getsrcpid(tmsg), tmsg->mb_bind);
		DMONITOR();
		/* just fall through, some requests will recover */
	}

	if (mps_msg_iscancel(tmsg)) {
		cmn_err(CE_WARN, "atcs: Transaction cancelled: minor=%d, action=%d\n",
			ln->l_minor, BIND_ACTION(tmsg));
		mps_msg_showmsg(tmsg);
		DMONITOR();
		/* just fall through, some requests will recover */
	}

	rtnstat = ((struct ATCSReceiveResp *)mps_msg_getudp(tmsg))->Status;
	if (rtnstat == ATCSFailure) {
		switch (((struct ATCSReceiveResp *)mps_msg_getudp(tmsg))->Type) {
			/*
			**	Per atcs users guide, if we send a CCI detach message while
			**	there is a pending CCI attach, we will get an error reply
			**	for the pending CCI attach, followed by a reply to the
			**	CCI detach.
			*/
			case CCIAttachC:
				if (ln->l_cci_state & ATTACH_CANCEL) {
					ln->l_cci_state &= ~ATTACH_CANCEL;
					break;
				}
			/* FALLTHRU */
			default:
				cmn_err(CE_WARN, "atcs: atcs msg err: minor=%d, action=%d\n",
					ln->l_minor, BIND_ACTION(tmsg));
				mps_msg_showmsg(tmsg);
				DMONITOR();
				break;
		}
		/* just fall through, some requests will recover */
	}
	--ln->l_actions[BIND_ACTION(tmsg)];
	DDEBUG(DINTR, (CE_CONT, "atcsintr: minor=%d, action=%s\n",
		ln->l_minor, ActionName[BIND_ACTION(tmsg)]));
	switch (BIND_ACTION(tmsg)) {
		/*
		 * A_SYNC: this is a response to a request that the requestor
		 * wanted to wait for.Save the response for him and wake him up.
		 * The CCI switch response is checked here and if so, the
    	 * SWITCHED state of the line is reset. This was moved here 
    	 * from the atcsDoSwitch procedure to handle the case where 
       	 * the process that issued the switch ioctl gets killed. 
		 */ 

		case A_SYNC: {
			bcopy((caddr_t)mps_msg_getudp(tmsg), &ln->l_lmsg[0], 18);
			if ((mps_msg_getsrcpid(tmsg) == mps_mk_mb2soctopid(ln->l_CCISocket)) &&
						(ln->l_lmsg[0] == CCISwitchC)) {
				ln->l_cci_state &= ~SWITCHED;
				atcsstartio(tp, ln);
			}

			wakeup((caddr_t)&ln->l_actions[A_SYNC]);
			break;
		}
		/*
		 * A_ASYNC: response for a request that we don't care about.
		 */
		case A_ASYNC:
			break;
		/*
		 * A_SPCHAR: an interrupt char.  Call the line routine to
		 * do the character action.  We flush the input queue here
		 * because if the raw queue is full the VKILL would not
		 * get processed and we must do the call to character processing
		 * for VQUIT and VINTR to get the effect right now.  These
		 * special characters will also appear in the input stream so
		 * that's when the echoing of the VKILL will happen.
		 * atcsstartio() is called to repost the next
		 * special char request.
		*/
		case A_SPCHAR: {
			(void) drv_setparm(SYSRINT, 1);
			if (rtnstat == ATCSSuccess) {
				c = ((struct ATCSWaitForSpecialCharResp *)
						mps_msg_getudp(tmsg))->SpecialChar;
				if (c == tp->t_cc[VQUIT] || c == tp->t_cc[VINTR])
					if (tp->t_lflag & ISIG)
						PutInputChar(tp, c);
			}
			(void) atcsproc(tp, T_OUTPUT);
			break;
		}
		/*
		 * A_CD: carrier state change.  Save the real carrier state in
		 * l_state but then set t_state to the preceived state (via atcsSetCarr).
		 * This is done so that turning CLOCAL on an off will work properly.
		 * If CARR_ON is off, no output can happen (done by ttwrite)
		 * therefore CLOCAL has to keep CARR_ON all of the time.
		 */
		case A_CD: {
			(void) drv_setparm(SYSMINT, 1);
			switch(((struct ATCSWaitForCarrierResp *)mps_msg_getudp(tmsg))->State) {
				case ATCSWFCOn:
					/* Carrier Detect: Wakeup the process */
					ln->l_state |= CARRF;
					atcsSetCarr(tp, ln);
					wakeup((caddr_t)&tp->t_rdqp);
					break;
				case ATCSWFCOff:
					/* Carrier Loss: If not local line send signal if open */
					ln->l_state &= ~CARRF;
					if ((tp->t_state&CARR_ON) && !(tp->t_cflag&CLOCAL))
						iasy_hup(tp);
					atcsSetCarr(tp, ln);
					break;
			}
			(void) atcsproc(tp, T_OUTPUT);
			break;
		}
		/*
		 * A_CLEARBUSY: a response to a wait-for-low-water-mark request that
		 * was waiting for zero characters.  This means that the port is no
		 * longer busy outputting.  We also know that all of the port's
		 * output buffer is totally available.
		 */
		case A_CLEARBUSY:
			(void) drv_setparm(SYSXINT, 1);
			tp->t_state &= ~BUSY;
			ln->l_tbufavail = ln->l_tbufsiz;
			(void) atcsproc(tp, T_OUTPUT);
			break;
		/*
		 * A_OUTFLOW: a response to a wait-for-low-water-mark request.
		 * Reset what we think is available in the port's output queue
		 * and see if there is more to output.
		 * This "available" calculation is fallacious because we really
		 * can't tell what is on the board.  This calcuation is done to
		 * get an A_CLEARBUSY next time.  I'm going to get output flow
		 * control redone to allow reposting of the low water mark
		 * request and to have it return the available output space.
		 */
		case A_OUTFLOW:
			(void) drv_setparm(SYSXINT, 1);
			ln->l_tbufavail = ln->l_tbufsiz/2;
			(void) atcsproc(tp, T_OUTPUT);
			break;
		/*
		 * A_OUTBUFC: an output characters request is done.  Send more chars.
		 */
		case A_OUTBUFC:
			(void) drv_setparm(SYSXINT, 1);
			ln->l_state &= ~OUTBUSY;
			(void) atcsproc(tp, T_OUTPUT);
			break;
		/*
		 * A_INCHARS: characters coming in.  Receive the characters only
		 * if the port is open.
		 */
		case A_INCHARS:
			(void) drv_setparm(SYSRINT, 1);
			if (tp->t_state&ISOPEN)
				switch (mps_msg_getmsgtyp(tmsg)) {
				case MPS_MG_UNSOL: {
					/* date is in tmsg block */
					register unsigned char *cp;
					int cnt;
					struct ATCSReceiveUResp *rd;
					rd = (struct ATCSReceiveUResp *)mps_msg_getudp(tmsg);
					for (cp=(unchar *)&rd->Data[0],cnt=rd->Count; cnt--; ++cp)
						atcsProcessInputChar(tp, ln, *cp);
					if (ln->l_state & RECFF) {
						PutInputChar(tp, ln->l_mask); /* 0xff&l_mask */
						if (ln->l_state & RECFFNUL) 
							PutInputChar(tp, 0);
						ln->l_state &= ~(RECFF|RECFFNUL);
					}
					break;
				}
				case MPS_MG_BREQ: {
					/* data in data buffer */
					/* we don't ask for the data in this form */
					break;
				}
				default:
					cmn_err(CE_WARN, "atcs: unknown input type: min=%d\n",
						ln->l_minor);
			}
			(void) atcsproc(tp, T_OUTPUT);
			break;

		case A_CLOSE:
			atcsDoDetach(ln);
			DDEBUG(DWILF, (CE_CONT, "Detach: CCI Line = %d,\n",
					atcsMin2CCILine[ln->l_minor]));
			break;
		default:
			cmn_err(CE_WARN, "atcs: unknown action: act=%xH, min=%d\n",
				BIND_ACTION(tmsg), ln->l_minor);
	}
free_msg:
	if (mps_msg_gettrnsid(tmsg))
		(void) mps_free_tid(ln->l_chan, mps_msg_gettrnsid(tmsg));
	mps_free_msgbuf(tmsg);
}

atcsProcessInputChar(tp, ln, c)
register struct strtty *tp;
register struct atcs_lines *ln;
register unsigned char c;
{	if (tp->t_in.bu_ptr != NULL) {
		if (tp->t_iflag & PARMRK) { 
			switch (ln->l_state & (RECFF|RECFFNUL)) {
			/*
			 *  This code handles the parity marking case when INPCK is on
			 *  IGNPAR is off and PARMRK is on.  The atcs will only mark
			 *  parity errors so we keep the state of the mark in RECFF and
			 *  RECFFNUL.  RECFF goes on when a mark (0xFF) is received.
			 *  RECFFNUL goes on when the next character is a NUL.  The
			 *  character after that will be the parity error char and will
			 *  get turned to NUL if the flags warrent that.  If a NUL does
			 *  not follow the MARK and marking is enabled the character is
			 *  passed through.
			 *  Note that RECFF is only set when marking is being turned to
			 *  NULs.
			 */
				case 0:
					/* no flags.  See if the char is the mark character */
					if (c == 0xFF) {
						if ((tp->t_iflag&INPCK) && !(tp->t_iflag&IGNPAR)) {
							ln->l_state |= RECFF;
							DDEBUG(DWILF, (CE_CONT, "ProcessChar: marking FF\n"));
							return;
						}
					}
					break;
				case RECFF:
					/* last character was a mark char */
					if (c == 0) {
						/* received the NUL after the MARK */
						ln->l_state |= RECFFNUL;
						DDEBUG(DWILF, (CE_CONT, "ProcessChar: NUL after mark\n"));
						return;
					} else {
						/* MARK not followed by NUL.  Another MARK means one MARK */
						if ((c == 0xFF)&&(ln->l_mask == 0xFF)) {
							PutInputChar(tp, 0xFF); /* put in ignored MARK */
							/* process received char by falling through */
						}
						ln->l_state &= ~(RECFF|RECFFNUL);
						DDEBUG(DWILF, (CE_CONT, "ProcessChar: non-NUL after mark\n"));
					}
					break;
				case RECFF|RECFFNUL:
					/* received both MARK and NUL.  This is the bad char. */
					ln->l_state &= ~(RECFF|RECFFNUL);
					DDEBUG(DWILF, (CE_CONT, "ProcessChar: sending marked parity\n"));
					PutInputChar(tp, 0xFF);
					PutInputChar(tp, 0);
					break;
			}
		}
		/*
		 * process input character
		 * mask to return only data bits
		*/
		PutInputChar(tp, c&ln->l_mask);
	}
}

/*
 *	atcsstartio()
 *
 * This routine is designed to ba called at any time to start anything
 * that needs starting.  It sees if any of the outstanding send_rsvps
 * need to be started and does them.  If any output can go out, that
 * is started also.
 *
 * This routine should be called to start any of the WaitForXxxx()
 * calls because this routine checks to see if a request is already
 * outstanding before posting another one.  If a port is open by two
 * people, the ln structure will guarantee that only one request is
 * posted at a time.
 */
atcsstartio(tp, ln)
register struct strtty *tp;
register struct atcs_lines *ln;
{	int s;

	/* DDEBUG(DCALL, (CE_CONT, "atcsstartio: tp=%x, ln=%x\n", tp, ln) ); */

	s = SPL();
	/* if the line is attached restart all pending waits */
	if ((ln->l_cci_state & ATTACHED)
	&& !(ln->l_cci_state & SWITCHED)
	&& !(ln->l_cci_state & DETACH_SENT)) {
		/* if outstanding request for carrier state is not happening do it */
		if (!(ln->l_actions[A_CD])) {
			atcsWaitForCarrier(ln);
		}
		/* if outstanding request for special chars is not happening do it */
		if (!(ln->l_actions[A_SPCHAR])) {
			atcsWaitForSpecialChar(ln);
		}
		/* if outstanding request for input chars is not happening do it */
		if (!(ln->l_actions[A_INCHARS])) {
			atcsWaitForCharacters(ln);
		}
		/* check to see if output can go out */
		if (!(ln->l_state&OUTBUSY) && ((tp->t_state&(TTSTOP|TIMEOUT)) == 0))
			atcsGenerateOutput(tp, ln);

		/* busyness and flow control */
		if (ln->l_tbufavail != ln->l_tbufsiz) {
			tp->t_state |= BUSY;
			/*
			 * Notice this test for OUTBUSY!  This means that we won't
		 	 * output a flow control request until outputting isn't
		 	 * happening any more (better chance for OUTFLOW rather than
		 	 * CLEARBUSY) but, most importantly, it means that the solicited
		 	 * data out transfer is not immediatly followed by this unsolicited
		 	 * status transfer.  A hard to find bug was that, if the solicited
		 	 * channel was busy, a CLEARBUSY request generated here could beat
		 	 * the solicited data transfer generated by the atcsGenerateOutput()
		 	 * above thus screwing up bookkeeping (the CLEARBUSY
		 	 * would come back immediatly, we'd say all the output buffer was
		 	 * available, then the characters would arrive at the board.)
			 */
			if (!(ln->l_state&OUTBUSY) &&
					!(ln->l_actions[A_CLEARBUSY]) && 
					!(ln->l_actions[A_OUTFLOW])) {
				/*
                 * With ATCS, the way to tell if the buffer is available for
			 	 * more characters is to wait for a low water mark event.
			 	 * So, if we just output chars, we put out a WaitForSendComplete
			 	 * If the buffer is almost empty, we wait for the buffer to
			 	 * empty and then clear BUSY.
				 */
				struct ATCSWaitForSendCompleteReq smsg;
				smsg.Type = ATCSWaitForSendCompleteC;
				if (ln->l_tbufavail < ln->l_tbufsiz/2) {
					smsg.LowWaterMark = ln->l_tbufsiz/3;
					atcsSendCommand(ln, A_OUTFLOW, &smsg, sizeof(smsg));
				} else {
					smsg.LowWaterMark = 0;
					atcsSendCommand(ln, A_CLEARBUSY, &smsg, sizeof(smsg));
				}
			}
		}
	}
	splx(s);
	return;
}

/*
 * atcsGenerateOutput() -- Send any characters to the board.
 *
 * Characters are fetched from the output queue using l_output which
 * places pointers to some characters in t_out.  The characters are
 * left in the t_out block until they are all output.  If they cannot
 * all be sent to the port in one write (usually because it's output
 * buffer is nearly full) only part of the buffer is sent to the board
 * and the rest remain.  This also means that flush must remember to
 * zap any characters left here.
*/
atcsGenerateOutput(tp, ln)
register struct strtty *tp;
register struct atcs_lines *ln;
{	struct ATCSBlockSendReq bmsg;
	mps_msgbuf_t *mmsg;
	struct dma_buf *db;
	int bytes;
	unsigned char our_tid;

	if (!(tp->t_out.bu_cnt))
		if (!(CPRES & iasy_output(tp)))
			return;
	bytes = tp->t_out.bu_cnt;
	bytes = bytes > atcsMaxOChars ? atcsMaxOChars : bytes;
	bytes = bytes >= ln->l_tbufavail ? 0 : bytes;

	if (bytes > ATCSSIMaxChar) {
		/* the characters require a data block transfer */
		bmsg.Type = ATCSBlockSendC;
		if ((mmsg = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)NULL))
			cmn_err (CE_PANIC, "atcsGenerateOutput: Cannot get message buffer");
		if ((our_tid = mps_get_tid(ln->l_chan)) == (unchar)0) {
			cmn_err(CE_PANIC,"atcsGenerateOutput: cannot get transaction id\n");
		}
		mps_mk_sol(mmsg, ln->l_HisSocket, our_tid,
					(unsigned char *)&bmsg, sizeof(bmsg));
		BIND_ACTION(mmsg) = A_OUTBUFC;
		BIND_MINOR(mmsg) = ln->l_minor;
		if ((db = mps_get_dmabuf(2,DMA_NOSLEEP)) == ((struct dma_buf *)NULL))
			cmn_err (CE_PANIC, "atcs: Cannot get data buffer");
		db->count = bytes;
		db->address = kvtophys((caddr_t)tp->t_out.bu_ptr);
		DDEBUGMSG(DMSG, ("atcs cout:", mmsg));
		DMSGKEEP(mmsg);

		if (mps_AMPsend_rsvp(ln->l_chan, mmsg, db, (struct dma_buf *)NULL) == -1) {
			mps_msg_showmsg(mmsg);
			cmn_err(CE_PANIC, "atcs: send_rsvp failure: chan=%xH\n",
				ln->l_chan);
		}

		++(ln->l_actions[A_OUTBUFC]);
	} else if (bytes > 0) {
		/* the characters will fit in a command only buffer */
		struct ATCSSendImmediateReq smsg;
		smsg.Type = ATCSSendImmediateC;
		smsg.Count = bytes;
		bcopy((caddr_t)tp->t_out.bu_ptr, &smsg.Data[0], bytes);
		atcsSendCommand(ln, A_OUTBUFC, &smsg, sizeof(smsg));
	}
	if (bytes) {
		ln->l_state |= OUTBUSY;
		ln->l_tbufavail -= bytes;
		tp->t_out.bu_ptr += bytes;
		tp->t_out.bu_cnt -= bytes;
	}
}

/*
 * atcsparam() -- send configuration parameters to port
 *
 * This procedure sets up the baud rate and the parameter of the device.
 * The code depends on having the ttystructure filled out before a call is made
 * to atcsparam.  This routine assumes that BUSY is clear -- the caller
 * must wait for output to finish before calling this routine.
 */
int
atcsparam(tp, ln)
register struct strtty *tp;
register struct atcs_lines *ln;
{	int s;
	unsigned int	link, mode;
	unsigned int	sp, speed;
	struct ATCSSetLineParametersReq prm;

	DDEBUG(DCALL, (CE_CONT, "atcsparam minor %d ::\n",ln->l_minor));

	/*
	 * Critical region
	 */
	s = SPL();

	/* calling routine must wait for BUSY if they require */

	/*
	 * if speed is zero disable line else
	 * setup configuration message with current
	 * tty configuration and send to board
	 */
	sp = (tp->t_cflag&CBAUD);
	if (sp == 0) {
		/*
		 * only disable modem line
		 */
		if (!(tp->t_cflag&CLOCAL)) {
			(void) atcsproc(tp, T_DISCONNECT);
			splx(s);
			return(0);
		} else {
			splx(s);
			return(EINVAL);
		}
	}
	/*
	 * translate index into hardware dependent number
	 */
	if ((sp > MAXBAUDS) || ((speed = atcsbaud[sp]) == 0)) {
		splx(s);
		return(EINVAL);
	}
	/*
	 * construct a complete line configure message
	 */
	prm.InputBaudRate = speed;
	prm.OutputBaudRate = speed;
	link = mode = 0;
	if (tp->t_cflag & CSTOPB)
		link |= ATCSL2Stop;
	else
		link |= ATCSL1Stop;
	if (tp->t_cflag & PARENB) {
		if (tp->t_cflag & PARODD) {
			link |= ATCSLOddParity;
		} else {
			link |= ATCSLEvenParity;
		}
	} else {
		link |= ATCSLNoParity;
	}
	/* Set character size bits in "param" */
	switch (tp->t_cflag & CSIZE) {
		case CS8: link |= ATCSL8Bits; ln->l_mask = 0xff; break;
		case CS7: link |= ATCSL7Bits; ln->l_mask = 0x7f; break;
		case CS6: link |= ATCSL6Bits; ln->l_mask = 0x3f; break;
		case CS5: link |= ATCSL5Bits; ln->l_mask = 0x1f; break;
		default:
			splx(s);
			return(EINVAL);
	}
	atcsSetCarr(tp, ln);
	link |= ATCSLEnable;	/* transmitter and receiver enabled */
	if (tp->t_iflag & ISTRIP)
		mode |= ATCSMStrip;
	if (tp->t_iflag & IXON) {
		mode |= ATCSMXON;
		if (tp->t_iflag & IXANY)
			mode |= ATCSMResumeMode2;
		else
			mode |= ATCSMResumeMode1;
	}
	if (tp->t_lflag & ISIG)
		mode |= ATCSMSpCharEnable;
	if (tp->t_iflag & INPCK) {
		if (tp->t_iflag & IGNPAR) {
			mode |= ATCSMInErrDiscard;
		} else {
			if (tp->t_iflag & PARMRK)
				mode |= ATCSMInErrEsc;
			else
				mode |= ATCSMInErrNULL;
		}
	} else {
		mode |= ATCSMInErrIgnore;
	}
	if (tp->t_iflag & IXOFF)
		mode |= ATCSMTandem;
	prm.Link = link;
	prm.Mode0 = mode;
	prm.Mode1 = 0;
	prm.SpecialChar[0] = (unchar)0377;
	prm.SpecialChar[1] = (unchar)0377;
	prm.SpecialChar[2] = (unchar)0377;
	prm.SpecialChar[3] = (unchar)0377;
	if (tp->t_lflag & ISIG) {
		prm.SpecialChar[0] = (unchar)tp->t_cc[VINTR];
		prm.SpecialChar[1] = (unchar)tp->t_cc[VQUIT];
		prm.SpecialChar[2] = (unchar)tp->t_cc[VSWTCH];
	}
	if (tp->t_lflag & ICANON) {
		prm.SpecialChar[3] = (unchar)tp->t_cc[VKILL];
	}
	prm.reserved01 = 0;
	prm.reserved02 = 0;
	prm.Type = ATCSSetLineParametersC;
	atcsSendCommand(ln, A_ASYNC, &prm, sizeof(prm));
	splx(s);
	return(0);
}

/*
 * atcsSetCarr() -- set the carrier state flag correctly
 *
 * "Correctly" is to check the actual carrier state (in l_state) and
 * set the perceived carrier state (in t_state) to either on or off
 * depending on the actual carrier state (in l_state) and on CLOCAL.
 */
atcsSetCarr(tp, ln)
register struct strtty *tp;
register struct atcs_lines *ln;
{	int s;
	s = SPL();
	if (tp->t_cflag & CLOCAL)
		tp->t_state |= CARR_ON;
	else
		if (ln->l_state & CARRF)
			tp->t_state |= CARR_ON;
		else
			tp->t_state &= ~CARR_ON;
	splx(s);
}

/*
 * atcsSendClearDTR() -- send command to clear DTR
 */
atcsSendClearDTR(ln, action)
struct atcs_lines *ln;
int action;
{	struct ATCSSetModemSignalReq fmsg;

	fmsg.Type = ATCSSetModemSignalC;
	fmsg.RTSCommand = ATCSSMSNoChange;
	fmsg.DTRCommand = ATCSSMSClear;
	fmsg.BreakCommand = ATCSSMSNoChange;
	atcsSendCommand(ln, action, &fmsg, sizeof(fmsg));
}

/*
 * atcsSendSetDTR() -- send command to set DTR
 */
atcsSendSetDTR(ln, action)
struct atcs_lines *ln;
int action;
{	struct ATCSSetModemSignalReq fmsg;

	fmsg.Type = ATCSSetModemSignalC;
	fmsg.RTSCommand = ATCSSMSNoChange;
	fmsg.DTRCommand = ATCSSMSSet;
	fmsg.BreakCommand = ATCSSMSNoChange;
	atcsSendCommand(ln, action, &fmsg, sizeof(fmsg));
}

/* 
 * atcsWaitForCharacters() -- set up wait for input characters
 *
 * For the moment, we will only receive the characters in the control
 * only packet.  This saves us input buffer allocation.
 * For input control, we only do this request if there is room in
 * the input clist.  If no room, we don't do the input request.
 * Later, someone will call startio() and things will get going again.
 */
atcsWaitForCharacters(ln)
struct atcs_lines *ln;
{	struct ATCSReceiveReq rmsg;
	int count;

	if (ln->l_tty->t_state & TBLOCK)
		return;
	count = ln->l_tty->t_in.bu_cnt;
	count = count>atcsMaxIChars ? atcsMaxIChars : count;
	if (count > ATCSRSmallChars) {
		/* ask for chars in a data block */
		/* --- not written yet --- */
		cmn_err(CE_PANIC, "atcs: asking for chars in data block\n");
	} else {
		if (ln->l_actions[A_INCHARS] != 0) {
			cmn_err(CE_PANIC,
				"atcs: WaitForChars already outstanding, cnt=%d\n",
				ln->l_actions[A_INCHARS]);
		}
		/* just ask for an small block of chars */
		rmsg.Type = ATCSReceiveC;
		rmsg.MaxCount = ATCSRSmallChars;
		rmsg.MinCount = 10;
		rmsg.Timeout = 5;		/* 5*10ms = 1/20 of a sec */
		rmsg.IdleTime = 0;
		atcsSendCommand(ln, A_INCHARS, &rmsg, sizeof(rmsg));
	}
}

/* 
 * atcsWaitForCarrier() -- send atcs command to wait for carrier state change
 */
atcsWaitForCarrier(ln)
struct atcs_lines *ln;
{	struct ATCSWaitForCarrierReq cmsg;

	if (ln->l_actions[A_CD] != 0) {
		cmn_err(CE_PANIC,
			"atcs: atcsWaitForCarrier already outstanding, cnt=%d\n",
			ln->l_actions[A_CD]);
	}
	if (ln->l_state & CARRF) {
		/* we think it's on -- wait for it to go off */
		cmsg.Type = ATCSWaitForCarrierC;
		cmsg.State = ATCSWFCOff;
		atcsSendCommand(ln, A_CD, &cmsg, sizeof(cmsg));
	} else {
		/* we think it's off -- wait for it to go on */
		cmsg.Type = ATCSWaitForCarrierC;
		cmsg.State = ATCSWFCOn;
		atcsSendCommand(ln, A_CD, &cmsg, sizeof(cmsg));
	}
}

/*
 * atcsWaitForSpecialChar() -- set up wait for special input char
 */
atcsWaitForSpecialChar(ln)
struct atcs_lines *ln;
{	struct ATCSWaitForSpecialCharReq wmsg;

	if (ln->l_actions[A_SPCHAR] != 0) {
		cmn_err(CE_PANIC,
			"atcs: atcsWaitForSpecialChar already outstanding, cnt=%d\n",
			ln->l_actions[A_SPCHAR]);
	}
	wmsg.Type = ATCSWaitForSpecialCharC;
	atcsSendCommand(ln, A_SPCHAR, &wmsg, sizeof(wmsg));
}

/*
 * atcsSendCCICommand() -- send a cci command
 *
 * The built message is placed into a message control block and
 * sent out.  If the action is A_SYNC this driver will sleep until
 * all of the synchronious requests are done for this line.  Other
 * actions are legal to be called from interrupt routines.
 */
atcsSendCCICommand(ln, action, mssg, mssgl)
struct atcs_lines *ln;
int action;		/* A_xxxx -- action to take when complete */
char *mssg;		/* message to send */
int mssgl;		/* number of bytes in mssg */
{	register mps_msgbuf_t *mbp;
	unsigned char our_tid;

	if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)NULL))
		cmn_err (CE_PANIC, "atcsSendCCICommand: Cannot get message buffer");
	if ((our_tid = mps_get_tid(ln->l_chan)) == (unchar)0) {
		cmn_err(CE_PANIC,"atcsSendCommand: cannot get transaction id\n");
	}
	mps_mk_unsol(mbp, ln->l_CCISocket, our_tid, (unsigned char *)mssg, mssgl);
	BIND_ACTION(mbp) = action;
	BIND_MINOR(mbp) = ln->l_minor;
	DMSGKEEP(mbp);

	if (mps_AMPsend_rsvp(ln->l_chan, mbp,
					 (struct dma_buf *)NULL, (struct dma_buf *)NULL) == -1) {
		mps_msg_showmsg(mbp);
		/* this is a panic situation -- error occurs when params are wrong */
		cmn_err(CE_PANIC, "atcs: rsvp failure: ln=%xH, chan=%xH, min=%d\n",
			ln, ln->l_chan, ln->l_minor);
	} else {
		DDEBUGMSG(DMSG, ("atcs sccicmd:", mbp));
		++(ln->l_actions[action]);
		if (action == A_SYNC)
			while (ln->l_actions[A_SYNC]) 
				if (sleep((caddr_t)&ln->l_actions[A_SYNC], TTIPRI|PCATCH))
					return(EINTR);
	}
	return(0);
}

/*
 * atcsSendCommand() -- send an atcs command
 *
 * The built message is placed into a message control block and
 * sent out.  If the action is A_SYNC this driver will sleep until
 * all of the synchronious requests are done for this line.  Other
 * actions are legal to be called from interrupt routines.
 */
atcsSendCommand(ln, action, mssg, mssgl)
struct atcs_lines *ln;
int action;		/* A_xxxx -- action to take when complete */
char *mssg;		/* message to send */
int mssgl;		/* number of bytes in mssg */
{	register mps_msgbuf_t *mbp;
	unsigned char our_tid;

	if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)NULL))
		cmn_err (CE_PANIC, "atcs: Cannot get message buffer");
	if ((our_tid = mps_get_tid(ln->l_chan)) == (unchar)0) {
		cmn_err(CE_PANIC,"atcsSendCommand: cannot get transaction id\n");
	}
	mps_mk_unsol(mbp, ln->l_HisSocket, our_tid, (unsigned char *)mssg, mssgl);
	BIND_ACTION(mbp) = action;
	BIND_MINOR(mbp) = ln->l_minor;
	DMSGKEEP(mbp);

	if (mps_AMPsend_rsvp(ln->l_chan, mbp,
					 (struct dma_buf *)NULL, (struct dma_buf *)NULL) == -1) {
		mps_msg_showmsg(mbp);
		/* this is a panic situation -- error occurs when params are wrong */
		cmn_err(CE_PANIC, "atcs: rsvp failure: ln=%xH, chan=%xH, min=%d\n",
			ln, ln->l_chan, ln->l_minor);
	} else {
		DDEBUGMSG(DMSG, ("atcs scmd:", mbp));
		++(ln->l_actions[action]);
		if (action == A_SYNC)
			while (ln->l_actions[A_SYNC])
				(void) sleep((caddr_t)&ln->l_actions[A_SYNC], TTIPRI|PCATCH);
	}
}

#ifdef DEBUGATCS
/* print the MB2 transport message */
atcsMsgPrint(hdr, mbp)
char *hdr;			/* text to print as header */
struct msgbuf *mbp;	/* message to display */
{
	cmn_err (CE_CONT, hdr);
	mps_msg_showmsg(mbp);
}
#endif

/*
 *	Hardware dependant ioctl support.
*/
/* ARGSUSED */
void
atcshwdep(q, bp)
queue_t *q;
mblk_t *bp;	/* This is an ioctl not understood by the DI code */
{	struct iocblk *ioc;
	struct strtty *tp;
	int *argp, unit, error;

	ioc = (struct iocblk *)bp->b_rptr;
	tp = (struct strtty *)q->q_ptr;
	argp = (int *) bp->b_cont->b_rptr;	/* argument to ioctl */
	unit = tp - atcs_tty;
	switch (bp->b_datap->db_type) {
	case M_IOCTL:
		error = EINVAL;		/* NACK unknown ioctls */
		switch (ioc->ioc_cmd) {
		case ATCS_SWITCH:
			if ((error = atcsDoSwitch(unit, atcsMin2CCILine[unit], (ushort)(*argp))) == 0) {
				bp->b_datap->db_type = M_IOCACK;
				(void) putnext(RD(q), bp);
				break;
			}
			/* FALL THROUGH */
		default:
			ioc->ioc_error = error;		/* NACK unknown or failed ioctls */
			ioc->ioc_rval = -1;
			bp->b_datap->db_type = M_IOCNAK;
			(void) putnext(RD(q), bp);
			return;
		}
		break;
	default:
		cmn_err(CE_PANIC, "atcs_hwdep: illegal message type");
	}
}

/*
 *	atcsinit() - initialize driver before interrupts
 */
atcsinit()
{	register struct atcs_lines *ln;
	struct atcs_info *p;
	register int i;
	char valbuf [128];
	char temp [16];
	int state1, state2, c_code, ret_val, ret_val_hi;
	int count = 0;	/* count of atcs_info elements */

	DDEBUG(DINIT, (CE_CONT, "atcsinit\n"));
	if (atcsAlreadyInit)
		return;
	atcsAlreadyInit++;
	/* interrupts are off -- just initialize data structures */
	for (i=0; i<=MAX_BOARD; i++) {
		atcsSetupDone[i] = SETUP_INIT; /* initialize setup */
	}
	for (i=0; i<atcsMaxLines; i++) {
		atcsMin2CCILine[i] = -1;
	}

	for (ln=(&atcs_line[0]); ln<(&atcs_line[atcsMaxLines]); ++ln) {
		ln->l_state = 0;
		ln->l_mask = 0xff;
		ln->l_cci_state = 0;
		ln->l_reserved1 = 0;
		ln->l_sess_stat = 0;
		ln->l_minor = -1;
		ln->l_chan = -1;
		ln->l_HisSocket = -1;
		ln->l_CCISocket = -1;
		ln->l_MySocket = -1;
		ln->l_tbufsiz = 0;
		ln->l_tbufavail = ln->l_tbufsiz;
		for (i=0; i<A_MAX; i++)
			ln->l_actions[i] = 0;
	}

	if (!bps_open()) {
		p = atcs_info;
		state1 = 0;
		while ((!bps_get_wcval("ASYNC*", &state1, sizeof(valbuf), valbuf))
			&& (count < ICS_MAX_SLOT)) {
			state2 = 0;	
			c_code = 0;
			while (!bps_get_opt(valbuf, &state2, "hid:minor:port", 
								&c_code, sizeof(temp), temp)) {
				switch (c_code) {
				case 1: 
					if (!bps_get_integer(temp, &ret_val)) { 
						/* 
						 * over-write the default in space.c 
						 */
						p->slot = ret_val;
					}
					break;
				case 2: 
					if (!bps_get_range(temp, &ret_val, &ret_val_hi)) { 
						/* 
						 * over-write the default in space.c 
						 */
						p->beg_minor_num = ret_val;
						p->end_minor_num = ret_val_hi;
					}
					break;
				case 3: 
					if (!bps_get_integer(temp, &ret_val)) { 
						/* 
						 * over-write the default in space.c 
						 */
						p->beg_port_num = ret_val;
					}
					break;
				default:
					break;
				}
			}
			DDEBUG (DINIT, 
				(CE_CONT, "atcs: Using bps for slot (%d) minor (0x%x-0x%x) port (%d)\n",
								p->slot, p->beg_minor_num, p->end_minor_num,
								p->beg_port_num ) );
			(++p)->slot = ICS_MAX_SLOT;
			count++;
		}
	}

	for (p = atcs_info; p->slot < ICS_MAX_SLOT; p++) {
		atcs_tty = iasy_register(p->beg_minor_num,
								p->end_minor_num - p->beg_minor_num + 1,
								atcsproc,
								atcshwdep);
	}
}

/*
 * atcsstart1() -- initialize devices after the interrupts are on
 */
atcsstart1(minr, CCISocket, CCILine)
minor_t minr;		/* minor number to associate with the line */
mb2socid_t CCISocket;	/* socket to connect to CCI */
int CCILine;		/* CCI line that I get to handle */
{	register struct atcs_lines *ln;

	DDEBUG(DINIT, (CE_CONT, "atcsstart1: minor=%xH\n", minr));
	if (minr >= atcsMaxLines) {
		cmn_err(CE_WARN, "atcs: exceeded atcsMaxLines: minor=%d\n", minr);
		return;
	}
	ln = &atcs_line[minr];
	atcsMin2CCILine[minr] = CCILine;
	ln->l_minor = minr;	/* efficiency hack; '->' faster than ptr subtraction */
	ln->l_tty = &atcs_tty[minr];
	ln->l_HisSocket = ln->l_CCISocket = CCISocket;
	ln->l_MySocket = mps_mk_mb2socid(mps_lhid(), 0x2260+minr);	/* XXX GROT */
	if ((ln->l_chan = mps_open_chan(mps_mk_mb2soctopid(ln->l_MySocket),
				atcsintr, MPS_SRLPRIO)) == (uint)-1) {
		/* how can an mps_open_chan() fail? */
		cmn_err(CE_PANIC, "atcs: open chan failed: Dsoc=%xH, ln=%d\n",
			ln->l_HisSocket, CCILine);
	}
}
