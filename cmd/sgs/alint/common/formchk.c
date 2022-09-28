/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/formchk.c	1.12"
#include <stdio.h>
#include <string.h>
#include "p1.h"
#include "lnstuff.h"
#include "lpass2.h"
#include "xl_byte.h"

#define POINTER(x)	-(x+1)

/*
** Routines to check the format strings and arguments of functions
** in the printf and scanf families.
**
** GLOBAL ENTRY POINTS:
**	fmtinit()
**	fmtcheck()
*/


#ifdef __STDC__
static char *getfmt(FILEPOS);
static void chkprintf(STAB *, int, char *);
static void chkscanf(STAB *, int, char *);
static void fmtarg(STAB *, int, int);
static int generic(int);
static TY mkgeneric(int);
static TY mkungeneric(int);
#else
static char *getfmt();
static void chkprintf();
static void chkscanf();
static void fmtarg();
static int generic();
static TY mkgeneric();
static TY mkungeneric();
#endif



/* 
** A negative entry in any of the tables means it expects a pointer
** to the type.
** 
** Some format strings allow only a specified type (unsigned int),
** while others allow any signed or unsigned version of a type.
** For those that allow any version, a "generic" type has been
** made that will match any signed or unsigned version of the
** type (don't confuse the generic type with the generic type of
** the compiler.)
** i.e.
**    LN_GINT will match LN_INT, LN_UINT, or LN_SINT
** Similarly:
**    POINTER(LN_GINT) matches POINTER(LN_INT), POINTER(LN_UINT), and
**        POINTER(LN_SINT)
*/
static TY	pftab[256], 	/* printf %  */
		pfhtab[128], 	/* printf %h */
		pfltab[128],	/* printf %l */
		pfLtab[128],	/* printf %L */
		sftab[256],	/* scanf %   */
		sfhtab[128],	/* scanf %h  */
		sfltab[128],	/* scanf %l  */
		sfLtab[128];	/* scanf %L  */
		

typedef struct {
	char fm_c;	/* conversion character			*/
	TY  fm_ty;	/* expected arg type for fm_c		*/
	TY  fm_hty;	/* expected type if preceded by 'h'	*/
	TY  fm_lty;	/* expected type if preceded by 'l'	*/
	TY  fm_Lty;	/* expected type if preceded by 'L'	*/
} Fmtinit;


static Fmtinit pinit[] = {
	{ 'd', LN_GINT, LN_GINT, LN_GLONG },
	{ 'i', LN_GINT, LN_GINT, LN_GLONG },
	{ 'o', LN_GINT, LN_GINT, LN_GLONG },
	{ 'u', LN_GINT, LN_GINT, LN_GLONG },
	{ 'x', LN_GINT, LN_GINT, LN_GLONG },
	{ 'X', LN_GINT, LN_GINT, LN_GLONG },

	{ 'f', LN_DOUBLE , 0, 0, LN_LDOUBLE },
	{ 'e', LN_DOUBLE , 0, 0, LN_LDOUBLE },
	{ 'E', LN_DOUBLE , 0, 0, LN_LDOUBLE },
	{ 'g', LN_DOUBLE , 0, 0, LN_LDOUBLE },
	{ 'G', LN_DOUBLE , 0, 0, LN_LDOUBLE },

	{ 'c', LN_GINT },
	{ 's', POINTER(LN_GCHAR) },
	{ 'p', POINTER(LN_VOID) },
	{ 'n', POINTER(LN_GINT) },

	{ 0 }
};

