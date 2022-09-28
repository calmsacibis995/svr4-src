/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Cache.h	1.1"
#ifndef	Cache_h
#define	Cache_h

class Cache {
	int		fd;
	long		lo_cache;
	long		hi_cache;
	char *		cache_ptr;
	long		cache_size;
public:
			Cache( int );		// file descriptor
			~Cache();
	int		fill( long, long );	// offset, size
	void *		get( long, long );	// offset, size
	char *		get_string( long );	// offset
	void		empty();
	char *		ptr();
};
#endif

// end of Cache.h

