/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsocket:inet/nd_gethost.c	1.1"

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

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/tiuser.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h> 
#include <stdio.h>
#include <errno.h>
#include <netdir.h>

#define	MAXALIASES	35
#define	MAXADDRS	35

#define bcmp(s1, s2, len)	memcmp(s1, s2, len)
#define bcopy(s1, s2, len)	memcpy(s2, s1, len)
#define bzero(s, len)		memset((char *)(s), 0, len)

static char *h_addr_ptrs[MAXADDRS + 1];

static struct hostent host;
static char *host_aliases[MAXALIASES + 1];
static char hostbuf[BUFSIZ+1];
static struct in_addr host_addr;
static char HOSTDB[] = "/etc/hosts";
static FILE *hostf = NULL;
static char hostaddr[MAXADDRS*sizeof(u_long)];
static char *host_addrs[2];
static int stayopen = 0;
static char *any();

struct hostent *_gethtbyname();
struct hostent *_gethtbyaddr();
static int strcasecmp();

int h_errno;
extern errno;

/*
 * Copy the information acquired in the various "netdir" structures to the
 * "struct hostent" format that the user expects.  When done, free the
 * netdir structures and return a pointer to the hostent structure.
 * The hostent information is stored in local static variables, which are
 * overwritten on subsequent invocations.
 *
 * This routine is a utility for gethostbyname() and gethostbyaddr().
 */
static struct hostent *
nd_to_hostent(nconf, naddrs, hservs)
	struct netconfig *nconf;
	struct nd_addrlist *naddrs;
	struct nd_hostservlist *hservs;
{
	register char *cp;
	int n;
	int len;
	struct hostent *hp;
	struct nd_hostserv *sp;
	struct sockaddr_in *sin;
	struct netbuf *na;

	/*
	 * Official hostname...
	 */
	hp = &host;
	cp = hostbuf;
	hp->h_name = cp;
	sp = hservs->h_hostservs;
	(void) strcpy(cp, sp->h_host);
	cp += strlen(sp->h_host) + 1;
	/*
	 * Alias(es)...
	 */
	hp->h_aliases = host_aliases;
	for (n = 0, sp = &(hservs->h_hostservs[1]); n < (hservs->h_cnt - 1) && n < MAXALIASES; n++, sp++) {
		len = strlen(sp->h_host) + 1;
		if ((cp + len) > (hostbuf + sizeof(hostbuf))) {
			/*
			 * This alias, if added to the list and stored in the
			 * static buffer, would overflow that buffer.  We'll
			 * quit processing alias names, beginning with this
			 * one, to avoid the nasty overflow consequences.
			 */
			break;
		}
		(void) strcpy(cp, sp->h_host);
		host_aliases[n] = cp;
		cp += len;
	}
	host_aliases[n] = NULL;
	/*
	 * Address(es)...
	 */
	hp->h_addr_list = h_addr_ptrs;
	cp = hostaddr;
	for (n = 0, na = naddrs->n_addrs; n < naddrs->n_cnt && n < MAXADDRS; n++, na++) {
		sin = (struct sockaddr_in *) na->buf;
		*((u_long *) cp) = sin->sin_addr.s_addr;
		h_addr_ptrs[n] = cp;
		cp += sizeof(u_long);
	}
	h_addr_ptrs[n] = NULL;
	/*
	 * Address length and family...
	 */
	hp->h_length = sizeof(u_long);
	hp->h_addrtype = AF_INET;

	/*
	 * Free netdir structures and return a pointer to the hostent structure.
	 */
	(void) freenetconfigent(nconf);
	(void) netdir_free(naddrs, ND_ADDRLIST);
	(void) netdir_free(hservs, ND_HOSTSERVLIST);

	return (hp);
}

