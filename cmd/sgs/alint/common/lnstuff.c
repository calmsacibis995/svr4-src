/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/lnstuff.c	1.10.1.11"
/*
** This file contains routines that write to the intermediate file for
** pass 2 of lint.
*/
#include "p1.h"
#include "lnstuff.h"
#include <string.h>
#include "xl_byte.h"

extern	FILE *file1, *file2, *file3, *file4;

static	void ln2_type();
static	void ln2_outarg();
static	void ln2_args();
static	void ln_sort();
static	void ln_sort();
static	void ln2_fwrite();
static	FILEPOS ln2_strout();
void	ln2_outdef();
static	TY ln2_chgtype();


static char *su_fno = NULL;
static int su_line = 0;
static int def_su = 0;
static void ln2_suref();
static void ln2_sudef();


/*
** Given type ty, break it down into its base type (t->aty),
** ptr/ftn/ary modifiers (t->dcl_mod), const/volatile
** qualifiers (t->dcl_con, t->dcl_vol, t->dcl_nal, and the high
** 3 bits of t->aty for qualifiers on the generic type.)
** t->extra contains the internal type number if the generic type
** is a complex type (i.e. struct/union.)
*/
#define LN_INCREF(x,y,z,n)	(x |= (y << (n*z)))
static void
ln2_type(t,ty)
ATYPE *t;
T1WORD ty;
{
    int inc = 0;
    int modwarn = 0;
    int lastptr = 0;
    LNBUG(ln_dbflag > 1, ("ln2_type: %d", ty));

    /*
    ** Initialize all fields to 0 of the outgoing type record.
    */
    t->extra.ty = 0;
    t->extra.pos = 0;
    t->dcl_mod = 0;
    t->dcl_con = 0;
    t->dcl_vol = 0;

    for (;;) {
	switch (TY_TYPE(ty)) {
	    case TY_STRUCT: case TY_UNION:
		t->extra.ty = TY_UNQUAL(ty);
		ln2_suref(t->extra.ty);
		/* FALLTHRU */
	    case TY_CHAR:	case TY_UCHAR:	case TY_SCHAR: 
	    case TY_SHORT:	case TY_USHORT: case TY_SSHORT:
	    case TY_INT: 	case TY_UINT: 	case TY_SINT:
	    case TY_AINT:	case TY_AUINT:
	    case TY_LONG: 	case TY_ULONG: 	case TY_SLONG:
	    case TY_LLONG: 	case TY_ULLONG: case TY_SLLONG:
	    case TY_FLOAT: 	case TY_DOUBLE: case TY_LDOUBLE:
	    case TY_VOID:
	    case TY_ENUM:
		t->aty = ln2_chgtype(TY_TYPE(ty));
		LNBUG(ln_dbflag > 1, ("returned: %d\n", t->aty));

		/*
		** If the type is qualified, OR in the possible qualifiers.
		** Qualifiers on the base type are kept in aty, while
		** qualifiers on ftn/ary/ptr are kept in dcl_{con,vol}
		*/
		if (TY_ISQUAL(ty)) {
		    if (TY_ISCONST(ty))
			t->aty |= LCON;
		    if (TY_ISVOLATILE(ty))
			t->aty |= LVOL;
		}

		/*
		** Need to see if the last modifier was a pointer.  If it
		** was, or in the pointer bit.  This is done so that lint2
		** can quickly see if the last modifier is a pointer.
		** If a "pointer to function returning pointer to void"
		** and "pointer to function returning pointer to char"
		** are intermixed, they are allowed by checking to see
		** if the modifiers are the same, and if the last modifier
		** is a pointer.
		*/
		if (lastptr)
		    t->aty |= LPTR;
		LNBUG(ln_dbflag > 1, ("returning with : %d\n",t->aty));
		return;

	    case TY_PTR:
		LN_INCREF(t->dcl_mod, LN_PTR, inc, 2);
		lastptr=1;
		break;
	    case TY_ARY:
		LN_INCREF(t->dcl_mod, LN_ARY, inc, 2);
		lastptr=0;
		break;
	    case TY_FUN:
		LN_INCREF(t->dcl_mod, LN_FTN, inc, 2);
		lastptr=0;
		break;

	    default:
		fprintf(stderr,"ln2_type(): bad base type");
		break;
	}

	/*
	** Qualifiers for ARY/PTR/FUN.
	*/
	if (TY_ISQUAL(ty)) {
	    if (TY_ISCONST(ty))
		LN_INCREF(t->dcl_con, 1, inc, 1);
	    if (TY_ISVOLATILE(ty))
		LN_INCREF(t->dcl_vol, 1, inc, 1);
	}

	inc++;
	/*
	** Due to lint's design, it can only handle 16 type modifiers
	** (function/array/pointer).  Warn the user that only 16 will
	** be used.
	** Qualifiers are kept in shorts, and the ANSI standard
	** guarantees a short at least 16 bits.
	*/
	if (inc == 17) {
	    if (! modwarn)
		WERROR("type has > 16 modifiers, only 16 will be used"); 
	    inc--;
	    modwarn = 1;
	}
	ty = TY_DECREF(ty);
    }
}



