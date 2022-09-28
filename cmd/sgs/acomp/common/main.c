/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/main.c	55.1"
/* main.c */

/* Driver code for ANSI C compiler.  Handle option processing,
** call parser, clean up.
*/

#include "p1.h"
#include <signal.h>
#include <string.h>
#include "sgs.h"			/* For RELEASE string. */
#ifdef	MERGED_CPP
#include "interface.h"			/* CPP interface */
#endif
#ifdef	IEEE_HOST
/* CG's headers defined the macro FP_NEG, which happens to
** be the name of one of the symbols in ieeefp.h.  Since we
** don't need CG's FP_NEG here (or ieeefp.h's, for that matter),
** remove CG's to avoid a conflict.
*/
#undef	FP_NEG
#include <ieeefp.h>			/* host has IEEE floating point */
#endif

#ifndef	CPL_PKG
#define	CPL_PKG "package"
#endif
#ifndef	CPL_REL
#define	CPL_REL	"release"
#endif


int d1debug;				/* declaration processing debug */
int i1debug;				/* initialization debug */
int version = V_CI4_1;			/* Version flag:  default is CI 4.1 */
int verbose;				/* be noisy about warnings if non-0 */
#ifndef LINT
static int vstamp = 0;			/* 0:  no version stamp, 1:  stamp */
#endif

extern void earlyexit();
extern void exit();
#ifndef LINT
static void dovstamp();
#endif
static int setup();
static int process();
static int cleanup();

#ifdef LINT
#define p2flags(f)			/* doesn't exist for lint */
#endif


int
main(argc,argv)
int argc;
char * argv[];
{
    int retval = setup(argc, argv);

    retval |= process();
    retval |= cleanup();
    return( retval ? 2 : 0 );
}

static int pponly = 0;			/* 1 if only preprocess */

