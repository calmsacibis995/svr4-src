/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/pnoutref.c	1.11"
#include	"curses_inc.h"

/* wnoutrefresh for pads. */

pnoutrefresh(pad, pby, pbx, sby, sbx, sey, sex)
WINDOW	*pad;
int	pby, pbx, sby, sbx, sey, sex;
{
    extern	int	wnoutrefresh();

    return (_prefresh(wnoutrefresh, pad, pby, pbx, sby, sbx, sey, sex));
}
