/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/dcl2.c	1.1"
/*ident	"@(#)cfront:src/dcl2.c	1.17" */
/**************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


dcl2.c:
	Declaration of class, enum, and statement

*************************************************************************/

#include "cfront.h"
#include "size.h"

extern int tsize;
void classdef::dcl(Pname cname, Ptable tbl)
{
	int nmem;
	Pname p;
	Pptr cct;
	Pbase bt;
	Pname px;
	Ptable btbl;
	int bvirt;
	Pclass bcl;
	int i;
	int fct_seen = 0;
	int static_seen = 0;
	int local = tbl!=gtbl;
	int scope = PUBLIC;
	int protect = 0;
	Pname publist = 0;

	int byte_old = byte_offset;
	int bit_old = bit_offset;
	int max_old = max_align;
	int boff;

	int in_union = 0;
	int usz;
	int make_ctor = 0;
	int make_dtor = 0;

	/* this is the place for paranoia */
	if (this == 0) error('i',"0->Cdef::dcl(%p)",tbl);
	if (base != CLASS) error('i',"Cdef::dcl(%d)",base);
	if (cname == 0) error('i',"unNdC");
	if (cname->tp != this) error('i',"badCdef");
	if (tbl == 0) error('i',"Cdef::dcl(%n,0)",cname);
	if (tbl->base != TABLE) error('i',"Cdef::dcl(%n,tbl=%d)",cname,tbl->base);

	nmem = mem_list->no_of_names() + pubdef->no_of_names();

	switch (csu) {
	case UNION:
	case ANON:
		in_union = 1;
		if (virt_count) error("virtualF in union");
		break;
	 case CLASS:
		scope = 0;
		if (virt_count == 0) csu = STRUCT; // default simple
	}

	if (clbase) {
			if (clbase->base != TNAME) error("BC%nU",clbase);
			clbase = Pbase(clbase->tp)->b_name;
			bcl = (Pclass)clbase->tp;
			if ((bcl->defined&(DEFINED|SIMPLIFIED)) == 0) {
				error("BC%nU",clbase);
				goto nobase;
			}
			tbl = bcl->memtbl;
			if (tbl->base != TABLE) {
				error('i',"badBC table %p",tbl);
				goto nobase;
			}
			btbl = tbl;
			bvirt = bcl->virt_count;
			if (bcl->csu == UNION) error("C derived from union");
			if (in_union) error("derived union");
			if (pubbase == 0) csu = CLASS;
			boff = bcl->real_size;
			max_align = bcl->align();
			bit_ass = bcl->bit_ass;
	//	}
	}
	else {
	nobase:
		btbl = 0;
		bvirt = 0;
		boff = 0;
		tbl = gtbl;	// class NOT in local scope
		while (tbl!=gtbl && tbl->t_name) tbl = tbl->next; // nested classes
		max_align = AL_STRUCT;
		if (virt_count == 0) bit_ass = 1;	// can be bitwise copied
	}

	memtbl->set_scope(tbl);
	memtbl->set_name(cname);
	if (nmem) memtbl->grow((nmem<=2)?3:nmem);

	cc->stack();
	cc->not = cname;
	cc->cot = this;
//error('d',"Cdef%n",cname);
	byte_offset = usz = boff;
	bit_offset = 0;

	bt = new basetype(COBJ,cname);
	bt->b_table = memtbl;
	this_type = cc->tot = cct = new ptr(PTR,bt,0);
	PERM(cct);
	PERM(bt);

	for (p=mem_list; p; p=px) {
		Pname m;
		px = p->n_list;

		switch (p->base) {
		case PUBLIC:
				scope = PUBLIC;
				protect = 0;
				goto prpr;
		case PRIVATE:
				scope = 0;
				protect = 0;
				csu = CLASS;
				goto prpr;
		case PROTECTED:
				scope = 0;
				protect = PROTECTED;
				csu = CLASS;
		prpr:
//error('d',"%k: scope %d",p->base,scope);
			if (in_union) error("%k label in unionD",p->base);
			continue;
		}

		if (p->base == PR) {	// visibility control
			p->base = NAME;
			if (scope != PUBLIC) {
				error('s',"visibilityD not in public section");
				continue;
			}
			p->n_list = publist;
			publist = p;
			continue;
		}

//error('d',"mem%n %d scope %d",p,p->tp->base,scope);
		if (scope==0 || scope==PROTECTED) csu=CLASS;

		if (p->tp->base==FCT) {
			Pfct f = (Pfct)p->tp;
			Pblock b = f->body;
			f->body = 0;
			switch( p->n_sto ) {
			case AUTO:
			case STATIC:
			case REGISTER:
			case EXTERN:
				error("M%n cannot be%k",p,p->n_sto);
				p->n_sto = 0;
			}
			m =  p->dcl(memtbl,scope);
			if (m == 0 ) continue;
			if ( tsize ) { byte_offset += tsize; tsize = 0; }
			m->n_protect = protect;
			if (b) {
				if (m->tp->defined&(DEFINED|SIMPLIFIED))
					error("two definitions of%n",m);
				else if (p->where.line!=m->where.line)
					error('s',"previously declared%n cannot be defined inCD",p);
				else
					Pfct(m->tp)->body = b;
			}
			fct_seen = 1;
		}
		else {
			m = p->dcl(memtbl,scope);
			m->n_protect = protect;
			if (m) {
				if (m->n_stclass==STATIC) {
					static_seen = 1;
					m->n_sto = (tbl == gtbl) ? 0 : STATIC;
					if (m->n_initializer) error('s',"staticM%nWIr",m);
				}
				if (in_union) {
					if (usz < byte_offset) usz = byte_offset;
					byte_offset = 0;
				}
			}
		}
	}

//	if (local && fct_seen) error("FM of local%k %s",csu,string);

	if (in_union) byte_offset = usz;

	if (virt_count || bvirt) {	/* assign virtual indices */
		Pname vp[100];
		Pname nn;

		nn = has_ctor();
		if (nn==0 || nn->n_table!=memtbl) make_ctor = 1;

		{	//	FUDGE vtbl
			char s[128];
			sprintf(s,"%s__vtbl",string);
			Pname n = new name(s);
			n->tp = Pfctvec_type;
			Pname nn = gtbl->insert(n,0);
			nn->use();
		}

		if (virt_count = bvirt)
			for (i=0; i<bvirt; i++) vp[i] = bcl->virt_init[i];

for ( nn=memtbl->get_mem(i=1); nn; nn=memtbl->get_mem(++i) ) {
	switch (nn->tp->base) {
	case FCT:
	{	Pfct f = (Pfct)nn->tp;
		if (bvirt) {
			Pname vn = btbl->look(nn->string,0);
			if (vn) {	/* match up with base class */
				if (vn->n_table==gtbl) goto vvv;
				Pfct vnf;
				switch (vn->tp->base) {
				case FCT:
					vnf = (Pfct)vn->tp;
					if (vnf->f_virtual) {
						if (vnf->check(f,0)) error("virtual%nT mismatch:%t and%t",nn,f,vnf);
						f->f_virtual = vnf->f_virtual;
						vp[f->f_virtual-1] = nn;
					}
					else
						goto vvv;
					break;
				case OVERLOAD:
				{	Pgen g = (Pgen)vn->tp;
					if (f->f_virtual
					|| Pfct(g->fct_list->f->tp)->f_virtual)
						error('s',"virtual%n overloaded inBC but not in derivedC",nn);
					break;
				}
				default:
					goto vvv;
				}
			}
			else
				goto vvv;
		}
		else {
		vvv:
/*error('d',"vvv: %n f_virtual %d virt_count %d",nn,f->f_virtual,virt_count);*/
			if (f->f_virtual)  {
				f->f_virtual = ++virt_count;
				switch (f->f_virtual) {
				case 1:
				{	Pname vpn = new name("_vptr");
					vpn->tp = Pfctvec_type;
					(void) vpn->dcl(memtbl,PUBLIC);
					delete vpn;
				}
				default:
					vp[f->f_virtual-1] = nn;
				}
			}
		}
		break;
	}

	case OVERLOAD:
	{	Plist gl;
		Pgen g = (Pgen)nn->tp;
/*error('d',"overload%n bvirt==%d",nn,bvirt);*/
		if (bvirt) {
			Pname vn = btbl->look(nn->string,0);
			Pgen g2;
			Pfct f2;
			if (vn) {
/*error('d',"vn%n tp%k",vn,vn->tp->base);*/
				if (vn->n_table == gtbl) goto ovvv;
				switch (vn->tp->base) {
				default:
					goto ovvv;
				case FCT:
					f2 = (Pfct)vn->tp;
					if (f2->f_virtual
					|| Pfct(g->fct_list->f->tp)->f_virtual)
						error('s',"virtual%n overloaded in derivedC but not inBC",nn);
					break;
				case OVERLOAD:
					g2 = (Pgen)vn->tp;
						
					for (gl=g->fct_list; gl; gl=gl->l) {
						Pname fn = gl->f;
						Pfct f = (Pfct)fn->tp;
						Pname vn2 = g2->find(f,0);

						if (vn2 == 0) {
							if (f->f_virtual) error('s',"virtual overloaded%n not found inBC",fn);
						}
						else {
							Pfct vn2f = (Pfct)vn2->tp;
							if (vn2f->f_virtual) {
								f->f_virtual = vn2f->f_virtual;
								vp[f->f_virtual-1] = fn;
							}
						}
					}
					break;
				}
			}
			else
				goto ovvv;
		}
		else {
		ovvv:
			for (gl=g->fct_list; gl; gl=gl->l) {
				Pname fn = gl->f;
				Pfct f = (Pfct)fn->tp;

/*fprintf(stderr,"fn %s f %d %d %d count %d\n",fn->string,f,f->base,f->f_virtual,virt_count+1);*/
				if (f->f_virtual) {
					f->f_virtual = ++virt_count;
					switch (f->f_virtual) {
					case 1:
					{	Pname vpn = new name("_vptr");
						vpn->tp = Pfctvec_type;
						(void) vpn->dcl(memtbl,0);
						delete vpn;
					}
					default:
						vp[f->f_virtual-1] = fn;
					}
				}
			}
		}
		break;
	}
}
		}
		virt_init = new Pname[virt_count];
		for (i=0; i<virt_count; i++) virt_init[i] = vp[i];
	}

	Pname pnx;
	for (p=publist; p; p=pnx) {
		char* qs = p->n_qualifier->string;
		char* ms = p->string;
		Pname cx;
		Ptable ctbl;
		Pname mx;
		pnx = p->n_list;
//error('d',"dcl: publist %s::%s",qs,ms);
		if (strcmp(ms,qs)==0) ms = "_ctor";

		for (cx = clbase; cx; cx = Pclass(cx->tp)->clbase) {
			if (strcmp(cx->string,qs) == 0) goto ok;
		}
		error("publicQr %s not aBC",qs);
		continue;
	ok:
		ctbl = Pclass(cx->tp)->memtbl;
		mx = ctbl->lookc(ms,0);
//error('d',"ms %d %d %d",mx,Ebase,Epriv);
		if (Ebase) {	// cc->nof ??
			if (!Ebase->has_friend(cc->nof)) error("QdMN%n is in privateBC",p);
		}
		else if (Epriv) {
			if (!Epriv->has_friend(cc->nof)
			&& !(mx->n_protect && Epriv->baseof(cc->nof))) error("QdMN%n is private",p);
		}

		if (mx == 0) {
			error("C%n does not have aM %s",cx,p->string);
			p->tp = any_type;
		}
		else {
			if (mx->tp->base==OVERLOAD) error('s',"public specification of overloaded%n",mx);
			p->base = PUBLIC;
		}
		
		p->n_qualifier = mx;
		(void) memtbl->insert(p,0);
//error('d',"bbb");
		if (Nold) error("twoDs of CM%n",p);
	}

	if (bit_offset) byte_offset += (bit_offset/BI_IN_BYTE+1);
	real_size = byte_offset;
//error('d',"%s: rz=%d (bits %d)",string,byte_offset,bit_offset);
	if (byte_offset < SZ_STRUCT) byte_offset = SZ_STRUCT;
	int waste = byte_offset%max_align;
	if (waste) byte_offset += max_align-waste;
//error('d',"%s: sz=%d al=%d",string,byte_offset,max_align);
	obj_size = byte_offset;
	obj_align = max_align;
	
	if ( has_dtor() && has_ctor()==0)
		error('w',"%s has destructor but noK",string);
	
{	// now look look at the members
	Pname m;
	Pclass oc = in_class;
	Pname ct = has_ctor();
	Pname dt = has_dtor();
	int un = csu==UNION;

	if ( ct && ct->n_table != memtbl ) ct = 0;
	if ( dt && dt->n_table != memtbl ) dt = 0;

	if ( ct == 0 || dt == 0 || un )
	for (m=memtbl->get_mem(i=1); m; m=memtbl->get_mem(++i) ) {

		Ptype t = m->tp;
		switch (t->base) {
		default:
			if (ct == 0 && m->n_stclass != ENUM ) {
				if (t->is_ref()) error("R%n inC %sWoutK",m,string);
				if (t->tconst() && vec_const==0) error("constant%n inC %sWoutK",m,string);
			}
			break;
		case FCT:
		case OVERLOAD:
		case CLASS:
		case ENUM:
			continue;
		case VEC:
			break;
		}
		Pname cn = t->is_cl_obj();
		if (cn == 0) cn = cl_obj_vec;
		if (cn == 0) continue;

		Pclass cl = (Pclass)cn->tp;
		if (cl->bit_ass == 0) bit_ass = 0;	// no bit copy

		Pname ctor = cl->has_ctor();
		Pname dtor = cl->has_dtor();

		if (ctor) {
			if (m->n_stclass==STATIC) 
				error('s',"staticM%n ofC%nWK",m,cn);
			else if (un)
				error("M%n ofC%nWK in union",m,cn);
			else if (ct == 0) make_ctor = 1;
			}
				
		if (dtor) {
			if (m->n_stclass==STATIC) 
				error('s',"staticM%n ofC%nW destructor",m,cn);
			else if (un)
				error("M%n ofC%nW destructor in union",m,cn);
			else if (dt == 0) make_dtor = 1;
			}
		}
	}

	if (make_ctor) {
		Pname ct = has_ctor();
		if (ct==0 || ct->n_table!=memtbl) {
			// make a constructor for the class: x::x() {}
			// a base class's constructor is not good enough
			if (ct && has_ictor()==0) error("%k %s needs aK",csu,string);
			Pname n = new name(string);
			Pfct f = new fct(defa_type,0,1);
			n->tp = f;
			n->n_oper = TNAME;
			Pname m = n->dcl(memtbl,PUBLIC);
			Pfct(m->tp)->body = new block(curloc,0,0);
		}
	}

	if (make_dtor && has_dtor()==0) {
		// make a destructor for the class: x::x() {}
		Pname n = new name(string);
		Pfct f = new fct(defa_type,0,1);
		n->tp = f;
		n->n_oper = DTOR;
		Pname m = n->dcl(memtbl,PUBLIC);
		Pfct(m->tp)->body = new block(curloc,0,0);
	}

	defined |= DEFINED;

	Pname it = has_itor();
	for (p=memtbl->get_mem(i=1); p; p=memtbl->get_mem(++i)) {
	/* define members defined inline */
//error('d',"M%n",p);
		switch (p->tp->base) {
		case FCT:
		{	Pfct f = (Pfct)p->tp;
			if (f->body) {
				f->f_inline = 1;
				p->n_sto = STATIC;
				f->dcl(p);
			}
		/*	else if ((it||f->f_virtual)
			&& f->f_result==0
			&& f->returns->is_cl_obj()) make_res(f);*/
			break;
		}
		case OVERLOAD:
		{	Pgen g = (Pgen)p->tp;
			for (Plist gl=g->fct_list; gl; gl=gl->l) {
				Pname n = gl->f;
				Pfct f = (Pfct)n->tp;
				if (f->body) {
					f->f_inline = 1;
					n->n_sto = STATIC;
					f->dcl(n);
				}
			/*	else if ((it||f->f_virtual)
					&& f->f_result==0
					&& f->returns->is_cl_obj()) make_res(f);*/
			}
		}
		}
	}

	// define friends defined inline and modify return types if necessary
	for (Plist fl=friend_list; fl; fl=fl->l) {
		Pname p = fl->f;
		switch (p->tp->base) {
		case FCT:
		{	Pfct f = (Pfct)p->tp;

		/*	if ((it||f->f_virtual)
			&& f->f_result==0
			&& f->returns->is_cl_obj()) make_res(f);*/

			if (f->body
			&&  (f->defined&(DEFINED|SIMPLIFIED)) == 0) {
				f->f_inline = 1;
				p->n_sto = STATIC;
				f->dcl(p);
			}
			break;
		}
		case OVERLOAD:
		{	Pgen g = (Pgen)p->tp;
			Plist gl;
			for (gl=g->fct_list; gl; gl=gl->l) {
				Pname n = gl->f;
				Pfct f = (Pfct)n->tp;

			/*	if ((it||f->f_virtual)
				&& f->f_result==0
				&& f->returns->is_cl_obj()) make_res(f);*/

				if (f->body
				&&  (f->defined&(DEFINED|SIMPLIFIED)) == 0) {
					f->f_inline = 1;
					n->n_sto = STATIC;
					f->dcl(n);
				}
			}
		}
		}
	}

	byte_offset = byte_old;
	bit_offset = bit_old;
	max_align = max_old;

	cc->unstack();
}

