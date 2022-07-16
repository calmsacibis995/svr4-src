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

#ifndef _VM_TRACE_H
#define _VM_TRACE_H

#ident	"@(#)kern-vm:trace.h	1.3"

/*
 * Trace point definitions.  Beware of conflicts with the
 * window system keyboard translation macros (TR_ASCII et al).
 *
 * The set of active trace points is recorded in the variable
 * tracedevents.  Because we declare this variable as an fd_set,
 * the maximum allowable trace point index is bounded by FD_SETSIZE.
 *
 * For the most part, these trace points are quite specific to SunOS 4.0.
 *
 * The trace point definitions follow stringent conventions for the benefit
 * of postprocessors wishing to manipulate trace information.  The names
 * all start with TR_, and each definition is followed on the next line by
 * a comment whose content is a printf-like formatting string.  Successive
 * format items are fed by successive tr_datum* values from the current
 * trace item.  The only unusual format is %C, which is followed by an angle
 * bracketed, comma separated list of (unquoted) strings, the i-th of which
 * is to be printed when the associated value is i (0-origin).
 *
 * Note that the current trace points confine themselves to the C, d, and
 * x formats so that postprocessors can confine themselves to supporting
 * only these formats.  This situation may change when additional trace
 * points are defined.
 */

/*
 * User-level trace annotation.
 */
#define TR_STAMP		0	/* user: vtrace(VTR_STAMP, value); */
/* "value %d pid %d" */

/*
 * Paging trace points.  The TR_PG_* trace points all start by recording
 * pp, vp, and off values.
 */
#define TR_PG_POCLEAN		1	/* about to clean a page in pageout */
/* "pp %x vp %x off %d hand %d freemem %d" */
#define TR_PG_POFREE		2	/* add page to free list in pageout */
/* "pp %x vp %x off %d hand %d freemem %d" */
#define TR_PG_RECLAIM		3	/* page obtained in page_reclaim */
/* "pp %x vp %x off %d ap %x age %d freemem %d" */
#define TR_PG_ENTER		4 /* page_enter: page assoc. with <vp,off> */
/* "pp %x vp %x off %d retval %d" */
#define TR_PG_ABORT		5 /* page_abort: page disassoc. w/ <vp,off> */
/* "pp %x vp %x off %d code %C<aborted,kept,intrans>" */
#define TR_PG_FREE		6	/* call to page_free */
/* "pp %x vp %x off %d dontneed %d freemem %d code %C<gone,free,cachetl,cachehd>" */
#define TR_PG_ALLOC		7	/* single page allocated in page_get */
/* "pp %x vp %x off %d age %d fromcache %d" */
#define TR_PG_SEGMAP_FLT	8	/* segmap_fault sets p_ref = 1 */
/* "pp %x vp %x off %d softlock %d" */
#define TR_PG_SEGVN_FLT		9	/* segvn_fault sets p_ref = 1 */
/* "pp %x vp %x off %d softlock %d" */
#define TR_PG_PVN_DONE		10	/* pvn_done sets p_gone = 1 */
/* "pp %x vp %x off %d" */
#define TR_PG_PVN_GETDIRTY	11	/* pvn_getdirty sets p_ref = 0 */
/* "pp %x vp %x off %d" */
#define TR_PG_HAT_NEWSEG	12	/* hat_newseg OR's p_ref */
/* "pp %x vp %x off %d referenced %d" */
#define TR_PG_HAT_GETPME	13	/* hat_getpme OR's p_ref */
/* "pp %x vp %x off %d referenced %d" */
#define TR_PG_POREF		14	/* clock hand finds p_ref == 1 */
/* "pp %x vp %x off %d hand %d" */

#define TR_PAGE_GET_SLEEP	19	/* sleep during page alloc request */
/* "bytes %x canwait %d freemem %d after? %d" */
#define TR_PAGE_GET		20	/* page allocation request */
/* "bytes %x canwait %d freemem %d code %C<allocated,toobig,nomem>" */

/*
 * Page abort codes
 */
#define TRC_ABORT_ABORTED	0
#define TRC_ABORT_KEPT		1
#define TRC_ABORT_INTRANS	2

/*
 * Page free codes
 */
#define TRC_FREE_GONE		0
#define TRC_FREE_FREE		1
#define TRC_FREE_CACHETL	2
#define TRC_FREE_CACHEHD	3

/*
 * Page get codes
 */
#define TRC_GET_ALLOCATED	0
#define TRC_GET_TOOBIG		1
#define TRC_GET_NOMEM		2
/*
 * File system buffer tracing points; all trace <dev, bn>
 */