struct hostent *
gethostbyname(name)
	char *name;
{
	register char *cp;
	struct netconfig *nconf;
	struct nd_hostserv serv;
	struct nd_hostservlist *hservs;
	struct nd_addrlist *naddrs;

	/*
	 * Disallow names consisting only of digits/dots, unless
	 * they end in a dot.
	 */
	if (isdigit(name[0])) {
		for (cp = name;; ++cp) {
			if (!*cp) {
				if (*--cp == '.')
					break;
				h_errno = HOST_NOT_FOUND;
				return ((struct hostent *) NULL);
			}
			if (!isdigit(*cp) && *cp != '.') 
				break;
		}
	}

	/*
	 * Use the transport-independent (netdir) interface to obtain the
	 * host information for which the user is asking.  This will allow
	 * the dynamic shared object library to perform the actual name
	 * lookup, which could involve either the local static tables (i.e.,
	 * /etc/hosts) or the domain name server.
	 *
	 * Since the generic interface requires a hostname/service pair, we
	 * will provide an arbitrary port number (ascii string) instead of
	 * a service name, as we really don't care about the port number.
	 * Once we've retrieved the address information for the "name" passed
	 * to us, use the (first) address to obtain the official hostname and
	 * aliases.  Finally, encapsulate all this and return it to the user.
	 */
	if ((nconf = getnetconfigent("tcp")) == NULL &&
	    (nconf = getnetconfigent("udp")) == NULL) {
		/*
		 * Couldn't find an entry in /etc/netconfig for either
		 * Internet protocol, TCP or UDP.
		 */
		h_errno = HOST_NOT_FOUND;
		return ((struct hostent *) NULL);
	}
	/*
	 * Get the address(es) associated with the "name" passed to us via
	 * netdir_getbyname(), using the netconfig entry returned by the
	 * getnetconfigent() call above.
	 */
	serv.h_host = name;
	serv.h_serv = "111";
	naddrs = (struct nd_addrlist *) NULL;
	if (netdir_getbyname(nconf, &serv, &naddrs) != ND_OK ||
	    naddrs == (struct nd_addrlist *) NULL ||
	    naddrs->n_cnt < 1) {
		/*
		 * Hostname lookup failed, so clean up and return failure.
		 */
		(void) freenetconfigent(nconf);
		if (naddrs != (struct nd_addrlist *) NULL) {
			(void) netdir_free(naddrs, ND_ADDRLIST);
		}
		h_errno = HOST_NOT_FOUND;
		return ((struct hostent *) NULL);
	}
	/*
	 * Now get the official hostname and alias(es) information via
	 * netdir_getbyaddr(), using the first address returned by the
	 * netdir_getbyname() call above.
	 */
	hservs = (struct nd_hostservlist *) NULL;
	if (netdir_getbyaddr(nconf, &hservs, naddrs->n_addrs) != ND_OK ||
	    hservs == (struct nd_hostservlist *) NULL ||
	    hservs->h_cnt < 1) {
		/*
		 * Address lookup failed, so clean up and return failure.
		 */
		(void) freenetconfigent(nconf);
		if (naddrs != (struct nd_addrlist *) NULL) {
			(void) netdir_free(naddrs, ND_ADDRLIST);
		}
		if (hservs != (struct nd_hostservlist *) NULL) {
			(void) netdir_free(hservs, ND_HOSTSERVLIST);
		}
		h_errno = HOST_NOT_FOUND;
		return ((struct hostent *) NULL);
	}

	return (nd_to_hostent(nconf, naddrs, hservs));
}

