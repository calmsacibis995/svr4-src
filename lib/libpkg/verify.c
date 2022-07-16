/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:verify.c	1.21.4.1"

#include <stdio.h>
#include <limits.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/mkdev.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <varargs.h>
#include <pkgstrct.h>

extern struct group	*getgrnam(), *getgrgid();
extern struct passwd	*getpwnam(), *getpwuid();
extern void		setgrent(), setpwent();
extern int	cftime(),
		link(),
		unlink(),
		lstat(),
		readlink(),
		mkdir(),
		symlink(),
		chmod(),
		chown();

#define WDMSK 0177777L
#define BUFSIZE 512
#define DATEFMT	"%D %r"
#define TDELTA 15*60

#define ERR_UNKNOWN	"unable to determine object type"
#define ERR_EXIST	"pathname does not exist"
#define ERR_FTYPE	"file type <%c> expected <%c> actual"
#define ERR_LINK	"pathname not properly linked to <%s>"
#define ERR_SLINK	"pathname not symbolically linked to <%s>"
#define ERR_MTIME	"modtime <%s> expected <%s> actual"
#define ERR_SIZE	"file size <%ld> expected <%ld> actual"
#define ERR_CKSUM	"file cksum <%ld> expected <%ld> actual"
#define ERR_MAJMIN	"major/minor device <%d,%d> expected <%d,%d> actual"
#define ERR_PERM	"permissions <%04o> expected <%04o> actual"
#define ERR_GROUP	"group name <%s> expected <%s> actual"
#define ERR_OWNER	"owner name <%s> expected <%s> actual"
#define ERR_MODFAIL	"unable to fix modification time"
#define ERR_LINKFAIL	"unable to create link to <%s>"
#define ERR_SLINKFAIL	"unable to create symbolic link to <%s>"
#define ERR_DIRFAIL	"unable to create directory"
#define ERR_CDEVFAIL	"unable to create character-special device"
#define ERR_BDEVFAIL	"unable to create block-special device"
#define ERR_PIPEFAIL	"unable to create named pipe"
#define ERR_ATTRFAIL	"unable to fix attributes"
#define ERR_BADGRPID	"unable to determine group name for gid <%d>"
#define ERR_BADUSRID	"unable to determine owner name for gid <%d>"
#define ERR_BADGRPNM	"group name <%s> does not exist in /etc/group"
#define ERR_BADUSRNM	"owner name <%s> does not exist in /etc/passwd"

char	errbuf[PATH_MAX+512];

static int	cksumerr;
static unsigned docksum();

struct part { short unsigned hi,lo; };
static union hilo { /* this only works right in case short is 1/2 of long */
	struct part hl;
	long	lg;
} tempa, suma;


/*VARARGS*/
static void
reperr(va_alist)
va_dcl
{
	char *fmt, *pt;
	va_list	ap;
	int	n;

	va_start(ap);
	if(n = strlen(errbuf)) {
		pt = errbuf + n;
		*pt++ = '\n';
		*pt = '\0';
	} else
		pt = errbuf;
	if(fmt = va_arg(ap, char *)) {
		(void) vsprintf(pt, fmt, ap);
		pt += strlen(pt);
	} else
		errbuf[0] = '\0';
	va_end(ap);
}
/*ARGSUSED*/

int
cverify(fix, ftype, path, cinfo)
int fix;
char *ftype, *path;
struct cinfo *cinfo;
{
	struct stat	status;	/* file status buffer */
	struct utimbuf	times;
	unsigned	mycksum;
	int		setval, retcode;
	char		tbuf1[26], tbuf2[26];

	setval = (*ftype == '?');
	retcode = 0;
	reperr(NULL);

	if(stat(path, &status) < 0) {
		reperr(ERR_EXIST);
		return(VE_EXIST);
	}

	/* -1	requires modtimes to be the same */
	/*  0   reports modtime failure */
	/*  1   fixes modtimes */
	if(setval || (cinfo->modtime == BADCONT))
		cinfo->modtime = status.st_mtime;
	else if(status.st_mtime != cinfo->modtime) {
		if(fix > 0) {
			/* reset times on the file */
			times.actime = cinfo->modtime;
			times.modtime = cinfo->modtime;
			if(utime(path, &times)) {
				reperr(ERR_MODFAIL);
				retcode = VE_FAIL;
			}
		} else if(fix < 0) {
			/* modtimes must be the same */
			(void) cftime(tbuf1, DATEFMT, &cinfo->modtime);
			(void) cftime(tbuf2, DATEFMT, &status.st_mtime);
			reperr(ERR_MTIME, tbuf1, tbuf2);
			retcode = VE_CONT;
		} /*else
			retcode = VE_TIME;*/
	} 
	if(setval || (cinfo->size == BADCONT))
		cinfo->size = status.st_size;
	else if(status.st_size != cinfo->size) {
		if(!retcode /*|| (retcode == VE_TIME)*/)
			retcode = VE_CONT;
		reperr(ERR_SIZE, cinfo->size, status.st_size);
	} 

