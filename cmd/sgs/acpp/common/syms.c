/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acpp:common/syms.c	1.79"
/* syms.c - symbol table functions for ANSI cpp	 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include "cpp.h"
#include "buf.h"
#include "file.h"
#include "syms.h"

/* This file contains the macro symbol table and routines that manipulate it
** by processing #define and #undef directives, and related command line options.
*/

#ifdef DEBUG
#	define DBGRET() \
		if (DEBUG('s') > 2)				\
		{						\
			(void)fprintf(stderr, "enter()");	\
			st_mprint(mp);				\
		}
#	define	DBGTRY()	if (DEBUG('d') > 3) \
				{ \
					(void)fprintf(stderr, "try formal=`%.*s'\n", \
					 fp->rlen, fp->ptr.string); \
				}
#	define	DBGFOUND()	if( DEBUG('d') > 3) \
					(void)fputs("found formal\n", stderr);
#else
#	define DBGRET()
#	define	DBGTRY()
#	define	DBGFOUND()
#endif

#define	MACARRAYSZ	300
#define	HASHTABSZ	1023

#define	CLR(x)	(state &= ~(x))
#define SET(x)	(state |= (x))
#define TST(x)	(state & (x))

#define DATESZ	13	/* space for `"mmm dd yyyy"' */
#define FILESZ	30	/* initial value for dynamic character array */
#define LINESZ	10	/* for 32 bit longs, line number limited to `4294967295' */
#define TIMESZ	10	/* space for `"hh:mm:ss"' */

#ifndef PREDMACROS
#	define PREDMACROS ""	/* pre-defined macros */
#endif

typedef struct _option_ Option;
struct _option_ {
	Token * arg;
	Option * next;
};
/* Since the -U options take precedence over -D, the option arguments
** are tokenized, parsed, and then stored until the command line is finished.
** The storage is done on the two following stacks (defstk and undefstk).
** Then the symbol table operations are performed (all -D first, then all -U).
*/
static Option *	defstk;		/* top of stack of -D option arguments	*/	
static Option *	undefstk;	/* top of stack of -U option arguments	*/

/* The  symbol table of macros is hashed */
static Macro *	hashtab[HASHTABSZ]; /* pointers to buckets of Macro binary trees */

/* The ANSI C predefined macros are handled specially.
** Since the contents of their replacement lists may vary, the following
** objects allow access to change certain replacement lists as needed.
*/
static char	datearray[]= "\"           \"";	/* `"mmm dd yyyy"'	*/
static Token	datetk;			/* replacement list for __DATE__	*/
static char *	faptr;			/* current file path (dynamic array)	*/
static unsigned int filelimit = FILESZ+1; /* current limit to length of faptr[]	*/
static Token	filetk;			/* replacement list for __FILE__	*/
static Token	linetk;			/* replacement list for __LINE__	*/
static char	timearray[] = "\"        \"";	/* `"hh:mm:ss"' */
static Token	timetk;			/* replacement list for __TIME__	*/

/* The ANSI C predefined macros are defined in the following array.
** `defined' is included in this array in accordance 
** with the ANSI C prohibition on the user defining such a macro. 
** If the user tries to `#define defined', a diagnostic will be given
** just as if the user tried to #define an ANSI C predefined macro.
*/
static Macro pdmac[] = {
	{"__LINE__", 8, OBJECT_LIKE, &linetk,  (Macro *)0, M__LINE|M_MANIFEST },
	{"__FILE__", 8, OBJECT_LIKE, &filetk,  (Macro *)0, M__FILE|M_MANIFEST },
	{"__STDC__", 8, OBJECT_LIKE,       0,  (Macro *)0, M__STDC|M_MANIFEST },
	{"__DATE__", 8, OBJECT_LIKE, &datetk,  (Macro *)0, M__DATE|M_MANIFEST },
	{"__TIME__", 8, OBJECT_LIKE, &timetk,  (Macro *)0, M__TIME|M_MANIFEST },
	{"defined",  7, OBJECT_LIKE,       0,  (Macro *)0, M_DEFINED },
};

/* Macro allocation may be done from a fixed pool of free Macros.
** If there is no pool, or the pool is empty, new Macros are malloc()'ed.
** #undef'ed macros are not returned to the pool of free Macros.
*/
static Macro macarray[MACARRAYSZ+1];
static Macro * freemacro = macarray + MACARRAYSZ;
static Macro * macarray_ptr = macarray;


#ifdef TRANSITION
/* The number of Macros in the symbol table is tracked to aid
** in the discovery of whether unbounded macro expansion is occuring (-Xt)
*/
static int	nmacros = sizeof(pdmac)/sizeof(Macro);
#endif

#ifdef FILTER
/* To speed up the macro search algorithm, an
** array indexed by character is maintained.
** The character position within the macro corresponds
** to the bit position in the array member.
** When a Macro is introduced into the symbol table,
** each character up to a limit (the value of FILTER)
** causes a bit to be set in this table.
** When investigating if an identifier is in the
** symbol table, this table can indicate if such
** a name may exist in the table.
** In this way, many unecessary symbol table lookups
** can be avoided.
*/
char	st_chars[256];
#endif

#ifdef PERF
#define	BUCKETSZ	10
static	int bszlimit = BUCKETSZ;
static	int bucketsz[BUCKETSZ];	/* for now - should be dynamic */

static	int macro_cnt( 		/* Macro*	*/ );
#endif
static	void	enter(		/* Token *	*/ );
#ifdef FILTER
static	void	enterchars(	/* Macro *	*/ );
#endif
static	Macro **find(		/* char *, int	*/ );
static	void	forget(		/* Token *	*/ );
static	Macro *	newmacro(	/* void		*/ );
static	Option*	newoption(	/* void		*/ );
static	void	setfile(	/* void		*/ );
static	void	setline(	/* long		*/ );
static	Token *	stringchk(	/* Token *, Token * */ );
static	int 	formal_in_str(	/* Token *, Token * */ );

