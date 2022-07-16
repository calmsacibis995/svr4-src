/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/
#ident	"@(#)libpkg:spkgdir.c	1.1.3.1"
#include <stdio.h>
#include <limits.h>
#include <pkgstrct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pkgtrans.h>

extern char	*pkgdir;
extern char	*tmpnam(), *devattr();
extern void	progerr();
extern int	isdir(), 
		mkdir(),
		pkgtrans();

static char *pkgtdir = NULL;

static	char *pkg[] = {
	"all",
	NULL
};

spkgdir(device)
char *device;
{
	struct stat statbuf;
	char	*pt;

	if(device == NULL)
		return(0);
	else if((pt = devattr(device, "pathname")) && !isdir(pt)) {
		pkgdir = pt;
		return(0);
	}

	if(stat(device, &statbuf) || !(statbuf.st_mode & S_IFDIR)) {
		/* not a directory, so use pkgtrans */
		pkgtdir = pkgdir = tmpnam(NULL);
		if(!pkgdir || mkdir(pkgdir, 0700)) {
			progerr("unable to create temporary directory");
			return(1);
		}
		if(pkgtrans(device, pkgdir, pkg, PT_SILENT|PT_INFO_ONLY))
			return(1);
	} else {
		/* just a plain ole directory */
		pkgdir = device;
	}
	return(0);
}

upkgdir()
{
	char cmd[128];

	if(pkgtdir) {
		(void) sprintf(cmd, "rm -rf %s", pkgtdir);
		pkgtdir = NULL;
		if(system(cmd)) {
			progerr("unable to remove temporary directory %s",
				pkgtdir);
			return(1);
		}
	}
	return(0);
}
