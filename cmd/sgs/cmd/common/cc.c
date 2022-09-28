/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-cmd:common/cc.c	1.139.5.1"

#define	PARGS	if(debug){(void)printf("%scc: ", prefix);for(j=0;j<nlist[AV];j++)(void)printf(" '%s'",list[AV][j]);(void)printf("\n");}

/*===================================================================*/
/*                                                                   */
/*                 CC Command                                        */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/*     The cc command parses the command line to set up command      */
/*     lines for and exec whatever processes (preprocessor,          */
/*     compiler, optimizer, assembler, link editor) are required.    */
/*                                                                   */
/*===================================================================*/


#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<ccstypes.h>
#ifdef __STDC__
#include	<stdlib.h>
#endif
#include	"paths.h"
#include	"sgs.h"

/* performance statistics */

#ifdef PERF

#define	STATS( s )	if (stat == 1) {			\
				stats[ii].module = s;		\
				stats[ii].ttim = ttime;		\
				stats[ii++].perform = ptimes;	\
				if (ii > 25)			\
					pexit();		\
			}

static	FILE	*perfile;
static	long	ttime;

struct	tbuffer {
	long	proc_user_time;
	long	proc_system_time;
	long	child_user_time;
	long	child_system_time;
	} ptimes;

extern	long	times();

struct	perfstat {
	char	*module;
	long	ttim;
	struct	tbuffer	perform;
	} stats[30];

static	int	ii = 0;
static	int	stat = 0;

#endif

#define cunlink(x)	if (x)	(void)unlink(x);

/* tool names */

#define CRT0	"crt0.o"
#define MCRT0	"mcrt0.o"
#define PCRT0	"pcrt0.o"
#if defined(M32) || defined(i386)
#define MERGE 0
#define CRT1	"crt1.o"
#define CRTI	"crti.o"
#define VALUES	"values"
#define MCRT1	"mcrt1.o"
#define CRTN	"crtn.o"
#endif

#if defined(M32) || defined(i386)
#define PCRT1   "pcrt1.o"
#define PCRTI   "pcrti.o"
#define PCRTN	"pcrtn.o"
#endif

#if defined (M32) || defined(i386)
#ifndef MERGE
#define N_CPP   "cpp"
#endif
#define N_C0    "acomp"
#else
#define N_CPP   "cpp"
#define N_C0    "comp"
#endif


#define	N_OPTIM	"optim"

#define N_PROF	"basicblk"
#define	N_AS	"as"
#define	N_LD	"ld"


/* number of args always passed to ld; used to determine whether
 * or not to run ld when no files are given */

#define	MINLDARGS	3


/* list indexes */

#ifndef MERGE
#define	Xcp	0	/* index of list containing options to the compiler */
#else
#define	Xcp	1	/* index is the same if the compiler and cpp are merged */
#endif
#define	Xc0	1	/* index of list containing options to the compiler */
#define	Xc2	2	/* index of list containing options to the optimizer */
#define Xbb	3	/* index of list containing options to basicblk */
#define	Xas	4	/* index of list containing options to the assembler */
#define	Xld	5	/* index of list containing options to ld */
#define	AV	6	/* index of the argv list to be supplied to execvp */
#define	CF	7	/* index of list containing the names of files to be 
				compiled and assembled */

#define	NLIST	8	/* total number of lists */

/* option string for getopt() */





#if defined(M32) 
#define OPTSTR "A:B:Ccd:D:e:EfFgGh:HI:J:K:l:L:o:OpPq:Q:StT:u:U:vVW:X:Y:z:#"
#endif

#ifdef i386
#define OPTSTR  "cpq:fOSW:EPgGo:D:I:U:K:B:CHd:Vl:u:L:Y:A:vX:Q:Z:z:#"
#endif

/* file names */

static char	
#ifndef MERGE
	*cpp_out,	/* output of preprocessor */
#endif
	*c_out,		/* compiler output */
	*as_in,		/* assembler input */
	*as_out,	/* assembler output */
	*ld_out = NULL,	/* link editor output (object file) */
#ifndef MERGE
	*tmp1 = NULL,	/* temporaries */
#endif
	*tmp2 = NULL,
	*tmp4 = NULL,
	*tmp5 = NULL;


/* path names of the various tools and directories */

static char	
#ifndef MERGE
	*passcp = NULL,
#endif
	*passc0 = NULL,
	*passc2 = NULL,
	*passprof = NULL,
	*passas = NULL,
	*passld = NULL,
	*crtdir	= NULL,
	*libpath = NULL,
	*libdir = NULL,
	*llibdir = NULL;

#if defined(M32)
static	 char	*fpdir	= NULL,
	*fplibdir = NULL;
#endif

static	char	*profdir= LIBDIR;


/* flags: ?flag corresponds to ? option */
static int
	cflag	= 0,	/* compile and assemble only to .o; no load */
	Oflag	= 0,	/* optimizer request */
	Sflag	= 0,	/* leave assembly output in .s file */
	Eflag	= 0,	/* preprocess only, output to stdout */
	Pflag	= 0,	/* preprocess only, output to file.i */
	eflag	= 0,	/* error count */
	lflag	= 0,	/* turned on if -l option is supplied */
	dsflag	= 1,	/* turn off symbol attribute output in compiler */
	dlflag	= 1,	/* turn of line number output in compiler */
	pflag	= 0,	/* profile request for standard profiler */
	qpflag  = 0,    /* if = 1, then -p. if = 2, then -qp. else none */
	qarg	= 0,	/* profile request for xprof or lprof */
	gflag	= 0,	/* include libg.a on load line */
#if defined(M32) || defined(i386)
	Qflag   = 1,    /* turn on version stamping - turn off if -Qn 
				is specified*/
	Xwarn	= 0,	/* warn user if more than one -X options were 
				given with differing arguments */	
	Gflag	= 0,	/* == 0 include start up routines else don't */
#endif
	debug	= 0,	/* cc command debug flag (prints command lines) */
	Kabi	= 0;	/* 1 if Kminabii is given */

#if defined(M32)
static	char	*Karg	= NULL;	/* will hold the -K flag and argument (if -K is 					given) */

#if defined(SRC_PROD)
static	int	 K1arg  = 1; /* 1 = fpe mode (-Kfpe); 2 = fp mode (-K mau) */
#else
static	int	 K1arg  = 2; /* mau is default in binary product */
#endif

static	char	*Jarg   = NULL;  /* will hold the argument to -J when given */
#endif

#if defined(M32) || defined(i386)
static char	*Xarg	= NULL; /* will hold the -X flag and argument 
				  Xt is default for this release */
static  char    *Parg   = NULL; /* will hold the -KPIC or -Kpic argument */
static	char	*earg	= NULL; /* will hold the -e flag and argument */
static	char	*harg	= NULL; /* will hold the -h flag and argument */
#endif

static	char	*nopt;		/* current -W flag option */

/* lists */

