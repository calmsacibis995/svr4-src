/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:sxt.c	1.3"

/*
 * SXT --  Driver for shell layers
 */

#include "sys/param.h"
#include "sys/types.h"
#ifdef vax
#include "sys/page.h"
#endif
#if vax || u3b15 || u3b2 || i386
#include "sys/sysmacros.h"
#else
#include "sys/macro.h"
#include "sys/istk.h"
#endif
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/proc.h"
#include "sys/fs/s5dir.h"
#include "sys/tty.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/termio.h"
#include "sys/ttold.h"
#include "sys/sxt.h"
#ifdef i386
#include "sys/inline.h"
#endif

 
/*  A real terminal is associated with a number of virtual tty's.
 *  The real terminal remains the communicator to the low level
 *  driver,  while the virtual terminals and their queues are
 *  used by the upper level (ie., ttread, ttwrite).
 *
 *  real tty (tp)   
 *		linesw == SXT
 *		proc   == real driver proc routine
 *  virtual tty (vtp)
 *		linesw == original value of real tty
 *		proc   == SXTVTPROC
 */

/* Maximum size of a link structure */
#define LINKSIZE (sizeof(struct Link) + sizeof(struct Channel)*(MAXPCHAN-1))
#define LINKHEAD (sizeof(struct Link))

Link_p		linkTable[MAXLINKS];

#	if	SXTRACE == 1
char	tracing;		/* print osm msgs for tracing */
#	endif


char	sxtbusy[MAXLINKS], *sxtbuf ;

extern	int	sxt_cnt ;
extern	char	sxt_buf[] ;

Link_p	sxtalloc();

int sxtnullproc();
int sxtvtproc();



sxtopen(dev, flag)
{
	register Link_p lp;
	register chan;
	register struct tty *vtp;
	register bit;
	register int	oldpri;

	chan = CHAN(dev);

#if	SXTRACE == 1
	if (tracing)
		printf("!sxtopen: link %d, chan %d\n", LINK(dev), chan);
#endif

	/*  linkTable:  0 - unused, 1 - master slot reserved, 
	*			   >1 - master operational */
	if ((lp = linkTable[LINK(dev)]) == (Link_p)1) {
		u.u_error = EBUSY;
		return;
	}

	if (lp == NULL) {
		if (chan != 0)
			/* Master on chan 0 must be opened first */
			u.u_error = EINVAL;
		else
			linkTable[LINK(dev)] = (Link_p)1;
		return;		/* nothing else to do here */
	}

	if (chan == 0) {
		/* master already set up on this link	*/
		u.u_error = EBUSY;
		return;
	}

	if (chan >= lp->nchans) {
		/* channel number out of range	*/
		u.u_error = ENXIO;
		return;
	}

	bit = (1 << chan);
	if (lp->open & bit) {
		if ((flag & FEXCL) || (lp->xopen & bit)) {
			u.u_error = ENXIO;
			return;
		}
	}
	else {
		/* open new channel */
		lp->open |= bit;

		if (lp->chans[chan].tty.t_proc == sxtnullproc ||
				lp->chans[chan].tty.t_proc == sxtvtproc)
			ttyflush(&lp->chans[chan].tty, (FREAD | FWRITE));

#ifdef u3b
		kclear((char *) &lp->chans[chan], sizeof (struct Channel));
#else
		bzero((char *) &lp->chans[chan], sizeof (struct Channel));
#endif
		sxtlinit(&lp->chans[chan], chan, LINK(dev), lp->old,
			lp->line);
		if (flag & FEXCL)
			lp->xopen |= bit;
	}

	vtp = &lp->chans[chan].tty;

	/* do the pertinent part of ttopen() here */ 
	oldpri = spltty();
	if ((u.u_procp->p_pid == u.u_procp->p_pgrp)
	   && (u.u_ttyp == NULL)
	   && (vtp->t_pgrp == 0)) {
		u.u_ttyp = &vtp->t_pgrp;
		vtp->t_pgrp = u.u_procp->p_pgrp;
	}
	vtp->t_state |= ISOPEN;
	splx(oldpri);
}


