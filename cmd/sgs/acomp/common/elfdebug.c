/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/elfdebug.c	55.7.3.1"
/* elfdebug.c */

#ifdef ELF_OBJ

/* Define db_curline whether or not LINT is defined since it is used
 * in lex.c and aclex.l.
 */

int db_curline = 0;		/* start line of current statement */
#endif

#ifndef LINT

/* Code to support standard debugging output in ELF format.  The various
** flavors of output that are supported are:
**
**	1)  Start and end of function.
**	2)  Start and end of block.
**	3)  Symbol.
**	4)  Line number.
**
*/


#include "p1.h"

#ifdef ELF_OBJ				/* ELF-style debug output */

#include <stdio.h>
#include <string.h>
#include "dwarf.h"
#ifdef	__STDC__
#include <stdarg.h>
static void db_out(const char *, ...);
#else
#include <varargs.h>
static void db_out();
#endif

/* Define strings that introduce data generation pseudo-ops. */

#ifndef	DB_BYTE
#define	DB_BYTE	".byte"
#endif
#ifndef	DB_2BYTE
#define	DB_2BYTE ".2byte"
#endif
#ifndef	DB_4BYTE
#define	DB_4BYTE ".4byte"
#endif
#ifndef	DB_DOTSET
#define	DB_DOTSET ".set"
#endif
#ifndef	DB_DOTALIGN
#define	DB_DOTALIGN ".align"
#endif
#ifndef	DB_DOTSTRING
#define	DB_DOTSTRING ".string"
#endif

/* Section names.
** The first two are simply the section names.  The last is
** the full directive that resets to the previous section.
*/
#ifndef	DB_DOTDEBUG
#define	DB_DOTDEBUG ".debug"	/* Most debug info. */
#endif
#ifndef	DB_DOTLINE
#define	DB_DOTLINE ".line"		/* Line number information. */
#endif
#ifndef	DB_DOTPREVIOUS
#define	DB_DOTPREVIOUS "\t.previous\n"	/* Back to previous section. */
#endif

/* Label names. */
#ifndef LAB_DEBUG
#define LAB_DEBUG	"..D%d"		/* general debug label */
#endif
#ifndef LAB_LINENO
#define LAB_LINENO	"..LN%d"	/* line number label */
#endif
#ifndef LAB_FUNCNO
#define LAB_FUNCNO	"..FS%d"	/* function start label */
#endif
#ifndef LAB_TYPE
#define LAB_TYPE	"..T%d"		/* general type label */
#endif
#ifndef LAB_LAST
#define LAB_LAST	"..D%d.e"	/* debug end of block */
#endif
#ifndef LAB_PCLOW
#define	LAB_PCLOW	"..D%d.pclo"	/* debug-associated low pc */
#endif
#ifndef LAB_PCHIGH
#define	LAB_PCHIGH	"..D%d.pchi"	/* debug-associated high pc */
#endif
#ifndef	LAB_TEXTBEGIN
#define LAB_TEXTBEGIN	"..text.b"	/* beginning of text label */
#endif
#ifndef LAB_TEXTEND
#define LAB_TEXTEND	"..text.e"	/* end of text label */
#endif
#ifndef	LAB_LINEBEGIN
#define	LAB_LINEBEGIN	"..line.b"	/* start of line numbers */
#endif
#ifndef	LAB_LINEEND
#define	LAB_LINEEND	"..line.e"	/* end of line numbers */
#endif

/* This macro converts the register number in a symbol table entry's
** offset field into the register number that the debugger expects.
*/
#ifndef	DB_OUTREGNO
#define	DB_OUTREGNO(sid) (SY_REGNO(sid))
#endif


int db_linelevel = DB_LEVEL2;	/* line number debugger info level */
int db_symlevel = DB_LEVEL2;	/* symbol debugger info level */
static char * db_filename = "";	/* name of file for debug info */
static char * db_funcname = "";	/* name of function to generate label */
static int db_lastlineno;	/* line number at last line number output */
static int db_labno = 0;	/* current debug label number */
#define DB_GENLAB() (++db_labno)
static FILE * dbout = stdout;

static void db_begfile();
static void db_ucsymbol();
static void db_uclineno();
static void db_outmember();
static void db_outarray();
static void db_outfunction();
extern int db_curline;		/* current statement line */

/* This structure keeps track of whether a given type ID has had
** a user-defined type generated for it.  It's simply a bitmap,
** one bit per type number.
*/
#ifndef INI_DBTYPES
#define INI_DBTYPES 100		/* number of bytes to represent type IDs */
#endif

/* Maximum type number represented, +1. */
static TY_TYPEID db_maxtype = INI_DBTYPES*8;
/* Highest type number that needs to be produced eventually. */
static TY_TYPEID db_maxneedtype = 0;

static unsigned char db_tymask[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};
typedef unsigned char db_typebits[2];
#define DB_OUTBIT 0		/* 0-th element is for types already output */
#define DB_NEEDBIT 1		/* 1-st element is for types that are needed */
TABLE(Static, td_dbtypes, static, dbtypes_init,
		db_typebits, INI_DBTYPES, TD_ZERO, "type ID flags");

#define DB_TY_CHKOUT(t) db_ty_chkout(t)
#define DB_TY_SETOUT(t) db_ty_setout(t)
#define DB_TY_CHKNEED(t) db_ty_chkneed(t)
#define DB_TY_SETNEED(t) db_ty_setneed(t)

/* These data support a stack of entries that holds the label numbers
** for the current information entry and its sibling pointer at each level.
*/
#ifndef INI_DBSTACK
#define INI_DBSTACK 20		/* initial maximum depth of stack */
#endif

typedef struct {
    int db_infolab;		/* label of current info entry */
    int db_sibling;		/* sibling of current entry */
} db_stack;
TABLE(Static, td_dbstack, static, db_stack_init,
		db_stack, INI_DBSTACK, 0, "debug sibling stack");
#define DB_CURINFOLAB \
	(TD_ELEM(td_dbstack,db_stack,TD_USED(td_dbstack)).db_infolab)
