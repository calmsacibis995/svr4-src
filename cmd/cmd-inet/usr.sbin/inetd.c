#ident	"@(#)inetd.c	1.2	92/07/05	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/inetd.c	1.21.4.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


/*
 * Inetd - Internet super-server
 *
 * This program invokes all internet services as needed.
 * connection-oriented services are invoked each time a
 * connection is made, by creating a process.  This process
 * is passed the connection as file descriptor 0 and is
 * expected to do a getpeername to find out the source host
 * and port.
 *
 * Datagram oriented services are invoked when a datagram
 * arrives; a process is created and passed a pending message
 * on file descriptor 0.  Datagram servers may either connect
 * to their peer, freeing up the original socket for inetd
 * to receive further messages on, or ``take over the socket'',
 * processing all arriving datagrams and, eventually, timing
 * out.	 The first type of server is said to be ``multi-threaded'';
 * the second type of server ``single-threaded''. 
 *
 * Inetd uses a configuration file which is read at startup
 * and, possibly, at some later time in response to a hangup signal.
 * The configuration file is ``free format'' with fields given in the
 * order shown below.  Continuation lines for an entry must being with
 * a space or tab.  All fields must be present in each entry.
 *
 *	service name			must be in /etc/services
 *	socket type			stream/dgram/raw/rdm/seqpacket
 *	protocol			must be in /etc/protocols
 *	wait/nowait			single-threaded/multi-threaded
 *	user				user to run daemon as
 *	server program			full path name
 *	server program arguments	maximum of MAXARGS (5)
 *
 * for rpc services:
 *	service name/version		must be in /etc/rpc with version = 1-x
 *	socket type			stream/dgram/raw/rdm/seqpacket
 *	protocol			of the form rpc/protocol
 *	wait/nowait			single-threaded/multi-threaded
 *	user				user to run daemon as
 *	server program			full path name
 *	server program arguments	maximum of MAXARGS (5)
 *
 * Comment lines are indicated by a `#' in column 1.
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <netdb.h>
#include <rpc/rpcent.h>
#include <syslog.h>
#include <pwd.h>
#include <fcntl.h>
#include <tiuser.h>
#include <netdir.h>

#ifdef SYSV
#include <sac.h>

#define rindex(s, c)	strrchr(s, c)
#define index(s, c)	strchr(s, c)
#define bcopy(a,b,c)	memcpy(b,a,c)
#define bzero(s,n)	memset((s), 0, (n))

#ifndef sigmask
#define sigmask(m)      (1 << ((m)-1))
#endif

#define set2mask(setp) ((setp)->sigbits[0])
#define mask2set(mask, setp) \
	((mask) == -1 ? sigfillset(setp) : (((setp)->sigbits[0]) = (mask)))
	

static sigsetmask(mask)
	int mask;
{
	sigset_t oset;
	sigset_t nset;

	(void) sigprocmask(0, (sigset_t *)0, &nset);
	mask2set(mask, &nset);
	(void) sigprocmask(SIG_SETMASK, &nset, &oset);
	return set2mask(&oset);
}

static sigblock(mask)
	int mask;
{
	sigset_t oset;
	sigset_t nset;

	(void) sigprocmask(0, (sigset_t *)0, &nset);
	mask2set(mask, &nset);
	(void) sigprocmask(SIG_BLOCK, &nset, &oset);
	return set2mask(&oset);
}

#endif

#define	TOOMANY		40		/* don't start more than TOOMANY */
#define	CNT_INTVL	60		/* servers in CNT_INTVL sec. */
#define	RETRYTIME	(60*10)		/* retry after bind or server fail */

#define	SIGBLOCK	(sigmask(SIGCHLD)|sigmask(SIGHUP)|sigmask(SIGALRM))

#ifndef MIN
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

extern	int errno;

void	reapchild(), retry(), config();
void	termserv();
void	unregister();
char	*malloc();
long	strtol();

#ifndef SYSV
char	*index();
#else
void	sacmesg();
char	*getenv();
char	*strchr();
void	sigterm();

int	standalone = 0;		/* run without SAC */
#endif SYSV

int	debug = 0;
FILE	*debugfp;
int	ndescriptors;
int	nsock, maxsock;
fd_set	allsock;
int	options;
int	timingout;
int	nservers = 0;
int	maxservers;
struct	servent *sp;
int	calltrace = 0;		/* trace all connections */

/*
 * Each service requires one file descriptor for the socket we listen on
 * for requests for that service.  We don't allow more services than
 * we can allocate file descriptors for.  OTHERDESCRIPTORS is the number
 * of descriptors needed for other purposes; this number was determined
 * experimentally.
 */
#ifdef SYSV
#define	OTHERDESCRIPTORS	11
#else
#define	OTHERDESCRIPTORS	8
#endif SYSV

/*
 * Additional information carried in various fields:
 *
 * If "se_wait" is neither 0 nor 1, it's the PID of the server process
 * currently running for that service.
 * If "se_fd" is -1, it means that the service could not be set up,
 * e.g. the entry for that service couldn't be found in "/etc/services".
 */
struct	servtab {
	char	*se_service;		/* name of service */
	int	se_socktype;		/* type of socket to use */
	char	*se_proto;		/* protocol used */
	char	se_isrpc;		/* service is RPC-based */
	char	se_checked;		/* looked at during merge */
	pid_t	se_wait;		/* single threaded server */
	char	*se_user;		/* user name to run as */
	struct	biltin *se_bi;		/* if built-in, description */
	char	*se_server;		/* server program */
#define MAXARGV 5
	char	*se_argv[MAXARGV+1];	/* program arguments */
	int	se_fd;			/* open descriptor */
	union {
		struct	sockaddr_in ctrladdr;	/* bound address */
		struct {
			unsigned prog;	/* program number */
			unsigned lowvers;	/* lowest version supported */
			unsigned highvers;	/* highest version supported */
		} rpcnum;
	} se_un;
	int	se_count;		/* number started since se_time */
	struct	timeval se_time;	/* start of se_count */
	struct	servtab *se_next;
	char	*se_dev;		/* device name for TLI services */
	struct	t_info	se_info;	/* transport provider info for 
					   TLI services */
} *servtab;

/*
 * We define SOCK_TLI as a sort of pseudo socket type do differentiate
 * TLI services.  Lets hope no one ever defines a socket type with this
 * number!
 */
#define SOCK_TLI 1000

#define se_ctrladdr se_un.ctrladdr
#define se_rpc se_un.rpcnum

struct netconfig *_rpc_getconfip();

int echo_stream(), discard_stream(), machtime_stream();
int daytime_stream(), chargen_stream();
int echo_dg(), discard_dg(), machtime_dg(), daytime_dg(), chargen_dg();