/*
** Output an argument (either for function call, function
** definition, function prototype, struct/union/enum member definitions.)
** Type is the type number of the argument.
** If flag is non-zero, set extra to the value.
*/
static void
ln2_outarg(type, pos, out)
T1WORD type;
FILEPOS pos;
FILE *out;
{
    ATYPE t;
    LNBUG(ln_dbflag > 1, ("ln2_outarg: %ld %ld", type, pos));

    /*
    **
    */

    ln2_type(&t, type);

    if (pos > 0)
	t.extra.pos = pos;
    else if (pos < 0)
	t.aty |= LCONV;

    /*
    ** Is the type an enum?  Change type from enum to int, keeping
    ** qualifiers & other bit info.
    */
    if ((t.aty&LNQUAL) == LN_ENUM)
	t.aty = (t.aty & LNUNQUAL) | LN_INT;

    fwrite((char *)xl_t_atype(&t, pos > 0 ? XL_POS : XL_TY),
		sizeof(ATYPE), 1, out);
    LNBUG(ln_dbflag, ("\ttype: %d\n\tdcl_mod: %ld\n\textra: %ld\n\n", 
		t.aty, t.dcl_mod, t.extra));
}



/*
** For a function CALL:
** Write out information on the types of arguments passed to a function
** from first to last.
** Tree expected:
**        CALL				 CALL
**       /   \				/   \
**            ,		- or -		    ARG
**           / \
**	    ,  ARG
**	   / \
**        etc ...
*/
static void
ln2_args(tr)
ND1 * tr;
{ 
    LNBUG( ln_dbflag > 1, ("ln2_args"));

    if (tr->op == CM) {
	ln2_args(tr->left);
	tr = tr->right;
    }
    if (tr->op == FUNARG) {
	if (iscon(tr->left))
	    ln2_outarg(tr->type, -1L, file3);
	else ln2_outarg(tr->type, ln2_strout(tr->left), file3);
    } else if (tr->op == STARG)
	ln2_outarg(TY_DECREF(tr->type), 0L, file3);
    else lerror(0, "ln2_args: unable to traverse tree correctly");
}
	



/*
** Write out a string entry for [sf]printf, [sf]scanf checking in
** pass 2 to file4, return the position in the file of the start of
** the string.
** If this tree is not a string, don't write out anything, and return
** a position of 0.
*/
static FILEPOS
ln2_strout(tr)
ND1 *tr;
{
    FILEPOS pos=0;
    LNBUG(ln_dbflag > 1, ("ln2_strout"));

    if (tr->op == STRING) {
	pos = ftell(file4);
	fputs(tr->string, file4);
	putc('\0', file4);
    }
    return pos;
}



