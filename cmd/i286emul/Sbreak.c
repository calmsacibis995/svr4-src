/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:Sbreak.c	1.1"

/*
 * deal with the brk system call
 */

#include <stdio.h>
#include "vars.h"
#include <errno.h>

Sbreak( ap )
	ushort * ap;
{
#define BRK     1
#define SBRK    0
#define LARGE   2
#define SMALL   0

dprintf( "(S)break: args 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
	ap[0], ap[1], ap[2], ap[3], ap[4] ); fflush( stdout );
	switch ( (*ap++ == 0) + (uu_model != 0) * 2 ) {
	case BRK + SMALL:
		return sb( ap[0], ap[1] ); break;
	case BRK + LARGE:
		return lb( ap[0], ap[1] ); break;
	case SBRK + SMALL:
		return ss( ap[0], ap[1] ); break;
	case SBRK + LARGE:
		return ls( ap[0], ap[1] ); break;
	default:
		emprintf(  "compiler bug: bad type in switch\n" );
		errno = EAGAIN;
		return -1;
	}
}

/*
 * small model sbreak
 *
 * Note: due to the way the system call library is implemented,
 * a small model process never really does an sbrk!
 */
ss( incr, unused )
	int incr;
{
	int retval;
	int newbreak;

dprintf( "small sbreak\n" ); fflush(stdout);
	if ( incr > 0 )
		incr = ctob(btoc( incr ));
	else
		incr = -ctob(btoc( -incr ));
	retval = smdsize;
	newbreak = retval + incr;
	if ( newbreak < smssize || newbreak > 65536 ) {
		errno = EINVAL;
		return -1;
	}
	if ( newbreak > smdsize )
		clearmem( stackbase + smdsize, newbreak - smdsize );
	smdsize = newbreak;
	return retval;
}

/*
 * small model break
 */
sb( off, sel )
	unsigned int off, sel;
{
dprintf( "small break\n" ); fflush(stdout);
	if ( ((sel>>3)&0x1FFF) != firstds ) {
		errno = EINVAL;
		return -1;
	}
	off = ctob(btoc( off ));
	if ( off < smssize || off > 65536 ) {
		errno = ENOMEM;
		return -1;
	}
	smdsize = off;
	if ( off > smdsize )
		clearmem( stackbase + smdsize, off - smdsize );
	return 0;
}

/*
 * large sbreak
 */
ls( uincr, unused )
	int uincr;
{
	int incr;
	int retval;
	int ds;
	int memsize;

	incr = (short)(uincr & 0xffff);	/* sign-extend */
dprintf( "large sbreak(0x%x)\n", incr ); fflush(stdout);
	retval = ((lastds + 1)<<19) | 0x70000;
dprintf( "return will be 0x%x\n", retval ); fflush(stdout);
	if ( incr == 0 )
		return retval;

	if ( incr > 0 ) {
		char * base;
dprintf( "adding memory\n" ); fflush( stdout );
		if ( dsegs[1+lastds].size ) {
dprintf( "no room ( next segment already used )\n" ); fflush( stdout );
			errno = ENOMEM;
			return -1;
		}
		incr = ctob(btoc(incr));        /* round up */
dprintf( "incr normalized: 0x%x\n", incr ); fflush( stdout );
		base = getmem(incr);
		clearmem( base, incr );
		lastds++;                       /* new last data segment */
		setsegdscr( lastds * 8 + 7, base, incr, 1 );
		return retval;
	}
	/* getting rid of memory */

	incr = ctob(btoc(-incr));       /* how much we don't want */
dprintf( "getting rid of 0x%x bytes\n", incr ); fflush( stdout );

	/* figure out how much we have */

	ds = firstds;
	memsize = 0;
	do {
		memsize += dsegs[ds].size;
	} while ( ds++ != lastds );
dprintf( "proc has 0x%x bytes\n", memsize ); fflush( stdout );
	if ( incr >= memsize ) {
		errno = EINVAL;
		return -1;
	}

	/* we aren't giving it all away, so go ahead */

	while ( incr ) {
		if ( incr >= dsegs[lastds].size ) {
			incr -= dsegs[lastds].size;
			setsegdscr( lastds*8+7, 0, 0, 2 );
			lastds--;
		} else {
			/* segment is getting smaller */

			int newsize;
			char * newbase;

			newsize = dsegs[lastds].size - incr;
			incr = 0;
			newbase = getmem( newsize );

			copymem( dsegs[lastds].base, newbase, newsize );
			setsegdscr( lastds*8+7, 0, 0, 2 );
			setsegdscr( lastds*8+7, newbase, newsize, 1 );
		}
	}
	return retval;
}

/*
 * large model break
 */
lb( off, sel )
	unsigned sel, off;
{
	int index;
	char * newbase;
	int oldsize;

dprintf( "large break(0x%x:0x%x)\n", sel, off ); fflush(stdout);
	index = ( sel >> 3 ) & 0x1fff;
	off = ctob(btoc(off));
dprintf( "offset normalized to 0x%x\n", off ); fflush( stdout );

	if ( index > lastds ) {
		int lsrv;
		if ( index != lastds + 1 ) {
			errno = ENOMEM;
			return -1;
		}
		lsrv = ls( off, 0 );
dprintf( "ls returns %x\n", lsrv ); fflush( stdout );
dprintf( "lb returning %x\n", (lsrv == -1 ) ? -1 : 0 ); fflush(stdout);
		return (lsrv == -1) ? -1 : 0;
	}

	if ( index < firstds || (off == 0 && index == firstds) ) {
		errno = EINVAL;
		return -1;
	}
	if ( lastds > index ) {
		/*
		 * make sure breaking down to inside a segment
		 */
		if ( off > dsegs[index].size ) {
			errno = EINVAL;
			return -1;
		}
		while ( lastds > index ) {
			setsegdscr( lastds * 8 + 7, 0, 0, 2 );
			lastds--;
		}
	}
	/*
	 * adjusting last data segment
	 */
	oldsize = dsegs[index].size;
	if ( off < oldsize ) {
		newbase = getmem( off );
		copymem( dsegs[index].base, newbase, off );
		setsegdscr( index*8+7, 0, 0, 2 );
		setsegdscr( index*8+7, newbase, off, 1 );
	} else
	if ( off > oldsize ) {
		newbase = getmem(off);
		copymem( dsegs[index].base, newbase, oldsize );
		clearmem( newbase + oldsize, off - oldsize );
		setsegdscr( index*8+7, 0, 0, 2 );
		setsegdscr( index*8+7, newbase, off, 1 );
	}
	return 0;
}

/*
 * Memory clearing and copying.  Should be done in assembly, using
 * string instructions
 */
clearmem( base, size )
	register char * base;
	register size;
{
	while ( size-- > 0 )
		*base++ = 0;
}

copymem( src, dst, size )
	register char * src, *dst;
	register size;
{
	while ( size-- > 0 )
		*dst++ = *src++;
}