void enumdef::dcl(Pname, Ptable tbl)
{
#define FIRST_ENUM 0
	int nmem = mem->no_of_names();
	Pname p;
	Pname ns = 0;
	Pname nl = 0;
	int enum_old = enum_count;
	no_of_enumerators = nmem;

	enum_count = FIRST_ENUM;

	if (this == 0) error('i',"0->enumdef::dcl(%p)",tbl);
	Pname px;
	for(p=mem, mem=0; p; p=px) {
		Pname nn;
		px = p->n_list;
		if (p->n_initializer) {
			Pexpr i = p->n_initializer->typ(tbl);
			Neval = 0;
			enum_count = i->eval();
			if (Neval) error("%s",Neval);
			DEL(i);
			p->n_initializer = 0;
		}
		p->n_evaluated = 1;
		p->n_val = enum_count++; 
		nn = tbl->insert(p,0); /* ??? */
		if (Nold) {
			if (nn->n_stclass == ENUM)
				error( (p->n_val!=nn->n_val)?0:'w',"enumerator%n declared twice",nn);
			else
				error("incompatibleDs of%n",nn);
		}
		else {
			nn->n_stclass = ENUM; // no store will be allocated
			if (ns)
				nl->n_list = nn;
			else
				ns = nn;
			nl = nn;
		}
		delete p;
	}

	mem = ns;

	enum_count = enum_old;
	defined |= DEFINED;
}

