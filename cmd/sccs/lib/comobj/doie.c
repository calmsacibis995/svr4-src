/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/doie.c	6.4"
# include	"../../hdr/defines.h"

void
doie(pkt,ilist,elist,glist)
struct packet *pkt;
char *ilist, *elist, *glist;
{
	void	dolist();

	if (ilist) {
		if (pkt->p_verbose & DOLIST) {
			fprintf(pkt->p_stdout,"================\n");
			fprintf(pkt->p_stdout,"Included:\n");
			dolist(pkt,ilist,INCLUDE);
			fprintf(pkt->p_stdout,"================\n");
		}
		else dolist(pkt,ilist,INCLUDE);
	}
	if (elist) {
		if (pkt->p_verbose & DOLIST) {
			fprintf(pkt->p_stdout,"================\n");
			fprintf(pkt->p_stdout,"Excluded:\n");
			dolist(pkt,elist,EXCLUDE);
			fprintf(pkt->p_stdout,"================\n");
		}
		else dolist(pkt,elist,EXCLUDE);
	}
	if (glist)
		dolist(pkt,glist,IGNORE);
}
