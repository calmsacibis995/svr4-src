/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)nifg:cg/common/sty.y	1.11"
/*
	This program turns machine descriptions into a table.c file
*/
%}
/*	The input language uses many C conventions and notations */
/*	Types are represented as a shorthand:
	c	char
	i	int
	s	short
	l	long
	p	pointer
	P	alternate form of pointer
	F	frame pointer (CG only)
	t	structure
	v	void
	ux	unsigned x
	f	float
	d	double
/*	There are a number of builtin cost names:
	NONAME		a constant with no name field (not an address)
	CONVAL n	the constant n
	NCONVAL n	the constant -n
	POSRANGE n	constants in the range [0,2**n -1]
	SGRANGE n	constants in the range [-2**n, 2**n - 1]
	*/
/*	There are also several incidental needs, etc, specified
	1,2,3	Number of needed registers
	$P	need pairs
	$<	share left
	$>	share right
	$L	result left
	$R	result right
	$E	result in left or right, whichever is "better"
	$1,2,3	result in reg 1, 2, 3
	$C	operation produces correct condition codes
	$N	no value produced: side effects only
	$A	need them all
	$[	share left, LHS is preferred (if a temp register)
	$]	share right, RHS is preferred (if a temp register)
	$l	left side is referenced once more
	$r	right side is referenced once more
	$H	can honor exceptions
	$I	can ignore exceptions
	!H	cannot honor exceptions
	!I	cannot ignore exceptions
	*/


%{
/* This is a STINCC/REGSET version of sty.  Turn on STINCC to aid
** old makefiles.
*/
#ifndef	STINCC
#define STINCC				/* for mfile2 */
#endif
# include "ctype.h"
# include "mfile2.h"
# include <values.h>			/* to get BITSPERBYTE */
#define	STY				/* to change dope.h struct */
# include "dope.h"
#undef	STY
# include "string.h"

/* size of a pointer (in bits) on the machine running sty */
#define	HOSTPTRSIZE	(sizeof(char *) * BITSPERBYTE)

#ifndef	REGSET
typedef	int RST;			/* mostly to tie off references */
#define	RS_NRGS	0
#define	RS_TOT	0
#define	RS_BIT(n) 0
#define	RS_PAIR(n) 0
#define	RS_CHOOSE(n) 0
#endif

typedef struct {
	int	sha;	/* shape */
	int	ty;	/* type */
} SHTY;
SHTY	lshty,	/* left and right shape type */
	rshty;
static int	op,
		tyop,
		needs,
		rewrite,
		rewregs = -1;		/* number of REG shape containing
					** result reg bits for RESC[123] or -1
					*/
static int	opno;
static RST	pregset;		/* register set found during parsing */
static int	ophd[DSIZE];
static int	optl[DSIZE];
static unsigned long	shphd[DSIZE];	/* non-0 if corresponding OP can be root
					** of a shape tree; later converted to bit
					*/

struct optb {		/* operation table */
	int	op;	/* what operation is to be matched */
	int	tyop;	/* the type associated with the op node */
	int	nxop;	/* index of the next op */
	int	line;	/* stin file line number */
	SHTY	l,	/* shapes of the left side */
		r;	/*		right side */
	int	oneeds,	/* needs */
		orew,	/* rewrite info */
		rneeds;	/* shape number containing register set info for regs,
			** during parsing; offset in styrsbits of bit vectors
			** later
			*/
	char	*string;/* the output */
	int	strdef,	
		struse;
	int	isleaf,	/* 1 if template is a leaf template */
		moved;	/* 1 if template was moved by template ordering */
};
typedef struct optb OPTB;

# ifndef NOPTB
# define NOPTB 400
# endif

OPTB optb[NOPTB];

# ifndef NSTRING
# define NSTRING 10000
# endif

# ifndef NSTYSHPS
# define NSTYSHPS 4000
# endif

# ifndef NSTYPSHPS
# define NSTYPSHPS NSTYSHPS
#endif

# ifndef NWEIGHTS
# define NWEIGHTS NSTYSHPS/10
# endif

#ifndef	NTMPLTS
#define	NTMPLTS	100			/* max templates for an OP */
#endif

#ifdef	REGSET
#ifndef	NRMERGE
#define	NRMERGE	(TOTREGS * TOTREGS)	/* estimate of worst-case expansion */
#endif

#ifndef	NRSBITS
#define	NRSBITS	1000			/* for storing register set bit vectors */
#endif
#endif	/* def REGSET */

#ifndef	NRSSETS
#define	NRSSETS	100			/* number of unique register set bit
					** vector sets
					*/
#endif

#define	NSHNAME	8			/* size of name field */

typedef struct {
	int	sop,
		sleft,
		sright,
		ssh,
		scnt;
	char	shname[NSHNAME];
	RST	sregset;
} STYSHP;

static int	nshp = 0;
static int	nmshp = NSTYSHPS - 1;
/*static*/ STYSHP	shp[NSTYSHPS];
/*static*/ int	pshp[NSTYPSHPS]; /* lists of shapes corresponding to pshapes */
#ifndef	REGSET
/*static*/ int	pshpreg[NSTYSHPS]; /* number of REGs used by each of above */
#endif
static char	strng[NSTRING];
static char	*string;
static char	*pstring = strng;
static char	*asstring;
/* for statistics: */
static int	nstypshp;	/* number of pshp[] slots used */
static int	nweight;	/* number of weight slots used */
static int	maxtmplt;	/* number of template sorting slots used */
static int	nrmerge;	/* max. number of regsiter merge slots used */
static int	shphdbits;	/* number of shphd bits used */
static int	nrsbits = 0;	/* current number of entries in styrsbits[] */

/* predicate true on "wildcard" OPs */
#define	iswildcard(o)	(o == REG || o == CCODES || o == FREE)

extern void exit();
#ifdef CG
#define DEFAULT_NEEDS def_except
int def_except = 0;
#else
#define DEFAULT_NEEDS 0
#define NO_IGNORE 0
#define NO_HONOR 0
#endif

%}

%union {
	int	ival;
	char	*str;
	SHTY	shh;
	RST	regset;
};

%token STR DEF LPRF RPRF LSHR RSHR GOODCC NOVAL PNEED LRES RRES ERES
%token EXHON EXIG NOEXHON NOEXIG
%token LCOUNT RCOUNT
%token USERCOST CONVAL NCONVAL POSRANGE SGRANGE NONAME DCOST SHAPES OPCODES
%token EXCEPT
%token <ival> OP NM DIG DIG_ALL STYPE RREG
%left OP
%left STYPE

%type <str> STR
%type <ival> opcost num slist nterm nterm1 opop shape newshape
%type <ival> costnexp nexp cost cexpr cterm
%type <ival> regno opnregshape
%type <ival> excepts except
%type <regset> reglist regterm opregset
%type <shh> sref

	/*
	OP	the ops <, <<, >, >>, <=, >=, +, -, *, /, %, &, ^, ~, !
	NM	names (letter, followed by 0 or more letters or digits)
	STR	a string in ""
	DIG	a digit, 0-9
	Other letters are returned: (, ), {, }, etc.
	*/

%%

file	:	SHAPES lshapes OPCODES lops
		{	finished(); }
	;

lshapes	:	/* EMPTY */
	|	lshapes newshape
		{ if (bdebug) shprint( $2 ); }
	;

/* need to merge registers for register shape list only; for others,
** presume this was done for their constituents.
*/
newshape:	NM ':' slist ',' costnexp ';'
		{
#ifndef	REGSET
		    $$ = buildshape($1, MANY, $3, $5, 0);
#else
		    $$ = mergereg( buildshape($1, MANY, $3, $5, 0) );
#endif
		}

	|	NM ':' costnexp ';'
		{ $$ = buildshape($1, MANY, $3, -1, 0); }

	|	NM ':' opop shape opcost ';'
		{
		    /* get old or new shape */
		    int shape = fillshape($3, -1, -1, $4, pregset);

		    $$ = buildshape($1, MANY, shape, -1, 0);
		}
	;

opop	:	/* EMPTY */
		{	$$ = ICON; }
	|	OP opregset
		{
		    pregset = $2;		/* assume register set */
		    if ($1 == REG && pregset == 0)
			/* default if no register bits is all registers */
			pregset = RS_TOT;
		    else if ($1 != REG && pregset != 0)
			yyerror("register set specified for non-REG shape");
		}
	;

opregset:	/* EMPTY */
		{ $$ = 0; }		/* indicate no register info */
	|	'{' reglist '}'
		{
		    $$ = $2;
#ifndef	REGSET
		    yyerror("REGSET not enabled");
#endif
		}
	;

reglist	:	regterm
	|	reglist ',' regterm
		{ $$ = $1 | $3; }
	;

regterm	:	regno
		{ $$ = RS_BIT( $1 ); }
	|	regno OP regno
		/* this is embarrassingly awkward:  accept any OP, but
		** restrict the "acceptable" ones to MINUS.  It's easier
		** to do this than break out MINUS as a special case from
		** yylex().
		*/
		{
		    int temp;

		    if ($2 != MINUS)
			yyerror("syntax error");

		    if ($1 > $3) {
			yyerror("range out of order");
			temp = $1;
			$1 = $3;
			$3 = temp;
		    }

		    /* figure bit vector in one fell swoop! */

		    $$ = (RS_BIT($3+1)-1) - (RS_BIT($1)-1);
		}
	;

regno	:	num
		{
#ifdef	REGSET		/* TOTREGS not required in non-REGSET */
		    if ($1 < 0 || $1 >= TOTREGS) {
			yyerror("register number out of range");
			$$ = 0;
		    }
		    else
#endif
			$$ = $1;
		}
	;

lops	:	/* EMPTY */
		{	op = rewrite = 0;
			needs = DEFAULT_NEEDS; }
	|	lops lop
	;

lop	:	OP sref ',' sref ltail
		{	lshty = $2;
			rshty = $4;
			op = $1;
			tyop = TANY;
			dotmplt(0);
			tyop = op = rewrite = 0;
			needs = DEFAULT_NEEDS;
			rewregs = -1;
		}
	|	OP STYPE sref ',' sref ltail
		{	lshty = $3;
			rshty = $5;
			op = $1;
			tyop = $2;
			dotmplt(0);
			tyop = op = rewrite = 0;
			needs = DEFAULT_NEEDS;
			rewregs = -1;
		}
	|	OP sref ltail
		{	lshty = $2;
			rshty.sha = -1;
			rshty.ty = TANY;
			op = $1;
			tyop = TANY;
			dotmplt(0);
			tyop = op = rewrite = 0;
			needs = DEFAULT_NEEDS;
			rewregs = -1;
		}
	|	OP STYPE sref ltail
		{	lshty = $3;
			rshty.sha = -1;
			tyop = rshty.ty = $2;
			op = $1;
			dotmplt(0);
			tyop = op = rewrite = 0;
			needs = DEFAULT_NEEDS;
			rewregs = -1;
		}
	|	sref ltail
		{	rshty = $1;
			lshty.sha = -1;
			lshty.ty = TANY;
			doltmplt();
			tyop = op = rewrite = 0;
			needs = DEFAULT_NEEDS;
			rewregs = -1;
			}
	|	DCOST opcost ';'
	|	EXCEPT ':' excepts
		{
#ifdef CG
		def_except = $3;
#endif
		}
	;

excepts	:	/* EMPTY*/
		{
		$$ = 0;
		}
	|	excepts except
		{
		$$ = $1 | $2;
		}
	;
except	:	EXHON
		{ $$ = 0; }
	|	NOEXHON
		{ $$ = NO_HONOR; }
	|	EXIG
		{ $$ = 0; }
	|	NOEXIG
		{ $$ = NO_IGNORE; }
	;

/* the code to handle strings is really yuccky!  However, it makes
** it easier to concatenate strings, which are all accumulated in
** a static buffer.  yylex() does NOT null-terminate the string.
** That's left to the code that notes the end of a string.
**
** Remember line number before needs:  usually the OP, shapes, and needs
** all appear on the first line of a template, and that's the line
** number we'd like in the template for debug output.
*/

ltail	:	{ firstline=lineno; begnrgen(); }
		needs string { *pstring++ = '\0'; } opcost ';'
	;

string	:	STR
		{ asstring = $1; }
	|	string STR		/* concatenation for free; see above */
	;

needs	:	/* EMPTY */
		{
			needs = DEFAULT_NEEDS;
		}
	|	'{' nlist '}'
	;

nlist	:	/* EMPTY */
		{	needs = DEFAULT_NEEDS; }
	|	nlist DIG opnregshape
		{
#ifdef	REGSET
			static void nrgen();

			/* record register set needs */
			nrgen($3, $2/NREG, 0);	/* not result */
			if (rewregs < 0) rewregs = $3;
#else
			needs = (needs&~NCOUNT) | $2;
#endif
		}
	|	nlist DIG_ALL
		{
#ifdef	REGSET
			static void nrgen();

			nrgen(-1, -1, 0);	/* signal $A */
#endif
			needs = (needs & ~NCOUNT) | $2;

		}
	|	nlist RPRF
		{	needs |= RSHARE|RPREF; }
	|	nlist LPRF
		{	needs |= LSHARE|LPREF; }
	|	nlist RSHR
		{	needs |= RSHARE; }
	|	nlist LSHR
		{	needs |= LSHARE; }
	|	nlist PNEED
		{	needs |= NPAIR; }
	|	nlist EXHON
		{	needs &= ~NO_HONOR; }
	|	nlist NOEXHON
		{	needs |= NO_HONOR; }
	|	nlist EXIG
		{	needs &= ~NO_IGNORE; }
	|	nlist NOEXIG
		{	needs |= NO_IGNORE; }
	|	nlist GOODCC
		{	rewrite |= RESCC; }
	|	nlist NOVAL
		{
			if (rewrite)
				yyerror("$N incompatible with other results");
			rewrite = RNULL;
		}
	|	nlist LRES
		{	rewrite |= RLEFT; }
	|	nlist RRES
		{	rewrite |= RRIGHT; }
	|	nlist ERES
		{	rewrite |= REITHER; }
	|	nlist RREG opnregshape
		{
			static void nrgen();

#ifndef	REGSET		/* nrgen() does this in REGSET */
			if( !(needs&NCOUNT) ) needs |= $2;
#endif
			/* track register set needs */
			nrgen($3, $2/NREG, 1);
			if (rewregs < 0) rewregs = $3;
#ifndef	REGSET		/* nrgen() does this for REGSET */
			rewrite |= (($2==1)?RESC1:(($2==2)?RESC2:RESC3));
#endif
			}
	|	nlist LCOUNT
	|	nlist RCOUNT
	;

opnregshape :	/* EMPTY */
		{ $$ = -1; }
	|	':' NM
		{
		    $$ = chknreg( $2 );
#ifndef	REGSET
		    yyerror("REGSET not enabled");
#endif
		}
	;

num	:	DIG
	|	num DIG
		{	$$ = 10*$1 + $2; }
	;

opcost	:	cost
	|	/* EMPTY */
		{ $$ = 0; }
	;

cost	:	':' cexpr
		{	$$ = $2; }
	;

cexpr	:	cterm
	|	OP cterm
		{	$$ = 0; }
	|	cterm OP cterm
		{	$$ = 0; }
	;

cterm	:	'(' cexpr ')'
		{	$$ = $2; }
	|	num
	;

shape	:	/* EMPTY */
		{	$$ = 0; }
	|	NONAME
		{	$$ = NACON;	/* constant with no name */ }
	|	USERCOST num
		{	$$ = $2|SUSER;  /* user's cost ftn */ }
	|	CONVAL num
		{	$$ = $2|SVAL;  /* positive constant value */ }
	|	NCONVAL num
		{	$$ = $2|SNVAL;  /* negative constant value */ }
	|	POSRANGE num
		{	$$ = $2|SRANGE0;  /* positive range */ }
	|	SGRANGE num
		{	$$ = $2|SSRANGE;  /* signed range */ }
	;

