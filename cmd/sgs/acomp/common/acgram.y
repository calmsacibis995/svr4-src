/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)acomp:common/acgram.y	55.4"
/* acgram.y */

/* Grammar for ANSI C compiler.
**
** The grammar, itself, contains little intelligence about
** the representations of things, leaving that to other
** modules.  Almost everything is accomplished by function
** calls to those modules.  The hope is that this grammar
** could be used by other tools, such as syntax-directed
** editors.
*/

#include "p1.h"

/* If YYDEBUG not defined explicitly, make YYDEBUG follow NODBG. */
#ifndef YYDEBUG
#  ifdef NODBG
#    define YYDEBUG 0
#  else
#    define YYDEBUG 1
#  endif
#endif


%}
%union {
    int intval;
    DN declnode;
    char * charp;
    ND1 * nodep;
    TYCL tybit;
}

%start sourcefile


%token	L_AND
%token	L_ANDAND
%token	L_ASM
%token	L_ASGOP
%token	L_BREAK
%token	L_CASE
%token	L_CHAR_CONST
%token	L_COLON
%token	L_COMMA
%token	L_CONTINUE
%token	L_DEFAULT
%token	L_DIVOP
%token	L_DO
%token	L_DOTDOTDOT
%token	L_ELSE
%token	L_ENUM
%token	L_EQ
%token	L_EQUALOP
%token	L_FLOAT_CONST
%token	L_FOR
%token	L_GOTO
%token	L_IDENT
%token	L_IF
%token	L_INCOP
%token	L_INT_CONST
%token	L_LB
%token	L_LC
%token	L_LP
%token	L_MINUS
%token	L_OR
%token	L_OROR
%token	L_PLUS
%token	L_QUEST
%token	L_RB
%token	L_RC
%token	L_RELOP
%token	L_RETURN
%token	L_RP
%token	L_SEMI
%token	L_SHIFTOP
%token	L_SIZEOF
%token	L_SORU			/* struct or union */
%token	L_STAR
%token	L_STRING
%token	L_STROP
%token	L_SWITCH
%token	L_TYPENAME
%token	L_UNARYOP
%token	L_WHILE
%token	L_XOR
%token	L_class			/* stands for any storage class */
%token	L_type			/* stands for any type word */

%nonassoc L_LOWEST_PREC
%nonassoc L_ELSE
%right	L_ASGOP L_EQ
%right	L_QUEST L_COLON
%left	L_OROR
%left	L_ANDAND
%left	L_OR
%left	L_XOR
%left	L_AND
%left	L_EQUALOP
%left	L_RELOP
%left	L_SHIFTOP
%left	L_PLUS L_MINUS
%left	L_STAR L_DIVOP
%right	L_UNARYOP
%right	L_PLUSPLUS L_MINUSMINUS

%type <intval>	L_class L_type L_TYPENAME L_SORU L_ENUM
		L_INCOP
		L_LB L_PLUS L_AND L_ANDAND L_EQUALOP L_INCOP L_MINUS
		L_OR L_OROR L_QUEST L_STAR L_STROP L_XOR
		L_EQ L_UNARYOP L_SHIFTOP L_RELOP L_ASGOP L_DIVOP
%type <declnode> declarator
		pointer direct_declarator funarg
		direct_abstract_declarator abstract_declarator
		opt_abstract_declarator
		parameter_declaration parameter_list
		parameter_type_list opt_parameter_type_list identifier_list
		p_decl
		struct_declarator s_struct_declarator struct_declarator_list
		struct_declaration struct_declaration_list
		enumerator enumerator_list
%type <charp>	L_IDENT id_or_type
%type <nodep>	constant e .e opt_e constant_expression opt_constant_expression
		assignment_e cast_e unary_e postfix_e primary_e
		s_postfix_e func_call argument_expression_list
		type_name
		decl_array
		L_INT_CONST L_CHAR_CONST L_STRING L_FLOAT_CONST


%%

