/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

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

#ident	"@(#)ufs.cmds:ufs/df/df.c	1.11.4.1"

/*
 * df
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mntent.h>
#include <sys/errno.h>
#include <sys/fs/ufs_fs.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>

#include <stdio.h>
#include <sys/mnttab.h>

void	usage(), pheader();
char	*mpath();
int	aflag = 0;		/* even the uninteresitng ones */
int	bflag = 0;		/* print only number of kilobytes free */
int	eflag = 0;		/* print only number of file entries free */
int	gflag = 0;		/* print entire statvfs structure */
int	iflag = 0;		/* information for inodes */
int	nflag = 0;		/* print VFStype name */
int	tflag = 0;		/* print totals */
int	errflag = 0;
char	*typestr = "ufs";
long	t_totalblks, t_avail, t_free, t_used, t_reserved;
int	t_inodes, t_iused, t_ifree;

extern	int	optind;
extern char	*optarg;

union {
	struct fs iu_fs;
	char dummy[SBSIZE];
} sb;
#define sblock sb.iu_fs

/*
 * This structure is used to build a list of mntent structures
 * in reverse order from /etc/mnttab.
 */
struct mntlist {
	struct mnttab *mntl_mnt;
	struct mntlist *mntl_next;
};

struct mntlist *mkmntlist();
struct mnttab *mntdup();

char *subopts [] = {
#define A_FLAG		0
	"a",
#define I_FLAG		1
	"i",
	NULL
};

int
main(argc, argv)
	int argc;
	char *argv[];
{
	struct mnttab 		mnt;
	int			opt;
	char	*suboptions,    *value;
	int			suboption;

	while ((opt = getopt (argc, argv, "begko:t")) != EOF) {
		switch (opt) {

		case 'b':		/* print only number of kilobytes free */
			bflag++;
			break;

		case 'e':
			eflag++;	/* print only number of file entries free */
			iflag++;
			break;

		case 'g':
			gflag++;
			break;

		case 'n':
			nflag++;
			break;

		case 'k':
			break;

		case 'o':
			/*
			 * ufs specific options.
			 */
			suboptions = optarg;
			while (*suboptions != '\0') {
				switch ((suboption = getsubopt(&suboptions, subopts, &value))) {

				case I_FLAG:		/* information for inodes */
					iflag++;
					break;

				default:
					usage ();
				}
			}
			break;

		case 't':		/* print totals */
			tflag++;
			break;

		case 'V':		/* Print command line */
			{
				char			*opt_text;
				int			opt_count;

				(void) fprintf (stdout, "df -F ufs ");
				for (opt_count = 1; opt_count < argc ; opt_count++) {
					opt_text = argv[opt_count];
					if (opt_text)
						(void) fprintf (stdout, " %s ", opt_text);
				}
				(void) fprintf (stdout, "\n");
			}
			break;

		case '?':
			errflag++;
		}
	}
	if (errflag)
		usage();
	if (gflag && iflag) {
		printf("df: '-g' and '-o i' are mutually exclusive\n");
		exit(32);
	}
	if (bflag || eflag)
		tflag = 0;
	sync();
	if (argc <= optind) {
		register FILE *mtabp;
		int	status;

		if ((mtabp = fopen(MNTTAB, "r")) == NULL) {
			(void) fprintf(stderr, "df: ");
			perror(MNTTAB);
			exit(32);
		}
		pheader();
		while ((status = getmntent(mtabp, &mnt)) == NULL) {
			if (strcmp(mnt.mnt_fstype, MNTTYPE_IGNORE) == 0 ||
			    strcmp(mnt.mnt_fstype, MNTTYPE_SWAP) == 0)
				continue;
			if (strcmp(typestr, mnt.mnt_fstype) != 0) {
				continue;
			}
			dfreemnt(mnt.mnt_mountp, &mnt);
		}
		if (tflag)
			if (iflag)
				print_itotals();
			else
				print_totals ();
		(void) fclose(mtabp);
	} else {
		int num = argc ;
		int i ;
		struct mntlist *mntl ;

		pheader();
		aflag++;
		/*
		 * Reverse the list and start comparing.
		 */
		for (mntl = mkmntlist(); mntl != NULL && num ; 
				mntl = mntl->mntl_next) {
		   struct stat dirstat, filestat ;

		   bcopy (mntl->mntl_mnt, &mnt, sizeof (mnt));
		   if (stat(mnt.mnt_mountp, &dirstat)<0) {
			continue ;
		   }
		   for (i = optind; i < argc; i++) {
			if (argv[i]==NULL) continue ;
			if (stat(argv[i], &filestat) < 0) {
				(void) fprintf(stderr, "df: ");
				perror(argv[i]);
				argv[i] = NULL ;
				--num;
			} else {
			       if ((filestat.st_mode & S_IFMT) == S_IFBLK ||
			          (filestat.st_mode & S_IFMT) == S_IFCHR) {
					char *cp ;

					cp = mpath(argv[i]);
					if (*cp == '\0') {
						dfreedev(argv[i]);
						argv[i] = NULL ;
						--num;
						continue;
					}
					else {
					  if (stat(cp, &filestat) < 0) {
						(void) fprintf(stderr, "df: ");
						perror(argv[i]);
						argv[i] = NULL ;
						--num;
						continue ;
					  }
					}
				}
				if (strcmp(mnt.mnt_fstype, MNTTYPE_IGNORE) == 0 ||
				    strcmp(mnt.mnt_fstype, MNTTYPE_SWAP) == 0)
					continue;
				if ((filestat.st_dev == dirstat.st_dev) &&
				    (strcmp(typestr, mnt.mnt_fstype)==0)) {
					dfreemnt(mnt.mnt_mountp, &mnt);
					argv[i] = NULL ;
					--num ;
				}
			}
		}
	     }
		if (tflag)
			if (iflag)
				print_itotals ();
			else
				print_totals ();
	     if (num) {
		     for (i = optind; i < argc; i++) 
			if (argv[i]==NULL) 
				continue ;
			else
			   (void) fprintf(stderr, 
				"Could not find mount point for %s\n", argv[i]) ;
	     }
	}
	exit(0);
	/*NOTREACHED*/
}

