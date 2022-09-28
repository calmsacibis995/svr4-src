/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Rvalue.h	1.4"

// Rvalue.h -- generic "R" values; objects which are the result of an
// expression evaluation.  Includes fundamental types, structs, unions,
// and arrays.
//
// The bytes are held in a Itype or Vector, and the type of the value in a TYPE.

#ifndef RVALUE_H
#define RVALUE_H

#include "Vector.h"
#include "TYPE.h"
#include "Itype.h"

class Obj_info;
class Process;

class Rvalue {
    TYPE    _type;
    Vector  raw_bytes;
    Stype   stype;	// valid if raw_bytes.size() == 0
    Itype   itype;	// valid if stype != SINVALID
    int	    rep;	// number of instances of object, if multiple

public:
    Rvalue() {}
    Rvalue(Stype, Itype &);
    Rvalue(void *, int, TYPE &, int rep = 1);
    Rvalue(Iaddr);
    Rvalue(Rvalue &);
    ~Rvalue();

    Rvalue& operator=(Rvalue &);
    int operator==(Rvalue &);
    int operator!=(Rvalue &v)  { return !(*this == v); }

    void null()		 { raw_bytes.clear(); _type.null(); }

    int  isnull()	 { return _type.isnull();   }

    TYPE& type()	 { return _type; }

    unsigned char *raw();

    long size();

    Stype get_Itype(Itype &);  // SINVALID if cannot get as Itype member.

    int print(char *label = 0, char *fmt = 0, char *sep = 0);
};

#endif
