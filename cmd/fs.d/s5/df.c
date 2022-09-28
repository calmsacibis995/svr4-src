/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)s5.cmds:df.c	1.15.4.1"
/* s5 df */
#include <stdio.h>
#include <sys/param.h>
#include <sys/fs/s5param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/mnttab.h>
#include <sys/stat.h>
#include <ustat.h>
#include <sys/statvfs.h>
#include <sys/fs/s5filsys.h>
#include <sys/fs/s5fblk.h>
#include <sys/fs/s5dir.h>
#include <setjmp.h>
#include <sys/errno.h>
#include <string.h>
#include <sys/vnode.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5ino.h>

#define EQ(x,y) (strcmp(x,y)==0)
#define MOUNTED fs_name[0]
#define DEVLEN	1024
#define DEVLSZ 200	/* number of unique file systems we can remember */

#ifndef i386
#define PHYSBLKSZ 512	/* size of physical disk block */
#endif

extern char *optarg;
extern int optind, opterr;
extern int errno;
char *devnm();

struct	mnttab	Mp;
struct stat	S;
struct filsys	sblock;
struct ustat	*Fs = (struct ustat *)&sblock.s_tfree;
struct statvfs	Fs_info;

int	physblks;
long	bsize;
int 	bshift;
int	freflg, totflg, bflg, eflg, kflg,ebflg=0;
int	header=0;
int	fd;
#if u3b2 || u3b15
int	result;
#endif
daddr_t	blkno	= 1;
daddr_t	alloc();

char nfsmes[] = "s5 df: %-10.32s \
is not a special device or directory\n";

char nfsmes2[] = "s5 df: %-10.32s \
is not a s5 file system\n";

char usage[] = "s5 Usage:\ndf [-F s5] [generic_options] [-f] [directory | special ...]\n";

char dev_usage[] = "Usage:\ndevnm [directory]\n";

char	mnttab[] = MNTTAB;

char *
basename(s)
char *s;
{
	int n = 0;

	while(*s++ != '\0') n++;
	while(n-- > 0)
		if(*(--s) == '/') 
			return(++s);
	return(--s);
}

main(argc, argv)
char **argv;
{
	FILE	*fi;
	register i;
	char	 *res_name, *s;
	int	 c, k, errcnt = 0;

	s = basename(argv[0]);
	if(!strncmp(s,"devnm",5)) {
		if (argc < 2) {
			fprintf(stderr,dev_usage);
			exit(31);
		}
		while(--argc) {
			if(stat(*++argv, &S) == -1) {
				fprintf(stderr,"devnm: %s not found\n",*argv);
				errcnt++;
				continue;
			}
			res_name = devnm();
			if(res_name[0] != '\0')
				printf("%s %s\n", res_name, *argv);
			else {
				fprintf(stderr,"devnm: %s not found\n", *argv);
				errcnt++;
			}
		}
		if (errcnt != 0 ) {
			exit(31+errcnt);
		}
		else {
			exit(0);
		}
	}
	while((c = getopt(argc,argv,"?bektf")) != EOF) {
		switch(c) {
			case 'b':	/* print blocks free */
				bflg++;
				break;

			case 'e':	/* print only file entries free */
				eflg++;
				break;

			case 'k':	/* print allocation in kilobytes */
				kflg++;
				break;

			case 'f':	/* freelist count only */
				freflg++;
				break;

			case 't':	/* include total allocations */
				totflg = 1;
				break;

			case '?':	/* usage message */

			default:
				fprintf(stderr,usage);
				exit(31+1);
		}
	}

	if((fi = fopen(mnttab, "r")) == NULL) {
		fprintf(stderr,"s5 df: cannot open %s\n",mnttab);
		exit(31+1);
	}
	if (eflg && bflg) {
		header++;
		ebflg++;
	}
	while ((i = getmntent(fi, &Mp)) == 0) {
		if(argc > optind) {
				/* we are looking for specific file systems
				   in the mount table */
			for(k=optind;k < argc; k++) {
				if(argv[k][0] == '\0')
					continue;
				if(stat(argv[k], &S) == -1)
					S.st_dev = -1;

					/* does the argument match either
					   the mountpoint or the device? */
				if(EQ(argv[k], Mp.mnt_special)
				|| EQ(argv[k], Mp.mnt_mountp)) {
					errcnt += printit(Mp.mnt_special, Mp.mnt_mountp);
					argv[k][0] = '\0';
				} else	
					/* or is it on one of 
						the mounted devices? */
					if( ((S.st_mode & S_IFMT) == S_IFDIR)
					&& (EQ(devnm(), Mp.mnt_special))) {
						/* must re-do stat cuz
						   devnm() wiped it out */
						if(stat(argv[k], &S) == -1)
							S.st_dev = -1;
						errcnt += printit(Mp.mnt_special, argv[k]);
						argv[k][0] = '\0';
					}
			}
		} else {
				/* doing all things in the mount table */
			if(stat(Mp.mnt_mountp, &S) == -1)
				S.st_dev = -1;
			errcnt += printit(Mp.mnt_special, Mp.mnt_mountp);
		}
	}

	/* process arguments that were not in /etc/mnttab */
	for(i = optind; i < argc; ++i) {
		if(argv[i][0])
			errcnt += printit(argv[i], "\0");
	}
	fclose(fi);
	if (errcnt) {
		exit(31+errcnt);
	}
	exit(0);
}

