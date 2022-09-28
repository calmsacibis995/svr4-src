/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)ctrace:parser.y	1.15"
/* parser.y (was acgram.y) */

/* Grammar for ANSI C compiler.
**
** The grammar, itself, contains little intelligence about
** the representations of things, leaving that to other
** modules.  Almost everything is accomplished by function
** calls to those modules.  The hope is that this grammar
** could be used by other tools, such as syntax-directed
** editors.
*/

#include "global.h"

/* Suppress YYDEBUG if NODBG and no explicit YYDEBUG. */
#if defined(NODBG) && !defined(YYDEBUG)
#define YYDEBUG 0
#endif

#define min(X, Y)       (((int) X < (int) Y) ? X : Y)
#define max(X, Y)       (((int) X > (int) Y) ? X : Y)
#define max3(X, Y, Z)   max(X, max(Y, Z))
#define max4(W,X,Y,Z)   max(W, max(X, max(Y, Z)))

#define symbol_info(X, FIRST_SYM, LAST_SYM, TYPE) \
			X.start = FIRST_SYM.start,    /* default if $1 */ \
			X.end = LAST_SYM.end, \
			X.type = TYPE

#define add_trace(VAR, FIRST_SYM, LAST_SYM, TYPE) \
			(void) add_fcn(VAR, FIRST_SYM.start, LAST_SYM.end, TYPE)

#define expand_trace(VAR, FIRST_SYM, LAST_SYM) \
			expand_fcn(VAR, FIRST_SYM.start, LAST_SYM.end)

enum    bool fcn_body = no;	     /* function body indicator */

static	enum	bool err_flag = no;	/* in formal declaration statement */
static	enum	bool typename = no;		/* in typedef statement */
static  int     len;		    /* temporary variable */
static  int     fcn_line;	       /* function header line number */
static  char    fcn_name[IDMAX + 1];    /* function name */
static  char    fcn_text[SAVEMAX + 1];  /* function header text */
static  char    nf_name[IDMAX + 1];     /* non-function name */
static  enum    bool save_header = yes; /* start of function */
static  enum    bool flow_break = no;   /* return, goto, break, & continue */
static  enum    bool executable = no;   /* executable statements */
%}
%union {
	struct symbol_struct symbol;
}
/* no precedence section yet */

%start sourcefile


%token <symbol> L_AND
%token <symbol> L_ANDAND
%token <symbol> L_ASM
%token <symbol> L_ASGOP
%token <symbol> L_BREAK
%token <symbol> L_CASE
%token <symbol> L_CHAR_CONST
%token <symbol> L_COLON
%token <symbol> L_COMMA
%token <symbol> L_CONTINUE
%token <symbol> L_DEFAULT
%token <symbol> L_DIVOP
%token <symbol> L_DO
%token <symbol> L_DOTDOTDOT
%token <symbol> L_ELSE
%token <symbol> L_ENUM
%token <symbol> L_EQ
%token <symbol> L_EQUALOP
%token <symbol> L_FLOAT_CONST
%token <symbol> L_FOR
%token <symbol> L_FUNCTION
%token <symbol> L_GOTO
%token <symbol> L_IDENT
%token <symbol> L_IF
%token <symbol> L_INCOP
%token <symbol> L_INT_CONST
%token <symbol> L_LB
%token <symbol> L_LC
%token <symbol> L_LP
%token <symbol> L_MINUS
%token <symbol> L_OR
%token <symbol> L_OROR
%token <symbol> L_PLUS
%token <symbol> L_QUEST
%token <symbol> L_RB
%token <symbol> L_RC
%token <symbol> L_RELOP
%token <symbol> L_RETURN
%token <symbol> L_RP
%token <symbol> L_SEMI
%token <symbol> L_SHIFTOP
%token <symbol> L_SIZEOF
%token <symbol> L_SORU			/* struct or union */
%token <symbol> L_STAR
%token <symbol> L_STRING
%token <symbol> L_STROP
%token <symbol> L_SWITCH
%token <symbol> L_TYPENAME
%token <symbol> L_UNARYOP
%token <symbol> L_WHILE
%token <symbol> L_XOR
%token <symbol> L_class			/* stands for any storage class */
%token <symbol> L_type			/* stands for any type word */
%token <symbol> IOBUF JMP_BUF		/* used only by lookup() */
%token <symbol> SIG_DEF SIG_SYS TYP_DEF SEL_DEF
%token <symbol> TIME_T CLOCK_T SIZE_T
%token <symbol> STRRES STRCAT STRCMP STRCPY STRLEN STRNCAT STRNCMP
%token <symbol> MACRO TYPEDEF
%token <symbol> PP_IF PP_ELSE PP_ENDIF 