static	char	**list[NLIST];	/* list of lists */
static	int
	nlist[NLIST],	/* current index for each list */
	limit[NLIST],	/* length of each list */
	nxo	= 0,	/* number of .o files in list[Xld] */
	nxn	= 0,	/* number of non-.o files in list[Xld] */
	argcount;	/* initial length of each list */


#if defined(M32) || defined(i386)
static	char	*crt	= CRT1;
#else
static	char    *crt    = CRT0;         /* name of run-time startoff */
#endif

static	char	*prefix;

#if defined(M32) || defined(i386)
static	char 	*values	= VALUES;	/* name of the values-X[cat].o file */
#endif

#if defined(M32)
static	char	*Jlibpath = NULL; 	/* path to libs if -YJ */
#endif

static	int	sfile;			/* indicates current file in list[CF] is .s */

/* functions */

extern  int     optind,         /* arg list index */
                opterr,         /* turn off error message */
                optopt,         /* current option char */
                access(),
                unlink(),
		fork(),
		execvp(),
		wait(),
		creat(),
                getopt();

extern  void    exit();

extern  char    *getenv();
extern  int     putenv();

extern  char    *optarg;        /* current option argument */

#ifdef __STDC__
static	void	*stralloc();
#else
static	char	*stralloc(); 
#endif

static	char
	getsuf(),
	*setsuf(),
	*copy(),
	*getpref(),
	*makename(),
	*passname();

static void
	idexit(),
	linkedit(),
	mk_defpaths(),
	chg_pathnames(),
	addopt(),
	error(),
	mktemps(), 
	process_lib_path(),
	dexit();

static int
	callsys(),
	nodup(),
	getXpass(),
	compile(),
	optimize(),
	profile(),
	assemble(),
	preslash(),
	sufsa(),
	move();

