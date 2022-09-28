/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.comsat.c	1.7.2.1"

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


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <sys/ttold.h>
#include <utmp.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <pwd.h>

/*
 * comsat
 */
int	debug = 0;
#define	dprintf	if (debug) printf

struct	sockaddr_in sin = { AF_INET };
extern	errno;

char	hostname[MAXHOSTNAMELEN];
struct	utmp *utmp = NULL;
int	nutmp;
int	uf;
unsigned utmpmtime = 0;			/* last modification time for utmp */
unsigned utmpsize = 0;			/* last malloced size for utmp */
int	onalrm();
long	lastmsgtime;
char 	*malloc(), *realloc();

#ifndef SYSV
int	reapchildren();

#else

#define rindex strrchr
#define index strchr

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

#endif /* SYSV */


#define	MAXIDLE	120
#define NAMLEN (sizeof (uts[0].ut_name) + 1)

main(argc, argv)
	int argc;
	char *argv[];
{
	register int cc;
	char buf[BUFSIZ];
	char msgbuf[100];
	struct sockaddr_in from;
	int fromlen;

	/* verify proper invocation */
	fromlen = sizeof (from);
	if (getsockname(0, &from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getsockname");
		_exit(1);
	}
#ifdef SYSV
	chdir("/var/mail");
#else
	chdir("/var/spool/mail");
#endif /* SYSV */
	if ((uf = open(UTMP_FILE,0)) < 0) {
		openlog("comsat", 0, LOG_DAEMON);
		syslog(LOG_ERR, "%s: %m", UTMP_FILE);
		(void) recv(0, msgbuf, sizeof (msgbuf) - 1, 0);
		exit(1);
	}
	lastmsgtime = time(0);
	gethostname(hostname, sizeof (hostname));
	onalrm();
	signal(SIGALRM, onalrm);
	signal(SIGTTOU, SIG_IGN);
#ifndef SYSV
	signal(SIGCHLD, reapchildren);
#else
	signal(SIGCHLD, SIG_IGN); /*no zombies*/
#endif /* SYSV */
	for (;;) {
		cc = recv(0, msgbuf, sizeof (msgbuf) - 1, 0);
		if (cc <= 0) {
			if (errno != EINTR)
				sleep(1);
			errno = 0;
			continue;
		}
		if (nutmp == 0)			/* no users (yet) */
			continue;
		sigblock(sigmask(SIGALRM));
		msgbuf[cc] = 0;
		lastmsgtime = time(0);
		mailfor(msgbuf);
		sigsetmask(0);
	}
}

#ifndef SYSV
reapchildren()
{

	while (wait3((struct wait *)0, WNOHANG, (struct rusage *)0) > 0)
		;
}
#endif /* SYSV */

onalrm()
{
	struct stat statbf;

	if (time(0) - lastmsgtime >= MAXIDLE)
		exit(0);
	dprintf("alarm\n");
	alarm(15);
	fstat(uf, &statbf);
	if (statbf.st_mtime > utmpmtime) {
		dprintf(" changed\n");
		utmpmtime = statbf.st_mtime;
		if (statbf.st_size > utmpsize) {
			utmpsize = statbf.st_size + 10 * sizeof(struct utmp);
			if (utmp)
				utmp = (struct utmp *)realloc(utmp, utmpsize);
			else
				utmp = (struct utmp *)malloc(utmpsize);
			if (! utmp) {
				dprintf("malloc failed\n");
				exit(1);
			}
		}
		lseek(uf, 0, 0);
		nutmp = read(uf,utmp,statbf.st_size)/sizeof(struct utmp);
	} else
		dprintf(" ok\n");
}

mailfor(name)
	char *name;
{
	register struct utmp *utp = &utmp[nutmp];
	register char *cp;
	char *rindex();
	int offset;

	  /*
	   * Don't bother doing anything if nobody has every
	   * logged into the system.
	   */
	if (utmp == NULL || nutmp == 0)
		return;
	dprintf("mailfor %s\n", name);
	cp = name;
	while (*cp && *cp != '@')
		cp++;
	if (*cp == 0) {
		dprintf("bad format\n");
		return;
	}
	*cp = 0;
	offset = atoi(cp+1);
	while (--utp >= utmp)
		if (!strncmp(utp->ut_name, name, sizeof(utmp[0].ut_name)))
			notify(utp, offset);
}

char	*cr;

notify(utp, offset)
	register struct utmp *utp;
{
	FILE *tp;
	struct sgttyb gttybuf;
	char tty[20], name[sizeof (utmp[0].ut_name) + 1];
	struct stat stb;
	time_t timep[2];
	struct passwd *pwd;

	strcpy(tty, "/dev/");
	strncat(tty, utp->ut_line, sizeof(utp->ut_line));
	dprintf("notify %s on %s\n", utp->ut_name, tty);
	if (stat(tty, &stb) == 0 && (stb.st_mode & 0100) == 0) {
		dprintf("wrong mode\n");
		return;
	}
	if (fork())
		return;
	signal(SIGALRM, SIG_DFL);
	alarm(30);
	/*
	 * Do all operations that check protections as the user who
	 * will be getting the biff.
	 */
	if ((pwd = getpwnam(utp->ut_name)) == (struct passwd *) -1) {
		dprintf("getpwnam failed\n");
		exit(1);
	}
	if (setuid(pwd->pw_uid) == -1) {
		dprintf("setuid failed\n");
		exit(1);
	}
	if ((tp = fopen(tty,"w")) == 0) {
		dprintf("fopen failed\n");
		exit(-1);
	}
	ioctl(fileno(tp), TIOCGETP, &gttybuf);
	cr = (gttybuf.sg_flags&CRMOD) && !(gttybuf.sg_flags&RAW) ? "" : "\r";
	strncpy(name, utp->ut_name, sizeof (utp->ut_name));
	name[sizeof (name) - 1] = '\0';
	fprintf(tp,"%s\n\007New mail for %s@%.*s\007 has arrived:%s\n",
	    cr, name, sizeof (hostname), hostname, cr);
	fprintf(tp,"----%s\n", cr);
	stat(name, &stb);
	timep[0] = stb.st_atime;
	timep[1] = stb.st_mtime;
	jkfprintf(tp, name, offset);
	utime(name, timep);
	exit(0);
}

jkfprintf(tp, name, offset)
	register FILE *tp;
{
	register FILE *fi;
	register int linecnt, charcnt;
	char line[BUFSIZ];
	int inheader;

	dprintf("HERE %s's mail starting at %d\n",
	    name, offset);
	if ((fi = fopen(name,"r")) == NULL) {
		dprintf("Cant read the mail\n");
		return;
	}

	fseek(fi, offset, L_SET);

	/* 
	 * Print the first 7 lines or 560 characters of the new mail
	 * (whichever comes first).  Skip header crap other than
	 * From, Subject, To, and Date.
	 */
	linecnt = 7;
	charcnt = 560;
	inheader = 1;
	while (fgets(line, sizeof (line), fi) != NULL) {
		register char *cp;
		char *index();
		int cnt;

		if (linecnt <= 0 || charcnt <= 0) {  
			fprintf(tp,"...more...%s\n", cr);
			return;
		}
		if (strncmp(line, "From ", 5) == 0)
			continue;
		if (inheader && (line[0] == ' ' || line[0] == '\t'))
			continue;
		cp = index(line, ':');
		if (cp == 0 || (index(line, ' ') && index(line, ' ') < cp))
			inheader = 0;
		else
			cnt = cp - line;
		if (inheader &&
		    strncmp(line, "Date", cnt) &&
		    strncmp(line, "From", cnt) &&
		    strncmp(line, "Subject", cnt) &&
		    strncmp(line, "To", cnt))
			continue;
		cp = index(line, '\n');
		if (cp)
			*cp = '\0';
		fprintf(tp,"%s%s\n", line, cr);
		linecnt--, charcnt -= strlen(line);
	}
	fprintf(tp,"----%s\n", cr);
}

