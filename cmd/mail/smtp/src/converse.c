/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/converse.c	1.3.3.1"
/*
 * Do the necessary commands for a smtp transfer.  Start by waiting for the
 * connection to open, then send HELO, MAIL, RCPT, and DATA.  Check the
 * reply codes and give up if needed.
 * 
 * This code modified from the MIT UNIX TCP implementation:
 * Copyright 1984 Massachusetts Institute of Technology
 * 
 * Permission to use, copy, modify, and distribute this file
 * for any purpose and without fee is hereby granted, provided
 * that this copyright and permission notice appear on all copies
 * and supporting documentation, the name of M.I.T. not be used
 * in advertising or publicity pertaining to distribution of the
 * program without specific prior permission, and notice be given
 * in supporting documentation that copying and distribution is
 * by permission of M.I.T.  M.I.T. makes no representations about
 * the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 */

#include <stdio.h>
#include <signal.h>
#include "xmail.h"
#include "smtp.h"
#include "miscerrs.h"
#include "s_string.h"

#define TRUE 1
#define FALSE 0
#define MINUTES * 60
#define HOURS * 60 MINUTES
#define DAYS * 24 HOURS

char *strcat(), *strcpy();
#ifdef	BSD
char *sprintf();
#else	/*SV(ID)*/
#ifndef SVR4
int sprintf();
#endif
#endif

SIGRETURN ignore_signal();
SIGRETURN death();
char	*timemsg;
int	timelim;

converse(unixformat, from, rcpts, domain, sfi, sfo, mlfd)
char	*from;				/* from address */
namelist *rcpts;			/* to addresses */
char	*domain;
FILE	*sfi;				/* smtp input */
FILE	*sfo;				/* smtp output */
FILE	*mlfd;				/* mail file descriptor */
{
	extern expect();
	extern char *helohost;
	char buf[MAXSTR];
	namelist *np;

	(void) signal(SIGALRM, death);

	setalarm(5 MINUTES, "initial handshake");
	expect(220, sfi, sfo);			/* expect a service ready msg */

	(void) sprintf(buf, "HELO %s\n", helohost);
	tputs(buf, sfo);
	expect(250, sfi, sfo);			/* expect an OK */

	(void) strcpy(buf, "MAIL FROM:<");
	(void) strcat(buf, from);
	(void) strcat(buf, ">\n");
	tputs(buf, sfo);
	setalarm(10 MINUTES, "response to MAIL FROM/RCPT TO");
	expect(250, sfi, sfo);			/* expect OK */

	for (np=rcpts; np!=NULL; np=np->next) {
		(void) strcpy(buf, "RCPT TO:<");
		(void) strcat(buf, np->name);
		(void) strcat(buf, ">\n");
		tputs(buf, sfo);
		expect(250, sfi, sfo);		/* expect OK */
	}
	setalarm(10 MINUTES, "response to DATA");
	tputs("DATA\n", sfo);
	expect(354, sfi, sfo);
	setalarm(10 MINUTES, "sending mail data");
	do_data(unixformat, mlfd, sfo, from, rcpts, domain);
	setalarm(1 HOURS, "expecting delivery ack");
	expect(250, sfi, sfo);

	tputs("QUIT\n", sfo);
	setalarm(5 MINUTES, "response to QUIT");
	expect(221, sfi, sfo);			/* who cares? (Some do -ches)*/
}

/*
 *  escape '.'s at the beginning of lines and turn newlines into
 *  /r/n's.
 */
static char smlastchar;

smfputs(str, fp)
	char *str;
	FILE *fp;
{
	register char *cp;
	extern int debug;

	/*
	 *  escape a leading dot
	 */
	if(smlastchar=='\n' && str[0]=='.') {
		fputc('.', fp);
		if (debug>1)
			(void) putc('.', stderr);
	}

	/*
	 *  output the line
	 */
	for(cp=str; *cp; cp++){
		if(*cp=='\n')
			putc('\r', fp);
		putc(*cp, fp);
	}
	if(cp!=str)
		smlastchar = *(cp-1);
}


