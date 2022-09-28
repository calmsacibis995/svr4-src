/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/messages.c	1.8"
#include "p1.h"
#include <stdio.h>
#include <varargs.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#define DPRINTF dprintf

#define iscfile(x)	(! strcmp(x, sourcename))
#define WARN 0
#define ERR  1
#define INFO 2
#define BUFSIZE 1024
#define MAXHDRS 200
#define COLS	4		/* 4 columns per line */

static void buferr();
#ifndef LINT2
static void hdrinc();
static void bufhdr();
static void prerror();
static int  use_ikwid();
#endif
extern int unlink();

/*
** Routines in this file are used for both passes of lint.
** Both passes buffer in the same manner except that lint1 buffers header
** (include) messages and lint2 does not (not applicable).
**
** Some parts are "ugly" with all the #ifndef LINT2 - sorry.
*/

#ifndef LINT2
extern int 	nerrors;
extern char	*htmpname; 		/* buffered messages go here    */
static FILE	*htmpfile=NULL;	
extern char	*hdrs;			/* names of included files here */
static FILE	*hdrsfile=NULL;
extern char	*sourcename;		/* file name on command line	*/
static HDRITEM	*hdrlist;
static int	hdrnum = 0;
static int	hdrmax = MAXHDRS;
#endif

static char 	*ctmpname;
static FILE	*ctmpfile;

static struct {
	CITEM *start;
	CITEM *last;
} msgoff[NUMMSG];



/*
** tmpopen -
**
** open source message buffer file for writing
** If not lint2:
** 	open header message file for updating
**	if open fails, open for writing
*/
void
tmpopen()
{
    int i;
    LNBUG(ln_dbflag > 1, ("tmpopen"));

    /* initialize all pointers to null - nothing read/written yet */
    for (i=0;i<NUMMSG;i++)
	msgoff[i].start = msgoff[i].last = NULL;

    ctmpname = tmpnam(NULL);

    if ((ctmpfile = fopen(ctmpname, "w")) == NULL)
	lerror(FATAL,"cannot open message buffer file");
#ifndef LINT2
    if ( ( hdrlist = (HDRITEM*)malloc(MAXHDRS * sizeof(HDRITEM)) ) == NULL ) 
	lerror(FATAL,"cannot allocate space for header list");
        
    if (! LN_FLAG('F'))
	sourcename = strip(sourcename);
    /* 
    ** htmpfile contains the messages that come from files that were #include'd
    */
    if ((htmpfile = fopen(htmpname, "a")) == NULL)
	lerror(FATAL,"cannot open header file: %s\n", htmpname);

    /*
    ** hdrsfile contains the list of files that were #include'd, and what file
    ** each was first #include'd in.
    ** If the file does not exist, then there were no #include'd files yet.
    */
    if ((hdrsfile = fopen(hdrs, "r")) != NULL) {
	struct stat st;
	char *buf;

	if (stat(hdrs, &st))
	    lerror(FATAL,"can't stat header name file");
	if ((buf=malloc(sizeof(char) * st.st_size))==NULL)
	    lerror(FATAL,"can't malloc space for headers");
	fread(buf, sizeof(char), st.st_size, hdrsfile);
	(void)fclose(hdrsfile);
	for (;;) {
	    hdrlist[hdrnum].hname = buf;
	    if ((buf=strchr(buf,'\0'))==NULL)
		lerror(FATAL,"flakey header name file");
	    hdrlist[hdrnum].sname = ++buf;
	    if ((buf=strchr(buf,'\0'))==NULL)
		lerror(FATAL," flakey header name file");
	    ++buf;
	    hdrinc();
	    if (*buf == '\0')
		break;
	}
    }
#endif
}

#ifndef LINT2

static void
hdrinc()
{
    if ( ++hdrnum >= hdrmax ) {
    	hdrmax += MAXHDRS;   
   	hdrlist = (HDRITEM*) realloc((char*) hdrlist, hdrmax * sizeof(HDRITEM) );
	if ( hdrlist == NULL)
		lerror(FATAL, "can't reallocate space for header list");
    }
}

#endif


