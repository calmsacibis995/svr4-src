/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/typ2.c	1.1"
/*ident	"@(#)cfront:src/typ2.c	1.7" */
/**************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


typ2.c:

***************************************************************************/
 
#include "cfront.h"
#include "size.h"

extern int chars_in_largest;

void typ_init()
{	
	chars_in_largest = strlen(LARGEST_INT);

	defa_type = int_type = new basetype(INT,0);
	PERM(int_type);

	moe_type = new basetype(INT,0);
	PERM(moe_type);
	moe_type->b_const = 1;
	moe_type->check(0);

	uint_type = new basetype(INT,0);
	PERM(uint_type);
	uint_type->type_adj(UNSIGNED);
	uint_type->check(0);

	long_type = new basetype(LONG,0);
	PERM(long_type);
	long_type->check(0);

	ulong_type = new basetype(LONG,0);
	PERM(ulong_type);
	ulong_type->type_adj(UNSIGNED);
	ulong_type->check(0);

	short_type = new basetype(SHORT,0);
	PERM(short_type);
	short_type->check(0);

	ushort_type = new basetype(SHORT,0);
	PERM(ushort_type);
	ushort_type->type_adj(UNSIGNED);
	ushort_type->check(0);

	float_type = new basetype(FLOAT,0);
	PERM(float_type);

	double_type = new basetype(DOUBLE,0);
	PERM(double_type);

	zero_type = new basetype(ZTYPE,0);
	PERM(zero_type);
	zero->tp = zero_type;

	void_type = new basetype(VOID,0);
	PERM(void_type);

	char_type = new basetype(CHAR,0);
	PERM(char_type);

	uchar_type = new basetype(CHAR,0);
	PERM(uchar_type);
	uchar_type->type_adj(UNSIGNED);
	uchar_type->check(0);

	Pchar_type = new ptr(PTR,char_type,0);
	PERM(Pchar_type);

	Pint_type = new ptr(PTR,int_type,0);
	PERM(Pint_type);

	Pvoid_type = new ptr(PTR,void_type,0);
	PERM(Pvoid_type);

	Pfctchar_type = new fct( char_type, 0, 0 );
	Pfctchar_type = new ptr( PTR, Pfctchar_type, 0 );
	PERM(Pfctchar_type);

	Pfctvec_type = new fct(int_type,0,0);	// must be last, see basetype::normalize()
	Pfctvec_type = new ptr(PTR,Pfctvec_type,0);
	Pfctvec_type = new ptr(PTR,Pfctvec_type,0);
	PERM(Pfctvec_type);

	any_tbl = new table(TBLSIZE,0,0);
	gtbl = new table(GTBLSIZE,0,0);
	gtbl->t_name = new name("global");
}

Pbase basetype::arit_conv(Pbase t)
/*
	perform the "usual arithmetic conversions" C ref Manual 6.6
	on "this" op "t"
	"this" and "t" are integral or floating
	"t" may be 0
*/
{

	while ( base == TYPE ) this = Pbase(Pbase(this)->b_name->tp);
	while ( t && t->base == TYPE ) t = Pbase(Pbase(t)->b_name->tp);

	bit l;
	bit u;
	bit f;
	bit l1 = (base == LONG);
	bit u1 = b_unsigned;
	bit f1 = (base==FLOAT || base==DOUBLE);
	if (t) {
		bit l2 = (t->base == LONG);
		bit u2 = t->b_unsigned;
		bit f2 = (t->base==FLOAT || t->base==DOUBLE);
		l = l1 || l2;
		u = u1 || u2;
		f = f1 || f2;
	}
	else {
		l = l1;
		u = u1;
		f = f1;
	}

	if (f)		return double_type;
	if (l & u)	return ulong_type;
	if (l & !u)	return long_type;
	if (u)		return uint_type;
			return int_type;
}

bit vec_const = 0;
bit fct_const = 0;

bit type::tconst()
/*
	is this type a constant
*/
{
	Ptype t = this;
	vec_const = 0;
	fct_const = 0;
xxx:
	switch (t->base) {
	case TYPE:	if (Pbase(t)->b_const) return 1;
			t = Pbase(t)->b_name->tp; goto xxx;
	case VEC:	vec_const = 1; return 1;
	case PTR:
	case RPTR:	return Pptr(t)->rdo;
	case FCT:
	case OVERLOAD:  fct_const = 1; return 1;
	default:	return Pbase(t)->b_const;
	}
}

