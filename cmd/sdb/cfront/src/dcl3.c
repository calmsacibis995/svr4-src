/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/dcl3.c	1.1"
/*ident	"@(#)cfront:src/dcl3.c	1.8.1.4" */
/**************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.

dcl3.c:
	Etc routines used by ::dcl functions

*****************************************************************************/


#include "cfront.h"
#include "size.h"

void make_res(Pfct f)
/*
	returns X where X(X&) has been declared
	add "_result" argument of type X*
*/
{
	Pname rv = new name("_result");
	rv->tp  = new ptr(PTR,f->returns);
	rv->n_scope = FCT;	// not a ``real'' argument
	rv->n_used = 1;
	rv->n_list = f->argtype;
	if (f->f_this) f->f_this->n_list = rv;
	f->f_result = rv;
}

void name::check_oper(Pname cn)
/*
	check declarations of operators, ctors, dtors
*/
{
	switch (n_oper) {
	case CALL:
		if (cn == 0) error("operator() must be aM");
		break;
	case DEREF:
		if (cn == 0) error("operator[] must be aM");
		break;
	case 0:
	case TNAME:	/* may be a constructor */
		if (cn && strcmp(cn->string,string)==0) {
			if (tp->base == FCT) {
				Pfct f = (Pfct)tp;
				if (f->returns!=defa_type && fct_void==0)
					error("%s::%s()W returnT",string,string);
				f->returns = void_type;
				string = "_ctor";
				n_oper = CTOR;
			}
			else
				error('s',"struct%cnM%n",cn,cn);
		}
		else
			n_oper = 0;
		break;
	case DTOR:	/* must be a destructor */
		if (cn == 0) {
			n_oper = 0;
			error("destructor ~%s() not inC",string);
		}
		else if (strcmp(cn->string,string) == 0) {
		dto:
			Pfct f = (Pfct)tp;
			string = "_dtor";
			if (tp->base != FCT) {
				error("%s::~%s notF",cn->string,cn->string);
				tp = new fct(void_type,0,1);
			}
			else if (f->returns!=defa_type && f->returns!=void_type && fct_void==0)
				error("%s::~%s()W returnT",cn->string,cn->string);
			if (f->argtype) {
				if (fct_void==0) error("%s::~%s()WAs",cn->string,cn->string);
				f->nargs = 0;
				f->nargs_known = 1;
				f->argtype = 0;
			}
			f->returns = void_type;
		}
		else {
			if (strcmp(string,"_dtor") == 0) goto dto;
			error("~%s in %s",string,cn->string);
			n_oper = 0;
		}
		break;
	case TYPE:
		if (cn == 0) {
			error("operator%t() not aM",(Ptype)n_initializer);
			n_oper = 0;
			n_initializer = 0;
		}
		else {
			Pfct f = (Pfct)tp;
			Ptype tx = (Ptype)n_initializer;
/*error('d',"operator%t()",tx);*/
			n_initializer = 0;
			if (f->base != FCT) error("badT for%n::operator%t()",cn,tx);
			if (f->returns != defa_type) {
				if (f->returns->check(tx,0)) error("bad resultT for%n::operator%t()",cn,tx);
				DEL(f->returns);
			}
			if (f->argtype) {
				error("%n::operator%t()WAs",cn,tx);
				f->argtype = 0;
			}
			f->returns = tx;
			Pname nx = tx->is_cl_obj();
			if (nx && can_coerce(tx,cn->tp)) error("both %n::%n(%n) and %n::operator%t()",cn,cn,nx,tx);
			char buf[256];
			char* bb = tx->signature(buf);
			int l2 = bb-buf;
			if (*bb!=0) error('d',"impossible");
			if (255<l2) error('i',"N::check_oper():N buffer overflow");
			char* p = new char[l2+3];
			p[0] = '_';
			p[1] = 'O';
			strcpy(p+2,buf);
			string = p;
		}
		break;
	}
}


int inline_restr;	/* report use of constructs that the inline expanded
			   cannot handle here
			*/

