/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:startup.c	1.4.1.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/seg.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/buf.h"
#include "sys/reg.h"
#include "sys/utsname.h"
#include "sys/tty.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"

#include "sys/cred.h"
#include "sys/disp.h"
#include "sys/class.h"
#include "sys/mman.h"

#include "sys/vnode.h"
#include "sys/session.h"
#include "sys/kmem.h"

#include "sys/file.h"
#include "sys/uio.h"
#include "sys/conf.h"

#include "sys/bootinfo.h"

#include "vm/vm_hat.h"
#include "vm/seg.h"
#include "vm/seg_kmem.h"
#include "vm/seg_vn.h"
#include "vm/seg_u.h"
#include "vm/seg_map.h"
#include "vm/page.h"
#include "vm/bootconf.h"
#include "vm/faultcatch.h"

#ifdef AT386	/* 16 MB DMA */
#include "sys/dmaable.h"
#endif		/* 16 MB DMA */

#ifdef AT386
#include "sys/at_ansi.h"
#include "sys/kd.h"
#endif /*AT386*/
#ifdef	EISA
#include "sys/eisa.h"
#endif	/* EISA */

extern int availkmem;
short	prt_where = PRW_CONS;
extern dev_t	rootdev;
extern dev_t	swapdev;
extern pte_t *piownptbl;
extern int	lotsfree;
extern int	syssegsz;
extern int	piosegsz;
extern int	segmapsz;
extern char piosegs[];
#ifdef	EISA
int eisa_enable = 0;
#endif	/* EISA */

#define	TSSBITMASK	1	/* Always true. Needed for VP/ix, MERGE386 and X11 */

uint bmemsize = 0;  /* VPIX checks this to find hole in AT386 machines' memory */
uint maxkpclick;	/* Limit on kernel page pool */
uint firstfree;	 	

struct gdscr def_intf0;   /* default handler for int 0xf0, ... 0xff */

extern pte_t  kpd0[];
extern pte_t  kpt0[];
extern pte_t  kspt0[];

extern char	stext[];	/* Start of kernel text. */
extern char	sdata[];	/* Start of kernel data. */
extern char	sbss[];		/* Start of kernel bss. */
extern char	edata, end;

extern ulong	egafontptr[5];
unchar	*egafont_p[5] = { 0 };

			/* Chunks of physical memory that are NOT used for 
			 * kernel text and data.  We start allocating kernel
			 * bss from memNOTused[], so eventually entries in
			 * memNOTused[] actually refer to "used" memory.
			 *
			 * Note that memNOTused[] cannot be in bss.
			 */
struct	bootmem	memNOTused[B_MAXARGS + B_MAXARGS] = {0,0,0};
unsigned		memNOTusedcnt = 0;

unsigned		memused_idx = 0;
unsigned		minkpclick;	  /* min click for kernel page pool */

int			checksumOK = 0;	  /* flag for checksum */ 

/* This is handy for debugging changes to mlsetup() */
#define CKPT(c)	{int i; short *vid = (short *)0xd00b809e; *vid = c+0x700; \
			for (i = 0x40000; i-- > 0;) ; }

/* Routine called before main, to initialize various data structures.
*/

