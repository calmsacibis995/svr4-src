/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/cxref.c	1.10"
#include "p1.h"
/* #include "cxref.h" */
#include <string.h>

/*
** This file contains routines to write information to an intermediate
** file for CXref.
*/
static FILE *cxfile;
static FILE *cxtmpfile;
static char *cxtmpname;
#ifndef NODBG
int cx_debug = 0;
#endif

static infunc = 0;
static int sameas();


/*
** Initialize intermediate cxref file.  "opt" should be the name of the
** file.  Two files will be opened.  The final output file itself
** (file.lnt), and a temp file.
** This is done because certain information must come first in the
** cxref file (information about statics); when that information is
** seen, "fake" declarations are added to the cxref file; i.e.
**	fun(){
**	    foo();
**	}
**	static int foo() {}
** 
** Before cxref can add foo() to its symbol table, it must know it is
** static. When all done, the temp file is added to the end of 
** the temp file.
*/
void
cx_init(opt)
char *opt;
{
    char *fname;
    char *cxname;

    fname = (char *) malloc(strlen(opt) + 5);
    (void) strcpy(fname, opt);
    (void) strcat(fname, ".lnt");
    if ((cxfile = fopen(fname, "w")) == NULL)
	lerror(FATAL, "can't open intermediate cxref file");
    cxname = tmpnam(NULL);
    cxtmpname = (char *) malloc(strlen(cxname) + 1);
    strcpy(cxtmpname, cxname);
    if ((cxtmpfile = fopen(cxtmpname,"w")) == NULL)
	lerror(FATAL, "can't open temp file for cxref");
}



/*
** Check to see if the filename has changed - if so, write a NEWFILE
** record and the new filename.
*/
static void
cx_file()
{
    static char *fname = " ";
    char *f = er_curname();

    if (! LN_FLAG('F'))
	f = strip(f);

    if (strcmp(fname, f)) {
	fname = st_lookup(f);
	fprintf(cxtmpfile, "N%s\n", fname);
	LNBUG(cx_debug, ("N%s\n", fname));
    }
}



#define MAX_PARAM 100
/*
** Write info about identifiers
*/
void
cx_ident(sid, def)
SX sid;
int def;
{
    int rec, lev;
    I7 class;
    static SX *sxary;		/* pointer to parameter list	*/
    static int sptr = 0;	/* pointer into array of params */
    static int max_param = 0;	/* size of parameter list 	*/
    LNBUG(cx_debug > 1, ("cx_ident: %d %d", sid, def));

    /*
    ** If this is a function, and a def > 0, then return.
    ** Only print out a "F" record when def is negative.
    ** This negative value will be used as the line number.
    ** This is done to get the line number of the function
    ** to be on the opening brace.
    */
    if ((def > 0) && TY_ISFTN(SY_TYPE(sid)))
	return;

    sid = sameas(sid);
    lev = SY_LEVEL(sid);
    class = SY_CLASS(sid);

    /*
    ** Function parameter - buffer it for now.
    */
    if (lev == 1) {

	/* Too many parameters - expand array. */
	if (sptr >= max_param) {
	    max_param += MAX_PARAM;

	    /* for first time, use malloc; o/w use realloc */
	    if (max_param == MAX_PARAM)
		sxary = (SX *) malloc(sizeof(SX) * max_param);
	    else
		sxary = (SX *) realloc((char *)sxary, sizeof(SX)*max_param);
	}
	sxary[sptr++] = sid;
	return ;
    }

    /*
    ** If the level of the sid is 0, and the class is known to be
    ** static, then make the level -1.
    ** If the sid is a function and the above is not true, then
    ** make the level 0.
    ** In all other cases leave the level the same.
    */
    if ((class == SC_STATIC) && (lev == 0))
	lev = -1;
    else if (TY_ISFTN(SY_TYPE(sid)) || (class == SC_EXTERN))
	lev = 0;

    /*
    ** If this is a real definition, or the class is auto, then
    ** make this a definitions - for functions, make a 'F'; others
    ** are 'D'.
    ** All others are declarations - 'L'.
    */
    if (def || (class == SC_AUTO)) {
	if (TY_ISFTN(SY_TYPE(sid)))
	    rec = 'F';
	else rec = 'D';
    } else rec = 'L';

    /*
    ** Check to see if the filename has changed.
    */
    cx_file();

    /*
    ** If this is a static function definition, add a "fake" declaration
    ** to the cxref file; line number of 0 means it isn't real.
    */
    if ((rec == 'F') && (lev == -1))
	fprintf(cxfile, "L 0 -1 %s\n", SY_NAME(sid));

    fprintf(cxtmpfile,"%c %d %d %s\n", rec, er_getline(),
		lev, SY_NAME(sid));

    LNBUG(cx_debug, ("%c %d %d %s\n", rec, er_getline(), lev, SY_NAME(sid)));

    if (def && TY_ISFTN(SY_TYPE(sid))) {
	int i;
	infunc = 1;
	for (i=0;i<sptr;i++) {
	    fprintf(cxtmpfile,"D %d 1 %s\n", SY_LINENO(sxary[i]),
		SY_NAME(sxary[i]));
	    LNBUG(cx_debug, ("D %d 1 %s\n", SY_LINENO(sxary[i]),
			SY_NAME(sxary[i])));
	}
    }
    sptr = 0;
}