sref	:	nterm1
		{	$$.ty = 0;
			$$.sha = $1;
			}
	|	nterm1 STYPE     /* do this before doing in nterm */
		{	$$.sha = $1;
			$$.ty = $2; }
	;

slist	:	costnexp
	|	slist ',' costnexp
		{	$$ = bop( MANY, $1, $3 ); }
	;

costnexp:	nexp opregset opcost
		{
		    int s;

		    /* special case for alias for 'REG':  make new
		    ** register-set node if needed
		    */
		    if ($2 != 0) {
			if (
			       shp[$1].sop == MANY
			    && shp[$1].sright < 0
			    && (s = shp[$1].sleft) >= 0
			    && shp[s].sop == REG
			    ) {
				/* if current REG node is "all regs", use existing
				** version of a register-set specified node, if it
				** exists, or make one.
				*/
				if (shp[s].sregset == RS_TOT)
				    $$ = fillshape(REG, -1, -1, shp[s].ssh, $2);
				else
				    yyerror("can't have overriding register set");
			}
			else
			    yyerror("register set on non-REG shape");
		    }
		}
	;
	
nexp	:	nterm
	|	OP nterm
		{	$$ = uop( $1, $2 ); }
	|	nterm OP nterm
		{	$$ = bop( $2, $1, $3 ); }
	;

nterm	:	nterm1
	|	nterm1 STYPE
		{	$$ = top( $1, $2 );  }
	;

nterm1	:	'(' nexp ')'
		{	$$ = $2; }
	|	NM
	;

%%
/* "static" removed from definitions for lineno, odebug, rdebug, sdebug
** because of conflicts with mfile2.h .
*/
       int	lineno = 1;	/* line number of stin file */
static int	firstline;	/* first line number of a template */
static char *	filename = "<stdin>";
static FILE * dbg = stdout;	/* dest. for debug output */
/* debug flags */
static int bdebug = 0;		/* print shape trees as they're built */
static int mdebug = 0;		/* print register set merge info */
       int odebug = 0;		/* print template order by OP */
       int rdebug = 0;		/* print register use information */
       int sdebug = 0;		/* print info about shared shapes */
static int tdebug = 0;		/* print info about template ordering */
static int wdebug = 0;		/* print shape weights in output file */
static int Ndebug = 0;		/* non-0 to suppress template ordering */
static int Sdebug = 0;		/* print table statistics */
static int dumpcore = 0;	/* if 1, dump core on error */
static char * dbgname = NULL;	/* name of file for debug output */

main(argc, argv)
int argc;
char **argv;
{
	register int i;
	/* for getopt */
	extern int optind;
	extern char * optarg;
	int option;

#define	OPTSTRING "bd:morstwCNS"

	while ((option = getopt(argc, argv, OPTSTRING)) != EOF) {
	    switch( option ) {
	    case 'b':	++bdebug; break;
	    case 'd':	dbgname = optarg; break; /* name of debug file */
	    case 'm':	++mdebug; break;
	    case 'o':	++odebug; break;
	    case 'r':	++rdebug; break;
	    case 's':	++sdebug; break;
	    case 't':	++tdebug; break;
	    case 'w':	++wdebug; break;
	    case 'C':	++dumpcore; break;
	    case 'N':	++Ndebug; break;
	    case 'S':	++Sdebug; break;
	    default:
		fprintf(stderr,
		"Usage:  sty [options] input output\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
		"	-b	show shapes as they're built",
		"	-d file	dump debug output to <file>",
		"	-o	output template order",
		"	-r	output register use information",
		"	-s	output shape sharing information",
		"	-t	output template ordering information",
		"	-w	print template weights in output file",
		"	-C	dump core on error",
		"	-N	suppress template ordering",
		"	-S	print internal table statistics"
		);
		exit( 1 );
	    }
	}
	/* handle debug file before changing stdout */
	if (dbgname == NULL) {		/* use current stdout, not future */
	    if (optind+1 < argc) {	/* do only if we WILL change stdout */
		int dup();
		int newfd = dup(fileno(stdout));

		if (newfd >= 0)
		    if ((dbg = fdopen(newfd, "w")) == NULL)
		    dbg = stdout;	/* nothing worked! */;
	    }
	}
	else {
	    if ((dbg = fopen(dbgname, "w")) == NULL) {
		fprintf(stderr, "Can't open debug file %s\n", dbgname);
		exit ( 1 );
	    }
	}
	/* pick up files, if any */
	if (optind < argc) {
	    filename = argv[optind];
	    if (freopen(filename, "r", stdin) == NULL) {
		fprintf(stderr, "Can't open input file %s\n", filename);
		exit( 1 );
	    }
	    ++optind;
	}

	if (optind < argc) {
	    if (freopen(argv[optind], "w", stdout) == NULL) {
		fprintf(stderr, "Can't open output file %s\n", argv[optind]);
		exit( 1 );
	    }
	    ++optind;
	}

	for( i=0; i<DSIZE; ++i ) {
		shphd[i] = 0;
		optl[i] = ophd[i] = -1;
	}
	chkRST();
	mkdope();
	yyparse();
	/* Write version string of sty into output file. */
	printf("#ident \"%s\"\n", "@(#)nifg:cg/common/sty.y	1.11");
	return( 0 );
}

/* for REGSET implementations, check whether type RST is big
** enough to hold all the bits for the registers specified.
*/

static
chkRST()
{
#ifdef	REGSET
    unsigned long testpat = RS_BIT(TOTREGS) + 5;

    if ((RST) testpat != testpat)
	yyerror("type RST is too small");

    /* seed register set bit array with initial zero so offset zero
    ** is zero.
    */

    nrsave( (RST) 0 );
#endif
    return;
}

checkit( n )
{
	/* check a shape */
	STYSHP *p;
	if( n<0 ) return;
	if( n>=nshp && n<=nmshp || n>=NSTYSHPS ) {
		yyerror( "out of range shape: %d", n );
	}
	p = &shp[n];
	/* sop < 0 is flag for undefined shape */
	if( p->sop < 0 ) {
	    yyerror( "undefined shape %.*s", NSHNAME, p->shname );
	}
	else if ( p->sop >= DSIZE ) {
		yyerror( "out of range op: %d", p->sop );
	}

	switch( optype(p->sop) ) {

	case BITYPE:
		if( p->sright < 0 && p->sop != MANY ) {
			yyerror( "right side empty" );
		}
		checkit( p->sright );
	case UTYPE:
		if( p->sleft < 0 ) {
			yyerror( "left side empty" );
		}
		checkit( p->sleft );
	}
}

/* check needs register set:  right now, restricted to a
** single non-typed REG node.  Returns register shape
** node number.
*/

static int
chknreg( s )
int s;
{
    while (s >= 0 && shp[s].sop == MANY) {
	/* if right non-negative, bad register set */
	if (shp[s].sright >= 0)
	    break;			/* MANY OP yields error */
	s = shp[s].sleft;
    }

    if (s < 0 || shp[s].sop != REG)
	yyerror("illegal scratch register set");
    else if (shp[s].sregset & ~RS_NRGS)
	yyerror("scratch register set includes non-scratch registers");

    return( s );
}


	/* VARARGS */
yyerror( s, a, b )
char *s;
{
	fprintf( stderr, s, a, b );
	fprintf( stderr, ", file \"%s\", line %d\n", filename, lineno );
	if (dumpcore)
	    abort();
	exit( 1 );
}

static int	otb[20];	/* ops that are part of the shape */
static int	notb;		/* number of otb entries */

doltmplt()
{
	/*
	 * rhs has a leaf: process templates for all interesting
	 * operators in this leaf. 
	 * An interesting operator is one that is NOT MANY.
	 * This allows us to access the table for each operator
	 * that this leaf may implement.
	 */
	register int	i;
	register int	s;

	notb = 0;
	lout1( s = rshty.sha );
	if( !notb ) yyerror( "lout1() error" );
	for( i = notb-1;  i >= 0;  --i )
	{
		op = otb[i];
		tyop = TANY;
		refine( s );
		dotmplt(1);
	}
}

#ifndef	NREFSH
#define	NREFSH	100
#endif

static int nrefsh;			/* number of refine() shapes */
int refsh[NREFSH];

static int
refine(s)
register s;
{
    int i;
    int prev;
    void refsubsh();

    /* make a new shape containing only sub-shapes of s that use
    ** 'op' (a global).
    */

    nrefsh = 0;				/* start with 0 */
    refsubsh(s, op);			/* find sub-shapes */
    if (nrefsh == 0)
	yyerror("no refine sub-shapes?");
    
    /* build up a new shape from the parts from right to left */
    for (prev = -1, i = nrefsh-1; i >=0; --i)
	prev = bop(MANY, refsh[i], prev);
    
    if (shp[prev].shname[0] == '\0')
	(void) strcpy(shp[prev].shname, "refine"); /* give shape a name */
    
    lshty.sha = -1;
    rshty.sha = prev;
    return( 0 );			/* no costs anymore */
}
/* collect sub-shapes for refine */

static void
refsubsh(s, op)
int s;					/* current shape */
int op;					/* op we're looking for */
{
    if (s < 0) return;
    if (shp[s].sop == MANY) {
	refsubsh(shp[s].sleft, op);
	refsubsh(shp[s].sright, op);
    }
    else if (shp[s].sop == op) {	/* remember this shape */
	refsh[nrefsh++] = s;
	if (nrefsh >= NREFSH)
	    yyerror("too many refine shapes");
    }
    
    return;
}

lout1( n )
register n;
{
	register int i;
	while( n >= 0  &&  shp[n].sop == MANY )
	{
		lout1( shp[n].sleft );
		n = shp[n].sright;
	}
	if( n < 0 )
		return;
	for( i=0; i<notb; ++i )
	{
		if( otb[i] == shp[n].sop ) return;
	}
	otb[notb++] = shp[n].sop;
}

mkshp(s)
{
	/* make a shape that yields s */
	/* first, make s a MANY node */
	register i;

	if( s < 0 ) return( s );
	if( shp[s].sop == MANY ) i = s;
	else {
		/* look for a MANY node pointing to s */
		for( i= NSTYSHPS; i>nmshp; --i ) {
			if( shp[i].sright >= 0 ) continue;
			if( shp[i].sleft == s ) goto foundit;
		}
		/* must make a MANY node */
		i = bop( MANY, s, -1 );
	}

	/* now, make sure that i has a name, so it will be output */

	foundit: ;

	if( !shp[i].shname[0] ) {
		strcpy( shp[i].shname, "mkshp" );
	}
	return(i);
}

onebit(x)
register x;
{
	/* return 1 if x has at most 1 bit on, 0 otherwise */
	return( !(x&(x-1)) );
}

dotmplt(isleaf)
int isleaf;
{
	register OPTB *q;

	if( lshty.ty == 0 ) lshty.ty = TANY;
	if( rshty.ty == 0 ) rshty.ty = TANY;

	switch( op )
	{

	case 0:
		yyerror( "0 op" );

	case STAR:
	case REG:
	case UNARY MINUS:
	case UNARY AND:
	case FLD:
		if( needs&(LSHARE|RSHARE) ) needs |= (LSHARE|RSHARE);
		break;

	case ASSIGN:
	case ASG PLUS:
	case ASG MINUS:
	case ASG MUL:
	case ASG DIV:
	case ASG MOD:
	case ASG AND:
	case ASG OR:
	case ASG ER:
	case ASG LS:
	case ASG RS:
		if( !(rewrite & (RNULL|RESCC|RESC1|RESC2|RESC3|RRIGHT|REITHER)) )
		{
			rewrite |= RLEFT;
		}
	}
#ifdef CG
			/*Only put the exception flags on
			  templates dealing with exceptable nodes*/
	if (!can_except(op))
	{
		needs &= ~(NO_IGNORE|NO_HONOR);
	}
#endif
	if( !rewrite ) rewrite = RNULL;

	if( !onebit( rewrite & (RNULL|RLEFT|RRIGHT|REITHER|RESC1|RESC2|RESC3) ) )
	{
		yyerror( "multiple results -- illegal (%o)", rewrite );
	}
	if(    ((rewrite & RLEFT) && (needs&LSHARE))
	    || ((rewrite & RRIGHT) && (needs&RSHARE))
	    || ((rewrite & REITHER) && (needs&(LSHARE|RSHARE)))
	) {
		if( asstring[0] != 'Y' )
			yyerror( "don't share on result side of tree" );
	}
	if( needs && (needs&(LSHARE|RSHARE)) == needs )
	{
		yyerror( "don't share without allocating something" );
	}
	checkout(asstring);

	q = &optb[opno];
	if( opno >= NOPTB ) yyerror( "too many templates" );
	q->line = firstline;
	q->op = op;
	q->tyop = tyop;
	if( ophd[op]>=0 )
	{
		optb[optl[op]].nxop = opno;
		optl[op] = opno;
	}
	else
	{
		optl[op] = ophd[op] = opno;
	}
	q->nxop = -1;
	q->l = lshty;
	q->r = rshty;
	q->oneeds = needs;
	q->rneeds = endnrgen( needs & NPAIR );
	q->orew = rewrite;

	q->string = asstring;
	q->isleaf = isleaf;		/* note leaf templates */
	/* now, take care of special cases */
	if( optype(op) == LTYPE ) {  /* leaf */
		int s;
		if( (s=q->r.sha) >= 0 && trivial(s) ) {
			q->r.sha = -1;  /* clobber any right shape */
		}
	}
	++opno;
}

trivial( s ) {
	/* is shape s a trivial match for op */
	if( shp[s].sop == MANY ) {
		if( shp[s].sright >= 0 ) {
			return( 0 );  /* nontrivial */
		}
		s = shp[s].sleft;
	}
	if( shp[s].sop != op ) {
		return( 0 );
	}
	if( shp[s].ssh ) {
		return( 0 );
	}
	return( 1 );  /* ok to clobber */
}

checkout(string)
char *string;
{
	/* check out the string, looking at rewrite and needs */
	/* look for {U,I,C,A}{L,R,1,2,3} and \n */
	/* complain about:
	***	1, 2, 3 used, not allocated
	***	shared side after \n after temp used
	***	AL or AR used, w. side effect possible, more than once 
	*/

	/* flagl and flagr are 1 if L and R legal, 0 if not, -1 if
	/* they will be illegal after the next \n */

	register int flagl, flagr, prn, min, cond;
	register char *s;

	flagl = flagr = 1;
	cond = 0;

	for( s=string; *s; ++s )
	{
		switch( *s )
		{

		case '\\':
			++s;
			if( *s == '\\' ) ++s;
			else if( *s == 'n' )
			{
				if( flagl<0 ) flagl=0;
				if( flagr<0 ) flagr=0;
			}
			break;

		case 'Z':
			++s;
			if( *s=='(' )
			{
				while( *++s != ')' ) {;}
			}
			break;

		case 'Y':
			/* this string is asserted to be good; don't check */
			return;

		case 'R':
		case 'D':
			/* conditional; a lot of stuff no longer is true */
			cond = 1;
			break;

		case 'A':
		case 'C':
		case 'U':
		case 'I':
			++s;
			if( *s == '-' )
			{
				++s;
				min = 1;
			}
			else min = 0;
			if( *s == '(' )
			{
				++s;
				prn = 1;
			}
			else prn = 0;
			switch( *s )
			{

			case 'L':
				if( !flagl && !cond )
				{
					yyerror( "illegal L just at \"%s\"",
							s );
				}
				/* look for side-effects here */
				if( !min && seff(lshty.sha)) flagl = 0;
				break;

			case 'R':
				if( !flagr && !cond )
				{
					yyerror( "illegal R just at \"%s\"",
							s );
				}
				/* look for side-effects here */
				if( !min && seff(rshty.sha)) flagr = 0;
				break;

			case '1':
			case '2':
			case '3':
				if( (*s - '0') > (needs&NCOUNT) )
				{
					yyerror( "reg %c used, not allocated",
						*s );
				}
				if( (needs&LSHARE) && flagl ) flagl = -1;
				if( (needs&RSHARE) && flagr ) flagr = -1;

			case '.':
				break;

			default:
				yyerror( "illegal qualifier just at \"%s\"",
					s );
			}
			if( prn ) while( *s != ')' ) ++s;
		}
	}
}

