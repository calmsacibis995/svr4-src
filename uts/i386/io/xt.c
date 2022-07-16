/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:xt.c	1.3.1.1"

/*
**      Blit Packet Protocol Driver (i[23]86)
**
**      Multiplexes virtual 'tty' lines onto real 'tty' lines.
*/

#define XTDRIVER        1	/* Conditional compilation control */
#define PIGGY_BACK	0	/* Piggy back single char echo... */
#define FAST_BUT_BIG    0	/* Faster CRC algorithm uses 512 byte look-up table */
#define BLITIXON        0	/* If CNTL-S/CNTL-Q managed in BLIT */
#ifndef DEBUG
#define DEBUG           0	/* 1 to include debug code for dubious Blit opsys. */
#endif
#if DEBUG
#define DEBUGF(a,b) printf(a,b)
#else
#define DEBUGF(a,b)
#endif

#define NUMBYTES       (sizeof(struct Link)+sizeof(struct Channel)*(MAXPCHAN-1))

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/ino.h"
#include "sys/buf.h"
#include "sys/conf.h"
#ifndef i286
#include "sys/immu.h"
#endif /* not i286 */
#include "sys/proc.h"
#include "sys/fs/s5dir.h"
#include "sys/tty.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/termio.h"
#ifdef VPIX
#include "sys/tss.h"
#include "sys/v86.h"
#include "sys/stream.h"
#include "sys/asy.h"
#endif
#include "sys/ioctl.h"  /* for union ioctl_arg */
#include "sys/jioctl.h"
#include "sys/xtproto.h"
#include "sys/xt.h"
#ifdef i386
#include "sys/inline.h"
#endif

/** Pointers to dynamically allocated 'link' memory **/

static Link_p   linkTable[MAXLINKS];

/** Bytes per timeout scan (1/2 sec) for each speed (minimum 1) **/

static Pbyte speeds[16] =
{
	1,      2,      3,      5,      6,      7,      10,     15,
	30,     60,     90,     120,    240,    480,    960,    1
};


/** Table of ordered sequence numbers **/

static Pbyte    seqtable[2*SEQMOD];

static char     scanning;

static void     start();
static void     makepkt();
static void     control();
static void     xtlinit();
static int      recvpkt();
static int      retry();
static int      crc();
static int      mybcopy();

#ifdef vax
/*
**      N.B.    a 'movc3' of less than 36 bytes is faster than a 'calls' instruction
**              the 'crc' instruction takes 3.3 usecs/byte on a 780
*/
#endif

#define LINKSIZE (sizeof(struct Link) + sizeof(struct Channel)*(MAXPCHAN-1))
char    xtbusy[MAXLINKS], *xtbuf;
extern  int     xt_cnt;         /*      Defined in  space.c  */
Link_p  xtalloc();

static int xterrstart;     /* bad packets caught in start() */
static int xterrxtin;      /* bad packets caught in xtin() */
static int xterrclose;     /* bad packets caught in xtclose() */


xtopen(dev, flag)
int     dev, flag;
{
	register Link_p lp;
	register int    chan;
	register struct tty *tp;
	register int    bit;
	register int    oldpri;

	dev = minor(dev);
	chan = CHAN(dev);

	if ((lp = linkTable[LINK(dev)]) == (Link_p)1) {
		u.u_error = EBUSY;
		return; /* Already booked */
	}

	if (lp == (Link_p)0) {
		if (chan)
			u.u_error = EINVAL;
		else
			linkTable[LINK(dev)] = (Link_p)1;
		return; /* Wait for XTIOCLINK */
	}

	if (chan == 0) {
		/* allow multiple opens on channel 0
		 * xtread and xtwrite will catch reads and writes
		 * (only ioctl's allowed, not reads or writes
		 */
		if (flag & FEXCL)
			u.u_error = EBUSY;
		return;
	}

	if (chan >= lp->l.nchans) {
		u.u_error = ENXIO;
		return;
	}

	bit = (1 << chan);

	if (lp->l.open & bit) {
		if ((flag & FEXCL) || (lp->l.xopen & bit)) {
			u.u_error = ENXIO;
			return;
		}
	} else {
		lp->l.open |= bit;
		xtlinit(&lp->chans[chan], chan, (unsigned int)LINK(dev), lp->l.old);
		if (flag & FEXCL)
			lp->l.xopen |= bit;
	}

	tp = &lp->chans[chan].tty;
	oldpri = spltty();
	(*linesw[tp->t_line].l_open)(tp);
	splx(oldpri);
}



/*ARGSUSED*/
xtclose(dev, flag)
int     dev, flag;
{
	register Link_p      lp;
	register int         chan;
	register struct tty *tp;
	register int         oldpri;

	dev = minor(dev);
#ifdef OLD
	if ((lp = linkTable[LINK(dev)]) == (Link_p)1) {
		linkTable[LINK(dev)] = (Link_p)0;
		return;
	}

	/*
	 * check for possible bad pointer to avoid PANIC
	 */
	if(lp == (Link_p)0) {
		u.u_error = EBADF;
		return;
	}
#else
	if ((unsigned) (lp = linkTable[LINK(dev)]) <= (unsigned) 1) {
		if (lp == (Link_p)1) {
			linkTable[LINK(dev)] = (Link_p)0; /* normal */
		} else {
		    if (xterrclose++ % 25 == 1) { 
			printf("xt driver warning: bad lp (%d), ",lp);
			printf("xterrclose = %d\n",xterrclose);
			printf("Check for noisy terminal lines.\n");
		    }
		    u.u_error = EBADF;
		}
		return;
	}
#endif

	chan = CHAN(dev);
	tp = &lp->chans[chan].tty;
	oldpri = spltty();
	ttyflush(tp, (FREAD | FWRITE));
	tp->t_state &= ~BUSY;
	splx(oldpri);
	(*linesw[tp->t_line].l_close)(tp);

#ifdef VPIX
	lp->chans[chan].v86p = (v86_t *)0;
#endif

	chan = 1 << chan;
	lp->l.xopen &= ~chan;

	if ((lp->l.open &= ~chan) == 0) {
		lp->l.line->t_line = lp->l.old; /* Unstack line discipline */
		if (lp->l.agent.retcb)
			/* there is a cblock to give back */
			putcf(lp->l.agent.retcb);
		linkTable[LINK(dev)] = (Link_p)0;
		xtfree(lp);
	}
}



