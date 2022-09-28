/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Bdi.h	1.3"
#ifndef	Bdi_h
#define	Bdi_h
/*
	Access routines for debugging information
*/

#include	<stdio.h>
#include	"SectHdr.h"

class Bdi {
	int		fd;
	long		eoffset;
	long		soffset;
	long		lo_entry, hi_entry;
	long		lo_line, hi_line;
	long		lo_cache, hi_cache;
	char *		cache_ptr;
	long		file_position;
	SectHdr		secthdr;
	int		getsect( char *, long &, long &, long & );
	int		readdata( long, void *, int );
	long		stmt_offset();
public:
			Bdi( int );		// file descriptor
			~Bdi();
	long		entry_offset();
	int		entry_info( long &, long & );
	int		stmt_info( long &, long & );
	int		cache( long, long );
	char *		get_entry( long );
	char *		get_lineno( long );
};

#endif

// end of Bdi.h

