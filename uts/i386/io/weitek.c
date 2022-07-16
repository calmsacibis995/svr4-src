/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:weitek.c	1.3"

/*
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 */

/*
** routines that deal with the Weitek floating point chip
*/

#ifdef WEITEK

#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/sysmacros.h"
#include	"sys/systm.h"
#include	"sys/dir.h"
#include	"sys/signal.h"
#include	"sys/user.h"
#include	"sys/errno.h"
#include	"sys/trap.h"
#include	"sys/seg.h"
#include	"sys/sysinfo.h"
#include        "sys/immu.h"
#include	"sys/proc.h"
#include	"sys/weitek.h"
#include	"sys/reg.h"
#include	"sys/tss.h"
#include	"sys/signal.h"
#include	"sys/debug.h"
#include	"vm/page.h"

#define	TRUE		(0==0)
#define	FALSE		(!TRUE)

/* Define some needed WEITEK instructions */

#define	WINS_STOR	0x03
#define	WINS_STORD	0x23
#define	WINS_STCTX	0x31

/* Define 386 op codes */

#define	REP_REPE		0xF3
#define	REPNE			0xF2
#define	LOCK			0xF0
#define	ADDR_SIZE		0x67
#define	OP_SIZE			0x66
#define	CS_OVERRIDE		0x2E
#define	SS_OVERRIDE		0x36
#define	DS_OVERRIDE		0x3E
#define	ES_OVERRIDE		0x26
#define	FS_OVERRIDE		0x64
#define	GS_OVERRIDE		0x65

#define	MOV_TO_R8		0x8A
#define	MOV_TO_R16_32		0x8B
#define	MOV_FROM_R8		0x88
#define	MOV_FROM_R16_32		0x89
#define	MOV_TO_AL		0xA0
#define	MOV_TO_AX_EAX		0xA1
#define	MOV_FROM_AL		0xA2
#define	MOV_FROM_AX_EAX		0xA3
#define	MOVS8			0xA4
#define	MOVS16_32		0xA5
#define	LODS8			0xAC
#define	LODS16_32		0xAD
#define	STOS8			0xAA
#define	STOS16_32		0xAB
#define	MOV_IMM8		0xC6
#define	MOV_IMM16_32		0xC7

/* Define 386 flag bits */

#define	EFLAGS_DF		0x400	/* Direction flag for string inst. */

/* Decoded 386 instruction information */

typedef	struct {
	long	ins_loc;		/* Beginning of instruction */
	long	op_code_loc;	/* Location of op code */
	int	op_code;	/* Value of op code */
	int	op_size;	/* 1, 2, or 4 byte operand */
	int	addr_size;	/* 1, 2, or 4 byte address */
	int	len;		/* Number of bytes in instruction, <= 15 */
	int	op_error;	/* TRUE if an instruction error */
	int	repeat;		/* TRUE if a REP prefix seen */
} ins386;

/* Define the mappings of the register bits in the ModR/M byte to the offsets
of the corresponding registers in the u_ar0 element of the user information. */

static	Reg_to_user [8] = {EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI};

/* Define a mask for data which will be 1, 2, or 4 bytes long.  The
undefined sizes are zero to insure that nothing changes, but a consistency
check could look for the zeros since they are illegal sizes anyway.  */

static	size_mask [5] = {0, 0xFF, 0xFFFF, 0, 0xFFFFFFFF};

extern	void wtl1167_mr ();
extern	void wtl1167_rr ();
extern	void init_1167 ();
static	void store_value ();
static	long get_value ();
static	void decode_ins ();
static	void inc_string ();
static	int reg_op ();
static	long calc_esi (), calc_edi ();
static	int segdesc_D ();

char		weitek_kind = WEITEK_NO;    /* do we have the weitek chip?  */

struct proc	*weitek_proc = NULL;	/* process who owns weitek registers */
pte_t		weitek_kptn;		/* used to save original kptn entry  */

#ifdef WEITEK_DEBUG
int	weitek_debug = 1;
#endif

/*
** init_weitek
**      Initializes the Weitek hardware, as appropriate.
*/

init_weitek ()
{
	if (u.u_weitek == WEITEK_HW) {
		weitek_init ();
	}
}

/*
** weitek_save
**	save the Weitek floating point state into weitek_proc's user structure.
**
**	(optimized version also found in ../ml/misc.s)
*/
weitek_save()
{
#ifdef WEITEK_DEBUG
	if (weitek_debug > 2)
		printf("entering weitek_save...");
#endif

	if ((weitek_proc != NULL) && (u.u_procp == weitek_proc)) {
#ifdef WEITEK_DEBUG
		if (weitek_debug > 1)
			printf("going to call save_weitek...\n");
#endif
		if (u.u_weitek == WEITEK_HW) {
			save_weitek(u.u_weitek_reg);
		}
	}

#ifdef WEITEK_DEBUG
	if (weitek_debug > 2)
		printf("completed weitek_save\n");
#endif
}

