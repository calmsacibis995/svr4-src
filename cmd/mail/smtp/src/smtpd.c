/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/smtpd.c	1.5.3.1"
#ifndef lint
static char *sccsid = "@(#)smtpd.c	1.7 87/07/31";
#endif

/*
 * smtpd - SMTP listener: receives SMTP mail & invokes rmail.
 */

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <nlist.h>
#include <pwd.h>
#include <grp.h>
#ifdef TLI
# include <netconfig.h>
# include <netdir.h>
# include <poll.h>
#endif
#include <tiuser.h>
#include <stropts.h>
#if defined(SOCKET) || defined(BIND)
#ifndef SVR3
#  include <sys/uio.h>
#endif
# include <netdb.h>
# include <sys/socket.h>
#ifdef SVR3
#  include <sys/in.h>	/* WIN/3B */
#  include <sys/inet.h>	/* WIN/3B */
#else
#  include <netinet/in.h>
#endif
#endif

#ifdef INETD
#undef TLI
#endif

#define	LOG_CRIT	0
#define	LOG_NOTICE	1

#include "xmail.h"
#include "smtp.h"

/* forward declarations */
FILE		*popen();
SIGRETURN	reapchild();
void		refuse();

SIGRETURN	decrlimit();
SIGRETURN	incrlimit();
SIGRETURN	idlesmtp();
double		loadav();
void		initla();
long		starttime;

extern	char **environ;
extern	int errno, sys_nerr;
extern	char *sys_errlist[];
extern	char *sysname_read();

#ifdef SOCKET
struct sockaddr_in sin = { AF_INET };
struct sockaddr_in from;
struct servent *getservbyname();
#endif

#ifdef SOCKET
#ifndef SERVNAME
#ifndef	DEBUG
#define	SERVNAME "smtp"
#else				/*DEBUG*/
#define SERVNAME "smtpdebug"
#endif				/*DEBUG*/
#endif				/* SERVNAME */
#endif				/* SOCKET */

int	debug;
char	progname[] = "smtpd";	/* Needed for logging */
char	*helohost = NULL;
char	*thishost = NULL;
char	buzzoff[MAXSTR];
int	buzzlen;
int	accepted;
int	norun;

/* Logging stuff for smtpd */

/*
 * This is a macro because it is used frequently in signal routines, and
 * in 4.3 BSD procedure calls seem to be unreliable from the signal
 * processor, even though the mail routine is almost certainly idle
 * waiting for a socket connection.
 */

/*
 * Define NLIST_BUG if you have the nlist() bug
 * which causes large process growth
 */
#ifdef SVR4
#  define NLIST_BUG 1
#  define SLASH_UNIX "/stand/unix"
#else
#  define SLASH_UNIX "/unix"
#endif

struct	nlist nl[] = {
#ifndef	vax
	{ "avenrun" },
#else
	{ "_avenrun" },
#endif
#define	X_AVENRUN	0
	{ "" },
};


int	kmem;
char	*inet_ntoa();

int	idled = 0;		/* idling down */
double	load;			/* current system load */
double	loadlim = 0.0;		/* maximum system load before we reject calls */
int	running = 0;		/* number of smtpd-s running at present */
int	maxrunning = 0;		/* max number of simultaneous smtpd-s
				-1 = infinite */
char	*caller = NULL;		/* who is calling us */

#ifdef SYSLOG
#define	logit(sev, fmt, str) { \
	(void) sprintf(logm, "%s: %s", progname, fmt); \
	syslog(sev, logm, (str == "" && errno <= sys_nerr)? \
		sys_errlist[errno]: str); \
}
#else

logit(sev, fmt, str)
char *fmt, *str;
{
	char msg[BUFSIZ];

	if (*str == '\0' && errno <= sys_nerr)
		(void) sprintf(msg, fmt, sys_errlist[errno]);
	else
		(void) sprintf(msg, fmt, str);
	smtplog(msg);
}

#endif


