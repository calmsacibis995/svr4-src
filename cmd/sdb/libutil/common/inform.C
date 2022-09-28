#ident	"@(#)sdb:libutil/common/inform.C	1.3"
#include	"Monitor.h"
#include	"utility.h"

extern int	check_processes;
extern int	do_assoccmds();		// provided by user interface.

int
inform_processes()
{
	do_assoccmds();
	while ( check_processes )
	{
		check_processes = 0;
		e_monitor.check_status();
		do_assoccmds();
		interrupted = 0;
	}
	interrupted = 0;
	return 1;
}
