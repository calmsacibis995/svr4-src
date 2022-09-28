#ident	"@(#)sdb:libutil/common/eval_addr.C	1.1"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
evaluate_addr( Process * process, char * estring, char * fmt )
{
	if ( process == 0 )
	{
		printe("internal error: ");
		printe("null process pointer to evaluate_addr()\n");
		return 0;
	}
	else if ( estring == 0 )
	{
		printe("internal error: ");
		printe("null expression string to evaluate_addr()\n");
		return 0;
	}
	else
	{
		return process->evaluate_addr( estring, fmt );
	}
}
