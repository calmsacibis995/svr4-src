/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mbus:uts/i386/sys/lp_i350.h	1.1"
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
 *	Copyright 1990  Intel Corporation
*/

#ifndef	_SYS_I350_H
#define _SYS_I350_H
/*
 *  This is for the printer which is attached to the iSBX350 
 *  module on an 386/2x.
 *
 *     8255 is programmed for mode 1 (mode word: a8H)
 *
 *     Port A : OUTPUT  Port B : not used
 *     Port C : printer control signals
 *
 *  Port A bin definition:
 *	
 *	 1-7   - data
 *	  
 *  Port C bit definition: 
 *       
 *         0   - STB/ (strobe char to printer)
 *         1   - not used
 *         2   - not used
 *         3   - MINTR1 (interrupt to the 386/2x)
 *         4   - Line Printer SLCT (printer select)
 *         5   - Line Printer ERROR (paper out)
 *         6   - Line Printer ACK/
 *
*/

/*
 *	Hardware constants
*/
#define LP_TEST_PATTERN		0xaa	/* test pattern read back by probe */
#define LP_CONTROL_WORD		0xa8	/* 8255 control port */
#define LP_ONSTROBE			0x1		/* Turn data strobe signal on */
#define LP_OFFSTROBE		0x0		/* Turn data strobe signal off */
#define LP_SELECT			0x10	/* printer select bit */
#define LP_ACK				0x40 	/* printer ACK line */
#define LP_ERROR			0x20	/* printer error bit */
#define LP_INT_CLEAR		0x0d	/* enable interrupt - set port c bit 6 */
#define	LP_MINOR			4		/* Minor number for this device */

/*
 *	Device Structures
*/
struct lp_cfg {
		int p_level;	/* intr level */
		int p_porta;	/* 8255 port a -> data out */
		int p_portb;	/* 8255 port b -> status in */
		int p_portc;	/* 8255 port c -> strobe and interrupt f/f */
		int control;	/* 8255 control port */
};

/*
 * HW dependent macros
 */
#define	LP_DATAPORT(cfg)	((cfg)->p_porta)
#define	LP_INITIALISE_CHIP(cfg)	outb((cfg)->control, LP_CONTROL_WORD)
#define	LP_INITIALISE_PRINTER(cfg)
#define	LP_INTERRUPT_CLEAR(cfg) outb((cfg)->control, LP_INT_CLEAR)
#define	LP_OUTPUT_CHAR(cfg, c) 	outb ((cfg)->p_porta, ~(c))
#define	LP_TURNON_STROBE(cfg)	outb ((cfg)->p_portc, LP_ONSTROBE)
#define	LP_TURNOFF_STROBE(cfg)	outb ((cfg)->p_portc, LP_OFFSTROBE)
#define	LP_PRINTER_STATUS(cfg)	inb  ((cfg)->p_portc)
#define	LP_PRINTER_READY(stat) \
		(((stat) & LP_SELECT) && !((stat) & LP_ERROR) && ((stat) & LP_ACK))
#endif	/* _SYS_I350_H */
