/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)fstyp:S5fstyp.c	1.3.3.1"

/* s5: fstyp */
#include	<sys/param.h>
#include	<time.h>
#include	<sys/types.h>
#include	<sys/fs/s5param.h>
#include	<sys/fs/s5ino.h>
#include 	<sys/vnode.h>
#include	<sys/fs/s5dir.h>
#include	<stdio.h>
#include	<setjmp.h>
#include	<sys/fs/s5filsys.h>
#include	<sys/stat.h>
#include	<sys/fcntl.h>

#ifndef i386
#define 	PHYSBLKSZ	512
#endif

void	exit();
long	lseek();

main(argc,argv)
int	argc;
char	*argv[];
{

	int	fd;
	int 	vflag = 0;
	int 	i;
	int	result;
	long	bsize = 0;
	char	*dev;
	struct	stat	buf;
	struct	filsys	sblock;

	if (argc == 3) {
		dev = argv[2];
		vflag = 1;
	} 
	else
		dev = argv[1];
		
	/*
	 *	Read the super block associated with the device. 
	 */

	if ((fd = open(dev, O_RDONLY)) < 0) {
		fprintf(stderr,"s5 fstyp: cannot open <%s>\n",dev);
		exit(1);
	}
	
	if (lseek(fd, (long)SUPERBOFF, 0) < 0
		|| read(fd, &sblock, (sizeof sblock)) != (sizeof sblock)) {
		fprintf(stderr,"s5 fstyp: cannot read superblock\n");
		close(fd);
		exit(1);
	}
	
	/*
	 *	Determine block size for sanity check and if it is type s5
 	 */
	
	if(sblock.s_magic == FsMAGIC) {
		if(sblock.s_type == Fs1b) {
			bsize = 512;
#if u3b15 || u3b2 || i386
		} else if(sblock.s_type == Fs2b) {

#ifdef i386
			bsize = 1024; 
#else

			result = s5bsize(fd);
			if(result == Fs2b)
				bsize = 1024;
			else if(result == Fs4b)
				bsize = 2048;
			else {
				exit(1);
			}
#endif

		} else if(sblock.s_type == Fs4b) {
			bsize = 2048;
#else
		} else if(sblock.s_type == Fs2b) {
			bsize = 1024;
#endif
		} else 
			exit(1);
	fprintf(stdout,"s5\n");
	} 
	else {
		exit(1);
	}
	if (vflag) {
		printf("file system name (s_fname): %s\n", sblock.s_fname);
		printf("file system packname (s_fpack): %s\n", sblock.s_fpack);
		printf("magic number (s_magic): 0x%x\n", sblock.s_magic);
		printf("type of new file system (s_type): %ld\n", sblock.s_type);
		printf("file system state (s_state): 0x%x\n", sblock.s_state);
		printf("size of i-list (in blocks) (s_isize): %d\n", sblock.s_isize);
		printf("size of volume (in blocks) (s_fsize): %ld\n", sblock.s_fsize);
		printf("lock during freelist manipulation (s_flock): %d\n", sblock.s_flock);
		printf("lock during i-list manipulation (s_ilock): %d\n", sblock.s_ilock);
		printf("super block modified flag (s_fmod): %d\n", sblock.s_fmod);
		printf("super block read-only flag (s_ronly): %d\n", sblock.s_ronly);
		printf("last super block update (s_time): %d\n", sblock.s_time);
		printf("total free blocks (s_tfree): %ld\n", sblock.s_tfree);
		printf("total free inodes (s_tinode): %d\n", sblock.s_tinode);
		printf("gap in physical blocks (s_dinfo[0]): %d\n", sblock.s_dinfo[0]);
		printf("cylinder size in physical blocks (s_dinfo[1]): %d\n", sblock.s_dinfo[1]);
		printf("(s_dinfo[2]): %d\n", sblock.s_dinfo[2]);
		printf("(s_dinfo[3]): %d\n", sblock.s_dinfo[3]);
		printf("number of addresses in s_free (s_nfree): %d\n", sblock.s_nfree);
		printf("freelist (s_free[NICFREE)): \n");
		for (i=1; i< NICFREE; i++) {
			printf("%ld ", sblock.s_free[i]);
			if ((i%8) == 0)
				printf("\n");
		}
		printf("\nnumber of inodes in s_inode (s_ninode): %d\n", sblock.s_ninode);
	  	printf("free i-node list (s_inode[NICINOD]):\n");
	
		for (i=1; i< NICINOD; i++) {
			printf("%ld ", sblock.s_inode[i]);
			if ( (i%8) == 0)
				printf("\n");
		}
		printf("\n\nfill (s_fill[12]):\n");
		for (i=0; i< 12; i++) {
			printf("%ld ", sblock.s_fill[i]);
			if ((i != 0) && ((i%6) == 0))
				printf("\n");
		}
		printf("\n");
		
	}
	close(fd);

	exit(0);
}


#ifndef i386
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