#define DB_CURSIBLING \
	(TD_ELEM(td_dbstack,db_stack,TD_USED(td_dbstack)).db_sibling)


#ifndef	INI_BBFLAGS
#define	INI_BBFLAGS 10
#endif

/* This structure (array) holds flags that say whether a a debug
** entry has been generated for a particular block level.
*/
TABLE(Static, td_bbflags, static, bbflags_init,
		char, INI_BBFLAGS, 0, "block flags");
#define bblevel (TD_USED(td_bbflags))
#define BBFLAG (TD_ELEM(td_bbflags,char,bblevel))


/* Generate formatted output based on character string.
** The routine is printf-like, except only recognized characters
** are supported, including those output literally.  Escapes are:
**	L[dt]		output debug/type label (label follows)
**	LD		output end label corresponding to Ld
**	Lc		output label of current debug info entry
**	LC		output end label corresponding to Lc
**	L[lh]		output low/high pc label for current debug info entry
**	L[BE]		output begin/end text label
**	L[be]		output begin/end line numbers labels
**	Ln		output label for line number (number follows)
**	LL		output text label (number follows)
**	Lf		output function start label
**	B		output byte pseudo-op
**	C		output assembly comment delimiter and string
**	H		output unaligned halfword pseudo-op
**	S[dlp]		set section to debug/line/previous
**	SI		initialize sections with proper attributes
**	W		output unaligned word pseudo-op
**	b		output string as a bunch of hex-valued bytes + NUL
**	d		output decimal value
**	s		output string
**	x		output hex value
**	.d		output assembler expression that assigns selected
**			label the value of ".".
**	.[CD]		output assembler expression that assigns end label for
**			current debug info entry or selected one the value ".".
**	.[hl]		output assembler expression that assigns hi/lo pc label
**			for current debug info entry the value of ".".
**	.[BE]		output assembler expression that assigns begin/end text
**			label the value of ".".
**	.[be]		same for begin/end line numbers
**	.n		output assembler expression to assign line number label
**			the value of ".".
**	.f		output assembler expression to assign function start
**			label the value of ".".
**	=		output assembler expression that makes type label
**			in next arg equivalent to current debug info entry's
**			label
**	'		on recursive call, take next character literally
**	literal chars:  \t SP \n : ; -
**
** On a recursive call, any character is permitted, and is taken
** to be literal.
*/
static void
#ifdef __STDC__
db_out(register const char * fmt, ...)
{
#else
db_out(va_alist)
va_dcl
{
    register char * fmt;
#endif
    va_list ap;
    static int firstout = 1;		/* non-zero until first output */
    static int recursive = 0;

#ifdef __STDC__
    va_start(ap, fmt);
#else
    va_start(ap);
    fmt = va_arg(ap,char *);
#endif

    if (firstout) {
	cg_begfile();			/* do beginning of file stuff */
	firstout = 0;
	db_begfile();
    }

    ++recursive;			/* mark a recursive call */

    /* Walk the format string, decide what to do. */
    for (;;) {
	char * s;
	int i;
	switch( *fmt++ ){
	case 0:
	    /* End of string. */
	    va_end(ap);
	    --recursive;		/* back out one level */
	    return;
	case 'b':
	    s = va_arg(ap, char *);

#ifdef	DB_USEBYTE
	{
	    /* Output string a a set of bytes, with preceding length. */
	    int charno;

	    if (s1debug)
		db_out("C", s);

	    for (charno = 0; ; ++charno) {
		if ((charno % 8) == 0) {
		    if (charno != 0)
			putc('\n', dbout);
		    fprintf(dbout, "\t%s\t", DB_BYTE);
		}
		else
		    putc(',', dbout);
		fprintf(dbout, "%#x", s[charno]);
		if (s[charno] == 0)
		    break;		/* quit after seeing NUL */
	    }
	    putc('\n', dbout);
	}
#else
	    /* Output string as .string. */
	    fprintf(dbout, "\t%s\t\"%s\"\n", DB_DOTSTRING, s);
#endif
	    break;
	case 'd':
	    i = va_arg(ap, int);
	    fprintf(dbout, "%d", i);
	    break;
	case 'C':
	    /* Output comment string if -1s is set.  Output newline always. */
	    s = va_arg(ap, char *);
	    if (s1debug)
		fprintf(dbout, " %s %s", COMMENTSTR, s);
	    putc('\n', dbout);
	    break;
	case 's':
	    s = va_arg(ap, char *);
	    fputs(s, dbout);
	    break;
	case 'x':
	    i = va_arg(ap, int);
	    fprintf(dbout, "%#x", i);
	    break;
	case '\'':
	    /* quote character */
	    ++fmt;			/* skip to (past) next char, print it */
	    /*FALLTHRU*/
	/* literal chars */
	case ' ':
	case '\t':
	case '\n':
	case ':':
	case ';':
	case '-':
	    putc(fmt[-1], dbout);
	    break;
	case 'B':
	    fputs(DB_BYTE, dbout);
	    break;
	case 'H':
	    fputs(DB_2BYTE, dbout);
	    break;
	case 'W':
	    fputs(DB_4BYTE, dbout);
	    break;
	case 'L':
	    /* labels */
	    switch( *fmt++ ){
	    case 'c':			/* current entry */
		i = DB_CURINFOLAB;
		s = LAB_DEBUG;
		break;
	    case 'C':			/* end label, current entry */
		i = DB_CURINFOLAB;
		s = LAB_LAST;
		break;
	    case 'd':			/* debug label */
		i = va_arg(ap, int);
		s = LAB_DEBUG;
		break;
	    case 'D':			/* end label, debug label */
		i = va_arg(ap,int);
		s = LAB_LAST;
		break;
	    case 'l':			/* low pc */
		i = DB_CURINFOLAB;
		s = LAB_PCLOW;
		break;
	    case 'h':			/* high pc */
		i = DB_CURINFOLAB;
		s = LAB_PCHIGH;
		break;
	    case 'n':			/* line number label */
		i = va_arg(ap, int);
		s = LAB_LINENO;
		break;
	    case 'f':			/* function start label */
		i = va_arg(ap, int);
		s = LAB_FUNCNO;
		break;
	    case 't':			/* type label */
		i = va_arg(ap, TY_TYPEID);
		s = LAB_TYPE;
		break;
	    case 'B':			/* beginning of text label */
		s = LAB_TEXTBEGIN;
		break;
	    case 'E':
		s = LAB_TEXTEND;	/* end of text label */
		break;
	    case 'b':			/* beginning of line numbers */
		s = LAB_LINEBEGIN;
		break;
	    case 'e':
		s = LAB_LINEEND;	/* end of line numbers */
		break;
	    case 'L':			/* .text label */
		i = va_arg(ap, int);
		s = LABFMT;		/* use .text label format */
		break;
	    default:
		fmt -= 2; goto badfmt;
	    }
	    fprintf(dbout, s, i);
	    break;
	case '.':
	    /* Output label-setting directive. */
	    switch( *fmt++ ){
	    case 'd':			/* selected debug label */
		i = va_arg(ap, int);
		db_out("Ld:\n", i);
		break;
	    case 'D':
		i = va_arg(ap, int);	/* end label for selected label */
		db_out("LD:\n", i);
		break;
	    case 'C':
		i = DB_CURINFOLAB;	/* end label for current entry */
		db_out("LD:\n", i);
		break;
	    case 'l':			/* lo pc */
		db_out("Ll:\n");
		break;
	    case 'h':			/* hi pc */
		db_out("Lh:\n");
		break;
	    case 'B':			/* begin of text */
		db_out("LB:\n");
		break;
	    case 'E':			/* end of text */
		db_out("LE:\n");
		break;
	    case 'b':			/* begin of line numbers */
		db_out("Lb:\n");
		break;
	    case 'e':			/* end of line numbers */
		db_out("Le:\n");
		break;
	    case 'n':			/* line number label */
		i = va_arg(ap, int);
		db_out("Ln:\n", i);
		break;
	    case 'f':			/* function start label */
		i = va_arg(ap, int);
		db_out("Lf:\n", i);
		break;
	    default:
		fmt -= 2; goto badfmt;
	    }
	    break;
	case '=':
	    i = va_arg(ap, TY_TYPEID);
	    db_out("\ts Lt,Ld\n", DB_DOTSET, i, DB_CURINFOLAB);
	    break;
	case 'S':
	    /* Set location counter to section. */
	    switch( *fmt++ ){
	    case 'd':
		fprintf(dbout, "\t.section\t%s\n", DB_DOTDEBUG);
		break;
	    case 'l':
		fprintf(dbout, "\t.section\t%s\n", DB_DOTLINE);
		break;
	    case 'p':
		fputs(DB_DOTPREVIOUS, dbout);
		break;
	    /* Initialize sections with all their attributes. */
	    /* Right now, there are no attributes for these sections. */
	    case 'I':
		fprintf(dbout, "\t.section\t%s\n", DB_DOTDEBUG);
		fputs(DB_DOTPREVIOUS, dbout);
		if (db_linelevel == DB_LEVEL2) {
		    fprintf(dbout, "\t.section\t%s\n", DB_DOTLINE);
		    fputs(DB_DOTPREVIOUS, dbout);
		}
		break;
	    default:
		fmt -= 2; goto badfmt;
	    }
	    break;
	default:
	    fmt -= 1;
	    if (recursive) {		/* treat as literal if recursive call */
		putc(*fmt++, dbout);
		break;
	    }
badfmt: ;
	    cerror("db_out:  ??fmt:  %s", fmt);
	}
    }
    /*NOTREACHED*/
}

static int
db_ty_chkout(t)
T1WORD t;
/* Return non-zero if type t has been output as a user-defined type, else 0. */
{
    TY_TYPEID id = TY_MAPID(t);

    if (id >= db_maxtype)
	return( 0 );			/* beyond known range */
    return (TD_ELEM(td_dbtypes,db_typebits,id/8)[DB_OUTBIT] & db_tymask[id%8]);
}

static void
db_ty_setout(t)
T1WORD t;
/* Mark type t as having been output as a user-defined type. */
{
    TY_TYPEID id = TY_MAPID(t);
    int need = id/8 + 1;		/* number of bytes needed:  type 8 is
					** in the 2nd byte
					*/

    TD_USED(td_dbtypes) = need;
    if (need > td_dbtypes.td_allo)
	td_enlarge(&td_dbtypes, need);
    TD_CHKMAX(td_dbtypes);
    db_maxtype = 8 * td_dbtypes.td_allo;

    TD_ELEM(td_dbtypes,db_typebits,id/8)[DB_OUTBIT] |= db_tymask[id%8];
    return;
}


static int
db_ty_chkneed(t)
T1WORD t;
/* Return non-zero if type t needs to be output as a user-defined type,
** else 0.
*/
{
    TY_TYPEID id = TY_MAPID(t);

    if (id > db_maxneedtype)
	return( 0 );			/* beyond known range */
    return (
	TD_ELEM(td_dbtypes,db_typebits,id/8)[DB_NEEDBIT] & db_tymask[id%8]
	);
}

static void
db_ty_setneed(t)
T1WORD t;
/* Mark type t as needing to be output as a user-defined type. */
{
    TY_TYPEID id = TY_MAPID(t);
    int need = id/8 + 1;		/* number of bytes needed:  type 8 is
					** in the 2nd byte
					*/

    TD_USED(td_dbtypes) = need;
    if (need > td_dbtypes.td_allo)
	td_enlarge(&td_dbtypes, need);
    TD_CHKMAX(td_dbtypes);
    db_maxtype = 8 * td_dbtypes.td_allo;

    TD_ELEM(td_dbtypes,db_typebits,id/8)[DB_NEEDBIT] |= db_tymask[id%8];
    if (id > db_maxneedtype)		/* update maximum needed type */
	db_maxneedtype = id;
    return;
}


static void
db_push()
/* Start new level on debug stack.  New sibling label number is 0. */
{
    ++TD_USED(td_dbstack);		/* want to use another element */
    TD_NEED1(td_dbstack);
    TD_CHKMAX(td_dbstack);
    DB_CURSIBLING = 0;			/* no sibling yet */
    return;
}

static void
db_pop()
/* Exit current debug stack level.  Tie off any dangling sibling
** pointer.
*/
{
    db_out("Sd");
    if (DB_CURSIBLING)
	db_out(".d", DB_CURSIBLING);

    /* Output a null entry to tie off any non-existent children for
    ** this entry.  The phony entry has no content other than the length.
    */
    db_out("\tW\td\nSp", 4);
    if (--TD_USED(td_dbstack) < 0)
	cerror("debug stack underflow");
    return;
}


static void
db_s_entry(i)
int i;
/* Start new debug information entry with tag i.  Set up the current
** stack appropriately.  Sets output for debug section.
*/
{
    int sibling;
    char *tagname;

    if (s1debug) {
	switch(i) {
	case TAG_array_type:		tagname = "TAG_array_type"; break;
	case TAG_entry_point:		tagname = "TAG_entry_point"; break;
	case TAG_enumeration_type:	tagname = "TAG_enumeration_type"; break;
	case TAG_formal_parameter:	tagname = "TAG_formal_parameter"; break;
	case TAG_global_subroutine:	tagname = "TAG_global_subroutine"; break;
	case TAG_global_variable:	tagname = "TAG_global_variable"; break;
	case TAG_label:			tagname = "TAG_label"; break;
	case TAG_lexical_block:		tagname = "TAG_lexical_block"; break;
	case TAG_local_variable:	tagname = "TAG_local_variable"; break;
	case TAG_member:		tagname = "TAG_member"; break;
	case TAG_pointer_type:		tagname = "TAG_pointer_type"; break;
	case TAG_source_file:		tagname = "TAG_source_file"; break;
	case TAG_structure_type:	tagname = "TAG_structure_type"; break;
	case TAG_subroutine:		tagname = "TAG_subroutine"; break;
	case TAG_subroutine_type:	tagname = "TAG_subroutine_type"; break;
	case TAG_typedef:		tagname = "TAG_typedef"; break;
	case TAG_union_type:		tagname = "TAG_union_type"; break;
	case TAG_unspecified_parameters: tagname = "TAG_unspecified_parameters"; break;
	default:
	    tagname = "---unknown tag type ---"; break;
	}
    }
    else
	tagname = "";

    /* BEWARE!!
    ** There's a tricky recursion problem here.  The very first call
    ** to db_out triggers a call to db_begfile(), which, in turn, calls
    ** db_s_entry().  We have to keep the current/sibling information
    ** straight in case of such a call.  Therefore, force a slightly
    ** early call to db_out() just to be sure.
    */
    db_out("Sd");

    if((sibling = DB_CURSIBLING) == 0)
	sibling = DB_GENLAB();
    DB_CURINFOLAB = sibling;
    DB_CURSIBLING = DB_GENLAB();

    db_out(".d", sibling);
    db_out("\tW\tLC-Lc\n");
    db_out("\tH\txC", i, tagname);
    /* Output sibling attribute. */
    db_out("\tH\tx; W\tLdC", AT_sibling, DB_CURSIBLING, "AT_sibling");
    return;
}


static void
db_e_entry()
/* End debug information entry.  Generate end label, reset section. */
{
    db_out(".C");
    db_out("Sp");
    return;
}


static void
db_at_name(s)
char * s;
/* Output name attribute.  It may be null for certain error cases where
** a s/u/e is declared in a function prototype:  the tag and members go
** out of scope, but the type that refers to them lingers on.
*/
{
    if (s == 0)
	s = "";
    db_out("\tH\txC", AT_name, "AT_name");
    db_out("b", s);			/* does length, bytes */
    return;
}

static void
db_at_byte_size(size)
BITOFF size;
/* Output the size attribute for an object whose size is
** "size" bits.
*/
{
    db_out("\tH\tx; W\tdC", AT_byte_size, size/SZCHAR, "AT_byte_size");
    return;
}


static void
db_at_element_list(t)
T1WORD t;
/* Generate attribute for names and values of enumerators. */
{
    unsigned int mbrno;
    SX mbr;
    int lab = DB_GENLAB();

    /* Generate attribute, length, label for stuff. */
    db_out("\tH\txC\tH\tLD-Ld\n.d", AT_element_list, "AT_element_list",
	lab, lab, lab);
    
    /* Generate value, then string, for each enumerator. */
    for (mbrno = 0; (mbr = TY_G_MBR(t, mbrno)) != SY_NOSYM; ++mbrno)
	db_out("\tW\td\nb", SY_OFFSET(mbr), SY_NAME(mbr));

    /* Generate end label */
    db_out(".D", lab);
    return;
}


static void
db_at_pc()
/* Generate low and hi pc attributes.  The labels are always
** based on the current information entry.
*/
{
    db_out("\tH\tx; W LlC\tH\tx; W LhC",
		AT_low_pc, "AT_low_pc", AT_high_pc, "AT_high_pc");
    return;
}


static void
db_at_type(t)
T1WORD t;
/* Output type attribute.  By this time all sub-types that
** would result in a new debug info entry being generated
** must already have been flushed.
**
** We have to figure out which case we're dealing with, first:
** fundamental type/user-defined type, modified or not.
*/
{
    int elftype = FT_none;
    int usertype = 0;			/* user-defined type if non-0 */
    int nptrs = 0;			/* number of pointer modifiers */
    int attr;
    char * attrstring;
    char * ts;				/* name of fundamental type */

    /* Stop if we find a type already output.  Pointer to void is
    ** special:  a generic pointer.
    */
    for (;;) {
	t = TY_UNQUAL(t);		/* strip qualifiers, which the
					** debugger information does not
					** represent
					*/
	if (DB_TY_CHKOUT(t)) {
	    usertype = 1;		/* is user-defined type */
	    break;
	}
	else if (TY_ISPTR(t)) {
	    t = TY_DECREF(t);
	    if (TY_TYPE(t) == TY_VOID) {
		elftype = FT_pointer;
		ts = "void *";
		break;
	    }
	    ++nptrs;
	}
	else
	    break;
    }

    if (!usertype && elftype == FT_none) {
	switch( TY_TYPE(t) ){
	case TY_CHAR:		elftype = FT_char; ts = "char"; break;
	case TY_UCHAR:		elftype = FT_unsigned_char; ts = "uchar"; break;
	case TY_SCHAR:		elftype = FT_signed_char; ts = "schar"; break;

	case TY_SHORT:		elftype = FT_short; ts = "short"; break;
	case TY_USHORT:		elftype = FT_unsigned_short; ts = "ushort"; break;
	case TY_SSHORT:		elftype = FT_signed_short; ts = "sshort"; break;

	case TY_INT:		elftype = FT_integer; ts = "int"; break;
	case TY_UINT:		elftype = FT_unsigned_integer; ts = "uint"; break;
	case TY_SINT:		elftype = FT_signed_integer; ts = "sint"; break;

	case TY_LONG:		elftype = FT_long; ts = "long"; break;
	case TY_ULONG:		elftype = FT_unsigned_long; ts = "ulong"; break;
	case TY_SLONG:		elftype = FT_signed_long; ts = "slong"; break;

	case TY_FLOAT:		elftype = FT_float; ts = "float"; break;
	case TY_DOUBLE:		elftype = FT_dbl_prec_float; ts = "double"; break;
	case TY_LDOUBLE:	elftype = FT_ext_prec_float; ts = "ldouble"; break;

	case TY_VOID:		elftype = FT_void; ts = "void"; break;

	/* For these, assume that the description of the type will be
	** generated eventually.  It will have a number related to the
	** type's ID, as usual.
	*/
	case TY_STRUCT:
	case TY_UNION:
	case TY_ENUM:
	    usertype = 1;
	    /* If the guts of the type haven't been output, add this
	    ** type to the list of incomplete types.
	    */
	    if (! DB_TY_CHKOUT(t))
		DB_TY_SETNEED(t);	/* will need this one later */
	    break;

	default:
	    cerror("db_at_type:  can't output type %ld", (unsigned long) t);
	}
    }
    if (nptrs) {
	if (usertype) {
	    attr = AT_mod_u_d_type; attrstring = "AT_mod_u_d_type";
	}
	else {
	    attr = AT_mod_fund_type; attrstring =  "AT_mod_fund_type";
	}
    }
    else {
	if (usertype) {
	    attr = AT_user_def_type; attrstring = "AT_user_def_type";
	}
	else {
	    attr = AT_fund_type; attrstring = "AT_fund_type";
	}
    }
    db_out("\tH\txC", attr, attrstring);
    if (nptrs) {
	/* Generate length of whole attribute, modifiers. */
	int i;

	db_out("\tH\td\n", nptrs + (usertype ? 4 : 2));
	for (i = nptrs; i > 0; --i)
	    db_out("\tB\tx\n", MOD_pointer_to);
    }
    if (usertype)
	db_out("\tW\tLt\n", TY_MAPID(t));
    else
	db_out("\tH\txC", elftype, ts);
    return;
}


static void
db_walktype(t)
T1WORD t;
/* Assume we're in a state where it's okay to write new debug info entries.
/* Walk the type structure t and flush all embedded types that have
** not been output.  Struct/union/enum's are emitted when they're seen.
*/
{
    /* Strip qualifiers, which the debug information does not represent. */
    t = TY_UNQUAL(t);

    /* Quit if stuff has already been generated for a type. */
    if (DB_TY_CHKOUT(t))
	return;

    switch( TY_TYPE(t) ){
    case TY_PTR:
    {
	T1WORD nextt = TY_DECREF(t);

	/* If generic pointer */
	if (TY_TYPE(nextt) == TY_VOID)	/* Generic pointer */
	    break;
	/* Walk the type from here.  We will allow pointer modifiers,
	** because if we're going to generate a pointer type now, we
	** can handle modifiers.
	*/
	db_walktype(nextt);		/* Keep going */
	break;
    }
    case TY_ARY:
    {
	T1WORD newt = t;

	do {
	    newt = TY_DECREF(newt);
	} while (TY_ISARY(newt));

	db_walktype(newt);		/* walk from non-array type */
	db_outarray(t, 1);		/* output information for this array */
	break;
    }
    case TY_FUN:
	db_outfunction(t);		/* output function type description */
	break;
    } /* end switch */
    return;
}


static void
db_outarray(t, first)
T1WORD t;
int first;
/* Output a debug information entry for an array.  "first" is
** non-zero for the first of a sequence of array types, 0 for
** the rest.
*/
{
    T1WORD arrayof = TY_DECREF(t);
    int dimlab;				/* label number for start of
					** dimensions
					*/
    SIZE nelem = TY_NELEM(t);		/* number of elements at this level */

    /* Subscript information looks like this for C:
    **		1	format (FMT_FT_C_C)
    **		2	type (int)
    **		4	low bound (0)
    **		4	high bound (number of elements - 1)
    **	       --
    **
    ** If the upper bound is unknown, the format is FMT_FT_C_X, with
    ** an empty high bound expression.
    **
    ** Following the subscript stuff comes FMT_ET, the element type,
    ** which is an embedded at_type.
    */

    if (first) {
	db_s_entry(TAG_array_type);
	db_out("=", TY_MAPID(t));
	DB_TY_SETOUT(t);
	db_out("\tH\tx; H\txC", AT_ordering, ORD_row_major,
			"AT_ordering:  row major");
	db_out("\tH\txC", AT_subscr_data, "AT_subscr_data");
	dimlab = DB_GENLAB();
	db_out("\tH\tLD-Ld\n", dimlab, dimlab); /* length of subscript data */
	db_out(".d", dimlab);		/* remember start of subscript data */
    }
    
    /* Output subscript data at current level. */
    if (nelem == TY_NULLDIM || nelem == TY_ERRDIM)
	db_out("\tB\tx; H x; W s; H sC",
		FMT_FT_C_X, FT_signed_integer, "0", "0", "no bound");
    else
	db_out("\tB\tx; H\tx; W\tsds\n",
		FMT_FT_C_C, FT_signed_integer, "0,", nelem, "-1");

    if (TY_ISARY(arrayof))
	db_outarray(arrayof, 0);
    else {
	db_out("\tB\txC", FMT_ET, "FMT_ET");	/* output element type */
	db_at_type(arrayof);
    }

    if (first) {
	db_out(".D", dimlab);		/* end of subscript data */
	db_e_entry();			/* end of array entry */
    }

    return;
}

static void
db_outmember(sid)
SX sid;
/* Output debug information for each member of a s/u/e. */
{
    BITOFF mbroff = SY_OFFSET(sid);
    T1WORD t = SY_TYPE(sid);

    db_s_entry(TAG_member);
    db_at_name(SY_NAME(sid));
    db_at_type(t);
    if (SY_FLAGS(sid) & SY_ISFIELD) {
	/* Extra stuff for bitfields. */
	BITOFF size;
	BITOFF offset;
	BITOFF align = TY_ALIGN(t);

	SY_FLDUPACK(sid, size, offset);

	/* Calculate offset of field relative to an object of its
	** type's alignment.  Then adjust "mbroff" to reflect such
	** an object.
	** If RTOLBYTES, the bit offset is of the right-most bit,
	** and we need the position of the left-most one, which is
	** displaced by sizeof(field) from the opposite end of the
	** storage unit.
	*/
	mbroff = (offset/align) * align;
	offset %= align;
#ifdef RTOLBYTES
	offset = (TY_SIZE(t) - size) - offset;
#endif
	db_at_byte_size(TY_SIZE(t));	/* in a unit of its type */
	/* Output field size, offset. */
	db_out("\tH\tx; W\tdC", AT_bit_size, size, "AT_bit_size");
	db_out("\tH\tx; H\tdC", AT_bit_offset, offset, "AT_bit_offset");
    }
    /* Output location attribute; address of start of s/u is implicit. */
    db_out("\tH\tx; H d; B x; W d; B xC",
		AT_location, 1+4+1, OP_CONST, mbroff/SZCHAR, OP_ADD,
		"AT_location: OP_CONST val OP_ADD");
    db_e_entry();
    return;
}


static void
db_outfunction(t)
T1WORD t;
/* Output debug information entry for a function-returning type. */
{
    T1WORD rett = TY_DECREF(t);		/* return type */


    db_walktype(rett);			/* output return sub-type stuff */
    db_s_entry(TAG_subroutine_type);
    db_out("=", TY_MAPID(t));		/* label this type */
    DB_TY_SETOUT(t);			/* mark type as being generated */
    if (TY_TYPE(rett) != TY_VOID)	/* type attribute, except for void */
	db_at_type(rett);		/* describe the returned type */
    db_e_entry();


    /* Walk the parameter types, output any lower level information there. */
    db_walktype(TY_DECREF(t));

    /* Debug info entries for formals are children of the subroutine
    ** entry.
    */
    db_push();

    if (TY_HASPROTO(t)) {
	int i;
	int nparams;

	
	for (nparams = TY_NPARAM(t), i = 0; i < nparams; ++i) {
	    T1WORD paramt = TY_PROPRM(t,i);

	    db_walktype(paramt);

	    db_s_entry(TAG_formal_parameter);
	    db_at_type(paramt);
	    db_e_entry();
	}

	/* Represent "...". */
	if (TY_ISVARARG(t)) {
	    db_s_entry(TAG_unspecified_parameters);
	    db_e_entry();
	}
    }
    else {
	/* Unknown parameter information looks like "...". */
	db_s_entry(TAG_unspecified_parameters);
	db_e_entry();
    }

    db_pop();
    return;
}


static void
db_begfile()
/* Generate initial debug information after the rest of the compiler
** has decided on .file and .ident pseudo-ops.
*/
{
    static int firsttime = 1;

    if (! firsttime || db_symlevel == DB_LEVEL0) /* only do this once */
	return;
    
    firsttime = 0;
    cg_setlocctr(PROG);			/* force to .text */
    /* Set beginning of text label, initialize sections. */
    db_out(".BSI");
    db_s_entry(TAG_source_file);
    db_at_name(db_filename);
    db_out("\tH\tx; W\txC", AT_language, LANG_ANSI_C_V1, "AT_language");
    db_out("\tH\tx; W\tLBC\tH\tx; W\tLEC",
		AT_low_pc, "AT_low_pc", AT_high_pc, "AT_high_pc");
    if (db_linelevel == DB_LEVEL2)
	db_out("\tH\tx; W\tLbC", AT_stmt_list, "AT_stmt_list");
    db_e_entry();
    /* Set up start of line number information. */
    if (db_linelevel == DB_LEVEL2)
	db_out("Sl.b\tW\tLe-Lb; W\tLB\nSp");

    /* file entry owns all the others */
    db_push();
    return;
}

void
db_s_file(s)
char * s;
/* Do what needs to be done for debug information at the start of a file. */
{
    db_filename = st_lookup(s);		/* remember filename for later */
    return;
}


void
db_e_file()
/* Do what needs to be done for debug information at the end of a file. */
{
    if (db_symlevel == DB_LEVEL0)
	return;

    db_begfile();			/* be sure we've started debugging */

    /* Finish off file entry.  Its sibling label marks end of .debug
    ** for this file.
    */
    db_pop();				/* ties off sibling of current block */

    /* Force out dummy entry to align to 4-byte boundary.  This is a hack
    ** to cope with assemblers that insist on aligning everything, whether
    ** asked to or not.
    */
    {
	int lab = DB_GENLAB();
	db_out("Sd.d\tW\tLD-Ld\n\ts\t4\n.DSp",
		lab, lab, lab, DB_DOTALIGN, lab);
    }

    db_out("Sd.dSp", DB_CURSIBLING);	/* "theoretical" end of .debug */

    cg_setlocctr(PROG);			/* force to .text */
    db_out(".E");			/* end of text label */
    /* Final line number (zero) with end-of-text.  Then lay down end
    ** of line numbers label.
    */
    if (db_linelevel == DB_LEVEL2)
	db_out("Sl\tW\t0; H x; W LE-LB\n.eSp", 0xffff);

    if (TD_USED(td_dbstack) != 0)
	cerror("debug stack not zero");
    return;
}



void
db_begf(sid)
SX sid;
/* Do debug stuff at start of function definition. */
{
    T1WORD functype = SY_TYPE(sid);
    T1WORD rettype = TY_DECREF(functype);
    int tag = SY_CLASS(sid) == SC_EXTERN ?
		TAG_global_subroutine : TAG_subroutine;

    if (db_symlevel == DB_LEVEL0)
	return;

    db_funcname = SY_NAME(sid);		/* remember name to put out label */
    db_walktype(rettype);		/* force out lower-level type info */

    /* Always generate a new entry */
    db_s_entry(tag);
    db_at_name(db_funcname);
    /* Generate type attribute if there's a return value. */
    if (TY_TYPE(rettype) != TY_VOID)
	db_at_type(rettype);
    /* Low pc for a function is the same as the function's label. */
    db_out("\tH\tx; W\tsC", AT_low_pc, SY_NAME(sid), "AT_low_pc");
    db_out("\tH\tx; W\tLhC", AT_high_pc, "AT_high_pc");
    db_e_entry();

    SY_FLAGS(sid) |= SY_DBOUT;		/* mark debug information generated */

    /* If debugging is enabled, descend a level to allow for other
    ** symbols within the function, particularly its formals.
    */
    if (db_symlevel == DB_LEVEL2)
	db_push();
    return;
}


void
db_s_fcode()
/* Output debug information after function prologue, before real
** function code if no debug information has been issued for this 
** line yet.
*/
{
    if (db_linelevel == DB_LEVEL2 && db_curline > db_lastlineno) {
	cg_setlocctr(PROG);		/* put line number labels in .text */
	/* Output:
	** <LAB_FUNCNO><line>:
	**	.section .line
	**	.uaword <line> 
	**	.uahalf 0xffff
	**	.uaword <label>-..text.begin
	**
	** where label is the function name.
	*/
	db_out(".f", db_curline);
	db_out("Sl\tW\td; H x; W s-LB\nSp", db_curline, 0xffff, db_funcname);
	db_lastlineno = db_curline;
    }
    return;
}


void
db_e_fcode()
/* Generate debug output at end of function, before function epilogue. */
{
    db_lineno();			/* put out line number for epilogue */
    return;
}


void
db_endf()
/* Generate debug output at end of entire function definition. */
{
    switch (db_symlevel){
    case DB_LEVEL2:
	db_pop();			/* unwind from push in db_begf() */
	/*FALLTHRU*/
    case DB_LEVEL1:
	cg_setlocctr(PROG);
	db_out(".h");			/* Generate high pc label in text */
	/*FALLTHRU*/
    case DB_LEVEL0:
	break;
    }
    return;
}



static void
db_outblock()
/* Generate code to represent a new lexical block. */
{
    db_s_entry(TAG_lexical_block);
    db_at_pc();
    db_e_entry();
    cg_setlocctr(PROG);			/* force a label in .text */
    db_out(".l");			/* low pc */
    db_push();				/* things in the block owned by this
				        ** entry
					*/
    BBFLAG = 1;				/* have output entry at this level */
    return;
}

void
db_s_block()
/* Starting a new block.  Make sure there is enough space in the
** block begin flags.
*/
{
    if (++bblevel >= td_bbflags.td_allo) {
	TD_NEED1(td_bbflags);
	TD_CHKMAX(td_bbflags);
    }
    BBFLAG = 0;				/* zero flag at current level */
    return;
}


void
db_e_block()
/* Exiting a block.  Output high pc label if we've done a lexical
** block entry for this block.  Drop the current block level.
*/
{
    if (db_symlevel == DB_LEVEL2 && BBFLAG) {
	db_pop();
	cg_setlocctr(PROG);		/* force label in text */
	db_out(".h");			/* high pc */
    }
    if (--bblevel < 0)
	cerror("db_e_block():  confused block level");
    return;
}


void
db_symbol(sid)
SX sid;
/* Produce debug information for sid, conditional on global flag. */
{
    if (db_symlevel == DB_LEVEL2 && (SY_FLAGS(sid) & SY_DBOUT) == 0)
	db_ucsymbol(sid);
    return;
}


static void
db_ucsymbol(sid)
SX sid;
/* Do unconditional symbol output for the symbol whose table
** entry is sid.
*/
{
    int tag;
    SY_CLASS_t class = SY_CLASS(sid);

    /* No debugging for asm functions. */
    if (class == SC_ASM)
	return;

    /* ... or if the symbol is external and is neither defined nor
    ** tentatively defined.
    */
    if (class == SC_EXTERN && (SY_FLAGS(sid) & (SY_DEFINED|SY_TENTATIVE)) == 0)
	return;

    switch( class ){
    case SC_AUTO:	tag = TAG_local_variable; break;
    case SC_PARAM:
    {
	char * s = SY_NAME(sid);

	/* Assume normal parameter, check for "...". */
	tag = TAG_formal_parameter;
	if (s[0] == '.' && s[1] == '.' && s[2] == '.' && s[3] == 0)
	    tag = TAG_unspecified_parameters;
	break;
    }
    case SC_EXTERN:	tag = TAG_global_variable; break;
    case SC_TYPEDEF:	tag = TAG_typedef; break;
    case SC_STATIC:	tag = TAG_local_variable; break;
    case SC_LABEL:	tag = TAG_label; break;
    default:
	cerror("db_ucsymbol():  bad class");
    }

    /* Force out a lexical block debug entry for first declaration in a
    ** nested block, not including user-defined labels.
    */
    if (BBFLAG == 0 && SY_LEVEL(sid) > SL_INFUNC && tag != TAG_label)
       db_outblock();			/* force out code */

    db_walktype(SY_TYPE(sid));		/* walk type structure */
    /* Start new entry. */
    db_s_entry(tag);
    db_at_name(SY_NAME(sid));

    /* Do location descriptors. */
    switch (class) {
    case SC_EXTERN:
    case SC_STATIC:
    {
	db_at_type(SY_TYPE(sid));
	db_out("\tH\txC", AT_location, "AT_location");
	db_out("\tH\td; B x; W sC", 1+4, OP_ADDR, cg_extname(sid), "OP_ADDR");
	break;
    }
    case SC_AUTO:
    case SC_PARAM:
    {
	db_at_type(SY_TYPE(sid));
	db_out("\tH\txC", AT_location, "AT_location");
	if (SY_REGNO(sid) == SY_NOREG) {
	    db_out("\tH\td; B x; W d; B x; W d; B xC",
		1+4+1+4+1,
		OP_BASEREG, class == SC_AUTO ? DB_FRAMEPTR(sid) : DB_ARGPTR(sid),
		OP_CONST, SY_OFFSET(sid),
		OP_ADD,
		"OP_BASEREG OP_CONST OP_ADD"
	      );
	}
	else
	    db_out("\tH\td; B x; W dC",
		1+4, OP_REG, DB_OUTREGNO(sid), "OP_REG");
	break;
    }
    case SC_LABEL:
	/* low_pc attribute refers to location in .text, which is
	** label number in SY_OFFSET.
	*/
	db_out("\tH\tx; W LLC", AT_low_pc, SY_OFFSET(sid), "AT_low_pc");
	break;
    case SC_TYPEDEF:
	/* Need type attribute. */
	db_at_type(SY_TYPE(sid));
	break;
    }
    db_e_entry();
    SY_FLAGS(sid) |= SY_DBOUT;

    return;
}


void
db_lineno()
/* Output the current line, relative to the beginning of the
** current function.
*/
{
    if (db_linelevel == DB_LEVEL2 && db_curline > db_lastlineno) {
	db_uclineno(db_curline);
	db_lastlineno = db_curline;
    }
    return;
}

static void
db_uclineno(ln)
int ln;
/* Unconditional output line number ln. */
{
    cg_setlocctr(PROG);			/* put line number labels in .text */
    /* Output:
    **	label
    **	.section .line
    **	.uaword <line number>
    **	.uahalf 0xffff
    **	.uaword <label>-..text.begin
    */
    db_out(".nSl\tW\td; H x; W Ln-LB\nSp", ln, ln, 0xffff, ln);
    return;
}


void
db_sue(t)
T1WORD t;
/* Output member information for s/u/e t after doing so for all
** recursively referenced s/u/e's.
*/
{
    int tagno;
    unsigned int mbrno;
    SX tagsid;
    SX mbr;

    /* Suppress output if no members in list, or if full debugging
    ** output is disable.
    */
    if (db_symlevel != DB_LEVEL2 || ! TY_HASLIST(t))
	return;

    tagsid = TY_SUETAG(t);		/* get symbol ID for tag */

    /* Mark type as having been output.  We'll be doing so shortly.
    ** This takes care of potential recursion in the structure.
    */
    DB_TY_SETOUT(t);

    SY_FLAGS(tagsid) |= SY_DBOUT;	/* debugging will have been produced */

    /* Walk the type of each member. */
    for (mbrno = 0; (mbr = TY_G_MBR(t,mbrno)) != SY_NOSYM; ++mbrno)
	db_walktype(SY_TYPE(mbr));

    /* Begin entry. */
    switch(TY_TYPE(t)){
    case TY_STRUCT:	tagno = TAG_structure_type; break;
    case TY_UNION:	tagno = TAG_union_type; break;
    case TY_ENUM:	tagno = TAG_enumeration_type; break;
    }
    db_s_entry(tagno);
    db_out("=", TY_MAPID(t));
    db_at_name(SY_NAME(tagsid));
    db_at_byte_size(TY_SIZE(t));
    if (tagno == TAG_enumeration_type)
	db_at_element_list(t);	/* enumerators are an attribute */
    db_e_entry();
    db_push();			/* new level for members */
    /* Output stuff for each s/u member. */
    if (tagno != TAG_enumeration_type) {
	for (mbrno = 0; (mbr = TY_G_MBR(t,mbrno)) != SY_NOSYM; ++mbrno)
	    db_outmember(mbr);
    }
    db_pop();

    return;
}

void
db_sy_clear(sid)
SX sid;
/* Output debug information for symbol table entries that are being
** flushed.  This is the way we pick up s/u/e tags for which no
** information has been generated.
*/
{

    switch( SY_CLASS(sid) ) {
	T1WORD t;
    case SC_STRUCT:
    case SC_UNION:
    case SC_ENUM:
	if (DB_TY_CHKOUT((t = SY_TYPE(sid))))
	    break;			/* type information already generated */
	if (DB_TY_CHKNEED(t)) {		/* need to generate output for it? */

	    int tagno;

	    switch(TY_TYPE(t)){
	    case TY_STRUCT:	tagno = TAG_structure_type; break;
	    case TY_UNION:	tagno = TAG_union_type; break;
	    case TY_ENUM:	tagno = TAG_enumeration_type; break;
	    }
	    db_s_entry(tagno);
	    db_out("=", TY_MAPID(t));
	    db_at_name(SY_NAME(sid));
	    db_e_entry();

	    DB_TY_SETOUT(t);		/* mark type as output */
	}
	break;
    }
    return;
}

#endif /* def ELF_OBJ */

#endif /* ndef LINT */
