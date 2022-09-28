/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)sdb:cfront/src/gram.y	1.2"
/*ident	"@(#)cfront:src/gram.y	1.12" */
/*************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


gram.y:
	
	This is the C++ syntax analyser.

	Syntax extensions for error handling:
		nested functions
		any expression can be empty
		any expression can be a constant_expression

	A call to error() does not change the parser's state

***************************************************************************/

#include "cfront.h"
#include "size.h"

#define YYMAXDEPTH 300

static cdi = 0;
static Pnlist cd = 0, cd_vec[BLMAX];
static char stmt_seen = 0, stmt_vec[BLMAX];
static Plist tn_vec[BLMAX];

void sig_name(Pname);	// fcts put into norm2.c just to get them out of gram.y
Ptype tok_to_type(TOK);
void memptrdcl(Pname, Pname, Ptype, Pname);

#define lex_unget(x) back = x

#define Ndata(a,b)	b->normalize(Pbase(a),0,0)
#define Ncast(a,b)	b->normalize(Pbase(a),0,1)
#define Nfct(a,b,c)	b->normalize(Pbase(a),Pblock(c),0)
#define Ncopy(n)	(n->base==TNAME)?new name(n->string):n

#define Finit(p)	Pfct(p)->f_init
#define Fargdcl(p,q,r)	Pfct(p)->argdcl(q,r)
#define Freturns(p)	Pfct(p)->returns
#define Vtype(v)	Pvec(v)->typ
#define Ptyp(p)		Pptr(p)->typ

		/* avoid redefinitions */
#undef EOFTOK
#undef ASM
#undef BREAK
#undef CASE
#undef CONTINUE
#undef DEFAULT
#undef DELETE
#undef DO
#undef ELSE
#undef ENUM
#undef FOR
#undef FORTRAN
#undef GOTO
#undef IF
#undef NEW
#undef OPERATOR
#undef RETURN
#undef SIZEOF
#undef SWITCH
#undef THIS
#undef WHILE
#undef LP
#undef RP
#undef LB
#undef RB
#undef REF
#undef DOT	
#undef NOT	
#undef COMPL	
#undef MUL	
#undef AND	
#undef PLUS	
#undef MINUS	
#undef ER	
#undef OR	
#undef ANDAND
#undef OROR
#undef QUEST
#undef COLON
#undef ASSIGN
#undef CM
#undef SM	
#undef LC	
#undef RC
#undef ID
#undef STRING
#undef ICON
#undef FCON	
#undef CCON	
#undef ZERO
#undef ASOP
#undef RELOP
#undef EQUOP
#undef DIVOP
#undef SHIFTOP
#undef ICOP
#undef TYPE
#undef TNAME
#undef EMPTY
#undef NO_ID
#undef NO_EXPR
#undef ELLIPSIS
#undef AGGR
#undef MEM
#undef CAST
#undef ENDCAST
#undef MEMPTR
#undef PR
%}

%union {
	char*	s;
	TOK	t;
	int	i;
	loc	l;
	Pname	pn;
	Ptype	pt;
	Pexpr	pe;
	Pstmt	ps;
	Pbase	pb;
	Pnlist	nl;
	Pslist	sl;
	Pelist	el;
	PP	p;	// fudge: pointer to all class node objects
}
%{
extern YYSTYPE yylval, yyval;
extern int yyparse();

Pname syn()
{
ll:
	switch (yyparse()) {
	case 0:		return 0;	// EOF
	case 1:		goto ll;	// no action needed
	default:	return yyval.pn;
	}
}

void look_for_hidden(Pname n, Pname nn)
{
	Pname nx = ktbl->look(n->string,HIDDEN);
	if (nx == 0) error("nonTN%n before ::%n",n,nn);
	nn->n_qualifier = nx;
}
%}
/*
	the token definitions are copied from token.h,
	and all %token replaced by %token
*/
			/* keywords in alphabetical order */
%token EOFTOK		0
%token ASM		1
%token BREAK		3
%token CASE		4
%token CONTINUE		7
%token DEFAULT		8
%token DELETE		9
%token DO		10
%token ELSE		12
%token ENUM		13
%token FOR		16
%token FORTRAN		17
%token GOTO		19
%token IF		20
%token NEW		23
%token OPERATOR		24
%token RETURN		28
%token SIZEOF		30
%token SWITCH		33
%token THIS		34
%token WHILE		39

			/* operators in priority order (sort of) */
%token LP		40
%token RP		41
%token LB		42
%token RB		43
%token REF		44
%token DOT		45
%token NOT		46
%token COMPL		47
%token MUL		50
%token AND		52
%token PLUS		54
%token MINUS		55
%token ER		64
%token OR		65
%token ANDAND		66
%token OROR		67
%token QUEST		68
%token COLON		69
%token ASSIGN		70
%token CM		71
%token SM		72
%token LC		73
%token RC		74
%token CAST		113
%token ENDCAST		122
%token MEMPTR		173

			/* constants etc. */