void
showstatus(status)
char	*status;
{
	char msg[BUFSIZ];

	if (caller == NULL)
		sprintf(msg, "%-9s count=%d/%d, load=%.2f/%.2f%s",
			status, running, maxrunning, load, loadlim,
			idled? " Idled": "");
	else
		sprintf(msg, "%s: %-9s count=%d/%d, load=%.2f/%.2f%s",
			caller, status, running, maxrunning, load, loadlim,
			idled? " Idled": "");
	smtplog(msg);
}

/*
 * Report mail transfer statistics from conversed.  The V9 version
 * of this routine is empty.
 */
void
xferstatus(msg, nbytes)
char *msg;
long nbytes;
{
	long finishtime;
	char msg2[BUFSIZ];
	(void) time(&finishtime);
	sprintf(msg2, "%s: %s connected %ld seconds, %ld bytes",
		caller, msg, (long)(finishtime-starttime), nbytes);
	smtplog(msg2);
}

/*
 * Log an attempted DEBUG command, in response to the sendmail virus.
 */
void
logdanger(format, p0, p1, p2, p3)
	char *format, *p0, *p1, *p3;
{
	char buf[512], msg[512];

	sprintf(buf, format, p0, p1, p2, p3);
	sprintf(msg, "%s: %s", caller, buf);
	smtplog(msg);
}

/*
 * main - parse arguments and handle options
 */
main(argc, argv)
int argc;
char *argv[];
{
	int c;
	int errflg = 0;
	extern int optind;
	extern char *optarg;

	while ((c = getopt(argc, argv, "nH:h:dL:l:")) != EOF)
		switch (c) {
		case 'n':
			norun = 1;
			break;
		case 'H':
			helohost = optarg;
			break;
		case 'h':
			thishost = optarg;
			break;
		case 'd':
			++debug;
			break;
		case 'L':
			loadlim = atoi(optarg);
			break;
		case 'l':
			maxrunning = atoi(optarg);
			break;
		case '?':
		default:
			errflg++;
			break;
		}
	if (errflg) {
		(void) syslog(LOG_CRIT, "Usage: %s [-L sysloadlimit] [-l smtpdlim] [-d] [-h thishost] [-H helohost]\n",
						progname);
		exit(2);
	}
	if (helohost==NULL)
		helohost=sysname_read();

	if (optind >= argc)
		process();
#ifdef INETD
	else if (optind == argc - 1) {		/* one argument */
#ifdef SOCKET
		if (sscanf(argv[optind], "%lx.%hd", &from.sin_addr.s_addr,
		    &from.sin_port) != 2) {
			(void) syslog(LOG_CRIT,
				"in.smtpd: bad arg from inetd: %s\n",
				argv[optind]);
			exit(2);
		}
		from.sin_family = AF_INET;
		from.sin_addr.s_addr = htonl(from.sin_addr.s_addr);
		from.sin_port = htons(from.sin_port);
		(void) sprintf(buzzoff, "421 %s is too busy, please try later.\r\n", helohost);
		buzzlen=strlen(buzzoff);
		process();
#endif				/* SOCKET */
	}
#endif				/* INETD */
	else {
		(void) syslog(LOG_CRIT, "%s: too many args\n", progname);
		exit(2);
	}

	exit(0);
}

/*
 *  process() can be built in one of three ways:
 *	TLI	=> will use SVR4 TLI interface (no sockets stuff)
 *	INETD	=> uses sockets (as if forked of by /usr/etc/inetd)
 *	SOCKET	=> creates and uses sockets.
 */

#ifdef TLI

extern void t_log();

#define	NFD	20			/* Max # of transports */
#define	MAXCONN	10			/* Max # of connections per transport */

struct pollfd	fds[NFD];		/* Transport file descriptors */
struct netconfig ncf[NFD];		/* We need this later */
struct t_call *	calls[NFD][MAXCONN];	/* Call indications */
unsigned long	nfd;			/* How many transports are open */
extern char *	malloc();

char *strsave(str)
char *str;
{
	register char *p;

	if ((p = malloc(strlen(str) + 1)) == NULL) {
		smtplog("out of core!");
		exit(1);	/* nuts */
	}
	(void) strcpy(p, str);
	return p;
}

