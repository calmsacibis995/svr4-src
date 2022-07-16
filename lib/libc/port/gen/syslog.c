/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/syslog.c	1.6"

/* from "@(#)syslog.c 1.18 88/02/08 SMI"; from UCB 5.9 5/7/86 */

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 * SYSLOG -- print message on log file
 *
 * This routine looks a lot like printf, except that it
 * outputs to the log file instead of the standard output.
 * Also:
 *	adds a timestamp,
 *	prints the module name in front of the message,
 *	has some other formatting types (or will sometime),
 *	adds a newline on the end of the message.
 *
 * The output of this routine is intended to be read by /etc/syslogd.
 */

#ifndef DSHLIB
#ifdef __STDC__
	#pragma weak syslog = _syslog
	#pragma weak vsyslog = _vsyslog
	#pragma weak openlog = _openlog
	#pragma weak closelog = _closelog
	#pragma weak setlogmask = _setlogmask
#endif
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <varargs.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
/*#include <vfork.h>*/

#define	MAXLINE	1024			/* max message size */
#define NULL	0			/* manifest */

#define PRIMASK(p)	(1 << ((p) & LOG_PRIMASK))
#define PRIFAC(p)	(((p) & LOG_FACMASK) >> 3)
#define IMPORTANT 	LOG_ERR

#define logname "/dev/conslog"
#define ctty "/dev/syscon"

static struct __syslog {
	int	_LogFile;
	int	_LogStat;
	const char	*_LogTag;
	int	_LogMask;
	char	*_SyslogHost;
	int	_LogFacility;
} *__syslog;

#define	LogFile (__syslog->_LogFile)
#define	LogStat (__syslog->_LogStat)
#define	LogTag (__syslog->_LogTag)
#define	LogMask (__syslog->_LogMask)
#define	SyslogHost (__syslog->_SyslogHost)
#define	LogFacility (__syslog->_LogFacility)

extern	int errno;

/*VARARGS2*/
syslog(pri, fmt, va_alist)
	int pri;
	char *fmt;
	va_dcl
{
	va_list ap;

	va_start(ap);
	vsyslog(pri, fmt, ap);
	va_end(ap);
}

vsyslog(pri, fmt, ap)
	int pri;
	char *fmt;
	va_list ap;
{
	register char *b, *f, *o;
	register int c;
	char buf[MAXLINE + 1], outline[MAXLINE + 1];
	long now;
	pid_t pid;
	struct log_ctl hdr;
	struct strbuf dat;
	struct strbuf ctl;
	sigset_t sigs;
	int olderrno = errno;

	if (__syslog == 0)
		return;
	/* see if we should just throw out this message */
	if (pri <= 0 || PRIFAC(pri) >= LOG_NFACILITIES || (PRIMASK(pri) & LogMask) == 0)
		return;
	if (LogFile < 0)
		openlog(LogTag, LogStat | LOG_NDELAY, 0);

	/* set default facility if none specified */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* build the header */
	hdr.pri = pri;
	hdr.flags = SL_CONSOLE;
	hdr.level = 0;

	/* build the message */
	o = outline;
	time(&now);
	sprintf(o, "%.15s ", ctime(&now) + 4);
	o += strlen(o);
	if (LogTag) {
		strcpy(o, LogTag);
		o += strlen(o);
	}
	if (LogStat & LOG_PID) {
		sprintf(o, "[%d]", getpid());
		o += strlen(o);
	}
	if (LogTag) {
		strcpy(o, ": ");
		o += 2;
	}

	b = buf;
	f = fmt;
	while ((c = *f++) != '\0' && c != '\n' && b < &buf[MAXLINE]) {
		char *errmsg;
		if (c != '%') {
			*b++ = c;
			continue;
		}
		if ((c = *f++) != 'm') {
			*b++ = '%';
			*b++ = c;
			continue;
		}
		if ((errmsg = strerror(olderrno)) == NULL)
			sprintf(b, "error %d", olderrno);
		else
			strcpy(b, errmsg);
		b += strlen(b);
	}
	*b++ = '\n';
	*b = '\0';
	vsprintf(o, buf, ap);
	c = strlen(outline) + 1;	/* add one for NULL byte */
	if (c > MAXLINE)
		c = MAXLINE;

	/* set up the strbufs */
	ctl.maxlen = sizeof(struct log_ctl);
	ctl.len = sizeof(struct log_ctl);
	ctl.buf = (caddr_t)&hdr;
	dat.maxlen = sizeof(outline);
	dat.len = c;
	dat.buf = outline;

	/* output the message to the local logger */
	if (putmsg(LogFile, &ctl, &dat, 0) >= 0)
		return;
	if (!(LogStat & LOG_CONS))
		return;

	/* output the message to the console */
	pid = vfork();
	if (pid == -1)
		return;
	if (pid == 0) {
		int fd;

		signal(SIGALRM, SIG_DFL);
		sigprocmask(SIG_BLOCK, NULL, &sigs);
		sigdelset(&sigs, SIGALRM);
		sigprocmask(SIG_SETMASK, &sigs, NULL);
		alarm(5);
		fd = open(ctty, O_WRONLY);
		if (fd >= 0) {
			alarm(0);
			strcat(o, "\r");
			write(fd, o, c + 1);
			close(fd);
		}
		_exit(0);
	}
	if (!(LogStat & LOG_NOWAIT))
		waitpid(pid, (int *)0, 0);
}

/*
 * OPENLOG -- open system log
 */

openlog(ident, logstat, logfac)
	char *ident;
	int logstat, logfac;
{
	if (__syslog == 0) {
		__syslog = (struct __syslog *)calloc(1, sizeof (struct __syslog));
		if (__syslog == 0)
			return;
		LogFile = -1;		/* fd for log */
		LogStat	= 0;		/* status bits, set by openlog() */
		LogTag = "syslog";	/* string to tag the entry with */
		LogMask = 0xff;		/* mask of priorities to be logged */
		LogFacility = LOG_USER;	/* default facility code */
	}
	if (ident != NULL)
		LogTag = ident;
	LogStat = logstat;
	if (logfac != 0)
		LogFacility = logfac & LOG_FACMASK;
	if (LogFile >= 0)
		return;
	if (LogStat & LOG_NDELAY) {
		LogFile = open(logname, O_WRONLY);
		fcntl(LogFile, F_SETFD, 1);
	}
}

/*
 * CLOSELOG -- close the system log
 */

closelog()
{

	if (__syslog == 0)
		return;
	(void) close(LogFile);
	LogFile = -1;
}

/*
 * SETLOGMASK -- set the log mask level
 */
setlogmask(pmask)
	int pmask;
{
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
}