#define TR_BREADHIT		21	/* buffer read found in cache */
/* "" */
#define TR_BREADMISS		22	/* buffer read not in cache */
/* "" */
#define TR_BWRITE		23	/* buffer written */
/* "" */
#define TR_BRELSE		24	/* brelse */
/* "" */

/*
 *  Calibration trace record
 */
#define TR_CALIBRATE		30	/* calibration point in hardclock */
/* "seqnum %d sec %d usec %d" */

/*
 * Paging daemon trace points
 */
#define TR_PAGEOUT		31	/* pageout daemon statistics */
/* "after? %d nscan %d desscan %d freemem %d lotsfree %d" */
#define TR_PAGEOUT_CALL		32	/* pageout daemon call */
/* "callpoint %C<getnpage,getfrlw,schedcpu,schedpag,swdone>" */
#define TR_PAGEOUT_MAXPGIO	33	/* pushes > maxpgio in checkpage */
/* "freemem %d nscan %d" */
#define TR_PAGEOUT_WRAP		34	/* front clock hand wrap around */
/* "freemem %d hand %d" */
/*
 * Pageout daemon call location codes
 */
#define TRC_POCALL_GETNPAGE	0
#define TRC_POCALL_GETFRLW	1
#define TRC_POCALL_SCHEDCPU	2
#define TRC_POCALL_SCHEDPAG	3
#define TRC_POCALL_SWDONE	4

/*
 * Mapping of objects to vnode and <vnode, offset> values
 */
#define TR_MP_SWAP		40	/* allocate a page of swap */
/* "vp %x off %d ap %x" */
#define TR_MP_LNODE		41	/* allocate an lnode */
/* "vp %x lvp %x" */
#define TR_MP_SNODE		42	/* allocate an snode */
/* "vp %x dev %x makespecvp %d" */
#define TR_MP_INODE		43	/* allocate an inode */
/* "vp %x dev %x inode %x" */
#define TR_MP_RNODE		44	/* allocate an rnode */
/* "vp %x fsid %x %x len %x fid %x %x" */
#define TR_MP_TRUNC0		45	/* truncate file to zero length */
/* "vp %x" */
#define TR_MP_TRUNC		46	/* truncate file */
/* "vp %x newlen %d oldlen %d" */

/*
 * Segment trace points
 */
#define TR_SEG_GETMAP		50	/* get a segmap */
/* "seg %x addr %d type %C<kseg,segkmap,anon,file,unk> vp %x off %d" */
#define TR_SEG_RELMAP		51	/* release a segmap */
/* "seg %x addr %d type %C<kseg,segkmap,anon,file,unk> refcnt %d" */
#define TR_SEG_ALLOCPAGE	52	/* call to rm_allocpage */
/* "seg %x addr %d type %C<kseg,segkmap,anon,file,unk> vp %x off %d pp %x" */
#define TR_SEG_GETPAGE		53	/* call to VOP_GETPAGE */
/* "seg %x addr %d type %C<kseg,segkmap,anon,file,unk>" */
#define TR_SEG_KLUSTER		54	/* call to pvn_kluster */
/* "seg %x addr %d readahead %d" */

/*
 * Segment type codes
 */
#define TRC_SEG_KSEG		0
#define TRC_SEG_SEGKMAP		1
#define TRC_SEG_ANON		2
#define TRC_SEG_FILE		3
#define TRC_SEG_UNK		4

/*
 * Process trace points
 */
#define TR_PR_WAKEUP		60	/* before call to wakeup */
/* "ptime %d cpu %d stime %d runout %d" */

/*
 * Swapper trace points
 */
#define TR_SWAP_LOOP		70	/* at loop: in sched */
/* "mapwant %d avenrun %d avefree %d avefree30 %d pginrate %d pgoutrate %d" */
#define TR_SWAP_OUT		71	/* call to swapout */
/* "proc %x hardwap? %d freed %d" */
#define TR_SWAP_SLEEP		72	/* sched calls sleep() */
/* "chan %x runin %d runout %d wantin %d" */
#define TR_SWAP_IN_CHECK	73	/* decision point on calling swapin */
/* "proc %x freemem %d deficit %d needs %d outpri %d maxslp %d" */
#define TR_SWAP_OUT_CHECK	74	/* decision point on calling swapout */
/* "proc %x sleeper %d desperate %d deservin %d inpri %d maxslp %d" */
#define TR_SWAP_IN		75	/* call to swapin */
/* "proc %x taken %d" */
#define TR_SWAP_OUT_CHECK0	76	/* decision point on calling swapout */
/* "proc %x freemem %d rssize %d slptime %d maxslp %d swappable %x" */

/*
 * ufs trace points
 */
