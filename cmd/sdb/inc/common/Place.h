/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Place.h	1.2"

#ifndef PLACE_H
#define PLACE_H

#include "Itype.h"
#include "Reg.h"

/*
// NAME
//	Place (generalized address class)
//
// ABSTRACT
//	Place provides a generalized way to specify the location
//	in memory or register of a particular data item.
//
// DATA
//	kind		one of (pRegister, pAddress)
//	(union)		the actual register number or address 
//
// OPERATIONS
//	Basic comparison operators are provided to support
//	data structures based on ordered lists of Places.
//
*/

enum PlaceMark { // Note: enum values determine order of respective kinds.
    pRegister,
    pAddress,
    pUnknown
};

struct Place {
    PlaceMark kind;
    union {
	RegRef reg;
	Iaddr  addr;
    };
    void null()		{ kind = pUnknown; }
    int  isnull()	{ return kind == pUnknown; }
    Place()  		{ null(); }
    int  operator<(Place&);
    int  operator>(Place&);
    int  operator==(Place&);
};

#endif
