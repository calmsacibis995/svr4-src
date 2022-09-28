/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ufs.cmds:ufs/edquota/edquota.c	1.9.4.1"
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
 * Disk quota editor.
 */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/mnttab.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mntent.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/fs/ufs_quota.h>

#define	DEFEDITOR	"/usr/bin/vi"

struct fsquot {
	struct fsquot *fsq_next;
	struct dqblk fsq_dqb;
	char *fsq_fs;
	char *fsq_dev;
	char *fsq_qfile;
};

struct fsquot *fsqlist;

char	tmpfil[] = "/tmp/EdP.aXXXXX";
char	*qfname = "quotas";
extern int	optind;
extern char	*optarg;

void	sigsetmask();
int	sigblock();

main(argc, argv)
	int argc;
	char **argv;
{
	int uid;
	char	*basename;
	int	opt;
	int	i;

	basename = argv[0];
	if (argc < 2) {
		usage();
	}
	if (quotactl(Q_ALLSYNC, NULL, 0, NULL) != 0 && errno == EINVAL) {
		printf("Warning: Quotas are not compiled into this kernel\n");
		sleep(3);
	}
	if (getuid()) {
		fprintf(stderr, "%s: permission denied\n", basename);
		exit(32);
	}
	setupfs();
	if (fsqlist == NULL) {
		fprintf(stderr, "%s: no UFS filesystems with %s file\n",
		    MNTTAB, qfname);
		exit(32);
	}
	mktemp(tmpfil);
	close(creat(tmpfil, 0600));
	(void) chown(tmpfil, getuid(), getgid());
	while ((opt = getopt (argc, argv, "p:tV")) != EOF) {
		switch (opt) {

		case 't':
			gettimes(0);
			if (editit())
				puttimes(0);
			(void) unlink(tmpfil);
			exit(0);

		case 'p':
			uid = getentry(optarg);
			if (uid < 0) {
				(void) unlink(tmpfil);
				exit(32);
			}
			getprivs(uid);
			if (optind == argc) {
				unlink(tmpfil);
				usage();
			}
			for (i = optind; i < argc; i++) {
				uid = getentry(argv[i]);
				if (uid < 0) {
					(void)unlink(tmpfil);
					exit(32);
				}
/*					continue; */
				getdiscq(uid);
				putprivs(uid);
			}
			(void) unlink(tmpfil);
			exit(0);

		case 'V':		/* Print command line */
			{
				char		*opt_text;
				int		opt_count;

				(void) fprintf (stdout, "edquota -F UFS ");
				for (opt_count = 1; opt_count < argc ; opt_count++) {
					opt_text = argv[opt_count];
					if (opt_text)
						(void) fprintf (stdout, " %s ", opt_text);
				}
				(void) fprintf (stdout, "\n");
			}
			break;

		case '?':
			usage ();
		}
	}
	(void) unlink(tmpfil);
	for (i = optind; i < argc; i++) {
		uid = getentry(argv[i]);
		if (uid < 0)
			continue;
		getprivs(uid);
		if (editit())
			putprivs(uid);
	}
	(void) unlink(tmpfil);
	exit(0);
}

getentry(name)
	char *name;
{
	struct passwd *pw;
	int uid;

	if (alldigits(name))
		uid = atoi(name);
	else if (pw = getpwnam(name))
		uid = pw->pw_uid;
	else {
		fprintf(stderr, "%s: no such user\n", name);
		sleep(1);
		return (-1);
	}
	return (uid);
}

editit()
{
	register pid, xpid;
	int stat, omask;
	extern char *getenv();

#define	mask(s)	(1<<((s)-1))
	omask = sigblock(mask(SIGINT)|mask(SIGQUIT)|mask(SIGHUP));
 top:
	if ((pid = fork()) < 0) {
		extern errno;

		if (errno == EAGAIN) {
			fprintf(stderr, "You have too many processes\n");
			return(0);
		}
		if (errno == EAGAIN) {
			sleep(1);
			goto top;
		}
		perror("fork");
		return (0);
	}
	if (pid == 0) {
		register char *ed;

		(void) sigsetmask(omask);
		(void) setgid(getgid());
		(void) setuid(getuid());
		if ((ed = getenv("EDITOR")) == (char *)0)
			ed = DEFEDITOR;
		execlp(ed, ed, tmpfil, 0);
		perror(ed);
		exit(32);
	}
	while ((xpid = wait(&stat)) >= 0)
		if (xpid == pid)
			break;
	(void) sigsetmask(omask);
	return (!stat);
}

