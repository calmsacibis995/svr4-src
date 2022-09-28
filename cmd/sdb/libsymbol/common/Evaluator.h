/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Evaluator.h	1.3"
#ifndef Evaluator_h
#define Evaluator_h

#include	"Bdibuild.h"
#include	"Coffbuild.h"
#include	"Elfbuild.h"
#include	"NameList.h"
#include	"AddrList.h"
#include	"Attribute.h"

enum ftyp;

class Evaluator {
	int		fdesc;
	enum ftyp	file_type;
	long		first_offset;
	Attribute *	first_record;
	long		next_disp;
	long		elf_disp;
	int		at_end;
	int		noglobals;
	int		no_elf_syms;
	NameEntry *	current_entry;
	Bdibuild	bdibuild;
	Coffbuild	coffbuild;
	Elfbuild	elfbuild;
	NameList	namelist;
	AddrList	addrlist;
	enum ftyp	get_file_type();
	Attribute *	add_node( Attribute * );
	Attribute *	add_parent( Attribute *, Attribute * );
	Attribute *	add_children( Attribute *, Attribute * );
	NameEntry *	get_global( char * );
	void		add_globals();
public:
			Evaluator( int );

	Attribute *	first_file();
	Attribute *	arc( Attribute *, Attr_name );

	Attribute *	evaluate( NameEntry * );
	Attribute *	attribute( Attribute *,  Attr_name );
	NameEntry *	first_global();
	NameEntry *	next_global();
	Attribute *	find_global( char * );
	Attribute *	lookup_addr( Iaddr );
};

#endif

// end of Evaluator.h

