#ident	"@(#)sdb:libexp/i386/Call.C	1.7.1.1"

#include "Expr.h"
#include "Expr_priv.h"
#include "SDBinfo.h"
#include "Type.h"
#include "Itype.h"
#include "Tag.h"
#include "Reg.h"
#include "Process.h"
#include "utility.h"
#include "Interface.h"
#include "format.h"
#include "Reg.h"
#include <stdlib.h>
#include <string.h>

overload dump;

extern void dump( Symbol sym, char *label = 0 );

extern void dump( TYPE type, char *label = 0 );

extern int debugflag;

static char _call = 0xe8;
#define CALL	&_call

#define SAVESIZE	1   +  4
		   /* CALL    < pc relative address > */

int
Expr::do_call( char *fmt, Process *proc, Iaddr pc, Frame *frm )
{
//DBG	if ( debugflag & DBG_CALL )
//DBG		printe("do_call( '%s' ) fcn = '%s', this = %#x\n",
//DBG				fmt, etree->call.fcn_name, this);

	process = proc ? proc : current_process;
	frame = process ? process->topframe() : 0;

	if ( !process || !frame || process->is_proto() ||
			process->state == es_corefile ) {
		printe("not a live process\n");
		return 0;
	}

// Find address of _start so we can patch in CALL instruction.
// Do this before modifying any registers, so we can avoid having to
// undo our modifications.

	Iaddr start = 0;

	Symbol sym = process->find_global("_start");
	if ( sym.isnull() ) {
		printe("can't locate _start()!\n");
		return 0;
	} else {
		start = sym.pc( an_lopc );
	}


// Save registers we are about to modify.

	Execstate oldstate = process->state;

	Iaddr oldsp = process->getreg( REG_ESP );
	Iaddr oldap = process->getreg( REG_EBP );
	Iaddr oldpc = process->getreg( REG_EIP );
	Iaddr oldr0 = process->getreg( REG_EAX );

//	Iaddr oldr1 = process->getreg( REG_R1 );
//	Iaddr oldr2 = process->getreg( REG_R2 );

	Iaddr cursp = oldsp;

// Evaluate all the args and save the Rvalues, pushing temporary
// space for strings, arrays, and returned structs.

	Vector rvec;

	int argno = 0;

	rvec.clear();

	SDBinfo *arg = etree->next_arg;
//
// Reverse the argument list, because the arguments are pushed on
// on the stack in the reverse order.
//
	SDBinfo *x, *y;
	if ( arg ) {
		x = arg;
		y = arg->next_arg;
		x->next_arg = 0;
		while ( y ) {
			x = y;
			y = y->next_arg;
			x->next_arg = arg;
			arg = x;
		}
	}

//
// Now arg points to the reversed list.
//

	while ( arg ) {
		argno++;
//DBG		if ( debugflag & DBG_CALL ) {
//DBG			printe("arg %d = ", argno);
//DBG			dump_SDBinfo( arg );
//DBG		}
		Expr e(arg);
		if ( !e.eval(EV_RHS, process, pc, frm) ) {
			printe("can't evaluate argument %d\n",
					argno);
			return 0;
		}
		widen( e.rval, &e.type );
		int need = need_stack(e.rval, e.etree->kind);
		if ( need ) {
			int size = e.rval->size();	// not necessarily == need
			cursp -= need;
			if ( process->write(cursp, e.rval->raw(), size) != size ) {
				printe("can't write stack for temp, addr %#x, size %d\n",					cursp, size);
				cursp += need;
				return 0;
			}
			// change rval from X to (u_long)&X (address is now on stack)
			delete e.rval;
			e.rval = new Rvalue( cursp );
		}
		rvec.add(&e.rval, 4);
//DBG		if ( debugflag & DBG_CALL )
//DBG			e.rval->print(sf("arg %d",argno), 0, "/");
		e.rval = 0;		// so ~Expr won't delete it;
		arg = arg->next_arg;
	}

// Now resolve the function name (no wildcards).

	Symbol func = process->find_global( etree->call.fcn_name );
	if ( func.isnull() ) {
		// MORE: may be a static function
		printe("%s not found\n", etree->call.fcn_name );
		return 0;
	}


// Save space on the stack for return value if func returns struct or union.

	TYPE ret_type = return_type( &func, fmt );
	Symbol ut;
	if ( user_type(&ret_type, &ut) ) {
		if ( ut.tag() == t_structuretype || ut.tag() == t_uniontype ) {
//
//	change for 386 later on
//
			cursp -= ret_type.size();
			writereg( process, REG_EAX, cursp );	// hidden arg
		}
	}

// "push" args


	Symbol func_arg = func.child();
	Fund_type ft;
	Attribute *attr;
	TYPE type;
	Tag tag;
	int fun_narg;
	typedef struct fun_arg_type {
		Fund_type arg_type;
		struct fun_arg_type *next;
	} fun_arg_type;
	fun_arg_type *arg_flag = (fun_arg_type *)0;
	fun_arg_type *arg_tmp;
	fun_narg = 0;
	for ( int narg = 0 ; narg < argno ; narg++ ) {
		tag = t_none;
		attr = func_arg.attribute(an_tag);
		if ( attr && attr->form== af_tag)
			tag = attr->value.word;

		if (!func_arg.isnull() && tag == t_argument) {
			fun_narg ++;
			func_arg.type(type, an_type);
			fun_arg_type *sfloat_node = new fun_arg_type[1];
			type.fund_type(ft);
			sfloat_node->arg_type = ft;
			sfloat_node->next = arg_flag;
			arg_flag= sfloat_node;
			func_arg = func_arg.sibling();
		}

	}

	int ret, size;
	char arg_string[16];
	Iaddr argptr = cursp;
	Rvalue **rvalp = (Rvalue **)rvec.ptr();
	Rvalue *rval;
	double arg_double;
	float arg_single;
	unsigned long arg_long;
	for ( narg = argno ; narg ; --narg,++rvalp ) {
		rval = *rvalp;
		size = (int)rval->size();
		::memcpy(arg_string, rval->raw(), size);

		if ((arg_tmp = arg_flag) != 0 && narg <= fun_narg) {
			rval->type().fund_type(ft);
			switch (ft) {
			case ft_ulong:
				::memcpy( &arg_long, rval->raw(), size );
				switch (arg_tmp->arg_type) {
				case ft_lfloat:
					arg_double = (double)arg_long;
					::memcpy(arg_string, &arg_double, size=8);
					break;
				case ft_sfloat:
					arg_single = (float)arg_long;
					::memcpy(arg_string, &arg_single, size=4);
					break;
				}
				break;
			case ft_lfloat:
				::memcpy( &arg_double, rval->raw(), size );
				switch (arg_tmp->arg_type) {
				case ft_lfloat:	break;
				case ft_sfloat:
					arg_single = (float)arg_double;
					::memcpy(arg_string, &arg_single, size=4);
					break;
				default:
					arg_long = (long)arg_double;
					::memcpy(arg_string, &arg_long, size=4);
					break;
				}
				break;
			}
			arg_flag = arg_flag->next;
			delete arg_tmp;
		}
		cursp -= size;
		ret = process->write( cursp, arg_string, size);
		if ( ret != size ) {
			printe("can't write arg %d to stack, addr %#x, size %d, ret = %d\n",
				argno - narg + 1, cursp, size, ret);
			cursp += size;
		}
		cursp = round(cursp, 4);
	}

//
// Patch  _start with CALL instruction
//

	Iaddr funcaddr = func.pc( an_lopc );
	char savebuf[ SAVESIZE ];

	if ( process->read( start, SAVESIZE, savebuf ) != SAVESIZE ) {
		printe("couldn't read %d bytes at _start (%#x)\n",
				SAVESIZE, start);
		return 0;
	}

	Vector instr;

	Itype tvl;
	Iint4  offset;
	
	if ( process->write( start, &_call, 1) != 1 ) {
		printe("couldn't write 0xe8 to _start (%#x)\n", start);
		return 0;
	}
	
	tvl.iaddr = funcaddr;
	offset =  start + 5;
	tvl.iaddr = tvl.iaddr - offset;
	unsigned char addr_buf[4];
	unsigned char *addrptr = addr_buf;
	Iaddr start_1 = tvl.iaddr;

	*addrptr++ = start_1;
	*addrptr++ = start_1 >> 8;
	*addrptr++ = start_1 >> 16;
	*addrptr = start_1 >> 24;
	addrptr = addr_buf;

	
	if ( process->write( start + 1 , addrptr, 4) != 4 ) {
		printe("couldn't write four bytes to _start+1 (%#x)\n", start);
		return 0;
	}
	

// adjust registers

	writereg( process, REG_ESP, cursp );
	writereg( process, REG_EIP , start );



// call the function

	runnit( process, start+SAVESIZE, oldstate );

// if requested, fetch and print the result

	if ( fmt ) {
		Rvalue *retval = get_ret ( process, &ret_type );
		if ( retval ) {
			// transmogrify this Expr into the return value
			if ( own_tree ) {
				delete etree;
			}
			etree = SDBinfo_name( 1, 0, func.name() );
			own_tree = 1;
			current_sym = func;
			lab = sf("%s()",func.name());
			returnval = retval;
			type = ret_type;
			mode = sPRINT_RESULT;
			frame = process->topframe();
			decomp = new Decomp( etree, &type, 1);
			base = new Place;
			base->kind = pAddress;
			base->addr = (Iaddr)retval->raw();
			//     ^ this address is in SDB's memory, not in
			//     the subject process! see Expr::get_rvalue()
			while ( evaluate( fmt ) )
				;
		}
	}

// restore saved code and registers

	ret = process->write( start, savebuf, SAVESIZE );
	if ( ret != SAVESIZE )
		printe("couldn't restore code at _start %#x, ret = %d\n", start, ret);

	writereg( process, REG_ESP, oldsp );
	writereg( process, REG_EBP, oldap );
	writereg( process, REG_EIP, oldpc );
	writereg( process, REG_EAX, oldr0 );
//	writereg( process, REG_R1, oldr1 );
//	writereg( process, REG_R2, oldr2 );

	process->state = oldstate;

	return 1;
}
