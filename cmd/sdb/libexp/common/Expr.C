#ident	"@(#)sdb:libexp/common/Expr.C	1.31"

#include "Expr.h"
#include "Expr_priv.h"
#include "Symtab.h"
#include "NameList.h"
#include "Locdesc.h"
#include "Machine.h"
#include <stdlib.h>
#include <string.h>

// Expr.C -- public interface for libexp

// To enable debugging printe's, globally substitute /**/ for //DBG in this
// file, and in Rvalue.C, format.C, SDBinfo.C, Call.C and Call2.C.  Also,
// remove the two comment lines containing DBG from sdb.d/common/parser.Y.

//DBG#define DBG

#define SYMNAME(sym)	process->symbol_name(sym)

static char *
attr_string(int attr)
{
//DBG	switch (attr>>=16) {
//DBG	case an_nomore:		return "an_nomore";
//DBG	case an_tag:		return "an_tag";
//DBG	case an_name:		return "an_name";
//DBG	case an_child:		return "an_child";
//DBG	case an_sibling:	return "an_sibling";
//DBG	case an_parent:		return "an_parent";
//DBG	case an_count:		return "an_count";
//DBG	case an_type:		return "an_type";
//DBG	case an_elemtype:	return "an_elemtype";
//DBG	case an_elemspan:	return "an_elemspan";
//DBG	case an_subscrtype:	return "an_subscrtype";
//DBG	case an_lobound:	return "an_lobound";
//DBG	case an_hibound:	return "an_hibound";
//DBG	case an_basetype:	return "an_basetype";
//DBG	case an_resulttype:	return "an_resulttype";
//DBG	case an_argtype:	return "an_argtype";
//DBG	case an_bytesize:	return "an_bytesize";
//DBG	case an_bitsize:	return "an_bitsize";
//DBG	case an_bitoffs:	return "an_bitoffs";
//DBG	case an_litvalue:	return "an_litvalue";
//DBG	case an_stringlen:	return "an_stringlen";
//DBG	case an_lineinfo:	return "an_lineinfo";
//DBG	case an_location:	return "an_location";
//DBG	case an_lopc:		return "an_lopc";
//DBG	case an_hipc:		return "an_hipc";
//DBG	case an_visibility:	return "an_visibility";
//DBG	case an_scansize:	return "an_scansize";
//DBG	}
	static char buf[40];
	sprintf(buf, "unknown attr(%d)", attr);
	return buf;
}

static char *
form_string(int attr)
{
//DBG	switch (attr&=0xffff) {
//DBG	case af_none:			return "af_none";
//DBG	case af_tag:			return "af_tag";
//DBG	case af_int:			return "af_int";
//DBG	case af_locdesc:		return "af_locdesc";
//DBG	case af_stringndx:		return "af_stringndx";
//DBG	case af_coffrecord:		return "af_coffrecord";
//DBG	case af_coffline:		return "af_coffline";
//DBG	case af_coffpc:			return "af_coffpc";
//DBG	case af_spidoffs:		return "af_spidoffs";
//DBG	case af_fundamental_type:	return "af_fundamental_type";
//DBG	case af_symndx:			return "af_symndx";
//DBG	case af_reg:			return "af_reg";
//DBG	case af_addr:			return "af_addr";
//DBG	case af_local:			return "af_local";
//DBG	case af_visibility:		return "af_visibility";
//DBG	case af_lineinfo:		return "af_lineinfo";
//DBG	case af_attrlist:		return "af_attrlist";
//DBG	case af_cofffile:		return "af_cofffile";
//DBG	case af_symbol:			return "af_symbol";
//DBG	case af_bdioffs:		return "af_bdioffs";
//DBG	case af_bdiline:		return "af_bdiline";
//DBG	case af_elfoffs:		return "af_elfoffs";
//DBG	}
	static char buf[40];
	sprintf(buf, "unknown form(%d)", attr);
	return buf;
}

#undef DEFTAG
#define DEFTAG(VAL, NAME) case VAL: return NAME;

static char *
tag_string(Tag tag)
{
//DBG	switch (tag) {
//DBG#include "Tag1.h"
//DBG	default:
//DBG	; // fall through.
//DBG	}
	static char buff[20];
	sprintf(buff, "unknown Tag(%d)", tag);
	return buff;
}

overload dump;

void dump( Symbol sym, char *label = 0 )
{
//DBG	int *p = (int *)&sym;
//DBG	if ( !(debugflag & DBG_SYMDUMP) )
//DBG		return;
//DBG	if ( label ) printe("<%s>:\n", label);
//DBG	printe("Symbol.name()    = '%s'\n", sym.name());
//DBG	printe("Symbol.attrlist  = %#x\n", *(p+1));
//DBG	printe("Symbol.evaluator = %#x\n", *(p+2));
//DBG	printe("Symbol.ss_base   = %#x\n", *(p+3));
//DBG	for ( p = (int *)*(p+1) ; p && *p ; p+=2 ) {
//DBG	    if ( *p == ((an_tag<<16)|(af_tag)) )
//DBG		printe("%s, %s, %s\n",
//DBG			attr_string(*p), form_string(*p), tag_string(*(p+1)));
//DBG	    else
//DBG		printe("%s, %s, %#x\n",
//DBG			attr_string(*p), form_string(*p), *(p+1));
//DBG	}
}

void dump( TYPE type, char *label = 0 )
{
//DBG	if ( !(debugflag & DBG_SYMDUMP) )
//DBG		return;
//DBG	if ( label ) printe("<%s>: ",label);
//DBG	Fund_type fund;
//DBG	Symbol sym;
//DBG	if ( type.isnull() ) {
//DBG		printe("<null>\n");
//DBG	} else if ( type.fund_type(fund) ) {
//DBG		printe("fund = %d\n", fund);
//DBG	} else if ( type.user_type(sym) ) {
//DBG		dump(sym,"user");
//DBG	} else {
//DBG		printe("???\n");
//DBG	}
}

void
SymbolStream::init( char *pat, char *proc, int lev, Process *p,
						Frame *f, int global_only )
{
//DBG	if ( debugflag & DBG_SYMSTREAM )
//DBG		printe("SymbolStream::init( '%s', '%s', %d, %#x, %#x, %d )\n",
//DBG			pat, proc, lev, p, f, global_only);
	int len;
	state = SSinit;
	pattern = pat;
	has_wildcards = (strpbrk( pat, "*?" ) != 0);
	if ( proc && (len = strlen(proc)) > 1 )
		procedure = sf("%.*s", len-1, proc);	// drop ":"
	else
		procedure = proc;
	level = lev;
	process = p;
	_frame = f;
	do_locals = !global_only && !p->is_proto();
	do_globals = global_only || !procedure;
	current_global = 0;
	current_local.null();
	global_count = 0;
	first_time = 1;
	found_local = 0;
}

static int
frame_to_scope( Process *proc, Frame *frame, Symbol &func, Symbol &block )
{
	Iaddr pc = frame->getreg(REG_PC);
	Symtab *symtab = proc->find_symtab(pc);
	if( !symtab )
		return 0;

	func = symtab->find_entry(pc);
	block = symtab->find_scope(pc);

	int ok = (!block.isnull() && !func.isnull());

	if ( ok ) {
		Tag tag = block.tag();
		if ( tag != t_entry && tag != t_block && 
			tag != t_extlabel && tag != t_label )
			ok = 0;
	}

	return ok;
}

static int
tag_ok( Tag tag )
{
	switch( tag ) {
	case t_block:
	case t_entry:
	case t_extlabel:
	case t_sourcefile:
	case t_label:
		return 1;	// may have local symbols
	}
	return 0;
}

// Find the named function, either as a static visible from the given pc,
// or as a global (assumed to be visible everywhere).
Symbol
function_symbol( char *name, Process *process, Iaddr pc )
{
	Symbol sym;
	Symbol scope;
	Symbol nullsym;

//DBG	if ( debugflag & DBG_RESOLVE )
//DBG		printe("function_symbol( '%s', %#x, %#x )\n", name,process,pc);

	Symtab *symtab = process->find_symtab(pc);
	if ( symtab )
		scope = symtab->find_scope(pc);

	Symbol this_file;
	Tag tag;
	char *s;

	for (  ; !scope.isnull() ; scope = scope.parent() ) {
//DBG		dump( scope, "function_symbol() scope" );
		if ( interrupted )
			return nullsym;
		for ( sym = scope.child() ; !sym.isnull() ;
						sym = sym.sibling() ) {
			tag = sym.tag();
			if ( tag != t_entry && tag != t_block )
				continue;
			if ( (s = SYMNAME(sym)) == 0 )
				continue;
			if ( !strcmp(s, name) ) {
//DBG				dump( sym, "function_sym() result" );
				return sym;
			}
		}
		this_file = scope;
	}

	for ( scope = process->first_file() ; !scope.isnull() ;
					scope = process->next_file() ) {
		if ( scope == this_file )
			continue;
//DBG		dump( scope, "function_symbol() file scope" );
		for ( sym = scope.child() ; !sym.isnull() ;
						sym = sym.sibling() ) {
			tag = sym.tag();
			if ( tag != t_entry && tag != t_block )
				continue;
			if ( (s = SYMNAME(sym)) == 0 )
				continue;
			if ( !strcmp(s, name) ) {
//DBG				dump( sym, "function_sym() result" );
				return sym;
			}
		}
	}

	sym = process->find_global(name);

//DBG	dump( sym, "function_sym():find_global() result" );
	return sym;
}

//
// Within the pattern '*' matches zero more characters
//                    '?' matches exactly one character
//                    all other characters must be matched exactly.
//
// History: This pattern match algorithm was lifted from old sdb's eqpatr()
//          and cleaned up.  In particular, much tail recursion is avoided.
//
// Note: old sdb did NOT consider "arr" as a match to pattern "*arr";
//    this algorithm considers it a match - "*" matches zero or more chars.
static int
match_pattern( register char *pat, register char *str )
{
    for (; *pat != '\0' && *str != '\0'; ++pat, ++str) {
	switch (*pat) {
	case '?': break; // this position matched.
	case '*':
	    if (pat[1] == '\0') return 1;
	    for (register char *sp = str; *sp != '\0'; ++sp) {
		if (match_pattern(pat+1, sp)) return 1;
	    }
	    return 0;  // no match.
	default:
	    if (*pat != *str) return 0;
	}
    }
    // All wild cards are processed and/or the string is exhausted,
    // so match iff BOTH pattern and string remainders are empty.

    return *str == '\0' && *pat == '\0';
}

static int
match_name( register char *name, register char *pat, int has_wildcards )
{
	if ( !name )
		return 0;

	if ( has_wildcards ) {
		return match_pattern( pat, name );
	} else {
		return !strcmp( pat, name );
	}
}

