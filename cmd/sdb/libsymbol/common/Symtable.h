/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Symtable.h	1.3"
#ifndef Symtable_h
#define Symtable_h

#include	"Symbol.h"
#include	"Source.h"
#include	"Type.h"
class Evaluator;
class AddrEntry;
class NameEntry;

class Symtable {
	Evaluator *	evaluator;
	int		evaluate( NameEntry * );
	int		evaluate( AddrEntry * );
	void		add_globals();
	Symbol		addr_lookup( Iaddr );
public:
			Symtable( int );
	Symbol		first_symbol();
	Symbol		find_scope ( Iaddr );
	Symbol		find_entry ( Iaddr );
	Symbol		find_global( char * );
	int 		find_source( Iaddr, Symbol & );
	int 		find_source( char *, Symbol & );
	NameEntry *	first_global();
	NameEntry *	next_global();
	Symbol		global_symbol( NameEntry * );
	Symbol		ptr_type( Fund_type );
};

#endif

// end of Symtable.h

