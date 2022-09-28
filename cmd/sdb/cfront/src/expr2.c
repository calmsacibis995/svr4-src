/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/expr2.c	1.1"
/*ident	"@(#)cfront:src/expr2.c	1.24" */
/***************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.

expr2.c:

	type check expressions

************************************************************************/

#include "cfront.h"
#include "size.h"

static refd;
int tsize;
int t_const;

Pname make_tmp(char c, Ptype t, Ptable tbl)
{
	if (Cstmt) {	//	make Cstmt into a block
		if (Cstmt->memtbl == 0) Cstmt->memtbl = new table(4,tbl,0);
		tbl = Cstmt->memtbl;
	}
	Pname tmpx = new name(make_name(c));
	tmpx->tp = t;
	Pname tmp = tmpx->dcl(tbl,ARG); // ARG => no init
	Pname nn = tbl->t_name;
        if (nn && nn->tp->base == CLASS) tsize=tmp->tp->tsizeof();
	delete tmpx;
	tmp->n_scope = FCT;
	return tmp;
}

Pexpr init_tmp(Pname tmp, Pexpr init, Ptable tbl)
{
	Pname cn = tmp->tp->is_cl_obj();
	Pname ct = cn ? Pclass(cn->tp)->has_itor() : 0;
	Pexpr ass;

	tmp->n_assigned_to = 1;

	if (ct) {	// must initialize
		if (refd && 
                   (init->e1->base == NAME || init->e1->base==REF))
			init = new expr(G_CM,init,init->e1->address());

		if (refd) tbl = 0;
		Pref r = new ref(DOT,tmp,ct);
		Pexpr a = new expr(ELIST,init,0);
		ass = new expr(G_CALL,r,a);
		ass->fct_name = ct;
		if (tbl) ass = ass->typ(tbl);
	}
	else {		// can assign
		ass = new expr(ASSIGN,tmp,init);
		ass->tp = tmp->tp;
	}
	return ass;
}

void name::assign()
{
	if (n_assigned_to++ == 0) {
		switch (n_scope) {
		case FCT:
			if (n_used && n_addr_taken==0)  {
				Ptype t = tp;
			ll:
				switch (t->base) {
				case TYPE:
					t=Pbase(t)->b_name->tp; goto ll;
				case VEC:
					break;
				default:
					if (curr_loop)
						error('w',"%n may have been used before set",this);
					else
						error('w',"%n used before set",this);
				}
			}
		}
	}
}

int expr::lval(TOK oper)
{
	register Pexpr ee = this;
	register Pname n;
	int deref = 0;
	char* es;

	//if (this==0 || tp==0) error('i',"%d->lval(0)",this);

	switch (oper) {
	case ADDROF:
	case G_ADDROF:	es = "address of";	break;
	case INCR:
	case DECR:	es = "increment of";	goto def;
	case DEREF:	es = "dereference of";	break;
	default:	es = "assignment to";
	def:
		if (tp->tconst()) {
			if (oper) {
				if (base == NAME)
					error("%s constant%n",es,this);
				else
					error("%s constant",es);
			}
			return 0;
		}
	}
//error('d',"lval %s",es);
	for(;;) {
//error('d',"ee %d %k",ee->base,ee->base);
		switch (ee->base) {
		case G_CALL:
		case CALL:
			if (deref == 0) {
				switch (oper) {
				case ADDROF:
				case G_ADDROF:
				case 0:
					if (ee->fct_name
					&& Pfct(ee->fct_name->tp)->f_inline) return 1;
				}
			}
		default:
			if (deref == 0) {
				if (oper) error("%s %k (not an lvalue)",es,ee->base);
				return 0;
			}
			return 1;
		case ZERO:
		case CCON:
		case ICON:
		case FCON:
			if (oper) error("%s numeric constant",es);
			return 0;
		case STRING:
			if (oper) error('w',"%s string constant",es);
			return 1;

		case DEREF:
		{	Pexpr ee1 = ee->e1;
			if (ee1->base == ADDROF) /* *& vanishes */
				ee = ee1->e2;
			else {
				ee = ee1;
				deref = 1;
			}
			break;
		}

		case DOT:
//error('d',"lval dot: %k",ee->e1->base);
			switch (ee->e1->base) {		// update use counts, etc.
			case NAME:
//error('d',"lval dot: %n (oper %d)",Pname(ee->e1),oper);
				switch (oper) {
				case ADDROF:
				case G_ADDROF:	Pname(ee->e1)->take_addr();
				case 0:		break;
				case ASSIGN:	Pname(ee->e1)->n_used--;
				default:	Pname(ee->e1)->assign(); // asop
				}
				break;
			case DOT:
				Pexpr e = ee->e1;
				do e=e->e1; while (e->base==DOT);
				if (e->base == NAME) {
//error('d',"lval dot.dot: %n (oper %d)",Pname(e),oper);
					switch (oper) {
					case ADDROF:
					case G_ADDROF:	Pname(e)->take_addr();
					case 0:		break;
					case ASSIGN:	Pname(e)->n_used--;
					default:	Pname(e)->assign(); // asop
					}
				}
			}
			n = ee->mem;
			if (deref==0 && ee->e1->tp->tconst()) {
				switch (oper) {
				case 0:
				case ADDROF:
				case G_ADDROF:
				case DEREF:
					break;
				default:
					 error("%sM%n of%t",es,n,ee->e1->tp);
				}
				return 0;
			}
			goto xx;

		case REF:
			n = ee->mem;
			if (deref==0) {
				Ptype p = ee->e1->tp;
			zxc:
				switch (p->base) {
				case TYPE:	p = Pbase(p)->b_name->tp; goto zxc;
				case PTR:	
				case VEC:	break;
				default:	error('i',"%t->%n",p,n);
				}
				if (Pptr(p)->typ->tconst()) {
					switch (oper) {
					case 0:
					case ADDROF:
					case G_ADDROF:
					case DEREF:
						break;
					default:
						error("%sM%n of%t",es,n,Pptr(p)->typ);
					}
					return 0;
				}
			}
			goto xx;
		case NAME:
			n = Pname(ee);
		xx:
//error('d',"name%n xx:",n); 
			if (deref || oper==0) return 1;

			if (n->tp->base==FIELD && Pbase(n->tp)->b_bits==0) {
				error("%s 0-length field%n",es,n);
				return 0;
			}
			switch (oper) {
			case ADDROF:
			case G_ADDROF:
			{	Pfct f = (Pfct)n->tp;
				if (n->n_sto == REGISTER) {
					error("& register%n",n);
					return 0;
				}
				if (f == 0) {
					error("& label%n",n);
					return 0;
				}
				if (n->n_stclass == ENUM) {
					error("& enumerator%n",n);
					return 0;
				}
				if (n->tp->base==FIELD) {
					error("& field%n",es,n);
					return 0;
				}
				n->n_used--;
if (n->n_qualifier) // oops, not the real one
	n = Pclass(n->n_table->t_name->tp)->memtbl->look(n->string,0);
				n->take_addr();
//error('d',"%n: e %d s %d b %d i%d",n,n->n_evaluated,n->n_scope,f->base,f->f_inline);
				if ( (n->n_evaluated && n->n_scope!=ARG)
				|| (f->base==FCT && f->f_inline) ) {
					// address of const or inline: allocate it
					Pname nn = new name;
					if (n->n_evaluated && n->n_scope!=ARG) {
						n->n_evaluated = 0;	/* use allocated version */
						n->n_initializer = new ival(n->n_val);
					}
					*nn = *n;
//error('d',"dcl_list %d: %n",dcl_list,nn);
					nn->n_sto = STATIC;
					nn->n_list = dcl_list;
					dcl_list = nn;
				}
				break;
			}
			case ASSIGN:
				n->n_used--;
				n->assign();
				break;
			default:	/* incr ops, and asops */
				if (cc->tot && n==cc->c_this) {
					error("%n%k",n,oper);
					return 0;
				}
				n->assign();
			}
			return 1;
		}
	}
}

