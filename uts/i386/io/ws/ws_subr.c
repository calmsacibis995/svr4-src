/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ws/ws_subr.c	1.3.2.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/inline.h"
#include "sys/pic.h"
#include "sys/cmn_err.h"
#include "sys/kmem.h"
#include "sys/vt.h"
#include "sys/at_ansi.h"
#include "sys/kd.h"
#include "sys/ascii.h"
#include "sys/tss.h"
#include "sys/proc.h"	
#include "sys/termio.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/termios.h"
#include "sys/strtty.h"
#include "sys/xque.h"
#include "sys/ws/ws.h"
#include "sys/ws/chan.h"
#include "sys/vid.h"
#include "sys/jioctl.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/session.h"
#include "sys/strsubr.h"
#include "sys/cred.h"
#include "sys/conf.h"
#include "sys/ddi.h"

/*
 * Given a (wstation_t *),
 * return the active channel as a (channel_t *).
 */

channel_t *
ws_activechan(wsp)
register wstation_t	*wsp;
{
	return((channel_t *)*(wsp->w_chanpp + wsp->w_active));
}

/*
 * Given a (wstation_t *) and a channel number,
 * return the channel as a (channel_t *).
 */

channel_t *
ws_getchan(wsp, chan)
register wstation_t	*wsp;
register int	chan;
{
	return((channel_t *)*(wsp->w_chanpp + chan));
}

/*
 *
 */

ws_freechan(wsp)
register wstation_t	*wsp;
{
	register int	cnt;
	channel_t *chp;

	for (cnt = 0; cnt < WS_MAXCHAN; cnt++) {
		chp = * (wsp->w_chanpp + cnt);
		if (!chp) return (-1);
		if (!chp->ch_opencnt)
			return(cnt);
	}
	return(-1);
}

/*
 *
 */

int
ws_getchanno(cmux_minor)
minor_t cmux_minor;
{
	return (cmux_minor % WS_MAXCHAN);
}


int
ws_getws(cmux_minor)
minor_t cmux_minor;
{
	return (cmux_minor / WS_MAXCHAN);
}

extern struct attrmask	kb_attrmask[];
extern int	nattrmsks;

extern charmap_t	*ws_cmap_alloc();

ws_chinit(wsp, chp, chan)
wstation_t	*wsp;
channel_t	*chp;
int	chan;
{
	vidstate_t	*vp;
	termstate_t	*tsp;
	unchar	cnt;

	chp->ch_wsp = wsp;
	chp->ch_opencnt = 0;
	chp->ch_procp = (struct proc *)NULL;
	chp->ch_pid = 0;
	chp->ch_relsig = SIGUSR1;
	chp->ch_acqsig = SIGUSR1;
	chp->ch_frsig = SIGUSR2;
	if (!(chp->ch_strtty.t_state & (ISOPEN | WOPEN))) {
		chp->ch_strtty.t_line = 0;
		chp->ch_strtty.t_iflag = IXON | ICRNL | ISTRIP;
		chp->ch_strtty.t_oflag = OPOST | ONLCR;
		chp->ch_strtty.t_cflag = B9600 | CS8 | CREAD | HUPCL;
		chp->ch_strtty.t_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK;
		chp->ch_strtty.t_state |= CARR_ON;
		chp->ch_strtty.t_state |= (ISOPEN | WOPEN);
	}
	chp->ch_id = chan;
	chp->ch_dmode = wsp->w_dmode;
	chp->ch_vstate = wsp->w_vstate;		/* struct copy */
	chp->ch_flags = 0;
	vp = &chp->ch_vstate;
	tsp = &chp->ch_tstate;
	tsp->t_sending = tsp->t_sentrows = tsp->t_sentcols = 0;
	if (vp->v_cmos == MCAP_COLOR)
		tsp->t_flags = ANSI_MOVEBASE;
	else
		tsp->t_flags = 0;
	vp->v_undattr = wsp->w_vstate.v_undattr;
	tsp->t_flags = 0;
	tsp->t_rows = WSCMODE(vp)->m_rows;
	tsp->t_cols = WSCMODE(vp)->m_cols;
	tsp->t_scrsz = tsp->t_rows * tsp->t_cols;
	tsp->t_attrmskp = kb_attrmask;
 	if (vp->v_regaddr == MONO_REGBASE) {
 		tsp->t_attrmskp[1].attr = 0;
 		tsp->t_attrmskp[4].attr = 1;
 		tsp->t_attrmskp[34].attr = 7;
 	} else {
 		tsp->t_attrmskp[1].attr = BRIGHT;
 		tsp->t_attrmskp[4].attr = 0;
 		tsp->t_attrmskp[34].attr = 1;
 	}
	tsp->t_nattrmsk = nattrmsks;
	tsp->t_normattr = NORM;
	tsp->t_origin = 0;
	tsp->t_row = 0;
	tsp->t_col = 0;
	tsp->t_cursor = 0;
	tsp->t_curtyp = 0;
	tsp->t_undstate = 0;
	tsp->t_curattr = tsp->t_normattr;
	tsp->t_font = ANSI_FONT0;
	tsp->t_pstate = 0;
	tsp->t_ppres = 0;
	tsp->t_pcurr = 0;
	tsp->t_pnum = 0;
	tsp->t_ntabs = 9;
	for (cnt = 0; cnt < 9; cnt++)
		tsp->t_tabsp[cnt] = cnt * 8 + 8;
}

