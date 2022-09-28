/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/pass0.c	1.13"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <paths.h>
#include "sgs.h"
#include "gendefs.h"
#include "symbols.h"
#include <sys/stat.h>
#include <ccstypes.h>

/*
 *
 *	"pass0.c" is a file containing the source for the setup program
 *	for the SGS Assemblers.  Routines in this file parse the
 *	arguments, determining the source and object file names, invent
 *	names for temporary files, and execute the first pass of the
 *	assembler.
 *
 *	This program can be invoked with the command:
 *
 *			as [flags] ifile [-o ofile]
 *
 *	where [flags] are optional user-specified flags,
 *	"ifile" is the name of the assembly languge source file
 *	and "ofile" is the name of a file where the object program is
 *	to be written.  If "ofile" is not specified on the command line,
 *	it is created from "ifile" by the following algorithm:
 *
 *	     1.	If the name "ifile" ends with the two characters ".s",
 *		these are replaced with the two characters ".o".
 *
 *	     2.	If the name "ifile" does not end with the two
 *		characters ".s" the name "ofile" is created by 
 *		appending the characters ".o" to the name "ifile".
 *
 *
 *	The global array "filenames" is used to store all of the file names:
 *	filenames[0]		ifile
 *	filenames[1]		ofile
 *	filenames[2]		temp file 1
 *
 */

#define NO		0
#define YES		1
#define	MAXFLAGS	16 - 9	/* sizeof(xargp)-num of args required for pass1 */

short passnbr = 0;


short	spanopt = YES,		/* span-dependent optimizations */
	transition = NO,	/* no translation of COFF-based code */
	workaround = YES;


#if	M32
extern int need_mau;
extern short wflag;		/* warn about implicit use of 4th MAU register */
#endif
#if STATS
char * firstbrk;
extern char *sbrk();
#endif

extern void flags();
extern short	uflag;

#if DEBUG
short Oflag = NO;
#endif

extern char *filenames[];

#if M4ON
extern short rflag;
#endif

#if M32RSTFIX
extern short rstflag;	/* Don't generate workaround for RESTORE chip bug */
#endif

static int callsys();
char	*tempnam();
extern void werror();

static char prefix[]="asX";

#if M4ON
static short
	macro = MACRO;
static char *m4name = M4;
#if !NODEFS
static char
	*regdef = CM4DEFS;
#endif
#endif


#ifndef SGU_PKG
#define SGU_PKG "package"
#endif
#ifndef SGU_REL
#define SGU_REL "release"
#endif

short vstamp;

#define max(A,B) (((A)<(B))?(B):(A))

extern char *getenv(), *mktemp();
extern void comment();

void check_files();

/*
 *
 *	"tempnam" is the routine that allocates the temporary files needed
 *	by the assembler.  It searches directories for enough space for the
 *	files, then assigns a (guaranteed) unique filename using the prefix
 *	in the function call.
 *
 */

/*
 *
 *	"getargs" is a general purpose argument parsing routine.
 *	It locates flags (identified by a preceding minus sign ('-'))
 *	and initializes any associated flags for the assembler.
 *	If there are any file names in the argument list, then a
 *	pointer to the name is stored in a global variable for
 *	later use.
 *
 */

