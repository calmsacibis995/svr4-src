/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

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
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)ufs.cmds:ufs/mount/realpath.c	1.2.3.1"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/mntent.h>
#include <sys/stat.h>
#include <sys/errno.h>

extern int errno;

char *strchr(), *strrchr(), *strcpy();
char *getcwd(), *sprintf();

/*
 * Resolve all links for the given "name" and returns the result
 * in "actual". Returns pointer to the result if successful, else
 * NULL and errno is set. "actual" contains the path name of the
 * fault point.
 */
static char *
resolvelinks(name, actual)
	register char *name;
	char *actual;
{	int last_is_slash = 0;
	register char *p = actual;
	char namebuf[MAXPATHLEN];
	register char *tmpname = namebuf;

	actual[0] = NULL;
	if ((name == NULL) || (name[0] == NULL))
		return (actual);
	if (name[0] == '/') {
		actual[0] = '/';
		actual[1] = NULL;
		if (*++name == NULL)
			return (actual);
		++p;
	}

	if (name[strlen(name) - 1] == '/')
		++last_is_slash;

	strcpy(tmpname, name);	/* Work in tmp area only */

	/*
	 * "tmpname" points to the yet unresolved path name.
	 * "actual" points to the resolved path name so far.
	 */
	while (tmpname) {
		register char *start;		/* The new step */
		char tmpres[MAXPATHLEN];	/* Temp result */
		struct stat stbuf;

		start = tmpname;
		if (tmpname = strchr(tmpname, '/'))
			*tmpname++ = NULL;

		/* start points to the new step */
		sprintf(tmpres, "%s%s", actual, start);
		if (lstat(tmpres, &stbuf) == -1) {
			strcpy(actual, tmpres);	/* Point of error */
			return (NULL);
		}

		if ((stbuf.st_mode & S_IFMT) == S_IFLNK) {
			char buf[MAXPATHLEN];
			int count;

			if ((count = readlink(tmpres, buf, MAXPATHLEN)) <= 0) {
				strcpy(actual, tmpres);	/* Point of error */
				return (NULL);
			} else {
				char tmpdir[MAXPATHLEN];
				char tmpbuf[MAXPATHLEN];
				char *tptr = &tmpbuf[0];
				char *resolved_res;

				buf[count] = NULL;
				/*
				 * Recursively check for all new links.
				 */
				if (getcwd(tmpdir, MAXPATHLEN) == NULL)
					return (NULL);
				if (chdir(actual) == -1)
					return (NULL);
				resolved_res = resolvelinks(buf, tmpbuf);
				if (tmpbuf[0] == '/')
				  	/* The link starts from root */
					for (p = &actual[0];
						*p = *tptr; p++, tptr++);
				else
					for (; *p = *tptr; p++, tptr++);
				if (resolved_res == NULL) {
					(void) chdir(tmpdir);
					return (NULL);
				}
				if (chdir(tmpdir) == -1)
					return (NULL);
			}
		} else
			/* Append start to actual */
			for (; *p = *start; p++, start++);

		if (*(p - sizeof(char)) != '/')
			*p++ = '/';	/* Append '/' to actual */
		*p = NULL;
	}

	/* Add/delete "/" depending upon last_is_slash */
	if (last_is_slash == 0) {
		if ((*(p - sizeof(char)) == '/') && (actual[1] != NULL))
			*--p = NULL;
	} else if (*(p - sizeof(char)) != '/')
		*p++ = '/';
	*p = NULL;

	return (actual);
}

/*
 * Given a path, it removes all the '.' and '..' in it and returns 
 * pointer to the result if successful, else returns NULL and the
 * error number is set. The partially prepared result is in "dots".
 */
static char *
resolvedots(actual, dots)
	register char *actual;
	register char *dots;	/* The result */
{
	struct stat stbuf;
	int last_is_slash = 0;
	char pwd[MAXPATHLEN];
	char *endp;

	if (dots == NULL) {
		errno = EINVAL;
		return (NULL);
	}
	dots[0] = NULL;

	if ((actual == NULL) || (actual[0] == NULL))
		return (dots);
	if (actual[strlen(actual) - 1] == '/')
		++last_is_slash;
	if (getcwd(pwd, MAXPATHLEN) == NULL)
		return (NULL);

	if (lstat(actual, &stbuf) == -1) {
		strcpy(dots, actual);
		return (NULL);
	}
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		char dir[MAXPATHLEN];

		if ((chdir(actual) != 0) || (getcwd(dir, MAXPATHLEN) == NULL)) {
			(void) chdir(pwd);
			strcpy(dots, actual);
			return (NULL);
		}
		if ((actual[0] != '/') &&
		    (strncmp(pwd, dir, strlen(pwd)) == 0) &&
		    (strlen(pwd) != strlen(dir)))
			/* Copy only the relative part of the name */
			strcpy(dots, (char *) (&dir[0] + strlen(pwd) + 1));
		else
			strcpy(dots, dir);
	} else { /* 
		  * Last component is a file. Find the resolved
		  * name of the parent of this file, and then append
		  * the name of this file.
		  */
		char *pdir;

		if ((pdir = strrchr(actual, '/')) == NULL)
			strcpy(dots, actual);
		else {
			char partial_dir[MAXPATHLEN];

			*pdir++ = NULL;
			if ((chdir(actual) != 0) ||
			    (getcwd(partial_dir, MAXPATHLEN) == NULL)) {
				(void) chdir(pwd);
				strcpy(dots, actual);
				return (NULL);
			}
			if ((actual[0] != '/') &&
			    (strncmp(pwd, partial_dir, strlen(pwd)) == 0)) {
				if (strlen(pwd) == strlen(partial_dir))
					strcpy(dots, pdir);
				else
					sprintf(dots, "%s/%s",
					    (char *) (&partial_dir[0] +
						strlen(pwd) + 1), pdir);
			} else
				sprintf(dots, "%s/%s", partial_dir, pdir);
			*--pdir = '/';
		}
	}

	if (chdir(pwd) == -1)
		return (NULL);

	/* Add/delete "/" depending upon last_is_slash */
	endp = &dots[strlen(dots)];
	if (last_is_slash == 0) {
		if ((*(endp - sizeof(char)) == '/') && (dots[1] != NULL))
			*(endp - sizeof(char)) = NULL;
	} else if (*(endp - sizeof(char)) != '/') {
		*endp++ = '/';
		*endp = NULL;
	}

	return (dots);
}

/*
 * Given a path, it resolves all links, '.' and '..' in it and
 * returns pointer to the result if successful, else returns NULL and
 * error number is set. The partially prepared result is in "linkdots".
 * Assumes that "linkdots" has enough space allocated to it.
 */
char *
realpath(name, linkdots)
	char *name, *linkdots;
{
	char buf[MAXPATHLEN];

	if (resolvelinks(name, buf) == NULL) {
		strcpy(linkdots, buf);
		return (NULL);
	}
	return (resolvedots(buf, linkdots));
}
