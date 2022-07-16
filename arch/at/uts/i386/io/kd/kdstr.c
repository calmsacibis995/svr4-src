/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/kd/kdstr.c	1.2.2.3"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/inline.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/sysinfo.h"
#include "sys/pic.h"
#include "sys/cmn_err.h"
#include "sys/vt.h"
#include "sys/at_ansi.h"
#include "sys/kd.h"
#include "sys/tss.h"
#include "sys/ioctl.h"
#include "sys/termios.h"
#include "sys/stream.h"
#include "sys/strtty.h"
#include "sys/stropts.h"
#include "sys/xque.h"
#include "sys/ws/ws.h"
#include "sys/cred.h"
#include "sys/ws/chan.h"
#include "sys/ws/tcl.h"
#include "sys/jioctl.h"
#include "sys/kmem.h"
#include "sys/kb.h"
#include "sys/vid.h"
#include "sys/xdebug.h"
#include "sys/user.h"

#ifdef	BLTCONS
#include "kd_btc.h"
#endif

#include "sys/ddi.h"

int	kdopen(), kdclose(), kdwput(), kdwsrv();

struct module_info
	kd_info = { 42, "kd", 0, 32, 256, 128 };

static struct qinit
	kd_rinit = { NULL, NULL, kdopen, kdclose, NULL, &kd_info, NULL };

static struct qinit
	kd_winit = { kdwput, kdwsrv, kdopen, kdclose, NULL, &kd_info, NULL };

struct streamtab
	kdinfo = { &kd_rinit, &kd_winit, NULL, NULL };

wstation_t	Kdws = {0};

struct kdptrs {
	channel_t	*k_chanpp[WS_MAXCHAN+1];
	charmap_t	*k_charpp[WS_MAXCHAN+1];
	ushort	*k_scrnpp[WS_MAXCHAN+1];
} Kdptrs = { 0 };

channel_t	Kd0chan = {0};

int kddevflag = 0; /* We are new-style SVR4.0 driver */

unchar	Kd0tabs[ANSI_MAXTAB];

extern struct font_info fontinfo[];
extern ushort kd_iotab[][MKDBASEIO];
extern struct b_param kd_inittab[];
extern struct reginfo kd_regtab[];
extern unchar kd_ramdactab[];
extern unchar	*egafont_p[5];
extern struct cgareginfo kd_cgaregtab[];
extern struct m6845init kd_cgainittab[];
extern struct m6845init kd_monoinittab[];
extern long kdmonitor[];
extern char kd_swmode[];
extern struct attrmask	kb_attrmask[];
extern int	nattrmsks;

extern int	ws_kbtime(),ws_toglchange();

extern channel_t	*ws_activechan(),
			*ws_getchan();
extern ushort	ws_scanchar();
extern void	ws_cmap_free(),
		ws_cmap_init(),
		ws_cmap_reset(),
		ws_scrn_free(),
		ws_scrn_reset(),
		ws_scrn_init(),
		ws_scrn_alloc(),
		tcl_handler(),
		kdkb_cmd(),
		kdkb_force_enable(),
		tcl_scrolllock();

extern	charmap_t	*ws_cmap_alloc();

extern int	kdv_stchar(),
		kdv_mvword(),
		kdv_shiftset(),
		kdv_undattrset(),
		kdkb_tone(),
		kdvt_rel_refuse(),
		kdvt_acq_refuse(),
		kdkb_scrl_lock(),
		kdvt_activate(),
		kdv_cursortype(),
		kdvm_unmapdisp();

extern unchar	kdkb_getled();

extern ushort	ws_shiftkey();

extern long	inrtnfirm;	/* rtnfirm should set this to non-zero */

#ifdef EVGA
extern  int evga_inited;
extern  int evga_mode;
extern  unchar saved_misc_out;
extern  struct at_disp_info  disp_info[];
#endif	/*EVGA*/

struct ext_graph kd_vdc800 = {0}; 	/* vdc800 hook structure */

int	kdclrscr(),
	kdsetbase(),
	kdsetcursor(),
	kdputchar(),
	kdgetchar(),
	kdshiftset(),
	kdtone();


/*
 *
 */