mlsetup()
{
	register unsigned	nextclick;
	register paddr_t	free_paddr;
	unsigned		firstclick, lastclick;
	unsigned char		ob;
	unsigned		memsize, bss_size, core_size, i;
	pte_t			*pt;
	paddr_t			addr;
	extern struct map piomap[];
	extern int piomapsz;
	unchar *from, *to;
	int    cnt;
	int	binfosum;
	caddr_t	caddr;


#ifdef STARTUP_DEBUG_0
	/*
	 *	For startup debugging only:
	 *	If you want to write to device memory even before device memory
	 *	has been mapped through kernel virtual address space.
	 *	You could generate diagnostics by writing to device memory.
	 *
	 *	Code invocations of the following nature: e.g.,
	 *
	 *	before_device_memory("MLSETUP: has been invoked");
	 */
#endif

	/* checksum the bootinfo structure */
	binfosum = 0;
	for ( i = 0; i < (sizeof(struct bootinfo) - sizeof(int)); i++ )
		binfosum += ((char *)&bootinfo)[i];

	if ( binfosum == bootinfo.checksum )
		checksumOK = 1;

#ifdef AT386
	/*
 	 * Retrieve information passed from boot via the bootinfo struct.
 	 */
	bootarg_parse();

#ifdef	EISA
	/*
	 *  Check to see if this is an EISA machine
	 */
	eisa_enable = inb(EISA_CFG3) != 0;
#endif	/* EISA */

	/*
	 * Time to store the ROM font locations we got from the bios
	 * in uprt.s into the driver's font pointers.
	 */

	for (i = 0; i < 5; i++)
		egafont_p[i] = (unchar *)phystokv(ftop(egafontptr[i]));

#endif /* AT386 */


	/* On an AT386 machine, this is the size (in K) of "base" memory */
	bmemsize = bootinfo.memavail[0].extent >> 10; 


	/* Make sure memavail[] memory chunks are page-aligned */
	for (i = 0; i < bootinfo.memavailcnt; i++) {
		bootinfo.memavail[i].extent =
			ctob(btoc(bootinfo.memavail[i].extent));
		bootinfo.memavail[i].base =
				ctob(btoct(bootinfo.memavail[i].base));
	}

	/* Make sure memused[] memory chunks are page-aligned */
	for (i = 0; i < bootinfo.memusedcnt; i++) {
		bootinfo.memused[i].extent =
			ctob(btoc(bootinfo.memused[i].extent));
		bootinfo.memused[i].base =
				ctob(btoct(bootinfo.memused[i].base));
	}

	/*
	 * Handle relocation of kernel text in special memory, if possible.
	 * Revises bootinfo.memused[] based on kernel text relocation.
	 */
	ml_reloc_ktext();

	/* recalculate checksum in binfo, now that it has changed */

	binfosum = 0;
	for ( i = 0; i < (sizeof(struct bootinfo) - sizeof(int)); i++ )
		binfosum += ((char *)&bootinfo)[i];
	bootinfo.checksum = binfosum;

	/*
	 * Determine which portions of memavail[] are NOT already in
	 * memused[].  At this point, memused[] contains the tuples
	 * for memory allocated for kernel text and data, but not for bss.
	 */
	ml_unused_mem();

	free_paddr = memNOTused[0].base; /* first free physical addr */


	/* Allocate space for the BSS and set up kernel address page table */

	bss_size = btoc(&end) - btoct(sbss);	/* size of bss (clicks) */
	pt = kspt0 + btoct(sbss - KVSBASE);	/* ptr to bss ptes in kspt0[] */


	for (i = memsize = 0; bss_size-- > 0;) {
		if (i >= memNOTusedcnt)
			cmn_err(CE_PANIC, "mlsetup: not enough physical mem\n");
		if (free_paddr >= memNOTused[i].base +
						memNOTused[i].extent) {
			/* crossing unused mem boundary in memNOTused[] */
			memsize += memNOTused[i].extent;
			++i;
			free_paddr = memNOTused[i].base;
		}
		(pt++)->pg_pte = mkpte(PG_V, PFNUM(free_paddr));
		free_paddr += NBPC;
	}

	memused_idx = i;/* Save index into memNOTused[], so we know
			 * which memNOTused[] entries are (by now) actually
			 * used.
			 */
	if (free_paddr >= memNOTused[memused_idx].base
				+ memNOTused[memused_idx].extent) {
		/* Current free_paddr is crossing unused mem boundary in 
		 * memNOTused[].  Make free_paddr point to the next actual
		 * unused physical address.
		 */
		memused_idx++;
		free_paddr = memNOTused[memused_idx].base;
	}


	flushtlb();

	/* Check maxpmem tuneable */
	if (v.v_maxpmem) {
		unsigned maxpmem;
		unsigned part_used;
		unsigned clipped;
		unsigned extent;

		clipped = 0;

		/* 
		 * Don't let maxpmem be less than the size of physical memory
		 * (kernel text + data + bss + boot stuff) already used.
		 */
						/* core_size <= sizeof bss */
		part_used = free_paddr - memNOTused[memused_idx].base;
		core_size = memsize + part_used;

						/* add in rest of used memory */
		for (i = 0; i < bootinfo.memusedcnt; i++) {
			core_size += bootinfo.memused[i].extent;		
		}
		if (v.v_maxpmem < btoc(core_size))
			v.v_maxpmem = btoc(core_size);
		maxpmem = ctob(v.v_maxpmem);
		extent = memNOTused[memused_idx].base + 
				    memNOTused[memused_idx].extent - free_paddr;
		for (i = memused_idx; i < memNOTusedcnt; ) {
			if (extent > (maxpmem - core_size)) {
				memNOTused[i].extent = part_used + 
							maxpmem - core_size;
				clipped++;
			}
			core_size += memNOTused[i].extent - part_used;
			if (++i >= memNOTusedcnt)
				break;
			part_used = 0;
			extent = memNOTused[i].extent;
			
		}

		/* Update bootinfo.memavail[] if any memNOTused[] entries
		 * were clipped.
		 */
		if (clipped)
			ml_update_avail();	
	}

	/*	Zero all BSS. Can't use bzero since
	 *	the u block isn't mapped.
	 */
	wzero(sbss, (&end - sbss + 3) & ~3);


	/*	Set up memory parameters.
	 */

	nextclick = firstfree = btoct(free_paddr);

	physmem = 0;

	/* 
	 * Set maxclick according to the bootinfo memavail[].
	 */
	maxclick = 
		btoct(bootinfo.memavail[bootinfo.memavailcnt - 1].base
			+ bootinfo.memavail[bootinfo.memavailcnt - 1].extent);


#ifdef AT386
	/* Unfortunately, for AT386 we have to support device drivers
		which assume the device memory at bmemsize to 1M is always
		mapped at KVBASE + bmemsize. */
	nextclick = scanmem(nextclick, btoct(bmemsize*1024), btoct(1024*1024));
	physmem = 0;	/* Don't count device memory in physmem */
#endif


#ifdef STARTUP_DEBUG_1
	/*
	 *	For startup debugging only:
	 *	Now that device memory has been mapped through kernel virtual
	 *	address space, to generate diagnostics on screen you can
	 *	code invocations of the following nature: e.g.,
	 *
	 *	dispstring("MLSETUP: Device memory has been mapped");
	 *
	 *
	 */
#endif

	/* Set up linear map for all real memory */
	for (i=0; i < bootinfo.memavailcnt && bootinfo.memavail[i].extent;++i) {
		firstclick = btoct(bootinfo.memavail[i].base);
		lastclick = btoct(bootinfo.memavail[i].base +
						bootinfo.memavail[i].extent);
		nextclick = scanmem(nextclick, firstclick, lastclick);
	}


	/* After we have come up we no longer need addresses at linear 0. */
	/* Make them illegal.  These addresses were needed during the     */
	/* initial code in uprt.s as linear must equal physical for the   */
	/* <cs,pc> prior to the jmp.  Setting linear addresses at 0 to    */
	/* invalid breaks pmon.                                           */

	kpd0[0].pg_pte = mkpte(0, 0);

	flushtlb();

	wzero(phystokv(ctob(nextclick)), ctob(btoct(memNOTused[memused_idx].base +
			   memNOTused[memused_idx].extent) - nextclick));

	for (i = memused_idx+1; i < memNOTusedcnt; i++) {
		wzero(phystokv(memNOTused[i].base), memNOTused[i].extent);
	}

	/*
	 * Allocate u block for proc 0.
	 */

	nextclick = p0u(nextclick);

	/*	Initialize memory mapping for sysseg.
	**	the segment from which the kernel
	**	dynamically allocates space for itself.
	*/

	nextclick = sysseginit(nextclick);

	/*	Initialize the pio window segments.
	*/

	nextclick = pioseginit(nextclick);


	/*	Initialize the map used to allocate
	**	pio virtual space.
	*/

	mapinit(piomap, piomapsz);
#ifdef	STARTUP_DEBUG_1
	dispstring("piomapped ");
#endif
	mfree(piomap, piosegsz, btoc(piosegs));


	/*
	** Allocate page table for kvsegmap.
	*/

	/* Initialize kas stuff. At this point kvseg is initialized, hence calls
	 * to sptalloc now becomes valid - henceforth.
	 */

	nextclick = kvm_init(nextclick);
#ifdef	STARTUP_DEBUG_1
	dispstring("kvm_init done");
#endif


	/* Do scheduler initialization.
	*/

	dispinit();
#ifdef	STARTUP_DEBUG_1
	dispstring("dispinit done");
#endif


	/* Initialize the high-resolution timer free list.
	*/

	hrtinit();
#ifdef	STARTUP_DEBUG_1
	dispstring("hrtinit done ");
#endif


	/* Initialize the interval timer free list.
	*/

	itinit();
#ifdef	STARTUP_DEBUG_1
	dispstring("int timer done");
#endif


	/* Initialize process 0.
	*/

	p0init();
#ifdef	STARTUP_DEBUG_1
	dispstring("proc 0 done");
#endif

	/* Initialize process table.
	*/
	pid_init();
#ifdef	STARTUP_DEBUG_1
	dispstring("pid init done");
#endif

	/*
	 * This is for (pre-4.0 binary drivers) backward compatibility.
	 */
	fix_swtbls();
#ifdef	STARTUP_DEBUG_1
	dispstring("fix swtbls done");
#endif

#ifdef AT386	/* 16MB DMA support */

	if (dma_check_on) {

#ifdef STARTUP_DEBUG_1
		dispstring("set dmalimits");
#endif

		set_dmalimits();

#ifdef STARTUP_DEBUG_1
		dispstring("set dmalimits done");
#endif

		reserve_dma_pages();

#ifdef STARTUP_DEBUG_1
		dispstring("reserve dma pages done");
#endif
	}

#endif	/* 16MB DMA support */

}

