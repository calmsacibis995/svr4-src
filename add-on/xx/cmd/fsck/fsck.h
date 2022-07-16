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

#ident	"@(#)xx:cmd/fsck/fsck.h	1.2.1.1"

/*
 *      Derived from XENIX fsck.h 1.5 87/02/09
 */
/*      fsck - file system checker
 *
 *
 *      memory requirements:    (approx)
 *              1 bit per block in the file system      +
 *              2 bits per inode in the file system     +
 *              2 bytes per inode in the file system
 *
 *              Things are more complicated if there are more than 16
 *              disk blocks for every inode in the structure.
 *
 *              If the space is not available in memory, all the structures
 *              are allocated from a disk file.
 */

/*      Modification History
 *
 *      M001    07/13/81        JGL
 *              added some register declarations
 *
 *      M002    7/23/81         JJD
 *              corrected bug wherin names added to 'lost+found' directory
 *              didnt zero the rest of the directory entry.  Garbage
 *              after the trailing zero makes the directory entry impossible
 *              to open.
 *
 *	M003	1/21/82		JJD
 *		split up into multiple source files so that it can be
 *		compiled by cc68/a68.
 *
 *	M004	1/22/82		JJD
 *		changed MAXDATA on 68000 to be 1/4 megabyte
 *      M005    8 Feb 82        EA
 *              Added IFNAM to the file types recognized by fsck
 *              (for semaphores).
 *	M006	8/12/83		BAS
 *		3.0 Upgrade (uncommented)
 *		Lowered MAXDATA on 68000 by 36K bytes (220K) to allow
 *		fsck to grow its' stack.
 *		Check the version of the superblock and convert it to the
 *		version currently supported by the kernel.
 *		Converted the superblock back to its' previous version
 *		before updating it.
 *		Multilplexed files are still legal file types as long as
 *		the superblock is not in system 3 format.
 *		When the -n switch has been selected, a message will appear
 *		if the file system was dirty but not modified.
 *	M007	8/24/83		BAS
 *		If the -rr option is given and more than one file system is
 *		specified, then give an error message and exit.
 *		If the -rr option is given and the file system is not the
 *		root device, then ignore the -rr flag and continue cleaning
 *		the specified file system.
 *		Only clean the file system if a block device was specified
 *		and the file system is not mounted. NOTE - If the -n option
 *		is specified, fsck will continue regardless if the file system
 *		is mounted or not.
 *		The variable that specifies that the root file system is being
 *		checked (hotroot) is now set only if the device specified
 *		is a block device.				 
 *		A sleep of three seconds is no longer performed before 
 *		shutting down because sync now returns after the output
 *		buffers have been output.		
 *	M008	9/20/83		BAS
 *		Changed the algorithm for grabbing all of memory so that
 *		"memsize" cannot be negative.
 *	M009	10/24/83	BAS
 *		Reduced MAXDATA for the 8086 to less than 32K.  
 *		Sbrk was expecting a signed number and the number that
 *		was passed was larger than 32K (which has the sign
 *		bit turned on negative).  Sbrk recognizes a negative
 *		number as a request to give up it's stack and
 *		consequently fsck ran out of memory.
 *	M010	23 Nov 83	ADP
 *		- Fixed typo in diagnostic.
 *	M011	17 Apr 85	sco!jeffj
 *		Added some sys 5 extensions and enhancements.
 *		Some extra error checking and some options:
 *			-q quiet fsck. no Phase 1 size errors, unref'ed fifos
 *			   removed silently, superblock counts fixed auto
 *			   and free list automatically salvaged.
 *
 *			-D More complete directory checking for bad blocks.
 *
 *			-f fast.  Phases 1, 5, & 6 only.
 *	M012	28 Aug 85	sco!chapman
 *		- added support for autoboot recover device.
 *		if the -a flag is given write all output to /dev/recover
 *		as well as the console.
 *	M013	01 Oct 85	sco!chapman
 *		- removed "feature" that had fsck deliberatly panic
 *		when -rr'ing large roots if there was no -t flag.
 *	M014	05 Oct 85	sco!chapman
 *		- seek (on open) to last ascii character in recover device.
 *	M015	24 Sept. 86	katyb
 *		- increased size of table for inodes with zero links, i.e.
 *		changed MAXLNCNT from 20 to 50 for badlncnt[MAXLNCNT].
 *		- added third argument to shutdn(), AUTOFLAG from 
 *		/usr/include/sys/machdep.h, as required by merged kernel.
 *	M016	13 Oct 86	sco!chapman
 *		changed shutdn() calls to uadmin().
 */