printit(dev, fs_name)
char *dev, *fs_name;
{

	if(!MOUNTED || freflg) {	
		if((fd = open(dev, 0)) < 0) {
			fprintf(stderr,"s5 df: cannot open %s\n",dev);
			close(fd);
			return(1);
		}
		if(stat(dev, &S) < 0) {
			fprintf(stderr, "s5 df: cannot stat %s\n", dev);
			close(fd);
			return(1);
		}

		if(((S.st_mode & S_IFMT) == S_IFREG) 
		|| ((S.st_mode & S_IFMT) == S_IFIFO)) {
			printf(nfsmes, dev);
			close(fd);
			return(1);
		}

		sync();
		if(lseek(fd, (long)SUPERBOFF, 0) < 0
		|| read(fd, &sblock, (sizeof sblock)) != (sizeof sblock)) {
			close(fd);
			return(1);
		}	
		if(sblock.s_magic == FsMAGIC) {
			if(sblock.s_type == Fs1b) {
				physblks = 1;
				bsize = 512;
				bshift = 9;
#if u3b2 || u3b15
			} else if(sblock.s_type == Fs2b) {
				result = s5bsize(fd);
				if(result == Fs2b) {
					physblks = 2;
					bsize = 1024;
					bshift = 10;
				} else if(result == Fs4b) {
					physblks = 4;
					bsize = 2048;
					bshift = 11;
				} else {
					printf("          (%-12s): can't verify logical block size\n\t\t\t  root inode or root directory may be corrupted\n", dev);
					return(1);
				}
			} else if(sblock.s_type == Fs4b) {
				physblks = 4;
				bsize = 2048;
				bshift = 11;
#else				
			} else if(sblock.s_type == Fs2b) {
				physblks = 2;
				bsize = 1024;
				bshift = 10;
#endif

#ifdef i386
			} else if(sblock.s_type == Fs4b) {
				physblks = 4;
				bsize = 2048;
				bshift = 11;
#endif

			} else {
				printf("          (%-12s): can't determine logical block size\n", dev);
				return(1);
			}
		} else {
			physblks = 1;
			bsize = 512;
			bshift = 9;
		}
		/* file system sanity test */
		if(sblock.s_fsize <= (daddr_t)sblock.s_isize
		|| sblock.s_fsize < Fs->f_tfree
		|| sblock.s_isize < (ushort)Fs->f_tinode*sizeof(struct dinode)/bsize
		|| (long)sblock.s_isize*bsize/sizeof(struct dinode) > 0x10000L) {
			printf(nfsmes2, dev);
			return(1);
		}
		
	} else {	/* mounted file system */

		if(statvfs(fs_name, &Fs_info) != 0) {
			fprintf(stderr, "s5 df: cannot statvfs %s\t", dev);
			perror(" statvfs");
			return(1);
		}
			/* copy statvfs info into superblock structure */
		Fs->f_tfree = Fs_info.f_bfree*(Fs_info.f_frsize/512);
		Fs->f_tinode = Fs_info.f_ffree;
		strncpy(Fs->f_fname,Fs_info.f_fstr,6);
		strncpy(Fs->f_fpack,&Fs_info.f_fstr[6],6);
		physblks = 1;
	}
	if (kflg) {

		long cap, kbytes, used, avail;

		if (MOUNTED) 
			kbytes = Fs_info.f_blocks*Fs_info.f_frsize/1024;
		else
			kbytes=sblock.s_fsize*physblks/2;
		avail= (Fs->f_tfree*physblks)/2;
		used = kbytes - avail;
		cap = kbytes > 0 ? ((used*100L)/kbytes + 0.5) : 0;
		printf("filesystem         kbytes   used     avail    capacity  mounted on\n");
		printf("%-18s %-8ld %-8ld %-8ld %2ld%%        %-19.6s\n", 
			dev,
			kbytes,
			used,
			avail,
			cap,
			fs_name);
		exit(0);
	}

	if(!freflg) {
		if(totflg) {
			if(!MOUNTED) 
				printf("%-19.6s", sblock.s_fname);
			else
				printf("%-19.6s", fs_name);
			printf("(%-16s):", dev);
			printf("  %8d blocks%8u files\n",
				Fs->f_tfree*physblks, Fs->f_tinode);
			printf("                                total:");
			if(MOUNTED && !freflg)
				printf("  %8d blocks%8u files\n",
					Fs_info.f_blocks*(Fs_info.f_frsize/512),
					Fs_info.f_files);
			else
				printf("  %8d blocks%8u files\n",
					sblock.s_fsize*physblks, 
					(unsigned)((sblock.s_isize - 2)
						*bsize/sizeof(struct dinode)));
			exit(0);
		}
		if ( bflg || eflg ) {
			if (bflg)  {
				if (ebflg) {
					if(!MOUNTED) 
						printf("%-19.6s", sblock.s_fname);
					else
						printf("%-19.6s", fs_name);
					printf("(%-16s):  %8d kilobytes\n", 
					dev,
					((Fs->f_tfree*physblks)/2));
				}
				else {
					if (!header) {
						printf("Filesystem              avail\n");
						header++;
					}
					if(!MOUNTED) {
						if (sblock.s_fname[0] != NULL) {
							printf("%-19.6s", sblock.s_fname);
						}
						else {
							printf("%-19s", dev);
						}
					}
					else
						printf("%-19.6s", fs_name);
					printf("     %-8d\n", ((Fs->f_tfree*physblks)/2));
				
				}
			}
			if (eflg) {
				if (ebflg) {
					if(!MOUNTED) 
						printf("%-19.6s", sblock.s_fname);
					else
						printf("%-19.6s", fs_name);
					printf("(%-16s):", dev);
					printf("  %8u files\n", Fs->f_tinode);
				}
				else {
					if (!header) {
						printf("Filesystem              ifree\n");
						header++;
					}
					if(!MOUNTED) {
						if (sblock.s_fname[0] != NULL) {
							printf("%-19.6s", sblock.s_fname);
						}
						else {
							printf("%-19s", dev);
						}
					}
					else
						printf("%-19.6s", fs_name);
					printf("     %-8u\n", Fs->f_tinode);
				}
			}
		}
		else {
			if(!MOUNTED) 
				printf("%-19.6s", sblock.s_fname);
			else
				printf("%-19.6s", fs_name);
			printf("(%-16s):", dev);
			printf("  %8d blocks%8u files\n",
				Fs->f_tfree*physblks, Fs->f_tinode);
		}
		
	} else {
		daddr_t	i;

		i = 0;
		while(alloc())
			i++;
		if(!MOUNTED) 
			printf("%-19.6s", sblock.s_fname);
		else
			printf("%-19.6s", fs_name);
		if(totflg) {
			printf("(%-16s):  %8d blocks%8u files\n",dev, i*physblks, Fs->f_tinode); 
			printf("                                total:");
			if(MOUNTED && !freflg)
				printf("  %8d blocks%8u files\n",
					Fs_info.f_blocks*(Fs_info.f_frsize/512),
					Fs_info.f_files);
			else
				printf("  %8d blocks%8u files\n",
					sblock.s_fsize*physblks, 
					(unsigned)((sblock.s_isize - 2)
						*bsize/sizeof(struct dinode)));
			exit(0);
		}
		printf("(%-16s):  %8d blocks\n",dev, i*physblks);
		if (bflg) { 
			if(!MOUNTED) 
				printf("%-19.6s", sblock.s_fname);
			else
				printf("%-19.6s", fs_name);
			printf("(%-16s):  %8d kilobytes\n",dev, ((i*physblks)/2));
		}
		if (eflg) {
			if(!MOUNTED) 
				printf("%-19.6s", sblock.s_fname);
			else
				printf("%-19.6s", fs_name);
			printf("(%-16s):  %8u files\n",dev, Fs->f_tinode);
		}
		close(fd);
	}
	return(0);
}

