/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:i386/fmodf.c	1.4"
/*LINTLIBRARY*/

/* fmodf(x,y) - single precision version
 * Return f =x-n*y, for some integer n, where f has same sign
 * as x and |f| < |y|
 *
 *  for y == 0, f = 0 
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>

asm float xfmodf(float x, float y)
{
%mem	x,y;
	flds	y
	flds	x
.dorem:
	fprem
	fstsw	%ax
	testl	$0x400,%eax
	jne	.dorem
	ffree	%st(1)
}

float	fmodf(float x, float y)
{

	float xfmodf(float,float);
	if (y == (float)0.0) {
		struct exception exc;
		exc.type = DOMAIN;
		exc.name = "fmodf";
		exc.arg1 = (double)x;
		exc.arg2 = 0.0;
		exc.retval = (double)x;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else {
			if (!matherr(&exc)) {
				if (_lib_version == c_issue_4)
					(void)write(2,"fmodf: DOMAIN error\n",20);
				errno = EDOM;
			}
		}
		return (float)exc.retval;
	}
	return xfmodf(x,y);
}
