/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/NameList.h	1.2"
#ifndef NameList_h
#define NameList_h

#include	"Avltree.h"
#include	"Attribute.h"

class NameEntry : public Avlnode {
	char *		namep;
	Attr_form	form;
	Attr_value	value;
	void		newname( char * );
	friend class	Nameptr;
	friend class	Evaluator;
	friend class	NameList;
	friend class	Symtab;
	friend class	Symbol;
public:
			NameEntry();
			NameEntry( NameEntry & );
	int		operator<( Avlnode & );
	int		operator>( Avlnode & );
	Avlnode *	makenode();
	NameEntry &	operator=( NameEntry & );
	char *		name()	{	return namep;	}
};

enum Result;

class NameList : public Avltree {
public:
			NameList();
	NameEntry *	add( char *, long, Attr_form );
	NameEntry *	add( char *, void * );
};

#endif

// end of NameList.h

