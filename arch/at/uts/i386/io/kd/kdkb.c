/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/kd/kdkb.c	1.2.1.1"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/inline.h>
#include <sys/termios.h>
#include <sys/stream.h>
#include <sys/strtty.h>
#include <sys/stropts.h>
#include <sys/kd.h>
#include <sys/proc.h>
#include <sys/xque.h>
#include <sys/ws/ws.h>
#include <sys/ws/8042.h>
#include <sys/kb.h>
#include <sys/cmn_err.h>
#include <sys/sysmsg.h>
#include <sys/ddi.h>

extern wstation_t	Kdws;
extern channel_t	Kd0chan;
extern struct smsg_flags	smsg_flags;

void	kdkb_cmd();
channel_t	*ws_activechan();

/*
 *
 */

kdkb_init(wsp)
wstation_t	*wsp;
{
	kbstate_t	*kbp;
	channel_t	*chp;
	unchar	cb;

	chp = ws_activechan(wsp);
	if (chp == (channel_t *)NULL)
		chp = &Kd0chan;
	kbp = &chp->ch_kbstate;
	if (inb(KB_STAT) & KB_OUTBF) /* clear output buffer */
		(void)inb(KB_OUT);
	kdkb_type(wsp);		/* find out keyboard type */
	SEND2KBD(KB_ICMD, KB_RCB);
	while (!(inb(KB_STAT) & KB_OUTBF))
		;
	cb = inb(KB_OUT); /* get command byte */
	cb &= ~KB_DISAB; /* clear disable keyboard flag */
	cb |= KB_EOBFI; /* set interrupt on output buffer full flag */
	SEND2KBD(KB_ICMD, KB_WCB);
	while (inb(KB_STAT) & KB_INBF) /* wait input buffer clear */
		;
	outb(KB_IDAT, cb);
	kbp->kb_state = 0;
	(void) i8042_aux_port(); /* ign ret val -- this is for init */
	kdkb_cmd(LED_WARN);
	kdkb_cmd(TYPE_WARN);
}

/*
 * call with interrupts off only!! (Since this is called at init time,
 * we cannot use i8042* interface since it spl's.
 */

kdkb_type(wsp)
wstation_t	*wsp;
{
	int	cnt;
	unchar	byt;

	wsp->w_kbtype = KB_OTHER;
	SEND2KBD(KB_IDAT, KB_READID);
	while (!(inb(KB_STAT) & KB_OUTBF))
		;		/* wait for ACK byte */
	inb(KB_OUT);		/* and discard it */
	/* wait for up to about a quarter-second for response */
	for (cnt = 0; cnt < 20000 && !(inb(KB_STAT) & KB_OUTBF); cnt++)
		tenmicrosec();
	if (!(inb(KB_STAT) & KB_OUTBF))
		wsp->w_kbtype = KB_84;	/* no response indicates 84-key */
	else if (inb(KB_OUT) == 0xAB) { /* first byte of 101-key response */
		/* wait for up to about a quarter-second for next byte */
		for (cnt = 0; cnt < 20000 && !(inb(KB_STAT) & KB_OUTBF); cnt++)
			tenmicrosec();
		if ((byt = inb(KB_OUT)) == 0x41 || byt == 0x83 || byt == 0x85)
			/* these are apparently all valid 2nd bytes */
			wsp->w_kbtype = KB_101;
	}
	SEND2KBD(KB_ICMD, KB_ENAB);
}

int
kdkb_resend(cmd,ack)
unchar cmd,ack;
{
	int cnt = 10;
	int rv = 0;
	static int printwarn = 1;

#ifdef DEBUG
	cmn_err(CE_NOTE,"kdkb_resend: ack is 0x%x",ack);
#endif
	while (cnt --) {
		if (ack == 0xfe) {
			rv = i8042_send_cmd(cmd, P8042_TO_KBD,&ack,1);
			continue;
		}
		else if (ack == KB_ACK) {
			return (1);
	        } else 
			break;
	}
	cnt = 10;
	cmd = 0xf4;
	while (cnt --) {
		rv = i8042_send_cmd(cmd, P8042_TO_KBD,&ack,1);
		if (ack == KB_ACK) 
			return (0);
	}
#ifdef DEBUG
	cmn_err(CE_NOTE,"kdkb_resend: did not enable keyboard");
#endif
	if (!printwarn && smsg_flags.acef)
		return (0);
	cmn_err(CE_WARN,"Integral console keyboard not found. If you are using");
	cmn_err(CE_CONT,"the integral console, check the keyboard connection.\n");
	printwarn = 0;
	return (0);
}

