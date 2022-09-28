#ident	"@(#)sdb:libint/common/Pref.C	1.3"

#define	ALLOCATE_SPACE
#include "Pref.h"
#include "Vector.h"
#include "string.h"
#include "Interface.h"
#include "stdlib.h"
#include <ctype.h>

extern char *UnQuote( char * );

Pref pref;

#define PREF(name, alias, type, tab, mem, def, func) \
		mem = def;

Pref::Pref()
{
#include "Pref1.h"
}

#undef PREF

#define PREF(name, alias, type, tab, mem, def, func) \
	{ name, alias, tab, (void *)&pref.mem, sizeof(type), func },

static
PrefEntry pentry[] = {
#include "Pref1.h"
	{0, 0, 0, 0, 0, 0}
};

/* KLUDGE - work around for cfront 2.0 bug - remove when bug fixed. */
/*        - by using printint and printstr as initializers cfront   */
/*        - is inspired to generate C extern decls.                 */
typedef (*KLUDGETYPE)(PrefEntry*, void *, int, char *);
KLUDGETYPE Kludge1 = parseint;
KLUDGETYPE Kludge2 = parsestr;
/* END KLUDGE */

#undef PREF

struct PLink {
	PLink	  *next;
	PrefEntry *ptr;
	char	  *name;
	PLink()	{ next = 0; ptr = 0; name = 0; }
};

#define HSIZE  29	/* a small prime */

struct PHash {
	PLink	*arr[HSIZE];
	PHash();
};

static PHash phash;

inline int
hashf ( char *p )
{
	int hash = *p++ << 2;
	if ( *p ) hash += (*p++) << 1 ;
	if ( *p ) hash += *p;
	return hash % HSIZE;
}

PHash::PHash()
{
	memset((char *)this, 0, sizeof(PHash));		// quick zero

	register int h;
	register PLink *pl;
	register PrefEntry *p = pentry;
	for ( ; p->name ; ++p ) {
		h = hashf(p->name);
		pl = new PLink;
		pl->ptr = p;
		pl->name = p->name;
		pl->next = phash.arr[h];
		phash.arr[h] = pl;
		if ( !p->alias )
			continue;
		h = hashf(p->alias);
		pl = new PLink;
		pl->ptr = p;
		pl->name = p->alias;
		pl->next = phash.arr[h];
		phash.arr[h] = pl;
	}
}

PrefEntry *
plookup ( register char *name )
{
	register int h = hashf(name);
	for ( register PLink *pl = phash.arr[h] ; pl ; pl = pl->next ) {
		if ( *name == *(pl->name) && !strcmp(name, pl->name) )
			return pl->ptr;
	}
	return 0;
}

static long
getval ( PrefEntry *pe )
{
	switch ( pe->siz ) {
	case 1:	return *(char *) pe->ptr;
	case 2: return *(short *)pe->ptr;
	case 4: return *(long *) pe->ptr;
	default:
		ABORT("bad size in getval: %d\n", pe->siz);
		return 0;
	}
}

char *
PrefEntry::print()
{
	static char buf[80];
	long n = getval(this);

	if ( parsefn == parseint ) {
		sprintf(buf, "%d", n);
	} else if ( parsefn == parsestr ) {
		sprintf(buf, "\"%s\"", n);
	} else if ( parsefn == parsechar ) {
		if ( n == 0177 )	sprintf(buf, "^?");
		else if ( n < 040 )	sprintf(buf, "^%c", n+0100);
		else			sprintf(buf, "%c", n);
	} else if ( parsefn == parsekey ) {
		keyword *k = keywd;
		if ( !k ) {
			ABORT ("no keyword table for keyword-type pref '%s'",
					name);
		}
		for ( ; k->key ; ++k ) {
			if ( n == k->val )
				break;
		}
		if ( !k->key ) {
			ABORT ("bad value for keyword-type pref '%s' = %d",
				name, n);
		}
		sprintf(buf, "%s", k->key);
	} else {
		ABORT ("bad parsefn in PrefEntry::print:%#x\n", parsefn);
	}
	return buf;
}

