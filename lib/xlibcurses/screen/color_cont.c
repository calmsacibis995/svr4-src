/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:color_cont.c	1.3.1.1"

#include "curses_inc.h"

color_content(color, r, g, b)
short  color;
short  *r, *g, *b;
{
    register _Color *ctp;

    if (color < 0 || color > COLORS || !can_change ||
	(ctp = cur_term->_color_tbl) == (_Color *) NULL)
        return (ERR);

    ctp += color;
    *r = ctp->r;
    *g = ctp->g;
    *b = ctp->b;
    return (OK);
}
