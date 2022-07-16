/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988, 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */

#ident	"@(#)mbus:uts/i386/io/i354.c	1.3.2.3"

#ifndef lint
static char i354_copyright[] = "Copyright 1986, 1987, 1988, 1989 Intel Corporation 460951";
#endif /* lint */

/*
 *
 * Intel iSBX354 driver -- the onboard card containing a 82530 SCC UART
 *
 * Modems are not supported by this driver.
 *
 * Note on debugging this driver:
 *	When the 82530 is also the the console there are
 *	extreme problems with printing out info while
 *	trying to debug this driver.
 *
*/
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/signal.h"
#include "sys/param.h"
#include "sys/conf.h"
#include "sys/stream.h"
#include "sys/termio.h"
#include "sys/i354.h"
#include "sys/i82530.h"
#include "sys/strtty.h"
#include "sys/iasy.h"
#include "sys/ddi.h"


#ifdef DEBUG354
#define DINIT 0x01	/* during initialization */
#define DCALL 0x02	/* when called (open, close, ...) */
#define DIO   0x04	/* when IO done (read, write, ...) */
#define DINTR 0x08	/* interrupts */
#define DIOCTL 0x10	/* ioctl operations */
#define DWILF 0x20	/* "What I'm Looking For" -- temp debugging msgs */
static int i354debug = 0x0;
#define DDEBUG(x, y) if(i354debug&(x))cmn_err y
#else
#define DDEBUG(x, y)
#endif

extern struct i354cfg i354cfg[];
struct strtty *i354_tty;
extern int i354_cnt;			/* How many ports are configured */
int	i354alive = 0;			/* How many ports are alive */
int	i354proc();
void i354wakeup();
extern void outb(), splx(), ttrstrt(), wakeup();
extern void	signal();
extern int	ttiocom();
extern struct streamtab iasyinfo;	/* pointer to iasy driver */

extern struct strtty *iasy_register();
extern void iasy_input();
extern int  iasy_output();
extern void iasyhwdep();
extern void iasy_hup();

/* Routine for putting a char in the input c-list */
#define PutInChar(tx, c) if ((tx)->t_in.bu_ptr != NULL) { \
		*(tx)->t_in.bu_ptr = (c); \
		(tx)->t_in.bu_cnt--; \
		iasy_input((tx), L_BUF); \
	}

/*
 *	DEBUG CONSOLE SUPPORT
*/
/*
 * i354co() -- output kernel character to 354
 *
 * Kernel call to output characters to this device.  We cannot rely
 * on interrupts or anything else in the system except the hardware
 * connection to the device.
 */
i354co(thechar)
	int thechar;
{	register struct	i354cfg *mp;	/* I/O addresses for this particular port*/

	mp = &i354cfg[getminor(conssw.co_dev)];/* mp -> port addresses for line */
	i354w530(mp->m_ctrl, M_REG1, 0); /* Polled mode */
	while ( (i354r530(mp->m_ctrl, M_REG0)&M_TX_EMPTY) == 0) {
		drv_usecwait(10);
	}
	outb(mp->m_data, thechar);
	i354w530(mp->m_ctrl, M_REG1, M_P_SPEC|M_RX_INT|M_TX_INT_EN|M_EXT_EN);

	/* if char. is a newline, send a carriage return */
	if (thechar == '\n')
		i354co('\r');
}

/*
 * i354ci() -- read one char from 354 as a kernel console.
 *
 * Return the character or -1 if there is none.  Doesn't wait for anything.
 *
 * Actually, this is only used to get one character from the console
 * at installation time.  This does the pause between floppy changes
 * and the actual character is ignored.
 */
i354ci()
{	register struct	i354cfg *mp;	/* I/O addresses for this particular port*/
	int c;

	mp = &i354cfg[getminor(conssw.co_dev)];/* mp -> port addresses for line */
	i354w530(mp->m_ctrl, M_REG1, 0); /* Polled mode */
	if ( (i354r530(mp->m_ctrl, M_REG0)&M_CHAR_AV) == 0) {
		c = -1;
	} else {
		c = inb(mp->m_data)&0xFF;
	}
	i354w530(mp->m_ctrl, M_REG1, M_P_SPEC|M_RX_INT|M_TX_INT_EN|M_EXT_EN);
	return(c);
}

