/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/typ.c	1.1"
/*ident	"@(#)cfront:src/typ.c	1.12" */
/**************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


typ.c:


***************************************************************************/
 
#include "cfront.h"
#include "size.h"

Pbase short_type;
Pbase int_type;
Pbase char_type;
Pbase long_type;

Pbase uchar_type;
Pbase ushort_type;
Pbase uint_type;
Pbase ulong_type;

Pbase zero_type;
Pbase float_type;
Pbase double_type;
Pbase void_type;
Pbase any_type;

Ptype Pint_type;
Ptype Pchar_type;
Ptype Pvoid_type;
Ptype Pfctchar_type;
Ptype Pfctvec_type;

Ptable gtbl;
Ptable any_tbl; 

Pname Cdcl = 0;
Pstmt Cstmt = 0;

extern int suppress_error;

bit new_type = 0;

extern Ptype np_promote(TOK, TOK, TOK, Ptype, Ptype, TOK);
Ptype np_promote(TOK oper, TOK r1, TOK r2, Ptype t1, Ptype t2, TOK p)
/*
	an arithmetic operator "oper" is applied to "t1" and "t2",
	types t1 and t2 has been checked and belongs to catagories
	"r1" and "r2", respectively:
		A	ANY
		Z	ZERO
		I	CHAR, SHORT, INT, LONG, FIELD, or EOBJ
		F	FLOAT DOUBLE
		P	PTR (to something) or VEC (of something)
	test for compatability of the operands,
	if (p) return the promoted result type
*/
{
	if (r2 == A) return t1;

	switch (r1) {
	case A:	return t2;
	case Z:
		switch (r2) {
		case Z:		return int_type;
		case I:
		case F:		return (p) ? Pbase(t2)->arit_conv(0) : 0;
		case P:		return t2;
		default:	error('i',"zero(%d)",r2);
		}
	case I:
		switch (r2) {
		case Z: t2 = 0;
		case I:
		case F: return (p) ? Pbase(t1)->arit_conv(Pbase(t2)) : 0;
		case P: switch (oper) {
			case PLUS:
			case ASPLUS:	break;
			default:	error("int%kP",oper); return any_type;
			}
			return t2;
		case FCT: error("int%kF",oper); return any_type;
		default: error('i',"int(%d)",r2);
		}
	case F:
		switch (r2) {
		case Z: t2 = 0;
		case I:
		case F: return (p) ? Pbase(t1)->arit_conv(Pbase(t2)) : 0;
		case P: error("float%kP",oper); return any_type;
		case FCT: error("float%kF",oper); return any_type;
		default: error('i',"float(%d)",r2);
		}
	case P:
		switch (r2) {
		case Z: return t1;
		case I:
			switch (oper) {
			case PLUS:
			case MINUS:
			case ASPLUS:
			case ASMINUS: break;
			default: error("P%k int",oper); return any_type;
			}
			return t1;
		case F: error("P%k float",oper); return any_type;
		case P:
			if (t1->check(t2,ASSIGN)) {
				switch (oper) {
				case EQ:
				case NE:
				case LE:
				case GE:
				case GT:
				case LT:
				case QUEST:
					if (t2->check(t1,ASSIGN) == 0) goto zz;
				}
				error("T mismatch:%t %k%t",t1,oper,t2);
				return any_type;
			}
			zz:
			switch (oper) {
			case MINUS:
			case ASMINUS:	return int_type;
			case PLUS:
			case ASPLUS:	error("P +P"); return any_type;
			default:	return t1;
			}
		case FCT:	return t1;
		default:	error('i',"P(%d)",r2);
		}
	case FCT:
		error("F%k%t",oper,t2);
		return any_type;
	default:
		error('i',"np_promote(%d,%d)",r1,r2);
	}
}

