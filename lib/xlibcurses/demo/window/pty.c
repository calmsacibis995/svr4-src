/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:demo/window/pty.c	1.1"
/*	pty.c	4.21	82/03/23	*/

/*
 * Pseudo-teletype Driver
 * (Actually two drivers, requiring two entries in 'cdevsw')
 */
#include "pty.h"

#if NPTY > 0
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/tty.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/proc.h"

#undef	NPTY
#define	NPTY	32		/* crude XXX */

#define BUFSIZ 100		/* Chunk size iomoved from user */

/*
 * pts == /dev/tty[pP]?
 * ptc == /dev/ptp[pP]?
 */
struct	tty pt_tty[NPTY];
struct	pt_ioctl {
	int	pt_flags;
	int	pt_gensym;
	struct	proc *pt_selr, *pt_selw;
	int	pt_send;
} pt_ioctl[NPTY];

#define	PF_RCOLL	0x01
#define	PF_WCOLL	0x02
#define	PF_NBIO		0x04
#define	PF_PKT		0x08		/* packet mode */
#define	PF_STOPPED	0x10		/* user told stopped */
#define	PF_REMOTE	0x20		/* remote and flow controlled input */
#define	PF_NOSTOP	0x40

/*ARGSUSED*/
ptsopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;

	if (minor(dev) >= NPTY) {
		u.u_error = ENXIO;
		return;
	}
	tp = &pt_tty[minor(dev)];
	if ((tp->t_state & ISOPEN) == 0) {
		ttychars(tp);		/* Set up default chars */
		tp->t_flags = 0;	/* No features (nor raw mode) */
	} else if (tp->t_state&XCLUDE && u.u_uid != 0) {
		u.u_error = EBUSY;
		return;
	}
	if (tp->t_oproc)			/* Ctrlr still around. */
		tp->t_state |= CARR_ON;
	while ((tp->t_state & CARR_ON) == 0) {
		tp->t_state |= WOPEN;
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	(*linesw[tp->t_line].l_open)(dev, tp);
}

ptsclose(dev)
	dev_t dev;
{
	register struct tty *tp;

	tp = &pt_tty[minor(dev)];
	(*linesw[tp->t_line].l_close)(tp);
	ttyclose(tp);
}

ptsread(dev)
	dev_t dev;
{
	register struct tty *tp = &pt_tty[minor(dev)];
	register struct pt_ioctl *pti = &pt_ioctl[minor(dev)];

again:
	if (pti->pt_flags & PF_REMOTE) {
		while (tp == u.u_ttyp && u.u_procp->p_pgrp != tp->t_pgrp) {
			if (u.u_signal[SIGTTIN] == SIG_IGN ||
			    u.u_signal[SIGTTIN] == SIG_HOLD ||
	/*
			    (u.u_procp->p_flag&SDETACH) ||
	*/
			    u.u_procp->p_flag&SVFORK)
				return;
			gsignal(u.u_procp->p_pgrp, SIGTTIN);
			sleep((caddr_t)&lbolt, TTIPRI);
		}
		if (tp->t_rawq.c_cc == 0) {
#ifdef notdef
			if (tp->t_state & NBIO) {
				u.u_error = EWOULDBLOCK;
				return;
			}
#endif
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
			goto again;
		}
		while (tp->t_rawq.c_cc > 1 && passc(getc(&tp->t_rawq)) >= 0)
			;
		if (tp->t_rawq.c_cc == 1)
			(void) getc(&tp->t_rawq);
		if (tp->t_rawq.c_cc)
			return;
	} else
		if (tp->t_oproc)
			(*linesw[tp->t_line].l_read)(tp);
	wakeup((caddr_t)&tp->t_rawq.c_cf);
#ifdef notdef
	if (pti->pt_selw) {
		selwakeup(pti->pt_selw, pti->pt_flags & PF_WCOLL);
		pti->pt_selw = 0;
		pti->pt_flags &= ~PF_WCOLL;
	}
#endif
}

