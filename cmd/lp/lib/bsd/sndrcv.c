/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/bsd/sndrcv.c	1.2.2.1"

#if defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "lp.h"
#include "msgs.h"
#define WHO_AM_I	I_AM_OZ		/* to get oam.h to unfold */
#include "oam.h"
#include "lpd.h"

/*
 * Format and send message to lpsched
 * (die if any errors occur)
 */
/*VARARGS1*/
void
#if defined (__STDC__)
snd_msg(int type, ...)
#else
snd_msg(type, va_alist)
int	type;
va_dcl
#endif
{
	va_list	ap;

#if defined (__STDC__)
	va_start(ap, type);
#else
	va_start(ap);
#endif
	(void)_putmessage (Msg, type, ap);
	va_end(ap);
	if (msend(Msg) == -1) {
		lp_fatal(E_LP_MSEND); 
		/*NOTREACHED*/
	}
}

/*
 * Recieve message from lpsched
 * (die if any errors occur)
 */
void
#if defined (__STDC__)
rcv_msg(int type, ...)
#else
rcv_msg(type, va_alist)
int	type;
va_dcl
#endif
{
	va_list ap;
	int rc;

	if ((rc = mrecv(Msg, MSGMAX)) != type) {
		if (rc == -1)
			lp_fatal(E_LP_MRECV); 
		else
			lp_fatal(E_LP_BADREPLY, rc); 
		/*NOTREACHED*/
	}
#if defined (__STDC__)
	va_start(ap, type);
#else
	va_start(ap);
#endif
	rc = _getmessage(Msg, type, ap);
	va_end(ap);
	if (rc < 0) {
		lp_fatal(E_LP_GETMSG, PERROR); 
		/*NOTREACHED*/ 
	} 
}
