/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Coffbuild.h	1.2"
#ifndef Coffbuild_h
#define Coffbuild_h

#include	"Coff.h"
#include	"Fetalrec.h"
#include	"Itype.h"
#include	"Reflist.h"
#include	"Fetalline.h"

struct Syminfo;

class Coffbuild {
	Coff		coff;
	Fetalrec	fetalrec;
	Fetalrec	fetaltype;
	Fetalline	fetalline;
	Reflist		reflist;
	long		nextofs;
	long		linedisp;
	long		file_offset;
	long		global_offset;
	long		string_offset;
	struct syment	sym;
	union auxent	aux;
	char *		name;
	long		past_ef( long );
	void		find_arcs( long &, long & );
	void		get_arcs();
	void		get_data();
	void		get_type_C();
	void		get_type();
	void		get_addr_C_3B();
	void		get_addr_C();
	void		get_addr();
	void *		make_chunk( void *, int );
	char *		make_string( char * );
	int		find_record( long, int );
	void		get_lineinfo( long, long, long );
	char *		get_fcn_lineinfo( char *, char * );
public:
			Coffbuild( int );
	Attribute *	make_record( long, int );
	int		get_syminfo( long, Syminfo & );
	long		first_symbol();
	long		first_global();
	Lineinfo *	line_info( long, long );
	void		get_pc_info( long, Iaddr&, Iaddr& );
	void		cache( long, long );	// offset, size
	void		cache_globals();
	void		de_cache();
};

#endif

// end of Coffbuild.h

