/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_SYS_TERMIOX_H
#define	_SYS_TERMIOX_H

#ident	"@(#)head.sys:sys/termiox.h	1.4.5.1"

/* This structure provides an extended terminal interface. */
/* Features of this interface are optional and may not be */
/* implemented on all machines. */


#define NFF	5

/* hardware flow control modes */

#define RTSXOFF  0000001 /* Enable RTS hardware flow control on input */
#define CTSXON   0000002 /* Enable CTS hardware flow control on output */
#define DTRXOFF  0000004 /* Enable DTR hardware flow control on input */
#define CDXON    0000010 /* Enable CD hardware flow control on output */
#define ISXOFF   0000020 /* Enable isochronous hardware flow control on input */
          
/* clock modes */

#define XMTCLK    0000007 /* Transmit Clock Source: */
#define XCIBRG    0000000 /* Get transmit clock from */
                          /* internal baud rate generator */
#define XCTSET    0000001 /* Get transmit clock from */
                          /* transmitter signal element */
                          /* timing (DCE source) lead, */
                          /* CCITT V.24 circuit 114, */
                          /* EIA-232-D pin 15 */
#define XCRSET    0000002 /* Get transmit clock from */
                          /* receiver signal element */
                          /* timing (DCE source) lead, */
                          /* CCITT V.24 circuit 115, */
                          /* EIA-232-D pin 17 */

#define RCVCLK    0000070 /* Receive Clock Source: */
#define RCIBRG    0000000 /* get receive clock from internal */
                          /* baud rate generator */
#define RCTSET    0000010 /* Get receive clock from */
                          /* transmitter signal element */
                          /* timing (DCE source) lead, */
                          /* CCITT V.24 circuit 114, */
                          /* EIA-232-D pin 15 */
#define RCRSET    0000020 /* Get receive clock from */
                          /* receiver signal element */
                          /* timing (DCE source) lead, */
                          /* CCITT V.24 circuit 115, */
                          /* EIA-232-D pin 17 */

#define TSETCLK   0000700 /* Transmitter Signal Element */
                          /* timing (DTE source) lead, */
                          /* CCITT V.24 circuit 113, */
                          /* EIA-232-D pin 24, clock source: */
#define TSETCOFF  0000000 /* TSET clock not provided */
#define TSETCRBRG 0000100 /* Output receive baud rate generator */
                          /* on circuit 113 */
#define TSETCTBRG 0000200 /* Output transmit baud rate generator */
                          /* on circuit 113 */
#define TSETCTSET 0000300 /* Output transmitter signal element */
                          /* timing (DCE source) on circuit 113 */
#define TSETCRSET 0000400 /* Output receiver signal element */
                          /* timing (DCE source) on circuit 113 */

#define RSETCLK   0007000 /* Receiver Signal Element */
                          /* timing (DTE source) lead, */
                          /* CCITT V.24 circuit 128, */
                          /* no EIA-232-D pin, clock source: */
#define RSETCOFF  0000000 /* RSET clock not provided */
#define RSETCRBRG 0001000 /* Output receive baud rate generator */
                          /* on circuit 128 */
#define RSETCTBRG 0002000 /* Output transmit baud rate generator */
                          /* on circuit 128 */
#define RSETCTSET 0003000 /* Output transmitter signal element */
                          /* timing (DCE source) on circuit 128 */
#define RSETCRSET 0004000 /* Output receiver signal element */
                          /* timing (DCE source) on circuit 128 */


struct termiox {
	unsigned short x_hflag;		/* hardware flow control modes */
	unsigned short x_cflag;		/* clock modes */
	unsigned short x_rflag[NFF];	/* reserved modes*/
	unsigned short x_sflag;		/* spare modes */
};

#define XIOC    ('X'<<8)
#define TCGETX  (XIOC|1)
#define TCSETX  (XIOC|2)
#define TCSETXW (XIOC|3)
#define TCSETXF (XIOC|4)

#endif	/* _SYS_TERMIOX_H */
