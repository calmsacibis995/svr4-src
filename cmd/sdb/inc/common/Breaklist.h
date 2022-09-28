/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Breaklist.h	1.3"
#ifndef Breaklist_h
#define Breaklist_h

#include	"Itype.h"
#include	"Avltree.h"
#include	"Machine.h"

class Breakpoint;
class Assoccmds;

class Breaklist: public Avltree {
public:
			Breaklist();
			~Breaklist();
	Breakpoint *	add( Iaddr, Assoccmds * = 0, int = 1 );
	int		remove( Iaddr );
	int		disable( Iaddr );
	int		enable( Iaddr );
	Breakpoint *	lookup( Iaddr );
};

class Breakpoint: public Avlnode {
	Iaddr		_addr;
	char		_oldtext[BKPTSIZE];
	short		_flags;
	Assoccmds *	_assoccmds;
	friend class	Breaklist;
	friend class	Process;
public:
			Breakpoint( Iaddr = 0, Assoccmds * = 0, int = 1 );
	Iaddr		addr();
	char *		oldtext();
	Assoccmds *	assoccmds();
	int		operator>( Avlnode & );
	int		operator<( Avlnode & );
	Breakpoint &	operator=( Breakpoint & );
	void		value_swap( Avlnode * );
	Avlnode *	makenode();
	int		set( char * );
};

#endif

// end of Breaklist.h

