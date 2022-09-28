/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/expand.c	1.1"
/*ident	"@(#)cfront:src/expand.c	1.20" */
/*****************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


expand.c:

	expand inline functions

******************************************************************/

#include "cfront.h"

char* temp(char* vn, char* fn, char* cn)
/*
	make the name of the temporary: _Xvn_fn_cn
*/
{	if (vn[0]!='_' || vn[1]!='X') {
		int vnl = strlen(vn);
		int fnl = strlen(fn);
		int cnl = (cn)?strlen(cn):0;
		char* s = new char[vnl+fnl+cnl+5];

		s[0] = '_';
		s[1] = 'X';
		strcpy(s+2,vn);
		s[vnl+2] = '_';
		strcpy(s+vnl+3,fn);
		if (cnl) {
			s[vnl+fnl+3] = '_';
			strcpy(s+vnl+fnl+4,cn);
		}
		return s;
	}
	else
		return vn;

}

Pname dcl_local(Ptable scope, Pname an, Pname fn)
{
	if (scope == 0) {
		error('s',"cannot expand inlineF needing temporaryV in nonF context");
		return an;
	}
	if (an->n_stclass == STATIC) {
		error('s',"static%n in inlineF",an);
		return an;
	}
	Pname cn = fn->n_table->t_name;
	char* s = temp(an->string,fn->string,(cn)?cn->string:0);

/*
	if ( strcmp( s, an->string ) && scope->look( an->string, 0 ))
		{ an->string = s; return an; }
*/

	Pname nx = new name(s);
	nx->tp = an->tp;
	PERM(nx->tp);
	nx->n_used = an->n_used;
	nx->n_assigned_to = an->n_assigned_to;
	nx->n_addr_taken = an->n_addr_taken;
	nx->n_xref = an->n_xref;
	nx->lex_level = an->lex_level;
	Pname r = scope->insert(nx,0);
	r->n_stclass = an->n_stclass;
	delete nx;
	r->where.line = 0;
	return r;
}

needs_zero( Pexpr e )
/*
 	inline expansion of if statement needs to check that both
	sides of generated ?: are of compatible types;   
	val = 1:: ?(...,0) 2:: :(...,0) 0,3:: no action 
*/
{
  	int val = 0, tmp = 1;
	Pexpr ee = e->e1;

xxx:
	if ( ee->tp == 0 ) return val;
	switch( ee->tp->base ) {
		case TYPE:
		case TNAME:
			Pbase b = (Pbase)ee->tp;
			if ( b->b_name->tp->base == COBJ ) val += tmp;
			break;
		case COBJ:
			val += tmp; break;
	}

	if ( tmp == 1 ) {
		tmp++; if (ee = e->e2) goto xxx; 
	}

	return( val );
}

int ck_cast( Ptype t1, Ptype t2 )
/*
	return a value of type t2 from a function returning a t1
	return 1 if cast is needed
*/
{

	while (t1->base == TYPE) t1 = Pbase(t1)->b_name->tp;
	while (t2->base == TYPE) t2 = Pbase(t2)->b_name->tp;

	if (t1 == t2) return 0;

	if (t1->base != t2->base) return 1;

	switch (t1->base) {
	case CHAR:
	case SHORT:
	case INT:
	case LONG:
		if (Pbase(t1)->b_unsigned != Pbase(t2)->b_unsigned) return 1;
	}

	if (t1->base == COBJ) {
		Pname nn = Pbase(t1)->b_name;

		if (Pclass(nn->tp)->csu==UNION ) return 0; 

		if (t2->base==COBJ && nn->tp==Pbase(t2)->b_name->tp) return 0;

		return 1;
	}

	return 0;
}