xtioctl(dev, cmd, arg, mode)
int     dev, cmd;
union ioctl_arg arg;
int     mode;
{
	register Link_p lp;
	register int    nchans;
	register struct file *  fp;
	register struct tty *   tp;
	register unsigned short  tdev;
	register short  n;
	register int    oldpri;
	extern int      xtin();
	struct termio   cb;
/*
	static char     linkmem[NUMBYTES];
 * Above commented out 
 */

	dev = minor(dev);
	if ((lp = linkTable[LINK(dev)]) == (Link_p)1 && cmd != XTIOCLINK && cmd != HXTIOCLINK) {
		u.u_error = ENXIO;
		return;
	}

	/*
	 * check for possible bad pointer to avoid PANIC
	 */
	if(lp == (Link_p)0) {
		u.u_error = EBADF;
		return;
	}

	switch ((short) cmd) {
	case JTYPE:
		cmd = JMPX;
	case XTIOCTYPE:
	case JMPX:
		u.u_rval1 = cmd;
		break;

	case JTRUN:
		{
		char jtbuf[TTYHOG+1];
		register char *pc;
		register char *cp = jtbuf+1;
		register short c, l, nc = 1;

		do {
			if (nc > TTYHOG)
				u.u_error = EINVAL;
			if ((c = fubyte(arg.cparg++)) < 0)
				u.u_error = EFAULT;
			if (u.u_error) 
				return;
			nc++;
			*cp++ = c;
		} while (c > 0);

		jtbuf[0] = C_RUN;
		cp = jtbuf;
		tp = &lp->chans[0].tty;

		do {
			if (pc = tp->t_rbuf.c_ptr) {
				l = nc < CLSIZE ? nc : CLSIZE;
				for(c = 0; c < l; c++)
					*pc++ = *cp++;
				nc -= l;
				tp->t_rbuf.c_count -= l;
				(*linesw[tp->t_line].l_input)(tp);
			} else {
				STATS(lp,S_NORBUF);
				break;
			}
		} while (nc > 0);

		}
		break;
	case XTIOCLINK:
	case HXTIOCLINK:
		if ((nchans = ((struct xtioclm *) & arg.sparg)->nchans) > MAXPCHAN
		    || nchans < 1) {
			u.u_error = EINVAL;
			break;
		}
		if ((tp=cdevsw[major(u.u_ttyd)].d_ttys) == (struct tty *)0) {
			u.u_error=ENOTTY;
			break;
		}
		/* Real 'tty' line ** MASK is mandatory *
		 *   because of KMC assist code         */

		/* Use pointer arithmetic to find tty entry--can't use *
		 * indexing because minor numbers may not be sequential*/
		tp = (struct tty *)((char *)u.u_ttyp - ((char *)&tp->t_pgrp - (char *)tp));
		for (tdev = 0 ; (int)tdev < linecnt ; tdev++)
			if (linesw[tdev].l_input == xtin)
				break;
		if (tdev == linecnt) {
			/* 'linesw' not configured with 'xt' */
			u.u_error = ENXIO;
			break;
		}
		if (lp != (Link_p)1) {
			u.u_error = EBUSY;    /* Pre-empted! */
			break;
		}
		if ((lp = xtalloc()) == NULL) {
			u.u_error = ENOMEM;   /* No memory, try again */
			break;
		}
		/*              bzero((char *)lp, n); */
		lp->l.line = tp;
		lp->l.old = tp->t_line;       /* Remember old line discipline */
		lp->l.nchans = nchans;
		lp->l.lihiwat = (short)tthiwat[tp->t_cflag&CBAUD];
		for (n = 0 ; n < 2 * SEQMOD ; n++)
			seqtable[n] = n & SEQMASK;
		n = speeds[tp->t_cflag&CBAUD];
		/* Magic number is turn-around delay in blit */
		lp->l.xtimo = (nchans * sizeof(struct Packet) * NPCBUFS + n - 1) / n + 3;
		lp->l.rtimo = (sizeof(struct Packet) + n - 1) / n + 1;
		lp->l.open = lp->l.xopen = 1;   /* Open the channel */
		lp->l.pid = u.u_procp->p_pid;   /* controling process */
		lp->l.hex = (cmd == HXTIOCLINK);
		xtlinit(&lp->chans[0], 0, (unsigned int)LINK(dev), lp->l.old);
		oldpri = spltty();
		linkTable[LINK(dev)] = lp;      /* Now visible to xtscan */
		tp->t_line = tdev;              /* Stack new line discipline */
		tp->t_link = LINK(dev);         /* Back pointer */
		tp = &lp->chans[0].tty;
		tp->t_cc[VMIN] = 2;		/* jerq message size */
		(*linesw[tp->t_line].l_open)(tp);
		if (!scanning)
			xtscan();
		splx(oldpri);
	case JTIMOM:
		if (CHAN(dev)) {
			u.u_error = EINVAL;
			break;
		}
		tp = &lp->chans[0].tty;
		n = (lp->l.rtimo * 1000) / HZ;
		tdev = (lp->l.xtimo * 1000) / HZ;
		ttywait(tp);
		oldpri = spltty();
		if (cfreelist.c_next) {
			(void)putc(JTIMOM, &tp->t_outq);
			(void)putc(n, &tp->t_outq);
			(void)putc(n >> 8, &tp->t_outq);
			(void)putc(tdev, &tp->t_outq);
			(void)putc(tdev >> 8, &tp->t_outq);
			(void)xtvtproc(tp, T_OUTPUT);
		} else
			u.u_error = EIO;
		splx(oldpri);
		ttywait(tp);
		break;

#if XTRACE == 1
	case XTIOCTRACE:
		if (copyout((caddr_t) &lp->l.trace, arg.cparg, sizeof(struct Tbuf))) {
			u.u_error = EFAULT;
			break;
		}
		lp->l.trace.flags = TRACEON;
		lp->l.trace.used = 0;
		break;

	case XTIOCNOTRACE:
		lp->l.trace.flags = 0;
		break;
#endif

#if XTSTATS == 1
	case XTIOCSTATS:
		u.u_rtime = lbolt;
		if (copyout((caddr_t)lp->l.stats, arg.cparg, sizeof(lp->l.stats)))
			u.u_error = EFAULT;
		break;
#endif

#if XTDATA == 1
	case XTIOCDATA:
		if (copyout((caddr_t)lp, arg.cparg, (sizeof(struct Link) + sizeof(struct Channel) * (lp->l.nchans - 1))))
			u.u_error = EFAULT;
		break;
#endif
	case JWINSIZE:
		if (copyout((caddr_t) & lp->chans[CHAN(dev)].winsize, arg.cparg, sizeof(struct jwinsize)))
			u.u_error = EFAULT;
		break;

	case JTERM:
	case JBOOT:
	case JZOMBOOT:
		tp = &lp->chans[0].tty;
		ttywait(tp);
		oldpri = spltty();
		if (cfreelist.c_next) {
			(void)putc(cmd, &tp->t_outq);
			(void)putc(CHAN(dev), &tp->t_outq);
			(void)xtvtproc(tp, T_OUTPUT);
		} else
			u.u_error = EIO;
		splx(oldpri);
		ttywait(tp);
		break;

	case TIOCEXCL:
		lp->l.xopen |= (1 << CHAN(dev));
		break;

	case TIOCNXCL:
		lp->l.xopen &= ~(1 << CHAN(dev));
		break;

#ifdef VPIX
	case AIOCDOSMODE:
		lp->chans[CHAN(dev)].v86p = u.u_procp->p_v86;
		break;
	case AIOCNONDOSMODE:
		lp->chans[CHAN(dev)].v86p = (v86_t *)0;
		break;
	case AIOCINTTYPE:
		if (arg.iarg != V86VI_KBD)
			u.u_error = EINVAL;
		break;
#endif /* VPIX */

	case JAGENT:
		 {
			struct cblock *bp;
			char    *cp;
			int     sps;

			if (CHAN (dev)) {
				u.u_error = EINVAL;
				break;
			}

			sps = spltty();
			while (lp->l.agent.flag & AGBUSY) {
				/* wait for pending ioctl to complete */
				lp->l.agent.flag |= AGASLP;
				sleep((caddr_t)&lp->l.agent.flag, TTOPRI);
			}
			lp->l.agent.flag |= AGBUSY;
			splx(sps);

			/* copy in the descriptor and check a few things */
			if (copyin(arg.cparg, (caddr_t) & lp->l.agent.desc, sizeof(struct bagent))) {
				u.u_error = EFAULT;
				goto agentout;
			}

			if (((n = lp->l.agent.desc.size) < 1) || (n > MAXPKTDSIZE)) {
				u.u_error = EINVAL;
				goto agentout;
			}

			tp = &lp->chans[0].tty;

			if ((bp = getcf()) == NULL) {
				u.u_error = EIO;
				goto agentout;
			}
			bp->c_data[0] = (Pbyte)JAGENT;
			bp->c_data[1] = (Pbyte)n;
			if (copyin((caddr_t)lp->l.agent.desc.src, (caddr_t)&bp->c_data[2], n)) {
				u.u_error = EFAULT;
				putcf(bp);
				goto agentout;
			}
			bp->c_last = n + 2;
			sps = spltty();
			putcb (bp, &tp->t_outq); /* runs at spltty */
			(void)xtvtproc (tp, T_OUTPUT);

			if (sleep((caddr_t)&lp->l.agent.desc.dest, (TTOPRI | PCATCH))) {
				/* we were kicked out */
				u.u_error = EINTR;
				splx(sps);
			} else {
				/*
				 * output done, size of RETURNING blit data
				 * will finally be stored in arg->size
				 */
				bp = lp->l.agent.retcb;
				lp->l.agent.retcb = (struct cblock *)NULL;
				splx(sps);
				if (bp == (struct cblock *) NULL) {
					goto agentout;
				}
				cp = (char *) bp->c_data;
				n = (short) *cp++;
				if ((n < 1) || (n > MAXPKTDSIZE))
					n = MAXPKTDSIZE;
				lp->l.agent.desc.size = n;
				u.u_rval1 = n;
				if (copyout((caddr_t)cp, (caddr_t)lp->l.agent.desc.dest, n)) {
					u.u_error = EFAULT;
				}
				putcf(bp);
			}

agentout:
			lp->l.agent.flag &= ~AGBUSY;
			if (lp->l.agent.flag & AGASLP) {
				lp->l.agent.flag &= ~AGASLP;
				wakeup((caddr_t)&lp->l.agent.flag);
			}
			break;
		}

	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		if (copyin((caddr_t)arg.stparg, (caddr_t)&cb, sizeof cb)) {
			u.u_error = EFAULT;
			break;
		}
		if (cb.c_line != 0) {
			cb.c_line = 0;
			if (copyout((caddr_t)&cb, (caddr_t)arg.stparg, sizeof cb)) {
				u.u_error = EFAULT;
				break;
			}
		}
		/* fall thru */

	default:
		ttiocom(&lp->chans[CHAN(dev)].tty, cmd, arg, mode);
		break;
	}
}



