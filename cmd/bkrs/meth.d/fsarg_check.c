/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/fsarg_check.c	1.19.3.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/statvfs.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<sys/mnttab.h>
#include	<string.h>
#include	<method.h>
#include	<backup.h>
#include	<signal.h>

extern int	brlog();
extern int	bk_system();
extern int	close();
extern int	do_remount();
extern char	*devattr();
extern int	get_mnt_info();
extern long	lseek();
extern char	*malloc();
extern int	safe_read();
static int	chkrdev();

struct statvfs	Statvfs; /* used in iback, mback */

int
is_read_only(s)
char	*s;
{
	while ((s = strchr(s, '-')) != NULL) {
		if (s[1] == 'r')
			return (1);
	}
	return (0);
} /* is_read_only */

int
fsarg_check(mp)
register m_info_t	*mp;
{
	register int	i;
	int		lablen;
	int		rec;
	char		fsname[PATH_MAX+1];
	struct statvfs	*fs = &Statvfs;
#ifdef TRACE
brlog("fsarg_check: originating fsname mp->ofsname %s, mp->ofsdev %s", mp->ofsname, mp->ofsdev);
#endif

	(void) strcpy(fsname, mp->ofsname);
	if (!strcmp(fsname, "/")) {
		(void) strcat(fsname, ".");
	}
	else {
		(void) strcat(fsname, "/.");
	}
	if (i = statvfs(fsname, fs)) {
		sprintf(ME(mp), "Job ID %s: statvfs of %s failed: %s", mp->jobid, fsname, SE);
		brlog("statvfs of %s failed %s", fsname, SE);
		return(1);
	}
 	mp->fstype = strdup(fs->f_basetype);

#ifdef TRACE
	brlog("fstype of %s is %s", mp->ofsname, mp->fstype);
#endif

	/* check to see that ofsname is mounted on ofsdev */

        if ((rec = get_mnt_info(mp)) < 0) {
                return(1);
        }
	if (mp->ofs_mntopts == NULL) {		/* fs not mounted */
		sprintf(ME(mp), "Job ID %s: %s not mounted on %s", mp->jobid, mp->ofsname, mp->ofsdev);
                brlog(" %s not mounted on %s ", mp->ofsname, mp->ofsdev);
		return(1);
	}
	if (!is_read_only(mp->ofs_mntopts)) {	/* mounted read/write */
		if (mp->flags & mflag) {

			mp->mntinfo = DOUNMOUNT;	/* mount read only */

			if (i = do_remount(mp, "-r")) {	/* read only */
				brlog("cannot mount fs %s dev %s read only %s",
					mp->ofsname, mp->ofsdev, SE);

				if (i = do_remount(mp, 0)) { /* read write */
					brlog("cannot remount fs %s dev %s %s",
						mp->ofsname, mp->ofsdev, SE);
				}
				sprintf(ME(mp), "Job ID %s: umount/mount sequence failed", mp->jobid);
				return(1);
			}
			mp->mntinfo = DOREMOUNT;	/* needs remount */
		}
	}
	else { 
		mp->mntinfo = DONOTHING;
	}
	/*	if a label was provided check it */
	if (lablen = strlen(mp->ofslab)) { /* a label was provided */
		char	*pack = &fs->f_fstr[strlen(fs->f_fstr)+1];

		if (strcmp(mp->ofslab, pack)) {
			brlog("fsarg_check: label check of %s on %s failed, pack=%s",
				mp->ofslab, mp->ofsdev, pack);
			sprintf(ME(mp), "Job ID %s: label check of %s on %s failed",
				 mp->jobid, mp->ofslab, mp->ofsdev);
			return(1);
		}
#ifdef TRACE
		else {
			brlog("fsarg_check: label check of %s on %s matched pack=%s",
				mp->ofslab, mp->ofsdev, pack);
		}
#endif
	}
	return(0);
} /* fsarg_check() */

do_unmount(mp)
register m_info_t	*mp;
{
	char	umnt_cmd[513];
	int	i;

	/* attempt unmount of originating fs dev */

	(void) strcpy(umnt_cmd, "/sbin/umount ");
	(void) strcat(umnt_cmd, mp->ofsdev);

	i = bk_system(umnt_cmd);

	brlog("bk_system of %s returned 0x%x",umnt_cmd, i);

	return(i);
} /* do_unmount() */

