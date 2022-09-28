/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/print.c	1.1"
/*ident	"@(#)cfront:src/print.c	1.17.1.11" */
/**************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


print.c:

	print the output of simpl, typ, or syn in a form suitable for cc input

****************************************************************************/

#include "cfront.h"
#include "size.h"

extern FILE* out_file;
char emode = 0;
extern int ntok; 
int ntok;
bit Cast;
Pin curr_icall;
static int last_ll = 1;

int MAIN;	// fudge to get _main() called by main()

void puttok(TOK t)
/*
	print the output representation of "t"
*/
{
//	if (t<=0 || MAXTOK<t) error("illegal token %d",t);
//	char* s = keys[t];
//	if (s == 0) error("V representation token %d",t);
	if ( keys[ t ] ) putstring(keys[t]);
	if (12<ntok++) {
		ntok = 0;
		last_line.putline();
	}
	else if (t == SM) {
		ntok = 0;
		putch('\n');
		if (last_ll) last_line.line++;
	}
	else
		putch(' ');
}

#define MX	20
#define NTBUF	10
class dcl_buf {
	/*
		buffer for assembling declaration (or cast)
		left contains CONST_PTR	=> *CONST
			     CONST_RPTR => &CONST
				PTR	=> *
				RPTR	=> &
				LP	=> (
		right contains	RP	=> )
				VEC	=> [ rnode ]
				FCT	=> ( rnode )
				FIELD	=> : rnode
	*/
	Pbase b;
	Pname n;
	TOK left[MX], right[MX];
	Pnode	rnode[MX];
	Pclass	lnode[MX];
	int li, ri;
public:
	void	init(Pname nn)		{ b=0; n=nn; li=ri=0; }
	void	base(Pbase bb)		{ b = bb; }
	void	front(TOK t)		{ left[++li] = t; }
	void	front(Pclass c)		{ left[++li] = MEMPTR; lnode[li] = c; }
	void	back(TOK t, Pnode nod)	{ right[++ri] = t; rnode[ri] = nod; }
	void	paran() 		{ front(LP); back(RP,0); }
	void	put();
} *tbufvec[NTBUF] = {0}, *tbuf = 0;

int freetbuf = 0;

void dcl_buf::put()
{
	int i;

	if (MX<=li || MX<=ri) error('i',"T buffer overflow");
	if (b == 0) error('i',"noBT%s",Cast?" in cast":"");

	if (n && n->n_sto && n->n_sto!=REGISTER) puttok(n->n_sto);

	b->dcl_print();
	
	for( ; li; li--) {
		switch (left[li]) {
		case LP:
			putch('(');
			break;
		case PTR:
			putch('*');
			break;
		case RPTR:
			if (emode)
				putch('&');
			else
				putch('*');
			break;
		case CONST_PTR:
			if (emode)
				putstring("*const ");
			else
				putch('*');
			break;
		case CONST_RPTR:
			if (emode)
				putstring("&const ");
			else
				putch('*');
			break;
		case MEMPTR:
			if (lnode[li]) fprintf(out_file,"%s::",lnode[li]->string);
		}
	}

	if (n) n->print();

	for(i=1; i<=ri; i++)
		switch (right[i]) {
		case RP:
			putch(')');
			break;
		case VEC:
			putch('[');
			{	Pvec v = (Pvec) rnode[i];
				Pexpr d = v->dim;
				int s = v->size;
				if (d) d->print();
				if (s) fprintf(out_file,"%d",s);
			}
			putch(']');
			break;
		case FCT:
			Pfct(rnode[i])->dcl_print();
			break;
		case FIELD:
			{	Pbase f = (Pbase) rnode[i];
				Pexpr d = (Pexpr)f->b_name;
				int s = f->b_bits;
				putch(':');
				if (d) d->print();
				if (s) fprintf(out_file,"%d",s);
			}
			break;
		}
}

#define eprint(e) if (e) Eprint(e)

void Eprint(Pexpr e)
{
	switch (e->base) {
	case REF:
		if (Pref(e)->mem->tp->base == FCT) {
//error('d',"ref fct");
			// Yuk: suppress ``this'' in ``this->f''
			Pref(e)->mem->print();
			break;
		}
	case NAME:
	case ID:
	case ZERO:
	case ICON:
	case CCON:
	case FCON:
	case STRING:
	case IVAL:
	case TEXT:
	case CM:
	case G_CM:
	case ELIST:
	case COLON:
	case ILIST:
	case DOT:
	case THIS:
	case CALL:
	case G_CALL:
	case ICALL:
	case ANAME:
		e->print();
	case DUMMY:
		break;
	default:
		putch('(');
		e->print();
		putch(')');
	}
}