static void
xtlinit(chp, channo, link, ld)
register Ch_p   chp;
int     channo;
unsigned int     link;
char     ld;
{
	extern int      xtvtproc();

	ttinit(&chp->tty);
	chp->tty.t_state |= CARR_ON;
	chp->tty.t_line = ld;
	chp->tty.t_proc = xtvtproc;
	chp->chan.link = link;
	chp->chan.channo = channo;
	if (chp->chan.nextpkt == (Pks_p)0)
		chp->chan.nextpkt = chp->chan.pkts;
	chp->chan.outen = NPCBUFS;
}



xtread(dev)
int     dev;
{
	register Link_p         lp;
	register struct tty *   tp;

	dev = minor(dev);
	if ((lp = linkTable[LINK(dev)]) == (Link_p)1)
		u.u_error = ENXIO;
	else {
		/*
		 * check for possible bad pointer to avoid PANIC
		 */
		if(lp == (Link_p)0) {
			u.u_error = EBADF;
			return;
		}
		if ((CHAN(dev) == 0) && (lp->l.pid != u.u_procp->p_pid)) {
			u.u_error = ENXIO;
			return;
		}
		tp = &lp->chans[CHAN(dev)].tty;

		(*linesw[tp->t_line].l_read)(tp);
	}
}



xtwrite(dev)
int     dev;
{
	register Link_p         lp;
	register struct tty *   tp;

	dev = minor(dev);
	if ((lp = linkTable[LINK(dev)]) == (Link_p)1)
		u.u_error = ENXIO;
	else {
		/*
		 * check for possible bad pointer to avoid PANIC
		 */
		if(lp == (Link_p)0) {
			u.u_error = EBADF;
			return;
		}
		if ((CHAN(dev) == 0) && (lp->l.pid != u.u_procp->p_pid)) {
			u.u_error = ENXIO;
			return;
		}
		tp = &lp->chans[CHAN(dev)].tty;

		(*linesw[tp->t_line].l_write)(tp);
	}
}



/*
**      Driver output processor for virtual 'tty's
*/

xtvtproc(tp, cmd)
register struct tty *   tp;
short     cmd;
{
	register Pch_p  pcp = PCHANMATCH(tp);
	register int    x = spltty();   /** This should be unnecessary! **/

	switch (cmd) {
	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		goto restart;

	case T_WFLUSH:
		tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
		tp->t_tbuf.c_count = 0;
#if BLITIXON != 1
	case T_RESUME:
		tp->t_state &= ~TTSTOP;
#endif
	case T_OUTPUT:

restart:

#if BLITIXON != 1
		if (tp->t_state & TTSTOP)
			break;
#endif
		if (tp->t_tbuf.c_ptr == NULL || tp->t_tbuf.c_count == 0) {
			if (tp->t_tbuf.c_ptr)
				tp->t_tbuf.c_ptr -= tp->t_tbuf.c_size;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp)))
				break;
#if PIGGY_BACK == 1
			if (tp->t_tbuf.c_count == 1 && (pcp->flags & XACK)) {
				/* Here, for the sake of efficiency, we put a
				** single char echo in the acknowledgement packet
				** as control data.
				*/
				register int i;

				for(i=0; i < NPCBUFS; i++)
					if(pcp->pkts[i].state == PX_WAIT)
						break;
				if (i == NPCBUFS) { /* no unACKed packets */
					pcp->cdata[GET_SEQ(LINKMATCH(pcp)->l.rpkt.pkt)] = *tp->t_tbuf.c_ptr++;
					tp->t_tbuf.c_count = 0;
					pcp->flags |= XCDATA;
				}
				break;
			}
