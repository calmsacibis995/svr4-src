/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)xrestore:restor.c	1.3"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

/*
 *	@(#) restor.c 1.2 88/05/05 xrestore:restor.c
 */
/*
 * Revision history:
 *
 *	8/2/82 G. Harris
 *	Various bug fixes from V7 incorporated.  "getfile" was called with "d"
 *	instead of "ino" as its first argument ("d" is not used in restor [rRc])
 *	and a duplicate "curino = 0;" was deleted.  (For some reason, both
 *	these fixes were NOT in the Berkeley 4.1BSD restor!)
 *
 *	Various bug fixes from Berkeley incorporated.  Files are no longer
 *	skipped by using gethead(); instead, getfile with both extract
 *	functions being null() is used, which means that files which are
 *	themselves dumps(!) can be safely skipped.
 *	The free block and free inode count in the super block (s_tfree and
 *	s_tinode) are now correctly updated by the 'r' and 'R' functions.
 *
 *	Added the "restor c" keyword; it has the same syntax as "restor r",
 *	only it reads the dump tape to verify it after a dump.
 *	This check verifies that the tape has no I/O errors or bad checksums,
 *	that every file on the tape exists on the filesystem and every file
 *	listed as clear on the tape is clear on the filesystem, and that each
 *	inode on the tape is the same as the inode on the filesystem.
 *	If the modification time of the file on the tape and on the filesystem
 *	is the same, the data in the file on the tape and on the filesystem
 *	is also compared.  Also modified "dread" and the code that uses it;
 *	"dread" doesn't exit on an I/O error, it returns an error code.  All
 *	places that call "dread" check whether it gets an error; if it is
 *	a data block read on a verify, it just prints a message, otherwise
 *	it exits.
 *
 *	Added "restor T" keyword; same syntax as "restor t", only it prints
 *	a full listing of the dump tape (same as "dumpdir"; dumpdir was
 *	just a pile of code stolen from restor, so why bother with two
 *	programs).
 *
 *	Added "restor X" keyword; same syntax as "restor x", only when a file
 *	is restored it tries to put it into its "original" place (i.e., if
 *	restoring "usr/man/man1/ls.1" it tries to put it in usr/man/man1/ls.1;
 *	if a directory is specified, it tries to restore all of the files in
 *	that directory).  All directories needed to extract the file are
 *	created, and given the proper mode and ownership.
 *
 *	Added small define for non-separate-I&D machines.
 *
 *	"F" key added; takes an argument which is the file number on the first
 *	tape to skip to (if the dump is multi-reel, it assumes that the
 *	continuation of the dump on second and subsequent reels begins on the
 *	first file of those reels).
 *	Checks added for the "f" and "F" option to make sure there really is
 *	an argument for those options.
 *
 *	Modified to put the scratch file on "/tmp" instead of the current
 *	directory.
 *
 *	Handling of all 7000 variables which contain an i-number cleaned up.
 *
 *	Code which checks the bitmap fixed so that when it scans the bitmap,
 *	only those files which have not been restored already are counted
 *	as being on this tape.  Furthermore, the ONTAPE flag in the list of
 *	files to be extracted was not being used, so it was removed.
 *
 *	M001	23 Feb 84	barrys/andyp
 *	- Changed label 'done' to label 'Ldone'.  The scope of labels is
 *	  ambiguous in the C standard, and there is a procedure called done()
 *	  which could confuse things.
 *	M002	01 Apr 84	andyp
 *	- Get physio buffer using physalloc(3).
 *	M003	15 May 84	andyp
 *	- Size adjustments for data+bss+stack.
 *	M004	02 Jul 84	andyp
 *	- More physio fixes, missed some last time around.
 *	03/05/82	JGL	M006
 *		- Added -k switch for dumping to non-tape volumes
 *	M005	26 Dec 84	andyp
 *	- gets returns NULL, not EOF on error
 * 	M006	April 1, 1985	sco!rr
 * 	- We have to #include sys/types.h now since it 
 * 	  is no longer included in sys/param.h.
 *	M007	Jan 2, 1986	ericc
 *	- modified the 'X' option to extract special files properly.  However,
 *	  the 'x' option still creates zero length regular files; it is dubious
 *	  that special files named by their i-numbers are useful.
 *	M008	Feb 7, 1986	ericc
 *	- ensured that the 'r' and 'R' options zero out the di_size and
 *	  di_addr[] fields of the inodes of named pipes.  Note that di_addr[0]
 *	  is the device number for device special files !!
 *	M009	29 April 1985	sco!blf
 *		- The 'c' keyword was not referenced in main() yet listed
 *		  in the one of the two usage messages (tho not the manual
 *		  page), so try adding it back to main() to prevent the
 *		  "Bad key character" complaint... 
 *		- The 'c' keyword also didn't quite work correctly;
 *		  change the logic and make it work better.
 *		- Added 'C' keyword to do even more checking.
 *		- Make both usage messages the same (& readable).
 *		- Check that one of the key letters xXrRtT is given.
 *		- Improve random error messages (why do most go to stdout?).
 *		- Fflush(stdout) before reading from stdin.
  *	M010	Sep 21 85	sco!guy
 *		- added code to test for null file system argument and
 *		print appropriate error message
 *	M011	15 Jan 1987	sco!katyb
 *		- Changed error message in addent() to be more descriptive to
 *		the user: When there were too many valid file arguments for
 *		xtrlist[], the message used to be "...table overflow...", it
 *		is now "...too many files...".
 *      M012	1 May 1987	davidby
 *		- Merged SCO and MS versions, standardized use of cmd
 *		variable in printfs, renumbered merge notes, etc..
 *
 *	M013    AT&T - Removed r, R, c, C options. It is insane to
 *              allow a XENIX filesystem to be restored on top of
 *              a UNIX filesystem.
 */
#ifdef	M_SDATA
# define	SMALL
#endif

/*
 * For SMALL, we have a constraint on total data.  Thus we must limit
 * the total size of our tables.
 * For LARGE, we want to maximize (within reason) the number of elts
 * in each table.
 */
#ifdef SMALL
#define	MAXINO	1300	/* max # of directory inodes */
#define	MAXXTR	100	/* max # of xtract args */
#else
#define	MAXINO	5000
#define	MAXXTR	3000
#endif

#define DEFFILE	"/etc/default/xrestor"
#define BITS	8
#define NCACHE	3

#ifndef STANDALONE
#include <sys/types.h>
#include <sys/param.h>

#ifdef i386
#include <sys/fs/s5param.h>

