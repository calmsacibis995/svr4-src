/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/lpass2.c	1.18"
#include "p1.h"
#include <stdio.h>
#include <varargs.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include "manifest.h"
#include "lnstuff.h"
#include "lpass2.h"
#include "tables2.h"
#include "xl_byte.h"

static void mloop();
static int lread();
static void chkcompat();
static void setuse();
static int eqtype();
static char *typepr();
static void pass1();
static void pass2();

union	rec r;		/* where all the input ends up 		*/
static	int start;	/* at the start of a module?		*/
static	int pass;		/* what pass in current module		*/
static	long endmod;		/* offset to end of module		*/
FLENS	lout;		/* header for each module		*/
FILEPOS curpos;		/* current offset in intermediate file	*/
char	*curname;

char 	str1[MAXPRNT];	/* used to hold the strings of types	*/
char	str2[MAXPRNT];

int	ln_flags[52];		/* command line arguments	*/
int	ln_Xcflag;
char	*cfno;			/* pointer to current file name */
MODULE	cmno;		/* current module number	*/
int	clno;			/* current line number of object*/

#ifndef NODBG
static	int idebug;		/* debugging off		*/
int	ln_dbflag;
#endif



/*
** main program for second pass
*/
main(argc, argv) 
int argc;
char *argv[]; 
{
    int c;
    char *ifilename = NULL;
    extern char *optarg;
    extern int optind;

    /*
    ** read options
    */
    while((c=getopt(argc,argv,"abhkmpsuvxyFX:1:2:T:V")) != EOF)
	switch(c) {
	    case 'h': case 'p': case 's': case 'x':
	    case 'u': case 'a': case 'b': case 'c':
	    case 'n': case 'v': case 'L': case 'F':
	    case 'm':
		LN_FLAG(c) = 1;
		break;

#ifndef NODBG
	    /*
	    ** Debugging options
	    */
	    case '2':
		while (*optarg) {
		    switch(*optarg) {
			case 'i': 
			    idebug = 1;
			    break;
			case 'l': 
			    ln_dbflag++; 
			    break;
			default: fprintf(stderr,"unknown pass2 option\n");
		    }
		    ++optarg;
		}
		break;
#endif

	    case '1':		/* first pass debug options - ignore */
		break;

	    case 'X':
		switch (*optarg) {
		    case 't':
			/* ln_Xtflag = 1; */
			break;
		    case 'a':
			/* ln_Xaflag = 1; */
			break;
		    case 'c':
			ln_Xcflag = 1;
			break;
		    default:
			fprintf(stderr,"unknown language version '%c'", 
							*optarg);
			break;
		}
		break;

	}

    /* 
    ** Initialize byte-translation package.
    */
    xl_init();

    /*
    ** Input filename passed by shell script.
    */
    ifilename = argv[optind];

    tmpopen( );

    /*
    ** Allocate space (ARGS) for reading in arguments to functions,
    ** parameters of functions, and members of s/u/e's.
    ** "strp" is used to hold names of members of s/u/e's.
    */
    atyp = (ATYPE (*)[]) malloc(ARGS * sizeof(ATYPE));
    strp = (char *(*)[]) malloc(ARGS * sizeof(char *));
    if ((atyp == NULL) || (strp == NULL))
	lerror(FATAL,"can't allocate space for argument lists");
    args_alloc = ARGS;

    /*
    ** Array of type "chunks" - when it runs out, more space will be
    ** allocated.
    */
    tary = tarychunk0;

    /*
    ** Initialize arrays for printf/scanf checking.
    */
    fmtinit();

    if ( !freopen(ifilename, "r", stdin))
	lerror(FATAL|CCLOSE,"cannot open intermediate file");

#ifndef NODBG
    if (idebug) 
	pif();
#endif

    /*
    ** Pass over the input 3 times.  We actually don't read everything
    ** times - the input file is sectioned up (see lread()).
    **
    **    Pass1 will read in all definitive definitions; functions, variables,
    **    and structure definitions.
    **
    **    Pass2 will read in all tentative defintions.
    **
    **    Pass3 will read in all function uses, as well as if functions
    **    have return values.
    */


    /*
    ** Read in those entries that have "definitive" definitions.
    ** LDI:   int i=1;
    **        foo() { }
    ** LDS:   static int is;
    **        static int foos() { }
    ** LIB:   entries in a lint library
    ** LSU:   definition of struct/union
    */
    start = 1;
    pass = 1;
    mloop(DEFN|LSU);
    rewind(stdin);

    /*
    ** LDC:	int i;
    ** LDX:	extern int i;
    ** LPR:   prototype declaration
    */
    start = 1;
    pass = 2;
    mloop(DECL|LPR);
    rewind(stdin);

    /*
    ** LRV:	function has return statement
    ** LUV:	function used in value context:  i=foo();
    ** LUE:	function used in effects context: foo();
    ** LUM:	Entity mentioned (used) somewhere else than a decl.
    */
    start = 1;
    pass = 3;
    mloop(USE|LRV|LUM);

    cleanup();
    hdrclose();
    exit(0);
    /* NOTREACHED */
}



