/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ufs.cmds:ufs/ncheck/ncheck.c	1.7.3.1"
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

/*
 * ncheck -- obtain file names from reading filesystem
 */

#define	NB		500
#define	MAXNINDIR	(MAXBSIZE / sizeof (daddr_t))

#include <sys/param.h>
#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>
#include <sys/fs/ufs_fsdir.h>
#include <stdio.h>

union {
	struct	fs	sblk;
	char xxx[SBSIZE];	/* because fs is variable length */
} real_fs;
#define sblock real_fs.sblk

struct	dinode	itab[MAXIPG];
struct 	dinode	*gip;
struct ilist {
	ino_t	ino;
	u_short	mode;
	short	uid;
	short	gid;
} ilist[NB];
struct	htab
{
	ino_t	h_ino;
	ino_t	h_pino;
	char	*h_name;
} *htab;
char *strngtab;
long hsize;
int strngloc;

struct dirstuff {
	int loc;
	struct dinode *ip;
	char dbuf[MAXBSIZE];
};

int	aflg;
int	sflg;
int	iflg; /* number of inodes being searched for */
int	mflg;
int	fi;
ino_t	ino;
int	nhent;
int	nxfile;

int	nerror;
daddr_t	bmap();
long	atol();
long	lseek();
char 	*malloc();
char 	*strcpy();
struct htab *lookup();
struct direct *dreaddir();

extern int	optind;
extern char	*optarg;

char *subopts [] = {
#define M_FLAG		0
	"m",
	NULL
	};

main(argc, argv)
	int argc;
	char *argv[];
{
	long n;
	int	opt;
	char	*suboptions,	*value;
	int	suboption;
	char	*p;
	int	first = 0;

	while ((opt = getopt (argc, argv, "ao:i:s")) != EOF) {
		switch (opt) {

		case 'a':
			aflg++;
			break;

		case 'o':
			/*
			 * ufs specific options.
			 */
			suboptions = optarg;
			while (*suboptions != '\0') {
				switch ((suboption = getsubopt(&suboptions, subopts, &value))) {
		
				case M_FLAG:
					mflg++;
					break;
		
				default:
					usage ();
				}
			}
			break;

		case 'i':
			while ((p = strtok((first++==0?optarg:0),", "))!=NULL) {
				if ((n = atoi(p)) == 0)
					break;
				ilist[iflg].ino = n;
				nxfile = iflg;
				iflg++;
			}
			break;

		case 's':
			sflg++;
			break;
#if 0
		case 'V':
			{
				int	opt_count;
				char	*opt_text;
	
				(void) fprintf (stdout, "ncheck -F ufs ");
				for (opt_count = 1; opt_count < argc ; opt_count++) {
					opt_text = argv[opt_count];
					if (opt_text)
						(void) fprintf (stdout, " %s ", opt_text);
				}
				(void) fprintf (stdout, "\n");
			}
			break;
#endif
		case '?':
			usage ();
		}
	}
	argc -= optind;
	argv = &argv[optind];
	while (argc--) {
		check(*argv);
		argv++;
	}
	return(nerror);
}

