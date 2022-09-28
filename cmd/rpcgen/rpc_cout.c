/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)rpcgen:rpc_cout.c	1.2.3.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#ifndef lint
static char sccsid[] = "@(#)rpc_cout.c 1.13 89/02/22 (C) 1987 SMI";
#endif

/*
 * rpc_cout.c, XDR routine outputter for the RPC protocol compiler 
 */
#include <stdio.h>
#if u3b2 || i386
#include <string.h>
#else
#include <strings.h>
#endif
/*#include <strings.h>*/
#include "rpc_util.h"
#include "rpc_parse.h"

/*
 * Emit the C-routine for the given definition 
 */
void
emit(def)
	definition *def;
{
	if (def->def_kind == DEF_PROGRAM || def->def_kind == DEF_CONST) {
		return;
	}
	print_header(def);
	switch (def->def_kind) {
	case DEF_UNION:
		emit_union(def);
		break;
	case DEF_ENUM:
		emit_enum(def);
		break;
	case DEF_STRUCT:
		emit_struct(def);
		break;
	case DEF_TYPEDEF:
		emit_typedef(def);
		break;
	}
	print_trailer();
}

static
findtype(def, type)
	definition *def;
	char *type;
{
	if (def->def_kind == DEF_PROGRAM || def->def_kind == DEF_CONST) {
		return (0);
	} else {
		return (streq(def->def_name, type));
	}
}

static
undefined(type)
	char *type;
{
	definition *def;

	def = (definition *) FINDVAL(defined, type, findtype);
	return (def == NULL);
}

static
print_header(def)
	definition *def;
{
	f_print(fout, "\n");
	f_print(fout, "bool_t\n");
	f_print(fout, "xdr_%s(xdrs, objp)\n", def->def_name);
	f_print(fout, "\tXDR *xdrs;\n");
	f_print(fout, "\t%s ", def->def_name);
	if (def->def_kind != DEF_TYPEDEF ||
	    !isvectordef(def->def.ty.old_type, def->def.ty.rel)) {
		f_print(fout, "*");
	}
	f_print(fout, "objp;\n");
	f_print(fout, "{\n");
}

static
print_trailer()
{
	f_print(fout, "\treturn (TRUE);\n");
	f_print(fout, "}\n");
}

static
print_ifopen(indent, name)
	int indent;
	char *name;
{
	tabify(fout, indent);
	f_print(fout, "if (!xdr_%s(xdrs", name);
}

static
print_ifarg(arg)
	char *arg;
{
	f_print(fout, ", %s", arg);
}

static
print_ifsizeof(prefix, type)
	char *prefix;
	char *type;
{
	if (streq(type, "bool")) {
		f_print(fout, ", sizeof(bool_t), xdr_bool");
	} else {
		f_print(fout, ", sizeof(");
		if (undefined(type) && prefix) {
			f_print(fout, "%s ", prefix);
		}
		f_print(fout, "%s), xdr_%s", type, type);
	}
}

static
print_ifclose(indent)
	int indent;
{
	f_print(fout, ")) {\n");
	tabify(fout, indent);
	f_print(fout, "\treturn (FALSE);\n");
	tabify(fout, indent);
	f_print(fout, "}\n");
}

