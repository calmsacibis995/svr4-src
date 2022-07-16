/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/ty_alpha.c	1.1"

#include "utility.h"

	/*****************************
	*  TYPE_ALPHA standard type  *
	*****************************/

static char * make_alpha (ap)
va_list * ap;
{
	int * width;

	if (Alloc (width, int))
		*width = va_arg (*ap, int);
	return (char *) width;
}

static char * copy_alpha (arg)
char * arg;
{
	int * width;

	if (Alloc (width, int))
		*width = *((int *) arg);
	return (char *) width;
}

static void free_alpha (arg)
char * arg;
{
	Free (arg);
}

static int fcheck_alpha (f, arg)
FIELD * f;
char * arg;
{
	int	width	= *((int *) arg);
	int	n	= 0;
	char *	v	= field_buffer (f, 0);

	while (*v && *v == ' ')
		++v;
	if (*v)
	{
		char * vbeg = v;
		while (*v && isalpha (*v))
			++v;
		n = v - vbeg;
		while (*v && *v == ' ')
			++v;
	}
	return *v || n < width ? FALSE : TRUE;
}

/*ARGSUSED*/

static int ccheck_alpha (c, arg)
int c;
char * arg;
{
	return isalpha (c);
}

/*
	TYPE_ALPHA

	usage:
		set_field_type (f, TYPE_ALPHA, width);

		int width;	minimum token width
*/
static FIELDTYPE typeALPHA =
{
				ARGS,			/* status	*/
				1,			/* ref		*/
				(FIELDTYPE *) 0,	/* left		*/
				(FIELDTYPE *) 0,	/* right	*/
				make_alpha,		/* makearg	*/
				copy_alpha,		/* copyarg	*/
				free_alpha,		/* freearg	*/
				fcheck_alpha,		/* fcheck	*/
				ccheck_alpha,		/* ccheck	*/
				(PTF_int) 0,		/* next		*/
				(PTF_int) 0,		/* prev		*/
};

FIELDTYPE * TYPE_ALPHA = &typeALPHA;
