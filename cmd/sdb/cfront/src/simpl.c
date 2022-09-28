/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/simpl.c	1.1"
/*ident	"@(#)cfront:src/simpl.c	1.31" */
/*******************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


simpl.c:

	simplify the typechecked function
	remove:		classes:
				class fct-calls
				operators
				value constructors and destructors
			new and delete operators (replace with function calls)
			initializers		(turn them into statements)
			constant expressions		(evaluate them)
			inline functions		(expand the calls)
			enums				(make const ints)
			unreachable code		(delete it)
	make implicit coersions explicit

	in general you cannot simplify something twice

*******************************************************************/

#include "cfront.h"
#include "size.h"
#include <ctype.h>

Pname make_tmp(char, Ptype, Ptable);
Pexpr init_tmp(Pname, Pexpr, Ptable);

Pname new_fct;
Pname del_fct;
Pname vec_new_fct;
Pname vec_del_fct;
Pstmt del_list;
Pstmt break_del_list;
Pstmt continue_del_list;
bit not_inl;	// is the current function an inline?
Pname curr_fct;	// current function
Pexpr init_list;
Pexpr one;

Pexpr cdvec(Pname f, Pexpr vec, Pclass cl, Pname cd, int tail)
/*
	generate a call to construct or destroy the elements of a vector
*/
{
	Pexpr sz = new texpr(SIZEOF,cl,0);	// sizeof elem
	sz->tp = uint_type;
					
	Pexpr esz = new texpr(SIZEOF,cl,0);	// noe = sizeof(vec)/sizeof(elem)
	esz->tp = int_type;

	Pexpr noe = new texpr(SIZEOF,vec->tp,0);
	noe->tp = int_type;
	noe = new expr(DIV,noe,esz);
	noe->tp = uint_type;

	Pexpr arg = (0<=tail) ? new expr(ELIST,zero,0) : 0;	// 0 or 1 for dtors

	arg = new expr(ELIST,cd,arg);		// constructor or destructor
	cd->lval(ADDROF);			// cd->take_addr();

	arg = new expr(ELIST,sz,arg);
	arg = new expr(ELIST,noe,arg);
	arg = new expr(ELIST,vec,arg);

	arg = new call(f,arg);
	arg->base = G_CALL;
	arg->fct_name = f;

	return arg;
}


Pstmt trim_tail(Pstmt tt)
/*
	strip off statements after RETURN etc.
	NOT general: used for stripping off spurious destructor calls
*/
{
	Pstmt tx;
	while (tt->s_list) {
//error('d',"ww %k",tt->base);
		switch (tt->base) {
		case PAIR:
			tx = trim_tail(tt->s2);
			goto txl;
		case BLOCK:
			tx = trim_tail(tt->s);
		txl:
//error('d',"txl%k ->%k",tt->base,tx->base);
			switch (tx->base) {
			case SM:
				break;
			case CONTINUE:
			case BREAK:
			case GOTO:
			case RETURN:
//error('d',"trim1 %k %k",tt->s_list->base,tx->s_list->base);
				tt->s_list = 0;
			default:
				return tx;
			}
		default:
			tt = tt->s_list;
			break;
		case RETURN:
//error('d',"trim2 %k",tt->s_list->base);
			tt->s_list = 0;
			return tt;
		}
	}
//error('d',"ss %k",tt->base);
	switch (tt->base) {
	case PAIR:	return trim_tail(tt->s2);
//	case LABEL:	return trim_tail(tt->s);
	case BLOCK:	if (tt->s) return trim_tail(tt->s);
	default:	return tt;
	}
}

void simpl_init()
{
	Pname nw = new name(oper_name(NEW));
	Pname dl = new name(oper_name(DELETE));
	Pname vn = new name("_vec_new");
	Pname vd = new name("_vec_delete");

	new_fct = gtbl->insert(nw,0);	/* void* operator new(long); */
	delete nw;
	Pname a = new name;
	a->tp = long_type;
	new_fct->tp = new fct(Pvoid_type,a,1);
	new_fct->n_scope = EXTERN;
	PERM(new_fct);
	PERM(new_fct->tp);
	new_fct->use();
//	new_fct->dcl_print(0);

	del_fct = gtbl->insert(dl,0);	/* void operator delete(void*); */
	delete dl;
	a = new name;
	a->tp = Pvoid_type;
	del_fct->tp = new fct(void_type,a,1);
	del_fct->n_scope = EXTERN;
	PERM(del_fct);
	PERM(del_fct->tp);
	del_fct->use();
//	del_fct->dcl_print(0);

	a = new name;
	a->tp = Pvoid_type;
	Pname al = a;
	a = new name;
	a->tp = int_type;
	a->n_list = al;
	al = a;
	a = new name;
	a->tp = int_type;
	a->n_list = al;
	al = a;
	a = new name;	
	a->tp = Pvoid_type;
	a->n_list = al;
	al = a;				/* (Pvoid, int, int, Pvoid) */

	vec_new_fct = gtbl->insert(vn,0);
	delete vn;
	vec_new_fct->tp = new fct(Pvoid_type,al,1);
	vec_new_fct->n_scope = EXTERN;
	PERM(vec_new_fct);
	PERM(vec_new_fct->tp);
	vec_new_fct->use();
//	vec_new_fct->dcl_print(0);

	a = new name;
	a->tp = int_type;
	al = a;
	a = new name;
	a->tp = Pvoid_type;
	a->n_list = al;
	al = a;
	a = new name;
	a->tp = int_type;
	a->n_list = al;
	al = a;
	a = new name;
	a->tp = int_type;
	a->n_list = al;
	al = a;
	a = new name;	
	a->tp = Pvoid_type;
	a->n_list = al;
	al = a;				/* (Pvoid, int, int, Pvoid, int) */

	vec_del_fct = gtbl->insert(vd,0);
	delete vd;
	vec_del_fct->tp = new fct(void_type,al,1);
	vec_del_fct->n_scope = EXTERN;
	PERM(vec_del_fct);
	PERM(vec_del_fct->tp);
	vec_del_fct->use();
//	vec_del_fct->dcl_print(0);

	one = new ival(1);
	one->tp = int_type;
	PERM(one);

	// assume void generates char
	putstring("char *_new(); char _delete(); char *_vec_new(); char _vec_delete();\n");

}

Ptable scope;		/* current scope for simpl() */
Pname expand_fn;	/* name of function being expanded or 0 */
Ptable expand_tbl;	/* scope for inline function variables */

Pname classdef::has_oper(TOK op)
{
	char* s = oper_name(op);
	Pname n;
	if (this == 0) error('i',"0->has_oper(%s)",s);
	n = memtbl->lookc(s,0);
	if (n == 0) return 0;
	switch (n->n_scope) {
	case 0:
	case PUBLIC:	return n;
	default:	return 0;
	}
}

int is_expr(Pstmt s)
/*
	is this statement simple enough to be converted into an expression for
	inline expansion?
*/
{
	int i = 0;
	for (Pstmt ss = (s->base == BLOCK) ? s->s : s; ss; ss = ss->s_list) {
//error('d',"ss %k",ss->base);
		switch (ss->base) {
		case BLOCK:
			if (Pblock(ss)->memtbl || is_expr(ss->s)==0) return 0;
		case SM:
			if (ss->e->base == ICALL) {
				Pname fn = ss->e->il->fct_name;
				Pfct  f  = (Pfct)fn->tp;
				if (f->f_expr == 0) return 0;
			}
			break;
		case IF:
			if (is_expr(ss->s)==0) return 0;
			if (ss->else_stmt && is_expr(ss->else_stmt)==0) return 0;
			break;
		default:
			return 0;
		}
		i++;
	}
	return i;
}

int no_of_returns;

