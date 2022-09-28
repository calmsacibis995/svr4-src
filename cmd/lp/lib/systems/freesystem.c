/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/systems/freesystem.c	1.3.3.1"
/* LINTLIBRARY */

# include	"lp.h"
# include	"systems.h"

/**
 **  freesystem() - FREE MEMORY ALLOCATED FOR SYSTEM STRUCTURE
 **/


#if	defined(__STDC__)
void freesystem ( SYSTEM * sp )
#else
void freesystem ( sp )
SYSTEM	*sp;
#endif
{
    if (!sp)
	return;

    if (sp->name)
	Free(sp->name);

    if (sp->passwd)
	Free(sp->passwd);

    if (sp->comment)
	Free(sp->comment);
}