Pexpr Ninit;	// default argument used;
int Nstd;	// standard coercion used (derived* =>base* or int=>long or ...)

bit gen_match(Pname n, Pexpr arg)
/*
	look for an exact match between "n" and the argument list "arg" 
*/
{
	Pfct f = Pfct(n->tp);
	register Pexpr e;
	register Pname nn;

	for (e=arg, nn=f->argtype; e; e=e->e2, nn=nn->n_list) {
		Pexpr a = e->e1;
		Ptype at = a->tp;
		if (at->base == ANY) return 0;
		if (nn == 0) return f->nargs_known==ELLIPSIS;

		Ptype nt = nn->tp;

		switch (nt->base) {
		case RPTR:
			if (at == zero_type) return 0; //break;
			if (nt->check(at,COERCE)) {
				Pptr pt = at->addrof();
				nt->base = PTR;		// handle derived classes
				if (nt->check(pt,COERCE)) {
					nt->base = RPTR;
					delete pt;
					return 0;
				}
				nt->base = RPTR;
				delete pt;
			}
			break;
		default:
			switch( at->base ) 
			{
			default: 
				if (nt->check(at,COERCE)) return 0;
				break;
			case OVERLOAD:
			{
				register Plist gl;
				Pgen g = (Pgen)at;
				int no_match = 1;

				for (gl = g->fct_list; gl; gl=gl->l) 
				{
					Pname nn = gl->f;
					Ptype t  = nn->tp;
					if (nt->check(t,COERCE)==0) 
				   		{ no_match = 0; break; }
				}

				if ( no_match ) return 0;
			} 
			} 
		}
	}

	if (nn) {
		Ninit = nn->n_initializer;
		return Ninit!=0;
	}

	return 1;
}

Pname Ncoerce;

bit can_coerce(Ptype t1, Ptype t2)
/*	return number of possible coercions of t2 into t1,
	Ncoerce holds a coercion function (not constructor), if found
*/
{
	Ncoerce = 0;
	if (t2->base == ANY) return 0;

	switch (t1->base) {
	case RPTR:
	rloop:
		switch (t2->base) {
		case TYPE:
			t2 = Pbase(t2)->b_name->tp;
			goto rloop;
	//	case VEC:
	//	case PTR:
	//	case RPTR:
	//		if (t1->check(t2,COERCE) == 0) return 1;
		default:	
		{	Ptype tt2 = t2->addrof();
			if (t1->check(tt2,COERCE) == 0) return 1;
			Ptype tt1 = Pptr(t1)->typ;
			int i = can_coerce(tt1,t2);
			return i;
		}
		}
	}

	Pname c1 = t1->is_cl_obj();
	Pname c2 = t2->is_cl_obj();
	int val = 0;

	if (c1) {
		Pclass cl = (Pclass)c1->tp;
		if (c2 && c2->tp==cl) return 1;

		/*	look for constructor
				with one argument
				or with default for second argument
			of acceptable type
		*/
		Pname ctor = cl->has_ctor();
		if (ctor == 0) goto oper_coerce;
		register Pfct f = (Pfct)ctor->tp;

		switch (f->base) {
		case FCT:
			switch (f->nargs) {
			case 1:
			one:
			{	Ptype tt = f->argtype->tp;
				if (tt->check(t2,COERCE)==0) val = 1;
				if (tt->base == RPTR) {
					Pptr pt = t2->addrof();	// handle derived classed
					tt->base = PTR;
					if (tt->check(pt,COERCE) == 0) val = 1;
					tt->base = RPTR;
					delete pt;
				}
				goto oper_coerce;
			}
			default:
				if (f->argtype->n_list->n_initializer) goto one;
			case 0:
				goto oper_coerce;
			}
		case OVERLOAD:
		{	register Plist gl;

			for (gl=Pgen(f)->fct_list; gl; gl=gl->l) { // look for match
				Pname nn = gl->f;
				Pfct ff = (Pfct)nn->tp;
				switch (ff->nargs) {
				case 0:
					break;
				case 1:
				over_one:
				{	Ptype tt = ff->argtype->tp;
					if (tt->check(t2,COERCE) == 0) val = 1;
					if (tt->base == RPTR) {
						Pptr pt = t2->addrof();	// handle derived classed
						tt->base = PTR;
						if (tt->check(pt,COERCE) == 0) {
							tt->base = RPTR;
							delete pt;
							val = 1;
							goto oper_coerce;
						}
						tt->base = RPTR;
						delete pt;
					}
					break;
				}
				default:
					if (ff->argtype->n_list->n_initializer) goto over_one;
				}
			}
			goto oper_coerce;
		}
		default:
			error('i',"cannot_coerce(%k)\n",f->base);
		}
	}
oper_coerce:
	if (c2) {	
		Pclass cl = (Pclass)c2->tp;
		int std = 0;
		for (register Pname on=cl->conv; on; on=on->n_list) {
//error('d',"oper_coerce%n %t %d",on,(on)?on->tp:0,on);
			Pfct f = (Pfct)on->tp;
			Nstd = 0;
			if (t1->check(f->returns,COERCE) == 0) {
				if (Nstd==0) {	// forget solutions involving standard conversions
					if (std) {	// forget
						val = 1;
						std = 0;
					}
					else
						val++;
					Ncoerce = on;
				}
				else {	// take note only if no exact match seen
					if (val==0 || std) {
						Ncoerce = on;
						val++;
						std = 1;
					}
				}
			}
		}
	}

	if (val) return val;
	if (c1 && Pclass(c1->tp)->has_itor()) return 0;
	if (t1->check(t2,COERCE)) return 0;
	return 1;
}

int gen_coerce(Pname n, Pexpr arg)
/*
	look to see if the argument list "arg" can be coerced into a call of "n"
	1: it can
	0: it cannot or it can be done in more than one way
*/
{
	Pfct f = (Pfct) n->tp;
	register Pexpr e;
	register Pname nn;
//error('d',"gen_coerce%n %d",n,arg);
	for (e=arg, nn=f->argtype; e; e=e->e2, nn=nn->n_list) {
		if (nn == 0) return f->nargs_known==ELLIPSIS;
		Pexpr a = e->e1;
		Ptype at = a->tp;
		int i = can_coerce(nn->tp,at);
//error('d',"a1 %k at%t argt%t -> %d",a->base,at,nn->tp,i);
		if (i != 1) return 0;
	}
	if (nn && nn->n_initializer==0) return 0;
	return 1;
}


Pname Nover;
int Nover_coerce;

int over_call(Pname n, Pexpr arg)
/*	
	return 2 if n(arg) can be performed without user defined coercion of arg
	return 1 if n(arg) can be performed only with user defined coercion of arg
	return 0 if n(arg) is an error
	Nover is the function found, if any
*/
{	
	register Plist gl;
	Pgen g = (Pgen) n->tp;
	if (arg && arg->base!= ELIST) error('i',"ALX");
//error('d',"over_call%n base%k arg %d%k", n, g->base, arg, arg?arg->tp->base:0);
	Nover_coerce = 0;
	switch (g->base) {
	default:	error('i',"over_call(%t)\n",g);
	case OVERLOAD:	break;
	case FCT:
		Nover = n;
		Ninit = 0;
		if (gen_match(n,arg) && Ninit==0) return 2;
		return gen_coerce(n,arg);
	}

	Pname exact = 0;
	int no_exact = 0;
	for (gl=g->fct_list; gl; gl=gl->l) {		/* look for match */
		Nover = gl->f;
		Ninit = 0;
		Nstd = 0;
//error('d',"exact? %n",Nover);
		if (gen_match(Nover,arg) && Ninit==0) {
//error('d',"%n: nstd %d",Nover,Nstd);
			if (Nstd == 0) return 2;
			if (exact)
				no_exact++;
			else
				exact = Nover;
		}
			
	}

	if (exact) {
//error('d',"exact%n %d",exact,no_exact);
		if (no_exact) error('w',"more than one standard conversion possible for%n",n);
		Nover = exact;
		return 2;
	}
//error('d',"exact == 0");
	Nover = 0;
	for (gl=g->fct_list; gl; gl=gl->l) {		/* look for coercion */
		Pname nn = gl->f;
//error('d',"over_call: gen_coerce(%n,%k) %d",nn,arg->e1->base,gen_coerce(nn,arg));
		if (gen_coerce(nn,arg)) {
			if (Nover) {
				Nover_coerce = 2;
				return 0;		/* ambiguous */
			}
			Nover = nn;
		}
	}

	return Nover ? 1 : 0;
}

