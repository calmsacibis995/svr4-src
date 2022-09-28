/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:expfile.c	2.10.3.1"

#include "uucp.h"

/*
 * expand file name expansion is based on first characters
 *	/	-> fully qualified pathname. no
 *		   processing necessary
 *	~	-> prepended with login directory
 *	~/	-> prepended with Pubdir
 *	default	-> prepended with current directory
 *	file	-> filename to expand
 * returns: 
 *	0	-> ok
 *      FAIL	-> no Wrkdir name available
 */
int
expfile(file)
register char *file;
{
	register char *fpart, *up;
	uid_t uid;
	char user[NAMESIZE], save[MAXFULLNAME];
	extern int gninfo(), canPath();

	strcpy(save, file);
	if (*file != '/')
	    if (*file ==  '~') {
		/* find / and copy user part */
		for (fpart = save + 1, up = user; *fpart != '\0'
			&& *fpart != '/'; fpart++)
				*up++ = *fpart;
		*up = '\0';
		if ((user[0]=='\0') || (gninfo(user, &uid, file) != 0)){
			(void) strcpy(file, Pubdir);
		}
		(void) strcat(file, fpart);
	    } else {
		(void) sprintf(file, "%s/%s", Wrkdir, save);
		if (Wrkdir[0] == '\0')
			return(FAIL);
	    }

	if (canPath(file) != 0) { /* I don't think this will ever fail */
	    (void) strcpy(file, CORRUPTDIR);
	    return(FAIL);
	} else
	    return(0);
}


/*
 * make all necessary directories
 *	name	-> directory to make
 *	mask	-> mask to use during directory creation
 * return: 
 *	0	-> success
 * 	FAIL	-> failure
 */
int
mkdirs(name, mask)
mode_t mask;
register char *name;
{
	register char *p;
	mode_t omask;
	char dir[MAXFULLNAME];

	strcpy(dir, name);
	if (*LASTCHAR(dir) != '/')
	    	(void) strcat(dir, "/");
	p = dir + 1;
	for (;;) {
	    if ((p = strchr(p, '/')) == NULL)
		return(0);
	    *p = '\0';
	    if (DIRECTORY(dir)) {
		/* if directory exists and is owned by uucp, child's
		    permissions should be no more open than parent */
		if (__s_.st_uid == UUCPUID)
		    mask |= ((~__s_.st_mode) & PUB_DIRMODE);
	    } else {
		DEBUG(4, "mkdir - %s\n", dir);
		omask = umask(mask);
		if (mkdir(dir, PUB_DIRMODE) == FAIL) {
		    umask(omask);
		    return (FAIL);
		}
		umask(omask);
	    }
	    *p++ = '/';
	}
	/* NOTREACHED */
}

/*
 * expand file name and check return
 * print error if it failed.
 *	file	-> file name to check
 * returns: 
 *      0	-> ok
 *      FAIL	-> if expfile failed
 */
int
ckexpf(file)
char *file;
{
	if (expfile(file) == 0)
		return(0);

	fprintf(stderr, "Illegal filename (%s).\n", file);
	return(FAIL);
}


/*
 * make canonical path out of path passed as argument.
 *
 * Eliminate redundant self-references like // or /./
 * (A single terminal / will be preserved, however.)
 * Dispose of references to .. in the path names.
 * In relative path names, this means that .. or a/../..
 * will be treated as an illegal reference.
 * In full paths, .. is always allowed, with /.. treated as /
 *
 * returns:
 *	0	-> path is now in canonical form
 *	FAIL	-> relative path contained illegal .. reference
 */

int
canPath(path)
register char *path;	/* path is modified in place */
{
    register char *to, *fr;

    to = fr = path;
    if (*fr == '/') *to++ = *fr++;
    for (;;) {
	/* skip past references to self and validate references to .. */
	for (;;) {
	    if (*fr == '/') {
		fr++;
		continue;
	    }
	    if ((strncmp(fr, "./", 2) == SAME) || EQUALS(fr, ".")) {
		fr++;
		continue;
	    }
	    if ((strncmp(fr, "../", 3) == SAME) || EQUALS(fr, "..")) {
		fr += 2;
		/*	/.. is /	*/
		if (((to - 1) == path) && (*path == '/')) continue;
		/* error if no previous component */
		if (to <= path) return (FAIL);
		/* back past previous component */
		while ((--to > path) && (to[-1] != '/'));
		continue;
	    }
	    break;
	}
	/*
	 * What follows is a legitimate component,
	 * terminated by a null or a /
	 */
	if (*fr == '\0') break;
	while (((*to++ = *fr) != '\0') && (*fr++ != '/'));
    }
    /* null path is . */
    if (to == path) *to++ = '.';
    *to = '\0';
    return (0);
}