#endif
		}
		if (pcp->outen && !(pcp->flags & XACK))
			start(tp, pcp);
		break;

#if BLITIXON != 1
	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		break;
#endif

	case T_BLOCK:
		tp->t_state &= ~TTXON;
		tp->t_state |= TTXOFF | TBLOCK;
		start(tp, pcp);
		break;

	case T_RFLUSH:
		if (!(tp->t_state & TBLOCK))
			break;
	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF | TBLOCK);
		tp->t_state |= TTXON;
		start(tp, pcp);
		break;

		/*       case T_BREAK:
		/** send break packet? **/
	}

	splx(x);
}



/*
**      Line discipline interfaces
*/

/*
**      Create packets and move them to the real output queue
*/

static void
start(tp, pcp)
register struct tty *tp;
register Pch_p       pcp;
{
	register Link_p lp;
	register Pks_p  psp;
	register int    count;
	register struct cblock *cb;
	register int    i;
	char    *fixpt1;                        /* RHODES FIX */
	char    *fixpt2;                        /* RHODES FIX */
	int     fixcnt;                 /* RHODES FIX */


	if ((lp = LINKMATCH(pcp)) == (Link_p)0 || lp == (Link_p)1) {
	    if (xterrstart++ % 25 == 1) {  
		printf("xt driver warning: start(%d,%d):",(int)(tp),pcp);
		printf(" lp=%x, Excessive bad packets.\n",lp);
		printf("xterrstart = %d\n",xterrstart);
		printf("Check for noisy terminal lines.\n");
	    }
	    return;     /* bad channel number... ignore packet */
	}

	if (pcp->flags & (XACK | XNAK)) {
		if (cb = getcf()) {
			struct ackpkt { 
				Pbyte ackpkt[4+2+EDSIZE]; 
			};

			psp = &lp->l.rpkt;
			SET_CNTL(psp->pkt, 1);
			if (pcp->flags & XCDATA) {
				STATS(lp, S_XACK);
				psp->pkt.data[1] = pcp->cdata[GET_SEQ(psp->pkt)];
				psp->pkt.data[0] = ACK;
				count = 2;
			} else if (pcp->flags & XNAK) {
				STATS(lp, S_XNAK);
				psp->pkt.data[0] = NAK;
				count = 1;
			} else {
				STATS(lp, S_XACK);
				count = 0;
			}
			psp->pkt.HEADER_DSIZE = count;
			count += 2;
#ifndef vax
			(void)crc((Pbyte *) & psp->pkt, count);
#else
			/* WARNING - assumes 'count' in r7 */
			asm("   crc     _crc16t,$0,r7,(r8)      ");
			/* WARNING - assumes 'psp' in r8 */
			asm("   movw    r0,(r3)                 ");
#endif
			count += EDSIZE;
			if (!lp->l.hex) {
			    /* bcopy((char *)&psp->pkt, cb->c_data, count); **/
			    /* *((struct ackpkt *)(&cb->c_data[0])) = *((struct ackpkt *)(&psp->pkt)); */      /* RHODES FIX */
			    fixpt1 = (char *)(&cb->c_data[0]);      /* RHODES FIX */
			    fixpt2 = (char *)(&psp->pkt);   /* RHODES FIX */
			    for (fixcnt = 0; fixcnt < 8; fixcnt++) {  /*RHODES FIX*/
				    *fixpt1++ = *fixpt2++;  /* RHODES FIX */
			    }       /* RHODES FIX */
			    cb->c_last = count;
			} else {
			    cb->c_last = mybcopy((char *)&psp->pkt, cb->c_data, count);
			}
			putcb(cb, &lp->l.line->t_outq);
			Logpkt(&psp->pkt, lp, XMITLOG);
			STATS(lp, S_XPKTS);
		} else
			STATS(lp, S_NOCFREE);
		pcp->flags &= ~(XACK | XNAK | XCDATA);
	} else
		cb = (struct cblock *)0;

	/*
	**      If there is room on real 'tty' output queue:
	**      round up packets and move 'em out.
	*/

	if ((count = lp->l.lihiwat - lp->l.line->t_outq.c_cc) >= sizeof(struct Packet)
	     && (tp->t_state & ISOPEN))
		for (psp = pcp->nextpkt, i = NPCBUFS ; i--;) {
			if (psp->size == 0
			     && tp->t_tbuf.c_count
			     && pcp->outen
#if BLITIXON != 1
			     && !(tp->t_state & TTSTOP)
#endif
			)
				makepkt(tp, pcp, psp);
			if (psp->state == PX_READY && count >= (int)psp->size) {
				/** N.B. assumes (sizeof Packet) <= CBSIZE **/

				count -= psp->size;
				if ((cb = getcf()) == (struct cblock *)0) {
					STATS(lp, S_NOCFREE);
					cb++;
					break;
				}
				if (!lp->l.hex) {
#ifndef vax
					bcopy((caddr_t)&psp->pkt, (caddr_t)cb->c_data, (int)psp->size);
#else
					*((Pkt_p)(&cb->c_data[0])) = psp->pkt;
#endif
					cb->c_last = psp->size;
			    	} else {
					cb->c_last = mybcopy((char *)&psp->pkt, cb->c_data, (int)psp->size);
			    	}
				/* Packet to real 'tty' */
				putcb(cb, &lp->l.line->t_outq);
				psp->state = PX_WAIT;
				lp->l.lchan = pcp->channo;
				psp->timo = lp->l.xtimo;
				Logpkt(&psp->pkt, lp, XMITLOG);
				STATS(lp, S_XPKTS);
			}
			if (++psp >= &pcp->pkts[NPCBUFS])
				psp = pcp->pkts;
		}

	if (cb)
		/* Starts real 'tty' output driver */
		(*lp->l.line->t_proc)(lp->l.line, T_OUTPUT);

	if (pcp->outen != NPCBUFS)
		tp->t_state |= BUSY;
	else
		tp->t_state &= ~BUSY;
}


/* the following routine copies one buffer to another, while encoding    */
/* from an 8-bit path to 6 bits.  As a result, the length of the new     */
/* buffer (which is returned with the function) is roughly 4/3 times     */
/* the original buffer size(n).                                  */
/* This is probably ok, as MAX PKT SIZE is 36 bytes (2 + 32 + 2) */
/* giving us a new maximium buffer size of 48, which is less     */
/* the allowable 64 bytes (from the tty structure)               */