main (argc, argv)
	int argc;
	char *argv[];
{
	int	c;		/* current option char from getopt() */
	char	*t, *s, *p;	/* char ptr temporary */
	int	done;		/* used to indicate end of arguments passed by -W */
	int	i;		/* loop counter, array index */
	char 	*chpiece = NULL,	/* list of pieces whose default location has
					   been changed by the -Y option */
		*npath = NULL;	/* new location for pieces changed by -Y option */
	char	*optstr = OPTSTR;

	opterr = 0;
#ifdef PERF
	if ((perfile = fopen("cc.perf.info", "r")) != NULL) {
		fclose(perfile);
		stat = 1;
	}
#endif

	prefix = getpref( argv[0] );

	/* initialize the lists */
	argcount= argc + 6;
	c = sizeof(char *) * argcount;
	for (i = 0; i < NLIST; i++) {
		nlist[i] = 0;
		list[i] = (char **)stralloc((unsigned)c);
		limit[i]= argcount;
	}

	setbuf(stdout, (char *) NULL);	/* unbuffered output */

	while (optind < argc) {
		c = getopt(argc, argv, optstr);
		switch (c) {

		case 'c':       /* produce .o files only */
                        cflag++;
                        break;

		case 'C':       /* tell compiler to leave comments in (lint) */
                        addopt(Xcp,"-C");
                        break;

		case 'D': 	/* Define name with value def, otherwise 1 */
			t = stralloc(strlen(optarg)+2);
                        (void) sprintf(t,"-%c%s",c,optarg);
                        addopt(Xcp,t);
                        break;

		case 'e':       /* Make name the new entry point in ld */
                                /* Take last occurence */
                        earg = stralloc(strlen(optarg) + 3);
                        (void) sprintf(earg,"-%c%s",optopt,optarg);
                        break;
		case 'E':       /* run only preprocessor part of
                                   compiler, output to stdout */
                        Eflag++;
                        addopt(Xcp,"-E");
                        cflag++;
                        break;

		case 'f':       /* floating point interpreter */
                        (void) fprintf(stderr, "-f option ignored on this processor\n");
                        break;

		case 'g':       /* turn on symbols and line numbers */
                        dsflag = dlflag = 0;
                        gflag++;
                        break;

		case 'H':       /* preprocessor - print pathnames of 
				   included files on stderr */
                        addopt(Xcp,"-H");
                        break;

		case 'I':
                        t = stralloc(strlen(optarg)+2);
                        (void) sprintf(t,"-%c%s",c,optarg);
                        addopt(Xcp,t);
                        break;

		case 'l':       /* ld: include library */
                        lflag++;
                        /*FALLTHRU*/
                case 'L':       /* ld: alternate lib path prefix */
                        t = stralloc(strlen(optarg) + 3);
                        (void) sprintf(t,"-%c%s",optopt,optarg);
                        addopt(Xld,t);
                        break;
		
		case 'o':       /* object file name */
                        if (optarg[0]) {
                                ld_out = optarg;
                                if (((c = getsuf(ld_out)) == 'c') ||
                                        (c == 'i') || (c == 's')){
                                        (void) fprintf(stderr,"Illegal suffix for object file %s\n", ld_out);
                                        exit(17);
                                }
                        }
                        break;

		case 'O':       /* invoke optimizer */
                        Oflag++;
                        break;

		case 'p':	/* standard profiler */

			pflag++;
			qpflag=1;

#if defined(M32) || defined(i386)
			crt = MCRT1;
#else
			crt = MCRT0;
#endif
			if ( qarg != 0) {
				(void) fprintf(stderr,"Warning: using -p, ignoring the -q%c option\n",qarg);
				qarg= 0; /* can only have one type 
						of profiling on */
			}
			break;

		case 'P':       /* run only preprocessor part of
                                   compiler, output to file.i */
                        Pflag++;
                        addopt(Xcp,"-P");
                        cflag++;
                        break;

		case 'q':	/* xprof, lprof or standard profiler */

			if (strcmp(optarg,"p") == 0) {
				pflag++;
				qpflag=2;
#if defined(M32) || defined(i386)
				crt = MCRT1;
#else
				crt = MCRT0;
#endif
				if ( qarg != 0) {
                                	(void) fprintf(stderr,"Warning: using -qp, ignoring the -q%c option\n",qarg);
                                	qarg= 0; /* can only have one 
						    type of profiling on */
                        		}

			} else if (strcmp(optarg,"l")==0 || strcmp(optarg,"x")==0) {
				qarg = optarg[0];
				if (pflag != 0) {
					(qpflag == 1)?
					  (void) fprintf(stderr,"Warning: using -q%c, ignoring the -p option\n",qarg):
					  (void) fprintf(stderr,"Warning: using -q%c, ignoring the -qp option\n",qarg);

					pflag= 0;
				}
			} else {
				(void) fprintf(stderr,"No such profiler %sprof\n",optarg);
				exit(1);
			}
			break;

		case 'S':	/* produce assembly code only */
			Sflag++;
			cflag++;
			break;
		case 'u':       /* ld: enter sym arg as undef in sym tab */
                        t = stralloc(strlen(optarg) + 4);
                        addopt(Xld,"-u");
                        addopt(Xld,optarg);
                        break;

		case 'U':
                        t = stralloc(strlen(optarg)+2);
                        (void) sprintf(t,"-%c%s",c,optarg);
                        addopt(Xcp,t);
                        break;

                case 'V':       /* version flag or ld VS flag */
                        addopt(Xcp,"-V");
#ifndef MERGE
                        addopt(Xc0,"-V");
#endif
                        addopt(Xc2,"-V");
                        addopt(Xas,"-V");
                        addopt(Xld,"-V");
                        (void) fprintf(stderr, "%scc: %s %s\n",
                                prefix, CPL_PKG, CPL_REL);
                        break;

		case 'W':
			if (optarg[1] != ',' || optarg[2] == '\0') {
				(void) fprintf(stderr,"Invalid subargument: -W%s\n", optarg);
				exit(1);
			}
			t = optarg;
			if ((i = getXpass((c = *t), "-W")) == -1) {
				(void) fprintf(stderr,"Invalid subargument: -W%s\n", optarg);
				exit(1);
			}
			t+=2;
			done=0;
			while (!done) {
				p=t;
				while (((s = strchr(p,',')) != NULL) &&
								(*(s-1) == '\\')) {
					p=s;
					s--;
					while (*s != '\0') {
						*s = *(s+1);
						s++;
					}
				}
				if (s != NULL)
					*s = '\0';
				else
					done=1;
				nopt =stralloc(strlen(t)+1);
				(void) strcpy(nopt, t);
				addopt(i,nopt);
				t+= strlen(t) + 1;
			}
			break;

		case 'Y':
			if (((chpiece=strtok(optarg,",")) != optarg) ||
				((npath=strtok(NULL,",")) == NULL)) {
				(void) fprintf(stderr,"Invalid argument: -Y %s\n",optarg);
				exit(1);
			} else if ((t=strtok(NULL," ")) != NULL) {
				(void) fprintf(stderr,"Too many arguments: -Y %s,%s,%s\n",chpiece, npath,t);
				exit(1);
			}
			chg_pathnames(prefix, chpiece, npath);
			break;

#if defined(M32) || defined(i386)

		case 'A':       /* preprocessor - asserts the predicate 
				   and may associate the pp-tokens with 
				   it as if a #assert */
                        t = stralloc(strlen(optarg) + 3);
                        (void) sprintf(t,"-%c%s",optopt,optarg);
                        addopt(Xcp,t);
                        break;

                case 'B':       /* Govern library searching algorithm in ld */
                        if((strcmp(optarg,"dynamic") == 0) ||
			   (strcmp(optarg,"static") == 0)  ||
			   (strcmp(optarg,"symbolic") == 0))
			{
                        	t = stralloc(strlen(optarg) + 3);
                        	(void) sprintf(t,"-%c%s",optopt,optarg);
                        	addopt(Xld,t);
                        } 
			else
                        	(void) fprintf(stderr, 
				  "Warning:  illegal option -B%s\n",optarg);
                        break;


                case 'd':       /* Govern linking: -dy dynamic binding;
                                   -dn static binding
                                */
                        switch (optarg[0]) {
                                case 'y':
                                        addopt(Xld,"-dy");
                                        break;
                                case 'n':
                                        addopt(Xld,"-dn");
                                        break;
                                default:
                                        (void) fprintf(stderr,
					"Warning: illegal option -d%c\n"
					,optarg[0]);
                        }
                        break;

		case 'G':       /* used with the -dy option to direct linker
                                   to produce a shared object. The start up
                                   routine (crt1.o) shall not be called */

                        Gflag++;
                        addopt(Xld, "-G");
                        break;

		case 'h':       /* ld: Use name as the output filename in the
                                   link_dynamic structure. Take last occurence.
                                */
                        harg = stralloc(strlen(optarg) + 3);
                        (void) sprintf(harg,"-%c%s",optopt,optarg);
                        break;

		case 'Q':       /* add version stamping information */
                        switch (optarg[0]) {
                                case 'y':
                                        Qflag = 1;
                                        break;
                                case 'n':
                                        Qflag = 0;
                                        break;
                                default:
                                        (void) fprintf(stderr,
					"illegal option -Q %c\n",optarg[0]);
                                        exit(1);
                        }
                        break;

		case 't': 	/* Report execution times for the tools */
			(void) fprintf(stderr,
			"Warning: the -t option is not available in b3\n");
                        break;
                case 'T':	
                        (void) fprintf(stderr,
			"Warning: the -T option is not available in b3\n");
                        break;

                case 'v':       /* tell comp to run in verbose mode */
                        addopt(Xc0,"-v");
                        break;

                case 'X':       /* ANSI C options */
                        if (Xarg != NULL && Xarg[2] != optarg[0])
                                Xwarn = 1;

                        switch (optarg[0]) {
                                case 't':
                                        Xarg = "-Xt";
                                        break;
                                case 'a':
                                        Xarg = "-Xa";
                                        break;
                                case 'c':
                                        Xarg = "-Xc";
                                        break;
                                default:
                                        (void) fprintf(stderr,
					"illegal option -X%c\n", optarg[0]);
                                        exit(1);
                        }
                        break;

		case 'z':       /* turn on asserts in the linker */
                        if((strcmp(optarg,"text") == 0) ||
                           (strcmp(optarg,"defs") == 0) ||
                           (strcmp(optarg,"nodefs") == 0) )
                        {
                                t = stralloc(strlen(optarg) + 3);
                                (void) sprintf(t,"-%c%s",optopt,optarg);
                                addopt(Xld,t);
                        }
                        else
                                (void) fprintf(stderr,
                                "Warning:  illegal option -z%s\n",optarg);
                        break;
#endif

#if defined(M32) 
                case 'F':
                        (void) fprintf(stderr, "Warning: -F has no affect - option will be removed in the next release.\n");
                        break;
#endif

#if defined(M32) 
		case 'J': /* source archive */
			Jarg = stralloc(strlen(optarg) + 3);
			(void) strcpy(Jarg, optarg);
			break;
#endif
#if defined(M32) || defined(i386)

		case 'K':       /* optimize for size or speed */
                        while ((s=strtok(optarg, ",")) != NULL)
                        {
                                if (strcmp(s, "PIC")==0)
                                        Parg = "-KPIC";
                                else if (strcmp(s, "pic")==0)
                                        Parg = "-Kpic";
				else if (strcmp(s, "minabi") == 0) {
					addopt(Xld,"-I");
					addopt(Xld,LDSO_NAME);
					Kabi = 1;
				}

#if defined(M32) 
                                else if (strcmp(s,"sd")==0)
                                        Karg = "-Ksd";
                                else if (strcmp(s,"sz")==0)
                                        Karg = "-Ksz";
                                else if (strcmp(s, "fpe")==0) {
#if defined(SRC_PROD)
                                        K1arg = 1;
#else
					K1arg = 2;
					(void) fprintf(stderr,"Warning, -Kfpe is obsolete, assuming -Kmau\n");
#endif
				}
                                else if (strcmp(s, "mau")==0)
                                        K1arg = 2;
#endif
                                else {
                                        (void) fprintf(stderr,
					"Illegal argument to -K flag, '-K %s'\n"
					,optarg);
                                        exit(1);
                                }
                                optarg = NULL;
                        }
                        break;
#endif

#if defined(i386)
		case 'Z':	/* cpp: pack structures for 386 */
			if (optarg[0] == 'p')
			{
				switch (optarg[1])
				{
					case '1':
						addopt(Xc0, "-Zp1");
                                                break;
                                        case '2':
                                                addopt(Xc0, "-Zp2");
                                                break;
                                        case '4':
                                                addopt(Xc0, "-Zp4");
                                                break;
                                        case '\0':
                                                addopt(Xc0, "-Zp1");
                                                break;
                                        default:
                                                (void) fprintf(stderr, "Illegal argument to -Zp flag\n");
                                                exit(1);
                                }
                        } else {
                                (void) fprintf(stderr, "Illegal argument to -Z flag\n");
                                exit(1);
                        }
			break;
#endif


		case '?':	/* opt char not in optstr; pass to ld */
			if ( strchr(OPTSTR,optopt) != NULL ) {
				(void) fprintf(stderr, 
				  "Option -%c requires an argument\n", optopt);
				exit(1);
			}
			t = stralloc( 3 );
			(void) sprintf(t,"-%c",optopt);
			addopt(Xld,t);
			break;

		case '#':	/* cc command debug flag */
			debug++;
			break;

		case EOF:	/* file argument */
			t = argv[optind++]; /* -> file name */
			if (((c = getsuf(t)) == 'c') ||
				(c == 'i') || (c == 's') || Eflag) {
				addopt(CF,t);
				t = setsuf(t, 'o');
			}
			if (nodup(list[Xld], t)) {
				addopt(Xld,t);
				if (getsuf(t) == 'o')
					nxo++;
				else
					nxn++;
			}
			break;
		} /* end switch */
	} /* end while */

	if ( (nxo == 0) && (nxn == 0) ) {
		(void)fprintf(stderr,"usage: cc [ options] files\n");
		exit(1);
		}
#if defined(M32) || defined(i386)
        if (earg != NULL)
                addopt(Xld, earg);
	if (harg != NULL)
		addopt(Xld, harg);
#endif

#if defined(M32) || defined(i386)
	/* -KPIC and -Kpic are passed to acomp as -2k on the 3b2 */
	if (Parg != NULL)
		addopt(Xc0, "-2k"); 
#endif

#if defined(M32) 
	if (K1arg == 2)
		addopt(Xas, "-Kmau");
#endif

	/* if -q option is given, make sure basicblk exists */
	if (qarg) {
		if(Oflag) {
			Oflag = 0;
			(void) fprintf(stderr,"%cprof and optimization mutually exclusive; -O disabled\n",qarg);
		}
		passprof= makename(profdir,prefix,N_PROF);
		if (debug <= 2) {
		  if (access(passprof, 0)==-1) {
			(void) fprintf(stderr,"%cprof is not available\n",qarg);
			exit(1);
		  }
		}
#if defined(M32) || defined(i386)
		crt = PCRT1;
#else
		crt = PCRT0;
#endif
		dsflag = dlflag = 0;
		gflag++;
		addopt(Xld,"-lprof");
		addopt(Xld,"-lelf");
		addopt(Xld,"-lm");
	}

	/* if -o flag was given, the output file shouldn't overwrite an input file */
	if (ld_out != NULL) {
		if (!nodup(list[Xld], ld_out)) {
			(void) fprintf(stderr,"-o would overwrite %s\n",ld_out);
			exit(1);
		}
	}

	if (nlist[CF] && !(Pflag || Eflag)) /* more than just the preprocessor is
			        	     * running, so temp files are required */
		mktemps();

	if (eflag)
		dexit();

	if (signal(SIGHUP, SIG_IGN) == SIG_DFL)
		(void) signal(SIGHUP, idexit);
	if (signal(SIGINT, SIG_IGN) == SIG_DFL)
		(void) signal(SIGINT, idexit);
	if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
		(void) signal(SIGTERM, idexit);

	mk_defpaths(prefix);

#if defined(M32) || defined(i386)
	if (Oflag)
	{
		if (Parg != NULL)
			addopt(Xc2, Parg);

#if defined(M32)
		if (Karg != NULL)
			addopt(Xc2, Karg);
		if (K1arg == 2)
			addopt(Xc2,"-Kmau");
#if defined(SRC_PROD)
		else if (K1arg == 1)
			addopt(Xc2,"-Kfpe");
#endif

		if (Jarg != NULL) /* optimizer specific library */
		{
			if ( preslash(Jarg) ) {
				if ( sufsa(Jarg) ) {
					   t=stralloc(strlen(Jarg) +3);
					   (void) sprintf(t,"-J%s",Jarg);
					   addopt(Xc2,t);
(void) fprintf(stderr," Warning:The format \"-J/path/libx.sa\" may be obsolete in the future. Use \"-Jx\".\n");  
					} else {
					  (void) fprintf(stderr," -J%s must end in .sa\n",optarg);
					}
			}
			else
			{
			/* fix - up add path/lib....sa */
			if (Jlibpath == NULL )
				{
					Jlibpath=stralloc(strlen(fplibdir));
					(void) strcpy(Jlibpath,fplibdir);
				}
				t=stralloc(strlen(Jarg)+ strlen(Jlibpath)+10);
				(void) sprintf(t,"-J%s/lib%s.sa",Jlibpath,Jarg);
				addopt(Xc2,t);
			}
		}
#endif
	}
#endif

#if defined(M32) || defined(i386)
	if (Qflag) {
		addopt(Xcp,"-Qy");
#ifndef MERGE
		addopt(Xc0,"-Qy");
#endif
		addopt(Xas,"-Qy"); 
		addopt(Xld,"-Qy");
		if (Oflag) 
			addopt(Xc2,"-Qy");
	}

	if ( Xarg != NULL) { /* if more then one -X option and each has a differnt */
		if ( Xwarn)  /* argument, then warning */
		(void) fprintf(stderr,"Warning: using %s, ignoring all other -X options\n",Xarg);
		addopt(Xcp,Xarg);
#ifndef MERGE
		addopt(Xc0,Xarg);
#endif
		if (Oflag)
			addopt(Xc2,Xarg);
	}	
	else
		Xarg = "-Xt"; /* -Xt is the default */
#endif

	/* process each file (name) in list[CF] */

	for (i = 0; i < nlist[CF]; i++) {
		if (nlist[CF] > 1)
			(void) fprintf(stderr,"%s:\n", list[CF][i]);
		sfile = (getsuf(list[CF][i]) == 's');
		if (sfile && !Eflag && !Pflag && !Sflag) {
			as_in = list[CF][i];
			(void) assemble(i);
			continue;
		}
		else if (sfile && Sflag) {
			(void) fprintf(stderr,"Conflicting option -S with %s\n", list[CF][i]);                                continue;
                        }
		if (getsuf(list[CF][i]) == 'i') {
			if ( Eflag || Pflag ) {
				(void) fprintf(stderr,"Conflicting option -%c with %s\n", Eflag ? 'E' : 'P', list[CF][i]);
				continue;
			}
#ifndef MERGE
			cpp_out = list[CF][i];
#endif
		}
#ifndef MERGE
		else if (!preprocess(i))
			continue;
#endif
			

		if (!compile(i))
			continue;

		if (Oflag)
			(void) optimize(i);

		if (passprof)
			(void) profile(i);

		if (!Sflag && !sfile)
			(void) assemble(i);

	} /* end loop */

	if (!eflag && !cflag)
		linkedit();

	dexit();
	/*NOTREACHED*/
}

