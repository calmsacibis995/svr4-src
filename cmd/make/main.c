/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)make:main.c	1.15.1.1"
/*	@(#)make:main.c	1.8 of 5/1/89	*/

#include "defs"
#include <signal.h>
#include <ccstypes.h>
#include <errno.h>

/* NAME
**	make/build - maintain, update and regenerate groups of programs.
**
** OPTIONS for make and build
** 	'-f'  The description file is the next argument;
** 	     	(makefile is the default)
** 	'-p'  Print out a complete set of macro definitions and target
**		descriptions
** 	'-i'  Ignore error codes returned by invoked commands
** 	'-S'  stop after any command fails (normally do parallel work)
** 	'-s'  Silent mode.  Do not print command lines before invoking
**	'-n'  No execute mode.  Print commands, but do not execute them.
** 	'-t'  Touch the target files but don't issue command
** 	'-q'  Question.  Check if object is up-to-date;
** 	      	returns exit code 0 if up-to-date, -1 if not
** 	'-u'  unconditional flag.  Ignore timestamps.
**	'-m'  Print memory.  No-op if no getu system call
**	'-d'  Debug.  No-op unless compiled with -DMKDEBUG
** Additional OPTIONS for build
** 	'-v'  the next argument is the viewpath string
** 	'-o'  track the files linked or copied
*/

#define TURNOFF(a)	(Mflags &= ~(a))
#define TURNON(a)	(Mflags |= (a))


static char	makefile[] = "makefile",
	Makefile[] = "Makefile",
	Makeflags[] = "MAKEFLAGS",
	RELEASE[] = "RELEASE";

char	Nullstr[] = "",
	funny[ARY256];

extern CHARSTAR builtin[];
extern errno;
CHARSTAR *linesptr = builtin;

FILE *fin;

struct nameblock pspace;
NAMEBLOCK curpname = &pspace,
	mainname,
	firstname;

LINEBLOCK sufflist;
VARBLOCK firstvar;
PATTERN firstpat ;
OPENDIR firstod;

int	ndir = 0;		/* number of directories in viewpath*/
char	viewpath[MAXVPATH],	/* viewpaths live here, from env or command */
	*directs[MAXDIR+1];	/* pointers to all directory levels */

CHAIN	lnkd_files,		/* list of files linked to directory */
	last_lnkdfs;		/* backward pointer of above */
int	lib_cpd = NO;		/* library copied into the node ? */

void	(*intstat) (),
	(*quitstat) (),
	(*hupstat) (),
	(*termstat) ();

/*
**	Declare local functions and make LINT happy.
*/

static int	rddescf();
static int	rdd1();
static void	printdesc();
static void	prname();
static void	getmflgs();
static void	setflags();
static void	optswitch();
static void	usage();
static void	setmflgs();
static void	readenv();
static int	eqsign();
static void	callyacc();

int	waitpid = 0;

#ifdef pdp11			/* pdp11: 16 bit int/32 bit long */
long	Mflags = MH_DEP;	/* initialize the make flags */
#else
int	Mflags = MH_DEP;
#endif

int	BLDCALL = NO,		/* Was object invoked as build(YES) or make(NO)? */
	okdel = YES;


