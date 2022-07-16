/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988, 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/i410.c	1.3"

#ifndef lint
static char i410_copyright[] = "Copyright 1986, 1987, 1988, 1989 Intel Corporation 460957";
#endif /* lint */

/************************************************************************
 *
 *	Name:
 *		iSBC410 device driver for UNIX V.3 on `386
 *
 *	Purpose:
 *		This is the set of routines that constitute the device
 *		for the iSBC 410 Intelligent Serial Interface Controller.
 *
 *	Comments:
 * 		Debug switches are: DEBUG410 for iSBC 410 support.
 *
 *************************************************************************/

#include "sys/types.h"
#include "sys/ics.h"
#include "sys/mps.h"
#include "sys/cmn_err.h"
#include "sys/ccimp.h"

#ifdef DEBUG410
#define DDEBUG(x,y) if(i410debug&(x))cmn_err y
#define DINIT 1
#define DCALL 2
#define DINTR 4
#define DIOCTL 8
#define DMSG 16
int	i410debug = 0; /* debug output control */
#define DDEBUGMSG(x,y) i410MsgPrint(x,y)
#else
#define DDEBUG(x,y)
#define DDEBUGMSG(x,y)
#endif

/* values configured in i410/space.c */
extern char *i410cfg[];

/* bitmap of the boards with CCI servers */
extern ulong cci_bitmap;

/* Routines to actually handle i410 lines */
extern int atcsproc();
extern void atcshwdep();

#define RESET410 4	/* number of times to reset 410 trying to get it working */


int i410devflag = 0;	/* V4 DDI/DKI driver */
#define ONE_MSEC	1000
#define ONE_SEC		(1000*ONE_MSEC)
#define BIST_WAIT	12	/* Seconds to wait for BIST complete */
#define I410_RESET	0x80
#define I410_ERROR	0x80
#define I410_INBIST	0x04
#define I410_BISTOK	0x20

/*
 *	i410init() - initialize driver before interrupts
 */
i410init()
{
	int num410 = 0;
	int slot;
	int resetcount;
	int firmrec;
	int j, st;

	cciinit();	/* Insure CCI is initialized */
	for (slot=0; slot<ICS_MAX_SLOT; slot++) {
		if (ics_agent_cmp(i410cfg, slot) != 0)
			continue;
		num410++;
		for (resetcount=0; resetcount<RESET410; ++resetcount) {
			/* within this loop, 'continue' means to reset the board */
			/*   and 'break' means the board is known good or bad */
			if (resetcount != 0) {
				/* if not first time, reset the board */
				cmn_err(CE_CONT,
					"iSBC 410 not cooperating. Resetting slot %d.\n", slot);
				/* set a local board reset */
				ics_write(slot, ICS_GeneralControl, I410_RESET);
				drv_usecwait(100*ONE_MSEC);
				/* clear reset and let the board go */
				ics_write(slot, ICS_GeneralControl, 0);
				/* wait a little so the board will come up */
				/* (a board waits 250ms for master intervention) */
				drv_usecwait(ONE_SEC);
			}
			if (ics_read(slot, ICS_BistSupportLevel) != 0) {
				/* wait for the bist tests to complete */
				for (j = 0; j < BIST_WAIT; j++) {
					/* check the BIST running bit */
					if ((st=ics_read(slot, ICS_BistSlaveStatus) & I410_INBIST) == 0)
						break;
					drv_usecwait(ONE_SEC);
				}
				if (j >= BIST_WAIT) {
					cmn_err(CE_CONT,
						"i410 bist taking too long: slot=%d, stat=%xH, id=%xH\n",
						slot, st, ics_read(slot, ICS_BistTestID));
					continue;
				}
				if ((st = ics_read(slot, ICS_BistSlaveStatus)) & I410_ERROR) {
					cmn_err(CE_CONT,
						"i410 slave test error: slot=%d, stat=%xH, id=%xH\n",
						slot, st, ics_read(slot, ICS_BistTestID));
					continue;
				}
				if (!((st=ics_read(slot, ICS_BistMasterStatus)) & I410_BISTOK)) {
					cmn_err(CE_CONT,
						"i410 master test error: slot=%d, gs=%xH, id=%xH\n",
						slot, st, ics_read(slot, ICS_BistTestID));
					continue;
				}
			}
			if ((st = ics_read(slot, ICS_GeneralStatus)) & I410_ERROR) {
				/* general error on the board */
				cmn_err(CE_CONT,
					"i410 general status error: slot=%d, stat=%xH, id=%xH\n",
					slot, st, ics_read(slot, ICS_BistTestID));
				continue;
			}

			cmn_err(CE_CONT,
				"iSBC 186/410 (slot=%d, class=%xH, gs=%xH, gc=%xH) found.\n",
				slot, ics_read(slot, ICS_ClassID),
				ics_read(slot, ICS_GeneralStatus),
				ics_read(slot, ICS_GeneralControl) );
			cci_bitmap |= 1<<slot;
			break;
		}
		if (resetcount >= RESET410) {
			cmn_err(CE_WARN, "iSBC 410 will not initialize: slot=%d.\n", slot);
		}
	}
	if (num410 <= 0) {
		cmn_err(CE_CONT, "i410init: no i186/410s found\n");
		return;
	}
}

/*
 *	i410start() - initialize board after interrupts enabled
 */
i410start()
{ 
}

#ifdef DEBUG410
/* print the MB2 transport message */
i410MsgPrint(hdr, mbp)
	char *hdr;			/* text to print as header */
	struct msgbuf *mbp;	/* message to display */
{
	cmn_err(CE_CONT, hdr);
	mps_msg_showmsg(mbp);
}
#endif
