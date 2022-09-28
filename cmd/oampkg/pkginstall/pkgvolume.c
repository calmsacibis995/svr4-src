/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/pkgvolume.c	1.5.4.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/mount.h>
#include <pkgdev.h>

extern char	instdir[],
		pkgbin[];

extern void	progerr(),
		quit();
extern int	chdir(),
		pkgumount(),
		pkgmount(),
		ckvolseq();

void
pkgvolume(devp, pkg, part, nparts)
struct pkgdev *devp;
char	*pkg;
int	part;
int	nparts;
{
	static int	cpart = 0;
	char	path[PATH_MAX];
	int	n;

	if(devp->cdevice)
		return;
	if(cpart == part)
		return;
	cpart = part;

	if(part == 1) {
		if(ckvolseq(instdir, 1, nparts)) {
			progerr("corrupt directory structure");
			quit(99);
		}
		cpart = 1;
		return;
	}

	if(devp->mount == NULL) {
		if(ckvolseq(instdir, part, nparts)) {
			progerr("corrupt directory structure");
			quit(99);
		}
		return;
	}

	for(;;) {
		(void) chdir("/");
		if(n = pkgumount(devp)) {
			progerr("attempt to unmount <%s> failed (%d)", 
				devp->bdevice, n);
			quit(99);
		}
		if(n = pkgmount(devp, pkg, part, nparts, 1))
			quit(n);
		(void) sprintf(path, "%s/%s", devp->dirname, pkg);
		if(ckvolseq(path, part, nparts) == 0)
			break;
	}
}
