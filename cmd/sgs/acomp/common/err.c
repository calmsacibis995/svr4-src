/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/err.c	55.2"
/* err.c */

/* This module contains error reporting routines.
** stdout and stderr are purged around error messages so
** output and errors will appear adjacent in files.
*/

#include <stdio.h>
#include <string.h>
#include "p1.h"
#ifdef	MERGED_CPP
#include "interface.h"			/* cpp interface */
#endif

/* Sorry about this mess:  the idea here is to be able to take
** advantage of readonly data by putting error message strings
** in readonly space.  To do that we define function prototypes
** for werror(), uerror(), cerror() in err.h.  But that means
** some fancy footwork here in defining those functions.  We'll
** use a macro to define the function header, depending on
** whether this is an ANSI C environment or not.
*/

#ifdef	__STDC__
#include <stdarg.h>
#define	FUNHDR(name,arg1name) \
name(const char *arg1name,...) { \
    va_list args; \
    va_start(args, arg1name);
#else	/* non-ANSI C case */
#include <varargs.h>
#define FUNHDR(name,arg1name) \
name(arg1name, va_alist) \
char *arg1name; \
va_dcl \
{ \
    va_list args; \
    va_start(args);
#endif

extern int elineno;			/* line number for errors */
int err_dump = 0;			/* dump core on internal error if 1 */
#ifdef	MERGED_CPP
/* CPP may pass null pointer for filename during start-up processing. */
static char * t_filename;
#define filename ((t_filename = PP_CURNAME()) ? t_filename : "")
					/* use preprocessor's idea of name */
#else
static char * filename = "";		/* current file's name */
#endif

/*ARGSUSED*/
void
er_filename(f, l)
char * f;
int l;
/* Set new filename string f, with line number l.  Because of the
** way the scanner works, set the internal line number to l-1.  If
** f is NULL, don't change the current filename.
*/
{
#ifndef	MERGED_CPP
    if (f)
	filename = st_lookup(f);
#endif
    elineno = l-1;
    return;
}


void
er_markline()
/* Print an assembly language line number marker. */
{
    CG_PRINTF(("%s -- %d \"%s\"\n", COMMENTSTR, elineno, filename));
    return;
}

#ifndef LINT
/* lint has its own routines for printing warning messages */

/* print a warning message */

void
FUNHDR(werror, fmt)
/* { */
    fflush(stdout);
    (void) fprintf(stderr, "\"%s\", line %d: warning: ", filename, elineno);
    (void) vfprintf(stderr, fmt, args);
    va_end(args);
    putc('\n', stderr);
    fflush(stderr);
    return;
}



/* print an error message */


void
FUNHDR(uerror, fmt)
/* { */
    fflush(stdout);
    (void) fprintf(stderr, "\"%s\", line %d: ", filename, elineno);
    (void) vfprintf(stderr, fmt, args);
    va_end(args);
    putc('\n', stderr);
    fflush(stderr);
    ++nerrors;
    return;
}

/* Print a warning with an explicit line number or current, if < 0. */

void
#ifdef __STDC__
wlerror(int lineno, const char *fmt,...) {
    va_list args;
    va_start(args, fmt);
#else	/* non-ANSI C case */
wlerror(lineno, fmt, va_alist)
int lineno;
char *fmt;
va_dcl
{
    va_list args;
    va_start(args);
#endif
    fflush(stdout);
    (void) fprintf(stderr, "\"%s\", line %d: warning: ", filename,
		lineno > 0 ? lineno : elineno);
    (void) vfprintf(stderr, fmt, args);
    va_end(args);
    putc('\n', stderr);
    fflush(stderr);
    return;
}


/* Print an error with an explicit line number or current, if < 0. */

void
#ifdef __STDC__
ulerror(int lineno, const char *fmt,...) {
    va_list args;
    va_start(args, fmt);
#else	/* non-ANSI C case */
ulerror(lineno, fmt, va_alist)
int lineno;
char *fmt;
va_dcl
{
    va_list args;
    va_start(args);
#endif
    fflush(stdout);
    (void) fprintf(stderr, "\"%s\", line %d: ", filename,
		lineno > 0 ? lineno : elineno);
    (void) vfprintf(stderr, fmt, args);
    va_end(args);
    putc('\n', stderr);
    fflush(stderr);
    ++nerrors;
    return;
}
#endif
/* endif ifdef LINT */

/* Internal error print. */

void
FUNHDR(cerror,fmt)
/* Internal compiler error.  If we get here and there have been
** user errors, assume that they led to the internal error, and
** just say goodbye.  Otherwise print a fatal error and exit.
** In debugging mode, always print the internal diagnostic for
** reference purposes.
*/
/* { */
#ifdef	NODBG
    if (nerrors == 0)
#endif
    {
	if (nerrors != 0) {
	    putc('[', stderr);
	}

	(void) fprintf(stderr, "\"%s\", line %d: internal compiler error:  ",
		    filename, elineno);
	(void) vfprintf(stderr, fmt, args);
	if (nerrors != 0) {
	    putc(']', stderr);
	}
	putc('\n', stderr);
    }
    va_end(args);

    if (nerrors != 0)
	UERROR("cannot recover from previous errors");
    else if (err_dump)
	abort();
 
    {
	extern void earlyexit();
	earlyexit();
    }
    /*NOTREACHED*/
}

void
yyerror(s)
const char *s;
/* Stub routine for yacc's error reporting. */
{
    char * token;
    unsigned int toklen;
    lx_errtoken(&token, &toklen);	/* get error token */
    UERROR("%s before or at: %.*s", s, toklen, token);
    return;
}

#ifndef	NODBG

void
FUNHDR(dprintf, fmt)
/* { */
    (void) vfprintf(stderr, fmt, args);
    va_end(args);
    return;
}

#endif	/*NODBG*/


/*
** Return to lint the current line number.
*/
int
er_getline()
{
    return elineno;
}

#ifdef LINT

/*
** Return to lint the current file name.
*/
char *
er_curname()
{
    return filename;
}
#endif
