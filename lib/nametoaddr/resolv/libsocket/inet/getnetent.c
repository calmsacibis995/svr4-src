/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nametoaddr:resolv/libsocket/inet/getnetent.c	1.1.1.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>

#define	MAXALIASES	35

static char NETDB[] = "/etc/networks";
static FILE *netf = NULL;
static char line[BUFSIZ+1];
static struct netent net;
static char *net_aliases[MAXALIASES];
int _net_stayopen;
static char *_rs_any();

_rs_setnetent(f)
	int f;
{
	if (netf == NULL)
		netf = fopen(NETDB, "r" );
	else
		rewind(netf);
	_net_stayopen |= f;
}

_rs_endnetent()
{
	if (netf) {
		fclose(netf);
		netf = NULL;
	}
	_net_stayopen = 0;
}

struct netent *
_rs_getnetent()
{
	char *p;
	register char *cp, **q;

	if (netf == NULL && (netf = fopen(NETDB, "r" )) == NULL)
		return (NULL);
again:
	p = fgets(line, BUFSIZ, netf);
	if (p == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = _rs_any(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	net.n_name = p;
	cp = _rs_any(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = _rs_any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	net.n_net = _rs_inet_network(cp);
	net.n_addrtype = AF_INET;
	q = net.n_aliases = net_aliases;
	if (p != NULL) 
		cp = p;
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &net_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = _rs_any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&net);
}

static char *
_rs_any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}
