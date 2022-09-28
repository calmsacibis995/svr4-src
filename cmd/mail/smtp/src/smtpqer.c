/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/smtpqer.c	1.5.4.1"
/* <@(#)smtpqer.c	1.4, 8/1/89> */
/*
 *  Name:
 *	smtpqer - Modified for use as a System V mail(1) surrogate
 *	for the SMTP transport.
 *
 *  Usage:
 *	/usr/lib/mail/surrcmd/smtpqer [-n] [-H helohost] [-d domain]
 *		[-a toaddr] [-u] from_address to_address
 */

#ifdef SVR3
#ifndef TLI
#include <netdb.h>
#endif

#define	MODIFIED_SMTPQER 1	/* Expect 2 args: from & to */
#define	EXIT_OK		99	/* Mail handled by this surrogate */
#define	EXIT_CONTINUE	0	/* Continue with next surrogate */
#endif

#ifdef SVR4
#ifndef TLI
#include <netdb.h>
#endif

#define	EXIT_OK		0	/* Mail handled by this surrogate */
#define	EXIT_CONTINUE	1	/* Continue with next surrogate */
#endif

/*
 * smtpqer -- put mail (on stdin) in an smtpq subdirectory, with a shell script
 *            to send it via smtp
 */

#define USAGE "usage: smtpqer [-n] [-H  helohost] [-d domain] [-a toaddr] [-u] from tohost to...\n"
#define DIRDOMLEVEL 2
#define SPOOLNAMSIZ 12	/* keep spool dir names no longer (12 allows L.name on v9) */

#include "mail.h"
#include "s_string.h"
#include "xmail.h"
#include <varargs.h>

/* globals */
char datafile[1024];
char progname[] = "smtpqer";	/* Needed for logging */
int debug = 0;
int no_mxlookup = 0;
void bomb();

extern spoolsubdir[];
/* imports (other than in .h's) */
extern void exit();
extern char *sysname_read();

/* interrupt handling */
SIGRETURN
catcher(notused)
int notused;
{
	bomb("interrupted\n");
	/*NOTREACHED*/
}

#ifdef TLI
#include <netconfig.h>
#include <netdir.h>
#include <tiuser.h>

int validhost(host)
char *host;
{
	extern struct netconfig *getnetpath();
	extern void *		setnetpath();
	struct nd_hostserv	ndh;
	struct netconfig *	ncp;
	struct nd_addrlist *	nap;
	void *			handle;

	ndh.h_host = host;
	ndh.h_serv = "smtp";

	if ((handle = setnetpath()) != NULL) {
		while ((ncp = getnetpath(handle)) != NULL) {
			if (ncp->nc_semantics == NC_TPI_CLTS)
				continue;
			if (mxnetdir(ncp, &ndh, &nap) != 0)
				continue;
			endnetpath(handle);
			return 1;
		}
	
		endnetpath(handle);
	}
	return 0;
}
#endif

