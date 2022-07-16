/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SEG_H
#define _SYS_SEG_H

#ident	"@(#)head.sys:sys/seg.h	11.1.8.1"

/*
 * The segment structure is prototype and place holder for segment descriptors
 * s_base contains the 32 bit base field of the descriptor.
 * s_limacc contains 20 limit bits and 12 accbits in this order
 *
 * 0                      19  20  21      22   23   24  25,26 27-31
 * _____________________________________________________________________________
 * |                                                                           |
 * |   20 bit limit field   | G | B or D | 0 | AVL | P | DPL | Other Attributes|
 * |___________________________________________________________________________|
 * 
 *	The structure gets fixed up at run time to look like a descriptor
 *
 */

/* segment */
struct seg_desc {
	unsigned long s_base;	/* segment base */
	unsigned long s_limacc;	/* 4 limit and access bytes */
};

/* descriptor */
struct dscr {
	unsigned int	a_lim0015:16,
		 	a_base0015:16,
		  	a_base1623:8,
			a_acc0007:8,
			a_lim1619:4,
			a_acc0811:4,
			a_base2431:8;
};

/* access rights for data segments */ 
#define UDATA_ACC1	0xF2 	/* present dpl=3 writable */
#define KDATA_ACC1	0x92	/* present dpl=0 writable */
#define DATA_ACC2	0xC	/* 4Kbyte gran. 4Gb limit avl=0 */
#define DATA_ACC2_S	0x4	/* 1 byte gran., 32bit operands, avl=0 */
#define UTEXT_ACC1	0xFA 	/* present dpl=3 readable */
#define KTEXT_ACC1	0x9A 	/* present dpl=0 readable */
#define TEXT_ACC2	0xC	/* 4Kbyte gran., 32 bit operands avl=0 */
#define TEXT_ACC2_S	0x4	/* 1 byte gran., 32 bit operands avl=0 */
#define LDT_UACC1	0xE2	/* present dpl=3 type=ldt */
#define LDT_KACC1	0x82	/* present dpl=0 type=ldt */
#define LDT_ACC2	0x0	/* G=0 avl=0*/
#define TSS3_KACC1	0x89 	/* present dpl=0 type=available 386 TSS */
#define TSS3_KBACC1     0x8B    /* present dpl=0 type=busy 386 TSS      */
#define	TSS2_KACC1	0x81 	/* present dpl=0 type=available 286 TSS */
#define TSS3_UACC1	0xE9 	/* present dpl=3 type=available 386 TSS */
#define TGATE_UACC1     0xE5    /* present dpl=3 type=task gate         */
#define	TSS2_UACC1	0xE1 	/* present dpl=3 type=available 286 TSS */
#define TSS_ACC2	0x0	/* g=0 avl=0 */
#define SEG_CONFORM	0X4	/* conforming bit in acc0007 */

#define MKDSCR(base, limit, acc1, acc2) \
	{(u_long)(base), ((u_long)(limit)|((u_char)(acc2)<<20)|((u_char)(acc1)<<24))}

/* selector definitions */
#define LDTSEL		0x140	/* LDT for the current process */
#define UTSSSEL		0x148	/* TSS for the current process */
#define KTSSSEL 	0x150	/* TSS for the scheduler */
#define KCSSEL		0x158	/* kernel code segment selector */
#define KDSSEL		0x160	/* kernel data segment selector */
#define DFTSSSEL	0x168	/* TSS for double fault handler */
#define JTSSSEL		0x170
#define MON1SEL		0x178	/* Selector to get to monitor int 1 handler */
#define MON3SEL		0x180	/* Selector to get to monitor int 3 handler */
#define FPESEL		0x193	/* Selector for the FP emulator image */
#define XTSSSEL         0x188   /* XTSS for dual-mode processes */
#define GRANBIT		0x8	/* bit in acc0811 for granularity */

/* user selectors */
#define	USER_CS		0x17	/* user's code segment */
#define	USER_DS		0x1F	/* user's data segment */
#define	USER_SCALL	0x07	/* call gate for system calls */
#define	USER_SIGCALL	0x0F	/* call gate for sigreturn */
#define USER_FPSTK	0x27	/* alias 32 bit stack selector for emulator */
#define USER_FP		0x2F	/* selector for fp emulator to u area */
#define CSALIAS_SEL	0x37	/* CS alias selector for USER_DS (for XENIX */
				/* 	execseg()/unexecseg() system calls). */

#define IDTSZ		256
#define MONIDTSZ	16
#define MINLDTSZ        256	/* initial index of ldt */
#define MAXLDTSZ        8192	/* maximum index of ldt */
#define GDTSZ           90
#define SEL_LDT		4	/* mask to determine if sel is GDT or LDT */

#define KTBASE          0xC0008000
#define KDBASE          0xC0068000

/*
 *  Call/Interrupt/Trap Gate table descriptions
 *
 *	This is the structure used for declaration of Gates.
 *	If this is changed in any way, the code in uprt.s
 *	must be changed to match.  It is especially important
 *	that the type byte be in the last position so that the
 *	real mode start up code can determine if the gate is
 *	intended to be a gate or segment descriptor.
 */

struct gate_desc {
	unsigned long  g_off;		/* offset */
	unsigned short g_sel;		/* selector */
	unsigned char  g_wcount;	/* word count */
	unsigned char  g_type;		/* type of gate and access rights */
};

/* ...and the way the hardware sees it */

struct gdscr {
	unsigned int	gd_off0015:16,
		 	gd_selector:16,
			gd_unused:8,
			gd_acc0007:8,
			gd_off1631:16;
};
 
/* access rights field for gates */

#define GATE_UACC	0xE0		/* present and dpl = 3 */
#define GATE_KACC	0x80		/* present and dpl = 0 */
#define GATE_386CALL	0xC		/* 386 call gate */
#define GATE_386INT	0xE		/* 386 int gate */
#define GATE_386TRP	0xF		/* 386 trap gate */
#define GATE_TSS	0x5		/* Task gate */

/* make an interrupt gate */
#define MKGATE(rtn, sel, acc) \
	{(u_long)(rtn), (u_short)(sel), (u_char)0, (u_char)(acc)}
#define MKINTG(rtn)	MKGATE(rtn, KCSSEL, GATE_KACC|GATE_386INT)
#define MKKTRPG(rtn)	MKGATE(rtn, KCSSEL, GATE_KACC|GATE_386TRP)
#define MKUTRPG(rtn)	MKGATE(rtn, KCSSEL, GATE_UACC|GATE_386TRP)

#define	seltoi(sel)	((u_short)(sel) >> 3)

#endif	/* _SYS_SEG_H */
