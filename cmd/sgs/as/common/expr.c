/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/expr.c	1.13"
#include <stdio.h>
#include <syms.h>
#include "symbols.h"
#include "gendefs.h"
#include "section.h"

#if i386
#include "instab.h"
#define OPND_TYPE rexpr
#else /* M32 */
#include "program.h"
#define OPND_TYPE OPERAND
#endif

/*
 *	The following routines are used by expr1() to build expression
 *	trees from information from the fields of OPND_TYPE.
 */

/*
 *	Change an expression to type 4 (see expr.h) by making it point to
 *	an expression tree
 */
#define point_to_tree(expr,tree)	(expr)->exptype = UNDEF; \
					(expr)->unevaluated = 1;\
					(expr)->expval = 0;\
					(expr)->symptr = (symbol *)tree;
extern unsigned short line;
extern void	deflab();
extern void aerror();


/*
 *	create an expression tree leaf from operand info and have
 *	"symptr" field of operand point to it. This function is called to
 *	transform an expression of type 3 to type 4 (see expr.h).
 */
void
treeleaf(expr)
OPND_TYPE *expr;
{
	EXPTREE *tree;
	tree = (EXPTREE *)alloc(sizeof(EXPTREE));
	tree->is_leaf = 1;
	tree->t_leaf.symptr = expr->symptr;
	tree->t_leaf.value = expr->expval;
	point_to_tree(expr,tree)
}

/*
 *	create an expression tree node (non-terminal) and have the
 *	"symptr" field of root point to it.  lopnd and ropnd are assumed
 *	to hold pointers to EXPTREE structures.
 */
void
treenode(op,lopnd,ropnd,root)
unsigned char op;
OPND_TYPE *lopnd, *ropnd, *root;
{
	register EXPTREE *tree;
	tree = (EXPTREE *)alloc(sizeof(EXPTREE));
	tree->is_leaf = 0;
	tree->t_node.op = op;
	if (lopnd)
		tree->t_node.left = (EXPTREE *)(lopnd->symptr);
	else
		tree->t_node.left = NULL;
	if (ropnd)
		tree->t_node.right = (EXPTREE *)(ropnd->symptr);
	else
		tree->t_node.right = NULL;
	point_to_tree(root,tree);
}

/*
 *	This is to handle the case of an expression where one term is of
 *	type 4 and the other term is of type 1 and the operation is addition.
 *	We simply add the value from the type 1 term into the appropriate
 *	field in the tree structure for the type 4 term.  We also transform
 *	the first term to type 4, if necessary, for canonical reasons.
 */
OPND_TYPE *
merge(term1,term2)
OPND_TYPE *term1, *term2;
{
	if (ABSOLUTE(*term2)) {
		if (((EXPTREE *)(term1->symptr))->is_leaf)
			((EXPTREE *)(term1->symptr))->t_leaf.value += term2->expval;
		else {
			treeleaf(term2);
			treenode(PLUS_OP,term1,term2,term1);
		}
		return(term1);
	} else {
		((EXPTREE *)(term2->symptr))->t_leaf.value += term1->expval;
		point_to_tree(term2,(EXPTREE *)(term2->symptr))
		return(term2);  
	}
}




/*
 *	The following routines are used to handle the case of an
 *	unevaluated expression or a forward reference found while parsing
 *	".set <id>, <expr>" in yyparse().  If <expr> contains a forward
 *	reference or is an otherwise unevaluated expression, we delay
 *	"setting" the fields for the symbol table entry for <id> by
 *	allocating a structure on a linked list.  The structure is used to
 *	store (1) a pointer to the symbol table entry for <id>, (2) a flag
 *	indicating whether the information for <expr> is in a symbol table
 *	entry or in an expression tree, and (3) a pointer to either the
 *	symbol table entry or the expression tree that holds the information
 *	for <expr>. 
 *
 *	After parsing the entire file,  we traverse the linked list and
 *	correct the fields of the symbol table entry for each <id>.  This may
 *	call for either reducing an expression tree or for copying values
 *	from fields of another symbol table entry.
 */

static struct set_struct *s_chain;
static struct set_struct *size_chain;

