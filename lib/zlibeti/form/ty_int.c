/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/ty_int.c	1.2"

#include "utility.h"

extern long atol ();

	/*******************************
	*  TYPE_INTEGER standard type  *
	*******************************/

typedef struct {

	int	prec;
	long	vmin;
	long	vmax;
}
	INTEGER;

static char * make_int (ap)
va_list * ap;
{
	INTEGER * n;

	if (Alloc (n, INTEGER))
	{
		n -> prec = va_arg (*ap, int);
		n -> vmin = va_arg (*ap, long);
		n -> vmax = va_arg (*ap, long);
	}
	return (char *) n;
}

static char * copy_int (arg)
char * arg;
{
	INTEGER * n;

	if (Alloc (n, INTEGER))
		*n = *((INTEGER *) arg);
	return (char *) n;
}

static void free_int (arg)
char * arg;
{
	Free (arg);
}

static int fcheck_int (f, arg)
FIELD * f;
char * arg;
{
	INTEGER *	n = (INTEGER *) arg;
	long		vmin = n -> vmin;
	long		vmax = n -> vmax;
	int		prec = n -> prec;
	char *		x = field_buffer (f, 0);
	char		buf[80];

	while (*x && *x == ' ')
		++x;
	if (*x)
	{
		char * t = x;

		if (*x == '-')
			++x;
		while (*x && isdigit (*x))
			++x;
		while (*x && *x == ' ')
			++x;
		if (! *x)
		{
			long v = atol (t);

			if (vmin >= vmax || (v >= vmin && v <= vmax))
			{
				(void)sprintf (buf, "%.*ld", prec, v);
				(void)set_field_buffer (f, 0, buf);
				return TRUE;
			}
		}
	}
	return FALSE;
}

#define charok(c)	(isdigit (c) || c == '-')

/*ARGSUSED*/

static int ccheck_int (c, arg)
int c;
char * arg;
{
	return charok (c);
}

/*
	TYPE_INTEGER

	usage:
		set_field_type (f, TYPE_INTEGER, precision, vmin, vmax);

		int precision;	for padding with leading zeros
		double vmin;	minimum acceptable value
		double vmax;	maximum acceptable value
*/
static FIELDTYPE typeINTEGER =
{
				ARGS,			/* status	*/
				1,			/* ref		*/
				(FIELDTYPE *) 0,	/* left		*/
				(FIELDTYPE *) 0,	/* right	*/
				make_int,		/* makearg	*/
				copy_int,		/* copyarg	*/
				free_int,		/* freearg	*/
				fcheck_int,		/* fcheck	*/
				ccheck_int,		/* ccheck	*/
				(PTF_int) 0,		/* next		*/
				(PTF_int) 0,		/* prev		*/
};

FIELDTYPE * TYPE_INTEGER = &typeINTEGER;
