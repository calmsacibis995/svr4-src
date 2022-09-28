/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgremove/quit.c	1.5.3.1"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/utsname.h>
#include "install.h"

#define MAILCMD	"/usr/bin/mail"

extern struct admin
		adm;
extern int	started,
		failflag,
		warnflag,
		reboot, ireboot;
extern char	*prog,
		*msgtext,
		*pkginst;

extern void	*calloc(),
		exit(),
		logerr(),
		ptext();

static char	*reason();

void		trap(), quit();
static void	mailmsg(), quitmsg();

void
trap(signo)
int signo;
{
	if((signo == SIGINT) || (signo == SIGHUP))
		quit(3);
	else
		quit(1);
}

void
quit(retcode)
int	retcode;
{
	(void) signal(SIGINT, SIG_IGN);

	if(retcode != 99) {
		if((retcode % 10) == 0) {
			if(failflag)
				retcode += 1;
			else if(warnflag)
				retcode += 2;
		}

		if(ireboot)
			retcode = (retcode % 10) + 20;
		if(reboot)
			retcode = (retcode % 10) + 10;
	}

	/* send mail to appropriate user list */
	mailmsg(retcode);

	/* display message about this installation */
	quitmsg(retcode);
	exit(retcode);
	/*NOTREACHED*/
}

static void
quitmsg(retcode)
int	retcode;
{
	char	*status;

	status = reason(retcode);

	(void) putc('\n', stderr);
	ptext(stderr, "Removal of <%s> %s.", pkginst, status);

	if(retcode && !started)
		ptext(stderr, "No changes were made to the system.");
}

static void
mailmsg(retcode)
int retcode;
{
	struct utsname utsbuf;
	FILE	*pp;
	char	*status, *cmd;

	if(!started || (adm.mail == NULL))
		return;

	cmd = calloc(strlen(adm.mail) + sizeof(MAILCMD) + 2, sizeof(char));
	if(cmd == NULL) {
		logerr("WARNING: unable to send e-mail notofication");
		return;
	}

	(void) sprintf(cmd, "%s %s", MAILCMD, adm.mail);
	if((pp = popen(cmd, "w")) == NULL) {
		logerr("WARNING: unable to send e-mail notofication");
		return;
	}

	if(msgtext)
		ptext(pp, msgtext);

	(void) strcpy(utsbuf.nodename, "(unknown)");
	(void) uname(&utsbuf);
	status = reason(retcode);
	ptext(pp, "\nRemoval of <%s> package instance on %s %s.", pkginst,
		utsbuf.nodename, status);

	if(pclose(pp)) 
		logerr("WARNING: e-mail notification may have failed");
}

static char *
reason(retcode)
{
	char	*status;

	switch(retcode) {
	  case  0:
	  case 10:
	  case 20:
		status = "was successful";
		break;

	  case  1:
	  case 11:
	  case 21:
		status = "failed";
		break;

	  case  2:
	  case 12:
	  case 22:
		status = "partially failed";
		break;

	  case  3:
	  case 13:
	  case 23:
		status = "was terminated due to user request";
		break;

	  case  4:
	  case 14:
	  case 24:
		status = "was suspended (administration)";
		break;

	  case  5:
	  case 15:
	  case 25:
		status = "was suspended (interaction required)";
		break;

	  case 99:
		status = "failed (internal error)";
		break;

	  default:
		status = "failed with an unrecognized error code.";
		break;
	}

	return(status);
}
