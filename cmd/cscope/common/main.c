/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/main.c	1.8"
/*	cscope - interactive C symbol cross-reference
 *
 *	main functions
 */

#include "global.h"
#include "version.h"	/* FILEVERSION and FIXVERSION */
#include <curses.h>
#include <fcntl.h>	/* O_RDONLY */
#include <sys/types.h>	/* needed by stat.h */
#include <sys/stat.h>	/* stat */

/* defaults for unset environment variables */
#define	EDITOR	"vi"
#define	SHELL	"sh"
#define TMPDIR	"/usr/tmp"

/* note: these digraph character frequencies were calculated from possible 
   printable digraphs in the cross-reference for the C compiler */
char	dichar1[] = " teisaprnl(of)=c";	/* 16 most frequent first chars */
char	dichar2[] = " tnerpla";		/* 8 most frequent second chars 
					   using the above as first chars */
char	dicode1[256];		/* digraph first character code */
char	dicode2[256];		/* digraph second character code */

char	*editor, *home, *shell;	/* environment variables */
char	*argv0;			/* command name */
BOOL	compress = YES;		/* compress the characters in the crossref */
int	dispcomponents = 1;	/* file path components to display */
#if CCS
BOOL	displayversion;		/* display the C Compilation System version */
#endif
BOOL	editallprompt = YES;	/* prompt between editing files */
int	fileargc;		/* file argument count */
char	**fileargv;		/* file argument values */
int	fileversion;		/* cross-reference file version */
BOOL	incurses;		/* in curses */
BOOL	isuptodate;		/* consider the crossref up-to-date */
BOOL	linemode;		/* use line oriented user interface */
char	*namefile;		/* file of file names */
char	*newreffile;		/* new cross-reference file name */
FILE	*newrefs;		/* new cross-reference */
BOOL	ogs;			/* display OGS book and subsystem names */
char	*prependpath;		/* prepend path to file names */
char	*reffile = REFFILE;	/* cross-reference file path name */
FILE	*refsfound;		/* references found file */
int	symrefs = -1;		/* cross-reference file */
char	temp1[PATHLEN + 1];	/* temporary file name */
char	temp2[PATHLEN + 1];	/* temporary file name */
BOOL	truncate;		/* truncate symbols to 8 characters */

static	BOOL	buildonly;	/* only build the cross-reference */
static	BOOL	fileschanged;	/* assume some files changed */
static	long	offset;		/* file trailer offset */
static	BOOL	onesearch;	/* one search only in line mode */
static	char	*tmpdir;	/* temporary directory */
static	BOOL	unconditional;	/* unconditionally build the databse */