/*
 *	BOOT TIME INITITALIZATION
*/
/*
 * i354init() -- initialize driver with interrupts off
 *
 *  This is called before the interrupts are turned on to initalize
 *  the device.  Reset the puppy and program it do async IO and to
 *  give interrupts for input and output.
 */
void
i354init()
{	register struct	i354cfg *mp;	/* I/O addresses for this particular port*/
	unsigned	porta, portb;
	unsigned	tc = (BRG_CONSTANT / (long) 9600 ) -2;
	int		i, i354open();

	/*
	 * Unconditionally, register i354 device with iasy driver so
	 * that the tty nodes are always created for i354 ports
	 * and thus avoid confusion in tty node names.
	 *
	 * Compute the iasy major number.  
	*/
	for (i = 0; i < cdevcnt; i++) {
		if (cdevsw[i].d_str == &iasyinfo)	/* Streams i354 via iasy */
			break;
	}
	if (i >= cdevcnt)
		return;
	/*
	 *	Register terminal server.
	*/
	i354_tty = iasy_register(0, i354_cnt, i354proc, iasyhwdep);

	/*
	 *	Each 82530 implements a pair of ports.  It is assumed the user
	 *	will configure both ports in adjacent i354cfg[] entries.
	*/
	mp = i354cfg;
	i354alive = 0;
	while (mp < i354cfg+i354_cnt) {
		porta = mp[CH_A].m_ctrl;
		portb = mp[CH_B].m_ctrl;
		DDEBUG(DINIT, (CE_CONT, "i354: config %x %x\n", porta, mp[CH_A].m_data) );

		/*
		 * software reset to 82530 because hardware
		 * reset doesn't necessarily reset the 82530.
		 */
		i354w530(porta, M_REG9, M_RESET);
		drv_usecwait(1000);

		/*
		 * write vector to channel A w reg 2,
		 * read it back.
		 * if same as written, chip exists.
		 */
		i354w530(porta, M_REG2, TEST_VECT);
		if (i354r530(porta, M_REG2) != TEST_VECT) {
			cmn_err(CE_CONT,"iSBX 354 at 0x%x NOT found\n", porta);
			mp[CH_A] = mp[CH_A+2];	/* Replace this one with the next */ 
			mp[CH_B] = mp[CH_B+2];
			i354_cnt -= 2;
			continue;
		}
		/*
		 * initialize 82530 with iSDM parameters
		 *
		 * the master interrupt enable bit is set, but all
		 * interrupt sources are masked until a channel is opened.
		 */
		i354w530(portb, M_REG4, M_1STOP|M_16X);
		i354w530(portb, M_REG1, 0);
		i354w530(portb, M_REG2, 0);
		i354w530(portb, M_REG3, mp[CH_B].m_reg3=0);
		i354w530(portb, M_REG5, mp[CH_B].m_reg5=M_TX_8BPC|M_TX_EN);
		i354w530(portb, M_REG6, 0);
		i354w530(portb, M_REG7, 0);
		i354w530(portb, M_REG10, 0);
		i354w530(portb, M_REG11, M_RX_TX_CLKS);
		i354w530(portb, M_REG14, M_BRG_SRC);
		i354w530(portb, M_REG12, tc&0xff);
		i354w530(portb, M_REG13, tc>>8&0xff);
		i354w530(portb, M_REG14, M_BRG_SRC|M_BRG_EN);
		i354w530(portb, M_REG15, 0);

		i354w530(porta, M_REG4, M_1STOP|M_16X);
		i354w530(porta, M_REG1, 0);
		i354w530(porta, M_REG2, 0);
		i354w530(porta, M_REG3, mp[CH_A].m_reg3=M_RX_8BPC|M_RX_EN);
		i354w530(porta, M_REG5, mp[CH_A].m_reg5=M_TX_8BPC|M_RTS|M_DTR|M_TX_EN);
		i354w530(porta, M_REG6, 0);
		i354w530(porta, M_REG7, 0);
		i354w530(porta, M_REG10, 0);
		i354w530(porta, M_REG11, M_RX_TX_CLKS);
		i354w530(porta, M_REG14, M_BRG_SRC);
		i354w530(porta, M_REG12, tc&0xff);
		i354w530(porta, M_REG13, tc>>8&0xff);
		i354w530(porta, M_REG14, M_BRG_SRC|M_BRG_EN);
		i354w530(porta, M_REG15, 0);

		/*
		 * Enable interrupts from the chip (however all sources
		 * are masked until an open) and set non vectored mode.
		 */
		i354w530(portb, M_REG9, M_NV|M_MIE);
		drv_usecwait(1000);	/* Need some extra delay here */
		i354alive += 2;	/* two more ports live */
		cmn_err(CE_CONT, "iSBX 354 at 0x%x found\n", porta);

		/*
		 * flush possible garbage out of input FIFOs
		 */
		while (i354r530(porta, M_REG0) & M_CHAR_AV)
			(void) i354r530(porta, M_REG8);

		while (i354r530(portb, M_REG0) & M_CHAR_AV)
			(void) i354r530(portb, M_REG8);
		mp += 2;	/* proceed with next configuration entry */
	}

	/*	Set up unit zero as the system console. */
	consregister(i354ci, i354co, makedevice(i, 0));
}


