/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/directives.c	1.9"
#include "p1.h"
#include <string.h>

/* 
** Lint directive processing.
**
** ENTRY POINTS:
**	ln_initdir();
**	ln_setflags();
*/

/* Number of directives */
#define NUMDIRECTS	11

/* Maximum length of a lint directive */
#define MAXDIRLEN	20

/* Return values from ln_direct() */
#define IKWID_DIRECT	2
#define NORM_DIRECT	1
#define NO_DIRECT	0


static char baduse[]=
	"directive must be outside function: /* %s */";
static char badcombo[]=
	"directives can't be used together: /* PRINTFLIKE */ /* SCANFLIKE */";
static char implies[]=
	"dubious use of /* VARARGS */ with: /* %s */";
static char needs_arg[]=
	"directive requries argument: /* %s */";

/*
** Array to hold values of lint directives.
*/
int	ln_directs[NUMDIRECTS];

#ifdef __STDC__
static int ln_getnum(char *);
static char *ln_getword(char *, char []);
static void print_ikwid(char *);
static int ln_direct(char *);
static int ln_chkusage(int, char *, int, int);
#else
static int ln_getnum();
static char *ln_getword();
static void print_ikwid();
static int ln_direct();
static int ln_chkusage();
#endif



/*
** Initialize the values of directives that are not 0 -
** ARGSUSED, VARARGS, PRINTFLIKE, and SCANFLIKE have the
** value set to -1 unless the directive was seen.
*/
void
ln_initdir()
{
    LN_DIR(ARGSUSED) = LN_DIR(VARARGS) = 
	LN_DIR(PRINTFLIKE) = LN_DIR(SCANFLIKE) = -1;
}



/*
** ln_getnum - used for processing of lint directives (comments)
**
** return the optional number after a lint directive 
*/
static int
ln_getnum(str)
char *str;
{
    long result;
#ifndef __STDC__
    long strtol();
#endif
    char *pt;

    result = strtol(str, &pt, 10);
    LNBUG(ln_dbflag > 1, ("ln_getnum: %d", result));
    if (pt[0] != '\0')
	return -1;
    return result;
}



/*
** ln_getword - return a word in a comment
*/
static char *
ln_getword(from, to)
char *from;
char to[];
{
    int st, len;
    LNBUG(ln_dbflag > 1, ("ln_getword"));

    len = strcspn(from + (st=strspn(from, " \t\n")), " \t\n*");
    if (len == 0)
	return NULL;
    if (len > MAXDIRLEN)
	return NULL;
    strncpy(to, from + st, len);
    to[len] = '\0';
    return from + len + st;
}



/*
** ln_setflags - called from lex part
**
** Set a lint directive flag if one of the special comments was found
*/
/* ARGSUSED */
void
ln_setflags(str, len)
char *str;
unsigned int len;
{
    char wrd[MAXDIRLEN+1];	/* +1 for NULL */
    int res;
    LNBUG(ln_dbflag > 1, ("ln_setflags: %d", len));

    /* 
    ** Skip over the initial '/' and '*'
    */
    str += 2;

    str = ln_getword(str, wrd);
    while ((str != NULL) && (res = ln_direct(wrd)))
	if (res == IKWID_DIRECT) {
	    if (LN_FLAG('k'))
		print_ikwid(str);
	    else LN_DIR(IKWID) = er_getline();
	    return;
	} else str = ln_getword(str, wrd);
    return;
}


static void
print_ikwid(str)
char *str;
{
    int i=0;
    LNBUG(ln_dbflag > 1, ("print_ikwid: %s", str));

    while ((str[i] != '*') && (str[i+1] != '/'))
	i++;
    str[i] = '\0';
    lierror(str+1);
    str[i] = '*';
}