sourcefile:
		file			{ cg_eof(C_TOKEN); }
	|	/* EMPTY */ 		{ cg_eof(C_NOTOKEN); }
	;

/* External Definitions */

file:
		s_external_definition
	|	file s_external_definition
	;

external_definition:
		function_definition
	|	declaration
/*STRICT:  ANSI doesn't allow declaration-specifiers to be omitted
**	   for external-definition.  UNIX C allows this antique
**	   construction.  UNIX C also allows a completely empty
**	   declaration.
*/
	|	init_declarator_list L_SEMI
	|			     L_SEMI { dcl_topnull(); }
	|	oldasm
	|	error
	;

s_external_definition:
	/* Prepare for next declaration/definition. */
		{ dcl_start(); } external_definition
	;

function_definition:
		                         f_declarator function_body
	|	s_declaration_specifiers f_declarator function_body
	;

function_body:
		{ dcl_s_formal(); dcl_start(); }
		formal_declaration_list
		{ dcl_e_formal(); dcl_s_func(); }
		compound_statement
		{ dcl_e_func(); }
	 ;

formal_declaration_list:
		declaration_list
	|	/*EMPTY*/
	;



/* Declarations */

/* Ideally dcl_start() would be called before each declaration.
** However, putting such a call where it is needed introduces
** conflicts into the grammar.  Thus, they are sprinkled around
** at the end of declarations to prepare for the one that would
** follow.
*/

declaration:
		s_declaration_specifiers init_declarator_list    decl_semi
	/* Check for vacuous declaration if no declarator list. */
	|	s_declaration_specifiers { dcl_chkend(D_DECL); } decl_semi
	;

/* This production is used to prepare for the next declaration in a
** declaration list.
*/

decl_semi:	L_SEMI { dcl_start(); }
	;

declaration_specifiers:
		type_or_class
	|	declaration_specifiers type_or_class
	;

/* s_declaration_specifiers records the base type and storage
** class for a declarator or abstract_declarator.
*/
s_declaration_specifiers:
		declaration_specifiers { dcl_e_ds(); }
	;

type_or_class:
		L_class 		{ dcl_tycl($1); }
	|	type_specifier
	;

init_declarator_list:
		init_declarator
	|	init_declarator_list 
		{ dcl_clr_type(); }
		L_COMMA 
		{ LX_GETCURLINE(); } 
		init_declarator
	;

init_declarator:
		s_declarator
	|	i_declarator 
		{ dcl_set_type(); } 
		L_EQ 
		{ in_start(); } 
		initializer 
		{ in_end(); }
	;

/* BEWARE:  the following non-terminals are highly sensitive
** to their trailing context.  An f_declarator has either a
** type or { following.  An i_declarator has an = following.
** An s_declarator has an identifier or ; following.
*/

/* An s_declarator marks the bounds of a declarator. */

s_declarator:	dcl_s_dcor declarator { (void) dcl_norm($2,DS_NORM); }
	;

/* An f_declarator marks the bounds of the declarator part of
** a function definition.
*/

f_declarator:	dcl_s_dcor declarator { (void) dcl_norm($2,DS_FUNC); }
	;

/* An i_declarator marks the bounds of the declarator part of
** an init-declarator that has an initializer.
*/

i_declarator:	dcl_s_dcor declarator { (void) dcl_norm($2,DS_INIT); }

/* This non-terminal is broken out to prevent yacc conflicts. */
dcl_s_dcor:	{ dcl_s_dcor(); }

type_specifier:
		L_type		{ dcl_tycl($1); }
	|	L_TYPENAME	{
				    /* dcl_type returns pointer to name
				    ** if type's name is rejected as typedef
				    */
				    YYSTYPE temp;

/* yacc's that have no YYBACKUP will be unable to handle
** redeclared typedefs.
*/
				    temp.charp = dcl_type($1);
#ifdef	YYBACKUP
				    if (temp.charp != 0)
					YYBACKUP(L_IDENT, temp);
#endif
				}
	/* The dcl_tycl() stuff is done when the keyword
	** is seen for these.
	*/
	|	struct_or_union_specifier
	|	enum_specifier
	;

