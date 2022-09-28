/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cxref:cxref.h	1.1"
/*
** Everytime a symbol is used, it gets a USE record
*/
typedef struct use {
	char type;
	int line;
	struct use *next;
} USE;

/*
** For every symbol, it has a FILEUSE for each file it is
** used in.
*/
typedef struct fileuse {
	char *fname;
	struct use *usage;
	struct use *last;
	struct fileuse *next;
} FILEUSE;

/*
** Each level has a list of files it was used in.
** Level 0 is filescope, levels 1 and up are in block levels.
** Level -1 means it is static.
** mno is the module number this symbol was defined in;
** helps keep track of automatics with the same level,
** but different module numbers.
*/
typedef struct lev {
	short level;
	short mno;
	char *funcname;
	struct fileuse *files;
	struct lev *next;
} LEV;

/*
** Each symbol name has a list of levels, where each level is
** a different instant of the symbol name (different blocks,
** static, etc...)
*/
typedef struct xref {
	char *name;
	struct lev *levels;
	struct xref *next;
} XREF;