SymbolStream::SymbolStream( SymbolStream &s )
{
	pattern =	 s.pattern;
	has_wildcards =	 s.has_wildcards;
	procedure =	 s.procedure;
	level =		 s.level;
	cur_level =	 s.cur_level;
	process =	 s.process;
	_frame =	 s._frame;
	do_locals =	 s.do_locals;
	do_globals =	 s.do_globals;
	state =		 s.state;
	current_file =	 s.current_file;
	scope_file =	 s.scope_file;
	current_global = s.current_global;
	current_local =	 s.current_local;
	func =		 s.func;
	block =		 s.block;
	scope =		 s.scope;
	global_count =	 s.global_count;
	lab =		 s.lab;
}

int
SymbolStream::match( char *func )
{
//DBG	if ( debugflag & DBG_MATCH )
//DBG		printe("match('%s') procedure = '%s', level = %d\n",func,
//DBG					procedure, level);
	if ( procedure != 0 && !match_pattern(procedure, func) ) {
//DBG		if ( debugflag & DBG_MATCH )
//DBG			printe("procedure doesn't match\n");
		return 0;		// this func doesn't match
	}

	++cur_level;			// matched the "proc:", if specified

	if ( level == level_unspec ) {
//DBG		if( debugflag & DBG_MATCH )
//DBG			printe("procedure matches, level_unspec\n");
		return 1;		// all levels match if unspecified
	}

//DBG	if ( debugflag & DBG_MATCH ) {
//DBG		if ( level == cur_level )
//DBG			printe("procedure and cur_level (%d) match\n", cur_level);
//DBG		else
//DBG			printe("cur_level (%d) doesn't match %d\n", cur_level, level);
//DBG	}

	return level == cur_level;
}

// Returns next symbol which matches pattern, procedure, and level.
// Returns null symbol if no more.
Symbol
SymbolStream::next()
{
	Symbol sym;
	Tag tag;

//DBG	if ( debugflag & DBG_SYMSTREAM )
//DBG		printe("SymbolStream::next() pattern = '%s', state = %d, first_time = %d\n",
//DBG				pattern, state, first_time);

	lab.clear();

// "done" is the loop exit, either normal or abnormal (interrupt)
// within the SSdoing_locals case:
//    "do_frame" checks the validity of _frame
//    "next_scope" walks up the nested block hierarchy
//    "next_local" fetches the next local from the current scope

	while ( sym.isnull() && !interrupted ) switch ( state ) {
	default:
		printe("bad state %d in SymbolStream::next()\n", state);
		break;
	case SSinit:
		if ( do_locals ) {
			state = SSdoing_locals;
			continue;
		} else {
			state = SSdoing_files;
			continue;
		}
		break;
	case SSdoing_locals:
//DBG		if ( debugflag & DBG_SYMSTREAM )
//DBG			printe("doing locals, first_time = %d\n", first_time);
//DBG		if ( debugflag & DBG_SYMSTREAM )
//DBG			printe("%s has %swildcards\n", pattern,
//DBG				has_wildcards ? "" : "no ");
do_frame:	if ( !first_time && !_frame ) {
			state = SSdoing_files;
			continue;
		}

		if ( interrupted )
			goto done;

		if ( block.isnull() ) {
			Symbol curfunc;
			scope.null();
			int ok = frame_to_scope(process, _frame, func, block);
//DBG			if ( debugflag & DBG_SYMSTREAM )
//DBG				printe("frame_to_scope returns %d, func = %s\n",
//DBG					ok, func.isnull()?"<null>":func.name());
			if ( !procedure ) {
				curfunc = process->current_function();
				if ( !curfunc.isnull() &&
					 ::strcmp(curfunc.name(), func.name()) != 0 ) {
                                        _frame = _frame->caller();
                                        if (_frame) {
						block.null();
                                        	goto do_frame;
					}
					else
						goto done;
                                }
				if ( curfunc.isnull() || !SYMNAME(curfunc) )
					procedure = "";
				else
					procedure = str( SYMNAME(curfunc) );
//DBG				if ( debugflag & DBG_SYMSTREAM )
//DBG					printe("new procedure = '%s'\n",
//DBG						procedure);
			}
			if ( first_time ) {
				// check if current line is in same block as PC
				Symbol curscope;
				curscope = process->current_scope();
				if ( curscope != block ) {
//DBG					if ( debugflag & DBG_SYMSTREAM )
//DBG						printe("curscope != block\n");
					scope = curscope;
					func = curfunc;
					if ( func.isnull() && procedure[0] && !ok ) {
						_frame = _frame->caller();
						if (_frame) {
							first_time = 0;
							block.null();
							goto do_frame;
						}
						else
							goto done;
					}
					current_local.null();
					goto next_local;
				}
				first_time = 0;
				if ( !_frame ) {
//DBG					if ( debugflag & DBG_SYMSTREAM )
//DBG						printe("_frame == 0\n");
					state = SSdoing_files;
					continue;
				} else {
//DBG					if ( debugflag & DBG_SYMSTREAM )
//DBG						printe("_frame != 0\n");
				}
			}
			if ( !ok || !match(SYMNAME(func)) ) {
				_frame = _frame->caller();
				block.null();
				goto do_frame;
			}
		}

		// now we have a valid func and block

		if ( scope.isnull() ) {
			scope = block;
			current_local.null();
		}
		tag = scope.tag();
		while ( !tag_ok(tag) ) {
next_scope:		scope = scope.parent();
			if ( scope.isnull() ) {
				if ( first_time ) {
					first_time = 0;
				} else if (_frame) {
					_frame = _frame->caller();
				}
				if (_frame) {
					block.null();
					goto do_frame;
				}	
				else
					goto done;
			}
			scope_file = scope;
			tag = scope.tag();
			if ( interrupted )
				goto done;
		}

		// now we have a valid scope, get next matching local

next_local:	if ( current_local.isnull() ) {
			current_local = scope.child();
		} else {
			current_local = current_local.sibling();
		}
		if ( current_local.isnull() ) {
			if ( state == SSdoing_files )
				continue;
			else
				goto next_scope;
		}

		if ( interrupted )
			goto done;

		tag = current_local.tag();

		switch( tag ) {
		case t_argument:
		case t_variable:
		case t_entry:
		case t_label:
//		case t_enumlittype:
			break;			// tag is ok
		default:
			goto next_local;	// bad tag, skip it
		}

		if ( !current_file.isnull()) {
			Attribute *a=scope.attribute(an_tag);
			if (a->value.word == t_sourcefile)
				goto next_local;
		}
		if ( !match_name(SYMNAME(current_local),
					pattern, has_wildcards) ){
			goto next_local;
		}

		// got a match

		if ( state == SSdoing_locals ) {
			lab.push( SYMNAME(func) );
		} else {
			Attribute *a = current_local.attribute( an_visibility );
			if ( !a )	// globals don't have a visibility attr
				++global_count;
		}
		lab.push( ":" );
		lab.push( SYMNAME(current_local) );

		sym = current_local;
		found_local = 1;

		break;
	case SSdoing_files:
//DBG		if ( debugflag & DBG_SYMSTREAM ) printe("doing files\n");
//DBG		if ( debugflag & DBG_SYMSTREAM )
//DBG			printe("procedure = '%s'\n", procedure);
		if ( procedure && *procedure ) {
			state = SSnomore;
			continue;
		}
		if ( current_file.isnull() ) {	// first time for this case
			current_file = process->first_file();
//DBG			if ( debugflag & DBG_SYMSTREAM )
//DBG				printe("first_file() = '%s'\n",
//DBG						current_file.name());
			if ( current_file.isnull() ) {
				state = SSdoing_globals;
			} else {
				scope = current_file;
				goto next_local;
			}
		} else {	// current_file is not null, not first time
			if ( current_local.isnull() ) {
next_file:			current_file = process->next_file();
//DBG				if ( debugflag & DBG_SYMSTREAM )
//DBG					printe("next_file() = '%s'\n",
//DBG							current_file.name());
				if ( !scope_file.isnull() &&
						current_file == scope_file )
					goto next_file;	// skip it
			}
			if ( current_file.isnull() ) {
				state = SSdoing_globals;
			} else {
				scope = current_file;
				goto next_local;
			}
		}
		break;
	case SSdoing_globals:
		// Scan globals only if told to AND pattern has
		// wildcards (or it doesn't have wildcards and we
		// haven't matched a global yet).
		if ( !do_globals || (!has_wildcards && global_count > 0) ) {
			state = SSnomore;
			continue;
		}
		if ( !current_global ) {
			current_global = process->first_global();
//DBG			if ( debugflag & DBG_SYMSTREAM )
//DBG				printe("first global is %#x '%s'\n",
//DBG					current_global,
//DBG					current_global->name());
		} else {
			NameEntry *next = process->next_global();
//DBG			if ( debugflag & DBG_SYMSTREAM )
//DBG				printe("next global is %#x '%s'\n", next,
//DBG					next?next->name():"<null>");
			if ( next == 0 || next->name() == 0 ) {
				current_global = 0;
				sym.null();
				state = SSnomore;
				continue;
			}
			current_global = next;
		}
		if ( current_global ) {
			sym = process->find_global( current_global->name() );
//DBG			if ( debugflag & DBG_SYMSTREAM )
//DBG				printe("find_global('%s') found '%s'\n",
//DBG					current_global->name(), SYMNAME(sym));
			if ( sym.isnull() ) {
				current_global = 0;
				state = SSnomore;
				continue;
			}
		} else {
			// must not be any more globals...
			current_global = 0;
			sym.null();
			state = SSnomore;
			continue;
		}

		// now have a non-null symbol in sym
		if ( !match_name( SYMNAME(sym), pattern, has_wildcards ) ) {
			sym.null();
			continue;
		}
		// got a match
		lab.push( ":" );
		lab.push( SYMNAME(sym) );
		++global_count;
		break;
	case SSnomore:
		goto done;		// nothing to do
	} // end of switch() and while()

done:
//DBG	if ( debugflag & DBG_SYMSTREAM )
//DBG		printe("sym.isnull() = %d\n", sym.isnull());
//DBG	if ( debugflag & DBG_SYMSTREAM )
//DBG		printe("SymbolStream::next() returns '%s'\n", SYMNAME(sym));

	return sym;
}

Decomp::Decomp( SDBinfo *p, TYPE *t, int should_complete, int pnii )
{
//DBG	if ( debugflag & DBG_DECOMP )
//DBG		printe("new Decomp(%#x, %#x, %d) this = %#x\n",
//DBG				p, t, should_complete, this);
	node = p;
	first_time = 1;
	isfield = 0;
	isindex = 0;
	index = 0;
	field.null();
	if ( t )
		type = *t;
	else
		type.null();
	next_decomp = 0;
	completable = should_complete;
	prev_node_is_integer = pnii;
//DBG	dump(type,"Decomp constructor type");
}

Decomp::~Decomp()
{
//DBG	if ( debugflag & DBG_DECOMP )
//DBG		printe("delete Decomp %#x\n", this);
	if ( next_decomp )
		delete next_decomp;
	if ( node && node->temporary ) {
		if ( node->prev_decomp )
			node->prev_decomp->next_decomp = 0;
//DBG		if ( debugflag & DBG_DECOMP )
//DBG			printe("Decomp::~Decomp() this = %#x, deletes %#x\n",
//DBG					this, node);
		delete node;
	}
}