/*
 * Write to pseudo-tty.
 * Wakeups of controlling tty will happen
 * indirectly, when tty driver calls ptsstart.
 */
ptswrite(dev)
	dev_t dev;
{
	register struct tty *tp;

	tp = &pt_tty[minor(dev)];
	if (tp->t_oproc)
		(*linesw[tp->t_line].l_write)(tp);
}

/*
 * Start output on pseudo-tty.
 * Wake up process selecting or sleeping for input from controlling tty.
 */
ptsstart(tp)
	struct tty *tp;
{
	register struct pt_ioctl *pti = &pt_ioctl[minor(tp->t_dev)];

	if (tp->t_state & TTSTOP)
		return;
	if (pti->pt_flags & PF_STOPPED) {
		pti->pt_flags &= ~PF_STOPPED;
#ifdef notdef
		pti->pt_send = TIOCPKT_START;
#endif
	}
	ptcwakeup(tp);
}

ptcwakeup(tp)
	struct tty *tp;
{
	struct pt_ioctl *pti = &pt_ioctl[minor(tp->t_dev)];

	if (pti->pt_selr) {
#ifdef notdef
		selwakeup(pti->pt_selr, pti->pt_flags & PF_RCOLL);
#endif
		pti->pt_selr = 0;
		pti->pt_flags &= ~PF_RCOLL;
	}
	wakeup((caddr_t)&tp->t_outq.c_cf);
}

/*ARGSUSED*/
ptcopen(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	struct pt_ioctl *pti;

	if (minor(dev) >= NPTY) {
		u.u_error = ENXIO;
		return;
	}
	tp = &pt_tty[minor(dev)];
	if (tp->t_oproc) {
		u.u_error = EIO;
		return;
	}
	tp->t_oproc = ptsstart;
	if (tp->t_state & WOPEN)
		wakeup((caddr_t)&tp->t_rawq);
	tp->t_state |= CARR_ON;
	pti = &pt_ioctl[minor(dev)];
	pti->pt_flags = 0;
	pti->pt_send = 0;
}

ptcclose(dev)
	dev_t dev;
{
	register struct tty *tp;

	tp = &pt_tty[minor(dev)];
	if (tp->t_state & ISOPEN)
		gsignal(tp->t_pgrp, SIGHUP);
	tp->t_state &= ~CARR_ON;	/* virtual carrier gone */
	flushtty(tp, FREAD|FWRITE);
	tp->t_oproc = 0;		/* mark closed */
}

