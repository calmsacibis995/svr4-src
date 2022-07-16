/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/utility.c	1.7"

#include "utility.h"

#define Scrollable(f)		((f)->drows > (f)->rows || (f)->dcols > (f)->cols)
#define Connected(f)		((f) -> form != (FORM *) 0)
#define OnPage(f)		((f) -> page == P((f) -> form))
#define Posted(f)		(Status ((f) -> form, POSTED))
#define Visible(f)		(Opt (f, O_VISIBLE) && OnPage(f))
#define isCurrent(f)		((f) == C((f) -> form))
#define Justified(f)	(												\
							Just (f) != NO_JUSTIFICATION	&&			\
							OneRow(f) && Opt( f, O_STATIC ) &&			\
							f->dcols == f->cols							\
						)

	/**************
	*  _data_beg  *
	**************/

char * _data_beg (v, n)
char * v;
int n;
{
/*
	return ptr to first non-blank char in v[n] (v on failure)
*/
	char * vend = v + n;
	while (v < vend && *v == ' ') ++v;
	return v == vend ? v - n : v;
}

	/**************
	*  _data_end  *
	**************/

char * _data_end (v, n)
char * v;
int n;
{
/*
	return ptr to char after last non-blank char in v[n] (v on failure)
*/
	char * vend = v + n;
	while (vend > v && *(vend-1) == ' ') --vend;
	return vend;
}

	/**************
	*  _whsp_beg  *
	**************/

char * _whsp_beg (v, n)
char * v;
int n;
{
/*
	return ptr to first blank in v[n] (v on failure)
*/
	char * vend = v + n;
	while (v < vend && *v != ' ') ++v;
	return v == vend ? v - n : v;
}

	/**************
	*  _whsp_end  *
	**************/

char * _whsp_end (v, n)
char * v;
int n;
{
/*
	return ptr to char after last blank in v[n] (v on failure)
*/
	char * vend = v + n;
	while (vend > v && *(vend-1) != ' ') --vend;
	return vend;
}

	/*******************
	*  _adjust_cursor  *
	*******************/

void _adjust_cursor (f, v)
FORM * f;
char * v;
{
/*
	adjust cursor based on offset of v from beginning of field buffer
*/
	int pos = v - Buf (C(f));

	Y(f) = pos / Xmax(f);
	X(f) = pos - Y(f) * Xmax(f);

	if (Y(f) >= Ymax(f))
		Y(f) = 0;
}

	/****************
	*  _buf_to_win  *
	****************/

void _buf_to_win (f, w)
FIELD * f;
WINDOW * w;
{
/*
	copy buffer to window (trailing blanks on each line are not copied)
*/
	char *	v = Buf (f);
	int	xmax, ymax, y, n;

	getmaxyx (w, ymax, xmax);

	for (y = 0; y < ymax; ++y)
	{
		if (n = _data_end (v, xmax) - v)
		{
			(void)wmove (w, y, 0);
			(void)waddnstr (w, v, n);
		}
		v += xmax;
	}
}

	/****************
	*  _win_to_buf  *
	****************/

void _win_to_buf (w, f)
WINDOW * w;
FIELD * f;
{
/*
	copy window to buffer
*/
	int		i;
	int		size	= BufSize (f);
	int		pad	= Pad (f);
	char *		v	= Buf (f);

	(void)wmove (w, 0, 0);
	(void)winnstr (w, v, size);

	if (pad != ' ')
		for (i = 0; i < size; ++i, ++v)
			if (*v == pad)
				*v = ' '; /* replace pad char with blank */
}

	/*********************
	*  _pos_form_cursor  *
	*********************/

int _pos_form_cursor (f)
FORM * f;
{
/*
	move to cursor position and sync up cursor
*/
	WINDOW *	w = W(f);
	FIELD *		c = C(f);

	if (! w)
		return E_SYSTEM_ERROR;

	(void)wmove (w, Y(f), X(f));

	if (Opt (c, O_PUBLIC))
	{
		if (Scrollable (c))
		{
			int row, col;

			if (OneRow(c))
			{
				row = c->frow;
				col = c->fcol + X(f) - B(f);
			}
			else
			{
				row = c -> frow + Y(f) - T(f);
				col = c -> fcol + X(f);
			}

			(void)wmove (
				Sub(f),
				row,
				col
			);
			wcursyncup (Sub(f));
		}
		else
			wcursyncup (w);
	}
	else
	{
		(void)wmove (Sub(f), c -> frow, c -> fcol);
		wcursyncup (Sub(f));
	}
	return E_OK;
}

	/********************
	*  _update_current  *
	********************/

