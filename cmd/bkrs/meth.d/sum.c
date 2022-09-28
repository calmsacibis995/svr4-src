/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/sum.c	1.9.2.1"

#include	<limits.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<bktypes.h>
#include 	"libadmIO.h"
#include	<method.h>
#include	<errno.h>

/* code from cmd/sum.c to sum a single backup media */

extern int	brlog();
extern int	close();
extern int	g_close();
extern GFILE	*g_open();
extern int	g_read();

void
sum(buf, nbytes, cur_sum)
register char	*buf;
register long	nbytes;
unsigned	*cur_sum;
{
	register unsigned	sump = *cur_sum;
	register long		i;
	register		c;

	for (i = 0; i < nbytes; i++) {
		c = *buf++;

		if (sump&01)
			sump = (sump >> 1) + 0x8000;
		else
			sump >>= 1;

		sump += c;

		sump &= 0xFFFF;
	}
	*cur_sum = sump;
} /* sum() */

chk_vol_sum(mp, f, nbytes, filename, old_sum)
m_info_t	*mp;
GFILE		**f;
long		nbytes;
char		*filename;
unsigned	old_sum;
{
	long		l;
	char		buf[4096];
	int		n;
	int		i;
	unsigned	new_sum;

	(void) g_close(*f);

	(*f) = g_open (filename, O_RDONLY, 0);

	if ((*f) == NULL) {
		brlog("reopen of %s failed %s", filename, SE);
		return(1);
	}
	new_sum = 0;

#ifdef TRACE
	brlog("chk_vol_sum: nbytes=%d",nbytes);
#endif
	while (nbytes > 0) {
		n = (nbytes > 4096) ? 4096 : nbytes;

		if ((i = g_read(*f, buf, n)) != n) {
			brlog("chk_vol_sum: read %d returned %d nbytes=%d  %s",
					n, i,nbytes, SE);
			return(1);
		}
		sum(buf, (long) n, &new_sum);
		nbytes -= n;
	}
#ifdef TRACE
	brlog("chk_vol_sum: old_sum=%x new_sum=%x",old_sum,new_sum);
#endif
	return(new_sum != old_sum);
} /* ckh_vol_sum() */
