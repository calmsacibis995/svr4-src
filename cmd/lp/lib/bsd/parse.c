/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/bsd/parse.c	1.2.2.1"

#include <string.h>
#include <stdlib.h>
#if defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "lpd.h"

/*
 * Parse request-id into destination and number
 * (destroys input string by replacing "-" with NULL byte)
 */
void
#if defined (__STDC__)
parseReqid(char *reqid, char **pdest, char **pnum)
#else
parseReqid(reqid, pdest, pnum)
char	 *reqid;
char	**pdest;
char	**pnum;
#endif
{
	char	*p;

	if (p = strrchr(reqid, '-'))
		*p++ = NULL;
	if (pdest)
		*pdest = reqid;
	if (pnum)
		*pnum = p;
}

/*
 * Parse user specification (host!user) into host and user part
 * (destroys input string by replacing "!" with NULL byte if host part
 *  requested)
 */
void
#if defined (__STDC__)
parseUser(char *name, char **phost, char **puser)
#else
parseUser(name, phost, puser)
char	 *name;
char	**phost;
char	**puser;
#endif
{
	register char	*p;

	if (p = strchr(name, '!')) {
		if (phost) {
			*p = NULL;
			*phost = name;
		}
		if (puser)
			*puser = ++p;
	} else {
		if (phost)
			*phost = Lhost;
		if (puser)
			*puser = name;
	}
}

/*
 * Piece together request-id from component parts (destination and number)
 */
char *
#if defined (__STDC__)
mkreqid(char *dest, char *id)
#else
mkreqid(dest, id)
char	*dest;
char	*id;
#endif
{
	char	*buf;

	if (buf = malloc(strlen(dest) + strlen(id) + 2))
		sprintf(buf, "%s-%s", dest, id);
	return(buf);
}

/*
 * Parse file names and sizes from flist string
 * (no more than 'maxn' fields are processed)
 */
#if defined (__STDC__)
parseflist(register char *cp, int maxn, char **files, char **sizes)
#else
parseflist(cp, maxn, files, sizes)
register char	 *cp;
int		  maxn;
char		**files;
char		**sizes;
#endif
{
	int	n = 0;

	for (cp = getitem(cp, ' '); n < maxn && cp; cp = getitem(NULL, ' ')) {
		if (files)
			files[n] = cp;
		if (cp = strrchr(cp, ':')) {
			*cp++ = NULL;
			if (sizes)
				sizes[n] = cp;
		}
		n++;
	}
	return(n);
}

/*
 * Return pointer to next list item delimited by (unexcaped) 'sep' or 
 * single-quote (side-effect: removes escape characters from field returned)
 */
char *
#if defined (__STDC__)
getitem(register char *cp, char sep)
#else
getitem(cp, sep)
register char	*cp;
char		 sep;
#endif
{
	register char	*cp2;
	static char	*nxtitem;
	char		*delim = " '";

	delim[0] = sep;
	if (!cp && !(cp = nxtitem))
		return(NULL);
	cp2 = cp += strspn(cp, delim); 
	if (!*cp)
		return(NULL);
	while (cp2 = strpbrk(cp2, delim))  {
		if (escaped(cp2)) {
	     		cp2++;
			continue;
		}
		*cp2 = NULL;
		break;
	}
	if (cp2)
		nxtitem = cp2 + 1;
	else
		nxtitem = NULL;
	rmesc(cp);
	return(cp);
}

/*
 * Convert request-id to job-id (strip destination part and zero-fill)
 */
char *
#if defined (__STDC__)
rid2jid(char *reqid)
#else
rid2jid(reqid)
char	*reqid;
#endif
{
	register char	*cp;
	static char	 buf[SIZEOF_JOBID+1];
	int		 n;

	if (cp = strrchr(reqid, '-')) {
		n = atoi(cp+1) % NJOBIDS;
		sprintf(buf, "%0*d", SIZEOF_JOBID, n);
		return(buf);
	} else
		return(NULL);
}