/*
** mloop - main loop
** Each pass of the main loop reads in names from the intermediate 
** file that have characteristics which overlap with the
** characteristics specified as the parameter.
*/
static void
mloop(m)
int m;
{
    STAB *q;
    LNBUG(ln_dbflag > 2, ("mloop"));

    while(lread(m)) {
	q = find();		/* symbol table entry of item read in	*/
	if (q->decflag) { 	/* the item has been previously entered */
	    setuse(q);
	    chkcompat(q);	/* in table - check compatibilities	*/
	} else setuse(q);	/* o/w enter info into symbol table	*/
    }
}



/* 
** lread - read a line from intermediate file into r.l 
** Return 0 at EOF, or 1 when an item of type "m" has been found.
**
** Format of intermediate file:
** The intermediate file consists of a section for each file that was
** linted.   Each of these sections is broken up into 4 parts, plus
** a header record.
** The header record contains offsets from the current position to
** each of the other sections:
**   ___________
**  |__________|  - header record with numbers X1,X2,X3,X4
**  |  PASS 1  |  - length X1
**  |__________|
**  |          |
**  |  PASS 2  |  - length X2
**  |__________|
**  |          |
**  |  PASS 3  |  - length X3
**  |          |
**  |__________|
**  |          |
**  |  STRINGS |  - length X4
**  |__________|
**  |__________|  - header record with numbers Y1,Y2,Y3,Y4
**  |  PASS 1  |  - length Y1
**    etc....
**
** Pass1 contains "definitive" definitions.
** Pass2 contains "tentative" definitions.
** Pass3 contains info about function uses.
** STRINGS contains strings passed to functions for [sf]printf checks.
**
** For lint to read items in pass1, it only has to go over the file
** once (old lint read through each and every record 3 times.)
** It reads until it reaches X1, then skips to X1+X2+X3+X4, reads
** the new header record, etc...
**
** The header record also includes the module # and the lint version.
*/
static int
lread(m)
int m;
{
    register n;
    LNBUG(ln_dbflag > 2, ("lread"));

    for (;;) {
	/*
	** Start is non-zero if we are at the start of a new module.
	** If the FLENS entry can not be read, we are (should be) at
	** the end of the file.
	*/
	if (start) {
	    curpos = ftell(stdin);
	    if (fread((char *)&lout, sizeof(FLENS), 1, stdin) != 1)
		return(0);

	    lout = *(xl_t_flens(&lout));
	    if (lout.ver != LINTVER)
		lerror(FATAL,"using old .ln file - relint");

	    /* 
	    ** In pass 1 we just keep on reading sequentially
	    ** Pass 2 will skip over the definitive definitions.
	    ** Pass 3 will skip over all definitions to the uses
	    ** section.
	    */
	    switch (pass) { 
		case 1: 
		    break; 
		case 2: 
		    (void)fseek(stdin, curpos + lout.f1, 0);
		    break;
		case 3:
		    (void)fseek(stdin, curpos + lout.f1 + lout.f2, 0);
		    break;
	    }
	    /*
	    ** No longer at the start of a module - compute the address
	    ** of the end of the module (start of the next one or EOF.)
	    */
	    start = 0;
	    endmod = curpos + lout.f1 + lout.f2 + lout.f3 + lout.f4;
	    cmno = lout.mno;
	}

	/* 
	** Read in line from intermediate file.
	*/
	if (fread((char *)&r, sizeof(r), 1, stdin) != 1) 
	    formerr();		/* exits */

	r = *(xl_t_rec(&r, XL_LINE));
	/*
	** End marker for current pass; set start to 1 to indicate that
	** a new module has starated, and seek the end of the current
	** one (start of the next.)
	*/
	if (r.l.decflag & LND) {
	    start = 1;
	    (void)fseek(stdin, endmod, 0);
	    continue;
	}

	/*
	** New filename and module number - set "cmno".
	** The purpose of the module number (translation unit) is to
	** is to handle static scoping correctly.  A module is a file 
	** with all its include files.  From a scoping point of view,
	** there is no difference between a variable in a file and a
	** variable in an included file.  The module number itself is 
	** not meaningful; it must only be unique to ensure that modules
	** are distinguishable.
	** (the module number is actually the process # given when
	**  lint1 ran)
	*/
	if (r.l.decflag & LFN) {
	    r.f.fn = getstr();
	    cfno = l_hash(r.f.fn);
	    continue;
	}

	/*
	** Number of arguments is negative for VARARGS or "..." + 1.
	*/
	clno = r.l.fline;
	curname = getstr();
	clno = 0;
	n = r.l.nargs;
	if (n<0) 
	    n = -n - 1;

	/*
	** See if we have enough space allocated for all the arguments
	** (or s/u/e members).  If not, expand the table (this should
	** seldom happen more than once.)
	*/
	if (n > args_alloc)
	    arg_expand(n);

	/*
	** This is a struct/union definition.  In addition to reading
	** in info on each member, we must read in the member name.
	*/
	if (r.l.decflag & LSU) {
	    int i;
	    for (i=0;i<n;i++) {
		ATYPE *arg = &getarg(i);
		fread((char *) arg, sizeof(ATYPE), 1, stdin);
		*arg = *(xl_t_atype(arg, XL_TY));
		argname(i) = getstr();
	    }
	} 
	/*
	** Otherwise it is either a function definition or a function
	** call.  These can be read in all at once because there is
	** no tag_name to separate them.
	*/
	else if (n) {
	    int i;
	    ATYPE *arg = &getarg(0);
	    fread ((char *) arg, sizeof(ATYPE), n, stdin);
	    for (i=n; i>0; --i, ++arg) 
		    *arg = *(xl_t_atype(arg, XL_TY));
	}

	/* 
	** Return with entry only if it has correct characteristics.
	** Otherwise there is an error.
	*/
	if (r.l.decflag & m)
	    return (1);
	else formerr();		/* exits */
    }
}