#undef  FsTYPE
#define FsTYPE  2
#define M_I386  1

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/vnode.h>
#include <sys/fs/s5param.h>
#include <sys/fs/xxfilsys.h>
#include <sys/fs/s5dir.h>
#include <sys/fs/xxfblk.h>
#include <sys/fs/s5ino.h>
#include <sys/fs/s5inode.h>
#include <sys/stat.h>
/* #include <sys/uadmin.h>      */       /* M016 */
#include <ustat.h>
/* #include <sys/machdep.h>     */

#define ROOTINO S5ROOTINO       /* 5.3 */
#define NINDIR	(XXBSIZE / sizeof(daddr_t))
#define INOPB	16			/* XXBSIZE/sizeof(struct dinode */
#define BMASK	01777			/* XXBSIZE - 1 */
#define BSHIFT	10			/* log2(XXBSIZE) */
#define IFMPC	0x3000			/* multiplexed char special */
#define IFMPB	0x7000			/* multiplexed block special */
#define INOSHIFT 4			/* log2(INOPB) */

#define	itod(x)	(daddr_t)(((unsigned)(x)+(2*INOPB-1))>>INOSHIFT)
#define	itoo(x)	(int)(((unsigned)(x)+(2*INOPB-1))&(INOPB-1))

typedef	int	(*SIG_TYP)();

#define NDIRECT	(XXBSIZE/sizeof(struct direct))
#define SPERB	(XXBSIZE/sizeof(short))

#define NO	0
#define YES	1

#define	MAXDUP	10		/* limit on dup blks (per inode) */
#define	MAXBAD	10		/* limit on bad blks (per inode) */

#define STEPSIZE	9	/* default step for freelist spacing */
#define CYLSIZE		400	/* default cyl size for spacing */
#define MAXCYL		500	/* maximum cylinder size */

#define BITSPB	8		/* number bits per byte */
#define BITSHIFT	3	/* log2(BITSPB) */
#define BITMASK	07		/* BITSPB-1 */
#define LSTATE	2		/* bits per inode state */
#define STATEPB	(BITSPB/LSTATE)	/* inode states per byte */
#define USTATE	0		/* inode not allocated */
#define FSTATE	01		/* inode is file */
#define DSTATE	02		/* inode is directory */
#define CLEAR	03		/* inode is to be cleared */
#define SMASK	03		/* mask for inode state */
#define EMPT	32		/* size of empty directory M011 */


typedef struct dinode	DINODE;
typedef struct direct	DIRECT;

#define ALLOC	((dp->di_mode & IFMT) != 0)
#define DIR	((dp->di_mode & IFMT) == IFDIR)
#define REG	((dp->di_mode & IFMT) == IFREG)
#define BLK	((dp->di_mode & IFMT) == IFBLK)
#define CHR	((dp->di_mode & IFMT) == IFCHR)
#define MPC	((dp->di_mode & IFMT) == IFMPC)
#define MPB	((dp->di_mode & IFMT) == IFMPB)
#define FIFO	((dp->di_mode & IFMT) == IFIFO) /* M011 */

#ifdef IFNAM                                    /* M005 */
#define NAM     ((dp->di_mode & IFMT) == IFNAM)
#define SPECIAL (BLK || CHR || NAM)
#define V7_SPECIAL (BLK || CHR || MPC || MPB || NAM)	/* M006 */
#else
#define SPECIAL	(BLK || CHR)
#define V7_SPECIAL	(BLK || CHR || MPC || MPB)	/* M006 */
#endif

#define MAXPATH 200		/* max path nam len. M011 */
#define NINOBLK	11		/* num blks for raw reading */
#define MAXRAW	110		/* largest raw read (in blks) */
daddr_t	startib;		/* blk num of first in raw area */
unsigned niblk;			/* num of blks in raw area */

struct bufarea {
	struct bufarea	*b_next;		/* must be first */
	daddr_t	b_bno;
	union {
		char	b_buf[XXBSIZE];		/* buffer space */
		short	b_lnks[SPERB];		/* link counts */
		daddr_t	b_indir[NINDIR];	/* indirect block */
		struct filsys b_fs;		/* super block */
		struct fblk b_fb;		/* free block */
		struct dinode b_dinode[INOPB];	/* inode block */
		DIRECT b_dir[NDIRECT];		/* directory */
	} b_un;
	char	b_dirty;
};

typedef struct bufarea BUFAREA;

BUFAREA	inoblk;			/* inode blocks */
BUFAREA	fileblk;		/* other blks in filesys */
BUFAREA	sblk;			/* file system superblock */
BUFAREA	*poolhead;		/* ptr to first buffer in pool */