void name::dcl_print(TOK list)
/*
	Print the declaration for a name (list==0) or a name list (list!=0):
		For each name
		(1) print storage class
		(2) print base type
		(3) print the name with its declarators
	Avoid (illegal) repetition of basetypes which are class or enum declarations
	(A name list may contain names with different base types)
	list == SM :	terminator SM
	list == 0:	single declaration with terminator SM
	list == CM :	separator CM
*/
{
	Pname n;

	if (this == 0) error("0->N::dcl_print()");

	for (n=this; n; n=n->n_list) {
		Ptype t = n->tp;
		int sm = 0;

		if (t == 0) error('i',"N::dcl_print(%n)T missing",n);
		if (n->n_stclass==ENUM) continue;

		if (n->where.line!=last_line.line)
			if (last_ll = n->where.line)
				n->where.putline();
			else
				last_line.putline();
	
		int tc = Pbase(t)->b_const;
		for (Ptype tt = t; tt->base==TYPE; tt = Pbase(tt)->b_name->tp)
			tc |= Pbase(tt)->b_const;

		switch (t->base) {
		case CLASS:
			if (n->base != TNAME) {
				Pclass(t)->dcl_print(n);
				sm = 1;
			}
			break;

		case ENUM:
			Penum(t)->dcl_print(n);
			sm = 1;
			break;

		case FCT:
		{	Pfct f = (Pfct) t;
			if (n->base == TNAME) puttok(TYPEDEF);

			if (f->f_inline) {
				if (f->f_virtual || n->n_addr_taken) {
					TOK st = n->n_sto;
					Pblock b = f->body;
					f->body = 0;
				/*	n->n_sto = 0;	*/
					t->dcl_print(n);
					n->n_sto = st;
					f->body = b;
				}
				else
					sm = 1; // no SM
			}
			else if (n->n_table==gtbl && strcmp(n->string,"main")==0) {
				MAIN = 1;
				gtbl->look("main",0)->use();
				t->dcl_print(n);
				MAIN = 0;
			}
			else
				t->dcl_print(n);
			break;
		}

		case OVERLOAD:
		{	Pgen g = (Pgen) t;
			Plist gl;
			fprintf(out_file,"\t/* overload %s: */\n",g->string);
			for (gl=g->fct_list; gl; gl=gl->l) {
				Pname nn = gl->f;
				nn->dcl_print(0);
				sm = 1;
			}
			break;
		}

		case ASM:
			fprintf(out_file,"asm(\"%s\")\n",(char*)Pbase(t)->b_name);
			break;

		case INT:
		case CHAR:
		case LONG:
		case SHORT:
		tcx:
			// do not allocate space for constants unless necessary
			if (tc
			&& n->n_sto!=EXTERN	// extern const one;
						// const one = 1;
						// allocates storage
			&& (n->n_scope==EXTERN	// FUDGE const one = 1;
						// is treated as static
						// need loader support
				|| n->n_scope==STATIC
				|| n->n_scope==FCT)
			) {
				if (n->n_evaluated) {
					sm = 1;	/* no ; */
					break;
				}
			}
			tc = 0;
			// no break;

		default:
		{	Pexpr i = n->n_initializer;

			if (tc) {
				switch (tt->base) {
				case CHAR:
				case SHORT:
				case INT:
				case LONG:
					goto tcx;
				}
			}
			if (n->base == TNAME) puttok(TYPEDEF);
			if (n->n_stclass == REGISTER) {
				// (imperfect) check against member functions
				// register s a; a.f() illegal
				Pname cln = n->tp->is_cl_obj();
				if (cln) {
					Pclass cl = Pclass(cln->tp);
					if (cl->csu!=CLASS
					&& cl->clbase==0
					&& cl->itor==0
					&& cl->virt_count==0) puttok(REGISTER);
				}
				else
					puttok(REGISTER);
			}

			if (i) {
				if (n->n_sto==EXTERN && n->n_stclass==STATIC) {
					n->n_initializer = 0;
					t->dcl_print(n);
					puttok(SM);
					n->n_initializer = i;
					n->n_sto = 0;
					t->dcl_print(n);
					n->n_sto = EXTERN;
				}
				else
					t->dcl_print(n);
			}
			else if (n->n_evaluated && Pbase(t)->b_const) {
				if (n->n_sto==EXTERN && n->n_stclass==STATIC) {
					int v = n->n_evaluated;
					n->n_evaluated = 0;
					t->dcl_print(n);
					puttok(SM);
					n->n_evaluated = v;
					n->n_sto = 0;
					t->dcl_print(n);
					n->n_sto = EXTERN;
				}
				else
					t->dcl_print(n);
			}
			else {
				if (fct_void==0
				&& n->n_sto==0
				&& n_stclass==STATIC
				&& n->n_table==gtbl) {
					switch (t->base) {
					case CHAR:
					case SHORT:
					case INT:
					case LONG:
					case FLOAT:
					case DOUBLE:
					case EOBJ:
					case PTR:
						// "int a;" == "int a = 0;"
						n->n_initializer = i = zero;
					}
				}
				t->dcl_print(n);
			}

			if (n->n_scope!=ARG) {
				if (i) {
					puttok(ASSIGN);
					if (t!=i->tp
					&& i->base!=ZERO
					&& i->base!=ILIST /*&& i->tp!=Pchar_type*/) {
						Ptype t1 = n->tp;
					cmp:
						switch (t1->base) {
						default:
							i->print();
							break;
						case TYPE:	
							t1 = Pbase(t1)->b_name->tp;
							goto cmp;
						case VEC:
							if (Pvec(t1)->typ->base==CHAR) {
								i->print();
								break;
							}
						case PTR:
						case RPTR:
							if (i->tp==0 || n->tp->check(i->tp,0)) {
								putch('(');
								bit oc = Cast;
								Cast = 1;
								t->print();
								Cast = oc;
								putch(')');
							}
							eprint(i);
						}
					}
					else
						i->print();
				}
				else if (n->n_evaluated) {
					puttok(ASSIGN);
					if (n->tp->base != INT) {
						putstring("((");
						bit oc = Cast;
						Cast = 1;
						n->tp->print();
						Cast = oc;
						fprintf(out_file,")%d)",n->n_val);
					}
					else
						fprintf(out_file,"%d",n->n_val);
				}
			}
		}
		}

		switch (list) {
		case SM:
			if (sm==0) puttok(SM);
			break;
		case 0:
			if (sm==0) puttok(SM);
			return;
		case CM:
			if (n->n_list) puttok(CM);
			break;
		}
	}
} 

