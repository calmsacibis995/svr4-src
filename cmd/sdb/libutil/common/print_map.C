#ident	"@(#)sdb:libutil/common/print_map.C	1.1"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
print_map( Process * process )
{
	if ( process == 0 )
	{
		printe("internal error: ");
		printe("process pointer was zero\n");
		return 0;
	}
	else
	{
		return process->print_map();
	}
}
