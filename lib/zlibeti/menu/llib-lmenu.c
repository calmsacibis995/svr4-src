/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/llib-lmenu.c	1.4"
/*LINTLIBRARY*/
#include "menu.h"

WINDOW *win;
MENU *menu;
ITEM **items;
ITEM *item;

ITEM	*current_item(m) MENU *m; {return (ITEM *)0;}
int	free_item(i) ITEM *i; {return E_OK;}
int	free_menu(m) MENU *m; {return E_OK;}
char	*item_description(i) ITEM *i; {return "description";}
PTF_void	item_init(m) MENU *m; {return (PTF_void)0;}
char	*item_name(i) ITEM *i; {return "name";}
OPTIONS	item_opts(i) ITEM *i; {return O_SELECTABLE;}
PTF_void	item_term(m) MENU *m; {return (PTF_void)0;}
char	*item_userptr(i) ITEM *i; {return "item_userptr";}
int	item_count(m) MENU *m; {return 0;}
int	item_index(i) ITEM *i; {return 0;}
int	item_value(i) ITEM *i; {return 0;}
int	item_visible(i) ITEM *i; {return TRUE;}
chtype	menu_back(m) MENU *m; {return A_NORMAL;}
int	menu_driver(m, c) MENU *m; int c; {return E_OK;}
chtype	menu_fore(m) MENU *m; {return A_NORMAL;}
void	menu_format(m, r, c) MENU *m; int *r, *c; {}
chtype	menu_grey(m) MENU *m; {return A_NORMAL;}
PTF_void	menu_init(m) MENU *m; {return (PTF_void)0;}
ITEM	**menu_items(m) MENU *m; {return items;}
char	*menu_mark(m) MENU *m; {return "-";}
OPTIONS	menu_opts(m) MENU *m; {return O_ONEVALUE;}
int	menu_pad(m) MENU *m; {return ' ';}
char	*menu_pattern(m) MENU *m; {return "pattern";}
WINDOW	*menu_sub(m) MENU *m; {return win;}
PTF_void	menu_term(m) MENU *m; {return (PTF_void)0;}
char	*menu_userptr(m) MENU *m; {return "menu_userptr";}
WINDOW	*menu_win(m) MENU *m; {return win;}
ITEM	*new_item(n, d) char *n, *d; {return item;}
MENU	*new_menu(i) ITEM **i; {return menu;}
int	pos_menu_cursor(m) MENU *m; {return E_OK;}
int	post_menu(m) MENU *m; {return E_OK;}
int	scale_menu(m, r, c) MENU *m; int *r, *c; {return E_OK;}
int	set_current_item(m, i) MENU *m; ITEM *i; {return E_OK;}
int	set_item_init(m, f) MENU *m; PTF_void f; {return E_OK;}
int	set_item_opts(i, o) ITEM *i; OPTIONS o; {return E_OK;}
int	set_item_term(m, f) MENU *m; PTF_void f; {return E_OK;}
int	set_item_userptr(i, u) ITEM *i; char *u; {return E_OK;}
int	set_item_value(i, v) ITEM *i; int v; {return E_OK;}
int	set_menu_back(m, a) MENU *m; chtype a; {return E_OK;}
int	set_menu_fore(m, a) MENU *m; chtype a; {return E_OK;}
int	set_menu_format(m, r, c) MENU *m; int r, c; {return E_OK;}
int	set_menu_grey(m, a) MENU *m; chtype a; {return E_OK;}
int	set_menu_init(m, f) MENU *m; PTF_void f; {return E_OK;}
int	set_menu_items(m, i) MENU *m; ITEM **i; {return E_OK;}
int	set_menu_mark(m, s) MENU *m; char *s; {return E_OK;}
int	set_menu_opts(m, o) MENU *m; OPTIONS o; {return E_OK;}
int	set_menu_pad(m, p) MENU *m; int p; {return E_OK;}
int	set_menu_pattern(m, p) MENU *m; char *p; {return E_OK;}
int	set_menu_sub(m, w) MENU *m; WINDOW *w; {return E_OK;}
int	set_menu_term(m, f) MENU *m; PTF_void f; {return E_OK;}
int	set_menu_userptr(m, u) MENU *m; char *u; {return E_OK;}
int	set_menu_win(m, w) MENU *m; WINDOW *w; {return E_OK;}
int	set_top_row(m, i) MENU *m; int i; {return E_OK;}
int	top_row(m) MENU *m; {return 0;}
int	unpost_menu(m) MENU *m; {return E_OK;}
