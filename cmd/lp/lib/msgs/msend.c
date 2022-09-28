/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/msgs/msend.c	1.4.2.1"
/* LINTLIBRARY */
# include	<errno.h>

# include	"lp.h"
# include	"msgs.h"

extern MESG	*lp_Md;
extern int	discon3_2_is_running;

/*
** msend() - SEND A MESSAGE VIA FIFOS
*/

#if	defined(__STDC__)
int msend ( char * msgbuf )
#else
int msend (msgbuf)
    char	*msgbuf;
#endif
{
    int		rval;

    do
    {
	if ((rval = mwrite(lp_Md, msgbuf)) < 0)
	{
	    /*
	    ** "mclose()" will try to say goodbye to the Spooler,
	    ** and that, of course, will fail. But we'll call
	    ** "mclose()" anyway, for the other cleanup it does.
	    */
	    if (errno == EPIPE)
	    {
		if (!discon3_2_is_running)
		    (void)mclose ();
		errno = EIDRM;
	    }
	}
    }
    while (rval < 0 && errno == EINTR);

    return(rval);
}