int page_hashsz;		/* Size of page hash table table (power of 2) */
struct page **page_hash;	/* Page hash table */
struct seg  *segkmap;		/* kernel generic mapping segment */
struct seg  *segu;		/* u area mapping segment - for kvsegu segment */
int segu_size;			/* Size of floating u area */

kvm_init(nextfree)
register int nextfree;
{
	extern int	kvsegmap[];	/* kv segmap */
	extern int	kvsegu[];	/* u area segment */
	struct segmap_crargs a;

	/* XXX - hard coded value for extra 384K of high physical memory.
	*/
	(void) seg_attach(&kas, KVBASE, (KVXBASE + (384*1024)) - KVBASE, &kpseg);
	(void) segkmem_create(&kpseg, (caddr_t) NULL);

	(void) seg_attach(&kas, (caddr_t) stext,
		(unsigned) sbss - (unsigned) stext + (unsigned) (&end - sbss),
		&ktextseg);
	(void) segkmem_create(&ktextseg, (caddr_t) NULL);

	(void) seg_attach(&kas, syssegs, ctob(syssegsz), &kvseg);
	(void) segkmem_create(&kvseg, (caddr_t) NULL);

	(void) seg_attach(&kas, piosegs, ctob(piosegsz), &kpioseg);
	(void) segkmem_create(&kpioseg, (caddr_t) NULL);

	nextfree = mktables(nextfree);

	hat_init();
#ifdef	STARTUP_DEBUG_1
	dispstring("hat init  done");
#endif

	/*
	 *	Kernel memory allocator initialization. Should be done now
	 *	inorder to allocate physical memory for "kvsegmap" and 
	 *	and "kvsegu" kernel segments - as done immediately below.
	 */

	kmem_init();
#ifdef	STARTUP_DEBUG_1
	dispstring("kmem_init done");
#endif

	/*
	 *  	  The "segkmap" page table is NOT allocated here.
	 *	  This will be dynamically allocated by the hat_memload().
	 */

	segkmap = seg_alloc(&kas, kvsegmap, ctob(segmapsz));
	if (segkmap == NULL)
		cmn_err(CE_PANIC,"cannot allocate segkmap");
	a.prot = PROT_READ | PROT_WRITE;
	if (segmap_create(segkmap, (caddr_t)&a) != 0)
		cmn_err(CE_PANIC,"segmap_create segkmap");
#ifdef	STARTUP_DEBUG_1
	dispstring("seg_kmap created");
#endif


	/* floating u area support */

	/*
	 *	Size of the floating u area = Total Number of processes *
	 *				 the max ublock size of a process.
	 */

	if ((segu_size = ctob(v.v_proc * MAXUSIZE)) <= 0)
		cmn_err(CE_PANIC,"No space for floating u areas\n");

	/*
	 *	We may have to skip some segu slots (those which cross
	 *	page table boundaries), so adjust segu_size accordingly.
	 */

#if (MAXUSIZE % NPGPT) != 0
	segu_size += ctob((ptnum(segu_size) + 1) * MAXUSIZE);
#endif

	/*
	 *	Allocate Floating u area segment structure and
	 *	fix Base and Size of Floating u area segment.
	 */

	if ((segu = seg_alloc(&kas, kvsegu, segu_size)) == NULL)
		cmn_err(CE_PANIC,"cannot allocate segu");

	/*
	 *	Create pool of ublock "slots" for Floating u area segment.
	 */

	a.prot = PROT_READ | PROT_WRITE;
	if (segu_create(segu, (caddr_t)&a) != 0)
		cmn_err(CE_PANIC,"segu_create segu");
#ifdef	STARTUP_DEBUG_1
	dispstring("seg_u created   ");
#endif

	return(nextfree);
}


/*  Allocate page structures and page hash structures for all user free memory.
 *  Scheme: Chew up as much as NON-DMA-able memory as possible for this.
 */
