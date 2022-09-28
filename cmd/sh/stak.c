/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:stak.c	1.8.3.1"

/*
 * UNIX shell
 */

#include	"defs.h"


/* ========	storage allocation	======== */

unsigned char *
getstak(asize)			/* allocate requested stack */
int	asize;
{
	register unsigned char	*oldstak;
	register int	size;

	size = round(asize, BYTESPERWORD);
	oldstak = stakbot;
	staktop = stakbot += size;
	return(oldstak);
}

/*
 * set up stack for local use
 * should be followed by `endstak'
 */
unsigned char *
locstak()
{
	if (brkend - stakbot < BRKINCR)
	{
		if (setbrk(brkincr) == -1)
			error(nostack);
		if (brkincr < BRKMAX)
			brkincr += 256;
	}
	return(stakbot);
}

unsigned char *
savstak()
{
	assert(staktop == stakbot);
	return(stakbot);
}

unsigned char *
endstak(argp)		/* tidy up after `locstak' */
register unsigned char	*argp;
{
	register unsigned char	*oldstak;

	*argp++ = 0;
	oldstak = stakbot;
	stakbot = staktop = (unsigned char *)round(argp, BYTESPERWORD);
	return(oldstak);
}

tdystak(x)		/* try to bring stack back to x */
register unsigned char	*x;
{
	while ((unsigned char *)stakbsy > x)
	{
		free(stakbsy);
		stakbsy = stakbsy->word;
	}
	staktop = stakbot = max(x, stakbas);
	rmtemp(x);
}

stakchk()
{
	if ((brkend - stakbas) > BRKINCR + BRKINCR)
		setbrk(-BRKINCR);
}

unsigned char *
cpystak(x)
unsigned char	*x;
{
	return(endstak(movstr(x, locstak())));
}
