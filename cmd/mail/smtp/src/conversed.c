/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/conversed.c	1.5.3.1"
#ifndef lint
static char *sccsid = "@(#)converse.c	1.9 87/07/31";
#endif
/*  Copyright 1984 Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this program
for any purpose and without fee is hereby granted, provided
that this copyright and permission notice appear on all copies
and supporting documentation, the name of M.I.T. not be used
in advertising or publicity pertaining to distribution of the
program without specific prior permission, and notice be given
in supporting documentation that copying and distribution is
by permission of M.I.T.  M.I.T. makes no representations about
the suitability of this software for any purpose.  It is pro-
vided "as is" without express or implied warranty.		*/

/*
 * smtpd - World's most trivial SMTP server.  Only accepts the MAIL, FROM,
 * RCPT, and DATA commands.  Generates a data file for the mail
 * daemon and kicks the mail daemon off.
 */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>

#include <sys/types.h>
#include <fcntl.h>
#include "s_string.h"

#include "xmail.h"
#include "smtp.h"
#include "cmds.h"
#include "miscerrs.h"
#include "s_string.h"

/* fundamental constants */
#define TRUE 1
#define FALSE 0
#define SECONDS		1
#define MINUTES		60
#define HOURS		(60 * MINUTES)

/* tunable constants */
#define	SHORTTIME	(5 * MINUTES)	/* enough time for easy stuff */
#define	LONGTIME	(2 * HOURS)	/* max time, DATA to `.' */

extern char *UPASROOT;

typedef long in_name;			/* internet host address */

static string *rcvrs;

FILE	*datafd;			/* data file descriptor */
FILE	*abortfd=0;			/* Where alarm complains */

char	dataname[NAMSIZ], rcptto[BUFSIZ];		/* data file name */

typedef int event;

extern int death();
extern int alarmsend();


extern char *convertaddr();

extern char *helohost;
extern char *thishost;
extern int queuing;
extern int norun;
extern char spoolsubdir[];

extern void logdanger();

#ifdef SIMPLELOG
#include <sys/file.h>
#endif

extern void xferstatus();
long nbytes=0;

static char mailfrom[BUFSIZ];
static char *fromaddr;

/*
 * A couple of statistical and logging things.
 */
int	n_rcpt = 0;
int	virus;

#ifndef NSYSFILE
#define NSYSFILE 3
#define NOFILE 32
#endif NSYSFILE

SIGRETURN
alarmtr(s)
	int s;
{
	death(E_TEMPFAIL);
}

/*
 * This is the routine which processes incoming smtp commands from the
 * user.  It goes to sleep awaiting network input.  When a complete
 * command is received, the tcp receiver task awakens us to process it.
 * Currently only the commands listed in the command table are accepted.
 * This routine never returns.
 */
converse(fi, fo, accept_call)
FILE *fi, *fo;
int accept_call;
{
	char greeting[MAXSTR];

	(void) signal(SIGALRM, alarmtr);
	(void) alarm(SHORTTIME);		/* make sure we eventually go away */
	if(accept_call)
		(void) sprintf(greeting, "220 %s SMTP\n", helohost);
	else
		(void) sprintf(greeting, "421 %s too busy, please try later\n", helohost);
	rcvrs = s_reset(rcvrs);
	(void) tputs(greeting, fo);

	do_helo(fi, fo);		/* wait for the hello */

	/*
	 *  avoid annoying interuptions
	 */
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	for (;;) {			/* until QUIT */
		do_mail(fi, fo);	/* wait for the mail command */
		while (do_rcpt(fi, fo))	/* do all the recipients */
			n_rcpt++;
		(void) alarm(LONGTIME);
		do_data(fi, fo);	/* do the data */
	}
}

/*
 *  Wait for the user to send the HELO command.  Punt out if he sends
 *  QUIT or RSET.
 *
 *  The spooling directory depends on the calling host.  The host name
 *  is used to connect to the appropriate spool directory.
 */
do_helo(fi, fo)
FILE *fi, *fo;
{
	char	cmdbuf[MAXSTR];
	char	greeting[MAXSTR];
	int	buflen;
	char	*hp;
	char	*parse_hello();

	for (;;) {		/* until HELO, QUIT, or RSET */
		buflen = tgets(cmdbuf, sizeof cmdbuf, fi);	/* wait for command */
		switch (cmdparse(cmdbuf, buflen)) {
		case QUIT:
		case RSET:
			quit(fi, fo);
		case NOOP:
			(void) tputs("250 OK\n", fo);
			continue;
		case HELO:
			hp = parse_hello(cmdbuf, sizeof(cmdbuf));
			if(gotodir(hp)<0){	
				(void) tputs("554 Transaction failed -- I/O error\n", fo);
				death(E_IOERR);
			}
			(void) sprintf(greeting, "250 %s\n", helohost);
			(void) tputs(greeting, fo);
			return;
		case DEBG:
			(void) logdanger("DEBUG");
			(void) tputs("200 OK\n", fo);
			virus = 1;
			continue;
		case NONE:
			bitch(cmdbuf, fo);
			continue;
		default:
			(void) tputs("503 Expecting HELO\n", fo);
			continue;
		}
	}
}