void name::simpl()
{
//error('d',"name::simpl%n %d %k",this,tp->base,tp->base);
	if (base == PUBLIC) return;

	if (tp == 0) error('i',"%n->N::simple(tp==0)",this);

	switch (tp->base) {
	case 0:
		error('i',"%n->N::simpl(tp->B==0)",this);

	case OVERLOAD:
	{	for (Plist gl = Pgen(tp)->fct_list; gl; gl=gl->l) gl->f->simpl();
		break;
	}

	case FCT:
	{	Pfct f = (Pfct)tp;
		Pname n;
		Pname th = f->f_this;

		if (th) {
			// Make "this" a register if it is used more than twice:
			if (2 < th->n_used) 
				th->n_sto = REGISTER;
			else th->n_sto = 0;
			if (n_oper == CTOR) f->s_returns = th->tp;
		}

		if (tp->defined & (SIMPLIFIED | ~DEFINED) ) return;

		for (n=th?th:(f->f_result?f->f_result:f->argtype); n; n=n->n_list) n->simpl();

		if (f->body) {
			Ptable oscope = scope;
			scope = f->body->memtbl;
			if (scope == 0) error('i',"%n memtbl missing",this);
			curr_fct = this;
			f->simpl();
			if (f->f_inline && debug==0) {
				if (MIA<=f->nargs) {
					error('w',"too manyAs for inline%n (inline ignored)",this);
					f->f_inline = 0;
					scope = oscope;
					break;
				}
				int i = 0;
				for (n=th?th:(f->f_result?f->f_result:f->argtype); n; n=n->n_list) {
					n->base = ANAME;
					n->n_val = i++;
					if (n->n_table != scope) error('i',"aname scope: %s %d %d\n",n->string,n->n_table,scope);
				}
				expand_tbl = (f->returns->base!=VOID || n_oper==CTOR) ? scope : 0;
				expand_fn = this;
				if (expand_tbl) {
				genlab:
//error('d',"%t%n",f->returns,expand_fn);
						// value returning: generate expr
					Pexpr ee = (Pexpr)f->body->expand();
						// the body still holds the memtbl
					f->f_expr = (ee->base==CM) ? ee : new expr(CM,zero,ee);
					/* print.c assumes expansion into comma expression */
				}
				else {
					if (is_expr(f->body)) {
						// can generate expr: do
						f->s_returns = int_type;
						expand_tbl = scope;
						goto genlab;
					}
					// not value return: can generate block
//error('d',"void%n",expand_fn);
					f->f_expr = 0;
					f->body = (Pblock)f->body->expand();
				}
				expand_fn = 0;
				expand_tbl = 0;
			}
			scope = oscope;
		}
		break;
	}

	case CLASS:
		Pclass(tp)->simpl();
		break;
/*
	case EOBJ:
		tp->base = INT;
		break;
*/
	default:
//error('d',"%n tp %t n_init %d",this,tp, n_initializer);
		break;
	}

	if (n_initializer) n_initializer->simpl();
	tp->defined |= SIMPLIFIED;
}