void name::print()
/*
	print just the name itself
*/
{
	if (this == 0) error('i',"0->N::print()");

	if (string == 0) {
		if (emode) putch('?');
		return;
	}

	switch (base) {
	default:
		error('i',"%p->N::print() base=%d",this,base);
	case TNAME:
		putst(string);
		return;
	case NAME:
	case ANAME:
		break;
	}

	if (emode) {
		Ptable tbl;
		char* cs = 0;
		bit f = 0;
		if (tp) {
			switch (tp->base) {
			case OVERLOAD:
			case FCT:
				f = 1;
			default:
				if (tbl=n_table) {
					if (tbl == gtbl) {
						if (f == 0) putstring("::");
					}
					else {
						if (tbl->t_name) {
							cs = tbl->t_name->string;
							fprintf(out_file,"%s::",cs);
						}
					}
				}

				if (n_scope==ARG && strcmp(string,"this")==0) {
					// tell which "this" it is
					Ptype tt = Pptr(tp)->typ;
					Pname cn = Pbase(tt)->b_name;
					fprintf(out_file,"%s::",cn->string);
				}

			case CLASS:
			case ENUM:
		//	case TYPE:
				break;
			}
			switch (n_oper) {
			case TYPE:
				putstring("operator ");
				Pfct(tp)->returns->dcl_print(0);
				break;
			case 0:
				putstring(string);
				break;
			case DTOR:
				putch('~');
			case CTOR:
				if (cs)
					putstring(cs);
				else {
					putstring("constructor");
					f = 0;
				}
				break;
			case TNAME:
				putstring(string);
				break;
			default:
				putstring("operator ");
				putstring(keys[n_oper]);
				break;
			}
			if (f) putstring("()");
		}
		else
			if (string) putstring(string);
		return;
	}
	
	if (tp) {
		Ptable tbl;
		int i = n_union;

		switch (tp->base) {
		default:
			if (tbl=n_table) {
				Pname tn;
				if (tbl == gtbl) {
					if (i) fprintf(out_file,"_O%d.__C%d_",i,i);
					break;
				}
				if (tn=tbl->t_name) {
					if (i)
						fprintf(out_file,"_%s__O%d.__C%d_",tn->string,i,i);
					else
						fprintf(out_file,"_%s_",tn->string);
					break;
				}
			}

			switch (n_stclass) {
			case STATIC:
			case EXTERN:
				if (i)
					fprintf(out_file,"_O%d.__C%d_",i,i);
				else if (n_sto==STATIC && tp->base!=FCT)
//					putstring("_static_");
					fprintf(out_file,"_st%d_",lex_level);
				break;
			default:
				if (i)
// lex_level of element is 1 greater than type
					fprintf(out_file,"_au%d__O%d.__C%d_",lex_level-1,i,i);
				else
//					putstring("_auto_");
					fprintf(out_file,"_au%d_",lex_level);
			}
			break;
		case CLASS:
		case ENUM:
			break;
		}
	}

	if (string) putst(string);
}


void type::print()
{
	switch (base) {
	case PTR:
	case RPTR:
		Pptr(this)->dcl_print(0);
		break;
	case FCT:
		Pfct(this)->dcl_print();
		break;
	case VEC:
		Pvec(this)->dcl_print(0);
		break;
	case CLASS:
	case ENUM:
		if (emode)
			fprintf(out_file,"%s",base==CLASS?"class":"enum");
		else
			error('i',"%p->T::print(%k)",this,base);
		break;
	case TYPE:
		if (Cast) {
			if (Pbase(this)->b_name->tp->base == PTR)
				{ Pbase(this)->b_name->print(); break; }
			Pbase(this)->b_name->tp->print();
			break;
		}
	default:
		Pbase(this)->dcl_print();
	}
}

char* type::signature(register char* p)
/*
	take a signature suitable for argument types for overloaded
	function names
*/
{
#define SDEL	'_'

	Ptype t = this;
	int pp = 0;

xx:
//error('d',"xx(%d) %d %k",this,t,t->base);
	switch (t->base) {
	case TYPE:	t = Pbase(t)->b_name->tp;	goto xx;
	case PTR: 	*p++ = 'P';	t = Pptr(t)->typ;	pp=1;	goto xx;
	case RPTR:	*p++ = 'R';	t = Pptr(t)->typ;	pp=1;	goto xx;
	case VEC:	*p++ = 'V';	t = Pvec(t)->typ;	pp=1;	goto xx;
	case FCT:
	{	Pfct f = (Pfct)t;
		*p++ = 'F';
	//	if (f->f_result) {			// 'T' 'P' resulttype '_'
	//		*p++ = 'T';
	//		p = f->f_result->tp->signature(p);
	//		*p++ = SDEL;
	//	}
		if (pp) {				// 'T' result type '_'
			*p++ = 'T';
			p = f->returns->signature(p);
			*p++ = SDEL;
		}
		for (Pname n=f->argtype; n; n=n->n_list) {	// argtype '_'
			if (n->n_xref) *p++ = 'X';
			p = n->tp->signature(p);
			*p++ = SDEL;
		}
		*p++ = SDEL;
		if (f->nargs_known == ELLIPSIS) *p++ = 'E';
		*p = 0;
		return p;
	}
	}

	if ( Pbase(t)->b_unsigned ) *p++ = 'U';

	switch (t->base) {
	case ANY:	*p++ = 'A';	break;
	case ZTYPE:	*p++ = 'Z';	break;
	case VOID:	*p++ = 'V';	break;
	case CHAR:	*p++ = (pp)?'C':'I';	break;
	case SHORT:	*p++ = (pp)?'S':'I';	break;
	case EOBJ:
	case INT:	*p++ = 'I';	break;
	case LONG:	*p++ = 'L';	break;
	case FLOAT:	*p++ = 'F';	break;
	case DOUBLE:	*p++ = 'D';	break;
	case COBJ:	*p++ = 'C';
			strcpy(p,Pbase(t)->b_name->string);
			while (*p++) ;
			*(p-1) = SDEL;
			break;
	case FIELD:
	default:
		error('i',"signature of %k",t->base);
	}

	*p = 0;
	return p;
}

void basetype::dcl_print()
{
	Pname nn;
	Pclass cl;

	if (emode) {
		if (b_virtual) puttok(VIRTUAL);
		if (b_inline) puttok(INLINE);
		if (b_const) puttok(CONST);
	}
	if (b_unsigned) puttok(UNSIGNED);

	switch (base) {
	case ANY:
		putstring("any ");
		break;

	case ZTYPE:
		putstring("zero ");
		break;

	case VOID:
		if (emode == 0) {
			puttok(CHAR);
			break;
		}
	case CHAR:
	case SHORT:
	case INT:
	case LONG:
	case FLOAT:
	case DOUBLE:
		puttok(base);
		break;

	case EOBJ:
		nn = b_name;
	eob:
		if (emode == 0)
			puttok(INT);
		else {
			puttok(ENUM);
			nn->print();
		}
		break;

	case COBJ:
		nn = b_name;
	cob:
		cl = (Pclass)nn->tp;
		switch (cl->csu) {
		case UNION:
		case ANON:	puttok(UNION); break;
		default:	puttok(STRUCT);
		}
		putst(cl->string);
		break;

	case TYPE:
		if (emode == 0) {
			switch (b_name->tp->base) {
			case COBJ:
				nn = Pbase(b_name->tp)->b_name;
				goto cob;
			case EOBJ:
				nn = Pbase(b_name->tp)->b_name;
				goto eob;
			}
		}
		b_name->print();
		break;

	default:
		if (emode) {
			if (0<base && base<=MAXTOK && keys[base])
				fprintf(out_file," %s",keys[base]);
			else
				putch('?');
		}
		else
			error('i',"%p->BT::dcl_print(%d)",this,base);
	}
}

