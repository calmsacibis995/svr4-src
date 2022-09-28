/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Locdesc.h	1.2"
#ifndef Locdesc_h
#define Locdesc_h

#include	"Vector.h"
#include	"Wordstack.h"
#include	"Place.h"

enum LocOp;

typedef LocOp *	Addrexp;	// just a pointer

class Process;
class Frame;

class Locdesc {
	Vector		vector;
	Wordstack	stack;
	void		calculate_expr( Place &, Process *, Frame * );
public:
			Locdesc()	{}
			~Locdesc()	{}

	Locdesc &	clear();
	Locdesc &	add();
	Locdesc &	deref4();
	Locdesc &	reg( int );
	Locdesc &	basereg( int );
	Locdesc &	offset( long );
	Locdesc &	addr( unsigned long );

	Locdesc &	adjust( unsigned long );

	Addrexp		addrexp();
	int		size()	{ return vector.size();	}
	Locdesc &	operator=( Addrexp );

	Place		place( Process *, Frame * );
	Place		place( Process *, Frame *, Iaddr );

	int		basereg_offset( RegRef, long& );
};

#endif

// end of Locdesc.h
