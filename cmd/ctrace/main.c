/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ctrace:main.c	1.17"
/*	ctrace - C program debugging tool
 *
 *	main function - interprets arguments
 *
 */
#include "global.h"
#include "sgs.h"		/* for the package and release variables */

/* global options */
enum	bool suppress = no;	/* suppress redundant trace output (-s) */
enum	bool pound_line = no;	/* input preprocessed so suppress #line	*/
int	tracemax = TRACE_DFLT;	/* maximum traced variables per statement (-t number) */

/* local options */
static	enum	bool Preprocessor = no;	/* run the preprocessor (-P) */
static	enum	bool octal = no;	/* print variable values in octal (-o) */
static	enum	bool hex = no;		/* print variable values in hex (-x) */
static	enum	bool unsign = no;	/* print variable values as unsigned (-u) */
static	enum	bool floating = no;	/* print variable values as float (-e) */
static	enum	bool only = no;		/* trace only these functions (-f) */
static	enum	bool basic = no;	/* use basic functions only (-b) */
static	int	loopmax = LOOPMAX;	/* maximum loop size for loop recognition (-l number) */
static	char	*print_fcn = "printf(";	/* run-time print function (-p string) */
static	char	*codefile = NULL;	/* run-time package file name (-r file) */
static  char	*compiler_name = PP_COMMAND;	/* c compiler (-n compiler_name) */


/* global data */
char	*filename = "";		/* input file name */
enum	bool trace_fcn;	   /* indicates if this function should be traced */
extern int yydebug;

/* local data */
static	int	error_code = 0;		/* exiting error code */
static	int	fcn_count = 0;		/* number of functions (-f and -v) */
static	char	**fcn_names;		/* function names (-f and -v) */
static	char	*pp_command;		/* preprocessor command (-P) */
static	char	*pp_options;		/* preprocessor options (-n) */
static	enum	bool fcn_traced = no;	/* indicates if at least one function was traced */

/* necessary for ANSI C */
static 	void	append(), runtime(), quit(), msg_header();
int	yyparse();

/* main function */

