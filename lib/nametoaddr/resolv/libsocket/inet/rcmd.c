/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nametoaddr:resolv/libsocket/inet/rcmd.c	1.1.2.1"

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

#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include "../../resolvabi.h"

#ifdef SYSV
#define bcopy(s1, s2, len)	memcpy(s2, s1, len)
#define bzero(s, len)		memset(s, 0, len)
#define index(s, c)		strchr(s, c)
char	*strchr();
#else
char	*index();
#endif SYSV

extern	errno;
char	*strcpy();
static char *domain;

_rs_rcmd(ahost, rport, locuser, remuser, cmd, fd2p)
	char **ahost;
	u_short rport;
	char *locuser, *remuser, *cmd;
	int *fd2p;
{
	int s, timo = 1, retval;
	pid_t pid;
	struct sockaddr_in sin, from;
	char c;
	int lport = IPPORT_RESERVED - 1;
	struct hostent *hp;
#ifdef SYSV
	void	(*spipe)();
	sigset_t oldmask;
	sigset_t newmask;
#else
	int oldmask;
#endif SYSV


	pid = getpid();
	hp = _rs_gethostbyname(*ahost);
	if (hp == 0) {
		fprintf(stderr, "%s: unknown host\n", *ahost);
		return (-1);
	}
	*ahost = hp->h_name;
#ifdef SYSV
	spipe = sigset(SIGPIPE, SIG_IGN);
	bzero((char *) &newmask, sizeof (newmask));
	(void) sigaddset(&newmask, SIGURG);
	(void) sigprocmask(SIG_BLOCK, &newmask, &oldmask);
#else
	oldmask = sigblock(sigmask(SIGURG));
#endif SYSV
	for (;;) {
		s = _rs_rresvport(&lport);
		if (s < 0) {
			if (errno == EAGAIN)
				fprintf(stderr, "socket: All ports in use\n");
			else
				perror("rcmd: socket");
#ifdef SYSV
			(void) sigset(SIGPIPE, spipe);
			(void) sigprocmask(SIG_SETMASK, &oldmask, 
					   (sigset_t *) 0);
#else
			sigsetmask(oldmask);
#endif SYSV
			return (-1);
		}
		_rs_fcntl(s, F_SETOWN, pid);
		sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr, hp->h_length);
		sin.sin_port = rport;
		if (_rs_connect(s, &sin, sizeof (sin)) >= 0)
			break;
		(void) close(s);
		if (errno == EADDRINUSE) {
			lport--;
			continue;
		}
		if (errno == ECONNREFUSED && timo <= 16) {
			sleep(timo);
			timo *= 2;
			continue;
		}
		if (hp->h_addr_list[1] != NULL) {
			int oerrno = errno;

			fprintf(stderr,
			    "connect to address %s: ", _rs_inet_ntoa(sin.sin_addr));
			errno = oerrno;
			perror(0);
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr,
			    hp->h_length);
			fprintf(stderr, "Trying %s...\n",
				_rs_inet_ntoa(sin.sin_addr));
			continue;
		}
		perror(hp->h_name);
#ifdef SYSV
		(void) sigset(SIGPIPE, spipe);
		(void) sigprocmask(SIG_SETMASK, &oldmask, (sigset_t *) 0);
#else
		sigsetmask(oldmask);
