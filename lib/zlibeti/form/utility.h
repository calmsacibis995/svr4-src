/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:form/utility.h	1.8"

#ifndef UTILITY_H
#define UTILITY_H

#include "form.h"
#include <memory.h>
#include <string.h>
#include <ctype.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

extern char *	malloc ();
extern void	free ();

typedef	int		BOOLEAN;

	/***************************
	*  miscellaneous #defines  *
	***************************/

#define	MIN(x,y)		((x) < (y) ? (x) : (y))

/*
	form status flags
*/
#define POSTED			0x0001	/* posted flag			*/
#define DRIVER			0x0002	/* inside init/term routine	*/
#define OVERLAY			0x0004	/* insert/overlay mode		*/
#define WIN_CHG			0x0010	/* window change (system flag)	*/
#define BUF_CHG			0x0020	/* buffer change (system flag)	*/
/*
	field status flags
*/
#define USR_CHG			0x0001	/* buffer change (user's flag)	*/
#define TOP_CHG			0x0002	/* toprow change (system flag)	*/
#define NEW_PAGE		0x0004	/* new page (system flag)	*/
#define GROWABLE		0x0008	/* growable page (system flag)	*/
/*
	field type status flags
*/
#define LINKED			0x0001	/* conjunctive field type	*/
#define ARGS			0x0002	/* has additional arguments	*/
#define CHOICE			0x0004	/* has choice functions		*/
/*
	form/field/fieldtype status manipulation macros
*/
#define Status(f,s)		((f) -> status & (s))
#define Set(f,s)		((f) -> status |= (s))
#define Clr(f,s)		((f) -> status &= ~(s))
/*
	form/field option manipulation macros
*/
#define Opt(f,x)		((f) -> opts & (x))
/*
	alloc/free with check
*/
#define Alloc(x,t)		((x = (t *) malloc (sizeof (t))) != (t *)0)
#define arrayAlloc(x,n,t)	((x = (t *) malloc ((n) * sizeof (t))) != (t *)0)
#define Free(x)			{ if (x) free(x); }
/*
	field type macros
*/
#define MakeArg(f,p,err)	(_makearg	((f) -> type, p, err))
#define CopyArg(f,err)		(_copyarg	((f) -> type, (f) -> arg, err))
#define FreeArg(f)		(_freearg	((f) -> type, (f) -> arg))
#define CheckField(f)		(_checkfield	((f) -> type, (f), (f) -> arg))
#define CheckChar(f,c)		(_checkchar	((f) -> type, (c), (f) -> arg))
#define NextChoice(f)		(_nextchoice	((f) -> type, (f), (f) -> arg))
#define PrevChoice(f)		(_prevchoice	((f) -> type, (f), (f) -> arg))
#define IncrType(type)		{ if (type) ++(type -> ref); }
#define DecrType(type)		{ if (type) --(type -> ref); }
/*
	form/field init/term calls
*/
#define init_field(f)		{					\
					if ((f) -> fieldinit)		\
					{				\
						Set (f, DRIVER);	\
						(*(f) -> fieldinit)(f);	\
						Clr (f, DRIVER);	\
					}				\
				}
#define term_field(f)		{					\
					if ((f) -> fieldterm)		\
					{				\
						Set (f, DRIVER);	\
						(*(f) -> fieldterm)(f);	\
						Clr (f, DRIVER);	\
					}				\
				}
#define init_form(f)		{					\
					if ((f) -> forminit)		\
					{				\
						Set (f, DRIVER);	\
						(*(f) -> forminit)(f);	\
						Clr (f, DRIVER);	\
					}				\
				}
#define term_form(f)		{					\
					if ((f) -> formterm)		\
					{				\
						Set (f, DRIVER);	\
						(*(f) -> formterm)(f);	\
						Clr (f, DRIVER);	\
					}				\
				}
/*
	page macros
*/
#define P(f)			((f) -> curpage)
#define Pmin(f,p)		((f) -> page [p].pmin)
#define Pmax(f,p)		((f) -> page [p].pmax)
#define Smin(f,p)		((f) -> page [p].smin)
#define Smax(f,p)		((f) -> page [p].smax)
/*
	form macros
*/
#define Form(f)			((f) ? (f) : _DEFAULT_FORM)
#define ValidIndex(f,i)		((i) >= 0 && (i) < (f) -> maxfield)
#define ValidPage(f,i)		((i) >= 0 && (i) < (f) -> maxpage)
#define C(f)			((f) -> current)
#define W(f)			((f) -> w)
#define X(f)			((f) -> curcol)
#define Y(f)			((f) -> currow)
#define T(f)			((f) -> toprow)
#define B(f)			((f) -> begincol)
#define Xmax(f)			(C(f) -> dcols)
#define Ymax(f)			(C(f) -> drows)
#define Win(f)			((f) -> win ? (f) -> win : stdscr)
#define Sub(f)			((f) -> sub ? (f) -> sub : Win(f))
/*
	field macros
*/
#define Field(f)		((f) ? (f) : _DEFAULT_FIELD)
#define Buf(f)			((f) -> buf)
#define	OneRow(f)		((f)->rows + (f)->nrow == 1)
#define GrowSize(f)		(((f) -> rows + (f) -> nrow) * (f) -> cols)
#define BufSize(f)		((f) -> drows  * (f) -> dcols)
#define Buffer(f,n)		(Buf(f) + (n) * (BufSize(f) + 1))
#define LineBuf(f,n)		(Buf(f) + (n) * (f) -> dcols)
#define TotalBuf(f)		((BufSize(f) + 1) * ((f) -> nbuf + 1))
#define Just(f)			((f) -> just)
#define Fore(f)			((f) -> fore)
#define Back(f)			((f) -> back)
#define Pad(f)			((f) -> pad)
/*
	system externs
*/
extern int		_next_page ();		/* REQ_NEXT_PAGE	*/
extern int		_prev_page ();		/* REQ_PREV_PAGE	*/
extern int		_first_page ();		/* REQ_FIRST_PAGE	*/
extern int		_last_page ();		/* REQ_LAST_PAGE	*/

