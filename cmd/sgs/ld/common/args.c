/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/args.c	1.27"

/*
** Module args
** process command line options
*/

/****************************************
** imports
****************************************/

#include	<stdio.h>
#ifdef	__STDC__
#include	<unistd.h>
#include	<stdlib.h>
#endif	/* __STDC__ */
#include	<fcntl.h>
#include	<string.h>
#include	"sgs.h"
#include	"paths.h"
#include	"globals.h"

extern char*
	optarg;
extern int
	optind,
	opterr;

#ifndef	__STDC__
int	getopt();
void	exit();
#endif


/****************************************
** local constants
****************************************/

/*  getopt(3) command line options */
#ifndef	DEBUG
#define	OPTIONS	"abd:e:h:I:l:mno:rstu:xz:B:GL:M:Q:VY:"
#else
#define	OPTIONS	"abd:e:h:I:l:mno:rstu:xz:B:D:GL:M:Q:VY:"
#endif

/* constants used by the usage_mess() function */
#define FULL_USAGE	0
#define SUMM_USAGE	1


/****************************************
** local variables
****************************************/

static Setstate	dflag = NOT_SET;		/* Flag saver for dmode */

static Boolean	Vflag = FALSE;
static Boolean	fileopts = FALSE;
static char*	Mflag;
static Boolean	zseen = FALSE;
static Boolean  YPflag = FALSE;


#ifdef DEBUG
static struct arg_code {
	char* dbg_arg;		/* Command line argument name */
	Dbgset dbg_code;	/* Internal code for this debugging level */
} debug_opts[] = {
	{ "args", DBG_ARGS },
	{ "files", DBG_FILES },
	{ "globals", DBG_GLOBALS },
	{ "libs", DBG_LIBS },
	{ "main", DBG_MAIN },
	{ "map", DBG_MAP },
	{ "outfile", DBG_OUTFILE },
	{ "reloc", DBG_RELOC },
	{ "sections", DBG_SECTIONS },
	{ "syms", DBG_SYMS },
	{ "update", DBG_UPDATE },
	{ "util", DBG_UTIL },
	{ NULL, DBG_MAX }
};
#endif

/****************************************
** local function declarations
****************************************/

LPROTO(void usage_mess, (int));

#ifdef DEBUG
LPROTO(void set_debug, (char*));
#endif

/****************************************
** local function definitions
****************************************/

#ifdef	DEBUG
/*
 * Turn on debugging flags from the -D option
 * Flags are of the form one,two,three, ...
 */

static void
set_debug(which)
	char* which;		/* Which debugging flags should be set */
{
	char* dflags;		/* Temporary to hold a copy of "which" */
	struct arg_code* acp;	/* Pointer to cycle through debug_opts array */

	dflags = (char*) mymalloc(strlen(which) + 1);
	(void) strcpy(dflags, which);

	/*
	 * If the option is -Dall then turn on all flags
	 */
	if (strncmp(dflags, "all", 3) == SAME) {
		int i;

		for (i = 0; i < DBG_MAX; i++)
			debug_bits[i] = TRUE;
		return;
	}

	/*
	 * Otherwise the option should be of the form -Dopt,opt,opt, ...
	 * Separate the pieces and turn on the appropriate flags
	 */
	if ((dflags = strtok(dflags, ",")) != NULL) {
		do {
			for (acp = debug_opts; acp->dbg_arg != NULL; acp++) {
				if (strcmp(dflags, acp->dbg_arg) == SAME) {
					lderror(MSG_DEBUG, "setting debug option %s",
						acp->dbg_arg);
					debug_bits[acp->dbg_code] = TRUE;
					break;
				}
			}
		} while ((dflags = strtok(NULL, ",")) != NULL);
	}
}
#endif	/* DEBUG */


/****************************************
** global function definitions
****************************************/

