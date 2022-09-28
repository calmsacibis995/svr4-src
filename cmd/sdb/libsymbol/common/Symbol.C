#ident	"@(#)sdb:libsymbol/common/Symbol.C	1.8"
#include	"Symbol.h"
#include	"Source.h"
#include	"Evaluator.h"
#include	"Locdesc.h"
#include	"Interface.h"
#include	"Tag.h"
#include	"TYPE.h"
#include	<string.h>

Symbol::Symbol()
{
	namep = 0;
	attrlist = 0;
	evaluator = 0;
	ss_base = 0;
}

Symbol::Symbol( Symbol & symbol )
{
	namep = symbol.namep;
	attrlist = symbol.attrlist;
	evaluator = symbol.evaluator;
	ss_base = symbol.ss_base;
}

Symbol&
Symbol::operator=( const Symbol & symbol )
{
	namep = symbol.namep;
	attrlist = symbol.attrlist;
	evaluator = symbol.evaluator;
	ss_base = symbol.ss_base;
	return *this;
}

void
Symbol::null()
{
	namep = 0;
	attrlist = 0;
	evaluator = 0;
	ss_base = 0;
}

int
Symbol::isnull()
{
	return ( attrlist == 0 );
}

char *
Symbol::name()
{
	Attribute *	a;

	if ( namep != 0 )
	{
		return namep;
	}
	else if ( a = evaluator->attribute( attrlist, an_name )  )
	{
		namep = (char*)a->value.ptr;
	}
	else
	{
		namep = 0;
	}
	return namep;
}

Symbol
Symbol::arc( Attr_name attrname )
{
	Symbol		symbol;
	Attribute *	a;

	if ( (a = evaluator->arc(attrlist,attrname)) != 0 )
	{
		symbol.attrlist = (Attribute*) a->value.ptr;
		symbol.evaluator = evaluator;
		symbol.ss_base = ss_base;
	}
	return symbol;
}

Attribute *
Symbol::attribute( Attr_name attrname )
{
	Attribute *	a;

	if ( evaluator == 0 )
	{
		return 0;
	}
	else if ( a = evaluator->attribute( attrlist, attrname )  )
	{
		return a;
	}
	else
	{
		return 0;
	}
}

unsigned long
Symbol::pc( Attr_name attr_name )
{
	Attribute	* a;

	if ( (a = attribute(attr_name)) == 0 )
	{
		return ~0;
	}
	else
	{
		return a->value.word + ss_base;
	}
}

int
Symbol::source( Source & s )
{
	Attribute	* a;

	if ( (a = attribute(an_lineinfo)) == 0 )
	{
		s.lineinfo = 0;
		s.ss_base = 0;
	}
	else
	{
		s.lineinfo = (Lineinfo *) a->value.ptr;
		s.ss_base = ss_base;
	}
	return 1;
}

int
Symbol::type(TYPE& t, Attr_name attr)
{
	Attribute *a;
	Symbol s(*this);

	if ((a = attribute(attr)) == 0) {
		t.null();
		return 0;
	}
	switch (a->form)
	{
		case af_fundamental_type:
			t = Fund_type(a->value.word);
			break;
		case af_symbol:
			s.namep = 0;
			s.attrlist = (Attribute *)a->value.ptr;
			t = s;
			break;
		default:
			return 0;
	}
	return 1;
}

int
Symbol::locdesc(Locdesc & desc, Attr_name attr)
{
	Attribute *a;

	if ((a = attribute(attr)) == 0)
	{
		desc.clear();
		return 0;
	}
	switch (a->form)
	{
		case af_locdesc:
			desc = Addrexp(a->value.ptr);
			desc.adjust( ss_base );
			return 1;
		default:
			return 0;
	}
}

Tag
Symbol::tag(char *msg)
{
	register Attribute *a = attribute(an_tag);

	if (a != 0 && a->form == af_tag && a->value.word != 0)
	{
		return a->value.word;
	}
	if (msg)
	{
		printe("No Tag: %s symbol.parent() is %s\n", msg, parent().name());
	}
	return t_none;
}