#ifdef PERF
static int
macro_cnt(mp)
	register Macro* mp;
/* Given a pointer to a Macro, this routine links thru the
** macro list to count the number of Macro entries.
*/
{
	register int count;

	count = 0;
	for (; mp; mp = mp->next)
		count++;
	return count;
		
}
#endif

static void
enter(inputp)
	Token * inputp;
/* Given a pointer to a Token list, this routine checks
** for macro redefinition and, if necessary, enters 
** macro definition into the symbol table.
** The first Token in the input list is the macro name (and
** a packed list of formals if there are any), followed by
** the replacement list Tokens, if any.
*/
{
	register Token * tp;	/* Token in the `inputp' list */
	register Macro * mp;	/* macro definition */
	register Macro **mpp;	/* pointer to `mp' */

#ifdef DEBUG
	if (DEBUG('s') > 2)
	{
		(void)fputs("enter called with:", stderr);
		tk_prl(inputp);
	}
	else if (DEBUG('s') > 0)
		(void)fprintf(stderr, "enter(tp=%#lx=%.*s)\n",
			inputp, inputp->rlen, inputp->ptr.string);
#endif
	COMMENT(inputp != 0);
	COMMENT(inputp->code == C_MacroName || inputp->code == C_Manifest);
	tp = inputp;
	mpp = find(tp->ptr.string, tp->rlen);
	if ((mp = *mpp) == (Macro *) 0)
	{
		mp = newmacro();
		*mpp = mp;
		mp->next = 0;
	}
	else if ( !(ST_ISUNDEF(mp)) )
	{
		if (ST_ISCONSTRAINED(mp))
		{
			if (ST_ISPREDEFINED(mp))
				WARN( "ANSI C predefined macro cannot be redefined" );
			else
			{
				COMMENT(ST_ISUNARYOP(mp));
				WARN( "cannot define \"defined\"" );
			}
			COMMENT(tp == inputp);
			tk_rml(tp);
			return;
		}
		if (mp->nformals != tp->aux.argno)
		/* EMPTY */	;
		else if (mp->nformals > 0	/* compare argument spelling */
		 && strcmp(mp->name, tp->ptr.string)
		 && (pp_flags & F_Xc))
		/* EMPTY */	;
		else if (mp->replist != 0 && tp->next == 0)
		/* EMPTY */	;
		else
		{
			register Token *tp2;	/* existing replacement list */

			for (tp2 = mp->replist; (tp = tp->next) != 0; )
			{
				if (tp2 == 0)
					break;
				if (tp->code != tp2->code || tp->rlen != tp2->rlen)
					break;
				if (TK_ISFORMAL(tp)
				 && tp->aux.argno != tp2->aux.argno)
					break;
				else if ((tp->code == C_Paste)
				 && tp->aux.ws != tp2->aux.ws)
					break;
				else if
				 (strncmp(tp->ptr.string, tp2->ptr.string, tp->rlen)!= 0)
					break;
				tp2 = tp2->next;
			}
			if (tp == 0 && tp2 == 0)
			{
				tk_rml(inputp);
				return;
			}
		}
		TKWARN("macro redefined", inputp);
		tk_rml(mp->replist);
		tp = inputp;
		/* for now - free(mp->name) ? */
	}
	mp->name = tp->ptr.string;
	mp->namelen = tp->rlen;
	mp->nformals = tp->aux.argno;
	mp->flags = (tp->code == C_Manifest) ? M_MANIFEST : 0;
	mp->replist = tp = tk_rm(tp);
#ifdef CXREF
	if (pp_flags & F_CXREF) 
		pp_xref(mp, bf_lineno, 'D');
#endif
	for ( ; tp != 0; tp = tp->next)
		TK_SETREPL(tp);
#ifdef INCCOMP
	mp->lineno = fl_baseline();
#endif
#ifdef FILTER
	enterchars(mp);
#endif
	DBGRET();
}

#ifdef FILTER
static void
enterchars(mp)
	Macro * mp;
/*
** Set the bits in the character table, corresponding to the spelling
** of the macro name.
*/
{
	register unsigned char * cp;	/* a character in the macro name	*/

	cp = (unsigned char *) mp->name;
	switch (mp->namelen)
	{
	default:
		/*FALLTHRU*/
	case 8:	st_chars[cp[7]] |= (char) (0x1 << 7);
		/*FALLTHRU*/
	case 7:	st_chars[cp[6]] |= (char) (0x1 << 6);
		/*FALLTHRU*/
	case 6:	st_chars[cp[5]] |= (char) (0x1 << 5);
		/*FALLTHRU*/
	case 5:	st_chars[cp[4]] |= (char) (0x1 << 4);
		/*FALLTHRU*/
	case 4:	st_chars[cp[3]] |= (char) (0x1 << 3);
		/*FALLTHRU*/
	case 3:	st_chars[cp[2]] |= (char) (0x1 << 2);
		/*FALLTHRU*/
	case 2:	st_chars[cp[1]] |= (char) (0x1 << 1);
		/*FALLTHRU*/
	case 1:	st_chars[cp[0]] |= (char) (0x1 << 0);
	}
}
#endif

static Macro **
find(name, len)
/* Given a pointer to the spelling and the length of a macro name,
** this routine returns a pointer to a pointer to the corresponding Macro
** in the symbol table.
** The returned Macro may or may not be active (i.e it may have been
** the object of a #define directive.)
** If there is such no entry in the symbol table,
** the contents of the pointer to Macro location is zero.
*/
	char *name;
	unsigned int len;
{
	register Macro **mpp;	/* return value	*/
	register Macro * mp;	/* pointer to a Macro in the symbol table */
	register unsigned hval;	/* hash value in symbol table */
	register char *	cp;	/* character in the macro name */
	register int l;		/* character offset in the macro name */

	COMMENT(len > 0);
	hval = *(cp = name);
	for (l = len; --l > 0; )
		hval += *++cp << 1;
	hval %= sizeof(hashtab) / sizeof(Macro *);
	mpp = &hashtab[hval];
	cp = name;
	l = len;
	while ((mp = *mpp) != 0)
	{
               if ((mp->namelen == l) && (strncmp(mp->name, cp, l) == 0))
                        break;
                else
                        mpp = &mp->next;
        }
	return (mpp);
}

