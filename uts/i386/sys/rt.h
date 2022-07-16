/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_RT_H
#define _SYS_RT_H

#ident	"@(#)head.sys:sys/rt.h	1.4.6.1"
/*
 * Real-time dispatcher parameter table entry
 */
typedef struct	rtdpent {
	int	rt_globpri;	/* global (class independent) priority */
	long	rt_quantum;	/* default quantum associated with this level */
} rtdpent_t;


/*
 * Real-time class specific proc structure
 */
typedef struct rtproc {
	long		rt_pquantum;	/* time quantum given to this proc */
	long		rt_timeleft;	/* time remaining in procs quantum */
	short		rt_pri;		/* priority within rt class */
	ushort		rt_flags;	/* flags defined below */
	struct proc	*rt_procp;	/* pointer to proc table entry */
	char		*rt_pstatp;	/* pointer to p_stat */
	int		*rt_pprip;	/* pointer to p_pri */
	uint		*rt_pflagp;	/* pointer to p_flag */
	struct rtproc	*rt_next;	/* link to next rtproc on list */
	struct rtproc	*rt_prev;	/* link to previous rtproc on list */
} rtproc_t;


/* Flags */
#define RTRAN	0x0001		/* process has run since last swap out */
#define RTBACKQ	0x0002		/* proc goes to back of disp q when preempted */


#ifdef _KERNEL
/*
 * Kernel version of real-time class specific parameter structure
 */
typedef struct	rtkparms {
	short	rt_pri;
	long	rt_tqntm;
} rtkparms_t;
#endif	/* _KERNEL */

#endif	/* _SYS_RT_H */
