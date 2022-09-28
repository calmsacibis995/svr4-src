/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ufs.cmds:ufs/quotacheck/quotacheck.c	1.8.6.1"
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
 * Fix up / report on disc quotas & usage
 */
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mntent.h>
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>
#include <sys/fs/ufs_quota.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mnttab.h>
#include <sys/vfstab.h>
#include <pwd.h>

#define MAXMOUNT	50

union {
	struct	fs	sblk;
	char	dummy[MAXBSIZE];
} un;
#define	sblock	un.sblk

#define	ITABSZ	256
struct	dinode	itab[ITABSZ];
struct	dinode	*dp;

#define LOGINNAMESIZE	8
struct fileusage {
	struct fileusage *fu_next;
	u_long fu_curfiles;
	u_long fu_curblocks;
	u_short	fu_uid;
	char fu_name[LOGINNAMESIZE + 1];
};
#define FUHASH 997
struct fileusage *fuhead[FUHASH];
struct fileusage *lookup();
struct fileusage *adduid();
int highuid;

int fi;
ino_t ino;
struct	dinode	*ginode();
char *malloc(), *makerawname();
char *mntopt(), *hasvfsopt();

extern int	optind;
extern char	*optarg;
extern int	errno;

int	vflag;		/* verbose */
int	aflag;		/* all file systems */
int	pflag;		/* fsck like parallel check */

#define QFNAME "quotas"
char *listbuf[MAXMOUNT];
struct dqblk zerodqbuf;
struct fileusage zerofileusage;

