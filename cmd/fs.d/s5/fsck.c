/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)s5.cmds:fsck.c	1.22.6.2"

/*
 *  This is the System V specific fsck for
 *  512, 1024 and 2048 block file systems.
 *
 *  Note that ONLY ONE file system can be checked at a time
 *  due to all the error exits now implemented.
 *  Most notably, -m (sanity check) exits with the needed exit code.
 *
 *  For backwards compatibility,
 *  more than one device may be specified
 *  on the command line, and they may be of different block sizes.
 *  Checking more than one file system is being discouraged because it
 *  restricts them all to the same fstype,
 *  and will terminate at the first severe error.
 *
 *  If multiple file systems need checking, the generic fsck
 *  should be used to execute the specific fscks needed.
 *
 *  There is now one executable for all 3 System V block sizes.
 *  -m was added (eliminates need for fsstat)
 */

/*
 *  Wed Jan 13 1988
 *  FUTURE DIRECTIONS:  under SVR4, the ustat() call will still exist,
 *  but when /tbl is implemented, a better mechanism for testing
 *  if a device is mounted will be implemented, replacing the need for ustat().
 *  Note that ustat() is used only to test if a device is mounted.
 *  The contents of the status returned is not used in fsck.
 */

/*  FsTYPE must be set  */
#if !defined (FsTYPE)
/* #    error "FsTYPE_is_not_set"     USE IN ANSI-COMPILERS! */
#include "FsTYPE_is_not_set"
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/fcntl.h>	/*  defines O_RDONLY, O_WRONLY  */
#ifdef STANDALONE
#include <stand.h>
#else
#include <sys/param.h>
#include <sys/types.h>
#include <sys/fs/s5param.h>
#endif
#include <locale.h>	

/*
 *  At this time,
 *  this is compiled with Fstype set to 4, meaning a 2k file system.
 *  If a larger block size is used, be sure to compile this with the
 *  largest block size and set the following variable accordingly.
 */


/*
 *  The following parameters are set according to the block size
 *  of the file system.  They are found in sys/fs/s5param.h,
 *  but depending on the FsTYPE preprocessor variable.
 *  In order to have one executable for all SV file systems
 *  regardless of the FsTYPE varaible, the main() program is
 *  compiled with the highest number so as to allocate the
 *  biggest buffer possible.
 *  The initialization routines for the smaller block sizes are
 *  compiled separately with FsTYPE set appropriately and linked
 *  with this to produce one executable.
 *
 *  After the superblock is read, the variables are adjusted if needed.
 *
 *  The variables F_xxx are used in place of their the xxx constants,
 *  except:
 *	BSIZE is needed as a preprocessor constant for declaring the size of:
 *		bufarea.b_buf[BSIZE];		/* buffer space
 *	INOPB is needed as a preprocessor constant for declaring the size of:
 *		bufarea.b_dinode[INOPB];	/* inode block
 *	NINDIR is needed as a preprocessor constant for declaring the size of:
 *		b_indir[NINDIR];		/* indirect block
 *
 *  The macros itod and itoo from <sys/sysmacros.h> are redefined
 *  in terms of the variables.
 */


#define	BSIZE  2048
#define	INOPB  32
#define	NINDIR  (BSIZE/ sizeof(daddr_t))
#define BSHIFT 11
#define BMASK  03777
#define INOSHIFT 5

int	F_BSIZE = BSIZE;
int	F_BSHIFT = BSHIFT;
int	F_BMASK = BMASK;
int	F_INOPB = INOPB;
int	F_NINDIR = NINDIR;
int	F_INOSHIFT = INOSHIFT;
int	F_NUMTRIPLE = 3;

#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*F_INOPB-1))>>F_INOSHIFT)
#define	itoo(x)	(int)(((unsigned)(x)+(2*F_INOPB-1))&(F_INOPB-1))



#include <signal.h>
#if u3b2 || u3b15 || i386
#include <sys/uadmin.h>
#endif
#include <sys/vnode.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5filsys.h>
#include <sys/fs/s5dir.h>
#include <sys/fs/s5fblk.h>
#include <sys/fs/s5ino.h>
#include <sys/stat.h>
#include <sys/ustat.h>

#if u3b2 || u3b15
#include <sys/sys3b.h>
#endif

#define	SBSIZE	512

#define NDIRECT	(F_BSIZE/sizeof(struct direct))
#define DECL_NDIRECT	(BSIZE/sizeof(struct direct))
#define SPERB	(F_BSIZE/sizeof(short))
#define DECL_SPERB	(BSIZE/sizeof(short))

#ifndef i386
#define PHYSBLKSZ 512
#endif

#define NO	0
#define YES	1

#define	MAXDUP	10		/* limit on dup blks (per inode) */
#define	MAXBAD	10		/* limit on bad blks (per inode) */

#define STEPSIZE	7	/* default step for freelist spacing */
#define CYLSIZE		400	/* default cyl size for spacing */
#define MAXCYL		1000	/* maximum cylinder size */

#define BITSPB	8		/* number bits per byte */
#define BITSHIFT	3	/* log2(BITSPB) */
#define BITMASK	07		/* BITSPB-1 */
#define LSTATE	2		/* bits per inode state */
#define STATEPB	(BITSPB/LSTATE)	/* inode states per byte */
#define USTATE	0		/* inode not allocated */
#define FSTATE	01		/* inode is file */
#define DSTATE	02		/* inode is directory */
#define CLEAR	03		/* inode is to be cleared */
#define EMPT	32		/* empty directory? */
#define SMASK	03		/* mask for inode state */

typedef struct dinode	DINODE;
typedef struct direct	DIRECT;

#define ALLOC	((dp->di_mode & S_IFMT) != 0)
#define DIR	((dp->di_mode & S_IFMT) == S_IFDIR)
#define REG	((dp->di_mode & S_IFMT) == S_IFREG)
#define BLK	((dp->di_mode & S_IFMT) == S_IFBLK)
#define CHR	((dp->di_mode & S_IFMT) == S_IFCHR)
#define FIFO	((dp->di_mode & S_IFMT) == S_IFIFO)
#define FLNK	((dp->di_mode & S_IFMT) == S_IFLNK)
#define FNAM	((dp->di_mode & S_IFMT) == S_IFNAM)
#define SPECIAL (BLK || CHR || FNAM)
#define ftypeok(dp)	(REG||DIR||BLK||CHR||FIFO||FLNK || FNAM)

#define NINOBLK	11		/* num blks for raw reading */
#define MAXRAW	110		/* largest raw read (in blks) */
daddr_t	startib;		/* blk num of first in raw area */
unsigned niblk;			/* num of blks in raw area */

struct bufarea {
	struct bufarea	*b_next;		/* must be first */
	daddr_t	b_bno;
	union {
		char	b_buf[BSIZE];		/* buffer space */
		short	b_lnks[DECL_SPERB];	/* link counts */
		daddr_t	b_indir[NINDIR];	/* indirect block */
		struct filsys b_fs;		/* super block */
		struct fblk b_fb;		/* free block */
		struct dinode b_dinode[INOPB];	/* inode block */
		DIRECT b_dir[DECL_NDIRECT];	/* directory */
	} b_un;
	char	b_dirty;
};

typedef struct bufarea BUFAREA;


BUFAREA	inoblk;			/* inode blocks */
BUFAREA	fileblk;		/* other blks in filesys */
BUFAREA	sblk;			/* file system superblock */
BUFAREA	*poolhead;		/* ptr to first buffer in pool */

#define initbarea(x)	(x)->b_dirty = 0;(x)->b_bno = (daddr_t)-1
#define dirty(x)	(x)->b_dirty = 1
#define inodirty()	inoblk.b_dirty = 1
#define fbdirty()	fileblk.b_dirty = 1
#define sbdirty()	sblk.b_dirty = 1

#define freeblk		fileblk.b_un.b_fb
#define dirblk		fileblk.b_un.b_dir
#define superblk	sblk.b_un.b_fs

struct filecntl {
	int	rfdes;
	int	wfdes;
	int	mod;
};

struct filecntl	dfile;		/* file descriptors for filesys */
struct filecntl	sfile;		/* file descriptors for scratch file */

typedef unsigned MEMSIZE;

MEMSIZE	memsize;		/* amt of memory we got */

#ifdef i386
extern void *end;
#define MAXDATA (((MEMSIZE)350*1024) + (long) &end)
#endif

#if u3b15
#define MAXDATA (((MEMSIZE)120*2048) + (MEMSIZE)sbrk(0))
#endif
#if u3b2
#define MAXDATA (((MEMSIZE)64*2048) + (MEMSIZE)sbrk(0))
#endif
#ifdef vax
#ifdef STANDALONE
#define	MAXDATA ((MEMSIZE)256*1024)
#else
#define	MAXDATA ((MEMSIZE)350*1024)
#endif
#endif

#define	DUPTBLSIZE	100	/* num of dup blocks to remember */
daddr_t	duplist[DUPTBLSIZE];	/* dup block table */
daddr_t	*enddup;		/* next entry in dup table */
daddr_t	*muldup;		/* multiple dups part of table */

#define MAXLNCNT	100	/* num zero link cnts to remember */
ino_t	badlncnt[MAXLNCNT];	/* table of inos with zero link cnts */
ino_t	*badlnp;		/* next entry in table */

char 	preen;			/* check innocuous inconsistencies */
char	sflag;			/* salvage free block list */
char	csflag;			/* salvage free block list (conditional) */
char	nflag;			/* assume a no response */
char	yflag;			/* assume a yes response */
char	tflag;			/* scratch file specified */
char	rplyflag;		/* any questions asked? */
char	qflag;			/* less verbose flag */
char	Dirc;			/* extensive directory check */
char	fast;			/* fast check- dup blks and free list check */
char	hotroot;		/* checking root device */
char	fixstate;		/* is FsSTATE to be fixed? */
char	sanity_only = 0;	/* set for -m (sanity check)  */
char	rawflg;			/* read raw device */
char	rebflg;			/* needs reboot if set, remount okay if clear */
char	rmscr;			/* remove scratch file when done */
char	fixfree;		/* corrupted free list */
char	*membase;		/* base of memory we get */
char	*blkmap;		/* ptr to primary blk allocation map */
char	*freemap;		/* ptr to secondary blk allocation map */
char	*statemap;		/* ptr to inode state table */
int	pathp;			/* index to pathname position */
int	thisname;		/* index to current pathname component */
int	maxpath;		/* current size of pathname */
char	*srchname;		/* name being searched for in dir */
char	pss2done;			/* do not check dir blks anymore */
char	initdone;
char	*pathname;
char	scrfile[MAXPATHLEN];
char	devname[MAXPATHLEN];
char	*lfname =	"lost+found";
char    *use = "s5 Usage:\nfsck [-F s5] [generic_options] [special ...]\nfsck [-F s5] [generic_options] [-y] [-n] [-p] [-sX] [-SX] [-t file] [-l] [-q] [-D] [-f] [special ...]\n";

short	*lncntp;		/* ptr to link count table */

int     err = 0;		/* flag if an error occurred   */
int	cylsize;		/* num blocks per cylinder */
int	stepsize;		/* num blocks for spacing purposes */
int	badblk;			/* num of bad blks seen (per inode) */
int	dupblk;			/* num of dup blks seen (per inode) */
int	(*pfunc)();		/* function to call to chk blk */

ino_t	inum;			/* inode we are currently working on */
ino_t	imax;			/* number of inodes */
ino_t	parentdir;		/* i number of parent directory */
ino_t	lastino;		/* hiwater mark of inodes */
ino_t	lfdir;			/* lost & found directory */
ino_t	orphan;			/* orphaned inode */

off_t	filsize;		/* num blks seen in file */
off_t	bmapsz;			/* num chars in blkmap */

