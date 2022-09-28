/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ufs.cmds:ufs/ufsdump/dumpmain.c	1.7.3.1"
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

#include "dump.h"

int	notify = 0;	/* notify operator flag */
int	blockswritten = 0;	/* number of blocks written on current tape */
int	tapeno = 0;	/* current tape number */
int	density = 0;	/* density in bytes/0.1" */
int	tenthsperirg;	/* inter-record-gap in 0.1"'s */
int	ntrec = 0;	/* # tape blocks in each tape record */
int	cartridge = 0;	/* Assume non-cartridge tape */
int	tracks;		/* # tracks on a cartridge tape */
char	*host;
char	*mb();
int	anydskipped;	/* set true in mark() if any directories are skipped */
			/* this lets us avoid map pass 2 in some cases */

main(argc, argv)
	int	argc;
	char	*argv[];
{
	char		*arg;
	int		bflag = 0, i;
	float		fetapes;
	register	struct	vfstab	*dt;

	/*
	 * Put out an audit message to reflect the parameters passed
	 */
	time(&(spcl.c_date));

	tsize = 0;	/* Default later, based on 'c' option for cart tapes */
	if (arg = (char *)rindex(argv[0], '/'))
		arg++;
	else
		arg = argv[0];
	if (*arg == 'r') {
		fprintf (stderr, "rdump: remote dumping is not supported\n");
		exit (32);
	} else
		tape = TAPE;
	disk = NULL;
	increm = NINCREM;
	temp = TEMP;
	if (TP_BSIZE / DEV_BSIZE == 0 || TP_BSIZE % DEV_BSIZE != 0) {
		msg("TP_BSIZE must be a multiple of DEV_BSIZE\n");
		dumpabort();
	}
	incno = '9';
	uflag = 0;
	arg = "u";
	if (argc > 1) {
		argv++;
		argc--;
		arg = *argv;
		if (*arg == '-')
			arg++;
	}
	while(*arg)
	switch (*arg++) {
	case 'w':
		lastdump('w');		/* tell us only what has to be done */
		exit(0);
		break;
	case 'W':			/* what to do */
		lastdump('W');		/* tell us the current state of what has been done */
		exit(0);		/* do nothing else */
		break;

	case 'f':			/* output file */
		if (argc > 1) {
			argv++;
			argc--;
			tape = *argv;
		}
		break;

	case 'd':			/* density, in bits per inch */
		if (argc > 1) {
			argv++;
			argc--;
			density = atoi(*argv) / 10;
		}
		break;

	case 's':			/* tape size, feet */
		if (argc > 1) {
			argv++;
			argc--;
			tsize = atol(*argv);
			tsize *= 12L*10L;
		}
		break;

	case 't':			/* tracks */
		if (argc > 1) {
			argv++;
			argc--;
			tracks = atol(*argv);
		}
		break;

	case 'b':			/* blocks per tape write */
		if (argc > 1) {
			argv++;
			argc--;
			bflag++;
			ntrec = atol(*argv);
			if (ntrec <= 0 || (ntrec&1)) {
				msg("Block size must be a positive, even integer\n");
				dumpabort();
			}
			ntrec /= 2;
		}
		break;

	case 'c':			/* Tape is cart. not 9-track */
		cartridge++;
		break;

	case '0':			/* dump level */
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		incno = arg[-1];
		break;

	case 'u':			/* update /etc/dumpdates */
		uflag++;
		break;

	case 'n':			/* notify operators */
		notify++;
		break;

	default:
		fprintf(stderr, "dump: bad key '%c%'\n", arg[-1]);
		Exit(X_ABORT);
	}
	if (argc > 1) {
		argv++;
		argc--;
		disk = *argv;
	}
	if (strcmp(tape, "-") == 0) {
		pipeout++;
		tape = "standard output";
	}
	if (disk == NULL) {
		fprintf(stderr,
		  "ufs usage: ufsdump [0123456789fustdWwncb [argument]] filesystem\n");
		Exit(31+X_ABORT);
	}

	/*
	 * Determine how to default tape size and density
	 *
	 *         	density				tape size
	 * 9-track	1600 bpi (160 bytes/.1")	2300 ft.
	 * 9-track	6250 bpi (625 bytes/.1")	2300 ft.
	 *
	 * Most Sun-2's came with 4 track (20MB) cartridge tape drives,
	 * while most other machines (Sun-3's and non-Sun's) come with
	 * 9 track (45MB) cartridge tape drives.  Some Sun-2's came with
	 * 9 track drives, but there is no way for the software to detect
	 * which drive type is installed.  Sigh...  We make the gross
	 * assumption that #ifdef mc68010 will test for a Sun-2.
	 *
 	 * cartridge	8000 bpi (100 bytes/.1")	425 * tracks ft.
	 */
	if (density == 0)
		density = cartridge ? 100 : 160;
	if (tracks == 0)
		tracks = 9;
	if (tsize == 0)
		tsize = cartridge ? 425L*120L : 2300L*120L;
	if (cartridge)
		tsize *= tracks;
	if (!bflag) {
		if (cartridge)
			ntrec = CARTRIDGETREC;
		else if (density >= 625)
			ntrec = HIGHDENSITYTREC;
		else
			ntrec = NTREC;
	}

	if (signal(SIGHUP, sighup) == SIG_IGN)
		signal(SIGHUP, SIG_IGN);
	if (signal(SIGTRAP, sigtrap) == SIG_IGN)
		signal(SIGTRAP, SIG_IGN);
	if (signal(SIGFPE, sigfpe) == SIG_IGN)
		signal(SIGFPE, SIG_IGN);
	if (signal(SIGBUS, sigbus) == SIG_IGN)
		signal(SIGBUS, SIG_IGN);
	if (signal(SIGSEGV, sigsegv) == SIG_IGN)
		signal(SIGSEGV, SIG_IGN);
	if (signal(SIGTERM, sigterm) == SIG_IGN)
		signal(SIGTERM, SIG_IGN);


	if (signal(SIGINT, ufsinterrupt) == SIG_IGN)
		signal(SIGINT, SIG_IGN);

	set_operators();	/* /etc/group snarfed */
	getfstab();		/* /etc/vfstab snarfed */
	/*
	 *	disk can be either the full special file name,
	 *	the suffix of the special file name,
	 *	the special name missing the leading '/',
	 *	the file system name with or without the leading '/'.
	 */
	dt = (struct vfstab *)fstabsearch(disk);
	if (dt != 0)
		disk = rawname(dt->vfs_special);
	getitime();		/* /etc/dumpdates snarfed */

	msg("Date of this level %c dump: %s\n", incno, prdate(spcl.c_date));
 	msg("Date of last level %c dump: %s\n",
		lastincno, prdate(spcl.c_ddate));
	msg("Dumping %s ", disk);
	if (dt != 0)
		msgtail("(%s) ", dt->vfs_mountp);
	msgtail("to %s", tape);
	if (host)
		msgtail(" on host %s", host);
	msgtail("\n");

	fi = open(disk, 0);
	if (fi < 0) {
		msg("Cannot open %s\n", disk);
		Exit(X_ABORT);
	}
	esize = 0;
	sblock = (struct fs *)buf;
	sync();
	bread(SBLOCK, sblock, SBSIZE);
	if (sblock->fs_magic != FS_MAGIC) {
		msg("bad sblock magic number\n");
		dumpabort();
	}
	msiz = roundup(howmany(sblock->fs_ipg * sblock->fs_ncg, NBBY),
		TP_BSIZE);
	clrmap = (char *)calloc(msiz, sizeof(char));
	dirmap = (char *)calloc(msiz, sizeof(char));
	nodmap = (char *)calloc(msiz, sizeof(char));

	anydskipped = 0;
	msg("mapping (Pass I) [regular files]\n");
	pass(mark, (char *)NULL);		/* mark updates esize */

	if (anydskipped) {
		do {
			msg("mapping (Pass II) [directories]\n");
			nadded = 0;
			pass(add, dirmap);
		} while(nadded);
	} else				/* keep the operators happy */
		msg("mapping (Pass II) [directories]\n");

	bmapest(clrmap);
	bmapest(nodmap);

	if (cartridge) {
		/*
		 * Estimate number of tapes, assuming streaming stops at
		 * the end of each block written, and not in mid-block.
		 * Assume no erroneous blocks; this can be compensated for
		 * with an artificially low tape size.
		 */
		tenthsperirg = 16;	/* actually 15.48, says Archive */
		fetapes = 
		(	  esize		/* blocks */
			* TP_BSIZE	/* bytes/block */
			* (1.0/density)	/* 0.1" / byte */
		  +
			  esize		/* blocks */
			* (1.0/ntrec)	/* streaming-stops per block */
			* tenthsperirg	/* 0.1" / streaming-stop */
		) * (1.0 / tsize );	/* tape / 0.1" */
	} else {
		/* Estimate number of tapes, for old fashioned 9-track tape */
#ifdef sun
		/* sun has long irg's */
		tenthsperirg = (density == 625) ? 6 : 12;
#else
		tenthsperirg = (density == 625) ? 5 : 8;
#endif
		fetapes =
		(	  esize		/* blocks */
			* TP_BSIZE	/* bytes / block */
			* (1.0/density)	/* 0.1" / byte */
		  +
			  esize		/* blocks */
			* (1.0/ntrec)	/* IRG's / block */
			* tenthsperirg	/* 0.1" / IRG */
		) * (1.0 / tsize );	/* tape / 0.1" */
	}
	etapes = fetapes;		/* truncating assignment */
	etapes++;
	/* count the nodemap on each additional tape */
	for (i = 1; i < etapes; i++)
		bmapest(nodmap);
	esize += i + ntrec;	/* headers + ntrec trailer blocks */
	msg("estimated %ld blocks (%s) on %3.2f tape(s).\n",
	    esize*2, mb(esize), fetapes);

	alloctape();			/* Allocate tape buffer */

	otape();			/* bitmap is the first to tape write */
	time(&(tstart_writing));
	bitmap(clrmap, TS_CLRI);

	msg("dumping (Pass III) [directories]\n");
 	pass(dirdump, dirmap);

	msg("dumping (Pass IV) [regular files]\n");
	pass(dump, nodmap);

	msg("UFSDUMP: %ld blocks (%s) on %d tape%s\n",
	    spcl.c_tapea*2, mb(spcl.c_tapea), spcl.c_volume,
	    spcl.c_volume > 1 ? "s" : "");
	msg("UFSDUMP IS DONE\n");

	putitime();
	ufsrewind();
	broadcast("UFSDUMP IS DONE!\7\7\n");
	Exit(X_FINOK);
}

