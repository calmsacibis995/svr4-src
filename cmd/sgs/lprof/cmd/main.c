/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/main.c	1.16.1.3"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

/*
 *      Line-by-line execution count and coverage report generator
 *
 *      Usage:
 *
 *	lprof -m dfile1,dfile2[,dfile(n)] -d destfile [-T]
 *	lprof [-psTx] [-I incdir] [-r srcfile] [-c covfile] [-f func] [-o prog]
 *
 *      where "prog" is object file that was profiled
 *
 *      Options:
 *      
 *      -m      merge data files, output to destfile
 *              (-m must appear with -d option)
 *	-T	time stamp override
 *      -s      print summary information
 *      -p      (default) print listing and coverage information
 *	-r	produce listings for srcfiles only
 *		(default is all source files in object file that
 *		were compiled with coverage option)
 *	-c	use covfile instead of prof.cov for data
 *		-f OPTION CURRENTLY NOT IMPLEMENTED
 *	-f	produce information for these functions only
 *	-o	use this object file (if different path)
 *	-I	look in directory
 *	-l	lprof-style: display numbered lines with execution counts.
 *	-x	xprof-style: mark (U) lines Not covered instead of counts.
 *      
 *      Note that -d must be accompanied by -m, and that -d may appear
 *      before -m.  Note too that -l and -x are mutually exclusive, and
 *	that the last one (of those two) specified overrides any others.
 *
 */

#include <stdio.h>
#include "sgs.h"
#include "glob.h"
#include "env.h"
#include "coredefs.h"
#include "retcode.h"


#define XCMD	'x'
#define LCMD	'l'
#define XSUFF	".cov"
#define LSUFF	".cnt"

/* Bit macro and flag bit definitions */

#define FBIT(pos)       (01 << (pos))   /* Returns value with bit pos set */
#define F_LIST          FBIT(0)         /* Set if "-p" seen */
#define F_LINE		FBIT(1)		/* Set if "-l" seen */
#define F_MERGE         FBIT(2)         /* Set if "-m" seen */
#define F_MDEST		FBIT(3)         /* Set if "-d" seen */
#define F_SUM           FBIT(4)         /* Set if "-s" seen */
#define F_DATA          FBIT(5)         /* Set if "-c" seen */
#define F_TSO		FBIT(6)		/* Set if "-T" seen */
#define F_INC		FBIT(7)		/* Set if "-I" seen */

int flags;      /* for options */
int set_flag;	/* "s" (set) option to "-e" */
int reset_flag;	/* "r" (reset) option to "-e" */

extern int _tso_flag; 	/* time stamp override flag for covmerg */
extern int _com_flag;	/* complete override option for covreloc */

short lxprint;
short VwasSpecified=0;	/* 1 if -V was specified */

extern char *optarg;
extern int optind;

static char cmd_name;
void exit();

int	c_size = 1,		/* Size of dynamic arrays containing names */
	f_size = FNC_SZ,	/* input on command line */
	s_size = SOURC_SZ,
	i_size = INC_SZ;


/*  more_space

	Macro performs a check and allocates more space if needed for 
	the dynamic arrays that contain names input on the command line
	for the following items:  coverage files, function names,
	source files, and include directories.  The command line
	options are:  -m, -f, -r, and -I, respectively.  The arrays are
	a subset of the "command" structure defined in the file env.h.	
	The size of a chunk allocated for each array is defined in glob.h.
									*/
#define more_space(ptr, next, fullsize, chunksize) if (next > (fullsize -1)) \
	 { \
	    	fullsize += chunksize; \
	    	if ((ptr = (char **)realloc ((char *)ptr, sizeof(char *) * (fullsize))) == NULL) \
	    	{ \
		    fprintf (stderr,"ERROR:  Out of space\n"); \
		    exit(1); \
	    	} \
	 }

/*  init_array

	Initialize the arrays in the "command" structure that contain names 
	input on the command line.  Note:  when -m is not used as a
	command line option, this routine will be called to create
	a cov_ptr with size one (only one coverage data file may be
	used for a given call of lprof).
								*/

#define init_array(ptr, size) \
	if ((ptr = (char **)calloc (size, sizeof(char *))) == NULL) \
	    { \
		fprintf(stderr,"ERROR:  Out of space"); \
		exit(1); \
	    } \