ptcread(dev)
	dev_t dev;
{
	register struct tty *tp;
	struct pt_ioctl *pti;

	tp = &pt_tty[minor(dev)];
	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return;
	pti = &pt_ioctl[minor(dev)];
	if (pti->pt_flags & PF_PKT) {
		if (pti->pt_send) {
			(void) passc(pti->pt_send);
			pti->pt_send = 0;
			return;
		}
		(void) passc(0);
	}
	while (tp->t_outq.c_cc == 0 || (tp->t_state&TTSTOP)) {
#ifdef notdef
		if (pti->pt_flags&PF_NBIO) {
			u.u_error = EWOULDBLOCK;
			return;
		}
#endif
		sleep((caddr_t)&tp->t_outq.c_cf, TTIPRI);
	}
#ifdef CCA_PAGE
	while(tp->t_outq.c_cc) {
		register int c;
		c = firstc(&tp->t_outq);
		if ((c&0177) == '\n') {
			ntpage(tp);
			if (tp->t_state&TTSTOP)
				break;
		}
		c = getc(&tp->t_outq);
		if (passc(c) < 0)
			break;
	}
#else
	while (tp->t_outq.c_cc && passc(getc(&tp->t_outq)) >= 0)
		;
#endif
	if (tp->t_outq.c_cc <= TTLOWAT(tp)) {
		if (tp->t_state&ASLEEP) {
			tp->t_state &= ~ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
#ifdef notdef
		if (tp->t_wsel) {
			selwakeup(tp->t_wsel, tp->t_state & WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~WCOLL;
		}
#endif
	}
}

ptsstop(tp, flush)
	register struct tty *tp;
	int flush;
{
	struct pt_ioctl *pti = &pt_ioctl[minor(tp->t_dev)];

	/* note: FLUSHREAD and FLUSHWRITE already ok */
	if (flush == 0) {
#ifdef notdef
		flush = TIOCPKT_STOP;
#endif
		pti->pt_flags |= PF_STOPPED;
	} else {
		pti->pt_flags &= ~PF_STOPPED;
	}
	pti->pt_send |= flush;
	ptcwakeup(tp);
}

#ifdef SELECT
ptcselect(dev, rw)
	dev_t dev;
	int rw;
{
	register struct tty *tp = &pt_tty[minor(dev)];
	struct pt_ioctl *pti = &pt_ioctl[minor(dev)];
	struct proc *p;
	int s;

	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return (1);
	s = spl5();
	switch (rw) {

	case FREAD:
		if (tp->t_outq.c_cc && (tp->t_state&TTSTOP) == 0) {
			splx(s);
			return (1);
		}
		if ((p = pti->pt_selr) && p->p_wchan == (caddr_t)&selwait)
			pti->pt_flags |= PF_RCOLL;
		else
			pti->pt_selr = u.u_procp;
		break;

	case FWRITE:
		if ((pti->pt_flags & PF_REMOTE) == 0 || tp->t_rawq.c_cc == 0) {
			splx(s);
			return (1);
		}
		if ((p = pti->pt_selw) && p->p_wchan == (caddr_t)&selwait)
			pti->pt_flags |= PF_WCOLL;
		else
			pti->pt_selw = u.u_procp;
		break;
	}
	splx(s);
	return (0);
}
#endif

ptcwrite(dev)
	dev_t dev;
{
	register struct tty *tp;
	register char *cp, *ce;
	register int cc;
	char locbuf[BUFSIZ];
	int cnt = 0;
	struct pt_ioctl *pti = &pt_ioctl[minor(dev)];

	tp = &pt_tty[minor(dev)];
	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return;
	do {
		cc = MIN(u.u_count, BUFSIZ);
		cp = locbuf;
		iomove(cp, (unsigned)cc, B_WRITE);
		if (u.u_error)
			break;
		ce = cp + cc;
again:
		if (pti->pt_flags & PF_REMOTE) {
			if (tp->t_rawq.c_cc) {
#ifdef notdef
				if (pti->pt_flags & PF_NBIO) {
					u.u_count += ce - cp;
					u.u_error = EWOULDBLOCK;
					return;
				}
#endif
				sleep((caddr_t)&tp->t_rawq.c_cf, TTOPRI);
				goto again;
			}
			(void) b_to_q(cp, cc, &tp->t_rawq);
			(void) putc(0, &tp->t_rawq);
			wakeup((caddr_t)&tp->t_rawq);
			return;
		}
		while (cp < ce) {
			while (tp->t_delct && tp->t_rawq.c_cc >= TTYHOG - 2) {
				wakeup((caddr_t)&tp->t_rawq);
#ifdef notdef
				if (tp->t_state & NBIO) {
					u.u_count += ce - cp;
					if (cnt == 0)
						u.u_error = EWOULDBLOCK;
					return;
				}
#endif
				/* Better than just flushing it! */
				/* Wait for something to be read */
				sleep((caddr_t)&tp->t_rawq.c_cf, TTOPRI);
				goto again;
			}
			(*linesw[tp->t_line].l_rint)(*cp++, tp);
			cnt++;
		}
	} while (u.u_count);
}

/*ARGSUSED*/
ptyioctl(dev, cmd, addr, flag)
	caddr_t addr;
	dev_t dev;
{
	register struct tty *tp = &pt_tty[minor(dev)];
	register struct pt_ioctl *pti = &pt_ioctl[minor(dev)];
	unsigned rc;
	struct winsize ws;

	/* IF CONTROLLER STTY THEN MUST FLUSH TO PREVENT A HANG ??? */
	if (cdevsw[major(dev)].d_open == ptcopen) {
		if (cmd == FIONREAD) {
			rc = 0;
			if (tp->t_outq.c_cc && (tp->t_state&TTSTOP) == 0) {
				rc = tp->t_outq.c_cc;
			}
			if (copyout((caddr_t)&rc, addr, sizeof(rc)))
				u.u_error = EFAULT;
			return;
		}

#ifdef TIOCPKT
		if (cmd == TIOCPKT) {
			int packet;
			if (copyin((caddr_t)addr, (caddr_t)&packet, sizeof (packet))) {
				u.u_error = EFAULT;
				return;
			}
			if (packet)
				pti->pt_flags |= PF_PKT;
			else
				pti->pt_flags &= ~PF_PKT;
			return;
		}
#endif
#ifdef TIOCREMOTE
		if (cmd == TIOCREMOTE) {
			int remote;
			if (copyin((caddr_t)addr, (caddr_t)&remote, sizeof (remote))) {
				u.u_error = EFAULT;
				return;
			}
			if (remote)
				pti->pt_flags |= PF_REMOTE;
			else
				pti->pt_flags &= ~PF_REMOTE;
			flushtty(tp, FREAD|FWRITE);
			return;
		}
#endif
#ifdef FIONBIO
		if (cmd == FIONBIO) {
			int nbio;
			if (copyin((caddr_t)addr, (caddr_t)&nbio, sizeof (nbio))) {
				u.u_error = EFAULT;
				return;
			}
			if (nbio)
				pti->pt_flags |= PF_NBIO;
			else
				pti->pt_flags &= ~PF_NBIO;
			return;
		}
#endif
		if (cmd == TIOCSETP)
			while (getc(&tp->t_outq) >= 0);
	}
	if (cmd == JWINSIZE) {
		ws.bytesx = tp->t_cwincols;
		ws.bytesy = tp->t_cwinrows;
		ws.bitsx = tp->t_pwincols;
		ws.bitsy = tp->t_pwinrows;
		if (copyout((caddr_t)&ws, addr, sizeof(struct tchars)))
			u.u_error = EFAULT;
		return;
	}
	if (cmd == JSWINSIZE) {
		if (copyin(addr, (caddr_t)&ws, sizeof(struct tchars)))
			u.u_error = EFAULT;
		tp->t_cwincols = ws.bytesx;
		tp->t_cwinrows = ws.bytesy;
		tp->t_pwincols = ws.bitsx;
		tp->t_pwinrows = ws.bitsy;
		return;
	}
	if (ttioctl(tp, cmd, addr, dev) == 0)
		u.u_error = ENOTTY;
	{ int stop = (tp->t_un.t_chr.t_stopc == ('s'&037) &&
		      tp->t_un.t_chr.t_startc == ('q'&037));
	if (pti->pt_flags & PF_NOSTOP) {
		if (stop) {
#ifdef notdef
			pti->pt_send &= TIOCPKT_NOSTOP;
			pti->pt_send |= TIOCPKT_DOSTOP;
#endif
			pti->pt_flags &= ~PF_NOSTOP;
			ptcwakeup(tp);
		}
	} else {
		if (stop == 0) {
#ifdef notdef
			pti->pt_send &= ~TIOCPKT_DOSTOP;
			pti->pt_send |= TIOCPKT_NOSTOP;
#endif
			pti->pt_flags |= PF_NOSTOP;
			ptcwakeup(tp);
		}
	}
	}
}
#endif
