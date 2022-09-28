/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/systems/llib-llpsys.c	1.1.2.1"

# include	<errno.h>
# include	<stdio.h>
# include	<stdlib.h>
# include	<string.h>
# include	<sys/types.h>

# include	"lp.h"
# include	"systems.h"

/* LINTLIBRARY */

/*
**	From:	delsystem.c
*/

#if	defined(__STDC__)
int delsystem ( const char * name )
#else
int delsystem ( name )
char	*name;
#endif
{
    static int	return_value;
    return return_value;
}

/*
**	From:	freesystem.c
*/
#if	defined(__STDC__)
void freesystem ( SYSTEM * sp )
#else
void freesystem ( sp )
SYSTEM	*sp;
#endif
{
}

/*
**	From:	getsystem.c
*/
#if	defined(__STDC__)
SYSTEM * getsystem ( const char * name )
#else
SYSTEM * getsystem ( name )
char	*name;
#endif
{
    static SYSTEM	*return_value;
    return return_value;
}

/*
**	From:	putsystem.c
*/
#if	defined(__STDC__)
int putsystem ( const char * name, const SYSTEM * sysbufp )
#else
int putsystem ( name, sysbufp )
char	*name;
SYSTEM	*sysbufp;
#endif
{
    static int	returned_value;
    return returned_value;
}
