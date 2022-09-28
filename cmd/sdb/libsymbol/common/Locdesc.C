#ident	"@(#)sdb:libsymbol/common/Locdesc.C	1.4"

#include	<memory.h>
#include	"Locdesc.h"
#include	"Process.h"
#include	"Frame.h"

enum LocOp	{
			loc_add,
			loc_deref4,
			loc_reg,
			loc_basereg,
			loc_offset,
			loc_addr,
		};

Locdesc &
Locdesc::clear()
{
	char	byte;

	byte = 0;
	vector.clear().add(&byte,1);
	return *this;
}

Locdesc &
Locdesc::add()
{
	char	byte;

	byte = loc_add;
	vector.add(&byte,1);
	return *this;
}

Locdesc &
Locdesc::deref4()
{
	char	byte;

	byte = loc_deref4;
	vector.add(&byte,1);
	return *this;
}

Locdesc &
Locdesc::reg( int r )
{
	char	byte;
	short	s;

	byte = loc_reg;
	s = (short)r;
	vector.add(&byte,1).add(&s,2);
	return *this;
}

Locdesc &
Locdesc::basereg( int r )
{
	char	byte;
	short	s;

	byte = loc_basereg;
	s = (short)r;
	vector.add(&byte,1).add(&s,2);
	return *this;
}

Locdesc &
Locdesc::offset( long l )
{
	char	byte;

	byte = loc_offset;
	vector.add(&byte,1).add(&l,4);
	return *this;
}

Locdesc &
Locdesc::addr( unsigned long l )
{
	char	byte;

	byte = loc_addr;
	vector.add(&byte,1).add(&l,4);
	return *this;
}

Addrexp
Locdesc::addrexp()
{
	if ( vector.size() > 0 )
	{
		(*(char*)vector.ptr()) = vector.size();
		return (Addrexp)vector.ptr();
	}
	else
	{
		return 0;
	}
}

Locdesc &
Locdesc::operator=( Addrexp aexp )
{
	int	len;

	vector.clear();
	if ( aexp != 0 )
	{
		len = *(char *)aexp;
		vector.add(aexp,len);
	}
	return *this;
}

Place
Locdesc::place( Process * process, Frame * frame )
{
	Place		lvalue;

	while ( stack.not_empty() ) stack.pop();
	if ( vector.size() == 0 )
	{
		lvalue.null();
	}
	else
	{
		calculate_expr( lvalue, process, frame );
	}
	return lvalue;
}

Place
Locdesc::place( Process * process, Frame * frame, Iaddr addr )
{
	Place		lvalue;

	while ( stack.not_empty() ) stack.pop();
	if ( vector.size() == 0 )
	{
		lvalue.null();
	}
	else
	{
		stack.push( addr );
		calculate_expr( lvalue, process, frame );
	}
	return lvalue;
}

void
Locdesc::calculate_expr( Place & lvalue, Process * process, Frame * frame )
{
	int		len;
	char *		p;
	PlaceMark	kind;
	short		regname;
	unsigned long	word;
	short		hwrd;

	if ( (len = *(char*)vector.ptr()) < 1 )
	{
		lvalue.null();
	}
	else
	{
		--len;
		kind = pAddress;
		p = (char*)vector.ptr() + 1;
		while ( len > 0 )
		{
			switch (*p)
			{
				case loc_add:
					stack.push( stack.pop() + stack.pop() );
					--len;
					++p;
					break;
				case loc_deref4:
					if ( process == 0 )
					{
						kind = pUnknown;
						len = 0;
					}
					else if ( process->read(stack.pop(),
							4,(char*)&word) != 4 )
					{
						kind = pUnknown;
						len = 0;
					}
					else
					{
						stack.push( word );
						--len;
						++p;
					}
					break;
				case loc_reg:
					kind = pRegister;
					++p;
					memcpy((char *)&regname, p,
					       sizeof(regname));
					p += 2;
					len -= 3;
					break;
				case loc_basereg:
					if ( (frame == 0) && (process == 0) )
					{
						kind = pUnknown;
						len = 0;
					}
					else if ( frame == 0 )
					{
						++p;
						memcpy((char *)&hwrd, p,
						       sizeof(hwrd));
						stack.push(process->curframe()->getreg(hwrd) );
						p += 2;
						len -= 3;
					}
					else
					{
						++p;
						memcpy((char *)&hwrd, p,
						       sizeof(hwrd));
						stack.push(frame->getreg(hwrd));
						p += 2;
						len -= 3;
					}
					break;
				case loc_addr:
				case loc_offset:
					++p;
					memcpy((char *)&word, p, sizeof(word));
					stack.push( word );
					p += 4;
					len -= 5;
					break;
				default:
					kind = pUnknown;
					len = 0;
			}
		}
		switch ( kind )
		{
			case pUnknown:
				lvalue.null();
				break;
			case pAddress:
				lvalue.kind = pAddress;
				lvalue.addr = stack.pop();
				break;
			case pRegister:
				lvalue.kind = pRegister;
				lvalue.reg = regname;
				break;
		}
	}
}

Locdesc &
Locdesc::adjust( unsigned long base )
{
	int		len;
	char *		p;
	unsigned long	word;

	if ( (len = *(char*)vector.ptr()) > 4 )
	{
		--len;
		p = (char*)vector.ptr() + 1;
		while ( len > 0 )
		{
			switch ( *p )
			{
				case loc_add:
				case loc_deref4:
					--len;
					++p;
					break;
				case loc_reg:
				case loc_basereg:
					p += 3;
					len -= 3;
					break;
				case loc_offset:
					p += 5;
					len -= 5;
					break;
				case loc_addr:
					++p;
					memcpy((char *)&word, p, sizeof(word));
					word += base;
					memcpy( p, (char *)&word, sizeof(word));
					p += 4;
					len -= 5;
					break;
				default:
					break;
			}
		}
	}
	return *this;
}

/*
int
Locdesc::basereg_offset( RegRef basereg, long& offset )
{
    // look for locdesc: basereg offset +
    // return 0 if this form is not found.
    // if it is found return 1 and set offset.
    // Warning: assumes basereg given before offset.

    // length op(basereg) op(address) op(+)
    //   1   +   3     +     5   +      1  = req_size
    const req_size = 10;

    if (vector.size() < req_size) return 0;  // not big enough.

    char *aexp = (char *)vector.ptr();
    int   len  = *aexp++;

    if (len != req_size) return 0; // wrong size.

    if (*aexp++ != loc_basereg) return 0; // 1st op wrong.
    short regnum;
    memcpy((char *)&regnum, aexp, sizeof(regnum)); aexp += sizeof(regnum);
    if (regnum != (int)basereg) return 0; // wrong base register.

    if (*aexp++ != loc_offset) return 0; // 2nd op wrong.
    memcpy((char *)&offset, aexp, sizeof(offset)); aexp += sizeof(offset);

    if (*aexp++ != loc_add) return 0;    // 3rd op wrong.

    return 1; // "basereg offset +" matched; offset set; ok.
}
*/
