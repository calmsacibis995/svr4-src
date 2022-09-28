/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/to822addr.c	1.3.3.1"
#include <stdio.h>
#include "s_string.h"
#include "addrformat.h"

/*
 *	Convert from `bang' to `%@' format or `source routing' format
 *	or `mixed rfc976' format.
 *	Let a!b!c!d be addr with host prepended if not gatewaying:
 *
 *	   a!b!c!d ->	d%c%b@a.domain		(format==PERCENT)
 *		   ->	@a.domain,@b:d@c	(format==SOURCEROUTE)
 *		   ->	b!c!d@a.domain		(format==RFC976)
 *
 *	where the .domain is added only if domain!=0 and 'a' has no '.'s
 *	
 *	   a!b!uucp!c!d -> c!d%b@a.domain or @a.domain:c!d@b
 *
 *	Or, if addr has one component only:
 *
 *	   a	->	a@host.domain    if host!=0
 */
extern char *
convertaddr(addr, domain, format)
	char *addr;
	char *domain;
{
	static string *buf;
	register int i=0;
	register char *sp;
	int j, d;
	char *field[128];

	/* parse the '!' format address */
	buf = s_reset(buf);
	field[i] = addr;
	sp = addr;
	while (*sp) {
		if (*sp == '!') {
			*sp = '\0';
			if (strcmp(field[i], "uucp") == 0) {
				/* the token 'uucp' stops the parsing */
				field[i] = ++sp;
				break;
			}
			field[++i] = ++sp;
			if(format==RFC976)
				break;
		} else
			++sp;
	}
	if (buf==NULL)
		buf= s_new();
	s_restart(buf);
	if (i==0) {
		s_append(buf, field[0]);
	} else {
		if (format!=SOURCEROUTE) {
			s_append(buf, field[i--]);
			while (i>=1) {
				s_append(buf, "%");
				s_append(buf, field[i--]);
			}
			addhostdom(buf, field[0], domain);
		} else {
			/*
			 *  count leading domain fields (non-domains don't count)
			 */
			d = 0;
			if(domain || strchr(field[0], '.'))
				d++;
			for( ; d<i; d++)
				if(strchr(field[d], '.')==0)
					break;
			/*
			 *  if there are more than 1 leading domain elements,
			 *  put them in as source routing
			 */
			if(d > 1){
				addhostdom(buf, field[0], domain);
				for(j=1; j<d-1; j++){
					s_append(buf, ",");
					s_append(buf, "@");
					s_append(buf, field[j]);
				}
				s_append(buf, ":");
			}

			/*
			 *  throw in the non-domain elements separated by '!'s
			 */
			s_append(buf, field[d]);
			for(j=d+1; j<=i; j++) {
				s_append(buf, "!");
				s_append(buf, field[j]);
			}
			if(d)
				addhostdom(buf, field[d-1], domain);
		}
	}
	return s_to_c(s_copy(s_to_c(buf)));
}

addhostdom(buf, host, dom)
	string *buf;
	char *host;
	char *dom;
{
	char *cp;

	s_append(buf, "@");
	s_append(buf, host);
	if(dom && !strchr(host, '.')){
		cp = strchr(dom, '.');
		if(cp!=NULL)
			s_append(buf, cp);
	}
}