/*
 * ws_openresp -- expected call fron principal stream upon receipt of
 * CH_CHANOPEN message from CHANMUX
 */
void
ws_openresp(qp,mp,protop,chp,error)
queue_t *qp;
mblk_t *mp;
ch_proto_t *protop;
channel_t *chp;
unsigned long error;
{
	mblk_t *charmp,*scrmp;

	mp->b_datap->db_type = M_PCPROTO;
	protop->chp_stype = CH_PRINC_STRM;
	protop->chp_stype_cmd = CH_OPEN_RESP;
	protop->chp_stype_arg = error;
	qreply(qp,mp);

	if (error)
		return;

	chp->ch_opencnt++;
	if (!(charmp = allocb(sizeof(ch_proto_t), BPRI_HI)))
		return;

	charmp->b_datap->db_type = M_PROTO;
	charmp->b_wptr += sizeof(ch_proto_t);
	protop = (ch_proto_t *)charmp->b_rptr;
	protop->chp_type = CH_CTL;
	protop->chp_stype = CH_CHR;
	protop->chp_stype_cmd = CH_CHRMAP;
	protop->chp_stype_arg = (unsigned long)chp->ch_charmap_p;

	if ((scrmp = copymsg(charmp)) == (mblk_t *) NULL)
		qreply(qp,charmp);
	else {
		qreply(qp, charmp);
		protop = (ch_proto_t *) scrmp->b_rptr;
		protop->chp_stype_cmd = CH_SCRMAP;
		protop->chp_stype_arg = (unsigned long)&chp->ch_scrn;
		qreply(qp,scrmp);
	}
	return;
}

/*
 * ws_preclose -- call before doing actual channel close in principal
 * stream. Should be called upon receipt of a CH_CLOSECHAN message
 * from CHANMUX 
 */
ws_preclose(wsp, chp)
wstation_t	*wsp;
channel_t *chp;
{
	channel_t	*achp;
	int oldpri,id;

	chp->ch_flags &= ~CHN_KILLED;
	wsp->w_noacquire = 0;
	if (wsp->w_forcetimeid && (wsp->w_forcechan == chp->ch_id)) {
		untimeout(wsp->w_forcetimeid);
		wsp->w_forcetimeid = 0;
		wsp->w_forcechan = 0;
	}

	if (!(achp = ws_activechan(wsp))) {
		cmn_err(CE_WARN, "ws_preclose: no active channel");
		return;
	}
	chp->ch_opencnt = 0;
	ws_automode(wsp, chp);
	chp->ch_dmode = wsp->w_dmode;
	chp->ch_vstate = wsp->w_vstate;		/* struct copy */
	chp->ch_flags = 0;
	if (chp == achp)
		if (achp->ch_prevp != achp)
			(void )ws_activate(wsp,chp->ch_prevp,VT_FORCE);
		else
			(void)ws_activate(wsp,ws_getchan(wsp,0),VT_FORCE);	

	oldpri = splhi();
	if (chp->ch_prevp)
		chp->ch_prevp->ch_nextp = chp->ch_nextp;
	if (chp->ch_nextp)
		chp->ch_nextp->ch_prevp = chp->ch_prevp;
	chp->ch_prevp = chp->ch_nextp = chp;
	splx(oldpri);
}

