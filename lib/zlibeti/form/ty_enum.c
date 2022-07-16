/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/ty_enum.c	1.2"

#include "utility.h"

	/****************************
	*  TYPE_ENUM standard type  *
	****************************/

typedef struct {

	char **	list;
	int	checkcase;
	int	checkuniq;
	int	count;
}
	ENUM;

static char * make_enum (ap)
va_list * ap;
{
	ENUM * n;

	if (Alloc (n, ENUM))
	{
		char **		v;

		n -> list	= va_arg (*ap, char **);
		n -> checkcase	= va_arg (*ap, int);
		n -> checkuniq	= va_arg (*ap, int);

		for (v = n -> list; *v; ++v)
		;
		n -> count = v - n -> list;
	}
	return (char *) n;
}

static char * copy_enum (arg)
char * arg;
{
	ENUM * n;

	if (Alloc (n, ENUM))
		*n = *((ENUM *) arg);
	return (char *) n;
}

static void free_enum (arg)
char * arg;
{
	Free (arg);
}

#define NO_MATCH		0
#define PARTIAL_MATCH		1
#define EXACT_MATCH		2

static int cmp (x, v, checkcase)
char * x;
char * v;
int checkcase;
{
	while (*v && *v == ' ')			/* remove leading blanks */
		++v;
	while (*x && *x == ' ')			/* remove leading blanks */
		++x;

	if (*v == '\0')
		return *x == '\0' ? EXACT_MATCH : NO_MATCH;

	if (checkcase)				/* case is significant */
	{
		while (*x++ == *v)
			if (*v++ == '\0')
				return EXACT_MATCH;
	}
	else					/* ignore case */
	{
		while (toupper (*x++) == toupper (*v))
			if (*v++ == '\0')
				return EXACT_MATCH;
	}
	while (*v && *v == ' ')			/* remove trailing blanks */
		++v;
	if (*v)
		return NO_MATCH;
	else
		return *--x ? PARTIAL_MATCH : EXACT_MATCH;
}

static int fcheck_enum (f, arg)
FIELD * f;
char * arg;
{
	ENUM *		n		= (ENUM *) arg;
	char **		list		= n -> list;
	int		checkcase	= n -> checkcase;
	int		checkuniq	= n -> checkuniq;
	int		m;
	char *		v		= field_buffer (f, 0);
	char *		x;

	while (x = *list++)
		if (m = cmp (x, v, checkcase))
		{
			char * value = x;

			if (checkuniq && m != EXACT_MATCH)
				while (x = *list++)
					if (m = cmp (x, v, checkcase))
					{
						if (m == EXACT_MATCH)
						{
							value = x;
							break;
						}
						else
							value = (char *) 0;
					}
			if (! value)
				return FALSE;

			(void)set_field_buffer (f, 0, value);
			return TRUE;
		}

	return FALSE;
}

static int next_enum (f, arg)
FIELD * f;
char * arg;
{
	ENUM *		n		= (ENUM *) arg;
	char **		list		= n -> list;
	int		checkcase	= n -> checkcase;
	int		count		= n -> count;
	char *		v		= field_buffer (f, 0);

	while (count--)
		if (cmp (*list++, v, checkcase) == EXACT_MATCH)
			break;
	if (count <= 0)
		list = n -> list;

	if (count >= 0 || cmp ("", v, checkcase) == EXACT_MATCH)
	{
		(void)set_field_buffer (f, 0, *list);
		return TRUE;
	}
	return FALSE;
}

static int prev_enum (f, arg)
FIELD * f;
char * arg;
{
	ENUM *		n		= (ENUM *) arg;
	char **		list		= n -> list + n -> count - 1;
	int		checkcase	= n -> checkcase;
	int		count		= n -> count;
	char *		v		= field_buffer (f, 0);

	while (count--)
		if (cmp (*list--, v, checkcase) == EXACT_MATCH)
			break;
	if (count <= 0)
		list = n -> list + n -> count - 1;

	if (count >= 0 || cmp ("", v, checkcase) == EXACT_MATCH)
	{
		(void)set_field_buffer (f, 0, *list);
		return TRUE;
	}
	return FALSE;
}

/*
	TYPE_ENUM

	usage:
		set_field_type (f, TYPE_ENUM, list, checkcase, checkuniq);

		char ** list;	list of acceptable strings
		int checkcase;	TRUE - upper/lower case is significant
		int checkuniq;	TRUE - unique match required
/*

*/
static FIELDTYPE typeENUM =
{
				ARGS | CHOICE,		/* status	*/
				1,			/* ref		*/
				(FIELDTYPE *) 0,	/* left		*/
				(FIELDTYPE *) 0,	/* right	*/
				make_enum,		/* makearg	*/
				copy_enum,		/* copyarg	*/
				free_enum,		/* freearg	*/
				fcheck_enum,		/* fcheck	*/
				(PTF_int) 0,		/* ccheck	*/
				next_enum,		/* next		*/
				prev_enum,		/* prev		*/
};

FIELDTYPE * TYPE_ENUM = &typeENUM;
