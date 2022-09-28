#ident	"@(#)sdb:libsymbol/common/Bdi.C	1.5"
#include	"Bdi.h"
#include	<fcntl.h>
#include	<memory.h>
#include	<osfcn.h>
#include	<string.h>
#include	<malloc.h>
#include	"elf.h"

#define strequal(x,y)	(strcmp(x,y)==0)

Bdi::Bdi( int fildes ) : secthdr( fildes )
{
	fd = fildes;
	lo_cache = 0;
	hi_cache = -1;
	cache_ptr = 0;
	lo_entry = hi_entry = 0;
	lo_line = hi_line = 0;
	eoffset = 0;
	soffset = 0;
	::lseek(fd,0L,0);
	file_position = 0;
}

int
Bdi::readdata( long position, void * ptr, int length)
{
	register int	x;

	::errno = 0;
	if ( (lo_cache <= position) && ((position+length) <= hi_cache) )
	{
		::memcpy((char*)ptr, (position-lo_cache+cache_ptr), length);
		position += length;
		return length;
	}
	else if ( ( (file_position= ::lseek(fd,position,0)) == -1 ) )
	{
		return 0;
	}
	else if ( (x = ::read(fd, (char*)ptr, length )) == 0)
	{
		return x;
	}
	else
	{
		file_position += x;
		return x;
	}
}

Bdi::~Bdi()
{
	::close(fd);
	if ( cache_ptr != 0 )
		::free( cache_ptr );
}

int
Bdi::getsect( char * sectionname, long & base, long & size, long & offset )
{
	long	old_filepos;

	old_filepos = file_position;
	if ( secthdr.getsect( sectionname, base, size, offset ) == 0 )
	{
		file_position = ::lseek(fd,old_filepos,0);
		return 0;
	}
	else
	{
		file_position = ::lseek(fd,old_filepos,0);
		return 1;
	}
}

int
Bdi::cache( long offset, long size )
{
	int	x;

	if ( (offset < lo_cache) || ( (offset+size) > hi_cache ) )
	{
		if ( size > (hi_cache - lo_cache) )
		{
			if (cache_ptr != 0 ) ::free( cache_ptr );
			cache_ptr = ::malloc( size );
		}
		if ( (offset != file_position) &&
				( (file_position= ::lseek(fd,offset,0)) == -1 ) )
		{
			return 0;
		}
		else if ( size == 0 )
		{
			lo_cache = offset;
			hi_cache = offset;
			return 1;
		}
		else if ( (x = ::read(fd, cache_ptr, size )) == 0)
		{
			return 0;
		}
		else
		{
			file_position += x;
			lo_cache = offset;
			hi_cache = offset + x;
			return 1;
		}
	}
	return 1;
}

int
Bdi::entry_info( long & offset, long & base )
{
	long	size;

	if ( getsect( ".debug", base, size, offset ) == 0 )
	{
		return 0;
	}
	else
	{
		lo_entry = base;
		hi_entry = base + size;
		return 1;
	}
}

long
Bdi::entry_offset()
{
	long		base, size;

	if ( eoffset != 0 )
	{
		return eoffset;
	}
	else if ( getsect( ".debug", base, size, eoffset ) == 0 )
	{
		return 0;
	}
	else
	{
		lo_entry = eoffset;
		hi_entry = eoffset + size;
		return eoffset;
	}
}

int
Bdi::stmt_info( long & offset, long & base )
{
	long	size;

	if ( getsect( ".line", base, size, offset ) == 0 )
	{
		return 0;
	}
	else
	{
		lo_line = offset;
		hi_line = offset + size;
		return 1;
	}
}

long
Bdi::stmt_offset()
{
	long		base, size;

	if ( soffset != 0 )
	{
		return soffset;
	}
	else if ( getsect( ".line", base, size, soffset ) == 0 )
	{
		return 0;
	}
	else
	{
		lo_line = soffset;
		hi_line = soffset + size;
		return soffset;
	}
}

char *
Bdi::get_entry( long offset )
{
	long	reclen;

	if ( offset < entry_offset() )
	{
		return 0;
	}
	else if ( offset >= hi_entry )
	{
		return 0;
	}
	else if ( readdata(offset, &reclen, sizeof(long)) != sizeof(long) )
	{
		return 0;
	}
	else if ( (offset+reclen) > hi_entry )
	{
		return 0;
	}
	else if ( cache( offset, reclen ) == 0 )
	{
		return 0;
	}
	else
	{
		return offset - lo_cache + cache_ptr;
	}
}

char *
Bdi::get_lineno( long offset )
{
	long	sblen;

	if ( offset < stmt_offset() )
	{
		return 0;
	}
	else if ( offset >= hi_line )
	{
		return 0;
	}
	else if ( readdata(offset, &sblen, sizeof(long)) != sizeof(long) )
	{
		return 0;
	}
	else if ( (offset+sblen) > hi_line )
	{
		return 0;
	}
	else if ( cache( offset, sblen ) == 0 )
	{
		return 0;
	}
	else
	{
		lo_cache = 0;
		hi_cache = -1;
		return cache_ptr;
	}
}
