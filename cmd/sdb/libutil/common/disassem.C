#ident	"@(#)sdb:libutil/common/disassem.C	1.2"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
disassemble( Process * process, Location * location, int sm, unsigned long * x )
{
	Iaddr	addr;

	if ( process == 0 )
	{
		printe("internal error: ");
		printe("process pointer was zero\n");
		return 0;
	}
	else if ( get_addr( process, location, addr ) == 0 )
	{
		printe("could not disassemble specified location\n");
		return 0;
	}
	else
	{
		return process->disassemble( addr, sm, x );
	}
}