/*
** For function calls (CALL or UNARY CALL)
**
**	- count the number of arguments by traversing the right branch
** 	- write out name, linenumber, and the number of args
**	- if it is a CALL, write out arguments
*/
void
ln2_funccall(ret, goal)
ND1 * ret;
int goal;
{
    int args=0;
    ND1 *tr;
    LNBUG( ln_dbflag > 1, ("ln2_funccall: %d", goal));

    /*
    ** Write a call entry out only if there is a symbol, it is a function,
    ** and this isn't a lint library.
    */
    if (!IS_SYMBOL(ret->left) || !TY_ISFTN(SY_TYPE(ret->left->rval)) ||
	LN_DIR(LINTLIBRARY))
	    return;

    /* 
    ** Count the number of arguments.
    */
    if (ret->op == CALL) {
	tr = ret->right;
	args=1;
	while (tr->op == CM) {
	    tr = tr->left;
	    args++;
	}
    }

    /* 	
    ** Write information for function call.
    */
    tr = ret->left;

    if (SY_CLASS(tr->rval) == SC_AUTO)
	return;
    else 
	ln2_outdef(tr->rval, (goal==EFF) ? LUE : LUV, args);

    if (ret->op == CALL)		/* write info on arguments       */
	ln2_args(ret->right);
}




/*
** The function with type ty has been declared as PRINTFLIKE or SCANFLIKE.
** Make sure that the flag'th argument is a character pointer.
**
** This will return the number of args that will be checked in pass2.
*/
static int
ln2_chkform(ty, flag, isproto, args)
T1WORD ty;
int flag, isproto, args;
{
    T1WORD argty;
    LNBUG(ln_dbflag > 1, ("ln2_chkform: %ld %d %d %d",ty,flag,isproto,args));

    /*
    ** If the function is really a prototype declaration (by using
    ** the directive PROTOLIB), get the argument via TY_PROPRM(), o/w
    ** get it with SY_TYPE().
    */
    if (isproto)
	argty = TY_PROPRM(ty, flag-1);
    else argty = SY_TYPE(dcl_g_arg(flag-1));

    /*
    ** Must be char * - if so, set the LN_DIR(VARARGS) to 0 (indicating it
    ** takes a variable # of args, normally it is -1), and return
    ** flag.
    ** Otherwise issue a lint error and return the # of args
    */
    if (TY_ISPTR(argty) && (TY_TYPE(TY_DECREF(argty)) == TY_CHAR)) {
	LN_DIR(VARARGS) = flag;
	return flag;
    } 
    WERROR("argument %d must be \"char *\" for PRINTFLIKE/SCANFLIKE",flag);
    return args;
}