void fct::simpl()
/*
	call only for the function definition (body != 0)

	simplify argument initializers, and base class initializer, if any
	then simplify the body, if any

	for constructor:call allocator if this==0 and this not assigned to
			(auto and static objects call constructor with this!=0,
			the new operator generates calls with this==0)
			call base & member constructors
	for destructor:	call deallocator (no effect if this==0)
			case base & member destructors

	for arguments and function return values look for class objects
	that must be passed by constructor X(X&).

	Allocate temporaries for class object expressions, and see if
	class object return values can be passed as pointers.

	call constructor and destructor for local class variables.
*/
{
	Pexpr th = f_this;
	Ptable tbl = body->memtbl;
	Pstmt ss = 0;
	Pstmt tail;
	Pname cln; 
	Pclass cl;
	Pstmt dtail = 0;

	not_inl = debug || f_inline==0;
	del_list = 0;
	continue_del_list = 0;
	break_del_list = 0;
	scope = tbl;
	if (scope == 0) error('i',"F::simpl()");

	if (th) {
		Pptr p = (Pptr)th->tp;
		cln = Pbase(p->typ)->b_name;
		cl = (Pclass)cln->tp;
	}

	if (curr_fct->n_oper == DTOR) {		/* initialize del_list */
		Pexpr ee;
		Pstmt es;
		Pname bcln = cl->clbase;
		Pclass bcl;
		Pname d;

		Pname fa = new name("_free");	/* fake argument for dtor */
		fa->tp = int_type;
		Pname free_arg = fa->dcl(body->memtbl,ARG);
		delete fa;
		f_this->n_list = free_arg;

		Ptable tbl = cl->memtbl;
		int i;
		Pname m;

		/* generate calls to destructors for all members of class cl */
		for (m=tbl->get_mem(i=1); m; m=tbl->get_mem(++i) ) {
			Ptype t = m->tp;
			Pname cn;
			Pclass cl;
			Pname dtor;
			if (m->n_stclass == STATIC) continue;

			if (cn = t->is_cl_obj()) {
				cl = (Pclass)cn->tp;
				if (dtor = cl->has_dtor()) {
					/*	dtor(this,0);	*/
					Pexpr aa = new expr(ELIST,zero,0);
					ee = new ref(REF,th,m);
					ee = new ref(DOT,ee,dtor);
					ee = new call(ee,aa);
					ee->fct_name = dtor;
					ee->base = G_CALL;
					es = new estmt(SM,curloc,ee,0);
					if (dtail)
						dtail->s_list = es;
					else
						del_list = es;
					dtail = es;	
				}
			}
			else if (cl_obj_vec) {
				cl = Pclass(cl_obj_vec->tp);
				if (dtor = cl->has_dtor()) {
					Pexpr mm = new ref(REF,th,m);
					mm->tp = m->tp;
					Pexpr ee = cdvec(vec_del_fct,mm,cl,dtor,0);
					es = new estmt(SM,curloc,ee,0);
					if (dtail)
						dtail->s_list = es;
					else
						del_list = es;
					dtail = es;
				}
			}
		}

		// delete base
		if (bcln
		&& (bcl=(Pclass)bcln->tp)
		&& (d=bcl->has_dtor()) ) {	// base.dtor(this,_free);
			Pexpr aa = new expr(ELIST,free_arg,0);
			ee = new ref(REF,th,d);
			ee = new call(ee,aa);
			/*ee->fct_name = d; NO would suppress virtual */
			ee->base = G_CALL;
			es = new estmt(SM,curloc,ee,0);
		}
		else {				// if (_free) _delete(this);
			Pexpr aa  = new expr(ELIST,th,0);
			ee = new call(del_fct,aa);
			ee->fct_name = del_fct;
			ee->base = G_CALL;
			es = new estmt(SM,curloc,ee,0);
			es = new ifstmt(curloc,free_arg,es,0);
		}
		free_arg->use();
		Pname(th)->use();
		if (dtail)
			dtail->s_list = es;
		else
			del_list = es;
		del_list = new ifstmt(curloc,th,del_list,0);
		if (del_list) del_list->simpl();
	}

	int ass_count;

	if (curr_fct->n_oper == CTOR) {
		Pexpr ee;
		Ptable tbl = cl->memtbl;
		Pname m;
		int i;

		/*
			generate: this=base::base(args)
			this->b_init == base::base(args) or 0
		*/
		if (b_init) {
//error('d',"b_init %k",b_init->base);
			switch (b_init->base) {
			case ASSIGN:
			case CM:
				break;
			default:
			{	Pcall cc = (Pcall)b_init;
				Pname bn = cc->fct_name;
				Pname tt = Pfct(bn->tp)->f_this;
				ass_count = tt->n_assigned_to;
				cc->simpl();
				init_list = new expr(ASSIGN,th,cc);
			}
			}
		}
		else {
			ass_count = 0;
			init_list = 0;
		}

		if (cl->virt_count) {	/* generate: this->_vptr=virt_init; */
			Pname vp = cl->memtbl->look("_vptr",0);
			Pexpr vtbl = new text_expr(cl->string,"_vtbl");
			ee = new ref(REF,th,vp);
			ee = new expr(ASSIGN,ee,vtbl);
			init_list =  (init_list) ? new expr(CM,init_list,ee) : ee;
		}
for (Pname nn = f_init; nn; nn=nn->n_list) {
	if (nn->n_initializer == 0) continue;
	Pname m = tbl->look(nn->string, 0);
	if (m && m->n_table == tbl) m->n_initializer = nn->n_initializer;
}
		/* generate cl::cl(args) for all members of cl */
		for (m=tbl->get_mem(i=1); m; m=tbl->get_mem(++i) ) {
			Ptype t = m->tp;
			Pname cn;
			Pclass cl;
			Pname ctor;

			switch (m->n_stclass) {
			case STATIC:
			case ENUM:
				continue;
			}
			switch (t->base) {
			case FCT:
			case OVERLOAD:
			case CLASS:
			case ENUM:
				continue;
			}
			if (m->base == PUBLIC) continue;

			if (cn=t->is_cl_obj()) {
		 		Pexpr ee = m->n_initializer;
				m->n_initializer = 0;	// from fct must not persist until next fct

				if (ee == 0) {		// try default
					cl = (Pclass)cn->tp;
					if (ctor = cl->has_ictor()) {
						ee = new ref(REF,th,m);
						ee = new ref(DOT,ee,ctor);
						ee = new call(ee,0);
						ee->fct_name = ctor;
						ee->base = G_CALL;
						ee = ee->typ(tbl);	// look for default arguments
					}
					else if (cl->has_ctor()) {
					   error("M%n needsIr (no defaultK forC %s)",m,cl->string);
					}
				}

				if (ee) {
					ee->simpl();
					if (init_list)
						init_list = new expr(CM,init_list,ee);
					else
						init_list = ee;
				} 
			}
			else if (cl_obj_vec) {
				cl = (Pclass)cl_obj_vec->tp;
				if (ctor = cl->has_ictor()) {
					// _new_vec(vec,noe,sz,ctor);
					Pexpr mm = new ref(REF,th,m);
					mm->tp = m->tp;
					Pexpr ee = cdvec(vec_new_fct,mm,cl,ctor,-1);
					ee->simpl();
					if (init_list)
						init_list = new expr(CM,init_list,ee);
					else
						init_list = ee;
				}
				else if (cl->has_ctor()) {
					error("M%n[] needsIr (no defaultK forC %s)",m,cl->string);
				}
			}
			else if (m->n_initializer) {
				// init of non-class mem
				// set in fct.mem_init()
				if (init_list)
					init_list = new expr(CM,init_list,m->n_initializer);
				else
					init_list = m->n_initializer;
				m->n_initializer = 0;	// from fct must not persist until next fct
			}
			else if (t->is_ref()) {
				error("RM%n needsIr",m);
			}
			else if (t->tconst() && vec_const==0) {
				error("constM%n needsIr",m);
			}
		} // for m
	}

	no_of_returns = 0;
	tail = body->simpl();
//error('d',"tail %d %k del_list %d (rb %d %k res %d) no_of_returns %d",tail,tail->base,del_list,returns->base,returns->base,f_result,no_of_returns);
	if (returns->base!=VOID || f_result) {	// return must have been seen
		if (no_of_returns) {		// could be OK
			Pstmt tt = (tail->base==RETURN || tail->base==LABEL) ? tail : trim_tail(tail);
//error('d',"tb %k tt %k",tail->base,tt->base);
			switch (tt->base) {
			case RETURN:
			case GOTO:
				del_list = 0;	// no need for del_list
				break;
			case SM:
				switch (tt->e->base) {
				case ICALL:
				case G_CALL:
					goto chicken;
				};
			default:
				if (strcmp(curr_fct->string,"main"))	
					error('w',"maybe no value returned from%n",curr_fct);
			case IF:
			case SWITCH:
			case DO:
			case WHILE:
			case FOR:
			case LABEL:
			chicken:		// don't dare write a warning
				break;
			}
		}
		else {				// must be an error
			if (strcmp(curr_fct->string,"main"))	
				error('w',"no value returned from%n",curr_fct);
		}
		if (del_list) goto zaq;
	}
	else if (del_list) {	// return may not have been seen
	zaq:
		if (tail)
			tail->s_list = del_list;
		else
			body->s = del_list;
		tail = dtail;
	}

	if (curr_fct->n_oper == DTOR) {
		// body => if (this == 0) body
		body->s = new ifstmt(body->where,th,body->s,0);
	}

	if (curr_fct->n_oper == CTOR) {

		if  ( Pname(th)->n_assigned_to == 0 ) {
		/* generate:	if (this==0) this=_new( sizeof(class cl) );
				init_list ;
		*/
			Pname(th)->n_assigned_to = ass_count ? ass_count : FUDGE111;
			Pexpr sz = new texpr(SIZEOF,cl,0);
			sz->tp = uint_type;
			Pexpr ee = new expr(ELIST,sz,0);
			ee = new call(new_fct,ee);
			ee->fct_name = new_fct;
			ee->base = G_CALL;
			ee->simpl();
			ee = new expr(ASSIGN,th,ee);
			Pstmt es = new estmt(SM,body->where,ee,0);
			ee = new expr(EQ,th,zero);
			ifstmt* ifs = new ifstmt(body->where,ee,es,0);
			/*ifs->simpl();
				do not simplify
				or "this = " will cause an extra call of base::base
			*/
			if (init_list) {
				es = new estmt(SM,body->where,init_list,0);
				es->s_list = body->s;
				body->s = es;
				if (tail == 0) tail = es;
			}
			ifs->s_list = body->s;
			body->s = ifs;
			if (tail == 0) tail = ifs;
		}

		Pstmt st = new estmt(RETURN,curloc,th,0);
		if (tail)
			tail->s_list = st;
		else
			body->s = st;
		tail = st;
	}
}

