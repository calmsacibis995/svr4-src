/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bfs.cmds:bfs.cmds/fsck.c	1.9.3.1"
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/fs/bfs.h>
#include <sys/fs/bfs_compact.h>
#include <sys/stat.h>

char *Usage = "bfs usage:\nfsck [-F bfs] [generic options] [-y | -n] special\n";

struct sanityw {
	daddr_t fromblock;
	daddr_t toblock;
	daddr_t bfromblock;
	daddr_t	btoblock;
};

#define	BFSBUFSIZE 8192
char bfs_buffer[BFSBUFSIZE];
char superblk[512];
int superblkno = -1;
char buf4ino[512];
int bufblkno=-1;

main(argc, argv)
char **argv;
int argc;
{
	int fd;
	struct bsuper bs;
	struct bdsuper bd;
	char *special = NULL;
	char mflag = 0;
	char yflag = 0;
	char nflag = 0;
	char mod = 0;
	int r;
	struct stat statarea;
	struct {
		daddr_t tfree;
		ino_t tinode;
		char fname[6];
		char fpack[6];
	} ustatarea;

	extern char *optarg;
	extern int optind, opterr;
	int cc;
	int errflg=0;

	while ((cc = getopt(argc, argv, "mny")) != -1)
		switch (cc) {
		case 'm':
			if (nflag || yflag)
				errflg++;
			else
				mflag++;
			break;
		case 'n':
			if (mflag || yflag)
				errflg++;
			else
				nflag++;
			break;
		case 'y':
			if (mflag || nflag)
				errflg++;
			else
				yflag++;
			break;
		case '?':
			errflg++;
			break;
		}

	if (errflg || (argc -optind) < 1) {
		fprintf(stderr, Usage);
		exit (31+8);
	}
	special= argv[optind];
	if (special == NULL) {
		fprintf(stderr, Usage);
		exit(31+8);
	}
	if(stat(special, &statarea) < 0) {
		fprintf(stderr, "bfs %s: %s: can not stat\n", argv[0], special);
		exit(31+8);
	}
	if((statarea.st_mode & S_IFMT) == S_IFBLK) {
		if(ustat(statarea.st_rdev,&ustatarea) >= 0) {
			if (!nflag) {
			    fprintf(stderr, "bfs %s: %s: mounted file system\n",
					 argv[0], special);
				exit(31+2);
			}
		}
	}
	else if((statarea.st_mode & S_IFMT) == S_IFCHR) {
		if(ustat(statarea.st_rdev,&ustatarea) >= 0) {
			fprintf(stderr, "bfs %s: %s file system is mounted as \
a block device, ignored\n", argv[0], special);
			exit(31+3);
		}
	}
	else 
	{
	      fprintf(stderr,"bfs %s: %s is not a block or character device\n",
			argv[0], special);
		exit(31+8);
	}

	fd = open(special, O_RDWR, O_SYNC);
	if (fd < 0)
	{
		fprintf(stderr, "bfs %s: can not open special file %s\n",
			argv[0], special);
		perror("fsck");
		exit(31+8);
	}

	seek_read(fd, BFS_SUPEROFF, &bd, sizeof(struct bdsuper));

	if (bd.bdsup_bfsmagic != BFS_MAGIC) {
		fprintf(stderr, "bfs %s: %s is not a bfs file system\n",
			argv[0], special);
		exit(31+8);
	}
	bs.bsup_start = bd.bdsup_start;
	bs.bsup_end = bd.bdsup_end;
	bs.bsup_devnode = (struct vnode *)fd;

	if (!mflag)
		printf("Checking %s:\n", special);

	r = check_compaction(&bs, &bd, mflag, nflag, yflag, &mod);
	if (mflag)
	{
		if (r)
		{
			printf("bfs %s: %s not okay\n", argv[0], special);
			exit(r);
		}

		printf("bfs %s: %s okay\n", argv[0], special);
		exit(0);
	}

	check_dirents(&bs);
	if (mod)
		printf("File system was modified.\n");
	else
		printf("File system was not modified.\n");

	exit(0);
}