/*
** hdrclose 
** If not lint2:
** write header file name/count to header message buffer file, then close file.
** Dump out source messages.
*/
void
hdrclose()
{
    int i;
#ifdef __STDC__
    int unlink(const char *);
#else
    int unlink();
#endif
    LNBUG(ln_dbflag > 1, ("hdrclose"));

#ifndef LINT2
    (void)fclose(htmpfile);
    if ((hdrsfile = fopen(hdrs, "w")) == NULL)
	lerror(FATAL,"cannot write headers name file");
    for (i=0; i < hdrnum; i++) {
	fprintf(hdrsfile,"%s%c%s%c",hdrlist[i].hname,'\0',
			hdrlist[i].sname,'\0');
    }
    putc('\0',hdrsfile);
    (void)fclose(hdrsfile);
#endif

    (void)fclose(ctmpfile);

    /* reopen source file for reading */
    if ((ctmpfile=fopen(ctmpname, "r")) == NULL)
	lerror(HCLOSE|FATAL,"cannot read message buffer file");
  
    for (i=0;i < NUMMSG;i++) {
	CITEM * ptr=msgoff[i].start;

	/* Not all messages will have been seen - if the start is still
	** null, then we know that message has not occurred, so skip
	** to the next one.
	*/
	if (ptr != NULL)
	    printf("\n%s\n", msg[i].compound);

	if (msg[i].colform == LONE) {
	    char s[BUFSIZE];
	    while (ptr != NULL) {
		(void)fseek(ctmpfile, ptr->offset, 0);
		(void) fgets(s, BUFSIZE, ctmpfile);

		/* the index into the file is not valid - lint dies */
		if (s == NULL)
		    lerror(HCLOSE|FATAL,"bad index into message buffer file");
		printf("    %s",s);
		ptr=ptr->next;
	    }
	} else if (msg[i].colform == MULTI) {
	    char s[BUFSIZE];
	    int i,j=0;
	    while (ptr != NULL) {
		(void)fseek(ctmpfile, ptr->offset, 0);
		(void) fgets(s, BUFSIZE, ctmpfile);

		/* the index into the file is not valid - lint dies */
		if (s == NULL)
		    lerror(HCLOSE|FATAL,"bad index into message buffer file");
		else if (s[(i=strlen(s)-1)] == '\n')
		    s[i] = '\0';
		else lerror(0,"missing new line in buffered file");

		if (++j == COLS) {
		    printf("    %s",s);
		    if (ptr != NULL) {
			j = 0;
			putc('\n', stdout);
		    }
		} else printf("    %-16s",s);
		ptr=ptr->next;
		if (ptr == NULL) {
		    if (j == COLS)
			putc('\n',stdout);
		    putc('\n',stdout);
		}
	    }
	} else lerror(HCLOSE|FATAL,"bad colform item in msgbuf.c");
    }

    (void)fclose(ctmpfile);
    if (unlink(ctmpname))
	lerror(0,"couldn't remove file");
}



/*
** lerror -
** 	lint error message routine
**
** As with cerror, assume that if there were lint internal errors,
** they came about from the previous errors.  Only print them if
** debugging is on, and print within "[]"
*/
/* VARARGS */
void
lerror(va_alist)
va_dcl
{
    va_list args;
    int code;
    char *fmt;
#ifndef LINT2
    char *ln_filename = er_curname();
    int lineno = er_getline();
    char *fname;
#endif
    LNBUG(ln_dbflag > 1, ("lerror"));

#if defined(NODBG) && !defined(LINT2)
    if (nerrors == 0)
#endif
    {
#ifndef LINT2
	if (nerrors != 0) {
	    putc('[', stderr);
	}
#endif

	va_start(args);
	code = va_arg(args, int);
	fmt = va_arg(args, char *);

#ifndef LINT2
	if (LN_FLAG('F'))
	    fname = ln_filename;
	else fname = strip(ln_filename);

	if (LN_FLAG('s'))
	    fprintf(stderr,"\"%s\", line %d: lint error: ", fname, lineno);
	else fprintf(stderr,"(%d) lint error: ", lineno);
#else
	fprintf(stderr,"lint error: ");
#endif
	vfprintf(stderr, fmt, args);

#ifndef LINT2
	if (nerrors != 0)
	    putc(']', stderr);
#endif
	fprintf(stderr,"\n");

	if ((code & CCLOSE) && (ctmpfile != NULL)) {
	    (void)fclose(ctmpfile);
	    (void) unlink(ctmpname);
	}
#ifndef LINT2
	if ((code & HCLOSE) && (htmpfile != NULL)) {
	    (void)fclose(htmpfile);
	    (void) unlink(htmpname);
	}
#endif
	va_end(args);

	if (code & FATAL)
	    exit(FATAL);
    }
}





/*
** Buffer a message.
*/
/* VARARGS */
void
bwerror(va_alist)
va_dcl
{
    va_list args;
    LNBUG(ln_dbflag > 1, ("bwerror"));

    va_start(args);
#ifndef LINT2
    buferr(er_getline(), args);
#else
    buferr(args);
#endif
    va_end(args);
}