void	sighup(){	msg("SIGHUP()  try rewriting\n"); sigAbort();}
void	sigtrap(){	msg("SIGTRAP()  try rewriting\n"); sigAbort();}
void	sigfpe(){	msg("SIGFPE()  try rewriting\n"); sigAbort();}
void	sigbus(){	msg("SIGBUS()  try rewriting\n"); sigAbort();}
void	sigsegv(){	msg("SIGSEGV()  ABORTING!\n"); abort();}
void	sigalrm(){	msg("SIGALRM()  try rewriting\n"); sigAbort();}
void	sigterm(){	msg("SIGTERM()  try rewriting\n"); sigAbort();}

sigAbort()
{
	if (pipeout) {
		msg("Unknown signal, cannot recover\n");
		dumpabort();
	}
	msg("Rewriting attempted as response to unknown signal.\n");
	fflush(stderr);
	fflush(stdout);
	close_rewind();
	exit(31+X_REWRITE);
}

char *
rawname(cp)
	char *cp;
{
	static char rawbuf[32];
	char *dp = (char *)rindex(cp, '/');
	struct stat statb;

	if (dp == 0)
		return (0);

	/* for device naming convention /dev/dsk/c1d0s2 */
	sprintf(rawbuf, "/dev/rdsk/%s", dp+1);
	if (stat(rawbuf, &statb) == 0)
		return(rawbuf);

	/* for device naming convention /dev/save */
	*dp = 0;
	strcpy(rawbuf, cp);
	*dp = '/';
	strcat(rawbuf, "/r");
	strcat(rawbuf, dp+1);
	return (rawbuf);
}

static char *
mb(blks)
	long blks;
{
	static char buf[16];

	if (blks < 1024)
		sprintf(buf, "%ldKB", blks);
	else
		sprintf(buf, "%.2fMB", ((double)blks) / 1024.);
	return (buf);
}
