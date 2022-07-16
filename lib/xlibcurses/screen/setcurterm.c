/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/setcurterm.c	1.5"
#include "curses_inc.h"

/*
 * Establish the terminal that the #defines in term.h refer to.
 */

TERMINAL *
setcurterm(newterminal)
register TERMINAL *newterminal;
{
    register	TERMINAL	*oldterminal = cur_term;

    if (newterminal)
    {
#ifdef	_VR3_COMPAT_CODE
	acs_map = cur_term->_acs32map;
#else	/* _VR3_COMPAT_CODE */
	acs_map = cur_term->_acsmap;
#endif	/* _VR3_COMPAT_CODE */
	cur_bools = newterminal->_bools;
	cur_nums = newterminal->_nums;
	cur_strs = newterminal->_strs;
	cur_term = newterminal;
    }
    return (oldterminal);
}