%token ID		80
%token STRING		81
%token ICON		82
%token FCON		83
%token CCON		84

%token ZERO		86

			/* groups of tokens */
%token ASOP		90	/* op= */
%token RELOP		91	/* LE GE LT GT */
%token EQUOP		92	/* EQ NE */
%token DIVOP		93	/* DIV MOD */
%token SHIFTOP		94	/* LS RS */
%token ICOP		95	/* INCR DECR */

%token TYPE		97	/*	INT FLOAT CHAR DOUBLE
					REGISTER STATIC EXTERN AUTO
					CONST INLINE VIRTUAL FRIEND
					LONG SHORT UNSIGNED
					TYPEDEF */
%token TNAME		123
%token EMPTY		124
%token NO_ID		125
%token NO_EXPR		126
%token ELLIPSIS		155	/* ... */
%token AGGR		156	/* CLASS STRUCT UNION */
%token MEM		160	/* :: */
%token PR		175	/* PUBLIC PRIVATE PROTECTED */


%type <p>	external_def fct_dcl fct_def att_fct_def arg_dcl_list 
		base_init init_list binit
		data_dcl ext_def vec ptr
		type tp enum_dcl moe_list
		moe 
		tag class_head class_dcl cl_mem_list 
		cl_mem dl decl_list 
		fname decl initializer stmt_list
		block statement simple ex_list elist e  term prim
		cast_decl cast_type c_decl c_type c_tp
		arg_decl at arg_type arg_list arg_type_list
		new_decl new_type
		condition
		TNAME tn_list MEMPTR
%type <l>	LC RC SWITCH CASE DEFAULT FOR IF DO WHILE GOTO RETURN DELETE
		BREAK CONTINUE
%type <t>	oper
		EQUOP DIVOP SHIFTOP ICOP RELOP ASOP
		ANDAND OROR PLUS MINUS MUL ASSIGN OR ER AND 
		LP LB NOT COMPL AGGR
		TYPE PR
%type <s>	CCON ZERO ICON FCON STRING
%type <pn>	ID 

%left	EMPTY
%left	NO_ID
%left	RC LC ID BREAK CONTINUE RETURN GOTO DELETE DO IF WHILE FOR CASE DEFAULT
	AGGR ENUM TYPE
%left	NO_EXPR

%left	CM
%right	ASOP ASSIGN
%right	QUEST COLON
%left	OROR
%left	ANDAND
%left	OR
%left	ER
%left	AND
%left	EQUOP
%left	RELOP
%left	SHIFTOP
%left	PLUS MINUS
%left	MUL DIVOP MEMPTR
%right	NOT COMPL NEW
%right	CAST ICOP SIZEOF
%left	LB LP DOT REF MEM

%start ext_def

%%
/*
	this parser handles declarations one by one,
	NOT a complete .c file
*/






/************** DECLARATIONS in the outermost scope: returns Pname (in yylval) ***/

ext_def		:  external_def		{	return 2; }
		|  SM			{	return 1; }
		|  EOFTOK		{	return 0; }
		;

external_def	:  data_dcl
			{	modified_tn = 0; if ($<pn>1==0) $<i>$ = 1; }
		|  att_fct_def
			{	goto mod; }
		|  fct_def
			{	goto mod; }
		|  fct_dcl
			{ mod:	if (modified_tn) {
					restore();
					modified_tn = 0;
				}
			}
		|  ASM LP STRING RP SM
			{	Pname n = new name(make_name('A'));
				n->tp = new basetype(ASM,0);
				Pbase(n->tp)->b_name = Pname($<s>3);
				$$ = n;
			}
		;

fct_dcl		:  decl ASSIGN initializer SM
			{	error('s',"T ofIdE too complicated (useTdef or leave out theIr)");
				goto fix;
			}
		|  decl SM
			{	Pname n;
				Ptype t;
			fix:
				if ((n=$<pn>1) == 0) {
					error("syntax error:TX");
					$$ = Ndata(defa_type,n);
				}
				else if ((t=n->tp) == 0) {
					error("TX for%n",n);
					$$ = Ndata(defa_type,n);
				}
				else if (t->base==FCT) {
					if (Pfct(t)->returns==0)
						$$ = Nfct(defa_type,n,0);
					else
						$$ = Ndata(0,n);
				}
				else {
					error("syntax error:TX for%k%n",t->base,n);
					$$ = Ndata(defa_type,n);
				}
			}
		;


att_fct_def	:  type decl arg_dcl_list base_init block
			{	Pname n = Nfct($1,$<pn>2,$5);
				Fargdcl(n->tp,name_unlist($<nl>3),n);
				Finit(n->tp) = $<pn>4;
				$$ = n;
			} 
		;

