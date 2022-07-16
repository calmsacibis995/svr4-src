/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/_set_tab.c	1.7"
/*
* _set_tab - set _ctype[], _numeric[] to requested locale-based info.
*/
#include "synonyms.h"
#include <locale.h>
#include "_locale.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int
_set_tab(loc, cat)
const char *loc;
int cat;
{
	unsigned char my_ctype[SZ_TOTAL];
	const static char *name[2] = { "LC_CTYPE", "LC_NUMERIC" };
	unsigned char *space;
	int size;
	register int fd;
	register int ret = -1;

	size = (cat ? SZ_NUMERIC : SZ_TOTAL);
	if ((fd = open(_fullocale(loc, name[cat]), O_RDONLY)) == -1)
		return ret;
	else if (read(fd, (char *)my_ctype, size) == size) 
	{
		space = (cat ? _numeric : _ctype);
		(void)memcpy(space, my_ctype, size);
		ret = 0;
	}
	(void)close(fd);
	return ret;
}
