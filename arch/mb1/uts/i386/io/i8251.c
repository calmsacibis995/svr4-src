/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	


#ident	"@(#)mb1:uts/i386/io/i8251.c	1.3.1.1"

#ifndef lint
static char i8251_copyright[] = "Copyright 1987, 1988, 1989 Intel Corp. 462845";
#endif  /* lint */

/*	
 *	
 * This set of procedures implements the console device driver. 
 * Procedures provided:
 *      i8251co     --  prints directly onto the console for kernel putchar
 *      i8251ci     --  reads a character directly from the console
 *      i8251intr   --  service an 8251 interrupt
 *
 * Internal routines are:
 *      i8251proc   --  general command routine that initiates actions
 *      i8251init   --  intitialize devices
 *      i8251param  --  parameter interface
 *      i8251xintr  --  service tx complete interrupt
 *      i8251tmout  --  catch missed interrupts
 *      i8251T_prog --  program the baud rate generator
 *      i8251U_reset--  initial reset for the USART
 *      i8251U_prog --  program line characteristics on the USART
 *      i8251delay  --  causes driver to synchronously delay a bit
 *
 *  These routine implement the 8251 USART/PIT device driver.  This is
 *  intended to provide full support for the 8251
 *  console driver.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/i8251.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#ifndef lint
#include "sys/inline.h"
#endif
#include "sys/immu.h"
#include "sys/clockcal.h"
#include "sys/proc.h"
#include "sys/stream.h"
#include "sys/termio.h"
#include "sys/strtty.h"
#include "sys/clock.h"
#include "sys/sysmacros.h"
#include "sys/cmn_err.h"
#include "sys/iasy.h"
#include "sys/ddi.h"

extern time_t           last_time[];    /* output char watchdog timeout */

#ifdef lint
extern void outb();
#endif
extern void rtnfirm();
extern void signal();
extern void spinwait();
extern void splx();
extern void tenmicrosec();
extern int timeout();
extern void ttrstrt();
#undef wakeup
extern void wakeup();
extern int  ttiocom();
extern struct strtty *iasy_register();
extern void iasy_input();
extern int iasy_output();
extern void iasyhwdep();
extern void iasy_hup();
extern struct streamtab iasyinfo;

void i8251tmout();			

#define	ON	1
#define	OFF	0

/*
 * Critical region defines.  These allow interrupts to be turned off and
 * on as needed.  BEGIN and ENDCRITICAL must bracket the whole of a critical
 * section of code.  Within the brackets interrupts are off.  RESTORE can
 * turn interrupts on for pieces of code that might potentially take a long
 * time, and SAFE can turn them off again.
 */
#define SAFE            (_oldpri = spltty())
#define BEGINCRITICAL   {int _oldpri; SAFE;
#define RESTORE         splx(_oldpri)
#define ENDCRITICAL     RESTORE; }

#define MAXTIME         2               /* 2 sec */
static char             i8251tmrun=0;   /* timer set up? */
struct strtty		*i8251_tty;

extern struct		conssw conssw;	/* console switch */
extern ushort           i8251_cnt;          /* number of 8251s */
extern struct i8251cfg  i8251cfg[];     /* ...and a cfg for each 8251 */
extern ushort           i8251alive[];   /* does this 8251 exist? */
extern ushort           i8251speed[];   /* last programmed baud rate count */
extern ushort           i8251state[];   /* last set of line characteristics */
extern ushort           i8251break[];   /* break detect state */
static ushort           i8251cons = 0xff; /* console index in tty,cfg */

/*
 * i8251restart_co is used by i8251co() to determine if the console USART
 * should be reset and programmed.  This is a debugging service, and all
 * initialization should eventially end up in i8251init(), and i8251restart_co
 * and the associated code should be removed.  Anyways, if it's value is
 * non-zero, then we need to program the baud rate and line characteristics.
 * If it is 0xff, then we should reset the chip as well.
 */
static int              i8251restart_co = 0xff;

/*
 * Baud rate table.  Indexed by the stty baud rate value (and retained in
 * i8251speed[]), it gives the value with which to program the generator.
 */
ushort i8251baud[CBAUD+1] = {
	US_B0,     US_B50,    US_B75,     US_B110,
	US_B134,   US_B150,   US_B200,    US_B300,
	US_B600,   US_B1200,  US_B1800,   US_B2400,
	US_B4800,  US_B9600,  US_B19200,  US_B38400
};