%nonassoc <symbol> L_LOWEST_PREC
%nonassoc L_ELSE
%right L_ASGOP L_EQ
%right L_QUEST L_COLON
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

%type <symbol> .e
%type <symbol> L_AND
%type <symbol> L_ANDAND
%type <symbol> L_ASGOP
%type <symbol> L_CHAR_CONST
%type <symbol> L_COMMA
%type <symbol> L_DIVOP
%type <symbol> L_ENUM
%type <symbol> L_EQ
%type <symbol> L_EQUALOP
%type <symbol> L_FLOAT_CONST
%type <symbol> L_IDENT
%type <symbol> L_INCOP
%type <symbol> L_INT_CONST
%type <symbol> L_LB
%type <symbol> L_MINUS
%type <symbol> L_OR
%type <symbol> L_OROR
%type <symbol> L_PLUS
%type <symbol> L_QUEST
%type <symbol> L_RELOP
%type <symbol> L_SHIFTOP
%type <symbol> L_SORU
%type <symbol> L_STAR
%type <symbol> L_STRING
%type <symbol> L_STROP
%type <symbol> L_TYPENAME
%type <symbol> L_UNARYOP
%type <symbol> L_XOR
%type <symbol> L_class
%type <symbol> L_type
%type <symbol> abstract_declarator
%type <symbol> argument_expression_list
%type <symbol> assignment_e
%type <symbol> cast_e
%type <symbol> cmpdL_LC
%type <symbol> cmpdL_RC
%type <symbol> compound_statement
%type <symbol> constant
%type <symbol> constant_expression
%type <symbol> dcl_s_dcor
%type <symbol> declL_LP
%type <symbol> declL_RP
%type <symbol> decl_array
%type <symbol> declaration
%type <symbol> declaration_list
%type <symbol> declaration_specifiers
%type <symbol> declarator
%type <symbol> direct_abstract_declarator
%type <symbol> direct_declarator
%type <symbol> e
%type <symbol> enu_list
%type <symbol> enum
%type <symbol> enum_opt_comma
%type <symbol> enum_specifier
%type <symbol> enumerator
%type <symbol> enumerator_list
%type <symbol> error
%type <symbol> expression_statement
%type <symbol> f_declarator
%type <symbol> funL_LP
%type <symbol> funL_RP
%type <symbol> funarg
%type <symbol> func_call
%type <symbol> i_declarator
%type <symbol> id_type_list
%type <symbol> id_or_function
%type <symbol> id_or_type
%type <symbol> identifier_list
%type <symbol> if_prefix
%type <symbol> init_declarator
%type <symbol> init_declarator_list
%type <symbol> init_type_declaration
%type <symbol> initializer
%type <symbol> initializer_list
%type <symbol> int_strfcn
%type <symbol> iteration_statement
%type <symbol> jump_statement
%type <symbol> labeled_statement
%type <symbol> oldasm
%type <symbol> opt_L_COMMA
%type <symbol> opt_abstract_declarator
%type <symbol> opt_constant_expression
%type <symbol> opt_e
%type <symbol> opt_parameter_type_list
%type <symbol> opt_statement_list
%type <symbol> p_decl
%type <symbol> parameter_declaration
%type <symbol> parameter_list
%type <symbol> parameter_type_list
%type <symbol> pointer
%type <symbol> postfix_e
%type <symbol> pp_else_statement
%type <symbol> pp_if_statement
%type <symbol> primary_e
%type <symbol> ptrL_STAR
%type <symbol> s_declaration_specifiers
%type <symbol> s_declarator
%type <symbol> s_postfix_e
%type <symbol> s_struct_declarator
%type <symbol> selection_statement
%type <symbol> statement
%type <symbol> statement_list
%type <symbol> strfcn
%type <symbol> strfcn_name
%type <symbol> string_list
%type <symbol> struct_declaration
%type <symbol> struct_declaration_list
%type <symbol> struct_declarator
%type <symbol> struct_declarator_list
%type <symbol> struct_or_union
%type <symbol> struct_or_union_specifier
%type <symbol> type_name
%type <symbol> type_or_class
%type <symbol> type_specifier
%type <symbol> type_specifier_list
%type <symbol> unary_e


%%