/* This non-terminal is useful when types and identifers can
** be disambiguated by context, but the separate name-spaces
** haven't been set yet.
*/
id_or_type:	L_IDENT
	|	L_TYPENAME		{ $$ = dcl_name($1); }
	;

struct_or_union_specifier:
		struct_or_union id_or_type { dcl_tag($2, D_NOLIST, D_NOFORCE); }
	|	struct_or_union id_or_type { dcl_tag($2, D_LIST, D_NOFORCE); }
							s_st_decl_list
	|	struct_or_union            { dcl_tag((char *) 0,D_LIST,D_NOFORCE); }
							s_st_decl_list 
	;

struct_or_union:
		L_SORU 			{ dcl_tycl($1); }
	;

/* There's some superfluous grammar hacking here to allow backward
** compatibility with code PCC accepted that lacked the trailing ;
** and still avoid shift/reduce conflicts.  Sorry about that.
*/
s_st_decl_list:
		L_LC
		{ dcl_s_soru(); }
		struct_declaration_list
		struct_opt_semi
		L_RC
		{ dcl_e_soru($3); }
	;

struct_opt_semi:
		/* EMPTY */ { dcl_nosusemi(); }
	|	L_SEMI
	;

struct_declaration_list:
		struct_declaration		{ $$ = dcl_mlist(DN_NULL, $1); }
	|	struct_declaration_list L_SEMI struct_declaration { $$ = dcl_mlist($1, $3); }
	;

struct_declaration:
		{ dcl_start(); }
		type_specifier_list
		{ dcl_e_ds(); }			/* end decl. specifiers */
		struct_declarator_list	{ $$ = $4; }
	;

/* Allow empty member list for backward compatibility. */

struct_declarator_list:
		s_struct_declarator		{ $$ = dcl_mlist(DN_NULL, $1); }
	|	struct_declarator_list 
		{ dcl_clr_type(); }
		L_COMMA 
		s_struct_declarator
		{ $$ = dcl_mlist($1, $4); dcl_set_type(); }
	|	/*EMPTY*/		{ $$ = dcl_mbr(DN_NULL,D_NOFIELD,ND1NIL);
					  $$ = dcl_mlist(DN_NULL,$$);
					}
	;

/* Put semantics around struct/union declarator. */

s_struct_declarator:
		{ dcl_s_dcor(); } struct_declarator { $$ = $2; }
	;
		
struct_declarator:
		declarator		{ $$ = dcl_mbr($1,D_NOFIELD,ND1NIL); }
	|	declarator  L_COLON constant_expression
					{ $$ = dcl_mbr($1,D_FIELD,$3); }
	|	/* EMPTY */ L_COLON constant_expression
					{ $$ = dcl_mbr(DN_NULL,D_FIELD,$2); }
	|	error			{ $$ = DN_NULL; }
	;

enum_specifier:
		enum id_or_type { dcl_s_enu($2,D_LIST); }         enu_list
	|	enum            { dcl_s_enu((char *) 0,D_LIST); } enu_list
	|	enum id_or_type { dcl_s_enu($2,D_NOLIST); }
	;

enum:		L_ENUM			{ dcl_tycl( $1 ); }
	;

enu_list:	L_LC enumerator_list enum_opt_comma L_RC { dcl_e_enu($2); }
	;

enumerator_list:
		enumerator			{ $$ = dcl_elist(DN_NULL, $1); }
	|	enumerator_list L_COMMA enumerator
						{ $$ = dcl_elist($1,$3); }
	;

enum_opt_comma:
		/*EMPTY*/
	|	L_COMMA			{ dcl_noenumcomma(); }
	;

enumerator:
		id_or_type { $$ = dcl_menu($1,D_NOEXPR); }
	|	id_or_type { $$ = dcl_menu($1,D_EXPR); }
		    L_EQ constant_expression { $$ = dcl_eexpr($<declnode>2, $4); }
	;