char *i8251type[] = {
	"8251 USART",
	"Onboard USART",
	"iSBX 351"
};

/* If we are using the kernel with pmon, the variable allowdebug */
/* is now set to allow control p to enter the kernel debugger.   */
/* the kernel debugger can change this variable if one desires   */
/* to send control p straight thru.  Unfortunately, pmon is no   */
/* longer usable.                                                */
int     allowdebug = 0;

/*
 * This procedure provides the console out routine for the
 * kernel's putchar (printf) routine.  It merely polls the
 * USART until the TX Ready bit is set and then outputs the character.
 *
 * Note that for debugging purposes, there is an initialization
 * sequence hardcoded into this routine.  This will have to remain
 * until the true driver runs correctly.
 */
i8251co(c)
char	c;
{
	struct  i8251cfg *udef;
	ushort  port;

	/*
	 * If this is the first time through this routine, then we need to
	 * program the baud rate generator and USART;  possibly we need to
	 * do a full reset as well.  See declaration of i8251restart_co above.
	 */
	if (i8251restart_co) {
		if (i8251restart_co == 0xff) {
			/* Set index in tty and cfg structs */
			i8251cons = minor(conssw.co_dev);
		}
		/* Start the timer before programming the USART */
		i8251T_prog((dev_t)i8251cons, 8);
		if (i8251restart_co == 0xff)	/* Full chip reset */
			i8251U_reset(i8251cons);
		/* Program the chip and save the line characteristics used */
		i8251state[i8251cons] = S_BAUDF|S_8BPC|S_2STOP;
		i8251U_prog((dev_t)i8251cons, (char)i8251state[i8251cons],
			(S_TXEN|S_DTR|S_RXEN|S_ER|S_RTS));
		i8251restart_co = 0;
	}
	udef = &i8251cfg[i8251cons];
	port = udef->u_cntrl;
	/* Wait until the USART thinks that it is ready.        */
	while(!(inb(port) & S_TXRDY))
		tenmicrosec();    /* Don't hit the USART too hard */
	/* We got it, output the character. */
	outb(udef->u_data,c);
	/* Add carriage return to newline */
	if (c == '\n')			
		i8251co('\r');	
}


/*
 * This procedure reads a character from the console.
 * It returns -1 if there is no character in the USART receive buffer.
 * Note that interrupts must be disabled so that i8251intr doesn't
 * get the character first.
 *
 */
i8251ci()
{
	struct  i8251cfg *udef;
	ushort  port;

	udef = &i8251cfg[i8251cons];
	port = udef->u_cntrl;
	/* Return -1 if we don't have receive ready */
	if (!(inb(port)&S_RXRDY)) {
		tenmicrosec();    /* Don't hit the USART too hard */
		return -1;
	}
	/* Read the character */
	return inb(udef->u_data);
} 

/*
 * Param detects modifications to the line characteristics and reprograms the
 * USART or PIT as needed.  If successful, return 0; else return errno.
 *
 */
int
i8251param(dev)
dev_t	dev;
{
	struct strtty   *tp;

	unsigned int	flags;  /* to ease access to c_flags */
	ushort		s;      /* 8250 baud rate */
	int		ret_val = 0;

	tp = &i8251_tty[dev];
	flags = tp->t_cflag;
	s = i8251baud[flags&CBAUD];
	if (s == 0)  {
		/*
		 *	Baud rate is zero -- turn off DTR.
		 */
		if (tp->t_cflag&CLOCAL)
			ret_val = EINVAL; /* Oops, not a modem, bad speed */
		else
			outb(i8251cfg[dev].u_cntrl, S_ER); /* turn off DTR */
		return(ret_val);
	} else {
		/*
		 * Set the baud rate
		 */
		if (s != i8251speed[dev]) {
			i8251speed[dev] = s;
			BEGINCRITICAL
			    i8251T_prog(dev, s);
			ENDCRITICAL
		}
	}
	/*
	 * Set up new line characteristics, determined
	 * from the value of t_cflag.  At this point
	 * in time, the only things that you can modify
	 * are bits per character, parity enable, type of
	 * parity, and stop bits.
	 */
	s = S_BAUDF;
	/* Set word length;  reset field first */
	switch (flags & CSIZE) {
	case CS8:
		s |= S_8BPC;
		break;
	case CS7:
		s |= S_7BPC;
		break;
	case CS6:
		s |= S_6BPC;
		break;
	case CS5:
		s |= S_5BPC;
	}
	/* check parity */
	if (flags & PARENB) {
		s |= S_PAREN;
		if (!(flags & PARODD))
			s |= S_PAREVEN;
	}
	if (flags & CSTOPB)
		s |= S_2STOP;
	else
		s |= S_1STOP;
	i8251state[dev] = s;
	BEGINCRITICAL
	    i8251U_prog(dev, (char)s,
		    (char)(S_TXEN|S_DTR|S_ER|S_RTS|(flags&CREAD? S_RXEN : 0)));
	ENDCRITICAL
	return(ret_val);
}


