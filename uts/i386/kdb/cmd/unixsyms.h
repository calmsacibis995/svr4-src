/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:cmd/unixsyms.h	1.3"

#define MAXSYMSIZE	100000
#define MAXSYMS		(MAXSYMSIZE/sizeof(struct symbols))

#define	MAGICSYM "symtable"		/* name of symbol in kernel where
					   output will be written */
#define PATCHSIZE 30000                 /* default size of symbol table */
#define SIZESYM	"kdb_symsize"		/* name of symbol for table size */

struct symbols
{
	long nameoffset;	/* offset into namepool */
	long value;		/* symbol value (n_value) */
} ;


extern struct symbols symtable[];       /* an entry per symbol in input */
extern int kdb_symsize;			/* size of symtable */
extern char namepool[];                 /* to hold all symbol names */
