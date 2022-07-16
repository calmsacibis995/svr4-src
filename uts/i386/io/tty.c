/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-io:tty.c	1.3"

/*
 * general TTY subroutines
 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/proc.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/termio.h"
#include "sys/termios.h"
#include "sys/ttold.h"
#include "sys/sysinfo.h"

/*
 * tty low and high water marks
 * high < TTYHOG
 */
int	tthiwat[16] = {
	0, 70, 70, 70,
	70, 70, 70, 120,
	120, 180, 180, 240,
	240, 240, 100, 100,
};
int	ttlowat[16] = {
	0, 20, 20, 20,
	20, 20, 20, 40,
	40, 60, 60, 80,
	80, 80, 50, 50,
};

char	ttcchar[NCC] = {
	CINTR,
	CQUIT,
	CERASE,
	CKILL,
	CEOF,
	0,
	0,
	0
};

/* null clist header */
struct clist ttnulq;

/* canon buffer */
char	canonb[CANBSIZ];
/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */
char	maptab[] = {
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,'|',000,000,000,000,000,'`',
	'{','}',000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,'~',000,
	000,'A','B','C','D','E','F','G',
	'H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W',
	'X','Y','Z',000,000,000,000,000,
};

/*
 * common ioctl tty code
 */
ttiocom(tp, cmd, arg, mode)
register struct tty *tp;
{
	register flag;
	struct termio cb;
	/* XENIX Support */
	struct sgttyb tb;	/* for Merged Product ioctl extensions */
	/* End XENIX Support */
	unsigned short t;       /* V7 */
	struct tc tun;          /* V7 */

	switch (cmd) {
	case IOCTYPE:
		u.u_rval1 = TIOC;
		break;

	case TCSETAW:
	case TCSETAF:
		ttywait(tp);
		if (cmd == TCSETAF)
			ttyflush(tp, (FREAD|FWRITE));
	case TCSETA:
		if (copyin((caddr_t)arg, (caddr_t)&cb, sizeof cb)) {
			u.u_error = EFAULT;
			break;
		}
		if (tp->t_line != cb.c_line) {
			if (cb.c_line < 0 || cb.c_line >= linecnt) {
				u.u_error = EINVAL;
				break;
			}
			(*linesw[tp->t_line].l_ioctl)(tp, LDCLOSE, 0, mode);
		}
		flag = tp->t_lflag;
		tp->t_iflag = cb.c_iflag;
		tp->t_oflag = cb.c_oflag;
		tp->t_cflag = cb.c_cflag;
		tp->t_lflag = cb.c_lflag;
		bcopy((caddr_t)cb.c_cc, (caddr_t)tp->t_cc, NCC);
		if (tp->t_line != cb.c_line) {
			tp->t_line = cb.c_line;
			(*linesw[tp->t_line].l_ioctl)(tp, LDOPEN, 0, mode);
		} else if (tp->t_lflag != flag) {
			(*linesw[tp->t_line].l_ioctl)(tp, LDCHG, flag, mode);
		}
		return(1);

	case TCGETA:
		cb.c_iflag = tp->t_iflag;
		cb.c_oflag = tp->t_oflag;
		cb.c_cflag = tp->t_cflag;
		cb.c_lflag = tp->t_lflag;
		cb.c_line = tp->t_line;
		bcopy((caddr_t)tp->t_cc, (caddr_t)cb.c_cc, NCC);
		if (copyout((caddr_t)&cb, (caddr_t)arg, sizeof cb))
			u.u_error = EFAULT;
		break;

	case TCSBRK:
		ttywait(tp);
		if (arg == 0)
			(*tp->t_proc)(tp, T_BREAK);
		break;

	case TCXONC:
		switch (arg) {
		case 0:
			(*tp->t_proc)(tp, T_SUSPEND);
			break;
		case 1:
			(*tp->t_proc)(tp, T_RESUME);
			break;
		case 2:
			(*tp->t_proc)(tp, T_BLOCK);
			break;
		case 3:
			(*tp->t_proc)(tp, T_UNBLOCK);
			break;
		default:
			u.u_error = EINVAL;
		}
		break;

	case TCFLSH:
		switch (arg) {
		case 0:
		case 1:
		case 2:
			ttyflush(tp, (arg - FOPEN)&(FREAD|FWRITE));
			break;

		default:
			u.u_error = EINVAL;
		}
		break;

/* conversion aide only */
	case TIOCSETP:
		ttywait(tp);
		ttyflush(tp, (FREAD|FWRITE));
		if (copyin((caddr_t)arg, (caddr_t)&tb, sizeof(tb))) {
			u.u_error = EFAULT;
			break;
		}
		tp->t_iflag = 0;
		tp->t_oflag = 0;
		tp->t_lflag = 0;
		tp->t_cflag = (tb.sg_ispeed&CBAUD)|CREAD;
		if ((tb.sg_ispeed&CBAUD)==B110)
			tp->t_cflag |= CSTOPB;
		tp->t_cc[VERASE] = tb.sg_erase;
		tp->t_cc[VKILL] = tb.sg_kill;
		flag = tb.sg_flags;
		if (flag&O_HUPCL)
			tp->t_cflag |= HUPCL;
		if (flag&O_XTABS)
			tp->t_oflag |= TAB3;
		else if (flag&O_TBDELAY)
			tp->t_oflag |= TAB1;
		if (flag&O_LCASE) {
			tp->t_iflag |= IUCLC;
			tp->t_oflag |= OLCUC;
			tp->t_lflag |= XCASE;
		}
		if (flag&O_ECHO)
			tp->t_lflag |= ECHO;
		if (!(flag&O_NOAL))
			tp->t_lflag |= ECHOK;
		if (flag&O_CRMOD) {
			tp->t_iflag |= ICRNL;
			tp->t_oflag |= ONLCR;
			if (flag&O_CR1)
				tp->t_oflag |= CR1;
			if (flag&O_CR2)
				tp->t_oflag |= ONOCR|CR2;
		} else {
			tp->t_oflag |= ONLRET;
			if (flag&O_NL1)
				tp->t_oflag |= CR1;
			if (flag&O_NL2)
				tp->t_oflag |= CR2;
		}
		if (flag&O_RAW) {
			tp->t_cc[VTIME] = 1;
			tp->t_cc[VMIN] = 6;
			tp->t_iflag &= ~(ICRNL|IUCLC);
			tp->t_cflag |= CS8;
		} else {
			tp->t_cc[VEOF] = CEOF;
			tp->t_cc[VEOL] = 0;
			tp->t_cc[VEOL2] = 0;
			tp->t_iflag |= BRKINT|IGNPAR|ISTRIP|IXON|IXANY;
			tp->t_oflag |= OPOST;
			tp->t_cflag |= CS7|PARENB;
			tp->t_lflag |= ICANON|ISIG;
		}
		tp->t_iflag |= INPCK;
		if (flag&O_ODDP)
			if (flag&O_EVENP)
				tp->t_iflag &= ~INPCK;
			else
				tp->t_cflag |= PARODD;
		if (flag&O_VTDELAY)
			tp->t_oflag |= FFDLY;
		if (flag&O_BSDELAY)
			tp->t_oflag |= BSDLY;
		return(1);

	/*
	 * Sets the parameters associated with the terminal,
	 * but does not delay or flush input.
	 */
	case TIOCSETN:
		if (copyin((caddr_t)arg, (caddr_t) &tb, sizeof(tb)) == -1) {
			u.u_error = EFAULT;
			break;
		}
		tp->t_iflag = 0;
		tp->t_oflag = 0;
		tp->t_lflag = 0;
		tp->t_cflag &= CLOCAL|HUPCL;
 		tp->t_cflag |= (tb.sg_ispeed&CBAUD)|CREAD;
		if ((tb.sg_ispeed&CBAUD)==B110)
			tp->t_cflag |= CSTOPB;
		tp->t_cc[VERASE] = tb.sg_erase;
		tp->t_cc[VKILL] = tb.sg_kill;
		flag = tb.sg_flags;
/*****          if (flag&O_HUPCL)               *****/
/*****                  tp->t_cflag |= HUPCL;   *****/
		if (flag&O_XTABS)
			tp->t_oflag |= TAB3;
		else if (flag&O_TBDELAY)
			if(flag&O_TAB1)
			    tp->t_oflag |= TAB1;
			else tp->t_oflag |= TAB2;
		if (flag&O_LCASE) {
			tp->t_iflag |= IUCLC;
			tp->t_oflag |= OLCUC;
			tp->t_lflag |= XCASE;
		}
		if (flag&O_ECHO)
			tp->t_lflag |= ECHO|ECHOK|ECHOE;
		if (flag&O_CRMOD) {
			tp->t_iflag |= ICRNL;
			tp->t_oflag |= ONLCR;
			if (flag&O_CR1)
				tp->t_oflag |= CR1;
			if (flag&O_CR2)
				tp->t_oflag |= ONOCR|CR2;
		} else {
			tp->t_oflag |= ONLRET;
			if (flag&O_NL1)
				tp->t_oflag |= CR1;
			if (flag&O_NL2)
				tp->t_oflag |= CR2;
		}
		if (tp->t_mstate == 0)
			tp->t_iflag |= ISTRIP;
		if (flag&O_RAW || flag&O_CBREAK) {	/* raw mode */
			if(tp->t_lflag&ICANON){
			    tp->t_cc[VCEOF] = tp->t_cc[VEOF];
			    tp->t_cc[VCEOL] = tp->t_cc[VEOL];
			}
			tp->t_cc[VTIME] = 0;
			tp->t_cc[VMIN] = 1;
			if (flag&O_RAW) {
				tp->t_iflag &= ~(ISTRIP|ICRNL|IUCLC);
				tp->t_cflag |= CS8;
			} else {
				tp->t_iflag |= BRKINT|IGNPAR|IXON|IXANY;
				tp->t_oflag |= OPOST;
				tp->t_cflag |= CS7|PARENB;
				tp->t_lflag |= ISIG;
			}
		} else {        /* cooked mode */
			tp->t_cc[VEOF] = tp->t_cc[VCEOF];
			tp->t_cc[VEOL] = tp->t_cc[VCEOL];
			tp->t_iflag |= BRKINT|IGNPAR|IXON|IXANY;
			tp->t_oflag |= OPOST;
			tp->t_cflag |= CS7|PARENB;
			tp->t_lflag |= ICANON|ISIG;
		}
		tp->t_iflag |= INPCK;
		if (flag&O_ODDP)
			if (flag&O_EVENP) {
				tp->t_iflag &= ~INPCK;
				tp->t_cflag &= ~(PARENB|CSIZE);
				tp->t_cflag |= CS8;
			} else {
				tp->t_cflag |= PARODD;
			}
		if (flag&O_VTDELAY)
			tp->t_oflag |= FFDLY;
		if (flag&O_BSDELAY)
			tp->t_oflag |= BSDLY;
		if ((tp->t_iflag & IXON) == 0)
			(*tp->t_proc)(tp, T_RESUME);	/* avoid ^S hang */

		return(1);

	case TIOCGETP:
		tb.sg_ispeed = tp->t_cflag&CBAUD;
		tb.sg_ospeed = tb.sg_ispeed;
		tb.sg_erase = tp->t_cc[VERASE];
		tb.sg_kill = tp->t_cc[VKILL];
		flag = 0;
		if (tp->t_cflag&HUPCL)
			flag |= O_HUPCL;
		if (!(tp->t_lflag&ICANON))
			flag |= O_RAW;
		if (tp->t_lflag&XCASE)
			flag |= O_LCASE;
		if (tp->t_lflag&ECHO)
			flag |= O_ECHO;
		if (!(tp->t_lflag&ECHOK))
			flag |= O_NOAL;
		if (tp->t_cflag&PARODD)
			flag |= O_ODDP;
		else if (tp->t_iflag&INPCK)
			flag |= O_EVENP;
		else
			flag |= O_ODDP|O_EVENP;
		if (tp->t_oflag&ONLCR) {
			flag |= O_CRMOD;
			if (tp->t_oflag&CR1)
				flag |= O_CR1;
			if (tp->t_oflag&CR2)
				flag |= O_CR2;
		} else {
			if (tp->t_oflag&CR1)
				flag |= O_NL1;
			if (tp->t_oflag&CR2)
				flag |= O_NL2;
		}
		if ((tp->t_oflag&TABDLY)==TAB3)
			flag |= O_XTABS;
		else if (tp->t_oflag&TAB1)
			flag |= O_TBDELAY;
		if (tp->t_oflag&FFDLY)
			flag |= O_VTDELAY;
		if (tp->t_oflag&BSDLY)
			flag |= O_BSDELAY;
		tb.sg_flags = flag;
		if (copyout((caddr_t)&tb, (caddr_t)arg, sizeof(tb)))
			u.u_error = EFAULT;
		break;

	/* changes the settings of special characters */
	case TIOCSETC:
		if (copyin((caddr_t)arg, (caddr_t) &tun, sizeof(tun)) == -1)
			u.u_error = EFAULT;
		tp->t_cc[VINTR] = tun.t_intrc;
		tp->t_cc[VQUIT] = tun.t_quitc;
		tp->t_cc[VCEOF] = tun.t_eofc;
		tp->t_cc[VCEOL] = tun.t_brkc;
		if(tp->t_lflag&ICANON){
		    tp->t_cc[VEOF] = tun.t_eofc;
		    tp->t_cc[VEOL] = tun.t_brkc;
		}
		/* changing start & stop not supported */
		break;

	/* Gets the current settings of special characters */
	case TIOCGETC:
		tun.t_intrc = tp->t_cc[VINTR];
		tun.t_quitc = tp->t_cc[VQUIT];
		tun.t_eofc = tp->t_cc[VCEOF];
		tun.t_brkc = tp->t_cc[VCEOL];
		tun.t_startc = CSTART;
		tun.t_stopc = CSTOP;
		if (copyout((caddr_t)&tun, (caddr_t)arg, sizeof(tun)) == -1) {
			u.u_error = EFAULT;
			break;
		}
		break;

	/*
	 * Sets exclusive usr mode: no further opens
	 * are permitted until the file has been closed.
	 */
	case TIOCEXCL:
		tp->t_lflag |= XCLUDE;
		break;

	/* Resets exclusive use mode: further opens are permitted */
	case TIOCNXCL:
		tp->t_lflag &= ~XCLUDE;
		break;

	/*
	 * causes the terminal to hangup when the
	 * file is closed for the last time.
	 */
	case TIOCHPCL:
		tp->t_cflag |= HUPCL;
		break;

	/* Flushes all characters waiting in input or output queues */
	case TIOCFLUSH:
		ttyflush(tp, (FREAD|FWRITE));
		break;

	/* Gets the discipline associated with the terminal */
	case TIOCGETD:
		t = tp->t_line;
		if (copyout((caddr_t)&t, (caddr_t)arg, sizeof(t)) == -1)
			u.u_error = EFAULT;
		break;

	/* Sets the line descipline associated with the terminal */
	case TIOCSETD:
		if (copyin((caddr_t)arg, (caddr_t)&t, sizeof(t)) == -1) {
			u.u_error = EFAULT;
			break;
		}
		if ((int)t >= linecnt) {
			u.u_error = ENXIO;
			break;
		}
		if (tp->t_line)
			(*linesw[tp->t_line].l_ioctl)(tp, LDCLOSE, (faddr_t)0, mode);
		if (t)
			(*linesw[tp->t_line].l_ioctl)(tp, LDOPEN, (faddr_t)0, mode);
		if (u.u_error == 0)
			tp->t_line = t;
		break;

	/* returns a non-zero value if there are chars in the input queue */
	case FIORDCHK:
		if ((tp->t_state&CARR_ON) == 0)
			break;
		/*
		 * Check the character count in canonical queue.
		 */

		if(tp->t_canq.c_cc != 0)
			u.u_rval1 = 1;
		/*
		 * Otherwise, check the character count in raw queue.
		 */

		else if((tp->t_lflag&ICANON) == 0 && tp->t_rawq.c_cc > 0)
			u.u_rval1 = 1;
		/*
		 * Lastly, check the delimiter count.
		 */

		else if((tp->t_lflag&ICANON) != 0 && tp->t_delct > 0)
			u.u_rval1 = 1;
		break;	/* return 0 if non of above, 1 otherwise */

	case DIOCGETP:	/* Retained for backwards compatibility. */
	case DIOCSETP:	/* This function is a No-op.		 */
		break;
		

	default:
		if ((cmd&IOCTYPE) == LDIOC)
			(*linesw[tp->t_line].l_ioctl)(tp, cmd, arg, mode);
		else
			u.u_error = EINVAL;
		break;
	}
	return(0);
}