/*
 * i8251intr is the entry point for all interrupts.  There are four different
 *	interrupt types:
 *		Special receive line condition   --  generally a rcv error
 *		Receive char available interrupt --  read the char
 *              Transmit buffer empty interrupt  --  send another char
 *		External status change interrupt --  DCD detect/dropped 
 *
 */
i8251intr(vec)
int	vec;
{
	register ushort	mdev;
	struct strtty   *tp;
	unsigned char   c;
	unsigned char   reason;
	int		again;

	for (mdev=0; mdev < i8251_cnt; mdev++) {
		if((i8251alive[mdev]) &&
		    ((i8251cfg[mdev].in_intr == vec) ||
		    (i8251cfg[mdev].out_intr == vec)) ) {
			break;
		}
	}
	if (mdev >= i8251_cnt)
		return;

	(void) drv_getparm(TIME, &last_time[mdev]);
	tp = &i8251_tty[mdev];
	again = 1;
	while (again--) {
		reason = inb(i8251cfg[mdev].u_cntrl);
		/*
		 * Check output first, since input affects TXRDY (by
		 * initiating echo).
		*/
		if (reason & S_TXRDY)
			i8251xintr(mdev);
		if (reason & (S_RXRDY|S_BRKDET|S_ERRCOND)) {
			char  lbuf[3];    /* local character buffer */
			short lcnt;       /* count of chars in lbuf */

			again = 1;
			(void) drv_setparm(SYSRINT, 1);
			/* get the character        */
			c = inb(i8251cfg[mdev].u_data) & 0xff;
			/*
			 * Reset Parity, Over-run, and framing errors, if any.
			 */
			if (reason & S_ERRCOND) {    /* clear error bits */
				while (!(inb(i8251cfg[mdev].u_cntrl) & S_TXRDY))
					tenmicrosec();
				/* We know CREAD, since we got input interrupt */
				outb(i8251cfg[mdev].u_cntrl,
				     S_ER|S_DTR|S_RTS|S_TXEN|S_RXEN);
			}
			/*
			 * The chip will leave S_BRKDET on until the break is
			 * over.  We can hardly spin-wait for it to go away,
			 * so we note it and ignore it until we notice that
			 * it has gone away.
			 */
			if (reason & S_BRKDET) {
				i8251break[mdev] = 1;
				goto check_modem; /* only first time */
			}
			/*
			 * If break used to be set, do break processing.
			 */
			if (i8251break[mdev]) {
				i8251break[mdev] = 0; /* done, turn flag off */
				if ((tp->t_iflag&(BRKINT|IGNBRK)) == BRKINT) 
					iasy_input((tp), L_BREAK);
				goto check_modem;
			}
			/*
			 * Now done with S_BRKDET processing.  Each break is
			 * preceded by a framing error, so we ignore framing
			 * errors.
			 */
			if (reason & S_FRERROR)
				goto check_modem;
			lcnt = 1;	/* Default to input one byte */
			lbuf[0] = c;
			/*
			 * Now account for parity and over-run errors.
			 * There is nothing to do for over-runs.  The character
			 * is lost.  Parity errors have special processing here.
			 */
			if ((tp->t_iflag & INPCK) && (reason & S_PERROR)) {
				if (tp->t_iflag&IGNPAR)
					goto check_modem;
				if (tp->t_iflag&PARMRK) {
					lbuf[2] = 0377;
					lbuf[1] = 0;
					lcnt = 3;
				} else
					lbuf[0] = 0;
			} else {
				/*
				 * The character is valid.
				 */
				if (tp->t_iflag&ISTRIP)
					lbuf[0] &= 0177;
				else {
					if (c == 0377 && tp->t_iflag&PARMRK) {
						lbuf[1] = 0377;
						lcnt = 2;
					}
				}
				c &= 0177;	/* Make further tests easy */
				if (allowdebug
				&&  c == 0x10
				&&  mdev == i8251cons) { /* debugger trap ^p */
#ifdef DEBUGGER
					debugger(&vec,0);
#else  /* DEBUGGER */
					rtnfirm();
#endif /* DEBUGGER */
					goto check_modem;
				}
				/*
				 * If we are supposed to do xon/xoff protocol,
				 * and the char we got is one of those guys,
				 * take care of it.
				 */
				if (tp->t_iflag & IXON) {
					/*
					 * If we are stopped, and if the char
					 * is a xon, or if we should restart
					 * on any char, resume
					 */
					if (tp->t_state & TTSTOP) {
						/* got a ctl-q or resume char */
						if (c==CSTART
						||  tp->t_iflag&IXANY)
							(void) i8251proc(tp, T_RESUME);
					} else {
						/* not stopped by ctl-s */
						if (c == CSTOP)
							(void) i8251proc(tp,T_SUSPEND);
					}
					if (c == CSTART || c == CSTOP)
						goto check_modem; /*throw away*/
				}  /* end if IXON */
			}
			/*
			 * Stuff the char(s) into buffer and tell the
			 * line discipline
			*/
			if (tp->t_in.bu_ptr == NULL)
				goto check_modem;
			while (lcnt) {
				*tp->t_in.bu_ptr = lbuf[--lcnt];
				tp->t_in.bu_cnt--;
				iasy_input((tp), L_BUF);
			}
		}
	check_modem:
		/*
		 * Now look at carrier on non-local lines.  We do this on every
		 * interrupt because there is no carrier detect or carrier loss
		 * interrupt.
		 */
		if (!(tp->t_cflag & CLOCAL)) {
		    if (tp->t_state & CARR_ON) {
			if (!(reason & S_DSRDY)) {
			(void) drv_setparm(SYSMINT, 1);
				tp->t_state &= ~CARR_ON;
				iasy_hup(tp);                 /* drop DTR in close, not here */
				wakeup((caddr_t)&tp->t_oflag);	
			}
		    } else {
			if (reason & S_DSRDY) {
			(void) drv_setparm(SYSMINT, 1);
				tp->t_state |= CARR_ON;
				wakeup((caddr_t)&tp->t_rdqp);
			}
		    }
		}
	}
}


