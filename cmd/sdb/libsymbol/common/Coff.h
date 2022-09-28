/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Coff.h	1.1"
#ifndef	Coff_h
#define	Coff_h
/*
	Common Object File Format access routines.
*/

#include	"Cache.h"
#include	"Itype.h"
#include	<stdio.h>
#include	<a.out.h>

class Coff {
	int		fd;
	int		got_header, got_line_offset;
	long		line_offset;
	long		sym_offset;
	long		str_offset;
	struct filehdr	header;
	Cache		line_cache;
	Cache		symbol_cache;
	Cache		name_cache;
	int		get_header();
public:
			Coff( int );
			~Coff();
	long		first_symbol();
	long		line_info();
	long		get_symbol( long, struct syment &, union auxent & );
	char *		get_name( struct syment );
	char *		get_lineno( long, long );
	int		cache( long, long );
	int		sectno( char * );
	void		de_cache();
};
#endif

// end of Coff.h

