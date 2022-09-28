/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/expr.c	1.1"
/*ident	"@(#)cfront:src/expr.c	1.13" */
/***************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


expr.c:

	type check expressions

************************************************************************/

#include "cfront.h"
#include "size.h"

Pname make_tmp(char, Ptype, Ptable);
Pexpr init_tmp(Pname, Pexpr, Ptable);

int const_save;

Pexpr expr::address()
{
//error('d',"address %k %d %s",base,base,base==NAME||base==ANAME?string:"?");
	switch (base) {
	case DEREF:
		if (e2 == 0) return e1;			// &*e	=> e
		break;
	case CM:
	case G_CM:
		e2 = e2->address();			// &(e1,e2) => (e1, &e2)
		return this;
	case CALL:
	case G_CALL:
	{
		Pname fn = fct_name;
		Pfct f = (fn)?(Pfct)fn->tp:0;
		if ( f && f->f_inline && fn->n_used > 1 )
			return this;
		break;
	}
	case NAME:
		if (Pname(this)->n_stclass == REGISTER) 
			error("& register%n",Pname(this));
		Pname(this)->take_addr();
	}

	register Pexpr ee = new expr(G_ADDROF,0,this);
	if (tp) {					// tp==0 ???
		ee->tp = tp->addrof();
		switch (tp->base) {
		case PTR:
			Pptr(ee->tp)->memof = Pptr(tp)->memof;
			break;
		case FCT:
			Pptr(ee->tp)->memof = Pfct(tp)->memof;
			break;
		case OVERLOAD:
			Pptr(ee->tp)->memof = Pfct(Pgen(tp)->fct_list->f->tp)->memof;
		}
	}

	return ee;
}

Pexpr expr::contents()
{
//error('d',"deref %k %d",base,base);
	if (base==ADDROF || base==G_ADDROF) return e2;		// *&
	register Pexpr ee = new expr(DEREF,this,0);
	if (tp) ee->tp = Pptr(tp)->typ;				// tp==0 ???
	return ee;
}

int bound;
int chars_in_largest;	// no of characters in largest int

