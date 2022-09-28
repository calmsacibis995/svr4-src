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

#ident	"@(#)ufs.cmds:ufs/fsck/utilities.c	1.8.3.1"

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mntent.h>
#include <sys/fs/ufs_fs.h>
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#define _KERNEL
#include <sys/fs/ufs_fsdir.h>
#undef _KERNEL
#include <sys/mnttab.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#if 0
#include <sys/cmn_err.h>
#endif
#include <string.h>
#include "fsck.h"
#include <sys/vfstab.h>

long	lseek();

ftypeok(dp)
	DINODE *dp;
{
	switch (dp->di_mode & IFMT) {

	case IFDIR:
	case IFREG:
	case IFBLK:
	case IFCHR:
	case IFLNK:
	case IFSOCK:
	case IFIFO:
		return (1);

	default:
		if (debug)
			printf("bad file type 0%o\n", dp->di_mode);
		return (0);
	}
}

reply(s)
	char *s;
{
	char line[80];

	if (preen)
		pfatal("INTERNAL ERROR: GOT TO reply()");
	printf("\n%s? ", s);
	if (nflag || dfile.wfdes < 0) {
		printf(" no\n\n");
		exitstat = 36;		/* remember there's still an error */
		return (0);
	}
	if (yflag) {
		printf(" yes\n\n");
		return (1);
	}
	if (getline(stdin, line, sizeof(line)) == EOF)
		errexit("\n");
	printf("\n");
	if (line[0] == 'y' || line[0] == 'Y')
		return (1);
	else {
		exitstat = 36;		/* remember there's still an error */
		return (0);
	}
}

getline(fp, loc, maxlen)
	FILE *fp;
	char *loc;
{
	register n;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[maxlen-1];
	while ((n = getc(fp)) != '\n') {
		if (n == EOF)
			return (EOF);
		if (!isspace(n) && p < lastloc)
			*p++ = n;
	}
	*p = 0;
	return (p - loc);
}

BUFAREA *
getblk(bp, blk, size)
	register BUFAREA *bp;
	daddr_t blk;
	long size;
{
	register struct filecntl *fcp;
	daddr_t dblk;

	fcp = &dfile;
	dblk = fsbtodb(&sblock, blk);
	if (bp->b_bno == dblk)
		return (bp);
	flush(fcp, bp);
	bp->b_errs = bread(fcp, bp->b_un.b_buf, dblk, size);
	bp->b_bno = dblk;
	bp->b_size = size;
	return (bp);
}

flush(fcp, bp)
	struct filecntl *fcp;
	register BUFAREA *bp;
{
	register int i, j;

	if (!bp->b_dirty)
		return;
	if (bp->b_errs != 0)
		pfatal("WRITING ZERO'ED BLOCK %d TO DISK\n", bp->b_bno);
	bp->b_dirty = 0;
	bp->b_errs = 0;
	bwrite(fcp, bp->b_un.b_buf, bp->b_bno, (long)bp->b_size);
	if (bp != &sblk)
		return;
	for (i = 0, j = 0; i < sblock.fs_cssize; i += sblock.fs_bsize, j++) {
		bwrite(&dfile, (char *)sblock.fs_csp[j],
		    fsbtodb(&sblock, sblock.fs_csaddr + j * sblock.fs_frag),
		    sblock.fs_cssize - i < sblock.fs_bsize ?
		    sblock.fs_cssize - i : sblock.fs_bsize);
	}
}

rwerr(s, blk)
	char *s;
	daddr_t blk;
{

	if (preen == 0)
		printf("\n");
	pfatal("CANNOT %s: BLK %ld", s, blk);
	if (reply("CONTINUE") == 0)
		errexit("Program terminated\n");
}

