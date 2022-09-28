#ident	"@(#)inode.c	1.2	91/09/21	JPB"
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)ufs.cmds:ufs/fsck/inode.c	1.3.3.1"

#include <stdio.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/mntent.h>
#include <sys/fs/ufs_fs.h>
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#define _KERNEL
#include <sys/fs/ufs_fsdir.h>
#undef _KERNEL
#include <sys/mnttab.h>
#include "fsck.h"

ckinode(dp, idesc)
	DINODE *dp;
	register struct inodesc *idesc;
{
	register daddr_t *ap;
	int ret, n, ndb, offset;
	DINODE dino;

	idesc->id_fix = DONTKNOW;
	idesc->id_entryno = 0;
	idesc->id_filesize = dp->di_size;
	if (SPECIAL(dp))
		return (KEEPON);
	dino = *dp;
	ndb = howmany(dino.di_size, sblock.fs_bsize);
	for (ap = &dino.di_db[0]; ap < &dino.di_db[NDADDR]; ap++) {
		if (--ndb == 0 && (offset = blkoff(&sblock, dino.di_size)) != 0)
			idesc->id_numfrags =
				numfrags(&sblock, fragroundup(&sblock, offset));
		else
			idesc->id_numfrags = sblock.fs_frag;
		if (*ap == 0)
			continue;
		idesc->id_blkno = *ap;
		if (idesc->id_type == ADDR)
			ret = (*idesc->id_func)(idesc);
		else
			ret = dirscan(idesc);
		if (ret & STOP)
			return (ret);
	}
	idesc->id_numfrags = sblock.fs_frag;
	for (ap = &dino.di_ib[0], n = 1; n <= NIADDR; ap++, n++) {
		if (*ap) {
			idesc->id_blkno = *ap;
			ret = iblock(idesc, n,
				dino.di_size - sblock.fs_bsize * NDADDR);
			if (ret & STOP)
				return (ret);
		}
	}
	return (KEEPON);
}

iblock(idesc, ilevel, isize)
	struct inodesc *idesc;
	register ilevel;
	long isize;
{
	register daddr_t *ap;
	register daddr_t *aplim;
	int i, n, (*func)(), nif, sizepb;
	BUFAREA ib;
	char buf[BUFSIZ];
	extern int pass1check();
	extern int dirscan();

	if (idesc->id_type == ADDR) {
		func = idesc->id_func;
		if (((n = (*func)(idesc)) & KEEPON) == 0)
			return (n);
	} else
		func = dirscan;
	if (outrange(idesc->id_blkno, idesc->id_numfrags)) /* protect thyself */
		return (SKIP);
	initbarea(&ib);
	getblk(&ib, idesc->id_blkno, sblock.fs_bsize);
	if (ib.b_errs != NULL)
		return (SKIP);
	ilevel--;
	for (sizepb = sblock.fs_bsize, i = 0; i < ilevel; i++)
		sizepb *= NINDIR(&sblock);
	nif = isize / sizepb + 1;
	if (nif > NINDIR(&sblock))
		nif = NINDIR(&sblock);
	if (idesc->id_func == pass1check && nif < NINDIR(&sblock)) {
		aplim = &ib.b_un.b_indir[NINDIR(&sblock)];
		for (ap = &ib.b_un.b_indir[nif]; ap < aplim; ap++) {
			if (*ap == 0)
				continue;
			sprintf(buf, "PARTIALLY TRUNCATED INODE I=%d",
				idesc->id_number);
			if (dofix(idesc, buf)) {
				*ap = 0;
				dirty(&ib);
			}
		}
		flush(&dfile, &ib);
	}
	aplim = &ib.b_un.b_indir[nif];
	for (ap = ib.b_un.b_indir, i = 1; ap < aplim; ap++, i++)
		if (*ap) {
			idesc->id_blkno = *ap;
			if (ilevel > 0)
				n = iblock(idesc, ilevel, isize - i * sizepb);
			else
				n = (*func)(idesc);
			if (n & STOP)
				return (n);
		}
	return (KEEPON);
}

outrange(blk, cnt)
	daddr_t blk;
	int cnt;
{
	register int c;

	if ((unsigned)(blk+cnt) > fmax)
		return (1);
	c = dtog(&sblock, blk);
	if (blk < cgdmin(&sblock, c)) {
		if ((blk+cnt) > cgsblock(&sblock, c)) {
			if (debug) {
				printf("blk %d < cgdmin %d;",
				    blk, cgdmin(&sblock, c));
				printf(" blk+cnt %d > cgsbase %d\n",
				    blk+cnt, cgsblock(&sblock, c));
			}
			return (1);
		}
	} else {
		if ((blk+cnt) > cgbase(&sblock, c+1)) {
			if (debug)  {
				printf("blk %d >= cgdmin %d;",
				    blk, cgdmin(&sblock, c));
				printf(" blk+cnt %d > sblock.fs_fpg %d\n",
				    blk+cnt, sblock.fs_fpg);
			}
			return (1);
		}
	}
	return (0);
}

