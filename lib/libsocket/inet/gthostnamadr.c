/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsocket:inet/gthostnamadr.c	1.1.3.1"


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
#include <netdb.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef havdndbm
#include <ndbm.h>
#endif
#include <ctype.h>

#ifdef SYSV
#define bcmp(s1, s2, len)	memcmp(s1, s2, len)
#define bcopy(s1, s2, len)	memcpy(s2, s1, len)
#endif SYSV

#define	MAXALIASES	35

static struct hostent host;
static char *host_aliases[MAXALIASES];
static char hostbuf[BUFSIZ+1];
static char *host_addrs[2];

int h_errno;

/*
 * The following is shared with gethostent.c
 */
extern	char *_host_file;
#ifdef havdndbm
DBM	*_host_db = (DBM *)NULL;
#endif
int	_host_stayopen;	/* set by sethostent(), cleared by endhostent() */

#ifdef havdndbm
static struct hostent *
fetchhost(key)
	datum key;
{
        register char *cp, *tp, **ap;
	int naliases;

        if (key.dptr == 0)
                return ((struct hostent *)NULL);
	key = dbm_fetch(_host_db, key);
	if (key.dptr == 0)
                return ((struct hostent *)NULL);
        cp = key.dptr;
	tp = hostbuf;
	host.h_name = tp;
	while (*tp++ = *cp++)
		;
	bcopy(cp, (char *)&naliases, sizeof(int)); cp += sizeof (int);
	for (ap = host_aliases; naliases > 0; naliases--) {
		*ap++ = tp;
		while (*tp++ = *cp++)
			;
	}
	*ap = (char *)NULL;
	host.h_aliases = host_aliases;
	bcopy(cp, (char *)&host.h_addrtype, sizeof (int));
	cp += sizeof (int);
	bcopy(cp, (char *)&host.h_length, sizeof (int));
	cp += sizeof (int);
	host.h_addr_list = host_addrs;
	host.h_addr = tp;
	bcopy(cp, tp, host.h_length);
        return (&host);
}
#endif

struct hostent *
gethostbyname(nam)
	register char *nam;
{
	register struct hostent *hp;
	register char **cp;
#ifdef havdndbm
        datum key;
#endif
	char lowname[128];
	register char *lp = lowname;
	
	while (*nam)
		if (isupper(*nam))
			*lp++ = tolower(*nam++);
		else
			*lp++ = *nam++;
	*lp = '\0';

#ifdef havdndbm
	if ((_host_db == (DBM *)NULL)
	  && ((_host_db = dbm_open(_host_file, O_RDONLY)) == (DBM *)NULL)) {
#endif
		sethostent(_host_stayopen);
		while (hp = gethostent()) {
			if (strcmp(hp->h_name, lowname) == 0)
				break;
			for (cp = hp->h_aliases; cp != 0 && *cp != 0; cp++)
				if (strcmp(*cp, lowname) == 0)
					goto found;
		}
	found:
		if (!_host_stayopen)
			endhostent();
		return (hp);
#ifdef havdndbm
	}
        key.dptr = lowname;
        key.dsize = strlen(lowname);
	hp = fetchhost(key);
	if (!_host_stayopen) {
		dbm_close(_host_db);
		_host_db = (DBM *)NULL;
	}
	if ( hp == NULL)
		h_errno = HOST_NOT_FOUND;
        return (hp);
#endif
}

struct hostent *
gethostbyaddr(addr, length, type)
	char *addr;
	register int length;
	register int type;
{
	register struct hostent *hp;
#ifdef havdndbm
        datum key;

	if ((_host_db == (DBM *)NULL)
	  && ((_host_db = dbm_open(_host_file, O_RDONLY)) == (DBM *)NULL)) {
#endif
		sethostent(_host_stayopen);
		while (hp = gethostent()) {
			if (hp->h_addrtype == type && hp->h_length == length
			    && bcmp(hp->h_addr, addr, length) == 0)
				break;
		}
		if (!_host_stayopen)
			endhostent();
		if ( hp == NULL)
			h_errno = HOST_NOT_FOUND;
		return (hp);
#ifdef havdndbm
	}
        key.dptr = addr;
        key.dsize = length;
	hp = fetchhost(key);
	if (!_host_stayopen) {
		dbm_close(_host_db);
		_host_db = (DBM *)NULL;
	}
	if ( hp == NULL)
		h_errno = HOST_NOT_FOUND;
        return (hp);
#endif
}
#ifdef EASY
struct hostent *
gethostbyname(nam)
char *nam;
{
	return ((struct hostent *) 0);
}

struct hostent *
gethostbyaddr(addr, length, type)
char *addr;
int length;
int type;
{
	return ((struct hostent *) 0);
}
#endif
