/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/dcl.c	1.1"
/*ident	"@(#)cfront:src/dcl.c	1.13" */
/**************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.

	
dcl.c:

	``declare'' all names, that is insert them in the appropriate symbol tables.

	Calculate the size for all objects (incl. stack frames),
	and find store the offsets for all members (incl. auto variables).
	"size.h" holds the constants needed for calculating sizes.

	Note that (due to errors) functions may nest

*****************************************************************************/


#include "cfront.h"
#include "size.h"

class dcl_context ccvec[MAXCONT], * cc = ccvec;
int byte_offset;
int bit_offset;
int max_align;
int stack_size;
int enum_count;
int friend_in_class = 0;

Pname dclass(Pname, Ptable);
Pname denum(Pname, Ptable);
void dargs(Pname, Pfct, Ptable);
void merge_init(Pname, Pfct, Pfct);

Pname name::dcl(Ptable tbl, TOK scope)
/*
	enter a copy of this name into symbol table "tbl";
		- create local symbol tables as needed
	
	"scope" gives the scope in which the declaration was found
		- EXTERN, FCT, ARG, PUBLIC, or 0
	Compare "scope" with the specified storage class "n_sto"
		- AUTO, STATIC, REGISTER, EXTERN, OVERLOAD, FRIEND, or 0

	After name::dcl()
	n_stclass ==	0		class or enum member
			REGISTER	auto variables declared register
			AUTO		auto variables not registers
			STATIC		statically allocated object
	n_scope ==	0		private class member
			PUBLIC		public class member
			EXTERN		name valid in this and other files
			STATIC		name valid for this file only
			FCT		name local to a function
			ARG		name of a function argument
			ARGT		name of a type defined in an
					argument list

	typecheck function bodies;
	typecheck initializers;

	note that functions (error recovery) and classes (legal) nest

	The return value is used to chain symbol table entries, but cannot
	be used for printout because it denotes the sum of all type information
	for the name

	names of typenames are marked with n_oper==TNAME

	WARNING: The handling of scope and storage class is cursed!
*/
{
	Pname nn;
	Ptype nnt = 0;
	Pname odcl = Cdcl;

//	if (this == 0) error('i',"0->N::dcl()");
//	if (tbl == 0) error('i',"%n->N::dcl(tbl=0,%k)",this,scope);
//	if (tbl->base != TABLE) error('i',"%n->N::dcl(tbl=%d,%k)",this,tbl->base,scope);
//	if (tp == 0) error('i',"N::dcl(%n,%k)T missing",this,scope);
/*fprintf(stderr,"(%d %s)->dcl(tbl=%d,scope=%d) tp = (%d %d)\n",this,string,tbl,scope,tp,tp->base); fflush(stderr);*/
	Cdcl = this;
	switch (base) {
	case TNAME:
		tp->dcl(tbl);
		PERM(tp);
		nn = new name(string);
		nn->base = TNAME;
		nn->tp = tp;
		(void) tbl->insert(nn,0);
		delete nn;
		Cdcl = odcl;
		return this;
	case NAME:
		switch (n_oper) {
		case TNAME:
			if (tp->base != FCT) n_oper = 0;
			break;
		case COMPL:
			if (tp->base != FCT) {
				error("~%s notF",string);
				n_oper = 0;
			}
			break;
		}
		break;
	default:
		error('i',"NX in N::dcl()");
	}

	if (n_qualifier) {	/*	class function: c::f(); */
		if (tp->base != FCT) {
			error("QdN%n inD of nonF",this);
			Cdcl = odcl;
			return 0;
		}

		Pname cn = n_qualifier;
		switch (cn->base) {
		case TNAME:
			break;
		case NAME:
			cn = gtbl->look(cn->string,0);
			if (cn && cn->base==TNAME) break;
		default:
			error("badQr%n for%n",n_qualifier,this);
			Cdcl = odcl;
			return 0;
		}

		cn = Pbase(cn->tp)->b_name;
		if (n_oper) check_oper(cn);

		Pclass cl = (Pclass)cn->tp;
		if (cl == cc->cot) {
			n_qualifier = 0;
			goto xdr;
		}
		else if ((cl->defined&(DEFINED|SIMPLIFIED)) == 0) {
			error("C%nU",cn);
			Cdcl = odcl;
			return 0;
		}

		Ptable etbl = cl->memtbl;
		Pname x = etbl->look(string,0);
		if(x==0 || x->n_table!=etbl) {
			error("%n is not aM of%n",this,cn);
			Cdcl = odcl;
			return 0;
		}
	}
xdr:
	if (n_oper && tp->base!=FCT && n_sto!=OVERLOAD)
		error("operator%k not aF",n_oper);


	/*	if a storage class was specified
			check that it is legal in the scope 
		else
			provide default storage class
		some details must be left until the type of the object is known
	*/

	n_stclass = n_sto;
	n_scope = scope;	/* default scope & storage class */

	switch (n_sto) {
	default:
		error('i',"unexpected %k",n_sto);
	case FRIEND:
	{
		Pclass cl = cc->cot;

		switch (scope) {
		case 0:
		case PUBLIC:
			break;
		default:
			error("friend%n not inCD(%k)",this,scope);
			base = 0;
			Cdcl = odcl;
			return 0;
		}

		switch (n_oper) {
		case 0:
		case NEW:
		case DELETE:
		case CTOR:
		case DTOR:
		case TYPE:
			n_sto = 0;
			break;
		default:
			n_sto = OVERLOAD;
		}

		switch (tp->base) {
	/*	case INT:	 undefined: implicitly define as class
			nn = tname(CLASS);
			nn->tp->dcl(gtbl);
			break;
	*/
		case COBJ:
			nn = Pbase(tp)->b_name;
			break;
		case CLASS:
			nn = this;
			break;
		case FCT:
			cc->stack();
			cc->not = 0;
			cc->tot = 0;
			cc->cot = 0;
			friend_in_class++;
			n_sto = EXTERN;
			nn = dcl(gtbl,EXTERN);
			friend_in_class--;
			cc->unstack();
			if (nn->tp->base == OVERLOAD) {
				Pgen g = (Pgen)nn->tp;
				nn = g->find((Pfct)tp,1);
			}
			break;
		default:
			error("badT%t of friend%n",tp,this);
		}
		PERM(nn);
		cl->friend_list = new name_list(nn,cl->friend_list);
		Cdcl = odcl;
		return nn;
	}
	case OVERLOAD:
		n_sto = 0;
		switch (scope) {
		case 0:
		case PUBLIC:
			error('w',"overload inCD (ignored)");
			switch (tp->base) {
			case INT:
				base = 0;
				Cdcl = odcl;
				return this;
			case FCT:
				return dcl(tbl,scope);
			}
		}
		if (n_oper && tp->base==FCT) break;
		nn = tbl->insert(this,0);

		if (Nold) {
			if (nn->tp->base != OVERLOAD) {
				error("%n redefined as overloaded",this);
				nn->tp = new gen(string);
			}
		}
		else {
			nn->tp = new gen(string);
		}

		switch (tp->base) {
		case INT:
			base = 0;
			Cdcl = odcl;
			return nn;
		case FCT:
			break;
		default:
			error("N%n ofT%k cannot be overloaded",this,tp->base);
			Cdcl = odcl;
			return nn;
		}
		break;
	case REGISTER:
		if (tp->base == FCT) {
			error('w',"%n: register (ignored)",this);
			goto ddd;
		}
	case AUTO:
		switch (scope) {
		case 0:
		case PUBLIC:
		case EXTERN:
			error("%k not inF",n_sto);
			goto ddd;
		}	
		if (n_sto != REGISTER) n_sto = 0;	
		break;
	case EXTERN:
		switch (scope) {
		case ARG:
			error("externA");
			goto ddd;
		case 0:
		case PUBLIC:
			/* extern is provided as a default for functions without body */
			if (tp->base != FCT) error("externM%n",this);
			goto ddd;
		}
		n_stclass = STATIC;
		n_scope = EXTERN;	/* avoid FCT scoped externs to allow better checking */
		break;
	case STATIC:
		switch (scope) {
		case ARG:
			error("static used forA%n",this);
			goto ddd;
		case 0:
		case PUBLIC:
			n_stclass = STATIC;
			n_scope = scope;
			break;
		default:
			n_scope = STATIC;
		}
		break;
	case 0:
	ddd:
		switch (scope) {	/* default storage classes */
		case EXTERN:
			n_scope = EXTERN;
			n_stclass = STATIC;
			break;
		case FCT:
			if (tp->base == FCT) {
				n_stclass = STATIC;
				n_scope = EXTERN;
			}
			else
				n_stclass = AUTO;
			break;
		case ARG:
			n_stclass = AUTO;
			break;
		case 0:
		case PUBLIC:
			n_stclass = 0;
			break;
		}
	}

	
	/*
		now insert the name into the appropriate symbol table,
		and compare types with previous declarations of that name

		do type dependent adjustments of the scope
	*/

	switch (tp->base) {
	case ASM:
	{	Pbase b = (Pbase)tp;
		Pname n = tbl->insert(this,0);
		n->assign();
		n->use();
		char* s = (char*) b->b_name;	// save asm string. Shoddy
		int ll = strlen(s);
		char* s2 = new char[ll+1];
		strcpy(s2,s);
		b->b_name = Pname(s2);
		return this;
		}

	case CLASS:
		nn = dclass(this,tbl);
		Cdcl = odcl;
		return nn;

	case ENUM:
		nn = denum(this,tbl);
		Cdcl = odcl;
		return nn;

	case FCT:
		nn = dofct(tbl,scope);
		if (nn == 0) {
			Cdcl = odcl;
			return 0;
		}
		break;

	case FIELD:
		switch (n_stclass) {
		case 0:
		case PUBLIC:
			break;
		default:
			error("%k field",n_stclass);
			n_stclass = 0;
		}

		if (cc->not==0 || cc->cot->csu==UNION) {
			error(cc->not?"field in union":"field not inC");
			PERM(tp);
			Cdcl = odcl;
			return this;
		}

		if (string) {
			nn = tbl->insert(this,0);
			n_table = nn->n_table;
			if (Nold) error("twoDs of field%n",this);
		}

		tp->dcl(tbl);
		field_align();
		break;

	case COBJ:
	{	Pclass cl = Pclass(Pbase(tp)->b_name->tp);

		if (cl->csu == ANON) {	// export member names to enclosing scope
		//	if (tbl == gtbl) error('s',"global anonymous union");

			char* p = cl->string;
			while (*p++ != 'C');	/* UGH!!! */
			int uindex = str_to_int(p);

		// cannot cope with use counts for ANONs:
			Pbase(tp)->b_name->n_used = 1;
			Pbase(tp)->b_name->n_assigned_to = 1;

			Ptable mtbl = cl->memtbl;
			int i;
			for (Pname nn=mtbl->get_mem(i=1); nn; nn=mtbl->get_mem(++i)) {
				if (nn->tp->base == FCT) {
					error('s',"M%n for anonymous union",nn);
					break;
				}
				Ptable tb = nn->n_table;
				nn->n_table = 0;
				Pname n = tbl->insert(nn,0);
				if (Nold) {
					error("twoDs of%n (one in anonymous union)",nn);
					break;
				}
				n->n_union = uindex;
				nn->n_table = tb;
			}
		}
		goto cde;
	}

	case VEC:
	case PTR:
	case RPTR:
		tp->dcl(tbl);

	default:
	cde:
		nn = tbl->insert(this,0);
//error('d',"cde: %d->insert(%n) -> %d",tbl,nn,Nold);

		n_table = nn->n_table;
		if (Nold) {
			if (nn->tp->base == ANY) goto zzz;

			if (tp->check(nn->tp,0)) {
				error("twoDs of%n;Ts:%t and%t",this,nn->tp,tp);
				Cdcl = odcl;
				return 0;
			}

			if (n_sto && n_sto!=nn->n_scope) 
				error("%n declared as both%k and%k",this, n_sto,
				     (nn->n_sto)?nn->n_sto
						:((scope==FCT)?AUTO:EXTERN));
			else if (nn->n_scope==STATIC && n_scope==EXTERN)
				error("%n both static and extern",this);
			else if (nn->n_sto==STATIC && n_sto==STATIC ) 
				error("static%n declared twice",this);
			else {
				if (n_sto==0
				&& nn->n_sto==EXTERN
				&& n_initializer
				&& tp->tconst())
					n_sto = EXTERN;
				n_scope = nn->n_scope;

				switch (scope) {
				case FCT:
					error("twoDs of%n",this);
					Cdcl = odcl;
					return 0;
				case ARG:
					error("twoAs%n",this);
					Cdcl = odcl;
					return 0;
				case 0:
				case PUBLIC:
					error("twoDs ofM%n",this);
					Cdcl = odcl;
					return 0;
				case EXTERN:
					if (fct_void==0
					&& n_sto==0
					&& nn->n_sto==0) {
						error("two definitions of%n",this);
						Cdcl = odcl;
						return 0;
					}
				}
			}
			n_scope = nn->n_scope;
/* n_val */
			if (n_initializer) {
				if (nn->n_initializer || nn->n_val) error("twoIrs for%n",this);
				nn->n_initializer = n_initializer;
			}
			if (tp->base == VEC) {
			//	handle:	 extern v[]; v[200];

				Ptype ntp = nn->tp;
				while (ntp->base == TYPE) ntp = Pbase(ntp)->b_name->tp;

				if (Pvec(ntp)->dim == 0) Pvec(ntp)->dim = Pvec(tp)->dim;
				if (Pvec(ntp)->size == 0) Pvec(ntp)->size = Pvec(tp)->size;
			}
		}
		else {	// check for the ambiguous plain "int a;"
		/*	if (n_initializer==0
			&&  n_sto==0
			&&  scope==EXTERN) {
				error('w',"%n does not have storageC or initializer",this);
			}
		*/
			if (scope!=ARG && n_sto!=EXTERN && n_initializer==0 && tp->base==VEC && Pvec(tp)->size==0)
				error(&where,"dimension missing for vector%n",this);
		}
	
	zzz:
		if (base != TNAME) {
			Ptype t = nn->tp;
//fprintf(stderr,"tp %d %d nn->tp %d %d\n",tp,tp->base,nn->tp,nn->tp?nn->tp->base:0); fflush(stderr);
			if (t->base == TYPE) {
				Ptype tt = Pbase(t)->b_name->tp;
				if (tt->base == FCT) nn->tp = t = tt;
			}

			switch (t->base) {
			case FCT:
			case OVERLOAD:
				break;
			default:
				fake_sizeof = 1;
				switch (nn->n_stclass) {
				default:
					if (nn->n_scope != ARG) {
						int x = t->align();
						int y = t->tsizeof();

						if (max_align < x) max_align = x;

						while (0 < bit_offset) {
							byte_offset++;
							bit_offset -= BI_IN_BYTE;
						}
						bit_offset = 0;

						if (byte_offset && 1<x) byte_offset = ((byte_offset-1)/x)*x+x;
						nn->n_offset = byte_offset;
						byte_offset += y;
					}
					break;
				case STATIC:
					t->tsizeof();	// check that size is known
				}
				fake_sizeof = 0;
			}
		}

	{	Ptype t = nn->tp;
		int const_old = const_save;
		bit vec_seen = 0;
		Pexpr init = n_initializer;

		if (init) {
			switch (n_scope) {
			case 0:
			case PUBLIC:
				if (n_stclass != STATIC) error("Ir forM%n",this);
				break;
			}
		}

	/*	if (n_scope == EXTERN) break;		*/

	lll:
		switch (t->base) {
		case RPTR:
			if (init) {
				if (nn->n_scope == ARG) break;
				init = init->typ(tbl);
				if (n_sto==STATIC && init->lval(0)==0)
					error("Ir for staticR%n not an lvalue",this);
				else
					nn->n_initializer = n_initializer = ref_init(Pptr(t),init,tbl);
				nn->assign();
			}
			else {
				switch (nn->n_scope) {
				default:
					if (n_sto == EXTERN) break;
					error("unIdR%n",this);
				case ARG:
					break;
				case PUBLIC:
				case 0:
					if (n_sto == STATIC) error("a staticM%n cannot be aR",this);
					break;
				}
			}
			break;
		case COBJ:
		{	Pname cn = Pbase(t)->b_name;
			Pclass cl = (Pclass)cn->tp;
			Pname ctor = cl->has_ctor();
			Pname dtor = cl->has_dtor();
//error('d',"c/dtor %n %n %n init %d",cn,ctor,dtor,init);
			if (dtor) {
				Pstmt dls;
				switch ( nn->n_scope ) {
				case EXTERN:
					if (n_sto==EXTERN) break;
				case STATIC:
				{	Ptable otbl = tbl;
						// to collect temporaries generated
						// in static destructors where we
						// can find them again (in std_tbl)
					if (std_tbl == 0) std_tbl = new table(8,gtbl,0);
					tbl = std_tbl;
					if (vec_seen) {	/*  _vec_delete(vec,noe,sz,dtor,0); */
						int esz = cl->tsizeof();
						Pexpr noe = new ival(nn->tp->tsizeof()/esz);
						Pexpr sz = new ival(esz);
						Pexpr arg = new expr(ELIST,dtor,zero);
						dtor->lval(ADDROF);
						arg = new expr(ELIST,sz,arg);
						arg = new expr(ELIST,noe,arg);
						arg = new expr(ELIST,nn,arg);
						arg = new call(vec_del_fct,arg);
						arg->base = G_CALL;
						arg->fct_name = vec_del_fct;
						arg->tp = any_type;	// avoid another type check
						dls = new estmt(SM,nn->where,arg,0);
					}
					else {	// nn->cl::~cl(0);
						Pref r = new ref(DOT,nn,dtor);
						Pexpr ee = new expr(ELIST,zero,0);
						Pcall dl = new call(r,ee);
						dls = new estmt(SM,nn->where,dl,0);
						dl->base = G_CALL;
						dl->fct_name = dtor;
						dl->tp = any_type;	// avoid another check
					}
					// destructors for statics are executed in reverse order
					if (st_dlist) dls->s_list = st_dlist;
					st_dlist = dls;
					tbl = otbl;
				}
				}
			}
			if (ctor)	{
				Pexpr oo = nn;
				for (int vi=vec_seen; vi; vi--) oo = oo->contents();
				int sti = 0;
//error('d',"ctor init=%d n_scope=%d",init,nn->n_scope);
				switch (nn->n_scope) {
				case EXTERN:
					if (init==0 && n_sto==EXTERN) goto ggg;
				case STATIC:
					sti = 1;
					if (tbl != gtbl) {
// prohibited only to avoid having to handle local variables in the
// constructors argument list
	error('s',"local static%n ofCWK",this);
					}
				default:
					if (vec_seen && init) {
						error("Ir forCO%n\[\]",this);
						n_initializer = init = 0;
					}
					break;
				case ARG:
//error('d',"init %d",init);
/*
					if (init) {
						// check default arguments
						init = new texpr(VALUE,cl,0);
						init->e2 = oo;
						nn->n_initializer
							= n_initializer
							= init
							= init->typ(tbl);
					}
*/
				case PUBLIC:
				case 0:
					goto ggg;
				}
				const_save = 1;
				nn->assign();
//error('d',"init %d %n tbl %d",init,nn,tbl);
				Ptable otbl = tbl;
				if (sti) {	// to collect temporaries generated
						// in static initializers where we
						// can find them again (in sti_tbl)
					if (sti_tbl == 0) sti_tbl = new table(8,gtbl,0);
					tbl = sti_tbl;
					if (n_sto == EXTERN) nn->n_sto = n_sto = 0;
				}
//error('d',"init %d %d vec_seen %d",init,init?init->base:0,vec_seen);
				if (init) {
					if (init->base==VALUE) {
//error('d',"value %d",init->tp2->base);
						switch (init->tp2->base) {
						case CLASS:
							if (Pclass(init->tp2)!=cl) goto inin;
							break;
						default:
							Pname  n2 = init->tp2->is_cl_obj();
							if (n2==0 || Pclass(n2->tp)!=cl) goto inin;
						}
						init->e2 = oo;
						init = init->typ(tbl);
						if (init->base == G_CM)	// beware of type conversion operators
						switch (init->tp2->base) {
						case CLASS:
							if (Pclass(init->tp2)!=cl) goto inin;
							break;
						default:
							Pname  n2 = init->tp2->is_cl_obj();
							if (n2==0 || Pclass(n2->tp)!=cl) goto inin;
						}
					}
					else {
					inin:
//error('d',"inin1: %d %k",init->base,init->base);
						init = init->typ(tbl);
						if (init->base==G_CM && nn->tp->check(init->tp,ASSIGN)==0)
							(void) replace_temp(init,nn->address());
						else
							init = class_init(nn,nn->tp,init,tbl);
//error('d',"iii init=%d",init);
					}
				}
				else {
					init = new texpr(VALUE,cl,0);
					init->e2 = oo;
					init = init->typ(tbl);
				}
				Pname c;
				if (vec_seen) {
					c = cl->has_ictor();
					if (c == 0)
						error("vector ofC%n that does not have aK taking noAs",cn);
					else if (Pfct(c->tp)->nargs)
						error('s',"defaultAs forK for vector ofC%n",cn);
				}
				
	if (sti) {
		if (vec_seen) {	// _vec_new(vec,noe,sz,ctor);
			int esz = cl->tsizeof();
			Pexpr noe = new ival(nn->tp->tsizeof()/esz);
			Pexpr sz = new ival(esz);
			Pexpr arg = new expr(ELIST,c,0);
			c->lval(ADDROF);
			arg = new expr(ELIST,sz,arg);
			arg = new expr(ELIST,noe,arg);
			arg = new expr(ELIST,nn,arg);
			init = new call(vec_new_fct,arg);
			init->base = G_CALL;
			init->fct_name = vec_new_fct;
			init->tp = any_type;
		}
		else {
//error('d',"init%n: %k",nn,init->base);
			switch (init->base) {
			case DEREF:		// *constructor?
				if (init->e1->base == G_CALL) {	
					Pname fn = init->e1->fct_name;
					if (fn==0 || fn->n_oper!=CTOR) goto as;
					init = init->e1;
					break;
				}
				goto as;
			case ASSIGN:
				if (init->e1 == nn) break;	// simple assignment
			as:	
			default:	
				init = new expr(ASSIGN,nn,init);
			}
		}
		Pstmt ist = new estmt(SM,nn->where,init,0);
		// constructors for statics are executed in order
		static Pstmt itail = 0;
		if (st_ilist == 0)
			st_ilist = ist;
		else
			itail->s_list = ist;
		itail = ist;
		init = 0;
	} // if (sti)
				nn->n_initializer = n_initializer = init;
				const_save = const_old;
				tbl = otbl;
			}
			else if (init == 0)		/* no initializer */
				goto str;
			else if (cl->is_simple()
				&& cl->csu!=UNION
				&& cl->csu!=ANON) {	// struct
				if ( init->base == ILIST &&
					(cl->defined&(DEFINED|SIMPLIFIED))==0)
					{
						error("struct%nU: cannotI%n",cn,nn);
						Cdcl = odcl;
						return 0;
					}
				init = init->typ(tbl);
				if (nn->tp->check(init->tp,ASSIGN)==0
				&& init->base==G_CM) 
					(void) replace_temp(init,nn->address());
				else
					goto str;
			}
			else if (init->base == ILIST) {	// class or union
				error("cannotI%nWIrL",nn);
			}
			else {			// bitwise copy ok?
						// possible to get here?
//error('d',"not simple %n",this);
				init = init->typ(tbl);
				if (nn->tp->check(init->tp,ASSIGN)==0) {
					if (init->base==G_CM) 
						(void) replace_temp(init,nn->address());
					else
						goto str;
				}
				else
					error("cannotI%n:%k %s has noK",nn,cl->csu,cl->string);
			}
			break;
		}
		case VEC:	
			t = Pvec(t)->typ;
			vec_seen++;
			goto lll;
		case TYPE:
			if (init==0 && Pbase(t)->b_const) {
				switch (n_scope) {
				case ARG:
				case 0:
				case PUBLIC:
					break;
				default:
					if (n_sto!=EXTERN && t->is_cl_obj()==0) error("unId const%n",this);
				}
			}
			t = Pbase(t)->b_name->tp;
			goto lll;
		default:
		str:
//error('d',"str: %n",this);
			if (init == 0) {
				switch (n_scope) {
				case ARG:
				case 0:
				case PUBLIC:
					break;
				default:
					if (n_sto!=EXTERN && t->tconst()) error("unId const%n",this);
				}

				break;
			}

			const_save = 	   const_save
					|| n_scope==ARG
					|| (t->tconst() && vec_const==0);
			nn->n_initializer = n_initializer = init = init->typ(tbl);
			if (const_save) PERM(init);
			nn->assign();
			const_save = const_old;

			switch (init->base) {
			case ILIST:
				new_list(init);
				list_check(nn,nn->tp,0);
				if (next_elem()) error("IrL too long");
				break;
			case STRING:
				if (nn->tp->base == VEC) {
					Pvec v = (Pvec)nn->tp;
					if (v->typ->base == CHAR) {
					/*	error('w',"\"char[] = string\"");*/
						int sz = v->size;
						int isz = Pvec(init->tp)->size;
						if (sz == 0)
							v->size = isz;
						else if (sz < isz)
							error("Ir too long (%d characters) for%n[%d]",isz,nn,sz);
						break;
					}
				}
			default:
			{	Ptype nt = nn->tp;
				int ntc = Pbase(nt)->b_const;

				if (vec_seen) {
					error("badIr for vector%n",nn);
					break;
				}
			tlx:
				switch (nt->base) {
				case TYPE:
					nt = Pbase(nt)->b_name->tp;
					ntc |= Pbase(nt)->b_const;
					goto tlx;
				case INT:
				case CHAR:
				case SHORT:
					if (init->base==ICON && init->tp==long_type)
						error('w',"longIr constant for%k%n",nn->tp->base,nn);
				case LONG:
					if (Pbase(nt)->b_unsigned
					&& init->base==UMINUS
					&& init->e2->base==ICON)
						error('w',"negativeIr for unsigned%n",nn);
					if (ntc && scope!=ARG) {
						int i;
						Neval = 0;
						i = init->eval();
						if (Neval == 0) {
							DEL(init);
							nn->n_evaluated = n_evaluated = 1;
							nn->n_val = n_val = i;
							nn->n_initializer = n_initializer = 0;
						}
					}
					break;
				case PTR:
					n_initializer = init = ptr_init(Pptr(nt),init,tbl);
				} 

			{	Pexpr x = try_to_coerce(nt,init,"initializer",tbl);
				if (x) {
					n_initializer = x;
					goto stgg;
				}
			}

			if (nt->check(init->tp,ASSIGN))
				error("badIrT%t for%n (%tX)",init->tp,this,nn->tp);
			else {
			stgg:
				if (init && n_stclass== STATIC) {
					/* check if non-static variables are used */
					/* INCOMPLETE */
					switch (init->base) {
					case NAME:
						if (init->tp->tconst()==0) error("V%n used inIr for%n",init,nn);
						break;
					case DEREF:
					case DOT:
					case REF:
					case CALL:
					case G_CALL:
					case NEW:
						error("%k inIr of static%n",init->base,nn);
					}
			}
			}
		}
		} /* switch */
	} /* block */
	} /* default */

	} /* switch */
ggg:
	PERM(nn);
	switch (n_scope) {
	case FCT:
		nn->n_initializer = n_initializer;
		break;
	default:
	{/*	Pexpr ii = nn->n_initializer;*/
		Ptype t = nn->tp;
	/*	if (ii) PERM(ii);*/
	px:
		PERM(t);
		switch (t->base) {
		case PTR:
		case RPTR:
		case VEC:	t = Pptr(t)->typ; goto px;
		case TYPE:	t = Pbase(t)->b_name->tp; goto px;
		case FCT:	t = Pfct(t)->returns; goto px; /* args? */
		} 
	}
	}
	
	Cdcl = odcl;
	return nn;
}