char **strsave2(strs, n)
char **strs;
unsigned long n;
{
	register char **p;
	register int i;

	if ((p = (char **)malloc(n * sizeof(char *))) == NULL) {
		smtplog("out of core!");
		exit(1);	/* nuts */
	}
	for (i = 0; i < n; i++)
		p[i] = strsave(strs[i]);
	return p;
}

int my_netdir_getbyname(ncp, ndh, nap)
struct netconfig *ncp;
struct nd_hostserv *ndh;
struct nd_addrlist **nap;
{
#ifdef BIND
	/* Special case tcp (so that we can open all interfaces (su89-28602)) */
	if (strcmp(ncp->nc_proto, "tcp") == 0) {
		register struct sockaddr_in *xp;
		register struct netbuf *nbuf;
		register struct servent *sp;
		struct sockaddr_in sin;

		if ((sp = getservbyname("smtp", "tcp")) == NULL)
			return 1;
		*nap = (struct nd_addrlist *) malloc(sizeof(struct nd_addrlist));
		if (*nap == (struct nd_addrlist *) 0)
			return 1;
		(*nap)->n_cnt = 0;
		memset((char *)&sin, '\0', sizeof(sin));
		sin.sin_port = sp->s_port;
		sin.sin_family = AF_INET;

		(*nap)->n_cnt = 1;
		(*nap)->n_addrs = (struct netbuf *) malloc(sizeof(struct netbuf));
		xp = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
		if ((xp == (struct sockaddr_in *) 0) || ((*nap)->n_addrs == (struct netbuf *) 0))
			return 1;
	
		*xp = sin;
		nbuf = &(*nap)->n_addrs[0];
	
		nbuf->maxlen = sizeof(struct sockaddr_in);
		nbuf->len    = sizeof(struct sockaddr_in);
		nbuf->buf    = (char *) xp;
		if (debug)
			fprintf(stderr, "add %s\n", inet_ntoa(sin.sin_addr));

		return 0;
	}
#endif

	if (netdir_getbyname(ncp, ndh, nap) == 0)
		return 0;

	return 1;
}

open_transports()
{
	register int		i;
	struct nd_hostserv	ndh;
	struct netconfig *	ncp;
	struct nd_addrlist *	nap;
	struct netbuf *		nbp;
	struct utsname		u;
	struct t_bind *		bp;
	char			msg[BUFSIZ];
	void *			handle;
	extern struct netconfig *getnetpath();
	extern void *		setnetpath();

	(void) uname(&u);
	ndh.h_host = u.nodename;
	ndh.h_serv = "smtp";

	if ((handle = setnetpath()) == NULL) {
		smtplog("ERROR: can't find /etc/netconfig");
		return 1;
	}

	nfd = 0;
	while ((ncp = getnetpath(handle)) != NULL) {
		if (my_netdir_getbyname(ncp, &ndh, &nap))
			continue;

		/* Open Transport endpoint. */
		if ((fds[nfd].fd = t_open(ncp->nc_device, O_RDWR, NULL)) < 0) {
			t_log("could not open transport");
			continue;
		}

		/* Allocate address struct */
		if ((bp = (struct t_bind *) t_alloc(fds[nfd].fd, T_BIND, T_ALL)) == NULL) {
			t_log("could not allocate t_bind");
			continue;
		}

		/* Bind to an address */
		bp->qlen = MAXCONN;
		nbp = nap->n_addrs;
		for (i = 0; i < nap->n_cnt; i++, nbp++) {
			bp->addr = *nbp;
			if (t_bind(fds[nfd].fd, bp, bp) >= 0)
				break;
		}
		if (i >= nap->n_cnt) {
			t_log("could not bind address");
			continue;
		}

		/* Log that the transport is open */
		sprintf(msg, "transport <%s> opened on fd <%d> address <%s>",
			ncp->nc_netid, fds[nfd].fd, taddr2uaddr(ncp, nbp));
		smtplog(msg);

		/* Save the netconfig entry for later use */
		ncf[nfd] = *ncp;
		ncf[nfd].nc_netid     = strsave(ncp->nc_netid);
		ncf[nfd].nc_protofmly = strsave(ncp->nc_protofmly);
		ncf[nfd].nc_proto     = strsave(ncp->nc_proto);
		ncf[nfd].nc_device    = strsave(ncp->nc_device);
		ncf[nfd].nc_lookups   = strsave2(ncp->nc_lookups, ncp->nc_nlookups);

		fds[nfd].events = POLLIN;
		nfd++;
	}
	endnetpath(handle);

	if (nfd == 0) {
		smtplog("could not open any transports");
		return 1;
	}
	return 0;
}