kdinit()
{
	if (Kdws.w_init)
		return;
	Kdws.w_init++;
	Kdws.w_stchar = kdv_stchar;
	Kdws.w_clrscr = kdclrscr;
	Kdws.w_setbase = kdsetbase;
	Kdws.w_activate = kdvt_activate;
	Kdws.w_setcursor = kdsetcursor;
	Kdws.w_active = 0;
	Kdws.w_bell = kdtone;
	Kdws.w_shiftset = kdshiftset;
	Kdws.w_undattr = kdv_undattrset;
	Kdws.w_rel_refuse = kdvt_rel_refuse;
	Kdws.w_acq_refuse = kdvt_acq_refuse;
	Kdws.w_unmapdisp = kdvm_unmapdisp;
	Kdws.w_ticks = 0;
	Kdws.w_mvword = kdv_mvword;
	Kdws.w_switchto = (channel_t *)NULL;
	Kdws.w_scrllck = kdkb_scrl_lock;
	Kdws.w_cursortype = kdv_cursortype;
	Kdws.w_wsid = 0; /* not used by KD, but initialize it anyway */
	Kdws.w_private = (caddr_t) NULL; /* not used by KD */
	Kdws.w_qp = (queue_t *)NULL;
	if ((Kdws.w_mp = allocb(4, BPRI_MED)) == (mblk_t *)NULL)
		cmn_err(CE_PANIC, "kdinit: no msg blocks");
	kdv_init(&Kd0chan);
	Kdws.w_chanpp = Kdptrs.k_chanpp;
	Kdws.w_scrbufpp = Kdptrs.k_scrnpp;
	Kd0chan.ch_tstate.t_tabsp = Kd0tabs;
	Kd0chan.ch_nextp = Kd0chan.ch_prevp = &Kd0chan;
	Kdws.w_chanpp[0] = &Kd0chan;
	ws_chinit(&Kdws, &Kd0chan, 0);
	ws_cmap_init(&Kdws,KM_NOSLEEP);
	ws_scrn_init(&Kdws,KM_NOSLEEP);
	Kd0chan.ch_charmap_p = ws_cmap_alloc(&Kdws,KM_NOSLEEP);
	ws_scrn_alloc(&Kdws,&Kd0chan);
	Kdptrs.k_charpp[0] = Kd0chan.ch_charmap_p;
	if (!(Kdws.w_scrbufpp[0] = (ushort *)kmem_alloc(sizeof(ushort) * KD_MAXSCRSIZE, KM_NOSLEEP)))
		cmn_err(CE_WARN, "kdintr: out of memory for screen");
	kdclrscr(&Kd0chan, Kd0chan.ch_tstate.t_origin, Kd0chan.ch_tstate.t_scrsz);
	Kdws.w_init++;
	inb(KB_IDAT);	/* read scan data - clear possible spurious data */
	drv_setparm(SYSRINT, 1);	/* reset keyboard interrupts */
}

/*
 *
 */

kdstart()
{
	int	oldpri;

	oldpri = splhi();
	kdkb_init(&Kdws);
	splx(oldpri);
}

/*
 *
 */

kdopen(qp, dev_p, flag, sflag, credp)
queue_t	*qp;
dev_t	*dev_p;
int	flag,
	sflag;
cred_t	*credp;
{
	int	indx, err;
	channel_t	*chp;
	unchar	*tabsp;
	ushort	*scrp;

	if (qp->q_ptr != (caddr_t) NULL) 
		return (EBUSY);
	indx = getminor(*dev_p);
	if (indx > WS_MAXCHAN)
		return (ENODEV);
	if ((chp = ws_getchan(&Kdws, indx)) == (channel_t *)NULL) {
		if (!(chp = (channel_t *)kmem_zalloc(sizeof(channel_t), KM_SLEEP))) {
			cmn_err(CE_WARN, "kdopen: out of memory");
			return(ENOMEM);
		}
		if (!(tabsp = (unchar *)kmem_alloc(ANSI_MAXTAB, KM_SLEEP))) {
			cmn_err(CE_WARN, "kdopen: out of memory for tabs");
			return(ENOMEM);
		}
		Kdws.w_nchan ++;	/* increment count of configured channels */
		Kdws.w_chanpp[indx] = chp;
		chp->ch_tstate.t_tabsp = tabsp;
		ws_chinit(&Kdws, chp, indx);
		chp->ch_charmap_p = ws_cmap_alloc(&Kdws,KM_SLEEP);
		ws_scrn_alloc(&Kdws,chp);
		Kdptrs.k_charpp[indx] = chp->ch_charmap_p;
	}
	qp->q_ptr = (caddr_t)chp;
	WR(qp)->q_ptr = qp->q_ptr;
	chp->ch_qp = qp;
	if (!Kdws.w_qp)
		Kdws.w_qp = chp->ch_qp;
	return(0);
}