/*
** Called for the definition of a function, or the declaration of a
** function prototype.
**
** func is the sid of the function, and args is the number of arguments for
** that function.
** isproto is non-zero if func is a prototype declaration.
*/
void
ln2_funcdef(func, args, isproto)
SX func;
int args, isproto;
{
    int argno, nargs;
    T1WORD ty = SY_TYPE(func);
    I7 class = SY_CLASS(func);
    static char arg_mis[] = "argument number mismatch with directive: /* %s */";
    LNBUG(ln_dbflag > 1, ("ln2_funcdef: %d %d %d", func, args, isproto));

    if ((LN_DIR(ARGSUSED) >= 0) && (args < LN_DIR(ARGSUSED)))
	WERROR(arg_mis, "ARGSUSED");

    /*
    ** If the function takes a variable number of args:
    **    - true if function declared with "..."
    **    - true if VARARGS was used because the compiler puts
    **       that information into the function's type.
    ** When a function is defined with a function prototype, the "..."
    ** counts as an arg.  Subtract one from args if this is the case
    ** (function prototype declarations don't count "..." as an arg)
    */
    if (TY_ISVARARG(SY_TYPE(func)))
	if (LN_DIR(VARARGS) == -1) {
	    if (! isproto)
		args--;
	    LN_DIR(VARARGS) = args;
	}

    /*
    ** If the VARARGS directive was used, LN_DIR(VARARGS) has the number 
    ** that are to be checked (0 if none was specified.)
    ** If the real # of arguments is less than that specified by VARARGS,
    ** issue a warning.
    */
    if (LN_DIR(VARARGS) >= 0) {
	if (args < LN_DIR(VARARGS))
	    WERROR(arg_mis,"VARARGS");
	else args = LN_DIR(VARARGS); 
    } 

    /*
    ** If the PRINTFLIKE directive was used, make sure there are at
    ** least LN_DIR(PRINTFLIKE) arguments.  If not, don't make this PRINTFLIKE.
    ** The LN_DIR(PRINTFLIKE) argument must be char *.
    */
    if (LN_DIR(PRINTFLIKE) != -1) {
	if (args < LN_DIR(PRINTFLIKE)) {
	    WERROR(arg_mis, "PRINTFLIKE");
	    LN_DIR(PRINTFLIKE) = -1;
	} else args = ln2_chkform(ty, LN_DIR(PRINTFLIKE), isproto, args);
    }

    /*
    ** Similar for SCANFLIKE
    */
    else if (LN_DIR(SCANFLIKE) != -1) {
	if (args < LN_DIR(SCANFLIKE)) {
	    WERROR(arg_mis, "SCANFLIKE");
	    LN_DIR(SCANFLIKE) = -1;
	} else args = ln2_chkform(ty, LN_DIR(SCANFLIKE), isproto, args);
    }
	
    /*
    ** If this definition takes a variable number of arguments, indicate
    ** so to lint2 by making the # of arguments negative + 1.
    ** This is applied to functions declared as VARARGS, PRINTFLIKE,
    ** SCANFLIKE, or "..."
    */
    if (LN_DIR(VARARGS) >= 0)
	nargs = -(args+1);
    else nargs = args;

    /* reset VARARGS flag to default */
    LN_DIR(VARARGS) = -1;

    /* Don't apply PRINTFLIKE or SCANFLIKE to asms. */
    if (class == SC_ASM) {
	static char asm_use[] =
	    "directive can not be applied to asms: /* %s */";
	if (LN_DIR(PRINTFLIKE) > 0) {
	    WERROR(asm_use, "PRINTFLIKE");
	    LN_DIR(PRINTFLIKE) = -1;
	} else if (LN_DIR(SCANFLIKE) > 0) {
	    WERROR(asm_use, "SCANFLIKE");
	    LN_DIR(SCANFLIKE) = -1;
	}
    }

    /*
    ** If LINTLIBRARY is used:
    **	- don't write info out about statics & asms
    **  - only write out info about prototypes if the PROTOLIB directive
    **	  was used, and pretend the prototype has a return in it if it
    **	  is non-void
    **	- write a LIB entry out for all other definitions
    */
    if (LN_DIR(LINTLIBRARY)) {
	if ((class == SC_STATIC) || (class == SC_ASM))
	    goto skip;
	if (isproto) {
	    if (! LN_DIR(PROTOLIB))
		goto skip;
	    if (TY_DECREF(SY_TYPE(func)) != TY_VOID)
		ln2_outdef(func, LRV, 0);
	    ln2_outdef(func, LIB|LPR, nargs);
	} else ln2_outdef(func, LIB, nargs);
    }

    /*
    ** A normal file (not library):
    **	- don't write info out about static and asm prototypes because
    **	  lint2 only checks consistency of cross-file prototypes
    **	- write out a LDS entry for statics
    **	- write out a LDS entry for asms, and fake a return if the asm
    **	  is non-void
    **	- all other cases write a LDI entry
    */
    else {
	if (isproto) {
	    if ((class == SC_STATIC) || (class == SC_ASM))
		goto skip;
	    else ln2_outdef(func, LPR, nargs);
	} else if (class == SC_STATIC)
	    ln2_outdef(func, LDS, nargs);
	else if (class == SC_ASM) {
	    ln2_outdef(func, LDS, nargs);
	    if (TY_DECREF(SY_TYPE(func)) != TY_VOID)
		ln2_outdef(func, LRV, 0);
	} else ln2_outdef(func, LDI, nargs);
    }


    /* 
    ** Prototype Declaration: 
    **		Write out the arguments for a prototype declaration 
    ** 		(write args to file 1 if LINTLIBRARY was used).
    **
    ** Old-style definition with prototype declaration: 
    ** 		Write out the arguments for a prototype definition.
    */

    if (isproto || TY_HASPROTO(SY_TYPE(func))) {
        FILE *fdeffile = isproto && !LN_DIR(LINTLIBRARY) ? file2 : file1;
	for (argno = 0; argno < args; argno++)
	    ln2_outarg(TY_PROPRM(ty, argno), 0L, fdeffile);
    }

    /*
    ** Old-style defintion:
    **		Write out the arguments for an old-style definition.
    */
    else
	for (argno = 0; argno < args; argno++)
	    ln2_outarg(dcl_efftype(SY_TYPE(dcl_g_arg(argno))), 0L, file1);

    skip:
    /* reset PRINTFLIKE/SCANFLIKE flags to default */
    LN_DIR(PRINTFLIKE) = LN_DIR(SCANFLIKE) = -1;
}



