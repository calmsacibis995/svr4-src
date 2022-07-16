/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TSPROC_H
#define _SYS_TSPROC_H

#ident	"@(#)head.sys:sys/tsproc.h	1.1.5.1"
/* time-sharing class specific proc structure */

typedef struct tsproc {
	long	ts_timeleft;	/* time remaining in procs quantum */
	short	ts_umdpri;	/* user mode priority within ts class */
	short	ts_upri;	/* user priority */
	short	ts_upovrflw;	/* user requested priority which was out */
				/*  of defined range by this amount */
	char	ts_nice;	/* nice value for compatibility */
	short	ts_dispwait;	/* number of wall clock seconds since start */
				/*   of quantum (not reset upon preemption */
	ushort	ts_flags;	/* flags defined below */
	struct proc *ts_procp;	/* pointer to proc table entry */
	char	*ts_pstatp;	/* pointer to p_stat */
	int	*ts_pprip;	/* pointer to p_pri */
	uint	*ts_pflagp;	/* pointer to p_flag */
	char	*ts_ptimep;	/* pointer to p_time */
	struct cred **ts_pcredpp;	/* pointer to p_cred */
} tsproc_t;

extern tsproc_t	ts_proc[];

/* flags */
#define	TSINUSE	0x0001		/* ts_proc entry in use */
#define	TSKPRI	0x0002		/* proc at kernel mode priority */

#endif	/* _SYS_TSPROC_H */