main(argc, argv)
int	argc;
char	**argv;
{
	FILE	*names;			/* name file pointer */
	int	oldnum;			/* number in old cross-ref */
	char	path[PATHLEN + 1];	/* file path */
	register FILE	*oldrefs;	/* old cross-reference file */
	register char	*s;
	register int	c, i;
	pid_t	pid;
	BOOL	command();
	char	*mygetenv();
	void	dispinit(), includedir(), initcompress(), initsymtab(), skiplist(), sourcedir();
	
	/* save the command name for messages */
	argv0 = argv[0];
	
	/* set the options */
	while (--argc > 0 && (*++argv)[0] == '-') {
		for (s = argv[0] + 1; *s != '\0'; s++) {
			
			/* look for an input field number */
			if (isdigit(*s)) {
				field = *s - '0';
				if (field > 8) {
					field = 8;
				}
				if (*++s == '\0' && --argc > 0) {
					s = *++argv;
				}
				if ((int)strlen(s) > PATLEN) {
					(void) fprintf(stderr, "cscope: pattern too long, cannot be > %d characters\n", PATLEN);
					exit(1);
				}
				(void) strcpy(pattern, s);
				goto nextarg;
			}
			switch (*s) {
			case '-':	/* end of options */
				--argc;
				++argv;
				goto lastarg;
			case 'V':	/* print the version number */
#if CCS
				displayversion = YES;
				break;
#else
				(void) fprintf(stderr, "%s: version %d%s\n", argv0,
					FILEVERSION, FIXVERSION);
				exit(0);
#endif
			case 'b':	/* only build the cross-reference */
				buildonly = YES;
				break;
			case 'c':	/* ASCII characters only in crossref */
				compress = NO;
				break;
			case 'C':	/* turn on caseless mode for symbol searches */
				caseless = YES;
				egrepcaseless(caseless);	/* simulate egrep -i flag */
				break;
			case 'd':	/* consider crossref up-to-date */
				isuptodate = YES;
				break;
			case 'e':	/* suppress ^E prompt between files */
				editallprompt = NO;
				break;
			case 'L':
				onesearch = YES;
				/* FALLTHROUGH */
			case 'l':
				linemode = YES;
				break;
			case 'o':	/* display OGS book and subsystem names */
				ogs = YES;
				break;
			case 'T':	/* truncate symbols to 8 characters */
				truncate = YES;
				break;
			case 'u':	/* unconditionally build the cross-reference */
				unconditional = YES;
				break;
			case 'U':	/* assume some files have changed */
				fileschanged = YES;
				break;
			case 'f':	/* alternate cross-reference file */
			case 'i':	/* file containing file names */
			case 'I':	/* #include file directory */
			case 'p':	/* file path components to display */
			case 'P':	/* prepend path to file names */
			case 's':	/* additional source file directory */
			case 'S':
				c = *s;
				if (*++s == '\0' && --argc > 0) {
					s = *++argv;
				}
				if (*s == '\0') {
					(void) fprintf(stderr, "%s: -%c option: missing or empty value\n", 
						argv0, c);
					goto usage;
				}
				switch (c) {
				case 'f':	/* alternate cross-reference file */
					reffile = s;
					break;
				case 'i':	/* file containing file names */
					namefile = s;
					break;
				case 'I':	/* #include file directory */
					includedir(s);
					break;
				case 'p':	/* file path components to display */
					if (*s < '0' || *s > '9' ) {
						(void) fprintf(stderr, "%s: -p option: missing or invalid numeric value\n", 
							argv0);
						goto usage;
					}
					dispcomponents = atoi(s);
					break;
				case 'P':	/* prepend path to file names */
					prependpath = s;
					break;
				case 's':	/* additional source directory */
				case 'S':
					sourcedir(s);
					break;
				}
				goto nextarg;
			default:
				(void) fprintf(stderr, "%s: unknown option: -%c\n", argv0, 
					*s);
			usage:
				(void) fprintf(stderr, "Usage:  cscope [-bcdelLoTuUV] [-f file] [-i file] [-I directory] [-s directory]\n");
				(void) fprintf(stderr, "               [-p number] [-P path] [-[0-8] text] [source files]\n");
				exit(1);
			}
		}
nextarg:	;
	}
lastarg:
	/* read the environment */
	editor = mygetenv("EDITOR", EDITOR);
	editor = mygetenv("VIEWER", editor);	/* use viewer if set */
	home = getenv("HOME");
	shell = mygetenv("SHELL", SHELL);
	tmpdir = mygetenv("TMPDIR", TMPDIR);
	
	/* see if you can create the cross-reference in this dir */
	if (reffile[0] != '/' && access(".", WRITE) != 0) {

		/* no, put it in the home directory */
		s = mymalloc((unsigned) (strlen(reffile) + strlen(home) + 2));
		(void) sprintf(s, "%s/%s", home, reffile);
		reffile = s;
	}
	/* if the cross-reference is to be considered up-to-date */
	if (isuptodate == YES) {
		if ((oldrefs = fopen(reffile, "r")) == NULL) {
			(void) fprintf(stderr, "cscope: cannot open file %s\n", reffile);
			exit(1);
		}
		/* get the crossref file version but skip the current directory */
		if (fscanf(oldrefs, "cscope %d %*s", &fileversion) != 1) {
			(void) fprintf(stderr, "cscope: cannot read file version from file %s\n", reffile);
			exit(1);
		}
		if (fileversion >= 8) {

			/* see if there are options in the database */
			for (;;) {
				(void) getc(oldrefs);	/* skip the blank */
				if ((c = getc(oldrefs)) != '-') {
					(void) ungetc(c, oldrefs);
					break;
				}
				switch (c = getc(oldrefs)) {
				case 'c':	/* ASCII characters only */
					compress = NO;
					break;
				case 'T':	/* truncate symbols to 8 characters */
					truncate = YES;
					break;
				}
			}
			initcompress();

			/* seek to the trailer */
			if (fscanf(oldrefs, "%d", &offset) != 1) {
				(void) fprintf(stderr, "cscope: cannot read trailer offset from file %s\n", reffile);
				exit(1);
			}
			if (fseek(oldrefs, offset, 0) == -1) {
				(void) fprintf(stderr, "cscope: cannot seek to trailer in file %s\n", reffile);
				exit(1);
			}
		}
		/* skip the source and include directory lists */
		skiplist(oldrefs);
		skiplist(oldrefs);

		/* get the number of source files */
		if (fscanf(oldrefs, "%d", &nsrcfiles) != 1) {
			(void) fprintf(stderr, "cscope: cannot read source file size from file %s\n", reffile);
			exit(1);
		}
		/* get the source file list */
		srcfiles = (char **) mymalloc(nsrcfiles * sizeof(char *));
		if (fileversion >= 9) {

			/* allocate the string space */
			if (fscanf(oldrefs, "%d", &oldnum) != 1) {
				(void) fprintf(stderr, "cscope: cannot read string space size from file %s\n", reffile);
				exit(1);
			}
			s = mymalloc((unsigned) oldnum);
			(void) getc(oldrefs);	/* skip the newline */
			
			/* read the strings */
			if (fread(s, oldnum, 1, oldrefs) != 1) {
				(void) fprintf(stderr, "cscope: cannot read source file names from file %s\n", reffile);
				exit(1);
			}
			/* change newlines to nulls */
			for (i = 0; i < nsrcfiles; ++i) {
				srcfiles[i] = s;
				for (++s; *s != '\n'; ++s) {
					;
				}
				*s = '\0';
				++s;
			}
			/* if there is a file of source file names */
			if (namefile != NULL && (names = vpfopen(namefile, "r")) != NULL ||
			    (names = vpfopen(NAMEFILE, "r")) != NULL) {
	
				/* read any -p option from it */
				while (fscanf(names, "%s", path) == 1 && *path == '-') {
					i = path[1];
					s = path + 2;		/* for "-Ipath" */
					if (*s == '\0') {	/* if "-I path" */
						(void) fscanf(names, "%s", path);
						s = path;
					}
					switch (i) {
					case 'p':	/* file path components to display */
						if (*s < '0' || *s > '9') {
							(void) fprintf(stderr, "cscope: -p option in file %s: missing or invalid numeric value\n", 
								namefile);
						}
						dispcomponents = atoi(s);
					}
				}
				(void) fclose(names);
			}
		}
		else {
			for (i = 0; i < nsrcfiles; ++i) {
				if (fscanf(oldrefs, "%s", path) != 1) {
					(void) fprintf(stderr, "cscope: cannot read source file name from file %s\n", reffile);
					exit(1);
				}
				srcfiles[i] = stralloc(path);
			}
		}
		(void) fclose(oldrefs);
	}
	else {
		/* save the file arguments */
		fileargc = argc;
		fileargv = argv;
	
		/* get source directories from the environment */
		if ((s = getenv("SOURCEDIRS")) != NULL) {
			sourcedir(s);
		}
		/* make the source file list */
		srcfiles = (char **) mymalloc(msrcfiles * sizeof(char *));
		makefilelist();
		if (nsrcfiles == 0) {
			(void) fprintf(stderr, "cscope: no source files found\n");
			exit(1);
		}
		/* get include directories from the environment */
		if ((s = getenv("INCLUDEDIRS")) != NULL) {
			includedir(s);
		}
		/* add /usr/include to the #include directory list */
		includedir("/usr/include");

		/* initialize the C keyword table */
		initsymtab();
	
		/* create the file name used for a new cross-reference */
		newreffile = mymalloc((unsigned) (strlen(reffile) + strlen(home) + 3));
		if ((s = strrchr(reffile, '/')) != NULL) {
			++s;
			(void) strcpy(newreffile, reffile);
			(void) sprintf(newreffile + (s - reffile), "n%s", s);
		}
		else {
			(void) sprintf(newreffile, "n%s", reffile);
		}
		/* build the cross-reference */
		initcompress();
		build();
		if (buildonly == YES) {
			exit(0);
		}
	}
	/* open the cross-reference file */
	if ((symrefs = open(reffile, O_RDONLY)) == -1) {
		(void) fprintf(stderr, "cscope: cannot open file %s\n", reffile);
		exit(1);
	}
	/* ignore the interrupt signal and if running in the foreground */
	if (linemode == NO && signal(SIGINT, SIG_IGN) != SIG_IGN) {
		void myexit();	/* needed by ctrace */

		/* cleanup on the quit signal */
		signal(SIGQUIT, myexit);
	}
	/* cleanup on the hangup signal */
	signal(SIGHUP, myexit);

	/* create the temporary files */
	pid = getpid();
	(void) sprintf(temp1, "%s/cscope%d.1", tmpdir, pid);
	(void) sprintf(temp2, "%s/cscope%d.2", tmpdir, pid);

	/* if using the line oriented user interface so cscope can be a 
	   subprocess to emacs or samuel */
	if (linemode == YES) {
		if (*pattern != '\0') {		/* do any optional search */
			if (search() == YES) {
				while ((c = getc(refsfound)) != EOF) {
					(void) putchar(c);
				}
			}
		}
		if (onesearch == YES) {
			myexit(0);
		}
		for (;;) {
			char buf[PATLEN + 2];
			
			(void) printf(">> ");
			(void) fflush(stdout);
			if (fgets(buf, sizeof(buf), stdin) == NULL) {
				myexit(0);
			}
			/* remove any trailing newline character */
			if (*(s = buf + strlen(buf) - 1) == '\n') {
				*s = '\0';
			}
			switch (*buf) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':	/* samuel only */
				field = *buf - '0';
				(void) strcpy(pattern, buf + 1);
				search();
				(void) printf("cscope: %d lines\n", totallines);
				while ((c = getc(refsfound)) != EOF) {
					(void) putchar(c);
				}
				break;

			case 'r':	/* rebuild database cscope style */
			case ctrl('R'):
				freefilelist();
				makefilelist();
				/* FALLTHROUGH */

			case 'R':	/* rebuild database samuel style */
				rebuild();
				(void) putchar('\n');
				break;

			case 'C':	/* clear file names */
				freefilelist();
				(void) putchar('\n');
				break;

			case 'F':	/* add a file name */
				(void) strcpy(path, buf + 1);
				if (infilelist(path) == NO &&
				    (s = inviewpath(path)) != NULL) {
					addsrcfile(path, s);
				}
				(void) putchar('\n');
				break;

			case 'q':	/* quit */
			case ctrl('D'):
			case ctrl('Z'):
				myexit(0);

			default:
				(void) fprintf(stderr, "cscope: unknown command '%s'\n", buf);
				break;
			}
		}
	}
	else {	/* screen-oriented user interface */

		/* pause before clearing the screen if there have been error messages */
		if (errorsfound == YES) {
			errorsfound = NO;
			askforreturn();
		}
		/* initialize the curses display package */
		initscr();		/* initialize the screen */
		nonl();			/* don't translate an output \n to \n\r */
		cbreak();		/* single character input */
		noecho();		/* don't echo input characters */
#if TERMINFO
		keypad(stdscr, TRUE);	/* enable the keypad */
#if SVR2 && !BSD && !V9 && !u3b2 && !sun
		fixkeypad();		/* fix for getch() intermittently returning garbage */
#endif
#endif
#if UNIXPC
		standend();		/* turn off reverse video */
#endif
		incurses = YES;
	}
	clear();	/* clear the screen */
	mouseinit();	/* initialize any mouse interface */
	dispinit();	/* initialize display parameters */
	setfield();	/* set the initial cursor position */
	display();	/* display the version number and input fields */

	/* do any optional search */
	if (*pattern != '\0') {
		atfield();		/* move to the input field */
		command(ctrl('Y'));	/* search */
		display();		/* update the display */
	}
	for (;;) {
		atfield();	/* move to the input field */
		
		/* exit if the quit command is entered */
		if ((c = mygetch()) == EOF || c == ctrl('D') || c == ctrl('Z')) {
			break;
		}
		/* execute the commmand, updating the display if necessary */
		if (command(c) == YES) {
			display();
		}
	}
	/* cleanup and exit */
	myexit(0);
	/* NOTREACHED */
}