Pstmt curr_loop;
Pstmt curr_switch;
Pblock curr_block;

void stmt::reached()
{
	register Pstmt ss = s_list;

	if (ss == 0) return;

	switch (ss->base) {
	case LABEL:
	case CASE:
	case DEFAULT:
		break;
	default:
		error('w',"S not reached");
		for (; ss; ss=ss->s_list) {	// delete unreacheable code
			switch (ss->base) {
			case LABEL:
			case CASE:
			case DEFAULT:		// reachable
				s_list = ss;
				return;
			case DCL:		// the dcl may be used later
						// keep to avoid cascading errors
			case IF:
			case DO:
			case WHILE:
			case SWITCH:
			case FOR:
			case BLOCK:		// may hide a label
				s_list = ss;
				return;
			}
		}
		s_list = 0;
	}
}

bit arg_err_suppress;

Pexpr check_cond(Pexpr e, TOK b, Ptable tbl)
{
//error('d',"check_cond(%k %k) tbl %d",e->base,b,tbl);
	Pname cn;
	if (cn = e->tp->is_cl_obj()) {	
		Pclass cl = (Pclass)cn->tp;
		int i = 0;
		Pname found = 0;
		for (Pname on = cl->conv; on; on=on->n_list) {
			Pfct f = (Pfct)on->tp;
			Ptype t = f->returns;
		xx:
			switch (t->base) {
			case TYPE:
				t = Pbase(t)->b_name->tp;
				goto xx;
			case FLOAT:
			case DOUBLE:
			case PTR:
				if (b == DEREF) break;
			case CHAR:
			case SHORT:
			case INT:
			case LONG:
			case EOBJ:
				i++;
				found = on;
			}
		}
		switch (i) {
		case 0:
			error("%nO in%kE",cn,b);
			return e;
		case 1:
		{
//error('d',"cond%t<-%t:%n",Pfct(found->tp)->returns,e->tp,found);
			Pclass cl = (Pclass)cn->tp;
			Pref r = new ref(DOT,e,found);
			r->tp = found->tp;
			Pexpr c = new expr(G_CALL,r,0);
			c->fct_name = found;
		//	c->tp = Pfct(found->tp)->returns;
			return c->typ(tbl);
		}
		default:
			error("%d possible conversions for%nO in%kE",i,cn,b);
			return e;
		}
		
	}
	if (e->tp->num_ptr(b) == FCT) error("%k(F)",b);
	return e;
}

