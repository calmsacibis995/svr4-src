/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/ty_num.c	1.2"

#include "utility.h"

extern double atof ();

	/*******************************
	*  TYPE_NUMERIC standard type  *
	*******************************/

typedef struct {

	int	prec;
	double	vmin;
	double	vmax;
}
	NUMERIC;

static char * make_num (ap)
va_list * ap;
{
	NUMERIC * n;

	if (Alloc (n, NUMERIC))
	{
		n -> prec = va_arg (*ap, int);
		n -> vmin = va_arg (*ap, double);
		n -> vmax = va_arg (*ap, double);
	}
	return (char *) n;
}

static char * copy_num (arg)
char * arg;
{
	NUMERIC * n;

	if (Alloc (n, NUMERIC))
		*n = *((NUMERIC *) arg);
	return (char *) n;
}

static void free_num (arg)
char * arg;
{
	Free (arg);
}

static int fcheck_num (f, arg)
FIELD * f;
char * arg;
{
	NUMERIC *	n = (NUMERIC *) arg;
	double		vmin = n -> vmin;
	double		vmax = n -> vmax;
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
		if (*x == '.')
		{
			++x;
			while (*x && isdigit (*x))
				++x;
		}
		while (*x && *x == ' ')
			++x;
		if (! *x)
		{
			double v = atof (t);

			if (vmin >= vmax || (v >= vmin && v <= vmax))
			{
				(void)sprintf (buf, "%.*f", prec, v);
				(void)set_field_buffer (f, 0, buf);
				return TRUE;
			}
		}
	}
	return FALSE;
}

#define charok(c)	(isdigit (c) || c == '-' || c == '.')

/*ARGSUSED*/

static int ccheck_num (c, arg)
int c;
char * arg;
{
	return charok (c);
}

/*
	TYPE_NUMERIC

	usage:
		set_field_type (f, TYPE_NUMERIC, precision, vmin, vmax);

		int precision;	digits to right of decimal point
		double vmin;	minimum acceptable value
		double vmax;	maximum acceptable value
*/
static FIELDTYPE typeNUMERIC =
{
				ARGS,			/* status	*/
				1,			/* ref		*/
				(FIELDTYPE *) 0,	/* left		*/
				(FIELDTYPE *) 0,	/* right	*/
				make_num,		/* makearg	*/
				copy_num,		/* copyarg	*/
				free_num,		/* freearg	*/
				fcheck_num,		/* fcheck	*/
				ccheck_num,		/* ccheck	*/
				(PTF_int) 0,		/* next		*/
				(PTF_int) 0,		/* prev		*/
};

FIELDTYPE * TYPE_NUMERIC = &typeNUMERIC;