static void
forget(tp)
/* Given a pointer to an identifier Token, this routine deactivates
** the corresponding macro definition, if any, in the symbol table.
** This routine diagnoses any attempt to undefine an ANSI predefined macro.
*/
	register Token *tp;
{
	register Macro *mp;	/* Macro to be undefined */

#ifdef DEBUG
	if (DEBUG('s') > 0)
		(void)fprintf(stderr, "forget(tp=%#lx=%.*s)\n",
			tp, tp->rlen, tp->ptr.string);
#endif
	COMMENT(tp->code == C_Identifier);
	mp = *find(tp->ptr.string, tp->rlen);
	if (mp == 0)
	{
#ifdef DEBUG
		if (DEBUG('s') > 1)
			(void)fputs("No such macro #define'd\n", stderr);
#endif
		return;
	}
	if (ST_ISUNDEF(mp))
		return;
	if (ST_ISCONSTRAINED(mp))
	{
		if ( (pp_flags & F_Xt) && (mp->flags & M__STDC) )
			WARN( "undefining __STDC__" );
		else {
			if (ST_ISPREDEFINED(mp))
				WARN("ANSI C predefined macro cannot be undefined" );
			else
			{
				COMMENT(ST_ISUNARYOP(mp));
				WARN( "cannot undefine \"defined\"" );
			}
			return;
		}
	}
#ifdef INCCOMP
	{
		Macro * newmp;

		*(newmp=newmacro()) = *mp;
		mp->prevdef = newmp;
	}
#else
	tk_rml(mp->replist);
#endif
	mp->nformals = UNDEFINED;
}

static Macro *
newmacro()
/* This routine returns a pointer to an unused Macro.
** All other fields have garbage values in them.
*/
{
	register Macro *mp;	/* return value */

	if (freemacro < macarray_ptr)
	{
		macarray_ptr = 
			(Macro *) pp_malloc(sizeof(Macro) * (MACARRAYSZ + 1));
		freemacro = macarray_ptr + MACARRAYSZ;
	}
	mp = freemacro--;
	mp->flags = 0;
#ifdef INCCOMP
	mp->prevdef = 0;
#endif
#ifdef TRANSITION
	nmacros++;
#endif
	return mp;
}

static Option *
newoption()
/* This routine returns a pointer to a new Option.
** All the fields of the Option have garbage values in them.
*/
{
	 return (Option *)pp_malloc(sizeof(Option));
}

static void
setfile()
/* Sets the value for the replacement
** list for the predefined __FILE__ macro.
*/
{
	register char *src, *dst;
	register int len;	/* length of file name */
	register int maxlen;	/* max length of resulting file name string */

	src = fl_curname();
	len =  strlen(src); 
	maxlen = 2 * len;
	if ( maxlen  > filelimit - 3 )
		filetk.ptr.string = faptr = 
			pp_realloc(faptr, (unsigned) (filelimit = maxlen + 3));
	dst = faptr;
	*dst++ = '"';
	while (*src) {
		switch(*src) {
			case '\r':
				*dst++ = '\\';
				*dst++ = 'r';
				src++;
				len += 2;
				break;
			case '\n':
				*dst++ = '\\';
				*dst++ = 'n';
				src++;
				len += 2;
				break;
			case '"':
				/*FALLTHRU*/
			case '\\':
				*dst++ = '\\';
				len++;
				/*FALLTHRU*/
			default:
				*dst++ = *src++;
		}
	}
	*dst++ = '"';
	*dst = '\0';
	filetk.rlen = len+2;
}

static void
setline(number)
	long number;
/* Sets the replacement
** list for the predefined __LINE__ macro.
*/
{
	linetk.ptr.string = ch_alloc(LINESZ + 1);
	linetk.rlen = sprintf(linetk.ptr.string, "%ld", number);
	COMMENT(linetk.rlen <= LINESZ);
}

static int
formal_in_str(strtp, list)
	Token * strtp;
	Token * list;
/* Given Token pointers to a string literal or character constant
** pp-token and a canonicalized list of a macro
** definition and its' formals, this routine looks for occurrences of
** the formals within the tokenized contents of the pp-token spelling.
** Return value is 1 if a formal was found, else 0.
*/
{
	register Token * tp;	/* a generic Token pointer */
	int gotformal;	/* boolean: does string spelling contain a formal ? */
	int nformals;	/* number of formal in list */
	char *eosptr;	/* end of strtp string */
	char *bosptr;	/* beginning of strtp string */
	char savechar;	/* save last char while '\0' is used to tokenize */

#ifdef DEBUG
	if (DEBUG('d') > 2)
	{
		(void)fprintf(stderr, "string(strtp=`%.*s', list=`%.*s')\n",
		 strtp->rlen, strtp->ptr.string, list->rlen, list->ptr.string);
		(void)fprintf(stderr, "nformals=%d\n", list->aux.argno);
	}
#endif
	COMMENT(strtp->code == C_String || strtp->code == C_C_Constant);
	tp = strtp;
	bosptr = tp->ptr.string;
	eosptr = bosptr + tp->rlen - 1;
	savechar = *eosptr;
	*eosptr = '\0';
	gotformal = 0;
	nformals = list->aux.argno;
	for (tp = tk_tokenize(bosptr + 1); tp != 0; tp = tk_rm(tp))
	{
		if (tp->code == C_Identifier)
		{
			register int 	i;
			register Token * fp;

			fp = list->next;
			for (i = 0; i < nformals; i++)
			{
				DBGTRY();
				if (tp->rlen == fp->rlen && strncmp
			 	(tp->ptr.string, fp->ptr.string, tp->rlen) == 0)
				{
					DBGFOUND();
					gotformal++;
					break;
				}
				fp = fp->next;
			}
			if (gotformal)
				break;
		}
	}
	*eosptr = savechar;
	return(gotformal);
}