/*
 * Send command to keyboard. Assumed to only be called when spl's are valid
 */

void
kdkb_cmd(cmd)
register unchar	cmd;
{
	register int	cnt,rv;
	register unsigned char	ledstat;
	unchar	ack;
	channel_t	*chp;
	kbstate_t	*kbp;

	chp = ws_activechan(&Kdws);
	if (chp == (channel_t *) NULL)
		chp = &Kd0chan;
	kbp = &chp->ch_kbstate;
	i8042_acquire();
	rv = i8042_send_cmd(cmd, P8042_TO_KBD,&ack,1);
#ifdef DEBUG
	cmn_err(CE_NOTE,"!rv was %x",rv);
#endif
	if (ack != KB_ACK) {
#ifdef DEBUG
		cmn_err(CE_WARN,"kdkb_cmd: unknown cmd %x ack %x",cmd,ack);
#endif
		if (!kdkb_resend(cmd,ack)) {
			i8042_release();
			return;
		}
	} 

	switch (cmd) {
	case LED_WARN:	/* send led status next */
		ledstat = 0;
		if (kbp->kb_state & CAPS_LOCK)
			ledstat |= LED_CAP;
		if (kbp->kb_state & NUM_LOCK)
			ledstat |= LED_NUM;
		if (kbp->kb_state & SCROLL_LOCK)
			ledstat |= LED_SCR;
		i8042_send_cmd(ledstat, P8042_TO_KBD,&ack,1);
		if (ack != KB_ACK) {
#ifdef DEBUG
			cmn_err(CE_WARN,"kdkb_cmd: LED_WARN, no ack from kbd");
#endif
			if (!kdkb_resend(cmd,ack)) {
				i8042_release();
				return;
			}
		}
		break;
	case TYPE_WARN:	/* send typematic */
		i8042_send_cmd(TYPE_VALS, P8042_TO_KBD,&ack,1);
		if (ack != KB_ACK) {
#ifdef DEBUG
			cmn_err(CE_WARN,"kdkb_cmd: TYPE_WARN, no ack from kbd");
#endif
			if (!kdkb_resend(cmd,ack)) {
				i8042_release();
				return;
			}
		}
		break;
	case KB_ENAB: /* waiting for keyboard ack */
		i8042_program(P8042_KBDENAB);
		break;
	default:
		cmn_err(CE_WARN,"kdkb_cmd: illegal kbd command %x",cmd);
		break;
	}
	i8042_release();
}

/*
 *
 */

kdkb_cktone()
{
	unchar regval;

	if (Kdws.w_ticks-- > 1)
		timeout(kdkb_cktone, (caddr_t)0, BELLLEN);
	else {	/* turn tone off */
		regval = (inb(TONE_CTL) & ~TONE_ON);
		outb(TONE_CTL, regval);
	}
}

/*
 *
 */

kdkb_tone()
{
	int oldpri;
	unchar regval;

	oldpri = splhi();
	if (!Kdws.w_ticks) {
		Kdws.w_ticks = BELLCNT;
		splx(oldpri);
		outb(TIMERCR, T_CTLWORD);
		outb(TIMER2, NORMBELL & 0xFF);
		outb(TIMER2, NORMBELL >> 8);
		/* turn tone generation on */
		regval = (inb(TONE_CTL) | TONE_ON);
		outb (TONE_CTL, regval);
		/* go away and let tone ring a while */
		timeout (kdkb_cktone, (caddr_t)0, BELLLEN);
	} else {
		splx(oldpri);
		Kdws.w_ticks = BELLCNT; /* make it ring BELLCNT longer */
	}
}

/*
 *
 */

kdkb_sound(freq)
int	freq;
{
	unchar	regval;

	if (freq) {	/* turn sound on? */
		outb(TIMERCR, T_CTLWORD);
		outb(TIMER2, (freq & 0xff));
		outb(TIMER2, ((freq >> 8) & 0xff));
		regval = (inb(TONE_CTL) | TONE_ON);
	} else
		regval = (inb(TONE_CTL) & ~TONE_ON);
	outb(TONE_CTL, regval);
}

/*
 *
 */

kdkb_toneoff()
{
	unchar	regval;

	Kdws.w_tone = 0;
	regval = (inb(TONE_CTL) & ~TONE_ON);
	outb(TONE_CTL, regval);
	wakeup((caddr_t) &Kdws.w_tone);
}

/*
 *
 */

