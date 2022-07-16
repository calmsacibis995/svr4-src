/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbshutdown:shutdown.c	1.2.2.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <utmp.h>
#include <pwd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <nfs/nfs.h>
#include <rpcsvc/mount.h>
#include <rpcsvc/rwall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/syslog.h>

/*
 *	/usr/etc/shutdown when [messages]
 *
 *	allow super users to tell users and remind users
 *	of iminent shutdown of unix
 *	and shut it down automatically
 *	and even reboot or halt the machine if they desire
 */

#define	EPATH	"PATH=/usr/ucb:/bin:/usr/bin:/usr/etc:"
#define	REBOOT	"reboot"
#define	HALT	"halt"
#define MAXINTS 20
#define	HOURS	*3600
#define MINUTES	*60
#define SECONDS
#define NLOG		600		/* no of bytes possible for message */
#define	NOLOGTIME	5 MINUTES
#define IGNOREUSER	"sleeper"

struct hostlist {
    char *host;
    struct hostlist *nxt;
} *hostlist;

char	hostname[MAXHOSTNAMELEN];
char	buf[BUFSIZ];

void	timeout();
time_t	getsdt();

extern	char *malloc();

extern	char *ctime();
extern	struct tm *localtime();
extern	long time();

extern	char *strcpy();
extern	char *strncat();
extern	off_t lseek();

struct	utmp utmp;
int	sint;
int	stogo;
char	tpath[] =	"/dev/";
int	killflg = 1;
int	doreboot = 0;
int	halt = 0;
int	fast = 0;
char	*nosync = NULL;
char	nosyncflag[] = "-n";
char	term[sizeof tpath + sizeof utmp.ut_line];
char	tbuf[BUFSIZ];
#ifdef	DEBUG
char	fastboot[] = "fastboot";
#else
char	fastboot[] = "/fastboot";
#endif
time_t	nowtime;
jmp_buf	alarmbuf;

struct interval {
	int stogo;
	int sint;
} interval[] = {
	4 HOURS,	1 HOURS,
	2 HOURS,	30 MINUTES,
	1 HOURS,	15 MINUTES,
	30 MINUTES,	10 MINUTES,
	15 MINUTES,	5 MINUTES,
	10 MINUTES,	5 MINUTES,
	5 MINUTES,	3 MINUTES,
	2 MINUTES,	1 MINUTES,
	1 MINUTES,	30 SECONDS,
	0 SECONDS,	0 SECONDS
};

char *shutter, *getlogin();

