/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ws/ws_tcl.c	1.3"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#include "sys/inline.h"
#include "sys/tty.h"
#include "sys/termios.h"
#include "sys/cmn_err.h"
#include "sys/vt.h"
#include "sys/at_ansi.h"
#include "sys/ascii.h"
#include "sys/kd.h"
#include "sys/stream.h"
#include "sys/strtty.h"
#include "sys/stropts.h"
#include "sys/proc.h"
#include "sys/xque.h"
#include "sys/ws/ws.h"
#include "sys/ws/chan.h"
#include "sys/ws/tcl.h"

extern channel_t *ws_activechan();

/*
 *
 */

void
tcl_handler(wsp, mp, tsp, chp)
register wstation_t *wsp;
mblk_t *mp;
termstate_t *tsp;
channel_t *chp;
{
	register ch_proto_t *protop;
	mblk_t *bp;

	protop = (ch_proto_t *) mp->b_rptr;
	switch (protop->chp_stype_cmd) {

	case TCL_CURS_TYP: {
		ushort type;
		type = protop->chp_stype_arg;
		if (type == tsp->t_curtyp)
			break;
		tsp->t_curtyp = type;
		(*wsp->w_cursortype)(wsp, chp, tsp);
		break;
	  }

	case TCL_BELL:
		(*wsp->w_bell)(wsp, chp);
		break;

	case TCL_BACK_SPCE:
		tcl_bs(wsp, chp, tsp);
		break;

	case TCL_NEWLINE:
	case TCL_V_TAB:
		if (tsp->t_row == tsp->t_rows - 1) {
			tsp->t_ppar[0] = 1;
			tcl_scrlup(wsp, chp, tsp);
		}
		else {
			tsp->t_row++;
			tsp->t_cursor += tsp->t_cols;
		}
		(*wsp->w_setcursor)(chp, tsp);
		break;

	case TCL_CRRGE_RETN:
		tsp->t_cursor -= tsp->t_col;
		tsp->t_col = 0;
		(*wsp->w_setcursor)(chp, tsp);
		break;

	case TCL_H_TAB:
		tcl_ht(wsp, chp, tsp);
		break;

	case TCL_BACK_H_TAB:
		tcl_bht(wsp, chp, tsp);
		break;

	case TCL_KEYCLK_ON:
		wsp->w_flags |= WS_KEYCLICK;
		break;

	case TCL_KEYCLK_OFF:
		wsp->w_flags &= ~WS_KEYCLICK;
		break;

	case TCL_CLR_TAB:
		tsp->t_ppar[0] = 0;
		tcl_clrtabs(tsp);
		break;

	case TCL_CLR_TABS:
		tsp->t_ppar[0] = 3;
		tcl_clrtabs(tsp);
		break;

	case TCL_SET_TAB:
		tcl_settab(tsp);
		break;

	case TCL_SHFT_FT_OU:
		(*wsp->w_shiftset)(wsp, chp, 0); 
		break;

	case TCL_SHFT_FT_IN:
		(*wsp->w_shiftset)(wsp, chp, 1); 
		break;

	case TCL_SCRL_UP:
		tsp->t_ppar[0] = (ushort) protop->chp_stype_arg;
		tcl_scrlup(wsp, chp, tsp);
		break;

	case TCL_SCRL_DWN:
		tsp->t_ppar[0] = (ushort) protop->chp_stype_arg;
		tcl_scrldn(wsp, chp, tsp);
		break;


	case TCL_SEND_SCR:
		tcl_sendscr(wsp,chp,tsp);
		break;

	case TCL_LCK_KB:
		wsp->w_flags |= WS_LOCKED;
		break;
		
	case TCL_UNLCK_KB:
		wsp->w_flags &= ~WS_LOCKED;
		break;

	case TCL_SET_ATTR:
		tsp->t_ppar[0] = (ushort) protop->chp_stype_arg;
		tsp->t_pnum = 1;
		tcl_sfont(wsp, chp, tsp);
		break;

	case TCL_POS_CURS: {
		tcl_data_t *dp;
		short x, y;

		if ( (bp = mp->b_cont) == (mblk_t *) NULL) {
			cmn_err(CE_WARN, "kd_do_tcl: unexpected end of msg");
			return;
		}

		dp = (tcl_data_t *)bp->b_rptr;

		x = dp->mv_curs.delta_x;
		y = dp->mv_curs.delta_y;

		if (dp->mv_curs.x_type == TCL_POSABS)
			tsp->t_col = x;
		else
			tsp->t_col += x;

		if (dp->mv_curs.y_type == TCL_POSABS)
			tsp->t_row = y;
		else
			tsp->t_row += y;

		if (tsp->t_col < 0) tsp->t_col = 0;
		if (tsp->t_row < 0) tsp->t_row = 0;
		
		tcl_cursor(wsp, chp);
		break;
	   } /* TCL_POS_CURS */

	case TCL_DISP_CLR:
		tsp->t_ppar[0] = (ushort) protop->chp_stype_arg;
		(*wsp->w_clrscr)(chp, tsp->t_cursor, tsp->t_ppar[0]);
		break;

	case TCL_DISP_RST:
		tcl_reset(wsp, chp, tsp);
		break;

	case TCL_ERASCR_CUR2END:
		tsp->t_ppar[0] = 0;
		tcl_escr(wsp, chp, tsp);
		break;

	case TCL_ERASCR_BEG2CUR:
		tsp->t_ppar[0] = 1;
		tcl_escr(wsp, chp, tsp);
		break;

	case TCL_ERASCR_BEG2END:
		tsp->t_ppar[0] = 2;
		tcl_escr(wsp, chp, tsp);
		break;

	case TCL_ERALIN_CUR2END:
		tsp->t_ppar[0] = 0;
		tcl_eline(wsp, chp, tsp);
		break;

	case TCL_ERALIN_BEG2CUR:
		tsp->t_ppar[0] = 1;
		tcl_eline(wsp, chp, tsp);
		break;

	case TCL_ERALIN_BEG2END:
		tsp->t_ppar[0] = 2;
		tcl_eline(wsp, chp, tsp);
		break;

	case TCL_INSRT_LIN:
		tsp->t_ppar[0] = (ushort)protop->chp_stype_arg;
		tcl_iline(wsp, chp, tsp);
		break;

	case TCL_DELET_LIN:
		tsp->t_ppar[0] = (ushort)protop->chp_stype_arg;
		tcl_dline(wsp, chp, tsp);
		break;

	case TCL_INSRT_CHR:
		tsp->t_ppar[0] = (ushort)protop->chp_stype_arg;
		tcl_ichar(wsp, chp, tsp);
		break;

	case TCL_DELET_CHR:
		tsp->t_ppar[0] = (ushort)protop->chp_stype_arg;
		tcl_dchar(wsp, chp, tsp);
		break;

	default:
		cmn_err(CE_WARN, "ws: do not understand TCL %d",protop->chp_stype_cmd);
		break;
	}
}