struct hostent *
gethostbyaddr(addr, len, type)
	char *addr;
	int len, type;
{
	struct netconfig *nconf;
	struct nd_hostservlist *hservs;
	struct nd_addrlist *naddrs;
	struct sockaddr_in sa;
	struct netbuf nb;
	
	if (type != AF_INET || len != sizeof(u_long)) {
		return ((struct hostent *) NULL);
	}

	/*
	 * Use the transport-independent (netdir) interface to obtain the
	 * host information for which the user is asking.  This will allow
	 * the dynamic shared object library to perform the actual name
	 * lookup, which could involve either the local static tables (i.e.,
	 * /etc/hosts) or the domain name server.
	 *
	 * Since the generic interface requires a full sockaddr_in structure,
	 * including port number and Internet address, we will provide an
	 * arbitrary port number.  Once we've retrieved the hostname/alias(es)
	 * information for the "addr" passed to us, use the first hostname to
	 * obtain the full list of addresses (probably only one, but done for
	 * completeness).  Finally, encapsulate this and return it to the user.
	 */
	if ((nconf = getnetconfigent("tcp")) == NULL &&
	    (nconf = getnetconfigent("udp")) == NULL) {
		/*
		 * Couldn't find an entry in /etc/netconfig for either
		 * Internet protocol, TCP or UDP.
		 */
		h_errno = HOST_NOT_FOUND;
		return ((struct hostent *) NULL);
	}
	/*
	 * Get the official hostname and alias(es) information via
	 * netdir_getbyaddr().  First, construct a sockaddr_in structure to
	 * contain the address, and a netbuf structure (whose "buf" pointer
	 * points to the sockaddr_in structure just constructed).
	 */
	bzero(&sa, sizeof(sa));
	sa.sin_port = htons(111);
	sa.sin_family = AF_INET;
	bcopy(addr, &sa.sin_addr.s_addr, sizeof(u_long));
	nb.maxlen = sizeof(struct sockaddr_in);
	nb.len = sizeof(struct sockaddr_in);
	nb.buf = (char *) &sa;
	hservs = (struct nd_hostservlist *) NULL;
	if (netdir_getbyaddr(nconf, &hservs, &nb) != ND_OK ||
	    hservs == (struct nd_hostservlist *) NULL ||
	    hservs->h_cnt < 1) {
		/*
		 * Address lookup failed, so clean up and return failure.
		 */
		(void) freenetconfigent(nconf);
		if (hservs != (struct nd_hostservlist *) NULL) {
			(void) netdir_free(hservs, ND_HOSTSERVLIST);
		}
		h_errno = HOST_NOT_FOUND;
		return ((struct hostent *) NULL);
	}
	/*
	 * Now get the address(es) associated with the official hostname
	 * just retrieved via netdir_getbyaddr().  This may be overkill, but
	 * it ensures that ALL addresses are returned to the user, not just
	 * the one they passed to us.
	 */
	naddrs = (struct nd_addrlist *) NULL;
	if (netdir_getbyname(nconf, hservs->h_hostservs, &naddrs) != ND_OK ||
	    naddrs == (struct nd_addrlist *) NULL ||
	    naddrs->n_cnt < 1) {
		/*
		 * Hostname lookup failed, so clean up and return failure.
		 */
		(void) freenetconfigent(nconf);
		if (hservs != (struct nd_hostservlist *) NULL) {
			(void) netdir_free(hservs, ND_HOSTSERVLIST);
		}
		if (naddrs != (struct nd_addrlist *) NULL) {
			(void) netdir_free(naddrs, ND_ADDRLIST);
		}
		h_errno = HOST_NOT_FOUND;
		return ((struct hostent *) NULL);
	}

	return (nd_to_hostent(nconf, naddrs, hservs));
}

_sethtent(f)
	int f;
{
	if (hostf == NULL)
		hostf = fopen(HOSTDB, "r" );
	else
		rewind(hostf);
	stayopen |= f;
}

_endhtent()
{
	if (hostf && !stayopen) {
		(void) fclose(hostf);
		hostf = NULL;
	}
}

struct hostent *
_gethtent()
{
	char *p;
	register char *cp, **q;

	if (hostf == NULL && (hostf = fopen(HOSTDB, "r" )) == NULL)
		return (NULL);
again:
	if ((p = fgets(hostbuf, BUFSIZ, hostf)) == NULL)
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
#if BSD >= 43 || defined(h_addr)	/* new-style hostent structure */
	host.h_addr_list = host_addrs;
#endif
	host.h_addr = hostaddr;
	*((u_long *)host.h_addr) = inet_addr(p);
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

struct hostent *
_gethtbyname(name)
	char *name;
{
	register struct hostent *p;
	register char **cp;
	
	_sethtent(0);
	while (p = _gethtent()) {
		if (strcasecmp(p->h_name, name) == 0)
			break;
		for (cp = p->h_aliases; *cp != 0; cp++)
			if (strcasecmp(*cp, name) == 0)
				goto found;
	}
found:
	_endhtent();
	return (p);
}

struct hostent *
_gethtbyaddr(addr, len, type)
	char *addr;
	int len, type;
{
	register struct hostent *p;

	_sethtent(0);
	while (p = _gethtent())
		if (p->h_addrtype == type && !bcmp(p->h_addr, addr, len))
			break;
	_endhtent();
	return (p);
}

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static char charmap[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

static
strcasecmp(s1, s2)
	register char *s1, *s2;
{
	register char *cm = charmap;

	while (cm[*s1] == cm[*s2++])
		if (*s1++ == '\0')
			return(0);
	return(cm[*s1] - cm[*--s2]);
}