static int
setup(argc, argv)
int argc;
char * argv[];
/* Do initializations of compiler and, if appropriate, cpp
** and lint.
*/
{
    static void setsig();
    extern void sh_proc();
    extern char * optarg;
    extern int optind;
    int c;
    char * infname = "";		/* Presumed input filename (standard in) */
    char * outfname = "";		/* Presumed output filename */
    char * errfname = "";		/* Filename for error messages. */

#define	COMP_ARGS "1:2:X:L:Vad:f:i:o:vpQ:R:Z:"
    static char args[100] = COMP_ARGS;	/* may append CPP args, too */

#ifdef	MERGED_CPP
    if( PP_INIT(argv[0], sh_proc) == PP_FAILURE)
	return( 1 );			/* initialization failure */
    (void) strcat(args, PP_OPTSTRING()); /* add in CPP options, which are
					** ignored
					*/
#endif
#ifdef	LINT
    (void) strcat(args, ln_optstring()); /* add in lint options */
#endif

    while (optind < argc) {
	c = getopt(argc, argv, args);
	switch( c ) {
	    char * cp;			/* for walking optarg */
	case '1':
	    /* Pass 1 debugging flags. */
	    cp = optarg;
	    while(*cp) {
		switch( *cp ) {
		case 'a':	++a1debug; break;
		case 'b':	++b1debug; break;
		case 'd':	++d1debug; break;
		case 'i':	++i1debug; break;
		case 'o':	++o1debug; break;
		case 's':	++s1debug; break;
		{
		    extern int yydebug;		/* yacc's debug flag */
		case 'y':	++yydebug; break;
		}
		case 'z':
		    p2flags("z");		/* really a Pass 2 flag */
		    break;
		}
		++cp;
	    }
	    break;
	case '2':
	    /* Pass 2 (CG) debugging flags. */
	    p2flags(optarg);
	    break;
	case 'a':
	{
	    extern int err_dump;
	    err_dump = 1;		/* force dump on compiler error */
	    break;
	}
#ifndef LINT
	case 'V':
	    fprintf(stderr,"acomp: %s %s\n", CPL_PKG, CPL_REL);
	    break;
	case 'Q':
	    if (optarg[1] == '\0') {
		if (optarg[0] == 'y') {
		    vstamp = 1;
		    break;
		}
		else if (optarg[0] == 'n') {
		    vstamp = 0;
		    break;
		}
	    }
	    UERROR("bad -Q");
	    break;
#endif
	case 'R':
	    /* Choose register allocation style. */
#ifdef FAT_ACOMP
	    switch (*optarg) {
	    case 'g':	al_regallo = RA_GLOBAL; break;
	    case 'o':	al_regallo = RA_OLDSTYLE; break;
	    case 'n':	al_regallo = RA_NONE; break;
	    default:	WERROR("unknown allocation style '%c'", *optarg); break;
	    }
#endif
	    break;
	case 'X':
	    /* Choose language version. */
	    switch (*optarg) {
	    case 't':	version = V_CI4_1; break;	/* Transition */
	    case 'a':	version = V_ANSI; break;	/* ANSI interp. */
	    case 'c':	version = (V_ANSI|V_STD_C); break; /* strict */
	    default:	UERROR("unknown language version '%c'", *optarg);
	    }
	    if (optarg[1] != '\0')
		UERROR("language version \"%s\"?", optarg);
	    break;
	case 'Z':
#ifdef	PACK_PRAGMA
	    if (optarg[0] == 'p' && optarg[1] != '\0')
		Pack_align = Pack_default = Pack_string(&optarg[1]);
	    else
#endif
	    {
		WERROR("invalid -Z");
	    }
	    break;
	case 'v':
	    ++verbose; break;
	case 'p':			/* Turn on profiling. */
	    cg_profile(); break;
	case 'L':
	{
	    int code;

	    /* Select loop generation code. */
	    switch (optarg[0]) {
	    case 't':	code = LL_TOP; break;
	    case 'b':	code = LL_BOT; break;
	    case 'd':	code = LL_DUP; break;
	    default:
		WERROR("loop code type %c?", *optarg);
		goto noselect;
	    }
	    if (optarg[1] == ',') {
		switch( optarg[2] ) {
		case 'w':	sm_while_loop_code = code; break;
		case 'f':	sm_for_loop_code = code; break;
		default:
		    WERROR("loop type %c?", optarg[2]);
		}
	    }
	    else
		sm_while_loop_code = sm_for_loop_code = code;
noselect:;
	    break;
	}
	case 'd':
	{
	    for (cp = optarg; *cp; ++cp) {
		switch(*cp) {
		/* These affect debugging level.  These values are
		** implicit here.
		**	0	DB_LEVEL2
		**	1	DB_LEVEL0
		**	>=2	DB_LEVEL1
		*/
		case 'l':	++db_linelevel; break;
		case 's':	++db_symlevel; break;
		default:
		    WERROR("-d%c?", *cp);
		}
	    }
	}
	    break;
	case 'i':			/* Set input filename. */
	    infname = optarg; break;
	case 'o':			/* Set output filename. */
	    outfname = optarg; break;
	case 'f':
	    errfname = optarg; break;
	case 'E':
	case 'P':
	    pponly = 1; break;		/* only possible if MERGED_CPP */
	default:
	    break;			/* ignore other options */
	} /* end switch */
	
#ifdef	MERGED_CPP
	/* Pass options to CPP. */
	if (PP_OPT(c, c == EOF ? argv[optind] : optarg) == PP_FAILURE)
	    return( 3 );		/* error processing args. */
#else
	if (c == EOF) {
	    /* Assume there may be further arguments, but that what
	    ** we're looking at are filename arguments.
	    */
	    if (*infname == '\0')
		infname = argv[optind];
	    else if (*outfname == '\0')
		outfname = argv[optind];
	}
#endif
#ifdef	LINT
	if (ln_opt(c, c == EOF ? argv[optind] : optarg))
	    return( 4 );
#endif
	if (c == EOF)
	    ++optind;			/* bump past current arg. */
    } /* end while */
#ifdef	MERGED_CPP			/* CPP does file handling */
    if (PP_OPT(EOF, (char *) 0) == PP_FAILURE)
	exit(5);			/* signal end of options */
    infname = fl_curname();		/* get current CPP filename */
#else
    if (*infname && freopen(infname, "r", stdin) == NULL) {
	UERROR("cannot open %s", infname);
	exit( 2 );
    }
    if (*outfname && freopen(outfname, "w", stdout) == NULL) {
	UERROR("cannot open %s", outfname);
	exit( 2 );
    }
    if (*errfname)
	infname = errfname;		/* for reporting purposes */

#endif	/* def MERGED_CPP */
#ifdef	LINT
    if (ln_opt(EOF, (char *) 0))
	return( 6 );
#endif
    if (! pponly) {
	setsig();			/* set up signal handling */
	p2init();			/* initialize CG stuff */
	tt_init();			/* initialize types */
	DB_S_FILE(infname);		/* do initial debug stuff */
	cg_filename(infname);		/* Set filename for CG. */
	er_filename(infname,1);		/* Set filename for error processing. */
    }
    return( 0 );
}


