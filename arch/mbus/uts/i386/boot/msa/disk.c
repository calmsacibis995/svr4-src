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

#ident	"@(#)mbus:uts/i386/boot/msa/disk.c	1.3.1.3"

#include "../sys/boot.h"
#include "../sys/libfm.h"
#include "../sys/error.h"
#include "../sys/s2main.h"
#include "sys/vtoc.h"
#include "sys/alttbl.h"
#include "sys/fdisk.h"
#include "sys/ivlab.h"
#include "sys/vnode.h"
#include "sys/fs/bfs.h"

extern ushort ourDS;

/*
 * the following must be set up for the use of the high level filesystem 
 * routines
 */
struct 		alt_info Alt_tbl;	/* alternate sector table */
bfstyp_t	boot_fs_type=UNKNOWN;	/* boot file system,BFS,s5 or UNKNOWN */
bfstyp_t	root_fs_type=UNKNOWN;	/* root file system type */
off_t 		root_delta;		/* root starting sector on disk */
extern 		scsidiskflag;		/* is the hard disk a scsi disk? */

char	buf[SBUFSIZE];	/* get_fs() only, can't be register */

off_t 	boot_delta;	/* starting sec of boot file system (blfile.c)*/

#define	RD_BUFSIZ	1024

#ifdef DEBUG
extern int _disk_read_ptr[2];
#endif
/*
 *	Read a logical block from the disk without alternating or cacheing.
*/
ulong g_actual, g_status;
read_oneblk(bno, bufp, ds)
register daddr_t bno;
register char *bufp;
register ushort ds;
{	register	int	offset;
	register ulong	secno;
		
	secno = bno * (RD_BUFSIZ/dev_gran);
	for(offset = 0; offset < RD_BUFSIZ; offset += dev_gran) {
		debug(printf("read_oneblk: sector %d\n", secno));
		disk_read(secno++, ONE_BLOCK, bufp+offset,
				ds, &g_actual, &g_status);
	}
}

/*
 * get_fs: 	initialize the driver; open the disk, find
 *		the boot and root slice and fill in the global 
 *		variables and tables.
 *	returns the begining offset of ROOT for BL_init.
*/
off_t
get_fs()
{
	register struct btblk	*btp;
	register struct pdinfo	*pip;
	register struct vtoc	*vtp;
	register daddr_t	secno, blkno;
	register int		i;
	register ushort		ptag;
	register char *bufp;

	/* Read in the volume label to get an idea where the FS might be */
	read_oneblk(BTBLK_LOC*dev_gran/RD_BUFSIZ, buf, ourDS);
	btp = (struct btblk *)buf;
	if (btp->bolt.magic_word_1 != MAGIC_WORD_1)
		error(STAGE2, E_FILE_SYSTEM, "Can't find volume label");

	root_delta = btp->ivlab.v_fsdelta;

	/* read in pdinfo to find where the bad block tables are */
	secno = unix_start + VTOC_SEC;
	debug(printf("\nReading disk info (pdinfo) from sector %ld\n", secno));
	blkno = (secno * dev_gran) / RD_BUFSIZ;
	read_oneblk(blkno, buf, ourDS);
	pip = (struct pdinfo *)buf;

	/*
	 * read in alternate sector table
	 *	(Assumes alt tbl starts on sector boundary.)
	*/
	if (pip->sanity == VALID_PD) {
		secno = unix_start + pip->alt_ptr/dev_gran;
		blkno = (secno * dev_gran) / RD_BUFSIZ;
		i = (ulong)(pip->alt_len + (RD_BUFSIZ - 1))/RD_BUFSIZ;
	} else {
		secno = blkno = i = 0;
	}
	debug(printf("alts at sector(s) %ld to %ld\n", blkno, blkno + i - 1));
	for (bufp = (char *)&Alt_tbl; i-- > 0; bufp += RD_BUFSIZ)
		read_oneblk(blkno++, bufp, ourDS);
	if ((Alt_tbl.alt_sanity != ALT_SANITY)
	||  (Alt_tbl.alt_version != ALT_VERSION)) {
		Alt_tbl.alt_sec.alt_used = Alt_tbl.alt_trk.alt_used = 0;
		debug(printf("Invalid alt table\n"));
	}

	/*
	 * look in vtoc to find start of stand and root partition
	 *	(Assumes vtoc is in same sector as pdinfo.)
	 */
	vtp = (struct vtoc *)&buf[pip->vtoc_ptr % dev_gran];

	if (vtp->v_sanity != VTOC_SANE) {
		boot_fs_type = s5;	/* No VTOC, assume s5 */
		return(root_delta);	/* No VTOC, return ivlab data */
	}
	for (i = 0; i < (int)vtp->v_nparts; i++) {

		switch (vtp->v_part[i].p_tag)  {

		case V_STAND: 
			debug(printf("\nfound V_STAND partition\n"));
			boot_delta=vtp->v_part[i].p_start;
			boot_fs_type = BFS;	/* originally UNKNOWN */
			break;

		case V_ROOT: 
			debug(printf("\nfound V_ROOT partition\n"));
			boot_fs_type = s5;	/* originally UNKNOWN */
			root_delta=vtp->v_part[i].p_start;
			break;
		default:
			break;
		}
	}

	if ( !boot_delta && !root_delta ) {
		printf("\nboot:No file system (stand or root) to boot from.\n");
		return (0);
	}

	debug(printf("ret from get_fs: root_delta == %d\n", root_delta));
	return (root_delta);
}