Pstmt block::simpl()
{
	int i;
	Pname n;
	Pstmt ss=0, sst=0;
	Pstmt dd=0, ddt=0;
	Pstmt stail;
	Ptable old_scope = scope;

	if (own_tbl == 0) {
		ss = (s) ? s->simpl() : 0;
		return ss;
	}

	scope = memtbl;
	if(scope->init_stat == 0) scope->init_stat = 1; /* table is simplified. */

	for (n=scope->get_mem(i=1); n; n=scope->get_mem(++i)) {
		Pstmt st = 0;
		Pname cln;
		Pexpr in = n->n_initializer;
//error('d',"auto %n",n);
		if (in) scope->init_stat = 2; /* initializer in this scope */

		switch (n->n_scope) {
		case ARG:
		case 0:
		case PUBLIC:
			 continue;
		}

		if (n->n_stclass == STATIC) continue;

		if (in && in->base==ILIST)
			error('s',"initialization of automatic aggregates");

		if (n->tp == 0) continue; /* label */

		if (n->n_evaluated) continue;

		/* construction and destruction of temporaries is handled locally */
		{	char* s = n->string;
			register char c3 = s[3];
			if (s[0]=='_' && s[1]=='D' && isdigit(c3)) continue;
		}
		if ( cln=n->tp->is_cl_obj() ) {
			Pclass cl = (Pclass)cln->tp;
			Pname d = cl->has_dtor();

			if (d) {			// n->cl.dtor(0);
 				Pref r = new ref(DOT,n,d);
 				Pexpr ee = new expr(ELIST,zero,0);
 				Pcall dl = new call(r,ee);
 				Pstmt dls = new estmt(SM,n->where,dl,0);
 				dl->base = G_CALL;
 				dl->fct_name = d;
			
			/*
				if (dd)
					ddt->s_list = dls;
				else
					dd = dls;
				ddt = dls;
			*/
				if (dd) {
					dls->s_list = dd;
					dd = dls;
				}
				else
					ddt = dd = dls;
			}

			if (in) {
				switch (in->base) {
				case DEREF:		// *constructor?
					if (in->e1->base == G_CALL) {
						Pname fn = in->e1->fct_name;
						if (fn==0 || fn->n_oper!=CTOR) goto ddd;
						st = new estmt(SM,n->where,in->e1,0);
						n->n_initializer = 0;
						break;
					}
					goto ddd;
				case G_CM:
					st = new estmt(SM,n->where,in->e1,0);
					n->n_initializer = 0;
					break;
				case ASSIGN:		// assignment to "n"?
					if (in->e1 == n) {
						st = new estmt(SM,n->where,in,0);
						n->n_initializer = 0;
						break;
					}
				default:
					goto ddd;
				}
			}
		}
		else if (cl_obj_vec) {
			Pclass cl = Pclass(cl_obj_vec->tp);
			Pname d = cl->has_dtor();
			Pname c = cl->has_ictor();

			n->n_initializer = 0;

			if (c) {	//  _vec_new(vec,noe,sz,ctor);
				Pexpr a = cdvec(vec_new_fct,n,cl,c,-1);
				st = new estmt(SM,n->where,a,0);
			}

			if (d) {	//  _vec_delete(vec,noe,sz,dtor,0);
				Pexpr a = cdvec(vec_del_fct,n,cl,d,0);
				Pstmt dls = new estmt(SM,n->where,a,0);
			/*	if (dd)
					ddt->s_list = dls;
				else
					dd = dls;
				ddt = dls;
			*/
				if (dd) {
					dls->s_list = dd;
					dd = dls;
				}
				else
					ddt = dd = dls;
			}

		}
		else if (in /*&& n->n_scope==FCT*/) {
			switch (in->base) {
			case ILIST:
				switch (n->n_scope) {
				case FCT:
				case ARG:
					error('s',"Ir list for localV%n",n);
				}
				break;
			case STRING:
				if (n->tp->base==VEC) break; /* BUG char vec only */
			default:
			ddd:
			{	Pexpr ee = new expr(ASSIGN,n,in);
				st = new estmt(SM,n->where,ee,0);
				n->n_initializer = 0;
			}
			}
		}

		if (st) {
			if (ss)
				sst->s_list = st;
			else
				ss = st;
			sst = st;
		}
	}

	if (dd) {
		Pstmt od = del_list;
		Pstmt obd = break_del_list;
		Pstmt ocd = continue_del_list;

		dd->simpl();
		del_list = (od) ? Pstmt(new pair(curloc,dd,od)) : dd;
		break_del_list = (break_del_list&&obd) ? Pstmt(new pair(curloc,dd,obd)) : dd;
		continue_del_list = (continue_del_list&&ocd) ? Pstmt(new pair(curloc,dd,ocd)) : dd;

		stail  = (s) ? s->simpl() : 0;

		Pfct f = (Pfct)curr_fct->tp;
		if (this!=f->body
		|| f->returns->base==VOID
		|| (f->returns->base != VOID && no_of_returns == 0 )
		|| strcmp(curr_fct->string,"main")==0 ) {
		// not dropping through the bottom of a value returning function
			if (stail) {
				Pstmt tt = (stail->base==RETURN || stail->base==LABEL) ? stail : trim_tail(stail);
				if (tt->base != RETURN) stail->s_list = dd;
			}
			else
				s = dd;
			stail = ddt;
		}

		del_list = od;
		continue_del_list = ocd;
		break_del_list = obd;
	}
	else
		stail  = (s) ? s->simpl() : 0;

	if (ss) {	/* place constructor calls */
		ss->simpl();
		sst->s_list = s;
		s = ss;
		if (stail == 0) stail = sst;
	}
	
	scope = old_scope;

	return stail;
}


void classdef::simpl()
{
	int i;
	Pname m;
	Pclass oc = in_class;

	in_class = this;
	for (m=memtbl->get_mem(i=1); m; m=memtbl->get_mem(++i) ) {
		Pexpr i = m->n_initializer;
		m->n_initializer = 0;
		m->simpl();
		m->n_initializer = i;
	}
	in_class = oc;

	for (Plist fl=friend_list; fl; fl=fl->l) {	/* simplify friends */
		Pname p = fl->f;
		switch (p->tp->base) {
		case FCT:
		case OVERLOAD:
			p->simpl();
		}
	}
}

Ptype nstd_type;
void expr::simpl()
{
//error('d',"expr.simpl (%d) %d%k e1=%d e2=%d tp2=%d cf%n",permanent,this,base,e1,e2,tp2,curr_fct);
	if (this==0 || permanent==2) return;

	switch (base) {
/*
	case BLOCK:
	case SM:
	case IF:
	case FOR:
	case WHILE:
	case SWITCH:
		error('i',"%k inE",base);

	case VALUE:
		error('i',"E::simpl(value)");
*/
	case ICALL:   
        return;
	case G_ADDROF:
	case ADDROF:
		e2->simpl();
		switch (e2->base) {
		case DOT:
		case REF:
		{	Pref r = (Pref)e2;
			Pname m = r->mem;
			if (m->n_stclass == STATIC) {	// & static member
				Pexpr x;
			delp:
				x = e2;
				e2 = m;
				r->mem = 0;
				DEL(x);
			}
			else if (m->tp->base == FCT) {	// & member fct
				Pfct f = (Pfct)m->tp;
				if (f->f_virtual) {	// &p->f ==> p->vtbl[fi]
					int index = f->f_virtual;
					Pexpr ie = (1<index) ? new ival(index-1):0;
					if (ie) ie->tp = int_type;
					Pname vp = m->n_table->look("_vptr",0);
					r->mem = vp;
					base = DEREF;
					e1 = e2;
					e2 = ie;
				}
				else {
					goto delp;
				}
			}
		}
		}
		break;

	default:
		if (e1) e1->simpl();
		if (e2) e2->simpl();
		break;

	case NAME:
	case DUMMY:
	case ICON:
	case FCON:
	case CCON:
	case IVAL:
	case FVAL:
	case LVAL:
	case STRING:
	case ZERO:
	case ILIST:
	case SIZEOF:
		return;
/*
	case SIZEOF:
		base = IVAL;
		i1 = tp2->tsizeof();
		tp2 = 0;	// can't DEL(tp2)
		break;
*/
	case G_CALL:
	case CALL:
		Pcall(this)->simpl();
		break;

	case NEW:
		simpl_new();
		return;

	case DELETE:
		simpl_delete();
		break;

	case QUEST:
		cond->simpl();
		e2->simpl();
		// no break
	case CAST:
	case REF:
		e1->simpl();
		break;

	case DOT:
		e1->simpl();
		switch (e1->base) {
		case CM:
		case G_CM:
		 {	// &( , name). => ( ... , &name)->
			Pexpr ex = e1;
			cfr:
			switch (ex->e2->base) {
			case NAME:
				base = REF;
				ex->e2 = ex->e2->address();
				break;
			case CM:
			case G_CM:
				ex = ex->e2;
				goto cfr;
			}
		}
		}
		break;

	case ASSIGN:
	{	Pfct f = (Pfct)curr_fct->tp;
		Pexpr th = f->f_this;

		if (e1) e1->simpl();
		if (e2 && e2->base == ASSIGN) nstd_type = e2->tp; 
		if (e2) e2->simpl();

		if (th && th==e1 && curr_fct->n_oper==CTOR && init_list) {
			// this=e2 => (this=e2,init_list)
			base = CM;
			e1 = new expr(ASSIGN,e1,e2);
			e2 = init_list;
//fprintf( stderr, "\nnstd_type: %d, th->tp: %d", nstd_type, th->tp );
			if ( nstd_type && nstd_type == th->tp ) {
				Pexpr ee = new expr(CM, e1, e2);
				e1 = ee; e2 = th;
			}
		}
		if ( nstd_type ) nstd_type=0;
		break;
	}
	}
	
	if (tp == int_type) {
		Neval = 0;
		int i = eval();
		if (Neval == 0) {
			base = IVAL;
			i1 = i;
		}
	}
}