int
do_remount(mp, mntopts)
register m_info_t	*mp;
char			*mntopts;	/* non zero, mount read only */
{
	int	i;
	char	mnt_cmd[513];

	/* unmount/mount setting type to root fs type */

	i = do_unmount(mp);

	if (is_read_only(mntopts))
		(void) sprintf( mnt_cmd, "/sbin/mount -F %s -r %s %s",
			mp->fstype, mp->ofsdev, mp->ofsname );
	else (void) sprintf( mnt_cmd, "/sbin/mount -F %s %s %s",
		mp->fstype, mp->ofsdev, mp->ofsname );

	i = bk_system (mnt_cmd);

	brlog("bk_system of %s returned 0x%x",mnt_cmd,i);

	return(i);
} /* do_remount() */

void
remount(mp)
register m_info_t	*mp;
{
	int	i;

	if (mp->mntinfo != DOREMOUNT) {		/* remount not needed */
#ifdef TRACE
		brlog("remount - no action needed");
#endif
		return;
	}
	if (i = do_remount(mp, mp->ofs_mntopts)) { /* remount failed */
		brlog("remount of fs %s on dev %d read/write failed -%s",
					mp->ofsname, mp->ofsdev, SE);
	}
} /* remount() */

get_mnt_info(mp)
register m_info_t	*mp;
{
	FILE		*sd;
	int		i;
	char		*bdev = NULL;
	struct mnttab	mtab;
	extern int	bklevels;

	mp->ofs_mntopts = NULL;		/* change if found */

	if (mp->dtype == IS_DPART) {	/* archive going to partition */
		bdev = devattr(mp->dname, "bdevice");
	}
	BEGIN_CRITICAL_REGION;

	sd = fopen( MNTTAB, "r" );

	END_CRITICAL_REGION;

        if (sd == NULL) {
		sprintf(ME(mp), "Job ID %s: cannot open %s", mp->jobid, MNTTAB);
                brlog(" cannot open %s %s ", MNTTAB, SE);
                return(-1);
        }
        while (!getmntent(sd, &mtab)) {
		if (!chkrdev(mp->ofsdev, mtab.mnt_special)) {
			if (!strcmp(mp->ofsname, mtab.mnt_mountp)) {
				if ((mp->ofs_mntopts = malloc(strlen(mtab.mnt_mntopts))) == NULL) {
					brlog("malloc failed");
					sprintf(ME(mp), "Job ID %s: out of memory", mp->jobid);
					return(-1);
				}
				strcpy(mp->ofs_mntopts, mtab.mnt_mntopts);
				mp->ofsdevmnt = mp->ofsname;
			}
			else {
				if (!(i = strlen(mtab.mnt_mountp)))
					break;

				mp->ofsdevmnt = strdup(mtab.mnt_mountp);
			}
	 	}	
		else if (bdev) {
			if (!chkrdev(bdev, mtab.mnt_special)) { /* dname mounted on */
				brlog("archive device %s is a mounted fs (%s)",
					bdev, mtab.mnt_mountp);
				sprintf(ME(mp), "Job ID %s: archive device %s is mounted (%s)", mp->jobid, bdev, mtab.mnt_mountp);
				return(-1);
			}
		}
        }
	if (mp->nfsdev) {		/* is it mounted on */
		(void) fclose (sd);
		BEGIN_CRITICAL_REGION;

		sd = fopen( MNTTAB, "r" );

		END_CRITICAL_REGION;

		if (sd == NULL) {
			sprintf(ME(mp), "Job ID %s: cannot open %s", mp->jobid, MNTTAB);
			brlog(" cannot open %s %s ", MNTTAB, SE);
			return(-1);
		}
		while (!getmntent(sd, &mtab)) {
			if ((!strcmp(mp->nfsdev, mtab.mnt_special))) {
				if (!(i = strlen(mtab.mnt_mountp)))
					break;
				mp->nfsdevmnt = strdup(mtab.mnt_mountp);
			}
	 	}	
        }
	(void) fclose (sd);

	return(0);
} /* get_mnt_info() */

/* Check to see if two special dev's have the same maj/min pair.
** If equal return 0, else return -1.
*/
static
int chkrdev(orig_dev, mnt_dev)
char *orig_dev, *mnt_dev;
{
	int ret;
	struct stat buf1;
	struct stat buf2;
	if(orig_dev == NULL || mnt_dev == NULL)
		return(-1);
	
	if (!strcmp(orig_dev, mnt_dev))
		return(0);

	if (lstat(orig_dev, &buf1) || lstat(mnt_dev, &buf2))
		return(-1);

	/* check to see if the dev's are really the same */
	if (buf1.st_rdev != buf2.st_rdev)
		return(-1);
	return(0);
}
