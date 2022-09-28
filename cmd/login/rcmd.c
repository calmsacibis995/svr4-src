/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)login:rcmd.c	1.1.5.1"

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
#include <pwd.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/systeminfo.h>
#include <sys/param.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#ifdef SYSV
#define bcopy(s1, s2, len)	memcpy(s2, s1, len)
#define bzero(s, len)		memset(s, '\0', len)
#define index(s, c)		strchr(s, c)
char	*strchr();
#else
char	*index();
#endif SYSV

extern	errno;
char	*strcpy();
static char *domain;


ruserok(rhost, superuser, ruser, luser)
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
	int euid = -1;

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
			if (!_validuser(hostf, fhost, luser, ruser, baselen)) {
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
		if (euid != -1)
	    		(void) seteuid (euid);
	  	return(-1);
	}
	(void)fstat(fileno(hostf), &sbuf);
	if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid) {
	  	fclose(hostf);
		if (euid != -1)
		  	(void) seteuid (euid);
		return(-1);
	}

	if (!_validuser(hostf, fhost, luser, ruser, baselen)) {
	  	(void) fclose(hostf);
		if (euid != -1)
			(void) seteuid (euid);
		return(0);
	}

	(void) fclose(hostf);
	if (euid != -1)
       		(void) seteuid (euid);
	return (-1);
}

_validuser(hostf, rhost, luser, ruser, baselen)
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
			if (_checkhost(rhost, ahost+1, baselen))
				break;
		}
		else
			hostmatch = _checkhost(rhost, ahost, baselen);
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

_checkhost(rhost, lhost, len)
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
		if (sysinfo(SI_HOSTNAME, ldomain, MAXHOSTNAMELEN) == -1) {
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
