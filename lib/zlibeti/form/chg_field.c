/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/chg_field.c	1.2"

#include "utility.h"

#define first(f)	(f -> field [Pmin (f, P(f))])
#define last(f)		(f -> field [Pmax (f, P(f))])
#define sfirst(f)	(f -> field [Smin (f, P(f))])
#define slast(f)	(f -> field [Smax (f, P(f))])

#define Active(f)	(Opt (f, O_ACTIVE) && Opt (f, O_VISIBLE))

	/*********
	*  next  *
	*********/

static FIELD * next (f)
FIELD * f;
{
/*
	return next active field on page after f (user defined order)
*/
	FORM *		t	= f -> form;
	FIELD **	p	= t -> field + f -> index;
	FIELD **	pmin	= t -> field + Pmin (t, P(t));
	FIELD **	pmax	= t -> field + Pmax (t, P(t));

	do
		p = p == pmax ? pmin : p+1;

	while (! Active (*p) && *p != f);

	return *p;
}


	/*********
	*  prev  *
	*********/

static FIELD * prev (f)
FIELD * f;
{
/*
	return previous active field on page before f
*/
	FORM *		t	= f -> form;
	FIELD **	p	= t -> field + f -> index;
	FIELD **	pmin	= t -> field + Pmin (t, P(t));
	FIELD **	pmax	= t -> field + Pmax (t, P(t));

	do
		p = p == pmin ? pmax : p-1;

	while (! Active (*p) && *p != f);

	return *p;
}

	/**********
	*  snext  *
	**********/

static FIELD * snext (f)
FIELD * f;
{
/*
	return next active field on page after f (sorted order)
*/
	FIELD * x = f;

	do
		f = f -> snext;

	while (! Active (f) && f != x);

	return f;
}

	/**********
	*  sprev  *
	**********/

static FIELD * sprev (f)
FIELD * f;
{
/*
	return previous active field on page before f (sorted order)
*/
	FIELD * x = f;

	do
		f = f -> sprev;

	while (! Active (f) && f != x);

	return f;
}

	/*********
	*  left  *
	*********/

static FIELD * left (f)
FIELD * f;
{
/*
	return active field on page left of f
*/
	int row = f -> frow;

	do
		f = sprev (f);

	while (f -> frow != row);

	return f;
}

	/**********
	*  right  *
	**********/

static FIELD * right (f)
FIELD * f;
{
/*
	return active field on page right of f
*/
	int row = f -> frow;

	do
		f = snext (f);

	while (f -> frow != row);

	return f;
}

	/*******
	*  up  *
	*******/

static FIELD * up (f)
FIELD * f;
{
/*
	return active field on page above f
*/
	int row = f -> frow;
	int col = f -> fcol;

	do
		f = sprev (f);

	while (f -> frow == row && f -> fcol != col);

	if (f -> frow != row)
	{
		row = f -> frow;

		while (f -> frow == row && f -> fcol > col)
			f = sprev (f);

		if (f -> frow != row)
			f = snext (f);
	}
	return f;
}

	/*********
	*  down  *
	*********/

static FIELD * down (f)
FIELD * f;
{
/*
	return active field on page below f
*/
	int row = f -> frow;
	int col = f -> fcol;

	do
		f = snext (f);

	while (f -> frow == row && f -> fcol != col);

	if (f -> frow != row)
	{
		row = f -> frow;

		while (f -> frow == row && f -> fcol < col)
			f = snext (f);

		if (f -> frow != row)
			f = sprev (f);
	}
	return f;
}

	/****************
	*  _next_field  *
	****************/

int _next_field (f)
FORM * f;
{
	return _set_current_field (f, next (C(f)));
}

	/****************
	*  _prev_field  *
	****************/

int _prev_field (f)
FORM * f;
{
	return _set_current_field (f, prev (C(f)));
}

	/*****************
	*  _first_field  *
	*****************/

int _first_field (f)
FORM * f;
{
	return _set_current_field (f, next (last (f)));
}

	/****************
	*  _last_field  *
	****************/

int _last_field (f)
FORM * f;
{
	return _set_current_field (f, prev (first (f)));
}

	/*****************
	*  _snext_field  *
	*****************/

int _snext_field (f)
FORM * f;
{
	return _set_current_field (f, snext (C(f)));
}

	/*****************
	*  _sprev_field  *
	*****************/

int _sprev_field (f)
FORM * f;
{
	return _set_current_field (f, sprev (C(f)));
}

	/******************
	*  _sfirst_field  *
	******************/

int _sfirst_field (f)
FORM * f;
{
	return _set_current_field (f, snext (slast (f)));
}

	/*****************
	*  _slast_field  *
	*****************/

int _slast_field (f)
FORM * f;
{
	return _set_current_field (f, sprev (sfirst (f)));
}

	/****************
	*  _left_field  *
	****************/

int _left_field (f)
FORM * f;
{
	return _set_current_field (f, left (C(f)));
}

	/*****************
	*  _right_field  *
	*****************/

int _right_field (f)
FORM * f;
{
	return _set_current_field (f, right (C(f)));
}

	/**************
	*  _up_field  *
	**************/

int _up_field (f)
FORM * f;
{
	return _set_current_field (f, up (C(f)));
}

	/****************
	*  _down_field  *
	****************/

int _down_field (f)
FORM * f;
{
	return _set_current_field (f, down (C(f)));
}

	/******************
	*  _first_active  *
	******************/

FIELD * _first_active (f)
FORM * f;
{
	return next (last (f));
}
