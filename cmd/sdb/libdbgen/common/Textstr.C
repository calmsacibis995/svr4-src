#ident	"@(#)sdb:libdbgen/common/Textstr.C	1.2"
/*
	Desc:	General purpose null-terminatec character string.
	Include:	Textstr.h
	Overview:	Textstr is a general purpose mechanism for storing
and manipulating null-terminated character strings that may be of any
arbitrary length.
The share many attributes with Bytestrs,
except they always end in an null-byte.
*/

#include	"Textstr.h"
#include	<memory.h>
#include	<string.h>

/**
	Initialized constructor.
	Given a pointer to a character array and a length
	create a Textstr of that content and length.
*/
Textstr ::
Textstr( char *cp, unsigned long len )
{
	tssp = new tss;
	if( cp[ len-1 ] != '\0' )
		len++;
	tssp->ptr = new char[ len ];
	memcpy( tssp->ptr, cp, len-1 );
	tssp->ptr[ len-1 ] = '\0';
	tssp->refcnt = 1;
}


/**
	Initialized constructor.
	Given a pointer to a null-byte terminated character array
	create a Textstr of that content, including the null-byte.
*/
Textstr ::
Textstr( char *cp )
{
	unsigned long	len = strlen( cp ) + 1;

	tssp = new tss;
	tssp->ptr = new char[ len ];
	memcpy( tssp->ptr, cp, len );
	tssp->refcnt = 1;
}


Textstr ::
~Textstr()
{
	if( tssp  &&  --(tssp->refcnt) == 0 ) {
		delete tssp->ptr;
		delete tssp;
	}
}


/**
	Assign a null-terminated character string to a Textstr.
	A copy of the string is made within the Textstr.
*/
Textstr&
Textstr ::
operator= ( char *cp )
{
	if( cp == tssp->ptr )
		return *this;
	if( tssp->refcnt > 1 ) {
		tssp->refcnt--;
		tssp = new tss;
	}
	else
		delete tssp->ptr;
	
	unsigned long	len;

	tssp->ptr = new char[ ( len = strlen( cp ) + 1 ) ];
	memcpy( tssp->ptr, cp, len );
	tssp->refcnt = 1;
	return *this;
}


/**
	Assign one Textstr to another.
	The target Textstr takes on the value of the source Textstr.
	The previous value of the target Textstr is lost.
*/
Textstr&
Textstr ::
operator= ( Textstr& t )
{
	if( t.tssp )
		t.tssp->refcnt++;
	if( tssp  &&  --(tssp->refcnt) == 0 ) {
		delete tssp->ptr;
		delete tssp;
	}
	tssp = t.tssp;
	return *this;
}


/**
	Concatenate Textstrs or a Textstr and a null-terminated string.
	Create a new space big enough to hold the concatenation, fill it,
	throw away the orignal space.
	Return the original Textstr object.
*/
Textstr &
Textstr ::
concat( char *cp )
{
	unsigned long	len1, len2;
	char		*np;

	len1 = strlen( tssp->ptr );
	len2 = strlen( cp ) + 1;
	np = new char[ len1 + len2 ];
	memcpy( np, tssp->ptr, len1 );
	memcpy( np + len1, cp, len2 );
	if( tssp->refcnt > 1 ) {
		tssp->refcnt--;
		tssp = new tss;
	}
	else
		delete tssp->ptr;
	
	tssp->ptr = np;
	tssp->refcnt = 1;
	return *this;
}


/*
	This is offered as an efficiency consideration.
	cp MUST be the result of a "new char[ length ]".
*/
Textstr&
Textstr ::
newcontent( char *cp )
{
	if( tssp->ptr == 0 ) {
		tssp->ptr = cp;
		tssp->refcnt = 1;
	}
	else {
		if( tssp->refcnt == 1 ) {
			delete tssp->ptr;
			tssp->ptr = cp;
		}
		else {
			tssp->refcnt--;
			tssp = new tss;
			tssp->ptr = cp;
			tssp->refcnt = 1;
		}
	}
	return *this;
}

Textstr
textfmt( char *fmt ... )
{
	va_list	args;
	char buf[4000];

	va_start( args, fmt );
	vsprintf( buf, fmt, args );
	va_end( args );
	char *p = new char[ strlen(buf) + 1 ];
	strcpy(p, buf);
	return Textstr().newcontent( p );
}
