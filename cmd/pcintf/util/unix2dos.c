/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:util/unix2dos.c	1.1"
#include	"sccs.h"
SCCSID(@(#)unix2dos.c	1.9	LCC);	/* Modified: 11/17/89 12:37:31 */
/*	@LCCID(unix2dos.c, 1.9, 11/17/89, 12:37:31)	*/

/****************************************************************************

	Copyright (c) 1985 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

****************************************************************************/

/***************************************************************\
*                                                               *
*  unix2dos: convert files from unix format to dos format       *
*                                                               *
*  A dos file is a sequence of lines ended by CR/LF             *
*  with end of file marked by a control-Z.  (There may be       *
*  data after the "logical" end of file).                       *
*  A unix file is a sequence of lines ended by NL (same as LF)  *
*  with no special end of file mark character (and no data      *
*  after end of file, obviously).                               *
*                                                               *
*  The program assumes ascii text and strips characters to      *
*  7 bits unless the -b (binary) option is given.               *
*  The program will convert lower to upper case with the -u     *
*  option and upper to lower with the -l option.                *
*								*
*  Normally, the program will turn a LF into a CR/LF only if	*
*  the line does not already have one or more CRs before the	*
*  LF.  This allows the user to run unix2dos on dos files	*
*  without creating longer and longer strings of CRs followed	*
*  a LF.  If the user really wants this behavior, he may	*
*  force EVERY LF into CR/LF by using the -f (force) option.	*
*								*
*  If -b (binary) is NOT specified, the program will stop on	*
*  control-z.							*
*								*
*	Syntax:							*
*                                                               *
*	unix2dos [ options ] [ files ] [ options ] ...		*
*                                                               *
*		no files: standard input to standard output	*
*		one file: file to standard output		*
*		two files: file1 to file 2			*
*								*
*		options may be specified with "-" option	*
*		or on the DOS compiled version with "/" option	*
*								*
\***************************************************************/

/*
 *	Modification history
 *
 *		 7-05-85	Rich Patterson	Added support for Microsoft
 *						C version 3. Removed support
 *						for Lattice due to lib bug.
 *						
 *		 7-05-85	Rich Patterson	Added error handling for
 *						fputc under MSCV3 and UNIX
 *						with function fputc_chk.
 *
 *		 9-09-85	David Oderberg	In fputc_chk: 1) added 3rd
 *						condition so errno==0 doesn't
 *						print error msg & exit, 2)
 *						changed dos2unix -> unix2dos
 *
 *               1-01-86        David Peet      Removed message "Stopped at
 *                                              DOS end of file".
 *                                              Made changes so Unix version
 *                                              could be made:
 *                                              Removed hardwired define of
 *                                              MSCV3, and put setbuf inside
 *                                              ifdef C86 .
 *		9/3/87		Jeremy Daw      Put or MSCV4 flag in and 
 *						removed writing of ^Z at EOF.
 *
 *      7/18/89     Peter Dobson    Only write ^Z at EOF if output is to
 *                                  a file spec on command line, not out
 *                                  to a file redirected with > or >>.
 */

#include <stdio.h>

	/***********************\
	*  handle dos/unix c86	*
	\***********************/


#ifdef MICROSOFT
#include <fcntl.h>
#include <io.h>
#endif

#ifdef MICROSOFT
#define	DOS	1
#define READ	"rb"
#define WRITE	"rb+"
#define CREATE	"wb+"
#endif

#ifndef DOS
#define	READ	"r"
#define WRITE	"r+"
#define CREATE	"w+"
#endif


	/***********************\
	*  language extensions  *
	\***********************/

#define FALSE   0
#define TRUE    1

	/***************\
	*  constants    *
	\***************/

#define CR      015                     /* carriage return */
#define NL      012                     /* new line */
#define CTLZ    032                     /* control-z */

	/***************\
	*  globals      *
	\***************/

char  scratch[] = "\n\
# PC-Interface copyright (c) 1984, 1987 by Locus Computing Corporation.\n\
# All Rights Reserved.\n\
";
char *cname;

/* Warning message displayed when processing a file in non-Binary and
 * a character with the MSB set is encountered
 */

char	*msg_warning="Warning: 8 bit character detected without -b option.\n";
char	*msg_err[] = {
	"Same file specified for both input and output.\n",
	"Input file is zero length.\n",
	"An error has occurred while reading input file.\n"
};

	/***************\
	* sub  routines *
	\***************/
/*
 * int
 * samefile(in_fd, out_fd)
 * char	*in_fd, *out_fd;
 *
 *   This function returns 0 if:
 *	the files described by in_fd and out_fd are not the same file
 *
 *   Other error returns are as follows:
 *	1 == the two files are the same file
 *	-1 == any error is encountered
 *	-2 == input file is zero length
 *
 */

int
samefile(in_fd, out_fd)
char	*in_fd, *out_fd;
{
	char	byte_save;	/* byte save location */
	char	read_buf;
	int	i, out_zl, in_zl;
	int	ind, outd;
	long	lseek();

	in_zl = out_zl = FALSE;
	read_buf = byte_save = 0;

	ind = open(in_fd, 0);
	outd = open(out_fd, 0);
	if ((ind < 0) || (outd < 0)) {
		close(ind);
		close(outd);
		return(-1);
	}

	i = read(ind, &read_buf, 1);
	if (i == 0)
		in_zl = TRUE;
	else if (i < 0) {
		close(ind);
		close(outd);
		return(-1);
	}

	i = read(outd, &byte_save, 1);
	if (i == 0)
		out_zl = TRUE;
	else if (i < 0) {
		close(ind);
		close(outd);
		return(-1);
	}

	if ((out_zl != in_zl) || (read_buf != byte_save)) {
		close(ind);
		close(outd);
		return(0);	/* first bytes differ, assume different */
	}

	if (in_zl)
		return(-2);

	close(ind);
	close(outd);

	outd = open(out_fd, 1);
	if (outd < 0) {
		close(ind);
		close(outd);
		return(-1);
	}

	read_buf++;	/* new data different always than old */

	if (write(outd, &read_buf, 1) != 1) {
		close(outd);
		return(-1);
	}

	close(outd);
	ind = open(in_fd, 0);
	if (ind < 0) {
		return(-1);
	}

	if (read(ind, &read_buf, 1) != 1)
		return(-1);
	close(ind);

	outd = open(out_fd, 1);
	if (outd < 0) {
		close(outd);
		return(-1);
	}

	if (write(outd, &byte_save, 1) != 1)
		return(-1);

	close(outd);

	byte_save ++;
	return(byte_save == read_buf);
}

/*
 * Print program usage
 *
 */

void
pr_usage(fp)
FILE	*fp;
{
	fprintf(fp, "Usage: %s [-fblu] [input] [output]\n", cname);
}

/* 
 * Extract the last path item for use as program name 
 * 
 */ 
 
char	* 
copy_name(path) 
char	*path;		/* normally argv[0] */ 
{ 
	char	*p; 
 
	p = path + strlen(path) - 1; 
 
	while (p) { 
		if ((*p == ':') || (*p == '/') || (*p == '\\') ||
		    p == path) { 
			if (p == path)
				path = p;
			else
				path = p + 1; 
			p = NULL; 
		} else 
			--p; 
	} 
	return(path); 
} 
 
 
	/****************\
	*  main program  *
	\****************/

main(argc, argv)
int argc;               /* number of arguments */
char *argv[];		/* array of argument pointers */
{
	register int    x;              /* current input character */
	register int	crcount = 0;	/* carriage return count */
	register int    upper = FALSE;  /* map to upper case */
	register int    lower = FALSE;  /* map to lower case */
	register int    binary = FALSE; /* binary mode */
	register int	force = FALSE;	/* force mode */
	FILE   		*input = NULL;  /* current input file */
	FILE   		*output = NULL; /* current input file */
	int		i;		/* general integer */
	unsigned long	bin_char= 0;	/* count of binary characters */
	int		created_out = FALSE;
	int		in_file;	/* index into argv for input name */
	int		out_file;	/* index into argv for output name */

	/******************\
	*  initialization  *
	\******************/

	cname = copy_name(argv[0]);

	/* process option and file arguments */

	for (i = 1; i < argc; i++) {

		/* test for option */
		switch ( *(argv[i]) ) {
		case '-':
#ifdef DOS
		case '/':
#endif
			while ( *(++argv[i]) != 0 ) {
				switch ( *(argv[i]) ) {

				case 'b':
				case 'B':
					binary = TRUE;
					continue;
				case 'f':
				case 'F':
					force = TRUE;
					continue;
				case 'u':
				case 'U':
					upper = TRUE;
					continue;
				case 'l':
				case 'L':
					lower = TRUE;
					continue;
				default:
					fprintf(stderr,
					 "Invalid option -%c\n",
					 *argv[i]);
					pr_usage(stderr);
					exit(1);
				}
			}
			continue;

		default:
			/* must be a file argument */
			if (input == NULL) {
				if((binary&&(upper||lower)) || (upper&&lower))
                                    {
		                    fprintf(stderr, 
                                    "Invalid combination of options.\n");
                                    exit(-1);
	                            }
				in_file = i;
				input = fopen(argv[i], READ);
				if (input == NULL) {
					fprintf(stderr,
						"Can't open input file %s.\n",
						argv[in_file]);
					exit(-1);
				}
				continue;
			}

			if (output == NULL) {
				out_file = i;
				output = fopen(argv[out_file], WRITE);
				if (output == NULL) {
					output = fopen(argv[out_file], CREATE);
					if (output == NULL) {
						fprintf(stderr,
						 "Can't open output file %s.\n",
						 argv[out_file]);
						exit(-1);
					}
					created_out ++;
				}
				continue;
			}
		}
	}

	if ( (binary && (upper || lower)) || (upper && lower) ) {
		fprintf(stderr, "Invalid combination of options.\n");
		exit(-1);
	}

	if ((input != NULL) && (output != NULL) && (!created_out)) {
		i = samefile(argv[in_file], argv[out_file]);
		if (i) {
			switch(i) {
			case 1:
				fprintf(stderr, "%s", msg_err[0]);
				break;
			case -1:
				fprintf(stderr, "%s", msg_err[2]);
				break;
			case -2:
				fprintf(stderr, "%s", msg_err[1]);
				break;
			default:
				break;
			}
			exit(1);
		}
		fclose(output);
		output = fopen(argv[out_file], CREATE);
		if (output == NULL) {
			fprintf(stderr, "Can't open output file %s.\n",
					argv[out_file]);
			exit(-1);
		}
	}


#ifndef DOS
	if (input == NULL) input = stdin;
	if (output == NULL) output = stdout;
#endif

#ifdef MICROSOFT
	/* make sure if using stdin or stdout that they are in binary */

	if (input == NULL) {
		setmode(fileno(stdin), O_BINARY);
		input = stdin;
	}
	if (output == NULL) {
		setmode(fileno(stdout), O_BINARY);
		output = stdout;
	}
#endif


	/*************\
	*  char loop  *
	\**************/
nextchar:

	x = fgetc(input);
	if ((x != EOF) && !binary) {
		if (x & 0x80) {
			if (!(bin_char++))
				fprintf(stderr, msg_warning);
		}

		x &= 0177;
		if (upper && (x >= 'a' && x <= 'z')) x += ('A' - 'a');
		else if (lower && (x >= 'A' && x <= 'Z')) x -= ('A' - 'a');
	}

	switch (x) {

	case EOF:
	forceeof:
		if (force) while (crcount--) fputc_chk(CR, output);

        /*  if it ain't a tty, and ain't redirected output
         *  (could be followed by >> output, and ^Z in middle)
         *  then EOF it
         */
        if (isatty(fileno(output)) != TRUE && output != stdout)
			fputc_chk(CTLZ, output);

		fflush(output);
		if (ferror(input)) {
			fprintf(stderr,
				"An error has occurred while reading input file.\n");
			exit(-1);
		}
		if (ferror(output)) {
			fprintf(stderr,
				"An error has occurred while writing output file.\n");
			exit(-1);
		}
		exit(0);

	case CR:
		++crcount;
		goto nextchar;

	case NL:
		if (!force) crcount = 0;
		fputc_chk(CR, output);
		goto outputchar;

	case CTLZ:
		if (binary) goto outputchar;
		goto forceeof;

	default:
	outputchar:
		for ( ; crcount; --crcount) fputc_chk(CR, output);
		fputc_chk(x, output);
		goto nextchar;
	}
}

/*
 * fputc_chk		Uses fputc and then checks for an error in the stream.
 *			if an error is found then the program aborts using
 *			_exit with a status of 1
 */

fputc_chk(c, stream)
int	c;
FILE * stream;
{
	extern int errno;

	if ((fputc(c, stream) < 0) && ferror(stream) && errno) {
		perror(cname);		/* use internal error */
		_exit(1);
	}
}
