#ident	"@(#)immu.h	1.2	92/03/01	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_IMMU_H
#define _SYS_IMMU_H

#ident	"@(#)head.sys:sys/immu.h	11.18.4.1"

/*
 * Page Table Entry Definitions
 */

typedef union pte {    /*  page table entry  */

/*                                                 */
/*                        DEBUG                    */
/*                        only                     */
/*  +---------------------+---+---+-+-+---+-+-+-+  */
/*  |        pfn          |tag|   |m|r|   |u|r|p|  */
/*  +---------------------+---+---+-+-+---+-+-+-+  */
/*            20            3   2  1 1  2  1 1 1   */
/*                                                 */
	struct {
		uint pg_v	:  1,	/* Page is valid, i.e. present */
		     pg_rw	:  1,	/* read/write */
		     pg_us	:  1,	/* user/supervisor */
		     		:  2,	/* Reserved by hardware.	*/
		     pg_ref	:  1,	/* Page has been referenced */
		     pg_mod	:  1,	/* Page has been modified */
				:  2,	/* Reserved by hardware.	*/
		     pg_tag	:  3,	/* Unused software bits.	*/
		     pg_pfn	: 20;	/* Physical page frame number	*/
	} pgm;
	uint	pg_pte;		/* Full page table entry	*/
} pte_t;

/*
 *	Page Table
 */

#define NPGPT		1024	/* Nbr of pages per page table (seg). */

typedef struct ptbl {
	int page[NPGPT];
} ptbl_t;

/* Page table entry dependent constants */

#define	NBPP		4096		/* Number of bytes per page */
#define	NBPPT		4096		/* Number of bytes per page table */
#define	BPTSHFT		12 		/* LOG2(NBPPT) if exact */
#define	NPTPP		1		/* Nbr of page tables per page.	*/
#define	NPTPPSHFT	0		/* Shift for NPTPP. */

/* Following added because svid says ulimit works in 512 byte units. so we must
  have something independent of the blocksize of the file system implementation
 */
#define NUPP		8		/* Number ulimit blocks per page    */
#define UPPSHFT		3		/* Shift for ulimit blocks per page */

#define PNUMSHFT	12		/* Shift for page number from addr. */
#define PNUMMASK	0x3FF		/* Mask for index in page table */
#define POFFMASK        0xFFF		/* Mask for offset into page. */
#define	PTOFFMASK	0x3FF		/* Mask for offset into page table/dir*/
#define	PNDXMASK	PTOFFMASK	/* Mask for offset into kptbl.*/
#define PGFNMASK	0xFFFFF		/* Mask page frame nbr after shift. */
#define PTNUMSHFT	22		/* Shift for page table num from addr */
#define PTSIZE		4096		/* Page table size in bytes */
#define	PTMASK		(PTSIZE - 1)	/* Mask for page table size */
#define VPTSIZE		(1<<PTNUMSHFT)	/* Virtual bytes described by	*/
					/* a page table.		*/

/* Page descriptor (table) entry field masks */

#define PT_ADDR		0xFFC00000	/* physical page table address */
#define PG_ADDR		0xFFFFF000	/* physical page address */
#ifdef DEBUG
#define PG_LOCKCNT	0x00000E00	/* mapping SOFTLOCK count (software) */
#endif
#define PG_M		0x00000040	/* modify bit */
#define PG_REF		0x00000020	/* reference bit */
#define	PG_PCD		0x00000010	/* 1=Page Cache Disable */
#define	PG_PWT		0x00000008	/* 1=Page Write Transparent */
#define	PG_US		0x00000004	/* 0=supervisor, 1=user */
#define	PG_RW		0x00000002	/* 0=read-only, 1=read/write */
#define PG_V		0x00000001	/* page valid bit */
#define PG_P		PG_V		/* for source compatibility */
#define PTE_RW		(PG_RW|PG_US)
#define PTE_PROTMASK	PTE_RW

#define O_PG_LOCK	0x00000800	/* OBSOLETE; for binary compatibility */
#define PG_LOCK		0		/* OBSOLETE; for source compatibility */

#define XMEM_BIT	0x80000000	/* bit to indicate 2GB extra mem */

/* The page number within a section. */

#define ptnum(X)	((uint)(X) >> PTNUMSHFT)

#define pgndx(x)	(((x) >> PNUMSHFT) & PNDXMASK)

/* Round up page table address */

#define ptround(p)	((int *) (((uint)(p) + PTSIZE-1) & ~(PTSIZE-1)))

/* Round down page table address */

#define pttrunc(p)	((int *) ((uint)(p) & ~(PTSIZE-1)))
#define ptalign(p)	pttrunc(p)

