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

#ident	"@(#)ufs.cmds:ufs/fsck/dir.c	1.2.5.1"

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/mntent.h>
#include <sys/fs/ufs_fs.h>
/*#include <vm/time.h>*/
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#define _KERNEL
#include <sys/fs/ufs_fsdir.h>
#undef _KERNEL
#include <sys/mnttab.h>
#include "fsck.h"

#define MINDIRSIZE	(sizeof (struct dirtemplate))

char	*endpathname = &pathname[BUFSIZ - 2];
char	*lfname = "lost+found";
struct	dirtemplate emptydir = { 0, DIRBLKSIZ};
struct	dirtemplate dirhead = { 0, 12, 1, ".", 0, DIRBLKSIZ - 12, 2, ".." };

DIRECT	*fsck_readdir();

descend(parentino, inumber)
	struct inodesc *parentino;
	ino_t inumber;
{
	register DINODE *dp;
	struct inodesc curino;

	bzero((char *)&curino, sizeof(struct inodesc));
	if (statemap[inumber] != DSTATE)
		errexit("BAD INODE %d TO DESCEND", statemap[inumber]);
	statemap[inumber] = DFOUND;
	dp = ginode(inumber);
	if (dp->di_size == 0) {
		direrr(inumber, "ZERO LENGTH DIRECTORY");
		if (reply("REMOVE") == 1)
			statemap[inumber] = DCLEAR;
		return;
	}
	if (dp->di_size < MINDIRSIZE) {
		direrr(inumber, "DIRECTORY TOO SHORT");
		dp->di_size = MINDIRSIZE;
		if (reply("FIX") == 1)
			inodirty();
	}
	if ((dp->di_size & (DIRBLKSIZ - 1)) != 0) {
		pwarn("DIRECTORY %s: LENGTH %d NOT MULTIPLE OF %d",
			pathname, dp->di_size, DIRBLKSIZ);
		dp->di_size = roundup(dp->di_size, DIRBLKSIZ);
		if (preen)
			printf(" (ADJUSTED)\n");
		if (preen || reply("ADJUST") == 1)
			inodirty();
	}
	curino.id_type = DATA;
	curino.id_func = parentino->id_func;
	curino.id_parent = parentino->id_number;
	curino.id_number = inumber;
	(void)ckinode(dp, &curino);
}

dirscan(idesc)
	register struct inodesc *idesc;
{
	register DIRECT *dp;
	int dsize, n;
	long blksiz;
	char dbuf[DIRBLKSIZ];

	if (idesc->id_type != DATA)
		errexit("wrong type to dirscan %d\n", idesc->id_type);
	if (idesc->id_entryno == 0 &&
	    (idesc->id_filesize & (DIRBLKSIZ - 1)) != 0)
		idesc->id_filesize = roundup(idesc->id_filesize, DIRBLKSIZ);
	blksiz = idesc->id_numfrags * sblock.fs_fsize;
	if (outrange(idesc->id_blkno, idesc->id_numfrags)) {
		idesc->id_filesize -= blksiz;
		return (SKIP);
	}
	idesc->id_loc = 0;
	for (dp = fsck_readdir(idesc); dp != NULL; dp = fsck_readdir(idesc)) {
		dsize = dp->d_reclen;
		bcopy((char *)dp, dbuf, dsize);
		idesc->id_dirp = (DIRECT *)dbuf;
		if ((n = (*idesc->id_func)(idesc)) & ALTERED) {
			getblk(&fileblk, idesc->id_blkno, blksiz);
			if (fileblk.b_errs != NULL) {
				n &= ~ALTERED;
			} else {
				bcopy(dbuf, (char *)dp, dsize);
				dirty(&fileblk);
				sbdirty();
			}
		}
		if (n & STOP) 
			return (n);
	}
	return (idesc->id_filesize > 0 ? KEEPON : STOP);
}

/*
 * get next entry in a directory.
 */
