/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)sdb:inc/common/Avltree.h	1.2.1.1"

#ifndef Avltree_h
#define Avltree_h

//
//	Threaded Avl tree package
//

class Avlnode {

friend class Avltree;

	Avlnode*		leftchild;
	Avlnode*		rightchild;
	Avlnode*		nextnode;
	Avlnode*		prevnode;
	short			bal;

	void			balanceL(Avlnode** parent, int* balance);
	void			balanceR(Avlnode** parent, int* balance);
	void			del(Avlnode** parent, Avlnode** node,
					int* balance);

	Avlnode*		sprout(Avlnode** parent, Avlnode* node,
					int* balance);

public:
	virtual Avlnode*	makenode();	// save this in tree memory.
	virtual void		value_swap(Avlnode*);
	virtual int		operator<(Avlnode & node);
	virtual int		operator>(Avlnode & node);
	
	Avlnode*		next() { return nextnode; }
	Avlnode*		prev() { return prevnode; }

	Avlnode*		tinsert(Avlnode** parent, Avlnode* node);
	Avlnode*		tlookup(Avlnode* node);
	int			tdelete(Avlnode** parent, Avlnode* node,
					int* balance);
	void			tdestroy();

	Avlnode();
};

class Avltree {

	Avlnode*		firstnode;
	Avlnode*		lastnode;

public:

	Avlnode*		root;

	Avlnode*		tfirst() { return firstnode; }
	Avlnode*		tlast() { return lastnode; }

	Avlnode*		tinsert(Avlnode & node);
	Avlnode*		tlookup(Avlnode & node);
	int			tdelete(Avlnode & node);
	void			tdestroy();

	Avltree()		{ root = firstnode = lastnode = 0; }
	~Avltree()		{ if (root) root->tdestroy(); }

};



#endif

// end of Avltree.h
