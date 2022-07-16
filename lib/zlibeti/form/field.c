/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/field.c	1.7"

#include "utility.h"

	/******************
	*  default field  *
	******************/

static FIELD default_field =
{
			0,			/* status	*/
			0,			/* rows		*/
			0,			/* cols		*/
			0,			/* frow		*/
			0,			/* fcol		*/
			0,			/* drows	*/
			0,			/* dcols	*/
			0,			/* maxgrow	*/
			0,			/* nrow		*/
			0,			/* nbuf		*/
			NO_JUSTIFICATION,	/* just		*/
			0,			/* page		*/
			0,			/* index	*/
			' ',			/* pad		*/
			A_NORMAL,		/* fore		*/
			A_NORMAL,		/* back		*/
			O_VISIBLE	|
			O_ACTIVE	|
			O_PUBLIC	|
			O_EDIT		|
			O_WRAP		|
			O_BLANK		|
			O_AUTOSKIP	|
			O_NULLOK	|
			O_PASSOK	|
			O_STATIC,		/* opts		*/
			(FIELD *) 0,		/* snext	*/
			(FIELD *) 0,		/* sprev	*/
			(FIELD *) 0,		/* link		*/
			(FORM *) 0,		/* form		*/
			(FIELDTYPE *) 0,	/* type		*/
			(char *) 0,		/* arg		*/
			(char *) 0,		/* buf		*/
			(char *) 0,		/* usrptr	*/
};

FIELD * _DEFAULT_FIELD	= &default_field;

	/*************
	*  MakeType  *
	*************/

static int MakeType (f, ap)
FIELD * f;
va_list * ap;
{
	int err = 0;

	f -> arg = MakeArg (f, ap, &err);	/* pick off type specific args	*/

	if (err)
	{
		FreeArg (f);			/* release type specific args	*/
		f -> type = (FIELDTYPE *) 0;
		f -> arg = (char *) 0;
		return FALSE;
	}
	IncrType (f -> type);			/* increment reference count	*/
	return TRUE;
}

	/*************
	*  CopyType  *
	*************/

static int CopyType (f, fsrc)
FIELD * f;
FIELD * fsrc;
{
	int err = 0;

	f -> type = fsrc -> type;		/* copy field type		*/
	f -> arg = CopyArg (fsrc, &err);	/* copy type specific info	*/

	if (err)
	{
		FreeArg (f);			/* release type specific args	*/
		f -> type = (FIELDTYPE *) 0;
		f -> arg = (char *) 0;
		return FALSE;
	}
	IncrType (f -> type);			/* increment reference count	*/
	return TRUE;
}

	/*************
	*  FreeType  *
	*************/

static void FreeType (f)
FIELD * f;
{
	DecrType (f -> type);			/* decrement reference count	*/
	FreeArg (f);				/* release type specific args	*/
}

	/**************
	*  new_field  *
	**************/

FIELD * new_field (rows, cols, frow, fcol, nrow, nbuf)
int rows;	/* number of visible rows		*/
int cols;	/* number of visible cols		*/
int frow;	/* first row relative to form origin	*/
int fcol;	/* first col relative to form origin	*/
int nrow;	/* number of off screen rows		*/
int nbuf;	/* number of additional buffers		*/
{
	FIELD * f = (FIELD *) 0;
	int i, size;

	if (	rows > 0
	&&	cols > 0
	&&	frow >= 0
	&&	fcol >= 0
	&&	nrow >= 0
	&&	nbuf >= 0
	&&	Alloc (f, FIELD)	)
	{
		*f = *_DEFAULT_FIELD;

		f -> rows	= rows;
		f -> cols	= cols;
		f -> frow	= frow;
		f -> fcol	= fcol;
		f -> drows	= rows + nrow;
		f -> dcols	= cols;
		f -> nrow	= nrow;
		f -> nbuf	= nbuf;
		f -> link	= f;

		if (CopyType (f, _DEFAULT_FIELD))
		{
			size = TotalBuf (f);

			if (arrayAlloc (Buf (f), size, char))
			{
				(void)memset (Buf (f), ' ', size);

				for (i = 0; i <= f -> nbuf; ++i)
					*(Buffer (f, i+1) - 1) = '\0';
				return f;
			}
		}
	}
	(void)free_field (f);
	return (FIELD *) 0;
}

	/**************
	*  dup_field  *
	**************/

FIELD * dup_field (field, frow, fcol)
FIELD * field;	/* field to duplicate			*/
int frow;	/* first row relative to form origin	*/
int fcol;	/* first col relative to form origin	*/
{
	FIELD * f = (FIELD *) 0;
	int size;

	if (	field
	&&	frow >= 0
	&&	fcol >= 0
	&&	Alloc (f, FIELD)	)
	{
		*f = *_DEFAULT_FIELD;

		f -> frow = frow;
		f -> fcol = fcol;
		f -> link = f;

		f -> rows	= field -> rows;
		f -> cols	= field -> cols;
		f -> drows	= field -> drows;
		f -> dcols	= field -> dcols;
		f ->maxgrow = field -> maxgrow;
		f -> nrow	= field -> nrow;
		f -> nbuf	= field -> nbuf;
		f -> just	= field -> just;
		f -> fore	= field -> fore;
		f -> back	= field -> back;
		f -> pad	= field -> pad;
		f -> opts	= field -> opts;
		f -> usrptr	= field -> usrptr;
		f -> status = Status( field, GROWABLE );

		if (CopyType (f, field))
		{
			size = TotalBuf (f);

			if (arrayAlloc (Buf (f), size, char))
			{
				(void)memcpy (Buf (f), Buf (field), size);
				return f;
			}
		}
	}
	(void)free_field (f);
	return (FIELD *) 0;
}

	/***************
	*  link_field  *
	***************/