/*
** check_flags(int argc)
** checks the command line option flags for consistency.
*/
void
check_flags(argc)
	int	argc;		/* the usual passed from main */
{
	if(YPflag && (libdir != NULL || llibdir != NULL))
		lderror(MSG_FATAL,
			"-YP and -Y%c may not be specified concurrently",
			      libdir ? 'L' : 'U');
	if (rflag && dmode) {
		if (dflag == SET_TRUE)
			lderror(MSG_FATAL,
			    "the -dy and -r flags are incompatible");
		else 
			dmode = FALSE;
	}
	if (dmode) {
		Bflag_dynamic = TRUE;		/* on by default in dmode -- settings will be rechecked in pfiles */
		if (aflag)
		{
			lderror(MSG_FATAL,
			    "the -dy and -a flags are incompatible");
		}
		if (!Gflag)		/* dynamically linked executable */
		{
			if (znflag)
                                zdflag = FALSE;
			else if (!zdflag)
                                zdflag = TRUE;
			if (Bflag_symbolic)
			{
                                Bflag_symbolic = FALSE;
                                lderror(MSG_WARNING,
				    "-Bsymbolic ignored when building a dynamic executable");
			}
			if (dynoutfile_name != NULL)
			{
                                lderror(MSG_WARNING,
				    "-h ignored when building a static executable");
                                dynoutfile_name = NULL;
			}
		} else
			if (znflag)
                                zdflag = FALSE;
	} else /*if (!dmode)*/ {
		if (bflag){
			lderror(MSG_WARNING,
			    "the -dn and -b flags are incompatible; ignoring -b");
			bflag = FALSE;
		}
		if (dynoutfile_name != NULL){
			lderror(MSG_WARNING,
			    "-dn and -h are incompatible; ignoring -h");
			dynoutfile_name = NULL;
		}
		if (!zdflag && !rflag)
			zdflag = TRUE;
		if (ztflag){
			lderror(MSG_WARNING,
			    "the -dn and -ztext flags are incompatible; ignoring -ztext");
			ztflag = FALSE;
		}
		if (Bflag_dynamic)
			lderror(MSG_FATAL,
			    "the -dn and -Bdynamic flags are incompatible");
		if (Gflag)
			lderror(MSG_FATAL,
			    "-dn and -G flags are incompatible");
		if(aflag && rflag)
			lderror(MSG_FATAL,
			    "-a -r is illegal in this version of ld");
		/* aflag should be on by default, but is off by
		 * default if rflag is on
		 */
		if (!aflag && !rflag)
			aflag = TRUE;
		if (rflag ){
			/* we can only strip the symbol table and string table
			 * if no output relocations will refer to them
			 */
			if(sflag){
				lderror(MSG_WARNING,
				    "-r and -s both set; only debugging information stripped");
			}
			if(!aflag && interp_path != NULL){
				lderror(MSG_WARNING,
				    "-r and -I flags are incompatible;  -I ignored");
				interp_path = NULL;
			}
		}
	}
	if (Mflag != NULL && dmode)
		lderror(MSG_FATAL,"-M and -dy are incompatible");
	else if (Mflag != NULL && !aflag)
		lderror(MSG_FATAL,"-M illegal when not building a static executable  file");

	if(!fileopts){
		if(Vflag && argc == 2)
			exit(EXIT_SUCCESS);
		else
			lderror(MSG_FATAL,
				"no files on input command line");
	}
	ecrit_setup();
	if (Mflag != NULL)
		map_parse(Mflag);
}

