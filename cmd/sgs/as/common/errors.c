/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/errors.c	1.6"

/*
 *
 *	"errors.c" is a file containing a routine "aerror" that prints
 *	out  error messages and (currently) terminates execution when
 *	an error is encountered.  It prints out  the  file  and  line
 *	number  where  the  error  was  encountered and unlinks all
 *	temporary files.
 *	The following global variables are used in this file:
 *
 *	line	Gives the line  number  in  the  current  file  being
 *		assembled.
 *
 *	fderr	The file descriptor for error output.
 *
 *	filenames  This is an array of pointers to character strings
 *		that contains pointers to all of the file names
 *		obtained from the argument list. This array is used
 *		to obtain the names of the temporary files to be
 *		removed.
 *
 */

#include <stdio.h>
#include <signal.h>
#include "systems.h"
#include "gendefs.h"
#include "symbols.h"

#define MAXERRS	30

void errmsg();
void delexit();
unsigned short line = 1;
short anyerrs = 0;
symbol *cfile;	/* name of `c' source file */

FILE	*fderr;

char	*filenames[NFILES];

#if M4ON
short rflag = NO;		/* if set, unlink input file when through */
#endif

/* ARGSUSED */
void
onintr(i)
int i;
{
	i = 0;
	(void) signal(SIGINT,onintr);
	(void) signal(SIGTERM,onintr);
	delexit();
}

void
aerror(str)
register char *str;
{
	errmsg("",str);
	delexit();
}

void
elferror(str)
register char *str;
{
	register int err;

	if ((err = elf_errno()) != 0)
		errmsg(str,elf_errmsg(err));
	delexit();
}
void
yyerror(str)
char *str;
{
	errmsg("",str);
	if (++anyerrs > MAXERRS) {
		(void) fprintf(stderr,"Too many errors - Goodbye\n");
		delexit();
	}
}

void
werror(str)
char *str;
{
	errmsg("Warning: ",str);
}

void
errmsg(str1,str2)
char *str1,*str2;
{
	char *msgfile;
	static char nullstr[] = "";
	static short firsterr = 1;

	if (firsterr) {
		if (cfile)
			msgfile = cfile->name;
		else if (filenames[0] != NULL)
			msgfile = filenames[0];
		else
		  	msgfile = nullstr;
		(void) fprintf(stderr,"Assembler: %s\n",msgfile);
		firsterr = 0;
	}
	(void) fprintf(stderr,"\taline %u",line);
	(void) fprintf(stderr,"\t");
	(void) fprintf(stderr,": %s%s\n",str1,str2);
}

void
delexit()
{
	void deltemps();

	(void) unlink(filenames[1]); /* unlink object (.o) file */
	deltemps();
	exit(127);
}

void
deltemps()
{
	register short i;

#if M4ON
	if (rflag)
		(void) unlink(filenames[0]);
#endif
	for (i = 2; i < NFILES; ++i)
		(void) unlink(filenames[i]);
}
