#ident	"@(#)sdb:libexp/common/SDBinfo.C	1.3"

// SDBinfo -- internal representation of sdb expressions

#include "SDBinfo.h"
#include "Interface.h"
#include "str.h"
#include <string.h>

extern int debugflag;

#define DBG_SDBINFO	0x8000

char *SDB_kind_string(SDBinfo_kind k);

SDBinfo::SDBinfo( SDBinfo_kind k )
{
	::memset( (char *)this, 0, sizeof(*this) );
	kind = k;
//DBG	if ( debugflag & DBG_SDBINFO )
//DBG		printe("new SDBinfo( %s ) this = %#x\n",
//DBG			SDB_kind_string(k), this );
}

SDBinfo::~SDBinfo()
{
//DBG	if ( debugflag & DBG_SDBINFO )
//DBG		printe("~SDBinfo() kind = %s, this = %#x\n",
//DBG					SDB_kind_string(kind), this);
//DBG	if ( debugflag & DBG_SDBINFO )
//DBG		printe("next_decomp = %#x, next_arg = %#x\n",
//DBG				next_decomp, next_arg);
	delete next_decomp;
	next_decomp = 0;
	delete next_arg;
	next_arg = 0;
// ikASSIGN is now taken care of by eval_assign(), in parser.Y
//	if ( kind == ikASSIGN ) {
//		if ( debugflag & DBG_SDBINFO )
//			printe("assign.lhs = %#x, assign.rhs = %#x\n",
//				assign.lhs, assign.rhs);
//		delete assign.lhs;
//		assign.lhs = 0;
//		delete assign.rhs;
//		assign.rhs = 0;
//	}
}

SDBinfo *
SDBinfo_name(int glob, char *proc, char *var)
{
    register SDBinfo *n = new SDBinfo(ikNAME);

    n->name.global_only = glob;
    n->name.procedure   = proc ? str(proc) : 0;
    n->name.id          = str(var);
    n->name.level       = -1;	// none specified.

    return n;
}

SDBinfo *
SDBinfo_index(int with_star, long lo, long hi)
{
    register SDBinfo *n = new SDBinfo(ikINDEX);

    n->index.is_star = with_star;
    n->index.low     = lo;
    n->index.high    = hi;

    return n;
}

SDBinfo *
SDBinfo_select(SDBinfo_kind k, char *membpat, SDBinfo *pfx)
{
    register SDBinfo *n = new SDBinfo(k);
    n->select.field = str(membpat);

    return SDBinfo_append_decomp(pfx, n);
}

SDBinfo *
SDBinfo_append_arg(SDBinfo *list, SDBinfo *addon)
{
//DBG    if ( debugflag & DBG_SDBINFO )
//DBG		printe("SDBinfo_append_arg( %#x, %#x )\n", list, addon);

    if (list == 0) return addon;

    for (register SDBinfo *last = list;
	 last->next_arg != 0;
	 last = last->next_arg);

    last->next_arg = addon;

    return list;
}

SDBinfo *
SDBinfo_append_decomp(SDBinfo *list, SDBinfo *addon)
{
//DBG    if ( debugflag & DBG_SDBINFO )
//DBG		printe("SDBinfo_append_decomp( %#x, %#x )\n", list, addon);

    if (list == 0) return addon;

    for (register SDBinfo *last = list;
	 last->next_decomp != 0;
	 last = last->next_decomp);

    last->next_decomp = addon;
    if ( addon )
    	addon->prev_decomp = last;

//DBG    if ( debugflag & DBG_SDBINFO )
//DBG		printe("last = %#x\n", last);

    return list;
}

char *
SDB_kind_string(SDBinfo_kind k)
{
static char buff[40];

    switch (k) {
    case ikUninitialized: return "Uninitialized";
    case ikNAME:          return "NAME";
    case ikREGISTER:      return "REGISTER";
    case ikDOT:           return "DOT";
    case ikREF:           return "REF";
    case ikINDEX:         return "INDEX";
    case ikCALL:          return "CALL";
    case ikASSIGN:        return "ASSIGN";
    case ikINTEGER:       return "INTEGER";
    case ikCHAR:          return "CHAR";
    case ikFLOAT:         return "FLOAT";
    case ikSTRING:        return "STRING";
    default:
	sprintf(buff, "SDBinfo_kind(%d)", k);
	return buff;
    }
}

inline void
indent(int n) { printx("%*s", 4*n, ""); }

void
dump_SDBinfo(SDBinfo *info, int level)
{
    if (info == 0) {
	printx("(SDBinfo *)0\n");
	return;
    }
    indent(level);
    printx("SDBinfo: %s, ", SDB_kind_string(info->kind));

    switch (info->kind) {
    case ikNAME:
	printx("global_only: %d, procedure: %s, id: %s, level: %d\n",
	       info->name.global_only,
	       (info->name.procedure ? info->name.procedure : ""),
	       info->name.id,
	       info->name.level);
	break;
    case ikREGISTER:
	printx("regref: %d\n", info->reg.regref);
	break;
    case ikDOT:
    case ikREF:
	printx("field: %s\n", info->select.field);
	break;
    case ikINDEX:
	printx("is_star: %d, low: %d, high: %d\n",
	       info->index.is_star, info->index.low, info->index.high);
	break;
    case ikCALL:
	printx("fcn_name: %s\n", info->call.fcn_name);
	// next_arg is arglist.
	break;
    case ikASSIGN:
	printx(" -- lhs --\n");
	dump_SDBinfo(info->assign.lhs, 2);
	indent(level);
	printx(" -- rhs --\n");
	dump_SDBinfo(info->assign.rhs, 2);
	break;
    case ikINTEGER:
	printx("integer: %d\n", info->integer);
	break;
    case ikCHAR:
	printx("char: %c(%#x)\n", info->cchar, info->cchar);
	break;
    case ikFLOAT:
	printx("dbl: %g\n", info->dbl);
	break;
    case ikSTRING:
	printx("string: %s\n", info->string);
	break;
    case ikUninitialized:
    default: ; /*nothing*/
    }
    if (info->next_decomp != 0) {
	indent(level);
	printx("--> next_decomp:\n");
	dump_SDBinfo(info->next_decomp, level+1);
    }
    if (info->next_arg != 0) {
	indent(level);
	printx("--> next_arg:\n");
	dump_SDBinfo(info->next_arg, 1);
    }
}
