/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgchk/checkmap.c	1.2.6.1"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkglocs.h>

extern int	errno, qflag, lflag, Lflag, pkgcnt;
extern short	npaths;
extern char	*errstr,
		*basedir,
		*pathlist[],
		**pkg,
		**environ;
extern short	used[];

extern int	ckentry(),
		pkgnmchk(),
		srchcfile(),
		setlist(),
		gpkgmap();
extern char	*getenv(),
		*fpkgparam();
extern void	*calloc(),
		free();
extern void	putparam(),
		progerr(),
		logerr(),
		exit();

#define nxtentry(p) \
		(maptyp ? srchcfile(p, "*", fp, NULL) : gpkgmap(p, fp))
#define MALLSIZ		50

#define WRN_NOPKG	"WARNING: no pathnames were associated with <%s>"
#define WRN_NOPATH	"WARNING: no information associated with pathname <%s>"
#define ERR_NOENTRIES	"no entries processed for <%s>"
#define ERR_NOMEM	"unable to allocate dynamic memory, errno=%d"
#define ERR_PKGMAP	"unable to open pkgmap file <%s>"
#define ERR_ENVFILE	"unable to open environment file <%s>"

static struct cfent
		entry;
static int	selpath(), selpkg(), selclass(), shellmatch();
static char	**classes;

/* this routine checks all files which are referenced
 * in the pkgmap which is identified by mapfile arg
 */

int
checkmap(maptyp, mapfile, envfile, pkginst)
int	maptyp;
char	*mapfile, *envfile, *pkginst;
{
	FILE *fp;
	struct pinfo *pinfo;
	int	n, count, selected, errflg;
	char	param[64], *value;
	
	if(envfile != NULL) {
		if((fp = fopen(envfile, "r")) == NULL) {
			progerr(ERR_ENVFILE, envfile);
			return(-1);
		}
		param[0] = '\0';
		environ = NULL;
		while(value = fpkgparam(fp, param)) {
			if(strcmp("PATH", param))
				putparam(param, value);
			free(value);
			param[0] = '\0';
		}
		(void) fclose(fp);
		basedir = getenv("BASEDIR");
	}

	if((fp = fopen(mapfile, "r")) == NULL) {
		progerr(ERR_PKGMAP, mapfile);
		return(-1);
	}

	if(!setlist(&classes, qstrdup(getenv("CLASSES"))))
		classes = (char **)NULL;

	errflg = count = 0;
	while(n = nxtentry(&entry)) {
		if(n < 0) {
			logerr("ERROR: garbled entry");
			logerr("pathname: %s", entry.path);
			logerr("problem: %s", errstr);
			exit(99);
		}
		if(n == 0)
			break; /* done with file */

		if(maptyp) {
			/* check to see if the entry we just read
			 * is associated with one of the packages
			 * we have listed on the command line
			 */
			selected = 0;
			pinfo = entry.pinfo;
			while(pinfo) {
				if(selpkg(pinfo->pkg)) {
					selected++;
					break;
				}
				pinfo = pinfo->next;
			}
			if(!selected)
				continue; /* not selected */
		}

		/* check to see if the pathname associated with the
		 * entry we just read is associated with the list
		 * of paths we supplied on the command line
		 */
		if(!selpath(entry.path))
			continue; /* not selected */

		if(classes && !selclass(entry.class))
			continue;
		count++;
		if(ckentry((envfile ? 1 : 0), maptyp, &entry, fp))
			errflg++;
	}
	(void) fclose(fp);
	if(envfile) {
		/* free up environment resources */
		for(n=0; environ[n]; n++)
			free(environ[n]);
		free(environ);
	}
	if(!count && pkginst) {
		progerr(ERR_NOENTRIES, pkginst);
		errflg++;
	}
	if(maptyp) {
		/* make sure each listed package was associated with
		 * an entry from the prototype or pkgmap
		 */
		(void) selpkg(NULL);
	}
	if(!qflag && !lflag && !Lflag) {
		/* make sure each listed pathname was associated with
		 * an entry from the prototype or pkgmap
		 */
		(void) selpath(NULL);
	}
	return(errflg);
}

static int
selclass(class)
char *class;
{
	int i;
	for(i = 0; classes[i]; i++)
		if(!strcmp(class, classes[i]))
			return 1;
	return 0;
}
static int
selpkg(p)
char *p;
{
	static char *selected;
	register int i;

	if(p == NULL) {
		if(selected == NULL)
			return(0); /* return value not important */
		for(i=0; i < pkgcnt; ++i) {
			if(selected[i] == NULL)
				logerr(WRN_NOPKG, pkg[i]);
		}
		return(0); /* return value not important */
	} else if(pkgcnt == 0)
		return(1);
	else if(selected == NULL) {
		selected = (char *) calloc((unsigned) (pkgcnt+1), sizeof(char));
		if(selected == NULL) {
			progerr(ERR_NOMEM, errno);
			exit(99);
			/*NOTREACHED*/
		}
	}

	for(i=0; i < pkgcnt; ++i) {
		if(pkgnmchk(p, pkg[i], 0) == 0) {
			if(selected != NULL)
				selected[i] = 'b';
			return(1);
		}
	}
	return(0);
}

static int
selpath(path)
char *path;
{
	int n;

	if(!npaths)
		return(1); /* everything is selectable */

	for(n=0; n < npaths; n++) {
		if(path == NULL) {
			if(!used[n])
				logerr(WRN_NOPATH, pathlist[n]);
		} else if(!shellmatch(pathlist[n], path)) {
			used[n] = 1;
			return(1);
		}
	}
	return(0); /* not selected */
}

static int
shellmatch(spec, path)
char	*spec, *path;
{
	while(*spec && (*spec == *path)) {
		spec++, path++;
	}
	if((*spec == *path) || (*spec == '*'))
		return(0);
	return(1);
}

