/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)localedef:colltbl/diag.c	1.1.3.1"
#include <stdio.h>
#include <varargs.h>
#include "colltbl.h"
#define SYNERR		0
#define SYNWARN		1
#define CMDERR		2
#define CMDWARN		3
#define ERRINDEX	(index > maxdiags) ? maxdiags + 1 : index

/* External Variables */
extern int Status, Lineno;
extern char *Cmd, *Infile;

void
error(va_alist)
va_dcl
{
	register int index;
	va_list ap;
	static char *diag_front[] = {
		" \"%s\", line %d: error: ",
		" \"%s\", line %d: warning: ",
		" error: ",
		" warning: "
	};
	static struct {
		int type;
		int status;
		char *msg;
	} diagnostics[] = {
		/* GEN_ERR */	SYNERR, ERROR, "error in %s\n",
		/* DUPLICATE */	SYNERR, ERROR, "duplicate %s name: %s\n",
		/* EXPECTED */	SYNERR, ERROR, "%s expected\n",
		/* ILLEGAL */	SYNERR, ERROR, "illegal %s: %s\n",
		/* TOO_LONG */	SYNWARN, ABORT, "%s too long: %s\n",
		/* INSERTED */	SYNWARN, WARNING, "inserted %s\n",
		/* NOT_FOUND */	SYNERR, ERROR, "%s not found: %s\n",
		/* NOT_DEFINED */	SYNERR, ERROR, "no %s defined\n",
		/* TOO_MANY */	SYNERR, ABORT, "too many %s\n",
		/* INVALID */	SYNERR, ERROR, "invalid %s: %s\n", 
		/* BAD_OPEN */	CMDERR, ABORT, "can't open %s\n", 
		/* NO_SPACE */	CMDERR, ABORT, "%s out of space!\n", 
		/* NEWLINE */	SYNERR, ERROR, "found newline in %s\n",
		/* REGERR */	SYNERR, ERROR, "regular expression error: %s\n",
		/* CMDWARN */	CMDWARN, WARNING, "no diagnostic message available\n",
		/* YYERR */	SYNERR, ERROR, "syntax error\n",
		/* PRERR */	SYNERR, ABORT, "%s\n"
	};
	static int maxdiags = (sizeof(diagnostics) / sizeof(diagnostics[0])) - 1;

	va_start(ap);
	index = (int) va_arg(ap, int *);
	fprintf(stderr,"%s:",Cmd);
	fprintf(stderr,diag_front[diagnostics[ERRINDEX].type],Infile,Lineno);
	vfprintf(stderr,diagnostics[ERRINDEX].msg,ap);
	va_end(ap);
	if(diagnostics[ERRINDEX].status == ABORT)
		exit(ABORT);
	Status = diagnostics[ERRINDEX].status;
}


void
usage()
{
#ifdef REGEXP
	fprintf(stderr,"%s format:  %s  [ -r ] [ file | - ]\n", Cmd, Cmd);
#else
	fprintf(stderr,"%s format:  %s  [ file | - ]\n", Cmd, Cmd);
#endif
	exit(Status);
}

#ifdef REGEXP
void
regerr(i)
int	i;
{
	char	*msg;

	switch(i) {
	case 11:
		msg = "range endpoint too large.";
		break;
	case 16:
		msg = "bad number.";
		break;
	case 25:
		msg = "digit out of range.";
		break;
	case 36:
		msg = "illegal or missing delimiter.";
		break;
	case 41:
		msg = "no remembered search string.";
		break;
	case 42:
		msg = "( ) imbalance.";
		break;
	case 43:
		msg = "too many (.";
		break;
	case 44:
		msg = "more than 2 numbers given in { }.";
		break;
	case 45:
		msg = "} expected after \\.";
		break;
	case 46:
		msg = "first number exceeds second in { }.";
		break;
	case 49:
		msg = "[ ] imbalance.";
		break;
	case 50:
		msg = "regular expression overflow.";
		break;
	case 51:
		msg = "<< >> imbalance.";
		break;
	case 52:
		msg = "unknown macro.";
		break;
	default:
		msg = "bad regular expression.";
		break;
	}
	error(REGERR, msg);
}
#endif
