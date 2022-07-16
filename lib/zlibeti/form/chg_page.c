/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/chg_page.c	1.1"

#include "utility.h"

#define first(f)	(0)
#define last(f)		(f -> maxpage - 1)

	/*********
	*  next  *
	*********/

static int next (f)
FORM * f;
{
/*
	return next page after current page (cyclic)
*/
	int p = P(f);

	if (++p > last (f))
		p = first (f);
	return p;
}

	/*********
	*  prev  *
	*********/

static int prev (f)
FORM * f;
{
/*
	return previous page before current page (cyclic)
*/
	int p = P(f);

	if (--p < first (f))
		p = last (f);
	return p;
}

	/***************
	*  _next_page  *
	***************/

int _next_page (f)
FORM * f;
{
	return _set_form_page (f, next (f), (FIELD *) 0);
}

	/***************
	*  _prev_page  *
	***************/

int _prev_page (f)
FORM * f;
{
	return _set_form_page (f, prev (f), (FIELD *) 0);
}

	/****************
	*  _first_page  *
	****************/

int _first_page (f)
FORM * f;
{
	return _set_form_page (f, first (f), (FIELD *) 0);
}

	/***************
	*  _last_page  *
	***************/

int _last_page (f)
FORM * f;
{
	return _set_form_page (f, last (f), (FIELD *) 0);
}
