/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/stak.c	1.2.3.1"

#include	"defs.h"


/* ========	storage allocation	======== */

/*
 * allocate requested stack
 */

STKPTR	stak_alloc(asize)
unsigned 	asize;
{
	register STKPTR oldstak;
	register int 	size;
	size=round(asize,BYTESPERWORD);
	while((sh.staktop=sh.stakbot+size) > sh.brkend)
		sh_addmem((int)round(sh.staktop-sh.brkend,BRKINCR));
	oldstak=sh.stakbot;
	sh.stakbot = sh.staktop;
	return(oldstak);
}

/*
 * set up stack for local use
 * should be followed by `stak_end'
 */

STKPTR	stak_begin()
{
	if(sh.brkend-sh.stakbot<BRKINCR)
		sh_addmem(BRKINCR);
	return(sh.staktop=sh.stakbot);
}


/*
 * tidy up after `stak_begin'
 */

STKPTR	stak_end(argp)
register char *argp;
{
	register STKPTR oldstak;
	*argp++=0;
	oldstak=sh.stakbot;
	sh.stakbot += round((STKPTR)argp-sh.stakbot,BYTESPERWORD);
	sh.staktop = sh.stakbot;
	return(oldstak);
}

/*
 * try to bring stack back to x
 */

void	stak_reset(x)
register STKPTR  x;
{
	while(ADR(sh.stakbsy)>ADR(x))
	{
		free((char*)sh.stakbsy);
		sh.stakbsy = sh.stakbsy->word;
	}
	sh.staktop=sh.stakbot=max(ADR(x),ADR(sh.stakbas));
	if(st.iotemp > (struct ionod*)x)
		io_rmtemp((struct ionod*)x);
}

stak_check()
{
	register int size = -3*BRKINCR;
#ifdef INT16
	if((sh.brkend-sh.stakbas) >= BRKMAX)
		size = BRKMAX;
	else
#endif /* INT16 */
		size += round(sh.brkend-sh.stakbas,BRKINCR);
	if(size > 0)
		sh_addmem(-size);
}

STKPTR	stak_copy(x)
register const char	*x;
{
	register const char *y=x;
	while(*y++);
	return((STKPTR)strcpy(stak_alloc((unsigned)(y-x)),x));
}
