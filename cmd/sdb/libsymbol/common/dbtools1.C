#ident	"@(#)sdb:libsymbol/common/dbtools1.C	1.2"

#include <string.h>
#include "Interface.h"
#include "Tag.h"
#include "Symbol.h"

/* -- dbtools.C contains utility routines to support debugging.
 *    When all debugging flags are turned off this file should
 *    NOT be linked into the final product.
*/


#define no_tag (-1)

void
Symbol::dump(char *msg)
{
Tag tag = no_tag;

    if (msg == 0) msg = "Symbol";
    printe("--> %s: ", msg);
    if (strlen(msg) > 40) printe("\n");

    if (isnull()) {
	printe("<null symbol>\n");
    } else {
	Attribute *a = attribute(an_tag);
	if (a == 0 || a->value.word == t_none) {
	    printe("NO TAG,  ");
	} else {
	    tag = a->value.word;
	    printe("tag=%s,   ", tag_string(tag));
	}

	char *nname = name();
	if (nname != 0) printe("name=%s, ", nname);

	printe("\n");
    }
}
