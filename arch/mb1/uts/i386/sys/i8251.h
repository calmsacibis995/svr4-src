/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_I8251_H
#define _SYS_I8251_H

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mb1:uts/i386/sys/i8251.h	1.3"

#define ISPEED		13	/* initial baud rate of 9600==(13);300==(7)*/
#define MINORMSK 	0x1F	/* reserve bit 7 ; bit 6 for modem */
#define MODEMMSK	0x40	/* bit 6 of the minor number sets modem op */

/*
 * Structures for the 8251 USART
 * ____________________________
 *
 * Commands used for operation and initialization of
 * USART and PIT
 * ____________________________
 *
 * Refer to Intel Microsystem Components Handbook for further information
 * on the 8251 USART and 8254 Programmable Interval Timer
 *
 * Device PHYSICAL port layout
 * Based on Data Block Select
 *
 *
 * Usart I/O functions:
 *
 *	Write: I/O
 *	Read:  I/O
 *
 */

struct i8251cfg{
	ushort	u_data;
	ushort	u_cntrl;
	ushort	t_data;
	ushort	t_cntrl;
	int	in_intr;
	int	out_intr;
	char	t_number;
	char	b_type;
};

/*
 * Type bits used to determine what the 8251 is mounted on.
 */

#define N8251TYPES	2
#define T_GENERIC	0
#define T_ONBOARD	1
#define	T_iSBC351	2


/*
 *
 * 8253/8254 PIT commands
 *
 *	RATEMD0:	read/load timer 0 for mode 3 (baud rate generator)
 *
 *	U8251SPEED:	int constant of 1600 pit count in hex.
 *
 */

#define	RATEMD0		0x36
#define U8251SPEED	0x0640

/*
 * 8251 USART command instructions
 *	I001  split instructions into separate bits
 */

#define	S_TXEN	0x01		/* transmitter enable */
#define	S_DTR	0x02		/* data terminal ready */
#define	S_RXEN	0x04		/* receiver enable */
#define	S_SBRK	0x08		/* send break char */
#define	S_ER	0x10		/* error reset */
#define	S_RTS	0x20		/* request to send */
#define	S_IR	0x40		/* internal reset */



#define S_TXRDY         0x01            /* transmitter ready for character */
#define S_RXRDY		0x02 		/* receiver has data */
#define S_TXEMPTY       0x04            /* transmit buffer empty */
#define S_PERROR	0x08		/* parity error */
#define S_OVERRUN	0x10		/* overrun */
#define S_FRERROR	0x20		/* framing error */
#define S_BRKDET        0x40            /* input break detect */
#define	S_DSRDY		0x80		/* carrier present */

#define S_ERRCOND       (S_PERROR|S_FRERROR)	/* I001 */

/*
 * 8251 USART mode instructions
 */

#define S_BAUDF		0x02	/* baud rate factor = 16x */
#define S_5BPC		0x00	/* 5 bits per char */
#define S_6BPC		0x04	/* 6 bits per char */
#define S_7BPC		0x08	/* 7 bits per char */
#define S_8BPC		0x0C	/* 8 bits per char */
#define S_PAREN		0x10	/* parity enable */
#define S_PAREVEN	0x20	/* even parity */
#define S_1STOP		0x40	/* 1 stop bit */
#define S_2STOP		0xC0	/* 2 stop bits */

/*
 * the following used to be in a file called usart.h.
 * since the 351 is the only driver that uses this stuff, it's
 * been moved into this include file.
 */

/*
 * Baud rate selection.  These go into the clock counter.
 */
#define	US_B38400	2	/* probably drops characters */
#define	US_B19200	4	/* probably drops characters */
#define	US_B9600	8
#define	US_B4800	16
#define	US_B2400	32
#define US_B1800	42	/* I002 42.66/42 => 1.5% error */
#define	US_B1200	64
#define	US_B600		128
#define	US_B300		256
#define US_B200		384
#define	US_B150		512
#define	US_B134		571	/* I002 added for full SVID support */
#define	US_B110		698
#define	US_B75		1024
#define US_B50		1536
#define US_B0		0

#endif	/* _SYS_I8251_H */