/*
 *	INTERRUPT TIME EXECUTION THREAD
*/
/*
 * i354intr() -- process device interrupt
 *
 * This procedure is called by system with interrupts off
 * when the 354 interrupts the processor. The 354 specified by the
 * interrupt level is polled. Since the 82530 has two channels, a
 * determination is made as to which channel interrupted. The appro-
 * priate strtty structure is obtained and the lineswitch ttystart is
 * called (which in turn calls i354proc).
 *
 */
/*ARGSUSED*/
void
i354intr(level)
	int	level;			/* Unused */
{	struct	i354cfg	*mp_a;		/* base I/O address for whole 82530 */
	register struct	strtty *tp;	/* strtty structure */
	register struct	i354cfg *mp;	/* I/O addresses for this particular port*/
	int	c;
	int	rr;			/* read reg */
	int	unit;

	/* loop through all chips */
	mp_a = (struct i354cfg *)&i354cfg[CH_A];
	while (mp_a < i354cfg+i354_cnt) {
	    /* loop through all interrupts currently active on the chip */
	    while (i354r530(mp_a->m_ctrl, M_REG3) != 0) {
			/* ch. B rr2 status in vector */
			rr = i354r530(mp_a[CH_B].m_ctrl, M_REG2);
			/*
		 	* determine which channel we are talking to
		 	*/
			unit = mp_a - i354cfg;
			mp = mp_a;		/* mp -> port addresses for line */
			if (!(rr & M_CHA)) {
				unit++;		/* channel B interrupt */
				mp++;
			}
			tp = &i354_tty[unit];	/* tp -> pointer to strtty struct */
	
			DDEBUG(DINTR, (CE_CONT, "i354intr: rr=%x\n", rr ) );
			DDEBUG(DINTR, (CE_CONT, "unit=%d mp=%x mp=%x\n", unit, mp, mp) );
	
			switch ((rr>>1) & 0x7) {	/* on interrupt vector in SR 2 */
				case 0:	/* channel B, Transmitter buffer empty */
				case 4:	/* channel A, Transmitter buffer empty */
					i354w530(mp->m_ctrl, M_REG0, M_RS_TX_INT);
					/* If no console output, start next char; else wait */
					if (i354r530(mp->m_ctrl, M_REG0) & M_TX_EMPTY) {
						drv_setparm(SYSXINT, 1);
						tp->t_state &= ~BUSY;		/* clear busy bit */
						(void) i354proc(tp, T_OUTPUT);
					}
					break;
		
				case 1:	/* channel B, ext/status change */
				case 5:	/* channel A, ext/status change */
					i354w530(mp->m_ctrl, M_REG0, M_RS_EX_INT);
					drv_setparm(SYSMINT, 1);
					rr = i354r530(mp->m_ctrl, M_REG0);

					(void) i354r530(mp->m_ctrl, M_REG8); /* Flush the NULL */

					if (rr & M_DCD) {
						/* carrier on */
						mp->m_state |= CARRF;
						i354SetCarr(tp, mp);
						wakeup((caddr_t)&tp->t_rdqp);
					}
					else {
						/* carrier off */
						mp->m_state &= ~CARRF;
						if ((tp->t_state&CARR_ON) && !(tp->t_cflag&CLOCAL))
							iasy_hup(tp);
						i354SetCarr(tp, mp);
					}
					if (rr & M_BRK_EN) { /* Break received */
						mp->m_state |= BRK_IN_PROGRESS;
						i354w530(mp->m_ctrl, M_REG0, M_ERR_RES);
						if (!(tp->t_iflag&IGNBRK))
				    		iasy_input((tp), L_BREAK);
					}
					else { /* Break condition terminated */
						mp->m_state &= ~BRK_IN_PROGRESS;
					}
					break;
				case 2:	/* channel B, Receiver Ready */
				case 6:	/* channel A, Receiver Ready */
					drv_setparm(SYSRINT, 1);
					do {
						c = inb(mp->m_data) & mp->m_mask;
						if (!(mp->m_state & BRK_IN_PROGRESS))
							i354ProcessInChar(tp, c);
					} while ( (inb(mp->m_ctrl) & M_CHAR_AV) != 0);
					break;
		
				case 3:	/* channel B, Special Receive */
				case 7:	/* channel A, Special Receive */
					drv_setparm(SYSRINT, 1);
					c = inb(mp->m_data) & mp->m_mask;
					rr = i354r530(mp->m_ctrl, M_REG1);
					i354w530(mp->m_ctrl, M_REG0, M_ERR_RES);
					if (rr & (M_PERROR|M_FRERROR|M_OVERRUN)) {
			    		if (tp->t_iflag & INPCK) {
							/* parity errors are either nulled or marked */
							if (tp->t_iflag & PARMRK) {
								PutInChar(tp, 0377);
								PutInChar(tp, 0);
								PutInChar(tp, c&0377);
								break;
							}
							else if (!(tp->t_iflag & IGNPAR)) {
								PutInChar(tp, 0);
								break;
							}
			    		}
					}
					i354ProcessInChar(tp, c);
					break;
				default:
					/* error condition -- can't happen */
					break;
			}
			i354w530(mp->m_ctrl, M_REG0, M_EOI);	/* reset highest IUS */
	    }
	    mp_a += 2;
	}
}

