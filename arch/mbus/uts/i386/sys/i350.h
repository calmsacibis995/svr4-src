/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
 *	Copyright 1989  Intel Corporation
*/

#ident	"@(#)mbus:uts/i386/sys/i350.h	1.1"

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

#define	LPLWAT		50		/* line printer low water mark */
#define LP_MSG   	1		/* allow error mesage display */
#define LP_NO_MSG   	0		/* allow error mesage display */
/*
 *	Hardware constants
*/
#define TEST		0xaa	/* test pattern read back by probe */
#define PR_ACK_BAR	0x10 	/* printer ACK line */
#define ONSTROBE	0x1	/* Turn data strobe signal on */
#define OFFSTROBE	0	/* Turn data strobe signal off */
#define LP_PORTA_350	0x80	/* data 0-7 */
#define LP_PORTB_350	0x82	/* unused */
#define LP_PORTC_350	0x84	/* hi nibble in lo nibble out */
#define LP_CONTROL_350	0x86	/* 8255 control port */
#define LP_INT_LEVEL_350   7	/* interrupt level */
#define PRINT_INIT_350	0xa8	/* control word for 8255 */
#define PR_SELECT_350	0x10	/* printer select bit */
#define PR_ERROR_350	0x20	/* printer error bit */
#define PR_ACK_BAR_350	0x40	/* printer ~ack bit */	
#define ENABLE_INT_350	0x0d	/* enable interrupt - set port c bit 6 */
/*
* lp states
*/
#define LP_ALIVE	0x01	/* lp hardware  present */
#define LP_DAEMON	0x02	/* turn on daemon to do polled wakeup */
/*
 *	Device Structures
*/
struct i350_cfg {
		int p_level;	/* intr level */
		int p_porta;	/* 8255 port a -> data out */
		int p_portb;	/* 8255 port b -> status in */
		int p_portc;	/* 8255 port c -> strobe and interrupt f/f */
		int control;	/* 8255 control port */
};

#endif	/* _SYS_I350_H */