sourcefile:
		file			
	|	/* EMPTY */ 
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
	|	init_declarator_list L_SEMI { 
			if (!executable) {
				puttext($2.end);
			}
		}
	|	L_SEMI  { if (!executable) {
			puttext($1.end);
			}
		}
	|	oldasm
	;

s_external_definition:
	/* Prepare for next declaration/definition. */
		external_definition
	;

function_definition:
		f_declarator function_body {
			save_header=yes;
		}
	|	s_declaration_specifiers f_declarator function_body {
			save_header=yes;
		}
	;

function_body:
		formal_declaration_list
		L_LC {
			puttext (yyleng);
			fcn_body = yes;
			}
		formal_declaration_list {
			if (err_flag) (void)putchar('{');
			/* see if this function is to be traced */
			 tr_fcn(fcn_name);
		       
			/* declare a temporary variable for string function results */
			(void)printf("char *_ct;");   /* char pointer may be bigger than int */
			/* trace the function header */
			tr_stmt(fcn_line, fcn_text, yes);
			executable = yes;
			if (err_flag) (void)putchar('}');
			}
		opt_statement_list 
		L_RC {
			executable = no;
			fcn_body = no;
			if (flow_break)
				flow_break = no;
			else
				tr_stmt(NO_LINENO, "/* return */", yes);
			puttext(yyleng);
		}
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
	|	s_declaration_specifiers  decl_semi
	|	TYPEDEF declaration_specifiers {
			/* put flag on to look for names */
			typename = yes;
		}
		id_type_list decl_semi {
			typename = no;
		}
	|	error semi_or_curly {
			err_flag = yes;
		}
	;

id_type_list:
		init_type_declaration
	|	id_type_list L_COMMA init_type_declaration
	;

init_type_declaration:
		L_TYPENAME
	|	init_declarator
	;

semi_or_curly:
		L_RC
	|	decl_semi
	;

/* This production is used to prepare for the next declaration in a
** declaration list.
*/

decl_semi:	L_SEMI  { if (!executable) {
			    puttext($1.end);
			   }
			}
	;

declaration_specifiers:
		type_or_class
	|	declaration_specifiers type_or_class
	;

/* s_declaration_specifiers records the base type and storage
** class for a declarator or abstract_declarator.
*/
s_declaration_specifiers:
		declaration_specifiers {
			/* save the function name */
			if (save_header) {
				len = min($1.end - $1.start, IDMAX);
				(void)strncpy(fcn_name, yytext + $1.start, (unsigned) len);
				fcn_name[len] = '\0';
			}
		 }
	;

type_or_class:
		L_class 		
	|	type_specifier
	;

init_declarator_list:
		init_declarator
	|	init_declarator_list L_COMMA init_declarator
	;

init_declarator:
		s_declarator
	|	i_declarator L_EQ  initializer 
	;

/* BEWARE:  the following non-terminals are highly sensitive
** to their trailing context.  An f_declarator has either a
** type or { following.  An i_declarator has an = following.
** An s_declarator has an identifier or ; following.
*/

/* An s_declarator marks the bounds of a declarator. */

s_declarator:	dcl_s_dcor declarator  
	;

/* An f_declarator marks the bounds of the declarator part of
** a function definition.
*/

f_declarator:	dcl_s_dcor declarator {
			save_header = no;
			fcn_body = yes;
			/* see if this function is to be traced */
			tr_fcn(fcn_name);
		} 
	;

/* An i_declarator marks the bounds of the declarator part of
** an init-declarator that has an initializer.
*/

i_declarator:	dcl_s_dcor declarator  

/* This non-terminal is broken out to prevent yacc conflicts. */
dcl_s_dcor:	
		{ ; }
	;

type_specifier:
		L_type
	|	L_TYPENAME
	|	struct_or_union_specifier
	|	enum_specifier
	;

/* This non-terminal is useful when types and identifers can
** be disambiguated by context, but the separate name-spaces
** haven't been set yet.
*/
id_or_type:     L_IDENT
	|	L_TYPENAME		
	|	L_FUNCTION
	;

struct_or_union_specifier:
		struct_or_union id_or_type 
	|	struct_or_union id_or_type s_st_decl_list
	|	struct_or_union s_st_decl_list 
	;

struct_or_union:
		L_SORU 			
	;

/* There's some superfluous grammar hacking here to allow backward
** compatibility with code PCC accepted that lacked the trailing ;
** and still avoid shift/reduce conflicts.  Sorry about that.
*/
s_st_decl_list:
		L_LC {
			/* output this part of the statement so the first
			** struct element declaration looks like a new
			** statement so the typedef scanning works 
			*/
			if (!executable) {
				puttext($1.end);
			}
		    }
		struct_declaration_list
		struct_opt_semi
		L_RC
	;

