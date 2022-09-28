/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/SDBinfo.h	1.1"

#ifndef SDBINFO_H
#define SDBINFO_H

#include "Symbol.h"
#include "TYPE.h"
#include "Itype.h"
#include "Place.h"

//     SDB_info -- intermediate representation of sdb expressions.

class  Process;
class  Value;
class  Rvalue;
class  Frame;

enum SDBinfo_kind {
    ikUninitialized = 0,
    ikNAME,
    ikREGISTER,
    ikDOT,
    ikREF,
    ikINDEX,
    ikCALL,
    ikASSIGN,
    ikINTEGER,
    ikCHAR,
    ikFLOAT,
    ikSTRING,
};

// SDBinfo is kept as a discriminated union to
// make storage management more efficient.

class SDBinfo {
public:
    SDBinfo_kind  kind;
    int		  temporary;	// for use by class Decomp
    SDBinfo      *next_decomp;	// selection and index decomposition.
    SDBinfo	 *prev_decomp;	// back pointer
    SDBinfo      *next_arg;	// procedure call argument list.
    union {
	struct {
	    int     global_only;
	    char   *procedure;
	    char   *id;
	    int     level;	// -1 => not specified.
	} name;

	struct {
	    int regref;
	} reg;

	struct {
	    int  is_star;
	    long low;
	    long high;
	} index;

	struct {
	    char *field;
	} select;

	struct {
	    char *fcn_name;
	} call;

	struct {
	    SDBinfo *lhs;
	    SDBinfo *rhs;
	} assign;

	char   *string;
	double  dbl;
	long    integer;
	char    cchar;
    };

    SDBinfo( SDBinfo_kind = ikUninitialized );

    ~SDBinfo();		// recursively free a tree
};

char     *SDB_kind_string(SDBinfo_kind);
void      dump_SDBinfo(SDBinfo *, int i = 1);
SDBinfo  *SDBinfo_append_arg(SDBinfo *, SDBinfo *);
SDBinfo  *SDBinfo_append_decomp(SDBinfo *, SDBinfo *);
SDBinfo  *SDBinfo_name(int, char *, char *);
SDBinfo  *SDBinfo_index(int, long, long);
SDBinfo  *SDBinfo_select(SDBinfo_kind, char *, SDBinfo *);

#endif
