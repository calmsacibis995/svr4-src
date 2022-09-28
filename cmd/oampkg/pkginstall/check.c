/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/check.c	1.13.4.1"
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <utmp.h>
#include <dirent.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkglocs.h>
#include "install.h"

extern struct admin adm;
extern struct cfent
		**eptlist;
extern struct mergstat
		*mstat;
extern int	errno, ckquit,
		nointeract, nocnflct,
		nosetuid, rprcflag;
extern char	ilockfile[], rlockfile[],
		instdir[], savlog[],
		tmpdir[], pkgloc[],
		pkgbin[], pkgsav[],
		errbuf[], 
		*pkginst, *msgtext;

extern char	*qstrdup(),
		*getenv();
extern struct utmp 
		*getutid();
extern void	progerr(), logerr(),
		echo(), ptext(),
		quit(), free();
extern int	access(), mkdir(),
		ckyorn(), dockdeps(),
		dockspace(), creat(),
		close(), cverify();

#define DISPSIZ	20	/* number of items to display on one page */

#define MSG_RUNLEVEL \
	"\\nThe current run-level of this machine is <%s>, \
	which is not a run-level suggested for installation of \
	this package.  Suggested run-levels (in order of preference) include:"
#define HLP_RUNLEVEL \
	"If this package is not installed in a run-level which has been \
	suggested, it is possible that the package may not install or \
	operate properly.  If you wish to follow the run-level suggestions, \
	answer 'n' to stop installation of the package."
#define MSG_STATECHG \
	"\\nTo change states, execute\\n\\tshutdown -y -i%s -g0\\n\
	after exiting the installation process. \
	Please note that after changing states you \
	may have to mount appropriate filesystem(s) \
	in order to install this package."

#define ASK_CONFLICT	"Do you want to install these conflicting files"
#define MSG_CONFLICT \
	"\\nThe following files are already installed on the system and \
	are being used by another package:"
#define HLP_CONFLICT \
	"If you choose to install conflicting files, the files listed above \
	will be overwritten and/or have their access permissions changed.  \
	If you choose not to install these files, installation will proceed \
	but these specific files will not be installed.  Note that sane \
	operation of the software being installed may require these files \
	be installed; thus choosing to not to do so may cause inapropriate \
	operation.  If you wish to stop installation of this package, \
	enter 'q' to quit."

#define ASK_SETUID	"Do you want to install these setuid/setgid files"
#define MSG_SETUID \
	"\\nThe following files are being installed with setuid and/or setgid \
	permissions or are overwriting files which are currently setuid/setgid:"
#define HLP_SETUID \
	"The package being installed appears to contain processes which will \
	have their effective user or group ids set upon execution.  History \
	has shown that these types of processes can be a source of major \
	security impact on your system. \
	If you choose not to install these files, installation will proceed \
	but these specific files will not be installed.  Note that sane \
	operation of the software being installed may require these files \
	be installed; thus choosing to not to do so may cause inapropriate \
	operation.  If you wish to stop installation of this package, \
	enter 'q' to quit."

#define MSG_PARTINST \
	"\\nThe installation of this package was previously terminated \
	and installation was never successfully completed."
#define MSG_PARTREM \
	"\\nThe removal of this package was terminated at some point \
	in time, and package removal was only partially completed."
#define HLP_PARTIAL \
	"Installation of partially installed packages is normally allowable, \
	but some packages providers may suggest that a partially installed \
	package be completely removed before re-attempting installation. \
	Check the documentation provided with this package, and then answer \
	'y' if you feel it is advisable to continue the installation process."

#define HLP_SPACE \
	"It appears that there is not enough free space on your system in \
	which to install this package.  It is possible that one or more \
	filesystems are not properly mounted.  Neither installation of the \
	package nor its operation can be guaranteed under these conditions. \
	If you choose to disregard this warning, enter 'y' to continue the \
	installation process."
#define HLP_DEPEND \
	"The package being installed has indicated a dependency on \
	the existence (or non-existence) of another software package. \
	If this dependency is not met before continuing, the package \
	may not install or operate properly.  If you wish to disregard \
	this dependency, answer 'y' to continue the installation process."