main(argc,argv)
	int argc;
	char **argv;
{
	register i, ufd;
	register char *f;
	char *ts;
	time_t sdt;
	int h, m;
	int first;
	void finish_sig();
	FILE *termf;
	struct passwd *pw, *getpwuid();
	extern char *strcat();
	extern uid_t geteuid();
	struct hostlist *hl;
	char *shutdown_program;
	char *shutdown_action;

	shutter = getlogin();
	if (shutter == 0 && (pw = getpwuid(getuid())))
		shutter = pw->pw_name;
	if (shutter == 0)
		shutter = "???";
	(void) gethostname(hostname, sizeof (hostname));
	openlog("shutdown", 0, LOG_AUTH);
	argc--, argv++;
	while (argc > 0 && (f = argv[0], *f++ == '-')) {
		while (i = *f++) switch (i) {
		case 'k':
			killflg = 0;
			continue;
		case 'n':
			nosync = nosyncflag;
			continue;
		case 'f':
			fast = 1;
			continue;
		case 'r':
			doreboot = 1;
			continue;
		case 'h':
			halt = 1;
			continue;
		default:
			(void) fprintf(stderr,
			    "shutdown: '%c' - unknown flag\n", i);
			(void) fprintf(stderr,
			    "Usage: shutdown [ -krhfn ] shutdowntime [ message ]\n");
			finish("Usage: shutdown [ -krhfn ] shutdowntime [ message ]",
			    "", 1);
		}
		argc--, argv++;
	}
	if (argc < 1) {
		(void) fprintf(stderr,
		    "Usage: shutdown [ -krhfn ] shutdowntime [ message ]\n");
		finish("Usage: shutdown [ -krhfn ] shutdowntime [ message ]",
		    "", 1);
	}
	if (doreboot && halt) {
		(void) fprintf(stderr,
		    "shutdown: Incompatible switches '-r' & '-h'\n");
		finish("shutdown: Incompatible switches '-r' & '-h'",
		    "", 1);
	}
	if (fast && (nosync == nosyncflag)) {
		(void) fprintf(stderr,
		    "shutdown: Incompatible switches '-f' & '-n'\n");
		finish("shutdown: Incompatible switches '-f' & '-n'",
		    "", 1);
	}
	if (geteuid()) {
		(void) fprintf(stderr, "shutdown: NOT super-user\n");
		finish("shutdown: NOT super-user", "", 1);
	}
	gethostlist();
	nowtime = time((long *)0);
	sdt = getsdt(argv[0]);
	argc--, argv++;
	i = 0;
	while (argc-- > 0) {
		if (i + strlen(*argv) > NLOG)
			break;	/* no more room for the message */
		i += strlen(*argv) + 1;
	}
	m = ((stogo = sdt - nowtime) + 30)/60;
	h = m/60; 
	m %= 60;
	ts = ctime(&sdt);
	(void) printf("Shutdown at %5.5s (in ", ts+11);
	if (h > 0)
		(void) printf("%d hour%s ", h, h != 1 ? "s" : "");
	(void) printf("%d minute%s) ", m, m != 1 ? "s" : "");
#ifndef DEBUG
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
#endif
	(void) signal(SIGTTOU, SIG_IGN);
	(void) signal(SIGTERM, finish_sig);
	(void) signal(SIGALRM, timeout);
	(void) setpriority(PRIO_PROCESS, 0, PRIO_MIN);
	(void) fflush(stdout);
#ifndef DEBUG
	if (i = fork()) {
		(void) printf("[pid %d]\n", i);
		exit(0);
	}
#else
	(void) putc('\n', stdout);
#endif
	sint = 1 HOURS;
	f = "";
	ufd = open("/etc/utmp",0);
	if (ufd < 0) {
		perror("shutdown: /etc/utmp");
		exit(1);
	}
	first = 1;
	if (doreboot) {
		shutdown_program = REBOOT;
		shutdown_action = "reboot";
	} else if (halt) {
		shutdown_program = HALT;
		shutdown_action = "halt";
	} else {
		shutdown_program = NULL;
		shutdown_action = "shutdown";
	}
	for (;;) {
		for (i = 0; stogo <= interval[i].stogo && interval[i].sint; i++)
			sint = interval[i].sint;
		if (stogo > 0 && (stogo-sint) < interval[i].stogo)
			sint = stogo - interval[i].stogo;
		if (sint >= stogo || sint == 0)
			f = "FINAL ";
		nowtime = time((long *)0);
		(void) lseek(ufd, 0L, 0);
		while (read(ufd,(char *)&utmp,sizeof utmp)==sizeof utmp)
		if (utmp.ut_name[0] &&
		    strncmp(utmp.ut_name, IGNOREUSER, sizeof(utmp.ut_name))) {
			/* 
			 * don't write to pty's unless they are rlogin sessions
			 */
			if (setjmp(alarmbuf))
				continue;
			(void) strcpy(term, tpath);
			(void) strncat(term, utmp.ut_line, sizeof utmp.ut_line);
			(void) alarm(3);
#ifdef DEBUG
			if ((termf = stdout) != NULL)
#else
			if ((termf = fopen(term, "w")) != NULL)
#endif
			{
				(void) alarm(0);
				setbuf(termf, tbuf);
				(void) fprintf(termf, "\n\r\n");
				warn(termf, sdt, nowtime, f, first);
				(void) alarm(5);
#ifdef DEBUG
				(void) fflush(termf);
#else
				(void) fclose(termf);
#endif
				(void) alarm(0);
			}
		}
		for (hl = hostlist; hl != NULL; hl = hl->nxt)
			rwarn(hl->host, sdt, nowtime, f, first);
		if (stogo <= 0) {
			(void) printf("\n\007\007System shutdown time has arrived\007\007\n");
			syslog(LOG_CRIT, "%s by %s",
				    shutdown_action, shutter);
			sleep(2);
			if (!killflg) {
				(void) printf("but you'll have to do it yourself\n");
				finish("but you'll have to do it yourself",
				    "", 0);
			}
			if (fast)
				doitfast();
#ifndef DEBUG
			(void) putenv(EPATH);
			if (shutdown_program != NULL)
				execlp(shutdown_program, shutdown_program,
				    "-l", nosync, (char *)0);
			else {
				(void) kill(1, SIGTERM);	/* sync */
				(void) kill(1, SIGTERM);	/* sync */
				sleep(20);
			}
#else
			if (shutdown_program) {
				(void) printf("%s ", shutdown_program);
				if (fast)
					(void) printf("-l (without fsck's)\n");
				else if (nosync != NULL)
					(void) printf("-l %s\n", nosync);
				else
					(void) printf("-l\n");
			} else {
				(void) printf("kill -TERM 1");
				if (fast)
					(void) printf(" (without fsck's)\n");
				else
					(void) printf("\n");
			}
#endif
			finish("", "", 0);
		}
		stogo = sdt - time((long *) 0);
		if (stogo > 0 && sint > 0)
			sleep((unsigned)(sint<stogo ? sint : stogo));
		stogo -= sint;
		first = 0;
	}
	/* NOTREACHED */
}

