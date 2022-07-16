/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _VM_VM_HAT_H
#define _VM_VM_HAT_H

#ident	"@(#)kern-vm:vm_hat.h	1.3.1.1"

#include "sys/immu.h"

/*
 * VM - Hardware Address Translation management.
 *
 * This file describes the contents of the machine specific
 * hat data structures and the machine specific hat procedures.
 * The machine independent interface is described in <vm/hat.h>.
 */

/* The 386 HAT design is based on dividing a page table into
 * chunks that are a power of 2 long.
 * These chunks allow the mapping pointer overhead to be
 * reduced for the typical small process.
 * The number of chunks times the number of entries equals 1024.
 * The initial design uses 32 chunks of 32 entries.
 * If translation caching (stealable translation accounting)
 * is implemented, this may change to 16 chunks of 64 entries.
 * This allows four accounting words to be available per chunk
 * (instead of 1 word) in the accounting chunk (chunk 0) of
 * a mapping chunk page.
 * The extra words would be used for a time stamp and pointers
 * for a doubly linked list.
 * To acommodate this possible change, the following defines exist.
 */
#define HAT_MCPP	32	/* number of mapping chunks per page */
#define HAT_EPMC	32	/* number of entries per mapping chunk */
typedef union hatmap {
	uint hat_mapv;
	union hatmap *hat_mnext;
} hatmap_t;

typedef struct hatmc {
	hatmap_t hat_map[HAT_EPMC];
} hatmc_t;

/* 
 * The hatmcp_t:
 */
typedef struct {
	hatmc_t *hat_mcp;
} hatmcp_t;

#define HATMAP_ADDR	0xFFFFFF80

/* The hatpt structure is the machine dependent page table accounting
 * structure internal to the 386 HAT layer.
 * It links an address space to a currently resident page table
 * and the currently resident mapping pointer chunks for that
 * page table.
 * It contains an active PTE count and all locking information.
 * It also contains hatpt pointers for a doubly linked, circular
 * linked list of active page tables for an address space.
 */
typedef struct hatpt {
	struct	hatpt *hatpt_forw;	/* forward page table list ptr */
	struct	hatpt *hatpt_back;	/* backward page table list ptr */
	struct	hatpt *hatpt_next;	/* forward activept list ptr */
	struct	hatpt *hatpt_prev;	/* backward activept list ptr */
	pte_t	hatpt_pde;		/* PDE for page table */
	pte_t	*hatpt_pdtep;		/* PDT entry pointer */
	struct	as *hatpt_as;		/* pointer back to containing as */
	cnt_t	hatpt_aec;		/* active entry count */
	cnt_t	hatpt_locks;		/* count of locked PTEs */
	hatmcp_t hatpt_mcp[HAT_MCPP];	/* mapping chunk pointer array */
} hatpt_t;

/*
 * The hat structure is the machine dependent hardware address translation
 * structure kept in the address space structure to show the translation.
 */
typedef struct hat {
	struct	hatpt *hat_pts;		/* current page table list */
	struct	hatpt *hat_ptlast;	/* last page table to fault */
} hat_t;

typedef struct hatpgtc {
	pte_t	hat_pte[HAT_EPMC];
} hatpgtc_t;

/* The mapping chunk page declarations:
 */

typedef union {
	struct {
		uint hat_mcaec	:  6,	/* active PTE count for chunk */
				:  1,	/* spare bit */
		     hat_ptcndx	: 25;	/* high order bits of pointer
					 * to page table chunk.  Zeroed
					 * low order bits completes
					 * the pointer.
					 */
	} ptp;
	uint hat_ptpv;
	hatpgtc_t *hat_pgtcp;
} hatptcp_t;

#define HATPTC_ADDR	0xFFFFFF80


typedef union hatmpga {
	uint	hat_mpgabits;		/* mapping chunk page allocation bits.
					 * This may get changed if translation
					 * cache needs it.
					 */
	hatptcp_t hatptcp[HAT_MCPP];	/* pointers to the page table chunks
					 * for the 31 (1-31) mapping chunks
					 * in the page.
					 * Slot 0 corresponds to the hatmpga
					 * data, so it is available for other use.
					 */
} hatmpga_t;

#define HATMCFULL	0xFFFFFFFF

typedef union hatmcpg {
	hatmpga_t hat_mcpga;
	hatmc_t hat_mc[HAT_MCPP];
} hatmcpg_t;