fct_def		:  decl arg_dcl_list base_init block
			{	Pname n = Nfct(defa_type,$<pn>1,$4);
				Fargdcl(n->tp,name_unlist($<nl>2),n);
				Finit(n->tp) = $<pn>3;
				$$ = n;
			}
		;

base_init	:  COLON init_list
			{	$$ = $2; }
		|  %prec EMPTY
			{	$$ = 0; }
		;

init_list	:  binit
		|  init_list CM binit
			{	$<pn>$ = $<pn>3;  $<pn>$->n_list = $<pn>1; }
		;

binit		:  LP elist RP
			{	$<pn>$ = new name;
				$<pn>$->n_initializer = $<pe>2;
			}
		|  ID LP elist RP
			{	$<pn>$ = $1;
				$<pn>$->n_initializer = $<pe>3;
			}
		;




/*************** declarations: returns Pname ********************/

arg_dcl_list	:  arg_dcl_list data_dcl
			{	if ($<pn>2 == 0)
					error("badAD");
				else if ($<pn>2->tp->base == FCT)
					error("FD inAL (%n)",$<pn>2);
				else if ($1)
					$<nl>1->add($<pn>2);
				else
					$<nl>$ = new nlist($<pn>2);
			}
		|  %prec EMPTY
			{	$$ = 0; }
		;

dl		:  decl
		|  ID COLON e		%prec CM
			{	$$ = $<pn>1;
				$<pn>$->tp = new basetype(FIELD,$<pn>3);
		 	}
		|  COLON e		%prec CM
			{	$$ = new name;
				$<pn>$->tp = new basetype(FIELD,$<pn>2);
			}
		|  decl ASSIGN initializer
			{	Pexpr e = $<pe>3; 
				if (e == dummy) error("emptyIr");
				$<pn>1->n_initializer = e; 
			}
		;

decl_list	:  dl
			{	if ($1) $<nl>$ = new nlist($<pn>1); }
		|  decl_list CM dl
			{	if ($1)
					if ($3)
						$<nl>1->add($<pn>3);
					else
						error("DL syntax");
				else {
					if ($3) $<nl>$ = new nlist($<pn>3);
					error("DL syntax");
				}
			}
		;

data_dcl	:  type decl_list SM	{ $$ = Ndata($1,name_unlist($<nl>2)); }
		|  type SM		{ $$ = $<pb>1->aggr(); }
		
		;

tp		:  TYPE			{ $$ = new basetype($<t>1,0); }
		|  TNAME		{ $$ = new basetype(TYPE,$<pn>1); }
		|  class_dcl
		|  enum_dcl
		;

type		:  tp
		|  type TYPE		{ $$ = $<pb>1->type_adj($<t>2); }
		|  type TNAME		{ $$ = $<pb>1->name_adj($<pn>2); }
		|  type class_dcl	{ $$ = $<pb>1->base_adj($<pb>2); }
		|  type enum_dcl	{ $$ = $<pb>1->base_adj($<pb>2); }
		;




/***************** aggregate: returns Pname *****************/


enum_dcl	:  ENUM LC moe_list RC		{ $$ = end_enum(0,$<pn>3); }
		|  ENUM tag LC moe_list RC	{ $$ = end_enum($<pn>2,$<pn>4); }
		;

moe_list	:  moe
			{	if ($1) $<nl>$ = new nlist($<pn>1); }
		|  moe_list CM moe
			{	if( $3)
					if ($1)
						$<nl>1->add($<pn>3);
					else
						$<nl>$ = new nlist($<pn>3);
			}
		;

moe		:  ID
			{	$$ = $<pn>1; $<pn>$->tp = moe_type; }
		|  ID ASSIGN e
			{	$$ = $<pn>1;
				$<pn>$->tp = moe_type;
				$<pn>$->n_initializer = $<pe>3;
			}
		|  /* empty: handle trailing CM: enum e { a,b, }; */
			{	$$ = 0; }
		;


class_dcl	:  class_head cl_mem_list RC
			{	
				ccl->mem_list = name_unlist($<nl>2);
				end_cl();
			}
		|  class_head cl_mem_list RC TYPE
			{	
				ccl->mem_list = name_unlist($<nl>2);
				end_cl();
				error("`;' or declaratorX afterCD");
				lex_unget($4);
				/* lex_unget($4); but only one unget, sorry */
			}
		;

class_head	:  AGGR LC
			{	$$ = start_cl($<t>1,0,0); }
		|  AGGR tag LC
			{	$$ = start_cl($<t>1,$<pn>2,0); }
		|  AGGR tag COLON TNAME LC
			{	$$ = start_cl($<t>1,$<pn>2,$<pn>4);
				if ($<t>1 == STRUCT) ccl->pubbase = 1;
			}
		|  AGGR tag COLON PR TNAME LC
			{	
				$$ = start_cl($<t>1,$<pn>2,$<pn>5);
				ccl->pubbase = 1;
			}
