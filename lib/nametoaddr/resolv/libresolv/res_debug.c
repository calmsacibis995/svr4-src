/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:resolv/libresolv/res_debug.c	1.1.2.1"

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

#if defined(lint) && !defined(DEBUG)
#define DEBUG
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include "res.h"

extern char *_rs_p_cdname(), *_rs_p_rr(), *_rs_p_type(), *_rs_p_class();
extern char *_rs_inet_ntoa();

char *_res_opcodes[] = {
	"QUERY",
	"IQUERY",
	"CQUERYM",
	"CQUERYU",
	"4",
	"5",
	"6",
	"7",
	"8",
	"UPDATEA",
	"UPDATED",
	"UPDATEDA",
	"UPDATEM",
	"UPDATEMA",
	"ZONEINIT",
	"ZONEREF",
};

char *_res_resultcodes[] = {
	"NOERROR",
	"FORMERR",
	"SERVFAIL",
	"NXDOMAIN",
	"NOTIMP",
	"REFUSED",
	"6",
	"7",
	"8",
	"9",
	"10",
	"11",
	"12",
	"13",
	"14",
	"NOCHANGE",
};

/* TWG name of same function */
p_query(msg)
	char *msg;
{
	return(_rs_p_query(msg));
}

_rs_p_query(msg)
	char *msg;
{
#ifdef DEBUG
	_rs_fp_query(msg,stdout);
#endif
}

/* TWG name of same function */
fp_query(msg,file)
	char *msg;
	FILE *file;
{
	return(_rs_fp_query(msg,file));
}

/*
 * Print the contents of a query.
 * This is intended to be primarily a debugging routine.
 */
_rs_fp_query(msg,file)
	char *msg;
	FILE *file;
{
#ifdef DEBUG
	register char *cp;
	register HEADER *hp;
	register int n;

	/*
	 * Print header fields.
	 */
	hp = (HEADER *)msg;
	cp = msg + sizeof(HEADER);
	fprintf(file,"HEADER:\n");
	fprintf(file,"\topcode = %s", _res_opcodes[hp->opcode]);
	fprintf(file,", id = %d", _rs_ntohs(hp->id));
	fprintf(file,", rcode = %s\n", _res_resultcodes[hp->rcode]);
	fprintf(file,"\theader flags: ");
	if (hp->qr)
		fprintf(file," qr");
	if (hp->aa)
		fprintf(file," aa");
	if (hp->tc)
		fprintf(file," tc");
	if (hp->rd)
		fprintf(file," rd");
	if (hp->ra)
		fprintf(file," ra");
	if (hp->pr)
		fprintf(file," pr");
	fprintf(file,"\n\tqdcount = %d", _rs_ntohs(hp->qdcount));
	fprintf(file,", ancount = %d", _rs_ntohs(hp->ancount));
	fprintf(file,", nscount = %d", _rs_ntohs(hp->nscount));
	fprintf(file,", arcount = %d\n\n", _rs_ntohs(hp->arcount));
	/*
	 * Print question records.
	 */
	if (n = _rs_ntohs(hp->qdcount)) {
		fprintf(file,"QUESTIONS:\n");
		while (--n >= 0) {
			fprintf(file,"\t");
			cp = _rs_p_cdname(cp, msg, file);
			if (cp == NULL)
				return;
			fprintf(file,", type = %s", _rs_p_type(_rs__getshort(cp)));
			cp += sizeof(u_short);
			fprintf(file,", class = %s\n\n", _rs_p_class(_rs__getshort(cp)));
			cp += sizeof(u_short);
		}
	}
	/*
	 * Print authoritative answer records
	 */
	if (n = _rs_ntohs(hp->ancount)) {
		fprintf(file,"ANSWERS:\n");
		while (--n >= 0) {
			fprintf(file,"\t");
			cp = _rs_p_rr(cp, msg, file);
			if (cp == NULL)
				return;
		}
	}
	/*
	 * print name server records
	 */
	if (n = _rs_ntohs(hp->nscount)) {
		fprintf(file,"NAME SERVERS:\n");
		while (--n >= 0) {
			fprintf(file,"\t");
			cp = _rs_p_rr(cp, msg, file);
			if (cp == NULL)
				return;
		}
	}
	/*
	 * print additional records
	 */
	if (n = _rs_ntohs(hp->arcount)) {
		fprintf(file,"ADDITIONAL RECORDS:\n");
		while (--n >= 0) {
			fprintf(file,"\t");
			cp = _rs_p_rr(cp, msg, file);
			if (cp == NULL)
				return;
		}
	}
#endif
}

