/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ufs.cmds:ufs/quota/quota.c	1.8.3.1"
/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */
/*
 * Disk quota reporting program.
 */
#include <stdio.h>
#include <sys/mnttab.h>
#include <ctype.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/mntent.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/fs/ufs_quota.h>

int	vflag;
int	nolocalquota;

extern int	optind;
extern char	*optarg;

#define QFNAME	"quotas"

#define kb(n)   (howmany(dbtob(n), 1024))

main(argc, argv)
	int argc;
	char *argv[];
{
	int	opt;
	int	i;

	while ((opt = getopt (argc, argv, "vV")) != EOF) {
		switch (opt) {

		case 'v':
			vflag++;
			break;

		case 'V':		/* Print command line */
			{
				char		*opt_text;
				int		opt_count;

				(void) fprintf (stdout, "quota -F UFS ");
				for (opt_count = 1; opt_count < argc ; opt_count++) {
					opt_text = argv[opt_count];
					if (opt_text)
						(void) fprintf (stdout, " %s ", opt_text);
				}
				(void) fprintf (stdout, "\n");
			}
			break;

		case '?':
			fprintf(stderr, "quota: %c: unknown option\n",
				opt);
			fprintf(stderr, "ufs usage: quota [-v] [username]\n");
			exit(32);
		}
	}
	if (quotactl(Q_ALLSYNC, NULL, 0, NULL) < 0 && errno == EINVAL) {
		if (vflag)
			fprintf(stderr,"There are no quotas on this system\n");
		nolocalquota++;
	}
	if (argc == optind) {
		showuid(getuid());
		exit(0);
	}
	for (i = optind; i < argc; i++) {
		if (alldigits(argv[i]))
			showuid(atoi(argv[i]));
		else
			showname(argv[i]);
	}
	exit(0);
}

showuid(uid)
	int uid;
{
	struct passwd *pwd = getpwuid(uid);

	if (uid == 0) {
		if (vflag)
			printf("no disk quota for uid 0\n");
		return;
	}
	if (pwd == NULL)
		showquotas(uid, "(no account)");
	else
		showquotas(uid, pwd->pw_name);
}

showname(name)
	char *name;
{
	struct passwd *pwd = getpwnam(name);

	if (pwd == NULL) {
		fprintf(stderr, "quota: %s: unknown user\n", name);
		exit(32);
	}
	if (pwd->pw_uid == 0) {
		if (vflag)
			printf("no disk quota for %s (uid 0)\n", name);
		exit(32);
	}
	showquotas(pwd->pw_uid, name);
}

showquotas(uid, name)
	int uid;
	char *name;
{
	struct mnttab mntp;
	FILE *mtab;
	struct dqblk dqblk;
	int myuid;
	int status;

	myuid = getuid();
	if (uid != myuid && myuid != 0) {
		printf("quota: %s (uid %d): permission denied\n", name, uid);
		exit(32);
	}
	if (vflag)
		heading(uid, name);
	mtab = fopen(MNTTAB, "r");
	while ((status = getmntent(mtab, &mntp)) == NULL) {
		if (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) == 0) {
			if (nolocalquota ||
			    (quotactl(Q_GETQUOTA,
			      mntp.mnt_mountp, uid, &dqblk) != 0 &&
			     !(vflag && getdiskquota(&mntp, uid, &dqblk))) )
					continue;
		} else {
			continue;
		}
		if (dqblk.dqb_bsoftlimit == 0 && dqblk.dqb_bhardlimit == 0 &&
		    dqblk.dqb_fsoftlimit == 0 && dqblk.dqb_fhardlimit == 0)
			continue;
		if (vflag)
			prquota(&mntp, &dqblk);
		else
			warn(&mntp, &dqblk);
	}
	fclose(mtab);
}

warn(mntp, dqp)
	struct mnttab *mntp;
	struct dqblk *dqp;
{
	struct timeval tv;

