#ident	"@(#)sdb:libutil/common/grab_core.C	1.7"
#include	"utility.h"
#include	"oslevel.h"
#include	"Process.h"
#include	"Interface.h"
#include	"SectHdr.h"
#include	<fcntl.h>
#include	<osfcn.h>
#include	<string.h>

extern char	executable_name[];
Process *	last_core;

Process *
grab_core( char *adotout, char *core, int mode, sigset_t sigset, int flatmap )
{
	int		textfd, corefd;
	Process *	process;
	int		pid;
	char *		s;

	corefd = -1;

	if ( (textfd = ::open(adotout,mode)) == -1 )
	{
		printe("no executable '%s' exists\n",adotout);
		return 0;
	}
	else if ( (core != 0) && ((corefd = ::open(core,mode)) == -1 ) )
	{
		::close( textfd );
		printe("no core file '%s' exists\n",core);
		return 0;
	}

	SectHdr		secthdr( textfd );

	if ( !live_proc( textfd ) )
	{
		if ( !secthdr.has_debug_info() )
		{
			printe("Warning: no -g information in %s\n",adotout);
		}
		::strcpy( executable_name, adotout );
		process = new Process( textfd, corefd, flatmap );
		last_core = process;
	}
	else if ( corefd != -1 )
	{
		::close( textfd );
		::close( corefd );
		printe("grabbing live process %s, ignoring core file %s\n",
			adotout,core);
		s = ::strrchr( adotout, '/' );
		pid = atoi( s + 1 );
		process = grab_process( pid, sigset );
	}
	else
	{
		::close( textfd );
		s = ::strrchr( adotout, '/' );
		pid = atoi( s + 1 );
		process = grab_process( pid, sigset );
	}

	current_process = process;
	return process;
}