mktables(nextfree)
int nextfree;
{
	register int i, j, m, totalclicks, actualclicks;
	struct page *pp;
	struct pageac *pgmaptbl;


	/*  Find total clicks left in all unused memory segments.
	 */

	totalclicks = btoct(memNOTused[memused_idx].base +
				memNOTused[memused_idx].extent) - nextfree;
	for (j = memused_idx + 1; j < memNOTusedcnt; j++)
		totalclicks += btoct(memNOTused[j].extent);

	/*	Initialize the map used to allocate
	**	kernel virtual space.  Don't let this be bigger than
	**	(totalclicks - lotsfree) to avoid thrashing.
	*/

	mapinit(sptmap, v.v_sptmap);
#ifdef	STARTUP_DEBUG_1
	dispstring("sptmapped ");
#endif
	availkmem = (totalclicks * 3) / 4;
	mfree(sptmap, syssegsz, btoc(syssegs));

	/*  Find total bytes needed for the page hash structures.
	 */
	m = totalclicks / PAGE_HASHAVELEN;
	while (m & (m - 1))
		m = (m | (m - 1)) + 1;
	page_hashsz = m;
	i = m * sizeof(struct page *);

	/*  Find total bytes needed for page hash structures and page structures.
	 */
	m = totalclicks;
	i += m * sizeof(struct page);
	i = btoc(i);			/* total clicks needed */

	/* Remember kernel virtual start for page and page hash structures - use
	*  NON-DMA-ABLE page if possible.
	*/
	pp = (struct page *) sptalloc(1, PG_V, (caddr_t)non_dma_page(&nextfree),
#ifdef AT386	/* 16 MB support */
				KM_NO_DMA
#else
				0
#endif	/* 16 MB support */
				);

	/* Allocate space for the rest page and page hash structures - use
	*  NON-DMA-ABLE pages if possible.
	*/
	for (j = 1; j < i; j++)
		sptalloc(1, PG_V, (caddr_t)non_dma_page(&nextfree),
#ifdef AT386	/* 16 MB support */
				KM_NO_DMA
#else
				0
#endif	/* 16 MB support */
			);

	page_hash = (struct page **) (pp + m);

	/* Fix up current segment base and extent to make computation easier.
	*/
	memNOTused[memused_idx].extent = ctob(btoct(memNOTused[memused_idx].base +
					memNOTused[memused_idx].extent) - nextfree);
	memNOTused[memused_idx].base = ctob(nextfree);

	/* Start filling in the page map table accouting structures attributes.
	*/
	for (actualclicks = 0, pgmaptbl = pageac_table, j = memused_idx;
		j < memNOTusedcnt; j++, pgmaptbl++) {
		pgmaptbl->num = btoct(memNOTused[j].extent);
		actualclicks += pgmaptbl->num;
		pgmaptbl->firstpfn = btoct(memNOTused[j].base);
		pgmaptbl->firstpp = pp;
		pp += pgmaptbl->num;
		page_init(pgmaptbl);
	}

	/*  ASSERT: kernel virtual for page structures lower than kernel virtual
	 *	    for page hash structures.
	 */
	if ((char *)pp > (char *)page_hash)
		cmn_err(CE_PANIC,"Invalid page struct and page hash tables\n");

	/*  Total real and swappable memory equals actual amount of available
	 *  user free memory.
	 */
	availrmem = availsmem = actualclicks;


#ifdef AT386	/* 16MB DMA support */

#ifdef STARTUP_DEBUG_1
	dispstring("CALLING dma page init");
#endif
	dma_page_init();

#ifdef STARTUP_DEBUG_1
	dispstring("dma page init done");
#endif

#endif	/* 16MB DMA support */


	/*  Return recently allocated page structures to the free page pool list.
	 *
	 */


	memialloc();

	return(nextfree);
}

/*	Allocate page tables for the kernel segment sysseg.
 *	syssegs must be on a page table boundary.
*/

sysseginit(nextfree)
int nextfree;
{
	register pte_t *pt;
	register int i;
	register int pgcnt;

	i = ptnum(syssegs);
	for (pgcnt = 0 , pt = &kpd0[i] ; pgcnt < 2048 ; pgcnt += 1024, pt++)
	{
		if (!PG_ISVALID(pt)) {
			pt->pg_pte = 
				mkpte(PG_V | PG_US, non_dma_page(&nextfree));
			wzero(phystokv (ctob(pt->pgm.pg_pfn)), NBPP);
		}
	}

	/* kptbl points to the syssegs page table */

	/* kptbl = (pte_t *)phystokv(ctob(pt->pgm.pg_pfn)); */
	return(nextfree);
}

/*	Allocate page tables for the kernel segment pioseg.
 *	piosegs must be on a page table boundary.
*/

pioseginit(nextfree)
int nextfree;
{
	register pte_t *pt;
	register int i;

	i = ptnum(piosegs);
	pt = &kpd0[i];
	if (!PG_ISVALID(pt)) {
		pt->pg_pte = mkpte(PG_V, non_dma_page(&nextfree));
		wzero(phystokv (ctob(pt->pgm.pg_pfn)), NBPP);
	}

	/* piownptbl points to the piosegs page table */

	piownptbl = (pte_t *)phystokv(ctob(pt->pgm.pg_pfn));
	return(nextfree);
}


/*
 *	Allocate proc 0 u area.
 */

p0u(nextfree)
int nextfree;
{
	register pte_t	*ptptr;

	/* Allocate the usertable page table for &u */

	ptptr = &kpd0[ptnum(&u)];

	if (!PG_ISVALID(ptptr))
		ptptr->pg_pte = mkpte(PG_V, non_dma_page(&nextfree));
	wzero(phystokv(ctob(ptptr->pgm.pg_pfn)), ctob(1));
	usertable = (pte_t *) phystokv(ctob(ptptr->pgm.pg_pfn)) + pnum(&u);

	PG_SETPROT(ptptr, PTE_RW);  /* Page directory entry is user read/write */

	/*
	 *  Add a system-wide page below the ublock (stack grows downwards) to
	 *  prevent kernel stack overflow. Note: This page is not per-process and
	 *  hence cannot be saved/restored as a context of any given process.
	 *
	 *  Note: 80386 B1 Errata 13 doesn't apply here, since we never return
	 *	  to user mode while on this section of the stack.
	 */

	/* The page table */

	ptptr = &kpd0[ptnum((char *)&u - NBPP)];
	if (!PG_ISVALID(ptptr))
		ptptr->pg_pte = mkpte(PG_V, non_dma_page(&nextfree));
	PG_SETPROT(ptptr, PTE_RW);

	/* And the physical page */

	ptptr = (pte_t *) phystokv(ctob(ptptr->pgm.pg_pfn)) +
					pnum((char *)&u - NBPP);
	ptptr->pg_pte = mkpte(PG_V, non_dma_page(&nextfree));
	wzero(((char *)&u - NBPP), NBPP);

	return(nextfree);
}


