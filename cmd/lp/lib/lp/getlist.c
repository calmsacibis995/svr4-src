/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/getlist.c	1.13.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"
#include "errno.h"
#include "stdlib.h"

#include "lp.h"

#if	defined(__STDC__)
static char		*unq_strdup ( char * , char * );
#else
static char		*unq_strdup();
#endif

/**
 ** getlist() - CONSTRUCT LIST FROM STRING
 **/

/*
 * Any number of characters from "ws", or a single
 * character from "hardsep", can separate items in the list.
 */

char **
#if	defined(__STDC__)
getlist (
	char *			str,
	char *			ws,
	char *			hardsep
)
#else
getlist (str, ws, hardsep)
	register char		*str,
				*ws;
	char			*hardsep;
#endif
{
	register char		**list,
				*p,
				*sep,
				c;

	int			n,
				len;

	char			buf[10];


	if (!str || !*str)
		return (0);

	/*
	 * Construct in "sep" the full list of characters that
	 * can separate items in the list. Avoid a "malloc()"
	 * if possible.
	 */
	len = strlen(ws) + strlen(hardsep) + 1;
	if (len > sizeof(buf)) {
		if (!(sep = Malloc(len))) {
			errno = ENOMEM;
			return (0);
		}
	} else
		sep = buf;
	strcpy (sep, hardsep);
	strcat (sep, ws);

	/*
	 * Skip leading white-space.
	 */
	str += strspn(str, ws);
	if (!*str)
		return (0);

	/*
	 * Strip trailing white-space.
	 */
	p = strchr(str, '\0');
	while (--p != str && strchr(ws, *p))
		;
	*++p = 0;

	/*
	 * Pass 1: Count the number of items in the list.
	 */
	for (n = 0, p = str; *p; ) {
		if ((c = *p++) == '\\')
			p++;
		else
			if (strchr(sep, c)) {
				n++;
				p += strspn(p, ws);
				if (
					!strchr(hardsep, c)
				     && strchr(hardsep, *p)
				) {
					p++;
					p += strspn(p, ws);
				}
			}
	}

	/*
	 * Pass 2: Create the list.
	 */

	/*
	 * Pass 1 counted the number of list separaters, so
	 * add 2 to the count (includes 1 for terminating null).
	 */
	if (!(list = (char **)Malloc((n+2) * sizeof(char *)))) {
		errno = ENOMEM;
		goto Done;
	}

	/*
	 * This loop will copy all but the last item.
	 */
	for (n = 0, p = str; *p; )
		if ((c = *p++) == '\\')
			p++;
		else
			if (strchr(sep, c)) {

				p[-1] = 0;
				list[n++] = unq_strdup(str, sep);
				p[-1] = c;

				p += strspn(p, ws);
				if (
					!strchr(hardsep, c)
				     && strchr(hardsep, *p)
				) {
					p++;
					p += strspn(p, ws);
				}
				str = p;

			}

	list[n++] = unq_strdup(str, sep);

	list[n] = 0;

Done:	if (sep != buf)
		Free (sep);
	return (list);
}

/**
 ** unq_strdup()
 **/

static char *
#if	defined(__STDC__)
unq_strdup (
	char *			str,
	char *			sep
)
#else
unq_strdup (str, sep)
	char			*str,
				*sep;
#endif
{
	register int		len	= 0;

	register char		*p,
				*q,
				*ret;


	for (p = str; *p; p++)
		if (*p != '\\' || !p[1] || !strchr(sep, p[1]))
			len++;
	if (!(q = ret = Malloc(len + 1)))
		return (0);
	for (p = str; *p; p++)
		if (*p != '\\' || !p[1] || !strchr(sep, p[1]))
			*q++ = *p;
	*q = 0;
	return (ret);
}