TOK type::kind(TOK oper, TOK v)
/*	v ==	I	integral
		N	numeric
		P	numeric or pointer
*/
{
	Ptype t = this;
xx:
	switch (t->base) {
	case ANY:	return A;
	case ZTYPE:	return Z;
	case FIELD:
	case CHAR:
	case SHORT:
	case INT:
	case LONG:
	case EOBJ:	return I;
	case FLOAT:
	case DOUBLE:	if (v == I) error("float operand for %k",oper);	return F;
	case PTR:	if (v != P) error("P operand for %k",oper);
			switch (oper) {
			case INCR:
			case DECR:
			case MINUS:
			case PLUS:
			case ASMINUS:
			case ASPLUS:
				if (Pptr(t)->memof || Pptr(t)->typ->base==FCT)
					error("%t operand of%k",this,oper);
				else
					Pptr(t)->typ->tsizeof(); // get increment
				break;
			default:
				if (Pptr(t)->memof || Pptr(t)->typ->base==FCT)
					error("%t operand of%k",this,oper);
			case ANDAND:
			case OROR:
			case ASSIGN:
			case NE:
			case EQ:
			case IF:
			case WHILE:
			case DO:
			case FOR:
			case QUEST:
			case NOT:
					break;
			
			}
			return P;
	case RPTR:	error("R operand for %k",oper);
			return A;
	case VEC:	if (v != P) error("V operand for %k",oper);	return P;
	case TYPE:	t = Pbase(t)->b_name->tp;			goto xx;
	case FCT:	if (v != P) error("F operand for %k",oper);	return FCT;
	case CLASS:
	case ENUM:	error("%k operand for %k",base,oper);		return A;
	default:	error("%t operand for %k",this,oper);		return A;
	}
}

void type::dcl(Ptable tbl)
/*
	go through the type (list) and
	(1) evaluate vector dimentions
	(2) evaluate field sizes
	(3) lookup struct tags, etc.
	(4) handle implicit tag declarations
*/
{
	Ptype t = this;

	if (this == 0) error('i',"T::dcl(this==0)");
	if (tbl->base != TABLE) error('i',"T::dcl(%d)",tbl->base);

xx:
	switch (t->base) {
	case PTR:
	case RPTR:
	{	Pptr p = (Pptr)t;
		t = p->typ;
		if (t->base == TYPE) {
			Ptype tt = Pbase(t)->b_name->tp;
			if (tt->base == FCT) p->typ = tt;
			return;
		}
		goto xx;
	}

	case VEC:
	{	Pvec v = (Pvec)t;
		Pexpr e = v->dim;
		if (e) {
			Ptype et;
			v->dim = e = e->typ(tbl);
			et = e->tp;
			if (et->integral(0) == A)  {
				error("UN in array dimension");
			}
			else if (!new_type) {
				int i;
				Neval = 0;
				i = e->eval();
				if (Neval) error("%s",Neval);
				else if (i == 0)
					error('w',"array dimension == 0");
				else if (i < 0) {
					error("negative array dimension");
					i = 1;
				}
				v->size = i;
				DEL(v->dim);
				v->dim = 0;
			}
		}
		t = v->typ;
	llx:
		switch (t->base) {
		case TYPE:
			t = Pbase(t)->b_name->tp;
			goto llx;
		case FCT:
			v->typ = t;
			break;
		case VEC:
			if (Pvec(t)->dim==0 && Pvec(t)->size==0) error("null dimension (something like [][] seen)");
		}
		goto xx;
	}

	case FCT:
	{	Pfct f = Pfct(t);
		for (Pname n=f->argtype; n; n = n->n_list) n->tp->dcl(tbl);
		Pname cn = f->returns->is_cl_obj();
		if (cn && Pclass(cn->tp)->has_itor()) {
			Pname rv = new name("_result");
			rv->tp  = new ptr(PTR,f->returns);
			rv->n_scope = FCT;	// not a ``real'' argument
			rv->n_used = 1;
			rv->n_list = f->argtype;
			if (f->f_this) f->f_this->n_list = rv;
			f->f_result = rv;
		}
		t = f->returns;
		goto xx;
	}

	case FIELD:
	{	Pbase f = (Pbase)t;
		Pexpr e = (Pexpr)f->b_name;
		int i;
		Ptype et;
		e = e->typ(tbl);
		f->b_name = (Pname)e;
		et = e->tp;
		if (et->integral(0) == A) {
			error("UN in field size");
			i = 1;
		}
		else {
			Neval = 0;
			i = e->eval();
			if (Neval)
				error("%s",Neval);
			else if (i < 0) {
				error("negative field size");
				i = 1;
			}
			else if (f->b_fieldtype->tsizeof()*BI_IN_BYTE < i)
				error("field size > sizeof(%t)",f->b_fieldtype);
			DEL(e);
		}
		f->b_bits = i;
		f->b_name = 0;
		break;
	}

	}
}