/*
 * Close of /dev/kd/* (when the chanmux is being disassembled),
 * and /dev/vtmon
 */

kdclose(qp, credp)
queue_t	*qp;
cred_t	*credp;
{
	channel_t	*chp = (channel_t *)qp->q_ptr;
	int	indx, oldpri;

	if (!chp->ch_id)	/* channel 0 never "really" closes */
		return;
	if (Kdws.w_qp == chp->ch_qp) {
		oldpri = splhi();
		if (Kdws.w_timeid) {
			untimeout(Kdws.w_timeid);
			Kdws.w_timeid = 0;
		}
		Kdws.w_qp = (queue_t *)NULL;
		splx(oldpri);
	}
	flushq(WR(qp),FLUSHALL);
	chp->ch_qp = (queue_t *)NULL;
	qp->q_ptr = WR(qp)->q_ptr = (caddr_t)NULL;
	oldpri = splstr();
	indx = chp->ch_id;
	if (Kdws.w_scrbufpp[indx]) {
		kmem_free(Kdws.w_scrbufpp[indx], sizeof(ushort) * KD_MAXSCRSIZE);
	}
	Kdws.w_scrbufpp[indx] = (ushort *)NULL;
	kmem_free(chp->ch_tstate.t_tabsp, ANSI_MAXTAB);
	ws_cmap_free(&Kdws,chp->ch_charmap_p);
	ws_scrn_free(&Kdws,chp);
	kmem_free(Kdws.w_chanpp[indx], sizeof(channel_t));
	Kdws.w_chanpp[indx] = (channel_t *)NULL;
	Kdws.w_nchan --; /* decrement count of configured channels */
	splx(oldpri);
}

/*
 *
 */

kdwput(qp, mp)
queue_t	*qp;
mblk_t	*mp;
{
	putq(qp, mp);
}

kdwsrv(qp)
queue_t	*qp;
{
	channel_t	*chp = (channel_t *)qp->q_ptr;
	mblk_t	*mp;
	termstate_t	*tsp;

	tsp = &chp->ch_tstate;
	while ((mp = getq(qp))) {
		switch (mp->b_datap->db_type) {
		case M_PROTO:
		case M_PCPROTO:
			if ((mp->b_wptr - mp->b_rptr) != sizeof(ch_proto_t)) {
				cmn_err(CE_NOTE, "kdwput: bad M_PROTO or M_PCPROTO msg");
				freemsg(mp);
				break;
			}
			kdproto(qp, mp);
			continue;
		case M_DATA:
			/* writes as if to /dev/null when in KD_GRAPHICS mode */
			if (chp->ch_dmode != KD_GRAPHICS) {
				while (mp->b_rptr != mp->b_wptr)
					tcl_norm(&Kdws, chp, tsp, *mp->b_rptr++);
			}
			freemsg(mp);
			break;
		case M_CTL:
			ws_mctlmsg(qp, mp);
			continue;
		case M_IOCTL:
			kdmioctlmsg(qp, mp);
			continue;
		case M_IOCDATA:
			kdmiocdatamsg(qp, mp);
			continue;
		case M_STARTI:
		case M_STOPI:
		case M_READ:	/* ignore, no buffered data */
			freemsg(mp);
			continue;
		case M_FLUSH:
			*mp->b_rptr &= ~FLUSHW;
			if (*mp->b_rptr & FLUSHR)
				qreply(qp, mp);
			else
				freemsg(mp);
			continue;
		default:
			cmn_err(CE_NOTE, "kdwput: bad msg %x", mp->b_datap->db_type);
		}
		qenable(qp);
		return;
	}
}

/*
 *
 */

