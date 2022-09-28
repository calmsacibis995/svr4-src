/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)xinstall:fixperm.c	1.1"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

/*
 *	@(#) fixperm.c 1.15 86/12/05 
 *
 *	Copyright (C) The Santa Cruz Operation, 1985.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 */

/***	fixperm - 
 *
 *	MODIFICATION HISTORY
 *	M001	May  8, 1984	vwh
 *	- link() failure no longer fatal.
 *	- added error message when regular file missing and -c used.
 *	M002	Jul  23, 1984	vwh
 *	- test XE_EXEC bit, complain if not executable.
 *	M003	Sep 19, 1984	ericc
 *	- named pipes (file mode 'p...') are now permitted.
 *	- modified error message in check().
 *	M004	Oct  12, 1984	vwh
 *	- added Dflag to print directory list.
 *	M005	Jun  27, 1985	ncm
 *	- Add previous sco changes (vflag, disallow absolute paths,
 *	  allow perms file to be standard input.
 *	- Package specifiers may now be arbitrary length strings.
 *	- All pathnames are followed by an optional volume identifier.
 *	- wflag prints pathnames with their volume identifiers.
 *	- Version is now XE_V5.
 *	- Note that vol numbers for special files is not supported,
 *	  but this should not be an issue unless distributed volumes
 *	  actually contain special files (as with cpio, but not tar).
 *	M006	Jul  05, 1985	ncm
 *	- Corrected handling of duplicate [ug]ids
 *	- Added ability to undefine specified packages.
 *	M007	Aug  15, 1985	ncm
 *	- Added -i flag which checks only if the defined packages are
 *	  actually installed.  Returns NOTINST if no, INST if yes,
 *	  and PARTINST if the packages are partially installed.
 *	M008	Sep  11, 1985	ncm
 *	- upper case file types indicate files (and links) are optional.
 *	M009	Sep  16, 1985	rr
 *	- only issue warning if absolute path name in permlist and verbose.
 *	M010	Jan  6, 1986	rr
 *	- only check passwd and group entries if you're really fixing perms
 *	M011	Jan  22, 1986	ncm
 *	- print stats on pseudo package `ALL' when verbose iflag is set
 *	M012	Jan  30, 1986	ncm
 *	- make uflag work in conjunction with the iflag.  Also ignore the
 *	  installation status of packages consisting of only optional files.
 *	M013	Apr  09, 1986	rr
 *	- commented out all file type checking for now. this is not portable
 *	  and will in the future be enabled by an option when run on SCO XNX.
 *	M014	Jun  11, 1986	ncm
 *	- changed comment from M013 to #ifdef M_XENIX
 *	M015	Jul  15, 1986	rr
 *	- added "o" file type (meaning "ok, don't check my type").
 *	M016	Jul  18, 1986	guy
 *	- added gflag to print devices, also skip major/minor when getting
 *	  volume number token
 *	M017	Aug  26, 1986 beckyw
 *	- bbn doesn't know about version = XE_V5.
 *	M018	Aug 26, 1986 beckyw
 *	- code for checkfile, checkexec and checkarch is now
 *	ifdef'd under M_XENIX.
 *	M019    Aug 26, 1986 beckyw
 *	- Named pipes are not supported on version 7.  This
 *		case is ifdef'd under SYS3
 *	M020	Aug 26, 1986 beckyw
 *	- The makedev macro is included in param.h, so we don't
 *	include sysmacros.h.
 *	M021	Oct  30, 1986 ronh
 *	- changed ifndef BBN to ifdef M_XENIX
 *	M022	Nov  13, 1986 ronh
 *	- changed       #ifndef M_SYS3        to	#if SYS3 || M_SYS3
 *							#else
 *		 	# define strchr index	 	# define strchr index
 *	  Fixes problem with non-XENIX systemIII/V.
 *	M023	Feb   23, 1988 grf
 *	- Warns users if they try to install devices with fixperm.  UNIX 
 *	  idcmd's remove all devices and re-make them based on what is in
 *	  the $CONF/node.d directory.  
 *	- Also removed ifdefs for SYS3 and M_SYS3
 */