void fct::dcl(Pname n)
{
	int nmem = TBLSIZE;
	Pname a;
	Pname ll;
	Ptable ftbl;

	Pptr cct = 0;
	int const_old = const_save;

	int bit_old = bit_offset;
	int byte_old = byte_offset;
	int max_old = max_align;
	int stack_old = stack_size;

	if (base != FCT) error('i',"F::dcl(%d)",base);
	if (body == 0) error('i',"F::dcl(body=%d)",body);
	if (n==0 || n->base!=NAME) error('i',"F::dcl(N=%d %d)",n,(n)?n->base:0);

	if (body->memtbl == 0) body->memtbl = new table(nmem+3,n->n_table,0);
	ftbl = body->memtbl;
	body->own_tbl = 1;
	ftbl->real_block = body;

	max_align = 0;//AL_FRAME;
	stack_size = byte_offset = 0;//SZ_BOTTOM;
	bit_offset = 0;

	cc->stack();
	cc->nof = n;
	cc->ftbl = ftbl;

	switch (n->n_scope) {
	case 0:
	case PUBLIC:
		cc->not = n->n_table->t_name;
		cc->cot = (Pclass)cc->not->tp;
		cc->tot = cc->cot->this_type;
		if (f_this==0 || cc->tot==0) error('i',"F::dcl(%n): f_this=%d cc->tot=%d",n,f_this,cc->tot);
		f_this->n_table = ftbl;		// fake for inline printout
		cc->c_this = f_this;
	}

	if (f_result == 0) {
		// protect against: class x; x f(); class x { x(x&); ....
		Pname rcln = returns->is_cl_obj();
		if (rcln && (/*f_virtual || */Pclass(rcln->tp)->has_itor())) make_res(this);
	}
	if (f_result) f_result->n_table = ftbl;		// fake for inline printout

	Pname ax;
	for (a=argtype, ll=0; a; a=ax) {
		ax = a->n_list;
		Pname nn = a->dcl(ftbl,ARG);
		nn->n_assigned_to = nn->n_used = nn->n_addr_taken = 0;
		nn->n_list = 0;
		switch (a->tp->base) {
		case CLASS:
		case ENUM:	/* unlink types declared in arg list */
			a->n_list = dcl_list;
			dcl_list = a;
			break;
		default:
			if (ll)
				ll->n_list = nn;
			else
				argtype = nn;
			ll = nn;
			delete a;
		}
	}

		// link in f_result and f_this:
	if (f_result) f_result->n_list = argtype;
	if (f_this) f_this->n_list = f_result ? f_result : argtype;
//error('d',"f %p f_this (%p->%p) f_result %p argtype %p",this,f_this,f_this->n_list,f_result,argtype);
	/*
		handle initializers for base class and member classes
		this->f_init == list of names of classes to be initialized
		no name		=> base class
				=> constructor call in f_init->n_initializer
		name "m"	=> member object
				=> constructor call in m->n_initializer
	*/
	if (n->n_oper != CTOR) {
		if (f_init) error(0,"unexpectedAL: not aK");
	}
	else {
//error('d',"%n:f_init %d %d",n,f_init,f_init?f_init->base:0);
		if (f_init) {				// explicit initializers
			Pname  bn  = cc->cot->clbase;
			Ptable tbl = cc->cot->memtbl;
			Pexpr binit = 0;
			Pname nx;
			const_save = 1;
			for ( Pname nn=f_init; nn; nn=nx) {
				nx = nn->n_list;
				Pexpr i = nn->n_initializer;
				char* s = nn->string;

				if (s) {
					Pname m = tbl->look(s,0);
					if (m) {
						// class member initializer
						if (m->n_table == tbl)
							nn->n_initializer = mem_init(m,i,ftbl);
						else {
							error("%n not inC%n",m,n);
							nn->n_initializer = 0;
						}
					}
					else if (m = ktbl->look(s,0)) {
						// named base class initializer
						binit = base_init(bn,i,ftbl);
						nn->n_initializer = 0;
					}
					else {
						error("%n not inC%n",m,n);
						nn->n_initializer = 0;
					}
				}
				else if (bn) {
					// unnamed base class initializer
					binit = base_init(bn,i,ftbl);
					nn->n_initializer = 0;
				}
				else
					error( "unexpectedAL: noBC" );
			} // for
			const_save = const_old;
			b_init = binit;
		}

		if (b_init == 0) { // try default initialization of base class
			Pname  bn  = cc->cot->clbase;
			if (bn && Pclass(bn->tp)->has_ctor())
				b_init = base_init(bn,0,ftbl);
		}
	}

	PERM(returns);
	const_save = f_inline?1:0;
	inline_restr = 0;
	body->dcl(ftbl);
	if( f_inline && inline_restr && returns->base!=VOID) {
		f_inline = 0;
		char* s = (inline_restr & 8) ? "loop"
				: (inline_restr & 4) ? "switch"
					: (inline_restr & 2) ? "goto"
						: (inline_restr & 1) ? "label"
							: "" ;
		error('w', "\"inline\" ignored, %n contains %s",n,s);
	}
	const_save = const_old;

	if (f_inline) isf_list = new name_list(n,isf_list);

	defined |= DEFINED;

//	frame_size = stack_size + SZ_TOP;
//	frame_size = ((frame_size-1)/AL_FRAME)*AL_FRAME+AL_FRAME;
	bit_offset = bit_old;
	byte_offset = byte_old;
	max_align = max_old;
	stack_size = stack_old;

	cc->unstack();
}


Pexpr fct::base_init(Pname bn, Pexpr i, Ptable ftbl)
/*
	have base class bn and expr list i
	return "( *(base*)this ) . ctor( i )"
	ctor call generated in expr.typ()
*/
{
	Pclass bcl = (Pclass)bn->tp;
	Pname bnw = bcl->has_ctor();
//error('d',"base_init%n %d i %d %d bcl %d %d",bn,bnw,i,i?i->base:0,bcl,bcl?bcl->base:0);
	if (bnw) {
		Ptype t = bnw->tp;
		Pfct f  = Pfct((t->base==FCT) ? t : Pgen(t)->fct_list->f->tp);
		Ptype ty = f->f_this->tp;		// this
		Pexpr th = new texpr(CAST,ty,f_this);	// (base*)this
		Pexpr v  = new texpr(VALUE,bcl,i);	// ?.base(i)
		v->e2    = new expr(DEREF,th,0);	// (*(base*)this).base(i)
		v = v->typ(ftbl);			// *base(&*(base*)this,i)
//error('d',"v %d %k",v,v->base);
		switch (v->base) {
		case DEREF:
		{	Pexpr vv = v;
			v = v->e1;			// base(&*(base*)this,i)
		//	delete vv;
			break;
		}
		case ASSIGN:		// degenerate base(base&): *(base*)this=i
			th = new texpr(CAST,ty,f_this);
			v = new expr(CM,v,th);	// (*(base*)this=i,(base*)this);
			v = v->typ(ftbl);
			break;
		default:
			error('i',"F::base_init: unexpected %k",v->base);
		}
		return v;
	} else
		error(0,"unexpectedAL: noBCK");
	return 0;
}


