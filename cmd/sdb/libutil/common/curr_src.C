#ident	"@(#)sdb:libutil/common/curr_src.C	1.2"
#include	"Interface.h"
#include	"Process.h"
#include	"utility.h"

char *
current_src( Process * process, long * line )
{
	if ( process == 0 )
	{
		printe("internal error: ");
		printe("null pointer to current_src()\n");
		return 0;
	}
	if ( line != 0 )
	{
		*line = process->current_line();
	}
	return process->current_srcfile;
}
