/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Wordstack.h	1.1"
#ifndef Wordstack_h
#define Wordstack_h

#include	"Vector.h"

class Wordstack {
	Vector		vector;
	int		count;
public:
			Wordstack()	{	count = 0;	}
			~Wordstack()	{}
	void		push( unsigned long );
	unsigned long	pop();
	unsigned long	item( int );
	int		not_empty()	{	return count>0;	}
};

#endif

// end of Wordstack.h