struct biltin {
	char	*bi_service;		/* internally provided service name */
	int	bi_socktype;		/* type of socket supported */
	short	bi_fork;		/* 1 if should fork before call */
	short	bi_wait;		/* 1 if should wait for child */
	int	(*bi_fn)();		/* function which performs it */
} biltins[] = {
	/* Echo received data */
	"echo",		SOCK_STREAM,	1, 0,	echo_stream,
	"echo",		SOCK_DGRAM,	0, 0,	echo_dg,

	/* Internet /dev/null */
	"discard",	SOCK_STREAM,	1, 0,	discard_stream,
	"discard",	SOCK_DGRAM,	0, 0,	discard_dg,

	/* Return 32 bit time since 1970 */
	"time",		SOCK_STREAM,	0, 0,	machtime_stream,
	"time",		SOCK_DGRAM,	0, 0,	machtime_dg,

	/* Return human-readable time */
	"daytime",	SOCK_STREAM,	0, 0,	daytime_stream,
	"daytime",	SOCK_DGRAM,	0, 0,	daytime_dg,

	/* Familiar character generator */
	"chargen",	SOCK_STREAM,	1, 0,	chargen_stream,
	"chargen",	SOCK_DGRAM,	0, 0,	chargen_dg,
	0
};

#define NUMINT	(sizeof(intab) / sizeof(struct inent))
char	*CONFIG = "/etc/inetd.conf";
char	**Argv;
char 	*LastArg;
#ifdef SYSV
struct	pmmsg	Pmmsg;
char	Tag[PMTAGSIZE];
char	State = PM_STARTING;
int	Sfd, Pfd;
#endif SYSV

main(argc, argv, envp)
	int argc;
	char *argv[], *envp[];
{
	register struct servtab *sep;
	register struct passwd *pwd;
	char *cp, buf[50];
	int i, dofork;
	pid_t pid;
#ifndef SYSV
	struct sigvec sv;
#else
	char *istate;
	int pidfd;
	int ret;
	char pidstring[BUFSIZ];
#endif /* SYSV */

	Argv = argv;
	if (envp == 0 || *envp == 0)
		envp = argv;
	while (*envp)
		envp++;
	LastArg = envp[-1] + strlen(envp[-1]);
	argc--, argv++;
	while (argc > 0 && *argv[0] == '-') {
		for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

		case 'd':
			debug = 1;
			options |= SO_DEBUG;
			break;

		case 's':
			standalone = 1;
			break;

		case 't':
			calltrace = 1;
			break;

		default:
			if ( standalone )
				fprintf(stderr,
			    	"inetd: Unknown flag -%c ignored.\n", *cp);
			else
				syslog(LOG_WARNING,
			    	"inetd: Unknown flag -%c ignored.\n", *cp);
			break;
		}
		argc--, argv++;
	}
	if ( debug )
		if ( standalone )
			debugfp = stderr;
		else
			debugfp = fopen("/var/saf/inetd/log","a");

	if (argc > 0)
		CONFIG = argv[0];
	ndescriptors = getdtablesize();
	maxservers = ndescriptors - OTHERDESCRIPTORS;

#ifdef SYSV
	if (!standalone) {
		istate = getenv("ISTATE");
		if (!strcmp(istate, "enabled"))
			State = PM_ENABLED;
		else if (!strcmp(istate, "disabled"))
			State = PM_DISABLED;
		else {
			syslog(LOG_ERR, "Invalid initial state");
			exit(-1);
		}
		strcpy(Tag, getenv("PMTAG"));
		/* fill in things that don't change */
		Pmmsg.pm_size = 0;
		Pmmsg.pm_maxclass = 1;
		strcpy(Pmmsg.pm_tag, Tag);
		if (debug)
			fprintf (debugfp, "Istate = %s, Tag = <%s>\n", 
				 istate, Tag);
		if (!debug) {
			(void) open("/", O_RDONLY);
			(void) dup2(0, 1);
			(void) dup2(0, 2);
			setsid();
		}
		if ((Sfd = open("../_sacpipe", O_RDWR)) < 0) {
			syslog(LOG_ERR, "Could not open _sacpipe file");
			exit(-1);
		}
		if ((Pfd = open("_pmpipe", O_RDWR)) < 0) {
			syslog(LOG_ERR, "Could not open _pmpipe file");
			exit(-1);
		}
		FD_SET(Pfd, &allsock);
		nsock++;
		if ((pidfd = open("_pid", O_WRONLY | O_CREAT, 0644)) < 0) {
			syslog(LOG_ERR, "Could not create _pid file");
			exit(-1);
		}
		if (lockf(pidfd, 2, 0L) < 0) {
			syslog(LOG_ERR, "Could not lock _pid file");
			exit(-1);
		}
		pid = getpid();
		i = sprintf(pidstring, "%ld", pid) + 1;	/* +1 for null */
		ftruncate(pidfd, 0);
		while ((ret = write(pidfd, pidstring, i)) != i) {
			if (errno == EINTR)
				continue;
			if (ret < 0) {
				syslog(LOG_ERR, "Could not write _pid file");
				exit(-1);
			}
		}
	} /* if !standalone */
	else if (!debug) {
		if (fork())
			exit(0);
		(void) close(0);
		(void) close(1);
		(void) close(2);
		(void) open("/", O_RDONLY);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		setsid();
	}
	openlog("inetd", LOG_PID | LOG_NOWAIT, LOG_DAEMON);
	(void) sigset(SIGALRM, retry);
	if (!standalone)
		(void) sigset(SIGTERM, sigterm);
	config();
	(void) sigset(SIGHUP, config);
	(void) sigset(SIGCHLD, reapchild);

#else

	if (!debug) {
		if (fork())
			exit(0);
		(void) close(0);
		(void) close(1);
		(void) close(2);
		(void) open("/", O_RDONLY);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		{
			int tt = open("/dev/tty", O_RDWR);
			if (tt > 0) {
				ioctl(tt, TIOCNOTTY, (char *)0);
				(void) close(tt);
			  }
		}
	}
	openlog("inetd", LOG_PID | LOG_NOWAIT, LOG_DAEMON);
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = SIGBLOCK;
	sv.sv_handler = retry;
	sigvec(SIGALRM, &sv, (struct sigvec *)0);
	config();
	sv.sv_handler = config;
	sigvec(SIGHUP, &sv, (struct sigvec *)0);
	sv.sv_handler = reapchild;
	sigvec(SIGCHLD, &sv, (struct sigvec *)0);