/*
 * ws_close_chan() is called after principal stream-specific close() routine
 * is called. This routine sends up the CH_CLOSE_ACK message CHANMUX is
 * sleeping on
 */

void
ws_closechan(qp, wsp, chp, mp)
queue_t *qp;
wstation_t	*wsp;
channel_t *chp;
mblk_t *mp;
{
	ch_proto_t *protop;

	protop = (ch_proto_t *) mp->b_rptr;
	protop->chp_stype = CH_PRINC_STRM;
	protop->chp_stype_cmd = CH_CLOSE_ACK;
	qreply(qp,mp);
}


/*
 *
 */

ws_activate(wsp, chp, force)
wstation_t	*wsp;
channel_t	*chp;
int	force;
{
	channel_t	*achp;
	int	oldpri;

	if (chp == (achp = ws_activechan(wsp)))
		return(1);
	if (!ws_procmode(wsp, achp) || force || PTRACED(achp->ch_procp))
		return(ws_switch(wsp, chp, force));
	oldpri = splstr();
	if (wsp->w_switchto) {	/* switch pending */
		splx(oldpri);
		return(0);
	}
	splx(oldpri);
	if (wsp->w_noacquire)
		return(0);
	psignal(achp->ch_procp, achp->ch_relsig);
	wsp->w_switchto = chp;
	achp->ch_timeid = timeout(wsp->w_rel_refuse, wsp, 10 * HZ);
	return(1);
}

/*
 *
 */

ws_switch(wsp, chp, force)
wstation_t	*wsp;
channel_t	*chp;
int	force;
{
	channel_t	*achp;
	int	oldpri;
	ch_proto_t *protop;
	mblk_t *mp;

	if (wsp->w_forcetimeid || (chp->ch_id != 0 && CHNFLAG(chp, CHN_KILLED)))
		return (0);
	if ((mp = allocb(sizeof(ch_proto_t),BPRI_HI)) == (mblk_t *)NULL)
		return(0);
	achp = ws_activechan(wsp);
	oldpri = splhi();
	if (achp->ch_timeid) {
		untimeout(achp->ch_timeid);
		achp->ch_timeid = 0;
	}
	if ((*wsp->w_activate)(chp, force)) {
		mp->b_datap->db_type = M_PROTO;
		mp->b_wptr += sizeof(ch_proto_t);
		protop = (ch_proto_t *)mp->b_rptr;
		protop->chp_type = CH_CTL;
		protop->chp_stype = CH_PRINC_STRM;
		protop->chp_stype_cmd = CH_CHANGE_CHAN;
		drv_getparm(LBOLT, &protop->chp_tstmp);
		protop->chp_chan = chp->ch_id;
		putnext(chp->ch_qp, mp);
	} else {
		freemsg(mp);
		splx(oldpri);
		return(0);
	}
	wsp->w_switchto = (channel_t *)NULL;
	achp->ch_flags &= ~CHN_ACTV;
	if (ws_procmode(wsp, achp) && force && !PTRACED(achp->ch_procp)) 
		psignal(achp->ch_procp, achp->ch_frsig);
	chp->ch_flags |= CHN_ACTV;
	splx(oldpri);
	if (ws_procmode(wsp, chp) && !PTRACED(chp->ch_procp)) {	
		wsp->w_noacquire++;
		psignal(chp->ch_procp, chp->ch_acqsig);
		chp->ch_timeid = timeout(wsp->w_acq_refuse, chp, 10*HZ);
	}
	/* if new vt is waiting to become active, then wake it up */
	if (CHNFLAG(chp, CHN_WACT)) {
		chp->ch_flags &= ~CHN_WACT;
		wakeup(&chp->ch_flags);
	}
	wakeup(chp);
	return(1);
}

/*
 *
 */

