#ident	"@(#)sdb:libsymbol/common/Cache.C	1.1"
#include	"Cache.h"
#include	<malloc.h>
#include	<osfcn.h>
#include	<string.h>

Cache::Cache( int filedes )
{
	fd = filedes;
	lo_cache = 0;
	hi_cache = -1;
	cache_ptr = 0;
	cache_size = 0;
}

Cache::~Cache()
{
	if ( cache_ptr != 0 )
	{
		::free( cache_ptr );
	}
}

int
Cache::fill( long offset, long size )
{
	int	x;

	if ( (offset >= lo_cache) && ( offset + size ) < hi_cache )
	{
		return 1;
	}
	if ( (size + 1) > cache_size )
	{
		if ( cache_ptr != 0 ) ::free( cache_ptr );
		cache_ptr = ::malloc( size + 1 );
	}
	if ( ::lseek( fd, offset, 0 ) == -1 )
	{
		if ( cache_ptr != 0 ) ::free( cache_ptr );
		cache_ptr = 0;
		cache_size = 0;
		return 0;
	}
	else if ( (x = ::read( fd, cache_ptr, size )) == 0 )
	{
		if ( cache_ptr != 0 ) ::free( cache_ptr );
		cache_ptr = 0;
		cache_size = 0;
		return 0;
	}
	else
	{
		lo_cache = offset;
		hi_cache = offset + x;
		cache_ptr[x] = '\0';
		cache_size = x + 1;
		return 1;
	}
}

void *
Cache::get( long offset, long length )
{
	if ( (lo_cache <= offset) && ((offset + length - 1) < hi_cache) )
	{
		return offset - lo_cache + cache_ptr;
	}
	else if ( fill( offset, length ) == 0 )
	{
		return 0;
	}
	else
	{
		return offset - lo_cache + cache_ptr;
	}
}

char *
Cache::get_string( long offset )
{
	char *	p;

	if ( (offset < lo_cache) || ( hi_cache <= offset) )
	{
		fill( offset, 2048 );
		return cache_ptr;
	}
	else if ( offset == lo_cache )
	{
		return cache_ptr;
	}
	p = cache_ptr + offset - lo_cache;
	if ( (offset - lo_cache + ::strlen(p)) >= 2048 )
	{
		fill( offset, 2048 );
		return cache_ptr;
	}
	else
	{
		return p;
	}
}

void
Cache::empty()
{
	if ( cache_ptr != 0 )
	{
		::free( cache_ptr );
	}
	cache_ptr = 0;
	lo_cache = 0;
	hi_cache = -1;
	cache_size = 0;
}

char *
Cache::ptr()
{
	return cache_ptr;
}
