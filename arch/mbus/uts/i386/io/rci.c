/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/rci.c	1.3.1.1"

#ifndef lint
static char rci_copyright[] = "Copyright 1988, 1989 Intel Corporation 463557";
#endif /*lint*/

/*
 *	Notes:
 *		Set your tab stops to 4.  (For vi, ":set ts=4 sw=4").
 *
 *		This driver uses the firmware communication region on the local CPU to
 *		receive replys from RCI servers.  RCILocateServerResp messages from RCI
 *		servers are stored starting with the first available byte of the region.
 *		Once a server has been located, it is used as the console and responses
 *		from it follow the area reserved for RCILocateServerResp messages. 
*/
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/signal.h"
#include "sys/param.h"
#include "sys/conf.h"
#include "sys/mps.h"
#include "sys/rcimp.h"
#include "sys/ics.h"
#include "sys/cred.h"
#include "sys/ddi.h"


mb2socid_t rciConsole;
ushort rciOffset;
long rciChannel = 0;

/*
 * rciSendImmediate() -- send an rci command
 *
 *	Note that the parts of the message that control the placement of the
 *	RCI server's response are set here and should not be set by the caller.
*/
unchar
rciSendImmediate(msg, msglen)
char *msg;		/* message to send */
int msglen;		/* number of bytes in msg */
{	register mps_msgbuf_t *mbp;

	/*
	 *	Clear the status flag in interconnect space.
	*/
	ics_write(ICS_MY_SLOT_ID, rciOffset, 0);
	/*
	 *	Build and send the request.
	*/
	msg[1] = ics_myslotid();
	msg[2] = rciOffset &  0xFF;
	msg[3] = rciOffset >> 8;
	if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)0))
		cmn_err (CE_PANIC, "rciSendImmediate: Cannot get message buffers");
	mps_mk_unsol(mbp, rciConsole, 0, (unsigned char *)msg, msglen);
	if (mps_AMPsend(rciChannel, mbp) == -1) {
		mps_msg_showmsg(mbp);
		cmn_err(CE_PANIC, "rci: send failure: chan=0x%x\n", rciChannel);
	}
	/*
	 *	Wait for response in interconnect space.
	*/
	while (ics_read(ICS_MY_SLOT_ID, rciOffset) == 0)
		drv_usecwait(10);
	return(ics_read(ICS_MY_SLOT_ID, rciOffset));
}

/*
 *	This is code to implement a broadcast locate protocol.
 *	This should ideally be modified to use the location service when
 *	that gets implemented.
*/
int
rciServerMsg(tmsg)
struct msgbuf *tmsg;
{
	mps_free_msgbuf(tmsg);
}

#define ONE_SEC (1000*1000)	/* microseconds per second */
mb2socid_t
rcilocate()
{	struct RCILocateServerReq lmsg;
	struct RCILocateServerResp lmsgr;
	struct ics_rw_struct ics;
	mps_msgbuf_t *mbp;
	ushort HostId;

	rciChannel = mps_open_chan(RCI_PORT_ID, rciServerMsg, MPS_SRLPRIO);
	if (rciChannel == -1) {
		cmn_err(CE_PANIC, "rcilocate: cannot open broadcast channel\n");
	}
	rciOffset = ics_find_rec(ICS_MY_SLOT_ID, ICS_FW_COM_REG) + ICS_DATA_OFFSET;
	if ((mbp = mps_get_msgbuf(KM_NOSLEEP)) == ((mps_msgbuf_t *)0))
		cmn_err (CE_PANIC, "rcilocate: Cannot get message buffers");
	lmsg.Type = RCILocateServerC;
	lmsg.Slot = ics_myslotid();
	lmsg.Register = rciOffset;
	mps_mk_brdcst(mbp, RCI_PORT_ID, (unsigned char *)&lmsg, sizeof(lmsg));
	ics_write(ICS_MY_SLOT_ID, rciOffset, 0);
	/* 224A crashes if we do the broadcast more than once?? */
	if (mps_AMPsend(rciChannel, mbp) == -1)
		cmn_err(CE_PANIC, "rcilocate: Broadcast failure. mbp=%x\n", mbp);
	drv_usecwait(2*ONE_SEC);			/* wait for response */
	if (ics_read(ICS_MY_SLOT_ID, rciOffset) != RCISuccess) {
		mps_close_chan(rciChannel);
		cmn_err(CE_CONT, "RCI Console server not found\n");
		return(0);
	}
	ics.slot_id = ICS_MY_SLOT_ID;	/* Set up to receive the response */
	ics.reg_id = rciOffset;
	ics.count = sizeof(lmsgr);
	ics.buffer = (unsigned char *)&lmsgr;
	ics_rdwr(ICS_READ_ICS, &ics); /* Get the response */
	rciOffset += sizeof(struct RCILocateServerResp);
	mps_close_chan(rciChannel);
	HostId = (lmsgr.HostIdH<<8) + lmsgr.HostIdL;
	return(mps_mk_mb2socid(HostId, RCI_PORT_ID));
}

