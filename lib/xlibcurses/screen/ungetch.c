/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/ungetch.c	1.4"
#include "curses_inc.h"

/* Place a char onto the beginning of the input queue. */

ungetch(ch)
int	ch;
{
    register	int	i = cur_term->_chars_on_queue, j = i - 1;
    register	short	*inputQ = cur_term->_input_queue;

    /* Place the character at the beg of the Q */

    while (i > 0)
	inputQ[i--] = inputQ[j--];
    cur_term->_ungotten++;
    inputQ[0] = -ch - 0100;
    cur_term->_chars_on_queue++;
}
