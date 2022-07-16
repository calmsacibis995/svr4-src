/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PIT_H
#define _SYS_PIT_H

#ident	"@(#)head.sys:sys/pit.h	1.1.3.1"

/*
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 */

#if defined (MB1) || defined (MB2)
/* Definitions for 8254 Programmable Interrupt Timer ports on 386/20 */
#define	PITCTR0_PORT	0xD0		/* counter 0 port */	
#define	PITCTR1_PORT	0xD2		/* counter 1 port */	
#define	PITCTR2_PORT	0xD4		/* counter 2 port */	
#define	PITCTL_PORT	0xD6		/* PIT control port */
#endif

#ifdef AT386
/* Definitions for 8254 Programmable Interrupt Timer ports on AT 386 */
#define	PITCTR0_PORT	0x40		/* counter 0 port */	
#define	PITCTR1_PORT	0x41		/* counter 1 port */	
#define	PITCTR2_PORT	0x42		/* counter 2 port */	
#define	PITCTL_PORT	0x43		/* PIT control port */
#define	PITAUX_PORT	0x61		/* PIT auxiliary port */
#define SANITY_CTR0	0x48		/* sanity timer counter */
#define SANITY_CTL	0x4B		/* sanity control word */
#define SANITY_CHECK	0x461		/* bit 7 set if sanity timer went off*/
#define FAILSAFE_NMI	0x80		/* to test if sanity timer went off */
#define ENABLE_SANITY	0x04		/* Enables sanity clock NMI ints */
#define RESET_SANITY	0x00		/* resets sanity NMI interrupt */
#endif /* AT386 */

/* Definitions for 8254 commands */

/* Following are used for Timer 0 */
#define PIT_C0          0x00            /* select counter 0 */
#define	PIT_LOADMODE	0x30		/* load least significant byte followed
					 * by most significant byte */
#define PIT_NDIVMODE	0x04		/*divide by N counter */
#define	PIT_SQUAREMODE	0x06		/* square-wave mode */
#define	PIT_ENDSIGMODE	0x00		/* assert OUT at end-of-count mode*/

/* Used for Timer 1. Used for delay calculations in countdown mode */
#define PIT_C1          0x40            /* select counter 1 */
#define	PIT_READMODE	0x30		/* read or load least significant byte
					 * followed by most significant byte */
#define	PIT_RATEMODE	0x06		/* square-wave mode for USART */

#if defined(MB1)
#define CLKNUM 12300			/* clock speed for the timer in hz 
					 * divided by the constant HZ
					 * ( defined in param.h )
					 */
#endif /* MB1 */

#if defined(MB2)
#define CLKNUM 12500			/* clock speed for the timer in hz 
					 * divided by the constant HZ
					 * ( defined in param.h )
					 */
#endif /* MB2 */

#ifdef AT386
#define	CLKNUM	(1193167/HZ)		/* clock speed for timer */
#define SANITY_NUM	0xFFFF		/* Sanity timer goes off every .2 secs*/
/* bits used in auxiliary control port for timer 2 */
#define	PITAUX_GATE2	0x01		/* aux port, PIT gate 2 input */
#define	PITAUX_OUT2	0x02		/* aux port, PIT clock out 2 enable */
#endif /* AT386 */

#endif	/* _SYS_PIT_H */
