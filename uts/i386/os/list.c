/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:list.c	1.3"

#include "sys/list.h"

/* Generic lists
 * Lists are circular, doubly-linked, with headers.
 * When a list is empty, both pointers in the header
 * point to the header itself.
 */

#if !defined(NULL)
#define	NULL	0
#endif

/* Link new into list before old
 */
void
ls_ins_before(old, new)
	register struct ls_elt *old;
	register struct ls_elt *new;
{
	new->ls_prev = old->ls_prev;
	new->ls_next = old;
	new->ls_prev->ls_next = new;
	new->ls_next->ls_prev = new;
}
/* Link new into list after old
 */
void
ls_ins_after(old, new)
	register struct ls_elt *old;
	register struct ls_elt *new;
{
	new->ls_next = old->ls_next;
	new->ls_prev = old;
	new->ls_next->ls_prev = new;
	new->ls_prev->ls_next = new;
}

/* Unlink first element in the specified list and return
 * element's address, or NULL if the list is empty.
 * Resets result list pointers to empty list state.
 */
struct ls_elt *
ls_remque(p)
	struct ls_elt *p;
{
	register struct ls_elt *result = NULL;

	if (!LS_ISEMPTY(p)) {
		result = p->ls_next;
		result->ls_prev->ls_next = result->ls_next;
		result->ls_next->ls_prev = result->ls_prev;
		LS_INIT(result);
	}
	return result;
}

/* Unlink denoted element for list.
 * Resets elements pointers to emptyh list state.
 */
void
ls_remove(p)
	struct ls_elt *p;
{
	p->ls_prev->ls_next = p->ls_next;
	p->ls_next->ls_prev = p->ls_prev;
	LS_INIT(p);
}
