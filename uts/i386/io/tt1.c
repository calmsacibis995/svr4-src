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

#ident	"@(#)kern-io:tt1.c	1.3.1.1"

/*
 * Line discipline 0
 * and alternative line discipline for serial printers (no RESUME on close).
 * No Virtual Terminal Handling
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/proc.h"
#include "sys/conf.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/sysinfo.h"
#include "sys/emap.h"

#if DEBUG
int	debug_tty = 0;
#endif

extern char partab[];

/*
 * routine called on first teletype open.
 * establishes a process group for distribution
 * of quits and interrupts from the tty.
 */
ttopen(tp)
register struct tty *tp;
{
	register struct proc *pp;
	int error;

	pp = u.u_procp;

#if DEBUG
	if (debug_tty == 0x01) {
		printf("ttopen: pid=%d, pgrp=%x\n",pp->p_pid, pp->p_pgrp);
		printf("ttopen: ttyp=%d, tppgrp=%x\n",u.u_ttyp, tp->t_pgrp);
	}
#endif /* DEBUG */

	if ((pp->p_pid == pp->p_pgrp)
	 && (u.u_ttyp == NULL)
	 && (tp->t_pgrp == 0)) {
		u.u_ttyp = &tp->t_pgrp;
		tp->t_pgrp = pp->p_pgrp;
	}

	/*
	 * Check to see if Exclusive mode is set.
	 * If it is, then do not allow any open if
	 * caller is not super user.
	 * Added for implementing MP ioctl extensions,
	 * particularly TIOCEXCL call.
	 */

	if ( (tp->t_lflag & XCLUDE) && (error=drv_priv(u.u_cred)) ) {
#if DEBUG
		if (debug_tty == 0x01) {
			printf("In ttopen(): XCLUDE set, tp->lflag = 0x%x\n",
								tp->t_lflag);
			printf("In ttopen(): u.u_error = EBUSY\n");
		}
#endif
		u.u_error = error;
		return;
	}

	ttioctl(tp, LDOPEN, 0, 0);
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
}

