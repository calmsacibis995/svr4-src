/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_I546_H
#define _SYS_I546_H

/*	Copyright (c) 1983, 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mb1:uts/i386/sys/i546.h	1.3"

#define SPL		spltty		/* good enough to keep interrupts out */

#define CLEAR		0		/* no message received or sent */
#define I546ERROR	0x02		/* hardware error flag */
#define INTR_CLR	0x04		/* clear 546/48 interrupt */
#define I546LIMIT	0xC000		/* size of 546/48 memory size - 1 */
#define I546ACC		DSA_DATA	/* access permit */
#define	I546TYPES	4		/* maximum variant board types */
#define	I188BOARD	1		/* board type for standard 188/48 */
#define	I548BOARD	2		/* board type for 547/548 board */
#define	I546BOARD	3		/* board type for 546 with clock */
#define RESET		1		/* board reset command/ response */
#define iSBX354s	0x0101		/* multimodule init request */
/* flags in l_state */
#define ALIVE		0x01		/* operational flag, board and line */
#define INSTOP		0x04		/* input interrupts stopped */
#define INBUSY		0x08		/* output stopped while input busy */
#define INFLUSH		0x10		/* flush all char on board */
#define CARRF		0x40		/* actual carrier state */
#define RECFF		0x80		/* received parity marking FF */
#define RECFFNUL	0x02		/* received parity marking FF and NUL */

#define i546LINES	12		/* max lines per 546 board */

#define	LPSTAT_GET	(('p'<<8)|0)	/* get the parallel port status */

/*
 * iSBC 546/48 Private Data Structures
 *------------------------------------
 *
 * These are the descriptions of the data structures required
 * by the 546/48 device driver to manage the 546/48 hardware.
 */

/*
 * The structure of the message buffer
 */
struct i546msg	{
	char	m_type;		/* message command type */
	char	m_line;		/* line on board being commanded */
	ushort	m_cnt;		/* byte count for data transfers */
	ushort	m_ptr;		/* offset pointer, from board base */
	ushort	m_buf[5];	/* variable message data segment */
	};

/*
 * line structure, one per line on a board
 */
struct i546line {
	char	l_state;	/* driver specific line state */
	char	l_mask;		/* output delay timeout value */
	ushort	l_ibsiz;	/* size of input line buffer */
	ushort	l_iba;		/* input line buffer base address */
	ushort	l_ibp;		/* input buffer data pointer */
	ushort	l_ibc;		/* input buffer data count */
	ushort	l_obsiz;	/* size of output line buffer */
	ushort	l_oba;		/* output line buffer base address */
	ushort	l_obp;		/* output buffer available pointer */
	ushort	l_obc;		/* output buffer usage count */
	ushort	l_ocnt;		/* output outstanding message count */
	};


/*
 * Structure of all the information to know the state of
 * the iSBC 546/48 board's firmware
 */
struct	i546board {
	int	b_alive; 	/* set at init if board is there */
	short	b_select;	/* segment selector of board */
	ushort	b_port;		/* board's interrupt port */
	char	b_type;		/* board type; 546 = 1; LCTC8 = 2 */
	char	b_version;	/* firmware version */
	struct i546control *b_addr; /* kernel linear address of bd's memory */
	struct	i546msg b_msg;	/* message buffer, task time */
	struct	i546line b_line[i546LINES];	/* one per line state data */
	};

/*
 * board configuration structure declaration
 * there is one structure for each board
 * with predefines configuration data.
 */
struct	i546cfg {
	long	c_addr;		/* board's physical address */
	int	c_port;		/* board's interrupt address */
	int	c_level;	/* board's interrupt level */
	};


/*
 * Macros to obtain the board and line numbers
 * from the the device number.
 */
#define	BRDNO(d)	(minor(d) / i546LINES)
#define	LINNO(d)	(minor(d) % i546LINES)


/*
 * Commands Message Types to the iSBC 546/48
 */
