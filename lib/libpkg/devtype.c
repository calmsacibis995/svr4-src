/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:devtype.c	1.3.6.1"
#include <stdio.h>
#include <string.h>
#include <pkgdev.h>

extern char	*devattr();
extern void	logerr(),
		free();
extern int	isdir();
extern long	atol();

int
devtype(alias, devp)
char	*alias;
struct pkgdev *devp;
{
	char *name;
	devp->mntflg = 0;
	devp->name = alias;
	devp->dirname = devp->pathname = devp->mount = NULL;
	devp->fstyp = devp->cdevice = devp->bdevice = devp->norewind = NULL;
	devp->rdonly = 0;
	devp->capacity = 0;

	/* see if alias represents an existing file */
	if(alias[0] == '/') {
		if(!isdir(alias)) {
			devp->dirname = devp->name;
			return(0);
		}
	}

	/* see if alias represents a mountable device (e.g., a floppy) */
	if((devp->mount=devattr(alias, "mountpt")) && devp->mount[0]) {
		devp->bdevice = devattr(alias, "bdevice");
		if(!devp->bdevice || !devp->bdevice[0]) {
			if(devp->bdevice) {
				free(devp->bdevice);
				devp->bdevice = NULL;
			}
			return(-1);
		}
		devp->dirname = devp->mount;
	} else if(devp->mount) {
		free(devp->mount);
		devp->mount = NULL;
	}

	devp->cdevice = devattr(alias, "cdevice");
	if(devp->cdevice && devp->cdevice[0])  {
		/* check for capacity */
		if(name = devattr(alias, "capacity")) {
			if(name[0])
				devp->capacity = atol(name);
			free(name);
		}
		/* check for norewind device */
		devp->norewind = devattr(alias, "norewind");
		if(devp->norewind && !devp->norewind[0]) {
			free(devp->norewind);
			devp->norewind = NULL;
		}

		/* mountable devices will always have associated raw device */
		return(0);
	}
	if(devp->cdevice) {
		free(devp->cdevice);
		devp->cdevice = NULL;
	}
	/*
	 * if it is not a raw device, it must be a directory or a regular file
	 */
	name = devattr(alias, "pathname");
	if(!name || !name[0]) {
		/* Assume a regular file */
		if(name)
			free(name);
		devp->pathname = alias;
		return 0;
	}
	if(!isdir(name))
		devp->dirname = name;
	else
		devp->pathname = name;
	return(0);
}
