/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)du:du.c	1.23"

/*
 * du -- summarize disk usage
 *	du [-ars] [name ...]
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char	path[PATH_MAX+1], name[PATH_MAX+1];
int	aflg;
int	rflg;
int	sflg;
char	*dot = ".";

#define ML	1000
struct {
	dev_t	dev;
	ino_t	ino;
} ml[ML];
int	mlx = 0;

long	descend();

main(argc, argv)
	int argc;
	char **argv;
{
	long blocks = 0;
	register c;
	extern int optind;
	register char *np;
	register pid_t pid, wpid;
	int status, retcode = 0;

	setbuf(stderr, NULL);
	while ((c = getopt(argc, argv, "ars")) != EOF)
		switch (c) {

		case 'a':
			aflg++;
			continue;

		case 'r':
			rflg++;
			continue;

		case 's':
			sflg++;
			continue;

		default:
			fprintf(stderr, "usage: du [-ars] [name ...]\n");
			exit(2);
		}
	if (optind == argc) {
		argv = &dot;
		argc = 1;
		optind = 0;
	}
	do {
		if (optind < argc - 1) {
			pid = fork();
			if (pid == (pid_t)-1) {
				perror("du: No more processes");
				exit(1);
			}
			if (pid != 0) {
				while ((wpid = wait(&status)) != pid
				    && wpid != (pid_t)-1)
					;
				if (pid != (pid_t)-1 && status != 0)
					retcode = 1;
			}
		}
		if (optind == argc - 1 || pid == 0) {
			(void) strcpy(path, argv[optind]);
			(void) strcpy(name, argv[optind]);
			if (np = strrchr(name, '/')) {
				*np++ = '\0';
				if (chdir(*name ? name : "/") < 0) {
					if (rflg) {
						fprintf(stderr, "du: ");
						perror(*name ? name : "/");
					}
					exit(1);
				}
			} else
				np = path;
			blocks = descend(path, *np ? np : ".", &retcode);
			if (sflg)
				printf("%ld\t%s\n", blocks, path);
			if (optind < argc - 1)
				exit(0);
		}
		optind++;
	} while (optind < argc);
	exit(retcode);
}

DIR	*dirp = NULL;

long
descend(base, name, retcode)
	char *base, *name;
	int  *retcode;
{
	char *ebase0, *ebase;
	struct stat stb;
	int i;
	long blocks = 0;
	long curoff = 0;
	register struct dirent *dp;

	ebase0 = ebase = strchr(base, 0);
	if (ebase > base && ebase[-1] == '/')
		ebase--;
	if (lstat(name, &stb) < 0) {
		if (rflg) {
			fprintf(stderr, "du: ");
			perror(base);
		}
		*retcode = 1; 
		*ebase0 = 0;
		return 0;
	}
	if (stb.st_nlink > 1 && (stb.st_mode & S_IFMT) != S_IFDIR) {
		for (i = 0; i <= mlx; i++)
			if (ml[i].ino == stb.st_ino && ml[i].dev == stb.st_dev)
				return 0;
		if (mlx < ML) {
			ml[mlx].dev = stb.st_dev;
			ml[mlx].ino = stb.st_ino;
			mlx++;
		}
	}
	blocks = stb.st_blocks;
	if ((stb.st_mode & S_IFMT) != S_IFDIR) {
		if (aflg)
			printf("%ld\t%s\n", blocks, base);
		return blocks;
	}
	if (dirp != NULL)
		(void) closedir(dirp);
	if ((dirp = opendir(name)) == NULL) {
		if (rflg) {
			fprintf(stderr, "du: ");
			perror(base);
		}
		*retcode = 1;
		*ebase0 = 0;
		return 0;
	}
	if (chdir(name) < 0) {
		if (rflg) {
			fprintf(stderr, "du: ");
			perror(base);
		}
		*retcode = 1;
		*ebase0 = 0;
		(void) closedir(dirp);
		dirp = NULL;
		return 0;
	}
	while (dp = readdir(dirp)) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		(void) sprintf(ebase, "/%s", dp->d_name);
		curoff = telldir(dirp);
		blocks += descend(base, ebase+1, retcode);
		*ebase = 0;
		if (dirp == NULL) {
			if ((dirp = opendir(".")) == NULL) {
				if (rflg) {
					fprintf(stderr,
					  "du: Can't reopen '.' in ");
					perror(base);
				}
				*retcode = 1;
				return 0;
			}
			seekdir(dirp, curoff);
		}
	}
	(void) closedir(dirp);
	dirp = NULL;
	if (sflg == 0)
		printf("%ld\t%s\n", blocks, base);
	if (chdir("..") < 0) {
		if (rflg) {
			(void) sprintf(strchr(base, '\0'), "/..");
			fprintf(stderr,
			  "du: Can't change directories to '..' in ");
			perror(base);
		}
		exit(1);
	}
	*ebase0 = 0;
	return blocks;
}