main(argc, argv)
int argc;
char **argv;
{
    struct command cmd;
    char *suffix;
    int i; 
    char *basename();
    char *strcat();
    char *strcpy();

    cmd_name = LCMD; /* Only lprof will be kept now */
    suffix = LSUFF;
    lxprint = LOPT; /* print like lprof unless -x option specified */

    init_cmd(&cmd); 

    /* process options */
    /* add f to string when -f is implemented */
    while ((i = getopt(argc, argv, "spxlI:r:c:o:m:d:TV")) != EOF) {
        switch (i) {
            case 's':   /* print summary info */
                flags |= F_SUM;  /* Set flag */
                break;
            case 'p':   /* print listing (default) */
                flags |= F_LIST;  /* Set flag */
                break;
	    case 'I':	/* include directories */
		flags |= F_INC;	  /* Set flag */

		/* Make sure inc array has enough space */
		more_space(cmd.incdir_ptr, cmd.inc_next, i_size, INC_SZ);
		cmd.incdir_ptr[cmd.inc_next++] = optarg;
		break;
	    case 'r':	/* use source files */
		/* Check source file name array for space and input first name */
		more_space(cmd.sourc_ptr, cmd.sourc_next, s_size, SOURC_SZ);
		cmd.sourc_ptr[cmd.sourc_next++] = optarg;

		/* Input rest of source file names on the command line */
		while ((optind < argc) && (argv[optind][0] != '-'))  {
		    more_space(cmd.sourc_ptr, cmd.sourc_next, s_size, SOURC_SZ);
		    cmd.sourc_ptr[cmd.sourc_next++] = argv[optind++];
		}
		break;
	/* -f OPTION NOT IMPLEMENTED 
	    case 'f':
		more_space(cmd.fnc_ptr, cmd.fnc_next, f_size, FNC_SZ);
		cmd.fnc_ptr[cmd.fnc_next++] = optarg;

		while ((optind < argc) && (argv[optind][0] != '-'))  {
		    more_space(cmd.fnc_ptr, cmd.fnc_next, f_size, FNC_SZ);
		    cmd.fnc_ptr[cmd.fnc_next++] = argv[optind++];
		}
		break;
	*/
            case 'c':   /* get name of .cov file */
                flags |= F_DATA;  /* Set flag */
                cmd.cov_ptr[0] = optarg;
		cmd.cov_next = 1;
                break;
	    case 'o':	/* get name of obj file */
		cmd.obj_ptr = optarg;
		break;
            case 'm':  /* merge data files */
                flags |= F_MERGE;  /* Set flag */

		/* Make sure there is enough space in coverage name file array */ 
		more_space(cmd.cov_ptr, cmd.cov_next, c_size, COV_SZ);
		cmd.cov_ptr[cmd.cov_next++] = optarg; 	/* Input first data file */

		/* Read all remaining data files on command line into array */
		while ((optind < argc) && (argv[optind][0] != '-'))  {
		    more_space(cmd.cov_ptr, cmd.cov_next, c_size, COV_SZ);
		    cmd.cov_ptr[cmd.cov_next++] = argv[optind++]; 
		}
                break;
            case 'd':  /* output of merge */
                flags |= F_MDEST;  /* Set flag */
                /* get name of destination file */
		cmd.dest_ptr = optarg;
                break;
	    case 'T':	/* time stamp override */
		flags |= F_TSO;	 /* Set flag */
		_tso_flag = 1;
		break;
	    case 'x':
		lxprint = XOPT;
		break;
	    case 'l':
		lxprint = LOPT;
		break;
	    case 'V':
		(void) fprintf(stderr, "lprof: %s %s\n", CPPT_PKG, CPPT_REL);
		VwasSpecified=1;
		break;
            case '?':   /* unrecognized option */
                usage();
        }  /* end switch */
    } /* end while */

    /* if -V the only argument, just exit. */
    if (VwasSpecified && argc==2 && !flags )
	exit(1);

#if DEBUG
    pflags(&cmd);
    if (cmd.obj_ptr != NULL)
	printf("aout = %s\n", cmd.obj_ptr);
#endif

    /* check to see that if -d present, so is -m */
    if ( (flags & F_MDEST) && (!(flags & F_MERGE)) ) {
        fprintf(stderr, "%cprof: must have -m option with -d option\n", cmd_name);
        exit(1);
    }

    /* if -m and no -d, also error */
    if ( (flags & F_MERGE) && (!(flags & F_MDEST)) ) {
        fprintf(stderr, "%cprof: must have -d option with -m option\n", cmd_name);
        exit(1);
    }

    if ((flags & F_TSO) && !(flags & F_MERGE)) {
	fprintf(stderr,"%cprof: must have -m option with -T option\n", cmd_name);
	exit(1);
    }

    if (optind < argc) {
	/* either missing mfiles, or missing -d */
	fprintf(stderr, "%cprof: argument mismatch\n", cmd_name);
	usage();
    }
    if(!(flags & F_MERGE) && !(flags & F_SUM) && !(flags & F_LINE))
        flags |= F_LIST;  /* Set default flag */
    if (!(flags & F_DATA) && !(flags & F_MERGE)) {
	if (cmd.obj_ptr != NULL)	/* Object file name available */
	    cmd.cov_ptr[0] = basename(cmd.obj_ptr);
	else
	    cmd.cov_ptr[0] = basename("a.out");	/* Use default object file name */
	strcat(cmd.cov_ptr[0], suffix);
	cmd.cov_next = 1;
#if DEBUG
	printf("profout = %s\n", cmd.cov_ptr[0]);
#endif
    }

    if (flags & F_MERGE) {
	if (cmd.cov_next < 2) {
	    fprintf(stderr, "lprof: Merge (-m) needs at least two files.\n");
	    exit(1);
	}
	for (i = 0; i < cmd.cov_next; i++) {
	    int j;

	    for (j = i+1; j < cmd.cov_next; j++) {
		if (strcmp(cmd.cov_ptr[i],cmd.cov_ptr[j]) == 0) {
		    fprintf(stderr, "lprof: Merge (-m) does not allow duplicate names.\n");
		    exit(1);
		}
	    }
	}
	CAmerge(&cmd);
    }

    else {
	if (flags & F_SUM) {
	    CAreport(SUM,&cmd);
	}

	if (flags & F_LINE) {
	    CAreport(LINE,&cmd);
	}

	if (flags & F_LIST) {
	    if (cmd.fnc_next == 0)
		CAreport(LISTALL,&cmd);
	    else
		CAreport(LIST,&cmd);
	}
    }
}

