/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/fmterr.c	6.3"
# include	"../../hdr/defines.h"

void
fmterr(pkt)
register struct packet *pkt;
{
	int	fatal();
	(void) fclose(pkt->p_iop);
	sprintf(Error,"format error at line %d (co4)",pkt->p_slnno);
	fatal(Error);
}
