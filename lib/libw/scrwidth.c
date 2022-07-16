/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libw:scrwidth.c	1.3"
#include	"libw.h"
#include	<ctype.h>
#include "_wchar.h"

int scrwidth(c)
wchar_t c;
{
	if(!wisprint(c))
		return 0;
	if(!multibyte || c <= 0177)
		return 1;
	if(MB_CUR_MAX > 3 || eucw1 > 2)
		return((c & P11) == P11 ? scrw1 :
		(c & P01) ? scrw2 : scrw3);
	else
		return((c & H_P11) == H_P11 ? scrw1 :
		(c & H_P01) ? scrw2 : scrw3);
}