#endif SYSV

	for (;;) {
	    int ctrl, n;
	    fd_set readable;
	    struct sockaddr_in rem_addr;
	    int remaddrlen = sizeof (rem_addr);

	    while (nsock == 0)
		    sigpause(0);
	    readable = allsock;
	    if ((n = select(maxsock + 1, &readable, (fd_set *)0,
		(fd_set *)0, (struct timeval *)0)) <= 0) {
		    if (n < 0 && errno != EINTR)
				syslog(LOG_WARNING, "select: %m\n");
		    if (n < 0 && errno == EBADF) {
			    syslog(LOG_WARNING, 
				   "readable:  %x %x %x %x %x %x %x %x \n",
				   readable.fds_bits[0], 
				   readable.fds_bits[1], 
				   readable.fds_bits[2], 
				   readable.fds_bits[3], 
				   readable.fds_bits[4], 
				   readable.fds_bits[5],
				   readable.fds_bits[6],
				   readable.fds_bits[7]);
			    syslog(LOG_WARNING, 
				   "allsock:  %x %x %x %x %x %x %x %x \n",
				   allsock.fds_bits[0], 
				   allsock.fds_bits[1], 
				   allsock.fds_bits[2], 
				   allsock.fds_bits[3], 
				   allsock.fds_bits[4], 
				   allsock.fds_bits[5],
				   allsock.fds_bits[6],
				   allsock.fds_bits[7]);
		    }
		    sleep(1);
		    continue;
	    }
#ifdef SYSV
	    if (!standalone && n && FD_ISSET(Pfd, &readable)) {
		sacmesg();
		n--;
	    }
#endif
	    for (sep = servtab; n && sep; sep = sep->se_next)
	    if (sep->se_fd != -1 && FD_ISSET(sep->se_fd, &readable)) {
		int ctrlisnew = 0;

		n--;
		if (debug)
			fprintf(debugfp, "someone wants %s\n", sep->se_service);
		if (!sep->se_wait && sep->se_socktype == SOCK_STREAM) {
			ctrl = accept(sep->se_fd, &rem_addr, &remaddrlen);
			if (debug)
				fprintf(debugfp, "accept, ctrl %d\n", ctrl);
			if (ctrl < 0) {
				if (errno == EINTR)
					continue;
				syslog(LOG_WARNING, "accept: %m");
				continue;
			}
			ctrlisnew = 1;
		} else if (!sep->se_wait && (sep->se_socktype == SOCK_TLI) &&
			   (sep->se_info.servtype != T_CLTS)) {
			ctrl = tli_accept(sep->se_fd, &rem_addr, &remaddrlen,
					  sep->se_dev);
			if (debug)
				fprintf(debugfp, "tli_accept, ctrl %d\n", ctrl);
			if (ctrl < 0)
				continue;
			ctrlisnew = 1;
		} else
			ctrl = sep->se_fd;
		(void) sigblock(SIGBLOCK);
		pid = 0;
		dofork = (sep->se_bi == 0 || sep->se_bi->bi_fork);
		if (dofork) {
			if (sep->se_count++ == 0)
			    (void)gettimeofday(&sep->se_time,
			        (struct timezone *)0);
			else if (sep->se_count >= TOOMANY) {
				struct timeval now;

				(void)gettimeofday(&now, (struct timezone *)0);
				if (now.tv_sec - sep->se_time.tv_sec >
				    CNT_INTVL) {
					sep->se_time = now;
					sep->se_count = 1;
				} else {
					syslog(LOG_ERR,
			"%s/%s server failing (looping), service terminated",
					    sep->se_service, sep->se_proto);
					termserv(sep);
					sep->se_count = 0;
					sigsetmask(0);
					if (!timingout) {
						timingout = 1;
						alarm(RETRYTIME);
					}
					continue;
				}
			}
			pid = fork();
		}
		if (pid < 0) {
			if (ctrlisnew)
				(void) close(ctrl);
			sigsetmask(0);
			sleep(1);
			continue;
		}
		if (pid && sep->se_wait) {
			sep->se_wait = pid;
			FD_CLR(sep->se_fd, &allsock);
			nsock--;
		}

		if (calltrace && (!dofork || pid)) {
			if ((sep->se_socktype == SOCK_STREAM) ||
			    ((sep->se_socktype == SOCK_TLI) &&
			     (sep->se_info.servtype != T_CLTS)))
				/* don't waste time doing a gethostbyaddr */
				(pid) ?	syslog(LOG_INFO, 
				       "%s[%d] from %s %d",
				       sep->se_service, pid,
				       inet_ntoa (rem_addr.sin_addr),
				       rem_addr.sin_port) :
				syslog(LOG_INFO, 
				       "%s from %s %d",
				       sep->se_service,
				       inet_ntoa (rem_addr.sin_addr),
				       rem_addr.sin_port);
		}

		sigsetmask(0);
		if (pid == 0) {
			char addrbuf[32];
			int tt;

			if (debug)
#ifdef SYSV
				if (dofork)
					setsid();
#else
				if (dofork && 
				    (tt = open("/dev/tty", O_RDWR)) > 0) {
					ioctl(tt, TIOCNOTTY, 0);
					(void) close(tt);
				}
#endif SYSV

			sprintf(addrbuf, "%x.%d", ntohl(rem_addr.sin_addr.s_addr),
					ntohs(rem_addr.sin_port));
                       if (dofork) 
                                /* 
                                 * only close those descriptors that might
                                 * actually be open.
                                 */
                                for (i = maxsock + 5; --i > 2; )
                                        if (i != ctrl)
                                                (void) close(i);

			if (sep->se_bi)
				(*sep->se_bi->bi_fn)(ctrl, sep);
			else {
				(void) dup2(ctrl, 0);
				(void) close(ctrl);
				(void) dup2(0, 1);
				(void) dup2(0, 2);
				if ((pwd = getpwnam(sep->se_user)) == NULL) {
					syslog(LOG_ERR,
						"getpwnam: %s: No such user",
						sep->se_user);
					if (sep->se_socktype != SOCK_STREAM)
						recv(0, buf, sizeof (buf), 0);
					_exit(1);
				}
				if (pwd->pw_uid) {
					if (setgid((gid_t)pwd->pw_gid) < 0) {
					  syslog(LOG_ERR,
						"setgid(%d): %m", pwd->pw_gid);
					  if (sep->se_socktype != SOCK_STREAM)
						recv(0, buf, sizeof (buf), 0);
					  _exit(1);
					}
					initgroups(pwd->pw_name, pwd->pw_gid);
					if (setuid((uid_t)pwd->pw_uid) < 0) {
					  syslog(LOG_ERR,
						"setuid(%d): %m", pwd->pw_uid);
					  if (sep->se_socktype != SOCK_STREAM)
						recv(0, buf, sizeof (buf), 0);
					  _exit(1);
					}
				}
				if (sep->se_argv[0] != NULL) {
				    if (!strcmp(sep->se_argv[0], "%A"))
					execl(sep->se_server,
						rindex(sep->se_server, '/')+1,
						sep->se_socktype == SOCK_DGRAM
						? (char *)0 : addrbuf, (char *)0);
				    else
					execv(sep->se_server, sep->se_argv);
			        } else
					execv(sep->se_server, sep->se_argv);
				if ((sep->se_socktype != SOCK_STREAM) &&
				    (sep->se_socktype != SOCK_TLI))
					recv(0, buf, sizeof (buf), 0);
				syslog(LOG_ERR, "execv %s: %m", sep->se_server);
				_exit(1);
			}
		}
		if (ctrlisnew)
			(void) close(ctrl);
	}
    }
}