void type::dcl_print(Pname n)
/*
	"this" type is the type of "n". Print the declaration
*/
{
	Ptype t = this;
	Pfct f;
	Pvec v;
	Pptr p;
	TOK pre = 0;

	if (t == 0) error('i',"0->dcl_print()");
	if (n && n->tp!=t) error('i',"not %n'sT (%p)",n,t);

	if (base == OVERLOAD) {
		if (emode) {
			puttok(OVERLOAD);
			return;
		}
		Pgen g = (Pgen) this;
		Plist gl;
		fprintf(out_file,"\t/* overload %s: */\n",g->string);
		for (gl=g->fct_list; gl; gl=gl->l) {
			Pname nn = gl->f;
			nn->tp->dcl_print(nn);
			if (gl->l) puttok(SM);
		}
		return;
	}

	tbuf = tbufvec[freetbuf];
	if (tbuf == 0) {
		if (freetbuf == NTBUF-1) error('i',"AT nesting overflow");
		tbufvec[freetbuf] = tbuf = new class dcl_buf;
	}
	freetbuf++;
	tbuf->init(n);
	if (n && n->n_xref) tbuf->front(PTR);

	while (t) {
		TOK k;

		switch (t->base) {
		case PTR:
			p = (Pptr)t;
			k = (p->rdo) ? CONST_PTR : PTR;
			goto ppp;
		case RPTR:
			p = (Pptr)t;
			k = (p->rdo) ? CONST_RPTR : RPTR;
		ppp:
			tbuf->front(k);
			if (emode && p->memof) tbuf->front(p->memof);
			pre = PTR;
			t = p->typ;
			break;
		case VEC:
			v = (Pvec)t;
			if (Cast) {
				tbuf->front(PTR);
				pre = PTR;
			}
			else {
				if (pre == PTR) tbuf->paran();
				tbuf->back(VEC,v);
				pre = VEC;
			}
			t = v->typ;
			break;
		case FCT:
			f = (Pfct)t;
			if (pre == PTR)
				tbuf->paran();
			else if (emode && f->memof)
				tbuf->front(f->memof);
			tbuf->back(FCT,f);
			pre = FCT;

			if ( f->f_inline &&
				f->returns &&
				f->returns->base == VOID &&
                                ( f->s_returns &&
				  f->s_returns->base != PTR) )
					t = f->returns;
			else
				t = (f->s_returns) ? f->s_returns : f->returns;
			break;
		case FIELD:
			tbuf->back(FIELD,t);
			tbuf->base( Pbase(Pbase(t)->b_fieldtype) );
			t = 0;
			break;
		case CLASS:
		case ENUM:
			error('i',"unexpected%k asBT",t->base);
		case 0:
			error('i',"noBT(B=0)");
		case TYPE:
			if (Cast) { // unravel type in case it contains vectors
				t = Pbase(t)->b_name->tp;
				break;
			}
		default: // the base has been reached
			tbuf->base( Pbase(t) );
			t = 0;
			break;
		} // switch
	} // while

	tbuf->put();
	freetbuf--;
}

void fct::dcl_print()
{
	Pname nn;

	if (emode) {
		putch('(');
		for (nn=argtype; nn;) {
			nn->tp->dcl_print(0);
			if (nn=nn->n_list) puttok(CM); else break;
		}
		switch (nargs_known) {
		case 0:		//	putst("?"); break;
		case ELLIPSIS:	puttok(ELLIPSIS); break;
		}
		putch(')');
		return;
	}

	Pname at = (f_this) ? f_this : (f_result) ? f_result : argtype; 
	putch('(');
	if (body && Cast==0) {

		for (nn=at; nn;) {
			nn->print();
			if (nn=nn->n_list) puttok(CM); else break;
		}
		putch(')');
	
		if (at) at->dcl_print(SM);

		if (MAIN) {
			putstring("{ _main(); ");
			body->print();
#ifdef apollo
			// Force linker to use C++ produces exit()
			putst(" exit(0);");
#endif
			putch('}');
		}
		else
			body->print();
	}
	else
		putch(')');
}

void classdef::print_members()
{
	int i;
	
	if (clbase) {
		Pclass bcl = (Pclass)clbase->tp;
		bcl->print_members();
	}

	for (Pname nn=memtbl->get_mem(i=1); nn; nn=memtbl->get_mem(++i)) {
		if (nn->base==NAME
		&& nn->n_union==0
		&& nn->tp->base!=FCT
		&& nn->tp->base!=OVERLOAD
		&& nn->tp->base!=CLASS
		&& nn->tp->base!=ENUM
		&& nn->n_stclass != STATIC) {
			Pexpr i = nn->n_initializer;
			nn->n_initializer = 0;
			nn->dcl_print(0);
			nn->n_initializer = i;
		}
	}
}

