/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/msgs/mdestroy.c	1.5.3.1"
# include	<string.h>
# include	<stropts.h>
# include	<errno.h>
# include	<stdlib.h>
# include	<unistd.h>

# include	"lp.h"
# include	"msgs.h"

#if	defined(__STDC__)
int mdestroy ( MESG * md )
#else
int mdestroy (md)
    MESG	*md;
#endif
{
    if (md->type != MD_MASTER || md->file == NULL)
    {
	errno = EINVAL;
	return(-1);
    }

    if (fdetach(md->file) != 0)
        return(-1);

    if (ioctl(md->writefd, I_POP, "connld") != 0)
        return(-1);

    Free(md->file);
    md->file = NULL;

    (void) mdisconnect(md);

    return(0);
}
