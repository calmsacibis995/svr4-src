/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:lib/msgs/mopen.c	1.5.2.1"
/* LINTLIBRARY */

# include	<errno.h>

# include	"lp.h"
# include	"msgs.h"


MESG	*lp_Md = 0;

/*
** mopen() - OPEN A MESSAGE PATH
*/

int
mopen ()
{
    if (lp_Md != NULL)
    {
	errno = EEXIST;
	return (-1);
    }

    if ((lp_Md = mconnect(Lp_FIFO, 0, 0)) == NULL)
	return(-1);

    return(0);
}
