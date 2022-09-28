/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/vpinit.c	1.3"
/* vpinit - initialize vpdirs or update vpdirs based on currentdir */

#include <stdio.h>	/* stderr */
#include <string.h>
#include "vp.h"

#if !NOMALLOC
char	**vpdirs;	/* directories (including current) in view path */
#else
char	vpdirs[MAXDIR][DIRLEN + 1];
#define	MAXVPATH (MAXDIR * (DIRLEN + 1))
#endif
int	vpndirs;	/* number of directories in view path */

extern	char	*argv0;	/* command name (must be set in main function) */

void
vpinit(currentdir)
char	*currentdir;
{
	char	*suffix;	/* path from view path node */
	char	*vpath;		/* VPATH environment variable value */
	char	buf[MAXPATH + 1];
	int	i;
	char	*s;
	char	*getenv(), *getwd(), *strcat(), *strcpy();
#if !NOMALLOC
	char	*mymalloc(), *stralloc();
	void	free();
#else
	char	*node;		/* view path node */
	char	vpathbuf[MAXVPATH + 1];
#endif
	
	/* if an existing directory list is to be updated, free it */
	if (currentdir != NULL && vpndirs > 0) {
#if !NOMALLOC
		for (i = 0; i < vpndirs; ++i) {
			free(vpdirs[i]);
		}
		free((char *) vpdirs);
#endif
		vpndirs = 0;
	}
	/* return if the directory list has been computed */
	/* or there isn't a view path environment variable */
	if (vpndirs > 0 || (vpath = getenv("VPATH")) == NULL ||
	    *vpath == '\0') {
		return;
	}
	/* if not given, get the current directory name */
	if (currentdir == NULL && (currentdir = getwd(buf)) == NULL) {
		(void) fprintf(stderr, "%s: cannot get current directory name\n", argv0);
		return;
	}
	/* see if this directory is in the first view path node */
	for (i = 0; vpath[i] == currentdir[i] && vpath[i] != '\0'; ++i) {
		;
	}
	if ((vpath[i] != ':' && vpath[i] != '\0') ||
	    (currentdir[i] != '/' && currentdir[i] != '\0')) {
		return;
	}
	suffix = &currentdir[i];
#if !NOMALLOC

	/* count the nodes in the view path */
	vpndirs = 1;
	for (i = 0; vpath[i] != '\0'; ++i) {
		if (vpath[i] == ':') {
			++vpndirs;
		}
	}
	/* create the source directory list */
	vpdirs = (char **) mymalloc(vpndirs * sizeof(char *));

	/* don't change VPATH in the environment */
	vpath = stralloc(vpath);
	
	/* split the view path into nodes */
	for (i = 0, s = vpath; *s != '\0'; ++i) {
		vpdirs[i] = s;
		while (*s != '\0' && *++s != ':') {
			if (*s == '\n') {
				*s = '\0';
			}
		}
		if (*s != '\0') {
			*s++ = '\0';
		}
	}
	/* convert the view path nodes to directories */
	for (i = 0; i < vpndirs; ++i) {
		s = mymalloc((unsigned) (strlen(vpdirs[i]) + 
			strlen(suffix) + 1));
		(void) strcpy(s, vpdirs[i]);
		(void) strcat(s, suffix);
		vpdirs[i] = s;
	}
	free(vpath);
#else
	/* don't change VPATH in the environment */
	if (strlen(vpath) > MAXVPATH) {
		(void) fprintf(stderr, "%s: VPATH is longer than %d characters: %s\n", argv0, MAXVPATH, vpath);
		return;
	}
	(void) strcpy(vpathbuf, vpath);
	s = vpathbuf;
	
	/* convert the view path nodes to directories */
	while (*s != '\0') {
		
		/* get the next node */
		node = s;
		while (*s != '\0' && *++s != ':') {
			if (*s == '\n') {
				*s = '\0';
			}
		}
		if (*s != '\0') {
			*s++ = '\0';
		}
		/* ignore a directory that is too long */
		if (strlen(node) + strlen(suffix) > DIRLEN) {
			(void) fprintf(stderr, "%s: VPATH directory is longer than %d characters: %s%s\n", argv0, DIRLEN, node, suffix);
		}
		else if (vpndirs >= MAXDIR) {
			(void) fprintf(stderr, "%s: VPATH has more than %d nodes\n", argv0, vpndirs);
			return;
		}
		else {
			/* create the view path directory */
			(void) strcpy(vpdirs[vpndirs], node);
			(void) strcat(vpdirs[vpndirs], suffix);
			++vpndirs;
		}
	}
#endif
}
