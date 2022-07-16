/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/form.c	1.8"

#include "utility.h"

#define	MAX_BUF		81

	/*****************
	*  default form  *
	*****************/

static FORM default_form =
{
			0,			/* status	*/
			0,			/* rows		*/
			0,			/* cols		*/
			0,			/* currow	*/
			0,			/* curcol	*/
			0,			/* toprow	*/
			0,			/* begincol */
			-1,			/* maxfield	*/
			-1,			/* maxpage	*/
			-1,			/* curpage	*/
			O_NL_OVERLOAD	|
			O_BS_OVERLOAD,		/* opts		*/
			(WINDOW *) 0,		/* win		*/
			(WINDOW *) 0,		/* sub		*/
			(WINDOW *) 0,		/* w		*/
			(FIELD **) 0,		/* field	*/
			(FIELD *) 0,		/* current	*/
			(_PAGE *) 0,		/* page		*/
			(char *) 0,		/* usrptr	*/
			(PTF_void) 0,		/* forminit	*/
			(PTF_void) 0,		/* formterm	*/
			(PTF_void) 0,		/* fieldinit	*/
			(PTF_void) 0,		/* fieldterm	*/
};

FORM * _DEFAULT_FORM = &default_form;

	/***********
	*  insert  *
	***********/

static FIELD * insert (f, head)
FIELD * f;
FIELD * head;
{
/*
	insert field f into sorted list pointed to by head.
	return (possibly new) head of list.
*/
	FIELD * p;
	FIELD * newhead;
	int frow, fcol;

	if (head)
	{
		p = newhead = head;

		frow = f -> frow;
		fcol = f -> fcol;

		while (	(p -> frow < frow)
		||	(p -> frow == frow && p -> fcol < fcol))
		{
			p = p -> snext;

			if (p == head)
			{
				head = (FIELD *) 0;
				break;
			}
		}
		f -> snext		= p;
		f -> sprev		= p -> sprev;
		f -> snext -> sprev	= f;
		f -> sprev -> snext	= f;

		if (p == head)
			newhead = f;	/* insert at head of list */
	}
	else
		newhead = f -> sprev = f -> snext = f;	/* initialize new list */

	return newhead;
}

	/**************
	*  sort_form  *
	**************/

static void sort_form (f)
FORM * f;
{
/*
	sort fields on form (per page)
*/
	FIELD ** field;
	FIELD * p;
	int i, page, pmin, pmax;

	field = f -> field;

	for (page = 0; page < f -> maxpage; ++page)	/* for each page */
	{
		p = (FIELD *) 0;

		pmin = Pmin (f, page);
		pmax = Pmax (f, page);

		for (i = pmin; i <= pmax; ++i)		/* for each field */
		{
			field[i] -> index = i;
			field[i] -> page = page;

			p = insert (field[i], p);
		}
		Smin (f, page) = p -> index;		/* set sorted min */
		Smax (f, page) = p -> sprev -> index;	/* set sorted max */
	}
}

	/**********
	*  merge  *
	**********/

static void merge (f, form) /* adjust form dimensions to include field f */
FIELD * f;
FORM * form;
{
/*
	xmax/ymax is the minimum window size to hold field f
*/
	int xmax = f -> fcol + f -> cols;
	int ymax = f -> frow + f -> rows;

	if (form -> rows < ymax) form -> rows = ymax;
	if (form -> cols < xmax) form -> cols = xmax;
}

	/**********************
	*  disconnect_fields  *
	**********************/

static void disconnect_fields (form)
FORM * form;
{
/*
	disconnect fields from form
*/
	FIELD ** f = form -> field;

	if (f)
		while (*f)
		{
			if ((*f) -> form == form)
				(*f) -> form = (FORM *) 0;
			++f;
		}

	form -> rows		= 0;
	form -> cols		= 0;
	form -> maxfield	= -1;
	form -> maxpage		= -1;
	form -> field		= (FIELD **) 0;
}

	/*******************
	*  connect_fields  *
	*******************/

static int connect_fields (f, x)
FORM * f;
FIELD ** x;
{
/*
	connect fields to form
*/
	_PAGE *		page;

	int		nf,		/* number of fields	*/
			np;		/* number of pages	*/

	int		i;

	f -> field = x;
	f -> maxfield = 0;
	f -> maxpage = 0;

	if (! x)
		return E_OK;	/* null field array */

	for (nf = 0, np = 0; x[nf]; ++nf)
	{
		if (nf == 0 || Status (x[nf], NEW_PAGE))
			++np;			/* count pages */

		if (x[nf] -> form)
			return E_CONNECTED;
		else
			x[nf] -> form = f;	/* connect field to form */
	}
	if (nf == 0)
		return E_BAD_ARGUMENT;		/* no fields */

	if (arrayAlloc (f -> page, np, _PAGE))
	{
		page = f -> page;

		for (i = 0; i < nf; ++i)
		{
			if (i == 0)
				page -> pmin = i;

			else if (Status (x[i], NEW_PAGE))
			{
				page -> pmax = i - 1;
				++page;
				page -> pmin = i;
			}
			merge (x[i], f);
		}
		page -> pmax = nf - 1;
		f -> maxfield = nf;
		f -> maxpage = np;
		sort_form (f);
		return E_OK;
	}
	return E_SYSTEM_ERROR;
}

	/*************
	*  new_form  *
	*************/