extern int ics_cpunum();					/* Which CPU am I? */
extern struct streamtab iasyinfo;	/* pointer to iasy driver */
extern atcsopen();					/* pointer to ATCS driver */
extern int atcs_slot_to_line(int slot);

rciinit()
{	struct RCIEnableLineReq emsg;
	struct RCISetLineParametersReq smsg;
	register int i;
	int rcici(), rcico();
	dev_t co_dev;

	/*
	 *	If we already know a server, we connected to it when we found it.
	*/
	if (rciConsole)
		return;
	/*
	 *	Do broadcast and hope an RCI server responds.
	*/
	rciConsole = rcilocate();
	if (!rciConsole)
		return;
	rciChannel = mps_open_chan(rciConsole, rciServerMsg, MPS_SRLPRIO);
	if (rciChannel == -1) {
		cmn_err(CE_PANIC, "rciinit: cannot open channel\n");
	}
	/*
	 *	Compute the iasy major number.  If iasy is configured as the
	 *	console, adjust the console minor number to select window cpunum+1
	 *	on that board.  (Window 0 is graphics in the 279 implementation).
	*/
	for (i = 0; i < cdevcnt; i++) {
		if (cdevsw[i].d_open == atcsopen)	/* Non-streams ATCS */
			break;
		if (cdevsw[i].d_str == &iasyinfo)	/* Streams ATCS via iasy */
			break;
	}
	if (i >= cdevcnt) {
		cmn_err(CE_WARN,
				"Can't use RCI console without iasy or ATCS support\n");
		return;
	}
	co_dev=makedevice(i, atcs_slot_to_line(mps_mk_mb2soctohid(rciConsole))
						+ ics_cpunum());
	/*
	 *	Enable the console.
	*/
	emsg.Type = RCIEnableLineC;
	emsg.LineID = ics_myslotid();
	if (rciSendImmediate((char *)&emsg, sizeof(emsg)) != RCISuccess) {
		cmn_err(CE_WARN, "Can't enable RCI line\n");
		return;
	}
	/*
	 *	Configure the console.
	*/
	smsg.Type = RCISetLineParametersC;
	smsg.LineID = ics_myslotid();
	smsg.Link = RCIL7Bits|RCIL1Stop|RCILEvenParity|RCILEnable;
	smsg.InputBaudRate = RCIB9600;
	smsg.OutputBaudRate = RCIB9600;
	smsg.Mode = RCIMErrIgnore;
	if (rciSendImmediate((char *)&smsg, sizeof(smsg)) != RCISuccess) {
		cmn_err(CE_WARN, "Can't set RCI line parameters\n");
		return;
	}
	consregister(rcici, rcico, co_dev);
	cmn_err(CE_CONT, "RCI console server initialized on host %d\n",
				mps_mk_mb2soctohid(rciConsole));
}

/*
 *	Console output to an RCI device
*/
rcico(thechar)
int thechar;
{	struct RCITransmitReq smsg;

	if (thechar == '\n')
		rcico('\r');		/* Kludge newline to work */
	smsg.Type = RCITransmitC;
	smsg.LineID = ics_myslotid();
	smsg.Count = 1;
	smsg.Data[0] = thechar;
	while (rciSendImmediate((char *)&smsg, sizeof(smsg)) != RCISuccess) {
		/* Lost the console server */
		rciConsole = 0;
		mps_close_chan(rciChannel);
		rciinit();
	}
}

/*
 *	Console input to an RCI device
*/
int
rcici()
{	struct RCISenseInputReq smsg;
	int thechar;

	smsg.Type = RCISenseInputC;
	smsg.LineID = ics_myslotid();
	if (rciSendImmediate((char *)&smsg, sizeof(smsg)) != RCIHaveInput) {
		return(-1);
	}
	/*
	 *	Get the character
	*/
	thechar = ics_read(ICS_MY_SLOT_ID, rciOffset+1);
	return(thechar);
}
