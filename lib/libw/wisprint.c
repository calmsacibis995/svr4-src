/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libw:wisprint.c	1.1.1.1"
#include	<stdlib.h>
#include	<ctype.h>
#include	"_wchar.h"
int wisprint(c)
wchar_t c;
{
	if((int)c > 0377)
		return(1);
	if((int)c < 0 || (c <= 0177 || !multibyte) && !isprint(c) || iscntrl(c))
		return(0);
	return(1);
}