Pexpr expr::typ(Ptable tbl)
/*
	find the type of "this" and place it in tp;
	return the typechecked version of the expression:
	"tbl" provides the scope for the names in "this"
*/
{
	//if (this == 0) error('i',"0->expr::typ");
	Pname n;
	Ptype t = 0;
	Ptype t1, t2;
	TOK b = base;
	TOK r1, r2;

#define nppromote(b)	t=np_promote(b,r1,r2,t1,t2,1)
#define npcheck(b)	(void)np_promote(b,r1,r2,t1,t2,0)
	if (tbl->base != TABLE) error('i',"expr::typ(%d)",tbl->base);
//if (b == NAME) error('d',"name %d %d %s",this,string,string?string:"?");
	if (tp) {
/*error('d',"expr::typ %d (checked) tbl=%d",this,tbl);*/
		if (b == NAME) Pname(this)->use();
		return this;
	}
//error('d',"expr::typ %d%k e1 %d%k e2 %d%k tbl %d\n",this,base,e1,e1?e1->base:0,e2,e2?e2->base:0,tbl);
	switch (b) {		/* is it a basic type */
	case DUMMY:
		error("emptyE");
		tp = any_type;
		return this;

	case ZERO:
		tp = zero_type;
		return this;

	case IVAL:
		tp = int_type;
		return this;

	case FVAL:
		tp = float_type;
		return this;

	case ICON:
		/*	is it long?
			explicit long?
			decimal larger than largest signed int
			octal or hexadecimal larger than largest unsigned int
		 */
	{	int ll = strlen(string);
		switch (string[ll-1]) {
		case 'l':
		case 'L':
			switch (string[ll-2]) {
			case 'u':
			case 'U':
				string[ll-2] = 0;
				this = new texpr(CAST,ulong_type,this);
				return typ(tbl);
			}
		lng:
			tp = long_type;
			goto save;
		case 'u':
		case 'U':			// 1u => unsigned(1)
			switch (string[ll-2]) {
			case 'l':
			case 'L':
				string[ll-2] = 0;
				t = ulong_type;
				break;
			default:
				string[ll-1] = 0;
				t = uint_type;
			}
			this = new texpr(CAST,t,this);
			return typ(tbl);
		}

		if  (string[0] == '0') {	// assume 8 bits in byte
			switch (string[1]) {
			case 'x':
			case 'X':
				if (SZ_INT+SZ_INT < ll-2) goto lng;
				goto nrm;
			default:
				if (BI_IN_BYTE*SZ_INT < (ll-1)*3) goto lng;
				goto nrm;
			}
		}
		else {
			if (ll<chars_in_largest) {
		nrm:
				tp = int_type;
				goto save;
			}
			if (ll>chars_in_largest) goto lng;
			char* p = string;
			char* q = LARGEST_INT;
			do if (*p++>*q++) goto lng; while (*p);
		}

		goto nrm;
	}
	case CCON:
		tp = char_type;
		goto save;

	case FCON:
		tp = double_type;
		goto save;

	case STRING:			// type of "as\tdf" is char[6]
					// c_strlen counts the terminating '\0'
	{	Pvec v = new vec(char_type,0);
		v->size = c_strlen(string);
		tp = v;
	}
	save:
		if (const_save) {	// "as\tdf" needs 7 chars for storage
			char* p = new char[strlen(string)+1];
			strcpy(p,string);
			string = p;
		}

		return this;

	case THIS:
		delete this;
		if (cc->tot) {
			cc->c_this->use();
			return cc->c_this;
		}
		error("this used in nonC context");
		n = new name("this");
		n->tp = any_type;
		return tbl->insert(n,0); 

	case NAME:
	{	Pexpr ee = tbl->find_name(Pname(this),0,0);
//error('d',"name%n %d %t",ee,Pname(ee)->n_xref,ee->tp);
		if (ee->tp->base == RPTR) return ee->contents();

		if (ee->base==NAME && Pname(ee)->n_xref) {
			// fudge to handle X(X&) args
			ee = new expr(DEREF,ee,0);
			ee->tp = ee->e1->tp;	// !!
		}

		return ee;
	}

	case ADDROF:
	case G_ADDROF:	// handle lookup for &s::i
		if (e2->base == NAME) e2 = tbl->find_name(Pname(e2),3,0);
		if (e2->base==NAME && Pname(e2)->n_xref) {
			// fudge to handle X(X&) args
			e2 = new expr(DEREF,e2,0);
			e2->tp = e2->e1->tp;	// !!
		}
		break;

	case SIZEOF:
		if (tp2) {
			tp2->dcl(tbl);
			if (e1 && e1!=dummy) {
				e1 = e1->typ(tbl);
				DEL(e1);
				e1 = dummy;
			}
		}
		else if (e1 == dummy) {
			error("sizeof emptyE");
			tp = any_type;
			return this;
		}
		else {
			e1 = e1->typ(tbl);
			tp2 = e1->tp;
		}
		tp = int_type;
		return this;

	case CAST:
		return docast(tbl);

	case VALUE:
		return dovalue(tbl);

	case NEW:
		return donew(tbl);

	case DELETE:	// delete e1 OR delete[e2] e1
	{	int i;
		if (e1->base == ADDROF) error('w',"delete &E");
		e1 = e1->typ(tbl);
		i = e1->tp->num_ptr(DELETE);
		if (i != P) error("nonP deleted");
		if (e2) {
			e2 = e2->typ(tbl);
			e2->tp->integral(DELETE);
		}
		tp = void_type;
		return this;
	}

	case ILIST:	/* an ILIST is pointer to an ELIST */
		e1 = e1->typ(tbl);
		tp = any_type;
		return this;

	case ELIST:
	{	Pexpr e;
		Pexpr ex;

		if (e1 == dummy && e2==0) {
			error("emptyIrL");
			tp = any_type;
			return this;
		}
				
		for (e=this; e; e=ex) {
			Pexpr ee = e->e1;
/*error('d',"e %d %d ee %d %d",e,e?e->base:0,ee,ee?ee->base:0);*/
			if (e->base != ELIST) error('i',"elist%k",e->base);
			if (ex = e->e2) {	/* look ahead for end of list */
				if (ee == dummy) error("EX in EL");
				if (ex->e1 == dummy && ex->e2 == 0) {
					/* { ... , } */
					DEL(ex);
					e->e2 = ex = 0;
				}
			}
			e->e1 = ee->typ(tbl);
			t = e->e1->tp;
				
		}
		tp = t;
		return this;
	}

	case DOT:
	case REF:
//error('d',"ref/dot e1 = %p e2 = %p",e1,e2);
	if (e2) {	// a .* p => &a MEMPTR p => appropriate indirection
		Pexpr a = e1->typ(tbl);
		if (base == DOT) a = a->address();
		Pexpr p = e2->typ(tbl);
		Ptype pt = p->tp;
//error('d',"p: %k (%t) %t",p->base,pt,a->tp);
		while (pt->base == TYPE) pt = Pbase(pt)->b_name->tp;
		if (pt->base!=PTR || Pptr(pt)->memof==0) {
			error("P toMFX in .*E: %t",pt);
			tp = any_type;
			return this;
		}
		Pclass pm = Pptr(pt)->memof;
		Pname cn = Pptr(a->tp)->typ->is_cl_obj();
		Pclass mm = cn ? Pclass(cn->tp) : 0;
		if (mm!=pm && pm->baseof(mm)==0) {
			error("badOT in .*E: %t (%s*X)",a->tp,pm->string);
			tp = any_type;
			return this;
		}
		Ptype tx = Pptr(pt)->typ;
		while (tx->base == TYPE) tx = Pbase(tx)->b_name->tp;
		if (tx->base == FCT) {	// a.*p => (&a MEMPTR p)
			base = MEMPTR;
			tp2 = mm;	// keep the class for simpl.c
			e1 = a;
			e2 = p;
		}
		else {	// a .* p => *(typeof(p))((char*)&a + (int)p-1)
			a = new texpr(CAST,Pchar_type,a);
			a->tp = Pchar_type;
			p = new texpr(CAST,int_type,p);
			p->tp = int_type;
			p = new expr(MINUS,p,one);
			p->tp = int_type;
			Pexpr pl = new expr(PLUS,a,p);
			pl->tp = Pint_type;
			base = DEREF;
			e1 = new texpr(CAST,pt,pl);
			e1->tp = pt;
			e2 = 0;
		}
		tp = tx;
		if (tp->base == RPTR) return contents();
		return this;
	}
	else {
		Pbase b;
		Ptable atbl;
		Pname nn;
		char* s;
		Pclass cl;

		e1 = e1->typ(tbl);
		t = e1->tp;
		if (base == REF) {
		xxx:
//error('d',"xxx %t",t);
			switch (t->base) {
			case TYPE:	t = Pbase(t)->b_name->tp;	goto xxx;
			default:	error("nonP ->%n",mem); t = any_type;
			case ANY:	goto qqq;
			case PTR:
			case VEC:	b = Pbase(Pptr(t)->typ);	break;
			}
		}
		else {
		qqq:
			switch (t->base) {
			case TYPE:	t = Pbase(t)->b_name->tp;	goto qqq;
			default:	error("nonO .%n",mem); t = any_type;
			case ANY:
			case COBJ:	break;
			}

//error('d',"dot e1 %k %d",e1->base,e1->base);
			switch (e1->base) { // FUDGE, but cannot use lval (consts)
			case CM:	// ( ... , x). => ( ... , &x)->
			{	Pexpr ex = e1;
			cfr:
				switch (ex->e2->base) {
				case NAME:
					base = REF;
					ex->e2 = ex->e2->address();
					e1->tp = t = ex->e2->tp;
					goto xxx;
				case CM:
					ex = ex->e2;
					goto cfr;
				}
				break;
			}
			case CALL:
			case G_CALL:	// f(). => (tmp=f(),&tmp)->
			{	Pname tmp = make_tmp('T',e1->tp,tbl);
				e1 = init_tmp(tmp,e1,tbl);
				Pexpr aa = tmp->address();
				e1 = new expr(CM,e1,aa);
				e1->tp = aa->tp;
				base = REF;
				break;
			}
			}
			b = Pbase(t);
		}

	xxxx:
		switch (b->base) {
		case TYPE:
			b = (Pbase) b->b_name->tp;
			goto xxxx;
		default:
			error("(%t) before %k%n (%n not aM)",e1->tp,base,mem,mem);
		case ANY:
			atbl = any_tbl;
			break;
		case COBJ:
			if (atbl = b->b_table) break;
			s = b->b_name->string;	/* lookup the class name */
			if (s == 0) error('i',"%kN missing",CLASS);
//error('d',"lookup %s",s);
			nn = tbl->look(s,CLASS);
			if (nn == 0) error('i',"%k %sU",CLASS,s);
			if (nn != b->b_name) b->b_name = nn;
			cl = (Pclass) nn->tp;
			PERM(cl);
			if (cl == 0) error('i',"%k %s'sT missing",CLASS,s);
			b->b_table = atbl = cl->memtbl;
		}

		nn = (Pname)atbl->find_name(mem,2,0);
//error('d',"nn%n %d %d %t",nn,nn->n_stclass,nn->n_scope,nn->tp);
		if (nn->n_stclass == 0) {
			mem = nn;	
			tp = nn->tp;
			if (tp->base == RPTR) return contents();
			return this;
		}
		if (nn->tp->base == RPTR) return contents();
		return nn;
	}

	case CALL:	/* handle undefined function names */
		if (e1->base==NAME && e1->tp==0) {
//error('d',"call %d %s",e1,e1->string);
			e1 = tbl->find_name(Pname(e1),1,e2);
		}
		if (e1->base==NAME && Pname(e1)->n_xref) {
			// fudge to handle X(X&) args
			e1 = new expr(DEREF,e1,0);
			e1->tp = e1->e1->tp;	// !!
		}
		break;

	case QUEST:
		cond = cond->typ(tbl);
	}
//error('d',"b%k %d %d",b,e1,e2);
	if (e1) {
		e1 = e1->typ(tbl);
		if (e1->tp->base == RPTR) e1 = e1->contents();
		t1 = e1->tp;
	}
	else
		t1 = 0;

	if (e2) {
		e2 = e2->typ(tbl);
		if (e2->tp->base == RPTR) e2 = e2->contents();
		t2 = e2->tp;
	}
	else 
		t2 = 0;

	switch (b) {	// filter out non-overloadable operators
	default:
	{	Pexpr x = try_to_overload(tbl);
		if (x) return x;
	}
	case CM:
	case G_CM:
	case QUEST:
	case G_ADDROF:
	case G_CALL:
		break;
	}

	t = (t1==0) ? t2 : (t2==0) ? t1 : 0;
/*fprintf(stderr,"%s: e1 %d %d e2 %d %d\n",oper_name(b),e1,e1?e1->base:0,e2,e2?e2->base:0);*/

	switch (b) {		/* are the operands of legal types */
	case G_CALL:
	case CALL:
//error('d',"call e1 %d e2 %d e1->tp %d",e1,e2,e1->tp);
		tp = fct_call(tbl);	/* two calls of use() for e1's names */
		if (tp->base == RPTR) return contents();
		return this;

	case DEREF:
//error('d',"deref %t",t?t:t1);
		if (e1 == dummy) error("O missing before []\n");
		if (e2 == dummy) error("subscriptE missing");
		if (t) {	/*	*t	*/
			while (t->base == TYPE) t = Pbase(t)->b_name->tp;
			t->vec_type();
			if (Pptr(t)->memof) error("P toM deRd");
			tp = t->deref();
		}
		else {					// e1[e2] that is *(e1+e2)
			if (t1->vec_type()) {		// e1[e2]
				switch (t2->base) {
				case CHAR:
				case SHORT:
				case INT:
				case LONG:
				case EOBJ:
					break;
				default:
				{	Pname cn = t2->is_cl_obj();
					if (cn)	// conversion to integral?
						e2 = check_cond(e2,DEREF,tbl);
					else
						t2->integral(DEREF);
				}
				}
				while (t1->base == TYPE) t1 = Pbase(t1)->b_name->tp;
				if (Pptr(t1)->memof) error("P toM deRd");
				tp = t1->deref();
			}
			else if (t2->vec_type()) {	// really e2[e1]
				t1->integral(DEREF);
				while (t2->base == TYPE) t2 = Pbase(t2)->b_name->tp;
				if (Pptr(t2)->memof) error("P toM deRd");
				tp = t2->deref();
			}
			else {
				error("[] applied to nonPT:%t[%t]",t1,t2);
				tp = any_type;
			}
		}
		if (tp->base == RPTR) return contents();
		return this;

	case G_ADDROF:
	case ADDROF:	
		if (e2->lval(b) == 0) {
			tp = any_type;
			return this;
		}
		tp = t->addrof();
		if (t->tconst() && vec_const==0 && fct_const==0) Pptr(tp)->rdo = 1;
		switch (e2->base) {
		case NAME:
		mname:					// check for &s::i
		{	Pname n2 = Pname(e2);
			Pname cn = (n2->n_table!=gtbl) ? n2->n_table->t_name : 0;
			if (cn == 0) break;
			Pptr(tp)->memof = Pclass(cn->tp);
			if (t->base==FCT) {
				n2->lval(ADDROF); // ``outline'' inlines 
				Pfct f = Pfct(t);
//error('d',"&%n %d virt %d",n2,n2,f->f_virtual);
				if (f->f_virtual)		// vtbl index + 1
					e1 = new ival(f->f_virtual);
				else				// use the pointer
					break;
			}
			else {
				if (n2->n_stclass != STATIC)	// offset + 1
					e1 = new ival(n2->n_offset+1);
				else {			// can't do it
				//	error('s',"&%n (%n is static)",n2,n2);
				//	break;
					tp = new ptr(PTR,e2->tp);
					return this;
				}
			}
			e1->tp = int_type;
			e2 = 0;
			tp2 = tp;
			base = CAST;
			return this;
		}
		case DOT:
		case REF:
		{	Pname m = e2->mem;
			Pfct f = (Pfct)m->tp;
			if (f->base==FCT) {
				if (bound==0 && e2->e1==cc->c_this && m->n_qualifier) {
					// FUDGE: &this->x::f => &x::f
					DEL(e2);
					e2 = m;
					goto mname;
				}
				error('w',"address of boundF (try %s::*PT and &%s::%s address)",m->n_table->t_name->string,m->n_table->t_name->string,m->string);
				if (f->f_virtual==0 || m->n_qualifier) {
					// & x.f  = & f
					DEL(e2);
					e2 = m;
				}
			}
		}
		}
		return this;

	case UMINUS:
		t->numeric(UMINUS);
		tp = t;
		return this;

	case UPLUS:
		t->num_ptr(UPLUS);
		error('s',"unary + (ignored)");
		tp = t;
		base = PLUS;
		e1 = zero;
		return this;

	case NOT:
		e2 = check_cond(e2,NOT,tbl);
		tp = int_type;
		return this;

	case COMPL:
		t->integral(COMPL);
		tp = t;
		return this;

	case INCR:
	case DECR:
		if (e1) e1->lval(b);
		if (e2) e2->lval(b);
		r1 = t->num_ptr(b);
		tp = t;
		return this;
	
	}

	if (e1==dummy || e2==dummy || e1==0 || e2==0) error("operand missing for%k",b);
	switch (b) {
	case MUL:
	case DIV:
		r1 = t1->numeric(b);
		r2 = t2->numeric(b);
		nppromote(b);
		break;

	case PLUS:
		r2 = t2->num_ptr(PLUS);
		r1 = t1->num_ptr(PLUS);
		if (r1==P && r2==P) error("P +P");
		nppromote(PLUS);
		tp = t;
		break;

	case MINUS:
		r2 = t2->num_ptr(MINUS);
		r1 = t1->num_ptr(MINUS);
		if (r2==P && r1!=P && r1!=A) error("P - nonP");
		nppromote(MINUS);
		tp = t;
		break;

	case LS:
	case RS:
	case AND:
	case OR:
	case ER:
		switch (e1->base) {
		case LT:
		case LE:
		case GT:
		case GE:
		case EQ:
		case NE:
			error('w',"%kE as operand for%k",e1->base,b);
		}
		switch (e2->base) {
		case LT:
		case LE:
		case GT:
		case GE:
		case EQ:
		case NE:
			error('w',"%kE as operand for%k",e2->base,b);
		}
	case MOD:
		r1 = t1->integral(b);
		r2 = t2->integral(b);
		nppromote(b);
		break;

	case LT:
	case LE:
	case GT:
	case GE:
	case EQ:
	case NE:
		r1 = t1->num_ptr(b);
		r2 = t2->num_ptr(b);
		npcheck(b);
		t = int_type;
		break;

	case ANDAND:
	case OROR:
		e1 = check_cond(e1,b,tbl);
		e2 = check_cond(e2,b,tbl);
		t = int_type;
		break;

	case QUEST:
		{
		Pname c1, c2;
		cond = check_cond(cond,b,tbl);
		// still doesn't do complete checking for possible conversions...
		if (t1==t2
		|| (	(c1=t1->is_cl_obj())
			&& (c2=t2->is_cl_obj())
			&& (c1->tp==c2->tp)
		))
			t = t1;
		else {
			r1 = t1->num_ptr(b);
			r2 = t2->num_ptr(b);

			if (r1==FCT && r2==FCT) {	// fudge
				if (t1->check(t2,ASSIGN)) error("badTs in ?:E: %t and %t",t1,t2);
				t = t1;
			}
			else
				nppromote(b);
				
			if (t!=t1 && t->check(t1,0)) {
				e1 = new texpr(CAST,t,e1);
				e1->tp = t;
			}
			if (t!=t2 && t->check(t2,0)) {
				e2 = new texpr(CAST,t,e2);
				e2->tp = t;
			}
			
		}
		}
		break;

	case ASPLUS:
		r1 = t1->num_ptr(ASPLUS);
		r2 = t2->num_ptr(ASPLUS);
		if (r1==P && r2==P) error("P +=P");
		nppromote(ASPLUS);
		goto ass;

	case ASMINUS:
		r1 = t1->num_ptr(ASMINUS);
		r2 = t2->num_ptr(ASMINUS);
		if (r2==P && r1!=P && r1!=A) error("P -= nonP");
		nppromote(ASMINUS);
		goto ass;

	case ASMUL:
	case ASDIV:
		r1 = t1->numeric(b);
		r2 = t1->numeric(b);
		nppromote(b);
		goto ass;

	case ASMOD:
		r1 = t1->integral(ASMOD);
		r2 = t2->integral(ASMOD);
		nppromote(ASMOD);
		goto ass;

	case ASAND:
	case ASOR:
	case ASER:
	case ASLS:
	case ASRS:
		r1 = t1->integral(b);
		r2 = t2->integral(b);
		npcheck(b);
		t = int_type;
		goto ass;
	ass:
		as_type = t;	/* the type of the rhs */
		t2 = t;

	case ASSIGN:
		if (e1->lval(b) == 0) {
			tp = any_type;
			return this;
		}
	lkj:
		switch (t1->base) {
		case TYPE:
			t1 = Pbase(t1)->b_name->tp;
			goto lkj;
		case INT:
		case CHAR:
		case SHORT:
			if (e2->base==ICON && e2->tp==long_type)
				error('w',"long constant assigned to%k",t1->base);
		case LONG:
			if (b==ASSIGN
			&& Pbase(t1)->b_unsigned
			&& e2->base==UMINUS
			&& e2->e2->base==ICON)
				error('w',"negative assigned to unsigned");
			break;
		case PTR:
			if (b == ASSIGN) {
				e2 = ptr_init(Pptr(t1),e2,tbl);
				t2 = e2->tp;
			}
			break;
		case COBJ:
		{	Pname c1 = t1->is_cl_obj();

			if (c1) {
				Pname c2 = t2->is_cl_obj();
//error('d',"%t=%t %d %d",t1,t2,c1,c2);
				if (c1 != c2) {
					e2 = new expr(ELIST,e2,0);
					e2 = new texpr(VALUE,t1,e2);
					e2->e2 = e1;
					e2 = e2->typ(tbl);
//					*this = *e2;
					tp = t1;
					return this;
				}
				else {	// check for bitwise copy
					Pclass cl = Pclass(c1->tp);
//error('d',"bit %d",cl->bit_ass);
					if (cl->bit_ass == 0)
						error('s',"bitwise copy: %s has aMW operator=()",cl->string);
					else if (cl->itor && cl->has_dtor())
						error('w',"bitwise copy: %s has destructor and %s(%s&) but not assignment",cl->string,cl->string,cl->string);	
				}
			}
			break;
		}
		}

//error('d',"check(%t,%t)",e1->tp,t2);
		{	Pexpr x = try_to_coerce(t1,e2,"assignment",tbl);
			if (x)
				e2 = x;
			else if (e1->tp->check(t2,ASSIGN))
				error("bad assignmentT:%t =%t",e1->tp,t2);
		}		
		t = e1->tp;				/* the type of the lhs */
		break;
	case CM:
	case G_CM:
		t = t2;
		break;
	default:
		error('i',"unknown operator%k",b);
	}

	tp = t;
	return this;
}

