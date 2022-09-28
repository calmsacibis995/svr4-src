/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ufs.cmds:ufs/quot/quot.c	1.7.3.1"
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
 * quot
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/mnttab.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mntent.h>
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>

#define	ISIZ	(MAXBSIZE/sizeof(struct dinode))
union {
	struct fs u_sblock;
	char dummy[SBSIZE];
} sb_un;
#define sblock sb_un.u_sblock
struct dinode itab[MAXBSIZE/sizeof(struct dinode)];

struct du {
	struct	du *next;
	long	blocks;
	long	blocks30;
	long	blocks60;
	long	blocks90;
	long	nfiles;
	int	uid;
#define	NDU	2048
} du[NDU];
int	ndu;
#define	DUHASH	8209	/* smallest prime >= 4 * NDU */
#define	HASH(u)	((u) % DUHASH)
struct	du *duhash[DUHASH];

#define	TSIZE	2048
int	sizes[TSIZE];
long	overflow;

int	nflg;
int	fflg;
int	cflg;
int	vflg;
int	hflg;
int	aflg;
long	now;

unsigned	ino;

char	*malloc();
char	*getname();

extern int	optind;
extern char	*optarg;

main(argc, argv)
	int argc;
	char *argv[];
{
	register int n;
	int	opt;
	int	i;

	if (argc == 1) {
		fprintf(stderr,
		    "ufs Usage: quot [-nfcvha] [filesystem ...]\n");
		exit(32);
	}
				
	now = time(0);
	while ((opt = getopt (argc, argv, "nfcvhaV")) != EOF) {
		switch (opt) {

		case 'n':
			nflg++; break;

		case 'f':
			fflg++; break;

		case 'c':
			cflg++; break;

		case 'v':
			vflg++; break;

		case 'h':
			hflg++; break;

		case 'a':
			aflg++; break;

		case 'V':		/* Print command line */
			{
				char		*opt_text;
				int		opt_count;

				(void) fprintf (stdout, "quot -F UFS ");
				for (opt_count = 1; opt_count < argc ; opt_count++) {
					opt_text = argv[opt_count];
					if (opt_text)
						(void) fprintf (stdout, " %s ", opt_text);
				}
				(void) fprintf (stdout, "\n");
			}
			break;

		case '?':
			fprintf(stderr,
			    "ufs usage: quot [-nfcvha] [filesystem ...]\n");
			exit(32);
		}
	}
	if (aflg) {
		quotall();
	}
	for (i = optind; i < argc; i++) {
		if ((getdev(&argv[i]) == 0) && (check(argv[i], (char *)NULL) == 0))
			report();
	}
	exit (0);
}

quotall()
{
	FILE *fstab;
	struct mnttab mntp;
	register char *cp;
	char dev[80];
	int	status;
	struct stat statb;

	fstab = fopen(MNTTAB, "r");
	if (fstab == NULL) {
		fprintf(stderr, "quot: no %s file\n", MNTTAB);
		exit(32);
	}
	while ((status = getmntent(fstab, &mntp)) == NULL) {
		if  (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) != 0)
			continue;
		cp = (char *)rindex(mntp.mnt_special, '/');
		if (cp == 0)
			continue;
		sprintf(dev, "/dev/rdsk/%s", cp + 1);
		if (stat(dev, &statb) != 0)
			sprintf(dev, "/dev/r%s", cp + 1);

		if (check(dev, mntp.mnt_mountp) == 0)
			report();
	}
	fclose(fstab);
}

