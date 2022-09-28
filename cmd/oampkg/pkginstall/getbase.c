/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/getbase.c	1.8.4.1"

#include <stdio.h>
#include <limits.h>
#include <valtools.h>
#include <string.h>
#include "install.h"

extern struct admin
		adm;

extern char	*msgtext,
		pkgloc[],
		*pkginst;

extern int	nointeract;

extern void	progerr(),
		quit(),
		echo(),
		ptext(),
		putparam(),
		mappath();
extern int	isdir(),
		ckyorn(),
		ckpath(),
		mkdir();
extern char	*qstrdup(),
		*pkgparam(),
		*getenv();

#define MSG_REQBASEDIR \
"Installation of this package requires a base directory."

#define MSG_MUSTEXIST \
"\\nThe selected base directory <%s> must exist \
before installation is attempted."

#define MSG_YORNPRMPT \
"Do you want this directory created now"

#define MSG_PROMPT \
"Enter path to package base directory"

#define MSG_HELP \
"Installation of this package requires that \
a UNIX directory be available for installation \
of appropriate software.  This directory may \
be part of any mounted filesystem, or may \
itself be a mount point.  In general, it is \
unwise to select a base directory which already \
contains other files and/or directories."

char	*basedir;

static void	mkbasedir();

void
getbase() 
{
	char	path[PATH_MAX];
	int	n;

	/* check to see if the package BASEDIR was
	 * already defined during a previous installation
	 * of this package instance
	 */
	basedir = pkgparam(pkginst, "BASEDIR");
	if((basedir != NULL) && basedir[0]) {
		mkbasedir(0);	/* no interaction */
		return;
	}

	/* check to see if the package developer thought
	 * this package was relocatable by providing a
	 * default value for BASEDIR
	 */
	basedir = getenv("BASEDIR");
	if((basedir == NULL) || !basedir[0])
		return;

	/* check package administration to see if
	 * the basedir admin parameter was provided
	 */
	if((adm.basedir != NULL) && strcmp(adm.basedir, "ask")) {
		if(strcmp(adm.basedir, "default")) {
			basedir = adm.basedir;
			mappath(2, basedir); /* expand any package variables */
		}
		mkbasedir(0);
		return;
	} else if(nointeract) {
		msgtext = MSG_REQBASEDIR;
		ptext(stderr, msgtext);
		quit(5);
	}

	mappath(2, basedir); /* expand any package variables */

	/* interact with user to determine base directory */
	if(n = ckpath(path, P_ABSOLUTE|P_DIR|P_WRITE, basedir, 
	   NULL, MSG_HELP, MSG_PROMPT))
		quit(n);
	basedir = qstrdup(path);
	mkbasedir(1);
}

static void
mkbasedir(flag) 
int	flag;
{
	char	ans;
	int	n;

	if(isdir(basedir)) {
		if(flag) {
			/* interact to confirm directory creation */
			ptext(stderr, MSG_MUSTEXIST, basedir);
			if(nointeract)
				quit(5);

			if(n = ckyorn(&ans, NULL, NULL, NULL, MSG_YORNPRMPT))
				quit(n);
			if(ans != 'y')
				quit(3);
		}
		if(mkdir(basedir, 0755)) {
			progerr("unable to make directory <%s>", basedir);
			quit(99);
		}
	}

	echo("Using <%s> as the package base directory.", basedir);
	putparam("BASEDIR", basedir);
}
