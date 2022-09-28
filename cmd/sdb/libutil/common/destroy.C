#ident	"@(#)sdb:libutil/common/destroy.C	1.4"
#include	"utility.h"
#include	"oslevel.h"
#include	"Interface.h"
#include	"Process.h"
#include	"Monitor.h"
#include	<osfcn.h>

int
destroy_process( Process * process )
{
	if ( process == 0 )
	{
		return 0;
	}
	else if ( process->state == es_corefile )
	{
		return 1;
	}
	else if ( stop_proc( process->key ,0 ) == 0 )
	{
		printe("internal error: could not stop process\n");
		return 0;
	}
	else if ( dispose_process( process ) == 0 )
	{
		printe("No real process to destroy\n");
		return 0;
	}
	else if ( process->drop_et() == 0 )
	{
		printe("internal error: could not drop event table\n");
		return 0;
	}
	else if ( e_monitor.remove( process ) == 0 )
	{
		printe("internal error: could not remove process from monitor\n");
		return 0;
	}
	else if ( kill_proc( process->key ) == 0 )
	{
		printe("internal error: could not kill process\n");
		return 0;
	}
	else
	{
		if ( process->is_child() )
			wait(0);
		delete process;
		return 1;
	}
}
