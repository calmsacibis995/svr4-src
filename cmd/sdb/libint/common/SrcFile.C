#ident	"@(#)sdb:libint/common/SrcFile.C	1.10"
#include	"SrcFile.h"
#include	<osfcn.h>
#include	<fcntl.h>
#include	<string.h>
#include	<memory.h>
#include	<sys/stat.h>

#if defined(__STDC__) || defined (__cplusplus)
static void check_newer(char *fnpath);
#else
static void check_newer();
#endif

SrcFile::SrcFile( int fd )
{
	long	number = 0;

	fptr = ::fdopen( fd, "r" );
	vector.add( &number, sizeof( long ) );
	vector.add( &number, sizeof( long ) );
	hi = 1;
	not_last = 1;
}

SrcFile::~SrcFile()
{
	::fclose( fptr );
}

extern char *	global_path;
extern int	pathage;

static SrcFile *
open_srcfile( char * name )
{
	SrcFile *	file;
	char *		x;
	char *		p;
	int		len, pathlen;
	static char	buf[200];
	int		fd;
	extern int	Wflag;

	file = 0;
	if ( (fd = ::open( name, O_RDONLY)) != -1 )
	{
		file = new SrcFile( fd );
		if ( !Wflag )
			check_newer(name);
		return file;
	}
	if( global_path == 0 )
	{
		global_path=new char[2];
		global_path[0]='.';
		global_path[1]='\0';
	}
	pathlen = ::strlen( global_path );
	x = global_path;
	while ( pathlen > 0 )
	{
		len = ::strcspn( x, ":" );
		::memcpy( buf, x, len );
		buf[len] = '/';
		buf[len+1] = '\0';
		::strcat( buf, name );
		if ( (fd = ::open( buf, O_RDONLY)) != -1 )
		{
			file = new SrcFile( fd );
			if ( !Wflag )
				check_newer(buf);
			return file;
		}
		else
		{
			x += ( len + 1 );
			pathlen -= ( len + 1 );
		}
	}
	if ( (p = ::strrchr( name, '/' )) == 0 )
	{
		return 0;
	}
	pathlen = ::strlen( global_path );
	x = global_path;
	while ( pathlen > 0 )
	{
		len = ::strcspn( x, ":" );
		::memcpy( buf, x, len );
		buf[len] = '/';
		buf[len+1] = '\0';
		::strcat( buf, p );
		if ( (fd = ::open( buf, O_RDONLY)) != -1 )
		{
			file = new SrcFile( fd );
			if ( !Wflag )
				check_newer(buf);
			return file;
		}
		else
		{
			x += ( len + 1 );
			pathlen -= ( len + 1 );
		}
	}
	return file;
}

static void
check_newer(char *fnpath)
{
	typedef struct	newer {
		ino_t		fino;
		dev_t		fent;
		struct newer	*next;
	} newer;
	static newer	*newerhead = (newer *)0;

	struct stat 	stbuf;
	extern char 	*symfil;
	extern time_t	SymFilTime;

	if (fnpath==0)
		return;

	if (fnpath[0] == '\0' || stat(fnpath,&stbuf) == -1) {
		printf("cannot access `%s'\n", fnpath);
		return;
	}
	if (stbuf.st_mtime > SymFilTime) {
		for( newer* ptr = newerhead; ptr != (newer *)0; ptr = ptr->next) {
			if (ptr->fent == stbuf.st_dev && ptr->fino == stbuf.st_ino)
				break;
		}
		if (ptr == (newer *)0) {
			newer *newerfil = new newer[1];

			printf("Warning: `%s' newer than `%s'\n",fnpath,symfil);
			newerfil->fino = stbuf.st_ino;
			newerfil->fent = stbuf.st_dev;
			newerfil->next = newerhead;
			newerhead = newerfil;
		}
	}
	return;
}

#define NFILES  2	/* later 10, bkr */

static char *		fpath[NFILES];

static SrcFile *	ftab[NFILES];

static int		age[NFILES];

static int		nextslot = 0;

SrcFile *
find_srcfile( char * path )
{
	SrcFile *	file;

	if ( path == 0 )
	{
		return 0;
	}

	for ( register int i = 0 ; i < NFILES ; i++ ) {
		if ( fpath[i] && !::strcmp( path, fpath[i]) ) {
			file = ftab[i];
			if ( (file == 0) || (age[i] != pathage) )
			{
				file = open_srcfile( path );
				if ( file != 0 )
					file->name = fpath[i];
				age[i] = pathage;
			}
			return file;
		}
	}

	if ( fpath[nextslot] ) delete fpath[nextslot];
	fpath[nextslot] = new char [ ::strlen(path) + 1 ];
	::strcpy( fpath[nextslot], path );

	if ( ftab[nextslot] ) delete ftab[nextslot];
	ftab[nextslot] = open_srcfile( path );
	age[nextslot] = pathage;

	file = ftab[nextslot];
	if ( file != 0 )
		file->name = fpath[nextslot];

	if ( ++nextslot >= NFILES ) nextslot = 0;

	return file;
}

#define SBSIZE	513

static char	buf[SBSIZE];

char *
SrcFile::line( int num )
{
	long *	array;
	long	offset;

	if ( num <= 0 )
	{
		return 0;
	}
	array = (long*) vector.ptr();
	offset = array[hi];
	while ( num > hi )
	{
		if ( ::fseek( fptr, offset, 0 ) != 0 )
		{
			return 0;
		}
		else if ( ::fgets( buf, SBSIZE, fptr ) == 0 )
		{
			return 0;
		}
		else
		{
			offset = ::ftell( fptr );
			vector.add( &offset, sizeof( long ) );
		}
		++hi;
	}
	array = (long*) vector.ptr();
	offset = array[num];
	if ( ::fseek( fptr, offset, 0 ) != 0 )
	{
		return 0;
	}
	else if ( ::fgets( buf, SBSIZE, fptr ) == 0 )
	{
		return 0;
	}
	else
	{
		return buf;
	}
}

long
SrcFile::num_lines()
{
	long *	array;
	long	offset;

	array = (long*) vector.ptr();
	offset = array[hi];
	while ( not_last )
	{
		if ( ::fseek( fptr, offset, 0 ) != 0 )
		{
			return 0;
		}
		else if ( ::fgets( buf, SBSIZE, fptr ) == 0 )
		{
			not_last = 0;
			--hi;
		}
		else
		{
			offset = ::ftell( fptr );
			vector.add( &offset, sizeof( long ) );
			++hi;
		}
	}
	return hi;
}

char *
SrcFile::filename()
{
	return name;
}
