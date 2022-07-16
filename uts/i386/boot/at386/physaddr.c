/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)boot:boot/at386/physaddr.c	1.1.2.1"

#include "../sys/boot.h"

/* this is stolen from <sys/seg.h>; don't want the whole thing */

struct dscr {
	unsigned int	a_lim0015:16,
		 	a_base0015:16,
		  	a_base1623:8,
			a_acc0007:8,
			a_lim1619:4,
			a_acc0811:4,
			a_base2431:8;
};

#define	GRANULARITY	8


/*	physaddr() converts a logical address to a physical one.
 *	The passed pointer is presumed to be relative to the %ds selector.
 *	Since paging is not turned on, the conversion is 
 *
 *		physaddr = %ds selector base + offset 
 *
 *	Note that we must take the granularity bit in the selector 
 *	into consideration when calculating the selector base.
 */

paddr_t
physaddr( p )
char	*p;
{
	struct dscr	*gdt;
	paddr_t		paddr;
	int		selector;

	/* set up gdt to point at the particular descriptor in question */

	selector = getDS();
	gdt = (struct dscr *)(&GDTstart);

	gdt += (selector / sizeof(struct dscr));

	/* extract the base address from the descriptor */

	paddr = (gdt->a_base2431 << 24) + (gdt->a_base1623 << 16) + 
			gdt->a_base0015;

	/* if the granularity bit is on, we have to multiply by 4096 */

	if ( gdt->a_acc0811 & GRANULARITY )
		paddr *= 4096;

	/* finally, add in the offset */

	paddr += (paddr_t)p;

	return( paddr );
}
