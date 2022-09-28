#ident	"@(#)sdb:libutil/common/grabbed.C	1.1"
#include	"utility.h"
#include	"oslevel.h"
#include	"Process.h"
#include	"Interface.h"

int
grabbed( Process *process )
{
	if (process == 0)
	{
		printe("internal error : null Process pointer\n");
		return 0;
	}
	return process->grabbed;
}