Pstmt stmt::expand()
/*
	copy the statements with the formal arguments replaced by ANAMES 

	called once only per inline function
	expand_tbl!=0 if the function should be transformed into an expression
	and expand_tbl is the table for local variables
*/
{
	if (this == 0) error('i',"0->S::expand() for%n",expand_fn);
//error('d',"stmt %d:%k s=%d e=%d l=%d",this,base,s,e,s_list);

	if (memtbl) {	/* check for static variables */
		register Ptable t = memtbl;
		register int i;
		for (register Pname n = t->get_mem(i=1); n; n=t->get_mem(++i)) {
			if (n->n_stclass == STATIC) {
				error('s',"static%n in inlineF",n);
				n->n_stclass = AUTO;
			}
			n->where.line = 0;
		}
	}

	if (expand_tbl) {	/* make expression */
		Pexpr ee;
		static ret_seen = 0;

		if (memtbl /*&& base!=BLOCK*/ ) { /* temporaries */
			int i;
			Pname n;
			Ptable tbl = memtbl;
			for (n = tbl->get_mem(i=1); n; n=tbl->get_mem(++i)) {
				if (n->base!=NAME || n->tp==any_type) continue;

				if ( base == BLOCK 
					&& tbl->real_block == this
					&& n->lex_level<2 ) continue;

				char *s = n->string;
				if ( s[0]=='_' && s[1]=='X' ) continue;

				Pname nn = dcl_local(scope,n,expand_fn);
				nn->base = NAME;
				n->string = nn->string;
			}
		}

		switch (base) {
		default:
			error('s',"%kS in inline%n",base,expand_fn);
			return (Pstmt)dummy;

		case BLOCK:
			if (s_list) {
				ee = (Pexpr) s_list->expand();
				if (s) {
					ee = new expr(CM, (Pexpr)s->expand(), ee);
					PERM(ee);
				}
				return (Pstmt) ee;
			}

			if (s) return s->expand();

			return (Pstmt) zero;

		case PAIR:
			ee = s2 ? (Pexpr)s2->expand() : 0;
			ee = new expr(CM, s?(Pexpr)s->expand():0, ee);
			if (s_list) ee = new expr(CM, ee, (Pexpr)s_list->expand());
			PERM(ee);
			return (Pstmt) ee;

		case RETURN:
			ret_seen = 1;
			s_list = 0;

			if ( e == 0 ) 
				ee = zero;
			else {

			ee = e->expand();
			Ptype tt = Pfct(expand_fn->tp)->returns;
			if ( ck_cast( tt, ee->tp ) )
				ee = new texpr( CAST, tt, ee );
			}
	
			return Pstmt(ee);

		case SM:
			if (e== 0 || e==dummy)
				ee = zero;
			else {
				if (e->base == DEREF) e = e->e1;
				ee = e->expand();
			}
			if (s_list) {
				ee = new expr(CM, ee, (Pexpr)s_list->expand());
				PERM(ee);
			}
			return (Pstmt)ee;

		case ASM:
			if (s_list) {
				ee = new expr(CM, ee, (Pexpr)s_list->expand());
				PERM(ee);
			}
			return (Pstmt)ee;

		case IF:
		{	ret_seen = 0;
			Pexpr qq = new expr(QUEST,Pexpr(s->expand()),0);
			qq->cond = e->expand();
			qq->e2 = else_stmt ? Pexpr(else_stmt->expand()) : zero;

		        switch( needs_zero( qq )) {
                        case 1: qq->e1 = new expr( CM, qq->e1, zero ); break;
                        case 2: qq->e2 = new expr( CM, qq->e2, zero ); break;
			}

			if (ret_seen && s_list) error('s',"S after \"return\" in inlineF");
			ret_seen = 0;
			if (s_list) qq = new expr(CM,qq,Pexpr(s_list->expand()));
			PERM(qq);
			return Pstmt(qq);
		}
		}
	}

	where.line = 0;

	switch (base) {
	default:
		if (e) e = e->expand();
		break;
	case PAIR:
		if (s2) s2 = s2->expand();
		break;
	case ASM:
	case BLOCK:
		break;
	case FOR:
		if (for_init) for_init = for_init->expand();
		if (e2) e2 = e2->expand();
		break;
	case LABEL:
	case GOTO:
	case RETURN:
	case BREAK:
	case CONTINUE:
		error('s',"%kS in inline%n",base,expand_fn);
	}

	if (s) s = s->expand();
	if (s_list) s_list = s_list->expand();
	PERM(this);
	return this;
}

Pexpr expr::expand()
{
	if (this == 0) error('i',"E::expand(0)");
//error('d',"%n: expr %d: b=%d %k e1=%d e2=%d",expand_fn,this,base,base,e1,e2);
	switch (base) {
	case NAME:
//error('d',"expr::expand(name %n %d)",this,Pname(this)->n_scope);
		if (expand_tbl && Pname(this)->n_scope==FCT) {
			Pname n = (Pname)this;
			char* s = n->string;
			if (s[0]=='_' && s[1]=='X') break;
			Pname cn = expand_fn->n_table->t_name;
			n->string = temp(s,expand_fn->string,(cn)?cn->string:0);
		}
	case DUMMY:
	case ICON:
	case FCON:
	case CCON:
	case IVAL:
	case FVAL:
	case LVAL:
	case STRING:
	case ZERO:
	case TEXT:
	case ANAME:
		break;
	case ICALL:
		if (expand_tbl && e1==0) {
			Pname fn = il->fct_name;
			Pfct f = (Pfct)fn->tp;
			if (f->returns==void_type
			&& f->s_returns!=int_type
			&& fn->n_oper!=CTOR)
				error('s',"non-value-returning inline%n called in value-returning inline%n",fn,expand_fn);
			else
				error("inline%n called before defined",fn);
		}
		break;
	case SIZEOF:
	case CAST:
		if (tp2) PERM(tp2);
		goto rrr;
	case QUEST:
		cond = cond->expand();
	default:
		if (e2) e2 = e2->expand();
	case REF:
	case DOT:
	rrr:
		if (e1) e1 = e1->expand();
		break;
	}

	PERM(this);
	return this;
}