#define MSG_PRIV \
	"\\nThis package contains scripts which will be executed with \
	super-user permission during the process of installing this package."
#define HLP_PRIV \
	"During the installtion of this package, certain scripts provided \
	with the package will execute with super-user permission.  These \
	scripts may modify or otherwise change your system without your \
	knowledge.  If you are certain of the origin and trustworthiness \
	of the package being installed, answer 'y' to continue the \
	installation process."

#define ASK_CONT \
	"Do you want to continue with the installation of this package"
#define HLP_CONT \
	"If you choose 'y', installation of this package will continue.  \
	If you want to stop installation of this package, choose 'n'."

static int	mkpath();
void
ckpartial()
{
	char	ans[2];
	int	n;

	if(ADM(partial, "nocheck"))
		return;

	if(access(ilockfile, 0) == 0) {
		msgtext = MSG_PARTINST;
		ptext(stderr, msgtext);
		if(ADM(partial, "quit"))
			quit(4);
		if(nointeract)
			quit(5);
		msgtext = NULL;

		ckquit = 0;
		if(n = ckyorn(ans, NULL, NULL, HLP_PARTIAL, ASK_CONT))
			quit(n);
		if(ans[0] != 'y')
			quit(3);
		ckquit = 1;
	}

	if(access(rlockfile, 0) == 0) {
		msgtext = MSG_PARTREM;
		ptext(stderr, msgtext);
		if(ADM(partial, "quit"))
			quit(4);
		if(nointeract)
			quit(5);
		msgtext = NULL;

		ckquit = 0;
		if(n = ckyorn(ans, NULL, NULL, HLP_PARTIAL, ASK_CONT))
			quit(n);
		if(ans[0] != 'y')
			quit(3);
		ckquit = 1;
	}
}

void
ckrunlevel()
{
	struct utmp utmp;
	struct utmp *putmp;
	char	ans[2], *pt, *istates, *pstate;
	int	n;
	char	*uxstate;

	if(ADM(runlevel, "nocheck"))
		return;

	pt = getenv("ISTATES");
	if(pt == NULL)
		return;

	utmp.ut_type = RUN_LVL;
	putmp = getutid(&utmp);
	if(putmp == NULL) {
		progerr("unable to determine current run-state");
		quit(99);
	}
	uxstate = strtok(&putmp->ut_line[10], " \t\n");
	
	istates = qstrdup(pt);
	if((pt = strtok(pt, " \t\n,")) == NULL)
		return; /* no list is no list */
	pstate = pt;
	do {
		if(!strcmp(pt, uxstate)) {
			free(istates);
			return;
		}
	} while(pt = strtok(NULL, " \t\n,"));

	msgtext = MSG_RUNLEVEL;
	ptext(stderr, msgtext, uxstate);
	pt = strtok(istates, " \t\n,");
	do {
		ptext(stderr, "\\t%s", pt);
	} while(pt = strtok(NULL, " \t\n,"));
	free(istates);
	if(ADM(runlevel, "quit"))
		quit(4);
	if(nointeract)
		quit(5);
	msgtext = NULL;

	ckquit = 0;
	if(n = ckyorn(ans, NULL, NULL, HLP_RUNLEVEL, ASK_CONT))
		quit(n);
	ckquit = 1;

	if(ans[0] == 'n') {
		ptext(stderr, MSG_STATECHG, pstate);
		quit(3);
	} else if(ans[0] != 'y')
		quit(3);
}

void
ckdepend()
{
	int	n;
	char	ans[2];
	char	path[PATH_MAX];

	if(ADM(idepend, "nocheck"))
		return;

	(void) sprintf(path, "%s/install/depend", instdir);
	if(access(path, 0))
		return; /* no dependency file provided by package */

	echo("## Verifying package dependencies.");
	if(dockdeps(path, 0)) {
		msgtext = "Dependency checking failed.";
		if(ADM(idepend, "quit"))
			quit(4);
		if(nointeract)
			quit(5);
		msgtext = NULL;

		ckquit = 0;
		if(n = ckyorn(ans, NULL, NULL, HLP_DEPEND, ASK_CONT))
			quit(n);
		if(ans[0] != 'y')
			quit(3);
		ckquit = 1;
	}
}

