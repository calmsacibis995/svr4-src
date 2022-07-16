/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbdf:df.c	1.2.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

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
int	iflag;
int	aflag;
int	type;
char	*typestr;

union {
	struct fs iu_fs;
	char dummy[SBSIZE];
} sb;
#define sblock sb.iu_fs

/*
 * This structure is used to build a list of mntent structures
 * in reverse order from /etc/mtab.
 */
struct mntlist {
	struct mnttab *mntl_mnt;
	struct mntlist *mntl_next;
};

struct mntlist *mkmntlist();
struct mnttab *mntdup();
int globstat = 0;

int
main(argc, argv)
	int argc;
	char **argv;
{
	struct mnttab mnt;
	int status;

	/*
	 * Skip over command name, if present.
	 */
	if (argc > 0) {
		argv++;
		argc--;
	}

	while (argc > 0 && (*argv)[0]=='-') {
		switch ((*argv)[1]) {

		case 'i':
			iflag++;
			break;

		case 't':
			type++;
			argv++;
			argc--;
			if (argc <= 0)
				usage();
			typestr = *argv;
			break;

		case 'a':
			aflag++;
			break;

		default:
			usage();
		}
		argc--, argv++;
	}
	if (argc > 0 && type) {
		usage();
	}
	sync();
	if (argc <= 0) {
		register FILE *mtabp;

                if ((mtabp = fopen(MNTTAB, "r")) == NULL) {
                        (void) fprintf(stderr, "df: ");
                        perror(MNTTAB);
                        exit(1);
                }
		pheader();
		while ((status = getmntent(mtabp, &mnt)) == NULL) {
			if (strcmp(mnt.mnt_fstype, MNTTYPE_IGNORE) == 0 ||
			    strcmp(mnt.mnt_fstype, MNTTYPE_SWAP) == 0)
				continue;
			if (type && strcmp(typestr, mnt.mnt_fstype) != 0) {
				continue;
			}
			dfreemnt(mnt.mnt_mountp, &mnt);
		}
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

		   bcopy(mntl->mntl_mnt, &mnt, sizeof(mnt));
		   if (stat(mnt.mnt_mountp, &dirstat)<0) {
			continue ;
		   }
		   for (i = 0; i < argc; i++) {
			if (argv[i]==NULL) continue ;
			if (stat(argv[i], &filestat) < 0) {
				(void) fprintf(stderr, "df: ");
				perror(argv[i]);
				globstat = 1;
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
						globstat = 1;
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
				    (!type || strcmp(typestr, mnt.mnt_fstype)==0)){
					dfreemnt(mnt.mnt_mountp, &mnt);
					argv[i] = NULL ;
					--num ;
				}
			}
		}
	     }
	     if (num) {
		     for (i = 0; i < argc; i++) 
			if (argv[i]==NULL) 
				continue ;
			else {
				(void) fprintf(stderr, 
				"Could not find mount point for %s\n", argv[i]) ;
				globstat=1;
			}
	     }
	}
	exit(globstat);
	/*NOTREACHED*/
}

void
pheader()
{
	if (iflag)
		(void) printf("Filesystem             iused   ifree  %%iused");
	else
		(void) printf("Filesystem            kbytes    used   avail capacity");
	(void) printf("  Mounted on\n");
}