static Token *
stringchk(strtp, list)
	Token * strtp;
	Token * list;
/* Given Token pointers to a string literal or character constant
** pp-token and a canonicalized list of a macro
** definition and its' formals, this routine looks for occurrences of
** the formals within the tokenized contents of the pp-token spelling.
** If there are no occurrences, a pointer to the original
** pp-token is returned. Otherwise, new tokens will be created that
** cause macro replacement within the spelling of the pp-token,
** Then, the return value is a pointer to the list of new Tokens,
** the last of which will point to the "next" field of the original
** pp-token.
**
** This routine aids in the recreation of 4.1 behavior, disallowed
** in ANSI C, where occurrences of a formal within the contents
** of a replacement list string literal will be replaced by the actual parameter.
*/
{
	register Token * tp;	/* a generic Token pointer */
	Token * retlist;	/* the return list when there's a formal	*/
	register Token * retp;	/* the last Token in `retlist'	*/
	register char *	spelling;/* contents of char constant or string literal */
	int gotformal;	/* boolean: does string spelling contain a formal ? */

#ifdef DEBUG
	if (DEBUG('d') > 2)
	{
		(void)fprintf(stderr, "string(strtp=`%.*s', list=`%.*s')\n",
		 strtp->rlen, strtp->ptr.string, list->rlen, list->ptr.string);
		(void)fprintf(stderr, "nformals=%d\n", list->aux.argno);
	}
#endif
	COMMENT(strtp->code == C_String || strtp->code == C_C_Constant);
	tp = strtp;
	spelling = ch_saven(tp->ptr.string, tp->rlen);
	*(spelling + tp->rlen - 1) = '\0';
	gotformal = 0;
	retp = retlist = tk_new();
	COMMENT(spelling[0] == '\"' || spelling[0] == '\'');
	retp->ptr.string = spelling;
	retp->rlen = 1;
	retp->code = C_Goofy;
	for (tp = tk_tokenize(spelling + 1); tp != 0; tp = tk_rm(tp))
	{
		switch (tp->code)
		{
		case C_Identifier:
		{
			register Token * fp;
			register int 	i;
			int nformals;

			fp = list->next;
			nformals = list->aux.argno;
			for (i = 0; i < nformals; i++)
			{
				DBGTRY();
				if (tp->rlen == fp->rlen && strncmp
				 (tp->ptr.string, fp->ptr.string, tp->rlen) == 0)
				{
					DBGFOUND();
					gotformal++;
					retp = retp->next = tk_paste();
					retp = retp->next = tk_new();
					retp->code = C_MergeFormal;
					retp->aux.argno = (short) i;
					retp->rlen = 0;
					goto continu;
				}
				fp = fp->next;
			}
		}
		default:retp = retp->next = tk_paste();
			retp = retp->next = tk_cp(tp); /* for now - don't tk_rm() */
			retp->code = C_Goofy;
		}
continu:	continue;
	}
	if (gotformal)
	{
		retp = retp->next = tk_paste();
		retp = retp->next = tk_new();
		COMMENT(spelling[0] == '\"' || spelling[0] == '\'');
		retp->ptr.string = spelling;
		retp->rlen = 1;
		retp->code = C_Goofy;
		retp->next = strtp->next;
#ifdef DEBUG
		if (DEBUG('d') > 2)
		{
			(void)fputs("string() returns:", stderr);
			tk_prl(retlist);
		}
#endif
		/* for now - tk_rm(strtp) */
		return retlist;
	}
	else
	{
#ifdef DEBUG
		if (DEBUG('d') > 2)
		{
			(void)fputs("string() doesn't change:", stderr);
			tk_pr(strtp, '\n');
		}
#endif
		retp->next = 0;
		tk_rml(retlist);
		return strtp;
	}
}
	
void
st_argdef(arg)
	const char *arg;
/* Given a pointer to an argument string from a -D command line option,
** this routine parses the option and diagnoses violations of the syntax:
** -Dname[=[value]].
*/
{
	register Token* tp;	/* a Token in the option argument	*/
	register const char *cp = arg;
	Token *base = 0;
	static Option *endstk;

#ifdef DEBUG
	if (DEBUG('s') > 0)
		(void)fprintf(stderr, "argdef(arg=%s)\n", arg);
#endif
	/* Must do own tokenizing here.  Consider case of 
	** -Dmacro==5.  This should be parsed as "macro",
	** "=", "=", "5".  Instead, tk_tokenize would return
	** "macro", "==", "5", which is incorrect.
	*/
	if(isalpha(*cp) || *cp == '_') {	/* parse identifier */
		do {
			++cp;
		} while(isalpha(*cp) || isdigit(*cp) || *cp == '_');
		base = tp = tk_new();
		tp->ptr.string = (char *)arg;
		tp->rlen = cp - arg;
		tp->code = C_Identifier;
		tp->number = bf_lineno;
		tp->next = 0;
		tp->aux.hid = 0;
		while(isspace(*cp))
			++cp;
		if (*cp == '=') {		/* Don't parse == */
			tp->next = tk_new();
			tp = tp->next;
			tp->ptr.string = (char *)cp;
			tp->rlen = 1;
			tp->code = C_Assign;
			tp->number = bf_lineno;
			tp->next = 0;
			++cp;
		}
		tp->next = tk_tokenize(cp);
		tp = base;
	}
	else
		tp = tk_tokenize(arg);
		
	if (tp->code != C_Identifier)
		UERROR( "-D option argument not an identifier" );
	else
	{
		register Option * p;	/* a member of the -D option stack */
		Token * name;		/* the identifier in the option argument */

		tp->code = C_MacroName;
		tp->aux.argno = OBJECT_LIKE;
		name = tp;
		tp = tp->next = tk_rmws(tp->next);
		if ( tp == (Token *) 0)
			name->next = tk_bool(1);
		else
		{
			if (tp->code == C_Assign)
					name->next = tk_rmws(tk_rm(tp));
			else
			{
				WARN( "-D option argument not followed by \"=\"" );
				tk_rml(tp);
				name->next = tk_bool(1);
			}
		}
		p = newoption();
		p->arg = name;
		
		/* Maintain stack in order so that -Dmacro=1 -Dmacro=2
		** returns 2 for macro.  This is to avoid unnecessary
		** deviations from the old cpp.
		*/
		p->next = 0;
		if (defstk)
			endstk->next = p;
		else
			defstk = p;
		endstk = p;
	}
}

