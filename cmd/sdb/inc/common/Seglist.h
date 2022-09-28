/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Seglist.h	1.13"
#ifndef Seglist_h
#define Seglist_h

#include	"Itype.h"
#include	"Link.h"
#include	"Symbol.h"
#include	"oslevel.h"

class Core;
class Segment;
class Symnode;
class Symtab;
class NameEntry;

#define	LD_SO_BUFSIZ	128

class Seglist: private Link {
	Segment *	mru_segment;
	Symbol		current_file;
	Symnode *	segment_file;
	Symnode *	segment_global;
	Iaddr		r_debug_addr;
	Iaddr		rtld_addr;
	Iaddr           stack_hi, stack_lo;
	Link		symlist;
	Process *	proc;
	char		ld_so[LD_SO_BUFSIZ];
	Iaddr		find_r_debug( char *, Key );
	int		add( int, Key, char *, int, Iaddr, Segment * );
	int		add_static_shlib( int, Key, int );
	int		add_dynamic_text( int );
	int		add_symnode( char * , Iaddr );
	Segment *	next()	{ return (Segment*)Link::next();	}
	int		build_dynamic( Key );
	int		build_static( Key );
	Iaddr		rtld_base( Key );
public:
			Seglist( Process *);
			~Seglist();
	int		setup( int fd, int &rtl_used );
	int		buildable( Key );
	int		build( Key );
	int		rtl_used();
	Iaddr		rtl_addr( Key );
	int		readproto( int, int, Core *, int );
	Symtab *	find_symtab( Key, Iaddr );
	Segment *	find_segment( Iaddr );
	int		find_source( Key, char * , Symbol & );
	NameEntry *	first_global( Key );
	NameEntry *	next_global( Key );
	Symbol		first_file( Key );
	Symbol		next_file( Key );
	Symbol		find_global( Key, char * );
	int		print_map();
	int		get_brtbl( Key, char * );
	int		has_stsl();
	int		ptr_type( Key, Fund_type, Symbol & );
	int		in_stack( Iaddr );
	int		in_text( Iaddr );
	void		update_stack( Key );
};

#endif

// end of Seglist.h