/* TWG name of same function */
char *
p_cdname(cp, msg, file)
	char *cp, *msg;
	FILE *file;
{
	return(_rs_p_cdname(cp, msg, file));
}

char *
_rs_p_cdname(cp, msg, file)
	char *cp, *msg;
	FILE *file;
{
#ifdef DEBUG
	char name[MAXDNAME];
	int n;

	if ((n = _rs_dn_expand(msg, msg + 512, cp, name, sizeof(name))) < 0)
		return (NULL);
	if (name[0] == '\0') {
		name[0] = '.';
		name[1] = '\0';
	}
	fputs(name, file);
	return (cp + n);
#endif
}

/* TWG name of same function */
p_rr(cp, msg, file)
	char *cp, *msg;
	FILE *file;
{
	return(_rs_p_rr(cp, msg, file));
}

/*
 * Print resource record fields in human readable form.
 */
char *
_rs_p_rr(cp, msg, file)
	char *cp, *msg;
	FILE *file;
{
#ifdef DEBUG
	int type, class, dlen, n, c;
	struct in_addr inaddr;
	char *cp1;

	if ((cp = _rs_p_cdname(cp, msg, file)) == NULL)
		return (NULL);			/* compression error */
	fprintf(file,"\n\ttype = %s", _rs_p_type(type = _rs__getshort(cp)));
	cp += sizeof(u_short);
	fprintf(file,", class = %s", _rs_p_class(class = _rs__getshort(cp)));
	cp += sizeof(u_short);
	fprintf(file,", ttl = %u", _rs__getlong(cp));
	cp += sizeof(u_long);
	fprintf(file,", dlen = %d\n", dlen = _rs__getshort(cp));
	cp += sizeof(u_short);
	cp1 = cp;
	/*
	 * Print type specific data, if appropriate
	 */
	switch (type) {
	case T_A:
		switch (class) {
		case C_IN:
			bcopy(cp, (char *)&inaddr, sizeof(inaddr));
			if (dlen == 4) {
				fprintf(file,"\tinternet address = %s\n",
					_rs_inet_ntoa(inaddr));
				cp += dlen;
			} else if (dlen == 7) {
				fprintf(file,"\tinternet address = %s",
					_rs_inet_ntoa(inaddr));
				fprintf(file,", protocol = %d", cp[4]);
				fprintf(file,", port = %d\n",
					(cp[5] << 8) + cp[6]);
				cp += dlen;
			}
			break;
		default:
			cp += dlen;
		}
		break;
	case T_CNAME:
	case T_MB:
#ifdef OLDRR
	case T_MD:
	case T_MF:
#endif /* OLDRR */
	case T_MG:
	case T_MR:
	case T_NS:
	case T_PTR:
		fprintf(file,"\tdomain name = ");
		cp = _rs_p_cdname(cp, msg, file);
		fprintf(file,"\n");
		break;

	case T_HINFO:
		if (n = *cp++) {
			fprintf(file,"\tCPU=%.*s\n", n, cp);
			cp += n;
		}
		if (n = *cp++) {
			fprintf(file,"\tOS=%.*s\n", n, cp);
			cp += n;
		}
		break;

	case T_SOA:
		fprintf(file,"\torigin = ");
		cp = _rs_p_cdname(cp, msg, file);
		fprintf(file,"\n\tmail addr = ");
		cp = _rs_p_cdname(cp, msg, file);
		fprintf(file,"\n\tserial=%ld", _rs__getlong(cp));
		cp += sizeof(u_long);
		fprintf(file,", refresh=%ld", _rs__getlong(cp));
		cp += sizeof(u_long);
		fprintf(file,", retry=%ld", _rs__getlong(cp));
		cp += sizeof(u_long);
		fprintf(file,", expire=%ld", _rs__getlong(cp));
		cp += sizeof(u_long);
		fprintf(file,", min=%ld\n", _rs__getlong(cp));
		cp += sizeof(u_long);
		break;

	case T_MX:
		fprintf(file,"\tpreference = %ld,",_rs__getshort(cp));
		cp += sizeof(u_short);
		fprintf(file," name = ");
		cp = _rs_p_cdname(cp, msg, file);
		break;

	case T_MINFO:
		fprintf(file,"\trequests = ");
		cp = _rs_p_cdname(cp, msg, file);
		fprintf(file,"\n\terrors = ");
		cp = _rs_p_cdname(cp, msg, file);
		break;

	case T_UINFO:
		fprintf(file,"\t%s\n", cp);
		cp += dlen;
		break;

	case T_UID:
	case T_GID:
		if (dlen == 4) {
			fprintf(file,"\t%ld\n", _rs__getlong(cp));
			cp += sizeof(int);
		}
		break;

	case T_WKS:
		if (dlen < sizeof(u_long) + 1)
			break;
		bcopy(cp, (char *)&inaddr, sizeof(inaddr));
		cp += sizeof(u_long);
		fprintf(file,"\tinternet address = %s, protocol = %d\n\t",
			_rs_inet_ntoa(inaddr), *cp++);
		n = 0;
		while (cp < cp1 + dlen) {
			c = *cp++;
			do {
 				if (c & 0200)
					fprintf(file," %d", n);
 				c <<= 1;
			} while (++n & 07);
		}
		putc('\n',file);
		break;

#ifdef ALLOW_T_UNSPEC
	case T_UNSPEC:
		{
			int NumBytes = 8;
			char *DataPtr;
			int i;

			if (dlen < NumBytes) NumBytes = dlen;
			fprintf(file, "\tFirst %d bytes of hex data:",
				NumBytes);
			for (i = 0, DataPtr = cp; i < NumBytes; i++, DataPtr++)
				fprintf(file, " %x", *DataPtr);
			fputs("\n", file);
			cp += dlen;
		}
		break;
#endif /* ALLOW_T_UNSPEC */

	default:
		fprintf(file,"\t???\n");
		cp += dlen;
	}
	if (cp != cp1 + dlen)
		fprintf(file,"packet size error (%#x != %#x)\n", cp, cp1+dlen);
	fprintf(file,"\n");
	return (cp);
#endif
}