struct_opt_semi:
		/* EMPTY */ 
	|	L_SEMI { if (!executable) {
			    puttext($1.end);
			    }
			}
	;

struct_declaration_list:
		struct_declaration		
	|	struct_declaration_list L_SEMI  { 
			if (!executable) {
				 puttext($2.end);
			}
		}
		struct_declaration
	;

struct_declaration:
		type_specifier_list struct_declarator_list
	;

/* Allow empty member list for backward compatibility. */

struct_declarator_list:
		s_struct_declarator		
	|	struct_declarator_list L_COMMA s_struct_declarator
	|	/*EMPTY*/		{ ; }
	;

/* Put semantics around struct/union declarator. */

s_struct_declarator:
		struct_declarator 
	;
		
struct_declarator:
		declarator		
	|	declarator  L_COLON constant_expression
	|	/* EMPTY */ L_COLON constant_expression
	|	error			
	;

enum_specifier:
		enum id_or_type enu_list
	|	enum enu_list
 	|	enum id_or_type 
	;

enum:		L_ENUM			
	;

enu_list:	L_LC enumerator_list enum_opt_comma L_RC 
	;

enumerator_list:
		enumerator			
	|	enumerator_list L_COMMA enumerator
	;

enum_opt_comma:
		/*EMPTY*/	{ ; }
	|	L_COMMA			
	;

enumerator:
		id_or_type 
	|	id_or_type L_EQ constant_expression 
	;

declarator:
	 		direct_declarator
	|	pointer direct_declarator 
	;

direct_declarator:
		id_or_function {
			/* save the function name */
			if (save_header) {
				len = min($1.end - $1.start, IDMAX);
				 (void)strncpy(fcn_name, yytext + $1.start, (unsigned) len);
				fcn_name[len] = '\0';
			}
			/* see if save for typedef processing */
			if (typename) {
			/* save the name because it may be a typedef name */
				len = min($1.end - $1.start, IDMAX);
				 (void)strncpy(nf_name, yytext + $1.start, (unsigned) len);
				nf_name[len] = '\0';
				(void)add_symbol(nf_name, L_TYPENAME);
			}
		  }
	|	strfcn_name		/* special string function names */
	|	declL_LP declarator declL_RP 
	|	direct_declarator decl_array 
	|	direct_declarator funarg {
			static char tmp_name[IDMAX];
			/* save the function header */
			if (save_header) {
				fcn_line = yylineno;
				fcn_text[0] = '\n';
				 (void)strncpy(fcn_text + 1, yytext + $1.start, SAVEMAX - 1);
				puttext($2.end);
			}
			len = min($1.end - $1.start, IDMAX);
			(void)strncpy(tmp_name, yytext + $1.start, (unsigned) len);
			tmp_name[len] = '\0';
			(void)add_symbol(tmp_name, L_FUNCTION);
		  }
	;

decl_array:	L_LB opt_constant_expression L_RB 
	;

funarg:		funL_LP parameter_type_list funL_RP  {
			$$.end = $3.end;  }
	|	funL_LP identifier_list     funL_RP  {
			$$.end = $3.end;  }
	|	funL_LP			    funL_RP  {
			$$.end = $2.end;  }
	;

funL_LP:	L_LP 
	;
funL_RP:	L_RP 
	;
declL_LP:	L_LP 
	;
declL_RP:	L_RP 
	;

pointer:
	 	ptrL_STAR			
	|	ptrL_STAR type_specifier_list   
	|	ptrL_STAR type_specifier_list pointer
	|	ptrL_STAR		     pointer
	;

type_specifier_list:
		type_specifier
	|	type_specifier_list type_specifier
	;

/* Bracket pointer stuff to get const/volatile modifiers.
** Attach modifier to non-terminal.
*/

ptrL_STAR:
		L_STAR	
	;

/* f(...) not allowed in strictly conforming ANSI C */
parameter_type_list:
		parameter_list
	|	L_DOTDOTDOT		/* extension */
	|	parameter_list L_COMMA L_DOTDOTDOT
	;

parameter_list:
		parameter_declaration
	|	parameter_list L_COMMA parameter_declaration
	;


/* To avoid reduce/reduce conflicts, break out pieces of
** s_declarator and abstract_declarator, and put them
** around p_decl.
*/
parameter_declaration:
		s_declaration_specifiers p_decl
	;

