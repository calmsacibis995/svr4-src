/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpstat/class.c	1.5.3.1"

#include "stdio.h"

#include "lp.h"
#include "class.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


#if	defined(__STDC__)
static void		putcline ( CLASS * );
#else
static void		putcline();
#endif

/**
 ** do_class()
 **/

void
#if	defined(__STDC__)
do_class (
	char **			list
)
#else
do_class (list)
	char			**list;
#endif
{
	register CLASS		*pc;

	printlist_setup ("\t", 0, 0, 0);
	while (*list) {
		if (STREQU(NAME_ALL, *list))
			while ((pc = getclass(NAME_ALL)))
				putcline (pc);

		else if ((pc = getclass(*list)))
			putcline (pc);

		else {
			LP_ERRMSG1 (ERROR, E_LP_NOCLASS, *list);
			exit_rc = 1;
		}
		list++;
	}
	printlist_unsetup ();
	return;
}

/**
 ** putcline()
 **/

static void
#if	defined(__STDC__)
putcline (
	CLASS *			pc
)
#else
putcline (pc)
	register CLASS		*pc;
#endif
{
	(void) printf("members of class %s:\n", pc->name);
		printlist (stdout, pc->members);
	return;
}