/*
 *	Allocate proc 0 u area and set up proc 0.
 */

p0init()
{
	register proc_t		*pp;
	register caddr_t	ldtp;
#if VPIX || TSSBITMASK || MERGE386
	register        char    *iobm;
	register        int     iobmcount;
#endif /* VPIX || TSSBITMASK || MERGE386 */
	extern struct gate_desc	idt[];
	extern struct seg_desc	gdt[];
	extern int		userstack[];
	extern struct tss386	ktss;

	/* Allocate proc structure for proc 0 */
	pp = (struct proc *)kmem_zalloc(sizeof(struct proc), KM_NOSLEEP);
	if (pp == NULL) {
proc0_fail:
		cmn_err(CE_PANIC,"process 0 - creation failed\n");
	}
	curproc = proc_sched = pp;
#ifdef	STARTUP_DEBUG_1
	dispstring("got struct for proc0");
#endif

	/* Initialize process data */
	pp->p_usize = MINUSIZE;
	pp->p_stat = SONPROC;
	pp->p_flag = SLOAD | SSYS | SLOCK | SULOAD;
	pp->p_pri = curpri = v.v_maxsyspri;
	pp->p_clfuncs = class[0].cl_funcs;
	pp->p_clproc = (caddr_t)pp;

	pp->p_pidp = &pid0;
	pp->p_pgidp = &pid0;
	pp->p_sessp = &session0;
	pid0.pid_pglink = pp;

	/* Allocate the floating ublock */
	pp->p_segu = (struct seguser *)segu_get(pp, 1);
	if (pp->p_segu == NULL)
		goto proc0_fail;
#ifdef	STARTUP_DEBUG_1
	dispstring("got seg_u for proc0 ");
#endif

	/* Copy the ublock page table into usertable, so we can use &u */
	bcopy((caddr_t)pp->p_ubptbl, (caddr_t)usertable,
					MINUSIZE * sizeof(pte_t));

	/* Zero out the ublock */
	wzero((char *)&u, ctob(MINUSIZE));
#ifdef	STARTUP_DEBUG_1
	dispstring("uarea cleared      ");
#endif

	/*	Initialize proc 0's ublock
	*/

	u.u_procp   = pp;
	u.u_userstack = (unsigned long) userstack;
	u.u_sub     = u.u_userstack + sizeof(int);
	u.u_tsize   = 0;
	u.u_dsize   = 0;
	u.u_ssize   = 0;
	u.u_cmask   = (mode_t) CMASK;

	bcopy((caddr_t)rlimits, (caddr_t)u.u_rlimit,
		sizeof(struct rlimit) * RLIM_NLIMITS);

	/* XENIX Support */
	/*
	 *	Set the default trap handlers for int 0xf0, ... 0xff to
	 *	be invalid traps.  These are used for xenix 286 floating
	 *	point.
	 */
	def_intf0 = * (struct gdscr *) &idt[0xf0];
	* (struct gdscr *) u.u_fpintgate = def_intf0;
	/* End XENIX Support */

	/*
	 *	Now tss is set at the end of (struct user)  - after ufchunk.
	 *	Further ufchunks (to build ufchunk list) are allocated from
	 *	using kmem_zalloc() from "kvseg" (syssegs) - hence the tss
	 *	won't be overrun or corrupted.
	 */
	u.u_tss = (struct tss386 *)
		((int) ((char *)&u.u_flist + sizeof(struct ufchunk) + 3) & ~3L);
#if !VPIX && !TSSBITMASK && !MERGE386
	u.u_sztss = sizeof(struct tss386);
#else /* if VPIX || TSSBITMASK || MERGE386 */
	u.u_sztss = sizeof(struct tss386) + (MAXTSSIOADDR + 1) / 8;
#endif /* VPIX || TSSBITMASK || MERGE386 */

	/* Set up UTSS to refer to current proc's tss (via u) */

	setdscrlim (&gdt[seltoi(UTSSSEL)], u.u_sztss - 1);
	setdscrbase(&gdt[seltoi(UTSSSEL)], (uint)u.u_tss);
	loadtr(UTSSSEL);

	u.u_tss_desc = gdt[seltoi(UTSSSEL)];
	setdscrbase(&u.u_tss_desc, (uint)PTOU(pp) + ((uint)u.u_tss - UVUBLK));
	setdscracc1(&u.u_tss_desc, TSS3_KACC1);

	/*
	 * align ldt on an 8 byte boundary, check size and fixup gdt entry
	 */
	u.u_ldtlimit = MINLDTSZ;
	ldtp = (caddr_t)(((uint)u.u_tss + u.u_sztss + 7) & ~7L);
	pp->p_ldt = (caddr_t)PTOU(pp) + (u_int)(ldtp - UVUBLK);
	setdscrbase(&gdt[seltoi(LDTSEL)], pp->p_ldt);
	u.u_ldt_desc = gdt[seltoi(LDTSEL)];

	if (btoc(ldtp + (u.u_ldtlimit + 1) * sizeof(struct dscr) - UVUBLK)
						> MINUSIZE) {
		cmn_err(CE_PANIC,"MINUSIZE (%d) insufficient\n", MINUSIZE);
	}

	/*	Initialize proc 0's TSS. Fix it up so that
	 *	the stack pointers are set up correctly.
	 */

	bcopy((caddr_t) &ktss, (caddr_t) u.u_tss, sizeof(ktss));
#if VPIX || TSSBITMASK || MERGE386
	u.u_tss->t_bitmapbase = (sizeof (struct tss386)) << 16;
	iobm = (char *) u.u_tss + sizeof(struct tss386);
	for (iobmcount = (MAXTSSIOADDR + 1) / 8; iobmcount > 0; iobmcount--)
	    *iobm++ = 0xff;
#endif /* VPIX || TSBITMASK || MERGE386 */

	u.u_tss->t_esp0 = u.u_tss->t_esp = (unsigned long)((char *)&u + KSTKSZ);
	u.u_tss->t_ldt = LDTSEL;

	/*
	 * Initialize the page fault error handling routine.
	 * The standard routine does a longjmp to u.u_fault_catch.fc_jmp
	 */
	u.u_fault_catch.fc_func = fc_jmpjmp;

	/*
	 * Confirm that the configured number of supplementary groups
	 * is between the min and the max.  If not, print a message
	 * and assign the right value.
	 */
	if (ngroups_max < NGROUPS_UMIN) {
		cmn_err(CE_NOTE, 
		  "Configured value of NGROUPS_MAX (%d) is less than \
min (%d), NGROUPS_MAX set to %d\n", ngroups_max, NGROUPS_UMIN, NGROUPS_UMIN);
		ngroups_max = NGROUPS_UMIN;
	}
	if (ngroups_max > NGROUPS_UMAX) {
		cmn_err(CE_NOTE,
		  "Configured value of NGROUPS_MAX (%d) is greater than \
max (%d), NGROUPS_MAX set to %d\n", ngroups_max, NGROUPS_UMAX, NGROUPS_UMAX);
		ngroups_max = NGROUPS_UMAX;
	}
}