main(argc, argv)
int argc;
char *argv[];
{
	register int c;
	int errflg=0;
	extern int optind;
	extern char *optarg;
	char *p;
	char *domain=0;
	int unixformat=0;
	int norun=0;
	char *helohost=0;
	char *toaddr=0;
	char *from=0;
	string *to=s_new();
	char msg[BUFSIZ/2];
#ifdef	MODIFIED_SMTPQER
	register int l;
	char host[256];
#else
	char *host=0;
#endif

	signal(SIGHUP, catcher);
	signal(SIGINT, catcher);
	while ((c=getopt(argc, argv, "g:H:d:ua:nN")) != EOF)
		switch(c) {
		case 'H':	helohost=optarg;	break;
		case 'd':	domain=optarg;		break;
		case 'u':	unixformat++;		break;
		case 'a':	toaddr=optarg;		break;
		case 'n':	norun++;		break;
		case 'N':	no_mxlookup++;		break;
		case '?':
		default:
				errflg++;		break;
		}
#ifdef	MODIFIED_SMTPQER
	/* There should be two arguments left (who from and to) */
	if (errflg || (argc - optind) < 2)
		bomb(USAGE);
	from=argv[optind++];
	if ((p = strchr(argv[optind], '!')) == NULL)
		exit(EXIT_CONTINUE);	/* means it is not for SMTP */

	/* Get host part of the name */
	l = p - argv[optind];
	strncpy(host, argv[optind], l);
	host[l] = '\0';

	/* Is it a valid SMTP host? */
#ifdef TLI
	if (validhost(host) == 0)
#else
	if (gethostbyname(host) == 0)
#endif
		exit(EXIT_CONTINUE);	/* means it is not for SMTP */

#else
	if (errflg || (argc - optind) < 3)
		bomb(USAGE);

	/* Set domain name (if not set) */
	if (domain == (char *) 0) {
		extern char *maildomain();
		(void) xsetenv(MAILCNFG);
		domain = maildomain();
	}

#endif
	if (helohost==NULL)
		helohost=s_to_c(s_copy(sysname_read()));

#ifndef	MODIFIED_SMTPQER
	from=argv[optind++];
	host=argv[optind++];
#ifdef TLI
	if (validhost(host) == 0)
		exit(EXIT_CONTINUE);	/* means it is not for SMTP */
#endif
#endif

	for (p=host; *p; p++)
		if (isupper(*p))
			*p=tolower(*p);
	for (; optind < argc; optind++) {
		s_append(to, argv[optind]);
		s_append(to, " ");
	}

	/*
	 *  make spool files:
	 * 	C.xxxxxxxxxxxx	- the control file
	 * 	D.xxxxxxxxxxxx	- the data file
	 */
	if(gotodir(host)<0)
		bomb("going to spool directory %s\n", spoolsubdir);
	makedata(from, domain, s_to_c(to));
	makectl(unixformat, helohost, domain, from, toaddr, host, s_to_c(to));

	/*
	 *  run the queue for the receiver
	 */
	if(!norun)
		smtpsched("Qsmtpsched", spoolsubdir);

	/* Successfully queued, log it */
	sprintf(msg, "queued message for host <%s> user <%s>", host, s_to_c(to));
	smtplog(msg);
	exit(EXIT_OK);	/* succesfully queued */
	/*NOTREACHED*/
}

/*
 *  the data file is pre-converted to rfc822
 */
makedata(from, domain, to)
	char *from;
	char *domain;
	char *to;
{
	int fd;
	FILE *fp;

	/*
	 *  create data file
	 */
	strcpy(datafile, "D.xxxxxxxxxxxx");
	fd = mkdatafile(datafile);
	if(fd<0)
		bomb("creating spool file\n");
	fp = fdopen(fd, "w");

	/*
	 *  copy data
	 */
	clearerr(fp);
	clearerr(stdin);
	copymsg(stdin, fp);
	fflush(fp);

	/*
	 *  make sure it worked
	 */
	if(ferror(fp) || ferror(stdin)){
		unlink(datafile);
		bomb("writing data file");
	}
	fclose(fp);
}

/*
 *  just copy input to output
 */
copymsg(in, out)
	FILE *in;
	FILE *out;
{
	char buf[4096];
	int n;

	while(n=fread(buf, 1, sizeof(buf), in))
		if(fwrite(buf, 1, n, out)!=n)
			bomb("writing data file");
}

/*
 *  make a control file.  the two line contain
 *	<reply-addr>	<dest>
 * 	-H <hello host> -d <domain> <reply_addr> <dest> <recipients>
 */
makectl(unixformat, helo, domain, from, daddr, dest, to)
char *helo, *domain, *from, *daddr, *dest, *to;
{
	string *msg = s_new();

	s_append(msg, from);
	s_append(msg, " ");
	s_append(msg, dest);
	s_append(msg, "\n");
	if (unixformat)
		s_append(msg, "-u ");
	if (no_mxlookup)
		s_append(msg, "-N ");
	if (domain && *domain) {
		s_append(msg, "-d ");
		s_append(msg, domain);
		s_append(msg, " ");
	}
	if (daddr) {
		s_append(msg, "-a ");
		s_append(msg, daddr);
		s_append(msg, " ");
	}
	s_append(msg, "-H ");
	s_append(msg, helo);
	s_append(msg, " ");
	s_append(msg, from);
	s_append(msg, " ");
	s_append(msg, dest);
	s_append(msg, " ");
	s_append(msg, to);
	s_append(msg, "\n");
	if (mkctlfile('C', datafile, s_to_c(msg)) < 0)
		bomb("creating control file\n");
}

void
bomb(va_alist)
va_dcl
{
	va_list args;
	char *msg;

	va_start(args);
	fprintf(stderr, "smtpqer: ");
	msg = va_arg(args, char *);
	vfprintf(stderr, msg, args);
	va_end(args);
	if (datafile[0]!=0)
		unlink(datafile);
	exit(EXIT_CONTINUE);
}
