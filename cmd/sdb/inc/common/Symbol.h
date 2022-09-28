/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Symbol.h	1.3"

#ifndef Symbol_h
#define Symbol_h

#include	"Attribute.h"

class	Evaluator;
class	TYPE;
class	Source;
class	Locdesc;
enum	Tag;

class Symbol {
	char *		namep;
	Attribute *	attrlist;
	Evaluator *	evaluator;
	unsigned long	ss_base;
	friend class	Symtab;
	friend class	Symtable;
	friend class	TYPE;
public:
			Symbol();
			Symbol ( Symbol & );

	Symbol &	operator=( Symbol & );
	int		operator==( Symbol &s )	{
				return  attrlist  == s.attrlist &&
					evaluator == s.evaluator &&
					ss_base   == s.ss_base; }
	int		operator!=( Symbol &s )	{
				return  !(*this == s); }
	Symbol		arc( Attr_name );
	Tag		tag( char * = 0 );
	Attribute *	attribute( Attr_name );
	char *		name();
	unsigned long	pc( Attr_name );
	int		source( Source & );
	int		type(TYPE&, Attr_name = an_type);
	int		locdesc(Locdesc&, Attr_name = an_location);
	void		null();
	int		isnull();
	void		dump( char * = 0 );

	Symbol		parent()	{	return arc(an_parent);	}
	Symbol		child()		{	return arc(an_child);	}
	Symbol		sibling()	{	return arc(an_sibling);	}
};

#endif

// end of Symbol.h