Pexpr fct::mem_init(Pname member, Pexpr i, Ptable ftbl)
/*
	return "member_ctor( m, i )"
*/
{
//error('d',"mem_init(%n) init %p tbl %p",member,i,ftbl);
	if (member->n_stclass == STATIC) error('s',"MIr for static%n",member);
	if (i) i = i->typ(ftbl);
	Pname cn = member->tp->is_cl_obj();	// first find the class name
	if (cn) {
		Pclass mcl = Pclass(cn->tp);	// then find the classdef
		Pname ctor = mcl->has_ctor();
		Pname icn;
//error('d',"cn%n ct %d it %d icn%n",cn,ctor,mcl->has_itor(),i->tp->is_cl_obj());
		if (mcl->has_itor()==0
		&& i
		&& (icn=i->tp->is_cl_obj())
		&& Pclass(icn->tp)==mcl) {	// bitwise copy
			Pref tn = new ref (REF,f_this,member);
			Pexpr init = new expr(ASSIGN,tn,i);
			return init->typ(ftbl);
		}
		else if (ctor) {	// generate: this->member.cn::cn(args)
			Pref tn = new ref(REF,f_this,member);
			Pref ct = new ref(DOT,tn,ctor);
			Pexpr c = new expr(G_CALL,ct,i);
			return c->typ(ftbl);
		}
		else {
			error("Ir forM%nW noK",member);
			return 0;
		}
	}

	if (cl_obj_vec) {
		error('s',"Ir forCM vectorWK");
		return 0;
	}

//error('d',"mem %n %t %k",member,member->tp,i->base);
	if (i->e2) error("Ir for%m not a simpleE",member);	// i is an ELIST
	i = i->e1;
	Pref tn = new ref (REF,f_this,member);

	if (member->tp->tconst()) {
		TOK t = member->tp->set_const(0);
		switch (t) {
		case ANY:
		case VEC:
		case RPTR:
			error("MIr for%kM%n",member);
			return 0;
		}
		i = new expr(ASSIGN,tn,i);
		i = i->typ(ftbl);
		member->tp->set_const(1);
		return i;
	}
	
	if (member->tp->is_ref()) {
		i = ref_init(Pptr(member->tp),i,ftbl);
		i = new expr(ASSIGN,tn,i);
		member->assign();	// cannot call typ: would cause dereference
		return i;
	}

	i = new expr(ASSIGN,tn,i);
	return i->typ(ftbl);	// typ performs the type check on the assignment
	
}

Pexpr replace_temp(Pexpr e, Pexpr n)
/*
	e is on the form
				f(&temp,arg) , temp
			or
				&temp->ctor(arg) , temp
			or
				x->f(&temp,arg) , temp
	change it to
				f(n,arg)
			or
				n->ctor(arg)
*/
{
	Pexpr c = e->e1;	// f(&temp,arg) or &temp->ctor(args)
	Pexpr ff = c->e1;
	Pexpr a = c->e2;	// maybe ELIST(&temp,arg)
	Pexpr tmp = e->e2;

//error('d',"suppress(%d %k) %n",tmp->base,tmp->base,tmp->base==NAME?tmp:0);
	if (tmp->base==DEREF) tmp = tmp->e1;
	if (tmp->base==CAST) tmp = tmp->e1;
	if (tmp->base==ADDROF || tmp->base==G_ADDROF) tmp = tmp->e2;
	if (tmp->base != NAME) error('i',"replace %k",tmp->base);
	tmp->tp = any_type;	// temporary not used: suppress it

//error('d',"replace_temp(%k %k) c %k ff %k",e->base,n->base,c->base,ff->base);
	switch (ff->base) {
	case REF:
		if (ff->e1->base==G_ADDROF && ff->e1->e2==tmp)
			a = ff;				// &tmp -> f()
		break;
	case DOT:
		if (ff->e1->base==NAME && ff->e1==tmp) {
			a = ff;				// tmp . f()
			a->base = REF;
		}
		break;
	}
	a->e1 = n;
	return c;
}

Pname classdef::has_ictor()
/*
	does this class have a constructor taking no arguments?
*/
{
	Pname c = has_ctor();
	Pfct f;
	Plist l;

	if (c == 0) return 0;

	f = (Pfct)c->tp;

	switch (f->base) {
	default:
		error('i',"%s: badK (%k)",string,c->tp->base);
	
	case FCT:
		switch (f->nargs) {
		case 0:		return c;
		default:	if (f->argtype->n_initializer) return c;
		}
		return 0;

	case OVERLOAD:
		for (l=Pgen(f)->fct_list; l; l=l->l) {
			Pname n = l->f;
			f = (Pfct)n->tp;
			switch (f->nargs) {
			case 0:		return n;
			default:	if (f->argtype->n_initializer) return n;
			}
		}
		return 0;
	}
}

gen::gen(char* s)
{
	char * p = new char[ strlen(s)+1 ];
	strcpy(p,s);
	string = p;
	base = OVERLOAD;
	fct_list = 0;
}

Pname gen::add(Pname n,int sig)
/*
	add "n" to the tail of "fct_list"
	(overloaded names are searched in declaration order)

	detect:	 	multiple identical declarations
			declaration after use
			multiple definitions
*/
{
	Pfct f = (Pfct)n->tp;
	Pname nx;

	if (f->base != FCT) error("%n: overloaded nonF",n);

	if ( fct_list && (nx=find(f,1)) )
		Nold = 1;
	else {
		char* s = string;

		if (fct_list || sig || n->n_oper) {
			char buf[256];
			char* bb = n->tp->signature(buf);
			int l2 = bb-buf;
			if (*bb!=0) error('d',"impossible sig");
			if (255 < l2) error('i',"gen::add():N buffer overflow");
			int l1 = strlen(s);
			char* p = new char[l1+l2+1];
			strcpy(p,s);
			strcpy(p+l1,buf);
			n->string = p;
		}
		else 
			n->string = s;

		nx = new name;
		*nx = *n;
		PERM(nx);
		Nold = 0;
		if (fct_list) {
			Plist gl;
			for (gl=fct_list; gl->l; gl=gl->l) ;
			gl->l = new name_list(nx,0); 
		}
		else
			fct_list = new name_list(nx,0);
		nx->n_list = 0;
	}
	return nx;
}