main(argc, argv)
int	argc;
CHARSTAR argv[];
{
#ifdef unix
	void	intrupt();
#endif
	register NAMEBLOCK p;
	register CHARSTAR s;
	register int i;
	time_t	tjunk;
	char tempvpath[MAXVPATH];	/* pathname to  .cms file built here */
	char	*getcwd(), *getenv(), *sname(),
		*cdir;
	void	fatal(), fatal1(), setvar(), enbint();
	int	dummy, nfargs, chdir(), doname(), isdir(),
		descset = 0,
		vpthset = 0;

#ifdef METERFILE
	meter(METERFILE);
#endif
	for (s = "#|=^();&<>*?[]:$`'\"\\\n" ; *s ; ++s)
		funny[*s] |= META;
	for (s = "\n\t :=;{}&>|" ; *s ; ++s)
		funny[*s] |= TERMINAL;
	funny['\0'] |= TERMINAL;

	TURNON(INTRULE);	/* Default internal rules, turned on */

	init_lex();

  if (STREQ(strpbrk(sname(argv[0]), "b"), "build")) {
		BLDCALL = YES;

		for (i = 1; i < argc; i++) 
			if ( argv[i] && (argv[i][0] == MINUS) &&
		    	(argv[i][1] == 'v') && (argv[i][2] == CNULL)) {
				/*	
			 	*	'-v <viewpath>' option
			 	*/
			 	char *tptr;
				argv[i] = 0;
				argv[i] = 0;
				if ( i >= argc - 1 || argv[i+1] == 0)
					fatal("no viewpath argument after -v flag (bu2)");
				if ( strlen(argv[i+1]) >= (unsigned int) (MAXVPATH - 1) )
v_error:			fatal("viewpath name too long for array (bu4)");
				(void) cat(&viewpath[0], argv[i+1], 0);
				tptr = (char *)malloc(strlen(viewpath)+sizeof("VPATH="));
				(void) cat(tptr, "VPATH=", viewpath, 0);
				if (putenv(tptr)==0)
					;
				else
					fatal("putenv couldn't malloc (bu4)");
				++vpthset;
				argv[i+1] = 0;
			}


		if(!vpthset){
			/* set BLDCALL if $HOME/.cms exists */
			(void) cat(tempvpath, getenv("HOME"), "/.cms", 0);
		}
			
	}

	builtin[1] = "MAKE=make";

	getmflgs();			/* Init $(MAKEFLAGS) variable */
	setflags(argc, argv);		/* Set command line flags */
	setvar("$", "$");

	/*	Read command line "=" type args and make them readonly.	*/

	TURNON(INARGS | EXPORT);
#ifdef MKDEBUG
	if (IS_ON(DBUG)) 
		printf("Reading \"=\" args on command line.\n");
#endif
	for (i = 1; i < argc; ++i)
		if ( argv[i] && argv[i][0] != MINUS &&
		     eqsign(argv[i]) )
			argv[i] = 0;

	TURNOFF(INARGS | EXPORT);

	if (IS_ON(INTRULE)) {	/* Read internal definitions and rules. */
#ifdef MKDEBUG
		if (IS_ON(DBUG))
			printf("Reading internal rules.\n");
#endif
		(void)rdd1((FILE *) NULL);
	}
	TURNOFF(INTRULE);	/* Done with internal rules, now. */

	/* Read environment args.
	 * Let file args which follow override, unless 'e'
	 * in MAKEFLAGS variable is set.
	 */
	if (ANY((varptr(Makeflags))->varval.charstar, 'e') )
		TURNON(ENVOVER);
#ifdef MKDEBUG
	if (IS_ON(DBUG))
		printf("Reading environment.\n");
#endif
	TURNON(EXPORT);
	readenv();
	TURNOFF(EXPORT | ENVOVER);

	if (BLDCALL) {
		/* construct the list of directories for the viewpath 
		 * using -v flag arg, VPATH environment variable, or .cms file
		 * (in that order).
		 */
		char 	*vptr,		/* viewpath string pointer */
			*cptr, *dptr,	/* temp pointers used for viewpaths */
			*dirname;

		if (!vpthset) {	/* -v flag used already? */
			if ((vptr = getenv("VPATH")) &&
			    !(STREQ(vptr, ""))) {
					if ( strlen(vptr) >= (unsigned int) (MAXVPATH - 1))
						goto v_error;
				(void) cat(viewpath, vptr, 0);
				vptr = viewpath;
				vpthset++;
			} else {
				FILE	*fid;

				CHKARY(main, tempvpath, MAXVPATH)  /*macro defined in defs*/
				if ( fid = fopen(tempvpath, "r") ) {
					(void)fgets(viewpath, MAXVPATH, fid);
					(void)fclose(fid);
					if (*viewpath)
						vpthset++;
				}
			}
		}
		if (vpthset) {
			if ( strlen(viewpath) >= (unsigned int) (MAXVPATH - 1))
				goto v_error;
#ifdef MKDEBUG
			if (IS_ON(DBUG)) 
				fprintf(stdout, "viewpath %s\n", viewpath);
#endif
			vptr = viewpath;
			while (*vptr) {
				if (++ndir > MAXDIR)
					fatal1("more than %d nodes in viewpath", MAXDIR );
				directs[ndir] = vptr;
				while (*vptr && (*++vptr != ':'))
					if (*vptr == '\n') 
						*vptr = '\0';
				if (*vptr) 
					*vptr++ = '\0';
			}
			if ( !(cdir = getcwd((char *)NULL, MAXPATHLEN)) )
c_error:			if(errno == ERANGE)
					fatal("Pathname too long: getcwd (bu5)");
				else
					fatal("getcwd (bu5)");
			for ( i = ndir ; i >= 1 ; i--) { /* check viewpaths */
				if (chdir(directs[i]))
					fatal1("viewpath contains non-existent directory:<%s> (bu6)",
						directs[i]);
			}
			if ( !(dirname = getcwd((char *)NULL, MAXPATHLEN)) ) goto c_error;
			(void) chdir(cdir);
			cptr = cdir;
			dptr = dirname;
			while ((*dptr) && (*cptr) && (*cptr == *dptr)){cptr++;dptr++;}
			if (*dptr || (*cptr && *cptr != '/'))
				fatal("improper viewpath specification (bu44)");
			for (i = 1; i <= ndir; i++) {
				while (chdir(directs[i]) && (i <= ndir)) {
					register int tj, ti = i;

					for (tj = ti + 1; tj <= ndir; ) directs[ti++] = directs[tj++];
					ndir--;
				}
				if (i > ndir)
					break;
				if (*directs[i] != '/') {
					if (!(dirname = getcwd(NULL, MAXPATHLEN)))
						goto c_error;
					directs[i] = dirname;
				}
				(void) chdir(cdir);
				(void) cat(tempvpath, directs[i], cptr, 0);
				CHKARY(main, tempvpath, MAXPATHLEN)  /*macro defined in defs*/
				directs[i] = copys(compath(tempvpath));
#ifdef MKDEBUG
				if ( IS_ON(DBUG) ) 
					fprintf(stdout, "directory %d = %s\n",
							i, directs[i]);
#endif
			}
		}
	}

	if (!(BLDCALL &&	/* invoked as 'make' or */
	      vpthset)) {	/*  no viewpath set */
		if ( !(cdir = getcwd((char *)NULL, MAXPATHLEN)) )
			goto c_error;
		directs[++ndir] = copys(cdir);
	}

	/*	Get description file in the following order:
	**	 - command line '-f' parameters
	**	 - default names (makefile, Makefile, s.makefile, s.Makefile)
	*/
	for (i = 1; i < argc; i++)
		if ( argv[i] && (argv[i][0] == MINUS) &&
		    (argv[i][1] == 'f') && (argv[i][2] == CNULL)) {
			argv[i] = 0;
			if (i >= argc - 1 || argv[i+1] == 0)
				fatal("no description file after -f flag (bu1)");
			if ( rddescf(argv[++i], YES) )
				fatal1("cannot open %s", argv[i]);
			argv[i] = 0;
			++descset;
		}
	if ( !descset )
#ifdef unix
		if ( rddescf(makefile, NO))
			if ( rddescf(Makefile, NO) )
				if ( rddescf(makefile, YES))
					(void)rddescf(Makefile, YES);
#endif
#ifdef gcos
		(void)rddescf(makefile, NO);
#endif

	if (IS_ON(PRTR)) 
		printdesc(NO);

	if ( SRCHNAME(".IGNORE") ) 
		TURNON(IGNERR);
	if ( SRCHNAME(".SILENT") ) 
		TURNON(SIL);
	if (p = SRCHNAME(".SUFFIXES")) 
		sufflist = p->linep;
	if ( !sufflist ) 
		fprintf(stderr, "No suffix list.\n");

#ifdef unix
	intstat = signal(SIGINT, SIG_IGN);
	quitstat = signal(SIGQUIT, SIG_IGN);
	hupstat = signal(SIGHUP, SIG_IGN);
	termstat = signal(SIGTERM, SIG_IGN);
	enbint(intrupt);
#endif

	nfargs = 0;

	for (i = 1; i < argc; ++i)
		if ( s = argv[i] ) {
			if ( !(p = SRCHNAME(s)) )
				p = makename(s);

			++nfargs;

			/* another entry in list of files linked/copied */

			last_lnkdfs = lnkd_files = ALLOC(chain);
			lnkd_files->nextchain = NULL;
			(void)doname(p, 0, &tjunk, &dummy);
#ifdef MKDEBUG
			if (IS_ON(DBUG)) 
				printdesc(YES);
#endif
		}

	/*
	 * If no file arguments have been encountered, make the first
	 * name encountered that doesn't start with a dot
	 */
	if ( !nfargs )
		if ( !mainname )
			fatal("no arguments or description file (bu7)");
		else {
			last_lnkdfs = lnkd_files = ALLOC(chain);
			lnkd_files->nextchain = NULL;
			(void)doname(mainname, 0, &tjunk, &dummy);
#ifdef MKDEBUG
			if (IS_ON(DBUG)) 
				printdesc(YES);
#endif
		}

	mkexit(0);	/* make succeeded; no fatal errors */
	/*NOTREACHED*/
}



