/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TSS_H
#define _SYS_TSS_H

#ident	"@(#)head.sys:sys/tss.h	1.1.2.1"

/* Flags Register */

typedef struct flags {
	uint	fl_cf	:  1,		/* carry/borrow */
			:  1,		/* reserved */
		fl_pf	:  1,		/* parity */
			:  1,		/* reserved */
		fl_af	:  1,		/* carry/borrow */
			:  1,		/* reserved */
		fl_zf	:  1,		/* zero */
		fl_sf	:  1,		/* sign */
		fl_tf	:  1,		/* trace */
		fl_if	:  1,		/* interrupt enable */
		fl_df	:  1,		/* direction */
		fl_of	:  1,		/* overflow */
		fl_iopl :  2,		/* I/O privilege level */
		fl_nt	:  1,		/* nested task */
			:  1,		/* reserved */
		fl_rf	:  1,		/* reset */
		fl_vm	:  1,		/* virtual 86 mode */
		fl_res	: 14;		/* reserved */
} flags_t;

#define	PS_C		0x0001		/* carry bit			*/
#define	PS_P		0x0004		/* parity bit			*/
#define	PS_AC		0x0010		/* auxiliary carry bit		*/
#define	PS_Z		0x0040		/* zero bit			*/
#define	PS_N		0x0080		/* negative bit			*/
#define	PS_T		0x0100		/* trace enable bit		*/
#define	PS_IE		0x0200		/* interrupt enable bit		*/
#define	PS_D		0x0400		/* direction bit		*/
#define	PS_V		0x0800		/* overflow bit			*/
#define	PS_IOPL		0x3000		/* I/O privilege level		*/
#define	PS_NT		0x4000		/* nested task flag		*/
#define	PS_RF		0x10000		/* Reset flag			*/
#define	PS_VM		0x20000		/* Virtual 86 mode flag		*/

/*
 * Maximum I/O address that will be in TSS bitmap
 */
#define	MAXTSSIOADDR	0x3ff

/*
 * 386 TSS definition
 */

struct tss386 {
	unsigned long t_link;
	unsigned long t_esp0;
	unsigned long t_ss0;
	unsigned long t_esp1;
	unsigned long t_ss1;
	unsigned long t_esp2;
	unsigned long t_ss2;
	paddr_t	      t_cr3;
	unsigned long t_eip;
	unsigned long t_eflags;
	unsigned long t_eax;
	unsigned long t_ecx;
	unsigned long t_edx;
	unsigned long t_ebx;
	unsigned long t_esp;
	unsigned long t_ebp;
	unsigned long t_esi;
	unsigned long t_edi;
	unsigned long t_es;
	unsigned long t_cs;
	unsigned long t_ss;
	unsigned long t_ds;
	unsigned long t_fs;
	unsigned long t_gs;
	unsigned long t_ldt;
	unsigned long t_bitmapbase;
};

#define		PS_USER		3
#define		PS_KERNEL	0

#endif	/* _SYS_TSS_H */
