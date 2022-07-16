/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/hrtnewres.c	1.3"

#include	"synonyms.h"
#include	<sys/types.h>
#include	<sys/dl.h>
#include	<sys/evecb.h>
#include	<sys/hrtcntl.h>


/*	Convert interval expressed in htp->hrt_res to new_res.
**
**	Calculate: (interval * new_res) / htp->hrt_res  rounding off as
**		specified by round.
**
**	Note:	All args are assumed to be positive.  If
**	the last divide results in something bigger than
**	a long, then -1 is returned instead.
*/

_hrtnewres(htp, new_res, round)
register hrtime_t *htp;
register ulong new_res;
long round;
{
	register long  interval;
	dl_t		dint;
	dl_t		dto_res;
	dl_t		drem;
	dl_t		dfrom_res;
	dl_t		prod;
	dl_t		quot;
	register long	numerator;
	register long	result;
	ulong		modulus;
	ulong		twomodulus;
	long		temp;

	if (htp->hrt_res <= 0 || new_res <= 0 ||
			new_res > NANOSEC || htp->hrt_rem < 0)
		return(-1);

	if (htp->hrt_rem >= htp->hrt_res) {
		htp->hrt_secs += htp->hrt_rem / htp->hrt_res;
		htp->hrt_rem = htp->hrt_rem % htp->hrt_res;
	}

	interval = htp->hrt_rem;
	if (interval == 0) {
		htp->hrt_res = new_res;
		return(0);
	}

	/*	Try to do the calculations in single precision first
	**	(for speed).  If they overflow, use double precision.
	**	What we want to compute is:
	**
	**		(interval * new_res) / hrt->hrt_res
	*/

	numerator = interval * new_res;

	if (numerator / new_res  ==  interval) {
			
		/*	The above multiply didn't give overflow since
		**	the division got back the original number.  Go
		**	ahead and compute the result.
		*/
	
		result = numerator / htp->hrt_res;
	
		/*	For HRT_RND, compute the value of:
		**
		**		(interval * new_res) % htp->hrt_res
		**
		**	If it is greater than half of the htp->hrt_res,
		**	then rounding increases the result by 1.
		**
		**	For HRT_RNDUP, we increase the result by 1 if:
		**
		**		result * htp->hrt_res != numerator
		**
		**	because this tells us we truncated when calculating
		**	result above.
		**
		**	We also check for overflow when incrementing result
		**	although this is extremely rare.
		*/
	
		if (round == HRT_RND) {
			modulus = numerator - result * htp->hrt_res;
			if ((twomodulus = 2 * modulus) / 2 == modulus) {

				/*
				 * No overflow (if we overflow in calculation
				 * of twomodulus we fall through and use
				 * double precision).
				 */
				if (twomodulus >= htp->hrt_res) {
					temp = result + 1;
					if (temp - 1 == result)
						result++;
					else
						return(-1);
				}
				htp->hrt_res = new_res;
				htp->hrt_rem = result;
				return(0);
			}
		} else if (round == HRT_RNDUP) {
			if (result * htp->hrt_res != numerator) {
				temp = result + 1;
				if (temp - 1 == result)
					result++;
				else
					return(-1);
			}
			htp->hrt_res = new_res;
			htp->hrt_rem = result;
			return(0);
		} else {	/* round == HRT_TRUNC */
			htp->hrt_res = new_res;
			htp->hrt_rem = result;
			return(0);
		}
	}
	
	/*	We would get overflow doing the calculation is
	**	single precision so do it the slow but careful way.
	**
	**	Compute the interval times the resolution we are
	**	going to.
	*/

	dint.dl_hop	= 0;
	dint.dl_lop	= interval;
	dto_res.dl_hop	= 0;
	dto_res.dl_lop	= new_res;
	prod		= lmul(dint, dto_res);

	/*	For HRT_RND the result will be equal to:
	**
	**		((interval * new_res) + htp->hrt_res / 2) / htp->hrt_res
	**
	**	and for HRT_RNDUP we use:
	**
	**		((interval * new_res) + htp->hrt_res - 1) / htp->hrt_res
	**
	** 	This is a different but equivalent way of rounding.
	*/

	if (round == HRT_RND) {
		drem.dl_hop = 0;
		drem.dl_lop = htp->hrt_res / 2;
		prod	    = ladd(prod, drem);
	} else if (round == HRT_RNDUP) {
		drem.dl_hop = 0;
		drem.dl_lop = htp->hrt_res - 1;
		prod	    = ladd(prod, drem);
	}

	dfrom_res.dl_hop = 0;
	dfrom_res.dl_lop = htp->hrt_res;
	quot		 = ldivide(prod, dfrom_res);

	/*	If the quotient won't fit in a long, then we have
	**	overflow.  Otherwise, return the result.
	*/

	if (quot.dl_hop != 0) {
		return(-1);
	} else {
		htp->hrt_res = new_res;
		htp->hrt_rem = quot.dl_lop;
		return(0);
	}
}