void set_caller(ncp, addr)
struct netconfig *ncp;
struct netbuf *addr;
{
	static char buf[48];

	sprintf(buf, "<%s,%s>", ncp->nc_netid, taddr2uaddr(ncp, addr));
	caller = buf;
}

int accept_call(i)
{
	extern int t_errno;
	int server_fd, fd;
	int j, pid;


	fd = fds[i].fd;

	for (j = 0; j < MAXCONN; j++) {
		if (calls[i][j] == NULL)
			continue;

		if ((server_fd = t_open(ncf[i].nc_device, O_RDWR, NULL)) < 0) {
			t_log("can't t_open transport");
			return -1;
		}

		if (t_bind(server_fd, NULL, NULL) < 0) {
			t_log("can't bind address for server");
			return -1;
		}

		if (t_accept(fd, server_fd, calls[i][j]) < 0) {
			if (t_errno == TLOOK) {
				t_close(server_fd);
				return 0;
			}
			t_log("t_accept failed");
			return -1;
		}

		set_caller(&ncf[i], &(calls[i][j]->addr));
		t_free(calls[i][j], T_CALL);
		calls[i][j] = NULL;

		/* Set up file descriptor for read/write use */
		if (ioctl(server_fd, I_PUSH, "tirdwr") < 0) {
			t_log("push tirdwr failed");
			(void) t_close(server_fd);
			return -1;
		}

		/* Start server conversation */
		load = loadav();
		(void) time(&starttime);
		if (!idled && 
		   ((maxrunning == 0)   || (running < maxrunning)) &&
		   ((loadlim <= 0.001)  || (load <= loadlim))) {
			/* fork a child for this connection */
			running++;
			showstatus("accepted");
			if ((pid = fork()) < 0) {
				logit(LOG_CRIT, "can't fork!!", "");
				running--;
			} else if (pid == 0) {
				(void) signal(SIGCLD,  (void (*)())SIG_DFL);
				(void) signal(SIGUSR1, (void (*)())SIG_DFL);
				(void) signal(SIGUSR2, (void (*)())SIG_DFL);
				(void) signal(SIGHUP,  (void (*)())SIG_DFL);
				doit(server_fd, 1);  /* listen to SMTP dialogue */
				/* NOTREACHED */
				exit(0);
			}
		} else {
			logit(LOG_NOTICE, "Connection refused: ", "");
			showstatus("refused");
		}
		(void) t_close(server_fd);
	}
}

void handle_event(i)
register int i;
{
	register int j, fd, tlook;

	fd = fds[i].fd;
	tlook = t_look(fd);
	if (tlook == T_LISTEN) {
		for (j = 0; j < MAXCONN; j++)
			if (calls[i][j] == NULL)
				break;
		if (j >= MAXCONN) {
			smtplog("can't handle any more calls");
			return;
		}
		if ((calls[i][j] = (struct t_call *) t_alloc(fd, T_CALL, T_ALL)) == NULL) {
			t_log("can't alloc t_call");
			return;
		}
		if (t_listen(fd, calls[i][j]) < 0) {
			t_log("t_listen failed");
			return;
		}
	} else if (tlook == T_DISCONNECT) {
		register struct t_discon *dcp;

		dcp = (struct t_discon *) t_alloc(fd, T_DIS, T_ALL);
		if (t_rcvdis(fd, dcp) < 0) {
			t_log("t_rcvdis failed");
			return;
		}
		for (j = 0; j < nfd; j++) {
			if (dcp->sequence == calls[i][j]->sequence) {
				t_free(calls[i][j], T_CALL);
				calls[i][j] = NULL;
			}
		}
		t_free(dcp, T_DIS);
	} else {
		t_log("t_look failed");
	}
}

