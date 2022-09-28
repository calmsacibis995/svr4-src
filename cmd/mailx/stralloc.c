/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:stralloc.c	1.3.5.1"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Memory allocation routines.
 * Memory handed out here are reclaimed at the top of the command
 * loop each time, so they need not be freed.
 */

#include "rcv.h"

/*
 * Allocate size more bytes of space and return the address of the
 * first byte to the caller.  An even number of bytes are always
 * allocated so that the space will always be on a word boundary.
 * The string spaces are of exponentially increasing size, to satisfy
 * the occasional user with enormous string size requests.
 */

VOID *
salloc(size)
	unsigned size;
{
	register char *t;
	register unsigned s;
	register struct strings *sp;
	int index;

	s = size;
	s += 3;		/* 3b's need alignment on quad boundary */
	s &= ~03;
	index = 0;
	for (sp = &stringdope[0]; sp < &stringdope[NSPACE]; sp++) {
		if (sp->s_topFree == NOSTR && (STRINGSIZE << index) >= s)
			break;
		if (sp->s_nleft >= s)
			break;
		index++;
	}
	if (sp >= &stringdope[NSPACE])
		panic("String too large");
	if (sp->s_topFree == NOSTR) {
		index = sp - &stringdope[0];
		sp->s_topFree = (char *) calloc(STRINGSIZE << index,
		    (unsigned) 1);
		if (sp->s_topFree == NOSTR) {
			fprintf(stderr, "No room for space %d\n", index);
			panic("Internal error");
		}
		sp->s_nextFree = sp->s_topFree;
		sp->s_nleft = STRINGSIZE << index;
	}
	sp->s_nleft -= s;
	t = sp->s_nextFree;
	sp->s_nextFree += s;
	return(t);
}

/*
 * Reallocate size bytes of space and return the address of the
 * first byte to the caller.  The old data is copied into the new area.
 */

VOID *
srealloc(optr, size)
	VOID *optr;
	unsigned size;
{
	VOID *nptr = salloc(size);
	if (nptr)
		memcpy(nptr, optr, size);
	return nptr;
}

/*
 * Reset the string area to be empty.
 * Called to free all strings allocated
 * since last reset.
 */
void
sreset()
{
	register struct strings *sp;
	register int index;

	if (noreset)
		return;
	minit();
	index = 0;
	for (sp = &stringdope[0]; sp < &stringdope[NSPACE]; sp++) {
		if (sp->s_topFree == NOSTR)
			continue;
		sp->s_nextFree = sp->s_topFree;
		sp->s_nleft = STRINGSIZE << index;
		index++;
	}
}