/* New 1024 byte file system */
#define	BSIZE		1024		/* size of secondary block (bytes) */
#define SBUFSIZE	1024		/* system buffer size */
#define	BSHIFT		10		/* log2(BSIZE) */
#define	NINDIR		(BSIZE/sizeof(daddr_t))	/* BSIZE/sizeof(daddr_t) */
#define	BMASK		01777		/* BSIZE-1 */
#define INOPB		16		/* BSIZE/sizeof(struct dinode) */
#define INOSHIFT	4		/* log2(INOPB) */
#define	NMASK		0377		/* NINDIR-1 */
#define	NSHIFT		8		/* log2(NINDIR) */
#define NDPC		4		/* number of blocks per click */
#define Fs2BLK		0x8000		/* large block flag in bsize */
#endif

#include <stdio.h>
#include <string.h>
#include <signal.h>
#endif
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/fblk.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#include "dumprestor.h"

#ifndef ROOTINO
#define ROOTINO ((ino_t) 2)
#endif
#ifndef NADDR
#define NADDR 13
#endif

#define	MWORD(m,i) (m[(unsigned)(i-1)/MLEN])
#define	MBIT(i)	(1<<((unsigned)(i-1)%MLEN))
#define	BIS(i,w)	(MWORD(w,i) |=  MBIT(i))
#define	BIC(i,w)	(MWORD(w,i) &= ~MBIT(i))
#define	BIT(i,w)	(MWORD(w,i) & MBIT(i))

long	atol();

struct	filsys	sblock;
struct	spcl	spcl;

short	nowrite = 0;					/* M009 */
int	fi;
ino_t	maxi, curino;

long	ksize;
long	kbytes;

int	mt;
char	*magtape;
#ifdef STANDALONE
char	mbuf[50];
#endif
int	filenum = 1;

#ifndef STANDALONE
extern	int errno;					/* M009 */
daddr_t	seekpt;
int	df, ofile;
char	dirfile[] = "/tmp/rstXXXXXX";

/*
 * Table of all the directory inodes.
 */
struct inotab {
	ino_t	t_ino;		/* i-number */
	unsigned short t_mode;	/* mode and type of file */
	short	t_uid;		/* owner's user id */
	short	t_gid;		/* owner's group id */
	daddr_t	t_seekpt;	/* offset within directory file of the contents */
} inotab[MAXINO];
int	ipos;

struct arglist {
	ino_t	a_ino;
	char	*a_name;
} arglist[MAXXTR];
int	narg;

#define INUSE	1		/* slot in extract list is in use */
#define SEEN	2		/* inode was seen while reading tape */
struct xtrlist {
	ino_t	x_ino;
	char	x_flags;
} xtrlist[MAXXTR];
int	nxtr;

char	name[12];

char	drblock[BSIZE];
int	bpt;
#endif

int	eflag;

int	volno = 1;

ino_t	extrino;		/* i-number of file we are currently extracting */

struct dinode tino, dino;
daddr_t	taddr[NADDR];

daddr_t	curbno;

short	dumpmap[MSIZ];
short	clrimap[MSIZ];


int bct = NTREC+1;
/*char tbf[NTREC*BSIZE];*/ /* M004 */
char	*tbf;			/* M004 */

struct	cache {
	daddr_t	c_bno;
	int	c_time;
	char	c_block[BSIZE];
} cache[NCACHE];
int	curcache;
char	*physbuf;		/* M002 */	

char	prebuf[256];
char	nambuf[256];		/* pathname of file we restored */

char *malloc();
char *defread();
struct inotab *isearch();
daddr_t	lseek();
char *cmd;

main(argc, argv)
char *argv[];
{
	register char *cp;
	char command = '\0';					/* M009 */
	int done();

	cmd = argv[0];
	/* M002 physbuf; M004 tbf */
	if ( (physbuf = malloc( BSIZE )) == NULL ||
	  (tbf = malloc( NTREC * BSIZE )) == NULL ) {
		printf( "%s: cannot malloc physio buffers\n",cmd );
		exit( 1 );
	};

#ifndef STANDALONE
	if (defopen(DEFFILE) == 0) {
		magtape = strdup(defread("archive="));
	}
	mktemp(dirfile);
	if (argc < 2)						/* M009 */
		usage(cmd);					/* M009 */
	argv++;
	argc -= 2;
	for (cp = *argv++; *cp; cp++) {
		switch (*cp) {
		case '-':
			break;
		case 'f':
			if (argc == 0) {			/* M007... */
				printf("%s -f: Missing archive name\n", cmd);
				exit(1);
			}
			magtape = *argv++;
			argc--;
			break;
		case 'F':
			if (argc == 0) {			/* M007... */
				printf("%s -F: Missing tapefile number\n", cmd);
				exit(1);
			}
			filenum = atoi(*argv);			/* M007 */
			argc--;
			if (filenum <= 0) {			/* M007... */
				printf("%s -F: Bad tapefile number: %s\n", cmd, *argv);
				exit(1);
			}
			argv++;					/* M007 */
			break;
		case 'k':                                       /* M006 */
			if(argc == 0) {				/* M006 M007... */
				printf("%s -k: Missing volume size\n", cmd);
				exit(1);
			}
			ksize = atol(*argv++);            /* M006 */
			argc--;                         /* M006 */
			if (ksize <= 40L) {             /* M006 */
			    printf("Volume size (%ldK) too small\n", ksize);
			    exit(1);                    /* M006 */
			}                               /* M006 */
			break;

		case 'C':
		case 'c':
		case 'r':
		case 'R':				/* M013 */
			printf ("Option %s is not supported.\n", cp);
			usage(cmd);
				
		case 't':
		case 'T':
		case 'x':
		case 'X':
			command = *cp;
			break;
		default:
			printf("%s: Bad key character %c\n", cmd, *cp);
			usage(cmd);				/* M007 */
		}
	}
	if (command == '\0')					/* M007 */
		usage(cmd);					/* M007 */

	if (command == 'x' || command == 'X' || command == 'T') {
		if (signal(SIGINT, done) == SIG_IGN)
			signal(SIGINT, SIG_IGN);
		if (signal(SIGTERM, done) == SIG_IGN)
			signal(SIGTERM, SIG_IGN);

		df = creat(dirfile, 0666);
		if (df < 0) {
			printf("%s: Cannot create directory temporary %s\n",cmd, dirfile);
			exit(1);
		}
		close(df);
		df = open(dirfile, 2);
	}
	doit(command, argc, argv);
	if (command == 'x' || command == 'X' || command == 'T')
		unlink(dirfile);
	exit(0);
#else
	magtape = "tape";
	doit('r', 1, 0);
#endif
}

