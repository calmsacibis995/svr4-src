/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:form.h	1.1"
#ifndef _FORM
#define _FORM

#include "menu.h"
#include "track.h"
#include "sys/window.h"

#define F_LRMARGIN	(M_LRMARGIN)	/* left/right margin		*/
#define F_TBMARGIN	(M_TBMARGIN)	/* top/bottom margin		*/
#define F_MAXWIDTH	(M_MAXWIDTH)	/* maximum width		*/
#define F_MAXHEIGHT	(M_MAXHEIGHT)	/* maximum height		*/

#define F_BORDFLAGS	(BORDHELP|BORDCANCEL)

#define F_BEGIN		0x1		/* begin a menu			*/
#define F_END		0x2		/* end (delete) the menu	*/
#define F_INPUT		0x4		/* accept input			*/
#define F_POPUP		(F_BEGIN|F_END|F_INPUT)

					/* form flags			*/
#define F_WINSON	0x1		/* use son placement algorithm	*/
#define F_WINNEW	0x2		/* use new placement algorithm	*/
#define F_USEWIN	0x4		/* use supplied window		*/
#define F_NOMOVE	0x10		/* to turn off move icon	*/
#define F_NOHELP	0x20		/* to turn off help icon	*/
#define F_RDONLY	0x40		/* read only form, no inputs	*/

					/* field flags			*/
#define F_CLEARIT	0x1		/* clear entry on first edit op	*/
#define F_MONLY		0x2		/* force menu choices if menu	*/

#define FERR_OK		0		/* no error			*/
#define FERR_TOOBIG	-1		/* too many items in menu	*/
#define FERR_ARGS	-2		/* catch all for bad args	*/
#define FERR_NOWIN	-3		/* can't create a window	*/
#define FERR_SYS	-4		/* system error (signal)	*/
#define FERR_GETSTAT	-5		/* error in 'stat'ing old win	*/
#define FERR_SETSTAT	-6		/* error in 'stat'ing old win	*/
#define FERR_NOMEM	-7		/* if malloc failed		*/
#define FERR_BIG	-8		/* error window too big		*/
#define FERR_WRITE	-9		/* if a write fails		*/

typedef struct
{
	char		*fl_name;	/* field name			*/
	char		fl_row;		/* field row			*/
	char		fl_ncol;	/* name column			*/
	char		fl_fcol;	/* field column			*/
	char		fl_len;		/* field length			*/
	char		fl_flags;	/* field flags			*/
	char		*fl_value;	/* field value (initial/final)	*/
	menu_t		*fl_menu;	/* optional menu pointer	*/
	char		*fl_prompt;	/* field prompt			*/
} field_t;

typedef struct
{
	char		*f_label;	/* form label			*/
	char		*f_name;	/* form name			*/
	char		f_flags;	/* form flags			*/
	int		f_win;		/* form window			*/
	track_t		*f_track;	/* form track list		*/
	field_t		*f_fields;	/* fields			*/
	field_t		*f_curfl;	/* current field		*/
} form_t;

#endif /* _FORM */
