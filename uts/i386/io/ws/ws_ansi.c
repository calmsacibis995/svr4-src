/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ws/ws_ansi.c	1.3.1.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#include "sys/inline.h"
#include "sys/tty.h"
#include "sys/termio.h"
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

/*
 *
 */

wsansi_parse(wsp, chp, addr, cnt)
wstation_t	*wsp;
channel_t	*chp;
unchar	*addr;
int	cnt;
{
	termstate_t	*tsp = &chp->ch_tstate;
	register ushort	ch; 
	register int	i;

	while (cnt--) {
		ch = *addr++ & 0xFF; 
		if (ch == A_ESC || ch == A_CSI || (ch < ' ' && tsp->t_font == ANSI_FONT0))
			wsansi_cntl(wsp, chp, tsp, ch);
		else
			tcl_norm(wsp, chp, tsp, ch);
	}  /* while (cnt--) */
}

/*
 *
 */

wsansi_cntl(wsp, chp, tsp, ch)
wstation_t	*wsp;
channel_t	*chp;
termstate_t	*tsp;
ushort	ch;
{
	switch (ch) {
	case A_BEL:
		(*wsp->w_bell)(wsp, chp);
		break;
	case A_BS:
		tcl_bs(wsp, chp, tsp);
		break;
	case A_HT:
		tcl_ht(wsp, chp, tsp);
		break;
	case A_NL:
	case A_VT:
		if (tsp->t_row == tsp->t_rows - 1) {
			tsp->t_ppar[0] = 1;
			tcl_scrlup(wsp, chp, tsp);
		} else {
			tsp->t_row++;
			tsp->t_cursor += tsp->t_cols;
		}
		(*wsp->w_setcursor)(chp, tsp);
		break;
	case A_FF:
		tcl_reset(wsp, chp, tsp);
		break;
	case A_CR:
		tsp->t_cursor -= tsp->t_col;
		tsp->t_col = 0;
		(*wsp->w_setcursor)(chp, tsp);
		break;
	case A_GS:
		tcl_bht(wsp, chp, tsp);
		break;
	default:
		break;
	}
}

