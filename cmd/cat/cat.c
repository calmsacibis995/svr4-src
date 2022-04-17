/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)cat:cat.c	1.18"
/*
**	Concatenate files.
*/


#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<locale.h>

#define	IDENTICAL(A,B)	(A.st_dev==B.st_dev && A.st_ino==B.st_ino)

char	buffer[BUFSIZ];

int	silent = 0;		/* s flag */
int	visi_mode = 0;		/* v flag */
int	visi_tab = 0;		/* t flag */
int	visi_newline = 0;	/* e flag */
int	errnbr = 0;

main(argc, argv)
int    argc;
char **argv;
{
	register FILE *fi;
	register int c;
	extern	int optind;
	int	errflg = 0;
	int	stdinflg = 0;
	int	status = 0;
	struct stat source, target;

	(void)setlocale(LC_ALL, "");
#ifdef STANDALONE
	/*
	 * If the first argument is NULL,
	 * discard arguments until we find cat.
	 */
	if (argv[0][0] == '\0')
		argc = getargv ("cat", &argv, 0);
#endif

	/*
	 * Process the options for cat.
	 */

	while( (c=getopt(argc,argv,"usvte")) != EOF ) {
		switch(c) {

		case 'u':

			/*
			 * If not standalone, set stdout to
	 		 * completely unbuffered I/O when
			 * the 'u' option is used.
			 */

#ifndef	STANDALONE
			setbuf(stdout, (char *)NULL);
#endif
			continue;

		case 's':
		
			/*
			 * The 's' option requests silent mode
			 * where no messages are written.
			 */

			silent++;
			continue;

		case 'v':
			
			/*
			 * The 'v' option requests that non-printing
			 * characters (with the exception of newlines,
			 * form-feeds, and tabs) be displayed visibly.
			 *
			 * Control characters are printed as "^x".
			 * DEL characters are printed as "^?".
			 * Non-printable  and non-contrlol characters with the
			 * 8th bit set are printed as "M-x".
			 */

			visi_mode++;
			continue;

		case 't':

			/*
			 * When in visi_mode, this option causes tabs
			 * to be displayed as "^I".
			 */

			visi_tab++;
			continue;

		case 'e':

			/*
			 * When in visi_mode, this option causes newlines
			 * and form-feeds to be displayed as "$" at the end
			 * of the line prior to the newline.
			 */

			visi_newline++;
			continue;

		case '?':
			errflg++;
			break;
		}
		break;
	}

	if (errflg) {
		if (!silent)
			fprintf(stderr,"usage: cat -usvte [-|file] ...\n");
		exit(2);
	}

	/*
	 * Stat stdout to be sure it is defined.
	 */

	if(fstat(fileno(stdout), &target) < 0) {
		if(!silent)
			fprintf(stderr, "cat: Cannot stat stdout\n");
		exit(2);
	}

	/*
	 * If no arguments given, then use stdin for input.
	 */

	if (optind == argc) {
		argc++;
		stdinflg++;
	}

	/*
	 * Process each remaining argument,
	 * unless there is an error with stdout.
	 */


	for (argv = &argv[optind];
	     optind < argc && !ferror(stdout); optind++, argv++) {

		/*
		 * If the argument was '-' or there were no files
		 * specified, take the input from stdin.
		 */

		if (stdinflg
		 ||((*argv)[0]=='-' 
		 && (*argv)[1]=='\0'))
			fi = stdin;
		else {
			/*
			 * Attempt to open each specified file.
			 */

			if ((fi = fopen(*argv, "r")) == NULL) {
				if (!silent)
				   fprintf(stderr, "cat: cannot open %s\n",
								*argv);
				status = 2;
				continue;
			}
		}
		
		/*
		 * Stat source to make sure it is defined.
		 */

		if(fstat(fileno(fi), &source) < 0) {
			if(!silent)
			   fprintf(stderr, "cat: cannot stat %s\n", *argv);
			status = 2;
			continue;
		}


		/*
		 * If the source is not a character special file or a
		 * block special file, make sure it is not identical
		 * to the target.
		 */
	
		if (!S_ISCHR(target.st_mode)
		 && !S_ISBLK(target.st_mode)
		 && IDENTICAL(target, source)) {
			if(!silent)
			   fprintf(stderr, "cat: input/output files '%s' identical\n",
						stdinflg?"-": *argv);
			if (fclose(fi) != 0 ) 
				fprintf(stderr, "cat: close error\n");
			status = 2;
			continue;
		}

		/*
		 * If in visible mode, use vcat; otherwise, use cat.
		 */

		if (visi_mode)
			status = vcat(fi);
		else
			status = cat(fi);

		/*
		 * If the input is not stdin, flush stdout.
		 */

		if (fi!=stdin) {
			fflush(stdout);
			
			/* 
			 * Attempt to close the source file.
			 */

			if (fclose(fi) != 0) 
				if (!silent)
					fprintf(stderr, "cat: close error\n");
		}
	}
	
	/*
	 * When all done, flush stdout to make sure data was written.
	 */

	fflush(stdout);
	
	/*
	 * Display any error with stdout operations.
	 */

	if (errnbr = ferror(stdout)) {
		if (!silent) {
			fprintf (stderr, "cat: output error(%d)\n", errnbr);
			perror("");
		}
		status = 2;
	}
	exit(status);
}

int
cat(fi)
	FILE *fi;
{
	register int fi_desc;
	register int nitems;

	fi_desc = fileno(fi);

	/*
	 * While not end of file, copy blocks to stdout. 
	 */

	while ((nitems=read(fi_desc,buffer,BUFSIZ))  > 0) {
		if ((errnbr = write(1,buffer,(unsigned)nitems)) != nitems) {
			if (!silent) {
				if (errnbr == -1)
					errnbr = 0;
				fprintf(stderr, "cat: output error (%d/%d characters written)\n", errnbr, nitems);
				perror("");
			}
			return(2);
		}
	}

	return(0);
}


vcat(fi)
	FILE *fi;
{
	register int c;

	while ((c = getc(fi)) != EOF) 
	{
		/*
		 * For non-printable and non-cntrl  chars, use the "M-x" notation.
		 */
		if ( ! isprint(c) && ! iscntrl(c) ) 
			{
			putchar('M');
			putchar('-');
			c-= 0200;
			}
		/*
		 * Display plain characters
		 */
		 if (  isprint(c) )
			{
			putchar(c);
			continue;
			}
		/*
		 * Display tab as "^I" if visi_tab set
		 */

		if ( (c == '\t') || (c == '\f') )
			{
			if (! visi_tab)
				putchar(c);
			else
				{
				putchar('^');
				putchar(c^0100);
				}
			continue;
			}
		/*
		 * Display newlines as "$<newline>"
		 * if visi_newline set
		 */
		if ( c == '\n')
			{
			if (visi_newline) 
				putchar('$');
			putchar(c);
			continue;
			}
		/*
		 * Display control characters
		 */
		if ( c <  0200 )
			{
			putchar('^');
			putchar(c^0100);
			}
		else
			{
			putchar('M');
			putchar('-');
			putchar('x');
			}
	}
	return(0);
}
