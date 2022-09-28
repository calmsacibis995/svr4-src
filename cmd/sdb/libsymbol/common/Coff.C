#ident	"@(#)sdb:libsymbol/common/Coff.C	1.2"
#include	"Coff.h"
#include	<fcntl.h>
#include	<memory.h>
#include	<osfcn.h>
#include	<string.h>
#include	<malloc.h>

#define equal(x,y)	(strcmp(x,y)==0)

Coff::Coff( int fdes ) : line_cache(fdes),symbol_cache(fdes),name_cache(fdes)
{
	fd = fdes;
	got_header = 0;
}

Coff::~Coff() {}

int
Coff::get_header()
{
	if ( got_header )
	{
		return 1;
	}
	else if ( ::lseek( fd, 0L, 0 ) == -1 )
	{
		return 0;
	}
	else if ( ::read( fd, (char*) &header, FILHSZ ) != FILHSZ )
	{
		return 0;
	}
	else if ( !ISCOFF(header.f_magic) )
	{
		return 0;
	}
	else
	{
		got_header = 1;
		return 1;
	}
}

long
Coff::first_symbol()
{
	if ( get_header() == 0 )
	{
		return 0;
	}
	else
	{
		sym_offset = header.f_symptr;
		str_offset = header.f_symptr+(header.f_nsyms*18);
	}
	return sym_offset;
}

int
Coff::sectno( char * sname )
{
	SCNHDR *	shdrp;
	int		i,scnt;
	long		size;
	Cache		header_cache( fd );

	if ( get_header() == 0 )
	{
		return 0;
	}
	size = header.f_nscns * sizeof(SCNHDR);
	shdrp = (SCNHDR*)header_cache.get( FILHSZ + header.f_opthdr, size );
	scnt = header.f_nscns;
	for (i = 0 ; i < scnt ; i++)
	{
		if ( equal( sname, shdrp->s_name ) )
		{
			return i + 1;
		}
		else
		{
			++shdrp;
		}
	}
	return 0;
}

long
Coff::line_info()
{
	SCNHDR *	shdrp;
	int		i,scnt;
	long		size;
	Cache		header_cache( fd );

	if ( got_line_offset )
	{
		return line_offset;
	}
	else if ( get_header() == 0 )
	{
		return 0;
	}
	line_offset = 0;
	size = header.f_nscns * sizeof(SCNHDR);
	shdrp = (SCNHDR*)header_cache.get( FILHSZ + header.f_opthdr, size );
	scnt = header.f_nscns;
	for (i = 0 ; i < scnt ; i++)
	{
		if ( equal( ".text", shdrp->s_name ) )
		{
			line_offset = shdrp->s_lnnoptr;
			break;
		}
		else
		{
			++shdrp;
		}
	}
	got_line_offset = 1;
	return line_offset;
}

long
Coff::get_symbol( long offset, struct syment & sym, union auxent & aux )
{
	char *	p;

	if (offset < sym_offset || offset >= str_offset)
	{
		return 0;
	}
	else if ( (p = (char *)symbol_cache.get( offset, 36 )) == 0 )
	{
		return 0;
	}
	::memcpy(  (char*)&sym, p, 18 );
	if ( sym.n_numaux != 0 )
	{
		::memcpy( (char*)&aux, p+18, 18 );
	}
	return offset + 18 + (sym.n_numaux * 18);
}

char *
Coff::get_name( struct syment sym )
{
	static char	buf[9];
	long		offset;

	if (sym.n_zeroes != 0)
	{
		::memcpy(buf,sym.n_name,8);
		buf[8] = '\0';
		return buf;
	}
	else
	{
		offset = sym.n_offset + str_offset;
		return name_cache.get_string( offset );
	}
}

char *
Coff::get_lineno( long offset, long count )
{
	if ( (offset == 0 ) || (offset >= sym_offset) )
	{
		return 0;
	}
	else
	{
		return (char*)line_cache.get( offset, count * LINESZ );
	}
}

int
Coff::cache( long offset, long length )
{
	return symbol_cache.fill( offset, length );
}

void
Coff::de_cache()
{
	symbol_cache.empty();
}
