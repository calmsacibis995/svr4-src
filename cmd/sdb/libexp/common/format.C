#ident	"@(#)sdb:libexp/common/format.C	1.8"

#include "format.h"
#include "Expr_priv.h"
#include "Process.h"
#include "Interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern Process *current_process;

overload dump;
extern void dump( Symbol, char * = 0 );
extern void dump( TYPE, char * = 0 );

int
format_bytes( Vector &raw, char *label, char* sep, char *fmt, TYPE & type )
{
//DBG	if ( debugflag & DBG_RVAL )
//DBG		printe("format_bytes( vec '%08x', '%s', '%s', '%s' )\n",
//DBG			*(long*)raw.ptr(), label, sep, fmt);
//DBG	dump( type, "format_bytes type" );

	int csmode = (sep == 0);	// call-stack mode, no newlines
	if ( csmode ) sep = "=";

	if ( sep[1] == ' ' )		// "x" command, RegAccess::display_regs
		csmode = 1;

	int count, length, explicit;
	char *mode = "x";
	int ret;
	unsigned char buf[1024];

	if ( !fmt ) fmt = default_fmt( type );

	if ( !parse_fmt( fmt, count, length, mode, explicit ) ) {
		// it parsed successfully in Expr::evaluate()!
		printe("parse_fmt('%s') failed in Rvalue::print()\n", fmt);
		return 0;
	}

	if ( !*mode )
		mode = default_fmt( type );

	unsigned long ul;
#define ul_byte(n)	((unsigned char *)&ul)[n]
	unsigned short us;
	unsigned char uc;
	double d;

	Symbol		sym;

	char *format = 0;

	switch( *mode ) {
	case 'd':	format = "%d";		break;
	case 'u':	format = "%u";		break;
	case 'o':	format = "%#o";		break;
	case 'x':	format = "%#x";		break;
	case 'f':	format = "%g";		break;
	case 'g':	format = "%g";		break;
	case '#':	format = "%#x";		break;
	case '\0':	format = "%#x";		break;
	}

	unsigned char *p = (unsigned char *)raw.ptr();
	int size = raw.size();
//DBG	if ( debugflag & DBG_RVAL )
//DBG		printe("raw.size() = %d, type.size() = %d\n", size, type.size());
	int rep = size / type.size();

	if ( label && sep )
		printx("%s%s", label, sep);

	switch ( *mode ) {
	case '#':
		long l = *(long*)p;
		if ( !type.user_type(sym) ) {
			return 0;
		} else if ( sym.tag() != t_enumtype ) {
			return 0;
		} else {
			Symbol mem;
			for ( mem = sym.child(); !mem.isnull() ; mem = mem.sibling() ) {
				if ( mem.tag() != t_enumlittype ) {
					printe("bad tag in enum list: %d\n", mem.tag());
					return 0;
				}
				Attribute *a = mem.attribute(an_litvalue);
				if ( !a ) {
					printe("missing an_litvalue attribute!\n");
					return 0;
				}
				if ( a->value.word == l ) {	// found it!
					a = mem.attribute(an_name);
					if ( !a ) {
						printe("missing an_name attribute!\n");
						return 0;
					}
					printx("%s", a->value.ptr);
					return 1;
				}
			}
//DBG			if ( debugflag & DBG_ENUM )
//DBG				printe("value %#x not found\n", l);
			// not found, fall back to hex format
			printx("%#x", l );
		}
		break;
	case 'c':
		if ( explicit && length ) { // explicit count and length
			length *= count;
		} else if ( explicit ) {    // explicit count, implicit length
			length = count;
		} else if ( length == 0 ) { // implicit count and length
			length = size;
		}
		if ( size < length ) {
			// need help from Expr!
			printx("size < length in format_bytes()!\n");
			return 0;
		}
		for ( ; length > 0 ; --length ) {
			printx("'%s'%s", fmt_str(p++, 1, 1),
				(length>1) ? " " : "");
		}
		break;
	case 'd':
	case 'u':
	case 'o':
	case 'x':
//DBG		if ( debugflag & DBG_RVAL )
//DBG			printe("length = %d, count = %d, size = %d, rep = %d\n",
//DBG				length, count, size, rep);
		if ( length == 0 )
			length = size / rep;
		if ( count * length > size ) {
			// need help from Expr!
			printx("count * length > size in format_bytes()!\n");
			return 0;
		}
		while ( count-- ) {
		    int len = length;
		    switch ( len ) {
		    case 1:
			ul = uc = *p++;
			if ( *mode == 'd' )
				ul = sext( ul, len );
			printx(format, ul);
			break;
		    case 2:
			ul = us = *(unsigned short*)p;
			p += 2;
			if ( *mode == 'd' )
				ul = sext( ul, len );
			printx(format, ul);
			break;
		    case 4:
			ul = *(unsigned long*)p;
			p += 4;
			if ( *mode == 'd' )
				ul = sext( ul, len );
			printx(format, ul);
			break;
		    default:
			if ( *mode == 'x' ) {
			    format = "0x%08x";
			    while ( len >= 4 ) {
				ul = *(unsigned long*)p;
				p += 4;
				printx(format, ul);
				format = "_%08x";
				len -= 4;
			    }
			    if ( len > 0 ) {	// clean up leftovers
				int more = len;
				int i = 0;
				ul_byte(i++) = *p++;
				more--;
				while( more-- )
				    ul_byte(i++) = *p++;
				if ( *format == '_' ) switch ( size ) {
				case 3:	format = "_%06x";	break;
				case 2:	format = "_%04x";	break;
				case 1:	format = "_%02x";	break;
				} else switch ( size ) {
				case 3:	format = "0x%06x";	break;
				case 2:	format = "0x%04x";	break;
				case 1:	format = "0x%02x";	break;
				}
				printx(format, ul);
			    }
			} else {
			    printe("can't use `%s' format with object of size %d\n", mode, length);
			    return 0;
			}
			break;
		    }
		    if ( count ) printx("\t");
		}
		break;
	case 'f':
	case 'g':
		d = fetch_double( p, size );
		printx(format, d);
		break;
	case 's':
		ul = *(unsigned long*)p;
		p += 4;

		if ( ul == 0 ) {
			printx("(null)");
		}

		if ( !current_process ) {
			printe("no process\n");
			return 0;
		} else {
			ret = current_process->read(ul, 1023, buf);
			if ( !ret ) {
				printx("(invalid character pointer value %#x)", ul);
			} else {
				if ( !explicit )
					count = (ret > 128) ? 128 : ret;
				printx("\"%s\"", fmt_str(buf, count, explicit));
			}
		}
		break;
	case 'a':
		if ( !explicit )
			count = (size > 128) ? 128 : 0;

		printx("\"%s\"", fmt_str(p, count, explicit));
		break;
	default:
		// others should have been handled in Expr::evaluate()
		printe("unexpected mode '%s' in format_bytes()\n", mode);
		return 0;
	}
	if ( !csmode )
		printx("\n");

	return 1;
}

