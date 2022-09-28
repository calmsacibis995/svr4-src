/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/msgs/_getmessage.c	1.9.2.1"
/* LINTLIBRARY */

# include	<stdarg.h>
# include	<string.h>
# include	<errno.h>

# include	"msgs.h"

extern char	*_lp_msg_fmts[];
extern int	errno;

/* VARARGS */
#if	defined(__STDC__)
int _getmessage ( char * buf, short rtype, va_list arg )
#else
int _getmessage (buf, rtype, arg)
    char	*buf;
    short	rtype;
    va_list	arg;
#endif
{
    char	*endbuf;
    char	*fmt;
    char	**t_string;
    int		temp = 0;
    long	*t_long;
    short	*t_short;
    short	etype;

    if (buf == (char *)0)
    {
	errno = ENOSPC;
	return(-1);
    }

    /*
     * We assume that we're given a buffer big enough to hold
     * the header.
     */

    endbuf = buf + (long)stoh(buf);
    if ((buf + MESG_DATA) > endbuf)
    {
	errno = ENOMSG;
	return(-1);
    }

    etype = stoh(buf + MESG_TYPE);
    if (etype < 0 || etype > LAST_MESSAGE)
    {
	errno = EBADMSG;
        return(-1);
    }

    if (etype != rtype)
    {
	if (rtype > 0 && rtype <= LAST_MESSAGE)
	    fmt = _lp_msg_fmts[rtype];
	else
	{
	    errno = EINVAL;
	    return(-1);
	}
    }
    else
	fmt = _lp_msg_fmts[etype];

    buf += MESG_LEN;

    while (*fmt != '\0')
	switch(*fmt++)
	{
	    case 'H':
	        if ((buf + 4) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}

		t_short = va_arg(arg, short *);
		*t_short = stoh(buf);
		buf += 4;
		break;

	    case 'L':
		if ((buf + 8) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}

		t_long = va_arg(arg, long *);
		*t_long = stol(buf);
		buf += 8;
		break;

	    case 'D':
		if ((buf + 4) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}

		t_short = va_arg(arg, short *);
		*t_short = stoh(buf);
		buf += 4;
		t_string = va_arg(arg, char **);
		if ((buf + *t_short) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}
		(*t_short)--;		/* Don't mention the null we added */
		*t_string = buf;
		buf += *t_short;
		break;

	    case 'S':
		if ((buf + 4) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}

		t_string = va_arg(arg, char **);
		temp = stoh(buf);
		buf += 4;
		if ((buf + temp) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}

		*t_string = buf;
		buf += temp;
		break;
	}
    return(etype);
}