/*
		|  AGGR tag COLON TNAME TNAME LC
			{	char* s = $<pn>4->string;
				if (strcmp(s,"public")) error("unX %s after : inCD",s);
				$$ = start_cl($<t>1,$<pn>2,$<pn>5);
				ccl->pubbase = 1;
			}
		|  AGGR tag COLON TNAME ID LC
			{	char* s = $<pn>4->string;
				if (strcmp(s,"public")) error("unX %s after : inCD",s);
				$$ = start_cl($<t>1,$<pn>2,$<pn>5);
				ccl->pubbase = 1;
			}
		|  AGGR tag COLON ID LC
			{	$$ = start_cl($<t>1,$<pn>2,$<pn>4);
				if ($<t>1 == STRUCT) ccl->pubbase = 1;
			}
*/
		;

tag		:  ID
			{ $$ = $1; }
		|  TNAME
		;

cl_mem_list	:  cl_mem_list cl_mem
			{	if ($2) {
					if ($1)
						$<nl>1->add_list($<pn>2);
					else
						$<nl>$ = new nlist($<pn>2);
				}
			}
		|  %prec EMPTY
			{	$$ = 0; }
		;

cl_mem		:  data_dcl
		|  att_fct_def SM
		|  att_fct_def
		|  fct_def SM
		|  fct_def
		|  fct_dcl
		|  PR COLON	{	$$ = new name; $<pn>$->base = $<t>1; }
		|  tn_list tag SM 
			{	Pname n = Ncopy($<pn>2);
				n->n_qualifier = $<pn>1;
				n->base = PUBLIC;
				$$ = n;
			}
		;



/************* declarators:	returns Pname **********************/

/*	a ``decl'' is used for function and data declarations,
		and for member declarations
		(it has a name)
	an ``arg_decl'' is used for argument declarations
		(it may or may not have a name)
	an ``cast_decl'' is used for casts
		(it does not have a name)
	a ``new_decl'' is used for type specifiers for the NEW operator
		(it does not have a name, and PtoF and PtoV cannot be expressed)
*/

fname		:  ID
			{	$$ = $<pn>1; }
		|  COMPL TNAME
			{	$$ = Ncopy($<pn>2);
				$<pn>$->n_oper = DTOR;
			}
		|  OPERATOR oper
			{	$$ = new name(oper_name($2));
				$<pn>$->n_oper = $<t>2;
			}
		|  OPERATOR c_type
			{	Pname n = $<pn>2;
				n->string = "_type";
				n->n_oper = TYPE;
				n->n_initializer = (Pexpr)n->tp;
				n->tp = 0;
				$$ = n;
			}
		;

oper		:  PLUS
		|  MINUS
		|  MUL
		|  AND
		|  OR
		|  ER
		|  SHIFTOP
		|  EQUOP
		|  DIVOP
		|  RELOP
		|  ANDAND
		|  OROR
		|  LP RP	{	$$ = CALL; }
		|  LB RB	{	$$ = DEREF; }
		|  NOT
		|  COMPL
		|  ICOP
		|  ASOP
		|  ASSIGN
		|  NEW		{	$$ = NEW; }
		|  DELETE	{	$$ = DELETE; }
		|  REF		{	$$ = REF; }
		|  DOT		{	$$ = DOT; }
		;

tn_list		:  TNAME DOT
		   { 
	             if ( $<pn>1->tp->base != COBJ )
			error( "T of%n not aC", $<pn>1 );	
		   }
		|  TNAME MEM
		   { 
	             if ( $<pn>1->tp->base != COBJ )
			error( "T of%n not aC", $<pn>1 );	
		   }
		|  tn_list TNAME DOT	{ error("CNs do not nest"); }
		|  tn_list ID DOT	{ error("CNs do not nest"); }
		;


