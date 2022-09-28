/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/p1.h	55.1"
/* p1.h */

/* Include pass 1 include files in the proper order.
** Since types.h and sym.h are potentially circular (symbol table
** contains types, type table contains symbol indices), define
** symbol index and type word here.
*/

#include "host.h"

typedef int SX;				/* symbol index */

#define	CG

#ifdef	__STDC__
#include "stdlib.h"
#else
#include "ansisup.h"
#endif

#ifndef	MACDEFS_H
#include "macdefs.h"
#define MACDEFS_H
#endif
#include "manifest.h"
#include <mddefs.h>			/* ANSI C-specific machine-dependent
					** definitions.  Use one found by
					** search path, rather than current
					** directory.
					*/
#ifdef LINT
#undef FAT_ACOMP			/* lint always works statement-at-a-
					** time
					*/
#endif

/* typedef SIZE T1WORD; */		/* type word:  index in type table */
typedef TWORD T1WORD;			/* make type word consistent with CG */

#ifndef LINT2

#include "tblmgr.h"
#include "types.h"
#include "sym.h"
#include "decl.h"
#include "err.h"
#include "node.h"
#include "trees.h"
#include "stmt.h"
#include "cgstuff.h"
#include "optim.h"
#include "aclex.h"
#include "lexsup.h"
#include "target.h"
#include "debug.h"
#include "init.h"
#include "p1allo.h"
#ifdef LINT
#include "lint.h"
#include "msgbuf.h"
#endif  /* LINT */

#define	ND1NIL	((ND1 *) 0)		/* Pass 1 NIL pointer */

/* Pass 1 debug flags */
extern int a1debug;			/* offset/register allocation */
extern int b1debug;			/* tree building */
extern int d1debug;			/* declaration processing */
extern int i1debug;			/* initialization processing */
extern int o1debug;			/* tree optimization */
extern int s1debug;			/* print source line */
extern int version;			/* language version: */
#define	V_CI4_1		(1<<0)		/* Compatible with C Issue 4.1 */
#define	V_ANSI		(1<<1)		/* Conforming ANSI compiler (choose
					**	ANSI interpretation of conflicts)
					*/
#define	V_STD_C		(1<<2)		/* Strictly conforming ANSI C */
extern int verbose;			/* produce warnings on dubious code */

/* CG debug flags.  These better match CG's ideas of the names. */
extern int odebug;			/* print pass 2 trees */
extern int sdebug;			/* print tree matching info */
extern int asmdebug;			/* print asm debug information */


#else
#include "msgbuf.h"
#endif /* LINT2 */