ttinit(tp)
register struct tty *tp;
{
	tp->t_line = 0;
	tp->t_iflag = 0;
	tp->t_oflag = 0;
	tp->t_cflag = SSPEED|CS8|CREAD|HUPCL;
	tp->t_lflag = 0;
	bcopy((caddr_t)ttcchar, (caddr_t)&tp->t_cc[0], NCC);
}

ttywait(tp)
register struct tty *tp;
{
	register int	oldpri;
	static	int	rate[] =
	{
		HZ+1 ,	/* avoid divide-by-zero, as well as unnecessary delay */
		50 ,
		75 ,
		110 ,
		134 ,
		150 ,
		200 ,
		300 ,
		600 ,
		1200 ,
		1800 ,
		2400 ,
		4800 ,
		9600 ,
		19200 ,
		38400 ,
	} ;

	oldpri = spltty();
	while (tp->t_outq.c_cc || (tp->t_state&(BUSY|TIMEOUT))) {
		tp->t_state |= TTIOW;
		sleep((caddr_t)&tp->t_oflag, TTOPRI);
	}
	splx(oldpri);
				/* delay 11 bit times to allow uart to empty */
				/* add one to allow for truncation */
				/* add one to allow for partial clock tick */
	delay(1+1+11*HZ/rate[tp->t_cflag&CBAUD]) ;
}