void
Decomp::complete( TYPE *type )
{
//DBG	if ( debugflag & DBG_COMPLETE )
//DBG		printe("complete() node = %#x, node->next_decomp = %#x, completable = %d\n",
//DBG			node, node?node->next_decomp:0, completable);

	if ( !node )
		return;

	if ( node->next_decomp && !next_decomp) { // not the last SDBinfo
		next_decomp =
			new Decomp( node->next_decomp, type, completable,
				node->kind == ikINTEGER );
		return;
	}

	Symbol user;
	if ( !type->user_type(user) ) {
//DBG		if ( debugflag & DBG_COMPLETE )
//DBG			dump(*type, "complete() fund type");
		next_decomp = new Decomp( 0, type, completable,
				node->kind == ikINTEGER );
		return;				// not a struct/union/array
	}

//DBG	if ( debugflag & DBG_COMPLETE )
//DBG		dump(*type, "complete() user type");

	if ( !completable )
		return;

	Tag t = user.tag();
	SDBinfo *next = 0;
//DBG	if ( debugflag & DBG_COMPLETE )
//DBG		printe("complete() t = %s\n", tag_string(t));
	if ( (t == t_structuretype) || (t == t_uniontype) ) {
		next = new SDBinfo(ikDOT);
		next->select.field = "*";
		next->temporary = 1;
		SDBinfo_append_decomp( node, next );
		next_decomp = new Decomp( next, type, 1,
				node->kind == ikINTEGER );
	} else if ( t == t_arraytype ) {
		TYPE memtype;
		Fund_type fund;
		type->deref_type(memtype, 0);
		if ( memtype.fund_type(fund) && (fund == ft_char) ) {
			// don't index, treat as a string
			next_decomp = new Decomp( 0, type, 1,
				node->kind == ikINTEGER );
		} else {
			next = SDBinfo_index(1, 0, -1); // [*]
			next->temporary = 1;
			SDBinfo_append_decomp( node, next );
			next_decomp = new Decomp( next, type, 1,
				node->kind == ikINTEGER );
		}
	} else if ( t == t_enumtype ) {
		next_decomp = new Decomp( 0, type, completable,
				node->kind == ikINTEGER );
	} else if ( t == t_pointertype && node && node->prev_decomp ) {
			// don't deref, to avoid circularity
		next_decomp = new Decomp( 0, type, 0,
				node->kind == ikINTEGER );
	}
//DBG	if ( debugflag & DBG_COMPLETE )
//DBG		printe("complete() next = %#x\n", next);
}

static void
get_bounds( TYPE &type, int &lo, int &hi, int &span )
{
	Symbol symbol;
	Attribute *a;

	lo = 0;
	hi = 0;
	span = 0;

//DBG	dump(type,"get_bounds");

	if ( !type.user_type(symbol) ) {
		return;
	}

	if ( symbol.tag() == t_pointertype ) {	// must de-ref
		TYPE dtype;
		type.deref_type(dtype);
		lo = 0;
		hi = 0;
		span = dtype.size();
		return;
	}

	a = symbol.attribute(an_lobound);
	if (a != 0 && a->form == af_int) {
		lo = a->value.word;
	}

	a = symbol.attribute(an_hibound);
	if (a != 0 && a->form == af_int) {
		hi = a->value.word;
	}

	int size = type.size();
	int nelem = hi - lo + 1;
	if ( nelem > 0 )
		span = size / nelem;
//DBG	if ( debugflag & DBG_BOUNDS )
//DBG		printe("bounds = [%d;%d], nelem = %d, size = %d, span = %d\n",
//DBG			lo, hi, nelem, size, span);
}

//DBGstatic int ReTuRn = 0;

#ifdef DBG
#define Return(val)	return ((ReTuRn=(val)),((debugflag&DBG_DECOMP)?(printe("%x.next() returns %d at line %d\n", this, ReTuRn, __LINE__)):0)), (ReTuRn)
#else
#define Return(val)	return val
#endif

int
Decomp::next()
{
//DBG	if ( debugflag & DBG_DECOMP )
//DBG		printe("Decomp::next() this = %#x, node = %#x, next_decomp = %#x, first_time = %d\n",
//DBG		this, node, next_decomp, first_time);

	if ( !node ) {
		if ( first_time ) {	// match only once
			first_time = 0;
			Return(1);
		} else {
			Return(0);
		}
	}

	// have a valid SDBinfo node

	if ( next_decomp && next_decomp->next() )	// pass the buck
		Return(1);

	// no next_decomp, or next_decomp->next() returned 0

	if ( next_decomp ) {
		delete next_decomp;	// recursive delete
		next_decomp = 0;
	}

	// now have valid node, but no next_decomp

	isfield = (node->kind == ikDOT || node->kind == ikREF);
	isindex = (node->kind == ikINDEX);

//DBG	if ( debugflag & DBG_DECOMP )
//DBG		printe("Decomp::next() this = %#x, isfield = %d, isindex = %d\n",
//DBG		this, isfield, isindex);

	if ( isfield ) {
//DBG		if ( debugflag & DBG_DECOMP ) printe("%#x is field\n", this);
		if ( first_time ) {
//DBG			if ( debugflag & DBG_DECOMP )
//DBG				printe("%#x first_time\n", this);
			first_time = 0;
//DBG			dump(type,"first_time, type");
			if ( !type.isnull() ) {
			    if ( node->kind == ikREF && !prev_node_is_integer ) {
				Tag tag;
				TYPE deref_type;
				int ret = type.deref_type(deref_type, &tag);
				if ( tag != t_pointertype ) {
//DBG				    if ( debugflag & DBG_DECOMP )
//DBG				    	printe("can't apply `->' to non-pointer\n");
				    Return(1);
				} else if ( ret == 0 ) {
				    printe("can't dereference\n");
				    Return(0);
				}
				type = deref_type;
//DBG				dump(type,"dereffed type");
			    }
			    if ( type.form() == TF_user ) {
				Symbol symbol;
				type.user_type(symbol);
				field = symbol.child();
//DBG				if ( debugflag & (DBG_DECOMP|DBG_TYPES) )
//DBG				    printe("type.form() = TF_user, field.isnull() = %d, field.name() = '%s'\n", field.isnull(), field.name());
//DBG				dump(field,"first field");
			    } else {
				printe("can't dereference, not user type\n");
				Return(0);
			    }
			} else {
			    printe("(null type) %s %s\n",
					(node->kind == ikDOT) ? "." : "->",
					node->select.field);
			    Return(0);
			}
		} else {	// not first_time
			field = field.sibling();
//DBG			if ( debugflag & (DBG_DECOMP|DBG_TYPES) )
//DBG				printe("field.name() = '%s'\n",
//DBG					field.name());
//DBG			dump(field,"next field");
		}
		while ( !field.isnull() ) {
			TYPE memtype;
//DBG			if ( debugflag & (DBG_DECOMP|DBG_TYPES) )
//DBG				printe("%#x.field.name() = '%s'\n",
//DBG					this, field.name());
			if ( interrupted )
				Return(0);
			switch( field.tag() ) {
			case t_unionmem:
			case t_structuremem:
			case t_bitfield:
			    if ( match_pattern( node->select.field,
						field.name() ) ) {
//DBG				dump(field,"matching field");
				field.type(memtype);
				if ( !completable )
					Return(1);
				complete ( &memtype );
				if ( next_decomp && next_decomp->next() )
				    Return(1);
				// else continue while()
				delete next_decomp;
			    }
			    break;
			default:
			    printe("unexpected tag in struct/union\n");
			    break;
			}
			field = field.sibling();
		}
		Return(0);
	} else if ( isindex ) {
		if ( first_time ) {
			first_time = 0;
			int low, high;
			if ( !type.isnull() ) {
			    get_bounds(type, low, high, span);
			} else {
			    if ( node->index.is_star )
			        printe("(null type)[*]\n");
			    else if ( node->index.low == node->index.high )
				printe("(null type)[%d]\n", node->index.low);
			    else
				printe("(null type)[%d;%d]\n",
					node->index.low, node->index.high);
			    Return(0);
			}
			if ( node->index.is_star ) {
			    node->index.is_star = 0;
			    node->index.low = low;
			    node->index.high = high;
			}
			index = node->index.low;
//DBG			if ( debugflag & DBG_DECOMP )
//DBG			    printe("%#x.isindex: index = %d\n",
//DBG					this, index);
			TYPE memtype;
			type.deref_type(memtype, 0);
//DBG			dump(type,"index(first time), memtype");
			complete( &memtype );
			if ( next_decomp && next_decomp->next() )
			    Return(1);
			else
			    Return(0);	// no more
		} else {	// not first_time
//DBG			if ( debugflag & DBG_DECOMP )
//DBG				printe("%#x.isindex: index = %d\n",
//DBG					this, index+1);
			if ( ++index <= node->index.high ) {
				TYPE memtype;
				type.deref_type(memtype, 0);
//DBG				dump(type,"index(not first time), memtype");
				complete( &memtype );
				if ( next_decomp && next_decomp->next() )
				    Return(1);
				else
				    Return(0);	// no more
			} else {
				Return(0);
			}
		}
	} else {	// not a field or an index
		if ( first_time ) {
			first_time = 0;
			// must be first node, try for next decomposition
			complete( &type );
			if ( next_decomp )
				Return(next_decomp->next());
			else
			    	Return(1);	// first_time only
		} else {
			Return(0);	// not first_time, match only once
		}
	}
	// not reached
}

static Locdesc loc;

long
Decomp::cur_offset(Process *proc, Frame *frame)
{
	long offset = 0;
	if ( isfield ) {
		if ( field.locdesc(loc) ) {
			Place p = loc.place( proc, frame );
			if ( p.kind == pAddress ) {
			    offset = p.addr;
			} else {
			    printe("bad place->kind in Decomp::cur_offset()\n");
			}
		} else {
			printe("can't get location for field '%s'\n",
				field.name());
		}
	} else if ( isindex ) {
		offset = index * span;
	} else {
		if ( debugflag )
			printe("cur_offset() is neither field nor index\n");
	}
	return offset;
}

Expr::Expr(Symbol s)	// s should be a function argument, see callstack.C
{
	estring = 0;
	lab = s.name();
	own_tree = 1;
	etree = SDBinfo_name( 0, 0, lab );
	place = 0;
	process = 0;
	frame = 0;
	rval = 0;
	current_sym = s;
	yield_count = 0;
	decomp = 0;
	symbolstream = 0;
	base = 0;
	mode = sARG;
	returnval = 0;
	isbitfield = 0;
	bits = 0;
	bito = 0;
}

Expr::Expr(char *e)	// used only by watchpoint mechanism
{
	estring = str(e);
	lab = estring;
	own_tree = 0;
	etree = 0;
	place = 0;
	process = 0;
	frame = 0;
	rval = 0;
	yield_count = 0;
	decomp = 0;
	symbolstream = new SymbolStream();
	base = 0;
	mode = sWATCH;
	returnval = 0;
	isbitfield = 0;
	bits = 0;
	bito = 0;
}

