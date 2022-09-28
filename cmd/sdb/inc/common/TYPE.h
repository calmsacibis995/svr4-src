/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/TYPE.h	1.3"

#ifndef TYPE_h
#define TYPE_h

#include "Symbol.h"

enum Stype;

enum Type_form {
    TF_fund,  // char, short, int, unsigned int, ...
    TF_user   // ptr, array, struct, enum, ...
};
enum Fund_type;

class TYPE {
    Type_form _form;
    Fund_type ft;     // meaningful iff form == TF_fund.
    Symbol    symbol; // meaningful iff form == TF_user.
public:

    void null();      // make null.
    int  isnull();    // is null ?
    TYPE()  { null(); }

    TYPE(TYPE&);		// bit copy. (needed because of cfront bug)
    TYPE& operator=(TYPE&);	// bit copy. (needed because of cfront bug)

    TYPE& operator=(Fund_type);       // init as a fundamental type.
    TYPE& operator=(Symbol&);         // init as a user defined type.

    Type_form form() { return _form; }
    int       fund_type(Fund_type&);  // read type; return 1 iff form TF_fund.
    int       user_type(Symbol&);     // read type; return 1 iff form TF_user.

    int size();			  // size in bytes
    int get_Stype(Stype&);	  // map to machine type or fail.
    int is_ptr();		  // true iff type is pointer type.

    int deref_type(TYPE&, Tag * = 0);

    void dump(char* = 0); // see libsymbol/.../dbtools*.C
};

#endif