seff( s )
register s;
{
	if (s < 0) return( 0 );
	if( shp[s].sop == INCR || shp[s].sop == DECR ) return( 1 );
	return( seff( shp[s].sleft ) | seff( shp[s].sright ) );
}

uop( o, a )
register o,a;
{
	if( o == MUL ) o = STAR;
	else if( o == MINUS ) o = UNARY MINUS;
	else if( o == AND ) o = UNARY AND;
	return( bop( o, a, -1 ) );
}

top( a, ty )
register a,ty;
{
	/* build a type node over a */
	/* must be done differently than uop, since types must be copied */
	register STYSHP * shapea;
	int i;

	if( a < 0 )
		return( a );
	checkit( a );
	if( shp[a].sop == MANY )
	{
		register l, r;
		l = shp[a].sleft;
		r = shp[a].sright;
		if( l >= 0 )
			l = top( l, ty );
		if( r >= 0 )
			r = top( r, ty );
		return( bop( MANY, l, r ) );
	}
	if( shp[a].ssh )
	{
		yyerror( "can't type a special node" );
	}

	/* see if we can use another node first */
	shapea = &shp[a];
	if ((i = uniqshp(shapea->sop,shapea->sleft,shapea->sright,ty|SPTYPE,shapea->sregset)) >= 0) {
	    if (sdebug > 0)
		fprintf(dbg, "-->>top: line %d shares shape with shape %d\n",
			lineno, i);
	    return( i );
	}

	shp[nshp] = *shapea;
	shp[nshp].shname[0] = '\0';
	shp[nshp].ssh = ty|SPTYPE;
	nshp++;
	checkit( nshp-1 );
	return( nshp-1 );
}

bop( o, a, b )
register o, a, b;
{
	register STYSHP	*p;
	register int	l, r, ret;
	int i;

	checkit( a );
	checkit( b );

	if( o != MANY )
	{
		while( shp[a].sop == MANY && shp[a].sright < 0 ) {
			a = shp[a].sleft;
		}
		while( b >= 0 && shp[b].sop == MANY && shp[b].sright < 0 ) {
			b = shp[b].sleft;
		}
		if( a>=0  &&  shp[a].sop == MANY )
		{
			/* distribute MANY nodes to top */
			l = bop( o, shp[a].sleft, b );
			r = bop( o, shp[a].sright, b );
			return( bop( MANY, l, r ) );
		}
		if( b>=0 && shp[b].sop == MANY )
		{
			/* distribute MANY nodes to top */
			l = bop( o, a, shp[b].sleft );
			r = bop( o, a, shp[b].sright );
			return( bop( MANY, l, r ) );
		}
	}

	/* create new node or get existing one, return */

	return( fillshape(o, a, b, 0, 0) );
}

int olist[] = { PLUS, MINUS, MUL, DIV, MOD, LS, RS, OR, ER, AND, -1 };

struct nam
{
	char	*tntn;
	int	tnty;
};
#ifdef	__STDC__
#define NameV(x)	#x,	x	/* name-value pairs */
#else
#define NameV(x)	"x",	x	/* name-value pairs */
#endif
struct nam Tnam[] = {
	NameV(TANY),
	NameV(TCHAR),
	NameV(TSHORT),
	NameV(TINT),
	NameV(TLONG),
	NameV(TFLOAT),
	NameV(TDOUBLE),
	NameV(TUCHAR),
	NameV(TUSHORT),
	NameV(TUNSIGNED),
	NameV(TULONG),
	NameV(TSTRUCT),
	NameV(TPOINT),
	NameV(TPOINT2),
	NameV(TVOID),
	NameV(urTINT),
	NameV(urTUNSIGNED),
	0,		0,
};
struct nam Rwnam[] = {
	NameV(RLEFT),
	NameV(RRIGHT),
	NameV(REITHER),
	NameV(RESC1),
	NameV(RESC2),
	NameV(RESC3),
	NameV(RESCC),
	NameV(RNOP),
	NameV(RNULL),
	0,	0
};
struct nam Ndnam[] = {
	NameV(LSHARE),
	NameV(RSHARE),
	NameV(NPAIR),
	NameV(LPREF),
	NameV(RPREF),
	NameV(LMATCH),
	NameV(RMATCH),
	NameV(NO_IGNORE),
	NameV(NO_HONOR),
	0,	0,
};

finished()
{
    void finshape(), finoptb();
    void outshape(), outoptb(), outstats();
    void outnr();

    /* start output file */
    printf( "# include \"mfile2.h\"\n\n" );

    finshape();			/* finish shape processing */
    finoptb();			/* finish op table processing */

#ifdef	REGSET
    outnr();			/* output needs register bit vectors */
#endif

    outshape();			/* output shape information */
    outoptb();			/* output op table information */
    if (Sdebug)			/* do statistics */
	outstats();
}

static void
finshape()
{
	register OPTB	*q;
	register int	i;
	int mkpshp(), mkshp();

	/* everything that gets used should be a MANY shape with name */
	/* mkshp is called to cause this to be true */

	for( i=0; i<opno; ++i )
	{
		q = &optb[i];
		q->l.sha = mkshp(q->l.sha);
		q->r.sha = mkshp(q->r.sha);
	}

	/* Prepare pshapes array; also assign starting points (borrow .ssh)
	** so we can find them later.  Sort the shapes by shape number now.
	** Before outputting them, sort them by weights.
	*/

	nstypshp = 0;
	for( i=NSTYSHPS-1; i>nmshp; --i ) {
		if( !shp[i].shname[0] ) continue;
		shp[i].ssh = nstypshp;
		nstypshp = mkpshp( i, nstypshp );
		pshp[nstypshp++] = -1;	/* mark end of this pshape */
		if (nstypshp >= NSTYPSHPS)
		    yyerror("too many pshp shapes");
	}

	/* collect info about which OPs can head a shape */

	for( i = NSTYSHPS-1;  i > nmshp; --i )
	    setshphd( i );		/* mark heads of shapes */

	return;
}

static void
finoptb()
{
	register int i, j;
	OPTB * q, * q1;
	int op;
	void chkfunny();
	void chkreguse();
	void sorttmplt();
	void proporder();

	/* check for funny situations common in stin files and clean up */

	chkfunny();

	/* check for templates where too many registers are needed */

	chkreguse();

	/* sort templates for best order for code generation */

	sorttmplt();

	/* print order, if desired */

	if (odebug)
	    proporder();

	/* chain binary ops together with the op= form */
	for( i=0; (op=olist[i])>=0; ++i )
	{
		if( ophd[op]<0 )
			ophd[op] = ophd[ASG op];
		else
			optb[optl[op]].nxop = ophd[ASG op];
	}
	if( ophd[STCALL]<0 )
		ophd[STCALL] = ophd[CALL];
	if( ophd[UNARY STCALL]<0 )
		ophd[UNARY STCALL] = ophd[UNARY CALL];

#ifdef IN_LINE
	/* set up ophd[] entries so INCALL op uses CALL templates */
	ophd[INCALL] = ophd[CALL];
	ophd[UNARY INCALL] = ophd[UNARY CALL];
#endif

	/* avoid identical strings in optb[] by finding identical strings,
	giving them names and putting them at the front of table.c */
	for( i=0; i<opno; ++i )
	{
		q = &optb[i];
		q->struse = 1;
		q->strdef = -1;
		for(j = 0; j < i; j++)
		{
			q1 = &optb[j];
			if(!q1->struse)
				continue;
			if(strcmp(q->string,q1->string)==0)
			{
				q->strdef = j;
				q1->struse++;
				q->struse = 0;
				break;
			}
		}
	}
	for(i=0; i<opno; i++)
	{
		q = &optb[i];
		if(q->struse>1)
		{
			printf("static char Str%d[] = \"%s\";\n", i, q->string);
			q->struse = 0;
			q->strdef = i;
		}
	}

}

/* check for funnies in templates and clean up */

static void
chkfunny()
{
    int tmplt;
    int ptmplt = -1;			/* previous template */
    int hasfree = -1;			/* template # with FREE shape */
    int hasreg = -1;			/* template # with first REG shape */
    int hasregl = -1;			/* template # with last REG shape */

    /* keep just GENLAB and COMOP templates with FREE or REG as shapes */

    for (tmplt = ophd[GENLAB]; tmplt >= 0; tmplt = optb[tmplt].nxop) {
	int pshpno;
	int found = 0;

	for (	pshpno = shp[optb[tmplt].l.sha].ssh;
		!found && pshp[pshpno] >= 0;
		++pshpno) {
	    if (shp[pshp[pshpno]].sop == FREE) {
		hasfree = tmplt; found = 1;
	    }
	    else if (shp[pshp[pshpno]].sop == REG) {
		found = 1;
		hasregl = tmplt;	/* set last */
		if (hasreg < 0)		/* set first, if needed */
		    hasreg = tmplt;
	    }
	}

	if (!found) {
	    fprintf(stderr, "template %d useless, deleted\n", optb[tmplt].line);
	    if (ptmplt != -1) {
		optb[ptmplt].nxop = optb[tmplt].nxop;
	    }
	}
	ptmplt = tmplt;
    }

    if (hasfree >= 0 && hasreg >= 0) {	/* we have both kinds */
	optb[hasfree].nxop = hasreg;	/* tack list of REGs after FREE */
	optb[hasregl].nxop = -1;
	ophd[GENLAB] = hasfree;
	optl[GENLAB] = hasregl;
    }
    else if (hasfree >= 0) {
	ophd[GENLAB] = optl[GENLAB] = hasfree;
	optb[hasfree].nxop = -1;
    }
    else if (hasreg >= 0) {
	ophd[GENLAB] = hasreg;
	optl[GENLAB] = hasregl;
	optb[hasregl].nxop = -1;
    }
    /* by default, leave what was there there if can't find what we want */


    hasfree = hasreg = -1;
    for (tmplt = ophd[COMOP]; tmplt >= 0; tmplt = optb[tmplt].nxop) {
	int pshpno;
	int found = 0;

	for (	pshpno = shp[optb[tmplt].r.sha].ssh;
		!found && pshp[pshpno] >= 0;
		++pshpno) {
	    if (shp[pshp[pshpno]].sop == FREE) {
		hasfree = tmplt; found = 1;
	    }
	    else if (shp[pshp[pshpno]].sop == REG) {
		hasreg = tmplt; found = 1;
	    }
	}

	if (!found)
	    fprintf(stderr, "template %d useless, deleted\n", optb[tmplt].line);
    }

    if (hasfree >= 0 && hasreg >= 0) {	/* we have both kinds */
	optb[hasfree].nxop = hasreg;
	optb[hasreg].nxop = -1;
	ophd[COMOP] = hasfree;
	optl[COMOP] = hasreg;
    }
    else if (hasfree >= 0) {
	ophd[COMOP] = optl[COMOP] = hasfree;
	optb[hasfree].nxop = -1;
    }
    else if (hasreg >= 0) {
	ophd[COMOP] = optl[COMOP] = hasreg;
	optb[hasreg].nxop = -1;
    }
    return;
}
/* Check for templates that require too many registers.  The compiler gets
** into trouble when it selects a template that needs too many registers.
** A notable example arises when $A is used and one or both operands have
** shapes that need registers that can't be shared.  Warn about such
** situations and force the compiler to match the shape exactly.
*/

static void
chkreguse()
{
    int tmplt;				/* template number */
    int regsinshp();
    int pshpno;
    int warningno = 0;			/* index number on warnings */

#ifndef	REGSET
/* REGSET must calculate on the fly, taking into account which
** set of registers is selected for scratch.
*/
    /* for each pshp[] element, get number of registers used */

    for (pshpno = 0; pshpno < nstypshp; ++pshpno) {
	if (pshp[pshpno] < 0) continue;
	pshpreg[pshpno] = regsinshp(pshp[pshpno]);
    }
#endif	/*ndef REGSET */

    for (tmplt = 0; tmplt < opno; ++tmplt) {
	register OPTB * ptmplt = &optb[tmplt];
	register int needs = ptmplt->oneeds; /* template's needs */
	int lneedregs = 0;		/* registers needed by left */
	int rneedregs = 0;		/* registers needed by right */
	int tneedregs = needs & NCOUNT;	/* registers needed initially by tmplt */
#ifdef	REGSET
	RST scratchset;			/* register set for scratch registers */
	int avail;			/* available scratch registers in the
					** scratch sets mentioned in template
					*/
#else
#   define avail NRGS			/* for non-REGSET, always all scratch */
#endif
	int maxreg();

	if (needs & NPAIR) tneedregs *= 2;/* if pairs, double the number */

	/* assume that sharing will work, don't count possibly shared regs */

#ifndef	REGSET
	if ((needs & (LSHARE|LPREF)) == 0)
	    lneedregs = maxreg(ptmplt->l.sha);
	
	if ((needs & (RSHARE|RPREF)) == 0)
	    rneedregs = maxreg(ptmplt->r.sha);
#else
	/* Determine scratch register set and the total number of such
	** registers.
	*/
	if (ptmplt->rneeds == 0) {	/* no explicit scratch; assume NRGS */
	    scratchset = RS_NRGS;
	    avail = NRGS;
	}
	else {
	    RST rsallbits();
	    register int index;
	    RST tscratchset = rsallbits(ptmplt);
	    
	    scratchset = tscratchset;
	    /* figure out how many registers there are in scratch set */
	    avail = 0;
	    while (tscratchset != RS_NONE) {
		tscratchset -= RS_CHOOSE(tscratchset);
		++avail;
	    }
	}

	if ((needs & (LSHARE|LPREF)) == 0)
	    lneedregs = maxreg(ptmplt->l.sha, scratchset);
	
	if ((needs & (RSHARE|RPREF)) == 0)
	    rneedregs = maxreg(ptmplt->r.sha, scratchset);
#endif	/* ndef REGSET */
	
	if (rdebug)
	    fprintf(dbg, "template %d needs %d + %dL + %dR registers out of %d\n",
			    ptmplt->line, tneedregs, lneedregs, rneedregs, avail);
	
	/* check for out of registers */
	if (tneedregs + lneedregs + rneedregs > avail) {
	    fprintf(stderr, "warning(%d):  template %d can run out of registers\n",
			++warningno, ptmplt->line);
	    
	    if (lneedregs != 0 && (needs & (LSHARE|LPREF)) == 0) {
		ptmplt->oneeds |= LMATCH; /* force left side exact match */
		fprintf(stderr, "\tleft side must match exactly\n");
	    }

	    if (rneedregs != 0 && (needs & (RSHARE|RPREF)) == 0) {
		ptmplt->oneeds |= RMATCH; /* force right side exact match */
		fprintf(stderr, "\tright side must match exactly\n");
	    }
	}
    } /* end for */
    return;
}
/* add up the scratch registers used by a single shape */

#ifndef	REGSET

static int
regsinshp( sshpno )
int sshpno;				/* s-shape number of root of shape tree */
{
    int regsize();

    if ( sshpno < 0 ) return 0;

    /* return size of REG, based on type */
    if (shp[sshpno].sop == REG) {
	return( regsize((unsigned long) shp[sshpno].ssh ));
    }

    /* otherwise add sizes */

    return( regsinshp(shp[sshpno].sleft) + regsinshp(shp[sshpno].sright ));
}