void stmt::dcl()
/*
	typecheck statement "this" in scope "curr_block->tbl"
*/
{
	Pstmt ss;
	Pname n;
	Pname nn;
	Pstmt ostmt = Cstmt;

	for (ss=this; ss; ss=ss->s_list) {
		Pstmt old_loop, old_switch;
		Cstmt = ss;
		Ptable tbl = curr_block->memtbl;
/*error('d',"ss %d%k tbl %d e %d%k s %d%k sl %d%k", ss, ss->base, tbl, ss->e, (ss->e)?ss->e->base:0, ss->s, (ss->s)?ss->s->base:0, ss->s_list, (ss->s_list)?ss->s_list->base:0);*/
		switch (ss->base) {
		case BREAK:
			if (curr_loop==0 && curr_switch==0)
				error("%k not in loop or switch",BREAK);
			ss->reached();
			break;

		case CONTINUE:
			if (curr_loop == 0) error("%k not in loop",CONTINUE);
			ss->reached();
			break;

		case DEFAULT:
			if (curr_switch == 0) {
				error("default not in switch");
				break;
			}
			if (curr_switch->has_default) error("two defaults in switch");
			curr_switch->has_default = ss;
			ss->s->s_list = ss->s_list;
			ss->s_list = 0;
			ss->s->dcl();
			break;

		case SM:
		{	TOK b = ss->e->base;
			switch (b) {
			case DUMMY:
				ss->e = 0;
				break;
					// check for unused results
					// don't check operators that are likely
					// to be overloaded to represent "actions":
					// ! ~ < <= > >= << >>
			case EQ:
			case NE:
			case PLUS:
			case MINUS:
			case REF:
			case DOT:
			case MUL:
			case DIV:
			case ADDROF:
			case AND:
			case OR:
			case ER:
			case DEREF:
			case ANDAND:
			case OROR:
			case NAME:
				if (ss->e->tp) break;	// avoid looking at generated code
				ss->e = ss->e->typ(tbl);
				if (ss->e->base == CALL) break;
				if (ss->e->tp->base != VOID) error('w',"result of%kE not used",b);
				break;
			default:
				ss->e = ss->e->typ(tbl);
			}
			break;
		}
		case RETURN:
		{	Pname fn = cc->nof;
			Pfct f = Pfct(fn->tp);
			Ptype rt = f->returns;
			Pexpr v = ss->e;
			if (v != dummy) {
				if (rt->base == VOID) {
					error('w',"unexpected return value");
					/*refuse to return the value:*/
					ss->e = dummy;
				}
				else {
					v = v->typ(tbl);
				lx:
					switch (rt->base) {
					case TYPE:
						rt = Pbase(rt)->b_name->tp;
						goto lx;
					case RPTR:
						if (v->lval(0)==0
						&& v->tp->tconst()==0)
							error('w',"R to non-lvalue returned");
						else if (v->base==NAME
						&& Pname(v)->n_scope==FCT)
							error('w',"R to localV returned");
						v = ref_init(Pptr(rt),v,tbl);
//error('d',"rptr %k",v->base);
					case ANY:
						break;
					case COBJ:

//error('d',"ret %d rt %t v%k %d inl %d vtp %t check %d",f->f_result,rt,v->base,v->base,f->f_inline,v->tp,rt->check(v->tp,ASSIGN));
if (v->base == DEREF) {
	Pexpr v1 = v->e1;
	if (v1->base==CAST) {
		Pexpr v2 = v1->e1;
		if (v2->base == G_CM) {	// *(T)(e1,e2) => (e1,*(T)e2)
			Pexpr v3 = v2->e2;
			v2->e2 = v;
			v2->tp = v->tp;
			v = v2;
			v1->e1 = v3;
		}
	}
}
	if (f->f_result) {
		if (rt->check(v->tp,ASSIGN)==0 && v->base==G_CM)
			v = replace_temp(v,f->f_result);
		else {
			v = class_init(f->f_result->contents(),rt,v,tbl);
			Pname rcn = rt->is_cl_obj();
			if (Pclass(rcn->tp)->has_itor()==0) {
				// can happen for virtuals and for user defined conversions
				v->tp = rt;
				v = new expr(ASSIGN,f->f_result->contents(),v);
				v->tp = rt;
			}
		}
	}
	else
		v = class_init(0,rt,v,tbl);
						break;
					case PTR:
						v = ptr_init(Pptr(rt),v,tbl);
						goto def;
					case INT:
					case CHAR:
					case LONG:
					case SHORT:
						if (Pbase(rt)->b_unsigned
						&& v->base==UMINUS
						&& v->e2->base==ICON)
							error('w',"negative retured fromF returning unsigned");
					default:
					def:
						if (rt->check(v->tp,ASSIGN)) {
							Pexpr x = try_to_coerce(rt,v,"return value",tbl);
							if (x)
								v = x;
							else
								error("bad return valueT for%n:%t (%tX)",fn,v->tp,rt);
						}
					}
					ss->ret_tp = rt;
					ss->e = v;
				}
			}
			else {
				if (rt->base != VOID) error('w',"return valueX");
			}
			ss->reached();
			break;
		}

		case DO:	// in DO the stmt is before the test
			inline_restr |= 8;
			old_loop = curr_loop;
			curr_loop = ss;
			if (ss->s->base == DCL) error('s',"D as onlyS in do-loop");
			ss->s->dcl();
			ss->e = ss->e->typ(tbl);
			ss->e = check_cond(ss->e,DO,tbl);
			curr_loop = old_loop;
			break;

		case WHILE:
			inline_restr |= 8;
			old_loop = curr_loop;
			curr_loop = ss;
			ss->e = ss->e->typ(tbl);
			ss->e = check_cond(ss->e,WHILE,tbl);
			if (ss->s->base == DCL) error('s',"D as onlyS in while-loop");
			ss->s->dcl();
			curr_loop = old_loop;
			break;

		case SWITCH:
		{	int ne = 0;
			inline_restr |= 4;
			old_switch = curr_switch;
			curr_switch = ss;
			ss->e = ss->e->typ(tbl);
			ss->e = check_cond(ss->e,SWITCH,tbl);
			{	Ptype tt = ss->e->tp;
			sii:
				switch (tt->base) {
				case TYPE:
					tt = Pbase(tt)->b_name->tp; goto sii;
				case EOBJ:
					ne = Penum(Pbase(tt)->b_name->tp)->no_of_enumerators;
				case ZTYPE:
				case ANY:
				case CHAR:
				case SHORT:
				case INT:
				case LONG:
				case FIELD:
					break;
				default:
					error("%t switchE must be converted to int",ss->e->tp);
				}
			}
			ss->s->dcl();
			if (ne) {	/* see if the number of cases is "close to"
					   but not equal to the number of enumerators
					*/
				int i = 0;
				Pstmt cs;
				for (cs=ss->case_list; cs; cs=cs->case_list) i++;
				if (i && i!=ne) {
					if (ne < i) {
				ee:		error('w',"switch (%t)W %d cases (%d enumerators)",ss->e->tp,i,ne);
					}
					else {
						switch (ne-i) {
						case 1: if (3<ne) goto ee;
						case 2: if (7<ne) goto ee;
						case 3: if (23<ne) goto ee;
						case 4: if (60<ne) goto ee;
						case 5: if (99<ne) goto ee;
						}
					}
				}
			}
			curr_switch = old_switch;
			break;
		}
		case CASE:
			if (curr_switch == 0) {
				error("case not in switch");
				break;
			}
			ss->e = ss->e->typ(tbl);
			ss->e->tp->num_ptr(CASE);
			{	Ptype tt = ss->e->tp;
			iii:
				switch (tt->base) {
				case TYPE:
					tt = Pbase(tt)->b_name->tp; goto iii;
				case ZTYPE:
				case ANY:
				case CHAR:
				case SHORT:
				case INT:
				case LONG:
					break;
				default:
					error('s',"%t caseE",ss->e->tp);
				}
			}
			if (1) {
				Neval = 0;
				int i = ss->e->eval();
				if (Neval == 0) {
					Pstmt cs;
					for (cs=curr_switch->case_list; cs; cs=cs->case_list) {
						if (cs->case_value == i) error("case %d used twice in switch",i);
					}
					ss->case_value = i;
					ss->case_list = curr_switch->case_list;
					curr_switch->case_list = ss;
				}
				else
					error("bad case label: %s",Neval);
			}
			if (ss->s->s_list) error('i',"case%k",ss->s->s_list->base);
			ss->s->s_list = ss->s_list;
			ss->s_list = 0;
			ss->s->dcl();
			break;

		case GOTO:
			inline_restr |= 2;
			ss->reached();
		case LABEL:
			/* Insert label in function mem table;
			   labels have function scope.
			*/
			n = ss->d;
			nn = cc->ftbl->insert(n,LABEL);

			/* Set a ptr to the mem table corresponding to the scope
			   in which the label actually occurred.  This allows the
			   processing of goto's in the presence of ctors and dtors
			*/
			if (ss->base == LABEL) {
				nn->n_realscope = curr_block->memtbl;
				inline_restr |= 1;
			}

			if (Nold) {
				if (ss->base == LABEL) {
					if (nn->n_initializer) error("twoDs of label%n",n);
					nn->n_initializer = (Pexpr)1;
				}
				if (n != nn) ss->d = nn;
			}
			else {
				if (ss->base == LABEL) nn->n_initializer = (Pexpr)1;
				nn->where = ss->where;
			}
			if (ss->base == GOTO)
				nn->use();
			else {
				if (ss->s->s_list) error('i',"label%k",ss->s->s_list->base);
				ss->s->s_list = ss->s_list;
				ss->s_list = 0;
				nn->assign();
			}
			if (ss->s) ss->s->dcl();
			break;

		case IF:
		{	Pexpr ee = ss->e->typ(tbl);
			if (ee->base == ASSIGN) {
				Neval = 0;
				(void)ee->e2->eval();
				if (Neval == 0)
					error('w',"constant assignment in condition");
			}
			ss->e = ee = check_cond(ee,IF,tbl);
//error('d',"if (%t)",ee->tp);
			switch (ee->tp->base) {
			case INT:
			case ZTYPE:
			{	int i;
				Neval = 0;
				i = ee->eval();
//error('d',"if (int:%k) => (i %s)",ss->e->base,i,Neval?Neval:"0");
				if (Neval == 0) {
					Pstmt sl = ss->s_list;
					if (i) {
						DEL(ss->else_stmt);
						ss->s->dcl();
						*ss = *ss->s;
					}
					else {
						DEL(ss->s);
						if (ss->else_stmt) {
							ss->else_stmt->dcl();
							*ss = *ss->else_stmt;
						}
						else {
							ss->base = SM;
							ss->e = dummy;
							ss->s = 0;
						}
					}
					ss->s_list = sl;
					continue;
				}
			}
			}
			ss->s->dcl();
			if (ss->else_stmt) ss->else_stmt->dcl();
			break;
		}
		case FOR:
			inline_restr |= 8;
			old_loop = curr_loop;
			curr_loop = ss;
			if (ss->for_init) {
				Pstmt fi = ss->for_init;
				switch (fi->base) {
				case SM:
					if (fi->e == dummy) {
						ss->for_init = 0;
						break;
					}
				default:
					fi->dcl();
					break;
				case DCL:
					fi->dcl();
//error('d',"dcl=>%k %d",fi->base,fi->base);
					switch (fi->base) {
					case BLOCK:
					{
					/* { ... for( { a } b ; c) d ; e }
						=>
					   { ... { a for ( ; b ; c) d ; e }}
					*/
						Pstmt tmp = new stmt (SM,curloc,0);
						*tmp = *ss;	/* tmp = for */
						tmp->for_init = 0;
						*ss = *fi;	/* ss = { } */
						if (ss->s)
							ss->s->s_list = tmp;
						else
							ss->s = tmp;
						curr_block = (Pblock)ss;
						tbl = curr_block->memtbl;
						Cstmt = ss = tmp;	/* rest of for and s_list */
						break;
					}
					}
				}
			}
			if (ss->e == dummy)
				ss->e = 0;
			else {
				ss->e = ss->e->typ(tbl);
				ss->e = check_cond(ss->e,FOR,tbl);
			}
			if (ss->s->base == DCL) error('s',"D as onlyS in for-loop");
			ss->s->dcl();
			ss->e2 = (ss->e2 == dummy) ? 0 : ss->e2->typ(tbl);
			curr_loop = old_loop;
			break;

		case DCL:	/* declaration after statement */
		{
			/*	collect all the contiguous DCL nodes from the
				head of the s_list. find the next statement
			*/
			int non_trivial = 0;
			int count = 0;
			Pname tail = ss->d;
			for (Pname nn=tail; nn; nn=nn->n_list) {
				//	find tail;
				//	detect non-trivial declarations
				count++;
//error('d',"dcl:%n list %d stc %d in %d",nn,nn->n_list,nn->n_sto,nn->n_initializer);
				if (nn->n_list) tail = nn->n_list;
				Pname n = tbl->look(nn->string,0);
				if (n && n->n_table==tbl) non_trivial = 2;
				if (non_trivial == 2) continue;
				if ((nn->n_sto==STATIC && nn->tp->base!=FCT)
				|| nn->tp->is_ref()
				|| (nn->tp->tconst() && fct_const==0)) {
					non_trivial = 2;
					continue;
				}
				Pexpr in = nn->n_initializer;
				if (in)
					switch (in->base) {
					case ILIST:
					case STRING:
						non_trivial = 2;
						continue;
					default:
						non_trivial = 1;
					}
				Pname cln = nn->tp->is_cl_obj();
				if (cln == 0) cln = cl_obj_vec;
				if (cln == 0) continue;
				if (Pclass(cln->tp)->has_ctor()) {
					non_trivial = 2;
					continue;
				}
				if (Pclass(cln->tp)->has_dtor()) non_trivial = 2;
			}
//error('d',"non_trivial %d",non_trivial);
			while( ss->s_list && ss->s_list->base==DCL ) {
				Pstmt sx = ss->s_list;
				tail = tail->n_list = sx->d;	// add to tail
				for (nn=sx->d; nn; nn=nn->n_list) {
					//	find tail;
					//	detect non-trivial declarations
					count++;
					if (nn->n_list) tail = nn->n_list;
					Pname n = tbl->look(nn->string,0);
					if (n && n->n_table==tbl) non_trivial = 2;
					if (non_trivial == 2) continue;
					if ((nn->n_sto==STATIC && nn->tp->base!=FCT)
					|| nn->tp->is_ref()
					|| (nn->tp->tconst() && fct_const==0)) {
						non_trivial = 2;
						continue;
					}
					Pexpr in = nn->n_initializer;
					if (in)
						switch (in->base) {
						case ILIST:
						case STRING:
							non_trivial = 2;
							continue;
						}
					non_trivial = 1;
					Pname cln = nn->tp->is_cl_obj();
					if (cln == 0) cln = cl_obj_vec;
					if (cln == 0) continue;
					if (Pclass(cln->tp)->has_ctor()) {
						non_trivial = 2;
						continue;
					}
					if (Pclass(cln->tp)->has_dtor()) continue;
				}
				ss->s_list = sx->s_list;
			/*	delete sx;	*/
			}
			Pstmt next_st = ss->s_list;
//error('d',"non_trivial %d curr_block->own_tbl %d inline_restr %d",non_trivial,curr_block->own_tbl,inline_restr);
			if (non_trivial==2	// must
			|| (non_trivial==1	// might
				&& ( curr_block->own_tbl==0	// why not?
				|| inline_restr&3		/* label seen */)
			  	)
			) {
				if (curr_switch && non_trivial==2) {
					Pstmt cs = curr_switch->case_list;
					Pstmt ds = curr_switch->has_default;
					Pstmt bl;
					if (cs == 0)
						bl = ds;
					else if (ds == 0)
						bl = cs;
					else if (cs->where.line<ds->where.line)
						bl = ds;
					else
						bl = cs;
//error('d',"bl %d %k bltbl %d tbl %d",bl,bl->base,bl->memtbl,tbl);
					if ((bl==0 || bl->s->base!=BLOCK) && curr_switch->s->memtbl==tbl)
						error('s',"non trivialD in switchS (try enclosing it in a block)");
				}

				/*	Create a new block,
					put all the declarations at the head,
					and the remainder of the slist as the
					statement list of the block.
				*/
				ss->base = BLOCK;

				/*	check that there are no redefinitions since the last
					"real" (user-written, non-generated) block
				*/
				for( nn=ss->d; nn; nn=nn->n_list ) {
					Pname n;
					if( curr_block->own_tbl
					&&  (n=curr_block->memtbl->look(nn->string,0))
					&&  n->n_table->real_block==curr_block->memtbl->real_block)
						error("twoDs of%n",n);
				}

				/*	attach the remainder of the s_list
					as the statement part of the block.
				*/
				ss->s = next_st;
				ss->s_list = 0;

				/*	create the table in advance, in order to set the
					real_block ptr to that of the enclosing table
				*/
				ss->memtbl = new table(count+4,tbl,0);
				ss->memtbl->real_block = curr_block->memtbl->real_block;

				Pblock(ss)->dcl(ss->memtbl);
			}
			else {	/*	to reduce the number of symbol tables,
					do not make a new block,
					instead insert names in enclosing block,
					and make the initializers into expression
					statements.
				*/
				Pstmt sss = ss;
				for( nn=ss->d; nn; nn=nn->n_list ) {
					Pname n = nn->dcl(tbl,FCT);
//error('d',"%n->dcl(%d) -> %d init %d sss=%d ss=%d",nn,tbl,n,n->n_initializer,sss,ss);
					if (n == 0) continue;
					Pexpr in = n->n_initializer;
					n->n_initializer = 0;
					if (ss) {
						sss->base = SM;
						ss = 0;
					}
					else
						sss = sss->s_list = new estmt(SM,sss->where,0,0);
					if (in) {
						switch (in->base) {
						case G_CALL:	/* constructor? */
						{
							Pname fn = in->fct_name;
							if (fn && fn->n_oper==CTOR) break;
						}
						default:
							in = new expr(ASSIGN,n,in);
						}
						sss->e = in->typ(tbl);
					}
					else
						sss->e = dummy;
				}
				ss = sss;
				ss->s_list = next_st;
			}
			break;
		}

		case BLOCK:
			Pblock(ss)->dcl(tbl);
			break;

		case ASM:
			/* save string */
		{
			char* s = (char*)ss->e;
			int ll = strlen(s);
			char* s2 = new char[ll+1];
			strcpy(s2,s);
			ss->e = Pexpr(s2);
			break;
		}

		default:
			error('i',"badS(%p %d)",ss,ss->base);
		}
	}

	Cstmt = ostmt;
}

