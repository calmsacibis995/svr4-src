/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/asy.h	1.1.2.1"

#define SPL()		spltty()/* protection from interrupts */

/*
 * Definitions for INS8250 / 16550  chips
 */

	/* defined as offsets from the data register */
#define DAT     0   /* receive/transmit data */
#define ICR     1   /* interrupt control register */
#define ISR     2   /* interrupt status register */
#define LCR     3   /* line control register */
#define MCR     4   /* modem control register */
#define LSR     5   /* line status register */
#define MSR     6   /* modem status register */
#define DLL     0   /* divisor latch (lsb) */
#define DLH     1   /* divisor latch (msb) */

/*
 * INTEL 8210-A/B & 16450/16550 Registers Structure.
 */

/* Line Control Register */
#define		WLS0	0x01		/*word length select bit 0 */	
#define		WLS1	0x02		/*word length select bit 2 */	
#define		STB	0x04		/* number of stop bits */
#define		PEN	0x08		/* parity enable */
#define		EPS	0x10		/* even parity select */
#define		SETBREAK 0x40		/* break key */
#define		DLAB	0x80		/* divisor latch access bit */
#define 	RXLEN   0x03    /* # of data bits per received/xmitted character */
#define 	STOP1   0x00
#define 	STOP2   0x04
#define 	PAREN   0x08
#define 	PAREVN  0x10
#define 	PARMARK 0x20
#define 	SNDBRK  0x40
#define 	DLAB    0x80


#define		BITS5	0x00		/* 5 bits per char */
#define		BITS6	0x01		/* 6 bits per char */
#define		BITS7	0x02		/* 7 bits per char */
#define		BITS8	0x03		/* 8 bits per char */

/* Line Status Register */
#define		RCA	0x01		/* data ready */
#define		OVRRUN	0x02		/* overrun error */
#define		PARERR	0x04		/* parity error */
#define		FRMERR	0x08		/* framing error */
#define		BRKDET 0x10		/* a break has arrived */
#define		XHRE	0x20		/* tx hold reg is now empty */
#define		XSRE	0x40		/* tx shift reg is now empty */
#define		RFBE	0x80		/* rx FIFO Buffer error */

/* Interrupt Id Regisger */
#define		MSTATUS	0x00
#define		TxRDY	0x02
#define		RxRDY	0x04
#define		ERROR_INTR	0x08

/* Interrupt Enable Register */
#define		FFTMOUT 0x0c		/* tmp for ringbuf mary */
#define		RSTATUS 0x06		/* tmp for ringbuf mary */
#define		RIEN	0x01		/* Received Data Ready */
#define		TIEN	0x02		/* Tx Hold Register Empty */
#define		SIEN	0x04	/* Receiver Line Status */
#define		MIEN	0x08	/* Modem Status */

/* Modem Control Register */
#define		DTR		0x01	/* Data Terminal Ready */
#define		RTS		0x02	/* Request To Send */
#define		OUT1		0x04	/* Aux output - not used */
#define		OUT2		0x08	/* turns intr to 386 on/off */	
#define		ASY_LOOP	0x10	/* loopback for diagnostics */

/* Modem Status Register */
#define		DCTS		0x01	/* Delta Clear To Send */
#define		DDSR		0x02	/* Delta Data Set Ready */
#define		DRI		0x04	/* Trail Edge Ring Indicator */
#define		DDCD		0x08	/* Delta Data Carrier Detect */
#define		CTS		0x10	/* Clear To Send */
#define		DSR		0x20	/* Data Set Ready */
#define		RI		0x40	/* Ring Indicator */
#define		DCD		0x80	/* Data Carrier Detect */

#define 	DELTAS(x) 	((x)&(DCTS|DDSR|DRI|DDCD))
#define 	STATES(x) 	((x)(CTS|DSR|RI|DCD))


#define 	asychan(dev)    (dev&0x0f)
#define 	asymajor(dev)   ((dev>>8)&0x7f)
#define 	FIFOEN	0x8f	/* fifo enabled, w/ 8 byte trigger */

/* asy_flags definitions */
#define 	XBRK	0x01            /* xmitting break in progress */
#define		HWDEV	0x02		/* Hardware device being used */
#define		HWFLWO	0x04		/* H/W Flow ON */
#define		HWFLWS	0x08		/* Start H/W after CSTOP */
#define		ASY82510	0x40	/* 1 - 82510 , 0 - 16450/16550/8250 */
#define		ASYHERE	0x80		/* adapter is present */
#define 	BRKTIME	HZ/4

/*
 * Defines for ioctl calls (VP/ix)
 */

#define AIOC			('A'<<8)
#define AIOCINTTYPE		(AIOC|60)	/* set interrupt type */
#define AIOCDOSMODE		(AIOC|61)	/* set DOS mode */
#define AIOCNONDOSMODE		(AIOC|62)	/* reset DOS mode */
#define AIOCSERIALOUT		(AIOC|63)	/* serial device data write */
#define AIOCSERIALIN		(AIOC|64)	/* serial device data read */
#define AIOCSETSS		(AIOC|65)	/* set start/stop chars */
#define AIOCINFO		(AIOC|66)	/* tell usr what device we are */

/* Ioctl alternate names used by VP/ix */
#define VPC_SERIAL_DOS		AIOCDOSMODE	
#define VPC_SERIAL_NONDOS	AIOCNONDOSMODE
#define VPC_SERIAL_INFO		AIOCINFO
#define VPC_SERIAL_OUT		AIOCSERIALOUT
#define VPC_SERIAL_IN		AIOCSERIALIN

/* Serial in/out requests */
#define SO_DIVLLSB		1
#define SO_DIVLMSB		2
#define SO_LCR			3
#define SO_MCR			4
#define SI_MSR			1
#define SIO_MASK(elem)		(1<<((elem)-1))


/*
 * Asychronous configuration Structures 
 */

struct asy{
	int		asy_flags;
	unsigned	asy_dat;
	unsigned	asy_icr;
	unsigned	asy_isr;
	unsigned	asy_lcr;
	unsigned	asy_mcr;
	unsigned	asy_lsr;
	unsigned	asy_msr;
	unsigned	asy_vect;
	struct msgb	*asyrbp;
	minor_t		asy_dev;
#ifdef MERGE386
	int (*asy_ppi_func)();		/* Merge asy func pointer */
	unsigned char *asy_ppi_data;	/* merge data */
#endif /* MERGE386 */
};

