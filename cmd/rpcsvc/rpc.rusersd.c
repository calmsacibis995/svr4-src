/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rpcsvc:rpc.rusersd.c	1.12.1.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#ifndef lint
static	char sccsid[] = "@(#)rpc.rusersd.c 1.4 89/04/06 Copyr 1984 Sun Micro";
#endif

/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <rpc/rpc.h>
#include <memory.h>
#include <netconfig.h>
#include <stropts.h>
#ifdef SYSLOG
#include <syslog.h>
#else
#define LOG_ERR 1
#define openlog(a, b, c)
#endif
#include <utmp.h>
#include <rpcsvc/rusers.h>
#include <sys/resource.h>

#ifdef DEBUG
#define RPC_SVC_FG
#endif

#define _RPCSVC_CLOSEDOWN 120

static void rusers_service();
static void closedown();
static void msgout();
static unsigned min();

static int _rpcpmstart;		/* Started by a port monitor ? */
static int _rpcfdtype;		/* Whether Stream or Datagram ? */
static int _rpcsvcdirty;	/* Still serving ? */


#define	DIV60(t)	((t+30)/60)	/* x/60 rounded */
#define MAXINT 0x7fffffff
#define NLNTH 8			/* sizeof ut_name */

struct utmparr utmparr;
struct utmpidlearr utmpidlearr;
int cnt;

main()
{
	pid_t pid;
	int i;
	char mname[FMNAMESZ + 1];

	if (!ioctl(0, I_LOOK, mname) &&
		(!strcmp(mname, "sockmod") || !strcmp(mname, "timod"))){
		char *netid;
		struct netconfig *nconf = NULL;
		SVCXPRT *transp;
		int pmclose;
		extern char *getenv();

		_rpcpmstart = 1;
		if ((netid = getenv("NLSPROVIDER")) == NULL) {
			msgout("cannot get transport name");
		} else if ((nconf = getnetconfigent(netid)) == NULL) {
			msgout("cannot get transport info");
		}
		if (strcmp(mname, "sockmod") == 0) {
			if (ioctl(0, I_POP, 0) || ioctl(0, I_PUSH, "timod")) {
				msgout("could not get the right module");
				exit(1);
			}
		}
		pmclose = (t_getstate(0) != T_DATAXFER);
		if ((transp = svc_tli_create(0, nconf, NULL, 0, 0)) == NULL) {
			msgout("cannot create server handle");
			exit(1);
		}
		if (nconf)
			freenetconfigent(nconf);
		if (!svc_reg(transp, RUSERSPROG, RUSERSVERS_ORIG, rusers_service, 0)) {
			msgout("unable to register (RUSERSPROG, RUSERSVERS_ORIG).");
			exit(1);
		}
		if (!svc_reg(transp, RUSERSPROG, RUSERSVERS_IDLE, rusers_service, 0)) {
			msgout("unable to register (RUSERSPROG, RUSERSVERS_IDLE).");
			exit(1);
		}
		if (pmclose) {
			(void) signal(SIGALRM, closedown);
			(void) alarm(_RPCSVC_CLOSEDOWN);
		}
		svc_run();
		msgout("svc_run returned");
		exit(1);
		/* NOTREACHED */
	}
#ifndef RPC_SVC_FG
	pid = fork();
	if (pid < 0) {
		perror("cannot fork");
		exit(1);
	}
	if (pid)
		exit(0);
	for (i = 0 ; i < 20; i++)
		(void) close(i);
	setsid();
	openlog("rusers", LOG_PID, LOG_DAEMON);
#endif
	if (!svc_create(rusers_service, RUSERSPROG, RUSERSVERS_ORIG, "netpath")) {
 		msgout("unable to create (RUSERSPROG, RUSERSVERS_ORIG) for netpath.");
		exit(1);
	}
	if (!svc_create(rusers_service, RUSERSPROG, RUSERSVERS_IDLE, "netpath")) {
 		msgout("unable to create (RUSERSPROG, RUSERSVERS_IDLE) for netpath.");
		exit(1);
	}

	svc_run();
	msgout("svc_run returned");
	exit(1);
	/* NOTREACHED */
}

getutmp(all, idle)
	int all;		/* give all listings? */
	int idle;		/* get idle time? */
{
	struct utmp buf; struct ru_utmp **p;
	struct utmpidle **q, *console;
	int minidle;
	FILE *fp;
	char *file;
	char name[NLNTH];
	
	cnt = 0;
	file = "/etc/utmp";
	if ((fp = fopen(file, "r")) == NULL) {
		fprintf(stderr, "can't open %s\n", file);
		exit(1);
	};
	p = utmparr.uta_arr;
	q = utmpidlearr.uia_arr;
	while (fread(&buf, sizeof(buf), 1, fp) == 1) {
		if (buf.ut_line[0] == 0 || buf.ut_name[0] == 0)
			continue;
		/* 
		 * if tty[pqr]? and not remote, then skip it
		 */
#if !defined(u3b2) && !defined(i386)
		if (!all && nonuser(buf))
			continue;
#else
/*sy5*/
		if (!all && (buf.ut_type != USER_PROCESS))
			continue;
#endif
		/* 
		 * need to free this
		 */
		if (idle) {
			*q = (struct utmpidle *) malloc(sizeof(struct utmpidle));
			if (strncmp(buf.ut_line, "console",
				strlen("console")) == 0) {
				console = *q;
				strncpy(name, buf.ut_name, NLNTH);
			}
			usys5to_ru(&buf, &((*q)->ui_utmp));
			/*bcopy(&buf, &((*q)->ui_utmp), sizeof(buf));*/
			(*q)->ui_idle = findidle(buf.ut_line,
						sizeof(buf.ut_line));
#ifdef DEBUG
			printf("%-10s %-10s  %s; idle %d",
					buf.ut_line, buf.ut_name,
					ctime(&buf.ut_time),
					(*q)->ui_idle);
#endif
			q++;
		}
		else {
			*p = (struct ru_utmp *)malloc(sizeof(struct ru_utmp));
			 usys5to_ru(&buf, *p);
			/*bcopy(&buf, *p, sizeof(buf));*/
#ifdef DEBUG
			printf("%-10s %-10s  %s",
					buf.ut_line, buf.ut_name,
					ctime(&buf.ut_time));
#endif
			p++;
		}
		cnt++;
	}
	/* 
	 * On the console, the user may be running a window system; if so,
	 * their activity will show up in the last-access times of
	 * "/dev/kbd" and "/dev/mouse", so take the minimum of the idle
	 * times on those two devices and "/dev/console" and treat that as
	 * the idle time.
	 */
	if (idle && console)
		console->ui_idle = min(console->ui_idle,
 			min((unsigned)findidle("kbd", strlen("kbd")),
 			    (unsigned)findidle("mouse", strlen("mouse"))));
 
}