main(argc, argv)
int	argc;
char	*argv[];
{
	int	i, c;
	char    *s, *strsave();
	static char *v_stmp;
	extern char *optarg;
#ifndef __STDC__
	extern int  getopt();
#endif
	extern int  optind;

	/* set the options */
	if ((pp_options = strsave(PP_OPTIONS)) == NULL) {
		(void)fprintf(stderr, "ctrace: out of storage");
		exit(1);
	}

#ifdef TESTDEB
	yydebug = 1;
#endif

	while ( (c=getopt(argc, argv, "PVD:I:U:Q:n:oxuebsp:r:l:t:f:v:")) != EOF ) {
		switch (c) {
		case 'P':	/* run the preprocessor */
			Preprocessor = yes;
			break;
		case 'D':	/* options for preprocessor */
		case 'I':
		case 'U': {
			char *str=(char *)malloc(sizeof(char)*(strlen(optarg)+3));
			Preprocessor = yes;
			(void)sprintf(str, "-%c%s", c, optarg);
			append(&pp_options, str);
			free((char *)str);
			break;
			}
        	case 'V':
            		(void)fprintf(stderr,"ctrace: %s, %s\n", ESG_PKG, ESG_REL);
            		break;
        	case 'Q': 	/* print out the ctrace version stamp */
			v_stmp = optarg;
			if(*v_stmp != 'y' && *v_stmp != 'n') {
				(void)fprintf(stderr, "ctrace: -Q option should be followed by [y/n]\n");
				exit(1);
			} else if (*v_stmp == 'y')
				(void)printf("#ident\t\"ctrace: %s\"\n", ESG_REL);
            		break;
		case 'n':	/* another C compiler name rather than cc */
			compiler_name = optarg;
			break;
		case 'o':	/* variable output in octal */
			octal = yes;
			break;
		case 'x':	/* variable output in hex */
			hex = yes;
			break;
		case 'u':	/* variable output as unsigned */
			unsign = yes;
			break;
		case 'e':	/* print variable values as float */
			floating = yes;
			break;
		case 'b':	/* use basic functions only */
			basic = yes;
			break;
		case 's':	/* suppress redundant trace output */
			suppress = yes;
			break;
		case 'p':	/* run-time print function */
			print_fcn = optarg;
			break;
		case 'r':	/* run-time code file name */
			codefile = optarg;
			break;
		case 'f':	/* trace only these functions */
			only = yes;
			/* FALLTHRU */
		case 'v':	/* trace all but these functions */
			if (*optarg == '-') {
				(void)fprintf(stderr, "ctrace: option -%c requires function names\n", c);
				exit(1);
			}
			fcn_names = &argv[optind-1];/* get next arg */
			if( *fcn_names[0] == '-' ) {
				char *tmp_opt = strchr(argv[optind-1], '-') + 2;
				(void)strcpy(argv[optind-1], tmp_opt);
			}
			++fcn_count;
			while (optind < argc) {
				/* look for the file name or next option */
				s = argv[optind];/* get next arg */
				if (s[0] == '-' || s[strlen(s) - 2] == '.')
					break;
				if ((int)strlen(s) >= IDMAX) s[IDMAX] = '\0';
				fcn_count++;
				optind++;
			}
			break;
		case 'l':	/* options with a numeric argument */
			if ( *optarg<'0' || *optarg>'9' || (i = atoi(optarg)) < 0 ){
				(void)fprintf(stderr, "ctrace: bad numeric value to -l option\n");
				exit(1);
			} else 
				loopmax = i;
			break;
		case 't':	/* maximum traced variables per statement */
			if ((i = atoi(optarg)) <= 0 ) {
				(void)fprintf(stderr, "ctrace: bad numeric value to -t option\n");
				exit(1);
			} else {
				tracemax = i;
				if (tracemax > TRACEMAX) /* force into range */
					tracemax = TRACEMAX;
			}
			break;
		default:
			(void)fprintf(stderr, "Usage: ctrace [-beosuxPV] [C preprocessor options] [-f|v functions] [-Q(y/n)]\n");
			(void)fprintf(stderr, "              [-p string] [-r file] [-l number] [-t number] [file]\n");
			exit(1);
		}
	}
	/* open the input file */
	yyin = stdin; /* default to the standard input */
	if (argc > optind) {
		filename = argv[optind]; /* save the file name for messages */
		if ((yyin = fopen(filename, "r")) == NULL) {
			(void)fprintf(stderr, "ctrace: cannot open file %s\n", filename);
			exit(1);
		}
	}
	/* assign C compiler name and options to pp_command */
	pp_command = (char *)malloc((unsigned)(strlen(compiler_name)+strlen(pp_options)+2));
	if (pp_command == NULL) {
		(void)fprintf(stderr, "ctrace: out of storage");
		exit(1);
	}
	(void)strcpy(pp_command, compiler_name);
	(void)strcat(pp_command, " ");
	(void)strcat(pp_command, pp_options);

	/* run the preprocessor if requested */
	if (Preprocessor) {
		if (strcmp(filename, "") != 0) {
			append(&pp_command, filename);
		}
		if ((yyin = popen(pp_command, "r")) == NULL) {
			(void)fprintf(stderr, "ctrace: cannot run preprocessor");
			exit(1);
		}
		pound_line = yes;	/* suppress #line insertion */
	}
	/*
	** #define CTRACE so you can put this in your code:
	**	#ifdef CTRACE
	**		ctroff();
	**	#endif
	*/
	(void)printf("#define CTRACE 1\n");
	
	/* prepend signal.h (needed by the runtime package) so programs that
	   call signal but don't include signal.h won't get a "redeclaration
	   of signal" compiler error */
	/* prepend a #line statement with the original file name for 
	   compiler errors in programs with no preprocessor statements */
	(void)printf("#line 1 \"%s\"\n", filename);
	
	/* put the keywords into the symbol table */
	init_symtab();
	
	/* trace the executable statements */
	(void) yyparse();

	/* output the run-time package of trace functions */
	if (fcn_traced)
		runtime();

	/* final processing */
	quit();
	/* NOTREACHED */
}
/* append an argument to the command */

static void
append(s1, s2)
char	**s1, *s2;
{
	/* increase the command string size */
	if ((*s1 = realloc(*s1, (unsigned)(strlen(*s1) + strlen(s2) + 4))) == NULL) {
		(void)fprintf(stderr, "ctrace: out of storage");
		exit(1);
	}
	/* append the argument with single quotes around it to preserve any special chars */
	(void)strcat(*s1, " '");
	(void)strcat(*s1, s2);
	(void)strcat(*s1, "'");
}
/* determine if this function should be traced */