void call::simpl()
/*
	fix member function calls:
		p->f(x) becomes f(p,x)
		o.f(x)  becomes f(&o,x)
	or if f is virtual:
		p->f(x) becomes ( *p->_vptr[ type_of(p).index(f)-1 ] )(p,x)
	replace calls to inline functions by the expanded code
*/
{
	Pname fn = fct_name;
	Pfct f = (fn) ? (Pfct)fn->tp : 0;

	if (fn == 0) e1->simpl();

	if (f) {
		switch(f->base) {
		case ANY:
			return;
		case FCT:
			break;
		case OVERLOAD:
		{	Pgen g = (Pgen)f;
			fct_name = fn = g->fct_list->f;
			f = (Pfct)fn->tp;
		}
		}
	}
	
	switch (e1->base) {
	case MEMPTR:	// (p ->* q)(args)
	{	Pexpr p = e1->e1;
		Pexpr q = e1->e2;
		Pclass cl = Pclass(e1->tp2);
                Pfct f = Pfct(q->tp->deref());

		if (cl->virt_count == 0) {	// no virtuals: simple
			// (p ->* q)(args) => (*q)(p,args)
			e2 = new expr(ELIST,p,e2);
			e1 = new expr(DEREF,q,0);
			e1->tp = q->tp->deref();
		}
		else {					// may be virtual: yuk
			if (f->f_this == 0) {
				if (f->memof == 0) error('i',"memof missing");
				Pname tt = new name("this");
				tt->n_scope = ARG;
				tt->n_sto = ARG;
				tt->tp = f->memof->this_type;
				PERM(tt);
				f->f_this = tt;
				tt->n_list = f->f_result ? f->f_result : f->argtype;
			}
			// (p ->* q)(args) =>
			// ((int)q&~127) ? (*q)(p,args) : (p->vtbl[(int)*q-1])(p,args)
			// (((int)q&~127) ? *q : p->vtbl[(int)*q-1]) (p,args)

			// first beware of side effects:
			if (q->not_simple()) error('s',"second operand of .* too complicated");
			if (p->not_simple()) error('s',"first operand of .* too complicated");
			// (int)q&~127 :
			Pexpr c = new ival(127);
			Pexpr pp = new texpr(CAST,SZ_INT==SZ_WPTR?int_type:long_type,q);
			c = new expr(COMPL,0,c);
			c = new expr(AND,pp,c);

			// *q :
			//Pexpr nc = new expr(DEREF,q,0);
			//nc->tp2 = Ptype(f->f_this);	// encode argtype

			// ((&a)->vptr)[(int)q-1)] :
			Pexpr ie = new texpr(CAST,int_type,q);
			ie = new expr(MINUS,ie,one);
			Pname vp = cl->memtbl->look("_vptr",0);
			Pexpr vc = new expr(DEREF,new ref(REF,p,vp),ie);

			if ( f->returns->base == VOID )
				vc = new texpr( CAST, Pfctchar_type, vc );

			base = G_CALL;
			//e1 = new expr(QUEST,nc,vc);
			e1 = new expr(QUEST,q,vc);
			e1->cond = c;
			e1 = new expr(DEREF, e1, 0 );
			e1->tp2 = Ptype(f->f_this); // encode argtype
			e2 = new expr(ELIST,p,e2);
		}
		break;
	}
	case DOT:
	case REF:
	{	Pref r = (Pref)e1;
		Pexpr a1 = r->e1;
//error('d',"simpl fn %s f %d fv %d",fn?fn->string:"?",f,f?f->f_virtual:0);
		if (f && f->f_virtual) {
			Pexpr a11 = 0;

			switch(a1->base) {	// see if temporary might be needed
			case NAME:
				a11 = a1;
				break;
//next two cases are new
			case REF:
			case DOT:
				if (a1->e1->base == NAME) a11 = a1;
				break;
			case ADDROF:
			case G_ADDROF:
				if (a1->e2->base == NAME) a11 = a1;
				break;
			}

			if (e1->base == DOT) {
				if (a11) a11 = a11->address();
				a1 = a1->address();
			}
			
			if (a11 == 0) {	// temporary (maybe) needed
				   	// e->f() => (t=e,t->f(t))
				Pname nx = new name(make_name('K'));
				nx->tp = a1->tp;
				Pname n = nx->dcl(scope,ARG); /* no init! */
				delete nx;
				Pname cln = a1->tp->is_cl_obj();
				if (cln && Pclass(cln->tp)->has_itor()) n->n_xref = 1;
				n->n_scope = FCT;
				n->assign();
				a11 = n;
				a1 = new expr(ASSIGN,n,a1);
				a1->tp = n->tp;
				a1->simpl();
				Pcall cc = new call(0,0);
				*cc = *this;
				base = CM;
				e1 = a1;
				e2 = cc;
				this = cc;
			}
			e2 = new expr(ELIST,a11,e2);
			int index = f->f_virtual;
			Pexpr ie = (1<index)?new ival(index-1):0;
			Pname vp = fn->n_table->look("_vptr",0);
			Pexpr vptr = new ref(REF,a11,vp);	/* p->vptr */
			Pexpr ee = new expr(DEREF,vptr,ie);	/* p->vptr[i] */
			Ptype pft = new ptr(PTR,f);
			ee = new texpr(CAST,pft,ee);		/* (T)p->vptr[i] */
			ee->tp = (Ptype)f->f_this;		/* encode argtype */
			e1 = new expr(DEREF,ee,0);		/* *(T)p->vptr[i] */
								/* e1->tp must be 0, means "argtype encoded" */
			fct_name = 0;
			fn = 0;
			e2->simpl();
			return;				/* (*(T)p->vptr[i])(e2) */
		}
		else {
//error('d',"a1 %k%k%n %t",a1->base,e1->base,r->mem,r->mem->tp);
			Ptype tt = r->mem->tp;
		llp:
			switch (tt->base) {
			// default:	// pointer to function: (n->ptr_mem)(args); do nothing
			case TYPE:
				tt = Pbase(tt)->b_name->tp; goto llp;
			case FCT:
			case OVERLOAD:	// n->fctmem(args);
			if ( a1->base != QUEST ) {
				if (e1->base == DOT) a1 = a1->address();
				e2 = new expr(ELIST,a1,e2);
				e1 = r->mem;
			}
			else {
				Pexpr t0 = new expr( base, 0, 0 );
				Pexpr t1 = new expr( e1->base, 0, 0 );
				*t0 = *(Pexpr)this;
				*t1 = *e1; 
				t0->e1 = t1;
				t1->e1 = a1->e1;
				a1->e1 = t0;

				Pexpr tt0 = new expr( base, 0, 0 );
				Pexpr tt1 = new expr( e1->base, 0, 0 );
				*tt0 = *(Pexpr)this;
				*tt1 = *e1; 
				tt0->e1 = tt1;
				tt1->e1 = a1->e2;
				a1->e2 = tt0;

				*(Pexpr)this = *a1;
				Pexpr(this)->simpl();
				return;
				}
			}
		}
	}
	}

	e2->simpl();

	if (e1->base==NAME && e1->tp->base==FCT) {
		/* reconstitute fn destroyed to suppress "virtual" */
		fct_name = fn = (Pname)e1;
		f = (Pfct)fn->tp;
	}

	Pexpr ee;
	if (fn && f->f_inline && debug==0) {
		ee = f->expand(fn,scope,e2);
		if (ee) 
			*Pexpr(this) = *ee;
		else 

		if ( this->tp->base == TYPE 
			&& Pbase(this->tp)->b_name 
			&& Pbase(this->tp)->b_name->tp->base == COBJ) 
		{
          		Pexpr ee1 = new expr( this->base, 0, 0 );
	  		*ee1 = *Pexpr(this);
	  		Pname tmp = make_tmp( 'T', this->tp, scope );
	  		ee = new expr( ASSIGN, tmp, ee1 );
	  		Pexpr ee2 = new expr( G_ADDROF,0,tmp );
	  		ee = new expr( CM, ee, ee2 );
	  		*Pexpr(this ) = *ee;
	 	 }
	}
}

void ccheck(Pexpr e)
/*
	 Is there a conditional in this expression? (not perfect)
*/
{
	if ( e )
	switch (e->base) {
	case QUEST:
	case ANDAND:
	case OROR:
		error('s',"E too complicated: uses%k and needs temorary ofCW destructor",e->base);
		break;
	case LT:
	case LE:
	case GT:
	case GE:
	case EQ:
	case NE:
	case ASSIGN:
	case ASPLUS:
	case ASMINUS:
	case G_CM:
	case CM:
	case PLUS:
	case MINUS:
	case MUL:
	case DIV:
	case OR:
	case ER:
	case AND:
	case G_CALL:
	case CALL:
	case ELIST:
		ccheck(e->e1);
	case NOT:
	case COMPL:
	case CAST:
	case ADDROF:
	case G_ADDROF:
		ccheck(e->e2);
	}
}