#define pnum(X)  	(((uint)(X) >> PNUMSHFT) & PTOFFMASK) 
#define pfnum(X)	(((uint)(X) >> PNUMSHFT) & PGFNMASK)
#define PFNUM(X)	pfnum(X)

/* Following added because svid says ulimit works in 512 byte units, so we must
   have something independent of the blocksize of the file system implementation

	Ulimit blocks (512 bytes each) and pages.
 */

#define utop(UU)	(((UU) + NUPP -1) >> UPPSHFT)

/* Page tables (1024 entries == 4k bytes) to pages. */

#define	pttopgs(x)	(((x) + NPTPP - 1) >> NPTPPSHFT)
#define	pttob(x)	((x) << BPTSHFT)
#define	btopt(x)	(((x) + NBPPT - 1) >> BPTSHFT)

/* Form page table entry from modes and page frame number */

#define	mkpte(mode,pfn)	(mode | ((uint)(pfn) << PNUMSHFT))

/*	The following macros are used to set/check the value
 *	of the bits in a page descriptor (table) entry 
 *
 *	Atomic instruction is available to clear the present bit,
 *	other bits are set or cleared in a word operation.
 */

#define PG_ISVALID(pte) 	((pte)->pgm.pg_v)
#define PG_SETVALID(pte)	((pte)->pg_pte |= PG_V)
#define PG_CLRVALID(pte)	((pte)->pg_pte &= ~PG_V)

#define PG_SETMOD(pte)   	((pte)->pg_pte |= PG_M)	
#define PG_CLRMOD(pte)   	((pte)->pg_pte &= ~PG_M)	

#define PG_SETREF(pte)    	((pte)->pg_pte |= PG_REF)
#define PG_CLRREF(pte)    	((pte)->pg_pte &= ~PG_REF)

#define PG_ISWRITEABLE(pte)	((pte)->pgm.pg_rw)
#define PG_CLRW(pte)		((pte)->pg_pte &= ~(PG_RW))

#define PG_SETPROT(pte,b)	((pte)->pg_pte |= b)	/* Set r/w access */
#define PG_CLRPROT(pte)		((pte)->pg_pte &= ~(PTE_PROTMASK))

#ifdef DEBUG

/*
 * Macros to keep track of SOFTLOCKs on a mapping.
 */

#define PG_ISLOCKED(pte)	((pte)->pgm.pg_tag)
#define PG_SETLOCK(pte)		{ \
	if ((pte)->pgm.pg_tag == 7) \
		cmn_err(CE_PANIC, "Too many SOFTLOCKs on mapping !\n"); \
	++(pte)->pgm.pg_tag; \
}
#define PG_CLRLOCK(pte)	{ \
	if ((pte)->pgm.pg_tag == 0) \
		cmn_err(CE_PANIC, "SOFTUNLOCK on an unlocked mapping !\n"); \
	--(pte)->pgm.pg_tag; \
}

#else

#define PG_ISLOCKED(pte)
#define PG_SETLOCK(pte)
#define PG_CLRLOCK(pte)

#endif

#define SOFFMASK	0x3FFFFF	/* Mask for page table alignment */
#define SGENDMASK	0x3FFFFC	/* Mask for page table end alignment */

/*  access modes  */

#define KNONE  (unsigned char)  0x00
#define KEO    (unsigned char)  0x40	/* KRO on WE32000	*/
#define KRE    (unsigned char)  0x80
#define KRWE   (unsigned char)  0xC0	/* KRW on WE32000	*/

#define UNONE  (unsigned char)  0x00
#define UEO    (unsigned char)  0x01	/* URO on WE32000	*/
#define URE    (unsigned char)  0x02
#define URWE   (unsigned char)  0x03	/* URW on WE32000	*/

#define UACCESS (unsigned char) 0x03
#define KACCESS (unsigned char) 0xC0

#define SEG_RO	(KRWE|URE)
#define SEG_RW	(KRWE|URWE)


#define PAGNUM(x)   (((uint)(x) >> PNUMSHFT) & PNUMMASK)
#define PAGOFF(x)   (((uint)(x)) & POFFMASK)


/*	The following variables describe the memory managed by
**	the kernel.  This includes all memory above the kernel
**	itself.
*/

extern int	syssegs[];	/* Start of the system segment	*/
				/* from which kernel space is	*/
				/* allocated.  The actual value	*/
				/* is defined in the vuifile.	*/
extern pte_t	*kptbl;		/* Kernel page table.  Used to	*/
				/* map sysseg.			*/
extern pte_t	*usertable;	/* Common page table.  Used to	*/
				/* map the current ublock.	*/
extern int	maxmem;		/* Maximum available free	*/
				/* memory.			*/
extern int	freemem;	/* Current free memory.		*/
extern int	availrmem;	/* Available resident (not	*/
				/* swapable) memory in pages.	*/