#ifndef LINT2
/*
** Buffer a message, but use line 'line'
*/
/* VARARGS */
void
bwerrorln(line, va_alist)
int line;
va_dcl
{
    va_list args;
    LNBUG(ln_dbflag > 1, ("bwerrorln"));

    va_start(args);
    buferr(line, args);
    va_end(args);
}
#endif
    


/*
** buferr
**	buffer a message
*/
/* VARARGS */
static void
#ifndef LINT2
buferr(line, args)
int line;
#else
buferr(args)
#endif
va_list args;
{
    int indx;
#ifndef LINT2
    char *fname;
    char *ln_filename = er_curname();
#endif
    LNBUG(ln_dbflag > 1, ("buferr"));

#ifndef LINT2
    if (! use_ikwid(line, WARN))
	return;

    if (LN_FLAG('F'))
	fname = ln_filename;
    else fname = strip(ln_filename);
#endif

    indx = va_arg(args, int);

    /*
    ** if the -s flag was used, print the message to stderr immediately with
    ** no buffering - doesn't matter if file is simple/compound or 
    ** include/normal
    */
    if (LN_FLAG('s')) {
#ifndef LINT2
	printf("\"%s\", line %d: warning: ", fname, line);
#endif
	(void) vprintf(msg[indx].simple, args);
	putc('\n', stdout);
    } else {
#ifndef LINT2
	if ((iscfile(fname)) || (ln_filename[0] == NULL))  {
#endif
	    if (msgoff[indx].start == NULL)
		msgoff[indx].start = 
			msgoff[indx].last=(CITEM *)malloc(sizeof(CITEM));
	    else {
		msgoff[indx].last->next = (CITEM *)malloc(sizeof(CITEM));
		msgoff[indx].last = msgoff[indx].last->next;
	    }
	    msgoff[indx].last->next = NULL;
	    msgoff[indx].last->offset = ftell(ctmpfile);
#ifndef LINT2
	    (void) fprintf(ctmpfile, "(%d) ", line);
#endif
	    (void) vfprintf(ctmpfile, msg[indx].format, args);
	    putc('\n', ctmpfile);
#ifndef LINT2
	 } else bufhdr(fname, line, msg[indx].simple, WARN, args);
#endif
    }
}



#ifndef LINT2
/* VARARGS */
void
lwerror(va_alist)
va_dcl
{
    va_list args;
    LNBUG(ln_dbflag > 1, ("lwerror"));

    va_start(args);
    prerror(WARN, er_getline(), args);
    va_end(args);
}


/* VARARGS */
void
lwlerror(line, va_alist)
int line;
va_dcl
{
    va_list args;
    LNBUG(ln_dbflag > 1, ("lwlerror"));

    va_start(args);
    prerror(WARN, ( line > 0 ? line : er_getline() ), args);
    va_end(args);
}



/* VARARGS */
void
luerror(va_alist)
va_dcl
{
    va_list args;
    LNBUG(ln_dbflag > 1, ("luerror"));
    
    va_start(args);
    prerror(ERR, er_getline(), args);
    va_end(args);
    ++nerrors;
}

/* VARARGS */
void
lulerror(line, va_alist)
int line;
va_dcl
{
    va_list args;
    LNBUG(ln_dbflag > 1, ("lulerror"));
    
    va_start(args);
    prerror(ERR, ( line > 0 ? line : er_getline() ), args);
    va_end(args);
    ++nerrors;
}



/*
** lierror doesn't really take a variable # of args, but because the
** prerror function expects its arguments as a va_list, lierror() will
** be declared as such.
*/
/* VARARGS */
void
lierror(va_alist)
va_dcl
{
    va_list args;
    LNBUG(ln_dbflag > 1, ("lierror"));

    va_start(args);
    prerror(INFO, er_getline(), args);
    va_end(args);
} 