/*
** Check for validity of lint directives - those listed here can only
** be used before a function declaration.
** If ahead of any other declaration, give a warning.
*/
static void
ln2_chkdir()
{
    static const char baduse[] = 
	"directive must precede function definition: /* %s */";
    SX curfunc = ln_curfunc();

    if (curfunc == SY_NOSYM) {
	if (LN_DIR(ARGSUSED) >= 0) {
	    WERROR(baduse, "ARGSUSED");
	    LN_DIR(ARGSUSED) = -1;
	}
	if (LN_DIR(VARARGS) >= 0) {
	    WERROR(baduse, "VARARGS");
	    LN_DIR(VARARGS) = -1;
	}
	if (LN_DIR(PRINTFLIKE) >= 0) {
	    WERROR(baduse, "PRINTFLIKE");
	    LN_DIR(PRINTFLIKE) = -1;
	}
	if (LN_DIR(SCANFLIKE) >= 0) {
	    WERROR(baduse, "SCANFLIKE");
	    LN_DIR(SCANFLIKE) = -1;
	}
    }
}




/*
** Called for every identifier that is added to the symbol table.
** Write information to the interface file.
**
** Write code LDI for:
** 	- symbols that have external linkage defined and initialized
**	  (e.g. int i=2;)
**	- functions with external linkage defined
**	  (e.g. void fun() {})
**
** Write code LDS for:
**	- entities with static linkage defined
**	  (e.g. static int i;  - or - static int fun() {})
**
** Write code LIB for:
**  - entities with external linkage in a file with the lintlibrary directive
**
** Write LDC for tentative definitions (but not for tentative function def's)
**
** Write LDX for function declarations with external linkage
*/
void
ln2_ident(sid)
SX sid;
{
    T1WORD ty = SY_TYPE(sid);
    int def = SY_FLAGS(sid)&SY_DEFINED;
    int fun = TY_ISFTN(ty);
    LNBUG( ln_dbflag > 1, ("ln2_ident: %d", sid));

    /* 
    ** Function definitions are handled in ln2_funcdef.
    ** If this function hasn't been defined and is a prototype, write
    ** out a prototype declaration for it.
    */
    if (fun) {
	int nparam;
	if (def)
	    return;
	else if ((nparam=TY_NPARAM(ty)) != -1) {
	    ln2_funcdef(sid, nparam, 1);
	    return;
	}
    }

    /* Check illegal lint directives. */
    ln2_chkdir();

    if (SY_CLASS(sid) == SC_EXTERN) {
	/*
	** If LINTLIBRARY was used, make this a
	** lint library.  Write out the information if it is defined
	** or if there is a tentative definition.
	*/
	if (LN_DIR(LINTLIBRARY)) {
	    if (   def 
		|| ((SY_FLAGS(sid)&SY_TENTATIVE) && !fun)
		|| (LN_DIR(PROTOLIB) && !fun)
	       )
	    ln2_outdef(sid, LIB, 0);
	}

	/*
	** For any symbol that is defined write out a LDI entry
	** e.g. int i=1;
	**      foo() {}
	*/
	else if (def)
	    ln2_outdef(sid, LDI, 0);

	/*
	** If it is tentative and not a function (not a func declaration
	** either new or old-style), write out a LDC entry
	** e.g. int i;
	*/
	else if ((SY_FLAGS(sid)&SY_TENTATIVE) && (! fun))
	    ln2_outdef(sid, LDC, 0);

	/* 
	** o/w it is a declaration
	*/
	else if (! (SY_FLAGS(sid)&SY_TOMOVE))
	    ln2_outdef(sid, LDX, 0);
    }

    /*
    ** o/w it is a static 
    */
    else if (SY_CLASS(sid) == SC_STATIC) {
	    if (!LN_DIR(LINTLIBRARY) && (SY_LEVEL(sid) == SL_EXTERN) && 
		(SY_FLAGS(sid)&SY_DEFINED))
		    ln2_outdef(sid, LDS, 0);
    }
}


