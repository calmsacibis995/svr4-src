/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/private.h	1.9"
#include "menu.h"

/* Menu macros to access menu structure */

#define Height(m)	(m)->height
#define Width(m)	(m)->width
#define Rows(m)		(m)->rows
#define Cols(m)		(m)->cols
#define FRows(m)	(m)->frows
#define FCols(m)	(m)->fcols
#define MaxName(m)	(m)->namelen
#define MaxDesc(m)	(m)->desclen
#define Marklen(m)	(m)->marklen
#define Itemlen(m)	(m)->itemlen
#define Pattern(m)	(m)->pattern
#define Pindex(m)	(m)->pindex
#define IthPattern(m, i)	(m)->pattern[i]
#define Win(m)		(m)->win
#define Sub(m)		(m)->sub
#define UserWin(m)	(m)->userwin
#define UserSub(m)	(m)->usersub
#define UW(m)		(UserWin(m) ? UserWin(m) : stdscr)
#define US(m)		(UserSub(m) ? UserSub(m) : UW(m))
#define Items(m)	(m)->items
#define IthItem(m,i)	(m)->items[i]
#define Nitems(m)	(m)->nitems
#define Current(m)	(m)->curitem
#define Top(m)		(m)->toprow
#define Pad(m)		(m)->pad
#define Fore(m)		(m)->fore
#define Back(m)		(m)->back
#define Grey(m)		(m)->grey
#define InvalidAttr(a)	(((a) & (chtype) A_ATTRIBUTES) != (a))
#define Mhelp(m)	(m)->help
#define	Muserptr(m)	(m)->userptr
#define Mopt(m)		(m)->opt
#define Mark(m)		(m)->mark
#define Mstatus(m)	(m)->status
#define Posted(m)	(Mstatus(m) & _POSTED)
#define Indriver(m)	(Mstatus(m) & _IN_DRIVER)
#define LinkNeeded(m)	(Mstatus(m) & _LINK_NEEDED)
#define SetPost(m)	(Mstatus(m) |= _POSTED)
#define SetDriver(m)	(Mstatus(m) |= _IN_DRIVER)
#define SetLink(m)	(Mstatus(m) |= _LINK_NEEDED)
#define ResetPost(m)	(Mstatus(m) &= ~_POSTED)
#define ResetDriver(m)	(Mstatus(m) &= ~_IN_DRIVER)
#define ResetLink(m)	(Mstatus(m) &= ~_LINK_NEEDED)
#define SMinit(m)	(m)->menuinit
#define SMterm(m)	(m)->menuterm
#define SIinit(m)	(m)->iteminit
#define SIterm(m)	(m)->itemterm
#define Minit(m)	if (m->menuinit) { \
			  SetDriver(m); \
			  (m)->menuinit(m); \
			  ResetDriver(m); \
			}
#define Mterm(m)	if (m->menuterm) { \
			  SetDriver(m); \
			  (m)->menuterm(m); \
			  ResetDriver(m); \
			}
#define Iinit(m)	if (m->iteminit) { \
			  SetDriver(m); \
			  (m)->iteminit(m); \
			  ResetDriver(m); \
			}
#define Iterm(m)	if (m->itemterm) { \
			  SetDriver(m); \
			  (m)->itemterm(m); \
			  ResetDriver(m); \
			}

/* Define access to Mopt */

#define OneValue(m)	(Mopt(m) & O_ONEVALUE)
#define ShowDesc(m)	(Mopt(m) & O_SHOWDESC)
#define RowMajor(m)	(Mopt(m) & O_ROWMAJOR)
#define IgnoreCase(m)	(Mopt(m) & O_IGNORECASE)
#define ShowMatch(m)	(Mopt(m) & O_SHOWMATCH)
#define Cyclic(m)	(!(Mopt(m) & O_NONCYCLIC))

/* Item macros to access item structure */

#define Name(i)		(i)->name.str
#define NameLen(i)	(i)->name.length
#define Description(i)	(i)->description.str
#define DescriptionLen(i)	(i)->description.length
#define Index(i)	(i)->index
#define Y(i)		(i)->y
#define X(i)		(i)->x
#define Imenu(i)	(i)->imenu
#define Value(i)	(i)->value
#define Ihelp(i)	(i)->help
#define Iuserptr(i)	(i)->userptr
#define Iopt(i)		(i)->opt
#define Istatus(i)	(i)->status
#define Up(i)		(i)->up
#define Down(i)		(i)->down
#define Left(i)		(i)->left
#define Right(i)	(i)->right
#define Selectable(i)	(Iopt(i) & O_SELECTABLE)

/* Default menu macros */

#define Dfl_Menu	(&_Default_Menu)
#define Dfl_Item	(&_Default_Item)

#define max(a,b)	((a)>(b))?(a):(b)
#define min(a,b)	((a)<(b))?(a):(b)

extern MENU		_Default_Menu;
extern ITEM		_Default_Item;

extern void		free ();
extern char		*calloc (),
			*malloc ();
extern void		_affect_change (),
			_chk_current (),
			_chk_top (),
			_disconnect(),
			_draw (),
			_link_items (),
			_move_post_item (),
			_movecurrent (),
			_position_cursor (),
			_scale (),
			_show ();
