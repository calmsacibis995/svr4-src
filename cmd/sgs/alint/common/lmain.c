/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/lmain.c	1.13"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "p1.h"
#include "lnstuff.h"
#include "ldope.h"
#include "sgs.h"
#include "xl_byte.h"

/*
** Each command line flag has a flag in ln_flags from A-Za-z
*/
int	ln_flags[52];

/*
** Debugging flag for cxref.
*/
#ifndef NODBG
extern int cx_debug;
#endif

/*
** Lint uses four intermediate files.
** file1 holds definitive definitions.
** file2 holds tentative definitions, declarations, prototypes.
** file3 holds uses, return values.
** file4 holds ascii strings used as parameters to functions
**		  (for printf/scanf checking)
*/
char	*sourcename;		/* file being linted			*/
char	*htmpname;		/* lint messages from headers here	*/
char	*hdrs;			/* names of headers			*/
static char	*fname1;	/* for definitions			*/
static char	*fname2;	/* for declarations			*/
static char	*fname3;	/* for uses				*/
static char	*fname4;	/* string file				*/

FILE 	*file1;
FILE	*file2;
FILE	*file3;
FILE	*file4;


/*
** These are the sizes and alignments of the types used when linting 
** with the -p option.
*/
static PORTSIZE ln_size[] = {
	{ TY_CHAR,	8,	8 },
	{ TY_UCHAR, 	8,	8 },
	{ TY_SCHAR,	8,	8 },
	{ TY_SHORT,	16,	16 },
	{ TY_USHORT,	16,	16 },
	{ TY_SSHORT,	16,	16 },
	{ TY_INT,	16,	16 },
	{ TY_UINT,	16,	16 },
	{ TY_AINT,	16,	16 },
	{ TY_AUINT,	16,	16 },
	{ TY_SINT,	16,	16 },
	{ TY_LONG,	32,	32 },
	{ TY_ULONG,	32,	32 },
	{ TY_SLONG,	32,	32 },
	{ TY_FLOAT,	32,	32 },
	{ TY_DOUBLE,	64,	64 },
	{ TY_LDOUBLE,	64,	64 },
	{ TY_PTR,	16,	16 },
};


static int ln_init();


/*
** Return the list of arguments lint takes to the front end.
*/
const char *
ln_optstring()
{
    static const char ln_argstr[] = "abhkmpsuvxyF1:2:T:VR:W";
    LNBUG(ln_dbflag > 1, ("ln_optstring"));
    return (char *) ln_argstr;
}



/*
** Process options.
** For most options, just set a LN_FLAG().
** For EOF (no more options), call ln_init().
** For N, set up the names of the intermediate files.
** For S, set the name of the source file (name on command line.)
** For 1, set debugging flags.
*/
int
ln_opt(c, opt)
int c;
char *opt;
{
    static char *tmpnames;
#ifndef __STDC__
    char *malloc();
#endif
    LNBUG(ln_dbflag > 1, ("ln_opt: %c %s",c,opt));
    switch (c) {
	case EOF:
	    if (opt != (char *) 0) {
		sourcename = (char *) malloc((strlen(opt)+1)*sizeof(char));
		strcpy(sourcename, opt);
	    } else if (! ln_init())
		return 1;
	    break;

	case 'p':
	    lntt_init(ln_size, sizeof(ln_size) / sizeof(PORTSIZE));
	    LN_FLAG(c) = 1;
	    break;

	case 'a':  case 'b':  case 'h':  case 'k':  case 'm':
	case 's':  case 'u':  case 'v':	 case 'x':  
	case 'F':  case 'W':
	    LN_FLAG(c) = 1;
	    break;

	case 'V':
            fprintf(stderr,"lint: %s %s\n", CPL_PKG, CPL_REL);
            break;

	case 'y':
	    LN_DIR(LINTLIBRARY) = 1;
	    break;

	case 'R':
	    LN_FLAG(c) = 1;
	    cx_init(opt);
	    break;

	/* 
	** temp files - these are passed in a predefined order:
	**	- header messages dump
	**	- header file names dump
	**	- temp file1
	**	- temp file2
	**	- temp file3
	**	- temp file4
	*/
	case 'T':
	    tmpnames = (char *) malloc(strlen(opt) + 1);
	    strcpy(tmpnames, opt);
	    LN_FLAG(c) = 1;
	    htmpname = strtok(tmpnames, ",");
	    hdrs = strtok(NULL, ",");
	    fname1 = strtok(NULL, ",");
	    fname2 = strtok(NULL, ",");
	    fname3 = strtok(NULL, ",");
	    fname4 = strtok(NULL, ",");
	    if ((htmpname == NULL) 
		|| (hdrs == NULL)
		|| (fname1 == NULL)
		|| (fname2 == NULL)
		|| (fname3 == NULL)
		|| (fname4 == NULL)
	       )
		lerror(FATAL, "bad temp files");
	    LNBUG(ln_dbflag, ("htmpname: %s\nhdrs: %s\n",htmpname,hdrs));
	    LNBUG(ln_dbflag, ("fname1,2,3,4: %s %s %s %s\n",fname1,fname2,fname3,fname4));
	    break;

#ifndef NODBG
	case '1':
	    while (*opt) {
		if (*opt == 'l')
		    ++ln_dbflag;
		else if (*opt == 'R')
		    ++cx_debug;
		++opt;
	    }
	    break;
#endif
	case '2':	/* pass 2 (lint2) options - set in lint2	*/
	    break;
    }
    return 0;
}