#ifdef unix

void
intrupt()
{
	if (! BLDCALL) {
		time_t	exists();
		int	isprecious(), member_ar(), unlink();
		void	mkexit();
		int	lev, ret_isdir;
		CHARSTAR p;

		NAMEBLOCK lookup_name();

		if (okdel && IS_OFF(NOEX) && IS_OFF(TOUCH) && 
		    (p = (varptr("@")->varval.charstar)) &&
		    (exists( lookup_name(p), &lev) != -1) && 
		    (!isprecious(p)) &&
		    (!member_ar(p)))	/* don't remove archives generated during make */
		{
			if ((ret_isdir = isdir(p)) == 1)
				fprintf(stderr, "\n*** %s NOT removed.\n", p);
			else if ( !( ( ret_isdir == 2 ) || unlink(p) ) )
				fprintf(stderr, "\n*** %s removed.\n", p);
		}
	}
	fprintf(stderr, "\n");
	mkexit(2);
	/*NOTREACHED*/
}


void
enbint(onintr)
void (*onintr)();
{
	if (intstat == SIG_DFL)
		(void)signal(SIGINT, onintr);
	if (quitstat == SIG_DFL)
		(void)signal(SIGQUIT, onintr);
	if (hupstat == SIG_DFL)
		(void)signal(SIGHUP, onintr);
	if (termstat == SIG_DFL)
		(void)signal(SIGTERM, onintr);
}

