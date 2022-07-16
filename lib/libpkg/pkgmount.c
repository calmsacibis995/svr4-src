/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:pkgmount.c	1.10.6.1"

#include <stdio.h>
#include <string.h>
#include <pkgdev.h>
#include <pkginfo.h>
#include <sys/types.h>
#include <devmgmt.h>
#include <sys/mount.h>

extern int	errno;
extern FILE	*epopen();
extern void	quit(),
		progerr(),
		logerr();
extern int	getvol(),
		pkgexecl();

#define CMDSIZ	256
#define ERR_FSTYP	"unable to determine fstype for <%s>"
#define MOUNT		"/sbin/mount"
#ifndef PRESVR4
#define MOUNT		"/sbin/mount"
#define UMOUNT		"/sbin/umount"
#define FSTYP		"/sbin/fstyp"
#else
#define MOUNT		"/etc/mount"
#define UMOUNT		"/etc/umount"
#define FSTYP		"/etc/fstyp"
#endif

#define LABEL0	"Insert %%v %d of %d for <%s> package into %%p."
#define LABEL1	"Insert %%v for <%s> package into %%p."
#define LABEL2	"Insert %%v %d of %d into %%p."
#define LABEL3	"Insert %%v into %%p."

int	Mntflg = 0;

int
pkgmount(devp, pkg, part, nparts, getvolflg)
struct pkgdev *devp;
char	*pkg;
int	part, nparts, getvolflg;
{
	int	n, flags;
	char	*pt, prompt[64], cmd[CMDSIZ];
	FILE	*pp;

	if(part && nparts) {
		if(pkg)
			(void) sprintf(prompt, LABEL0, part, nparts, pkg);
		else
			(void) sprintf(prompt, LABEL2, part, nparts);
	} else if(pkg)
		(void) sprintf(prompt, LABEL1, pkg);
	else
		(void) sprintf(prompt, LABEL3);

	n = 0;
	for(;;) {
		if(!getvolflg && n)
			/*
			 * Return to caller if not prompting
			 * and error was encountered.
			 */
			return -1;
		if(getvolflg && (n = getvol(devp->bdevice, NULL, 
		   (devp->rdonly ? 0 : DM_FORMFS|DM_WLABEL), prompt))) {
			if(n == 3)
				return(3);
			if(n == 2)
				progerr("unknown device <%s>", devp->bdevice);
			else
				progerr("unable to obtain package volume");
			return(99);
		}

		if(devp->fstyp == NULL) {
			(void) sprintf(cmd, "%s %s", FSTYP, devp->bdevice);
			if((pp = epopen(cmd, "r")) == NULL) {
				rpterr();
				logerr(ERR_FSTYP, devp->bdevice);
				n = -1;
				continue;
			}
			cmd[0] = '\0';
			if(fgets(cmd, CMDSIZ, pp) == NULL) {
				logerr(ERR_FSTYP, devp->bdevice);
				(void) pclose(pp);
				n = -1;
				continue;
			}
			if(epclose(pp)) {
				rpterr();
				logerr(ERR_FSTYP, devp->bdevice);
				n = -1;
				continue;
			}
			if(pt = strpbrk(cmd, " \t\n"))
				*pt = '\0';
			if(cmd[0] == '\0') {
				logerr(ERR_FSTYP, devp->bdevice);
				n = -1;
				continue;
			}	
			devp->fstyp = strdup(cmd);
		}

		if(Mntflg) {
#ifdef PRESVR4
			n = mount(devp->bdevice, devp->mount, 0);
#else
			flags = MS_FSS;
			if(devp->rdonly)
				flags |= MS_RDONLY;
			n = mount(devp->bdevice, devp->mount,
				flags, devp->fstyp);
#endif
		} else if(devp->rdonly) {
#ifdef PRESVR4
			n = pkgexecl(NULL, NULL, MOUNT, "-r", "-f", devp->fstyp,
				devp->bdevice, devp->mount, NULL);
#else
			n = pkgexecl(NULL, NULL, MOUNT, "-r", "-F", devp->fstyp,
				devp->bdevice, devp->mount, NULL);
#endif
		} else {
#ifdef PRESVR4
			n = pkgexecl(NULL, NULL, MOUNT, "-f", devp->fstyp,
				devp->bdevice, devp->mount, NULL);
#else
			n = pkgexecl(NULL, NULL, MOUNT, "-F", devp->fstyp,
				devp->bdevice, devp->mount, NULL);
#endif
		}
		if(n) {
			progerr("mount of %s failed", devp->bdevice);
			continue;
		}
		devp->mntflg++; 
		break;
	}
	return(0);
}

int
pkgumount(devp)
struct pkgdev *devp;
{
	int	n;

	if(!devp->mntflg)
		return(0);

	if(Mntflg)
		n = umount(devp->bdevice);
	else
		n = pkgexecl(NULL, NULL, UMOUNT, devp->bdevice, NULL);
	if(n == 0)
		devp->mntflg = 0;
	return(n);
}