/* set up the digraph character tables for text compression */

void
initcompress()
{
	register int	i;
	
	if (compress == YES) {
		for (i = 0; i < 16; ++i) {
			dicode1[(unsigned)(dichar1[i])] = i * 8 + 1;
		}
		for (i = 0; i < 8; ++i) {
			dicode2[(unsigned)(dichar2[i])] = i + 1;
		}
	}
}

/* rebuild the database */

void
rebuild()
{
	close(symrefs);
	build();
	if ((symrefs = open(reffile, O_RDONLY)) == -1) {
		(void) fprintf(stderr, "cscope: cannot open file: %s\n", reffile);
		myexit(1);
	}
	/* revert to the initial display */
	if (refsfound != NULL) {
		(void) fclose(refsfound);
		refsfound = NULL;
	}
}

/* build the cross-reference */

void
build()
{
	register char	*cp;
	register int	i;
	register FILE	*oldrefs;	/* old cross-reference file */
	register time_t	reftime;	/* old crossref modification time */
	char	*oldfile;		/* file in old cross-reference */
	char	newdir[PATHLEN + 1];	/* directory in new cross-reference */
	char	olddir[PATHLEN + 1];	/* directory in old cross-reference */
	char	oldname[PATHLEN + 1];	/* name in old cross-reference */
	int	oldnum;			/* number in old cross-ref */
	struct	stat statstruct;	/* file status */
	int	firstfile;		/* first source file in pass */
	int	lastfile;		/* last source file in pass */
	int	built = 0;		/* built crossref for these files */
	int	copied = 0;		/* copied crossref for these files */
	BOOL	samelist();
	char	*getoldfile();
	int	compare();		/* needed for qsort call */
	void	buildprogress(), crossref(), putheader(), putlist();

	/* normalize the current directory relative to the home directory so
	   the cross-reference is not rebuilt when the user's login is moved */
	(void) strcpy(newdir, currentdir);
	if (strcmp(currentdir, home) == 0) {
		(void) strcpy(newdir, "$HOME");
	}
	else if (strncmp(currentdir, home, strlen(home)) == 0) {
		(void) sprintf(newdir, "$HOME%s", currentdir + strlen(home));
	}
	/* sort the source file names (needed for rebuilding) */
	qsort((char *) srcfiles, (unsigned) nsrcfiles, sizeof(char *), compare);

	/* if there is an old cross-reference and its current directory matches */
	/* or this is an unconditional build */
	if ((oldrefs = fopen(reffile, "r")) != NULL && unconditional == NO &&
	    fscanf(oldrefs, "cscope %d %s", &fileversion, olddir) == 2 &&
	    (strcmp(olddir, currentdir) == 0 || /* remain compatible */
	     strcmp(olddir, newdir) == 0)) {
		if (fileversion >= 8) {
			BOOL	oldcompress = YES;
			BOOL	oldtruncate = NO;
			int	c;

			/* see if there are options in the database */
			for (;;) {
				(void) getc(oldrefs);	/* skip the blank */
				if ((c = getc(oldrefs)) != '-') {
					(void) ungetc(c, oldrefs);
					break;
				}
				switch (c = getc(oldrefs)) {
				case 'c':	/* ASCII characters only */
					oldcompress = NO;
					break;
				case 'T':	/* truncate symbols to 8 characters */
					oldtruncate = YES;
					break;
				}
			}
			/* check the old and new option settings */
			if (oldcompress != compress || oldtruncate != truncate) {
				(void) fprintf(stderr, "cscope: -c or -T option mismatch between command line and old symbol database\n");
				goto force;
			}
			/* seek to the trailer */
			if (fscanf(oldrefs, "%d", &offset) != 1 ||
			    fseek(oldrefs, offset, 0) == -1) {
				(void) fprintf(stderr, "cscope: incorrect symbol database file format\n");
				goto force;
			}
		}
		/* get the cross-reference file's modification time */
		fstat(fileno(oldrefs), &statstruct);
		reftime = statstruct.st_mtime;

		/* if assuming that some files have changed */
		if (fileschanged == YES) {
			goto outofdate;
		}
		/* see if the directory lists are the same */
		if (samelist(oldrefs, srcdirs, nsrcdirs) == NO ||
		    samelist(oldrefs, incdirs, nincdirs) == NO ||
		    fscanf(oldrefs, "%d", &oldnum) != 1 ||	/* get the old number of files */
		    fileversion >= 9 && fscanf(oldrefs, "%*s") != 0) {	/* skip the string space size */
			goto outofdate;
		}
		/* see if the list of source files is the same and
		   none have been changed up to the included files */
		for (i = 0; i < nsrcfiles; ++i) {
			if (fscanf(oldrefs, "%s", oldname) != 1 ||
			    strnotequal(oldname, srcfiles[i]) ||
			    stat(srcfiles[i], &statstruct) != 0 ||
			    statstruct.st_mtime > reftime) {
				goto outofdate;
			}
		}
		/* the old cross-reference is up-to-date */
		/* so get the list of included files */
		while (i++ < oldnum && fscanf(oldrefs, "%s", oldname) == 1) {
			addsrcfile(basename(oldname), oldname);
		}
		(void) fclose(oldrefs);
		return;
		
	outofdate:
		/* if the database format has changed, rebuild it all */
		if (fileversion != FILEVERSION) {
			(void) fprintf(stderr, "cscope: converting to new symbol database file format\n");
			goto force;
		}
		/* reopen the old cross-reference file for fast scanning */
		if ((symrefs = open(reffile, O_RDONLY)) == -1) {
			(void) fprintf(stderr, "cscope: cannot open file %s\n", reffile);
			myexit(1);
		}
		/* get the first file name in the old cross-reference */
		readblock();		/* read the first cross-ref block */
		scanpast('\t');		/* skip the header */
		oldfile = getoldfile();
	}
	else {	/* force cross-referencing of all the source files */
	force:	reftime = 0;
		oldfile = NULL;
	}
	/* open the new cross-reference file */
	if ((newrefs = fopen(newreffile, "w")) == NULL) {
		(void) fprintf(stderr, "cscope: cannot open file %s\n", newreffile);
		myexit(1);
	}
	(void) fprintf(stderr, "cscope: building symbol database\n");
	putheader(newdir);
	fileversion = FILEVERSION;

	/* output the leading tab expected by crossref() */
	(void) putc('\t', newrefs);

	/* make passes through the source file list until the last level of
	   included files is processed */
	firstfile = 0;
	lastfile = nsrcfiles;
	for (;;) {

		/* get the next source file name */
		for (i = firstfile; i < lastfile; ++i) {
			
			/* report progress every 3 seconds */
			buildprogress(built, copied);
			
			/* if the old file has been deleted get the next one */
			while (oldfile != NULL && strcmp(srcfiles[i], oldfile) > 0) {
				oldfile = getoldfile();
			}
			/* if there isn't an old database or this is a new file */
			if (oldfile == NULL || strcmp(srcfiles[i], oldfile) < 0) {
				crossref(srcfiles[i]); /* crossref it */
				++built;
			}
			/* if this file was modified */
			else if (stat(srcfiles[i], &statstruct) == 0 &&
			    statstruct.st_mtime > reftime) {
				crossref(srcfiles[i]); /* crossref it */
				++built;
				
				/* skip its old crossref so modifying the last
				   source file does not cause all included files
				   to be built.  Unfortunately a new file that is
				   alphabetically last will cause all included
				   files to be build, but this is less likely */
				oldfile = getoldfile();
			}
			else {	/* copy its cross-reference */
				++copied;
				putfilename(srcfiles[i]);
				setmark('\t');
				cp = blockp;
				for (;;) {
					/* copy up to the next \t */
					do {	/* innermost loop optimized to only one test */
						while (*cp != '\t') {
							(void) putc(*cp++, newrefs);
						}
					} while (*++cp == '\0' && (cp = readblock()) != NULL);

					(void) putc('\t', newrefs);	/* copy the \t */
					
					/* get the next character */
					if (*(cp + 1) == '\0') {
						cp = readblock();
					}
					/* exit if at the end of this file's xref */
					if (cp == NULL || *cp == NEWFILE) {
						break;
					}
					/* look for an #included file */
					if (*cp == INCLUDE) {
						int	c, j;
						
						(void) putc(INCLUDE, newrefs);
						blockp = cp;
						skiprefchar();
						putstring(oldname);
						incfile(oldname + 1, *oldname);

						/* compress digraphs */
						for (j = 0; (c = (unsigned)oldname[j]) != '\0'; ++j) {
							if (dicode1[c] && dicode2[(unsigned)(oldname[j + 1])]) {
								c = (0200 - 2) + (unsigned)dicode1[c] + (unsigned)dicode2[(unsigned)(oldname[j + 1])];
								++j;
							}
							(void) putc(c, newrefs);
						}
						setmark('\t');
						cp = blockp;
					}
				}
				blockp = cp;
				oldfile = getoldfile();
			}
		}
		/* see if any included files were found */
		if (lastfile == nsrcfiles) {
			break;
		}
		firstfile = lastfile;
		lastfile = nsrcfiles;
	
		/* sort the included file names */
		qsort((char *) &srcfiles[firstfile], (unsigned) (lastfile - 
			firstfile), sizeof(char *), compare);
	}
	/* add a null file name to the trailing tab */
	(void) fprintf(newrefs, "%c\n", NEWFILE);

	/* get the file offset */
	offset = ftell(newrefs);
	
	/* output the source and include directory and file lists */
	putlist(srcdirs, nsrcdirs);
	putlist(incdirs, nincdirs);
	putlist(srcfiles, nsrcfiles);

	/* check for file system out of space */
	if (ferror(newrefs)) {
		(void) perror("cscope: write failed");
		(void) fclose(newrefs);
		(void) unlink(newreffile);
		myexit(1);
	}
	/* rewrite the header with the trailer offset */
	rewind(newrefs);
	putheader(newdir);
	(void) fclose(newrefs);
	
	/* close and remove the old cross-reference file */
	if (symrefs >= 0) {
		close(symrefs);
	}
	if (oldrefs != NULL) {
		(void) fclose(oldrefs);
		(void) unlink(reffile);
	}
	/* replace it with the new cross-reference file */
	if (link(newreffile, reffile) == -1) {
		(void) fprintf(stderr, "cscope: cannot link file %s to file %s\n", 
			newreffile, reffile);
		(void) perror("cscope");
		myexit(1);
	}
	if (unlink(newreffile) == -1) {
		(void) fprintf(stderr, "cscope: cannot unlink file %s\n", newreffile);
		(void) perror("cscope");
		errorsfound = YES;
	}
}
	
