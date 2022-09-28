/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Watchlist.h	1.2"
#ifndef Watchlist_h
#define Watchlist_h

#include	"Avltree.h"
#include	"Expr.h"
#include	"Itype.h"
#include	"Frame.h"
#include	"Place.h"
#include	"Rvalue.h"

class Watchpoint;

class Watchlist: public Avltree {
public:
			Watchlist();
			~Watchlist();
	Watchpoint *	add( Expr & );
	Watchpoint *	lookup( Expr & );
	int		remove( Expr & );
};

class Watchpoint: public Avlnode {
	char *		symname;
	Expr		expr;
	Place		lvalue;
	Rvalue		rvalue;
	FrameId		frameid;
	Iaddr		lopc,hipc;
	short		flags;
	friend class	Watchlist;
	friend class	Process;
public:
			Watchpoint( Expr & );
			~Watchpoint();
	int		operator>( Avlnode & );
	int		operator<( Avlnode & );
	Watchpoint &	operator=( Watchpoint & );
	void		value_swap( Avlnode * );
	Avlnode *	makenode();
	int		set_value( Frame * );
};

#endif

// end of Watchlist.h