daddr_t
alloc()
{
	int i;
	daddr_t	b;
		/* Would you believe the 3b20 dfc requires read counts mod 64?
		   Believe it! (see dfc1.c).
		*/
	char buf[(sizeof(struct fblk) + 63) & ~63];
	struct fblk *fb = (struct fblk *)buf;

	if (!sblock.s_nfree)
		return(0);	
	i = --sblock.s_nfree;
	if(i>=NICFREE) {
		printf("bad free count, b=%ld\n", blkno);
		return(0);
	}
	b = sblock.s_free[i];
	if(b == 0)
		return(0);
	if(b<(daddr_t)sblock.s_isize || b>=sblock.s_fsize) {
		printf("bad free block (%ld)\n", b);
		return(0);
	}
	if(sblock.s_nfree <= 0) {

		bread(b, buf, sizeof(buf));
		blkno = b;
		sblock.s_nfree = fb->df_nfree;
		for(i=0; i<NICFREE; i++)
			sblock.s_free[i] = fb->df_free[i];
	}
	return(b);
}

bread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	int n;

	if((lseek(fd, bno<<bshift, 0)) < 0) {
		perror("bread: seek error");
		exit(31+1);
	}
	if((n=read(fd, buf, cnt)) != cnt) {
		perror("bread: read error");
		printf("read error %x, count = %d\n", bno, n);
		exit(31+1);
	}
}