void
reapchild()
{

	pid_t pid;
	register struct servtab *sep;
#ifdef SYSV
#define sys_siglist _sys_siglist
#endif SYSV
	extern char *sys_siglist[];

#ifdef SYSV
	for (;;) {
		int options; 
		int error;
		siginfo_t info;
		
		options = WNOHANG | WEXITED;
		bzero ((char *) &info, sizeof(info));
		error = waitid(P_ALL, 0, &info, options);
		if ((error == -1) || (info.si_pid == 0))
			break;
		if (debug)
			fprintf(debugfp, "%d reaped\n", info.si_pid);
		for (sep = servtab; sep; sep = sep->se_next)
			if (sep->se_wait == info.si_pid) {
				if (info.si_status) {
					if (info.si_signo) {
						if (info.si_signo > NSIG)
							syslog(LOG_WARNING,
							    "%s: Signal %d%s",
							    sep->se_server,
							    info.si_signo,
							    ((info.si_code == 
							      CLD_DUMPED)?
							    " - core dumped" :
							    ""));
						else
							syslog(LOG_WARNING,
							    "%s: %s%s",
							    sep->se_server,
							    sys_siglist[info.si_signo],
							    ((info.si_code == 
							      CLD_DUMPED)?
							    " - core dumped" :
							    ""));
					} else {
						syslog(LOG_WARNING,
						    "%s: exit status %d",
						    sep->se_server,
						    info.si_status);
					}
				}
#else
	for (;;) {
		union wait status;
		pid = wait3(&status, WNOHANG, (struct rusage *)0);
		if (pid <= 0)
			break;
		if (debug)
			fprintf(debugfp, "%d reaped\n", pid);
		for (sep = servtab; sep; sep = sep->se_next)
			if (sep->se_wait == pid) {
				if (status.w_status) {
					if (status.w_termsig) {
						if (status.w_termsig > NSIG)
							syslog(LOG_WARNING,
							    "%s: Signal %d%s",
							    sep->se_server,
							    status.w_termsig,
							    (status.w_coredump?
							    " - core dumped" :
							    ""));
						else
							syslog(LOG_WARNING,
							    "%s: %s%s",
							    sep->se_server,
							    sys_siglist[status.w_termsig],
							    (status.w_coredump?
							    " - core dumped" :
							    ""));
					} else {
						syslog(LOG_WARNING,
						    "%s: exit status %d",
						    sep->se_server,
						    status.w_retcode);
					}
				}
#endif /* SYSV */
				if (debug)
					fprintf(debugfp, "restored %s, fd %d\n",
					    sep->se_service, sep->se_fd);
				FD_SET(sep->se_fd, &allsock);
				nsock++;
				sep->se_wait = 1;
			}
	}
}

void
config()
{
	register struct servtab *sep, *cp, **sepp;
	struct servtab *getconfigent(), *enter();
	int omask;

	if (!setconfig()) {
		syslog(LOG_ERR, "%s: %m", CONFIG);
		return;
	}
	for (sep = servtab; sep; sep = sep->se_next)
		sep->se_checked = 0;
	while (cp = getconfigent()) {
		for (sep = servtab; sep; sep = sep->se_next) {
			if (sep->se_isrpc) {
				if (cp->se_isrpc
				    && sep->se_rpc.prog == cp->se_rpc.prog
				    && strcmp(sep->se_proto, cp->se_proto) == 0)
					break;
			} else {
				if (!cp->se_isrpc
				    && strcmp(sep->se_service, cp->se_service) == 0
				    && strcmp(sep->se_proto, cp->se_proto) == 0)
					break;
			}
		}
		if (sep != 0) {
			int i;

			omask = sigblock(SIGBLOCK);
			if (cp->se_bi == 0)
				sep->se_wait = cp->se_wait;
#define SWAP(a, b) { char *c = a; a = b; b = c; }
			if (cp->se_user)
				SWAP(sep->se_user, cp->se_user);
			if (cp->se_server)
				SWAP(sep->se_server, cp->se_server);
			for (i = 0; i < MAXARGV; i++)
				SWAP(sep->se_argv[i], cp->se_argv[i]);
			sigsetmask(omask);
			freeconfig(cp);
			if (debug)
				print_service("REDO", sep);
		} else {
			sep = enter(cp);
			if (debug)
				print_service("ADD ", sep);
		}
		sep->se_checked = 1;
		if (sep->se_isrpc) {
			if (sep->se_fd != -1)
				termserv(sep);
			else
				unregister(sep);	/* just in case */
			if (nservers >= maxservers) {
				syslog(LOG_ERR, "%s/%s: too many services (max %d)",
				    sep->se_service, sep->se_proto,
				    maxservers);
				sep->se_fd = -1;
				continue;
			}
			setuprpc(sep);
		} else {
			sp = getservbyname(sep->se_service, sep->se_proto);
			if (sp == NULL) {
				syslog(LOG_ERR, "%s/%s: unknown service",
				    sep->se_service, sep->se_proto);
				if (sep->se_fd != -1)
					termserv(sep);
				continue;
			}
			if (sp->s_port != sep->se_ctrladdr.sin_port) {
				sep->se_ctrladdr.sin_port = sp->s_port;
				if (sep->se_fd != -1)
					termserv(sep);
			}
			if (sep->se_fd == -1) {
				if (nservers >= maxservers) {
					syslog(LOG_ERR, "%s/%s: too many services (max %d)",
					    sep->se_service, sep->se_proto,
					    maxservers);
					sep->se_fd = -1;
					continue;
				}
				setup(sep);
			}
		}
	}
	endconfig();
	/*
	 * Purge anything not looked at above.
	 * XXX - if we add a service and delete one, the new service
	 * will be added to the count of services before the deleted one
	 * is subtracted.  This means you may run out of services if this
	 * happens; however, we can't do much about that, since we get
	 * the socket for the new one before we close the one for the
	 * deleted service, and if we allowed that extra service we might
	 * run out of descriptors.
	 */
	omask = sigblock(SIGBLOCK);
	sepp = &servtab;
	while (sep = *sepp) {
		if (sep->se_checked) {
			sepp = &sep->se_next;
			continue;
		}
		*sepp = sep->se_next;
		if (sep->se_fd != -1)
			termserv(sep);
		if (debug)
			print_service("FREE", sep);
		freeconfig(sep);
		free((char *)sep);
	}
#ifdef SYSV
	/* in case we are disabled */
	if (!standalone && (State == PM_DISABLED)) {
		for (sep = servtab; sep; sep = sep->se_next) {
			if (sep->se_fd != -1)
				termserv(sep);
		}
	}
#endif SYSV
	(void) sigsetmask(omask);
}

/*
 * Try again to establish sockets on which to listen for requests
 * for non-RPC-based services (if the attempt failed before, it was either
 * because a socket could not be created, or more likely because the socket
 * could not be bound to the service's address - probably because there was
 * already a daemon out there with a socket bound to that address).
 */
void
retry()
{
	register struct servtab *sep;

	timingout = 0;
	for (sep = servtab; sep; sep = sep->se_next) {
		if (sep->se_fd == -1 && !sep->se_isrpc) {
			if (nservers < maxservers)
				setup(sep);
		}
	}
}

/*
 * Set up to accept requests for a non-RPC-based service.  Create a socket
 * to listen for connections or datagrams, bind it to the address of that
 * service, add its socket descriptor to the list of descriptors to poll,
 * and increment the number of socket descriptors and active services.
 */
setup(sep)
	register struct servtab *sep;
{
	int on = 1;

	if (sep->se_socktype == SOCK_TLI) {
		int qlen;

		if ((sep->se_fd = tli_socket(sep->se_dev, &sep->se_info)) 
		    < 0) {
			syslog(LOG_ERR, "%s/%s: tli_socket: %m",
			       sep->se_service, sep->se_proto);
			return;
		}
		
		if (sep->se_info.servtype == T_CLTS)
			qlen = 0;
		else
			qlen = 10;

		if (tli_bind(sep->se_fd, &sep->se_ctrladdr,
			     sizeof (sep->se_ctrladdr), qlen) < 0) {
			syslog(LOG_ERR, "%s/%s: tli_bind: %m",
			       sep->se_service, sep->se_proto);
			(void) close(sep->se_fd);
			sep->se_fd = -1;
			if (!timingout) {
				timingout = 1;
				alarm(RETRYTIME);
			}
			return;
		}
	} else {
		if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) {
			syslog(LOG_ERR, "%s/%s: socket: %m",
			       sep->se_service, sep->se_proto);
			return;
		}

#define	turnon(fd, opt) \
setsockopt(fd, SOL_SOCKET, opt, (char *)&on, sizeof (on))
		if (strcmp(sep->se_proto, "tcp") == 0 && (options & SO_DEBUG) &&
		    turnon(sep->se_fd, SO_DEBUG) < 0)
			syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m");
		if (turnon(sep->se_fd, SO_REUSEADDR) < 0)
			syslog(LOG_ERR, "setsockopt (SO_REUSEADDR): %m");
#undef turnon

		if (bind(sep->se_fd, &sep->se_ctrladdr,
			 sizeof (sep->se_ctrladdr)) < 0) {
			syslog(LOG_ERR, "%s/%s: bind: %m",
			       sep->se_service, sep->se_proto);
			(void) close(sep->se_fd);
			sep->se_fd = -1;
			if (!timingout) {
				timingout = 1;
				alarm(RETRYTIME);
			}
			return;
		}
		if (sep->se_socktype == SOCK_STREAM)
			listen(sep->se_fd, 10);
	}
	FD_SET(sep->se_fd, &allsock);
	nsock++;
	if (sep->se_fd > maxsock)
		maxsock = sep->se_fd;
	nservers++;
}

/*
 * Set up to accept requests for an RPC-based service.  Create a socket
 * to listen for connections or datagrams, bind it to a system-chosen
 * address, register it with the portmapper under that address, add its
 * socket descriptor to the list of descriptors to poll, and increment
 * the number of socket descriptors and active services.
 */
setuprpc(sep)
	register struct servtab *sep;
{
	register int i;
	short port;
	struct sockaddr_in addr;
	struct netbuf *na;
	struct netconfig *nconf;
	char buf[32];
	int len = sizeof(struct sockaddr_in);
	
	if (sep->se_socktype == SOCK_TLI) {
		int qlen;

		if ((sep->se_fd = tli_socket(sep->se_dev, &sep->se_info)) 
		    < 0) {
			syslog(LOG_ERR, "%s/%s: tli_socket: %m",
			       sep->se_service, sep->se_proto);
			return;
		}
		
		if (sep->se_info.servtype == T_CLTS)
			qlen = 0;
		else
			qlen = 10;

		addr.sin_family = AF_INET;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (tli_bind(sep->se_fd, &addr, sizeof (addr), qlen) < 0) {
			syslog(LOG_ERR, "%s/%s: tli_bind: %m",
			       sep->se_service, sep->se_proto);
			(void) close(sep->se_fd);
			sep->se_fd = -1;
			return;
		}
	} else {
		if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) {
			syslog(LOG_ERR, "%s/%s: socket: %m",
			       sep->se_service, sep->se_proto);
			return;
		}
		addr.sin_family = AF_INET;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(sep->se_fd, &addr, sizeof(addr)) < 0) {
			syslog(LOG_ERR, "%s/%s: bind: %m",
			       sep->se_service, sep->se_proto);
			(void) close(sep->se_fd);
			sep->se_fd = -1;
			return;
		}
		if (getsockname(sep->se_fd, &addr, &len) != 0) {
			syslog(LOG_ERR, "%s/%s: getsockname: %m",
			       sep->se_service, sep->se_proto);
			(void) close(sep->se_fd);
			sep->se_fd = -1;
			return;
		}
	}
	for (i = sep->se_rpc.lowvers; i <= sep->se_rpc.highvers; i++) {
		/*
		 * this code take from librpc/pmap_clnt.c
		 */

		/* sep->se_proto is of the form "rpc/tcp" for rpc services */
		nconf = _rpc_getconfip(sep->se_proto + strlen ("rpc/"));
		if ( !nconf )
			continue;	/* for */
		port = ntohs(addr.sin_port);
		(void) sprintf(buf, "0.0.0.0.%d.%d", ((port >> 8) & 0xff), 
			       (port & 0xff));
		na = uaddr2taddr(nconf, buf);
		if ( !na ) {
			freenetconfigent(nconf);
			continue;	/* for */
		}
		(void) rpcb_set(sep->se_rpc.prog, i, nconf, na);
		netdir_free((char *)na, ND_ADDR);
		freenetconfigent(nconf);
	}
	if (sep->se_socktype == SOCK_STREAM)
		listen(sep->se_fd, 10);
	FD_SET(sep->se_fd, &allsock);
	nsock++;
	if (sep->se_fd > maxsock)
		maxsock = sep->se_fd;
	nservers++;
}