decl		:  decl arg_list
			{	Freturns($2) = $<pn>1->tp;
				$<pn>1->tp = $<pt>2;
			}
		|  TNAME arg_list
			{	Pname n = $<pn>1;
				$$ = Ncopy(n);
				if (ccl && strcmp(n->string,ccl->string)) n->hide();
				$<pn>$->n_oper = TNAME;
				Freturns($2) = $<pn>$->tp;
				$<pn>$->tp = $<pt>2;
			}
		|  decl LP elist RP
			/*	may be class object initializer,
				class object vector initializer,
				if not elist will be a CM or an ID
			*/
			{	TOK k = 1;
				Pname l = $<pn>3;
				if (fct_void && l==0) k = 0;
				$<pn>1->tp = new fct($<pn>1->tp,l,k);
			}
		|  TNAME LP elist RP
			{	TOK k = 1;
				Pname l = $<pn>3;
				if (fct_void && l==0) k = 0;
				$$ = Ncopy($<pn>1);
				$<pn>$->n_oper = TNAME;
				$<pn>$->tp = new fct(0,l,k);
			} 
		|  TNAME LP MEMPTR decl RP arg_list
			{	memptrdcl($<pn>3,$<pn>1,$<pt>6,$<pn>4);
				$$ = $4;
			}
		|  fname
		|  ID DOT fname
			{	$$ = Ncopy($<pn>3);
				$<pn>$->n_qualifier = $1;
			}
		|  tn_list fname
			{	$$ = $2;
				set_scope($<pn>1);
				$<pn>$->n_qualifier = $<pn>1;
			}
		|  tn_list TNAME
			{	$$ = Ncopy($<pn>2);
				set_scope($<pn>1);
				$<pn>$->n_oper = TNAME;
				$<pn>$->n_qualifier = $<pn>1;
			}
		|  ptr decl	%prec MUL
			{	Ptyp($1) = $<pn>2->tp;
				$<pn>2->tp = $<pt>1;
				$$ = $2;
			}
		|  ptr TNAME	%prec MUL
			{	$$ = Ncopy($<pn>2);
				$<pn>$->n_oper = TNAME;
				$<pn>2->hide();
				$<pn>$->tp = $<pt>1;
			}
		|  TNAME vec	%prec LB
			{	$$ = Ncopy($<pn>1);
				$<pn>$->n_oper = TNAME;
				$<pn>1->hide();
				$<pn>$->tp = $<pt>2;
			}
		|  decl vec	%prec LB	
			{	Vtype($2) = $<pn>1->tp;
				$<pn>1->tp = $<pt>2;
			}
		|  LP decl RP arg_list
			{	Freturns($4) = $<pn>2->tp;
				$<pn>2->tp = $<pt>4;
				$$ = $2;
			}
		|  LP decl RP vec
			{	Vtype($4) = $<pn>2->tp;
				$<pn>2->tp = $<pt>4;
				$$ = $2;
			}
		;

arg_decl	:  ID
			{	$$ = $<pn>1; }
		|  ptr TNAME	%prec MUL
			{	$$ = Ncopy($<pn>2);
				$<pn>$->n_oper = TNAME;
				$<pn>2->hide();
				$<pn>$->tp = $<pt>1;
			}
		|  %prec NO_ID
			{	$$ = new name; }
		|  ptr arg_decl		%prec MUL
			{	Ptyp($1) = $<pn>2->tp;
				$<pn>2->tp = (Ptype)$1;
				$$ = $2;
			}
		|  arg_decl vec		%prec LB
			{	Vtype($2) = $<pn>1->tp;
				$<pn>1->tp = (Ptype)$2;
			}
		|  LP arg_decl RP arg_list
			{	Freturns($4) = $<pn>2->tp;
				$<pn>2->tp = (Ptype)$4;
				$$ = $2;
			}
		|  LP arg_decl RP vec
			{	Vtype($4) = $<pn>2->tp;
				$<pn>2->tp = (Ptype)$4;
				$$ = $2;
			}
		;

new_decl	:  %prec NO_ID
			{	$$ = new name; }
		|  ptr new_decl		%prec MUL
			{	Ptyp($1) = $<pn>2->tp;
				$<pn>2->tp = (Ptype)$1;
				$$ = $2;
			}
		|  new_decl vec		%prec LB
			{	Vtype($2) = $<pn>1->tp;
				$<pn>1->tp = (Ptype)$2;
			}
		;

cast_decl	:  %prec NO_ID
			{	$$ = new name; }
		|  ptr cast_decl			%prec MUL
			{	Ptyp($1) = $<pn>2->tp;
				$<pn>2->tp = (Ptype)$1;
				$$ = $2;
			}
		|  cast_decl vec			%prec LB
			{	Vtype($2) = $<pn>1->tp;
				$<pn>1->tp = (Ptype)$2;
			}
		|  LP cast_decl RP arg_list
			{	Freturns($4) = $<pn>2->tp;
				$<pn>2->tp = $<pt>4;
				$$ = $2;
			}
		|  LP cast_decl RP vec
			{	Vtype($4) = $<pn>2->tp;
				$<pn>2->tp = $<pt>4;
				$$ = $2;
			}
		;

c_decl		:  %prec NO_ID
			{	$$ = new name; }
		|  ptr c_decl				%prec MUL
			{	Ptyp($1) = $<pn>2->tp;
				$<pn>2->tp = (Ptype)$1;
				$$ = $2;
			}
		;



/***************** statements: returns Pstmt *****************/