ws_procmode(wsp, chp)
wstation_t	*wsp;
channel_t	*chp;
{
	register int	oldpri;

	if (chp->ch_procp && !validproc(chp->ch_procp,chp->ch_pid)) {
		oldpri = splstr();
		ws_automode(wsp, chp);
		splx(oldpri);
	}
	return(CHNFLAG(chp, CHN_PROC));
}

/*
 *
 */

ws_automode(wsp, chp)
wstation_t	*wsp;
channel_t	*chp;
{
	channel_t	*achp;
	struct map_info	*map_p = &wsp->w_map;
	struct proc	*procp;

	achp = ws_activechan(wsp);
	if (chp == achp && map_p->m_procp && map_p->m_procp == chp->ch_procp) {
		if (!validproc(chp->ch_procp,chp->ch_pid)) {
			map_p->m_procp = (struct proc *)0;
			chp->ch_flags &= ~CHN_MAPPED;
			map_p->m_cnt = 0;
			map_p->m_chan = 0;
		} else {
			drv_getparm(UPROCP, &procp);
			if (map_p->m_procp == procp)
				(*wsp->w_unmapdisp)(chp, map_p);
		}
	}
	chp->ch_procp = (struct proc *)NULL;
	chp->ch_pid = 0;
	chp->ch_flags &= ~CHN_PROC;
	chp->ch_relsig = SIGUSR1;
	chp->ch_acqsig = SIGUSR1;
	chp->ch_frsig = SIGUSR2;
}

/*
 *
 */

ws_xferwords(srcp, dstp, cnt, dir)
register ushort	*srcp, *dstp;
register int	cnt;
char	dir;
{
	switch (dir) {
	case UP:
		while (cnt--)
			*dstp-- = *srcp--;
		break;
	default:
		while (cnt--)
			*dstp++ = *srcp++;
		break;
	}
}

/*
 *
 */

ws_setlock(wsp, lock)
wstation_t	*wsp;
int	lock;
{

	if (lock)
		wsp->w_flags |= KD_LOCKED;
	else
		wsp->w_flags &= ~KD_LOCKED;
}

ws_sigkill(wsp)
wstation_t	*wsp;
{
	int chan;
	vidstate_t vbuf;
	channel_t *chp;
	struct map_info	*map_p;

	map_p = &wsp->w_map;
	chan = wsp->w_forcechan;
	if (wsp->w_forcetimeid && (wsp->w_active == chan)) {
		chp = (channel_t *) ws_getchan(wsp,chan);
		if (chp == NULL) return;
		bcopy(&chp->ch_vstate,&vbuf,sizeof(vidstate_t));
		ws_chinit(wsp,chp,chan);
		chp->ch_opencnt = 1;
		chp->ch_flags |= CHN_KILLED;
		if (map_p->m_procp && map_p->m_chan == chp->ch_id) 
			bzero(map_p,sizeof(struct map_info));
		bcopy(&vbuf,&chp->ch_vstate,sizeof(vidstate_t));
		chp->ch_vstate.v_cvmode = wsp->w_vstate.v_dvmode;
		wsp->w_forcetimeid = 0;
		wsp->w_forcechan = 0;
		tcl_reset(wsp,chp,&chp->ch_tstate);
		putctl1(chp->ch_qp->q_next,M_ERROR,ENXIO);
		if (chp->ch_nextp) {
			chp->ch_nextp->ch_prevp = chp->ch_prevp;
			ws_activate(wsp,chp->ch_nextp,VT_NOFORCE);
		}
		if (chp->ch_prevp) 
			chp->ch_prevp->ch_nextp = chp->ch_nextp;

	}
}


ws_noclose(wsp)
wstation_t	*wsp;
{
	int chan;
	chan = wsp->w_forcechan;
	if (wsp->w_forcetimeid && (wsp->w_active == chan)) {
		putctl1(wsp->w_chanpp[wsp->w_active]->ch_qp->q_next,M_PCSIG,SIGKILL);
		wsp->w_forcetimeid = timeout(ws_sigkill, wsp, 5*HZ);
	}
	else {
		wsp->w_forcetimeid = 0;
		wsp->w_forcechan = 0;
	}
}