/*
 * static char sccsid[] = "@(#)fixperm.c	1.15 86/12/05 ";
 */

#include <stdio.h>
#include <ctype.h>
#include <a.out.h>
#include <ar.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef makedev
#ifndef BBN	 /* M020 */	
#include <sys/sysmacros.h>	/* contains the makedev macro */
#endif 	/* BBN */
#endif 	/* makedev */


#ifdef M_WORDSWAP
#  define hostwswap XC_WSWAP	/* set if host words swapped */
#else
#  define hostwswap 0
#endif

#define BADINODE	0	/* impossible inode number */

/* program return codes */
#define	OK		0	/* at least one package found in permlist */
#define	INST		0	/* if iflag, package is fully installed */
#define	ERROR		1	/* asssorted errors */
#define	SYNERR		2	/* syntax errors */
#define	NOPKG		3	/* no package was found in permlist */
#define	NOTINST		4	/* if iflag, package is not installed */
#define	PARTINST	5	/* if iflag, package is partially installed */

#define	FALSE		0
#define	TRUE		1
#define	REQFLD		0	/* required field	M005 */
#define	OPTFLD		1	/* optional field 	M005 */
#define	UNDEFINED	0	/* undefined package	M012 */
#define	DEFINED		1	/* defined package 	M012 */
#define MAXPKGS		50	/* maximum package definitions M005 */
#define MAXIDS		20	/* maximum uid/gid definitions */
#define MAXPATH		512	/* maximum full path name */
#define WHITESPACE	" \t\n"

char *firsttok();
char *nexttok();
extern char *fgets();
extern char *strchr();
extern char *strdup();
extern struct passwd *getpwnam();
extern struct group *getgrnam();
extern int optind;			/* M005 */
extern char *optarg;			/* M005 */

int cflag = 0;		/* create files when not found */
int nflag = 0;		/* report errors only */
int sflag = 0;		/* modify major/minor existing special files */
int Sflag = 0;		/* executable files must be x.out segmented */
int fflag = 0;		/* only print file list */
int Dflag = 0;		/* only print directory list */
int vflag = 0;		/* be verbose about errors 	M005 */
int wflag = 0;		/* print volume identifiers 	M005 */
int dflag = 0;		/* package names are defined	M006 */
int uflag = 0;		/* package names are undefined	M006 */
int iflag = 0;		/* check only if package is installed	M007 */
int aflag = 0;		/* all files specified must exist	M008 */
int gflag = 0;		/* print devices	M016 */
#ifdef M_XENIX		/* M017 */
int version = XE_V5;	/* os version identifier	M005 */
#endif 	/* M_XENIX */	/* M021 */

struct pkg {
	char *pkgname;
	int pkgdefn;	/* M012 */
	int pkginst;
};
int makeone;		/* Set when this specific file is to be created */
int makemode;		/* File mode for specials or plain files */
int maketyp;		/* Major/minor for char and block specials */
char *Prog;		/* This program's name */
char *fn;		/* current perms file */
FILE *fp;		/* current perms FILE pointer */
char name[MAXPATH];	/* current file name to be checked/adjusted */
int lineno;		/* current line number for error messages */
struct pkg dist[MAXPKGS];/* names and data of defined packages 	M007 */
struct pkg *pp;		/* pointer to current package name and data M007 */
int npkgs = 0;		/* number of package names defined	M005 */
int allpkgs = 0;	/* set if all packages are to be used	M005 */
int pkgfound = NOPKG;	/* was package found in the permfile 	M005 */
struct stat statbuf;

#define MKDIR "/bin/mkdir "
char sysbuf[150] = MKDIR;
char *sysbuf2 = sysbuf + sizeof(MKDIR) - 1;	/* 2nd word of sysbuf */