kdmioctlmsg(qp, mp)
queue_t	*qp;
mblk_t	*mp;
{
	struct iocblk	*iocp;
	channel_t	*chp = (channel_t *)qp->q_ptr;
	struct strtty	*sttyp;
	mblk_t	*tmp;
	ch_proto_t	*protop;
/* XXX
	if (mp->b_wptr - mp->b_rptr != sizeof(struct iocblk)) {
		cmn_err(CE_NOTE, "!kdmioctlmsg: bad M_IOCTL msg");
		return;
	}
*/
	sttyp = (struct strtty *)&chp->ch_strtty;
	iocp = (struct iocblk *)mp->b_rptr;
	switch (iocp->ioc_cmd) {

	case KIOCINFO:
		iocp->ioc_rval = ('k' << 8) | 'd';
		ws_iocack(qp, mp, iocp);
		break;

	case KDGKBTYPE:
		if (!(tmp = allocb(sizeof(unchar), BPRI_MED))) {
			cmn_err(CE_NOTE, "!kdmioctlmsg: can't get msg for reply to KDGKBTYPE");
			ws_iocnack(qp, mp, iocp, ENOMEM);
			break;
		}
		*(unchar *)tmp->b_rptr = Kdws.w_kbtype;
		tmp->b_wptr += sizeof(unchar);
		ws_copyout(qp, mp, tmp, sizeof(unchar));
		break;

	case KDGETLED:
		if (!(tmp = allocb(sizeof(unchar), BPRI_MED))) {
			cmn_err(CE_NOTE, "!kdmioctlmsg: can't get msg for reply to KDGETLED");
			ws_iocnack(qp, mp, iocp, ENOMEM);
			break;
		}
		*(unchar *)tmp->b_rptr = ws_getled(&chp->ch_kbstate);
		tmp->b_wptr += sizeof(unchar);
		ws_copyout(qp, mp, tmp, sizeof(unchar));
		break;

	case KDSETLED:
		if (Kdws.w_timeid) {
			untimeout(Kdws.w_timeid);
			Kdws.w_timeid = 0;
		}
		if (!(tmp = allocb(sizeof(ch_proto_t), BPRI_HI))) {
			ws_iocnack(qp, mp, iocp, ENOMEM);
			break;
		}
		kdkb_setled(chp, &chp->ch_kbstate, *(unchar *)mp->b_cont->b_rptr);
		tmp->b_datap->db_type = M_PROTO;
		tmp->b_wptr += sizeof(ch_proto_t);
		protop = (ch_proto_t *)tmp->b_rptr;
		protop->chp_type = CH_CTL;
		protop->chp_stype = CH_CHR;
		protop->chp_stype_cmd = CH_LEDSTATE;
		protop->chp_stype_arg = (chp->ch_kbstate.kb_state & ~NONTOGGLES);
		ws_iocack(qp, mp, iocp);
		ws_kbtime(&Kdws);
		qreply(qp, tmp);
		break;

	case TCSETSW:
	case TCSETSF:
	case TCSETS: {
		struct termios	*tsp;

		if (!mp->b_cont) {
			ws_iocnack(qp, mp, iocp, EINVAL);
			break;
		}
		tsp = (struct termios *)mp->b_cont->b_rptr;
		sttyp->t_cflag = tsp->c_cflag;
		sttyp->t_iflag = tsp->c_iflag;
		ws_iocack(qp, mp, iocp);
		break;
	}
	case TCSETAW:
	case TCSETAF:
	case TCSETA: {
		struct termio	*tp;

		if (!mp->b_cont) {
			ws_iocnack(qp, mp, iocp, EINVAL);
			break;
		}
		tp = (struct termio *)mp->b_cont->b_rptr;
		sttyp->t_cflag = (sttyp->t_cflag & 0xffff0000 | tp->c_cflag);
		sttyp->t_iflag = (sttyp->t_iflag & 0xffff0000 | tp->c_iflag);
		ws_iocack(qp, mp, iocp);
		break;
	}
	case TCGETA: {
		struct termio	*tp;

		if (mp->b_cont)	/* bad user supplied parameter */
			freemsg(mp->b_cont);
		if ((mp->b_cont = allocb(sizeof(struct termio), BPRI_MED)) == (mblk_t *)NULL) {
			cmn_err(CE_NOTE, "!kdmioctlmsg: can't get msg for reply to TCGETA");
			freemsg(mp);
			break;
		}
		tp = (struct termio *)mp->b_cont->b_rptr;
		tp->c_iflag = (ushort)sttyp->t_iflag;
		tp->c_cflag = (ushort)sttyp->t_cflag;
		mp->b_cont->b_wptr += sizeof(struct termio);
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = sizeof(struct termio);
		qreply(qp, mp);
		break;
	}
	case TCGETS: {
		struct termios	*tsp;

		if (mp->b_cont)	/* bad user supplied parameter */
			freemsg(mp->b_cont);
		if ((mp->b_cont = allocb(sizeof(struct termios), BPRI_MED)) == (mblk_t *)NULL) {
			cmn_err(CE_NOTE, "!kdmioctlmsg: can't get msg for reply to TCGETS");
			freemsg(mp);
			break;
		}
		tsp = (struct termios *)mp->b_cont->b_rptr;
		tsp->c_iflag = sttyp->t_iflag;
		tsp->c_cflag = sttyp->t_cflag;
		mp->b_cont->b_wptr += sizeof(struct termios);
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = sizeof(struct termios);
		qreply(qp, mp);
		break;
	}

	case KBIO_SETMODE:
		ws_iocnack(qp,mp,iocp,EINVAL);
		break;

	case KBIO_GETMODE:
		iocp->ioc_rval = KBM_XT;
		ws_iocack(qp,mp,iocp);
		break;

/* NOT IMPLEMENTED
	case TIOCSWINSZ:
		ws_iocack(qp, mp, iocp);
		break;
	case TIOCGWINSZ:
	case JWINSIZE:
		ws_winsz(qp, mp, chp, iocp->ioc_cmd);
		break;
NOT IMPLEMENTED */
	case TCSBRK:
		ws_iocack(qp, mp, iocp);
		break;
	case GIO_ATTR:	/* return current attribute */
		iocp->ioc_rval = tcl_curattr(chp);
		ws_iocack(qp, mp, iocp);
		break;

	case VT_OPENQRY: /* return number of first free VT */
		if (!(tmp = allocb(sizeof(int), BPRI_MED))) {
			cmn_err(CE_NOTE, "!kdmioctlmsg: can't get msg for reply to VT_OPENQRY");
			ws_iocnack(qp, mp, iocp, ENOMEM);
			break;
		}
		*(int *)tmp->b_rptr = ws_freechan(&Kdws);
		tmp->b_wptr += sizeof(int);
		ws_copyout(qp, mp, tmp, sizeof(int));
		break;
	default:
#ifdef MERGE386
		if (kdppi_ioctl(qp, mp, iocp, chp)){
			cmn_err(CE_NOTE, "!kdmioctlmsg %x", iocp->ioc_cmd);
			break;
		}
#endif /* MERGE386 */
#ifdef DEBUG1
		cmn_err(CE_NOTE, "!kdmioctlmsg %x", iocp->ioc_cmd);
#endif
		ws_iocnack(qp, mp, iocp, EINVAL);
	}
}