void
pheader()
{
	if (nflag)
		(void) printf("VFStype name - ufs\n");
	if (iflag)
		if (eflag)
			(void) printf("Filesystem            ifree\n");
		else
			(void) printf("Filesystem             iused   ifree  %%iused");
	else {
		if (gflag)
			(void) printf("Filesystem        f_type f_fsize f_bfree f_bavail f_files f_ffree f_fsid f_flag f_fstr\n");
		else
			if (bflag)
				(void) printf("Filesystem            avail\n");
			else {
				(void) printf("Filesystem            kbytes    used   avail capacity");
			}
		}
	if ((!eflag) && (!bflag) && (!gflag))
		(void) printf("  Mounted on\n");
}

/*
 * Report on a block or character special device.  Assumed not to be 
 * mounted.  N.B. checks for a valid UFS superblock.  
 */
dfreedev(file)
	char *file;
{
	long totalblks, availblks, avail, free, used;
	int fi;

	fi = open(file, 0);
	if (fi < 0) {
		(void) fprintf(stderr, "df: ");
		perror(file);
		return;
	}
	if (bread(file, fi, SBLOCK, (char *)&sblock, SBSIZE) == 0) {
		(void) close(fi);
		return;
	}
	if (sblock.fs_magic != FS_MAGIC) {
		(void) fprintf(stderr, "df: %s: not a ufs file system\n", 
		    file);
		(void) close(fi);
		return;
	}
	(void) printf("%-20.20s", file);
	if (iflag) {
		int inodes = sblock.fs_ncg * sblock.fs_ipg;
		used = inodes - sblock.fs_cstotal.cs_nifree;
		if (eflag)
			(void) printf("%8ld\n",
			    sblock.fs_cstotal.cs_nifree);
		else
			(void) printf("%8ld%8ld%6.0f%% ", used, sblock.fs_cstotal.cs_nifree,
			    inodes == 0 ? 0.0 : (double)used / (double)inodes * 100.0);
		if (tflag) {
			t_inodes += inodes;
			t_iused += used;
			t_ifree += sblock.fs_cstotal.cs_nifree;
		}
	} else {
		totalblks = sblock.fs_dsize;
		free = sblock.fs_cstotal.cs_nbfree * sblock.fs_frag +
		    sblock.fs_cstotal.cs_nffree;
		used = totalblks - free;
		availblks = totalblks * (100 - sblock.fs_minfree) / 100;
		avail = availblks > used ? availblks - used : 0;
		if (bflag) {
			(void) printf("%8d\n",
			    avail* sblock.fs_fsize / 1024);
		} else {
			(void) printf("%8d%8d%8d",
			    totalblks * sblock.fs_fsize / 1024,
			    used * sblock.fs_fsize / 1024,
			    avail * sblock.fs_fsize / 1024);
			(void) printf("%6.0f%%",
			    availblks==0? 0.0: (double)used/(double)availblks * 100.0);
			(void) printf("  ");
		}
		if (tflag) {
			t_totalblks += totalblks * sblock.fs_fsize / 1024;
			t_used += used * sblock.fs_fsize / 1024;
			t_avail += avail * sblock.fs_fsize / 1024;
			t_free += free;
		}
	}
	if ((!bflag) && (!eflag))
		(void) printf("  %s\n", mpath(file));
	(void) close(fi);
}