#ifndef STANDALONE					/* M009 begin... */
usage(name)
char *name;
{
	printf("Usage: %s {xX}[options] files...\n", name);
	printf("       %s {tT}[options]\n", name);
	printf("Options: [f archive] [F tapefile] [k volumesize]\n");
	exit(2);
}
#endif							/* ...end M009 */

doit(command, argc, argv)
char	command;
int	argc;
char	*argv[];
{
	extern char *ctime();
	register i, j, k;
	ino_t	ino;
#ifndef STANDALONE
	int	null(), printem();
	int	addent();
	int	fdget();
	int	xtrfile(), skip();
	int	vfyfile(), vfyskip();
#endif
	int	rstrfile(), rstrskip();
	struct dinode *ip, *ip1;

#ifndef STANDALONE
	if ((mt = open(magtape, 0)) < 0) {
		printf("%s: Cannot open dump volume %s\n", cmd, magtape);
		exit(1);
	}
	skipfiles(filenum - 1);	/* skip to "filenum"th file */
#else
	do {
		printf("Dump Medium? ");
		gets(mbuf);
		mt = open(mbuf, 0);
	} while (mt == -1);
	magtape = mbuf;
#endif
	switch(command) {
#ifndef STANDALONE
	case 't':
	case 'T':
		if (readhdr(&spcl) == 0) {
			printf("Volume is not a dump tape\n");
			exit(1);
		}
		printf("Dump   date: %s", ctime(&spcl.c_date));
		printf("Dumped from: %s", ctime(&spcl.c_ddate));
		if (command == 't')
			return;
		if (checkvol(&spcl, 1) == 0) {
			printf("Volume is not volume 1 of the dump\n");
			return;
		}
		pass1();	/* This sets the various maps on the way by */
		strcpy(prebuf, "");
		recurse(prebuf, (ino_t)ROOTINO, printem, null);
		return;
	case 'x':
	case 'X':
		if (readhdr(&spcl) == 0) {
			printf("Volume is not a dump volume\n");
			exit(1);
		}
		if (checkvol(&spcl, 1) == 0) {
			printf("Volume is not volume 1 of the dump\n");
			exit(1);
		}
		pass1();  /* This sets the various maps on the way by */
		nxtr = 0;
		i = 0;
		while (i < MAXXTR-1 && argc--) {
			if ((ino = psearch(*argv)) == 0 || BIT(ino, dumpmap) == 0) {
				printf("%s: not on the dump\n", *argv++);
				continue;
			}
			arglist[i].a_name = *argv;
			arglist[i].a_ino = ino;
			if (command == 'X') {
				strcpy(prebuf, *argv);
				recurse(prebuf, ino, addent, addent);
			} else
				addent(*argv, ino);
			argv++;
			i++;
		}
		narg = i;
		if( nxtr == 0 )		/* no files found, nothing to do */
			exit(-1);
newvol:
		flsht();
		close(mt);
getvol:
		printf("Mount desired dump volume: Specify volume #: ");
		fflush(stdout);					/* M009 */
		if (gets(tbf) == NULL)
			return;
		volno = atoi(tbf);
		if (volno <= 0) {
			printf("Volume numbers are positive numerics\n");
			goto getvol;
		}
		mt = open(magtape, 0);
		skipfiles(filenum - 1);	/* skip to "filenum"th file */
		if (readhdr(&spcl) == 0) {
			printf("Volume is not a dump volume\n");
			goto newvol;
		}
		if (checkvol(&spcl, volno) == 0) {
			printf("Wrong volume (%d)\n", spcl.c_volume);
			goto newvol;
		}
rbits:
		while (gethead(&spcl) == 0)
			;
		if (checktype(&spcl, TS_INODE) == 1) {
			printf("Can't find inode mask!\n");
			goto newvol;
		}
		if (checktype(&spcl, TS_BITS) == 0)
			goto rbits;
		readbits(dumpmap);
		i = 0;
		for (k = 0; xtrlist[k].x_flags; k++) {
			if (BIT(xtrlist[k].x_ino, dumpmap)) {
				if ((xtrlist[k].x_flags&SEEN) == 0)
					i++;
			}
		}
		while (i > 0) {
again:
			if (ishead(&spcl) == 0)
				while(gethead(&spcl) == 0)
					;
			if (checktype(&spcl, TS_END) == 1) {
				printf("End of dump\n");
checkdone:
				for (k = 0; xtrlist[k].x_flags; k++)
					if ((xtrlist[k].x_flags&SEEN) == 0)
						goto newvol;
					return;
			}
			if (checktype(&spcl, TS_INODE) == 0) {
				gethead(&spcl);
				goto again;
			}
			for (k = 0; xtrlist[k].x_flags; k++) {
				if (spcl.c_inumber == xtrlist[k].x_ino) {
					if (command == 'x') {
						printf("Extract file %u\n", xtrlist[k].x_ino);
						sprintf(name, "%u", xtrlist[k].x_ino);
						if ((ofile = creat(name, 0666)) < 0) {
							printf("%s: cannot create file\n", name);
							getfile(spcl.c_inumber, null, null, spcl.c_dinode.di_size);
						} else {
							chmod(name, spcl.c_dinode.di_mode&(~IFMT));
							chown(name, spcl.c_dinode.di_uid, spcl.c_dinode.di_gid);
							getfile(spcl.c_inumber, xtrfile, skip, spcl.c_dinode.di_size);
							close(ofile);
							utime(name,&(spcl.c_dinode.di_atime));
						}
					} else {
						strcpy(nambuf, "");
						extrino = spcl.c_inumber;
						for (j = 0; j < narg; j++) {
							strcpy(prebuf, arglist[j].a_name);
							recurse(prebuf, arglist[j].a_ino, fdget, fdget);
						}
						if (extrino == spcl.c_inumber)
							getfile(spcl.c_inumber, null, null, spcl.c_dinode.di_size);
					}
					xtrlist[k].x_flags |= SEEN;
					i--;
					goto Ldone;
				}
			}
			getfile(spcl.c_inumber, null, null, spcl.c_dinode.di_size);
Ldone:
			;
		}
		goto checkdone;
#endif
	case 'r':
	case 'R':
#ifndef STANDALONE
	case 'c':
	case 'C':						/* M009 */
		if ((fi = open(*argv, nowrite ? 0 : 2)) < 0) {	/* M009 M010 */
			*argv ? printf("%s: cannot open\n", *argv) :
				printf("%s: No file system specified\n", cmd);
			exit(1);
		}
#else
		do {
			char charbuf[50];

			printf("Disk? ");
			gets(charbuf);
			fi = open(charbuf, 2);
		} while (fi == -1);
#endif
#ifndef STANDALONE
		if (command == 'R') {
			printf("Enter starting volume number: ");
			fflush(stdout);				/* M009 */
			if (gets(tbf) == NULL) {	/*M005*/
				volno = 1;
				printf("\n");
			}
			else
				volno = atoi(tbf);
		}
		else
#endif
			volno = 1;
		if (!nowrite) {					/* M009 */
			printf("Last chance before scribbling on %s. ",
#ifdef STANDALONE
									"disk");
#else
									*argv);
			fflush(stdout);				/* M009 */
#endif
			while (getchar() != '\n');
		}
		if (dread((daddr_t)1, (char *)&sblock, sizeof(sblock)) < 0)
			exit(1);	/* can't restore or compare, superblock is bad */
		maxi = (sblock.s_isize-2)*INOPB;
		if (readhdr(&spcl) == 0) {
			printf("Missing volume record\n");
			exit(1);
		}
		if (checkvol(&spcl, volno) == 0) {
			printf("Dump volume is not volume %d\n", volno);
			exit(1);
		}
		gethead(&spcl);
		for (;;) {
ragain:
			if (ishead(&spcl) == 0) {
				printf("Missing header block\n");
				while (gethead(&spcl) == 0)
					;
				eflag++;
			}
			if (checktype(&spcl, TS_END) == 1) {
				printf("End of dump\n");
				close(mt);
				dwrite( (daddr_t) 1, (char *) &sblock);	/*M009*/
				return;
			}
			if (checktype(&spcl, TS_CLRI) == 1) {
				readbits(clrimap);
				for (ino = 1; ino <= maxi; ino++)
					if (BIT(ino, clrimap) == 0) {
						getdino(ino, &tino);
						if (tino.di_mode == 0)
							continue;
						if (nowrite)	/* M009 */
							printf("Inode %u: clear on tape, not clear on filesystem\n",
							    ino);
						else {
							itrunc(&tino);
							clri(&tino);
							putdino(ino, &tino);
						}
					}
				dwrite( (daddr_t) 1, (char *) &sblock);	/*M009*/
				goto ragain;
			}
			if (checktype(&spcl, TS_BITS) == 1) {
				readbits(dumpmap);
				goto ragain;
			}
			if (checktype(&spcl, TS_INODE) == 0) {
				printf("Unknown header type\n");
				eflag++;
				gethead(&spcl);
				goto ragain;
			}
			ino = spcl.c_inumber;
			if (eflag)
				printf("Resynced at inode %u\n", ino);
			eflag = 0;
			if (ino > maxi) {
				printf("%u: ilist too small\n", ino);
				gethead(&spcl);
				goto ragain;
			}
			dino = spcl.c_dinode;
			getdino(ino, &tino);
			curbno = 0;
			if (command == 'C') {			/* M009 */
				if (tino.di_mode == 0) {
					printf("Inode %u: clear on filesystem, not clear on tape\n",
					    ino);
					getfile(ino, null, null, dino.di_size);
					continue;
				}

				/*
				 * If the inode change times are different,
				 * comparing the inodes is extremely likely to
				 * fail and that failure will not convey any
				 * useful information, so don't bother
				 * comparing; just note the fact.
				 * The same holds for the inode modification
				 * times and comparing the data.
				 */
				if (tino.di_ctime != dino.di_ctime) {
					if (tino.di_mtime != dino.di_mtime) {
						printf("Inode %u: inode and data changed during dump\n", ino);
						getfile(ino, null, null, dino.di_size);
						continue;
					}
					printf("Inode %u: inode (but not data) changed during dump\n", ino);
				} else {
					if (tino.di_mode != dino.di_mode)
						printf("Inode %u: mode compare error\n", ino);
					if (tino.di_nlink != dino.di_nlink)
						printf("Inode %u: link count compare error\n", ino);
					if (tino.di_uid != dino.di_uid)
						printf("Inode %u: uid compare error\n", ino);
					if (tino.di_gid != dino.di_gid)
						printf("Inode %u: gid compare error\n", ino);
					if (tino.di_size != dino.di_size)
						printf("Inode %u: size compare error\n", ino);
					switch(dino.di_mode&IFMT) {

					case IFBLK:
					case IFCHR:
						for (i = 0; i < 40; i++) {
							if (tino.di_addr[i] != dino.di_addr[i]) {
								printf("Inode %u: address compare error\n", ino);
								break;
							}
						}
						break;
					}
				}
				l3tol(taddr, tino.di_addr, NADDR);
				if (tino.di_mtime != dino.di_mtime) {
					printf("Inode %u: data (but not inode) changed during dump\n", ino);
					getfile(ino, null, null, dino.di_size);
				} else
					getfile(ino, vfyfile, vfyskip, dino.di_size);
			} else {
				ip = &tino;
				ip1 = &dino;
				itrunc(ip);
				clri(ip);

				for (i = 0; i < NADDR; i++)
					taddr[i] = 0;
								/* M008 begin */
				switch(ip1->di_mode & IFMT) {
				case IFIFO:
					ip->di_size = 0;
					break;
				default:
					l3tol(taddr, dino.di_addr, 1);
					ip->di_size = ip1->di_size;
					break;
				}
				getfile(ino, rstrfile, rstrskip, dino.di_size);
				ltol3(ip->di_addr, taddr, NADDR);
								/* M008 end */
				ip->di_mode = ip1->di_mode;
				ip->di_nlink = ip1->di_nlink;
				ip->di_uid = ip1->di_uid;
				ip->di_gid = ip1->di_gid;
				ip->di_atime = ip1->di_atime;
				ip->di_mtime = ip1->di_mtime;
				ip->di_ctime = ip1->di_ctime;
				putdino(ino, &tino);
			}
		}
	}
}

