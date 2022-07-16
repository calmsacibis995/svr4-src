/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_WEITEK_H
#define _SYS_WEITEK_H

#ident	"@(#)head.sys:sys/weitek.h	1.1.2.1"

/*
 * Weitek floating point processor definitions
 */

#define WEITEK_VADDR 0xFFC00000
#define WEITEK_ADDRS 0xFFFF0000
#define WEITEK_SIZE  0x00010000
#define WEITEK_MAXADDR (WEITEK_VADDR + WEITEK_SIZE - 1)

/*
 * masks for accumulated exception byte
 */
#define	WFPDE	0x00000080	/* data chain exception			*/
#define	WFPUOE	0x00000040	/* unimplemented opcode			*/
#define	WFPPE	0x00000020	/* precision				*/
#define	WFPUE	0x00000010	/* underflow				*/
#define	WFPOE	0x00000008	/* overflow				*/
#define	WFPZE	0x00000004	/* zero divide				*/
#define	WFPEE	0x00000002	/* enabled exception			*/
#define WFPIE	0x00000001	/* invalid operation			*/

/*
 * Define all of the execption bits and the number of bits to left-shift
 * to turn exception bits into exception mask bits.
 */
#define	WFPAE		0x000000FD
#define	WFPAEEM_SHFT	16

/*
 *  masks for process context register exception mask byte
 */
#define	WFPDM	0x00800000	/* data chain exception	*/
#define	WFPUOM	0x00400000	/* unimplemented opcode	*/
#define	WFPPM	0x00200000	/* precision		*/
#define	WFPUM	0x00100000	/* underflow		*/
#define	WFPOM	0x00080000	/* overflow		*/
#define	WFPZM	0x00040000	/* zero divide		*/
#define	WFPIM	0x00010000	/* invalid operation	*/
#define WFPB17	0x00020000	/* bit 17 always set	*/

/*
 *  rounding modes for process context register
 */
#define	WFPRN	0x00000000	/* round to nearest value		*/
#define	WFPRZ	0x04000000	/* round toward zero			*/
#define	WFPRP	0x08000000	/* round toward positive infinity	*/
#define	WFPRM	0x0C000000	/* round toward negative infinity	*/
#define WFPRIS	0x00000000	/* round to integer based on RND	*/
#define WFPRIZ	0x02000000	/* round integer to zero always		*/
#define WFPB24	0x01000000      /* bit 24 always set                    */

extern char		weitek_kind;
#define	WEITEK_NO	0	/* no chip support	*/
#define	WEITEK_HW	1	/* chip present		*/
#define	WEITEK_SW	2	/* emulator present	*/

extern struct proc	*weitek_proc;
extern unsigned long	weitek_paddr;
extern void		save_weitek();
extern void		restore_weitek();
extern int		weitek_debug;

/* Definitions for the Weitek emulator */

#define PCR_INTR  0x00000002l

struct ctxt_type {
   long REGISTERS[32]; /* The 1167 register file */
   long PCR;           /* The current PCR register */
};

/* Short hand defines to access the registers and pcr from the structure */
#define registers ctxt -> REGISTERS
#define pcr       ctxt -> PCR

/* global mapping to weitek physical space */
extern int	weitek_pt;

#endif	/* _SYS_WEITEK_H */
