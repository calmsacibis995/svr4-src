/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbdu:du.c	1.2.1.1"

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
 * du
 */
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <dirent.h>


char	path[BUFSIZ], name[BUFSIZ];
int	aflg;
int	sflg;
char	*dot = ".";

#define ML	1000
struct {
	int	dev;
	ino_t	ino;
} ml[ML];
int	mlx;

long	descend();
char	*strchr(), *strrchr(), *strcpy();

#define DEV_BSHIFT	9 	/* log2(512)  512 being the block size */
#define dbtob(db)		/* calculate (db * DEV_BSIZE) */   \
	((unsigned)(db) << DEV_BSHIFT)

#define	kb(n)	(howmany(dbtob(n), 1024))


main(argc, argv)
	int argc;
	char **argv;
{
	long blocks = 0;
	register char *np;
	int pid, wpid;
	int status, retcode=0;

	argc--, argv++;
again:
	if (argc && !strcmp(*argv, "-s")) {
		sflg++;
		argc--, argv++;
		goto again;
	}
	if (argc && !strcmp(*argv, "-a")) {
		aflg++;
		argc--, argv++;
		goto again;
	}
	if (argc == 0) {
		argv = &dot;
		argc = 1;
	}
	do {
		if (argc > 1) {
			pid = fork();
			if (pid == -1) {
				fprintf(stderr, "No more processes.\n");
				exit(1);
			}
			if (pid != 0) {
                                while ((wpid = wait(&status)) != pid
                                    && wpid != (pid_t)-1)
                                        ;
                                if (pid != (pid_t)-1) {
                                        if (status != 0)
                                                retcode = 1;
                                }
                        }
		}
		if (argc == 1 || pid == 0) {
			(void) strcpy(path, *argv);
			(void) strcpy(name, *argv);
			if (np = strrchr(name, '/')) {
				*np++ = '\0';
				if (chdir(*name ? name : "/") < 0) {
					perror(*name ? name : "/");
					exit(1);
				}
			} else
				np = path;
			blocks = descend(path, *np ? np : ".", &retcode);
			if (sflg)
				printf("%ld\t%s\n", kb(blocks), path);
			if (argc > 1)
				exit(1);
		}
		argc--, argv++;
	} while (argc > 0);
	exit(retcode);
	/* NOTREACHED */
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
	long curoff = NULL;
	register struct dirent *dp;
	long nblock();


	ebase0 = ebase = strchr(base, 0);
	if (ebase > base && ebase[-1] == '/')
		ebase--;
	if (lstat(name, &stb) < 0) {
		perror(base);
		*retcode = 1;
		*ebase0 = 0;
		return (0);
	}
	if (stb.st_nlink > 1 && (stb.st_mode&S_IFMT) != S_IFDIR) {
		for (i = 0; i <= mlx; i++)
			if (ml[i].ino == stb.st_ino && ml[i].dev == stb.st_dev)
				return (0);
		if (mlx < ML) {
			ml[mlx].dev = stb.st_dev;
			ml[mlx].ino = stb.st_ino;
			mlx++;
		}
	}
	blocks = nblock(stb.st_size);
	if ((stb.st_mode&S_IFMT) != S_IFDIR) {
		if (aflg)
			printf("%ld\t%s\n", kb(blocks), base);
		return (blocks);
	}
	if (dirp != NULL)
		closedir(dirp);
	dirp = opendir(name);
	if (dirp == NULL) {
		perror(base);
		*ebase0 = 0;
		return (0);
	}
	if (chdir(name) < 0) {
		perror(base);
		*ebase0 = 0;
		*retcode = 1;
		closedir(dirp);
		dirp = NULL;
		return (0);
	}
	while (dp = readdir(dirp)) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		(void) sprintf(ebase, "/%s", dp->d_name);
		curoff = telldir(dirp);
		blocks += descend(base, ebase+1, retcode);
		*ebase = 0;
		if (dirp == NULL) {
			dirp = opendir(".");
			if (dirp == NULL) {
				perror(".");
				*retcode = 1;
				return (0);
			}
			seekdir(dirp, curoff);
		}
	}
	closedir(dirp);
	dirp = NULL;
	if (sflg == 0)
		printf("%ld\t%s\n", kb(blocks), base);
	if (chdir("..") < 0) {
		(void) sprintf(strchr(base, 0), "/..");
		perror(base);
		*retcode = 1;
		exit(1);
	}
	*ebase0 = 0;
	return (blocks);
}



/*
 *	nblock - calculate the number of blocks used to
 *	store this amount of data.
 *	XXX Two problems with this algorithm.
 *	First, in calculating the number of disk blocks
 *	allocated, it makes assumptions about the logical
 *	disk block size.  At the moment, it assumes an
 *	S5 format (traditional) with 1K block size.
 *	This could be rectified through use of a new
 *	system call that provides the block size of
 *	the file system (similar to BSD stat).  This
 *	would help make du file system independent.
 *	Statvfs(2) does provide the block & fragment size,
 *	but can only be used on open files.
 *
 *	Second, the algorithm fails to calculate
 *	the amount of disk space used internally to
 *	link the data blocks together.
 *
 *	These two problems could be rectified
 *	if an allocation number were provided in a system
 *	call similar to Sun's stat.
 */

#define BUFSIZE	1024		/* file system allocation unit */
#define PPERB	(BUFSIZE/512)	/* 512 byte units per allocation */

long nblock(size)
long size;
{
	/* du reports no. of blocks in 512-byte block,   
	 * in 1K file sytem, minimum allocation is 2 blocks 
     	 */
	return(PPERB*((size + (BUFSIZE -1))/BUFSIZE));
}
