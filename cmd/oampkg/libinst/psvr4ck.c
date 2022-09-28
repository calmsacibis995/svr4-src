/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:libinst/psvr4ck.c	1.7.5.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#ifdef PRESVR4
#include <sys/types.h>
#endif /* PRESVR4 */
#include <dirent.h>
#include <ctype.h>
#include <sys/utsname.h>

extern void	progerr(),
		quit(),
		ptext(),
		echo();
extern int	access(),
		ckyorn();
extern char	*qstrdup();

#define MAILCMD		"/bin/mail"
#define ERR_MAIL	"unable to send electronic mail notification"
#define ERR_OVERWRITE	"unable to determine overwrite list"
#define ERR_PIPE	"unable to open pipe to process <%s>"
#define ASK_CONT	"Do you want to continue processing this package"
#define MSG_CONFLICT \
	"The following files are currently being used by other \
	packages on the system, and may be overwritten by the \
	installation of this pre-SVR4 package:"
#define HLP_CONFLICT \
	"If you choose to continue installation, it is possible that you \
	will overwrite files which are part of another package that is \
	already installed on the system.  If you want to assure that the \
	files are not overwritten, answer 'n' to stop the installation \
	process."
#define MSG_NOTVER \
	"The media being processed is in an old (pre-SVR4) \
	format and it is not possible to verify that \
	the inserted media belongs to the <%s> package."
#define HLP_NOTVER \
	"If you choose to continue installation, it is possible that you \
	will install the wrong package.  If you are sure the media being \
	installed contains the package you wish to install, answer 'y' \
	to continue the installation process."
#define MSG_CONFIRM \
	"The media being processed is in an old (pre-SVR4) \
	format and appears to be part of the <%s> package."
#define HLP_CONFIRM \
	"The installation of older-style (pre-SVR4) packages is, in general, \
	not as robust as installing standard packages.  Older packages may \
	attempt things during installation which overwrite existing files \
	or otherwise modify the system without your approval.  If you wish \
	to allow installation of identified pre-SVR4 package, answer 'y' to \
	continue the installation process."

static char	*Rlist[] = {
	"/install/install/Rlist",
	"/install/install/RLIST",
	"/install/install/rlist",
	NULL
};
static char	ckcmd[] = "/usr/sbin/pkgchk -L -i %s";

void
psvr4pkg(ppkg)
char	**ppkg;
{
	struct dirent *drp;
	DIR	*dirfp;
	char	*pt;
	int	n;
	char	ans, path[PATH_MAX];

	if(*ppkg) {
		(void) sprintf(path, "/install/new/usr/options/%s.name", *ppkg);
		if(access(path, 0)) {
			ptext(stderr, MSG_NOTVER, *ppkg);
			if(n = ckyorn(&ans, NULL, NULL, HLP_NOTVER, ASK_CONT))
				quit(n);
			if(ans != 'y')
				quit(3);
		}
		return;
	}

	if(dirfp = opendir("/install/new/usr/options")) {
		while(drp = readdir(dirfp)) {
			if(drp->d_name[0] == '.')
				continue;
			if(pt = strchr(drp->d_name, '.')) {
				if(!strcmp(pt, ".name")) {
					*pt = '\0';
					*ppkg = qstrdup(drp->d_name);
					break;
				}
			}
		}
		(void) closedir(dirfp);
	}

	if(*ppkg) {
		ptext(stderr, MSG_CONFIRM, *ppkg);
		if(n = ckyorn(&ans, NULL, NULL, HLP_CONFIRM, ASK_CONT))
			quit(n);
	} else {
		ptext(stderr, MSG_NOTVER, *ppkg);
		if(n = ckyorn(&ans, NULL, NULL, HLP_NOTVER, ASK_CONT))
			quit(n);
	}
	if(ans != 'y')
		quit(3);
}

void
psvr4cnflct()
{
	FILE	*pp;
	int	n, found;
	char	*pt,
		ans,
		cmd[PATH_MAX+sizeof(ckcmd)],
		path[PATH_MAX];

	for(n=0; Rlist[n] != NULL; n++) {
		if(access(Rlist[n], 0) == 0)
			break;
	}
	if(Rlist[n] == NULL)
		return; /* Rlist file not found on device */

	(void) sprintf(cmd, ckcmd, Rlist[n]);
	echo("## Checking for conflicts with installed packages");
	echo("   (using %s provided by pre-SVR4 package)", Rlist[n]);
	if((pp = popen(cmd, "r")) == NULL) {
		progerr(ERR_PIPE, cmd);
		progerr(ERR_OVERWRITE);
		quit(99);
	}

	found = 0;
	while(fgets(path, PATH_MAX, pp)) {
		if(!found++)
			ptext(stderr, MSG_CONFLICT);
		if(pt = strpbrk(path, " \t\n"))
			*pt = '\0';
		echo("\t%s", path);
	}
	if(pclose(pp)) {
		progerr(ERR_OVERWRITE);
		quit(99);
	}

	if(found) {
		if(n = ckyorn(&ans, NULL, NULL, HLP_CONFLICT, ASK_CONT))
			quit(n);
		if(ans != 'y')
			quit(3);
	}
}

void
psvr4mail(list, msg, retcode, pkg)
char	*list, *msg;
int	retcode;
char	*pkg;
{
	struct utsname utsbuf;
	FILE	*pp;
	char	cmd[BUFSIZ];

	if(list == NULL)
		return;
		
	while(isspace(*list))
		list++;
	if(*list == '\0')
		return;

	/* send e-mail notifications */
	(void) sprintf(cmd, "%s %s", MAILCMD, list);
	if((pp = popen(cmd, "w")) == NULL) {
		progerr(ERR_PIPE, MAILCMD);
		progerr(ERR_MAIL);
		quit(99);
	}

	(void) strcpy(utsbuf.nodename, "(unknown)");
	(void) uname(&utsbuf);
	ptext(pp, msg, pkg, utsbuf.nodename, retcode);

	if(pclose(pp)) {
		progerr(ERR_MAIL);
		quit(99);
	}
}
