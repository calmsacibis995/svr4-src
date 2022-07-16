/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)at:uts/i386/io/kd/kdvt.c	1.2.2.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/proc.h"	
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/inline.h"
#include "sys/cmn_err.h"
#include "sys/vt.h"
#include "sys/at_ansi.h"
#include "sys/kd.h"
#include "sys/stream.h"
#include "sys/termios.h"
#include "sys/strtty.h"
#include "sys/stropts.h"
#include "sys/xque.h"
#include "sys/ws/ws.h"
#include "sys/cred.h"
#include "sys/ws/chan.h"
#include "sys/vid.h"
#include "sys/ws/tcl.h"
#include "sys/kb.h"
#include "sys/kmem.h"
#include "sys/vdc.h"
#include "sys/ddi.h"

/* VDC800 hook */
extern struct ext_graph kd_vdc800;

extern struct vdc_info	Vdc;
extern wstation_t	Kdws;

channel_t	*ws_activechan(),
		*ws_getchan();

extern int	kdvm_unmapdisp();

int	ws_freechan();
extern channel_t Kd0chan;

#ifdef EVGA
extern int evga_inited;
extern int cur_mode_is_evga;
extern unchar saved_misc_out;
extern struct at_disp_info disp_info[];
#endif	/*EVGA*/

/*
 *
 */

kdvt_open(chp, ppid)
channel_t	*chp;
pid_t	ppid;
{
	channel_t	*achp;
	int oldpri,indx;
	ushort *scrp;

	if (!(achp = ws_activechan(&Kdws))) {
		cmn_err(CE_WARN, "kdvt_open: no active channel");
		return(EINVAL);
	}
	indx = chp->ch_id;
	if (!Kdws.w_scrbufpp[indx]) {
		if (!(scrp = (ushort *)kmem_alloc(sizeof(ushort) * KD_MAXSCRSIZE, KM_NOSLEEP))) {
			cmn_err(CE_WARN, "kdvt_open: out of memory for new screen for virtual terminal 0x%x",indx);
			kdvt_switch(K_VTF); /* switch to channel 0 */
			return(ENOMEM);
		}
		Kdws.w_scrbufpp[indx] = scrp;
		if (chp != achp)
			chp->ch_vstate.v_scrp = scrp;
		kdclrscr(chp, chp->ch_tstate.t_origin, chp->ch_tstate.t_scrsz);
	}
	oldpri = splhi();
	chp->ch_nextp = achp->ch_nextp;
	achp->ch_nextp = chp;
	chp->ch_prevp = chp->ch_nextp->ch_prevp;
	chp->ch_nextp->ch_prevp = chp;
	splx(oldpri);
	if (chp != achp && ppid != 1 && !ws_activate(&Kdws, chp, VT_NOFORCE))
		kdvt_rel_refuse();
	return(0);
}

/*
 *
 */

kdvt_close(chp)
channel_t	*chp;
{
	channel_t	*achp;
	int oldpri,id;

	id = chp->ch_id;
	if (id != 0 && Kdws.w_scrbufpp[id]) {
		chp->ch_vstate.v_scrp = (ushort *) NULL;
		kmem_free(Kdws.w_scrbufpp[id],sizeof(ushort) * KD_MAXSCRSIZE); 
		Kdws.w_scrbufpp[id] = (ushort *) NULL;
	}
}

kdvt_isnormal(chp)
channel_t *chp;
{
	ws_mapavail(chp, &Kdws.w_map);
	if (chp->ch_dmode == KD_GRAPHICS || CHNFLAG(chp, CHN_MAPPED)|| CHNFLAG(chp, CHN_PROC))
		return (0);
	else
		return 1;
}

/* VDC800 hooks */
	
void (*kd_vdc800_vgamode)();

void
kd_vdc800_vgapass(vgapass_p)
void ( *vgapass_p)();
{
	kd_vdc800_vgamode = vgapass_p;
}

/*
 *
 */