getprivs(uid)
	register int uid;
{
	register struct fsquot *fsqp;
	FILE *fd;

	getdiscq(uid);
	if ((fd = fopen(tmpfil, "w")) == NULL) {
		fprintf(stderr, "edquota: ");
		perror(tmpfil);
		exit(32);
	}
	for (fsqp = fsqlist; fsqp; fsqp = fsqp->fsq_next) {
		fprintf(fd,
"fs %s blocks (soft = %ld, hard = %ld) inodes (soft = %ld, hard = %ld)\n"
			, fsqp->fsq_fs
			, dbtob(fsqp->fsq_dqb.dqb_bsoftlimit) / 1024
			, dbtob(fsqp->fsq_dqb.dqb_bhardlimit) / 1024
			, fsqp->fsq_dqb.dqb_fsoftlimit
			, fsqp->fsq_dqb.dqb_fhardlimit
		);
	}
	fclose(fd);
}

putprivs(uid)
	register int uid;
{
	FILE *fd;
	struct dqblk ndqb;
	char line[BUFSIZ];
	int changed = 0;

	fd = fopen(tmpfil, "r");
	if (fd == NULL) {
		fprintf(stderr, "Can't re-read temp file!!\n");
		return;
	}
	while (fgets(line, sizeof(line), fd) != NULL) {
		register struct fsquot *fsqp;
		char *cp, *dp, *next();
		int n;

		cp = next(line, " \t");
		if (cp == NULL)
			break;
		*cp++ = '\0';
		while (*cp && *cp == '\t' && *cp == ' ')
			cp++;
		dp = cp, cp = next(cp, " \t");
		if (cp == NULL)
			break;
		*cp++ = '\0';
		for (fsqp = fsqlist; fsqp; fsqp = fsqp->fsq_next) {
			if (strcmp(dp, fsqp->fsq_fs) == 0)
				break;
		}
		if (fsqp == NULL) {
			fprintf(stderr, "%s: unknown file system\n", cp);
			continue;
		}
		while (*cp && *cp == '\t' && *cp == ' ')
			cp++;
		n = sscanf(cp,
"blocks (soft = %ld, hard = %ld) inodes (soft = %ld, hard = %ld)\n"
			, &ndqb.dqb_bsoftlimit
			, &ndqb.dqb_bhardlimit
			, &ndqb.dqb_fsoftlimit
			, &ndqb.dqb_fhardlimit
		);
		if (n != 4) {
			fprintf(stderr, "%s: bad format\n", cp);
			continue;
		}
		changed++;
		ndqb.dqb_bsoftlimit = btodb(ndqb.dqb_bsoftlimit * 1024);
		ndqb.dqb_bhardlimit = btodb(ndqb.dqb_bhardlimit * 1024);
		/*
		 * It we are decreasing the soft limits, set the time limits
		 * to zero, in case the user is now over quota.
		 * the time limit will be started the next time the
		 * user does an allocation.
		 */
		if (ndqb.dqb_bsoftlimit < fsqp->fsq_dqb.dqb_bsoftlimit)
			fsqp->fsq_dqb.dqb_btimelimit = 0;
		if (ndqb.dqb_fsoftlimit < fsqp->fsq_dqb.dqb_fsoftlimit)
			fsqp->fsq_dqb.dqb_ftimelimit = 0;
		fsqp->fsq_dqb.dqb_bsoftlimit = ndqb.dqb_bsoftlimit;
		fsqp->fsq_dqb.dqb_bhardlimit = ndqb.dqb_bhardlimit;
		fsqp->fsq_dqb.dqb_fsoftlimit = ndqb.dqb_fsoftlimit;
		fsqp->fsq_dqb.dqb_fhardlimit = ndqb.dqb_fhardlimit;
	}
	fclose(fd);
	if (changed)
		putdiscq(uid);
}

gettimes(uid)
	register int uid;
{
	register struct fsquot *fsqp;
	FILE *fd;
	char btime[80], ftime[80];

	getdiscq(uid);
	if ((fd = fopen(tmpfil, "w")) == NULL) {
		fprintf(stderr, "edquota: ");
		perror(tmpfil);
		exit(32);
	}
	for (fsqp = fsqlist; fsqp; fsqp = fsqp->fsq_next) {
		fmttime(btime, fsqp->fsq_dqb.dqb_btimelimit);
		fmttime(ftime, fsqp->fsq_dqb.dqb_ftimelimit);
		fprintf(fd,
"fs %s blocks time limit = %s, files time limit = %s\n"
			, fsqp->fsq_fs
			, btime
			, ftime
		);
	}
	fclose(fd);
}