/*
 * This is logically a part of i8251intr, but that's too big.  This code
 * handles transmit buffer empty interrupts, dealing with current xon/xoff
 * state and availability of more data to transmit.
 *
 */
i8251xintr(mdev)
register int    mdev;
{
	struct strtty   *tp;

	(void) drv_setparm(SYSXINT, 1);
	tp = &i8251_tty[mdev];
	/*
	 * If we are supposed to send an xoff or xon, do it now
	 */
	if (tp->t_state & TTXON) {
		tp->t_state &= ~TTXON;
		tp->t_state |= BUSY;
		outb(i8251cfg[mdev].u_data, CSTART);
	} else {
		if (tp->t_state & TTXOFF) {
			tp->t_state &= ~TTXOFF;
			tp->t_state |= BUSY;
			outb(i8251cfg[mdev].u_data, CSTOP);
		} else {
			/* Otherwise, just try to initiate more output */
			tp->t_state &= ~BUSY;
			(void) i8251proc( tp, T_OUTPUT );
		}
	}
}


/*
 * General command routine that performs device specific operations for
 * generic i/o commands.  All commands are performed with tty level interrupts
 * disabled.
 *
 */
int
i8251proc(tp, cmd)
struct strtty	*tp;
int		cmd;
{
	dev_t           dev;
	int		ret_val = 0;

	BEGINCRITICAL
	    /*
	     * get device number and control port
	     */
		dev = tp->t_dev;
	    /*
	     * based on cmd, do various things to the device
	     */
	    switch (cmd) {
	    case T_TIME:            /* stop sending a break */
		tp->t_state &= ~TIMEOUT;
		outb(i8251cfg[dev].u_cntrl,
		     S_RTS|S_DTR|S_TXEN|((tp->t_cflag&CREAD)?S_RXEN:0));
		goto start;
	    case T_WFLUSH:          /* output flush         */
		/* flush local buffer, next output from iasy_output() */
		tp->t_out.bu_cnt = 0;
	    case T_RESUME:          /* enable output        */
		tp->t_state &= ~TTSTOP;
		/* fall through */
	    case T_OUTPUT:          /* do some output       */
start:
		{   
			register struct t_buf *tbuf;

			/* If we are stopped, doing a break, or busy, no-op */
			if (tp->t_state & (TIMEOUT|TTSTOP|BUSY))
				break;
			/*
			 * Check for characters indirectly through the
			 * linesw table.  If there are any, ship one out.
			 */
			tbuf = &tp->t_out;
			if (!tbuf->bu_cnt)
			    if (!(CPRES & iasy_output(tp)))
				break;
			/* specify busy, and output a char */
			tp->t_state |= BUSY;
			outb( i8251cfg[dev].u_data, *tbuf->bu_ptr++ );
			--(tbuf->bu_cnt);
			/* reset the time so we can catch a missed interrupt */
			(void) drv_getparm(TIME, &last_time[dev]);
			break;
		}
	    case T_CONNECT:         /* called by iasy_open */
		if (!i8251tmrun) {
			(void) drv_getparm(TIME, &last_time[dev]);
			i8251tmout((caddr_t)0);
    		}
		ret_val = i8251param(dev);
    		if ((tp->t_cflag & CLOCAL) || inb(i8251cfg[dev].u_cntrl) & S_DSRDY)
			tp->t_state |= CARR_ON;
		else
			tp->t_state &= ~CARR_ON;
		break;
	    case T_SUSPEND:         /* block on output      */
		tp->t_state |= TTSTOP;
		break;
	    case T_BLOCK:           /* block on input       */
		/*
		 * Either we send a xoff right now, or
		 * we make sure that the next char sent
		 * is an xoff.
		 */
		tp->t_state &= ~TTXON;      /* turn off XON */
		tp->t_state |= TBLOCK;
		if (tp->t_state & BUSY)
			tp->t_state |= TTXOFF;
		else {
			tp->t_state |= BUSY;
			/* send ctl-s */
			outb(i8251cfg[dev].u_data, CSTOP);
		}
		break;
	    case T_RFLUSH:          /* flush input          */
		/* if we are not blocked on input, nothing to do */
		if (!(tp->t_state & TBLOCK))
			break;
		/* FALL THROUGH */
	    case T_UNBLOCK:         /* enable input         */
		tp->t_state &= ~(TTXOFF | TBLOCK);
		/*
		 * Again, if we are not busy, send an xon
		 * right now. Otherwise, make sure that the next
		 * char out will be an xon
		 */
		if (tp->t_state & BUSY)
			tp->t_state |= TTXON;
		else {
			tp->t_state |= BUSY;
			/* send ctl-q */
			outb(i8251cfg[dev].u_data, CSTART);
		}
		break;
	    case T_BREAK:           /* send a break         */
		outb(i8251cfg[dev].u_cntrl,
		     S_SBRK|S_RTS|S_DTR|S_TXEN|((tp->t_cflag&CREAD)?S_RXEN:0));
		(void) timeout(ttrstrt, (caddr_t)tp, HZ/4);
		break;
	    case T_PARM:            /* update parameters    */
		ret_val = i8251param(dev);
		break;
	    case T_DISCONNECT:      /* called by iasy_close */
		/*
		 *	turn off DTR and RTS and disable interrupts.
		 */
		i8251U_prog(dev, (char)i8251state[dev], S_ER);
		/* indicate that we need a light reprogramming */
		if (minor(dev) == i8251cons)
			i8251restart_co = 1;
		break;
	    case T_SWTCH:           /* shell layer switch   */
		/* nothing for us to do */
		break;
	    default:
		break;
	    }
	ENDCRITICAL
	return(ret_val);
}

