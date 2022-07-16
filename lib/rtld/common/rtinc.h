/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rtld:common/rtinc.h	1.2"

/* global macros - allow us to use ANSI or non-ANSI compilation */

/*
 * CONST is for stuff we know is constant.
 * VOID is for use in void* things.
*/
#ifdef	__STDC__
#define	CONST	const
#define	VOID	void
#else	
#define	CONST	/* empty */
#define	VOID	char
#endif

/*
 * ARGS(t) allows us to use function prototypes for
 * ANSI C and not use them for old C.
 * Example:
 *   void fido ARGS((int bone, unsigned fetch));
 * which expands to
 *   void fido (int bone, unsigned fetch);   in ANSI C
 * and
 *   void fido ();  in old C.
 * NOTE the use of two sets of parentheses.
*/
#ifdef	__STDC__
#define	ARGS(t)	t
#else	
#define	ARGS(t) ()
#endif	

/* make sure we get all commonly needed include files */
#include <sys/types.h>
#include "machdep.h"
#include <elf.h>
#include <link.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include "rtld.h"
#include "externs.h"
#include "dllib.h"

#ifdef	DEBUG
/*
 * Print a debugging message
 * Usage: DPRINTF(DBG_MAIN, (MSG_DEBUG, "I am here"));
 */
#define	DPRINTF(D,M)	if (_debugflag & (D)) _rtfprintf M
#else	/* DEBUG */
#define	DPRINTF(D,M)
#endif	/* DEBUG */
