#ident	"@(#)sdb:libexp/i386/Call2.C	1.5"

#include "Expr.h"
#include "Expr_priv.h"
#include <stdlib.h>
#include <string.h>

extern int debugflag;

extern sigset_t e_sigset;

Rvalue *package_itype( Stype stype, Itype *itype )
{
//DBG	if ( debugflag & (DBG_CALL | DBG_RVAL) )
//DBG		printe("package_itype( %d, '%08x' )\n", stype, *itype);
	return new Rvalue( stype, *itype );
}

void runnit( Process *process, Iaddr target, Execstate oldstate )
{
	set_signal_set( process, e_sigset );
	int ret = process->run( 1, target, 0, -1 );
			// clear_signal, to_addr, bpt_count, verbosity
	
	if ( !ret )
		printe("run() failed\n");
	else /*dbg*/
		printf("run passed\n\n"); /*dbg*/
	inform_processes();
	process->state = oldstate;
}

void writereg ( Process *process, RegRef which, Iaddr val )
{
	Itype ity;
	ity.iaddr = val;
	int ret = process->writereg( which, Saddr, ity);
	if ( !ret )
		printe("can't writereg() regref = %d, val = %#x\n", which, val);
}


Iaddr round ( Iaddr x, int align ) // round up to mult of align
{
	int d = x % align;

	if ( d )
		return x + (align - d);
	else
		return x; 
}

int need_stack( Rvalue *rval, SDBinfo_kind kind )
{
	TYPE type;
	type = rval->type();

	Symbol user;
	Fund_type ft;
	if ( type.user_type(user) ) {
//DBG		if ( debugflag & DBG_CALL )
//DBG			printf("need_stack() got user type, tag = %d\n", user.tag());
		switch ( user.tag() ) {
		case t_arraytype:
			return type.size();
		case t_pointertype:
			if ( kind == ikSTRING )
				return round(rval->size(), 4);
			else
				return 0;
		default:
			return 0;
		}
	} else if ( type.fund_type(ft) ) {
			// fundamental type, no extra stack needed
		type.fund_type(ft);
//DBG		if ( debugflag & DBG_CALL )
//DBG			printf("need_stack() got fund type %d\n", ft);
	} else {
		printe("null type in need_stack()!\n");
	}
	return 0;
}

int user_type( TYPE *type, Symbol *sym )
{
	return type->user_type( *sym );
}

void widen( Rvalue *rval, TYPE *type )
{
	Fund_type ft;

	if ( !type->fund_type(ft) )
		return;

	switch( ft ) {
	case ft_char:
	case ft_uchar:
	case ft_schar:
	case ft_short:
	case ft_ushort:
	case ft_sshort:
	case ft_int:
	case ft_uint:
	case ft_sint:
		convert( *rval, ft, ft_ulong );
		*type = ft_ulong;
		break;
	case ft_sfloat:
		convert( *rval, ft, ft_lfloat );
		*type = ft_lfloat;
		break;
	}
}

TYPE return_type( Symbol *func, char *fmt )
{
	TYPE ret_type;
	if ( func->type(ret_type, an_resulttype) ) {
//DBG		if ( debugflag & DBG_CALL )
//DBG			printe("an_resulttype succeeded\n");
	} else if ( func->type(ret_type, an_type) ) {
//DBG		if ( debugflag & DBG_CALL )
//DBG			printe("an_type succeeded\n");
	}
	if ( ret_type.isnull() ) {
		if ( fmt ) {	// we're going to use the result, warn user
			printe("can't determine return type of function, ");
			printe("assuming int\n");
		}
		ret_type = ft_int;
	}
	return ret_type;
}

double result()
{
	/* This function doesn't do anything, because the
	   value is in the top of the floating point stack
	*/
}

Rvalue *get_ret( Process *process, TYPE *ret_type )
{
	Rvalue *ret_val = 0;
	Fund_type ft;
	Symbol ut;
	Iaddr word;
	Itype itype;
	unsigned long *p;
	if ( ret_type->fund_type(ft) ) {
		switch( ft ) {
		default:
			word = process->getreg( REG_EAX );
			ret_val = new Rvalue( word );
			break;
		case ft_lfloat:
			itype.idfloat = result();
			ret_val = package_itype( Sdfloat, &itype );
			break;
		case ft_none:
		case ft_xfloat:
		case ft_scomplex:
		case ft_lcomplex:
		case ft_set:
		case ft_void:
			printe("bad return type, ft = %d\n", ft);
			break;
		}
	} else if ( ret_type->user_type(ut) ) {
		Tag tag = ut.tag();
		char *buf;
		int size = ret_type->size();
		switch( tag ) {
		case t_pointertype:
		case t_enumtype:
			word = process->getreg( REG_EAX );
			ret_val = new Rvalue( word );
			break;
		case t_structuretype:
		case t_uniontype:
			word = process->getreg( REG_EAX );
			buf = new char[ size ];
			if ( process->read(word,size,buf) == size ) {
				ret_val = new Rvalue( buf, size, *ret_type );
			} else {
				printe("couldn't read struct/union return value from address %#x\n", word);
			}
			break;
		case t_arraytype:
		case t_reftype:
		case t_functiontype:
		case t_enumlittype:
		case t_functargtype:
		case t_discsubrtype:
		case t_bitfieldtype:
		case t_stringtype:
		case t_bitfield:
		case t_typedef:
		case t_unspecargs:
		default:
			printe("bad return type, ut tag = %d\n", tag);
			break;
		}
	} else {
		printe("return type is null\n");
	}
	return ret_val;
}
