/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:resolv/libresolv/res_query.c	1.1.1.1"

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
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#endif /* SYSV */
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "res.h"

#if PACKETSZ > 1024
#define MAXPACKET	PACKETSZ
#else
#define MAXPACKET	1024
#endif

extern int errno;
int h_errno;

/*  TWG name of same function */
res_query(name, class, type, answer, anslen)
	char *name;		/* domain name */
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer buffer */
{
	return(_rs_res_query(name, class, type, answer, anslen));
}

/*
 * Formulate a normal query, send, and await answer.
 * Returned answer is placed in supplied buffer "answer".
 * Perform preliminary check of answer, returning success only
 * if no error is indicated and the answer count is nonzero.
 * Return the size of the response on success, -1 on error.
 * Error number is left in h_errno.
 * Caller must parse answer and determine whether it answers the question.
 */
_rs_res_query(name, class, type, answer, anslen)
	char *name;		/* domain name */
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer buffer */
{
	char buf[MAXPACKET];
	HEADER *hp;
	int n;

	if ((_res.options & RES_INIT) == 0 && _rs_res_init() == -1)
		return (-1);
#ifdef DEBUG
	if (_res.options & RES_DEBUG)
		printf("_rs_res_query(%s, %d, %d)\n", name, class, type);
#endif
	n = _rs_res_mkquery(QUERY, name, class, type, (char *)NULL, 0, NULL,
	    buf, sizeof(buf));

	if (n <= 0) {
#ifdef DEBUG
		if (_res.options & RES_DEBUG)
			printf("res_query: mkquery failed\n");
#endif
		h_errno = NO_RECOVERY;
		return (n);
	}
	n = _rs_res_send(buf, n, answer, anslen);
	if (n < 0) {
#ifdef DEBUG
		if (_res.options & RES_DEBUG)
			printf("res_query: send error\n");
#endif
		h_errno = TRY_AGAIN;
		return(n);
	}

	hp = (HEADER *) answer;
	if (hp->rcode != NOERROR || _rs_ntohs(hp->ancount) == 0) {
#ifdef DEBUG
		if (_res.options & RES_DEBUG)
			printf("rcode = %d, ancount=%d\n", hp->rcode,
			    _rs_ntohs(hp->ancount));
#endif
		switch (hp->rcode) {
			case NXDOMAIN:
				h_errno = HOST_NOT_FOUND;
				break;
			case SERVFAIL:
				h_errno = TRY_AGAIN;
				break;
			case NOERROR:
				h_errno = NO_DATA;
				break;
			case FORMERR:
			case NOTIMP:
			case REFUSED:
			default:
				h_errno = NO_RECOVERY;
				break;
		}
		return (-1);
	}
	return(n);
}

/*  TWG name of same function */
res_search(name, class, type, answer, anslen)
	char *name;		/* domain name */
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer */
{
	return(_rs_res_search(name, class, type, answer, anslen));
}

/*
 * Formulate a normal query, send, and retrieve answer in supplied buffer.
 * Return the size of the response on success, -1 on error.
 * If enabled, implement search rules until answer or unrecoverable failure
 * is detected.  Error number is left in h_errno.
 * Only useful for queries in the same name hierarchy as the local host
 * (not, for example, for host address-to-name lookups in domain in-addr.arpa).
 */
