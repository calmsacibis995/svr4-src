#ident	"@(#)sdb:libsymbol/common/Fetalrec.C	1.1"
#include	"Fetalrec.h"
#include	<string.h>
#include	<malloc.h>

Fetalrec::Fetalrec()
{
	Attribute	attribute;

	attribute.name = an_count;
	attribute.form = af_int;
	attribute.value.word = 0;
	vector.add(&attribute,sizeof(Attribute));
	count = 1;
}

Fetalrec &
Fetalrec::add_attr(Attr_name name, Attr_form form, long word )
{
	Attribute	attribute;

	attribute.name = name;
	attribute.form = form;
	attribute.value.word = word;
	vector.add(&attribute,sizeof(Attribute));
	count++ ;
	return * this;
}

Fetalrec &
Fetalrec::add_attr(Attr_name name, Attr_form form, void * ptr )
{
	Attribute	attribute;

	attribute.name = name;
	attribute.form = form;
	attribute.value.ptr = ptr;
	vector.add(&attribute,sizeof(Attribute));
	count++ ;
	return * this;
}

Fetalrec &
Fetalrec::add_attr(Attr_name name, Attr_form form, const Attr_value & value )
{
	Attribute	attribute;

	attribute.name = name;
	attribute.form = form;
	attribute.value = value;
	vector.add(&attribute,sizeof(Attribute));
	count++ ;
	return * this;
}

Attribute *
Fetalrec::put_record()
{
	Attribute *	x;
	Attribute	attribute, *p;

	attribute.name = an_nomore;
	attribute.form = af_none;
	attribute.value.word = 0;
	vector.add(&attribute,sizeof(Attribute));
	count++ ;
	p = (Attribute *) vector.ptr();
	p->value.word = count;
	x = (Attribute *)::malloc(vector.size());
	::memcpy((char*)x,(char*)vector.ptr(),vector.size());
	vector.clear();	
	attribute.name = an_count;
	attribute.form = af_int;
	attribute.value.word = 0;
	vector.add(&attribute,sizeof(Attribute));
	count = 1;
	return x;
}
