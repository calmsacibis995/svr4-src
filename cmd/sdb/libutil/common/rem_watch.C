#ident	"@(#)sdb:libutil/common/rem_watch.C	1.1"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
rem_watch( Process * process, char * estring )
{
	if ( process == 0 )
	{
		printe("internal error: ");
		printe("process pointer was zero\n");
		return 0;
	}
	else if ( process->remove_wpt( estring ) == 0 )
	{
		printe("failure to remove watchpoint\n");
		return 0;
	}
	else
	{
		return 1;
	}
}