declarator:
	 	        direct_declarator
	|	pointer direct_declarator { $$ = dcl_ofptr($1,$2); }
	;

direct_declarator:
		L_IDENT { $$ = dcl_dcor($1); }
	|	declL_LP declarator declL_RP { $$ = $2; }
	|	direct_declarator decl_array { $$ = dcl_arrof($1, $2); }
	|	direct_declarator funarg { $$ = dcl_func($1, $2); }
	;

decl_array:	{ dcl_set_type(); } L_LB opt_constant_expression L_RB { $$ = $3; }
	;

funarg:		funL_LP parameter_type_list funL_RP	{ $$ = $2; }
	|	funL_LP identifier_list     funL_RP	{ $$ = $2; }
	|	funL_LP			    funL_RP	{ $$ = DN_NULL; }
	;

funL_LP:	L_LP { dcl_f_lp(); }
	;
funL_RP:	L_RP { dcl_f_rp(); }
	;
declL_LP:	L_LP { dcl_lp(); }
	;
declL_RP:	L_RP { dcl_rp(); }
	;

pointer:
	 	ptrL_STAR			{ $$ = dcl_ptrto(DN_NULL); }
	|	ptrL_STAR type_specifier_list   { $$ = dcl_ptrto(DN_NULL); }
	|	ptrL_STAR type_specifier_list pointer
						{ $$ = dcl_ptrto($3); }
	|	ptrL_STAR                     pointer
						{ $$ = dcl_ptrto($2); }
	;

type_specifier_list:
		type_specifier
	|	type_specifier_list type_specifier
	;

/* Bracket pointer stuff to get const/volatile modifiers.
** Attach modifier to non-terminal.
*/

ptrL_STAR:
		L_STAR	{ dcl_s_ptrtsl(); }
	;

/* f(...) not allowed in strictly conforming ANSI C */
parameter_type_list:
		parameter_list
	|	L_DOTDOTDOT		/* extension */
			{ $$ = dcl_plist(DN_NULL, dcl_vparam()); }
	|	parameter_list L_COMMA L_DOTDOTDOT
			{ $$ = dcl_plist($1, dcl_vparam()); }
	;

parameter_list:
		parameter_declaration
			{ $$ = dcl_plist(DN_NULL, $1); }
	|	parameter_list L_COMMA parameter_declaration
			{ $$ = dcl_plist($1, $3); }
	;


/* To avoid reduce/reduce conflicts, break out pieces of
** s_declarator and abstract_declarator, and put them
** around p_decl.
*/
parameter_declaration:
		{ dcl_start(); }
		s_declaration_specifiers
		{ dcl_s_dcor(); }
		p_decl
		{ $$ = dcl_param($4); }
	;

p_decl:		/* EMPTY */		{ $$ = DN_NULL; }
	|	declarator
	|	abstract_declarator
	;

identifier_list:
		L_IDENT
			{ $$ = dcl_plist(DN_NULL, dcl_ident($1)); }
	|	identifier_list L_COMMA L_IDENT
			{ $$ = dcl_plist($1, dcl_ident($3)); }
	|	error	{ $$ = DN_NULL; }
	;

type_name:
		{ dcl_s_absdcl(); }
		type_specifier_list
		{ dcl_e_ds(); dcl_s_dcor(); }
		opt_abstract_declarator
		{ $$ = tr_type(dcl_e_absdcl($4)); }
	;

opt_abstract_declarator:
		/* EMPTY */		{ $$ = DN_NULL; }
	|	abstract_declarator	{ dcl_chkend(D_ABSDECL); }
	;

abstract_declarator:
		pointer				   { $$ = dcl_ofptr($1,DN_NULL); }
	|	pointer direct_abstract_declarator { $$ = dcl_ofptr($1,$2); }
	|	        direct_abstract_declarator
	;