bit vrp_equiv;	// vector == pointer equivalence used in check()
bit const_problem;
extern t_const;

bit type::check(Ptype t, TOK oper)
/*
	check if "this" can be combined with "t" by the operator "oper"

	used for check of
			assignment types		(oper==ASSIGN)
			declaration compatability	(oper==0)
			argument types			(oper==ARG)
			return types			(oper==RETURN)
			overloaded function  name match	(oper==OVERLOAD)
			overloaded function coercion	(oper==COERCE)

	NOT for arithmetic operators

	return 1 if the check failed
*/
{
	Ptype t1 = this;
	Ptype t2 = t;
	TOK b1, b2;
	bit t2_const = 0, t1_const = t_const; t_const = 0;
	bit first = 1;
	TOK r;
//error('d',"check (%p: %t) (%p: %t)",t1,t1,t2,t2);
	if (t1==0 || t2==0) error('i',"check(%p,%p,%d)",t1,t2,oper);

	vrp_equiv = 0;
	const_problem = 0;

	while (t1 && t2) {
	top:
//error('d',"top: %t (%d) %t (%d)",t1,t1->base,t2,t2->base);
		if (t1 == t2) return 0;
		if (t1->base == ANY || t2->base == ANY) return 0;

		b1 = t1->base;
		if (b1 == TYPE) {
			if (!oper) t1_const = Pbase(t1)->b_const;
			t1 = Pbase(t1)->b_name->tp;
			goto top;
		}

		b2 = t2->base;
		if (b2 == TYPE) {
			if (!oper) t2_const = Pbase(t2)->b_const;
			t2 = Pbase(t2)->b_name->tp;
			goto top;
		}

		if (b1 != b2) {
			switch (b1) {
			case PTR:
				vrp_equiv = 1;
				switch (b2) {
				case VEC:
					t1 = Pptr(t1)->typ;
					t2 = Pvec(t2)->typ;
					first = 0;
					goto top;
				case FCT:
					t1 = Pptr(t1)->typ;
					if (first==0 || t1->base!=b2) return 1;
					first = 0;
					goto top;
				}
				first = 0;
				break;
			case VEC:
				vrp_equiv = 1;
				first = 0;
				switch (b2) {
				case PTR:
					switch(oper) {
					case 0:
					case ARG:
					case ASSIGN:
					case COERCE:
						break;
					case OVERLOAD:
					default:
						return 1;
					}
					t1 = Pvec(t1)->typ;
					t2 = Pptr(t2)->typ;
					goto top;
				}
				break;
			}
			goto base_check; 
		}

		switch (b1) {
		case VEC:
			first = 0;
		{	Pvec v1 = (Pvec)t1;
			Pvec v2 = (Pvec)t2;
			if (v1->size != v2->size)
				switch (oper) {
				case OVERLOAD:
				case COERCE:
					return 1;
				}
			t1 = v1->typ;
			t2 = v2->typ;
		}
			break;

		case PTR:
		case RPTR:
			first = 0;
		{	Pptr p1 = (Pptr)t1;
			Pptr p2 = (Pptr)t2;
			if (p1->memof != p2->memof) {
/*				Pclass c1 = p1->memof;
				Pclass c2 = p2->memof;
//error('d',"c1 %s c2 %s", c1->string, c2->string);
				if (c1==0 || c2==0) return 1;
				for(;;) {	//	is c2 derived from c1 ?
					if (c2->clbase==0) return 1;
					c2 = Pclass(c2->clbase->tp);
//error('d',"c1 %s c2 %s 2", c1->string, c2->string);
					if (c2 == c1) {
						Nstd++;
						break;
					}
				}
*/
				if (p2->memof->baseof(p1->memof)) return 1;
				Nstd++;
			}
			t1 = p1->typ;
			t2 = p2->typ;
			if (oper==ARG && t1->base==TYPE) 
				t1_const = Pbase(p1->typ)->b_const;
			if (oper==ARG && t2->base==TYPE ) 
				t2_const = Pbase(p2->typ)->b_const;
			// rdo
			if (oper==0) {
				if (p1->rdo!=p2->rdo) {
					const_problem = 1;
					return 1;
				}
				if (b1==RPTR && t1->tconst()!=t2->tconst())
					const_problem = 1;
			}
			break;
		}

		case FCT:
			first = 0;
		{	Pfct f1 = (Pfct)t1;
			Pfct f2 = (Pfct)t2;
			Pname a1 = f1->argtype;
			Pname a2 = f2->argtype;
			TOK k1 = f1->nargs_known;
			TOK k2 = f2->nargs_known;
			int n1 = f1->nargs;
			int n2 = f2->nargs;
//error('d',"k %d %d n %d %d body %d %d",k1,k2,n1,n2,f1->body,f2->body);
			if (f1->memof != f2->memof) {
				if (f2->memof->baseof(f1->memof)) return 1;
				Nstd++;
			}
			if ( (k1 && k2==0) || (k2 && k1==0) ){
				if (f2->body == 0) return 1;
			}

			if (n1!=n2 && k1 && k2) {
				goto aaa;
			}
			else if (a1 && a2) {
				int i = 0;
				while (a1 && a2) {
					i++;
					if ( a1->tp->check(a2->tp,oper?OVERLOAD:0) ) return 1;
					a1 = a1->n_list;
					a2 = a2->n_list;
				}
				if (a1 || a2) goto aaa;
			}
			else if (a1 || a2) {
			aaa:
				if (k1 == ELLIPSIS) {
					switch (oper) {
					case 0:
						if (a2 && k2==0) break;
						return 1;
					case ASSIGN:
						if (a2 && k2==0) break;
						return 1;
					case ARG:
						if (a1) return 1;
						break;
					case OVERLOAD:
					case COERCE:
						return 1;
					}
				}
				else if (k2 == ELLIPSIS) {
					return 1;
				}
				else if (k1 || k2) {
					return 1;
				}
			}
			t1 = f1->returns;
			t2 = f2->returns;
		}
			break;

		case FIELD:
			goto field_check;
		case CHAR:
		case SHORT:
		case INT:
		case LONG:
			goto int_check;
		case FLOAT:
		case DOUBLE:
			goto float_check;
		case EOBJ:
			goto enum_check;
		case COBJ:
			goto cla_check;
		case ZTYPE:
		case VOID:
			return 0;
		default:
			error('i',"T::check(o=%d %d %d)",oper,b1,b2);
		}
	}

	if (t1 || t2) return 1;
	return 0;

field_check:
	switch (oper) {
	case 0:
	case ARG:
		error('i',"check field?");
	}
	return 0;

float_check:
	if (first==0 && b1!=b2 && b2!=ZTYPE) return 1;

int_check:
	if (Pbase(t1)->b_unsigned != Pbase(t2)->b_unsigned) {
		if (oper)
			Nstd++;
		else
			return 1;
	}
enum_check:
const_check:

	if ( !t1_const ) t1_const = t1->tconst();
	if ( !t2_const ) t2_const = t2->tconst();

	if ( (oper==0 && t1_const!=t2_const )
	|| (first==0 && t2_const && t1_const==0)) {
		const_problem= 1;
		return 1;
	}
	return 0;

cla_check:
	{	Pbase c1 = (Pbase)t1;
		Pbase c2 = (Pbase)t2;
		Pname n1 = c1->b_name;
		Pname n2 = c2->b_name;
//error('d',"c1 %d c2 %d n1 %d %s n2 %d %s oper %d first %d",c1,c2,n1,n1->string,n2,n2->string,oper,first);
		if (n1 == n2) goto const_check;
		if (first) return 1;

		switch (oper) {
		case 0:
		case OVERLOAD:
			return 1;
		case ARG:
		case ASSIGN:
		case RETURN:
		case COERCE:
		{
			/*	is c2 derived from c1 ? */
			Pname b = n2;
			Pclass cl;
			while (b) {
				cl = (Pclass) b->tp;
				b = cl->clbase;
//if (b)fprintf(stderr,"n2=(%d %s) b=(%d %s) n1=(%d %s) pub %d\n",n2,n2->string,b,b->string,n1,n1->string,cl->pubbase); else fprintf(stderr,"b==0\n");
				if (b && cl->pubbase==0) return 1;
				if (b == n1) {
					Nstd++;
					goto const_check;
				}
			}
			return 1;
		}
		}
	}
	goto const_check;

base_check:
//error('d',"base_check t1=%t t2=%t oper=%d %s",t1,t2,oper,first?"first":"");
	if (oper)
	if (first) {
		if (b1==VOID || b2==VOID) return 1;
	}
	else {
		if (b1 == VOID) {		// check for void* = T*
			register Ptype tx = this;
		txloop:
			switch (tx->base) {	// t1 == void*
			default:	return 1;
			case VOID:	break;
			case PTR:
			case VEC:	tx = Pvec(tx)->typ; goto txloop;
			case TYPE:	tx = Pbase(tx)->b_name->tp; goto txloop;
			}

			tx = t;
		bloop:
			switch (tx->base) {	// t2 == T*
			default:	return 1;
			case VEC:
			case PTR:
			case FCT:	Nstd++; goto const_check;
			case TYPE:	tx = Pbase(tx)->b_name->tp; goto bloop;
			}
		}
		if (b2 != ZTYPE) return 1;
	}

	switch (oper) {
	case 0:
	case OVERLOAD:
		return 1;
//	case OVERLOAD:
	case COERCE:
		switch (b1) {
		case EOBJ:
		case ZTYPE:
		case CHAR:
		case SHORT:
		case INT:
			switch (b2) {
			case EOBJ:
			case ZTYPE:
			case CHAR:
			case SHORT:
			case INT:
			case FIELD:
				goto const_check;
			}
			return 1;
		case LONG:	/* char, short, and int promotes to long */
			switch (b2) {
			case ZTYPE:
			case EOBJ:
			case CHAR:
			case SHORT:
			case INT:
			case FIELD:
				Nstd++;
				goto const_check;
			}
			return 1;
		case FLOAT:
			switch (b2) {
			case ZTYPE:
				Nstd++;
			case FLOAT:
			case DOUBLE:
				goto const_check;
			}
			return 1;
		case DOUBLE:	// char, short, int, and float promotes to double
			switch (b2) {
			case ZTYPE:
			case EOBJ:
			case CHAR:
			case SHORT:
			case INT:
				Nstd++;
			case FLOAT:
			case DOUBLE:
				goto const_check;
			}
			return 1;
		case PTR:
			switch (b2) {
			case ZTYPE:
				Nstd++;
				goto const_check;
			}
		case RPTR:
		case VEC:
		case COBJ:
		case FCT:
			return 1;
		}
	case ARG:
	case ASSIGN:
	case RETURN:
		switch (b1) {
		case COBJ:
			return 1;
		case EOBJ:
		case ZTYPE:
		case CHAR:
		case SHORT:
		case INT:
		case LONG:
			suppress_error++;
			r = t2->num_ptr(ASSIGN);
			suppress_error--;
			switch (r) {
			case F:		error('w',"%t assigned to%t",t2,t1); break;
			case A:
			case P:
			case FCT:	return 1;
			}
			break;
		case FLOAT:
		case DOUBLE:
			suppress_error++;
			r = t2->numeric(ASSIGN);
			suppress_error--;
			break;
		case VEC:
			return 1;
		case PTR:
			suppress_error++;
			r = t2->num_ptr(ASSIGN);
			suppress_error--;
			switch (r) {
			case A:
			case I:
			case F:	return 1;
			case FCT:	if (Pptr(t1)->typ->base != FCT) return 1;
			}
			break;
		case RPTR:
			return 1;
		case FCT:
			switch (oper) {
			case ARG:
			case ASSIGN:
				return 1;
			}
		}	
		break;
	}
	goto const_check;
}