/*
 *
 *	This procedure checks the alternate block table and remaps bad 
 *	blocks if needed.  Accepts the sector number as a parameter and 
 *	returns the remapped sector number.
 *
 *	newsector
 *		is a 32-bit quantity returning the remapped sector number.
 *	sector
 *		is a 32-bit quantity identifying the sector number to be 
 *		checked in the alternate block table.
*/
ulong
altmap(sector)
register ulong	sector;
{	register ushort altidx;

	for(altidx=0;altidx<Alt_tbl.alt_sec.alt_used;altidx++) {
		if ( Alt_tbl.alt_sec.alt_bad[altidx] == sector) {
			sector = Alt_tbl.alt_sec.alt_base + altidx;
			break;
		}
	}
	debug(printf("altmap: returning %d\n", sector));
	return(sector);
}

/*
 *	This procedure implements the block read function for disks.
 *	It transfers BFS_BSIZE/s5blksiz bytes in static buffer and does 
 * 	alternate blocking. If the block is already cached, it returns.
 *
 *	bno	is a 32-bit quantity identifying the block number on the disk 
 *		to be read.
*/
extern char 	gbuf[];			/* size is MAX_BSIZE */
extern int	s5blksiz;
extern daddr_t 	gbuf_cache;
extern off_t	fsdelta;		/* sector offset to filesystem */

dread(bno)
register ulong	bno;
{	register	int	offset;
	register 	ulong	secno;
	register	int	bsize;
		
	debug(printf("DREAD bno: %ld, gbuf_cache: %ld\n", bno, gbuf_cache));

	if (bno == gbuf_cache)
		return;
	
	/* determine physical sector number on disk */

	switch (boot_fs_type)  {
	case s5: 
		bsize = s5blksiz;
		secno = (bno*bsize)/dev_gran + root_delta;
		break;
	case BFS:
		bsize = BFS_BSIZE;
		secno = (bno*bsize)/dev_gran + boot_delta;
		break;
	default:
		fatal("dread:Unknown filesystem type\n");
		break;
	}

	for(offset = 0; offset < bsize; offset += dev_gran) {
		debug(printf("dread: sector %d\n", secno));
		disk_read(altmap(secno++), ONE_BLOCK, &gbuf[offset], 
				ourDS, &g_actual, &g_status);
		debug(printf("Have Read sector\n"));
	}

	if ( (bno*bsize)%dev_gran != 0)
		iomove (gbuf+bsize, ourDS, gbuf, ourDS, bsize);

	gbuf_cache = bno;
}