#else	/* ifdef REGSET */

static int
regsinshp( sshpno, scratchset )
int sshpno;				/* s-shape number of root of shape tree */
RST scratchset;				/* scratch registers of interest */
{
    int regsize();

    if ( sshpno < 0 ) return 0;

    /* Claim we have a scratch register in the shape only if
    ** all scratch registers in the shape are members of the
    ** scratch set.  Otherwise there are other scratch registers
    ** outside the designated scratch register set that could hold
    ** the subtree for this shape.
    */
    /* IS THIS RIGHT FOR PAIRS? */
    if (shp[sshpno].sop == REG) {
	register RST scratchregs = shp[sshpno].sregset & RS_NRGS;
	if (scratchregs == 0 || (scratchregs & scratchset) != scratchregs)
	    return( 0 );		/* no countable scratch set regs
					** in this REG
					*/
	else
	    return( regsize((unsigned long) shp[sshpno].ssh ));
    }

    /* otherwise add sizes */

    return(    regsinshp(shp[sshpno].sleft, scratchset)
	     + regsinshp(shp[sshpno].sright, scratchset)
	  );
}

#endif	/* ndef REGSET */
/* return size of a REG shape */

static int
regsize( types )
unsigned long types;			/* special shape type bits */
{
    int size = 1;			/* presumed size of register */
    if ((types & SPTYPE) == 0)		/* no type specified */
	return( 1 );			/* assume single register */
    
    types &= ~SPTYPE;			/* remove bit */

    while (types != 0) {
	unsigned long bit = types & ((~types)+1);
					/* extract right-most bit */
	int newsize = szty( bit );	/* get size of this type */

	if (newsize > size) size = newsize;
	types -= bit;			/* remove the bit */
    }
    return( size );
}
/* determine maximum number of registers required by any shape in
** a collection of shapes
*/

#ifndef	REGSET

static int
maxreg(sshpno)
int sshpno;				/* s-shape that heads the collection */
{
    register int pshpno;		/* index into pshp[], pshpreg[] */
    int max = 0;			/* maximum so far */

    if (sshpno < 0) return( 0 );	/* no shape */

    /* follow reference in s-shape to p-shape array */
    for (pshpno = shp[sshpno].ssh; pshp[pshpno] >= 0; ++pshpno)
	if (pshpreg[pshpno] > max) max = pshpreg[pshpno];
 
    return( max );
}

#else	/* ifdef REGSET */

static int
maxreg(sshpno, scratchset)
int sshpno;				/* s-shape that heads the collection */
RST scratchset;				/* scratch register set to look for */
{
    register int pshpno;		/* index into pshp[], pshpreg[] */
    int max = 0;			/* maximum so far */
    int temp;

    if (sshpno < 0) return( 0 );	/* no shape */

    /* follow reference in s-shape to p-shape array */
    for (pshpno = shp[sshpno].ssh; pshp[pshpno] >= 0; ++pshpno) {
	/* figure maximum in all sshapes comprising pshape */
	temp = regsinshp( pshp[pshpno], scratchset );
	if (temp > max) max = temp;
    }
 
    return( max );
}

#endif	/* ndef REGSET */

static void
outshape()
{
	int i;
	STYSHP * p;
	int pshpindex;
	void saaddr();
	void chkundef();
	void chkdups();
	void outshphd();
	int outpshp();

	chkundef();			/* check for undefined shapes */

	printf( "\n# define SHNL ((SHAPE *)0)\n" );
	printf( "# define S(x) (&shapes[x])\n" );

	printf( "\n#ifdef NODBG\nstatic\n#endif\nSHAPE shapes[] = {\n" );
	for( i=0, p=shp; i<nshp; ++i,++p )
	{
		printf( "/*%4d */ ", i);
		printf( "%4d,\t",p->sop );
		saaddr(p->sleft);
		saaddr(p->sright);
		printf( "%#9o,", p->ssh );
#ifdef	REGSET
		printf( "\t%#o,", (p->sop == REG ? p->sregset : 0) );
#endif
		if( p->shname[0] )
			printf( "\t/* %.*s */\n", NSHNAME, p->shname );
		else
			putchar( '\n' );
		if(p->shname[0])
			chkdups(i);
	}
	printf( "\n};\n\n" );


	outshphd();			/* output shphd table */

	printf( "\n# define PSHNL ((SHAPE **)0)\n" );
	printf( "# define P(x) (&pshape[x])\n" );

	printf( "\n#ifdef NODBG\nstatic\n#endif\nSHAPE *pshape[] = {\n" );
	pshpindex = 0;
	for( i = NSTYSHPS-1;  i > nmshp; --i ) {
		int temp;
		if( !shp[i].shname[0] )
			continue;
		printf( "/*%4d %.*s */\t", pshpindex, NSHNAME, shp[i].shname  );
		temp = pshpindex;	/* remember current index */
		pshpindex = outpshp( i, pshpindex );
		shp[i].ssh = temp;	/* set output index for templates */
		printf( "SHNL,\n");
		++pshpindex;		/* count one for SHNL */
	}
	printf( "};\n\n" );
	return;
}
/* Check for undefined shapes.  At this stage we're looking for MANY
** nodes that have any descendents whose sop field is < 0.
*/

static void
chkundef()
{
    int i;

    /* Just look at named MANY nodes.  Backward search is related to the
    ** order of appearance in input.
    */
    for (i = NSTYSHPS-1; i > nmshp; --i) {

	if (shp[i].sop >= 0 && shp[i].sop != MANY)
	    yyerror("confused chkundef(), op %d, shape %d", shp[i].sop, i);
	
	if (shp[i].sop < 0)
	    yyerror("undefined shape %.*s", NSHNAME, shp[i].shname);
    }
    return;
}

static void
outoptb()
{
	OPTB * q;
	int i;
	void prop();
	void shpprint();
	void prnam();

#ifdef	REGSET
	printf( "# define\tRSNULL\t((RST *) 0)\n");
#endif
	printf( "#ifdef NODBG\nstatic\n#endif\nstruct optab table[] = {\n\n" );
	for( i=0; i<opno; ++i )
	{
		q = &optb[i];
		printf( "/* # %d, line %d */\n", i, q->line );
		prop( q->op );
		prnam( q->tyop, Tnam );
		if( q->nxop >= 0 )
			printf( "\t&table[%d],\n", q->nxop );
		else
			printf( "\t0,\n" );

		shpprint(q->l.sha, q->l.ty);
		shpprint(q->r.sha, q->r.ty);
		printf("\t\t");
		prnam(q->oneeds, Ndnam);
#ifdef	REGSET
		printf("\t\t%d,\n", q->rneeds);
#endif
		printf("\t\t");
		prnam(q->orew, Rwnam);
		if(q->strdef>=0)
			printf("\t\tStr%d,\n", q->strdef);
		else
			printf("\t\t\"%s\",\n", q->string);
		printf("\t\t%d,\n", q->line );
	}
	printf( "\n};\n" );

	printf( "OPTAB *ophead[] = {\n" );

	for( i=0; i<DSIZE; ++i )
	{
		if( ophd[i] < 0 ) printf( "	0,\n" );
		else printf( "	&table[%d],\n", ophd[i] );
	}
	printf( "};\n" );
}
/* output statistics */

static void
outstats()
{
    /* s-shapes */
    fprintf(stderr, "shp	%d(%d+%d)/%d\n",
				nshp-1+NSTYSHPS-(nmshp+1),
				nshp-1,
				NSTYSHPS-(nmshp+1),
				NSTYSHPS
	);
    
    /* p-shapes */
    fprintf(stderr, "pshp	%d/%d\n", nstypshp, NSTYPSHPS);

    /* optab table */
    fprintf(stderr, "optb	%d/%d\n", opno, NOPTB);

    /* string table */
    fprintf(stderr, "strng	%d/%d\n", pstring-strng, NSTRING);

    /* template sorting table */
    fprintf(stderr, "tmplt	%d/%d\n", maxtmplt, NTMPLTS);

    /* shape weights sorting table */
    fprintf(stderr, "weight	%d/%d\n", nweight, NWEIGHTS);

#ifdef	REGSET
    /* register set merge table */
    fprintf(stderr, "register merge	%d/%d\n", nrmerge, NRMERGE);

    /* register set bit vector table */
    fprintf(stderr, "register sets	%d/%d\n", nrsbits, NRSBITS);
#endif

    /* number of shape head bits used */
    fprintf(stderr, "shphd bits		%d/%d\n", shphdbits, HOSTPTRSIZE);

    return;
}
/* add shapes to pshapes array:  elaborate the s-shapes */

static int
mkpshp( i, pshpno )
int i;				/* shape number to elaborate */
int pshpno;			/* current index in pshp array */
{
    if (i >= 0) {
	if (shp[i].sop == MANY) {
	    pshpno = mkpshp( shp[i].sleft, pshpno );
	    pshpno = mkpshp( shp[i].sright, pshpno );
	}
	else
	    pshp[pshpno++] = i;

	if (pshpno >= NSTYPSHPS)
	    yyerror("too many pshp shapes");
    }
    
    return( pshpno );
}
/* sort s-shapes of a p-shape by s-shape number; use
** insertion sort, because array usually nearly sorted;
** borrowed from CACM 27/4, p. 288
*/

static void
sortpshp(lb, hb)
register int lb;			/* lower bound */
int hb;					/* upper bound */
{
    int i;
    register int cur;			/* current index */
    register int temp;

    for (i = lb+1; i <= hb; ++i) {
	/* pshp[lb..i-1] already sorted */
	/* insert pshp[i] in its place in previously sorted stuff */
	for ( temp = pshp[cur=i]; cur > lb && pshp[cur-1] > temp; --cur)
	    pshp[cur] = pshp[cur-1];
	pshp[cur] = temp;
    }
    return;
}
/* Order templates.  The idea here is that we want to put
** templates with more restrictive shapes ahead of templates
** containing more general shapes, all other things being
** equal.  To do this, we need an ordering relation for
** shapes, and another one for the templates themselves.
** Remembering that p-shapes containing a REG shape will
** match anything, our ordering relation considers that
** such p-shapes are always supersets of p-shapes that don't
** contain REG shapes.
*/

static int tmplts[NTMPLTS];		/* array for sorting */

static void
sorttmplt()
{
    int i, j;
    int tmplt;
    int sorti, sortcur;

    /* sort p-shapes for benefit of shape-order-relation */
    for( i=NSTYSHPS-1; i>nmshp; --i ) {
	if( !shp[i].shname[0] ) continue;
	/* find end of pshape, marked by -1 */
	for (j = shp[i].ssh; pshp[j] >= 0; ++j)
	    ;
	sortpshp( shp[i].ssh, j-1 );
    }

    /* check for ordering suppression here, rather than before shape
    ** ordering to make comparisons of table.c output easier.
    */

    if (Ndebug)
	return;

    /* for each OP in ophd, elaborate chain of templates in tmplts array,
    ** sort according to ordering criteria, then rebuild ophd list
    */

    for (i = 0; i<DSIZE; ++i) {
	int ntmplt;
	int next;

	if (ophd[i] < 0) continue;	/* no templates for this OP */
	/* don't sort GENLAB or COMOP (special hack) -- see chkfunny() */
	if (i == GENLAB || i == COMOP) continue;

	if (tdebug > 1)
	    fprintf(dbg, "ordering OP %s\n", opst[i]);

	/* quit when tmplt is the last in the chain */
	for (ntmplt=0, tmplt = ophd[i]; tmplt >= 0; ++ntmplt) {
	    tmplts[ntmplt] = tmplt;
	    if (ntmplt >= NTMPLTS-2)
		yyerror("too many templates for OP %d", i);
	    tmplt = optb[tmplt].nxop;	/* follow chain */
	}
	tmplts[ntmplt] = -1;		/* terminate list */
	if (ntmplt > maxtmplt)		/* remember max. for stats */
	    maxtmplt = ntmplt;

	/* use ordering relation to sort templates (insertion sort);
	** Alas, the ordering relation is not transitive.  Use the
	** textbook insertion sort, but sort in descending order.
	** This causes us to examine most of the previous templates,
	** but it insures that we get the right order.
	*/
	for (sorti = 1; sorti < ntmplt; ++sorti) {
	    int temp;
	    int cmptmplt();

	    /* entries 0..sorti-1 are in descending order */
	    for (	temp = tmplts[sortcur=sorti];
			sortcur > 0 && cmptmplt(tmplts[sortcur-1], temp) <= 0;
			--sortcur
		)
		tmplts[sortcur] = tmplts[sortcur-1];
	    /* expect templates to find their way to the head of the list */
	    if (sortcur != 0) { 		/* if something's moved */
		optb[temp].moved = 1;		/* note this template moved */
		if (tdebug) {
		    fprintf(dbg, "moving template %d ahead of template %d",
			optb[temp].line, optb[tmplts[sortcur-1]].line);
		    if (tdebug == 1 && optb[temp].isleaf)
			fprintf(dbg, " (op %s)", opst[i]);
		    putc('\n', dbg);
		}
	    }
	    tmplts[sortcur] = temp;
	}

	/* rebuild linked list of templates for this OP, going backward */

	next = -1;
	for (j = 0; j < ntmplt; ++j) {
	    optb[tmplts[j]].nxop = next;
	    next = tmplts[j];
	}
	ophd[i] = next;
	optl[i] = tmplts[0];
    }	/* end FOR loop on OPs */
    return;
}
/* template ordering relation.
** This relation depends on:
** 	OP types
**	needs
**	relation between p-shapes for the operands
**
** In general, we want to place "more specific" templates before "less
** specific" ones.  Specificity has to do with the p-shapes of the two
** templates' operands, the types on the OPs, and the needs.  There is
** a close tie-in here with how the code generator does its work,
** obviously.  For example, since REG operands of type INT will match
** any integral type, we often say that templates containing them as shapes
** are logically after templates that contain shapes for, say, CHAR REGs.
**
** The following table represents the ordering of two templates, given
** the ordering of the left and right shapes:
**
**
**			RIGHT SHAPE ORDER
**
**			<	=	>	?
**		    ================================
**		    |
**		<   |	<	<	<	<
**		    |
**  LEFT	=   |	<	=	>	?
**  SHAPE	    |
**  ORDER	>   |	>	>	>	>
**		    |
**		?   |	?	?	?	?
**
*/

/* codes for template and shape comparison; the values for the last three
** are significant
*/

#define	NOORDER	1000			/* unordered */
#define	LEQLR	0			/* left and right objects same (L==R) */
#define	LSUBOFR	(-1)			/* left (p-shape) subset of right (L < R) */
#define	RSUBOFL	1			/* right (p-shape) subset of left (L > R) */

/* rewrites if template has a result */
#define	RESULT	(RLEFT|RRIGHT|REITHER|RESC1|RESC2|RESC3)

