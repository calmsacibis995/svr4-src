/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/svr4.c	1.2"


#include "syn.h"
#include <sys/utsname.h>
#include "libelf.h"
#include "decl.h"


int
_elf_svr4()
{
	struct utsname	u;
	static int	vers = -1;

	if (vers == -1)
	{
		if (uname(&u) > 0)
			vers = 1;
		else
			vers = 0;
	}
	return vers;
}
