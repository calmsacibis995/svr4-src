#ident	"@(#)space.c	1.2	92/04/13	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)master:kdb-util/space.c	1.2"

#include "sys/kdebugger.h"
#include "config.h"

struct kdebugger *debuggers;

#if defined(AT386) && defined(ASY_0)
int asyputchar(), asygetchar();
static struct conssw asysw = {
	asyputchar,	0,	asygetchar
};
#endif

struct conssw *dbg_io[] = {
	&conssw,
#if defined(AT386) && defined(ASY_0)
	&asysw,
#endif
};

struct conssw **cdbg_io = dbg_io;
int ndbg_io = sizeof(dbg_io) / sizeof(struct conssw *);

char symtable[KDBSYMSIZE] = {0};
int kdb_symsize = KDBSYMSIZE;

/*
 * As a security feature, the kdb_security flag (set by the KDBSECURITY
 * tuneable) is provided.  If it is non-zero, the debugger should ignore
 * attempts to enter from a console key sequence.
 */
int kdb_security = KDBSECURITY;