static
print_ifstat(indent, prefix, type, rel, amax, objname, name)
	int indent;
	char *prefix;
	char *type;
	relation rel;
	char *amax;
	char *objname;
	char *name;
{
	char *alt = NULL;

	switch (rel) {
	case REL_POINTER:
		print_ifopen(indent, "pointer");
		print_ifarg("(char **)");
		f_print(fout, "%s", objname);
		print_ifsizeof(prefix, type);
		break;
	case REL_VECTOR:
		if (streq(type, "string")) {
			alt = "string";
		} else if (streq(type, "opaque")) {
			alt = "opaque";
		}
		if (alt) {
			print_ifopen(indent, alt);
			print_ifarg(objname);
		} else {
			print_ifopen(indent, "vector");
			print_ifarg("(char *)");
			f_print(fout, "%s", objname);
		}
		print_ifarg(amax);
		if (!alt) {
			print_ifsizeof(prefix, type);
		}
		break;
	case REL_ARRAY:
		if (streq(type, "string")) {
			alt = "string";
		} else if (streq(type, "opaque")) {
			alt = "bytes";
		}
		if (streq(type, "string")) {
			print_ifopen(indent, alt);
			print_ifarg(objname);
		} else {
			if (alt) {
				print_ifopen(indent, alt);
			} else {
				print_ifopen(indent, "array");
			}
			print_ifarg("(char **)");
			if (*objname == '&') {
				f_print(fout, "%s.%s_val, (u_int *)%s.%s_len",
					objname, name, objname, name);
			} else {
				f_print(fout, "&%s->%s_val, (u_int *)&%s->%s_len",
					objname, name, objname, name);
			}
		}
		print_ifarg(amax);
		if (!alt) {
			print_ifsizeof(prefix, type);
		}
		break;
	case REL_ALIAS:
		print_ifopen(indent, type);
		print_ifarg(objname);
		break;
	}
	print_ifclose(indent);
}

/* ARGSUSED */
static
emit_enum(def)
	definition *def;
{
	print_ifopen(1, "enum");
	print_ifarg("(enum_t *)objp");
	print_ifclose(1);
}

static
emit_union(def)
	definition *def;
{
	declaration *dflt;
	case_list *cl;
	declaration *cs;
	char *object;
	char *vecformat = "objp->%s_u.%s";
	char *format = "&objp->%s_u.%s";

	print_stat(&def->def.un.enum_decl);
	f_print(fout, "\tswitch (objp->%s) {\n", def->def.un.enum_decl.name);
	for (cl = def->def.un.cases; cl != NULL; cl = cl->next) {
		cs = &cl->case_decl;
		f_print(fout, "\tcase %s:\n", cl->case_name);
		if (!streq(cs->type, "void")) {
			object = alloc(strlen(def->def_name) + strlen(format) +
				       strlen(cs->name) + 1);
			if (isvectordef (cs->type, cs->rel)) {
				s_print(object, vecformat, def->def_name, 
					cs->name);
			} else {
				s_print(object, format, def->def_name, 
					cs->name);
			}
			print_ifstat(2, cs->prefix, cs->type, cs->rel, cs->array_max,
				     object, cs->name);
			free(object);
		}
		f_print(fout, "\t\tbreak;\n");
	}
	dflt = def->def.un.default_decl;
	if (dflt != NULL) {
		if (!streq(dflt->type, "void")) {
			f_print(fout, "\tdefault:\n");
			object = alloc(strlen(def->def_name) + strlen(format) +
				       strlen(dflt->name) + 1);
			if (isvectordef (dflt->type, dflt->rel)) {
				s_print(object, vecformat, def->def_name, 
					dflt->name);
			} else {
				s_print(object, format, def->def_name, 
					dflt->name);
			}

			print_ifstat(2, dflt->prefix, dflt->type, dflt->rel,
				     dflt->array_max, object, dflt->name);
			free(object);
			f_print(fout, "\t\tbreak;\n");
		}
	} else {
		f_print(fout, "\tdefault:\n");
		f_print(fout, "\t\treturn (FALSE);\n");
	}
	f_print(fout, "\t}\n");
}

static
emit_struct(def)
	definition *def;
{
	decl_list *dl;

	for (dl = def->def.st.decls; dl != NULL; dl = dl->next) {
		print_stat(&dl->decl);
	}
}

static
emit_typedef(def)
	definition *def;
{
	char *prefix = def->def.ty.old_prefix;
	char *type = def->def.ty.old_type;
	char *amax = def->def.ty.array_max;
	relation rel = def->def.ty.rel;

	print_ifstat(1, prefix, type, rel, amax, "objp", def->def_name);
}

static
print_stat(dec)
	declaration *dec;
{
	char *prefix = dec->prefix;
	char *type = dec->type;
	char *amax = dec->array_max;
	relation rel = dec->rel;
	char name[256];

	if (isvectordef(type, rel)) {
		s_print(name, "objp->%s", dec->name);
	} else {
		s_print(name, "&objp->%s", dec->name);
	}
	print_ifstat(1, prefix, type, rel, amax, name, dec->name);
}
