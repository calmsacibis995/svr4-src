/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tar:tar.c	1.27.1.11"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mkdev.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/errno.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <string.h>
#include <sum.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#ifndef LPNMAX
#include <limits.h>
#define LPNMAX PATH_MAX
#define LFNMAX NAME_MAX
#endif /* ! LPNMAX */

#ifndef MINSIZE
#define MINSIZE 250
#endif
#define	DEF_FILE "/etc/default/tar"


extern	char	*fgets(),
		*getcwd();

extern	long	time();

extern 	int	lchown(),
		readlink(),
		symlink(),
		utime();

extern	void	sumout(),
		sumupd(),
		sumpro(),
		sumepi();

extern	pid_t	wait();
extern 	int	errno;

static	void	add_file_to_table(),
		backtape(),
		build_table(),
		closevol(),
		copy(),
		done(),
		dorep(),
		dotable(),
		doxtract(),
		fatal(),
		flushtape(),
		getdir(),
		getempty(),
		getwdir(),
		initarg(),
		longt(),
		newvol(),
		passtape(),
		putempty(),
		putfile(),
		readtape(),
		seekdisk(),
		splitfile(),
		tomodes(),
		usage(),
		vperror(),
		writetape(),
		xblocks(),
		xsfile();

static	int	bcheck(),
		checkdir(),
		checksum(),
		checkupdate(),
		checkw(),
		cmp(),
		defset(),
		endtape(),
		is_in_table(),
		notsame(),
		prefix(),
		response();

static	daddr_t	bsrch();
static	char	*nextarg();
static	long	kcheck();
char	*defread();
static	unsigned int hash();
static	void onintr(), onhup(), onquit();


/* -DDEBUG	ONLY for debugging */
#ifdef	DEBUG
#undef	DEBUG
#define	DEBUG(a,b,c)	(void) fprintf(stderr,"DEBUG - "),(void) fprintf(stderr, a, b, c)
#else
#define	DEBUG(a,b,c)
#endif

#define	TBLOCK	512	/* tape block size--should be universal */

#ifdef	BSIZE
#define	SYS_BLOCK BSIZE	/* from sys/param.h:  secondary block size */
#else	/* BSIZE */
#define	SYS_BLOCK 512	/* default if no BSIZE in param.h */
#endif	/* BSIZE */

#define NBLOCK	20
#define NAMSIZ	100
#define PRESIZ	155
#define MAXNAM	256
#define	MODEMASK 0777777	/* file creation mode mask */
#define	MAXEXT	9	/* reasonable max # extents for a file */
#define	EXTMIN	50	/* min blks left on floppy to split a file */

#define EQUAL	0	/* SP-1: for `strcmp' return */
#define	TBLOCKS(bytes)	(((bytes) + TBLOCK - 1)/TBLOCK)	/* useful roundup */
#define	K(tblocks)	((tblocks+1)/2)	/* tblocks to Kbytes for printing */

#define	MAXLEV	18
#define	LEV0	1

#define TRUE	1
#define FALSE	0

/* Was statically allocated tbuf[NBLOCK] */
union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];	/* size of this extent if file split */
		char mtime[12];
		char chksum[8];
		char typeflag;
		char linkname[NAMSIZ];
		char magic[6];
		char version[2];
		char uname[32];
		char gname[32];
		char devmajor[8];
		char devminor[8];
		char prefix[155];
		char extno;		/* extent #, null if not split */
		char extotal;	/* total extents */
		char efsize[10];	/* size of entire file */
	} dbuf;
} dblock, *tbuf;

char old_extno[4];		/* extent #, null if not split */
char old_efsize[12];	/* size of entire file */

static
struct gen_hdr {
	ulong	g_mode,         /* Mode of file */
		g_uid,          /* Uid of file */
		g_gid;          /* Gid of file */
	long	g_filesz;       /* Length of file */
	ulong	g_mtime,        /* Modification time */
		g_cksum,        /* Checksum of file */
		g_devmajor,          /* File system of file */
		g_devminor;         /* Major/minor numbers of special files */
} Gen;

static
struct linkbuf {
	ino_t	inum;
	dev_t	devnum;
	int     count;
	char	pathname[MAXNAM];
	struct	linkbuf *nextp;
} *ihead;

/* see comments before build_table() */
#define TABLE_SIZE 512
struct	file_list	{
	char	*name;			/* Name of file to {in,ex}clude */
	struct	file_list	*next;	/* Linked list */
};
static	struct	file_list	*exclude_tbl[TABLE_SIZE], *include_tbl[TABLE_SIZE];

static	struct stat stbuf;

static	int	checkflag = 0;
static	int	Xflag, Fflag, iflag, hflag, Bflag, Iflag;
static	int	rflag, xflag, vflag, tflag, mt, cflag, mflag, pflag;
static	int	uflag;
static	int	eflag;
static	int	sflag, Sflag;
static	int	oflag;
static	int	bflag, kflag, Aflag;
static	int	term, chksum, wflag, recno, 
		first = TRUE, defaults_used = FALSE, linkerrok;
static	int	freemem = 1;
static	int	nblock = NBLOCK;
static	int	Errflg = 0;
static	int	totfiles = 0;

static	dev_t	mt_dev;				/* device containing output file */
static	ino_t	mt_ino;				/* inode number of output file */

static	int update = 1;		/* for `open' call */

static	daddr_t	low;
static	daddr_t	high;

static	FILE	*tfile;
static	FILE	*vfile = stdout;
static	char	tname[] = "/tmp/tarXXXXXX";
static	char	archive[] = "archive0=";
static	char	*Xfile;
static	char	*usefile;
static	char	*Filefile;
static	char	*Sumfile;
static	FILE	*Sumfp;
static	struct suminfo	Si;	

static	int	mulvol;		/* multi-volume option selected */
static	long	blocklim;	/* number of blocks to accept per volume */
static	long	tapepos;	/* current block number to be written */
long	atol();		/* to get blocklim */
static	int	NotTape;	/* true if tape is a disk */
static	int	dumping;	/* true if writing a tape or other archive */
static	int	extno;		/* number of extent:  starts at 1 */
static	int	extotal;	/* total extents in this file */
static	long	efsize;		/* size of entire file */
static	ushort	Oumask = 0;	/* old umask value */

static	ushort	Ftype = S_IFMT;

void
main(argc, argv)
int	argc;
char	*argv[];
{
	char *cp;
#ifdef XENIX_ONLY
#if SYS_BLOCK > TBLOCK
	struct stat statinfo;
#endif
#endif

	if (argc < 2)
		usage();

	if ((tbuf = (union hblock *) calloc(sizeof(union hblock) * NBLOCK, sizeof(char))) == (union hblock *) NULL) {
		(void) fprintf(stderr, "tar: cannot allocate physio buffer\n");
		exit(1);
	}
	tfile = NULL;
	argv[argc] = 0;
	argv++;
	/*
	 * Set up default values.
	 * Search the option string looking for the first digit or an 'f'.
	 * If you find a digit, use the 'archive#' entry in DEF_FILE.
	 * If 'f' is given, bypass looking in DEF_FILE altogether. 
	 * If no digit or 'f' is given, still look in DEF_FILE but use '0'.
	 */
	if ((usefile = getenv("TAPE")) == (char *)NULL) {
		for (cp = *argv; *cp; ++cp)
			if (isdigit(*cp) || *cp == 'f')
				break;
		if (*cp != 'f') {
			archive[7] = (*cp)? *cp: '0';
			if (!(defaults_used = defset(archive))) {
				usefile = NULL;
				nblock = 1;
				blocklim = 0;
				NotTape = 0;
			}
		}
	}

	for (cp = *argv++; *cp; cp++)
		switch(*cp) {
		case 'f':
			usefile = *argv++;
			if (strcmp(usefile, "-") == 0)
				nblock = 1;
			break;
		case 'F':
			Filefile = *argv++;
			Fflag++;
			if (strcmp(Filefile, "-") == 0)
				nblock = 1;
			break;
		case 'c':
			cflag++;
			rflag++;
			update = 1;
			break;
		case 'u':
			uflag++;     /* moved code after signals caught */
			rflag++;
			update = 2;
			break;
		case 'r':
			rflag++;
			update = 2;
			break;
		case 'v':
			vflag++;
			break;
		case 'w':
			wflag++;
			break;
		case 'x':
			xflag++;
			break;
		case 'X':
			if (*argv == 0) {
				(void) fprintf(stderr,
		"tar: exclude file must be specified with 'X' option\n");
				usage();
			}
			Xflag = 1;
			Xfile = *argv++;
			if (strcmp(Xfile, "-") == 0)
				nblock = 1;
			build_table(exclude_tbl, Xfile);
			break;
		case 't':
			tflag++;
			break;
		case 'm':
			mflag++;
			break;
		case 'p':
			pflag++;
			break;
		case 'S':
			Sflag++;
			/*FALLTHRU*/
		case 's':
			sflag++;
			Sumfile = *argv++;
			break;
		case '-':
			nblock = 1;
			break;
		case '0':	/* numeric entries used only for defaults */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;
		case 'b':
			bflag++;
			nblock = bcheck(*argv++);
			break;
		case 'k':
			kflag++;
			blocklim = kcheck(*argv++);
			break;
		case 'n':	/* not a magtape (instead of 'k') */
			NotTape++;	/* assume non-magtape */
			break;
		case 'l':
			linkerrok++;
			break;
		case 'e':
			eflag++;
			break;
		case 'o':
			oflag++;
			break;
		case 'A':
			Aflag++;
			break;
		case 'L':
		case 'h':
			hflag++;
			break;
		case 'i':
			iflag++;
			break;
		case 'B':
			Bflag++;
			break;

#ifdef i386
		case 'q':
			break;
#endif

		default:
			(void) fprintf(stderr, "tar: %c: unknown option\n", *cp);
			usage();
		}

	if (Xflag && Fflag) {
		(void) fprintf(stderr, "tar: specify only one of X or F.\n");
		usage();
	}
	if (!rflag && !xflag && !tflag)
		usage();
	if ((rflag && xflag) || (xflag && tflag) || (rflag && tflag)) {
		(void) fprintf(stderr, "tar: specify only one of [txru].\n");
		usage();
	}
	if (cflag && !*argv && !*Filefile)
		fatal("Missing filenames");
	if (rflag && !cflag && !NotTape && nblock != 1)
		fatal("Blocked tapes cannot be updated");
	if (usefile == NULL)
		fatal("device argument required");
#ifdef XENIX_ONLY
#if SYS_BLOCK > TBLOCK
	/* if user gave blocksize for non-tape device check integrity */
	(void) fprintf(stderr, "SYS_BLOCK == %d, TBLOCK == %d\n", SYS_BLOCK, TBLOCK);
	if (cflag &&			/* check only needed when writing */
	    NotTape &&
	    stat(usefile, &statinfo) >= 0 &&
	    ((statinfo.st_mode & S_IFMT) == S_IFCHR) &&
	    (nblock % (SYS_BLOCK / TBLOCK)) != 0)
		fatal("blocksize must be multiple of %d.", SYS_BLOCK/TBLOCK);
#endif
#endif
	if (sflag) {
	    if (Sflag && !mulvol)
		fatal("'S' option requires 'k' option.");
	    if ( !(cflag || xflag || tflag) || ( !cflag && (Filefile != NULL || *argv != NULL)) )
		(void) fprintf(stderr, "tar: warning: 's' option results are predictable only with 'c' option or 'x' or 't' option and 0 'file' arguments\n");
	    if (strcmp(Sumfile, "-") == 0)
		Sumfp = stdout;
	    else if ((Sumfp = fopen(Sumfile, "w")) == NULL)
		fatal("cannot open %s.", Sumfile);
	    sumpro(&Si);
	}

	if (rflag) {
		if (cflag && tfile != NULL) {
			usage();
			done(1);
		}
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			(void) signal(SIGINT, onintr);
		if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
			(void) signal(SIGHUP, onhup);
		if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
			(void) signal(SIGQUIT, onquit);
/*              if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
 *                      signal(SIGTERM, onterm);
 */
		if (uflag) {
			(void) mktemp(tname);
			if ((tfile = fopen(tname, "w")) == NULL)
				fatal("cannot create temporary file (%s)",
				      tname);
			(void) fprintf(tfile, "!!!!!/!/!/!/!/!/!/! 000\n");
		}
		if (strcmp(usefile, "-") == 0) {
			if (cflag == 0)
				fatal("can only create standard output archives.");
			vfile = stderr;
			mt = dup(1);
			if (nblock != 1)
				fatal("cannot create blocked standard output archives.");
			++bflag;
		}
		else if ((mt = open(usefile, update)) < 0) {
			if (cflag == 0 || (mt =  creat(usefile, 0666)) < 0)
openerr:
				fatal("cannot open %s.", usefile);
		}
		/* Get inode and device number of output file */
		(void) fstat(mt,&stbuf);
		mt_ino = stbuf.st_ino;
		mt_dev = stbuf.st_dev;
		if (Aflag && vflag)
			(void) printf("Suppressing absolute pathnames\n");
		dorep(argv);
	}
	else if (xflag) {
		/* for each argument, check to see if there is a "-I file" pair.
	 	* if so, move the 3rd argument into "-I"'s place, build_table()
	 	* using "file"'s name and increment argc one (the second increment
	 	* appears in the for loop) which removes the two args "-I" and "file"
	 	* from the argument vector.
	 	*/
		for (argc = 0; argv[argc]; argc++) {
			if (!strcmp(argv[argc], "-I")) {
				if (!argv[argc+1]) {
					(void) fprintf(stderr,
				    	"tar: missing argument for -I flag\n");
					done(2);
				} else {
					Iflag = 1;
					if (Fflag) {
						(void) fprintf(stderr, "tar: only one of I or F.\n");
						usage();
					}

					argv[argc] = argv[argc+2];
					build_table(include_tbl, argv[++argc]);
				}
			}
		}
		if (strcmp(usefile, "-") == 0) {
			mt = dup(0);
			if (nblock != 1)
				fatal("cannot read blocked standard input archives.");
			++bflag;
		}
		else if ((mt = open(usefile, 0)) < 0)
			goto openerr;
		if (Aflag && vflag)
			(void) printf("Suppressing absolute pathnames.\n");
		doxtract(argv);
	}
	else if (tflag) {
		/* for each argument, check to see if there is a "-I file" pair.
	 	* if so, move the 3rd argument into "-I"'s place, build_table()
	 	* using "file"'s name and increment argc one (the second increment
	 	* appears in the for loop) which removes the two args "-I" and "file"
	 	* from the argument vector.
	 	*/
		for (argc = 0; argv[argc]; argc++)
			if (!strcmp(argv[argc], "-I"))
				if (!argv[argc+1]) {
					(void) fprintf(stderr,
				    	"tar: missing argument for -I flag\n");
					done(2);
				} else {
					Iflag = 1;
					if (Fflag) {
						(void) fprintf(stderr, "tar: only one of I or F.\n");
						usage();
					}
					argv[argc] = argv[argc+2];
					build_table(include_tbl, argv[++argc]);
				}
		if (strcmp(usefile, "-") == 0) {
			mt = dup(0);
			if (nblock != 1)
				fatal("cannot read blocked standard input archives");
			++bflag;
		}
		else if ((mt = open(usefile, 0)) < 0)
			goto openerr;
		dotable(argv);
	}
	else
		usage();

	if (sflag) {
		sumepi(&Si);
		sumout(Sumfp, &Si);
		(void) fprintf(Sumfp, "\n");
	}

	done(Errflg);
}