DINODE *
ginode(inumber)
	ino_t inumber;
{
	daddr_t iblk;
	static ino_t startinum = 0;	/* blk num of first in raw area */
	register DINODE *dp;

	if (inumber < UFSROOTINO || inumber > imax)
		errexit("bad inode number %d to ginode\n", inumber);
	if (startinum == 0 ||
	    inumber < startinum || inumber >= (ino_t)(startinum + (ino_t)INOPB(&sblock))) {
		iblk = itod(&sblock, (int)inumber);
		getblk(&inoblk, iblk, sblock.fs_bsize);
		startinum = (ino_t)(((int)inumber / INOPB(&sblock)) * INOPB(&sblock));
	}
	dp =&inoblk.b_un.b_dinode[(int)inumber % INOPB(&sblock)];
	if (dp ->di_eftflag != EFT_MAGIC) {
		dp->di_mode = dp->di_smode;
		dp->di_uid = dp->di_suid;
		dp->di_gid = dp->di_sgid;
	}
	return(dp);
}

clri(idesc, s, flg)
	register struct inodesc *idesc;
	char *s;
	int flg;
{
	register DINODE *dp;

	dp = ginode(idesc->id_number);
	if (flg == 1) {
		pwarn("%s %s", s, DIRCT(dp) ? "DIR" : "FILE");
		pinode(idesc->id_number);
	}
	if (preen || reply("CLEAR") == 1) {
		if (preen)
			printf(" (CLEARED)\n");
		n_files--;
		(void)ckinode(dp, idesc);
		zapino(dp);
		statemap[idesc->id_number] = USTATE;
		inodirty();
	}
}

findname(idesc)
	struct inodesc *idesc;
{
	register DIRECT *dirp = idesc->id_dirp;

	if (dirp->d_ino != idesc->id_parent)
		return (KEEPON);
	bcopy(dirp->d_name, idesc->id_name, dirp->d_namlen + 1);
	return (STOP);
}

findino(idesc)
	struct inodesc *idesc;
{
	register DIRECT *dirp = idesc->id_dirp;

	if (dirp->d_ino == 0)
		return (KEEPON);
	if (strcmp(dirp->d_name, idesc->id_name) == 0 &&
	    dirp->d_ino >= UFSROOTINO && dirp->d_ino <= imax) {
		idesc->id_parent = dirp->d_ino;
		return (STOP);
	}
	return (KEEPON);
}

pinode(ino)
	ino_t ino;
{
	register DINODE *dp;
	register char *p;
	struct passwd *pw;
	char *ctime();

	printf(" I=%u ", ino);
	if (ino < UFSROOTINO || ino > imax)
		return;
	dp = ginode(ino);
	printf(" OWNER=");
	if ((pw = getpwuid((int)dp->di_uid)) != 0)
		printf("%s ", pw->pw_name);
	else
		printf("%d ", dp->di_uid);
	printf("MODE=%o\n", dp->di_mode);
	if (preen)
		printf("%s: ", devname);
	printf("SIZE=%ld ", dp->di_size);
	p = ctime(&dp->di_mtime);
	printf("MTIME=%12.12s %4.4s ", p+4, p+20);
}

blkerr(ino, s, blk)
	ino_t ino;
	char *s;
	daddr_t blk;
{

	pfatal("%ld %s I=%u", blk, s, ino);
	printf("\n");
	switch (statemap[ino]) {

	case FSTATE:
		statemap[ino] = FCLEAR;
		return;

	case DSTATE:
		statemap[ino] = DCLEAR;
		return;

	case FCLEAR:
	case DCLEAR:
		return;

	default:
		errexit("BAD STATE %d TO BLKERR", statemap[ino]);
		/* NOTREACHED */
	}
}

/*
 * allocate an unused inode
 */
ino_t
allocino(request, type)
	ino_t request;
	int type;
{
	register ino_t ino;
	register DINODE *dp;

	if (request == 0)
		request = UFSROOTINO;
	else if (statemap[request] != USTATE)
		return (0);
	for (ino = request; ino < imax; ino++)
		if (statemap[ino] == USTATE)
			break;
	if (ino == imax)
		return (0);
	switch (type & IFMT) {
	case IFDIR:
		statemap[ino] = DSTATE;
		break;
	case IFREG:
	case IFLNK:
		statemap[ino] = FSTATE;
		break;
	default:
		return (0);
	}
	dp = ginode(ino);
	dp->di_db[0] = allocblk(1);
	if (dp->di_db[0] == 0) {
		statemap[ino] = USTATE;
		return (0);
	}
	dp->di_smode = dp->di_mode = type;
	dp->di_eftflag = EFT_MAGIC;                                 
	time(&dp->di_atime);
	dp->di_mtime = dp->di_ctime = dp->di_atime;
	dp->di_size = sblock.fs_fsize;
	dp->di_blocks = btodb(sblock.fs_fsize);
	n_files++;
	inodirty();
	return (ino);
}

/*
 * deallocate an inode
 */
freeino(ino)
	ino_t ino;
{
	struct inodesc idesc;
	extern int pass4check();
	DINODE *dp;

	bzero((char *)&idesc, sizeof(struct inodesc));
	idesc.id_type = ADDR;
	idesc.id_func = pass4check;
	idesc.id_number = ino;
	dp = ginode(ino);
	(void)ckinode(dp, &idesc);
	zapino(dp);
	inodirty();
	statemap[ino] = USTATE;
	n_files--;
}
