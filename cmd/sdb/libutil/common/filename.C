#ident	"@(#)sdb:libutil/common/filename.C	1.1"
#include	"Interface.h"
#include	"SrcFile.h"
#include	"utility.h"

char *
filename( SrcFile * srcfile )
{
	if ( srcfile == 0 )
	{
		printe("internal error: ");
		printe("null pointer to src_text()\n");
		return 0;
	}
	else
	{
		return srcfile->filename();
	}
}