/*
 *
 */

kdproto(qp, mp)
queue_t *qp;
mblk_t *mp;
{
	register ch_proto_t *chprp;
	channel_t *chp;
	int error;

	chp = (channel_t *)qp->q_ptr;
	chprp = (ch_proto_t *)mp->b_rptr;
	switch (chprp->chp_stype) {
	case CH_TCL:
		if (chprp->chp_stype_cmd == TCL_FLOWCTL) {
			tcl_scrolllock(&Kdws,chp,chprp->chp_stype_arg);
			break;
		}
		/* writes as if to /dev/null when in KD_GRAPHICS mode */
		if (chp->ch_dmode != KD_GRAPHICS) 
			tcl_handler(&Kdws, mp, &chp->ch_tstate, chp);
		break;
	case CH_CHAN:
		switch (chprp->chp_stype_cmd) {
		case CH_CHANOPEN:
			error = kdvt_open(chp, chprp->chp_stype_arg);
			ws_openresp(qp,mp,chprp,chp,error);
			return; /* do not free mp because ws_openresp will */

		case CH_CHANCLOSE:
			if (chp->ch_id == 0 && chp == ws_activechan(&Kdws)) {
			   vidstate_t *vp;
			   vp = &chp->ch_vstate;
#ifdef EVGA
			   evga_ext_rest(vp->v_cvmode);
#endif	/*EVGA*/

			   if (vp->v_cvmode != vp->v_dvmode) {
				kdv_setdisp(chp,vp,&chp->ch_tstate,Kdws.w_vstate.v_dvmode);
				kdclrscr(chp, chp->ch_tstate.t_origin, chp->ch_tstate.t_scrsz);
			   }
#ifdef EVGA
			   /* kdv_setdisp() sets vp->v_cvmode to new mode */
			   evga_ext_init(vp->v_cvmode);
#endif EVGA
			}
			ws_preclose(&Kdws, chp);
			kdvt_close(chp);
			chp->ch_kbstate.kb_state = 0;
			ws_closechan(qp, &Kdws, chp, mp);
			ws_cmap_reset(&Kdws,chp->ch_charmap_p);
			ws_scrn_reset(&Kdws,chp);
			if (chp == ws_activechan(&Kdws))
				kdkb_setled(chp,&chp->ch_kbstate,0); /* turn off LEDs */
			else
				chp->ch_vstate.v_scrp = Kdws.w_scrbufpp[chp->ch_id];
			if (chp->ch_id != 0) {
				ws_chinit(&Kdws, chp, chp->ch_id);
				if (Kdws.w_scrbufpp[chp->ch_id])
					kdclrscr(chp, chp->ch_tstate.t_origin, chp->ch_tstate.t_scrsz);
			}
			return; /* don't free mp */
		default:
			cmn_err(CE_WARN, "kd_proto, received unknown CH_CHAN %d", chprp->chp_stype_cmd);
		}
		break;
	case CH_XQ:
		ws_xquemsg(chp, chprp->chp_stype_cmd);
		break;
	default:
		cmn_err(CE_WARN, "kd_proto, received unknown CH_CTL %d", chprp->chp_stype);
		break;
	}
	freemsg(mp);
}

