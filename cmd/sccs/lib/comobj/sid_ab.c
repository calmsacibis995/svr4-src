/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/sid_ab.c	6.2"
# include	"../../hdr/defines.h"


char *
sid_ab(p,sp)
register char *p;
register struct sid *sp;
{
	extern	char	*satoi();
	if (*(p = satoi(p,&sp->s_rel)) == '.')
		p++;
	if (*(p = satoi(p,&sp->s_lev)) == '.')
		p++;
	if (*(p = satoi(p,&sp->s_br)) == '.')
		p++;
	p = satoi(p,&sp->s_seq);
	return(p);
}