void temp_in_cond(Pexpr ee, Pstmt ss, Ptable tbl)
/*
	insert destructor calls 'ss' into condition 'ee'
*/
{
	ccheck(ee);
	while (ee->base==CM || ee->base==G_CM) ee = ee->e2;
	Ptype ct = ee->tp;
/*
	Cstmt = 0;
	Pname tmp = make_tmp('Q',ct,tbl);
*/
	Pname n = new name(make_name('Q'));
	n->tp = ct;
	Pname tmp = n->dcl(tbl,ARG);
	delete n;
	tmp->n_scope = FCT;
	Pexpr v = new expr(0,0,0);
	*v = *ee;
	v = new texpr(CAST,ct,v);
	v->tp = ct;
	Pexpr c = new expr(ASSIGN,tmp,v);
	c->tp = ct;
	ee->base = CM;
	ee->e1 = c;
	Pexpr ex = 0;
	for (Pstmt sx = ss; sx; sx = sx->s_list) {
		ex = (ex) ? new expr(CM,ex,sx->e) : sx->e;
		ex->tp = sx->e->tp;
	}
	ee->e2 = new expr(CM,ss->e,tmp);
	ee->e2->tp = ct;
}

bit not_safe(Pexpr e)
{

	switch (e->base) {
	default:
		return 1;
/*
	case CALL:
	case G_CALL:
	case DOT:
	case REF:
	case ANAME:
		return 1;
*/
	case NAME:
		// if the name is automatic and has a destructor it is not safe
		// to destroy it before returning an expression depending on it
	{	Pname n = Pname(e);
		if (n->n_table!=gtbl && n->n_table->t_name==0) {
			Pname cn = n->tp->is_cl_obj();
			if (cn && Pclass(cn->tp)->has_dtor()) return 1;
		}
	}
	case IVAL:
	case ICON:
	case CCON:
	case FCON:
	case STRING:
		return 0;
	case NOT:
	case COMPL:
	case ADDROF:
	case G_ADDROF:
		return not_safe(e->e2);
	case DEREF:
		//return not_safe(e->e1) || e->e2?not_safe(e->e2):0;
		{
		int i = not_safe(e->e1);
		if (i) return i;
		if (e->e2) return not_safe(e->e2);
		return 0;
		}
	case CM:
	case PLUS:
	case MINUS:
	case MUL:
	case DIV:
	case MOD:
	case ASSIGN:
	case ASPLUS:
	case ASMINUS:
	case ASMUL:
	case ASDIV:
	case OR:
	case AND:
	case OROR:
	case ANDAND:
	case LT:
	case LE:
	case GT:
	case GE:
	case EQ:
	case NE:
		return not_safe(e->e1) || not_safe(e->e2);
	case QUEST:
		return not_safe(e->cond) || not_safe(e->e1) || not_safe(e->e2);
	}
}
		
	
Pexpr curr_expr;	/* to protect against an inline being expanded twice
			   in a simple expression keep track of expressions
			   being simplified
			*/