/*
 * flush TTY queues
 */
ttyflush(tp, cmd)
register struct tty *tp;
{
	register struct cblock *cp;
	register s;

	if (cmd&FWRITE) {
		while ((cp = getcb(&tp->t_outq)) != NULL)
			putcf(cp);
		(*tp->t_proc)(tp, T_WFLUSH);
		if (tp->t_state&OASLP) {
			tp->t_state &= ~OASLP;
			wakeup((caddr_t)&tp->t_outq);
		}
		if (tp->t_state&TTIOW) {
			tp->t_state &= ~TTIOW;
			wakeup((caddr_t)&tp->t_oflag);
		}
	}
	if (cmd&FREAD) {
		while ((cp = getcb(&tp->t_canq)) != NULL)
			putcf(cp);
		s = spltty();
		while ((cp = getcb(&tp->t_rawq)) != NULL)
			putcf(cp);
		tp->t_delct = 0;
		splx(s);
		(*tp->t_proc)(tp, T_RFLUSH);
		if (tp->t_state&IASLP) {
			tp->t_state &= ~IASLP;
			wakeup((caddr_t)&tp->t_rawq);
		}
	}
}

/*
 * Transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 */
canon(tp)
register struct tty *tp;
{
	register char *bp;
	register c, esc;
	register int	oldpri;
	register struct cblock *cb;

	oldpri = spltty();
	/* If the character count on the raw queue is 0, make
	   the delimeter count 0. */
	if (tp->t_rawq.c_cc == 0)
		tp->t_delct = 0;
	/*
	  If we don't have any delimeters in the raw queue (t_delct==0),
	  we can't do any canonical processing.
	*/
	while (tp->t_delct == 0) {
		/* If we have no carrier, or NO DELAY was specified, just 
		   restore the execution priority and return (0). */
		if (!(tp->t_state&CARR_ON) || (u.u_fmode&FNDELAY)) {
			splx(oldpri);
			return;
		}
		if (!(tp->t_lflag&ICANON) && tp->t_cc[VMIN]==0) {
			if (tp->t_cc[VTIME]==0)
				break;
			tp->t_state &= ~RTO;
			if (!(tp->t_state&TACT))
				tttimeo(tp);
		}
		tp->t_state |= IASLP;
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	if (!(tp->t_lflag&ICANON)) {
		while (cb = getcb(&tp->t_rawq))
			putcb(cb, &tp->t_canq);
		tp->t_delct = 0;
		splx(oldpri);
		return;
	}
	splx(oldpri);
	bp = canonb;
	esc = 0;
	while ((c=getc(&tp->t_rawq)) >= 0) {
		if (!esc) {
			if (c == '\\') {
				esc++;
			} else if (c == tp->t_cc[VERASE]) {
				if (bp > canonb)
					bp--;
				continue;
			} else if (c == tp->t_cc[VKILL]) {
				bp = canonb;
				continue;
			} else if (c == tp->t_cc[VEOF]) {
				break;
			}
		} else {
			esc = 0;
			if (c == tp->t_cc[VERASE] ||
			    c == tp->t_cc[VKILL] ||
			    c == tp->t_cc[VEOF])
				bp--;
			else if (tp->t_lflag&XCASE) {
				if ((c < 0200) && maptab[c]) {
					bp--;
					c = maptab[c];
				} else if (c == '\\')
					continue;
			} else if (c == '\\')
				esc++;
		}
		*bp++ = c;
		if (c == '\n' || c == tp->t_cc[VEOL] || c == tp->t_cc[VEOL2])
			break;
		if (bp >= &canonb[CANBSIZ])
			bp--;
	}
	tp->t_delct--;
	c = bp - canonb;
	sysinfo.canch += c;
	bp = canonb;
/* faster copy ? */
	while (c--)
		putc(*bp++, &tp->t_canq);
	return;
}

/*
 * Restart typewriter output following a delay timeout.
 * The name of the routine is passed to the timeout
 * subroutine and it is called during a clock interrupt.
 */
ttrstrt(tp)
register struct tty *tp;
{

	(*tp->t_proc)(tp, T_TIME);
}