/*
 * process - handle incoming connections over all transports for
 *	which the "smtp" service is defined.
 */
process()
{
	register int i;
	register struct passwd *pw = NULL;
	register struct group *gr = NULL;
#ifdef SVR3
	struct passwd *getpwnam();
	struct group *getgrnam();
#endif
	char msg[BUFSIZ/2];

	/* force creation of queuedir; make sure it has the right permissions */
	if (chdir(SMTPQROOT) < 0) {
		(void) mkdir(SMTPQROOT, 0775);
		(void) chdir(SMTPQROOT);
	}
	if (((pw = getpwnam("uucp")) != NULL) && ((gr = getgrnam("mail")) != NULL)) {
		(void) chmod(".", 0775);
		(void) chown(".", pw->pw_uid, gr->gr_gid);
	}

	if (fork())			/* run in the background */
		exit(0);
	for (i = 0; i < _NFILE; i++)	/* close most file descriptors */
		(void) close(i);
	(void) setpgrp();		/* leave current process group */
	(void) open("/dev/null", 0);	/* reopen them on harmless streams */
	(void) dup2(0, 1);
	(void) dup2(0, 2);

	initla();
	load = loadav();
	showstatus("startup");

	if (open_transports())
		exit(0);

	(void) signal(SIGCLD,  (void (*)())reapchild);	/* gross hack! */
	(void) signal(SIGUSR1, (void (*)())decrlimit);
	(void) signal(SIGUSR2, (void (*)())incrlimit);
	(void) signal(SIGHUP,  (void (*)())idlesmtp);


	for (;;) {
		int rc = poll(fds, nfd, -1);

		if ((rc < 0) && (errno == EINTR))
			continue;	/* signals are to be expected */
		if (rc < 0) {
			sprintf(msg, "poll failed, errno = %d", errno);
			smtplog(msg);
			exit(1);
		}

		for (i = 0; i < nfd; i++) {
			switch (fds[i].revents) {
			case POLLIN:
				handle_event(i);
				accept_call(i);
				/* fall thru */
			case 0:
				break;
			default:
				sprintf(msg, "unexpected event %d on fd %d", fds[i].revents, fds[i].fd);
				smtplog(msg);
				exit(1);
			}
		}
	}
	/*NOTREACHED*/
}

#else

#ifdef INETD

/*
 * process - process input file
 */
process()
{
	struct servent *sp;

	sp = getservbyname(SERVNAME, "tcp");
	if (sp == 0) {
		logit(LOG_CRIT, "tcp/%s: unknown service\n", SERVNAME);
		exit(1);
	}
	sin.sin_port = sp->s_port;

	/* connection on fd 0 from inetd */
	doit(0, 1);
	/* NOTREACHED */
	exit(0);
}

#else

/*
 * process - process input file
 */
