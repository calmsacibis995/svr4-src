/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/flushto.c	6.3"
# include	"../../hdr/defines.h"

void
flushto(pkt,ch,put)
register struct packet *pkt;
register char ch;
int put;
{
	register char *p;
	void	fmterr(), putline();
	char 	*getline();

	while ((p = getline(pkt)) != NULL && !(*p++ == CTLCHAR && *p == ch))
		pkt->p_wrttn = (char) put;

	if (p == NULL)
		fmterr(pkt);

	putline(pkt,(char *) 0);
}
