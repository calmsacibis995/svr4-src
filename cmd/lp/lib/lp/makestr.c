/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/makestr.c	1.6.3.1"
/* LINTLIBRARY */

#if	defined(__STDC__)
#include "stdarg.h"
#else
#include "varargs.h"
#endif

#include "string.h"
#include "errno.h"
#include "stdlib.h"

#include "lp.h"

/**
 ** makestr() - CONSTRUCT SINGLE STRING FROM SEVERAL
 **/

/*VARARGS1*/
char *
#if	defined(__STDC__)
makestr (
	char *			s,
	...
)
#else
makestr (s, va_alist)
	char *			s;
	va_dcl
#endif
{
	register va_list	ap;

	register char		*component,
				*p,
				*q;

	register int		len;

	char			*ret;


#if	defined(__STDC__)
	va_start (ap, s);
#else
	va_start (ap);
#endif

	for (len = strlen(s); (component = va_arg(ap, char *)); )
		len += strlen(component);

	va_end (ap);

	if (!len) {
		errno = 0;
		return (0);
	}

	if (!(ret = Malloc(len + 1))) {
		errno = ENOMEM;
		return (0);
	}

#if	defined(__STDC__)
	va_start (ap, s);
#else
	va_start (ap);
#endif

	for (
		p = ret, component = s;
		component;
		component = va_arg(ap, char *)
	)
		for (q = component; *q; )
			*p++ = *q++;
	*p = 0;

	va_end(ap);

	return (ret);
}