static int
cmptmplt(lt, rt)
int lt;					/* "left" template # (optb #) */
int rt;					/* ditto, for right side */
{
    OPTB * left = &optb[lt];
    OPTB * right = &optb[rt];
    int torder = NOORDER;		/* assumed template order */
    int lcmp, rcmp;			/* result of comparing left, right opns */
    int cmpresult();
    int cmppshp();
    void prshpcmp();
    void shprtype();

    if (left->op != right->op)
	yyerror("OP screw-up in cmptmplt()");
    
    /* hairy stuff only when types overlap */
    if ((left->tyop & right->tyop) == 0)	/* no overlap */
	goto returnval;

    /* take care of ordering based on results rewrites */
    if ((torder = cmpresult(left->orew, right->orew)) != NOORDER)
	goto returnval;

    /* If one template's types are a proper subset of the other's,
    ** the first is < the second.
    */
    if (left->tyop != right->tyop) {
	if ((left->tyop & right->tyop) == left->tyop) {
	    /* left is subset of right */
	    torder = LSUBOFR;
	    goto returnval;
	}
	else if ((left->tyop & right->tyop) == right->tyop) {
	    /* right is subset of left */
	    torder = RSUBOFL;
	    goto returnval;
	}
    }

    /* get respective orders of left and right shapes */

    lcmp = rcmp = NOORDER;
    if ((left->l.ty & right->l.ty) != 0)	/* types must offer some hope */
	lcmp = cmppshp(&left->l, &right->l, asgop(left->op));
    if ((left->r.ty & right->r.ty) != 0)
	rcmp = cmppshp(&left->r, &right->r, 0);

    if (tdebug > 1) {
	fprintf(dbg, "comparing templates %d::%d\n", left->line, right->line);
	fprintf(dbg, "left operands:  ");
	prshpcmp(left->l.sha, lcmp, right->l.sha);
	fprintf(dbg, ", right operands:  ");
	prshpcmp(left->r.sha, rcmp, right->r.sha);
	putc('\n', dbg);
    }

    /* Choose an ordering based on the order of the operands.
    ** We want more restrictive shapes ahead of less restrictive.
    ** match() examines right sides before left sides.  The ordering
    ** here considers left sides before right sides because the
    ** compiler seems to produce better code that way.
    */
    if ((torder = lcmp) == LEQLR)
	torder = rcmp;

returnval:				/* choose value to return */

    /* this assumes LSUBOFR < 0 and RSUBOFL > 0 */
    if (torder == NOORDER)
	torder = left->line - right->line;
    /* remark about equivalent templates */
    else if (   torder == LEQLR		/* equivalent templates */
	     && (left->orew & RESULT) != 0 && (right->orew & RESULT) != 0
	     && (left->l.ty & right->l.ty) != 0
	     && (left->r.ty & right->r.ty) != 0) {
					
	/* remark about shapes covering one another */
	fprintf(stderr, "line %d hides line %d", left->line, right->line);

	if (left->isleaf)		/* so we know which OP */
	    fprintf(stderr, " (op %s)", opst[left->op]);

	if (left->l.ty != right->l.ty) {
	    fprintf(stderr, " left type ");
	    shprtype(stderr, (unsigned long) (left->l.ty & right->l.ty));
	}
	if (left->r.ty != right->r.ty) {
	    fprintf(stderr, " right type ");
	    shprtype(stderr, (unsigned long) (left->r.ty & right->r.ty));
	}
	if (left->tyop != right->tyop) {
	    fprintf(stderr, " for types ");
	    shprtype(stderr, (unsigned long) (left->tyop & right->tyop));
	}
	putc('\n', stderr);
    }

    if (tdebug > 1) 
	fprintf(dbg, "template %d %s %d\n",
				left->line,
				(torder ? (torder > 0 ? ">" : "<") : "=="),
				right->line);
    return( torder );
}
/* Determine relation between two rewrites.
** Consider $N templates to be more specific than $C,
** $C more specific than any others
*/

static int
cmpresult(left, right)
int left;				/* left rewrite */
int right;
{
    register int result;		/* keep track of prospective result */

    if (left == RNULL)		result = 1;	/* arbitrary numbers */
    else if (left == RESCC)	result = 2;
    else			result = 3;

    if (right == RNULL)		result -= 1;
    else if (right == RESCC)	result -= 2;
    else			result -= 3;

    if (result == 0)
	return( NOORDER );		/* assume no order for same results */
    else if (result < 0)
	return ( LSUBOFR );		/* left < right */
    else
	return ( RSUBOFL );		/* right < left */
}
/* print relation between two shapes */

static void
prshpcmp( lshp, rel, rshp )
int lshp;				/* left shape # */
int rel;				/* relation */
int rshp;				/* right shape */
{
    char * relstring;

    switch( rel ) {
    case NOORDER:  relstring = "??"; break;
    case LSUBOFR:  relstring = "<"; break;
    case RSUBOFL:  relstring = ">"; break;
    case LEQLR:    relstring = "=="; break;
    default:	   relstring = "?bug?"; break;
    }

    fprintf(dbg, "%.*s %s %.*s",
		NSHNAME, (lshp < 0 ? "--" : shp[lshp].shname),
		relstring,
		NSHNAME, (rshp < 0 ? "--" : shp[rshp].shname));
    return;
}
/* Compare two p-shapes.  This comparison depends on identical shapes
** being shared, and thus having the same s-shape number, leaving only
** fundamentally different shapes with uniqe s-shape numbers.  We try
** to find cases where one p-shape is a sub- or super-set of another.
** The p-shape numbers are indices in the shp[] array of (presumably) MANY's.
** For non-lvalue shapes, a p-shape containing a REG node automatically
** is a super-set of one that does not.  However, if both p-shapes contain
** REGs and no other order can be determined, consider p-shapes with non-INT
** integral types to come before INT-types.
** For REGSET, check whether left and right shapes are just REG shapes,
** albeit different register sets, and look for subset relation.
*/

static int
cmppshp( lshty, rshty, lval )
SHTY * lshty;				/* pointer to left shape/type */
SHTY * rshty;				/* pointer to right shape/type */
int lval;				/* non-zero if left shape used in asgop */
{
    register int retval = NOORDER;	/* presumed order unless something else */
    int l = lshty->sha;			/* left p-shape number */
    int r = rshty->sha;			/* right p-shape number */
    int lindex, rindex;			/* pshp[] indices */
    int lshp, rshp;			/* corresponding members of pshp[] */
    int lstart, rstart;			/* starting points for left, right */
    int lbigger = 0, rbigger = 0;	/* remember if one side has more elements */
    unsigned long lreg, rreg;		/* left, right register types, if any */
    unsigned long hasreg();

    if (l == r) return(LEQLR);		/* same shape (includes case of -1::-1) */
    if (l < 0 || r < 0) return NOORDER;
    lstart = shp[l].ssh;		/* get pshp[] indices */
    rstart = shp[r].ssh;

    if (! lval) {			/* check for REG shapes */
	if (lreg = hasreg(lstart)) {
	    if (!(rreg = hasreg(rstart)))
		return( RSUBOFL );	/* left has REG, right doesn't */
	}
	else if (rreg = hasreg(rstart))
	    return( LSUBOFR );		/* right has REG, left doesn't */
	
	/* Reach here if both p-shapes contain REG shapes; decide if we have
	** an INT/non-INT case.  Modify types by types in templates.
	*/
	lreg &= lshty->ty;
	rreg &= rshty->ty;

	if ((lreg & (TINT|TUNSIGNED)) != 0 && (rreg & (TINT|TUNSIGNED|TFLOAT|TDOUBLE)) == 0)
	    retval = RSUBOFL;		/* call right < left */
	else if ((lreg & (TINT|TUNSIGNED|TFLOAT|TDOUBLE)) == 0 && (rreg & (TINT|TUNSIGNED)) != 0)
	    retval = LSUBOFR;		/* call left < right */
    }

#ifdef	REGSET
    /* Look for just REG shapes on each side, with subset relation between
    ** them.
    */
    if (   pshp[lstart+1] < 0 && shp[lshp = pshp[lstart]].sop == REG
	&& pshp[rstart+1] < 0 && shp[rshp = pshp[rstart]].sop == REG
    ){
	STYSHP * plshp = &shp[lshp];
	STYSHP * prshp = &shp[rshp];
	/* get types on the regs */
	int ltype = ((plshp->ssh & SPTYPE) ? TANY : plshp->ssh) & lshty->ty;
	int rtype = ((prshp->ssh & SPTYPE) ? TANY : prshp->ssh) & rshty->ty;
	RST lrset = plshp->sregset;
	RST rrset = prshp->sregset;

	/* check for subset relation of register set and type */
	if ((ltype & rtype) == ltype && (lrset & rrset) == lrset)
	    return( LSUBOFR );
	else if ((ltype & rtype) == rtype && (lrset & rrset) == rrset)
	    return( RSUBOFL );
	else
	    return( NOORDER );
    }
#endif	/* def REGSET */

    /* look for sub-/super-set relation */

    /* we assume, here, entries in pshp[] are ordered by s-shape number;
    ** this makes it easier to check for sub-/super-set
    */
    for (	lindex = lstart, rindex = rstart;
		lshp = pshp[lindex], rshp = pshp[rindex], lshp >= 0 && rshp >= 0;
		++lindex, ++rindex
	) {
	if (lshp < rshp) {		/* left side has more members than right */
	    if (rbigger)		/* is this a change? */
		return( retval );	/* yes */
	    ++lbigger;			/* indicate left side has more */
	
	    do {			/* scan until left side catches right */
		++lindex;
	    } while ((lshp = pshp[lindex]) < rshp && lshp >= 0);
	    if (lshp < 0)
		return( retval );	/* ran out of left members */
	}
	else if (lshp > rshp) {		/* inverse situation from above */
	    if (lbigger)
		return( retval );
	    ++rbigger;

	    do {
		rindex++;
	     } while((rshp = pshp[rindex]) < lshp && rshp >= 0);
	    if (rshp < 0)
		return( retval );
	}

	/* at this point we could have fallen out of one of the do-while loops
	** above; in any case, the two entries we're looking at must be the
	** same
	*/

	if (lshp != rshp)
	    return( retval );
    }

    /* At this point we've tracked through both arrays successfully.
    ** One or both current shapes are -1.  If we arrived at the end of
    ** each p-shape simultaneously, one or the other could still have
    ** more members.
    ** 
    ** One hitch:  we could have the following situation:
    **	left:	a b c d e
    ** right:	  b c d e f
    ** We have to declare these as NOORDER.
    */

    if (    (lshp <= 0 && lbigger)
	||  (rshp <= 0 && rbigger)
	)
	return( retval );

    if (lshp >= 0)			/* left has more members */
	retval = RSUBOFL;
    else if (rshp >= 0)
	retval = LSUBOFR;
    
    /* both must be -1 */
    else if (lbigger)
	retval = RSUBOFL;
    else if (rbigger)
	retval = LSUBOFR;
    else
	retval = LEQLR;			/* must be identical here */

    /* look for cases where, for example, L < R, but L's only shape
    ** is a REG shape.  In this case it's better to say R > L.
    */
    if (    retval == LSUBOFR
	&&  lindex == lstart+1		/* had one left shape */
	&&  shp[pshp[lstart]].sop == REG /* this is the one left shape */
	)
	retval = RSUBOFL;
    else if (    retval == RSUBOFL
	    &&   rindex == rstart+1
	    &&   shp[pshp[rstart]].sop == REG
	    )
	retval = LSUBOFR;
    
    return( retval );
}
/* check a p-shape for a REG shape */

static unsigned long
hasreg(pshpno)				/* return REG's type if has, else 0 */
register int pshpno;
{
    register int sshpno;
    register STYSHP * shpptr;

    while ((sshpno = pshp[pshpno++]) >= 0) {
	shpptr = &shp[sshpno];
	if (shpptr->sop == REG)
	    return( (shpptr->ssh & SPTYPE) == 0 ? TANY : shpptr->ssh );
					/* found a REG; return its type */
    }
    
    return( 0L );			/* no REG found */
}
/* print order of templates, by OP */

static void
proporder()
{
    int op;
    int tmpltno;
    OPTB * tmplt;

    for (op = 0; op < DSIZE; ++op) {
	if ((tmpltno = ophd[op]) < 0) continue;	/* no templates, this OP */

	/* get around problem with laser printer when "," begins line */
	if (*opst[op] == ',')
	    putc(' ', dbg);

	fprintf(dbg, "%s", opst[op]);
	do {
	    tmplt = &optb[tmpltno];
	    fprintf(dbg, "\t%5d", tmplt->line);
	    if (tmplt->isleaf || tmplt->moved)
		fprintf(dbg,"\t%s%s",
			(tmplt->isleaf ? "leaf" : ""),
			(tmplt->moved ? "\tmoved" : "")
		);
	    putc('\n', dbg);
	    tmpltno = tmplt->nxop;
	} while (tmpltno >= 0);
    }
    return;
}
/* Output a pshape, consisting of s-shapes, terminated by -1.
** We find remember the shape number, as well as each one's
** weight, based on the number of non-REG nodes therein.
** We then sort the shapes by weight, lowest first,
** and output the shapes in that order.
** The idea is to provide the match code in pcc2 with "constant"
** trees first (those with no registers therein), then with
** nodes containing registers, biggest first.
*/

typedef struct shwt {
    int shpno;				/* shape number */
    int shpwt;				/* weight for this shape */
} SHWT;

static SHWT weights[NWEIGHTS];		/* allocate array of shape weights */

/* Output a collection of shapes in the pshapes array.
** Sort them by weight first.
*/

static int				/* return final pshp output index */
outpshp(shno, pindex)
int shno;				/* shape of interest */
int pindex;				/* starting pshp index in output */
{
    int getm();
    void wqsort();
    int i, count;
    unsigned long opmask = 0;		/* bit vector of bits for OPs */
    int index;				/* index of next array element
					** (== # in array)
					*/

    index = getm(shno, 0);		/* collect the shapes and weights */
    if (index > nweight)		/* remember maximum */
	nweight = index;
    wqsort(0, index-1);			/* sort entries */

    /* calculate OR of bits for OPs heading shapes in this pshape; assume
    ** shphd has been calculated already.
    */

    for (i = 0; i < index; i++)
	opmask |= shphd[shp[weights[i].shpno].sop];

    /* figure out pointer to first REG shape, if any, and output */

    for (i = 0; i < index; i++) {
	int o = shp[weights[i].shpno].sop;

	if (iswildcard(o))
	    break;			/* found wildcard op */
    }

    pindex += 2;			/* P-index will be two larger, due to
					** OP mask and pointer into list
					*/
    /* note that if we found no wildcard, i==index, which will result in pointer to
    ** SHNL at end
    */
    printf("(SHAPE *) 0x%lx, (SHAPE *) P(%d),\n\t\t", opmask, pindex+i);
					/* output mask and pointer, suitably cast */

    /* output the entries */

    for (i=0, count=0; i < index; i++) {
	int w = weights[i].shpno;
	if (w > nshp)
	    yyerror("internal error in outpshp():  bad shape # %d\n", w);

	printf(" S(%d)", w);
	if (wdebug)
	    printf(" /*(%d)*/", weights[i].shpwt);
	printf(", ");
	if ((++count & 07) == 0)	/* start new line after 8 on line */
	    printf("\n\t\t");
    }
    return(pindex + index);
}

#define	WILDMAGIC 10000			/* magic weight for wildcard OP node */

static int				/* return next available slot */
getm(shno, index)			/* collect shapes for pshape */
int shno;				/* current shape number */
int index;				/* place in weight array to store first
					** real shape
					*/
{
    int getw();
    int weight;
    int pshpno = shp[shno].ssh;		/* starting pshape array index */
    int s;

    while ((s = pshp[pshpno++]) >= 0) {
	weight = getw(s);		/* get this node's weight */

    /* perform magic transformation on weights of trees containing wildcard nodes */
	if (weight >= WILDMAGIC)
	    weight = WILDMAGIC - weight%WILDMAGIC;
    
	weights[index].shpno = s;	/* remember shape number and weight */
	weights[index++].shpwt = weight;
	if (index >= NWEIGHTS)
	    yyerror("pshape weight table overrun");
    }
    return( index );
}
/* calculate a shape tree's weight.  The weight is the sum of the number
** of non-WILD nodes.  WILD nodes count as WILDMAGIC.
*/

