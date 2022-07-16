/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/bootlib/bfsfilesys.c	1.1.2.1"

#include	"sys/types.h"
#include	"sys/elog.h"
#include	"sys/iobuf.h"
#include 	"sys/immu.h"
#include	"sys/vtoc.h"
#include	"sys/vnode.h"
#include	"sys/vfs.h"
#include	"sys/fs/bfs.h"
#include 	"../sys/libfm.h"
#include 	"sys/sysmacros.h"
#include	"../sys/boot.h"


extern off_t fd;
#ifdef AT386
extern struct bootattr *battrp;
extern kbd_ck_val_t kbd_check;
#endif
extern ushort ourDS;
extern char gbuf[];

char data[BFS_BSIZE * 2];

#define	BYTE_TO_BLK(x)	((x) / BFS_BSIZE)
#define	EXC_BLK(x)	((x) & (BFS_BSIZE - 1))


int
bfsread(foffset,buffer,selector,nbytes)
register off_t foffset;
register char *buffer;
register ushort selector;
register int nbytes;
{	register int i;
	register long size,max_bytes,sbyte,count, rcount;
	register off_t offset;
	register char *mem,*bp;

#ifdef AT386
	debug(printf("in bfsread offset %x nbytes %x size %x\n",foffset,nbytes,battrp->size));

	/* if (( foffset + nbytes) > battr.size)
		nbytes = battr.size - foffset;
	
	if (nbytes < 0)
		return(0);
	*/
#endif
	mem= (char *)buffer;
	size = rcount = nbytes;
	offset = fd + foffset;

	/*
	 * for bfs a fd is the offset of the file
 	 */

	while (size){
		/* If this read doesn't start on a sector boundary we must

		 * read in the entire sector and copy part of it to memory. 
		 */

		if ((offset % BFS_BSIZE) != 0){
			nbytes = BFS_BSIZE - (offset % BFS_BSIZE);
			max_bytes = MIN(nbytes, size);
			nbytes = max_bytes;
			sbyte = offset % BFS_BSIZE;
			bp = data + sbyte;
			if (read_block(offset/BFS_BSIZE, data, ourDS) == -1)
				return(-1);
			iomove(bp, ourDS, mem, selector, nbytes);
			mem += nbytes;
			/* for (i=nbytes; i--; *mem++ = *bp++); */
			size -= nbytes;
			offset += nbytes;

			continue;
		}

		max_bytes = size;

		/* 
		/* 
		 * Misaligned read.  Again we must read the entire sector
		 * and copy part of it. 
		 */

		if (max_bytes < BFS_BSIZE){
			if (read_block(offset/BFS_BSIZE, data, ourDS) == -1)
				return(-1);
			bp = data;
			iomove(bp, ourDS, mem, selector, max_bytes);
			mem += max_bytes;
			/* for (i=max_bytes; i-- ; *mem++ = *bp++); */
			size -= max_bytes;
			offset += max_bytes;
			continue;
		}


		/* Now attempt to read multiple sectors at once */

		if ((count = big_read(offset/BFS_BSIZE, mem, 
				selector, max_bytes/BFS_BSIZE)) == -1)
			return(-1);
		size -= count;
		offset += count;
		mem += count;
	}

	return(rcount);
}


/*
 * open a file and return a fd. In BFS a fd is the byte offset of the file
 * on the disk.
 */