void visible_check(Pname found, char* string, Pexpr e, bit no_virt)
/*
	is ``found'' visible?
*/
{

	Pexpr e1 = e->e1;
	Pbase b;

	if ( e->base == NEW ) {
 	if ( found->tp->base == OVERLOAD ) {
    		Ninit = 0; int no_match = 1;
    		for ( Plist gl=Pgen(found->tp)->fct_list; gl; gl=gl->l ) 
		{ 

        		if ( e->e1 == 0 ) {
           			if ( Pfct(gl->f->tp)->nargs_known &&
                		     Pfct(gl->f->tp)->nargs == 0 )
              			{ found = gl->f; break; }
	   		else continue;
			}

        		if ( gen_match( gl->f, e ) && Ninit == 0 )
             			{ found = gl->f; break; }

		        if ( no_match && gen_coerce( gl->f, e ) )
             			{no_match = 0; found=gl->f; }
			} 
	}
	Pfct fn = (found->tp->base == OVERLOAD)
		  ? Pfct(Pgen(found->tp)->fct_list->f->tp)
		  : Pfct(found->tp);
	Ptype pt = fn->s_returns;
	b = Pbase( Pptr(pt)->typ );
	goto xxxx;
}

	if (e1)
	switch (e1->base) {
	default:
		if (no_virt) e->e1 = found;	// instead of using fct_name
		return;

	case REF:
	{	if (no_virt) e1->mem = found;	// instead of using fct_name
		if (e1->e1 == 0) return;	// constructor: this==0
		for (Ptype pt=e1->e1->tp; pt->base==TYPE; pt=Pbase(pt)->b_name->tp);
		b = Pbase(Pptr(pt)->typ);
		break;
	}
	case DOT:
		if (no_virt) e1->mem = found;	// instead of using fct_name
		b = Pbase(e1->e1->tp);
	}

xxxx:
	switch (b->base) {
	case TYPE:	b = Pbase(b->b_name->tp);	goto xxxx;
	case ANY:	return;
	case COBJ:	break;
	default:	error('i',"no tblx %p",b);
	}

	Ptable tblx = b->b_table;
	if (tblx->base!=TABLE) error('i',"tblx %p %d",tblx,tblx->base);
	if (tblx->lookc(string,0) == 0) return; // error('i',"visibility check %p %s",tblx,string);

	switch (found->n_scope) {
	case 0:
		if (Epriv
		&& Epriv!=cc->cot
		&& !Epriv->has_friend(cc->nof)
		&& !(found->n_protect && Epriv->baseof(cc->nof))) {
			error("%n is %s",found,found->n_protect?"protected":"private");
			break;
		}
		/* no break */
	case PUBLIC:
		if (Ebase && (cc->cot==0 || (Ebase!=cc->cot->clbase->tp && !Ebase->has_friend(cc->nof))))
			  error("%n is from a privateBC",found);
	}
}

Ptype expr::fct_call(Ptable tbl)
/*
	check "this" call:
		 e1(e2)
	e1->typ() and e2->typ() has been done
*/
{
	Pfct f;
	Pname fn;
	int x;
	int k;
	Pname nn;
	Pexpr e;
	Ptype t;
	Pexpr arg = e2;
	Ptype t1;
	int argno;
	Pexpr etail = 0;
	Pname no_virt;	// set if explicit qualifier was used: c::f()

	switch (base) {
	case CALL:
	case G_CALL:	break;
	default:	error('i',"fct_call(%k)",base);
	}

	if (e1==0 || (t1=e1->tp)==0) error('i',"fct_call(e1=%d,e1->tp=%t)",e1,t1);
	if (arg && arg->base!=ELIST) error('i',"badAL%d%k",arg,arg->base);

//error('d',"fct_call %d %k",e1->base,e1->base);
	switch (e1->base) {
	case NAME:
		fn = (Pname)e1;
		switch (fn->n_oper) {
		case 0:
		case CTOR:
		case DTOR:
		case TYPE:
			break;
		default:	// real operator: check for operator+(1,2);
			if (arg == 0) break;
			Pexpr a = arg->e1;	// first operand
			if (a->tp->is_cl_obj() || a->tp->is_ref()) break;
			a = arg->e2;
			if (a == 0)		// unary
				error("%k of basicT",fn->n_oper);
			else {			// binary
				a = a->e1;	// second operand
				if (a->tp->is_cl_obj() || a->tp->is_ref()) break;
				error("%k of basicTs",fn->n_oper);
			}
			break;
		}
		no_virt = fn->n_qualifier;
		break;
	case REF:
	case DOT:
		fn = e1->mem;
		no_virt = fn->n_qualifier;
		break;
	case MEMPTR:
	default:
		fn = 0;
		no_virt = 0;
	};

lll:
	switch (t1->base) {
	case TYPE:
		t1 = Pbase(t1)->b_name->tp;
		goto lll;

	case PTR:	// pf() allowed as shorthand for (*pf)()
		if (Pptr(t1)->typ->base == FCT) {
			if (Pptr(t1)->memof) error("O missing in call throughP toMF");
			t1 = Pptr(t1)->typ;
			fn = 0;
			goto case_fct;
		}

	default:
		error("call of%n;%n is a%t",fn,fn,e1->tp);

	case ANY:
		return any_type;
	
	case OVERLOAD:
	{	register Plist gl;
		Pgen g = (Pgen) t1;
		Pname found = 0;
		Pname exact = 0;
		int no_exact = 0;

		for (gl=g->fct_list; gl; gl=gl->l) {	// look for match
			register Pname nn = gl->f;
			Ninit = 0;
			Nstd = 0;

			if (gen_match(nn,arg)) {
				if (Nstd == 0)  {
					found = nn;
					goto fnd;
				}
				if (exact)
					no_exact++;
				else
					exact = nn;
			}
			
		}

		if (exact) {
			if (no_exact) error('w',"%d standard conversion possible for%n",no_exact+1,fn);
			found = exact;
			goto fnd;
		}

//error('d',"exact == 0");
		for (gl=g->fct_list; gl; gl=gl->l) {	/* look for coercion */
			register Pname nn = gl->f;
//error('d',"gen_coerce %s %d\n",nn->string?nn->string:"?",arg->base);
			if (gen_coerce(nn,arg)) {
				if (found) {
					error("ambiguousA for overloaded%n",fn);
					goto fnd;
				}
				found = nn;
			}
		}
	
	fnd:
		if (fct_name = found) {
//error('d',"found%n %s %s",found,found->string,g->string);
			f = (Pfct)found->tp;
			visible_check(found,g->string,this,no_virt!=0);
		}
		else {
			error("badAL for overloaded%n",fn);
			return any_type;
		}
		break;
	}
	case FCT:
	case_fct:
//error('d',"fct %n %s",fn,fn->string);
		f = Pfct(t1);
		if ((fct_name=fn) && fn->n_oper==CTOR) {
			visible_check(fn,fn->string,this,0);
		}
	}

	if (no_virt) fct_name = 0;

	t = f->returns;
	x = f->nargs;
	k = f->nargs_known;
//error('d',"fct_name%n",fct_name);

	if (k == 0) {
		if (fct_void && fn && x==0 && arg)
			if (no_of_badcall++ == 0) badcall = fn;
		goto rlab;
	}

	for (e=arg, nn=f->argtype, argno=1; e||nn; nn=nn->n_list, e=etail->e2, argno++) {
		Pexpr a;

		if (e) {
			a = e->e1;
//error('d',"e %d%k a %d%k e2 %d",e,e->base,a,a->base,e->e2);
			etail = e;

			if (nn) {	/* type check */
				Ptype t1 = nn->tp;
			lx:
				switch (t1->base) {
				case TYPE:
					if (!t_const) t_const = Pbase(t1)->b_const;
					t1 = Pbase(t1)->b_name->tp;
					goto lx;
				case RPTR:
					a = ref_init(Pptr(t1),a,tbl);
					goto cbcb;
				case COBJ:
					if (a->base!=G_CM
					|| t1->check(a->tp,ASSIGN))
						a = class_init(0,t1,a,tbl);
					if (nn->n_xref) {
						// (temp.ctor(arg),&arg)
						a = a->address();
					}
					else {
		// defend against:
		//	int f(X); ... X(X&);
		Pname cln = Pbase(t1)->b_name;	
		if (cln && Pclass(cln->tp)->has_itor()) {
			// mark X(X&) arguments
			nn->n_xref = 1;
			a = a->address();
		}
					}
				cbcb:
					if (a->base==G_CM) {
						if (a->e1->base==DEREF) a->e1 = a->e1->e2; // (*e1,e2) => (e1,e2)
						if (a->e1->base==G_CALL
						&& Pname(a->e1->fct_name)
						&& Pname(a->e1->fct_name)->n_oper==CTOR
						&& (a->e2->base==G_ADDROF || a->e2->base==ADDROF))
							a = a->e1;	// (ctor(&tmp),&tmp) => ctor(&tmp)
					}
					e->e1 = a;
					break;
				case ANY:
					goto rlab;
				case PTR:
					e->e1 = a = ptr_init(Pptr(t1),a,tbl);
					goto def;
				case CHAR:
				case SHORT:
				case INT:
					if (a->base==ICON && a->tp==long_type)
						error('w',"long constantA for%n,%kX",fn,t1->base);
				case LONG:
					if (Pbase(t1)->b_unsigned
					&& a->base==UMINUS
					&& a->e2->base==ICON)
						error('w',"negativeA for%n, unsignedX",fn);
				default:
				def:
				{	Pexpr x = try_to_coerce(t1,a,"argument",tbl);
					if (x) 
						e->e1 = x;
					else if (t1->check(a->tp,ARG)){
						if (arg_err_suppress==0) error("badA %dT for%n:%t (%tX)",argno,fn,a->tp,nn->tp);
						return any_type;
					}
				}
				}
			}
			else {
				if (k != ELLIPSIS) {
					if (arg_err_suppress==0) error("unexpected %dA for%n",argno,fn);
					return any_type;
				}
				goto rlab;
			}
		}
		else {	/* default argument? */
			a = nn->n_initializer;
//error('d',"arg missing: %n %d as %d",nn,a,arg_err_suppress);
			if (a == 0) {
				if (arg_err_suppress==0) error("A %d ofT%tX for%n",argno,nn->tp,fn);
				return any_type;
			}
//error('d',"%n: perm=%d",nn,a->permanent);
			a->permanent = 2;	// ought not be necessary, but it is
			e = new expr(ELIST,a,0);
			if (etail)
				etail->e2 = e;
			else
				e2 = e;
			etail = e;
		}
	}
rlab:
//error('d',"rlab %n: res %d used %d",fn,f->f_result,fct_name->n_used);
	if (fn && f->f_result==0) {
		// protect against class cn; cn f(); ... class cn { cn(cn&): ... };
		Pname cn = f->returns->is_cl_obj();
		if (cn && Pclass(cn->tp)->has_itor()) {
			int ll = fct_name->n_table==gtbl ? 2 : 1;	// amazing fudge: use count doubled!
			if (ll<fct_name->n_used) error('s',"%n returning %n called before %s(%s&)D seen",fn,cn,cn->string,cn->string);
			make_res(f);
		}
	}
	if (f->f_result) {		// f(args) => f(&temp,args),temp
		Pname tn = make_tmp('R',f->returns,tbl);
		e2 = new expr(ELIST,tn->address(),e2);
		Pexpr ee = new expr(0,0,0);
		*ee = *this;
		base = G_CM;		// f(&temp,args),temp
		e1 = ee;
		e2 = tn;
	}

	return t;
}

