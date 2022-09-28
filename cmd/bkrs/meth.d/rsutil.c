/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/rsutil.c	1.5.2.1"

#include	<limits.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<method.h>
#include	<errno.h>

#ifndef NULL
#define NULL	0
#endif

#define MDIR	"could not create directory %s %s"
#define SDIR	"create of directory(s) failed"
#define DCREAT	"creat(s) failed"
#define CREAT	"creat of %s failed %s"
#define MDIRF	"could not create directory for %s %s"
#define DMKNOD	"mknod(s) failed"
#define MKNOD	"mknod of %s failed %s"
#define SDIR	"create of directory(s) failed"
#define FNEWER	"file %s exists, newer than requested restore date"
#define UFAIL	"unlink %s failed %s"
#define UDFAIL	"unlink of some existing file(s) failed, not overwritten"
#define DIRNEWER "existing file(s) newer than restore date, not overwritten"
#define AOLDER	"file %s exists, newer than archive file"

extern char	*result();
extern int	safe_stat();
extern void	set_time();

struct stat	Xstatb;

int
check_nondir(namep, fp, f_mtime)
char		*namep;
file_rest_t	*fp;
time_t		f_mtime;
{
	int	i;

	if (safe_stat(namep, &Xstatb) == 0) { 	/* already exists */
		brlog("%s EXISTS mtime=0x%x, f_mtime=0x%x, restore date=0x%x",
			namep, Xstatb.st_mtime, f_mtime, fp->ldate);

		if ((Xstatb.st_mtime) > fp->ldate) {
#ifdef TRACE
			brlog("ARCHIVE NEWER THAN RESTORE DATE");
#endif
			if (!fp->type) {
				fp->rindx = R_EXIST;
				fp->status = F_UNSUCCESS;

				if (!RM(fp)) {
					RM(fp) = result(1, FNEWER, namep);
				}
				return(-1);
			}
			else {
				if (!RM(fp)) {
					RM(fp) = result(0, DIRNEWER);
				}
				return(-2);	/* may find another file */
			}
		}
		else if ((Xstatb.st_mtime) > f_mtime) {
#ifdef TRACE
		     brlog("EXISTING FILE NEWER THAN ARCHIVE!");
#endif
			if (!fp->type) {
				fp->rindx = R_EXIST;

				if (!RM(fp)) {
					RM(fp) = result(1, AOLDER, namep);
				}
				return(-1);
			}
			else {
				return(-2);	/* may find another file */
			}
		}
		else {
#ifdef TRACE
			brlog("OVERWRITE EXISTING FILE!");
#endif
			while (((i = unlink(namep)) == -1) && (errno == EINTR))
				;
#ifdef TRACE
			brlog("unlink: i=%d, errno=%d",i, errno);
#endif
			if ((i == -1) && (errno != ENOENT)) {
#ifdef TRACE
			     brlog("type=%d",fp->type);
#endif
				if (! fp->type) {
					brlog("unable to unlink %s %s",namep,SE);
					fp->rindx = R_EXIST;
					fp->status = F_UNSUCCESS;

					if (!RM(fp)) {
						RM(fp) = result(2, UFAIL,
							 namep, SE);
					}
					return(-1);
				}
				else {
					if (!RM(fp)) {
						RM(fp) = result(0, UDFAIL);
					}
					return(-2);
				}
			}
		}
	}
	return(0);
} /* check_nondir() */

set_modes(namep, mode, uid, gid, mtime)
char	*namep;
mode_t	mode;
uid_t	uid;
gid_t	gid;
time_t	mtime;
{
	int	i;

	if (i = chmod(namep, mode)) {
		brlog("chmod of %s failed  %s", namep, SE); 
	}
	if (i = chown(namep, uid, gid)) {
		brlog("chown of %s failed  %s", namep, SE); 
	}
	set_time(namep, mtime, mtime);
} /* set_modes() */

void
set_time(namep, atime, mtime)	/* set access and modification times */
register char	*namep;
time_t		atime;
time_t		mtime;
{
	static time_t	timevec[2];

	timevec[0] = atime;
	timevec[1] = mtime;
	(void) utime(namep, (struct utimbuf *)timevec);
} /* set_time() */

int
create_a_special(namep, fp, np, mode, rdev)
char		*namep;
char		*np;
file_rest_t	*fp;
mode_t		mode;
dev_t		rdev;
{
	int	ans;

/* try creating (only twice) */
	ans = 0;
	do {
		if (mknod(namep, mode, rdev) < 0) {
			ans += 1;
		}else {
			ans = 0;
			break;
		}
	} while (ans < 2 && missdir(np, fp) == 0);

	if (ans == 1) {
		if (fp->type) {
			if (!RM(fp)) {
				RM(fp) = result(0, SDIR);
			}
		}
		else {
			fp->status = F_UNSUCCESS;
			fp->rindx = R_INCOMP;

			if (!RM(fp)) {
				RM(fp) = result(2, MDIRF, namep, SE);
			}
		}
		brlog("Cannot create directory for %s %s", namep, SE);
		return(-1);
	}else if (ans == 2) {
		if (fp->type) {
			if (!RM(fp)) {
				RM(fp) = result(0, DMKNOD);
			}
		}
		else {
			fp->status = F_UNSUCCESS;
			fp->rindx = R_INCOMP;
			if (!RM(fp)) {
				RM(fp) = result(2, MKNOD,
					namep, SE);
			}
		}
		brlog("Cannot mknod %s %s", namep, SE);
		return(-1);
	}

} /* create_a_special() */