/*
 * Send the data from the specified mail file out on the current smtp
 * connection.  Do the appropriate netascii conversion and hidden '.'
 * padding.  Send the <CRLF>.<CRLF> at completion.
 */
do_data(unixformat, sfi, sfo, from, rcpts, domain)
	register FILE *sfi;		/* mail file descriptor */
	register FILE *sfo;		/* smtp files */
	char *from;
	namelist *rcpts;
	char *domain;
{
	extern int debug;
	static string *rcvr;
	char buf[4096];
	namelist *p;
	long nchars;

	/*
	 *  turn rcpts into a , list of receivers
	 */
	rcvr = s_reset(rcvr);
	for(p = rcpts; p; p = p->next){
		s_append(rcvr, p->name);
		if(p->next)
			s_append(rcvr, ", ");
	}

	/*
	 *  send data to output
	 */
	setalarm(5 MINUTES, "start sending mail data");
	smlastchar = '\n';
	if(unixformat){
		nchars = 0;
		while(fgets(buf, sizeof(buf), sfi)!=NULL) {
			smfputs(buf, sfo);
			nchars += strlen(buf)+1;
			if (nchars>1024) {
				nchars -= 1024;
				setalarm(5 MINUTES, "sending mail data");
				if (debug)
					putc('.', stderr);
			}
		}
	} else {
		if(to822(smfputs, sfi, sfo, from, domain, s_to_c(rcvr))<0){
			fprintf(stderr, "bad input file\n");
			bomb(E_IOERR);
		}
	}

	/*
	 *  terminate the DATA command with \r\n.\r\n
	 */
	if(smlastchar != '\n'){
		fputs("\r\n", sfo);
		if(debug)
			fputs("\n", stderr);
	}
	fputs(".\r\n", sfo);

	/*
	 *  see if we screwed up
	 */
	setalarm(30 MINUTES, "finishing data");
	fflush(sfo);
	if (ferror(sfo)) {
		perror("write error in smtp");
		bomb(E_IOERR);
	}
	if(debug)
		fputs("\nFinished sending.\n", stderr);
}

/*
 * Expect a reply message with the specified code.  If the specified code
 * is received return TRUE; otherwise print the error message out on the
 * standard output and give up.  Note that the reply can be a multiline
 * message.
 */
expect(code, sfi, sfo)
int	code;
FILE	*sfi, *sfo;
{
	int retcd;
	char cmdbuf[BUFSIZ];
	extern int debug;

	if (debug)
		(void) fprintf(stderr, "expect %d ", code);
	/* get whole reply */
more:
	while (tgets(cmdbuf, sizeof cmdbuf, sfi) > 0) {	/* get input line */
		if (cmdbuf[3] != '-')	/* continuation line? */
			break;		/* no, last line */
	}
	if (sscanf(cmdbuf, "%d", &retcd) !=1 ){
		int l=strlen(cmdbuf)-1;
		if (l>=0 && cmdbuf[l]=='\n')
			cmdbuf[l]='\0';
		if (debug)
			(void) fprintf(stderr, "non-numeric command reply (%s)\n", cmdbuf);
		goto more;
	}
	if (retcd == code) {
		if (debug)
			(void) fprintf(stderr, "got it\n");
		return 1;
	}
	if (retcd/100 == code/100) {
		if (debug)
			(void) fprintf(stderr, "got it close enough (%d)\n", retcd);
		return 1;
	}
	if (debug)
		(void) fprintf(stderr, "FAIL (got %d)\n", retcd);
	/* print the error line */
	(void) fprintf(stderr, "<<< %s", cmdbuf);
	tputs ("QUIT\n", sfo);
	bomb(retcd);		/* map smtp errors to mailsys errors */
}

setalarm(limit, message)
	char *message;
{
	timelim = limit;
	timemsg = message;
	alarm(limit);
}

/* Maximum time to live elapsed.  Die right now. */
SIGRETURN
death()
{
	(void) fprintf(stderr, "Timer (%d sec) expired: %s.\n", timelim, timemsg);
	exit(1);
}

SIGRETURN
ignore_signal(){}
