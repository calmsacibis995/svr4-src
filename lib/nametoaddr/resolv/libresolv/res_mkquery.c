/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:resolv/libresolv/res_mkquery.c	1.1.2.1"

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
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "res.h"

#ifndef SYSV
extern	char *sprintf();
#endif SYSV

/*  TWG name of function */
res_mkquery(op, dname, class, type, data, datalen, newrr, buf, buflen)
	int op;			/* opcode of query */
	char *dname;		/* domain name */
	int class, type;	/* class and type of query */
	char *data;		/* resource record data */
	int datalen;		/* length of data */
	struct rrec *newrr;	/* new rr for modify or append */
	char *buf;		/* buffer to put query */
	int buflen;		/* size of buffer */
{
	return(_rs_res_mkquery(op, dname, class, type, data, datalen, 
		newrr, buf, buflen));
}

/*
 * Form all types of queries.
 * Returns the size of the result or -1.
 */
_rs_res_mkquery(op, dname, class, type, data, datalen, newrr, buf, buflen)
	int op;			/* opcode of query */
	char *dname;		/* domain name */
	int class, type;	/* class and type of query */
	char *data;		/* resource record data */
	int datalen;		/* length of data */
	struct rrec *newrr;	/* new rr for modify or append */
	char *buf;		/* buffer to put query */
	int buflen;		/* size of buffer */
{
	register HEADER *hp;
	register char *cp;
	register int n;
	char dnbuf[MAXDNAME];
	char *dnptrs[10], **dpp, **lastdnptr;
	extern char *index();

#ifdef DEBUG
	if (_res.options & RES_DEBUG)
		printf("_rs_res_mkquery(%d, %s, %d, %d)\n", op, dname, class, type);
#endif DEBUG
	/*
	 * Initialize header fields.
	 */
	hp = (HEADER *) buf;
	hp->id = _rs_htons(++_res.id);
	hp->opcode = op;
	hp->qr = hp->aa = hp->tc = hp->ra = 0;
	hp->pr = (_res.options & RES_PRIMARY) != 0;
	hp->rd = (_res.options & RES_RECURSE) != 0;
	hp->rcode = NOERROR;
	hp->qdcount = 0;
	hp->ancount = 0;
	hp->nscount = 0;
	hp->arcount = 0;
	cp = buf + sizeof(HEADER);
	buflen -= sizeof(HEADER);
	dpp = dnptrs;
	*dpp++ = buf;
	*dpp++ = NULL;
	lastdnptr = dnptrs + sizeof(dnptrs)/sizeof(dnptrs[0]);
	/*
	 * If the domain name contains no dots (single label), then
	 * append the default domain name to the one given.
	 */
	if ((_res.options & RES_DEFNAMES) && dname != 0 && dname[0] != '\0' &&
	    index(dname, '.') == NULL) {
		if (!(_res.options & RES_INIT))
			if (_rs_res_init() == -1)
				return(-1);
		if (_res.defdname[0] != '\0')
			dname = sprintf(dnbuf, "%s.%s", dname, _res.defdname);
	}
	/*
	 * perform opcode specific processing
	 */
	switch (op) {
	case QUERY:
		buflen -= QFIXEDSZ;
		if ((n = _rs_dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return (-1);
		cp += n;
		buflen -= n;
		_rs_putshort(type, cp);
		cp += sizeof(u_short);
		_rs_putshort(class, cp);
		cp += sizeof(u_short);
		hp->qdcount = _rs_htons(1);
		if (op == QUERY || data == NULL)
			break;
		/*
		 * Make an additional record for completion domain.
		 */
		buflen -= RRFIXEDSZ;
		if ((n = _rs_dn_comp(data, cp, buflen, dnptrs, lastdnptr)) < 0)
			return (-1);
		cp += n;
		buflen -= n;
		_rs_putshort(T_NULL, cp);
		cp += sizeof(u_short);
		_rs_putshort(class, cp);
		cp += sizeof(u_short);
		_rs_putlong(0, cp);
		cp += sizeof(u_long);
		_rs_putshort(0, cp);
		cp += sizeof(u_short);
		hp->arcount = _rs_htons(1);
		break;

	case IQUERY:
		/*
		 * Initialize answer section
		 */
		if (buflen < 1 + RRFIXEDSZ + datalen)
			return (-1);
		*cp++ = '\0';	/* no domain name */
		_rs_putshort(type, cp);
		cp += sizeof(u_short);
		_rs_putshort(class, cp);
		cp += sizeof(u_short);
		_rs_putlong(0, cp);
		cp += sizeof(u_long);
		_rs_putshort(datalen, cp);
		cp += sizeof(u_short);
		if (datalen) {
			bcopy(data, cp, datalen);
			cp += datalen;
		}
		hp->ancount = _rs_htons(1);
		break;

#ifdef ALLOW_UPDATES
	/*
	 * For UPDATEM/UPDATEMA, do UPDATED/UPDATEDA followed by UPDATEA
	 * (Record to be modified is followed by its replacement in msg.)
	 */
	case UPDATEM:
	case UPDATEMA:

	case UPDATED:
		/*
		 * The res code for UPDATED and UPDATEDA is the same; user
		 * calls them differently: specifies data for UPDATED; server
		 * ignores data if specified for UPDATEDA.
		 */
	case UPDATEDA:
		buflen -= RRFIXEDSZ + datalen;
		if ((n = _rs_dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return (-1);
		cp += n;
		_rs_putshort(type, cp);
                cp += sizeof(u_short);
                _rs_putshort(class, cp);
                cp += sizeof(u_short);
		_rs_putlong(0, cp);
		cp += sizeof(u_long);
		_rs_putshort(datalen, cp);
                cp += sizeof(u_short);
		if (datalen) {
			bcopy(data, cp, datalen);
			cp += datalen;
		}
		if ( (op == UPDATED) || (op == UPDATEDA) ) {
			hp->ancount = _rs_htons(0);
			break;
		}
		/* Else UPDATEM/UPDATEMA, so drop into code for UPDATEA */

	case UPDATEA:	/* Add new resource record */
		buflen -= RRFIXEDSZ + datalen;
		if ((n = _rs_dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
			return (-1);
		cp += n;
		_rs_putshort(newrr->r_type, cp);
                cp += sizeof(u_short);
                _rs_putshort(newrr->r_class, cp);
                cp += sizeof(u_short);
		_rs_putlong(0, cp);
		cp += sizeof(u_long);
		_rs_putshort(newrr->r_size, cp);
                cp += sizeof(u_short);
		if (newrr->r_size) {
			bcopy(newrr->r_data, cp, newrr->r_size);
			cp += newrr->r_size;
		}
		hp->ancount = _rs_htons(0);
		break;

#endif ALLOW_UPDATES
	}
	return (cp - buf);
}