extern bit const_problem;

Pname gen::find(Pfct f, bit warn)
{	
	for (Plist gl=fct_list; gl; gl=gl->l) {
		Pname nx = gl->f;
		Pfct fx = (Pfct)nx->tp;
		Pname a, ax;
		int vp = 0;
		int cp = 0;
		int op = 0;
		int xp = 0;
		int ma = 0;

		if (fx->nargs_known!=f->nargs_known
		&& fx->nargs
		&& fx->nargs_known!=ELLIPSIS) continue;

		int acnt = (fx->nargs>f->nargs)?fx->nargs:f->nargs;
		for (ax=fx->argtype,a=f->argtype; a&&ax; ax=ax->n_list,a=a->n_list)
		{

			Ptype at = ax->tp;
			Ptype atp = a->tp;

			int atpc = 0;
			int rr = 0;
			acnt--;

				/*
				   warn against:
					overload f(X&), f(X);
				   and
					overload f(int), f(enum e);
				   and
					overload f(int), f(const);
				   and
					overload f(char), f(int);
				   and
					overload f(x&), f(const x&);
				*/

		aaa:
			switch (atp->base) {
			case TYPE:
				if (Pbase(atp)->b_const) atpc = 1;
				atp = Pbase(atp)->b_name->tp;
				goto aaa;
			case RPTR:
				if (warn && Pptr(atp)->typ->check(at,0)==0) op += 1;
				rr = 1;
			}

		atl:
			switch (at->base) {
			case TYPE:
				if (Pbase(at)->b_const != atpc) cp = 1;
				at = Pbase(at)->b_name->tp;
				goto atl;
			case RPTR:
				if (warn && Pptr(at)->typ->check(atp,0)==0) op += 1;
				rr = 1;
				goto defa;
			case FLOAT:
			case DOUBLE:
				if (warn) {
					if (atp->base!=at->base) {
						switch(atp->base) {
						case FLOAT:
						case DOUBLE:	op += 1;
						}
					}
					else xp += 1;
				}
				goto defa;
			case EOBJ:
				if (warn) {
					if  (atp->base!=at->base) {
						switch (atp->base) {
						case CHAR:
						case SHORT:
						case INT:	op += 1;
						}
					}
					else if (Pbase(atp)->b_name->tp!=Pbase(at)->b_name->tp)
						op += 1;
					else xp += 1;
				}
				goto defa;
			case CHAR:	
			case SHORT:
			case INT:
				if (warn) {
					if (atp->base!=at->base) {
						switch (atp->base) {
						case SHORT:
						case INT:
						case CHAR:
						case EOBJ:	op += 1;
						}
					}
					else xp += 1;
				}
				
			case LONG:
				if (at->check(atp,0)) {
					if (const_problem) cp = 1;
					if (acnt) 
						{ ma = 1; break; }
					else goto xx;
				}
			default:
			defa:
				if (at->check(atp,0)) {
					if (const_problem && rr==0) cp = 1;
					if (acnt) 
						{ ma = 1; break; }
					else goto xx;
				}
				if (const_problem) cp = 1;
				if (vrp_equiv) vp = 1;
			}

		}

		if (ma) goto xx;

		if (ax) {
			if (warn && ax->n_initializer)
				error("Ir makes overloaded%n ambiguous",nx);
			continue;
		}

		if (a) {
			if (warn && a->n_initializer)
				error("Ir makes overloaded%n ambiguous",nx);
			continue;
		}

		if (warn && fx->returns->check(f->returns,0))
			error("two different return valueTs for overloaded%n: %t and %t",nx,fx->returns,f->returns);
		if (warn) {
			if (vp) error('w',"ATs differ (only): [] vs *");
			if (op || cp) error('w',"the overloading mechanism cannot tell a%t from a%t",fx,f);
		}
		return nx;
	xx:
		if ( warn && fx->nargs <= vp+op+cp+xp 
		  && fx->nargs == f->nargs ) {
			// warn only in all argument are ``similar'', not for:
			// overload f(int,double), f(const,int);
			if (vp) error('w',"ATs differ (only): [] vs *");
			if (op || cp) error('w',"the overloading mechanism cannot tell a%t from a%t",fx,f);
		}
	}

	return 0;
}

int name::no_of_names()
{
	register int i = 0;
	register Pname n;
	for (n=this; n; n=n->n_list) i++;
	return i;
}

static Pexpr lvec[20], *lll;
static Pexpr list_back = 0;

void new_list(Pexpr lx)
{
	if (lx->base != ILIST) error('i',"IrLX");

	lll = lvec;
	lll++;
	*lll = lx->e1;
}

Pexpr next_elem()
{
	Pexpr e;
	Pexpr lx;

	if (lll == lvec) return 0;

 	lx = *lll;

	if (list_back) {
		e = list_back;
		list_back = 0;
		return e;
	}

	if (lx == 0) {				/* end of list */
		lll--;
		return 0;
	}

	switch (lx->base) {
	case ELIST:
		e = lx->e1;
		*lll = lx->e2;
		switch (e->base) {
		case ILIST:
			lll++;
			*lll = e->e1;
			return Pexpr(1);	// start of new ILIST
		case ELIST:
			error("nestedEL");
			return 0;
		default:
			return e;
		}
	default:
		error('i',"IrL");
	}
}