Expr::Expr(SDBinfo *t)	// used by parser.Y
{
	etree = t;
	own_tree = 0;
	estring = 0;
	lab = 0;
	place = 0;
	process = 0;
	frame = 0;
	rval = 0;
	yield_count = 0;
	decomp = 0;
	symbolstream = new SymbolStream();
	base = 0;
	mode = sUNINIT;
	returnval = 0;
	isbitfield = 0;
	bits = 0;
	bito = 0;
}

Expr::Expr( Expr &e )
{
	etree = e.etree;
	own_tree = 0;
	estring = str(e.estring);
	lab = e.lab ? str(e.lab) : 0;
	place = new Place;
	*place = *e.place;
	process = e.process;
	frame = e.frame;
	current_sym = e.current_sym;
	rval = new Rvalue;
	*rval = *e.rval;
	yield_count = 0;
	decomp = 0;
	symbolstream = new SymbolStream( *e.symbolstream );
	base = new Place;
	*base = *e.base;
	mode = e.mode;
	returnval = e.returnval;
	isbitfield = e.isbitfield;
	bits = e.bits;
	bito = e.bito;
}

Expr &
Expr::operator=( Expr &e )
{
	etree = e.etree;
	own_tree = 0;
	estring = str(e.estring);
	lab = e.lab ? str(e.lab) : 0;
	place = new Place;
	*place = *e.place;
	process = e.process;
	frame = e.frame;
	current_sym = e.current_sym;
	rval = new Rvalue;
	*rval = *e.rval;
	yield_count = 0;
	decomp = 0;
	symbolstream = new SymbolStream( *e.symbolstream );
	base = new Place;
	*base = *e.base;
	mode = e.mode;
	returnval = e.returnval;
	isbitfield = e.isbitfield;
	bits = e.bits;
	bito = e.bito;
	return *this;
}

Expr::~Expr()
{
	if ( own_tree && etree )
		delete etree;
	delete place;
	delete rval;
	delete symbolstream;
	delete decomp;
	delete base;
	delete returnval;
}

int
Expr::eval( int m, Process *proc, Iaddr pc, Frame *frm )
{
	process = proc ? proc : current_process;
	frame = frm ? frm : process->topframe();

	if ( mode == sARG ) {
		TYPE type;
		current_sym.type(type);
		decomp = new Decomp( etree, &type, 1 );
		if ( !find_lval(pc) ) {
//DBG			if ( debugflag & DBG_ERRORS )
//DBG				printe("find_lval() failed in Expr::eval()\n");
			return 0;	// can't find l-value
		}
		evaluate(0);
//DBG		if ( debugflag & DBG_ERRORS )
//DBG			printe("Expr::eval() returns %d\n", yield_count);
		return yield_count;
	}

	if ( mode != sWATCH ) {
		mode = (m==EV_LHS) ? sLHS : sVAL;
	}
	if ( !etree && !parse() ) {
//DBG		if ( debugflag & DBG_ERRORS )
//DBG			printe("parse() failed in Expr::eval()\n");
		return 0;	// parse failed
	}
	resolve(pc);
	if ( !find_lval(pc) ) {
//DBG		if ( debugflag & DBG_ERRORS )
//DBG			printe("find_lval() failed in Expr::eval()\n");
		return 0;	// can't find l-value
	}
	evaluate(0);
//DBG	if ( debugflag & DBG_ERRORS )
//DBG		printe("Expr::eval() returns %d\n", yield_count);
	return yield_count;
}

int
Expr::print( char *fmt,  Process *proc, Iaddr pc, Frame *frm )
{
	mode = sPRINT_VAL;
	if ( !etree && !parse() ) {
//DBG		if ( debugflag & DBG_ERRORS )
//DBG			printe("parse() failed in Expr::print()\n");
		return 0;	// parse failed
	}
	process = proc ? proc : current_process;
	frame = frm ? frm : process->topframe();
	while ( resolve(pc) ) {
		if ( !find_lval(pc) ) {
//DBG			if ( debugflag & DBG_ERRORS )
//DBG				printe("find_lval() failed in Expr::print()\n");
			return 0;	// can't find l-value
		}
		while ( evaluate(fmt) )
			;
	}
//DBG	if ( debugflag & DBG_ERRORS )
//DBG		printe("Expr::print() returns %d\n", yield_count);
	return yield_count;
}

int
Expr::printaddr( char *fmt,  Process *proc, Iaddr pc, Frame *frm )
{
	mode = sPRINT_ADDR;
	int ret = 0;
	if ( !etree && !parse() ) {
//DBG		if ( debugflag & DBG_ERRORS )
//DBG			printe("parse() failed in Expr::printaddr()\n");
		return 0;	// parse failed
	}
	process = proc ? proc : current_process;
	frame = frm ? frm : process->topframe();
	while ( resolve(pc) ) {
		if ( !find_lval(pc) ) {
//DBG			if ( debugflag & DBG_ERRORS )
//DBG				printe("find_lval() failed in Expr::printaddr()\n");
			return 0;	// can't find l-value
		}
		while ( evaluate(fmt) )
			;
	}
//DBG	if ( debugflag & DBG_ERRORS )
//DBG		printe("Expr::printaddr() returns %d\n", yield_count);
	return yield_count;
}

int
Expr::lvalue( Place &p )
{
	if ( place )
		p = *place;
	return (place != 0);
}

int
Expr::rvalue( Rvalue &rv )
{
	if ( rval )
		rv = *rval;
	return (rval != 0);
}

Symbol
Expr::lhs_symbol()
{
	return current_sym;
}

Iaddr
Expr::next_addr()
{
	if ( place && place->kind == pAddress && rval ) {
		Iaddr next = place->addr + rval->size();
		return (next + (sizeof(int)-1)) & ~(sizeof(int) - 1);
	}
	return 0;
}

int
Expr::parse()
{
	char *line = sf(">>>> %s\n", estring);
	::parse(line,&etree);
//DBG	if ( debugflag & DBG_PARSE )
//DBG		printe("parse('>>>> %s',%#x) = ", estring, &etree),
//DBG				dump_SDBinfo(etree);
	return etree != 0;
}

static int
getreg( char *lab, Process *proc, Frame *frame, RegRef reg,
					Stype &stype, Itype &itype)
{
//DBG	if ( debugflag & DBG_GETREG )
//DBG		printe("getreg( %d, Stype %d ) frame = %#x\n",
//DBG						reg, stype, frame);
	if ( reg == -1 ) {
		return 0;
	}
	if ( proc->is_proto() ) {
		printe("no process\n");
		return 0;
	}
	RegAttrs *a = regattrs( reg );
	if ( stype == SINVALID )
		stype = a->stype;
	if ( frame->readreg( reg, stype, itype) ) {
		printe("can't read register %s\n", lab);
		return 0;
	}
	return 1;
}

static int bitmask[33] = {	// mask off all but low "n" bits (0<=n<=32)
	0x00000000,
	0x00000001, 0x00000003, 0x00000007, 0x0000000f,	//  1,  2,  3,  4
	0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,	//  5,  6,  7,  8
	0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,	//  9, 10, 11, 12
	0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,	// 13, 14, 15, 16
	0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,	// 17, 18, 19, 20
	0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,	// 21, 22, 23, 24
	0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,	// 25, 26, 27, 28
	0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff,	// 29, 30, 31, 32
};

static int extract( int i, int bits, int bito, TYPE type )
{
	register int n = i >> (32 - (bits+bito));
	n &= bitmask[bits];
	Fund_type ft;
	if ( type.fund_type(ft) ) {
		switch( ft ) {
		case ft_schar:
		case ft_sshort:
		case ft_sint:
		case ft_slong:
			// sign extend
			if ( n & (1<<(bits-1)) )
				n |= ~bitmask[bits];
			break;
		}
	}
	return n;
}

Rvalue *
Expr::get_rvalue(int repeat)
{
	if ( !base ) {		// no lvalue, must be a constant
		if ( repeat != 1 ) {
			printe("get_rvalue(%d) base == 0\n", repeat);
			return 0;
		}
//DBG		if ( debugflag & DBG_RVAL )
//DBG			printe("get_rvalue(%d) constant kind %d\n",
//DBG					repeat, etree->kind);
		void *data;
		int len;
		TYPE type;
		switch ( etree->kind ) {
		case ikINTEGER:
			data = &etree->integer;
			len  = sizeof(long);
			type = ft_ulong;
			break;
		case ikCHAR:
			data = &etree->cchar;
			len  = 1;
			type = ft_char;
			break;
		case ikFLOAT:
			data = &etree->dbl;
			len  = sizeof(double);
			type = ft_lfloat;
			break;
		case ikSTRING:
			data = etree->string;
			len  = strlen(etree->string) + 1;
			type = process->ptr_type( ft_char );
			break;
		}
		Rvalue *rval = new Rvalue(data, len, type, repeat);
		if (etree->kind == ikSTRING) {
#ifndef m88k
// this causes sdb on the m88k to fail - the problem still needs to
// be tracked down
			delete data;
#endif
		}
		return rval;
	}
	if ( mode == sPRINT_RESULT ) {		// already have bytes, just
		Rvalue *rval = 0;		// re-package
		if ( repeat != 1 ) {
			printe("get_rvalue() mode = sPRINT_RESULT, repeat = %d\n", repeat);
			return 0;
		}
		if ( place->kind != pAddress ) {
			printe("get_rvalue() mode = sPRINT_RESULT, place->kind = %d\n", place->kind);
			return 0;
		}
		int len;
		if ( isbitfield ) {
			len = sizeof(int);
		} else {
			len = type.size();
		}
		if ( len <= 0 ) {
			printe("Assuming %s is int\n", (char *)decomp_lab);
			len = sizeof(int);
			type = ft_int;
		}
		char *buf = new char[len];
		memcpy(buf, (void *)place->addr, len);
//DBG		if ( debugflag & DBG_CALL ) {
//DBG			dump(type, "in get_rvalue()");
//DBG			printe("get_rvalue() place->addr = %#x, returnval->raw() = %#x\n",
//DBG				place->addr, returnval->raw());
//DBG			printe("len = %d, buf = '%08x'\n", len, *(long*)buf);
//DBG		}
		if ( isbitfield ) {
			int n = *(int *)buf;
			n = extract(n, bits, bito, type);
			*(int *)buf = n;
		}
		rval = new Rvalue(buf, len, type, 1);
		delete buf;
		return rval;
	}
	if ( place->kind == pRegister ) {
		if ( repeat != 1 ) {
			printe("can't use count with register object\n");
			return 0;
		}
		RegAttrs *a = regattrs(place->reg);
		if ( !a ) {
			printe("unknown RegRef %d in get_rvalue()\n",
								place->reg);
			return 0;
		}
//DBG		if ( debugflag & DBG_RVAL )
//DBG			printe("get_rvalue(), reg %s, size %d\n",
//DBG				place->reg, a->size);
		Stype stype = a->stype;
		Itype itype;
		if ( !getreg( lab, process, frame, place->reg,
				stype, itype ) )
			return 0;
		return new Rvalue(stype, itype);
	} else if ( place->kind == pAddress ) {
		int len;
//DBG		if ( debugflag & DBG_RVAL )
//DBG			dump(type, "get_rvalue() type");
		if ( isbitfield ) {
			len = repeat * sizeof(int);
		} else {
			len = repeat * type.size();
		}
//DBG		if ( debugflag & DBG_RVAL )
//DBG			printe("get_rvalue() isbitfield = %d, type.size = %d, repeat = %d, len = %d\n",
//DBG					isbitfield, type.size(), repeat, len);
		if ( len <= 0 ) {
			printe("Assuming %s is int\n", (char *)decomp_lab);
			len = repeat * sizeof(int);
			type = ft_int;
		}
		char *buf = new char[len];
		int ret = process->read( place->addr, len, buf );
		if ( ret <= 0 ) {
			printe("can't read address %#x\n", place->addr);
			delete buf;
			return 0;
		} else if ( ret < len ) {
			printe("only got %d of %d bytes from address %#x\n",
				ret, len, place->addr);
			delete buf;
			return 0;
		}
//DBG		if ( debugflag & DBG_RVAL )
//DBG			printe("get_rvalue(), addr %#x, len %d\n",
//DBG				place->addr, len);
		if ( isbitfield ) {
//DBG			if ( debugflag & DBG_RVAL )
//DBG				printe("get_rvalue(), isbitfield, bits = %d, bito = %d\n", bits, bito);
			for ( int i = 0 ; i < repeat ; i++ ) {
				int n = ((int *)buf)[i];
				n = extract(n, bits, bito, type);
				((int *)buf)[i] = n;
			}
			type = ft_int;
		}
		Rvalue *rval = new Rvalue(buf, len, type, repeat);
		delete buf;
		return rval;
	} else {
		printe("bad place->kind (%d) in get_rvalue()!\n", place->kind);
		return 0;
	}
}

