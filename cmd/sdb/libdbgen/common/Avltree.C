#ident	"@(#)sdb:libdbgen/common/Avltree.C	1.4"
#include	"Avltree.h"

static Avlnode* leftmost;
static Avlnode* rightmost;

//
//	Avltree routines
//

Avlnode*
Avltree::tinsert(Avlnode & node)
{
	Avlnode* ret;

	leftmost = rightmost = 0;
	if (root == 0) {
		root = node.makenode();
		firstnode = lastnode = root;
		return root;
	}
		
	ret = root->tinsert(&root, &node);
	if (leftmost)
		firstnode = leftmost;
	if (rightmost)
		lastnode = rightmost;
	return ret;
}

Avlnode*
Avltree::tlookup(Avlnode & node)
{
	if (root)
		return root->tlookup(&node);

	return (Avlnode*) 0;
}

int
Avltree::tdelete(Avlnode & node)
{
	int ret;
	int balance = 0;

	leftmost = rightmost = 0;
	if (root) {
		if (firstnode == lastnode) {
			ret = root->tdelete(&root, &node, &balance);
			if (ret) {
				firstnode = lastnode = 0;
				root = 0;
			}
			return ret;
		}
		ret = root->tdelete(&root, &node, &balance);
		if (leftmost)
			firstnode = leftmost;
		if (rightmost)
			lastnode = rightmost;
		return ret;
	}

	return 0;
}

void
Avltree::tdestroy()
{
	if (root) {
		if (root->leftchild)
			root->leftchild->tdestroy();
		if (root->rightchild)
			root->rightchild->tdestroy();
		delete root;
		root = 0;
		firstnode = lastnode = 0;
	}
}

//
//	Avlnode constructor
//

Avlnode::Avlnode()
{
	leftchild = rightchild = 0;
	nextnode = prevnode = 0;
	bal = 0;
}

Avlnode *
Avlnode::makenode()
{
	return new Avlnode;
}

void
Avlnode::value_swap(Avlnode* n)
{
}

int
Avlnode::operator>( Avlnode & )
{
	return 1;
}

int
Avlnode::operator<( Avlnode & )
{
	return 1;
}

Avlnode*
Avlnode::tinsert(Avlnode** parent, Avlnode* node)
{
	int	balance = 0;

	return (*parent)->sprout(parent, node, &balance);
}


Avlnode *
Avlnode::tlookup(Avlnode* node)
{
	if (node) {
		if (*this < *node) {
			if (rightchild)
				return rightchild->tlookup(node);
			else
				return (Avlnode*) 0;
		}
		else if (*this > *node) {
			if (leftchild)
				return leftchild->tlookup(node);
			else
				return (Avlnode*) 0;
		}
		else
			return this;
	}
	return (Avlnode*) 0;
}


int
Avlnode::tdelete(Avlnode** parent, Avlnode* node, int* balance)
{
	Avlnode* p1;
	int ret;

	if (node == 0)
		return 0;

	if (*this < *node) {
		if (rightchild)
			ret = rightchild->tdelete(&rightchild, node, balance);
		else
			ret = 0;
		if (*balance)
			(*parent)->balanceR(parent, balance);
	}
	else if (*this > *node) {
		if (leftchild)
			ret = leftchild->tdelete(&leftchild, node, balance);
		else
			ret = 0;
		if (*balance)
			(*parent)->balanceL(parent, balance);
	}
	else {
		p1 = *parent;
		if (p1->rightchild == 0) {
			*parent = p1->leftchild;
			*balance = 1;
		}
		else if (p1->leftchild == 0) {
			*parent = p1->rightchild;
			*balance = 1;
		}
		else {
			this->del(&(p1->leftchild), &p1, balance);
			if (*balance)
				(*parent)->balanceL(parent, balance);
		}
		// actual deletion of node
		// adjust prevnode and nextnode
		if (p1->prevnode)
			p1->prevnode->nextnode = p1->nextnode;
		else
			leftmost = p1->nextnode;
		if (p1->nextnode)
			p1->nextnode->prevnode = p1->prevnode;
		else
			rightmost = p1->prevnode;
		delete p1;
		ret = 1;
	}
	return ret;
}

void
Avlnode::tdestroy()
{
	if (leftchild)
		leftchild->tdestroy();
	if (rightchild)
		rightchild->tdestroy();
	delete this;
}

void
Avlnode::del(Avlnode** parent, Avlnode** node, int* balance)
{
	if ((*parent)->rightchild) {
		this->del(&(*parent)->rightchild, node, balance);
		if (*balance)
			(*parent)->balanceR(parent, balance);
	}
	else {
		(*node)->value_swap(*parent);
		*node = *parent;
		*parent = (*parent)->leftchild;
		*balance = 1;
	}
}