void list_check(Pname nn, Ptype t, Pexpr il)
/*
	see if the list "lll" can be assigned to something of type "t"
	"nn" is the name of the variable for which the assignment is taking place.
	"il" is the last list element returned by next_elem()
*/
{
	Pexpr e;
	bit lst = 0;
	int i;
        int tdef = 0;
	Pclass cl;
     
	if (il == Pexpr(1))
		lst = 1;
	else if (il)
		list_back = il;

zzz:
	switch (t->base) {
	case TYPE:
		t = Pbase(t)->b_name->tp;
		tdef += 1;
		goto zzz;

	case VEC:
	{	Pvec v = (Pvec)t;
		Ptype vt = v->typ;

		if (v->size) {	/* get at most v->size initializers */
			if (v->typ->base == CHAR) {
				e = next_elem();
				if (e->base == STRING) {	// v[size] = "..."
					int isz = Pvec(e->tp)->size;
					if (v->size < isz) error("Ir too long (%d characters) for%n[%d]",isz,nn,v->size);
					break;
				}
				else
					list_back = e;
			}
			for (i=0; i<v->size; i++) { // check next list element type
			ee:
				e = next_elem();
				if (e == 0) goto xsw; // too few initializers are ok
			vtz:
//error('d',"vtz: %d",vt->base);
				switch (vt->base) {
				case TYPE:
					vt = Pbase(vt)->b_name->tp;
					goto vtz;
				case VEC:
				case COBJ:
					list_check(nn,vt,e);
					break;
				default:
					if (e == (Pexpr)1) {
						error("unexpectedIrL");
						goto ee;
					}
					if (vt->check(e->tp,ASSIGN))
						error("badIrT for%n:%t (%tX)",nn,e->tp,vt);
				}
			}
			if ( lst && (e=next_elem()) ) error("end ofIrLX after vector");
		xsw:;
		}
		else {		/* determine v->size */
			i = 0;
		xx:
			while ( e=next_elem() ) {	// get another initializer
				i++;
			vtzz:
//error('d',"vtzz: %d",vt->base);
				switch (vt->base) {
				case TYPE:
					vt = Pbase(vt)->b_name->tp;
					goto vtzz;
				case VEC:
				case COBJ:
					list_check(nn,vt,e);
					break;
				default:
					if (e == (Pexpr)1) {
						error("unexpectedIrL");
						goto xx;
					}
					if (vt->check(e->tp,ASSIGN))
						error("badIrT for%n:%t (%tX)",nn,e->tp,vt);
				}
			}
			if ( !tdef ) v->size = i;
		}
		break;
	}

	case CLASS:
		cl = (Pclass)t;
		goto ccc;

	case COBJ:	/* initialize members */
		cl = Pclass(Pbase(t)->b_name->tp);
	ccc:
	{	Ptable tbl = cl->memtbl;
		Pname m;

		if (cl->clbase) list_check(nn,cl->clbase->tp,0);

		for (m=tbl->get_mem(i=1); m; m=tbl->get_mem(++i)) {
			Ptype mt = m->tp;
			switch (mt->base) {
			case FCT:
			case OVERLOAD:
			case CLASS:
			case ENUM:
				continue;
			}
			if (m->n_stclass == STATIC) continue;
			/* check assignment to next member */
		dd:
			e = next_elem();
			if (e == 0) return; //break;
		mtz:
//error('d',"mtz%n: %d",m,mt->base);
			switch (mt->base) {
			case TYPE:
				mt = Pbase(mt)->b_name->tp;
				goto mtz;
			case CLASS:
			case ENUM:
				break;
			case VEC:
			case COBJ:
				list_check(nn,m->tp,e);
				break;
			default:
				if (e == (Pexpr)1) {
					error("unexpectedIrL");
					goto dd;
				}
				if (mt->check(e->tp,ASSIGN))
					error("badIrT for%n:%t (%tX)",m,e->tp,m->tp);
			}
		}
		if (lst && (e=next_elem()) ) error("end ofIrLX afterCO");
		break;
	}
	default:
		e = next_elem();

		if (e == 0) {
			error("noIr forO");
			break;
		}
		
		if (e == (Pexpr)1) {
			error("unexpectedIrL");
			break;
		}
		if (t->check(e->tp,ASSIGN)) error("badIrT for%n:%t (%tX)",nn,e->tp,t);
		if (lst && (e=next_elem()) ) error("end ofIrLX afterO");
		break;
	}
}

Pname dclass(Pname n, Ptable tbl)
{	
	Pclass cl;
	Pbase bt;
	Pname bn;
	Pname nx = ktbl->look(n->string,0);		// TNAME
	if (nx == 0) {
	/*	search for hidden name for
		(1) nested class declaration
		(2) local class declaration
	*/
		int tn = 0;
		for (nx=ktbl->look(n->string,HIDDEN); nx; nx=nx->n_tbl_list) {
			if (nx->n_key != HIDDEN) continue;
			if (nx->tp->base != COBJ) {
				tn = 1;
				continue;
			}
			bt = (Pbase)nx->tp;
			bn = bt->b_name;
			cl = (Pclass)bn->tp;
			if (cl == 0) continue;
			goto bbb;
		}

		if (tn)
			error("%n redefined using typedef",n);
		else
			error('i',"%n is not aCN",n);
	}
	else {
/*
		if (tbl != gtbl) { 	// careful: local class def
			if (nx->lex_level == 0) // imperfect
				error('s',"localC%n and globalC%n",n,nx);
		}
*/
		bt = (Pbase)nx->tp;			// COBJ
		bn = bt->b_name;
	}
bbb:
	Pname ln = tbl->look(bn->string,0);
	if (ln && ln->n_table==tbl) error('w',"%n redefined",ln);
	bn->where = nx->where;
	Pname bnn = tbl->insert(bn,CLASS);		// copy for member lookup
	cl = (Pclass)bn->tp;
							// CLASS
	if (cl->defined&(DEFINED|SIMPLIFIED))
			error("C%n defined twice",n);
	else {
		if (bn->n_scope == ARG) bn->n_scope = ARGT;
		cl->dcl(bn,tbl);
	}
	n->tp = cl;
	return bnn;
}