/*
 *
 */

tcl_bs(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
termstate_t	*tsp;
{
	if (!tsp->t_col)
		return;
	tsp->t_col--;
	tsp->t_cursor--;
	(*wsp->w_setcursor)(chp, tsp);
}

/*
 *
 */

tcl_ht(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
termstate_t	*tsp;
{
	register int	tabindex = 0;
	register ushort	tabval;

	do {
		tabval = tsp->t_tabsp[tabindex];
		if (++tabindex >= (int) tsp->t_ntabs)
			tabval = tsp->t_cols;
	} while (tabval <= tsp->t_col);
	if (tabval >= tsp->t_cols) {
		if (tsp->t_row == (tsp->t_rows - 1)) {
			tsp->t_ppar[0] = 1;
			tcl_scrlup(wsp, chp, tsp);
			tsp->t_cursor -= tsp->t_col;
		} else {
			tsp->t_row++;
			tsp->t_cursor += tsp->t_cols - tsp->t_col;
		}
		tsp->t_col = 0;
	} else {
		tabval -= tsp->t_col;
		tsp->t_cursor += tabval;
		tsp->t_col += tabval;
	}
	(*wsp->w_setcursor)(chp, tsp);
}

/*
 *
 */

tcl_reset(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
termstate_t	*tsp;
{
	tsp->t_origin = 0;
	tsp->t_curattr = tsp->t_normattr;
	(*wsp->w_clrscr)(chp, tsp->t_origin, tsp->t_scrsz);
	tsp->t_row = 0; /* clear all state variables */
	tsp->t_col = 0;
	tsp->t_cursor = 0;
	tsp->t_pstate = 0;
	(*wsp->w_setbase)(chp, tsp);
	(*wsp->w_setcursor)(chp, tsp);
}

/*
 *
 */

tcl_bht(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
termstate_t	*tsp;
{
	register int	tabindex = 9;
	register ushort	tabval;

	do {
		if (--tabindex < 0) {
			tabval = 0;
			break;
		}
		tabval = tsp->t_tabsp[tabindex];
	} while (tabval >= tsp->t_col);
	if (tabval < tsp->t_tabsp[0]) {
		tsp->t_cursor -=  tsp->t_col;
		tsp->t_col = 0;
	} else {
		tabval = tsp->t_col - tabval;
		tsp->t_cursor -= tabval;
		tsp->t_col -= tabval;
	}
	(*wsp->w_setcursor)(chp, tsp);
}

/*
 *
 */

tcl_norm(wsp, chp, tsp, ch)
wstation_t	*wsp;
channel_t	*chp;
termstate_t	*tsp;
ushort	ch;
{
	ushort	dch;	/* "display" character (character plus attribute) */

	dch = ((tsp->t_curattr << 8) | ch);
        (*wsp->w_stchar)(chp, tsp->t_cursor, dch, 1);
	if (tsp->t_col == tsp->t_cols - 1) {
		if (tsp->t_row == tsp->t_rows - 1) {
			tsp->t_ppar[0] = 1;
			tcl_scrlup(wsp, chp, tsp);
			tsp->t_cursor -= tsp->t_col;
		} else {
			tsp->t_row++;
			tsp->t_cursor++;
		}
		tsp->t_col = 0;
	} else {
		tsp->t_col++;
		tsp->t_cursor++;
	}
	(*wsp->w_setcursor)(chp, tsp);
}

/*
 *
 */

tcl_cursor(wsp, chp)
wstation_t	*wsp;
channel_t	*chp;
{
	register ushort	row, col;
	termstate_t	*tsp = &chp->ch_tstate;

	row = tsp->t_row;
	col = tsp->t_col;
	while (row < 0)
		row += tsp->t_rows;
	while (row >= tsp->t_rows)
		row -= tsp->t_rows;
	tsp->t_row = row;
	while (col < 0)
		col += tsp->t_cols;
	while (col >= tsp->t_cols)
		col -= tsp->t_cols;
	tsp->t_col = col;
	tsp->t_cursor = row * tsp->t_cols + col + tsp->t_origin;
	(*wsp->w_setcursor)(chp, tsp);
}

/*
 *
 */

tcl_scrlup(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
termstate_t	*tsp;
{
	if (tsp->t_flags & T_ANSIMVBASE) {
		register ushort	last;
		register int	count;

		count = tsp->t_cols * tsp->t_ppar[0];
		last = tsp->t_origin + tsp->t_scrsz;
		tsp->t_origin += count;
		(*wsp->w_clrscr)(chp, last, count);
		(*wsp->w_setbase)(chp, tsp);
		tcl_cursor(wsp, chp);
	} else {
		register ushort	row, col, cursor;

		row = tsp->t_row;
		col = tsp->t_col;
		cursor = tsp->t_cursor;
		tsp->t_cursor = tsp->t_origin;
		tsp->t_row = tsp->t_col = 0;
		tcl_dline(wsp, chp, tsp);
		tsp->t_row = row;
		tsp->t_col = col;
		tsp->t_cursor = cursor;
	}
}

/*
 *
 */

tcl_scrldn(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	short	row, col;
	ushort	cursor;

	if (tsp->t_flags & T_ANSIMVBASE) {
		register int	count;

		count = tsp->t_cols * tsp->t_ppar[0];
		tsp->t_origin -= count;
		(*wsp->w_clrscr)(chp, tsp->t_origin, count);
		(*wsp->w_setbase)(chp, tsp);
		tcl_cursor(wsp, chp);
	} else {
		/*
		 * Save row, col, and cursor.
		 */
		row = tsp->t_row;
		col = tsp->t_col;
		cursor = tsp->t_cursor;
		/*
		 * Get tcl_iline to do the work.
		 */
		tsp->t_cursor = tsp->t_origin;
		tsp->t_row = tsp->t_col = 0;
		tcl_iline(wsp, chp, tsp);
		/*
		 * Restore row, col, and cursor.
		 */
		tsp->t_row = row;
		tsp->t_col = col;
		tsp->t_cursor = cursor;
	}
}

/*
 *
 */

tcl_dchar(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	register ushort	from, last;
	register int	cnt;

	if (!tcl_ckchar(wsp, chp, tsp))
		return;
	from = tsp->t_cursor + tsp->t_ppar[0];
	cnt = tsp->t_cols - tsp->t_col - tsp->t_ppar[0];
	last = tsp->t_cursor - tsp->t_col + tsp->t_cols - tsp->t_ppar[0];
	(*wsp->w_mvword)(chp, from, tsp->t_cursor, cnt, DOWN);
	(*wsp->w_clrscr)(chp, last, tsp->t_ppar[0]);
}

/*
 *
 */

tcl_dline(wsp, chp, tsp) 
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	register ushort	from, to, last;
	register int	cnt;

	if (!tcl_ckline(wsp, chp, tsp))
		return;
	to = tsp->t_cursor - tsp->t_col;
	from = to + tsp->t_ppar[0] * tsp->t_cols;
	cnt = (tsp->t_rows - tsp->t_row - tsp->t_ppar[0]) * tsp->t_cols;
	(*wsp->w_mvword)(chp, from, to, cnt, DOWN);
	last = to + cnt;
	cnt = tsp->t_ppar[0] * tsp->t_cols;
	(*wsp->w_clrscr)(chp, last, cnt);
}

/*
 *
 */

tcl_iline(wsp, chp, tsp) 
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	register ushort	from, to, last;
	register int	cnt;

	if (!tcl_ckline(wsp, chp, tsp))
		return;
	from = tsp->t_cursor - tsp->t_col;
	to = from + tsp->t_ppar[0] * tsp->t_cols;
	cnt = (tsp->t_rows - tsp->t_row - tsp->t_ppar[0]) * tsp->t_cols;
	(*wsp->w_mvword)(chp, from, to, cnt, UP);
	(*wsp->w_clrscr)(chp, from, tsp->t_ppar[0] * tsp->t_cols);
}

/*
 *
 */

tcl_eline(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	register ushort	last = tsp->t_cursor - tsp->t_col;

	switch (tsp->t_ppar[0]) {
	case 0:	/* erase cursor to end of line */
		(*wsp->w_clrscr)(chp, tsp->t_cursor, tsp->t_cols - tsp->t_col);
		break;

	case 1:	/* erase beginning of line to cursor */
		(*wsp->w_clrscr)(chp, last, tsp->t_col + 1);
		break;

	default:	/* erase whole line */
		(*wsp->w_clrscr)(chp, last, tsp->t_cols);
	}
}

/*
 *
 */

tcl_escr(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	int	cnt = tsp->t_cursor - tsp->t_origin;

	switch (tsp->t_ppar[0]) {
	case 0:	/* erase cursor to end of screen */
		(*wsp->w_clrscr)(chp, tsp->t_cursor, tsp->t_scrsz - cnt);
		break;
	case 1:	/* erase beginning of screen to cursor */
		(*wsp->w_clrscr)(chp, tsp->t_origin, (cnt + 1));
		break;
	default:	/* erase whole screen */
		(*wsp->w_clrscr)(chp, tsp->t_origin, tsp->t_scrsz);
		break;
	}
}

/*
 *
 */

tcl_clrtabs(tsp)
register termstate_t	*tsp;
{
	register int	cnt1, cnt2;

	if (tsp->t_ppar[0] == 3) { /* clear all tabs */
		tsp->t_ntabs = 0;
		return;
	}
	if (tsp->t_ppar[0] || !tsp->t_ntabs)
		return;
	for (cnt1 = 0; cnt1 < (int) tsp->t_ntabs; cnt1++) {
		if (tsp->t_tabsp[cnt1] == tsp->t_col) {
			for (cnt2 = cnt1; cnt2 < (int) tsp->t_ntabs - 1; cnt2++)
				tsp->t_tabsp[cnt2] = tsp->t_tabsp[cnt2 + 1];
			tsp->t_ntabs--;
			return;
		}
	}
}

/*
 *
 */

tcl_ichar(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	register ushort to, cnt;

	to = tsp->t_cursor + tsp->t_ppar[0];
	cnt = tsp->t_cols - tsp->t_col - tsp->t_ppar[0];

	if (!tcl_ckchar(wsp, chp, tsp))
		return;
	(*wsp->w_mvword)(chp, tsp->t_cursor, to, cnt, UP);
	(*wsp->w_clrscr)(chp, tsp->t_cursor, tsp->t_ppar[0]);
}

/*
 *
 */

tcl_sparam(tsp, paramcnt, newparam)
register termstate_t	*tsp;
register int	paramcnt;
register ushort	newparam;
{
	register int	cnt;

	for (cnt = 0; cnt < paramcnt; cnt++) {
		if (tsp->t_ppar[cnt] == -1)
			tsp->t_ppar[cnt] = newparam;
	}
}

/*
 *
 */

tcl_sfont(wsp, chp, tsp) 
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	register ushort	parnum = tsp->t_pnum;
	register int	cnt = 0;
	ushort	curattr = tsp->t_curattr,
		param;

	do {
		param = tsp->t_ppar[cnt];
		if (param < tsp->t_nattrmsk) {
			if (param == -1) 
				param = 0;
			curattr &= tsp->t_attrmskp[param].mask;
			curattr |= tsp->t_attrmskp[param].attr;
			switch (param) {
			case 5:
			case 6:
			case 4:
			case 38:
			case 39:
				(*wsp->w_undattr)(wsp, chp, &curattr, param);
				break;
			case 10:
				tsp->t_font = ANSI_FONT0;
				(*wsp->w_shiftset)(wsp, chp, 0); 

				break;
			case 11:
				tsp->t_font = ANSI_FONT1;
				break;
			case 12:
				tsp->t_font = ANSI_FONT2;
				break;
			default:
				break;
			}
		}
		cnt++;
		parnum--;
	} while (parnum > 0);
	tsp->t_curattr = curattr;
	tsp->t_pstate = 0;
}

/*
 *
 */

tcl_ckchar(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	register ushort	avail;

	if (!tsp->t_ppar[0])
		return(0);
	avail = tsp->t_cols - tsp->t_col;
	if (tsp->t_ppar[0] <= avail)
		return(1);
	/* if there are too many chars, erase them */	
	(*wsp->w_clrscr)(chp, tsp->t_cursor, avail);
	return(0);
}

/*
 *
 */

tcl_ckline(wsp, chp, tsp)
wstation_t	*wsp;
channel_t	*chp;
register termstate_t	*tsp;
{
	register ushort avail;

	if (!tsp->t_ppar[0])
		return(0);
	avail = tsp->t_rows - tsp->t_row;
	if (tsp->t_ppar[0] <= avail)
		return(1);
	/* if we have too many lines, erase them */	
	(*wsp->w_clrscr)(chp, tsp->t_cursor - tsp->t_col, avail * tsp->t_cols);
	return(0);
}

/*
 *
 */

tcl_settab(tsp)
register termstate_t	*tsp;
{
	register int	cnt1, cnt2;

	if (tsp->t_ntabs == 0) { /* no tabs set yet */
		tsp->t_tabsp[0] = tsp->t_col;
		tsp->t_ntabs++;
		return;
	}
	if (tsp->t_ntabs == ANSI_MAXTAB) /* no more tabs can be set */
		return;
	for (cnt1 = 0; cnt1 < (int) tsp->t_ntabs; cnt1++)
		if (tsp->t_tabsp[cnt1] >= tsp->t_col)
			break;
	if (tsp->t_tabsp[cnt1] == tsp->t_col)
		return; /* tab already set here */
	for (cnt2 = tsp->t_ntabs; cnt2 > cnt1; cnt2--)
		tsp->t_tabsp[cnt2] = tsp->t_tabsp[cnt2 - 1];
	tsp->t_tabsp[cnt1] = tsp->t_col;
	tsp->t_ntabs++;
}

/*
 *
 */

tcl_curattr(chp)
channel_t	*chp;
{
	termstate_t	*tsp = &chp->ch_tstate;

	return((int)tsp->t_curattr);
}

void
tcl_scrolllock(wsp, chp, arg)
register wstation_t *wsp;
channel_t *chp;
int arg;
{
	kbstate_t *kbp;
	mblk_t *tmp;
	ch_proto_t *protop;

	kbp = &chp->ch_kbstate;
	if (arg == TCL_FLOWON)
		kbp->kb_state |= SCROLL_LOCK;
	else
		kbp->kb_state &= ~SCROLL_LOCK;

	if ( (chp == ws_activechan(wsp)) && wsp->w_scrllck) 
		(*wsp->w_scrllck)(wsp, chp);

	if (!(tmp = allocb(sizeof(ch_proto_t), BPRI_HI)))
		return;

	tmp->b_datap->db_type = M_PROTO;
	tmp->b_wptr += sizeof(ch_proto_t);
	protop = (ch_proto_t *)tmp->b_rptr;
	protop->chp_type = CH_CTL;
	protop->chp_stype = CH_CHR;
	protop->chp_stype_cmd = CH_LEDSTATE;
	protop->chp_stype_arg = (kbp->kb_state & ~NONTOGGLES);
	putnext(chp->ch_qp, tmp);
}

/*
 * Send contents of screen to user
 */
tcl_sendscr(wsp,chp,tsp)
wstation_t *wsp;
channel_t	*chp;
termstate_t	*tsp;
{
	struct termios	*termp;		/* Terminal structure */
	unchar		ch;		/* Character */
	register ushort	*cp;		/* Pointer to characters */
	mblk_t	*bp, *bp1, *cbp, *mp, *mp1;
	struct iocblk	*iocp;
	vidstate_t	*vp;
	ch_proto_t *protop;
	ushort rows=0,cols = 0;

	/* disable echo by telling LDTERM that we'll do it (we won't, of course,
	 * because we want ECHO disabled). Generate an M_CTL message for
	 * LDTERM that tells it we'll do the canonical processing for echo.
	 */
	if ( (bp = allocb(sizeof(struct iocblk),BPRI_HI)) == (mblk_t *) NULL) {
		(*wsp->w_bell)(wsp, chp);
		return;
	}
	if ((bp1 = allocb(sizeof(struct termios),BPRI_HI)) == (mblk_t *) NULL) {
		(*wsp->w_bell)(wsp, chp);
		freeb(bp);
		return;
	}
	bp->b_wptr += sizeof(struct iocblk);
	iocp = (struct iocblk *)bp->b_rptr;
	bp->b_datap->db_type = M_CTL;
	iocp->ioc_cmd = MC_PART_CANON;
	bp->b_cont = bp1;
	bp1->b_wptr += sizeof(struct termios);
	termp = (struct termios *)bp1->b_rptr;
	termp->c_iflag = 0;
	termp->c_oflag = 0;
	termp->c_lflag = ECHO;
	if ( (cbp = copymsg(bp)) == (mblk_t *) NULL) {
		(*wsp->w_bell)(wsp, chp);
		freemsg(bp);
		return;
	}
	termp = (struct termios *)cbp->b_cont->b_rptr;
	termp->c_lflag = 0;
	if ((mp = allocb(sizeof(ch_proto_t),BPRI_HI)) == (mblk_t *) NULL) {
		(*wsp->w_bell)(wsp, chp);
		freemsg(bp);
		freemsg(cbp);
		return;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_wptr += sizeof (ch_proto_t);
	protop = (ch_proto_t *)mp->b_rptr;
	protop->chp_type = CH_DATA;
	protop->chp_stype = CH_NOSCAN;
	/* allocate a buffer to hold the current screen contents. Leave room
	 * for two characters for the NL -- hence t_cols+1)
	 */
	if ((mp1 = allocb(tsp->t_rows*(tsp->t_cols +1),BPRI_HI)) == (mblk_t *) NULL) {
		(*wsp->w_bell)(wsp, chp);
		freemsg(bp);
		freemsg(cbp);
		freeb(mp);
		return;
	}
	mp->b_cont = mp1;

	vp = &chp->ch_vstate;
	cp = vp->v_scrp + (tsp->t_origin & vp->v_scrmsk);
	while (rows < tsp->t_rows) {	/* For all rows */
		while (cols < tsp->t_cols) {	/* For all columns */
			*mp1->b_wptr++ = *cp++ & 0xff;	/* add char to msg */
			cols++;	/* Did one more column */
		}
		cols = 0;	/* Reset column count */
		*mp1->b_wptr++ = (unchar) '\n';	/* Add newline and flush */
		rows++;		/* Did one more row */
	}

	/* turn echo off */
	putnext(chp->ch_qp, bp);
	/* put the CH_NOSCAN message on the queue */
	putnext(chp->ch_qp, mp);
	/* turn echo back on */
	putnext(chp->ch_qp, cbp);
}