kdvt_switch(ch)
ushort	ch;
{
	channel_t	*newchp, *chp, *vtmchp;
	extern void kd_vdc800_release();
	int	chan = -1;

	if ((chp = ws_activechan(&Kdws)) == (channel_t *) NULL) {
		cmn_err(CE_WARN,"kdvt_switch: Could not find active channel!");
		return;
	}
/* check if VDC800 is open and active vt is not in proc mode */
	if(kd_vdc800.procp && (chp->ch_flags & CHN_PROC) == 0){
		if(prfind(kd_vdc800.pid) != NULL){
			kdvt_rel_refuse();
			return;
		} else {  /* process died */
			if(kd_vdc800_vgamode)
				(*kd_vdc800_vgamode)();
			kd_vdc800_release();
		}
	}
	if (ch >= K_VTF && ch <= K_VTL)
		chan = ch - K_VTF;
	switch (ch) {
	case K_NEXT:
		chan = chp->ch_nextp->ch_id;
		break;
	case K_PREV:
		chan = chp->ch_prevp->ch_id;
		break;
	case K_FRCNEXT:
		if (kdvt_isnormal(chp)) {
			chan = chp->ch_nextp->ch_id;
			break;
		}
		ws_force(&Kdws, chp);
		return;
	case K_FRCPREV:
		if (kdvt_isnormal(chp)) {
			chan = chp->ch_prevp->ch_id;
			break;
		}
		ws_force(&Kdws, chp);
		return;
	default:
		break;
	}
	if (chan != -1 && (newchp = ws_getchan(&Kdws, chan))) {
		if (newchp->ch_opencnt || (chan == 0)) {
			if (ws_activate(&Kdws, newchp, VT_NOFORCE))
				return;
		} else if ((vtmchp = ws_getchan(&Kdws, WS_MAXCHAN))) {
			ws_notifyvtmon(vtmchp, ch);
			return;
		}
	}
	kdvt_rel_refuse();
}

/*
 *
 */

void	ws_xferkbstat();

kdvt_activate(chp, force)
channel_t	*chp;
int	force;
{
	channel_t	*achp;
	vidstate_t	*vp;
	termstate_t	*tsp;
	kbstate_t	*kbp;
	int	cnt, oldpri;
	struct proc	*procp;
	unchar	omode,tmp;


	if (Kdws.w_flags & WS_NOCHANSW)
		return(0);
	if ((achp = ws_activechan(&Kdws)) == (channel_t *)NULL) {
		cmn_err(CE_WARN, "kdvt_activate: no active channel");
		return(0);
	}
	ws_mapavail(achp, &Kdws.w_map);
	if ((achp->ch_dmode == KD_GRAPHICS || CHNFLAG(achp, CHN_MAPPED)) && !CHNFLAG(achp, CHN_PROC))
		return(0);
	/* save state of current channel */
	oldpri = splstr();
	vp = &achp->ch_vstate;
	tsp = &achp->ch_tstate;
	kdkb_sound(0);
	if (achp->ch_dmode != KD_GRAPHICS) {	/* save text mode state */
		if (!(force & VT_NOSAVE))
			kdv_scrxfer(achp, KD_SCRTOBUF);
		tsp->t_cursor -= tsp->t_origin;
		tsp->t_origin = 0;
	} else {
		vp->v_font = 0;
	}
	vp->v_scrp = *(Kdws.w_scrbufpp + achp->ch_id);
	if (force & VT_NOSAVE) {
		vp->v_cvmode = vp->v_dvmode; 
		achp->ch_dmode = KD_TEXT0;
		tsp->t_cols = WSCMODE(vp)->m_cols;
		tsp->t_rows = WSCMODE(vp)->m_rows;
		tsp->t_scrsz = tsp->t_rows * tsp->t_cols;
	} else
		vp->v_dvmode = vp->v_cvmode; /* XXX -- 3.2 behavior was the reverse! */

	omode = vp->v_cvmode;
	if (Kdws.w_timeid) {
		untimeout(Kdws.w_timeid);
		Kdws.w_timeid = 0;
	}
	kbp = &achp->ch_kbstate;
	ws_xferkbstat(kbp,&chp->ch_kbstate);
	ws_rstmkbrk(Kdws.w_qp, &Kdws.w_mp, kbp->kb_state, NONTOGGLES);  
	ws_kbtime(&Kdws);
	Kdws.w_active = chp->ch_id;
	Kdws.w_qp = chp->ch_qp;
	vp = &chp->ch_vstate;
	tsp = &chp->ch_tstate;
	kbp = &chp->ch_kbstate;

	/* Cause calls to kdv_setmode to block */

	Kdws.w_flags |= WS_NOMODESW;

#ifdef EVGA
	evga_ext_rest(cur_mode_is_evga);
#endif	/*EVGA*/

/* XXX used to be kdv_setdisp */
	if (chp->ch_dmode == KD_GRAPHICS)
		kdv_rst(tsp,vp);
	else
		kdv_setdisp(chp,vp,tsp,vp->v_cvmode);

#ifdef EVGA
	evga_ext_init(vp->v_cvmode);
#endif	/*EVGA*/

	if (chp->ch_dmode != KD_GRAPHICS) {
		if (VTYPE(V400) || DTYPE(Kdws,KD_EGA) || DTYPE(Kdws,KD_VGA)) {
			if (vp->v_undattr == UNDERLINE) {
				kdv_setuline(vp,1);
				kdv_mvuline(vp,1);
			} else {
				kdv_setuline(vp,0);
				kdv_mvuline(vp,0);
			}
		}
		kdsetbase(chp, tsp);
		kdv_scrxfer(chp, KD_BUFTOSCR);
		kdsetcursor(chp, tsp);
		if (DTYPE(Kdws,KD_VGA)) {
			(void) inb(vp->v_regaddr + IN_STAT_1);
			outb(0x3c0,0x10);	/* attribute mode control reg */
			tmp = inb(0x3c1);
			if (tsp->t_flags & T_BACKBRITE)
				outb(0x3c0,(tmp & ~0x08));
			else
				outb(0x3c0,(tmp | 0x08));
			outb(0x3c0, 0x20);	/* turn palette on */
		}
	} 
	if ((chp->ch_dmode != KD_GRAPHICS) || !(WSCMODE(vp)->m_font))
	kdv_enable(vp);
	if (Kdws.w_timeid) {
		untimeout(Kdws.w_timeid);
		Kdws.w_timeid = 0;
	}
	ws_rstmkbrk(Kdws.w_qp, &Kdws.w_mp, kbp->kb_state, (kbp->kb_state & NONTOGGLES));
	ws_kbtime(&Kdws);
	kdkb_cmd(LED_WARN);
	Kdws.w_flags &= ~WS_NOMODESW;
	wakeup(&Kdws.w_flags);
	splx(oldpri);
	return(1);
}