int _update_current (f)
FORM * f;
{
/*
	sync up current field
*/
	WINDOW *	w = W(f);
	FIELD *		c = C(f);

	if (! w)
		return E_SYSTEM_ERROR;

	if (Opt (c, O_PUBLIC))
	{
		if (Scrollable (c))
		{
			if (OneRow( c ))
			{
				int xmax = B(f) + c->cols;

				if (X(f) < B(f))
					B(f) = X(f);
				else if (X(f) >= xmax)
					B(f) = X(f) - c->cols + 1;

				(void)copywin ( w,
								Sub(f),
								0,
								B(f),
								c->frow,
								c->fcol,
								c->frow,
								c->fcol + c->cols - 1,
								FALSE );

			}
			else
			{
				int	ymax = T(f) + c -> rows;
				int	ys, ye;
	
				if (Y(f) < T(f))
				{
					T(f) = Y(f);
					Set (c, TOP_CHG);
				}
				if (Y(f) >= ymax)
				{
					T(f) = Y(f) - c -> rows + 1;
					Set (c, TOP_CHG);
				}
				if (Status (c, TOP_CHG))
				{
					ys = T(f);
					ye = ys + c -> rows;
					Clr (c, TOP_CHG);
				}
				else
				{
	/*
					intersect changed lines with visible lines
	*/
					for (ys = T(f); ys < ymax; ++ys)
						if (is_linetouched (w, ys))
							break;
	
					for (ye = ys; ye < ymax; ++ye)
						if (! is_linetouched (w, ye))
							break;
				}
				if (ye - ys)
				{
					(void)copywin (
						w,
						Sub(f),
						ys,
						0,
						c -> frow + ys - T(f),
						c -> fcol,
						c -> frow + ye - T(f) - 1,
						c -> fcol + c -> cols - 1,
						FALSE
					);
				}
			}
			wsyncup (Sub(f));
		}
		else
			wsyncup (w);
	}
	(void)untouchwin (w);
	return _pos_form_cursor (f);
}

	/************
	*  justify  *
	************/

static void justify (f, w)
FIELD * f;
WINDOW * w;
{
/*
	justify field f in window w as given by Just (f)
*/
	char *	v	= _data_beg (Buf (f), BufSize (f));
	char *	vend	= _data_end (Buf (f), BufSize (f));
	int	n	= vend - v;
	int	x	= 0;

	if (n)
	{
		switch (Just (f))
		{
			case JUSTIFY_LEFT:
				break;
			case JUSTIFY_CENTER:
				x = (f -> cols - n) / 2;
				break;
			case JUSTIFY_RIGHT:
				x = f -> cols - n;
				break;
		}
		(void)wmove (w, 0, x);
		(void)waddnstr (w, v, n);
	}
}

	/**************
	*  unjustify  *
	**************/

static void unjustify (f, w)
FIELD * f;
WINDOW * w;
{
/*
	left justify field f in window w for editing
*/
	char *	v	= _data_beg (Buf (f), BufSize (f));
	char *	vend	= _data_end (Buf (f), BufSize (f));
	int	n	= vend - v;

	if (n)
	{
		(void)wmove (w, 0, 0);
		(void)waddnstr (w, v, n);
	}
}

	/*****************
	*  _sync_buffer  *
	*****************/

void _sync_buffer (f)
FORM * f;
{
/*
	sync current field with characters in window
*/
	if (Status (f, WIN_CHG))
	{
		Clr (f, WIN_CHG);
		Set (f, BUF_CHG);
		_win_to_buf (W(f), C(f));
		(void)wmove (W(f), Y(f), X(f));
	}
}

	/*****************
	*  _sync_linked  *
	*****************/

