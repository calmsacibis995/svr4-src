/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getcwd.c	1.17"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/

#ifdef __STDC__
	#pragma weak getcwd = _getcwd
#endif
#include 	"synonyms.h"
#include "shlib.h"
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <errno.h>
#include        <dirent.h>
#include        <string.h>
#include        <stdlib.h>
#define        NULL 0

#define MAX_PATH 1024
#define MAX_NAME 512
#define BUF_SIZE 1536 /* 3/2 of MAX_PATH for /a/a/a... case */

/* 
 * This algorithm does not use chdir.  Instead, it opens a 
 * successive strings for parent directories, i.e. .., ../..,
 * ../../.., and so forth.
 */
extern DIR	*opendir();
extern struct dirent	*readdir();
extern int	closedir();
extern int	stat(), fstat();

char *getcwd(str, size)
char *str;
int size;
{
	char dotdots[BUF_SIZE+MAX_NAME];
	struct stat		cdir;	/* current directory status */
	struct stat		tdir;
	struct stat		pdir;	/* parent directory status */
	DIR			*pdfd;	/* parent directory stream */

	struct dirent *dir;
	char *dotdot = dotdots + BUF_SIZE - 3;
	char *dotend = dotdots + BUF_SIZE - 1; 
	int i, maxpwd, alloc, saverr, ret; 
	
	if(size <= 0) {
		errno = EINVAL;
		return NULL;
	}
	
	if(stat(".", &pdir) < 0)
		return NULL;
	
	alloc = 0;
	if(str == NULL)  {
		if((str = (char *)malloc((unsigned)size)) == NULL) {
			errno = ENOMEM;
			return NULL;
		}
		alloc = 1;
	}
	
	*dotdot = '.';
	*(dotdot+1) = '.';
	*(dotdot+2) = '\0';
	maxpwd = size--;
	str[size] = 0;

	for(;;)
	{
		/* update current directory */
		cdir = pdir;

		/* open parent directory */
		if ((pdfd = opendir(dotdot)) == 0)
			break;

		if(fstat(pdfd->dd_fd, &pdir) < 0)
		{
			saverr = errno;
			(void)closedir(pdfd);
			errno = saverr;
			break;
		}

		/*
		 * find subdirectory of parent that matches current 
		 * directory
		 */
		if(cdir.st_dev == pdir.st_dev)
		{
			if(cdir.st_ino == pdir.st_ino)
			{
				/* at root */
				(void)closedir(pdfd);
				if (size == (maxpwd - 1))
					/* pwd is '/' */
					str[--size] = '/';

				strcpy(str, &str[size]);
				return str;
			}
			do
			{
				if ((dir = readdir(pdfd)) == 0)
				{
					saverr = errno;
					(void)closedir(pdfd);
					errno = saverr;
					goto out;
				}
			}
			while (dir->d_ino != cdir.st_ino);
		}
		else
		{
			/*
			 * must determine filenames of subdirectories
			 * and do stats
			 */
			*dotend = '/';
			do
			{
				if ((dir = readdir(pdfd)) == 0)
				{
					saverr = errno;
					(void)closedir(pdfd);
					errno = saverr;
					goto out;
				}
				strcpy(dotend + 1, dir->d_name);
				/* skip over non-stat'able
				 * entries
				 */
				ret = stat(dotdot, &tdir);
				
			}		
			while(ret == -1 || tdir.st_ino != cdir.st_ino || tdir.st_dev != cdir.st_dev);
		}
		(void)closedir(pdfd);

		i = strlen(dir->d_name);

		if (i > size - 1) {
			errno = ERANGE;
			break;
		} else {
			/* copy name of current directory into pathname */
			size -= i;
			strncpy(&str[size], dir->d_name, i);
			str[--size] = '/';
		}
		if (dotdot - 3 < dotdots) 
			break;
		/* update dotdot to parent directory */
		*--dotdot = '/';
		*--dotdot = '.';
		*--dotdot = '.';
		*dotend = '\0';
	}
out:
	if(alloc)
		free(str);
	return NULL;
}