#ifndef MERGE

/*===================================================================*/
/*								     */
/*                   PREPROCESSOR                                    */
/*                                                                   */
/*===================================================================*/

static
preprocess (i)
	int i; /* list[CF] index of filename */
{
	int j;
	
	nlist[AV]= 0;
	/* build argv argument to callsys */
	addopt(AV,passname(prefix, N_CPP));
	for (j = 0; j < nlist[Xcp]; j++)
		addopt(AV,list[Xcp][j]); 	/* add options */
#ifdef MC68
#ifdef vax
	addopt(AV,"-Uvax");
#endif
#ifdef u3b	
	addopt(AV,"-Uu3b");
#endif
	addopt(AV,"-Uunix");
	addopt(AV,"-Dmc68000");
#ifdef INT16BIT
	addopt(AV,"-Dmc68k16");
#else
	addopt(AV,"-Dmc68k32");
#endif
#endif

	addopt(AV,list[CF][i]);         /* add input file name - default stdin */
	addopt(AV,cpp_out=Eflag ? "-" : (Pflag ? setsuf(list[CF][i], 'i') : tmp2));
					/* add output file name - default stdout */

	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	if (callsys(passcp, list[AV])) {
		cflag++;
		eflag++;
		return(0);
	}
	if (Pflag || Eflag)
		return(0);
	return(1);
}

