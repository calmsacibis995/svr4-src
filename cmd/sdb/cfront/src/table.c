/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/table.c	1.1"
/*ident	"@(#)cfront:src/table.c	1.6" */

/**************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.

	
table.c:

*****************************************************************************/

#include "cfront.h"

char* keys[MAXTOK+1];
/*
	keys[]  holds the external form for tokens with fixed representation 
	illegal tokens and those with variable representation have 0 entries
*/

/*
	the class table functions assume that new initializes store to 0
*/

table::table(short sz, Ptable nx, Pname n)
/*
	create a symbol table with "size" entries
	the scope of table is enclosed in the scope of "nx"

	both the vector of class name pointers and the hash table
	are initialized containing all zeroes
	
	to simplify hashed lookup entries[0] is never used
	so the size of "entries" must be "size+1" to hold "size" entries
*/
{
	base = TABLE;
	t_name = n;
	size = sz = (sz<=0) ? 2 : sz+1;
//fprintf(stderr,"table::table %d %s %d (%d %d)\n", this, (n)?n->string:"?", sz,(sz*3)/2);
	entries = new Pname[sz];
	hashsize = sz = (sz*3)/2;
	hashtbl = new short[sz];
	next = nx;
	free_slot = 1;
} 


Pname table::look(char* s, TOK k)
/*
	look for "s" in table, ignore entries which are not of "k" type
	look and insert MUST be the same lookup algorithm
*/
{
	Ptable t;
	register char * p;
	register char * q;
	register int i;
	Pname n;
	int rr;

//	if (s == 0) error('i',"%d->look(0)",this);
//	if (this == 0) error('i',"0->look(%s)",s);
//	if (base != TABLE) error('i',"(%d,%d)->look(%s)",this,base,s);

	/* use simple hashing with linear search for overflow */

	p = s;
	i = 0;
	while (*p) i += (i + *p++); /* i<<1 ^ *p++ better?*/
	rr = (0<=i) ? i : -i;

	for (t=this; t; t=t->next) {	
		/* in this and all enclosing scopes look for name "s" */
		Pname* np = t->entries;
		int mx = t->hashsize;
		short* hash = t->hashtbl;
		int firsti = i = rr%mx;

		do {			
			if (hash[i] == 0) goto not_found;
			n = np[hash[i]];
			if (n == 0) error('i',"hashed lookup");
			p = n->string;		/* strcmp(n->n_string,s) */
			q = s;
			while (*p && *q)
				if (*p++ != *q++) goto nxt;
			if (*p == *q) goto found;
		nxt:
			if (mx <= ++i) i = 0;		/* wrap around */
		} while (i != firsti);

	found:
		for (; n; n=n->n_tbl_list){	/* for  all name "s"s look for a key match */
			if (n->n_key == k) return n;
		}

	not_found:;
	}

	return 0;	/* not found && no enclosing scope */
}

bit Nold;	/* non-zero if last insert() failed */

Pname table::insert(Pname nx, TOK k)
/*
	the lookup algorithm MUST be the same as look
	if nx is found return the older entry otherwise a copy of nx;
	Nold = (nx found) ? 1 : 0;
*/
{
	register char * p;
	register int i;
	Pname n;
	Pname* np = entries;
	Pname* link;
	int firsti;
	int mx = hashsize;
	short* hash = hashtbl;
	char* s = nx->string;

	if (s==0) error('i',"%d->insert(0,%d)",this,k);
	nx->n_key = k;
	if (nx->n_tbl_list || nx->n_table) error('i',"%n in two tables",nx);
	/* use simple hashing with linear search for overflow */

	p = s;
	i = 0;
	while (*p) i += (i + *p++);
	if (i<0) i = -i;
	firsti = i = i%mx;

	do {	/* look for name "s" */
		if (hash[i] == 0) {
			hash[i] = free_slot;
			goto add_np;
		}
		n = np[hash[i]];
		if (n == 0) error('i',"hashed lookup");
		if (strcmp(n->string,s) == 0) goto found;
/*
		p = n->string;
		q = s;
		while (*p && *q) if (*p++ != *q++) goto nxt;
		if (*p == *q) goto found;
	nxt:
*/
		if (mx <= ++i) i = 0;	/* wrap around */
	} while (i != firsti);

	error("N table full");

found:	


	for(;;) {
		if (n->n_key == k) { Nold = 1; return n; }

		if (n->n_tbl_list)
			n = n->n_tbl_list;
		else {
			link = &(n->n_tbl_list);
			goto re_allocate;
		}
	}

add_np:
	if (size <= free_slot) {
		grow(2*size);
		return insert(nx,k);
	}

	link = &(np[free_slot++]);

re_allocate:
	{	
		Pname nw = new name;
		*nw = *nx;
		char* ps = new char[strlen(s)+1]; // copy string to safer store
		strcpy(ps,s);
		Nstr++;
		nw->string = ps;
		nw->n_table = this;
		*link = nw;
		Nold = 0;
		Nname++;
		return nw;
	}
}