ttclose(tp)
register struct tty *tp;
{
	if ((tp->t_state&ISOPEN) == 0)
		return;
	tp->t_state &= ~ISOPEN;
	ttioctl(tp, LDCLOSE, 0, 0);
	tp->t_lflag &= ~XCLUDE;
#if DEBUG
	if (debug_tty == 0x01)
		printf("In ttclose(): tp->t_lflag = ~XCLUDE...\n");
#endif
	tp->t_pgrp = 0;
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ttread(tp)
register struct tty *tp;
{
	register struct clist *tq;

	tq = &tp->t_canq;
	if (tq->c_cc == 0)
		canon(tp);
	while (u.u_count!=0 && u.u_error==0) {
		if (u.u_count >= CLSIZE) {
			register int n;
			register struct cblock *cp;

			if ((cp = getcb(tq)) == NULL)
				break;
			n = min(u.u_count, cp->c_last - cp->c_first);
			if (copyout((caddr_t)&cp->c_data[cp->c_first], u.u_base, n))
				u.u_error = EFAULT;
			putcf(cp);
			u.u_base += n;
			u.u_count -= n;
		} else {
			register int c;

			if ((c = getc(tq)) < 0)
				break;
			if (subyte(u.u_base++, c))
				u.u_error = EFAULT;
			u.u_count--;
		}
	}
	if (tp->t_state&TBLOCK) {
		if (tp->t_rawq.c_cc<TTXOLO) {
			(*tp->t_proc)(tp, T_UNBLOCK);
		}
	}
}

/*
 * Called from device's write routine after it has
 * calculated the tty-structure given as argument.
 */
ttwrite(tp)
register struct tty *tp;
{
	register int	oldpri;

	if (!(tp->t_state&CARR_ON))
		return;
	while (u.u_count) {
		if (tp->t_outq.c_cc > tthiwat[tp->t_cflag&CBAUD]) {
			oldpri = spltty();
			/*
			 * Do output outside the loop since the console doesn't
			 * issue an interrupt to wake up the sleep when the
			 * output is completed.
			 */
			(*tp->t_proc)(tp, T_OUTPUT);
			while (tp->t_outq.c_cc > tthiwat[tp->t_cflag&CBAUD]) {
				tp->t_state |= OASLP;
				sleep((caddr_t)&tp->t_outq, TTOPRI);
			}
			splx(oldpri);
		}
		if (u.u_count >= (CLSIZE/2)) {
			register int n;
			register struct cblock *cp;

			if ((cp = getcf()) == NULL)
				break;
			n = min(u.u_count, cp->c_last);
			if (copyin(u.u_base, (caddr_t)cp->c_data, n)) {
				u.u_error = EFAULT;
				putcf(cp);
				break;
			}
			u.u_base += n;
			u.u_count -= n;
			cp->c_last = n;
			ttxput(tp, cp, n);
		} else {
			register int c;

			c = fubyte(u.u_base++);
			if (c<0) {
				u.u_error = EFAULT;
				break;
			}
			u.u_count--;
			ttxput(tp, c, 0);
		}
	}
out:
	oldpri = spltty();
	(*tp->t_proc)(tp, T_OUTPUT);
	splx(oldpri);
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 */
ttin(tp, code)
register struct tty *tp;
{
	register int c;
	register int flg;
	register char *cp;
	ushort nchar, nc;

	if (code == L_BREAK) {
		signal(tp->t_pgrp, SIGINT);
		ttyflush(tp, (FREAD|FWRITE));
		return;
	}
	nchar = tp->t_rbuf.c_size - tp->t_rbuf.c_count;
	if (nchar == 0)		/* No char left in rx buffer */
		return;

	/* reinit rx control block */
	tp->t_rbuf.c_count = tp->t_rbuf.c_size;

	/*
	 * if enabled, do input emapping.
	 * This is the channel mapping feature 
	 * for the MP adopted from xenix.
	 */

	cp = tp->t_rbuf.c_ptr;		/* pick up a char from rx buffer */
	if (tp->t_mstate) {
		nchar = emmapin(tp, cp, nchar);
		if (tp->t_merr && tp->t_lflag&ECHO) {
			ttxput(tp, '\7', 0);
			if (nchar == 0)
				(*tp->t_proc)(tp, T_OUTPUT);
		}
	}

	flg = tp->t_iflag;
	/* KMC does all but IXOFF */
	if (tp->t_state&EXTPROC)
		flg &= IXOFF;
	nc = nchar;

#if DEBUG
	if (debug_tty == 0x02)
		printf("ttin: nchar=%d, char=%x\n",nchar,*cp);
#endif /* DEBUG */

	if ((int)nc < cfreelist.c_size || (flg & (INLCR|IGNCR|ICRNL|IUCLC))) {
			/* must do per character processing */
		for (;nc--; cp++) {
			c = *cp;
			if (c == '\n' && flg&INLCR)
				*cp = c = (unsigned char) '\r';
			else if (c == '\r')
				if (flg&IGNCR)
					continue;
				else if (flg&ICRNL)
					*cp = c = (unsigned char) '\n';
			if (flg&IUCLC && 'A' <= c && c <= 'Z')
				c += 'a' - 'A';
			if (putc(c, &tp->t_rawq))
				continue;
			sysinfo.rawch++;
		}
		cp = tp->t_rbuf.c_ptr;
	} else {
		/* may do block processing */
		putcb(CMATCH((struct cblock *)cp), &tp->t_rawq);
		sysinfo.rawch += nc;
		/* allocate new rx buffer */
		if ((tp->t_rbuf.c_ptr = (char *) getcf()->c_data)
			== (char *) ((struct cblock *)NULL)->c_data) {
			tp->t_rbuf.c_ptr = NULL;
			return;
		}
		tp->t_rbuf.c_count = cfreelist.c_size;
		tp->t_rbuf.c_size = cfreelist.c_size;
	}


	if (tp->t_rawq.c_cc > TTXOHI) {
		if (flg&IXOFF && !(tp->t_state&TBLOCK))
			(*tp->t_proc)(tp, T_BLOCK);
		if (tp->t_rawq.c_cc > TTYHOG) {
			ttyflush(tp, FREAD);
			return;
		}
	}

	flg = lobyte(tp->t_lflag);
	if (flg) while (nchar--) {
		c = *cp++;
		if (flg&ISIG) {
#if DEBUG
			if (debug_tty == 0x02)
				printf("ttin(): ISIG set\n");
#endif /* DEBUG */
			if (c == tp->t_cc[VINTR]) {
#if DEBUG
				if (debug_tty == 0x02)
					printf("ttin(): intr!! pgrp=%x\n",
								tp->t_pgrp);
#endif /* DEBUG */
				signal(tp->t_pgrp, SIGINT);
				if (!(flg&NOFLSH))
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
			if (c == tp->t_cc[VQUIT]) {
#if DEBUG
				if (debug_tty == 0x02)
					printf("ttin(): quit!! pgrp=%x\n",
								tp->t_pgrp);
#endif /* DEBUG */
				signal(tp->t_pgrp, SIGQUIT);
				if (!(flg&NOFLSH))
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
			if (c == tp->t_cc[VSWTCH]) {
#if DEBUG
				if (debug_tty == 0x02)
					printf("ttin(): quit!! pgrp=%x\n",
								tp->t_pgrp);
#endif /* DEBUG */
				if (!(flg&NOFLSH))
					ttyflush(tp, (FREAD|FWRITE));
				(*tp->t_proc)(tp, T_SWTCH);
				continue;
			}
		}
		if (flg&ICANON) {
			if (c == '\n') {
				if (flg&ECHONL)
					flg |= ECHO;
				tp->t_delct++;
			} else if (c == tp->t_cc[VEOL] || c == tp->t_cc[VEOL2])
				tp->t_delct++;
			if (!(tp->t_state&CLESC)) {
				if (c == '\\')
					tp->t_state |= CLESC;
				if (c == tp->t_cc[VERASE] && flg&ECHOE) {
					if (flg&ECHO)
						ttxput(tp, '\b', 0);
					flg |= ECHO;
					ttxput(tp, ' ', 0);
					c = (unsigned char) '\b';
				} else if (c == tp->t_cc[VKILL] && flg&ECHOK) {
					if (flg&ECHO)
						ttxput(tp, c, 0);
					flg |= ECHO;
					c = (unsigned char) '\n';
				} else if (c == tp->t_cc[VEOF]) {
					flg &= ~ECHO;
					tp->t_delct++;
				}
			} else {
				if (c != '\\' || (flg&XCASE))
					tp->t_state &= ~CLESC;
			}
		}
		if (flg&ECHO) {
			ttxput(tp, c, 0);
			(*tp->t_proc)(tp, T_OUTPUT);
		}
	}
	if (!(flg&ICANON)) {
		tp->t_state &= ~RTO;
		if (tp->t_rawq.c_cc >= (int)tp->t_cc[VMIN])
			tp->t_delct = 1;
		else if (tp->t_cc[VTIME]) {
			if (!(tp->t_state&TACT))
				tttimeo(tp);
		}
	}
	if (tp->t_delct && (tp->t_state&IASLP)) {
		tp->t_state &= ~IASLP;
		wakeup((caddr_t)&tp->t_rawq);
	}
}

/*
 * Put character(s) on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the base level for output, and from
 * interrupt level for echoing.
 * The QESC char and the char following it are handled exceptionally
 * when doing post processing on characters.
 * If a char in the buffer is a QESC char then it is put on the outq.
 * The following char designates the number of timeout ticks to be done
 * in ttout() if it is greater than QESC itself.  If mapping flag is set
 * the timeout char is mapped prior to going on outq, otherwise it is just
 * put on the outq without mapping.
 * When the character following QESC is not greater than QESC, it will be
 * acted on like all other chars.
 */

union ucp_union {
	ushort	ch;
	struct cblock *ptr;
} ;

ttxput(tp, ucp, ncode)
register struct tty *tp;
union ucp_union	ucp;
{
	register int c, c2;
	register int flg;
	register unsigned char *cp;
	register char *colp;
	int ctype;
	int cs;
	struct cblock *scf;
	emcp_t ecp;
	int nc;

	if (tp->t_state&EXTPROC) {
		if (tp->t_lflag&XCASE)
			flg = OPOST;
		else
			flg = 0;
	} else
		flg = tp->t_oflag;

	if (ncode == 0) {
		ncode++;
		/* char is in least significant byte of first param */
		cp = (unsigned char *)&ucp.ch;
		scf = NULL;
		/*
		 * If no post processing needed on the ouput character,
		 * and if mapping is required, simply put it on the outq.  
		 * Otherwise, do output mapping on the char before placing
		 * it on the outq.
		 */
		if (!(flg&OPOST)) {
			if (!tp->t_mstate) {
				sysinfo.outch++;
				putc(ucp.ch, &tp->t_outq);
			} else {	/* must do output emapping.	*/
					/* mapping flag is set.		*/
				ecp = emmapout(tp, ucp.ch, &nc);
				while (--nc >= 0) {
					sysinfo.outch++;
					putc(*ecp++, &tp->t_outq);
				}
			}
			return;
		}
	} else {
		cp = (unsigned char *)&ucp.ptr->c_data[ucp.ptr->c_first];
		scf = ucp.ptr;
		if (!(flg&OPOST)) {
			if (!tp->t_mstate) {
				sysinfo.outch += ncode;
				putcb(ucp.ptr, &tp->t_outq);
			} else {	/* must do output emapping on buffer */
					/* mapping flag is set.		*/
				while (--ncode >= 0) {
					ecp = emmapout(tp, *cp++, &nc);
					while (--nc >= 0) {
						sysinfo.outch++;
						putc(*ecp++, &tp->t_outq);
					}
				}
				putcf(scf);
			}
			return;
		}
	}

	while (--ncode >= 0) {
	   c = *cp++;
	   /*
	    * The following code handles the QESC special case.
	    * If the char is a QESC then the intention is to send 
	    * a QESC, which we do by sending 2 QESCs in a row if no.
	    * mapping is required.  If mapping is needed on output chars
	    * then we map QESC like all other chars.
	    */
	   if (c == QESC && !(tp->t_state&EXTPROC)) {
		if (!tp->t_mstate) {
			/* two QESC in a row means QESC itself */
			putc(QESC, &tp->t_outq);
	   		putc(QESC, &tp->t_outq);
		}
		else {
			/* map the QESC char since flag is set */
			ecp = emmapout(tp, c, &nc);
			while (--nc >= 0) {
				sysinfo.outch++;
				putc(*ecp++, &tp->t_outq);
			}
		}
		continue;
	    }

	   if (tp->t_mstate)	/* must do output emapping */
		    ecp = emmapout(tp, c, &nc);
	   else
		    nc = 1;	/* One char to deal with when no mapping */

	   while (--nc >= 0) {
		if (tp->t_mstate)
			c = *ecp++;
		/*
		 * Generate escapes for upper-case-only terminals.
		 */
		if (tp->t_lflag&XCASE) {
			colp = "({)}!|^~'`\\\0";
			while (*colp++)
				if (c == *colp++) {
					putc('\\', &tp->t_outq);
					c = colp[-2];
					break;
				}
			if ('A' <= c && c <= 'Z')
				putc('\\', &tp->t_outq);
		}
		if (flg&OLCUC && 'a' <= c && c <= 'z')
			c += 'A' - 'a';
		cs = c;
		/*
		 * Calculate delays.
		 * The numbers here represent clock ticks
		 * and are not necessarily optimal for all terminals.
		 * The delays are indicated by characters above 0200.
		 */
		ctype = partab[c];
		colp = &tp->t_col;
		c = 0;
		switch (ctype&077) {

		case 0:	/* ordinary */
			(*colp)++;

		case 1:	/* non-printing */
			break;

		case 2:	/* backspace */
			if (flg&BSDLY)
				c = 2;
			if (*colp)
				(*colp)--;
			break;

		case 3:	/* line feed */
			if (flg&ONLRET)
				goto cr;
			if (flg&ONLCR) {
				if (!(flg&ONOCR && *colp==0)) {
					sysinfo.outch++;
					putc('\r', &tp->t_outq);
				}
				goto cr;
			}
		nl:
			if (flg&NLDLY)
				c = 5;
			break;

		case 4:	/* tab */
			c = 8 - ((*colp)&07);
			*colp += c;
			ctype = flg&TABDLY;
			if (ctype == TAB0) {
				c = 0;
			} else if (ctype == TAB1) {
				if (c < 5)
					c = 0;
			} else if (ctype == TAB2) {
				c = 2;
			} else if (ctype == TAB3) {
				sysinfo.outch += c;
				do
					putc(' ', &tp->t_outq);
				while (--c);
				continue;
			}
			break;

		case 5:	/* vertical tab */
			if (flg&VTDLY)
				c = 0177;
			break;

		case 6:	/* carriage return */
			if (flg&OCRNL) {
				cs = '\n';
				goto nl;
			}
			if (flg&ONOCR && *colp == 0)
				continue;
		cr:
			ctype = flg&CRDLY;
			if (ctype == CR1) {
				if (*colp)
					c = max((*colp>>4) + 3, 6);
			} else if (ctype == CR2) {
				c = 5;
			} else if (ctype == CR3) {
				c = 9;
			}
			*colp = 0;
			break;

		case 7:	/* form feed */
			if (flg&FFDLY)
				c = 0177;
			break;
		}
		sysinfo.outch++;
		putc(cs, &tp->t_outq);
		/*
		 * Do the delay work if no external process
		 * is supposed to handle it.
		 */
		if (!(tp->t_state&EXTPROC)) {
			if (c) {
				if ((c < 32) && flg&OFILL) {
					if (flg&OFDEL)
						cs = 0177;
					else
						cs = 0;
					putc(cs, &tp->t_outq);
					if (c > 3)
						putc(cs, &tp->t_outq);
				} else {
					/*
				 	 * Delay calculated based on oflag bits.
				 	 * put a QESC and the delay ticks
				 	 * on the outq.  ttout() will handle
					 * the delaying.
				 	 */
					putc(QESC, &tp->t_outq);
					putc(c|0200, &tp->t_outq);
				}

			}
		}
	   }
	}
	if (scf != NULL)
		putcf(scf);
}

/*
 * Get next packet from output queue.
 * Called from xmit interrupt complete.
 */

ttout(tp)
register struct tty *tp;
{
	register struct ccblock *tbuf;
	register c;
	register char *cptr;
	register retval;
	extern ttrstrt();

	if (tp->t_state&TTIOW && tp->t_outq.c_cc==0) {
		tp->t_state &= ~TTIOW;
		wakeup((caddr_t)&tp->t_oflag);
	}
delay:
	tbuf = &tp->t_tbuf;
	if (tp->t_schar) {	/* some delay value given */
		if (tbuf->c_ptr) {
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
			tbuf->c_ptr = NULL;
		}
		tp->t_state |= TIMEOUT;
		timeout((void (*)())ttrstrt, (caddr_t)tp, (tp->t_schar&0177)+6);
		tp->t_schar = 0;
		return(0);
	}
	retval = 0;

/*
 * All of these flags signify some delay
 * and if set some delaying is involved.
 */
#define	ADELAY	(NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY)

	if ((tp->t_state&EXTPROC) || (!(tp->t_oflag&OPOST))
		|| ((tp->t_oflag & ADELAY))) {

		/* There is no delaying, just worry abount the char byte. */
		if (tbuf->c_ptr)
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
		if ((tbuf->c_ptr = (char *)getcb(&tp->t_outq)) == NULL)
			return(0);
		tbuf->c_count = ((struct cblock *)tbuf->c_ptr)->c_last -
				((struct cblock *)tbuf->c_ptr)->c_first;
		tbuf->c_size = tbuf->c_count;
		tbuf->c_ptr = (char *) &((struct cblock *)tbuf->c_ptr)->c_data
				[((struct cblock *)tbuf->c_ptr)->c_first];
		retval = CPRES;
	} else {		/* watch for timing	*/
		if (tbuf->c_ptr == NULL) {
			if ((tbuf->c_ptr = (char *) getcf()->c_data)
				== (char *) ((struct cblock *)NULL)->c_data) {
				tbuf->c_ptr = NULL;
				return(0);	/* Add restart? */
			}
		}
		tbuf->c_count = 0;
		cptr = tbuf->c_ptr;
		while ((c=getc(&tp->t_outq)) >= 0) {
			/*
			 * QESC designates that the next char,
			 * if greater than QESC itself, is the time 
			 * out tick.  If two QESC following each other,
			 * one is passed down.
			 */
			if (c == QESC) {
				if ((c = getc(&tp->t_outq)) < 0)
					break;
				if (c > QESC) {
					/*
					 * Hi order byte of l_lflag used to
					 * contains the timeout value. Now we
					 * have a dedicated tty member for it.
					 */
					tp->t_schar = c;
					if (!retval)
						goto delay;
					break;
				}
			}
			retval = CPRES;
			*cptr++ = c;
			tbuf->c_count++;
			if ((int)tbuf->c_count >= cfreelist.c_size)
				break;
		}
		tbuf->c_size = tbuf->c_count;
	}

out:
	if (tp->t_state&OASLP &&
		tp->t_outq.c_cc<=ttlowat[tp->t_cflag&CBAUD]) {
		tp->t_state &= ~OASLP;
		wakeup((caddr_t)&tp->t_outq);
	}
	return(retval);
}

tttimeo(tp)
register struct tty *tp;
{
	tp->t_state &= ~TACT;
	if (tp->t_lflag&ICANON || tp->t_cc[VTIME] == 0)
		return;
	if (tp->t_rawq.c_cc == 0 && tp->t_cc[VMIN])
		return;
	if (tp->t_state&RTO) {
		tp->t_delct = 1;
		if (tp->t_state&IASLP) {
			tp->t_state &= ~IASLP;
			wakeup((caddr_t)&tp->t_rawq);
		}
	} else {
		tp->t_state |= RTO|TACT;
		timeout((void (*)())tttimeo, (caddr_t)tp, tp->t_cc[VTIME]*(HZ/10));
	}
}

/*
 * I/O control interface
 */
ttioctl(tp, cmd, arg, mode)
register struct tty *tp;
{
	ushort	chg;
	register int	oldpri;

	switch (cmd) {
	case LDOPEN:
		if (tp->t_rbuf.c_ptr == NULL) {
			register struct cblock	*cblock;

			/* allocate RX buffer */
			while ((cblock = getcf()) == (struct cblock *) NULL) {
				cfreelist.c_flag = 1;
				sleep((caddr_t)&cfreelist, TTOPRI);
			}
			tp->t_rbuf.c_ptr = (char *) cblock->c_data;
			tp->t_rbuf.c_count = cfreelist.c_size;
			tp->t_rbuf.c_size  = cfreelist.c_size;
			(*tp->t_proc)(tp, T_INPUT);
		}
		break;

	case LDCLOSE:
		oldpri = spltty();
		(*tp->t_proc)(tp, T_RESUME);
		splx(oldpri);
		ttywait(tp);
		ttyflush(tp, FREAD);
		if (tp->t_tbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_tbuf.c_ptr));
			tp->t_tbuf.c_ptr = NULL;
		}
		if (tp->t_rbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_rbuf.c_ptr));
			tp->t_rbuf.c_ptr = NULL;
		}
		break;

	case LDCHG:
		chg = tp->t_lflag^arg;
		if (!(chg&ICANON))
			break;
		oldpri = spltty();
		if (tp->t_canq.c_cc) {
			if (tp->t_rawq.c_cc) {
				tp->t_canq.c_cc += tp->t_rawq.c_cc;
				tp->t_canq.c_cl->c_next = tp->t_rawq.c_cf;
				tp->t_canq.c_cl = tp->t_rawq.c_cl;
			}
			tp->t_rawq = tp->t_canq;
			tp->t_canq = ttnulq;
		}
		tp->t_delct = tp->t_rawq.c_cc;
		splx(oldpri);
		break;

	case LDSMAP:
		emsetmap(tp, arg);
		break;

	case LDGMAP:
		emgetmap(tp, arg);
		break;

	case LDNMAP:
		emunmap(tp);
		break;

	default:
		break;
	}
}