static int
mybcopy (from, to, n)                   /* for hex paths */
unsigned char *from, *to;
int n;
{
	unsigned char c1, c2, c3, *fp, *tp;
	short i;
	for (fp = from, tp = to, i = 0 ; i < n ; i+=3) {
		c1 = *fp++;
		if (i+1 < n)
			c2 = *fp++;
		if (i+2 < n)
			c3 = *fp++;

		*tp++ = 0x40 | (c1&0xc0)>>2 | (c2&0xc0)>>4 | (c3&0xc0)>>6;
		*tp++ = 0x40 | (c1&0x3f);
		if (i+1 < n)
			*tp++ = 0x40 | (c2&0x3f);
		if (i+2 < n)
			*tp++ = 0x40 | (c3&0x3f);
	}
	*to &= 0x3f;                  /* beginning of packet indicator */
	return(tp - to);
}



/*
**      Extract data from virtual output queue, and make up a packet
*/

static void
makepkt(tp, pcp, psp)
register struct tty *   tp;
register Pch_p          pcp;
register Pks_p          psp;
{
	register int count = min(tp->t_tbuf.c_count, MAXPKTDSIZE);
	struct datapt { 
		char datapt[MAXPKTDSIZE]; 
	};

#ifndef vax
	bcopy(tp->t_tbuf.c_ptr, (char *)psp->pkt.data, count);
#else
	*((struct datapt *)(&psp->pkt.data[0])) = *((struct datapt *)(&tp->t_tbuf.c_ptr[0]));
#endif
	tp->t_tbuf.c_ptr += count;
	if ((tp->t_tbuf.c_count -= count) == 0) {
		tp->t_tbuf.c_ptr -= tp->t_tbuf.c_size;
		(*linesw[tp->t_line].l_output)(tp);
	}
	psp->pkt.HEADER_DSIZE = count;
	count += 2;
	SET_PTYP(psp->pkt, 1);
	SET_CNTL(psp->pkt, 0);
	SET_SEQ(psp->pkt, pcp->xseq++);
	SET_CHAN(psp->pkt, pcp->channo);
#ifndef vax
	(void)crc((Pbyte *) & psp->pkt, count);
#else
	asm("   crc     _crc16t,$0,r8,(r9)      ");
	asm("   movw    r0,(r3)                 ");
#endif
	psp->size = count + EDSIZE;
	psp->state = PX_READY;
	pcp->outen--;
}



#if XTRACE == 1

/*
**      Maintains a circular buffer of the last 'PKTHIST' transactions
*/

static void
logpkt(pkp, lp, dir)
Pkt_p                   pkp;
register Link_p         lp;
int     dir;
{
	register struct Tpkt *  tp;
	struct pktpt { 
		Pbyte pktpt[PKTPTSZ]; 
	};
	char    *fixpt1;                        /* RHODES FIX */
	char    *fixpt2;                        /* RHODES FIX */
	int     fixcnt;                 /* RHODES FIX */

	tp = &lp->l.trace.log[lp->l.trace.index];
	/*      *((struct pktpt *)(&tp->pktpart[0])) = *((struct pktpt *)pkp); */  /* RHODES FIX */
	fixpt1 = (char *)(&tp->pktpart[0]);     /* RHODES FIX */
	fixpt2 = (char *)(pkp); /* RHODES FIX */
	for (fixcnt = 0; fixcnt < 8; fixcnt++) {  /*RHODES FIX*/
		*fixpt1++ = *fixpt2++;  /* RHODES FIX */
	}       /* RHODES FIX */
	tp->flag = dir;
	tp->time = lbolt;
	if (++lp->l.trace.index >= PKTHIST)
		lp->l.trace.index = 0;
	if (lp->l.trace.used < PKTHIST)
		lp->l.trace.used++;
}


#endif



/*
**      Scan active channel groups to restart blocked output
*/

xtscan()
{
	register Pks_p  psp;
	register Ch_p   chp;
	register int    bufs;
	register int    chans;
	register Link_p lp;
	register int    link;
	register int    restart;

	scanning = 0;

	for (link = 0 ; link < MAXLINKS ; link++)
		if ((lp = linkTable[link]) != (Link_p)0 && lp != (Link_p)1) {
			scanning++;
			if (lp->l.rpkt.timo && --lp->l.rpkt.timo == 0) {
				lp->l.rpkt.state = PR_NULL;
				STATS(lp, S_RTIMO);
			}
			for (chp = &lp->chans[lp->l.lchan], chans = lp->l.nchans ; chans--;) {
				if (++chp >= &lp->chans[lp->l.nchans])
					chp = lp->chans;
				if ((chp->tty.t_state & ISOPEN) == 0)
					continue;
				restart = 0;
				for (psp = chp->chan.nextpkt, bufs = NPCBUFS ; bufs--;) {
					if (psp->timo && --psp->timo == 0) {
						psp->state = PX_READY;
						STATS(lp, S_XTIMO);
						restart++;
					} else if (psp->state == PX_READY
					           || (psp->size == 0 && chp->tty.t_tbuf.c_count && chp->chan.outen))
						restart++;
					if (++psp >= &chp->chan.pkts[NPCBUFS])
						psp = chp->chan.pkts;
				}
				if (restart)
					start(&chp->tty, &chp->chan);
				if (chp->tty.t_outq.c_cc == 0) {
					if ((chp->tty.t_state & (TTIOW | BUSY | TIMEOUT)) == TTIOW) {
						chp->tty.t_state &= ~TTIOW;
						wakeup((caddr_t) & chp->tty.t_oflag);
						STATS(lp, S_WIOW);
					}
/*					if (chp->tty.t_state & OASLP) {
**						chp->tty.t_state &= ~OASLP;
**						wakeup((caddr_t)&chp->tty.t_outq);
**						STATS(lp, S_WOAS);
**					   }
*/                                                      }
			}
		}

	if (scanning)
		timeout((void (*)())xtscan, (char *)0, HZ/2);
}



/*
**      Xt line disciplines
*/

/*
**      Move real 'tty' 'outq' to 'tbuf'
*/

xtout(tp)
register struct tty *           tp;
{
	register struct ccblock *       tbuf;

	tbuf = &tp->t_tbuf;
	if (tbuf->c_ptr)
		putcf(CMATCH((struct cblock *)tbuf->c_ptr));
	if ((tbuf->c_ptr = (char *)getcb(&tp->t_outq)) == NULL)
		return 0;
	tbuf->c_count = ((struct cblock *)tbuf->c_ptr)->c_last - 
	    ((struct cblock *)tbuf->c_ptr)->c_first;
	tbuf->c_size = tbuf->c_count;
	tbuf->c_ptr = (caddr_t) &((struct cblock *)tbuf->c_ptr)->c_data
	    [((struct cblock *)tbuf->c_ptr)->c_first];
	return CPRES;
}



/*
**      Demultiplex partial packets received in real 'tty' 'rbuf'
*/

#define Pkt             pkp->pkt
#define State           pkp->state
#define Timo            pkp->timo
#define Count           pkp->size
#define Data            Pkt.data
#define Nextseq         chp->chan.rseq
#define Flags           chp->chan.flags