	time(&(tv.tv_sec));
	tv.tv_usec = 0;
	if (dqp->dqb_bhardlimit &&
	     dqp->dqb_curblocks >= dqp->dqb_bhardlimit) {
		printf(
"Block limit reached on %s\n",
		    mntp->mnt_mountp
		);
	} else if (dqp->dqb_bsoftlimit &&
	     dqp->dqb_curblocks >= dqp->dqb_bsoftlimit) {
		if (dqp->dqb_btimelimit == 0) {
			printf(
"Over disk quota on %s, remove %dK\n",
			    mntp->mnt_mountp,
			    kb(dqp->dqb_curblocks - dqp->dqb_bsoftlimit + 1)
			);
		} else if (dqp->dqb_btimelimit > tv.tv_sec) {
			char btimeleft[80];

			fmttime(btimeleft, dqp->dqb_btimelimit - tv.tv_sec);
			printf(
"Over disk quota on %s, remove %dK within %s\n",
			    mntp->mnt_mountp,
			    kb(dqp->dqb_curblocks - dqp->dqb_bsoftlimit + 1),
			    btimeleft
			);
		} else {
			printf(
"Over disk quota on %s, time limit has expired, remove %dK\n",
			    mntp->mnt_mountp,
			    kb(dqp->dqb_curblocks - dqp->dqb_bsoftlimit + 1)
			);
		}
	}
	if (dqp->dqb_fhardlimit &&
	    dqp->dqb_curfiles >= dqp->dqb_fhardlimit) {
		printf(
"File count limit reached on %s\n",
		    mntp->mnt_mountp
		);
	} else if (dqp->dqb_fsoftlimit &&
	    dqp->dqb_curfiles >= dqp->dqb_fsoftlimit) {
		if (dqp->dqb_ftimelimit == 0) {
			printf(
"Over file quota on %s, remove %d file%s\n",
			    mntp->mnt_mountp,
			    dqp->dqb_curfiles - dqp->dqb_fsoftlimit + 1,
			    ((dqp->dqb_curfiles - dqp->dqb_fsoftlimit + 1) > 1 ?
				"s" :
				"" )
			);
		} else if (dqp->dqb_ftimelimit > tv.tv_sec) {
			char ftimeleft[80];

			fmttime(ftimeleft, dqp->dqb_ftimelimit - tv.tv_sec);
			printf(
"Over file quota on %s, remove %d file%s within %s\n",
			    mntp->mnt_mountp,
			    dqp->dqb_curfiles - dqp->dqb_fsoftlimit + 1,
			    ((dqp->dqb_curfiles - dqp->dqb_fsoftlimit + 1) > 1 ?
				"s" :
				"" ),
			    ftimeleft
			);
		} else {
			printf(
"Over file quota on %s, time limit has expired, remove %d file%s\n",
			    mntp->mnt_mountp,
			    dqp->dqb_curfiles - dqp->dqb_fsoftlimit + 1,
			    ((dqp->dqb_curfiles - dqp->dqb_fsoftlimit + 1) > 1 ?
				"s" :
				"" )
			);
		}
	}
}

heading(uid, name)
	int uid;
	char *name;
{
	printf("Disk quotas for %s (uid %d):\n", name, uid);
	printf("%-12s %7s%7s%7s%12s%7s%7s%7s%12s\n"
		, "Filesystem"
		, "usage"
		, "quota"
		, "limit"
		, "timeleft"
		, "files"
		, "quota"
		, "limit"
		, "timeleft"
	);
}

prquota(mntp, dqp)
	register struct mnttab *mntp;
	register struct dqblk *dqp;
{
	struct timeval tv;
	char ftimeleft[80], btimeleft[80];
	char *cp;

	time(&(tv.tv_sec));
	tv.tv_usec = 0;
	if (dqp->dqb_bsoftlimit && dqp->dqb_curblocks >= dqp->dqb_bsoftlimit) {
		if (dqp->dqb_btimelimit == 0) {
			strcpy(btimeleft, "NOT STARTED");
		} else if (dqp->dqb_btimelimit > tv.tv_sec) {
			fmttime(btimeleft, dqp->dqb_btimelimit - tv.tv_sec);
		} else {
			strcpy(btimeleft, "EXPIRED");
		}
	} else {
		btimeleft[0] = '\0';
	}
	if (dqp->dqb_fsoftlimit && dqp->dqb_curfiles >= dqp->dqb_fsoftlimit) {
		if (dqp->dqb_ftimelimit == 0) {
			strcpy(ftimeleft, "NOT STARTED");
		} else if (dqp->dqb_ftimelimit > tv.tv_sec) {
			fmttime(ftimeleft, dqp->dqb_ftimelimit - tv.tv_sec);
		} else {
			strcpy(ftimeleft, "EXPIRED");
		}
	} else {
		ftimeleft[0] = '\0';
	}
	if (strlen(mntp->mnt_mountp) > 12) {
		printf("%s\n", mntp->mnt_mountp);
		cp = "";
	} else {
		cp = mntp->mnt_mountp;
	}
	printf("%-12.12s %7d%7d%7d%12s%7d%7d%7d%12s\n",
	    cp,
	    kb(dqp->dqb_curblocks),
	    kb(dqp->dqb_bsoftlimit),
	    kb(dqp->dqb_bhardlimit),
	    btimeleft,
	    dqp->dqb_curfiles,
	    dqp->dqb_fsoftlimit,
	    dqp->dqb_fhardlimit,
	    ftimeleft
	);
}