unsigned long
sext( unsigned long l, int size )
{
	switch ( size ) {
	case 1:	if ( l & 0x80 )		l |= 0xffffff00;	break;
	case 2:	if ( l & 0x8000 )	l |= 0xffff0000;	break;
	case 4:		/* nothing to do */			break;
	default:
		printe("bad size (%d) in sext()\n", size);	
	}

	return l;
}

//
//	format is  "count" "length" "mode", all parts optional
//
int
parse_fmt( char *fmt, int &count, int &length, char *&mode, int &explicit )
{
	char *p = fmt;
	count = strtoul(fmt, &p, 10);	// count must be positive, decimal
	if ( p == fmt ) {
		count = 1;		// no count present
		explicit = 0;
	} else {
		explicit = 1;
	}

	switch ( *p ) {			// mode is b, h, l, or missing
	case 'b':	length = 1;	++p;	break;
	case 'h':	length = 2;	++p;	break;
	case 'l':	length = 4;	++p;	break;
	default:	length = 0;		break;
	}

	switch ( *p ) {
	default:
		printe("unknown format: %c\n", *p);
		return 0;
	case '\0':
		mode = p;
		break;
	case '#':
	case 'c':
	case 'd':
	case 'u':
	case 'o':
	case 'x':
	case 'f':
	case 'g':
	case 's':
	case 'a':
	case 'p':
	case 'i':
	case 'I':
		mode = p++;
		break;
	}

	if ( *p != '\0' && *p != '\n' ) {
		printe("extra characters in format: '%s'\n", p);
		return 0;
	}

	return 1;
}

static char str_buf[1024];

char *
fmt_str( unsigned char *p, int count, int count_was_explicit )
{
	register char *q = str_buf;
	int limit = 1000;
	
	while ( limit-- > 0 && (!count_was_explicit || count-- > 0) ) {
		if ( *p == '\0' && !count_was_explicit ) {
			break;
		} else if ( *p >= ' ' && *p < 127 ) {	/* printable */
			switch ( *p ) {
			default:   *q++ = *p;			      break;
			case '\"': *q++ = '\\'; *q++ = '\"'; limit--; break;
			case '\\': *q++ = '\\'; *q++ = '\\'; limit--; break;
			}
		} else if ( *p < ' ' ) {		/* control */
			switch ( *p ) {
			default:
				sprintf(q, "\\%03o", *p);
				q += 4;
				limit -= 3;
				break;
			case '\0': *q++ = '\\'; *q++ = '0';  limit--; break;
			case '\a': *q++ = '\\'; *q++ = 'a';  limit--; break;
			case '\b': *q++ = '\\'; *q++ = 'b';  limit--; break;
			case '\f': *q++ = '\\'; *q++ = 'f';  limit--; break;
			case '\n': *q++ = '\\'; *q++ = 'n';  limit--; break;
			case '\r': *q++ = '\\'; *q++ = 'r';  limit--; break;
			case '\t': *q++ = '\\'; *q++ = 't';  limit--; break;
			case '\v': *q++ = '\\'; *q++ = 'v';  limit--; break;
			}
		} else {				/* hyper-ascii? */
			sprintf(q, "\\%03o", *p);
			q += 4;
			limit -= 3;
		}
		++p;
	}
	*q = 0;
	return str_buf;
}
