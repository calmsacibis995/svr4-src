/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/rsh.c	1.4.6.2"

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
#include <sys/ioctl.h>
#include <sys/file.h>
/* just for FIONBIO ... */
#include <sys/filio.h>

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <netdb.h>

#ifdef SYSV
#define	rindex		strrchr
#define	index		strchr
#define	bcopy(a,b,c)	memcpy((b),(a),(c))
#define	bzero(s,n)	memset((s), 0, (n))
#endif /* SYSV */

int	error();
char	*index(), *rindex(), *malloc(), *getpass(), *strcpy();
struct passwd	*getpwuid();

int	errno;
int	options;
int	rfd2;
int	sendsig();


#ifdef SYSV
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

#define	mask(s)	(1 << ((s) - 1))

/*
 * rsh - remote shell
 */
/* VARARGS */
main(argc, argv)
	int argc;
	char *argv[];
{
	pid_t pid;
	int rem;
	char *host, *cp, **ap, buf[BUFSIZ], *args, *user = 0;
	register int cc;
	int asrsh = 0;
	struct passwd *pwd;
	int readfrom, ready;
	int one = 1;
	struct servent *sp;
	int omask;
	int c;
	int ac;
	char **av;
	extern int optind;
	extern char *optarg;
	struct linger linger;

	linger.l_onoff = 1;
	linger.l_linger = 60;

	/*
	 * must be compatible with old-style command line parsing,
	 * which allowed hostname to come before arguments, so
	 *	rsh host -l user cmd
	 * instead of
	 *	rsh -l user host cmd
	 * is permissible, even tho' it doesn't fit the command
	 * syntax standard.
	 */

	host = rindex(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	if ( strcmp(host, "rsh") == 0 ) {
		if ( argc > 1 && argv[1][0] != '-' ) {
			ac = argc -1; av = &argv[1];
		} else {
			ac = argc; av = argv;
		}
	} else {
		ac = argc; av = argv;
	}
	while ( (c = getopt(ac, av, "l:ndLwe:8")) != -1 ) {
		switch (c) {
		case 'l':
			user = optarg;
			break;
		case 'n':
			(void) close(0);
			(void) open("/dev/null", 0);
			break;
		case 'd':
			options |= SO_DEBUG;
			break;
		/*
		 * Ignore the -L, -w, -e and -8 flags to allow aliases
		 * with rlogin to work
		 */
		case 'L': case 'w': case 'e': case '8':
			break;
		default:
			goto usage;
		}
	}
	if ( strcmp(host, "rsh") == 0 ) {
		if ( ac == argc ) {
			if ( optind >= argc )
				goto usage;
			host = argv[optind];
		} else {
			host = argv[1];
		}
		++optind;
		asrsh = 1;
	}
	
	if ( optind >= argc ) {
		if (asrsh)
			argv[0] = "rlogin";
		execv("/usr/bin/rlogin", argv);
		execv("/usr/ucb/rlogin", argv);
		fprintf(stderr, "No local rlogin program found\n");
		exit(1);
	}
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "who are you?\n");
		exit(1);
	}
	cc = 0;
	for (ap = &argv[optind]; *ap; ap++)
		cc += strlen(*ap) + 1;
	if ((cp = args = malloc(cc)) == (char *)NULL) {
		perror("malloc");
		exit(1);
	}
	memset(args, 0,  cc);
	for (ap = &argv[optind]; *ap; ap++) {
		(void) strcpy(cp, *ap);
		while (*cp)
			cp++;
		if (ap[1])
			*cp++ = ' ';
	}
	sp = getservbyname("shell", "tcp");
	if (sp == 0) {
		fprintf(stderr, "rsh: shell/tcp: unknown service\n");
		exit(1);
	}
        rem = rcmd(&host, sp->s_port, pwd->pw_name,
	    user ? user : pwd->pw_name, args, &rfd2);
        if (rem < 0)
                exit(1);
	if (rfd2 < 0) {
		fprintf(stderr, "rsh: can't establish stderr\n");
		exit(2);
	}
	if (options & SO_DEBUG) {
		if (setsockopt(rem, SOL_SOCKET, SO_DEBUG, &one, sizeof (one)) < 0)
			perror("rsh: setsockopt (stdin) (ignored)");
		if (setsockopt(rfd2, SOL_SOCKET, SO_DEBUG, &one, sizeof (one)) < 0)
			perror("rsh: setsockopt (stderr) (ignored)");
	}
	if (setsockopt(rem, SOL_SOCKET, SO_LINGER, (char *)&linger,
		sizeof(linger)) < 0)
			perror("rsh: SO_LINGER (stdin) (ignored)");
	if (setsockopt(rfd2, SOL_SOCKET, SO_LINGER, (char *)&linger,
		sizeof(linger)) < 0)
			perror("rsh: SO_LINGER (stderr) (ignored)");
	(void) setuid(getuid());
	omask = sigblock(mask(SIGINT)|mask(SIGQUIT)|mask(SIGTERM));
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, (void (*)())sendsig);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, (void (*)())sendsig);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, (void (*)())sendsig);
        pid = fork();
        if (pid < 0) {
		perror("rsh: fork");
                exit(1);
        }
	ioctl(rfd2, FIONBIO, &one);
	ioctl(rem, FIONBIO, &one);
        if (pid == 0) {
		char *bp; int rembits, wc;
		(void) close(rfd2);
	reread:
		errno = 0;
		cc = read(0, buf, sizeof buf);
		if (cc <= 0)
			goto done;
		bp = buf;
	rewrite:
		rembits = 1<<rem;
		if (select(sizeof(int)*8, 0, &rembits, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("rsh: select");
				exit(1);
			}
			goto rewrite;
		}
		if ((rembits & (1<<rem)) == 0)
			goto rewrite;
		wc = write(rem, bp, cc);
		if (wc < 0) {
			if (errno == EWOULDBLOCK)
				goto rewrite;
			goto done;
		}
		cc -= wc; bp += wc;
		if (cc == 0)
			goto reread;
		goto rewrite;
	done:
		(void) shutdown(rem, 1);
		exit(0);
	}
	sigsetmask(omask);
	readfrom = (1<<rfd2) | (1<<rem);
	do {
		ready = readfrom;
		if (select(sizeof(int)*8, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("rsh: select");
				exit(1);
			}
			continue;
		}
		if (ready & (1<<rfd2)) {
			errno = 0;
			cc = read(rfd2, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
					readfrom &= ~(1<<rfd2);
			} else
				(void) write(2, buf, cc);
		}
		if (ready & (1<<rem)) {
			errno = 0;
			cc = read(rem, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
					readfrom &= ~(1<<rem);
			} else
				(void) write(1, buf, cc);
		}
        } while (readfrom);
        (void) kill(pid, SIGKILL);
	exit(0);
usage:
	fprintf(stderr,
	    "usage: rsh [ -l login ] [ -n ] host command\n");
	exit(1);
	/* NOTREACHED */
}

sendsig(signo)
	char signo;
{

	(void) write(rfd2, &signo, 1);
}
