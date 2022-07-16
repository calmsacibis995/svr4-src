/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/chg_data.c	1.5"

#include "utility.h"

#define AT_BOTTOM(f)	(Y(f)==Ymax(f)-1)			/* last line	*/
#define AT_END(f)	(Y(f)==Ymax(f)-1 && X(f)==Xmax(f)-1)	/* last char	*/
#define AT_BEGINNING(f)	(Y(f)==0 && X(f)==0)			/* first char	*/

	/******************
	*  room_for_line  *
	******************/

static int room_for_line (f)
FORM * f;
{
	char * v;

	_sync_buffer (f);
	v = LineBuf (C(f), Ymax(f)-1);
	return v == _data_end (v, Xmax(f));	/* check for empty line */
}

	/******************
	*  room_for_char  *
	******************/

static int room_for_char (f)
FORM * f;
{
	WINDOW * w = W(f);
	int c;

	(void)wmove (w, Y(f), Xmax(f) - 1);
	c = winch (w) & A_CHARTEXT;
	(void)wmove (w, Y(f), X(f));
	return c == Pad (C(f));			/* check for empty char */
}

	/******************
	*  extra_padding  *
	******************/

static int extra_padding (str, nstr)		/* used for word wrapping */
char * str;
int nstr;
{
	int c = *(str + nstr - 1);

	if (c == '"' || c == '\'')
		c = *(str + nstr - 2);

	return (c == '.' || c == '?' || c == '!' || c == ':') ? 2 : 1;

}

	/***************
	*  grow_field  *
	***************/

BOOLEAN
_grow_field( c, chunks )
FIELD	*c;
int		chunks;
{
	/* This function handles the growth of dymanically growable fields	*/
	/* Returns TRUE if successful, FALSE otherwise						*/

	FORM		*f = c->form;
	WINDOW		*w = W( f );
	BOOLEAN		current = Status( f, POSTED ) && c == C( f );
	char		*old_buf;
	char		*new_buf;
	char		*save;
	int			old_len = BufSize( c );
	int			grow;
	int			lcv;
	int			max = c->maxgrow;
	int			i;

	if ( current && Status( f, WIN_CHG ))
	{
		_win_to_buf( w, c );
		Clr( f, WIN_CHG );
		Set( f, BUF_CHG );
	}

	if ( OneRow( c ))
	{
		grow = chunks * c->cols;

		if ( max )
			grow = MIN( max - c->dcols, grow );

		c->dcols += grow;

		if ( c->dcols == max )
			Clr( c, GROWABLE );
	}
	else
	{
		grow = chunks * (c->rows + c->nrow);

		if ( max )
			grow = MIN( max - c->drows, grow );

		c->drows += grow;
		grow *= c->cols;

		if ( c->drows == max )
			Clr( c, GROWABLE );
	}

	save = old_buf = Buf( c );
	new_buf = Buf( c ) = malloc( TotalBuf( c ));

	if ( !new_buf )
		return FALSE;

	lcv = c->nbuf + 1;

	for ( i = 0; i < lcv; i++ )
	{
		(void)memcpy( new_buf, old_buf, old_len );
		(void)memset( new_buf + old_len, ' ', grow );
		old_buf += old_len + 1;
		new_buf += old_len + grow;
		*new_buf++ = '\0';
	}

	free( save );								/* delete old buffer */

	if ( current )
	{
		delwin( w );
		W( f ) = w = newwin( c->drows, c->dcols, 0, 0 );

		if ( !w )
			return FALSE;

		wbkgdset( w, Pad( c ) | Back( c ));
		(void)wattrset( w, Fore( c ));
		(void)werase( w );
		_buf_to_win( c, w );
		(void)untouchwin( w );
		(void)wmove( w, Y(f), X(f));
	}

	if ( c->link != c )
	{
		FIELD	*p = c->link;

		while ( p != c )
		{
			Buf( p ) = Buf( c );
			p->drows = c->drows;
			p->dcols = c->dcols;
			/* _sync_field( p ) */
			p = p->link;
		}
	}

	return TRUE;
}

	/***************
	*  insert_str  *
	***************/