check(file, fsdir)
	char *file;
	char *fsdir;
{
	FILE *fstab;
	register int i, j, nfiles;
	register struct du **dp;
	daddr_t iblk;
	int c, fd;


	/*
	 * Initialize tables between checks;
	 * because of the qsort done in report()
	 * the hash tables must be rebuilt each time.
	 */
	for (i = 0; i < TSIZE; i++)
		sizes[i] = 0;
	overflow = 0;
	for (dp = duhash; dp < &duhash[DUHASH]; dp++)
		*dp = 0;
	ndu = 0;
	fd = open(file, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "quot: ");
		perror(file);
		exit(32);
	}
	printf("%s", file);
	if (fsdir == NULL) {
		struct mnttab mntp;
		int	status;

		fstab = fopen(MNTTAB, "r");
        	if (fstab == NULL) {
                	fprintf(stderr, "quot: no %s file\n", MNTTAB);
                	exit(32);
        	}
		while ((status = getmntent(fstab, &mntp)) == NULL) {
			if  (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) != 0)
                        	continue;
			if  (strcmp(mntp.mnt_special, file) != 0)
				continue;
			fsdir = mntp.mnt_mountp;
		}
	}
	if (fsdir != NULL && *fsdir != '\0')
		printf(" (%s)", fsdir);
	printf(":\n");
	sync();
	bread(fd, SBLOCK, (char *)&sblock, SBSIZE);
	if (nflg) {
		if (isdigit(c = getchar()))
			ungetc(c, stdin);
		else while (c != '\n' && c != EOF)
			c = getchar();
	}
	nfiles = sblock.fs_ipg * sblock.fs_ncg;
	for (ino = 0; ino < nfiles; ) {
		iblk = fsbtodb(&sblock, itod(&sblock, ino));
		bread(fd, iblk, (char *)itab, sblock.fs_bsize);
		for (j = 0; j < INOPB(&sblock) && ino < nfiles; j++, ino++) {
			if (ino < UFSROOTINO)
				continue;
			acct(&itab[j]);
		}
	}
	close(fd);
	return (0);
}

acct(ip)
	register struct dinode *ip;
{
	register struct du *dp;
	struct du **hp;
	long blks, frags, size;
	int n;
	static fino;

	if (ip->di_eftflag != EFT_MAGIC) {
		ip->di_mode = ip->di_smode;
		ip->di_uid = ip->di_suid;
	}
	if ((ip->di_mode & IFMT) == 0)
		return;
	/*
	 * By default, take block count in inode.  Otherwise (-h),
	 * take the size field and estimate the blocks allocated.
	 * The latter does not account for holes in files.
	 */
	if (!hflg)
		size = ip->di_blocks / 2;
	else {
		blks = lblkno(&sblock, ip->di_size);
		frags = blks * sblock.fs_frag +
			numfrags(&sblock, dblksize(&sblock, ip, blks));
		size = frags * sblock.fs_fsize / 1024;
	}
	if (cflg) {
		if ((ip->di_mode&IFMT) != IFDIR && (ip->di_mode&IFMT) != IFREG)
			return;
		if (size >= TSIZE) {
			overflow += size;
			size = TSIZE-1;
		}
		sizes[size]++;
		return;
	}
	hp = &duhash[HASH(ip->di_uid)];
	for (dp = *hp; dp; dp = dp->next)
		if (dp->uid == ip->di_uid)
			break;
	if (dp == 0) {
		if (ndu >= NDU)
			return;
		dp = &du[ndu++];
		dp->next = *hp;
		*hp = dp;
		dp->uid = ip->di_uid;
		dp->nfiles = 0;
		dp->blocks = 0;
		dp->blocks30 = 0;
		dp->blocks60 = 0;
		dp->blocks90 = 0;
	}
	dp->blocks += size;
#define	DAY (60 * 60 * 24)	/* seconds per day */
	if (now - ip->di_atime > 30 * DAY)
		dp->blocks30 += size;
	if (now - ip->di_atime > 60 * DAY)
		dp->blocks60 += size;
	if (now - ip->di_atime > 90 * DAY)
		dp->blocks90 += size;
	dp->nfiles++;
	while (nflg) {
		register char *np;

		if (fino == 0)
			if (scanf("%d", &fino) <= 0)
				return;
		if (fino > ino)
			return;
		if (fino < ino) {
			while ((n = getchar()) != '\n' && n != EOF)
				;
			fino = 0;
			continue;
		}
		if (np = getname(dp->uid))
			printf("%.7s	", np);
		else
			printf("%d	", ip->di_uid);
		while ((n = getchar()) == ' ' || n == '\t')
			;
		putchar(n);
		while (n != EOF && n != '\n') {
			n = getchar();
			putchar(n);
		}
		fino = 0;
		break;
	}
}

bread(fd, bno, buf, cnt)
	unsigned bno;
	char *buf;
{

	lseek(fd, (long)bno * DEV_BSIZE, L_SET);
	if (read(fd, buf, cnt) != cnt) {
		fprintf(stderr, "quot: read error at block %u\n", bno);
		exit(32);
	}
}