TOK type::set_const(bit mode)
/*
	make someting a constant or variable, return 0 if no problem
*/
{
	Ptype t = this;
xxx:
	switch (t->base) {
	case TYPE:	Pbase(t)->b_const = mode;
			t = Pbase(t)->b_name->tp; goto xxx;
	case ANY:
	case RPTR:
	case VEC:	return t->base;		// constant by definition
	case PTR:	Pptr(t)->rdo = mode; return 0;
	default:	Pbase(t)->b_const = mode; return 0;
	}
}

int type::is_ref()
{
	Ptype t = this;
xxx:
	switch (t->base) {
	case TYPE:	t = Pbase(t)->b_name->tp; goto xxx;
	case RPTR:	return 1;
	default:	return 0;
	}
}

int type::align()
{
	Ptype t = this;
xx:
/*fprintf(stderr,"align %d %d\n",t,t->base);*/
	switch (t->base) {
	case TYPE:	t = Pbase(t)->b_name->tp; goto xx;
	case COBJ:	t = Pbase(t)->b_name->tp; goto xx;
	case VEC:	t = Pvec(t)->typ; goto xx;
	case ANY:	return 1;
	case CHAR:	return AL_CHAR;
	case SHORT:	return AL_SHORT;
	case INT:	return AL_INT;
	case LONG:	return AL_LONG;
	case FLOAT:	return AL_FLOAT;
	case DOUBLE:	return AL_DOUBLE;
	case PTR:
	case RPTR:	return AL_WPTR;
	case CLASS:	return Pclass(t)->obj_align;
	case ENUM:
	case EOBJ:	return AL_INT;
	case VOID:	error("illegal use of void"); return AL_INT;
	default:	error('i',"(%d,%k)->type::align",t,t->base);
	}
}

bit fake_sizeof;

int type::tsizeof()
/*
	the sizeof type operator
	return the size in bytes of the types representation
*/
{
	Ptype t = this;
zx:
	if (t == 0) error('i',"typ.tsizeof(t==0)");
	switch (t->base) {
	case TYPE: 
	case COBJ:	t = Pbase(t)->b_name->tp; goto zx;
	case ANY:	return 1;
	case VOID:	return 0;
	case ZTYPE:	return SZ_WPTR;	/* assume pointer */
	case CHAR:	return SZ_CHAR;
	case SHORT:	return SZ_SHORT;
	case INT:	return SZ_INT;
	case LONG:	return SZ_LONG;
	case FLOAT:	return SZ_FLOAT;
	case DOUBLE:	return SZ_DOUBLE;
	case VEC:
		{	Pvec v = Pvec(t);
			if (v->size == 0) {
				if (fake_sizeof == 0) error('w',"sizeof vector with undeclared dimension");
				return SZ_WPTR;	// vector argument has sizeof ptr
			}
			return v->size * v->typ->tsizeof();
		}
	case PTR:
	case RPTR:
		t = Pptr(t)->typ;
	xxx:
		switch (t->base) {
		default:	return SZ_WPTR;
		case CHAR:	return SZ_BPTR;
		case TYPE:	t = Pbase(t)->b_name->tp; goto xxx;
		}
	case FIELD:
	{	Pbase b = (Pbase)t;
		return b->b_bits/BI_IN_BYTE+1;
	}
	case CLASS:	
	{	Pclass cl = (Pclass)t;
		int sz = cl->obj_size;
		if ((cl->defined&(DEFINED|SIMPLIFIED)) == 0) {
			error("%sU, size not known",cl->string);
			return SZ_INT;
		}
		return sz;
	}
	case EOBJ:
	case ENUM:	return SZ_INT;
	default:	error('i',"sizeof(%d)",t->base);
	}
}

bit type::vec_type()
{
	Ptype t = this;
xx:
	switch (t->base) {
	case ANY:
	case VEC:
	case PTR:
	case RPTR:	return 1;
	case TYPE:	t = Pbase(t)->b_name->tp; goto xx;
	default:	return 0;
	}
}

Ptype type::deref()
/*	index==1:	*p
	index==0:	p[expr]
*/
{
	Ptype t = this;
xx:
	switch (t->base) {
	case TYPE:
		t = Pbase(t)->b_name->tp;
		goto xx;
	case PTR:
	case RPTR:
	case VEC:
		if (t == Pvoid_type) error("void* deRd");
		return Pvec(t)->typ;
	case ANY:
		return t;
	default:
		error("nonP deRd");
		return any_type;
	}
}