/* 
** chkcompat - check for compatibility
**	Compare item in symbol table (stab) with item just read in.
**	The work of this routine is not well defined; there are many
**	checks that might be added or changes.
*/
static void
chkcompat(q) 
STAB *q; 
{
    register int i;
    int usage, lerrno, line1, line2;
    char *name, *file1, *file2;
    STYPE *qq;
    LNBUG(ln_dbflag > 1, ("chkcompat"));

    name = q->name;		/* name of item	*/
    file1 = q->fno;		/* file defined in */
    line1 = q->fline;		/* line defined at */
    file2 = cfno;		/* file of secondary use/decl/def */
    line2 = r.l.fline;		/* line of secondary use/decl/def */

    /*
    ** usage is non-zero if either entry is a use of a function.
    */
    usage = ((q->decflag&USE) || (r.l.decflag&USE));

    /*
    ** Do an argument check only if item is a function; if it is
    ** both a function and a variable, it will get caught later on.
    ** There are identical checks for functions involving a prototype -
    ** issue different message in that case.
    */
    if (LN_ISFTN(r.l.type.dcl_mod) && LN_ISFTN(q->symty.t.dcl_mod) && 
	(q->decflag & (DEFN|USE|LPR)) && (r.l.decflag & (DEFN|USE|LPR))) {
	
	/*
	** printf/scanf type checking ...
	*/
	if ((q->decflag & (LPF|LSF)) && usage) {
	    fmtcheck(q);
	    if (r.l.nargs > q->nargs)
		r.l.nargs = q->nargs;
	    q->use |= VARARGS2;
	}

	if (q->nargs != r.l.nargs) {
	    /* 
	    ** "function called with variable number of arguments" - 7
	    ** "function declared with variable number of arguments" - 12
	    */
	    if ((q->use&VARARGS2) && (r.l.nargs < 0))
		r.l.nargs = -r.l.nargs - 1;
		
	    if ((! (q->use&VARARGS2)) || (r.l.nargs < q->nargs)) {
		if (usage)
		    lerrno = 7;
		else lerrno = 12;
		BWERROR(lerrno, name, file1, line1, file2, line2);
	    }

	    if (r.l.nargs > q->nargs)
		r.l.nargs = q->nargs;
	    if (! (q->decflag & (DEFN|LPR))) {
		q->nargs = r.l.nargs;
		q->use |= VARARGS2;
	    }
	}

	/*
	** "function argument ( number) used inconsistently" - 6
	** "function argument ( number) declared inconsistently" - 11
	*/
	if (usage)
	    lerrno = 6;
	else lerrno = 11;

	for (i=0, qq=q->symty.next; i<r.l.nargs; ++i,qq=qq->next) {
	    if (! eqtype (getarg(i), cmno, qq->t, q->mno, usage)) {
		ptype(str1, qq->t, q->mno);
		ptype(str2, getarg(i), cmno);
		BWERROR(lerrno, name, i+1, file1, line1, str1,
			file2, line2, str2);
	    }
	}
    }

    /* 
    ** "value used inconsistently" 
    ** The return value of a function definition or prototype must
    ** match the return value used.
    ** If there was no definition or prototype for this function,
    ** use the first LUV (use) as the basis for comparison.
    */
    if ((q->decflag & (DEFN|LPR|LUV)) && (r.l.decflag==LUV) &&
	(! eqtype (r.l.type, cmno, q->symty.t, q->mno, 1))) {
	    ptype(str1, q->symty.t, q->mno);
	    ptype(str2, r.l.type, cmno);
	    BWERROR(4, name, file1, line1, str1, file2, line2, str2);
    }

    /*
    ** "multiply defined"
    ** If both are definitions, then it has been multiply defined 
    ** regardless if they are the same type.
    */
    if ((q->decflag & (LIB|LPR)) || (r.l.decflag & (LIB|LPR)))
	/* EMPTY */ ;
    else if ((q->decflag & DEFN) && (r.l.decflag & DEFN))
	BWERROR(3, name, file1, line1, file2, line2);

    /*
    ** "value type declared inconsistently"
    ** If the return value declared in a definition, prototype, declaration
    ** does not match another prototype, declaration issue diagnostic.
    */
    if ((q->decflag & (DEFN|LPR|DECL)) && 
	(r.l.decflag & (LPR|DECL)) && 
	(! eqtype(r.l.type, cmno, q->symty.t, q->mno, 0))) {
	    ptype(str1, q->symty.t, q->mno);
	    ptype(str2, r.l.type, cmno);
	    BWERROR(5, name, file1, line1, str1, file2, line2, str2);
    }
}