Pstmt stmt::simpl()
/*
	return a pointer to the last statement in the list, or 0
*/
{
	if (this == 0) error('i',"0->S::simpl()");
/*error('d',"stmt.simpl %d%k e %d%k s %d%k sl %d%k\n",this,base,e,e?e->base:0,s,s?s->base:0,s_list,s_list?s_list->base:0); fflush(stderr);*/

	curr_expr = e;

	switch (base) {
	default:
		error('i',"S::simpl(%k)",base);

	case ASM:
		break;

	case BREAK:
		if (break_del_list) {	// break => { _dtor()s; break; }
			Pstmt bs = new stmt(base,where,0);
			Pstmt dl = break_del_list->copy();
			base = BLOCK;
			s = new pair(where,dl,bs);
		}
		break;

	case CONTINUE:
		if (continue_del_list) { // continue => { _dtor()s; continue; }
			Pstmt bs = new stmt(base,where,0);
			Pstmt dl = continue_del_list->copy();
			base = BLOCK;
			s = new pair(where,dl,bs);
		}
		break;

	case DEFAULT:
		s->simpl();
		break;

	case SM:
		if (e) {
			if (e->base == DEREF) e = e->e1;
			e->simpl();
			if (e->base == DEREF) e = e->e1;
		}
		break;

	case RETURN:
	{	/*	return x;
			=>
				{ dtor()s; return x; }
			OR (returning an X where X(X&) is defined) =>
				{ ctor(_result,x); _dtor()s; return; }
			OR (where x needs temporaries)
			OR (where x might involve an object to be destroyed) =>
				{ _result = x; _dtor()s;  return _result; }
			return;		=>
				{ _dtor()s; return; }
			OR (in constructors) =>
				{ _dtor()s; return _this; }
		*/

		no_of_returns++;
		Pstmt dl = (del_list) ? del_list->copy() : 0;
		Pfct f = Pfct(curr_fct->tp);
//error('d',"curr_fct%n %d %k e==%d del %d",curr_fct,curr_fct->tp->base,curr_fct->tp->base,e,dl);
		if (e == 0) e = dummy;
		if (e==dummy && curr_fct->n_oper==CTOR) e = f->f_this;

		if (f->f_result) {	// ctor(_result,x); dtors; return;
//error('d',"simpl res e %d memtbl %d",e->base,memtbl);
			if (e->base == G_CM) e = replace_temp(e,f->f_result);
			e->simpl();
			Pstmt cs = new estmt(SM,where,e,0);
			if (dl)	cs = new pair(where,cs,dl);
			base = PAIR;
			s = cs;
			s2 = new estmt(RETURN,where,0,0);
#ifdef RETBUG
			s2->empty = 1; // fudge to bypass C bug (see print.c)
			s2->ret_tp = ret_tp;
#endif
		}
		else {			// dtors; return e;
//error('d',"simpl e %d memtbl %d",e->base,memtbl);
			e->simpl();
			if (dl) {
				if (e!=dummy && not_safe(e)) {
				// { _result = x; _dtor()s;  return _result; }
					Ptable ftbl = Pfct(curr_fct->tp)->body->memtbl;
					
					Pname r = ftbl->look("_result",0);
					if (r == 0) {
						r = new name("_result");
						r->tp = ret_tp;
						Pname rn = r->dcl(ftbl,ARG);
						rn->n_scope = FCT;
						rn->assign();
						delete r;
						r = rn;
					}
					Pexpr as = new expr(ASSIGN,r,e);
					as->tp = ret_tp;	// wrong if = overloaded, but then X(X&) ought to have been used 
					Pstmt cs = new estmt(SM,where,as,0);
					cs = new pair(where,cs,dl);
					base = PAIR;
					s = cs;
					s2 = new estmt(RETURN,where,r,0);
					s2->ret_tp = ret_tp;
				}
				else {
				// { _dtor()s;  return x; }
					base = PAIR;
					s = dl;
					s2 = new estmt(RETURN,where,e,0);
				}
			}
		}
		break;
	}

	case WHILE:
	case DO:
		e->simpl();
		{	Pstmt obl = break_del_list;
			Pstmt ocl = continue_del_list;
			break_del_list = 0;
			continue_del_list = 0;
			s->simpl();
			break_del_list = obl;
			continue_del_list = ocl;
		}
		break;

	case SWITCH:
		e->simpl();
		{	Pstmt obl = break_del_list;
			break_del_list = 0;
			s->simpl();
			break_del_list = obl;
		}
		switch (s->base) {
		case DEFAULT:
		case LABEL:
		case CASE:
			break;
		case BLOCK:
			if (s->s)
			switch (s->s->base) {
			case BREAK:	// to cope with #define Case break; case
			case CASE:
			case LABEL:
			case DEFAULT:
				break;
			default:
				goto df;
			}
			break;
		default:
		df:
			error('w',&s->where,"S not reached: case label missing");
		}
		break;

	case CASE:
		e->simpl();
		s->simpl();
		break;

	case LABEL:
		if (del_list) error('s',"label in blockW destructors");
		s->simpl();
		break;

	case GOTO:
		/*	If the goto is going to a different (effective) scope,
			then it is necessary to activate all relevant destructors
			on the way out of nested scopes, and issue errors if there
			are any constructors on the way into the target.

			Only bother if the goto and label have different effective
			scopes. (If mem table of goto == mem table of label, then
			they're in the same scope for all practical purposes.
		*/
		{
		Pname n = scope->look( d->string, LABEL );
		if (n == 0) error('i',&where,"label%n missing",d);
		if(n->n_realscope!=scope && n->n_assigned_to) {

			/*	Find the root of the smallest subtree containing
				the path of the goto.  This algorithm is quadratic
				only if the goto is to an inner or unrelated scope.
			*/

			Ptable r = 0;

			for(Ptable q=n->n_realscope; q!=gtbl; q=q->next) {
				for( Ptable p = scope; p != gtbl; p = p->next ) {
					if( p==q ) {
						r = p;	// found root of subtree!
						goto xyzzy;
					}
				}
			}

xyzzy:			if( r==0 ) error( 'i',&where,"finding root of subtree" );

			/* At this point, r = root of subtree, n->n_realscope
			 * = mem table of label, and scope = mem table of goto. */

			/* Climb the tree from the label mem table to the table
			 * preceding the root of the subtree, looking for
			 * initializers and ctors.  If the mem table "belongs"
			 * to an unsimplified block(s), the n_initializer field
			 * indicates presence of initializer, otherwise initializer
			 * information is recorded in the init_stat field of
			 * mem table. */

			for( Ptable p=n->n_realscope; p!=r; p=p->next )
				if( p->init_stat == 2 ) {
					error(&where,"goto%n pastDWIr",d);
					goto plugh; /* avoid multiple error msgs */
				}
				else if( p->init_stat == 0 ) {
					int i;
					for(Pname nn=p->get_mem(i=1);nn;nn=p->get_mem(++i))
						if(nn->n_initializer||nn->n_evaluated){
							error(&nn->where,"goto%n pastId%n",d,nn);
							goto plugh;
						}
				}
plugh:

			/* Proceed in a similar manner from the point of the goto,
			 * generating the code to activate dtors before the goto. */
			/* There is a bug in this code.  If there are class objects
			 * of the same name and type in (of course) different mem
			 * tables on the path to the root of the subtree from the
			 * goto, then the innermost object's dtor will be activated
			 * more than once. */

			{
			Pstmt dd = 0, ddt = 0;

			for( Ptable p=scope; p!=r; p=p->next ) {
				int i;
				for(Pname n=p->get_mem(i=1);n;n=p->get_mem(++i)) {
		Pname cln;
		if (n->tp == 0) continue; /* label */

		if ( cln=n->tp->is_cl_obj() ) {
			Pclass cl = (Pclass)cln->tp;
			Pname d = cl->has_dtor();

			if (d) {	/* n->cl::~cl(0); */
				Pref r = new ref(DOT,n,d);
				Pexpr ee = new expr(ELIST,zero,0);
				Pcall dl = new call(r,ee);
				Pstmt dls = new estmt(SM,n->where,dl,0);
				dl->base = G_CALL;
				dl->fct_name = d;
				if (dd)
					ddt->s_list = dls;
				else
					dd = dls;
				ddt = dls;
			}

		}
		else if (cl_obj_vec) {
			Pclass cl = (Pclass)cl_obj_vec->tp;
		//	Pname c = cl->has_ictor();
			Pname d = cl->has_dtor();

			if (d) {	//  _vec_delete(vec,noe,sz,dtor,0);
				Pexpr a = cdvec(vec_del_fct,n,cl,d,0);
				Pstmt dls = new estmt(SM,n->where,a,0);
				if (dd)
					ddt->s_list = dls;
				else
					dd = dls;
				ddt = dls;
			}
		}
				} /* end mem table scan */
			} /* end dtor loop */

			/* "activate" the list of dtors obtained. */

			if( dd ) {
				dd->simpl();
				Pstmt bs = new stmt( base, where, 0 );
				*bs = *this;
				base = PAIR;
				s = dd;
				s2 = bs;
			}
			}
		} /* end special case for non-local goto */
		}
		break;

	case IF:
		e->simpl();
		s->simpl();
		if (else_stmt) else_stmt->simpl();
		break;

	case FOR:	// "for (s;e;e2) s2; => "s; for(;e,e2) s2"
		if (for_init) {
			for_init->simpl();
/*
			if (for_init->base==SM && for_init->e->tp==void_type)
				error('s',"call of inline voidF in forE");
*/
		}
		if (e) e->simpl();
		if (e2) {
			curr_expr = e2;
			e2->simpl();
 			if (e2->base==ICALL && e2->tp==void_type)
				error('s',"call of inline voidF in forE");
		}
		{	Pstmt obl = break_del_list;
			Pstmt ocl = continue_del_list;
			break_del_list = 0;
			continue_del_list = 0;
			s->simpl();
			break_del_list = obl;
			continue_del_list = ocl;
		}
		break;

	case BLOCK:
		Pblock(this)->simpl();
		break;

	case PAIR:
		break;
	}

	/*if (s) s->simpl();*/
	if (base!=BLOCK && memtbl) {
		int i;
		Pstmt t1 = (s_list) ? s_list->simpl() : 0;
		Pstmt tx = t1 ? t1 : this;
//error('d',"(%d%k): t1 %d %k tx %d %k",base,base,t1?t1->base:0,t1?t1->base:0,tx?tx->base:0,tx?tx->base:0);
		Pstmt ss = 0;
		Pname cln;
		for (Pname tn = memtbl->get_mem(i=1); tn; tn=memtbl->get_mem(++i)) {
			if ( cln=tn->tp->is_cl_obj() ) {
				Pclass cl = (Pclass)cln->tp;
				Pname d = cl->has_dtor();
				if (d) {	/* n->cl::~cl(0); */
					Pref r = new ref(DOT,tn,d);
					Pexpr ee = new expr(ELIST,zero,0);
					Pcall dl = new call(r,ee);
					Pstmt dls = new estmt(SM,tn->where,dl,0);
					dl->base = G_CALL;
					dl->fct_name = d;
					dls->s_list = ss;
					ss = dls;
//error('d',"%d (tbl=%d): %n.%n %d->%d",this,memtbl,tn,d,ss,ss->s_list);
				}
			}
		}
		if (ss) {
			Pstmt t2 = ss->simpl();
			switch (base) {
			case IF:
			case WHILE:
			case DO:
			case SWITCH:
				temp_in_cond(e,ss,memtbl);
				break;

			case PAIR:	// can hide a return
			{	Pstmt ts = s2;
				while (ts->base==PAIR) ts = ts->s2;
				if (ts->base == RETURN) {	// sordid
					this = ts;
					goto retu;
				}
				goto def;
			}
			case RETURN:
			retu:
//error('d',"retu %d",e);
			{	
				if (e == 0) {
					// return; dtors; => dtors; return;
					Pstmt rs = new estmt(RETURN,where,0,0);
					rs->empty = empty;	// BSD fudge
					rs->ret_tp = ret_tp;
					base = PAIR;
					s = ss;
					s2 = rs;
					return t1 ? t1 : rs;
				}
				ccheck(e);
				Pname cln = e->tp->is_cl_obj();
//error('d',"xx %d %d",cln,Pclass(cln->tp)->has_oper(ASSIGN));
				if (cln==0 || Pclass(cln->tp)->has_oper(ASSIGN)==0) {
					//  ... return e; dtors; =>
					//  ... X r; ... r = e; dtors; return r;
					Pname rv = new name("_rresult"); // NOT "_result"
					rv->tp = ret_tp /* e->tp */;
					if (memtbl == 0) memtbl = new table(4,0,0);
					Pname n = rv->dcl(memtbl,FCT);
					n->n_assigned_to = 1;
					delete rv;
					Pstmt rs = new estmt(RETURN,where,n,0);
					rs->ret_tp = ret_tp;
					base = SM;
					e = new expr(ASSIGN,n,e);
					e->tp = n->tp;
					Pstmt ps = new pair(where,ss,rs);
					ps->s_list = s_list;
					s_list = ps;
					return t1 ? t1 : rs;
				}
			}
				
			case FOR:	// don't know which expression the temp comes from
				error('s',&where,"E in %kS needs temporary ofC%nW destructor",base,cln);
				break;

			case SM:	// place dtors after all "converted" DCLs
				if (t1) {
					for (Pstmt ttt, tt=this;
						(ttt=tt->s_list) && ttt->base==SM;
						tt = ttt) ;
					t2->s_list = ttt;
					tt->s_list = ss;
					return t1!=tt ? t1 : t2;
				}
			default:
			def:
//error('d',"%k: t1 %d t2 %d ss %d s_list %d",base,t1,t2,ss,s_list);
				if (e) ccheck(e);
				if (t1) {	// t1 == tail of statment list
					t2->s_list = s_list;
					s_list = ss;
					return t1;
				}
				s_list = ss;
				return t2;
			}
		}
		return (t1) ? t1 : this;
	}

	return (s_list) ? s_list->simpl() : this;
}

