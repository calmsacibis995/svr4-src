/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/norm2.c	1.1"
/*ident	"@(#)cfront:src/norm2.c	1.6" */
/************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


norm2.c:

	"normalization" handles problems which could have been handled
	by the syntax analyser; but has not been done. The idea is
	to simplify the grammar and the actions associated with it,
	and to get a more robust error handling

****************************************************************************/

#include "cfront.h"
#include "size.h"
const NBITE = (CHUNK-8)/sizeof(name)-1;
const EBITE = (CHUNK-8)/sizeof(expr)-1;
const SBITE = (CHUNK-8)/sizeof(stmt)-1;

fct::fct(Ptype t, Pname arg, TOK known)
{
	Nt++;
	base = FCT;
	nargs_known = known;
	returns = t;
	argtype = arg; 
/*fprintf(stderr,"fct t %d %d arg %d %d -> %d\n",t, t?t->base:0, arg, arg?arg->base:0, this);*/

	if (arg==0 || arg->base==ELIST) return;

	register Pname n;
	for (n=arg; n; n=n->n_list) {
		switch (n->tp->base) {
		case VOID:
			argtype = 0;
			nargs = 0;
			nargs_known = 1;
			if (n->string)
				error("voidFA%n",n);
			else if (nargs || n->n_list) {
				error("voidFA");
				nargs_known = 0;
			}
			break;
		case CLASS:
		case ENUM:
			break;
		default:
			nargs++;
		}
	}
}

Pexpr expr_free;

expr::expr(TOK ba, Pexpr a, Pexpr b)
{
	register Pexpr p;

	if (this) goto ret;

	if ( (p=expr_free) == 0 ) {
		register Pexpr q = (Pexpr) chunk(1);
		for (p=expr_free=&q[EBITE-1]; q<p; p--) p->e1 = p-1;
		(p+1)->e1 = 0;
	}
	else
		expr_free = p->e1;

	this = p;

	permanent = 0;
	tp = 0;
	tp2 = 0;

	Ne++;	// excluding names
ret:
	base = ba;
	e1 = a;
	e2 = b;
}

expr::~expr()
{
	NFe++;
	e1 = expr_free;
	expr_free = this;
	this = 0;
}

Pstmt stmt_free;

stmt::stmt(TOK ba, loc ll, Pstmt a)
{
	register Pstmt p;

	if ( (p=stmt_free) == 0 ) {
		register Pstmt q = (Pstmt) chunk(1);
		for (p=stmt_free=&q[SBITE-1]; q<p; p--) p->s_list = p-1;
		(p+1)->s_list = 0;
	}
	else
		stmt_free = p->s_list;

	this = p;

	permanent = 0;
	e = e2 = 0;
	memtbl = 0;
	else_stmt = 0;
	s_list = 0;

	Ns++;
	base = ba;
	where = ll;
	s=a;
}

stmt::~stmt()
{
	NFs++;
	s_list = stmt_free;
	stmt_free = this;
	this = 0;
}

classdef::classdef(TOK b)
{
	base = CLASS;
	csu = b;
	memtbl = new table(CTBLSIZE,0,0);
}

basetype::basetype(TOK b, Pname n)
{
	Nbt++;
	switch (b) {
	case 0:				break;
	case TYPEDEF:	b_typedef = 1;	break;
	case INLINE:	b_inline = 1;	break;
	case VIRTUAL:	b_virtual = 1;	break;
	case CONST:	b_const = 1;	break;
	case UNSIGNED:	b_unsigned = 1;	break;
	case FRIEND:
	case OVERLOAD:
	case EXTERN:
	case STATIC:
	case AUTO:
	case REGISTER:	b_sto = b;	break;
	case SHORT:	b_short = 1;	break;
	case LONG:	b_long = 1;	break;
	case ANY:
	case ZTYPE:
	case VOID:
	case CHAR:
	case INT:
	case FLOAT:
	case DOUBLE:	base = b; 	break;
	case TYPE:
	case COBJ:
	case EOBJ:
	case FIELD:
	case ASM:
		base = b;
		b_name = n;
		break;
	case SIGNED:
	case VOLATILE:
		error('w',"\"%k\" not implemented (ignored)",b);
		break;
	default:
		error('i',"badBT:%k",b);
	}
}

Pname name_free;

