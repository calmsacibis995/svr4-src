/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ifndef _SYS_VMMETER_H
#define _SYS_VMMETER_H

#ident	"@(#)head.sys:sys/vmmeter.h	1.5.3.1"

/*
 * Virtual memory related instrumentation
 */

/*
 * Note that all the vmmeter entries between v_first and v_last
 *  must be unsigned [int], as they are used as such in vmmeter().
 */
struct vmmeter {
#define	v_first	v_swtch
	unsigned v_swtch;	/* context switches */
	unsigned v_trap;	/* calls to trap */
	unsigned v_syscall;	/* calls to syscall() */
	unsigned v_intr;	/* device interrupts */
	unsigned v_pdma;	/* pseudo-dma interrupts XXX: VAX only */
	unsigned v_pswpin;	/* pages swapped in */
	unsigned v_pswpout;	/* pages swapped out */
	unsigned v_pgin;	/* pageins */
	unsigned v_pgout;	/* pageouts */
	unsigned v_pgpgin;	/* pages paged in */
	unsigned v_pgpgout;	/* pages paged out */
	unsigned v_intrans;	/* intransit blocking page faults */
	unsigned v_pgrec;	/* total page reclaims (includes pageout) */
	unsigned v_xsfrec;	/* found in free list rather than on swapdev */
	unsigned v_xifrec;	/* found in free list rather than in filsys */
	unsigned v_exfod;	/* pages filled on demand from executables */
			/* XXX: above entry currently unused */
	unsigned v_zfod;	/* pages zero filled on demand */
	unsigned v_vrfod;	/* fills of pages mapped by vread() */
			/* XXX: above entry currently unused */
	unsigned v_nexfod;	/* number of exfod's created */
			/* XXX: above entry currently unused */
	unsigned v_nzfod;	/* number of zfod's created */
			/* XXX: above entry currently unused */
	unsigned v_nvrfod;	/* number of vrfod's created */
			/* XXX: above entry currently unused */
	unsigned v_pgfrec;	/* page reclaims from free list */
	unsigned v_faults;	/* total page faults taken */
	unsigned v_scan;	/* page examinations in page out daemon */
	unsigned v_rev;		/* revolutions of the paging daemon hand */
	unsigned v_seqfree;	/* pages taken from sequential programs */
			/* XXX: above entry currently unused */
	unsigned v_dfree;	/* pages freed by daemon */
	unsigned v_fastpgrec;	/* fast reclaims in locore XXX: VAX only */
#define	v_last v_fastpgrec
	unsigned v_swpin;	/* swapins */
	unsigned v_swpout;	/* swapouts */
};

#ifdef _KERNEL
/*
 *	struct	v_first to v_last		v_swp*
 *	------	-----------------		------
 *	cnt	1 second interval accum		5 second interval accum
 *	rate	5 second average		previous interval
 *	sum			free running counter
 */
struct	vmmeter cnt, rate, sum;
#endif

/*
 * Systemwide totals computed every five seconds.
 * All these are snapshots, except for t_free.
 */
struct vmtotal {
	short	t_rq;		/* length of the run queue */
	short	t_dw;		/* jobs in ``disk wait'' (neg priority) */
	short	t_pw;		/* jobs in page wait */
	short	t_sl;		/* ``active'' jobs sleeping in core */
	short	t_sw;		/* swapped out ``active'' jobs */
	int	t_vm;		/* total virtual memory */
			/* XXX: above entry currently unused */
	int	t_avm;		/* active virtual memory */
			/* XXX: above entry currently unused */
	short	t_rm;		/* total real memory in use */
	short	t_arm;		/* active real memory */
	int	t_vmtxt;	/* virtual memory used by text */
			/* XXX: above entry currently unused */
	int	t_avmtxt;	/* active virtual memory used by text */
			/* XXX: above entry currently unused */
	short	t_rmtxt;	/* real memory used by text */
			/* XXX: above entry currently unused */
	short	t_armtxt;	/* active real memory used by text */
			/* XXX: above entry currently unused */
	short	t_free;		/* free memory pages (60 second average) */
};

#ifdef _KERNEL
struct	vmtotal total;
#endif

#endif	/* _SYS_VMMETER_H */