main(argc, argv)
	int argc;
	char **argv;
{
	struct mnttab mntp;
	struct vfstab vfsbuf;
	register struct fileusage *fup;
	char **listp;
	int listcnt;
	char quotafile[MAXPATHLEN + 1];
	FILE *mtab, *vfstab;
	int errs = 0;
	struct passwd *pw;
	int	status;
	int	opt;

	while ((opt = getopt (argc, argv, "vapV")) != EOF) {
		switch (opt) {

		case 'v':
			vflag++;
			break;

		case 'a':
			aflag++;
			break;
	
		case 'p':
			pflag++;
			break;

		case 'V':		/* Print command line */
			{
				char		*opt_text;
				int		opt_count;

				(void) fprintf (stdout, "quotacheck -F UFS ");
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
	if (argc <= optind && !aflag) {
		usage ();
	}

	(void) setpwent();
	while ((pw = getpwent()) != 0) {
		fup = lookup((u_short)pw->pw_uid);
		if (fup == 0) {
			fup = adduid((u_short)pw->pw_uid);
			strncpy(fup->fu_name, pw->pw_name,
				sizeof(fup->fu_name));
		}
	}
	(void) endpwent();

	if (quotactl(Q_ALLSYNC, NULL, 0, NULL) < 0 && errno == EINVAL && vflag)
		printf( "Warning: Quotas are not compiled into this kernel\n");
	sync();

	if (aflag) {
		/*
		 * Go through vfstab and make a list of appropriate
		 * filesystems.
		 */
		listp = listbuf;
		listcnt = 0;
		if ((vfstab = fopen(VFSTAB, "r")) == NULL) {
			fprintf(stderr, "Can't open ");
			perror(VFSTAB);
			exit(31+8);
		}
		while ((status = getvfsent(vfstab, &vfsbuf)) == NULL) {
			if (strcmp(vfsbuf.vfs_fstype, MNTTYPE_UFS) != 0 ||
			hasvfsopt(&vfsbuf, MNTOPT_RO))
				continue;
			*listp = malloc(strlen(vfsbuf.vfs_special) + 1);
			strcpy(*listp, vfsbuf.vfs_special);
			listp++;
			listcnt++;
		}
		fclose(vfstab);
		*listp = (char *)0;
		listp = listbuf;
	} else {
		listp = &argv[optind];
		listcnt = argc - optind;
	}
	if (pflag) {
		errs = preen(listcnt, listp);
	} else {
		if ((mtab = fopen(MNTTAB, "r")) == NULL) {
			fprintf(stderr, "Can't open ");
			perror(MNTTAB);
			exit(31+8);
		}
		while ((status = getmntent(mtab, &mntp)) == NULL) {
			if (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) == 0 &&
			    !hasmntopt(&mntp, MNTOPT_RO) &&
			    (oneof(mntp.mnt_special, listp, listcnt) ||
			     oneof(mntp.mnt_mountp, listp, listcnt)) ) {
				sprintf(quotafile,
				    "%s/%s", mntp.mnt_mountp, QFNAME);
				errs +=
				    chkquota(mntp.mnt_special,
					mntp.mnt_mountp, quotafile);
			}
		}
		fclose(mtab);
	}
	while (listcnt--) {
		if (*listp)
			fprintf(stderr, "Cannot check %s\n", *listp);
	}
	if (errs > 0)
		errs += 31;
	exit(errs);
}

preen(listcnt, listp)
	int listcnt;
	char **listp;
{
	struct mnttab mntp;
	register int passno, anygtr;
	register int errs;
	FILE *mtab;
	int status;
	char quotafile[MAXPATHLEN + 1];
	int stat;

	passno = 1;
	errs = 0;
	if ((mtab = fopen(MNTTAB, "r")) == NULL) {
		fprintf(stderr, "Can't open ");
		perror(MNTTAB);
		exit(31+8);
	}
	do {
		rewind(mtab);
		anygtr = 0;

		while ((stat = getmntent(mtab, &mntp)) == NULL) {
#ifdef never
			if (mntp.mnt_passno > passno)
				anygtr = 1;

			if (mntp.mnt_passno != passno)
				continue;
#endif /* never */

			if (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) != 0 ||
			    hasmntopt(&mntp, MNTOPT_RO) ||
			    (!oneof(mntp.mnt_special, listp, listcnt) &&
			     !oneof(mntp.mnt_mountp, listp, listcnt)) )
				continue;

			switch (fork()) {
			case -1:
				perror("fork");
				exit(31+8);
				break;

			case 0:
				sprintf(quotafile, "%s/%s",
					mntp.mnt_mountp, QFNAME);
				exit(31+chkquota(mntp.mnt_special,
					mntp.mnt_mountp, quotafile));
			}
		}

		while (wait(&status) != -1) 
			errs += WHIBYTE(status);

		passno++;
	} while (anygtr);
	fclose(mtab);

	return (errs);
}

chkquota(fsdev, fsfile, qffile)
	char *fsdev;
	char *fsfile;
	char *qffile;
{
	register struct fileusage *fup;
	dev_t quotadev;
	FILE *qf;
	register u_short uid;
	register u_short highquota;
	int cg, i;
	char *rawdisk;
	struct stat statb;
	struct dqblk dqbuf;
	extern int errno;

	rawdisk = makerawname(fsdev);
	if (vflag)
		printf("*** Checking quotas for %s (%s)\n", rawdisk, fsfile);
	fi = open(rawdisk, 0);
	if (fi < 0) {
		perror(rawdisk);
		return (1);
	}
	qf = fopen(qffile, "r+");
	if (qf == NULL) {
		perror(qffile);
		close(fi);
		return (1);
	}
	if (fstat(fileno(qf), &statb) < 0) {
		perror(qffile);
		fclose(qf);
		close(fi);
		return (1);
	}
	quotadev = statb.st_dev;
	if (stat(fsdev, &statb) < 0) {
		perror(fsdev);
		fclose(qf);
		close(fi);
		return (1);
	}
	if (quotadev != statb.st_rdev) {
		fprintf(stderr, "%s dev (0x%x) mismatch %s dev (0x%x)\n",
			qffile, quotadev, fsdev, statb.st_rdev);
		fclose(qf);
		close(fi);
		return (1);
	}
	bread(SBLOCK, (char *)&sblock, SBSIZE);
	ino = 0;
	for (cg = 0; cg < sblock.fs_ncg; cg++) {
		dp = NULL;
		for (i = 0; i < sblock.fs_ipg; i++)
			acct(ginode());
	}
	highquota = 0;
	for (uid = 0; (int)uid <= (int)highuid; uid++) {
		(void) fseek(qf, (long)dqoff(uid), 0);
		(void) fread(&dqbuf, sizeof(struct dqblk), 1, qf);
		if (feof(qf))
			break;
		fup = lookup(uid);
		if (fup == 0)
			fup = &zerofileusage;
		if (dqbuf.dqb_bhardlimit || dqbuf.dqb_bsoftlimit ||
		    dqbuf.dqb_fhardlimit || dqbuf.dqb_fsoftlimit) {
			highquota = uid;
		} else {
			fup->fu_curfiles = 0;
			fup->fu_curblocks = 0;
		}
		if (dqbuf.dqb_curfiles == fup->fu_curfiles &&
		    dqbuf.dqb_curblocks == fup->fu_curblocks) {
			fup->fu_curfiles = 0;
			fup->fu_curblocks = 0;
			continue;
		}
		if (vflag) {
			if (pflag || aflag)
				printf("%s: ", rawdisk);
			if (fup->fu_name[0] != '\0')
				printf("%-10s fixed:", fup->fu_name);
			else
				printf("#%-9d fixed:", uid);
			if (dqbuf.dqb_curfiles != fup->fu_curfiles)
				printf("  files %d -> %d",
				    dqbuf.dqb_curfiles, fup->fu_curfiles);
			if (dqbuf.dqb_curblocks != fup->fu_curblocks)
				printf("  blocks %d -> %d",
				    dqbuf.dqb_curblocks, fup->fu_curblocks);
			printf("\n");
		}
		dqbuf.dqb_curfiles = fup->fu_curfiles;
		dqbuf.dqb_curblocks = fup->fu_curblocks;
		(void) fseek(qf, (long)dqoff(uid), 0);
		(void) fwrite(&dqbuf, sizeof(struct dqblk), 1, qf);
		(void) quotactl(Q_SETQUOTA, fsfile, uid, &dqbuf);
		fup->fu_curfiles = 0;
		fup->fu_curblocks = 0;
	}
	(void) fflush(qf);
	(void) ftruncate(fileno(qf), (highquota + 1) * sizeof(struct dqblk));
	fclose(qf);
	close(fi);
	return (0);
}

acct(ip)
	register struct dinode *ip;
{
	register struct fileusage *fup;

	if (ip == NULL)
		return;
	if (ip->di_eftflag != EFT_MAGIC) {
		ip->di_mode = ip->di_smode;
		ip->di_uid = ip->di_suid;
	}
	if (ip->di_mode == 0)
		return;
	fup = adduid((u_short)ip->di_uid);
	fup->fu_curfiles++;
	if ((ip->di_mode & IFMT) == IFCHR || (ip->di_mode & IFMT) == IFBLK)
		return;
	fup->fu_curblocks += ip->di_blocks;
}

oneof(target, olistp, on)
	char *target;
	register char **olistp;
	register int on;
{
	char **listp = olistp;
	int n = on;

	while (n--) {
		if (*listp && strcmp(target, *listp) == 0) {
			*listp = (char *)0;
			return (1);
		}
		listp++;
	}
	return (0);
}

struct dinode *
ginode()
{
	register unsigned long iblk;

	if (dp == NULL || ++dp >= &itab[ITABSZ]) {
		iblk = itod(&sblock, ino);
		bread((u_long)fsbtodb(&sblock, iblk),
		    (char *)itab, sizeof itab);
		dp = &itab[(int)ino % (int)INOPB(&sblock)];
	}
	if (ino++ < UFSROOTINO)
		return(NULL);
	return(dp);
}

bread(bno, buf, cnt)
	long unsigned bno;
	char *buf;
{
	extern off_t lseek();
	register off_t pos;

	pos = (off_t)dbtob(bno);
	if (lseek(fi, pos, 0) != pos) {
		perror("lseek");
		exit(31+1);
	}

	(void) lseek(fi, (long)dbtob(bno), 0);
	if (read(fi, buf, cnt) != cnt) {
		perror("read");
		exit(31+1);
	}
}

struct fileusage *
lookup(uid)
	register u_short uid;
{
	register struct fileusage *fup;

	for (fup = fuhead[uid % FUHASH]; fup != 0; fup = fup->fu_next)
		if (fup->fu_uid == uid)
			return (fup);
	return ((struct fileusage *)0);
}

struct fileusage *
adduid(uid)
	register u_short uid;
{
	struct fileusage *fup, **fhp;
	extern char *calloc();

	fup = lookup(uid);
	if (fup != 0)
		return (fup);
	fup = (struct fileusage *)calloc(1, sizeof(struct fileusage));
	if (fup == 0) {
		fprintf(stderr, "out of memory for fileusage structures\n");
		exit(31+1);
	}
	fhp = &fuhead[uid % FUHASH];
	fup->fu_next = *fhp;
	*fhp = fup;
	fup->fu_uid = uid;
	if ((int)uid > (int)highuid)
		highuid = uid;
	return (fup);
}

char *
makerawname(name)
	char *name;
{
	register char *cp;
	char tmp, ch;
	static char rawname[MAXPATHLEN];
	struct stat statb;

	cp = (char *)rindex(name, '/');
	if (cp == NULL)
		return (name);
	/* for device naming convention /dev/dsk/c1d0s2 */
	sprintf(rawname, "/dev/rdsk/%s", cp + 1);
	if (stat(rawname, &statb) == 0)
		return(rawname);

	/* for device naming convention /dev/save */

	strcpy(rawname, name);
	cp = (char *)rindex(rawname, '/');
	cp++;
	for (ch = 'r'; *cp != '\0'; ) {
		tmp = *cp;
		*cp++ = ch;
		ch = tmp;
	}
	*cp++ = ch;
	*cp = '\0';
	return (rawname);
}

usage ()
{
	fprintf(stderr, "ufs usage:\n");
	fprintf(stderr, "quotacheck [-v] [-p] -a\n");
	fprintf(stderr, "quotacheck [-v] [-p] filesys ...\n");
	exit(31+1);
}

#include <sys/errno.h>

quotactl (cmd, mountp, uid, addr)
	int		cmd;
	char		*mountp;
	int		uid;
	caddr_t		addr;
{
	int 		fd;
	int 		status;
	struct quotctl	quota;
	char		mountpoint[256];
	FILE		*fstab;
	struct mnttab	mntp;


	if ((mountp == NULL) && (cmd == Q_ALLSYNC)) {
	/*
	 * Find the mount point of any ufs file system.   This is
	 * because the ioctl that implements the quotactl call has
	 * to go to a real file, and not to the block device.
	 */
	if ((fstab = fopen(MNTTAB, "r")) == NULL) {
		fprintf (stderr, "%s: ", MNTTAB);
		perror ("open");
		exit (31+1);
	}
	fd = 0;
	while ((status = getmntent(fstab, &mntp)) == NULL) {
		if (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) != 0 ||
			hasmntopt(&mntp, MNTOPT_RO))
			continue;
		(void) strcpy(mountpoint, mntp.mnt_mountp);
		strcat (mountpoint, "/quotas");
		if ((fd = open(mountpoint, O_RDWR)) == 0)
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
		errno =  ENOENT;
		return (-1);
	}
	strcpy(mountpoint, mountp);
	strcat (mountpoint, "/quotas");
	if ((fd = open (mountpoint, O_RDWR)) < 0) {
		fprintf (stderr, "quotactl: ");
		perror ("open");
		exit (31+1);
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

char *
hasvfsopt(vfs, opt)
        register struct vfstab *vfs;
        register char *opt;
{
        char *f, *opts;
        static char *tmpopts;

        if (tmpopts == 0) {
                tmpopts = (char *)calloc(256, sizeof (char));
                if (tmpopts == 0)
                        return (0);
        }
        strcpy(tmpopts, vfs->vfs_mntopts);
        opts = tmpopts;
        f = mntopt(&opts);
        for (; *f; f = mntopt(&opts)) {
                if (strncmp(opt, f, strlen(opt)) == 0)
                        return (f - tmpopts + vfs->vfs_mntopts);
        }
        return (NULL);
}
