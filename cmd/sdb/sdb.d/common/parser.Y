%{
#ident	"@(#)sdb:sdb.d/common/parser.Y	1.34"
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "utility.h"
#include "SDBinfo.h"
#include "Expr.h"
#include "str.h"
#include "Interface.h"
#include "Process.h"
#include "Type.h"
#include <sgs.h>
#include <stdlib.h>
#include <errno.h>

#ifdef i386
// hw debug register support
#include "DbregAccess.h"

DbregAccess *proc_dbreg=NULL;         // debug register set for this process
                                      // global because it is needed in Process.ev.C
static Expr *watch_expr=NULL;         // expression to watch ( hw monitor)
#endif

#define MAXTOK	5120	/* longest acceptable input line, see exec(2) */
int yylex();
static int termflag=0;
static FILE *inputfile=stdin;
extern char *symfil;
static char r_args[MAXTOK];
static int r_cnt;
int toknum = 0;			// number of tokens so far in this command

int debugflag=0, vmode;
#define WINDOW 10
static char oldargs[MAXTOK];
static int oldargs_bad = 0;
static char *label;		// label for expr, built by "variable", etc.
static char *index_label;	// label for indexes, built by "indexes"
static char *index_part;	// label for indexes, built by "index"
static char *loc_label;		// label for locations, built by "linenum"

static SDBinfo *dot = 0;
static Iaddr last_addr = 0;

static SDBinfo **expr_tree = 0;

static SDBinfo *t;		// used by following five functions
static void print_expr(char *label, char *fmt);
static void print_addr(char *label, char *fmt);
static void eval_fcall(char *fmt);
static void eval_assign();
static void delete_tree();
static RegRef validate_reg(char *);

extern int dore(char *,int);
extern void fforward(long);
extern void fback(long);
extern void fprintn(int);
extern void fprint();
extern void ffind(long);
static int check_srcfile(Process *);
static char bsconv(char*&);
static void scroll_data(int count);

extern int yydebug;
#define YYDEBUG	1
%}
%union {
	char *sval;
	struct {
		int isimpl;
		long int value;
	} oival;
	Location loc;
	SDBinfo *tree;
};
%token	<sval>
		NUM
		NNUM
		FNUM
		FSEARCH
		BSEARCH
		e_CMD
		r_CMD
		b_CMD
		dm_CMD
		cm_CMD
		FINP_CMD
		SHELL_CMD
		SLASHFORM
		EQUALFORM
		QMARKFORM
		PROCPREF
		VNAME
		PNAME
		REGNAME
		PLINENUM
		FLINENUM
		ARROW
		STRING
		SCHAR
		COMMENT
%type	<sval>
		oprocpref
		cvariable
		fieldname
		slashform
		qmarkform
%type	<oival>
		cpart
		ocount
%type	<loc>
		linenum
		olinenum
%type	<tree>
		variable
		value
		rvalue
		indexes
		index
		fcall
		oargs
		args
		arg
		num
%%
command_line
	: COMMENT '\n'
	| commands '\n'
	| commands COMMENT '\n'
	| '>' '>' '>' '>' watch_expr '\n'
		{
			if ( expr_tree == 0 ) {
				printf("error: null pointer for watch expression\n");
				delete t;
			} else {
				*expr_tree = t;
				t = 0;
			}
		}
	| error '\n'
	;
commands
	: command					{ delete_tree(); }
	| commands ';' { inform_processes(); } command	{ delete_tree(); }
	;
