/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nametoaddr:tcpip/file_db.c	1.3.2.2"

/*
 * This is the C library "getXXXbyYYY" routines after they have been stripped
 * down to using just the file /etc/hosts and /etc/services. When linked with 
 * the internal name to address resolver routines for TCP/IP they provide 
 * the file based version of those routines.
 *
 * Copyright (c) 1989 by Sun Microsystems, Inc.
 *
 * This file defines gethostbyname(), gethostbyaddr(), getservbyname(), 
 * and getservbyport(). The C library routines are not used as they may
 * one day be based on these routines, and that would cause recursion
 * problems. 
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netdir.h>
#include <netinet/in.h>

#define	MAXALIASES	35
#define MAXADDRS	10
#define	MAXADDRSIZE	14

static struct hostdata {
	FILE	*hostf;
	char	*current;
	int	currentlen;
	int	stayopen;
	char	*host_aliases[MAXALIASES];
	char	hostaddr[MAXADDRSIZE][MAXADDRS];
	char	*addr_list[MAXADDRS+1];
	char	line[BUFSIZ+1];
	struct	hostent host;
} *hostdata, *_hostdata();

static struct servdata {
	FILE	*servf;
	char	*current;
	int	currentlen;
	int	stayopen;
	char	*serv_aliases[MAXALIASES];
	struct	servent serv;
	char	line[BUFSIZ+1];
} *servdata, *_servdata();

static struct hostent *_gethostent(), *he_interpret();
static char HOSTDB[] = "/etc/hosts";

static struct servent *_getservent(), *se_interpret();
static char SERVDB[] = "/etc/services";

static void sethostent(), endhostent(), setservent(), endservent();
char *malloc(), *calloc();

static u_long inet_addr();

static struct hostdata *
_hostdata()
{
	register struct hostdata *d = hostdata;

	if (d == 0) {
		d = (struct hostdata *)calloc(1, sizeof (struct hostdata));
		hostdata = d;
	}
	return (d);
}

struct hostent *
_tcpip_gethostbyaddr(addr, len, type)
	char *addr;
	int len, type;
{
	register struct hostdata *d = _hostdata();
	register struct hostent *p;

	if (d == 0)
		return ((struct hostent*)NULL);
	sethostent(0);
	while (p = _gethostent()) {
		if (p->h_addrtype != type || p->h_length != len)
			continue;
		if (memcmp(p->h_addr_list[0], addr, len) == 0)
			break;
	}
	endhostent();
	return (p);
}

struct hostent *
_tcpip_gethostbyname(name)
	register char *name;
{
	register struct hostdata *d = _hostdata();
	register struct hostent *p;
	register char **cp;

	if (d == 0)
		return ((struct hostent*)NULL);

	if (((int)inet_addr(name)) != -1) { /* parse 1.2.3.4 case */
		sprintf(d->line, "%s %s\n", name, name);
		return(he_interpret());
	}

	if (strcmp(name, HOST_ANY) == 0) 
		return ((struct hostent *)NULL);

	sethostent(0);
	while (p = _gethostent()) {
		if (strcasecmp(p->h_name, name) == 0) {
			break;
		}
		for (cp = p->h_aliases; *cp != 0; cp++) 
			if (strcasecmp(*cp, name) == 0)
				break;
		if (*cp) 
			break;	/* We found it */
	}

	endhostent();
	return (p);
}

static void
sethostent(f)
	int f;
{
	register struct hostdata *d = _hostdata();

	if (d == 0)
		return;
	if (d->hostf == NULL)
		d->hostf = fopen(HOSTDB, "r");
	else
		rewind(d->hostf);
	if (d->current)
		free(d->current);
	d->current = NULL;
	d->stayopen |= f;
}

static void
endhostent()
{
	register struct hostdata *d = _hostdata();

	if (d == 0)
		return;
	if (d->current && !d->stayopen) {
		free(d->current);
		d->current = NULL;
	}
	if (d->hostf && !d->stayopen) {
		fclose(d->hostf);
		d->hostf = NULL;
	}
}

static struct hostent *
_gethostent()
{
	register struct hostdata *d = _hostdata();

	if (d->hostf == NULL && (d->hostf = fopen(HOSTDB, "r")) == NULL)
		return (NULL);
	if (fgets(d->line, BUFSIZ, d->hostf) == NULL)
		return (NULL);
	return (he_interpret());
}

/*
 * This routine interprets the current line. 
 */
static struct hostent *
he_interpret()
{
	register struct hostdata *d = _hostdata();
	char 			 *p;
	register char 		 *cp, **q;

	if (d == 0)
		return (0);

	p = d->line;
	/* Check for comment lines (start with # mark) */
	if (*p == '#')
		return (_gethostent());
	cp = strpbrk(p, "#\n");
	if (cp == NULL)
		return (_gethostent());
	*cp = '\0'; /* Null out any trailing comment */

	/* Now check for whitespace */
	cp = strpbrk(p, " \t");
	if (cp == NULL)
		return (_gethostent());
	*cp++ = '\0'; /* This breaks the line into name/address components */

	/* return one address */
	d->addr_list[0] = (d->hostaddr[0]);
	d->addr_list[1] = NULL;
	d->host.h_addr_list = d->addr_list;
	*((u_long *)d->host.h_addr) = inet_addr(p);
	d->host.h_length = sizeof (u_long);
	d->host.h_addrtype = AF_INET;

	/* skip whitespace between address and the name */
	while (*cp == ' ' || *cp == '\t')
		cp++;
	d->host.h_name = cp;
	q = d->host.h_aliases = d->host_aliases;

	cp = strpbrk(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';

	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &(d->host_aliases[MAXALIASES - 1]))
			*q++ = cp;
		cp = strpbrk(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&d->host);
}

