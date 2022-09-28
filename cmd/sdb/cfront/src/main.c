/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/main.c	1.1"
/*ident	"@(#)cfront:src/main.c	1.26" */
/***********************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.


main.c:

	Initialize global environment
	Read argument line
	Start compilation
	Clean up end exit

**************************************************************************/

#include <time.h>
#include <ctype.h>

char* ctime(long*);
long time(long*);
long start_time, stop_time;

#include "cfront.h"

char* prog_name = "<<cfront 1.2.4 8/23/87>>";

char* src_file_name;

//bit Styp = 1;
//bit Ssimpl = 1;

bit old_fct_accepted;
bit fct_void;

char* line_format = "\n# %d \"%s\"\n";

#ifdef unix
#include <signal.h>

void core_dump()
{
	if (error_count) {
		fprintf(stderr,"sorry, cannot recover from previous error\n");
	}
	else
		error('i',"bus error (or something nasty like that)");
	ext(99);
}
#endif

Plist isf_list;
Pstmt st_ilist;
Pstmt st_dlist;
Ptable sti_tbl;
Ptable std_tbl;

int Nspy;
int Nfile = 1 , Nline, Ntoken;
int Nfree_store, Nalloc, Nfree;
int Nname;
int Nn, Nbt, Nt, Ne, Ns, Nc, Nstr, Nl;
extern int NFn, NFtn, NFbt, NFpv, NFf, NFe, NFs, NFc, NFl;
int vtbl_opt = -1;	// how to deal with vtbls:
			// -1 static and defined
			// 0 external and supposed to be defined elsewhere
			// 1 external and defined

 simpl_init();
 typ_init();
 syn_init();
 lex_init();
 error_init();
 print_free();
 read_align(char*);
 print_align(char*);

void spy(char* s)
{
	if (s) fprintf(stderr,"%s:\n",s);
	fprintf(stderr,"files=%d lines=%d tokens=%d\n",Nfile, Nline, Ntoken);
	fprintf(stderr,"Names: distinct=%d global=%d type=%d\n",
		Nname, gtbl->max(), ktbl->max());
	fflush(stderr);
	if (start_time && stop_time) {
		fprintf(stderr,"start time: %s", ctime(&start_time) );
		fprintf(stderr,"stop time:  %s", ctime(&stop_time) );
		fprintf(stderr,"real time delay %ld: %d lines per second\n",
			stop_time-start_time, Nline/(stop_time-start_time) );
		fflush(stderr);
	}
	print_free();
	fflush(stderr);
	fprintf(stderr,"sizeof: n=%d bt=%d f=%d pv=%d s=%d e=%d c=%d l=%d\n",
		sizeof(name), sizeof(basetype), sizeof(fct), sizeof(ptr),
		sizeof(stmt), sizeof(expr), sizeof(classdef), sizeof(elist) );
	fprintf(stderr,"alloc(): n=%d bt=%d t=%d e=%d s=%d c=%d l=%d str=%d\n",
		Nn, Nbt, Nt, Ne, Ns, Nc, Nl, Nstr);
	fprintf(stderr,"free(): n=%d bt=%d t=%d e=%d s=%d c=%d l=%d\n",
		NFn, NFbt, NFpv+NFf, NFe, NFs, NFc, NFl);
	fflush(stderr);
	fprintf(stderr,"%d errors\n",error_count);
	fflush(stderr);
}

Pname dcl_list;		// declarations generated while declaring something else

char *st_name(char*);	// generates names of static ctor, dtor callers

