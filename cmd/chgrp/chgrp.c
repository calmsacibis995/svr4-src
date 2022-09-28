/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Portions Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/

#ident	"@(#)chgrp:chgrp.c	1.10"

/*
 * chgrp  [-h] [-R] gid file ...
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <dirent.h>
#include <sys/dir.h>
#include <unistd.h>

struct	group	*gr;
struct	stat	stbuf;
gid_t	gid;
int	hflag = 0;
int	status;
int 	acode = 0;
extern  int optind;

main(argc, argv)
int  argc;
char *argv[];
{
	register c;
	int rflag = 0;

	while ((c = getopt(argc, argv, "hR")) != EOF)
		switch(c) {
			case 'R':
				rflag++;
				break;
			case 'h':
				hflag++;
				break;
			default:
				fprintf(stderr,"usage: chgrp [-h] [-R] gid file...\n");
				exit(4);
		}

	/*
	 * Check for sufficient arguments
	 * or a usage error.
	 */
	argc -= optind;
	argv = &argv[optind];
	if (argc < 2) {
		fprintf(stderr,"chgrp: usage: chgrp [-h] [-R] gid file...\n");
		exit(4);
	}
	if(isnumber(argv[0])) {
		gid = (gid_t)atoi(argv[0]); /* gid is an int */
	} else {
		if((gr=getgrnam(argv[0])) == NULL) {
			fprintf(stderr,"chgrp: unknown group: %s\n",argv[0]);
			exit(4);
		}
		gid = gr->gr_gid;
	}

	for(c=1; c<argc; c++) {
		/* do stat for directory arguments */
		if (hflag) {
			if (lstat(argv[c], &stbuf) < 0) {
				status = Perror(argv[c]);
				continue;
			}
		} else {
			if (stat(argv[c], &stbuf) < 0) {
				status = Perror(argv[c]);
				continue;
			}
		}
		if (rflag && ((stbuf.st_mode & S_IFMT) == S_IFDIR)) {
			status += chownr(argv[c], stbuf.st_uid, gid);
			continue;
		}
		if (hflag) {
			if (lchown(argv[c], stbuf.st_uid, gid) < 0) {
				status = Perror(argv[c]);
			}
		} else {
			if (chown(argv[c], stbuf.st_uid, gid) < 0) {
				status = Perror(argv[c]);
			}
		}
	}
	exit(status += acode);
}


chownr(dir, uid, gid)
	char *dir;
	uid_t uid;
	gid_t gid;
{
	register struct dirent *dp;
	register DIR *dirp;
	struct stat st;
	char savedir[1024];
	int ecode = 0;

	if (getcwd(savedir,1024) == 0) {
		fprintf(stderr,"chgrp: %s\n", savedir);
		exit(255);
	}

	/*
	 * Change what we are given before doing its contents.
	 */
	if (hflag) {
		if (lchown(dir, uid, gid) < 0 && Perror(dir))
			return (1);
	} else {
		if (chown(dir, uid, gid) < 0 && Perror(dir))
			return (1);
	}
	
	if (chdir(dir) < 0) 
		return(Perror(dir));
	if ((dirp = opendir(".")) == NULL) 
		return(Perror(dir));
	dp = readdir(dirp);
	dp = readdir(dirp); /* read "." and ".." */
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (hflag) {
			if (lstat(dp->d_name, &st) < 0) {
				ecode += Perror(dp->d_name);
				break;
			}
		} else {
			if (stat(dp->d_name, &st) < 0) {
				ecode += Perror(dp->d_name);
				break;
			}
		}

		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			acode += chownr(dp->d_name, st.st_uid, gid);
			continue;
		}
		if (hflag) {
			if (lchown(dp->d_name, stbuf.st_uid, gid) < 0) {
				acode += Perror(dp->d_name);
			}
		} else {
			if (chown(dp->d_name, stbuf.st_uid, gid) < 0) {
				acode += Perror(dp->d_name);
			}
		}
	}
	closedir(dirp);
	if (chdir(savedir) < 0) {
		fprintf(stderr,"chgrp: can't change back to %s\n", savedir);
		exit(255);
	}
	return (ecode);
}


isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}	

Perror(s)
	char *s;
{
	fprintf(stderr,"chgrp: ");
	perror(s);
	return(1);
}