/*
 * Wait for the user to send the MAIL command.  Punt out if he sends
 * QUIT or RSET.
 */
do_mail(fi, fo)
FILE *fi, *fo;
{
	char	cmdbuf[MAXSTR];
	int	buflen;

	for (;;) {		/* until MAIL, QUIT, or RSET */
		buflen = tgets(cmdbuf, sizeof cmdbuf, fi);	/* wait for command */
		switch (cmdparse(cmdbuf, buflen)) {
		case QUIT:
		case RSET:
			quit(fi, fo);
		case NOOP:
			(void) tputs("250 OK\n", fo);
			continue;
		case MAIL:
			strcpy(mailfrom, cmdbuf);
			(void) tputs("250 OK\n", fo);
			return;
		case DEBG:
			(void) logdanger("DEBUG");
			(void) tputs("200 OK\n", fo);
			virus = 1;
			continue;
		case NONE:
			bitch(cmdbuf, fo);
			continue;
		default:
			(void) tputs("503 Expecting MAIL\n", fo);
			continue;
		}
	}
}

/*
 * Wait for the user to send the RCPT command.  Punt out if he sends
 * QUIT or RSET.  Returns TRUE if a RCPT command was received, FALSE
 * if a DATA command was received.
 */
do_rcpt(fi, fo)
FILE *fi, *fo;
{
	char	cmdbuf[MAXSTR];
	int	buflen;

	for (;;) {		/* until RCPT, DATA, QUIT, or RSET */
		buflen = tgets(cmdbuf, sizeof cmdbuf, fi);	/* wait for command */
		switch (cmdparse(cmdbuf, buflen)) {
		case QUIT:
		case RSET:
			quit(fi, fo);
		case NOOP:
			(void) tputs("250 OK\n", fo);
			continue;
		case RCPT:
#ifdef SIMPLELOG
			strcat(rcptto, cmdbuf);
#endif
			if (!parse_rcpt(cmdbuf, buflen)) {
				(void) tputs("501 Syntax error in recipient name\n", fo);
				continue;
			}
			(void) tputs("250 OK\n", fo);
			return(TRUE);
		case DATA:
			if (*s_to_c(rcvrs) == 0) {
				(void) tputs("503 Expecting RCPT\n", fo);
				continue;
			}
			if (!init_xfr()) {	/* set up data file */
				(void) tputs("451 Can't initialize transfer\n", fo);
				death(E_CANTOPEN);
			}
			(void) tputs("354 Start mail input; end with <CRLF>.<CRLF>\n", fo);
			return(FALSE);
		case DEBG:
			(void) logdanger("DEBUG");
			(void) tputs("200 OK\n", fo);
			virus = 1;
			continue;
		case NONE:
			bitch(cmdbuf, fo);
			continue;
		default:
			(void) tputs("503 Expecting RCPT or DATA\n", fo);
			continue;
		}
	}
}

/*
 *  input a line at a time till a <cr>.<cr>.  return the count of the characters
 *  input.  if EOF is reached, return -1.  if <cr>.<cr> is reached, return 0.
 */
static int atend;		/* true when <cr>.<cr> is reached */

char *
smfgets(buf, len, fi)
	char *buf;
	int len;
	FILE *fi;
{
	int n;
	int i;

	if(atend)
		return NULL;
	n = tgets(buf, len, fi);
	if (n < 0)
		return NULL;
	if (buf[0] == '.') {
		if(buf[1] == '\n'){
			atend = 1;
			return NULL;
		} else if(buf[1] == '.'){
			for(i=1; i<=n; i++)
				buf[i-1] = buf[i];
		}
	}
	nbytes += n;
	return buf;
}