void
ckspace()
{
	int	n;
	char	ans[2];
	char	path[PATH_MAX];

	if(ADM(space, "nocheck"))
		return;

	echo("## Verifying disk space requirements.");
	(void) sprintf(path, "%s/install/space", instdir);
	if(access(path, 0) == 0)
		n = dockspace(path);
	else
		n = dockspace(NULL);

	if(n) {
		msgtext = "Space checking failed.";
		if(ADM(space, "quit"))
			quit(4);
		if(nointeract)
			quit(5);
		msgtext = NULL;

		ckquit = 0;
		if(n = ckyorn(ans, NULL, NULL, HLP_SPACE, ASK_CONT))
			quit(n);
		if(ans[0] != 'y')
			quit(3);
		ckquit = 1;
	}
}

void
ckdirs()
{
	char	path[PATH_MAX];

 	if(mkpath(PKGADM)) {
		progerr("unable to make packaging directory <%s>", PKGADM);
		quit(99);
	}
	(void) sprintf(path, "%s/admin", PKGADM);
 	if(mkpath(path)) {
		progerr("unable to make packaging directory <%s>", path);
		quit(99);
	}
	(void) sprintf(path, "%s/logs", PKGADM);
 	if(mkpath(path)) {
		progerr("unable to make packaging directory <%s>", path);
		quit(99);
	}
 	if(mkpath(PKGSCR)) {
		progerr("unable to make packaging directory <%s>", PKGSCR);
		quit(99);
	}
	if(mkpath(PKGLOC)) {
		progerr("unable to make packaging directory <%s>", PKGLOC);
		quit(99);
	}
}

void
ckpkgdirs()
{
 	if(mkpath(pkgloc)) {
		progerr("unable to make packaging directory <%s>", pkgloc);
		quit(99);
	}
 	if(mkpath(pkgbin)) {
		progerr("unable to make packaging directory <%s>", pkgbin);
		quit(99);
	}
 	if(mkpath(pkgsav)) {
		progerr("unable to make packaging directory <%s>", pkgsav);
		quit(99);
	}
}

static int
mkpath(p)
char *p;
{
	char	*pt;

	pt = (*p == '/') ? p+1 : p;
	do {
		if(pt = strchr(pt, '/'))
			*pt = '\0';
		if(access(p, 0) && mkdir(p, 0755))
			return(-1);
		if(pt)
			*pt++ = '/';
	} while(pt);
	return(0);
}

void
ckconflct()
{
	int	i, n, count;
	char	ans[2];

	if(ADM(conflict, "nochange")) {
		nocnflct++;
		return;
	} else if(ADM(conflict, "nocheck"))
		return;

	echo("## Checking for conflicts with packages already installed.");
	count = 0;
	for(i=0; eptlist[i]; i++) {
		if(!mstat[i].shared)
			continue;
		if(mstat[i].contchg) {
			if(!count++) 
				ptext(stderr, MSG_CONFLICT);
			else if(!nointeract && ((count % DISPSIZ) == 0)) {
				echo("[Hit any key to continue display]");
				(void) getc(stdin);
			}
			echo("\t%s", eptlist[i]->path);
		} else if(mstat[i].attrchg) {
			if(!count++) 
				ptext(stderr, MSG_CONFLICT);
			else if(!nointeract && ((count % DISPSIZ) == 0)) {
				echo("[Hit any key to continue display]");
				(void) getc(stdin);
			}
			echo("\t%s <attribute change only>", eptlist[i]->path);
		}
	}

	if(count) {
		msgtext="Conflict checking failed.";
		if(ADM(conflict, "quit"))
			quit(4);
		if(nointeract)
			quit(5);
		msgtext = NULL;

		if(n = ckyorn(ans, NULL, NULL, HLP_CONFLICT, ASK_CONFLICT))
			quit(n);
		if(ans[0] == 'n') {
			ckquit = 0;
			if(n = ckyorn(ans, NULL, NULL, HLP_CONT, ASK_CONT))
				quit(n);
			if(ans[0] != 'y')
				quit(3);
			ckquit = 1;
			nocnflct++;
			rprcflag++;
		}
	}
}