ckfini()
{

	flush(&dfile, &fileblk);
	flush(&dfile, &sblk);
	if (sblk.b_bno != SBLOCK) {
		sblk.b_bno = SBLOCK;
		sbdirty();
		flush(&dfile, &sblk);
	}
	flush(&dfile, &inoblk);
	flush(&dfile, &cgblk);
	(void)close(dfile.rfdes);
	(void)close(dfile.wfdes);
}

bread(fcp, buf, blk, size)
	register struct filecntl *fcp;
	char *buf;
	daddr_t blk;
	long size;
{
	char *cp;
	int i, errs;

	if (lseek(fcp->rfdes, (long)dbtob(blk), 0) < 0)
		rwerr("SEEK", blk);
	else if (read(fcp->rfdes, buf, (int)size) == size)
		return (0);
	rwerr("READ", blk);
	if (lseek(fcp->rfdes, (long)dbtob(blk), 0) < 0)
		rwerr("SEEK", blk);
	errs = 0;
	pfatal("THE FOLLOWING SECTORS COULD NOT BE READ:");
	for (cp = buf, i = 0; i < size; i += DEV_BSIZE, cp += DEV_BSIZE) {
		if (read(fcp->rfdes, cp, DEV_BSIZE) < 0) {
			printf(" %d,", blk + i / DEV_BSIZE);
			bzero(cp, DEV_BSIZE);
			errs++;
		}
	}
	printf("\n");
	return (errs);
}

bwrite(fcp, buf, blk, size)
	register struct filecntl *fcp;
	char *buf;
	daddr_t blk;
	long size;
{
	int i;
	char *cp;

	if (fcp->wfdes < 0)
		return;
	if (lseek(fcp->wfdes, (long)dbtob(blk), 0) < 0)
		rwerr("SEEK", blk);
	else if (write(fcp->wfdes, buf, (int)size) == size) {
		fcp->mod = 1;
		return;
	}
	rwerr("WRITE", blk);
	if (lseek(fcp->wfdes, (long)dbtob(blk), 0) < 0)
		rwerr("SEEK", blk);
	pfatal("THE FOLLOWING SECTORS COULD NOT BE WRITTEN:");
	for (cp = buf, i = 0; i < size; i += DEV_BSIZE, cp += DEV_BSIZE)
		if (write(fcp->wfdes, cp, DEV_BSIZE) < 0)
			printf(" %d,", blk + i / DEV_BSIZE);
	printf("\n");
	return;
}

/*
 * allocate a data block with the specified number of fragments
 */
allocblk(frags)
	int frags;
{
	register int i, j, k;

	if (frags <= 0 || frags > sblock.fs_frag)
		return (0);
	for (i = 0; i < fmax - sblock.fs_frag; i += sblock.fs_frag) {
		for (j = 0; j <= sblock.fs_frag - frags; j++) {
			if (getbmap(i + j))
				continue;
			for (k = 1; k < frags; k++)
				if (getbmap(i + j + k))
					break;
			if (k < frags) {
				j += k;
				continue;
			}
			for (k = 0; k < frags; k++)
				setbmap(i + j + k);
			n_blks += frags;
			return (i + j);
		}
	}
	return (0);
}

/*
 * Free a previously allocated block
 */
freeblk(blkno, frags)
	daddr_t blkno;
	int frags;
{
	struct inodesc idesc;

	idesc.id_blkno = blkno;
	idesc.id_numfrags = frags;
	pass4check(&idesc);
}

/*
 * Find a pathname
 */