puttimes(uid)
	register int uid;
{
	FILE *fd;
	char line[BUFSIZ];
	int changed = 0;
	double btimelimit, ftimelimit;
	char bunits[80], funits[80];

	fd = fopen(tmpfil, "r");
	if (fd == NULL) {
		fprintf(stderr, "Can't re-read temp file!!\n");
		return;
	}
	while (fgets(line, sizeof(line), fd) != NULL) {
		register struct fsquot *fsqp;
		char *cp, *dp, *next();
		int n;

		cp = next(line, " \t");
		if (cp == NULL)
			break;
		*cp++ = '\0';
		while (*cp && *cp == '\t' && *cp == ' ')
			cp++;
		dp = cp, cp = next(cp, " \t");
		if (cp == NULL)
			break;
		*cp++ = '\0';
		for (fsqp = fsqlist; fsqp; fsqp = fsqp->fsq_next) {
			if (strcmp(dp, fsqp->fsq_fs) == 0)
				break;
		}
		if (fsqp == NULL) {
			fprintf(stderr, "%s: unknown file system\n", cp);
			continue;
		}
		while (*cp && *cp == '\t' && *cp == ' ')
			cp++;
		n = sscanf(cp,
"blocks time limit = %lf %[^,], files time limit = %lf %s\n"
			, &btimelimit
			, bunits
			, &ftimelimit
			, funits
		);
		if (n != 4 ||
		    !unfmttime(btimelimit, bunits,
		      &fsqp->fsq_dqb.dqb_btimelimit) ||
		    !unfmttime(ftimelimit, funits,
		      &fsqp->fsq_dqb.dqb_ftimelimit)) {
			fprintf(stderr, "%s: bad format\n", cp);
			continue;
		}
		changed++;
	}
	fclose(fd);
	if (changed)
		putdiscq(uid);
}

char *
next(cp, match)
	register char *cp;
	char *match;
{
	register char *dp;

	while (cp && *cp) {
		for (dp = match; dp && *dp; dp++)
			if (*dp == *cp)
				return (cp);
		cp++;
	}
	return ((char *)0);
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

static struct {
	int c_secs;			/* conversion units in secs */
	char * c_str;			/* unit string */
} cunits [] = {
	{60*60*24*28, "month"},
	{60*60*24*7, "week"},
	{60*60*24, "day"},
	{60*60, "hour"},
	{60, "min"},
	{1, "sec"}
};

fmttime(buf, time)
	char *buf;
	register u_long time;
{
	double value;
	int i;

	if (time == 0) {
		strcpy(buf, "0 (default)");
		return;
	}
	for (i = 0; i < sizeof(cunits)/sizeof(cunits[0]); i++) {
		if (time >= cunits[i].c_secs)
			break;
	}
	value = (double)time / cunits[i].c_secs;
	sprintf(buf, "%.2f %s%s", value, cunits[i].c_str, value > 1.0? "s": "");
}

int
unfmttime(value, units, timep)
	double value;
	char *units;
	u_long *timep;
{
	int i;

	if (value == 0.0) {
		*timep = 0;
		return (1);
	}
	for (i = 0; i < sizeof(cunits)/sizeof(cunits[0]); i++) {
		if (strncmp(cunits[i].c_str,units,strlen(cunits[i].c_str)) == 0)
			break;
	}
	if (i >= sizeof(cunits)/sizeof(cunits[0]))
		return (0);
	*timep = (u_long)(value * cunits[i].c_secs);
	return (1);
}

setupfs()
{
	struct mnttab mntp;
	register struct fsquot *fsqp;
	struct stat statb;
	dev_t fsdev;
	FILE *mtab;
	char qfilename[MAXPATHLEN + 1];
	extern int errno;
	extern char *malloc();
	int status;

	mtab = fopen(MNTTAB, "r");
	while ((status = getmntent(mtab, &mntp)) == NULL) {
		if (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) != 0)
			continue;
		if (stat(mntp.mnt_special, &statb) < 0)
			continue;
		if ((statb.st_mode & S_IFMT) != S_IFBLK)
			continue;
		fsdev = statb.st_rdev;
		sprintf(qfilename, "%s/%s", mntp.mnt_mountp, qfname);
		if (stat(qfilename, &statb) < 0 || statb.st_dev != fsdev)
			continue;
		fsqp = (struct fsquot *)malloc(sizeof(struct fsquot));
		if (fsqp == NULL) {
			fprintf(stderr, "out of memory\n");
			exit (31+1);
		}
		fsqp->fsq_next = fsqlist;
		fsqp->fsq_fs = malloc(strlen(mntp.mnt_mountp) + 1);
		fsqp->fsq_dev = malloc(strlen(mntp.mnt_special) + 1);
		fsqp->fsq_qfile = malloc(strlen(qfilename) + 1);
		if (fsqp->fsq_fs == NULL || fsqp->fsq_dev == NULL ||
		    fsqp->fsq_qfile == NULL) {
			fprintf(stderr, "out of memory\n");
			exit (31+1);
		}
		strcpy(fsqp->fsq_fs, mntp.mnt_mountp);
		strcpy(fsqp->fsq_dev, mntp.mnt_special);
		strcpy(fsqp->fsq_qfile, qfilename);
		fsqlist = fsqp;
	}
	fclose(mtab);
}