off_t
bfsopen(fname)
char *fname;
{
	struct bdsuper bd;
	struct bfs_dirent *dir;
	register struct bfs_ldirs *ld;
	off_t byte_off;
	off_t sect_off;
	off_t buff_off;
	off_t offset;
	off_t endofdir;
	off_t dirlastblk;
	register int i;
	register int j;
	register int k = 0;
	register found = 0;
	char	*fcomp;

	debug(printf("fname %s ",fname));

	/* Take last component of the fname */
	for (fcomp = fname; *fname != '\0';) {
		if (*fname++ == '/')
			fcomp = fname;
	}
		
	debug(printf("comp %s\n",fcomp));

	/*
	 * find out offset of the ROOT inode
	 */

	byte_off = BFS_INO2OFF(BFSROOTINO);

	/*
	 * find the block offset. Must round down.
	 */
	sect_off = BYTE_TO_BLK(byte_off);

	read_block(sect_off, data, ourDS);

	/*
	 * Find out the offset into the buffer just read.
	 */
	buff_off = EXC_BLK(byte_off);

	/*
	 * If all the ROOT inode is not in the first block
	 * we read, then read one more block.
	 */
	if ((BFS_BSIZE - buff_off) < sizeof(struct bfs_dirent))
		read_block(sect_off +1, data + BFS_BSIZE, ourDS);

	dir = (struct bfs_dirent *) (data + buff_off);

	endofdir = dir->d_eoffset;
	dirlastblk = endofdir / BFS_BSIZE;
	offset = dir->d_sblock;
	byte_off = offset * BFS_BSIZE;

	/*
	 * Search for the file name in the directory. If found, determine
	 * the offset on disk of where the inode of file is stored.
	 * Upon success, return the offset of the file in bytes.
	 * If failure, return -1
	 */
	for (i = offset; i < dirlastblk + 1 && found == 0; i += 2) {
		read_block(i, data, ourDS);
		read_block(i + 1, data + BFS_BSIZE, ourDS);

		for (j=0; j < sizeof(data) && byte_off < endofdir;
		     j += sizeof(struct bfs_ldirs),
		     byte_off += sizeof(struct bfs_ldirs) )  {

			ld = (struct bfs_ldirs *) (data + j);
			if (ld->l_ino == 0)
				continue;
			if (strncmp(ld->l_name, fcomp, BFS_MAXFNLEN) == 0) {
				found ++;
				break;
			}
		}
	}
	if (!found)
		return (-1);

	/*
	 * The file was found in the directory.
	 * Find out offset of the file inode
	 */
	byte_off = BFS_INO2OFF(ld->l_ino);

	/*
	 * find the block offset. Must round down.
	 */
	sect_off = BYTE_TO_BLK(byte_off);

	read_block(sect_off, data, ourDS);

	/*
	 * Find out the offset into the buffer just read.
	 */
	buff_off = EXC_BLK(byte_off);

	/*
	 * If all the file inode is not in the first block
	 * we read, then read one more block.
	 */
	if ((BFS_BSIZE - buff_off) < sizeof(struct bfs_dirent))
		read_block(sect_off +1, data + BFS_BSIZE, ourDS);

	dir = (struct bfs_dirent *) (data + buff_off);
#ifdef AT386
	battrp->size = dir->d_eoffset - dir->d_sblock +1;
	battrp->mtime = dir->d_fattr.va_mtime;
	battrp->ctime = dir->d_fattr.va_ctime;
	battrp->blksize = BFS_BSIZE;
#endif
	return((dir->d_sblock) * BFS_BSIZE);
}

#if 0
/*
 *	Function to list all of the files in /stand.  Reads two sectors at a time.
*/
list_dir(name)
register char *name;
{
	register char twosect[SBUFSIZE*2];
	register off_t offset;
	register struct bdsuper *super;
	register long start,end;
	register int i;
	register struct bfs_dirent *dir;
	register long old;
	register off_t dirsect;
	register int j;
	register int k = 0;

	if (read_block((off_t)0, twosect, ourDS) == -1)
		return(-1);
	super = (struct bdsuper *)twosect + BFS_SUPEROFF;
	start = super->bdsup_start;
	end = super->bdsup_end;
	dirsect = (BFS_DIRSTART / BFS_BSIZE);
	old = BFS_DIRSTART % BFS_BSIZE;

	printf("\nThe files in /stand are:\n");

	for (i=dirsect; i < (start / BFS_BSIZE); i++){
		if (read_block(i, twosect, ourDS) == -1)
			return(-1);
		if (read_block(i+1, twosect + BFS_BSIZE, ourDS) == -1)
			return(-1);
		for (j=old; j < ((BFS_BSIZE*2) - sizeof(struct bfs_dirent)); 
					j+=sizeof(struct bfs_dirent)){

			/* Read each dirent into "dir" */

			dir = (struct bfs_dirent *)(twosect+j);

			if (dir->d_fname[0] == '\0')	/* Empty file */
				continue;

			/* Print the filenames, six per line. */
			printf("%s     ", dir->d_fname);
			if (++k == 6){
				k = 0;
				printf("\n");
			}
		}

		old = j - BFS_BSIZE;
	}

	printf("\n");
	return (0);

}
#endif

/* Function to read only one sector from a device */

read_block(sector, mem, selector)
register long sector;
register char *mem;
register ushort selector;
{
	dread(sector);
#ifdef AT386
	if ( kbd_check && ischar() )
		return( -1 );
#endif
	iomove(gbuf, ourDS, mem, selector, BFS_BSIZE);
	return(0);
}

/*
 *	The speed of this boot program comes from this function.  It attempts to
 *	read multiple sectors from the boot device. 
*/
int
big_read(sector, mem, selector, nsectors)
register long sector;
register char *mem;
register ushort selector;
register long nsectors;
{
	int tcount = 0;


	/* Transfer the stuff */

	while(nsectors-- != 0) {
		
		dread(sector);
#ifdef AT386
		if ( kbd_check && ischar() )
			return( -1 );
#endif
		iomove(gbuf, ourDS, mem, selector, BFS_BSIZE);

		tcount += BFS_BSIZE;
		mem += BFS_BSIZE;
		sector++;
	}

	return (tcount);
}

#ifdef AT386
iomove(svaddr, selector1, dvaddr, selector2, count)
register char *svaddr;
register ushort  selector1;
register char  *dvaddr;
register ushort  selector2;
register int	count;
{
	memcpy(physaddr(dvaddr), physaddr(svaddr), count);
}
#endif