#endif


/*===================================================================*/
/*                                                                   */
/*                  COMPILER 					     */
/*                                                                   */
/*===================================================================*/

static int
compile (i)
	int i;
{
	int j;
	int front_ret;
	
	nlist[AV]= 0;
	addopt(AV,passname(prefix, N_C0));
	addopt(AV,"-i");
#ifndef MERGE
	addopt(AV,cpp_out);
#else
	addopt(AV,list[CF][i]);
#endif
	addopt(AV,"-o");
#ifdef MERGE
	if (Eflag || Pflag)
		addopt(AV, Eflag ? "-" : setsuf(list[CF][i], 'i') );
	else
#endif
		addopt(AV,c_out = as_in = (Sflag && !Oflag && !qarg) ? setsuf(list[CF][i], 's') : tmp2);

	addopt(AV,"-f");
	addopt(AV,list[CF][i]);

#ifdef MERGE
	if(!Eflag && !Pflag) {
#endif
		if (dsflag)
			addopt(AV,"-ds");
		if (dlflag)
			addopt(AV,"-dl");
		if (pflag)
			addopt(AV,"-p");

#ifdef MERGE
	}
#endif
	for (j = 0; j < nlist[Xc0]; j++)
		addopt(AV,list[Xc0][j]);

	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	front_ret = callsys( passc0, list[AV] );
#ifdef MERGE
	if ((Eflag || Pflag) && front_ret == 0 )
		return(0);
	else
#endif
	if (front_ret >= 1) {
		cflag++;
		eflag++;
		if (Pflag){
			cunlink(setsuf(list[CF][i], 'i'));
			}
		else {
			cunlink(c_out);
			}
		return(0);
	}

#ifdef PERF
	STATS("compiler ");
#endif
	return(1);
}


/*===================================================================*/
/*                                                                   */
/*                      OPTIMIZER                                    */
/*                                                                   */
/*===================================================================*/

static int
optimize (i)
	int i;
{
	int j;
	
	nlist[AV]= 0;
#if defined(M32) 
		addopt(AV,passname(prefix, passname("new", N_OPTIM)) );
#else
		addopt(AV,passname(prefix, N_OPTIM));
#endif

	addopt(AV,"-I");
	addopt(AV,c_out);
	addopt(AV,"-O");
	addopt(AV,as_in
		 = (Sflag && !qarg) ? setsuf(list[CF][i], 's') : tmp4);
	for (j = 0; j < nlist[Xc2]; j++)
		addopt(AV,list[Xc2][j]);
	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	if (callsys(passc2, list[AV])) {
		if (Sflag) {
			if (move(c_out, as_in)) { /* move failed */
				cunlink(c_out);
				return(0);
			}
		}
		else {
			cunlink(as_in);
			as_in = c_out;
		}
		(void) fprintf(stderr,"Optimizer failed, -O ignored for %s\n", list[CF][i]);
	} else {
		c_out= as_in;
		cunlink(tmp2);
		}

#ifdef PERF
	STATS("optimizer");
#endif

	return(1);
}

/*===================================================================*/
/*                                                                   */
/*                      PROFILER                                     */
/*                                                                   */
/*===================================================================*/

static int
profile(i)
	int i;
{
	int j;
	
	nlist[AV]= 0;
	addopt(AV,passname(prefix, N_PROF));
	if (qarg == 'l')
		addopt(AV,"-l");
	else
		addopt(AV,"-x");

	if (Qflag)  /* By default, version stamping option is passed */
		addopt(AV,"-Qy"); 

	for (j=0; j < nlist[Xbb]; j++)
		addopt(AV,list[Xbb][j]);

	addopt(AV,c_out);
	addopt(AV,as_in
		 = Sflag ? setsuf(list[CF][i], 's') : tmp5);
	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	if (callsys(passprof, list[AV])) {
		if (Sflag) {
			if (move(c_out, as_in)) { /* move failed */
				cunlink(c_out);
				return(0);
			}
		}
		else {
			cunlink(as_in);
			as_in = c_out;
		}
		(void) fprintf(stderr,"Profiler failed, '-q %c' ignored for %s\n", qarg, list[CF][i]);
	} else 
		(void)unlink(tmp2);
		

#ifdef PERF
	STATS("profiler");
#endif

	return(1);
}
	
/*===================================================================*/
/*                                                                   */
/*                    ASSEMBLER                                      */
/*                                                                   */
/*===================================================================*/

