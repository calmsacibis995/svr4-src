/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)newgrp:newgrp.c	1.11"
/*
 * newgrp [group]
 *
 * rules
 *	if no arg, group id in password file is used
 *	else if group id == id in password file
 *	else if login name is in member list
 *	else if password is present and user knows it
 *	else too bad
 */
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <crypt.h>
#include <string.h>
#include <stdlib.h>

#define	SHELL	"/usr/bin/sh"

#define PATH	"PATH=:/usr/bin:/bin"
#define SUPATH	"PATH=:/usr/bin:/sbin:/usr/sbin:/etc:/bin"
#define ELIM	128

char	PW[] = "newgrp: Password";
char	NG[] = "newgrp: Sorry";
char	PD[] = "newgrp: Permission denied";
char	UG[] = "newgrp: Unknown group";
char	NS[] = "newgrp: You have no shell";

char homedir[64]="HOME=";
char logname[20]="LOGNAME=";

char *envinit[ELIM];
extern char **environ;
char *path=PATH;
char *supath=SUPATH;


main(argc,argv)
char *argv[];
{
	register char *s;
	register struct passwd *p;
	char *rname();
	gid_t chkgrp();
	int eflag = 0;
	uid_t uid;
	char *shell, *dir, *name;

#ifdef	DEBUG
	chroot(".");
#endif
	if ((p = getpwuid(getuid())) == NULL) 
		error(NG);
	endpwent();
	if(argc > 1 && *argv[1] == '-'){
		eflag++;
		argv++;
		--argc;
	}
	if (argc > 1)
		p->pw_gid = chkgrp(argv[1], p);

	uid = p->pw_uid;
	dir = strcpy((char *)malloc(strlen(p->pw_dir)+1),p->pw_dir);
	name = strcpy((char *)malloc(strlen(p->pw_name)+1),p->pw_name);
	if (setgid(p->pw_gid) < 0 || setuid(getuid()) < 0)
		error(NG);
	if (!*p->pw_shell) {
		if ((shell = getenv("SHELL")) != NULL) {
			p->pw_shell = shell;
		} else {
			p->pw_shell = SHELL;
		}
	}
	if(eflag){
		char *simple;

		strcat(homedir, dir);
		strcat(logname, name);
		envinit[2] = logname;
		chdir(dir);
		envinit[0] = homedir;
		if (uid == 0)
			envinit[1] = supath;
		else
			envinit[1] = path;
		envinit[3] = NULL;
		environ = envinit;
		shell = strcpy((char *)malloc(strlen(p->pw_shell) + 2), "-");
		shell = strcat(shell,p->pw_shell);
		simple = strrchr(shell,'/');
		if(simple){
			*(shell+1) = '\0';
			shell = strcat(shell,++simple);
		}
	}
	else	shell = p->pw_shell;
	execl(p->pw_shell,shell, NULL);
	error(NS);
}

warn(s)
char *s;
{
	fprintf(stderr, "%s\n", s);
}

error(s)
char *s;
{
	warn(s);
	exit(1);
}

gid_t
chkgrp(gname, p)
char	*gname;
struct	passwd *p;
{
	register char **t;
	register struct group *g;

	g = getgrnam(gname);
	endgrent();
	if (g == NULL) {
		warn(UG);
		return getgid();
	}
	if (p->pw_gid == g->gr_gid || getuid() == 0)
		return g->gr_gid;
	for (t = g->gr_mem; *t; ++t) {
		if (strcmp(p->pw_name, *t) == 0)
			return g->gr_gid;
	}
	if (*g->gr_passwd) {
		if (!isatty(fileno(stdin)))
			error(PD);
		if (strcmp(g->gr_passwd, crypt(getpass(PW), g->gr_passwd)) == 0)
			return g->gr_gid;
	}
	warn(NG);
	return getgid();
}
/*
 * return pointer to rightmost component of pathname
 */
char *rname(pn)
char *pn;
{
	register char *q;

	q = pn;
	while (*pn)
		if (*pn++ == '/')
			q = pn;
	return q;
}
