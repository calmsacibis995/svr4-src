/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:libc-i386/fp/fpsetsticky.c	1.4"

#include <ieeefp.h>
#include "fp.h"
#include "synonyms.h"

#ifdef __STDC__
	#pragma weak fpsetsticky = _fpsetsticky
#endif

fp_except
_fpsetsticky(newsticky)
fp_except newsticky;
{
	struct _sw87 sw;
	_envbuf87 buf;
	fp_except oldsticky;

	_getsw(sw);
	oldsticky = (fp_except)sw.excp;
	sw.excp = newsticky;
	_putsw(sw, buf);
	return oldsticky;
}