static int insert_str (f, y, off, nstr)		/* used for word wrapping */
FORM * f;
int y;
int off;
int nstr;
{
	WINDOW *	w	= W(f);
	FIELD *		c = C(f);
	char *		vbeg	= LineBuf (c, y);
	char *		v	= _data_end (vbeg, Xmax(f));
	int		x	= v - vbeg;
	int		n	= Xmax(f) - x;
	int		pad	= extra_padding (Buf(c) + off, nstr);
	int		siz	= nstr + 1 + pad;
	int		ret = E_REQUEST_DENIED;

	if (n >= siz)	/* check for fit on this line */
	{
		(void)wmove (w, y, 0);
		(void)winsnstr (w, Buf(c) + off, nstr);
		(void)wmove (w, y, nstr);
		(void)winsnstr (w, "  ", pad);
	}
	else		/* wrap */
	{
		if ( y == Ymax(f)-1 && Status( c, GROWABLE ))
		{
			if ( !_grow_field( c, 1 ))
				return E_SYSTEM_ERROR;

			vbeg = LineBuf( c, y );			/* grow changes buffer */
			w = W(f);						/* grow changes window */
		}

		v = _whsp_end (
			vbeg,
			_data_beg (vbeg + Xmax(f) - siz, siz) - vbeg
		);
		x = v - vbeg;
		n = Xmax(f) - x - n;

		if (y < Ymax(f)-1 && (ret = insert_str (f, y+1, v - Buf(c), n)) == E_OK)
		{
			(void)wmove (w, y, x);
			(void)wclrtoeol (w);
			(void)wmove (w, y, 0);
			(void)winsnstr (w, Buf(c) + off, nstr);
			(void)wmove (w, y, nstr);
			(void)winsnstr (w, "  ", pad);
		}
		else
			return ret;	/* no room for wrap */
	}
	return E_OK;
}

	/************
	*  wrap_ok  *
	************/

static int wrap_ok (f)				/* used for word wrapping */
FORM * f;
{
/*
	when this routine is called a char has already been added/inserted
	on the screen at Y(f), X(f).  this routine checks to see if the current
	line needs wrapping and if so attempts the wrap.  if unsuccessful
	it deletes the char at Y(f), X(f) and returns FALSE.
*/
	FIELD *		c = C(f);
	BOOLEAN		at_bottom = AT_BOTTOM(f);
	int			ret = E_REQUEST_DENIED;

	if (	Opt (c, O_WRAP)
	&&	! OneRow(c)
	&&	! room_for_char (f)
	&&	(!at_bottom || Status( c, GROWABLE )))
	{
		WINDOW *	w;
		char * vbeg;
		char * v;
		int x, n;

		if ( at_bottom && !_grow_field( c, 1 ))
			return E_SYSTEM_ERROR;

		vbeg = LineBuf( c, Y(f));
		w = W(f);

		_win_to_buf (w, c);	/* sync buffer without changing flags */

		v = _whsp_end (vbeg, Xmax(f));
		x = v - vbeg;
		n = Xmax(f) - x;

		if (x && (ret = insert_str (f, Y(f)+1, v - Buf(c), n)) == E_OK )
		{
			w = W( f );					/* window may change in insert_str */

			(void)wmove (w, Y(f), x);
			(void)wclrtoeol (w);

			if (X(f) >= x)
			{
				++Y(f);
				X(f) = X(f) - x;
			}
		}
		else	/* error condition */
		{
			if ( ret == E_SYSTEM_ERROR )
				return E_SYSTEM_ERROR;

			(void)wmove (w, Y(f), X(f));
			(void)wdelch (w);		/* delete the char */
			_win_to_buf (w, c);	/* restore buffer  */
			return E_REQUEST_DENIED;
		}
	}
	return E_OK;
}

	/****************
	*  wrap_buffer  *
	****************/