p_decl:		/* EMPTY */		{ ; }
	|	declarator
	|	abstract_declarator
	;

identifier_list:
		id_or_function
	|	identifier_list L_COMMA id_or_function
	|	error	
	;

id_or_function:
		L_IDENT
	|	L_FUNCTION
	;

type_name:
		type_specifier_list
		opt_abstract_declarator
	;

opt_abstract_declarator:
		/* EMPTY */		{ ; }
	|	abstract_declarator	
	;

abstract_declarator:
		pointer				   
	|	pointer direct_abstract_declarator 
	|		direct_abstract_declarator
	;


direct_abstract_declarator:
		declL_LP abstract_declarator declL_RP 
	|	direct_abstract_declarator decl_array
	|				   decl_array
	|	direct_abstract_declarator funL_LP opt_parameter_type_list funL_RP
	|				   funL_LP opt_parameter_type_list funL_RP
	;

opt_parameter_type_list:
		parameter_type_list
	|	/*EMPTY*/		{ ; }
	;

initializer:
		assignment_e	
	|	L_LC initializer_list opt_L_COMMA L_RC 
	;

opt_L_COMMA:
		L_COMMA
	|	/*EMPTY*/{ ; }
	;

initializer_list:
		initializer
	|	initializer_list L_COMMA
			{ puttext($2.end); } initializer
	;


/* Statements */

statement:
		labeled_statement
	|	pp_if_statement
	|	compound_statement
	|	expression_statement
	|	selection_statement
			{ /* there could have been embedded break statements. */
			flow_break = no;
			}
	|	iteration_statement
			{ /* there could have been embedded break statements. */
			flow_break = no;
			}
	|	jump_statement
			{
			flow_break = yes;
			}
	|	oldasm
	|	error L_SEMI
			{
			puttext(yyleng);
			}
	|	error L_RC
			{
			puttext(yyleng);
			}
	;

/* Recognize old-style asm() call. */
oldasm:		L_ASM L_LP L_STRING L_RP L_SEMI
	;

pp_if_statement:
		PP_IF { puttext(yyleng); (void)printf("\n"); }
		pp_else_statement
		PP_ENDIF { puttext(yyleng); (void)printf("\n"); }
	;

pp_else_statement:
		opt_statement_list
		PP_ELSE { puttext(yyleng); (void)printf("\n"); }
		opt_statement_list
	|	opt_statement_list
	;

labeled_statement:
		id_or_type L_COLON
			{
			puttext (yyleng);
			 tr_stmt (yylineno, yytext, yes);
			}
		statement
	|	L_CASE  constant_expression L_COLON
			{
			puttext (yyleng);
			 tr_stmt (yylineno, yytext, yes);
			}
		statement
	|	L_DEFAULT L_COLON
			{
			puttext (yyleng);
			 tr_stmt (yylineno, yytext, yes);
			}
		statement
	;

compound_statement:
		cmpdL_LC declaration_list
			{
			if (executable )
			{
				/* declare a temporary variable for
			  	** string function results
				** char pointer may be bigger than int 
				*/
				(void)printf("char *_ct;");
			}
			/* trace the function header */
			if (flow_break)
				flow_break = no;
			else
				tr_stmt(fcn_line, fcn_text, yes);
			/* will turn off tracing when finish
			** with a function_body  
			*/
			executable = yes;
			}
			 opt_statement_list cmpdL_RC
	|	cmpdL_LC
			{

			if (executable )
			{
				/* declare a temporary variable for
			  	** string function results
				** char pointer may be bigger than int 
				*/
				(void)printf("char *_ct;");
			}
			/* trace the function header */
			if (flow_break)
				flow_break = no;
			else
				tr_stmt(fcn_line, fcn_text, yes);
			/* will turn off tracing when finish
			** with a function_body  
			*/
			executable = yes;
			}
			opt_statement_list cmpdL_RC
	;

cmpdL_LC:	L_LC 
			{
			puttext(yyleng);

			/* trace the brace after any declarations */
			fcn_line = yylineno;
			(void)strncpy(fcn_text, yytext, SAVEMAX);
			executable = no;
			}
	;
cmpdL_RC:	L_RC 
	   		{
			if (flow_break)
				flow_break = no;
			else
				tr_stmt(yylineno, yytext, yes);
			puttext(yyleng);
			}
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
	|	/*EMPTY*/  { ; }
	;


expression_statement:
		opt_e L_SEMI
			{
			tr_stmt(yylineno, yytext, yes);
			tr_vars(0, yyleng);
			reset();
			}
	;

