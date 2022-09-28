/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/msgbuf.c	1.3"
#include "msgbuf.h"
/*
** REMEMBER TO CHANGE NUMMSG in msgbuf.h every time a message is added
** or deleted from this file
**
** This file contains the text for all the messages that are to be buffered.
** They are of the form:
**		compound message
**		format for arguments in a compound message
**		a code (MULTI/LONE) for multi column output
**		a simple message
**
** The compound message is one line of text, with no format control.  It is
** printed as a header
** The format controls the manner in which the arguments are output.  The
** number for format controls should be equal to the number of arguments
** passed to it by BWERROR.
** The code is LONE for just one entry per line, or MULTI for 4 columns.
** The simple message is printed when the -s option is used - it should
** convey the same information as the combination of the compound message
** and the format, but in a single line.
** The number for format arguments should be equal, and in the same order
** as the format for the compound message
*/
#ifndef LINT2
/*
** pass 1 buffered lint messages
*/
static char ptr_cast[] = "pointer casts may be troublesome";
static char null_eff[] = "statement has null effect";
static char stmt_rch[] = "statement not reached";
static char ptr_alig[] = "pointer cast may result in improper alignment";
static char bit_sign[] = "bitwise operation on signed value possibly nonportable";
static char con_larg[] = "conversion to larger integral type may sign-extend incorrectly";
static char imp_narr[] = "assignment causes implicit narrowing conversion";

struct msgent msg[] = {

/* 0 */	{ "static unused",
	  "%s",
	  MULTI,
	  "static unused: %s" },

/* 1 */	{ "argument unused in function",
	  "%s in %s",
	  LONE,
	  "argument unused in function: %s in %s" },

/* 2 */	{ "variable unused in function",
	  "%s in %s",
	  LONE,
	  "variable unused in function: %s in %s" },

/* 3 */	{ "set but not used in function",
	  "%s in %s",
	  LONE,
	  "set but not used in function: %s in %s" },

/* 4 */	{ ptr_cast,
	  "",
	  MULTI,
	  ptr_cast },

/* 5 */	{ null_eff,
	  "",
	  MULTI,
	  null_eff },

/* 6 - Available */	{ "", "", 0, "" },

/* 7 - Available */	{ "", "", 0, "" },

/* 8 - Available */	{ "", "", 0, "" },

/* 9 */	{ stmt_rch,
	  "",
	  MULTI,
	  stmt_rch },

/*10 */ { "implicitly declared to return int",
	  "%s",
	  MULTI,
	  "implicitly declared to return int: %s" },

/*11 */ { ptr_alig,
	  "",
	  MULTI,
	  ptr_alig },

/*12 */ { bit_sign,
	  "",
	  MULTI,
	  "bitwise operation on signed value nonportable" },

/*13 */ { con_larg,
	  "",
	  MULTI,
	  con_larg },

/*14 */ { imp_narr,
	  "",
	  MULTI,
	  imp_narr },

/*15 */ { "function falls off bottom without returning value",
	  "%s",
	  MULTI,
	  "function falls off bottom without returning value: %s" },

/*16 */	{ "declaration unused in block",
	  "%s",
	  LONE,
	  "declaration unused in block: %s" }
};


#else
/*
** pass 2 lint buffered messages
*/
	
struct msgent msg[] = {

/* 0 */	{ "name used but not defined",	
	  "%-16s \t%s(%d)",
	  LONE,
	  "name used but not defined: %s in %s(%d)" },

/* 1 */	{ "name defined but never used",	
	  "%-16s \t%s(%d)",
	  LONE,
	  "name defined but never used: %s in %s(%d)" },

/* 2 */	{ "name declared but never used or defined",	
	  "%-16s \t%s(%d)",
	  LONE,
	  "name declared but never used or defined: %s in %s(%d)" },
	  
/* 3 */	{ "name multiply defined",	
	  "%-16s \t%s(%d) :: %s(%d)",
	  LONE,
	  "name multiply defined: %s in %s(%d) and %s(%d)" },

/* 4 */	{ "value type used inconsistently",	
	  "%-16s \t%s(%d) %s :: %s(%d) %s",
	  LONE,
	  "value type used inconsistently: %s in %s(%d) %s and %s(%d) %s" },

/* 5 */	{ "value type declared inconsistently",	
	  "%-16s \t%s(%d) %s :: %s(%d) %s",
	  LONE,
	  "value type declared inconsistently: %s in %s(%d) %s and %s(%d) %s" },
	  
/* 6 */	{ "function argument ( number ) used inconsistently",
	  "%s (arg %d) \t%s(%d) %s :: %s(%d) %s",
	  LONE,
	  "argument used inconsistently: %s(arg %d) in %s(%d) %s and %s(%d) %s" },

/* 7 */	{ "function called with variable number of arguments",	
	  "%-16s \t%s(%d) :: %s(%d)",
	  LONE,
	  "function called with variable number of args: %s in %s(%d) and %s(%d)" },

/* 8 */	{ "function value is used, but none returned",	
	  "%s",
	  MULTI,
	  "function value is used, but none returned: %s" },

/* 9 */	{ "function returns value which is always ignored",	
	  "%s",
	  MULTI,
	  "function returns value which is always ignored: %s" },

/*10 */	{ "function returns value which is sometimes ignored",
	  "%s",
	  MULTI,
	  "function returns value which is sometimes ignored: %s" },

/*11 */ { "function argument ( number ) declared inconsistently",
	  "%s (arg %d) \t%s(%d) %s :: %s(%d) %s",
	  LONE,
	  "function argument declared inconsistently: %s(arg %d) in %s(%d) %s and %s(%d) %s" },

/*12 */ { "function declared with variable number of arguments",
	  "%-16s\t%s(%d) :: %s(%d)",
	  LONE,
	  "function declared with variable number of args: %s in %s(%d) and %s(%d)" },

/*13 */ { "may be indistinguishable due to truncation",
	  "%s (%d) in %s :: %s (%d) in %s",
	  LONE,
	  "%s (%d) in %s may be indistinguishable from %s (%d) in %s due to truncation" },

/* 14 */ { "too many arguments for format",
	   "%-16s \t%s (%d)",
	   LONE,
	   "too many arguments for format: %s in %s (%d)" },

/* 15 */ { "malformed format string",
	   "%-16s \t%s (%d)",
	   LONE,
	   "malformed format string: %s in %s (%d)" },

/* 16 */ { "too few arguments for format",
	   "%-16s \t%s (%d)",
	   LONE,
	   "too few arguments for format: %s in %s (%d)" },

/* 17 */ { "function argument ( number ) type inconsistent with format",
	   "%s (arg %d) \t%s :: (format) %s\t%s(%d)",
	    LONE,
	   "function argument type inconsistent with format: %s(arg %d) %s and (format) %s in %s(%d)" },

/* 18 */ { "declared global, could be static",
	   "%-16s \t%s(%d)",
	   LONE,
	   "declared global, could be static: %s in %s(%d)" }
};
#endif
