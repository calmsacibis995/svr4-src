/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_VT_H
#define _SYS_VT_H

#ident	"@(#)head.sys:sys/vt.h	1.1.2.1"

/*
 * definitions for PC AT virtual terminal driver
 */

#define	VTINDEX(dev)	((dev & 0xff) % 15)
#define	VTMAX	0x0f

#define	VT_FORCE	1	/* flag to force a vt switch */
#define	VT_NOFORCE	0	/* dont force a vt switch */
#define	VT_NOSAVE	2	/* On close - don't save screen */

/*
 * Bits in vtty flags word.
 */
#define	VT_ACTIVE	0x0001	/* set if this is the active vt */
#define	VT_PROCMOD	0x0002	/* set if vt has process control mode set */
#define	VT_WAIT		0x0004	/* set if writes to hang if not active vt */
#define	VT_HIDDEN	0x0008	/* set if vt on ring is to be skipped */
#define VT_WAITACT	0x0010	/* set if vt needs wakeup when switched to */

/*
 * all vttys for a device should be circularly linked via the
 * link field of the structure, active vttys are doubly circularly
 * linked via the next and prev fields.
 */
struct vtty {	/* virtual terminal structure */
	struct vtty	*link;	/* next vtty on device */
	struct vtty	*prev;	/* previous vtty in ring */
	struct vtty	*next;	/* next vtty in ring */
	struct vtty	*switchto;	/* vtty to be activated */
	struct	tty	*tp;	/* tty associated with this vtty */
	caddr_t		dsp;	/* device specific info for this vtty */
	struct proc	*procp;	/* ptr to process controlling this vtty */
	short	pid;		/* pid of process controlling this vtty */
	ushort	flags;		/* flags for this vtty */
	char	opencnt;	/* number of opens on this vtty */
	char	vtno;		/* vt index number of this vtty */
	short	relsig;		/* signal to send to request release */
	short	acqsig;		/* signal to send to signal acquisition */
	short	frsig;		/* signal to send on forced removal */
	int	timeid;		/* timeout id for this vtty */
	int	(*dsactivate)(); /* device specific activation routine */
	int	(*dsrefuse)();	/* device specific release refused routine */
	int	(*dsautomode)(); /* device specific switch to auto mode */
};

/*
 * IOCTLs supported by the VT
 */
#define VTIOC		('v'<<8)
#define VT_OPENQRY	(VTIOC|1)	/* inquires if this vt already open */
#define	VT_SETMODE	(VTIOC|2)	/* set vt into auto or process mode */
#define	VT_GETMODE	(VTIOC|3)	/* returns mode vt is currently in */
#define	VT_RELDISP	(VTIOC|4)	/* tells vt when display released */
#define	VT_ACTIVATE	(VTIOC|5)	/* activates specified vt */
#define VT_WAITACTIVE	(VTIOC|6)	/* wait for vt to be activated */
#define VT_GETSTATE	(VTIOC|100)	/* return active and open vts */
#define VT_SENDSIG	(VTIOC|101)	/* send signal to specified vts */

struct vt_mode {
	char	mode;	/* mode to set vt into */
	char	waitv;	/* if != 0, vt hangs on writes when not active */
	short	relsig;	/* signal to use for release request */
	short	acqsig;	/* signal to use for display acquired */
	short	frsig;	/* signal to use for forced release */
};

#define	VT_AUTO		0	/* this vt's switching is automatic */
#define	VT_PROCESS	1	/* this vt's switching controlled by process */
#define VT_ACKACQ	2	/* ack form v86 acquire routine */

/*
 * IOCTL supported by xterm to get vt name
 */
#define TIOCVTNAME	(('T' << 8) | 250)
#define VTNAMESZ	32

/*
 * structure used by VT_GETSTATE and VT_SENDSIG ioctls
 */

struct vt_stat {
	ushort	v_active,
		v_signal,
		v_state;
} ;

#endif	/* _SYS_VT_H */