getpathname(namebuf, curdir, ino)
	char *namebuf;
	ino_t curdir, ino;
{
	int len;
	register char *cp;
	struct inodesc idesc;
	extern int findname();

	if (statemap[ino] != DSTATE && statemap[ino] != DFOUND) {
		strcpy(namebuf, "?");
		return;
	}
	bzero(&idesc, sizeof(struct inodesc));
	idesc.id_type = DATA;
	cp = &namebuf[BUFSIZ - 1];
	*cp-- = '\0';
	if (curdir != ino) {
		idesc.id_parent = curdir;
		goto namelookup;
	}
	while (ino != UFSROOTINO) {
		idesc.id_number = ino;
		idesc.id_func = findino;
		idesc.id_name = "..";
		if ((ckinode(ginode(ino), &idesc) & STOP) == 0)
			break;
	namelookup:
		idesc.id_number = idesc.id_parent;
		idesc.id_parent = ino;
		idesc.id_func = findname;
		idesc.id_name = namebuf;
		if ((ckinode(ginode(idesc.id_number), &idesc) & STOP) == 0)
			break;
		len = strlen(namebuf);
		cp -= len;
		if (cp < &namebuf[MAXNAMLEN])
			break;
		bcopy(namebuf, cp, len);
		*--cp = '/';
		ino = idesc.id_number;
	}
	if (ino != UFSROOTINO) {
		strcpy(namebuf, "?");
		return;
	}
	bcopy(cp, namebuf, &namebuf[BUFSIZ] - cp);
}

catch()
{

	ckfini();
	exit(37);
}

/*
 * When preening, allow a single quit to signal
 * a special exit after filesystem checks complete
 * so that reboot sequence may be interrupted.
 */
catchquit()
{
	extern returntosingle;

	printf("returning to single-user after filesystem check\n");
	returntosingle = 1;
	(void)signal(SIGQUIT, SIG_DFL);
}

/*
 * Ignore a single quit signal; wait and flush just in case.
 * Used by child processes in preen.
 */
voidquit()
{

	sleep(1);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_DFL);
}

/*
 * determine whether an inode should be fixed.
 */
dofix(idesc, msg)
	register struct inodesc *idesc;
	char *msg;
{

	switch (idesc->id_fix) {

	case DONTKNOW:
		if (idesc->id_type == DATA)
			direrr(idesc->id_number, msg);
		else
			pwarn(msg);
		if (preen) {
			printf(" (SALVAGED)\n");
			idesc->id_fix = FIX;
			return (ALTERED);
		}
		if (reply("SALVAGE") == 0) {
			idesc->id_fix = NOFIX;
			return (0);
		}
		idesc->id_fix = FIX;
		return (ALTERED);

	case FIX:
		return (ALTERED);

	case NOFIX:
		return (0);

	default:
		errexit("UNKNOWN INODESC FIX MODE %d\n", idesc->id_fix);
	}
	/* NOTREACHED */
}

/* VARARGS1 */
errexit(s1, s2, s3, s4)
	char *s1;
{
	printf(s1, s2, s3, s4);
	exit(39);
}

/*
 * An inconsistency occured which shouldn't during normal operations.
 * Die if preening, otherwise just printf.
 */
/* VARARGS1 */
pfatal(s, a1, a2, a3)
	char *s;
{

	if (preen) {
		printf("%s: ", devname);
		printf(s, a1, a2, a3);
		printf("\n");
		printf("%s: UNEXPECTED INCONSISTENCY; RUN fsck MANUALLY.\n",
			devname);
		exit(36);
	}
	printf(s, a1, a2, a3);
}

/*
 * Pwarn is like printf when not preening,
 * or a warning (preceded by filename) when preening.
 */
/* VARARGS1 */
pwarn(s, a1, a2, a3, a4, a5, a6)
	char *s;
{

	if (preen)
		printf("%s: ", devname);
	printf(s, a1, a2, a3, a4, a5, a6);
}

#ifndef lint
/*
 * Stub for routines from kernel.
 */
panic(s)
	char *s;
{

	pfatal("INTERNAL INCONSISTENCY:");
	errexit(s);
}
#define CE_PANIC 3
void
cmn_err(level, s)
	register int level;
	char *s;
{

	if (level == CE_PANIC) {
		pfatal("INTERNAL INCONSISTENCY:");
		errexit(s);
	}
	else
		printf(s);
}
#endif

/*
 * Check to see if unraw version of name is already mounted.
 * Since we do not believe /etc/mnttab, we stat the mount point
 * to see if it is really looks mounted.
 */