#endif SYSV
		return (-1);
	}
	lport--;
	if (fd2p == 0) {
		write(s, "", 1);
		lport = 0;
	} else {
		char num[8];
		int s2 = _rs_rresvport(&lport), s3;
		int len = sizeof (from);

		if (s2 < 0)
			goto bad;
		_rs_listen(s2, 1);
		(void) sprintf(num, "%d", lport);
		if (write(s, num, strlen(num)+1) != strlen(num)+1) {
			perror("write: setting up stderr");
			(void) close(s2);
			goto bad;
		}
		s3 = _rs_accept(s2, &from, &len);
		(void) close(s2);
		if (s3 < 0) {
			perror("accept");
			lport = 0;
			goto bad;
		}
		*fd2p = s3;
		from.sin_port = _rs_ntohs((u_short)from.sin_port);
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED) {
			fprintf(stderr,
			    "socket: protocol failure in circuit setup.\n");
			goto bad2;
		}
	}
	(void) write(s, locuser, strlen(locuser)+1);
	(void) write(s, remuser, strlen(remuser)+1);
	(void) write(s, cmd, strlen(cmd)+1);
	retval = read(s, &c, 1);
	if (retval != 1) {
		if (retval == 0) {
		    fprintf(stderr,
		      "Protocol error, %s closed connection\n", *ahost);
		} else if (retval < 0) {
		    perror(*ahost);
		} else {
		    fprintf(stderr,
		      "Protocol error, %s sent %d bytes\n", *ahost, retval);
		}
		goto bad2;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
#ifdef SYSV
	(void) sigset(SIGPIPE, spipe);
	(void) sigprocmask(SIG_SETMASK, &oldmask, (sigset_t *) 0);
#else
	sigsetmask(oldmask);
#endif SYSV
	return (s);
bad2:
	if (lport)
		(void) close(*fd2p);
bad:
	(void) close(s);
#ifdef SYSV
	(void) sigset(SIGPIPE, spipe);
	(void) sigprocmask(SIG_SETMASK, &oldmask, (sigset_t *) 0);
#else
	sigsetmask(oldmask);
#endif SYSV
	return (-1);
}

_rs_rresvport(alport)
	int *alport;
{
	struct sockaddr_in sin;
	int s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	s = _rs_socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (-1);
	for (;;) {
		sin.sin_port = _rs_htons((u_short)*alport);
		if (_rs_bind(s, (caddr_t)&sin, sizeof (sin)) >= 0)
			return (s);
		if (errno != EADDRINUSE) {
			(void) close(s);
			return (-1);
		}
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			(void) close(s);
			errno = EAGAIN;		/* close */
			return (-1);
		}
	}
}

_rs_ruserok(rhost, superuser, ruser, luser)
	char *rhost;
	int superuser;
	char *ruser, *luser;
{
	FILE *hostf;
	char fhost[MAXHOSTNAMELEN];
	register char *sp, *p;
	int baselen = -1;

	struct stat sbuf;
	struct passwd *pwd;
	char pbuf[MAXPATHLEN];
	uid_t euid = (uid_t)-1;

	sp = rhost;
	p = fhost;
	while (*sp) {
		if (*sp == '.') {
			if (baselen == -1)
				baselen = sp - rhost;
			*p++ = *sp++;
		} else {
			*p++ = isupper(*sp) ? tolower(*sp++) : *sp++;
		}
	}
	*p = '\0';

	/* check /etc/hosts.equiv */
	if (!superuser) {
		if ((hostf = fopen("/etc/hosts.equiv", "r")) != NULL) {
			if (!_rs__validuser(hostf, fhost, luser, ruser, baselen)) {
				(void) fclose(hostf);
				return(0);
		        }
			(void) fclose(hostf);
		}
	}

	/* check ~/.rhosts */

	if ((pwd = getpwnam(luser)) == NULL)
       		return(-1);
	(void)strcpy(pbuf, pwd->pw_dir);
	(void)strcat(pbuf, "/.rhosts");

	/* 
	 * Read .rhosts as the local user to avoid NFS mapping the root uid
	 * to something that can't read .rhosts.
	 */
	euid = geteuid();
	(void) seteuid (pwd->pw_uid);
	if ((hostf = fopen(pbuf, "r")) == NULL) {
		if (euid != (uid_t)-1)
	    		(void) seteuid (euid);
	  	return(-1);
	}
	(void)fstat(fileno(hostf), &sbuf);
	if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid) {
	  	fclose(hostf);
		if (euid != (uid_t)-1)
		  	(void) seteuid (euid);
		return(-1);
	}

	if (!_rs__validuser(hostf, fhost, luser, ruser, baselen)) {
	  	(void) fclose(hostf);
		if (euid != (uid_t)-1)
			(void) seteuid (euid);
		return(0);
	}

	(void) fclose(hostf);
	if (euid != (uid_t)-1)
       		(void) seteuid (euid);
	return (-1);
}