selection_statement:
		if_prefix %prec L_LOWEST_PREC	
	|	if_prefix L_ELSE 
			{
			puttext( yyleng);
			(void)putchar( '{');
			tr_stmt( yylineno, yytext, yes);
			}
		statement 
			{
			(void)putchar( '}' );
			}
	|	L_SWITCH L_LP e L_RP 
			{
			tr_stmt( yylineno, yytext, yes);
			tr_vars( 0, yyleng);
			reset();
			(void)putchar( '{' );
/* don't trace the '{' before the first case */
			flow_break = yes;
			}
		statement 
			{
			(void)putchar( '}' );
			}
	;

if_prefix:	L_IF L_LP e L_RP 
			{
			tr_stmt(yylineno, yytext, yes);
			tr_vars(0, yyleng);
			reset();
			(void)putchar('{');
			}
		statement
			{
			(void)putchar( '}' );
			}
	;

iteration_statement:
		L_WHILE L_LP e L_RP 
			{
			/* insert the statement trace after the "while(" */
			tr_vars(0, $2.end);
			tr_stmt(yylineno, yytext, no);
			if (trace_fcn && !too_long)
				(void)putchar(',');
			tr_vars($3.start, yyleng);
			reset();
			(void)putchar('{');
			}
		statement 
			{
			(void)putchar( '}' ); 
			}
	|	L_DO 
			{
			puttext(yyleng);
			(void)putchar('{');
			tr_stmt(yylineno, yytext, yes);
			}
		statement L_WHILE L_LP e L_RP L_SEMI
			{
			tr_stmt(yylineno, yytext, yes);
			(void)putchar('}');
			tr_vars(0, yyleng);
			reset();
			}
	|	L_FOR L_LP opt_e L_SEMI opt_e L_SEMI opt_e L_RP 
			{
			/* put the stmt trace before the first and last
			** exp because they may cause an execution error 
			*/
			tr_stmt(yylineno, yytext, yes);
			tr_vars(0, $6.end);
			tr_stmt(yylineno, yytext, no);
			if ($7.start != $7.end && trace_fcn && !too_long) 
				/* check for a non-null exp */
				(void)putchar(',');
			tr_vars($7.start, yyleng);
			reset();
			(void)putchar('{');
			}
		statement 
			{
			(void)putchar( '}' );
			}
	;

jump_statement:
		L_GOTO id_or_type L_SEMI
			{
			tr_stmt(yylineno, yytext, yes);
			puttext(yyleng);
			}
	|	L_CONTINUE L_SEMI
			{
			tr_stmt(yylineno, yytext, yes);
			puttext(yyleng);
			}
	|	L_BREAK L_SEMI
			{
			tr_stmt(yylineno, yytext, yes);
			puttext(yyleng);
			}
	|	L_RETURN opt_e L_SEMI
			{
			tr_stmt(yylineno, yytext, yes);
			tr_vars(0, yyleng);
			reset();
			}
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
	|	e L_COMMA assignment_e	
			{bop:
			symbol_info($$, $1, $3, max3($1.type, $3.type, repeatable));
}
	;

opt_e:
		e
	|	/*EMPTY*/
			{
			$$.start = $$.end = $<symbol>0.end; 
			}
	;

constant_expression:
		.e 
	;

opt_constant_expression:
		constant_expression
	|	/*EMPTY*/		  { ; }
	;

assignment_e:
		.e
	/* The next two productions are illegal ANSI C, but are added
	** to provide compatibility with UNIX C, which allowed benign
	** casts on the left side of assign-ops.  The left side of each
	** was originally unary_e.
	*/
	|	cast_e  L_ASGOP assignment_e    
			{
			symbol_info($$, $1, $3, max3($1.type, $3.type, repeatable));
			if ($1.type != side_effect)
				/* keep inner expressions lower on the trace stack */
				rm_trace($1); 

			add_trace($1, $1, $3, assign);
			}
	|       cast_e  L_EQ    assignment_e    
			{
			symbol_info($$, $1, $3, max3($1.type, $3.type, repeatable));
			if ($1.type != side_effect) {
	    		/* keep inner expressions lower on the stack
			** so *a++ = *b++ is traced properly 
			*/
				 rm_trace($1);

				/* don't trace a=b=1 */
				if (suppress && $3.type == constant)
					$$.type = constant;
				else {
				/* trace only a in a=b.  Note: (p = p->next)
				** cannot be traced as one variable.  Don't set
				** to variable because "(a=b)" will expand to
				** a term and then to a normal trace 
				*/

					if (suppress && $3.type == variable)
						 rm_trace($3);
					/* trace the assigned variable */
					add_trace($1, $1, $3, assign);
				}
			}
			}
	;

