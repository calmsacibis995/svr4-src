#ident	"@(#)sdb:libutil/common/num_lines.C	1.1"
#include	"Interface.h"
#include	"SrcFile.h"
#include	"utility.h"

long
num_lines( SrcFile * srcfile )
{
	if ( srcfile == 0 )
	{
		printe("internal error: ");
		printe("null pointer to src_text()\n");
		return 0;
	}
	else
	{
		return srcfile->num_lines();
	}
}