FIELD * link_field (field, frow, fcol)
FIELD * field;	/* field to link to			*/
int frow;	/* first row relative to form origin	*/
int fcol;	/* first col relative to form origin	*/
{
	FIELD * f = (FIELD *) 0;

	if (	field
	&&	frow >= 0
	&&	fcol >= 0
	&&	Alloc (f, FIELD)	)
	{
		*f = *_DEFAULT_FIELD;

		f -> frow = frow;
		f -> fcol = fcol;

		f -> link = field -> link;
		field -> link = f;		/* add field to linked list */

		f -> buf	= field -> buf;
		f -> rows	= field -> rows;
		f -> cols	= field -> cols;
		f -> drows	= field -> drows;
		f -> dcols	= field -> dcols;
		f ->maxgrow = field -> maxgrow;
		f -> nrow	= field -> nrow;
		f -> nbuf	= field -> nbuf;
		f -> just	= field -> just;
		f -> fore	= field -> fore;
		f -> back	= field -> back;
		f -> pad	= field -> pad;
		f -> opts	= field -> opts;
		f -> usrptr	= field -> usrptr;
		f -> status = Status( field, GROWABLE );

		if (CopyType (f, field))
			return f;
	}
	(void)free_field (f);
	return (FIELD *) 0;
}

	/***************
	*  free_field  *
	***************/

int free_field (f)
FIELD *	 f;
{
	FIELD * p;

	if (! f)
		return E_BAD_ARGUMENT;

	if (f -> form)
		return E_CONNECTED;

	if (f -> link != f)		/* check for linked field	*/
	{
		for (p = f -> link; p -> link != f; p = p -> link)
		;
		p -> link = f -> link;	/* delete from list		*/
	}
	else
		Free (Buf (f));		/* free buffer space		*/

	FreeType (f);
	Free (f);
	return E_OK;
}

	/***************
	*  field_info  *
	***************/

int field_info (f, rows, cols, frow, fcol, nrow, nbuf)
FIELD * f;	/* field whose information is wanted	*/
int * rows;	/* number of visible rows		*/
int * cols;	/* number of visible cols		*/
int * frow;	/* first row relative to form origin	*/
int * fcol;	/* first col relative to form origin	*/
int * nrow;	/* number of off screen rows		*/
int * nbuf;	/* number of additional buffers		*/
{
	if (! f)
		return E_BAD_ARGUMENT;

	*rows = f -> rows;
	*cols = f -> cols;
	*frow = f -> frow;
	*fcol = f -> fcol;
	*nrow = f -> nrow;
	*nbuf = f -> nbuf;
	return E_OK;
}

	/******************
	*  set_max_field  *
	******************/

int set_max_field( f, max )
FIELD	*f;
int		max;
{
	BOOLEAN	onerow = OneRow(f);

	if ( !f || max && ((onerow && f->dcols > max)
						|| (!onerow && f->drows > max)))
		return E_BAD_ARGUMENT;

	f->maxgrow = max;
	Clr( f, GROWABLE );

	if ( !Opt( f, O_STATIC ) && ((!max || onerow && f->dcols < max)
									|| (!onerow && f->drows < max)))
	{
		Set( f, GROWABLE );
	}

	return E_OK;
}

	/***********************
	*  dynamic_field_info  *
	***********************/

int dynamic_field_info (f, drows, dcols, max)
FIELD * f;		/* field whose information is wanted	*/
int * drows;	/* number of actual rows		*/
int * dcols;	/* number of actual cols		*/
int * max;		/* maximum growth allowable, else -1	*/
{
	if (! f)
		return E_BAD_ARGUMENT;

	*drows = f -> drows;
	*dcols = f -> dcols;
	*max   = f -> maxgrow;
	return E_OK;
}

	/***************
	*  move_field  *
	***************/

int move_field (f, frow, fcol)
FIELD * f;	/* field to move			*/
int frow;	/* first row relative to form origin	*/
int fcol;	/* first col relative to form origin	*/
{
	if (! f || frow < 0 || fcol < 0)
		return E_BAD_ARGUMENT;

	if (f -> form)
		return E_CONNECTED;

	f -> frow = frow;
	f -> fcol = fcol;
	return E_OK;
}

	/*******************
	*  set_field_type  *
	*******************/

#ifdef __STDC__
int set_field_type (FIELD *f, FIELDTYPE *ft, ...)
#else
int set_field_type (va_alist)
va_dcl
#endif
{
	va_list		ap;
	int		v = E_SYSTEM_ERROR;
#ifndef __STDC__
	FIELD		*f;
	FIELDTYPE	*ft;
#endif

#ifdef __STDC__
	va_start (ap, ft);
#else
	va_start (ap);
	f = va_arg (ap, FIELD *);		/* pick off field	*/
	ft = va_arg (ap, FIELDTYPE *);		/* pick off field type	*/
#endif
	f = Field (f);
	FreeType (f);				/* free old type	*/
	f -> type = ft;

	if (MakeType (f, &ap))			/* set up new type	*/
		v = E_OK;
	va_end (ap);
	return v;
}

FIELDTYPE * field_type (f)
FIELD * f;
{
	return Field(f) -> type;
}

char * field_arg (f)
FIELD * f;
{
	return Field(f) -> arg;
}

	/*****************
	*  set_new_page  *
	*****************/

int set_new_page (f, flag)
FIELD * f;
int flag;
{
	f = Field (f);

	if (f -> form)
		return E_CONNECTED;

	if (flag)
		Set (f, NEW_PAGE);
	else
		Clr (f, NEW_PAGE);

	return E_OK;
}

int new_page (f)
FIELD * f;
{
	if (Status (Field (f), NEW_PAGE))
		return TRUE;
	else
		return FALSE;
}

