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

#ident	"@(#)ufs.cmds:ufs/ufsrestore/main.c	1.4.3.1"

/*
 *	Modified to recursively extract all files within a subtree
 *	(supressed by the h option) and recreate the heirarchical
 *	structure of that subtree and move extracted files to their
 *	proper homes (supressed by the m option).
 *	Includes the s (skip files) option for use with multiple
 *	dumps on a single tape.
 *	8/29/80		by Mike Litzkow
 *
 *	Modified to work on the new file system and to recover from
 *	tape read errors.
 *	1/19/82		by Kirk McKusick
 *
 *	Full incremental restore running entirely in user code and
 *	interactive tape browser.
 *	1/19/83		by Kirk McKusick
 */

#include "restore.h"
#include "dumprestore.h"
#include <signal.h>

int	bflag = 0, cvtflag = 0, dflag = 0, vflag = 0, yflag = 0;
int	hflag = 1, mflag = 1;
char	command = '\0';
long	dumpnum = 1;
long	volno = 0;
long	ntrec;
char	*dumpmap;
char	*clrimap;
ino_t	maxino;
time_t	dumptime;
time_t	dumpdate;
FILE 	*terminal;

main(argc, argv)
	int argc;
	char *argv[];
{
	register char *cp;
	ino_t ino;
	char *inputdev = "/dev/rmt8";
	char *symtbl = "./restoresymtable";
	char name[MAXPATHLEN];
	extern void onintr();

	if (signal(SIGINT, onintr) == SIG_IGN)
		(void) signal(SIGINT, SIG_IGN);
	if (signal(SIGTERM, onintr) == SIG_IGN)
		(void) signal(SIGTERM, SIG_IGN);
	if (argc < 2) {
usage:
		fprintf(stderr, "ufs usage:\n%s%s%s%s%s",
			"\tufsrestore tfhsvy [file file ...]\n",
			"\tufsrestore xfhmsvy [file file ...]\n",
			"\tufsrestore ifhmsvy\n",
			"\tufsrestore rfsvy\n",
			"\tufsrestore Rfsvy\n");
		done(1);
	}
	argv++;
	argc -= 2;
	command = '\0';
	for (cp = *argv++; *cp; cp++) {
		switch (*cp) {
		case '-':
			break;
		case 'c':
			cvtflag++;
			break;
		case 'd':
			dflag++;
			break;
		case 'h':
			hflag = 0;
			break;
		case 'm':
			mflag = 0;
			break;
		case 'v':
			vflag++;
			break;
		case 'y':
			yflag++;
			break;
		case 'f':
			if (argc < 1) {
				fprintf(stderr, "missing device specifier\n");
				done(1);
			}
			inputdev = *argv++;
			argc--;
			break;
		case 'b':
			/*
			 * change default tape blocksize
			 */
			bflag++;
			if (argc < 1) {
				fprintf(stderr, "missing block size\n");
				done(1);
			}
			ntrec = atoi(*argv++);
			if (ntrec <= 0) {
				fprintf(stderr, "Block size must be a positive integer\n");
				done(1);
			}
			if (ntrec <= 0 || (ntrec&1)) {
				fprintf(stderr, "Block size must be a positive, even integer\n");
				done(1);
			}
			ntrec /= 2;
			argc--;
			break;
		case 's':
			/*
			 * dumpnum (skip to) for multifile dump tapes
			 */
			if (argc < 1) {
				fprintf(stderr, "missing dump number\n");
				done(1);
			}
			dumpnum = atoi(*argv++);
			if (dumpnum <= 0) {
				fprintf(stderr, "Dump number must be a positive integer\n");
				done(1);
			}
			argc--;
			break;
		case 't':
		case 'R':
		case 'r':
		case 'x':
		case 'i':
			if (command != '\0') {
				fprintf(stderr,
					"%c and %c are mutually exclusive\n",
					*cp, command);
				goto usage;
			}
			command = *cp;
			break;
		default:
			fprintf(stderr, "Bad key character %c\n", *cp);
			goto usage;
		}
	}
	if (command == '\0') {
		fprintf(stderr, "must specify i, t, r, R, or x\n");
		goto usage;
	}
	setinput(inputdev);
	if (argc == 0) {
		argc = 1;
		*--argv = ".";
	}
	switch (command) {
	/*
	 * Interactive mode.
	 */
	case 'i':
		setup();
		extractdirs(1);
		initsymtable((char *)0);
		runcmdshell();
		done(0);
	/*
	 * Incremental restoration of a file system.
	 */
	case 'r':
		setup();
		if (dumptime > 0) {
			/*
			 * This is an incremental dump tape.
			 */
			vprintf(stdout, "Begin incremental restore\n");
			initsymtable(symtbl);
			extractdirs(1);
			removeoldleaves();
			vprintf(stdout, "Calculate node updates.\n");
			treescan(".", UFSROOTINO, nodeupdates);
			findunreflinks();
			removeoldnodes();
		} else {
			/*
			 * This is a level zero dump tape.
			 */
			vprintf(stdout, "Begin level 0 restore\n");
			initsymtable((char *)0);
			extractdirs(1);
			vprintf(stdout, "Calculate extraction list.\n");
			treescan(".", UFSROOTINO, nodeupdates);
		}
		createleaves(symtbl);
		createlinks();
		setdirmodes();
		checkrestore();
		if (dflag) {
			vprintf(stdout, "Verify the directory structure\n");
			treescan(".", UFSROOTINO, verifyfile);
		}
		dumpsymtable(symtbl, (long)1);
		done(0);
	/*
	 * Resume an incremental file system restoration.
	 */
	case 'R':
		setupR();
		initsymtable(symtbl);
		skipmaps();
		skipdirs();
		createleaves(symtbl);
		createlinks();
		setdirmodes();
		checkrestore();
		dumpsymtable(symtbl, (long)1);
		done(0);
	/*
	 * List contents of tape.
	 */
	case 't':
		setup();
		extractdirs(0);
		initsymtable((char *)0);
		while (argc--) {
			canon(*argv++, name);
			ino = dirlookup(name);
			if (ino == 0)
				continue;
			treescan(name, ino, listfile);
		}
		done(0);
	/*
	 * Batch extraction of tape contents.
	 */
	case 'x':
		setup();
		extractdirs(1);
		initsymtable((char *)0);
		while (argc--) {
			canon(*argv++, name);
			ino = dirlookup(name);
			if (ino == 0)
				continue;
			if (mflag)
				pathcheck(name);
			treescan(name, ino, addfile);
		}
		createfiles();
		createlinks();
		setdirmodes();
		if (dflag)
			checkrestore();
		done(0);
	}
}
