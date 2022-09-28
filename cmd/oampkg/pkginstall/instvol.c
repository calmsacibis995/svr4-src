/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/instvol.c	1.17.5.3"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkgdev.h>
#include <pkglocs.h>
#include "install.h"

extern struct admin 
		adm;
extern struct pkgdev
		pkgdev;
extern struct cfent 
		**eptlist;
extern struct mergstat
		*mstat;
extern char	tmpdir[],
		pkgsav[],
		pkgbin[],
		errbuf[],
		instdir[],
		*errstr,
		*pkginst,
		**class;
extern unsigned	nclass;
extern int	dbchg,
		errno,
		nosetuid,
		nocnflct,
		warnflag;

extern char	*qstrdup(),
		*srcpath();
extern struct pinfo
		*eptstat();
extern void	*calloc(),
		progerr(),
		logerr(),
		ckreturn(),
		pkgvolume(),
		backup(),
		echo(),
		quit();
extern int	access(),
		chdir(),
		cppath(),
		averify(),
		cverify(),
		finalck(),
		ocfile(),
		srchcfile(),
		swapcfile(),
		putcfile(),
		pkgexecl();

#define ck_efile(s, p)	\
		((p->cinfo.modtime >= 0) && \
		p->ainfo.local && \
		cverify(0, &p->ftype, s, &p->cinfo))

#define LSIZE	(2*PATH_MAX+1)
#define MAXCHAR		14

#define ERR_CASFAIL	"class action script did not complete successfully"
#define ERR_TMPFILE	"unable to open temp file <%s>"
#define ERR_CORRUPT	"source path <%s> is corrupt"
#define ERR_CHDIR	"unable to change directory to <%s>"
#define ERR_CFBAD	"bad entry read of contents file"
#define ERR_CFMISSING	"missing entry in contents file for <%s>"

static char	**script;
static int	eocflag;
static int	domerg();
static void	findscripts(), endofclass();
static char	*trunc();

void
instvol(srcinst, part, nparts)
char	*srcinst;
int	part;
int	nparts;
{
	FILE	*listfp;
	int	i, n, count, tcount;
	char	*listfile,
		*srcp,
		*rsrcp,
		*dstp;
	
	if(part == 1) {
		pkgvolume(&pkgdev, srcinst, part, nparts);
		findscripts();
	}

	tcount = 0;
	for(i=0; i < nclass; i++) {
		eocflag = count = 0;
		listfp = (FILE *) 0;
		for(;;) {
			n = domerg((count ? 0 : part), nparts, class[i], 
				&srcp, &dstp);
			if(n < 0)
				break; /* no more entries to process */
			if(!tcount++) {
				/* first file to install */
				echo("## Installing part %d of %d.", 
					part, nparts);
			}
			count++;
			if(script[i] && !listfp) {
				/* create list file */
				listfile = tempnam(tmpdir, "list");
				if((listfp = fopen(listfile, "w")) == NULL) {
					progerr(ERR_TMPFILE, listfile);
					quit(99);
				}
			}
			pkgvolume(&pkgdev, srcinst, part, nparts);

			/* check if can access srcp - if not,
			 * truncate all parts of the path to 14 chars
			 */
			if(srcp) {
				if(access(srcp, F_OK) == -1) {
					rsrcp = trunc(srcp);
					strcpy(srcp, rsrcp);
				}
			}
			if(script[i] || strchr("en", eptlist[n]->ftype)) {
				if(ck_efile(srcp, eptlist[n])) {
					progerr(ERR_CORRUPT, srcp);
					logerr(errbuf);
					warnflag++;
					continue;
				}
			}
			if(script[i]) {
				/* just collect names in a temporary file 
				 * that will be used as the stdin when the
				 * class action script is invoked
				 */
				(void) fprintf(listfp, "%s %s\n", 
					(srcp ? srcp: "/dev/null"), dstp);
			} else {
				if (!(ADM(list_files,"nocheck")))
					echo(dstp);
				if(srcp) {
					/* copy from source media to target
					 * path and fix file mode and
					 * permission now in case installation
					 * is halted
					 */
					if(cppath(1, srcp, dstp))
						warnflag++;
					else if(!finalck(eptlist[n], 1, 1)) {
						/* everything checks here */
						mstat[n].attrchg = 0;
						mstat[n].contchg = 0;
					}
				}
			}
		}

		/* we have now completed processing of all pathnames
		 * associated with this volume and class
		 */

		if(script[i]) {
			/* 
			 * execute appropriate class action script
			 * with list of source/destination pathnames
			 * as the input to the script
			 */
			if(chdir(pkgbin)) {
				progerr(ERR_CHDIR, pkgbin);
				quit(99);
			}
			if(listfp) 
				(void) fclose(listfp);
			if(eocflag) {
				/* since there are no more volumes which
				 * contain pathnames associated with this
				 * class, execute class action script with
				 * the ENDOFCLASS argument; we do this even
				 * if none of the pathnames associated with
				 * this class and volume needed installation
				 * to guarantee the class action script is
				 * executed at least once during package
				 * installation
				 */
				n = pkgexecl((listfp ? listfile : "/dev/null"), 
				   NULL, SHELL, script[i], "ENDOFCLASS", NULL);
				ckreturn(n, ERR_CASFAIL);
			} else if(count) {
				/* execute class action script */
				n = pkgexecl(listfile, NULL, SHELL, script[i], 
					NULL);
				ckreturn(n, ERR_CASFAIL);
			}
		}

		if(eocflag) {
			/* 
			 * finalize merg; this checks to make sure file 
			 * attributes are correct and any links specified
			 * are created
			 */
			endofclass(class[i], (script[i] ? 0 : 1));
		}
	}
	if(tcount == 0)
		echo("## Installation of part %d of %d is complete.", 
			part, nparts);
}