extern symbol *dot;
extern symbol *lookup();
/*
 *	This function is called by remember_set() to check for an
 *	expression tree with a pointer to the symbol dot. It returns a
 *	pointer to the leaf where the pointer to dot appears, else NULL.
 */
static EXPTREE * findleafwdot(root)
EXPTREE *root;
{
	EXPTREE *p;
	if (root == NULL)
		return(NULL);
	else if (root->is_leaf) {
		if (root->t_leaf.symptr == dot)
			return(root);
		else
			return(NULL);
	}
	else if (p = findleafwdot(root->t_node.left))
		return(p);
	else
		return(findleafwdot(root->t_node.right));
}

/*	This function is called by the ".set" handler to delay
 *	computation of the "value" field of sym.  A set_struct is allocated
 *	and chained on s_chain and the relevant info is stored.
 */
void
remember_set_or_size(sym,ptr,flag,set_or_size)
symbol *sym;
VOID *ptr;
BYTE flag;
BYTE set_or_size;
{
	register struct set_struct *temp;

	temp = (struct set_struct *)alloc(sizeof(struct set_struct));
	temp->sym = sym;
	temp->evalflag = flag;
	temp->lineno = line;
	if (flag == SETTO_SYM || flag == SET_SIZE)
		temp->setfrom.sym = (symbol *)ptr;
	else if (flag == SETTO_XPTR) { 
		EXPTREE *p;
		if (p = findleafwdot((EXPTREE *)ptr)) {
		/* dot can't be "remembered" so we must replace its
		 * occurrence in the tree with a newly installed temporary
		 * symbol which holds the current info from dot
		 */
			static int uniq = 0;
			char newlab[20];
			register symbol *s;	
			(void) sprintf(newlab,".DOT#%d",uniq++);
			p->t_leaf.symptr = s = lookup(newlab,INSTALL);
			s->value = dot->value;
			s->sectnum = dot->sectnum;
			s->binding = STB_LOCAL;
			s->flags &= ~GO_IN_SYMTAB;
			if (sectab[dot->sectnum].flags & SHF_EXECINSTR)
				deflab(s);
		}
		temp->setfrom.xptr = (EXPTREE *)ptr;
	}
	if (set_or_size == REMEMBER_SET) {
		temp->next = s_chain;
		s_chain = temp;
		} 
	else {
		temp->next = size_chain;
		size_chain = temp;
	}
}

/*
 *	This function traverses s_chain to see if any of them holds sym. 
 *	If yes, return a pointer to the structure.
 */
static
struct set_struct *findsyminchain(sym,chain)
register symbol *sym;
register struct set_struct *chain;
{
	register struct set_struct *s;
	for (s = chain;s;s = s->next)
		if (s->sym == sym)
			return(s);
	return(NULL);
}

/*
 * 	correctset() is called, after parsing the entire file, to process a
 * 	set_struct allocated in remember_set().
 */

static void
correctset(s)
register struct set_struct *s;
{
	void tree_eval();
	register symbol *sym;
	register symbol *sym2;
	register EXPTREE *xptr;
	struct set_struct *s2;

	line = s->lineno;
	if (s->evalflag == BEING_EVALD)
		aerror("cycle in expression caused by .set");
	sym = s->sym;
	if (s->evalflag == SETTO_SYM) {
		s->evalflag = BEING_EVALD;
		sym2 = s->setfrom.sym;
		if (s2 = findsyminchain(sym2,s_chain))
			/* to ensure sym2->value is good */
			correctset(s2);
		sym->value += sym2->value;
		sym->sectnum = sym2->sectnum;
		if (WEAK(sym)) {
			if (COMMON(sym2))
				aerror("cannot set a weak symbol to a common symbol");
			if (!BIT_IS_ON(sym,SIZE_SET)){
				if (s2 = findsyminchain(sym2,size_chain))
			
					remember_set_or_size(sym,sym2,
							     SET_SIZE,REMEMBER_SIZE);
				else
					sym->size = sym2->size;
			}
			if (!BIT_IS_ON(sym,TYPE_SET))
				sym->type = sym2->type;
		}
		s->evalflag = EVALUATED;
	} else if (s->evalflag == SETTO_XPTR) {
		s->evalflag = BEING_EVALD;
		tree_eval(xptr = s->setfrom.xptr);
		sym->value = xptr->t_leaf.value;
		if (sym2 = xptr->t_leaf.symptr) {
			sym->value += sym2->value;
			sym->sectnum = sym2->sectnum;
			if (WEAK(sym)) {
				if (!BIT_IS_ON(sym,SIZE_SET))
					remember_set_or_size(sym,sym2,SET_SIZE,REMEMBER_SIZE);
				if (!BIT_IS_ON(sym,TYPE_SET))
					sym->type = sym2->type;
			}
		} else
			sym->sectnum = SHN_ABS;
		s->evalflag = EVALUATED;
	}
}