void
st_argundef(arg)
	const char *arg;
/* Given a pointer to an argument string from a -U command line option,
** this routine parses the option and diagnoses syntax violations.
*/
{
	register Token * tp;	/* a Token in the option argument	*/

#ifdef DEBUG
	if (DEBUG('s') > 0)
		(void)fprintf(stderr, "argundef(arg=\"%s\")\n", arg);
#endif
	tp = tk_tokenize(arg);
	if (tp->code != C_Identifier)
		UERROR( "-U option argument not an identifier" );
	else
	{
		register Option * p;	/* a member of the -U option stack */

		p = newoption();
		p->arg = tp;
		p->next = undefstk ? undefstk : 0;
		undefstk = p;
		if ((tp = tp->next = tk_rmws(tp->next)) != 0)
		{
			WARN( "tokens ignored after \"-U{identifier}\"" );
			tk_rml(tp);
		}
	}
}

Token *
st_define(list)
	Token *	list;
/* Given a pointer to a Token list from a #define directive,
** this routine diagnoses any syntax errors and invokes the
** entry of the macro definition into the symbol table.
*/
{
	Token *args, *repl;
	register Token *tp, *prev, *fp;
	register int state;
	register int nformals;
	register char *p;
	int len;
	int i;

	/* values for state */

#	define	S_start		0
#	define	S_gotformal	0x01
#	define	S_gotrparen	0x02
#	define	S_zapwhite	0x04
#	define	S_gotsharp	0x08
#	define	S_gotpaste	0x10
#	define	S_furtherop	0x20

#ifdef DEBUG
	if (DEBUG('d') > 0)
		(void)fprintf(stderr, "st_define(list=%#lx)\n", list);
#endif
	/*
	* Keep macro's in canonical form:  All white-space
	* before and after the replacement tokens is tossed.
	* All white-space between tokens replaced with a
	* single blank.  Replace formal parameters with marks
	* as appropriate.  Keep the original formal names as
	* a packed argument list for checking later.
	*/
	if ((tp = list) == 0)
	{
		UERROR("empty #define directive line");
		return (0);
	}
	if (tp->code != C_Identifier)
	{
		UERROR("#define requires macro name");
		return (tp);
	}
	len = tp->rlen;
	prev = tp;
	repl = 0;
	args = 0;
	state = S_start;
	/*
	* Canonicalize the parameter token list.
	*/
	if ((tp = tp->next) == 0 || tp->code != C_LParen)
	{
		nformals = OBJECT_LIKE;	/* no parameters for this macro */
	}
	else
	{
		nformals = 0;
		for (;;)
		{
			prev->next = tp->next;
			tp = tk_rm(tp);
			if (TST(S_gotrparen))
				break;
			if (tp == 0)
			{
				UERROR("incomplete #define macro parameter list");
				return (list);
			}
		again:;
			switch (tp->code)
			{
			default:
				TKERROR("invalid token in #define macro parameters", tp);
				return (list);

			case C_RParen:
				if (TST(S_gotformal|S_gotrparen) == 0
				 && nformals > 0)
					goto badlist;
				SET(S_gotrparen);
				continue;

			case C_WhiteSpace:
				continue;

			case C_Identifier:
				if (TST(S_gotformal|S_gotrparen))
				{
				badlist:;
					UERROR("syntax error in macro parameters");
					return (list);
				}
				SET(S_gotformal);
				CLR(S_gotrparen);
				fp = args;
				
				/* Loop through args looking for duplicate name. */
				for (i=0;  i<nformals;  i++ ) {
				    if (tp->rlen == fp->rlen
				        && !strncmp(tp->ptr.string, 
					            fp->ptr.string, 
					            tp->rlen)
					) {
				        TKWARN("duplicate formal parameter",tp);
					break;
				    }
				    fp = fp->next;
				}
				/* remember last formal with this name */
				if (i == nformals)	/* no duplicate */
					fp = tp;
				fp->aux.argno = (short) nformals;

				nformals++;
				len += tp->rlen + 1;	/* room for separator */
				prev = tp;
				if (args == 0)
					args = tp;
				tp = tp->next;
				goto again;

			case C_Comma:
				if (TST(S_gotformal|S_gotrparen) == 0)
					goto badlist;
				CLR(S_gotformal|S_gotrparen);
				continue;
			}
		}
	}
	/*
	* Canonicalize the replacement token list.
	* Replace code for each use of formals:
	*	formal		->	C_Formal
	*	# formal	->	C_StrFormal
	*	formal ##	->	C_UnexpFormal
	*	## formal	->	C_UnexpFormal
	*
	* The resulting code is pretty horrible...
	*/
	SET(S_zapwhite);
	CLR(S_gotpaste|S_gotsharp); 
	fp = 0;
	while (tp != 0)
	{
		switch (TK_ENUMNO(tp->code))
		{
		CASE(C_Sharp)
			SET(S_furtherop);
			if (nformals == OBJECT_LIKE)
			{
				CLR(S_gotsharp|S_gotpaste);
				break;
			}
			if (TST(S_gotsharp|S_gotpaste))
			{
				WARN("bad use of \"#\" or \"##\" in macro #define");
			}
			CLR(S_gotpaste|S_zapwhite);
			SET(S_gotsharp);
			prev->next = tp->next;
			tp = tk_rm(tp);
			continue;

		CASE(C_Paste)
			SET(S_furtherop);
			tp->aux.ws = 0;
			if (TST(S_gotsharp|S_gotpaste))
			{
				WARN("bad use of \"#\" or \"##\" in macro #define");
				CLR(S_gotsharp|S_gotpaste);
			}
			SET(S_gotpaste|S_zapwhite);
			if (fp == 0)
				WARN("cannot begin macro replacement with \"##\"");
			if (prev->code == C_WhiteSpace)
			{
				COMMENT((tp->aux.ws & W_RIGHT) == 0);
				tp->aux.ws = W_LEFT;
				fp->next = tp;
				(void)tk_rm(prev);
				len--;
				prev = fp;
			}
			if (prev->code == C_Formal)
				prev->code = C_UnexpFormal;
			break;
		CASE(C_Identifier)	/* check for formal parameter */
		{
			register int i;

			SET(S_furtherop);
			fp = args;
#ifdef TRANSITION
			if (pp_flags & F_Xt)
			{
				if (tp->rlen == list->rlen
				 && strncmp(tp->ptr.string, list->ptr.string, tp->rlen) == 0)
					tp->aux.hid = 1;
			}
#endif
			for (i = 0; i < nformals; i++)
			{
				if (fp->rlen == tp->rlen &&
					strncmp(tp->ptr.string, fp->ptr.string, tp->rlen) == 0) 
					break;
				fp = fp->next;
			}
			if (i < nformals) {
				tp->rlen = 0;
				tp->code = (TST(S_gotsharp)) ? C_StrFormal
					: (TST(S_gotpaste)) ? C_UnexpFormal
					: C_Formal;
				tp->aux.argno = fp->aux.argno;
			}
			else if (TST(S_gotsharp))
				UERROR("non-formal identifier follows \"#\" in #define");
			CLR(S_gotsharp|S_gotpaste);
			break;
		}
		CASE(C_WhiteSpace)
			if (TST(S_gotpaste))
			{
				COMMENT(prev->code == C_Paste);
				prev->aux.ws |= W_RIGHT;
			}
			if (TST(S_zapwhite))
			{
				prev->next = tp->next;
				tp = tk_rm(tp);
			}
			else
			{
#ifdef TRANSITION
				if (tp->ptr.string[0] == '/'
				 && tp->next != 0
				 && (TST(S_gotsharp)) == 0
				 && (	(prev->code == C_Identifier)
				 	|| (prev->code == C_I_Constant)
				 	|| (prev->code == C_Formal)
				 	|| (prev->code == C_UnexpFormal)
				    )
				 && (	(tp->next->code == C_Identifier)
				 	|| (tp->next->code == C_I_Constant)
				    ) )
				{
					COMMENT((TST(S_gotpaste)) == 0);
					COMMENT(tp->ptr.string[1]=='*'
				 	 ||tp->ptr.string[1]=='/');
					if (pp_flags & F_Xt)
					{
						WARN("comment is replaced by \"##\"");
						tp->ptr.string = "##";
						tp->rlen = 2;
						tp->code = C_Paste;
						tp->aux.ws = 0;
						SET(S_gotpaste|S_zapwhite);
						if (prev->code == C_Formal)
							prev->code = C_UnexpFormal;
						SET(S_furtherop);
						break;
					}
					else
						WARN( "comment does not concatenate tokens" );
				}
#endif
				SET(S_zapwhite);
				tp->ptr.string = " ";
				tp->rlen = 1;
				fp = prev;	/* in case at end */
				len++;
				prev = tp;
				tp = tp->next;
			}
			continue;

#ifdef TRANSITION
		CASE(C_C_Constant)
		CASE(C_String)
		{
			Token * temp;
			static	char no[] = "no ";
			static	char body[] = "macro replacement within a "; /*WARN*/
			static	char str[] = "string literal";
			static	char chr[] = "character constant";
			char msg[sizeof(no) - 1 + sizeof(body) - 1 +
			 (sizeof(chr) > sizeof(str) ? sizeof(chr) : sizeof(str))];

			if (nformals > 0)
			{
				list->aux.argno = (short) nformals;
				if (pp_flags & F_Xt)
				{
					if ((temp = stringchk(tp, list)) != tp)
					{
						(void) strcpy(msg, body);
						(void) strcat(msg,
						 tp->code == C_String ? str : chr);
						WARN(msg);
						SET(S_furtherop);
						prev->next = tp = temp;
						continue;
					}
				}
				else
				{
					if (formal_in_str(tp, list))
					{
						(void) strcpy(msg, no);
						(void) strcat(msg, body);
						(void) strcat(msg,
						 tp->code == C_String ? str : chr);
						WARN(msg);
					}
				}
			}
			CLR(S_gotpaste);
			break;
		}
#endif
		default:CLR(S_gotpaste);
			break;
		}
		fp = prev;
		if (TST(S_gotsharp))
		{
			UERROR("\"#\" must be followed by formal identifier in #define");
			CLR(S_gotsharp|S_zapwhite);
		}
		else if ((TST(S_gotpaste) && TST(S_zapwhite)) == 0)
			CLR(S_zapwhite);
		if (repl == 0)
			repl = tp;
		len += tp->rlen;
		prev = tp;
		tp = tp->next;
	}
	if (TST(S_gotpaste|S_gotsharp))
		WARN("cannot end macro replacement with \"#\" or \"##\"");
	if (fp != 0)
	{
		if (fp->next->code == C_WhiteSpace)
		{
			(void)tk_rm(fp->next);
			fp->next = 0;
			len--;
		}
	}
#ifdef DEBUG
	if (DEBUG('d') > 2)
	{
		(void)fputs("dodefine(): After canonicalization: ", stderr);
		tk_prl(list);
	}
#endif
	/*
	* Pack information into Token list as expected by enter().
	* First in list is the name being #define'd with its number
	* of formals in aux.argno.  Followed by the replacement Token's.
	* A single shared buffer is used for the characters in the
	* following form:
	*	<name><formal1>,<formal2>,...,<formal-n>\0<first repl> ...
	*	^					  ^
	*     First->ptr.string				Replacement->ptr.string
	*/
	if (len == 0) pp_internal("len is 0");
	p = ch_alloc(len + sizeof((char)'\0'));
	tp = list;
	tp->code = TST(S_furtherop) ? C_MacroName : C_Manifest;
	(void)strncpy(p, tp->ptr.string, tp->rlen);
	tp->ptr.string = p;
	tp->aux.argno = (short) nformals;
	p += tp->rlen;
	prev = tp;
	tp = tp->next;
	if (nformals > 0)
	{
		do
		{
			(void)strncpy(p, tp->ptr.string, tp->rlen);
			p += tp->rlen;
			*p++ = ',';
			tp = tk_rm(tp);
		} while (--nformals > 0);
		p[-1] = '\0';	/* end of packed formals */
	}
	prev->next = tp;
	while (tp != 0)
	{
		(void)strncpy(p, tp->ptr.string, tp->rlen);
		tp->ptr.string = p;
		p += tp->rlen;
		tp = tp->next;
	}
	*p = '\0';
#ifdef DEBUG
	if (DEBUG('d') > 3)
	{

		(void)fputs("After copy & packing: `", stderr);
		pp_printmem(list->ptr.string, len);
		(void)fputs("': ", stderr);
		tk_prl(list);
	}
#endif
	enter(list);
	return (0);
}

