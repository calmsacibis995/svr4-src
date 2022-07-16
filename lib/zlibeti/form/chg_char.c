/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/chg_char.c	1.5"

#include "utility.h"

#define SizePrev(f,v)	((v) - Buf(f))			/* from beginning to v	*/
#define SizeNext(f,v)	(BufSize(f) - SizePrev(f, v))	/* from v through end	*/
#define	OffscreenRows(c)	((c)->drows - (c)->rows)
#define	OffscreenCols(c)	((c)->dcols - (c)->cols)

	/***************
	*  _next_char  *
	***************/

int _next_char (f)
FORM * f;
{
/*
	move to next char with wrap to next line at end of line
*/
	if (++X(f) == Xmax(f))
	{
		if (++Y(f) == Ymax(f))
		{
			--X(f);
			--Y(f);
			return E_REQUEST_DENIED;	/* at last char */
		}
		X(f) = 0;
	}
	return E_OK;
}

	/***************
	*  _prev_char  *
	***************/

int _prev_char (f)
FORM * f;
{
/*
	move to previous char with wrap to previous line at beginning of line
*/
	if (--X(f) < 0)
	{
		if (--Y(f) < 0)
		{
			++X(f);
			++Y(f);
			return E_REQUEST_DENIED;	/* at first char */
		}
		X(f) = Xmax(f) - 1;
	}
	return E_OK;
}

	/***************
	*  _next_line  *
	***************/

int _next_line (f)
FORM * f;
{
/*
	move to beginning of next line
*/
	if (++Y(f) == Ymax(f))
	{
		--Y(f);
		return E_REQUEST_DENIED;	/* at last line */
	}
	X(f) = 0;
	return E_OK;
}

	/***************
	*  _prev_line  *
	***************/

int _prev_line (f)
FORM * f;
{
/*
	move to beginning of previous line
*/
	if (--Y(f) < 0)
	{
		++Y(f);
		return E_REQUEST_DENIED;	/* at first line */
	}
	X(f) = 0;
	return E_OK;
}

	/***************
	*  _next_word  *
	***************/

int _next_word (f)
FORM * f;
{
/*
	move to beginning of next word
*/
	FIELD *		c = C(f);
	char *		v = LineBuf (c, Y(f)) + X(f);	/* position in buffer */
	char *		t;

	_sync_buffer (f);

	t = _whsp_beg (v, SizeNext (c, v));
	v = _data_beg (t, SizeNext (c, t));

	if (v == t)
		return E_REQUEST_DENIED;	/* at last word */

	if ( OneRow( c ) && c->dcols != c->cols )  /* one row and field has grown */
	{
		t = v;

		while ( *t != ' ' && *t != '\0' )  /* find end of word + 1 */
			t++;

		if ( t - (Buf(c) + B(f)) > c->cols )
		{
			if ( t - v > c->cols )		/* word longer than visible field */
			{
				B(f) = v - Buf(c);
			}
			else
			{
				B(f) = t - (Buf(c) + c->cols);
			}

			X(f) = v - Buf(c);
			return E_OK;
		}
	}

	_adjust_cursor (f, v);
	return E_OK;
}

	/***************
	*  _prev_word  *
	***************/

int _prev_word (f)
FORM * f;
{
/*
	move to beginning of previous word
*/
	FIELD *		c = C(f);
	char *		v = LineBuf (c, Y(f)) + X(f);	/* position in buffer */
	char *		t;

	_sync_buffer (f);

	t = _data_end (Buf (c), SizePrev (c, v));
	v = _whsp_end (Buf (c), SizePrev (c, t));

	if (v == t)
		return E_REQUEST_DENIED;	/* at first word */

	_adjust_cursor (f, v);
	return E_OK;
}

	/***************
	*  _beg_field  *
	***************/

int _beg_field (f)
FORM * f;
{
/*
	move to first non-pad char in field
*/
	FIELD *	c = C(f);

	_sync_buffer (f);
	_adjust_cursor (f, _data_beg (Buf (c), BufSize (c)));
	return E_OK;
}

	/***************
	*  _end_field  *
	***************/

