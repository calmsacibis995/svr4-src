#ident	"@(#)sdb:libutil/common/rel_proc.C	1.3"
#include	"utility.h"
#include	"oslevel.h"
#include	"Process.h"
#include	"Monitor.h"
#include	<osfcn.h>

int
release_process( Process * process, int run )
{
	if ( process == 0 )
	{
		return 0;
	}
	else if ( stop_proc( process->key, 0 ) == 0 )
	{
		return 0;
	}
	else if ( dispose_process( process ) == 0 )
	{
		return 0;
	}
	else if ( process->drop_et() == 0 )
	{
		return 0;
	}
	else if ( e_monitor.remove( process ) == 0 )
	{
		return 0;
	}
	else if ( run && (process->run( 0, 0, 0, 0 ) == 0) )
	{
		return 0;
	}
	else
	{
		delete process;
		return 1;
	}
}