bit expr::not_simple()
/*
	is a temporary variable needed to hold the value of this expression
	as an argument for an inline expansion?
	return 1; if side effect
	return 2; if modifies expression
*/
{
	int s;
/*error('d',"not_simple%k",base);*/
	switch (base) {
	default:
		return 2;
	case ZERO:
	case IVAL:
	case FVAL:
	case ICON:
	case CCON:
	case FCON:
	case STRING:
	case NAME:	/* unsafe (alias) */
		return 0;
	case SIZEOF:
		return (e1 == 0 || e1 == dummy) ? 0 : e1->not_simple();
	case G_ADDROF:
	case ADDROF:
		return e2->not_simple();
	case CAST:
	case DOT:
	case REF:
		return e1->not_simple();
	case UMINUS:
	case NOT:
	case COMPL:
		return e2->not_simple();
	case DEREF:
		s = e1->not_simple();
		if (1<s) return 2;
		if (e2==0) return s;
		return s |= e2->not_simple();
	case MUL:
	case DIV:
	case MOD:
	case PLUS:
	case MINUS:
	case LS:
	case RS:
	case AND:
	case OR:
	case ER:
	case LT:
	case LE:
	case GT:
	case GE:
	case EQ:
	case NE:
	case ANDAND:
	case OROR:
	case CM:
		s = e1->not_simple();
		if (1<s) return 2;
		return s |= e2->not_simple();
	case QUEST:
		s = cond->not_simple();
		if (1<s) return 2;
		s |= e1->not_simple();
		if (1<s) return 2;
		return s |= e2->not_simple();
	case ANAME:
		if (curr_icall) {
			Pname n = (Pname)this;
			int argno = n->n_val;
			for (Pin il=curr_icall; il; il=il->i_next)
				if (n->n_table == il->i_table) goto aok;
			goto bok;
		aok:
			return (il->local[argno]) ? 0 : il->arg[argno]->not_simple();
		}
	bok:	error('i',"expand aname%n",this);
	case G_CM:
	case VALUE:
	case NEW:
	case CALL:
	case G_CALL:
	case ICALL:
	case ASSIGN:
	case INCR:
	case DECR:
	case ASPLUS:
	case ASMINUS:
	case ASMUL:
	case ASDIV:
	case ASMOD:
	case ASAND:
	case ASOR:
	case ASER:
	case ASLS:
	case ASRS:
		return 2;
	}
}