extern int in_class_dcl;
void block::dcl(Ptable tbl)
/*
	Note: for a block without declarations memtbl denotes the table
	for the enclosing scope.
	A function body has its memtbl created by fct::dcl().
*/
{
	int bit_old = bit_offset;
	int byte_old = byte_offset;
	int max_old = max_align;
	Pblock block_old = curr_block;

	if (base != BLOCK) error('i',"block::dcl(%d)",base);

	curr_block = this;

	if (d) {
		own_tbl = 1;
		if (memtbl == 0) {
			int nmem = d->no_of_names()+4;
			memtbl = new table(nmem,tbl,0);
			memtbl->real_block = this;
			/*	this is a "real" block from the
				source text, and not one created by DCL's
				inside a block. */
		}
		else
			if (memtbl != tbl) error('i',"block::dcl(?)");

		Pname nx;
		for (Pname n=d; n; n=nx) {
			nx = n->n_list;
			n->dcl(memtbl,FCT);
			switch (n->tp->base) {
			case CLASS:
			case ANON:
			case ENUM:
				break;
			default:
				delete n;
			}
		}
	}
	else
		memtbl = tbl;

	if (s) {
		Pname odcl = Cdcl;
		Pname m;
		int i;

		s->dcl();

		if (own_tbl)
		for (m=memtbl->get_mem(i=1); m; m=memtbl->get_mem(++i)) {
			Ptype t = m->tp;

			if (in_class_dcl) m->lex_level -=1;
			if (t == 0) {
				if (m->n_assigned_to == 0)
				   error("label %sU",m->string);
				if (m->n_used == 0)
				   error('w',"label %s not used", m->string);
				continue;
			}
		ll:
			switch (t->base) {
			case TYPE:	t = Pbase(t)->b_name->tp; goto ll;
			case CLASS:
			case ENUM:
			case FCT:
			case VEC:	continue;
			}

			if (m->n_addr_taken == 0) {
				if (m->n_used) {
					if (m->n_assigned_to) {
					}
					else {
						switch (m->n_scope) {
						case FCT:
							Cdcl = m;
							error('w',"%n used but not set",m);
						}
					}
				}
				else {
					if (m->n_assigned_to) {
					}
					else {
						switch (m->n_scope) {
						case ARG:
							if (m->string[0]=='_' && m->string[1]=='A') break; /* generated name: cannot be used */
						case FCT:
							Cdcl = m;
							error('w',"%n not used",m);
						}
					}
				}
			}
		}
		Cdcl = odcl;
	}

	d = 0;

	if (bit_offset) byte_offset += SZ_WORD;
	if (stack_size < byte_offset) stack_size = byte_offset;
	bit_offset = bit_old;
	byte_offset = byte_old;
	curr_block = block_old;
}

