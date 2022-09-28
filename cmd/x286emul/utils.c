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

#ident	"@(#)x286emul:utils.c	1.1"

#include "vars.h"
#include <setjmp.h>
#include <sys/types.h>
#include <sys/sysi86.h>
#include <sys/immu.h>

char *
getmem( size )
{
	register char * cp;

	cp = malloc( size );
	if ( cp == 0 ) {
		emprintf(  "Malloc of %d byte failed\n", size );
		exit(1);
	}
	clearmem( cp, size );
	return cp;
}

emprintf( FMT, arg0, arg1, arg2, arg3, arg4, arg5 )
	char *FMT;
	long arg0, arg1, arg2, arg3, arg4, arg5;
{
	fprintf( stderr, "x286emul: " );
	fprintf( stderr, FMT, arg0, arg1, arg2, arg3, arg4, arg5 );
}

/*
 * convert 286 selector:offset to 386 pointer
 */
char * cvtptr( seloff )
	unsigned int seloff;
{
	int index, off;

	if ( seloff == 0 )
		return 0;
	off = OFF(seloff);
	index = SELTOIDX(SEL(seloff));

	if ( index >= Numdsegs || off >= Dsegs[index].lsize )
		return BAD_ADDR;
	return Dsegs[index].base + off;
}

/*
 * convert 286 selector:offset to 386 pointer, and check for legality
 */
jmp_buf bad_pointer;

char * cvtchkptr( seloff )
	unsigned int seloff;
{
	int index, off;
	long answer;

	if ( seloff == 0 )
		longjmp( bad_pointer, 1 );
	off = OFF(seloff);
	index = SELTOIDX(SEL(seloff));

	if ( index >= Numdsegs )
		answer = (unsigned long)BAD_ADDR;
	else
		answer = (long)Dsegs[index].base;
	if ( answer == (unsigned long)BAD_ADDR || off >= Dsegs[index].lsize)
		longjmp( bad_pointer, 1 );
	return (char *)answer + off;
}

#define ALLOCDSEGS	32	/* how many Dsegs to alloc at a time */

nextfreedseg(cnt, whence)
	int cnt, whence;
{
	int start, end, i, found;

	found = 0;
	if (whence == ANYWHERE) {
		for (i = Lastdseg; !found && i < Numdsegs; i++) {
			if ((Dsegs[i].base == BAD_ADDR) ||
			    			(Dsegs[i].lsize == 0))	{
				start = i;
				if (cnt > 1) {
					end = i + cnt - 1;
					while (i < end) {
						if (i >= Numdsegs &&	
							!moredsegs(ALLOCDSEGS))
							return 0;
						if ((Dsegs[i].base != BAD_ADDR)
						       && (Dsegs[i].lsize != 0))
							break;
						i++;
					}
					if (i == end+1) ++found;
				} else {
					++found;
				}
			}
		}
	}

	if (!found) {
		if (( Dsegs[Numdsegs-1].base != BAD_ADDR ) 
			&& (Dsegs[Numdsegs-1].lsize != 0 ))
			if ( !moredsegs(ALLOCDSEGS + cnt) )
				return 0;
	
		for ( i = Numdsegs-1; i--; i >= 0 )
			if (( Dsegs[i].base != BAD_ADDR )
				&& (Dsegs[i].lsize != 0 ))
				break;
		start = i+1;
		end = start + cnt - 1;
		if (end > Numdsegs) {
			if ( !moredsegs(ALLOCDSEGS + end - Numdsegs) )
				return 0;
		}
		
	}

	return start;
}

moredsegs( n )
{
	struct dseg * old;
	int i;

	old = Dsegs;
	Dsegs = (struct dseg *)malloc( sizeof(*Dsegs) * (Numdsegs+n) );
	if ( Dsegs == 0 ) {
		Dsegs = old;
		return 0;
	}
	if ( n < 0 )
		Numdsegs += n;
	for ( i = 0; i < Numdsegs; i++ )
		Dsegs[i] = old[i];
	if ( n > 0 ) {
		Numdsegs += n;
		for ( i = Numdsegs-1; n-- > 0; i-- ) {
			Dsegs[i].base = BAD_ADDR;
			Dsegs[i].lsize = 0;
			Dsegs[i].psize = 0;
		}
	}
	if ( old )
		free(old);
	return 1;
}

/*
 * PatchMem changes the memory mapped file image for all execution instances
 * sharing the image.  The change begins at addr, goes for len bytes, and
 * is replaced with the data found at image.
 */
PatchMem(addr, len, image)
	char * addr;
	long len;
	char * image;
{
	struct cmf c;

#ifdef DEBUG
	if (len < 0 || len > 65535) {
		fprintf( dbgfd, "PatchMem: bad length 0x%lx\n",len);
		exit(1);
	}
#endif

	c.cf_srcva = image;
	c.cf_dstva = addr;
	c.cf_count = len;

	if (sysi86(SI86PCHRGN, &c) == -1) {
		emprintf("Couldn't PCHRGN\n");
		perror("");
		exit(1);
	}
}

/*
 * replace with something more efficient later
 */
copymem( src, dst, count )
	char * src, * dst;
	int count;
{
	while ( count-- > 0 )
		*dst++ = *src++;
}

clearmem( src, size )
	char * src;
	int size;
{
	while ( size-- > 0 )
		*src++ = 0;
}