/*
 * Called from interrupt handler when keyboard interrupt occurs.
 */

kdintr()
{
	register unchar	rawscan,	/* raw keyboard scan code */
			scan;	/* "cooked" scan code */
	channel_t	*achp;	/* active channel pointer */
	charmap_t	*cmp;	/* character map pointer */
	keymap_t	*kmp;
	kbstate_t	*kbp;	/* pointer to keyboard state */
	queue_t	*qp;
	mblk_t	*mp;
	unchar	kbrk,
		oldprev,
		tmp;
	ushort	ch,
		okbstate,
		shift,
		msk;

	if (!(inb(KB_STAT) & KB_OUTBF)){ /* no data from keyboard? */
		rawscan = inb(KB_IDAT);	/* clear possible spurious data*/
		drv_setparm(SYSRINT, 1);	/* don't care if it succeeds */
		return;			/* return immediately */
	}
	rawscan = inb(KB_IDAT);		/* read scan data */
	drv_setparm(SYSRINT, 1);	/* don't care if it succeeds */
	if (rawscan == KB_ACK) {	/* ack from keyboard? */
		return;			/* Spurious ACK -- cmds to keyboard now polled */
	}
	if (!Kdws.w_init)	/* can't do anything anyway */
		return;
	kbrk = rawscan & KBD_BREAK;
	if ((achp = ws_activechan(&Kdws)) == (channel_t *)NULL) {
		cmn_err(CE_NOTE, "kdintr: received interrupt before active channel");
   		return;
	}
	kbp = &achp->ch_kbstate;
	if (Kdws.w_qp != achp->ch_qp) {
		cmn_err(CE_NOTE, "kdintr: no active channel queue");
		return;
	}
	qp = Kdws.w_qp;
	if ((cmp = achp->ch_charmap_p) == (charmap_t *)NULL) {
		cmn_err(CE_NOTE, "kdintr: no valid ch_charmap_p");
		return;
	}
	Kdws.w_intr++;
	okbstate = kbp->kb_state;
	kmp = cmp->cr_keymap_p;
	oldprev = kbp->kb_prevscan;
	ch = ws_scanchar(cmp, kbp, rawscan, 0);
	/* check for handling extended scan codes correctly */
	/* this is because ws_scanchar calls ws_procscan on its own */
	if (oldprev == 0xe0 || oldprev == 0xe1)
		kbp->kb_prevscan = oldprev;
	scan = ws_procscan(cmp, kbp, rawscan);
	if (!kbrk)
		kdkb_keyclick(ch);
	if (kdkb_locked(ch, kbrk)) {
		Kdws.w_intr = 0;
		return;
	}
	if (!kbrk) {
		if ( (ws_specialkey(kmp,kbp,scan) || kbp->kb_sysrq)
		     && kdcksysrq(cmp, kbp, ch, scan)) {
			Kdws.w_intr = 0;
			return;
		}
	}else if ( kbp->kb_sysrq)
			if(kbp->kb_srqscan == scan){
				Kdws.w_intr = 0;
				return;
			}
	if (ws_toglchange(okbstate,kbp->kb_state))
		kdkb_cmd(LED_WARN);
	if (Kdws.w_timeid) {
		untimeout(Kdws.w_timeid);
		Kdws.w_timeid = 0;
	}
	if (ws_enque(Kdws.w_qp, &Kdws.w_mp, rawscan))
		Kdws.w_timeid = timeout(ws_kbtime, &Kdws, HZ / 29);
	Kdws.w_intr = 0;
}