void table::grow(int g)
{
	short* hash;
	register int j;
	int mx; 
	register Pname* np;
	Pname n;

	if (g <= free_slot) error('i',"table.grow(%d,%d)",g,free_slot);
	if (g <= size) return;
/* fprintf(stderr,"tbl.grow %d %s %d->%d\n", this, (t_name)?t_name->string:"?", size, g+1); fflush(stderr); */
	size = mx = g+1;

	np = new Pname[mx];
	for (j=0; j<free_slot; j++) np[j] = entries[j];
	delete entries;
	entries = np;

	delete hashtbl;
	hashsize = mx = (g*3)/2;
	hash = hashtbl = new short[mx];

	for (j=1; j<free_slot; j++) {	/* rehash(np[j]); */
		char * s = np[j]->string;
		register char * p;
		char * q;
		register int i;
		int firsti;

		p = s;
		i = 0;
		while (*p) i += (i + *p++);
		if (i<0) i = -i;
		firsti = i = i%mx;

		do {	/* look for name "s" */
			if (hash[i] == 0) {
				hash[i] = j;
				goto add_np;
			}
			n = np[hash[i]];
			if (n == 0) error('i',"hashed lookup");
			p = n->string;	/* strcmp(n->n_string,s) */
			q = s;
			while (*p && *q) if (*p++ != *q++) goto nxt;
			if (*p == *q) goto found;
		nxt:
			if (mx <= ++i) i = 0;	/* wrap around */
		} while (i != firsti);

		error('i',"rehash??");

	found:
		error('i',"rehash failed");

	add_np:;
	}
}

Pclass Ebase;
Pclass Epriv;	/* extra return values from lookc() */

Pname table::lookc(char* s, TOK)
/*
	like look().

	look and insert MUST be the same lookup algorithm

*/
{
	Ptable t;
	register char * p;
	register char * q;
	register int i;
	Pname n;
	int rr;

//	if (s == 0) error('i',"%d->look(0)",this);
//	if (this == 0) error('i',"0->look(%s)",s);
//	if (base != TABLE) error('i',"(%d,%d)->look(%s)",this,base,s);

	Ebase = 0;
	Epriv = 0;

	/* use simple hashing with linear search for overflow */

	p = s;
	i = 0;
	while (*p) i += (i + *p++);
	rr = (0<=i) ? i : -i;

	for (t=this; t; t=t->next) {	
		/* in this and all enclosing scopes look for name "s" */
		Pname* np = t->entries;
		int mx = t->hashsize;
		short* hash = t->hashtbl;
		int firsti = i = rr%mx;
		Pname tname = t->t_name;

		do {			
			if (hash[i] == 0) goto not_found;
			n = np[hash[i]];
			if (n == 0) error('i',"hashed lookup");
			p = n->string;		/* strcmp(n->n_string,s) */
			q = s;
			while (*p && *q)
				if (*p++ != *q++) goto nxt;
			if (*p == *q) goto found;
		nxt:
			if (mx <= ++i) i = 0;		/* wrap around */
		} while (i != firsti);

	found:
		do {	// for  all name "s"s look for a key match
			if (n->n_key == 0) {
				if (tname) {
					if (n->base == PUBLIC)
						n = n->n_qualifier;
					else if (n->n_scope == 0)
						Epriv = (Pclass)tname->tp;
				}
				return n;
			}
		} while (n=n->n_tbl_list);


	not_found:
		if (tname) {
			Pclass cl = (Pclass)tname->tp;
			if (cl && cl->clbase && cl->pubbase==0) Ebase = (Pclass)cl->clbase->tp;
		}
	}

	Ebase = Epriv = 0;
	return 0;	/* not found && no enclosing scope */
}


Pname table::get_mem(int i)
/*
	return a pointer to the i'th entry, or 0 if it does not exist
*/
{
	return (i<=0 || free_slot<=i) ? 0 : entries[i];
}