int
Expr::resolve(Iaddr pc)
{
	TYPE type;
	RegAttrs *a;
	int complete = 0;

	if ( pc == ~0 )
		pc = process->pc_value();

	delete decomp;
	decomp = 0;			// reset iterator

	switch( etree->kind ) {
// first 8 cases return and do not fall out of switch	
	default:
	case ikUninitialized:
		printe("bad kind in Expr::resolve() (%d)\n", etree->kind);
		return 0;
	case ikDOT:
	case ikREF:
	case ikINDEX:
	case ikCALL:
	case ikASSIGN:
		printe("can't happen: kind == %d in Expr::resolve()\n",
				etree->kind);
		return 0;
	case ikNAME:
		if ( mode != sWATCH || rval == 0 ) {
			if ( current_sym.isnull() ) {
				symbolstream->init( etree->name.id,
					etree->name.procedure,
					etree->name.level,
					process, frame,
					etree->name.global_only );
			}
			current_sym = symbolstream->next();
		}
		lab = symbolstream->label();
//DBG		if ( debugflag & DBG_RESOLVE )
//DBG			printe("resolve()d to '%s'\n", lab);
		current_sym.type(type);
		complete = (mode != sARG);
		decomp = new Decomp( etree, &type, complete );
		if (current_sym.isnull() && !etree->name.global_only
			&& !symbolstream->globalcount()
			&& !symbolstream->found_local) {
			etree->name.global_only = 1;
			return(resolve(pc));
		}
		return !current_sym.isnull();

// last 5 cases fall out of switch...
	case ikINTEGER:
		lab = sf("%#x", etree->integer);
		if ( etree->next_decomp ) {
			type = process->SDB_type;
			complete = 1;
		} else {
			type = ft_pointer;	// so default format is 'x'
		}
		break;
	case ikREGISTER:
		a = regattrs( etree->reg.regref );
		lab = (a) ? str(a->name) : "unknown register";
		if ( a->flags & FPREG )
			type = ft_lfloat;
		else
			type = ft_pointer;	// so default format is 'x'
		break;
	case ikCHAR:
		type = ft_char;
		break;
	case ikFLOAT:
		type = ft_lfloat;	// long float (double)
		break;
	case ikSTRING:
		type = process->ptr_type( ft_char );
		break;
	}

	decomp = new Decomp( etree, &type, complete );

	return yield_count++ < 1;		// once only
}

int
Expr::find_lval(Iaddr pc)
{
	if ( pc == ~0 )
		pc = process->pc_value();

	delete place;
	place = 0;
	delete base;
	base = 0;

	switch( etree->kind ) {
	default:
	case ikUninitialized:
		printe("bad kind in Expr::find_lval() (%d)\n", etree->kind);
		return 0;
	case ikINTEGER:
		if ( mode == sVAL || mode == sARG ) {
			;			// int on right has no lval
		} else {
			base = new Place;	// int on left is address
			base->kind = pAddress;
			base->addr = etree->integer;
		}
		break;
	case ikREGISTER:
		base = new Place;
		base->kind = pRegister;
		base->reg  = etree->reg.regref;
		break;
	case ikNAME:
		if ( current_sym.isnull() ) {
			printe("%s: No match\n", etree->name.id);
			return 0;
		} else {
			Locdesc loc;
//DBG			dump(current_sym,"find_lval current_sym");
			if ( current_sym.locdesc(loc) ) {
				base = new Place;
				Frame *f = symbolstream ?
					symbolstream->frame() : frame;
				*base = loc.place(process, f, 0);
			} else {
				Attribute *a = current_sym.attribute(an_lopc);
				if ( a == 0 ) {
					printe("no address for %s\n",
						SYMNAME(current_sym));
					return 0;	// no address
				}
				base = new Place;
				base->kind = pAddress;
				base->addr = current_sym.pc(an_lopc);
			}
		}
		break;
	case ikDOT:
	case ikREF:
	case ikINDEX:
	case ikCALL:
	case ikASSIGN:
		printe("can't happen: kind == %d in Expr::find_lval()!\n",
				etree->kind);
		return 0;
	case ikCHAR:
	case ikFLOAT:
	case ikSTRING:
		break;		// not an lvalue
	}

//DBG	if ( debugflag & DBG_ADDR )
//DBG		printe("base = %#x, base->kind = %d, base->addr = %#x\n",
//DBG			base, base?base->kind:0, base?base->addr:0);

	return 1;
}

static int de_ref( char *lab, Place *place, Process *process, Frame *frame )
{
	Itype itype;
	if ( !place )
		return 0;
	if ( !process ) {
		printe("no process\n");
		return 0;
	}
	if ( place->kind == pAddress ) {
		Iaddr addr;
		int ret = process->read(place->addr,
					sizeof(Iaddr), (char *)&addr);
		if ( ret != sizeof(Iaddr) ) {
			printe("can't read address %#x\n", place->addr);
			return 0;
		}
		place->addr = addr;
	} else if ( place->kind == pRegister ) {
		if ( !frame ) {
			printe("can't access registers\n");
			return 0;
		}
		Stype stype = Saddr;
		if ( !getreg( lab, process, frame, place->reg, stype, itype ) )
			return 0;
		place->kind = pAddress;
		place->addr = itype.iaddr;
	} else {
		printe("bad place->kind (%d) in de_ref()!\n", place->kind);
		return 0;
	}
	return 1;
}

static void set_aggr( Process *process, TYPE type, int &done )
{
	if ( done )
		return;

	Symbol ut;

	if ( type.user_type(ut) ) {
		Tag t = ut.tag();
		switch( t ) {
		case t_structuretype:
		case t_uniontype:
			process->SDB_type = type;
			done = 1;
			break;
		}
	}
}

static int isbit( Symbol symbol, int &siz, int &off )
{
	if ( !symbol.isnull() ) {
		Attribute *a = symbol.attribute(an_bitsize);
		Attribute *b = symbol.attribute(an_bitoffs);
		if ( a && b && a->form == af_int && b->form == af_int ) {
			siz = a->value.word;
			off = b->value.word;
			return 1;
		}
	}
	return 0;
}

void
Expr::do_decomps()
{
	Decomp *curdecomp = decomp;
	Decomp *prev_decomp = decomp;
	decomp_lab.clear();
	decomp_lab.push(lab);

	int aggr_done = 0;

	Frame *f = symbolstream ? symbolstream->frame() : 0;
	if ( !f ) f = process->topframe();

	type.null();

	delete place;
	place = 0;

//DBG	if ( debugflag & DBG_ADDR )
//DBG		printe("do_decomps() curdecomp = %#x\n", curdecomp);

	if ( !decomp )
		return;

	place = new Place;
	if ( base ) {
		*place = *base;
	} else {	// no lvalue to modify, so lie
		place->kind = pAddress;
		place->addr = 0;
	}

	while ( curdecomp ) {
		set_aggr( process, curdecomp->cur_type(), aggr_done );
		if ( (mode == sLHS || mode == sVAL || mode == sWATCH) &&
				curdecomp->nextdecomp() &&
				curdecomp->nextdecomp()->pos() &&
				curdecomp->nextdecomp()->pos()->temporary ) {
			// don't continue; want WHOLE thing
			type = curdecomp->cur_type();
//DBG			dump(type,"do_decomps, WHOLE type");
			isbitfield = isbit( curdecomp->fieldsym(), bits, bito );
			set_aggr( process, type, aggr_done );
			break;
		}
		if ( curdecomp->pos() ) {
			switch( curdecomp->pos()->kind ) {
			case ikDOT:	decomp_lab.push(".");	break;
			case ikREF:	decomp_lab.push("->");	break;
			case ikINDEX:	decomp_lab.push("[");	break;
			}
		}	
		if ( curdecomp->is_field() ) {
//DBG			if ( debugflag & DBG_ADDR )
//DBG			    printe("%#x is_field, cur_offset() = %d\n",
//DBG				curdecomp,
//DBG				curdecomp->cur_offset(process, frame));
//DBG			dump(curdecomp->cur_type(),"cur_type()");
			if ( curdecomp->pos()->kind == ikREF ) {
//DBG			    if ( debugflag & DBG_ADDR )
//DBG				printe("de-reffing pointer\n");
			    if ( !de_ref( lab, place, process, f ) )
				return;
//DBG			    if ( debugflag & DBG_ADDR )
//DBG				printe("de-reffed pointer = %#x\n",
//DBG					place->addr);
			}
			if ( place->kind != pAddress ) {
			    printe("register DOT field!\n");
			    return;
			}
			place->addr += curdecomp->cur_offset(process, frame);
			decomp_lab.push( curdecomp->cur_name() );
		} else if ( curdecomp->is_index() ) {
//DBG			if ( debugflag & DBG_ADDR )
//DBG			    printe("%#x is_index, cur_offset() = %d\n",
//DBG				curdecomp,
//DBG				curdecomp->cur_offset(process, frame));
			dump(curdecomp->cur_type(),"cur_type()");
			Symbol user;
			curdecomp->cur_type().user_type(user);
			if ( !user.isnull() && user.tag() == t_pointertype ) {
//DBG			    if ( debugflag & DBG_ADDR )
//DBG				printe("de-reffing pointer\n");
			    if ( !de_ref( lab, place, process, f ) )
				return;
//DBG			    if ( debugflag & DBG_ADDR )
//DBG				printe("de-reffed pointer = %#x\n",
//DBG					place->addr);
			}
			if ( place->kind != pAddress ) {
			    printe("register not de-reffed!\n");
			    return;
			}
			place->addr += curdecomp->cur_offset(process, frame);
			decomp_lab.push( sf("%d]",curdecomp->cur_index()) );
		} else {
//DBG			if ( debugflag & DBG_ADDR )
//DBG			    printe("%#x is neither index nor field, mode = %d, nextdecomp() = %#x\n",
//DBG					curdecomp, mode, curdecomp->nextdecomp());
		}
		if ( !curdecomp->nextdecomp() ) {
			type = curdecomp->cur_type();
//DBG			dump(type,"do_decomps, last decomp");
			isbitfield = isbit( prev_decomp->fieldsym(),
							bits, bito );
		}
		prev_decomp = curdecomp;
		curdecomp = curdecomp->nextdecomp();
		if ( mode == sPRINT_ADDR &&
				curdecomp &&
				curdecomp->pos() &&
				curdecomp->pos()->temporary ) {
			decomp = 0;
			break;
		}
	}
//DBG	if ( debugflag & DBG_ADDR )
//DBG		printe("do_decomps() curdecomp = %#x, final kind = %s, %#x\n",
//DBG			curdecomp,
//DBG			(place->kind==pAddress) ? "pAddress" : "pRegister",
//DBG			place->addr);

}