	mycksum = docksum(path);
	if(setval || (cinfo->cksum == BADCONT))
		cinfo->cksum = mycksum;
	else if((mycksum != cinfo->cksum) || cksumerr) {
		if(!retcode /*|| (retcode == VE_TIME)*/)
			retcode = VE_CONT;
		reperr(ERR_CKSUM, cinfo->cksum, mycksum);
	}

	return(retcode);
}
	
static unsigned
docksum(path)
char *path;
{
	register int	ca;
	register FILE *fp;
	unsigned lsavhi, lsavlo;

	cksumerr = 0;
	if((fp = fopen(path, "r")) == NULL) {
		cksumerr++;
		return(0);
	}

	suma.lg = 0;
	while((ca = getc(fp)) != EOF)
		suma.lg += ca & WDMSK;
	tempa.lg = (suma.hl.lo & WDMSK) + (suma.hl.hi & WDMSK);
	lsavhi = (unsigned) tempa.hl.hi;
	lsavlo = (unsigned) tempa.hl.lo;
	(void) fclose(fp);
	return(lsavhi+lsavlo);
}

int
averify(fix, ftype, path, ainfo)
int	fix;
char	*ftype, *path;
struct ainfo *ainfo;
{
	struct stat	status;	/* file status buffer */
	struct group	*grp;	/* group entry buffer */
	struct passwd	*pwd;
	int		n;
	int		setval;
	int		uid, gid;
	int		dochown;
	int		retcode;
	char		myftype;
	char		buf[PATH_MAX];
	ino_t		my_ino;
	dev_t		my_dev;

	setval = (*ftype == '?');
	retcode = 0;
	reperr(NULL);

	if(*ftype == 'l') {
		if(stat(path, &status) < 0) {
			retcode = VE_EXIST;
			reperr(ERR_EXIST);
		} 

		my_ino = status.st_ino;
		my_dev = status.st_dev;

		if(retcode || (status.st_nlink < 2) || 
		(stat(ainfo->local, &status) < 0) ||
		(my_dev != status.st_dev) || (my_ino != status.st_ino)) {
			if(fix) {
				(void) unlink(path);
				if(link(ainfo->local, path)) {
					reperr(ERR_LINKFAIL, ainfo->local);
					return(VE_FAIL);
				}
				retcode = 0;
			} else {
				reperr(ERR_LINK, ainfo->local);
				return(VE_CONT);
			}
		}
		return(retcode);
	}

	retcode = 0;

	if((*ftype == 's') ? lstat(path, &status) : stat(path, &status)) {
		reperr(ERR_EXIST);
		retcode = VE_EXIST;
		myftype = '?';
	} else {
		/* determining actual type of existing object */
		switch(status.st_mode & S_IFMT) {
#ifndef PRESVR4
               	  case S_IFLNK:
                        myftype = 's';
                        break;
#endif
		  case S_IFIFO:
			myftype = 'p';
			break;

		  case S_IFCHR:
			myftype = 'c';
			break;

		  case S_IFDIR:
			myftype = 'd';
			break;

		  case S_IFBLK:
			myftype = 'b';
			break;

		  case S_IFREG:
		  case 0:
			myftype = 'f';
			break;

		  default:
			reperr(ERR_UNKNOWN);
			return(VE_FTYPE);
		}
	}

	if(setval)
		*ftype = myftype;
	else if(!retcode && (*ftype != myftype) && 
	   ((myftype != 'f') || !strchr("ilev", *ftype)) &&
	   ((myftype != 'd') || (*ftype != 'x'))) {
		reperr(ERR_FTYPE, *ftype, myftype);
		retcode = VE_FTYPE;
	}

	if(!retcode && (*ftype == 's')) {
		/* make sure that symbolic link is correct */
		n = readlink(path, buf, PATH_MAX);
		if(n < 0) {
			reperr(ERR_SLINK, ainfo->local);
			retcode = VE_CONT;
		} else {
			buf[n] = '\0';
			if(strcmp(buf, ainfo->local)) {
				reperr(ERR_SLINK, ainfo->local);
				retcode = VE_CONT;
			}
		}
	}