void
ws_force(wsp, chp)
wstation_t	*wsp;
channel_t	*chp;
{
	wsp->w_forcetimeid = timeout(ws_noclose, wsp, 10*HZ);
	wsp->w_forcechan = chp->ch_id;
	putctl1(chp->ch_qp->q_next,M_PCSIG,SIGINT);
	putctl(chp->ch_qp->q_next,M_HANGUP);
}

/*
 *
 */

ws_mctlmsg(qp, mp)
queue_t	*qp;
mblk_t	*mp;
{
	struct iocblk	*iocp;

	if (mp->b_wptr - mp->b_rptr != sizeof(struct iocblk)) {
		cmn_err(CE_NOTE, "!kdmctlmsg: bad M_CTL msg");
		freemsg(mp);
		return;
	}
	if ((iocp = (struct iocblk *)mp->b_rptr)->ioc_cmd != MC_CANONQUERY) {
		cmn_err(CE_NOTE, "!ws_mctlmsg: M_CTL msg not MC_CANONQUERY");
		return;
	}
	iocp->ioc_cmd = MC_DO_CANON;
	qreply(qp, mp);
}

/*
 *
 */

ws_notifyvtmon(vtmchp, ch)
channel_t	*vtmchp;
unchar	ch;
{
	mblk_t	*mp;

	if (!(mp = allocb(sizeof(unchar) * 1, BPRI_MED))) {
		cmn_err(CE_NOTE, "!kdnotifyvtmon: can't get msg");
		return;
	}
	*mp->b_wptr++ = ch;
	putnext(vtmchp->ch_qp, mp);
}

/*
 *
 */

ws_iocack(qp, mp, iocp)
queue_t	*qp;
mblk_t	*mp;
struct iocblk	*iocp;
{
	mblk_t	*tmp;

	mp->b_datap->db_type = M_IOCACK;
	if ((tmp = unlinkb(mp)) != (mblk_t *)NULL)
		freeb(tmp);
	iocp->ioc_count = iocp->ioc_error = 0;
	qreply(qp, mp);
}

/*
 *
 */

ws_iocnack(qp, mp, iocp, error)
queue_t	*qp;
mblk_t	*mp;
struct iocblk	*iocp;
int	error;
{
	mp->b_datap->db_type = M_IOCNAK;
	iocp->ioc_rval = -1;
	iocp->ioc_error = error;
	qreply(qp, mp);
}

/*
 *
 */

ws_copyout(qp, mp, tmp, size)
queue_t	*qp;
mblk_t	*mp, *tmp;
uint	size;
{
	struct copyreq	*cqp;

	cqp = (struct copyreq *)mp->b_rptr;
	cqp->cq_size = size;
	cqp->cq_addr = (caddr_t)*(long *)mp->b_cont->b_rptr;
	cqp->cq_flag = 0;
	cqp->cq_private = (mblk_t *)NULL;
	mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
	mp->b_datap->db_type = M_COPYOUT;
	if (mp->b_cont)
		freemsg(mp->b_cont);
	mp->b_cont = tmp;
	qreply(qp, mp);
}

/*
 *
 */

ws_mapavail(chp, map_p)
channel_t	*chp;
struct map_info	*map_p;
{
	if (!map_p->m_procp) {
		chp->ch_flags &= ~CHN_MAPPED;
		return;
	}

	if (!validproc(map_p->m_procp,map_p->m_pid)) {
		map_p->m_procp = (struct proc *)0;
		map_p->m_pid = (pid_t) 0;
		chp->ch_flags &= ~CHN_MAPPED;
		map_p->m_cnt = 0;
		map_p->m_chan = 0;
	}
}


/*
 *
 */

#define XQDISAB	0
#define XQENAB	1