process()
{
#ifdef SOCKET
	int s, pid;
	struct servent *sp;

	sp = getservbyname(SERVNAME, "tcp");
	if (sp == 0) {
		logit(LOG_CRIT, "tcp/%s: unknown service\n", SERVNAME);
		exit(1);
	}
	sin.sin_port = sp->s_port;
#endif		/* SOCKET */

#ifndef DEBUG
	if (fork())			/* run in the background */
		exit(0);
	for (s = 0; s < 10; s++)	/* close most file descriptors */
		(void) close(s);
	(void) setpgrp();		/* leave current process group */
	(void) open("/dev/null", 0);	/* reopen them on harmless streams */
	(void) dup2(0, 1);
	(void) dup2(0, 2);
#endif

	initla();
	load=loadav();
	showstatus("startup");

#ifdef SOCKET
	/* create internet socket s; retry 5 times at 5 s. intervals if no luck */
	while ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		static int nlog = 0;

		if (nlog++ <= 5)
			logit(LOG_CRIT, "socket", "");
		sleep(5);
	}
	/* set socket options, notably keepalive */
	if (debug) {
		int debugval = 1;

		if (setsockopt(s, SOL_SOCKET, SO_DEBUG,
		    (char *)&debugval, sizeof(int)) < 0)
			logit(LOG_CRIT, "setsockopt (SO_DEBUG)", "");
	}
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)0, 0) < 0)
		logit(LOG_CRIT, "setsockopt (SO_KEEPALIVE)", "");
	/* bind socket to SERVNAME (SMTP) port; retry as above on failure */
	while (bind(s, (struct sockaddr *)&sin, sizeof sin) < 0) {
		static int nlog = 0;

		if (nlog++ <= 100)
			logit(LOG_CRIT, "bind", "");
		sleep(5);
	}
	(void) signal(SIGCLD,  (SIG_TYP)reapchild);	/* gross hack! */
	(void) signal(SIGUSR1, (SIG_TYP)decrlimit);
	(void) signal(SIGUSR2, (SIG_TYP)incrlimit);
	(void) signal(SIGHUP,  (SIG_TYP)idlesmtp);
	/* listen with 5 input buffers on socket (?) */
	if (listen(s, 5) == -1)
		logit(LOG_CRIT, "listen", "");
	for (;;) {
		int conn, fromlen = sizeof from;

		/* get a connection on fd conn; stores src host addr in from */
		conn = accept(s, (struct sockaddr_in *)&from, &fromlen);
		if (conn < 0) {
			static int nlog = 0;

			if (errno == EINTR)
				continue;
			if (++nlog <= 5)
				logit(LOG_CRIT, "accept", "");
			sleep(1);
			continue;
		}
		load = loadav();
		accepted=accept_call(caller = inet_ntoa(from.sin_addr));
		switch (accepted) {
			case 0:	showstatus("rejected");
				(void) close(conn);
				sleep(15);
				continue;
			case 1: showstatus("accepted");  break;
			case 2: showstatus("special");   break;
		}
		running++;
		(void) time(&starttime);
		/* fork a child for this connection */
		if ((pid = fork()) < 0) {
			logit(LOG_CRIT, "can't fork!!", "");
			running--;
		} else if (pid == 0) {
			(void) signal(SIGCHLD, (SIG_TYP)SIG_DFL);
			(void) signal(SIGUSR1, (SIG_TYP)SIG_DFL);
			(void) signal(SIGUSR2, (SIG_TYP)SIG_DFL);
			(void) signal(SIGHUP,  (SIG_TYP)SIG_DFL);
			doit(conn, accepted); /* listen to SMTP dialogue */
			/* NOTREACHED */
			exit(0);
		}
		(void) close(conn);
		sleep(12);
	}
	/*NOTREACHED*/
#endif			/* SOCKET */
}

accept_call(caller)
char *caller;
{
#define	NO	0
#define YES	1
	register int i;

	if (idled)
		return NO;
	if ((loadlim > 0.001) && (load > loadlim))
		return NO;
	if ((maxrunning >= 0) && (running >= maxrunning))
		return NO;
	return YES;
}

#endif	/* INETD */
#endif	/* SVR4 */

#ifndef INETD
SIGRETURN
reapchild(s)
	int s;
{
	int status;
	/* gross hack! */
	(void) wait(&status);
	running--;
	if(idled && running <= 0) {
		showstatus("exiting");
		exit(0);
	}
	(void) signal(SIGCLD,  (void (*)())reapchild);	/* gross hack! */
}
#endif			/* INETD */

int	cleanup();

/*
 * handle some input.  never returns.
 */
doit(f, accepted)
int f, accepted;
{
	FILE *fi, *fo;
	struct passwd *p;
#ifdef SVR3
	struct passwd *getpwnam();
#endif

	/*
	 *  become uucp
	 */
	p = getpwnam("uucp");
	if(p)
		setuid(p->pw_uid);
	umask(002);

	if ((fi = fdopen(f, "r")) == NULL)
		logit(LOG_CRIT, "fdopen of socket for input", "");
	if ((fo = fdopen(f, "w")) == NULL)
		logit(LOG_CRIT, "fdopen of socket for output", "");

	converse(fi, fo, accepted);
	/* NOTREACHED */
	return 0;
}

/*
 * loadav - return the 1 minute load average.
 *	(Found by looking in kernel for avenrun).
 */