char *default_fmt( TYPE type )
{
	Fund_type fund;
	Symbol user;
	char *fmt = "!NONE!";
	if ( type.fund_type(fund) ) {
		switch( fund ) {
		case ft_pointer:
		case ft_none:
		default:
			fmt = "x";
			break;
		case ft_char:
		case ft_uchar:
			fmt = "c";
			break;
		case ft_schar:
		case ft_short:
		case ft_sshort:
		case ft_int:
		case ft_sint:
		case ft_long:
		case ft_slong:
			fmt = "d";
			break;
		case ft_ushort:
		case ft_uint:
		case ft_ulong:
			fmt = "u";
			break;
		case ft_sfloat:
			fmt = "f";
			break;
		case ft_lfloat:
		case ft_xfloat:
			fmt = "g";
			break;
		}
//DBG		if ( debugflag & DBG_TYPES )
//DBG			printe("default_fmt() fund_type = %d, fmt = %s\n",
//DBG								fund, fmt);
	} else if ( type.user_type(user) ) {
		Tag t = user.tag();
		TYPE deref_type;
		Fund_type fund2;
		Symbol user2;
		switch( t ) {
		case t_none:
		case t_enumlittype:
		case t_structuretype:
		case t_uniontype:
		case t_functargtype:
		case t_discsubrtype:
		case t_bitfieldtype:
		default:
			fmt = "x";
			break;
		case t_enumtype:
			fmt = "#";	// not legal user format character
			break;
		case t_typedef:
			fmt = "x";	// MORE?
			break;
		case t_stringtype:
			fmt = "s";
			break;
		case t_functiontype:
			fmt = "p";
			break;
		case t_pointertype:
		case t_arraytype:
//DBG			dump(user,"t_{pointer|array}type user");
			if ( !type.deref_type(deref_type) ) {
//DBG				if ( debugflag & DBG_TYPES )
//DBG					printe("pointer or arraytype has no deref_type!\n");
				fmt = "x";
			} else if ( deref_type.fund_type(fund2) &&
							fund2 == ft_char ) {
//DBG				if ( debugflag & DBG_TYPES )
//DBG					printe("fund2 = ft_char\n");
				fmt = (t==t_pointertype) ? "s" : "a";
			} else if ( deref_type.user_type(user2) &&
					user2.tag() == t_functiontype ) {
//DBG				if ( debugflag & DBG_TYPES )
//DBG					printe("user2.tag() = t_functiontype\n");
				fmt = (t==t_pointertype) ? "p" : "x";
			} else {
//DBG				if ( debugflag & DBG_TYPES )
//DBG					printe("fund2 = %d\n", fund2);
				fmt = "x";
			}
			break;
		}
//DBG		if ( debugflag & DBG_TYPES ) {
//DBG			printe("default_fmt() user_type.tag = %s, fmt = %s\n",
//DBG							tag_string(t), fmt);
//DBG			dump(user,"default_fmt user_type");
//DBG		}
	} else {
//DBG		if ( debugflag & DBG_TYPES )
//DBG			printe("null type in default_fmt!\n");
		fmt = "x";
	}
	return fmt;
}

static int ft_size( Fund_type ft )
{
	switch ( ft ) {
	default:
		printe("bad Fund_type (%d) in ft_size()\n", ft);
		return 0;
	case ft_scomplex:
	case ft_lcomplex:
	case ft_set:
	case ft_void:
		return 0;
	case ft_char:
	case ft_schar:
	case ft_uchar:
		return 1;
	case ft_short:
	case ft_sshort:
	case ft_ushort:
		return 2;
	case ft_none:
	case ft_int:
	case ft_sint:
	case ft_uint:
	case ft_long:
	case ft_slong:
	case ft_ulong:
	case ft_pointer:
	case ft_sfloat:
		return 4;
	case ft_lfloat:
	case ft_xfloat:		// MORE; use real size here
		return 8;
	}
}

static void *
itype_off( Itype &itype, Stype stype, TYPE &type )
{
	char *p = (char *) &itype;
	if ( type.size() == ft_size(stype) )
		return p;
	switch ( type.size() ) {
	default:
		printe("bad type.size() %d in itype_off()\n", type.size());
		return p;
	case 8:
	case 4:
	case 2:
	case 1:
		return p + ft_size(stype) - type.size();
	}
}