/*
 * Report on a block or character special device.  Assumed not to be 
 * mounted.  N.B. checks for a valid 4.2BSD superblock.  
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
		globstat = 1;
		return;
	}
	if (bread(file, fi, SBLOCK, (char *)&sblock, SBSIZE) == 0) {
		(void) close(fi);
		return;
	}
	if (sblock.fs_magic != FS_MAGIC) {
		(void) fprintf(stderr, "df: %s: not a UNIX filesystem\n", 
		    file);
		(void) close(fi);
		globstat = 1;
		return;
	}
	(void) printf("%-20.20s", file);
	if (iflag) {
		int inodes = sblock.fs_ncg * sblock.fs_ipg;
		used = inodes - sblock.fs_cstotal.cs_nifree;
		(void) printf("%8ld%8ld%6.0f%% ", used, sblock.fs_cstotal.cs_nifree,
		    inodes == 0 ? 0.0 : (double)used / (double)inodes * 100.0);
	} else {
		totalblks = sblock.fs_dsize;
		free = sblock.fs_cstotal.cs_nbfree * sblock.fs_frag +
		    sblock.fs_cstotal.cs_nffree;
		used = totalblks - free;
		availblks = totalblks * (100 - sblock.fs_minfree) / 100;
		avail = availblks > used ? availblks - used : 0;
		(void) printf("%8d%8d%8d",
		    totalblks * sblock.fs_fsize / 1024,
		    used * sblock.fs_fsize / 1024,
		    avail * sblock.fs_fsize / 1024);
		(void) printf("%6.0f%%",
		    availblks==0? 0.0: (double)used/(double)availblks * 100.0);
		(void) printf("  ");
	}
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
		globstat = 1;
		return;
	}

	if (!aflag && fs.f_blocks == 0) {
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
		(void) printf("%8ld%8ld%6.0f%% ", used, fs.f_ffree,
		    files == 0? 0.0: (double)used / (double)files * 100.0);
	} else {
		long totalblks, avail, free, used, reserved;

		totalblks = fs.f_blocks;
		free = fs.f_bfree;
		used = totalblks - free;
		avail = fs.f_bavail;
		reserved = free - avail;
		if (avail < 0)
			avail = 0;
		(void) printf("%8d%8d%8d", totalblks * fs.f_bsize / 1024,
		    used * fs.f_bsize / 1024, avail * fs.f_bsize / 1024);
		totalblks -= reserved;
		(void) printf("%6.0f%%",
		    totalblks==0? 0.0: (double)used/(double)totalblks * 100.0);
		(void) printf("  ");
	}
	(void) printf("  %s\n", mnt->mnt_mountp);
}

/*
 * Given a name like /dev/rrp0h, returns the mounted path, like /usr.
 */
char *
mpath(file)
	char *file;
{
	FILE *mntp;
	struct mnttab mnt;
	int status;

        if ((mntp = fopen(MNTTAB, "r")) == 0) {
                (void) fprintf(stderr, "df: ");
		perror(MNTTAB);
                exit(1);
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
	if ((n = read(fi, buf, (unsigned) cnt)) < 0) {
		/* probably a dismounted disk if errno == EIO */
		if (errno != EIO) {
			(void) fprintf(stderr, "df: read error on ");
			perror(file);
			(void) fprintf(stderr, "bno = %ld\n", bno);
			globstat = 1;
		} else {
			(void) fprintf(stderr, "df: premature EOF on %s\n",
			    file);
			(void) fprintf(stderr, 
			   "bno = %ld expected = %d count = %d\n", bno, cnt, n);
			globstat = 1;
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
		exit(1);
	}
	return (ret);
}

struct mnttab *
mntdup(mnt)
	register struct mnttab *mnt;
{
	register struct mnttab *new;

	new = (struct mnttab *)xmalloc(sizeof(*new));

	new->mnt_special = (char *)xmalloc(strlen(mnt->mnt_special) + 1);
	(void) strcpy(new->mnt_special, mnt->mnt_special);

	new->mnt_mountp = (char *)xmalloc(strlen(mnt->mnt_mountp) + 1);
	(void) strcpy(new->mnt_mountp, mnt->mnt_mountp);

	new->mnt_fstype = (char *)xmalloc(strlen(mnt->mnt_fstype) + 1);
	(void) strcpy(new->mnt_fstype, mnt->mnt_fstype);

	new->mnt_mntopts = (char *)xmalloc(strlen(mnt->mnt_mntopts) + 1);
	(void) strcpy(new->mnt_mntopts, mnt->mnt_mntopts);

#ifdef never
	new->mnt_freq = mnt->mnt_freq;
	new->mnt_passno = mnt->mnt_passno;
#endif

	return (new);
}

void
usage()
{

	(void) fprintf(stderr, "usage: df [ -i ] [ -a ] [ -t type | file... ]\n");
	exit(1);
}

struct mntlist *
mkmntlist()
{
	FILE *mounted;
	struct mntlist *mntl;
	struct mntlist *mntst = NULL;
	struct mnttab mnt;
	int status;

        if ((mounted = fopen(MNTTAB, "r")) == 0) {
                (void) fprintf(stderr, "df: ");
                perror(MNTTAB);
                exit(1);
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