Pexpr ref_init(Pptr p, Pexpr init, Ptable tbl)
/*
	initialize the "p" with the "init"
*/
{
	register Ptype it = init->tp;
	Ptype p1 = p->typ;
	Pname c1 = p1->is_cl_obj();

rloop:
	switch (it->base) {
	case TYPE:
		it = Pbase(it)->b_name->tp; goto rloop;
	default:
		{	Ptype tt = it->addrof();
			p->base = PTR;	// allow &x for y& when y : public x
					// but not &char for int&
			int x = p->check(tt,COERCE);
			if (x == 0) {
				if (init->tp->tconst() && vec_const == 0) {
					// not ``it''
					Pptr(tt)->rdo = 1;
					if (p->typ->tconst() == 0) error("R to constO");
				}
				if (p->check(tt,COERCE)) error("R to constO");
				p->base = RPTR;

				if (init->lval(0)) return init->address();

//				if (init->base==G_CALL	// &inline function call?
//				&& init->fct_name
//				&& Pfct(init->fct_name->tp)->f_inline )
//					return init->address();

			//	p1 = p->typ;
				goto xxx;
			}
			p->base = RPTR;
		}
	}

	if (c1) {
		refd = 1;	/* disable itor */
		Pexpr a = class_init(0,p1,init,tbl);
		refd = 0;
		if (a==init && init->tp!=any_type) goto xxx;
		switch (a->base) {
		case G_CALL:
		case CM:
			init = a;
			goto xxx;
		}
		return a->address();
	}

	if (p1->check(it,0)) {
		error("badIrT:%t (%tX)",it,p);
		if (init->base != NAME) init->tp = any_type;
		return init;
	}

xxx:	/*
		here comes the test of a ``fundamental theorem'':
		a structure valued expression is
			(1) an lvalue of type T (possibly const)
		or	(2) the result of a function (a _result if X(X&) is defined)
		or	(3) a * or [] or ? or , expression
	*/
	switch (init->base) {
	case NAME:
	case DEREF:
	case REF:
	case DOT:			// init => &init
		if (it->tconst() && vec_const==0) goto def;
		init->lval(ADDROF);

	case G_CM:			// & (f(&temp), temp)
		return init->address();
		
	case QUEST:
//error('d',"quest %n ->%d",c1,Pclass(c1->tp)->has_itor());
		switch (init->e1->base) {	// try for: &(c?a:b) => c?&a:&b
		case NAME:
		case DEREF:
		case REF:
		case DOT:
			if (init->e1->tp->tconst() && vec_const==0) break;
		case G_CM:
			switch (init->e2->base) {
			case NAME:
			case DEREF:
			case REF:
			case DOT:
				if (init->e2->tp->tconst() && vec_const==0) break;
			case G_CM:
				init->e1 = init->e1->address();
				init->e2 = init->e2->address();
				return init;
			}
		}
		
		if (Pclass(c1->tp)->has_itor()) {
			error('s',"?:E ofT%n: %s(%s&)Dd",c1,c1->string,c1->string);
			return init;
		}
		goto def;
	case CALL:
	case G_CALL:
//		if (Pclass(c1->tp)->has_itor()) { }
		goto def;
		
	case CM:			// try for &(... , b) => (... , &b)
	{	Pexpr ee = init->e2;
	cml:
		switch (ee->base) {
		case CM:	ee = ee->e2; goto cml;
		case G_CM:
		case NAME:
		case DEREF:	return init->address();
		}
		// no break
	}

	default:
	def:
	{	
		if (tbl == gtbl) error('s',"Ir for staticR not an lvalue");
		Pname n = make_tmp('I',p1,tbl);
		Pexpr a;
		if (c1 != init->tp->is_cl_obj()) {
			// derived class => must cast: ``it Ix; (Ix=init,(p)&Ix);''
			n->tp = init->tp;
			a = n->address();
			PERM(p);
			a = new texpr(CAST,p,a);
			a->tp = p;
		}
		else
			a = n->address();

		if (init->base == ASSIGN && init->e1->base == DEREF)
			init = new expr( G_CM, init, init->e1->e1 );

		refd = 1;
		Pexpr as = init_tmp(n,init,tbl);
		refd = 0;
		a = new expr(CM,as,a);
		a->tp = a->e2->tp;
		return a;
	}
	}
}