int _end_field (f)
FORM * f;
{
/*
	move after last non-pad char in field
*/
	FIELD *	c = C(f);
	char *	end;

	_sync_buffer (f);
	end = _data_end (Buf (c), BufSize (c));

	if ( end == Buf(c) + BufSize(c) )
		end--;

	_adjust_cursor (f, end );
	return E_OK;
}

	/**************
	*  _beg_line  *
	**************/

int _beg_line (f)
FORM * f;
{
/*
	move to first non-pad char on current line
*/
	FIELD *	c = C(f);

	_sync_buffer (f);
	_adjust_cursor (f, _data_beg (LineBuf (c, Y(f)), Xmax(f)));
	return E_OK;
}

	/**************
	*  _end_line  *
	**************/

int _end_line (f)
FORM * f;
{
/*
	move after last non-pad char on current line
*/
	FIELD *	c = C(f);
	char *	end;

	_sync_buffer (f);
	end = _data_end (LineBuf (c, Y(f)), Xmax(f));

	if ( end == LineBuf(c, Y(f)) + Xmax(f) )
		end--;

	_adjust_cursor (f, end );
	return E_OK;
}

	/***************
	*  _left_char  *
	***************/

int _left_char (f)
FORM * f;
{
/*
	move left
*/
	if (--X(f) < 0)
	{
		++X(f);
		return E_REQUEST_DENIED;	/* at left side */
	}
	return E_OK;
}

	/****************
	*  _right_char  *
	****************/

int _right_char (f)
FORM * f;
{
/*
	move right
*/
	if (++X(f) == Xmax(f))
	{
		--X(f);
		return E_REQUEST_DENIED;	/* at right side */
	}
	return E_OK;
}

	/*************
	*  _up_char  *
	*************/

int _up_char (f)
FORM * f;
{
/*
	move up
*/
	if (--Y(f) < 0)
	{
		++Y(f);
		return E_REQUEST_DENIED;	/* at top */
	}
	return E_OK;
}

	/***************
	*  _down_char  *
	***************/

int _down_char (f)
FORM * f;
{
/*
	move down
*/
	if (++Y(f) == Ymax(f))
	{
		--Y(f);
		return E_REQUEST_DENIED;	/* at bottom */
	}
	return E_OK;
}

	/***************
	*  _scr_fline  *
	***************/

int _scr_fline (f)
FORM * f;
{
/*
	scroll forward one line
*/
	FIELD * c = C(f);

	if (++T(f) > OffscreenRows( c ))
	{
		--T(f);
		return E_REQUEST_DENIED;	/* at bottom */
	}
	++Y(f);
	Set (c, TOP_CHG);
	return E_OK;
}

	/***************
	*  _scr_bline  *
	***************/

int _scr_bline (f)
FORM * f;
{
/*
	scroll backward one line
*/
	FIELD * c = C(f);

	if (--T(f) < 0)
	{
		++T(f);
		return E_REQUEST_DENIED;	/* at top */
	}
	--Y(f);
	Set (c, TOP_CHG);
	return E_OK;
}

	/***************
	*  _scr_fpage  *
	***************/

int _scr_fpage (f)
FORM * f;
{
/*
	scroll forward one page (C(f) -> rows)
*/
	FIELD *		c = C(f);
	int		m = OffscreenRows( c ) - T(f);
	int		n = c -> rows < m ? c -> rows : m;

	if (n)
	{
		Y(f) += n;
		T(f) += n;
		Set (c, TOP_CHG);
		return E_OK;
	}
	return E_REQUEST_DENIED;	/* at bottom */
}

	/***************
	*  _scr_bpage  *
	***************/

int _scr_bpage (f)
FORM * f;
{
/*
	scroll backward one page (C(f) -> rows)
*/
	FIELD *		c = C(f);
	int		m = T(f);
	int		n = c -> rows < m ? c -> rows : m;

	if (n)
	{
		Y(f) -= n;
		T(f) -= n;
		Set (c, TOP_CHG);
		return E_OK;
	}
	return E_REQUEST_DENIED;	/* at top */
}

	/***************
	*  _scr_fhpage  *
	***************/