static	void
usage()
{
	(void) fprintf(stderr, "Usage: tar -{txruc}[0-9vfbk[FX]hLiBelmnopwA] [tapefile] [blocksize] [tapesize] [argfile] [exclude-file] [-I include-file] files ...\n");
	done(1);
}

/***    dorep - do "replacements"
 *
 *      Dorep is responsible for creating ('c'),  appending ('r')
 *      and updating ('u');
 */

static	void
dorep(argv)
char	*argv[];
{
	register char *cp, *cp2, *p;
	char wdir[MAXPATHLEN], tempdir[MAXPATHLEN], *parent = (char *) NULL;
	char file[MAXPATHLEN], origdir[MAXPATHLEN];
	FILE *fp = (FILE *)NULL;
	FILE *ff = (FILE *)NULL;


	if (!cflag) {
		getdir();                       /* read header for next file */
		while (!endtape()) {	     /* changed from a do while */
			passtape();             /* skip the file data */
			if (term)
				done(Errflg);   /* received signal to stop */
			getdir();
		}
		backtape();			/* was called by endtape */
		if (tfile != NULL) {
			char buf[200];

			(void) sprintf(buf, "sort +0 -1 +1nr %s -o %s; awk '$1 != prev {print; prev=$1}' %s >%sX;mv %sX %s",
				tname, tname, tname, tname, tname, tname);
			(void) fflush(tfile);
			(void) system(buf);
			(void) freopen(tname, "r", tfile);
			(void) fstat(fileno(tfile), &stbuf);
			high = stbuf.st_size;
		}
	}

	dumping = 1;
	getwdir(wdir);
	if (mulvol) {	/* SP-1 */
		if (nblock && (blocklim%nblock) != 0)
			fatal("Volume size not a multiple of block size.");
		blocklim -= 2;			/* for trailer records */
		if (vflag)
			(void) printf("Volume ends at %luK, blocking factor = %dK\n",
				K(blocklim - 1), K(nblock));
	}
	if (Fflag) {
		if (Filefile != NULL) {
			if ((ff = fopen(Filefile, "r")) == NULL)
				vperror(0, Filefile);
		} else {
			(void) fprintf(stderr, "tar: F requires a file name.\n");
			usage();
		}
	}

	(void) strcpy(origdir, wdir);
	while ((*argv || fp || ff) && !term) {
		cp2 = 0;
		if (fp || !strcmp(*argv, "-I")) {
			if (Fflag) {
				(void) fprintf(stderr, "tar: only one of I or F.\n");
				usage();
			}
			if (fp == NULL) {
				if (*++argv == NULL) {
					(void) fprintf(stderr,
				    "tar: missing file name for -I flag.\n");
					done(1);
				} else if ((fp = fopen(*argv++, "r")) == NULL)
					vperror(0, argv[-1]);
				continue;
			} else if ((fgets(file, MAXPATHLEN-1, fp)) == NULL) {
				(void) fclose(fp);
				fp = NULL;
				continue;
			} else {
				cp = cp2 = file;
				if (p = strchr(cp2, '\n'))
					*p = 0;
			}
		} else if (!strcmp(*argv, "-C") && argv[1]) {
			if (Fflag) {
				(void) fprintf(stderr, "tar: only one of F or C\n");
				usage();
			}
			if (chdir(*++argv) < 0)
				vperror(0, "can't change directories to %s", *argv);
			else
				(void) getcwd(wdir, (sizeof(wdir)));
			argv++;
			continue;
		} else if (Fflag && (ff != NULL)) {
			if ((fgets(file, MAXPATHLEN-1, ff)) == NULL) {
				(void) fclose(ff);
				ff = NULL;
				continue;
			} else {
				cp = cp2 = file;
				if (p = strchr(cp2, '\n'))
					*p = 0;
			}
		} else 
			cp = cp2 = strcpy(file, *argv++);

		parent = wdir;
		for (; *cp; cp++)
			if (*cp == '/')
				cp2 = cp;
		if (cp2 != file) {
			*cp2 = '\0';
			if (chdir(file) < 0) {
				vperror(0,
				    "can't change directories to %s", file);
				continue;
			}
			parent = getcwd(tempdir, (sizeof(tempdir)+2));
			*cp2 = '/';
			cp2++;
		}
		putfile(file, cp2, parent, LEV0);
		if (chdir(origdir) < 0)
			vperror(0, "cannot change back?: ", wdir);
	}
	putempty(2L);
	flushtape();
	closevol();	/* SP-1 */
	if (linkerrok == 1)
		for (; ihead != NULL; ihead = ihead->nextp) {
			if (ihead->count == 0)
				continue;
			(void) fprintf(stderr, "tar: missing links to %s\n", ihead->pathname);
			if (eflag)
				done(1);
		}
}



/***    endtape - check for tape at end
 *
 *      endtape checks the entry in dblock.dbuf to see if its the
 *      special EOT entry.  Endtape is usually called after getdir().
 *
 *	endtape used to call backtape; it no longer does, he who
 *	wants it backed up must call backtape himself
 *      RETURNS:        0 if not EOT, tape position unaffected
 *                      1 if     EOT, tape position unaffected
 */
static	int
endtape()
{
	if (dblock.dbuf.name[0] == '\0') {	/* null header = EOT */
		return(1);
	}
	else
		return(0);
}

/***    getdir - get directory entry from tar tape
 *
 *      getdir reads the next tarblock off the tape and cracks
 *      it as a directory.  The checksum must match properly.
 *
 *      If tfile is non-null getdir writes the file name and mod date
 *      to tfile.
 */

#define MODEMASK1 0007777

static	void
getdir()
{
	register struct stat *sp;
	char k[4], j[2];
	char *a, *b;
	

#ifdef i386
	int	sunflag = 0;
#endif
	a = &k[0];
	b = &j[0];

top:
	readtape( (char *) &dblock);
	if (dblock.dbuf.name[0] == '\0')
		return;
	totfiles++;
	sp = &stbuf;
	(void) sscanf(dblock.dbuf.mode, "%8lo", &Gen.g_mode);
	(void) sscanf(dblock.dbuf.uid, "%8lo", &Gen.g_uid);
	(void) sscanf(dblock.dbuf.gid, "%8lo", &Gen.g_gid);
	(void) sscanf(dblock.dbuf.size, "%12lo", &Gen.g_filesz);
	(void) sscanf(dblock.dbuf.mtime, "%12lo", &Gen.g_mtime);
	(void) sscanf(dblock.dbuf.chksum, "%8lo", &Gen.g_cksum);

#ifdef i386
	/* Check for UNIX System V/386 Release 3.2 / XENIX tar file */
	/* Pre 4.0 tar tape only contains files */
	if ((Gen.g_mode & Ftype) == 0) {
		Gen.g_mode |= S_IFREG;
	}

	/* Check for SUN Style tar files */
	if (dblock.dbuf.name[strlen(dblock.dbuf.name)-1] == '/')
		sunflag = 1;
#endif 

	sp->st_mode = Gen.g_mode;
	sp->st_uid = Gen.g_uid;
	sp->st_gid = Gen.g_gid;
	sp->st_size = Gen.g_filesz;
	sp->st_mtime = Gen.g_mtime;

	if (!strncmp(dblock.dbuf.magic, "ustar", 5)) {
		(void) sscanf(dblock.dbuf.devmajor, "%8lo", &Gen.g_devmajor);
		(void) sscanf(dblock.dbuf.devminor, "%8lo", &Gen.g_devminor);
		if (dblock.dbuf.extno != '\0') {	/* split file? */
			extno = dblock.dbuf.extno;
			extotal = dblock.dbuf.extotal;
			(void) sscanf(dblock.dbuf.efsize, "%10lo", &efsize);
		} else
			extno = 0;	/* tell others file not split */
	} else {
		/* We are reading in an old xenix tar archive that may	*
		 * have files split across floppies. If this is the case*
		 * then the information we need is in the 20 bytes	*
		 * following the linkname field in the header.  The	*
		 * extension number is in the first four bytes of the	*
		 * magic field.  The total number of extensions is in	*
		 * the last two bytes of the magic field and the first	*
		 * two bytes of the version field.  The size of each 	*
		 * extension is in the first twelve bytes of the uname	*
		 * field.						*/
		

		strncpy(&old_extno[0], &dblock.dbuf.magic[0], 4);
		if (old_extno != '\0') {
			strncpy(a, &dblock.dbuf.magic[4], 2);
			strncpy(b, &dblock.dbuf.version[0], 2);
			strncat(a,b,2);
			strncpy(&old_efsize[0], &dblock.dbuf.uname[0],12);
			(void)sscanf(old_extno, "%04o", &extno);
			(void)sscanf(a, "%04o", &extotal);
			(void)sscanf(old_efsize, "%lo", &efsize);
		} else
			extno = 0;
	}
	
	chksum = Gen.g_cksum;
	if (chksum != checksum()) {
		(void) fprintf(stderr, "tar: directory checksum error\n");
		if (iflag)
			goto top;
		done(2);
	}

#ifdef i386
	if (sunflag) 
		dblock.dbuf.typeflag = '5';
#endif

	Gen.g_mode &= MODEMASK1;
	if (tfile != NULL)
		(void) fprintf(tfile, "%s %s\n", dblock.dbuf.name, dblock.dbuf.mtime);
}



/***    passtape - skip over a file on the tape
 *
 *      passtape skips over the next data file on the tape.
 *      The tape directory entry must be in dblock.dbuf.  This
 *      routine just eats the number of blocks computed from the
 *      directory size entry; the tape must be (logically) positioned
 *      right after thee directory info.
 */
static	void
passtape()
{
	long blocks;
	char buf[TBLOCK];

	/*
	 * Types link(1), sym-link(2), char special(3), blk special(4),
	 *  directory(5), and FIFO(6) do not have data blocks associated
	 *  with them so just skip reading the data block.
	 */
	if (dblock.dbuf.typeflag == '1' || dblock.dbuf.typeflag == '2' ||
		dblock.dbuf.typeflag == '3' || dblock.dbuf.typeflag == '4' ||
		dblock.dbuf.typeflag == '5' || dblock.dbuf.typeflag == '6')
		return;
	blocks = TBLOCKS(stbuf.st_size);

	/* if operating on disk, seek instead of reading */
	if (NotTape && !sflag)
		seekdisk(blocks);
	else
		while (blocks-- > 0)
			readtape(buf);
}

