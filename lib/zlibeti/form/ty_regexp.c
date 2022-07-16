/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/ty_regexp.c	1.2"

#include "utility.h"

	/******************************
	*  TYPE_REGEXP standard type  *
	******************************/

extern char *regcmp(), *regex();

static char * make_rexp (ap)
va_list * ap;
{
	return regcmp (va_arg (*ap, char *), 0); /* (...)$n will dump core */
}

static char * copy_rexp (arg)
char * arg;
{
	char * rexp;

	if (arrayAlloc (rexp, (strlen (arg) + 1), char))
		(void)strcpy (rexp, arg);
	return rexp;
}

static void free_rexp (arg)
char * arg;
{
	Free (arg);
}

static int fcheck_rexp (f, arg)
FIELD * f;
char * arg;
{
	return regex (arg, field_buffer (f, 0)) ? TRUE : FALSE;
}

/*
	TYPE_REGEXP

	usage:
		set_field_type (f, TYPE_REGEXP, expression);

		char * expression;	regular expression REGCMP(3X)
*/
static FIELDTYPE typeREGEXP =
{
				ARGS,			/* status	*/
				1,			/* ref		*/
				(FIELDTYPE *) 0,	/* left		*/
				(FIELDTYPE *) 0,	/* right	*/
				make_rexp,		/* makearg	*/
				copy_rexp,		/* copyarg	*/
				free_rexp,		/* freearg	*/
				fcheck_rexp,		/* fcheck	*/
				(PTF_int) 0,		/* ccheck	*/
				(PTF_int) 0,		/* next		*/
				(PTF_int) 0,		/* prev		*/
};

FIELDTYPE * TYPE_REGEXP = &typeREGEXP;
