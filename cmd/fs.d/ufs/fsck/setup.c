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

#ident	"@(#)ufs.cmds:ufs/fsck/setup.c	1.8.4.1"

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/sysmacros.h>
#include <sys/mntent.h>
#include <sys/fs/ufs_fs.h>
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#include <sys/stat.h>
#define _KERNEL
#include <sys/fs/ufs_fsdir.h>
#undef _KERNEL
#include <sys/mnttab.h>
#include <string.h>
#include "fsck.h"
#include <sys/vfstab.h>
#include <sys/ustat.h>

char	*calloc();
extern int	mflag;
extern char 	hotroot;

char *
setup(dev)
	char *dev;
{
	dev_t rootdev;
	struct stat statb;
	daddr_t super = bflag ? bflag : SBLOCK;
	int i, j;
	long size;
	BUFAREA asblk;
	static char devstr[MAXPATHLEN];
#	define altsblock asblk.b_un.b_fs
	char	*raw, *rawname();
	int mntcode;
	struct ustat ustatb;

	if (stat("/", &statb) < 0)
		errexit("Can't stat root\n");
	rootdev = statb.st_dev;

	strcpy(devstr, dev);
	devname = dev;
restat:
	if (stat(devstr, &statb) < 0) {
		printf("Can't stat %s\n", devstr);
		return (0);
	}
	rawflg = 0;
	if ((statb.st_mode & S_IFMT) == S_IFBLK) {
		if (rootdev == statb.st_rdev)
			hotroot++;
		else if (ustat(statb.st_rdev, &ustatb) >= 0) {
			if (!nflag) {
				fprintf(stderr, "%s is a mounted file system, ignored\n",
				dev);
				return(0);
			}
		}	
	}			
	else if ((statb.st_mode & S_IFMT) == S_IFCHR)
		rawflg++;
	else if ((statb.st_mode & S_IFMT) == S_IFDIR) {
		FILE *vfstab;
		struct vfstab vfsbuf;
		/*
		 * Check vfstab for a mount point with this name
		 */
		if ((vfstab = fopen(VFSTAB, "r")) == NULL) {
			errexit("Can't open checklist file: %s\n", VFSTAB);
		}
		while (getvfsent(vfstab, &vfsbuf) == NULL) {
			if (strcmp(devstr,vfsbuf.vfs_mountp) == 0) {
				if (strcmp(vfsbuf.vfs_fstype, MNTTYPE_UFS) != 0) {
					/*
					 * found the entry but it is not a
					 * ufs filesystem, don't check it
					 */
					fclose(vfstab);
					return (0);
				}
				strcpy(devstr, vfsbuf.vfs_special);
				if (rflag) {
					raw =
					    rawname(unrawname(vfsbuf.vfs_special));
					strcpy(devstr, raw);
				}
				goto restat;
			}
		}
		fclose(vfstab);
	} else {
		if (reply("file is not a block or character device; OK") == 0)
			return (0);
	}
	if (mounted(devstr))
		if (rawflg)
			mountedfs++;
		else {
			printf("%s is mounted, fsck on BLOCK device ignored\n",
				devstr);
			exit(33);
		}
	if ((dfile.rfdes = open(devstr, 0)) < 0) {
		printf("Can't open %s\n", devstr);
		return (0);
	}
	if (preen == 0)
		printf("** %s", devstr);
	if (nflag || (dfile.wfdes = open(devstr, 1)) < 0) {
		dfile.wfdes = -1;
		if (preen)
			pfatal("NO WRITE ACCESS");
		printf(" (NO WRITE)");
	}
	if (preen == 0)
		printf("\n");
	dfile.mod = 0;
	lfdir = 0;
	initbarea(&sblk);
	initbarea(&fileblk);
	initbarea(&inoblk);
	initbarea(&cgblk);
	initbarea(&asblk);
	/*
	 * Read in the super block and its summary info.
	 */
	if (bread(&dfile, (char *)&sblock, super, (long)SBSIZE) != 0)
		return (0);
	sblk.b_bno = super;
	sblk.b_size = SBSIZE;
	/*
	 * run a few consistency checks of the super block
	 */
	if (sblock.fs_magic != FS_MAGIC)
		{ badsb("MAGIC NUMBER WRONG"); return (0); }
	if (sblock.fs_ncg < 1)
		{ badsb("NCG OUT OF RANGE"); return (0); }
	if (sblock.fs_cpg < 1 || sblock.fs_cpg > MAXCPG)
		{ badsb("CPG OUT OF RANGE"); return (0); }
	if (sblock.fs_ncg * sblock.fs_cpg < sblock.fs_ncyl ||
	    (sblock.fs_ncg - 1) * sblock.fs_cpg >= sblock.fs_ncyl)
		{ badsb("NCYL DOES NOT JIVE WITH NCG*CPG"); return (0); }
	if (sblock.fs_sbsize > SBSIZE)
		{ badsb("SIZE PREPOSTEROUSLY LARGE"); return (0); }
	if (mflag)
		return (devstr);
	/*
	 * Check and potentially fix certain fields in the super block.
	 */
	if (sblock.fs_optim != FS_OPTTIME && sblock.fs_optim != FS_OPTSPACE) {
		pfatal("UNDEFINED OPTIMIZATION IN SUPERBLOCK");
		if (reply("SET TO DEFAULT") == 1) {
			sblock.fs_optim = FS_OPTTIME;
			sbdirty();
		}
	}
	if ((sblock.fs_minfree < 0 || sblock.fs_minfree > 99)) {
		pfatal("IMPOSSIBLE MINFREE=%d IN SUPERBLOCK",
			sblock.fs_minfree);
		if (reply("SET TO DEFAULT") == 1) {
			sblock.fs_minfree = 10;
			sbdirty();
		}
	}
	/*
	 * Set all possible fields that could differ, then do check
	 * of whole super block against an alternate super block.
	 * When an alternate super-block is specified this check is skipped.
	 */
	if (bflag)
		goto sbok;
	getblk(&asblk, cgsblock(&sblock, sblock.fs_ncg - 1), sblock.fs_sbsize);
	if (asblk.b_errs != NULL)
		return (0);
	altsblock.fs_link = sblock.fs_link;
	altsblock.fs_rlink = sblock.fs_rlink;
	altsblock.fs_time = sblock.fs_time;
	altsblock.fs_cstotal = sblock.fs_cstotal;
	altsblock.fs_cgrotor = sblock.fs_cgrotor;
	altsblock.fs_fmod = sblock.fs_fmod;
	altsblock.fs_clean = sblock.fs_clean;
	altsblock.fs_ronly = sblock.fs_ronly;
	altsblock.fs_flags = sblock.fs_flags;
	altsblock.fs_maxcontig = sblock.fs_maxcontig;
	altsblock.fs_minfree = sblock.fs_minfree;
	altsblock.fs_optim = sblock.fs_optim;
	altsblock.fs_rotdelay = sblock.fs_rotdelay;
	altsblock.fs_maxbpg = sblock.fs_maxbpg;
	altsblock.fs_state = sblock.fs_state;
	bcopy((char *)sblock.fs_csp, (char *)altsblock.fs_csp,
		sizeof sblock.fs_csp);
	bcopy((char *)sblock.fs_fsmnt, (char *)altsblock.fs_fsmnt,
		sizeof sblock.fs_fsmnt);
	if (bcmp((char *)&sblock, (char *)&altsblock, (int)sblock.fs_sbsize))
		{ badsb("TRASHED VALUES IN SUPER BLOCK"); return (0); }
sbok:
	fmax = sblock.fs_size;
	imax = sblock.fs_ncg * sblock.fs_ipg;
	/*
	 * read in the summary info.
	 */
	for (i = 0, j = 0; i < sblock.fs_cssize; i += sblock.fs_bsize, j++) {
		size = sblock.fs_cssize - i < sblock.fs_bsize ?
		    sblock.fs_cssize - i : sblock.fs_bsize;
		sblock.fs_csp[j] = (struct csum *)calloc(1, (unsigned)size);
		if (bread(&dfile, (char *)sblock.fs_csp[j],
		    fsbtodb(&sblock, sblock.fs_csaddr + j * sblock.fs_frag),
		    size) != 0)
			return (0);
	}
	/*
	 * allocate and initialize the necessary maps
	 */
	bmapsz = roundup(howmany(fmax, NBBY), sizeof(short));
	blockmap = calloc((unsigned)bmapsz, sizeof (char));
	if (blockmap == NULL) {
		printf("cannot alloc %d bytes for blockmap\n", bmapsz);
		goto badsb;
	}
	statemap = calloc((unsigned)(imax + 1), sizeof(char));
	if (statemap == NULL) {
		printf("cannot alloc %d bytes for statemap\n", imax + 1);
		goto badsb;
	}
	lncntp = (short *)calloc((unsigned)(imax + 1), sizeof(short));
	if (lncntp == NULL) {
		printf("cannot alloc %d bytes for lncntp\n", 
		    (imax + 1) * sizeof(short));
		goto badsb;
	}

	return (devstr);

badsb:
	ckfini();
	return (0);
#	undef altsblock
}

badsb(s)
	char *s;
{

	if (preen)
		printf("%s: ", devname);
	printf("BAD SUPER BLOCK: %s\n", s);
	pwarn("USE -b OPTION TO FSCK TO SPECIFY LOCATION OF AN ALTERNATE\n");
	pfatal("SUPER-BLOCK TO SUPPLY NEEDED INFORMATION; SEE fsck(1M).\n");
}