void
Avlnode::balanceL(Avlnode** parent, int* balance)
{
	Avlnode*	p1;
	Avlnode*	p2;
	int		b1;
	int		b2;

	switch (bal) {

	case -1:
		bal = 0;
		break;

	case 0:
		bal = 1;
		*balance = 0;
		break;

	case 1:
		p1 = rightchild;
		b1 = p1->bal;
		if (b1 >= 0) {
			rightchild = p1->leftchild;
			p1->leftchild = this;
			if (b1 == 0) {
				bal = 1;
				p1->bal = -1;
				*balance = 0;
			}
			else {
				bal = 0;
				p1->bal = 0;
			}
			*parent = p1;
		}
		else {
			p2 = p1->leftchild;
			b2 = p2->bal;
			p1->leftchild = p2->rightchild;
			p2->rightchild = p1;
			rightchild = p2->leftchild;
			p2->leftchild = this;
			if (b2 == 1)
				bal = -1;
			else
				bal = 0;
			if (b2 == -1)
				p1->bal = 1;
			else
				p1->bal = 1;
			*parent = p2;
			p2->bal = 0;
		}
				
	}
}

void
Avlnode::balanceR(Avlnode** parent, int* balance)
{
	Avlnode*	p1;
	Avlnode*	p2;
	int		b1;
	int		b2;

	switch (bal) {

	case 1:
		bal = 0;
		break;

	case 0:
		bal = -1;
		*balance = 0;
		break;

	case -1:
		p1 = leftchild;
		b1 = p1->bal;
		if (b1 <= 0) {
			leftchild = p1->rightchild;
			p1->rightchild = this;
			if (b1 == 0) {
				bal = -1;
				p1->bal = 1;
				*balance = 0;
			}
			else {
				bal = 0;
				p1->bal = 0;
			}
			*parent = p1;
		}
		else {
			p2 = p1->rightchild;
			b2 = p2->bal;
			p1->rightchild = p2->leftchild;
			p2->leftchild = p1;
			leftchild = p2->rightchild;
			p2->rightchild = this;

			if (b2 == -1)
				bal = 1;
			else
				bal= 0;
			if (b2 == 1)
				p1->bal = -1;
			else
				p1->bal = 0;
			*parent = p2;
			p2->bal = 0;
		}
	}
}

Avlnode*
Avlnode::sprout(Avlnode** parent, Avlnode* node, int* balance)
{
	Avlnode*	p1;
	Avlnode*	p2;
	Avlnode*	ret;

	if (*this > *node) {
		if (leftchild)
			ret = leftchild->sprout(&leftchild, node, balance);
		else {
			ret = leftchild = node->makenode();

			if (prevnode) {
				leftchild->prevnode = prevnode;
				prevnode->nextnode = leftchild;
				prevnode = leftchild;
			}
			else {
				leftmost = leftchild;
				prevnode = leftchild;
			}
			leftchild->nextnode = this;
			*balance = 1;
		}
		if (*balance) {
			switch ((*parent)->bal) {

			case 1:	// right branch was longer
				(*parent)->bal = 0;
				*balance = 0;
				break;

			case 0: // balance was okay
				(*parent)->bal = -1;
				break;

			case -1: // left branch was already too long
				p1 = (*parent)->leftchild;
				if (p1->bal == -1) { // LL
					(*parent)->leftchild = p1->rightchild;
					p1->rightchild = *parent;
					(*parent)->bal = 0;
					*parent = p1;
				}
				else { // LR
					p2 = p1->rightchild;
					p1->rightchild = p2->leftchild;
					p2->leftchild = p1;
					(*parent)->leftchild = p2->rightchild;
					p2->rightchild = *parent;

					if (p2->bal == -1)
						(*parent)->bal = 1;
					else
						(*parent)->bal = 0;

					if (p2->bal == 1)
						p1->bal = -1;
					else
						p1->bal = 0;
					*parent = p2;
				}
				(*parent)->bal = 0;
				*balance = 0;
				break;
			}
		}
		return ret;
	}

	if (*this < *node) {
		if (rightchild)
			ret = rightchild->sprout(&rightchild, node, balance);
		else {
			ret = rightchild = node->makenode();
			if (nextnode) {
				rightchild->nextnode = nextnode;
				nextnode->prevnode = rightchild;
				nextnode = rightchild;
			}
			else {
				rightmost = rightchild;
				nextnode = rightchild;
			}
			rightchild->prevnode = this;

			*balance = 1;
		}
		if (*balance) {

			switch((*parent)->bal) {

			case -1:
				(*parent)->bal = 0;
				*balance = 0;
				break;

			case 0:
				(*parent)->bal = 1;
				break;

			case 1:
				p1 = (*parent)->rightchild;
				if (p1->bal == 1) { // RR
					(*parent)->rightchild = p1->leftchild;
					p1->leftchild = *parent;
					(*parent)->bal = 0;
					*parent = p1;
				}
				else { //  RL
					p2 = p1->leftchild;
					p1->leftchild = p2->rightchild;
					p2->rightchild = p1;
					(*parent)->rightchild = p2->leftchild;
					p2->leftchild = *parent;

					if (p2->bal == 1)
						(*parent)->bal = -1;
					else
						(*parent)->bal = 0;

					if (p2->bal == -1)
						p1->bal = 1;
					else
						p1->bal = 0;
					
					*parent = p2;
				}
				(*parent)->bal = 0;
				*balance = 0;
			}
		}
		return ret;
	}
	// it was already here
	return (Avlnode*) 0;
}