typedef struct hatpgt {
	hatpgtc_t hat_pgtc[HAT_MCPP];
} hatpgt_t;

/* Hat-specific parts of a page structure:
   struct phat is always valid, struct phat2 is overlaid with p_dblist[]. */

struct phat {
	caddr_t mappings;	/* List of mappings (translations) for this
				   page; must be zero if no mappings. */
};

struct phat2 {
	union {
		hatpt_t *ptap;
		hatmcpg_t *mcpgp;
	} ph_use;
};

#define phat_ptap	p_hat2.ph_use.ptap
#define phat_mcpgp	p_hat2.ph_use.mcpgp

/*
 * Flags to pass to hat_ptalloc().
 *
 * NOTE: HAT_NOSLEEP and HAT_CANWAIT must match up with
 *       P_NOSLEEP and P_CANWAIT (respectively) in vm_page.h
 */
#define	HAT_NOSLEEP	0	/* return immediately if no memory */
#define HAT_CANWAIT	1	/* wait if no memory currently available */
#define HAT_NOSTEAL	2	/* don't steal a PT from another process */	

/*
 * Flags to pass to hat_ptsteal().
 */
#define HAT_PTUSED	0	/* stolen PT will be reused immediately */
				/* called from hat_ptalloc() */
#define HAT_PTFREE	1	/* stolen PT will be freed, */
				/* called from hat_mcalloc() */


#define HATMCNOSHFT	17	/* PNUMSHFT + LOG2(HAT_EPMC) */
#define HATMCNOMASK	(HAT_MCPP-1)
#define HATMCNDXSHFT	PNUMSHFT
#define HATMCNDXMASK	(HAT_EPMC-1)
#define MAPMCNOSHFT	7	
#define MAPMCNDXSHFT	2
#define HATMCNO(v)	(((uint)(v) >>HATMCNOSHFT) & HATMCNOMASK)
#define HATMCNDX(v)	(((uint)(v) >>HATMCNDXSHFT) & HATMCNDXMASK)
#define HATMAPMCNO(map)	(((uint)(map) >>MAPMCNOSHFT) & HATMCNOMASK)
#define HATMAPMCNDX(map)(((uint)(map) >>MAPMCNDXSHFT) & HATMCNDXMASK)
#define HATMCSIZE	(1 << HATMCNOSHFT)
#define HATVMC_ADDR	0xFFFE0000

#ifdef _KERNEL

extern void restorepd();

#endif /* _KERNEL */

/* some HAT-specific macros: */

/* obtain the virtual address that maps the page frame number in a pte */
#define ptetokv(pte)	(xphystokv((pte) & PG_ADDR))

/* from a mapping chunk pointer to the address of mapping chunk */
#define mcptomapp(mcp)	((hatmap_t *)((mcp)->hat_mcp))

/* from a mapping pointer to the page table chunk pointer */
#define mapptoptcp(mapp) ((hatptcp_t *)((uint)(mapp) & PG_ADDR) \
				+ HATMAPMCNO(mapp))

/* from a mapping pointer to the page table entry pointer */
#define mapptoptep(mapp) ((pte_t *)(((hatptcp_t *)((uint)(mapp) & PG_ADDR) \
			  + HATMAPMCNO(mapp))->hat_ptpv & HATPTC_ADDR) \
			  + HATMAPMCNDX(mapp))

/* obtain page table chunk point from ptcp */
#define ptcptoptep(ptcp) (pte_t *)(ptcp->hat_ptpv & HATPTC_ADDR) 

#define	APPEND_PT(PT, LIST)	{					\
				(LIST).hatpt_prev->hatpt_next = PT;	\
				(PT)->hatpt_prev = (LIST).hatpt_prev;	\
				(PT)->hatpt_next = &LIST;			\
				(LIST).hatpt_prev = PT;			\
			}

#define	PREPEND_PT(PT, LIST)	{					\
				(LIST).hatpt_next->hatpt_prev = PT;	\
				(PT)->hatpt_next = (LIST).hatpt_next;	\
				(PT)->hatpt_prev = &LIST;			\
				(LIST).hatpt_next = PT;			\
			}

#define	REMOVE_PT(PT)	{						\
			(PT)->hatpt_prev->hatpt_next = (PT)->hatpt_next;	\
			(PT)->hatpt_next->hatpt_prev = (PT)->hatpt_prev;	\
			}

#endif	/* _VM_VM_HAT_H */