/*
** After all entries have been read, process each entry in the symbol
** table.
*/
void
lastone(q) 
STAB *q; 
{
    int uses=q->use;
    LNBUG(ln_dbflag > 1, ("lastone"));

    switch (q->decflag) {
	/*
	** There was no definition or declaration for this name.
	** Issue "name used but not defined"
	** (suppress with -u)
	*/
	case LUV:
	case LUE:
	case LUM:
	    if (! LN_FLAG('u'))
		BWERROR(0, q->name, q->usefno, q->useline);
	    break;

	/*
	** There was a declaration for this name, but it was
	** never defined.
	** Issue "name used but not defined" if it was used.
	** Issue "name declared, never used or defined" if it was not
	** used. (suppress both with -u)
	*/
	case LDX:
	case LPR:
	    if (uses&USED) {
		if (! LN_FLAG('u'))
			BWERROR(0, q->name, q->usefno, q->useline);
	    } else if (! LN_FLAG('x'))
		BWERROR(2, q->name, q->fno, q->fline);
	    break;

	/*
	** If we have a definition, it is never used, and it is not main;
	** issue "name defined but never used"
	**
	** If the definition is never mentioned in another file, issue
	**    "declared global, could be static"
	*/
	case LDI:
	case LDC:
	case LDI|LPF:
	case LDC|LPF:
	case LDI|LSF:
	case LDC|LSF:
	    if (strcmp(q->name, "main") == 0)
		break;
	    if (! (uses&USED)) {
		if (!LN_FLAG('u'))
		    BWERROR(1, q->name, q->fno, q->fline);
	    } else if (! (uses&OKGLOB)) {
		if (!LN_FLAG('m'))
		    BWERROR(18, q->name, q->fno, q->fline);
	    }
    }

    /*
    ** The function returns a value, and was used for effects in at
    ** least one case.
    ** If it was used for its value as well, issue "sometimes ignored"
    ** o/w issue "always ignored"
    */
    if ((uses&(RVAL|EUSED)) == (RVAL|EUSED)) {

	/* "%s returns value which is sometimes ignored\n" */
	if (uses & VUSED) 
	    BWERROR(10, q->name);

	/* "%s returns value which is always ignored\n" */
	else BWERROR(9, q->name);
    }

    /*
    ** Function used for its value, but doesn't return one.
    */
    /* "%s value is used, but none returned\n" */
    if ((uses&(RVAL|VUSED)) == (VUSED) && (q->decflag&(LDI|LDS|LIB)))
	BWERROR(8, q->name);
}