static void
getargs(xargc,xargv)
	register int xargc;
	register char **xargv;
{
	register char ch;

	while (xargc-- > 0) {
		if (**xargv == '-') {
			while ((ch = *++*xargv) != '\0')
				switch (ch) {
#if M4ON
				case 'm':
					macro = ! macro;
					break;
				case 'Y': /* specify directory for m4 and/or cm4defs */
					{ char *dirname, *keys;

					  if (*++*xargv == '\0') {
						xargc--;
						xargv++;
					  }
					  keys = dirname = (*xargv);
					  while (*dirname)
						if (*dirname == ',') {
							dirname++;
							break;
						} else dirname++;
					  while (*keys != ',' && *keys != '\0'){
						switch (*keys) {
						case 'm' :
							m4name=malloc(strlen(dirname)+4);
							(void) strcpy(m4name,dirname);
							(void) strcat(m4name,"/m4");
							break;
#if !NODEFS
						case 'd' :
							regdef=malloc(strlen(dirname)+9);
							(void) strcpy(regdef,dirname);
							(void) strcat(regdef,"/cm4defs");
							break;
#endif
						default :
							werror(
							"Unknown -Y suboption");
							break;
						}
						keys++;
					  }
					  *xargv = dirname + strlen(dirname) - 1;
					}
					break;
#endif
				case 'o':
					if (*++*xargv == '\0') {
						xargv++;
						xargc--;
					}
					filenames[1]=malloc(strlen(*(xargv))+1);
					(void) strcpy(filenames[1],*(xargv));
					while (*++*xargv != '\0') ;
					--*xargv;
					break;
				case 'd':
					if (*++*xargv == 'l')
						werror("-dl option is ignored");
					else {
						char errmsg[100];
						(void) sprintf(errmsg,
						  "-d%c unknown option",**xargv);
						aerror(errmsg);
						if (**xargv == '0')
							--*xargv;
					     }
					break;

				case 'u':
					uflag = YES;
					break;

				case 'V':
					/* version flag */
					(void) fprintf
					(stderr,"as: %s %s\n", SGU_PKG,SGU_REL);
					break;
				case 'Q':
					 if (*++*xargv == '\0') {
                                                xargc--;
                                                xargv++;
                                        }
					switch(**xargv) {
						case 'y': 
							vstamp = YES; 
							break;
						case 'n':
							vstamp = NO;
							break;
						default:
							aerror
							("Unknown -Q suboption");
							vstamp = NO;
							break;
					}
					break;
#if M32
				case 'K':
					 if (*++*xargv == '\0') {
                                                xargc--;
                                                xargv++;
                                        }

					if (strcmp(*xargv, "mau") == NULL)
						need_mau = YES;
					else
                                                aerror("Unknown -K suboption");
                                        *xargv += strlen(*xargv) - 1;
					break;
				case 'f':
					/* no frills; disable work-arounds */
					workaround = NO;
#if M32RSTFIX
					rstflag = NO;
#endif	/* M32RSTFIX */
					break;
#endif /* M32 */
				case 'n':
					switch (*++*xargv) {
#if M32
					case 'f': /* -nf option */
					/* no frills; disable work-arounds */
						werror("Obsolete -nf option: use -f");
						workaround = NO;
#if M32RSTFIX
						rstflag = NO;
#endif	/* M32RSTFIX */
						break;
					case 'a': /* -na option */
						/* disable 32001 work-arounds */
						werror("Obsolete -na option");
						break;
					case 'm': /* -nm option: obsolete */
						/* set mau required flag */
						werror("Obsolete -nm option: use -Kmau");
						break;
#endif /* M32 */
					default:	/* -n option */
						spanopt = NO;
						--*xargv;
					}
					break;
#if DEBUG
				case 'O':
					Oflag = YES;
					break;
#endif
#if M4ON
				case 'R':
					rflag = YES;
					break;
#endif
#if M32RSTFIX
				case 'r': /* Don't generate the workaround
					   * for the RESTORE chip bug. 
                                           */
					rstflag = NO;
					break;
#endif	/* M32RSTFIX */
#if VAX
				case 'j': /* invoke ljas instead of as */
					if (strcmp(xargv[0],"ljas")) {
						(void) execvp("ljas",xargv);
						aerror("cannot exec ljas");
						};
					break;
#endif
#if M32
				case 'w': /* set mau warning flag */
					wflag = YES;
					break;
#endif /* M32 */
				case 'T': /* transition mode assembly */
					transition = YES;
					break;
				default:
					flags(ch);
					break;
				}
			xargv++;
		}
		else {
			char errmsg[100];
			if (filenames[0] == 0)
				filenames[0] = *xargv;
			else
				{
				(void) sprintf(errmsg,
				  "Only one file assembled - %s ignored",*xargv);
				werror(errmsg);
				}
				
			xargv++;
		}
	}
}

/*
 *
 *	"main" is the main driver for the assembler. It calls "getargs"
 *	to parse the argument list, set flags, and stores pointers
 *	to any file names in the argument list .
 *	If the output file was not specified in the argument list
 *	then the output file name is generated. Next the temporary
 *	file names are generated and the first pass of the assembler
 *	is invoked.
 *
 */