void classdef::dcl_print(Pname)
{ 
	Plist l;
	TOK c = csu==CLASS ? STRUCT : csu;

	int i;

	// ???
	for (Pname nn=memtbl->get_mem(i=1); nn; nn=memtbl->get_mem(++i) ) {
		if (nn->base==NAME && nn->n_union==0) {
			if (nn->tp->base == CLASS) Pclass(nn->tp)->dcl_print(nn);
		}
		else if ( nn->base == TNAME && Pbase(nn->tp)->base != COBJ )
			nn->dcl_print(0);
	}

	puttok(c);
	putst(string);

	if (c_body == 0) return;
	c_body = 0;

	int sm = 0;
	int sz = tsizeof();

	fprintf(out_file,"{\t/* sizeof %s == %d */\n",string,obj_size);
	//if (last_ll) last_line.line++;
	if (real_size) 
		print_members();
	else
		putstring("char _dummy; ");
	putstring("};\n");

	if (virt_count) {	/* print initialized jump-table */

		for (nn=memtbl->get_mem(i=1); nn; nn=memtbl->get_mem(++i) ) {
			if (nn->base==NAME && nn->n_union==0) {	/* declare function names */
				Ptype t = nn->tp;
				switch (t->base) {
				case FCT:
				{	Pfct f =(Pfct) t;
					if (f->f_virtual == 0) break;
					if (f->f_inline && vtbl_opt==-1) puttok(STATIC);
					f->returns->print();
					nn->print();
					putstring("()");
					puttok(SM);
					break;
				}
					case OVERLOAD:
					{	Pgen g = (Pgen)t;
						Plist gl;
						for (gl=g->fct_list; gl; gl=gl->l) {
							Pfct f = (Pfct) gl->f->tp;
							if (f->f_virtual == 0) continue;
							if (f->f_inline) puttok(STATIC);
							f->returns->print();
							gl->f->print();
							putstring("()");
							puttok(SM);
						}
					}
					}
				}
			}

			switch (vtbl_opt) {
			case -1:
				putstring("static ");
			case 1:
				fprintf(out_file,"int (*%s__vtbl[])() = {",string);
				for (i=0; i<virt_count; i++) {
					putstring("\n(int(*)()) ");
					virt_init[i]->print();
					puttok(CM);
				}
				putstring("0}");
				puttok(SM);
				break;
			case 0:
				fprintf(out_file,"extern int (*%s__vtbl[])();",string);
				break;
			}
                }

		for (nn=memtbl->get_mem(i=1); nn; nn=memtbl->get_mem(++i) ) {
			if (nn->base==NAME && nn->n_union==0) {
				Ptype t = nn->tp;
				switch (t->base) {
				case FCT:
				case OVERLOAD:
					break;
				default:
					if (nn->n_stclass == STATIC) {
						TOK b = nn->n_sto;	// silly
						nn->n_sto = 0;
						if (nn->tp->tconst()) {
							if ( nn->n_assigned_to ) 
     								nn->n_sto = STATIC; 
							else error( "uninitialized const%n", nn ); }
						nn->dcl_print(0);
						nn->n_sto = b;
					}
				}
			}
		}

		for (nn=memtbl->get_mem(i=1); nn; nn=memtbl->get_mem(++i) ) {
			if (nn->base==NAME && nn->n_union==0) {
				Pfct f = (Pfct)nn->tp;
				switch (f->base) {
			case FCT:
				/* suppress duplicate or spurious declaration */
				if (f->f_virtual || f->f_inline) break;
			case OVERLOAD:
				nn->dcl_print(0);
				}
			}
		}

		for (l=friend_list; l; l=l->l) {
			Pname nn = l->f;
			switch (nn->tp->base) {
			case FCT:
				Cast = 1;
				nn->dcl_print(0);
				Cast = 0;
				break;
			case OVERLOAD: /* first fct */
				l->f = nn = Pgen(nn->tp)->fct_list->f;
				nn->dcl_print(0);
				break;
			}
		}
}


void enumdef::dcl_print(Pname n)
{
	if (mem) {
		fprintf(out_file,"/* enum %s */\n",n->string);
		mem->dcl_print(SM);
	}
}

int addrof_cm = 0;

