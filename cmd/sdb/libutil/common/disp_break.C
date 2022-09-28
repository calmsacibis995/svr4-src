#ident	"@(#)sdb:libutil/common/disp_break.C	1.1"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
display_breaks( Process * process )
{
	if ( process == 0 )
	{
		printe("internal error : null Process pointer\n");
		return 0;
	}
	else
	{
		return process->display_bkpts();
	}
}