getdiscq(uid)
	register uid;
{
	register struct fsquot *fsqp;
	int fd;

	for (fsqp = fsqlist; fsqp; fsqp = fsqp->fsq_next) {
		if (quotactl(Q_GETQUOTA,fsqp->fsq_dev,uid,&fsqp->fsq_dqb)!=0) {
			if ((fd = open(fsqp->fsq_qfile, O_RDONLY)) < 0) {
				fprintf(stderr, "edquota: ");
				perror(fsqp->fsq_qfile);
				continue;
			}
			(void) lseek(fd, (long)dqoff(uid), L_SET);
			switch (read(fd, &fsqp->fsq_dqb, sizeof(struct dqblk))){
			case 0:
				/*
				 * Convert implicit 0 quota (EOF)
				 * into an explicit one (zero'ed dqblk)
				 */
				bzero((caddr_t)&fsqp->fsq_dqb,
				    sizeof (struct dqblk));
				break;

			case sizeof (struct dqblk):	/* OK */
				break;

			default:			/* ERROR */
				fprintf(stderr, "edquota: read error in ");
				perror(fsqp->fsq_qfile);
				break;
			}
			close(fd);
		}
	}
}

putdiscq(uid)
	register uid;
{
	register struct fsquot *fsqp;

	for (fsqp = fsqlist; fsqp; fsqp = fsqp->fsq_next) {
		if (quotactl(Q_SETQLIM,fsqp->fsq_dev,uid,&fsqp->fsq_dqb) != 0) {
			register int fd;

			if ((fd = open(fsqp->fsq_qfile, O_RDWR)) < 0) {
				fprintf(stderr, "edquota: ");
				perror(fsqp->fsq_qfile);
				continue;
			}
			(void) lseek(fd, (long)dqoff(uid), L_SET);
			if (write(fd, &fsqp->fsq_dqb, sizeof(struct dqblk)) !=
			    sizeof(struct dqblk)) {
				fprintf(stderr, "edquota: ");
				perror(fsqp->fsq_qfile);
			}
			close(fd);
		}
	}
}

void
sigsetmask(omask)
	u_int	omask;
{
	int	i;

	for (i = 0; i < 32; i++) {
		if (omask & (1 << i)) {
			if (sigignore(1 << i) == (int)SIG_ERR) {
				fprintf (stderr, "Bad signal 0x%x\n", (1 << i));
				exit (31+1);
			}
		}
	}


}

int
sigblock(omask)
	u_int	omask;
{
	u_int	previous = 0;
	u_int	temp;
	int	i;

	for (i = 0; i < 32; i++) {
		if (omask & (1 << i)) {
			if ((temp = sigignore (1 << i)) == (int)SIG_ERR) {
				fprintf (stderr, "Bad signal 0x%x\n", (1 << i));
				exit (31+1);
			}
			if (i == 0)
				previous = temp;
		}
	}
	return (previous);
}

usage ()
{

	fprintf(stderr, "ufs usage:\n");
	fprintf(stderr, "\tedquota [-p username] username ...\n");
	fprintf(stderr, "\tedquota -t\n");
	exit(1);

}

#include <sys/errno.h>

quotactl (cmd, special, uid, addr)
	int		cmd;
	char		*special;
	int		uid;
	caddr_t		addr;
{
	int		fd;
	int		status;
	struct quotctl	quota;
	char		mountpoint[256]; 
	FILE		*fstab;
	struct mnttab	mntp;


	if ((special == NULL) && (cmd == Q_SYNC)) {
		fd = 0;
		cmd = Q_ALLSYNC;
	}
		/*
		 * Find the mount point of the special device.   This is
		 * because the fcntl that implements the quotactl call has
		 * to go to a real file, and not to the block device.
		 */
	else {
		if ((fstab = fopen(MNTTAB, "r")) == NULL) {
			fprintf (stderr, "%s: ", MNTTAB);
			perror ("open");
			exit (31+1);
		}
		mountpoint[0]='\0';
		while ((status = getmntent(fstab, &mntp)) == NULL) {
			if (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) != 0 ||
			    hasmntopt(&mntp, MNTOPT_RO))
				continue;
			if (strcmp(special, mntp.mnt_special) == 0) {
				(void) strcpy(mountpoint, mntp.mnt_mountp);
			}
		}
		fclose(fstab);
		if (mountpoint[0] == '\0') {
			errno = ENOENT;
			return (-1);
		}
		strcat (mountpoint, "/quotas");
		if ((fd = open (mountpoint, O_RDWR)) < 0) {
			fprintf (stderr, "quotactl: ");
			perror ("open");
			exit (31+1);
		}
	}
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