#endif



static int
rddescf(descfile, sflg)		/* read and parse description file */
CHARSTAR descfile;
int	sflg;		/* if YES try s.descfile */
{
	FILE *k, *opdescf();

	if (STREQ(descfile, "-"))
		return( rdd1(stdin) );

retry:	if (k = opdescf(descfile)) {
#ifdef MKDEBUG
		if (IS_ON(DBUG))
			printf("Reading %s\n", descfile);
#endif
		return( rdd1(k) );
	}

	if ( !sflg || !vpget(descfile, CD, varptr(RELEASE)->varval.charstar) )
		return(1);
	sflg = NO;
	goto retry;
}



FILE *opdescf(desfnam)		/* search viewpath for description file (desfnam) */
char *desfnam;
{
	register int	vdir;	/* loop index for viewpath */
	char	descf[MAXPATHLEN];	/* buffer for full path name of description file */
	FILE 	*k;		/* file des for description file*/

	if (*desfnam == SLASH)
		return(fopen(desfnam, "r"));

	for (vdir = 1; vdir <= ndir; vdir++) {
		/* create the pathname */
		(void) cat(descf, directs[vdir], "/", desfnam, 0);
		CHKARY(opdescf, descf, MAXPATHLEN)  /*macro defined in defs*/
		(void)compath(descf);

		if ( k = fopen(descf, "r") )
			return(k);		/* success-- found it */
	}
	return(NULL);
}


/**	used by yyparse		**/
extern int	yylineno;
extern CHARSTAR zznextc;


static int
rdd1(k)
FILE *k;
{
	int yyparse();
	void	fatal();

	fin = k;
	yylineno = 0;
	zznextc = 0;

	if ( yyparse() )
		fatal("description file error (bu9)");

	if ( ! (fin == NULL || fin == stdin) )
		(void)fclose(fin);

	return(0);
}