daddr_t	smapblk;		/* starting blk of state map */
daddr_t	lncntblk;		/* starting blk of link cnt table */
daddr_t	fmapblk;		/* starting blk of free map */
daddr_t	n_free;			/* number of free blocks */
daddr_t	n_blks;			/* number of blocks used */
daddr_t	n_files;		/* number of files seen */
daddr_t	fmin;			/* block number of the first data block */
daddr_t	fmax;			/* number of blocks in the volume */

#define minsz(x,y)	(x>y ? y : x)
#define howmany(x,y)	(((x)+((y)-1))/(y))
#define roundup(x,y)	((((x)+((y)-1))/(y))*(y))
#define outrange(x)	(x < fmin || x >= fmax)
#define zapino(x)	clear((x),sizeof(DINODE))

#define setlncnt(x)	dolncnt(x,0)
#define getlncnt()	dolncnt(0,1)
#define declncnt()	dolncnt(0,2)

#define setbmap(x)	domap(x,0)
#define getbmap(x)	domap(x,1)
#define clrbmap(x)	domap(x,2)

#define setfmap(x)	domap(x,0+4)
#define getfmap(x)	domap(x,1+4)
#define clrfmap(x)	domap(x,2+4)

#define setstate(x)	dostate(x,0)
#define getstate()	dostate(0,1)

#define DATA	1
#define ADDR	0
#define BBLK	2
#define ALTERD	010
#define KEEPON	04
#define SKIP	02
#define STOP	01
#define REM	07

/*
 *  These macros are used in size calculations
 */
#define MAX_FILE 0x7fffffff
#define S1 10 * F_BSIZE
#define S2 F_NINDIR * F_BSIZE
#define S3 F_NINDIR * F_NINDIR * F_BSIZE
#define S4 MAX_FILE - ( S3 + S2 + S1 )

DINODE	*ginode();
BUFAREA	*getblk();
BUFAREA	*search();
int	dirscan();
int	chkblk();
int	findino();
void	catch();
int	mkentry();
int	chgdd();
int	pass1();
int	pass1b();
int	pass2();
int	pass4();
int	pass5();

#ifdef i386
long	lseek();
#endif

#if u3b15 || u3b2 || i386

#ifndef i386
int	s5bsize();
#endif

int	prmptbsize();
#endif
char	id = ' ';
dev_t	pipedev = -1;	/* is pipedev (and != -1) iff the standard input
			 * is a pipe, which means we can't check pipedev! */
void (*signal())();
int 	fcorr_flg;	/* indicates l option ; create list */
struct patherr {	/* inode str in linked list */
	int inode;	/* of possibly damaged pathnames */
	char *pname;
	struct	patherr *next_ptr;
} patherr;
struct patherr *root_ptr;	/* linked list pointers */
struct patherr	*cur_ptr;

main(argc,argv)
int	argc;
char	*argv[];
{
	struct stat statbuf;
	extern	char *optarg;
	extern int optind;
	int	c;

	/*  this is for multiple fsck */
	(void)setlocale(LC_ALL, "");
	if ( argv[0][0] >= '0' && argv[0][0] <= '9' ) id = argv[0][0];

#ifdef STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv ("fsck", &argv, 0);
#else
	setbuf(stdin,NULL);
	setbuf(stdout,NULL);
	sync();
#endif
	maxpath = 512;
	if ((pathname = (char *)malloc(maxpath)) == NULL) {
		printf("Could not allocate space for pathname\n");
		exit(31+1);
	}

	while ((c=getopt (argc, argv, "t:T:s;S;?nNyYbBqlDfxmp")) != -1)
		{
		switch(c)
		  {
		  case 'l':
				fcorr_flg++;
				break;
	     	  case 'p':
				preen++;
				break;
		  case 't':
		  case 'T':
				tflag++;
				if(*optarg == '-')
					errexit("%c Bad -t option\n",id);
				strcpy(scrfile,optarg);
				if( (stat(scrfile,&statbuf) >= 0) &&
					((statbuf.st_mode & S_IFMT) != S_IFREG) )
					errexit("%c Illegal scratch file <%s>\n",
						id,scrfile);
				break;
		  case 's':	/* salvage flag */
				stype(optarg);
				sflag++;
				break;
		  case 'S':	/* conditional salvage */
				stype(optarg);
				csflag++;
				break;
		  case 'n':	/* default no answer flag */
		  case 'N':
				nflag++;
				break;
		  case 'y':	/* default yes answer flag */
		  case 'Y':
				yflag++;
				break;
		  case 'q':
				qflag++;
				break;
		  case 'D':
				Dirc++;
				break;
		  case 'F':
		  case 'f':
				fast++;
				break;
		  case 'm':
				sanity_only++;
				break;
		  case '?':
				errexit(use);
		}
	}
	/*  the generic fsck in /sbin/fsck uses /etc/vfstab as
	 *  a list of file systems to check. 
	 *  No arguments is now an error for the
	 *  fstype specific fsck
	 */
	if(optind >= argc)
		errexit(use);
	if(nflag && preen)
		{
			preen=0;
		}
	/* If both -y and -n are specified, -n overrides */
	if(nflag && yflag)
		yflag=0;
	if(qflag && preen)
		qflag=0;
	if(yflag && preen)
		yflag=0;
	if(nflag && sflag)
		{
		error("%c Incompatible options: -n and -s\n",id);
		errexit(use);
		}
	if(nflag && qflag)
		{
		error("%c Incompatible options: -n and -q\n",id);
		errexit(use);
		}
	/* If sanity check flag is set, then there should be no other options */
	if (sanity_only) {
		if ((argc - 3) > atoi(&sanity_only)) {
			error("s5 fsck: Incompatible options: cannot use -m with other options\n");
			errexit(use);
		}
	}
	if(sflag && csflag)
		sflag = 0;
	if(csflag) nflag++;


#if !STANDALONE
	for ( ; optind < argc ; optind++) {

		initbarea(&sblk);
		if(checksb(argv[optind]) == NO) {
			continue;
		}

/*  Fsck used to be three executables, each compiled with FsTYPE set to
 *  1, 2, or 4 to determine the block size, as follows:
 *  On the 3b2:
 *  FsTYPE	block size	executable
 *	1	512		fsck512
 *	2	1024		fsck, fsck1k
 *	4	2048		fsck2k
 *
 *  On the 3b15:
 *  FsTYPE	block size	executable
 *	1	512		fsck1b
 *	2	2048		fsck
 *	4	1024		fsck2b
 *
 *  Now, fsck is one executable.   The macros from s5param.h
 *  (BSIZE, BSHIFT, BMASK, INOPB) are replaced with variables.
 *  This executable MUST be compiled with FsTYPE set to the largest
 *  blocksize to allocate the biggest buffers needed.
 *  The superblock is read and evaluated, and 
 *  the limits are adjusted accordingly.
 *
 *  The following code exists to deal with different logical block 
 *  sizes and different interpretations of s_type for 3b2 vs. 3b15.
 *  If s_type is Fs2b we don't really know what the block size of the file 
 *  system is so we call a heuristic function to try to find out.
 *  If if the heuristic fails we still don't know the block size
 *  so we ask the user.
 */

	if ((superblk.s_magic != FsMAGIC)
	||  (superblk.s_nfree < 0)
	||  (superblk.s_nfree > NICFREE)
	||  (superblk.s_ninode < 0)
	||  (superblk.s_ninode > NICINOD)
	||  (superblk.s_type < 1))
		errexit ("s5 fsck: %s is not an s5 file system\n",argv[optind]);

	if (sanity_only)
		check_sanity (argv[optind]);  /*  this never returns  */

	switch (superblk.s_type)
	  {
	  case Fs4b:
		init_2048();
#ifdef DEBUG
		printf("type is 2k, switch 1\n");
#endif
		break;

	  case Fs1b:
		init_512();
#ifdef DEBUG
		printf("type is 512, switch 1\n");
#endif
		break;

	  case Fs2b:

#ifdef i386
/* The case for Fs2b for i386 does not need to invoke the s5bsize function.
 * An s_type value of Fs2b is sufficient to positively identify the block
 * size as 1024.
 */
		init_1024();
#else	
		switch (s5bsize(dfile.rfdes))
		  {
		  case Fs4b:
			init_2048();
#ifdef DEBUG
			printf("type is 2k, switch 2\n");
#endif
			break;
		  case Fs2b:
			init_1024();
#ifdef DEBUG
			printf("type is 1k, switch 2\n");
#endif
			break;

		  default:
			/*  ask the user for the blocksize */
			switch (prmptbsize(argv[optind]))
			  {
			  case Fs4b:
				init_2048();
#ifdef DEBUG
				printf("type is 2k, switch 3\n");
#endif
				break;

			  case Fs1b:
				init_512();
#ifdef DEBUG
				printf("type is 512, switch 3\n");
#endif
				break;

			  case Fs2b:
				init_1024();
#ifdef DEBUG
				printf("type is 1k, switch 3\n");
#endif
				break;

			  default:
				errexit ("fsck quitting\n");
			  }
		  }
#endif
		break;

	  default:
		/*  ask the user for the blocksize */
		switch (prmptbsize(argv[optind]))
		  {
		  case Fs1b:
			init_512();
#ifdef DEBUG
			printf("type is 512, switch 3\n");
#endif
			break;

		  case Fs2b:
			init_1024();
#ifdef DEBUG
			printf("type is 1k, switch 3\n");
#endif
			break;

		  case Fs4b:
			init_2048();
#ifdef DEBUG
			printf("type is 2k, switch 3\n");
#endif
			break;

		  default:
			errexit ("fsck quitting\n");
		  }
	  }


		if(!initdone) {
			initmem();
			initdone++;
		}
#ifdef DEBUG

#ifdef i386
	printf("checking %s in fsck\n", argv[optind]);
#else
	printf("checking %s in fsck\n", argv[i]);
#endif

#endif /* DEBUG */

		check(argv[optind]);
	}
#else			/* STANDALONE */
	initmem();
		for ( ; optind < argc ; optind++ ) {
			initbarea(&sblk);
			if(checksb(argv[optind]) == NO) {
				continue;
			}
#if FsTYPE==2
			if(superblk.s_magic != FsMAGIC ||
			(superblk.s_magic == FsMAGIC && superblk.s_type == Fs1b)) {
#ifdef u3b15
				error("%c %s not a 2k file system\n",
					id,argv[optind]);
#else
				error("%c %s not a 1k file system\n",
					id,argv[optind]);
#endif
				continue;
			}
#endif

			check(argv[optind]);
		}
	}
#endif			/* STANDALONE */

	exit(err);

/* NOTREACHED */
}


error(s1,s2,s3,s4)
char *s1, *s2, *s3, *s4;
{
/* set err to 40.  If this is the root file system, bcheckrc will */
/* attempt to remount if fsck exits with 40 */
	err=40;
	fprintf(stderr,s1,s2,s3,s4);
}


errexit(s1,s2,s3,s4)
char *s1, *s2, *s3, *s4;
{
	error(s1,s2,s3,s4);
	exit(31+8);
}

initmem()
{
	register n;
	struct stat statbuf;
	void (*sg)();
	char *sbrk();

	memsize = (MEMSIZE)sbrk(sizeof(int));
	memsize = MAXDATA - memsize - sizeof(int);
	while(memsize >= 2*sizeof(BUFAREA) &&
		(membase = sbrk(memsize)) == (char *)-1)
		memsize -= 1024;
	if(memsize < 2*sizeof(BUFAREA))
		errexit("%c Can't get memory\n",id);
#ifndef STANDALONE
	for(n = 1; n < NSIG; n++) {
		if(n == SIGCLD || n == SIGPWR)
			continue;
		sg = signal(n,catch);
		if(sg != SIG_DFL)
			signal(n,sg);
	}
#endif
	/* Check if standard input is a pipe. If it is, record pipedev so
	 * we won't ever check it */
	if ( fstat( 0, &statbuf) == -1 )
		errexit("%c Can't fstat standard input\n", id);
	if ( (statbuf.st_mode & S_IFMT) == S_IFIFO ) pipedev = statbuf.st_dev;
}