void
cksetuid()
{
	int	i, n, count;
	char	ans[2];

	if(ADM(setuid, "nocheck"))
		return;

	if(ADM(setuid, "nochange")) {
		nosetuid++;
		return;
	}

	echo("## Checking for setuid/setgid programs.");
	count = 0;
	for(i=0; eptlist[i]; i++) {
		if(mstat[i].setuid) {
			if(!count++) 
				ptext(stderr, MSG_SETUID);
			else if(!nointeract && ((count % DISPSIZ) == 0)) {
				echo("[Hit any key to continue display]");
				(void) getc(stdin);
			}
			echo("\t%s <setuid %s>", eptlist[i]->path, 
				eptlist[i]->ainfo.owner);
		}
		if(mstat[i].setgid) {
			if(!count++) 
				ptext(stderr, MSG_SETUID);
			else if(!nointeract && ((count % DISPSIZ) == 0)) {
				echo("[Hit any key to continue display]");
				(void) getc(stdin);
			}
			echo("\t%s <setgid %s>", eptlist[i]->path, 
				eptlist[i]->ainfo.group);
		}
	}

	if(count) {
		msgtext="Setuid/setgid processes detected.";
		if(ADM(setuid, "quit"))
			quit(4);
		if(nointeract)
			quit(5);
		msgtext = NULL;

		if(n = ckyorn(ans, NULL, NULL, HLP_SETUID, ASK_SETUID))
			quit(n);
		if(ans[0] == 'n') {
			ckquit = 0;
			if(n = ckyorn(ans, NULL, NULL, HLP_CONT, ASK_CONT))
				quit(n);
			if(ans[0] != 'y')
				quit(3);
			ckquit = 1;
			nosetuid++;
			rprcflag++;
		}
	}
}

void
ckpriv()
{
	struct dirent *dp;
	DIR	*dirfp;
	int	n, found;
	char	ans[2], path[PATH_MAX];

	if(ADM(action, "nocheck"))
		return;
	
	(void) sprintf(path, "%s/install", instdir);
	if((dirfp = opendir(path)) == NULL) 
		return;

	found = 0;
	while((dp = readdir(dirfp)) != NULL) {
		if(!strcmp(dp->d_name, "preinstall") ||
		   !strcmp(dp->d_name, "postinstall") ||
		   !strncmp(dp->d_name, "i.", 2)) {
			found++;
			break;
		}
	}
	(void) closedir(dirfp);

	if(found) {
		ptext(stderr, MSG_PRIV);
		msgtext = "Package scripts were found.";
		if(ADM(action, "quit"))
			quit(4);
		if(nointeract)
			quit(5);
		msgtext = NULL;

		ckquit = 0;
		if(n = ckyorn(ans, NULL, NULL, HLP_PRIV, ASK_CONT))
			quit(n);
		if(ans[0] != 'y')
			quit(3);
		ckquit = 1;
	}
}

void
ckpkgfiles()
{
	register int i;
	struct cfent	*ept;
	int	errflg;
	char	source[PATH_MAX];

	errflg = 0;
	for(i=0; eptlist[i]; i++) {
		ept = eptlist[i];
		if(ept->ftype != 'i')
			continue;

		if(ept->ainfo.local) {
			(void) sprintf(source, "%s/%s", instdir, 
				ept->ainfo.local);
		} else if(!strcmp(ept->path, PKGINFO)) {
			(void) sprintf(source, "%s/%s", instdir, ept->path);
		} else {
			(void) sprintf(source, "%s/install/%s", instdir, 
				ept->path);
		}
		if(cverify(0, &ept->ftype, source, &ept->cinfo)) {
			errflg++;
			progerr("packaging file <%s> is corrupt", source);
			logerr(errbuf);
		}
	}
	if(errflg)
		quit(99);
}