void new_key(char* s, TOK toknum, TOK yyclass)
/*
	make "s" a new keyword with the representation (token) "toknum"
	"yyclass" is the yacc token (for example new_key("int",INT,TYPE); )
	"yyclass==0" means yyclass=toknum;
*/
{
	Pname n = new name(s);
	Pname nn = ktbl->insert(n,0);
//	if (Nold) error('i',"keyword %sD twice",s);
	nn->base = toknum;
	nn->syn_class = (yyclass) ? yyclass : toknum;
	keys[(toknum==LOC)?yyclass:toknum] = s;
	delete n;
}


Pexpr table::find_name(register Pname n, bit f, Pexpr/* args*/)
/*
	find the true name for "n", implicitly define if undefined
	f==1:	n()
	f==2:	r->n or o.n
	f==3:	&n

*/
{
	Pname q = n->n_qualifier;
	register Pname qn = 0;
	register Pname nn;
	Pclass cl;	/* class specified by q */
//error('d',"%p->find_name(%n,%d) gtbl==%p",this,n,f,gtbl);
	if (n->n_table) {
		nn = n;
		n = 0;
		if (f == 3) f = 0;
		goto xx;
	}

	if (q) {
		Ptable tbl;
//error('d',"qq %n %n",q,n);
		if (q == sta_name)
			tbl = gtbl;
		else {
			Ptype t = Pclass(q->tp);
			if (t == 0) error('i',"Qr%n'sT missing",q);

			if (q->base == TNAME) {
				if (t->base != COBJ) {
					error("badT%k forQr%n",t->base,q);
					goto nq;
				}
				t = Pbase(t)->b_name->tp;
			}
			if (t->base != CLASS) {
				error("badQr%n(%k)",q,t->base);
				goto nq;
			}
			cl = Pclass(t);
			tbl = cl->memtbl;
		}

		qn = tbl->look(n->string,0);
//error('d',"qn == %d",qn);
		if (qn == 0) {
			n->n_qualifier = 0;
			nn = 0;
			goto def;
		}

		if (q == sta_name) {	/* explicitly global */
			qn->use();
			delete n;
			return qn;
		}

		/* else check visibility */
	}
	else
		if (f == 3) f = 0;

nq:
	if (cc->tot) {
	{	for (Ptable tbl = this;;) {	// loop necessary to get past
						// local re-definitions
			nn = tbl->lookc(n->string,0);
//error('d',"cc->tot:%n nn=%n sto%k sco%k tbl=%d",n,nn,nn->n_stclass,nn->n_scope,tbl);
			if (nn == 0) goto qq;	/* try for friend */

			switch (nn->n_scope) {
			case 0:
			case PUBLIC:
				if (nn->n_stclass == ENUM) break;

				if (nn->tp->base == OVERLOAD) break;

				if (Ebase
				&& cc->cot->clbase
				&& Ebase!=cc->cot->clbase->tp
				&& !Ebase->has_friend(cc->nof))
					error("%n is from a privateBC",n);

				if (Epriv
				&& Epriv!=cc->cot
				&& !Epriv->has_friend(cc->nof)
				&& !(nn->n_protect && Epriv->baseof(cc->nof)))
					error("%n is %s",n,nn->n_protect?"protected":"private");
			}

			if (qn==0 || qn==nn) break;

			if ((tbl=tbl->next) == 0) {	/* qn/cl test necessary? */
				if (/* (qn->n_stclass==STATIC
					|| qn->tp->base==FCT
					|| qn->tp->base==OVERLOAD)
				&&  */ (	qn->n_scope==PUBLIC
					|| cl->has_friend(cc->nof)) ) {
					nn = qn;
					break;
				}
				else {
					if (f != 3) {
						error("QdN%n not in scope",n);
						goto def;
					}
					break;
				}
			}
		}
	}
	xx:
//error('d',"xx: nn=%n qn=%n n=%n f=%d",nn,qn,n,f);
		if (nn == 0) goto def;
		nn->use();
		if (2 <= f) {
			if (qn && nn->n_stclass==0)
				switch (nn->n_scope) {
				case 0:
				case PUBLIC:	/* suppress virtual */
					switch (qn->tp->base) {
					case FCT:
					case OVERLOAD:
						if (f == 3) return qn;
						*n = *qn;
						n->n_qualifier = q;
						return n;
					}
				}
			if (nn->n_table == gtbl) error("M%n not found",n);
			if (n) delete n;
			return nn;
		}

		switch (nn->n_scope) {
		case 0:
		case PUBLIC:
//error('d',"st %d th %d",nn->n_stclass,cc->c_this);
			switch (nn->n_stclass) {
			case 0:
				if (qn)	{	/* suppress virtual */
					switch (qn->tp->base) {
					case FCT:
					case OVERLOAD:
						*n = *qn;
						n->n_qualifier = q;
						nn = n;
						n = 0;
					}
				}

				if (cc->c_this == 0) {
					switch (nn->n_oper) {
					case CTOR:
					case DTOR:
						break;
					default:  // in static member initializer
						error("%n cannot be used here",nn);
						return nn;
					}
				}

				if (n) delete n;
				{	Pref r = new ref(REF,cc->c_this,nn);
					cc->c_this->use();
					r->tp = nn->tp;
					return r;
				}
			}
		default:
			if (n) delete n;
			return nn;
		}
	}
qq:
//error('d',"qq: n%n qn%d",n,qn);
	if (qn) {
		// check for p->base::mem :
			// nasty where derived::mem is public
			// and base::mem is private
		// NOT DONE

	/* static member? */
		if (qn->n_scope==0  && !cl->has_friend(cc->nof) ) {
			error("%n is private",qn);
			if (n) delete n;
			return qn;
		}

		switch (qn->n_stclass) {
		case STATIC:
			break;
		default:
			switch (qn->tp->base) {
			case FCT:
			case OVERLOAD:	/* suppress virtual */
				if (f == 1) error("O missing for%n",qn);
				if (f == 3) return qn;
				*n = *qn;
				n->n_qualifier = q;
				return n;
			default:
				if (f < 2) error("O missing for%n",qn);
			}
		}

		if (n) delete n;
		return qn;
	}

	if ( nn = lookc(n->string,0) ) {
		switch (nn->n_scope) {
		case 0:
		case PUBLIC:
			if (nn->n_stclass == ENUM) break;

			if (nn->tp->base == OVERLOAD) break;

			if (Ebase && !Ebase->has_friend(cc->nof) )
				error("%n is from privateBC",n);

			if (Epriv
			&& !Epriv->has_friend(cc->nof)
			&& !(nn->n_protect && Epriv->baseof(cc->nof)) )
				error("%n is %s",n,nn->n_protect?"protected":"private");
		}
	}

	if (nn) {
//error('d',"found %n",nn);
		if (f==2 && nn->n_table==gtbl) error("M%n not found",n);
		nn->use();
		if (n) delete n;
		return nn;
	}

def:	/* implicit declaration */
//error('d',"implicit f %d",f);
	n->n_qualifier = 0;
	if (f == 1) {	/* function */
		if (n->tp) error('i',"find_name(fct_type?)");
		n->tp = new fct(defa_type,0,0);
		n->n_sto = EXTERN;
	/*	if (fct_void) {
			n->tp = new fct(defa_type,0,0);
		}
		else {
			Pexpr e;
			Pname at = 0;
			Pname att;
			
			for (e=args; e; e=e->e2) {
				Pname ar = new name;
				if (e->base != ELIST) error('i',"badA %k",e->base);
				e->e1 = e->e1->typ(this);
				ar->tp = e->e1->base==STRING ? Pchar_type : e->e1->tp;
				switch (ar->tp->base) {
				case ZTYPE:
					ar->tp = defa_type;
					break;
				case FIELD:
					ar->tp = int_type;
					break;
				case ANY:
				default:
					PERM(ar->tp);
				}
				if (at)
					att->n_list = ar;
				else
					at = ar;
				att = ar;
			}
			n->tp = new fct(defa_type,at,1);

		}
	*/
	}
	else {
		n->tp = any_type;
		if (this != any_tbl)
			if (cc->not
                        && ( !strcmp( n->string, cc->not->string) )
			&& (cc->cot->defined&(DEFINED|SIMPLIFIED) ) == 0)
				error("C%n isU",cc->not);
			else
				error("%n isU",n);
	}

	nn = n->dcl(gtbl,EXTERN);
	nn->n_list = 0;
	nn->use();
	nn->use();	/* twice to cope with "undef = 1;" */
	if (n) delete n;

	if (f==1)
		if (fct_void) {
			if  (no_of_undcl++ == 0) undcl = nn;
		}
		else
			error('w',"undeclaredF%n called",nn);

	return nn;
}

