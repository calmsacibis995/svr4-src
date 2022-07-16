/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/slk_label.c	1.2"
#include	"curses_inc.h"

/* Return the current label of key number 'n'. */

char *
slk_label(n)
int	n;
{
    register	SLK_MAP	*slk = SP->slk;

    /* strip initial blanks */
    /* for (; *lab != '\0'; ++lab)
	if(*lab != ' ')
	    break; */
    /* strip trailing blanks */
    /* for (; cp > lab; --cp)
	if (*(cp-1) != ' ')
	    break; */


    return (!slk || n < 1 || n > slk->_num) ? NULL : slk->_lval[n - 1];
}