void
PrefEntry::help()
{
	printx("A legal value for '%s' is\n", name);
	if ( parsefn == parseint ) {
		printx("an integer in the range 0 to %d\n", (1<<(siz*8)) - 1);
	} else if ( parsefn == parsestr ) {
		printx("a double-quoted string with the following escapes:\n");
		printx("	\\n		newline\n");
		printx("	\\r		return\n");
		printx("	\\t		horizontal tab\n");
		printx("	\\v		vertical tab\n");
		printx("	\\b		backspace\n");
		printx("	\\f		formfeed\n");
		printx("	\\\\		backslash\n");
		printx("	\\'		single quote\n");
		printx("	\\\"		double quote\n");
		printx("	\\nnn		1, 2, or 3 octal digits\n");
		printx("	\\xnnn		1, 2, or 3 hex digits\n");
	} else if ( parsefn == parsechar ) {
		printx("a single ASCII character, or a two-character sequence\n");
		printx("where the first character is ^\n");
		printx("   ^a is equal to control-A\n");
		printx("   ^? is equal to RUBOUT (DEL)\n");
	} else if ( parsefn == parsekey ) {
		keyword *k = keywd;
		if ( !k ) {
			ABORT ("no keyword table for keyword-type pref '%s'",
					name);
		}
		printx("one of the following keywords (either upper or lower case)\n");
		for ( ; k->key ; ++k ) {
			printx("	%s\n", k->key);
		}
	} else {
		ABORT ("bad parsefn in PrefEntry::print:%#x\n", parsefn);
	}
}

static void putint(int n, void *p, int size, char *caller)
{
	switch ( size ) {
	case 1:
		if ( n & ~0xff )
			printe("warning, %d truncated to fit in 8 bits!\n", n);
		*(char *)p = n;
		break;
	case 2:
		if ( n & ~0xffff )
			printe("warning, %d truncated to fit in 16 bits!\n", n);
		*(short *)p = n;
		break;
	case 4:
		*(long *)p = n;
		break;
	default:
		ABORT("bad size in %s: %d\n", caller, size );
		break;
	}
}

int
parseint ( PrefEntry *, void *p, int size, char *s )
{		// parse s as an integer, assign to *p as a "size"-byte int
	long n;
	char *eos;

	n = strtol(s, &eos, 0);		// allow 0x10, 0377

	if ( eos == s || *eos != '\0' ) {
		printe("illformed integer:%s\n", s);
		return -1;
	}

	putint(n, p, size, "parseint");

	return 0;
}

int
parsekey ( PrefEntry *pe, void *p, int size, char *s )
{
	register keyword *k = pe->keywd;

	if ( !k ) {
		ABORT("missing keyword table for preference '%s'", pe->name);
	}

	for ( register char *q = s ; *q ; q++ ) {
		if ( isupper(*q) )
			*q = tolower(*q);
		else
			*q = *q;
	}

	for ( ; k->key ; k++ ) {
		if ( !strcmp(s, k->key) )
			break;
	}

	if ( !k->key ) {
		printe("invalid keyword '%s' for preference '%s'\n", s, pe->name);
		return -1;
	}

	putint(k->val, p, size, "parsekey");

#ifdef NOT_PROTOTYPE
	if ( pe->ptr == &pref.edit ) {
		switch ( k->val ) {
		default:
			abort();
		break;
		case ed_vi:
			Edit_mode = EDITVI;
			break;
		case ed_emacs:
			Edit_mode = EMACS;
			break;
		case ed_none:
			Edit_mode = 0;
			break;
		}
	}
#endif

	return 0;
}

static int
hexify(int c)
{
	if ( (c >= '0') && (c <= '9') )
		return c - '0';
	else if ( (c >= 'a') && (c <= 'f') )
		return c - 'a' + 10;
	else if ( (c >= 'A') && (c <= 'F') )
		return c - 'A' + 10;
	else
		return -1;
}