extern int		_next_field ();		/* REQ_NEXT_FIELD	*/
extern int		_prev_field ();		/* REQ_PREV_FIELD	*/
extern int		_first_field ();	/* REQ_FIRST_FIELD	*/
extern int		_last_field ();		/* REQ_LAST_FIELD	*/
extern int		_snext_field ();	/* REQ_SNEXT_FIELD	*/
extern int		_sprev_field ();	/* REQ_SPREV_FIELD	*/
extern int		_sfirst_field ();	/* REQ_SFIRST_FIELD	*/
extern int		_slast_field ();	/* REQ_SLAST_FIELD	*/
extern int		_left_field ();		/* REQ_LEFT_FIELD	*/
extern int		_right_field ();	/* REQ_RIGHT_FIELD	*/
extern int		_up_field ();		/* REQ_UP_FIELD		*/
extern int		_down_field ();		/* REQ_DOWN_FIELD	*/

extern int		_next_char ();		/* REQ_NEXT_CHAR	*/
extern int		_prev_char ();		/* REQ_PREV_CHAR	*/
extern int		_next_line ();		/* REQ_NEXT_LINE	*/
extern int		_prev_line ();		/* REQ_PREV_LINE	*/
extern int		_next_word ();		/* REQ_NEXT_WORD	*/
extern int		_prev_word ();		/* REQ_PREV_WORD	*/
extern int		_beg_field ();		/* REQ_BEG_FIELD	*/
extern int		_end_field ();		/* REQ_END_FIELD	*/
extern int		_beg_line ();		/* REQ_BEG_LINE		*/
extern int		_end_line ();		/* REQ_END_LINE		*/
extern int		_left_char ();		/* REQ_LEFT_CHAR	*/
extern int		_right_char ();		/* REQ_RIGHT_CHAR	*/
extern int		_up_char ();		/* REQ_UP_CHAR		*/
extern int		_down_char ();		/* REQ_DOWN_CHAR	*/

extern int		_new_line ();		/* REQ_NEW_LINE		*/
extern int		_ins_char ();		/* REQ_INS_CHAR		*/
extern int		_ins_line ();		/* REQ_INS_LINE		*/
extern int		_del_char ();		/* REQ_DEL_CHAR		*/
extern int		_del_prev ();		/* REQ_DEL_PREV		*/
extern int		_del_line ();		/* REQ_DEL_LINE		*/
extern int		_del_word ();		/* REQ_DEL_WORD		*/
extern int		_clr_eol ();		/* REQ_CLR_EOL		*/
extern int		_clr_eof ();		/* REQ_CLR_EOF		*/
extern int		_clr_field ();		/* REQ_CLR_FIELD	*/
extern int		_ovl_mode ();		/* REQ_OVL_MODE		*/
extern int		_ins_mode ();		/* REQ_INS_MODE		*/
extern int		_scr_fline ();		/* REQ_SCR_FLINE	*/
extern int		_scr_bline ();		/* REQ_SCR_BLINE	*/
extern int		_scr_fpage ();		/* REQ_SCR_FPAGE	*/
extern int		_scr_fhpage ();		/* REQ_SCR_FHPAGE	*/
extern int		_scr_bpage ();		/* REQ_SCR_BPAGE	*/
extern int		_scr_bhpage ();		/* REQ_SCR_BHPAGE	*/

extern int		_scr_fchar ();		/* REQ_SCR_FCHAR	*/
extern int		_scr_bchar ();		/* REQ_SCR_BCHAR	*/
extern int		_scr_hfline ();		/* REQ_SCR_HFLINE	*/
extern int		_scr_hbline ();		/* REQ_SCR_HBLINE	*/
extern int		_scr_hfhalf ();		/* REQ_SCR_HFHALF	*/
extern int		_scr_hbhalf ();		/* REQ_SCR_HBHALF	*/

extern int		_validation ();		/* REQ_VALIDATION	*/
extern int		_next_choice ();	/* REQ_NEXT_CHOICE	*/
extern int		_prev_choice ();	/* REQ_PREV_CHOICE	*/

extern char *		_makearg ();
extern char *		_copyarg ();
extern void		_freearg ();
extern int		_checkfield ();
extern int		_checkchar ();
extern int		_nextchoice ();
extern int		_prevchoice ();

extern BOOLEAN		_grow_field();
extern FIELD *		_first_active ();
extern char *		_data_beg ();
extern char *		_data_end ();
extern char *		_whsp_beg ();
extern char *		_whsp_end ();
extern void		_buf_to_win ();
extern void		_win_to_buf ();
extern void		_adjust_cursor ();
extern void		_sync_buffer ();
extern int		_sync_linked ();
extern int		_sync_field ();
extern int		_sync_attrs ();
extern int		_sync_opts ();
extern int		_validate ();
extern int		_set_current_field ();
extern int		_set_form_page ();
extern int		_pos_form_cursor ();
extern int		_update_current ();
extern int		_data_entry ();
extern int		_page_navigation ();
extern int		_field_navigation ();
extern int		_data_navigation ();
extern int		_data_manipulation ();
extern int		_misc_request ();

#endif	/* UTILITY_H */
