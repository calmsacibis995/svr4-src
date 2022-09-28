/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/msgs/getmessage.c	1.7.2.1"
/* LINTLIBRARY */
/*
*/

#if	defined(__STDC__)
# include	<stdarg.h>
#else
# include	<varargs.h>
#endif

/* VARARGS */
#if	defined(__STDC__)
int getmessage ( char * buf, short type, ... )
#else
int getmessage (buf, type, va_alist)
    char	*buf;
    short	type;
    va_dcl
#endif
{
    va_list	arg;
    int		rval;
    int	_getmessage();

#if	defined(__STDC__)
    va_start(arg, type);
#else
    va_start(arg);
#endif
    
    rval = _getmessage(buf, type, arg);
    va_end(arg);
    return(rval);
}
