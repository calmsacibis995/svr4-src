/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/mbstowcs.c	1.2"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <stdlib.h>

size_t
mbstowcs(pwcs, s, n)
wchar_t	*pwcs;
const char *s;
size_t n;
{
	int	i, val;

	for (i = 0; i < n; i++) {
		if ((val = mbtowc(pwcs++, s, MB_CUR_MAX)) == -1)
			return(val);
		if (val == 0)
			break;
		s += val;
	}
	return(i);
}