check(dev)
char *dev;
{
	register DINODE *dp;
	register n;
	register ino_t *blp;
	ino_t savino;
	daddr_t blk;
	BUFAREA *bp1, *bp2;

	if(pipedev != -1) {
		strcpy(devname,dev);
		strcat(devname,"\t");
	}
	else
		devname[0] = '\0';
	if(setup(dev) == NO)
		return;

	printf("%c %s** Phase 1 - Check Blocks and Sizes\n",id,devname);
	pfunc = pass1;
	for(inum = 1; inum <= imax; inum++) {
		if((dp = ginode()) == NULL)
			continue;
		if(ALLOC) {
			lastino = inum;
			if(ftypeok(dp) == NO) {
				error("%c %sUNKNOWN FILE TYPE I=%u",id,devname,inum);
				if(dp->di_size)
					printf(" (NOT EMPTY)");
				if(reply("CLEAR", "CLEARED", 1) == YES) {
					zapino(dp);
					inodirty();
				}
				continue;
			}
			n_files++;
			if(setlncnt(dp->di_nlink) <= 0) {
				if(badlnp < &badlncnt[MAXLNCNT])
					*badlnp++ = inum;
				else {
					error("%c %sLINK COUNT TABLE OVERFLOW",id,devname);
					if(reply("CONTINUE", "EXITING, RUN FSCK MANUALLY", 0) == NO)
						errexit("");
				}
			}
			setstate(DIR ? DSTATE : FSTATE);
			badblk = dupblk = 0;
			filsize = 0;
			ckinode(dp,ADDR);
			if(!SPECIAL && ((n = getstate()) == DSTATE || n == FSTATE))
				sizechk(dp);
		}
		else if(dp->di_mode != 0) {
			error("%c %sPARTIALLY ALLOCATED INODE I=%u",id,devname,inum);
			if(dp->di_size)
				printf(" (NOT EMPTY)");
			if(reply("CLEAR", "CLEARED", 1) == YES) {
				zapino(dp);
				inodirty();
			}
		}
	}


	if(enddup != &duplist[0]) {
		printf("%c %s** Phase 1b - Rescan For More DUPS\n",id,devname);
		pfunc = pass1b;
		for(inum = 1; inum <= lastino; inum++) {
			if(getstate() != USTATE && (dp = ginode()) != NULL)
				if(ckinode(dp,ADDR) & STOP)
					break;
		}
	}
	if(rawflg) {
		if(inoblk.b_dirty)
			bwrite(&dfile,membase,startib,niblk*F_BSIZE);
		inoblk.b_dirty = 0;
		if(poolhead) {
			clear(membase,niblk*F_BSIZE);
			for(bp1 = poolhead;bp1->b_next;bp1 = bp1->b_next);
			bp2 = &((BUFAREA *)membase)[(niblk*F_BSIZE)/sizeof(BUFAREA)];
			while(--bp2 >= (BUFAREA *)membase) {
				initbarea(bp2);
				bp2->b_next = bp1->b_next;
				bp1->b_next = bp2;
			}
		}
		rawflg = 0;

	}


if(!fast) {
	printf("%c %s** Phase 2 - Check Pathnames\n",id,devname);
	inum = S5ROOTINO;
	thisname = pathp = 0;
	pfunc = pass2;
	switch(getstate()) {
		case USTATE:
			errexit("%c %sROOT INODE UNALLOCATED. TERMINATING.\n",id,devname);
		case FSTATE:
			error("%c %sROOT INODE NOT DIRECTORY",id,devname);
			if(reply("FIX", "EXITING, RUN FSCK MANUALLY", 0) == NO || (dp = ginode()) == NULL)
				errexit("");
			dp->di_mode &= ~S_IFMT;
			dp->di_mode |= S_IFDIR;
			inodirty();
			setstate(DSTATE);
		case DSTATE:
			descend();
			break;
		case CLEAR:
			error("%c %sDUPS/BAD IN ROOT INODE\n",id,devname);
			if(reply("CONTINUE", "EXITING, RUN FSCK MANUALLY", 0) == NO)
				errexit("");
			setstate(DSTATE);
			descend();
	}


	pss2done++;
	printf("%c %s** Phase 3 - Check Connectivity\n",id,devname);
	for(inum = S5ROOTINO; inum <= lastino; inum++) {
		if(getstate() == DSTATE) {
			pfunc = findino;
			srchname = "..";
			savino = inum;
			do {
				orphan = inum;
				if((dp = ginode()) == NULL)
					break;
				filsize = dp->di_size;
				parentdir = 0;
				ckinode(dp,DATA);
				if((inum = parentdir) == 0)
					break;
			} while(getstate() == DSTATE);
			inum = orphan;
			if(linkup() == YES) {
				thisname = pathp = 0;
				addpath("?", 1);
				pfunc = pass2;
				descend();
			}
			inum = savino;
		}
	}


	printf("%c %s** Phase 4 - Check Reference Counts\n",id,devname);
	pfunc = pass4;
	for(inum = S5ROOTINO; inum <= lastino; inum++) {
		switch(getstate()) {
			case FSTATE:
				if(n = getlncnt())
					adjust((short)n);
				else {
					for(blp = badlncnt;blp < badlnp; blp++)
						if(*blp == inum) {
							if((dp = ginode()) &&
							dp->di_size) {
								if((n = linkup()) == NO)
								   clri("UNREF",NO);
								if (n == REM)
								   clri("UNREF",REM);
							}
							else
								clri("UNREF",YES);
							break;
						}
				}
				break;
			case DSTATE:
				clri("UNREF",YES);
				break;
			case CLEAR:
				clri("BAD/DUP",YES);
		}
	}
	if(imax - n_files != superblk.s_tinode) {
		error("%c %sFREE INODE COUNT WRONG IN SUPERBLK",id,devname);
		if (qflag) {
			superblk.s_tinode = imax - n_files;
			sbdirty();
			printf("\n%c %sFIXED\n",id,devname);
		}
		else if(reply("FIX", "FIXED", 1) == YES) {
			superblk.s_tinode = imax - n_files;
			sbdirty();
		}
	}
	flush(&dfile,&fileblk);


}	/* if fast check, skip to phase 5 */
	printf("%c %s** Phase 5 - Check Free List ",id,devname);
#if u3b2 || u3b15
	rebflg = dfile.mod+fileblk.b_dirty+sblk.b_dirty+inoblk.b_dirty;
#endif
	if(sflag || (csflag && rplyflag == 0)) {
		printf("(Ignored)\n");
		fixfree = YES;
	}
	else {
		printf("\n");
		if(freemap)
			copy(blkmap,freemap,(MEMSIZE)bmapsz);
		else {
			for(blk = 0; blk < fmapblk; blk++) {
				bp1 = getblk(NULL,blk);
				bp2 = getblk(NULL,blk+fmapblk);
				copy(bp1->b_un.b_buf,bp2->b_un.b_buf,F_BSIZE);
				dirty(bp2);
			}
		}
		badblk = dupblk = 0;
		freeblk.df_nfree = superblk.s_nfree;
		for(n = 0; n < NICFREE; n++)
			freeblk.df_free[n] = superblk.s_free[n];
		freechk();
		if(badblk)
			error("%c %s%d BAD BLKS IN FREE LIST\n",id,devname,badblk);
		if(dupblk)
			error("%c %s%d DUP BLKS IN FREE LIST\n",id,devname,dupblk);

		if(fixfree == NO) {
			if((n_blks+n_free) != (fmax-fmin)) {
				error("%c %s%ld BLK(S) MISSING\n",id,devname,
					fmax-fmin-n_blks-n_free);
				fixfree = YES;
			}
			else if(n_free != superblk.s_tfree) {
				error("%c %sFREE BLK COUNT WRONG IN SUPERBLK",id,devname);
				if(qflag) {
					superblk.s_tfree = n_free;
					sbdirty();
					printf("\n%c %sFIXED\n",id,devname);
				}
				else if(reply("FIX", "FIXED", 1) == YES) {
					superblk.s_tfree = n_free;
					sbdirty();
				}
			}
		}
		if(fixfree) {
			error("%c %sBAD FREE LIST",id,devname);
			if(qflag && !sflag) {
				fixfree = YES;
				printf("\n%c %sSALVAGED\n",id,devname);
			}
			else if(reply("SALVAGE", "SALVAGED", 1) == NO)
				fixfree = NO;
		}
	}

	if(fixfree) {
		printf("%c %s** Phase 6 - Salvage Free List\n",id,devname);
		makefree();
		n_free = superblk.s_tfree;
	}
	flush(&dfile,&fileblk);
	flush(&dfile,&inoblk);
	flush(&dfile,&sblk);
	if (dfile.mod) {
		fixstate = 1;
	} else {
		fixstate = 0;
	}
#if u3b2 || u3b15 || i386
	if (hotroot && (superblk.s_state == FsACTIVE))
		rebflg = 1;
	else if ((superblk.s_state + (long)superblk.s_time) != FsOKAY) {
		if (qflag) {
			fixstate = 1;
		} else if(dfile.mod || rplyflag) {
			if (reply("SET FILE SYSTEM STATE TO OKAY", "FILE SYSTEM STATE SET TO OK", 1) == YES)
				fixstate = 1;
			else
				fixstate = 0;
		} else if (nflag) {
			printf("%c %sFILE SYSTEM STATE NOT SET TO OKAY\n",id,devname);
			fixstate = 0;
		} else {
			printf("%c %sFILE SYSTEM STATE SET TO OKAY\n",id,devname);
			fixstate = 1;
		}
	}
#endif
	printf("%c %s%ld files %ld blocks %ld free\n",id,devname,
		n_files,n_blks<<(F_BSHIFT-9),n_free<<(F_BSHIFT-9));

	if (root_ptr)
		fcorr_dump(dev);

	if(dfile.mod || fixstate) {
#ifndef STANDALONE
		time(&superblk.s_time);
#endif
#if u3b2 || u3b15 || i386
		if(hotroot && (fixstate || rebflg)) {
				superblk.s_state = FsACTIVE;
		} else if(fixstate || rebflg)
			superblk.s_state = FsOKAY - (long)superblk.s_time;

#endif
		sbdirty();
	}
	ckfini();
#ifndef STANDALONE
	sync();
	if (dfile.mod) {
		if (hotroot) {
			printf("\n%c %s*** ROOT FILE SYSTEM WAS MODIFIED ***\n",
				id,devname);
					  exit(40);
		} else 
			printf("%c %s*** FILE SYSTEM WAS MODIFIED ***\n",id,devname);
	}
#endif
}


ckinode(dp,flg)
register DINODE *dp;
register flg;
{
	register daddr_t *ap;
	register ret;
	int (*func)(), n;
	daddr_t	iaddrs[NADDR];

	if(SPECIAL)
		return(KEEPON);
	l3tol(iaddrs,dp->di_addr,NADDR);
	switch(flg) {
		case ADDR:
			func = pfunc;
			break;
		case DATA:
			func = dirscan;
			break;
		case BBLK:
			func = chkblk;
	}
	for(ap = iaddrs; ap < &iaddrs[NADDR-3]; ap++) {
		if(*ap && (ret = (*func)(*ap,((ap == &iaddrs[0]) ? 1 : 0))) & STOP)
			if(flg != BBLK)
				return(ret);
	}
	for(n = 1; n < 4; n++) {
		if(*ap && (ret = iblock(*ap,n,flg)) & STOP) {
			if(flg != BBLK)
				return(ret);
		}
		ap++;
	}
	return(KEEPON);
}

