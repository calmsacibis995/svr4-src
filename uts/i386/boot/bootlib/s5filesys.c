/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)boot:boot/bootlib/s5filesys.c	1.1.3.3"


/* include files specific to boot */
#include "../sys/boot.h"
#include "../sys/inode.h"
#include "../sys/libfm.h"

/* generic head.sys include files */
#include "sys/dir.h"
#include "sys/ino.h"
#include "sys/alttbl.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5filsys.h"

/*
 * Stand-alone filesystem routines.
 */
extern ushort ourDS, mem_alias_sel;

/* boot_delta and Alt_tbl are set in the initialization routine */

extern off_t	boot_delta;		/* sector offset to filesystem */
#ifdef AT386
extern kbd_ck_val_t kbd_check;
#endif
extern bfstyp_t boot_fs_type;

#ifdef WINI
extern struct alt_info Alt_tbl;		/* alternate sector table */
#endif /* WINI */


/* global data buffer and cached block id */
char	gbuf[MAX_BSIZE];
daddr_t	gbuf_cache = -1; 

#ifndef AT386
extern	ulong	g_actual, g_status;
#endif

/* s5 block size related */
int	s5blksiz;		/* logical block size */
int	s5inopb;	/* inodes per logical block */
int	s5nindir;	

#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*s5inopb-1))/s5inopb)
#define	itoo(x)	(int)(((unsigned)(x)+(2*s5inopb-1))%s5inopb)

/* 
 * inode information:
 * space for direct-pointers & 1 indirect block 
 * plus MAXDINDR double indirect blocks. 
 */

#define	NADDR		13	/* number of pointers in a disk inode */
#define	MAXDINDR	20	/* max no of double indirect blocks supported */

daddr_t	iaddr[NADDR];
inode_t	in;

/* block for doubly indirect inodes */

long indnum;
long indaddr[MAXDINDR+1];
long indbuf[MAX_BSIZE/sizeof(daddr_t)];


long breadi();
char *getcomp();

/*
	Checks if root is s5 and initializes variables.
*/
int
s5_init(ptnstart)
off_t	ptnstart;
{
	register off_t	supblkst;
	static struct	 filsys	supblk;	
	register	ushort	ds;

	ulong	altmap();


	/* get start of s5 superblock on disk */
	supblkst = ptnstart + SUPERBOFF/dev_gran;

#ifdef AT386 
	/* read s5 superblock */	
	disk( supblkst, physaddr(&supblk), (short)1 );
#else
	disk_read(altmap(supblkst), 1L, &supblk, 
			ourDS, &g_actual, &g_status);
#endif

	/* check if root is s5 type */
	if ( supblk.s_magic != FsMAGIC ) {
		debug(printf("s5_init: not an s5 filesystem\n"));
		return -1;
	}

	switch ( supblk.s_type) {
	case Fs1b:
		s5blksiz=512;
		break;
	case Fs2b:
		s5blksiz=1024;
		break;
	case Fs4b:
		s5blksiz=2048;
		break;
	}

	s5inopb=s5blksiz/sizeof(struct dinode); 
	s5nindir=s5blksiz/sizeof(daddr_t);

	debug(printf("s5_init: s5 filesystem, %d bytes\n", s5blksiz));
	return 0;

}

/*
 * bnami(): 	Convert path name to inode #; inum is cwd.
 * 		Returns inode number (0 if not found).
 */
struct direct	comp, dir;

short
bnami(inum, path)
register short		inum;
register char	*path;
{
	register long		count;
	register long		offset;

	/*
	 * Loop, scanning path.
	 */
	for(;;) {

		while(*path == '/')			/* skip leading /'s */
			path++;

		/*
		 * If null path, found it!
		 */

		if (*path == '\0')
			return(inum);

		/*
		 * Get inode, find entry in directory.
		 * It must be a directory.
		 */
		if (biget(inum) <= 0) {
			return(0);		/* didn't find */
		}

		if ((in.i_ftype & IFMT) != IFDIR) {
			return(0);		/* not a directory */
		}

		 /* Loop thru directory, looking for name. */
		
		path = getcomp(path, comp.d_name);	/* get component */

		offset = 0;
		
		while(count = breadi(offset, (char *)&dir, ourDS, (long)sizeof(dir))) {

			if (dir.d_ino != 0)
				if ( strncmp( comp.d_name, dir.d_name, DIRSIZ) == 0 ) {
					inum = dir.d_ino;
					break;
				}

			offset += count;
		}

		if ( count == 0 )
			return(0);	/* ran out of directory */
	}
}


/*
 * biget():	input disk-version of inode; sets global inode struct 'in'.
 *		Returns 0 if bad inum, -1 if file too big, inum on success.
 */
 