Pstmt stmt::copy()
// now handles dtors in the expression of an IF stmt
// not general!
{
	Pstmt ns = new stmt(0,curloc,0);

	*ns = *this;
	if (s) ns->s = s->copy();
	if (s_list) ns->s_list = s_list->copy();

	switch (base) {
	case PAIR:
		ns->s2 = s2->copy();
		break;
	}

	return ns;
}

void expr::simpl_new()
/*
	change NEW node to CALL node
*/
{
	Pname cln;
	Pname ctor;
	int sz = 1;
//	int esz;
	Pexpr var_expr = 0;
	Pexpr const_expr;
	Ptype tt = tp2;
	Pexpr arg;
	Pexpr szof;

	if ( cln=tt->is_cl_obj() ) {
		Pclass cl = (Pclass)cln->tp;
		if ( ctor=cl->has_ctor() ) {	/* 0->cl_ctor(args) */
			Pexpr p = zero;
			if (ctor->n_table != cl->memtbl) {
			/*	no derived constructor: pre-allocate */
				Pexpr ce = new texpr(SIZEOF,tt,0);
				ce->tp = uint_type;
				ce = new expr(ELIST,ce,0);
				p = new expr(G_CALL,new_fct,ce);
				p->fct_name = new_fct;
			}
			Pcall c = (Pcall)e1;
			c->e1 = new ref(REF,p,(Pname)c->e1);
		/*	c->set_fct_name(ctor);*/
			c->simpl();
			*this = *Pexpr(c);
			return;
		}
	}
	else if (cl_obj_vec) {
		Pclass cl = (Pclass)cl_obj_vec->tp;
		ctor = cl->has_ictor();
		if (ctor == 0) {
			if (cl->has_ctor()) error("new %s[], no defaultK",cl->string);
			cl_obj_vec = 0;
		}
	}

xxx:
	switch (tt->base) {
	case TYPE:
		tt = Pbase(tt)->b_name->tp;
		goto xxx;

	default:
	//	esz = tt->tsizeof();
	//	esz = szof;
		szof = new texpr(SIZEOF,tt,0);
		szof->tp = uint_type;
		break;

	case VEC:
	{	Pvec v = (Pvec)tt;
		if (v->size)
			sz *= v->size;
		else if (v->dim)
			var_expr = (var_expr) ? new expr(MUL,var_expr,v->dim) : v->dim;
		else {
			sz = SZ_WPTR;		// <<< sizeof(int*)
			break;
		}
		tt = v->typ;
		goto xxx;
		}
	}

	if (cl_obj_vec) { // _vec_new(0,no_of_elements,element_size,ctor)
		const_expr = new ival(sz);
		Pexpr noe = (var_expr) ? (sz!=1) ? new expr(MUL,const_expr,var_expr) : var_expr : const_expr;
	//	const_expr = new ival(esz);
		const_expr = szof;
		const_expr->tp = uint_type;
		base = CALL;
		arg = new expr(ELIST,ctor,0);
		/*ctor->take_addr();*/
		ctor->lval(ADDROF);
		arg = new expr(ELIST,const_expr,arg);
		arg = new expr(ELIST,noe,arg);
		arg = new expr(ELIST,zero,arg);
		base = CAST;
		tp2 = tp;
		e1 = new expr(G_CALL,vec_new_fct,arg);
		e1->fct_name = vec_new_fct;
		simpl();
		return;
	}

	/* call _new(element_size*no_of_elements) */
//	sz *= esz;
	if (sz == 1)
		arg = (var_expr) ? new expr(MUL,szof,var_expr) : szof;
	else {
		const_expr = new ival(sz);
		const_expr->tp = uint_type;
		const_expr = new expr(MUL,const_expr,szof);
		const_expr->tp = uint_type;
		arg = (var_expr) ? new expr(MUL,const_expr,var_expr) : const_expr;
	}
//	arg = (var_expr) ? (sz!=1) ? new expr(MUL,const_expr,var_expr) : var_expr : const_expr;
	arg->tp = uint_type;
//error('d',"new: (%t)_new(...)",tp);
	base = CAST;
	tp2 = tp;
	e1 = new expr(G_CALL,new_fct,new expr(ELIST,arg,0));
	e1->fct_name = new_fct;
	simpl();
}

void expr::simpl_delete()
/*
	delete p => _delete(p);
		    or  cl::~cl(p,1);
	delete[s]p => _delete(p);
			or vec_del_fct(p,vec_sz,elem_sz,~cl,1);
*/		 
{
	Ptype tt = e1->tp;

ttloop:
	switch (tt->base) {
	case TYPE:	tt = Pbase(tt)->b_name->tp; goto ttloop;
	case VEC:
	case PTR:	tt = Pptr(tt)->typ; break;
	}

	Pname cln = tt->is_cl_obj();
	Pclass cl;
	Pname n;

        if (cln && (n=Pclass(cln->tp)->has_dtor())) {   // ~cl() might be virtual                
                Pexpr r = e1;
                if (e2 == 0) {          // e1->cl::~cl(1)
        		Pexpr rrr = new ref(REF,r,n);
        		Pexpr aaa = new expr(ELIST,one,0);
        		Pexpr ee = new call(rrr,aaa);
        		ee->fct_name = Pname(n);
        		ee->base = G_CALL;
                        if (Pfct(n->tp)->f_virtual) {
                                ee = new expr(QUEST,ee,zero);
                                ee->tp = ee->e1->tp;
                                ee->cond = r;
                        }
                        *this = *ee;
                        delete ee;
                        simpl();
                        return;
                }
		else {		// del_cl_vec(e1,e2,elem_size,~cl,1);
			Pexpr sz = new texpr(SIZEOF,tt,0);
			sz->tp = uint_type;
			Pexpr arg = one;
			if (Pfct(n->tp)->f_virtual) {
				// beware of side effects in expression e1
				if (e1->base != NAME) error('s',"PE too complicated for delete[]");
				Pexpr a = new ref(REF,e1,n);
				a = a->address();
				arg = new expr(ELIST,a,arg);
			}
			else {
				arg = new expr(ELIST,n,arg);
				n->lval(ADDROF);	// n->take_addr();
			}
			arg = new expr(ELIST,sz,arg);
			arg = new expr(ELIST,e2,arg);
			arg = new expr(ELIST,e1,arg);
			base = G_CALL;
			e1 = vec_del_fct;
			e2 = arg;
			fct_name = vec_del_fct;
		}
	}
	else if (cl_obj_vec) {
		error('s',"delete vector of vectors");
	}
	else {					// _delete(e1)
		base = G_CALL;
		e2 = new expr(ELIST,e1,0);
		e1 = fct_name = del_fct;
	}
	//	*this = *typ(gtbl);
	Pcall(this)->simpl();
}
