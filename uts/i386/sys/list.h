/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_LIST_H
#define _SYS_LIST_H

#ident	"@(#)head.sys:sys/list.h	1.2.7.1"
/* Generic lists
 * Lists are circular and doubly-linked, with headers.
 * When a list is empty, both pointers in the header
 * point to the header itself.
 */

/* list element */
typedef struct ls_elt {
	struct ls_elt *ls_next;
	struct ls_elt *ls_prev;
	/* your ad in this space */
} ls_elt_t;

/* 
 * All take as arguments side effect-free pointers to list structures
 */
#define LS_ISEMPTY(listp)	\
	(((struct ls_elt *)(listp))->ls_next == (struct ls_elt *)(listp))
#define LS_INIT(listp) {			\
	((struct ls_elt *)(listp))->ls_next =	\
	((struct ls_elt *)(listp))->ls_prev =	\
	((struct ls_elt *)(listp));		\
}

#define LS_REMOVE(listp)	ls_remove((struct ls_elt *)(listp))

/* 
 * For these five, ptrs are to list elements, but qp and stackp are
 * implicitly headers.
 */
#define LS_INS_BEFORE(oldp, newp)	\
	ls_ins_before((struct ls_elt *)(oldp), (struct ls_elt *)(newp))
 
#define LS_INS_AFTER(oldp, newp)	\
	ls_ins_after((struct ls_elt *)(oldp), (struct ls_elt *)(newp))

#define LS_INSQUE(qp, eltp)	\
	ls_ins_before((struct ls_elt *)(qp), (struct ls_elt *)(eltp))

/* result needs cast; NULL result if empty queue
 */
#define LS_REMQUE(qp)		ls_remque((struct ls_elt *)(qp))

#define LS_PUSH(stackp, newp) \
	ls_ins_after((struct ls_elt *)(stackp), (struct ls_elt *)(newp))

/* result needs cast; NULL result if empty stack
 */
#define LS_POP(stackp)		ls_remque((struct ls_elt *)(stackp))

extern void ls_ins_before();
extern void ls_ins_after();
extern struct ls_elt *ls_remque();
extern void ls_remove();


#endif	/* _SYS_LIST_H */
