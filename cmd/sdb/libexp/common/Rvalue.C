#ident	"@(#)sdb:libexp/common/Rvalue.C	1.13"

// Rvalue -- generic "R" values; objects which are the result of an
// expression evaluation.  Includes fundamental types, structs, unions,
// and arrays.

#include "Rvalue.h"
#include "Expr_priv.h"
#include "Type.h"
#include "Interface.h"
#include <memory.h>
#include "format.h"

static Fund_type
fund_type( Stype s )
{
	switch( s ) {
	default:
	case SINVALID:	return ft_none;
	case Schar:	return ft_char;
	case Sint1:	return ft_schar;
	case Sint2:	return ft_short;
	case Sint4:	return ft_int;
	case Suchar:	return ft_uchar;
	case Suint1:	return ft_uchar;
	case Suint2:	return ft_ushort;
	case Suint4:	return ft_uint;
	case Ssfloat:	return ft_sfloat;
	case Sdfloat:	return ft_lfloat;
	case Sxfloat:	return ft_xfloat;
	case Saddr:	return ft_ulong;
	case Sbase:	return ft_ulong;
	case Soffset:	return ft_ulong;
	}
}

Rvalue::Rvalue(Stype s, Itype& i)
{
//DBG	if ( debugflag & DBG_RVAL )
//DBG		printe("Rvalue::Rvalue(Stype %d, Itype '%08x')\n", 
//DBG					s, i.iaddr);
	_type = fund_type(s);
	raw_bytes.clear();
	stype = s;
	itype = i;
	rep = 1;
}

Rvalue::Rvalue(void *p, int len, TYPE& type, int repeat)
{
	unsigned long buf;
	unsigned char *q = (unsigned char *)&buf;
	unsigned char *r = (unsigned char *)p;
	q[0] = r[0];
	q[1] = r[1];
	q[2] = r[2];
	q[3] = r[3];
	//*q++ = *r++;	*q++ = *r++;	*q++ = *r++;	*q = *r;
//DBG	if ( debugflag & DBG_RVAL )
//DBG		printe("Rvalue::Rvalue('%08x', len %d, TYPE, %d)\n",
//DBG				buf, len, repeat);
	_type = type;
	raw_bytes.clear().add(p, len);
	stype = SINVALID;
	rep = repeat;
}

Rvalue::Rvalue(Iaddr addr)
{
//DBG	if ( debugflag & DBG_RVAL )
//DBG		printe("Rvalue::Rvalue(Iaddr %#x)\n", addr);
	_type = ft_ulong;
	raw_bytes.clear();
	stype = Saddr;
	itype.iaddr = addr;
	rep = 1;
}

Rvalue::Rvalue(Rvalue& rval)
{
	_type	  = rval._type;
	raw_bytes = rval.raw_bytes;	// deep copy
	stype 	  = rval.stype;
	itype	  = rval.itype;
	rep	  = rval.rep;
}

Rvalue::~Rvalue()
{
	// nothing to do
}

Rvalue&
Rvalue::operator=(Rvalue& rval)
{
	_type	  = rval._type;
	raw_bytes = rval.raw_bytes;	// deep copy
	stype	  = rval.stype;
	itype	  = rval.itype;
	rep	  = rval.rep;
	return *this;
}

static unsigned char convbuf[ sizeof(Itype) ];

static void itype2vec( Itype& itype, Stype stype, Vector &raw )
{
	register unsigned char *p = convbuf;
	register int size = 0;

	switch ( stype ) {
	case Sxfloat:
#if SIZEOF_XFLOAT == 16
							     size = 16;
		p[15] = itype.rawbytes[15];
		p[14] = itype.rawbytes[14];
		p[13] = itype.rawbytes[13];
		p[12] = itype.rawbytes[12];
#endif
		p[11] = itype.rawbytes[11];	if ( !size ) size = 12;
		p[10] = itype.rawbytes[10];
		p[9]  = itype.rawbytes[9];
		p[8]  = itype.rawbytes[8];
		// fall through...
	case Sdfloat:				if ( !size ) size = 8;
		p[7] = itype.rawbytes[7];
		p[6] = itype.rawbytes[6];
		p[5] = itype.rawbytes[5];
		p[4] = itype.rawbytes[4];
		// fall through...
	case Sint4:
	case Suint4:
	case Saddr:
	case Sbase:
	case Soffset:
	case Ssfloat:				if ( !size ) size = 4;
		p[3] = itype.rawbytes[3];
		p[2] = itype.rawbytes[2];
		// fall through...
	case Sint2:
	case Suint2:				if ( !size ) size = 2;
		p[1] = itype.rawbytes[1];
		// fall through...
	case Schar:
	case Suchar:
	case Sint1:
	case Suint1:				if ( !size ) size = 1;
		p[0] = itype.rawbytes[0];
		break;
	default:
	case SINVALID:		// necessary?
		printe("bad stype (%d) in itype2vec()\n", stype);
		break;
	}

//DBG	if ( debugflag & DBG_RVAL )
//DBG		printe("itype2vec(Itype '%08x', Stype %d, Vector) size = %d, vec = '%08x'\n",
//DBG				itype.iaddr, stype, size, *(long *)convbuf);

	raw.clear().add(convbuf, size);
}

int
Rvalue::operator==(Rvalue& rval)
{
	if ( _type.form() != rval._type.form() )
		return 0;
	if ( _type.form() == TF_fund ) {
		Fund_type f1, f2;
		_type.fund_type(f1);
		rval._type.fund_type(f2);
		if ( f1 != f2 )
			return 0;
	} else {
		Symbol s1, s2;
		_type.user_type(s1);
		rval._type.user_type(s2);
		if ( s1 != s2 )
			return 0;
	}
	if ( size() != rval.size() )
		return 0;

	return ! memcmp( raw(), rval.raw(), size() );
}

Stype
Rvalue::get_Itype(Itype& /*itype*/)
{
	return SINVALID;
}

int
Rvalue::print(char *label, char *fmt, char *sep)
{
//DBG	if ( debugflag & DBG_RVAL )
//DBG		printe("Rvalue::print('%s', '%s', '%s')\n", label, fmt, sep);

	if ( _type.isnull() ) {
		printe("null type in Rvalue::print()!\n");
		return 0;
	} else if ( size() == 0 ) {
		printe("rvalue size == 0!\n");
		return 0;
	} else {
		return format_bytes(raw_bytes, label, sep, fmt, _type);
	}

}


unsigned char *
Rvalue::raw()
{
	if ( raw_bytes.size() == 0 ) {
		itype2vec(itype, stype, raw_bytes);
	}
	return (unsigned char *)raw_bytes.ptr();
}

long
Rvalue::size()
{
	if ( raw_bytes.size() == 0 ) {
		itype2vec(itype, stype, raw_bytes);
	}
	return raw_bytes.size();
}