int
st_hidden(mp)
	Macro *	mp;
/* Given a pointer to an active Macro definition, this routine
** returns a non-zero number if the macro may be expanded.
*/
{
	COMMENT(find(mp->name, mp->namelen) != 0);
	COMMENT((*find(mp->name, mp->namelen))->nformals != UNDEFINED);
	return mp->flags & M_HIDDEN;
}

void
st_hide(mp)
	Macro *	mp;
/* Given a pointer to an active Macro definition, this routine
** sets an indication in the symbol table that the macro may not be expanded.
** It is expected that the macro is not already "hidden".
*/
{
	COMMENT(find(mp->name, mp->namelen) != 0);
	COMMENT((*find(mp->name, mp->namelen))->nformals != UNDEFINED);
	COMMENT((mp->flags & M_HIDDEN) == 0);
	mp->flags |= M_HIDDEN;
#ifdef DEBUG
	if ( DEBUG('s') > 0 )
		(void)fprintf(stderr,"st_hide(mp=%#lx=%.*s)\n",mp,mp->namelen,mp->name);
#endif
}

void
st_init(nodefaults)
	int nodefaults;	/* boolean: if true, there are no predefined macros */
/* Given a flag that indicates if there shall be any unix predefined macros,
** this routine initializes the data structures in syms.c.
** This routine must be called after command line is processed,
** because the -X flag and -A- command line options affects predefined macros.
*/
/*
** The static array predefined is inititalized to contain the value of 
** the macro PREDMACROS, whic is set in the makefile. These are the predefined
** macros like "unix" and "u3b2".
**
*/
{
	register Macro ** mpp;	/* a symbol table location */
	register Macro * mp;	/* a member of pdmac[]	*/
	register char * cp;	/* information returned by asctime(3C) */
	static long	myclock;/* used to set __DATE__ and __TIME__	*/
	static char * predefined[] = {PREDMACROS};

	if (predefined != 0 && nodefaults == 0 && (pp_flags & F_Xc) == 0)
	{
		register Token* tp;	/* a unix predefined macro name */
		register char ** cpp;	/* points to a member of predefined[] */

	
		cpp = predefined + (sizeof(predefined)/sizeof(char *)) - 1;
		do
		{
			tp = tk_new();
			tp->ptr.string = *cpp;
			tp->rlen = strlen(*cpp);
			tp->aux.argno = OBJECT_LIKE;
			tp->code = C_Manifest;
			tp->next = tk_bool(1);
			tp->number = 0;
			enter(tp);
		} while (cpp-- != predefined);
	}
	myclock = time(0);
	cp = asctime(localtime(&myclock));
	(void) memcpy(datearray+1, cp+4, 7);
	(void) memcpy(datearray+8, cp+20, 4);
	datetk.ptr.string = datearray;
	datetk.rlen = DATESZ;
	datetk.code = C_String;
	TK_SETREPL(&datetk);
	(void) memcpy(timearray+1, cp+11, 8);
	timetk.ptr.string = timearray;
	timetk.rlen = TIMESZ;
	timetk.code = C_String;
	TK_SETREPL(&timetk);
	filetk.ptr.string = faptr = pp_malloc(filelimit);
	filetk.code = C_String;
	TK_SETREPL(&filetk);
	linetk.code = C_I_Constant;
	TK_SETREPL(&linetk);
	for (mp = pdmac; mp < pdmac + 6; mp++)
	{
		mpp = find(mp->name, (unsigned int) mp->namelen);
		*mpp = mp;
#ifdef FILTER
		enterchars(mp);
#endif
	}
	COMMENT((pdmac[2].flags & M_PREDEFINED) == M__STDC);
	pdmac[2].replist = tk_bool((unsigned long) (pp_flags & F_Xc));
	{
		register Option* p;	/* -D and -U option stacks */

		for(p = defstk; p; p = p->next)
			enter(p->arg);
		for(p = undefstk; p; p = p->next)
			forget(p->arg);
	}
}