DIRECT *
fsck_readdir(idesc)
	register struct inodesc *idesc;
{
	register DIRECT *dp, *ndp;
	long size, blksiz;

	blksiz = idesc->id_numfrags * sblock.fs_fsize;
	getblk(&fileblk, idesc->id_blkno, blksiz);
	if (fileblk.b_errs != NULL) {
		idesc->id_filesize -= blksiz - idesc->id_loc;
		return NULL;
	}
	if (idesc->id_loc % DIRBLKSIZ == 0 && idesc->id_filesize > 0 &&
	    idesc->id_loc < blksiz) {
		dp = (DIRECT *)(dirblk.b_buf + idesc->id_loc);
		if (dircheck(idesc, dp))
			goto dpok;
		idesc->id_loc += DIRBLKSIZ;
		idesc->id_filesize -= DIRBLKSIZ;
		dp->d_reclen = DIRBLKSIZ;
		dp->d_ino = 0;
		dp->d_namlen = 0;
		dp->d_name[0] = '\0';
		if (dofix(idesc, "DIRECTORY CORRUPTED"))
			dirty(&fileblk);
		return (dp);
	}
dpok:
	if (idesc->id_filesize <= 0 || idesc->id_loc >= blksiz)
		return NULL;
	dp = (DIRECT *)(dirblk.b_buf + idesc->id_loc);
	idesc->id_loc += dp->d_reclen;
	idesc->id_filesize -= dp->d_reclen;
	if ((idesc->id_loc % DIRBLKSIZ) == 0)
		return (dp);
	ndp = (DIRECT *)(dirblk.b_buf + idesc->id_loc);
	if (idesc->id_loc < blksiz && idesc->id_filesize > 0 &&
	    dircheck(idesc, ndp) == 0) {
		size = DIRBLKSIZ - (idesc->id_loc % DIRBLKSIZ);
		dp->d_reclen += size;
		idesc->id_loc += size;
		idesc->id_filesize -= size;
		if (dofix(idesc, "DIRECTORY CORRUPTED"))
			dirty(&fileblk);
	}
	return (dp);
}

/*
 * Verify that a directory entry is valid.
 * This is a superset of the checks made in the kernel.
 */
dircheck(idesc, dp)
	struct inodesc *idesc;
	register DIRECT *dp;
{
	register int size;
	register char *cp;
	int spaceleft;

	size = DIRSIZ(dp);
	spaceleft = DIRBLKSIZ - (idesc->id_loc % DIRBLKSIZ);
	if (dp->d_ino < imax &&
	    dp->d_reclen != 0 &&
	    (int)dp->d_reclen <= spaceleft &&
	    (dp->d_reclen & 0x3) == 0 &&
	    (int)dp->d_reclen >= size &&
	    idesc->id_filesize >= size &&
	    dp->d_namlen <= MAXNAMLEN) {
		if (dp->d_ino == 0)
			return (1);
		for (cp = dp->d_name, size = 0; size < (int)dp->d_namlen; size++)
			if (*cp++ == 0)
				return (0);
		if (*cp == 0)
			return (1);
	}
	return (0);
}

direrr(ino, s)
	ino_t ino;
	char *s;
{
	register DINODE *dp;

	pwarn("%s ", s);
	pinode(ino);
	printf("\n");
	if (ino < UFSROOTINO || ino > imax) {
		pfatal("NAME=%s\n", pathname);
		return;
	}
	dp = ginode(ino);
	if (ftypeok(dp))
		pfatal("%s=%s\n", DIRCT(dp) ? "DIR" : "FILE", pathname);
	else
		pfatal("NAME=%s\n", pathname);
}