void
tr_fcn(identifier)
char	*identifier;
{
	int	i;
	
	/* set default trace mode */
	if (only)
		trace_fcn = no;
	else
		trace_fcn = yes;
	
	/* change the mode if this function is in the list */
	for (i = 0; i < fcn_count; ++i)
		if (strcmp(identifier, fcn_names[i])  == 0){
			if (only)
				trace_fcn = yes;
			else
				trace_fcn = no;
			break;
		}
	/* indicate if any function was traced */
	if (trace_fcn)
		fcn_traced = yes;
	return ;
}
/* output the run-time package of trace functions */

static void
runtime()
{
	register int	c;
	register FILE	*code;
	char	pathname[100];
	/*	char	*s;  */
	
	/* force the output to a new line for the preprocessor statements */
	(void)putchar('\n');
	
	/* if stdio.h has already been included and preprocessed then
	   just define stdout.  _iob would be defined twice if stdio.h is
	   included again */
	if (!basic) {
		if ((signal_preprocessed || syssig_preprocessed) && Preprocessor) {
			if (signal_preprocessed)
				(void)printf("#define _SIGNAL_H\n");
			if (syssig_preprocessed)
				(void)printf("#define _SYS_SIGNAL_H\n");
			(void)printf("#define SIGBUS\t10\n");
			(void)printf("#define SIGSEGV\t11\n");
		}
		if (types_preprocessed && Preprocessor)
			(void)printf("#define _SYS_TYPES_H\n");
		if (select_preprocessed && Preprocessor)
			(void)printf("#define _SYS_SELECT_H\n");
		if (timet_preprocessed && Preprocessor)
			(void)printf("#define _TIME_T\n");
		if (clockt_preprocessed && Preprocessor)
			(void)printf("#define _CLOCK_T\n");
		if (sizet_preprocessed && Preprocessor)
			(void)printf("#define _SIZE_T\n");
		(void)printf("#include <signal.h>\n\n");
	}
	if (stdio_preprocessed) {
		(void)printf("#undef stdout\n");
		(void)printf("#ifdef __STDC__\n");
		(void)printf("#\tdefine stdout\t(&__iob[1])\n");
		(void)printf("#else\n");
		(void)printf("#\tdefine stdout\t(&_iob[1])\n");
		(void)printf("#endif\n");
	}
	/* if the output is not to be buffered then include stdio.h to get the stdout symbol */
	else if (!basic)
		(void)printf("#include <stdio.h>\n");

	/* see if setjmp.h has already been included and preprocessed.
	   jmp_buf would be defined twice if it is included again */
	if (!setjmp_preprocessed && !basic)
		(void)printf("#include <setjmp.h>\n");
		
	/* make preprocessor definitions to tailor the code */
	(void)printf("#define VM_CT_ %d\n", tracemax);
	(void)printf("#define PF_CT_ %s\n", print_fcn);
	if (octal)
		(void)printf("#define O_CT_\n");
	if (hex)
		(void)printf("#define X_CT_\n");
	if (unsign)
		(void)printf("#define U_CT_\n");
	if (floating)
		(void)printf("#define E_CT_\n");
	if (basic)
		(void)printf("#define B_CT_\n");
	if (loopmax != 0)
		(void)printf("#define LM_CT_ %d\n", loopmax);
		
	/* construct the file name of the runtime trace package */
	(void)strcpy(pathname, RUNTIME);
	if (codefile != NULL) {
		(void)strcpy(pathname, codefile);
	}
	/* open the file */
	if ((code = fopen(pathname, "r")) == NULL) {
		(void)fprintf(stderr, "ctrace: cannot open runtime code file %s\n", pathname);
		exit(1);
	}
	/* output the runtime trace package */
	(void)printf("#line 1 \"%s\"\n", pathname);
	while ((c = getc(code)) != EOF) {
		(void)putchar(c);
	}
}
/* error and warning message functions */
void
fatal(text)
char *text;
{
	error(text);
	error_code = 3;
	quit();
}
void
error(text)
char *text;
{
	msg_header();
	(void)fprintf(stderr, "%s\n", text);
	error_code = 2;
}
void 
warning(text)
char *text;
{
	msg_header();
	(void)fprintf(stderr, "warning: %s\n", text);
}
static void
msg_header()
{
	(void)fprintf(stderr, "ctrace: ");
	if (strcmp(filename, "") != 0)
		(void)fprintf(stderr, "\"%s\", ", filename);
	(void)fprintf(stderr, "line %d: ", yylineno);
}
static void
quit()
{
	if (error_code > 1) {
		(void)fprintf(stderr, "ctrace: see man page Diagnostics section for what to do next\n");
	}
	exit(error_code);
}