xtin(tp)
register struct tty *   tp;
{
	register Pbyte  *cp;
	register Link_p lp;
	register int    n;
	register Pks_p  pkp;
	Pbyte *cp1, *cp2;
	static unsigned char temp;
	short i;
	static short count = 6;


	/* bad ptr in xtin suspected... */
	if ((unsigned long) tp <= (unsigned long)  1) {
		if (xterrxtin++ % 25 == 1) { 
			printf("xt driver warning: bad tp (%d)\n", tp);
			printf("xterrxtin = %d\n",xterrxtin);
			printf("Check for noisy terminal lines.\n");
		}
		return;
	}

	if ((n = tp->t_rbuf.c_size - tp->t_rbuf.c_count) == 0)
		return;
	tp->t_rbuf.c_count = tp->t_rbuf.c_size;
	cp = (Pbyte *) tp->t_rbuf.c_ptr;
	lp = linkTable[tp->t_link];
	if (lp->l.hex) {
		for (i = 0, cp2=cp1=cp ; i < n ; i++, cp2++) {
			if (*cp2 < 0x40 && ((*cp2 & 0xe0) != 0x20))
				continue;
			count += 2;
			if (((*cp2 & 0xe0) == 0x20) || (count == 8)) {
				count = 0;
				temp = *cp2;
			}
			else
				*cp1++ = (*cp2 & 0x3f) | ((temp << count) & 0xc0);
		}
		n = cp1 - cp;
		if (n == 0) return;
	}
	pkp = &lp->l.rpkt;

	do {
		switch (State) {
		case PR_NULL:
			pkp->pkt.header[0] = *cp++;
			if (GET_PTYP(pkp->pkt) != 1) {
#if XTRACE == 1
				Logpkt((Pkt_p)(cp - 1), lp, RECVLOG);
				LOCKTRACE(lp);
#endif
				STATS(lp, S_BADHDR);
				break;
			}
			Timo = (Pbyte) lp->l.rtimo;
			State = PR_SIZE;
			continue;

		case PR_SIZE:
			pkp->pkt.header[1] = *cp++;
			if ((Count = pkp->pkt.HEADER_DSIZE) > MAXPKTDSIZE) {
#if XTRACE == 1
				Logpkt(&Pkt, lp, RECVLOG);
				LOCKTRACE(lp);
#endif
				STATS(lp, S_BADSIZE);
				break;
			}
			Count += EDSIZE;
			lp->l.rdatap = (char *)Data;
			State = PR_DATA;
			continue;

		case PR_DATA:
			*lp->l.rdatap++ = *cp++;
			if (--Count != 0)
				continue;

			/*
			**      Now at CRC
			*/

			STATS(lp, S_RPKTS);
			Logpkt(&Pkt, lp, RECVLOG);

			if (crc((Pbyte *) & Pkt, (int)(pkp->pkt.HEADER_DSIZE + 2)))
				STATS(lp, S_CRCERR);
			else {
				register Ch_p   chp = &lp->chans[GET_CHAN(pkp->pkt)];

				if (GET_CHAN(pkp->pkt) >= lp->l.nchans) {
					STATS(lp, S_BADCHAN);
					break;
				}

				if (GET_CNTL(pkp->pkt))
					control(lp, chp);
				else if (GET_SEQ(pkp->pkt) == Nextseq) {
					chp->chan.cdata[GET_SEQ(pkp->pkt)] = 0;
					Flags |= XACK;
					if (recvpkt(lp, chp, tp))
						Nextseq++;
					else
						Flags &= ~XACK;
				} else if (retry(pkp, chp)) {
					STATS(lp, S_RDUP);
					Flags |= XACK;
				} else {
					STATS(lp, S_OUTSEQ);
					LOCKTRACE(lp);
					Flags |= XNAK;
				}
				start(&chp->tty, &chp->chan);
			}
		}

		Timo = 0;
		State = PR_NULL;
	} while (--n);
}



/*
**      Deal with control packet
*/

static void
control(lp, chp)
register Link_p lp;
Ch_p            chp;
{
	register Pks_p  pkp = &lp->l.rpkt;
	register Pch_p  pcp = &chp->chan;
	register Pks_p  psp = pcp->nextpkt;
	register Pbyte *lastseqp = &seqtable[GET_SEQ(pkp->pkt)+SEQMOD];
	register Pbyte *seqp = lastseqp -(NPCBUFS-1);
	register int    hit = 0;

	if (pkp->pkt.HEADER_DSIZE == 0)
		goto ack;

	switch (Data[0]) {
	case ACK:
ack:
		/** This and all lesser sequenced packets ok **/

		STATS(lp, S_RACK);

		do 
			if (*seqp == GET_SEQ(psp->pkt)) {
				if (psp->state == PX_WAIT) {
					psp->state = PX_OK;
					psp->size = 0;
					psp->timo = 0;
					hit++;
				}
				if (++psp >= &pcp->pkts[NPCBUFS])
					psp = pcp->pkts;
			}
		while (++seqp <= lastseqp);

		if (hit) {
			pcp->nextpkt = psp;
			if (hit > 1) {
				/** Only condition where an UNBLK maybe lost! **/

				if ((pcp->outen += (hit - 1)) > NPCBUFS)
					pcp->outen = NPCBUFS;
				STATS(lp, S_LOSTACK);
			}
			break;
		}
		STATS(lp, S_BADACK);
		return;

	case NAK:
		/** Retransmit this and all lesser sequenced packets **/

		STATS(lp, S_RNAK);

		do 
			if (*seqp == GET_SEQ(psp->pkt)) {
				if (psp->state == PX_WAIT) {
					psp->state = PX_READY;
					STATS(lp, S_NAKRETRYS);
					hit++;
				}
				if (++psp >= &pcp->pkts[NPCBUFS])
					psp = pcp->pkts;
			}
		while (++seqp <= lastseqp);

		if (!hit) {
			STATS(lp, S_BADNAK);
			LOCKTRACE(lp);
			return;
		}
		break;

	case PCDATA:
		break;

	default:
		STATS(lp, S_BADCNTL);
		LOCKTRACE(lp);
		return;
	} 

	if (pkp->pkt.HEADER_DSIZE > 1)
#if DEBUG == 1
		if (Data[1] != C_UNBLK || pkp->pkt.HEADER_DSIZE != 2) {
			STATS(lp, S_BADCDATA);
			LOCKTRACE(lp);
		} else
#endif
		if (++pcp->outen > NPCBUFS)
			pcp->outen = NPCBUFS;
}



/*
**      Process data from Blit
*/

static int      
recvpkt(lp, chp, rtp)
register Link_p lp;
Ch_p            chp;
struct tty     *rtp;
{
	register Pks_p       pkp = &lp->l.rpkt;
	register struct tty *tp = &chp->tty;
	register Pbyte      *cp = Data;
	register short    c;

