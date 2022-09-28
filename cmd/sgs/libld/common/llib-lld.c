/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/llib-lld.c	1.4"
/*LINTLIBRARY*/
#include <ar.h>
#include <stdio.h>
#include <values.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "linenum.h"
#include "scnhdr.h"
#include "syms.h"

LDFILE * allocldptr() { LDFILE *ldptr; return(ldptr); }

int freeldptr(ldptr) LDFILE *ldptr; { int success; return(success); }

int ldclose(ldptr) LDFILE *ldptr; { int success; return(success); }
int ldaclose(ldptr) LDFILE *ldptr; { int success; return(success); }

int ldahread(ldptr, arhead) LDFILE *ldptr; ARCHDR *arhead; { int success; return(success); }

LDFILE *ldaopen(filename,oldptr) char *filename; LDFILE *oldptr; { LDFILE *ldptr; return(ldptr); }

int ldfhread(ldptr, filehead) LDFILE *ldptr; FILHDR *filehead; { int success; return(success); }

char *ldgetname(ldptr,symbol) LDFILE *ldptr; SYMENT *symbol; { char *buffer; return(buffer); }

int ldlread(ldptr,fcnindex,linenum,linent) LDFILE *ldptr; long fcnindex; 
	unsigned short linenum; LINENO linent; { int success; return(success); }
int ldlinit(ldptr, fcnindex) LDFILE *ldptr; long fcnindex; { int success; return(success); }
int ldlitem(ldptr, linenum, linent) LDFILE *ldptr; unsigned short linenum;
	LINENO linent; { int success; return(success); }

int ldlseek(ldptr, sectindx) LDFILE *ldptr; unsigned short sectindx; { int success; return(success); }
int ldnlseek(ldptr, sectname) LDFILE *ldptr; char *sectname; { int success; return(success); }

int ldrseek(ldptr, sectindx) LDFILE *ldptr; unsigned short sectindx; { int success; return(success); }
int ldnrseek(ldptr, sectname) LDFILE *ldptr; char *sectname; { int success; return(success); }

int ldshread(ldptr, sectindx, secthead) LDFILE *ldptr; unsigned short sectindx;
	SCNHDR *secthead; { int success; return(success); }
int ldnshread(ldptr, sectname, secthead) LDFILE *ldptr; char *sectname;
	SCNHDR *secthead; { int success; return(success); }

int ldsseek(ldptr,sectindx) LDFILE *ldptr; unsigned short sectindx; { int success; return(success); }
int ldnsseek(ldptr,sectname) LDFILE *ldptr; char *sectname; { int success; return(success); }

int ldohseek(ldptr) LDFILE *ldptr; { int success; return(success); }

LDFILE *ldopen(filename, ldptr) char *filename; LDFILE *ldptr; { LDFILE *ldptrb;
	return(ldptrb); }

long ldtbindex(ldptr) LDFILE *ldptr; { long index; return(index); }

int ldtbread(ldptr, symindex, symbol) LDFILE *ldptr; long symindex;
	SYMENT *symbol; { int success; return(success); }

int ldtbseek(ldptr) LDFILE *ldptr; { int success; return(success); }

long sgetl(buffer) char *buffer; { long byt; return(byt); }
sputl(value, buffer) long value; char *buffer; { }

int vldldptr(ldptr) LDFILE *ldptr; { int success; return(success); }