static void
printdesc(prntflag)
int	prntflag;
{
	register NAMEBLOCK p;
	register VARBLOCK vp;
	register CHAIN pch;
#ifdef unix
	if (prntflag) {
		register OPENDIR od;
		fprintf(stderr, "Open directories:\n");
		for (od = firstod; od; od = od->nextopendir)
			fprintf(stderr, "\t%s\n", od->dirn);
	}
#endif
	if (firstvar) 
		fprintf(stderr, "Macros:\n");
	for (vp = firstvar; vp; vp = vp->nextvar)
		if ( !(vp->v_aflg) )
			printf("%s = %s\n", vp->varname,
				((vp->varval.charstar) == NULL? " ":vp->varval.charstar));
		else {
			fprintf(stderr, "Lookup chain: %s\n\t", vp->varname);
			for (pch = (vp->varval.chain); pch; pch = pch->nextchain)
				fprintf(stderr, " %s", (pch->datap.nameblock)->namep);
			fprintf(stderr, "\n");
		}

	for (p = firstname; p; p = p->nextname)
		prname(p, prntflag);
	printf("\n");
	(void)fflush(stdout);
}


static void
prname(p, prntflag)
register NAMEBLOCK p;
{
	register DEPBLOCK dp;
	register SHBLOCK sp;
	register LINEBLOCK lp;

	if (p->linep)
		printf("\n%s:", p->namep);
	else
		fprintf(stderr, "\n%s:", p->namep);

	if (prntflag)
		fprintf(stderr, "  done=%d", p->done);

	if (p == mainname) 
		fprintf(stderr, " (MAIN NAME)");

	for (lp = p->linep; lp; lp = lp->nextline) {
		if ( dp = lp->depp ) {
			fprintf(stderr, "\n depends on:");
			for (; dp; dp = dp->nextdep)
				if ( dp->depname) {
					printf(" %s", dp->depname->namep);
					printf(" ");
				}
		}
		if (sp = lp->shp) {
			printf("\n");
			fprintf(stderr, " commands:\n");
			for ( ; sp; sp = sp->nextsh)
				printf("\t%s\n", sp->shbp);
		}
	}
}


static void
getmflgs()		/* export command line flags for future invocation */
{
	void	setvar();
	int	sindex();
	register CHARSTAR *pe, p;
	register VARBLOCK vpr = varptr(Makeflags);

	setvar(Makeflags, "ZZZZZZZZZZZZZZZZ");
	vpr->varval.charstar[0] = CNULL;
	vpr->envflg = YES;
	vpr->noreset = YES;
	/*optswitch('b');*/
	for (pe = environ; *pe; pe++)
		if ( !sindex(*pe, "MAKEFLAGS=") ) {
			for (p = (*pe) + sizeof Makeflags; *p; p++)
				optswitch(*p);
			return;
		}
}


static void
setflags(ac, av)
register int	ac;
CHARSTAR *av;
{
	register int	i, j;
	register char	c;
	int	flflg = 0, 		/* flag to note '-f' option. */
		viewflg = 0;		/* flag to note '-v' option  */

	for (i = 1; i < ac; ++i) {
		if (flflg || viewflg) {
			viewflg = flflg = 0;
			continue;		}
		if (av[i] && av[i][0] == MINUS) {
			if (ANY(av[i], 'f'))
				flflg++;

			if (ANY(av[i], 'v') )
				viewflg++;

			for (j = 1 ; (c = av[i][j]) != CNULL ; ++j)
				optswitch(c);

			if (flflg)
				av[i] = "-f";
			else if (viewflg)
				av[i] = "-v";
			else
				av[i] = 0;
		}
	}
}