/*
 * i354ProcessInChar() -- do the input processing for a character
 *
 * This is called from the interrupt routine to handle all the per
 * character processing stuff.
 */
i354ProcessInChar(tp, c)
	register struct strtty *tp;
	int c;
{
	if (!(tp->t_state & ISOPEN))
		return;
	if (tp->t_iflag & ISTRIP) {
		c &= 0177;
	} else {
		if (c == 0377 && tp->t_iflag&PARMRK) {
			/* if marking parity errors, this character gets doubled */
			PutInChar(tp, 0377);
			PutInChar(tp, 0377);
			return;
		}
	}
	if (tp->t_iflag & IXON) {
		/* if stopped, see if to turn on output */
		if (tp->t_state & TTSTOP) {
			if (c == CSTART || tp->t_iflag&IXANY) {
				(void) i354proc(tp, T_RESUME);
			}
		} else {
			/* maybe we're supposed to stop output */
			if (c == CSTOP) {
				(void) i354proc(tp, T_SUSPEND);
			}
		}
		if (c==CSTART || c==CSTOP) {
			/* these two characters are never seen by the reader */
			return;
		}
	}
	PutInChar(tp, c);
}

/*
 * i354proc() -- low level device dependant operations
 *
 * It is called at both task time by the line discipline routines,
 * and at interrupt time by i354intr().
 * i354proc handles any device dependent functions required
 * upon suspending, resuming, blocking, or unblocking output; flushing
 * the input or output queues; timing out; sending break characters,
 * or starting output.
 */