/*
 * Shut down a service.  Unregister it if it's an RPC service, remove
 * its socket descriptor from the list of descriptors to poll, close
 * that socket descriptor, set it to -1 (to indicate that it's shut down),
 * and decrement the number of socket descriptors and active services.
 */
void
termserv(sep)
	register struct servtab *sep;
{
	if (sep->se_isrpc) {
		/*
		 * Have to do this here because there might be multiple
		 * registers of the same prognum.
		 */
		unregister(sep);
	}
	FD_CLR(sep->se_fd, &allsock);
	nsock--;
	(void) close(sep->se_fd);
	sep->se_fd = -1;
	nservers--;
}

/*
 * Unregister an RPC service.
 */
void
unregister(sep)
	register struct servtab *sep;
{
	register int i;
	struct netconfig *udpnconf, *tcpnconf;
	
	udpnconf = _rpc_getconfip("udp");
	tcpnconf = _rpc_getconfip("tcp");
	for (i = sep->se_rpc.lowvers; i <= sep->se_rpc.highvers; i++) {
		if ( udpnconf )
			(void) rpcb_unset(sep->se_rpc.prog, i, udpnconf);
		if ( tcpnconf )
			(void) rpcb_unset(sep->se_rpc.prog, i, tcpnconf);
	}
	if ( udpnconf )
		freenetconfigent(udpnconf);
	if ( tcpnconf )
		freenetconfigent(tcpnconf);
}

