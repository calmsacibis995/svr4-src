/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/typedef.h	1.1"
/*ident	"@(#)cfront:src/typedef.h	1.4" */
typedef unsigned char TOK;
typedef class node * PP;
typedef char bit;
typedef int (*PFI)();
typedef void (*PFV)();
typedef class node * Pnode;
typedef struct key * Pkey;
typedef class name * Pname;
typedef class basetype * Pbase;
typedef class type * Ptype;
typedef class fct  * Pfct;
typedef class field * Pfield;
typedef class expr * Pexpr;
typedef class qexpr * Pqexpr;
typedef class texpr * Ptexpr;
typedef class classdef * Pclass;
typedef class enumdef * Penum;
typedef class stmt * Pstmt;
typedef class estmt * Pestmt;
typedef class tstmt * Ptstmt;
typedef class vec * Pvec;
typedef class ptr * Pptr;
typedef class block * Pblock;
typedef class table * Ptable;
typedef class loc Loc;
typedef class call * Pcall;
typedef class gen* Pgen;
typedef class ref * Pref;
typedef class name_list * Plist;
typedef class iline * Pin;
typedef class nlist * Pnlist;
typedef class slist * Pslist;
typedef class elist * Pelist;