/*
 *
 */

kdvt_ioctl(chp, cmd, arg, crp, rvalp)
channel_t	*chp;
int	cmd,
	arg;
struct cred	*crp;
int	*rvalp;
{
	channel_t	*newchp;
	int	rv = 0, oldpri, retval, cnt;
	struct vt_mode	vtmode;
	struct vt_stat	vtinfo;
	struct proc	*procp;

	switch (cmd) {
	case VT_GETMODE:
		vtmode.mode = ws_procmode(&Kdws, chp) ? VT_PROCESS : VT_AUTO;
		vtmode.waitv = CHNFLAG(chp, CHN_WAIT) ? 1 : 0;
		vtmode.relsig = chp->ch_relsig;
		vtmode.acqsig = chp->ch_acqsig;
		vtmode.frsig = chp->ch_frsig;
		if (copyout(&vtmode, arg, sizeof(vtmode)) < 0)
			rv = EFAULT;
		break;
	case VT_SETMODE:
		if (copyin(arg, &vtmode, sizeof(vtmode)) < 0) {
			rv = EFAULT;
			break;
		}
		if (vtmode.mode == VT_PROCESS) {
			unsigned long pid;
			if (!ws_procmode(&Kdws,chp)) {
				drv_getparm(UPROCP, &chp->ch_procp);
				drv_getparm(PPID, &pid);
				chp->ch_pid = (pid_t)pid;
				chp->ch_flags |= CHN_PROC;
			}
		} else if (vtmode.mode == VT_AUTO) {
			oldpri = splstr();
			if (CHNFLAG(chp, CHN_PROC)) {
				ws_automode(&Kdws, chp);
				chp->ch_procp = (struct proc *)0;
				chp->ch_pid = 0;
				chp->ch_flags &= ~CHN_PROC;
			}
			splx(oldpri);
		} else {
			rv = EINVAL;
			break;
		}
		if (rv == EINVAL)
			break;
		if (vtmode.waitv)
			chp->ch_flags |= CHN_WAIT;
		else
			chp->ch_flags &= ~CHN_WAIT;
		if (vtmode.relsig) {
			if (vtmode.relsig < 0 || vtmode.relsig >= NSIG) {
				rv = EINVAL;
				break;
			} else
				chp->ch_relsig = vtmode.relsig;
		}
		if (vtmode.acqsig) {
			if (vtmode.acqsig < 0 || vtmode.acqsig >= NSIG) {
				rv = EINVAL;
				break;
			} else
				chp->ch_acqsig = vtmode.acqsig;
		}
		if (vtmode.frsig) {
			if (vtmode.frsig < 0 || vtmode.frsig >= NSIG) {
				rv = EINVAL;
				break;
			} else
				chp->ch_frsig = vtmode.frsig;
		}
		break;
	case VT_RELDISP:
		if ((int)arg == VT_ACKACQ) {
			Kdws.w_noacquire = 0;
			if (CHNFLAG(chp, CHN_ACTV)) {
				untimeout(chp->ch_timeid);
				chp->ch_timeid = 0;
			}
			break;
		}
		if (!Kdws.w_noacquire) {
			if (chp != ws_activechan(&Kdws)) {
				rv = EACCES;
				break;
			}
			if (!Kdws.w_switchto) {
				rv = EINVAL;
				break;
			}
		}
		/* VDC800 hook */
		if(kd_vdc800.procp != (struct proc *) 0){
			kdvt_rel_refuse();
			rv = EACCES;
			break;
		}
		oldpri = splstr();
		if (arg && ws_switch(&Kdws, Kdws.w_switchto, VT_NOFORCE)) {
			splx(oldpri);
			break;
		}
		kdvt_rel_refuse();
		splx(oldpri);
		if (arg) rv = EBUSY;
		break;
	case VT_ACTIVATE:
		if (!(newchp = ws_getchan(&Kdws, arg))) {
			rv = ENXIO;
			break;
		}
		if (!newchp->ch_opencnt) {
			rv = ENXIO;
			break;
		}
		ws_activate(&Kdws, newchp, VT_NOFORCE);
		break;
	case VT_WAITACTIVE:
		if (ws_procmode(&Kdws, chp) || chp == ws_activechan(&Kdws))
			break;
		oldpri = splstr();
		chp->ch_flags |= CHN_WACT;
		sleep(&chp->ch_flags, TTOPRI);
		splx(oldpri);
		break;
	case VT_GETSTATE:
		if ((newchp = ws_activechan(&Kdws)))
			vtinfo.v_active = newchp->ch_id;
		vtinfo.v_state = 0;
		newchp = chp;
		do {
			vtinfo.v_state |= (1 << newchp->ch_id);
			newchp = newchp->ch_nextp;
		} while (newchp != chp);
		if (copyout(&vtinfo, arg, sizeof(vtinfo)) < 0)
			rv = EFAULT;
		break;
	case VT_SENDSIG:
		if (copyin(arg, &vtinfo, sizeof(vtinfo)) < 0) {
			rv = EFAULT;
			break;
		}
		for (cnt = 0; cnt < WS_MAXCHAN; cnt++) { 
			if (!(vtinfo.v_state & (1 << cnt)))
				continue;
			if (!(newchp = ws_getchan(&Kdws, cnt)))
				continue;
			if (!newchp->ch_opencnt)
				continue;
			/* signal process group of channel */
			if (!newchp->ch_qp) {
				cmn_err(CE_NOTE,"Kdvt: warning: found no queue pointer for integral VT %d",cnt);
				continue;
			}
			putctl1(newchp->ch_qp->q_next, M_SIG, vtinfo.v_signal);
		}
		break;
	default:
		rv = ENXIO;
	}
	return(rv);
}

/*
 *
 */

kdvt_rel_refuse()
{
	int	oldpri;

	oldpri = splhi();
	if (Kdws.w_switchto)
		Kdws.w_switchto = (channel_t *)NULL;
	kdkb_tone();
	splx(oldpri);
}

/*
 * the timeout set for a process mode VT to obtain the VT has expired. We leave the
 * user in the ''limbo'' state of having the process mode VT be the active VT.
 * This way, if it does respond with a VT_RELDISP/ACQACK ioctl, the process will
 * indeed own the VT. If the process is still alive, this should happen.
 * If the process is dead, the next attempt to switch will detect that it is dead
 * and the user will be allowed to switch out. Otherwise, there is VT-FORCE.
 */

kdvt_acq_refuse(chp)
channel_t	*chp;
{
	if (!Kdws.w_noacquire)
		return;
	Kdws.w_switchto = (channel_t *)NULL;
	chp->ch_timeid = 0;
	Kdws.w_noacquire = 0;
	kdvt_rel_refuse();
}