dfreemnt(file, mnt)
	char *file;
	struct mnttab *mnt;
{
	struct statvfs fs;

	if (statvfs(file, &fs) < 0) {
		(void) fprintf(stderr, "df: ");
		perror(file);
		return;
	}

	if (fs.f_blocks == 0) {
		return;
	}
	if (strlen(mnt->mnt_special) > 20) {
		(void) printf("%s\n", mnt->mnt_special);
		(void) printf("                    ");
	} else {
		(void) printf("%-20.20s", mnt->mnt_special);
	}
	if (iflag) {
		long files, used;

		files = fs.f_files;
		used = files - fs.f_ffree;
		if (eflag)
			(void) printf("%8ld\n",
			    fs.f_ffree);
		else
			(void) printf("%8ld%8ld%6.0f%% ", used, fs.f_ffree,
			    files == 0? 0.0: (double)used / (double)files * 100.0);
		if (tflag) {
			t_inodes += files;
			t_iused += used;
			t_ifree += fs.f_ffree;
		}
	} else {
		if (gflag) {
			print_statvfs (&fs);
		} else {
			long totalblks, avail, free, used, reserved;

			totalblks = fs.f_blocks;
			free = fs.f_bfree;
			used = totalblks - free;
			avail = fs.f_bavail;
			reserved = free - avail;
			if (avail < 0)
				avail = 0;
				if (bflag) {
					(void) printf("%8d\n",
					    avail * fs.f_frsize / 1024);
				} else {
				(void) printf("%8d%8d%8d", totalblks * fs.f_frsize / 1024,
				    used * fs.f_frsize / 1024, avail * fs.f_frsize / 1024);
				totalblks -= reserved;
				(void) printf("%6.0f%%",
				    totalblks==0? 0.0: (double)used/(double)totalblks * 100.0);
			(void) printf("  ");
			if (tflag) {
				t_totalblks += (totalblks + reserved) * fs.f_bsize / 1024;
				t_reserved += reserved;
				t_used += used * fs.f_frsize / 1024;
				t_avail += avail * fs.f_frsize / 1024;
				t_free += free;
			}
		}
		}
	}
	if ((!bflag) && (!eflag) && (!gflag))
		(void) printf("  %s\n", mnt->mnt_mountp);
}

/*
 * Given a name like /dev/dsk/c1d0s2, returns the mounted path, like /usr.
 */
char *
mpath(file)
	char *file;
{
	FILE *mntp;
	struct mnttab 	mnt;
	int	status;

	if ((mntp = fopen(MNTTAB, "r")) == 0) {
		(void) fprintf(stderr, "df: ");
		perror(MNTTAB);
		exit(32);
	}

	while ((status = getmntent(mntp, &mnt)) == 0) {
		if (strcmp(file, mnt.mnt_special) == 0) {
			(void) fclose(mntp);
			return (mnt.mnt_mountp);
		}
	}
	(void) fclose(mntp);
	return "";
}

long lseek();

