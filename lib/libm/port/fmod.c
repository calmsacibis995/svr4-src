/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/fmod.c	1.9"
/*LINTLIBRARY*/

/* fmod(x,y)
 * Return f =x-n*y, for some integer n, where f has same sign
 * as x and |f| < |y|
 *
 *  for y == 0, f = x 
 * This implementation calculates IEEE rem and uses that to calculate
 * fmod
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>

double	fmod(x, y)
register double	x, y;
{
#ifdef	M32 /* for use by machine rem instruction if MAU present on
	     * 32100 processors
	     */
	double x1, y1;
	extern int _fp_hw;
#endif
	register double	r;

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
	if (y < 0.0)
		y = -y;
#ifdef M32
	/* on 32100 processors, if MAU present, use machine rem 
	 * instruction
	 */
	if (_fp_hw) {
		x1 = x; /* MAU operands can't be in registers */
		y1 = y;
		asm("	mfremd2	8(%fp),0(%fp)");
		r = x1;
	}
	else
		r = remainder(x, y);
#else
	r = remainder(x, y);
#endif
	/*
	 * At this point we have remainder(x,y)
	 * to get fmod(x,y), we test the signs of x and of the
	 * remainder - if they're the same, we are done,
	 * else if x is negative, replace remainder with remainder -|y|,
	 * if x positive with remainder + |y| 
	 */
	if (x < 0.0) {
		if (r > 0.0)
			r -= y;
	}
	else { 
		if (r < 0.0)
			r += y;
	
	}
	return r;
}