/* string comparison function for qsort */

int
compare(s1, s2)
char	**s1;
char	**s2;
{
	return(strcmp(*s1, *s2));
}

/* display search progress */

void
buildprogress(built, copied)
int	built, copied;
{
	static	long	start;
	long	now;
	long	time();

	/* save the start time */
	if (built == 0 && copied == 0) {
		start = time((long *) NULL);
	}
	/* display the progress every 3 seconds */
	else if ((now = time((long *) NULL)) - start >= 3) {
		start = now;
		(void) fprintf(stderr, "cscope: %d files built, %d files copied\n", 
			built, copied);
	}
}

/* get the next file name in the old cross-reference */

char *
getoldfile()
{
	static	char	file[PATHLEN + 1];	/* file name in old crossref */

	if (blockp != NULL) {
		do {
			if (*blockp == NEWFILE) {
				skiprefchar();
				putstring(file);
				if (file[0] != '\0') {	/* if not end-of-crossref */
					return(file);
				}
				return(NULL);
			}
		} while (scanpast('\t') != NULL);
	}
	return(NULL);
}

/* output the cscope version, current directory, -c option, and space for the
   file trailer offset */

void
putheader(dir)
char	*dir;
{
	(void) fprintf(newrefs, "cscope %d %s", FILEVERSION, dir);
	if (compress == NO) {
		(void) fprintf(newrefs, " -c");
	}
	if (truncate == YES) {
		(void) fprintf(newrefs, " -T");
	}
	(void) fprintf(newrefs, " %.10ld\n", offset);
}

