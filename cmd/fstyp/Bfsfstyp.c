/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)fstyp:Bfsfstyp.c	1.1.3.1"
#include <stdio.h>
#include <sys/types.h>
#include <sys/vnode.h>
#include "sys/fs/bfs.h"

char buf4ino[512];
int  bufblkno = -1;

main(argc, argv)
int argc;
char **argv;
{
	register int i;
	int fd;
	char vflag = 0;
	char *special;
	char sector[512];
	long *magic;
	long freefiles,freeblocks;
	struct bdsuper *super;
	struct bfs_dirent dir;

	if ((argc < 2) || (argc > 3))
	{
		fprintf(stderr, "Usage: %s [-v] special\n", argv[0]);
		exit(1);
	}

	for (i=1; i < argc; i++)
	{
		if ((strcmp(argv[i], "-v")) == 0)
			vflag++;
		else
			special = argv[i];
	}

	fd = open(special, 0);
	if (fd < 0)
	{
		fprintf(stderr, "%s: Cannot open %s.\n", argv[0], special);
		exit(1);
	}


	read(fd, sector, 512);

	super = (struct bdsuper *)(sector + BFS_SUPEROFF);

	if (super->bdsup_bfsmagic != BFS_MAGIC)
		exit(1);

	if (!vflag)
	{
		printf("bfs\n");
		exit(0);
	}


	freefiles = (super->bdsup_start - BFS_DIRSTART) / sizeof(struct bfs_dirent);
	freeblocks = (super->bdsup_end + 1  - super->bdsup_start) / BFS_BSIZE;
	printf("Total number of files: %d, Total number of blocks: %d\n",
		freefiles, (super->bdsup_end +1) / BFS_BSIZE);

	freefiles = 0;
	for (i=BFS_DIRSTART; i < super->bdsup_start; i+=sizeof(struct bfs_dirent))
	{
		get_ino(fd, i, &dir, sizeof(struct bfs_dirent));
		if (dir.d_ino == 0)
			freefiles++;
		else if (dir.d_eblock != 0)
			freeblocks -= (dir.d_eblock - dir.d_sblock) + 1;
	}

	printf("Files free: %d, Blocks free: %d\n", freefiles, freeblocks);
	close(fd);

	exit(0);
}

int
seek_read(fd, offset, buf, len)
	int fd;
	off_t offset;
	char *buf;
	long len;
{
	long lseek();

	lseek(fd, offset, 0);
	read(fd, buf, len);
	return 0;
}


int
get_ino(fd, ioffset, ibuf, len)
int  fd;
int ioffset;
char ibuf[];
int  len;
{
	int i = 0;
	int j;
	long iblk;

	iblk = ioffset / 512;
	if (bufblkno != iblk) {
		seek_read(fd, (iblk * 512), buf4ino, 512);
		bufblkno = iblk;
	}

	j = ioffset % 512;
	while (i < len) {
		if (j < 512)
			ibuf[i++] = buf4ino[j++];
		else {
			read(fd, buf4ino, 512);
			j = 0;
			bufblkno++;
		}
	}
	return(0);
}