/* This function traverses the chain of set_struct structures and
 * calls correctset() on each. This function is called in aspass1()
 * after yyparse() is called.
 */
void
correctallsets()
{
	register struct set_struct *s;

	for (s = s_chain;s;s = s->next)
		correctset(s);
}


/*
 *	This function provides the second half of the handler for
 *	unevaluated expressions.  The function recursively traverses and
 *	evaluates an expression tree.  Constant values and symbol values are
 *	propagated up the tree, from the leaves to the root.  Values are
 *	merged as they move up the tree, based on the appropriate operators.
 *	Ultimately the root becomes a leaf with the correct values for the
 *	original exprssion.
 */
void
tree_eval(root)
EXPTREE *root;
{
	register EXPTREE *left, *right;
	register symbol *symptr;
	struct set_struct *s;
	char errmsg[100];
	if (root->is_leaf) {
		if (symptr = root->t_leaf.symptr) {
			if (s = findsyminchain(symptr,s_chain))
				/* to ensure symptr->value is good */
				correctset(s);
#if DEBUG
			(void) printf("(%s=%d + %d) ",symptr->name,
				symptr->value,root->t_leaf.value);
#endif
			if ((root->t_leaf.exptype = get_sym_exptype(symptr)) == ABS){
				root->t_leaf.value += symptr->value;
				root->t_leaf.symptr = NULLSYM;
			}
		} else {
			root->t_leaf.exptype = ABS;
#if DEBUG
			(void) printf("(%d) ",root->t_leaf.value);
#endif
		}
	} else {
		if (left = root->t_node.left)
			tree_eval(left);
		if (right = root->t_node.right)
			tree_eval(right);
#if DEBUG
		(void) printf("<%d> ",root->t_node.op);
#endif
		switch (root->t_node.op) {
		case UMINUS_OP:
			/* right child must be absolute */
			/* left child is null */
			if (right->t_leaf.exptype != ABS) {
				(void)sprintf(errmsg,"Illegal unary minus in ... \"-%s\"\n",
					right->t_leaf.symptr->name);
				aerror(errmsg);
			}
			root->t_leaf.value = - (right->t_leaf.value);
			root->t_leaf.exptype = ABS;
			root->t_leaf.symptr = NULLSYM;
			break;
		case PLUS_OP:	
			/* one child must be absolute */
			if (left->t_leaf.exptype == ABS) {
				/* abs + (abs/rel/undef) */
				root->t_leaf.exptype = right->t_leaf.exptype;
				root->t_leaf.symptr = right->t_leaf.symptr;
			} else if (right->t_leaf.exptype != ABS)
				/* (rel/undef) + (rel/undef) */ {
				(void)sprintf(errmsg,"Illegal addition in ... \"%s + %s\"\n",
					left->t_leaf.symptr->name, 
					right->t_leaf.symptr->name);
				aerror(errmsg);
			} else {
				/* (rel/undef) + abs */
				root->t_leaf.exptype = left->t_leaf.exptype;
				root->t_leaf.symptr = left->t_leaf.symptr;
			}
			root->t_leaf.value = left->t_leaf.value
						+ right->t_leaf.value;
			break;
		case MINUS_OP:	
			if (left->t_leaf.exptype == ABS) {
				if (right->t_leaf.exptype != ABS) {
					/* abs - (rel/undef) */
					(void)sprintf(errmsg,
						"Illegal subtraction in ... \"%s - %s\"\n",
						left->t_leaf.symptr->name, 
						right->t_leaf.symptr->name);
					aerror(errmsg);
					}
				/* abs - abs */
				root->t_leaf.exptype = ABS;
				root->t_leaf.symptr = NULL;
			} else if (right->t_leaf.exptype == ABS) {
				/* (rel/undef) - abs */
				root->t_leaf.exptype = left->t_leaf.exptype;
				root->t_leaf.symptr = left->t_leaf.symptr;
			} else if ((right->t_leaf.exptype == UNDEF) ||
				   (left->t_leaf.exptype == UNDEF)) {
				/* undef-rel or rel-undef or undef-undef */
				(void)sprintf(errmsg,"Illegal subtraction in ... \"%s - %s\"\n",
					left->t_leaf.symptr->name, 
					right->t_leaf.symptr->name);
				aerror(errmsg);
			} else {
				/* rel - rel */
				if (right->t_leaf.symptr->sectnum != left->t_leaf.symptr->sectnum)	
					aerror("Illegal subtraction - symbols from different sections");
				root->t_leaf.exptype = ABS;
				root->t_leaf.symptr = NULL;
				left->t_leaf.value += left->t_leaf.symptr->value;
				right->t_leaf.value += right->t_leaf.symptr->value;
			}
			root->t_leaf.value = left->t_leaf.value
						- right->t_leaf.value;
			break;
			
		case MULT_OP:	
			/* both children must be absolute */
			if (left->t_leaf.exptype != ABS ||
			    right->t_leaf.exptype != ABS) {
				(void)sprintf(errmsg,"Illegal multiplication in ... \"%s * %s\"\n",
					left->t_leaf.symptr->name, 
					right->t_leaf.symptr->name);
				aerror(errmsg);
			}
			root->t_leaf.exptype = ABS;
			root->t_leaf.symptr = NULL;
			root->t_leaf.value = left->t_leaf.value
						* right->t_leaf.value;
			break;
		case DIVIDE_OP:
			/* both children must be absolute */
			if (left->t_leaf.exptype != ABS ||
			    right->t_leaf.exptype != ABS) {
				(void)sprintf(errmsg,"Illegal division in ... \"%s / %s\"\n",
					left->t_leaf.symptr->name, 
					right->t_leaf.symptr->name);
				aerror(errmsg);
			}
			root->t_leaf.exptype = ABS;
			root->t_leaf.symptr = NULL;
			root->t_leaf.value = left->t_leaf.value
						/ right->t_leaf.value;
			break;
			
		}
		root->is_leaf = 1;
	}
}