ws_notify(chp, state)
channel_t	*chp;
int	state;
{
	mblk_t	*mp;
	ch_proto_t	*protop;

	if (!(mp = allocb(sizeof(ch_proto_t), BPRI_MED))) {
		cmn_err(CE_WARN, "ws_notify: cannot alloc msg");
		return(0);
	}
	mp->b_wptr += sizeof(ch_proto_t);
	mp->b_datap->db_type = M_PROTO;
	protop = (ch_proto_t *)mp->b_rptr;
	protop->chp_type = CH_CTL;
	protop->chp_stype = CH_XQ;
	if (state == XQENAB) {
		protop->chp_stype_cmd = CH_XQENAB;
		protop->chp_stype_arg = (long)&chp->ch_xque;
	} else {
		protop->chp_stype_cmd = CH_XQDISAB;
		protop->chp_stype_arg = 0;
	}
	if (chp->ch_qp == chp->ch_wsp->w_qp)
		ws_kbtime(chp->ch_wsp);
	putnext(chp->ch_qp, mp);
	sleep(&chp->ch_xque, PZERO+1);
	if (state == XQENAB && !CHNFLAG(chp, CHN_QRSV))	/* something's wrong */
		return(0);
	return(1);
}

/*
 *
 */

ws_queuemode(chp, arg)
channel_t	*chp;
int	arg;
{
	struct kd_quemode	qmode;
	xqInfo	*xqp = &chp->ch_xque;
	struct proc	*procp;
	int error = 0;

	if (arg) {	/* enable queue mode */
		while (CHNFLAG(chp, CHN_QRSV))
			sleep(&chp->ch_flags, PZERO+1);
		if (xqp->xq_proc && !validproc(xqp->xq_proc,xqp->xq_pid))
			xq_close(xqp);
		if (xqp->xq_queue)	/* already in queue mode */
			return(EBUSY);
		chp->ch_flags |= CHN_QRSV;
		if (copyin(arg, (caddr_t)&qmode, sizeof(qmode)) < 0) {
			chp->ch_flags &= ~CHN_QRSV;
			wakeup(&chp->ch_flags);
			return(EFAULT);
		}
		qmode.qaddr = xq_init(xqp, qmode.qsize, qmode.signo,&error);
		if (!qmode.qaddr || error || !ws_notify(chp, XQENAB)) {
			chp->ch_flags &= ~CHN_QRSV;
			wakeup(&chp->ch_flags);
			return(error ? error : EFAULT);
		}
		if (copyout((caddr_t)&qmode, arg, sizeof(qmode)) < 0) {
			(void)ws_notify(chp, XQDISAB);
			xq_close(xqp);
			chp->ch_flags &= ~CHN_QRSV;
			wakeup(&chp->ch_flags);
			return(EFAULT);
		}
		chp->ch_flags &= ~CHN_QRSV;
		wakeup(&chp->ch_flags);
	} else if (xqp->xq_queue) {
		pid_t ppid;
		drv_getparm(PPID, &ppid);
		if (ppid != xqp->xq_pid)
			return(EACCES);
		(void)ws_notify(chp, XQDISAB);
		xq_close(xqp);
	}
	return(0);
}

/*
 *
 */

ws_xquemsg(chp, reply)
channel_t	*chp;
long	reply;
{
	if (reply == CH_XQENAB_NACK)
		chp->ch_flags &= ~CHN_QRSV;
	wakeup(&chp->ch_xque);
}

/*
 *
 */

ws_ck_kd_port(vp, port)
vidstate_t	*vp;
ushort	port;
{
	register int	indx, cnt;

	for (cnt = 0; cnt < MKDIOADDR; cnt++) {
		if (vp->v_ioaddrs[cnt] == port)
			return(1);
		if (!vp->v_ioaddrs[cnt])
			break;
	}
	return(0);
}


/*
 *
 */

