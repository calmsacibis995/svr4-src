/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ufs.cmds:ufs/ff/ff.c	1.9.3.2"

/*
 * ff -- obtain file names from reading filesystem
 */

#define	NB		500
#define	MAXNINDIR	(MAXBSIZE / sizeof (daddr_t))

#include <sys/param.h>
#include <sys/types.h>
#include <sys/mntent.h>
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#include <sys/stat.h>
#include <sys/fs/ufs_fs.h>
#include <sys/fs/ufs_fsdir.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>

#define	MIN_PHYS_READ	BBSIZE
#define	DAY		(24*60*60)


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
int	Aflg = 0;	/* accessed in n days */
int	Mflg = 0;	/* modified in n days */
int	Nflg = 0;	/* modified more recently than 'file' */
int	Cflg = 0;	/* changed within n days */
int	aflg = 0;	/* print the names `.'  and  `..' */
int	sflg = 0;	/* print only special files and files with set-user-ID mode */
int	Sflg = 0;	/* print file size */
int	iflg = 0;	/* number of inodes being searched for */
int	Iflg = 0;	/* do not print i-number */
int	Lflg = 0;	/* supplementary list of multiply linked files */
int	mflg = 0;
int	pflg = 0;	/* a prefix exists */
int	uflg = 0;	/* print the owner's login name */
int	fi;
ino_t	ino;
int	nhent;
int	nxfile;
int	imax;		/* highest inode number */
int	inode_reads;
int	passwd_lookups;
int	Adelay;		/* Access delay */
int	Asign;		/* Access sign */
int	Mdelay;		/* Modify delay */
int	Msign;		/* Modify sign */
int	Cdelay;		/* change delay */
int	Csign;		/* change sign */
time_t	Nage;		/* Last modification time of the file */
char	*Lname;		/* filename for supplementary list */
FILE	*Lfile;		/* file for supplementary list */

int	nerror = 0;
daddr_t	bmap();
long	atol();
long	lseek();
void 	*malloc();
char 	*strcpy();
struct htab *lookup();
struct direct *dreaddir();
char	*prefix;
time_t	Today;

extern int	optind;
extern char	*optarg;

char *subopts [] = {
#define A_FLAG		0
	"a",
#define M_FLAG		1
	"m",
#define S_FLAG		2
	"s",
	NULL
	};

void check(), pass1(), pass2(), pass3(), pname(), bread();

main(argc, argv)
	int argc;
	char *argv[];
{
	long n;
	int	opt;
	char	*suboptions,	*value;
	time_t	mod_time();
	char *p;
	int first= 0;

	Today = time((time_t *)0);
	while ((opt = getopt (argc, argv, "Ia:c:i:lm:n:o:p:su")) != EOF) {
		switch (opt) {

		case 'a':
			Aflg++;
			Adelay = atoi(optarg);
			Asign = optarg[0];
			break;

		case 'I':
			Iflg++;
			break;

		case 'c':
			Cflg++;
			Cdelay = atoi(optarg);
			Csign = optarg[0];
			break;

		case 'l':
			Lflg++;
			Lname = tmpnam ((char *)0);
			if ((Lfile = fopen (Lname, "w+")) == NULL) {
				perror ("open");
				(void) fprintf (stderr,
					"ff: unable to open temp file, -l ignored\n");
				Lflg = 0;
			}
			break;

		case 'm':
			Mflg++;
			Mdelay = atoi(optarg);
			Msign = optarg[0];
			break;

		case 'n':
			Nflg++;
			Nage = mod_time(optarg);
			break;

		case 'o':
			/*
			 * ufs specific options.
			 */
			suboptions = optarg;

			while (*suboptions != '\0') {
				switch ((getsubopt(&suboptions, subopts, &value))) {
		
				case A_FLAG:
					aflg++;
					break;

				case M_FLAG:
					mflg++;
					break;

				case S_FLAG:
					sflg++;
					break;
		
				default:
					usage ();
				}
			}
			break;

		case 'i':
			while ((p=strtok((first++==0?optarg:0),", ")) != NULL) {	
				
				if((n= atoi(p)) == 0)
					break;
				ilist[iflg].ino = n;
				nxfile = iflg;
				iflg++;
			}
			break;

		case 'p':
			prefix = optarg;
			pflg++;
			break;

		case 's':
			Sflg++;
			break;

		case 'u':
			uflg++;
			break;

		case '?':
			usage ();
		}
	}
	argc -= optind;
	argv = &argv[optind];
	while (argc--) {
		(void)check(*argv);
		argv++;
	}
	if (Lflg) {
		out_multilinks ();
	}
	if (nerror)
		return(32);
	else
		return (0);
}