int _scr_fhpage (f)
FORM * f;
{
/*
	scroll forward one half page (C(f)->rows + 1)/2)
*/
	FIELD *		c = C(f);
	int		m = OffscreenRows( c ) - T(f);
	int		h = (c->rows + 1)/2;
	int		n = h < m ? h : m;

	if (n)
	{
		Y(f) += n;
		T(f) += n;
		Set (c, TOP_CHG);
		return E_OK;
	}
	return E_REQUEST_DENIED;	/* at bottom */
}

	/***************
	*  _scr_bhpage  *
	***************/

int _scr_bhpage (f)
FORM * f;
{
/*
	scroll backward one half page (C(f)->rows + 1)/2)
*/
	FIELD *		c = C(f);
	int		m = T(f);
	int		h = (c->rows + 1)/2;
	int		n = h < m ? h : m;

	if (n)
	{
		Y(f) -= n;
		T(f) -= n;
		Set (c, TOP_CHG);
		return E_OK;
	}
	return E_REQUEST_DENIED;	/* at top */
}

	/***************
	*  _scr_fchar  *
	***************/

int _scr_fchar (f)
FORM * f;
{
/*
	horizontal scroll forward one char
*/
	FIELD * c = C(f);

	if (++B(f) > OffscreenCols( c ))
	{
		--B(f);
		return E_REQUEST_DENIED;	/* at end */
	}
	++X(f);
	return E_OK;
}

	/***************
	*  _scr_bchar  *
	***************/

int _scr_bchar (f)
FORM * f;
{
/*
	horizontal scroll backward one char
*/
	FIELD * c = C(f);

	if (--B(f) < 0)
	{
		++B(f);
		return E_REQUEST_DENIED;	/* at beginning */
	}
	--X(f);
	return E_OK;
}

	/****************
	*  _scr_hfline  *
	****************/

int _scr_hfline (f)
FORM * f;
{
/*
	horizontal scroll forward one line (C(f)->cols)
*/
	FIELD *		c = C(f);
	int		m = OffscreenCols( c ) - B(f);
	int		n = c -> cols < m ? c -> cols : m;

	if (n)
	{
		X(f) += n;
		B(f) += n;
		return E_OK;
	}
	return E_REQUEST_DENIED;	/* at end */
}

	/****************
	*  _scr_hbline  *
	****************/

int _scr_hbline (f)
FORM * f;
{
/*
	horizontal scroll backward one line (C(f)->cols)
*/
	FIELD *		c = C(f);
	int		m = B(f);
	int		n = c -> cols < m ? c -> cols : m;

	if (n)
	{
		X(f) -= n;
		B(f) -= n;
		return E_OK;
	}
	return E_REQUEST_DENIED;	/* at end */
}

	/****************
	*  _scr_hfhalf  *
	****************/

int _scr_hfhalf (f)
FORM * f;
{
/*
	horizontal scroll forward one half line (C(f)->cols/2)
*/
	FIELD *		c = C(f);
	int		m = OffscreenCols( c ) - B(f);
	int		h = (c->cols + 1)/2;
	int		n = h < m ? h : m;

	if (n)
	{
		X(f) += n;
		B(f) += n;
		return E_OK;
	}
	return E_REQUEST_DENIED;	/* at end */
}

	/****************
	*  _scr_hbhalf  *
	****************/

int _scr_hbhalf (f)
FORM * f;
{
/*
	horizontal scroll backward one half line (C(f)->cols/2)
*/
	FIELD *		c = C(f);
	int		m = B(f);
	int		h = (c->cols + 1)/2;
	int		n = h < m ? h : m;

	if (n)
	{
		X(f) -= n;
		B(f) -= n;
		return E_OK;
	}
	return E_REQUEST_DENIED;	/* at top */
}