main(argc, argv)
int argc;
char **argv;
{
	register int c;
	int errflag = 0;

	Prog = argv[0];
	while ((c = getopt(argc, argv, "acsSfDlnvwgd:u:i")) != EOF) {
		switch (c) {
		case 'c':	cflag = 1;
				/* FALL THROUGH */
		case 's':	sflag = 1;
				break;
		case 'S':	Sflag = 1;
				break;
		case 'f':	fflag = 1;
				break;
		case 'D':	Dflag = 1;
				break;
		case 'l':	Dflag = fflag = 1;
				break;
		case 'n':	nflag = 1;
				break;
		case 'v':	vflag = 1;
				break;
		case 'w':	wflag = 1;
				break;
		case 'i':	iflag = 1;			/* M007 */
				break;
		case 'a':	aflag = 1;			/* M008 */
				break;
		case 'u':	uflag = 1;
				if (dflag)
				    fatal("only one of -u and -d allowed\n");
				setdist(optarg, UNDEFINED);	/* M012 */
				break;
		case 'd':	dflag = 1;
				if (uflag)
				    fatal("only one of -u and -d allowed\n");
				setdist(optarg, DEFINED);	/* M012 */
				break;
		case 'g':	gflag = 1;			/* M016 */
				break;
		case '?':	errflag++;
				break;
		}
	}
	if (errflag) {
		fatal("usage: %s [-csSfDlnvwiag] [-d pkg] [-u pkg] permlists",
			Prog);
	}
	if (wflag && !(fflag || Dflag)) {
		error("-w ignored unless -f, -l, or -D also specified");
	}
	if (geteuid() != 0 && (iflag | nflag | fflag | Dflag) == 0) { /* M007 */
		fatal("Must be super-user");
	}
	if (npkgs == 0) {		/* M007 */
		allpkgs++;
	}
	umask(0);			/* mode must be exactly as listed */
	while (optind < argc) {		/* M005 begin */
		fn = argv[optind++];
		if (!strcmp(fn, "-")) {
			fn = "standard input";
			fp = stdin;
			fixperm();
		}
		else if ((fp = fopen(fn, "r")) == NULL) {
			error("cannot open perms file: %s", fn);
		}
		else {
			fixperm();
			fclose(fp);
		}
	}
	if (vflag) {
		if (pkgfound == NOPKG)
			error("package(s) not found in perms file");
		else if (iflag) {
			if (ckdist("ALL"))			/* M011 */
				pp->pkginst = checkexit();	/* M011 */
			showinst();
		}
	}
	if (iflag)
		exit(checkexit());
	if (!(fflag || Dflag))		/* M010 */
		ckpwent();		/* check all uid & gid definitions */
	exit(pkgfound);		/* M005 end   M007 end */
}


setdist(s, defined)			/* M012 */
register char *s;
register int defined;			/* M012 */
{
	if (npkgs == MAXPKGS) {
		fatal("only %d package specifiers allowed", MAXPKGS);
	}
	dist[npkgs].pkgname = s;
	dist[npkgs].pkgdefn = defined;	/* M012 */
	dist[npkgs].pkginst = NOPKG;
	npkgs++;
}


ckdist(s)
register char *s;
{
	register int i;
	int found = 0;

	if (allpkgs && !iflag) {	/* default to all packages */
		return(1);
	}
	for (i = 0; i < npkgs; i++) {
		if (found = !strcmp(dist[i].pkgname, s))
			break;
	}
	if (iflag) {
		if (!found && (allpkgs || uflag)) {
			setdist(strdup(s), DEFINED);	/* M012 */
			found = 1;
		}
		if (found) {
			if (dist[i].pkginst == PARTINST)
				found = 0;
			else
				pp = &dist[i];
		}
	}
	return(found ? dist[i].pkgdefn : uflag);	/* M012 */
}


