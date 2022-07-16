/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:gpkglist.c	1.6.5.1"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <valtools.h>
#include <pkginfo.h>

extern int	errno;
extern char	*pkgdir;

extern int	pkgnmchk(),
		fpkginfo(),
		ckitem(),
		pkginfo(),
		setinvis(),
		setitem();
extern void	*calloc(),
		*realloc(),
		free(),
		progerr();
extern char	*fpkginst();
extern CKMENU	*allocmenu();

#define CMDSIZ	512
#define LSIZE	256
#define MAXSIZE	128
#define MALLOCSIZ 128

#define ERR_MEMORY	"memory allocation failure, errno=%d"
#define ERR_NOPKG	"no package associated with <%s>"
#define HEADER	"The following packages are available:"
#define HELP	\
	"Please enter the package instances you wish to process \
	from the list provided (or 'all' to process all packages.)"

#define PROMPT	\
	"Select package(s) you wish to process (or 'all' to process \
	all packages)."

char **
gpkglist(dir, pkg)
char	*dir;
char	**pkg;
{
	struct _choice_ *chp;
	struct pkginfo info;
	char	*inst;
	CKMENU	*menup;
	char	temp[LSIZE];
	char	*savedir, **nwpkg;
	int	i, n;

	savedir = pkgdir;
	pkgdir = dir;

	info.pkginst = NULL; /* initialize for memory handling */
	if(pkginfo(&info, "all", NULL, NULL)) {
		errno = ENOPKG; /* contains no valid packages */
		pkgdir = savedir;
		return(NULL);
	}
	if(pkg[0] == NULL) {
		menup = allocmenu(HEADER, CKALPHA);
		if(setinvis(menup, "all")) {
			errno = EFAULT;
			return(NULL);
		}
		do {
			(void) sprintf(temp, "%s %s\n(%s) %s", info.pkginst,
				info.name, info.arch, info.version);
			if(setitem(menup, temp)) {
				errno = EFAULT;
				return(NULL);
			}
		} while(pkginfo(&info, "all", NULL, NULL) == 0);
		/* clear memory usage by pkginfo */
		(void) pkginfo(&info, NULL, NULL, NULL); 
		pkgdir = savedir;	/* restore pkgdir to orig value */

		nwpkg = (char **) calloc(MALLOCSIZ, sizeof(char **));
		n = ckitem(menup, nwpkg, MALLOCSIZ, "all", NULL, HELP, PROMPT);
		if(n) {
			free(nwpkg);
			errno = ((n == 3) ? EINTR : EFAULT);
			pkgdir = savedir;
			return(NULL);
		}
		if(!strcmp(nwpkg[0], "all")) {
			chp = menup->choice;
			for(n=0; chp; ) {
				nwpkg[n] = strdup(chp->token);
				if((++n % MALLOCSIZ) == 0) {
					nwpkg = (char **) realloc(nwpkg, 
						(n+MALLOCSIZ)* sizeof(char**));
					if(nwpkg == NULL) {
						progerr(ERR_MEMORY, errno);
						errno = ENOMEM;
						return(NULL);
					}
				}
				chp = chp->next;
				nwpkg[n] = NULL;
			}
		} else {
			for(n=0; nwpkg[n]; n++)
				nwpkg[n] = strdup(nwpkg[n]);
		}
		(void) setitem(menup, NULL); /* free resources */
		free(menup);
		pkgdir = savedir;
		return(nwpkg);
	}
	(void) pkginfo(&info, NULL, NULL, NULL); 
		/* clear memory usage by pkginfo */

	nwpkg = (char **) calloc(MALLOCSIZ, sizeof(char **));

	/*
	 * pkg array contains the instance identifiers to
	 * be selected, or possibly wildcard definitions
	 */
	i = n = 0;
	do {
		if(pkgnmchk(pkg[i], "all", 1)) {
			/* wildcard specification */
			(void) fpkginst(NULL);
			inst = fpkginst(pkg[i], NULL, NULL);
			if(inst == NULL) {
				progerr(ERR_NOPKG, pkg[i]);
				free(nwpkg);
				nwpkg = NULL;
				errno = ESRCH;
				break;
			} 
			do {
				nwpkg[n] = strdup(inst);
				if((++n % MALLOCSIZ) == 0) {
					nwpkg = (char **) realloc(nwpkg, 
						(n+MALLOCSIZ)* sizeof(char**));
					if(nwpkg == NULL) {
						progerr(ERR_MEMORY, errno);
						errno = ENOMEM;
						return(NULL);
					}
				}
				nwpkg[n] = NULL;
			} while(inst = fpkginst(pkg[i], NULL, NULL));
		} else {
			if(fpkginfo(&info, pkg[i])) {
				progerr(ERR_NOPKG, pkg[i]);
				free(nwpkg);
				nwpkg = NULL;
				errno = ESRCH;
				break;
			}
			nwpkg[n] = strdup(pkg[i]);
			if((++n % MALLOCSIZ) == 0) {
				nwpkg = (char **) realloc(nwpkg, 
					(n+MALLOCSIZ)* sizeof(char**));
				if(nwpkg == NULL) {
					progerr(ERR_MEMORY, errno);
					errno = ENOMEM;
					return(NULL);
				}
			}
			nwpkg[n] = NULL;
		}
	} while(pkg[++i]);

	(void) fpkginst(NULL);
	(void) fpkginfo(&info, NULL);
	pkgdir = savedir;	/* restore pkgdir to orig value */
	return(nwpkg);
}
