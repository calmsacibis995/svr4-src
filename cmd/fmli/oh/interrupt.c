/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1988 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:oh/interrupt.c	1.2"

#include "wish.h"

/*  routines defined in this file: */

void intr_handler();



/* declare data structures for interrupt  feature
 * the following is a copy of interrupt.h without the `extern'
 */


struct {
    bool  interrupt;
    char *oninterrupt;
    bool  skip_eval;
} Cur_intr;


/* intr_handler
          This routine will execute  on receipt of a SIGINT while
	  executing a fmli builtin  or external executable (if
	  interrupts are enabled)
	  
	  Sets flag  to tell eval() to throw away the rest of the
	  descriptor it is parsing.
*/
void
intr_handler()
{
    Cur_intr.skip_eval = TRUE;
}

