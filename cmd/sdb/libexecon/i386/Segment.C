#ident	"@(#)sdb:libexecon/i386/Segment.C	1.3"
#include	"Object.h"
#include	"Segment.h"
#include	"oslevel.h"
#include	<fcntl.h>
#include	<string.h>
#include	<stdlib.h>
#include	<osfcn.h>
extern int debugflag;

Segment::Segment(Key k, char * s, Iaddr lo, long sz, long b, long ss_base, 
							int text):(0)
{
	int	len;

	access = k;
	pathname = 0;
	if ( s != 0 )
	{
		len = ::strlen(s) + 1;
		pathname = ::malloc( len );
		::strcpy( pathname, s );
	}
	loaddr = lo;
	hiaddr = loaddr + sz;
	base = b;
	sym.ss_base = ss_base;
	sym.symtable = 0;
	is_text = text;
}

int
Segment::read( Iaddr addr, void * buffer, int len )
{
	long	offset;

	offset = addr - loaddr + base;
	if (debugflag) { /*dbg*/
		printf("Segment::read addr = %#x loaddr = %#x base = %#x\n",
				addr, loaddr, base);
		printf("Segment::read offset = %#x\n",offset); /*dbg*/
	}
	return ::get_bytes( access, offset, buffer, len );
}

int
Segment::write( Iaddr addr, void * buffer, int len )
{
	long	offset;

	offset = addr - loaddr + base;
	return ::put_bytes( access, offset, buffer, len );
}

static int	size[] = {	0,
				sizeof(Ichar),
				sizeof(Iint1),
				sizeof(Iint2),
				sizeof(Iint4),
				sizeof(Iuchar),
				sizeof(Iuint1),
				sizeof(Iuint2),
				sizeof(Iuint4),
				sizeof(Isfloat),
				sizeof(Idfloat),
				sizeof(Ixfloat),
				sizeof(Iaddr),
				sizeof(Ibase),
				sizeof(Ioffset),
				0
			};

int
stype_size( Stype stype )
{
	return size[stype];
}

inline Iuint4
swap4( Iuint4 right)
{
	register char	*lhs, *rhs;
	Iuint4		left;

	rhs = (char*) &right;
	lhs = (char*) &left;
	lhs[0] = rhs[3];
	lhs[1] = rhs[2];
	lhs[2] = rhs[1];
	lhs[3] = rhs[0];
	return left;
}

inline Iuint2
swap2( Iuint2 right)
{
	register char	*lhs, *rhs;
	Iuint2		left;

	rhs = (char*) &right;
	lhs = (char*) &left;
	lhs[0] = rhs[1];
	lhs[1] = rhs[0];
	return left;
}

static void
swap( Stype stype, Itype & in, Itype & out )
{
	switch ( stype )
	{
		case Schar:	out.ichar = in.ichar;			break;
		case Suchar:	out.iuchar = in.iuchar;			break;
		case Sint1:	out.iint1 = in.iint1;			break;
		case Sint2:	out.iint2 = swap2( in.iint2 );		break;
		case Sint4:	out.iint4 = swap4( in.iint4 );		break;
		case Suint1:	out.iuint1 = in.iuint1;			break;
		case Suint2:	out.iuint2 = swap2( in.iuint2 );	break;
		case Suint4:	out.iuint4 = swap4( in.iuint4 );	break;
		case Saddr:	out.iaddr = swap4( in.iaddr );		break;
		case Ssfloat:	out.isfloat = in.isfloat;		break;
		case Sdfloat:	out.idfloat = in.idfloat;		break;
		case Sxfloat:	out.ixfloat = in.ixfloat;		break;
		case Sbase:	out.ibase = swap4( in.ibase );		break;
		case Soffset:	out.ioffset = swap4( in.ioffset );	break;
		case SINVALID:	break;	// shut up cfront
	}
}

int
Segment::read( Iaddr addr, Stype stype, Itype & itype )
{
	long	offset;
	int	len,sz;
	Itype	buftype;

	offset = addr - loaddr + base;
	sz = size[stype];
	if ( (len = ::get_bytes( access, offset, &buftype, sz )) != sz )
	{
		return len;
	}
	itype = buftype;
	return len;
}

int
Segment::write( Iaddr addr, Stype stype, const Itype & itype )
{
	long	offset;
	int	len;
	Itype	buftype;

	offset = addr - loaddr + base;
	buftype = itype;
	len = ::put_bytes( access, offset, &buftype, size[stype] );
	return len;
}

int
Segment::get_symtable( Key key )
{
	int	fd;

	if ( sym.symtable != 0 )
	{
		return 1;
	}
	else if ( pathname && *pathname )
	{
		fd = ::open( pathname, O_RDONLY );
		sym.symtable = ::get_symtable( fd );
		::close( fd );
		return 1;
	}
	else
	{
		fd = ::open_object( key, 0 );
		sym.symtable = ::get_symtable( fd );
		::close( fd );
		return 1;
	}
}
