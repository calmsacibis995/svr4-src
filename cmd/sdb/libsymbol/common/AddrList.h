/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/AddrList.h	1.2"
#ifndef AddrList_h
#define AddrList_h

#include	"Attribute.h"
#include	"Avltree.h"
#include	"Itype.h"

class AddrEntry : public Avlnode {
	Iaddr		loaddr,hiaddr;
	Attr_form	form;
	Attr_value	value;
	friend class	AddrList;
	friend class	Symtab;
	friend class	Evaluator;
public:
			AddrEntry();
			AddrEntry( AddrEntry & );
	int		operator<( Avlnode & );
	int		operator>( Avlnode & );
	Avlnode *	makenode();
	AddrEntry &	operator=( AddrEntry & );
};

enum Result;

class AddrList : public Avltree {
public:
			AddrList();
	AddrEntry *	add( Iaddr, Iaddr, long, Attr_form );
	void		complete();
};

#endif

// end of AddrList.h