int
i354proc(tp, cmd)
	register struct strtty *tp;
	int	cmd;
{
	register struct	i354cfg *mp;
	int s;
	int ret_val = 0;

	mp = (struct i354cfg *) &i354cfg[(int)(tp - i354_tty)];

	s = SPL();
	switch(cmd)  {
	case T_WFLUSH:			/* flush output queue */
		tp->t_out.bu_cnt = 0;   /* abandon this buffer */
	case T_RESUME:			/* resume output */
		tp->t_state &= ~TTSTOP;
		i354startio(tp);
		break;
	case T_SUSPEND:			/* suspend output */
		tp->t_state |= TTSTOP;
		break;
	case T_BLOCK:			/* send stop char */
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK|TTXOFF;
		i354startio(tp);
		break;
	case T_RFLUSH:			/* flush input queue */
		if (!(tp->t_state & TBLOCK))
			break;
		/* FALLTHROUGH */
	case T_UNBLOCK:			/* send start char */
		tp->t_state &= ~(TTXOFF | TBLOCK);
		tp->t_state |= TTXON;
		i354startio(tp);
		break;
	case T_TIME:			/* time out */
		tp->t_state &= ~TIMEOUT;
		i354startio(tp);
		break;
	case T_BREAK:			/* send null for .25 sec */
		/* pull transmitter low */
		i354w530(mp->m_ctrl, M_REG5, mp->m_reg5|M_BRK);
		/* disable receiver  */
		i354w530(mp->m_ctrl, M_REG3, mp->m_reg3&~M_RX_EN);
		(void) timeout(i354wakeup, (caddr_t)tp, HZ/4);
		break;
	case T_OUTPUT:			/* start output */
		i354startio(tp);
		break;
	case T_CONNECT:			/* connect to the server */
		i354SetDTR(tp);		/* Do CONNECT request */
		/* enable interrupts */
		i354w530(mp->m_ctrl, M_REG1, M_P_SPEC|M_RX_INT|M_TX_INT_EN|M_EXT_EN);
		/* enable External/Status interrupts by unmasking them in WR15 */
		i354w530(mp->m_ctrl,M_REG15,M_DCD_EN|M_BRK_EN);
		i354SetCarr(tp, mp);	/* Determine connection status */
		break;
	case T_PARM:			/* output parameters */
		ret_val = i354param(tp - i354_tty);
		break;
	case T_DISCONNECT:
		i354ClearDTR(tp);
		break;
	case T_SWTCH:
		break;
	default:
		break;
	};	/* end switch */
	splx(s);
	return(ret_val);
}

/*
 * i354startio() -- start output on an 82530 channel if needed.
 *
 * Get a character from the character queue, output it to the
 * channel and set the BUSY flag. The BUSY flag gets reset by i354intr
 * when the character has been transmitted.
 */
i354startio(tp)
	register struct strtty *tp;
{
	register struct t_buf *tbuf;
	struct	i354cfg *mp;

	mp = (struct i354cfg *) &i354cfg[((int)(tp - i354_tty))];
	DDEBUG(DIO, (CE_CONT, "i354startio:tp=%x, mp=%x\n", tp, mp) );
	/* busy or timing out? or stopped?? */
	if(tp->t_state&(TIMEOUT|BUSY|TTSTOP))	{
		return;			/* wait until a more opportune time */
	}
	if (tp->t_state & (TTXON|TTXOFF)) {
		tp->t_state |= BUSY;
		if (tp->t_state & TTXON) {
			tp->t_state &= ~TTXON;
			outb(mp->m_data, CSTART);
		} else {
			tp->t_state &= ~TTXOFF;
			outb(mp->m_data, CSTOP);
		}
		return;
	}
	tbuf = &tp->t_out;
	if (!(tbuf->bu_ptr && tbuf->bu_cnt)) {
		if (!(CPRES & iasy_output(tp)))
			return;
	}
	tp->t_state |= BUSY;
	outb(mp->m_data, (char)(*(tbuf->bu_ptr++)));
	--(tbuf->bu_cnt);
}