stmt_list	:  stmt_list statement
			{	if ($2)
					if ($1)
						$<sl>1->add($<ps>2);
					else {
						$<sl>$ = new slist($<ps>2);
						stmt_seen = 1;
					}
			}
		|  statement
			{	if ($1) {
					$<sl>$ = new slist($<ps>1);
					stmt_seen = 1;
				}
			}
		;

condition	:  LP e RP
			{	$$ = $2;
				if ($<pe>$ == dummy) error("empty condition");
				stmt_seen = 1;
			}
		;

block		:  LC
			{	cd_vec[cdi] = cd;
				stmt_vec[cdi] = stmt_seen;
				tn_vec[cdi++] = modified_tn;
				cd = 0;
				stmt_seen = 0;
				modified_tn = 0;
			}
			stmt_list RC
			{	Pname n = name_unlist(cd);
				Pstmt ss = stmt_unlist($<sl>3);
				$$ = new block($<l>1,n,ss);
				if (modified_tn) restore();
				cd = cd_vec[--cdi];
				stmt_seen = stmt_vec[cdi];
				modified_tn = tn_vec[cdi];
				if (cdi < 0) error('i',"block level(%d)",cdi);
			}
		|  LC RC
			{	$$ = new block($<l>1,0,0); }
		|  LC error RC
			{	$$ = new block($<l>1,0,0); }
		;

simple		:  e
			{	$$ = new estmt(SM,curloc,$<pe>1,0);	}
		|  BREAK
			{	$$ = new stmt(BREAK,$<l>1,0); }
		|  CONTINUE
			{	$$ = new stmt(CONTINUE,$<l>1,0); }
		|  RETURN e
			{	$$ = new estmt(RETURN,$<l>1,$<pe>2,0); }
		|  GOTO ID
			{	$$ = new lstmt(GOTO,$<l>1,$<pn>2,0); }
		|  DO { stmt_seen=1; } statement WHILE condition
			{	$$ = new estmt(DO,$<l>1,$<pe>5,$<ps>3); }
		;

statement	:  simple SM
		|  ASM LP STRING RP SM
			{	
				if (stmt_seen)
					$$ = new estmt(ASM,curloc,(Pexpr)$<s>3,0);
				else {
					Pname n = new name(make_name('A'));
					n->tp = new basetype(ASM,(Pname)$<s>3);
					if (cd)
						cd->add_list(n);
					else
						cd = new nlist(n);
					$$ = 0;
				}
			}
		|  data_dcl
			{	Pname n = $<pn>1;
				if (n)
					if (stmt_seen) {
						$$ = new block(n->where,n,0);
						$<ps>$->base = DCL;
					}
					else {
						if (cd)
							cd->add_list(n);
						else
							cd = new nlist(n);
						$$ = 0;
					}
			}
		|  att_fct_def
			{	Pname n = $<pn>1;
				lex_unget(RC);
				error(&n->where,"%n's definition is nested (did you forget a ``}''?)",n);
				if (cd)
					cd->add_list(n);
				else
					cd = new nlist(n);
				$$ = 0;
			}
		|  block
		|  IF condition statement
			{	$$ = new ifstmt($<l>1,$<pe>2,$<ps>3,0); }
		|  IF condition statement ELSE statement
			{	$$ = new ifstmt($<l>1,$<pe>2,$<ps>3,$<ps>5); }
		|  WHILE condition statement
			{	$$ = new estmt(WHILE,$<l>1,$<pe>2,$<ps>3); }
		|  FOR LP { stmt_seen=1; } statement e SM e RP statement
			{	$$ = new forstmt($<l>1,$<ps>4,$<pe>5,$<pe>7,$<ps>9); }
		|  SWITCH condition statement
			{	$$ = new estmt(SWITCH,$<l>1,$<pe>2,$<ps>3); }
		|  ID COLON { $$ = $1; stmt_seen=1; } statement
			{	Pname n = $<pn>3;
				$$ = new lstmt(LABEL,n->where,n,$<ps>4);
			}
		|  CASE { stmt_seen=1; } e COLON statement
			{	if ($<pe>3 == dummy) error("empty case label");
				$$ = new estmt(CASE,$<l>1,$<pe>3,$<ps>5);
			}
		|  DEFAULT COLON { stmt_seen=1; } statement
			{	$$ = new stmt(DEFAULT,$<l>1,$<ps>4); }
		;



/********************* expressions: returns Pexpr **************/


elist		: ex_list
			{	Pexpr e = expr_unlist($<el>1);
				while (e && e->e1==dummy) {
					register Pexpr ee2 = e->e2;
					if (ee2) error("EX inEL");
					delete e;
					e = ee2;
				}
				$$ = e;
			}

ex_list		:  initializer		%prec CM
			{	$<el>$ = new elist(new expr(ELIST,$<pe>1,0)); }
		|  ex_list CM initializer
			{	$<el>1->add(new expr(ELIST,$<pe>3,0)); }
		;

