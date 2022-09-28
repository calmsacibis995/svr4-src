/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/getepenv.c	1.4"

#include	<stdio.h>
#include	"wish.h"
#include	"moremacros.h"
#include 	"sizes.h"

/*
 * getepenv looks for name=value in environment and in $HOME/pref/.environ
 * If not present, return NULL.
 */
char *
getepenv(name)
char	*name;
{
	char	path[PATHSIZ];
	register char	*ptr;
	extern char	*Home;
	char	*anyenv();
	char	*getenv();

	if ((ptr = getAltenv(name)) || (ptr = getenv(name)))
		return strsave(ptr);
	strcpy(path, Home);
	strcat(path, "/pref/.environ");
	return anyenv(path, name);
}

/*
 * anyenv lloks in path for name=value and returns value
 * value is backslash processed and expanded before it is returned
 */
char *
anyenv(path, name)
char	*path;
char	*name;
{
	char	buf[BUFSIZ];
	char	fpbuf[BUFSIZ];
	register char	*ptr;
	register int	c;
	register FILE	*fp;
	char	*fgets();
	char	*expand();
	char	*unbackslash();

	if ((fp = fopen(path, "r")) == NULL)
		return NULL;
	setbuf(fp, fpbuf);
	/* for (each line of .environ file) */
	for (c = !EOF; c != EOF; ) {
		ptr = name;
		while (*ptr && (c = getc(fp)) == *ptr)
			ptr++;
		/* if ("name=" found) get rest of line */
		if (*ptr == '\0' && (c = getc(fp)) == '=' && fgets(buf, sizeof(buf), fp)) {
			if (buf[c = strlen(buf) - 1] == '\n')
				buf[c] = '\0';
			fclose(fp);
			return expand(unbackslash(buf));
		}
		/* skip rest of line */
		while (c != EOF && c != '\n')
			c = getc(fp);
	}
	fclose(fp);
	return NULL;
}
