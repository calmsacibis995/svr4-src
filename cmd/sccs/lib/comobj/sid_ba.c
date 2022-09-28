/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/sid_ba.c	6.3"
# include	"../../hdr/defines.h"


char *
sid_ba(sp,p)
register struct sid *sp;
register char *p;
{
	sprintf(p,"%d.%d",sp->s_rel,sp->s_lev);
	while (*p++)
		;
	--p;
	if (sp->s_br) {
		sprintf(p,".%d.%d",sp->s_br,sp->s_seq);
		while (*p++)
			;
		--p;
	}
	return(p);
}
