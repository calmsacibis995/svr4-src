/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)s5.cmds:clri.c	1.22"
/*
 * clri filsys inumber ... (with support for 512, 1KB and 2KB block sizes)
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5ino.h>
#include <sys/fs/s5dir.h>
#include <sys/fs/s5filsys.h>

#define BSIZE  2048
/* FSBSIZE is maximum logical block size */
#if (vax || u3b || u3b15 || u3b2 || i386)
#if u3b2 || u3b15 || i386
#define FSBSIZE 2048
#else
#define FSBSIZE	1024
#endif
#else
#define	FSBSIZE BSIZE 
#endif

#define ISIZE	(sizeof(struct dinode))
#define	NBINODE	(FSBSIZE/ISIZE)

#ifndef i386
#define PHYSBLKSZ 512
#endif

struct filsys sblock;
int	bsize, nbinode;

struct	ino
{
	char	junk[ISIZE];
};
struct	ino	buf[NBINODE];
int	status;

main(argc, argv)
char *argv[];
{
	register i, f;
	unsigned n;
	int j, k;
	long off;
#if u3b15 || u3b2
	int result;
#endif

	if(argc < 3) {
		printf("s5 Usage:\nclri [-F s5] [generic_options] special inumber ...\n");
		exit(31+4);
	}
	f = open(argv[1], 2);
	if(f < 0) {
		printf("cannot open %s\n", argv[1]);
		exit(31+4);
	}
	if(lseek(f, (long)SUPERBOFF, 0) < 0
	   || read(f, &sblock, (sizeof sblock)) != (sizeof sblock)) {
		printf("cannot read super-block of %s\n", argv[1]);
		exit(31+4);
	}
	if(sblock.s_magic != FsMAGIC) {
		printf("s5 clri:  %s is not an s5 file system\n",argv[1]);
		exit(35);
	} 
	if(sblock.s_type == Fs1b) {
		bsize = 512;
#if u3b15 || u3b2
	} else if(sblock.s_type == Fs2b) {
		result = s5bsize(f);
		if(result == Fs2b)
			bsize = 1024;
		else if(result == Fs4b)
			bsize = 2048;
		else {
			printf("can't verify block size of %s\nroot inode or root directory may be corrupted!\n",argv[1]);
			exit(31+4);
		}
	} else if(sblock.s_type == Fs4b) {
		bsize = 2048;
#else
	} else if(sblock.s_type == Fs2b) {
		bsize = 1024;
#endif

#ifdef i386
	} else if(sblock.s_type == Fs4b) {
		bsize = 2048;
#endif

	} else {
		printf("can't determine logical  block  size of %s\n", argv[1]);
		exit(31+4);
	}
	nbinode = bsize / ISIZE;
	for(i=2; i<argc; i++) {
		if(!isnumber(argv[i])) {
			printf("%s: is not a number\n", argv[i]);
			status = 1;
			continue;
		}
		n = atoi(argv[i]);
		if(n == 0) {
			printf("%s: is zero\n", argv[i]);
			status = 1;
			continue;
		}
		off = ((n-1)/nbinode + 2) * (long)bsize;
		lseek(f, off, 0);
		if(read(f, (char *)buf, bsize) != bsize) {
			printf("%s: read error\n", argv[i]);
			status = 1;
		}
	}
	if(status)
		exit(31+status);
	for(i=2; i<argc; i++) {
		n = atoi(argv[i]);
		printf("clearing %u\n", n);
		off = ((n-1)/nbinode + 2) * (long)bsize;
		lseek(f, off, 0);
		read(f, (char *)buf, bsize);
		j = (n-1)%nbinode;
		for(k=0; k<ISIZE; k++)
			buf[j].junk[k] = 0;
		lseek(f, off, 0);
		write(f, (char *)buf, bsize);
	}
	exit(status);
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(c < '0' || c > '9')
			return(0);
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
#endif