static int
assemble (i)
	int i;
{
	int j;
	
	nlist[AV]= 0;
	addopt(AV,passname(prefix, N_AS));
#if !defined(M32) && !defined(i386)
	if (dlflag)
		addopt(AV,"-dl");
#endif

	addopt(AV,"-o");
	addopt(AV,as_out = setsuf(list[CF][i], 'o'));
	for (j = 0; j < nlist[Xas]; j++)
		addopt(AV,list[Xas][j]);
	addopt(AV,as_in);
	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	if (callsys(passas, list[AV])) {
		cflag++;
		eflag++;
		cunlink(as_out);
		return(0);
	}

#ifdef PERF
	STATS("assembler");
#endif

	return(1);
}


/*===================================================================*/
/*                                                                   */
/*                LINKAGE EDITOR                                     */
/*                                                                   */
/*===================================================================*/

static void
linkedit ()
{
	int j;
	char *str;
	char *t;
	char *ldpath;
	static	char *cclibs, *cclibs1, *cclibs2;
	static char whites[] = "\n\t\b \f\v";
	
	nlist[AV]= 0;
	addopt(AV,passname(prefix, N_LD));

		if ( Gflag == 0)
			addopt(AV,makename(crtdir,prefix,crt));

#if defined(M32) || defined(i386)
        if (Gflag && ( qarg == 'l') )
		addopt(AV,makename(crtdir,prefix,PCRTI));
	else
		addopt(AV,makename(crtdir,prefix,CRTI));
#endif

#if defined(M32) || defined(i386)
			t = stralloc( strlen(values) + strlen(Xarg) + 2);
			(void) sprintf(t,"%s%s.o",values,Xarg);
			addopt(AV,makename(crtdir,prefix,t));
#endif


	if (ld_out != NULL)
        {
                addopt(AV,"-o");
                addopt(AV,ld_out);
        }

	for (j = 0; j < nlist[Xld]; j++) /* app files, opts, and libs */
		addopt(AV,list[Xld][j]);

	cclibs = getenv("CCLIBS");
	if (cclibs != NULL)
	{
		if ( (cclibs1 = strtok(cclibs, whites)) != NULL)
			addopt(AV,cclibs1);

		while ( (cclibs2 = strtok(NULL, whites)) != NULL )
			addopt(AV,cclibs2);
	}

	if (Gflag == 0)
	{
		addopt(AV,"-lc");
	}


#if !IAPX && !MC68 && !M32 && !i386
	if (gflag)
		addopt(AV,"-lg");
#endif

#if defined(M32) || defined(i386)
	if (Gflag && (qarg == 'l'))
		addopt(AV,makename(crtdir,prefix,PCRTN));
	else
		addopt(AV,makename(crtdir,prefix,CRTN));
#endif
	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	if (nlist[AV] > MINLDARGS) /* if file given or flag set by user */
	{
		PARGS;
		if (nxo || nxn)
			eflag |= callsys(passld, list[AV]);
	}

	if ((nlist[CF] == 1) && (nxo == 1) && (eflag == 0) && (debug <= 2) )
		/* delete .o file if single file compiled and loaded */
		cunlink(setsuf(list[CF][0], 'o'));

#ifdef PERF
	STATS("link edit");
#endif

}


/* 
   chg_pathnames() overrides the default pathnames as specified by the -Y option
*/

static void
chg_pathnames(prefix, chpiece, npath)
char *prefix;
char *chpiece;
char *npath;
{
	char	*t;
	char	*topt;
	char 	*optim_prefix = NULL;

	for (t = chpiece; *t; t++)
		switch (*t) {
#ifndef MERGE
		case 'p':
			passcp = makename( npath, prefix, N_CPP );
			break;
#endif
		case '0':
			passc0 = makename( npath, prefix, N_C0 );
			break;
		case '2':

#if defined(M32) 
			if ( strlen(prefix) == 0 )
				passc2 = makename( npath, "new", N_OPTIM );
			else {
				optim_prefix = stralloc(strlen(prefix)+3);
				(void) strcpy(optim_prefix,prefix);
				(void) strcat(optim_prefix,"new");
				passc2 = makename( npath, optim_prefix, N_OPTIM );
				}
#else
			passc2 = makename( npath, prefix, N_OPTIM );
#endif
			break;
		case 'b':
			profdir= stralloc(strlen(npath));
			(void) strcpy(profdir,npath);
			break;
		case 'a':
			passas = makename( npath, prefix, N_AS );
			break;
		case 'l':
			passld = makename( npath, prefix, N_LD );
			break;
		case 'S':
			crtdir= stralloc(strlen(npath));
			(void) strcpy(crtdir,npath);
			break;
		case 'I':
			topt = stralloc(strlen(npath)+4);
			(void) sprintf(topt,"-Y%s",npath);
			addopt(Xcp,topt);
			break;
#if defined(M32) 
		case 'J':
			Jlibpath=stralloc(strlen(npath));
			(void) strcpy(Jlibpath,npath);
			break;
#endif

		case 'L':
			if(libpath) {
				(void) fprintf(stderr,"-YL may not be used with -YP\n");
				exit(1);
			}
			libdir = stralloc(strlen(npath));
			(void) strcpy(libdir,npath);
			break;
		case 'U':
			if(libpath) {
				(void) fprintf(stderr,"-YU may not be used with -YP\n");
				exit(1);
			}
			llibdir = stralloc(strlen(npath));
			(void) strcpy(llibdir,npath);
			break;

#if defined(M32) 
		case 'F':
#if defined(SRC_PROD)
			if (K1arg == 2)
			{
				if(libpath) {
					(void) fprintf(stderr,"-YF may not be used with -YP\n");
					exit(1);
				}
				fplibdir= stralloc(strlen(npath));
				(void) strcpy(fplibdir,npath);
			}
#else
			fprintf(stderr,"Warning, -YF is obsolete, ignored\n");
#endif
			break;
#endif
		case 'P':
#if defined(M32)
			if(fplibdir || libdir || llibdir) {
				(void) fprintf(stderr, "-YP may not be used with -YL, -YU, or -YF\n");
				exit(1);
			}
#else
			if(libdir || llibdir) {
				(void) fprintf(stderr, "-YP may not be used with -YL or -YU\n");
				exit(1);
			}
#endif
			libpath = stralloc(strlen(npath));
			(void) strcpy(libpath,npath);
			break;

			
		default: /* error */
			(void)fprintf(stderr,"Bad option -Y %s,%s\n",chpiece, npath);
			exit(1);
		} /* end switch */
}

