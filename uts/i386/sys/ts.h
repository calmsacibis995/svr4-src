/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TS_H
#define _SYS_TS_H

#ident	"@(#)head.sys:sys/ts.h	1.5.5.1"
/*
 * time-sharing dispatcher parameter table entry
 */
typedef struct tsdpent {
	int	ts_globpri;	/* global (class independent) priority */
	long	ts_quantum;	/* time quantum given to procs at this level */
	short	ts_tqexp;	/* ts_umdpri assigned when proc at this level */
				/*   exceeds its time quantum */
	short	ts_slpret;	/* ts_umdpri assigned when proc at this level */
				/*  returns to user mode after sleeping */
	short	ts_maxwait;	/* bumped to ts_lwait if more than ts_maxwait */
				/*  secs elapse before receiving full quantum */
	short	ts_lwait;	/* ts_umdpri assigned if ts_dispwait exceeds  */
				/*  ts_maxwait */				
} tsdpent_t;


/*
 * time-sharing class specific proc structure
 */
typedef struct tsproc {
	long	ts_timeleft;	/* time remaining in procs quantum */
	short	ts_cpupri;	/* system controlled component of ts_umdpri */
	short	ts_uprilim;	/* user priority limit */
	short	ts_upri;	/* user priority */
	short	ts_umdpri;	/* user mode priority within ts class */
	char	ts_nice;	/* nice value for compatibility */
	unsigned char ts_flags;	/* flags defined below */
	short	ts_dispwait;	/* number of wall clock seconds since start */
				/*   of quantum (not reset upon preemption) */
	struct proc *ts_procp;	/* pointer to proc table entry */
	char	*ts_pstatp;	/* pointer to p_stat */
	int	*ts_pprip;	/* pointer to p_pri */
	uint	*ts_pflagp;	/* pointer to p_flag */
	struct tsproc *ts_next;	/* link to next tsproc on list */
	struct tsproc *ts_prev;	/* link to previous tsproc on list */
} tsproc_t;


/* flags */
#define	TSKPRI	0x01		/* proc at kernel mode priority */
#define	TSBACKQ	0x02		/* proc goes to back of disp q when preempted */
#define	TSFORK	0x04		/* proc has forked, so don't reset full quantum */

#endif	/* _SYS_TS_H */