kdkb_mktone(chp, arg)
channel_t	*chp;
int	arg;
{
	ushort	freq, length;
	int	tval, oldpri;
	unchar	regval;

	freq = (ushort)((long)arg & 0xffff);
	length = (ushort)(((long)arg >> 16) & 0xffff);
	if (!freq || !(tval = ((ulong)(length * HZ) / 1000L)))
		return;
	oldpri = splstr();
	while (Kdws.w_tone)
		sleep((caddr_t)&Kdws.w_tone, TTOPRI);
	Kdws.w_tone = 1;
	splx(oldpri);
	/* set up timer mode and load initial value */
	outb(TIMERCR, T_CTLWORD);
	outb(TIMER2, freq & 0xff);
	outb(TIMER2, (freq >> 8) & 0xff);
	/* turn tone generator on */
	regval = (inb(TONE_CTL) | TONE_ON);
	outb(TONE_CTL, regval);
	timeout(kdkb_toneoff, 0, tval);
}

/*
 *
 */

kdkb_setled(chp,kbp,led)
channel_t *chp;
kbstate_t	*kbp;
unchar	led;
{
	if (led & LED_CAP)
		kbp->kb_state |= CAPS_LOCK;
	else
		kbp->kb_state &= ~CAPS_LOCK;
	if (led & LED_NUM)
		kbp->kb_state |= NUM_LOCK;
	else
		kbp->kb_state &= ~NUM_LOCK;
	if (led & LED_SCR)
		kbp->kb_state |= SCROLL_LOCK;
	else
		kbp->kb_state &= ~SCROLL_LOCK;
	if (chp == ws_activechan(&Kdws))
		kdkb_cmd(LED_WARN);
}

kdkb_scrl_lock(chp)
channel_t	*chp;
{
	int oldpri;

	kdkb_cmd(LED_WARN);
}

/*
 *
 */

kdkb_locked(ch, kbrk)
ushort	ch;
unchar	kbrk;
{
	int	locked = (Kdws.w_flags & WS_LOCKED) ? 1 : 0;

	if (kbrk)
		return(locked);
	if (Kdws.w_flags & WS_LOCKED) {	/* we are locked, do we unlock? */
		switch (Kdws.w_lkstate) {
		case 0:	/* look for ESC */
			if (ch == '\033')
				Kdws.w_lkstate++;
			else
				Kdws.w_lkstate = 0;
			break;
		case 1:	/* look for '[' */
			if (ch == '[')
				Kdws.w_lkstate++;
			else
				Kdws.w_lkstate = 0;
			break;
		case 2:	/* look for '2' */
			if (ch == '2')
				Kdws.w_lkstate++;
			else
				Kdws.w_lkstate = 0;
			break;
		case 3:	/* look for 'l' */
			if (ch == 'l')
				Kdws.w_flags &= ~WS_LOCKED;
			Kdws.w_lkstate = 0;
			break;
		}
	} else {	/* we are unlocked, do we lock? */
		switch (Kdws.w_lkstate) {
		case 0:	/* look for ESC */
			if (ch == '\033')
				Kdws.w_lkstate++;
			else
				Kdws.w_lkstate = 0;
			break;
		case 1:	/* look for '[' */
			if (ch == '[')
				Kdws.w_lkstate++;
			else
				Kdws.w_lkstate = 0;
			break;
		case 2:	/* look for '2' */
			if (ch == '2')
				Kdws.w_lkstate++;
			else
				Kdws.w_lkstate = 0;
			break;
		case 3:	/* look for 'h' */
			if (ch == 'h')
				Kdws.w_flags |= WS_LOCKED;
			Kdws.w_lkstate = 0;
			break;
		}
	}
	return(locked);
}

/*
 *
 */

kdkb_keyclick(ch)
register ushort	ch;
{
	register unchar	tmp;
	register int	cnt;

	if (Kdws.w_flags & WS_KEYCLICK) {
		tmp = (inb(TONE_CTL) | TONE_ON);	/* start click */
		outb(TONE_CTL, tmp);
		for (cnt = 0; cnt < 0xff; cnt++)
			;
		tmp = (inb(TONE_CTL) & ~TONE_ON);	/* end click */
		outb(TONE_CTL, tmp);
	}
}


/* this routine is called by the kdputchar to force an enable of
 * the keyboard. We do this to avoid the spls and flags of the
 * i8042 interface
 */

void
kdkb_force_enable()
{
	SEND2KBD(KB_ICMD, KB_ENAB);
}
