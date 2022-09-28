/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpadmin/send_message.c	1.9.2.1"

#include "stdio.h"

#if	defined(__STDC__)
#include "stdarg.h"
#else
#include "varargs.h"
#endif

#include "msgs.h"

#define	WHO_AM_I	I_AM_LPADMIN
#include "oam.h"

#include "lpadmin.h"


/**
 ** send_message() - HANDLE MESSAGE SENDING TO SPOOLER
 **/

/*VARARGS1*/

void
#if	defined(__STDC__)
send_message (
	int			type,
	...
)
#else
send_message (type, va_alist)
	int			type;
	va_dcl
#endif
{
	va_list			ap;

	int			n;

	char			msgbuf[MSGMAX];


	if (!scheduler_active)
		return;

#if	defined(__STDC__)
	va_start (ap, type);
#else
	va_start (ap);
#endif

	(void)_putmessage (msgbuf, type, ap);

	va_end (ap);

	if (msend(msgbuf) == -1) {
		LP_ERRMSG (ERROR, E_LP_MSEND);
		done(1);
	}

	return;
}
