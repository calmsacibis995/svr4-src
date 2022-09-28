#ident	"@(#)sdb:libutil/common/set_sig.C	1.4"
#include	"prioctl.h"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
set_signal_set( Process * process, sigset_t sigset )
{
	if ( process == 0 )
	{
		printe("internal error: ");
		printe("process pointer was zero\n");
		return 0;
	}
	else
	{
		return process->set_sig_catch( sigset );
	}
}
