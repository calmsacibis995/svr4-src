#ident	"@(#)sdb:libexecon/i386/Memory.C	1.1"
#include	"Memory.h"
#include	"oslevel.h"
#include	<memory.h>

Memory::Memory()
{
	age = -1;
}

Memory::Memory( Key k, Iaddr lo, unsigned long size, long b, int s ):link(this)
{
	swapflags = s;
	access = k;
	loaddr = lo;
	hiaddr = ( size == 0 ) ? ~0 : loaddr + size;
	base = b;
}

int
Memory::read( Iaddr addr, void * buffer, int len )
{
	long	offset;

	offset = addr - loaddr + base;
	return ::get_bytes( access, offset, buffer, len );
}

int
Memory::write( Iaddr addr, void * buffer, int len )
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

int
Memory::read( Iaddr addr, Stype stype, Itype & itype )
{
	extern int debugflag; /*dbg*/
	long	offset;
	int	len,sz;
	char	buf;

	offset = addr - loaddr + base;
	sz = size[stype];
	switch ( stype )
	{
		case Sint1:
			len = ::get_bytes( access, offset, &buf, 1 );
			sz = 1;
			itype.iint1 = buf;
			if ( buf & 0x80 )
			{
				itype.iint1 |= ~0xff;
			}
			break;
		case Suchar:
			len = ::get_bytes( access, offset, &buf, 1 );
			sz = 1;
			itype.iuchar = buf;
			if ( buf & 0x80 )
			{
				itype.iuchar &= 0xff;
			}
			break;
		case Suint1:
			len = ::get_bytes( access, offset, &buf, 1 );
			sz = 1;
			itype.iuint1 = buf;
			if ( buf & 0x80 )
			{
				itype.iuint1 &= 0xff;
			}
			break;
		case Schar:
			len = ::get_bytes( access, offset, &buf, 1 );
			sz = 1;
			itype.ichar = buf;
			if ( buf & 0x80 )
			{
				itype.ichar |= ~0xff;
			}
			break;
		default:
			len = ::get_bytes( access, offset, &itype, sz );
	}
	if (debugflag) {
		printf("Memory:read: swapflag = %d size = %d\n",swapflags,sz); /*dbg*/
		printf("Memory:read: addr = 0x%x sz = %d len = %d\n",offset,size); /*dbg*/
	}
	if ( len != sz )
	{
		return len;
	}
	else if ( swapflags )
	{
		switch (stype)
		{
			case Sint2:
				itype.iint2 = swap2( itype.iint2 );
				break;
			case Suint2:
				itype.iuint2 = swap2( itype.iuint2 );
				break;
			case Sint4:
				itype.iint4 = swap4( itype.iint4 );
				break;
			case Suint4:
				itype.iuint4 = swap4( itype.iuint4 );
				break;
			case Saddr:
				itype.iaddr = swap4( itype.iaddr );
				break;
			case Sbase:
				itype.ibase = swap4( itype.ibase );
				break;
			case Soffset:
				itype.ioffset = swap4( itype.ioffset );
				break;
			default:
				break;
		}
		return len;
	}
	else
	{
		return len;
	}
}

int
Memory::write( Iaddr addr, Stype stype, const Itype & itype )
{
	long	offset;
	int	len,sz;
	Itype	outtype;

	offset = addr - loaddr + base;
	sz = size[stype];
	if ( swapflags )
	{
		switch (stype)
		{
			case Sint2:
				outtype.iint2 = swap2( itype.iint2 );
				break;
			case Suint2:
				outtype.iuint2 = swap2( itype.iuint2 );
				break;
			case Sint4:
				outtype.iint4 = swap4( itype.iint4 );
				break;
			case Suint4:
				outtype.iuint4 = swap4( itype.iuint4 );
				break;
			case Saddr:
				outtype.iaddr = swap4( itype.iaddr );
				break;
			case Sbase:
				outtype.ibase = swap4( itype.ibase );
				break;
			case Soffset:
				outtype.ioffset = swap4( itype.ioffset );
				break;
			default:
				::memcpy( (char*)&outtype, (char*)&itype, sz );
				break;
		}
	}
	else
	{
		::memcpy( (char*)&outtype, (char*)&itype, sz );
	}
	len = ::put_bytes( access, offset, &outtype, sz );
	return len;
}

Memory &
Memory::operator=( Memory & memory )
{
	access = memory.access;
	loaddr = memory.loaddr;
	hiaddr = memory.hiaddr;
	base = memory.base;
	swapflags = memory.swapflags;
	age = memory.age;
	return *this;
}