short
biget(inum)
register short	inum;
{
	register char	*cp;		/* used to save disk addr into iaddr */
	register char	*blkp;		/* disk addresses in inode */
	register struct dinode	*dp;	/* to access fields in inode */
	register int	i;
	register int	j;

	if (inum <= 0)
		return(0);			/* Sorry! */


	dread(itod(inum));
	dp = (struct dinode *)gbuf;
	dp += itoo(inum);

	in.i_ftype = dp->di_mode;
	in.i_size = dp->di_size;
	in.i_mtime = dp->di_mtime;

	/*
	 * Get address pointers from inode.
	 * Our long words are held low[0],low[1],high[0],high[1]
	 * The 3 bytes in the dinode are low[0],low[1],high[0].
	 */
	cp = (char *)iaddr;
	blkp = (char *)dp->di_addr;
	for(i = 0; i < NADDR; i++) {
		*cp++ = *blkp++;
		*cp++ = *blkp++;
		*cp++ = *blkp++;
		*cp++ = 0;
	}

	/* 
	 * Get indirect blocks adresses - expects only MAXDINDR indirect blocks.
         * First indirect block is in inode, the other indirect blocks
	 * should be accessed via double indirect block.
	*/ 

	/* get first indirect block address */
	if ((indaddr[0] = iaddr[NADDR-3]) != 0) {
		debug(printf("file has indirect blocks\n"));
	} 

	/* get other indirect blks addrs(if any) thro' double indirect blk */	
	if (iaddr[NADDR-2] != 0)
	{
	 	debug(printf("file has double indirect blocks\n"));
		dread(iaddr[NADDR-2]);

		for (j = 0; ((long *)gbuf)[j] != 0  && j < MAXDINDR; j++)
		{
			indaddr[j+1] = ((long *)gbuf)[j];
			debug(printf("indaddr[ %d] = %ld\n", j+1,indaddr[j+1])); 
		}

		if (j == MAXDINDR) {
			printf("file too big\n");
			return (-1);
		}
	}

	/* mark indirect block cache as invalid, and return */
	indnum = -1;

	debug(printf("returning from  s5filesys:biget()\n"));
	return (inum);		
}


/*
 * breadi():	Read the current UNIX file pointed to by inode 'in', 
 *		transfer data to sel:addr, a physical address.
 *		Returns count of bytes read, 0 at EOF.
 */

long		
breadi(loc, addr, sel, count)
register long	loc;		/* offset in file */
register caddr_t	addr;	/* offset of destination */
register ulong sel;		/* selector of destination */
register long	count;		/* amount to fetch */
{
	register unsigned 	xc;		/* local transfer count */
	register long			tcount;	/* total transfer count */
	register unsigned		offset;	/* offset in gbuf */
	register int		i;

	/* 
	 * Restrict count, if it would go beyond EOF. 
	 * Return 0 already at EOF.
	 */

	if (loc + count > in.i_size)
		count = in.i_size - loc;

	if ( count < 0 )
		return(0);


	/* Transfer the stuff */

	tcount = 0;

	while(count != 0) {

		offset = loc % s5blksiz;
		xc = s5blksiz - offset;

		if (xc > count)
			xc = count;

		if ((i = loc / s5blksiz) < NADDR-3)
			dread(iaddr[i]);
		else {
			/* Need to get indirect block if we don't have it yet */

			if ((i -= NADDR-3) / s5nindir != indnum) {
				dread(indaddr[indnum = i / s5nindir]);
				iomove(gbuf, ourDS, (char *)indbuf, ourDS, s5blksiz);
			}
			dread(indbuf[i % s5nindir]);
		}
#ifdef AT386
		if ( kbd_check && ischar() )
			return( -1 );
#endif
		iomove(gbuf+offset, ourDS, addr, sel, xc);

		loc += xc;
		tcount += xc;
		count -= xc;
		addr += xc;
	}

	return (tcount);
}


#ifdef AT386
/*
 * dread(): 	Read the disk at bno and transfer s5blksiz bytes 
 *		into "gbuf" (static).
 * 		If block is already cached there, done.
 */
dread(bno)
register daddr_t	bno;
{
	register int	offset;
	register daddr_t		secno;
	register daddr_t		rsecno;
	register int		i, count, tcount, trkno;

	if (bno == gbuf_cache) 
		return;

	/* determine physical sector number on disk */

	switch (boot_fs_type)  {
	case s5: 
		count = s5blksiz / dev_gran;
		break;
	case BFS:
		count = 1;
		break;
	default:
		printf("dread:Unknown filesystem type\n");
		break;
	}

	secno = bno * count + boot_delta;

#ifdef WINI
	for (offset = 0; count-- > 0; offset += dev_gran, ++secno) {

		rsecno = secno;
		trkno = secno / spt;

		/* 
		 * test to see if calculated track is in a bad track and
	   	 * revector to the alternate one 
		 */

		for (i = 0; i < (int)Alt_tbl.alt_trk.alt_used; i++) {
			if (trkno == Alt_tbl.alt_trk.alt_bad[i]) {
				rsecno = Alt_tbl.alt_trk.alt_base
						+ rsecno + (i - trkno) * spt;
				debug(printf("\nTrack %d remapped to %ld\n",trkno,Alt_tbl.alt_trk.alt_base/spt+i));
				break;
			}
		}

		/* 
		 * test to see if calculated sector is a bad sector and
	   	 * revector to the alternate one 
		 */

		for (i = 0; i < (int)Alt_tbl.alt_sec.alt_used; i++) {
			if (rsecno == Alt_tbl.alt_sec.alt_bad[i]) {
				rsecno = Alt_tbl.alt_sec.alt_base + i;
				debug(printf("\nSector %ld remapped to %ld\n",secno,rsecno));
				break;
			}
		}

		disk( rsecno, physaddr(&gbuf[offset]), (short)1 );
		
	}

#else /* WINI */

	for (offset = 0;;) {
		tcount = spt - (secno % spt);
		if (tcount > count)
			tcount = count;
		disk(secno, physaddr(&gbuf[offset]), (short) tcount);
		if ((count -= tcount) == 0)
			break;
		secno += tcount;
		offset += tcount * dev_gran;
	}

#endif /* WINI */

	gbuf_cache = bno;
}
#endif

/*
 * getcomp():	Get next path component from a string.
 *	 	Return next position in string as value.
 */

char *
getcomp(path, comp)
register char *path;
register char *comp;
{
	register i;
	register char	c;

	for(i = 0; *path && *path != '/'; i++) {
		c = *path++;
		if (i < DIRSIZ)
			*comp++ = c;
	}
	while(i++ < DIRSIZ)
		*comp++ = 0;

	return(path);
}