main(argc,argv)
int argc;
char **argv;
{
	register short index, count;
	FILE	*fd;
	static void callm4();
	extern void aspass1();
	extern void aspass2();

	argc--;
	if (argc <= 0) {
		(void) fprintf(stderr,"Usage: %sas [options] file\n",SGS);
		exit(1);
	}
	argv++;
	getargs(argc, argv);
#if STATS
	firstbrk = sbrk(0);
#endif

/*	Check to see if input file exits */
	if ((fd = fopen(filenames[0],"r")) != NULL)
		(void) fclose(fd);
	else {
		(void) fprintf(stderr,"Nonexistent file\n");
		exit(1);
	}
	if (filenames[1] == 0) {
		for(index=0,count=0;filenames[0][index]!='\0';index++,count++)
			if(filenames[0][index]=='/')
				count = -1;
		if(filenames[0][index-2]=='.' && filenames[0][index-1]=='s') {
			filenames[1] = malloc(strlen(filenames[0]+index-count)+1);
			(void) strcpy(filenames[1],filenames[0]+index-count);
			filenames[1][count-1]='o';
		}
		else
		{
			filenames[1] = malloc(strlen(filenames[0]+index-count)+3);
			(void) strcpy(filenames[1],filenames[0]+index-count);
			(void) strcpy(filenames[1]+count,".o");
		}
	check_files(filenames[0],filenames[1]);
	} else if (strcmp(filenames[0],filenames[1]) == NULL){
		(void) fprintf(stderr,
			       "Assembler: -o would overwrite input file \"%s\"\n",
		               filenames[0]);
		exit(1);
	}


	for (index = 2; index < NFILES; index++) {
		prefix[2] = 'A' + index -1;
		filenames[index] = tempnam(TMPDIR,prefix);
		}

#if M4ON
	if (macro) {
		/* tell pass1 to unlink its input when through */
		rflag = YES;
		callm4();
	}
#endif

	aspass1();
	aspass2();
	exit(0);
/*NOTREACHED*/
} /* main */

#if M4ON
static void
callm4()
{
	static char
		*av[] = { "m4", 0, 0, 0};

	/*
	*	The following code had to be added with a 'u370' ifdef due
	*	to a MAXI bug concerning static pointers. We think it's fixed
	*	that is why the code is commented out. It can be deleted
	*	when this fact is verified.
	*
	*	char *regdef, *tvdef;
	*	regdef = CM4DEFS;
	*	tvdef = CM4TVDEFS;
	*/

	av[0] = m4name;
#if !NODEFS
	av[1] = regdef;
	av[2] = filenames[0];
#else
	av[1] = filenames[0];
#endif
	prefix[2] = 'A';
	filenames[0] = tempnam(TMPDIR,prefix); 		/* m4 output file */
	if (callsys(m4name, av, filenames[0]) != 0) {
		(void) unlink(filenames[0]);
		(void) fprintf(stderr,"Assembly inhibited\n");
		exit(100);
	}
	return;
} /* callm4 */

static callsys(f,v,o)
	char f[], *v[];
	char *o;	/* file name, if any, for redirecting stdout */
{
	int t, status;
        pid_t pid;

	if ((pid=fork())==0) {
		if (o != NULL) {	/* redirect stdout */
			if (freopen(o, "w", stdout) == NULL) {
				(void) fprintf(stderr,"Can't open %s\n", o);
				exit(100);
			}
		}
		(void) execv(f, v);
		(void) fprintf(stderr,"Can't find %s\n", f);
		exit(100);
	} else
		if (pid == -1) {
			(void) fprintf(stderr,"Try again\n");
			return(100);
		}
	while(pid!=wait(&status));
	if ((t=(status&0377)) != 0) {
		if (t!=SIGINT)		/* interrupt */
			{
			(void) fprintf(stderr,"status %o\n",status);
			(void) fprintf(stderr,"Fatal error in %s\n", f);
			}
		exit(100);
	}
	return((status>>8) & 0377);
} /* callsys */
#endif


void
dovstamp()
{
	char stamp[200];
	(void) sprintf(stamp, "as: %s", SGU_REL);
	comment(stamp);
	return;
}


/* make sure input file and output file are not the same */
void
check_files(infile,outfile)
char *infile;
char *outfile;
{
	struct stat inbuf;
	struct stat outbuf;

	if (stat(infile, &inbuf) == -1) { 
		(void) fprintf(stderr,			
			"Assembler: Cannot stat(2) input file\n");
		   exit(1); 
	}
	if ((stat(outfile, &outbuf) != -1) &&
	    (ino_t) inbuf.st_ino == (ino_t) outbuf.st_ino && 
	    (dev_t) outbuf.st_dev == (dev_t) outbuf.st_dev){
		(void) fprintf(stderr,
		"Assembler: Input file is the same as output file \"%s\"\n"		        ,filenames[0]);
		exit(1);
	}
}