void
check(file)
	char *file;
{
	register int i, j, c;

	fi = open(file, 0);
	if(fi < 0) {
		(void) fprintf(stderr, "ff: cannot open %s\n", file);
		nerror++;
		return;
	}
	nhent = 0;
	(void) printf("%s:\n", file);
	sync();
	(void)bread(SBLOCK, (char *)&sblock, SBSIZE);
	if (sblock.fs_magic != FS_MAGIC) {
		(void) fprintf(stderr, "%s: not a ufs file system\n", file);
		nerror++;
		return;
	}
	imax = sblock.fs_ncg * sblock.fs_ipg;
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
		(void)bread(fsbtodb(&sblock, cgimin(&sblock, c)), (char *)itab,
		    (int)(sblock.fs_ipg * sizeof (struct dinode)));
		for(j = 0; j < sblock.fs_ipg; j++) {
			if (itab[j].di_smode != 0) {
				if (itab[j].di_eftflag != EFT_MAGIC) {
					itab[j].di_mode = itab[j].di_smode;
					itab[j].di_uid = itab[j].di_suid;
					itab[j].di_gid = itab[j].di_sgid;
				}
				(void)pass1(&itab[j]);
			}
			ino++;
		}
	}
	ilist[nxfile+1].ino = 0;
	ino = 0;
	for (c = 0; c < sblock.fs_ncg; c++) {
		(void)bread(fsbtodb(&sblock, cgimin(&sblock, c)), (char *)itab,
		    (int)(sblock.fs_ipg * sizeof (struct dinode)));
		for(j = 0; j < sblock.fs_ipg; j++) {
			if (itab[j].di_smode != 0) {
				if (itab[j].di_eftflag != EFT_MAGIC)
					itab[j].di_mode = itab[j].di_smode;
				(void)pass2(&itab[j]);
			}
			ino++;
		}
	}
	ino = 0;
	for (c = 0; c < sblock.fs_ncg; c++) {
		(void)bread(fsbtodb(&sblock, cgimin(&sblock, c)), (char *)itab,
		    (int)(sblock.fs_ipg * sizeof (struct dinode)));
		for(j = 0; j < sblock.fs_ipg; j++) {
			if (itab[j].di_smode != 0) {
				if (itab[j].di_eftflag != EFT_MAGIC)
					itab[j].di_mode = itab[j].di_smode;
				(void)pass3(&itab[j]);
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

void
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

void
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

void
pass3(ip)
	register struct dinode *ip;
{
	register struct direct *dp;
	struct dirstuff dirp;
	struct dinode   *dip;
	int k;
	struct dinode	*ginode();
	char		*user_name();

	if((ip->di_mode&IFMT) != IFDIR)
		return;
	dirp.loc = 0;
	dirp.ip = ip;
	gip = ip;
	for(dp = dreaddir(&dirp); dp != NULL; dp = dreaddir(&dirp)) {
		if(aflg==0 && dotname(dp))
			continue;
		if (sflg == 0 && iflg == 0)
			goto pr;
		for (k = 0; ilist[k].ino != 0; k++)
			if (ilist[k].ino == dp->d_ino)
				break;
		if (ilist[k].ino == 0)
			continue;
		if (mflg)
			(void) printf("mode %-6o uid %-5d gid %-5d ino ",
			    ilist[k].mode, ilist[k].uid, ilist[k].gid);
	pr:
		if (Sflg || uflg || Aflg || Mflg || Cflg || Nflg || Lflg)
			dip = ginode(dp->d_ino);
		if ((!Aflg || cmp((Today - dip->di_un.di_icom.ic_atime)/DAY, (long)Adelay, Asign)) &&
		    (!Mflg || cmp((Today - dip->di_un.di_icom.ic_mtime)/DAY, (long)Mdelay, Msign)) &&
			(!Cflg || cmp((Today - dip->di_un.di_icom.ic_mtime)/DAY, (long)Cdelay, Csign)) &&
			    (!Nflg || cmp(dip->di_un.di_icom.ic_mtime, Nage, '+'))) {
			if (pflg)
				(void) fprintf(stdout, "%s", prefix);
			(void)pname(stdout, ino, 0);
			(void) printf("/%s", dp->d_name);
			if (lookup(dp->d_ino, 0))
				(void) printf("/.");
			if (Iflg == 0)
				(void) printf("\t%u", dp->d_ino);
			if (Sflg)
				(void) printf("\t%6d", dip->di_un.di_icom.di_size);
			if (uflg)
				(void) printf("\t%s", user_name(dip->di_un.di_icom.ic_uid));
			(void) printf("\n");
			if (Lflg && (dip->di_un.di_icom.ic_nlink > 1)) {
				(void) fprintf(Lfile, "%-5u\t",
					dp->d_ino);
				(void) fprintf(Lfile, "%-5u\t",
					dip->di_un.di_icom.ic_nlink);
				if (pflg)
					(void) fprintf(Lfile, "%s", prefix);
				(void)pname(Lfile, ino, 0);
				(void) fprintf(Lfile, "/%s\n", dp->d_name);
			}
		}
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
			(void)bread(fsbtodb(&sblock, d), dirp->dbuf,
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

void
pname(stream, i, lev)
	FILE	*stream;
	ino_t i;
	int lev;
{
	register struct htab *hp;

	if (i==UFSROOTINO)
		return;
	if ((hp = lookup(i, 0)) == 0) {
		(void) fprintf(stream, "???");
		return;
	}
	if (lev > 10) {
		(void) fprintf(stream, "...");
		return;
	}
	(void)pname(stream, hp->h_pino, ++lev);
	(void) fprintf(stream, "/%s", hp->h_name);
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
		(void) fprintf(stderr, "ff: hsize of %d is too small\n", 
									hsize);
		exit(32);
	}
	hp->h_ino = i;
	return(hp);
}

void
bread(bno, buf, cnt)
	daddr_t bno;
	char *buf;
	int cnt;
{
	register i;
	int got;

	if (lseek(fi, (long)(bno * DEV_BSIZE), 0) == (long) -1) {
		(void) fprintf(stderr, "ff: lseek error %d\n", 
							bno * DEV_BSIZE);
		for(i=0; i < cnt; i++)
			buf[i] = 0;
		return;
	}

	got = read((int)fi, buf, cnt);
	if (got != cnt) {
		perror ("read");
		(void) fprintf(stderr, 
			"ff: (wanted %d got %d blk %d)\n",
			cnt, got, bno);
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
		(void) fprintf(stderr, "ff    : %u - huge directory\n", ino);
		return((daddr_t)0);
	}
	(void)bread(fsbtodb(&sblock, gip->di_ib[0]), (char *)ibuf, sizeof(ibuf));
	return(ibuf[i]);
}

struct dinode	*
ginode(inumber)
	ino_t	inumber;
{
	daddr_t		iblk;
	daddr_t		dblk;
	int		ioff;
	static daddr_t	curr_dblk;
	static char	buf[MIN_PHYS_READ];
	struct dinode	*ibuf;

	if (inumber < UFSROOTINO || (int)inumber > imax) {
		(void) fprintf (stderr, "bad inode number %d to ginode\n", inumber);
		exit (32);
	}
	iblk = itod(&sblock, (int)inumber);
	dblk = fsbtodb(&sblock, iblk);
	ioff = itoo(&sblock, (int)inumber);
	if (dblk != curr_dblk) {
		(void)bread(dblk, &buf[0], sizeof(buf));
		curr_dblk = dblk;
		inode_reads++;
	}
	ibuf = (struct dinode *)&buf[0];
	ibuf += ioff;
	return (ibuf);
}

#define HASHNAMESIZE 16

struct name_ent {
	struct name_ent	*name_nxt;
	int		name_uid;
	char		*name_string;
};
struct name_ent *hashtable[HASHNAMESIZE];

char *
user_name (uid)
{
	int		h_index;
	struct name_ent	*hp;
	struct passwd	*pwent;

	h_index = uid % HASHNAMESIZE;
	for (hp = hashtable[h_index]; hp != NULL; hp = hp->name_nxt) {
		if (hp->name_uid == uid) {
			return (hp->name_string);
		}
	}
	hp = (struct name_ent *) calloc (1, sizeof (struct name_ent));
	hp->name_nxt = hashtable[h_index];
	hp->name_uid = uid;
	hashtable[h_index] = hp;
	if ((pwent = getpwuid(uid)) == NULL) {
		hp->name_string = "unknown";
	} else {
		hp->name_string = (char *) strdup (pwent->pw_name);
	}
	passwd_lookups++;

return (hp->name_string);
}

cmp (a, b, s)
	register long a, b;
	register	s;
{
	if(s == '+')
		return(a > b);
	if(s == '-')
		return(a < -(b));
	return(a == b);
}

/*
 * We can't do this one by reading the disk directly, since there
 * is no guarantee that the file is even on a local disk.
 */
time_t
mod_time (file)
	char	*file;
{
	struct stat	stat_buf;

	if (stat(file, &stat_buf) < 0) {
		(void) fprintf (stderr, "ff: can't stat '%s' - ignored\n", file);
		return (0);
	}
	return (stat_buf.st_mtime);
}

out_multilinks ()
{
	int	length;

	if ((length = fseek(Lfile, 0L, 2)) < 0) {
		perror ("fseek");
		exit (32);
	} else
		if ((length = ftell(Lfile)) > 0) {
			(void) fprintf (stdout, "\nmultilink files\nIno\tLinks\tPathname\n\n");
			rewind(Lfile);
			while (length-- > 0)
				putc (getc (Lfile), stdout);
		} else
			(void) fprintf (stdout, "No multilink files\n");
	fclose (Lfile);
}

usage ()
{
	(void) fprintf (stderr, "ufs usage: ff [-F ufs] [generic options] [-o a,m,s] special\n");
	exit (32);
}