void expr::print()
{
	if (this == 0) error('i',"0->E::print()");
	if (this==e1 || this==e2) error('i',"(%p%k)->E::print(%p %p)",this,base,e1,e2);
	switch (base) {
	case NAME:
	{	Pname n = Pname(this);
		if (n->n_evaluated && n->n_scope!=ARG) {
			if (tp->base != INT) {
				putstring("((");
				bit oc = Cast;
				Cast = 1;
				tp->print();
				Cast = oc;
				fprintf(out_file,")%d)",n->n_val);
			}
			else
				fprintf(out_file,"%d",n->n_val);
		}
		else
			n->print();
		break;
	}
	case ANAME:
		if (curr_icall) {	// in expansion: look it up
			Pname n = Pname(this);
			int argno = n->n_val;

			for (Pin il=curr_icall; il; il=il->i_next)
				if (n->n_table == il->i_table) goto aok;
			goto bok;
		aok:
			if (n = il->local[argno]) {
				n->print();
			}
			else {
				Pexpr ee = il->arg[argno];
				Ptype t = il->tp[argno];
				if (ee==0 || ee==this) error('i',"%p->E::print(A %p)",this,ee);
				if (ee->tp==0
				|| (t!=ee->tp
					&& t->check(ee->tp,0)
					&& t->is_cl_obj()==0
					&& eobj==0)
				) {
					putstring("((");
					bit oc = Cast;
					Cast = 1;
					t->print();
					Cast = oc;
					putch(')');
					eprint(ee);
					putch(')');
				}
				else
					eprint(ee);
			}
		}
		else {
		bok:	/* in body: print it: */
			Pname(this)->print();
		}
		break;

	case ICALL:
	{	il->i_next = curr_icall;
		curr_icall = il;
		if (il == 0) error('i',"E::print: iline missing");
		Pexpr a0 = il->arg[0];
		int val = QUEST;
		if (il->fct_name->n_oper != CTOR) goto dumb;

		/*
			find the value of "this"
	   		if the argument is a "this" NOT assigned to
			by the programmer, it was initialized
		*/
		switch (a0->base) {
		case ZERO:
			val = 0;
			break;
		case ADDROF:
		case G_ADDROF:
			val = 1;
			break;
		case CAST:
			if (a0->e1->base == ANAME || a0->e1->base == NAME) {
				Pname a = (Pname)a0->e1;
				if (a->n_assigned_to == FUDGE111) val = FUDGE111;
			}
		}
		if (val==QUEST) goto dumb;
		/*
			now find the test:  "(this==0) ? _new(sizeof(X)) : 0"

			e1 is a comma expression,
			the test is either the first sub-expression
				or the first sub-expression after the assignments
					initializing temporary variables
		 */

	{	Pexpr e = e1;
	lx:
		switch (e->base) {
		case CM:
			e = (e->e2->base==QUEST || e->e1->base==ASSIGN)?e->e2:e->e1;
			goto lx;

		case QUEST:
		{	Pexpr q = e->cond;
			if (q->base==EQ && q->e1->base==ANAME && q->e2==zero) {
				Pexpr saved = new expr(0,0,0);
				Pexpr from = (val==0) ? e->e1 : e->e2;
				*saved = *e;
				*e = *from;
				eprint(e1);
				*e = *saved;
				delete saved;
				curr_icall = il->i_next;
				return;
			}
		}
		}
	}
	dumb:
		eprint(e1);
		if (e2) Pstmt(e2)->print();
		curr_icall = il->i_next;
		break;
	}
	case REF:
	case DOT:
		eprint(e1);
		puttok(base);
		if (mem->base == NAME)
			Pname(mem)->print();
		else
			mem->print();
		break;

	case VALUE:
		tp2->print();
		puttok(LP);

//		if (e2) {
//			putstring("/* &");
//			e2->print();
//			putstring(", */");
//		}
		if (e1) e1->print();
		puttok(RP);
		break;

	case SIZEOF:
		puttok(SIZEOF);
		if (e1 && e1!=dummy) {
			eprint(e1);
		}
		else if (tp2) {
			putch('(');
			if (tp2->base == CLASS) {			
				if ( Pclass(tp2)->csu == UNION )
					putstring( "union " );
				else putstring("struct ");
				putstring(Pclass(tp2)->string);
			}
			else
				tp2->print();
			putch(')');
		}
		break;

	case NEW:
		puttok(NEW);
		tp2->print();
		if (e1) {
			putch('(');
			e1->print();
			putch(')');
		}
		break;

	case DELETE:
		puttok(DELETE);
		e1->print();
		break;

	case CAST:
		putch('(');
		if (tp2->base != VOID) {
			putch('(');
			bit oc = Cast;
			Cast = 1;
			tp2->print();
			Cast = oc;
			putch(')');	
		}
		eprint(e1);
		putch(')');
		break;

	case ICON:
	case FCON:
	case CCON:
	case ID:
		if (string) putst(string);
		break;

	case STRING:
		fprintf(out_file,"\"%s\"",string);
		break;

	case THIS:
	case ZERO:
		putstring("0 ");
		break;

	case IVAL:
		fprintf(out_file,"%d",i1);
		break;

	case TEXT:
		if ( string2 )
			fprintf( out_file, " %s_%s", string, string2 );
		else
			fprintf( out_file, " %s", string );
		break;

	case DUMMY:
		break;

	case G_CALL:
	case CALL:
	{	Pname fn = fct_name;
		Pname at;
		if (fn) {
			Pfct f = (Pfct)fn->tp;

			if (f->base==OVERLOAD) {	// overloaded after call
				Pgen g = (Pgen)f;
				fct_name = fn = g->fct_list->f;
				f = (Pfct)fn->tp;
			}
			fn->print();
			at = (f->f_this) ? f->f_this : (f->f_result) ? f->f_result : f->argtype;
		}
		else {
			Pfct f = Pfct(e1->tp);

			if (f)	{	// pointer to fct
				while (f->base == TYPE) f = Pfct(Pbase(f)->b_name->tp);
				if (f->base == PTR) {
					putstring("(*");
					e1->print();
					putch(')');
					f = Pfct(Pptr(f)->typ);
					while (f->base == TYPE) f = Pfct(Pbase(f)->b_name->tp);
				}
				else
					eprint(e1);

				// must be FCT
				at = (f->f_result) ? f->f_result : f->argtype;
			}
			
			else {	// virtual: argtype encoded
				// f_this already linked to f_result and/or argtype
				at = (e1->base==QUEST) ? Pname(e1->e1->tp2) : Pname(e1->tp2);
//fprintf(stderr, "\nat: %d %s", at, at?at->string:"no name" );
				eprint(e1);
			}

		}
		puttok(LP);
		if (e2) {
			if (at) {
				Pexpr e = e2;
				while (at) {
					Pexpr ex;
					Ptype t = at->tp;

					if (e == 0) error('i',"A missing for %s()",fn?fn->string:"??");
					if (e->base == ELIST) {
						ex = e->e1;
						e =  e->e2;
					}
					else
						ex = e;

					if (ex==0) error('i',"A ofT%t missing",t);
					if (t!=ex->tp
					&& ex->tp
					&& t->check(ex->tp,0)
					&& t->is_cl_obj()==0
					&& eobj==0) {
						putch('(');
						bit oc = Cast;
						Cast = 1;
						t->print();
						Cast = oc;
						putch(')');
#ifdef sun
if (ex->base == DIV) { // defend against perverse SUN cc bug
	putstring("(0+");
	eprint(ex);
	putch(')');
}
else
#endif
						eprint(ex);
					}
					else
						ex->print();
					at = at->n_list;
					if (at) puttok(CM);
				}
				if (e) {
					puttok(CM);
					e->print();
				}		 
			}
			else
				e2->print();
		}
		puttok(RP);
		break;
	}

	case ASSIGN:
		if (e1->base==ANAME && Pname(e1)->n_assigned_to==FUDGE111) {
		// suppress assignment to "this" that has been optimized away
			Pname n = (Pname)e1;
			int argno = n->n_val;
			Pin il;
			for (il=curr_icall; il; il=il->i_next)
				if (il->i_table == n->n_table) goto akk;
			goto bkk;
		akk:
			if (il->local[argno] == 0) {
				e2->print();
				break;
			}
		}
	case EQ:
	case NE:
	case GT:
	case GE:
	case LE:
	case LT:
	bkk:
		eprint(e1);
		puttok(base);

		if (e1->tp!=e2->tp && e2->base!=ZERO) {
			// cast, but beware of int!=long etc.
			Ptype t1 = e1->tp;
		cmp:
			switch (t1->base) {
			default:
				break;
			case TYPE:
				t1 = Pbase(t1)->b_name->tp; goto cmp;
			case PTR:
			case RPTR:
			case VEC:
				if (e2->tp==0
				|| (Pptr(t1)->typ!=Pptr(e2->tp)->typ
					&& t1->check(e2->tp,0))) {
					putch('(');
					bit oc = Cast;
					Cast = 1;
					e1->tp->print();
					Cast = oc;
					putch(')');	
				}
			}
		}

		eprint(e2);
		break;

	case DEREF:
		if (e2) {
			eprint(e1);
			putch('[');
			e2->print();
			putch(']');
		}
		else {
			putch('*');
			eprint(e1);
		}
		break;

	case ILIST:
		puttok(LC);
		if (e1) e1->print();
		puttok(RC);
		break;

	case ELIST:
	{	Pexpr e = this;
		for(;;) {
			if (e->base == ELIST) {
				e->e1->print();
				if (e = e->e2)
					puttok(CM);
				else
					return;
			}
			else {
				e->print();
				return;
			}
		}
	}

	case QUEST:
	{	// look for (&a == 0) etc.
		extern bit binary_val;
		Neval = 0;
		binary_val = 1;
		int i = cond->eval();
		binary_val = 0;
		if (Neval == 0)
			(i?e1:e2)->print();
		else {
			eprint(cond);
			puttok(QUEST);
			eprint(e1);
			puttok(COLON);
			eprint(e2);
		}
		break;
	}

	case CM:	// do &(a,b) => (a,&b) for previously checked inlines
	case G_CM:
		puttok(LP);

		switch (e1->base) {
		case ZERO:
		case IVAL:
		case ICON:
		case NAME:
		case DOT:
		case REF:
		case FCON:
		case FVAL:
		case STRING:
			goto le2;	// suppress constant a: &(a,b) => (&b)
		default:
			{	int oo = addrof_cm;	// &(a,b) does not affect a
				addrof_cm = 0;
				eprint(e1);
				addrof_cm = oo;
			}
			puttok(CM);
		le2:
			if (addrof_cm) {
				switch (e2->base) {
				case CAST:
					switch (e2->e2->base) {
					case CM:
					case G_CM:
					case ICALL:	goto ec;
					}
				case NAME:
				case DOT:
				case DEREF:
				case REF:
				case ANAME:
					puttok(ADDROF);
					addrof_cm--;
					eprint(e2);
					addrof_cm++;
					break;
				case ICALL:
				case CALL:
				case CM:
				case G_CM:
				ec:
					eprint(e2);
					break;
				case G_CALL:
					/* & ( e, ctor() ) with temporary optimized away */
					if (e2->fct_name
					&& e2->fct_name->n_oper==CTOR) {
						addrof_cm--;
						eprint(e2);
						addrof_cm++;
						break;
					}
				default:
					error('i',"& inlineF call (%k)",e2->base);
				}
			}
			else
			//	e2->print();
				eprint(e2);
			puttok(RP);
		}
		break;

	case UMINUS:
	case NOT:
	case COMPL:
		puttok(base);
		eprint(e2);
		break;
	case ADDROF:
	case G_ADDROF:
		switch (e2->base) {	// & *e1 or &e1[e2]
		case DEREF:
			if (e2->e2 == 0) {	// &*e == e
				e2->e1->print();
				return;
			}
			break;
		case ICALL:
			addrof_cm++;	// assumes inline expanded into ,-expression
			eprint(e2);
			addrof_cm--;
			return;
		}

		// suppress cc warning on &fct
		if (e2->tp==0 || e2->tp->base!=FCT) puttok(ADDROF);

		eprint(e2);
		break;

	case PLUS:
	case MINUS:
	case MUL:
	case DIV:
	case MOD:
	case LS:
	case RS:
#ifdef DK
	case AND:	putc('&'); break;
	case OR:	putc('|'); break;
	case ER:	putc('^'); break;
	case ANDAND:	putstring("&&"); break;
	case OROR:	putstring("||"); break;
#else
	case AND:
	case OR:
	case ER:
	case ANDAND:
	case OROR:
#endif
	case ASOR:
	case ASER:
	case ASAND:
	case ASPLUS:
	case ASMINUS:
	case ASMUL:
	case ASMOD:
	case ASDIV:
	case ASLS:
	case ASRS:
	case DECR:
	case INCR:
		eprint(e1);
		puttok(base);
		eprint(e2);
		break;

	default:
		error('i',"%p->E::print%k",this,base);
	}
}