time_t
getsdt(s)
	register char *s;
{
	time_t t, t1, tim;
	register char c;
	struct tm *lt;

	if (strcmp(s, "now") == 0)
		return(nowtime);
	if (*s == '+') {
		++s; 
		t = 0;
		for (;;) {
			c = *s++;
			if (!isdigit(c))
				break;
			t = t * 10 + c - '0';
		}
		if (t <= 0)
			t = 5;
		t *= 60;
		tim = time((long *) 0) + t;
		return(tim);
	}
	t = 0;
	while (strlen(s) > 2 && isdigit(*s))
		t = t * 10 + *s++ - '0';
	if (*s == ':')
		s++;
	if (t > 23)
		goto badform;
	tim = t*60;
	t = 0;
	while (isdigit(*s))
		t = t * 10 + *s++ - '0';
	if (t > 59)
		goto badform;
	tim += t; 
	tim *= 60;
	t1 = time((long *) 0);
	lt = localtime(&t1);
	t = lt->tm_sec + lt->tm_min*60 + lt->tm_hour*3600;
	if (tim < t || tim >= (24*3600)) {
		/* before now or after midnight */
		(void) printf("That must be tomorrow\nCan't you wait till then?\n");
		finish("That must be tomorrow", "Can't you wait till then?",
		    0);
	}
	return (t1 + tim - t);
badform:
	(void) printf("Bad time format\n");
	finish("Bad time format", "", 0);
	/*NOTREACHED*/
}

warn(termf, sdt, now, type, first)
	FILE *termf;
	time_t sdt, now;
	char *type;
	int first;
{
	char *ts;
	register delay = sdt - now;

	if (delay > 8)
		while (delay % 5)
			delay++;

	(void) fprintf(termf,
	    "\007\007\t*** %sSystem shutdown message from %s@%s ***\r\n\n",
		    type, shutter, hostname);

	ts = ctime(&sdt);
	if (delay > 10 MINUTES)
		(void) fprintf(termf, "System going down at %5.5s\r\n", ts+11);
	else if (delay > 95 SECONDS) {
		(void) fprintf(termf, "System going down in %d minute%s\r\n",
		    (delay+30)/60, (delay+30)/60 != 1 ? "s" : "");
	} else if (delay > 0) {
		(void) fprintf(termf, "System going down in %d second%s\r\n",
		    delay, delay != 1 ? "s" : "");
	} else
		(void) fprintf(termf, "System going down IMMEDIATELY\r\n");
}

doitfast()
{
	FILE *fastd;

	if ((fastd = fopen(fastboot, "w")) != NULL) {
		(void) putc('\n', fastd);
		(void) fclose(fastd);
	}
}

rwarn(host, sdt, now, type, first)
	char *host;
	time_t sdt, now;
	char *type;
	int first;
{
	char *ts;
	register delay = sdt - now;
	char *bufp;

	if (delay > 8)
		while (delay % 5)
			delay++;

	(void) sprintf(buf,
	    "\007\007\t*** %sShutdown message for %s from %s@%s ***\r\n\n",
		type, hostname, shutter, hostname);
	ts = ctime(&sdt);
	bufp = buf + strlen(buf);
	if (delay > 10 MINUTES) {
		(void) sprintf(bufp, "%s going down at %5.5s\r\n", hostname,
		    ts+11);
	}
	else if (delay > 95 SECONDS) {
		(void) sprintf(bufp, "%s going down in %d minute%s\r\n",
		    hostname, (delay+30)/60, (delay+30)/60 != 1 ? "s" : "");
	} else if (delay > 0) {
		(void) sprintf(bufp, "%s going down in %d second%s\r\n",
		    hostname, delay, delay != 1 ? "s" : "");
	} else {
		(void) sprintf(bufp, "%s going down IMMEDIATELY\r\n",
		    hostname);
	}
	bufp = buf + strlen(buf);
	rprintf(host, buf);
}