fmttime(buf, time)
	char *buf;
	register long time;
{
	int i;
	static struct {
		int c_secs;		/* conversion units in secs */
		char * c_str;		/* unit string */
	} cunits [] = {
		{60*60*24*28, "months"},
		{60*60*24*7, "weeks"},
		{60*60*24, "days"},
		{60*60, "hours"},
		{60, "mins"},
		{1, "secs"}
	};

	if (time <= 0) {
		strcpy(buf, "EXPIRED");
		return;
	}
	for (i = 0; i < sizeof(cunits)/sizeof(cunits[0]); i++) {
		if (time >= cunits[i].c_secs)
			break;
	}
	sprintf(buf, "%.1f %s", (double)time/cunits[i].c_secs, cunits[i].c_str);
}

alldigits(s)
	register char *s;
{
	register c;

	c = *s++;
	do {
		if (!isdigit(c))
			return (0);
	} while (c = *s++);
	return (1);
}

int
getdiskquota(mntp, uid, dqp)
	struct mnttab *mntp;
	int uid;
	struct dqblk *dqp;
{
	int fd;
	dev_t fsdev;
	struct stat statb;
	char qfilename[MAXPATHLEN];
	extern int errno;

	if (stat(mntp->mnt_special, &statb) < 0 ||
	    (statb.st_mode & S_IFMT) != S_IFBLK)
		return (0);
	fsdev = statb.st_rdev;
	sprintf(qfilename, "%s/%s", mntp->mnt_mountp, QFNAME);
	if (stat(qfilename, &statb) < 0 || statb.st_dev != fsdev)
		return (0);
	if ((fd = open(qfilename, O_RDONLY)) < 0)
		return (0);
	(void) lseek(fd, (long)dqoff(uid), L_SET);
	switch (read(fd, dqp, sizeof(struct dqblk))) {
	case 0:				/* EOF */
		/*
		 * Convert implicit 0 quota (EOF)
		 * into an explicit one (zero'ed dqblk).
		 */
		bzero((caddr_t)dqp, sizeof(struct dqblk));
		break;

	case sizeof(struct dqblk):	/* OK */
		break;

	default:			/* ERROR */
		close(fd);
		return (0);
	}
	close(fd);
	return (1);
}

#include <sys/errno.h>

quotactl (cmd, mountp, uid, addr)
	int		cmd;
	char		*mountp;
	int		uid;
	caddr_t		addr;
{
	int		fd;
	int		status;
	struct quotctl	quota;
	char		mountpoint[256];

	FILE		*fstab;
	struct mnttab	mntp;


	if ((mountp == NULL) && (cmd == Q_ALLSYNC)) {
	/*
	 * Find the mount point of any mounted file system. This is
	 * because the ioctl that implements the quotactl call has
	 * to go to a real file, and not to the block device.
	 */
	if ((fstab = fopen(MNTTAB, "r")) == NULL) {
		fprintf (stderr, "%s: ", MNTTAB);
		perror ("open");
		exit (32);
	}
	fd = 0;
	while ((status = getmntent(fstab, &mntp)) == NULL) {
		if (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) != 0 ||
			hasmntopt(&mntp, MNTOPT_RO))
			continue;
		(void) strcpy(mountpoint, mntp.mnt_mountp);
		strcat (mountpoint, "/quotas");
		if ((fd = open(mountpoint, O_RDONLY)) == 0)
			break;
	}
	fclose(fstab);
	if (fd == 0) {
		errno = ENOENT;
		printf("quotactl: no quotas file on any mounted file system\n");
		return(-1);
	}
	}
	else {
	if (mountp == NULL || mountp[0] == '\0') {
		errno = ENOENT; 
		return (-1); 
	}
	(void) strcpy(mountpoint, mountp);
	strcat (mountpoint, "/quotas");
	if ((fd = open (mountpoint, O_RDONLY)) < 0) {
		fprintf (stderr, "quotactl: %s ", mountpoint);
		perror ("open");
		return (-1);
	}
	}	/* else */
	quota.op = cmd;
	quota.uid = uid;
	quota.addr = addr;
	status = ioctl (fd, Q_QUOTACTL, &quota);
	if (fd != 0)
		close (fd);
	return (status);
}

char *
hasmntopt(mnt, opt)
        register struct mnttab *mnt;
        register char *opt;
{
        char *f, *opts;
        static char *tmpopts;
	char	*mntopt();

        if (tmpopts == 0) {
                tmpopts = (char *)calloc(256, sizeof (char));
                if (tmpopts == 0)
                        return (0);
        }
        strcpy(tmpopts, mnt->mnt_mntopts);
        opts = tmpopts;
        f = mntopt(&opts);
        for (; *f; f = mntopt(&opts)) {
                if (strncmp(opt, f, strlen(opt)) == 0)
                        return (f - tmpopts + mnt->mnt_mntopts);
        }
        return (NULL);
}

static char *
mntopt(p)
        char **p;
{
        char *cp = *p;
        char *retstr;

        while (*cp && isspace(*cp))
                cp++;
        retstr = cp;
        while (*cp && *cp != ',')
                cp++;
        if (*cp) {
                *cp = '\0';
                cp++;
        }
        *p = cp;
        return (retstr);
}