check(file)
	char *file;
{
	register int i, j, c;

	fi = open(file, 0);
	if(fi < 0) {
		(void) fprintf(stderr, "ncheck: cannot open %s\n", file);
		nerror++;
		return;
	}
	nhent = 0;
	(void) printf("%s:\n", file);
	sync();
	bread(SBLOCK, (char *)&sblock, SBSIZE);
	if (sblock.fs_magic != FS_MAGIC) {
		(void) printf("%s: not a ufs file system\n", file);
		nerror++;
		return;
	}
	hsize = sblock.fs_ipg * sblock.fs_ncg - sblock.fs_cstotal.cs_nifree + 1;
	htab = (struct htab *)malloc((unsigned)(hsize * sizeof(struct htab)));
	strngtab = (char *)malloc((unsigned)(30 * hsize));
	if (htab == 0 || strngtab == 0) {
		(void) printf("not enough memory to allocate tables\n");
		nerror++;
		return;
	}
	ino = 0;
	for (c = 0; c < sblock.fs_ncg; c++) {
		bread(fsbtodb(&sblock, cgimin(&sblock, c)), (char *)itab,
		    (int)(sblock.fs_ipg * sizeof (struct dinode)));
		for(j = 0; j < sblock.fs_ipg; j++) {
			if (itab[j].di_smode != 0) {
				if (itab[j].di_eftflag != EFT_MAGIC){
					itab[j].di_mode = itab[j].di_smode;
					itab[j].di_uid = itab[j].di_suid;
					itab[j].di_gid = itab[j].di_sgid;
				}
				pass1(&itab[j]);
			}
			ino++;
		}
	}
	ilist[nxfile+1].ino = 0;
	ino = 0;
	for (c = 0; c < sblock.fs_ncg; c++) {
		bread(fsbtodb(&sblock, cgimin(&sblock, c)), (char *)itab,
		    (int)(sblock.fs_ipg * sizeof (struct dinode)));
		for(j = 0; j < sblock.fs_ipg; j++) {
			if (itab[j].di_smode != 0) {
				if (itab[j].di_eftflag != EFT_MAGIC)
					itab[j].di_mode = itab[j].di_smode;
				pass2(&itab[j]);
			}
			ino++;
		}
	}
	ino = 0;
	for (c = 0; c < sblock.fs_ncg; c++) {
		bread(fsbtodb(&sblock, cgimin(&sblock, c)), (char *)itab,
		    (int)(sblock.fs_ipg * sizeof (struct dinode)));
		for(j = 0; j < sblock.fs_ipg; j++) {
			if (itab[j].di_mode != 0) {
				if (itab[j].di_eftflag != EFT_MAGIC)
					itab[j].di_mode = itab[j].di_smode;
				pass3(&itab[j]);
			}
			ino++;
		}
	}
	(void) close(fi);
	for (i = 0; i < hsize; i++)
		htab[i].h_ino = 0;
	for (i = iflg; i < NB; i++)
		ilist[i].ino = 0;
	nxfile = iflg;
}

pass1(ip)
	register struct dinode *ip;
{
	int i;

	if (mflg)
		for (i = 0; i < iflg; i++)
			if (ino == ilist[i].ino) {
				ilist[i].mode = ip->di_mode;
				ilist[i].uid = ip->di_uid;
				ilist[i].gid = ip->di_gid;
			}
	if ((ip->di_mode & IFMT) != IFDIR) {
		if (sflg==0 || nxfile>=NB)
			return;
		if ((ip->di_mode&IFMT)==IFBLK || (ip->di_mode&IFMT)==IFCHR
		  || ip->di_mode&(ISUID|ISGID)) {
			ilist[nxfile].ino = ino;
			ilist[nxfile].mode = ip->di_mode;
			ilist[nxfile].uid = ip->di_uid;
			ilist[nxfile++].gid = ip->di_gid;
			return;
		}
	}
	(void) lookup(ino, 1);
}

pass2(ip)
	register struct dinode *ip;
{
	register struct direct *dp;
	struct dirstuff dirp;
	struct htab *hp;

	if((ip->di_mode&IFMT) != IFDIR)
		return;
	dirp.loc = 0;
	dirp.ip = ip;
	gip = ip;
	for (dp = dreaddir(&dirp); dp != NULL; dp = dreaddir(&dirp)) {
		if(dp->d_ino == 0)
			continue;
		hp = lookup(dp->d_ino, 0);
		if(hp == 0)
			continue;
		if(dotname(dp))
			continue;
		hp->h_pino = ino;
		hp->h_name = &strngtab[strngloc];
		strngloc += strlen(dp->d_name) + 1;
		(void) strcpy(hp->h_name, dp->d_name);
	}
}