ws_winsz(qp, mp, chp, cmd)
queue_t	*qp;
mblk_t 	*mp;
channel_t	*chp;
int	cmd;
{
	vidstate_t	*vp = &chp->ch_vstate;
	mblk_t	*tmp;

	switch (cmd) {
	case TIOCGWINSZ: {
		struct winsize	*winp;

		if ((tmp = allocb(sizeof(struct winsize), BPRI_MED)) == (mblk_t *)NULL) {
			cmn_err(CE_NOTE, "!ws_winsz: can't get msg for reply to TIOCGWINSZ");
			freemsg(mp);
			break;
		}
		winp = (struct winsize *)tmp->b_rptr;
		winp->ws_row = (ushort)(WSCMODE(vp)->m_rows & 0xffff);
		winp->ws_col = (ushort)(WSCMODE(vp)->m_cols & 0xffff);
		winp->ws_xpixel = (ushort)(WSCMODE(vp)->m_xpels & 0xffff);
		winp->ws_ypixel = (ushort)(WSCMODE(vp)->m_ypels & 0xffff);
		tmp->b_wptr += sizeof(struct winsize);
		ws_copyout(qp, mp, tmp, sizeof(struct winsize));
		break;
	}
	case JWINSIZE: {
		struct jwinsize	*jwinp;

		if ((tmp = allocb(sizeof(struct jwinsize), BPRI_MED)) == (mblk_t *)NULL) {
			cmn_err(CE_NOTE, "!ws_winsz: can't get msg for reply to JWINSIZE");
			freemsg(mp);
			break;
		}
		jwinp = (struct jwinsize *)tmp->b_rptr;
		jwinp->bytesx = (char)(WSCMODE(vp)->m_cols & 0xff);
		jwinp->bytesy = (char)(WSCMODE(vp)->m_rows & 0xff);
		jwinp->bitsx = (short)(WSCMODE(vp)->m_xpels & 0xffff);
		jwinp->bitsy = (short)(WSCMODE(vp)->m_ypels & 0xffff);
		tmp->b_wptr += sizeof(struct jwinsize);
		ws_copyout(qp, mp, tmp, sizeof(struct jwinsize));
		break;
	}
	default:
		break;
	}
}

/* 
 * ws routine for obtaining controlling TTY device number.  The assumption
 * is made that this routine is called with user context
 */

int
ws_getctty(devp)
register dev_t *devp;
{
	register sess_t *sp;

	sp = u.u_procp->p_sessp;
	if (sp->s_dev == NODEV)
		return ENXIO;
	if (sp->s_vp == NULL)
		return EIO;
	*devp = sp->s_dev;
	return 0;
}

/*
 * WS routine for performing ioctls. Allows the mouse add-on to 
 * be protected from cdevsw[] dependencies
 */

extern vnode_t *specfind();

ws_ioctl(dev,cmd,arg,mode,crp,rvalp)
dev_t dev;
int cmd, arg, mode;
cred_t *crp;
int *rvalp;
{
	vnode_t *vp;
	int error;
	
	if (cdevsw[getmajor(dev)].d_str) {
		vp = specfind(dev,VCHR); /* does a VN_HOLD on the vnode */
		if (vp == (vnode_t *) NULL)
			return (EINVAL);
		error = strioctl(vp, cmd, arg, mode,U_TO_K, crp, rvalp);
		VN_RELE(vp); /* lower reference count */
	} else
		error = (*cdevsw[getmajor(dev)].d_ioctl)
				  (dev, cmd, arg, mode, crp, rvalp);
	return error;
}

/*
 * Return (via two pointers to longs) the screen resolution for the
 * active channel.  For text modes, return the number of columns and
 * rows, for graphics modes, return the number of x and y pixels.
 */

extern wstation_t	Kdws;

void
ws_scrnres(xp, yp)
ulong	*xp,
	*yp;
{
	vidstate_t	*vp = &(ws_activechan(&Kdws)->ch_vstate);

	if (!WSCMODE(vp)->m_font) {	/* graphics mode */
		*xp = WSCMODE(vp)->m_xpels;
		*yp = WSCMODE(vp)->m_ypels;
	} else {			/* text mode */
		*xp = WSCMODE(vp)->m_cols;
		*yp = WSCMODE(vp)->m_rows;
	}
}


/* The following routines support COFF-based SCO applications that
 * use KD driver ioctls that overlap with STREAMS ioctls.
 */

extern unchar ws_compatflgs[];

void
ws_setcompatflgs(dev)
dev_t dev;
{
	ws_compatflgs[getminor(dev)/8] |= 1 << (getminor(dev) % 8);
}

void
ws_clrcompatflgs(dev)
dev_t dev;
{
	ws_compatflgs[getminor(dev)/8] &= ~(1 << (getminor(dev) % 8));
}

int
ws_compatset(dev)
dev_t dev;
{
	return (ws_compatflgs[getminor(dev)/8] & (1 << (getminor(dev) % 8)));
}

extern int maxminor;

void
ws_initcompatflgs(dev)
dev_t dev;
{
	bzero(&ws_compatflgs[0],maxminor/8 + 1);
}