initializer	:  e				%prec CM
		|  LC elist RC
			{	Pexpr e;
				if ($2)
					e = $<pe>2;
				else
					e = new expr(ELIST,dummy,0);
				$$ = new expr(ILIST,e,0);
			}
		;



e		:  e ASSIGN e
			{	binop:	$$ = new expr($<t>2,$<pe>1,$<pe>3); }
		|  e PLUS e	{	goto binop; }
		|  e MINUS e	{	goto binop; }
		|  e MUL e	{	goto binop; }
		|  e AND e	{	goto binop; }
		|  e OR e	{	goto binop; }
		|  e ER e	{	goto binop; }
		|  e SHIFTOP e	{ 	goto binop; }
		|  e EQUOP e	{	goto binop; }
		|  e DIVOP e	{	goto binop; }
		|  e RELOP e	{	goto binop; }
		|  e ANDAND e	{	goto binop; }
		|  e OROR e	{	goto binop; }
		|  e ASOP e	{	goto binop; }
		|  e CM e	{	goto binop; }
		|  e QUEST e COLON e
			{	$$ = new qexpr($<pe>1,$<pe>3,$<pe>5); }
		|  DELETE term 
			{	$$ = new expr(DELETE,$<pe>2,0); }
		|  DELETE LB e RB term
			{	$$ = new expr(DELETE,$<pe>5,$<pe>3); }
		|  term
		|  %prec NO_EXPR
			{	$$ = dummy; }
		;

term		:  TYPE LP elist RP
			{ 	$$ = new texpr(VALUE,tok_to_type($<t>1),$<pe>3); }
		|  TNAME LP elist RP
			{	$$ = new texpr(VALUE,$<pn>1->tp,$<pe>3); }
		|  NEW new_type
			{	Ptype t = $<pn>2->tp; $$ = new texpr(NEW,t,0); }
		|  NEW LP new_type RP
			{	Ptype t = $<pn>3->tp; $$ = new texpr(NEW,t,0); }
		|  term ICOP
			{	$$ = new expr($<t>2,$<pe>1,0); }
		|  CAST cast_type ENDCAST term %prec ICOP
			{	$$ = new texpr(CAST,$<pn>2->tp,$<pe>4); }
		|  MUL term
			{	$$ = new expr(DEREF,$<pe>2,0); }
		|  AND term
			{	$$ = new expr(ADDROF,0,$<pe>2); }
		|  MINUS term
			{	$$ = new expr(UMINUS,0,$<pe>2); }
		|  PLUS term
			{	$$ = new expr(UPLUS,0,$<pe>2); }
		|  NOT term
			{	$$ = new expr(NOT,0,$<pe>2); }
		|  COMPL term
			{	$$ = new expr(COMPL,0,$<pe>2); }
		|  ICOP term
			{	$$ = new expr($<t>1,0,$<pe>2); }
		|  SIZEOF term
			{	$$ = new texpr(SIZEOF,0,$<pe>2); }
		|  SIZEOF CAST cast_type ENDCAST %prec SIZEOF
			{	$$ = new texpr(SIZEOF,$<pn>3->tp,0); }
		|  term LB e RB
			{	$$ = new expr(DEREF,$<pe>1,$<pe>3); }
/*		|  term NOT term %prec LB
			{	$$ = new expr(DEREF,$<pe>1,$<pe>3); }
*/
		|  term LP elist RP
			{	Pexpr ee = $<pe>3;
				Pexpr e = $<pe>1;
				if (e->base == NEW)
					e->e1 = ee;
				else
					$$ = new call(e,ee);
			}
		|  term REF prim
			{	$$ = new ref(REF,$<pe>1,$<pn>3); }
		|  term REF MUL term
			{	$$ = new expr(REF,$<pe>1,$<pe>4); }
		|  term REF TNAME
			{	$$ = new ref(REF,$<pe>1,Ncopy($<pn>3)); }
		|  term DOT prim
			{	$$ = new ref(DOT,$<pe>1,$<pn>3); }
		|  term DOT MUL term
			{	$$ = new expr(DOT,$<pe>1,$<pe>4); }
		|  term DOT TNAME
			{	$$ = new ref(DOT,$<pe>1,Ncopy($<pn>3)); }
		|  MEM tag
			{	$$ = Ncopy($<pn>2);
				$<pn>$->n_qualifier = sta_name;
			}
		|  prim
		|  LP e RP
			{	$$ = $2; }
		|  ZERO
			{	$$ = zero; }
		|  ICON
			{	$$ = new expr(ICON,0,0);
				$<pe>$->string = $<s>1;
			}
		|  FCON
			{	$$ = new expr(FCON,0,0);
				$<pe>$->string = $<s>1;
			}
		|  STRING
			{	$$ = new expr(STRING,0,0);
				$<pe>$->string = $<s>1;
			}
		|  CCON
			{	$$ = new expr(CCON,0,0);
				$<pe>$->string = $<s>1;
			}
		|  THIS
			{	$$ = new expr(THIS,0,0); }
		;