static	char nbuf[20];
#ifndef SYSV
extern	char *sprintf();
#endif SYSV

/* TWG name of same function */
char *
p_type(type)
	int type;
{
	return(_rs_p_type(type));
}

/*
 * Return a string for the type
 */
char *
_rs_p_type(type)
	int type;
{
	switch (type) {
	case T_A:
		return("A");
	case T_NS:		/* authoritative server */
		return("NS");
#ifdef OLDRR
	case T_MD:		/* mail destination */
		return("MD");
	case T_MF:		/* mail forwarder */
		return("MF");
#endif /* OLDRR */
	case T_CNAME:		/* connonical name */
		return("CNAME");
	case T_SOA:		/* start of authority zone */
		return("SOA");
	case T_MB:		/* mailbox domain name */
		return("MB");
	case T_MG:		/* mail group member */
		return("MG");
	case T_MX:		/* mail routing info */
		return("MX");
	case T_MR:		/* mail rename name */
		return("MR");
	case T_NULL:		/* null resource record */
		return("NULL");
	case T_WKS:		/* well known service */
		return("WKS");
	case T_PTR:		/* domain name pointer */
		return("PTR");
	case T_HINFO:		/* host information */
		return("HINFO");
	case T_MINFO:		/* mailbox information */
		return("MINFO");
	case T_AXFR:		/* zone transfer */
		return("AXFR");
	case T_MAILB:		/* mail box */
		return("MAILB");
	case T_MAILA:		/* mail address */
		return("MAILA");
	case T_ANY:		/* matches any type */
		return("ANY");
	case T_UINFO:
		return("UINFO");
	case T_UID:
		return("UID");
	case T_GID:
		return("GID");
#ifdef ALLOW_T_UNSPEC
	case T_UNSPEC:
		return("UNSPEC");
#endif /* ALLOW_T_UNSPEC */
	default:
		return (sprintf(nbuf, "%d", type));
	}
}

/* TWG name of same function */
char *
p_class(class)
	int class;
{
	return(_rs_p_class(class));
}

/*
 * Return a mnemonic for class
 */
char *
_rs_p_class(class)
	int class;
{

	switch (class) {
	case C_IN:		/* internet class */
		return("IN");
	case C_ANY:		/* matches any class */
		return("ANY");
	default:
		return (sprintf(nbuf, "%d", class));
	}
}