name::name(char* s) : (NAME,0,0)
{
	register Pname p;

	if ( (p=name_free) == 0 ) {
		register Pname q = (Pname) chunk(1);
		for (p=name_free=&q[NBITE-1]; q<p; p--) p->n_tbl_list = p-1;
		(p+1)->n_tbl_list = 0;
	}
	else
		name_free = p->n_tbl_list;

	this = p;
//fprintf(stderr,"%d: new name %s %d ll %d bl %d\n",this,s,base,lex_level,bl_level);

	Nn++;
	string = s;
	where = curloc;
	lex_level = bl_level;

	// beware of alignment differences & pointer-zeros that are not int-zeros
	tp = 0;
	n_initializer = 0;
	n_table = 0;
	n_oper = 0;
	n_sto = 0;
	n_stclass = 0;
	n_scope = 0;
	n_union = 0;
	n_evaluated = 0;
	n_xref = 0;
	n_protect = 0;
	n_addr_taken = 0;
	n_used = 0;
	n_assigned_to = 0;
	n_val = 0;
	n_offset = 0;
	n_list = 0;
	n_tbl_list = 0;
	n_qualifier = 0;
}

name::~name()
{
	NFn++;
/*fprintf(stderr,"delete %d: %s %d\n",this,string,base);*/
	n_tbl_list = name_free;
	name_free = this;
	this = 0;
}


nlist::nlist(Pname n)
{
//	if (n == 0) error('i',"nlist.nlist(0)");

	head = n;
	for (Pname nn=n; nn->n_list; nn=nn->n_list);
	tail = nn;
	Nl++;
}

void nlist::add_list(Pname n)
{
	if (n->tp && (n->tp->defined & IN_ERROR)) return;

	tail->n_list = n;
	for (Pname nn=n; nn->n_list; nn=nn->n_list);
	tail = nn;
}

int NFl;

Pname name_unlist(Pnlist l)
{
	if (l == 0) return 0;
	Pname n = l->head;
	NFl++;
//fprintf(stderr,"name_unlist %d\n",l);
	delete l;
	return n;
}

Pstmt stmt_unlist(Pslist l)
{
	if (l == 0) return 0;
	Pstmt s = l->head;
	NFl++;
//fprintf(stderr,"stmt_unlist %d\n",l);
	delete l;
	return s;
}

Pexpr expr_unlist(Pelist l)
{
	if (l == 0) return 0;
	Pexpr e = l->head;
	NFl++;
//fprintf(stderr,"expr_unlist %d\n",l);
	delete l;
	return e;
}

void sig_name(Pname n)
{
	static char buf[256];
	buf[0] = '_';
	buf[1] = 'O';
	char* p = n->tp->signature(buf+2);
	if (255 < p-buf) error('i',"sig_name():N buffer overflow");
	n->string = buf;
	n->tp = 0;
}

Ptype tok_to_type(TOK b)
{
	Ptype t;
	switch (b) {
	case CHAR:	t = char_type; break;
	case SHORT:	t = short_type; break;
	case LONG:	t = long_type; break;
	case UNSIGNED:	t = uint_type; break;
	case FLOAT:	t = float_type; break;
	case DOUBLE:	t = double_type; break;
	case VOID:	t = void_type; break;
	default:	error("illegalK:%k",b);
	case INT:	t = int_type;
	}
	return t;
}

Pbase defa_type;
Pbase moe_type;
Pexpr dummy;
Pexpr zero;

Pclass ccl;
Plist modified_tn = 0;

static name sta_name_dummy;
Pname sta_name = &sta_name_dummy;

TOK back;

void memptrdcl(Pname bn, Pname tn, Ptype ft, Pname n)
{
	Pptr p = new ptr(PTR,0);
	p->memof = Pclass(Pbase(bn->tp)->b_name->tp);
	Pbase b = new basetype(TYPE,tn);
	PERM(p);
	Pfct f = Pfct(ft);
	Ptype t = n->tp;
	if (t) {
		p->typ = t;
	ltlt:
		switch (t->base) {
		case PTR:
		case RPTR:
		case VEC:
			if (Pptr(t)->typ == 0) {
				Pptr(t)->typ = b;
				break;
			}
			t = Pptr(t)->typ;
			goto ltlt;
		default:
			error('s',"P toMFT too complicated");
		}
	}
	else
	p->typ = b;	
	f->returns = p;
	n->tp = f;
}