static void
mk_defpaths(prefix)
char *prefix;
{
	char 	*optim_prefix = NULL;
	register char 	*nlibpath;

	/* make defaults */
#if defined(M32) 
	if (!fpdir)
		fpdir= makename(LIBDIR,"","fp");
	if (!fplibdir)
		fplibdir= makename(LIBDIR,"","fp");
	if(!Jlibpath)
		Jlibpath= makename(LIBDIR, "", "fp");
#endif

#ifndef MERGE
	if (!passcp)
		passcp = makename( LIBDIR, prefix, N_CPP );
#endif

	if (!passc0)
#if defined(M32) && defined(SRC_PROD)
	if(K1arg==2)
		passc0 = makename( fpdir, prefix, N_C0 );
	else
		passc0 = makename( LIBDIR, prefix, N_C0 );
#else
		passc0 = makename( LIBDIR, prefix, N_C0 );
#endif
	if (!passc2)
#if defined(M32) 
			if (strlen(prefix) == 0)
				passc2 = makename( LIBDIR, "new", N_OPTIM );
			else {
				optim_prefix = stralloc(strlen(prefix)+3);
				(void) strcpy(optim_prefix,prefix);
				(void) strcat(optim_prefix,"new");
				passc2 = makename( LIBDIR, optim_prefix, N_OPTIM );
				}
#else
			passc2 = makename( LIBDIR, prefix, N_OPTIM );
#endif
	if (!passas)
		passas = makename( BINDIR, prefix, N_AS );
	if (!passld)
		passld = makename( BINDIR, prefix, N_LD );
	if (!crtdir) {
		crtdir = LIBDIR;
#if defined(M32) && defined(SRC_PROD)
	if (K1arg==2)
		crtdir = fpdir;
#endif
	}

	if(!libpath)
		libpath = LIBPATH;
	if(libdir || llibdir) {
		nlibpath = stralloc(strlen(libpath) + strlen(libdir) + strlen(llibdir));
		process_lib_path(libpath,nlibpath);
		libpath = nlibpath;
	}
#if defined(M32) && defined(SRC_PROD)
	if(K1arg == 2) {
		nlibpath = stralloc(strlen(libpath) + strlen(fplibdir)+1);
		(void) strcpy(nlibpath,fplibdir);
		(void) strcat(nlibpath,":");
		(void) strcat(nlibpath,libpath);
		libpath = nlibpath;
	}
#endif
	if(Kabi) {
		nlibpath = stralloc(strlen(libpath) + strlen(ABILIBDIR)+1);
		(void) sprintf(nlibpath,"%s:%s",ABILIBDIR,libpath);
		libpath = nlibpath;
	}
		
	if (pflag) {
		int i;
		char * cp;
		char * cp2;

		nlibpath = libpath;
		/* count number of paths */
		for(i=0; ; i++) {
			nlibpath = strchr(nlibpath,':');
			if(nlibpath == 0) {
				i++;
				break;
			}
			nlibpath++;
		}

		/* get enough space for path/libp for every path in libpath +
			enough for the :s */
		nlibpath = stralloc(2 * strlen(libpath) - 1 + i * 6 );

		cp2 = libpath;
		while(cp =  strchr(cp2,':')) {
			if(cp == cp2)
				(void) strcat(nlibpath,"./libp:");
			else {
				(void) strncat(nlibpath,cp2,cp - cp2);
				(void) strcat(nlibpath,"/libp:");
			}
			cp2 = cp + 1;
		}

		if(*cp2 == '\0')
			(void) strcat(nlibpath,"./libp:");
		else {
			(void) strcat(nlibpath,cp2);
			(void) strcat(nlibpath,"/libp:");
		}

		(void) strcat(nlibpath,libpath);
		libpath = nlibpath;

	}
	addopt(Xld,"-Y");
	nlibpath = stralloc(strlen(libpath) + 2);
	(void) sprintf(nlibpath,"P,%s",libpath);
	addopt(Xld,nlibpath);
}


/* return the prefix of "cc" */

static char *
getpref( cp )
	char *cp;	/* how cc was called */
{
	static char	tprefix[BUFSIZ];  /* enough room for prefix and \0 */
	int		cmdlen,
			preflen;
	char		*prefptr,
			*ccname;

	ccname= "cc";
	if ((prefptr= strrchr(cp,'/')) == NULL)
		prefptr=cp;
	else
		prefptr++;
	cmdlen= strlen(prefptr);
	preflen= cmdlen - strlen(ccname);
	if ( (preflen < 0 )		/* if invoked with a name shorter
					   than ccname */
	    || (strcmp(prefptr + preflen, ccname) != 0)) {
		(void)fprintf(stderr, "command name must end in \"%s\"\n", ccname);
		exit(1);
		/*NOTREACHED*/
	} else {
		(void) strncpy(tprefix,prefptr,preflen);
		tprefix[preflen]='\0';
		return(tprefix);
	}
}

/* Add the string pointed to by opt to the list given by list[lidx]. */

static void
addopt(lidx, opt)
int	lidx;	/* index of list */
char	*opt;  /* new argument to be added to the list */
{
	/* check to see if the list is full */
	if ( nlist[lidx] == limit[lidx] - 1 ) {
		limit[lidx] += argcount;
		if ((list[lidx]= (char **)realloc((char *)list[lidx],
					limit[lidx]*sizeof(char *))) == NULL) {
			(void)fprintf(stderr, "Out of space\n");
			dexit();
		}
	}

	list[lidx][nlist[lidx]++]= opt;
}

/* make absolute path names of called programs */

static char *
makename( path, prefix, name )
	char *path;
	char *prefix;
	char *name;
{
	char	*p;

	p = stralloc(strlen(path)+strlen(prefix)+strlen(name)+1);
	(void) strcpy( p, path );
	(void) strcat( p, "/" );
	(void) strcat( p, prefix );
	(void) strcat( p, name );

	return( p );
}

/* make the name of the pass */

static char *
passname(prefix, name)
	char *prefix;
	char *name;
{
	char	*p;

	p = stralloc(strlen( prefix ) + strlen( name ));
	(void) strcpy( p, prefix );
	(void) strcat( p, name );
	return( p );
}

/*ARGSUSED0*/
static void
idexit(i)
	int i;
{
        (void) signal(SIGINT, idexit);
        (void) signal(SIGTERM, idexit);
        eflag = 100;
        dexit();
}


static void
dexit()
{
	if (!Pflag) {
		if (qarg)
			cunlink(tmp5);
		if (Oflag)
			cunlink(tmp4);
		cunlink(tmp2);
#ifndef MERGE
		cunlink(tmp1);
#endif
	}
#ifdef PERF
	if (eflag == 0)
		pwrap();
#endif
	exit(eflag);
}


static void
error(s, x, y)
	char *s, *x, *y;
{
	(void)fprintf(stderr , s, x, y);
	(void) putc('\n', stderr );
	cflag++;
	eflag++;
}




static char
getsuf(as)
	char as[];
{
	register int c;
	register char *s;

	s = as;
	c = 0;
	while ( *s )
		if (*s++ == '/')
			c = 0;
		else
			c++;
	s -= 2;
	if ((c > 2) && (*s++ == '.'))
		return(*s);
	return(0);
}

static char *
setsuf(as, ch)
	char *as;
	char ch;
{
	register char *s, *s1;
	register char *t1;

	s = s1 = copy(as);
	while (*s)
		if (*s++ == '/')
			s1 = s;

	t1 = s1; 
	s1 = s1 + ( strlen(s1) -1 );
	*s1 = ch;

	return(t1);
}

