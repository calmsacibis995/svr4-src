/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:cmd/lpsched/lpNet/bsdChild/netutil.c	1.5.3.1"

#include <unistd.h>
#include <stdio.h>
#if	defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <tiuser.h>
#include <setjmp.h>
#include <signal.h>
#include "lp.h"
#include "lpNet.h"
#include "lpd.h"

static	jmp_buf	 Env;
static	FILE	*fpRemote;

/*
 * SIGALRM handler
 */
/*ARGSUSED*/
static void
#if defined (__STDC__)
noconnect(int s)
#else
noconnect(s)
int	s;
#endif
{
	longjmp(Env, 1);
}

#if defined (__STDC__)
contimeout(void)
#else
contimeout()
#endif
{
	if (SIP->timeout > 0) {
		if (setjmp(Env)) {
			logit(LOG_INFO, "no connection");
			return(1);
		}
		(void)signal(SIGALRM, noconnect);
		alarm(SIP->timeout*60);
	}
	return(0);
}
/*
 * Used to establish outgoing connections.
 * (doesn't need stderr)
 */
#if defined (__STDC__)
openRemote(void)
#else
openRemote()
#endif
{
	int	ev;

	if (CONNECTED_TO_REMOTE) {
		logit(LOG_WARNING, "%s already connected to %s", Name, Rhost);
		if ((ev = t_look(CIP->fd)) < 0 ||
		     ev & (T_ERROR|T_DISCONNECT|T_ORDREL)) {
			DisconnectSystem(CIP);
			FreeConnectionInfo(&CIP);
		}
	}
 	if (!CONNECTED_TO_REMOTE && contimeout())
 		return(0);
 	while (!CONNECTED_TO_REMOTE) {
 		if (CIP = ConnectToSystem(SIP)) {
 			if (ioctl(CIP->fd, I_PUSH, "tirdwr") < 0) {
 				if ((ev = t_look(CIP->fd)) < 0 ||
 		     		     ev & (T_ERROR|T_DISCONNECT|T_ORDREL))
 					logit(LOG_WARNING, "disconnected");
 				else
 					logit(LOG_ERR,
					"%s can't PUSH tirdwr: %s",
					Name, PERROR);
				DisconnectSystem(CIP);
 				FreeConnectionInfo(&CIP);
 			} else
 				break;
 		}
 		if (!SIP->timeout || SIP->retry < 0) {
 			alarm(0);
  			return(0);
  		}
 		if (SIP->retry)
 			(void)sleep(SIP->retry*60);
 		logit(LOG_INFO, "%s retrying connection to %s", Name, Rhost);
  	}
 	alarm(0);
 	logit(LOG_INFO, "%s connected to %s", Name, Rhost); 
  	return(1);
}
/*
 * Close remote connection and free resources
 */
void
#if defined (__STDC__)
closeRemote(void)
#else
closeRemote()
#endif
{
	if (CONNECTED_TO_REMOTE) {
		logit(LOG_INFO, "%s disconnecting from %s", Name, Rhost); 
		DisconnectSystem(CIP);
		FreeConnectionInfo(&CIP);
		if (fpRemote) {
			(void)fclose(fpRemote);
			fpRemote = NULL;
		}
	}
}

/*
 * Read line delimited by newline from network connection
 */
char *
#if defined (__STDC__)
getNets(char *buf, int bufsize)
#else
getNets(buf, bufsize)
char	*buf;
int	 bufsize;
#endif
{
	if (!fpRemote && !(fpRemote = fdopen(CIP->fd, "r+")))
		return(NULL);
	return(fgets(buf, bufsize, fpRemote));
}

/*
 * Send lpd message to remote
 */
/*VARARGS1*/
#if defined (__STDC__)
snd_lpd_msg(int type, ...)
#else
snd_lpd_msg(type, va_alist)
int	type;
va_dcl
#endif
{
	va_list	 argp;
	char	*printer;
	char	*fname;
	char	*person;
	char	*users;
	char	*jobs;
	size_t	 size;
	int	 n;

#if defined (__STDC__)
	va_start(argp, type);
#else
	va_start(argp);
#endif

	switch(type) {

	case PRINTJOB:
	case RECVJOB:
		n = sprintf(Buf, "%c%s\n", type, va_arg(argp, char *));
		break;

	case DISPLAYQS:
	case DISPLAYQL:
		printer = va_arg(argp, char *);
		users = va_arg(argp, char *);
		jobs = va_arg(argp, char *);
		n = sprintf(Buf, "%c%s %s %s\n", type, printer, users, jobs);
		break;

	case RMJOB:
		printer = va_arg(argp, char *);
		person = va_arg(argp, char *);
		users = va_arg(argp, char *);
		jobs = va_arg(argp, char *);
		n = sprintf(Buf, "%c%s %s %s %s\n", 
				type, printer, person, users, jobs);
		break;

	case RECVJOB_2NDARY:
		type = va_arg(argp, int);

		switch(type) {

		case CLEANUP:
			n = sprintf(Buf, "%c\n", type);
			break;

		case READCFILE:
		case READDFILE:
			size = va_arg(argp, size_t);
			fname = va_arg(argp, char *);
			n = sprintf(Buf, "%c%lu %s\n", type, size, fname);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
	va_end(argp);
	logit(LOG_DEBUG, "sending %d byte lpd message: %d%s", n, *Buf, Buf+1);
	if (write(CIP->fd, Buf, n) != n) {
		logit(LOG_INFO, "lost connection");
		return(0);
	}
	return(1);
}