iblock(blk,ilevel,flg)
daddr_t blk;
register ilevel;
{
	register daddr_t *ap;
	register n;
	int (*func)();

	BUFAREA ib;

	if(flg == BBLK)		func = chkblk;
	else if(flg == ADDR) {
		func = pfunc;
		if(((n = (*func)(blk)) & KEEPON) == 0)
			return(n);
	}
	else
		func = dirscan;
	if(outrange(blk))
		return(SKIP);
	initbarea(&ib);
	if(getblk(&ib,blk) == NULL)
		return(SKIP);
	ilevel--;
	for(ap = ib.b_un.b_indir; ap < &ib.b_un.b_indir[F_NINDIR]; ap++) {
		if(*ap) {
			if(ilevel > 0)
				n = iblock(*ap,ilevel,flg);
			else
				n = (*func)(*ap,0);
			if(n & STOP && flg != BBLK)
				return(n);
		}
	}
	return(KEEPON);
}


chkblk(blk,flg)
register daddr_t blk;
{
	register DIRECT *dirp;
	register char *ptr;
	int zerobyte, baddir = 0, dotcnt = 0;

	if(outrange(blk))
		return(SKIP);
	if(getblk(&fileblk, blk) == NULL)
		return(SKIP);
	for(dirp = dirblk; dirp <&dirblk[NDIRECT]; dirp++) {
		ptr = dirp->d_name;
		zerobyte = 0;
		while(ptr <&dirp->d_name[DIRSIZ]) {
			if(zerobyte && *ptr) {
				baddir++;
				break;
			}
			if(flg) {
				if(ptr == &dirp->d_name[0] && *ptr == '.' &&
					*(ptr + 1) == '\0') {
					dotcnt++;
					if(inum != dirp->d_ino) {
						error("%c %sNO VALID '.' in DIR I = %u\n",
							id,devname,inum);
						baddir++;
					}
					break;
				}
				if(ptr == &dirp->d_name[0] && *ptr == '.' &&
					*(ptr + 1) == '.' && *(ptr + 2) == '\0') {
					dotcnt++;
					if(!dirp->d_ino) {
						error("%c %sNO VALID '..' in DIR I = %u\n",
							id,devname,inum);
						baddir++;
					}
					break;
				}
			}
			if(*ptr == '/') {
				baddir++;
				break;
			}
			if(*ptr == NULL) {
				if(dirp->d_ino && ptr == &dirp->d_name[0]) {
					baddir++;
					break;
				}
				else
					zerobyte++;
			}
			ptr++;
		}
	}
	if(flg && dotcnt < 2) {
		error("%c %sMISSING '.' or '..' in DIR I = %u\n",id,devname,inum);
		error("%c %sBLK %ld ",id,devname,blk);
		pinode();
		error("\n%c %sDIR=%s\n\n",id,devname,pathname);
		if  (fcorr_flg)
			fcorr_path(0);
		return(YES);
	}
	if(baddir) {
		error("%c %sBAD DIR ENTRY I = %u\n",id,devname,inum);
		error("%c %sBLK %ld ",id,devname,blk);
		pinode();
		error("\n%c %sDIR=%s\n\n",id,devname,pathname);
		if  (fcorr_flg)
			fcorr_path(0);
		return(YES);
	}
	return(KEEPON);
}

pass1(blk)
register daddr_t blk;
{
	register daddr_t *dlp;

	if(outrange(blk)) {
		blkerr("BAD",blk);
		if(++badblk >= MAXBAD) {
			error("%c %sEXCESSIVE BAD BLKS I=%u",id,devname,inum);
			if(reply("CONTINUE", "EXITING, RUN FSCK MANUALLY", 0) == NO)
				errexit("");
			return(STOP);
		}
		return(SKIP);
	}
	if(getbmap(blk)) {
		blkerr("DUP",blk);
		if(++dupblk >= MAXDUP) {
			error("%c %sEXCESSIVE DUP BLKS I=%u",id,devname,inum);
			if(reply("CONTINUE", "EXITING, RUN FSCK MANUALLY", 0) == NO)
				errexit("");
			return(STOP);
		}
		if(enddup >= &duplist[DUPTBLSIZE]) {
			error("%c %sDUP TABLE OVERFLOW.",id,devname);
			if(reply("CONTINUE", "EXITING, RUN FSCK MANUALLY", 0) == NO)
				errexit("");
			return(STOP);
		}
		for(dlp = duplist; dlp < muldup; dlp++) {
			if(*dlp == blk) {
				*enddup++ = blk;
				break;
			}
		}
		if(dlp >= muldup) {
			*enddup++ = *muldup;
			*muldup++ = blk;
		}
	}
	else {
		n_blks++;
		setbmap(blk); 
	}
	filsize++;
	return(KEEPON);
}

pass1b(blk)
register daddr_t blk;
{
	register daddr_t *dlp;

	if(outrange(blk))
		return(SKIP);
	for(dlp = duplist; dlp < muldup; dlp++) {
		if(*dlp == blk) {
			blkerr("DUP",blk);
			*dlp = *--muldup;
			*muldup = blk;
			return(muldup == duplist ? STOP : KEEPON);
		}
	}
	return(KEEPON);
}


pass2(dirp)
register DIRECT *dirp;
{
	register char *p;
	register n;
	register DINODE *dp;

	if((inum = dirp->d_ino) == 0)
		return(KEEPON);
	thisname = pathp;
	addpath(dirp->d_name, DIRSIZ);
	n = NO;
	if(inum > imax || inum < S5ROOTINO)
		n = direrr("I OUT OF RANGE");
	else {
	again:
		switch(getstate()) {
			case USTATE:
				n = direrr("UNALLOCATED");
				break;
			case CLEAR:
				if((n = direrr("DUP/BAD")) == YES)
					break;
				if((dp = ginode()) == NULL)
					break;
				setstate(DIR ? DSTATE : FSTATE);
				goto again;
			case FSTATE:
				declncnt();
				break;
			case DSTATE:
				declncnt();
				descend();
		}
	}
	pathp = thisname;
	if(n == NO)
		return(KEEPON);
	dirp->d_ino = 0;
	return(KEEPON|ALTERD);
}


pass4(blk)
register daddr_t blk;
{
	register daddr_t *dlp;

	if(outrange(blk))
		return(SKIP);
	if(getbmap(blk)) {
		for(dlp = duplist; dlp < enddup; dlp++)
			if(*dlp == blk) {
				*dlp = *--enddup;
				return(KEEPON);
			}
		clrbmap(blk);
		n_blks--;
	}
	return(KEEPON);
}


pass5(blk)
register daddr_t blk;
{
	if(outrange(blk)) {
		fixfree = YES;
		if(++badblk >= MAXBAD) {
			error("%c %sEXCESSIVE BAD BLKS IN FREE LIST.",id,devname);
			if(reply("CONTINUE", "EXITING, RUN FSCK MANUALLY", 0) == NO)
				errexit("");
			return(STOP);
		}
		return(SKIP);
	}
	if(getfmap(blk)) {
		fixfree = YES;
		if(++dupblk >= DUPTBLSIZE) {
			error("%c %sEXCESSIVE DUP BLKS IN FREE LIST.",id,devname);
			if(reply("CONTINUE", "EXITING, RUN FSCK MANUALLY", 0) == NO)
				errexit("");
			return(STOP);
		}
	}
	else {
		n_free++;
		setfmap(blk); 
	}
	return(KEEPON);
}


blkerr(s,blk)
daddr_t blk;
char *s;
{
#ifdef DEBUG
printf("in blkerr, id = %c, devname = '%s', blk = %ld, s = '%s', Inum = %u\n",
	id,devname,blk,s,inum);
#else
	printf("%c %s%ld %s I=%u\n",id,devname,blk,s,inum);
#endif
	setstate(CLEAR);	/* mark for possible clearing */
}


descend()
{
	register DINODE *dp;
	register int savname;
	off_t savsize;

	setstate(FSTATE);
	if((dp = ginode()) == NULL)
		return;
	if(Dirc && !pss2done)
		ckinode(dp,BBLK);
	savname = thisname;
	addpath("/", 1);
	savsize = filsize;
	filsize = dp->di_size;
	ckinode(dp,DATA);
	thisname = savname;
	pathname[--pathp] = '\0';
	filsize = savsize;
}


dirscan(blk)
register daddr_t blk;
{
	register DIRECT *dirp;
	register char *p1, *p2;
	register n;
	DIRECT direntry;

	if(outrange(blk)) {
		filsize -= F_BSIZE;
		return(SKIP);
	}
	for(dirp = dirblk; dirp < &dirblk[NDIRECT] &&
		filsize > 0; dirp++, filsize -= sizeof(DIRECT)) {
		if(getblk(&fileblk,blk) == NULL) {
			filsize -= (&dirblk[NDIRECT]-dirp)*sizeof(DIRECT);
			return(SKIP);
		}
		p1 = &dirp->d_name[DIRSIZ];
		p2 = &direntry.d_name[DIRSIZ];
		while(p1 > (char *)dirp)
			*--p2 = *--p1;
		if((n = (*pfunc)(&direntry)) & ALTERD) {
			if(getblk(&fileblk,blk) != NULL) {
				p1 = &dirp->d_name[DIRSIZ];
				p2 = &direntry.d_name[DIRSIZ];
				while(p1 > (char *)dirp)
					*--p1 = *--p2;
				fbdirty();
			}
			else
				n &= ~ALTERD;
		}
		if(n & STOP)
			return(n);
	}
	return(filsize > 0 ? KEEPON : STOP);
}


direrr(s)
char *s;
{
	register DINODE *dp;
	int n;

	printf("%c %s%s ",id,devname,s);
	pinode();
	if((dp = ginode()) != NULL && ftypeok(dp)) {
		printf("\n%c %s%s=%s",id,devname,DIR?"DIR":"FILE",pathname);
		if  (fcorr_flg)
			fcorr_path(0);
		if(DIR) {
			if(dp->di_size > EMPT) {
				if((n = chkempt(dp)) == NO) {
					printf(" (NOT EMPTY)\n");
				}
				else if(n != SKIP) {
					printf(" (EMPTY)");
					if(!nflag) {
						printf(" -- REMOVED\n");
						return(YES);
					}
					else
						printf("\n");
				}
			}
			else {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					return(YES);
				}
				else
					printf("\n");
			}
		}
		else if(REG)
			if(!dp->di_size) {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					return(YES);
				}
				else
					printf("\n");
			}
	}
	else {
		printf("\n%c %sNAME=%s",id,devname,pathname);
		if  (fcorr_flg)
			fcorr_path(0);
		if(!dp->di_size) {
			printf(" (EMPTY)");
			if(!nflag) {
				printf(" -- REMOVED\n");
				return(YES);
			}
			else
				printf("\n");
		}
		else
			printf(" (NOT EMPTY)\n");
	}
	return(reply("REMOVE", "REMOVED", 1));
}


adjust(lcnt)
register short lcnt;
{
	register DINODE *dp;
	register n;

	if((dp = ginode()) == NULL)
		return;
	if(dp->di_nlink == lcnt) {
		if((n = linkup()) == NO)
			clri("UNREF",NO);
		if(n == REM)
			clri("UNREF",REM);
	}
	else {
		printf("%c %sLINK COUNT %s",id,devname,
			(lfdir==inum)?lfname:(DIR?"DIR":"FILE"));
		pinode();
		printf("\n%c %sCOUNT %d SHOULD BE %d",id,devname,
			dp->di_nlink,dp->di_nlink-lcnt);
		if(reply("ADJUST", "ADJUSTED", 1) == YES) {
			dp->di_nlink -= lcnt;
			inodirty();
		}
	}
}