/*
 * Initialize uname info.
 * Machine-dependent code.
 */
void
inituname()
{
	extern char	*release;
	extern char	*version;

	/*
	 * Get the release and version of the system.
	 */
	if (release[0] != '\0') {
		strncpy(utsname.release, release, SYS_NMLN-1);
		utsname.release[SYS_NMLN-1] = '\0';
	}
	if (version[0] != '\0') {
		strncpy(utsname.version, version, SYS_NMLN-1);
		utsname.version[SYS_NMLN-1] = '\0';
	}
}

/* scan memory
 *	Set up page tables to map a chunk of physical
 *	memory 1-1 with linear. Return the next available click.
 */

scanmem(nextclick, firstclick, lastclick)
	uint nextclick, firstclick, lastclick;
{
	register uint	j, k;
	register pte_t	*pt, *pd;

	kpd0[0].pg_pte = mkpte(PG_V, PFNUM(_cr3()));
	j = xphystokv(ctob(firstclick));
	pd = kpd0 + (k = ptnum(j));
	pt = (pte_t *)ctob(k) + pgndx(j);

	/* NOTE: In the following loop, we go one page too far to get around
		 a compiler bug */

	for (j = firstclick; j <= lastclick; ++pd) {
		/* See if we need a new page table */
		if (!PG_ISVALID(pd) || (uint)pd->pgm.pg_pfn < firstfree) {
			pd->pg_pte = mkpte(PG_V, nextclick);
			nextclick = nclick_rtn(nextclick);
			wzero(ptalign(pt), NBPC);
		}
		for (k = j % NPGPT; k < NPGPT && j <= lastclick; ++k, ++j) {
			(pt++)->pg_pte = mkpte(PG_V, j);
		}
	}

	physmem += --j - firstclick;

	return(nextclick);
}


/* 
 * This routine relocates kernel text into physical memory that cannot be
 * dma'd into.  This is for the special case where the 384K of physical
 * memory below 1M is remapped to 2G+640K (Olivetti-style aliased memory) 
 * or 15M+640K (Compaq).
 *
 * N.B.  Assumes there is only one contiguous chunk of non-DMA-able memory.
 */
ml_reloc_ktext()
{
	register uint	pfn;
	register uint	i;
	register uint	j;
	uint	size, nodma, nondma_not_in, save_size;
	pte_t	*pt;
	struct	bootmem	memISused[B_MAXARGS + 1];

	/* 
	 * Determine whether there is any non-dma-able memory to
	 * relocate kernel text in.  The bootstrap will have flagged
	 * this as B_MEM_NODMA. 
	 */
	for (i = 0; i < bootinfo.memavailcnt; i++) {
		if ((bootinfo.memavail[i].flags & B_MEM_NODMA) == B_MEM_NODMA) 
			break; 
	}
	if (i >= bootinfo.memavailcnt) {
		/* didn't find any memory to relocate in */
		return;
	}
	nodma = i;

	/*
	 * Found memory to relocate kernel text into.
	 */


	if ((size = bootinfo.memavail[nodma].extent) > sdata - stext)
		size = sdata - stext;

	/* Make a temporary map to the relocation memory so we can copy */
	pt = kpt0 + btoct(stext - KVSBASE);
	pfn = PFNUM(bootinfo.memavail[nodma].base);
	for (i = btoc(size); i-- > 0;)
		(pt++)->pgm.pg_pfn = pfn++;
	flushtlb();

	/* Copy as much of kernel text into extra memory as possible */
	bcopy(stext, stext + KVBASE - KVSBASE, size);

	/* Remap relocated portion of kernel text addresses to copied portion */
	pt = kspt0 + btoct(stext - KVSBASE);
	pfn = PFNUM(bootinfo.memavail[nodma].base);
	for (i = btoc(size); i-- > 0;)
		(pt++)->pgm.pg_pfn = pfn++;

	flushtlb();

	/*
	 * Modify our idea of which parts of physical memory are in use to 
	 * reflect the new state of affairs.  I.e., the portion of the mem in
	 * high physical memory in which kernel text is relocated should be 
	 * added to "used memory" and flagged as kernel text.
	 * The portion of physical memory from which the kernel text was 
	 * moved should be removed from "used memory".  Put the results in
	 * memISused[].   Note that memavail[] and memused[] are ordered by
	 * increasing base address.  The memISused[] array remains so ordered.
	 *
	 * N.B.  In the worst case, the number of entries in memISused[] is
	 *	 1 greater than the number of entries in bootinfo.memused[].
	 *	 Panic if we hit this case.
	 */

	nondma_not_in = 1;
	size = ctob(btoc(size));	/* page-align size */
	save_size = size;		/* size of kernel text relocated */
	for (i = 0, j = 0; i < bootinfo.memusedcnt; ) {
		if ((bootinfo.memused[i].base > bootinfo.memavail[nodma].base)
				&& nondma_not_in) {
			/*
			 * Insert the used portion of the kernel text.  We only
			 * hit this case if there exists used physical
			 * memory ABOVE the memory into which kernel
			 * text has been relocated.  Special case, provided
			 * in the interest of generality. 
			 */
			nondma_not_in = 0;
			memISused[j] = bootinfo.memavail[nodma];
			memISused[j].flags |= B_MEM_KTEXT;
			memISused[j].extent = save_size;
			++j;
			continue;
		}
		if ((bootinfo.memused[i].flags & B_MEM_KTEXT) != B_MEM_KTEXT) {
			/*
			 * Not kernel text.  Just copy this memory entry to 
			 * memISused[].
			 */
			memISused[j] = bootinfo.memused[i];
			++j;
		}
		else
		/*
		 * Is kernel text.  
		 * Ignore the memused[] text entries until 'size' has been
		 * depleted -- we've relocated 'size' bytes of text.  Keep 
		 * track of partial/full text memused[] entries that did not
		 * get relocated (they remain used).
		 */
		if (bootinfo.memused[i].extent > size) {
			/* memused[] text entry still partially used */
			memISused[j] = bootinfo.memused[i];
			memISused[j].extent -= size;
			memISused[j].base += size;
			++j;
			size = 0;
		}
		else
			/* text entry no longer used (text relocated) */
			size -= bootinfo.memused[i].extent;
		
		++i;
	}

	if (nondma_not_in) {
		/*
		 * If not already done, insert the used portion of the mem.
		 * Gets done in the loop above iff there exists physical
		 * memory ABOVE the remapped text in high physical memory.
		 * Normally, this is not the case, and the remapped text will
		 * be in the highest physical memory available.
		 */
		memISused[j] = bootinfo.memavail[nodma];
		memISused[j].flags |= B_MEM_KTEXT;
		memISused[j].extent = save_size;
		j++;
	}

	/*
	 * Put memISused[] back into bootinfo.memused[].  Panic if we
	 * overflow memused[].  Should never happen...
	 */

	if (j >= (sizeof(bootinfo.memused)/sizeof(struct bootmem)))
		cmn_err(CE_PANIC, 
			     "ml_reloc_ktext:  bootinfo.memused[] overflow\n");

	bootinfo.memusedcnt = j;
	for (i = 0; i <= j; i++) {
		bootinfo.memused[i] = memISused[i];
	}

}

