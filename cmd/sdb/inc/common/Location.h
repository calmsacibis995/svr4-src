/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Location.h	1.1"
// FILE:    Location.h
// EMACS_MODES: !fill c, comcol=43
// AUTHOR:  Jonathan S. Shapiro (sfbug!shap)
//
// COPYRIGHT (c) 1986 by AT&T Information Systems, Inc.
//               All Rights Reserved
//               THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//
// CREATED: Fri Mar 13 11:04:50 EST 1987
//
// DESCRIPTION:
//
// Structure passed by the argument proceessing service routine back to the
// client side for processing.
//

class Process;
class Object;

#ifndef Itype_h
# include "Itype.h"
#endif
#ifndef Source_h
# include "Source.h"
#endif

class Location {
    Process *process;
    char *arg;			/* used to hold the argument string */
    char *argndx;			/* current char pos in arg */

    char *atpos;
    char *sharppos;
    char *colonpos;
    char *dotdotpos;

    int internal_isrange;	// t if location was A..B
    int internal_iscount;	// t if location was A:B
    int internal_isfunct;	// t if location was a function name
    int internal_islabel;	// t if location was a label
    int internal_isfile;	// t if location was a file (?)
    int internal_linevalid;
    int internal_addrvalid;

    int parse_numeric(char *, int);
    int parse_label();
    int parse_lineno();
public:
    Location(Process *, char *);
    ~Location()             { delete arg; }

    int isrange()		{ return internal_isrange; }
    int iscount()		{ return internal_iscount; }
    int isfunct()		{ return internal_isfunct; }
    int islabel()		{ return internal_islabel; }
    int isfile()		{ return internal_isfile; }
    int linevalid()		{ return internal_linevalid; }
    int addrvalid()		{ return internal_addrvalid; }

    Iaddr addr_low;		/* low end of range as an address */
    Iaddr addr_hi;		/* high end of range as an address */
				/* valid if the location is a range.
				   if the location is a function name,
				   addr_high is the upper bound of the
				   function, that is, if
					addr_low <= pc < addr_hi
				   then pc is in the function. */

    long line_low;		/* low end of range as a line number */
    long line_hi;		/* high end of range as a line number */
				/* possibly == low, only valid for line
				   ranges */

    long count;			/* count field  - always valid*/
    Source	source;		/* source file */
    int next();
} ;