// returns 0 if iteration should stop, 1 if may be called again
int
Expr::evaluate(char *fmt)
{
	int count = 1;
	int size = 0;
	char *format = "";
	int count_was_explicit = 0;
	int ret;
	Itype itype;

	char sep = (mode == sPRINT_VAL || mode == sPRINT_RESULT) ? '/' : '=';
	Frame *f = frame;
	if ( symbolstream ) f = symbolstream->frame();
	if ( !f ) f = process->topframe();

	if ( interrupted )
		return 0;

	if ( !decomp || !decomp->next() ) {	// nothing left to do
//DBG		if ( debugflag & DBG_DECOMP )
//DBG			printe("decomp = %#x, returning 0 from evaluate\n",
//DBG						decomp);
		return 0;
	}

	++yield_count;

	do_decomps();

//DBG	dump(type, "after do_decomps(), type");

	if ( mode == sLHS )
		return 0;	// exactly once

//DBG	if ( debugflag & DBG_EVALUATE )
//DBG		printe("Expr::evaluate('%s', %d) estring = '%s', lab = '%s'\n",
//DBG						fmt, mode, estring, lab);

	if ( !place ) {
//DBG		if ( debugflag & (DBG_EVALUATE|DBG_ERRORS) )
//DBG			printe("place is null in Expr::evaluate()\n");
		return 0;
	}

	if ( fmt && !parse_fmt(fmt, count, size, format, count_was_explicit) ) {
//DBG		if ( debugflag & (DBG_EVALUATE|DBG_ERRORS) )
//DBG			printe("parse_fmt() failed in Expr::evaluate()\n");
		return 0;
	}

	if ( *format == '\0' &&
			(mode == sPRINT_VAL || mode == sPRINT_RESULT) ) {
//DBG		dump(type,"evaluate, doing default_fmt");
		format = default_fmt( type );
		if ( !count_was_explicit && !size )
			fmt = format;
		else {
			// MORE!
		}
	}

	lastcom = DSCOM;	// reset by 'i' and 'I' formats

//DBG	if ( debugflag & DBG_EVALUATE )
//DBG		printe("fmt = '%s', format = '%s', size = %d, count = %d, explicit = %d\n", fmt, format, size, count, count_was_explicit);

	if ( *format == 'i' || *format == 'I' ) {
		if ( mode == sPRINT_ADDR ) {
			printe("illegal format `%s' for `=' command\n", format);
			return 0;
		}
		if ( size ) printe("length specifier ignored\n");
		if ( place->kind != pAddress ) {
			printe("bad location type for `%c' format\n", *format);
			return 0;
		}
		int symbolic = (*format == 'i');
		Location l;
		l.kind = lk_addr;
		l.locn.addr = place->addr;
		while ( count-- )
			disassemble( current_process, &l, symbolic, &l.locn.addr );
		instaddr = l.locn.addr;
		lastcom = DSICOM;
		return 1;
	} else if ( *format == '#' ) {
		// enum default format, not legal on cmd line
		if ( mode != sPRINT_VAL && mode != sPRINT_RESULT ) {
			printe("bad mode %d for enum\n", mode);
			return 0;
		}
		rval = get_rvalue(1);
		if ( rval->size() != sizeof(long) ) {
			printe("bad size for enum: %d\n", rval->size());
			return 0;
		}
		long l = *(long*)rval->raw();
//DBG		if ( debugflag & DBG_ENUM )
//DBG			printe("enum value = %#x\n", l);
//DBG		dump( type, "enum type" );
		Symbol sym;
		if ( !type.user_type(sym) ) {
			printe("enum type is not user type!\n");
			return 0;
		}
		Tag t = sym.tag();
		if ( t != t_enumtype ) {
			printe("enum type tag = %d!\n", t);
			return 0;
		}
		Symbol mem;
		for ( mem = sym.child(); !mem.isnull() ; mem = mem.sibling() ) {
//DBG			if ( debugflag & DBG_ENUM )
//DBG				dump( mem, "enum member");
			if ( mem.tag() != t_enumlittype ) {
				printe("bad tag in enum list: %d\n", mem.tag());
				return 0;
			}
			Attribute *a = mem.attribute(an_litvalue);
			if ( !a ) {
				printe("missing an_litvalue attribute!\n");
				return 0;
			}
			if ( a->value.word == l ) {	// found it!
				a = mem.attribute(an_name);
				if ( !a ) {
					printe("missing an_name attribute!\n");
					return 0;
				}
				printx("%s/%s\n", (char *)decomp_lab,
						a->value.ptr);
				return 1;
			}
		}
//DBG		if ( debugflag & DBG_ENUM )
//DBG			printe("value %#x not found\n", l);
		// not found, fall back to hex format
		printx("%s/%#x\n", (char *)decomp_lab, l);
		return 1;
	} else if ( *format == 'p' ) {
		if ( size ) printe("length specifier ignored\n");
		if ( count_was_explicit ) printe("count ignored\n");
		if ( place->kind != pAddress ) {
			if ( !de_ref( lab, place, process, frame ) )
				return 0;
		}
		if ( mode != sPRINT_ADDR ) {
			if ( !de_ref( lab, place, process, frame ) )
				return 0;
		}
		Symbol sym = process->find_entry( place->addr );
		if ( sym.isnull() ) {
			printx("%s%c%#x\n", (char *)decomp_lab, sep, place->addr);
		} else {
			long base = sym.pc(an_lopc);
			if ( base == place->addr )
				printx("%s%c%s\n", (char *)decomp_lab, sep,
					SYMNAME(sym));
			else
				printx("%s%c%s+%#x\n", (char *)decomp_lab, sep,
					SYMNAME(sym), place->addr - base);
		}
		return 1;
	} else if ( *format == 's' || *format == 'a' ) {
		if ( mode == sPRINT_ADDR ) {
			printe("illegal format `%s' for `=' command\n", format);
			return 0;
		}
		if ( size ) printe("length specifier ignored\n");
		Iaddr loc;
		if ( *format == 's' ) {
			if ( place->kind == pAddress ) {
			     if ( mode == sPRINT_RESULT ) {
				memcpy((void *)&itype.iaddr, (void *)place->addr, type.size());
			     }
			     else {
				ret = process->read(place->addr,Suint4,itype);
				if ( ret <= 0 ) {
					printx("%s/(invalid character pointer value %#x)\n",
					    (char *)decomp_lab, place->addr);
					return 1;
				}
			     }
			} else {
				Stype stype = Saddr;
				if ( !getreg( lab, process, f, place->reg,
						stype, itype ) ) {
					return 1;
				}
			}
			loc = itype.iaddr;
			if ( loc == 0 ) {
				printx("%s/(null)\n",
					    (char *)decomp_lab);
				return 1;
			}
		} else {	// *format == 'a'
			if ( place->kind == pRegister ) {
				printe("can't use `a' format with register\n");
				return 0;
			}
			loc = place->addr;
		}
		if ( !count_was_explicit ) {
			count = 128;
		}
		char *buf = new char[count+1];
		buf[0] = '\0';
		int n = process->read(loc, count, buf);
//DBG		if ( debugflag & DBG_ADDR )
//DBG			printe("process->read(%#x, %d, %#x) returned %d\n",
//DBG				loc, count, buf, n);
		buf[count] = '\0';
		if ( n <= 0 ) {
			delete buf;
			printx("%s/(invalid character pointer value %#x)\n",
					(char *)decomp_lab, loc);
			return 1;
		} else if ( n < count ) {
			delete buf;
			printx("%s/(short read (%d/%d) at %#x)",
				count, n, loc);
			return 1;
		}

//DBG		if ( debugflag & DBG_ADDR )
//DBG			printe("buf = '%s'\n", buf);

		printx("%s/\"%s\"\n", (char *)decomp_lab,
				fmt_str(buf,count,count_was_explicit));
		delete buf;
		return 1;
	}

	// if we get here, *format is a "normal" mode  [cduoxfg] or 0

	switch ( mode ) {
	case sLHS:
//DBG		if ( debugflag & DBG_EVALUATE )
//DBG			printe("got to case sLHS: in switch(mode)!\n");
		return 0;		// should have returned above...
	case sARG:
	case sVAL:
	case sWATCH:
//DBG		if ( debugflag & DBG_EVALUATE )
//DBG			printe("%s\n",
//DBG			(mode==sARG)?"sARG":(mode==sVAL)?"sVAL":"sWATCH");
		if ( place->kind == pAddress ) {
//DBG			if ( debugflag & DBG_EVALUATE )
//DBG				printe("pAddress\n");
			rval = get_rvalue(1);
			return 0;
		} else if ( place->kind == pRegister ) {
			// let getreg() determine proper Stype
			Stype stype = SINVALID;
			if ( !getreg( lab, process, f, place->reg,
					stype, itype ) ) {
				return 1;
			}
			RegAttrs *a = regattrs( place->reg );
			rval = new Rvalue( itype_off(itype, stype, type),
							type.size(), type, 1 );
			return 0;
		} else {
			printe("unknown place->kind %d\n", place->kind );
			return 0;
		}
		break;
	case sPRINT_VAL:
	case sPRINT_RESULT:
//DBG		if ( debugflag & DBG_EVALUATE )
//DBG			printe("%s\n", (mode==sPRINT_VAL)?
//DBG						"sPRINT_VAL":"sPRINT_RESULT");
		if ( place->kind == pAddress ) {
//DBG			if ( debugflag & DBG_EVALUATE )
//DBG				printe("pAddress\n");
			int fetchcount = count;
			if ( !count_was_explicit && size &&
					type.size() && type.size() < size )
				fetchcount = size / type.size();
				// fetch at least enough to make "size" bytes
			rval = get_rvalue(fetchcount);
			if ( rval )
				return rval->print(decomp_lab,fmt,"/");
			else
				return 1;
		} else if ( place->kind == pRegister ) {
			// let getreg() determine proper Stype
			Stype stype = SINVALID;
			Fund_type fund;
			type.fund_type(fund);
			if (fund == ft_lfloat)
				stype = Sdfloat;
			if ( !getreg( lab, process, f, place->reg,
					stype, itype ) ) {
				return 1;
			}
			rval = new Rvalue( stype, itype );
			if ( rval )
				return rval->print(decomp_lab,fmt,"/");
			else
				return 1;
		} else {
			printe("unknown place->kind %d\n", place->kind );
			return 1;
		}
		break;
	case sPRINT_ADDR:
		if ( place->kind == pAddress ) {
			char fmt_buf[4];
			if ( !format || !*format )
				format = "x";
			if ( count_was_explicit )
				printe("count ignored\n");
			switch ( size ) {
			case 0:
			case 4:	fmt_buf[0] = 'l';	break;
			case 2:	fmt_buf[0] = 'h';	break;
			case 1: fmt_buf[0] = 'b';	break;
			}
			fmt_buf[1] = *format;
			fmt_buf[2] = 0;
			rval = new Rvalue(place->addr);
			return rval->print(decomp_lab,fmt_buf,"=");
		} else if ( place->kind == pRegister ) {
			// grammar disallows "%pc=", but this
			// can still happen with "%pc/ ; .=" sequence
			RegAttrs *r = regattrs(place->reg);
			printx("%s=%s\n", (char *)decomp_lab, r->name);
			return 1;
		} else {
			printe("unknown place->kind %d\n", place->kind );
			return 0;
		}
		break;
	default:
		printe("bad mode (%d) in Expr::evaluate\n", mode);
		break;
	}
//DBG	if ( debugflag & (DBG_EVALUATE|DBG_ERRORS) )
//DBG		printe("Expr::evaluate() failed\n");
	return 0;
}

enum convaction {	// possible actions in converting among Fund_types
	convEQUIV,	// equivalent, nothing to do
	convNONE,	// not possible to convert
	convSEXT,	// sign extend
	convZEXT,	// zero extend
	convTRUNC,	// truncate
	convFTOI,	// is a float/double, want an int/short/long/etc.
	convITOF,	// is integral, want float/double
	convFTOD,	// is float, want double or extended
	convDTOF,	// is double or extended, want float
};

static int ft_conv_action( Fund_type from, Fund_type to )
{
	if ( from == to )
		return convEQUIV;

	switch ( to ) {
	case ft_scomplex:
	case ft_lcomplex:
	case ft_set:
	case ft_void:
		return convNONE;
	}

	switch ( from ) {
	case ft_none:
		switch ( to ) {
		case ft_none:
			return convEQUIV;
		case ft_short:	case ft_sshort:
		case ft_int:	case ft_sint:
		case ft_long:	case ft_slong:
			return convSEXT;
		case ft_char:	case ft_schar:	case ft_uchar:
		case ft_ushort:	case ft_uint:	case ft_ulong:
			return convZEXT;
		case ft_pointer:
		case ft_sfloat:	case ft_lfloat:	case ft_xfloat:
			return convNONE;
		}
		break;
	case ft_scomplex:
	case ft_lcomplex:
	case ft_set:
	case ft_void:
		return convNONE;
	case ft_schar:
		switch ( to ) {
		case ft_char:	case ft_schar:	case ft_uchar:
			return convEQUIV;
		case ft_short:	case ft_sshort:
		case ft_int:	case ft_sint:
		case ft_long:	case ft_slong:
			return convSEXT;
		case ft_ushort:	case ft_uint:	case ft_ulong:
		case ft_none:
			return convZEXT;
		case ft_pointer:
		case ft_sfloat:	case ft_lfloat:	case ft_xfloat:
			return convNONE;
		}
		break;
	case ft_char:
	case ft_uchar:
		switch ( to ) {
		case ft_char:	case ft_schar:	case ft_uchar:
			return convEQUIV;
		case ft_short:	case ft_sshort:	case ft_ushort:
		case ft_int:	case ft_sint:	case ft_uint:
		case ft_long:	case ft_slong:	case ft_ulong:
		case ft_none:
			return convZEXT;
		case ft_pointer:
		case ft_sfloat:	case ft_lfloat:	case ft_xfloat:
			return convNONE;
		}
		break;
	case ft_short:
	case ft_sshort:
		switch ( to ) {
		case ft_schar:	case ft_char:	case ft_uchar:
			return convTRUNC;
		case ft_short:	case ft_sshort:	case ft_ushort:
			return convEQUIV;
		case ft_int:	case ft_sint:
		case ft_long:	case ft_slong:
			return convSEXT;
		case ft_uint: case ft_ulong:
		case ft_none:
			return convZEXT;
		case ft_pointer:
		case ft_sfloat:	case ft_lfloat:	case ft_xfloat:
			return convNONE;
		}
		break;
	case ft_ushort:
		switch ( to ) {
		case ft_schar:	case ft_char:	case ft_uchar:
			return convTRUNC;
		case ft_short:	case ft_sshort:	case ft_ushort:
			return convEQUIV;
		case ft_int:	case ft_sint:	case ft_uint:
		case ft_long:	case ft_slong:	case ft_ulong:
		case ft_none:
			return convZEXT;
		case ft_pointer:
		case ft_sfloat:	case ft_lfloat:	case ft_xfloat:
			return convNONE;
		}
		break;
	case ft_int:
	case ft_sint:
	case ft_uint:
	case ft_long:
	case ft_slong:
	case ft_ulong:
		switch ( to ) {
		case ft_schar:	case ft_char:	case ft_uchar:
		case ft_short:	case ft_sshort:	case ft_ushort:
			return convTRUNC;
		case ft_int:	case ft_sint:	case ft_uint:
		case ft_long:	case ft_slong:	case ft_ulong:
		case ft_pointer:
		case ft_none:
			return convEQUIV;
		case ft_sfloat:	case ft_lfloat:	case ft_xfloat:
			return convITOF;
		}
		break;
	case ft_pointer:
		switch ( to ) {
		case ft_schar:	case ft_char:	case ft_uchar:
		case ft_short:	case ft_sshort:	case ft_ushort:
			return convNONE;
		case ft_int:	case ft_sint:	case ft_uint:
		case ft_long:	case ft_slong:	case ft_ulong:
		case ft_pointer:
		case ft_none:
			return convEQUIV;
		case ft_sfloat:	case ft_lfloat:	case ft_xfloat:
			return convNONE;
		}
		break;
	case ft_sfloat:
		switch ( to ) {
		case ft_schar:	case ft_char:	case ft_uchar:
		case ft_short:	case ft_sshort:	case ft_ushort:
			return convNONE;
		case ft_int:	case ft_sint:	case ft_uint:
		case ft_long:	case ft_slong:	case ft_ulong:
		case ft_none:
			return convFTOI;
		case ft_pointer:
			return convNONE;
		case ft_sfloat:
			return convEQUIV;
		case ft_lfloat:	case ft_xfloat:
			return convFTOD;
		}
		break;
	case ft_lfloat:
	case ft_xfloat:
		switch ( to ) {
		case ft_schar:	case ft_char:	case ft_uchar:
		case ft_short:	case ft_sshort:	case ft_ushort:
			return convNONE;
		case ft_int:	case ft_sint:	case ft_uint:
		case ft_long:	case ft_slong:	case ft_ulong:
		case ft_none:
			return convFTOI;
		case ft_pointer:
			return convNONE;
		case ft_sfloat:
			return convDTOF;
		case ft_lfloat:	case ft_xfloat:
			return convEQUIV;
		}
		break;
	}
}