static void
correctsize(s)
register struct set_struct *s;
{
	void tree_eval();
	register symbol *sym;
	register symbol *sym2;
	register EXPTREE *xptr;
	struct set_struct *s2;

	line = s->lineno;
	if (s->evalflag == BEING_EVALD)
		aerror("cycle in expression caused by .size");

	sym = s->sym;
	if (s->evalflag == SETTO_SYM) {
		s->evalflag = BEING_EVALD;
		sym2 = s->setfrom.sym;
		if (s2 = findsyminchain(sym2,size_chain))
			/* to ensure sym2->value is good */
			correctsize(s2);
		if (!(ABS_SYM(sym2)))
			aerror("Expression in '.size' must be absolute");
		sym->size += sym2->value;
		s->evalflag = EVALUATED;
	} else if (s->evalflag == SET_SIZE) {
		s->evalflag = BEING_EVALD;
		sym2 = s->setfrom.sym;
		if (s2 = findsyminchain(sym2,size_chain))
			/* to ensure sym2->value is good */
			correctsize(s2);
		sym->size = sym2->size;
		s->evalflag = EVALUATED;
	} else if (s->evalflag == SETTO_XPTR) {
		tree_eval(xptr = s->setfrom.xptr);
		sym->size = xptr->t_leaf.value;
		if (sym2 = xptr->t_leaf.symptr) 
			sym->size += sym2->value;
		s->evalflag = EVALUATED;
	}
}

void
correctallsizes()
{
	register struct set_struct *s;

	for (s = size_chain;s;s = s->next)
		correctsize(s);
}
