/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/swap.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)swap.c	3.5	LCC);	/* Modified: 10:11:18 7/14/87 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include "flip.h"
#include "pci_types.h"

int	rd_flag = 0;			/* reliable delivery flag */


/*
 * 	swap() -			auto-sense byte swapping routine.
 *					Returns how bytes were swapped.
 */

int
input_swap(ptr, pattern)
register struct input *ptr;
register long	pattern;
{
    register	int
	how;

#ifdef	VERSION_MATCHING
	register struct	connect_text *c_ptr;
#endif	/* VERSION_MATCHING */
	register struct	emhead       *e_ptr;

    short	tmpshort;

    long	tmplong;


    if ((how = FLIPHOW(pattern)) == NOFLIP)
	return;

#ifdef RS232PCI
    sflipm(ptr->rs232.f_cnt, tmpshort, how);
    sflipm(ptr->rs232.chks, tmpshort, how);
#endif /* RS232PCI */

    sflipm(ptr->hdr.pid, tmpshort, how);
    sflipm(ptr->hdr.fdsc, tmpshort, how);
    sflipm(ptr->hdr.mode, tmpshort, how);
    sflipm(ptr->hdr.date, tmpshort, how);
    sflipm(ptr->hdr.time, tmpshort, how);
    sflipm(ptr->hdr.b_cnt, tmpshort, how);
    sflipm(ptr->hdr.t_cnt, tmpshort, how);
    sflipm(ptr->hdr.inode, tmpshort, how);
    sflipm(ptr->hdr.versNum, tmpshort, how);
    lflipm(ptr->hdr.f_size, tmplong, how);
    lflipm(ptr->hdr.offset, tmplong, how);
    lflipm(ptr->hdr.pattern, tmplong, how);

	switch (ptr->hdr.req) {		/* process special cases */

		case CONNECT :
#ifdef	VERSION_MATCHING
				c_ptr = (struct connect_text *) ptr->text;
				sflipm(c_ptr->vers_major, tmpshort, how);
				sflipm(c_ptr->vers_minor, tmpshort, how);
				sflipm(c_ptr->vers_submin, tmpshort, how);
#endif	/* VERSION_MATCHING */
				break;

		default :
			/* This will check for emulation stream packets.
			 * unfortunately this is necessary.  We might
			 * want to change the way this is done later. - rp
			 */
				if (rd_flag) {
					if (ptr->pre.select ==  SHELL) {
				    	    e_ptr = (struct emhead *) ptr->text;
				    	    sflipm(e_ptr->dnum, tmpshort, how);
				            sflipm(e_ptr->anum, tmpshort, how);
				            sflipm(e_ptr->strsiz,tmpshort, how);
					}
				}
					
				break;
	}
    
	return how;
}


/*
 *	output_swap() -
 */

void
output_swap(ptr, how)
register	struct	output	*ptr;
register	int	how;
{
    short	tmpshort;

    long	tmplong;
    struct	emhead *e_ptr;

#ifndef RS232PCI
    ptr->hdr.pattern = SENSEORDER;
#endif /* !RS232PCI */

    if (how == NOFLIP)
	return;

#ifdef RS232PCI
    sflipm(ptr->rs232.f_cnt, tmpshort, how);
    sflipm(ptr->rs232.chks, tmpshort, how);
#endif /* RS232PCI */

    sflipm(ptr->hdr.pid, tmpshort, how);
    sflipm(ptr->hdr.fdsc, tmpshort, how);
    sflipm(ptr->hdr.mode, tmpshort, how);
    sflipm(ptr->hdr.date, tmpshort, how);
    sflipm(ptr->hdr.time, tmpshort, how);
    sflipm(ptr->hdr.b_cnt, tmpshort, how);
    sflipm(ptr->hdr.t_cnt, tmpshort, how);
    sflipm(ptr->hdr.inode, tmpshort, how);
    sflipm(ptr->hdr.versNum, tmpshort, how);
    lflipm(ptr->hdr.f_size, tmplong, how);
    lflipm(ptr->hdr.offset, tmplong, how);
    lflipm(ptr->hdr.pattern, tmplong, how);

/*
 * Swap emulation headers if and only if rd_flag is TRUE
 */

	/* This will check for emulation stream packets.
	 * unfortunately this is necessary.  We might
	 * want to change the way this is done later. - rp
	 */
	if (rd_flag) {
		if (ptr->pre.select ==  SHELL) {
	    	    e_ptr = (struct emhead *) ptr->text;
	    	    sflipm(e_ptr->dnum, tmpshort, how);
	            sflipm(e_ptr->anum, tmpshort, how);
	            sflipm(e_ptr->strsiz, tmpshort, how);
		}
	}
				
}