Pexpr class_init(Pexpr nn, Ptype tt, Pexpr init, Ptable tbl)
/*
	initialize "nn" of type "tt" with "init"
	if nn==0 make a temporary,
	nn may not be a name
*/
{	Pname c1 = tt->is_cl_obj();
	Pname c2 = init->tp->is_cl_obj();

	if (c1) {
		if (c1!=c2
		|| (refd==0 && Pclass(c1->tp)->has_itor())) {
			/*	really ought to make a temp if refd,
				but ref_init can do that
			*/
			int i = can_coerce(tt,init->tp);
			switch (i) {
			default:
				error("%d ways of making a%n from a%t",i,c1,init->tp);
				init->tp = any_type;
				return init;
			case 0:
				error("cannot make a%n from a%t",c1,init->tp);
				init->tp = any_type;
				return init;
			case 1:
				if (Ncoerce == 0) {
					Pexpr a = new expr(ELIST,init,0);
					a = new texpr(VALUE,tt,a);
					a->e2 = nn;
					return a->typ(tbl);
				}

				switch (init->base) {
#ifdef BSD
				case CALL:
				case G_CALL:
#endif
				case CM:
				case NAME:	/* init.coerce() */
				{	Pref r = new ref(DOT,init,Ncoerce);
					Pexpr rr = r->typ(tbl);
					init = new expr(G_CALL,rr,0);
					init->fct_name = Ncoerce;
					break;
				}
				default:	// (temp=init,temp.coerce())
				{	Pname tmp = make_tmp('U',init->tp,tbl); 
					Pexpr ass = init_tmp(tmp,init,tbl);
					Pref r = new ref(DOT,tmp,Ncoerce);
					Pexpr rr = r->typ(tbl);
					Pexpr c = new expr(G_CALL,rr,0);
					c->fct_name = Ncoerce;
					init = new expr(CM,ass,c);
				}
				}
				if (nn) {
					Pexpr a = new expr(ELIST,init,0);
					a = new texpr(VALUE,tt,a);
					a->e2 = nn;
					return a->typ(tbl);
				}
			}
			return init->typ(tbl);
		}
		else if (refd==0) {	// bitwise copy, check for dtor & operator=
			Pclass cl = Pclass(c1->tp);
			if (cl->itor==0) {
				if (cl->bit_ass == 0)
					error('w',"bitwise copy: %s has a memberW operator=()",cl->string);
				else if (cl->has_dtor() && cl->has_oper(ASSIGN))
					error('w',"bitwise copy: %s has assignment and destructor but not %s(%s&)",cl->string,cl->string,cl->string);
			}
		}
//error('d',"class_init%n: init %d %d:%t",nn,init->tp,init->tp->base,init->tp);
		return init;
	}

	if (tt->check(init->tp,ASSIGN) && refd==0) {
		error("badIrT:%t (%tX)",init->tp,tt);
		init->tp = any_type;
	}
	return init;
}

int char_to_int(char* s)
/*	assume s points to a string:
		'c'
	or	'\c'
	or	'\0'
	or	'\ddd'
	or multi-character versions of the above
	(hex constants have been converted to octal by the parser)
*/
{
	register int i = 0;
	register char c, d, e;

	switch (*s) {
	default:
		error('i',"char constant store corrupted");
	case '`':
		error('s',"bcd constant");
		return 0;
	case '\'':
		break;
	}

	for(;;)			/* also handle multi-character constants */
	switch (c = *++s) {
	case '\'':
		return i;
	case '\\':			/* special character */
		switch (c = *++s) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7':	/* octal representation */
			c -= '0';
			switch (d = *++s) {		/* try for 2 */
				
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7':
				d -= '0';
				switch (e = *++s) {	/* try for 3 */
					
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7':
					c = c*64+d*8+e-'0';
					break;
				default:
					c = c*8+d;
					s--;
				}
				break;
			default:
				s--;
			}
			break;
		case 'b':
			c = '\b';
			break;
		case 'f':
			c = '\f';
			break;
		case 'n':
			c = '\n';
			break;
		case 'r':
			c = '\r';
			break;
		case 't':
			c = '\t';
			break;
		case '\\':
			c = '\\';
			break;
		case '\'':
			c = '\'';
			break;
		}
		/* no break */
	default:
		if (i) i <<= BI_IN_BYTE;
		i += c;
	}
}

const A10 = 'A'-10;
const a10 = 'a'-10;

int str_to_int(register const char* p)
/*
	read decimal, octal, or hexadecimal integer
*/
{
	register c;
	register i = 0;

	if ((c=*p++) == '0') {
		switch (c = *p++) {
		case 0:
			return 0;

		case 'l':
		case 'L':	/* long zero */
			return 0;

		case 'x':
		case 'X':	/* hexadecimal */
			while (c=*p++)
				switch (c) {
				case 'l':
				case 'L':
					return i;
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					i = i*16 + c-A10;
					break;
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					i = i*16 + c-a10;
					break;
				default:
					i = i*16 + c-'0';
				}
			return i;

		default:	/* octal */
			do 
				switch (c) {
				case 'l':
				case 'L':
					return i;
				default:
					i = i*8 + c-'0';
				}
			while (c=*p++);
			return i;
		}
	}	
				/* decimal */
	i = c-'0';
	while (c=*p++)
		switch (c) {
		case 'l':
		case 'L':
			return i;
		default:
			i = i*10 + c-'0';
		}
	return i;
	
		
}

char* Neval;
bit binary_val;