static unsigned long get_long( unsigned char *p, int size )
{
	unsigned long l = 0;
#define l_byte(n)	((unsigned char *)&l)[n]
	unsigned long buf;
	unsigned char *q = (unsigned char *)&buf;
	unsigned char *r = p;
	*q++ = *r++;	*q++ = *r++;	*q++ = *r++;	*q = *r;

//DBG	if ( debugflag & DBG_ASSIGN )
//DBG		printe("get_long( '%08x', %d )\n", buf, size);

#ifdef	RTOLBYTES
	int i = 0;
#else
	int i = 4-size;
#endif
	switch ( size ) {
	default:
		printe("bad size (%d) in get_long()\n", size);
		break;
	case 4:		l_byte(i++) = *p++;
			l_byte(i++) = *p++;
	case 2:		l_byte(i++) = *p++;
	case 1:		l_byte(i++) = *p++;
	}

//DBG	if ( debugflag & DBG_ASSIGN )
//DBG		printe("get_long() returns %#x\n", l);
	return l;
}

// macro to compute byte offset in a "long" of a "size"-byte quantity
#ifdef	RTOLBYTES
#define byte_offset(size)	0
#else
#define byte_offset(size)	(4 - size)
#endif

int convert( Rvalue &rval, Fund_type from, Fund_type to )
{
	register unsigned char *p = rval.raw();
	register unsigned char *q = p;
	unsigned long l;
	double dbl;
	float flt;
	int from_size = ft_size(from);
	int to_size = ft_size(to);
	TYPE type;
	type = to;

	convaction action = ft_conv_action(from, to);

//DBG	if ( debugflag & DBG_ASSIGN )
//DBG		printe("convert(rval, from %d, to %d) action = %d\n",
//DBG				from, to, action);

	switch ( action ) {
	case convEQUIV:			// no conversion necessary
		break;
	case convNONE:			// no conversion possible
		return 0;
	case convSEXT:			// sign extend
		l = get_long(p, from_size);
		l = sext(l, from_size);
		q = (unsigned char *) &l;
		q += byte_offset(to_size);
		break;
	case convZEXT:			// zero extend
	case convTRUNC:			// truncate
		l = get_long(p, from_size);
		q = (unsigned char *) &l;
		q += byte_offset(to_size);
		break;
	case convFTOI:			// float -> int
		// convert to double, then to long, then to target type
		convert( rval, from, ft_lfloat );
		l = (long) *(double *)rval.raw();
		{ TYPE long_type; long_type = ft_long;
		  Rvalue n( &l, sizeof(long), long_type, 1);
		  rval = n;
		}
		convert( rval, ft_long, to );
		return 1;
	case convITOF:			// int -> float
		// convert to long, then to double, then to target type
		convert( rval, from, ft_long );
		dbl = *(long *)rval.raw();
		{ TYPE dbl_type; dbl_type = ft_lfloat;
		  Rvalue n( &dbl, sizeof(double), dbl_type, 1);
		  rval = n;
		}
		convert( rval, ft_lfloat, to );
		return 1;
	case convFTOD:			// float -> double
		dbl = *(float *)rval.raw();
		q = (unsigned char *)&dbl;
		break;
	case convDTOF:			// double -> float
		flt = *(double *)rval.raw();
		q = (unsigned char *)&flt;
		break;
	}

//DBG	if ( debugflag & DBG_ASSIGN )
//DBG		printe("convert() l = %#x, from_size = %d, to_size = %d\n",
//DBG				l, from_size, to_size);

//DBG	if ( debugflag & DBG_ASSIGN )
//DBG		printe("convert() rval.raw() = %#x, q = %#x, &l = %#x\n",
//DBG				rval.raw(), q, &l);

	Rvalue nrv( q, to_size, type, 1 );
	rval = nrv;
	return 1;
}

int
Expr::assign( Rvalue rval )
{
//DBG	if ( debugflag & DBG_ASSIGN )
//DBG		printe("Expr::assign()\n");

	Fund_type lhs_ft;
	Fund_type rhs_ft;
	Symbol lhs_ut;
	Symbol rhs_ut;

//DBG	dump( type, "lhs type" );
//DBG	dump( rval.type(), "rhs type" );

	if ( type.fund_type(lhs_ft) ) {		// LHS is fund_type
		;
	} else if ( type.user_type(lhs_ut) ) {	// LHS is user_type
		;
	} else {
		printe("Unknown type for %s\n", lab);
		return 0;
	}

	if ( rval.type().fund_type(rhs_ft) ) {		// RHS is fund_type
		;
	} else if ( rval.type().user_type(rhs_ut) ) {	// RHS is user_type
		;
	} else {
		printe("Unknown type for RHS\n");
		return 0;
	}

	// compare types and convert, if required

	if ( lhs_ut.isnull() ) {		// LHS is fundamental
		if ( rhs_ut.isnull() ) {	    // RHS is fundamental
			if ( lhs_ft != rhs_ft ) {
				if ( !convert(rval, rhs_ft, lhs_ft) ) {
					printe("type mismatch\n");
					return 0;
				}
			}
		} else {			    // RHS is user
			if ( type.size() == rval.type().size() ) {
				// special case, treat as equivalent
			} else {
				printe("type mismatch\n");
				return 0;
			}
		}
	} else {				// LHS is user
		if ( rhs_ut.isnull() ) {	    // RHS is fundamental
			if ( type.size() == rval.type().size() ) {
				// special case, treat as equivalent
			} else {
				printe("type mismatch\n");
				return 0;
			}
		} else {			    // both are user types
			if ( type.size() == 4 && rval.type().size() == 4 ) {
				// special case, treat as equivalent
			} else if ( lhs_ut != rhs_ut ) {
				printe("type mismatch\n");
				return 0;
			}
		}
	}

	// perform assignment

	if ( !base || !place ) {	// can't happen
		printe("Not an l-value\n");
		return 0;
	}

	int size = rval.size();
	if ( place->kind == pAddress ) {
		unsigned long buf;
		unsigned char *p = (unsigned char *)&buf;
		unsigned char *q = rval.raw();
		*p++ = *q++;	*p++ = *q++;	*p++ = *q++;	*p = *q;
//DBG		if ( debugflag & DBG_ASSIGN )
//DBG			printe("process->write(%#x, '%08x', %d)\n",
//DBG				place->addr, buf, size);
		int ret;
		if ( isbitfield ) {
			Itype it;
			ret = process->read(place->addr, Sint4, it);
			if ( ret <= 0 ) {
				printe("can't read 4 bytes from %#x\n",
					place->addr);
				return 0;
			}
			int o = it.iint4;
			int n;
			switch( size ) {
			case 4: n = *(int *)rval.raw();		break;
			case 2: n = *(short *)rval.raw();	break;
			case 1: n = *(char *)rval.raw();	break;
			}
//DBG			if ( debugflag & DBG_ASSIGN )
//DBG			    printe("o = %#x, n = %#x, bits = %d, bito = %d\n",
//DBG					o, n, bits, bito);
			int shift = 32 - (bits + bito);
			n <<= shift;
			n &=  (bitmask[bits] << shift);
			o &= ~(bitmask[bits] << shift);
			o |= n;
//DBG			if ( debugflag & DBG_ASSIGN )
//DBG			    printe("new n = %#x, new o = %#x\n", n, o);
			it.iint4 = o;
			size = 4;
			ret = process->write(place->addr, Sint4, it);
		} else {
			ret = process->write(place->addr, rval.raw(), size);
		}
		if ( ret <= 0 ) {
			printe("Can't write to object file or core file\n");
			return 0;
		} else if ( ret != size ) {
			printe("write() partially failed: %d out of %d bytes written to address %#x\n",
				ret, size, place->addr);
			return 0;
		}
	} else if ( place->kind == pRegister ) {
		RegAttrs *a = regattrs(place->reg);
		if ( !a ) {
			printe("unknown RegRef %d in assign()\n", place->reg);
			return 0;
		}
//DBG		if ( debugflag & DBG_ASSIGN )
//DBG			printe("assign(), reg %s, size %d\n",
//DBG				a->name, a->size);
		Stype stype = Suint4;
		Fund_type fund;
		type.fund_type(fund);
		if (fund == ft_lfloat)
			stype = Sdfloat;
		Itype itype;
		itype.rawbytes[0] = 0;		// shut up cfront
		if ( a->flags & FPREG ) {
			convert_to_fp( rval.raw(), size,
					&stype, itype.rawbytes );
		} else if ( a->size == size || fund == ft_lfloat) {
			memcpy(itype.rawbytes, rval.raw(), size);
		} else if ( a->size > size ) {	// must pad or extend
			int i;
			unsigned char *p = rval.raw();
			unsigned char fill = 0;	// MORE: sign extend?
#ifdef	RTOLBYTES
			for ( i = 0 ; i < size ; i++ )
				itype.rawbytes[i] = *p++;
			for ( ; i < 4 ; i++ )
				itype.rawbytes[i] = fill;
#else
			for ( i = 0 ; i < 4-size ; i++ )
				itype.rawbytes[i] = fill;
			for ( ; i < 4 ; i++ )
				itype.rawbytes[i] = *p++;
#endif
		} else {			// truncate from left
			if ( a->size != 4 ) {
				printe("register not 4 bytes!\n");
				return 0;
			}
#ifdef	RTOLBYTES
			memcpy(itype.rawbytes, rval.raw(), 4);
#else
			memcpy(itype.rawbytes, rval.raw()+(size-a->size), 4);
#endif
			size = 4;
		} 
		Frame *frame = symbolstream ? symbolstream->frame():0;
		if ( !frame ) frame = process->topframe();
//DBG		if ( debugflag & DBG_ASSIGN )
//DBG		    printe("frame->writereg(%s, '%08x', %d)\n",
//DBG			a->name, get_long(rval.raw(),4), size);
		int ret = frame->writereg(place->reg, stype, itype);
		if ( ret != 0 ) {
			printe("Can't write to register %s in core file\n",
					a->name);
			return 0;
		}
	} else {
		printe("bad place->kind (%d) in Expr::assign()\n",
				place->kind);
		return 0;
	}

	return 1;	// success
}
