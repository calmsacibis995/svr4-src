/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TRAP_H
#define _SYS_TRAP_H

#ident	"@(#)head.sys:sys/trap.h	1.1.2.1"

/*
 * Trap type values
 */

#define	DIVERR		0	/* divide by 0 error		*/
#define	SGLSTP		1	/* single step			*/
#define	NMIFLT		2	/* NMI				*/
#define	BPTFLT		3	/* breakpoint fault		*/
#define	INTOFLT		4	/* INTO overflow fault		*/
#define	BOUNDFLT	5	/* BOUND instruction fault	*/
#define	INVOPFLT	6	/* invalid opcode fault		*/
#define	NOEXTFLT	7	/* extension not available fault*/
#define	DBLFLT		8	/* double fault			*/
#define	EXTOVRFLT	9	/* extension overrun fault	*/
#define	INVTSSFLT	10	/* invalid TSS fault		*/
#define	SEGNPFLT	11	/* segment not present fault	*/
#define	STKFLT		12	/* stack fault			*/
#define	GPFLT		13	/* general protection fault	*/
#define	PGFLT		14	/* page fault			*/
#define	EXTERRFLT	16	/* extension error fault	*/
#define	ENDPERR		33	/* emulated extension error flt	*/
#define	ENOEXTFLT	32	/* emulated ext not present	*/

/*
 *  Values of error code on stack in case of page fault 
 */

#define	PF_ERR_MASK	0x01	/* Mask for error bit */
#define PF_ERR_PAGE	0	/* page not present */
#define PF_ERR_PROT	1	/* protection error */
#define PF_ERR_WRITE	2	/* fault caused by write (else read) */
#define PF_ERR_USER	4	/* processor was in user mode
					(else supervisor) */
#endif	/* _SYS_TRAP_H */