fixperm()
{
	register char *cp;
	register char *cp2;
	int skip;
	int ftype;
	int fmode;
	int uid;
	int gid;
	int nlinks;

	lineno = 0;
	while ((cp = firsttok()) != NULL) {

		/*
		 * Check for "uid name <uid>" definition
		 */
		if (strcmp("uid", cp) == 0 || strcmp("gid", cp) == 0) {
			if (iflag)			/* M007 */
				continue;
			gid = *cp;
			cp = nexttok(REQFLD);		/* get name */
			cp2 = nexttok(REQFLD);		/* get <uid> */
			if (!isdigit(*cp2)) {
				syntaxerr("bad %cid declaration syntax", gid);
			}
			identer(cp, atoi(cp2), gid);
			continue;
		}

		/*
		 * We should now have a `normal' perms file entry
		 */
		skip = !ckdist(cp); 		/* in this distrbution? */
		if (!skip)			/* M007 */
			pkgfound = OK;

		/*
		 * Get (optional) file type and mode
		 */
		cp = nexttok(REQFLD);		/* M005 */
		if (isdigit(*cp)) {
			ftype = 'f';		/* defaults to regular file */
		} else {
			ftype = *cp++;
		}
		if (iflag) {			/* M007 begin */
			nexttok(REQFLD);	/* skip to the links field */
			fmode = uid = gid = 0;
			goto getlinks;
		}				/* M007 end */
		fmode = atoio(cp);		/* ascii to int (octal) */

		/*
		 * Get numeric uid[/gid] or owner[/group]
		 */
		cp = nexttok(REQFLD);		/* M005 */
		if ((cp2 = strchr(cp, '/')) != NULL) {
			*cp2++ = '\0';		/* has optional gid or group */
		}
		if (isdigit(*cp)) {
			idverify(uid = atoi(cp), 'u');	/* numeric uid */
		} else {
			uid = idlookup(cp, 1, 'u');	/* owner */
		}

		if (cp2 == NULL) {
			idverify(gid = uid, 'g');	/* no gid or group */
		} else if (isdigit(*cp2)) {
			idverify(gid = atoi(cp2), 'g');	/* numeric gid */
		} else {
			gid = idlookup(cp2, 1, 'g');	/* group */
		}
		
		/*
		 * Get link count
		 */
	getlinks:				/* M007 */
		cp = nexttok(REQFLD);		/* M005 */
		if (!isdigit(*cp) || (nlinks = atoi(cp) -1) < 0) {
			syntaxerr("bad link count: `%s'", cp);
		}

		/*
		 * Get path name, disallow absolute path names.
		 */
		strcpy(name, nexttok(REQFLD));
		if (*name == '/' && vflag) {		/* M005 & M009 */
			error("%s must not start with / ", name);
		}

		/*
		 * Check for appropriate type, mode, links, etc.
		 * Create it if necessary.
		 */
		check(name, skip, ftype, fmode, uid, gid, nlinks);
	}
}