.e:
		.e L_QUEST e L_COLON .e
			{
			symbol_info($$, $1, $5,
			    max4($1.type, $3.type, $5.type, repeatable)); 
			}
	|	.e L_OROR .e			{ goto bop; }
	|	.e L_ANDAND .e			{ goto bop; }
	|	.e L_OR .e			{ goto bop; }
	|	.e L_XOR .e			{ goto bop; }
	|	.e L_AND .e			{ goto bop; }
	|	.e L_EQUALOP .e			{ goto bop; }
	|	.e L_RELOP .e			{ goto bop; }
	|	.e L_SHIFTOP .e			{ goto bop; }
	|	.e L_PLUS .e			{ goto bop; }
	|	.e L_MINUS .e			{ goto bop; }
	|	.e L_STAR .e			{ goto bop; }
	|	.e L_DIVOP .e			{ goto bop; }
	|	cast_e
			{
			if ($1.type == variable)
				add_trace($1, $1, $1, normal);
			}
	;

cast_e:
		unary_e
	|	L_LP type_name L_RP cast_e	
			{
			if ($4.type == variable)
				symbol_info( $$, $1, $4, variable);
			else
				symbol_info( $$, $1, $4, constant);
			}
	;

unary_e:
		s_postfix_e
/* next line was L_INCOP unary_e; changed for backward compatibility */
	|	L_INCOP cast_e
			{
			symbol_info($$, $1, $2, max($2.type, repeatable));
			if ($2.type != side_effect)
				add_trace($2, $1, $2, prefix);
			}
	|	L_UNARYOP cast_e	
			{
			unop:
			symbol_info($$, $1, $2, max($2.type, repeatable));
			if ($2.type == variable)
				add_trace($2, $2, $2, normal);
			}
	|	L_SIZEOF unary_e	
			{
			const_op:
			symbol_info($$, $1, $2, constant);
			if ($2.type == side_effect)
				$$.type = side_effect;
			/* don't trace sizeof(a) or &(a) */
			rm_trace($2);
			}
	|	L_SIZEOF L_LP type_name L_RP	
			{
			symbol_info($3, $2, $4, constant); 
			symbol_info($$, $1, $3, constant); 
			}
	/* spell out the ones that are also binary */
	|	L_AND cast_e		
			{
			goto const_op;
			}
	|	L_AND L_DOTDOTDOT	
	|	L_STAR cast_e		
			{
			symbol_info($$, $1, $2, $2.type);
			if ($2.type == variable) /* don't expand ++a to *++a */
				expand_trace($2, $1, $2);
			}
	|	L_PLUS cast_e		
			{
			goto unop;
			}
	|	L_MINUS cast_e		
			{
			goto unop;
			}
	;

postfix_e:
		primary_e
	|	s_postfix_e L_LB e L_RB	
			{
			symbol_info($$, $1, $4, max($1.type, $3.type));
			if ($$.type != side_effect)
				$$.type = variable;
			}
	|	func_call argument_expression_list L_RP
			{
			symbol_info($$, $1, $3, side_effect);
			/* prevent sizeof function compiler error */
			rm_trace($1);
			}
	|	func_call			  L_RP
			{
			symbol_info($$, $1, $2, side_effect);
			/* prevent sizeof function compiler error */
			rm_trace($1);
			}
	|	MACRO L_LP opt_e   L_RP {
			rm_all_trace($3);
			symbol_info($$, $1, $4, side_effect);
			}
	|	MACRO {
			symbol_info($$, $1, $1, side_effect);
			}
	|	s_postfix_e L_STROP id_or_type
			{
			symbol_info($$, $1, $3, $1.type);
			expand_trace($1, $1, $3);
			}
	|	s_postfix_e L_INCOP	
			{
			symbol_info($$, $1, $2, max($1.type, repeatable));
			if ($1.type != side_effect)
				add_trace($1, $1, $2, postfix);
			}
	|	strfcn
			{ ; }
	|	strfcn_name
			{
			$$.type = constant;
			}
	;

/* Check for undefined NAME in postfix_e. */
s_postfix_e:
		postfix_e		
	;
/* Function call:  check for undefined name, do the right thing. */
func_call:
		postfix_e L_LP
			{
			/*check for a new function name */
			$$.type = (enum symbol_type) add_symbol( yytext + token_start, constant);
			}
	;