adjust(idesc, lcnt)
	register struct inodesc *idesc;
	short lcnt;
{
	register DINODE *dp;

	dp = ginode(idesc->id_number);
	if (dp->di_nlink == lcnt) {
		if (linkup(idesc->id_number, (ino_t)0) == 0)
			clri(idesc, "UNREF", 0);
	} else {
		pwarn("LINK COUNT %s", (lfdir == idesc->id_number) ? lfname :
			(DIRCT(dp) ? "DIR" : "FILE"));
		pinode(idesc->id_number);
		printf(" COUNT %d SHOULD BE %d",
			dp->di_nlink, dp->di_nlink-lcnt);
		if (preen) {
			if (lcnt < 0) {
				printf("\n");
				pfatal("LINK COUNT INCREASING");
			}
			printf(" (ADJUSTED)\n");
		}
		if (preen || reply("ADJUST") == 1) {
			dp->di_nlink -= lcnt;
			inodirty();
		}
	}
}

mkentry(idesc)
	struct inodesc *idesc;
{
	register DIRECT *dirp = idesc->id_dirp;
	DIRECT newent;
	int newlen, oldlen;

	newent.d_namlen = 11;
	newlen = DIRSIZ(&newent);
	if (dirp->d_ino != 0)
		oldlen = DIRSIZ(dirp);
	else
		oldlen = 0;
	if ((int)dirp->d_reclen - oldlen < newlen)
		return (KEEPON);
	newent.d_reclen = dirp->d_reclen - oldlen;
	dirp->d_reclen = oldlen;
	dirp = (struct direct *)(((char *)dirp) + oldlen);
	dirp->d_ino = idesc->id_parent;	/* ino to be entered is in id_parent */
	dirp->d_reclen = newent.d_reclen;
	dirp->d_namlen = strlen(idesc->id_name);
	bcopy(idesc->id_name, dirp->d_name, dirp->d_namlen + 1);
	return (ALTERED|STOP);
}

chgino(idesc)
	struct inodesc *idesc;
{
	register DIRECT *dirp = idesc->id_dirp;

	if (bcmp(dirp->d_name, idesc->id_name, dirp->d_namlen + 1))
		return (KEEPON);
	dirp->d_ino = idesc->id_parent;;
	return (ALTERED|STOP);
}