/* 
** setuse - check new type to ensure that it is used
*/
static void
setuse(q) 
STAB *q;
{
    LNBUG(ln_dbflag > 2, ("setuse"));

    /* q->decflag is 0 if this is a new entry to put in the
    ** symbol table.
    */
    if (!q->decflag) {
	q->decflag = r.l.decflag;

	/*
	** struct/union definition.  Build up the
	** table of s/u/e defintions.
	*/
	if (q->decflag & LSU)
	    build_lsu(r.l.type.extra.ty, cmno, q);

	/* copy into symbol table:
	**	type
	**	# of args
	**	line number
	**	file number (char *)
	**	module number
	**	each argument
	*/
	q->symty.t = r.l.type;
	if (r.l.nargs < 0) {
	    q->nargs = -r.l.nargs - 1;
	    q->use = VARARGS2;
	} else {
	    q->nargs = r.l.nargs;
	    q->use = 0;
	}

	q->fline = r.l.fline;
	q->fno = cfno;
	q->mno = cmno;

	if (q->nargs) {
	    int i;
	    STYPE *qq;
	    for (i=0,qq= &q->symty; i<q->nargs; ++i,qq=qq->next) {
		qq->next = tget();
		qq->next->t = getarg(i);
		if (q->decflag & LSU)
		    qq->next->tag_name = argname(i);
	    }
	}
    }

    /*
    ** If there was:
    **	  - LRV : function has a return value
    **    - LUV : function used for its value
    **    - LUE : function used for effects
    **    - LUM : used somewhere
    */
    if (r.l.decflag & LRV)
	q->use |= RVAL;
    else if (r.l.decflag & LUV)
	q->use |= VUSED+USED;
    else if (r.l.decflag & LUE)
	q->use |= EUSED+USED;
    else if (r.l.decflag & LUM)
	q->use |= USED;



    /*
    ** A prototype "overrides" a normal declaration.
    ** Replace the info in the table with the new "stuff"
    */
    if ((q->decflag & DECL) && (r.l.decflag & LPR)) {
	q->decflag = 0;
	setuse(q);
	return ;
    }

    if ((q->decflag & LDX) && (r.l.decflag & LDC)) {
	q->decflag = 0;
	setuse(q);
/*	q->decflag = LDC; */
/* 	q->mno = cmno; */
    }

    /*
    ** There was a use of this function/variable - if the module it was
    ** used in is not the same as it was defined in, then it is ok
    ** to make this global.
    */
    if (r.l.decflag & (LUV|LUE|LUM)) {
	if (cmno != q->mno)
	    q->use |= OKGLOB;
	if (q->useline == 0) {
	    q->useline = r.l.fline;
	    q->usefno = cfno;
	}
	return;
    }
}



