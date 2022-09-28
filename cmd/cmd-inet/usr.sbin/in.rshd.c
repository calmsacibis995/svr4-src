/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.rshd.c	1.8.4.1"

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
 * remote shell server:
 *	remuser\0
 *	locuser\0
 *	command\0
 *	data
 */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#ifdef AUDIT
#include <sys/label.h>
#include <sys/audit.h>
#endif AUDIT

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#ifdef AUDIT
#include <pwdadj.h>
#endif AUDIT
#include <signal.h>
#include <netdb.h>
#include <syslog.h>

#ifdef SYSV
#include <sys/resource.h>
#include <sys/filio.h>

#define	killpg(a,b)	kill(-(a),(b))
#define rindex strrchr
#define index strchr
#endif SYSV

#ifndef NCARGS
#define NCARGS	5120
#endif /* NCARGS */

#ifdef AUDIT
#define AUDITNUM sizeof(audit_argv) / sizeof(char *)
#endif AUDIT

int	errno;
char	*index(), *rindex(), *strncat();
#ifdef AUDIT
char	*audit_argv[] = {"in.rshd", 0, 0, 0, 0, 0};
#endif AUDIT
/*VARARGS1*/
int	error();

/*ARGSUSED*/
main(argc, argv)
	int argc;
	char **argv;
{
	struct linger linger;
	int on = 1, fromlen;
	struct sockaddr_in from;

	openlog("rsh", LOG_PID | LOG_ODELAY, LOG_DAEMON);
	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
		_exit(1);
	}
	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
	    sizeof (on)) < 0)
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	linger.l_onoff = 1;
	linger.l_linger = 60;			/* XXX */
	if (setsockopt(0, SOL_SOCKET, SO_LINGER, (char *)&linger,
	    sizeof (linger)) < 0)
		syslog(LOG_WARNING, "setsockopt (SO_LINGER): %m");
	doit(dup(0), &from);
	/* NOTREACHED */
}

char	username[20] = "USER=";
char	homedir[64] = "HOME=";
char	shell[64] = "SHELL=";
char	*envinit[] =
#ifdef SYSV
	    {homedir, shell, "PATH=:/bin:/usr/bin", username, 0};
#else
	    {homedir, shell, "PATH=:/usr/ucb:/bin:/usr/bin", username, 0};
#endif /* SYSV */
char	**environ;

static char cmdbuf[NCARGS+1];

doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	char *cp;
	char locuser[16], remuser[16];

	struct passwd *pwd;
#ifdef AUDIT
	struct passwd_adjunct *apw, *getpwanam();
	audit_state_t astate;
#endif AUDIT

	int s;
	struct hostent *hp;
	char *hostname;
	short port;
	pid_t pid;
	int pv[2], cc;
	char buf[BUFSIZ], sig;
	int one = 1;

	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);
#ifdef SYSV
	(void) sigset(SIGCHLD, SIG_IGN);
#endif /* SYSV */
#ifdef DEBUG
	{ int t = open("/dev/tty", 2);
	  if (t >= 0) {
#ifdef SYSV
		setsid();
#else
		ioctl(t, TIOCNOTTY, (char *)0);
#endif SYSV
		(void) close(t);
	  }
	}
#endif
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	if (fromp->sin_family != AF_INET) {
		syslog(LOG_ERR, "malformed from address\n");
		exit(1);
	}
	if (fromp->sin_port >= IPPORT_RESERVED ||
	    fromp->sin_port < IPPORT_RESERVED/2) {
		syslog(LOG_NOTICE, "connection from bad port\n");
		exit(1);
	}
	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;
		if ((cc = read(f, &c, 1)) != 1) {
			if (cc < 0)
				syslog(LOG_NOTICE, "read: %m");
			shutdown(f, 1+1);
			exit(1);
		}
		if (c == 0)
			break;
		port = port * 10 + c - '0';
	}
	(void) alarm(0);
	if (port != 0) {
		int lport = IPPORT_RESERVED - 1;
		s = rresvport(&lport);
		if (s < 0) {
			syslog(LOG_ERR, "can't get stderr port: %m");
			exit(1);
		}
		if (port >= IPPORT_RESERVED) {
			syslog(LOG_ERR, "2nd port not reserved\n");
			exit(1);
		}
		fromp->sin_port = htons((u_short)port);
		if (connect(s, fromp, sizeof (*fromp)) < 0) {
			syslog(LOG_INFO, "connect second port: %m");
			exit(1);
		}
	}
	dup2(f, 0);
	dup2(f, 1);
	dup2(f, 2);
	hp = gethostbyaddr((char *)&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp)
		hostname = hp->h_name;
	else
		hostname = inet_ntoa(fromp->sin_addr);
	getstr(remuser, sizeof(remuser), "remuser");
	getstr(locuser, sizeof(locuser), "locuser");
	getstr(cmdbuf, sizeof(cmdbuf), "command");
#ifdef AUDIT
    	/*
	 * store common info. for audit record 
     	 */
	audit_argv[1] = remuser; 
	audit_argv[2] = locuser; 
	audit_argv[3] = hostname; 
	audit_argv[4] = cmdbuf; 