int
parsestr ( PrefEntry *, void *p, int /* size */, char *s )
{		// parse s as a double-quoted string, assign to *p as a char *
	Vector v;

	if ( !s || !*s ) {
		p = 0;
		return 0;
	}

	if ( *s == '"' ) {
		++s;
	} else {
		if ( *s != '\\' ) {
			printe("expecting quoted char, saw: %s\n", s );
			return -1;
		}
		s = UnQuote(s);
	}

	int i, x;
	char n;

	while ( *s ) {			// *s is current char
		if ( *s == '\\' ) {
			++s;
			switch ( *s ) {
			case 0:
				printe("illformed backslash escape\n");
				return -1;
			case 'n':	n = '\n'; v.add(&n, 1);		break;
			case 't':	n = '\t'; v.add(&n, 1);		break;
			case 'v':	n = '\v'; v.add(&n, 1);		break;
			case 'b':	n = '\b'; v.add(&n, 1);		break;
			case 'r':	n = '\r'; v.add(&n, 1);		break;
			case 'f':	n = '\f'; v.add(&n, 1);		break;
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				for ( i = 3, n = 0 ; i ; i--,s++ ) {
					if ( *s < '0' || *s > '7' ) {
						break;
					}
					n = (n<<3) + (*s - '0');
				}
				s--;		// ++ at bottom
				v.add(&n, 1);
				break;
			case 'x':
				++s;
				for ( i = 3, n = 0 ; i ; i--,s++ ) {
					if ( (x = hexify(*s)) < 0 ) {
						if ( n == 3 )
				printe("illformed hex escape: %s\n", s-2);
						break;
					}
					n = (n<<4) + x;
				}
				s--;		// ++ at bottom
				v.add(&n, 1);
				break;
			case '\\':
			case '\'':
			case '"':
			default:
				v.add(s, 1);	// skip backslash
				break;
			}
		} else if ( *s == '"' ) {
			n = 0;
			v.add(&n, 1);
			if ( *(s+1) != 0 ) {
				printe("illformed string: %s\n", s);
				return -1;
			} else {
				break;	// out of while()
			}
		} else {		// not backslash or double quote
			v.add(s, 1);
		}
		++s;
	}

	n = 0;
	v.add(&n, 1);		// ensure null terminated

	*(char **)p = (char *)malloc(v.size());
	strcpy(*(char **)p, (char *)v.ptr());
	return 0;
}

int
parsechar ( PrefEntry *, void *p, int size, char *s )
{		// parse s as a char, assign to *p as a "size"-byte int
	long n;

	if ( !s || !*s ) {
		printe("expecting character or ^character\n");
		return -1;
	}

	if ( *s == '^' ) {
		++s;
		if ( *s == '?' )
			n = 0177;
		else
			n = *s & 037;
	} else {
		n = *s;
	}

	putint(n, p, size, "parsechar");

	return 0;
}

#define npref  (sizeof(pentry)/sizeof(PrefEntry))

int
ckpref()	// ensure no duplication among names and aliases of preferences
		// (not normally called)
{
	char *names[2*npref];	// two for each PrefEntry in pentry
	register char **p = names;


	for ( register int h = 0 ; h < HSIZE ; h++ ) {	// for each hash bucket
		for ( register PLink* pl = phash.arr[h] ; pl ; pl = pl->next ) {
				// sanity check
			if ( pl->ptr < pentry || pl->ptr >= pentry + npref )
				abort();
				// check for duplicate names
			for ( register char **q = names ; q < p ; q++ ) {
				if ( !strcmp(pl->name, *q) ) {
					printf("dup: %s\n", pl->name);
					return 1;	//  found a duplicate
				}
			}
			*p++ = pl->name;	// enter name at end of list
		}
	}

	return 0;		// no duplicates
}


void
dumppref()	// print out the preferences and their values
{
	register PrefEntry *pe = pentry;

	while ( pe->name ) {
		printx("%-16s%-8s%s\n", pe->name, pe->alias, pe->print());
		pe++;
	}
}


void
dumphash()	// print the hash table and statistics
{
	register int h;
	register PLink *pl;
	register int count = 0;
	int longest = 0;
	register int total = 0;
	int bucket[HSIZE];

	for ( h = 0 ; h < HSIZE ; h++ ) {
		bucket[h] = 0;
	}

	for ( h = 0 ; h < HSIZE ; h++ ) {
		printx("[%2d]	", h);
		for ( pl = phash.arr[h] ; pl ; pl = pl->next ) {
			++count;
			printx("%s	", pl->name);
		}
		printx("\n");
		++bucket[count];
		total += count;
		if ( count > longest ) longest = count;
		count = 0;
	}

	printx("\ntotal names = %d\nnumber of buckets = %d\nlongest chain = %d\n", total, HSIZE, longest);

	printx("\nchain-length distribution:\nlen (cnt)\n");
	for ( h = 0 ; h <= longest ; h++ ) {
		printx("%2d (%2d) %.*s\n", h, bucket[h], bucket[h],
			"************************************************");
	}
	printx("\nhash-chain lengths:\n");
	for ( h = 0 ; h < HSIZE ; h++ ) {
		printx("[%2d]	", h);
		for ( pl = phash.arr[h] ; pl ; pl = pl->next ) {
			printx("*");
		}
		printx("\n");
	}
}

#ifdef NOT_PROTOTYPE
void set_edit_mode(int mode)	// called from editlib.C
{
	switch ( mode ) {
	case EDITVI:
		pref.edit = ed_vi;
		break;
	case EMACS:
		pref.edit = ed_emacs;
		break;
	case 0:
		pref.edit = ed_none;
		break;
	default:
		abort();
		break;
	}
}
#endif