#define ISINT(t)  (								\
		   (t) == LN_INT || (t) == LN_SINT || (t) == LN_UINT ||		\
                   (t) == LN_LONG || (t) == LN_SLONG || (t) == LN_ULONG		\
		  )
/* 
** Check the two type words to see if they are compatible -
** Return 0 if they are not, non-zero otherwise.
*/
static int
eqtype(pt1, mno1, pt2, mno2, nostrict) 
ATYPE pt1, pt2; 
MODULE mno1, mno2;
int nostrict;
{
    /*
    ** b1 and b2 will be the "base" types of the types passed in.
    ** This is the type stripped of qualifiers.
    ** con1 is non-zero if pt1 is a constant, con2 likewise.
    */
    TY b1=LN_TYPE(pt1.aty), b2=LN_TYPE(pt2.aty);
    int con1=(pt1.aty&LCONV) && ISINT(b1);
    int	con2=(pt2.aty&LCONV) && ISINT(b2);

    LNBUG(ln_dbflag > 1, ("eqtype: %ld %ld %d %d %ld %ld",pt1.aty,pt2.aty,
				b1,b2,pt1.dcl_mod,pt2.dcl_mod));

    /*
    ** Allow special case of pointer and NULL (actually pointer and
    ** any integer constant.)
    */
    if ((LN_ISPTR(pt1.dcl_mod) && con2) || 
	(LN_ISPTR(pt2.dcl_mod) && con1))
	return 1;

    /*
    ** Allow special case of pointer to void with pointer to anything (when
    ** nonstrict). Both types must be pointer to something (&LPTR), one of them
    ** has to be void, and the number of type modifiers on the other type
    ** must be at least that of the void type, but may add additional:
    **    char *c;  char **cc;
    **    void *v;  void **vv;
    ** 
    ** Compatible types:          Noncompatible types:
    **    c == v;                     c != vv;
    **    cc == v;
    **    cc == vv;
    */
    if (   nostrict
	&& (pt1.aty&LPTR) 
 	&& (pt2.aty&LPTR)
	&& (   ((b2 == LN_VOID) && (pt1.dcl_mod >= pt2.dcl_mod))
	    || ((b1 == LN_VOID) && (pt2.dcl_mod >= pt1.dcl_mod))
	   )
       )
	return 1;

    /* 
    ** The type modifiers must match; o/w we can return 0 right away.
    ** (type qualifiers only have to match if !nostrict)
    */
    if (pt1.dcl_mod != pt2.dcl_mod)
	return 0;

    /* 
    ** If both are structs, or both are unions, check for equality.
    */
    if (((b1==LN_STRUCT) && (b2==LN_STRUCT)) ||
	((b1==LN_UNION) || (b2==LN_UNION)))
	return (eq_struct(pt1.extra.ty, mno1, pt2.extra.ty, mno2));   

    /*
    ** This might be a little better defined ....
    ** A constant is passed in if extra is non-zero (and dcl_mod has
    ** no value, o/w extra is a file offset for a char *).
    */

    if (con1 && ISINT(b2)) {
	if ((SZINT == SZLONG) && !LN_FLAG('p'))
	    return 1;
	else if ((b2 == LN_UINT) && (b1 == LN_INT))
	    return 1;
	else if ((b2 == LN_ULONG) && (b1 == LN_LONG))
	    return 1;
    } else if (con2 && ISINT(b1)) {
	if (ISINT(b1) && (SZINT == SZLONG) && !LN_FLAG('p'))
	    return 1;
	else if ((b1 == LN_UINT) && (b2 == LN_INT))
	    return 1;
	else if ((b1 == LN_ULONG) && (b2 == LN_LONG))
	    return 1;
    }

    /*
    ** Simple promotions: SINT -> INT, and SLONG -> LONG
    */
    b1 = promote(b1);
    b2 = promote(b2);

    /*
    ** If nostrict, then we are comparing the use of an object.
    ** Otherwise we are comparing the definition of an object.
    ** For definition equivalence all the qualifiers must match as well.
    ** b2 is the entry in the symbol table, b1 is the entry just
    ** read in.
    **
    ** Allow pointer to void with anything (at this point, we
    ** know the modifiers are the same.)
    **    void *a() == char *b()
    **    void *(*a)() == struct foo *(*b)()
    **    void *a == int *b;
    **    void (**a)() != int (**b)()
    */
    if (nostrict) {
	if ((pt1.dcl_mod && pt1.aty&LPTR && (b2 == LN_VOID)) ||
	    (pt2.dcl_mod && pt2.aty&LPTR && (b1 == LN_VOID)))
		return 1;
	/*
	** For types to be equal:
	**	1. the base type must match (int, long, etc...)
	**	2. the type qualifiers on the function call
	**	   (pt1) must be less than those on the definition (pt2)
	**	   if pt1 and pt2 are pointers.
	**	   The first level of qualifiers are in the type (aty).
	*/
	return (   (b1 == b2)
		&& (	(!LN_ISPTR(pt1.dcl_mod))  /* neither is a pointer */
		   	|| (((pt1.aty&LCON) <= (pt2.aty&LCON))
			   && ((pt1.aty&LVOL) <= (pt2.aty&LVOL)))
		   )
	       );
    } else

	/*
	** Types must match exactly - both basetypes, and
	** qualifiers must be the same.
	*/
	return ((b1 == b2) &&
		((pt1.aty&LNUNQUAL) == (pt2.aty&LNUNQUAL)) &&
		(pt1.dcl_con == pt2.dcl_con) &&
		(pt1.dcl_vol == pt2.dcl_vol));
}



