/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:libc-i386/fp/fpsetmask.c	1.4"

#include <ieeefp.h>
#include "fp.h"
#include "synonyms.h"

#ifdef __STDC__
	#pragma weak fpsetmask = _fpsetmask
#endif
fp_except
fpsetmask(newmask)
fp_except newmask;
{
	struct _cw87 cw;
	fp_except oldmask;

	_getcw(&cw);
	oldmask = (fp_except)(~cw.mask & EXCPMASK);
	cw.mask = ~((unsigned)newmask) & EXCPMASK;
	_putcw(cw);
	return oldmask;
}