/*
 * Skip some number of files
 */
#ifndef STANDALONE
skipfiles(nfiles)
int nfiles;
{
	register int i;

	while (nfiles > 0) {
		while ((i = read(mt, tbf, NTREC*BSIZE)) != 0) {
			if (i < 0)
				printf("Tape read error\n");
		}
		nfiles--;
	}
}
#endif

/*
 * Read the tape, bulding up a directory structure for extraction
 * by name
 */
#ifndef STANDALONE
pass1()
{
	int	putdir(), null();
	int	firsttime = 0;

	while (gethead(&spcl) == 0) {
		printf("Can't find directory header!\n");
	}
	for (;;) {
		if (checktype(&spcl, TS_BITS) == 1) {
			if( firsttime == 0 ) {
				readbits(dumpmap);
				firsttime++;
			} else
				/* dummy read so we don't wipe out bitmap */
				readbits(clrimap);
			continue;
		}
		if (checktype(&spcl, TS_CLRI) == 1) {
			readbits(clrimap);
			continue;
		}
		if (checktype(&spcl, TS_INODE) == 0) {
finish:
			flsh();
			close(mt);
			return;
		}
		if ((spcl.c_dinode.di_mode & IFMT) != IFDIR) {
			goto finish;
		}
		if( ipos > MAXINO ) {
			fprintf(stderr, "%s: inotab - table overflow\n", cmd);
			exit(-1);
		}
		inotab[ipos].t_ino = spcl.c_inumber;
		inotab[ipos].t_mode = spcl.c_dinode.di_mode;
		inotab[ipos].t_uid = spcl.c_dinode.di_uid;
		inotab[ipos].t_gid = spcl.c_dinode.di_gid;
		inotab[ipos++].t_seekpt = seekpt;
		getfile(spcl.c_inumber, putdir, null, spcl.c_dinode.di_size);
		putent("\000\000/");
	}
}