/*
 * The services routines. These nearly identical to the host routines
 * above. The Interpret routine differs. Seems there should be some way
 * to fold this code together.
 */

static struct servdata *
_servdata()
{
	register struct servdata *d = servdata;

	if (d == 0) {
		d = (struct servdata *)calloc(1, sizeof (struct servdata));
		servdata = d;
	}
	return (d);
}

struct servent *
_tcpip_getservbyport(svc_port, proto)
	int svc_port;
	char *proto;
{
	register struct servdata *d = _servdata();
	register struct servent *p = NULL;
	register u_short port = svc_port;

	if (d == 0)
		return (0);

	setservent(0);
	while (p = _getservent()) {
		if (p->s_port != port)
			continue;
		if (proto == 0 || strcasecmp(p->s_proto, proto) == 0)
			break;
	}
	endservent();
	return (p);
}

struct servent *
_tcpip_getservbyname(name, proto)
	register char *name, *proto;
{
	register struct servdata *d = _servdata();
	register struct servent *p;
	register char **cp;

	if (d == 0)
		return (0);
	setservent(0);
	while (p = _getservent()) {
		if (proto != 0 && strcasecmp(p->s_proto, proto) != 0)
			continue;
		if (strcasecmp(name, p->s_name) == 0)
			break;
		for (cp = p->s_aliases; *cp; cp++)
			if (strcasecmp(name, *cp) == 0)
				break;
		if (*cp) 
			break;	/* we found it */
	}
	endservent();
	return (p);
}

static void
setservent(f)
	int f;
{
	register struct servdata *d = _servdata();

	if (d == 0)
		return;
	if (d->servf == NULL)
		d->servf = fopen(SERVDB, "r");
	else
		rewind(d->servf);
	if (d->current)
		free(d->current);
	d->current = NULL;
	d->stayopen |= f;
}

static void
endservent()
{
	register struct servdata *d = _servdata();

	if (d == 0)
		return;
	if (d->current && !d->stayopen) {
		free(d->current);
		d->current = NULL;
	}
	if (d->servf && !d->stayopen) {
		fclose(d->servf);
		d->servf = NULL;
	}
}

static struct servent *
_getservent()
{
	register struct servdata *d = _servdata();

	if (d == 0)
		return NULL;

	if (d->servf == NULL && (d->servf = fopen(SERVDB, "r")) == NULL)
		return (NULL);

	if (fgets(d->line, BUFSIZ, d->servf) == NULL)
		return (NULL);

	return (se_interpret());
}

static struct servent *
se_interpret()
{
	register struct servdata *d = _servdata();
	char *p;
	register char *cp, **q;

	if (d == 0)
		return (0);

	p = d->line;
	if (*p == '#')
		return (_getservent());

	cp = strpbrk(p, "#\n");
	if (cp == NULL)
		return (_getservent());
	*cp = '\0';

	d->serv.s_name = p;
	p = strpbrk(p, " \t");
	if (p == NULL)
		return (_getservent());
	*p++ = '\0';

	while (*p == ' ' || *p == '\t')
		p++;
	cp = strpbrk(p, ",/");
	if (cp == NULL)
		return (_getservent());
	*cp++ = '\0';
	d->serv.s_port = htons((u_short)atoi(p));
	d->serv.s_proto = cp;
	q = d->serv.s_aliases = d->serv_aliases;
	cp = strpbrk(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &(d->serv_aliases[MAXALIASES - 1]))
			*q++ = cp;
		cp = strpbrk(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&d->serv);
}

/* From (#)inet_addr.c 1.9 88/02/08 SMI"; from UCB 4.5 82/11/14 */

/*
 * Internet address interpretation routine.
 * All the network library routines call this
 * routine to interpret entries in the data bases
 * which are expected to be an address.
 * The value returned is in network order.
 */
static u_long
inet_addr(cp)
	register char *cp;
{
	register u_long val, base, n;
	register char c;
	u_long parts[4], *pp = parts;

again:
	/*
	 * Collect number up to ``.''.
	 * Values are specified as for C:
	 * 0x=hex, 0=octal, other=decimal.
	 */
	val = 0; base = 10;
	if (*cp == '0') {
		if (*++cp == 'x' || *cp == 'X')
			base = 16, cp++;
		else
			base = 8;
	}
	while (c = *cp) {
		if (isdigit(c)) {
			if ((c - '0') >= base)
			    break;
			val = (val * base) + (c - '0');
			cp++;
			continue;
		}
		if (base == 16 && isxdigit(c)) {
			val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
			cp++;
			continue;
		}
		break;
	}
	if (*cp == '.') {
		/*
		 * Internet format:
		 *	a.b.c.d
		 *	a.b.c	(with c treated as 16-bits)
		 *	a.b	(with b treated as 24 bits)
		 */
		if (pp >= parts + 4)
			return (-1);
		*pp++ = val, cp++;
		goto again;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp && !isspace(*cp))
		return (-1);
	*pp++ = val;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts;
	switch (n) {

	case 1:				/* a -- 32 bits */
		val = parts[0];
		break;

	case 2:				/* a.b -- 8.24 bits */
		val = (parts[0] << 24) | (parts[1] & 0xffffff);
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
			(parts[2] & 0xffff);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
		      ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
		break;

	default:
		return (-1);
	}
	val = htonl(val);
	return (val);
}

/*
 * XXX: This should be apart of C library. If not so, then please include
 * the correct routine. This is just a wrapper.
 */
int static
strcasecmp(a, b)
	char *a, *b;
{
	return (strcmp(a, b));
}