void run()
/*
	run the appropriate stages
*/
{
	Pname n;
	int i = 1;

	while (n=syn()) {
		Pname nn;
 		Pname nx;

		if (n == (Pname)1) continue;
	//	if (Styp == 0) {
	//		n->dcl_print(SM);
	//		lex_clear();
	//		continue;
	//	}

	/*	Now process list of declarations.
		Make sure the type stays around for all elements ofthe list.
		example:
			double cos(double x);
			double cos(double x), sin(double x);
	*/

		if (n->n_list) PERM(n->tp);

		for (nn=n; nn; nn=nx) {
			nx = nn->n_list;
			nn->n_list = 0;

			if (nn->dcl(gtbl,EXTERN) == 0) continue;

			if (error_count) continue;

		/*	if (Ssimpl) */ nn->simpl();

			/* handle generated declarations */
			for (Pname dx, d=dcl_list; d; d=dx) {
				dx = d->n_list;
				d->dcl_print(0);
				delete d;
			}
			dcl_list = 0;

			if (nn->base) nn->dcl_print(0);

			switch (nn->tp->base) {	/* clean up */
			default:
			{	Pexpr i = nn->n_initializer;
				if (i && i!=(Pexpr)1) DEL(i);							break;
			}

			case FCT:
			{	Pfct f = (Pfct)nn->tp;
				if (f->body && (debug || f->f_inline==0)) {
					DEL(f->body);
				/*	f->body = 0;  leave to detect re-definition, but do not use it */
				}
				break;
			}

			case CLASS:
			{	Pclass cl = (Pclass)nn->tp;
				register Pname p;
				Pname px;
				for (p=cl->mem_list; p; p=px) {
					px = p->n_list;
					if (p->tp)
						switch (p->tp->base) {
						case FCT:
						{	Pfct f = (Pfct)p->tp;
							if (f->body && (debug || f->f_inline==0)) {
								DEL(f->body);
								f->body = 0;
							}
						}
						case CLASS:
						case ENUM:
							break;
						case COBJ:
						case EOBJ:
							DEL(p);
							break;
						default:
							delete p;
						}
					else
						delete p;
				}
				cl->mem_list = 0;
				cl->permanent = 3;
				break;
			}
			}
			DEL(nn);
		}
		lex_clear();
	}

	switch (no_of_undcl) {
	case 0:
		break;
	case 1:
		error('w',"undeclaredF%n called",undcl);
		break;
	default:
		error('w',"%d undeclaredFs called; for example%n",no_of_undcl,undcl);
	}

	switch (no_of_badcall) {
	case 0:
		break;
	case 1:
		error('w',"%n declaredWoutAs calledWAs",badcall);
		break;
	default:
		error('w',"%d Fs declaredWoutAs calledWAs; for example%n",no_of_badcall,badcall);
	}

	Pname m;
	if (fct_void == 0)
	for (m=gtbl->get_mem(i=1); m; m=gtbl->get_mem(++i)) {
		if (m->base==TNAME
		|| m->n_sto==EXTERN
		|| m->n_stclass == ENUM) continue;

		Ptype t = m->tp;
		if (t == 0) continue;
	ll:
		switch (t->base) {
		case TYPE:	t=Pbase(t)->b_name->tp; goto ll;
		case CLASS:
		case ENUM:
		case COBJ:
		case OVERLOAD:
		case VEC:	continue;
		case FCT:
				if (Pfct(t)->f_inline || Pfct(t)->body==0) continue;
		}

		if (m->n_addr_taken==0
		&& m->n_used==0
		&& m->tp->tconst()==0)
			if (m->n_sto == STATIC)
				error('w',&m->where,"%n defined but not used",m);
		//	else
		//		error('w',&m->where,"%n defined but not used",m);
	}

        char * ctor_name = 0, *dtor_name = 0;

	if (st_ilist) {	/*	make an "init" function;
				it calls all constructors for static objects
			*/
		Pname n = new name( st_name("_STI") );
		Pfct f = new fct(void_type,0,1);
		n->tp = f;
		f->body = new block(st_ilist->where,0,0);
		f->body->s = st_ilist;
		f->body->memtbl = sti_tbl;
		n->n_sto = EXTERN;
		(void) n->dcl(gtbl,EXTERN);
		n->simpl();
		n->dcl_print(0);
		ctor_name = n->string;
	}

	if (st_dlist) {	/*	make a "done" function;
				it calls all destructors for static objects
			*/
		Pname n = new name( st_name("_STD") );
		Pfct f = new fct(void_type,0,1);
		n->tp = f;
		f->body = new block(st_dlist->where,0,0);
		f->body->s = st_dlist;
		f->body->memtbl = std_tbl;
		n->n_sto = EXTERN;
		(void) n->dcl(gtbl,EXTERN);
		n->simpl();
		n->dcl_print(0);
		dtor_name = n->string;
	}


#ifdef PATCH
		/*For fast load: make a static "__link" */
	if (ctor_name || dtor_name)
	{
		printf("static struct __link { struct __link * next;\n");
		printf("char (*ctor)(); char (*dtor)(); } __LINK = \n");
		printf("{ (struct __link *)0, %s, %s };\n",
			ctor_name ? ctor_name : "0",
			dtor_name ? dtor_name : "0");
	}
#endif

	if (debug==0) {			// print inline function definitions
		for (Plist l=isf_list; l; l=l->l) {
			Pname n = l->f;
			Pfct f = Pfct(n->tp);

			if (f->base == OVERLOAD) {
				n = Pgen(f)->fct_list->f;	// first fct
				f = Pfct(n->tp);
			}

			if (n->n_addr_taken || (f->f_virtual&&vtbl_opt!=0)) {
n->where.putline();
				n->tp->dcl_print(n);
}
		}
	}

	fprintf(out_file,"\n/* the end */\n");

}