/*
	the following routine is not currently used

static int wrap_buffer (f)	/ wrap text in buffer /
FIELD * f;
{
	char *		v = Buf (f);
	char *		vbeg;
	char *		str;
	char *		t;
	int		x = 0;
	int		y = 0;
	int		xmax = Xmax( f );
	int		ymax = Ymax( f );
	int		s = f -> cols;
	int		n;

	if (! arrayAlloc (str, xmax * ymax + 1, char))
		return E_SYSTEM_ERROR;

	while (*v)
	{
		while (*v && *v == ' ') ++v;	/ beginning of word	/
		vbeg = v;
		while (*v && *v != ' ') ++v;	/ end of word		/
		n = v - vbeg;

		if (n)
		{
			if (s <= n)
			{
				if (++y < ymax && n < xmax)
				{
					x = 0;
					s = xmax;
				}
				else
					break;
			}
			t = str + (y * xmax) + x;
			strncpy (t, vbeg, n);
			n += extra_padding (vbeg, n);
			s -= n;
			x += n;
		}
	}
	if (*v)
	{
		free (str);
		return E_REQUEST_DENIED;;
	}
	else
	{
		(void)set_field_buffer (f, 0, str);
		free (str);
		return E_OK;
	}
}
*/
	/**************
	*  _new_line  *
	**************/

int _new_line (f)
FORM * f;
{
/*
		overloaded operation

	if at beginning of field
		move to next field

	else if in OVERLAY mode
		if on last line of field
			clear to eol and move to next field
		else
			clear to eol and move to beginning of next line

	else if in INSERT mode
		if on last line of field
			move to next field
		else
			move text from cursor to eol to new line
*/
	BOOLEAN		at_bottom = AT_BOTTOM( f );
	FIELD *		c = C(f);

	if (Opt (f, O_NL_OVERLOAD) && AT_BEGINNING (f))
		return _field_navigation (_next_field, f);

	if (! Opt (c, O_EDIT))
		return E_REQUEST_DENIED;

	if (Status (f, OVERLAY))		/* OVERLAY mode	*/
	{
		if (at_bottom && (!Status( c, GROWABLE ) || OneRow(c)))
		{
			if (Opt (f, O_NL_OVERLOAD))
			{
				(void)wclrtoeol (W(f));
				Set (f, WIN_CHG);
				return _field_navigation (_next_field, f);
			}
			else
				return E_REQUEST_DENIED;
		}

		if ( at_bottom && !_grow_field( c, 1 ))
			return E_SYSTEM_ERROR;

		(void)wclrtoeol (W(f));
		++Y(f); X(f) = 0;
	}
	else					/* INSERT mode	*/
	{
		BOOLEAN		room;

		if (at_bottom && (!Status( c, GROWABLE ) || OneRow(c)))
		{
			if (Opt (f, O_NL_OVERLOAD))
				return _field_navigation (_next_field, f);
			else
				return E_REQUEST_DENIED;
		}

		room = !at_bottom && room_for_line( f );

		if (room || Status( c, GROWABLE ))
		{
			WINDOW	*w;
			char *	v;
			char *	vend;

			if ( !room && !_grow_field( c, 1 ))
				return E_SYSTEM_ERROR;

			w = W(f);
			v = LineBuf (c, Y(f)) + X(f);
			vend = _data_end (v, Xmax(f) - X(f));

			(void)wclrtoeol (w);
			++Y(f); X(f) = 0;
			(void)wmove (w, Y(f), X(f));
			(void)winsertln (w);
			(void)waddnstr (w, v, vend - v);
		}
		else
			return E_REQUEST_DENIED;
	}
	Set (f, WIN_CHG);
	return E_OK;
}

	/**************
	*  _ins_char  *
	**************/