void
ln2_suedef(ty)
T1WORD ty;
{
    SX sid = TY_SUETAG(ty);
    LNBUG(ln_dbflag > 1, ("ln2_suedef: %d\n", ty));

    /*
    ** Set the filename on this symbol.  Save it in string storage
    ** if need be.
    */
    SY_FILE(sid) = st_lookup(er_curname());

    if (SY_LEVEL(sid) != 0)
	ln2_sudef(ty);
}



/*
** Write out the definition for a struct or union.
** For a struct, write out the arguments and member names in the same
** order as they were declared.
** For a union, sort the members because the order of members doesn't
** matter.
*/
static void
ln2_sudef(ty)
T1WORD ty;
{
    int i, nelem;
    SX sid, sutag = TY_SUETAG(ty);
    I7 class = SY_CLASS(sutag);
    char *memname;
    static int maxstruct = 0;
    static SX *un_mem = NULL;
    LNBUG( ln_dbflag > 1, ("ln2_suedef: %d",ty));

    /*
    ** Something messed up here, try to recover though.
    */
    if ((class != SC_STRUCT) && (class != SC_UNION)) {
	lerror(0,"ln2_sudef() wants struct/union type");
	return;
    }

    /*
    ** Not enough memory to hold all the SX's; allocate more.
    */
    nelem = TY_NELEM(ty);

    if (nelem > maxstruct) {
	maxstruct = nelem;
	un_mem = (SX *) malloc(sizeof(SX) * maxstruct);
    }

    ln2_outdef(sutag, LSU, nelem);

    for (i=0;i<nelem;i++)
	un_mem[i] = TY_G_MBR(ty,(unsigned) i);

    if (class == SC_UNION)
	ln_sort(un_mem, nelem);

    for (i = 0; i < nelem; i++) {
	sid = un_mem[i];
	memname = SY_NAME(sid);
	LNBUG(ln_dbflag, ("\tname: %s\n", memname));
	ln2_outarg(SY_TYPE(sid), 0L, file1);
	if (fwrite(memname, strlen(memname)+1, 1, file1) == 0)
	    lerror(FATAL, "can't write member name to file");
    }
}




/*
** Just do a simple exchange sort.
*/
static void
ln_sort(a, n)
SX a[], n;
{
    int i,j;
    SX t;
    LNBUG( ln_dbflag > 1, ("ln_sort"));

    for (i=0;i<(n-1);i++)
	for (j=(i+1);j<n;j++)
	   if (strcmp(SY_NAME(a[i]),SY_NAME(a[j])) > 0) {
		t = a[i];
		a[i] = a[j];
		a[j] = t;
	    }
}
	
	

/*
** Write that the function, curfunc, has a return in it.
** This is called from ln_expr() when a return tree was seen.
** Check to see if this function already has a LRV record
** written (no reason to write more than 1 per function.)
*/
void
ln2_retval()
{
    static SX lastfunc = ND_NOSYMBOL;
    SX curfunc = ln_curfunc();
    LNBUG( ln_dbflag > 1, ("ln2_retval"));

    /* LRV record already written for this func, don't write again. */
    if (curfunc != lastfunc) {
	ln2_outdef(curfunc, LRV, 0);
	lastfunc = curfunc;
    }
}



