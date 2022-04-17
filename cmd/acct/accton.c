/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:accton.c	1.11.2.2"

/*
 *	accton - calls syscall with super-user privileges
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include        "acctdef.h"
#include	<errno.h>
#include	<sys/stat.h>
#include        <pwd.h>

uid_t     admuid;
struct  passwd *pwd, *getpwnam(); 

main(argc,argv)
int argc;
char **argv;
{
	register uid_t	uid;

	uid = getuid();
	if ((pwd = getpwnam("adm")) == NULL) 
		perror("cannot determine adm's uid"), exit(1);
	admuid = pwd->pw_uid;
	if(uid == ROOT || uid == admuid) {
		if(setuid(ROOT) == ERR) 
			perror("cannot setuid (check command mode and owner)"), exit(1);
		if (argv[1])
			ckfile(argv[1]);
		if (acct(argc > 1 ? argv[1] : 0) < 0)
			perror(argv[1]), exit(1);
		exit(0);

	}
	fprintf(stderr,"%s: permission denied\n", argv[0]);
	exit(1);
}

ckfile(admfile)
register char	*admfile;
{
	struct stat		stbuf;
	register struct stat	*s = &stbuf;

	if(stat(admfile, s) == ERR)
		if(creat(admfile, S_IRUSR|S_IWUSR|S_IRGRP|S_IWUSR|S_IROTH) == ERR) 
			perror("creat"), exit(1);

	if(s->st_uid != admuid || s->st_gid != (gid_t)admuid)
		if(chown(admfile, admuid, (gid_t)admuid) == ERR) 
			perror("cannot change owner"), exit(1);

	/* was if(s->st_mode & 0777 != 0664) */
	if((s->st_mode & S_IAMB) != S_IRUSR|S_IWUSR|S_IRGRP|S_IWUSR|S_IROTH)
		if(chmod(admfile, S_IRUSR|S_IWUSR|S_IRGRP|S_IWUSR|S_IROTH) == ERR) 
			perror("cannot chmod"), exit(1);
}
