/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/expr.h	1.6"

/*
Because of the case of unresolved symbol addresses and forward
references, evaluation of expressions is done partly in pass 1 and
partly in pass 2.  During pass 1, expressions are reduced bottom-up
as much as possible.  Pass 2 then completes the evaluation of those
expressions that were unfinished.  

Note that an expression with more than one external symbol falls
under the illegal case, as there is presently no way to pass the
proper relocation info to the link editor for it.  These expressions
are not detected until pass 2, so expression trees get built for
these too.  
*/


/* The following are expression fields to insert into type
definition of the "OPERAND" struct, used in pass 1. */
#define EXPR_MEMBERS	\
	unsigned char  exptype; \
	unsigned char  unevaluated;\
	symbol	*symptr;\
	long	expval;

/* Possible values for the exptype field: */
#define UNDEF	000
#define ABS	001
#define REL	002

/* Here's how to extract an exptype value from a symbol pointer: */
#define get_sym_exptype(sym) \
  ( (sym)->sectnum==SHN_ABS ? ABS : \
   ((sym)->sectnum==SHN_UNDEF||(sym)->sectnum==SHN_COMMON ? UNDEF : REL) \
  )

/* In pass 1, expressions can be partitioned into 4 sets: */
/* type 1 */
#define	ABSOLUTE(expr)	((expr).exptype == ABS)
	/* Expression has an absolute value, thus can be immediately
	evaluated.  The value can be directly obtained from the 
       	"expval" field.  Note that the following is synonymous:
		((expr).symptr == NULL)
	*/

/* type 2 */
#define RELOCATABLE(expr)	((expr).exptype == REL)
	/* Expression is of the form symbol+constant, where symbol
	has been previously defined and has a sectnum value between
	1 and SHN_LORESERVE-1.
	Thus expr holds a symbol pointer in field "symptr" and a
	constant in field "expval" (could be 0).
	*/

/* type 3 */
#define UNDEFINED(expr)	((expr).exptype == UNDEF && !(expr).unevaluated)
	/* Expression contains currently undefined or tentatively defined 
	symbols (i.e. symbols having sectnum value SHN_UNDEF or SHN_COMMON).  
	It is of form symbol+constant.  Thus expr holds a symbol pointer in
	field "symptr" and a constant in "expval" (could be 0). 
	Further evaluation of expr is done in pass 2 if possible
	(i.e. if the symbol becomes defined).  In most cases, this
	is done in one of the action routines.  However, if expr is
	a subexpression of another expression, expr is transformed
	into an expression of type 4 and becomes associated with a
	leaf of the tree for the larger expression, which is then
	evaluated in pass 2.
	*/

/* type 4 */
#define UNEVALUATED(expr)	((expr).unevaluated)
	/* Expression is non-simple, thus is left to be evaluated in
	pass 2.  Thus expr holds a pointer to an expression tree in
	field "symptr" (via casting).  In pass 2, it will be further
	evaluated and determined to be of type 1 or 2, undefined, or
	illegal.  Note that ((expr).exptype == UNDEF) also.
	*/

/* Expression tree structures, to postpone evaluation until pass 2. */
/* Trees are built while parsing expressions, then evaluated in pass 2 */
typedef struct exptreetag {
	char is_leaf;
	union
	{
		struct {
			struct exptreetag *left, *right;
			unsigned char op;
		} node;		/* is_leaf is zero */
		struct {
			long value;
			symbol *symptr;	/* NULL after eval in pass 2 */
			unsigned char exptype; /* not used til pass 2 */
		} leaf;		/* is_leaf is non-zero */
	} _union;
} EXPTREE;
#define t_node _union.node
#define t_leaf _union.leaf

/* values for the EXPTREE field "op" */
#define	UMINUS_OP	0x1
#define	PLUS_OP		0x2
#define	MINUS_OP	0x3
#define	MULT_OP		0x4
#define	DIVIDE_OP	0x5


#define REMEMBER_SIZE	0
#define REMEMBER_SET	1

/* The following data structure is to process of the ".set <id>, <expr>" 
directive for cases where <expr> contains a forward reference or
cannot otherwise be evaluated during parsing in pass 1. */
struct set_struct
{
	symbol	*sym;		/* symbol to be "set" (i.e. <id>) */
	BYTE	evalflag;	/* disposition of the evaluation */
	union			/* what to "set" symbol from (i.e. <expr>) */
	{
		EXPTREE *xptr;	/* expression tree? */
		symbol *sym;	/* another symbol? */
	} setfrom;
	unsigned short lineno;  /* line number for error handling */
	struct set_struct *next;	/* create a linked list */
};
/*  The following flags for the "evalflag" field of set_struct. */
#define SETTO_XPTR	1	/* The symbol's value is obtained by
				evaluating an expression tree pointed
				to by the "setfrom.xptr" field. */
#define SETTO_SYM	2	/* The symbol's value is obtained by
				getting the value pointed to by the
				"setfrom.sym" field. */ 
#define	BEING_EVALD	3	/* The symbol's value is in the process
				of being determined.  This flags ensures 
				that we don't hit a cycle in the
				evaluation. */
#define	EVALUATED	0	/* The symbol's value is good. */

#define SET_SIZE	4	/* The symbol's size is obtained from
				from another symbol as in the case of
				weak symbols. */

/* interfaces to Generate() */
#define generate(nbits,action,value,sym) \
	Generate((BYTE) (nbits),(unsigned char) (action), \
		(long) (value),(symbol *) (sym),(BYTE) 0)

#define exp_generate(nbits,action,exp) \
	Generate((BYTE) (nbits),(unsigned char) (action), \
		(long) (exp).expval,(symbol *) (exp).symptr, \
		(BYTE)UNEVALUATED(exp))
