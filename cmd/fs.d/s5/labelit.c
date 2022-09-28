/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)s5.cmds:labelit.c	1.35"
/* labelit with 512, 1K, and 2K block size support */
#include <stdio.h>
#include <sys/param.h>
#ifndef	RT
#include <signal.h>
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5dir.h>
#include <sys/fs/s5ino.h>
#include <sys/sysmacros.h>
#include <sys/fs/s5filsys.h>
#include <locale.h>

#ifndef i386
#define PHYSBLKSZ 512
#endif

 /* write fsname, volume # on disk superblock */
struct {
	char fill1[SUPERBOFF];
	union {
		char fill2[UBSIZE];
		struct filsys fs;
	} f;
} super;
#ifdef RT
#define IFTAPE(s) (equal(s,"/dev/mt",7)||equal(s,"mt",2))
#define TAPENAMES "'/dev/mt'"
#else
#define IFTAPE(s) (equal(s,"/dev/rmt",8)||equal(s,"rmt",3)||equal(s,"/dev/rtp",8)||equal(s,"rtp",3))
#define TAPENAMES "'/dev/rmt' or '/dev/rtp'"
#endif

#ifdef i386
struct {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time,
		t_length,
		t_dens;
	char	t_fill[472];
} Tape_hdr;
#else
struct {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time,
		t_length,
		t_dens,
		t_reelblks,
		t_blksize,
		t_nblocks;
	char	t_fill[472];
} Tape_hdr;
#endif

int fsi, fso;
#define DATE_FMT	"%a %b %e %H:%M:%S %Y\n"
static char time_buf[80];


void
sigalrm()
{
void	(*signal())();

	signal(SIGALRM,sigalrm);
}

extern	char *optarg;
extern  int	optind;

