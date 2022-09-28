/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/log.h	1.1"
#ifndef	LOG_H
#define	LOG_H

/* SCCSID(@(#)log.h	3.9	LCC);	/* Modified: 13:52:44 10/13/87 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	<stdio.h>

/* Logging/debug support */

extern FILE
	*logFile;               /* Log file stream */

struct dbg_struct {
	long	change;		/* Types of changes requested 	*/
	long	set;		/* Use this channel set 	*/
	long	on;		/* Turn these channels on 	*/
	long	off;		/* Turn these channels off 	*/
	long	flip;		/* Invert these channels 	*/
};

#ifndef NOLOG
extern void
	logOpen(),              /* Open a log file */
	logDOpen(),             /* Use a unix descriptor for logging */
	logClose(),		/* Close logging stream */
	fatal();		/* Printf to error loger and exit */

extern int
	log(),			/* Printf to log file */
	ulog();			/* Printf to log file - uncontional */

extern long
	newLogs();
#endif /* ~NOLOG */

extern unsigned long
	dbgEnable;		/* Enabled debug channels */

#define	DBG_LOG		0x0001		/* Local logging channel */
#define	DBG_RLOG	0x0002		/* Remote logging channel */
#define	DBG_VLOG	0x0004		/* Verbose logs */
#define	DBG_PLOG	0x0008		/* Packet logs */
#define DBG_RS232       0x0010          /* RS232 packets */
#define DBG_XPLOG       0x0020          /* log read/write/seek packets */
#define DBG_YPLOG       0x0040          /* log text of read/write packets */

#define PLOG_RCV        0               /* Received packet log */
#define PLOG_XMT        1               /* Transmited packet log */

#ifdef NOLOG
#define	debug(chan, logArgs)
#define logPacket(packet, kind, how)    /* real logPacket()  in logpacket.c */
#define logControl(rmtLogFlag)          /* real logControl() in remotelog.c */
#define logMessage(msgBuf, nBytes)      /* real logMessage() in remotelog.c */
#define logOpen(logBase, logPid)        /* real logOpen()    in log.c */
#define logDOpen(logDesc)               /* real logDOpen()   in log.c */
#define logClose()                      /* real logClose()   in log.c */
#define log(fmt, argVec)                /* real log()        in log.c */
#define ulog(fmt, argVec)               /* real ulog()       in log.c */
#define tlog(fmt, argVec)               /* real tlog()       in log.c */
#define logv(fmt, argVec)               /* real logv()       in log.c */
#define newLogs(logName, namePid, childEnable) /* real newLogs() in log.c */
#define vlog(vLogArgs)
#define Vlog(VLogArgs)
#define dbgOn(onChans)
#define dbgOff(offChans)
#define dbgToggle(togChans)
#define dbgSet(setChans)
#define dbgCheck(checkChans)    0
#else
#ifdef	DEBUG
#define	debug(chan, logArgs)	(((chan) & dbgEnable) ? ulog logArgs : 0)
#else	/* DEBUG! */
#define	debug(chan, logArgs)
#endif	/* !DEBUG */
#define	vlog(vLogArgs)	debug(DBG_VLOG, vLogArgs)
#define Vlog(VLogArgs)  (((DBG_LOG | DBG_VLOG) & dbgEnable) ? tlog VLogArgs:0)
#define	dbgOn(onChans)		(dbgEnable |= (onChans))
#define	dbgOff(offChans)	(dbgEnable &= ~(offChans))
#define	dbgToggle(togChans)	(dbgEnable ^= togChans)
#define	dbgSet(setChans)	(dbgEnable = (setChans))
#define	dbgCheck(checkChans)	(dbgEnable & (checkChans))
#endif /* ~NOLOG */

/*
   Predicate telling whether logs are currently enabled
*/
#define	logsOn()	(dbgEnable && (logFile != (FILE *)NULL))

/*
   Communications from pcidebug program
*/

#define	chanPat	"/tmp/pcichan.%d"	/* Channel file name pattern */

#define	CHG_SET		0x00000001L	/* Types of changes requested */
#define	CHG_ON		0x00000002L	/* Turn channels on */
#define	CHG_OFF		0x00000004L	/* Turn channels off */
#define	CHG_INV		0x00000008L	/* Invert channels */
#define	CHG_CLOSE	0x00000010L	/* Close log file */
#define	CHG_CHILD	0x00000020L	/* Change child's debugs */

#endif	/* !LOG_H */