Pname denum(Pname n, Ptable tbl)
{
	Pname nx = ktbl->look(n->string,0);		// TNAME
	if (nx == 0) nx = ktbl->look(n->string,HIDDEN);	// hidden TNAME
	Pbase bt = (Pbase)nx->tp;			// EOBJ
	Pname bn = bt->b_name;
	Pname bnn = tbl->insert(bn,CLASS);
	Penum en = (Penum)bn->tp;			// ENUM
	if (en->defined&(DEFINED|SIMPLIFIED))
		error("enum%n defined twice",n);
	else {
		if (bn->n_scope == ARG) bn->n_scope = ARGT;
		en->dcl(bn,tbl);
	}
	n->tp = en;
	return bnn;
}

void dargs(Pname, Pfct f, Ptable tbl)
{
	int oo = const_save;
	const_save = 1;
	for (Pname a=f->argtype; a; a=a->n_list) {
		Pexpr init;
		Pname cln = a->tp->is_cl_obj();
		if (cln && Pclass(cln->tp)->has_itor()) {
			// mark X(X&) arguments
			a->n_xref = 1;
		}
		if (init = a->n_initializer) {	// default argument
			if (cln) {
				if (init->base==VALUE) {
					switch (init->tp2->base) {
					case CLASS:
						if (Pclass(init->tp2)!=Pclass(cln->tp)) goto inin2;
						break;
					default:
						Pname n2 = init->tp2->is_cl_obj();
						if (n2==0 || Pclass(n2->tp)!=Pclass(cln->tp)) goto inin2;
					}
					init->e2 = a;
					init = init->typ(tbl);
					init->simpl();
					init->permanent = 2;
					a->n_initializer = init;
					error('s',"K as defaultA");
				}
				else {
				inin2:
					if (init->base == ILIST) error('s',"list as AIr");
					Pexpr i = init->typ(tbl);
					init = class_init(a,a->tp,i,tbl);
					if (i!=init && init->base==DEREF) error('s',"K needed forAIr");
					init->simpl();
					init->permanent = 2;
					a->n_initializer = init;
				}
			}
			else if (a->tp->is_ref()) {
				init = init->typ(tbl);
				int tcount = stcount;
				init = ref_init(Pptr(a->tp),init,tbl);
				if (tcount != stcount) error('s',"needs temporaryV to evaluateAIr");
				init->simpl();
				init->permanent = 2;
				a->n_initializer = init;
			}
			else {
				init = init->typ(tbl);
				if (a->tp->check(init->tp,ARG)) {
					int i = can_coerce(a->tp,init->tp);

					switch (i) {
					case 1:
						if (Ncoerce) {
							Pname cn = init->tp->is_cl_obj();
							Pclass cl = (Pclass)cn->tp;
							Pref r = new ref(DOT,init,Ncoerce);
							init = new expr(G_CALL,r,0);
							init->fct_name = Ncoerce;
							init->tp = a->tp;
						}
						break;
					default:
						error("%d possible conversions for defaultA",i);
					case 0:
						error("badIrT%t forA%n",init->tp,a);
						DEL(init);
						a->n_initializer = init = 0;
					}
				}
				
				if (init) {
					init->simpl();
					init->permanent = 2;
					a->n_initializer = init;
					Neval = 0;
					int i = init->eval();
					if (Neval == 0) {
						a->n_evaluated = 1;
						a->n_val = i;
					}
				}
			}
		}
	}
	const_save = oo;
}


void merge_init(Pname nn, Pfct f, Pfct nf)
{
	Pname a1 = f->argtype;
	Pname a2 = nf->argtype; 
	for (; a1; a1=a1->n_list, a2=a2->n_list) {
		int i1  =  a1->n_initializer || a1->n_evaluated;
		int i2  =  a2->n_initializer || a2->n_evaluated;
		if (i1) {
			if (i2
			&& (	a1->n_evaluated==0
			|| a2->n_evaluated==0
			|| a1->n_val!=a2->n_val)
			)
				error("twoIrs for%nA%n",nn,a1);
		}
		else if (i2) {
			a1->n_initializer = a2->n_initializer;
			a1->n_evaluated = a2->n_evaluated;
			a1->n_val = a2->n_val;
		}
	}
}

Pexpr try_to_coerce(Ptype rt, Pexpr e, char* s, Ptable tbl)
/*
	``e'' is of class ``cn'' coerce it to type ``rt''
*/
{
	int i;
	Pname cn;
//error('d',"try_to_coerce(%t, %t, %s, %d)",rt,e->tp,s,tbl);
	if ((cn=e->tp->is_cl_obj())
	&& (i=can_coerce(rt,e->tp))
	&& Ncoerce) {
//error('d',"use %n", Ncoerce);
		if (1 < i) error("%d possible conversions for %s",i,s);
		Pclass cl = Pclass(cn->tp);
		Pref r = new ref(DOT,e,Ncoerce);
		Pexpr rr = r->typ(tbl);
		Pexpr c = new expr(G_CALL,rr,0);
		c->fct_name = Ncoerce;
	//	c->tp = rt;
		return c->typ(tbl);
	}
	return 0;
}