/*
 * This routine determines which portions of physical memory (memavail)
 * are not already used (memused).  The unused memory tuples 
 * (paddr, extent, flags) are stored in memNOTused[].
 *
 */

ml_unused_mem()
{
    register paddr_t  free_paddr;
    register unsigned avail;
    register unsigned used;
    unsigned unused;
    unsigned long free_size;
    ushort   free_flags;

    unused = 0;
    for (avail = 0; avail < bootinfo.memavailcnt; avail++) {
	free_paddr = bootinfo.memavail[avail].base;
	free_size = bootinfo.memavail[avail].extent;
	free_flags = bootinfo.memavail[avail].flags;

	for (used = 0; used < bootinfo.memusedcnt; used++) {
		if ((bootinfo.memused[used].base >= free_paddr) &&
		    (bootinfo.memused[used].base < free_paddr + free_size)) {
			/*
			 * Split out the unused parts of the availmem[] entry
			 * from the used parts.
			 */
			if (free_paddr < bootinfo.memused[used].base) {
				/*
				 * There is a chunk of unused, available memory
				 * from free_paddr upto memused[used].base.
				 * Add it to memNOTused[], and skip free_paddr
				 * past the used chunk.
				 */
				memNOTused[unused].base = free_paddr;
				memNOTused[unused].extent = 
				      bootinfo.memused[used].base - free_paddr;
				memNOTused[unused].flags = free_flags;
				free_paddr = bootinfo.memused[used].base +
						bootinfo.memused[used].extent;
				free_size -= (memNOTused[unused].extent + 
						bootinfo.memused[used].extent);
				unused++;
				continue;
			}
			else
			if (free_paddr == bootinfo.memused[used].base) {
				/*
				 * Skip over the used chunk of memory
				 * starting at free_paddr.
				 */
				free_paddr = bootinfo.memused[used].base +
						bootinfo.memused[used].extent;
				free_size -= bootinfo.memused[used].extent;
			}
		}
	}

	if (free_size > (unsigned long) 0) {
		/*
		 * There is a free chunk of unused available memory at
		 * the end of the availmem[] entry.  Add it to memNOTused[].
		 */
		memNOTused[unused].base = free_paddr;
		memNOTused[unused].extent = free_size;
		memNOTused[unused].flags = free_flags;
		unused++;
	}
    }
    memNOTusedcnt = unused;
}

/*
 * This routine modifies bootinfo.memavail[] based on bootinfo.memused[] and
 * memNOTused[].  This is only necessary when v.v_maxpmem causes some real
 * physical memory to be ignored by the system.
 */
ml_update_avail()
{
	register unsigned used;
	register unsigned notused;
	register unsigned avail;

	used = notused = avail = 0;

	/* Prime the memavail[] array */
	if (bootinfo.memused[used].base < memNOTused[notused].base) 
		bootinfo.memavail[avail] = bootinfo.memused[used++];
	else
		bootinfo.memavail[avail] = memNOTused[notused++];
	bootinfo.memavail[avail].flags = 
		((bootinfo.memavail[avail].flags & B_MEM_NODMA) == B_MEM_NODMA)?
								B_MEM_NODMA : 0;

	/*
	 * Reconstruct memavail[] from memused[] and memNOTused[].  We
	 * know that both memused[] and memNOTused[] are in ascending
	 * 'base' order.  We also know that their entries do not overlap,
	 * but may specify chunks of memory that can be coalesced.
	 */
	while ((used < bootinfo.memusedcnt) && (notused < memNOTusedcnt)) {
		if (bootinfo.memused[used].base < memNOTused[notused].base) {
			if ((bootinfo.memavail[avail].base +
				bootinfo.memavail[avail].extent) 
					== bootinfo.memused[used].base) 
				bootinfo.memavail[avail].extent +=
						bootinfo.memused[used++].extent;
			else {
				bootinfo.memavail[++avail] = 
						bootinfo.memused[used++];
				bootinfo.memavail[avail].flags = 
					((bootinfo.memavail[avail].flags 
						& B_MEM_NODMA) == B_MEM_NODMA)?
								B_MEM_NODMA : 0;
			}
		}
		else {
			if ((bootinfo.memavail[avail].base +
				bootinfo.memavail[avail].extent) 
					== memNOTused[notused].base) 
				bootinfo.memavail[avail].extent +=
						memNOTused[notused++].extent;
			else
				bootinfo.memavail[++avail] = 
							memNOTused[notused++];
		}
	}


	/* Grab anything left in memused[] */
	while (used < bootinfo.memusedcnt) {
		if ((bootinfo.memavail[avail].base +
			bootinfo.memavail[avail].extent) 
				== bootinfo.memused[used].base) 
			bootinfo.memavail[avail].extent +=
					bootinfo.memused[used++].extent;
		else {
			bootinfo.memavail[++avail] = 
					bootinfo.memused[used++];
			bootinfo.memavail[avail].flags = 
				((bootinfo.memavail[avail].flags & 
						B_MEM_NODMA) == B_MEM_NODMA)?
								B_MEM_NODMA : 0;
		}
	}

	/* Grab anything left in memNOTused[] */
	while (notused < memNOTusedcnt) {
		if ((bootinfo.memavail[avail].base +
			bootinfo.memavail[avail].extent) 
				== memNOTused[notused].base) 
			bootinfo.memavail[avail].extent +=
					memNOTused[notused++].extent;
		else
			bootinfo.memavail[++avail] = memNOTused[notused++];
	}

	avail++;
	bootinfo.memavailcnt = avail;
	for ( ; avail < B_MAXARGS; avail++) {
		bootinfo.memavail[avail].base = 0;
		bootinfo.memavail[avail].extent = 0;
		bootinfo.memavail[avail].flags = 0;
	}
}