check(name, skip, ftype, fmode, uid, gid, nlinks)
char *name;
int skip;
int ftype;
int fmode;
int uid;
int gid;
int nlinks;
{
	register char *spectype;
	register char *linkname;
	int skiplinks = 0;
	int printlinks = 0;
	struct stat linkstat;
	char *vol; 		/* volume on which file is located M005 */
	int found;		/* indicates whether name exists   M007 */
	int optfile = FALSE;	/* set if file is optional 	   M008	*/

	if (skip) {
		skiplinks = 1;
		goto dolinks;
	}
	if (isupper(ftype)) {	/* upper case type = optional file M008	*/
		if (!aflag)	/* aflag forces all files to exist M008	*/
			optfile = TRUE;
		ftype = tolower(ftype);
	} 							/* M008 */
	if (fflag || Dflag || gflag) {				/* M016 */
		if ((fflag && ftype && strchr("efxao", ftype)) || /* M015 */
		   (Dflag && ftype == 'd') ||
		   /* M016 -- if gflag skip next token (major/minor)
		    *	      and print devices
		    */
		   (gflag && ftype && strchr("bc", ftype) && nexttok(OPTFLD))) {
			/* just print filename and vol, if specified */
			if (wflag && (vol = nexttok(OPTFLD)) != NULL) {
				printf("%s\t%s\n", name, vol);
			} else {
				printf("%s\n", name);
			}
			printlinks = TRUE;
		}
		skiplinks = TRUE;
		goto dolinks;
	}

	makeone = 0;
	makemode = 0;
	found = (stat(name, &statbuf) == -1 ? NOTINST : INST);	/* M007 begin */
	if (iflag) {
		if (optfile) 					/* M008 */
			skiplinks = TRUE;
		else if (pp->pkginst == NOPKG)
			pp->pkginst = found;
		else if (pp->pkginst != found)
			pp->pkginst = PARTINST;
		goto dolinks;
	}
	if (found == NOTINST) {					/* M007 end */
		if (!nflag && cflag) {
			makeone = 1;		/* Make the file */
		} else {
			if (!optfile)				/* M008 */
				error("file not found: %s", name);
			if (ftype == 'c' || ftype == 'b') {
				maketyp = getdev();	/* scan maj/min */
			}
			statbuf.st_ino = BADINODE;  /* unused inode # */
			goto dolinks;
		}
	}
	/*
	 * If we get here, the file already exists OR
	 *	1) -n was not used &&
	 *	2) -c was used &&
	 *	3) makeone has been set to 1
	 */
	switch (ftype) {
		case 'd':
			if (makeone) {
				strcpy(sysbuf2, name);
				system(sysbuf);
				if (stat(name, &statbuf) == -1) {
					fatal("\"%s\" failed", sysbuf);
				}
				makeone = 0;	/* Dir has been made */
			} else if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
				error("not a directory: %s", name);
			}
			break;

		case 'b':
			makemode = S_IFBLK;
			checkdev("block");
			break;

		case 'c':
			makemode = S_IFCHR;
			checkdev("char");
			break;

		case 'a':
		case 'f':
		case 'o':		/* M015 */
		case 'x':
			if (makeone) {
				statbuf.st_ino = BADINODE;  /* unused inode # */
				if (!optfile)			/* M008 */
					error("file not found: %s", name);
				goto dolinks;
			}
#ifdef M_XENIX
			checkfile(ftype);	/* M013 M014 */
#endif
			break;

		case 'e':
			makemode = S_IFREG;
			maketyp = 0;
			if (!makeone) {
				if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
					error("not a plain file: %s", name);
				} else if (statbuf.st_size) {
					error("not an empty file: %s", name);
				}
			}
			break;

		case 'p':					/*M003 begin*/
			makemode = S_IFIFO;
			maketyp = 0;
			if (!makeone) {
				if ((statbuf.st_mode & S_IFMT) != S_IFIFO) {
					error("not a named pipe: %s", name);
				}
 			}
			break;					/* M003 end */

		default:
			syntaxerr("file type %c unknown for %s", ftype, name);
			break;
	}
	if (makeone) {			/* Create file */
		/*  Warn users of devices not installed using UNIX idcmd's M004 */
		if (ftype == 'b' || ftype == 'c')
			warning("Devices installed by fixperm will be removed during kernel re-configuration");
		if (mknod(name, makemode | fmode, maketyp) == -1) {
			fatal("Cannot create %s", name);
		}
	} else if (fmode != (statbuf.st_mode & ~S_IFMT)) {
		if (nflag) {
			error("incorrect mode %o, should be %o: %s",
			    statbuf.st_mode & ~S_IFMT, fmode, name);
		} else if (chmod(name, fmode) == -1) {
			fatal("Cannot chmod %s 0%o", name, fmode);
		}
	}
	if (statbuf.st_uid != uid || statbuf.st_gid != gid) {
		if (nflag) {
			error("incorrect uid/gid %d/%d, should be %d/%d: %s",
			statbuf.st_uid, statbuf.st_gid, uid, gid, name);
		} else if (chown(name, uid, gid) == -1) {
			fatal("Cannot chown %s %d %d", name, uid, gid);
		}
	}