static void
findscripts()
{
	int	i;
	char	path[PATH_MAX];

	script = (char **) calloc(nclass, sizeof(char *));
	for(i=0; i < nclass; i++) {
		/* 
		 * locate appropriate installation class action script, if any;
		 * look on media for script, since it might be on the system due
		 * to a previous installation
		 */
		(void) sprintf(path, "%s/install/i.%s", instdir, class[i]);
		if(access(path, 0x01) == 0) {
			(void) sprintf(path, "%s/i.%s", pkgbin, class[i]);
			script[i] = qstrdup(path);
			continue;
		}

		(void) sprintf(path, "%s/i.%s", PKGSCR, class[i]);
		if(access(path, 0x01) == 0) {
			script[i] = qstrdup(path);
			continue;
		}
		script[i] = NULL;
	}
}

static int
domerg(part, nparts, myclass, srcp, dstp)
int	part, nparts;
char	*myclass;
char	**srcp, **dstp;
{
	static int	svindx = 0;
	static int	svpart = 0;
	static int	maxvol = 0;
	int	i;

	if(part) {
		maxvol = 0;
		svindx = 0;
		svpart = part;
	} else {
		i = svindx;
		part = svpart;
	}

	for(i=svindx; eptlist[i]; i++) {
		if(eptlist[i]->ftype == 'i')
			continue; /* ignore information files */
		if(strcmp(myclass, eptlist[i]->class))
			continue;
		if(eptlist[i]->volno > maxvol)
			maxvol = eptlist[i]->volno;
		if(part != eptlist[i]->volno)
			continue;
			
		if(nosetuid && (mstat[i].setuid || mstat[i].setgid)) {
			echo("%s <setuid/setgid process ignored>", 
				eptlist[i]->path);
			continue;
		}
		if(nocnflct && mstat[i].shared) {
			if(mstat[i].contchg || mstat[i].attrchg) {
				echo("%s <shared pathname ignored>", 
					eptlist[i]->path);
			}
			continue;
		}

		switch(eptlist[i]->ftype) {
		  case 'l':
			continue; /* defer to final proc */

		  case 's':
			if(averify(0, &eptlist[i]->ftype, eptlist[i]->path, 
			&eptlist[i]->ainfo))
			   if (!(ADM(list_files,"nocheck")))
				echo("%s <symbolic link>", eptlist[i]->path);
			/* fall through */
		  case 'd':
		  case 'x':
		  case 'c':
		  case 'b':
		  case 'p':
			if(averify(1, &eptlist[i]->ftype, eptlist[i]->path, 
				&eptlist[i]->ainfo) == 0) {
				mstat[i].contchg = mstat[i].attrchg = 0;
			}
		}

		if(mstat[i].contchg) {
			*dstp = eptlist[i]->path;
			if(strchr("fev", eptlist[i]->ftype)) {
				*srcp = eptlist[i]->ainfo.local;
				if(*srcp[0] == '~') {
					/* translate source pathname */
					*srcp = srcpath(instdir,
						&(eptlist[i]->ainfo.local[1]), 
						part, nparts);
				}
			} else
				*srcp = NULL;
			svindx = i+1;
			backup(*dstp, 1);
			return(i);
		}
		if(mstat[i].attrchg) {
			backup(eptlist[i]->path, 0);
			echo("%s <attribute change only>", eptlist[i]->path);
			/* fix the attributes now for robustness sake */
			if(averify(1, &eptlist[i]->ftype, eptlist[i]->path, 
				&eptlist[i]->ainfo) == 0) {
				mstat[i].attrchg = 0;
			}
		}
	}
	if(maxvol == part)
		eocflag++; /* endofclass */
	return(-1); /* no entry on this volume */
}