struct servtab *
enter(cp)
	struct servtab *cp;
{
	register struct servtab *sep;
	int omask;

	sep = (struct servtab *)malloc(sizeof (*sep));
	if (sep == (struct servtab *)0) {
		syslog(LOG_ERR, "Out of memory.");
		exit(-1);
	}
	*sep = *cp;
	sep->se_fd = -1;
	omask = sigblock(SIGBLOCK);
	sep->se_next = servtab;
	servtab = sep;
	sigsetmask(omask);
	return (sep);
}

FILE	*fconfig = NULL;
struct	servtab serv;
char	line[256];
char	*skip(), *nextline();

setconfig()
{

	if (fconfig != NULL) {
		fseek(fconfig, 0L, L_SET);
		return (1);
	}
	fconfig = fopen(CONFIG, "r");
	return (fconfig != NULL);
}

endconfig()
{

	if (fconfig == NULL)
		return;
	fclose(fconfig);
	fconfig = NULL;
}

struct servtab *
getconfigent()
{
	register struct servtab *sep = &serv;
	char *cp, *arg, *tp, *strp;
	int argc;
	struct rpcent *rpc;
	char *strdup();

more:
	bzero((char *) sep, sizeof(struct servtab));
	while ((cp = nextline(fconfig)) && *cp == '#')
		;
	if (cp == (char *) 0)
		return ((struct servtab *)0);
	sep->se_service = strdup(skip(&cp));
	arg = skip(&cp);
	if (strcmp(arg, "stream") == 0)
		sep->se_socktype = SOCK_STREAM;
	else if (strcmp(arg, "dgram") == 0)
		sep->se_socktype = SOCK_DGRAM;
	else if (strcmp(arg, "rdm") == 0)
		sep->se_socktype = SOCK_RDM;
	else if (strcmp(arg, "seqpacket") == 0)
		sep->se_socktype = SOCK_SEQPACKET;
	else if (strcmp(arg, "raw") == 0)
		sep->se_socktype = SOCK_RAW;
	else if (strcmp(arg, "tli") == 0)
		sep->se_socktype = SOCK_TLI;
	else
		sep->se_socktype = 0;

	sep->se_proto = strdup(skip(&cp));

	/*
	 * If this is a TLI service, we overload the "proto" field
	 * to identify the transport provider.  If the name begins with
	 * a "/", we assume it is a full pathname to the device. 
	 * Otherwise, we assume it names the final component under
	 * "/dev".  Perhaps we should consider it a netid in this case.
	 */
	if (sep->se_socktype == SOCK_TLI) {
		if (sep->se_proto[0] != '/') {
			char buf[MAXPATHLEN];
			if (strncmp(sep->se_proto, "rpc/", 4) == 0) {
				sprintf(buf, "/dev/%s", sep->se_proto + 
					strlen("rpc/"));
			} else
				sprintf(buf, "/dev/%s", sep->se_proto);
			sep->se_dev = strdup(buf);
		} else
			sep->se_dev = strdup(sep->se_proto);
	}
		
	if (strncmp(sep->se_proto, "rpc/", 4) == 0) {
		sep->se_isrpc = 1;
		sep->se_rpc.lowvers = 0;
		sep->se_rpc.highvers = 0;
		tp = sep->se_service;
		while (*tp != '/') {
			if (*tp == '\0') {
				sep->se_rpc.lowvers = 1;
				sep->se_rpc.highvers = 1;
				break;
			} else
				tp++;
		}
		*tp = '\0';
		if ((rpc = getrpcbyname(sep->se_service)) != NULL)
			sep->se_rpc.prog = rpc->r_number;
		else {
			strp = sep->se_service;
			sep->se_rpc.prog = strtol(strp, &strp, 10);
			if (strp != tp) {
				/*
				 * Service name isn't all-numeric, so
				 * since we didn't find it it must not
				 * be known.
				 */
				syslog(LOG_ERR, "%s/%s: unknown service",
				    sep->se_service, sep->se_proto);
				freeconfig(sep);
				goto more;
			}
		}
		if (sep->se_rpc.lowvers == 0) {
			/*
			 * The service name ended with a slash, so the
			 * version number(s) are explicitly specified.
			 */
			tp++;
			strp = tp;
			sep->se_rpc.lowvers = strtol(tp, &strp, 10);
			tp = strp;
			if (*tp == '-') {
				tp++;
				strp = tp;
				sep->se_rpc.highvers = strtol(tp, &strp, 10);
				if (*strp != '\0') {
					syslog(LOG_ERR, "%s/%s: bad high version number",
					    sep->se_service, sep->se_proto);
					freeconfig(sep);
					goto more;
				}
			} else if (*tp == '\0')
				sep->se_rpc.highvers = sep->se_rpc.lowvers;
			else {
				syslog(LOG_ERR, "%s/%s: bad version number",
				    sep->se_service, sep->se_proto);
				freeconfig(sep);
				goto more;
			}
		}
	} else
		sep->se_isrpc = 0;

	arg = skip(&cp);
	sep->se_wait = strcmp(arg, "wait") == 0;
	sep->se_user = strdup(skip(&cp));
	sep->se_server = strdup(skip(&cp));
	if (strcmp(sep->se_server, "internal") == 0) {
		register struct biltin *bi;

		for (bi = biltins; bi->bi_service; bi++)
			if (bi->bi_socktype == sep->se_socktype &&
			    strcmp(bi->bi_service, sep->se_service) == 0)
				break;
		if (bi->bi_service == 0) {
			syslog(LOG_ERR, "internal service %s/%s unknown",
			    sep->se_service, sep->se_proto);
			freeconfig(sep);
			goto more;
		}
		sep->se_bi = bi;
		sep->se_wait = bi->bi_wait;
	} else
		sep->se_bi = NULL;
	argc = 0;
	for (arg = skip(&cp); cp; arg = skip(&cp))
		if (argc < MAXARGV)
			sep->se_argv[argc++] = strdup(arg);
	while (argc <= MAXARGV)
		sep->se_argv[argc++] = NULL;
	return (sep);
}

freeconfig(cp)
	register struct servtab *cp;
{
	int i;

	if (cp->se_service)
		free(cp->se_service);
	if (cp->se_proto)
		free(cp->se_proto);
	if (cp->se_user)
		free(cp->se_user);
	if (cp->se_server)
		free(cp->se_server);
	for (i = 0; i < MAXARGV; i++)
		if (cp->se_argv[i])
			free(cp->se_argv[i]);
}