clri(s,flg)
char *s;
{
	register DINODE *dp;
	int n;

	if((dp = ginode()) == NULL)
		return;
	if(flg == YES) {
		if(!FIFO || !qflag || nflag) {
			printf("%c %s%s %s",id,devname,s,DIR?"DIR":"FILE");
			pinode();
		}
		if(DIR) {
			if(dp->di_size > EMPT) {
				if((n = chkempt(dp)) == NO) {
					printf(" (NOT EMPTY)\n");
				}
				else if(n != SKIP) {
					printf(" (EMPTY)");
					if(!nflag) {
						printf(" -- REMOVED\n");
						clrinode(dp);
						return;
					}
					else
						printf("\n");
				}
			}
			else {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					clrinode(dp);
					return;
				}
				else
					printf("\n");
			}
		}
		if(REG)
			if(!dp->di_size) {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" -- REMOVED\n");
					clrinode(dp);
					return;
				}
				else
					printf("\n");
			}
			else
				printf(" (NOT EMPTY)\n");
		if (FIFO && !nflag) {
			if(!qflag)	printf(" -- CLEARED");
			printf("\n");
			clrinode(dp);
			return;
		}
	}
	if(flg == REM)	clrinode(dp);
	else if(reply("CLEAR", "CLEARED", 1) == YES)
		clrinode(dp);
}


clrinode(dp)		/* quietly clear inode */
register DINODE *dp;
{

	n_files--;
	pfunc = pass4;
	ckinode(dp,ADDR);
	zapino(dp);
	inodirty();
}

chkempt(dp)
register DINODE *dp;
{
	register daddr_t *ap;
	register DIRECT *dirp;
	daddr_t blk[NADDR];
	int size;

	size = minsz(dp->di_size, (NADDR - 3) * F_BSIZE);
	l3tol(blk,dp->di_addr,NADDR);
	for(ap = blk; ap < &blk[NADDR - 3] && size > 0; ap++) {
		if(*ap) {
			if(outrange(*ap)) {
				printf("chkempt: blk %d out of range\n",*ap);
				return(SKIP);
			}
			if(getblk(&fileblk,*ap) == NULL) {
				printf("chkempt: Can't find blk %d\n",*ap);
				return(SKIP);
			}
			for(dirp=dirblk; dirp < &dirblk[NDIRECT] &&
			size > 0; dirp++) {
				if(dirp->d_name[0] == '.' &&
				(dirp->d_name[1] == '\0' || (
				dirp->d_name[1] == '.' &&
				dirp->d_name[2] == '\0'))) {
					size -= sizeof(DIRECT);
					continue;
				}
				if(dirp->d_ino)
					return(NO);
				size -= sizeof(DIRECT);
			}
		}
	}
	if(size <= 0)	return(YES);
	else	return(NO);
}

setup(dev)
char *dev;
{
	register n;
	register BUFAREA *bp;
	register MEMSIZE msize;
	char *mbase;
	daddr_t bcnt, nscrblk;
	dev_t rootdev;
	extern dev_t pipedev;	/* non-zero iff standard input is a pipe,
				 * which means we can't check pipedev */
	off_t smapsz, lncntsz, totsz;
	struct {
		daddr_t	tfree;
		ino_t	tinode;
		char	fname[6];
		char	fpack[6];
	} ustatarea;
	struct stat statarea;

	if(stat("/",&statarea) < 0)
		errexit("%c %sCan't stat root\n",id,devname);
	rootdev = statarea.st_dev;
	if(stat(dev,&statarea) < 0) {
		error("%c %sCan't stat %s\n",id,devname,dev);
		return(NO);
	}
	hotroot = 0;
	rawflg = 0;
	if((statarea.st_mode & S_IFMT) == S_IFBLK) {
		if(rootdev == statarea.st_rdev)
			hotroot++;
		else if(ustat(statarea.st_rdev,&ustatarea) >= 0) {
			if(!nflag) {
				error("%c %s%s is a mounted file system, ignored\n",
					id,devname,dev);
				return(NO);
			}
		}
		if ( pipedev == statarea.st_rdev )
		{	error( "%c %s%s is pipedev, ignored", id,
					devname, dev);
			return(NO);
		}
	}
	else if((statarea.st_mode & S_IFMT) == S_IFCHR){

#ifdef MB1
		checkbs(dev);
#endif

		rawflg++;
/*  These changes to protect the user from checking a file system as a  */
/*  character device while the file system is mounted as a block device */
/*  will work only as long as the major and minor numbers of the said */
/*  devices are the same. (Which is the case on the 3B2 and 3B15)		*/

		

	}

#ifndef DEBUG
	else
	{		
		error("%c %s%s is not a block or character device\n",id,devname,dev);
		return(NO);
	}
#endif

	printf("\n%c %s",id,dev);
	if((nflag && !csflag) || (dfile.wfdes == -1))
		printf(" (NO WRITE)");
	printf("\n");
	pss2done = 0;
	fixfree = NO;
	dfile.mod = 0;
	n_files = n_blks = n_free = 0;
	muldup = enddup = &duplist[0];
	badlnp = &badlncnt[0];
	lfdir = 0;
	rplyflag = 0;
	initbarea(&fileblk);
	initbarea(&inoblk);
	sfile.wfdes = sfile.rfdes = -1;
	rmscr = 0;
	if(getblk(&sblk,SUPERB) == NULL) {
		ckfini();
		return(NO);
	}
	imax = ((ino_t)superblk.s_isize - (SUPERB+1)) * F_INOPB;
	fmax = superblk.s_fsize;		/* first invalid blk num */

	fmin = (daddr_t)superblk.s_isize;
	bmapsz = roundup(howmany(fmax,BITSPB),sizeof(*lncntp));

	if(fmin >= fmax || 
		(imax/(ino_t)F_INOPB) != ((ino_t)superblk.s_isize-(SUPERB+1))) {
		error("%c %sSize check: fsize %ld isize %d\n",id,devname,
			superblk.s_fsize,superblk.s_isize);
		ckfini();
		return(NO);
	}
	printf("%c %sFile System: %.6s Volume: %.6s\n\n",id,devname,
		superblk.s_fname,superblk.s_fpack);

	smapsz = roundup(howmany((long)(imax+1),STATEPB),sizeof(*lncntp));
	lncntsz = (long)(imax+1) * sizeof(*lncntp);
	if(bmapsz > smapsz+lncntsz)
		smapsz = bmapsz-lncntsz;
	totsz = bmapsz+smapsz+lncntsz;
	msize = memsize;
	mbase = membase;
	if(rawflg) {
		if(msize < (MEMSIZE)(NINOBLK*F_BSIZE) + 2*sizeof(BUFAREA))
			rawflg = 0;
		else {
			msize -= (MEMSIZE)NINOBLK*F_BSIZE;
			mbase += (MEMSIZE)NINOBLK*F_BSIZE;
			niblk = NINOBLK;
			startib = fmax;
		}
	}
	clear(mbase,msize);
	if((off_t)msize < totsz) {
		bmapsz = roundup(bmapsz,F_BSIZE);
		smapsz = roundup(smapsz,F_BSIZE);
		lncntsz = roundup(lncntsz,F_BSIZE);
		nscrblk = (bmapsz+smapsz+lncntsz)>>F_BSHIFT;
		if(tflag == 0) {
			printf("\n%c %sNEED SCRATCH FILE (%ld BLKS)\n",id,devname,nscrblk);
			do {
				printf("%c %sENTER FILENAME:\n",id,devname);
				if((n = getline(stdin,scrfile,sizeof(scrfile))) == EOF)
					errexit("\n");
			} while(n == 0);
		}
		if(stat(scrfile,&statarea) < 0 ||
			(statarea.st_mode & S_IFMT) == S_IFREG)
			rmscr++;
		if((sfile.wfdes = creat(scrfile,0666)) < 0 ||
			(sfile.rfdes = open(scrfile,O_RDONLY)) < 0) {
			error("%c %sCan't create %s\n",id,devname,scrfile);
			ckfini();
			return(NO);
		}
		bp = &((BUFAREA *)mbase)[(msize/sizeof(BUFAREA))];
		poolhead = NULL;
		while(--bp >= (BUFAREA *)mbase) {
			initbarea(bp);
			bp->b_next = poolhead;
			poolhead = bp;
		}
		bp = poolhead;
		for(bcnt = 0; bcnt < nscrblk; bcnt++) {
			bp->b_bno = bcnt;
			dirty(bp);
			flush(&sfile,bp);
		}
		blkmap = freemap = statemap = (char *) NULL;
		lncntp = (short *) NULL;
		smapblk = bmapsz / F_BSIZE;
		lncntblk = smapblk + smapsz / F_BSIZE;
		fmapblk = smapblk;
	}
	else {
		if(rawflg && (off_t)msize > totsz+F_BSIZE) {
			niblk += (unsigned)((off_t)msize-totsz)>>F_BSHIFT;

			if(niblk > (MAXRAW >> (F_BSHIFT - 9)) )
				niblk = MAXRAW >> (F_BSHIFT -9);
			msize = memsize - (niblk*F_BSIZE);
			mbase = membase + (niblk*F_BSIZE);
		}
		poolhead = NULL;
		blkmap = mbase;
		statemap = &mbase[(MEMSIZE)bmapsz];
		freemap = statemap;
		lncntp = (short *)&statemap[(MEMSIZE)smapsz];
	}
	return(YES);
}

checksb(dev)
char *dev;
{
	if((dfile.rfdes = open(dev,O_RDONLY)) < 0) {
		error("%c %sCan't open %s\n",id,devname,dev);
		return(NO);
	}
	/*  nflag alone means no write.
	    nflag with csflag (-S) means answer NO to all, but open
	    for write to rebuild the freelist  */
	if((nflag && !csflag) || (dfile.wfdes = open(dev,O_WRONLY)) < 0)
		dfile.wfdes = -1;
	if(getblk(&sblk,SUPERB) == NULL) {
		ckfini();
		return(NO);
	}
return(YES);
}

DINODE *
ginode()
{
	register DINODE *dp;
	register char *mbase;
	register daddr_t iblk;

	if(inum > imax)
		return(NULL);
	iblk = itod(inum);
	if(rawflg) {
		mbase = membase;
		if(iblk < startib || iblk >= startib+niblk) {
			if(inoblk.b_dirty)
				bwrite(&dfile,mbase,startib,niblk*F_BSIZE);
			inoblk.b_dirty = 0;
			if(bread(&dfile,mbase,iblk,niblk*F_BSIZE) == NO) {
				startib = fmax;
				return(NULL);
			}
			startib = iblk;
		}
		dp = (DINODE *)&mbase[(unsigned)((iblk-startib)<<F_BSHIFT)];
	}
	else if(getblk(&inoblk,iblk) != NULL)
		dp = inoblk.b_un.b_dinode;
	else
		return(NULL);
	return(dp + itoo(inum));
}

reply(s, msg, yn)
char *s;
char *msg;
int yn;
{
	char line[80];

	rplyflag = 1;
	line[0] = '\0';
	if (preen) {
		printf("\n%c %s%s\n\n", id, devname, msg);
		if (yn)
			return(YES);
	        else
			return(NO);
	}
	printf("\n%c %s%s? ",id,devname,s);
	if(nflag || dfile.wfdes < 0) {
		printf(" no\n\n");
		return(NO);
	}
	if(yflag) {
		printf(" yes\n\n");
		return(YES);
	}
	while (line[0] == '\0') {
		if(getline(stdin,line,sizeof(line)) == EOF)
			errexit("\n");
		printf("\n");
		if(line[0] == 'y' || line[0] == 'Y')
			return(YES);
		if(line[0] == 'n' || line[0] == 'N')
			return(NO);
		printf("%c %sAnswer 'y' or 'n' (yes or no)\n",id,devname);
		line[0] = '\0';
	}
return(NO);
}