linkup(orphan, pdir)
	ino_t orphan;
	ino_t pdir;
{
	register DINODE *dp;
	int lostdir, len;
	ino_t oldlfdir;
	struct inodesc idesc;
	char tempname[BUFSIZ];
	extern int pass4check();

	bzero((char *)&idesc, sizeof(struct inodesc));
	dp = ginode(orphan);
	lostdir = DIRCT(dp);
	pwarn("UNREF %s ", lostdir ? "DIR" : "FILE");
	pinode(orphan);
	if (preen && dp->di_size == 0) {
		printf ("\n");
		return (0);
	}
	if (preen)
		printf(" (RECONNECTED)\n");
	else {
		if (lostdir && dp->di_size < MINDIRSIZE && !nflag)
		{
			printf ("\n");
			return (0);
		}
		if (reply("RECONNECT") == 0)
			return (0);
	}
	pathp = pathname;
	*pathp++ = '/';
	*pathp = '\0';
	if (lfdir == 0) {
		dp = ginode(UFSROOTINO);
		idesc.id_name = lfname;
		idesc.id_type = DATA;
		idesc.id_func = findino;
		idesc.id_number = UFSROOTINO;
		(void)ckinode(dp, &idesc);
		if (idesc.id_parent >= UFSROOTINO && idesc.id_parent < imax) {
			lfdir = idesc.id_parent;
		} else {
			pwarn("NO lost+found DIRECTORY");
			if (preen || reply("CREATE")) {
				lfdir = allocdir(UFSROOTINO, 0);
				if (lfdir != 0) {
					if (makeentry(UFSROOTINO, lfdir, lfname) != 0) {
						if (preen)
							printf(" (CREATED)\n");
					} else {
						freedir(lfdir, UFSROOTINO);
						lfdir = 0;
						if (preen)
							printf("\n");
					}
				}
			}
		}
		if (lfdir == 0) {
			pfatal("SORRY. CANNOT CREATE lost+found DIRECTORY");
			printf("\n\n");
			return (0);
		}
	} else {	/* lost+found already created */
		idesc.id_name = lfname;
	}
	dp = ginode(lfdir);
	if (!DIRCT(dp)) {
		pfatal("lost+found IS NOT A DIRECTORY");
		if (reply("REALLOCATE") == 0)
			return (0);
		oldlfdir = lfdir;
		if ((lfdir = allocdir(UFSROOTINO, 0)) == 0) {
			pfatal("SORRY. CANNOT CREATE lost+found DIRECTORY\n\n");
			return (0);
		}
		idesc.id_type = DATA;
		idesc.id_func = chgino;
		idesc.id_number = UFSROOTINO;
		idesc.id_parent = lfdir;	/* new inumber for lost+found */
		idesc.id_name = lfname;
		if ((ckinode(ginode(UFSROOTINO), &idesc) & ALTERED) == 0) {
			pfatal("SORRY. CANNOT CREATE lost+found DIRECTORY\n\n");
			return (0);
		}
		inodirty();
		idesc.id_type = ADDR;
		idesc.id_func = pass4check;
		idesc.id_number = oldlfdir;
		adjust(&idesc, lncntp[oldlfdir] + 1);
		lncntp[oldlfdir] = 0;
		dp = ginode(lfdir);
	}
	if (statemap[lfdir] != DFOUND) {
		pfatal("SORRY. NO lost+found DIRECTORY\n\n");
		return (0);
	}
	len = strlen(lfname);
	bcopy(lfname, pathp, len + 1);
	pathp += len;
	len = lftempname(tempname, orphan);
	if (makeentry(lfdir, orphan, tempname) == 0) {
		pfatal("SORRY. NO SPACE IN lost+found DIRECTORY");
		printf("\n\n");
		return (0);
	}
	lncntp[orphan]--;
	*pathp++ = '/';
	bcopy(idesc.id_name, pathp, len + 1);
	pathp += len;
	if (lostdir) {
		dp = ginode(orphan);
		idesc.id_type = DATA;
		idesc.id_func = chgino;
		idesc.id_number = orphan;
		idesc.id_fix = DONTKNOW;
		idesc.id_name = "..";
		idesc.id_parent = lfdir;	/* new value for ".." */
		(void)ckinode(dp, &idesc);
		dp = ginode(lfdir);
		dp->di_nlink++;
		inodirty();
		lncntp[lfdir]++;
		pwarn("DIR I=%u CONNECTED. ", orphan);
		printf("PARENT WAS I=%u\n", pdir);
		if (preen == 0)
			printf("\n");
	}
	return (1);
}

/*
 * make an entry in a directory
 */
makeentry(parent, ino, name)
	ino_t parent, ino;
	char *name;
{
	DINODE *dp;
	struct inodesc idesc;
	
	if (parent < UFSROOTINO || parent >= imax || ino < UFSROOTINO || ino >= imax)
		return (0);
	bzero(&idesc, sizeof(struct inodesc));
	idesc.id_type = DATA;
	idesc.id_func = mkentry;
	idesc.id_number = parent;
	idesc.id_parent = ino;	/* this is the inode to enter */
	idesc.id_fix = DONTKNOW;
	idesc.id_name = name;
	dp = ginode(parent);
	if (dp->di_size % DIRBLKSIZ) {
		dp->di_size = roundup(dp->di_size, DIRBLKSIZ);
		inodirty();
	}
	if ((ckinode(dp, &idesc) & ALTERED) != 0)
		return (1);
	if (expanddir(dp) == 0)
		return (0);
	return (ckinode(dp, &idesc) & ALTERED);
}

/*
 * Attempt to expand the size of a directory
 */