static int
getw(shno)
int shno;				/* shape number */
{
    register int weight;
    
    if (shno < 0) return( 0 );
    
    weight = getw(shp[shno].sleft) + getw(shp[shno].sright);
    if (iswildcard(shp[shno].sop))
	weight += WILDMAGIC;
    else
	weight++;
    return( weight );
}
/* comparison routine for quicksort */

static int
qcmp(i,j)
SHWT * i;
SHWT * j;
{
    return( i->shpwt - j->shpwt );
}


/* sort members of weights array */

static void
wqsort(low, high)
int low;				/* low bound to sort from */
int high;				/* high bound */
{
    void qsort();

    qsort((char *) &weights[low], (unsigned) (high-low+1), sizeof(SHWT), qcmp);
    return;
}

setshphd( i ) {
	/* set shphd[k] to 1 for every op appearing in shp[i] */
	int s;

	if( i<0 ) return;
	s = shp[i].sop;

	if( s == MANY ) {
		setshphd( shp[i].sleft );
		setshphd( shp[i].sright );
		}
	else shphd[s] = 1;
	}
/* output shape head table; convert 1, meaning OP can head shape, into
** a single bit mask for the OP.  This info gets used by the code that
** outputs the pshape array.  If we run out of bits, wrap them around
** and issue a warning.  The chief effect this has is possibly to slow
** down the compiler a bit.  Instead of recognizing cases where a shape
** is not going to match a tree except as a wildcard, we might try all
** the shapes before so concluding.  Oh well, it was just an optimization
** anyway.
**
** Beware of host/target confusions here.  The machine that sty
** runs on is presumed to be the same as the machine the built
** compiler will run on.
*/

static void
outshphd() {
    register int curbit = 0;
    int i;

    printf( "unsigned long shphd[] = {\n" );

    for (i = 0; i<DSIZE; i++) {
	if (shphd[i] != 0) {
	    shphd[i] = 1L << (curbit % HOSTPTRSIZE);
	    ++curbit;
	}
	printf("\t0x%lx,\t/* %s (%d) */\n",
	    shphd[i], (opst[i] == 0 ? "--" : opst[i]), i);

    }
    shphdbits = curbit;		/* for statistics, remember number of bits used */
    if (curbit > HOSTPTRSIZE)
	fprintf(stderr, "shphd bit masks wrap around %d:%d\n",
			shphdbits, HOSTPTRSIZE);
    printf("};\n\n");
    return;
}

static void
saaddr(sp)
register sp;
{
	if( sp < 0 ) printf( "SHNL,\t" );
	else printf( "S(%d),\t", sp );
}

int nodcnt;
int treecnt;

static void
chkdups(ii)
register ii;
{
	void shprint();
	treecnt = nodcnt = 0;
	chkdup1(ii);
	if(treecnt!=nodcnt)
	{
		fprintf(stderr,"DUPLICATE SHAPES IN TREE %s\n",shp[ii].shname);
		shprint(ii);
	}
	clrcnt(ii);
	return;
}

chkdup1(nn)
register nn;
{
	register STYSHP *p;

	p = &shp[nn];
	if((p->sleft >= 0) && (p->sop==MANY)) chkdup1(p->sleft);
	if((p->sright >= 0) && (p->sop==MANY)) chkdup1(p->sright);
	p->scnt++;
	nodcnt++;
	treecnt += p->scnt;
}

clrcnt(nn)
register nn;
{
	register STYSHP *p;

	p = &shp[nn];
	if(p->sleft >= 0) clrcnt(p->sleft);
	if(p->sright >= 0) clrcnt(p->sright);
	p->scnt = 0;
}

static void
shpprint(sha, ty)
register sha,ty;
{
	void prnam();

	if( sha < 0 ) printf( "\tPSHNL,\t");
	else printf( "\tP(%d),\t", shp[sha].ssh );
	prnam( ty, Tnam );
}

static void
prnam(ty,src)
register struct nam *src;
register ty;
{
	register int ii,jj;
	register flag = 0;
	register struct nam *tn;

	if(!ty)
	{
		printf("0x0,\n");
		return;
	}
	for(tn = src; tn->tntn; tn++)
		if( tn->tnty == ty )
		{
			printf( "%s,\n", tn->tntn );
			return;
		}
	for(ii = 0; ; ii++)
	{
		if((jj = (01<<ii)) > ty)
		{
			printf(",\n");
			return;
		}
		if(!(jj & ty))
			continue;
		for(tn = src; tn->tntn; tn++)
			if(tn->tnty==jj)
				break;
		if(flag++)
			printf("|");
		if(tn->tntn)
			printf("%s",tn->tntn);
		else
			printf("%#o",jj);
	}
}

static char	name[100];  /* had better be long enough */

yylex()
{
	register int c, i;

	for(;;)
	{

		c = getchar();
		if( c<0 ) return( -1 );
		switch( c )
		{

		case '\n':
			++lineno;
		case ' ':
		case '\b':
		case '\f':
		case '\v':
		case '\t':
			continue;

		case '<':
		case '>':
		case '+':
		case '-':
		case '=':
		case '*':
		case '%':
		case '/':
		case '&':
		case '|':
		case '^':
		case '!':
			name[0] = c;
			name[1] = getchar();
			name[2] = '\0';
			if( oplook(0) )
			{
				if( yylval.ival == LS || yylval.ival == RS )
				{
					if( (c=getchar()) == '=' ) {
						yylval.ival = ASG yylval.ival;
						c=getchar();
					}
					/* special hack for >>A (arith. shift) */
					if (    c == 'A'
					    &&  (  yylval.ival ==     RS
						|| yylval.ival == ASG RS
						)
					    )
						yylval.ival += ARS-RS;
					else ungetc( c, stdin );
				}
				return( OP );
			}
			ungetc( name[1], stdin );
			name[1] = '\0';
			if( oplook(0) ) return( OP );
			yyerror( "cannot deal with %c", name[0] );
			return( OP );

		case '~':
			yylval.ival = COMPL;
			return( OP );

		case '"':
			string = pstring;
			while( pstring < &strng[NSTRING-2] )
			{
				switch( c = getchar() ) {
				case '\t':
					*pstring++ = '\\';
					*pstring++ = 't';
					break;
				case '\n':
					*pstring++ = '\\';
					*pstring++ = 'n';
					++lineno;
					break;
				case '\\':	/* escape */
					*pstring++ = '\\';
					c = getchar();
					if( c == '\n' )
						++lineno;
					else if( c<0 )
						yyerror( "missing \"" );
					else if( isalpha(c) && isupper(c) )
						*pstring++ = '\\';
					*pstring++ = c;
					break;
				case '"':
					/* let user terminate string! */
					/* *pstring++ = '\0'; */
					goto eos;
				default:
					if( c<0 )
						yyerror( "missing \"" );
					*pstring++ = c;
				}
			}
		   eos:
			if( pstring >= &strng[NSTRING-2] )
				yyerror( "string table overflow" );
			yylval.str = string;
			return( STR );

		case '\'':
			for( i = 0;  i < sizeof name - 1;  ++i )
			{
				c = getchar();
				if( c == '\'' ) break;
				if( c == '\n' ) yyerror( "missing '" );
				name[i] = c;
			}
			name[i] = '\0';
			if( oplook(1) ) return( OP );
			yyerror( "bad op name: '%s'", name );

		case '[':
			for( i = 0;  i < sizeof name - 1;  ++i )
			{
				c = getchar();
				if( c == ']' ) break;
				if( c == '\n' ) yyerror( "missing ]" );
				name[i] = c;
			}
			name[i] = '\0';
			yylval.ival = tystr(name);
			return( STYPE );

		case '#':  /* comment */
		{
			char line[BUFSIZ];
			char *p;
			/* Look for #ident, flush to output. */
			if ((p = fgets(line, BUFSIZ, stdin)) == NULL)
			    yyerror( "unexpected EOF" );
			++lineno;
			while (*p == ' ' || *p == '\t')
			    ++p;
			if (strncmp(p, "ident", 5) != 0)
			    continue;
			p += 5;
			if (*p != ' ' && *p != '\t')
			    continue;
			/* newline is in buffer */
			printf("#ident %s", p+1);
			continue;
		}

		case '$':
			c = getchar();
			if( isdigit(c) )
			{
				yylval.ival = c-'0';
				return( RREG );
			}
			switch( c )
			{
			case '[':	return( LPRF );
			case ']':	return( RPRF );
			case '<':	return( LSHR );
			case '>':	return( RSHR );
			case 'L':	return( LRES );
			case 'R':	return( RRES );
			case 'E':	return( ERES );
			case 'P':	return( PNEED );
			case 'C':	return( GOODCC );
			case 'N':	return( NOVAL );
			case 'r':	return( RCOUNT );
			case 'l':	return( LCOUNT );
			case 'A':	yylval.ival = NRGS;
					return( DIG_ALL );
#ifdef CG
			case 'H':	return( EXHON );
			case 'I':	return( EXIG );
			case '-':
					switch(c = getchar())
					{
					case 'H':	return (NOEXHON);
					case 'I':	return (NOEXIG);
					}
					yyerror( "$-%c illegal", c );
#endif
			}
			yyerror( "$%c illegal", c );

		default:
			if( isdigit(c) )
			{
				yylval.ival = c-'0';
				return( DIG );
			}
			if( isalpha(c) )
			{
				/* collect the name */
				int i = 1;
				name[0] = c;
				while( isalpha( (c=getchar()) ) || isdigit(c) )
				{
					name[i++] = c;
				}
				name[i] = '\0';
				ungetc( c, stdin );
				return( lookup() );
			}
			return( c );
		}
	}
}

/* This table contains operators whose printstring in dope.h
** differs from the operator name that sty recognizes.
*/

struct nlist {
	char *shop;
	int vop;
} ot[] = {
	"UMINUS",	UNARY MINUS,
	"UAND",	UNARY AND,
	"COMOP",	COMOP,
#ifdef	CG
	"SEMI",	SEMI,
#endif
	"CM",		CM,
	"UCALL",	UNARY CALL,
	"USTCALL",	UNARY STCALL,
	"PDIV",		PDIV,
	"PMUL",		PMUL,
	"CON",	ICON,
	"CC",	CCODES,
	"FREE",	FREE,
	"",	-1 };

/* Look up operator.  There are several cases, depending on whether
** the operator was bracketed by quotes or not.
**
** inquote == 0:
**	1)  try opst field of dope[] only.
** inquote == 1:
**	1)  try special table first, then opst field of dope[].
*/

oplook(inquote)
int inquote;
{
	register int i;

	/* look up the first n chars of name in the above table */
	/* return 1 if it is an op, after setting yylval */
	if (inquote) {
	    for( i=0; ot[i].vop >= 0; ++i )
	    {
		    if( !strcmp( name, ot[i].shop ) )
		    {
			    yylval.ival = ot[i].vop;
			    return( 1 );
		    }
	    }
	}

	/* find op in dope[] */

	for (i = 0; indope[i].dopeop >= 0; ++i) {
	    if (strncmp(indope[i].opst, name, 8) == 0) {
		yylval.ival = indope[i].dopeop;
		return( 1 );
	    }
	}
	return( 0 );
}

/* all types we know about, except VOID */
#ifdef	CG
#define	TALL (TCHAR|TSHORT|TINT|TLONG|TUCHAR|TUSHORT|TUNSIGNED|TULONG|\
		TFLOAT|TDOUBLE|TPOINT|TPOINT2|TSTRUCT|TFPTR)
#else
#define	TALL (TCHAR|TSHORT|TINT|TLONG|TUCHAR|TUSHORT|TUNSIGNED|TULONG|\
		TFLOAT|TDOUBLE|TPOINT|TPOINT2|TSTRUCT)
#endif 	/* def CG */

tystr( p )
register char *p;	/* pointer to name */
{
	register i;
	int not = 0;			/* 1 if "not" flag on */

	/* lookup the types in name */
	if( !*p )
		return( TANY );
	else
		i = 0;
	if (*p == '!') {
	    not = 1;			/* set flag, bump pointer */
	    ++p;
	}

	for(;;)
	{
		switch( *p++ )
		{
		case '\0':
			/* invert bits if "not" set */
			return( not ? i ^ TALL : i );

		case 'c':
			i |= TCHAR;
		case ' ':
		case ',':
			continue;

		case 's':
			i |= TSHORT;
			continue;

		case 'i':
			i |= TINT;
			continue;

		case 'l':
			i |= TLONG;
			continue;

		case 'f':
			i |= TFLOAT;
			continue;

		case 'd':
			i |= TDOUBLE;
			continue;

		case 'P':
			i |= TPOINT2;
			continue;

		case 'p':
			i |= TPOINT;
			continue;

#ifdef	CG
		case 'F':
			i |= TFPTR;
			continue;
#endif

		case 't':
			i |= TSTRUCT;
			continue;

		case 'v':
			i |= TVOID;
			continue;

		case 'u':
			switch( *p )
			{
			case 'i':	i |= TUNSIGNED;  break;
			case 's':	i |= TUSHORT;  break;
			case 'c':	i |= TUCHAR;  break;
			case 'l':	i |= TULONG;  break;
			default:	yyerror( "bad u%c type", *p );
			}
			++p;
			continue;

		default:
			yyerror( "illegal type: %c", p[-1] );
		}
	}
}

struct nlist resw[] = {
	NameV(DCOST),
	NameV(SHAPES),
	NameV(OPCODES),
	NameV(USERCOST),
	NameV(CONVAL),
	NameV(NCONVAL),
	NameV(POSRANGE),
	NameV(SGRANGE),
	NameV(NONAME),
	NameV(EXCEPT),
	"",	-1,
};

lookup()
{
	/* look up the shape name in name, and return the index */
	register STYSHP *p;
	register int i;
	for( i=0; resw[i].vop >= 0; ++i )
	{
		if( !strcmp( name, resw[i].shop ) ) return( resw[i].vop );
	}
	for( i=NSTYSHPS-1, p= &shp[NSTYSHPS-1]; i>nmshp; --i,--p )
	{
		if( !strncmp( name, p->shname, NSHNAME ) )
		{
			 /* match */
			yylval.ival = i;
			return( NM );
		}
	}
	/* new entry; assume -1 will cause nmshp to be allocated */
	yylval.ival = fillshape(-1, -1, -1, 0, 0);
	if (p != &shp[yylval.ival])
	    yyerror("internal error in lookup()");
	strncpy( p->shname, name, NSHNAME );
	return( NM );
}
/*	print operator name */

static void
prop( op )
register int	op;
{
	register struct dopest	*np;
	for( np = indope;  np->dopeop >= 0;  np++ )
	{
		if( op == np->dopeop )
		{
			printf( "%s,", np->opst2 );
			return;
		}
	}
	yyerror("prop can't find OP %d", op);
}
/* print shapes, like e2print */

static void
shprint(s)
int s;					/* shape number */
{
    void sh1print();

    fprintf(dbg, "+++++++++\n");
    sh1print(s, 'T', 0);
    fprintf(dbg, "---------\n");
    return;
}

#define	TABSIZE	8			/* length of tab stop */
#define	INDENT 3			/* indentation per level */

static void
sh1print(s, let, level)
int s;
char let;
int level;
{
    STYSHP * shape = &shp[s];
    int op = shape->sop;
    int indent;
    void shspprt();

    if (shape->sright >= 0)
	sh1print(shape->sright, 'R', level+1);

    for (indent = INDENT*level; indent > TABSIZE; indent-=TABSIZE)
	putc('\t', dbg);
    while (indent-- > 0)
        putc(' ', dbg);

    fprintf(dbg, "%c.%d) ", let, s);
    if (shape->shname[0] != '\0')
	fprintf(dbg, "\"%.*s\" ", NSHNAME, shape->shname);
    fprintf(dbg, "op = %s", op >= 0 ? opst[op] : "(undefined)");

    /* check for special shapes, types */

    shspprt((unsigned long) shape->ssh); /* print special shape info, if any */

    /* print register set info, if appropriate */

    if (op == REG)
	fprintf(dbg, " { %#o }", shape->sregset);
    putc('\n', dbg);

    if (shape->sleft >= 0)
	sh1print(shape->sleft, 'L', level+1);

    return;
}
/* print special shape info */

