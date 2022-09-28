/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Fetalline.h	1.1"
#ifndef Fetalline_h
#define Fetalline_h

#include	"Vector.h"
#include	"Itype.h"

struct Lineinfo;

class Fetalline {
	Vector		vector;
	int		count;
public:
			Fetalline() {	count = 0;	}
			~Fetalline()	{}
	Fetalline &	add_line( Iaddr, long );
	Lineinfo *	put_line( Iaddr );
};

#endif

// end of Fetalline.h