bit warn = 1;	/* printout warning messages */
bit debug;	/* code generation for debugger */
char* afile = "";

int no_of_undcl, no_of_badcall;
Pname undcl, badcall;

main(int argc, char* argv[])
/*
	read options, initialize, and run
*/
{
	extern char* mktemp();
	register char * cp;
	short i;
#ifdef unix
	typedef void (*ST)(...);	// trick to circumvent problems with old
	ST sick = ST(&signal);		// (or C) versions <signal.h>
	(*sick)(SIGILL,core_dump);
	(*sick)(SIGIOT,core_dump);
	(*sick)(SIGEMT,core_dump);
	(*sick)(SIGFPE,core_dump);
	(*sick)(SIGBUS,core_dump);
	(*sick)(SIGSEGV,core_dump);
#endif

#ifdef apollo
	extern void set_sbrk_size(int);
	set_sbrk_size(1000000);		// resets free store size
#else
	char* malloc(unsigned);
	(void) malloc(0);	// suppress cashing in V8 malloc
#endif
	error_init();

	for (i=1; i<argc; ++i) {
		switch (*(cp=argv[i])) {
		case '+':
			while (*++cp) {
				switch(*cp) {
				case 'w':
					warn = 0;
error('w',"+w option will not be supported in future releases" );
					break;
				case 'd':
					debug = 1;
error('w',"+d option will not be supported in future releases" );
					break;
				case 'f':
					src_file_name = cp+1;
					goto xx;
				case 'x':	// read cross compilation table
					if (read_align(afile = cp+1)) {
						fprintf(stderr,"bad size-table (option +x)");
						exit(999);
					}
					goto xx;
				case 'V':	// C compatability
					fct_void = old_fct_accepted = 1;
fprintf(stderr,"\nwarning: +V option will not be supported in future releases\n" );
					break;
				case 'e':
					switch (*++cp) {
					case '0':
					case '1': 
						vtbl_opt = *cp-'0';
						break;
					default:
						fprintf(stderr,"bad +e option");
						exit(999);
					}
					break;	
				case 'S':
					Nspy++;
					break;
				case 'L':
					line_format = "\n#line %d \"%s\"\n";
					break;
				default:
					fprintf(stderr,"%s: unexpected option: +%c ignored\n",prog_name,*cp);

				}
			}
		xx:
			break;
		default:
			fprintf(stderr,"%s: bad argument \"%s\"\n",prog_name,cp);
			exit(999);
		}
	}




	fprintf(out_file,line_format+1,1,src_file_name?src_file_name:""); // strips leading \n
	fprintf(out_file,"\n/* %s */\n",prog_name);
	if (src_file_name) fprintf(out_file,"/* < %s */\n",src_file_name);

	if (Nspy) {
		start_time = time(0);
		print_align(afile);
	}
	fflush(stderr);
//	if (Ssimpl) print_mode = SIMPL;
	otbl_init();
	lex_init();
	syn_init();
	typ_init();
	simpl_init();
	scan_started = 1;
	curloc.putline();
	run();
	if (Nspy) {
		stop_time = time(0);
		spy(src_file_name);
	}

	exit( (0<=error_count && error_count<127) ? error_count : 127);
}


char* st_name(char* pref)
/*
	make name "pref|source_file_name|_" or "pref|source_file_name|_"
	where non alphanumeric characters are replaced with '_'
*/
{
	int prefl = strlen(pref);
	int strl = prefl + 2;
	if (src_file_name) strl += strlen(src_file_name);
	char* name = new char[strl];
	strcpy(name,pref);
	if (src_file_name) strcpy(name+prefl,src_file_name);
	name[strl-2] = '_';
	name[strl-1] = 0;
	char *p = name;
	while ( *++p ) if (!isalpha(*p) && !isdigit(*p)) *p = '_';
	return name;
}