printem(pathname, inum)
char *pathname;
ino_t	inum;
{
	if (BIT(inum, dumpmap))
		printf("%5u\t%s\n", inum, pathname);
}

/*
 * Process a file; pathname is the pathname and inum is the inumber.
 * If it is a plain file, just call the function ffunc on it.
 * If it is a directory, call the function dfunc on it, then call recurse
 * on each entry in that directory.
 */
recurse(pathname, inum, ffunc, dfunc)
char	*pathname;
ino_t	inum;
int	(*ffunc)();
int	(*dfunc)();
{
	register struct inotab *itp;
	daddr_t savseek;
	struct direct dir;
	register int len;

	if ((itp = isearch(inum)) == (struct inotab *)NULL)
		(*ffunc)(pathname, inum);
	else {
		(*dfunc)(pathname, inum);
		savseek = seekpt;
		mseek(itp->t_seekpt);
		for (;;) {
			getent((char *)&dir);
			if (direq(dir.d_name, "/"))
				break;
			if (direq(dir.d_name, ".") == 0 && direq(dir.d_name, "..") == 0) {
				len = strlen(pathname);
				strcat(pathname, "/");
				strncat(pathname, dir.d_name, sizeof(dir.d_name));
				recurse(pathname, dir.d_ino, ffunc, dfunc);
				pathname[len] = '\0';
			}
		}
		mseek(savseek);
	}
	return;
}

/*
 * Add an entry to the list of files to be extracted.
 */
addent(pathname, inum)
char	*pathname;
ino_t	inum;
{
	register int i;

	if (BIT(inum, dumpmap) == 0) {
		printf("%s: not on this dump\n", pathname);
		return;
	}
	if (nxtr >= MAXXTR) {
		/* M011 */
		printf("%s: too many files, can't restore\n", pathname);
		return;
	}
	for (i = 0; i < nxtr; i++) {
		if (xtrlist[i].x_ino == inum)
			goto found;
	}
	/* i == nxtr at this point; nxtr is the first free slot */
	xtrlist[i].x_ino = inum;
	xtrlist[i].x_flags |= INUSE;
	nxtr++;

found:
	printf("%s: inode %u\n", pathname, inum);
}
#endif

/*
 * Get a file or directory
 */
#ifndef STANDALONE
fdget(pathname, inum)
char	*pathname;
ino_t	inum;
{
	register int exists;
	extern skip(),null(),xtrfile();
	long dev;

	/*
	 * We haven't restored the file yet.
	 * Check whether the file we're currently working on restoring is the
	 * one we are up to in the directory tree; if not, just return.
	 * Otherwise, extract it.
	 */
	if (inum != extrino)
		return;		/* this isn't the file we're working on now */

	exists = 0;
	if (access(pathname, 0) >= 0)
		exists = 1;	/* file already exists */

	/*
	 * Check if we've already restored this file once.
	 * If so, we are restoring another link to it, so just link to the
	 * file we already restored.
	 * Otherwise, just return.
	 */
	if (strcmp(nambuf, "") != 0) {
		if (exists)
			return;	/* if file already exists, don't make the link */
		printf("Link file %s to %s\n", pathname, nambuf);
		if (link(nambuf, pathname) < 0)
			printf("%s: cannot link to %s\n", pathname, nambuf);
		return;		/* already extracted, don't do it again */
	}
	printf("Extract file %s\n", pathname);
	checkdir(pathname);	/* make sure all the directories leading up to it exist */
	if ( !(spcl.c_dinode.di_mode & IFMT) ||
	      (spcl.c_dinode.di_mode & IFMT) == IFREG) {
		if ((ofile = creat(pathname, 0666)) < 0) {
			printf("%s: cannot create file\n", pathname);
			return;
		}
		chmod(pathname, spcl.c_dinode.di_mode&(~IFMT));
		chown(pathname, spcl.c_dinode.di_uid, spcl.c_dinode.di_gid);
		getfile(inum, xtrfile, skip, spcl.c_dinode.di_size);
		close(ofile);
		utime(pathname,&(spcl.c_dinode.di_atime));
	}
	else if ((spcl.c_dinode.di_mode & IFMT) == IFDIR) {
		if (!exists) {
			if (mkdir(pathname) != 0)
				return;			/* create failed */
			chmod(pathname, spcl.c_dinode.di_mode&(~IFMT));
			chown(pathname, spcl.c_dinode.di_uid, spcl.c_dinode.di_gid);
		}
		getfile(inum, null, null, spcl.c_dinode.di_size);	/* skip directory contents on tape */
	}
	else {							/* M007 begin */
		if (!exists) {
			l3tol(&dev, spcl.c_dinode.di_addr, 1);
			if ( mknod(pathname, spcl.c_dinode.di_mode, (int) dev)
									< 0) {
				printf("%s: cannot create special file\n",
								pathname);
				return;
			}
			chmod(pathname, spcl.c_dinode.di_mode&(~IFMT));
			chown(pathname, spcl.c_dinode.di_uid, spcl.c_dinode.di_gid);
		}
		getfile(inum, null, null, spcl.c_dinode.di_size);	/* skip directory contents on tape */
	}							/* M007 end */
	strcpy(nambuf, pathname);	/* we've restored the first entry */
}