static Fmtinit sinit[] = {
	{ 'd', POINTER(LN_GINT), POINTER(LN_GSHORT), POINTER(LN_GLONG) },
	{ 'i', POINTER(LN_GINT), POINTER(LN_GSHORT), POINTER(LN_GLONG) },
	{ 'o', POINTER(LN_UINT), POINTER(LN_USHORT), POINTER(LN_ULONG) },
	{ 'u', POINTER(LN_UINT), POINTER(LN_USHORT), POINTER(LN_ULONG) },
	{ 'x', POINTER(LN_UINT), POINTER(LN_USHORT), POINTER(LN_ULONG) },
	{ 'X', POINTER(LN_UINT), POINTER(LN_USHORT), POINTER(LN_ULONG) },

	{ 'e', POINTER(LN_FLOAT), 0, POINTER(LN_DOUBLE), POINTER(LN_LDOUBLE) },
	{ 'E', POINTER(LN_FLOAT), 0, POINTER(LN_DOUBLE), POINTER(LN_LDOUBLE) },
	{ 'f', POINTER(LN_FLOAT), 0, POINTER(LN_DOUBLE), POINTER(LN_LDOUBLE) },
	{ 'g', POINTER(LN_FLOAT), 0, POINTER(LN_DOUBLE), POINTER(LN_LDOUBLE) },
	{ 'G', POINTER(LN_FLOAT), 0, POINTER(LN_DOUBLE), POINTER(LN_LDOUBLE) },

	{ 's', POINTER(LN_GCHAR) },
	{ '[', POINTER(LN_GCHAR) },
	{ 'c', POINTER(LN_GCHAR) },
	{ 'p', POINTER(LN_VOID) },
	{ 'n', POINTER(LN_GINT), POINTER(LN_GSHORT), POINTER(LN_GLONG) },

	{ 0 }
};



/*
** Initialize the 8 type tables for printf and scanf.
*/
void
fmtinit()
{
    int c,i;
    LNBUG(ln_dbflag > 1, ("fmtinit"));

    for (i=0; (c = pinit[i].fm_c) != 0; i++) {
	pftab[c] = pinit[i].fm_ty;
	pfhtab[c] = pinit[i].fm_hty;
	pfltab[c] = pinit[i].fm_lty;
	pfLtab[c] = pinit[i].fm_Lty;
    }
    for (i=0; (c = sinit[i].fm_c) != 0; i++) {
	sftab[c] = sinit[i].fm_ty;
	sfhtab[c] = sinit[i].fm_hty;
	sfltab[c] = sinit[i].fm_lty;
	sfLtab[c] = sinit[i].fm_Lty;
    }
}



/*
** Check format of entry just read in (in r.l), with the definition
** provided by the symbol table entry q.
*/
void
fmtcheck(q)
STAB *q;
{
    char *fmt;
    int arg = q->nargs; 
    ATYPE *a;
    LNBUG(ln_dbflag > 1, ("fmtcheck: %s %d %d", q->name, arg, r.l.nargs));

    /* 
    ** "function called with variable number of arguments"
    ** The number of arguments is less than the number required.
    */ 
    if (r.l.nargs < arg) {
	BWERROR(7, q->name, q->fno, q->fline, cfno, r.l.fline);
	return;
    }
    if (arg < 1)	/* should be caught in pass1 */
	return;

    /*
    ** Find the string in the string file.  If there is no string
    ** associated with the item just read in (in r), then return -
    ** no check can be made.
    */
    a = &getarg(arg-1);
    /* CONSTCOND */
    if (sizeof(a->extra.pos) != sizeof(a->extra.ty)) {
	/* reconvert the extra field since we first assumed it was a
	** ty.
	*/
	a->extra.ty = XL_XLATE(a->extra.ty);	/* undo the translation */
	a->extra.pos = XL_XLATE(a->extra.pos);
    }
    if (! (fmt = getfmt(getarg(arg-1).extra.pos)))
	return;
    if (q->decflag & LPF)
	chkprintf(q, arg, fmt);
    else chkscanf(q, arg, fmt);
}