do_data(fi, fo)
FILE *fi, *fo;
{
	string *cc;

	clearerr(fi);
	clearerr(datafd);

	/*
	 *  read data message
	 */
	atend = nbytes = 0;
	from822(thishost, smfgets, fi, datafd, fromaddr);
	fflush(datafd);
	if(ferror(datafd) || ferror(fi)){
		fclose(datafd);
		unlink(dataname);
		(void) tputs("554 Transaction failed -- error writing data file\n", fo);
		death(E_IOERR);
	}
	fclose(datafd);

	/*
	 *  create a control file.  the two lines are
	 *	<reply-address> <recipients>
	 *	<recipients>
	 */
	cc = s_new();
	s_append(cc, fromaddr ? fromaddr : "postmaster");
	s_append(cc, " ");
	s_append(cc, s_to_c(rcvrs));
	s_append(cc, "\n");
	s_append(cc, s_to_c(rcvrs));
	s_append(cc, "\n");
	if(mkctlfile('X', dataname, s_to_c(cc))<0){
		unlink(dataname);
		(void) tputs("554 Transaction failed -- can't make control file\n", fo);
		death(E_IOERR);
	}
	s_free(cc);
	(void) tputs("250 OK\n", fo);

	/*
	 *  reinitialize all the data pointers
	 */
	rcvrs = s_reset(rcvrs);
	*dataname = *rcptto = 0;
	fromaddr = 0;

}

/*
 * Create the data file for the transfer.  Get unique
 * names and create the files.
 */
init_xfr()
{
	int	dfd;			/* file desc. for data file */
	char	*cp;

	strcpy(dataname, "D.xxxxxxxxxxxx");
	if((dfd = mkdatafile(dataname)) < 0)
		return FALSE;
	datafd = fdopen(dfd, "w");	/* make stdio descriptor */
	if (datafd == NULL)
		return FALSE;

	/*
	 *  find the sender name if any
	 */
	if(*mailfrom){

		/* skip noise */
		for(cp=mailfrom+sizeof("MAIL FROM:")-1; *cp; cp++)
			/* RPE - added `$^& for security */
			if(strchr(";`$^&<>{}()\n| \t", *cp)==NULL)
				break;
		fromaddr = cp;

		/* find address */
		for(; *cp; cp++)
			/* RPE - added `$^& for security */
			if(strchr(";`$^&<>{}()\n| \t", *cp)!=NULL){
				*cp = '\0';
				break;
			}
	}
	if(fromaddr)
		fromaddr = convertaddr(fromaddr);
	
	return TRUE;
}

/*
 * Give up on the transfer.  Unlink the data file (if any),
 * close the tcp connection, and exit.
 */
quit(fi, fo)
FILE *fi, *fo;
{
	int i, maxfiles;
	char greeting[MAXSTR];

	(void) sprintf(greeting, "221 %s Terminating\n", helohost);
	(void) tputs(greeting, fo);
	(void) fclose(fi);
	(void) fclose(fo);
	xferstatus("finished", nbytes);

	/*
	 *  run the queue from this caller
	 */
	if ( (maxfiles=ulimit(4, 0)) < 0 )
#ifndef _NFILE
# define _NFILE 20
#endif
		maxfiles = _NFILE;

        for(i = 0; i < maxfiles; i++)
		(void) close(i);
	(void) open("/dev/null", O_RDWR);
	(void) dup(0);
	(void) dup(0);
	if(!norun)
		smtpsched("Dsmtpsched", spoolsubdir);

	exit(0);
}

/*
 * Parse the command part off the specified buffer.  Return the strchr
 * of the command in the command table(or 0 if the command is not
 * recognized).
 * The commands and indices accepted are listed in the include file
 * "cmds.h".
 * If the len parameter is -1 (as returned by tgets), issue the QUIT command.
 * This non-protocol extension was added to cool the jets of sail.stanford.edu.
 */
cmdparse(buf, len)
char *buf;
int len;
{
	register char *cmdp, *bufp;	/* command, buffer ptrs. */
	register struct	cmdtab	*ct;	/* cmd table ptr */
	register int i;			/* strchr in cmd table */
	int	clen;			/* length of this command */
	
	if (len == -1) {	/* EOF */
		buf = "QUIT";
		len = strlen(buf);
	}
	for (ct = &cmdtab[1], i = 1; ct->c_name != NULL; ct++, i++) {
		clen = ct->c_len;
		if (len < clen)		/* buffer shorter than command? */
			continue;
		/* case-insensitive matching of command names */
		for (cmdp = ct->c_name, bufp = buf;
		     clen > 0 && *cmdp == (ISLOWER(*bufp) ? TOUPPER(*bufp) : *bufp);
		     cmdp++, bufp++, clen--)
			;
		if (clen == 0) {		/* success */
			/* sendmail compatibility */
			if (i == ONEX || i == VERB)
				i = NOOP;
			return i;
		}
	}
	return 0;
}

/*
 *  Parse a hello and return a pointer to name of the last two elements
 *  of the calling machine's domain name (or last 14 chars).
 */