/*
 * Initialization code that the kernel runs when coming up.  Detect and
 * program any 8251s we see hanging around.
 *
 */
i8251init()
{
	register ushort	testval;
	ushort		dev;
	char		b_type;
	int		i;

	for (dev=0; dev < i8251_cnt; dev++) {
		i8251alive[dev] = 0; /* Pessimistic assumption */
		/* Probe for the board. */
		if (i8251cfg[dev].u_cntrl != 0) {  /* Is board configured? */
			spinwait(10);   /* In case it's the console */
			/* Program the timer for a LONG time */
			i8251T_prog((dev_t)dev, 0xffff);
			tenmicrosec();   /* Burp */
			testval = (short)inb(i8251cfg[dev].t_data);
			tenmicrosec();   /* Burp */
			testval |= ((short)inb(i8251cfg[dev].t_data)<<8);
			tenmicrosec();   /* Burp */
			if ((testval == 0) || (testval == 0xffff)) {
				testval = (short)inb(i8251cfg[dev].t_data);
				tenmicrosec();   /* Burp */
				testval|=((short)inb(i8251cfg[dev].t_data)<<8);
				tenmicrosec();   /* Burp */
			}
			if ((testval != 0) && (testval != 0xffff)) {
				i8251alive[dev] = 1;   /* We found a live one */
				if (dev == minor(conssw.co_dev))
				    i8251cons = dev; /* Save console's index */
				/*
				 * Program the device to be benign.
				 * If it is the console device, i8251co()
				 * will turn it back on during printf below.
				 */
				i8251T_prog((dev_t)dev, US_B9600);
				i8251U_reset(dev);
				i8251state[dev] = S_BAUDF|S_8BPC|S_1STOP;
				i8251U_prog((dev_t)dev, (char)i8251state[dev], S_ER);
				b_type = i8251cfg[dev].b_type;
				if (b_type > N8251TYPES) {
					b_type = 0;
				}
			}
			/* Be sure i8251co() reprograms chip */
			if (dev == i8251cons)
				i8251restart_co |= 1;
			tenmicrosec();
			cmn_err(CE_CONT,"%s (port %x)%s found.\n", i8251type[b_type],
			i8251cfg[dev].u_data, i8251alive[dev] ? "" : " NOT" );
			i8251speed[dev] = 0;   /* Not programmed */
		}
	}
	/*
	**	register terminal server.
	*/
	i8251_tty = iasy_register(0, (int)i8251_cnt, i8251proc, iasyhwdep);
	for (i = 0;  i < cdevcnt; i++) {
		if (cdevsw[i].d_str == &iasyinfo)
			break;
	}
	if (i >= cdevcnt)
		return;
	consregister(i8251ci, i8251co, makedevice(i, 0));
}