static int
ln_direct(str)
char *str;
{
    int num, rval = NO_DIRECT;
    int infunc = (ln_curfunc() != SY_NOSYM);
    LNBUG( ln_dbflag > 1, ("ln_direct: %s", str));

    switch (str[0]) {
	case 'A' :
	    if ((strncmp(str, "ARGSUSED", 8)==0) && 
		((num=ln_getnum(str+8)) != -1)) {
		if (infunc)
		    WERROR(baduse, "ARGSUSED");
		else {
		    LN_DIR(ARGSUSED) = num;
		    rval = NORM_DIRECT;
		}
	    }
	    break;

	case 'C' :
	    if ((strcmp(str,"CONSTANTCONDITION") == 0) || 
		(strcmp(str,"CONSTCOND") == 0)) {
		LN_DIR(CONSTANTCONDITION) = 1;
		rval = NORM_DIRECT;
	    }
	    break;

	case 'E' : {
	    extern int ln_elseflg, ln_ifflg;
	    if (strcmp(str,"EMPTY") == 0) {
		if (ln_elseflg || ln_ifflg) {
		    LN_DIR(EMPTY) = 1;
		    rval = NORM_DIRECT;
		} else WERROR("directive must be in if/else: /* EMPTY */");
	    }
	    break;
	}

	case 'L' :
	    if (strcmp(str,"LINTLIBRARY") == 0) {
		if (LN_FLAG('W'))
		    break;		/* ignore LINTLIBRARY for cflow */
		if (infunc)
		    WERROR(baduse, "LINTLIBRARY");
		else {
		    LN_DIR(LINTLIBRARY) = 1;
		    rval = NORM_DIRECT;
		}
	    } else if (strncmp(str, "LINTED", 6) == 0)
		rval = IKWID_DIRECT;
	    break;

	/*
	** NOTREACHED actually does two (opposite) things:
	** 1) it says the the user knows the next statement was not
	**    reached, therefore don't tell me what I already know.
	** 2) it says that the next statement is not reached
	**    (lint doesn't know it otherwise), so don't tell me
	**    "function falls off bottom w/o returning value"
	*/
	case 'N' :
	    if (strcmp(str,"NOTREACHED") == 0) {
		if (! infunc)
		    WERROR("must be used within function: /* NOTREACHED */");
		else {
		    sm_lnrch();
		    LN_DIR(NOTREACHED) = 1;
		    rval = NORM_DIRECT;
		}
	    }
	    break;

	case 'P' :
	    if (strncmp(str,"PROTOLIB", 8) == 0) {
		num = ln_getnum(str+8);
		if (num != -1) {
		    if ((num == 0) || (num == 1))
			LN_DIR(PROTOLIB) = num;
		    else WERROR("bad argument to /* PROTOLIB */");
		    if (! LN_DIR(LINTLIBRARY))
			WERROR("/* PROTOLIB */ can only be used with /* LINTLIBRARY */");
		    else rval = NORM_DIRECT;
		}
	    }

	    else if ((strncmp(str,"PRINTFLIKE",10)==0) &&
		((num=ln_getnum(str+10)) != -1)) {
		rval = ln_chkusage(PRINTFLIKE,"PRINTFLIKE",num,infunc);
	    }
	    break;

	case 'S' :
	    if ((strncmp(str,"SCANFLIKE",9)==0) && 
		((num=ln_getnum(str+9)) != -1)) {
		rval = ln_chkusage(SCANFLIKE,"SCANFLIKE",num,infunc);
	    }
	    break;

	case 'V' :
	    if ((strncmp(str,"VARARGS",7) == 0) && 
		((num=ln_getnum(str+7)) != -1)) {
		if (LN_DIR(SCANFLIKE) > 0)
		    WERROR(implies, "SCANFLIKE");
		else if (LN_DIR(PRINTFLIKE) > 0)
		    WERROR(implies, "PRINTFLIKE");
		else if (infunc)
		    WERROR(baduse, "VARARGS");
		else {
		    LN_DIR(VARARGS) = num;
		    rval = NORM_DIRECT;
		}
	    }
	    break;

	case 'I' :
	    /* I Know What I'm Doing */
	    if (strncmp(str,"IKWID", 5) == 0)
		rval = IKWID_DIRECT;
	    break;

	case 'F' :
	    if ((strncmp(str,"FALLTHRU",8) == 0) || 
		(strncmp(str,"FALLTHROUGH",11) == 0)) {
		LN_DIR(FALLTHRU) = 1;
		rval = NORM_DIRECT;
	    }
	    break;
    }
    return rval;
}


static int
ln_chkusage(direct, str, num, infunc)
int direct;
char *str;
int num;
int infunc;
{
    int rval = NORM_DIRECT;

    LN_DIR(direct) = num;
    if (num <= 0) {
	WERROR(needs_arg, str);
	LN_DIR(direct) = -1;
	return NO_DIRECT;
    }
    if (LN_DIR(direct == PRINTFLIKE ? SCANFLIKE : PRINTFLIKE) > 0) {
	WERROR(badcombo);
	LN_DIR(direct) = -1;
	return NO_DIRECT;
    }
    if (infunc) {
	WERROR(baduse, str);
	LN_DIR(direct) = -1;
	return NO_DIRECT;
    }
    if (LN_DIR(VARARGS) != -1) {
	WERROR(implies, str);
	LN_DIR(VARARGS) = 1;
	return NO_DIRECT;
    }
    return rval;
}