/*
** Initialize files for buffering and the intermediate files.
** Set up the dope array for lint.
*/
#define OPFILE(file, name)	((file = fopen(name, "w")) == NULL)
static int
ln_init()
{
#ifndef NODBG
    static char nullfile[]="/dev/null";
#endif
    FLENS lout;
    LNBUG(ln_dbflag > 1, ("ln_init"));

    if (! LN_FLAG('T')) {
#ifndef NODBG
	htmpname = hdrs = fname1 = fname2 = fname3 = fname4 = nullfile;
	LN_FLAG('s') = 1;
#else
	/*
	** No file arguments are only legal if -V was passed as well.
	*/
	exit(LN_FLAG('V') == 0);
#endif
    }
    xl_init();

    verbose = 1;	/* always call front end with -v option 	  */
    ldope();
    tmpopen();		/* open up files for buffering messages		  */
    ln_initdir();

    if (OPFILE(file1, fname1) 
	|| OPFILE(file2, fname2)
	|| OPFILE(file3, fname3) 
	|| OPFILE(file4, fname4)) 
    {
	    lerror(0, "cannot open scratch file");
	    return 0;
    }
	    
    /* 
    ** Write initial record to file1 - this record will, at the end of
    ** lint, contain the lengths of each of the segments of the file.
    */
    if (fwrite((char *)xl_t_flens(&lout), sizeof(FLENS), 1, file1) != 1) {
	lerror(0,"cannot write to output file 1");
	return 0;
    }

    /* 
    ** Write initial record to file4 - this file contains strings that will
    ** be used in the printf/scanf checking in pass2.  The extra field of
    ** each record contains the offset into this segment of the file.
    ** lint doesn't want offsets of 0, because 0 has a special meaning,
    ** so put "junk" in the file to insure this.
    */
    if (fputs("012345679",file4) == NULL) {
	lerror(0,"cannot write to string file");
	return 0;
    }

    return 1;
}



/*
** Clean up after processing is complete.
** Put an end marker at the end of each of the four files.
** Write out into the initial record in file1 the lengths of each of the
** four files.
*/
int
ln_cleanup()
{
    int rval=0;
    int getpid();
    FLENS lout;
    LNBUG(ln_dbflag > 1, ("ln_cleanup"));

    /*
    ** Check to see if the IKWID or LINTED directive was specified,
    ** but never used.
    ** Reset the directive before calling the warning routine to prevent
    ** the message coming out twice.
    */
    if (LN_DIR(IKWID) > 0) {
	int igline = LN_DIR(IKWID);
	LN_DIR(IKWID) = 0;
	WERRORLN(igline, "lint suppression directive not used");
    }

    ln2_endmark();	/* put a "end-marker" at the end of each pass */

    /*
    ** Write out the intermediate file lengths, the id of this file, and
    ** what version of lint is being used.
    */
    lout.f1 = ftell(file1);
    lout.f2 = ftell(file2);
    lout.f3 = ftell(file3);
    lout.f4 = ftell(file4);
    lout.ver = LINTVER;
    lout.mno = getpid();

    (void)fseek(file1, 0, 0);
    if (fwrite((char *)xl_t_flens(&lout), sizeof(FLENS), 1, file1) != 1) {
	lerror(0,"cannot write to output file 1");
	rval = 1;
    }

    (void)fclose(file1);
    (void)fclose(file2);
    (void)fclose(file3);
    (void)fclose(file4);
    hdrclose();
    if (LN_FLAG('R'))
	cx_end();
    return rval;
}