/* ARGSUSED */
void
i8251tmout(notused)
caddr_t notused; /* dummy variable that callout will supply */
{
	time_t diff;
	register unsigned    dev;

	for (dev=0; dev < i8251_cnt; dev++) {
		(void) drv_getparm(TIME, &diff);
		diff -= last_time[dev];
		if (diff > MAXTIME)  {
			/*
			 * Assume we missed an interrupt:
			 * force output of next char
			 */
			i8251intr(i8251cfg[dev].out_intr);
		}
	}
	i8251tmrun = 1;
	(void) timeout(i8251tmout, (caddr_t)0, HZ);
}


/*
 * This procedure programs an 8253/8254 PIT for a baud rate generator.
 *
 */
i8251T_prog(dev, speed)
dev_t	dev;
ushort	speed;
{
	register struct     i8251cfg *udef = &i8251cfg[dev];

	/* First, program the mode for baud rate generation */
	outb(udef->t_cntrl,(char) (RATEMD0 | (udef->t_number << 6)));
	tenmicrosec();
	/* Next, program the rate. */
	outb(udef->t_data,(char)(speed & 0xff));
	tenmicrosec();
	outb(udef->t_data,(char)(speed>>8));
	tenmicrosec();
}

/*
 * This procedure does the initial reset on an 8251 USART.
 *
 * Something to note in this routine:
 * The USART has an interesting initialization sequence.
 *    a) Send 4 zeroes to the control port.  This is to
 *       ensure that the USART is ready to take a reset command.
 *    b) Do a reset on the USART and program the mode.
 * This sets up the USART so that the next software reset will work.
 *
 */
i8251U_reset(dev)
ushort	dev;
{   
	register struct     i8251cfg *udef = &i8251cfg[dev];
	register ushort	cport;

	cport = udef->u_cntrl;
	/*
	 * First, we send 4 zero's to the USART,
	 * this will make sure it is ready to take our reset
	 */
	outb(cport,0);	
	tenmicrosec();
	outb(cport,0);
	tenmicrosec();
	outb(cport,0);
	tenmicrosec();
	outb(cport,0);
	tenmicrosec();
	/* Now we can reset, program it. */
	outb(cport, 0x40);
	tenmicrosec();
	outb(cport, (S_BAUDF|S_8BPC|S_2STOP));
	tenmicrosec();
	outb(cport, (S_ER));
	tenmicrosec();
}


i8251U_prog(dev, mode, command)
dev_t	dev;
char	mode, command;
{
	register struct     i8251cfg *udef = &i8251cfg[dev];
	register ushort      cport;

	cport = udef->u_cntrl;
	outb(cport, 0x40);
	tenmicrosec();
	outb(cport, mode);
	tenmicrosec();
	outb(cport, command);
	tenmicrosec();
}