main(argc, argv) 
char **argv; 
{
	long curtime;
	int i, c;
	int nflag = 0;

#ifdef i386
	char *dev = NULL;
	char *fsname = NULL;
	char *volume = NULL;
	char *magicsave = NULL;
#else
	char *dev;
	char *fsname;
	char *volume;
	char *magicsave;
#endif

	void	(*signal())();
#if u3b15 || u3b2
	int result;
#endif
#ifdef	RT
	static char usage = "s5 Usage:\nlabelit [-F s5] [generic_options] [-n] special [fsname volume]\n";
#else
	static char *usage = "s5 Usage:\nlabelit [-F s5] [generic_options] [-n] special [fsname volume]\n";
#endif

	(void)setlocale(LC_ALL, "");
	signal(SIGALRM, sigalrm);

#ifdef RT
	setio(-1,1);	/* use physical io */
#endif

	while ((c = getopt(argc, argv, "?n")) != EOF) {
		switch(c) {
			case 'n':
				nflag++;
				break;
			case '?':
				fprintf(stderr, "%s", usage);
				exit(32);
		}
	}
	if ((argc - optind) < 1) {
		fprintf(stderr, "%s", usage);
		exit(32);
	}
	dev = argv[optind++];
	/* for backwards compatability, -n may also appear as the last */ 
	/* in the command line, labelit device [fsname volname] -n */
	if (optind < argc) {
		if (strcmp(argv[optind],"-n") == 0) {
			nflag++;
			optind++;
		}
		else {
			fsname=argv[optind++];
		}
	}
	if (optind < argc) {
		if (strcmp(argv[optind],"-n") == 0) {
			nflag++;
		}
		else {
			volume=argv[optind];
		}
		optind++;
	}
	if (!nflag)
		if (optind < argc) {
			if (strcmp(argv[optind],"-n") == 0) {
				nflag++;
			}
		}
	if (nflag) {
		if(!IFTAPE(dev)) {
			fprintf(stderr, "labelit: `-n' option for tape only\n");
			fprintf(stderr, "\t'%s' is not a valid tape name\n", dev);
			fprintf(stderr, "\tValid tape names begin with %s\n", TAPENAMES);
			fprintf(stderr, usage);
			exit(33);
		}
		printf("Skipping label check!\n");
		goto do_it;
	}

	if((fsi = open(dev,0)) < 0) {
		fprintf(stderr, "labelit: cannot open device\n");
		exit(31+2);
	}

	if(IFTAPE(dev)) {
		alarm(5);
		read(fsi, &Tape_hdr, sizeof(Tape_hdr));
		magicsave = Tape_hdr.t_magic;
		alarm(0);
		if(!(equal(Tape_hdr.t_magic, "Volcopy", 7)||
		    equal(Tape_hdr.t_magic, "VOLCOPY", 7)||
		    equal(Tape_hdr.t_magic,"Finc",4))) {

#ifdef i386
			fprintf(stderr, "labelit: media not labelled!\n");
#else
			fprintf(stderr, "labelit: tape not labelled!\n");
#endif

			exit(31+2);
		}

#ifdef i386
		printf("%s floppy volume: %s, reel %d of %d reels\n",
#else
		printf("%s tape volume: %s, reel %d of %d reels\n",
#endif

			Tape_hdr.t_magic, Tape_hdr.t_volume, Tape_hdr.t_reel, Tape_hdr.t_reels);
		cftime(time_buf, DATE_FMT, &Tape_hdr.t_time);
		printf("Written: %s", time_buf);
		if((argc==2 && Tape_hdr.t_reel>1) || equal(Tape_hdr.t_magic,"Finc",4))
			exit(0);
	}
	if((i=read(fsi, &super, sizeof(super))) != sizeof(super))  {
		fprintf(stderr, "labelit: cannot read superblock\n");
		exit(31+2);
	}

#define	S	super.f.fs
	if ((S.s_magic != FsMAGIC)
	||  (S.s_nfree < 0)
	||  (S.s_nfree > NICFREE)
	||  (S.s_ninode < 0)
	||  (S.s_ninode > NICINOD)
	||  (S.s_type < 1)) {
		fprintf(stderr, "s5 labelit: %s is not an s5 file system\n",dev);
		exit(31+2);
	}

	switch(S.s_type)  {
	case Fs1b:
		printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 512b, ", 
			S.s_fname, S.s_fpack, S.s_fsize, (S.s_isize - 2) * 8);
		break;
#if u3b15 || u3b2
	case Fs2b:
		if(IFTAPE(dev))
			result = tps5bsz(dev, Tape_hdr.t_blksize);
		else
			result = s5bsize(fsi);
		if(result == Fs2b)
			printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 1Kb, ", 
			S.s_fname,S.s_fpack,S.s_fsize * 2,(S.s_isize - 2) * 16);
		else if(result == Fs4b)
			printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 2Kb, ", 
			S.s_fname,S.s_fpack,S.s_fsize * 4,(S.s_isize - 2) * 32);
		else {
			fprintf(stderr,"WARNING: can't determine logical block size of %s\n         root inode or root directory may be corrupted!\n", dev);
			printf("Current fsname: %.6s, Current volname: %.6s\n", S.s_fname, S.s_fpack);
		}
		break;
	case Fs4b:
		printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 2Kb, ", 
			S.s_fname,S.s_fpack,S.s_fsize * 4,(S.s_isize - 2) * 32);
		break;
#else
	case Fs2b:
		printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 1Kb, ", 
			S.s_fname,S.s_fpack,S.s_fsize * 2,(S.s_isize - 2) * 16);
		break;
#endif

#ifdef i386
	case Fs4b:
		printf("Current fsname: %.6s, Current volname: %.6s, Blocks: %ld, Inodes: %d\nFS Units: 2Kb, ", 
			S.s_fname,S.s_fpack,S.s_fsize * 4,(S.s_isize - 2) * 32);
		break;
#endif

	default:
		fprintf(stderr,"labelit: bad block type\n");
		printf("Current fsname: %.6s, Current volname: %.6s\n", S.s_fname, S.s_fpack);
	}
	cftime(time_buf, DATE_FMT, &S.s_time);
	printf("Date last modified: %s", time_buf);
	if(argc==2)
		exit(0);