int in_class_dcl;
Pname name::dofct(Ptable tbl, TOK scope)
{	
	Pfct f = (Pfct)tp;
	Pname class_name;
	Ptable etbl;
	int can_overload;
	int just_made = 0;

	in_class_dcl = cc->not!=0;
	if (f->f_inline) n_sto = STATIC;

	if (f->argtype) dargs(this,f,tbl);

	tp->dcl(tbl);		// must be done before the type check
	if (n_qualifier) {	// qualified name: c::f() checked above
		if (in_class_dcl) {
			error("unexpectedQdN%n",this);
			return 0;
		}
		class_name = Pbase(n_qualifier->tp)->b_name;
		etbl = Pclass(class_name->tp)->memtbl;
	  if ( f->f_virtual ) {
		error("virtual outsideCD %s::%n",
		       class_name->string, this );
		f->f_virtual = 0;
	  }
	}
	else {
		class_name = cc->not;
		// beware of local function declarations in member functions
		if (class_name && tbl!=cc->cot->memtbl) {
			class_name = 0;
			in_class_dcl = 0;
		}
		if (n_oper) check_oper(class_name);
		etbl = tbl;
	}

	Pfct(tp)->memof = class_name ? Pclass(class_name->tp) : 0;

	if (etbl==0 || etbl->base!=TABLE) error('i',"N::dcl: etbl=%d",etbl);

	switch (n_oper) {
	case NEW:
	case DELETE:
		switch (scope) {
		case 0:
		case PUBLIC:
			error("%nMF",this);
		}
	case 0:
		can_overload = in_class_dcl;
		break;
	case CTOR:
		if (f->f_virtual) {
			error("virtualK");
			f->f_virtual = 0;
		}
	case DTOR:
		if (fct_void) n_scope = PUBLIC;
		can_overload = in_class_dcl;
		break;
	case TYPE:
		can_overload = 0;
		break;
	case ASSIGN:
//error('d',"assign %n",class_name);
		if (class_name && f->nargs==1) {
			Ptype t = f->argtype->tp;
			Pname an = t->is_cl_obj();  // X::operator=(X) ?
			if (an==0 && t->is_ref()) { // X::operator=(X&) ?
				t = Pptr(t)->typ;
			rx1:
				switch (t->base) {
				case TYPE:	t = Pbase(t)->b_name->tp; goto rx1;
				case COBJ:	an = Pbase(t)->b_name;
				}
			}
			if (an && an==class_name) Pclass(an->tp)->bit_ass = 0;
//error('d',"%n ==> %d",an,Pclass(class_name)->bit_ass);
		}
		else if (f->nargs == 2) {
			Ptype t = f->argtype->tp;
			Pname an1;
			if (t->is_ref()) { // operator=(X&,?) ?
				t = Pptr(t)->typ;
			rx2:
				switch (t->base) {
				case TYPE:	t = Pbase(t)->b_name->tp; goto rx2;
				case COBJ:	an1 = Pbase(t)->b_name;
				}
			}
			t = f->argtype->n_list->tp;
			Pname an2 = t->is_cl_obj(); // operator=(X&,X) ?
			if (an2==0 && t->is_ref()) { // operator=(X&,X&) ?
				t = Pptr(t)->typ;
			rx3:
				switch (t->base) {
				case TYPE:	t = Pbase(t)->b_name->tp; goto rx3;
				case COBJ:	an2 = Pbase(t)->b_name;
				}
			}
			if (an1 && an1==an2) Pclass(an1->tp)->bit_ass = 0;
		}
	default:
		can_overload = 1;	/* all operators are overloaded */
	}

	switch (scope) {
	case FCT:
	case ARG:
	{	Pname nx = gtbl->insert(this,0);
		n_table = 0;
		n_tbl_list = 0;
		if (Nold && tp->check(nx->tp,0)) error('w',"%n has been declared both as%t and as%t",this,nx->tp,tp);
	}
	}

	Pname nn = etbl->insert(this,0);
	nn->assign();
	n_table = etbl;
			
	if (Nold) {
		Pfct nf = (Pfct)nn->tp;

		if (nf->base==ANY || f->base==ANY)
				;
		else if (nf->base == OVERLOAD) {
			Pgen g = (Pgen) nf;
			nn = g->add(this,0);
			string = nn->string;
			if (Nold == 0) {
				if (f->body) {
					if (n_qualifier) {
						error("badAL for overloaded %n::%s()",n_qualifier,g->string);
						return 0;
					}
				//	else if (f->f_inline==0 && n_oper==0)
				//		error('w',"overloaded %n definedWout being previously declared",nn);
				}
				goto thth;
			}
			else {
				if (f->body==0 && friend_in_class==0) error('w',"overloaded%n redeclared",nn);
			}
				
			nf = (Pfct)nn->tp;

			if (f->body && nf->body) {
				error("two definitions of overloaded%n",nn);
				return 0;
			}

			if (f->body) goto bdbd;
				
			goto stst;
		}
		else if (nf->base != FCT) {
			error("%n declared both as%t and asF",this,nf);
			f->body = 0;
		}
		else if (can_overload) {
			if (nf->check(f,OVERLOAD) || vrp_equiv) {
				if (f->body && n_qualifier) {
					error("badT for%n",nn);
					return 0;
				}
				Pgen g = new gen(string);
				Pname n1 = g->add(nn,in_class_dcl);
				Pname n2 = g->add(this,0);
				nn->tp = (Ptype)g;
				nn->string = g->string;
				nn = n2;
				goto thth;
			}
				
			if (in_class_dcl) {
				error("twoDs of%n",this);
				f->body = 0;
				return 0;
			}
				
			if (nf->body && f->body) {
				error("two definitions of%n",this);
				f->body = 0;
				return 0;
			}
				
			if (f->body) goto bdbd;

			goto stst;
		}
		else if (nf->check(f,0)) {
			switch (n_oper) {
			case CTOR:
			case DTOR:
				f->s_returns = nf->s_returns;
			}
			error("%nT mismatch:%t and%t",this,nf,f);
			f->body = 0;
		}
		else if (nf->body && f->body) {
			error("two definitions of%n",this);
			f->body = 0;
		}
		else if (f->body) {
		bdbd: 
			if (f->nargs_known && nf->nargs_known) merge_init(nn,f,nf);
			f->f_virtual = nf->f_virtual;
			f->f_this = nf->f_this;
			f->f_result = nf->f_result;
			f->s_returns = nf->s_returns;
/*fprintf(stderr,"bdbd %s: f %d inl %d nf %d inl %d\n",string,f,f->f_inline,nf,nf->f_inline);*/
			nn->tp = f;
			if (f->f_inline) {
				if (nf->f_inline==0) {
				        if (nn->n_used) error("%n called before defined as inline",nn);
					f->f_inline = nf->f_inline = 2;
					// can never be "outlined" 
					// because it has already
					// been declared extern
				}
				else
					nf->f_inline = 1;
				nn->n_sto = STATIC;
			}
			else if (nf->f_inline) {
				/*error("%n defined as inline but not declared as inline",this);*/
				f->f_inline = 1;
			}
			goto stst2;
		}
		else {	/* two declarations */
			f->f_this = nf->f_this;
			f->f_result = nf->f_result;
			f->s_returns = nf->s_returns;
		stst:
			if (f->nargs_known && nf->nargs_known) merge_init(nn,f,nf);
		stst2:
			if (f->f_inline) n_sto = STATIC;
			if (n_sto
			&& nn->n_scope!=n_sto
			&& friend_in_class==0
			&& f->f_inline==0){ // allow re-def to "static"
				switch( n_sto ) {
				case STATIC:
					nn->n_sto = STATIC;
					break;
				case EXTERN:
					if ( nn->n_scope == PUBLIC ||
     						nn->n_scope == 0 )
					break;
				default:
					error("%n both%k and%k",this,n_sto,nn->n_scope);
				}
			}
			n_scope = nn->n_scope; // first specifier wins
			n_sto = nn->n_sto;
			
		}
	/*	Pfct(nn->tp)->nargs_known = nf->nargs_known;	*/
	}
	else {	/* new function: make f_this for member functions */
		if (tbl==gtbl && n_oper) {	// overload operator
			Pgen g = new gen(string);
			Pname n1 = g->add(nn,1);
			nn->tp = Ptype(g);
			nn->string = g->string;
			string = n1->string;
			nn = n1;
		}
	thth:
		just_made = 1;
		if (f->f_inline)
			nn->n_sto = STATIC;
		else if (class_name==0 && n_sto==0 && f->body==0)
			nn->n_sto = EXTERN;

		if (class_name && etbl!=gtbl) {	/* beware of implicit declaration */
			Pname cn = nn->n_table->t_name;
			Pname tt = new name("this");
			tt->n_scope = ARG;
			tt->n_sto = ARG;
			tt->tp = Pclass(class_name->tp)->this_type;
			PERM(tt);
			Pfct(nn->tp)->f_this = f->f_this = tt;
			tt->n_list = f->argtype;
		}

		if (f->f_result == 0) {
			Pname rcln = f->returns->is_cl_obj();
			if (rcln && (/*f->f_virtual || */Pclass(rcln->tp)->has_itor())) make_res(f);
		}
		else if (f->f_this)
			f->f_this->n_list = f->f_result;

		if (f->f_virtual) {
			switch (nn->n_scope) {
			default:
				error("nonC virtual%n",this);
				break;
			case 0:
			case PUBLIC:
				cc->cot->virt_count = 1;
				Pfct(nn->tp)->f_virtual = 1;
				break;
			}
		}
	}

		/*	an operator must take at least one class object or
			reference to class object argument
		*/
	switch (n_oper) {
	case CTOR:
		if (f->nargs == 1) {	/* check for X(X) and X(X&) */
			Ptype t = f->argtype->tp;
		clll:
			switch (t->base) {
			case TYPE:
				t = Pbase(t)->b_name->tp;
				goto clll;
			case RPTR:			/* X(X&) ? */
				t = Pptr(t)->typ;
			cxll:
				switch (t->base) {
				case TYPE:
					t = Pbase(t)->b_name->tp;
					goto cxll;
				case COBJ:
					if (class_name == Pbase(t)->b_name)
						Pclass(class_name->tp)->itor = nn;
				}
				break;
			case COBJ:			/* X(X) ? */
				if (class_name == Pbase(t)->b_name)
					error("impossibleK: %s(%s)",class_name->string,class_name->string);
			}
		}
		break;
	case TYPE:
//error('d',"just_madeT %d %n",just_made,this);
		if (just_made) {
			nn->n_list = Pclass(class_name->tp)->conv;
			Pclass(class_name->tp)->conv = nn;
		}
		break;
	case DTOR:
	case NEW:
	case DELETE:
	case CALL:
	case 0:
		break;
	default:
		if (f->nargs_known != 1) {
			error("ATs must be fully specified for%n",nn);
		}
		else if (class_name == 0) {
			Pname a;
			switch (f->nargs) {
			case 1:
			case 2:
				for (a=f->argtype; a; a=a->n_list) {
					Ptype tx = a->tp;
					if (tx->base == RPTR) tx = Pptr(tx)->typ;
					if (tx->is_cl_obj()) goto cok;
				}
				error("%n must take at least oneCTA",nn);
				break;
			default:
				error("%n must take 1 or 2As",nn);
			}
		}
		else {
			switch (f->nargs) {
			case 0:
			case 1:
				break;
			default:
				error("%n must take 0 or 1As",nn);
			}
		}
	cok:;
	}

	int i = 0;	// check that every argument after an argument with
			// initializer have an initializer
	for (Pname a = f->argtype; a; a=a->n_list) {
		if (a->n_initializer)
			i = 1;
		else if (i)
			error("trailingA%n withoutIr",a);
	}

	/*
		the body cannot be checked until the name
		has been checked and entered into its table
	*/
	if (f->body) f->dcl(nn);
	return nn;
}