/* Return a NON-DMA-ABLE page from a NON-DMA-ABLE memory segment. If not possible
 * then return the next page from the current memory segment.
*/

non_dma_page(nextfree_ptr)
	register int *nextfree_ptr;	/* Pointer to current free page */
{
	register int i, j, rtn_click;

	/*  Scan from High to Low memory Segments ... NON-DMA-ABLE memory usually
	 *  in the high end.
	 */
	for (i = memNOTusedcnt - 1; i > memused_idx; i--)
		if ((memNOTused[i].flags & B_MEM_NODMA) == B_MEM_NODMA)
			break;

	/*  NON-DMA-ABLE memory not found or scan stopped at current segment.
	 *  Also indicate which is the next page that can be used from the current
	 *  memory segment (nextfree).
	 */
	if (i == memused_idx)
		*nextfree_ptr = nclick_rtn((rtn_click = *nextfree_ptr));
	else {
		/* NON-DMA-ABLE segment other than the current segment found.
		*/
		rtn_click = btoct(memNOTused[i].base);
		memNOTused[i].base += ctob(1);

		/* If entire segment used up -- deleted segment from list.
		*/
		if ( ! (memNOTused[i].extent -= ctob(1))) {
			for (j = i; j < memNOTusedcnt - 1; j++)
				memNOTused[j] = memNOTused[j+1];
			memNOTusedcnt--;		/* one less segment */
		}
	}
	return(rtn_click);	/* Return page number that can be used */
}

nclick_rtn(click)
register unsigned click;
{


	if ((memused_idx >= memNOTusedcnt) || (++click >= maxclick))
		return(-1);

	if (click >= btoct(memNOTused[memused_idx].base + 
					memNOTused[memused_idx].extent)) {
		if (++memused_idx >= memNOTusedcnt)
			return(-1);
		return(btoct(memNOTused[memused_idx].base));
	}

	return(click);
		
}


#ifdef STARTUP_DEBUG_1
/***** startup code DEBUGGING *********/

/*
 *	Once device memory has been mapped 1-to-1 through kernel virtual address
 *	space, this routine can be invoked to write into device memory.
 */
dispstring(str)
	char *str;
{
	char *memptr, *p;
	int delay, delay1, delay2;

	
	/*** FOR OTHER KINDS OF MONITORS

	memptr = (char *) phystokv(bmemsize * 1024);
	for (p = str; p && *p; p++, memptr++) {
		*memptr = *p;
		memptr++;
		*memptr = (char) 31;
	}

	memptr = (char *) phystokv(720896);
	for (p = str; p && *p; p++, memptr++) {
		*memptr = *p;
		memptr++;
		*memptr = (char) 31;
	}

	***/

	memptr = (char *) phystokv(753664);
	for (p = str; p && *p; p++, memptr++) {
		*memptr = *p;
		memptr++;
		*memptr = (char) 31;
	}
	for (delay = 0 ; delay < 1000000 ; delay++)
		;
}
/***** end of startup code DEBUGGING *********/
#endif	/* STARTUP_DEBUG_1 */

#ifdef STARTUP_DEBUG_0
/***** startup code DEBUGGING *********/

/*
 *	If you want to write to device memory even before the device memory
 *	1-to-1 map has been set up through kernel virtual address.
 *
 *	Warning: Machine configuration dependent: Assumes page#: 1019 is present.
 *		 You are free to use any other free (currently) page.
 */
before_device_memory(p)
	char *p;
{
		pte_t	*pd;
		pte_t	*pt;
		int	addr;
		unsigned int	save_pd0;
		unsigned int	save_pd1;
		unsigned int	save_pd2;

		save_pd0 = kpd0[0].pg_pte;
		kpd0[0].pg_pte = mkpte(PG_V, pfnum(_cr3()));
		addr = xphystokv(753664);
		pd = (pte_t *)kpd0 + ptnum(addr);
		save_pd1 = pd->pg_pte;
		pd->pg_pte = mkpte(PG_V, 1019);

		addr = ctob(1019) + (pnum(addr) * 4);
		save_pd2 = kpd0[pnum(addr)].pg_pte;
		kpd0[pnum(addr)].pg_pte = mkpte(PG_V, 1019);
		pt = (pte_t *)addr;
		pt->pg_pte = mkpte(PG_V, 184);

		dispstring(p);
		dispstring(p);

		flushtlb();

		kpd0[pnum(addr)].pg_pte = save_pd2;
		pd->pg_pte = save_pd1;
		kpd0[0].pg_pte = save_pd0;
		flushtlb();

}
/***** end of startup code DEBUGGING *********/
#endif	/* STARTUP_DEBUG_0 */