extern int	availsmem;	/* Available swapable memory in	*/
				/* pages.			*/

/*	Conversion macros
*/

/*	Get page number from system virtual address.  */


/*	Between kernel virtual address and physical page frame number.
*/

#define svtop(vaddr)	((((unsigned long) (vaddr) >= KVXBASE) ? \
			((vaddr) - KVXBASE) + X_MEMBIT : (vaddr) - KVBASE) \
			>> PNUMSHFT)

/*	Get system virtual address from page number.  */

#define ptosv(pfn)   (phystokv((pfn) << PNUMSHFT))


/*	These macros are used to map between kernel virtual
**      and physical address.
*/

extern paddr_t	svirtophys(/* va */);

#define phystokv(paddr) xphystokv(paddr)
 
/*	The xphystokv() macro works for all physical addresses
**	which correspond to main (RAM) memory.  This differs from
**	phystokv() in that it can handle the Olivetti's extra 384K.
**	Normally, this isn't an issue, since this 384K is used for
**	kernel text.
*/

#define xphystokv(paddr) (((unsigned long)(paddr) & XMEM_BIT) ? \
		(paddr) + (KVXBASE - XMEM_BIT) : (paddr) + KVBASE)

/*	Between kernel virtual address and physical page frame number.
*/

#define phystopfn(paddr)	((u_int)(paddr) >> PNUMSHFT)
#define pfntophys(pfn)  	((pfn) << PNUMSHFT)
#define kvtopfn(vaddr) 		(kvtophys(vaddr) >> PNUMSHFT)
#define pfntokv(pfn)   		(phystokv ((pfn) << PNUMSHFT))

/*	Between kernel virtual addresses and the kernel page
**	table.
*/

#define	kvtokptbl(x)	(&kptbl[pgndx((uint)(x) - (uint)syssegs)])

/*
 * pte_t *
 * vatopdte(v)
 * returns the page directory entry location of v.
 */

#define	vatopdte(v)	(&kpd0[ptnum(v)])

/*
 * pte_t *
 * vatopte(v, pdte)
 * returns the page table entry location of v.
 */

#define	vatopte(v, pdte) ((pte_t *)phystokv(ctob(pdte->pgm.pg_pfn)) + pnum(v))

/*
 * pte_t *
 * svtopte(v)
 * returns the pt entry location of v.
 *
 * This macro works only with paged virtual address.
 *
 */

#define svtopte(v) ((pte_t *)phystokv(ctob((uint)(vatopdte(v)->pgm.pg_pfn))) + pnum(v))

/*
 * svtopfn(v)
 */

#define svtopfn(v) (PAGNUM(svirtophys(v)))

/*	Page frame number to kernel pte.
*/

#define	pfntokptbl(p)	(kvtokptbl(pfntokv(p)))

/*	Convert segment:offset 8086 far pointer to address
*/

#define	ftop(x)	((((x) & 0xffff0000) >> 12) + ((x) & 0xffff))


/* flags used in ptmemall() call
*/

#define PHYSCONTIG 02
#define NOSLEEP    01


/*	Declarations for kernel variables
*/

#ifdef _KERNEL

extern pte_t	kpd0[];		/* Global system page directory */

#endif /* _KERNEL */


/*
 *  User address space offsets
 *
 *****************************  NOTE - NOTE  *********************************
 *
 *	 ANY CHANGES IN THE FOLLOWING DEFINES NEED TO BE REFLECTED IN
 *	    EITHER ml/misc.s, OR ml/ttrap.s, OR BOTH.
 */

#define UVBASE          ((unsigned)0x00000000L)     /* main store virtual address    */
#define UVSTACK         ((unsigned)0x7FFFFFFCL)     /* stack bottom virtual address  */
#define UVSHM           ((unsigned)0x80000000L)     /* Shared memory address         */
#define KVBASE          ((unsigned)0xC0000000L)     /* base of kernel memory map     */
#define KVXBASE         ((unsigned)0xC8000000L)     /* base for extended memory      */
#define KVSBASE         ((unsigned)0xD0000000L)     /* base for kernel text, data + bss */
#define UVUBLK          ((unsigned)0xE0000000L)     /* ublock virtual address        */

#define UVTEXT          UVBASE          /* beginning addrss of user text    */
#define UVEND		KVBASE		/* end of user virtual address range */
#define MINUVADR        UVTEXT          /* minimum user virtual address.    */
#define MAXUVADR        KVBASE          /* maximum user virtual address.    */
#define MINKVADR        KVBASE          /* minimum kernel virtual address.  */
#define MAXKVADR        UVUBLK          /* maximum kernel virtual address.  */

#define KADDR(v)        ((v) >= MINKVADR)

#define	SEL_RPL		0x03

#endif	/* _SYS_IMMU_H */
