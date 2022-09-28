#ident	"@(#)sdb:libutil/common/set_stmt.C	1.2"
#include	"Interface.h"
#include	"Process.h"
#include	"utility.h"

int
set_current_src( Process * process, char * filename, long line )
{
	if ( process == 0 )
	{
		printe("internal error: ");
		printe("null pointer to set_current_src()\n");
		return 0;
	}
	else
	{
		return process->set_current_stmt( filename, line );
	}
}