static void
bufhdr(fname, line, fmt, code, args)
char *fname, *fmt;
int line;
int code;
va_list args;
{
    LNBUG(ln_dbflag > 1, ("bufhdr"));

    /*
    ** if fname matches the last #included file AND the sourcename
    ** matches the last sourcename, buffer the message
    */
    if ((hdrnum != 0) && (strcmp(fname, hdrlist[hdrnum-1].hname) == 0) &&
	(strcmp(sourcename, hdrlist[hdrnum-1].sname) == 0)) {

	  if (code == WARN)
		(void) fprintf(htmpfile, "(%d) warning: ", line);
	  else if (code == ERR)
		(void) fprintf(htmpfile, "(%d) error: ", line);
	  else (void) fprintf(htmpfile, "(%d) info: ", line);

	  (void) vfprintf(htmpfile, fmt, args);
	  putc('\n', htmpfile);
	  (void) fflush(htmpfile);
    }

    /*
    ** Check to see if this #included file has been seen before
    ** If so, don't buffer it.  If not (i==hdrnum), then add the
    ** file name to the list, and buffer the message.
    */
    else {
 	int i=0;
	while ((i != hdrnum) && (strcmp(fname, hdrlist[i].hname) != 0))
	    i++;

	if (i == hdrnum) {
	    hdrlist[i].hname = 
		(char *) malloc(sizeof(char)*(strlen(fname)+1));
	    strcpy(hdrlist[i].hname, fname);
	    hdrlist[i].sname = sourcename;
	    hdrinc();

	    fprintf(htmpfile,"\n\n%s (as included in %s)\n", fname, sourcename);
	    fprintf(htmpfile,"=================\n");

	    if (code == WARN)
		(void) fprintf(htmpfile, "(%d) warning: ", line);
	    else if (code == ERR)
		(void) fprintf(htmpfile, "(%d) error: ", line);
	    else (void) fprintf(htmpfile, "(%d) info: ", line);

	    (void) vfprintf(htmpfile, fmt, args);
	    putc('\n', htmpfile);
	    (void) fflush(htmpfile);
	}
    }
}



/* prerror -
**
**	code is INFO for "(%d) info:"
**		WARN for "(%d) warning:"
**		ERR  for "(%d) error:"
**
**	out is the file to print to
**
**	va_alist is the format to print, and list of arguments
*/
static void
prerror(code, line, args)
int code;
int line;
va_list args;
{
    char *fname, *fmt;
    char *ln_filename = er_curname();
    LNBUG(ln_dbflag > 1, ("prerror: %s", ln_filename));

    if (! use_ikwid(line, code))
	return;
	
    fmt = va_arg(args, char *);

    if (LN_FLAG('F'))
	fname = ln_filename;
    else fname = strip(ln_filename);

    if (!LN_FLAG('s') && (ln_filename[0] != NULL) && (! iscfile(fname))) {
	bufhdr(fname, line, fmt, code, args);
	return ;
    }

    if (LN_FLAG('s'))
	printf("\"%s\", line %d: ", fname, line);
    else printf("(%d) ", line);
    printf("%s: ", (code==WARN) ? "warning" : (code==ERR) ? "error" : "info");

    vprintf(fmt, args);
    va_end(args);
    putc('\n', stdout);
}



/* strip -
** strip s to the basename
*/
char *
strip(s) 
char *s;
{
    static char *p;

    p = strrchr(s, '/');
    if (p == NULL)
	return s;
    else return (p+1);
}

static int
use_ikwid(line, code)
int line;
int code;
{
    int igline = LN_DIR(IKWID);
    LNBUG(ln_dbflag > 1, ("use_ikwid: %d %d %d", line, igline, code));

    /*
    ** The IKWID directive was not used, just return.
    */
    if (! igline)
	return 1;

    /*
    ** At this point, if igline is positive, the directive hadn't been
    ** used yet.  If it is negative, it has been used.
    */

    /*
    ** IKWID is not allowed on error messages.
    ** Complain if the code is ERR, and the directive is immediately
    ** above the error, and it wasn't used for anything else.
    */
    if ((code == ERR) && (igline == (line - 1))) {
	LN_DIR(IKWID) = 0;
	WERRORLN(igline,"lint suppression directive ignored on error message");
	return 1;
    }

    /*
    ** If the line containing the IKWID is the line immediately above
    ** the line with the warning, suppress the message, and set igline
    ** to the negative.  This is done so that lint knows the directive
    ** was used, and so it can warn when the directive was not used
    ** (only complain about unused IKWID directive if it was positive).
    */
    if (abs(igline) == (line - 1)) {
	LN_DIR(IKWID) = -abs(igline);
	return 0;
    } else if (igline > 0) {
	LN_DIR(IKWID) = 0;
	WERRORLN(igline,"lint suppression directive not used");
	return 1;
    } 

    /*
    ** There was an IKWID previously that was used, but it doesn't
    ** apply to this message (or any future ones.)
    ** Reset the directive.
    */
    LN_DIR(IKWID) = 0;
    return 1;
}
#endif