/*
** Promote the type -
**
** Allow combinations of char, short, int, signed char, signed short
** and signed int.
** Allow combinations of unsigned char, unsigned short and unsigned int.
*/
TY
promote(ty)
TY ty;
{
    LNBUG(ln_dbflag > 2, ("promote: %d",ty));

    switch (ty) {
	case LN_SINT:
	    return LN_INT;
	case LN_SLONG:
	    return LN_LONG;
	default:
	    return ty;
    }
}



/* Format error found in intermediate file.
*/
void
formerr()
{
    LNBUG(ln_dbflag, ("formerr"));
    lerror(FATAL,"Format error in intermediate file (old lint format?)");
}



/*
** lint will print out the types when a type mismatch occurs.
** typepr will print out the base type, with the qualifiers associated
** with it.
** If the type is a struct/union, the tag name will be printed as well
** (if there is a tag name.)
*/
static char *
typepr(t, mod, extra)
TY t;
MODULE mod;
T1WORD extra;
{
    static char p[50];
    char *s;
    LNBUG(ln_dbflag > 1, ("typepr: %d %d %ld",t,mod,extra));

    p[0] = '\0';
    if (t&LCON)
	strcat(p,"const ");
    if (t&LVOL)
	strcat(p,"volatile ");
    if (t&LCONV)
	strcat(p,"constant ");
    switch (t&LNQUAL) {
	case LN_CHAR:   	s="char "; break;
        case LN_UCHAR:  	s="unsigned char "; break;
        case LN_SCHAR:  	s="signed char "; break;
        case LN_SHORT:  	s="short "; break;
        case LN_USHORT: 	s="unsigned short "; break;
        case LN_SSHORT: 	s="signed short "; break;
        case LN_INT:    	s="int "; break;
        case LN_UINT:   	s="unsigned int "; break;
        case LN_SINT:   	s="signed int "; break;
        case LN_LONG:   	s="long "; break;
        case LN_ULONG:  	s="unsigned long "; break;
        case LN_SLONG:  	s="signed long "; break;
        case LN_LLONG:  	s="long long "; break;
        case LN_ULLONG: 	s="unsigned long long "; break;
        case LN_SLLONG: 	s="signed long long "; break;
        case LN_ENUM:   	s="enum "; break;
        case LN_FLOAT:  	s="float "; break;
        case LN_DOUBLE: 	s="double "; break;
        case LN_LDOUBLE:        s="long double "; break;
        case LN_VOID:   	s="void "; break;
        case LN_STRUCT:
        case LN_UNION:
	    {   ENT *e = tyfind(extra, mod);
		if ((t&LNQUAL) == LN_STRUCT)
		    s="struct ";
		else s = "union ";
		strcat(p, s);
		if ((e != NULL) && (e->st->name[0] != '.')) {
		    strcat(p, e->st->name);
		    strcat(p, " ");
		} else strcat(p, "<no-tag> ");
		return p;
	    }
        default:        	s="?? "; break;
    }
    strcat(p, s);
    return p;
}