/*
** Return a pointer to the string at offset 'a' of the current module.
*/
static char *
getfmt(a)
FILEPOS a;
{
    static char fmt[FMTSIZE];
    char *s=fmt;
    int c;
    FILEPOS pos;
    LNBUG(ln_dbflag > 1, ("getfmt: %ld", a));

    /*
    ** The string file starts at a positive non-zero offset.
    ** Anything else means a string was not passed to the function.
    */
    if (a <= 0)
	return 0;

    /*
    ** Save the current file position, and seek to the desired position.
    */
    pos = ftell(stdin);
    if (fseek(stdin, curpos + lout.f1 + lout.f2 + lout.f3 + a, 0)) {
	lerror(0,"cannot fseek string tables");
	(void)fseek(stdin, pos, 0);
	return 0;
    }

    /* 
    ** Get the string.
    */
    while (((c = fgetc(stdin)) != EOF) && c && (s < fmt+FMTSIZE-1))
	*s++ = (char) c;
    *s = '\0';

    /* return to old position */
    (void)fseek(stdin, pos, 0);
    LNBUG(ln_dbflag > 1, ("string: %s", fmt));
    return fmt;
}



/*
** Check for valid use of printf - this code based on a lint from 5ESS,
** with changes made for ANSI-C
*/
static void
chkprintf(q, arg, fmt)
STAB *q;
int arg;
char *fmt;
{
    int c;
    int dotflag, minusflag, plusflag, blankflag, sharpflag, zeroflag;
    TY ty;   
    LNBUG(ln_dbflag > 1, ("chkprintf: %s", fmt));

    while ((c = *fmt++) != '\0') {
	if (c != '%' || (c = *fmt++) == '%')
	    continue;

	dotflag = minusflag = plusflag = blankflag = sharpflag = zeroflag = 0;

	/*
	** Zero or more flags that modify the meaning of the
	** conversion specification.
	*/
	for (;;) {
	    switch (c) {
		case '-':
		    if (minusflag) goto bad;
		    minusflag = 1;
		    break;
		case '+':
		    if (plusflag) goto bad;
		    plusflag = 1;
		    break;
		case ' ':
		    if (blankflag) goto bad;
		    blankflag = 1;
		    break;
		case '#':
		    if (sharpflag) goto bad;
		    sharpflag = 1;
		    break;
		case '0':
		    if (zeroflag) goto bad;
		    zeroflag = 1;
		    break;
		default:
		    goto endfor;
	    }
	    c = *fmt++;
	}
	endfor:

	/*
	** An optional decimal integer specifying a minimum field width
	** (with an * indicating that an int argument supplies the
	** field width.)
	*/
	if (c == '*') {
	    fmtarg(q, arg++, LN_GINT);
	    c = *fmt++;
	} else for (; c >= '0' && c <= '9'; c = *fmt++);

	/*
	** An optional precision that gives the minimum number of digits
	** for the conversions, ..... The precision takes the form of a
	** period [.] followed by an optional decimal integer.
	*/
	if (c == '.') {
	    dotflag = 1;
	    if ((c = *fmt++) == '*') {
		fmtarg(q, arg++, LN_GINT);
		c = *fmt++;
	    } else for (; c >= '0' && c <= '9'; c = *fmt++);
	}

	ty = pftab[c];

	/*
	** An optional h specifying that a following d,i,o,u,x or X ...
	** An optional h specifying that a following n conversion ...
	** An optional l specifying that a following d,i,o,u,x or X ...
	** An optional l specifying that a following n conversion ...
	** An optional L specifying that a following e,E,f,g, or G ...
	*/
	if ((c == 'h') || (c == 'l') || (c == 'L'))
	    if (pftab[c = *fmt++])
		switch (fmt[-2]) {
		    case 'l' : ty = pfltab[c]; break;
		    case 'h' : ty = pfhtab[c]; break;
		    case 'L' : ty = pfLtab[c]; break;
		    default: lerror(0,"bad char in chkprintf"); return;
		}

	/*
	** The result is to be converted to "alternate form".  For
	** o ... x, X, ... E, E, f, g, and G ....
	** For other conversions, the behavior is undefined.
	*/
	if (sharpflag)
	    switch (c) {
		case 'o':  case 'x':  case 'X':  case 'e':  case 'E':  
		case 'f':  case 'g':  case 'G':
		    break;
		default:
		    goto bad;
	    }

	/*
	** For d, i, o, u, x, X, e, E, f, g, and G conversions .....
	** For other conversions, the behavior is undefined.
	*/
	if (zeroflag)
	    switch (c) {
		case 'd':  case 'i':  case 'o':  case 'u':  case 'x':
		case 'X':  case 'e':  case 'E':  case 'f':  case 'g':
		case 'G':
		    break;
		default:
		    goto bad;
	    }

	/*
	** An optional precision that gives ..... d, i, o, u, x, and
	** X conversions, the number of .... for e, E, and f conversions,
	** the maximum .... for g and G conversions, or the maximum ....
	** in s conversion.
	*/
	if (dotflag)
	    switch (c) {
		case 'd':  case 'i':  case 'o':  case 'u':  case 'x':
		case 'X':  case 's':  case 'e':  case 'E':  case 'f':
		case 'g':  case 'G':
		    break;
		default:
		    goto bad;
	    }

	if (ty)
	    fmtarg(q, arg++, ty);
	else goto bad;
    }

    LNBUG(ln_dbflag>1,("args: %d %d", arg, r.l.nargs));
    /* "too many arguments for format" */
    if (arg < r.l.nargs)
	BWERROR(14, q->name, cfno, r.l.fline);
    return;

bad:
    /* "malformed format string" */
    BWERROR(15, q->name, cfno, r.l.fline);
    return;
}



