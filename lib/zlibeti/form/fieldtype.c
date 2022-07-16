/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/fieldtype.c	1.1"

#include "utility.h"

typedef struct {

	char *		leftarg;
	char *		rightarg;
}
	LINK;

#define ArgL(n)		(((LINK *)(n)) -> leftarg)
#define ArgR(n)		(((LINK *)(n)) -> rightarg)

#define Ref(t)		((t) -> ref)
#define TypeL(t)	((t) -> left)
#define TypeR(t)	((t) -> right)
#define MakeA(t)	((t) -> makearg)
#define CopyA(t)	((t) -> copyarg)
#define FreeA(t)	((t) -> freearg)
#define Fcheck(t)	((t) -> fcheck)
#define Ccheck(t)	((t) -> ccheck)
#define Next(t)		((t) -> next)
#define Prev(t)		((t) -> prev)

	/**********************
	*  default fieldtype  *
	**********************/

static FIELDTYPE default_fieldtype =
{
			0,			/* status	*/
			0,			/* ref		*/
			(FIELDTYPE *) 0,	/* left		*/
			(FIELDTYPE *) 0,	/* right	*/
			(PTF_charP) 0,		/* makearg	*/
			(PTF_charP) 0,		/* copyarg	*/
			(PTF_void) 0,		/* freearg	*/
			(PTF_int) 0,		/* fcheck	*/
			(PTF_int) 0,		/* ccheck	*/
			(PTF_int) 0,		/* next		*/
			(PTF_int) 0,		/* prev		*/
};

FIELDTYPE * _DEFAULT_FIELDTYPE	= &default_fieldtype;

	/******************
	*  new_fieldtype  *
	******************/

FIELDTYPE * new_fieldtype (fcheck, ccheck)
PTF_int fcheck;		/* field validation function		*/
PTF_int ccheck;		/* character validation function	*/
{
	FIELDTYPE * t = (FIELDTYPE *) 0;

	if ((fcheck || ccheck) && Alloc (t, FIELDTYPE))
	{
		*t = *_DEFAULT_FIELDTYPE;

		Fcheck (t) = fcheck;
		Ccheck (t) = ccheck;
	}
	return t;
}

	/*******************
	*  link_fieldtype  *
	*******************/

FIELDTYPE * link_fieldtype (left, right)
FIELDTYPE * left;
FIELDTYPE * right;
{
	FIELDTYPE * t = (FIELDTYPE *) 0;

	if ((left || right) && Alloc (t, FIELDTYPE))
	{
		*t = *_DEFAULT_FIELDTYPE;

		Set (t, LINKED);

		if (Status (left, ARGS) || Status (right, ARGS))
			Set (t, ARGS);

		if (Status (left, CHOICE) || Status (right, CHOICE))
			Set (t, CHOICE);

		TypeL (t) = left;
		TypeR (t) = right;
		IncrType (left);	/* increment reference count */
		IncrType (right);	/* increment reference count */
	}
	return t;
}

	/*******************
	*  free_fieldtype  *
	*******************/

int free_fieldtype (t)
FIELDTYPE * t;
{
	if (! t)
		return E_BAD_ARGUMENT;

	if (Ref (t))
		return E_CONNECTED;

	if (Status (t, LINKED))
	{
		DecrType (TypeL (t));	/* decrement reference count */
		DecrType (TypeR (t));	/* decrement reference count */
	}
	Free (t);
	return E_OK;
}

	/**********************
	*  set_fieldtype_arg  *
	**********************/

int set_fieldtype_arg (t, makearg, copyarg, freearg)
FIELDTYPE *	t;
PTF_charP	makearg;
PTF_charP	copyarg;
PTF_void	freearg;
{
	if (t && makearg && copyarg && freearg)
	{
		Set (t, ARGS);
		MakeA (t) = makearg;
		CopyA (t) = copyarg;
		FreeA (t) = freearg;
		return E_OK;
	}
	return E_BAD_ARGUMENT;
}

	/*************************
	*  set_fieldtype_choice  *
	*************************/

int set_fieldtype_choice (t, next, prev)
FIELDTYPE *	t;
PTF_int		next;	/* next choice function */
PTF_int		prev;	/* prev choice function */
{
	if (t && next && prev)
	{
		Set (t, CHOICE);
		Next (t) = next;
		Prev (t) = prev;
		return E_OK;
	}
	return E_BAD_ARGUMENT;
}

	/*************
	*  _makearg  *
	*************/