char *
skip(cpp)
	char **cpp;
{
	register char *cp = *cpp;
	char *start;

again:
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '\0') {
		int i;

		if ((i = getc(fconfig)) != EOF) {
			char c = (char) i;
			ungetc(c, fconfig);
			if (c == ' ' || c == '\t')
				if (cp = nextline(fconfig))
					goto again;
		}
		*cpp = (char *)0;
		return ((char *)0);
	}
	start = cp;
	while (*cp && *cp != ' ' && *cp != '\t')
		cp++;
	if (*cp != '\0')
		*cp++ = '\0';
	*cpp = cp;
	return (start);
}

char *
nextline(fd)
	FILE *fd;
{
	char *cp;

	if (fgets(line, sizeof (line), fd) == NULL)
		return ((char *)0);
	cp = index(line, '\n');
	if (cp)
		*cp = '\0';
	return (line);
}

char *
strdup(cp)
	char *cp;
{
	char *new;

	if (cp == NULL)
		cp = "";
	new = malloc((unsigned)(strlen(cp) + 1));
	if (new == (char *)0) {
		syslog(LOG_ERR, "Out of memory.");
		exit(-1);
	}
	strcpy(new, cp);
	return (new);
}

setproctitle(a, s)
	char *a;
	int s;
{
	int size;
	register char *cp;
	struct sockaddr_in sin;
	char buf[80];

	cp = Argv[0];
	size = sizeof(sin);
	if (getpeername(s, &sin, &size) == 0)
		sprintf(buf, "-%s [%s]", a, inet_ntoa(sin.sin_addr)); 
	else
		sprintf(buf, "-%s", a); 
	strncpy(cp, buf, LastArg - cp);
	cp += strlen(cp);
	while (cp < LastArg)
		*cp++ = ' ';
}

/*
 * Internet services provided internally by inetd:
 */

/* ARGSUSED */
echo_stream(s, sep)		/* Echo service -- echo data back */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZ];
	int i;

	setproctitle("echo", s);
	while ((i = read(s, buffer, sizeof(buffer))) > 0 &&
	    write(s, buffer, i) > 0)
		;
	exit(0);
}

/* ARGSUSED */
echo_dg(s, sep)			/* Echo service -- echo data back */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZ];
	int i, size;
	struct sockaddr sa;

	size = sizeof(sa);
	if ((i = recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size)) < 0)
		return;
	(void) sendto(s, buffer, i, 0, &sa, sizeof(sa));
}

/* ARGSUSED */
discard_stream(s, sep)		/* Discard service -- ignore data */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZ];

	setproctitle("discard", s);
	while (1) {
		while (read(s, buffer, sizeof(buffer)) > 0)
			;
		if (errno != EINTR)
			break;
	}
	exit(0);
}

/* ARGSUSED */
discard_dg(s, sep)		/* Discard service -- ignore data */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZ];

	(void) read(s, buffer, sizeof(buffer));
}

#include <ctype.h>
#define LINESIZ 72
char ring[128];
char *endring;

initring()
{
	register int i;

	endring = ring;

	for (i = 0; i <= 128; ++i)
		if (isprint(i))
			*endring++ = i;
}

/* ARGSUSED */
chargen_stream(s, sep)		/* Character generator */
	int s;
	struct servtab *sep;
{
	char text[LINESIZ+2];
	register int i;
	register char *rp, *rs, *dp;

	setproctitle("discard", s);
	if (endring == 0)
		initring();

	for (rs = ring; ; ++rs) {
		if (rs >= endring)
			rs = ring;
		rp = rs;
		dp = text;
		i = MIN(LINESIZ, endring - rp);
		bcopy(rp, dp, i);
		dp += i;
		if ((rp += i) >= endring)
			rp = ring;
		if (i < LINESIZ) {
			i = LINESIZ - i;
			bcopy(rp, dp, i);
			dp += i;
			if ((rp += i) >= endring)
				rp = ring;
		}
		*dp++ = '\r';
		*dp++ = '\n';

		if (write(s, text, dp - text) != dp - text)
			break;
	}
	exit(0);
}

/* ARGSUSED */
chargen_dg(s, sep)		/* Character generator */
	int s;
	struct servtab *sep;
{
	char text[LINESIZ+2];
	register int i;
	register char *rp;
	static char *rs = ring;
	struct sockaddr sa;
	int size;

	if (endring == 0)
		initring();

	size = sizeof(sa);
	if (recvfrom(s, text, sizeof(text), 0, &sa, &size) < 0)
		return;
	rp = rs;
	if (rs++ >= endring)
		rs = ring;
	i = MIN(LINESIZ - 2, endring - rp);
	bcopy(rp, text, i);
	if ((rp += i) >= endring)
		rp = ring;
	if (i < LINESIZ - 2) {
		bcopy(rp, text, i);
		if ((rp += i) >= endring)
			rp = ring;
	}
	text[LINESIZ - 2] = '\r';
	text[LINESIZ - 1] = '\n';

	(void) sendto(s, text, sizeof(text), 0, &sa, sizeof(sa));
}

/*
 * Return a machine readable date and time, in the form of the
 * number of seconds since midnight, Jan 1, 1900.  Since gettimeofday
 * returns the number of seconds since midnight, Jan 1, 1970,
 * we must add 2208988800 seconds to this figure to make up for
 * some seventy years Bell Labs was asleep.
 */

long
machtime()
{
	struct timeval tv;

	if (gettimeofday(&tv, (struct timezone *)0) < 0) {
#ifdef SYSV
		syslog(LOG_INFO, "Unable to get time of day\n");
#else
		fprintf(debugfp, "Unable to get time of day\n");
#endif SYSV
		return (0L);
	}
	return (htonl((long)tv.tv_sec + 2208988800));
}

/* ARGSUSED */
machtime_stream(s, sep)
	int s;
	struct servtab *sep;
{
	long result;

	result = machtime();
	(void) write(s, (char *) &result, sizeof(result));
}

/* ARGSUSED */
machtime_dg(s, sep)
	int s;
	struct servtab *sep;
{
	long result;
	struct sockaddr sa;
	int size;

	size = sizeof(sa);
	if (recvfrom(s, (char *)&result, sizeof(result), 0, &sa, &size) < 0)
		return;
	result = machtime();
	(void) sendto(s, (char *) &result, sizeof(result), 0, &sa, sizeof(sa));
}

/* ARGSUSED */
daytime_stream(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	char *ctime();

	clock = time((time_t *) 0);

	sprintf(buffer, "%s\r", ctime(&clock));
	(void) write(s, buffer, strlen(buffer));
}

/* ARGSUSED */
daytime_dg(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	struct sockaddr sa;
	int size;
	char *ctime();

	clock = time((time_t *) 0);

	size = sizeof(sa);
	if (recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size) < 0)
		return;
	sprintf(buffer, "%s\r", ctime(&clock));
	(void) sendto(s, buffer, strlen(buffer), 0, &sa, sizeof(sa));
}

/*
 * print_service:
 *	Dump relevant information to stderr
 */
print_service(action, sep)
	char *action;
	struct servtab *sep;
{
	fprintf(debugfp,
	    "%s: %s proto=%s, wait=%d, user=%s builtin=%x server=%s\n",
	    action, sep->se_service, sep->se_proto,
	    sep->se_wait, sep->se_user, sep->se_bi, sep->se_server);
}


