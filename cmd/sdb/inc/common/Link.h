/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Link.h	1.1"

#ifndef LINK_H
#define LINK_H

// Link -- a doubly-linked circular list, possibly with a distinguished head.
//
// Singletons are linked to themselves.  "head->append(x)" places
// (the list) x immediately after head.  "head->prepend(x)" places
// x immediately before head, which is to say, at the tail of the list.
// One member of any list may be distinguished by the "_ishead" flag.
// The "next()" and "prev()" operations pretend that the list is non-
// circular, by examining the "_ishead" flag.  If you wish next() and
// prev() to ignore the distinction, you may use "Next()" and "Prev()",
// or else not set "_ishead" in any member of the list.

class Link {
	Link 	*_next;
	Link	*_prev;
	int	_ishead;
public:
	Link(int i = 0)	{ _next = this; _prev = this; _ishead = i; }
int	is_head()	{ return _ishead; }
Link   *Next()	{ return _next; }
Link   *Prev()	{ return _prev; }
Link   *next()  { return (_next && _next->_ishead) ? 0 : _next; }
Link   *prev()	{ return (_prev && _prev->_ishead) ? 0 : _prev; }
Link   *append(Link *);
Link   *prepend(Link *);
Link   *unlink();	// OK to pass a NULL this
};

class Linkitem : public Link {
public:
	  void *item;	    // pointer to real data
	  Linkitem(int i = 0):(i) {}
Linkitem *next()	{ return (Linkitem *) Link::next(); }
Linkitem *prev()	{ return (Linkitem *) Link::prev(); }
Linkitem *Next()	{ return (Linkitem *) Link::Next(); }
Linkitem *Prev()	{ return (Linkitem *) Link::Prev(); }
Linkitem *append(Link *p)	{ return (Linkitem *) Link::append(p); }
Linkitem *prepend(Link *p)	{ return (Linkitem *) Link::prepend(p); }
Linkitem *unlink()	{ return (Linkitem *) Link::unlink(); }
};

// Queue -- a deque implemented by Links
//
// Usage: Queue q; Link *l;  q.put(l);  if ( q.is_empty() ) ...  l = q.get();
//        q.unget(l);    // puts l back on head of queue
//	  l = q.unput(); // gets l from tail of queue

class Queue : private Link {	// not a public derivation;
				// Link operations are not avail
public:
	Queue():(1) {}	// creating a Queue creates a list head
int	is_empty()	{ return !next(); }
void	put( Link *p )	{ prepend( p ); }
Link   *get()		{ return next()->unlink(); }
void	unget( Link *p ){ append( p ); }
Link   *unput()		{ return prev()->unlink(); }
};

// Stack -- a stack implemented by Links

class Stack : private Link {	// not a public derivation;
				//Link operations are not avail
public:
	Stack():(1) {}	// creating a Stack creates a list head
int	is_empty()	{ return !next(); }
void	push( Link *p )	{ append( p ); }
Link   *pop()		{ return next()->unlink(); }
};


#endif /* LINK_H */
