#ident	"@(#)sdb:libdbgen/common/str.C	1.4"

// str() looks up the given string, and returns a pointer to the
// (unique) saved copy; if not found, it saves (with new) a copy
// of the string and returns it.  sf() is an sprintf() plus a str()
// of the result.  strn() looks up only the first "n" chars of its
// argument, and returns a pointer to a (unique) copy of them.
//
// str(0) returns a string with current statistics.
//
// Strings saved with str() must be considered read only!  They
// may be compared for equality by comparing their pointers.

#include <string.h>
#include <stdio.h>

#define SF_HASH 823
struct SF_CELL {
	SF_CELL *link;
	char     buf[1];		// should be [0] - cfront bug
};

char *str(char *x)
{
	static SF_CELL *Table[SF_HASH];
	static Calls, Strings, Worst, Bytes;
	register char *p;
	unsigned long len, h, i;
	register struct SF_CELL *s;

	if( !x ){
		static char report[128];
		sprintf( report, "strings=%d calls=%d worst=%d bytes=%d",
				 Strings,   Calls,   Worst,    Bytes );
		return report;
	}

	++Calls;	
	
	h = 0;
	for( len = 0, p = x; *p; )
		h += (*p++) << (++len%4);
	h %= SF_HASH;

	for( s=Table[h],i=1; s; s=s->link,++i )
		if(!strcmp(x,s->buf))
			return s->buf;			// found it

	++Strings;

	if( i>Worst ) Worst = i;

	len = (len+4+sizeof(SF_CELL*)) / 4 * 4;		/* vax */

	s = (SF_CELL*) new char [len];			// allocate new str
	if( !s )
		return "str(): out of memory";

	Bytes += len;

	s->link = Table[h];				// link in at head
	Table[h] = s;

	strcpy( s->buf, x );				// copy bytes

	return s->buf;
}

char *sf(char *f ... )		// up to 9 args after the format
{
	int *a = (int*)&f;
	char x[1024];		// result must not be longer than 1024 bytes!

	if ( !f ) {
		return str(0);	// sf(0) returns report string also
	}

	sprintf( x, f, a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9] );

	return str( x );
}

char *strn(char *s, int n)
{
	char *buf = new char[ n + 1 ];
	strncpy( buf, s, n );
	buf[n] = '\0';

	char *result = str( buf );

	delete buf;

	return result;
}
