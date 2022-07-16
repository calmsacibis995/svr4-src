/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/

#ident	"@(#)libpkg:isdir.c	1.3.3.1"

#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

extern int	sprintf();

isdir(path)
char *path;
{
	struct stat statbuf;

	if(!stat(path, &statbuf) && ((statbuf.st_mode & S_IFMT) == S_IFDIR))
		return(0);
	return(1);
}

isfile(dir, file)
char *dir;
char *file;
{
	struct stat statbuf;
	char	path[PATH_MAX];

	if(dir) {
		(void) sprintf(path, "%s/%s", dir, file);
		file = path;
	}

	if(!stat(file, &statbuf) && (statbuf.st_mode & S_IFREG))
		return(0);
	return(1);
}
