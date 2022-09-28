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

#ident	"@(#)fmli:oh/alias.c	1.9"

#include	<stdio.h>
#include	<string.h>
#include	"wish.h"
#include	"token.h"
#include	"slk.h"
#include	"actrec.h"
#include	"ctl.h"
#include	"moremacros.h"
#include	"sizes.h"

extern int Vflag;
extern char *Aliasfile;

#define MAX_ALIAS	32

static int	Num_alias = 0;
static struct	pathalias {
	char	*alias;
	char	*path;
} Alias[MAX_ALIAS];

char *
path_to_full(s)
char	*s;
{
	int	n;
	char	buf[PATHSIZ];
	register char	*p, *q;
	bool	b;
	struct actrec	*a;
	extern char	*Home, *Filecabinet;
	struct actrec	*wdw_to_ar();
	char	 *expand(), *alias_to_path();

	if (strcmp(s, "-i") == 0)	/* unfortunate kludge for inline objects */
		return(strsave(s));

	if (Num_alias == 0 && Aliasfile)
		get_aliases();

	/* check if a number, if so then path of an open folder */

	if ((n = atoi(s)) > 0 && strspn(s, "0123456789") == strlen(s) && 
			(a = wdw_to_ar(n)) != NULL)
		return(strsave(a->path));

	p = expand(s);
	if (*p == '/')	/* already a full path */
		return(p);

	/* check if an alias of another path */

	if (q = strchr(p, '/'))
		*q = '\0';
	if ((s = alias_to_path(p, q ? q + 1 : NULL)) != NULL)
		return(s);
	if (q)
		*q = '/'; 		/* restore p */

	if (Vflag) {
		/*
		 * relative to current folder if there is one, else FILECABINET
		 */

		if (ar_ctl(ar_get_current(), CTISDEST, &b) != FAIL && b == TRUE)
			sprintf(buf, "%s/%s", ar_get_current()->path, p);
		else
			sprintf(buf, "%s/%s", Filecabinet, p);
	}
	else 
		strcpy(buf, p); 
	free(p);
	return(strsave(buf));
}

static
get_aliases()
{
	char	path[PATHSIZ];
	extern char	*Home;

	if (Vflag) {
		sprintf(path, "%s/pref/pathalias", Home);
		get_one(path);
	}
	strcpy(path, Aliasfile);
	get_one(path);
}

static
get_one(path)
char	*path;
{
	FILE	*fp;
	char	buf[BUFSIZ];
	char	*p;
	char	 *expand();

	if ((fp = fopen(path, "r")) == NULL)
		return;

	while (Num_alias < MAX_ALIAS && fgets(buf, BUFSIZ, fp)) {
		if (p = strchr(buf, '=')) {
			buf[strlen(buf)-1] = '\0';	/* clip off the newline */
			*p = '\0';
			/* les 12/4
			if (Alias[Num_alias].alias)
				free(Alias[Num_alias].alias);
			*/
			Alias[Num_alias].alias = strsave(buf);
			Alias[Num_alias].path = expand(++p);
			Num_alias++;
		}
	}
	fclose(fp);
}

char *
alias_to_path(s, rest)
char	*s;
char	*rest;
{
	register int	i;
	
	for (i = 0; i < Num_alias; i++) {
		if (strCcmp(s, Alias[i].alias) == 0) {
			char *hold;
			register char *p;
			char path[PATHSIZ];

			if (!strchr(Alias[i].path, ':')) {
				strcpy(path, Alias[i].path);
				if (rest) {
					strcat(path, "/");
					strcat(path, rest);
				}
				return(strsave(path));
			}
			for (p = strtok(hold = strsave(Alias[i].path), ":"); p; p = strtok(NULL, ":")) {
				strcpy(path, p);
				if (rest) {
					strcat(path, "/");
					strcat(path, rest);
				}
				if (access(path, 0) == 0) {
					free(hold);
					return(strsave(path));
				}
			}
			break;
		}
	}
	return(NULL);
}
