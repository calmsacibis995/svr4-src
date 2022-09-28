#ident	"@(#)sdb:libexecon/common/Monitor.s.C	1.4"
#include	"Monitor.h"
#include	"Process.h"
#include	"Status.h"
#include	"Interface.h"

int	check_processes = 0;

Monitor	e_monitor;

Monitor::Monitor():(1)	{}

Monitor::~Monitor()
{
	Status *	status;
	Status *	nxt;

	for ( status = next(); status != 0 ; status = nxt )
	{
		nxt = status->next();
		delete status;
	}
}

int
Monitor::add( Process * process )
{
	if ( process == 0 )
	{
		return 0;
	}
	else if ( process->status != 0 )
	{
		return 0;
	}
	else
	{
		process->status = new Status( process );
		prepend( process->status );
		return 1;
	}
}

int
Monitor::remove( Process * process )
{
	if ( process == 0 )
	{
		return 0;
	}
	else if ( process->status == 0 )
	{
		return 0;
	}
	else
	{
		delete process->status;
		return 1;
	}
}

int
Monitor::track( Process * process )
{
	if ( process == 0 )
	{
		return 0;
	}
	else if ( process->status == 0 )
	{
		return 0;
	}
	else
	{
		return process->status->track();
	}
}

int
Monitor::check_status()
{
	Status *	status;
	Status *	nxt;
	prstatus	prstat;

	check_processes = 0;
	for ( status = next(); status != 0; status = nxt )
	{
		nxt = status->next();
		if ( status->inform == 0 )
		{
			continue;
		}
		status->inform = 0;
		if ( status->get_status( prstat ) == 0 )
		{
			dispose_process( status->process );
			status->process->cleanup_et();
			printx("Process %d has terminated.\n",status->process->key.pid);
			delete status->process;
			delete status;
			return 1;
		}
		else if ( status->process->inform( prstat ) == 0 )
		{
			return 0;
		}
	}
	return 1;
}
