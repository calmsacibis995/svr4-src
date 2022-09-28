/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)rpcgen:rpc_tblout.c	1.2.3.1"

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
static char sccsid[] = "@(#)rpc_tblout.c 1.4 89/02/22 (C) 1988 SMI";
#endif

/*
 * rpc_tblout.c, Dispatch table outputter for the RPC protocol compiler
 */
#include <stdio.h>
#if u3b2 || i386
#include <string.h>
#else
#include <strings.h>
#endif
/*#include <strings.h>*/
#include "rpc_parse.h"
#include "rpc_util.h"

#define TABSIZE		8
#define TABCOUNT	5
#define TABSTOP		(TABSIZE*TABCOUNT)

static char tabstr[TABCOUNT+1] = "\t\t\t\t\t";

static char tbl_hdr[] = "struct rpcgen_table %s_table[] = {\n";
static char tbl_end[] = "};\n";

static char null_entry[] = "\n\t(char *(*)())0,\n\
\txdr_void,\t\t\t0,\n\
\txdr_void,\t\t\t0,\n";

static char tbl_nproc[] = "int %s_nproc =\n\tsizeof(%s_table)/sizeof(%s_table[0]);\n\n";

void
write_tables()
{
	list *l;
	definition *def;

	f_print(fout, "\n");
	for (l = defined; l != NULL; l = l->next) {
		def = (definition *) l->val;
		if (def->def_kind == DEF_PROGRAM) {
			write_table(def);
		}
	}
}

static
write_table(def)
	definition *def;
{
	version_list *vp;
	proc_list *proc;
	int current;
	int expected;
	char progvers[100];
	int warning;

	for (vp = def->def.pr.versions; vp != NULL; vp = vp->next) {
		warning = 0;
		s_print(progvers, "%s_%s", locase(def->def_name), vp->vers_num);
		/* print the table header */
		f_print(fout, tbl_hdr, progvers);

		if (nullproc(vp->procs)) {
			expected = 0;
		} else {
			expected = 1;
			f_print(fout, null_entry);
		}
		for (proc = vp->procs; proc != NULL; proc = proc->next) {
			current = atoi(proc->proc_num);
			if (current != expected++) {
				f_print(fout,
			"\n/*\n * WARNING: table out of order\n */\n");
				if (warning == 0) {
					f_print(stderr,
				    "WARNING %s table is out of order\n",
					    progvers);
					warning = 1;
					nonfatalerrors = 1;
				}
				expected = current + 1;
			}
			f_print(fout, "\n\t(char *(*)())RPCGEN_ACTION(");

			/* routine to invoke */
			pvname(proc->proc_name, vp->vers_num);
			f_print(fout, "),\n");

			/* argument info */
			printit(proc->arg_prefix, proc->arg_type);

			/* result info */
			printit(proc->res_prefix, proc->res_type);
		}

		/* print the table trailer */
		f_print(fout, tbl_end);
		f_print(fout, tbl_nproc, progvers, progvers, progvers);
	}
}

static
printit(prefix, type)
	char *prefix;
	char *type;
{
	int len;
	int tabs;

	len = fprintf(fout, "\txdr_%s,", stringfix(type));
	/* account for leading tab expansion */
	len += TABSIZE - 1;
	/* round up to tabs required */
	tabs = (TABSTOP - len + TABSIZE - 1)/TABSIZE;
	f_print(fout, "%s", &tabstr[TABCOUNT-tabs]);

	if (streq(type, "void")) {
		f_print(fout, "0");
	} else {
		f_print(fout, "sizeof ( ");
		/* XXX: should "follow" be 1 ??? */
		ptype(prefix, type, 0);
		f_print(fout, ")");
	}
	f_print(fout, ",\n");
}