int
missdir(namep, fp)
register char	*namep;
file_rest_t	*fp;
{
	register char	*np;
	register	ctt = 2;
	struct stat	Xstatb;

	for (np = namep; *np; ++np)
		if (*np == '/') {
			if (np == namep) continue;    /* skip over root slash */
			*np = '\0';

			if (stat(namep, &Xstatb) == -1) {
				if ((ctt = makdir(namep, fp, 0, 0, 0)) != 0) {
					*np = '/';
					return(ctt);
				}
			}
			*np = '/';
		}
	if (ctt == 2)		/* the file already exists */
		ctt = 0;

	return (ctt);
} /* missdir() */

int
makdir(namep, fp, uid, gid, Dhead)		/* make needed directories */
register char		*namep;
file_rest_t		*fp;
uid_t			uid;
gid_t			gid;
struct dirs_made	*Dhead;
{
	int			ret;
	int			i;
	int			namlen;
	struct dirs_made	*dmp;
	struct dirs_made	*wrkp;

	if (ret = mkdir(namep, 0755)) {
		brlog("mkdir of %s failed - %s",namep,SE);
		return(ret);
	}
	if (i = chown(namep, uid, gid)) {
		brlog("chown of %s failed  %s", namep, SE); 
	}
	if (fp->rename_len) {		/* what we made is what they get */
		return(ret);
	}
	namlen = strlen(namep);		/* the null is in sizeof below */

	dmp = (struct dirs_made *) malloc ((sizeof(struct dirs_made)) +
					namlen);

	if (dmp == (struct dirs_made *) NULL) {	/* don't fail on this alone */
		return (ret);
	}
	if (!Dhead)
		return(ret);

	wrkp = Dhead->next;
	wrkp->prev = dmp;
	dmp->next = wrkp;
	Dhead->next = dmp;
	dmp->prev = Dhead;
	(void) strcpy(dmp->dirname, namep);

	return(ret);
} /* makdir() */

int
create_a_reg(namep,np,fp,mode,fd)
char		*namep;
char		*np;
file_rest_t	*fp;
mode_t		mode;
int		*fd;
{
	int	ans = 0;

/*  is a regular file -- try creating (only twice) */

	do {
		if ((*fd = creat(namep, mode)) < 0) {
			ans += 1;
		}else {
			ans = 0;
			break;
		}
	} while ((ans < 2) && (missdir(np, fp) == 0));

	if (ans == 1) {
		if (fp->type) {
			if (!RM(fp)) {
				RM(fp) = result(0, SDIR);
			}
		}
		else {
			fp->status = F_UNSUCCESS;
			fp->rindx = R_INCOMP;

			if (!RM(fp)) {
				RM(fp) = result(2, MDIRF,
					namep, SE);
			}
		}
		brlog("Cannot create directory for %s %s", namep, SE);
		return(-1);
	}else if (ans == 2) {
		if (fp->type) {
			if (!RM(fp)) {
				RM(fp) = result(0, DCREAT);
			}
		}
		else {
			fp->status = F_UNSUCCESS;
			fp->rindx = R_INCOMP;
			if (!RM(fp)) {
				RM(fp) = result(2, CREAT,
					namep, SE);
			}
		}
		brlog("Cannot create %s %s", namep, SE);
		return(-1);
	}
	return(0);
} /* create_a_reg() */

int
create_a_dir(namep, fp, Dhead, uid, gid)
char			*namep;
file_rest_t		*fp;
struct dirs_made	*Dhead;
uid_t			uid;
gid_t			gid;
{
	int	ans = 0;

	do {
		if (makdir(namep, fp, uid, gid, Dhead) != 0) {
			ans += 1;
		}else {
			ans = 0;
			break;
		}
	} while(ans < 2 && missdir(namep, fp) == 0);

	if (ans) {
		if (!(fp->type)) {
			fp->status = F_UNSUCCESS;
			fp->rindx = R_INCOMP;

			if (!RM(fp)) {
				RM(fp) = result(2, MDIR,
					namep, SE);
			}
		}
		else {
			if (!RM(fp)) {
				RM(fp) = result(0, SDIR);
			}
		}
		brlog("Cannot create directory %s %s",namep,SE);
		return(-1);
	}
	return(0);
} /* create_a_dir() */
