/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/bsd/findfld.c	1.1.2.1"

#include <string.h>
#include "lp.h"

/*
 * Return pointer to field in 'buf' beginning with 'ptn'
 * (Input string unmolested)
 */
char *
#if defined (__STDC__)
find_strfld(register char *ptn, register char *buf)
#else
find_strfld(ptn, buf)
register char	*ptn;
register char	*buf;
#endif
{
	char	**list;
	char	**pp;
	char	 *p = NULL;
	int 	  n = strlen(ptn);

	if (!(list = dashos(buf)))
		return(NULL);
	for (pp = list; *pp; pp++)
		if (STRNEQU(ptn, *pp, n)) {
			p = strdup(*pp);
			break;
		}
	freelist(list);
	return(p);
}

/*
 * Return pointer to list entry beginning with 'ptn'
 */
char *
#if defined (__STDC__)
find_listfld(register char *ptn, register char **list)
#else
find_listfld(ptn, list)
register char	 *ptn;
register char	**list;
#endif
{
	int	n = strlen(ptn);

	if (!list || !*list)
		return(NULL);
	for (; *list; ++list)
                if (STRNEQU(ptn, *list, n))
                        return (*list);
        return (NULL);
}