static void
rusers_service(rqstp, transp)
	register struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	_rpcsvcdirty = 1;
	switch (rqstp->rq_proc) {
	case 0:
		if (svc_sendreply(transp, xdr_void, 0) == FALSE) {
			fprintf(stderr, "err: rusersd");
		}
		break;
	case RUSERSPROC_NUM:
		utmparr.uta_arr = (struct ru_utmp **)
				malloc(MAXUSERS*sizeof(struct ru_utmp *));
		getutmp(0, 0);
		if (!svc_sendreply(transp, xdr_u_long, &cnt))
			perror("svc_rpc_send_results");
		free(utmparr.uta_arr);
		break;
	case RUSERSPROC_NAMES:
	case RUSERSPROC_ALLNAMES:
		if (rqstp->rq_vers == RUSERSVERS_ORIG) {
			utmparr.uta_arr = (struct ru_utmp **)
				malloc(MAXUSERS*sizeof(struct ru_utmp *));
			getutmp(rqstp->rq_proc == RUSERSPROC_ALLNAMES, 0);
			utmparr.uta_cnt = cnt;
			if (!svc_sendreply(transp, xdr_utmparr, &utmparr))
				perror("svc_rpc_send_results");
			svc_freeargs(transp, xdr_utmparr, &utmparr);
			free(utmparr.uta_arr);
		} else {
			utmpidlearr.uia_arr = (struct utmpidle **)
				malloc(MAXUSERS*sizeof(struct utmpidle *));
			getutmp(rqstp->rq_proc == RUSERSPROC_ALLNAMES, 1);
			utmpidlearr.uia_cnt = cnt;
			if (!svc_sendreply(transp, xdr_utmpidlearr, &utmpidlearr))
				perror("svc_rpc_send_results");
			svc_freeargs(transp, xdr_utmpidlearr, &utmpidlearr);
			free(utmpidlearr.uia_arr);
		}
		break;
	default: 
		svcerr_noproc(transp);
		break;
	}
	_rpcsvcdirty = 0;

}

/* find & return number of minutes current tty has been idle */
findidle(name, ln)
	char *name;
	int ln;
{
	time_t	now;
	struct stat stbuf;
	long lastaction, diff;
	char ttyname[20];

	strcpy(ttyname, "/dev/");
	strncat(ttyname, name, ln);
	if (stat(ttyname, &stbuf) < 0)
		return(MAXINT);
	time(&now);
	lastaction = stbuf.st_atime;
	diff = now - lastaction;
	diff = DIV60(diff);
	if (diff < 0) diff = 0;
	return(diff);
}

static
usys5to_ru(s5,bss)
	struct utmp *s5;
	struct ru_utmp *bss;
{
	int i;

	strncpy(bss->ut_name, s5->ut_name, 8);
	strncpy(bss->ut_line, s5->ut_line, 8);
	for (i = 0; i < 16; i++)
		bss->ut_host[i] = (char)0;
	bss->ut_time=s5->ut_time;
}

static void
msgout(msg)
	char *msg;
{
#ifdef RPC_SVC_FG
	if (_rpcpmstart)
		syslog(LOG_ERR, msg);
	else
		(void) fprintf(stderr, "%s\n", msg);
#else
	syslog(LOG_ERR, msg);
#endif
}

static void
closedown()
{
	if (_rpcsvcdirty == 0) {
		extern fd_set svc_fdset;
		static struct rlimit rl;
		int i, openfd;
		struct t_info tinfo;

		if (t_getinfo(0, &tinfo) || (tinfo.servtype == T_CLTS))
			exit(0);
		if (rl.rlim_max == 0)
			getrlimit(RLIMIT_NOFILE, &rl);
		for (i = 0, openfd = 0; i < rl.rlim_max && openfd < 2; i++)
			if (FD_ISSET(i, &svc_fdset))
				openfd++;
		if (openfd <= 1)
			exit(0);
	}
	(void) alarm(_RPCSVC_CLOSEDOWN);
}
 
 unsigned
 min(a, b)
 unsigned a;
 unsigned b;
 {
 	if (a < b)
 	  return(a);
 	else
 	  return(b);
}