/*
 * i354param() -- set device parameters from system strtty structure
 *
 * The algorithm:
 *	- validate speed
 *	- program the timer
 *	- program for the desired parameters
 */
/* this table must correspond to the values in termio.h */
int	i354baud[] = {
	0,		50,		75,		110,	134,	150,	200,	300,
	600,	1200,	1800,	2400,	4800,	9600,	19200,	0
};
#define MAXBAUDS 16

int
i354param(unit)
	int unit;
{
	register unsigned port;	/* port to reprogram */
	register int s;			/* new speed and mutex var */
	struct strtty *tp;
	int	speed;
	int	f;					/* used for misc temps */
	uint	cmd3, cmd4, cmd5;	/* commands to send to port reg */
	uint	tc;					/* 82530 time constant */

	port = i354cfg[unit].m_ctrl;	/* port address of 82530 */
	tp = &i354_tty[unit];
	s = tp->t_cflag&CBAUD;
	DDEBUG(DINIT, (CE_CONT, "i354param: u=%x, p=%x, tp=%x, s=%x\n",
		unit, port, tp, s) );
	if(s == 0) {
		if (tp->t_cflag & CLOCAL)
			return(EINVAL);
		else {
			/* hang up signal */
			i354ClearDTR(tp);
			return(0);
		}
	}

	if(s != i354cfg[unit].m_speed) {	/* if speed change not redundant... */
		i354cfg[unit].m_speed = s;	/* set to new speed */
		if (s >= MAXBAUDS) 		/* exceeds table */
			return(EINVAL);		/* early retirement */
		speed = i354baud[s];	/* speed <-- actual BAUD rate */
		if (speed == 0) 		/* if invalid entry */
			return(EINVAL);		/* early retirement */
		/*
		 * the following formula is taken from the 82530 technical
		 * manual. It is correct for a 16x clock and an SCC clock
		 * of 9.8304 MHz.
		 */
		tc = (BRG_CONSTANT / (long) speed ) -2;
		DDEBUG(DINIT, (CE_CONT, "i354baud: s=%d, sp=%d, tc=%d....\n", s, speed, tc) );
		s = SPL();
		i354w530(port, M_REG14, M_BRG_SRC); /* disable BAUD rate generator */
		i354w530(port, M_REG12, tc&0xff);
		i354w530(port, M_REG13, tc>>8&0xff);
		i354w530(port, M_REG14, M_BRG_SRC|M_BRG_EN);
		splx(s);
	}

	/*
	 * Set up parameters
	 */
	f = tp->t_cflag;	/* control flag */

	cmd3 = 0;
	cmd4 = M_16X;
	cmd5 = M_TX_EN | (i354cfg[unit].m_reg5&M_DTR);
	switch (f & CSIZE) {
		case CS8: cmd3 |= M_RX_8BPC;
				  cmd5 |= M_TX_8BPC;
				  i354cfg[unit].m_mask = 0xff;
				  break;
		case CS7: cmd3 |= M_RX_7BPC;
				  cmd5 |= M_TX_7BPC;
				  i354cfg[unit].m_mask = 0x7f;
				  break;
		case CS6: cmd3 |= M_RX_6BPC;
				  cmd5 |= M_TX_6BPC;
				  i354cfg[unit].m_mask = 0x3f;
				  break;
		case CS5: cmd3 |= M_RX_5BPC;
				  cmd5 |= M_TX_5BPC;
				  i354cfg[unit].m_mask = 0x1f;
				  break;
	}
	/* we leave E_RX_EN always on for any trips into DMON */
	/* if (f & CREAD) */
		cmd3 |= M_RX_EN;
	if (f & CSTOPB)
		cmd4 |= M_2STOP;
	else
		cmd4 |= M_1STOP;
	if (f & PARENB)
		cmd4 |= M_PAR_EN;
	if (!(f & PARODD))
		cmd4 |= M_PAR_EVEN;
	i354cfg[unit].m_reg3 = cmd3;
	i354cfg[unit].m_reg5 = cmd5;

	DDEBUG(DINIT, (CE_CONT, "i354pset: f=%x, c3=%x, c4=%x, c5=%x....\n",
		f, cmd3, cmd4, cmd5) );
	s = SPL();
	if (i354r530(port, M_REG0) & M_DCD) {
		i354cfg[unit].m_state |= CARRF;
	} else {
		i354cfg[unit].m_state &= ~CARRF;
	}
	i354SetCarr(tp, &i354cfg[unit]);

	i354w530(port, M_REG4, cmd4);
	/* enable interrupts and mark parity as special condition-for modem?*/
	/* i354w530(port, M_REG1, M_P_SPEC|M_RX_INT|M_TX_INT_EN|M_EXT_EN); */
	i354w530(port, M_REG3, cmd3);
	i354w530(port, M_REG5, cmd5);
	splx(s);
	return(0);
}