/*
** Check for legal use of scanf like functions.  This code also based
** on a 5ESS version of lint.
*/
static void
chkscanf(q, arg, fmt)
STAB *q;
int arg;
char *fmt;
{
    int c, suppress;
    TY ty;   
    LNBUG(ln_dbflag > 1, ("chkscanf: %s", fmt));

    while ((c = *fmt++) != '\0') {
	if (c != '%' || (c = *fmt++) == '%')
	    continue;

	/*
	** An optional assignment-suppressing character *.
	*/
	if (c == '*') {
	    suppress = 1;
	    c = *fmt++;
	} else suppress = 0;

	/*
	** An optional decimal integer that specifies the maximum field width.
	*/
	for (; c >= '0' && c <= '9'; c = *fmt++);

	ty = sftab[c];

	if (c == '[') {
	    if ((c = *fmt++) == '^')
		c = *fmt++;
	    if ( c == ']')
		c = *fmt++;
	    for (; c && c != ']'; c = *fmt++);
	    if (c != ']')
		goto bad;
	}

	/*
	** An optional h, l or L indicating the size of the receiving object.
	*/
	if (c == 'h' || c == 'l' || c == 'L')
	    if (pftab[c = *fmt++])
		switch (fmt[-2]) {
		    case 'h' : ty = sfhtab[c]; break;
		    case 'l' : ty = sfltab[c]; break;
		    case 'L' : ty = sfLtab[c]; break;
		    default: lerror(0,"bad char in chkscanf"); return;
		}

	if (!ty)
	    goto bad;
	else if (!suppress)
	    fmtarg(q, arg++, ty);
    }

    /* "too many arguments for format" */
    if (arg < r.l.nargs)
	BWERROR(14, q->name, cfno, r.l.fline);
    return;

bad:
    /* "malformed format string" */
    BWERROR(15, q->name, cfno, r.l.fline);
    return;
}