sxtclose(dev, flag)
{
	register Link_p lp;
	register chan;
	register struct tty *vtp;
	int sps;
	int i;

#if SXTRACE == 1
	if (tracing)
		printf("!sxtclose: link %d, chan %d\n", LINK(dev), CHAN(dev));
#endif

	if ((lp = linkTable[LINK(dev)]) == (Link_p)1) {
		/* no work to be done - master slot was only reserved,
		 * not used.					     
		 */
		linkTable[LINK(dev)] = NULL;
		return;
	}

	chan = CHAN(dev);
	vtp = &lp->chans[chan].tty;

#ifndef u3b
	vtp->t_state &= ~(BUSY|TIMEOUT);
#endif

	(*linesw[vtp->t_line].l_close)(vtp);

	vtp->t_pgrp = 1;		/* To keep out /dev/tty */
	vtp->t_state &= ~CARR_ON;

	chan = 1 << chan;
	lp->xopen &= ~chan;
	lp->open &= ~chan;

	sps = spltty();
	if (chan == 1)			/* e.g. channel 0 */
	{
		lp->controllingtty = 0;
		lp->line->t_line = lp->old;
		lp->line = vtp;		/* release real tty */
		vtp->t_proc = sxtnullproc;

		for (i = 1; i < lp->nchans; ++i)
		{
			vtp = &lp->chans[i].tty;

			vtp->t_pgrp = 1;
			if (vtp->t_proc == sxtnullproc ||
					vtp->t_proc == sxtvtproc)
				ttyflush(vtp, (FREAD | FWRITE));
		}
	}

	if (lp->open == 0) 
	{
		/* no other opens on this set of virt ttys */

		linkTable[LINK(dev)] = NULL;
		sxtfree(lp);
	}
	splx(sps);
}