/*
 * Check that all the directories in a pathname exist and create them
 * if they do not.
 */
checkdir(pathname)
register char *pathname;
{
	register char *cp;
	int stat;
	int inum;
	register struct inotab *ip;

	for (cp = pathname; *cp; cp++) {
		if ( pathname[0] == '/' )
			continue;
		if (*cp == '/') {
			*cp = '\0';
			if (access(pathname, 0) < 0) {
				if (mkdir(pathname) == 0) {
					if ((inum = psearch(pathname)) != 0
					    && (ip = isearch(inum)) != (struct inotab *)NULL) {
						chmod(pathname, ip->t_mode&(~IFMT));
						chown(pathname, ip->t_uid, ip->t_gid);
					}
				}
			}
			*cp = '/';
		}
	}
}

/*
 * Create a directory
 */
mkdir(pathname)
char *pathname;
{
	register int pid, rp;
	int stat;

	if ((pid = fork()) == 0) {
		execl("/bin/mkdir", "mkdir", pathname, (char *)NULL);
		execl("/usr/bin/mkdir", "mkdir", pathname, (char *)NULL);
		printf("Cannot find mkdir!\n");
		exit(-1);
	}
	while ((rp = wait(&stat)) >= 0 && rp != pid)
		;
	if (stat != 0)
		printf("Can't make directory %s\n", pathname);
	return(stat);
}
#endif

/*
 * Do the file extraction, calling the supplied functions
 * with the blocks
 */
getfile(n, f1, f2, size)
ino_t	n;
int	(*f2)(), (*f1)();
long	size;
{
	register i;
	struct spcl addrblock;
	char buf[BSIZE];

	addrblock = spcl;
	curino = n;
	goto start;
	for (;;) {
		if (gethead(&addrblock) == 0) {
			printf("Missing address (header) block\n");
			goto eloop;
		}
		if (checktype(&addrblock, TS_ADDR) == 0) {
			spcl = addrblock;
			curino = 0;
			return;
		}
start:
		for (i = 0; i < addrblock.c_count; i++) {
			if (addrblock.c_addr[i]) {
				readtape(buf);
				(*f1)(buf, size > BSIZE ? (long) BSIZE : size);
			}
			else {
				clearbuf(buf);
				(*f2)(buf, size > BSIZE ? (long) BSIZE : size);
			}
			if ((size -= BSIZE) <= 0) {
eloop:
				while (gethead(&spcl) == 0)
					;
				if (checktype(&spcl, TS_ADDR) == 1)
					goto eloop;
				curino = 0;
				return;
			}
		}
	}
}

/*
 * Do the tape i/o, dealing with volume changes
 * etc..
 */
readtape(b)
char *b;
{
	register i;
	struct spcl tmpbuf;

	if (bct >= NTREC) {
		for (i = 0; i < NTREC; i++)
			((struct spcl *)&tbf[i*BSIZE])->c_magic = 0;
		bct = 0;
		if (ksize)                         
			if (kbytes >= ksize) {
				kbytes = 0L;
				goto newvol;
			}
		kbytes += (NTREC * BSIZE)/1024;
		if ((i = read(mt, tbf, NTREC*BSIZE)) < 0) {
			printf("Tape read error: inode %u\n", curino);
			eflag++;
			for (i = 0; i < NTREC; i++)
				clearbuf(&tbf[i*BSIZE]);
			/* Berkeley says exit(1); */
		}
		if (i == 0 || i < NTREC*BSIZE) {
newvol:
			bct = NTREC + 1;
			volno++;
loop:
			flsht();
			close(mt);
								/* M009... */
			printf("\n\007Please mount volume %d: \007", volno);
			fflush(stdout);				/* M009 */
			while (getchar() != '\n')
				;
			if ((mt = open(magtape, 0)) == -1) {
				printf("Cannot open dump medium %s!\n",magtape);
				goto loop;
			}
			if (readhdr(&tmpbuf) == 0) {
				printf("Not a dump volume. Try again\n");
				goto loop;
			}
			if (checkvol(&tmpbuf, volno) == 0) {
				printf("Wrong volume. Try again\n");
				goto loop;
			}
			readtape(b);
			return;
		}
	}
	copy(&tbf[(bct++*BSIZE)], b, BSIZE);
}

flsht()
{
	bct = NTREC+1;
	kbytes = 0;
}

copy(f, t, s)
register char *f, *t;
{
	register i;

	i = s;
	do
		*t++ = *f++;
	while (--i);
}

clearbuf(cp)
register char *cp;
{
	register i;

	i = BSIZE;
	do
		*cp++ = 0;
	while (--i);
}

/*
 * Put and get the directory entries from the compressed
 * directory file
 */
#ifndef STANDALONE
putent(cp)
char	*cp;
{
	register i;

	for (i = 0; i < sizeof(ino_t); i++)
		writec(*cp++);
	for (i = 0; i < DIRSIZ; i++) {
		writec(*cp);
		if (*cp++ == 0)
			return;
	}
	return;
}

getent(bf)
register char *bf;
{
	register i;

	for (i = 0; i < sizeof(ino_t); i++)
		*bf++ = readc();
	for (i = 0; i < DIRSIZ; i++)
		if ((*bf++ = readc()) == 0)
			return;
	return;
}

/*
 * read/write te directory file
 */
writec(c)
char c;
{
	drblock[bpt++] = c;
	seekpt++;
	if (bpt >= BSIZE) {
		bpt = 0;
		write(df, drblock, BSIZE);
	}
}

readc()
{
	if (bpt >= BSIZE) {
		read(df, drblock, BSIZE);
		bpt = 0;
	}
	seekpt++;
	return(drblock[bpt++]);
}

mseek(pt)
daddr_t pt;
{
	bpt = BSIZE;
	seekpt = pt;
	lseek(df, pt, 0);
}

flsh()
{
	write(df, drblock, bpt+1);
}

/*
 * search the inode table
 * looking for inode inum
 */
struct inotab *
isearch(inum)
ino_t	inum;
{
	register struct inotab *itp;

	for (itp = &inotab[0]; itp < &inotab[MAXINO] && itp->t_ino; itp++)
		if (itp->t_ino == inum)
			return(itp);
	return((struct inotab *)NULL);
}

/*
 * search the directory inode inum
 * looking for entry cp
 */