int expr::eval()
{
	if (Neval) return 1;
//error('d',"eval %k",base);
	switch (base) {
	case ZERO:	return 0;
	case IVAL:	return i1;
	case ICON:	return str_to_int(string);
	case CCON:	return char_to_int(string);
	case FCON:	Neval = "float in constant expression"; return 1;
	case STRING:	Neval = "string in constant expression"; return 1;
	case EOBJ:	return Pname(this)->n_val;
	case SIZEOF:	return tp2->tsizeof();
	case NAME:
	{	Pname n = (Pname)this; 
		if (n->n_evaluated && n->n_scope != ARG) return n->n_val;
		if ( n->n_initializer
			&& n->n_scope != ARG
			&& n->n_initializer->base == IVAL
			&& n->n_initializer->i1 == n->n_val) return n->n_val;
		if (binary_val && strcmp(string,"_result")==0) return 8888;
		Neval = "cannot evaluate constant";
		return 1;
	}
	case ICALL:
		if (e1) {
			il->i_next = curr_icall;
			curr_icall = il;
			int i = e1->eval();
			curr_icall = il->i_next;
			return i;
		}
		Neval = "void inlineF";
		return 1;
	case ANAME:
	{	Pname n = (Pname)this;
		int argno = n->n_val;
		Pin il;
		for (il=curr_icall; il; il=il->i_next)
			if (il->i_table == n->n_table) goto aok;
		goto bok;
	aok:
		if (il->local[argno]) {
	bok:
			Neval = "inlineF call too complicated for constant expression";
			return 1;
		}
		Pexpr aa = il->arg[argno];
		return aa->eval();
	}
	case CAST:
	{	int i = e1->eval();
		Ptype tt = tp2;
	strip:
		switch (tt->base) {
		case TYPE:
			tt = Pbase(tt)->b_name->tp;
			goto strip;
		case LONG:
			error('w',"cast to long in constantE (ignored)");
			break;
		case INT:
		case CHAR:
		case SHORT:
			if (Pbase(tp2)->b_unsigned && i<0)
				Neval = "cast to unsigned in constant expression";
			else {
				int diff = int_type->tsizeof()-tp2->tsizeof();
				if (diff) {	// narrowing may affect the value
					int bits = diff*BI_IN_BYTE;
					int div = 256; // 2**8
					if (BI_IN_BYTE!=8) {
						error('i',"expr::eval() assumes 8 bit bytes please re-write it");
					}
					while (--diff) div *= 256;
					i = (i << bits) / div;
				}
			}
		}
		return i;
	}
	case UMINUS:
	case UPLUS:
	case NOT:
	case COMPL:
	case PLUS:
	case MINUS:
	case MUL:
	case LS:
	case RS:
	case NE:
	case LT:
	case LE:
	case GT:
	case GE:
	case AND:
	case OR:
	case ER:
	case DIV:
	case MOD:
	case QUEST:
	case EQ:
	case ANDAND:
	case OROR:
		break;
	case G_ADDROF:
	case ADDROF:
		if (binary_val) {	// beware of &*(T*)0
			switch (e2->base) {
			case NAME:
			case DOT:
			case REF:	return 9999;
			}
		}
	default:
		Neval = "bad operator in constant expression";
		return 1;
	}

	int i1 = (e1) ? e1->eval() : 0;
	int i2 = (e2) ? e2->eval() : 0;

	switch (base) {
	case UMINUS:	return -i2;
	case UPLUS:	return i2;
	case NOT:	return !i2;
	case COMPL:	return ~i2;
	case CAST:	return i1;
	case PLUS:	return i1+i2;
	case MINUS:	return i1-i2;
	case MUL:	return i1*i2;
	case LS:	return i1<<i2;
	case RS:	return i1>>i2;
	case NE:	return i1!=i2;
	case EQ:	return i1==i2;
	case LT:	return i1<i2;
	case LE:	return i1<=i2;
	case GT:	return i1>i2;
	case GE:	return i1>=i2;
	case AND:	return i1&i2;
 	case ANDAND:	return i1&&i2;
	case OR:	return i1|i2;
	case OROR:	return i1||i2;
	case ER:	return i1^i2;
	case MOD:	return (i2==0) ? 1 : i1%i2;
	case QUEST:	return (cond->eval()) ? i1 : i2;
	case DIV:	if (i2==0) {
				Neval = "divide by zero";
				error('w',"divide by zero");
				return 1;
			}
			return i1/i2;
	}
}

bit classdef::baseof(Pname f)
/*
	is ``this'' class a public base class of "f"'s class
	or its immediate base class
*/
{
	Ptable ctbl = f->n_table;
	Pname b = ctbl->t_name;
	// f is a member of class b or a class derived from ``b''
	for (;;) {
		if (b == 0) return 0;
		Pclass cl = Pclass(b->tp);
		if (cl == 0) return 0;
		if (cl == this) return 1;
	//	if (cl->pubbase==0 && cl!=this->clbase->tp) return 0;
		if (cl->pubbase == 0)	// immediate base class ?
			return cl->clbase && cl->clbase->tp==this;
		b = cl->clbase;
	}
}

bit classdef::baseof(Pclass cl)
/*
	is ``this'' class a public base class of "cl"
*/
{
	for (;;) {
		if (cl == 0) return 0;
		if (cl == this) return 1;
		if (cl->pubbase==0 && clbase && cl!=clbase->tp) return 0;
	//	if (cl->pubbase == 0) 	// immediate base class ?
	//		if (clbase && cl==clbase->tp) return 1;
		Pname b = cl->clbase;
		if (b == 0) return 0;
		cl = Pclass(b->tp);
	}
}

bit classdef::has_friend(Pname f)
/*
	does this class have function "f" as its friend?
*/
{
	if (f == 0) return 0;
	Ptable ctbl = f->n_table;
	for (Plist l=friend_list; l; l=l->l) {
		Pname fr = l->f;
		switch (fr->tp->base) {
		case CLASS:
			if (Pclass(fr->tp)->memtbl == ctbl) return 1;
			break;
		case COBJ:
			if (Pbase(fr->tp)->b_table == ctbl) return 1;
			break;
		case FCT:
			if (fr == f) return 1;
			break;
		case OVERLOAD:
			l->f = fr = Pgen(fr->tp)->fct_list->f;	// first fct
			if (fr == f) return 1;
			break;
		default:
			error('i',"bad friend %k",fr->tp->base);
		}
	}
	return 0;
}

Pexpr pt(Pfct ef, Pexpr e, Ptable tbl)
/*
	a kludge: initialize/assign-to pointer to function
*/
{
	Pfct f;
	Pname n = 0;
//error('d',"pt %k %d",e->base,e->base);
	switch (e->base) {
	case NAME:
		f = (Pfct)e->tp;
		n = Pname(e);
		switch (f->base) {
		case FCT:
		case OVERLOAD:
			e = new expr(G_ADDROF,0,e);
			e->tp = f;
		}
		goto ad;

	case DOT:
	case REF:
		f = (Pfct)e->mem->tp;
		switch (f->base) {
		case FCT:
		case OVERLOAD:
			n = Pname(e->mem);
			e = new expr(G_ADDROF,0,e);
			e = e->typ(tbl);
		}
		goto ad;

	case ADDROF:
	case G_ADDROF:
		f = (Pfct)e->e2->tp;
	ad:
		if (f->base == OVERLOAD) {
			Pgen g = (Pgen)f;
			n = g->find(ef,0);
			if (n == 0) error("cannot deduceT for &overloaded %s()",g->string);
			e->e2 = n;
			e->tp = n->tp;
		}
		if (n) n->lval(ADDROF);
	}
	return e;
}

Pexpr ptr_init(Pptr p, Pexpr init, Ptable tbl)
/*
	check for silly initializers

	char* p = 0L;	 ??	fudge to allow redundant and incorrect `L'
	char* p = 2 - 2; ??	worse
*/
{
	Ptype it = init->tp;
itl:
//error('d',"itl %t",it);
	switch (it->base) {
	case TYPE:
		it = Pbase(it)->b_name->tp; goto itl;
	case ZTYPE:
		if (init == zero) break;
	case INT:
	case CHAR:
	case SHORT:
	{	Neval = 0;
		int i = init->eval();
		if (Neval)
			error("badPIr: %s",Neval);
		else if (i)
			error("badPIr value %d",i);
		else {
			DEL(init);
			init = zero;
		}
		break;
	}		
	case LONG:
		if (init->base==ICON
		&& init->string[0]=='0'
		&& (init->string[1]=='L' || init->string[1]=='l')) {
			DEL(init);
			init = zero;
		}
	}

	return (p->typ->base == FCT) ? pt(Pfct(p->typ),init,tbl) : init;
}