sxtioctl(dev, cmd, arg, mode)
{
	register Link_p lp;
	register struct tty *tp, *vtp;
	register sxtld;
	struct termio cb;
	struct sgttyb tb;
	struct sxtblock sb;
	int flag;
	int n;
	int sps;
	int i;
	char c_line;
	extern sxtin();

#if	SXTRACE == 1
	if (tracing)
		printf("!sxtioctl: cmd %x, link %d, chan %d\n", cmd,
				LINK(dev), CHAN(dev));
#endif

	if ((lp = linkTable[LINK(dev)]) == (Link_p)1 && cmd != SXTIOCLINK) {
		/* only a link command would be valid here */
		u.u_error = ENXIO;
		return;
	}

	switch (cmd) {

	case SXTIOCLINK:
		if (arg > MAXPCHAN || arg < 1) {
			u.u_error = EINVAL;
			break;
		}
		if ((tp = cdevsw[major(u.u_ttyd)].d_ttys) == NULL) {
			u.u_error = ENOTTY;
			break;
		}

		tp = (struct tty *)((char *)u.u_ttyp - ((char *)&tp->t_pgrp - (char *)tp));	/* Real 'tty' line */

		/* find  sxt line discipline entry number in linesw */
		for (sxtld = 0; sxtld < linecnt; sxtld++)
			if (linesw[sxtld].l_input == sxtin)
				break;
		if (sxtld == linecnt) {
			u.u_error = ENXIO;	/* SXT not in linesw */
			break;
		}
		if (lp == (Link_p)0) {
			u.u_error = EBADF; 	/* file not opened */
			break;
		}
		if (lp != (Link_p)1) {
			u.u_error = EBUSY;	/* Pre-empted! */
			break;
		}

		if ((lp = sxtalloc(arg)) == NULL) {
			u.u_error = ENOMEM;	/* No memory, try again */
			break;
		}

		ttyflush(tp, FREAD|FWRITE);
		lp->dev = u.u_ttyd;	/* save major/minor dev #s	*/
		lp->controllingtty = 0;	/* channel 0			*/
		lp->lwchan = 0;	/* last channel to write	*/
		lp->wpending = 0;	/* write pending bits/chan	*/
		lp->wrcnt = 0;		/* number of writes on last channel written */
		lp->line = tp;
		lp->old = tp->t_line;	/* Remember old line discipline */
		lp->nchans = arg;

		lp->chanmask = 0xFF;
		for (i = lp->nchans; i < MAXPCHAN; ++i)
			lp->chanmask >>= 1;

		lp->open = lp->xopen = 1;	/* Open channel 0	*/
		sxtlinit(&lp->chans[0], 0, LINK(dev), lp->old, tp);

		sps = spltty();
		linkTable[LINK(dev)] = lp;	/* Now visible		*/
		tp->t_line = sxtld;	/* Stack new one		*/
		tp->t_link = LINK(dev);	/* Back pointer to link structure */
		vtp = &lp->chans[0].tty;

		/* do the pertinent part of ttopen() here		*/ 
		if ((u.u_procp->p_pid == u.u_procp->p_pgrp)
		   && (u.u_ttyp == NULL)
		   && (vtp->t_pgrp == 0)) {
			u.u_ttyp = &vtp->t_pgrp;
			vtp->t_pgrp = u.u_procp->p_pgrp;
		}
		vtp->t_state |= ISOPEN;
		splx(sps);
		break;

	case SXTIOCSWTCH:
		/* ***  make new vtty top dog -
		 *	download new vtty characteristics and wake up
		 */
		if (lp == (Link_p) 1 || lp == (Link_p) 0) {
			u.u_error = EINVAL;
			break;
		}
		if (!(1<<arg & lp->open)) {
			u.u_error = EINVAL;
			break;
		}
		sps = spltty();
		if (CHAN(dev) != 0) {
			/* only controller can switch windows */
			u.u_error = EPERM;
			splx(sps);
			break;
		}

#if	SXTRACE == 1
		if (tracing)
			printf("!sxtioctl: switch arg=%d, control=%d\n",
				arg,lp->controllingtty);
#endif
		if (lp->controllingtty != arg) {
				
			lp->controllingtty = arg;
			if (arg != 0) {
				/*  download valid portions of tty struct*/
				ttywait(&lp->chans[0].tty) ;
				tp = lp->line;
				vtp = &lp->chans[arg].tty;
				/*  change flags */
				tp->t_iflag = vtp->t_iflag;
				tp->t_oflag = vtp->t_oflag;
				tp->t_cflag = vtp->t_cflag;
				tp->t_lflag = vtp->t_lflag;
				bcopy(vtp->t_cc, tp->t_cc, NCC);
	
				/*  do download to have new values take effect */
				(*tp->t_proc)(tp, T_PARM);
			}
		}
		splx(sps);
		wakeup((caddr_t) &lp->chans[arg]);
		break;

	case SXTIOCWF:
		/* wait til chan arg is in foreground and
		 * then return
		 */
		if (lp == (Link_p) 1 || lp == (Link_p) 0) {
			u.u_error = EINVAL;
			break;
		}
		if (!(1<<arg & lp->open)) {
			u.u_error = EINVAL;
			break;
		}
		if (lp->controllingtty == arg)
			/* nothing to be done */
			break;

		sps = spltty();
		while (lp->controllingtty != arg)
			sleep((caddr_t) &lp->chans[arg], TTOPRI);
		splx(sps);

		break;

	case SXTIOCBLK:
		/*
		 *  set LOBLK in indicated window
		 */
		
		if (lp == (Link_p) 1 || lp == (Link_p) 0) {
			u.u_error = EINVAL;
			break;
		}
		if (!(1<<arg & lp->open)) {
			u.u_error = EINVAL;
			break;
		}
		vtp = &lp->chans[arg].tty;
		vtp->t_cflag |= LOBLK;
		break;

	case SXTIOCUBLK:
		/*
		 *  unset LOBLK in indicated window
		 */
		
		if (lp == (Link_p) 1 || lp == (Link_p) 0) {
			u.u_error = EINVAL;
			break;
		}
		if (!(1<<arg & lp->open) || (arg == 0)) {
			u.u_error = EINVAL;
			break;
		}
		vtp = &lp->chans[arg].tty;
		vtp->t_cflag &= ~LOBLK;
		wakeup((caddr_t) &lp->chans[arg]);
		break;

	case SXTIOCSTAT:
		/*
		 *  return bit map of blocked channels to user
		 */

		if (lp == (Link_p) 1 || lp == (Link_p) 0) {
			u.u_error = EINVAL;
			break;
		}
		
		sb.input = lp->iblocked;
		sb.output = lp->oblocked;
		if (copyout(&sb, arg, sizeof sb)) {
			u.u_error = EFAULT;
			break;
		}
		break;

#if SXTRACE == 1
	case SXTIOCTRACE:
		tracing = 1;
		break;

	case SXTIOCNOTRACE:
		tracing = 0;
		break;
#endif

	case TIOCEXCL:
		lp->xopen |= (1<<CHAN(dev));
		break;

	case TIOCNXCL:
		lp->xopen &= ~(1<<CHAN(dev));
		break;

	case TCGETA:
	case TIOCGETP:
		ttiocom(&lp->chans[CHAN(dev)].tty, cmd, arg, mode);
		break;

	case TIOCSETP:
		ttiocom(&lp->chans[CHAN(dev)].tty, cmd, arg, mode);
		if (CHAN(dev) == lp->controllingtty) {
			/* TIOCSETP real tty without flush or ttywait */
			tp = lp->line;
			/* next section lifted from tty.c */

			if (copyin(arg, &tb, sizeof(tb))) {
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
	
	
			/* download tty change */
			(*tp->t_proc)(tp, T_PARM);
		}
		break;

	case TCSETA:
	case TCSETAW:
	case TCSETAF: {
		int tdev;

		ttiocom(&lp->chans[CHAN(dev)].tty, cmd, arg, mode);
		if (CHAN(dev) == lp->controllingtty) {
			/* must perform action on real tty */
			/* insure that line disps remain correct */
			if (copyin(arg, &cb, sizeof cb)) {
				u.u_error = EFAULT;
				break;
			}
			c_line = cb.c_line;
			cb.c_line = lp->line->t_line;
			if (copyout(&cb, arg, sizeof cb)) {
				u.u_error = EFAULT;
				break;
			}
			tdev = expdev(lp->dev);
			(*cdevsw[major(lp->dev)].d_ioctl)
				(tdev, TCSETA, arg, mode, u.u_cred,&u.u_rval1);
			/*  now restore user buffer */
			cb.c_line = c_line;
			if (copyout(&cb, arg, sizeof cb)) {
				u.u_error = EFAULT;
				break;
			}
		}
		break;
	}
	case TCSBRK:
		ttiocom(&lp->chans[CHAN(dev)].tty, cmd, arg, mode);
		if (CHAN(dev) == lp->controllingtty && arg == 0)
			(*lp->line->t_proc)(lp->line, T_BREAK);
		break;

	case TCXONC:
	case TCFLSH:
		ttiocom(&lp->chans[CHAN(dev)].tty, cmd, arg, mode);
		break;


	default: {
		int tdev;

		ttiocom(&lp->chans[CHAN(dev)].tty, cmd, arg, mode);
		if (CHAN(dev) == lp->controllingtty)
			/* must perform action on real tty */
			tdev = expdev(lp->dev);
			(*cdevsw[major(lp->dev)].d_ioctl)
				(tdev, cmd, arg, mode, u.u_cred,&u.u_rval1);
		}
		break;
	}
}



sxtlinit(chp, channo, link, ld, tp)
register Ch_p chp;
register struct tty * tp;	/* real tty structure */
{

#if	SXTRACE == 1
		if (tracing)
			printf("!sxtlinit: channo %d\n", channo);
#endif

	ttinit(&chp->tty);
	chp->tty.t_line = ld;	/* old line discipline */
	chp->tty.t_proc = sxtvtproc;
	chp->tty.t_link = link;
	chp->tty.t_state |=  (tp->t_state
				& ~(OASLP|IASLP|TTIOW|BUSY|ISOPEN));

}


sxtread(dev)
{
	register Link_p lp;
	register struct tty *vtp;
	register int channo;
	int sps;


#if	SXTRACE == 1
	if (tracing)
		printf("!sxtread: link %d, chan %d\n", LINK(dev),
				CHAN(dev));
#endif

	channo = CHAN(dev);
	if ((lp = linkTable[LINK(dev)]) == (Link_p)1)
		u.u_error = ENXIO;
	else if (lp == (Link_p)0)
		u.u_error = EBADF;	/* link not opened */
	else if (!(lp->open & (1<<channo)))
		u.u_error = EBADF;
	else {
		vtp = &lp->chans[channo].tty;

		sps = spltty();
		while (lp->controllingtty != channo) {
			lp->iblocked |= (1 << channo);
			sleep((caddr_t) &lp->chans[channo], TTOPRI);
		}
		lp->iblocked &= ~(1 << channo);
		splx(sps);
		(*linesw[vtp->t_line].l_read)(vtp);	/* virt. tty */
	}
}


sxtwrite(dev)
{
	register Link_p	lp;
	register struct tty *vtp;
	register int channo;
	int sps;
	
#if	SXTRACE == 1
	if (tracing)
		printf("!sxtwrite: link %d, chan %d\n", LINK(dev),
				CHAN(dev));
#endif

	channo = CHAN(dev);
	if ((lp = linkTable[LINK(dev)]) == (Link_p)1)
		u.u_error = ENXIO;
	else if (lp == (Link_p)0)
		u.u_error = EBADF;	/* link not opened */
	else if (!(lp->open & (1<<channo)))
		u.u_error = EBADF;
	else {
		channo = CHAN(dev);
		vtp = &lp->chans[channo].tty;

		sps = spltty();
		while ((vtp->t_cflag & LOBLK) &&
		    (lp->controllingtty != channo)) {
			lp->oblocked |= (1 << channo);
			sleep((caddr_t) &lp->chans[channo], TTOPRI);
		}
		lp->oblocked &= ~(1 << channo);
		splx(sps);

		(*linesw[vtp->t_line].l_write)(vtp);	/* virt. tty */
	}
}


sxtrwrite(rtp)
register struct tty *rtp;
{
	register struct tty *vtp;
	register index;
	register Link_p	lp;

	/* write issued to a real tty device.  Look for the real tty's group
 	* of virtual ttys and do the write to the controllingtty of that 
 	* group.  Hash this???
 	*/
 	for (index = 0; index < MAXLINKS; index++) {
		if ((lp=linkTable[index]) != (Link_p)0 && lp != (Link_p)1) {
			if (lp->line == rtp)
				break;
		}
	}
	if (index == MAXLINKS)	/* no match */
		return;		/* drop output on floor */
	/* write to controlling tty 	    */
	vtp = &lp->chans[lp->controllingtty].tty;
	(*linesw[vtp->t_line].l_write)(vtp);
}




sxtvtproc(vtp, cmd)
register struct tty *vtp;
{
	register Link_p	lp;
	register cnt;

	/* 
	 *     called with a virtual tty.
	 */

#if	SXTRACE == 1
	if (tracing)
		printf("!sxtvtproc: cmd %d \n", cmd);
#endif

	switch (cmd) {

	default:
		return;

	case T_WFLUSH:
		lp = linkTable[vtp->t_link];
		if (lp->controllingtty == (vtp - &lp->chans[0].tty))
			(*lp->line->t_proc)(lp->line, T_WFLUSH);
		break;

	case T_RESUME:
		lp = linkTable[vtp->t_link];
		(*lp->line->t_proc)(lp->line, T_RESUME);
		break;

	case T_OUTPUT:
		lp = linkTable[vtp->t_link];
		cnt = vtp - &lp->chans[0].tty;
		lp->wpending |= 1<< cnt;
		(*lp->line->t_proc)(lp->line, T_OUTPUT);  /* real proc */
		break;

	case T_SUSPEND:
		lp = linkTable[vtp->t_link];
		(*lp->line->t_proc)(lp->line, T_SUSPEND);
		break;

	case T_RFLUSH:
		lp = linkTable[vtp->t_link];
		if (lp->controllingtty == (vtp - &lp->chans[0].tty))
			(*lp->line->t_proc)(lp->line, T_RFLUSH);
		break;

#if vax || u3b15 || u3b2 || i386
	case T_SWTCH:
		lp = linkTable[vtp->t_link];
		/* change control to channel 0 */
		lp->controllingtty = 0;
		wakeup ((caddr_t) &lp->chans[0]);
		break;
#endif
	}
}


sxtnullproc(vtp, cmd)
register struct tty *vtp;
{
	register Link_p lp;
	unsigned char	tmp;
	int cnt = 0;
	/* 
	 *     called with a virtual tty.
	 */

#if	SXTRACE == 1
	if (tracing)
		printf("!sxtnullproc: cmd %d \n", cmd);
#endif

	if (cmd == T_OUTPUT)
	{
		lp = linkTable[vtp->t_link];

		tmp = lp->wpending;
		while (tmp >>= 1)
			cnt++;

		ttyflush(&lp->chans[cnt].tty, FWRITE);
		lp->wpending = 0;
	}
}


/*
 * real tty output routine - give cblock to device
 * multiplexing done here!
 */
sxtout(tp)
struct tty *tp;
{
	register Link_p lp;
	register mask, cnt;
	register struct tty *vtp;
	unsigned char tmp;
	int sps;
	int retn;
	
	lp = linkTable[tp->t_link];

/*
 * Check for initialization
 */
	if ( lp == NULL )
		return 0;

#if	SXTRACE == 1
	if (tracing)
		printf("!sxtout:  link %d, chan %d\n", tp->t_link,
				lp->lwchan);
#endif

	sps = spltty();
	if (lp->lwchan)
	{
		cnt = 0;
		tmp = lp->lwchan;
		while (tmp >>= 1)
			cnt++;

		vtp = &lp->chans[cnt].tty;
		vtp->t_tbuf = tp->t_tbuf;

		if (lp->wrcnt >= SXTHOG && lp->wpending != lp->lwchan)
		{
			/* This vtty has been an SXT HOG!!, attempt to
			   give someone else a chance! */
			if (vtp->t_tbuf.c_ptr != NULL)
			{
				putcf(CMATCH((struct cblock *)vtp->t_tbuf.c_ptr));
				vtp->t_tbuf.c_ptr = NULL;
				vtp->t_tbuf.c_count = 0;
			}
			tp->t_tbuf = vtp->t_tbuf;
		}
		else 
		{
			/* This vtty not an SXT HOG so attempt to
			   do some more output on last written channel */
			if (retn = (*linesw[vtp->t_line].l_output)(vtp))
			{	
				/* got another tbuf from that virt. tty */
				tp->t_tbuf = vtp->t_tbuf;
				vtp->t_tbuf.c_ptr = NULL;
				vtp->t_tbuf.c_count = 0;

				lp->wrcnt = (lp->wrcnt >= SXTHOG) ? 1: (lp->wrcnt + 1);

				/* return 1 so real proc will transmit */
				splx(sps);
				return(retn);
			}
			else 
			{
				/* no more data on this virt terminal */
				if (vtp->t_tbuf.c_ptr != NULL)
				{
					putcf(CMATCH((struct cblock *)vtp->t_tbuf.c_ptr));
					vtp->t_tbuf.c_ptr = NULL;
					vtp->t_tbuf.c_count = 0;
				}
				tp->t_tbuf = vtp->t_tbuf;

				lp->wpending &= ~(lp->lwchan);
			}
		}
	}

	/* try to schedule next channel in round-robin list of wpending 
	   and dont give up until ALL possibilities are exhausted */
	while (lp->wpending != 0) 
	{

		for (cnt = 0; cnt < lp->nchans; ++cnt)
		{
			/* find next channel which had write pending */
			/* when done, lwchan specifies a candidate for reschedule */
			lp->lwchan = (lp->lwchan << 1) & lp->chanmask;
			if (lp->lwchan == 0)
				lp->lwchan = 1;
			if (lp->wpending & lp->lwchan)
				break;
		}

		if (cnt < lp->nchans) 		/* channel out of bounds? */
		{
			/* xlate bit to channel # */
			cnt = 0;
			tmp = lp->lwchan;
			while (tmp >>= 1)
				cnt++;

			vtp = &lp->chans[cnt].tty;

			if (retn = (*linesw[vtp->t_line].l_output)(vtp))
			{
#if		SXTRACE == 1
				if (tracing)
					printf("!sxtout: got tbuf from new vtty %d\n",
							cnt);
#endif
				/* got a tbuf from a different virt. tty */
				lp->wrcnt = 1;
				tp->t_tbuf = vtp->t_tbuf;
				vtp->t_tbuf.c_ptr = NULL;
				vtp->t_tbuf.c_count = 0;
				/* so real proc will transmit */
				splx(sps);
				return(retn);
			}
			else 
			{
#if		SXTRACE == 1
				if (tracing)
					printf("!sxtout: %d lied\n", cnt);
#endif
				/* false alarm!,  this is not a good candidate
				   since there is no more data on this
				   virt terminal */
				/* someone lied to us!!		*/
				if (vtp->t_tbuf.c_ptr != NULL) {
					putcf(CMATCH((struct cblock *) vtp->t_tbuf.c_ptr));
					vtp->t_tbuf.c_ptr = NULL;
					vtp->t_tbuf.c_count = 0;
				}
				tp->t_tbuf = vtp->t_tbuf;

				lp->wpending &= ~(lp->lwchan);
			}
		}
	}
	splx(sps);
	return(0);	/* we'll get called again....*/
}




/*
 * real tty input routine
 * returns data to controlling tty
 */
sxtin(tp, code)
register struct tty *tp;
{
	register struct tty *vtty;
	register Link_p	lp;
	register n;

#if	SXTRACE == 1
	if (tracing)
		printf("!sxtin:  link %d, code %d\n", tp->t_link, code);
#endif

	lp = linkTable[tp->t_link];
	n = lp->controllingtty;
	vtty = &lp->chans[n].tty;

	switch (code) {

#ifdef u3b
	case L_SWITCH:

		/* change control to channel 0 */
		/* first flush input queue     */
		if (!vtty->t_lflag & NOFLSH)
			ttyflush(vtty, FREAD);
		if (n != 0)
			lp->controllingtty = 0;
		wakeup ((caddr_t) &lp->chans[0]);
		break;

	case L_INTR:
	case L_QUIT:
#endif
	case L_BREAK:

		(*linesw[vtty->t_line].l_input)(vtty, code);
		break;
		
	case L_BUF:

		/* copy data to controlling tty */
		vtty->t_rbuf = tp->t_rbuf;

		(*linesw[vtty->t_line].l_input)(vtty, L_BUF);


		/* ttin() will have moved the rbuf to the inputq on the
		 * virtual tty and replaced rbuf.c_ptr with a new cblock
		 * just transfer this to the real tty rbuf field and we
		 * should be OK.
		 */
		tp->t_rbuf = vtty->t_rbuf;
		vtty->t_rbuf.c_ptr = NULL;
		vtty->t_rbuf.c_count = 0;

	default:
		return;

	}
}


sxtfree(lp)
Link_p lp;
{
	int i;

#if	SXTRACE == 1
	if (tracing)
		printf("!sxtfree\n");
#endif


	i = ((char *)lp - sxtbuf)/LINKSIZE;
	sxtbusy[i] = 0;
}


Link_p
sxtalloc(arg)
{
	register i;
	Link_p lp;

#if	SXTRACE == 1
	if (tracing)
		printf("!sxtalloc\n");
#endif

#if vax || u3b2 || i386
	if (sxtbuf == NULL)
		sxtinit();
#endif

	if (sxtbuf != NULL) {
		for (i=0; i < sxt_cnt; i++)
			if (!sxtbusy[i]) {
				lp = (Link_p) (sxtbuf + (i*LINKSIZE));
#ifdef u3b
				kclear((char *)lp, LINKHEAD);
#else
				bzero((char *)lp, LINKHEAD);
#endif
				sxtbusy[i] = 1;
				return(lp);
			}
	}
	return(NULL);
}



/***************************** 3b15 ******************************/
#if u3b15 || i386

sxtinit()
{
	extern char sxt_buf[];

	if ((sxtbuf = sxt_buf) == NULL)
		printf("sxt cannot allocate link buffers\n");
}
#endif
/**************************** 3b20 & 3b2 *******************************/
#if u3b || u3b2

sxtinit()
{

	if ((sxtbuf = (char *)kseg(btoc(sxt_cnt*LINKSIZE))) == NULL)
		printf("sxt cannot allocate link buffers\n");
}

#endif

/**************************** VAX only *******************************/
#ifdef vax

sxtinit()
{

	if ((sxtbuf = (char *)sptalloc(btoc(sxt_cnt*LINKSIZE),
				PG_V|PG_KW, 0)) == NULL)
		printf("sxt cannot allocate link buffers\n");
}
#endif
