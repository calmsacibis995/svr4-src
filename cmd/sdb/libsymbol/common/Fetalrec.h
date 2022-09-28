/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Fetalrec.h	1.1"
#ifndef Fetalrec_h
#define Fetalrec_h

#include	"Attribute.h"
#include	"Vector.h"

class Fetalrec {
	Vector		vector;
	int		count;
public:
			Fetalrec();
			~Fetalrec()	{}
	Fetalrec &	add_attr(Attr_name, Attr_form, long );
	Fetalrec &	add_attr(Attr_name, Attr_form, void * );
	Fetalrec &	add_attr(Attr_name, Attr_form, const Attr_value & );
	Attribute *	put_record();
};

#endif

// end of Fetalrec.h