getline(fp,loc,maxlen)
FILE *fp;
char *loc;
{
	register n, ignore_rest = 0;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[maxlen-1];
	while((n = getc(fp)) != '\n') {
		if(n == EOF)
			return(EOF);
		if(isspace(n)){
			if(p > loc){
				ignore_rest = 1;
			}
		}else{
			if(ignore_rest == 0 && p < lastloc){
				*p++ = n;
			}
		}
	}
	*p = 0;
	return(p - loc);
}



stype(p)
	register char *p;
{
	if(p == NULL)
		return;
	if (*(p+1) == 0) {
		if (*p == '3') {
			cylsize = 200;
			stepsize = 5;
			return;
		}
		if (*p == '4') {
			cylsize = 418;
			stepsize = 7;
			return;
		}
	}
	cylsize = atoi(p);
	while(*p && *p != ':')
		p++;
	if(*p)
		p++;
	stepsize = atoi(p);
	if(stepsize <= 0 || stepsize > cylsize ||
	cylsize <= 0 || cylsize > MAXCYL) {
		error("%c %sInvalid -s argument, defaults assumed\n",id,devname);
		cylsize = stepsize = 0;
	}
}


dostate(s,flg)
{
	register char *p;
	register unsigned byte, shift;
	BUFAREA *bp;

	byte = ((unsigned)inum)/STATEPB;
	shift = LSTATE * (((unsigned)inum)%STATEPB);
	if(statemap != NULL) {
		bp = NULL;
		p = &statemap[byte];
	}
	else if((bp = getblk(NULL,smapblk+(byte/F_BSIZE))) == NULL)
		errexit("%c %sFatal I/O error\n",id,devname);
	else
		p = &bp->b_un.b_buf[byte%F_BSIZE];
	switch(flg) {
		case 0:
			*p &= ~(SMASK<<(shift));
			*p |= s<<(shift);
			if(bp != NULL)
				dirty(bp);
			return(s);
		case 1:
			return((*p>>(shift)) & SMASK);
	}
	return(USTATE);
}


domap(blk,flg)
register daddr_t blk;
{
	register char *p;
	register unsigned n;
	register BUFAREA *bp;
	off_t byte;

	byte = blk >> BITSHIFT;
	n = 1<<((unsigned)(blk & BITMASK));
	if(flg & 04) {
		p = freemap;
		blk = fmapblk;
	}
	else {
		p = blkmap;
		blk = 0;
	}
	if(p != NULL) {
		bp = NULL;
		p += (unsigned)byte;
	}
	else if((bp = getblk(NULL,blk+(byte>>F_BSHIFT))) == NULL)
		errexit("%c %sFatal I/O error\n",id,devname);
	else
		p = &bp->b_un.b_buf[(unsigned)(byte&F_BMASK)];
	switch(flg&03) {
		case 0: /* set */
			*p |= n;
			break;
		case 1: /* get */
			n &= *p;
			bp = NULL;
			break;
		case 2: /* clear */
			*p &= ~n;
	}
	if(bp != NULL)
		dirty(bp);
	return(n);
}


dolncnt(val,flg)
short val;
{
	register short *sp;
	register BUFAREA *bp;

	if(lncntp != NULL) {
		bp = NULL;
		sp = &lncntp[(unsigned)inum];
	}
	else if((bp = getblk(NULL,lncntblk+((unsigned)inum/SPERB))) == NULL)
		errexit("%c %sFatal I/O error\n",id,devname);
	else
		sp = &bp->b_un.b_lnks[(unsigned)inum%SPERB];
	switch(flg) {
		case 0:
			*sp = val;
			break;
		case 1:
			bp = NULL;
			break;
		case 2:
			(*sp)--;
	}
	if(bp != NULL)
		dirty(bp);
	return(*sp);
}


BUFAREA *
getblk(bp,blk)
register daddr_t blk;
register BUFAREA *bp;
{
	register struct filecntl *fcp;

	if(bp == NULL) {
		bp = search(blk);
		fcp = &sfile;
	}
	else
		fcp = &dfile;
	if(bp->b_bno == blk)
		return(bp);
	if(blk == SUPERB && fcp == &dfile) {
		flush(fcp,bp);

#ifdef i386
		if(lseek(fcp->rfdes,(long)SUPERBOFF,0L) < 0)
#else
		if(lseek(fcp->rfdes,(long)SUPERBOFF,0) < 0)
#endif

			rwerr("SEEK",blk);
		else if(read(fcp->rfdes,bp->b_un.b_buf,SBSIZE) == SBSIZE) {
			bp->b_bno = blk;
			return(bp);
		}
		rwerr("READ",blk);
		bp->b_bno = (daddr_t)-1;
		return(NULL);
	}
	flush(fcp,bp);
	if(bread(fcp,bp->b_un.b_buf,blk,F_BSIZE) != NO) {
		bp->b_bno = blk;
		return(bp);
	}
	bp->b_bno = (daddr_t)-1;
	return(NULL);
}


flush(fcp,bp)
struct filecntl *fcp;
register BUFAREA *bp;
{
	if(bp->b_dirty) {
		if(bp->b_bno == SUPERB && fcp == &dfile) {
			if(fcp->wfdes < 0) {
				bp->b_dirty = 0;
				return;
			}

#ifdef i386
			if(lseek(fcp->wfdes,(long)SUPERBOFF,0) < 0L)
#else
			if(lseek(fcp->wfdes,(long)SUPERBOFF,0) < 0)
#endif

				rwerr("SEEK",bp->b_bno);
			else if(write(fcp->wfdes,bp->b_un.b_buf,SBSIZE) == SBSIZE) {
				fcp->mod = 1;
				bp->b_dirty = 0;
				return;
			}
			rwerr("WRITE",SUPERB);
			bp->b_dirty = 0;
			return;
		}
		bwrite(fcp,bp->b_un.b_buf,bp->b_bno,F_BSIZE);
	}
	bp->b_dirty = 0;
}

rwerr(s,blk)
char *s;
daddr_t blk;
{
	printf("\n%c %sCAN NOT %s: BLK %ld",id,devname,s,blk);
	if(reply("CONTINUE", "EXITING", 0) == NO)
		errexit("%c %sProgram terminated\n",id,devname);
}



ckfini()
{
	flush(&dfile,&fileblk);
	flush(&dfile,&sblk);
	flush(&dfile,&inoblk);
	close(dfile.rfdes);
	close(dfile.wfdes);
	close(sfile.rfdes);
	close(sfile.wfdes);
#ifndef STANDALONE
	if(rmscr) {
		unlink(scrfile);
	}
#endif
}


pinode()
{
	register DINODE *dp;
	register char *p;
	char buf[100];
	char uidbuf[200];
	char *ctime();

	buf[99] = '\0';
	printf(" I=%u ",inum);
	if (fcorr_flg)
		fcorr_path(inum);
	if((dp = ginode()) == NULL)
		return;
	printf(" OWNER=");
	if(getpw((int)dp->di_uid,uidbuf) == 0) {
		for(p = uidbuf; *p != ':'; p++);
		*p = 0;
		printf("%s ",uidbuf);
	}
	else {
		printf("%d ",dp->di_uid);
	}
	printf("MODE=%o\n",dp->di_mode);
	printf("%c %sSIZE=%ld ",id,devname,dp->di_size);
	strftime(buf, sizeof(buf) -1, "%b %e %H:%M %Y", localtime(&dp->di_mtime));
	printf("MTIME=%s\n", buf);
}


copy(fp,tp,size)
register char *tp, *fp;
MEMSIZE size;
{
	while(size--)
		*tp++ = *fp++;
}


freechk()
{
	register daddr_t *ap;

	if(freeblk.df_nfree == 0)
		return;
	do {
		if(freeblk.df_nfree <= 0 || freeblk.df_nfree > NICFREE) {
			printf("%c %sBAD FREEBLK COUNT\n",id,devname);
			fixfree = YES;
			return;
		}
		ap = &freeblk.df_free[freeblk.df_nfree];
		while(--ap > &freeblk.df_free[0]) {
			if(pass5(*ap) == STOP)
				return;
		}
		if(*ap == (daddr_t)0 || pass5(*ap) != KEEPON)
			return;
	} while(getblk(&fileblk,*ap) != NULL);
}


makefree()
{
	register i, cyl, step;
	int j;
	char flg[MAXCYL];
	short addr[MAXCYL];
	daddr_t blk, baseblk;

	superblk.s_nfree = 0;
	superblk.s_flock = 0;
	superblk.s_fmod = 0;
	superblk.s_tfree = 0;
	superblk.s_ninode = 0;
	superblk.s_ilock = 0;
	superblk.s_ronly = 0;
	if(cylsize == 0 || stepsize == 0) {
		step = superblk.s_dinfo[0];
		cyl = superblk.s_dinfo[1];
	}
	else {
		step = stepsize;
		cyl = cylsize;
	}
	if(step > cyl || step <= 0 || cyl <= 0 || cyl > MAXCYL) {
		error("%c %sDefault free list spacing assumed\n",id,devname);
		step = STEPSIZE;
		cyl = CYLSIZE;
	}
	superblk.s_dinfo[0] = step;
	superblk.s_dinfo[1] = cyl;
	clear(flg,sizeof(flg));

	step >>= (F_BSHIFT-9);
	cyl >>= (F_BSHIFT-9);

	i = 0;
	for(j = 0; j < cyl; j++) {
		while(flg[i])
			i = (i + 1) % cyl;
		addr[j] = i + 1;
		flg[i]++;
		i = (i + step) % cyl;
	}
	baseblk = (daddr_t)roundup(fmax,cyl);
	clear(&freeblk,F_BSIZE);
	freeblk.df_nfree++;
	for( ; baseblk > 0; baseblk -= cyl)
		for(i = 0; i < cyl; i++) {
			blk = baseblk - addr[i];
			if(!outrange(blk) && !getbmap(blk)) {
				superblk.s_tfree++;
				if(freeblk.df_nfree >= NICFREE) {
					fbdirty();
					fileblk.b_bno = blk;
					flush(&dfile,&fileblk);
					clear(&freeblk,F_BSIZE);
				}
				freeblk.df_free[freeblk.df_nfree] = blk;
				freeblk.df_nfree++;
			}
		}
	superblk.s_nfree = freeblk.df_nfree;
	for(i = 0; i < NICFREE; i++)
		superblk.s_free[i] = freeblk.df_free[i];
	sbdirty();
}


clear(p,cnt)
register char *p;
MEMSIZE cnt;
{
	while(cnt--)
		*p++ = 0;
}


BUFAREA *
search(blk)
daddr_t blk;
{
	register BUFAREA *pbp, *bp;

	for(bp = (BUFAREA *) &poolhead; bp->b_next; ) {
		pbp = bp;
		bp = pbp->b_next;
		if(bp->b_bno == blk)
			break;
	}
	pbp->b_next = bp->b_next;
	bp->b_next = poolhead;
	poolhead = bp;
	return(bp);
}


findino(dirp)
register DIRECT *dirp;
{
	register char *p1, *p2;

	if(dirp->d_ino == 0)
		return(KEEPON);
	for(p1 = dirp->d_name,p2 = srchname;*p2++ == *p1; p1++) {
		if(*p1 == 0 || p1 == &dirp->d_name[DIRSIZ-1]) {
			if(dirp->d_ino >= S5ROOTINO && dirp->d_ino <= imax)
				parentdir = dirp->d_ino;
			return(STOP);
		}
	}
	return(KEEPON);
}