#define minsz(x,y)	( (x) > (y) ? (y) : (x) )
#define initbarea(x)	(x)->b_dirty = 0;(x)->b_bno = (daddr_t)-1
#define dirty(x)	(x)->b_dirty = 1
#define inodirty()	inoblk.b_dirty = 1
#define fbdirty()	fileblk.b_dirty = 1
#define sbdirty()	sblk.b_dirty = 1

#define freeblk		fileblk.b_un.b_fb
#define dirblk		fileblk.b_un.b_dir		/*M006*/
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

#define  MAXDATA ((MEMSIZE)400*1024)

#define	DUPTBLSIZE	100	/* num of dup blocks to remember */

daddr_t	duplist[DUPTBLSIZE];	/* dup block table */
daddr_t	*enddup;		/* next entry in dup table */
daddr_t	*muldup;		/* multiple dups part of table */

#define MAXLNCNT	50	/* num zero link cnts to remember */
ino_t	badlncnt[MAXLNCNT];	/* table of inos with zero link cnts */
ino_t	*badlnp;		/* next entry in table */

extern	FILE *rcv_out;			/*M012*/

extern	char *recover;			/*M012*/

char	aflag;	/*M012*/	/* inir is being invoked during auto boot */
char	sflag;			/* salvage free block list */
char	csflag;			/* salvage free block list (conditional) */
char	nflag;			/* assume a no response */
char	yflag;			/* assume a yes response */
char	tflag;			/* scratch file specified */
char    rrflag;                 /* special /etc/inir root recovery */
char	cflag;			/* convert file system to current version */
char	qflag;			/* less verbose flag */
char	Dirc;			/* extensive directory check */
char	fast;			/* fast check- dup blks and free list check */
char	rplyflag;		/* any questions asked? */
char	hotroot;		/* checking root device */
char	rawflg;			/* read raw device */
char	rmscr;			/* remove scratch file when done */
char	fixfree;		/* corrupted free list */
char	*membase;		/* base of memory we get */
char	*blkmap;		/* ptr to primary blk allocation map */
char	*freemap;		/* ptr to secondary blk allocation map */
char	*statemap;		/* ptr to inode state table */
char	*pathp;			/* pointer to pathname position */
char	*thisname;		/* ptr to current pathname component */
char	*srchname;		/* name being searched for in dir */
char	pss2done;		/* M011 do not check dir blks anymore */
char	pathname[MAXPATH];	/* M011 */
char	scrfile[80];
char	*lfname;
char	*checklist;

short	*lncntp;		/* ptr to link count table */

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
off_t	maxblk;			/* largest logical blk in file */
off_t	bmapsz;			/* num chars in blkmap */

daddr_t	smapblk;		/* starting blk of state map */
daddr_t	lncntblk;		/* starting blk of link cnt table */
daddr_t	fmapblk;		/* starting blk of free map */
daddr_t	n_free;			/* number of free blocks */
daddr_t	n_blks;			/* number of blocks used */
daddr_t	n_files;		/* number of files seen */
daddr_t	fmin;			/* block number of the first data block */
daddr_t	fmax;			/* number of blocks in the volume */

#define howmany(x,y)	(((x)+((y)-1))/(y))
#define roundup(x,y)	((((x)+((y)-1))/(y))*(y))
#define outrange(x)	(x < fmin || x >= fmax)
#define zapino(x)	clear((char *)(x),sizeof(DINODE))

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

/*#define	printf		cprint				/* M011 */
/*#define	fprintf		cfprint				/* M011 */

#define BBLK	2
#define DATA	1
#define ADDR	0
#define ALTERD	010
#define KEEPON	04
#define SKIP	02
#define STOP	01
#define REM	07

/* int     (*signal())();       */
long	lseek();
long	time();
DINODE	*ginode();
BUFAREA	*getblk();
BUFAREA	*search();
int	dirscan();
int	findino();
void	catch();
int	mkentry();
int	chgdd();
int	pass1();
int	pass1b();
int	pass2();
int	pass3();
int	pass4();
int	pass5();
int 	chkblk();


extern  dev_t	pipedev;	/* M011 is pipedev if stdin is pipe */
int	Fsver;			/* M006 Version of file system */

extern	int		cvtfs();
#undef getfs
extern	int		getfs();
extern	int		putfs();
extern	int		versfs();

/* from xenix std.h */
#define	SYSBSIZE	XXBSIZE		/* system block size */
#define	MULBSIZE	512		/* multiplier 'block' */
#define	SYSTOMUL(sysblk)	((sysblk) * (SYSBSIZE / MULBSIZE))