static void
shspprt(sh)
unsigned long sh;			/* special shape info */
{
    unsigned long spval;		/* special shape value, if any */
    char * spstring;
    void shprtype();

    if ((sh & SPECIAL) == 0)
	return;
    
    if ((sh & SPTYPE) != 0) {		/* type info */
	shprtype(dbg, sh);
	return;
    }

    spval = sh & SVMASK;
    switch( sh & STMASK ) {		/* depends on which one */
    case SVAL:		spstring = "CONVAL"; break;
    case SNVAL:		spstring = "NEGVAL"; break;
    case SRANGE0:	spstring = "POSRANGE"; break;
    case SSRANGE:	spstring = "SGRANGE"; break;
    case NACON:		spstring = "NONAME"; break;
    case SUSER:		spstring = "USER"; break;
    default:
	yyerror("unknown special shape 0%o", (sh & STMASK));
	break;
    }
    
    fprintf(dbg, " %s %d", spstring, spval);
    return;
}


unsigned long typebit[] =
{
    TFLOAT,
    TDOUBLE,
    TLONG,
    TULONG,
    TINT,
    TUNSIGNED,
    TSHORT,
    TUSHORT,
    TCHAR,
    TUCHAR,
    TPOINT2,
    TPOINT,
    TSTRUCT,
    0
};

/* strings corresponding to above types */
char * typename[] =
{
    "f",
    "d",
    "l",
    "ul",
    "i",
    "ui",
    "s",
    "us",
    "c",
    "uc",
    "P",
    "p",
    "t"
};

static void
shprtype(file, type)
FILE * file;
unsigned long type;
{
    int i = 0;				/* array index */

    if (type == 0)
	return;
    
    fprintf(file, " [");

    while (typebit[i] != 0) {
	if ((type & typebit[i]) != 0)
	    fprintf(file, "%s", typename[i]);
	++i;
    }

    putc(']', file);
}
/* this routine checks for an existing instance of a shape,
** given the arguments, and returns its number.  If no
** similar shape is found, return -1
*/

static int
uniqshp(op, left, right, special, regset)
int op;					/* C-tree OP */
int left;				/* left shape */
int right;				/* right shape */
int special;				/* special shape info */
RST regset;				/* regset, if REG */
{
    int first, last;
    register int i;

    /* choose bounds for loop:  MANY nodes partitioned from the rest */

    if (op == MANY) {
	first = nmshp+1;
	last = NSTYSHPS;
    }
    else {
	first = 0;
	last = nshp;
    }

    for (i = first; i < last; ++i) {
	register STYSHP * s = &shp[i];

	if (   s->ssh == special
	    && s->sop == op
	    && (op != REG || s->sregset == regset)
	    && s->sleft == left
	    && s->sright == right
	    )
	    return( i );		/* found it */
    }
    return( -1 );			/* not found */
}
/* build a unique named shape.  We assume we're handed an otherwise
** empty new shape with a name filled in.  We want to reuse an earlier,
** existing shape, if possible.
*/

static int
buildshape(name, op, left, right, special)
int name;				/* named shape's number */
int op;					/* shape's OP */
int left;				/* left child */
int right;
int special;				/* special shape bits */
{
    /* check whether shape already exists */
    int oldshape;			/* previous equivalent shape, if any */

    if (shp[name].sop >= 0)		/* this name was previously defined */
	yyerror("redefinition of shape %.*s", NSHNAME, shp[name].shname);
    
    oldshape = uniqshp(MANY, left, right, special, 0);

    if (oldshape < 0) {			/* not found.  Make a new shape */
	shp[name].sop = op;
	shp[name].sleft = left;
	shp[name].sright = right;
	shp[name].ssh = special;
	shp[name].sregset = 0;
    }
    else {				/* matching shape found */
	if (sdebug)
	    fprintf(dbg,"buildshape:  %.*s reuses shape %d\n",
				NSHNAME, shp[name].shname, oldshape);

	if (shp[oldshape].shname[0] == 0) {

	    strncpy(shp[oldshape].shname, shp[name].shname, NSHNAME);

	    /* make node available again, if possible */
	    if (name == nmshp+1)
		++nmshp;
	    else if (name == nshp-1)
		--nshp;
	    shp[name].shname[0] = 0;	/* in any case, blast name field */
	    name = oldshape;
	}
	else {				/* put MANY above known node */
	    if (sdebug)
		fprintf(dbg, "\tmust put MANY over %.*s\n", NSHNAME,
				shp[oldshape].shname );
	    shp[name].sop = MANY;
	    shp[name].sleft = oldshape;
	    shp[name].sright = -1;
	    shp[name].ssh = special;
	}
    }
    return( name );
}
/* fillshape -- fill in data into new shape
**
** Fill in the OP, left, right, ssh, regset fields
** of a new shape.  If an equivalent shape exists,
** return it instead.  The new shape is chosen from
** nmshp if OP == MANY or OP < 0 (special for lookup()),
** or nshp, otherwise.  When OP < 0, don't check for
** uniqueness, but always allocate a new shape.
*/

static int				/* returns new shape number */
fillshape(op, left, right, special, regset)
int op;
int left;
int right;
int special;
RST regset;
{
    int newshape = (op < 0 ? -1 : uniqshp(op, left, right, special, regset));
    
    if (newshape >= 0) {
	if (sdebug)
	    fprintf(dbg, "-->>fillshape: line %d shares shape with shape %d\n",
		lineno, newshape);
    }
    else {
	/* we need a genuinely new shape */
	register STYSHP * s;

	newshape = ((op == MANY || op < 0) ? nmshp-- : nshp++);
	s = &shp[newshape];

	s->sop = op;
	s->sleft = left;
	s->sright = right;
	s->ssh = special;
	s->sregset = (op == REG ? regset : 0);
	s->shname[0] = '\0';		/* null out name field */

	/* check for overrun */

	if (nshp >= nmshp) {
	    yyerror("out of node space");
	    exit( 1 );
	}
    }
    return( newshape );
}
/* The following routines merge register sets, where possible.
** Here are some of the problems.
** 1.  We cannot change the structure of any nodes below named MANY's,
** because we have to assume they may be referred to elsewhere.
** (We could introduce ref-counts sometime later.)
** 2.  We must reduce register sets to their minima as early
** as possible to avoid combinatorial explosion in the
** distribution of OPs through MANY nodes.  Consider this example:
**	R:'REG'; r0: R{0}; r1: R{1}; r2: R{2};
**	regs: r1,r2,r3;
**	C: ;
**	oreg: *(regs+C);
** In this example, if "regs" isn't merged into a single node
** equivalent to R{0-2}, we will get three separate trees of
** the form *(rN+C), which gets expensive.
**
** The rules for register set merger are these:
** 1.  Two sets with the same .ssh field may have their register bits
** OR-ed together.
** 2.  If one set of registers (say, A) is a subset of another
** (B), and all of A's types are included in B's types, discard A.
**
**
** Assume that we are called to merge the register of an "slist".
** Assume that the register sets of shapes that are referred to by
** binary and unary OPs were already merged.  Thus the only REG nodes
** that must be merged are those ** at "top level",
** i.e., just below MANY nodes.
** The algorithm has three parts:
**	1.  Walk the shape tree, looking for REG nodes.  List them
**		in an auxiliary array.
**	2.  Calculate the minimal set of register nodes needed.
**	2a. If there are no register nodes, or the set is minimal, done.
**	3.  Walk the tree again.  Build a new copy, sharing as many nodes
**		as possible with the old tree.  Replace REG nodes in the
**		old tree with new REG nodes or -1, if all REG nodes used
**		up.
*/

#ifdef	REGSET
static int nmerge;			/* number of register nodes in list */
static int rmindex;			/* index into list */
static int rmssh[NRMERGE];		/* register node types in tree */
static RST rmrs[NRMERGE];		/* register sets for tree nodes */
static int rmreg[NRMERGE];		/* register node numbers for debug */
 /* register merge driver */

static int
mergereg(t)
int t;					/* top shape: presumed MANY with name */
{
    void mgetregs();
    int result;

    nmerge = 0;				/* zero current size of array */

    if (mdebug) {
	fprintf(dbg,"mergereg, at call:\n");
	shprint( t );
    }

    mgetregs( t );			/* get regs into array */
    /* remember max. for statistics */
    if (nmerge > nrmerge)
	nrmerge = nmerge;

    if (! mmrgregs()) {			/* figure smallest set */
	if (mdebug)
	    fprintf(dbg, "mergereg() returns with no change\n");
	return( t );			/* no REGs or no change */
    }

    rmindex = 0;			/* to fetch register info from array */
    result = mputregs( t );		/* put shape back together */

    /* Since there have been changes, move the name from the original node
    ** to a new one.  Build a tree of MANY's until we can stuff a name in.
    */
    while ( shp[result].shname[0] )
	result = fillshape( MANY, result, -1, 0, 0 );

    /* now put in name */
    (void) strncpy( shp[result].shname, shp[t].shname, NSHNAME );
    shp[t].shname[0] = '\0';		/* null out name from original */
    if (mdebug) {
	fprintf(dbg, "mergereg, after changes:\n");
	shprint( result );
    }
    return( result );
}

static void
mgetregs( shno )			/* get REG shapes into array */
int shno;				/* shape number */
{
    STYSHP * s;

    if (shno < 0)
	return;

    s = &shp[shno];

    switch (s->sop) {
    case REG:
	if (nmerge >= NRMERGE)
	    yyerror("register merge table overflow");
	else {
	    rmreg[nmerge] = shno;
	    rmssh[nmerge] = (s->ssh == 0 ? (SPTYPE|TANY) : s->ssh);
	    rmrs[nmerge++] = s->sregset;
	}
	break;
    case MANY:
	mgetregs( s->sleft );		/* do left and right */
	mgetregs( s->sright );
	break;
    /* for anything else, don't go further in shape tree */
    }
    return;
}
/* merge register set information in two phases:
** 1.  N x N pass to subsume sets with same ssh.
** 2.  Find cases where one set of registers is a subset of another,
**	try to subsume their types.
*/

static int
mmrgregs()
{
    int i,j;				/* loop indices */
    int changed = 0;			/* assume no change */

    if (nmerge <= 0)
	return( 0 );			/* obviously no change */
    
    /* Pass 1.  For current node, merge into it any register set
    ** bits for later nodes with same .ssh.  Set later node's ssh
    ** to -1 to mark as processed.
    */
    for (i = 0; i < nmerge; ++i) {
	if (rmssh[i] < 0) continue;	/* has been deleted */
	for (j = i+1; j < nmerge; ++j) {
	    if (rmssh[i] == rmssh[j]) {
		rmrs[i] |= rmrs[j];
		rmssh[j] = -1;		/* mark as processed */
		changed = 1;		/* something has changed */
		if (mdebug > 1)
		    fprintf(dbg, "pass 1:  %d subsumes %d\n", rmreg[i],
				rmreg[j]);
	    }
	}
    }
    /* Pass 2.  .ssh fields must now be unique!  Look for cases
    ** where one register set is the subset of another, delete smaller.
    */

    for (i = 0; i < nmerge; ++i) {
	RST irs, jrs;			/* register sets */

	/* can't do any merging with non-typed .ssh or already deleted */
	if (rmssh[i] < 0 || (rmssh[i] & SPTYPE) == 0) continue;
	irs = rmrs[i];			/* i's register set */

	for (j = i+1; j < nmerge; ++j) {
	    if (rmssh[j] < 0 || (rmssh[j] & SPTYPE) == 0) continue;
	    jrs = rmrs[j];		/* j's register set */

	    /* find out who's a subset of whom */

	    if (irs == jrs) {		/* if same registers, merge types */
		/* i subsumes j */
		rmssh[i] |= rmssh[j];	/* merge types */
		rmssh[j] = -1;
		changed = 1;	/* something has changed */
		if (mdebug > 1)
		    fprintf(dbg, "pass2:  %d subsumes %d\n", rmreg[i],
				    rmreg[j]);
	    }
	    else if ((irs & jrs) == jrs) {	/* j is subset of i */
		if ((rmssh[i] & rmssh[j]) == rmssh[j]) {
		    /* i subsumes j */
		    rmssh[j] = -1;
		    changed = 1;	/* something has changed */
		    if (mdebug > 1)
			fprintf(dbg, "pass2:  %d subsumes %d\n", rmreg[i],
					rmreg[j]);
		}
	    }
	    else if ((irs & jrs) == irs) {	/* i is a subset of j */
		if ((rmssh[i] & rmssh[j]) == rmssh[i]) {
		    /* j subsumes i */
		    rmssh[i] = -1;
		    changed = 1;	/* something changed */
		    if (mdebug > 1)
			fprintf(dbg, "pass2:  %d subsumes %d\n", rmreg[j],
					rmreg[i]);
		    break;		/* need to exit since i dead */
		}
	    }
	}
    }

    /* condense array, squeezing out -1's in rmssh; "i" is from, "j" is to */
    for (i=0, j=0; i < nmerge; ++i)
	if (rmssh[i] >= 0) {
	    /* move .ssh, .regset */
	    if ((rmssh[j] = rmssh[i]) == (SPTYPE | TANY))
		rmssh[j] = 0;		/* nothing special if all types */
	    rmrs[j] = rmrs[i];
	    ++j;			/* bump "to" index */
	}
    if ((nmerge = j) == 0)
	yyerror("no register sets left?");

    return( changed );
}

static int
mputregs( shno )
int shno;				/* shape number to start "copy" from */
{
    int newshape;
    int ls, rs;				/* left and right sides of MANY */

    if (shno < 0)
	return( shno );

    switch( shp[shno].sop ) {
    default:
	/* general case:  return the node we're handed */
	return( shno );
    
    case REG:
	if (rmindex >= nmerge)		/* all used up; return -1 */
	    return( -1 );
	
	newshape = fillshape(REG, -1, -1, rmssh[rmindex], rmrs[rmindex]);
	rmindex++;			/* bump index into array */
	return( newshape );		/* return new or reused REG shape */
    
    case MANY:
	/* process both sides, then figure out what to do */

	ls = mputregs( shp[shno].sleft );
	rs = mputregs( shp[shno].sright );

	if (shp[shno].sleft == ls && shp[shno].sright == rs)
	    return( shno );		/* no change */

	if (ls < 0) {
	    if (rs < 0)
		return( -1 );		/* don't need this anymore */
	    /* ls < 0, rs >= 0 */
	    ls = rs; rs = -1;		/* reverse them */
	}

	/* reduce (MANY (MANY ...) NIL) to inner MANY */

	if (rs < 0 && shp[ls].sop == MANY)
	    return( ls );
	
	/* build new shape, possibly reuse old one */
	newshape = fillshape(MANY, ls, rs, 0, 0);
	return( newshape );
    }
    /*NOTREACHED*/
}