Pexpr fct::expand(Pname fn, Ptable scope, Pexpr ll)
/*
	expand call to (previously defined) inline function in "scope"
	with the argument list "ll"
	(1) declare variables in "scope"
	(2) initialize argument variables
	(3) link to body
*/
{
//error('d',"expand%n inline=%d body %d defined %d f_expr %d last_exp=%d curr_expr=%d",fn,f_inline,body,defined ,f_expr,last_expanded,curr_expr);
	if ((body==0 && f_expr==0)			// before defined
	||  ((defined&SIMPLIFIED)==0)			// before simplified
	||  (Pfct(fn->tp)->body->memtbl==scope)		// while defining
	||  (f_inline>2)				// recursive call
	||  (last_expanded && last_expanded==curr_expr)	// twice in an expression
	) {
		if (f_inline == 2 || f_inline == 4)
 			error( 's', "cannot expand%n twice inE: %n declared as non-inline but defined as inline", fn, fn);

		fn->take_addr();			/* so don't expand */
		if (fn->n_addr_taken == 1) {
			Pname nn = new name;		/* but declare it */
			*nn = *fn;
			nn->n_list = dcl_list;
			nn->n_sto = STATIC;
			dcl_list = nn;
		}
		return 0;
	}

	f_inline += 2;

	Pin il = new iline;
	Pexpr ic = new texpr(ICALL,0,0);
	il->fct_name = fn;
	ic->il = il;
	ic->tp = returns;
	Pname n;
	Pname at = (f_this) ? f_this : (f_result) ? f_result : argtype;
	if (at) il->i_table = at->n_table;
	int i = 0;
	int not_simple = 0;	/* is a temporary argument needed? */

	for (n=at; n; n=n->n_list, i++) {
		/*	check formal/actual argument pairs
			and generate temporaries as necessary
		*/
		if (ll == 0) error('i',"F::expand(%n):AX",fn);
		Pexpr ee;

		if (ll->base == ELIST) {
			ee = ll->e1;
			ll = ll->e2;
		}
		else {
			ee = ll;
			ll = 0;
		}

		int s;	/* could be avoided when expanding into a block */
		Pname nn = 0;
//error('d',"n=%n addr %d ass %d used %d ee %k => %d",n,n->n_addr_taken,n->n_assigned_to,n->n_used,ee->base,ee->not_simple());
		if (n->n_assigned_to == FUDGE111) {	/* constructor's this */
			if (ee!=zero && ee->not_simple()==0) {
			// automatic or static, then we can use the actual variable
				il->local[i] = 0;
				goto zxc;
			}
		}

		if (n->n_addr_taken || n->n_assigned_to) {
			nn = dcl_local(scope,n,fn);
			nn->base = NAME;
			il->local[i] = nn;
			++not_simple;
		}
		else if (n->n_used
		&& (s=ee->not_simple())
		&& (1<s || 1<n->n_used) ) {	// not safe
			nn = dcl_local(scope,n,fn);
			nn->base = NAME;
			il->local[i] = nn;
			++not_simple;
		}
		else if (/*n==f_this &&*/ ee->not_simple()) {	// (this) not used: evaluate it anyway
//error('d',"this%n u %d a %d & %d",n, n->n_used, n->n_assigned_to,n->n_addr_taken);
			nn = dcl_local(scope,n,fn);
			nn->base = NAME;
			il->local[i] = nn;
			++not_simple;
		}
		else
			il->local[i] = 0;
	zxc:
		il->arg[i] = ee;
		il->tp[i] = n->tp;
	}
	
	Ptable tbl = body->memtbl;
	if (f_expr) {		// generate comma expression
		char loc_var = 0;
		/* look for local variables needing declaration: */
//error('d',"fn%n tbl %d",fn,tbl);
		for (n=tbl->get_mem(i=1); n; n=tbl->get_mem(++i) ) {
//error('d',"?%s: b=%d u%d =%d &=%d",n->string,n->base,n->n_used,n->n_assigned_to,n->n_addr_taken);
			if (n->base==NAME	// don't re-declare the args
			&& (n->n_used || n->n_assigned_to || n->n_addr_taken)) {
				Pname nn = dcl_local(scope,n,fn);
				nn->base = NAME;
				n->string = nn->string;
				loc_var++;
			}
		}

//error('d',"not_simple=%d loc_var=%d last_expanded=%d curr_expr=%d",not_simple,loc_var,last_expanded,curr_expr);
		if (not_simple || loc_var) last_expanded = curr_expr;

		Pexpr ex;
		if (not_simple) {
			Pexpr etail = ex = new expr(CM,0,0);
			for (i=0; i<MIA; i++) {
				Pname n = il->local[i];
				if (n == 0) continue;
				Pexpr e = il->arg[i];
//error('d',"mia%n: %k %d",n,e->base,e->base);
				etail->e1 = new expr(ASSIGN,n,e);
				if (--not_simple)
					etail = etail->e2 = new expr(CM,0,0);
				else
					break;
			}
			etail->e2 = f_expr;
		}
		else {
			ex = f_expr;
		}
		ic->e1 = ex;
	}
	else {		// generate block:
		for (n=tbl->get_mem(i=1); n; n=tbl->get_mem(++i) ) {
			// mangle local names
//error('d',"?%s: b=%d u%d =%d &=%d",n->string,n->base,n->n_used,n->n_assigned_to,n->n_addr_taken);
			if (n->base==NAME
			&& (n->n_used || n->n_assigned_to || n->n_addr_taken)) {
				Pname cn = fn->n_table->t_name;
				n->string = temp(n->string,fn->string,(cn)?cn->string:0);
			}
		}
		Pstmt ss;
		if (not_simple) {
			last_expanded = curr_expr;
			Pstmt st = new estmt(SM,curloc,0,0);
			st->where.line = 0;
			Pstmt stail = st;
			for (i=0; i<MIA; i++) {
				Pname n = il->local[i];
				if (n == 0) continue;
				Pexpr e = il->arg[i];
				stail->e = new expr(ASSIGN,n,e);
//error('d',"in block %n = %k",n,e->base);
				if (--not_simple) {
					stail = stail->s_list = new estmt(SM,curloc,0,0);
					stail->where.line = 0;
				}
				else
					break;
			}
			stail->s_list = body;
			ss = new block(curloc,0,st);
			ss->where.line = 0;
		}
		else {
			ss = body;
		}
		ic->e2 = Pexpr(ss);
	}

	f_inline -=2;
	return ic;
}
