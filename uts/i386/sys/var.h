/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_VAR_H
#define _SYS_VAR_H

#ident	"@(#)head.sys:sys/var.h	11.18.3.1"

/*
 * System Configuration Information
 */
struct var {
	int	v_buf;		/* Nbr of I/O buffers.			*/
	int	v_call;		/* Nbr of callout (timeout) entries.	*/
    int v_proc;     /* Max nbr of processes system wide */
    int v_filler;   /* (unused)             */
	int	v_nglobpris;	/* Nbr of global sched prios configured	*/
	int	v_maxsyspri;	/* Max global pri used by sys class.	*/
	int	v_clist;	/* Nbr of clists allocated.		*/
	int	v_maxup;	/* Max number of processes per user.	*/
	int	v_hbuf;		/* Nbr of hash buffers to allocate.	*/
	int	v_hmask;	/* Hash mask for buffers.		*/
	int	v_pbuf;		/* Nbr of physical I/O buffers.		*/
	int	v_sptmap;	/* Size of system virtual space		*/
				/* allocation map.			*/
	int	v_maxpmem;	/* The maximum physical memory to use.	*/
				/* If v_maxpmem == 0, then use all	*/
				/* available physical memory.		*/
				/* Otherwise, value is amount of mem to	*/
				/* use specified in pages.		*/
	int	v_autoup;	/* The age a delayed-write buffer must	*/
				/* be in seconds before bdflush will	*/
				/* write it out.			*/
	int	v_bufhwm;	/* high-water-mark of buffer cache	*/
				/* memory usage, in units of K Bytes
	/* XENIX Support */
	int 	v_scrn;		/* number of multi-screens. (XENIX) 	*/
	int 	v_emap;		/* number of i/o mappings (XENIX) 	*/
	int 	v_sxt;		/* number sxt's for shell layers (XENIX)*/
	int	v_xsdsegs;	/* Number of XENIX shared data segs     */
	int	v_xsdslots;	/* Number of slots in xsdtab[] per seg  */
	/* End XENIX Support */
};

extern struct var v;

#endif	/* _SYS_VAR_H */