Ferror()
{
    fprintf(stderr,"%cprof:  Illegal combination of options\n", cmd_name);
    usage();
}

usage()
{
   fprintf(stderr,
      "Usage:	lprof [-V] -m dfile1 dfile2[ dfile(n)] -d destfile [-T]\n"
   );
   fprintf(stderr,
      "       lprof [-Vsxlp] [-I incdir] [-r srcfile] [-c cntfile] [-o prog]\n"
   );
   exit(1);
}

char *
basename(file)
char *file;
{
    char *base, *covfile;
    char *strrchr();
    char *malloc();
    char *strcpy();

    base = strrchr(file, '/');
    if (base == NULL)
	base = file;
    else
	base++;
    /* allocate + 5 since at most ".cov" and null */
    covfile = malloc((unsigned)strlen(base)+5);
    strcpy(covfile, base);
    return(covfile);
}

/*  init_cmd

	Initialize "command" structure in env.h.  Structure contains items
	input on the command line.
									*/
init_cmd(cmd)
struct command *cmd;
{
    int i;

    cmd->tsh = 0;
    cmd->tsh_level = NULL;
    cmd->cov_reqst = 1;

    cmd->dest_ptr = NULL;

    cmd->obj_ptr = NULL; 

    /* Dynamic array indices */
    cmd->cov_next = 0;
    cmd->sourc_next = 0;
    cmd->fnc_next = 0;
    cmd->inc_next = 0;

    /* Initialize dynamic arrays with init_array macro */
    	/* more_space will be called to make the cov_ptr array larger if -m */
	/*  option encountered on command line */
    init_array(cmd->cov_ptr,c_size); 
    init_array(cmd->sourc_ptr,s_size);
    init_array(cmd->fnc_ptr,f_size);
    init_array(cmd->incdir_ptr,i_size);

    /* initialize globals */
    set_flag = 1;
    reset_flag = 0;
    _com_flag = 0;
    _tso_flag = 0;
}

#if DEBUG
/* print which options are on */
pflags(cmd)
struct command *cmd;
{
    int i;

    if (flags & F_MERGE) {
        printf("-m option\n");
        printf("    mfiles =");
	for (i = 0; i < cmd->cov_next; i++)
	    printf("%s ", cmd->cov_ptr[i]);
	printf("\n");
    }
    if (flags & F_MDEST) {
	printf("-d option\n");
	printf("    dfile = %s\n", cmd->dest_ptr);
    }
    if (flags & F_TSO) printf("-T option\n");
    if (flags & F_SUM) printf("-s option\n");
    if (flags & F_LIST) printf("-p option\n");
    if (flags & F_LINE) printf("-l option\n");
    if (flags & F_DATA) {
        printf("-c option\n");
        printf("    profout = %s\n", cmd->cov_ptr[0]);
    }
    if (flags & F_INC) {
	printf("incdirs = ");
	for (i = 0; i < INC_SZ; i++)
	    printf("%s ", cmd->incdir_ptr[i]);
	printf("\n");
    }
    if (cmd->sourc_next > 0) {
	printf("srcfiles = ");
	for (i = 0; i < cmd->sourc_next; i++)
	    printf("%s ", cmd->sourc_ptr[i]);
	printf("\n");
    }
    if (cmd->fnc_next > 0) {
	printf("funcs = ");
	for (i = 0; i < cmd->fnc_next; i++)
	    printf("%s ", cmd->fnc_ptr[i]);
	printf("\n");
    }
}
#endif