FORM * new_form (field)
FIELD ** field;
{
	FORM * f;

	if (Alloc (f, FORM))
	{
		*f = *_DEFAULT_FORM;

		if (connect_fields (f, field) == E_OK)
		{
			if (f -> maxpage)
			{
				P(f) = 0;
				C(f) = _first_active (f);
			}
			else
			{
				P(f) = -1;
				C(f) = (FIELD *) 0;
			}
			return f;
		}
	}
	(void)free_form (f);
	return (FORM *) 0;
}

	/**************
	*  free_form  *
	**************/

int free_form (f)
FORM * f;
{
	if (! f)
		return E_BAD_ARGUMENT;

	if (Status (f, POSTED))
		return E_POSTED;

	disconnect_fields (f);
	Free (f -> page);
	Free (f);
	return E_OK;
}

	/********************
	*  set_form_fields  *
	********************/

int set_form_fields (f, fields)
FORM * f;
FIELD ** fields;
{
	FIELD ** p;
	int v;

	if (! f)
		return E_BAD_ARGUMENT;

	if (Status (f, POSTED))
		return E_POSTED;

	p = f -> field;
	disconnect_fields (f);

	if ((v = connect_fields (f, fields)) == E_OK)
	{
		if (f -> maxpage)
		{
			P(f) = 0;
			C(f) = _first_active (f);
		}
		else
		{
			P(f) = -1;
			C(f) = (FIELD *) 0;
		}
	}
	else
		(void)connect_fields (f, p);	/* reconnect original fields */
	return v;
}

FIELD ** form_fields (f)
FORM * f;
{
	return Form (f) -> field;
}

	/****************
	*  field_count  *
	****************/

int field_count (f)
FORM * f;
{
	return Form (f) -> maxfield;
}

	/***************
	*  scale_form  *
	***************/

int scale_form (f, rows, cols)
FORM * f;
int * rows;
int * cols;
{
	if (! f)
		return E_BAD_ARGUMENT;

	if (! f -> field)
		return E_NOT_CONNECTED;

	*rows = f -> rows;
	*cols = f -> cols;
	return E_OK;
}

	/****************
	*  data_behind  *
	****************/

BOOLEAN
data_behind( f )
FORM	*f;
{
	return OneRow(C(f)) ? B(f) != 0 : T(f) != 0;
}

    /****************
    *  _data_ahead  *
    ****************/

static
char * _data_ahead (v, pad, n)
char * v;
char pad;
int n;
{
/*
    return ptr to last non-pad char in v[n] (v on failure)
*/
    char * vend = v + n;
    while (vend > v && *(vend - 1) == pad ) --vend;
    return vend;
}

	/***************
	*  data_ahead  *
	***************/

BOOLEAN
data_ahead( f )
FORM	*f;
{
	static char	buf[ MAX_BUF ];
	char		*bptr = buf;
	WINDOW		*w = W(f);
	FIELD		*c = C(f);
	int			ret = FALSE;
	int			pad = Pad(c);
	int			cols = c->cols;
	int			dcols;
	int			drows;
	int			flag = cols > MAX_BUF - 1;
	int			start;
	int			chunk;
	int			i;

	if ( flag )
		bptr = malloc( cols + 1 );

	if ( OneRow( c ))
	{
		dcols = c->dcols;
		start = B(f) + cols;

		while ( start < dcols )
		{
			chunk = MIN( cols, dcols - start );
			(void)wmove( w, 0, start );
			(void)winnstr( w, bptr, chunk );

			if ( bptr != _data_ahead( bptr, pad, chunk ))
			{
				ret = TRUE;
				break;
			}

			start += cols;
		}
	}
	else	/* else multi-line field */
	{
		drows = c->drows;
		start = T(f) + c->rows;
	
		while ( start < drows )
		{
			(void)wmove( w, start++, 0 );
			(void)winnstr( w, bptr, cols );
	
			if ( bptr != _data_ahead( bptr, pad, cols ))
			{
				ret = TRUE;
				break;
			}
		}
	}
	
	if ( flag )
		(void)free( bptr );

	(void)wmove( w, Y(f), X(f));
	return ret;
}