dolinks:
	while (nlinks-- > 0) {
		if ((linkname = firsttok()) == NULL) {
			syntaxerr("link filename missing");	/* M005 */
		}
		if (*linkname == '/' && vflag) {	/* M005 & M009 */
			error("%s must not start with / ", linkname);
		}
		if (skiplinks) {
			if (printlinks) {
				/* just print filename and vol, if specified */
				if (wflag && (vol = nexttok(OPTFLD)) != NULL) {
					printf("%s\t%s\n", linkname, vol);
				} else {
					printf("%s\n", linkname);
				}
			}
			continue;
		}
							/* M007 begin */
		found = (stat(linkname, &linkstat) == -1 ? NOTINST : INST);
		if (iflag) {
			if (pp->pkginst != found && !optfile)	/* M008 */
				pp->pkginst = PARTINST;
			continue;
		}
		if (found == NOTINST) {			/* M007 end */
			if (!nflag && cflag) {
				if (link(name, linkname) == -1 && !optfile) {
					error("can't link %s to %s",	/*M001*/
					    linkname, name);
				}
			} else if (!optfile) {		/* M008 */
				error("file not found: %s", linkname);	/*M003*/
			}
		} else if (statbuf.st_ino != linkstat.st_ino ||
		    statbuf.st_dev != linkstat.st_dev) {
			error("%s not linked to %s", linkname, name);
		}
	}
}




checkdev(spectype)
char *spectype;
{
	if (!makeone && (statbuf.st_mode & S_IFMT) != makemode) {
		error("not %s special: %s", spectype, name);
	} else if ((maketyp = getdev()) != -1 && maketyp != statbuf.st_rdev) {
		if (nflag) {
			error("incorrect major/minor: %s", name);
		} else if (sflag) {
			makeone = 1;
			unlink(name);
		}
	}
}



		  /* M018 */
#ifdef M_XENIX    /* The following 3 routines a xenix specific */

checkfile(ftype)
int ftype;
{
	register FILE *cfp;
	register int bout = 0;
	struct xexec xexec;

	if (ftype == 'o') return;		/* M015 */

	if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
		error("not a plain file: %s", name);
		return;
	}
	if ((cfp = fopen(name, "r")) == NULL) {
		error("cannot open file: %s", name);
		return;
	}
	if (fread((char *) &xexec, sizeof(struct xexec), 1, cfp) != 1) {
		ef (ftype == 'x' || ftype == 'a') {
			error("cannot read file header: %s", name);
		}
		fclose(cfp);
		return;
	}
	fclose(cfp);
	if (ftype == 'x') {
		checkexec(&xexec);
		return;
	} else {
		if (xexec.x_magic == 0) {
			xexec.x_magic = ((struct bexec *) &xexec)->xb_magic;
		}
		switch (xexec.x_magic) {
		case X_MAGIC:
		case A_MAGIC1:
		case A_MAGIC2:
		case A_MAGIC3:
		case A_MAGIC4:
		case SBSWAP(X_MAGIC):
		case SBSWAP(A_MAGIC1):
		case SBSWAP(A_MAGIC2):
		case SBSWAP(A_MAGIC3):
		case SBSWAP(A_MAGIC4):
			error("unexpected executable: %s", name);
			return;
		}
	}
	if (ftype == 'a') {
		checkarch((char *) &xexec);
	} else {
		switch (xexec.x_magic) {
		case ARCMAGIC:
		case SBSWAP(ARCMAGIC):
			error("unexpected archive: %s", name);
		}
	}
}




checkexec(xp)
register struct xexec *xp;
{
	if (xp->x_magic != X_MAGIC) {
		error("bad format executable file: %s", name);
	} else {
		if ((xp->x_renv & XE_EXEC) != XE_EXEC) {		/*M002*/
			error("not executable: %s", name);
		}
#ifdef XE_SEG
		if ((xp->x_renv & XE_SEG) == 0 && Sflag) {
			error("file is not x.out segmented: %s", name);
		}
#endif
		if (!vflag)
			return;
		if ((xp->x_cpu & XC_WSWAP) != hostwswap) {
			error("bad word order: %s", name);
		}
		if ((xp->x_renv & XE_VERS) != version) {
			error("bad version executable file: %s", name);
		}
		if ((xp->x_renv & XE_FS) == 0) {
			error("file is not fixed stack: %s", name);
		}
		if ((xp->x_renv & XE_SEP) == 0) {
			error("file is not separate I/D: %s", name);
		}
		if (xp->x_syms) {
			error("executable file not stripped: %s", name);
		}
	}
}




