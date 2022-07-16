/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsocket:inet/inet_sethost.c	1.1.2.1"


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
#ifdef havdndbm
#include <ndbm.h>
#endif

#define bcopy(s1, s2, len)	memcpy(s2, s1, len)

/*
 * Internet version.
 */
#define	MAXALIASES	35
#define	MAXADDRSIZE	14

static FILE *hostf = NULL;
static char line[BUFSIZ+1];
static char hostaddr[MAXADDRSIZE];
static struct hostent host;
static char *host_aliases[MAXALIASES];
static char *host_addrs[] = {
	hostaddr,
	NULL
};

/*
 * The following is shared with gethostnamadr.c
 */
char	*_host_file = "/etc/hosts";
int	_host_stayopen;
#ifdef havdndbm
DBM	*_host_db;	/* set by gethostbyname(), gethostbyaddr() */
#endif

static char *any();

sethostent(f)
	int f;
{
	if (hostf != NULL)
		rewind(hostf);
	_host_stayopen |= f;
}

endhostent()
{
	if (hostf) {
		fclose(hostf);
		hostf = NULL;
	}
#ifdef havdndbm
	if (_host_db) {
		dbm_close(_host_db);
		_host_db = (DBM *)NULL;
	}
#endif
	_host_stayopen = 0;
}

struct hostent *
gethostent()
{
	char *p;
	register char *cp, **q;
	u_long theaddr;

	if (hostf == NULL && (hostf = fopen(_host_file, "r" )) == NULL)
		return (NULL);
again:
	if ((p = fgets(line, BUFSIZ, hostf)) == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = any(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	cp = any(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
	host.h_addr_list = host_addrs;
	theaddr= inet_addr(p);
	if ( (long)theaddr == -1 )
		goto again;
	bcopy( &theaddr, host.h_addr_list[0], sizeof (u_long));
	host.h_length = sizeof (u_long);
	host.h_addrtype = AF_INET;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	host.h_name = cp;
	q = host.h_aliases = host_aliases;
	cp = any(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &host_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&host);
}

sethostfile(file)
	char *file;
{
	_host_file = file;
}

static char *
any(cp, match)
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