/*
** Write the current file name to file out.
** If the last filename written to file out is the same as the
** current one, don't write it again.
** Must keep a copy of each of the previous file names for the 
** three files (file4 has no filename records in it.)
*/
static void
ln2_fwrite(out, fnum)
FILE *out;
int fnum;
{
    union rec fsname;
    static char *oldfn[3] = { NULL, NULL, NULL };
    char *ln_filename = er_curname();
    extern char *sourcename;
    LNBUG(ln_dbflag > 1, ("ln2_fwrite"));

    if (def_su) ln_filename = su_fno;
    if (! LN_FLAG('F'))
	ln_filename = strip(ln_filename);

    /* 
    ** File is the same - return
    */
    if (! strcmp(oldfn[fnum-1], ln_filename))
	return;

    /* 
    ** Write the filename record 
    */
    fsname.f.decflag = LFN;
    fwrite((char *)xl_t_rec(&fsname, XL_F), sizeof(fsname), 1, out);

    /*
    ** If this is a lint library, then the filename should be
    ** either:
    **     sourcename -
    **		if current filename is the same as sourcename
    **     sourcename:ln_filename -
    **		if current filename is not the same as sourcename
    **		(i.e.  llib-lc:stdio.h)
    */
    if (LN_DIR(LINTLIBRARY) && strcmp(ln_filename, sourcename)) {
	fwrite(sourcename, strlen(sourcename), 1, out); 
	fputc(':', out);
    }

    fwrite(ln_filename, strlen(ln_filename)+1, 1, out);

    /*
    ** Copy back into the old filename.
    */
    oldfn[fnum-1] = st_lookup(ln_filename);
    LNBUG(ln_dbflag, ("\nFile: %s\n", ln_filename));
}



/*
** Generic routing to output all records to pass2.
** Based on the code, lint selects the right file to write to.
*/
void
ln2_outdef(sid, code, args)
SX sid;
int code, args;
{
    static union rec rc;
    int fno;
    int line = er_getline();
    FILE *out;
    if (def_su && (code&LSU)) line = su_line;
    LNBUG(ln_dbflag > 1, ("ln2_outdef: %d %d %d %d",sid,code,line,args));

    /* Find the right file to write to. */
    switch (code) {
	case LDI: case LDS: case LIB: case LSU: case LPR|LIB:
	    out = file1;
	    fno = 1;
	    break;
	case LDC: case LDX: case LPR:
	    out = file2;
	    fno = 2;
	    break;
	case LRV: case LUV: case LUE: case LUM:
	    out = file3;
	    fno = 3;
	    break;
	default:
	    lerror(FATAL,"bad code in ln2_outdef");
    }

    ln2_fwrite(out, fno);
    ln2_type(&rc.l.type, SY_TYPE(sid));
    rc.l.decflag = (short) code;
    rc.l.nargs = (short) args;
    rc.l.fline = line;

    /*
    ** If PRINTFLIKE or SCANFLIKE set, or in code.
    */
    if (LN_DIR(PRINTFLIKE) > 0)
	rc.l.decflag |= LPF;
    else if (LN_DIR(SCANFLIKE) > 0)
	rc.l.decflag |= LSF;


    if (fwrite((char *)xl_t_rec(&rc, XL_LINE|XL_TY), sizeof(rc), 1, out) == 0)
	lerror(FATAL,"couldn't write record to file");
    if (fwrite(SY_NAME(sid), strlen(SY_NAME(sid))+1, 1, out) == 0)
	lerror(FATAL,"couldn't write record to file");

#ifndef NODBG
    if (ln_dbflag > 0) {
	char *s;
	fprintf(stderr,"\nLINT2:\n");
	fprintf(stderr,"  sid: %d   name: %s   type: %d   extra: %ld   ",
			sid, SY_NAME(sid), rc.l.type.aty, rc.l.type.extra);
	fprintf(stderr,"dcl_mod: %d  args: %d   line: %d   ", rc.l.type.dcl_mod,rc.l.nargs, rc.l.fline);
	fprintf(stderr,"dcl_con: %d  dcl_vol: %d  ", rc.l.type.dcl_con, rc.l.type.dcl_vol);
	switch (rc.l.decflag) {
	    case LUV: s = "LUV"; break;
	    case LUE: s = "LUE"; break;
	    case LDI: s = "LDI"; break;
	    case LIB: s = "LIB"; break;
	    case LDC: s = "LDC"; break;
	    case LDX: s = "LDX"; break;
	    case LUM: s = "LUM"; break;
	    case LRV: s = "LRV"; break;
	    case LDS: s = "LDS"; break;
	    case LSU: s = "LSU"; break;
	    case LPR: s = "LPR"; break;
	    default:  s = "BAD-CODE"; break;
	}
	fprintf(stderr,"code: %s\n", s);
    }
#endif
}



