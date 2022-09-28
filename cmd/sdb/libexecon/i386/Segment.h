/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libexecon/i386/Segment.h	1.3"
#ifndef Segment_h
#define Segment_h

#include	"Itype.h"
#include	"Link.h"
#include	"Symtab.h"
#include	"oslevel.h"

class Segment: public Link {
	Iaddr		loaddr,hiaddr;
	Key		access;
	long		base;
	char *		pathname;
	int		is_text;
	Symtab		sym;
	Segment *	next()	{	return (Segment*)Link::next();	}
	friend class	Seglist;
public:
			Segment( Key, char *, Iaddr, long, long, long, int );
			~Segment()	{	unlink();	}

	int		read( Iaddr, void *, int );
	int		write( Iaddr, void *, int );
	int		read( Iaddr, Stype, Itype & );
	int		write( Iaddr, Stype, const Itype & );

	int		get_symtable( Key );
};

int 	stype_size( Stype );
#endif

// end of Segment.h

