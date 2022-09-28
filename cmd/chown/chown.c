/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)chown:chown.c	1.13"

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

/*
 * chown [-hR] uid file ...
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

struct	passwd	*pwd;
struct	stat	stbuf;
uid_t	uid;
int	status;
int hflag, rflag = 0;

main(argc, argv)
char *argv[];
{
	register c;
	ushort atoushort() ;
	int ch;
	extern int optind;
	int errflg = 0;

	while((ch = getopt(argc, argv, "hR")) != EOF)
	switch(ch) {
		case 'h' :
			hflag++;
			break;
		case 'R' :
			rflag++;
			break;
		default :
			errflg++;
			break;
	}

        /*
         * Check for sufficient arguments
         * or a usage error.
         */

        argc -= optind;
        argv = &argv[optind];

        if(errflg || argc < 2) {
                fprintf(stderr, "usage: chown [-h] [-R] uid file ...\n");
                exit(4);
        }

	if(isnumber(argv[0])) {
		uid = (uid_t)atoushort(argv[0]); /* uid is unsigned short */
		goto cho;
	}
	if((pwd=getpwnam(argv[0])) == NULL) {
		fprintf(stderr, "chown: unknown user id %s\n",argv[0]);
		exit(4);
	}
	uid = pwd->pw_uid;

cho:
	for(c=1; c<argc; c++) {
		if (lstat(argv[c], &stbuf) < 0) {
			status += Perror(argv[c]);
			continue;
		}
		if (rflag & ((stbuf.st_mode & S_IFMT) == S_IFLNK)) {
			if (!hflag) {
				if (stat(argv[c], &stbuf) < 0) {
					status += Perror(argv[c]);
					continue;
				}
				if ((stbuf.st_mode &S_IFMT) == S_IFDIR)
					status += chownr(argv[c], uid, -1);
				else
					if(chown(argv[c], uid, -1) < 0)
						status = Perror(argv[c]);
			}
			else
				if(lchown(argv[c], uid, -1) < 0)
					status = Perror(argv[c]);
		}
		else if (rflag && ((stbuf.st_mode&S_IFMT) == S_IFDIR)) {
                        status += chownr(argv[c], uid, -1);
		}
		else if (hflag) {
			if(lchown(argv[c], uid, -1) < 0) {
				status = Perror(argv[c]);
			}
		}
		else {
			if(chown(argv[c], uid, -1) < 0) {
				status = Perror(argv[c]);
			}
		}
	}
	exit(status);
}

chownr(dir, uid, gid)
char *dir;
uid_t uid;
gid_t gid;
{
        register DIR *dirp;
        register struct dirent *dp;
        struct stat st;
        char savedir[1024];
        int ecode = 0;
        extern char *getcwd();

        if (getcwd(savedir, 1024) == (char *)0) {
                Perror("getcwd");
                exit(255);
        }
        if (chdir(dir) < 0)
                return(Perror(dir));
        if ((dirp = opendir(".")) == NULL)
                return(Perror(dir));
        dp = readdir(dirp);
        dp = readdir(dirp); /* read "." and ".." */
        for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (lstat(dp->d_name, &st) < 0) {
			status += Perror(dp->d_name);
			continue;
		}
		if (rflag & ((st.st_mode & S_IFMT) == S_IFLNK)) {
			if (!hflag) {
				if (stat(dp->d_name, &st) < 0) {
					status += Perror(dp->d_name);
					continue;
				}
				if ((st.st_mode &S_IFMT) == S_IFDIR)
					status += chownr(dp->d_name, uid, -1);
				else
					if(chown(dp->d_name, uid, -1) < 0)
						status = Perror(dp->d_name);
			}
			else
				if(lchown(dp->d_name, uid, -1) < 0)
					status = Perror(dp->d_name);
		}
		else if (rflag && ((st.st_mode&S_IFMT) == S_IFDIR)) {
                        status += chownr(dp->d_name, uid, -1);
		}
		else if (hflag) {
			if(lchown(dp->d_name, uid, -1) < 0) {
				status = Perror(dp->d_name);
			}
		}
		else {
			if(chown(dp->d_name, uid, -1) < 0) {
				status = Perror(dp->d_name);
			}
		}
        }
        closedir(dirp);
        if (chdir(savedir) < 0) {
                fprintf(stderr, "chown: can't change back to %s", savedir);
                exit(255);
        }

        /*
         * Change what we are given after doing it's contents.
         */
        if (chown(dir, uid, -1) < 0)
                return(Perror(dir));

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

ushort
atoushort(s)
register char *s;
{
	register char c;
	register int i = 0 ;
	register ushort maxushort = ((ushort) ~0) ;

	while(c = *s++) {
		i = c - '0' + 10 * i;
		if(i > (int)maxushort) {
			fprintf(stderr,"chown: numeric user id too large\n");
			exit(4);
   		}
	}
	return (ushort) i ;
}

Perror(s)
char *s;
{
        fprintf(stderr,"chown: ");
        perror(s);
        return(1);
}