argument_expression_list:
		assignment_e
	|	argument_expression_list L_COMMA assignment_e
	;

primary_e:
		L_IDENT			
			{
			$$.type = variable; 
			}
	|	L_FUNCTION
			{
			$$.type = constant; 
			}
	|	constant
			{
			$$.type = constant; 
			}
	|	string_list
			{
			$$.type = constant; 
			}
	|	L_LP e L_RP
			{
			symbol_info($$, $1, $3, $2.type);
			if ($2.type == variable) /* don't expand ++a to (++a) */
				expand_trace($2, $1, $3);
			}
/* {
#ifdef LINT
    ln_paren($2);
#endif
} */
	;

string_list:
		L_STRING
	|	string_list L_STRING {
			symbol_info($$, $1, $2, constant);
		}
	;

strfcn_name :	STRRES
	| 	STRCAT
	| 	STRNCAT
	| 	STRCPY
	| 	STRCMP
	| 	STRNCMP
	| 	STRLEN
	;

strfcn  :	STRRES L_LP assignment_e opt_args L_RP {
			symbol_info($$, $1, $5, side_effect);
			if ($3.type != side_effect)
				add_trace($3, $1, $5, strres); /* result */
			}
	|	STRCAT L_LP assignment_e L_COMMA assignment_e L_RP {
			symbol_info($$, $1, $6, side_effect);
			if ($3.type != side_effect) 
			/* $5 is evaluated only once so it can have side effects */
				add_trace($3, $1, $6, string); /* result */
			if ($5.type != side_effect && $5.type != constant)
				add_trace($5, $5, $5, string); /* arg 2 */
			}
	|	STRNCAT L_LP assignment_e L_COMMA assignment_e L_COMMA assignment_e L_RP {
			symbol_info($$, $1, $8, side_effect);
			if ($3.type != side_effect)
				add_trace($3, $1, $8, string); /* result */
			}
	|	STRCPY L_LP assignment_e L_COMMA assignment_e L_RP {
			symbol_info($$, $1, $6, side_effect);
			if ($3.type != side_effect) {
				/* don't trace strcpy(a, "b") */
				if (suppress && $5.type == constant) {
					$$.type = constant;
					rm_trace($3);
				}
				else {
					/* don't trace b in strcpy(a, b); */
					if (suppress && $5.type == variable) {
						$$.type = repeatable; 
						/* prevent term rule trace */
						rm_trace($5);
					}
					else if ($5.type != side_effect && $5.type != constant)			      
						add_trace($5, $5, $5, string); /* arg 2 */
				       /* trace the assigned variable */
					add_trace($3, $1, $6, string); /* result */
				}
			}
			}
	|	int_strfcn    { /* trace a repeatable string function result */
			if ($1.type != side_effect)
				add_trace($1, $1, $1, normal); /* result */
			}
	;

int_strfcn :	STRCMP L_LP assignment_e L_COMMA assignment_e L_RP {
			symbol_info($$, $1, $6, max($3.type, $5.type));
				add_trace($3, $3, $3, string); /* arg 1 */
			if ($5.type != side_effect && $5.type != constant)
				add_trace($5, $5, $5, string); /* arg 2 */
			}
	|	STRNCMP L_LP assignment_e L_COMMA assignment_e L_COMMA assignment_e L_RP {
			symbol_info($$, $1, $8, max3($3.type, $5.type, $7.type));
			}
	|	STRLEN L_LP assignment_e L_RP {
			symbol_info($$, $1, $4, $3.type);
			if ($3.type != side_effect && $3.type != constant)
				add_trace($3, $3, $3, string); /* arg 1 */
			}
	;
opt_args:	/* EMPTY */
	|	L_COMMA  assignment_e
	;

%%

static
yyerror(s)
char *s;
{
	if (last_yychar == MACRO    || yychar == MACRO
	 || last_yychar == PP_IF    || yychar == PP_IF
	 || last_yychar == PP_ELSE  || yychar == PP_ELSE
	 || last_yychar == PP_ENDIF || yychar == PP_ENDIF ) {
		fatal("cannot handle preprocessor code, use -P option");
	}
	else if (strcmp(s, "yacc stack overflow") == 0) {
		fatal("'if ... else if' sequence too long");
	}
	else if (strcmp(s, "syntax error") == 0) {
		error("possible syntax error, try -P option");
	}
	else {	/* no other known yacc errors, but better be safe than sorry */
		fatal(s);
	}
	/* NOTREACHED */
}