/*
** Write an end marker for each segment of the intermediate file.
*/
#define ENDMARK(fp)	(fwrite((char *)&endmark, sizeof(endmark), 1, fp) == 0)
void
ln2_endmark()
{
    union rec endmark;
    LNBUG(ln_dbflag > 1, ("ln2_endmark"));

    endmark.f.decflag = LND;
    endmark = *xl_t_rec(&endmark, XL_F);

    if (ENDMARK(file1) || ENDMARK(file2) || ENDMARK(file3))
	lerror(FATAL,"couldn't write record to file");
}



static TY
ln2_chgtype(ty)
T1WORD ty;
{
    LNBUG(ln_dbflag > 1, ("ln2_chgtype: %ld", ty));

    switch (ty) {
	case TY_STRUCT:		return LN_STRUCT;
	case TY_UNION:		return LN_UNION;
	case TY_CHAR:		return LN_CHAR;
	case TY_UCHAR:		return LN_UCHAR;
	case TY_SCHAR:		return LN_SCHAR;
	case TY_SHORT:		return LN_SHORT;
	case TY_USHORT:		return LN_USHORT;
	case TY_SSHORT:		return LN_SSHORT;
	case TY_INT:		return LN_INT;
	case TY_UINT:		return LN_UINT;
	case TY_SINT:		return LN_SINT;
	case TY_AINT:		return LN_INT;
	case TY_AUINT:		return LN_UINT;
	case TY_LONG:		return LN_LONG;
	case TY_ULONG:		return LN_ULONG;
	case TY_SLONG:		return LN_SLONG;
	case TY_LLONG:		return LN_LLONG;
	case TY_ULLONG:		return LN_ULLONG;
	case TY_SLLONG:		return LN_SLLONG;
	case TY_FLOAT:		return LN_FLOAT;
	case TY_DOUBLE:		return LN_DOUBLE;
	case TY_LDOUBLE:	return LN_LDOUBLE;
	case TY_VOID:		return LN_VOID;
	case TY_ENUM:		return LN_ENUM;
	default:
	    lerror(0,"bad type to ln2_chgtype()");
	    return LN_INT;
    }
}


static T1WORD *su_stack;
static int max_stack = 0;
static int su_ptr = 0;

static int
on_stack(ty)
T1WORD ty;
{
    int i;
    LNBUG(ln_dbflag > 1, ("on_stack: %d\n", ty));

    for (i=0;i<su_ptr;i++)
	if (su_stack[i] == ty)
	    return 1;
    return 0;
}


#define STCKSZ 200
static void
ln2_suref(ty)
T1WORD ty;
{
    SX sid = TY_SUETAG(ty);
    LNBUG(ln_dbflag > 1, ("ln2_suref: %d", ty));

    if ((SY_LEVEL(sid) == 0) && !ISSET(sid) && !on_stack(ty)) {
	if (su_ptr >= max_stack) {
	    max_stack += STCKSZ;
	    if (max_stack == STCKSZ)
		su_stack = (T1WORD *) malloc(sizeof(T1WORD) * max_stack);
	    else
		su_stack = (T1WORD *) realloc((char *)su_stack, 
					sizeof(T1WORD) * max_stack);
	}
	su_stack[su_ptr++] = ty;
    }
}

void
ln2_def()
{
    int i;
    T1WORD ty;
    SX sid;
    LNBUG(ln_dbflag > 1, ("ln2_def"));

    def_su = 1;

    for (i=0;i<su_ptr;i++) {
	ty = su_stack[i];
	sid = TY_SUETAG(ty);
	SY_FLAGS(sid) |= SY_SET;
	if (TY_HASLIST(ty)) {
	    su_fno = SY_FILE(sid);
	    su_line = SY_LINENO(sid);
	    ln2_sudef(ty);
	}
    }

    def_su = 0;
}