direct_abstract_declarator:
		declL_LP abstract_declarator declL_RP { $$ = $2; }
	|	direct_abstract_declarator decl_array
			{ $$ = dcl_arrof($1, $2); }
	|	                           decl_array
			{ $$ = dcl_arrof(DN_NULL, $1); }
	|	direct_abstract_declarator funL_LP opt_parameter_type_list funL_RP
			{ $$ = dcl_func($1, $3); }
	|	                           funL_LP opt_parameter_type_list funL_RP
			{ $$ = dcl_func(DN_NULL, $2); }
	;

opt_parameter_type_list:
		parameter_type_list
	|	/*EMPTY*/		{ $$ = DN_NULL; }
	;

initializer:
		assignment_e	{ in_init($1); }
	|	L_LC { in_lc(); } initializer_list opt_L_COMMA L_RC { in_rc(); }
	;

opt_L_COMMA:
		L_COMMA
	|	/*EMPTY*/
	;

initializer_list:
		initializer
	|	initializer_list L_COMMA initializer
	;


/* Statements */

statement:
		labeled_statement
	|	compound_statement
	|	expression_statement
	|	selection_statement
	|	iteration_statement
	|	jump_statement
	|	oldasm
	|	error L_SEMI
	|	error L_RC
	;

/* Recognize old-style asm() call. */
oldasm:		L_ASM L_LP L_STRING { cg_asmold($3); } L_RP L_SEMI
	;

labeled_statement:
		id_or_type L_COLON { LX_GETCURLINE(); sm_deflab($1); } statement
	|	L_CASE  constant_expression { sm_sw_case($2); }	
		L_COLON 		    { LX_GETCURLINE(); }
		statement
	|	L_DEFAULT		    { sm_default(); }	
		L_COLON 		    { LX_GETCURLINE(); }
		statement
	;

compound_statement:
		cmpdL_LC declaration_list opt_statement_list cmpdL_RC
	|	cmpdL_LC                  opt_statement_list cmpdL_RC
	;

cmpdL_LC:	L_LC { sm_lc(); dcl_start(); }
	;
cmpdL_RC:	L_RC { sm_rc(); }
	;

declaration_list:
		declaration
	|	declaration_list declaration
	;

statement_list:
		statement
	|	statement_list statement
	;

opt_statement_list:
		statement_list
	|	/*EMPTY*/
	;


expression_statement:
		opt_e L_SEMI		{ sm_expr($1); }
	;

selection_statement:
		if_prefix %prec L_LOWEST_PREC	{ sm_if_end(); }
	|	if_prefix L_ELSE { sm_if_else(); } statement { sm_if_end(); }
	|	L_SWITCH L_LP e { sm_sw_start($3); } L_RP statement { sm_sw_end(); }
	;

if_prefix:	L_IF L_LP e L_RP { LX_GETCURLINE(); sm_if_start($3); } statement
	;

iteration_statement:
		L_WHILE { sm_wh_init(); }
		L_LP e L_RP { LX_GETCURLINE(); sm_wh_start($4); } 
		statement { sm_wh_end(); }
	|	L_DO { LX_GETCURLINE(); sm_do_start(); } statement
		L_WHILE L_LP e { sm_do_end($6); } L_RP L_SEMI
	|	L_FOR
		L_LP
		opt_e { sm_for_init($3); }
		L_SEMI
		opt_e { sm_for_control($6); }
		L_SEMI
		opt_e { LX_GETCURLINE(); sm_for_iter($9); }
		L_RP
		statement { sm_for_end(); }
	;

jump_statement:
		L_GOTO id_or_type { sm_goto($2); } L_SEMI
	|	L_CONTINUE { sm_continue(); } L_SEMI
	|	L_BREAK { sm_break(); } L_SEMI
	|	L_RETURN opt_e { sm_return($2); } L_SEMI
	;

/* Constants */

constant:
		L_FLOAT_CONST
	|	L_CHAR_CONST
/*	|	L_ENUM_CONST		enum constants look like names */
	|	L_INT_CONST
	;