_rs_res_search(name, class, type, answer, anslen)
	char *name;		/* domain name */
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer */
{
	register char *cp, **domain;
	int n, ret, got_nodata = 0;
	char *_rs_hostalias();

	if ((_res.options & RES_INIT) == 0 && _rs_res_init() == -1)
		return (-1);

	errno = 0;
	h_errno = HOST_NOT_FOUND;		/* default, if we never query */
	for (cp = name, n = 0; *cp; cp++)
		if (*cp == '.')
			n++;
	if (n == 0 && (cp = _rs_hostalias(name)))
		return (_rs_res_query(cp, class, type, answer, anslen));

	if ((n == 0 || *--cp != '.') && (_res.options & RES_DEFNAMES))
	    for (domain = _res.dnsrch; *domain; domain++) {
		ret = _rs_res_querydomain(name, *domain, class, type,
		    answer, anslen);
		if (ret > 0)
			return (ret);
		/*
		 * If no server present, give up.
		 * If name isn't found in this domain,
		 * keep trying higher domains in the search list
		 * (if that's enabled).
		 * On a NO_DATA error, keep trying, otherwise
		 * a wildcard entry of another type could keep us
		 * from finding this entry higher in the domain.
		 * If we get some other error (non-authoritative negative
		 * answer or server failure), then stop searching up,
		 * but try the input name below in case it's fully-qualified.
		 */
		if (errno == ECONNREFUSED) {
			h_errno = TRY_AGAIN;
			return (-1);
		}
		if (h_errno == NO_DATA)
			got_nodata++;
		if ((h_errno != HOST_NOT_FOUND && h_errno != NO_DATA) ||
		    (_res.options & RES_DNSRCH) == 0)
			break;
	}
	/*
	 * If the search/default failed, try the name as fully-qualified,
	 * but only if it contained at least one dot (even trailing).
	 */
	if (n)
		return (_rs_res_querydomain(name, (char *)NULL, class, type,
		    answer, anslen));
	if (got_nodata)
		h_errno = NO_DATA;
	return (-1);
}

/*  TWG name of same function */
res_querydomain(name, domain, class, type, answer, anslen)
	char *name, *domain;
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer */
{
	return(_rs_res_querydomain(name, domain, class, type, answer, anslen));
}

/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
_rs_res_querydomain(name, domain, class, type, answer, anslen)
	char *name, *domain;
	int class, type;	/* class and type of query */
	u_char *answer;		/* buffer to put answer */
	int anslen;		/* size of answer */
{
	char nbuf[2*MAXDNAME+2];
	char *longname = nbuf;
	int n;

#ifdef DEBUG
	if (_res.options & RES_DEBUG)
		printf("_rs_res_querydomain(%s, %s, %d, %d)\n",
		    name, domain, class, type);
#endif
	if (domain == NULL) {
		/*
		 * Check for trailing '.';
		 * copy without '.' if present.
		 */
		n = strlen(name) - 1;
		if (name[n] == '.' && n < sizeof(nbuf) - 1) {
			bcopy(name, nbuf, n);
			nbuf[n] = '\0';
		} else
			longname = name;
	} else
		(void)sprintf(nbuf, "%.*s.%.*s",
		    MAXDNAME, name, MAXDNAME, domain);

	return (_rs_res_query(longname, class, type, answer, anslen));
}

/*  TWG name of same function */
char *
hostalias(name)
	register char *name;
{
	return(_rs_hostalias(name));
}

char *
_rs_hostalias(name)
	register char *name;
{
	register char *C1, *C2;
	FILE *fp;
	char *file, *getenv(), *strcpy(), *strncpy();
	char buf[BUFSIZ];
	static char abuf[MAXDNAME];

	file = getenv("HOSTALIASES");
	if (file == NULL || (fp = fopen(file, "r")) == NULL)
		return (NULL);
	buf[sizeof(buf) - 1] = '\0';
	while (fgets(buf, sizeof(buf), fp)) {
		for (C1 = buf; *C1 && !isspace(*C1); ++C1);
		if (!*C1)
			break;
		*C1 = '\0';
		if (!_rs_strcasecmp(buf, name)) {
			while (isspace(*++C1));
			if (!*C1)
				break;
			for (C2 = C1 + 1; *C2 && !isspace(*C2); ++C2);
			abuf[sizeof(abuf) - 1] = *C2 = '\0';
			(void)strncpy(abuf, C1, sizeof(abuf) - 1);
			fclose(fp);
			return (abuf);
		}
	}
	fclose(fp);
	return (NULL);
}