expanddir(dp)
	register DINODE *dp;
{
	daddr_t lastbn, newblk;
	char *cp, firstblk[DIRBLKSIZ];

	lastbn = lblkno(&sblock, dp->di_size);
	if (lastbn >= NDADDR - 1)
		return (0);
	if ((newblk = allocblk(sblock.fs_frag)) == 0)
		return (0);
	dp->di_db[lastbn + 1] = dp->di_db[lastbn];
	dp->di_db[lastbn] = newblk;
	dp->di_size += sblock.fs_bsize;
	dp->di_blocks += btodb(sblock.fs_bsize);
	getblk(&fileblk, dp->di_db[lastbn + 1],
	    dblksize(&sblock, dp, lastbn + 1));
	if (fileblk.b_errs != NULL)
		goto bad;
	bcopy(dirblk.b_buf, firstblk, DIRBLKSIZ);
	getblk(&fileblk, newblk, sblock.fs_bsize);
	if (fileblk.b_errs != NULL)
		goto bad;
	bcopy(firstblk, dirblk.b_buf, DIRBLKSIZ);
	for (cp = &dirblk.b_buf[DIRBLKSIZ];
	     cp < &dirblk.b_buf[sblock.fs_bsize];
	     cp += DIRBLKSIZ)
		bcopy((char *)&emptydir, cp, sizeof emptydir);
	dirty(&fileblk);
	getblk(&fileblk, dp->di_db[lastbn + 1],
	    dblksize(&sblock, dp, lastbn + 1));
	if (fileblk.b_errs != NULL)
		goto bad;
	bcopy((char *)&emptydir, dirblk.b_buf, sizeof emptydir);
	pwarn("NO SPACE LEFT IN %s", pathname);
	if (preen)
		printf(" (EXPANDED)\n");
	else if (reply("EXPAND") == 0)
		goto bad;
	dirty(&fileblk);
	inodirty();
	return (1);
bad:
	dp->di_db[lastbn] = dp->di_db[lastbn + 1];
	dp->di_db[lastbn + 1] = 0;
	dp->di_size -= sblock.fs_bsize;
	dp->di_blocks -= btodb(sblock.fs_bsize);
	freeblk(newblk, sblock.fs_frag);
	return (0);
}

/*
 * allocate a new directory
 */
allocdir(parent, request)
	ino_t parent, request;
{
	ino_t ino;
	char *cp;
	DINODE *dp;

	ino = allocino(request, IFDIR|0755);
	dirhead.dot_ino = ino;
	dirhead.dotdot_ino = parent;
	dp = ginode(ino);
	getblk(&fileblk, dp->di_db[0], sblock.fs_fsize);
	if (fileblk.b_errs != NULL) {
		freeino(ino);
		return (0);
	}
	bcopy((char *)&dirhead, dirblk.b_buf, sizeof dirhead);
	for (cp = &dirblk.b_buf[DIRBLKSIZ];
	     cp < &dirblk.b_buf[sblock.fs_fsize];
	     cp += DIRBLKSIZ)
		bcopy((char *)&emptydir, cp, sizeof emptydir);
	dirty(&fileblk);
	dp->di_nlink = 2;
	inodirty();
	if (ino == UFSROOTINO) {
		lncntp[ino] = dp->di_nlink;
		return(ino);
	}
	if (statemap[parent] != DSTATE && statemap[parent] != DFOUND) {
		freeino(ino);
		return (0);
	}
	statemap[ino] = statemap[parent];
	if (statemap[ino] == DSTATE) {
		lncntp[ino] = dp->di_nlink;
		lncntp[parent]++;
	}
	dp = ginode(parent);
	dp->di_nlink++;
	inodirty();
	return (ino);
}

/*
 * free a directory inode
 */
freedir(ino, parent)
	ino_t ino, parent;
{
	DINODE *dp;

	if (ino != parent) {
		dp = ginode(parent);
		dp->di_nlink--;
		inodirty();
	}
	freeino(ino);
}

/*
 * generate a temporary name for the lost+found directory.
 */
lftempname(bufp, ino)
	char *bufp;
	ino_t ino;
{
	register ino_t in;
	register char *cp;
	int namlen;

	cp = bufp + 2;
	for (in = imax; in > 0; in /= 10)
		cp++;
	*--cp = 0;
	namlen = cp - bufp;
	in = ino;
	while (cp > bufp) {
		*--cp = (in % 10) + '0';
		in /= 10;
	}
	*cp = '#';
	return (namlen);
}