/* Expressions */

e:
		assignment_e
	|	e L_COMMA assignment_e	{ $$ = tr_build(COMOP, $1, $3); }
	;

opt_e:
		e
	|	/*EMPTY*/		{ $$ = ND1NIL; }
	;

constant_expression:
		.e 
	;

opt_constant_expression:
		constant_expression
	|	/*EMPTY*/		{ $$ = ND1NIL; }
	;

assignment_e:
		.e
	/* The next two productions are illegal ANSI C, but are added
	** to provide compatibility with UNIX C, which allowed benign
	** casts on the left side of assign-ops.  The left side of each
	** was originally unary_e.
	*/
	|	cast_e  L_ASGOP assignment_e    { $$ = tr_build($2, $1, $3); }
        |       cast_e  L_EQ    assignment_e    { $$ = tr_build($2, $1, $3); }
	;

.e:
		.e L_QUEST e L_COLON .e
			    { $$ = tr_build($2, $1, tr_build(COLON, $3, $5)); }
	|	.e L_OROR .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_ANDAND .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_OR .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_XOR .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_AND .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_EQUALOP .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_RELOP .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_SHIFTOP .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_PLUS .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_MINUS .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_STAR .e			{ $$ = tr_build($2, $1, $3); }
	|	.e L_DIVOP .e			{ $$ = tr_build($2, $1, $3); }
	|	cast_e
	;

cast_e:
		unary_e
	|	L_LP type_name L_RP cast_e	{ $$ = tr_build(CAST, $2, $4); }
	;

unary_e:
		s_postfix_e
/* next line was L_INCOP unary_e; changed for backward compatibility */
	|	L_INCOP cast_e
				{ $$ = tr_build($1, $2, ND1NIL); }
	|	L_UNARYOP cast_e	{ $$ = tr_build($1, $2, ND1NIL); }
	|	L_SIZEOF unary_e	{ $$ = tr_sizeof($2); }
	|	L_SIZEOF L_LP type_name L_RP	{ $$ = tr_sizeof($3); }
	/* spell out the ones that are also binary */
	|	L_AND cast_e		{ $$ =
					    tr_build(UNARY AND, $2, ND1NIL); }
	|	L_AND L_DOTDOTDOT	{ $$ = tr_dotdotdot(); }
	|	L_STAR cast_e		{ $$ = tr_build(STAR, $2, ND1NIL); }
	|	L_PLUS cast_e		{ $$ = tr_build(UPLUS, $2, ND1NIL); }
	|	L_MINUS cast_e		{ $$ =
					    tr_build(UNARY MINUS, $2, ND1NIL); }
	;

postfix_e:
		primary_e
	|	s_postfix_e L_LB e L_RB	{ $$ = tr_build($2, $1, $3); }
	|	func_call argument_expression_list L_RP
					{ $$ = tr_build(CALL, $1, $2); }
	|	func_call                          L_RP
					{ $$ = tr_build(UNARY CALL, $1, ND1NIL); }
	|	s_postfix_e L_STROP id_or_type
					{ $$ = tr_build($2, $1,
						tr_su_mbr($2,$1,$3)); }
	|	s_postfix_e L_INCOP	{ $$ = tr_build($2, ND1NIL, $1); }
	;

/* Check for undefined NAME in postfix_e. */
s_postfix_e:
		postfix_e		{ $$ = tr_chkname($1, 0); }
	;
/* Function call:  check for undefined name, do the right thing. */
func_call:
		postfix_e L_LP		{ $$ = tr_chkname($1, 1); }
	;

argument_expression_list:
		assignment_e
	|	argument_expression_list L_COMMA assignment_e
					{ $$ = tr_build(CM, $1, $3); }
	;

primary_e:
		L_IDENT			{ $$ = tr_name($1); }
	|	constant
	|	L_STRING
	|	L_LP e L_RP		{ $$ = $2; 
#ifdef LINT
    ln_paren($2);
#endif
	}
	;
%%