int _sync_linked (f)
FIELD * f;
{
/*
	sync fields linked to field f
*/
	FIELD *		p = f -> link;
	int		err = 0;

	while (p != f)
	{
		if (_sync_field (p) != E_OK)
			++err;
		p = p -> link;
	}
	return err ? E_SYSTEM_ERROR : E_OK;
}

	/******************
	*  display_field  *
	******************/

static int display_field (f)
FIELD * f;
{
/*
	display field f
*/
	WINDOW *	w = derwin (
					Sub(f -> form),
					f -> rows,
					f -> cols,
					f -> frow,
					f -> fcol
			);

	if (! w)
		return FALSE;

	wbkgdset (w, Pad (f) | Back (f));
	(void)wattrset (w, Fore (f));
	(void)werase (w);

	if (Opt (f, O_PUBLIC))
	{
		if (Justified (f))
			justify (f, w);
		else
			_buf_to_win (f, w);
	}
	wsyncup (w);
	(void)delwin (w);
	Clr (f, TOP_CHG);
	return TRUE;
}

	/****************
	*  erase_field  *
	****************/

static int erase_field (f)
FIELD * f;
{
/*
	erase field f
*/
	WINDOW *	w = derwin (
					Sub(f -> form),
					f -> rows,
					f -> cols,
					f -> frow,
					f -> fcol
			);

	if (! w)
		return FALSE;

	(void)werase (w);
	wsyncup (w);
	(void)delwin (w);
	return TRUE;
}

	/****************
	*  _sync_field  *
	****************/

int _sync_field (f)
FIELD * f;
{
/*
	sync the field after a change to the field buffer
*/
	int v = TRUE;

	if (Connected (f) && Posted (f) && Visible (f))
	{
		if (isCurrent (f))
		{
			FORM *		p = f -> form;
			WINDOW *	w = W(p);

			Clr (p, WIN_CHG | BUF_CHG);

			Y(p) = 0;
			X(p) = 0;
			T(p) = 0;
			B(p) = 0;
			(void)werase (w);

			if (Opt (f, O_PUBLIC) && Justified (f))
				unjustify (f, w);
			else
				_buf_to_win (f, w);

			Set (f, TOP_CHG);
			(void)_update_current (p);
		}
		else
			v = display_field (f);
	}
	Set (f, USR_CHG);

	return v ? E_OK : E_SYSTEM_ERROR;
}

	/****************
	*  _sync_attrs  *
	****************/

int _sync_attrs (f)
FIELD * f;
{
/*
	sync the field after a change to a field attribute
*/
	int v = TRUE;

	if (Connected (f) && Posted (f) && Visible (f))
	{
		if (isCurrent (f))
		{
			FORM *		p = f -> form;
			WINDOW *	w = W(p);

			_sync_buffer (p);

			wbkgdset (w, Pad (f) | Back (f));
			(void)wattrset (w, Fore (f));
			(void)werase (w);

			if (Opt (f, O_PUBLIC))
			{
				if (Justified (f))
					unjustify (f, w);
				else
					_buf_to_win (f, w);
			}
			else
			{
				(void)copywin (
					w,
					Sub(p),
					0,
					0,
					f -> frow,
					f -> fcol,
					f -> rows - 1,
					f -> cols - 1,
					FALSE
				);
				wsyncup (Sub(p));
				_buf_to_win (f, w);
			}
			Set (f, TOP_CHG);
			(void)_update_current (p);
		}
		else
			v = display_field (f);
	}
	return v ? E_OK : E_SYSTEM_ERROR;
}

	/***************
	*  _sync_opts  *
	***************/

