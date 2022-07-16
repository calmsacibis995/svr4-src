/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:lmul.c	1.3"

#include	"sys/types.h"
#include	"sys/dl.h"

dl_t	dl_zero;

dl_t
lmul(lop, rop)
dl_t	lop;
dl_t	rop;
{
	dl_t		ans;
	dl_t		tmp;
	register int	jj;

	ans = dl_zero;

	for(jj = 0 ; jj <= 63  ; jj++){
		if((lshiftl(rop, -jj).dl_lop & 1) == 0)
			continue;
		tmp = lshiftl(lop, jj);
		tmp.dl_hop &= 0x7fffffff;
		ans = ladd(ans, tmp);
	};

	return(ans);
}