/* put the name list into the cross-reference file */

void
putlist(names, count)
register char	**names;
register int	count;
{
	register int	i, size = 0;
	
	(void) fprintf(newrefs, "%d\n", count);
	if (names == srcfiles) {

		/* calculate the string space needed */
		for (i = 0; i < count; ++i) {
			size += strlen(names[i]) + 1;
		}
		(void) fprintf(newrefs, "%d\n", size);
	}
	for (i = 0; i < count; ++i) {
		(void) fputs(names[i], newrefs);
		(void) putc('\n', newrefs);
	}
}

/* see if the name list is the same in the cross-reference file */

BOOL
samelist(oldrefs, names, count)
register FILE	*oldrefs;
register char	**names;
register int	count;
{
	char	oldname[PATHLEN + 1];	/* name in old cross-reference */
	int	oldcount;
	register int	i;

	/* see if the number of names is the same */
	if (fscanf(oldrefs, "%d", &oldcount) != 1 ||
	    oldcount != count) {
		return(NO);
	}
	/* see if the name list is the same */
	for (i = 0; i < count; ++i) {
		if (fscanf(oldrefs, "%s", oldname) != 1 ||
		    strnotequal(oldname, names[i])) {
			return(NO);
		}
	}
	return(YES);
}

/* skip the list in the cross-reference file */

void
skiplist(oldrefs)
register FILE	*oldrefs;
{
	int	i;
	
	if (fscanf(oldrefs, "%d", &i) != 1) {
		(void) fprintf(stderr, "cscope: cannot read list size from file %s\n", reffile);
		exit(1);
	}
	while (--i >= 0) {
		if (fscanf(oldrefs, "%*s") != 0) {
			(void) fprintf(stderr, "cscope: cannot read list name from file %s\n", reffile);
			exit(1);
		}
	}
}

/* cleanup and exit */

void
myexit(sig)
int	sig;
{
	/* remove any temporary files */
	if (temp1[0] != '\0') {
		(void) unlink(temp1);
		(void) unlink(temp2);
	}
	/* restore the terminal to its original mode */
	if (incurses == YES) {
#if BSD || UNIXPC
		clear();
		refresh();
#endif
		endwin();
		mousecleanup();
	}
	/* dump core for debugging on the quit signal */
	if (sig == SIGQUIT) {
		abort();
	}
	exit(sig);
}