static void
optswitch(c)	/* Handle a single character option */
char	c;
{

	switch (c) {

	case 's':	/* silent flag */
		TURNON(SIL);
		setmflgs(c);
		return;

	case 'n':	/* do not exec any commands, just print */
		TURNON(NOEX);
		setmflgs(c);
		return;

	case 'e':	/* environment override flag */
		setmflgs(c);
		return;

	case 'p':	/* print description */
		TURNON(PRTR);
		return;

	case 'i':	/* ignore errors */
		TURNON(IGNERR);
		setmflgs(c);
		return;

	case 'S':
		TURNOFF(KEEPGO);
		setmflgs(c);
		return;

	case 'k':
		TURNON(KEEPGO);
		setmflgs(c);
		return;

	case 'r':	/* turn off internal rules */
		TURNOFF(INTRULE);
		return;

	case 't':	/* touch flag */
		TURNON(TOUCH);
		setmflgs(c);
		return;

	case 'q':	/* question flag */
		TURNON(QUEST);
		setmflgs(c);
		return;

	case 'g':	/* turn default $(GET) of files not found */
		TURNON(GET);
		setmflgs(c);
		return;

	case 'b':	/* use MH version of test for whether cmd exists */
		TURNON(MH_DEP);
		setmflgs(c);
		return;

	case 'B':	/* turn off -b flag */
		TURNOFF(MH_DEP);
		setmflgs(c);
		return;

	case 'd':	/* debug flag */
#ifdef MKDEBUG
		TURNON(DBUG);
		setmflgs(c);
#endif
		return;

	case 'm':	/* print memory map */
#ifdef GETU
		TURNON(MEMMAP);
		setmflgs(c);
#endif
		return;

	case 'f':	/* named makefile: handled by setflags() */
		return;

	case 'u':	/* unconditional build indicator */
		TURNON(UCBLD);
		setmflgs(c);
		return;
	}

	if(BLDCALL)
		switch(c) {

		case 'v':	/* Command line viewpath. */
			return;

		case 'o':	/* object tracking */
			TURNON(TRACK);
			setmflgs(c);
			return;
		}

	usage(c);
}


static void
usage(c)
char c;
{
	if (BLDCALL)
    fprintf(stderr, "build");
  else
		fprintf(stderr, "make");
	fprintf(stderr,": illegal option -- %c\nusage: ", c);
  if (BLDCALL)
    fprintf(stderr, "build [-v viewpath] [-o]\n\t");
  else
		fprintf(stderr, "make ");
	fprintf(stderr, "[-f makefile] [-p] [-i] [-k] [-s] [-r] [-n] [-u]\n\t");
#ifdef GETU
	fprintf(stderr, "[-m] ");
#endif
#ifdef MKDEBUG
	fprintf(stderr, "[-d] ");
#endif
	fprintf(stderr, "[-b] [-e] [-t] [-q] [names]\n");
#ifdef unix
	mkexit(1);
#endif
#ifdef gcos
	mkexit(0);
#endif
}


static void
setmflgs(c)		/* set up the cmd line input flags for EXPORT. */
register char	c;
{
	register CHARSTAR p = (varptr(Makeflags))->varval.charstar;
	for (; *p; p++)
		if (*p == c)
			return;
	*p++ = c;
	*p = CNULL;
}


/*
 *	If a string like "CC=" occurs then CC is not put in environment.
 *	This is because there is no good way to remove a variable
 *	from the environment within the shell.
 */
static void
readenv()
{
	register CHARSTAR *ea, p;

	ea = environ;
	for (; *ea; ea++) {
		for (p = *ea; *p && *p != EQUALS; p++)
			;
		if ((*p == EQUALS) && *(p + 1))
			(void)eqsign(*ea);
	}
}


static int
eqsign(a)
register CHARSTAR a;
{
	register CHARSTAR p;

	for (p = ":;=$\n\t"; *p; p++)
		if (ANY(a, *p)) {
			callyacc(a);
			return(YES);
		}
	return(NO);
}


static void
callyacc(str)
register CHARSTAR str;
{
	CHARSTAR lines[2];
	FILE 	*finsave = fin;
	CHARSTAR *lpsave = linesptr;
	fin = 0;
	lines[0] = str;
	lines[1] = 0;
	linesptr = lines;
	(void)yyparse();
	fin = finsave;
	linesptr = lpsave;
}

NAMEBLOCK
lookup_name(namep)
CHARSTAR namep;
{
	NAMEBLOCK p;
	for (p = firstname; p; p = p->nextname) {
		if (STREQ(namep, p->namep)) 
			return (p);
	}
	return ( NULL );
}