check_compaction(bs, bd, mflag, nflag, yflag, mod)
struct bsuper *bs;
struct bdsuper *bd;
char mflag,nflag,yflag,*mod;
{
	struct bfs_dirent dir;
	char ans[10];
	register int i;
	long fromblock,toblock;
	struct sanityw sw;
	

	if ((bd->bdcpb_fromblock == -1) || (bd->bdcpb_toblock == -1))
		return(0);

	if (mflag)
		return(31+1);

	printf("File system was in the middle of compaction.\n");

	if (nflag)
		return (0);

	if (!yflag)
	{
		printf("Complete compaction? ");
		fflush(stdout);
		scanf("%s", ans);
		if ((ans[0] != 'y') && (ans[0] != 'Y'))
			return(0);
	}
	if ((bd->bdcp_fromblock == -1) || (bd->bdcp_toblock == -1))
	{
		fromblock = bd->bdcpb_fromblock;
		toblock = bd->bdcpb_toblock;
		sw.fromblock = fromblock;
		sw.bfromblock = fromblock;
		sw.toblock = toblock;
		sw.btoblock = toblock;
		wr_sanityw(bs->bsup_devnode, BFS_SANITYWSTART,
		   &sw, sizeof(struct sanityw));
	}
	else
	{
		fromblock = bd->bdcp_fromblock;
		toblock = bd->bdcp_toblock;
	}


	for (i=BFS_DIRSTART; i < bs->bsup_start; i+= sizeof(struct bfs_dirent))
	{
		get_ino(bs->bsup_devnode, i, &dir, sizeof(struct bfs_dirent));
		if ((dir.d_sblock <= fromblock) && (dir.d_eblock > fromblock))
			break;
	}

	if ((dir.d_sblock > fromblock) || (dir.d_eblock <= fromblock))
	{
		/*
		 * Data blocks of file were already shifted and the inode
		 * of the file was updated. However, the system must have
		 * have crashed just before the sanity words were updated.
		 * Therefore will update them now.
		 */
		sw.fromblock = -1;
		sw.bfromblock = -1;
		sw.toblock = -1;
		sw.btoblock = -1;
		wr_sanityw(bs->bsup_devnode, BFS_SANITYWSTART,
		   &sw, sizeof(struct sanityw));
		(*mod)++;
		return (0);
	}

	printf("Finishing compaction of file (inode %ld)\n", dir.d_ino);

	bfs_shiftfile(bs, &dir, fromblock, i, toblock);

	(*mod)++;
	return (0);
}

check_dirents(bs)
struct bsuper *bs;
{
	register int i;
	struct bfs_dirent dir;
	int fd;
	long freeblocks, freefiles;
	long totblocks, totfiles;

	fd = (int)bs->bsup_devnode;

	freeblocks = (bs->bsup_end + 1 - bs->bsup_start) / BFS_BSIZE;
	totblocks =  (bs->bsup_end +1) / BFS_BSIZE;
	totfiles = (bs->bsup_start - BFS_DIRSTART) / sizeof(struct bfs_dirent);
	freefiles = totfiles;

	for (i=BFS_DIRSTART; i < bs->bsup_start; i+=sizeof(struct bfs_dirent))
	{
		get_ino(fd, i, &dir, sizeof(struct bfs_dirent));
		if (dir.d_ino == 0)
			continue;
		freefiles--;
		if (dir.d_eblock != 0)
			freeblocks -= (dir.d_eblock + 1 - dir.d_sblock);
	}
	printf("%ld total blocks\n%ld free blocks\n%ld total inodes\n",
		totblocks, freeblocks, totfiles);
	printf("%ld free inodes\n", freefiles);
}

min(a,b)
int a,b;
{
	if (a > b)
		return (b);
	else
		return (a);
}


/*
 * Shift the file described by dirent "dir",begining from "fromblock"
 * to "toblock".  "Offset" describes the location on the disk of the dirent.
 */