/*
** ptype will print the type of f in str
*/
static char typestr[MAXPRNT];
void
ptype(str, f, mod)
char *str;
ATYPE f;
MODULE mod;
{
    LNBUG(ln_dbflag > 2, ("ptype: %d %d %ld", f.aty, mod, f.extra.ty));
    typestr[0] = '\0';
    strcat(typestr,typepr(f.aty, mod, f.extra.ty));
    pass1(f.dcl_mod);
    pass2(f.dcl_mod);
    strcpy(str, typestr);
}



/*
** The algorithm for printing the types in ascii form comes from
** DMK's "sayit" program.
*/
static void
pass1(tt)
unsigned long tt;
{
    int flag;
    if (tt == 0)
	return;

    if (LN_ISPTR(tt)) {
	tt = tt >> 2;
	flag = (LN_ISARY(tt) || LN_ISFTN(tt));
	pass1(tt);
	if (flag)
	    strcat(typestr,"(*");
	else strcat(typestr,"*");
    } else pass1(tt >> 2);
}



static void
pass2(tt)
unsigned long tt;
{
    if (tt == 0)
	return;
    if (LN_ISARY(tt)) {
	strcat(typestr,"[]");
	pass2(tt >> 2);
    } else if (LN_ISFTN(tt)) {
	strcat(typestr,"()");
	pass2(tt >> 2);
    } else if (LN_ISPTR(tt)) {
	tt = tt >> 2;
	if (LN_ISARY(tt) || LN_ISFTN(tt))
	    strcat(typestr,")");
	pass2(tt);
    } else pass2(tt >> 2);
}