struct direct dbuf;

char *
devnm()
{
	int i;
	static dev_t fno;
	static struct devs {
		char *devdir;
		short dfd;
	} devd[] = {		/* in order of desired search */
		"/dev/dsk",0,
		"/dev",0,
		"/dev/rdsk",0
	};
	static char devnam[DEVLEN];

	devnam[0] = '\0';
	if(!devd[1].dfd) {	/* if /dev isn't open, nothing happens */
		for(i = 0; i < 3; i++) {
			devd[i].dfd = open(devd[i].devdir, 0);
		}
	}
	fno = S.st_dev;

	for(i = 0; i < 3; i++) {
		if((devd[i].dfd >= 0)
		   && (chdir(devd[i].devdir) == 0) 
		   && (dsearch(devd[i].dfd,fno))) {
			strcpy(devnam, devd[i].devdir);
			strcat(devnam,"/");
			strncat(devnam,dbuf.d_name,MAXNAMELEN);
			return(devnam);
		}
	}
	return(devnam);

}

dsearch(ddir,fno)
short ddir;
dev_t  fno;
{
	lseek(ddir, (long)0, 0);
	while(read(ddir, &dbuf, sizeof dbuf) == sizeof dbuf) {
		if(!dbuf.d_ino) continue;
		if(stat(dbuf.d_name, &S) == -1) {
			fprintf(stderr, "devnm: cannot stat %s\n",dbuf.d_name);
			return(0);
		}
		if((fno != S.st_rdev) 
		|| ((S.st_mode & S_IFMT) != S_IFBLK)
		|| (strcmp(dbuf.d_name,"swap") == 0)
		|| (strcmp(dbuf.d_name,"pipe") == 0)
			) continue;
		return(1);
	}
	return(0);
}
#if u3b2 || u3b15
/* heuristic function to determine logical block size of System V file system */

s5bsize(fd)
int fd;
{

	int results[3];
	int count;
	long address;
	long offset;
	char *buf;
	struct dinode *inodes;
	struct direct *dirs;
	char * p1;
	char * p2;
	
	results[1] = 0;
	results[2] = 0;

	buf = (char *)malloc(PHYSBLKSZ);

	for (count = 1; count < 3; count++) {

		address = 2048 * count;
		if (lseek(fd, address, 0) != address)
			continue;
		if (read(fd, buf, PHYSBLKSZ) != PHYSBLKSZ)
			continue;
		inodes = (struct dinode *)buf;
		if ((inodes[1].di_mode & S_IFMT) != S_IFDIR)
			continue;
		if (inodes[1].di_nlink < 2)
			continue;
		if ((inodes[1].di_size % sizeof(struct direct)) != 0)
			continue;
	
		p1 = (char *) &address;
		p2 = inodes[1].di_addr;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1   = *p2;
	
		offset = address << (count + 9);
		if (lseek(fd,offset,0) != offset)
			continue;
		if (read(fd, buf, PHYSBLKSZ) != PHYSBLKSZ)
			continue;
		dirs = (struct direct *)buf;
		if (dirs[0].d_ino != 2 || dirs[1].d_ino != 2 )
			continue;
		if (strcmp(dirs[0].d_name,".") || strcmp(dirs[1].d_name,".."))
			continue;
		results[count] = 1;
		}
	free(buf);
	
	if(results[1])
		return(Fs2b);
	if(results[2])
		return(Fs4b);
	return(-1);
}
#endif
