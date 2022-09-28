#ident	"@(#)sdb:libutil/common/set_path.C	1.4"

#include	<string.h>

char *	global_path = 0;
int	pathage = 0;

int
set_path( char * path )
{
	if ( global_path != 0 ) delete global_path;
	global_path = new char[ ::strlen( path ) + 1 ];
	::strcpy( global_path, path );
	++pathage;
	return 1;
}