static void
endofclass(myclass, ckflag)
char	*myclass;
int	ckflag;
{
	struct cfent entry;
	struct pinfo *pinfo;
	int	n, indx, flag;
	FILE	*fp;
	FILE	*fpo;

	if(ocfile(&fp, &fpo))
		quit(99);

	echo("[ verifying class <%s> ]", myclass);

	entry.pinfo = NULL;
	for(indx=0; ;indx++) {
		/* find next package object in this class */
		while(eptlist[indx]) {
			if((eptlist[indx]->ftype != 'i') && 
			   !strcmp(myclass, eptlist[indx]->class))
				break;
			indx++;
		}

		n = srchcfile(&entry, 
			(eptlist[indx] ? eptlist[indx]->path : NULL), fp, fpo);

		if(n == 0)
			break;
		else if(n < 0) {
			progerr(ERR_CFBAD);
			logerr("pathname=%s", entry.path);
			logerr("problem=%s", errstr);
			quit(99);
		} else if(n != 1) {
			progerr(ERR_CFMISSING, eptlist[indx]->path);
			quit(99);
		} 

		/*
		 * validate this entry and change the status
		 * flag in the 'contents' file
		 */
		if(eptlist[indx]->ftype == '-')
			(void) eptstat(&entry, pkginst, '@');
		else {
			if(eptlist[indx]->ftype == 'l') {
				if(averify(0, &eptlist[indx]->ftype, 
				eptlist[indx]->path, &eptlist[indx]->ainfo)) {
					if (!(ADM(list_files,"nocheck")))
					     echo("%s <linked pathname>", 
						eptlist[indx]->path);
					mstat[indx].attrchg++;
				}
			}

			flag = finalck(eptlist[indx], mstat[indx].attrchg, 
				(ckflag ? mstat[indx].contchg : (-1)));

			pinfo = entry.pinfo;
			while(pinfo) {
				if(!strcmp(pkginst, pinfo->pkg))
					break;
				pinfo = pinfo->next;
			}
			if(pinfo)
				pinfo->status = (flag ? '!' : '\0');
		}
		if(entry.npkgs) {
			if(putcfile(&entry, fpo))
				quit(99);
		}
	}
	(void) fclose(fp);
	if(swapcfile(fpo, (dbchg ? pkginst : NULL)))
		quit(99);
}

char 
*trunc(f1)
char *f1;
{
	static char	truncp[PATH_MAX];
	char		*path;

	truncp[0] = '\0';
	path = strtok(f1, "/");
	while(path) {
		strcat(truncp, "/");
		strncat(truncp, path, MAXCHAR);
		path = strtok(NULL, "/");
	}
	
	return(truncp);
}
