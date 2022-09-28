#ident	"@(#)sdb:libexecon/common/Reg.C	1.1"

// Reg.C -- register names and attributes, common code

#include <string.h>
#include "Reg.h"
#include "Type.h"

RegRef
regref(register char *name)
{
	register RegAttrs *p = regs;

	for ( ; p->ref != REG_UNK ; p++ )
		if ( strcmp(p->name, name) == 0 )
			return p->ref;
	return REG_UNK;
}

RegAttrs *
regattrs(register RegRef ref)
{
	register RegAttrs *p = regs;

	for ( ; p->ref != REG_UNK ; p++ )
		if ( ref == p->ref )
			break;
	return p;		// will be REG_UNK entry if ref not found
}

RegAttrs *
regattrs(register char *name)
{
	register RegAttrs *p = regs;

	for ( ; p->ref != REG_UNK ; p++ )
		if ( strcmp(p->name, name) == 0 )
			break;
	return p;		// will be REG_UNK entry if name not found
}

Fund_type
regtype(RegRef ref)
{
    RegAttrs *regattr = regattrs(ref);

    if (regattr->flags & FPREG) { //MORE? machine specific.
	switch (regattr->size) {
	case  4: return ft_sfloat;
	case  8: return ft_lfloat;
	case 12: return ft_xfloat;
	case 16: return ft_xfloat;
	default: return ft_none;
	}
    }
    return ft_slong;
}