/*
** process_flags(int argc, char** argv)
** processes the command line flags.  Argc and argv are the usual
** stuff passed to main.
**
** Even though the ld specifications say that we should not allow
** general options to be processed after seeing the first file option
** (-Bdynamic, -Bstatic, -lx), in the interest of backward
** compatibility with old makefiles, etc., we will allow all old options
** except -t to appear anywhere in the command line.  NOTE: all new
** makefiles should follow the requirements for command line ordering
** indicated above.
**
** The options recognized by ld are the following (initialized in
** globs.h):
**
**	OPTION		MEANING
**	-a { -dn only }	make the output file executable
**				1. complain about unresolved
**				   references (zdefs on)
**				2. define several "_xxxx" symbols
**
**	-b { -dy only }	turn off special handling for PIC/non-PIC
**			  relocations
**
**	-dy		dynamic mode: build a dynamically linked
**			  executable or a shared object - build a
**			  dynamic structure in the output file and
**			  make that file's symbols available for
**			  run-time linking
**
**	-dn		static mode: build a statically linked
**			  executable or a relocatable object file
**
**	-e name		make name the new entry point in
**			  ELF_PHDR.p_entry
**
**	-h name { -dy -G only }
**			make name the new output filename in the
**			  dynamic structure
**
**	-I name		make name the interpreter pathname written
**			  into the program execution header of the
**			  output file
**
**	-lx		search for the library libx.[so|a] using
**			  search directories
**
**	-m		generate a memory map
**
**	-n		included for backward compatiblility with old makefiles
**
**	-o name		use name as the output filename (default
**			  is in A_OUT in globs.c
**
**	-r { -dn only }	retain relocation in the output file -
**			  produce a relocatable object file
**
**	-s		strip the debug section and its relocations
**			  from the output file
**
**	-t		turnoff warnings about multiply-defined
**			  symbols that are not the same size
**
**	-u name		make name an undefined entry in the ld symbol
**			  table
**
**	-x		included for backward compatiblility with old makefiles
**
**	-z text { -dy only }	issue a fatal error if any text relocations remain
**		
**	-z defs		issue a fatal error if undefined symbols remain 
**		{ -dn }	forced on
**		{ -dy -G }
**			forced off
**		{ -dy !-G }
**			forced on
**
**	-z nodefs 	undefined symbols are allowable
**		  { -dn }
**			forced off
**		  { -dy -G }
**			forced on
**		  { -dy !-G }
**			forced off
**
**	-B static	in searching for libx, choose libx.a
**
**	-B dynamic { -dy only }
**			in searching for libx, choose libx.so
**
**	-B symbolic { -dy -G }
**			shared object symbol resolution flag ...
**
**	#ifdef	DEBUG
**	-D sect,sect,...
**			turn on debugging for each indicated section
**	#endif
**
**	-G { -dy }	produce a shared object
**
**	-L path		add path to prelibdirs
**
**	-M mapfilename	read a mapfile
**
**	-Qy		add ld version to comment section of output
**			  file
**
**	-Qn		do not add ld version
**
** 	-V		print ld version to stderr
**
**	-YL path	change LIBDIR to path
**
**	-YU path	change LLIBDIR to path
**
**
** Pass 1 -- process_flags: collects all options and sets flags
** check_flags -- checks for flag consistency
** Pass 2 -- process_files: skips the flags collected in pass 1 and processes files
*/

