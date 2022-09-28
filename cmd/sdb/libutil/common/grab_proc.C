#ident	"@(#)sdb:libutil/common/grab_proc.C	1.3"
#include	"utility.h"
#include	"oslevel.h"
#include	"Process.h"
#include	"EventTable.h"
#include	"Monitor.h"
#include	"Interface.h"
#include	<osfcn.h>

extern void parse_ps_args(char *);

Process *
grab_process( int pid, sigset_t sigset )
{
	Key		key;
	Process *	process;
	prstatus	status;
	char *		args;

	if ( ::get_key( pid, key ) == 0 )
	{
		process = 0;
	}
	else if ( stop_proc( key, &status ) == 0 )
	{
		process = 0;
	}
	else if ( (args = psargs( key )) == 0 )
	{
		process = 0;
	}
	else
	{
		parse_ps_args(args);
		process = new Process( key, 0 );
		current_process = process;
		process->set_sig_catch( sigset );
		e_monitor.add( process );
		e_monitor.track( process );
	}
	return process;
}
