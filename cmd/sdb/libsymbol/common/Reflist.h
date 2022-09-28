/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Reflist.h	1.1"
#ifndef Reflist_h
#define Reflist_h

#include	"Avltree.h"
#include	"Attribute.h"

class Refnode : public Avlnode {
public:
	long			diskloc;
	Attribute *		nodeloc;
	friend class		Reflist;
				Refnode(long l)
				 { diskloc = l; nodeloc = 0; }
				Refnode(long l, Attribute * s)
				 { diskloc = l; nodeloc = s; }
// virtual functions
	Avlnode *		makenode();
	int			operator>( Avlnode & );
	int			operator<( Avlnode & );
};

class Reflist : public Avltree {
public:
				Reflist() { };
	int			lookup(long, Attribute * &);
	void			add(long, Attribute *);
	void			tdump();
};

#endif

// end of reflist.h