void
cx_cpp(namelen, name, line, code)
int namelen;
char *name;
long line;
int code;
{
    cx_file();
    fprintf(cxtmpfile, "%c %d 0 %.*s\n", code, (int) line, namelen, name);
    LNBUG(cx_debug, ("%c %d 0 %.*s\n", code, (int) line, namelen, name));
}


void
cx_cppfile(namelen, name, line, code, fname)
int namelen;
char *name;
long line;
int code;
char *fname;
{
    fprintf(cxtmpfile, "N%s\n", fname);
    LNBUG(cx_debug, ("N%s\n", fname));
    fprintf(cxtmpfile, "%c %d 0 %.*s\n", code, (int) line, namelen, name);
    LNBUG(cx_debug, ("%c %d 0 %.*s\n", code, (int) line, namelen, name));
}


static int curlev = 0;

/*
** A new block has begun.
*/
void
cx_inclev()
{
    curlev++;
}



/*
** A block has ended.
*/
void
cx_declev()
{
    curlev--;
    if ((curlev == 0) && (infunc)) {
	infunc = 0;
	fprintf(cxtmpfile,"E\n");
	LNBUG(cx_debug, ("E\n"));
    }
}



/*
** A symbol has been used.
*/
void
cx_use(sid, goal)
SX sid;
int goal;
{
    int lev;
    I7 class;

    sid = sameas(sid);
    lev = SY_LEVEL(sid);
    class = SY_CLASS(sid);

    if (TY_ISFTN(SY_TYPE(sid)) || (class == SC_EXTERN))
	lev = 0;

    cx_file();
    
    fprintf(cxtmpfile, "%c %d %d %s\n", (goal&ASSG) ? 'A' : 'R',
	er_getline(), lev, SY_NAME(sid));
    LNBUG(cx_debug, ("%c %d %d %s\n", (goal&ASSG) ? 'A' : 'R',
		er_getline(), lev, SY_NAME(sid)));
}
    


void
cx_end()
{
    char buf[BUFSIZ];
    int n;
    fclose(cxtmpfile);
    if ((cxtmpfile = fopen(cxtmpname, "r")) == NULL) {
	fprintf(stderr,"can't open %s for input\n", cxtmpname);
	exit(1);
    }

    for (;;) {
	n = fread(buf, sizeof(char), BUFSIZ, cxtmpfile);
	fwrite(buf, sizeof(char), n, cxfile);
	if (n != BUFSIZ)
	    break;
    }
    fclose(cxfile);
    fclose(cxtmpfile);
    if (unlink(cxtmpname))
	lerror(0,"couldn't remove cxtmpfile");
}


static int
sameas(sid)
SX sid;
{
    SX ssid;
    LNBUG(cx_debug > 1, ("level"));

    while (   ((ssid = SY_SAMEAS(sid)) != SY_NOSYM)
	   && (ssid != sid)
	  )
	sid = ssid;
    return sid;
}