_rs__validuser(hostf, rhost, luser, ruser, baselen)
char *rhost, *luser, *ruser;
FILE *hostf;
int baselen;
{
	char *user;
	char ahost[MAXHOSTNAMELEN];
	int hostmatch, usermatch;
	register char *p;

#ifdef YP
	if (domain == NULL) {
                (void) usingypmap(&domain, NULL);
        }
#endif YP

	while (fgets(ahost, sizeof (ahost), hostf)) {
		p = ahost;
		while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0') {
			*p = isupper(*p) ? tolower(*p) : *p;
			p++;
		}
		if (*p == ' ' || *p == '\t') {
			*p++ = '\0';
			while (*p == ' ' || *p == '\t')
				p++;
			user = p;
			while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0')
				p++;
		} else
			user = p;
		*p = '\0';
		if (ahost[0] == '+' && ahost[1] == 0)
			hostmatch = 1;
#ifdef YP
		else if (ahost[0] == '+' && ahost[1] == '@')
			hostmatch = innetgr(ahost + 2, rhost,
			    NULL, domain);
		else if (ahost[0] == '-' && ahost[1] == '@') {
			if (innetgr(ahost + 2, rhost, NULL, domain))
				break;
		}
#endif YP
		else if (ahost[0] == '-') {
			if (_rs__checkhost(rhost, ahost+1, baselen))
				break;
		}
		else
			hostmatch = _rs__checkhost(rhost, ahost, baselen);
		if (user[0]) {
			if (user[0] == '+' && user[1] == 0)
				usermatch = 1;
#ifdef YP
			else if (user[0] == '+' && user[1] == '@')
				usermatch = innetgr(user+2, NULL,
				    ruser, domain);
			else if (user[0] == '-' && user[1] == '@') {
				if (hostmatch && innetgr(user+2, NULL,
				    ruser, domain))
					break;
			}
#endif YP
			else if (user[0] == '-') {
				if (hostmatch && !strcmp(user+1, ruser))
					break;
			}
			else
				usermatch = !strcmp(user, ruser);
		}
		else
			usermatch = !strcmp(ruser, luser);
		if (hostmatch && usermatch)
			return (0);
	}
	return (-1);
}

_rs__checkhost(rhost, lhost, len)
char *rhost, *lhost;
int len;
{
	static char *ldomain;
	static char *domainp;
	static int nodomain;
	register char *cp;

	if (ldomain == NULL) {
		ldomain = (char *)malloc(MAXHOSTNAMELEN+1);
		if (ldomain == 0)
			return (0);
	}

	if (len == -1)
		return(!strcmp(rhost, lhost));
	if (strncmp(rhost, lhost, len))
		return(0);
	if (!strcmp(rhost, lhost))
		return(1);
	if (*(lhost + len) != '\0')
		return(0);
	if (nodomain)
		return(0);
	if (!domainp) {
		/*
		 * "domainp" points after the first dot in the host name
		 */
		if (gethostname(ldomain, MAXHOSTNAMELEN) == -1) {
			nodomain = 1;
			return(0);
		}
		ldomain[MAXHOSTNAMELEN] = NULL;
		if ((domainp = index(ldomain, '.')) == (char *)NULL) {
			nodomain = 1;
			return(0);
		}
		domainp++;
		cp = domainp;
		while (*cp) {
			*cp = isupper(*cp) ? tolower(*cp) : *cp;
			cp++;
		}
	}
	return(!strcmp(domainp, rhost + len +1));
}