	if(retcode) {
		/* path doesn't exist or is different than it should be */
		if(fix) {
			(void) unlink(path); /* in case it exists */
			if(strchr("dx", *ftype)) {
				if(mkdir(path, ainfo->mode) ||
				(stat(path, &status) < 0)) {
					reperr(ERR_DIRFAIL);
					return(VE_FAIL);
				}
			} else if(*ftype == 's') {
				if(symlink(ainfo->local, path)) {
					reperr(ERR_SLINKFAIL, ainfo->local);
					return(VE_FAIL);
				}

			} else if(*ftype == 'c') {
				if(mknod(path, ainfo->mode | S_IFCHR, 
					makedev(ainfo->major, ainfo->minor)) ||
				(stat(path, &status) < 0)) {
					reperr(ERR_CDEVFAIL);
					return(VE_FAIL);
				}
			} else if(*ftype == 'b') {
				if(mknod(path, ainfo->mode | S_IFBLK,
					makedev(ainfo->major, ainfo->minor)) ||
				(stat(path, &status) < 0)) {
					reperr(ERR_BDEVFAIL);
					return(VE_FAIL);
				}
			} else if(*ftype == 'p') {
				if(mknod(path, ainfo->mode | S_IFIFO, NULL) ||
				(stat(path, &status) < 0)) {
					reperr(ERR_PIPEFAIL);
					return(VE_FAIL);
				}
			} else
				return(retcode);
				
		} else
			return(retcode);
	}

	if(*ftype == 's')
		return(0); /* don't check anything else */
	if(*ftype == 'i')
		return(0); /* don't check anything else */

	retcode = 0;
	if(strchr("cb", myftype)) {
		if(setval || (ainfo->major < 0))
			ainfo->major = major(status.st_rdev);
		if(setval || (ainfo->minor < 0))
			ainfo->minor = minor(status.st_rdev);
		/* check major & minor */
		if(status.st_rdev != makedev(ainfo->major, ainfo->minor)) {
			reperr(ERR_MAJMIN, ainfo->major, ainfo->minor,
				major(status.st_rdev), minor(status.st_rdev));
			retcode = VE_CONT;
		}
	}

	/* compare specified mode w/ actual mode excluding sticky bit */
	if(setval || (ainfo->mode == BADMODE))
		ainfo->mode = status.st_mode & 07777;
	else if((ainfo->mode & 06777) != (status.st_mode & 06777)) {
		if(fix) {
			if((ainfo->mode < 0) || 
			   (chmod(path, ainfo->mode) < 0))
				retcode = VE_FAIL;
		} else {
			reperr(ERR_PERM, ainfo->mode, 
				status.st_mode & 07777);
			if(!retcode)
				retcode = VE_ATTR;
		}
	}

	/* rewind group file */
	setgrent();
	dochown = 0;

	/* get group entry for specified group */
	if(setval || !strcmp(ainfo->group, BADGROUP)) {
		grp = getgrgid(status.st_gid);
		if(grp)
			(void) strcpy(ainfo->group, grp->gr_name);
		else {
			if(!retcode)
				retcode = VE_ATTR;
			reperr(ERR_BADGRPID, status.st_gid);
		}
		gid = status.st_gid;
	} else if((grp = getgrnam(ainfo->group)) == NULL) {
		reperr(ERR_BADGRPNM, ainfo->group);
		if(!retcode)
			retcode = VE_ATTR;
	} else if((gid=grp->gr_gid) != status.st_gid) {
		if(fix) {
			/* save specified GID */
			gid = grp->gr_gid;
			dochown++;
		} else {
			grp = getgrgid((int)status.st_gid);
			reperr(ERR_GROUP, ainfo->group, grp->gr_name);
			if(!retcode)
				retcode = VE_ATTR;
		}
	}

	/* rewind password file */
	setpwent();

	/* get password entry for specified owner */
	if(setval || !strcmp(ainfo->owner, BADOWNER)) {
		pwd = getpwuid((int)status.st_uid);
		if(pwd)
			(void) strcpy(ainfo->owner, pwd->pw_name);
		else {
			if(!retcode)
				retcode = VE_ATTR;
			reperr(ERR_BADUSRID, status.st_uid);
		}
		uid = status.st_uid;
	} else if ((pwd = getpwnam(ainfo->owner)) == NULL) {
		/* UID does not exist in password file */
		reperr(ERR_BADUSRNM, ainfo->owner);
		if(!retcode)
			retcode = VE_ATTR;
	} else if((uid=pwd->pw_uid) != status.st_uid) {
		/* get owner name for actual UID */
		if(fix) {
			uid = pwd->pw_uid;
			dochown++;
		} else {
			pwd = getpwuid((int)status.st_uid);
			reperr(ERR_OWNER, ainfo->owner, pwd->pw_name);
			if(!retcode)
				retcode = VE_ATTR;
		}
	}
	if(dochown && (chown(path, uid, gid) < 0))
		retcode = VE_FAIL; /* chown failed */

	if(retcode == VE_FAIL)
		reperr(ERR_ATTRFAIL);
	return(retcode);
}