pass3(ip)
	register struct dinode *ip;
{
	register struct direct *dp;
	struct dirstuff dirp;
	int k;

	if((ip->di_mode&IFMT) != IFDIR)
		return;
	dirp.loc = 0;
	dirp.ip = ip;
	gip = ip;
	for(dp = dreaddir(&dirp); dp != NULL; dp = dreaddir(&dirp)) {
		if(aflg==0 && dotname(dp))
			continue;
		if(sflg == 0 && iflg == 0)
			goto pr;
		for(k = 0; ilist[k].ino != 0; k++)
			if(ilist[k].ino == dp->d_ino)
				break;
		if (ilist[k].ino == 0)
			continue;
		if (mflg)
			(void) printf("mode %-6o uid %-5d gid %-5d ino ",
			    ilist[k].mode, ilist[k].uid, ilist[k].gid);
	pr:
		(void) printf("%-5u\t", dp->d_ino);
		pname(ino, 0);
		(void) printf("/%s", dp->d_name);
		if (lookup(dp->d_ino, 0))
			(void) printf("/.");
		(void) printf("\n");
	}
}

/*
 * get next entry in a directory.
 */
struct direct *
dreaddir(dirp)
	register struct dirstuff *dirp;
{
	register struct direct *dp;
	daddr_t lbn, d;

	for(;;) {
		if (dirp->loc >= dirp->ip->di_size)
			return NULL;
		if (blkoff(&sblock, dirp->loc) == 0) {
			lbn = lblkno(&sblock, dirp->loc);
			d = bmap(lbn);
			if(d == 0)
				return NULL;
			bread(fsbtodb(&sblock, d), dirp->dbuf,
			    (int)dblksize(&sblock, dirp->ip, (int)lbn));
		}
		dp = (struct direct *)
		    (dirp->dbuf + blkoff(&sblock, dirp->loc));
		dirp->loc += dp->d_reclen;
		if (dp->d_ino == 0)
			continue;
		return (dp);
	}
}

dotname(dp)
	register struct direct *dp;
{

	if (dp->d_name[0]=='.')
		if (dp->d_name[1]==0 ||
		   (dp->d_name[1]=='.' && dp->d_name[2]==0))
			return(1);
	return(0);
}

pname(i, lev)
	ino_t i;
	int lev;
{
	register struct htab *hp;

	if (i==UFSROOTINO)
		return;
	if ((hp = lookup(i, 0)) == 0) {
		(void) printf("???");
		return;
	}
	if (lev > 10) {
		(void) printf("...");
		return;
	}
	pname(hp->h_pino, ++lev);
	(void) printf("/%s", hp->h_name);
}

struct htab *
lookup(i, ef)
	ino_t i;
	int ef;
{
	register struct htab *hp;

	for (hp = &htab[(int)i%hsize]; hp->h_ino;) {
		if (hp->h_ino==i)
			return(hp);
		if (++hp >= &htab[hsize])
			hp = htab;
	}
	if (ef==0)
		return(0);
	if (++nhent >= hsize) {
		(void) fprintf(stderr, "ncheck: hsize of %d is too small\n", 
									hsize);
		exit(32);
	}
	hp->h_ino = i;
	return(hp);
}

bread(bno, buf, cnt)
	daddr_t bno;
	char *buf;
	int cnt;
{
	register i;
	int got;

	if (lseek(fi, (long)(bno * DEV_BSIZE), 0) == (long) -1) {
		(void) fprintf(stderr, "ncheck: lseek error %d\n", 
							bno * DEV_BSIZE);
		for(i=0; i < cnt; i++)
			buf[i] = 0;
		return;
	}

	got = read((int)fi, buf, cnt);
	if (got != cnt) {
		(void) fprintf(stderr, 
			"ncheck: read error %d (wanted %d got %d\n", cnt, got, 
									bno);
		for(i=0; i < cnt; i++)
			buf[i] = 0;
	}
}

daddr_t
bmap(i)
	daddr_t i;
{
	daddr_t ibuf[MAXNINDIR];

	if(i < NDADDR)
		return(gip->di_db[i]);
	i -= NDADDR;
	if(i > NINDIR(&sblock)) {
		(void) fprintf(stderr, "ncheck: %u - huge directory\n", ino);
		return((daddr_t)0);
	}
	bread(fsbtodb(&sblock, gip->di_ib[0]), (char *)ibuf, sizeof(ibuf));
	return(ibuf[i]);
}

usage ()
{
	(void) fprintf (stderr, "ufs usage: ncheck [-F ufs] [generic options] [-a -i #list -s] [-o m] special\n");
	exit (32);
}
