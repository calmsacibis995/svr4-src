/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Source.h	1.3"
#ifndef Source_h
#define Source_h

#include	"Itype.h"
#include	"Attribute.h"
#include	"Stmt.h"

struct	Lineinfo;

class Source {
	int		lastpcsub;
	Lineinfo *	lineinfo;
	Iaddr		ss_base;
	friend class	Symbol;
public:
			Source();
			Source( Source& );
	int		nostmts(){	return (lineinfo == 0);	}
	void		pc_to_stmt( Iaddr, long&, Iaddr* = 0, long* = 0, int = -1 );
	void		stmt_to_pc( long, Iaddr&, long* = 0, long* = 0, int = 0 );
	Stmt		first_stmt();
	void		next_stmt( Iaddr &, long &, long & );
	Source &	operator=( const Source & );
};

#endif

// end of Source.h