/*
 * A call that looks alot like accept(), but uses tli.
 */
int
tli_accept(fd, addr, len, dev) 
	int fd;
	char *addr;
	int *len;
	char *dev;
{
	struct t_call *call;
	int newfd;
	
	if ((call = (struct t_call *) 
	     t_alloc(fd, T_CALL, T_ALL)) == (struct t_call *) 0) {
		syslog(LOG_ERR, "tli_accept: t_alloc");
		return(-1);
	}
	if (t_listen(fd, call) == -1) {
		syslog(LOG_ERR, "tli_accept: t_listen");
		(void) t_free (call, T_CALL);
		return(-1);
	}
	bcopy(call->addr.buf, addr,
	      MIN (call->addr.len, sizeof(struct sockaddr_in)));
	*len = call->addr.len;
	if ((newfd = t_open(dev, O_RDWR, (struct t_info *) 0)) == -1) {
		syslog(LOG_ERR, "tli_accept: t_open");
		(void) t_free (call, T_CALL);
		return(-1);
	}
	if (t_bind(newfd, (struct t_bind *) 0, (struct t_bind *) 0) == -1) {
		syslog(LOG_ERR, "tli_accept: t_bind");
		(void) t_free (call, T_CALL);
		return(-1);
	}
	if (t_accept (fd, newfd, call) == -1) {
		if (debug)
			t_error("tli_accept: t_accept");
		syslog(LOG_ERR, "tli_accept: t_accept");
		(void) t_free (call, T_CALL);
		return(-1);
	}
	(void) t_free (call, T_CALL);
	return (newfd);
}






/*
 * A routine that doesn't look a whole lot like socket(), but
 * has the same effect, and uses tli.
 */
int
tli_socket(dev, info)
	char *dev;
	struct t_info *info;
{
	return (t_open(dev, O_RDWR, info));
}


/*
 * A routine vaguely reminicent of bind(), but which uses tli.
 */
int 
tli_bind(fd, addr, addrlen, qlen)
	int fd;
	struct sockaddr_in *addr;
	int addrlen;
	int qlen;
{
	struct t_bind *req, *ret;
	struct sockaddr_in *sin;
	extern u_int t_errno, t_nerr;
	extern char **t_errlist;
	
	if (((req = (struct t_bind *) t_alloc (fd, T_BIND, T_ALL)) ==
	     (struct t_bind *) NULL) ||
	    ((ret = (struct t_bind *) t_alloc (fd, T_BIND, T_ALL)) ==
	     (struct t_bind *) NULL)) {
		if ((t_errno == TSYSERR) || (t_errno > t_nerr))
			syslog(LOG_ERR, "tli_bind: t_alloc: %m");
		else
			syslog(LOG_ERR, "tli_bind: t_alloc: %s",
			       t_errlist[t_errno]);
		return(-1);
	}
	
	sin = (struct sockaddr_in *) req->addr.buf;
	bzero ((char *) sin, sizeof(struct sockaddr_in));
	sin->sin_family = AF_INET;
	sin->sin_port = addr->sin_port;
	req->addr.len = sizeof (struct sockaddr_in);
	req->qlen = qlen;
	if (t_bind (fd, req, ret) == -1) {
		if ((t_errno == TSYSERR) || (t_errno > t_nerr))
			syslog(LOG_ERR, "tli_bind:  t_bind: %m");
		else
			syslog(LOG_ERR, "tli_bind: t_bind: %s",
			       t_errlist[t_errno]);
		(void) t_free ((char *) req, T_BIND);
		(void) t_free ((char *) ret, T_BIND);
		return(-1);
	}
	
	sin = (struct sockaddr_in *) ret->addr.buf;
	if (addr->sin_port && (sin->sin_port != addr->sin_port)) {
		/* 
		 * XXX - TLI bind semantics lets the transport
		 * provider pick a new port number for us if 
		 * the one we asked for is "busy". 
		 */
		syslog(LOG_ERR, "tli_bind: t_bind gave us port %d",
		       sin->sin_port);
		(void) t_free ((char *) req, T_BIND);
		(void) t_free ((char *) ret, T_BIND);
		return(-1);
	}
	/*
	 * return bound address to user, unlike bind()
	 */
	addr->sin_addr = sin->sin_addr;
	addr->sin_port = sin->sin_port;
	(void) t_free ((char *) req, T_BIND);
	(void) t_free ((char *) ret, T_BIND);
	return(0);
}

#ifdef SYSV

#include <sys/resource.h>

#define	NOFILES 20	/* just in case */

int
getdtablesize()
{
	struct rlimit	rl;

	if ( getrlimit(RLIMIT_NOFILE, &rl) == 0 )
		return(rl.rlim_max);
	else
		return(NOFILES);
}


void
sacmesg()
{
	struct sacmsg sacmsg;
	register struct servtab *sep;

	if (read(Pfd, &sacmsg, sizeof(sacmsg)) < 0) {
		syslog(LOG_ERR, "Could not read _pmpipe");
		exit(-1);
	}
	switch (sacmsg.sc_type) {
	case SC_STATUS:
		Pmmsg.pm_type = PM_STATUS;
		Pmmsg.pm_state = State;
		break;
	case SC_ENABLE:
		syslog(LOG_INFO, "got SC_ENABLE message");
		if (State != PM_ENABLED) {
			for (sep = servtab; sep; sep = sep->se_next) {
				if ((sep->se_fd == -1) && (nservers < maxservers)) {
					if (sep->se_isrpc)
						setuprpc(sep);
					else
						setup(sep);
				}
			}
		}
		State = PM_ENABLED;
		Pmmsg.pm_type = PM_STATUS;
		Pmmsg.pm_state = State;
		break;
	case SC_DISABLE:
		syslog(LOG_INFO, "got SC_DISABLE message");
		if (State != PM_DISABLED) {
			for (sep = servtab; sep; sep = sep->se_next) {
				if (sep->se_fd != -1)
					termserv(sep);
			}
		}
		State = PM_DISABLED;
		Pmmsg.pm_type = PM_STATUS;
		Pmmsg.pm_state = State;
		break;
	case SC_READDB:
		/* ignore these, inetd doesn't use _pmtab's right now */
		syslog(LOG_INFO, "got SC_READDB message");
		Pmmsg.pm_type = PM_STATUS;
		Pmmsg.pm_state = State;
		break;
	default:
		syslog(LOG_WARNING, "got unknown message <%d> from sac", sacmsg.sc_type);
		Pmmsg.pm_type = PM_UNKNOWN;
		Pmmsg.pm_state = State;
		break;
	}
	if (write(Sfd, &Pmmsg, sizeof(Pmmsg)) != sizeof(Pmmsg))
		syslog(LOG_WARNING, "sanity response failed");
}

void
sigterm()
{
	syslog(LOG_INFO, "inetd terminating");
	exit(0);
}
#endif SYSV