rprintf(host, bufp)
	char *host, *bufp;
{
	int err;
	
#ifdef DEBUG
	(void) fprintf(stderr, "about to call %s\n", host);
#endif
	if (err = callrpcfast(host, (u_long)WALLPROG, (u_long)WALLVERS,
	    (u_long)WALLPROC_WALL, xdr_path, (char *)&bufp, xdr_void, 
	    (char *)NULL)) {
#ifdef DEBUG
		(void) fprintf(stderr, "couldn't make rpc call: ");
		clnt_perrno(err);
		(void) fprintf(stderr, "\n");
#endif
	    }
}

void
finish_sig()
{
	finish("SIGTERM", "", 1);
}

finish(s1, s2, exitcode)
	char *s1;
	char *s2;
{
	(void) signal(SIGTERM, SIG_IGN);
	exit(exitcode);
}

void
timeout()
{
	(void) signal(SIGALRM, (void(*)())timeout);
	longjmp(alarmbuf, 1);
}

gethostlist()
{
 	int s;
	struct mountlist *ml;
	struct hostlist *hl;
	struct sockaddr_in addr;
	CLIENT *cl;
	static struct timeval TIMEOUT = { 25, 0 };	
    
	/* 
	 * check for portmapper
	 */
	get_myaddress(&addr);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return;
	if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return;
	(void) close(s);

	/*
	 * First try tcp, then drop back to udp if
	 * tcp is unavailable (an old version of mountd perhaps)
	 * Using tcp is preferred because it can handle
	 * arbitrarily long export lists.
	 */
	cl = clnt_create(hostname, (u_long)MOUNTPROG, (u_long)MOUNTVERS,
	    "tcp");
	if (cl == NULL) {
		cl = clnt_create(hostname, (u_long)MOUNTPROG,
		    (u_long)MOUNTVERS, "udp");
		if (cl == NULL) {
			if (rpc_createerr.cf_stat != RPC_PROGNOTREGISTERED) {
				clnt_pcreateerror("shutdown warning");
			}
			return;
		}
	}

	ml = NULL;
	if (clnt_call(cl, MOUNTPROC_DUMP,
	    xdr_void, 0, xdr_mountlist, &ml, TIMEOUT) != RPC_SUCCESS) {
		clnt_perror(cl, "shutdown warning");
		return;
	}
	for (; ml != NULL; ml = ml->ml_nxt) {
		for (hl = hostlist; hl != NULL; hl = hl->nxt)
			if (strcmp(ml->ml_name, hl->host) == 0)
				goto again;
		hl = (struct hostlist *)malloc(sizeof(struct hostlist));
		hl->host = ml->ml_name;
		hl->nxt = hostlist;
		hostlist = hl;
	   again:;
	}
}

/* 
 * Don't want to wait for usual portmapper timeout you get with
 * callrpc or clnt_call, so use rmtcall instead.  Use timeout
 * of 8 secs, based on the per try timeout of 3 secs for rmtcall 
 */
callrpcfast(host, prognum, versnum, procnum, inproc, in, outproc, out)
	char *host;
	u_long prognum, versnum, procnum;
	xdrproc_t inproc, outproc;
	char *in, *out;
{
	struct sockaddr_in server_addr;
	struct hostent *hp;
	struct timeval rpctimeout;
	u_long port;

	if ((hp = gethostbyname(host)) == NULL)
		return ((int) RPC_UNKNOWNHOST);
	bcopy(hp->h_addr, (char *)&server_addr.sin_addr, hp->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port =  0;
	rpctimeout.tv_sec = 8;
	rpctimeout.tv_usec = 0;
	return ((int) pmap_rmtcall(&server_addr, prognum, versnum, procnum,
	  inproc, in, outproc, out, rpctimeout, &port));
}
