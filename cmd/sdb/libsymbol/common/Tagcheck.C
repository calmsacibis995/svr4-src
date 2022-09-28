#ident	"@(#)sdb:libsymbol/common/Tagcheck.C	1.1"

#include "Tag.h"
#include "Tagcheck.h"

// These functions are not inline. This is so that pointers to them can be
// passed.

int
stacktag(long x)	{ return (x == t_entry || x == t_extlabel); }

/* can labels be legitemate expression components?? */
int
vartag(long x)		{ return (((x > pt_startvars) && (x < pt_endvars))
				  || (x == t_entry)) ; }

int
typetag(long x)		{ return ((x > pt_starttypes) && (x < pt_endtypes)); }

int
addrtag(long x)
{
	return (((x) == t_functiontype) || ((x) == t_label) || ((x) == t_entry));
}

//begin - MORE: move to a debugging tools file.

#include <stdio.h>

#undef DEFTAG
#define DEFTAG(VAL, NAME) case VAL: return NAME;

static char *
tag_string(Tag tag)
{
    switch (tag) {
#include "Tag1.h"
    default:
	; // fall through.
    }
    static char buff[20];
    sprintf(buff, "unknown Tag(%d)", tag);
    return buff;
}

// end - move to debugging tools file.