do_it:
	if (nflag && argc < 5) {
		fprintf(stderr, "%s", usage);
		exit(34);
	} else if (!nflag && argc < 4) {
		fprintf(stderr, "%s", usage);
		exit(34);
	}
	printf("NEW fsname = %.6s, NEW volname = %.6s -- DEL if wrong!!\n",
		fsname, volume);
	sleep(10);
	sprintf(super.f.fs.s_fname, "%.6s", fsname);
	sprintf(super.f.fs.s_fpack, "%.6s", volume);

	close(fsi);
	fso = open(dev,1);
	if(IFTAPE(dev)) {
		strcpy(Tape_hdr.t_magic, "Volcopy");
		sprintf(Tape_hdr.t_volume, "%.6s", volume);
		if(write(fso, &Tape_hdr, sizeof(Tape_hdr)) < 0)
			goto cannot;
	}
	if(write(fso, &super, sizeof(super)) < 0) {
cannot:
		fprintf(stderr, "labelit cannot write label\n");
		exit(31+2);
	}
	exit(0);
}
equal(s1, s2, ct)
char *s1, *s2;
int ct;
{
	register i;

	for(i=0; i<ct; ++i) {
		if(*s1 == *s2) {;
			if(*s1 == '\0') return(1);
			s1++; s2++;
			continue;
		} else return(0);
	}
	return(1);
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
		if (lseek(fd, offset, 0) != offset)
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
/* heuristic function to determine logical block size of System V file system
 *  on tape
 */
tps5bsz(dev, blksize)
char	*dev;
long	blksize;
{

	int count;
	int skipblks;
	long offset;
	long address;
	char *buf;
	struct dinode *inodes;
	struct direct *dirs;
	char * p1;
	char * p2;
	

	buf = (char *)malloc(2 * blksize);

	close(fsi);

	/* look for i-list starting at offset 2048 (indicating 1KB block size) 
	 *   or 4096 (indicating 2KB block size)
	 */
	for(count = 1; count < 3; count++) {

		address = 2048 * count;
		skipblks = address / blksize;
		offset = address % blksize;
		if ((fsi = open(dev, 0)) == -1) {
			fprintf(stderr, "Can't open %s for input\n", dev);
			exit(31+1);
		}
		/* skip over tape header and any blocks before the potential */
		/*   start of i-list */
		read(fsi, buf, sizeof Tape_hdr);
		while (skipblks > 0) {
			read(fsi, buf, blksize);
			skipblks--;
		}

		if (read(fsi, buf, blksize) != blksize) {
			close(fsi);
			continue;
		}
		/* if first 2 inodes cross block boundary read next block also*/
		if ((offset + 2 * sizeof(struct dinode)) > blksize) {
		    if (read(fsi, &buf[blksize], blksize) != blksize) {
			close(fsi);
			continue;
		    }
		}
		close(fsi);
		inodes = (struct dinode *)&buf[offset];
		if((inodes[1].di_mode & S_IFMT) != S_IFDIR)
			continue;
		if(inodes[1].di_nlink < 2)
			continue;
		if((inodes[1].di_size % sizeof(struct direct)) != 0)
			continue;
	
		p1 = (char *) &address;
		p2 = inodes[1].di_addr;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1   = *p2;
	
		/* look for root directory at address specified by potential */
		/*   root inode */
		address = address << (count + 9);
		skipblks = address / blksize;
		offset = address % blksize;
		if ((fsi = open(dev, 0)) == -1) {
			fprintf(stderr, "Can't open %s for input\n", dev);
			exit(31+1);
		}
		/* skip over tape header and any blocks before the potential */
		/*   root directory */
		read(fsi, buf, sizeof Tape_hdr);
		while (skipblks > 0) {
			read(fsi, buf, blksize);
			skipblks--;
		}

		if (read(fsi, buf, blksize) != blksize) {
			close(fsi);
			continue;
		}
		/* if first 2 directory entries cross block boundary read next 
		 *   block also 
		 */
		if ((offset + 2 * sizeof(struct direct)) > blksize) {
		    if (read(fsi, &buf[blksize], blksize) != blksize) {
			close(fsi);
			continue;
		    }
		}
		close(fsi);

		dirs = (struct direct *)&buf[offset];
		if(dirs[0].d_ino != 2 || dirs[1].d_ino != 2 )
			continue;
		if(strcmp(dirs[0].d_name,".") || strcmp(dirs[1].d_name,".."))
			continue;

		if (count == 1) {
			free(buf);
			return(Fs2b);
		}
		else if (count == 2) {
			free(buf);
			return(Fs4b);
		}
	}
	free(buf);
	return(-1);
}
#endif
