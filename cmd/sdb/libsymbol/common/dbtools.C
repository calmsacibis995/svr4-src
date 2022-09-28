#ident	"@(#)sdb:libsymbol/common/dbtools.C	1.1"

#include "Interface.h"
#include "Tag.h"
#include "TYPE.h"

/* -- dbtools.C contains utility routines to support debugging.
 *    When all debugging flags are turned off this file should
 *    NOT be linked into the final product.
*/

void
TYPE::dump(char *label)
{
    if (label == 0) label = "TYPE";
    printe("%s: ", label);
    printe("TYPE::dump(char*) not done yet\n");
}