#define TR_UFS_RWIP		80	/* call to rwip */
/* "inode %x uio %x rw %x ioflag %d offset %d location %C<enter,getmap,bmapalloc,uiomove,release,iupdat,return>" */

/*
 * rwip location type codes
 */
#define TRC_RWIP_ENTER		0
#define TRC_RWIP_GETMAP		1
#define TRC_RWIP_BMAPALLOC	2
#define TRC_RWIP_UIOMOVE	3
#define TRC_RWIP_RELEASE	4
#define TRC_RWIP_IUPDAT		5
#define TRC_RWIP_RETURN		6

/*
 * Memory allocator trace points; all trace the amount of memory involved.
 */

/*
 * System call trace points.
 */

/*
 * Parameters needed for trace post-processing.  Be careful to keep this
 * up-to-date.  MAXTRACEID is the highest number used for a trace id.
 * MAXTRACECODE is the highest number used to represent a code, for example
 * the codes for TR_PG_FREE are gone, free, cachetl, and cachehd.  These are
 * represented as 0, 1, 2, and 3, respectively.  If 3 were the highest code
 * used for any trace point, that would be the value of MAXTRACECODE.

 */
#define MAXTRACEID	TR_UFS_RWIP
#define MAXTRACECODE	TRC_RWIP_RETURN

/*
 * Generic format for data saved with trace calls.
 *
 * The format of tr_time varies depending on whether or not there's
 * a high resolution timer available.  If so, it's the timer's value;
 * if not, it's the low 16 bits of hrestime.tv_sec concatenated to the
 * high 16 bits of hrestime.tv_usec.  Tr_pid records the process active
 * at the time of the trace call; it's not meaningful if called from
 * interrupt level.
 */
struct trace_rec {
	u_long	tr_time;
	short	tr_tag;
	u_short	tr_pid;
	u_long	tr_datum0;
	u_long	tr_datum1;
	u_long	tr_datum2;
	u_long	tr_datum3;
	u_long	tr_datum4;
	u_long	tr_datum5;
};



/*
 * Requests for the vtrace() system call.
 */
#define VTR_DISABLE	0		/* trace specified events */
#define VTR_ENABLE	1		/* don't trace specified events */
#define VTR_VALUE	2		/* return currently-traced events */
#define VTR_STAMP	3		/* cause TR_STAMP event */
#define VTR_RESET	4		/* reset eventstraced to zero */

#ifdef	_KERNEL
#ifdef	TRACE

extern fd_set	tracedevents;
extern u_int	tracebufents;
extern u_int	eventstraced;
extern struct	trace_rec *tracebuffer;
extern void	inittrace();
extern void	resettracebuf();
extern void	traceit();

#define pack(a, b)	((a)<<16)|(b)

/*
 * Lint doesn't believe that there are valid reasons for comparing
 * constants to each other...
 */
#ifdef	lint
#define trace(ev, d0, d1, d2, d3, d4, d5) \
	if (FD_ISSET((ev), &tracedevents)) \
		traceit((ev), (u_long)(d0), (u_long)(d1), (u_long)(d2), \
			(u_long)(d3), (u_long)(d4), (u_long)(d5))
#else	/* lint */
#define trace(ev, d0, d1, d2, d3, d4, d5) \
	if ((u_int)(ev) < FD_SETSIZE && FD_ISSET((ev), &tracedevents)) \
		traceit((ev), (u_long)(d0), (u_long)(d1), (u_long)(d2), \
			(u_long)(d3), (u_long)(d4), (u_long)(d5))
#endif	/* lint */

#define trace6(ev, d0, d1, d2, d3, d4, d5) \
	trace(ev, d0, d1, d2, d3, d4, d5)
#define trace5(ev, d0, d1, d2, d3, d4)	trace(ev, d0, d1, d2, d3, d4, 0)
#define trace4(ev, d0, d1, d2, d3)	trace(ev, d0, d1, d2, d3, 0, 0)
#define trace3(ev, d0, d1, d2)		trace(ev, d0, d1, d2, 0, 0, 0)
#define trace2(ev, d0, d1)		trace(ev, d0, d1, 0, 0, 0, 0)
#define trace1(ev, d0)			trace(ev, d0, 0, 0, 0, 0, 0)

#else	/* TRACE */

#define pack(a, b)

#define trace	trace6
#define trace6(ev, d0, d1, d2, d3, d4, d5)
#define trace5(ev, d0, d1, d2, d3, d4)
#define trace4(ev, d0, d1, d2, d3)
#define trace3(ev, d0, d1, d2)
#define trace2(ev, d0, d1)
#define trace1(ev, d0)

#endif	/* TRACE */
#endif	/* _KERNEL */

#endif	/* _VM_TRACE_H */