bfs_shiftfile(bs, dir, fromblock, offset, toblock)
	struct bsuper *bs;
	struct bfs_dirent *dir;
	daddr_t fromblock;
	off_t offset;
	daddr_t toblock;
{
	long gapsize;
	long maxshift;
	long filesize;
	off_t eof;
	long w4fsck[2];
	struct sanityw sw;

	gapsize = fromblock - toblock;
	maxshift = min(BFSBUFSIZE, gapsize*512);

	/*
	 * Write sanity words for fsck to denote compaction is in progress.
	 */
	sw.fromblock = fromblock;
	sw.toblock = toblock;
	sw.bfromblock = sw.fromblock;
	sw.btoblock = sw.toblock;
	wr_sanityw(bs->bsup_devnode,BFS_SANITYWSTART,
		      &sw, sizeof(struct sanityw));

	/*
	 * Calculate the new EOF.
	 */
	if (dir->d_eoffset / 512 == dir->d_eblock &&
	    dir->d_eblock >= sw.fromblock) {
		eof = (dir->d_eoffset - (dir->d_sblock * 512)) +
		       ((dir->d_sblock - gapsize) * 512);
		dir->d_eoffset = eof;
	}

	w4fsck[0] = -1;
	w4fsck[1] = -1;
	filesize = (dir->d_eblock - dir->d_sblock +1) * 512;

	/*
	 * Write as many sectors of the file at a time.
	 */
	while (sw.fromblock != (dir->d_eblock + 1)) {
		/*
		 * Must recalculate "maxshift" each time.
		 */
		maxshift = min(maxshift, (dir->d_eblock-sw.fromblock+1)*512);

		/*
		 * If gapsize is less than file size, must write words for fsck
		 * to denote that compaction is in progress (i.e which blocks
		 * are being shifted.)
		 * Otherwise, there is no need to write sanity words. If the
		 * system crashes during compaction, fsck can take it from 
		 * the top without data lost. 
		 */
		if (gapsize*512 < filesize) {
			sw.bfromblock = sw.fromblock;
			sw.btoblock = sw.toblock;
			wr_sanityw(bs->bsup_devnode,BFS_SANITYWSTART,
			  &sw, sizeof(struct sanityw));
		}

		seek_read(bs->bsup_devnode, sw.fromblock*BFS_BSIZE,
		  bfs_buffer, maxshift); 

		seek_write(bs->bsup_devnode, sw.toblock*BFS_BSIZE,
		  bfs_buffer, maxshift);

		sw.fromblock+= (maxshift / BFS_BSIZE);
		sw.toblock+= (maxshift / BFS_BSIZE);

		/*
		 * If gapsize is less than file size, must write a "-1" to
		 * the first 2 sanity words to let fsck know where compaction
		 * is.
		 */
		if (gapsize*512 < filesize)
			wr_sanityw(bs->bsup_devnode,BFS_SANITYWSTART,
			  w4fsck, sizeof(w4fsck));
	}

	/*
	 * Calculate the new values for inode and write it to disk.
	 */
	dir->d_sblock -= gapsize;
	dir->d_eblock -= gapsize;
	put_ino(bs->bsup_devnode, offset, dir, sizeof(struct bfs_dirent));

	/*
	 * Must write "-1" to all 4 sanity words for fsck to denote that
	 * compaction is not in progress.
	 */
	sw.fromblock = -1;
	sw.toblock = -1;
	sw.bfromblock = -1;
	sw.btoblock = -1;
	wr_sanityw(bs->bsup_devnode,BFS_SANITYWSTART,
	  &sw, sizeof(struct sanityw));
	return 0;
}


int
seek_read(fd, offset, buf, len)
	int fd;
	off_t offset;
	char *buf;
	long len;
{
	lseek(fd, offset, 0);
	read(fd, buf, len);
	return 0;
}

int
seek_write(fd, offset, buf, len)
	int fd;
	off_t offset;
	char *buf;
	long len;
{
	lseek(fd, offset, 0);
	write(fd, buf, len);
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

int
wr_sanityw(fd, swoffset, swbuf, len)
int  fd;
int swoffset;
char swbuf[];
int  len;
{
	int i = 0;
	int j;

	if (superblkno == -1) {
		seek_read(fd, BFS_SUPEROFF, superblk, 512);
		superblkno = 0;
	}
	j = swoffset;
	while (i < len)
		superblk[j++] = swbuf[i++];
	seek_write(fd, BFS_SUPEROFF, superblk, 512);
}

int
put_ino(fd, ioffset, ibuf, len)
int  fd;
int ioffset;
char ibuf[];
int  len;
{
	int i = 0;
	int j;
	long iblk;
	char onekbuf[1024];

	iblk = ioffset / 512;
	seek_read(fd, (iblk * 512), onekbuf, 1024);

	j = ioffset % 512;
	while (i < len)
		onekbuf[j++] = ibuf[i++];
	seek_write(fd, (iblk * 512), onekbuf, 1024);

	return(0);
}