/*
 *	LOW LEVEL UTILITY ROUTINES
*/
/*
 * i354SetCarr() -- set the carrier state flag correctly
 *
 * "Correctly" is to check the actual carrier state (in l_state) and
 * set the perceived carrier state (in t_state) to either on or off
 * depending on the actual carrier state (in l_state) and on CLOCAL.
 */
i354SetCarr(tp, mp)
	register struct strtty *tp;
	register struct i354cfg *mp;
{
	if (tp->t_cflag & CLOCAL)
		tp->t_state |= CARR_ON;
	else
		if (mp->m_state & CARRF)
			tp->t_state |= CARR_ON;
		else
			tp->t_state &= ~CARR_ON;
}

/*
 * i354wakeup() -- release transmitter output.
 *
 * It is used by the TCSBRK ioctl command.  After .25 sec
 * timeout (see case BREAK in i354proc), this procedure is called.
 *
 */
void i354wakeup(tp)
	struct strtty *tp;
{
	struct i354cfg *mp;
	int s;

	s = SPL();
	mp = (struct i354cfg *) &i354cfg[((int)(tp - i354_tty))];
	i354w530(mp->m_ctrl, M_REG5, mp->m_reg5);
	i354w530(mp->m_ctrl, M_REG3, mp->m_reg3); /* enable receiver */
	splx(s);
}

/*
 *	i354SetDTR() -- assert the DTR line for this port
 */
i354SetDTR(tp)
	struct strtty *tp;
{
	struct i354cfg *mp;
	int s;

	s = SPL();
	mp = &i354cfg[((int)(tp - i354_tty))];
	mp->m_reg5 |= M_DTR;
	i354w530(mp->m_ctrl, M_REG5, mp->m_reg5);
	splx(s);
}

/*
 *	i354ClearDTR() -- deassert the DTR line for this port
 */
i354ClearDTR(tp)
	struct strtty *tp;
{
	struct i354cfg *mp;
	int s;

	s = SPL();
	mp = &i354cfg[((int)(tp - i354_tty))];
	mp->m_reg5 &= ~M_DTR;
	i354w530(mp->m_ctrl, M_REG5, mp->m_reg5);
	splx(s);
}

/*
 * i354r530() -- read register from 82530
 */
i354r530(port, reg)
uint port;
uint reg;
{	register uint	i;

	asm("pushfl");	/* Save interrupt context */
	asm("cli");	/* Disable interrupts */
	outb(port, (char) reg);
	drv_usecwait(10);					/* delay for chip */
	i = inb(port);
	asm("popfl");	/* Restore interrupt context */
	return(i);
}

/*
 * i354w530() -- write into register on the 82530
 */
i354w530(port, reg, value)
uint port;
uint reg;
uint value;
{
	asm("pushfl");	/* Save interrupt context */
	asm("cli");	/* Disable interrupts */
	outb(port, (char) reg);
	drv_usecwait(10);					/* delay for chip */
	outb(port, (char) value);
	asm("popfl");	/* Restore interrupt context */
}

#ifdef lint
/*
 *	Reference each routine that the kernel does, to keep lint happy.
*/
main()
{
	i354init();
	i354intr(0);
	return 0;
}
#endif /*lint*/