int _ins_char (f)
FORM * f;
{
/*
	insert blank char with error on overflow
*/
	FIELD	*c = C(f);
	BOOLEAN	room = room_for_char(f);

	if ( CheckChar (c, ' ')
		 &&	(room || (OneRow(c)
					  && Status(c, GROWABLE))))
	{
		if ( !room && !_grow_field(c, 1))
			return E_SYSTEM_ERROR;

		(void)winsch (W(f), ' ');

		return wrap_ok( f );
	}
	return E_REQUEST_DENIED;
}

	/**************
	*  _ins_line  *
	**************/

int _ins_line (f)
FORM * f;
{
/*
	insert blank line with error on overflow
*/
	BOOLEAN		room = !AT_BOTTOM(f) && room_for_line( f );
	FIELD		*c = C(f);

	if (	CheckChar (c, ' ')
			&&	!OneRow( c )
			&&	( room || Status( c, GROWABLE )))
	{
		if ( !room && !_grow_field( c, 1 ))
			return E_SYSTEM_ERROR;

		X(f) = 0;
		(void)winsertln (W(f));
		return E_OK;
	}
	return E_REQUEST_DENIED;
}

	/**************
	*  _del_char  *
	**************/

int _del_char (f)
FORM * f;
{
/*
	delete char at cursor
*/
	(void)wdelch (W(f));
	return E_OK;
}

	/**************
	*  _del_prev  *
	**************/

int _del_prev (f)
FORM * f;
{
/*
		overloaded operation

	if at beginning of field
		move to previous field

	else if in OVERLAY mode
		if at beginning of line
			error
		else
			delete previous char

	else if in INSERT mode
		if at beginning of line
			if current line can fit on preceding
				join current line with preceding line
			else
				error
		else
			delete previous char
*/
	WINDOW *	w = W(f);
	FIELD *		c = C(f);

	if (AT_BEGINNING (f))
	{
		if (Opt (f, O_BS_OVERLOAD))
			return _field_navigation (_prev_field, f);
		else
			return E_REQUEST_DENIED;
	}
	if (! Opt (c, O_EDIT))
		return E_REQUEST_DENIED;

	if (--X(f) < 0)
	{
		++X(f);

		if (Status (f, OVERLAY))	/* OVERLAY mode	*/
			return E_REQUEST_DENIED;
		else				/* INSERT mode	*/
		{
			char *	p = LineBuf (c, Y(f) - 1);
			char *	v = LineBuf (c, Y(f));
			char *	pend;
			char *	vend;

			_sync_buffer (f);
			pend = _data_end (p, Xmax(f));
			vend = _data_end (v, Xmax(f));

			if ((vend - v) > (Xmax(f) - (pend - p)))
				return E_REQUEST_DENIED;
			else
			{
				(void)wdeleteln (w);
				_adjust_cursor (f, pend);
				(void)wmove (w, Y(f), X(f));
				(void)waddnstr (w, v, (vend - v));
			}
		}
	}
	else
	{
		(void)wmove (w, Y(f), X(f));
		(void)wdelch (w);
	}
	Set (f, WIN_CHG);
	return E_OK;
}

	/**************
	*  _del_line  *
	**************/

int _del_line (f)
FORM * f;
{
/*
	delete current line
*/
	X(f) = 0;
	(void)wdeleteln (W(f));
	return E_OK;
}

	/**************
	*  _del_word  *
	**************/

