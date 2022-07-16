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

#ident	"@(#)boot:boot/at386/memsize.c	1.1.4.1"

#include "../sys/boot.h"
#include "sys/param.h"

/* SGN(a) returns the sign of the number a: 1 if positive, -1 if negative */

#define	SGN(a)			((a) < 0 ? -1 : ((a) > 0 ? 1 : 0))

/*
 * memsize(): 	here we size memory, using the MEMRANGE information we have 
 *		garnered from the defaults file ("/etc/default/boot").
 *		The original memrng entries can have negative extents, 
 *		in which case we size downwards. However, the resulting
 *		memsize extents are strictly positive.
 *
 *		We find only the first memory extent in the supplied memrange.
 */

bmemsize()
{
	int		i, j;
	int		bootFound;
	struct bootmem 	*r;
	paddr_t		bootstrap, p, start;
	int		memtotal;

	memtotal=0;
	bootstrap = physaddr(0);

	for ( i = 0, j = 0; i < memrngcnt; i++ ) {

		r = &memrng[i];

		/* 
		 * if we are sizing downwards, we must start one click down 
		 * from the base (since the base is therefore the high address,
		 * and thus not included in the range)
		 */

		p = start = (r->extent > 0) ? (r->base) : (r->base - NBPC);

		debug(printf("Looking for memory in the range: %lx to %lx\n",
			r->base, r->base + r->extent)); 

		/* touch memory while in the interval */

		bootFound = FALSE;
		for ( ;INTERVAL(r->base, r->extent, p); 
				p += SGN( r->extent )*NBPC ) {

			/* 
			 * Don't try to touch memory where the bootstrap lives
		 	 */

			if ( INTERVAL( bootstrap, BOOTSIZE, p ) ) {
				bootFound = TRUE;
				continue;
			}

			/* if no response, done */

			if ( !touchpage(p) ) {
				break;
			}
		}

		/* nothing found */

		if ( p == start )
			continue;

		/* found memory; set up binfo.memavail */

		if ( r->extent < 0 ) {	/* sizing down */
			binfo.memavail[j].extent = ((long)r->base - p) - NBPC;
			binfo.memavail[j].base = p + NBPC;
		} else {		/* sizing up */
			binfo.memavail[j].extent = (p - (long)r->base);
			binfo.memavail[j].base = r->base;
		}
		memtotal += binfo.memavail[j].extent;

		binfo.memavail[j].flags = r->flags;

		/* 
		 * If the area occupied by the bootstrap is in the free
		 * area just found, temporarily remove the bootstrap space
		 * from memavail; this prevents programs from being loaded
		 * over the bootstrap.
		 */

		if ( bootFound ) {
			p = binfo.memavail[j].base + binfo.memavail[j].extent;
			binfo.memavail[j].extent = bootstrap -
						binfo.memavail[j].base;
			binfo.memavail[j].flags |= B_MEM_BOOTSTRAP;
			if (p > bootstrap + BOOTSIZE) {
				binfo.memavail[++j].base = bootstrap + BOOTSIZE;
				binfo.memavail[j].extent = p -
						binfo.memavail[j].base;
				binfo.memavail[j].flags = r->flags;
			}
		}

		j++;
	}
	binfo.memavailcnt = j;

	if (memreq > 0 && memreq > memtotal) {
		printf("\n\n%s\n", mreqmsg1);
		printf("%s\n\n", mreqmsg2);
		halt();		
	}
}
