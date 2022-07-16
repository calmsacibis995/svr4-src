/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:i386/fmod.c	1.4"
/*LINTLIBRARY*/

/* fmod(x,y)
 * Return f =x-n*y, for some integer n, where f has same sign
 * as x and |f| < |y|
 *
 *  for y == 0, f = 0 
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>

asm double xfmod(x, y)
{
%mem	x,y;
	fldl	y
	fldl	x
.dorem:
	fprem
	fstsw	%ax
	testl	$0x400,%eax
	jne	.dorem
	ffree	%st(1)
}

double	fmod(x, y)
double	x, y;
{

	double xfmod();
	if (y == 0.0) {
		struct exception exc;
		exc.type = DOMAIN;
		exc.name = "fmod";
		exc.arg1 = x;
		exc.arg2 = 0.0;
		exc.retval = x;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else {
			if (!matherr(&exc)) {
				if (_lib_version == c_issue_4)
					(void)write(2,"fmod: DOMAIN error\n",19);
				errno = EDOM;
			}
		}
		return exc.retval;
	}
	return xfmod(x,y);
}