void name::field_align()
// adjust alignment
{
		Pbase fld = (Pbase)tp;

		int a = (F_SENSITIVE) ? fld->b_fieldtype->align() : SZ_WORD;
		if (max_align < a) max_align = a;

		if (fld->b_bits == 0) {		// force word alignment
			int b;
			if (bit_offset)
				fld->b_bits = BI_IN_WORD - bit_offset;
			else if (b = byte_offset%SZ_WORD)
				fld->b_bits = b * BI_IN_BYTE;
			else
				fld->b_bits = BI_IN_WORD;
			if (max_align < SZ_WORD) max_align = SZ_WORD;
		}
		else if (bit_offset == 0) {	// take care of part of word
			int b = byte_offset%SZ_WORD;
			if (b) {
				byte_offset -= b;
				bit_offset = b*BI_IN_BYTE;
			}
		}
//error('d',"byteoff %d bitoff %d bits %d",byte_offset,bit_offset,fld->b_bits);
		int x = (bit_offset += fld->b_bits);
		if (BI_IN_WORD < x) {
			fld->b_offset = 0;
			byte_offset += SZ_WORD;
			bit_offset = fld->b_bits;
		}
		else {
			fld->b_offset = bit_offset;
			if (BI_IN_WORD == x) {
				bit_offset = 0;
				byte_offset += SZ_WORD;
			}
			else
				bit_offset = x;
		}
		n_offset = byte_offset;
}