static	void
putfile(longname, shortname, parent, lev)
char *longname;
char *shortname;
char *parent;
int lev;
{
	static char *getmem();
	int infile;
	long blocks;
	char buf[TBLOCK];
	char filetmp[NAMSIZ];
	register char *cp;
	char *name;
	struct dirent *dp;
	struct passwd *dpasswd;
	struct group *dgroup;
	DIR *dirp;
	int i = 0;
	long l;
	int split = 0;
	char newparent[MAXNAM+64];
	char tchar = NULL;
	char *prefix = NULL;
	char *tmpbuf = NULL;
	char goodbuf[256];
	char junkbuf[256];
	char abuf[155];
	char *lastslash;

	for(i=0; i <MAXNAM; i++)
		goodbuf[i] = '\0';
	for(i=0; i < NAMSIZ; i++)
		junkbuf[i] = '\0';
	for(i=0; i < PRESIZ; i++)
		abuf[i] = '\0';
	if (lev >= MAXLEV) {
		/*
		 * Notice that we have already recursed, so we have already
		 * allocated our frame, so things would in fact work for this
		 * level.  We put the check here rather than before each
		 * recursive call because it is cleaner and less error prone.
		 */
		(void) fprintf(stderr, "tar: directory nesting too deep, %s not dumped\n", longname);
		return;
	}
	if (!hflag)
		i = lstat(shortname, &stbuf);
	else
		i = stat(shortname, &stbuf);

	if (i < 0) {
		(void) fprintf(stderr, "tar: could not stat %s\n",longname);
		return;
	}

	/*
	 * Check if the input file is the same as the tar file we
	 * are creating
	 */
	if((mt_ino == stbuf.st_ino) && (mt_dev == stbuf.st_dev)) {
		(void) fprintf(stderr, "tar: %s same as archive file\n", longname);
		Errflg = 1;
		return;
	}

	if (tfile != NULL && checkupdate(longname) == 0) {
		return;
	}
	if (checkw('r', longname) == 0) {
		return;
	}

	if (Xflag) {
		if (is_in_table(exclude_tbl, longname)) {
			if (vflag) {
				(void) fprintf(vfile, "a %s excluded\n", longname);
			}
			return;
		}
	}
	/* If the length of the fullname is greater than 256,
	   print out a message and return.
	*/

	if ((split = strlen(longname)) > MAXNAM) {
		(void) fprintf(stderr, "tar: %s: file name too long\n",
		    longname);
		if (eflag)
			done(1);
		return;
	} else if (split > NAMSIZ) {
		/* The length of the fullname is greater than 100, so
		   we must split the filename from the path
		*/
		(void) strcpy(&goodbuf[0], longname);
		tmpbuf = goodbuf;
		lastslash = strrchr(tmpbuf, '/');
		i = strlen(lastslash++);
		/* If the filename is greater than 100 we can't
		   archive the file
		*/
		if (i > NAMSIZ) {
			(void) fprintf(stderr, "tar: %s: filename is greater than %d\n",
			lastslash, NAMSIZ);
			if (eflag)
				done(1);
			return;
		}
		(void) strncpy(&junkbuf[0], lastslash, strlen(lastslash));
		/* If the prefix is greater than 155 we can't archive the
		   file.
		*/
		if ((split - i) > PRESIZ) {
			(void) fprintf(stderr, "tar: %s: prefix is greater than %d\n",
			longname, PRESIZ);
			if (eflag)
				done(1);
			return;
		}
		(void) strncpy(&abuf[0], &goodbuf[0], split - i);
		name = junkbuf;
		prefix = abuf;
	} else {
		name = longname;
	}
	if (Aflag)
		if (prefix != NULL)
			while (*prefix == '/')
				++prefix;
		else
			while(*name == '/')
				++name;

	switch (stbuf.st_mode & S_IFMT) {
	case S_IFDIR:
		stbuf.st_size = 0;
		blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
		infile = open(shortname, 0);
		if (infile < 0) {
			(void) fprintf(stderr, "tar: %s: cannot open file\n", longname);
			Errflg = 1;
			return;
		}
		for (i = 0, cp = buf; *cp++ = longname[i++];)
			;
		*--cp = '/';
		*++cp = 0  ;
		if (!oflag) {
			tomodes(&stbuf);
			(void) sprintf(dblock.dbuf.name, "%s", name);
			dblock.dbuf.typeflag = '5';
			(void) sprintf(dblock.dbuf.linkname, "%s", "\0");
			(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
			(void) sprintf(dblock.dbuf.version, "%2s", "00");
			dpasswd = getpwuid(stbuf.st_uid);
			if (dpasswd == (struct passwd *) NULL)
				(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
			else
				(void) sprintf(dblock.dbuf.uname, "%s",  dpasswd->pw_name);
			dgroup = getgrgid(stbuf.st_gid);
			if (dgroup == (struct group *) NULL)
				(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
			else
				(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
			(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_dev));
			(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_dev));
			(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
			(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
			dblock.dbuf.typeflag = '5';
			writetape((char *)&dblock);
		}
		if (vflag) {
			if (NotTape)
				(void) printf("seek = %luK\t", K(tapepos));
			(void) printf("a %s/ ", longname);
			if (NotTape)
				(void) printf("%luK\n", K(blocks));
			else
				(void) printf("%lu tape blocks\n", blocks);
		}
		if (*shortname != '/')
			(void) sprintf(newparent, "%s/%s", parent, shortname);
		else
			(void) sprintf(newparent, "%s", shortname);
		if (chdir(shortname) < 0) {
			vperror(0, newparent);
			(void) close(infile);
			return;
		}
		if ((dirp = opendir(".")) == NULL) {
			vperror(0, "can't open directory %s", longname);
			if (chdir(parent) < 0)
				vperror(0, "cannot change back?: %s", parent);
			(void) close(infile);
			return;
		}
		while ((dp = readdir(dirp)) != NULL && !term) {
			if (!strcmp(".", dp->d_name) ||
			    !strcmp("..", dp->d_name))
				continue;
			(void) strcpy(cp, dp->d_name);
			l = telldir(dirp);
			(void) closedir(dirp);
			putfile(buf, cp, newparent, lev + 1);
			dirp = opendir(".");
			seekdir(dirp, l);
		}
		(void) closedir(dirp);
		if (chdir(parent) < 0)
			vperror(0, "cannot change back?: %s", parent);
		(void) close(infile);
		break;

	case S_IFLNK:
		if (stbuf.st_size + 1 >= NAMSIZ) {
			(void) fprintf(stderr, "tar: %s: symbolic link too long\n",
			    longname);
			if (eflag)
				done(1);
			return;
		}
		/*
		 * Sym-links need header size of zero since you
		 * don't store any data for this type.
		 */
		stbuf.st_size = 0;
		tomodes(&stbuf);
		(void) sprintf(dblock.dbuf.name, "%s",  name);
		i = readlink(shortname, filetmp, NAMSIZ - 1);
		if (i < 0) {
			vperror(0, "can't read symbolic link %s", longname);
			return;
		}
		(void) sprintf(dblock.dbuf.linkname, "%s", filetmp);
		dblock.dbuf.typeflag = '2';
		if (vflag)
			(void) fprintf(vfile, "a %s symbolic link to %s\n",
			    longname, filetmp);
		(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
		(void) sprintf(dblock.dbuf.version, "%2s", "00");
		dpasswd = getpwuid(stbuf.st_uid);
		if (dpasswd == (struct passwd *) NULL)
			(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.uname, "%s",  dpasswd->pw_name);
		dgroup = getgrgid(stbuf.st_gid);
		if (dgroup == (struct group *) NULL)
			(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
		(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_dev));
		(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_dev));
		(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
		(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
		dblock.dbuf.typeflag = '2';
		writetape((char *)&dblock);
		break;
	case S_IFREG:
		if ((infile = open(shortname, 0)) < 0) {
			vperror(0, longname);
			return;
		}

		blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
		if (stbuf.st_nlink > 1) {
			struct linkbuf *lp;
			int found = 0;

			for (lp = ihead; lp != NULL; lp = lp->nextp)
				if (lp->inum == stbuf.st_ino &&
				    lp->devnum == stbuf.st_dev) {
					found++;
					break;
				}
			if (found) {
				if(strlen(lp->pathname) > (size_t)(NAMSIZ -1)) {
					(void) fprintf(stderr, "tar: %s: linked to %s\n", longname, lp->pathname);
					(void) fprintf(stderr, "tar: %s: linked name too long\n", lp->pathname);
					if (eflag)
						done(1);
					(void) close(infile);
					return;
				}
				stbuf.st_size = 0;
				tomodes(&stbuf);
				(void) sprintf(dblock.dbuf.name, "%s", name);
				(void) sprintf(dblock.dbuf.linkname, "%s", lp->pathname);
				dblock.dbuf.typeflag = '1';
				(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
				(void) sprintf(dblock.dbuf.version, "%s", "00");
				dpasswd = getpwuid(stbuf.st_uid);
				if (dpasswd == (struct passwd *) NULL)
					(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
				else
					(void) sprintf(dblock.dbuf.uname, "%s", dpasswd->pw_name);
				dgroup = getgrgid(stbuf.st_gid);
				if (dgroup == (struct group *) NULL)
					(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
				else
					(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
				(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_dev));
				(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_dev));
				(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
				(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
				dblock.dbuf.typeflag = '1';
				if (mulvol && tapepos + 1 >= blocklim)
					newvol();
				writetape((char *) &dblock);
				if (vflag)
					if (NotTape)
						(void) fprintf(vfile, "seek = %luK\t", K(tapepos));
					(void) fprintf(vfile, "a %s link to %s\n",
					    longname, lp->pathname);
				lp->count--;
				(void) close(infile);
				return;
			} else {
				lp = (struct linkbuf *) getmem(sizeof(*lp));
				if (lp != NULL) {
					lp->nextp = ihead;
					ihead = lp;
					lp->inum = stbuf.st_ino;
					lp->devnum = stbuf.st_dev;
					lp->count = stbuf.st_nlink - 1;
					(void) strcpy(lp->pathname, longname);
				}
			}
		}
		tomodes(&stbuf);
		(void) sprintf(dblock.dbuf.name, "%s", name);

		/* correctly handle end of volume */

		DEBUG("putfile: %s wants %lu blocks\n", longname, blocks);
		if (vflag) {
			if (NotTape)
				(void) printf("seek = %luK\t", K(tapepos));
			(void) printf("a %s ", longname);
			if (NotTape)
				(void) printf("%luK\n", K(blocks));
			else
				(void) printf("%lu tape blocks\n", blocks);
		}
		(void) sprintf(dblock.dbuf.linkname, "%s", tchar);
		dblock.dbuf.typeflag = '0';
		(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
		(void) sprintf(dblock.dbuf.version, "%2s", "00");
		dpasswd = getpwuid(stbuf.st_uid);
		if (dpasswd == (struct passwd *) NULL)
			(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.uname, "%s",  dpasswd->pw_name);
		dgroup = getgrgid(stbuf.st_gid);
		if (dgroup == (struct group *) NULL)
			(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
		(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_dev));
		(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_dev));
		(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
		(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
		dblock.dbuf.typeflag = '0';
		while (mulvol && tapepos + blocks + 1 > blocklim) { /* file won't fit */
			if (eflag) {
				if (blocks <= blocklim) {
					newvol();
					break;
				}
				(void) fprintf(stderr, "tar: Single file cannot fit on volume\n");
				done(3);
			}
			/* split only if floppy has some room and file is large */
	    		if (blocklim - tapepos >= EXTMIN && blocks + 1 >= blocklim/10) {
				splitfile(longname, infile);
				(void) close(infile);
				return;
			}
			newvol();	/* not worth it--just get new volume */
		}
		writetape((char *)&dblock);
		while ((i = read(infile, buf, TBLOCK)) > 0 && blocks > 0) {
			if (term) {
				(void) fprintf(stderr, 
					"tar: Interrupted in the middle of a file\n");
				done(Errflg);
			}

			writetape(buf);
			blocks--;
		}
		(void) close(infile);
		if (i < 0)
			vperror(0, "Read error on %s", longname);
		else if (blocks != 0 || i != 0) {
			(void) fprintf(stderr, "tar: %s: file changed size\n",
			    longname);
			if (eflag)
				done(1);
		}
		putempty(blocks);
		break;
	case S_IFIFO:
		blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
		stbuf.st_size = 0;
		if (stbuf.st_nlink > 1) {
			struct linkbuf *lp;
			int found = 0;

			tomodes(&stbuf);
			(void) sprintf(dblock.dbuf.name, "%s", name);
			for (lp = ihead; lp != NULL; lp = lp->nextp)
				if (lp->inum == stbuf.st_ino &&
				    lp->devnum == stbuf.st_dev) {
					found++;
					break;
				}
			if (found) {
				if(strlen(lp->pathname) > (size_t)(NAMSIZ -1)) {
					(void) fprintf(stderr, "tar: %s: linked to %s\n", longname, lp->pathname);
					(void) fprintf(stderr, "tar: %s: linked name too long\n", lp->pathname);
					if (eflag)
						done(1);
					return;
				}
				(void) sprintf(dblock.dbuf.linkname, "%s", lp->pathname);
				dblock.dbuf.typeflag = '6';
				(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
				(void) sprintf(dblock.dbuf.version, "%2s", "00");
				dpasswd = getpwuid(stbuf.st_uid);
				if (dpasswd == (struct passwd *) NULL)
					(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
				else
					(void) sprintf(dblock.dbuf.uname, "%s",  dpasswd->pw_name);
				dgroup = getgrgid(stbuf.st_gid);
				if (dgroup == (struct group *) NULL)
					(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
				else
					(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
				(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_dev));
				(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_dev));
				(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
				(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
				dblock.dbuf.typeflag = '6';
				
				if (mulvol && tapepos + 1 >= blocklim)
					newvol();
				writetape( (char *) &dblock);
				if (vflag)
					if (NotTape)
						(void) fprintf(vfile, "seek = %luK\t", K(tapepos));
					(void) fprintf(vfile, "a %s link to %s\n",
					    longname, lp->pathname);
				lp->count--;
				return;
			} else {
				lp = (struct linkbuf *) getmem(sizeof(*lp));
				if (lp != NULL) {
					lp->nextp = ihead;
					ihead = lp;
					lp->inum = stbuf.st_ino;
					lp->devnum = stbuf.st_dev;
					lp->count = stbuf.st_nlink - 1;
					(void) strcpy(lp->pathname, longname);
				}
			}
		}
		tomodes(&stbuf);
		(void) sprintf(dblock.dbuf.name, "%s", name);

		while (mulvol && tapepos + blocks + 1 > blocklim) { 
			if (eflag) {
				if (blocks <= blocklim) {
					newvol();
					break;
				}
				(void) fprintf(stderr, "tar: Single file cannot fit on volume\n");
				done(3);
			}

	    		if (blocklim - tapepos >= EXTMIN && blocks + 1 >= blocklim/10) {
				splitfile(longname, infile);
				return;
			}
			newvol();
		}

		DEBUG("putfile: %s wants %lu blocks\n", longname, blocks);
		if (vflag) {
			if (NotTape)
				(void) printf("seek = %luK\t", K(tapepos));
			(void) printf("a %s ", longname);
			if (NotTape)
				(void) printf("%luK\n", K(blocks));
			else
				(void) printf("%lu tape blocks\n", blocks);
		}
		(void) sprintf(dblock.dbuf.linkname, "%s", tchar);
		dblock.dbuf.typeflag = '6';
		(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
		(void) sprintf(dblock.dbuf.version, "%2s", "00");
		dpasswd = getpwuid(stbuf.st_uid);
		if (dpasswd == (struct passwd *) NULL)
			(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.uname, "%s",  dpasswd->pw_name);
		dgroup = getgrgid(stbuf.st_gid);
		if (dgroup == (struct group *) NULL)
			(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
		(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_dev));
		(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_dev));
		(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
		(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
		dblock.dbuf.typeflag = '6';
		writetape((char *)&dblock);
		break;
	case S_IFCHR:
		blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
		stbuf.st_size = 0;
		if (stbuf.st_nlink > 1) {
			struct linkbuf *lp;
			int found = 0;

			tomodes(&stbuf);
			(void) sprintf(dblock.dbuf.name, "%s", name);
			for (lp = ihead; lp != NULL; lp = lp->nextp)
				if (lp->inum == stbuf.st_ino &&
				    lp->devnum == stbuf.st_dev) {
					found++;
					break;
				}
			if (found) {
				if(strlen(lp->pathname) > (size_t)(NAMSIZ -1)) {
					(void) fprintf(stderr, "tar: %s: linked to %s\n", longname, lp->pathname);
					(void) fprintf(stderr, "tar: %s: linked name too long\n", lp->pathname);
					if (eflag)
						done(1);
					return;
				}
				stbuf.st_size = 0;
				(void) sprintf(dblock.dbuf.linkname, "%s", lp->pathname);
				dblock.dbuf.typeflag = '3';
				(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
				(void) sprintf(dblock.dbuf.version, "%2s", "00");
				dpasswd = getpwuid(stbuf.st_uid);
				if (dpasswd == (struct passwd *) NULL)
					(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
				else
					(void) sprintf(dblock.dbuf.uname, "%s",  dpasswd->pw_name);
				dgroup = getgrgid(stbuf.st_gid);
				if (dgroup == (struct group *) NULL)
					(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
				else
					(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
				(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_dev));
				(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_dev));
				(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
				(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
				dblock.dbuf.typeflag = '3';
				if (mulvol && tapepos + 1 >= blocklim)
					newvol();
				writetape( (char *) &dblock);
				if (vflag)
					if (NotTape)
						(void) fprintf(vfile, "seek = %luK\t", K(tapepos));
					(void) fprintf(vfile, "a %s link to %s\n",
					    longname, lp->pathname);
				lp->count--;
				return;
			} else {
				lp = (struct linkbuf *) getmem(sizeof(*lp));
				if (lp != NULL) {
					lp->nextp = ihead;
					ihead = lp;
					lp->inum = stbuf.st_ino;
					lp->devnum = stbuf.st_dev;
					lp->count = stbuf.st_nlink - 1;
					(void) strcpy(lp->pathname, longname);
				}
			}
		}
		tomodes(&stbuf);
		(void) sprintf(dblock.dbuf.name, "%s", name);

		while (mulvol && tapepos + blocks + 1 > blocklim) { 
			if (eflag) {
				if (blocks <= blocklim) {
					newvol();
					break;
				}
				(void) fprintf(stderr, "tar: Single file cannot fit on volume\n");
				done(3);
			}

	    		if (blocklim - tapepos >= EXTMIN && blocks + 1 >= blocklim/10) {
				splitfile(longname, infile);
				return;
			}
			newvol();
		}

		DEBUG("putfile: %s wants %lu blocks\n", longname, blocks);
		if (vflag) {
			if (NotTape)
				(void) printf("seek = %luK\t", K(tapepos));
			(void) printf("a %s ", longname);
			if (NotTape)
				(void) printf("%luK\n", K(blocks));
			else
				(void) printf("%lu tape blocks\n", blocks);
		}
		(void) sprintf(dblock.dbuf.linkname, "%s", tchar);
		dblock.dbuf.typeflag = '3';
		(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
		(void) sprintf(dblock.dbuf.version, "%2s", "00");
		dpasswd = getpwuid(stbuf.st_uid);
		if (dpasswd == (struct passwd *) NULL)
			(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.uname, "%s",  dpasswd->pw_name);
		dgroup = getgrgid(stbuf.st_gid);
		if (dgroup == (struct group *) NULL)
			(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
		(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_rdev));
		(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_rdev));
		(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
		(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
		dblock.dbuf.typeflag = '3';
		writetape((char *)&dblock);
		break;
	case S_IFBLK:
		blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
		stbuf.st_size = 0;
		if (stbuf.st_nlink > 1) {
			struct linkbuf *lp;
			int found = 0;

			tomodes(&stbuf);
			(void) sprintf(dblock.dbuf.name, "%s", name);
			for (lp = ihead; lp != NULL; lp = lp->nextp)
				if (lp->inum == stbuf.st_ino &&
				    lp->devnum == stbuf.st_dev) {
					found++;
					break;
				}
			if (found) {
				if(strlen(lp->pathname) > (size_t)(NAMSIZ -1)) {
					(void) fprintf(stderr, "tar: %s: linked to %s\n", longname, lp->pathname);
					(void) fprintf(stderr, "tar: %s: linked name too long\n", lp->pathname);
					if (eflag)
						done(1);
					return;
				}
				stbuf.st_size = 0;
				(void) sprintf(dblock.dbuf.linkname, "%s", lp->pathname);
				dblock.dbuf.typeflag = '4';
				(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
				(void) sprintf(dblock.dbuf.version, "%2s", "00");
				dpasswd = getpwuid(stbuf.st_uid);
				if (dpasswd == (struct passwd *) NULL)
					(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
				else
					(void) sprintf(dblock.dbuf.uname, "%s",  dpasswd->pw_name);
				dgroup = getgrgid(stbuf.st_gid);
				if (dgroup == (struct group *) NULL)
					(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
				else
					(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
				(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_dev));
				(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_dev));
				(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
				(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
				dblock.dbuf.typeflag = '4';
				if (mulvol && tapepos + 1 >= blocklim)
					newvol();
				writetape( (char *) &dblock);
				if (vflag)
					if (NotTape)
						(void) fprintf(vfile, "seek = %luK\t", K(tapepos));
					(void) fprintf(vfile, "a %s link to %s\n",
					    longname, lp->pathname);
				lp->count--;
				return;
			} else {
				lp = (struct linkbuf *) getmem(sizeof(*lp));
				if (lp != NULL) {
					lp->nextp = ihead;
					ihead = lp;
					lp->inum = stbuf.st_ino;
					lp->devnum = stbuf.st_dev;
					lp->count = stbuf.st_nlink - 1;
					(void) strcpy(lp->pathname, longname);
				}
			}
		}
		tomodes(&stbuf);
		(void) sprintf(dblock.dbuf.name, "%s", name);


		while (mulvol && tapepos + blocks + 1 > blocklim) {
			if (eflag) {
				if (blocks <= blocklim) {
					newvol();
					break;
				}
				(void) fprintf(stderr, "tar: Single file cannot fit on volume\n");
				done(3);
			}

	    		if (blocklim - tapepos >= EXTMIN && blocks + 1 >= blocklim/10) {
				splitfile(longname, infile);
				return;
			}
			newvol();
		}

		DEBUG("putfile: %s wants %lu blocks\n", longname, blocks);
		if (vflag) {
			if (NotTape)
				(void) printf("seek = %luK\t", K(tapepos));
			(void) printf("a %s ", longname);
			if (NotTape)
				(void) printf("%luK\n", K(blocks));
			else
				(void) printf("%lu tape blocks\n", blocks);
		}
		(void) sprintf(dblock.dbuf.linkname, "%s", tchar);
		dblock.dbuf.typeflag = '4';
		(void) sprintf(dblock.dbuf.magic, "%s", "ustar");
		(void) sprintf(dblock.dbuf.version, "%2s", "00");
		dpasswd = getpwuid(stbuf.st_uid);
		if (dpasswd == (struct passwd *) NULL)
			(void) fprintf(stderr, "tar: could not get passwd information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.uname, "%s",  dpasswd->pw_name);
		dgroup = getgrgid(stbuf.st_gid);
		if (dgroup == (struct group *) NULL)
			(void) fprintf(stderr, "tar: could not get group information for %s\n", longname);
		else
			(void) sprintf(dblock.dbuf.gname, "%s", dgroup->gr_name);
		(void) sprintf(dblock.dbuf.devmajor, "%07o", major(stbuf.st_rdev));
		(void) sprintf(dblock.dbuf.devminor, "%07o", minor(stbuf.st_rdev));
		(void) sprintf(dblock.dbuf.prefix, "%s", prefix);
		(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
		dblock.dbuf.typeflag = '4';
		writetape((char *)&dblock);
		break;
	default:
		(void) fprintf(stderr, "tar: %s is not a file. Not dumped\n",
		    longname);
		if (eflag)
			done(1);
		break;
	}
	return;
}

/***	splitfile	dump a large file across volumes
 *
 *	splitfile(longname, fd);
 *		char *longname;		full name of file
 *		int ifd;		input file descriptor
 *
 *	NOTE:  only called by putfile() to dump a large file.
 */
static	void
splitfile(longname, ifd)
char *longname;
int ifd;
{
	long blocks, bytes, s;
	char buf[TBLOCK];
	register int i = 0, extents = 0;

	blocks = TBLOCKS(stbuf.st_size);	/* blocks file needs */

	/* # extents =
	 *	size of file after using up rest of this floppy
	 *		blocks - (blocklim - tapepos) + 1	(for header)
	 *	plus roundup value before divide by blocklim-1
	 *		+ (blocklim - 1) - 1
	 *	all divided by blocklim-1 (one block for each header).
	 * this gives
	 *	(blocks - blocklim + tapepos + 1 + blocklim - 2)/(blocklim-1)
	 * which reduces to the expression used.
	 * one is added to account for this first extent.
	 */
	extents = (blocks + tapepos - 1L)/(blocklim - 1L) + 1;

	if (extents < 2 || extents > MAXEXT) {	/* let's be reasonable */
		(void) fprintf(stderr, "tar: %s needs unusual number of volumes to split\ntar: %s not dumped\n", longname, longname);
		return;
	}
	dblock.dbuf.extotal = extents;
	bytes = stbuf.st_size;
	(void)sprintf(dblock.dbuf.efsize, "%lo", bytes);

	(void) fprintf(stderr, "tar: large file %s needs %d extents.\ntar: current device seek position = %luK\n", longname, extents, K(tapepos));

	s = (blocklim - tapepos - 1) * TBLOCK;
	for (i = 1; i <= extents; i++) {
		if (i > 1) {
			newvol();
			if (i == extents)
				s = bytes;	/* last ext. gets true bytes */
			else
				s = (blocklim - 1)*TBLOCK; /* whole volume */
		}
		bytes -= s;
		blocks = TBLOCKS(s);

		(void) sprintf(dblock.dbuf.size, "%011lo", s);
		dblock.dbuf.extno = i;
		(void) sprintf(dblock.dbuf.chksum, "%07o", checksum());
		writetape( (char *) &dblock);

		if (vflag)
			(void) printf("+++ a %s %luK [extent #%d of %d]\n",
				longname, K(blocks), i, extents);
		while (blocks > 0 && read(ifd, buf, TBLOCK) > 0) {
			blocks--;
			writetape(buf);
		}
		if (blocks != 0) {
			(void) fprintf(stderr, "tar: %s: file changed size\n", longname);
			(void) fprintf(stderr, "tar: aborting split file %s\n", longname);
			(void) close(ifd);
			return;
		}
	}
	(void) close(ifd);
	if (vflag)
		(void) printf("a %s %luK (in %d extents)\n",
			longname, K(TBLOCKS(stbuf.st_size)), extents);
}

static	void
doxtract(argv)
char	*argv[];
{
	struct	stat	xtractbuf;	/* stat on file after extracting */
	long blocks, bytes;
	char *curarg;
	int ofile;
	int newfile;			/* Does the file already exist  */
	int xcnt = 0;			/* count # files extracted */
	int fcnt = 0;			/* count # files in argv list */
	int dir = 0;
	ushort Uid;
	char *namep, *linkp;		/* for removing absolute paths */
	extern errno;
	char fullname[256];
	char dirname[256];
	char *preptr;
	int k = 0;
	int j;
	struct passwd *dpasswd;
	struct group *dgroup;
	int once = 1;
	int symflag;

	dumping = 0;	/* for newvol(), et al:  we are not writing */

	/*
	 * Count the number of files that are to be extracted 
	 */
	Uid = getuid();
	initarg(argv, Filefile);
	while (nextarg() != NULL)
		++fcnt;

	for (;;) {
		for (j = 0; j < MAXNAM; j++)
			fullname[j] = '\0';
		symflag = 0;
		namep = linkp = (char *) NULL;
		dir = 0;
		getdir();
		preptr = dblock.dbuf.prefix;
		if (*preptr != (char) NULL) {
			k = strlen(&dblock.dbuf.prefix[0]);
			if (k < PRESIZ) {
				(void) strcpy(&fullname[0], dblock.dbuf.prefix);
				j = 0;
				fullname[k++] = '/';
				while ((j < NAMSIZ) && (dblock.dbuf.name[j] != (char) NULL)) {
					fullname[k] = dblock.dbuf.name[j];
					k++; j++;
				} 
			} else if (k >= PRESIZ) {
					k = 0;
					while ((k < PRESIZ) && (dblock.dbuf.prefix[k] != (char) NULL)) {
						fullname[k] = dblock.dbuf.prefix[k];
						k++;
					}
					fullname[k++] = '/';
					j = 0;
					while ((j < NAMSIZ) && (dblock.dbuf.name[j] != (char) NULL)) {
						fullname[k] = dblock.dbuf.name[j];
						k++; j++;
					} 
			}
			namep = &fullname[0];
		} else
			namep = dblock.dbuf.name;
		if (Xflag) {
			if (is_in_table(exclude_tbl, namep)) {
				if (vflag)
					(void) fprintf(stderr, "%s excluded\n", namep);
				passtape();
				continue;
			}
		}
		if (Iflag) {
			if (is_in_table(include_tbl, namep))
				goto gotit;
		}
		initarg(argv, Filefile);
		if (endtape())
			break;
		if (Aflag)
			while (*namep == '/')  /* step past leading slashes */
				namep++;
		if ((curarg = nextarg()) == NULL)
			goto gotit;

		for ( ; curarg != NULL; curarg = nextarg())
			if (prefix(curarg, namep))
				goto gotit;
		passtape();
		continue;

gotit:
		if (checkw('x', namep) == 0) {
			passtape();
			continue;
		}
		if (once) {
			if (!strncmp(dblock.dbuf.magic, "ustar", 5)) {
				if (geteuid() == (uid_t) 0) {
					checkflag = 1;
					pflag = 1;
				} else {
					Oumask = umask(0); 	/* get file creation mask */
					(void) umask(Oumask);
				}
				once = 0;
			} else {
				if (geteuid() == (uid_t) 0) {
					pflag = 1;
					checkflag = 2;
				}
				if (!pflag) {
					Oumask = umask(0); 	/* get file creation mask */
					(void) umask(Oumask);
				}
				once = 0;
			}
		}

		(void) strcpy(&dirname[0], namep);
		if (checkdir(&dirname[0]) && (dblock.dbuf.typeflag == '5')) {
			dir = 1;
			if (vflag) {
				(void) printf("x %s, 0 bytes, ", &dirname[0]);
				if (NotTape)
					(void) printf("0K\n");
				else
					(void) printf("0 tape blocks\n");
			}
			goto filedone;
		}
		if (dblock.dbuf.typeflag == '6') {	/* FIFO */
			if(rmdir(namep) < 0) {
				if (errno == ENOTDIR)
					(void) unlink(namep);
			}
			linkp = dblock.dbuf.linkname; 
			if (*linkp !=  NULL) {
				if (Aflag && *linkp == '/')
					linkp++;
				if (link(linkp, namep) < 0) {
					(void) fprintf(stderr, "tar: %s: cannot link\n",namep);
					continue;
				}
				if (vflag)
					(void) printf("%s linked to %s\n", namep, linkp);
				xcnt++;		/* increment # files extracted */
				continue;
			}
			if (mknod(namep, (int)(Gen.g_mode|S_IFIFO), (int)Gen.g_devmajor) < 0) {
				vperror(0, "%s: mknod failed", namep);
				continue;
			}
			blocks = TBLOCKS(bytes = stbuf.st_size);
			if (vflag) {
				(void) printf("x %s, %lu bytes, ", namep, bytes);
				if (NotTape)
					(void) printf("%luK\n", K(blocks));
				else
					(void) printf("%lu tape blocks\n", blocks);
			}
			goto filedone;
		}
		if (dblock.dbuf.typeflag == '3' && !Uid) {	/* CHAR SPECIAL */
			if(rmdir(namep) < 0) {
				if (errno == ENOTDIR)
					(void) unlink(namep);
			}
			linkp = dblock.dbuf.linkname; 
			if (*linkp != NULL) {
				if (Aflag && *linkp == '/')
					linkp++;
				if (link(linkp, namep) < 0) {
					(void) fprintf(stderr, "tar: %s: cannot link\n",namep);
					continue;
				}
				if (vflag)
					(void) printf("%s linked to %s\n", namep, linkp);
				xcnt++;		/* increment # files extracted */
				continue;
			}
			if (mknod(namep, (int)(Gen.g_mode|S_IFCHR), (int)makedev(Gen.g_devmajor, Gen.g_devminor)) < 0) {
				vperror(0, "%s: mknod failed", namep);
				continue;
			}
			blocks = TBLOCKS(bytes = stbuf.st_size);
			if (vflag) {
				(void) printf("x %s, %lu bytes, ", namep, bytes);
				if (NotTape)
					(void) printf("%luK\n", K(blocks));
				else
					(void) printf("%lu tape blocks\n", blocks);
			}
			goto filedone;
		} else if (dblock.dbuf.typeflag == '3' && Uid) {
			(void) fprintf(stderr, "Can't create special %s\n", namep);
			continue;
		}
		if (dblock.dbuf.typeflag == '4' && !Uid) {	/* BLOCK SPECIAL */
			if(rmdir(namep) < 0) {
				if (errno == ENOTDIR)
					(void) unlink(namep);
			}
			linkp = dblock.dbuf.linkname; 
			if (*linkp != NULL) {
				if (Aflag && *linkp == '/')
					linkp++;
				if (link(linkp, namep) < 0) {
					(void) fprintf(stderr, "tar: %s: cannot link\n",namep);
					continue;
				}
				if (vflag)
					(void) printf("%s linked to %s\n", namep, linkp);
				xcnt++;		/* increment # files extracted */
				continue;
			}
			if (mknod(namep, (int)(Gen.g_mode|S_IFBLK), (int)makedev(Gen.g_devmajor, Gen.g_devminor)) < 0) {
				vperror(0, "%s: mknod failed", namep);
				continue;
			}
			blocks = TBLOCKS(bytes = stbuf.st_size);
			if (vflag) {
				(void) printf("x %s, %lu bytes, ", namep, bytes);
				if (NotTape)
					(void) printf("%luK\n", K(blocks));
				else
					(void) printf("%lu tape blocks\n", blocks);
			}
			goto filedone;
		} else if (dblock.dbuf.typeflag == '4' && Uid) {
			(void) fprintf(stderr, "Can't create special %s\n", namep);
			continue;
		}
		if (dblock.dbuf.typeflag == '2') {	/* symlink */
			linkp = dblock.dbuf.linkname; 
			if (Aflag && *linkp == '/')
				linkp++;
			if (rmdir(namep) < 0) {
				if (errno == ENOTDIR)
					(void) unlink(namep);
			}
			if (symlink(linkp, namep)<0) {
				vperror(0, "%s: symbolic link failed", namep);
				continue;
			}
			if (vflag)
				(void) fprintf(vfile, "x %s symbolic link to %s\n",
				    dblock.dbuf.name, linkp);
			symflag = 1;
			goto filedone;
		}
		if (dblock.dbuf.typeflag == '1') {
			linkp = dblock.dbuf.linkname; 
			if (Aflag && *linkp == '/')
				linkp++;
			if (rmdir(namep) < 0) {
				if (errno == ENOTDIR)
					(void) unlink(namep);
			}
			if (link(linkp, namep) < 0) {
				(void) fprintf(stderr, "tar: %s: cannot link\n",namep);
				continue;
			}
			if (vflag)
				(void) printf("%s linked to %s\n", namep, linkp);
			xcnt++;		/* increment # files extracted */
			continue;
		}

		if(dblock.dbuf.typeflag == '0'|| dblock.dbuf.typeflag == NULL) {
			if (rmdir(namep) < 0) {
				if (errno == ENOTDIR)
					(void) unlink(namep);
			}
			linkp = dblock.dbuf.linkname; 
			if (*linkp != NULL) {
				if (Aflag && *linkp == '/')
					linkp++;
				if (link(linkp, namep) < 0) {
					(void) fprintf(stderr, "tar: %s: cannot link\n",namep);
					continue;
				}
				if (vflag)
					(void) printf("%s linked to %s\n", namep, linkp);
				xcnt++;		/* increment # files extracted */
				continue;
			}
		newfile = ((stat(namep, &xtractbuf) == -1) ? TRUE : FALSE);
		if ((ofile = creat(namep, stbuf.st_mode & MODEMASK)) < 0) {
			(void) fprintf(stderr, "tar: %s - cannot create\n", namep);
			passtape();
			continue;
		}

		if (extno != 0)	{	/* file is in pieces */
			if (extotal < 1 || extotal > MAXEXT)
				(void) fprintf(stderr, "tar: ignoring bad extent info for %s\n", namep);
			else {
				xsfile(ofile);	/* extract it */
				extno = 0;
				goto filedone;
			}
		}
		extno = 0;	/* let everyone know file is not split */
		blocks = TBLOCKS(bytes = stbuf.st_size);
		if (vflag) {
			(void) printf("x %s, %lu bytes, ", namep, bytes);
			if (NotTape)
				(void) printf("%luK\n", K(blocks));
			else
				(void) printf("%lu tape blocks\n", blocks);
		}

		xblocks(bytes, ofile);
filedone:
		if (mflag == 0 && !symflag) {
			time_t timep[2];

			timep[0] = time((long *) 0);
			timep[1] = stbuf.st_mtime;
			if (utime(namep, timep) < 0)
				vperror(0, "can't set time on %s", namep);
		}
		/* moved this code from above */
		if (pflag && !symflag) 
			(void) chmod(namep, stbuf.st_mode & MODEMASK);
		if (!oflag)
			if (checkflag == 1) {
				if ((dpasswd = getpwnam(&dblock.dbuf.uname[0])) == (struct passwd *)NULL) {
					(void) fprintf(stderr, "tar: problem reading passwd entry\n");
					(void) fprintf(stderr, "tar: %s: owner not changed\n", namep);
				}
				if ((dgroup = getgrnam(&dblock.dbuf.gname[0])) == (struct group *)NULL) {
					(void) fprintf(stderr, "tar: problem reading group entry\n");
					(void) fprintf(stderr, "tar: %s: group not changed\n", namep);
				}
				if (symflag)
					(void) lchown(namep, dpasswd->pw_uid, dgroup->gr_gid);
				else
					(void) chown(namep, dpasswd->pw_uid, dgroup->gr_gid);
			} else if (checkflag == 2)
				if (symflag)
					(void) lchown(namep, stbuf.st_uid, stbuf.st_gid);
				else
					(void) chown(namep, stbuf.st_uid, stbuf.st_gid);

		if (!dir && (dblock.dbuf.typeflag == '0' || dblock.dbuf.typeflag == NULL || dblock.dbuf.typeflag == '1')) {
			if (fstat(ofile, &xtractbuf) == -1)
				(void) fprintf(stderr, "tar: cannot stat extracted file %s\n", ofile);
			if (newfile == TRUE && pflag && (xtractbuf.st_mode & MODEMASK) 
					!= ((stbuf.st_mode & ~Oumask) & MODEMASK))
				(void) fprintf(stderr, "tar: warning - file permissions have changed for %s\n", namep);
			(void) close(ofile);
		}
		xcnt++;			/* increment # files extracted */
		}
	}
	if (sflag) {
		getempty(1L);	/* don't forget extra EOT */
	}

	/*
	 * Check if the number of files extracted is different from the
	 * number of files listed on the command line 
	 */
	if (fcnt > xcnt ) {
		(void) fprintf(stderr, "tar: %d file(s) not extracted\n",fcnt-xcnt);
		Errflg = 1;
	}
}

/***	xblocks		extract file/extent from tape to output file	
 *
 *	xblocks(bytes, ofile);
 *		long bytes;	size of extent or file to be extracted
 *
 *	called by doxtract() and xsfile()
 */
static	void
xblocks(bytes, ofile)
long bytes;
int ofile;
{
	long blocks;
	char buf[TBLOCK];

	blocks = TBLOCKS(bytes);
	while (blocks-- > 0) {
		readtape(buf);
		if (bytes > TBLOCK) {
			if (write(ofile, buf, TBLOCK) < 0) {
exwrterr:
				(void) fprintf(stderr, "tar: %s: HELP - extract write error\n", dblock.dbuf.name);
				done(2);
			}
		} else
			if (write(ofile, buf, (int) bytes) < 0)
				goto exwrterr;
		bytes -= TBLOCK;
	}
}



/***	xsfile	extract split file
 *
 *	xsfile(ofd);	ofd = output file descriptor
 *
 *	file extracted and put in ofd via xblocks()
 *
 *	NOTE:  only called by doxtract() to extract one large file
 */

static	union	hblock	savedblock;	/* to ensure same file across volumes */

static	void
xsfile(ofd)
int ofd;
{
	register i, c;
	char name[NAMSIZ];	/* holds name for diagnostics */
	int extents, totalext;
	long bytes, totalbytes;

	(void) strncpy(name, dblock.dbuf.name, NAMSIZ); /* so we don't lose it */
	totalbytes = 0L;	/* in case we read in half the file */
	totalext = 0;		/* these keep count */

	(void) fprintf(stderr, "tar: %s split across %d volumes\n", name, extotal);

	/* make sure we do extractions in order */
	if (extno != 1) {	/* starting in middle of file? */
		(void) printf("tar: first extent read is not #1\nOK to read file beginning with extent #%d (y/n) ? ", extno);
		if (response() != 'y') {
canit:
			passtape();
			(void) close(ofd);
			return;
		}
	}
	extents = extotal;
	for (i = extno; ; ) {
		bytes = stbuf.st_size;
		if (vflag)
			(void) printf("+++ x %s [extent #%d], %lu bytes, %luK\n",
				name, extno, bytes, K(TBLOCKS(bytes)));
		xblocks(bytes, ofd);

		totalbytes += bytes;
		totalext++;
		if (++i > extents)
			break;

		/* get next volume and verify it's the right one */
		copy(&savedblock, &dblock);
tryagain:
		newvol();
		getdir();
		if (endtape()) {	/* seemingly empty volume */
			(void) fprintf(stderr, "tar: first record is null\n");
asknicely:
			(void) fprintf(stderr, "tar: need volume with extent #%d of %s\n", i, name);
			goto tryagain;
		}
		if (notsame()) {
			(void) fprintf(stderr, "tar: first file on that volume is not the same file\n");
			goto asknicely;
		}
		if (i != extno) {
			(void) fprintf(stderr, "tar: extent #%d received out of order\ntar: should be #%d\n", extno, i);
			(void) fprintf(stderr, "Ignore error, Abort this file, or load New volume (i/a/n) ? ");
			c = response();
			if (c == 'a')
				goto canit;
			if (c != 'i')		/* default to new volume */
				goto asknicely;
			i = extno;		/* okay, start from there */
		}
	}
	bytes = stbuf.st_size;
	if (vflag)
		(void) printf("x %s (in %d extents), %lu bytes, %luK\n",
			name, totalext, totalbytes, K(TBLOCKS(totalbytes)));
}



/***	notsame()	check if extract file extent is invalid
 *
 *	returns true if anything differs between savedblock and dblock
 *	except extno (extent number), checksum, or size (extent size).
 *	Determines if this header belongs to the same file as the one we're
 *	extracting.
 *
 *	NOTE:	though rather bulky, it is only called once per file
 *		extension, and it can withstand changes in the definition
 *		of the header structure.
 *
 *	WARNING:	this routine is local to xsfile() above
 */
static	int
notsame()
{
	return(
	    strncmp(savedblock.dbuf.name, dblock.dbuf.name, NAMSIZ)
	    || strcmp(savedblock.dbuf.mode, dblock.dbuf.mode)
	    || strcmp(savedblock.dbuf.uid, dblock.dbuf.uid)
	    || strcmp(savedblock.dbuf.gid, dblock.dbuf.gid)
	    || strcmp(savedblock.dbuf.mtime, dblock.dbuf.mtime)
	    || savedblock.dbuf.typeflag != dblock.dbuf.typeflag
	    || strncmp(savedblock.dbuf.linkname, dblock.dbuf.linkname, NAMSIZ)
	    || savedblock.dbuf.extotal != dblock.dbuf.extotal
	    || strcmp(savedblock.dbuf.efsize, dblock.dbuf.efsize)
	);
}


static	void
dotable(argv)
char	*argv[];
{
	char *curarg;
	int tcnt;			/* count # files tabled*/
	int fcnt;			/* count # files in argv list */
	char fullname[256];
	char *preptr;
	char *namep;
	int k = 0;
	int j;

	dumping = 0;

	/* if not on magtape, maximize seek speed */
	if (NotTape && !bflag) {
#if SYS_BLOCK > TBLOCK
		nblock = SYS_BLOCK / TBLOCK;
#else
		nblock = 1;
#endif
	}
	/*
	 * Count the number of files that are to be tabled
	 */
	fcnt = tcnt = 0;
	initarg(argv, Filefile);
	while (nextarg() != NULL)
		++fcnt;

	for (;;) {
		for (j = 0; j < MAXNAM; j++)
			fullname[j] = '\0';
		getdir();
		preptr = dblock.dbuf.prefix;
		if (*preptr != (char) NULL) {
			k = strlen(&dblock.dbuf.prefix[0]);
			if (k < PRESIZ) {
				(void) strcpy(&fullname[0], dblock.dbuf.prefix);
				j = 0;
				fullname[k++] = '/';
				while ((j < NAMSIZ) && (dblock.dbuf.name[j] != (char) NULL)) {
					fullname[k] = dblock.dbuf.name[j];
					k++; j++;
				} 
			} else if (k >= PRESIZ) {
					k = 0;
					while ((k < PRESIZ) && (dblock.dbuf.prefix[k] != (char) NULL)) {
						fullname[k] = dblock.dbuf.prefix[k];
						k++;
					}
					fullname[k++] = '/';
					j = 0;
					while ((j < NAMSIZ) && (dblock.dbuf.name[j] != (char) NULL)) {
						fullname[k] = dblock.dbuf.name[j];
						k++; j++;
					} 
			}
			namep = &fullname[0];
		} else
			namep = dblock.dbuf.name;
		if (Xflag) {
			if (is_in_table(exclude_tbl, namep)) {
				if (vflag) {
					(void) fprintf(stderr, "%s excluded\n", namep);
				}
				passtape();
				continue;
			}
		}
		if (Iflag) {
			if (is_in_table(include_tbl, namep))
				goto tableit;
		}
		initarg(argv, Filefile);
		if (endtape())
			break;
		if ((curarg = nextarg()) == NULL)
			goto tableit;
		for ( ; curarg != NULL; curarg = nextarg())
			if (prefix(curarg, dblock.dbuf.name))
				goto tableit;
		passtape();
		continue;
tableit:
		++tcnt;
		if (vflag)
			longt(&stbuf);
		(void) printf("%s", namep);
		if (extno != 0) {
			if (vflag)
				(void) printf("\n [extent #%d of %d] %lu bytes total",
					extno, extotal, efsize);
			else
				(void) printf(" [extent #%d of %d]", extno, extotal);
		}
		if (dblock.dbuf.typeflag == '1')
			(void) printf(" linked to %s", dblock.dbuf.linkname);
		if (dblock.dbuf.typeflag == '2')
			(void) printf(" symbolic link to %s", dblock.dbuf.linkname);
		(void) printf("\n");
		passtape();
	}
	if (sflag) {
		getempty(1L);	/* don't forget extra EOT */
	}
	/*
	 * Check if the number of files tabled is different from the
	 * number of files listed on the command line
	 */
	if (fcnt > tcnt ) {
		(void) fprintf(stderr, "tar: %d file(s) not found\n",fcnt-tcnt);
		Errflg = 1;
	}
}

static	void
putempty(n)
register long n;		/* new argument 'n' */
{
	char buf[TBLOCK];
	register char *cp;

	for (cp = buf; cp < &buf[TBLOCK]; )
		*cp++ = '\0';
	while (n-- > 0)
		writetape(buf);
	return;
}

static	void
getempty(n)
register long n;
{
	char buf[TBLOCK];
	register char *cp;

	if (!sflag)
		return;
	for (cp = buf; cp < &buf[TBLOCK]; )
		*cp++ = '\0';
	while (n-- > 0)
		sumupd(&Si, buf, TBLOCK);
	return;
}

static	void
verbose(st)
struct stat *st;
{
	register int i, j, temp;
	mode_t mode;
	char modestr[11];

	for (i = 0; i < 10; i++)
		modestr[i] = '-';
	modestr[i] = '\0';

	mode = st->st_mode;
	for (i = 0; i < 3; i++) {
		temp = (mode >> (6 - (i * 3)));
		j = (i * 3) + 1;
		if (S_IROTH & temp)
			modestr[j] = 'r';
		if (S_IWOTH & temp)
			modestr[j + 1] = 'w';
		if (S_IXOTH & temp)
			modestr[j + 2] = 'x';
	}
	temp = st->st_mode & Ftype;
	switch (temp) {
		case (S_IFIFO):
			modestr[0] = 'p';
			break;
		case (S_IFCHR):
			modestr[0] = 'c';
			break;
		case (S_IFDIR):
			modestr[0] = 'd';
			break;
		case (S_IFBLK):
			modestr[0] = 'b';
			break;
		case (S_IFREG): /* was initialized to '-' */
			break;
		case (S_IFLNK):
			modestr[0] = 'l';
			break;
		default:
			(void) fprintf(stderr, "tar: impossible file type");
	}
	if ((S_ISUID & Gen.g_mode) == S_ISUID)
		modestr[3] = 's';
	if ((S_ISVTX & Gen.g_mode) == S_ISVTX)
		modestr[9] = 't';
	if ((S_ISGID & Gen.g_mode) == S_ISGID && modestr[6] == 'x')
		modestr[6] = 's';
	else if ((S_ENFMT & Gen.g_mode) == S_ENFMT && modestr[6] != 'x')
		modestr[6] = 'l';
	(void)printf("%s", modestr);
}

static	void
longt(st)
register struct stat *st;
{
	register char *cp;

	verbose(st);
	(void) printf("%3d/%-3d", st->st_uid, st->st_gid);
	if (dblock.dbuf.typeflag == '2')	
		st->st_size = strlen(dblock.dbuf.linkname);
	(void) printf("%7lu", st->st_size);
	cp = ctime(&st->st_mtime);
	(void) printf(" %-12.12s %-4.4s ", cp+4, cp+20);
}

/*
 * Make all directories needed by `name'.  If `name' is itself
 * a directory on the tar tape (indicated by a trailing '/'),
 * return 1; else 0.
 */
static	int
checkdir(name)
	register char *name;
{
	register char *cp;

	if (dblock.dbuf.typeflag != '5') {
		/*
	 	* Quick check for existence of directory.
	 	*/
		if ((cp = strrchr(name, '/')) == 0)
			return (0);
		*cp = '\0';

	} else if ((cp = strrchr(name, '/')) == 0) {
		if (access(name, 0) < 0) {
			if(mkdir(name, 0777) < 0) {
				vperror(0, name);
				return(0);
			}
			return(1);
		}
		return(0);
	}

	if (access(name, 0) == 0) {	/* already exists */
		*cp = '/';
		return (cp[1] == '\0');	/* return (lastchar == '/') */
	}


	/*
	 * No luck, try to make all directories in path.
	 */
	for (cp = name; *cp; cp++) {
		if (*cp != '/')
			continue;
		*cp = '\0';
		if (access(name, 0) < 0) {
			if (mkdir(name, 0777) < 0) {
				vperror(0, name);
				*cp = '/';
				return (0);
			}
		}
		*cp = '/';
	}
	if (access(name, 0) < 0) {
		if (mkdir(name, 0777) < 0) {
			vperror(0, name);
			return (0);
		}
		return(1);
	}

	return (cp[-1]=='/');
}

static
void
onintr()
{
	(void) signal(SIGINT, SIG_IGN);
	term++;
}

static
void
onquit()
{
	(void) signal(SIGQUIT, SIG_IGN);
	term++;
}

static
void
onhup()
{
	(void) signal(SIGHUP, SIG_IGN);
	term++;
}

/*	uncomment if you need it
static
void
onterm()
{
	signal(SIGTERM, SIG_IGN);
	term++;
}
*/

static	void
tomodes(sp)
register struct stat *sp;
{
	register char *cp;

	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		*cp = '\0';
	(void) sprintf(dblock.dbuf.mode, "%07o", sp->st_mode & MODEMASK1);
	(void) sprintf(dblock.dbuf.uid, "%07o", sp->st_uid);
	(void) sprintf(dblock.dbuf.gid, "%07o", sp->st_gid);
	(void) sprintf(dblock.dbuf.size, "%011lo", sp->st_size);
	(void) sprintf(dblock.dbuf.mtime, "%011lo", sp->st_mtime);
}

static	int
checksum()
{
	register i;
	register char *cp;

	for (cp = dblock.dbuf.chksum; cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
		*cp = ' ';
	i = 0;
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		i += *cp;
	return(i);
}

static	int
checkw(c, name)
char c;
char *name;
{
	if (wflag) {
		(void) printf("%c ", c);
		if (vflag)
			longt(&stbuf);
		(void) printf("%s: ", name);
		if (response() == 'y'){
			return(1);
		}
		return(0);
	}
	return(1);
}

static	int
response()
{
	register int c;

	c = getchar();
	if (c != '\n')
		while (getchar() != '\n');
	else c = 'n';
	return((c >= 'A' && c <= 'Z') ? c + ('a'-'A') : c);
}

static	int
checkupdate(arg)
char	*arg;
{
	char name[MAXNAM];	/* was 100; parameterized */
	long	mtime;
	daddr_t seekp;
	static daddr_t	lookup();

	rewind(tfile);
	/*for (;;) {*/
		if ((seekp = lookup(arg)) < 0)
			return(1);
		(void) fseek(tfile, seekp, 0);
		(void) fscanf(tfile, "%s %lo", name, &mtime);
		return(stbuf.st_mtime > mtime);
/* NOTREACHED */
	/*}*/
/* NOTREACHED */
}

/***	newvol	get new floppy (or tape) volume
 *
 *	newvol();		resets tapepos and first to TRUE, prompts for
 *				for new volume, and waits.
 *	if dumping, end-of-file is written onto the tape.
 */

static	void
newvol()
{
	register int c;
	extern char *sys_errlist[];
	extern int errno;

	if (dumping) {
		DEBUG("newvol called with 'dumping' set\n", 0, 0);
		closevol();
		sync();
		tapepos = 0;
	} else
		first = TRUE;
	(void) close(mt);
	if (sflag) {
		sumepi(&Si);
		sumout(Sumfp, &Si);
		(void) fprintf(Sumfp, "\n");

		sumpro(&Si);
	}
	(void) fprintf(stderr, "tar: \007please insert new volume, then press RETURN.");
	(void) fseek(stdin, 0L, 2);	/* scan over read-ahead */
	while ((c = getchar()) != '\n' && ! term)
		if (c == EOF)
			done(0);
	if (term)
		done(0);
#ifdef LISA
	sleep(3);		/* yecch */
#endif

errno = 0;

	mt = strcmp(usefile, "-") == EQUAL  ?  dup(1) : open(usefile, dumping ? update : 0);
	if (mt < 0) {
		(void) fprintf(stderr, "tar: cannot reopen %s (%s)\n", dumping ? "output" : "input", usefile);

(void) fprintf(stderr, "update=%d, usefile=%s, mt=%d, [%s]\n", 
	update, usefile, mt, sys_errlist[errno]);

		done(2);
	}
}

/*
 * Write a trailer portion to close out the current output volume.
 */

static	void
closevol()
{
	putempty(2L);	/* 2 EOT marks */
	if (mulvol && Sflag) {
		/*
		 * blocklim does not count the 2 EOT marks;
		 * tapepos  does     count the 2 EOT marks;
		 * therefore we need the +2 below.
		 */
		putempty(blocklim + 2L - tapepos);
	}
	flushtape();
}

static	void
done(n)
{
	(void) unlink(tname);
	exit(n);
}

static	int
prefix(s1, s2)
register char *s1, *s2;
{
	while (*s1)
		if (*s1++ != *s2++)
			return(0);
	if (*s2)
		return(*s2 == '/');
	return(1);
}

static	void
getwdir(s)
char *s;
{
	int i;
	int	pipdes[2];

	(void) pipe(pipdes);
	if ((i = fork()) == 0) {
		(void) close(1);
		(void) dup(pipdes[1]);
		(void) execl("/bin/pwd", "pwd", 0);
		(void) execl("/usr/bin/pwd", "pwd", 0);
		(void) fprintf(stderr, "tar: pwd failed!\n");
		(void) printf("/\n");
		exit(1);
	}
	if (i == -1) {
		(void) fprintf(stderr, "tar: No process to get directory name!\n");
		done(7);
	}
	while (wait((int *)NULL) != (pid_t)-1)
			;
	(void) read(pipdes[0], s, MAXPATHLEN);
	while(*s != '\n')
		s++;
	*s = '\0';
	(void) close(pipdes[0]);
	(void) close(pipdes[1]);
}

#define	N	200
static	int	njab;
static	daddr_t
lookup(s)
char *s;
{
	register i;
	daddr_t a;

	for(i=0; s[i]; i++)
		if(s[i] == ' ')
			break;
	a = bsrch(s, i, low, high);
	return(a);
}

static	daddr_t
bsrch(s, n, l, h)
daddr_t l, h;
char *s;
{
	register i, j;
	char b[N];
	daddr_t m, m1;

	njab = 0;

loop:
	if(l >= h)
		return(-1L);
	m = l + (h-l)/2 - N/2;
	if(m < l)
		m = l;
	(void) fseek(tfile, m, 0);
	(void) fread(b, 1, N, tfile);
	njab++;
	for(i=0; i<N; i++) {
		if(b[i] == '\n')
			break;
		m++;
	}
	if(m >= h)
		return(-1L);
	m1 = m;
	j = i;
	for(i++; i<N; i++) {
		m1++;
		if(b[i] == '\n')
			break;
	}
	i = cmp(b+j, s, n);
	if(i < 0) {
		h = m;
		goto loop;
	}
	if(i > 0) {
		l = m1;
		goto loop;
	}
	return(m);
}

static	int
cmp(b, s, n)
  register char *b, *s;
{
	register i;

	if(b[0] != '\n')
		exit(2);
	for(i=0; i<n; i++) {
		if(b[i+1] > s[i])
			return(-1);
		if(b[i+1] < s[i])
			return(1);
	}
	return(b[i+1] == ' '? 0 : -1);
}

/***	seekdisk	seek to next file on archive
 *
 *	called by passtape() only
 *
 *	WARNING: expects "nblock" to be set, that is, readtape() to have
 *		already been called.  Since passtape() is only called
 *		after a file header block has been read (why else would
 *		we skip to next file?), this is currently safe.
 *
 *	changed to guarantee SYS_BLOCK boundary
 */
static	void
seekdisk(blocks)
long blocks;
{
	long seekval;
#if SYS_BLOCK > TBLOCK
	/* handle non-multiple of SYS_BLOCK */
	register nxb;	/* # extra blocks */
#endif

	tapepos += blocks;
	DEBUG("seekdisk(%lu) called\n", blocks, 0);
	if (recno + blocks <= nblock) {
		recno += blocks;
		return;
	}
	if (recno > nblock)
		recno = nblock;
	seekval = blocks - (nblock - recno);
	recno = nblock;	/* so readtape() reads next time through */
#if SYS_BLOCK > TBLOCK
	nxb = (int) (seekval % (long)(SYS_BLOCK / TBLOCK));
	DEBUG("xtrablks=%d seekval=%lu blks\n", nxb, seekval);
	if (nxb && nxb > seekval) /* don't seek--we'll read */
		goto noseek;
	seekval -= (long) nxb;	/* don't seek quite so far */
#endif
	if (lseek(mt, (long) TBLOCK * seekval, 1) == -1L) {
		(void) fprintf(stderr, "tar: device seek error\n");
		done(3);
	}
#if SYS_BLOCK > TBLOCK
	/* read those extra blocks */
noseek:
	if (nxb) {
		DEBUG("reading extra blocks\n",0,0);
		if (read(mt, tbuf, TBLOCK*nblock) < 0) {
			(void) fprintf(stderr, "tar: read error while skipping file\n");
			done(8);
		}
		recno = nxb;	/* so we don't read in next readtape() */
	}
#endif
}

static	void
readtape(buffer)
char *buffer;
{
	register int i, j;

	++tapepos;
	if (recno >= nblock || first) {
		if (first) {
			/*
			 * set the number of blocks to
			 * read initially.
			 * very confusing!
			 */
			if (bflag)
				j = nblock;
			else if (!NotTape)
				j = NBLOCK;
			else if (defaults_used)
				j = nblock;
			else
				j = NBLOCK;
		} else
			j = nblock;
		if ((i = read(mt, tbuf, TBLOCK*j)) < 0) {
			(void) fprintf(stderr, "tar: tape read error\n");
			done(3);
		}
		if (first) {
			if ((i % TBLOCK) != 0) {
				(void) fprintf(stderr, "tar: tape blocksize error\n");
				done(3);
			}
			i /= TBLOCK;
			if (rflag && i != 1 && !NotTape) {
				(void) fprintf(stderr, "tar: Cannot update blocked tapes\n");
				done(4);
			}
			if (vflag && i != nblock && i != 1) {
				if (NotTape)
					(void) fprintf(stderr, "tar: buffer size = %dK\n", K(i));
				else
					(void) fprintf(stderr, "tar: blocksize = %d\n", i);
			}
			nblock = i;
		}
		recno = 0;
	}

	first = FALSE;
	copy(buffer, &tbuf[recno++]);
	if (sflag)
		sumupd(&Si, buffer, TBLOCK);
	return;
}

static	void
writetape(buffer)
char *buffer;
{
	tapepos++;	/* output block count */

	first = FALSE;	/* removed setting of nblock if file arg given */
	if (recno >= nblock)
		flushtape();
	copy(&tbuf[recno++], buffer);
	if (recno >= nblock)
		flushtape();
}



/***    backtape - reposition tape after reading soft "EOF" record
 *
 *      Backtape tries to reposition the tape back over the EOF
 *      record.  This is for the -u and -r options so that the
 *      tape can be extended.  This code is not well designed, but
 *      I'm confident that the only callers who care about the
 *      backspace-over-EOF feature are those involved in -u and -r.
 *      Thus, we don't handle it correctly when there is
 *      a block factor, but the -u and -r options refuse to work
 *      on block tapes, anyway.
 *
 *      Note that except for -u and -r, backtape is called as a
 *      (apparently) unwanted side effect of endtape().  Thus,
 *      we don't bitch when the seeks fail on raw devices because
 *      when not using -u and -r tar can be used on raw devices.
 */

static	void
backtape()
{
	DEBUG("backtape() called, recno=%d nblock=%d\n", recno, nblock);

	/*
	 * The first seek positions the tape to before the eof;
	 * this currently fails on raw devices.
	 * Thus, we ignore the return value from lseek().
	 * The second seek does the same.
	 */
	(void) lseek(mt, (long) -(TBLOCK*nblock), 1);	/* back one large tape block */
	recno--;				/* reposition over soft eof */
	tapepos--;				/* back up tape position */
	if (read(mt, tbuf, TBLOCK*nblock) <= 0) {
		(void) fprintf(stderr, "tar: tape read error after seek\n");
		done(4);
	}
	(void) lseek(mt, (long) -(TBLOCK*nblock), 1);	/* back large block again */
}


/***    flushtape  write buffered block(s) onto tape
 *
 *      recno points to next free block in tbuf.  If nonzero, a write is done.
 *      Care is taken to write in multiples of SYS_BLOCK when device is
 *      non-magtape in case raw i/o is used.
 *
 *      NOTE:   this is called by writetape() to do the actual writing
 */
static	void
flushtape()
{

	DEBUG("flushtape() called, recno=%d\n", recno, 0);
	if (recno > 0) {	/* anything buffered? */
		if (NotTape) {
#if SYS_BLOCK > TBLOCK
			register i;

			/* an odd-block write can only happen when
			 * we are at the end of a volume that is not a tape.
			 * Here we round recno up to an even SYS_BLOCK
			 * boundary.
			 */
			if ((i = recno % (SYS_BLOCK / TBLOCK)) != 0) {
				DEBUG("flushtape() %d rounding blocks\n", i, 0);
				recno += i;	/* round up to even SYS_BLOCK */
			}
#endif
			if (recno > nblock)
				recno = nblock;
		}
		DEBUG("writing out %d blocks of %d bytes\n",
			(NotTape ? recno : nblock),
			(NotTape ? recno : nblock) * TBLOCK);
	
		if (write(mt, tbuf, (NotTape ? recno : nblock) * TBLOCK) < 0) {
			(void) fprintf(stderr, "tar: tape write error\n");
			done(2);
		}
		if (sflag)
			sumupd(&Si, tbuf, (NotTape ? recno : nblock) * TBLOCK);
		recno = 0;
	}
}

static	void
copy(to, from)
register char *to, *from;
{
	register i;

	i = TBLOCK;
	do {
		*to++ = *from++;
	} while (--i);
}

/***	initarg -- initialize things for nextarg.
 *
 *	argv		filename list, a la argv.
 *	filefile	name of file containing filenames.  Unless doing
 *		a create, seeks must be allowable (e.g. no named pipes).
 *
 *	- if filefile is non-NULL, it will be used first, and argv will
 *	be used when the data in filefile are exhausted.
 *	- otherwise argv will be used.
 */
static char **Cmdargv = NULL;
static FILE *FILEFile = NULL;
static long seekFile = -1;
static char *ptrtoFile, *begofFile, *endofFile; 

static	void
initarg(argv, filefile)
char *argv[];
char *filefile;
{
	struct stat statbuf;
	register char *p;
	int nbytes;

	Cmdargv = argv;
	if (filefile == NULL)
		return;		/* no -F file */
	if (FILEFile != NULL) {
		/*
		 * need to REinitialize
		 */
		if (seekFile != -1)
			(void) fseek(FILEFile, seekFile, 0);
		ptrtoFile = begofFile;
		return;
	}
	/*
	 * first time initialization 
	 */
	if ((FILEFile = fopen(filefile, "r")) == NULL) {
		(void) fprintf(stderr, "tar: cannot open (%s)\n", filefile);
		done(1);
	}
	(void) fstat(fileno(FILEFile), &statbuf);
	if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
		(void) fprintf(stderr,
			"tar: %s is not a regular file\n", filefile);
		(void) fclose(FILEFile);
		done(1);
	}
	ptrtoFile = begofFile = endofFile;
	seekFile = 0;
	if (!xflag)
		return;		/* the file will be read only once anyway */
	nbytes = statbuf.st_size;
	while ((begofFile = malloc(nbytes)) == NULL)
		nbytes -= 20;
	if (nbytes < 50) {
		free(begofFile);
		begofFile = endofFile;
		return;		/* no room so just do plain reads */
	}
	if (fread(begofFile, 1, nbytes, FILEFile) != nbytes) {
		(void) fprintf(stderr, "tar: could not read %s\n", filefile);
		done(1);
	}
	ptrtoFile = begofFile;
	endofFile = begofFile + nbytes;
	for (p = begofFile; p < endofFile; ++p)
		if (*p == '\n')
			*p = '\0';
	if (nbytes != statbuf.st_size)
		seekFile = nbytes + 1;
	else
		(void) fclose(FILEFile);
}

/***	nextarg -- get next argument of arglist.
 *
 *	The argument is taken from wherever is appropriate.
 *
 *	If the 'F file' option has been specified, the argument will be
 *	taken from the file, unless EOF has been reached.
 *	Otherwise the argument will be taken from argv.
 *
 *	WARNING:
 *	  Return value may point to static data, whose contents are over-
 *	  written on each call.
 */
static	char  *
nextarg()
{
	static char nameFile[LPNMAX];
	int n;
	char *p;

	if (FILEFile) {
		if (ptrtoFile < endofFile) {
			p = ptrtoFile;
			while (*ptrtoFile)
				++ptrtoFile;
			++ptrtoFile;
			return(p);
		}
		if (fgets(nameFile, LPNMAX, FILEFile) != NULL) {
			n = strlen(nameFile);
			if (n > 0 && nameFile[n-1] == '\n')
				nameFile[n-1] = '\0';
			return(nameFile);
		}
	}
	return(*Cmdargv++);
}

/*
 * kcheck()
 *	- checks the validity of size values for non-tape devices
 *	- if size is zero, mulvol tar is disabled and size is
 *	  assumed to be infinite.
 *	- returns volume size in TBLOCKS
 */
static	long
kcheck(kstr)
char	*kstr;
{
	long kval;

	kval = atol(kstr);
	if (kval == 0L) {	/* no multi-volume; size is infinity.  */
		mulvol = 0;	/* definitely not mulvol, but we must  */
		return(0);	/* took out setting of NotTape */
	}
	if (kval < (long) MINSIZE) {
		(void) fprintf(stderr, "tar: sizes below %luK not supported (%lu).\n",
				(long) MINSIZE, kval);
		if (!kflag)
			(void) fprintf(stderr, "bad size entry for %s in %s.\n",
				archive, DEF_FILE);
		done(1);
	}
	mulvol++;
	NotTape++;			/* implies non-tape */
	return(kval * 1024L / TBLOCK);	/* convert to TBLOCKS */
}

/*
 * bcheck()
 *	- checks the validity of blocking factors
 *	- returns blocking factor
 */
static	int
bcheck(bstr)
char	*bstr;
{
	int bval;

	bval = atoi(bstr);
	if (bval > NBLOCK || bval <= 0) {
		(void) fprintf(stderr, "tar: invalid blocksize. (Max %d)\n", NBLOCK);
		if (!bflag)
			(void) fprintf(stderr, "bad blocksize entry for '%s' in %s.\n",
				archive, DEF_FILE);
		done(1);
	}
	return(bval);
}

/*
 * defset()
 *	- reads DEF_FILE for the set of default values specified.
 *	- initializes 'usefile', 'nblock', and 'blocklim', and 'NotTape'.
 *	- 'usefile' points to static data, so will be overwritten
 *	  if this routine is called a second time.
 *	- the pattern specified by 'arch' must be followed by four
 *	  blank-separated fields (1) device (2) blocking,
 *    	                         (3) size(K), and (4) tape
 *	  for example: archive0=/dev/fd 1 400 n
 *	- the define's below are used in defcntl() to ignore case.
 */
#define	DC_GETFLAGS	0
#define	DC_SETFLAGS	1
#define DC_CASE		0001
#define DC_STD		((0) | DC_CASE)
static	int
defset(arch)
char	*arch;
{
	extern int defcntl(), defopen();
	extern char *defread();
	char *bp;

	if (defopen(DEF_FILE) != 0)
		return(FALSE);
	if (defcntl(DC_SETFLAGS, (DC_STD & ~(DC_CASE))) == -1) {
		(void) fprintf(stderr, "tar: error setting parameters for %s.\n",
				DEF_FILE);
		return(FALSE);			/* & following ones too */
	}
	if ((bp = defread(arch)) == NULL) {
		(void) fprintf(stderr, "tar: missing or invalid '%s' entry in %s.\n",
				arch, DEF_FILE);
		return(FALSE);
	}
	if ((usefile = strtok(bp, " \t")) == NULL) {
		(void) fprintf(stderr, "tar: '%s' entry in %s is empty!\n",
				arch, DEF_FILE);
		return(FALSE);
	}
	if ((bp = strtok(NULL, " \t")) == NULL) {
		(void) fprintf(stderr, 
			"tar: block component missing in '%s' entry in %s.\n",
		       	arch, DEF_FILE);
		return(FALSE);
	}
	nblock = bcheck(bp);
	if ((bp = strtok(NULL, " \t")) == NULL) {
		(void) fprintf(stderr,
			"tar: size component missing in '%s' entry in %s.\n",
		       	arch, DEF_FILE);
		return(FALSE);
	}
	blocklim = kcheck(bp);
	if ((bp = strtok(NULL, " \t")) != NULL)
		NotTape = (*bp == 'n' || *bp == 'N');
	else 
		NotTape = (blocklim > 0);
	defopen(NULL);
	DEBUG("defset: archive='%s'; usefile='%s'\n", arch, usefile);
	DEBUG("defset: nblock='%d'; blocklim='%ld'\n",
	      nblock, blocklim);
	DEBUG("defset: not tape = %d\n", NotTape, 0);
	return(TRUE);
}

/*
 * Following code handles excluded and included files.
 * A hash table of file names to be {in,ex}cluded is built.
 * For excluded files, before writing or extracting a file
 * check to see if it is in the exluce_tbl.
 * For included files, the wantit() procedure will check to
 * see if the named file is in the include_tbl.
 */
static	void
build_table(table, file)
struct file_list *table[];
char	*file;
{
	FILE	*fp;
	char	buf[512];

	if ((fp = fopen(file, "r")) == (FILE *)NULL)
		vperror(1, "could not open %s", file);
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		buf[strlen(buf) - 1] = '\0';
		add_file_to_table(table, buf);
	}
	(void) fclose(fp);
}

/*
 * Add a file name to the the specified table, if the file name has any
 * trailing '/'s then delete them before inserting into the table
 */
static	void
add_file_to_table(table, str)
struct file_list *table[];
char	*str;
{
	char	name[MAXNAM];
	unsigned int h;
	struct	file_list	*exp;

	(void) strcpy(name, str);
	while (name[strlen(name) - 1] == '/') {
		name[strlen(name) - 1] = NULL;
	}

	h = hash(name);
	exp = (struct file_list *) malloc(sizeof(struct file_list));
	exp->name = strcpy(malloc(strlen(name)+1), name);
	exp->next = table[h];
	table[h] = exp;
}

/*
 * See if a file name or any of the file's parent directories is in the
 * specified table, if the file name has any trailing '/'s then delete
 * them before searching the table
 */
static	int
is_in_table(table, str)
struct file_list *table[];
char	*str;
{
	char	name[MAXNAM];
	unsigned int	h;
	struct	file_list	*exp;
	char	*ptr;

	(void) strcpy(name, str);
	while (name[strlen(name) - 1] == '/') {
		name[strlen(name) - 1] = NULL;
	}

	/*
	 * check for the file name in the passed list
	 */
	h = hash(name);
	exp = table[h];
	while (exp != NULL) {
		if (strcmp(name, exp->name) == 0) {
			return(1);
		}
		exp = exp->next;
	}

	/*
	 * check for any parent directories in the file list
	 */
	while (ptr = strchr(name, '/')) {
		*ptr = NULL;
		h = hash(name);
		exp = table[h];
		while (exp != NULL) {
			if (strcmp(name, exp->name) == 0) {
				return(1);
			}
			exp = exp->next;
		}
	}

	return(0);
}

/*
 * Compute a hash from a string.
 */
static	unsigned int
hash(str)
char	*str;
{
	char	*cp;
	unsigned int	h;

	h = 0;
	for (cp = str; *cp; cp++) {
		h += *cp;
	}
	return(h % TABLE_SIZE);
}

static	char *
getmem(size)
{
	char *p = malloc((unsigned) size);

	if (p == NULL && freemem) {
		(void) fprintf(stderr,
		    "tar: out of memory, link and directory modtime info lost\n");
		freemem = 0;
		if (eflag)
			done(1);
	}
	return (p);
}

/* vperror() --variable argument perror.
 * Takes 3 args: exit_status, formats, args.  If exit status is 0, then
 * the eflag (exit on error) is checked -- if it is non-zero, tar exits
 * with the value of whatever "errno" is set to.  If exit_status is not
 * zero, then it exits with that error status. If eflag and exit_status
 * are both zero, the routine returns to where it was called.
 */
static	void
vperror(exit_status, fmt, args)
	register char *fmt;
{
	extern char *sys_errlist[];
	extern int errno;

	(void) fputs("tar: ", stderr);
	(void) vfprintf(stderr, fmt, &args);
	(void) fprintf(stderr, ": %s\n", sys_errlist[errno]);
	if (exit_status)
		done(exit_status);
	else if (eflag)
		done(errno);
}

/*VARARGS*/
static	void
fatal(s1, s2, s3, s4, s5)
char *s1, *s2, *s3, *s4, *s5;
{
	(void) fprintf(stderr, "tar: ");
	(void) fprintf(stderr, s1, s2, s3, s4, s5);
	(void) fprintf(stderr, "\n");
	done(1);
}