Macro *
st_ismacro(tp)
/* Given a pointer to an identifier Token, this routine
** returns  a pointer to the corresponding active entry in the
** symbol table. If there is no entry, or it is not active, a
** 0 is returned.
** If the return value is an ANSI C Predefined Macro, this routine
** will insure that it's replacement list is set properly.
** (i.e. __TIME__ is set to the current time)
*/
	register Token *tp;
{
	register Macro  *mp;	/* return value	*/
#ifdef FILTER
	register unsigned char * cp;	/* character in macro name */
#endif

#ifdef DEBUG
	if (DEBUG('s') > 0)
		(void)fprintf(stderr, "lookup(tp=%#lx=%.*s)",
			tp, tp->rlen, tp->ptr.string);
#endif
	COMMENT(tp->code == C_Identifier);
#ifdef FILTER
	cp = (unsigned char *)tp->ptr.string;
	switch (tp->rlen)
	{
	default:
	case 8:	if ((st_chars[cp[7]] & (0x1 << 7)) == 0)
		{
			mp = 0;
			goto ret;
		}
		/*FALLTHRU*/
	case 7:	if ((st_chars[cp[6]] & (0x1 << 6)) == 0)
		{
			mp = 0;
			goto ret;
		}
		/*FALLTHRU*/
	case 6:	if ((st_chars[cp[5]] & (0x1 << 5)) == 0)
		{
			mp = 0;
			goto ret;
		}
		/*FALLTHRU*/
	case 5:	if ((st_chars[cp[4]] & (0x1 << 4)) == 0)
		{
			mp = 0;
			goto ret;
		}
		/*FALLTHRU*/
	case 4:	if ((st_chars[cp[3]] & (0x1 << 3)) == 0)
		{
			mp = 0;
			goto ret;
		}
		/*FALLTHRU*/
	case 3:	if ((st_chars[cp[2]] & (0x1 << 2)) == 0)
		{
			mp = 0;
			goto ret;
		}
		/*FALLTHRU*/
	case 2:	if ((st_chars[cp[1]] & (0x1 << 1)) == 0)
		{
			mp = 0;
			goto ret;
		}
		/*FALLTHRU*/
	case 1:	if ((st_chars[cp[0]] & (0x1 << 0)) == 0)
		{
			mp = 0;
			goto ret;
		}
	}
#endif
	mp = *find(tp->ptr.string, tp->rlen);
	if (mp != 0)
		if (ST_ISUNDEF(mp))
			mp = 0;
		else if (ST_ISCONSTRAINED(mp))
			switch (mp->flags & M_CONSTRAINED)
			{
			case M_DEFINED:
				mp = 0;
				break;

			case M__LINE:
				setline(tp->number);
				break;

			case M__FILE:
				setfile();
				break;
			}
ret:	;
#ifdef DEBUG
	if (DEBUG('s') > 2 && mp)
	{
		(void)fputs( "returns mp:\n", stderr);
		st_mprint(mp);
	}
	else if (DEBUG('s') > 0)
		(void)fprintf(stderr," returns mp=%#lx\n",mp);
#endif
	return mp;
}

