/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/ctype.c	1.13"

#ifdef __STDC__
	#pragma weak setchrclass = _setchrclass
#endif
#include "synonyms.h"
#include <locale.h>

int
setchrclass(ccname)
const char *ccname;
{
	if (ccname == 0)
		ccname = "";
	if (setlocale(LC_CTYPE, ccname) == NULL)
		return(-1);
	return(0);
}
