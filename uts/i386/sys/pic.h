/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PIC_H
#define _SYS_PIC_H

#ident	"@(#)head.sys:sys/pic.h	1.1.2.1"

/* Definitions for 8259 Programmable Interrupt Controller */

#define PIC_NEEDICW4    0x01            /* ICW4 needed */
#define PIC_ICW1BASE    0x10            /* base for ICW1 */
#define PIC_86MODE      0x01            /* MCS 86 mode */
#define PIC_AUTOEOI     0x02            /* do auto eoi's */
#define PIC_SLAVEBUF    0x08            /* put slave in buffered mode */
#define PIC_MASTERBUF   0x0C            /* put master in bnuffered mode */
#define PIC_SPFMODE     0x10            /* special fully nested mode */
#define PIC_READISR     0x0B            /* Read the ISR */
#define PIC_NSEOI       0x20            /* Non-specific EOI command */

#define PIC_VECTBASE    0x40            /* Vectors for external interrupts */
					/* start at 64.                    */
/*
 * Interrupt configuration information specific to a particular computer.
 * These constants are used to initialize tables in modules/pic/space.c.
 * NOTE: The master pic must always be pic zero.
 */

#if defined (MB1) || defined (MB2)

#define NPIC    2                       /* 2 PICs */
/* Port addresses */
#define MCMD_PORT       0xC0            /* master command port */
#define MIMR_PORT       0xC2            /* master intr mask register port */
#define SCMD_PORT       0xC4            /* slave command port */
#define SIMR_PORT       0xC6            /* slave intr mask register port */
#define MASTERLINE      0x07            /* slave on IR7 of the master PIC */
#define SLAVEBASE       ((MASTERLINE+1)*8) /* slave IR0 interrupt number */
#define PICBUFFERED     1               /* PICs in buffered mode */
#define I82380          0               /* i82380 chip not used */

#endif /* MB1 || MB2 */


#ifdef AT386            /* AT386 board */

#define NPIC    2                       /* 2 PICs */
/* Port addresses */
#define MCMD_PORT       0x20            /* master command port */
#define MIMR_PORT       0x21            /* master intr mask register port */
#define SCMD_PORT       0xA0            /* slave command port */
#define SIMR_PORT       0xA1            /* slave intr mask register port */
#define MASTERLINE      0x02            /* slave on IR2 of master PIC */
#define SLAVEBASE       8               /* slave IR0 interrupt number */
#define PICBUFFERED     0               /* PICs not in buffered mode */
#define I82380          0               /* i82380 chip not used */

#endif /* AT386 */

#endif	/* _SYS_PIC_H */
