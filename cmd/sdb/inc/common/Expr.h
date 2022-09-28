/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Expr.h	1.8"

#ifndef EXPR_H
#define EXPR_H

// Expr.h -- Public interface to libexp
//
// usage: construct an Expr from a Symbol, char *, or expression tree,
// then either
//	(1) print(), or
//	(2) printaddr(), or
//	(3) eval()
//   then you may use any of lvalue(), rvalue(), or lhs_symbol()
//
// special case: if root of etree is ikCALL, should use do_call()
//
// A null Process pointer implies "current_process".
// A ~0 pc implies the current pc.
// A null Frame pointer implies the topmost frame.
//
// eval() returns 1 if the expression evaluates to a single rvalue, 0 otherwise
//
// print() and printaddr() return the number of items printed, 0 if an error
//
// lvalue() and rvalue() return 1 for success, 0 for failure
//
// lhs_symbol() returns a null Symbol if there is no addressible symbol
// associated with the expression
//

#include "Itype.h"
#include "Symbol.h"
#include "TYPE.h"
#include "Label.h"
#include "Rvalue.h"
#include "str.h"

class  Frame;
class  Process;
class  SDBinfo;
struct Place;
class  SymbolStream;		// private to Expr
class  Decomp;			// private to Expr

#define	EV_LHS	0	/* modes for Expr::eval() */
#define EV_RHS	1

class Expr {
    char         *estring;		// original text
    char	 *lab;			// label (may differ from estring)
    int		  own_tree;		// true if can delete etree
    SDBinfo	 *etree;		// expression tree
    Place	 *place;		// "l-value"
    Process	 *process;		// remembered for evaluate()
    Frame	 *frame;		// ditto
    Rvalue	 *rval;			// "r-value"
    int		  mode;			// evaluation mode, private
    Symbol	  current_sym;		// for name resolution
    int		  yield_count;		// trips though resolve()
    int parse();			// convert from estring to etree
    int resolve(Iaddr pc);		// convert from name to symbol
    int find_lval(Iaddr pc);		// convert from symbol to place
    int evaluate(char *fmt);		// convert from place to rval,
					// or print, if so requested
    Rvalue	 *get_rvalue(int);	// utility function
    SymbolStream *symbolstream;		// symbol iterator
    Decomp	 *decomp;		// decomposition iterator
    void	  do_decomps();		// decomposition utility func
    Place	 *base;			// "l-value" of root symbol
    TYPE	  type;			// type of current decomposition
    Label	  decomp_lab;		// current decomposition
    Rvalue	 *returnval;		// for printing result of fcn call
    int		  isbitfield;		// true if current decomp is a bitfield
    int		  bits;			// size in bits, if isbitfield
    int		  bito;			// offset in bits, if isbitfield
public:
    Expr(Symbol);	// shortcut for function arguments
    Expr(char *e);	// used by watchpoints
    Expr(SDBinfo *t);	// from eval_expr() and friends
    Expr(Expr &);	// used by watchpoint constructor
    ~Expr();

    Expr& operator=(Expr&);

    void	 label(char *l)	{ lab = str(l); }
    char	*label()	{ return lab; }
    char	*string()	{ return estring; }
    SDBinfo	*tree()		{ return etree; }

    int eval(int mode = EV_RHS, Process * = 0, Iaddr pc = ~0, Frame * = 0);
    int print(char *fmt, Process * = 0, Iaddr pc = ~0, Frame * = 0);
    int printaddr(char *fmt, Process * = 0, Iaddr pc = ~0, Frame * = 0);
    int do_call(char *fmt, Process * = 0, Iaddr pc = ~0, Frame * = 0);

    int lvalue(Place&);

    int rvalue(Rvalue&);

    int assign(Rvalue);

    Iaddr next_addr();

    Symbol	lhs_symbol();
};

extern Symbol function_symbol( char *name, Process *, Iaddr pc );

#endif
