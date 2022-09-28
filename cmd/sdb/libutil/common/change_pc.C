#ident	"@(#)sdb:libutil/common/change_pc.C	1.3"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
change_pc( Process * process, Location * location )
{
	Iaddr	addr;
	Itype	itype;

	if ( process == 0 )
	{
		printe("internal error: ");
		printe("process pointer was zero\n");
		return 0;
	}
	else if ( get_addr( process, location, addr ) == 0 )
	{
		return 0;
	}
	else
	{
		itype.iaddr = addr;
		return process->writereg( REG_PC, Saddr, itype );
	}
}