checkarch(buf)
char *buf;
{
	register struct ar_hdr *ap;

	if (*(unsigned short *) buf != ARCMAGIC) {
		error("file not archive: %s", name);
		return;
	}
	ap = (struct ar_hdr *) &buf[sizeof(unsigned short)];
	if (ap->ar_size > statbuf.st_size) {
		error("word swapped archive: %s", name);
	}
}
#endif	/* M_XENIX */




static char *tokp;

char *
firsttok()
{
	register char *cp;
	static char linebuf[MAXPATH];

	while ((cp = fgets(linebuf, sizeof(linebuf), fp)) != NULL) {
		lineno++;
		while (*cp && strchr(WHITESPACE, *cp)) {
			cp++;
		}
		if (*cp == '\0' || *cp == '#') {    /* if blank line or '#' */
			continue;		    /* is a comment line */
		}
		tokp = cp;
		return(nexttok(REQFLD));
	}
	return((char *) NULL);
}




char *
nexttok(opt)
int opt;
{
	register char *cp;
	register char *tokstart;

	cp = tokp;
	while (*cp && strchr(WHITESPACE, *cp)) {
		cp++;
	}
	if (*cp == '\0' && opt == REQFLD) {
		syntaxerr("missing data field");
	}
	tokstart = cp;			/* save start of token */
	while (*cp) {
		if (strchr(WHITESPACE, *cp)) {
			*cp++ = '\0';	/* terminate token and point to next */
			break;
		}
		cp++;
	}
	tokp = cp;
	return(tokstart);
}




getdev()
{
	register char *cp;
	register char *maj;

	maj = cp = nexttok(REQFLD);
	if (!isdigit(*cp) || (cp = strchr(cp,'/')) == NULL || !isdigit(*++cp)) {
		syntaxerr("bad major/minor: %s", maj);
	}
	return(makedev(atoi(maj), atoi(cp)));
}




atoio(s)
char *s;
{
	register int c;
	register int r = 0;

	while (c = *s++) {
		if (c < '0' || c > '7') {
			syntaxerr("bad mode digit: `%c'", c);
		}
		r = (r << 3) + c - '0';
	}
	return(r);
}




#define I_ISGID		0x8000
#define I_IDMASK	0x7fff


static struct {
	char		*i_name;
	unsigned short	i_id;
} idtab[MAXIDS];

static nids = 0;

identer(name, id, flag)
char *name;
int id;
int flag;		/* flag == 'g' for gid, 'u' for uid */
{
	int oldid;

	if ((oldid = idlookup(name, 0, flag)) >= 0) {
		if (oldid != id) {
			syntaxerr("`%cid %s %d' redefined to `%d'",
			    flag, name, oldid, id);
		}
		return;		/* M006 */
	}
	if (nids >= MAXIDS) {
		fatal("only %d uid/gid definitions allowed", MAXIDS);
	}
	if ((idtab[nids].i_name = strdup(name)) == NULL) {
		fatal("out of memory");
	}
	idtab[nids++].i_id = id | ((flag == 'g')? I_ISGID : 0);
}




idlookup(name, force, flag)
char *name;
int force;
int flag;		/* flag == 'g' for gid, 'u' for uid */
{
	register int i;
	register int gidbit;

	gidbit = (flag == 'g')? I_ISGID : 0;
	for (i = 0; i < nids; i++) {
		if ((idtab[i].i_id & I_ISGID) == gidbit &&
		    strcmp(idtab[i].i_name, name) == 0) {
			return(idtab[i].i_id & I_IDMASK);
		}
	}
	if (force) {
		syntaxerr("no %cid definition for `%s'", flag, name);
	}
	return(-1);
}




idverify(id, flag)
int id;
int flag;		/* flag == 'g' for gid, 'u' for uid */
{
	register int i;

	if (flag == 'g') {
		id |= I_ISGID;
	}
	for (i = 0; i < nids; i++) {
		if (idtab[i].i_id == id) {
			return;
		}
	}
	syntaxerr("no definition for %cid %d", flag, id & I_IDMASK);
}