Pexpr expr::try_to_overload(Ptable tbl)
{
	TOK bb = (base==DEREF && e2==0) ? MUL : base;

	Pname n1 = 0;
	Ptype t1 = 0;
	if (e1) {
		t1 = e1->tp;
		Ptype tx = t1;
		while (tx->base == TYPE) tx = Pbase(tx)->b_name->tp;
		n1 = tx->is_cl_obj();
	}

	Pname n2 = 0;
	Ptype t2 = 0;
	if (e2) {
		t2 = e2->tp;
		Ptype tx = t2;
		while (tx->base == TYPE) tx = Pbase(tx)->b_name->tp;
		n2 = tx->is_cl_obj();
	}

	if (n1==0 && n2==0) return 0;

	/* first try for non-member function:	op(e1,e2) or op(e2) or op(e1) */
	Pexpr oe2 = e2;
	Pexpr ee2 = (e2 && e2->base!=ELIST) ? e2 = new expr(ELIST,e2,0) : 0;
	Pexpr ee1 = (e1) ? new expr(ELIST,e1,e2) : ee2;
	char* obb = oper_name(bb);
	Pname gname = gtbl->look(obb,0);
	int go = gname ? over_call(gname,ee1) : 0;
	int nc = Nover_coerce;	// first look at member functions						// then if necessary check for ambiguities
	if (go) gname = Nover;
	int ns  = Nstd;

	if (n1) {				/* look for member of n1 */	
		Ptable ctbl = Pclass(n1->tp)->memtbl;
		Pname mname = ctbl->look(obb,0);
		if (mname == 0) goto glob;
		switch (mname->n_scope) {
		default:	goto glob;
		case 0:
		case PUBLIC:	break;		/* try e1.op(?) */
		}

		int mo = over_call(mname,e2);

		switch (mo) {
		case 0:	
			if (go == 2) goto glob;
			if (1 < Nover_coerce) goto am1;
			goto glob;
		case 1:	if (go == 2) goto glob;
			if (go == 1) {
			am1:
				error("ambiguous operandTs%n and%t for%k",n1,t2,bb);
				tp = any_type;
				return this;
			}
			else if (Pclass(n1->tp)->conv) {
				switch (bb) {
				case ASSIGN:
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
					// don't coerce left hand side of assignment
					break;
				default:
					if (Pfct(Pclass(n1->tp)->conv->tp)->returns->is_cl_obj()) break;
					error('w',"overloaded%k may be ambiguous.FWT%tused",bb,Nover->tp);
				}
			}
			break;
		case 2:
			if (go == 2 && ns <= Nstd) 
				error("%k defined both as%n and%n",bb,gname,Nover);
		}

		if (bb==ASSIGN && mname->n_table!=ctbl) {	/* inherited = */
			error("assignment not defined for class%n",n1);
			tp = any_type;
			return this;
		}

		base = G_CALL;			/* e1.op(e2) or e1.op() */
		e1 = new ref(DOT,e1,Nover);
		if (ee1) delete ee1;
		return typ(tbl);
	}
	
	if (n2 && e1==0) {			/* look for unary operator */
		Ptable ctbl = Pclass(n2->tp)->memtbl;
		Pname mname = ctbl->look(obb,0);
		if (mname == 0) goto glob;
		switch (mname->n_scope) {
		default:	goto glob;
		case 0:
		case PUBLIC:	break;		// try e2.op()
		}
		
		int mo = over_call(mname,0);

		switch (mo) {
		case 0:		
			if (1 < Nover_coerce) goto am2;
			goto glob;
		case 1:	if (go == 2) goto glob;
			if (go == 1) {
			am2:
				error("ambiguous operandT%n for%k",n2,bb);
				tp = any_type;
				return this;
			}
			break;
		case 2:
			if (go == 2 && ns <= Nstd) 
				error("%k defined both as%n and%n",bb,gname,Nover);
		}

		base = G_CALL;			/* e2.op() */
		e1 = new ref(DOT,oe2,Nover);
		e2 = 0;
		if (ee2) delete ee2;
		if (ee1 && ee1!=ee2) delete ee1;
		return typ(tbl);
	}
	
glob:
	if (1 < nc) {
		error("ambiguous operandTs%t and%t for%k",t1,t2,bb);
		tp = any_type;
		return this;
	}
	if (go) {
		if (go == 1) {	// conversion necessary => binary
			// very sloppy test:
			if ( (n1
				&& Pclass(n1->tp)->conv
				&& Pfct(Pclass(n1->tp)->conv->tp)->returns->is_cl_obj()==0)
			||   (n2
				&& Pclass(n2->tp)->conv
				&& Pfct(Pclass(n2->tp)->conv->tp)->returns->is_cl_obj()==0) )
				error('w',"overloaded%k may be ambiguous.FWT%tused",bb,gname->tp);
		}
		base = G_CALL;	// op(e1,e2) or op(e1) or op(e2)
		e1 = gname;
		e2 = ee1;
		return typ(tbl);
	}

	if (ee2) delete ee2;
	if (ee1 && ee1!=ee2) delete ee1;
	e2 = oe2;

	switch(bb) {
	case ASSIGN:
	case ADDROF:
		break;
	case DEREF:
		if (e2) {
			base = NOT;	// fudge to cope with ! as subscripting op
			Pexpr x = try_to_overload(tbl);
			if (x) return x;
			base = DEREF;
		}
	case CALL:
		if (n1 == 0) break;
	default:	/* look for conversions to basic types */
	{	int found = 0;
		if (n1) {
			int val = 0;
			Pclass cl = (Pclass)n1->tp;
			for (Pname on = cl->conv; on; on=on->n_list) {
				Pfct f = (Pfct)on->tp;
				if (bb==ANDAND || bb==OROR) {
					e1 = check_cond(e1,bb,tbl);
					return 0;
				}
				if (n2 
				|| (t2 && f->returns->check(t2,ASSIGN)==0)
				|| (t2 && t2->check(f->returns,ASSIGN)==0)) {
					Ncoerce = on;
					val++;
				}
			}
			switch (val) {
			case 0:
				if (base == NOT) return 0;
				break;
			case 1:
			{	Pref r = new ref(DOT,e1,Ncoerce);
				Pexpr rr = r->typ(tbl);
				e1 = new expr(G_CALL,rr,0);
				found = 1;
				break;
			}
			default:
				error('s',"ambiguous coercion of%n to basicT",n1);
			}
		}
		if (n2) {
			int val = 0;
			Pclass cl = (Pclass)n2->tp;
			for ( Pname on = cl->conv; on; on=on->n_list) {
				Pfct f = (Pfct)on->tp;
				if (bb==ANDAND || bb==OROR || bb==NOT) {
					e2 = check_cond(e2,bb,tbl);
					return 0;
				}
				if (n1 
				|| (t1 && f->returns->check(t1,ASSIGN)==0)
				|| (t1 && t1->check(f->returns,ASSIGN)==0)) {
					Ncoerce = on;
					val++;
				}
			}
			switch (val) {
			case 0:
				break;
			case 1:
			{	Pref r = new ref(DOT,e2,Ncoerce);
				Pexpr rr = r->typ(tbl);
				e2 = new expr(G_CALL,rr,0);
				found++;
				break;
			}
			default:
				error('s',"ambiguous coercion of%n to basicT",n2);
			}
		}
		if (found) return typ(tbl);
		if (t1 && t2)
			error("bad operandTs%t%t for%k",t1,t2,bb);
		else
			error("bad operandT%t for%k",t1?t1:t2,bb);
		tp = any_type;
		return this;
	}
	}
	return 0;
}

extern int bound;	// fudge for bound pointers to functions