double
loadav()
{
	long avenrun[3];

	if (kmem == -1)
		return (double) 0;
	lseek(kmem, (long)nl[X_AVENRUN].n_value, 0);
	read(kmem, avenrun, sizeof(avenrun));
/*	return(avenrun[1]);	five minute average */
	return ((double) avenrun[0]) / 256.0;	/* one minute average */
}


/* Initialize the load-average stuff */
void
initla()
{
#ifdef NLIST_BUG
	/* Get around the nasty bug in nlist that */
	/* causes lots of memory to be swallowed up */
	int pipes[2];

	if (pipe(pipes) < 0) {
		kmem = -1;
		return;
	}
	switch (fork()) {
	case 0:
		/* Child does nlist and writes to pipe */
		nlist(SLASH_UNIX, nl);
		if (nl[0].n_value) {
			(void) write(pipes[1], nl, sizeof nl);
			close(pipes[1]);
		}
		exit(0);
	case -1:
		kmem = -1;
		return;
	default:
		/* Parent reads nlist values from child */
		(void) close(pipes[1]);
		if (read(pipes[0], nl, sizeof nl) != sizeof nl) {
			(void) close(pipes[0]);
			(void) wait((int *)0);
			kmem = -1;
			return;
		}
		(void) close(pipes[0]);
		(void) wait((int *)0);
	}
#else
	nlist(SLASH_UNIX, nl);
	if (nl[0].n_value == 0) {
		kmem = -1;
		return;
	}
#endif

	if ((kmem = open("/dev/kmem", 0)) < 0) {
		kmem = -1;
		return;
	}
}

SIGRETURN
incrlimit(s)
	int s;
{
	maxrunning++;
	showstatus("incr");
}

SIGRETURN
decrlimit(s)
	int s;
{
	if(maxrunning > 0)
		maxrunning--;
	showstatus("decr");
}

SIGRETURN
idlesmtp(s)
	int s;
{
	idled = !idled;
	if (idled) {
		showstatus("idled");
	} else {
		showstatus("unidled");
	}
	if(idled && running <= 0) {
		showstatus("exiting");
		exit(0);
	}
}

#ifdef INETD
#include <fcntl.h>
#ifdef SVR4
static char bin_shell[] = "/usr/bin/sh";
#else
static char bin_shell[] = "/bin/sh";
#endif
static char shell[] = "sh";
static char shflg[]= "-c";
static char devnull[] = "/dev/null";

extern int fork(), execl(), wait();

/* Replacement for system() for use in in.smtpd (called from conversed.c) */
/* This is needed because the SYSV shell does not like stdin/out/err */
/* attached to a socket (as it will be in in.smtpd) */
int system(s)
char    *s;
{
        int     status, pid, w;
        void (*istat)(), (*qstat)();

        if((pid = fork()) == 0) {
		close(0);
		close(1);
		close(2);
		open(devnull, O_RDWR);
		open(devnull, O_RDWR);
		open(devnull, O_RDWR);
                (void) execl(bin_shell, shell, shflg, s, (char *)0);
                _exit(127);
        }
        istat = signal(SIGINT, SIG_IGN);
        qstat = signal(SIGQUIT, SIG_IGN);
        while((w = wait(&status)) != pid && w != -1)
                ;
        (void) signal(SIGINT, istat);
        (void) signal(SIGQUIT, qstat);
        return((w == -1)? w: status);
}
#endif

#ifdef SVR3
/* quick-and-dirty syslog() replacement for S5 */
#include <string.h>
#include <varargs.h>

#define	SYSLOG		"/usr/spool/smtpq/syslog"

extern char *ctime();

syslog(va_alist)
va_dcl
{
	va_list ap;
	register FILE *fp;
	register char *p;
	long now;
	int unused;

	va_start(ap);
	unused = va_arg(ap, int);
	if ((fp = fopen(SYSLOG, "a")) != NULL) {
		now = time((long*)0);
		p = ctime(&now);
		p[strlen(p)-1] = '\0';
		fprintf(fp, "%s: %d ", p, getpid());
		p = va_arg(ap, char *);
		vfprintf(fp, p, ap);
		fclose(fp);
	}
	va_end(ap);
}
#endif