ckpwent()
{
	register int i;
	register struct passwd *pp;
	register struct group *gp;

	/*
	 * In order to use the standard passwd/group library routines
	 * we try to chroot so the correct passwd/group files are used.
	 * If that fails but the current directory is /, we're ok.
	 */
	if (chroot(".") < 0) {
		struct stat root, dot;

		if (stat("/", &root) < 0 || stat(".", &dot) < 0 ||
		    root.st_ino != dot.st_ino) {
			error("must be super-user to check passwd entries");
			return;
		}
	}
	for (i = 0; i < nids; i++) {
		if (idtab[i].i_id & I_ISGID) {
			continue;
		}
		if ((pp = getpwnam(idtab[i].i_name)) == NULL) {
			error("no ./etc/passwd entry for %s", idtab[i].i_name);
		} else if (pp->pw_uid != (idtab[i].i_id & I_IDMASK)) {
			error("bad uid in ./etc/passwd entry for %s",
					idtab[i].i_name);
		}
	}
	for (i = 0; i < nids; i++) {
		if ((idtab[i].i_id & I_ISGID) == 0) {
			continue;
		}
		if ((gp = getgrnam(idtab[i].i_name)) == NULL) {
			error("no ./etc/group entry for %s", idtab[i].i_name);
		} else if (gp->gr_gid != (idtab[i].i_id & I_IDMASK)) {
			error("bad gid in ./etc/group entry for %s",
					idtab[i].i_name);
		}
	}
}




/*VARARGS1*/
syntaxerr(fmt, a, b, c, d, e, f, g, h, i)
char *fmt;
{
	fprintf(stderr, "%s: ", Prog);
	fprintf(stderr, fmt, a, b, c, d, e, f, g, h, i);
	fprintf(stderr, ", %s line %d\n", fn, lineno);
	exit(SYNERR);
}




/*VARARGS1*/
error(fmt, a, b, c, d, e, f, g, h, i)
char *fmt;
{
	fprintf(stderr, "%s: ", Prog);
	fprintf(stderr, fmt, a, b, c, d, e, f, g, h, i);
	fputc('\n', stderr);
}




/*VARARGS1*/
/*VARARGS1*/
warning(fmt, a, b, c, d, e, f, g, h, i)
char *fmt;
{
	fprintf(stderr, "WARNING: %s: ", Prog);
	fprintf(stderr, fmt, a, b, c, d, e, f, g, h, i);
	fputc('\n', stderr);
}




/*VARARGS1*/
fatal(fmt, a, b, c, d, e, f, g, h, i)
char *fmt;
{
	error(fmt, a, b, c, d, e, f, g, h, i);
	fprintf(stderr, "fatal error\n");
	exit(ERROR);
}

/*
 * showinst - displays a one word message describing whether or not
 *	each package in dist is installed.  Packages consisting of
 *	only optional files are considered to be `unknown'
 */
showinst()
{
	register int i;
	char *adj;

	for (i = 0; i < npkgs; ++i) {
		if (dist[i].pkgdefn != DEFINED)	/* M012 */
			continue;		/* M012 */
		switch(dist[i].pkginst) {
		case NOPKG:
			continue;		/* M012 */
		case INST:
			adj = "Yes";
			break;
		case NOTINST:
			adj = "No";
			break;
		case PARTINST:
			adj = "Part";
			break;
		default:
			fatal("internal error");
		}
		printf("%s\t%s\n", dist[i].pkgname, adj);
	}
}

checkexit()
{
	register int i;
	register int stat;

	/*
	 * M012 
	 * Ignore packages with only optional files (NOPKG)
	 * and undefined packages
	 */
	for (i = 0; i < npkgs; ++i) {
		if (dist[i].pkginst != NOPKG && dist[i].pkgdefn == DEFINED) {
			stat = dist[i].pkginst;
			break;
		}
	}
	for ( ; i < npkgs; ++i) {
		if (dist[i].pkgdefn == DEFINED &&
			dist[i].pkginst != stat && dist[i].pkginst != NOPKG)
				return(PARTINST);
	}
	return(stat);
}


