/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:menu.h	1.1"
#ifndef _MENU
#define _MENU

#include "track.h"
#include "sys/window.h"

extern char *strtok(), *strchr();
					/* cosmetic and build params	*/
#define MIN(a,b)	(((a)<(b))?(a):(b))
#define MAX(a,b)	(((a)>(b))?(a):(b))
#define ROUND(a,b)	(((a)+(b-1))/(b))
#define MINROW		4	/* min space in rows for vscroll arrows */

#define M_MAXLINE	76		/* size of typing line		*/
#define M_SHORT		15		/* size before leave 1 col	*/
#define M_MAXHEIGHT	20		/* maximum menu height		*/
#define M_MAXWIDTH	75		/* maximum menu width		*/

#define M_LRMARGIN	2		/* size of left, right margins	*/
#define M_TBMARGIN	1		/* size of top/bottom margins	*/
#define M_CSPACE	2		/* space between columns	*/

#define M_BORDFLAGS	(BORDHELP|BORDCANCEL|BORDVSCROLL|BORDRESIZE)

					/* menu operations		*/
#define M_BEGIN		0x1		/* begin a menu			*/
#define M_END		0x2		/* end (delete) the menu	*/
#define M_INPUT		0x4		/* accept input			*/
#define M_DESEL		0x8		/* de-select everything first	*/
#define M_POPUP		(M_BEGIN|M_END|M_INPUT|M_DESEL)

					/* menu flags			*/
#define M_SINGLE	0x1		/* allow only single selections	*/
#define M_USEWIN	0x2		/* use caller's window		*/
#define M_WINSON	0x4		/* use son placement algorithm	*/
#define M_WINNEW	0x8		/* use new placement algorithm	*/
#define M_NOMOVE	0X10		/* true if no move icon		*/
#define M_NOHELP	0X20		/* true if no help icon		*/
#define M_NORESIZE	0X40		/* true if no resize icon	*/
#define M_ASISTITLE	0X80		/* true if use title as is      */

					/* item flags			*/
#define M_MARKED	0x1		/* item is marked		*/
#define M_DIMMED	0x2		/* item is lower intensity	*/
#define M_REDISP	0x80		/* needs redisplay (internal)	*/

					/* return codes			*/
					/* positive number is keycode	*/
#define MERR_OK		0		/* returned when no keystroke	*/
#define MERR_SYS	-1		/* system err (signal)		*/
#define MERR_ARGS	-2		/* catch all for bad args	*/
#define MERR_NOWIN	-3		/* can't create a window	*/
#define MERR_TOOSMALL	-4		/* window is too small		*/
#define MERR_GETSTAT	-5		/* error during getstat		*/
#define MERR_SETSTAT	-6		/* error during setstat		*/
#define MERR_NOMEM	-7		/* malloc failed		*/
#define MERR_BIG	-8		/* window is too big		*/
#define MERR_WRITE	-9		/* window write failed		*/

typedef struct
{
	char		*mi_name;	/* name of item			*/
	char		mi_flags;	/* flags			*/
	int		mi_val;		/* user-supplied value		*/
} mitem_t;

typedef struct
{
	char		*m_label;	/* menu label			*/
	char		*m_title;	/* menu title			*/
	char		*m_prompt;	/* menu prompt			*/
	char		m_rows;		/* desired rows			*/
	char		m_cols;		/* desired cols			*/
	char		m_iwidth;	/* truncation width		*/
	char		m_iheight;	/* item height			*/
	char		m_flags;	/* flags			*/
	char		m_lbuf[M_MAXLINE]; /* input buffer		*/
	int		m_win;		/* window pointer		*/
	track_t		*m_track;	/* ptr to allocated track	*/
	int		m_oldwidth;	/* last known width		*/
	int		m_oldheight;	/* last known height		*/
	int		m_selcnt;	/* count of # selected		*/
	mitem_t		*m_items;	/* pointer to items		*/
	mitem_t		*m_curi;	/* current item			*/
	mitem_t		*m_topi;	/* top item			*/
} menu_t;

#endif /*_MENU*/
