#ident	"@(#)sdb:libsymbol/common/NameList.C	1.4"
#include	"Attribute.h"
#include	"builder.h"
#include	"NameList.h"
#include	<string.h>
#include	<malloc.h>

NameEntry::NameEntry()
{
	namep = 0;
	form = af_none;
	value.word = 0;
}

NameEntry::NameEntry( NameEntry & name_entry )
{
	namep = name_entry.namep;
	form = name_entry.form;
	value = name_entry.value;
}

// Make a NameEntry instance
Avlnode *
NameEntry::makenode()
{
	char *	s;

	s = ::malloc(sizeof(NameEntry));
	::memcpy(s,(char*)this,sizeof(NameEntry));
	return (Avlnode*)s;
}

int
NameEntry::operator>( Avlnode & t )
{
	NameEntry &	name_entry = *(NameEntry*)(&t);

	return ( ::strcmp(namep,name_entry.namep) > 0 );
}

int
NameEntry::operator<( Avlnode & t )
{
	NameEntry &	name_entry = *(NameEntry*)(&t);

	return ( ::strcmp(namep,name_entry.namep) < 0 );
}

NameEntry &
NameEntry::operator=( NameEntry & name_entry )
{
	namep = name_entry.namep;
	form = name_entry.form;
	value = name_entry.value;
	return *this;
}

void
NameEntry::newname( char * s )
{
	int	len;

	len = ::strlen(s)+1;
	namep = ::malloc( len );
	::memcpy(namep,s,len);
}

NameList::NameList() {}

NameEntry *
NameList::add( char * s, void * ptr )
{
	NameEntry	node;
	Avlnode *	t;

	node.newname(s);
	node.form = af_symbol;
	node.value.ptr = ptr;
	t = tinsert(node);
	return (NameEntry*)t;
}

NameEntry *
NameList::add( char * s, long w, Attr_form form )
{
	NameEntry	node;
	Avlnode *	t;

	node.newname(s);
	node.form = form;
	node.value.word = w;
	t = tinsert(node);
	return (NameEntry*)t;
}