char *
parse_hello(buf, len)
	char *buf;
	int len;
{
	char *bp = buf;
	char *lp;

	/* skip command */
	bp[len-1] = 0;
	for(; *bp && !isspace(*bp); bp++)
		;
	/* skip white */
	for(; isspace(*bp); bp++)
		;
	/* skip arg */
	lp = bp;
	for(; *bp && !isspace(*bp); bp++)
		;
	/* null terminate */
	*bp = 0;

	return lp;
}

/*
 * Parse the recipient spec in the buffer.  Start by stripping the
 * command off the front of the buffer.  Then call canon() to convert
 * the recpient name into a format acceptable to the mailer daemon
 * (ie. !-format).
 * Returns TRUE if parsed successfully, FALSE otherwise.
 */
/* ARGSUSED len */
parse_rcpt(buf, len)
char *buf;				/* command buffer */
int len;				/* size of buffer string */
{
	register char *from;		/* ptr to recipient name */
	char *end;
	char *rv;
	char *sysname_read();
	char *thissys;
	
	from = &buf[cmdtab[RCPT].c_len];
	while (*from == ' ' || *from == '\t')
		from++;
	if (*from == '<') {
		end = strchr(from++, '>');
		if (end == 0) {
#ifdef HOOTING
			(void) fprintf(stderr, "no > at end of string\n");
#endif
			return FALSE;
		}
		*end = 0;
	}

	/*
	 *  convert to lower case (this is wrong but rfc822 is case
	 *  insensitive)
	 */
	for(rv = from; *rv; rv++)
		if(isupper(*rv))
			*rv = tolower(*rv);

	/*
	 * convert address to bang format.  Assume the first site
	 * in the list is us and take it out.
	 */
	rv=convertaddr(from);
	if(end=strchr(rv, '!')){
		thissys = sysname_read();
		*end = '\0';
		if(strcmp(rv, thissys)==0)
			rv = end+1;
		else
			*end = '!';
	}

	/*
	 *  check for address syntax
 	 */
	if(shellchars(rv)){
		logdanger("syntax error: %s", rv);
		if(virus)
			rv = "upas.security";
		else
			return FALSE;
	}

	/*
	 *  add to list of recipients
	 */
	if(*s_to_c(rcvrs))
		s_append(rcvrs, " ");
	s_append(rcvrs, rv);
	return TRUE;
}


/* Time to live elapsed or io error. */
death(weapon)
{
#ifdef SIMPLELOG
	simplelog(weapon);
#endif
	(void) unlink(dataname);
	xferstatus("bombed", weapon);
	exit(1);
}

alarmsend()
{
	(void) tputs("451 Our mailer appears to be hung.\n", abortfd);
	close(fileno(abortfd));
	death(E_TEMPFAIL);
}

funnychars(str)
register char *str;
{
	for (;;)
		switch(*str++) {
		case '^':
		case '&':
		case '>':
		case '<':
		case '`':
		case '|':
		case ';':
		case '\'':
			return TRUE;

		case 0:
			return FALSE;
		}
}

#ifdef SIMPLELOG
simplelog(retcode)
{
	char buf[1024], *bptr, *status;
	int fd;
	time_t t;
	extern char *ctime();

	(void) time(&t);
	switch (retcode) {
	case E_CANTOPEN:
		status = "OPEN FAILED";
		break;
	case E_IOERR:
		status = "IO ERROR";
		break;
	case E_TEMPFAIL:
		status = "TIMED OUT";
		break;
	case 0:
		status = "OK";
		break;
	default:
		status = "DELIVERY FAILURE";
		break;
	}
	if (*mailfrom == 0)
		strcpy(mailfrom, "UNKNOWN");
	if (*rcptto == 0)
		strcpy(rcptto, "UNKNOWN");
	(void) sprintf(buf, "%64s %64s %64s %64s", mailfrom, rcptto, status, ctime(&t));
	for (bptr = buf; *bptr; bptr++)
		if (*bptr == '\n' || *bptr == '\r')
			*bptr = ' ';
	strcat(bptr, "\n");
	if ((fd = open("/usr/spool/mail/mail.log", O_WRONLY|O_APPEND|O_CREAT, 0666)) >= 0) {
		(void) write(fd, buf, strlen(buf));
		(void) close(fd);
	}
}
#endif

bitch(buf, fo)
char *buf;
FILE *fo;
{
	char gripe[BUFSIZ], *nlptr;

	if ((nlptr = strchr(buf, '\n')) != 0)
		*nlptr = 0;
	(void) sprintf(gripe, "502 %s ... Not recognized\n", buf);
	(void) tputs(gripe, fo);
}

#ifndef NOBOMB
bomb(err)
int err;
{
	death(err);
}
#endif
