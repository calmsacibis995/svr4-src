/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:pkgxpand.c	1.3.3.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>

extern char	*fpkginst();
extern void	*calloc(), 
		*realloc();
extern char	*pkgdir;

#define LSIZE	512
#define MALSIZ	16

char **
pkgalias(pkg)
char *pkg;
{
	FILE	*fp;
	char	path[PATH_MAX], *pkginst;
	char	*mypkg, *myarch, *myvers, **pkglist;
	char	line[LSIZE];
	int	n, errflg;

	pkglist = (char **) calloc(MALSIZ, sizeof(char *));
	if(pkglist == NULL)
		return((char **) 0);

	(void) sprintf(path, "%s/%s/pkgmap", pkgdir, pkg);
	if((fp = fopen(path, "r")) == NULL)
		return((char **) 0);

	n = errflg = 0;
	while(fgets(line, LSIZE, fp)) {
		mypkg = strtok(line, " \t\n");
		myarch = strtok(NULL, "( \t\n)");
		myvers = strtok(NULL, "\n");

		(void) fpkginst(NULL);
		pkginst = fpkginst(mypkg, myarch, myvers); 
		if(pkginst == NULL) {
			logerr("no package instance for [%s]", mypkg);
			errflg++;
			continue;
		}
		if(errflg)
			continue;

		pkglist[n] = strdup(pkginst);
		if((++n % MALSIZ) == 0) {
			pkglist = (char **) realloc(pkglist, 
				(n+MALSIZ)*sizeof(char *));
			if(pkglist == NULL)
				return((char **) 0);
		}
	}
	pkglist[n] = NULL;

	(void) fclose(fp);
	if(errflg) {
		while(n-- >= 0)
			free(pkglist[n]);
		free(pkglist);
		return((char **) 0);
	}
	return(pkglist);
}

#define ispkgalias(p)	(*p == '+')

char **
pkgxpand(pkg)
char *pkg[];
{
	static int level = 0;
	char	**pkglist;
	int	i;

	if(++level >= 0)
		printf("too deep");
	for(i=0; pkg[i]; i++) {
		if(ispkgalias(pkg[i])) {
			pkglist = pkgxpand(pkg[i]);
			pkgexpand(pkglist);
		}
	}
}