qcmp(p1, p2)
	register struct du *p1, *p2;
{
	char *s1, *s2;

	if (p1->blocks > p2->blocks)
		return (-1);
	if (p1->blocks < p2->blocks)
		return (1);
	s1 = getname(p1->uid);
	if (s1 == 0)
		return (0);
	s2 = getname(p2->uid);
	if (s2 == 0)
		return (0);
	return (strcmp(s1, s2));
}

report()
{
	register i;
	register struct du *dp;

	if (nflg)
		return;
	if (cflg) {
		register long t = 0;

		for (i = 0; i < TSIZE - 1; i++)
			if (sizes[i]) {
				t += i*sizes[i];
				printf("%d	%d	%d\n", i, sizes[i], t);
			}
		if (sizes[TSIZE -1 ])
			printf("%d	%d	%d\n",
			    TSIZE - 1, sizes[TSIZE - 1], overflow + t);
		return;
	}
	qsort(du, ndu, sizeof (du[0]), qcmp);
	for (dp = du; dp < &du[ndu]; dp++) {
		register char *cp;

		if (dp->blocks == 0)
			return;
		printf("%5d\t", dp->blocks);
		if (fflg)
			printf("%5d\t", dp->nfiles);
		if (cp = getname(dp->uid))
			printf("%-8s", cp);
		else
			printf("#%-8d", dp->uid);
		if (vflg)
			printf("\t%5d\t%5d\t%5d",
			    dp->blocks30, dp->blocks60, dp->blocks90);
		printf("\n");
	}
}

#include <pwd.h>
#include <utmp.h>

struct	utmp utmp;

#define NUID	256		/* power of two */
#define UIDMASK	(NUID-1)

#define	NMAX	(sizeof (utmp.ut_name))
#define SCPYN(a, b)	strncpy(a, b, NMAX)

struct ncache {
	int	uid;
	char	name[NMAX + 1];
} nc[NUID];
int entriesleft = NUID;

char *
getname(uid)
	int uid;
{
	register struct passwd *pw;
	register struct ncache *ncp;
	extern struct passwd *getpwent();
	extern struct passwd *getpwuid();

	/*
	 * Check cache for name.
	 */
	ncp = &nc[uid & UIDMASK];
	if (ncp->uid == uid && ncp->name[0])
		return (ncp->name);
	if (entriesleft) {
		/*
		 * If we haven't gone through the passwd file then
		 * fill the cache while seaching for name.
		 * This lets us run through passwd serially.
		 */
		if (entriesleft == NUID)
			setpwent();
		while ((pw = getpwent()) && entriesleft) {
			entriesleft--;
			ncp = &nc[pw->pw_uid & UIDMASK];
			if (ncp->name[0] == '\0' || pw->pw_uid == uid) {
				SCPYN(ncp->name, pw->pw_name);
				ncp->uid = uid;
			}
			if (pw->pw_uid == uid)
				return (ncp->name);
		}
		endpwent();
		entriesleft = 0;
		ncp = &nc[uid & UIDMASK];
	}
	/*
	 * Not in cache. Do it the slow way.
	 */
	pw = getpwuid(uid);
	if (!pw)
		return (0);
	SCPYN(ncp->name, pw->pw_name);
	ncp->uid = uid;
	return (ncp->name);
}

getdev(devpp)
	char **devpp;
{
	struct stat statb;
	FILE *fstab;
	struct mnttab mntp;
	int	status;


	if (stat(*devpp, &statb) < 0) {
		perror(*devpp);
		exit(32);
	}
	if ((statb.st_mode & S_IFMT) == S_IFBLK ||
	    (statb.st_mode & S_IFMT) == S_IFCHR) 
		return (0);
	fstab = fopen(MNTTAB, "r");
	if (fstab == NULL) {
		fprintf(stderr, "quot: no %s file\n", MNTTAB);
		exit(32);
	}
	while ((status = getmntent(fstab, &mntp)) == NULL) {
		if (strcmp(mntp.mnt_mountp, *devpp) == 0) {
			if (strcmp(mntp.mnt_fstype, MNTTYPE_UFS) != 0) {
				fprintf(stderr,
				    "quot: %s not ufs filesystem\n",
				    *devpp);
				exit(32);
			}
			*devpp = malloc(strlen(mntp.mnt_special) + 1);
			strcpy(*devpp, mntp.mnt_special);
			fclose(fstab);
			return (0);
		}
	}
	fclose(fstab);
	exit(32);
}
