/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)size:size/common/main.c	1.13"
/* UNIX HEADERS */
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>

/* ELF HEADERS */
#include	"libelf.h"

#ifdef i386
#include <sys/elf_386.h>
#endif

#ifdef M32
#include <sys/elf_M32.h>
#endif

/* SGS SPECIFIC HEADER */
#include	"sgs.h"

/* SIZE HEADER */
#include	"defs.h"

/* EXTERNAL VARIABLES DEFINED */
int		fflag = 0,	/* print full output if -f option is supplied */
		Fflag = 0,	/* print full output if -F option is supplied */
		nflag = 0;	/* include NOLOAD sections in size if -n option
				 * is supplied */
int		numbase = DECIMAL;
static int	errflag = 0;	/* Global error flag */
int		oneflag = 0;
int		exitcode = 0;   /* Global exit code */
char		*fname;
char		*archive;
int		is_archive = 0;

static char	*tool_name;

static void	usagerr();
extern int	close();

#define OPTSTR "VoxnfF"	/* option string for usage error message */
#define GETOPTSTR "VoxnfF?"	/* option string for getopt */

static Elf	*elf;
static Elf_Arhdr	*arhdr;



    /*
     *  main(argc, argv)
     *
     *  parses the command line
     *  opens, processes and closes each object file command line argument
     *
     *  defines:
     *      - int	numbase = HEX if the -x flag is in the command line
     *				= OCTAL if the -o flag is in the command line
     *				= DECIMAL if the -d flag is in the command line
     *
     *  calls:
     *      - process(filename) to print the size information in the object file
     *        filename
     *
     *  prints:
     *      - an error message if any unknown options appear on the command line
     *      - a usage message if no object file args appear on the command line
     *      - an error message if it can't open an object file
     *	      or if the object file has the wrong magic number
     *
     *  exits 1 - errors found, 0 - no errors
     */


main(argc, argv)

int	argc;
char	**argv;

{
    /* UNIX FUNCTIONS CALLED */
    extern	void	exit( );
    extern 	int	getopt();
    extern	void	error();

    /* SIZE FUNCTIONS CALLED */
    extern	void	process( );

    /* EXTERNAL VARIABLES USED */
    extern int		numbase;
    extern int		errflag;
    extern int		oneflag;
    extern int		optind;
    extern char		*fname;

    int c;
    static int	fd;
    extern char		*archive;
    Elf_Cmd		cmd;
    Elf			*arf;
    unsigned		Vflag=0;

    tool_name = argv[0];

	while ((c = getopt(argc, argv, GETOPTSTR)) != EOF) {
		switch (c) {

			case 'o':
			    	if (numbase != HEX)
			    	numbase = OCTAL;
			    	else (void) fprintf(stderr, "%ssize: -x set, -o ignored\n",SGS);
			    	break;

			case 'd':
			    	numbase = DECIMAL;
			    	break;

			case 'x':
				if (numbase != OCTAL)
				numbase = HEX;
				else (void)fprintf(stderr, "%ssize: -o set, -x ignored\n",SGS);
				break;

			case 'f':
				fflag++;
				break;

			case 'F':
				Fflag++;
				break;

			case 'n':
				nflag++;
				break;
			case 'V':
			    	(void) fprintf(stderr,"%ssize: %s %s\n",SGS,PLU_PKG,PLU_REL);
			    	Vflag++;
			    	break;
			case '?':
				errflag++;
				break;
			default:
			    	break;
		    
		
		}
	    } 
	if (errflag || (optind >= argc)) 
	{
		if (! (Vflag && (argc ==2) && !errflag) )
		{
			usagerr();
		}
	}

	
	if ( (argc - optind) == 1 )
	{
		oneflag++;			/*only one file to process*/
	}

	if (elf_version (EV_CURRENT) == EV_NONE){
		error(fname,"Libelf is out of date");
		exit(FATAL);				/*library out of date*/
	}

	for(; optind < argc; optind++){
	      if ((fd = open(argv[optind],O_RDONLY)) == -1)
			{
	     		fname = argv[optind];
			error(fname,"cannot open");
			}
	else{

	cmd = ELF_C_READ;
	arf = 0;

	if ((arf = elf_begin (fd, cmd, arf)) == 0)
	{
		/*error(fname, "cannot open");*/
		(void)fprintf(stderr, "%ssize: %s: %s\n", SGS, fname, elf_errmsg(-1));
		return(FATAL);
	}

	else if (elf_kind(arf) == ELF_K_COFF)
	{
		fname = argv[optind];
		(void)fprintf(stderr, "%s: %s: Warning - internal conversion of COFF file to ELF\n", tool_name, fname);
		/*elf_update(arf, ELF_C_NULL);*/
	}

	if (elf_kind(arf) == ELF_K_AR){
		archive = argv[optind];
		fname = argv[optind];
	}

	else {
	     archive="";
	     fname = argv[optind];
	}

	while (( elf = elf_begin(fd, cmd, arf )) != 0 )
	{
	    if ((arhdr = elf_getarhdr(elf))==0)
	    {
		if (elf_kind(arf) == ELF_K_NONE)
		{
			(void)fprintf(stderr, "%s: %s: invalid file type\n", tool_name, fname);
			exitcode++;
			break;
		}
		else
		{
			process(elf);
		}
	    }
	    else if ( arhdr->ar_name[0] != '/' )
	    {
		fname = arhdr->ar_name;
		if (elf_kind(arf) == ELF_K_NONE)
		{
			(void)fprintf(stderr, "%s: %s[%s]: invalid file type\n", tool_name, archive, fname);
			exitcode++;
			break;
		}
		else
		{
			if (elf_kind(elf) == ELF_K_COFF)
			{
				(void)fprintf(stderr, "%s: %s: Warning - internal conversion of COFF file to ELF\n", tool_name, fname);
				/*elf_update(elf, ELF_C_NULL);*/
			}
			is_archive++;
			process(elf);
		}
	    }
	cmd = elf_next(elf);
	elf_end(elf);
	}
	elf_end(arf);
	(void)close(fd);
	}
    }
    if (exitcode)
	exit(FATAL);
    else
	exit(0);

    return 0;
}
static void
usagerr()
	{

	(void)fprintf(stderr,"usage: %s [-%s] file(s)...\n", tool_name,OPTSTR);
	exitcode++;
	}