mkentry(dirp)
register DIRECT *dirp;
{
	register ino_t in;
	register char *p;

	if(dirp->d_ino)
		return(KEEPON);
	dirp->d_ino = orphan;
	in = orphan;
	p = &dirp->d_name[DIRSIZ];
	while(p != &dirp->d_name[6])
		*--p = 0;
	while(p > dirp->d_name) {
		*--p = (in % (ino_t)10) + '0';
		in /= (ino_t)10;
	}
	return(ALTERD|STOP);
}


chgdd(dirp)
register DIRECT *dirp;
{
	if(dirp->d_name[0] == '.' && dirp->d_name[1] == '.' &&
	dirp->d_name[2] == 0) {
		dirp->d_ino = lfdir;
		return(ALTERD|STOP);
	}
	return(KEEPON);
}


linkup()
{
	register DINODE *dp;
	register lostdir;
	register ino_t pdir;
	register ino_t *blp;
	int n;

	if((dp = ginode()) == NULL)
		return(NO);
	lostdir = DIR;
	pdir = parentdir;
	if(!FIFO || !qflag || nflag) {
		printf("%c %sUNREF %s ",id,devname,lostdir ? "DIR" : "FILE");
		pinode();
	}
	if(DIR) {
		if(dp->di_size > EMPT) {
			if((n = chkempt(dp)) == NO) {
				printf(" (NOT EMPTY)");
				if(!nflag) {
					printf(" MUST reconnect\n");
					goto connect;
				}
				else
					printf("\n");
			}
			else if(n != SKIP) {
				printf(" (EMPTY)");
				if(!nflag) {
					printf(" Cleared\n");
					return(REM);
				}
				else
					printf("\n");
			}
		}
		else {
			printf(" (EMPTY)");
			if(!nflag) {
				printf(" Cleared\n");
				return(REM);
			}
			else
				printf("\n");
		}
	}
	if(REG)
		if(!dp->di_size) {
			printf(" (EMPTY)");
			if(!nflag) {
				printf(" Cleared\n");
				return(REM);
			}
			else
				printf("\n");
		}
		else
			printf(" (NOT EMPTY)\n");
	if(FIFO && !nflag) {
		if(!qflag)	printf(" -- REMOVED");
		printf("\n");
		return(REM);
	}
	if(FIFO && nflag)
		return(NO);
	if(reply("RECONNECT", "RECONNECTED", 1) == NO)
		return(NO);
connect:
	orphan = inum;
	if(lfdir == 0) {
		inum = S5ROOTINO;
		if((dp = ginode()) == NULL) {
			inum = orphan;
			return(NO);
		}
		pfunc = findino;
		srchname = lfname;
		filsize = dp->di_size;
		parentdir = 0;
		ckinode(dp,DATA);
		inum = orphan;
		if((lfdir = parentdir) == 0) {
			printf("%c %sSORRY. NO lost+found DIRECTORY\n\n",id,devname);
			return(NO);
		}
	}
	inum = lfdir;
	if((dp = ginode()) == NULL || !DIR || getstate() != FSTATE) {
		inum = orphan;
		printf("%c %sSORRY. NO lost+found DIRECTORY\n\n",id,devname);
		return(NO);
	}
	if(dp->di_size & F_BMASK) {
		dp->di_size = roundup(dp->di_size,F_BSIZE);
		inodirty();
	}
	filsize = dp->di_size;
	inum = orphan;
	pfunc = mkentry;
	if((ckinode(dp,DATA) & ALTERD) == 0) {
		printf("%c %sSORRY. NO SPACE IN lost+found DIRECTORY\n\n",id,devname);
		return(NO);
	}
	declncnt();
	if((dp = ginode()) && !dp->di_nlink) {
		dp->di_nlink++;
		inodirty();
		setlncnt(getlncnt()+1);
		if(lostdir) {
			for(blp = badlncnt; blp < badlnp; blp++)
				if(*blp == inum) {
					*blp = 0L;
					break;
				}
		}
	}
	if(lostdir) {
		pfunc = chgdd;
		filsize = dp->di_size;
		ckinode(dp,DATA);
		inum = lfdir;
		if((dp = ginode()) != NULL) {
			dp->di_nlink++;
			inodirty();
			setlncnt(getlncnt()+1);
		}
		inum = orphan;
		printf("%c %sDIR I=%u CONNECTED. ",id,devname,orphan);
		printf("%c %sPARENT WAS I=%u\n\n",id,devname,pdir);
	}
	return(YES);
}


bread(fcp,buf,blk,size)
daddr_t blk;
register struct filecntl *fcp;
register MEMSIZE size;
char *buf;
{

#ifdef i386
	if(lseek(fcp->rfdes,blk<<F_BSHIFT,0) < 0L)
#else 
	if(lseek(fcp->rfdes,blk<<F_BSHIFT,0) < 0)
#endif

		rwerr("SEEK",blk);
	else if(read(fcp->rfdes,buf,size) == size)
		return(YES);
	rwerr("READ",blk);
	return(NO);
}


bwrite(fcp,buf,blk,size)
daddr_t blk;
register struct filecntl *fcp;
register MEMSIZE size;
char *buf;
{
	if(fcp->wfdes < 0)
		return(NO);

#ifdef i386
	if(lseek(fcp->wfdes,blk<<F_BSHIFT,0) < 0L)
#else
	if(lseek(fcp->wfdes,blk<<F_BSHIFT,0) < 0)
#endif

		rwerr("SEEK",blk);
	else if(write(fcp->wfdes,buf,size) == size) {
		fcp->mod = 1;
		return(YES);
	}
	rwerr("WRITE",blk);
	return(NO);
}

#ifndef STANDALONE
void
catch(dmy)
int dmy;
{
	ckfini();
	exit(31+4);
}
#endif

#ifdef MB1
checksb(dev)
char *dev;
{
	if((dfile.rfdes = open(dev,O_RDONLY)) < 0) {
		error("%c %sCan't open %s\n",id,devname,dev);
		return(NO);
	}
	/*  nflag alone means no write.
	    nflag with csflag (-S) means answer NO to all, but open
	    for write to rebuild the freelist  */
	if((nflag && !csflag) || (dfile.wfdes = open(dev,O_WRONLY)) < 0)
		dfile.wfdes = -1;
	if(getblk(&sblk,SUPERB) == NULL) {
		ckfini();
		return(NO);
	}
return(YES);
}
#endif

daddr_t
dbconv(i,dp)
int i;
DINODE *dp;
{

	union {
	char a[4];
	daddr_t n;
	} val;

#ifdef i386
	val.a[0]=dp->di_addr[i*3];
	val.a[1]=dp->di_addr[i*3 + 1];
	val.a[2]=dp->di_addr[i*3 + 2];
	val.a[3]='\0';
#else
	val.a[0]='\0';
	val.a[1]=dp->di_addr[i*3];
	val.a[2]=dp->di_addr[i*3 + 1];
	val.a[3]=dp->di_addr[i*3 + 2];
#endif
	return(val.n);
}



dexcess(dp,fsize)
register DINODE *dp;
daddr_t fsize;
{
	int bn,i,free;

	free=0;
	for (i= NADDR-1;i>=0;i--)
		   switch(i){
		case 10:
		case 11:
		case 12:if (fsize >= dp->di_size && (fsize - F_BSIZE) < dp->di_size)
				break;
			if ((bn=dbconv(i,dp)) != 0){
				freei(bn,(i-9),&fsize,dp->di_size,&free);
				if (!(fsize >= dp->di_size && (fsize - F_BSIZE) < dp->di_size) || fsize == S1 || fsize == S2 || fsize == S3 ){
					blfree(bn);
					dp->di_addr[i*3]='\0';
					dp->di_addr[i*3 + 1]='\0';
					dp->di_addr[i*3 + 2]='\0';
				}
			}
			 break;
		default: if (fsize >= dp->di_size && (fsize - F_BSIZE) < dp->di_size)
				break;
			 if ((bn=dbconv(i,dp)) != 0){
				blfree(bn);
				dp->di_addr[i*3]='\0';
				dp->di_addr[i*3 + 1]='\0';
				dp->di_addr[i*3 + 2]='\0';
				fsize -= F_BSIZE;
			 }
			 break;
		}
}



freei(bn,lev,fsize,disize,free)
register int bn,lev;
int *fsize,disize,*free;
{
	BUFAREA tmp;
	int i,lbn;

	getblk(&tmp,bn);
	lbn=bn;
	for(i=F_NINDIR-1;i>=0;i--){
		if (*fsize >= disize && (*fsize - F_BSIZE) < disize)
			break;
		if (lev == 1 && *free == 1) 
			*fsize -= F_BSIZE;
		if ((bn=tmp.b_un.b_indir[i]) != 0){
			if (lev > 1){
				freei(bn,lev-1,fsize,disize,free);
				if (*fsize >= disize && (*fsize - F_BSIZE) < disize)
					break;
			}
			blfree(bn);
			if (*free == 0) 
				*fsize -= F_BSIZE;
			*free = 1;
			tmp.b_un.b_indir[i] = 0;
		}
	}
	bwrite(&dfile,tmp.b_un.b_buf,lbn,F_BSIZE);
}



blfree(bn)
register int bn;
{
	int i;
	BUFAREA tmp;

	if (superblk.s_nfree >= NICFREE){
		getblk(&tmp,bn);
		for (i=0;i < NICFREE;i++)
			tmp.b_un.b_fb.df_free[i] = superblk.s_free[i];
		tmp.b_un.b_fb.df_nfree = superblk.s_nfree;
		bwrite(&dfile,tmp.b_un.b_buf,bn,F_BSIZE);
		superblk.s_nfree=0;
		}
	superblk.s_free[superblk.s_nfree++] = bn;
	superblk.s_tfree++;
	sbdirty();
	n_blks--;
	clrbmap(bn);
}



daddr_t
naddr(bn,DSIZE,fsize)
daddr_t bn,DSIZE,*fsize;

{
	BUFAREA tmp;
	int i,start;

	initbarea (&tmp);
	getblk(&tmp,bn);
	if (DSIZE == S3)
		start=F_NUMTRIPLE;
	else
		start=F_NINDIR;
	for (i=start-1; i > -1; i--)
		if (tmp.b_un.b_indir[i] == 0){
			*fsize -= DSIZE;
			if ( *fsize + DSIZE == MAX_FILE)
					*fsize +=1;
		}
		else
			break;
	return(tmp.b_un.b_indir[i]);
}



daddr_t
triple(found,dp)
int *found;
register DINODE *dp;
{
	daddr_t bn,fsize;

	if ((bn=dbconv(12,dp)) != 0){
		*found=1;
		fsize= S1 + S2 + S3 + S4;
		bn=naddr(bn,S3,&fsize);
		bn=naddr(bn,S2,&fsize);
		bn=naddr(bn,F_BSIZE,&fsize);
		return(fsize);
		}
	return(0);
}



daddr_t
dble(found,dp)
int *found;
register DINODE *dp;
{
	daddr_t bn,fsize;

	if ((bn=dbconv(11,dp)) != 0){
		*found=1;
		fsize= S1 + S2 + S3;
		bn=naddr(bn,S2,&fsize);
		bn=naddr(bn,F_BSIZE,&fsize);
		return(fsize);
		}
	return(0);
}



daddr_t
single(found,dp)
int *found;
register DINODE *dp;
{
	daddr_t bn,fsize;

	if ((bn=dbconv(10,dp)) != 0){
		*found=1;
		fsize= S1 + S2;
		bn=naddr(bn,F_BSIZE,&fsize);
		return(fsize);
		}
	return(0);
}



daddr_t
direct(i,found,dp)
int i,*found;
register DINODE *dp;
{

	if (dbconv(i,dp) != 0){
			*found =1;
			return((i+1) * F_BSIZE);
			}
	return(0);
}