prim		:  ID
			{	$$ = $<pn>1; }
		|  TNAME MEM tag
			{	$$ = Ncopy($<pn>3);
				$<pn>$->n_qualifier = $<pn>1;
			}
		|  ID MEM tag
			{	$$ = Ncopy($<pn>3);
				look_for_hidden($<pn>1,$<pn>$);
			}
		|  OPERATOR oper
			{	$$ = new name(oper_name($2));
				$<pn>$->n_oper = $<t>2;
			}
		|  TNAME MEM OPERATOR oper
			{	$$ = new name(oper_name($4));
				$<pn>$->n_oper = $<t>4;
				$<pn>$->n_qualifier = $<pn>1;
			}
		|  ID MEM OPERATOR oper
			{	$$ = new name(oper_name($4));
				$<pn>$->n_oper = $<t>4;
				look_for_hidden($<pn>1,$<pn>$);
			}
		|  OPERATOR c_type
			{	$$ = $2;
				sig_name($<pn>$);
			}
		|  TNAME MEM OPERATOR c_type
			{	$$ = $4;
				sig_name($<pn>$);
				$<pn>$->n_qualifier = $<pn>1;
			}
		|  ID MEM OPERATOR c_type
			{	$$ = $4;
				sig_name($<pn>$);
				look_for_hidden($<pn>1,$<pn>$);
			}
		;



/****************** abstract types (return type Pname) *************/

cast_type	:  type cast_decl
			{	$$ = Ncast($1,$<pn>2); }
		;

c_tp		:  TYPE	
			{	$$ = new basetype($<t>1,0); }
		|  TNAME
			{	$$ = new basetype(TYPE,$<pn>1); }
		;

c_type		:  c_tp c_decl
			{	$$ = Ncast($1,$<pn>2); }
		;

new_type	:  type new_decl
			{	$$ = Ncast($1,$<pn>2); };

arg_type	:  type arg_decl
			{	$$ = Ndata($1,$<pn>2); }
		|  type arg_decl ASSIGN initializer
			{	$$ = Ndata($1,$<pn>2);
				$<pn>$->n_initializer = $<pe>4;
			}
		;

arg_list	:  LP arg_type_list RP
			{	$$ = new fct(0,name_unlist($<nl>2),1); }
		|  LP arg_type_list ELLIPSIS RP
			{	$$ = new fct(0,name_unlist($<nl>2),ELLIPSIS); }
		|  LP arg_type_list CM ELLIPSIS RP
			{	$$ = new fct(0,name_unlist($<nl>2),ELLIPSIS); }
		;

arg_type_list	:  arg_type_list CM at
			{	if ($3)
					if ($1)
						$<nl>1->add($<pn>3);
					else {
						error("AD syntax");
						$<nl>$ = new nlist($<pn>3); 
					}
				else
					error("AD syntax");
			}
		|  at	%prec CM
			{	if ($1) $<nl>$ = new nlist($<pn>1); }
		;


at		:  arg_type
		|  %prec EMPTY
			{	$$ = 0; }
		;


ptr		:  MUL %prec NO_ID
			{	$$ = new ptr(PTR,0); }
		|  AND %prec NO_ID
			{	$$ = new ptr(RPTR,0); }
		|  MUL TYPE
			{	TOK t = $<t>2;
				switch (t) {
				case VOLATILE:
					error('w',"\"%k\" not implemented (ignored)",t);
					$$ = new ptr(PTR,0);
					break;
				default:
					error("syntax error: *%k",t);
				case CONST:
					$$ = new ptr(PTR,0,1);
				}
			}
		|  AND TYPE	
			{	TOK t = $<t>2;
				switch (t) {
				case VOLATILE:
					error('w',"\"%k\" not implemented (ignored)",t);
					$$ = new ptr(RPTR,0);
					break;
				default:
					error("syntax error: &%k",t);
				case CONST:
					$$ = new ptr(RPTR,0,1);
				}
			}
		| MEMPTR %prec NO_ID
			{	Pptr p = new ptr(PTR,0);
				p->memof = Pclass(Pbase($<pn>1->tp)->b_name->tp);
				$$ = p;
			}
	
		;

vec		:  LB e RB
			{	$$ = new vec(0,($<pe>2!=dummy)?$<pe>2:0 ); }
/*		|  NOT term %prec LB
			{	$$ = new vec(0,$<pe>2); }
*/
		|  NOT %prec LB
			{	$$ = new vec(0,0); }
		;

%%

