#ident	"@(#)sdb:libsymbol/common/TYPE.C	1.5"

#include <assert.h>
#include <string.h>
#include "Type.h"
#include "TYPE.h"
#include "Interface.h"
#include "Itype.h"
#include "Tag.h"

void
TYPE::null()
{
    _form = TF_fund;
    ft    = ft_none;
}

int
TYPE::isnull()
{
    return _form == TF_fund && ft == ft_none;
}

TYPE::TYPE(TYPE& t)
{
    _form   = t._form;
    ft      = t.ft;
    symbol  = t.symbol;
}

TYPE&
TYPE::operator=(TYPE& t)
{
    _form   = t._form;
    ft      = t.ft;
    symbol  = t.symbol;
    return *this;
}

TYPE&
TYPE::operator=(Fund_type ftype)
{
    _form = TF_fund;
    ft    = ftype;
    return *this;
}

TYPE&
TYPE::operator=(Symbol& symb)
{
    _form   = TF_user;
    symbol  = symb;
    return *this;
}

int
TYPE::fund_type(Fund_type& ftype)   // 'read' fundamental type.
{
    if (_form == TF_fund) {
	ftype = ft;
	return 1;
    }
    ftype = ft_none;
    return 0;
}

int
TYPE::user_type(Symbol& symb) // 'read' user defined type.
{
    if (_form == TF_user) {
	symb = symbol;
	return 1;
    }
    symb.null();
    return 0;
}

int
TYPE::size()
{
    if (isnull()) return 0; // don't know.

    if (form() == TF_user) {
	Attribute *a = symbol.attribute(an_bytesize);

	if (a != 0 && a->form == af_int) {
	    return a->value.word;
	}

	TYPE elemtype;
	symbol.type(elemtype, an_elemtype);
	if (!elemtype.isnull()) {
	    int span = elemtype.size();
	    a = symbol.attribute(an_lobound);
	    if (a != 0 && a->form == af_int) {
		int lo = a->value.word;
		a = symbol.attribute(an_hibound);
		if (a != 0 && a->form == af_int) {
			return span * (a->value.word - lo + 1);
		}
	    }
	}

	return 0;
    }
    assert(form() == TF_fund);

    switch (ft) { //MORE: make TYPE::size() <fund> properly machine dependent.
    case ft_char:
    case ft_schar:
    case ft_uchar:
	return 1;

    case ft_short:
    case ft_sshort:
    case ft_ushort:
	return 2;

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
	return 8;

    case ft_xfloat:
#ifdef u3b2
	return 12;
#endif
#ifdef sparc
	return 16;
#endif

    case ft_none:
    default:
//	printe("TYPE::size(), unknown Fund_type: %d, returning 4.\n", ft);
	return 0;
    }
}

int
TYPE::get_Stype(Stype& stype)
{
    switch (_form) {
    case TF_user:
	if (symbol.tag("TYPE::get_Stype()") == t_pointertype) {

	    stype = Saddr;	// for   register T *p;
				// ...   eval p
	}
	break;
    case TF_fund:
	switch (ft) {
	case ft_char:    stype = Schar;   break;  //MORE: machine dependent.
	case ft_schar:   stype = Schar;   break;
	case ft_uchar:   stype = Suchar;  break;
	case ft_short:  stype = Sint2;   break;
	case ft_sshort:  stype = Sint2;   break;
	case ft_ushort:  stype = Suint2;  break;
	case ft_int:    stype = Suint4;  break;
	case ft_sint:    stype = Suint4;  break;
	case ft_uint:    stype = Sint4;   break;
	case ft_long:   stype = Sint4;   break;
	case ft_slong:   stype = Sint4;   break;
	case ft_ulong:   stype = Suint4;  break;
	case ft_pointer: stype = Saddr;   break;
	case ft_sfloat:  stype = Ssfloat; break;
	case ft_lfloat:  stype = Sdfloat; break;
	case ft_xfloat:  stype = Sdfloat; break; //MORE: WRONG.
	case ft_void: stype = Saddr;	  break;
	case ft_none:
	default:
	    stype = SINVALID;
	    return 0;
	}
	break;
    default:
	stype = SINVALID;
	return 0;
    }
    return 1;
}

int
TYPE::is_ptr()	//MORE? also return true for ft_pointer (void *)
{
    return form() == TF_user && symbol.tag("TYPE::is_ptr()") == t_pointertype;
}

int
TYPE::deref_type(TYPE& dtype, Tag *tagp)	// ptr or 1 dim array.
{
    if (form() != TF_user) return 0;

    Tag       tag   = symbol.tag("TYPE::deref_type(TYPE&)");
    Attr_name attr;

    if (tagp) *tagp = tag;

    switch (tag) {
    case t_pointertype:
	attr = an_basetype;
	break;
    case t_arraytype:
	attr = an_elemtype;
	//MORE? > 1 dim array ?
	break;
    default:
	return 0;
    }
    return symbol.type(dtype, attr);
}