/*

This is used to restore the state of the Weitek board when a process resumes.

*/

weitek_restore (regs)
long	* regs;
{
	if (u.u_weitek == WEITEK_HW) {
		restore_weitek (regs);
	}
}

/* When an interrupt occurs send a signal to the process.	*/

weitintr(level)
int level;
{
	psignal(u.u_procp, SIGFPE);
}

/*
	this entry in kptn just happens to apply to both WEITEK_STCTX and 
	WEITEK_LDCTX, so we can map in just this entry and have sufficient
	access to detect the Weitek chip.
*/

#define	entry	0xC
extern pte_t	kptn[];

/*
	Replace kptn entry so that pmon mapping won't interfere with WEITEK 
	interface.  The following two routines are only called from ml/misc.s
*/

weitek_map()
{
	weitek_kptn.pg_pte = kptn[entry].pg_pte;
	kptn[entry].pg_pte = mkpte(PG_V, weitek_paddr >> PNUMSHFT);
}

weitek_unmap()
{
	kptn[entry].pg_pte = weitek_kptn.pg_pte;
}

/*

This clears the Weitek accumulated execeptions byte, both when the Weitek
board is actually present and when it is being emulated.

*/

clear_weitek_ae ()
{
	struct	ctxt_type * ctxt;

	if (u.u_weitek == WEITEK_HW)
		u.u_weitek_reg[WEITEK_CONTEXT] &= WEITEK_CAE;
}

/*

This resets the hardware or software interrupt for the Weitek.

*/

weitek_reset_intr ()
{
	struct	ctxt_type * ctxt;

	if (u.u_weitek == WEITEK_HW) {
		u.u_weitek_reg [0] = reset_weitek_intr ();
	}
}


/*	Allocate system wide page table for weitek and map its entries to the
 *	physical addresses on the weitek board. Called once during the lifetime
 *	of the system.
 */
int
init_weitek_pt()
{
	register struct page *pp;
	register pte_t *pt;
	register int weitek_phys_addr;

	/*	This routine can be invoked once only in the entire lifetime of the
	 *	system.
	 */
	ASSERT(weitek_pt < 0);

	/*	Get the weitek page table dynamically.
	 */
	pp = page_get(PAGESIZE, 1);
	ASSERT(pp != (struct page *)NULL);

	/*	Prevent the pageout demon or any else from getting to this page.
	 */
	pp->p_lock = pp->p_keepcnt = 1;

	/*  Physical page number of the weitek page table.
	 */
	weitek_pt = page_pptonum(pp);

	/*
	 *	Invalidate illegal references to unmapped portions of the pt.
	 */
	bzero((caddr_t) phystokv(ctob(weitek_pt)), NBPP);

	/*	Set the page table entries to point to the physical addresses on
	 *	the weitek board.
	 */
	for (pt = (pte_t *)phystokv(ctob(weitek_pt)) + pnum(WEITEK_VADDR),
		weitek_phys_addr = btoc(weitek_paddr);
		weitek_phys_addr < btoc(weitek_paddr + WEITEK_SIZE);
		weitek_phys_addr++, pt++) {
		pt->pgm.pg_pfn = weitek_phys_addr;
		PG_SETPROT(pt, PG_V | PG_US | PG_RW);	/* User readable/writeable */
	}
	return(0);
}

/*	Map in the weitek page table into its right place in the page directory.
 */
extern pte_t kpd0[];
int
map_weitek_pt()
{
	register pte_t *pt;

	/*  The weitek page table should have been created by now.
	 */
	ASSERT(weitek_pt >= 0 && 
		((struct page *)(page_numtopp(weitek_pt)))->p_lock == 1 &&
		((struct page *)(page_numtopp(weitek_pt)))->p_keepcnt == 1);

	/*	Kernel Page directory entry for the weitek virtual address.
	 */
	pt = &kpd0[0] + ptnum(WEITEK_VADDR);

	/*	Map in the weitek page table.
	 */
	pt->pgm.pg_pfn = weitek_pt;

	/*	Set correct protections - user readable/writeable.
	 */
	PG_SETPROT(pt, PG_V | PG_US | PG_RW);

	return(0);
}

/*	Unmap the weitek page table from its translation from the page directory.
 */
int
unmap_weitek_pt()
{
	((pte_t *)(kpd0 + ptnum(WEITEK_VADDR)))->pg_pte = (unsigned int) 0;
	return(0);
}

#else				/* WEITEK */

weitek_intr (level)
int level;
{
}
#endif /* WEITEK */