	while (pkp->pkt.HEADER_DSIZE-- != 0) {
		switch (c = *cp++) {
			register int    n;

		case C_SENDCHAR:        /* One char from layer */
			n = 1;
			goto sendnchar;

		case C_SENDNCHARS:
			n = pkp->pkt.HEADER_DSIZE;
sendnchar:
			pkp->pkt.HEADER_DSIZE -= n;
			if (!(tp->t_state & ISOPEN))
				return 1;
			while (n-- > 0) {
				c = *cp++;
#if BLITIXON != 1
				if (tp->t_iflag & IXON)
					if (tp->t_state & TTSTOP) {
						if ((c & 0177) == CSTART) {
							(void)xtvtproc(tp, T_RESUME);
							continue;
						}
						if (tp->t_iflag & IXANY)
							(void)xtvtproc(tp, T_RESUME);
						if ((c & 0177) == CSTOP)
							continue;
					}
					else if ((c & 0177) == CSTOP) {
						(void)xtvtproc(tp, T_SUSPEND);
						continue;
					}
					else if ((c & 0177) == CSTART)
						continue;
#endif
				if (tp->t_rbuf.c_ptr == NULL)
					return 0;
				if (tp->t_iflag & ISTRIP)
					c &= 0177;
#ifdef VPIX
				if (chp->v86p && !tp->t_rawq.c_cc)
					v86setint(chp->v86p, V86VI_KBD);
#endif
				*tp->t_rbuf.c_ptr++ = c;
				if (--tp->t_rbuf.c_count == 0) {
					tp->t_rbuf.c_ptr -= 
					    tp->t_rbuf.c_size - tp->t_rbuf.c_count;
					(*linesw[tp->t_line].l_input)(tp);
				}
			}
			tp->t_rbuf.c_ptr -= 
			    tp->t_rbuf.c_size - tp->t_rbuf.c_count;
			(*linesw[tp->t_line].l_input)(tp);
			break;
		case C_UNBLK:
			if (++chp->chan.outen > NPCBUFS)
				chp->chan.outen = NPCBUFS;
			break;

		case C_NEW:
		case C_DELETE:
		case C_EXIT:
			tp = &lp->chans[0].tty;
			if (tp->t_rbuf.c_ptr) {
				tp->t_rbuf.c_ptr[0] = c;
				tp->t_rbuf.c_ptr[1] = GET_CHAN(pkp->pkt);
				tp->t_rbuf.c_count -= 2;
				(*linesw[tp->t_line].l_input)(tp);
			} else
				STATS(lp, S_NORBUF);
			if (c != C_NEW)
				break;

		case C_RESHAPE:
			chp->winsize.bytesx = *cp++;
			chp->winsize.bytesy = *cp++;
			chp->winsize.bitsx = *cp++;
			chp->winsize.bitsx |= *cp++ << 8;
			chp->winsize.bitsy = *cp++;
			chp->winsize.bitsy |= *cp++ << 8;
			pkp->pkt.HEADER_DSIZE -= 6;
			break;

		case C_DEFUNCT:
			if (tp->t_state & ISOPEN) {
				signal(tp->t_pgrp, SIGTERM);
				ttyflush(tp, (FREAD | FWRITE));
			}
			break;

		case JAGENT & 0xFF:
			if (lp->l.agent.retcb != (struct cblock *)NULL)
				putcf(lp->l.agent.retcb);
			/* allocate new buffer to return the data to ioctl */
			if ((lp->l.agent.retcb = getcf()) == (struct cblock *) NULL)
				return 0;       /* Blit will retry */
			/* here we want to copy data from Data to retcb->c_data */
			bcopy((caddr_t)cp, (caddr_t)lp->l.agent.retcb->c_data, (unsigned)pkp->pkt.HEADER_DSIZE);
			wakeup((caddr_t)&lp->l.agent.desc.dest);
			return 1;

		default:
			STATS(lp, S_BADCTYPE);
			LOCKTRACE(lp);
			return 1;
		}

#if DEBUG == 1
		if (pkp->pkt.HEADER_DSIZE > MAXPKTDSIZE) {  /* ie. gone negative */
			STATS(lp, S_BADCOUNT);
			LOCKTRACE(lp);
			return 1;
		}
#endif
	}

	return 1;
}



/*
**      Packet out of sequence -- is this a valid retransmission?
*/

static int      
retry(pkp, chp)
register Pks_p  pkp;
Ch_p            chp;
{
	register Pbyte *lastseqp = &seqtable[Nextseq+SEQMOD-1];
	register Pbyte *seqp = lastseqp -(NPCBUFS-1);

	do 
		if (*seqp == GET_SEQ(pkp->pkt)) {
			if (chp->chan.cdata[GET_SEQ(pkp->pkt)])
				chp->chan.flags |= XCDATA;
			return 1;
		}
	while (++seqp <= lastseqp);

	return 0;
}



/*
**      Packet check bytes calculation
*/

#ifdef vax
/*
**      Vax "crc" instruction look-up table for polynomial = 0120001
**      (3.1 us./byte on 780)
*/

static unsigned long    crc16t[] = 
{
	0, 0146001, 0154001,  012000, 0170001,  036000,  024000, 0162001
	, 0120001,  066000,  074000, 0132001,  050000, 0116001, 0104001,  042000
};


/*ARGSUSED*/
static int      
crc(s, n)
unsigned char   *       s;
int     n;
{
	asm("   crc     _crc16t,$0,8(ap),*4(ap) ");
	asm("   cmpw    r0,(r3)                 ");
	asm("   beqlu   OK                      ");
	asm("   movw    r0,(r3)                 ");
	asm("   movl    $1,r0                   ");
	asm("   ret                             ");
	asm("OK:movw    r0,(r3)                 ");
	return 0;
}


#else

/*
**      These routines can be sped up by a factor of 60%
**      if hand coded after 'cc -O -S' on the vax.
*/

#define blobyte(X)      (X&0xff)
#define bhibyte(X)      ((X>>8)&0xff)

#if FAST_BUT_BIG != 1

/*
**      CRC16:  x**16 + x**15 + x**2 + 1 : using 64 byte look-up table
**      (18.2 us./byte on 780)
*/

static ushort   crc16t_32[2][16]         = 
{
	0, 0140301, 0140601, 0500, 0141401, 01700, 01200, 0141101,
	0143001, 03300, 003600, 0143501, 02400, 0142701, 0142201, 02100,
	0, 0146001, 0154001, 012000, 0170001, 036000, 024000, 0162001,
	0120001, 066000, 074000, 0132001, 050000, 0116001, 0104001, 042000
};


static int      
crc(buffer, nbytes)
register Pbyte *buffer;
int      nbytes;
{
	register ushort tcrc = 0;
	register long   temp;
	register long   i;

	if ((i = nbytes) > 0)
		do {
			temp = tcrc ^ *buffer++;
			tcrc = crc16t_32[0][temp & 017]
			     ^ crc16t_32[1][(temp>>4) & 017]
			     ^ (tcrc >> 8);
		} while (--i > 0);

	if (blobyte(tcrc) != *buffer)
		i++;
	*buffer++ = blobyte(tcrc);

	if (bhibyte(tcrc) != *buffer)
		i++;
	*buffer++ = bhibyte(tcrc);

	return i;
}