sizechk(dp)
register DINODE *dp;
{
	int found,i;
	daddr_t fsize;

	found=0;
	for ( i=NADDR-1; i >= 0 && !found; i--)
		switch(i){
		case 12: 
			 fsize=triple(&found,dp);
			 break;
		case 11: 
			 fsize=dble(&found,dp);
			 break;
		case 10: 
		  	 fsize=single(&found,dp);
			 break;
		default: 
			 fsize=direct(i,&found,dp);
			 break;
		}

	if(DIR && (dp->di_size % sizeof(DIRECT)) != 0) {
		error("%c %sDIRECTORY MISALIGNED I=%u\n",id,devname,inum);
		if (fcorr_flg)
			fcorr_path(inum);
		if (reply("RECOVER", "RECOVERED", 1) == YES){
			dp->di_size=fsize;
			inodirty();
			return;
			}
	}

	if ( fsize >= dp->di_size && (fsize -F_BSIZE) < dp->di_size)
			return;
		else
			{
			if (dp->di_size <= (fsize - F_BSIZE)) {
				error("%s SIZE ERROR I=%u\n",
					DIR ? "DIR":"FILE",inum);
				if (fcorr_flg)
					fcorr_path(inum);
				printf("DELETE OR RECOVER EXCESS DATA\n");
				if (reply("RECOVER EXCESS BLOCKS", "RECOVERED EXCESS BLOCKS", 1) == YES){
					dp->di_size=fsize;
					inodirty();
				}
				else
					if (reply("DELETE EXCESS BLOCKS", "DELETED EXCESS BLOCKS", 1) == YES){
						dexcess(dp,fsize);
						inodirty();
					}
			}
			}
}



#if u3b15 || u3b2 || i386

/* Prompt user for block size and return block size identifier. */
prmptbsize(fs)
char	*fs;
{
    char	bsize[MAXPATHLEN];
    char	fsname[MAXPATHLEN];

    if (id != ' ') {		/* if  we  were invoked through dfsck */
	strcpy(fsname, fs);
	strcat(fsname, "\t");
    }
    else
	fsname[0] = '\0';


    if ( yflag || nflag ) {
	printf("%c %sWARNING: SUPER BLOCK, ROOT INODE, OR ROOT\n", id, fsname);
	printf("%c %sDIRECTORY MAY BE CORRUPTED. fsck CAN'T DETERMINE\n", id, fsname);
	printf("%c %sLOGICAL BLOCK SIZE OF %s.\n", id, fsname, fs);
	printf("%c %sBLOCK SIZE COULD BE 512, 1024, OR 2048 BYTES.\n", id, fsname);
	printf("%c %sRE-RUN fsck WITHOUT -y OR -n OPTION AND YOU\n", id, fsname);
	printf("%c %sWILL BE PROMPTED FOR BLOCK SIZE.\n", id, fsname);
	errexit("\n");
    }
    else if (id != ' ') {
	printf("%c %sWARNING: SUPER BLOCK, ROOT INODE, OR ROOT\n", id, fsname);
	printf("%c %sDIRECTORY MAY BE CORRUPTED. fsck CAN'T DETERMINE\n", id, fsname);
	printf("%c %sLOGICAL BLOCK SIZE OF %s.\n", id, fsname, fs);
	printf("%c %sBLOCK SIZE COULD BE 512, 1024, OR 2048 BYTES.\n", id, fsname);
	printf("%c %sRE-RUN fsck FOR %s WITHOUT\n", id, fsname, fs);
	printf("%c %sINVOKING THROUGH dfsck AND YOU WILL BE PROMPTED\n", id, fsname);
	printf("%c %sFOR BLOCK SIZE.\n", id, fsname);
	errexit("\n");
    }
    else {
	printf("WARNING: SUPER BLOCK, ROOT INODE, OR ROOT DIRECTORY ON %s MAY\n", fs);
	printf("BE CORRUPTED. fsck CAN'T DETERMINE LOGICAL BLOCK SIZE OF %s\n", fs);
	printf("BLOCK SIZE COULD BE 512, 1024, OR 2048 BYTES.\n\n");
	printf("ENTER LOGICAL BLOCK SIZE OF %s IN BYTES (NOTE: INCORRECT\n", fs);
	printf("RESPONSE COULD DAMAGE FILE SYSTEM BEYOND REPAIR!)\n");
	printf("ENTER 512, 1024, OR 2048 OR ENTER s TO SKIP THIS FILE SYSTEM:  ");
	gets(bsize);
	while (strcmp(bsize, "s") != 0 &&
	       strcmp(bsize, "512") != 0 &&
	       strcmp(bsize, "1024") != 0 &&
	       strcmp(bsize, "2048") != 0) {
			printf("\nENTER 512, 1024, 2048, OR s: ");
			gets(bsize);
	}
	if (strcmp(bsize, "512") == 0)
		return(Fs1b);
	if (strcmp(bsize, "1024") == 0)
		return(Fs2b);
	if (strcmp(bsize, "2048") == 0)
		return(Fs4b);
	return(-1);
    }
/* NOTREACHED */
}
#endif


#ifndef i386
/* heuristic function to determine logical block size of System V file system */

s5bsize(fd)
int fd;
{

	int results[3];
	int count;
	long address;
	long offset;
	char *buf;
	struct dinode *inodes;
	struct direct *dirs;
	char * p1;
	char * p2;
	
	results[1] = 0;
	results[2] = 0;

	buf = (char *)malloc(PHYSBLKSZ);

	for (count = 1; count < 3; count++) {

		address = 2048 * count;
		if (lseek(fd, address, 0) != address)
			continue;
		if (read(fd, buf, PHYSBLKSZ) != PHYSBLKSZ)
			continue;
		inodes = (struct dinode *)buf;
		if ((inodes[1].di_mode & S_IFMT) != S_IFDIR)
			continue;
		if (inodes[1].di_nlink < 2)
			continue;
		if ((inodes[1].di_size % sizeof(struct direct)) != 0)
			continue;
	
		p1 = (char *) &address;
		p2 = inodes[1].di_addr;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1   = *p2;
	
		offset = address << (count + 9);
		if (lseek(fd, offset, 0) != offset)
			continue;
		if (read(fd, buf, PHYSBLKSZ) != PHYSBLKSZ)
			continue;
		dirs = (struct direct *)buf;
		if (dirs[0].d_ino != 2 || dirs[1].d_ino != 2 )
			continue;
		if (strcmp(dirs[0].d_name,".") || strcmp(dirs[1].d_name,".."))
			continue;
		results[count] = 1;
		}
	free(buf);
	
	if(results[1])
		return(Fs2b);
	if(results[2])
		return(Fs4b);
	return(-1);
}
#endif



/*
 * exit 0 - file system is unmounted and okay
 * exit 32 - file system is unmounted and needs checking
 * exit 33 - file system is mounted
 *          for root file system
 * exit 0 - okay
 * exit 32 - needs checking
 * exit 34 - cannot stat device
 */

check_sanity(filename)
char	*filename;
{
	struct stat stbd, stbr;
	struct ustat usb;

	if (stat(filename, &stbd) < 0) {
		fprintf(stderr, "s5 fsck: sanity check failed : cannot stat %s\n", filename);
		exit(34);
	}
	stat("/", &stbr);
	if (stbr.st_dev == stbd.st_rdev) {	/* root file system */
		if (superblk.s_state != FsACTIVE) {
			fprintf(stderr, "s5 fsck: sanity check: root file system needs checking\n");
			exit(32);
		} else {
			fprintf(stderr, "s5 fsck: sanity check: root file system okay\n");
			exit(0);
		}
	}
	if (ustat(stbd.st_rdev, &usb) == 0) {
		fprintf(stderr, "s5 fsck: sanity check: %s already mounted\n", filename);
		exit(33);
	}

	if ((superblk.s_state + (long)superblk.s_time) != FsOKAY) {
		fprintf(stderr, "s5 fsck: sanity check: %s needs checking\n", filename);
		exit(32);
	}
	fprintf(stderr, "s5 fsck: sanity check: %s okay\n", filename);
	exit(0);
}

extern int strcmp();
extern char *strchr();

int opterr = 1, optind = 1, optopt;
char *optarg;

int
getopt(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(-1);
		else if(strcmp(argv[optind], "--") == 0) {
			optind++;
			return(-1);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == 0) {
		if (opterr)
			fprintf(stderr, "%s: illegal option -- %c\n", *argv, c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			if (opterr)
				fprintf(stderr, "%s: option requires an argument -- %c\n", *argv, c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else if (*cp == ';') {
		if (argv[optind][++sp] != '\0')
			if (isoptarg(c, &argv[optind][sp])) {
				optarg = &argv[optind++][sp];
				sp = 1;
			} else
				optarg = NULL;
		else {
			sp = 1;
			if (++optind >= argc || !isoptarg(c, &argv[optind][0]))
				optarg = NULL;
			else
				optarg = argv[optind++];
		}
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

isoptarg(cc, arg)
	int	cc;
	char	*arg;
{
	if (cc == 's' || cc == 'S') {
		if (*(arg+1) == 0) {
			if (*arg == '3' || *arg == '4')
				return	1;
		}
		while(*arg >= '0' && *arg <= '9')
			arg++;
		if(*arg++ != ':')
			return	0;
		while(*arg >= '0' && *arg <= '9')
			arg++;
		if (*arg)
			return	0;
		return	1;
	}
	return	0;
}

fcorr_path(inode_num)
int inode_num;
{
	static int first_time = 1;
	char *pathptr;
	struct patherr *pptr;

	/* get new node */
	if ((pptr = (struct patherr *) malloc(sizeof(patherr))) == NULL) {
		printf("Failure to allocate new node in fcorr_path\n");
		return;
	}

	/* if arg to this function is non-zero, use inode number */
	/* else use global value of pathname for this node */
	if (inode_num == 0) {
		/* save current_pathname */
		if ((pathptr = (char *)malloc(strlen(pathname) + 1)) == NULL) {
			printf("Allocate of new pathname failed\n");
			return;
		}
		strcpy(pathptr, pathname);
		pptr->pname=pathptr;
		pptr->inode=0;
	}
	else
		pptr->inode = inode_num;

	pptr->next_ptr = (struct patherr *) NULL;
	if (first_time) {
		first_time = 0;
		root_ptr = pptr;
		cur_ptr = root_ptr;
		return;
	}
	cur_ptr->next_ptr=pptr;
	cur_ptr=pptr;
}

fcorr_dump(dev)
char *dev;
{
	char tmp_buf[8], ff_buf[1024];
	int i;
	int dev_fd;
	
	printf("Possibly damaged files found by fsck\n");
	sprintf(ff_buf, "/usr/sbin/ff -F s5 -I -i");

	/* next walk linked list and display saved pathnames or */
	/* stick inode number into command buffer for /usr/sbin/ff */

	while (root_ptr) {
		if (root_ptr->inode) {
			sprintf(tmp_buf, "%d,", root_ptr->inode);
			strcat(ff_buf, tmp_buf);
		}
		else
			printf(".%s\n", root_ptr->pname);
		root_ptr = root_ptr->next_ptr;
	}
	strcat(ff_buf, " ");
	strcat(ff_buf, dev);
/* close stdout here as s5 ff prints out header that contains 's5 ff:' to stderr*/

	close(2);
	dev_fd=open("/dev/null",O_WRONLY);
	system(ff_buf);
	close(dev_fd);
	open("/dev/tty",O_RDWR);
	printf("\n");
}

addpath(p, len)
	register char	*p;
{
	if (maxpath - pathp < len+1) {
		maxpath += 512;
		if (realloc(pathname, maxpath) == NULL) {
			printf("%c %sDIR pathname too deep\n",id,devname);
			printf("%c %sDIR pathname is <%s>\n",
			id,devname,pathname);
			ckfini();
			exit(31+4);
		}
	}
	while (len && *p) {
		pathname[pathp++] = *p++;
		len--;
	}
	pathname[pathp] = 0;
}