static int
process()
/* Do whatever processing is called for.  Return non-zero on error. */
{
    extern int yyparse();

#ifdef MERGED_CPP
    if (pponly) {
	lx_input();			/* preprocess, flush to output */
	return( fl_numerrors() );
    }
#endif
    if (yyparse() || nerrors)		/* quit on errors */
	return( 1 );
#ifdef MERGED_CPP
    if ( fl_numerrors() )
	return( 2 );			/* had preprocessing errors */
#endif
    return( 0 );
}

static int
cleanup()
/* Clean up after processing, check for finaly errors.
** Return non-zero on errors.
*/
{
#ifndef	LINT
    if (vstamp)
	dovstamp();			/* put out version stamp */
#endif
#ifdef	MERGED_CPP
    if (pponly)
	/* PP_CLEANUP() might produce new error messages, but not a failure. */
	return( PP_CLEANUP() == PP_FAILURE || fl_numerrors() );
#endif

    sy_clear(SL_EXTERN);		/* flush symbols at external level */
    DB_E_FILE();			/* do debug stuff for end of file */

#ifndef LINT
    if( p2done() )			/* close off CG stuff */
	/* p2done() returns non-zero if file I/O errors occurred. */
	UERROR("error writing output file");
#endif

    if (nerrors) {
#ifdef LINT
	(void) ln_cleanup();
#endif
	return( 1 );			/* could have new errors */
    }

    if (tcheck()) {
	tshow();			/* check for lost nodes */
	return( 2 );
    }
#ifdef	LINT
    if (ln_cleanup())
	return( 3 );
#endif
    return( 0 );
}

#ifndef LINT
static void
dovstamp()
/* Output version stamp into output file.  cg_ident()
** expects "-enclosed string.
*/
{
    char va[200];			/* presumed big enough */
    sprintf(va, "\"acomp: %s\"", CPL_REL);
#ifdef	MERGED_CPP
    if (pponly)
	CG_PRINTF(("#ident %s\n", va));
    else
#endif
	cg_ident(va);			/* generate .ident for this string */
    return;
}
#endif

static void
setsig()
/* catch signals if they're not now being ignored */
{
    static void catch_fpe();

    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	(void) signal(SIGHUP, earlyexit);
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	(void) signal(SIGINT, earlyexit);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	(void) signal(SIGQUIT, earlyexit);
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	(void) signal(SIGTERM, earlyexit);
    
    /* catch floating error */

#ifdef	IEEE_HOST
    /* Guarantee traps on overflow, invalid operations, divide by 0, etc. */
    (void) fpsetmask(FP_X_OFL|FP_X_INV|FP_X_DZ);
#endif
    (void) signal(SIGFPE, catch_fpe);
    return;
}

void
earlyexit()
/* Exit compiler early with error return code.  Clean up
** CG interface first.
*/
{
#ifdef LINT
    (void) ln_cleanup();
#endif
    p2abort();
    exit(10);				/* random non-0 number */
    /*NOTREACHED*/
}

/* Floating point exception handling */
static void (*fpe_func)();

void
save_fpefunc(p)
void (*p)();
/* Save the name of the fp exception handler routine. */
{
    fpe_func = p;
    return;
}

/*ARGSUSED*/
static void
catch_fpe(sig)
int sig;
/* Catch floating-point exceptions and handle them gracefully. */
{
    if (fpe_func){	/* handle floating exception as setup in optim.c */
	if (signal(SIGFPE, SIG_IGN) != SIG_IGN)		/* reset signal */
	    (void) signal(SIGFPE, catch_fpe);
	fpe_func();			/* call fpe handler */
    }
    else UERROR("floating-point constant folding causes exception");
    earlyexit();
}

