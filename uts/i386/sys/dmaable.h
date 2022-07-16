/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/dmaable.h	1.1.2.1"

#define	DMAABLESZ	4096		/* DMA boundary - in clicks - only on 386 */

#define	DO_DMA_CHECK		(tune.t_dmalimit > 0)
#define	DMA_CHECK_ENABLED	(tune.t_dmalimit > 0 && tune.t_dmalimit < pages_end)

#define	DMAABLE_PFN(pfn)	((pfn) < tune.t_dmalimit)
#define	LAST_DMAABLE_PFN	(tune.t_dmalimit)

extern	u_int	dma_freemem;	/* currently free dmaable memory */
extern	u_int	dma_limit_pfn;	/* Last dmaable click (inclusive) */
extern	int	dma_check_on;	/* Tuneable set and memory exceeds DMA capacity */
extern	page_t	*dma_limit_pp;	/* Last dmaable page structure (inclusive) */

extern	page_t	*dma_freelist;	/* Freelist pointer for DMA pages for remapping */
extern	u_int	dmaable_pages;	/* Total free pages on this free list */
extern  u_int	dmaable_free;	/* Current free pages on this free list */
extern	int	dmaable_sleep;	/* Processes sleep on this flag for DMA pages */
extern	mon_t	dma_pagelock;	/* multiprocessor sempahore lock for this freelist */

#define	DMA_PFN(pfn)	((pfn) <= dma_limit_pfn)
#define	DMA_PP(pp)	((pp) <= dma_limit_pp)
#define	DMA_BYTE(b)	(btoct(b) <= dma_limit_pfn)


extern	int	dmaable_rawio();
extern	int	dma_check_strategy();
extern	int	dma_stub_strategy();

struct	dma_checksw {
	int	(*d_strategy)();
	int	d_flags;
};

extern struct dma_checksw	*dma_checksw;

/*
 *	flag values
 */

#define	DMA_NOFLAGS	0x000
#define	DMA_OLDSTYLE	0x001
#define	DMA_NEWSTYLE	0x002
#define	DMA_GENSTRAT	0x004
#define	DMA_BREAKUP	0x008

#define	REASONABLE_DMA_PAGES	100