int _sync_opts (f, opts)
FIELD * f;
OPTIONS opts;
{
	int		v = TRUE;
	OPTIONS		oldopts = f -> opts;
	OPTIONS x = opts ^ oldopts;
/*
	x & opt indicates option opt has changed state
*/
	f -> opts = opts;

	if (Connected (f))
	{
		if (isCurrent (f))
		{
			f -> opts = oldopts;
			return E_CURRENT;
		}
		if (Posted (f) && OnPage (f))
		{
			if (x & O_VISIBLE)
			{
				if (Opt (f, O_VISIBLE))
					v = display_field (f);
				else
					v = erase_field (f);
			}
			else if (x & O_PUBLIC)
			{
				if (Opt (f, O_VISIBLE))
					v = display_field (f);
			}
		}
	}

	if ( x & O_STATIC )
	{
		BOOLEAN	onerow = OneRow(f);
		int		max = f->maxgrow;

		if ( Opt( f, O_STATIC ))		/* growth being turned off */
		{
			Clr( f, GROWABLE );

			if ( onerow
				&& f->cols == f->dcols
				&& Just(f) != NO_JUSTIFICATION
				&& Posted(f)
				&& OnPage(f)
				&& Opt(f, O_VISIBLE) )
			{
				display_field( f );
			}
		}
		else if ( !max || (onerow && f->dcols < max )
					   || (!onerow && f->drows < max ))
		{
			Set( f, GROWABLE );

			if ( onerow
				&& Just(f) != NO_JUSTIFICATION
				&& Posted(f)
				&& OnPage(f)
				&& Opt(f, O_VISIBLE) )
			{
				display_field( f );
			}
		}
	}

	return v ? E_OK : E_SYSTEM_ERROR;
}

	/**************
	*  _validate  *
	**************/

int _validate (f)
FORM * f;
{
/*
	validate current field
*/
	FIELD * c = C(f);

	_sync_buffer (f);

	if (Status (f, BUF_CHG) || ! Opt (c, O_PASSOK))
	{
		if (CheckField (c))
		{
			Clr (f, BUF_CHG);
			Set (c, USR_CHG);
			(void)_sync_linked (c);
		}
		else
			return FALSE;
	}
	return TRUE;
}

	/***********************
	*  _set_current_field  *
	***********************/

int _set_current_field (f, field)
FORM * f;
FIELD * field;
{
/*
	change current field on form f to given field.
	POSTED flag is set unless this is called from post_form().
*/
	WINDOW *	w = W(f);
	FIELD *		c = C(f);

	if (c != field || ! Status (f, POSTED))
	{
		if (w)
		{
			if (Visible (c))
			{
				(void)_update_current (f);

				if (Opt (c, O_PUBLIC))
				{
					if (Scrollable (c))
					{
						if (T(f) == 0)
							Clr (c, TOP_CHG);
						else
							Set (c, TOP_CHG);
					}
					else if (Justified (c))
					{
						(void)werase (w);
						justify (c, w);
						wsyncup (w);
					}
				}
			}
			(void)delwin (w);
		}
		c = field;

		if (! Opt (c, O_PUBLIC) || Scrollable (c))
			w = newwin (
				c -> drows,
				c -> dcols,
				0,
				0
			);
		else
			w = derwin (
				Sub (f),
				c -> rows,
				c -> cols,
				c -> frow,
				c -> fcol
			);

		if (! w)
			return E_SYSTEM_ERROR;

		C(f) = c;
		W(f) = w;
		wbkgdset (w, Pad (c) | Back (c));
		(void)wattrset (w, Fore (c));

		if (! Opt (c, O_PUBLIC) || Scrollable (c))
		{
			(void)werase (w);
			_buf_to_win (c, w);
		}
		else if (Justified (c))
		{
			(void)werase (w);
			unjustify (c, w);
			wsyncup (w);
		}
		(void)untouchwin (w);
		Clr (f, WIN_CHG | BUF_CHG);
	}
	Y(f) = 0;
	X(f) = 0;
	T(f) = 0;
	B(f) = 0;
	return E_OK;
}

	/*******************
	*  _set_form_page  *
	*******************/

int _set_form_page (f, page, c)
FORM * f;
int page;
FIELD * c;
{
/*
	display given page and set current field to c.
	if c is null, then set current field to first field on page.
	POSTED flag is set unless this is called from post_form().
*/
	if (P(f) != page || ! Status (f, POSTED))
	{
		FIELD * x = f -> field [Smin(f, page)];
		FIELD * p = x;

		(void)werase (Sub (f));
		P(f) = page;

		do
		{
			if (Opt (p, O_VISIBLE))
				if (! display_field (p))
					return E_SYSTEM_ERROR;
			p = p -> snext;

		} while (p != x);

		return c ? _set_current_field (f, c) : _first_field (f);
	}
	return E_OK;
}