/*
** We now know we have a legal format argument.
** Format is arg, corresponding argument is ty.
*/
static void
fmtarg(q, arg, ty)
STAB *q;
int arg;
TY ty;
{
    ATYPE a;
    TY ty2;
    LNBUG(ln_dbflag > 1, ("fmtarg: %d %d", arg, ty));

    /*
    ** There were too many "%?" in the format string.
    */
    if (arg >= r.l.nargs) {
	if (arg == r.l.nargs)
	    /* "too few arguments for format" */
	    BWERROR(16, q->name, cfno, r.l.fline);
	return;
    }

    /* 
    ** Get type without qualifiers - make it a pointer if the arg is.
    */ 
    ty2 = LN_TYPE(getarg(arg).aty);
    if (getarg(arg).dcl_mod == LN_PTR) 
	ty2 = POINTER(ty2);
    else ty2 = promote(ty2);

    /*
    ** The format is "generic" - make the argument generic as well.
    */
    if (generic(ty))
	ty2 = mkgeneric(ty2);

    /*
    ** Types should be equivalent now.
    */
    if (ty == ty2)
	return;

    /* CONSTANTCONDITION */
    if (SZINT == SZLONG)
	if (   ((ty == POINTER(LN_INT)) && (ty2 == POINTER(LN_LONG)))
	    || ((ty == POINTER(LN_LONG)) && (ty2 == POINTER(LN_INT)))
	   )
		return;

    a.dcl_con = 0;
    a.dcl_vol = 0;
    a.extra.ty = 0;
    ty = mkungeneric(ty);
    if (ty < 0) {
	a.aty = -(ty+1);
	a.dcl_mod = LN_PTR;
    } else {
	a.aty = ty;
	a.dcl_mod = 0;
    }

    /*
    ** str1 is the type passed to the function,
    ** str2 is the type in the format string.
    */
    ptype(str1, getarg(arg), cmno);
    ptype(str2, a, cmno);

    /* "function argument ( number ) type inconsistent with format" */
    BWERROR(17, q->name, arg+1, str1, str2, cfno, r.l.fline);
    return;
}


static int
generic(ty)
TY ty;
{
    switch (ty) {
	case LN_GCHAR:		case POINTER(LN_GCHAR):
	case LN_GSHORT:		case POINTER(LN_GSHORT):
	case LN_GINT:		case POINTER(LN_GINT):
	case LN_GLONG:		case POINTER(LN_GLONG):
	case LN_GLLONG:		case POINTER(LN_GLLONG):
	    return 1;
	default:
	    return 0;
    }
}


static TY
mkgeneric(ty)
TY ty;
{
    TY sty;
    LNBUG(ln_dbflag > 1, ("mkgeneric: %d", ty));

    if (ty < 0)
	sty = -(ty + 1);
    else sty = ty;

    switch (sty) {
	case LN_CHAR: case LN_UCHAR: case LN_SCHAR:
	    sty =  LN_GCHAR;
	    break;
	case LN_SHORT: case LN_USHORT: case LN_SSHORT:
	    sty =  LN_GSHORT;
	    break;
	case LN_INT: case LN_UINT: case LN_SINT:
	    sty =  LN_GINT;
	    break;
	case LN_LONG: case LN_ULONG: case LN_SLONG:
	    sty =  LN_GLONG;
	    break;
	case LN_LLONG: case LN_ULLONG: case LN_SLLONG:
	    sty =  LN_GLONG;
	    break;
    }

    LNBUG(ln_dbflag > 1, ("  returning: %d", (ty<0) ? POINTER(sty) : sty));
    if (ty < 0)
	return POINTER(sty);
    else return sty;
}


static TY
mkungeneric(ty)
TY ty;
{
    TY sty;
    LNBUG(ln_dbflag > 1, ("mkgeneric: %d", ty));

    if (ty < 0)
	sty = -(ty + 1);
    else sty = ty;

    switch (sty) {
	case LN_GCHAR:
	    sty =  LN_CHAR;
	    break;
	case LN_GSHORT: 
	    sty =  LN_SHORT;
	    break;
	case LN_GINT:
	    sty =  LN_INT;
	    break;
	case LN_GLONG: 
	    sty =  LN_LONG;
	    break;
	case LN_GLLONG: 
	    sty =  LN_LONG;
	    break;
    }

    LNBUG(ln_dbflag > 1, ("  returning: %d", (ty<0) ? POINTER(sty) : sty));
    if (ty < 0)
	return POINTER(sty);
    else return sty;
}