/*
 *
 */

kdmiocdatamsg(qp, mp)
queue_t	*qp;
mblk_t	*mp;
{
	if (!((struct copyresp *)mp->b_rptr)->cp_rval)
		ws_iocack(qp, mp, (struct iocblk *)mp->b_rptr);
	else
		freemsg(mp);
}

/*
 *
 */

int
kdcksysrq(cmp, kbp, ch, scan)
charmap_t	*cmp;
kbstate_t	*kbp;
ushort	ch;
unchar	scan;
{
	keymap_t	*kmp = cmp->cr_keymap_p;

	if (kbp->kb_sysrq) {
		kbp->kb_sysrq = 0;
		if (!*(*cmp->cr_srqtabp + scan)) {
			kdnotsysrq(kbp);
			return(0);
		}
		ch = *(*cmp->cr_srqtabp + scan);
	}
	if (ws_speckey(ch) == HOTKEY) {
		kdvt_switch(ch);
		return(1);
	}
	switch (ch) {
	case K_DBG:
		kbp->kb_sstate = kbp->kb_state;
		(*cdebugger)(DR_USER, NO_FRAME);
		kdnotsysrq(kbp);
		return(1);
	case K_RBT:
		kdresetcpu();
		return(0);
	case K_SRQ:
		if (ws_specialkey(kmp, kbp, scan)) {
			kbp->kb_sstate = kbp->kb_state;
			kbp->kb_srqscan = scan;
			kbp->kb_sysrq++;
			return(1);
		}
		break;
	default:
		break;
	}
	return(0);
}

/*
 *
 */

kdnotsysrq(kbp)
kbstate_t	*kbp;
{
	ushort	msk;

	if ((msk = kbp->kb_sstate ^ kbp->kb_state) != 0) {
		if (Kdws.w_timeid) {
			untimeout(Kdws.w_timeid);
			Kdws.w_timeid = 0;
		}
		ws_rstmkbrk(Kdws.w_qp, &Kdws.w_mp, kbp->kb_sstate, msk);
		(void)ws_enque(Kdws.w_qp, &Kdws.w_mp, kbp->kb_srqscan);
		(void)ws_enque(Kdws.w_qp, &Kdws.w_mp, 0x80 | kbp->kb_srqscan);
		ws_rstmkbrk(Kdws.w_qp, &Kdws.w_mp, kbp->kb_state, msk);
	}
}

/*
 * 
 */

kdclrscr(chp, last, cnt)
channel_t	*chp;
ushort	last;
int	cnt;
{
	if (cnt)
		kdv_stchar(chp, last, (ushort)(NORM << 8 | ' '), cnt);
}

/*
 * implement TCL_BELL functionality. Only do it if active channel.
 */

int
kdtone(wsp, chp)
wstation_t	*wsp;
channel_t	*chp;
{
	if (chp == ws_activechan(wsp))	/* active channel */
		kdkb_tone();
}	


/*
 * perform a font shift in/shift out if requested by the active
 * channel
 */

int
kdshiftset(wsp, chp, dir)
wstation_t	*wsp;
channel_t	*chp;
int	dir;
{
	if (chp == ws_activechan(wsp))	/* active channel */
		kdv_shiftset(&chp->ch_vstate, dir);
}

/*
 *
 */