#ifdef DEBUG
void
st_mprint(mp)
Macro *	mp;
/* Given a pointer to a Macro, this routine
** prints out the fields of a Macro structure for debugging.
*/
{
	(void)fprintf(stderr, "\tName:	%.*s\n\tManifest: %s\n\tNfrmls:\t%d\n\tReplacement ",
	 mp->namelen, mp->name, ST_ISMANIFEST(mp) ? "Yes" : "No", mp->nformals);
	if ( mp->replist )
	{
		tk_prl(mp->replist);
	}
	else
		(void)fputs("Token list: NULL", stderr);
	(void)putc('\n', stderr);
}
#endif

#ifdef TRANSITION
int
st_nmacros()
/* This routine returns the number of Macros in the symbol table. */
{
	return nmacros;
}
#endif

#ifdef PERF
void
st_perf()
/* Prints out symbol table performance information. */
{
	register int hval;
	register int n;
	register int nmacros;
	FILE* perfile;

	if ((perfile = fopen("st_perf.out", "a")) == NULL)
		FATAL( "cannot open st_perf.out" );
	for (nmacros = 0, hval = 0; hval<HASHTABSZ; hval++)
	{
		nmacros += (n = macro_cnt(hashtab[hval]));
		if (n >= bszlimit)
			FATAL( "bucketsz[] too small" );
		else
			bucketsz[n]++;
	}
	(void)fprintf(perfile, "%s: symbol table size = %d\n",fl_curname(), nmacros);
	(void)fputs("\nDistribution of Bucket sizes (indicating hash function effectiveness)\n", perfile);
	(void)fprintf(perfile, "Size(no. of macros)  Buckets\tPercent of buckets\n");
	for (n = 0; n < bszlimit; n++ )
	{
		(void)fprintf(perfile, "\t%d\t\t%d\t\t%7.3f\n",
		 n, bucketsz[n], 100*bucketsz[n]/(float)HASHTABSZ);
		nmacros -= bucketsz[n] * n;
		
	}
	if (nmacros)
		(void)fputs("ERROR in counting buckets\n", perfile);
	close(perfile);
}
#endif

Token *
st_undef(tp)
	register Token *tp;
/* Given a pointer to a Token list from a #undef directive,
** this routine diagnoses any syntax errors and initiates the
** deactivation of the macro definition in the symbol table.
*/
{
#ifdef DEBUG
	if (DEBUG('d') > 0)
		(void)fprintf(stderr, "doundef(tp=%#lx)\n", tp);
#endif
	if (tp == 0)
	{
		UERROR("empty #undef directive, identifier expected");
		return (Token *)0;
	}
	if (tp->code == C_Identifier)
	{
		forget(tp);
		tk_extra(tp->next);
		tp->next = 0;
	}
	else
		UERROR( "identifier expected after #undef" );
	return tp;
}
	
void
st_unhide(mp)
Macro *	mp;
/* Sets an indication in the symbol table that the macro
** pointed to by the function argument may be expanded.
** It is assumed that this macro has already been "hidden" by st_hide().
*/
{
	COMMENT(find(mp->name, mp->namelen) != 0);
	COMMENT((*find(mp->name, mp->namelen))->nformals != UNDEFINED);
	COMMENT((mp->flags & M_HIDDEN) != 0);
	mp->flags ^= M_HIDDEN;
#ifdef DEBUG
	if ( DEBUG('s') > 0 )
                (void)fprintf(stderr, "st_unhide(mp=%#lx=%.*s)\n",
		 mp, mp->namelen, mp->name);
#endif
}
