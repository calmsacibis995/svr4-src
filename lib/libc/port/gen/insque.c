/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/insque.c	1.4"

/* insque() and remque() insert or remove an element from a queue.
 * The queue is built from a doubly linked list whose elements are
 * defined by a structure where the first member of the structure
 * points to the next element in the queue and the second member
 * of the structure points to the previous element in the queue.
 */

#ifdef __STDC__
	#pragma weak insque = _insque
	#pragma weak remque = _remque
#endif

#include "synonyms.h"

struct qelem {
	struct qelem	*q_forw;
	struct qelem	*q_back;
	/* rest of structure */
};

void
insque(elem, pred)
struct qelem *elem, *pred;
{
	register struct qelem *pelem = elem;
	register struct qelem *ppred = pred;
	register struct qelem *pnext = ppred->q_forw;
	pelem->q_forw = pnext;
	pelem->q_back = ppred;
	ppred->q_forw = pelem;
	pnext->q_back = pelem;
}

void
remque(elem)
struct qelem *elem;
{
	register struct qelem *pelem = elem;
	pelem->q_forw->q_back = pelem->q_back;
	pelem->q_back->q_forw = pelem->q_forw;
}
