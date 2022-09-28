/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/chksid.c	6.3"
# include	"../../hdr/defines.h"

void
chksid(p,sp)
char *p;
register struct sid *sp;
{
	int fatal();
	if (*p ||
		(sp->s_rel == 0 && sp->s_lev) ||
		(sp->s_lev == 0 && sp->s_br) ||
		(sp->s_br == 0 && sp->s_seq))
			(void) fatal("invalid sid (co8)");
}
