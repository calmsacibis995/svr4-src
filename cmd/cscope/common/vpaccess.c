/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/vpaccess.c	1.1"
/* vpaccess - view path version of the access system call */

#include "vp.h"
#ifdef CCS
#include <sys/types.h>
#include "ccstypes.h"
#else
typedef int mode_t;
#endif
 
vpaccess(path, amode)
char	*path;
mode_t	amode;
{
	char	buf[MAXPATH + 1];
	int	returncode;
	int	i;

	if ((returncode = access(path, amode)) == -1 && path[0] != '/') {
		vpinit((char *) 0);
		for (i = 1; i < vpndirs; i++) {
			(void) sprintf(buf, "%s/%s", vpdirs[i], path);
			if ((returncode = access(buf, amode)) != -1) {
				break;
			}
		}
	}
	return(returncode);
}