#define	INIT		0x01	/* initialization message */
#define	ENABLE		0x02	/* enable line message */
#define	DISABL		0x03	/* disable line message */
#define	CONFIG		0x04	/* configure line parameters message */
#define	OUTPUT		0x05	/* transmit buffer message */
#define	ABORT		0x06	/* abort transmit buffer */
#define	SUSPND		0x07	/* suspend transmit message */
#define	RESUME		0x08	/* resume transmit message */
#define	DTRAST		0x09	/* assert DTR message */
#define	DTRCLR		0x10	/* clear DTR message */
#define BRKSET		0x11	/* set BREAK condition on output */
#define BRKCLR		0x12	/* clear BREAK condition on output */
#define	INPUT		0x15	/* clear receive buffer message */

/*
 * Commands Message Types from the iSBC 546/48
 */
#define	OUTCMP		0x01	/* transmit complete message */
#define	INAVIL		0x02	/* input available message */
#define	ONCARR		0x04	/* carrier detect message */
#define	OFCARR		0x05	/* carrier loss message */
#define INTCMP		0x06	/* initialization complete message */
#define	SPCOND		0x08	/* special character received message */
 
 
/* 
 * line configuration message parameters
 */
#define	PNO		0x00	/* no parity */
#define	PODD		0x03	/* odd parity */
#define	PEVEN		0x02	/* even parity */
#define C6BITS		0x00	/* 6 bit data */
#define	C7BITS		0x04	/* 7 bit data */
#define	C8BITS		0x08	/* 8 bit data */
#define	STBITS		0x00	/* stop bits - default (1) */
#define	STBITS1		0x00	/* stop bits - 1 */
#define	STBITS15	0x10	/* stop bits - 1.5 */
#define	STBITS2		0x20	/* stop bits - 2 */
#define PEDEL		0x00	/* delete (discard) input parity error chars */
#define PEHIBIT		0x40	/* set high bit on input parity error chars */
#define PEMARK		0x80	/* mark (0xFF,0) input parity error chars */
#define PEACCEPT	0xc0	/* accept input parity error chars as is */
#define	LNDISP		0x100	/* line discipline, for firmware */
#define	SPCHAR		0x02	/* special characters, for firmware */
#define SPHIWAT		0x100	/* special character high water mark */

/*
 * Supported baud rates of the 546 device driver
 */
#define	US_B0		0	/* Drop DTR		*/
#define	US_B50		50
#define	US_B75		75
#define US_B110		110
#define US_B134		134	/* Close enough?	*/
#define US_B150		150
#define US_B200		200
#define US_B300		300
#define US_B600		600
#define US_B1200	1200
#define US_B1800	1800
#define US_B2400	2400
#define US_B4800	4800
#define US_B9600	9600
#define US_B19200	19200
#define US_B38400	38400

/*
 * iSBC 546/48 board buffer addresses from the board base
 * and buffer sizes for each line and the aggregate per board.
 */
#define BDTEST		   16	/* board's static structure test area */
#define	BDSTART		16384	/* start of dual port for 546 boards */

#define OUTQBASE	 1808	/* base offset of output message queue */
#define INQBASE		  272	/* base offset of input message queue */

#define OUTBASE_8LINES	18864	/* base address of line output buffers */
#define OUTSIZE_8LINES	 1737	/* size of one line's output buffer */
#define OUTBASE_12LINES	26624	/* base address of line output buffers */
#define OUTSIZE_12LINES	 1877	/* size of one line's output buffer */

#define INBUFBASE	 3344	/* base address of line input buffers */
#define INLINSIZ	 1940	/* size of one line's input buffer */

#define i546QSIZE 96

struct i546control {
	/* TestEngBootArea */
		char MagicPattern[12];
		unsigned short JumpOffset;
		unsigned short JumpSelector;
	/* StaticStructures */
		unsigned char BoardType;
		char Version;
		char CompletionFlag;
		char ConfidenceTestResult;
		char SSfiller[124];
	/* DynamicStructures */
		char InQTail;
		char InQHead;
		char OutQTail;
		char OutQHead;
		char DSfiller[124];
	/* Queues */
		struct i546msg InQ[i546QSIZE];
		struct i546msg OutQ[i546QSIZE];
	/* ReceiveBuffers */
		char RBuffers[15520];
	/* TransmitBuffers */
		char TBuffers[13904];
};

#endif	/* _SYS_I546_H */