static int
callsys(f, v)
	char f[], *v[];
{
	register pid_t pid, w;
	char *tf;
	int status;

	(void) fflush(stdout);
	(void) fflush(stderr);

	if (debug >= 2) {	/* user specified at least two #'s */
		(void)fprintf(stderr,"%scc: process: %s\n", prefix, f);
		if (debug >= 3)	/* 3 or more #'s:  don't exec anything */
			return(0);
	}

#ifdef PERF
	ttime = times(&ptimes);
#endif

	if ((pid = fork()) == 0) {
		(void) execv(f, v);
		(void)fprintf(stderr,"Can't exec %s\n", f);
		exit(100);
	}
	else
		if (pid == -1) {
			(void)fprintf(stderr,"Process table full - try again later\n");
			eflag = 100;
			dexit();
		}
	while ((w = wait(&status)) != pid && w != -1) ;

#ifdef PERF
	ttime = times(&ptimes) - ttime;
#endif

	if (w == -1) {
		(void)fprintf(stderr,"Lost %s - No child process!\n", f);
		eflag = w;
		dexit();
	}
	else {
		if (((w = status & 0xff) != 0) && (w != SIGALRM)) {
			if (w != SIGINT) {
				(void)fprintf(stderr, "Fatal error in %s\n", f);
				(void)fprintf( stderr, "Status 0%d\n", status );
			}
			if (  (tf = strrchr(f,'/'))  == NULL )
				tf=f;
			else
				tf++;
#if defined(M32) 
			if ( strcmp(tf,passname(prefix,passname("new",N_OPTIM))) == 0 ) {
#else
			if ( strcmp(tf,passname(prefix,N_OPTIM)) == 0 ) {
#endif
				return(status);
				}				
			else {
				eflag = status;
                        	dexit();	
				}
		}
	}
	return((status >> 8) & 0xff);
}

static int
nodup(l, os)
	char **l, *os;
{
	register char *t;

	if (getsuf(os) != 'o')
		return(1);
	while(t = *l++) {
		if (strcmp(t,os) == 0)
			return(0);
	}
	return(1);
}

static int
move(from, to)
	char *from, *to;
{
	list[AV][0] = "mv";
	list[AV][1] = from;
	list[AV][2] = to;
	list[AV][3] = 0;
	if (callsys("mv", list[AV])) {
		error("Can't move %s to %s", from, to);
		return(1);
	}
	return(0);
}

static char *
copy(s)
	register char *s;
{
	register char *ns;

	ns = stralloc(strlen(s));
	return(strcpy(ns, s));
}


#ifdef __STDC__
static void *
#else
static char *
#endif
stralloc(n)
	unsigned int n;
{
	register char *s;

	if ((s = (char *)calloc((unsigned)(n+1),1)) == NULL) {
		error("out of space", (char *) NULL, (char *)NULL);
		dexit();
	}
	return(s);
}

static void
mktemps()
{
#ifndef MERGE
	tmp1 = tempnam(TMPDIR, "ctm1");
#endif
	tmp2 = tempnam(TMPDIR, "ctm2");
	tmp4 = tempnam(TMPDIR, "ctm4");
	tmp5 = tempnam(TMPDIR, "ctm5");
#ifndef MERGE
	if (!(tmp1 && tmp2 && tmp4 && tmp5) || creat(tmp1, (mode_t) 0666) < 0)
#else
	if (!(tmp2 && tmp4 && tmp5) || creat(tmp2, (mode_t) 0666) < 0)
#endif
		error( "%scc: cannot create temporaries: %s", prefix, tmp2);
}

static int
getXpass(c, opt)
	char	c,
		*opt;
{
	switch (c) {
	case '0':
		return(Xc0);
	case '2':
		return(Xc2);
	case 'p':
#ifdef MERGE
		return(Xc0);
#else
		return(Xcp);
#endif
	case 'b':
		return(Xbb);
	case 'a':
		return(Xas);
	case 'l':
		return(Xld);
	default:
		error("Unrecognized pass name: '%s%c'", opt, (char *)c);
		return(-1);
	}
}

#ifdef PERF
pexit()
{
	(void)fprintf(stderr, "Too many files for performance stats\n");
	dexit();
}
#endif

#ifdef PERF
pwrap()
{
	int	i;

	if ((perfile = fopen("cc.perf.info", "r")) == NULL)
		dexit();
	fclose(perfile);
	if ((perfile = fopen("cc.perf.info", "a")) == NULL)
		dexit();
	for (i = ii-1; i > 0; i--) {
		stats[i].perform.proc_user_time -= stats[i-1].perform.proc_user_time;
		stats[i].perform.proc_system_time -= stats[i-1].perform.proc_system_time;
		stats[i].perform.child_user_time -= stats[i-1].perform.child_user_time;
		stats[i].perform.child_system_time -= stats[i-1].perform.child_system_time;
	}
	for (i = 0; i < ii; i++)
		(void)fprintf(perfile, "%s\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\n",
			stats[i].module,stats[i].ttim,stats[i].perform);
	fclose(perfile);
}
#endif


static int
preslash(s)
char *s;
{
	if ( (strncmp(s,"/",1)) == NULL) {
		return(1);
	} else {
		return(0);
	}
}

static int
sufsa(s)
char *s;
{
/* returns 1 if sufix is .sa , 0 if not */
	int c;
	char t;
	c = 0;
	while (t = *s++)
		if ( t == '/')
			c = 0;
		else
			c++;
	s -= 4;
	if ( strncmp(s,".sa",3) == NULL ) {
		return(1);
	}
	return(0);
}

/* function to handle -YL and -YU substitutions in LIBPATH */

static char *
compat_YL_YU(index)
int index;
{
	/* user supplied -YL,libdir  and this is the pathname that corresponds 
		for compatibility to -YL (as defined in paths.h) */
	if(libdir && index == YLDIR)
		return(libdir);

	/* user supplied -YU,llibdir  and this is the pathname that corresponds 
		for compatibility to -YU (as defined in paths.h) */
	if(llibdir && index == YUDIR)
		return(llibdir);

	return(NULL);
}

static void
process_lib_path(pathlib,npathlib)
char * pathlib;
char * npathlib;
{
	int i;
	char * cp;
	char * cp2;


	for(i=1;; i++) {
		cp = strpbrk(pathlib,":");
		if(cp == NULL) {
			cp2 = compat_YL_YU(i);
			if(cp2 == NULL) {
				(void) strcpy(npathlib,pathlib);
			}
			else {
				(void) strcpy(npathlib,cp2);
			}
			return;
		}

		if(*cp == ':') {
			cp2 = compat_YL_YU(i);
			if(cp2 == NULL) {
				(void) strncpy(npathlib,pathlib,cp - pathlib +1);
				npathlib = npathlib + (cp - pathlib + 1);
			}
			else {
				(void) strcpy(npathlib,cp2);
				npathlib += strlen(cp2);
				*npathlib = ':';
				npathlib++;
			}
			pathlib = cp + 1;
			continue;
		}
		
	}
}