Pexpr expr::docast(Ptable tbl)
{	
	Ptype t;
	Ptype tt = t = tp2;
	tt->dcl(tbl);
zaq:				/* is the cast legal? */
//error('d',"tt %d %d",tt,tt?tt->base:0);
	switch (tt->base) {
	case TYPE:
		tt = Pbase(tt)->b_name->tp;	goto zaq;
	case RPTR:	// necessary?
	case PTR:
		if (Pptr(tt)->rdo) error("*const in cast");
	case VEC:
		tt = Pptr(tt)->typ;
		goto zaq;
	case FCT:
	//	tt = Pfct(tt)->returns;
	//	goto zaq;
		break;	// const is legal in function return types
	default:	
		if (Pbase(tt)->b_const) error("const in cast");
	}

	/* now check cast against value, INCOMPLETE */

//error('d',"cast e1 %d %k",e1,e1->base);
	tt = t;

	if (e1 == dummy) {
		error("E missing for cast");
		tp = any_type;
		return this;
	}

	int pmf = 0;
	Pexpr ee = e1;
	switch (ee->base) {
	case ADDROF:
		ee = ee->e2;
		switch (ee->base) {
		case NAME:	goto nm;
		case REF:	goto rf;
		}
		break;

	case NAME:
	nm:
		if (Pname(ee)->n_qualifier) pmf = 1;
		break;
		
	case REF:
	rf:
		if (ee->e1->base == THIS) bound = 1;
		break;
	}
	e1 = e1->typ(tbl);
//error('d',"pmf %d bound %d",pmf,bound);
	int b = bound;	// distinguish between explicit and implicit THIS
	bound = 0;
	pmf = pmf && e1->base==CAST;

	Ptype etp = e1->tp;
	while (etp->base == TYPE) etp = Pbase(etp)->b_name->tp;
		
	switch (etp->base) {
	case COBJ:
	{	Pexpr x = try_to_coerce(tt,e1,"cast",tbl);
		if (x) return x;
/*
			int i = can_coerce(tt,etp);
//error('d',"cast%t->%t -- %d%n",tt,etp,i,Ncoerce);
			if (i==1 && Ncoerce) {
				Pname cn = Pbase(etp)->b_name;
				Pclass cl = Pclass(cn->tp);
				Pref r = new ref(DOT,e1,Ncoerce);
				Pexpr rr = r->typ(tbl);
				Pexpr c = new expr(G_CALL,rr,0);
				c->fct_name = Ncoerce;
				c->tp = tt;
				*this = *Pexpr(c);
				delete c;	
				return this;
			}
*/
		break;
	}
	case VOID:
		if (tt->base == VOID) {
			tp = t;
			return this;
		}
		error("cast of void value");
	case ANY:
		tp = any_type;
		return this;
	}
	
legloop:
//error('d',"legloop %t",tt);
	switch (tt->base) {
	case TYPE:	
		tt = Pbase(tt)->b_name->tp; goto legloop;
	case PTR:
		switch (etp->base) {
		case COBJ:
			error("cannot castCO toP");
			break;
		case FCT:
			e1 = new expr(G_ADDROF,0,e1);
			bound = b;
			e1 = e1->typ(tbl);
			bound = 0;
			if (e1->base == CAST)
				pmf = 1;
			else
				break;
		case PTR:
			if (pmf) {
			zaqq:
				switch (tt->base) {
				case TYPE:
					tt = Pbase(tt)->b_name->tp;							goto zaqq;
				case PTR:
					if (Pptr(tt)->memof) break;
				default:
					error("%t cast to%t (%t is not aP toM)",e1->tp,tp2,tp2);
				}
			}
		}
		break;

	case RPTR:		// (x&)e: pretend e is an x
//error('d',"rptr%t(%t): e1 %d %k",t,etp,e1->base,e1->base);
		if ((e1->base==G_CM
			|| e1->base==CALL
			|| e1->base==G_CALL
			|| e1->lval(0))
		&& Pptr(tt)->typ->tsizeof()<=etp->tsizeof()) {
			e1 = e1->address();	// *(x*)&e
			tp = t;
			return contents();
		}
		else	
			error("cannot cast%t to%t",etp,t);
		break;

	case COBJ:
//error('d',"%n ctor %d",cn,ctor);
		base = VALUE;	// (x)e => x(e): construct an x from e
		e1 = new expr(ELIST,e1,0);
		return typ(tbl);

	case CHAR:
	case INT:
	case SHORT:
	case LONG:
	case FLOAT:
	case DOUBLE:
		switch (etp->base) {
		case COBJ:
			error("cannot castCO to%k",tt->base);
			break;
		}	
		break;
		
	}
	tp = t;
	return this;
}

Pexpr expr::dovalue(Ptable tbl)
{
	Ptype tt = tp2;
	Pclass cl;
	Pname cn;
//error('d',"value %d %d (%d %k)",tt,tt?tt->base:0,e1,e1?e1->base:0);
	
	tt->dcl(tbl);
vv:
	switch (tt->base) {
	case TYPE:
		tt = Pbase(tt)->b_name->tp;
		goto vv;

	case EOBJ:
	default:
		if (e1 == 0) {
			error("value missing in conversion to%t",tt);
			tp = any_type;
			return this;
		}
		base = CAST;
		e1 = e1->e1;	// strip ELIST
		return typ(tbl);

	case CLASS:
		cl = Pclass(tt);
		break;

	case COBJ:
		cn = Pbase(tt)->b_name;
		cl = Pclass(cn->tp);
	}

	if (e1 && e1->e2==0) {		// single argument
		e1->e1 = e1->e1->typ(tbl);
		if (tt->base==COBJ) {
			Pexpr x = try_to_coerce(tt,e1->e1,"type conversion",tbl);
			if (x) return x;
		}
		Pname acn = e1->e1->tp->is_cl_obj();
//error('d',"acn %n acn->tp %d cl %d",acn,acn->tp,cl);		
		if (acn && acn->tp==cl && cl->has_itor()==0) {
			if (e2) {	// x(x_obj) => e2=x_obj
				base = ASSIGN;
				Pexpr ee = e1->e1;
				e1 = e2;
				e2 = ee;
				tp = tp2;
				return this;
			}
			return e1->e1;	// x(x_obj) => x_obj
		}
	}

	/* x(a) => obj.ctor(a); where e1==obj */
	Pname ctor = cl->has_ctor();
//error('d',"value %d.%d",e2,ctor);
	if (ctor == 0) {
		error("cannot make a%n",cn);
		base = SM;
		e1 = dummy;
		e2 = 0;
		return this;
	}

	Pexpr ee;
	int tv = 0;
	if (e2 == 0) {		// x(a) => x temp; (temp.x(a),temp)
		Pname n = make_tmp('V',tp2,tbl);
		n->assign();
		if (tbl == gtbl) n->dcl_print(0);
		e2 = n;
		ee = new expr(G_CM,this,n);
		tv = 1;
	}
	else
		ee = this;
//error('d',"ee %k %d",ee->base,ee->base);
	Pexpr a = e1;
	base = G_CALL;
	e1 = new ref(DOT,e2,ctor);
	e2 = a;
	ee = ee->typ(tbl);
//error('d',"tv %d %t",tv,ee->tp);
	if (tv == 0) {	// deref value returned by constructor
		ee = new expr(DEREF,ee,0);
		ee->tp = ee->e1->tp;
	}
	return ee;
}

Pexpr expr::donew(Ptable tbl)
{
	Ptype tt = tp2;
	Ptype tx = tt;
	bit v = 0;
	bit old = new_type;
	new_type = 1;
	tt->dcl(tbl);
	new_type = old;
	if (e1) e1 = e1->typ(tbl);
ll:
	switch (tt->base) {
	default:
		if (e1) {
			error("Ir for nonCO created using \"new\"");
			e1 = 0;
		}
		break;
	case VEC:
		v = 1;
		tt = Pvec(tt)->typ;
		goto ll;
	case TYPE:
		tt = Pbase(tt)->b_name->tp;
		goto ll;
	case COBJ:
	{	Pname cn = Pbase(tt)->b_name;
		Pclass cl = (Pclass)cn->tp;
			
		if ((cl->defined&(DEFINED|SIMPLIFIED)) == 0) {
			error("new%n;%n isU",cn,cn);
		}
		else {
			Pname ctor = cl->has_ctor();
			TOK su;
			if (ctor) {
				if (v) {
					Pname ic;
					if (e1)
						error('s',"Ir for vector ofCO created using \"new\"");
					else if ((ic = cl->has_ictor())==0)
						error("vector ofC%n that does not have aK taking noAs",cn);
					else if (Pfct(ic->tp)->nargs)
						error('s',"defaultAs forK for vector ofC%n",cn);
				}

				if (cc->cot != cl) 
					visible_check(ctor, ctor->string, this, 0);
				e1 = new call(ctor,e1);
				e1 = e1->typ(tbl);
				/*(void) e1->fct_call(tbl);*/
			}
			else if (su=cl->is_simple()) { 
				if (e1) error("new%nWIr",cn);
			}
			else {
				// error('s',"not simple and noK?");
			}
		}
	}
	}
//error('d',"v==%d",v);
	tp = (v) ? (Ptype)tx : (Ptype)new ptr(PTR,tx,0);
	return this;
}
