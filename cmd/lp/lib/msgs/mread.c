/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/msgs/mread.c	1.6.3.1"
/* LINTLIBRARY */


#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stropts.h>

#include "lp.h"
#include "msgs.h"

extern int	Lp_prio_msg;

/*
**	Function:	int mread( MESG *, char *, int)
**	Args:		message descriptor
**			message buffer (var)
**			buffer size
**	Return:		The size of the message in message buffer.
**			or -1 on error.  Possible errnos are:
**		EINVAL	Bad value for md or msgbuf.
**		E2BIG	Not enough space for message.
**		EPIPE	Far end dropped the connection.
**		ENOMSG	No valid message available on fifo.
**
**	mread examines message descriptor and either calls read3_2
**	to read 3.2 HPI messages or getmsg(2) to read 4.0 HPI messages.
**	If a message is read, it is returned in message buffer.
*/

#if	defined(__STDC__)
int mread ( MESG * md, char * msgbuf, int size )
#else
int mread ( md, msgbuf, size )
MESG	*md;
char	*msgbuf;
int	size;
#endif
{
    int			flag = 0;
    char		buff [MSGMAX];
    struct strbuf	dat;
    struct strbuf	ctl;

    if (md == NULL || msgbuf == NULL)
    {
	errno = EINVAL;
	return(-1);
    }

    switch(md->type)
    {
      case MD_CHILD:
      case MD_STREAM:
      case MD_BOUND:
	if (size <= 0)
	{
	    errno = E2BIG;
	    return(-1);
	}
	dat.buf = msgbuf;
	dat.maxlen = size;
	dat.len = 0;
	ctl.buf = buff;
	ctl.maxlen = sizeof (buff);
	ctl.len = 0;
	flag = Lp_prio_msg;
	Lp_prio_msg = 0;	/* clean this up so there are no surprises */
	
	if (Getmsg(md, &ctl, &dat, &flag) < 0)
	{
	    if (errno == EBADF)
		errno = EPIPE;
	    return(-1);
	}

	if (dat.len == 0)
	{
	    (void) Close(md->readfd);
	    return(0);
	}
	break;

      case MD_USR_FIFO:
      case MD_SYS_FIFO:
	if (size < CONTROL_LEN)
	{
	    errno = E2BIG;
	    return(-1);
	}

	if (read3_2(md, msgbuf, size) < 0)
	    return(-1);
	break;
    }

    return((int)msize(msgbuf));
}