#endif AUDIT

	setpwent();
	pwd = getpwnam(locuser);
	if (pwd == NULL) {
		audit_write(1, "Login incorrect"); 
		error("Login incorrect.\n");
		exit(1);
	}
	endpwent();
#ifdef AUDIT
	setauid(pwd->pw_uid);
	/*
	 * get audit flags for user and set for all 
	 * processes owned by this uid 
	 */
	if ((apw = getpwanam(locuser)) != NULL) {
		astate.as_success = 0;
		astate.as_failure = 0;

		if ((getfauditflags(&apw->pwa_au_always, 
		  &apw->pwa_au_never, &astate)) != 0) {
			/*
             		* if we can't tell how to audit from the flags, audit
             		* everything that's not never for this user.
			*/
            		astate.as_success = apw->pwa_au_never.as_success ^ (-1);
            		astate.as_failure = apw->pwa_au_never.as_success ^ (-1);
		}
	}
	else {
		astate.as_success = -1;
		astate.as_failure = -1;
	}

	if (issecure())
		setuseraudit(pwd->pw_uid, &astate);
#endif AUDIT

	if (chdir(pwd->pw_dir) < 0) {
		(void) chdir("/");
#ifdef notdef
		error("No remote directory.\n");
		exit(1);
#endif
	}
	if (pwd->pw_passwd != 0 && *pwd->pw_passwd != '\0' &&
	    ruserok(hostname, pwd->pw_uid == 0, remuser, locuser) < 0) {
		audit_write(1, "Permission denied"); 
		error("Permission denied.\n");
		exit(1);
	}
	(void) write(2, "\0", 1);
	if (port) {
		if (pipe(pv) < 0) {
			audit_write(1, "Can't make pipe."); 
			error("Can't make pipe.\n");
			exit(1);
		}
		pid = fork();
		if (pid == (pid_t)-1)  {
			audit_write(1, "Error in fork()."); 
			error("Try again.\n");
			exit(1);
		}

#ifndef MAX
#define MAX(a,b) (((u_int)(a) > (u_int)(b)) ? (a) : (b))
#endif /* MAX */

		if (pid) {
			int width = MAX(s, pv[0]) + 1;
			fd_set ready;
			fd_set readfrom;

			(void) close(0); (void) close(1); (void) close(2);
			(void) close(f); (void) close(pv[1]);
			FD_ZERO (&ready);
			FD_ZERO (&readfrom);
			FD_SET (s, &readfrom);
			FD_SET (pv[0], &readfrom);
			if (ioctl(pv[0], FIONBIO, (char *)&one) == -1)
				syslog (LOG_INFO, "ioctl FIONBIO: %m");
			/* should set s nbio! */
			do {
				ready = readfrom;
				if (select(width, &ready, (fd_set *)0,
				    (fd_set *)0, (struct timeval *)0) < 0)
					break;
				if (FD_ISSET (s, &ready)) {
					if (read(s, &sig, 1) <= 0)
						FD_CLR (s, &readfrom);
					else
						killpg(pid, sig);
				}
				if (FD_ISSET (pv[0], &ready)) {
					errno = 0;
					cc = read(pv[0], buf, sizeof (buf));
					if (cc <= 0) {
						shutdown(s, 1+1);
						FD_CLR (pv[0], &readfrom);
					} else
						(void) write(s, buf, cc);
				}
			} while (FD_ISSET (s, &readfrom) || 
				 FD_ISSET (pv[0], &readfrom));
			exit(0);
		}
		setpgrp(0, getpid());
		(void) close(s); (void) close(pv[0]);
		dup2(pv[1], 2);
		(void) close(pv[1]);
	}
	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = "/bin/sh";
	(void) close(f);
	if (  setgid((gid_t)pwd->pw_gid) < 0 ) {
		error("Invalid gid.\n");
		exit(1);
	}
	initgroups(pwd->pw_name, pwd->pw_gid);

	/*
	 * write audit record before making uid switch  
	 */
	audit_write(0, "authorization successful"); 

	if ( setuid((uid_t)pwd->pw_uid) < 0 ) {
		error("Invalid uid.\n");
		exit(1);
	}
	environ = envinit;
	strncat(homedir, pwd->pw_dir, sizeof(homedir)-6);
	strncat(shell, pwd->pw_shell, sizeof(shell)-7);
	strncat(username, pwd->pw_name, sizeof(username)-6);
	cp = rindex(pwd->pw_shell, '/');
	if (cp)
		cp++;
	else
		cp = pwd->pw_shell;
	execl(pwd->pw_shell, cp, "-c", cmdbuf, (char *)0);
	perror(pwd->pw_shell);
	audit_write(1, "can't exec");
	exit(1);
}

/*VARARGS1*/
error(fmt, a1, a2, a3)
	char *fmt;
	int a1, a2, a3;
{
	char buf[BUFSIZ];

	buf[0] = 1;
	(void) sprintf(buf+1, fmt, a1, a2, a3);
	(void) write(2, buf, strlen(buf));
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		*buf++ = c;
		if (--cnt == 0) {
			error("%s too long\n", err);
			exit(1);
		}
	} while (c != 0);
}

audit_write(val, message)
int val;
char *message;
{
#ifdef AUDIT
	audit_argv[5] = message;
	audit_text(AU_LOGIN, val, val, AUDITNUM, audit_argv);
#endif AUDIT
}
