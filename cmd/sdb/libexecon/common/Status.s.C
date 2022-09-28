#ident	"@(#)sdb:libexecon/common/Status.s.C	1.2"
#include	"Status.h"
#include	"Process.h"
#include	"oslevel.h"
#include	<signal.h>

extern int	check_processes;

Status::Status( Process * p ) : (0)
{
	process = p;
	inform = 0;
	tracker_pid = -1;
}

Status::~Status()
{
	unlink();
}

int
Status::get_status( prstatus & prstat )
{
	return wait_for( process->key, prstat );
}

int
Status::track()
{
	inform = 1;
	check_processes = 1;
	return 1;
}