ino_t
search(inum, cp)
ino_t	inum;
char	*cp;
{
	register struct inotab *itp;
	struct direct dir;

	if ((itp = isearch(inum)) == (struct inotab *)NULL)
		return(0);
	mseek(itp->t_seekpt);
	do {
		getent((char *)&dir);
		if (direq(dir.d_name, "/"))
			return(0);
	} while (direq(dir.d_name, cp) == 0);
	return(dir.d_ino);
}

/*
 * Search the directory tree rooted at inode ROOTINO
 * for the path pointed at by n
 */
psearch(n)
char	*n;
{
	register int inum;
	register char *cp, *cp1;
	char c;

	inum = ROOTINO;
	if (*(cp = n) == '/')
		cp++;
next:
	cp1 = cp + 1;
	while (*cp1 != '/' && *cp1)
		cp1++;
	c = *cp1;
	*cp1 = 0;
	inum = search(inum, cp);
	if (inum == 0) {
		*cp1 = c;
		return(0);
	}
	*cp1 = c;
	if (c == '/') {
		cp = cp1+1;
		goto next;
	}
	return(inum);
}

direq(s1, s2)
register char *s1, *s2;
{
	register i;

	for (i = 0; i < DIRSIZ; i++)
		if (*s1++ == *s2) {
			if (*s2++ == 0)
				return(1);
		} else
			return(0);
	return(1);
}
#endif

/*
 * read/write a disk block, be sure to update the buffer
 * cache if needed.
 */
dwrite(bno, b)
daddr_t	bno;
char	*b;
{
	register i;

	for (i = 0; i < NCACHE; i++) {
		if (cache[i].c_bno == bno) {
			copy(b, cache[i].c_block, BSIZE);
			cache[i].c_time = 0;
			break;
		}
		else
			cache[i].c_time++;
	}
	lseek(fi, bno*BSIZE, 0);
	if(write(fi, b, BSIZE) != BSIZE) {
#ifdef STANDALONE
		printf("disk write error %ld\n", bno);
#else
		fprintf(stderr, "disk write error %ld\n", bno);
#endif
		exit(1);
	}
}

/*
 * dread - read "cnt" bytes of block "bno" into "buf".
 * Return 0 if successful, -1 on I/O error.
 */
dread(bno, buf, cnt)
daddr_t bno;
char *buf;
int cnt;
{
	register int i, j;

	j = 0;
	for (i = 0; i < NCACHE; i++) {
		if (++curcache >= NCACHE)
			curcache = 0;
		if (cache[curcache].c_bno == bno) {
			copy(cache[curcache].c_block, buf, cnt);
			cache[curcache].c_time = 0;
			return(0);
		}
		else {
			cache[curcache].c_time++;
			if (cache[j].c_time < cache[curcache].c_time)
				j = curcache;
		}
	}

	lseek(fi, bno*BSIZE, 0);
	if (read(fi, physbuf, BSIZE) != BSIZE) {	/* M002 */
#ifdef STANDALONE
		printf("read error %ld\n", bno);
#else							/* M009... */
		fprintf(stderr, "disk read error %d, block %ld\n", errno, bno);
#endif
		return(-1);
	}
	copy(physbuf, cache[j].c_block, BSIZE);		/* M002 */
	copy(cache[j].c_block, buf, cnt);
	cache[j].c_time = 0;
	cache[j].c_bno = bno;
	return(0);
}

/*
 * the inode manpulation routines. Like the system.
 *
 * clri zeros the inode
 */
clri(ip)
struct dinode *ip;
{
	int i, *p;
	if (ip->di_mode&IFMT)
		sblock.s_tinode++;
	i = sizeof(struct dinode)/sizeof(int);
	p = (int *)ip;
	do
		*p++ = 0;
	while(--i);
}

/*
 * itrunc/tloop/bfree free all of the blocks pointed at by the inode
 */
itrunc(ip)
register struct dinode *ip;
{
	register i;
	daddr_t bn, iaddr[NADDR];

	if (ip->di_mode == 0)
		return;
	i = ip->di_mode & IFMT;
	if (i != IFDIR && i != IFREG)
		return;
	l3tol(iaddr, ip->di_addr, NADDR);
	for(i=NADDR-1;i>=0;i--) {
		bn = iaddr[i];
		if(bn == 0) continue;
		switch(i) {

		default:
			bfree(bn);
			break;

		case NADDR-3:
			tloop(bn, 0, 0);
			break;

		case NADDR-2:
			tloop(bn, 1, 0);
			break;

		case NADDR-1:
			tloop(bn, 1, 1);
		}
	}
	ip->di_size = 0;
}

tloop(bn, f1, f2)
daddr_t	bn;
int	f1, f2;
{
	register i;
	daddr_t nb;
	union {
		char	data[BSIZE];
		daddr_t	indir[NINDIR];
	} ibuf;

	if (dread(bn, ibuf.data, BSIZE) < 0)
		exit(1);	/* can't restore or compare, indirect block is bad */
	for(i=NINDIR-1;i>=0;i--) {
		nb = ibuf.indir[i];
		if(nb) {
			if(f1)
				tloop(nb, f2, 0);
			else
				bfree(nb);
		}
	}
	bfree(bn);
}

bfree(bn)
daddr_t	bn;
{
	register i;
	union {
		char	data[BSIZE];
		struct	fblk frees;
	} fbun;
#define	fbuf fbun.frees

	if(sblock.s_nfree >= NICFREE) {
		fbuf.df_nfree = sblock.s_nfree;
		for(i=0;i<NICFREE;i++)
			fbuf.df_free[i] = sblock.s_free[i];
		sblock.s_nfree = 0;
		dwrite(bn, fbun.data);
	}
	sblock.s_free[sblock.s_nfree++] = bn;
	sblock.s_tfree++;
}

/*
 * allocate a block off the free list.
 */
daddr_t
balloc()
{
	daddr_t	bno;
	register i;
	static char zeroes[BSIZE];
	union {
		char	data[BSIZE];
		struct	fblk frees;
	} fbun;
#undef	fbuf
#define	fbuf fbun.frees

	if(sblock.s_nfree == 0 || (bno=sblock.s_free[--sblock.s_nfree]) == 0) {
#ifdef STANDALONE
		printf("Out of space\n");
#else
		fprintf(stderr, "Out of space.\n");
#endif
		exit(1);
	}
	if(sblock.s_nfree == 0) {
		if (dread(bno, (char *)&fbuf, BSIZE) < 0)
			exit(1);	/* can't restore or compare, superblock is bad */
		sblock.s_nfree = fbuf.df_nfree;
		for(i=0;i<NICFREE;i++)
			sblock.s_free[i] = fbuf.df_free[i];
	}
	dwrite(bno, zeroes);
	sblock.s_tfree--;
	return(bno);
}