char * _makearg (t, ap, err)
FIELDTYPE * t;
va_list * ap;
int * err;
{
/*
	invoke make_arg function associated with field type t.
	return pointer to argument information or null if none.
	increment err if an error is encountered.
*/
	char * p = (char *) 0;

	if (! t || ! Status (t, ARGS))
		return p;

	if (Status (t, LINKED))
	{
		LINK * n = (LINK *) 0;

		if (Alloc (n, LINK))
		{
			ArgL (n) = _makearg (TypeL (t), ap, err);
			ArgR (n) = _makearg (TypeR (t), ap, err);
			p = (char *) n;
		}
		else
			++(*err);		/* out of space */
	}
	else
		if (! (p = (*MakeA (t)) (ap)))
			++(*err);		/* make_arg had problem */
	return p;
}

	/*************
	*  _copyarg  *
	*************/

char * _copyarg (t, arg, err)
FIELDTYPE * t;
char * arg;
int * err;
{
/*
	invoke copy_arg function associated with field type t.
	return pointer to argument information or null if none.
	increment err if an error is encountered.
*/
	char * p = (char *) 0;

	if (! t || ! Status (t, ARGS))
		return p;

	if (Status (t, LINKED))
	{
		LINK * n = (LINK *) 0;

		if (Alloc (n, LINK))
		{
			ArgL (n) = _copyarg (TypeL (t), ArgL (arg), err);
			ArgR (n) = _copyarg (TypeR (t), ArgR (arg), err);
			p = (char *) n;
		}
		else
			++(*err);		/* out of space */
	}
	else
		if (! (p = (*CopyA (t)) (arg)))
			++(*err);		/* copy_arg had problem */
	return p;
}

	/*************
	*  _freearg  *
	*************/

void _freearg (t, arg)
FIELDTYPE * t;
char * arg;
{
/*
	invoke free_arg function associated with field type t.
*/
	if (! t || ! Status (t, ARGS))
		return;

	if (Status (t, LINKED))
	{
		_freearg (TypeL (t), ArgL (arg));
		_freearg (TypeR (t), ArgR (arg));
		Free (arg);
	}
	else
		(*FreeA (t)) (arg);
}

	/****************
	*  _checkfield  *
	****************/

int _checkfield (t, f, arg)
FIELDTYPE * t;
FIELD * f;
char * arg;
{
/*
	invoke check_field function associated with field type t.
*/
	if (! t)
		return TRUE;

	if (Opt (f, O_NULLOK))
	{
		char * v = Buf (f);

		while (*v && *v == ' ')
			++v;
		if (! *v)
			return TRUE;	/* empty field */
	}
	if (Status (t, LINKED))
		return	_checkfield (TypeL (t), f, ArgL (arg))
		||	_checkfield (TypeR (t), f, ArgR (arg));
	else
		if (Fcheck (t))
			return (*Fcheck (t)) (f, arg);
	return TRUE;
}

	/***************
	*  _checkchar  *
	***************/

int _checkchar (t, c, arg)
FIELDTYPE * t;
int c;
char * arg;
{
/*
	invoke check_char function associated with field type t.
*/
	if (! t)
		return TRUE;

	if (Status (t, LINKED))
		return	_checkchar (TypeL (t), c, ArgL (arg))
		||	_checkchar (TypeR (t), c, ArgR (arg));
	else
		if (Ccheck (t))
			return (*Ccheck (t)) (c, arg);
	return TRUE;
}

	/****************
	*  _nextchoice  *
	****************/

int _nextchoice (t, f, arg)
FIELDTYPE * t;
FIELD * f;
char * arg;
{
/*
	invoke next_choice function associated with field type t.
*/
	if (! t || ! Status (t, CHOICE))
		return FALSE;

	if (Status (t, LINKED))
		return	_nextchoice (TypeL (t), f, ArgL (arg))
		||	_nextchoice (TypeR (t), f, ArgR (arg));
	else
		return (*Next (t)) (f, arg);
}

	/****************
	*  _prevchoice  *
	****************/

int _prevchoice (t, f, arg)
FIELDTYPE * t;
FIELD * f;
char * arg;
{
/*
	invoke prev_choice function associated with field type t.
*/
	if (! t || ! Status (t, CHOICE))
		return FALSE;

	if (Status (t, LINKED))
		return	_prevchoice (TypeL (t), f, ArgL (arg))
		||	_prevchoice (TypeR (t), f, ArgR (arg));
	else
		return (*Prev (t)) (f, arg);
}