void
process_flags(argc, argv)
	int	argc;		/* the usual passed in from main() */
	char**	argv;
{
	Boolean	errflag = FALSE; /* an error has been seen */
	int	c;		/* character returned by getopt */

	if (argc < 2) {
		errflag = TRUE;
		goto usage;
	}
   getmore:
	while((c = getopt(argc, argv, OPTIONS)) != -1 ){
	DPRINTF(DBG_ARGS, (MSG_DEBUG, "args: process_flags: argc=%d, optind=%d",argc,optind));
	DPRINTF(DBG_ARGS, (MSG_DEBUG, "input argument to process_flags is: %#x",c));

		switch (c) {

		case 'a':
			aflag = TRUE;
			break;

		case 'b':
			bflag = TRUE;
			break;

		case 'd':
			if (optarg[0] == 'n' && optarg[1] == '\0') {
				if (dflag == NOT_SET)
				{
					dmode = FALSE;
					dflag = SET_FALSE;
				} else
					lderror(MSG_WARNING,
					    "-d used more than once");
			} else if (optarg[0] == 'y' && optarg[1] == '\0') {
				if (dflag == NOT_SET)
                                {
                                        dmode = TRUE;
                                        dflag = SET_TRUE;
                                } else
                                        lderror(MSG_WARNING,
                                            "-d used more than once");
			} else {
				lderror(MSG_FATAL, "illegal -d %s option", optarg);
			}
			break;

		case 'e':
			if (entry_point != NULL) 
				lderror(MSG_WARNING,
				    "-e specifies multiple program entry points");
			entry_point = optarg;
			break;

		case 'h':
			if (dynoutfile_name != NULL)
                                lderror(MSG_WARNING,
				    "-h specifies multiple output filenames for dynamic structure");
                        dynoutfile_name = optarg;
			break;

		case 'I':
			if (interp_path != NULL)
                                lderror(MSG_WARNING,
				    "-I specifies multiple interpreter paths");			
                        interp_path = optarg;
			break;

		case 'l':
			fileopts = TRUE;
			break;

		case 'm':
			mflag = TRUE;
			break;

		case 'n':
			break;

		case 'o':
			if (memcmp(outfile_name,A_OUT,sizeof(A_OUT)))
                                lderror(MSG_WARNING,
				    "-o specifies multiple output file names ");
			outfile_name = optarg;
			break;
		case 'r':
			if (rflag)
				lderror(MSG_WARNING,
					"-r used more than once");
			rflag = TRUE;
			break;

		case 's':
			sflag = TRUE;
			break;

		case 't':
			tflag = TRUE;
			break;

		case 'u':
			add_usym(optarg);
			break;

		case 'x':
			lderror(MSG_WARNING,"-x behavior is obsolete");
			break;

		case 'z':
			if (strncmp(optarg, "defs", 4) == SAME) {
				if (zseen)
					lderror(MSG_WARNING,"-zdefs/nodefs appears more than once: last taken");
				else
					zseen = TRUE;
				znflag = !(zdflag = TRUE);
			} else if (strcmp(optarg, "nodefs") == SAME) {
				if (zseen)
					lderror(MSG_WARNING,"-zdefs/nodefs appears more than once - last taken");
				else
					zseen = TRUE;
                                zdflag = !(znflag = TRUE);
			} else if (strncmp(optarg, "text", 4) == SAME) {
				ztflag = TRUE;
			} else {
				lderror(MSG_WARNING, "illegal -z %s option", optarg);
			}
			break;

#ifdef	DEBUG
		case 'D':
			set_debug(optarg);
			break;
#endif

		case 'B':
			if (strcmp(optarg, "dynamic") == SAME) {
				Bflag_dynamic = TRUE;
			} else if (strcmp(optarg, "static") == SAME) {
				Bflag_dynamic = FALSE;
			} else if (strcmp(optarg, "symbolic") == SAME) {
				Bflag_symbolic = TRUE;
			} else {
				lderror(MSG_FATAL, "illegal -B %s option", optarg);
			}
			break;

		case 'G':
			Gflag = TRUE;
			break;

		case 'L':
			break;

		case 'M':
			if (Mflag != NULL)
				lderror(MSG_FATAL,
					"more than one mapfile specified");
			Mflag = optarg;
			break;

		case 'Q':
			if (strcmp(optarg,"y") == SAME)
                                if (Qflag == NOT_SET)
					Qflag = SET_TRUE;
				else
					lderror(MSG_WARNING,
					    "-Q appears more than once; first setting retained");
			else
				if (strcmp(optarg,"n") == SAME)
					if (Qflag == NOT_SET)
						Qflag = SET_FALSE;
                                else
                                        lderror(MSG_WARNING,
                                            "-Q appears more than once; first setting retained");
			else 
				lderror(MSG_WARNING,"bad argument to -Q flag, ignored");
			break;

		case 'V':
			if(!Vflag)
				fprintf(stderr,"%sld: %s %s\n",SGS,CPL_PKG,CPL_REL);
			Vflag = TRUE;
			break;

		case 'Y':
			if (strncmp(optarg, "L,", 2) == SAME) {
                             	libdir = optarg+2;
			} else if (strncmp(optarg, "U,", 2) == SAME) {
                             	llibdir = optarg + 2;
			} else if(strncmp(optarg, "P,",2) == SAME) {
				YPflag = TRUE;
				libpath = optarg+2;
			} else {
				lderror(MSG_FATAL, "illegal -Y %s option", optarg);
			}
			break;

		case '?':
			usage_mess(FULL_USAGE);
			exit(EXIT_FAILURE);

		default:
			break;
		}		/* END: switch (c) */
	}
	for(;optind < argc; optind++){
		if(argv[optind][0] == '-')
			goto getmore;
		fileopts = TRUE;
		DPRINTF(DBG_ARGS,(MSG_DEBUG,"args: got a file argument %s", argv[optind]));
	}
 usage:
	if (errflag) {
		usage_mess(SUMM_USAGE);
		exit(EXIT_FAILURE);
	}
}



