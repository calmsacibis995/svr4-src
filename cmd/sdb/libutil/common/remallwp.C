#ident	"@(#)sdb:libutil/common/remallwp.C	1.1"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
remove_all_watch( Process * process, int ask )
{
	if ( process == 0 )
	{
		printe("internal error : null Process pointer\n");
		return 0;
	}
	else
	{
		return process->remove_all_wpts( ask );
	}
}
