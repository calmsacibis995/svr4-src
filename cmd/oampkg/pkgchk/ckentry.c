/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgchk/ckentry.c	1.2.6.1"

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include "install.h"

extern int	Lflag, lflag,
		aflag, cflag,
		fflag, qflag,
		nflag, xflag,
		vflag;
extern char	*errstr,
		*basedir,
		*device,
		pkgspool[],
		errbuf[];

extern void	mappath(),
		logerr(),
		progerr(),
		basepath(),
		mappath(),
		mapvar(),
		canonize(),
		tputcfent(),
		free(),
		exit();
extern int	access(),
		unlink(),
		putcfile(),
		gpkgmap(),
		srchcfile(),
		averify(),
		cverify();

#define nxtentry(p) \
		(maptyp ? srchcfile(p, "*", fp, NULL) : gpkgmap(p, fp))

#define ERR_SPOOLED	"unable to locate spooled object <%s>"

static int	xdir();
static char	*findspool();

int
ckentry(envflag, maptyp, ept, fp)
int	envflag, maptyp;
struct cfent	*ept;
FILE	*fp;
{
	int	a_err, c_err,
		errflg;
	char	*path;

	if(envflag && (ept->ftype != 'i')) {
		mappath(2, ept->path);
		basepath(ept->path, basedir);
	}
	canonize(ept->path);
	if(strchr("sl", ept->ftype)) {
		if(envflag) {
			mappath(2, ept->ainfo.local);
			if(strchr("l", ept->ftype) ||
			   (!isdot(ept->ainfo.local) &&
			    !isdotdot(ept->ainfo.local)))
				basepath(ept->ainfo.local, basedir);
		}
		if(strchr("l", ept->ftype) ||
		   (!isdot(ept->ainfo.local) &&
		    !isdotdot(ept->ainfo.local)))
			canonize(ept->ainfo.local);
	}
	if(envflag) {
		if(!strchr("isl", ept->ftype)) {
			mapvar(2, ept->ainfo.owner);
			mapvar(2, ept->ainfo.group);
		}
	}

	if(lflag) {
		tputcfent(ept, stdout);
		return(0);
	} else if(Lflag)
		return(putcfile(ept, stdout));

	errflg = 0;
	if(device) {
		if(strchr("dxslcbp", ept->ftype))
			return(0);
		if((path = findspool(ept)) == NULL) {
			progerr(ERR_SPOOLED, ept->path);
			return(-1);
		}
		c_err = cverify(0, &ept->ftype, path, &ept->cinfo);
		if(c_err) {
			logerr("ERROR: %s", path);
			logerr(errbuf);
			return(-1);
		}
	} else {
		a_err = 0;
		if(aflag && !strchr("in", ept->ftype)) {
			/* validate attributes */
			if(a_err = averify(fflag, &ept->ftype, 
			  ept->path, &ept->ainfo)) {
				errflg++;
				if(!qflag || (a_err != VE_EXIST)) {
					logerr("ERROR: %s", ept->path);
					logerr(errbuf);
				}
				if(a_err == VE_EXIST)
					return(-1);
			}
		}
		if(cflag && strchr("fev", ept->ftype) &&
		  (!nflag || ept->ftype != 'v')) {
			/* validate contents */
			if(c_err = cverify(fflag, &ept->ftype, 
			  ept->path, &ept->cinfo)) {
				errflg++;
				if(!qflag || (c_err != VE_EXIST)) {
					if(!a_err) 
						logerr("ERROR: %s", ept->path);
					logerr(errbuf);
				}
				if(c_err == VE_EXIST)
					return(-1);
			}
		}
		if(xflag && (ept->ftype == 'x')) {
			/* must do verbose here since ept->path will change */
			path = strdup(ept->path);
			if(xdir(maptyp, fp, path))
				errflg++;
			(void) strcpy(ept->path, path);
			free(path);
		}
	}
	if(vflag) 
		(void) fprintf(stderr, "%s\n", ept->path);
	return(errflg);
}

static int
xdir(maptyp, fp, dirname)
int	maptyp;
FILE	*fp;
char	*dirname;
{
	struct dirent *drp;
	struct cfent mine;
	struct pinfo *pinfo;
	DIR	*dirfp;
	long	pos;
	int	n, len, dirfound,
		errflg;
	char	badpath[PATH_MAX+1];

	pos = ftell(fp);

	if((dirfp = opendir(dirname)) == NULL) {
		progerr("unable to open directory <%s>", dirname);
		return(-1);
	}
	len = strlen(dirname);

	errflg = 0;
	(void) memset((char *)&mine, '\0', sizeof(struct cfent));
	while((drp = readdir(dirfp)) != NULL) {
		if(!strcmp(drp->d_name, ".") || !strcmp(drp->d_name, ".."))
			continue;
		dirfound = 0;
		while(n = nxtentry(&mine)) {
			if(n < 0) {
				logerr("ERROR: garbled entry");
				logerr("pathname: %s", mine.path);
				logerr("problem: %s", errstr);
				exit(99);
			}
			if(strncmp(mine.path, dirname, len) || 
			(mine.path[len] != '/'))
				break;
			if(!strcmp(drp->d_name, &mine.path[len+1])) {
				dirfound++;
				break;
			}
		}
		if(fseek(fp, pos, 0)) {
			progerr("unable to reset file position from xdir()");
			exit(99);
		}
		if(!dirfound) {
			(void) sprintf(badpath, "%s/%s", dirname, drp->d_name);
			if(fflag) {
				if(unlink(badpath)) {
					errflg++;
					logerr("ERROR: %s", badpath);
					logerr("unable to remove hidden file");
				}
			} else {
				errflg++;
				logerr("ERROR: %s", badpath);
				logerr("hidden file in exclusive directory");
			}
		}
	}

	if(maptyp) {
		/* clear memory we've used */
		while(pinfo = mine.pinfo) {
			mine.pinfo = pinfo->next;
			free((char *)pinfo);
		}
	}

	(void) closedir(dirfp);
	return(errflg);
}

static char *
findspool(ept)
struct cfent *ept;
{
	static char path[2*PATH_MAX+1];
	char host[PATH_MAX+1];

	(void) strcpy(host, pkgspool);
	if(ept->ftype == 'i') {
		if(strcmp(ept->path, "pkginfo"))
			(void) strcat(host, "/install");
	} else if(ept->path[0] == '/')
		(void) strcat(host, "/root");
	else
		(void) strcat(host, "/reloc");

	(void) sprintf(path, "%s/%s", host, 
		ept->path + (ept->path[0] == '/')); 
	if(access(path, 0) == 0)
		return(path);

	if((ept->ftype != 'i') && (ept->volno > 0)) {
		(void) sprintf(path, "%s.%d/%s", host, ept->volno,
			ept->path + (ept->path[0] == '/')); 
		if(access(path, 0) == 0)
			return(path);
	}
	return(NULL);
}