command
	: 'q'
		{
			termflag=1;
		}
	| 't'
		{
			print_stack(current_process, 0, -1);
		}
	| 'T'
		{
			print_stack(current_process, 1, -1);
		}
	| 'x'
		{
			Location l;

			if ( printregs( current_process, 3 ) )
			{
				l.kind = lk_pc;
				disassemble(current_process, &l, 1, &instaddr);
				lastcom = DSICOM;
			}
		}
	| 'X'
		{
			Location l;

			l.kind = lk_pc;
			if ( disassemble( current_process, &l, 1, &instaddr ) )
			{
				lastcom = DSICOM;
			}
			else
			{
				printf("no process\n");
				lastcom = NOCOM;
			}
		}
	| 'k'
		{
			destroy_process(current_process);
			lastcom = NOCOM;
		}
	| 'B'
		{
			display_breaks(current_process);
		}
	| 'D'
		{
			remove_all_breaks(current_process,0);
			printf("All breakpoints deleted\n");
		}
	| 'l'
		{
			if ( pc_current_src(current_process) == 0 )
			{
				Location l;

				l.kind = lk_pc;
				disassemble( current_process, &l, 1, &instaddr );
				lastcom = DSICOM;
			}
			else if ( check_srcfile(current_process) )
			{
				fprint();
				lastcom = PRCOM;
			}
			else
			{
				Location l;

				printf("no current source file\n");
				l.kind = lk_pc;
				disassemble( current_process, &l, 1, &instaddr );
				lastcom = DSICOM;
			}
		}
	| 'p'
		{
			if(check_srcfile(current_process)) {
				fprint();
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| NUM 'p'
		{
			if(check_srcfile(current_process)) {
				ffind(strtoul($1,(char **)0,0));
				fprint();
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| 'w'
		{
			long ln;

			if(check_srcfile(current_process)) {
				current_src(current_process,&ln);
				fback( WINDOW / 2 );
				fprintn( WINDOW );
				ffind(ln);
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| NUM 'w'
		{
			long ln;

			if(check_srcfile(current_process)) {
				ffind(strtoul($1,(char **)0,0));
				current_src(current_process,&ln);
				fback( WINDOW / 2 );
				fprintn( WINDOW );
				ffind(ln);
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| 'z'
		{
			if(check_srcfile(current_process)) {
				fprintn( WINDOW );
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| NUM 'z'
		{
			if(check_srcfile(current_process)) {
				ffind(strtoul($1,(char **)0,0));
				fprintn( WINDOW );
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| ocount '+'
		{
			if(check_srcfile(current_process)) {
				fforward($1 . isimpl ? 1 : $1 . value);
				fprint();
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| ocount '-'
		{
			if(check_srcfile(current_process)) {
				fback($1 . isimpl ? 1 : $1 . value);
				fprint();
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| ocount '+' 'p'
		{
			if(check_srcfile(current_process)) {
				fforward($1 . isimpl ? 1 : $1 . value);
				fprint();
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| ocount '-' 'p'
		{
			if(check_srcfile(current_process)) {
				fback($1 . isimpl ? 1 : $1 . value);
				fprint();
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| ocount '+' 'z'
		{
			if(check_srcfile(current_process)) {
				fforward($1 . isimpl ? 1 : $1 . value);
				fprintn(WINDOW);
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| ocount '-' 'z'
		{
			if(check_srcfile(current_process)) {
				fback($1 . isimpl ? 1 : $1 . value);
				fprintn(WINDOW);
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| ocount '+' 'w'
		{
			long int ln;

			if(check_srcfile(current_process)) {
				fforward($1 . isimpl ? 1 : $1 . value);
				current_src(current_process,&ln);
				fback(WINDOW/2);
				fprintn(WINDOW);
				ffind(ln);
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| ocount '-' 'w'
		{
			long int ln;

			if(check_srcfile(current_process)) {
				fback($1 . isimpl ? 1 : $1 . value);
				current_src(current_process,&ln);
				fback(WINDOW/2);
				fprintn(WINDOW);
				ffind(ln);
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| NUM
		{
			if(check_srcfile(current_process)) {
				ffind(strtoul($1,(char **)0,0));
				fprint();
				lastcom = PRCOM;
			}
			else {
				lastcom = NOCOM;
			}
		}
	| 'R'
		{
			char commnd[MAXTOK];
			extern sigset_t e_sigset;

			r_cnt = 1;
			r_args[0] = '\0';
			oldargs[0] = '\0';
			oldargs_bad = 0;
		R_command:
			sprintf(commnd,"%s %s",symfil,r_args);
#ifdef STUBS
			printe("RUN: '%s'\n", commnd);
#else
			if ( current_process ) {
				destroy_process(current_process);
			}
			if (create_process(commnd,e_sigset) == 0) {
				printf("could not create process\n");
			}
			else {
				inform_processes();
				set_signal_set( current_process, e_sigset );
				run(current_process, 1, 0, r_cnt);
			}
			lastcom = NOCOM;
#endif
		}
	| NUM 'R'
		{
			r_cnt = strtoul($1, (char **)0, 0);
			r_args[0] = '\0';
			oldargs[0] = '\0';
			oldargs_bad = 0;

			goto R_command;
		}
	| 'r'
		{
			extern char *symfil;
			r_cnt = 1;
			if ( oldargs_bad ) {
				printf("old arguments truncated; ");
				printf("please give explicit arguments\n");
			} else {
				strcpy(r_args,oldargs);
				printf("%s %s\n", symfil, r_args);
				goto R_command;
			}
		}
	| NUM 'r'
		{
			extern char *symfil;
			r_cnt = strtoul($1, (char **)0, 0);
			if ( oldargs_bad ) {
				printf("old arguments truncated; ");
				printf("please give explicit arguments\n");
			} else {
				strcpy(r_args,oldargs);
				printf("%s %s\n", symfil, r_args);
				goto R_command;
			}
		}
	| 'r' r_CMD
		{
			char *args;

			r_cnt = 1;

			for(args = $<sval>2;*args==' '||*args=='\t';args++)
				;
			if (*args=='\0') {
				if ( oldargs_bad ) {
					printf("old arguments truncated; ");
					printf("please give explicit arguments\n");
					break;
				} else {
					strcpy(r_args,oldargs);
				}
			} else {
				strcpy(r_args, args);
				strcpy(oldargs,r_args);
			}
			oldargs_bad = 0;
			goto R_command;
		}
	| NUM 'r' r_CMD
		{
			char *args;

			r_cnt = strtoul($1, (char **)0, 0);

			for(args = $<sval>3;*args==' '||*args=='\t';args++)
				;
			if (*args=='\0') {
				if ( oldargs_bad ) {
					printf("old arguments truncated; ");
					printf("please give explicit arguments\n");
					break;
				} else {
					strcpy(r_args,oldargs);
				}
			} else {
				strcpy(r_args, args);
				strcpy(oldargs,r_args);
			}
			oldargs_bad = 0;
			goto R_command;
		}
	| 's' ocount
		{
			if($2 . isimpl)
				$2 . value=1;
			stmt_step(current_process, 1, 0, $2 . value, vmode);
			lastcom = PRCOM;
		}
	| 'S' ocount
		{
			if($2 . isimpl)
				$2 . value=1;
			stmt_step_over(current_process, 1, 0, $2 . value, vmode);
			lastcom = PRCOM;
		}
	| 'i' ocount
		{
			if($2 . isimpl)
				$2 . value=1;
			instr_step(current_process, 1, 0, $2 . value, vmode);
			lastcom = PRCOM;
		}
	| 'I' ocount
		{
			if($2 . isimpl)
				$2 . value=1;
			instr_step(current_process, 0, 0, $2 . value, vmode);
			lastcom = PRCOM;
		}
	| 'v'
		{
			if(vmode) {
				vmode = 0;
			}
			else {
				vmode = 1;
			}
		}
	| NUM 'v'
		{
			long val = strtoul($1,(char **)0,0);
			if (!val) {
				if (vmode)
					vmode = 0;
				else
					vmode = 1;
			} else
				vmode = val;
		}
	| olinenum 'c' ocount
		{
			Location *l= &$1;

			if($3 . isimpl)
				$3 . value=1;

			if(l->kind==lk_stmt&&l->file_name==0&&l->locn.line==0)
				run(current_process,1,(Location *)0,$3 . value);
			else
				run(current_process,1,l,$3 . value);
			lastcom = NOCOM;
		}
	| olinenum 'C' ocount
		{
			Location *l= &$1;

			if($3 . isimpl)
				$3 . value=1;

			if(l->kind==lk_stmt&&l->file_name==0&&l->locn.line==0)
				run(current_process,0,(Location *)0,$3 . value);
			else
				run(current_process,0,l,$3 . value);
			lastcom = NOCOM;
		}
	| linenum 'g' ocount
		{
			Location *l= &$1;

			if($3 . isimpl)
				$3 . value=1;

			if( change_pc(current_process,l) == 0 )
			{
				printf("could not change location.\n");
			}
			else if ( run(current_process,1,0,$3 . value) == 0 )
			{
				printf("could not run process from new location.\n");
			}
			lastcom = NOCOM;
		}
	| NUM ':' 'g' ocount
		{
			Location l;
			l.kind=lk_addr;
			l.locn.addr=strtoul($1,(char **)0,0);
			if($4 . isimpl)
				$4 . value=1;

			if( change_pc(current_process,&l) == 0 )
			{
				printf("could not change location.\n");
			}
			else if ( run(current_process,1,0,$4 . value) == 0 )
			{
				printf("could not run process from new location.\n");
			}
			lastcom = NOCOM;
		}
	| NUM 'g' ocount
		{
			Location l;
			l.kind=lk_stmt;
			l.locn.line=strtoul($1,(char **)0,0);
			l.file_name=current_src(current_process,0);
			if($3 . isimpl)
				$3 . value=1;

			if( change_pc(current_process,&l) == 0 )
			{
				printf("could not change location.\n");
			}
			else if ( run(current_process,1,0,$3 . value) == 0 )
			{
				printf("could not run process from new location.\n");
			}
			lastcom = NOCOM;
		}
	| olinenum 'b' b_CMD
		{
			Location *l= &$1;
			Assoccmds *assoc = new_assoc();

			strcat($3,"\n");
			if ( add_assoc(assoc,$3) == 0 )
				assoc = 0;
			if(l->kind==lk_stmt&&l->file_name==0&&l->locn.line==0) {
				l->file_name = current_src(current_process,&l->locn.line);
			}
			set_break( current_process, l, assoc, 1 );
		}
	| olinenum 'b'
		{
			Location *l= &$1;

			if(l->kind==lk_stmt&&l->file_name==0&&l->locn.line==0) {
				l->file_name = current_src(current_process,&l->locn.line);
			}
			set_break( current_process, l, 0, 1 );
		}
	| olinenum 'd'
		{
			Location *l= &$1;

			if(l->kind==lk_stmt&&l->file_name==0&&l->locn.line==0) {
				remove_all_breaks(current_process,1);
			}
			else {
				remove_break(current_process,l);
			}
		}
	| olinenum 'a'
		{
			Location *l= &$1;
			Assoccmds *assoc = new_assoc();
			char *ac = "l;c\n";

			if(l->kind==lk_stmt&&l->file_name==0&&l->locn.line==0) {
				l->file_name = current_src(current_process,&l->locn.line);
			}

			if(l->kind==lk_fcn && l->locn.line==0) {
				ac = "T;c\n";
			}

			add_assoc(assoc,ac);
			set_break( current_process, l, assoc, 0 );
		}
	| variable cpart '/' slashform
		{
			t = $1;
			if ( !$2 . isimpl )
				t->name.level = $2 . value;
			print_expr(label,$4);
		}
	| variable cpart
		{
			t = $1;
			if ( !$2 . isimpl )
				t->name.level = $2 . value;
			print_expr(label,0);
		}
	| '.'
		{
			if ( !dot ) {
				printf("no current expression\n");
			} else {
				t = dot;	
				print_expr(".",0);
			}
		}
	| '.' '/' slashform
		{
			if ( !dot ) {
				printf("no current expression\n");
			} else {
				t = dot;	
				print_expr(".",$3);
			}
		}
	| '.' EQUALFORM
		{
			if ( !dot ) {
				printf("no current expression\n");
			} else {
				t = dot;	
				print_addr(".",$2);
			}
		}
	| REGNAME '/' slashform
		{
			t = new SDBinfo( ikREGISTER );
			t->reg.regref = validate_reg($1);
			print_expr($1,$3);
		}
	| REGNAME
		{
			t = new SDBinfo( ikREGISTER );
			t->reg.regref = validate_reg($1);
			print_expr($1,0);
		}
	| NUM '/' slashform
		{
			t = new SDBinfo( ikINTEGER );
			t->integer = strtoul($1, (char**)0, 0);
			print_expr($1,$3);
		}
	| variable cpart EQUALFORM
		{
			t = $1;
			if ( !$2 . isimpl )
				t->name.level = $2 . value;
			print_addr(label,$3);
			lastcom = NOCOM;
		}
	| linenum EQUALFORM
		{
			Location *l= &$1;

			if(get_addr(current_process,l,instaddr)!=0) {
				t = new SDBinfo( ikINTEGER );
				t->integer = instaddr;
				print_addr(loc_label,$2);
			}
			else {
				printf("no such address\n");
			}
			lastcom = NOCOM;
		}
	| NUM EQUALFORM
		{
			t = new SDBinfo( ikINTEGER );
			t->integer = strtoul($1, (char**)0, 0);
			print_addr($1,$2);
			lastcom = NOCOM;
		}
	| linenum '?' qmarkform
		{
			Location *l= &$1;

			if(get_addr(current_process,l,instaddr)!=0) {
				t = new SDBinfo( ikINTEGER );
				t->integer = instaddr;
				print_expr(loc_label,$3);
			}
			else {
				printf("no such address\n");
				lastcom = NOCOM;
			}
		}
	| NUM '?' qmarkform
		{
			Location l;
			l.kind = lk_stmt;
			l.locn.line = strtoul( $1, (char**)0, 0 );
			l.file_name = current_src( current_process, 0 );

			if(get_addr(current_process,&l,instaddr)!=0) {
				t = new SDBinfo( ikINTEGER );
				t->integer = instaddr;
				print_expr($1,$3);
			}
			else {
				printf("no such address\n");
				lastcom = NOCOM;
			}
		}
	| NUM ':' '?' qmarkform
		{
			t = new SDBinfo( ikINTEGER );
			t->integer = strtoul( $1, (char **)0, 0 );
			print_expr($1,$4);
		}
	| value '!' rvalue
		{
			t = new SDBinfo( ikASSIGN );
			t->assign.lhs = $1;
			t->assign.rhs = $3;
			eval_assign();
			lastcom = NOCOM;
		}
	| variable cpart dm_CMD ocount
		{
			char *buf = new char[ strlen(label) + 20 ];
			int cnt = $4 . value;
			if ( $4 . isimpl )
				cnt = -1;

			if ( $2 . isimpl )
				sprintf(buf, "%s", label);
			else
				sprintf(buf, "%s,%u", $2 . value);

#ifdef i386
			if(cnt==-1){ /* try to use hw data breakpoint */
			  proc_dbreg= new DbregAccess (current_process);
			  watch_expr = new Expr (buf);
			  if(proc_dbreg->set_hw_watch(watch_expr)!=-1){
			    run(current_process,1,(Location *)0,0);
			  }
			  else { // hw watchpoints failed, use sw watchpoints
			    delete proc_dbreg;
			    delete watch_expr;
			    proc_dbreg=NULL;
			    watch_expr=NULL;
			    if(set_watch(current_process,buf)!=0)
			      stmt_step(current_process,1,0,cnt,vmode);
			  }
			}
			else 
#endif
			if(set_watch(current_process,buf)!=0)
				stmt_step(current_process,1,0,cnt,vmode);

			delete buf;
			lastcom = NOCOM;
		}
	| cm_CMD ocount
		{
			int cnt = $2 . value;
			char *p = strchr($<sval>1, ':');
			*p = 0;
			if ( $2 . isimpl )
				cnt = -1;

#ifdef i386
			if(cnt==-1){ /* try to use hw data breakpoint */
			  proc_dbreg= new DbregAccess (current_process);
			  watch_expr = new Expr ($1);
			  if(proc_dbreg->set_hw_watch(watch_expr,1)!=-1){
			    run(current_process,1,(Location *)0,0);
			  }
			  else {// hw watchpoints failed, use sw watchpoints
			    delete proc_dbreg;
			    delete watch_expr;
			    proc_dbreg=NULL;
			    watch_expr=NULL;
			    if(set_watch(current_process,$1)!=0)
			      stmt_step(current_process,1,0,cnt,vmode);
			  }
			}
			else 
#endif
			if(set_watch(current_process,$1)!=0)
				stmt_step(current_process,1,0,cnt,vmode);
			lastcom = NOCOM;
		}
	| fcall
		{
			t = $1;
			eval_fcall(0);
			lastcom = NOCOM;
		}
	| fcall '/'
		{
			t = $1;
			eval_fcall("");
			lastcom = NOCOM;
		}
	| fcall '/' SLASHFORM
		{
			t = $1;
			eval_fcall($3);
			lastcom = NOCOM;
		}
	| 'e'
		{
e_command:		char *	s;
			Symbol curfunc;
			char *  f;

			s = current_src(current_process, 0);
			if ( s == 0 )
				s = "(no source file)";
			curfunc = current_process->current_function();
			if ( curfunc.isnull() )
				f = "(no current function)";
			else
				f = current_process->symbol_name(curfunc);
			printf("%s %s\n", f, s);
			lastcom = NOCOM;
		}
	| 'e' e_CMD
		{
			char *args=$2;
			long nl;

			while(*args==' '||*args=='\t') args++;

			if ( !*args ) {
				goto e_command;
			}
			else if ( strchr( args, ' ' ) ) {
				printf("`e' command error: too many arguments\n");
			}
			else if ( args[strlen(args)-1]=='/' ) {
				suffix_path( args );
			}
			else if ( strchr( args, '.' ) ) {
				if ( set_current_src(current_process,args,1L) )
				{
 					printf("current file is now %s\n",
						current_src(current_process,0));
				}
				else
				{
				printf("%s could not be found\n",args);
				}
			}
			else {
				char *	filename;
	
				if ( (filename = find_function(current_process,args,&nl)) == 0 ) {
					printf("no source file for function %s\n",args);
				}
				else {
					set_current_src(current_process,filename,nl);
					printf("current file is now %s\n",filename);
				}
			}
			lastcom = NOCOM;
		}
	| '/' FSEARCH
		{
			if(check_srcfile(current_process) == 0)
				lastcom = NOCOM;
			else if ( dore($2,1) )
				lastcom = PRCOM;
			else
				lastcom = NOCOM;
		}
	| '/'
		{
			if(check_srcfile(current_process) == 0)
				lastcom = NOCOM;
			else if ( dore("",1) )
				lastcom = PRCOM;
			else
				lastcom = NOCOM;
		}
	| '?' BSEARCH
		{
			if(check_srcfile(current_process) == 0)
				lastcom = NOCOM;
			else if ( dore($2,0) )
				lastcom = PRCOM;
			else
				lastcom = NOCOM;
		}
	| '?'
		{
			if(check_srcfile(current_process) == 0)
				lastcom = NOCOM;
			else if ( dore("",0) )
				lastcom = PRCOM;
			else
				lastcom = NOCOM;
		}
	| '!' SHELL_CMD
		{
			SIG_PF oldresp;

			oldresp = signal(SIGQUIT,(SIG_PF)0);
			system($2);
			oldresp = signal(SIGQUIT,oldresp);
			lastcom = NOCOM;
		}
	| '<' FINP_CMD
		{
			FILE *savefile = inputfile;
			char *args = $<sval>2;
			for (; *args==' '||*args=='\t'; ++args);
			if ((inputfile = fopen(args, "r")) == NULL) {
				printf("Cannot open command file %s\n", args);
				inputfile = savefile;
			}
			else {
				if (savefile != stdin) fclose(savefile);
			}
			lastcom = NOCOM;
		}
	| STRING
		{
			char *args=$1;
			char *from, *to = args;
			for( from = args ; *from ; ) {
				if (*from == '\\') {
					++from;
					*to++ = bsconv(from);
				}
				else {
					*to++ = *from++;
				}
			}
			*to = '\0';
			printf("%s\n",args);
			lastcom = NOCOM;
		}
	| 'V'
		{
			printf("sdb: %s%s\n",SGU_PKG,SGU_REL);
		}
	| 'M'
		{
			print_map( current_process );
		}
/* DBG
	| 'Y'
		{
			debugflag = !debugflag;
		}
	| 'Y' NUM
		{	unsigned long num = strtoul($2, (char **)0, 0);
			yydebug = num & 1;
			debugflag = num & ~1;
		}
 DBG */
	| '\004'
		{
			switch (lastcom) {
			case PRCOM:
				if(check_srcfile(current_process)) {
					fforward(1);
					printf("\r");
					fprintn(WINDOW);
					lastcom = PRCOM;
				}
				else {
					lastcom = NOCOM;
				}
				break;
			case DSCOM:
				scroll_data( WINDOW );
				break;
			case DSICOM:
				{
					Location l;
					int i;

					printf("\r");
					l.kind = lk_addr;
					for (i=0; i<WINDOW; i++) {
						l.locn.addr = instaddr;
						disassemble( current_process,
							&l, 1, &instaddr);
					}
				}
				break;
			case NOCOM:
				printf("\rnothing to scroll\n");
				break;
			}
		}
	|
		{
			switch (lastcom) {
			case PRCOM:
				if(check_srcfile(current_process)) {
					fforward(1);
					fprint();
				}
				else {
					lastcom = NOCOM;
				}
				break;
			case DSCOM:
				scroll_data( 1 );
				break;
			case DSICOM:
				{
				Location	l;

				l.kind = lk_addr;
				l.locn.addr = instaddr;
				disassemble( current_process, &l, 1, &instaddr);
				}
				break;
			case NOCOM:
				break;
			}
		}
	| error
	;
cpart
	: ',' NUM
		{
			$$ . isimpl = 0;
			$$ . value = strtoul($2, (char **)0, 0);
		}
	|
		{
			$$ . isimpl = 1;
		}
	;
linenum
	: PLINENUM
		{
			char *c;

			$$.kind=lk_fcn;

			for(c=$1;*c!=':';c++);
			*c++='\0';
			$$.fcn_name=$1;

			$$.locn.line=strtoul(c,(char **)0,0);
			loc_label = str($1);
		}
	| FLINENUM
		{
			char *c;

			$$.kind=lk_stmt;

			for(c=$1;*c!=':';c++);
			*c++='\0';
			$$.file_name=$1;

			$$.locn.line=strtoul(c,(char **)0,0);
			loc_label = str($1);
		}
	| PROCPREF
		{
			char *c;

			$$.kind=lk_fcn;

			for(c=$1;*c!=':';c++);
			*c++='\0';
			$$.fcn_name=$1;
			$$.locn.line=0;
			loc_label = str($1);
		}
	;
olinenum
	: linenum
	| NUM ':'
		{
			$$.kind=lk_addr;
			$$.locn.addr=strtoul($1,(char **)0,0);
		}
	| NUM
		{
			$$.kind=lk_stmt;
			$$.locn.line=strtoul($1,(char **)0,0);
			$$.file_name=current_src(current_process,0);
		}
	|
		{
			$$.kind=lk_stmt;
			$$.file_name=(char *)0;
			$$.locn.line=0;
		}
	;
variable
	: oprocpref VNAME
		{
			if ( *$<sval>1 == ':' )		/* :var		*/
				t = SDBinfo_name( 1, 0, $2 );
			else if ( *$<sval>1 )		/* proc:var	*/
				t = SDBinfo_name( 0, $1, $2 );
			else				/* var		*/
				t = SDBinfo_name( 0, 0, $2 );
			$$ = t;
			label = sf("%s%s", $<sval>1, $2);
		}
	| oprocpref cvariable
		{
			if ( *$<sval>1 == ':' )		/* :var		*/
				t = SDBinfo_name( 1, 0, $2 );
			else if ( *$<sval>1 )		/* proc:var	*/
				t = SDBinfo_name( 0, $1, $2 );
			else				/* var		*/
				t = SDBinfo_name( 0, 0, $2 );
			$$ = t;
			label = sf("%s%s", $<sval>1, $2);
		}
	| NUM '.' fieldname
		{
			t = new SDBinfo( ikINTEGER );
			t->integer = strtoul($1, (char **)0, 0);
			t = $$ = SDBinfo_select( ikDOT, $3, t );
			label = sf("%s.%s", $1, $3);
		}
	| NUM ARROW fieldname
		{
			t = new SDBinfo( ikINTEGER );
			t->integer = strtoul($1, (char **)0, 0);
			t = $$ = SDBinfo_select( ikREF, $3, t );
			label = sf("%s->%s", $1, $3);
		}
	| variable '.' fieldname
		{
			t = $$ = SDBinfo_select( ikDOT, $3, $1 );
			label = sf("%s.%s", label, $3);
		}
	| variable ARROW fieldname
		{
			t = $$ = SDBinfo_select( ikREF, $3, $1 );
			label = sf("%s->%s", label, $3);
		}
	| variable '[' indexes ']'
		{
			t = $$ = SDBinfo_append_decomp( $1, $3 );
			label = sf("%s[%s]", label, index_label);
		}
	;
fieldname
	: VNAME		{ $$ = $<sval>1; }
	| cvariable	{ $$ = $<sval>1; }
	;
indexes
	: index
		{
			t = $$ = $1;
			index_label = index_part;
		}
	| indexes ',' index
		{
			t = $$ = SDBinfo_append_decomp( $1, $3 );
			index_label = sf("%s,%s", index_label, index_part);
		}
	;
index
	: num
		{
			long n = $1->integer;
			delete t;
			t = $$ = SDBinfo_index( 0, n, n );
			index_part = sf("%d",n);
		}
	| num ';' num
		{
			long n = $1->integer;
			long m = $3->integer;
			delete $1;
			delete $3;
			if ( m < n ) {
				printf("upper bound must be >= lower bound\n");
				t = 0;
				YYERROR;
			} else {
				t = SDBinfo_index( 0, n, m );
				index_part = sf("%d;%d", n, m);
			}
			$$ = t;
		}
	| '*'
		{
			t = $$ = SDBinfo_index( 1, 0, -1 );
			index_part = str("*");
		}
	;
value
	: NUM
		{
			long n = strtoul( $1, (char **)0, 0 );
			t = $$ = new SDBinfo( ikINTEGER );
			t->integer = n;
		}
	| REGNAME
		{
			t = $$ = new SDBinfo( ikREGISTER );
			t->reg.regref = validate_reg( $1 );
		}
	| variable cpart
		{
			t = $$ = $1;
			if ( !$2 . isimpl )
				t->name.level = $2 . value;
		}
	;
rvalue
	: num
		{
			t = $$ = $1;
		}
	| SCHAR
		{
			char *arg = $1 + 1;	// skip initial quote
			char *from = arg;
			if ( *from == '\\' ) {
				++from;
				*arg = bsconv(from);
			}
			t = $$ = new SDBinfo( ikCHAR );
			t->cchar = *arg;
		}
	| STRING
		{
			char *args=$1;
			char *from, *to = args;
			for( from = args ; *from ; ) {
				if (*from == '\\') {
					++from;
					*to++ = bsconv(from);
				}
				else {
					*to++ = *from++;
				}
			}
			*to = '\0';
			t = $$ = new SDBinfo( ikSTRING );
			t->string = str( args );
		}
	| FNUM
		{
			t = $$ = new SDBinfo( ikFLOAT );
			t->dbl = strtod( $1, (char **)0 );
		}
	| REGNAME
		{
			t = $$ = new SDBinfo( ikREGISTER );
			t->reg.regref = validate_reg( $1 );
		}
	| variable cpart
		{
			t = $$ = $1;
			if ( !$2 . isimpl )
				t->name.level = $2 . value;
		}
	;
fcall
	: PNAME '(' oargs ')'
		{
			t = new SDBinfo( ikCALL );
			t->call.fcn_name = str( $1 );
			t = $$ = SDBinfo_append_arg( t, $3 );
		}
	;
oargs
	: args
		{
			t = $$ = $1;
		}
	|
		{
			t = $$ = 0;
		}
	;
args
	: arg
		{
			t = $$ = $1;
		}
	| args ',' arg
		{
			t = $$ = SDBinfo_append_arg( $1, $3 );
		}
	;
arg
	: num
		{
			t = $$ = $1;
		}
	| FNUM
		{
			t = $$ = new SDBinfo( ikFLOAT );
			t->dbl = strtod( $1, (char **)0 );
		}
	| SCHAR
		{
			char *arg = $1 + 1;	// skip initial quote
			char *from = arg;
			if ( *from == '\\' ) {
				++from;
				*arg = bsconv(from);
			}
			t = $$ = new SDBinfo( ikCHAR );
			t->cchar = *arg;
		}
	| STRING
		{
			char *args=$1;
			char *from, *to = args;
			for( from = args ; *from ; ) {
				if (*from == '\\') {
					++from;
					*to++ = bsconv(from);
				}
				else {
					*to++ = *from++;
				}
			}
			*to = '\0';
			t = $$ = new SDBinfo( ikSTRING );
			t->string = str( args );
		}
	| REGNAME
		{
			t = $$ = new SDBinfo( ikREGISTER );
			t->reg.regref = validate_reg( $1 );
		}
	| variable
		{
			t = $$ = $1;
		}
	;
num
	: NUM
		{
			long n = strtoul( $1, (char **)0, 0 );
			t = $$ = new SDBinfo( ikINTEGER );
			t->integer = n;
		}
	| NNUM
		{
			long n = strtol( $1, (char **)0, 0 );
			t = $$ = new SDBinfo( ikINTEGER );
			t->integer = n;
		}
	;
cvariable				/* one-letter commands as variables */
	: 'B' { $$ = $<sval>1; }
	| 'C' { $$ = $<sval>1; }
	| 'D' { $$ = $<sval>1; }
	| 'I' { $$ = $<sval>1; }
	| 'M' { $$ = $<sval>1; }
	| 'R' { $$ = $<sval>1; }
	| 'S' { $$ = $<sval>1; }
	| 'T' { $$ = $<sval>1; }
	| 'V' { $$ = $<sval>1; }
	| 'X' { $$ = $<sval>1; }
	| 'Y' { $$ = $<sval>1; }
	| 'a' { $$ = $<sval>1; }
	| 'b' { $$ = $<sval>1; }
	| 'c' { $$ = $<sval>1; }
	| 'd' { $$ = $<sval>1; }
	| 'e' { $$ = $<sval>1; }
	| 'g' { $$ = $<sval>1; }
	| 'i' { $$ = $<sval>1; }
	| 'k' { $$ = $<sval>1; }
	| 'l' { $$ = $<sval>1; }
	| 'p' { $$ = $<sval>1; }
	| 'q' { $$ = $<sval>1; }
	| 'r' { $$ = $<sval>1; }
	| 's' { $$ = $<sval>1; }
	| 't' { $$ = $<sval>1; }
	| 'v' { $$ = $<sval>1; }
	| 'w' { $$ = $<sval>1; }
	| 'x' { $$ = $<sval>1; }
	| 'z' { $$ = $<sval>1; }
	| '?' { $$ = $<sval>1; }
	| '*' { $$ = $<sval>1; }
	;
oprocpref
	: PROCPREF
		{
			$$ = $<sval>1;
		}
	| ':'
		{
			$$ = str(":");
		}
	|
		{
			$$ = str("");
		}
	;
ocount
	: NUM
		{
			$$ . isimpl=0;
			$$ . value=strtoul($1,(char **)0,0);
		}
	|
		{
			$$ . isimpl=1;
		}
	;
watch_expr
	: variable cpart
		{
			t = $1;
			if ( !$2. isimpl )
				t->name.level = $2 . value;
		}
	| NUM
		{
			t = new SDBinfo( ikINTEGER );
			t->integer = strtoul($1, (char **)0, 0);
		}
	;
slashform
	: /* empty */
		{
			$$ = 0;
		}
	| SLASHFORM
		{
			$$ = $1;
		}
	| cvariable
		{
			$$ = $1;
		}
	;
qmarkform
	: /* empty */
		{
			$$ = "i";
		}
	| QMARKFORM
		{
			$$ = $1;
		}
	| cvariable
		{
			$$ = $1;
		}
	;
%%

void
parse_ps_args( register char *p )
{
	static char symfilbuf[ PRARGSZ ];

	symfil = symfilbuf;
	oldargs_bad = 0;

	if ( p && *p ) {
		strcpy( symfilbuf, p );
	} else {
		oldargs_bad = 1;	// couldn't get psargs
	}

	if ( strlen(symfilbuf) == PRARGSZ - 1 ) {
		oldargs_bad = 1;	// args were truncated
	}

	p = strchr( symfilbuf, ' ' );
	if ( p ) {
		*p = '\0';
		strcpy( oldargs, ++p );
	}
}

extern int isatty(int);

static int getline(register char *lbuf)
{
	register int c;
	register char *maxbuf = lbuf + MAXTOK - 2;
	register int cnt = 0;

	errno = 0;
	c = getc(inputfile);
	if(c == EOF) {
		if(inputfile == stdin) {
			if ( isatty(fileno(stdin)) ) {
				if ( errno == EINTR ) {
					/* interrupt reads as a comment */
					lbuf[0] = '#';
					lbuf[1] = '\n';
					lbuf[2] = '\0';
					return 2;
				}
				else {
					/* EOF == scroll command */
					lbuf[0] = '\004';
					lbuf[1] = '\n';
					lbuf[2] = '\0';
					return 2;
				}
			}
			else {
				return 0;
			}
		}
		else {
			fclose(inputfile);
			inputfile = stdin;
			return getline(lbuf);
		}
	}
	while(c!='\n' && c!=EOF && lbuf<maxbuf) {
		*lbuf++ = c;
		++cnt;
		c = getc(inputfile);
	}
	++cnt;
	*lbuf++ = '\n';
	*lbuf   = '\0';

	return cnt;
}

char *lineptr;
static char *lastline;

int parse(char *line, SDBinfo **infop)
{
	int yyparse();

	lastline = lineptr = line;
	expr_tree = infop;

	return yyparse();
}

#ifdef TWO_DOT_OH
extern "C" {
#endif
void docommands();
#ifdef TWO_DOT_OH
}
#endif

void docommands()
{
	if ( (current_process!= 0) && (current_process->is_proto() == 0) &&
		!grabbed(current_process) )
		parse("l\n", 0);
	while(termflag==0) {
		char line [MAXTOK];

		inform_processes();
#ifdef i386
		if(proc_dbreg!=NULL){  // if hw watchpoints active
// now done in check_watchpoints
//		  proc_dbreg->interpret_status(); // interpret watchpoint status
		  delete proc_dbreg;  // deactivate
		  delete watch_expr;  // deactivate
		  proc_dbreg=NULL;
		  watch_expr=NULL;
		}
#endif
		remove_all_watch( current_process, 0 );
		printf("*");
		if ( !getline(line) )
			break;
		if(inputfile!=stdin)
			printf("*%s",line);
		parse(line, 0);
	}
	if(current_process) {
		if (current_process->is_child()) {
			destroy_process(current_process);
		} else if (current_process->state != es_none &&
			   current_process->state != es_corefile) {
			printf("releasing and running grabbed process\n");
			release_process(current_process,1);
		}
	}
}

static void yyerror(const char *e)
{
	char *s=lastline;
	extern char yytext[];
	int i;

	printf("%s:*%s",e,s);
	printf("%*s",strlen(e)+2+(lineptr-lastline)-strlen(yytext)," ");
	for(i=strlen(yytext);i>0;i--)
		printf("^");
	printf("\n");
}

static int check_srcfile(Process *proc)
{
#ifndef STUBS
	long ln;
	char *fn;
	SrcFile *sf;

	if(proc==0)
		return 0;

	if(
		(fn=current_src(proc,&ln))==0
			||
		(sf=find_srcfile(fn))==0
	) {
		printf("no source file\n");
		return 0;
	}
	else if(num_lines(sf)==0) {
		printf("no lines in file\n");
		return 0;
	}
#endif /*STUBS*/

	return 1;
}

/*  convert char to backslash value */
static char 
bsconv(char *&p)
{
	char c = *p++;
	char *q;
	switch (c) {
	case 'a':	return '\a';
	case 'b':	return '\b';
	case 'f':	return '\f';
	case 'n':	return '\n';
	case 'r':	return '\r';
	case 't':	return '\t';
	case 'v':	return '\v';
	case 'x':
		// do hex escape: 'x' followed by arbno hex digits
		c = strtoul(p, &q, 16);
		if ( p == q )	// no hex digits, as in '\xy'
			return 'x';
		p = q;
		return c;
	case '0':	case '1':	case '2':	case '3':
	case '4':	case '5':	case '6':	case '7':
		// do octal escape: 1, 2, or 3 octal digits
		c -= '0';
		if ( *p >= '0' && *p <= '7' )
			c = (c<<3) | (*p++ - '0');
		if ( *p >= '0' && *p <= '7' )
			c = (c<<3) | (*p++ - '0');
		return c;
	}
	return c;	// handle ?, backslash, single and double quote here
}

static void
eval_fcall(char *fmt)
{
#ifndef STUBS
	lastcom = NOCOM;
	if ( debugflag ) {
		printf("eval_fcall( '%s' ) t=%#x\n", fmt, t);
		dump_SDBinfo(t);
	}
	if ( ! t )
		return;
	Expr e(t);
	if ( !e.do_call(fmt) )
		printf("Evaluation failed\n");
#endif /*STUBS*/
}

static void
eval_assign()
{
#ifndef STUBS
	if ( debugflag ) {
		printf("eval_assign( %#x )\n", t);
		dump_SDBinfo(t);
	}
	if ( ! t )
		return;
	if ( t->kind != ikASSIGN ) {
		printf("eval_assign() called with kind = %d\n", t->kind);
		return;
	} else {
		Expr lhs(t->assign.lhs);
		if ( !lhs.eval(EV_LHS) ) {
			printf("Evaluation of lhs failed\n");
			return;
		}
		Expr rhs(t->assign.rhs);
		if ( !rhs.eval(EV_RHS) ) {
			printf("Evaluation of rhs failed\n");
			return;
		}
		Rvalue rval;
		if ( !rhs.rvalue(rval) ) {
			printf("Can't get value for rhs\n");
			return;
		}
		if ( lhs.assign(rval) ) {
			delete dot;
			dot = t->assign.lhs;
			last_addr = lhs.next_addr();
		} else {
			printf("Assignment failed\n");
			delete dot;
			dot = 0;
			last_addr = 0;
		}
	}
	if ( dot ) {	// assignment succeeded; print result
		Expr exp(dot);
		exp.print(0);
		delete t->assign.rhs;	// lhs == new dot, don't delete
		delete t;
		t = 0;
		lastcom = DSCOM;
	} else {
		lastcom = NOCOM;
	}
#endif /*STUBS*/
}

static void
print_expr( char *label, char *fmt )
{
#ifndef STUBS
	if ( *fmt != 'i' && 
	     *fmt != 'I' && 
	     (strcmp(label,".") != 0) && 
	     t->kind != ikINTEGER &&
	     (!current_process || current_process->is_proto())) {
                printe("no process\n");
                return;
        }
	lastcom = NOCOM;	// reset in e.print()
	if ( debugflag ) {
		printf("print_expr( '%s', '%s' ) t=%#x\n", label, fmt, t);
		dump_SDBinfo(t);
	}
	if ( ! t )
		return;
	if ( debugflag ) {
		printf("t   -> ");
		dump_SDBinfo(t);
		printf("dot -> ");
		dump_SDBinfo(dot);
	}
	if ( t == dot ) {
		t = 0;
		Expr e(dot);
		e.label(label);
		if ( !e.print( fmt ) ) {
			delete dot;		// no dot now
			dot = 0;
			last_addr = 0;
		}
	} else {
		Expr e(t);
		e.label(label);
		if ( e.print( fmt ) ) {
			delete dot;		// assign t to dot
			dot = t;
			t = 0;
			last_addr = e.next_addr();
		} else {
			delete dot;		// no dot now
			dot = 0;
			last_addr = 0;
			printf("No match\n");
		}
	}
#endif /*STUBS*/
}

static void
print_addr( char *label, char *fmt )
{
#ifndef STUBS
	lastcom = NOCOM;	// reset in e.printaddr()
	if ( debugflag ) {
		printf("print_addr( '%s', '%s' ) t=%#x\n", label, fmt, t);
		dump_SDBinfo(t);
	}
	if ( ! t )
		return;
	if ( debugflag ) {
		printf("t   -> ");
		dump_SDBinfo(t);
		printf("dot -> ");
		dump_SDBinfo(dot);
	}
	if ( t == dot ) {
		t = 0;
		Expr e(dot);
		e.label(label);
		if ( !e.printaddr( fmt ) ) {
			delete dot;		// no dot now
			dot = 0;
			last_addr = 0;
		}
	} else {
		Expr e(t);
		e.label(label);
		if ( e.printaddr( fmt ) ) {
			delete dot;		// assign t to dot
			dot = t;
			t = 0;
			last_addr = 0;
		} else {
			delete dot;		// no dot now
			dot = 0;
			last_addr = 0;
			printf("No match\n");
		}
	}
	lastcom = NOCOM;	// no scrolling of addresses
#endif /*STUBS*/
}

static void
delete_tree()
{
	delete t;
	t = 0;
}

static void scroll_data(int count)
{
	if ( !last_addr ) {
		lastcom = NOCOM;
		printf("\rnothing to scroll\n");
		return;
	}

	printf("\r");

	char *buf = new char[ count * sizeof(int) ];
	int ret = current_process->read(last_addr, count * sizeof(int), buf);
	if ( ret != count * sizeof(int) ) {
		printf("\rread failed\n");
		lastcom = NOCOM;
	} else {
		TYPE type;
		type = ft_int;
		int i;
		for (i=0; i<count; i++) {
			Rvalue rval(buf+4*i, sizeof(int), type, 1);
			rval.print(sf("0x%08x",last_addr),"x","/");
			last_addr += sizeof(int);
		}
	}
	delete buf;
}

static RegRef
validate_reg( char *regname )
{
	RegRef ref = regref( regname );

	if ( ref == -1 )
		printf("no such register: %s\n", regname );

	return ref;
}

#ifdef STUBS
int
disassemble(Process *, Location *, int , unsigned long *)
{
}
#endif
