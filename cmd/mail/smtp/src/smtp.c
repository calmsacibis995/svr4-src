/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/smtp.c	1.3.4.3"
/*
 * smtp -- client, send mail to remote smtp server
 * Set up for 4.2BSD networking system.
 * TODO:
 *	better mapping of error numbers.
 *	allow partial delivery to multiple recipients when only some
 *		fail (maybe)
 * 	send stuff from cmds.h instead of hard-coded here
 */

#define	USAGE "usage: %s [-u] [-N] [-H helohost] [-d domain] sender targethost recip1 recip2 ...\n"

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include "addrformat.h"
#include "miscerrs.h"
#include "smtp.h"
#include "s_string.h"
#include "aux.h"

char progname[] = "smtp";
int debug;
char *helohost;
int no_mxlookup = 0;

char *strcat(), *strcpy();
extern char *convertaddr();

char *convertto();

/*
 * main - parse arguments and handle options
 */
main(argc, argv)
int argc;
char *argv[];
{
	register int c;
	int errflg = 0;
	int unixformat = 0;
	char *domain = 0;
	char *sender = 0;
	char *host = 0;
	char *addr = 0;
	namelist *recips, *newname(), *appendname();
	FILE *sfi, *sfo;
	string *replyaddr=s_new();
	string *hh;

	extern int optind;
	extern char *optarg;

	while ((c = getopt(argc, argv, "a:uDd:H:N")) != EOF)
		switch (c) {
		case 'a':
			addr = optarg;
			break;
		case 'u':
			unixformat = 1;
			break;
		case 'D':
			debug = 1;
			break;
		case 'd':		/* domain name */
			domain = optarg;
			break;
		case 'H':		/* host for HELLO message */
			helohost = optarg;
			break;
		case 'N':
			no_mxlookup = 1;
			break;
		case '?':
		default:
			errflg++;
			break;
		}
	if (errflg || (argc - optind) < 3) {
		(void) fprintf(stderr, USAGE, progname);
		bomb(E_USAGE);
	}

	/*
	 *  figure out what to call ourselves
	 */
	if (helohost==NULL)
		helohost=s_to_c(s_copy(sysname_read()));

	/*
	 *  if there is no domain in the helo host name
	 *  and we the -d option is specified, domainify
	 *  the helo host
	 */
	if(strchr(helohost, '.')==0 && domain){
		hh = s_copy(helohost);
		s_append(hh, domain);
		helohost = s_to_c(hh);
	}

	/*
	 *  put our address onto the reply address
	 */
	if(strchr(argv[optind], '!')==0 || !domain){
		s_append(replyaddr, helohost);
		s_append(replyaddr, "!");
		s_append(replyaddr, argv[optind]);
	} else {
		s_append(replyaddr, argv[optind]);
	}
	optind++;

	/*
	 *  convert the arguments to 822 form
	 */
	sender = convertaddr(s_to_c(replyaddr), domain, SOURCEROUTE);
	host = argv[optind++];
	recips = newname(convertto(argv[optind++], unixformat, host));
	for (; optind < argc; optind++)
		recips = appendname(recips, convertto(argv[optind], unixformat, host));

	/*
	 *  open connection
	 */
	setup(addr ? addr : host, &sfi, &sfo);

	/*
	 *  hold the conversation
	 */
	converse(unixformat, sender, recips, domain, sfi, sfo, stdin);

	exit(0);
}

namelist *
newname(s)
	char *s;
{
	namelist *np;

	np = (namelist *)malloc(sizeof(namelist));
	if (np == NULL)
		bomb(1);
	np->name = s;
	np->next = NULL;
	return np;
}

/* could add at beginning, but let's maintain original order */
namelist *
appendname(nl, s)
	char *s;
	namelist *nl;
{
	register namelist *tl;

	if (nl == NULL)
		bomb(1);	/* shouldn't happen */
	for (tl=nl; tl->next!=NULL; tl=tl->next)
		;
	tl->next = newname(s);
	return nl;
}

/*
 *  convert a destination address to outgoing format
 *
 *	if unixformat, just leave it alone
 *
 *	if not add the destination host name.
 */
char *
convertto(recip, unixformat, desthost)
	char *recip;
	char *desthost;
{
	static string *buf;

	if(unixformat)
		return recip;
	
	buf = s_reset(buf);
	s_append(buf, desthost);
	s_append(buf, "!");
	s_append(buf, recip);
	return convertaddr(s_to_c(buf), 0, SOURCEROUTE);
}