kdsetval(addr, reglow, val)
ushort	addr;
unchar	reglow;
ushort	val;
{
	intr_disable();
	outb(addr, reglow);
	outb(addr + DATA_REG, val & 0xFF);
	outb(addr, reglow - 1);
	outb(addr + DATA_REG, (val >> 8) & 0xFF);
	intr_restore();
}

/*
 *
 */

kdsetcursor(chp, tsp)
channel_t	*chp;
termstate_t	*tsp;
{
	vidstate_t	*vp = &chp->ch_vstate;

	if (chp == ws_activechan(&Kdws)) {
#ifdef BLTCONS
              	if (btcpresent) {
                        btcursor(tsp->t_row, tsp->t_col); /* do blit
cursor */
                        if (vp->v_scrp == btcpbuf)	/* if blit
ONLY */
                                return;			/* done */
		}
#endif
		kdsetval(vp->v_regaddr, R_CURADRL, tsp->t_cursor);
	}
}

/*
 *
 */

kdsetbase(chp, tsp)
channel_t	*chp;
termstate_t	*tsp;
{
	vidstate_t	*vp = &Kdws.w_vstate;

	if (chp == ws_activechan(&Kdws))
		kdsetval(vp->v_regaddr, R_STARTADRL, tsp->t_origin);
}

/*
 *
 */

kdresetcpu()
{
	if (inrtnfirm) {
		softreset();
#ifdef BLTCONS
              	btcreset();
#endif
		SEND2KBD(KB_ICMD, KB_RESETCPU);
	} 
}

/*
 *
 */

kdputchar(ch)
unchar	ch;
{
	register int	cnt = 0;
	char		out[2];
	channel_t	*chp;

	if (!Kdws.w_init)
		kdinit();
	/* convert LF to CRLF */
	if (ch == '\n')
		out[cnt++] = '\r';
	out[cnt++] = ch;
	chp = ws_getchan(&Kdws, 0);
	wsansi_parse(&Kdws, chp, out, cnt);
}

/*
 *
 */

kdgetchar()
{
	register ushort ch;	/* processed scan code */
	register unchar rawscan, kbrk;	/* raw keyboard scan code */
	channel_t	*chp;
	charmap_t	*cmp;
	keymap_t	*kmp;
	kbstate_t	*kbp;

	/* if no character in keyboard output buffer, return -1 */
	if (!(inb(KB_STAT) & KB_OUTBF))
		return(-1);
	/* get the scan code */
	rawscan = inb(KB_IDAT);    /* Read scan data */
	kdkb_force_enable();
	chp = ws_getchan(&Kdws, 0);
	cmp = chp->ch_charmap_p;
	kmp = cmp->cr_keymap_p;
	kbp = &chp->ch_kbstate;
	kbrk = rawscan & KBD_BREAK;
	/*
	 * Call ? to convert scan code to a character.
	 * ? returns a short, with flags in the top byte and the
	 * character in the low byte.
	 * A legal ascii character will have the top 9 bits off.
	 */
	ch = ws_scanchar(cmp, kbp, rawscan, 0);
	(void)ws_shiftkey(ch, (rawscan & ~KBD_BREAK), kmp, kbp, kbrk);
	if (ch & 0xFF80) {
		switch (ch) {
		case K_DBG:
			(*cdebugger)(DR_USER, NO_FRAME);
			break;
		case K_RBT:
			kdresetcpu();
			return(ch);
		default:
			break;
		}
		return(-1);
	} else
		return(ch);
}

/* VDC800 hook */
unchar *
kd_vdc800_ramd_p()
{
	channel_t *achp;
	vidstate_t	*vp;

	achp = ws_activechan(&Kdws);
	vp = &achp->ch_vstate;
	return(&kd_ramdactab[0] + (WSCMODE(vp)->m_ramdac * 0x300));
}

kd_vdc800_access()
{
	channel_t *achp;
	dev_t devp;

	if(ws_getctty(&devp) || (getminor(devp) != Kdws.w_active))
		return(1);
	kd_vdc800.procp = u.u_procp;
	kd_vdc800.pid = u.u_procp->p_pid;
	return(0);
}
void
kd_vdc800_release()
{
	kd_vdc800.procp = (struct proc *) 0;
	kd_vdc800.pid = 0;
	return;
}