Pexpr aval(Pname a)
{
	int argno = a->n_val;
	Pin il;
	for (il=curr_icall; il; il=il->i_next)
		if (il->i_table == a->n_table) goto aok;
	return 0;
aok:
	Pexpr aa = il->arg[argno];
ll:
	switch (aa->base) {
	case CAST:	aa = aa->e1; goto ll;
	case ANAME:	return aval(Pname(aa));
	default:	return aa;
	}
}

#define putcond()	putch('('); e->print(); putch(')')

void stmt::print()
{
//error('d',"S::print %d:%k s %d s_list %d",this,base,s,s_list);
	if (where.line!=last_line.line)
		if (last_ll = where.line)
			where.putline();
		else
			last_line.putline();

	if (memtbl && base!=BLOCK) { /* also print declarations of temporaries */
		puttok(LC);
		Ptable tbl = memtbl;
		memtbl = 0;
		int i;
		int bl = 1;
		for (Pname n=tbl->get_mem(i=1); n; n=tbl->get_mem(++i)){
			if (n->tp == any_type) continue;
			/* avoid double declarartion of temporaries from inlines */
			char* s = n->string;
			if (s[0]!='_' || s[1]!='X') {
				n->dcl_print(0);
				bl = 0;
			}
			Pname cn;
			if (bl
			&& (cn=n->tp->is_cl_obj())
			&& Pclass(cn->tp)->has_dtor()) bl = 0;
		}

		if (bl) {
			Pstmt sl = s_list;
			s_list = 0;
			print();
			memtbl = tbl;
			puttok(RC);
			if (sl) {
				s_list = sl;
				sl->print();
			}
		}
		else {
			print();
			memtbl = tbl;
			puttok(RC);
		}
		return;
	}

	switch (base) {
	default:
		error('i',"S::print(base=%k)",base);

	case ASM:
		fprintf(out_file,"asm(\"%s\");\n",(char*)e);
		break;

	case DCL:
		d->dcl_print(SM);
		break;

	case BREAK:
	case CONTINUE:
		puttok(base);
		puttok(SM);
		break;

	case DEFAULT:
		puttok(base);
		puttok(COLON);
		s->print();
		break;

	case SM:
/*if (e->base==CALL || e->base==G_CALL) error('d',"%n",(Pname)e->e1);*/
		if (e) {
			e->print();
			if (e->base==ICALL && e->e2) break;	/* a block: no SM */
		}
		puttok(SM);
		break;

	case WHILE:
		puttok(WHILE);
		putcond();
		if (s->s_list) {
			puttok(LC);
			s->print();
			puttok(RC);
		}
		else
			s->print();
		break;

	case DO:
		puttok(DO);
		s->print();
		puttok(WHILE);
		putcond();
		puttok(SM);
		break;

	case SWITCH:
		puttok(SWITCH);
		putcond();
		s->print();
		break;

	case RETURN:
#ifdef RETBUG
		if (empty && ret_tp) {	// fudge to bypass C bug (see simpl.c)
			Pname cn = ret_tp->is_cl_obj();
			fprintf(out_file,"{struct %s _plain_silly;return _plain_silly;}",cn->string);
		}
		else
			
#endif
		{
		puttok(RETURN);
		if (e) {
			if (ret_tp && ret_tp!=e->tp) {
				Ptype tt = ret_tp;
			gook:
				switch (tt->base) {
				case TYPE:
					tt = Pbase(tt)->b_name->tp;
					goto gook;
				case COBJ:
					break;	// cannot cast to struct
				case RPTR:
				case PTR:
					if (Pptr(tt)->typ==Pptr(e->tp)->typ) break;
				default:
					if (e->tp==0 || ret_tp->check(e->tp,0)) {
						int oc = Cast;
						putch('(');
						Cast = 1;
						ret_tp->print();
						Cast = oc;
						putch(')');
					}
				}
			}
			eprint(e);
		}
		puttok(SM);
		}
		while (s_list && s_list->base==SM) s_list = s_list->s_list; // FUDGE!!
		break;

	case CASE:
		puttok(CASE);
		eprint(e);
		puttok(COLON);
		s->print();
		break;

	case GOTO:
		puttok(GOTO);
		d->print();
		puttok(SM);
		break;

	case LABEL:
		d->print();
		putch(':');
		s->print();
		break;

	case IF:
	{	int val = QUEST;
		if (e->base == ANAME) {
			Pname a = (Pname)e;
			Pexpr arg = aval(a);
//error('d',"arg %d%k %d (%d)",arg,arg?arg->base:0,arg?arg->base:0,arg?arg->e1:0);
			if (arg)
				switch (arg->base) {
				case ZERO:	val = 0; break;
				case ADDROF:
				case G_ADDROF:	val = 1; break;
				case IVAL:	val = arg->i1!=0;
			}
		}
//error('d',"val %d",val);
		switch (val) {
		case 1:
			s->print();
			break;
		case 0:
			if (else_stmt)
				else_stmt->print();
			else
				puttok(SM);	/* null statement */
			break;
		default:
			puttok(IF);
			putcond();
			if (s->s_list) {
				puttok(LC);
				s->print();
				puttok(RC);
			}
			else
				s->print();
			if (else_stmt) {
				puttok(ELSE);
				if (else_stmt->s_list) {
					puttok(LC);
					else_stmt->print();
					puttok(RC);
				}
				else
					else_stmt->print();
			}
		}
		break;
	}

	case FOR:
	{	int fi = for_init && (for_init->base!=SM || for_init->memtbl || for_init->s_list);
//error('d',"for(; %d%k; %d%k)",e,e->base,e2,e2->base);
		if (fi) {
			puttok(LC);
			for_init->print();
		}
		putstring("for(");
		if (fi==0 && for_init) for_init->e->print();
		putch(';');	// to avoid newline: not puttok(SM)
		if (e) e->print();
		putch(';');
		if (e2) e2->print();
		puttok(RP);
		s->print();
		if (fi) puttok(RC);
		break;
	}

	case PAIR:
		if (s&&s2) {
			puttok(LC);
			s->print();
			s2->print();
			puttok(RC);
		}
		else {
			if (s) s->print();
			if (s2) s2->print();
		}
		break;

	case BLOCK:
		puttok(LC);
		if (d) d->dcl_print(SM);
		if (memtbl && own_tbl) {
			int i;
			for (Pname n=memtbl->get_mem(i=1); n; n=memtbl->get_mem(++i)) {
				if (n->tp && n->n_union==0 && n->tp!=any_type)
					switch (n->n_scope) {
					case ARGT:
					case ARG:
						break;
					default:
						n->dcl_print(0);
					}
			}
		}
		if (s) s->print();
		putstring("}\n");
		if (last_ll && where.line) last_line.line++;
	}

	if (s_list) s_list->print();
}

void table::dcl_print(TOK s, TOK pub)
/*
	print the declarations of the entries in the order they were inserted
	ignore labels (tp==0)
*/
{
	register Pname* np;
	register int i;

	if (this == 0) return;

	np = entries;
	for (i=1; i<free_slot; i++) {
		register Pname n = np[i];
		switch (s) {
		case 0:
			n->dcl_print(0);
			break;
		case EQ:
			if (n->tp && n->n_scope == pub) n->dcl_print(0);
			break;
		case NE:
			if (n->tp && n->n_scope != pub) n->dcl_print(0);
			break;
		}
	}
}