/*
** Code to support register sets, particularly building the
** sets of bit vectors for scratch register needs.  The needs
** building is supported by the following functions:
**
**	begnrgen		prepare for needs-registers generation
**	endnrgen		finish with needs-registers generation
**	nrgen			record one register shape/# of regs pair
**
**	nrappend		saves one bit vector at end of local array
**	nrstore			saves one bit vector, radix sorted, in
**				local array
**	nrsave			save bit vector, possibly reusing existing one
**
**	rsallbits		returns OR of all scratch bits for template
**
**	outnr			outputs the information into table.c
**
** These auxiliary data structures help out:
**
**	styrsbits		array of register set bit vectors that
**				will be output; the set of bit vectors
**				for a particular register set need is
**				terminated by a 0 bit vector.
**	nrsbits			current number of entries in styrsbits[].
**	rssets			array of starting points for register
**				sets, indices in styrsbits[]
**	nrssets			number of elements in rssets[]
**	rsnset			array of register set bit vectors requested
**				in needs field
**	rsnnbit			corresponding number of registers requested
**				for each need register set
**	nrsneed			number of elements in above two arrays
**	nrsstart		starting point for current collection of bit
**				vectors for current scratch register set
**	nrspair			flag:  non-zero if registers must be allocated
**				as pairs (presumed to be adjacent bits)
**	rs_even			mask in which bits corresponding to "even"
**				registers of even/odd pair are 1
**	rs_hadall		non-zero if one of the scratch sets in the
**				template was $A
**
** At the completion of shape processing, bit vectors corresponding to
** the register sets for individual REG shapes may be added to the end
** of styrsbits[], and the .sregset field gets the bit vector index (an
** acknowledged perversion of the field's use).
*/

static RST styrsbits[NRSBITS];	/* register set bits */
static int rssets[NRSSETS];	/* register set indices */
static int nrssets = 0;		/* current number of entries in rssets[] */
static RST rsnset[NRGS];	/* needs' register set bit vectors */
static int rsnnbit[NRGS];	/* needs' corresponding numbers of bits */
static int nrsneed;		/* number of entries in two above tables */
static int nrsstart;		/* start of current register scratch set */
static int nrspair;		/* pairs-required flag */
static RST rs_even = RS_NONE;	/* bit mask of "even registers", if needed */
static int rs_hadall;		/* non-zero if $A seen in current template */

void nrstore();
void nrappend();
/* Prepare for generation of the bit vectors that correspond to scratch
** registers required by a template.
*/

static void
begnrgen()
{
#ifndef	ODDPAIR
    /* first time, build a bit-vector representing even-numbered registers */
    if (rs_even == RS_NONE) {
#ifdef RS_EVENREG
	rs_even = RS_EVENREG;	/* use implementor-supplied mask */
#else
	int regno;

	for (regno = 0; regno < TOTREGS; regno += 2)	/* even regs only */
	    rs_even |= RS_BIT(regno);
#endif /* def RS_EVENREG */
    }
#endif /* ndef ODDPAIR */

    nrsneed = 0;		/* initialize number of needs so far */
    rs_hadall = 0;		/* no $A seen yet */
    return;
}
/* Remember bit vector and number of registers to choose from it.
** If shno is < 0, there is no explicit register set shape, so use
** all scratch registers as bit vector.  Nbits < 0 is a special
** flag for $A, meaning all registers.  Reset list of needs, since
** $A overrides other things.
**
** Set "rewrite" with result bit (RESC?) if necessary.
** Update "needs" with proper number of registers.
*/

static void
nrgen( shno, nbits, isresult )
int shno;			/* shape number */
int nbits;			/* number of bits to choose from shape's
				** registers
				*/
int isresult;			/* non-zero if scratch reg is result
				** (like $2)
				*/
{
    RST regset;			/* register set bits */
    RST rstemp;			/* temporary for counting bits */
    int i;			/* likewise */

    if (nbits < 0) {		/* signal for $A */
	nrsneed = 0;		/* restart gathering array */
	rs_hadall = 1;		/* note we saw $A */
	needs |= NRGS*NREG;	/* will want all registers */
	return;
    }

    if (shno < 0)			/* get shape's register set */
	regset = RS_NRGS;		/* none specified -- use all scratch */
    else {
	/* make sure only scratch registers included */
	regset = shp[shno].sregset;
	if (regset & ~RS_NRGS)
	    yyerror("non-scratch register in scratch reg. specification");
    }

    /* Handle case where this is a result:  set "rewrite". */
    if (isresult) {
	int prevregs;			/* number of scratch regs so far */
	int resreg = nbits;		/* result register number */
	int resultonly = 0;		/* non-zero if just setting RESC? */

	if (rs_hadall)
	    prevregs = NRGS;
	else {
	    prevregs = 0;		/* current number of bits */
	    for (i = 0; i < nrsneed; ++i)
		prevregs += rsnnbit[i];	/* add 'em up from previous */
	}
	/* If $m:set $n case, assume n refers to previous set(s).
	** Otherwise adjust result bit.
	*/
	if (prevregs == 0 || shno >= 0) {
	    if (!rs_hadall)
		resreg = prevregs + nbits;
	}
	else if (resreg > prevregs)
	    yyerror("result reg number > number of scratch regs");
	else
	    resultonly = 1;		/* doing this just to set result */

	if (resreg > 3)
	    yyerror("result scratch register > 3");
	
	switch( resreg ){
	case 1:		rewrite |= RESC1; break;
	case 2:		rewrite |= RESC2; break;
	case 3:		rewrite |= RESC3; break;
	}

	if (resultonly)
	    return;			/* just setting result register */
    }

    if (!rs_hadall)
	needs += nbits*NREG;		/* bump number of needed registers */

	    
    /* count bits to verify that the shape really has enough */

    for (i = 0, rstemp = regset; rstemp != 0; ++i)
	rstemp -= RS_CHOOSE(rstemp); /* remove one bit at a time */
    
    if (nbits > i)
	yyerror("more scratch regs requested than available");
    
    /* Add register set to list. */
    rsnset[nrsneed] = regset;
    rsnnbit[nrsneed++] = nbits;
    return;
}
/* Finished with the needs for a template.  Actually generate the
** scratch register bit vectors (at the current end of the styrsbits[]
** array).  See if this set is a duplicate of some other set.  Finally,
** return the index in the styrsbits[] array for the current set of
** scratch registers.
**
** The new set of bit vectors gets put at the current end of the
** styrsbits[] array.  Then we search for any matching set.  If
** we find a match, we return its beginning index.  Otherwise we
** return the beginning of the current set.
*/

static int
endnrgen( pairs)
int pairs;				/* non-zero if register pairs required */
{
    void bchoose();
    int i;				/* loop index */
    RST allbits;

    /* If $A was set, must append to list a request for m registers from
    ** RS_NRGS, where m = NRGS - (number of registers explicitly requested)
    */
    if (rs_hadall) {
	int totregs = 0;		/* number of explicit registers */

	for (i = 0; i < nrsneed; ++i)
	    totregs += rsnnbit[i];
	
	if (totregs < NRGS)
	    nrgen( -1, NRGS-totregs, 0 );
    }

    if (nrsneed <= 0)			/* no scratch registers needed */
	return( 0 );

    /* Start appending to array of bit vectors. */

    nrappend( nrsneed );		/* first word is number of vector words */
    /* OR together all scratch bits */
    for (allbits = RS_NONE, i = 0; i < nrsneed; ++i)
	allbits |= rsnset[i];
    nrappend( allbits );

    nrsstart = nrsbits;			/* current starting point */
    nrspair = pairs;			/* set pairs flag */

    rsnnbit[nrsneed] = 0;		/* mark end of needs */
    bchoose( 0, RS_NONE );		/* actually generate the bit vectors */
    if (nrsbits == nrsstart)		/* didn't find any sets! */
	yyerror("impossible scratch register requirement");
    nrappend( 0 );			/* terminate each list thusly */

    /* look for a matching set of bits */

    for (i = 0; i < nrssets; ++i) {	/* try each known set starting point */
	int old = rssets[i];
	int new = nrsstart-2;

	for ( ; styrsbits[old] == styrsbits[new]; ++old, ++new ) {
	    if (styrsbits[old] == 0) {	/* found an existing match */
		nrsbits = nrsstart-2;	/* reset number of bit vectors, since
					** we don't need them.
					*/
		return( rssets[i] );	/* return its starting point */
	    }
	}
    }

    /* no matching set of bit vectors found */

    if (nrssets >= NRSSETS)		/* too many sets already? */
	yyerror("too many register sets");
    
    rssets[nrssets++] = nrsstart-2;	/* beginning of current set */
    return( nrsstart-2 );		/* return start of vectors */
}

#define rightbit( word ) (word & ~(word-1))

/* Choose all combinations of n bits from a bit vector
** Local arrays are used to store current context.
** The algorithm works in these steps:
**	1)  Fill arrays for current level.
**	2)  Working backward, exhaust all bits at level n,
**		then refill arrays from current level to level n.
*/

static RST barray[NRGS+1];			/* barray[i] contains current bit vector
					** for index value i
					*/

static void
bchoose( index, sofar )
int index;				/* index in rsnset, rsnnbit at current
					** level
					*/
RST sofar;				/* bits already generated previously */
{
    /* assume maximum of 9 scratch bits at a time desired */
    RST curbits[NRGS+1];		/* current bits per level */
    RST curmask[NRGS+1];		/* current OR of previous right bits */
    RST bits;				/* bits from which to generate current level */
    int n = rsnnbit[index];		/* number of bits needed at current level */

    register int curlev = 1;		/* current level working on locally */

    if (n == 0) {			/* done all needs bits */
	nrstore(barray, index);
	return;
    }

    /* get current working bits:  bits desired in current need set that
    ** haven't been used already in a prior set
    */
    bits = rsnset[index] & ~sofar;
    if (!bits)
	return;				/* no bits left to work with */

    curmask[0] = 0;			/* OR of predecessor bits */
    curbits[1] = bits;			/* to start things off */
    
    do {
	/* refill arrays */
	for ( curlev; curlev < n; ++curlev) {
	    RST rbit;			/* current right-most bit */
	    RST allbits;		/* all bits for this level */

	    /* get pairs or single bits */
	    rbit = rightbit( curbits[curlev] );
	    if (rbit == 0) break;

	    if (! nrspair)
		allbits = rbit;
	    else {			/* register pairs */
		for( ; (rbit = rightbit( curbits[curlev] )) != 0; curbits[curlev] -= rbit) {
#ifndef	ODDPAIR
		    /* for even/odd pair, first register must be one of even set */
		    if ((rbit & rs_even) == 0) continue;
#endif
		    /* check adjacent bit */
		    allbits = rbit | RS_PAIR(rbit);
		    if (curbits[curlev] & RS_PAIR(rbit))
			break;		/* neighboring bit is also 1 */
		}

		if (rbit == 0) break;	/* must go up a level */
	    }

	    /* the next level gets our current bits, less the ones we just chose;
	    ** then our level loses just the right bit
	    */
	    /* rbit contains right bit for this level */
	    curbits[curlev+1] = curbits[curlev] - allbits;
	    curbits[curlev] -= rbit;	/* remove bit at current level */
	    curmask[curlev] = curmask[curlev-1] | allbits;
	}

	if ( curlev == n ) {		/* did complete previous loop */
	    /* filled all array entries; generate bits */
	    while (curbits[n] != 0) {
		RST nextbit = rightbit( curbits[n] );

		curbits[n] -= nextbit;
		if (nrspair) {
		    /* want two adjacent bits */
		    nextbit |= RS_PAIR(nextbit);
		    if ((curbits[n] & nextbit) == 0)
			continue;	/* didn't have them both; try again */
		}
		/* remember current bit vector for this set of needs, call
		** recursively for next-level needs
		*/
		barray[index] = nextbit | curmask[n-1];
		bchoose( index+1, sofar|barray[index] );
	    }
	}

	/* find nearest previous non-zero set of bits; curlev is n
	** if we filled the bits array or is the level where we
	** found a zero curbit last
	*/
	while (--curlev > 0) {
	    if (curbits[curlev] != 0)
		break;
	}
    } while (curlev > 0);
    return;
}

/* append bit pattern to end of current bit vector list */

static void
nrappend( bits )
RST bits;
{
    if (nrsbits >= NRSBITS)
	yyerror("too many register set bit vectors (nrappend)");
    
    styrsbits[nrsbits++] = bits;
    return;
}

/* This routine stores a sequence of bit vectors in the proper place
** in the temporary set of same.  We use a radix sort on the OR of all
** of the bit vectors for a particular need-set.  The vectors in the
** array are headed by the OR, but the argument is not.
*/

static void
nrstore( barray, nwords )
RST barray[];				/* array of needs bits */
int nwords;				/* there are nwords of them */
{
    int i, j;
    RST allbits = RS_NONE;		/* figure out OR of all bits */

    for (i = 0; i < nwords; ++i)
	allbits |= barray[i];
    
    if (nwords > 1)
	++nwords;			/* for everything else, we want to count
					** the bit vectors PLUS their OR
					*/

    if (nrsbits+nwords >= NRSBITS)
	yyerror("too many register set bit vectors");

    /* Do the sort.  If we find the same ORed bit pattern, we have a duplicate
    ** (at least as far as we're concerned), and we discard it.
    */

    for (i = nrsstart; i < nrsbits; i += nwords) {
	RST temp;

	if (allbits == styrsbits[i])	/* duplicate? */
	    return;
	
	temp = allbits ^ styrsbits[i];
	temp = rightbit( temp );	/* beware of double evaluation in rb() */
	if ((temp & allbits) != 0)	/* allbits "<" current table entry */
	    break;
    }

    /* i now points at merged vector word of entry sitting where we want
    ** to put the new set of vectors
    */

    for (j = nrsbits-1; j >= i+nwords; --j)
	styrsbits[j] = styrsbits[j-nwords];	/* move things up */
    
    /* put in new entry */
    if (nwords > 1) {
	for (j = 0; j < nwords; ++j)
	    styrsbits[i+j+1] = barray[j];
    }
    
    /* stick in ORed bits */
    styrsbits[i] = allbits;

    nrsbits += nwords;			/* bump current array extent */
    return;
}
/* save bit pattern in register set table */

static int
nrsave( bits )
RST bits;
{
    register int i;

    for (i = 0; i < nrsbits; ++i)	/* try to find existing copy */
	if (styrsbits[i] == bits)
	    return( i );		/* can use existing table entry */

    /* must add new table entry */

    if (nrsbits >= NRSBITS)
	yyerror("register set bit vector overflow");
    
    styrsbits[nrsbits] = bits;
    return( nrsbits++ );
}
/* return the complete set of scratch bits, given a template */

static RST
rsallbits(tmplt)
OPTB * tmplt;
{
    return(tmplt->rneeds ? styrsbits[tmplt->rneeds+1] : RS_NONE);
}
/* output register set bit vectors */

static void
outnr()
{
    int i;

    printf("\n\nRST rsbits[] = {");

    for (i = 0; i < nrsbits; ++i) {
	if (!(i & 7))
	    printf("\n/* %d */", i);
	printf("\t%#o,", styrsbits[i]);	/* expect small bit vectors! */
    }

    printf("\n};\n\n");

    /* Also, output bit vector of even-numbered registers, if needed */

#if !defined(ODDPAIR) && !defined(RS_EVENREG)
    printf("RST rs_evenreg = %#o;\n\n", rs_even);
#endif

    return;
}

#else	/* def REGSET */
static void
begnrgen() {}

static int
endnrgen(a) {return 0;}

static void
nrgen(a,b,c) { }

#endif	/* def REGSET */
#ifdef CG

int
can_except(op)
int op;
{
	switch(op)
	{
	case UNARY MINUS:
	case CONV:
	case PLUS:
	case ASG PLUS:
	case MINUS:
	case ASG MINUS:
	case MUL:
	case ASG MUL:
	case DIV:
	case ASG DIV:
	case LS:
	case ASG LS:
	case INCR:
	case DECR:
	case CALL:
	case UNARY CALL:
	case STCALL:
	case UNARY STCALL:
	case ALLOC:
	case CAPCALL:
	case VLRETURN:
	case RSAVE:
		return 1;
	default:
		return 0;
	}
}
#endif 	/*CG*/