#else

/*
**      CRC16:  x**16 + x**15 + x**2 + 1 : using 512 byte look-up table
**      (11.1 us./byte on 780 (6.7 us/byte if hand coded))
*/

static ushort   crc16t_256[256]  = 
{
	0, 0140301, 0140601,    0500, 0141401,   01700,   01200, 0141101
	, 0143001,   03300,   03600, 0143501,   02400, 0142701, 0142201,   02100
	, 0146001,   06300,   06600, 0146501,   07400, 0147701, 0147201,   07100
	,  05000, 0145301, 0145601,   05500, 0144401,   04700,   04200, 0144101
	, 0154001,  014300,  014600, 0154501,  015400, 0155701, 0155201,  015100
	, 017000, 0157301, 0157601,  017500, 0156401,  016700,  016200, 0156101
	, 012000, 0152301, 0152601,  012500, 0153401,  013700,  013200, 0153101
	, 0151001,  011300,  011600, 0151501,  010400, 0150701, 0150201,  010100
	, 0170001,  030300,  030600, 0170501,  031400, 0171701, 0171201,  031100
	, 033000, 0173301, 0173601,  033500, 0172401,  032700,  032200, 0172101
	, 036000, 0176301, 0176601,  036500, 0177401,  037700,  037200, 0177101
	, 0175001,  035300,  035600, 0175501,  034400, 0174701, 0174201,  034100
	, 024000, 0164301, 0164601,  024500, 0165401,  025700,  025200, 0165101
	, 0167001,  027300,  027600, 0167501,  026400, 0166701, 0166201,  026100
	, 0162001,  022300,  022600, 0162501,  023400, 0163701, 0163201,  023100
	, 021000, 0161301, 0161601,  021500, 0160401,  020700,  020200, 0160101
	, 0120001,  060300,  060600, 0120501,  061400, 0121701, 0121201,  061100
	, 063000, 0123301, 0123601,  063500, 0122401,  062700,  062200, 0122101
	, 066000, 0126301, 0126601,  066500, 0127401,  067700,  067200, 0127101
	, 0125001,  065300,  065600, 0125501,  064400, 0124701, 0124201,  064100
	, 074000, 0134301, 0134601,  074500, 0135401,  075700,  075200, 0135101
	, 0137001,  077300,  077600, 0137501,  076400, 0136701, 0136201,  076100
	, 0132001,  072300,  072600, 0132501,  073400, 0133701, 0133201,  073100
	, 071000, 0131301, 0131601,  071500, 0130401,  070700,  070200, 0130101
	, 050000, 0110301, 0110601,  050500, 0111401,  051700,  051200, 0111101
	, 0113001,  053300,  053600, 0113501,  052400, 0112701, 0112201,  052100
	, 0116001,  056300,  056600, 0116501,  057400, 0117701, 0117201,  057100
	, 055000, 0115301, 0115601,  055500, 0114401,  054700,  054200, 0114101
	, 0104001,  044300,  044600, 0104501,  045400, 0105701, 0105201,  045100
	, 047000, 0107301, 0107601,  047500, 0106401,  046700,  046200, 0106101
	, 042000, 0102301, 0102601,  042500, 0103401,  043700,  043200, 0103101
	, 0101001,  041300,  041600, 0101501,  040400, 0100701, 0100201,  040100
};


static int      
crc(buffer, nbytes)
register Pbyte *buffer;
int     nbytes;
{
	register ushort tcrc = 0;
	register int    i;

	if ((i = nbytes) > 0)
		do
			tcrc = crc16t_256[(tcrc^(*buffer++))&0xff] ^ (tcrc >> 8);
		while (--i > 0);

	if (blobyte(tcrc) != *buffer)
		i++;
	*buffer++ = blobyte(tcrc);

	if (bhibyte(tcrc) != *buffer)
		i++;
	*buffer++ = bhibyte(tcrc);

	return i;
}

#endif
#endif

#define PRE1S4_9 /* we need this for errlayers, do we need errlayers? */
#ifdef PRE1S4_9
conlayers()             /* is the console running layers?? */
{
	struct tty      *tp;
	extern int      xtin();

				/* console has major dev. entry of 0 */
				/* get pointer to console tty structure */
	tp = cdevsw[0].d_ttys;

				/* use console line dicipline and linesw */
				/* to really determine if layers is running */
	if (linesw[tp->t_line].l_input == xtin)
		return(1);      /* true, layers is running */
	else
		return(0);      /* false, layers is not running */
}
#endif
struct tty *
errlayer()              /* return the tty pointer of the console error layer */
{
	Link_p          lp;     /* really a pointer */
	struct tty      *tp;    /* a pointer, REALLY */

				/* console has major dev. entry of 0 */
				/* cdevsw[0].ttys gives a pointer to */
				/* the console tty structure */
	tp = cdevsw[0].d_ttys;
				/* is the console running layers? */
	if (conlayers()) {      /* if yes.....                  */
				/* t_link is a back pointer through the */
				/* linkTable to the layers group running */
				/* on the that tty      */
		lp = linkTable[tp->t_link];
				/* now return a pointer to the error layer */
				/* tty structure, channel 2 */
		return(&lp->chans[2].tty);
	}
	else                    /* if no, then return the real console tty */
		return(tp);
}


Link_p
xtalloc()
{
	register i;
	Link_p lp;

	if (xtbuf == NULL) {
		/* This is a kludge because VAX config messes up init routines */
		xtinit();
		if (xtbuf == NULL)      {
			return NULL ;
		}
	}
	for (i=0; i < xt_cnt; i++)
		if (!xtbusy[i]) {
			lp = (Link_p) (xtbuf + (i*LINKSIZE));
			bzero((char *)lp, LINKSIZE);
			xtbusy[i] = 1;
			return lp;
		}
	return NULL;
}

xtfree(lp)
Link_p lp;
{
	long i;

	i = ((char *)lp - xtbuf)/LINKSIZE;
	xtbusy[i] = 0;
}
#if  defined(i286) || defined(i386)

xtinit()
{
	extern char xt_buf[];

	if((xtbuf = xt_buf) == NULL)
		printf("xt cannot allocate link buffers\n");
}

#else
xtinit()
{
	/* n = sizeof(struct Link) + sizeof(struct Channel)*(nchans-1); */
	xtbuf = xtbuffer;
	/*
	 * Code below was in the XT driver (VAX) - we don't have
	 * THE SAME CALLS(BTALLOC, ETC
	if ((xtbuf = (char *)sptalloc(btoc(xt_cnt*LINKSIZE), PG_V|PG_KW, 0)) == NULL)
		printf("xt cannot allocate link buffers\n");
	*/
	/* LINKSIZE is 2272! */
}
#endif

int
xtnullproc(tp, cmd)
register struct tty *   tp;
int                     cmd;
{
	ttyflush(tp, (FREAD|FWRITE));
}