/*
 * map a block number into a block address, ensuring
 * all of the correct indirect blocks are around. Allocate
 * the block requested if it is not already allocated, if rwflg is 1.
 */
daddr_t
bmap(iaddr, bn, rwflg)
daddr_t	iaddr[NADDR];
daddr_t	bn;
{
	register i;
	int j, sh;
	daddr_t nb, nnb;
	daddr_t indir[NINDIR];

	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if(bn < NADDR-3) {
		if (!rwflg) {
			if ((nb = iaddr[bn]) == 0)
				return((daddr_t)0);
		} else
			iaddr[bn] = nb = balloc();
		return(nb);
	}

	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	sh = 0;
	nb = 1;
	bn -= NADDR-3;
	for(j=3; j>0; j--) {
		sh += NSHIFT;
		nb <<= NSHIFT;
		if(bn < nb)
			break;
		bn -= nb;
	}
	if(j == 0) {
		return((daddr_t)0);
	}

	/*
	 * fetch the address from the inode
	 */
	if((nb = iaddr[NADDR-j]) == 0) {
		if(!rwflg)
			return((daddr_t)0);
		iaddr[NADDR-j] = nb = balloc();
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=3; j++) {
		if (dread(nb, (char *)indir, BSIZE) < 0)
			exit(1);	/* can't restore or compare, indirect block is bad */
		sh -= NSHIFT;
		i = (bn>>sh) & NMASK;
		nnb = indir[i];
		if(nnb == 0) {
			if (!rwflg)
				return((daddr_t)0);
			nnb = balloc();
			indir[i] = nnb;
			dwrite(nb, (char *)indir);
		}
		nb = nnb;
	}
	return(nb);
}

/*
 * read the tape into buf, then return whether or
 * or not it is a header block.
 */
gethead(buf)
struct spcl *buf;
{
	readtape((char *)buf);
	if (buf->c_magic != MAGIC || checksum((short *) buf) == 0)
		return(0);
	return(1);
}

/*
 * return whether or not the buffer contains a header block
 */
ishead(buf)
struct spcl *buf;
{
	if (buf->c_magic != MAGIC || checksum((short *) buf) == 0)
		return(0);
	return(1);
}

checktype(b, t)
struct	spcl *b;
int	t;
{
	return(b->c_type == t);
}


checksum(b)
short *b;
{
	register short i, j;

	j = BSIZE/sizeof(short);
	i = 0;
	do
		i += *b++;
	while (--j);
	if (i != CHECKSUM) {
		printf("Checksum error %o\n", i);
		return(0);
	}
	return(1);
}

checkvol(b, t)
struct spcl *b;
int t;
{
	if (b->c_volume == t)
		return(1);
	return(0);
}

readhdr(b)
struct	spcl *b;
{
	if (gethead(b) == 0)
		return(0);
	if (checktype(b, TS_TAPE) == 0)
		return(0);
	return(1);
}

/*
 * The next routines are called during file extraction to
 * put the data into the right form and place.
 */
#ifndef STANDALONE
xtrfile(b, size)
char	*b;
long	size;
{
	write(ofile, b, (int) size);
}

null() {;}

skip()
{
	lseek(ofile, (long) BSIZE, 1);
}
#endif


/* ARGSUSED */
rstrfile(b, s)
char *b;
long s;
{
	daddr_t d;

	d = bmap(taddr, curbno, 1);
	dwrite(d, b);
	curbno += 1;
}

/* ARGSUSED */
rstrskip(b, s)
char *b;
long s;
{
	curbno += 1;
}

#ifndef STANDALONE
vfyfile(b, s)
char *b;
long s;
{
	daddr_t d;
	char vfyblk[BSIZE];
	register char *b1, *b2;
	register unsigned size;

	if ((d = bmap(taddr, curbno, 0)) == (daddr_t)0)
		printf("Block missing in file: inode %u block %ld\n", curino, curbno);
	else {
		if (dread(d, vfyblk, BSIZE) < 0)
			printf("I/O error in file: inode %u block %ld\n", curino, curbno);
		else {
			b1 = vfyblk;
			b2 = b;
			size = BSIZE;
			do {
				if(*b1++ != *b2++) {
					printf("Data compare error: inode %u block %ld\n", curino, curbno);
					break;
				}
			} while (--size);
		}
	}
	curbno += 1;
}

vfyskip(b, s)
char *b;
long s;
{
	if (bmap(taddr, curbno, 0) != (daddr_t)0)
		printf("Block missing in dump: inode %u block %ld\n", curino, curbno);
	curbno += 1;
}
#endif

#ifndef STANDALONE
putdir(b)
char *b;
{
	register struct direct *dp;
	register i;

	for (dp = (struct direct *) b, i = 0; i < BSIZE; dp++, i += sizeof(*dp)) {
		if (dp->d_ino == 0)
			continue;
		putent((char *) dp);
	}
}
#endif

/*
 * read/write an inode from the disk
 */
getdino(inum, b)
ino_t	inum;
struct	dinode *b;
{
	daddr_t	bno;
	char buf[BSIZE];

	bno = (inum - 1)/INOPB;
	bno += 2;
	if (dread(bno, buf, BSIZE) < 0)
		exit(1);	/* can't restore or compare, ilist block is bad */
	copy(&buf[((inum-1)%INOPB)*sizeof(struct dinode)], (char *) b, sizeof(struct dinode));
}

putdino(inum, b)
ino_t	inum;
struct	dinode *b;
{
	daddr_t bno;
	char buf[BSIZE];

	if (b->di_mode&IFMT)
		sblock.s_tinode--;
	bno = ((inum - 1)/INOPB) + 2;
	if (dread(bno, buf, BSIZE) < 0)
		exit(1);	/* can't restore or compare, ilist block is bad */
	copy((char *) b, &buf[((inum-1)%INOPB)*sizeof(struct dinode)], sizeof(struct dinode));
	dwrite(bno, buf);
}

/*
 * read a bit mask from the tape into m.
 */
readbits(m)
short	*m;
{
	register i;

	i = spcl.c_count;

	while (i--) {
		readtape((char *) m);
		m += (BSIZE/(MLEN/BITS));
	}
	while (gethead(&spcl) == 0)
		;
}

done()
{
#ifndef STANDALONE
	unlink(dirfile);
#endif
	exit(0);
}