/*
** process_files(int argc, char** argv);
** processes the command line files.  Argc and argv are the usual
** stuff passed to main.
**
** Pass 1 -- process_flags: collects all options and sets flags
** check_flags -- checks for flag consistency
** Pass 2 -- process_files: skips the flags collected in pass 1 and processes files
*/

void
process_files(argc, argv)
	int	argc;
	char**	argv;

{
	int	c;		/* character returned by getopt */

	optind = 1;		/* reinitialize optind */

        if (argc < 2) {
		usage_mess(SUMM_USAGE);
		exit(EXIT_FAILURE);
	}
    getmore:
        while ((c = getopt(argc, argv, OPTIONS)) != -1){
	DPRINTF(DBG_ARGS, (MSG_DEBUG, "input argument to process_files is: %#x",c));

		switch (c) {
			case 'l':
				find_library(optarg);
				break;
			case 'B':
				if (strcmp(optarg, "dynamic") == SAME){
					if (dmode)
	                                	Bflag_dynamic = TRUE;
					else
						lderror(MSG_FATAL,
							"the -dn and -Bdynamic flags are incompatible");
				}else if (strcmp(optarg,"static") == SAME)
	                                Bflag_dynamic = FALSE;
				break;
			case 'L':
				add_libdir(optarg);
				break;
			default:
				break;
			}
		}
       for(;optind < argc; optind++){
		if(argv[optind][0] == '-')
                        goto getmore;
                cur_file_name = argv[optind];
		if ( (cur_file_fd = open(cur_file_name,O_RDONLY)) == -1)
			lderror(MSG_FATAL,"cannot open file for reading");
		process_infile(cur_file_name);
		DPRINTF(DBG_ARGS,(MSG_DEBUG,"args: got a file argument %s", argv[optind]));
	}
}

/* print usage message to stderr - 2 modes, summary message only,
 * and full usage message
 */
static void
usage_mess(mode)
int mode;
{
	fprintf(stderr,"usage: %sld [-abmrstGVd:e:h:l:o:u:z:B:I:L:M:Q:Y:] file(s) ...\n",SGS);

	if (mode == SUMM_USAGE)
		return;

	fprintf(stderr,"\t[-a create absolute file]\n");
	fprintf(stderr,"\t[-b do not do special PIC relocations in a.out]\n");
	fprintf(stderr,"\t[-m print memory map]\n");
	fprintf(stderr,"\t[-r create relocatable file]\n");
	fprintf(stderr,"\t[-s strip symbol and debugging information]\n");
	fprintf(stderr,"\t[-t do not warn for multiply defined symbols of different sizes]\n");
	fprintf(stderr,"\t[-G create shared object]\n");
	fprintf(stderr,"\t[-V print version information]\n");
	fprintf(stderr,"\t[-d y|n operate in dynamic|static mode]\n");
	fprintf(stderr,"\t[-e sym use sym as starting text location]\n");
	fprintf(stderr,"\t[-h name use name as internal shared object string]\n");
	fprintf(stderr,"\t[-l x search for libx.so or libx.a]\n");
	fprintf(stderr,"\t[-o outfile name output file outfile]\n");
	fprintf(stderr,"\t[-u symname create undefined symbol symname]\n");
	fprintf(stderr,"\t[-z defs|nodefs disallow|allow undefined symbols]\n");
	fprintf(stderr,"\t[-z text disallow output relocations against text]\n");
	fprintf(stderr,"\t[-B dynamic|static search for shared libraries|archives]\n");
	fprintf(stderr,"\t[-B symbolic bind external references to definitions\n\t\t when creating shared objects]\n");
	fprintf(stderr,"\t[-I interp use interp as path name of interpreter]\n");
	fprintf(stderr,"\t[-L path search for libraries in directory path]\n");
	fprintf(stderr,"\t[-M mapfile use processing directives contained in mapfile]\n");
	fprintf(stderr,"\t[-Q y|n do|do not place version information in output file]\n");
	fprintf(stderr,"\t[-YP,dirlist use dirlist as default path when searching for libraries]\n");
	return;
}
