/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:incdec.c	1.1.3.1"

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"
#include "ramdata.h"
#include "proto.h"

/*
 * Atomic operations for shared memory:
 * Increment or decrement a word in memory with one machine instruction.
 *
 * Although the window is small, encountering a forced process switch
 * in the middle of a read-alter-rewrite sequence could mess up the
 * multi-process coordination this supports.
 */

/*ARGSUSED*/
void
increment(p)
int * p;
{
#if u3b2 || u3b5 || u3b15 || u3b1500
	asm("MOVW    (%ap),%r0");
	asm("INCW    (%r0)");
#else
#if mc68k
	asm("mov.l	8(%fp),%a0");
	asm("add.l	&1,(%a0)");
#else
	(*p)++;
#endif
#endif
}

/*ARGSUSED*/
void
decrement(p)
int * p;
{
#if u3b2 || u3b5 || u3b15 || u3b1500
	asm("MOVW    (%ap),%r0");
	asm("DECW    (%r0)");
#else
#if mc68k
	asm("mov.l	8(%fp),%a0");
	asm("sub.l	&1,(%a0)");
#else
	--(*p);
#endif
#endif
}