int _del_word (f)
FORM * f;
{
/*
	delete word under cursor plus trailing blanks
*/
	FIELD *		c = C(f);
	WINDOW *	w = W(f);
	char *		y = LineBuf (c, Y(f));
	char *		t = y + Xmax(f);
	char *		v = y + X(f);
	char *		x = v;

	_sync_buffer (f);

	if (*v == ' ')
		return E_REQUEST_DENIED;

	_adjust_cursor (f, _whsp_end (y, X(f)));
	(void)wmove (w, Y(f), X(f));
	(void)wclrtoeol (w);

	v = _whsp_beg (v, t - v);
	v = _data_beg (v, t - v);

	if (v != x && *v != ' ')
		(void)waddnstr (w, v, _data_end (v, t - v) - v);

	return E_OK;
}

	/*************
	*  _clr_eol  *
	*************/

int _clr_eol (f)
FORM * f;
{
/*
	clear to end of line
*/
	(void)wclrtoeol (W(f));
	return E_OK;
}

	/*************
	*  _clr_eof  *
	*************/

int _clr_eof (f)
FORM * f;
{
/*
	clear to end of field
*/
	(void)wclrtobot (W(f));
	return E_OK;
}

	/***************
	*  _clr_field  *
	***************/

int _clr_field (f)
FORM * f;
{
/*
	clear entire field
*/
	X(f) = 0; Y(f) = 0;
	(void)werase (W(f));
	return E_OK;
}

	/**************
	*  _ovl_mode  *
	**************/

int _ovl_mode (f)
FORM * f;
{
/*
	go into overlay mode
*/
	Set (f, OVERLAY);
	return E_OK;
}

	/**************
	*  _ins_mode  *
	**************/

int _ins_mode (f)
FORM * f;
{
/*
	go into insert mode
*/
	Clr (f, OVERLAY);
	return E_OK;
}

	/****************
	*  _validation  *
	****************/

int _validation (f)
FORM * f;
{
/*
	apply validation function associated with field type
*/
	return _validate (f) ? E_OK : E_INVALID_FIELD;
}

	/*****************
	*  _next_choice  *
	*****************/

int _next_choice (f)
FORM * f;
{
/*
	apply next choice function associated with field type
*/
	_sync_buffer (f);
	return NextChoice (C(f)) ? E_OK : E_REQUEST_DENIED;
}

	/*****************
	*  _prev_choice  *
	*****************/

int _prev_choice (f)
FORM * f;
{
/*
	apply previous choice function associated with field type
*/
	_sync_buffer (f);
	return PrevChoice (C(f)) ? E_OK : E_REQUEST_DENIED;
}

	/****************
	*  _data_entry  *
	****************/

int _data_entry (f, ch)
FORM * f;
int ch;
{
/*
	enter printable ascii char ch in current field at cursor position
*/
	FIELD *		c = C(f);	/* current field	*/
	WINDOW *	w = W(f);	/* field window		*/
	BOOLEAN		at_end;
	int			ret;

	if (! Opt (c, O_EDIT))
		return E_REQUEST_DENIED;

	if (	AT_BEGINNING (f)
	&&	Opt (c, O_BLANK)
	&&	! Status (f, BUF_CHG)
	&&	! Status (f, WIN_CHG)	)

		(void)werase (w);

	if (Status (f, OVERLAY))	/* OVERLAY mode	*/
		(void)waddch (w, (chtype) ch);
	else				/* INSERT mode	*/
	{
		BOOLEAN	room = room_for_char(f);

		if (room || (OneRow(c) && Status(c, GROWABLE)))
		{
			if ( !room && !_grow_field(c, 1))
				return E_SYSTEM_ERROR;

			(void)winsch (w, (chtype) ch);
		}
		else
			return E_REQUEST_DENIED;
	}

	if ((ret = wrap_ok( f )) != E_OK )
		return ret;

	Set (f, WIN_CHG);

	at_end = AT_END( f );

	if (at_end && !Status( c, GROWABLE ) && Opt (c, O_AUTOSKIP))
		return _field_navigation (_next_field, f);

	if ( at_end && Status( c, GROWABLE ) && !_grow_field( c, 1 ))
		return E_SYSTEM_ERROR;

	(void)_next_char (f);
	return E_OK;
}