int
bread(file, fi, bno, buf, cnt)
	char *file;
	int fi;
	daddr_t bno;
	char *buf;
	int cnt;
{
	register int n;
	extern int errno;

	(void) lseek(fi, (long)(bno * DEV_BSIZE), 0);
	if ((n = read(fi, buf, cnt)) < 0) {
		/* probably a dismounted disk if errno == EIO */
		if (errno != EIO) {
			(void) fprintf(stderr, "df: read error on ");
			perror(file);
			(void) fprintf(stderr, "bno = %ld\n", bno);
		} else {
			(void) fprintf(stderr, "df: premature EOF on %s\n",
			    file);
			(void) fprintf(stderr, 
			   "bno = %ld expected = %d count = %d\n", bno, cnt, n);
		}
		return (0);
	}
	return (1);
}

char *
xmalloc(size)
	unsigned int size;
{
	register char *ret;
	char *malloc();
	
	if ((ret = (char *)malloc(size)) == NULL) {
		(void) fprintf(stderr, "umount: ran out of memory!\n");
		exit(32);
	}
	return (ret);
}

struct mnttab *
mntdup(mnt)
	register struct mnttab *mnt;
{
	register struct mnttab *new;

	new = (struct mnttab *)xmalloc(sizeof(*new));

	new->mnt_special = (char *)xmalloc((unsigned)(strlen(mnt->mnt_special) + 1));
	(void) strcpy(new->mnt_special, mnt->mnt_special);

	new->mnt_mountp = (char *)xmalloc((unsigned)(strlen(mnt->mnt_mountp) + 1));
	(void) strcpy(new->mnt_mountp, mnt->mnt_mountp);

	new->mnt_fstype = (char *)xmalloc((unsigned)(strlen(mnt->mnt_fstype) + 1));
	(void) strcpy(new->mnt_fstype, mnt->mnt_fstype);

	new->mnt_mntopts = (char *)xmalloc((unsigned)(strlen(mnt->mnt_mntopts) + 1));
	(void) strcpy(new->mnt_mntopts, mnt->mnt_mntopts);

#ifdef never
	new->mnt_freq = mnt->mnt_freq;
	new->mnt_passno = mnt->mnt_passno;
#endif /* never */

	return (new);
}

void
usage()
{

	(void) fprintf(stderr, "ufs usage: df [-F ufs] [generic options] [-o i] [directory | special]\n");
	exit(32);
}

struct mntlist *
mkmntlist()
{
	FILE *mounted;
	struct mntlist *mntl;
	struct mntlist *mntst = NULL;
	struct mnttab mnt;
	int	status;

	if ((mounted = fopen(MNTTAB, "r"))== NULL) {
		(void) fprintf(stderr, "df : ") ;
		perror(MNTTAB);
		exit(32);
	}
	while ((status = getmntent(mounted, &mnt)) == NULL) {
		mntl = (struct mntlist *)xmalloc(sizeof(*mntl));
		mntl->mntl_mnt = mntdup(&mnt);
		mntl->mntl_next = mntst;
		mntst = mntl;
	}
	(void) fclose(mounted);
	return(mntst);
}

print_statvfs (fs)
	struct statvfs	*fs;
{
	int	i;

	for (i = 0; i < FSTYPSZ; i++)
		(void) printf ("%c", fs->f_basetype[i]);
	(void) printf ("%8d%8d%8d",
	    fs->f_frsize,
	    fs->f_blocks,
	    fs->f_bavail);
	(void) printf ("%8d%8d%8d\n",
	    fs->f_files,
	    fs->f_ffree,
	    fs->f_fsid);
	(void) printf ("0x%x ",
	    fs->f_flag);
	for (i= 0; i < 14; i++)
	(void) printf ("%c", (fs->f_fstr[i] == '\0')? ' ':
	    fs->f_fstr[i]);
	printf("\n");
}

print_totals ()
{
	(void) printf ("Totals              %8d%8d%8d",t_totalblks, t_used, t_avail);
	(void) printf("%6.0f%%\n",
	    (t_totalblks - t_reserved)==0?
		0.0: (double)t_used/(double)(t_totalblks - t_reserved) * 100.0);
}

print_itotals ()
{
	(void) printf ("Totals              %8d%8d%6.0f%%\n",
	    t_iused,
	    t_ifree,
	    t_inodes == 0 ? 0.0 : (double)t_iused / (double)t_inodes * 100.0);
}