mounted(name)
	char *name;
{
	int found = 0;
	struct mnttab mnt;
	FILE *mnttab;
	struct stat device_stat, mount_stat;
	int status;
	char *blkname, *unrawname();
	static char buf[MAXPATHLEN];

	mnttab = fopen(MNTTAB, "r");
	if (mnttab == NULL) {
		printf("can't open %s\n", MNTTAB);
		return (0);
	}
	(void) strcpy(buf, name);
	blkname = unrawname(buf);
	while ((getmntent(mnttab, &mnt)) == NULL) {
		if (strcmp(mnt.mnt_fstype, MNTTYPE_UFS) != 0) {
			continue;
		}
		if (strcmp(blkname, mnt.mnt_special) == 0) {
			stat(mnt.mnt_mountp, &mount_stat);
			stat(mnt.mnt_special, &device_stat);
			if (device_stat.st_rdev == mount_stat.st_dev) {
				if (hasmntopt (&mnt, MNTOPT_RO) != 0)
					found = 2;	/* mounted as RO */
				else	
					found = 1; 	/* mounted as R/W */
			}
			break;
		}
	}
	fclose(mnttab);
	return (found);
}

/*
 * Check to see if name corresponds to an entry in vfstab, and that the entry
 * does not have option ro.
 */
writable(name)
	char *name;
{
	int rw = 1;
	struct vfstab vfsbuf;
	FILE *vfstab;

	vfstab = fopen(VFSTAB, "r");
	if (vfstab == NULL) {
		printf("can't open %s\n", VFSTAB);
		return (1);
	}
	while ((getvfsent(vfstab, &vfsbuf)) == NULL) {
		if (strcmp(vfsbuf.vfs_fstype, MNTTYPE_UFS) != 0) {
			if ((strcmp(name, vfsbuf.vfs_special) == 0) ||
			    (strcmp(name, vfsbuf.vfs_mountp) == 0)) {
				printf ("%s is nfs mounted - ignored\n", name);
				rw = 0;
			}
			continue;
		}
		if ((strcmp(name, vfsbuf.vfs_special) == 0) ||
		    (strcmp(name, vfsbuf.vfs_mountp) == 0)) {
			if (hasvfsopt(&vfsbuf, MNTOPT_RO)) {
				rw = 0;
			}
			break;
		}
	}
	fclose(vfstab);
	return (rw);
}

char *
xmalloc(size)
	int size;
{
	char *ret;
	
	if ((ret = (char *)malloc(size)) == NULL) {
		errexit("ran out of memory!\n");
	}
	return (ret);
}

struct mnttab *
mntdup(mnt)
	struct mnttab *mnt;
{
	struct mnttab *new;

	new = (struct mnttab *)xmalloc(sizeof(*new));

	new->mnt_special = (char *)xmalloc(strlen(mnt->mnt_special) + 1);
	strcpy(new->mnt_special, mnt->mnt_special);

	new->mnt_mountp = (char *)xmalloc(strlen(mnt->mnt_mountp) + 1);
	strcpy(new->mnt_mountp, mnt->mnt_mountp);

	new->mnt_fstype = (char *)xmalloc(strlen(mnt->mnt_fstype) + 1);
	strcpy(new->mnt_fstype, mnt->mnt_fstype);

	new->mnt_mntopts = (char *)xmalloc(strlen(mnt->mnt_mntopts) + 1);
	strcpy(new->mnt_mntopts, mnt->mnt_mntopts);

#ifdef never
	new->mnt_freq = mnt->mnt_freq;
	new->mnt_passno = mnt->mnt_passno;
#endif never

	return (new);
}

/* see if all numbers */
numbers(yp)
	char	*yp;
{
	if (yp == NULL)
		return	0;
	while ('0' <= *yp && *yp <= '9')
		yp++;
	if (*yp)
		return	0;
	return	1;
}

