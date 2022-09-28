/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/SectHdr.h	1.5"
#ifndef	SectHdr_h
#define	SectHdr_h
/*
	Access routines for section headers
*/

struct Seginfo {
        long            offset;
        unsigned long	vaddr;
	long		mem_size;
	long		file_size;
	int		loaded;
        int             executable;
        int             writable;
};

class SectHdr {
	int		fd;
	Seginfo *	seginfo;
	Seginfo *	get_elf_seginfo( int &, int & );
	Seginfo *	get_coff_seginfo( int &, int & );
	int		get_elf_sect( char *, long &, long &, long & );
	int		get_coff_sect( char *, long &, long &, long & );
public:
			SectHdr( int );		// file descriptor
			~SectHdr();
	int		getsect( char *, long &, long &, long & );
	Seginfo *	get_seginfo( int &, int & );	// count, shared
	int		find_symbol( char *, long & );
	int		print_map();
	char **		get_stsl_names(long, long);
	int		has_debug_info();
};

#endif

// end of SectHdr.h

